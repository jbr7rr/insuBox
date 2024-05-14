#ifndef PUMP_BLE_COMM_H
#define PUMP_BLE_COMM_H

#include <ble/BLEComm.h>

#include <optional>
#include <string>

class IScanCallback
{
public:
    struct ManufacturerData
    {
        uint16_t companyId;
        int32_t deviceSN;
        uint8_t deviceType;
        uint8_t version;
    };

    virtual void onDeviceFound(bt_addr_le_t addr, ManufacturerData manufacturerData) = 0;
};

class PumpBleComm : public IScanCallback, public IBLECallback
{
public:
    PumpBleComm();
    ~PumpBleComm();
    void init();
    void connect();

    void onDeviceFound(bt_addr_le_t addr, ManufacturerData manufacturerData) override;

    void onConnected(struct bt_conn *conn, uint8_t err) override;
    void onDisconnected(struct bt_conn *conn, uint8_t reason) override;
    void onSecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) override;

    uint8_t onDiscover(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                       struct bt_gatt_discover_params *params) override;
    uint8_t onGattChanged(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data,
                          uint16_t length) override;

private:
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

    std::optional<std::int32_t> mDeviceSN = std::nullopt;
    std::optional<bt_addr_le_t> mDeviceAddr = std::nullopt;

    BleConnection mConnection;
    struct bt_gatt_discover_params mDiscoverParams;
    struct bt_gatt_subscribe_params mSubscribeParams;
};

#endif // PUMP_BLE_COMM_H