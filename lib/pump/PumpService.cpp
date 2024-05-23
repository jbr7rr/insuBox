#include <pump/PumpService.h>
#include <pump/medtrum/MedtrumDevice.h>

PumpService::PumpService() {
    // Constructor
    mPumpDevice = &mMedtrumDevice;
}

PumpService::~PumpService() {
    // Destructor
    delete mPumpDevice;
}

void PumpService::init() {
    // Initialize the PumpDevice
    mPumpDevice->init();
}
