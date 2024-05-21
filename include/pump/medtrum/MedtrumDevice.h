#ifndef MEDTRUM_DEVICE_H
#define MEDTRUM_DEVICE_H

#include <pump/IPumpDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>
#include <pump/medtrum/comm/packets/AuthPacket.h>
#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>

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
    std::optional<uint32_t> mDeviceSN = 0xE4B83178;
    std::unique_ptr<MedtrumBasePacket> mActivePacket = nullptr;

    k_work_q mWorkQueue;
    K_THREAD_STACK_MEMBER(mWorkQueueBuffer, 2048);

    struct k_sem mCommandResponseSem;

    void _negotiateConnection();
};

#endif // MEDTRUM_DEVICE_H
