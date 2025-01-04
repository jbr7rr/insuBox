#ifndef INSULIN_DELIVERY_DEVICE_H
#define INSULIN_DELIVERY_DEVICE_H

#include <pump/PumpService.h> // PumpStatusUpdated
#include <zephyr/bluetooth/gatt.h>

class IInsulinDeliveryDevice
{
public:
    virtual ~IInsulinDeliveryDevice() = default;
    virtual void init() = 0;

    virtual void iddStatusUpdated(const PumpStatusUpdated &status) = 0;
    virtual void iddAnnunciationStatusUpdated(const AnnunciationType &annunciation, bool cancel = false) = 0;
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

    void iddStatusUpdated(const PumpStatusUpdated &status) override;
    void iddAnnunciationStatusUpdated(const AnnunciationType &annunciation, bool cancel = false) override;

protected:
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

private:
    struct IddStatusChangedChar
    {
        uint8_t flags[2]; // LE 16 flags
    };

    struct IddStatusChar
    {
        uint8_t therapyControlState;
        uint8_t operationalState;
        uint8_t reservoirLevel[2]; // LE 16 sfloat reservoir level
        uint8_t flags;
    };

    struct IddAnnunciationChar
    {
        uint8_t flags;
        uint16_t id;
        AnnunciationType type;
        AnnunciationStatus status;
        uint8_t aux[10];
    };

    IddStatusChangedChar mStatusChangedCharData = {};
    IddStatusChar mStatusCharData = {};
    IddAnnunciationChar mAnnunciationCharData = {};
};

#endif // INSULIN_DELIVERY_DEVICE_H