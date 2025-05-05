#ifndef IPUMP_DEVICE_H
#define IPUMP_DEVICE_H

#include <ctime>
#include <pump/PumpServiceMessages.h>

class IPumpDeviceCallback
{
public:
    virtual void pumpStatusUpdated(const PumpStatusUpdated &status) = 0;
    virtual void onBolusProgressUpdate(const BolusProgressUpdate &update) = 0;
};

class IPumpDevice
{
public:
    virtual ~IPumpDevice() = default;
    virtual void init() = 0;

    virtual void onBolusRequest(float amount, time_t timestamp) = 0;
    virtual void onStopBolus() = 0;
};

#endif // IPUMP_DEVICE_H
