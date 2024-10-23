#ifndef VIRTUAL_HMI_DEVICE_H
#define VIRTUAL_HMI_DEVICE_H

#include "IHmiDevice.h"

class VirtualHmiDevice : public IHmiDevice
{
public:
    VirtualHmiDevice();
    ~VirtualHmiDevice() override;
    void init() override;
};

#endif // VIRTUAL_HMI_DEVICE_H
