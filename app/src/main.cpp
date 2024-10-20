#include <ble/BLEComm.h>
#include <control/ControlService.h>
#include <events/EventDispatcher.h>
#include <hmi/HmiService.h>
#include <pump/PumpService.h>

#include <thread>

#include <pump/medtrum_bt/MedtrumBTDevice.h>
#include <pump/VirtualPumpDevice.h>

namespace
{
// Select the pump device to use
#ifdef CONFIG_IB_PUMP_MEDTRUM_BT
    auto pumpDevice = MedtrumBTDevice();
#elif defined(CONFIG_IB_PUMP_VIRTUAL)
    auto pumpDevice = VirtualPumpDevice();
#else
    #error "No pump device selected, error in config"
#endif

    EventDispatcher eventDispatcher;
    ControlService controlService;
    PumpService pumpService(pumpDevice);
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
