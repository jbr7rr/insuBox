#ifndef PUMP_SERVICE_H
#define PUMP_SERVICE_H

#include <pump/IPumpDevice.h>

class PumpService
{
public:
    PumpService(IPumpDevice &pumpDevice = PumpService::getPumpDevice());
    ~PumpService();
    void init();

private:
    IPumpDevice &mPumpDevice;

    /**
     * @brief Get the pump internal pump device object of the selected type in Kconfig
     *
     * @return IPumpDevice&
     */
    static IPumpDevice &getPumpDevice();
};

#endif // PUMP_SERVICE_H
