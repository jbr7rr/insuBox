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
    void onBtBluetoothStateChanged(struct bt_conn *conn, BtState state) override;

private:
    struct DisplayUpdateTask
    {
        InsuBoxHmiDevice *device;
        struct k_work_delayable work;
    };
    DisplayUpdateTask mDisplayUpdateTask;

    struct PairingScreenEntry {
        bt_conn *conn = nullptr;
        lv_obj_t *screen = nullptr;
    };
    static constexpr int MAX_PAIRING_SCREENS = 1; // Only one suppoted for now
    PairingScreenEntry mPairingScreens[MAX_PAIRING_SCREENS];

    IHmiCallback &mHmiCallback;
    k_work_q &mWorkQueue;

    const struct device *mDisplayDevice;
    const struct device *mKeypadDevice;

    void showMainScreen();

    bool storePairingScreen(bt_conn *conn, lv_obj_t *screen);
    void removePairingScreen(bt_conn *conn);
};

#endif // INSUBOX_HMI_DEVICE_H
