#include "bt_ids.h"
#include <control/bt_ids/InsulinDeliveryDevice.h>

#include <zephyr/sys/byteorder.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insulin_delivery_device);

namespace
{
    const bt_gatt_service_static idsService = bt_ids::getService();

    static constexpr uint8_t IDD_STATUS_CHANGED_IDX = 1;
    static constexpr uint8_t IDD_STATUS_IDX = 4;
}

InsulinDeliveryDevice::InsulinDeliveryDevice() {}

InsulinDeliveryDevice::~InsulinDeliveryDevice() {}

void InsulinDeliveryDevice::init()
{
    LOG_DBG("Initializing IDS device");
    bt_ids::init(*this);

    auto attr = idsService.attrs[0];
    LOG_DBG("attr: %d", attr.handle);
}

void InsulinDeliveryDevice::insulinPumpStatusUpdated(const PumpStatusUpdated &status)
{
    LOG_DBG("Pump status updated with values: therapyControlState=%u, operationalState=%u, reservoirLevel=%d, "
            "reservoirAttached=%d",
            static_cast<uint8_t>(status.therapyControlState), static_cast<uint8_t>(status.operationalState),
            status.reservoirLevel.value(), status.reservoirAttached);

    // Update status characteristic and indicate it
    mStatusCharData.therapyControlState = static_cast<uint8_t>(status.therapyControlState);
    mStatusCharData.operationalState = static_cast<uint8_t>(status.operationalState);
    sys_put_le16(status.reservoirLevel.value(), mStatusCharData.reservoirLevel);
    // reservoir attached is the only flag so far, and will be the only one we will support most likely in future
    // versions
    mStatusCharData.flags = status.reservoirAttached ? 0x01 : 0x00;

    static struct bt_gatt_indicate_params indicateStatusParams;
    indicateStatusParams.attr = &idsService.attrs[IDD_STATUS_IDX];
    indicateStatusParams.data = &mStatusCharData;
    indicateStatusParams.len = sizeof(mStatusCharData);

    int err = bt_gatt_indicate(NULL, &indicateStatusParams);
    if (err)
    {
        LOG_WRN("Indicate status failed (err %d)", err);
    }

    // Update status changed flags and indicate it
    uint16_t flagsStatusChanged = sys_get_le16(mStatusChangedCharData.flags);
    flagsStatusChanged |= static_cast<uint16_t>(StatusChangedFlags::THERAPY_CONTROL_STATE_CHANGED);
    flagsStatusChanged |= static_cast<uint16_t>(StatusChangedFlags::OPERATIONAL_STATE_CHANGED);
    flagsStatusChanged |= static_cast<uint16_t>(StatusChangedFlags::RESERVOIR_CHANGED);

    sys_put_le16(flagsStatusChanged, mStatusChangedCharData.flags);

    static struct bt_gatt_indicate_params indicateStatusChangedParams;
    indicateStatusChangedParams.attr = &idsService.attrs[IDD_STATUS_CHANGED_IDX];
    indicateStatusChangedParams.data = &mStatusChangedCharData;
    indicateStatusChangedParams.len = sizeof(mStatusChangedCharData);

    err = bt_gatt_indicate(NULL, &indicateStatusChangedParams);
    if (err)
    {
        LOG_WRN("Indicate status changed failed (err %d)", err);
    }
}

ssize_t InsulinDeliveryDevice::onReadIddStatusChanged(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                                      uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &mStatusChangedCharData, sizeof(mStatusChangedCharData));
}

ssize_t InsulinDeliveryDevice::onReadIddStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                               uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &mStatusCharData, sizeof(mStatusCharData));
}

ssize_t InsulinDeliveryDevice::onReadIddAnnunciationStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                           void *buf, uint16_t len, uint16_t offset)
{
    // TODO: Implement the function
    return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);
}

ssize_t InsulinDeliveryDevice::onReadIddFeatures(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                                 uint16_t len, uint16_t offset)
{
    // TODO: Implement the function
    return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);
}

ssize_t InsulinDeliveryDevice::onWriteIddStatusReaderControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                                  const void *buf, uint16_t len, uint16_t offset,
                                                                  uint8_t flags)
{
    // TODO: Implement the function
    return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);
}

ssize_t InsulinDeliveryDevice::onWriteCommandControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                          const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    // TODO: Implement the function
    return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);
}

ssize_t InsulinDeliveryDevice::onWriteIddRecordAccessControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                                  const void *buf, uint16_t len, uint16_t offset,
                                                                  uint8_t flags)
{
    // TODO: Implement the function
    return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);
}
