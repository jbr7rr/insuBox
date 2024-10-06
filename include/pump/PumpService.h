#ifndef PUMP_SERVICE_H
#define PUMP_SERVICE_H

#include <pump/IPumpDevice.h>
#include <pump/PumpService.h>

#include <pump/medtrum_bt/MedtrumBTDevice.h>

class PumpService
{
public:
    PumpService(IPumpDevice &pumpDevice);
    ~PumpService();
    void init();

private:
    IPumpDevice &mPumpDevice;
};

#endif // PUMP_SERVICE_H
