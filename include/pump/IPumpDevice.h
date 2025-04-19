#ifndef IPUMP_DEVICE_H
#define IPUMP_DEVICE_H

#include <ctime>

class IPumpDevice
{
public:
    virtual ~IPumpDevice() = default;
    virtual void init() = 0;

    virtual void onBolusRequest(float amount, time_t timestamp) = 0;
    virtual void onStopBolus() = 0;
};

#endif // IPUMP_DEVICE_H
