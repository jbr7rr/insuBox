#include <pump/medtrum_bt/MedtrumBTDevice.h>
#include <pump/medtrum_bt/comm/PumpBleComm.h>

#include <pump/medtrum_bt/comm/packets/AuthPacket.h>
#include <pump/medtrum_bt/comm/packets/SubscribePacket.h>
#include <pump/medtrum_bt/comm/packets/SynchronizePacket.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_medtrum_device);

MedtrumBTDevice::MedtrumBTDevice() : mPumpBleComm(*this), mNotificationPacket(mPumpSync)
{
    // Constructor
    mSubContainer.mDevice = this;
    k_work_queue_init(&mWorkQueue);
    k_work_queue_start(&mWorkQueue, mWorkQueueBuffer, K_THREAD_STACK_SIZEOF(mWorkQueueBuffer), 1, NULL);

    k_mutex_init(&mActivePacketMutex);
    k_sem_init(&mCommandResponseSem, 0, 1);

    k_work_init_delayable(&mSubContainer.mNegotiateConnectionWork, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SubContainer, mNegotiateConnectionWork);
        container->mDevice->_negotiateConnection();
    });
}

MedtrumBTDevice::~MedtrumBTDevice()
{
    // Destructor
}

void MedtrumBTDevice::init()
{
    // Initialize the MedtrumBTDevice
    LOG_DBG("Initializing MedtrumBTDevice");
    mPumpSync.init();
    mPumpBleComm.init();
    mPumpBleComm.connect(mDeviceSN.value_or(0));

    // TEMP
    auto state = mPumpSync.getPumpState();
    LOG_DBG("Pump state: %d", static_cast<uint8_t>(state));
    auto level = mPumpSync.getReservoirLevel();
    LOG_DBG("Reservoir level: %f", static_cast<double>(level));
}

void MedtrumBTDevice::onReadyForCommands()
{
    // Callback when the device is connected
    LOG_DBG("Connected to pump");

    // Maybe wrapper?
    k_work_reschedule_for_queue(&mWorkQueue, &mSubContainer.mNegotiateConnectionWork, K_NO_WAIT);
}

void MedtrumBTDevice::onDisconnected()
{
    // Callback when the device is disconnected
    LOG_DBG("Disconnected from pump");

    k_mutex_lock(&mActivePacketMutex, K_FOREVER);

    if (mActivePacket)
    {
        // TODO: Some kind of retry mechanism?
        // For now just give the semaphore
        k_sem_give(&mCommandResponseSem);
    }

    k_mutex_unlock(&mActivePacketMutex);
}

void MedtrumBTDevice::onWriteError()
{
    // Callback when there is a write error
    LOG_ERR("Write error");

    k_mutex_lock(&mActivePacketMutex, K_FOREVER);

    if (mActivePacket)
    {
        // TODO: Some kind of retry mechanism?
        // For now just give the semaphore
        k_sem_give(&mCommandResponseSem);
    }

    k_mutex_unlock(&mActivePacketMutex);
}

void MedtrumBTDevice::onCommandResponse(uint8_t *data, size_t length)
{
    // Callback when a command response is received
    LOG_DBG("Command response received");

    k_mutex_lock(&mActivePacketMutex, K_FOREVER);

    if (mActivePacket)
    {
        mActivePacket->onIndication(data, length);
        if (mActivePacket->isReady())
        {
            LOG_DBG("Packet ready");
            k_sem_give(&mCommandResponseSem);
        }
    }
    else
    {
        LOG_ERR("No active packet");
    }

    k_mutex_unlock(&mActivePacketMutex);
}

void MedtrumBTDevice::onNotification(uint8_t *data, size_t length)
{
    // Callback when a notification is received
    LOG_DBG("Notification received");
    mNotificationPacket.onNotification(data, length);
}

bool MedtrumBTDevice::sendPacketAndWaitForResponse(std::unique_ptr<MedtrumBasePacket> &&packet, k_timeout_t timeout)
{
    k_mutex_lock(&mActivePacketMutex, K_FOREVER);
    // Send a packet and wait for a response
    if (mActivePacket)
    {
        LOG_ERR("Already have an active packet");
        k_mutex_unlock(&mActivePacketMutex);
        return false;
    }
    mActivePacket = std::move(packet);
    auto &request = mActivePacket->getRequest();
    mPumpBleComm.writeCommand(request.data(), request.size());
    k_mutex_unlock(&mActivePacketMutex);

    k_sem_take(&mCommandResponseSem, timeout);

    k_mutex_lock(&mActivePacketMutex, K_FOREVER);
    bool result = mActivePacket->isReady() && !mActivePacket->isFailed();
    mActivePacket.reset();
    k_mutex_unlock(&mActivePacketMutex);

    return result;
}

// TASKS

void MedtrumBTDevice::_negotiateConnection()
{
    // Negotiate the connection
    LOG_DBG("Negotiating connection");

    bool result = sendPacketAndWaitForResponse(std::make_unique<AuthPacket>(mPumpSync, mDeviceSN.value_or(0)));
    if (!result)
    {
        LOG_ERR("Failed to authenticate");
        return;
    }

    result = sendPacketAndWaitForResponse(std::make_unique<SynchronizePacket>(mPumpSync));
    if (!result)
    {
        LOG_ERR("Failed to synchronize");
        return;
    }

    result = sendPacketAndWaitForResponse(std::make_unique<SubscribePacket>());
    if (!result)
    {
        LOG_ERR("Failed to subscribe");
        return;
    }
}