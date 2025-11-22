#ifndef INSUBOX_DEVICE_H
#define INSUBOX_DEVICE_H

#ifdef CONFIG_IB_PUMP_INSUBOX

#include <atomic>
#include <pump/IPumpDevice.h>
#include <pump/PumpServiceMessages.h>
#include <pump/insubox/motor/Motor.h>
#include <pump/insubox/posSensor/PosSensor.h>
#include <zephyr/kernel.h>

class InsuBoxDevice : public IPumpDevice, public IMotorCallback
{
public:
    InsuBoxDevice(IPumpDeviceCallback &pumpDeviceCallback);
    ~InsuBoxDevice();

protected:
    void init() override;

    void onBolusRequest(float amount, time_t timestamp) override;
    void onStopBolusRequest() override;
    void onRetractRequest() override;
    void onPrimeRequest() override;

    void onMotorCompleted(float delivered, float position, bool stopped, bool error) override;

private:
    enum State : uint8_t
    {
        IDLE,
        DELIVERING_BOLUS,
        RETRACTING,
        PRIMING,
        CAL_SENSOR,
    };
    std::atomic<State> mState = State::IDLE;
    struct SimpleTask
    {
        InsuBoxDevice *mDevice;
        k_work_delayable work;
    };
    SimpleTask mRetractTask;
    SimpleTask mCalSensorTask;
    SimpleTask mPrimeTask;
    struct BolusTask
    {
        InsuBoxDevice *device;
        k_work_delayable work;
        float requestedBolus;
        float deliveredBolus;
        time_t requestedTimestamp;
        std::atomic<bool> completed;
    } mBolusTask;

    IPumpDeviceCallback &mPumpDeviceCallback;
    Motor &mMotor;
    PosSensor &mPosSensor;

    float mMaxPlungerDifference = 0;

    void bolusTask();
    void retractTask();
    void calSensorTask();
    void primeTask();

    void sendBolusProgressUpdate();

    static Motor &createMotorInstance(IMotorCallback &callback);
    static PosSensor &createPosSensorInstance();
};

#endif // CONFIG_IB_PUMP_INSUBOX
#endif // INSUBOX_DEVICE_H
