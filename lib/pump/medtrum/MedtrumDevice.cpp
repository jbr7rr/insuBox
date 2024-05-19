#include <pump/medtrum/MedtrumDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_medtrum_device);


MedtrumDevice::MedtrumDevice() {
    // Constructor
}

MedtrumDevice::~MedtrumDevice() {
    // Destructor
}

void MedtrumDevice::init() {
    // Initialize the MedtrumDevice
    LOG_DBG("Initializing MedtrumDevice");
    pumpBleComm.init();
    pumpBleComm.connect();
}
