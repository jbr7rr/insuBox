#ifndef INSUBOX_DEVICE_H
#define INSUBOX_DEVICE_H

#ifdef CONFIG_IB_PUMP_INSUBOX

#include <pump/IPumpDevice.h>
#include <zephyr/kernel.h>

class InsuBoxDevice : public IPumpDevice
{
public:
    InsuBoxDevice();
    ~InsuBoxDevice();
    void init() override;
    void onBolusRequest(float amount, time_t timestamp) override;
    void onStopBolus() override;

private:
    struct SubContainer
    {
        InsuBoxDevice *mDevice;
        k_work_delayable sensorWork;
    } mSubContainer;

    void sensorWork();
};

#endif // CONFIG_IB_PUMP_INSUBOX
#endif // INSUBOX_DEVICE_H
