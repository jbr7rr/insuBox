#include <pump/PumpService.h>
#include <pump/medtrum/MedtrumDevice.h>

PumpService::PumpService()
{
    mPumpDevice = &mMedtrumDevice;
}

PumpService::~PumpService()
{
    delete mPumpDevice;
}

void PumpService::init()
{
    mPumpDevice->init();
}
