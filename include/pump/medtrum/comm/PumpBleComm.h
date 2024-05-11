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

    virtual void onDeviceFound(bt_addr_le_t addr,
                               ManufacturerData manufacturerData) = 0;
};

class PumpBleComm : public IScanCallback, public IBLECallback
{
public:
    PumpBleComm();
    ~PumpBleComm();
    void init();
    void connect();

    void onDeviceFound(bt_addr_le_t addr,
                       ManufacturerData manufacturerData) override;

    void onConnected(struct bt_conn *conn, uint8_t err) override;
    void onDisconnected(struct bt_conn *conn, uint8_t reason) override;
    void onSecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) override;

private:
    std::optional<std::int32_t> mDeviceSN = std::nullopt;
    std::optional<bt_addr_le_t> mDeviceAddr = std::nullopt;

    BleConnection mConnection;
};

#endif // PUMP_BLE_COMM_H