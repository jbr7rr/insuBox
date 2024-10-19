#include <hmi/HmiService.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

HmiService::HmiService()
{
    LOG_DBG("HmiService constructor");
}

HmiService::~HmiService() {}

void HmiService::init()
{
    LOG_DBG("Initializing HmiService");
}
