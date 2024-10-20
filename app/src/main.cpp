#include <ble/BLEComm.h>
#include <control/ControlService.h>
#include <hmi/HmiService.h>
#include <pump/PumpService.h>

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

    PumpService pumpService(pumpDevice);
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
