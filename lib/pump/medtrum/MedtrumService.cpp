#include <pump/medtrum/MedtrumService.h>
#include <pump/medtrum/comm/PumpBleComm.h>


MedtrumService::MedtrumService() {
    // Constructor
}

MedtrumService::~MedtrumService() {
    // Destructor
}

void MedtrumService::init() {
    // Initialize the MedtrumService
    pumpBleComm.init();
    pumpBleComm.connect();
}