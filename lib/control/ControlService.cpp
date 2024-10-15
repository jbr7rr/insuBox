
#include <control/ControlService.h>
#include "bt_cts/bt_cts.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_control_service);

ControlService::ControlService()
{
    LOG_DBG("ControlService constructor");
}

ControlService::~ControlService() {}

void ControlService::init()
{
    LOG_DBG("Initializing ControlService");
    bt_cts::init();
}
