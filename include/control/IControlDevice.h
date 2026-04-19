#ifndef ICONTROL_DEVICE_H
#define ICONTROL_DEVICE_H

#include <control/IdsEnums.h>
#include <pump/PumpServiceMessages.h>

class IControlDevice
{
public:
    virtual ~IControlDevice() = default;
    virtual void init() = 0;
    virtual void onPumpStatusUpdated(const PumpStatus &status) = 0;
    virtual void onAlarmStatusUpdated(const AnnunciationType &annunciation, bool cancel = false) = 0;
};

#endif // ICONTROL_DEVICE_H
