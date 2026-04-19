#include <ble/BLEComm.h>
#include <hmi/HmiService.h>
#include <hmi/VirtualHmiDevice.h>
#include <hmi/insubox/InsuBoxHmiDevice.h>
#include <pump/PumpService.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_hmi_service);

k_work_q HmiService::mWorkQueue;

namespace
{
    K_MSGQ_DEFINE(bluetoothStateChangeQueue, sizeof(BtBluetoothStateChanged), CONFIG_BT_MAX_CONN, 4);
}

HmiService::HmiService(EventDispatcher &dispatcher) : HmiService(dispatcher, getHmiDevice(*this)) {}

HmiService::HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice)
    : mDispatcher(dispatcher), mHmiDevice(hmiDevice)
{
    LOG_DBG("HmiService constructor");

    k_work_queue_init(&mWorkQueue);
    static k_work_queue_config config = {.name = "hmi", .no_yield = false, .essential = true};
    k_work_queue_start(&mWorkQueue, mWorkQueueBuffer, K_THREAD_STACK_SIZEOF(mWorkQueueBuffer), 0, &config);

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

    mBolusProgressUpdateTask.service = this;
    k_work_init(&mBolusProgressUpdateTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, BolusProgressUpdateTask, work);
        container->service->mHmiDevice.onBolusProgressUpdate(container->update);
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
        auto *container = CONTAINER_OF(work, SimpleTask, work);
        BtBluetoothStateChanged state;
        int err = k_msgq_get(&bluetoothStateChangeQueue, &state, K_NO_WAIT);
        if (err == -ENOMSG)
        {
            LOG_DBG("No Bluetooth state change message available in queue");
            return;
        }
        else if (err < 0)
        {
            LOG_ERR("Failed to get Bluetooth state change message from queue: %d", err);
            k_work_submit_to_queue(&mWorkQueue, work);
            return;
        }

        container->service->mHmiDevice.onBtBluetoothStateChanged(state.conn, state.state);
        if (k_msgq_num_used_get(&bluetoothStateChangeQueue) > 0)
        {
            k_work_submit_to_queue(&mWorkQueue, work);
        }
    });

    mDispatcher.subscribe<BtBluetoothStateChanged>([this](const BtBluetoothStateChanged &state) {
        LOG_DBG("Bluetooth state changed: conn=%p, state=%d", state.conn, static_cast<int>(state.state));
        if (k_msgq_put(&bluetoothStateChangeQueue, &state, K_NO_WAIT) != 0)
        {
            LOG_ERR("Failed to put Bluetooth state change message in queue");
            k_msgq_purge(&bluetoothStateChangeQueue);
            return;
        }
        k_work_submit_to_queue(&mWorkQueue, &mBtBluetoothStateChangedTask.work);
        LOG_DBG("Bluetooth state change message submitted to work queue");
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

HmiService::~HmiService()
{
    k_work_queue_drain(&mWorkQueue, true);
}

void HmiService::init()
{
    LOG_DBG("Initializing HmiService");
    k_work_submit_to_queue(&mWorkQueue, &mInitTask.work);
}

void HmiService::userBtPairingResponse(struct bt_conn *conn, bool accepted)
{
    mDispatcher.dispatch<BtPassKeyConfirmResponse>({conn, accepted});
}

void HmiService::bolusRequest(float amount, time_t timestamp)
{
    mDispatcher.dispatch<BolusRequest>({amount, timestamp});
}

void HmiService::stopBolusRequest()
{
    mDispatcher.dispatch<StopBolus>({});
}

void HmiService::retractRequest()
{
    mDispatcher.dispatch<RetractRequest>({});
}

void HmiService::primeRequest()
{
    mDispatcher.dispatch<PrimeRequest>({});
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
