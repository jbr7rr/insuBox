#include <pump/PumpService.h>
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
