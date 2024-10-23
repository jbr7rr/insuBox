#include <hmi/HmiService.h>
#include <hmi/VirtualHmiDevice.h>
#include <ble/BLEComm.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

HmiService::HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice) : mDispatcher(dispatcher), mHmiDevice(hmiDevice)
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

IHmiDevice &HmiService::getHmiDevice()
{
#ifdef CONFIG_IB_HMI_VIRTUAL
    static VirtualHmiDevice hmiDevice;
#else
#error "No hmi device selected, error in config"
#endif
    return hmiDevice;
}

void HmiService::onBtPassKeyConfirmRequest(const BtPassKeyConfirmRequest &request)
{
    LOG_DBG("Passkey confirm request");

    // TODO: Implement HMI to ask user to confirm the passkey
    // For now just accept the passkey and send response
    mDispatcher.dispatch<BtPassKeyConfirmResponse>({request.conn, true});
}
