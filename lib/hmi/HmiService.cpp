#include <ble/BLEComm.h>
#include <hmi/HmiService.h>
#include <hmi/VirtualHmiDevice.h>
#include <hmi/insubox/InsuBoxHmiDevice.h>
#include <pump/PumpService.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

k_work_q HmiService::mWorkQueue;

HmiService::HmiService(EventDispatcher &dispatcher) : HmiService(dispatcher, getHmiDevice(*this)) {}

HmiService::HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice)
    : mDispatcher(dispatcher), mHmiDevice(hmiDevice)
{
    k_work_queue_init(&mWorkQueue);
    static k_work_queue_config config = {.name = "hmi", .no_yield = false, .essential = true};
    k_work_queue_start(&mWorkQueue, mWorkQueueBuffer, K_THREAD_STACK_SIZEOF(mWorkQueueBuffer), 0, &config);

    LOG_DBG("HmiService constructor");

    mInitTask.service = this;
    k_work_init(&mInitTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SimpleTask, work);
        container->service->mHmiDevice.init();
    });

    mPassKeyDisplayTask.service = this;
    k_work_init(&mPassKeyDisplayTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, PassKeyDisplayTask, work);
        container->service->mHmiDevice.onUserBtPairingRequest(container->conn, container->passkey);
    });

    mDispatcher.subscribe<BtPassKeyConfirmRequest>([this](const BtPassKeyConfirmRequest &request) {
        if (k_work_busy_get(&mPassKeyDisplayTask.work))
        {
            LOG_ERR("Passkey display task is busy");
            mDispatcher.dispatch<BtPassKeyConfirmResponse>({request.conn, false});
            return;
        }
        this->mPassKeyDisplayTask.conn = request.conn;
        this->mPassKeyDisplayTask.passkey = request.passkey;
        k_work_submit_to_queue(&mWorkQueue, &mPassKeyDisplayTask.work);
    });

    mBtBluetoothStateChangedTask.service = this;
    k_work_init(&mBtBluetoothStateChangedTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, BtBluetoothStateChangedTask, work);
        container->service->mHmiDevice.onBtBluetoothStateChanged(container->conn, container->state);
    });

    mDispatcher.subscribe<BtBluetoothStateChanged>([this](const BtBluetoothStateChanged &state) {
        if (k_work_busy_get(&mBtBluetoothStateChangedTask.work))
        {
            LOG_ERR("Bluetooth state change task is busy");
            return;
        }
        this->mBtBluetoothStateChangedTask.conn = state.conn;
        this->mBtBluetoothStateChangedTask.state = state.state;
        k_work_submit_to_queue(&mWorkQueue, &mBtBluetoothStateChangedTask.work);
    });

    mBolusProgressUpdateTask.service = this;
    k_work_init(&mBolusProgressUpdateTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, BolusProgressUpdateTask, work);
        container->service->mHmiDevice.onBolusProgressUpdate(container->update);
    });

    mDispatcher.subscribe<BolusProgressUpdate>([this](const BolusProgressUpdate &update) {
        if (k_work_busy_get(&mBolusProgressUpdateTask.work))
        {
            LOG_ERR("Bolus progress update task is busy");
            return;
        }
        this->mBolusProgressUpdateTask.update = update;
        k_work_submit_to_queue(&mWorkQueue, &mBolusProgressUpdateTask.work);
    });
}

HmiService::~HmiService() {}

void HmiService::init()
{
    LOG_DBG("Initializing HmiService");
    k_work_submit_to_queue(&mWorkQueue, &mInitTask.work);
}

void HmiService::onUserBtPairingResponse(struct bt_conn *conn, bool accepted)
{
    mDispatcher.dispatch<BtPassKeyConfirmResponse>({conn, accepted});
}

void HmiService::onBolusRequest(float amount, time_t timestamp)
{
    mDispatcher.dispatch<BolusRequest>({amount, timestamp});
}

void HmiService::onStopBolus()
{
    mDispatcher.dispatch<StopBolus>({});
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
