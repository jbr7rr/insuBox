#ifndef VIRTUAL_PUMP_DEVICE_H
#define VIRTUAL_PUMP_DEVICE_H

#ifdef CONFIG_IB_PUMP_VIRTUAL

#include <pump/IPumpDevice.h>
#include <pump/PumpService.h>
#include <zephyr/kernel.h>

class VirtualPumpDevice : public IPumpDevice
{
public:
    VirtualPumpDevice(IPumpDeviceCallback &pumpDeviceCallback);
    ~VirtualPumpDevice();
    void init() override;

    void onBolusRequest(float amount, time_t timestamp) override;
    void onStopBolus() override;

private:
    struct SubContainer
    {
        VirtualPumpDevice *pumpDevice;
        k_work_delayable statusWork;
        float requestedBolus;
        float deliveredBolus;
        time_t requestedTimestamp;
    };

    SubContainer mSubContainer;
    IPumpDeviceCallback &mPumpDeviceCallback;

    void _updateStatus();
};

#endif // CONFIG_IB_VIRTUAL_PUMP
#endif // VIRTUAL_PUMP_DEVICE_H
