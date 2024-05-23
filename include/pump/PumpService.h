#ifndef PUMP_SERVICE_H
#define PUMP_SERVICE_H

#include <pump/IPumpDevice.h>
#include <pump/PumpService.h>

#include <pump/medtrum/MedtrumDevice.h>

class PumpService
{
public:
    PumpService();
    ~PumpService();
    void init();

private:
    IPumpDevice *mPumpDevice;

    // TODO: Determine build time which pump to use
    MedtrumDevice mMedtrumDevice;
};

#endif // PUMP_SERVICE_H
