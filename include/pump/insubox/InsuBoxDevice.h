#ifndef INSUBOX_DEVICE_H
#define INSUBOX_DEVICE_H

#ifdef CONFIG_IB_PUMP_INSUBOX

#include <pump/IPumpDevice.h>
#include <pump/PumpServiceMessages.h>
#include <pump/insubox/motor/Motor.h>
#include <zephyr/settings/settings.h>
#include <zephyr/kernel.h>

class InsuBoxDevice : public IPumpDevice, public IMotorCallback
{
public:
    InsuBoxDevice(IPumpDeviceCallback &pumpDeviceCallback);
    ~InsuBoxDevice();

protected:
    void init() override;

    void onBolusRequest(float amount, time_t timestamp) override;
    void onStopBolus() override;
    void onRetractRequest() override;

    void onMotorCompleted(float delivered, float position, bool stopped, bool error) override;

private:
    struct SubContainer
    {
        InsuBoxDevice *mDevice;
        k_work_delayable sensorWork;
    } mSubContainer;

    struct BolusTask
    {
        InsuBoxDevice *device;
        k_work_delayable bolusWork;
        float requestedBolus;
        float deliveredBolus;
        time_t requestedTimestamp;
        bool completed;
    } mBolusTask;

    IPumpDeviceCallback &mPumpDeviceCallback;
    Motor &mMotor;

    void sensorWork();
    void bolusWork();

    void sendBolusProgressUpdate();

    int loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param);
    static Motor &createMotorInstance(IMotorCallback &callback);
};

#endif // CONFIG_IB_PUMP_INSUBOX
#endif // INSUBOX_DEVICE_H
