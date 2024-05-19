#include <pump/medtrum/comm/PumpBleComm.h>
#include <zephyr/bluetooth/gatt.h>

#include "PumpScanner.h"
#include <pump/medtrum/crypt/Crypt.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ib_medtrum_ble);

namespace
{
    // "669A9001-0008-968F-E311-6050405558B3" Pump service
    bt_uuid_128 BT_UUID_MT_SERVICE =
        BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x669a9001, 0x0008, 0x968f, 0xe311, 0x6050405558b3));
    // "669a9120-0008-968f-e311-6050405558b3" Notifications from pump
    bt_uuid_128 BT_UUID_MT_READ =
        BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x669a9120, 0x0008, 0x968f, 0xe311, 0x6050405558b3));
    // "669a9101-0008-968f-e311-6050405558b3" Command, and indication with response
    bt_uuid_128 BT_UUID_MT_WRITE =
        BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x669a9101, 0x0008, 0x968f, 0xe311, 0x6050405558b3));
}

// TODO: Helper funcs, move to seperate file
void vector_add_le32(std::vector<uint8_t>& vec, uint32_t value) {
    vec.push_back(static_cast<uint8_t>(value & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

PumpBleComm::PumpBleComm()
{
    mConnection.callback = this;
    
    k_work_queue_init(&mWorkQueue);
    k_work_queue_start(&mWorkQueue, mWorkQueueBuffer, K_THREAD_STACK_SIZEOF(mWorkQueueBuffer), 1, NULL);
    k_mutex_init(&mWriteMutex);

    // Init our tasks
    mConnectTask.pumpBleComm = this;
    k_work_init_delayable(&mConnectTask.work, [](struct k_work *work) {
        auto task = CONTAINER_OF(work, TaskWrap, work);
        task->pumpBleComm->_connect();
    });

    mOnDiscoveredTask.pumpBleComm = this;
    k_work_init_delayable(&mOnDiscoveredTask.work, [](struct k_work *work) {
        auto task = CONTAINER_OF(work, TaskWrap, work);
        task->pumpBleComm->_onDiscovered();
    });
}

PumpBleComm::~PumpBleComm()
{
    // Destructor
}

void PumpBleComm::init()
{
    // Init pump specific BLE communication
    // General BLE init should be done at this point
}

void PumpBleComm::connect()
{
    // Schedule the connect task
    submitTask(mConnectTask);
}

void PumpBleComm::onDeviceFound(bt_addr_le_t addr, ManufacturerData manufacturerData)
{
    LOG_DBG("Pump found with SN: %d", manufacturerData.deviceSN);
    // Handle the found device
    // Save the device address
    mDeviceAddr = addr;
    // Connect to the device
    submitTask(mConnectTask);
}

void PumpBleComm::onConnected(struct bt_conn *conn, uint8_t err)
{
    // Handle the connected event
    if (err)
    {
        LOG_ERR("Failed to connect to the pump with error: %d", err);
        // Try again in 10 mins
        submitTask(mConnectTask, K_SECONDS(10));
        return;
    }
    LOG_DBG("Connected to the pump");
    // Discover services
    mDiscoverParams.uuid = &BT_UUID_MT_SERVICE.uuid;
    mDiscoverParams.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    mDiscoverParams.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    mDiscoverParams.type = BT_GATT_DISCOVER_PRIMARY;
    mDiscoveryState = DISCOVERY_MT_SERVICE;
    int ret = BLEComm::discover(&mConnection, &mDiscoverParams);
    if (!ret)
    {
        LOG_DBG("Discovering services");
    }
    else
    {
        LOG_ERR("Failed to discover services with error: %d", err);
    }
}

// TASK SHIZZL

void PumpBleComm::submitTask(TaskWrap &task, k_timeout_t delay)
{
    // Submit a task to the work queue
    k_work_reschedule_for_queue(&mWorkQueue, &task.work, delay);
}

void PumpBleComm::_connect()
{
    // Connect to the pump
    if (mDeviceAddr.has_value())
    {
        // Connect to the device
        int err = BLEComm::connect(mDeviceAddr.value(), &mConnection);
        if (!err)
        {
            LOG_DBG("Connecting to the pump");
        }
        else
        {
            LOG_ERR("Failed to connect to the pump");
        }
    }
    else
    {
        // Start scanning for the device
        PumpScanner::scanRequest request = {
            .instance = this,
            .targetDeviceSN = mDeviceSN.value_or(0),
        };
        PumpScanner::startScan(request);
    }
}

void PumpBleComm::_onDiscovered()
{
    // Handle the discovery task
    LOG_DBG("onDiscovered called");
    mWriteCommandsDataBuffer.clear();
    mWriteCommandsDataBuffer.push_back(0x05);
    mWriteCommandsDataBuffer.push_back(0x02);
    mWriteCommandsDataBuffer.insert(mWriteCommandsDataBuffer.end(), {0x00, 0x00, 0x00, 0x00});
    uint32_t key = Crypt::keyGen(mDeviceSN.value_or(0));
    vector_add_le32(mWriteCommandsDataBuffer, key);
    mWriteCommandPackets = new WriteCommandPackets(mWriteCommandsDataBuffer.data(), mWriteCommandsDataBuffer.size(), 0);

    // This would be part of our write func
    if (mWriteCommandPackets == nullptr)
    {
        return;
    }
    mWriteParams.data = mWriteDataBuffer;
    mWriteParams.offset = 0;
    mWriteParams.length = mWriteCommandPackets->getNextPacket(mWriteDataBuffer);
    mWriteParams.handle = mSubscribeWriteParams.value_handle;
    mWriteParams.func = [](struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params) {
        auto pumpBleComm = CONTAINER_OF(params, PumpBleComm, mWriteParams);
        pumpBleComm->onWrite(err, params);
    };

    int err = bt_gatt_write(mConnection.conn, &mWriteParams);
    if (err)
    {
        LOG_ERR("GATT write operation failed");
        return;
    }
}

void PumpBleComm::onWrite(uint8_t err, struct bt_gatt_write_params *params)
{
    // Handle the write event
    if (err)
    {
        LOG_ERR("Write failed with error: %d", err);
    }
    else
    {
        LOG_DBG("Write successful");
        LOG_HEXDUMP_DBG(params->data, params->length, "Data: ");
    }
}

// BLE CALLBACKS

void PumpBleComm::onDisconnected(struct bt_conn *conn, uint8_t reason)
{
    // Handle the disconnected event
    LOG_DBG("Disconnected from the pump");

    // Try to reconnect
    submitTask(mConnectTask, K_SECONDS(5));
}

void PumpBleComm::onSecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    // Handle the security changed event
    LOG_DBG("Security changed");
}

uint8_t PumpBleComm::onDiscover(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                struct bt_gatt_discover_params *params)
{
    // Handle the service discovered event
    if (!attr)
    {
        LOG_DBG("Discover stopped unexpectedly");
        mDiscoverParams = {};
        mDiscoveryState = DISCOVERY_MT_SERVICE;
        // Disconnect
        BLEComm::disconnect(&mConnection);
        return BT_GATT_ITER_STOP;
    }

    if (mDiscoveryState == DISCOVERY_MT_SERVICE && bt_uuid_cmp(mDiscoverParams.uuid, &BT_UUID_MT_SERVICE.uuid) == 0)
    {
        // Found the service
        LOG_DBG("Service found");
        // Discover the characteristics
        mDiscoverParams.uuid = &BT_UUID_MT_READ.uuid;
        mDiscoverParams.start_handle = attr->handle + 1;
        mDiscoverParams.type = BT_GATT_DISCOVER_CHARACTERISTIC;
        int err = BLEComm::discover(&mConnection, &mDiscoverParams);
        if (err)
        {
            LOG_ERR("Discover failed (err %d)", err);
        }
        mDiscoveryState = DISCOVERY_MT_READ;
    }
    else if (mDiscoveryState == DISCOVERY_MT_READ && bt_uuid_cmp(mDiscoverParams.uuid, &BT_UUID_MT_READ.uuid) == 0)
    {
        // Found the read characteristic
        LOG_DBG("Read characteristic found");
        // Discover the descriptor for the read characteristic
        mDiscoverParams.uuid = BT_UUID_GATT_CCC;
        mDiscoverParams.start_handle = attr->handle + 1;
        mDiscoverParams.type = BT_GATT_DISCOVER_DESCRIPTOR;
        mSubscribeReadParams.value_handle = bt_gatt_attr_value_handle(attr);
        int err = BLEComm::discover(&mConnection, &mDiscoverParams);
        if (err)
        {
            LOG_ERR("Discover failed (err %d)", err);
        }
        mDiscoveryState = DISCOVERY_MT_READ_CCC;
    }
    else if (mDiscoveryState == DISCOVERY_MT_READ_CCC && bt_uuid_cmp(mDiscoverParams.uuid, BT_UUID_GATT_CCC) == 0)
    {
        // Found the CCC descriptor
        LOG_DBG("CCC descriptor found");
        // Subscribe to the read characteristic
        mSubscribeReadParams.ccc_handle = attr->handle;
        mSubscribeReadParams.value = BT_GATT_CCC_NOTIFY;
        int err = BLEComm::subscribe(&mConnection, &mSubscribeReadParams);
        if (err && err != -EALREADY)
        {
            LOG_ERR("Subscribe failed (err %d)", err);
            return BT_GATT_ITER_STOP;
        }
        else
        {
            LOG_DBG("Subscribed to the read characteristic");
        }

        mDiscoverParams.uuid = &BT_UUID_MT_WRITE.uuid;
        mDiscoverParams.start_handle = attr->handle + 1;
        mDiscoverParams.type = BT_GATT_DISCOVER_CHARACTERISTIC;
        err = BLEComm::discover(&mConnection, &mDiscoverParams);
        if (err)
        {
            LOG_ERR("Discover failed (err %d)", err);
        }
        mDiscoveryState = DISCOVERY_MT_WRITE;
    }
    else if (mDiscoveryState == DISCOVERY_MT_WRITE && bt_uuid_cmp(mDiscoverParams.uuid, &BT_UUID_MT_WRITE.uuid) == 0)
    {
        // Found the write characteristic
        LOG_DBG("Write characteristic found");
        // Discover the descriptor for the write characteristic
        mDiscoverParams.uuid = BT_UUID_GATT_CCC;
        mDiscoverParams.start_handle = attr->handle + 1;
        mDiscoverParams.type = BT_GATT_DISCOVER_DESCRIPTOR;
        mSubscribeWriteParams.value_handle = bt_gatt_attr_value_handle(attr);
        int err = BLEComm::discover(&mConnection, &mDiscoverParams);
        if (err)
        {
            LOG_ERR("Discover failed (err %d)", err);
        }
        mDiscoveryState = DISCOVERY_MT_WRITE_CCC;
    }
    else if (mDiscoveryState == DISCOVERY_MT_WRITE_CCC && bt_uuid_cmp(mDiscoverParams.uuid, BT_UUID_GATT_CCC) == 0)
    {
        // Found the CCC descriptor
        LOG_DBG("CCC descriptor found");
        // Subscribe to the write characteristic
        mSubscribeWriteParams.ccc_handle = attr->handle;
        mSubscribeWriteParams.value = BT_GATT_CCC_INDICATE;
        int err = BLEComm::subscribe(&mConnection, &mSubscribeWriteParams);
        if (err && err != -EALREADY)
        {
            LOG_ERR("Subscribe failed (err %d)", err);
            return BT_GATT_ITER_STOP;
        }
        else
        {
            LOG_DBG("Subscribed to the write characteristic");
        }

        LOG_DBG("Discovery done");
        mDiscoveryState = DISCOVERY_DONE;
        submitTask(mOnDiscoveredTask);
    }

    return BT_GATT_ITER_STOP;
}

uint8_t PumpBleComm::onGattChanged(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data,
                                   uint16_t length)
{
    if (!data)
    {
        LOG_ERR("Data is null, unsubscribed");
        return BT_GATT_ITER_STOP;
    }
    LOG_HEXDUMP_INF(data, length, "On GATT changed with data: ");

    // Handle the GATT changed event

    return BT_GATT_ITER_CONTINUE;
}

// END BLECALLBACKS
