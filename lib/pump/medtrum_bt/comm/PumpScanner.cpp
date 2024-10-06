#include "PumpScanner.h"
#include <pump/medtrum_bt/comm/PumpBleComm.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ib_medtrum_pump_scanner);

PumpScanner::scanRequest PumpScanner::mScanRequest = {};

// TODO: When creating a connection we have to disable the scanner, so if we want multiple connections we need to handle
// this, maybe setup a general scanner in BLEComm ?

namespace
{
    constexpr uint16_t MANUFACTURER_ID = 18305;   // Medtrum manufacturer ID
    constexpr uint8_t BOOTLOADER_DEVICE_TYPE = 1; // Bootloader device type
}

bool PumpScanner::startScan(scanRequest request)
{
    // TODO: This might need a mutex, and a queue or something to handle multiple scan requests
    if (mScanRequest.instance != nullptr)
    {
        // Wait for current scan to finish
        return false;
    }

    // Save the scan request
    mScanRequest = request;

    // Start scanning for the device
    const struct bt_le_scan_param scan_param = {
        .type = BT_HCI_LE_SCAN_PASSIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = 0x0010,
        .window = 0x0010,
    };
    int err = bt_le_scan_start(&scan_param, scan_cb);
    if (err)
    {
        LOG_ERR("Starting scanning failed (err %d)", err);
    }
    else
    {
        LOG_DBG("Scanning started");
    }
    return true;
}

void PumpScanner::scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *buf)
{
    // Extract manufacturer data
    IScanCallback::ManufacturerData mfgData = {};
    bt_data_parse(buf, data_cb, &mfgData);

    // Check if the manufacturer data is valid
    if (mfgData.companyId == MANUFACTURER_ID)
    {
        // Found Medtrum manufacturer data
        LOG_DBG("Found Medtrum device");
        LOG_DBG("Device SN: %02X %02X %02X %02X", (mfgData.deviceSN >> 24) & 0xFF, (mfgData.deviceSN >> 16) & 0xFF,
                (mfgData.deviceSN >> 8) & 0xFF, mfgData.deviceSN & 0xFF);
        LOG_DBG("Device Type: %d", mfgData.deviceType);
        LOG_DBG("Version: %d", mfgData.version);

        // Check if the device is the target device, and filter out bootloader device
        if (mScanRequest.instance != nullptr && mScanRequest.targetDeviceSN == mfgData.deviceSN &&
            mfgData.deviceType != BOOTLOADER_DEVICE_TYPE)
        {
            bt_le_scan_stop();

            // Found the target device
            mScanRequest.instance->onDeviceFound(*addr, mfgData);

            // Stop scanning
            mScanRequest.instance = nullptr;
        }
    }
    else
    {
        // LOG_DBG("Not a Medtrum device");
    }
}

bool PumpScanner::data_cb(struct bt_data *data, void *user_data)
{
    IScanCallback::ManufacturerData mfgData = {};
    struct net_buf_simple btData;

    switch (data->type)
    {
    case BT_DATA_MANUFACTURER_DATA:
        if (data->data_len < 2)
        {
            return false;
        }

        net_buf_simple_init_with_data(&btData, (void *)data->data, data->data_len);
        mfgData.companyId = net_buf_simple_pull_le16(&btData);
        if (mfgData.companyId == MANUFACTURER_ID)
        {
            // Found Medtrum manufacturer data
            mfgData.deviceSN = net_buf_simple_pull_le32(&btData);
            mfgData.deviceType = net_buf_simple_pull_u8(&btData);
            mfgData.version = net_buf_simple_pull_u8(&btData);
            if (user_data != NULL)
            {
                // Copy the manufacturer data to the user data
                memcpy(user_data, &mfgData, sizeof(mfgData));
            }
        }
        return true;
    default:
        return true;
    }

    return false;
}
