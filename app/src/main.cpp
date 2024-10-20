#include <ble/BLEComm.h>
#include <control/ControlService.h>
#include <hmi/HmiService.h>
#include <pump/PumpService.h>

namespace
{
    PumpService pumpService;
    ControlService controlService;
    HmiService hmiService;
}

int main(void)
{
    BLEComm::init();
    pumpService.init();
    controlService.init();
    hmiService.init();

    return 0;
}
