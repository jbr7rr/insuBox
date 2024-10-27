#include <ble/BLEComm.h>
#include <hmi/HmiService.h>
#include <hmi/VirtualHmiDevice.h>

#include <zephyr/zbus/zbus.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

HmiService::HmiService(EventDispatcher &dispatcher) : HmiService(dispatcher, getHmiDevice(*this)) {}

HmiService::HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice)
    : mDispatcher(dispatcher), mHmiDevice(hmiDevice)
{
    k_work_queue_init(&mWorkQueue);
    k_work_queue_start(&mWorkQueue, mWorkQueueBuffer, K_THREAD_STACK_SIZEOF(mWorkQueueBuffer), 0, NULL);

    LOG_DBG("HmiService constructor");

    mPassKeyDisplayTask.service = this;
    k_work_init(&mPassKeyDisplayTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, PassKeyDisplayTask, work);
        container->service->mHmiDevice.onUserBtPairingRequest(container->conn, container->passkey);
    });

    mDispatcher.subscribe<BtPassKeyConfirmRequest>([this](const BtPassKeyConfirmRequest &request) {
        this->mPassKeyDisplayTask.conn = request.conn;
        this->mPassKeyDisplayTask.passkey = request.passkey;
        k_work_submit_to_queue(&mWorkQueue, &mPassKeyDisplayTask.work);
    });
}

HmiService::~HmiService() {}

void HmiService::init()
{
    LOG_DBG("Initializing HmiService");
    mHmiDevice.init();
}

void HmiService::onUserBtPairingResponse(struct bt_conn *conn, bool accepted)
{
    LOG_DBG("User pairing response: %d", accepted);
    mDispatcher.dispatch<BtPassKeyConfirmResponse>({conn, accepted});
}

IHmiDevice &HmiService::getHmiDevice(IHmiCallback &hmiCallback)
{
#ifdef CONFIG_IB_HMI_VIRTUAL
    static VirtualHmiDevice hmiDevice(hmiCallback);
#else
#error "No hmi device selected, error in config"
#endif
    return hmiDevice;
}
