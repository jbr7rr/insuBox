#ifndef PUMP_SERVICE_H
#define PUMP_SERVICE_H

#include <control/IdsEnums.h>
#include <ctime>
#include <events/EventDispatcher.h>
#include <optional>
#include <pump/IPumpDevice.h>
#include <utils/sfloat.h>

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

struct BolusRequest
{
    float amount;
    time_t timestamp;
};

struct StopBolus
{
};

struct BolusProgressUpdate
{
    float requestedAmount;
    time_t requestedTimestamp;
    float deliveredAmount;
    time_t deliveredTimestamp;
    bool completed;
};

class IPumpServiceCallback
{
public:
    virtual void pumpStatusUpdated(const PumpStatusUpdated &status) = 0;
    virtual void onBolusProgressUpdate(const BolusProgressUpdate &update) = 0;
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
    void onBolusProgressUpdate(const BolusProgressUpdate &update) override;

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
