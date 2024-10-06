#ifndef MEDTRUM_BASAL_TYPE_H
#define MEDTRUM_BASAL_TYPE_H

#include "BasalEndReason.h"
#include <cstdint>

class BasalType
{
public:
    enum Type : uint8_t
    {
        NONE = 0,
        STANDARD,
        EXERCISE,
        HOLIDAY,
        PROGRAM_A,
        PROGRAM_B,
        ABSOLUTE_TEMP,
        RELATIVE_TEMP,
        PROGRAM_C,
        PROGRAM_D,
        SICK,
        AUTO,
        NEW,
        SUSPEND_LOW_GLUCOSE,
        SUSPEND_PREDICT_LOW_GLUCOSE,
        SUSPEND_AUTO,
        SUSPEND_MORE_THAN_MAX_PER_HOUR,
        SUSPEND_MORE_THAN_MAX_PER_DAY,
        SUSPEND_MANUAL,
        SUSPEND_KEY_LOST,
        STOP_OCCLUSION,
        STOP_EXPIRED,
        STOP_EMPTY,
        STOP_PATCH_FAULT,
        STOP_PATCH_FAULT2,
        STOP_BASE_FAULT,
        STOP_DISCARD,
        STOP_BATTERY_EMPTY,
        STOP,
        PAUSE_INTERRUPT,
        PRIME,
        AUTO_MODE_START,
        AUTO_MODE_EXIT,
        AUTO_MODE_TARGET_100,
        AUTO_MODE_TARGET_110,
        AUTO_MODE_TARGET_120,
        AUTO_MODE_BREAKFAST,
        AUTO_MODE_LUNCH,
        AUTO_MODE_DINNER,
        AUTO_MODE_SNACK,
        AUTO_MODE_EXERCISE_START,
        AUTO_MODE_EXERCISE_EXIT
    };

    // Allow implicit conversion to uint8_t
    operator uint8_t() const
    {
        return value;
    }

    BasalType() : value(NONE) {}

    // Allow implicit conversion from Type
    BasalType(Type v) : value(v) {}

    // Allow implicit conversion from uint8_t
    BasalType(uint8_t v) : value(static_cast<Type>(v)) {}

    static BasalType fromBasalEndReason(BasalEndReason endReason)
    {
        switch (endReason)
        {
        case BasalEndReason::SUSPEND_LOW_GLUCOSE:
            return SUSPEND_LOW_GLUCOSE;
        case BasalEndReason::SUSPEND_PREDICT_LOW_GLUCOSE:
            return SUSPEND_PREDICT_LOW_GLUCOSE;
        case BasalEndReason::SUSPEND_AUTO:
            return SUSPEND_AUTO;
        case BasalEndReason::SUSPEND_MORE_THAN_MAX_PER_HOUR:
            return SUSPEND_MORE_THAN_MAX_PER_HOUR;
        case BasalEndReason::SUSPEND_MORE_THAN_MAX_PER_DAY:
            return SUSPEND_MORE_THAN_MAX_PER_DAY;
        case BasalEndReason::SUSPEND_MANUAL:
            return SUSPEND_MANUAL;
        case BasalEndReason::STOP_OCCLUSION:
            return STOP_OCCLUSION;
        case BasalEndReason::STOP_EXPIRED:
            return STOP_EXPIRED;
        case BasalEndReason::STOP_EMPTY:
            return STOP_EMPTY;
        case BasalEndReason::STOP_PATCH_FAULT:
            return STOP_PATCH_FAULT;
        case BasalEndReason::STOP_PATCH_FAULT2:
            return STOP_PATCH_FAULT2;
        case BasalEndReason::STOP_BASE_FAULT:
            return STOP_BASE_FAULT;
        case BasalEndReason::STOP_PATCH_BATTERY_EMPTY:
            return STOP_BATTERY_EMPTY;
        case BasalEndReason::STOP_DISCARD:
            return STOP_DISCARD;
        case BasalEndReason::STOP_MAG_SENSOR_NO_CALIBRATION:
        case BasalEndReason::STOP_LOW_BATTERY:
        case BasalEndReason::STOP_AUTO_EXIT:
        case BasalEndReason::STOP_CANCEL:
        case BasalEndReason::STOP_LOW_SUPER_CAPACITOR:
            return STOP;
        default:
            return NONE;
        }
    }

private:
    Type value;
};

#endif // MEDTRUM_BASAL_TYPE_H
