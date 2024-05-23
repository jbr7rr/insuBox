#include <pump/medtrum/MedtrumDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>

#include <pump/medtrum/comm/packets/AuthPacket.h>
#include <pump/medtrum/comm/packets/SubscribePacket.h>
#include <pump/medtrum/comm/packets/SynchronizePacket.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_medtrum_device);

MedtrumDevice::MedtrumDevice() : mPumpBleComm(*this)
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

MedtrumDevice::~MedtrumDevice()
{
    // Destructor
}

void MedtrumDevice::init()
{
    // Initialize the MedtrumDevice
    LOG_DBG("Initializing MedtrumDevice");
    mPumpBleComm.init();
    mPumpBleComm.connect(mDeviceSN.value_or(0));
}

void MedtrumDevice::onReadyForCommands()
{
    // Callback when the device is connected
    LOG_DBG("Connected to pump");

    // Maybe wrapper?
    k_work_reschedule_for_queue(&mWorkQueue, &mSubContainer.mNegotiateConnectionWork, K_NO_WAIT);
}

void MedtrumDevice::onDisconnected()
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

void MedtrumDevice::onWriteError()
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

void MedtrumDevice::onCommandResponse(uint8_t *data, size_t length)
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

void MedtrumDevice::onNotification(uint8_t *data, size_t length)
{
    // Callback when a notification is received
    LOG_DBG("Notification received");
}

bool MedtrumDevice::sendPacketAndWaitForResponse(std::unique_ptr<MedtrumBasePacket> &&packet, k_timeout_t timeout)
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

void MedtrumDevice::_negotiateConnection()
{
    // Negotiate the connection
    LOG_DBG("Negotiating connection");

    bool result = sendPacketAndWaitForResponse(std::make_unique<AuthPacket>(mDeviceSN.value_or(0)));
    if (!result)
    {
        LOG_ERR("Failed to authenticate");
        return;
    }

    result = sendPacketAndWaitForResponse(std::make_unique<SynchronizePacket>());
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
