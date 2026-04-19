
#include <control/ControlService.h>
#include <control/IdsEnums.h>
#include <pump/PumpService.h>

#ifdef CONFIG_IB_CONTROL_IDS
#include <control/bt_ids/InsulinDeliveryDevice.h>
#endif

#ifdef CONFIG_IB_CONTROL_KALEIDO
#include <control/kaleido/KaleidoDevice.h>
#endif

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_control_service);

ControlService::ControlService(EventDispatcher &dispatcher) : ControlService(dispatcher, getControlDevice(dispatcher))
{
}

ControlService::ControlService(EventDispatcher &dispatcher, IControlDevice &controlDevice)
    : mDispatcher(dispatcher), mControlDevice(controlDevice)
{
    LOG_DBG("ControlService constructor");

    mDispatcher.subscribe<PumpStatus>(
        [this](const PumpStatus &status) { this->mControlDevice.onPumpStatusUpdated(status); });
}

ControlService::~ControlService() {}

void ControlService::init()
{
    LOG_DBG("Initializing ControlService");
    mControlDevice.init();
}

IControlDevice &ControlService::getControlDevice(EventDispatcher &dispatcher)
{
#ifdef CONFIG_IB_CONTROL_IDS
    static InsulinDeliveryDevice insulinDeliveryDevice;
    return insulinDeliveryDevice;
#elif defined(CONFIG_IB_CONTROL_KALEIDO)
    static KaleidoDevice kaleidoDevice{dispatcher};
    return kaleidoDevice;
#endif
}
