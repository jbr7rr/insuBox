#ifndef CONTROL_SERVICE_H
#define CONTROL_SERVICE_H

#include <control/IControlDevice.h>
#include <events/EventDispatcher.h>

class ControlService
{
public:
    ControlService(EventDispatcher &dispatcher);
    ControlService(EventDispatcher &dispatcher, IControlDevice &controlDevice);
    ~ControlService();
    void init();

private:
    EventDispatcher &mDispatcher;
    IControlDevice &mControlDevice;

    static IControlDevice &getControlDevice(EventDispatcher &dispatcher);
};

#endif // CONTROL_SERVICE_H