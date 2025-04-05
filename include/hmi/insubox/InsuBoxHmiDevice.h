#ifndef INSUBOX_HMI_DEVICE_H
#define INSUBOX_HMI_DEVICE_H

#include <hmi/HmiService.h>
#include <hmi/IHmiDevice.h>
#include <lvgl.h>

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

class InsuBoxHmiDevice : public IHmiDevice
{
public:
    InsuBoxHmiDevice(IHmiCallback &hmiCallback, k_work_q &workQueue);
    ~InsuBoxHmiDevice() override;
    void init() override;

    void onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey) override;

private:
    struct DisplayUpdateTask
    {
        InsuBoxHmiDevice *device;
        struct k_work_delayable work;
    };
    DisplayUpdateTask mDisplayUpdateTask;

    IHmiCallback &mHmiCallback;
    k_work_q &mWorkQueue;

    const struct device *mDisplayDevice;
    const struct device *mKeypadDevice;

    void showMainScreen();
};

#endif // INSUBOX_HMI_DEVICE_H
