#ifndef CONTROL_SERVICE_H
#define CONTROL_SERVICE_H

#include <control/bt_ids/InsulinDeliveryDevice.h>
#include <events/EventDispatcher.h>

class ControlService
{
public:
    ControlService(EventDispatcher &dispatcher);
    ControlService(EventDispatcher &dispatcher, IInsulinDeliveryDevice &insulinDeliveryDevice);
    ~ControlService();
    void init();

private:
    EventDispatcher &mDispatcher;
    IInsulinDeliveryDevice &mInsulinDeliveryDevice;

    static IInsulinDeliveryDevice &getInsulinDeliveryDevice();
};

#endif // CONTROL_SERVICE_H