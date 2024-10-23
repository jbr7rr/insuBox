#ifndef IHMI_DEVICE_H
#define IHMI_DEVICE_H

class IHmiDevice
{
public:
    virtual ~IHmiDevice() = default;
    virtual void init() = 0;
};

#endif // IHMI_DEVICE_H
