#ifndef MEDTRUM_DEVICE_H
#define MEDTRUM_DEVICE_H

#include <pump/IPumpDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>
#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>
#include <pump/medtrum/comm/packets/AuthPacket.h>

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
    PumpBleComm mPumpBleComm;
    std::optional<uint32_t> mDeviceSN = 0xE4B83178;
    std::unique_ptr<MedtrumBasePacket> mActivePacket = nullptr;

    std::vector<uint8_t> mWriteCommandsDataBuffer = {}; // Buffer for whole command, maybe need to move this somewhere?
};

#endif // MEDTRUM_DEVICE_H
