#include "../bt_cts/bt_cts.h"
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
    bt_cts::init();
    bt_ids::init(*this);

    auto attr = idsService.attrs[0];
    LOG_DBG("attr: %d", attr.handle);
}

void InsulinDeliveryDevice::onPumpStatusUpdated(const PumpStatus &status)
{
    uint16_t flagsStatusChanged = sys_get_le16(mStatusChangedCharData.flags);

    if (status.therapyControlState.has_value())
    {
        LOG_DBG("Therapy control state: %d", static_cast<uint8_t>(status.therapyControlState.value()));
        mStatusCharData.therapyControlState = static_cast<uint8_t>(status.therapyControlState.value());
        flagsStatusChanged |= static_cast<uint16_t>(StatusChangedFlags::THERAPY_CONTROL_STATE_CHANGED);
    }

    if (status.operationalState.has_value())
    {
        LOG_DBG("Operational state: %d", static_cast<uint8_t>(status.operationalState.value()));
        mStatusCharData.operationalState = static_cast<uint8_t>(status.operationalState.value());
        flagsStatusChanged |= static_cast<uint16_t>(StatusChangedFlags::OPERATIONAL_STATE_CHANGED);
    }

    if (status.reservoirLevel.has_value())
    {
        LOG_DBG("Reservoir level: %d", status.reservoirLevel.value().value());
        sys_put_le16(status.reservoirLevel.value().value(), mStatusCharData.reservoirLevel);
        flagsStatusChanged |= static_cast<uint16_t>(StatusChangedFlags::RESERVOIR_CHANGED);
    }

    if (status.reservoirAttached.has_value())
    {
        LOG_DBG("Reservoir attached: %d", status.reservoirAttached.value());
        mStatusCharData.flags = status.reservoirAttached.value() ? 0x01 : 0x00;
        flagsStatusChanged |= static_cast<uint16_t>(StatusChangedFlags::RESERVOIR_CHANGED);
    }

    static struct bt_gatt_indicate_params indicateStatusParams;
    indicateStatusParams.attr = &idsService.attrs[IDD_STATUS_IDX];
    indicateStatusParams.data = &mStatusCharData;
    indicateStatusParams.len = sizeof(mStatusCharData);

    int err = bt_gatt_indicate(NULL, &indicateStatusParams);
    if (err)
    {
        LOG_WRN("Indicate status failed (err %d)", err);
    }

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

void InsulinDeliveryDevice::onAlarmStatusUpdated(const AnnunciationType &annunciation, bool cancel)
{
    // TODO
    ;
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
