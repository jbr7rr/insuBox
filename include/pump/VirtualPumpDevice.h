#ifndef VIRTUAL_PUMP_DEVICE_H
#define VIRTUAL_PUMP_DEVICE_H

#ifdef CONFIG_IB_PUMP_VIRTUAL

#include <pump/IPumpDevice.h>

class VirtualPumpDevice : public IPumpDevice
{
public:
    VirtualPumpDevice();
    ~VirtualPumpDevice();
    void init() override;
};

#endif // CONFIG_IB_VIRTUAL_PUMP
#endif // VIRTUAL_PUMP_DEVICE_H
