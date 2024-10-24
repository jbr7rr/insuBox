#ifndef PUMP_BLE_COMM_H
#define PUMP_BLE_COMM_H

#include <ble/BLEComm.h>
#include <pump/medtrum_bt/comm/WriteCommandPackets.h>

#include <optional>
#include <string>

class PumpBleCallback
{
public:
    virtual void onReadyForCommands() = 0;
    virtual void onDisconnected() = 0;
    virtual void onWriteError() = 0;
    virtual void onCommandResponse(uint8_t *data, size_t length) = 0;
    virtual void onNotification(uint8_t *data, size_t length) = 0;
};

class IScanCallback
{
public:
    struct ManufacturerData
    {
        uint16_t companyId;
        uint32_t deviceSN;
        uint8_t deviceType;
        uint8_t version;
    };

    virtual void onDeviceFound(bt_addr_le_t addr, ManufacturerData manufacturerData) = 0;
};

class PumpBleComm : public IScanCallback, public IBLECallback
{
public:
    PumpBleComm(PumpBleCallback &callback);
    ~PumpBleComm();
    void init();

    /**
     * @brief Create a connection to the pump
     *
     * @param deviceSN Serial number of the pump
     *
     */
    void connect(uint32_t deviceSN);

    /**
     * @brief Write a command to the pump
     *
     * @param data Pointer to the data to write, needs to be valid until the write is complete
     * @param length Length of the data to write
     *
     * @return True if the command was was dispatched, false otherwise
     *
     */
    bool writeCommand(uint8_t *data, size_t length);

    void onDeviceFound(bt_addr_le_t addr, ManufacturerData manufacturerData) override;
    void onConnected(struct bt_conn *conn, uint8_t err) override;
    void onDisconnected(struct bt_conn *conn, uint8_t reason) override;
    void onSecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) override;

    uint8_t onDiscover(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                       struct bt_gatt_discover_params *params) override;
    uint8_t onGattChanged(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data,
                          uint16_t length) override;

private:
    struct SubContainer
    {
        PumpBleComm *pumpBleComm;
        // Tasks
        k_work_delayable connectWork;
        k_work_delayable writeWork;
        // Params where we need to put a callback in
        struct bt_gatt_write_params writeParams;
    } mSubContainer;

    enum DiscoveryState
    {
        DISCOVERY_MT_SERVICE,
        DISCOVERY_MT_READ,
        DISCOVERY_MT_READ_CCC,
        DISCOVERY_MT_WRITE,
        DISCOVERY_MT_WRITE_CCC,
        DISCOVERY_DONE
    };

    DiscoveryState mDiscoveryState = DISCOVERY_MT_SERVICE;
    struct bt_gatt_discover_params mDiscoverParams;

    std::optional<std::uint32_t> mDeviceSN = std::nullopt;
    std::optional<bt_addr_le_t> mDeviceAddr = std::nullopt;

    BleConnection mConnection;
    PumpBleCallback &mCallback;

    // Read stuff
    struct bt_gatt_subscribe_params mSubscribeReadParams;

    // Write stuff
    struct bt_gatt_subscribe_params mSubscribeWriteParams;
    std::optional<WriteCommandPackets> mWriteCommandPackets = std::nullopt;
    int mSequenceNumber = 0;
    uint8_t mWriteDataBuffer[WriteCommandPackets::PACKET_SIZE];

    // Task stuff
    k_work_q mWorkQueue;
    K_KERNEL_STACK_MEMBER(mWorkQueueBuffer, KB(2));
    void submitWork(k_work_delayable &task, k_timeout_t delay = K_MSEC(100));
    // Task related functions
    void _connect();
    void _write();

    // Direct callbacks from BLE stack
    void onWrite(uint8_t err, struct bt_gatt_write_params *params);
};

#endif // PUMP_BLE_COMM_H
