
#include "bt_cts/bt_cts.h"
#include <control/ControlService.h>
#include <control/IdsEnums.h>
#include <pump/PumpService.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_control_service);

ControlService::ControlService(EventDispatcher &dispatcher) : ControlService(dispatcher, getInsulinDeliveryDevice()) {}

ControlService::ControlService(EventDispatcher &dispatcher, IInsulinDeliveryDevice &insulinDeliveryDevice)
    : mDispatcher(dispatcher), mInsulinDeliveryDevice(insulinDeliveryDevice)
{
    LOG_DBG("ControlService constructor");

    mDispatcher.subscribe<PumpStatusUpdated>(
        [this](const PumpStatusUpdated &status) { this->mInsulinDeliveryDevice.insulinPumpStatusUpdated(status); });
}

ControlService::~ControlService() {}

void ControlService::init()
{
    LOG_DBG("Initializing ControlService");
    mInsulinDeliveryDevice.init();
    bt_cts::init();
}

IInsulinDeliveryDevice &ControlService::getInsulinDeliveryDevice()
{
    static InsulinDeliveryDevice insulinDeliveryDevice;
    return insulinDeliveryDevice;
}
