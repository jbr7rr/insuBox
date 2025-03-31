#ifndef INSUBOX_HMI_DEVICE_H
#define INSUBOX_HMI_DEVICE_H

#include <hmi/HmiService.h>
#include <hmi/IHmiDevice.h>

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

class InsuBoxHmiDevice : public IHmiDevice
{
public:
    InsuBoxHmiDevice(IHmiCallback &hmiCallback);
    ~InsuBoxHmiDevice() override;
    void init() override;

    void onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey) override;

private:
    IHmiCallback &mHmiCallback;

    const struct device *mDisplayDevice;
    struct k_work_delayable mDisplayUpdateTask;

    void updateDisplay();
};

#endif // INSUBOX_HMI_DEVICE_H
