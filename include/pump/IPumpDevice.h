#ifndef IPUMP_DEVICE_H
#define IPUMP_DEVICE_H

class IPumpDevice
{
public:
    virtual ~IPumpDevice() = default;
    virtual void init() = 0;
};

#endif // IPUMP_DEVICE_H
