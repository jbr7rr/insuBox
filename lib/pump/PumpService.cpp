#include <pump/PumpService.h>
#include <pump/VirtualPumpDevice.h>
#include <pump/insubox/InsuBoxDevice.h>
#include <pump/medtrum_bt/MedtrumBTDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_pump_service);

PumpService::PumpService(EventDispatcher &dispatcher) : PumpService(dispatcher, getPumpDevice(*this)) {}

PumpService::PumpService(EventDispatcher &dispatcher, IPumpDevice &pumpDevice)
    : mDispatcher(dispatcher), mPumpDevice(pumpDevice)
{
    LOG_DBG("PumpService constructor");

    mDispatcher.subscribe<BolusRequest>([this](const BolusRequest &request) {
        LOG_DBG("Bolus request received: %f", static_cast<double>(request.amount));
        mPumpDevice.onBolusRequest(request.amount, request.timestamp);
    });

    mDispatcher.subscribe<StopBolus>([this](const StopBolus &stop) {
        LOG_DBG("Stop bolus request received");
        mPumpDevice.onStopBolus();
    });

    mDispatcher.subscribe<RetractRequest>([this](const RetractRequest &retract) {
        LOG_DBG("Retract request received");
        mPumpDevice.onRetractRequest();
    });
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

void PumpService::onBolusProgressUpdate(const BolusProgressUpdate &update)
{
    LOG_DBG("Bolus progress update: requested %.2f, delivered %.2f, timestamp %lld",
            static_cast<double>(update.requestedAmount), static_cast<double>(update.deliveredAmount),
            update.deliveredTimestamp);
    mDispatcher.dispatch<BolusProgressUpdate>(update);
}

IPumpDevice &PumpService::getPumpDevice(IPumpDeviceCallback &pumpDeviceCallback)
{
#ifdef CONFIG_IB_PUMP_INSUBOX
    static InsuBoxDevice pumpDevice(pumpDeviceCallback);
#elif defined(CONFIG_IB_PUMP_MEDTRUM_BT)
    static MedtrumBTDevice pumpDevice;
#elif defined(CONFIG_IB_PUMP_VIRTUAL)
    static VirtualPumpDevice pumpDevice(pumpDeviceCallback);
#else
#error "No pump device selected, error in config"
#endif
    return pumpDevice;
}
