#include <hmi/VirtualHmiDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_hmi_device);

VirtualHmiDevice::VirtualHmiDevice()
{
    LOG_DBG("VirtualHmiDevice constructor");
}

VirtualHmiDevice::~VirtualHmiDevice()
{
    LOG_DBG("VirtualHmiDevice destructor");
}

void VirtualHmiDevice::init()
{
    LOG_DBG("VirtualHmiDevice init");
}
