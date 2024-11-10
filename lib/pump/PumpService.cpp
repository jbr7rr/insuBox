#include <pump/PumpService.h>
#include <pump/VirtualPumpDevice.h>
#include <pump/medtrum_bt/MedtrumBTDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_pump_service);

PumpService::PumpService(EventDispatcher &dispatcher) : PumpService(dispatcher, getPumpDevice(*this)) {}

PumpService::PumpService(EventDispatcher &dispatcher, IPumpDevice &pumpDevice)
    : mDispatcher(dispatcher), mPumpDevice(pumpDevice)
{
    LOG_DBG("PumpService constructor");
}

PumpService::~PumpService() {}

void PumpService::init()
{
    LOG_DBG("Initializing PumpService");
    mPumpDevice.init();
}

void PumpService::pumpStatusUpdated(const PumpStatusUpdated &status)
{
    mDispatcher.dispatch<PumpStatusUpdated>(status);
}

IPumpDevice &PumpService::getPumpDevice(IPumpServiceCallback &pumpServiceCallback)
{
#ifdef CONFIG_IB_PUMP_MEDTRUM_BT
    static MedtrumBTDevice pumpDevice;
#elif defined(CONFIG_IB_PUMP_VIRTUAL)
    static VirtualPumpDevice pumpDevice(pumpServiceCallback);
#else
#error "No pump device selected, error in config"
#endif
    return pumpDevice;
}
