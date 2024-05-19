#ifndef PUMP_SERVICE_H
#define PUMP_SERVICE_H

#include <pump/PumpService.h>
#include <pump/IPumpDevice.h>

class PumpService {
public:
    PumpService();
    ~PumpService();
    void init();

private:
    IPumpDevice *mPumpDevice;
};

#endif // PUMP_SERVICE_H
