#include <pump/medtrum/MedtrumDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>
#include <pump/medtrum/crypt/Crypt.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_medtrum_device);

// TODO: Helper funcs, move to seperate file
void vector_add_le32(std::vector<uint8_t> &vec, uint32_t value)
{
    vec.push_back(static_cast<uint8_t>(value & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

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

    // Temp code
    mWriteCommandsDataBuffer.clear();
    mWriteCommandsDataBuffer.push_back(0x05);
    mWriteCommandsDataBuffer.push_back(0x02);
    mWriteCommandsDataBuffer.insert(mWriteCommandsDataBuffer.end(), {0x00, 0x00, 0x00, 0x00});
    uint32_t key = Crypt::keyGen(mDeviceSN.value_or(0));
    vector_add_le32(mWriteCommandsDataBuffer, key);

    mPumpBleComm.writeCommand(mWriteCommandsDataBuffer.data(), mWriteCommandsDataBuffer.size());
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
}

void MedtrumDevice::onNotification(uint8_t *data, size_t length)
{
    // Callback when a notification is received
    LOG_DBG("Notification received");
}
