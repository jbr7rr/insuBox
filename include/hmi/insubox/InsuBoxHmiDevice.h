#ifndef INSUBOX_HMI_DEVICE_H
#define INSUBOX_HMI_DEVICE_H

#ifdef CONFIG_IB_HMI_INSUBOX

#include <hmi/HmiService.h>
#include <hmi/IHmiDevice.h>
#include <lvgl.h>

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>

class Buzzer;

class InsuBoxHmiDevice : public IHmiDevice
{
public:
    InsuBoxHmiDevice(IHmiCallback &hmiCallback, k_work_q &workQueue);
    ~InsuBoxHmiDevice() override;
    void init() override;

    void onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey) override;
    void onBtBluetoothStateChanged(struct bt_conn *conn, BtState state) override;
    void onBolusProgressUpdate(BolusProgressUpdate &update) override;

private:
    struct DisplayUpdateTask
    {
        InsuBoxHmiDevice *device;
        struct k_work_delayable work;
    };
    DisplayUpdateTask mDisplayUpdateTask;

    struct PairingScreenEntry
    {
        InsuBoxHmiDevice *device;
        bt_conn *conn = nullptr;
        lv_obj_t *screen = nullptr;
    };
    static constexpr int MAX_PAIRING_SCREENS = 1; // Only one suppoted for now
    PairingScreenEntry mPairingScreens[MAX_PAIRING_SCREENS];
    bool mDisplayOn = true;

    struct BolusUiState
    {
        lv_obj_t *popup = nullptr;
        lv_obj_t *mainScreenLabel = nullptr;
        BolusProgressUpdate currentUpdate{};
    } mBolusUi;

    IHmiCallback &mHmiCallback;
    k_work_q &mWorkQueue;
    Buzzer &mBuzzer;

    const struct device *mDisplayDevice;
    const struct device *mKeypadDevice;

    void showMainScreen();
    void showBolusScreen();
    void createBolusProgressPopup();
    void createBolusProgressWidget();

    PairingScreenEntry *storePairingScreen(bt_conn *conn, lv_obj_t *screen);
    void removePairingScreen(bt_conn *conn);

    static Buzzer &createBuzzerInstance();
};

#endif // CONFIG_IB_HMI_INSUBOX
#endif // INSUBOX_HMI_DEVICE_H
