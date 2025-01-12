#ifndef INSUBOX_DEVICE_H
#define INSUBOX_DEVICE_H

#ifdef CONFIG_IB_PUMP_INSUBOX

#include <pump/IPumpDevice.h>

class InsuBoxDevice : public IPumpDevice
{
public:
    InsuBoxDevice();
    ~InsuBoxDevice();
    void init() override;
};

#endif // CONFIG_IB_PUMP_INSUBOX
#endif // INSUBOX_DEVICE_H
