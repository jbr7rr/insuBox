#ifndef IDS_ENUMS_H
#define IDS_ENUMS_H

#include <cstdint>
#include <zephyr/sys/util.h>

enum class TherapyControlState : uint8_t
{
    UNDETERMINED = 0X0F,
    STOP = 0x33,
    PAUSE = 0x3C,
    RUN = 0x55
};

enum class OperationalState : uint8_t
{
    UNDETERMINED = 0X0F,
    OFF = 0x33,
    STANDBY = 0x3C,
    PREPARING = 0x55,
    PRIMING = 0X5A,
    WAITING = 0x56,
    READY = 0x96
};

enum class StatusChangedFlags : uint8_t
{
    THERAPY_CONTROL_STATE_CHANGED = BIT(0),
    OPERATIONAL_STATE_CHANGED = BIT(1),
    RESERVOIR_CHANGED = BIT(2),
    ANNUNCIATION_CHANGED = BIT(3),
    TOTAL_DAILY_INSULIN_CHANGED = BIT(4),
    ACTIVE_BASAL_RATE_CHANGED = BIT(5),
    ACTIVE_BOLUS_CHANGED = BIT(6),
    HISTORY_EVENT_RECORDED = BIT(7)
};

#endif // IDS_ENUMS_H