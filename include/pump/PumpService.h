#ifndef PUMP_SERVICE_H
#define PUMP_SERVICE_H

#include <events/EventDispatcher.h>
#include <pump/IPumpDevice.h>

class PumpService : public IPumpDeviceCallback
{
public:
    PumpService(EventDispatcher &dispatcher);
    PumpService(EventDispatcher &dispatcher, IPumpDevice &pumpDevice);
    ~PumpService();
    void init();

protected:
    void pumpStatusUpdate(const PumpStatus &status) override;
    void bolusProgressUpdate(const BolusProgressUpdate &update) override;

private:
    EventDispatcher &mDispatcher;
    IPumpDevice &mPumpDevice;

    /**
     * @brief Get the pump internal pump device object of the selected type in Kconfig
     *
     * @return IPumpDevice&
     */
    static IPumpDevice &getPumpDevice(IPumpDeviceCallback &pumpDeviceCallback);
};

#endif // PUMP_SERVICE_H
