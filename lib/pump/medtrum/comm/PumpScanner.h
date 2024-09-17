#ifndef PUMP_SCANNER_H
#define PUMP_SCANNER_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#include <optional>
#include <string>

#include <pump/medtrum/comm/PumpBleComm.h>

class PumpScanner
{
public:
    PumpScanner() = default;
    ~PumpScanner() = default;

    struct scanRequest
    {
        IScanCallback *instance;
        uint32_t targetDeviceSN;
    };

    static bool startScan(scanRequest request);

private:
    static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *buf);
    static bool data_cb(struct bt_data *data, void *user_data);
    static scanRequest mScanRequest;
};

#endif // PUMP_SCANNER_H
