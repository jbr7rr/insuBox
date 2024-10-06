#ifndef MEDTRUM_BASAL_END_REASON_H
#define MEDTRUM_BASAL_END_REASON_H

#include <cstdint>

class BasalEndReason
{
public:
    enum Reason : uint8_t
    {
        SUCCESS = 0,
        SUSPEND_LOW_GLUCOSE,
        SUSPEND_PREDICT_LOW_GLUCOSE,
        SUSPEND_AUTO,
        SUSPEND_MORE_THAN_MAX_PER_HOUR,
        SUSPEND_MORE_THAN_MAX_PER_DAY,
        SUSPEND_MANUAL,
        STOP_OCCLUSION,
        STOP_EXPIRED,
        STOP_EMPTY,
        STOP_PATCH_FAULT,
        STOP_PATCH_FAULT2,
        STOP_BASE_FAULT,
        STOP_PATCH_BATTERY_EMPTY,
        STOP_MAG_SENSOR_NO_CALIBRATION,
        STOP,
        STOP_LOW_BATTERY,
        STOP_AUTO_EXIT,
        STOP_CANCEL,
        STOP_LOW_SUPER_CAPACITOR,
        STOP_DISCARD,
        PAUSE_INTERRUPT,
        AUTO_MODE_EXIT,
        AUTO_MODE_EXIT_MIN_DELIVERY_TOO_LONG,
        AUTO_MODE_EXIT_NO_GLUCOSE_3_HOUR,
        AUTO_MODE_EXIT_MAX_DELIVERY_TOO_LONG
    };

    // Allow implicit conversion to uint8_t
    operator uint8_t() const
    {
        return value;
    }

    // Allow implicit conversion from Type
    BasalEndReason(Reason v) : value(v) {}

private:
    Reason value;
};

#endif // MEDTRUM_BASAL_END_REASON_H
