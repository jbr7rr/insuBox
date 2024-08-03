#ifndef MEDTRUM_DEVICE_H
#define MEDTRUM_DEVICE_H

#include <pump/IPumpDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>
#include <pump/medtrum/comm/packets/AuthPacket.h>
#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>
#include <pump/medtrum/comm/packets/NotificationPacket.h>
#include <pump/medtrum/MedtrumPumpSync.h>

#include <memory>

class MedtrumDevice : public IPumpDevice, PumpBleCallback
{
public:
    MedtrumDevice();
    ~MedtrumDevice();
    void init() override;

    void onReadyForCommands() override;
    void onDisconnected() override;
    void onWriteError() override;
    void onCommandResponse(uint8_t *data, size_t length) override;
    void onNotification(uint8_t *data, size_t length) override;

private:
    struct SubContainer
    {
        MedtrumDevice *mDevice;
        k_work_delayable mNegotiateConnectionWork;
    } mSubContainer;

    PumpBleComm mPumpBleComm;
    std::optional<uint32_t> mDeviceSN = 0x39B36926;

    struct k_mutex mActivePacketMutex;
    std::unique_ptr<MedtrumBasePacket> mActivePacket = nullptr;

    MedtrumPumpSync mPumpSync;
    NotificationPacket mNotificationPacket;

    bool sendPacketAndWaitForResponse(std::unique_ptr<MedtrumBasePacket> &&packet, k_timeout_t timeout = K_SECONDS(60));

    // Task stuff
    k_work_q mWorkQueue;
    K_THREAD_STACK_MEMBER(mWorkQueueBuffer, KB(4));
    struct k_sem mCommandResponseSem;
    void _negotiateConnection();
};

#endif // MEDTRUM_DEVICE_H
