#include <cstdint>

class PumpState
{
public:
    enum Type : uint8_t
    {
        NONE = 0,
        IDLE = 1,
        FILLED = 2,
        PRIMING = 3,
        PRIMED = 4,
        EJECTING = 5,
        EJECTED = 6,
        ACTIVE = 32,
        ACTIVE_ALT = 33,
        LOW_BG_SUSPENDED = 64,
        LOW_BG_SUSPENDED2 = 65,
        AUTO_SUSPENDED = 66,
        HOURLY_MAX_SUSPENDED = 67,
        DAILY_MAX_SUSPENDED = 68,
        SUSPENDED = 69,
        PAUSED = 70,
        OCCLUSION = 96,
        EXPIRED = 97,
        RESERVOIR_EMPTY = 98,
        PATCH_FAULT = 99,
        PATCH_FAULT2 = 100,
        BASE_FAULT = 101,
        BATTERY_OUT = 102,
        NO_CALIBRATION = 103,
        STOPPED = 128
    };

    PumpState() : value(NONE) {}

    // Allow implicit conversion to uint8_t
    operator uint8_t() const
    {
        return value;
    }

    // Allow implicit conversion from Type
    PumpState(Type v) : value(v) {}

    // Allow implicit conversion from uint8_t
    PumpState(uint8_t v) : value(static_cast<Type>(v)) {}

private:
    Type value;
};