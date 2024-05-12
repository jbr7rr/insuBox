#include <pump/medtrum/comm/PumpBleComm.h>
#include <zephyr/bluetooth/gatt.h>

#include "PumpScanner.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ib_medtrum_ble);

PumpBleComm::PumpBleComm()
{
    mConnection.callback = this;
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
    // Connect to the pump
    if (mDeviceAddr.has_value())
    {
        // Connect to the device
        if (BLEComm::connect(mDeviceAddr.value(), &mConnection))
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
            .targetDeviceSN = static_cast<int32_t>(0xC29415AB),
        };
        PumpScanner::startScan(request);
    }
}

void PumpBleComm::onDeviceFound(bt_addr_le_t addr,
                                ManufacturerData manufacturerData)

{
    LOG_DBG("Pump found with SN: %d", manufacturerData.deviceSN);
    // Handle the found device
    // Save the device address
    mDeviceAddr = addr;
    // Connect to the device
    connect();
}

void PumpBleComm::onConnected(struct bt_conn *conn, uint8_t err)
{
    // Handle the connected event
    LOG_DBG("Connected to the pump");
    // Discover services
    if (BLEComm::discoverServices(&mConnection))
    {
        LOG_DBG("Discovering services");
    }
    else
    {
        LOG_ERR("Failed to discover services");
    }
}

void PumpBleComm::onDisconnected(struct bt_conn *conn, uint8_t reason)
{
    // Handle the disconnected event
    LOG_DBG("Disconnected from the pump");
}

void PumpBleComm::onSecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    // Handle the security changed event
    LOG_DBG("Security changed");
}

void PumpBleComm::onAttributeDiscovered(const struct bt_gatt_attr *attr)
{
    // Handle the service discovered event
    LOG_DBG("Attribute discovered with handle: %d", attr->handle);
}