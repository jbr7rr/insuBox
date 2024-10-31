
#include "bt_cts/bt_cts.h"
#include <control/ControlService.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_control_service);

ControlService::ControlService(IInsulinDeliveryDevice &insulinDeliveryDevice)
    : mInsulinDeliveryDevice(insulinDeliveryDevice)
{
    LOG_DBG("ControlService constructor");
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
