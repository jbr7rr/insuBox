#include <pump/VirtualPumpDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_pump_device);

VirtualPumpDevice::VirtualPumpDevice()
{
    LOG_DBG("VirtualPumpDevice constructor");
}

VirtualPumpDevice::~VirtualPumpDevice()
{
    LOG_DBG("VirtualPumpDevice destructor");
}

void VirtualPumpDevice::init()
{
    LOG_DBG("VirtualPumpDevice init");
}
