#ifndef PUMP_SERVICE_MESSAGES_H
#define PUMP_SERVICE_MESSAGES_H

#include <control/IdsEnums.h>
#include <ctime>
#include <optional>
#include <utils/sfloat.h>
#include <zephyr/types.h>

struct PumpStatus
{
    std::optional<TherapyControlState> therapyControlState;
    std::optional<OperationalState> operationalState;
    std::optional<SFloat> reservoirLevel;
    std::optional<bool> reservoirAttached;
};

struct PumpAnnunciationStatus
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

struct RetractRequest
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

#endif // PUMP_SERVICE_MESSAGES_H
