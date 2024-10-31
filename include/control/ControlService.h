#ifndef CONTROL_SERVICE_H
#define CONTROL_SERVICE_H

#include <control/bt_ids/InsulinDeliveryDevice.h>

class ControlService
{
public:
    ControlService(IInsulinDeliveryDevice &insulinDeliveryDevice = ControlService::getInsulinDeliveryDevice());
    ~ControlService();
    void init();
private:
    IInsulinDeliveryDevice &mInsulinDeliveryDevice;

    static IInsulinDeliveryDevice &getInsulinDeliveryDevice();
};

#endif // CONTROL_SERVICE_H