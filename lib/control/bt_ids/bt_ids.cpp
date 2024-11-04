#include "bt_ids.h"
#include <zephyr/bluetooth/gatt.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_bt_ids);

#define BT_UUID_IDS_IDD_STATUS_CHANGED_VAL 0x2b20
#define BT_UUID_IDS_IDD_STATUS_CHANGED BT_UUID_DECLARE_16(BT_UUID_IDS_IDD_STATUS_CHANGED_VAL)

#define BT_UUID_IDS_IDD_STATUS_VAL 0x2b21
#define BT_UUID_IDS_IDD_STATUS BT_UUID_DECLARE_16(BT_UUID_IDS_IDD_STATUS_VAL)

#define BT_UUID_IDS_IDD_ANNUNCIATION_STATUS_VAL 0x2b22
#define BT_UUID_IDS_IDD_ANNUNCIATION_STATUS BT_UUID_DECLARE_16(BT_UUID_IDS_IDD_ANNUNCIATION_STATUS_VAL)

#define BT_UUID_IDS_IDD_FEATURES_VAL 0x2b23
#define BT_UUID_IDS_IDD_FEATURES BT_UUID_DECLARE_16(BT_UUID_IDS_IDD_FEATURES_VAL)

#define BT_UUID_IDS_IDD_STATUS_READER_CONTROL_POINT_VAL 0x2b24
#define BT_UUID_IDS_IDD_STATUS_READER_CONTROL_POINT BT_UUID_DECLARE_16(BT_UUID_IDS_IDD_STATUS_READER_CONTROL_POINT_VAL)

#define BT_UUID_IDS_COMMAND_CONTROL_POINT_VAL 0x2b25
#define BT_UUID_IDS_COMMAND_CONTROL_POINT BT_UUID_DECLARE_16(BT_UUID_IDS_COMMAND_CONTROL_POINT_VAL)

#define BT_UUID_IDS_COMMAND_DATA_VAL 0x2b26
#define BT_UUID_IDS_COMMAND_DATA BT_UUID_DECLARE_16(BT_UUID_IDS_COMMAND_DATA_VAL)

#define BT_UUID_IDS_IDD_RECORD_ACCESS_CONTROL_POINT_VAL 0x2b27
#define BT_UUID_IDS_IDD_RECORD_ACCESS_CONTROL_POINT BT_UUID_DECLARE_16(BT_UUID_IDS_IDD_RECORD_ACCESS_CONTROL_POINT_VAL)

#define BT_UUID_IDS_IDD_HISTORY_DATA_VAL 0x2b28
#define BT_UUID_IDS_IDD_HISTORY_DATA BT_UUID_DECLARE_16(BT_UUID_IDS_IDD_HISTORY_DATA_VAL)

namespace bt_ids
{
    IInsulinDeliveryDeviceCallback *mInsulinDeliveryDevice;
    void init(IInsulinDeliveryDeviceCallback &insulinDeliveryDevice)
    {
        mInsulinDeliveryDevice = &insulinDeliveryDevice;
        LOG_DBG("Initializing IDS service");
    }

    static ssize_t readIddStatusChanged(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                        uint16_t offset)
    {
        if (mInsulinDeliveryDevice)
        {
            return mInsulinDeliveryDevice->onReadIddStatusChanged(conn, attr, buf, len, offset);
        }
        else
        {
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }
    }

    static ssize_t readIddStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                 uint16_t offset)
    {
        if (mInsulinDeliveryDevice)
        {
            return mInsulinDeliveryDevice->onReadIddStatus(conn, attr, buf, len, offset);
        }
        else
        {
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }
    }

    static ssize_t readIddAnnunciationStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                             uint16_t len, uint16_t offset)
    {
        if (mInsulinDeliveryDevice)
        {
            return mInsulinDeliveryDevice->onReadIddAnnunciationStatus(conn, attr, buf, len, offset);
        }
        else
        {
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }
    }

    static ssize_t readIddFeatures(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                   uint16_t offset)
    {
        if (mInsulinDeliveryDevice)
        {
            return mInsulinDeliveryDevice->onReadIddFeatures(conn, attr, buf, len, offset);
        }
        else
        {
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }
    }

    static ssize_t writeIddStatusReaderControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                    const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
    {
        if (mInsulinDeliveryDevice)
        {
            return mInsulinDeliveryDevice->onWriteIddStatusReaderControlPoint(conn, attr, buf, len, offset, flags);
        }
        else
        {
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }
    }

    static ssize_t writeCommandControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                                            uint16_t len, uint16_t offset, uint8_t flags)
    {
        if (mInsulinDeliveryDevice)
        {
            return mInsulinDeliveryDevice->onWriteCommandControlPoint(conn, attr, buf, len, offset, flags);
        }
        else
        {
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }
    }

    static ssize_t writeIddRecordAccessControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                    const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
    {
        if (mInsulinDeliveryDevice)
        {
            return mInsulinDeliveryDevice->onWriteIddRecordAccessControlPoint(conn, attr, buf, len, offset, flags);
        }
        else
        {
            return BT_GATT_ERR(BT_ATT_ERR_UNLIKELY);
        }
    }

    BT_GATT_SERVICE_DEFINE(
        bt_ids, BT_GATT_PRIMARY_SERVICE(BT_UUID_IDS),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_IDD_STATUS_CHANGED, BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_READ_AUTHEN, readIddStatusChanged, NULL, NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_IDD_STATUS, BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_READ_AUTHEN, readIddStatus, NULL, NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_IDD_ANNUNCIATION_STATUS, BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_READ_AUTHEN, readIddAnnunciationStatus, NULL, NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_IDD_FEATURES, BT_GATT_CHRC_READ, BT_GATT_PERM_READ_AUTHEN, readIddFeatures,
                               NULL, NULL),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_IDD_STATUS_READER_CONTROL_POINT, BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_WRITE_AUTHEN, NULL, writeIddStatusReaderControlPoint, NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_COMMAND_CONTROL_POINT, BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_WRITE_AUTHEN, NULL, writeCommandControlPoint, NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_COMMAND_DATA, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_WRITE_AUTHEN, NULL, NULL,
                               NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_IDD_RECORD_ACCESS_CONTROL_POINT, BT_GATT_CHRC_WRITE | BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_WRITE_AUTHEN, NULL, writeIddRecordAccessControlPoint, NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN),
        BT_GATT_CHARACTERISTIC(BT_UUID_IDS_IDD_HISTORY_DATA, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ_AUTHEN, NULL, NULL,
                               NULL),
        BT_GATT_CCC(NULL, BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN));

    const bt_gatt_service_static &getService()
    {
        return bt_ids;
    }
}
