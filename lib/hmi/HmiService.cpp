#include <ble/BLEComm.h>
#include <hmi/HmiService.h>
#include <hmi/VirtualHmiDevice.h>
#include <hmi/insubox/InsuBoxHmiDevice.h>

#include <zephyr/zbus/zbus.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

k_work_q HmiService::mWorkQueue;

HmiService::HmiService(EventDispatcher &dispatcher) : HmiService(dispatcher, getHmiDevice(*this)) {}

HmiService::HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice)
    : mDispatcher(dispatcher), mHmiDevice(hmiDevice)
{
    k_work_queue_init(&mWorkQueue);
    static k_work_queue_config config = {
        .name = "hmi_work_queue",
        .no_yield = false,
        .essential = true
    };
    k_work_queue_start(&mWorkQueue, mWorkQueueBuffer, K_THREAD_STACK_SIZEOF(mWorkQueueBuffer), 0, &config);

    LOG_DBG("HmiService constructor");

    mPassKeyDisplayTask.service = this;
    k_work_init(&mPassKeyDisplayTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, PassKeyDisplayTask, work);
        container->service->mHmiDevice.onUserBtPairingRequest(container->conn, container->passkey);
    });

    mDispatcher.subscribe<BtPassKeyConfirmRequest>([this](const BtPassKeyConfirmRequest &request) {
        if (k_work_busy_get(&mPassKeyDisplayTask.work))
        {
            LOG_ERR("Passkey display task is busy");
            return;
        }
        this->mPassKeyDisplayTask.conn = request.conn;
        this->mPassKeyDisplayTask.passkey = request.passkey;
        k_work_submit_to_queue(&mWorkQueue, &mPassKeyDisplayTask.work);
    });
}

HmiService::~HmiService() {}

void HmiService::init()
{
    LOG_DBG("Initializing HmiService");

    struct task
    {
        HmiService *service;
        struct k_work work;
    };
    static task initWork;
    initWork.service = this;
    k_work_init(&initWork.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, task, work);
        container->service->mHmiDevice.init();
    });
    k_work_submit_to_queue(&mWorkQueue, &initWork.work);
}

void HmiService::onUserBtPairingResponse(struct bt_conn *conn, bool accepted)
{
    mDispatcher.dispatch<BtPassKeyConfirmResponse>({conn, accepted});
}

IHmiDevice &HmiService::getHmiDevice(IHmiCallback &hmiCallback)
{
#ifdef CONFIG_IB_HMI_VIRTUAL
    static VirtualHmiDevice hmiDevice(hmiCallback);
#elif defined(CONFIG_IB_HMI_INSUBOX)
    static InsuBoxHmiDevice hmiDevice(hmiCallback, mWorkQueue);
#else
#error "No hmi device selected, error in config"
#endif
    return hmiDevice;
}
