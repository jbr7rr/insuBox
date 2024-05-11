#include <pump/medtrum/MedtrumService.h>
#include <pump/medtrum/comm/PumpBleComm.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_medtrum_service);


MedtrumService::MedtrumService() {
    // Constructor
}

MedtrumService::~MedtrumService() {
    // Destructor
}

void MedtrumService::init() {
    // Initialize the MedtrumService
    LOG_DBG("Initializing MedtrumService");
    pumpBleComm.init();
    pumpBleComm.connect();
}