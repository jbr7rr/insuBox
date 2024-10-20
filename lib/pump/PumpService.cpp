#include <pump/PumpService.h>
#include <pump/VirtualPumpDevice.h>
#include <pump/medtrum_bt/MedtrumBTDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_pump_service);

PumpService::PumpService(IPumpDevice &pumpDevice) : mPumpDevice(pumpDevice) {}

PumpService::~PumpService() {}

void PumpService::init()
{
    LOG_DBG("Initializing PumpService");
    mPumpDevice.init();
}

IPumpDevice &PumpService::getPumpDevice()
{
#ifdef CONFIG_IB_PUMP_MEDTRUM_BT
    static MedtrumBTDevice pumpDevice;
#elif defined(CONFIG_IB_PUMP_VIRTUAL)
    static VirtualPumpDevice pumpDevice;
#else
#error "No pump device selected, error in config"
#endif
    return pumpDevice;
}
