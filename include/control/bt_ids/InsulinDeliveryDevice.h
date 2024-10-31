#ifndef INSULIN_DELIVERY_DEVICE_H
#define INSULIN_DELIVERY_DEVICE_H

#include <zephyr/bluetooth/gatt.h>

class IInsulinDeliveryDevice
{
public:
    virtual ~IInsulinDeliveryDevice() = default;
    virtual void init() = 0;
};

class IInsulinDeliveryDeviceCallback
{
public:
    virtual ssize_t onReadIddStatusChanged(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                           uint16_t len, uint16_t offset) = 0;
    virtual ssize_t onReadIddStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                    uint16_t offset) = 0;
    virtual ssize_t onReadIddAnnunciationStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf,
                                                uint16_t len, uint16_t offset) = 0;
    virtual ssize_t onReadIddFeatures(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                      uint16_t offset) = 0;
    virtual ssize_t onWriteIddStatusReaderControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                       const void *buf, uint16_t len, uint16_t offset,
                                                       uint8_t flags) = 0;
    virtual ssize_t onWriteCommandControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                                               uint16_t len, uint16_t offset, uint8_t flags) = 0;
    virtual ssize_t onWriteIddRecordAccessControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                                       const void *buf, uint16_t len, uint16_t offset,
                                                       uint8_t flags) = 0;
};

class InsulinDeliveryDevice : public IInsulinDeliveryDevice, public IInsulinDeliveryDeviceCallback
{
public:
    InsulinDeliveryDevice();
    ~InsulinDeliveryDevice();
    void init();

    ssize_t onReadIddStatusChanged(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                   uint16_t offset) override;
    ssize_t onReadIddStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                            uint16_t offset) override;
    ssize_t onReadIddAnnunciationStatus(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                                        uint16_t offset) override;
    ssize_t onReadIddFeatures(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len,
                              uint16_t offset) override;
    ssize_t onWriteIddStatusReaderControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                                               uint16_t len, uint16_t offset, uint8_t flags) override;
    ssize_t onWriteCommandControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                                       uint16_t len, uint16_t offset, uint8_t flags) override;
    ssize_t onWriteIddRecordAccessControlPoint(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf,
                                               uint16_t len, uint16_t offset, uint8_t flags) override;
};

#endif // INSULIN_DELIVERY_DEVICE_H