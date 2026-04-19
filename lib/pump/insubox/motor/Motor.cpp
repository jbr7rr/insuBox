#include <cmath>
#include <pump/insubox/motor/Motor.h>
#include <string>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_motor);

namespace
{
    // Motor settings, for motor: m3, ratio 1:298
    constexpr auto STEPPER_MICRO_STEP = STEPPER_MICRO_STEP_1;
#ifdef CONFIG_IB_PUMP_INSUBOX_MOTOR_M3_1_298
    constexpr auto INTERVAL_MAX = (15000000 / STEPPER_MICRO_STEP);
    constexpr auto INTERVAL_MIN = (750000 / STEPPER_MICRO_STEP);
    constexpr auto FULL_STEPS_PER_UNIT = 1144;
    constexpr auto STEPS_PER_UNIT = (STEPPER_MICRO_STEP * FULL_STEPS_PER_UNIT);
#elif defined(CONFIG_IB_PUMP_INSUBOX_MOTOR_M3_1_50)
#warn "This motor is very likely to skip steps, use only for testing!"
    // Note: This motor skips steps very easily under load, maybe one can fine tune the settings to get it to work
    // properly, but that is unlikely. For now we go with the slower 1:298 motor
    constexpr auto INTERVAL_MAX = (90000000 / STEPPER_MICRO_STEP);
    constexpr auto INTERVAL_MIN = (500000 / STEPPER_MICRO_STEP);
    constexpr auto FULL_STEPS_PER_UNIT = 192;
    constexpr auto STEPS_PER_UNIT = (STEPPER_MICRO_STEP * FULL_STEPS_PER_UNIT);
#else
#error "No motor configuration selected"
#endif

    constexpr char settingsSubKey[] = "ib";
    constexpr char settingsPlungerPosKey[] = "plPos";
}

Motor::Motor(IMotorCallback &callback) : mCallback(callback)
{
    if (!device_is_ready(mPwmStepVref.dev))
    {
        LOG_ERR("PWM device not ready");
    }

    if (!device_is_ready(mStepperDev))
    {
        LOG_ERR("Stepper device not ready");
    }

    stepper_set_event_callback(mStepperDev, drvCallback, this);
    stepper_set_micro_step_res(mStepperDev, STEPPER_MICRO_STEP);

    settings_subsys_init();
    settings_load_subtree_direct(
        settingsSubKey,
        [](const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param) {
            return static_cast<Motor *>(param)->loadCb(key, len, read_cb, cb_arg, param);
        },
        this);
}

Motor::~Motor() {}

int Motor::deliver(float units, uint8_t speed, uint8_t powerPct)
{
    if (units < 0)
    {
        return -EINVAL;
    }

    int err = prepareForMove(speed, powerPct);
    if (err)
    {
        return err;
    }

    int steps = static_cast<int>(round(units * STEPS_PER_UNIT));

    LOG_INF("Delivering %.2f units at speed %d with power %d%%", static_cast<double>(units), speed, powerPct);
    err = stepper_move_by(mStepperDev, steps);
    if (err)
    {
        LOG_ERR("Failed to move motor: %d", err);
        stepper_disable(mStepperDev);
        setVref(0);
        return err;
    }

    return 0;
}

int Motor::moveToPosition(float units, uint8_t speed, uint8_t powerPct)
{
    int err = prepareForMove(speed, powerPct);
    if (err)
    {
        return err;
    }

    LOG_INF("Moving to position %.2f at speed %d with power %d%%", static_cast<double>(units), speed, powerPct);
    err = stepper_move_to(mStepperDev, static_cast<int32_t>(round(units * STEPS_PER_UNIT)));
    if (err)
    {
        LOG_ERR("Failed to move motor: %d", err);
        stepper_disable(mStepperDev);
        setVref(0);
        return err;
    }

    return 0;
}

void Motor::stop()
{
    // Stop the motor immediately
    LOG_INF("Stopping motor");

    int err = stepper_stop(mStepperDev);
    if (err)
    {
        LOG_ERR("Failed to stop motor: %d", err);
        return;
    }
    return;
}

int Motor::setPosition(float position)
{
    LOG_INF("Setting position to %.2f", static_cast<double>(position));

    int err = stepper_set_reference_position(mStepperDev, static_cast<int32_t>(round(position * STEPS_PER_UNIT)));
    if (err)
    {
        LOG_ERR("Failed to set reference position: %d", err);
        return err;
    }

    mCurrentPosition = position;
    std::string key = std::string(settingsSubKey) + "/" + settingsPlungerPosKey;
    settings_save_one(key.c_str(), &mCurrentPosition, sizeof(mCurrentPosition));
    return 0;
}

std::optional<float> Motor::getPosition() const
{
    return mCurrentPosition;
}

int Motor::prepareForMove(uint8_t speed, uint8_t powerPct)
{
    if (!mCurrentPosition.has_value())
    {
        LOG_ERR("Current position is not set");
        return -ENOENT;
    }

    if (speed > 100 || speed == 0 || powerPct > 150 || powerPct == 0)
    {
        LOG_ERR("Invalid input params: speed %d, powerPct %d", speed, powerPct);
        return -EINVAL;
    }

    int err = setVref(powerPct);
    if (err)
    {
        LOG_ERR("Failed to enable Vref: %d", err);
        return err;
    }

    int interval = INTERVAL_MIN + (INTERVAL_MAX - INTERVAL_MIN) * (100 - speed) / 100;
    err = stepper_set_microstep_interval(mStepperDev, interval);
    if (err)
    {
        LOG_ERR("Failed to set microstep interval: %d", err);
        setVref(0);
        return err;
    }

    err = stepper_enable(mStepperDev);
    if (err)
    {
        LOG_ERR("Failed to enable motor: %d", err);
        setVref(0);
        return err;
    }

    return 0;
}

int Motor::setVref(uint8_t powerPct)
{
    // Allow some overdrive
    if (powerPct > 150)
    {
        LOG_ERR("Power percentage out of range");
        return -EINVAL;
    }
    /**
     * From the DRV8428 datasheet:
     * The chopping current (IFS) can be calculated as IFS (A) = VREF (V) / KV (V/A) = VREF (V) / 3 (V/A).
     * We have a voltage divider which halves the vdd (which is 3v3) to 1.65.
     * So 100% Duty cycle = 1.65 / 3 = 0.55A
     *
     * The motor is rated for 160mA per phase
     * So we can set the current to 160mA / 0.55A = 29% of the max current.
     *
     * PWM Freq: 100kHz
     */

    constexpr float DUTY_CYCLE_MAX_RATED_CURRENT = 0.3f;
    float dutyCycleFactor = powerPct * DUTY_CYCLE_MAX_RATED_CURRENT / 100.0f;

    uint32_t period = 10000;
    uint32_t pulse = static_cast<uint32_t>((period * dutyCycleFactor));
    int ret = pwm_set_dt(&mPwmStepVref, period, pulse);
    if (ret)
    {
        return ret;
    }
    return 0;
}

void Motor::drvCallback(const struct device *dev, enum stepper_event event, void *userData)
{
    // Handle motor driver events
    LOG_INF("Driver callback triggered with event %d", event);

    Motor *motor = static_cast<Motor *>(userData);
    if (motor == nullptr)
    {
        LOG_ERR("Motor ptr is null");
        return;
    }

    bool stopped = false;
    bool error = false;
    float previousPosition = motor->mCurrentPosition.value();
    int32_t stepperPos = 0;
    int err = stepper_get_actual_position(dev, &stepperPos);
    if (err)
    {
        LOG_ERR("Failed to get actual position: %d", err);
        error = true;
    }
    else
    {
        motor->mCurrentPosition = static_cast<float>(stepperPos) / STEPS_PER_UNIT;
        std::string key = std::string(settingsSubKey) + "/" + settingsPlungerPosKey;
        settings_save_one(key.c_str(), &motor->mCurrentPosition, sizeof(motor->mCurrentPosition));
    }

    switch (event)
    {
    case STEPPER_EVENT_STEPS_COMPLETED:
        LOG_DBG("STEPPER_EVENT_STEPS_COMPLETED");
        break;
    case STEPPER_EVENT_STALL_DETECTED:
        LOG_ERR("STEPPER_EVENT_STALL_DETECTED");
        error = true;
        break;
    case STEPPER_EVENT_LEFT_END_STOP_DETECTED:
        LOG_DBG("STEPPER_EVENT_LEFT_END_STOP_DETECTED");
        break;
    case STEPPER_EVENT_RIGHT_END_STOP_DETECTED:
        LOG_DBG("STEPPER_EVENT_RIGHT_END_STOP_DETECTED");
        break;
    case STEPPER_EVENT_STOPPED:
        LOG_DBG("STEPPER_EVENT_STOPPED");
        stopped = true;
        break;
    case STEPPER_EVENT_FAULT_DETECTED:
        LOG_ERR("STEPPER_EVENT_FAULT_DETECTED");
        error = true;
        break;
    }

    // TODO: Maybe separate enable/disable methods?
    stepper_disable(dev);
    motor->setVref(0);
    motor->mCallback.onMotorCompleted(motor->mCurrentPosition.value() - previousPosition,
                                      motor->mCurrentPosition.value(), stopped, error);
}

int Motor::loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
{
    // Load the settings from the settings subsystem
    if (strcmp(key, settingsPlungerPosKey) == 0)
    {
        LOG_DBG("Loading plunger position");
        float position;
        int len = read_cb(cb_arg, &position, sizeof(position));
        if (len != sizeof(position))
        {
            LOG_ERR("Failed to read plunger position from settings");
            return -1;
        }
        setPosition(position);
    }
    else
    {
        LOG_ERR("Unknown key: %s", key);
        return -1;
    }

    return 0;
}
