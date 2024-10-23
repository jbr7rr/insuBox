#include <ble/BLEComm.h>
#include <control/ControlService.h>
#include <hmi/HmiService.h>
#include <pump/PumpService.h>

namespace
{
    EventDispatcher eventDispatcher;
    PumpService pumpService;
    ControlService controlService;
    HmiService hmiService(eventDispatcher);
}

int main(void)
{
    BLEComm::init(&eventDispatcher);
    pumpService.init();
    controlService.init();
    hmiService.init();

    return 0;
}
