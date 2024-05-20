#include <pump/medtrum/MedtrumDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>

#include <pump/medtrum/comm/packets/AuthPacket.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_medtrum_device);

MedtrumDevice::MedtrumDevice() : mPumpBleComm(*this)
{
    // Constructor
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

    // TODO: Temp code
    mActivePacket = std::make_unique<AuthPacket>(mDeviceSN.value_or(0));
    auto &request = mActivePacket->getRequest();
    mPumpBleComm.writeCommand(request.data(), request.size());
}

void MedtrumDevice::onDisconnected()
{
    // Callback when the device is disconnected
    LOG_DBG("Disconnected from pump");
}

void MedtrumDevice::onWriteError()
{
    // Callback when there is a write error
    LOG_ERR("Write error");
}

void MedtrumDevice::onCommandResponse(uint8_t *data, size_t length)
{
    // Callback when a command response is received
    LOG_DBG("Command response received");

    if (mActivePacket)
    {
        bool ready = mActivePacket->onNotification(data, length);
        if (ready)
        {
            LOG_DBG("Packet ready");
            mActivePacket.reset();
        }
    }
}

void MedtrumDevice::onNotification(uint8_t *data, size_t length)
{
    // Callback when a notification is received
    LOG_DBG("Notification received");
}
