#include "bt_cts.h"
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/timeutil.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_bt_cts);

namespace
{
    static uint8_t ct[10];
}

namespace bt_cts
{
    void init()
    {
        LOG_DBG("Initializing CTS service");
        // set ct to 0
        memset(ct, 0, sizeof(ct));
    }

    static ssize_t readCurrentTime(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                   uint16_t offset)
    {
        // get time
        struct timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);
        struct tm timeinfo = *gmtime(&currentTime.tv_sec);

        uint16_t year = sys_cpu_to_le16(timeinfo.tm_year + 1900);
        memcpy(ct, &year, sizeof(year));

        ct[2] = timeinfo.tm_mon + 1;
        ct[3] = timeinfo.tm_mday;
        ct[4] = timeinfo.tm_hour;
        ct[5] = timeinfo.tm_min;
        ct[6] = timeinfo.tm_sec;
        // day of the week
        ct[7] = timeinfo.tm_wday;

        // Fractions 256 part of 'Exact Time 256
        ct[8] = 0;

        return bt_gatt_attr_read(conn, attr, buf, len, offset, ct, sizeof(ct));
    }

    static ssize_t writeCurrentTime(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                                    uint16_t len, uint16_t offset, uint8_t flags)
    {
        if (!buf || len != sizeof(ct))
        {
            LOG_ERR("Invalid attribute length");
            return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
        }

        memcpy(ct, buf, len);

        // Update the system time
        struct tm timeinfo;
        timeinfo.tm_year = sys_get_le16(&ct[0]) - 1900;
        timeinfo.tm_mon = ct[2] - 1;
        timeinfo.tm_mday = ct[3];
        timeinfo.tm_hour = ct[4];
        timeinfo.tm_min = ct[5];
        timeinfo.tm_sec = ct[6];
        time_t time = mktime(&timeinfo);
        struct timespec currentTime;
        currentTime.tv_sec = time;
        currentTime.tv_nsec = 0;
        clock_settime(CLOCK_REALTIME, &currentTime);

        return len;
    }

    static ssize_t readLocalTimeInfo(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                     uint16_t offset)
    {
        // Always UTC for now
        // TODO: Arrange local time info
        // GATT layout: see https://btprodspecificationrefs.blob.core.windows.net/gatt-specification-supplement/GATT_Specification_Supplement.pdf#3.136.3

        uint16_t timeOffset = 0;
        return bt_gatt_attr_read(conn, attr, buf, len, offset, &timeOffset, sizeof(timeOffset));
    }

    BT_GATT_SERVICE_DEFINE(bt_cts, BT_GATT_PRIMARY_SERVICE(BT_UUID_CTS),
                           BT_GATT_CHARACTERISTIC(BT_UUID_CTS_CURRENT_TIME,
                                                  BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE | BT_GATT_CHRC_NOTIFY,
                                                  BT_GATT_PERM_READ | BT_GATT_PERM_WRITE_AUTHEN, readCurrentTime,
                                                  writeCurrentTime, NULL),
                           BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
                           BT_GATT_CHARACTERISTIC(BT_UUID_CTS_LOCAL_TIME_INFO, BT_GATT_CHRC_READ, BT_GATT_PERM_READ,
                                                  readLocalTimeInfo, NULL, NULL));

} // namespace bt_cts
