#ifndef PUMP_SERVICE_H
#define PUMP_SERVICE_H

#include <control/IdsEnums.h>
#include <events/EventDispatcher.h>
#include <pump/IPumpDevice.h>
#include <utils/sfloat.h>
#include <optional>

struct PumpStatusUpdated
{
    std::optional<TherapyControlState> therapyControlState;
    std::optional<OperationalState> operationalState;
    std::optional<SFloat> reservoirLevel;
    std::optional<bool> reservoirAttached;
};

struct PumpAnnunciationStatusUpdated
{
    AnnunciationType annunciation;
    bool cancel;
};

class IPumpServiceCallback
{
public:
    virtual void pumpStatusUpdated(const PumpStatusUpdated &status) = 0;
};

class PumpService : public IPumpServiceCallback
{
public:
    PumpService(EventDispatcher &dispatcher);
    PumpService(EventDispatcher &dispatcher, IPumpDevice &pumpDevice);
    ~PumpService();
    void init();

protected:
    void pumpStatusUpdated(const PumpStatusUpdated &status) override;

private:
    EventDispatcher &mDispatcher;
    IPumpDevice &mPumpDevice;

    /**
     * @brief Get the pump internal pump device object of the selected type in Kconfig
     *
     * @return IPumpDevice&
     */
    static IPumpDevice &getPumpDevice(IPumpServiceCallback &pumpServiceCallback);
};

#endif // PUMP_SERVICE_H
