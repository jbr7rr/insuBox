#include <hmi/HmiService.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

HmiService::HmiService(EventDispatcher &dispatcher) : mDispatcher(dispatcher)
{
    LOG_DBG("HmiService constructor");
    mDispatcher.subscribe<BtPassKeyConfirmRequest>([this](const BtPassKeyConfirmRequest &request) {
        this->onBtPassKeyConfirmRequest(request);
    });
}

HmiService::~HmiService() {}

void HmiService::init()
{
    LOG_DBG("Initializing HmiService");
}

void HmiService::onBtPassKeyConfirmRequest(const BtPassKeyConfirmRequest &request)
{
    LOG_DBG("Passkey confirm request");

    // TODO: Implement HMI to ask user to confirm the passkey
    // For now just accept the passkey and send response
    mDispatcher.dispatch<BtPassKeyConfirmResponse>({request.conn, true});
}
