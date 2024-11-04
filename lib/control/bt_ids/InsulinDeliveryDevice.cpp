#include "bt_ids.h"
#include <control/bt_ids/InsulinDeliveryDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insulin_delivery_device);

namespace
{
    const bt_gatt_service_static idsService = bt_ids::getService();
}

InsulinDeliveryDevice::InsulinDeliveryDevice() {}

InsulinDeliveryDevice::~InsulinDeliveryDevice() {}

void InsulinDeliveryDevice::init()
{
    LOG_DBG("Initializing IDS service");
    bt_ids::init(*this);

    auto attr = idsService.attrs[0];
    LOG_DBG("attr: %d", attr.handle);
}

ssize_t InsulinDeliveryDevice::onReadIddStatusChanged(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                                      uint16_t len, uint16_t offset)
{
    // TODO: Implement the function
    return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);
}

ssize_t InsulinDeliveryDevice::onReadIddStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                               uint16_t len, uint16_t offset)
{
    // TODO: Implement the function
    return BT_GATT_ERR(BT_ATT_ERR_NOT_SUPPORTED);
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
