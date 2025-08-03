#include <cmath>
#include <pump/insubox/motor/Motor.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_motor);

namespace
{
    constexpr int UNITS_PER_MICRO_STEP = (4 * 190); // 4 microsteps per step, 190 steps per revolution
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
    stepper_set_micro_step_res(mStepperDev, STEPPER_MICRO_STEP_4);
}

Motor::~Motor() {}

int Motor::deliver(float units, uint8_t speed)
{
    if (!mCurrentPosition.has_value())
    {
        LOG_ERR("Current position is not set");
        return -ENOENT;
    }

    if (units < 0 || speed > 100 || speed == 0)
    {
        return -EINVAL;
    }

    LOG_INF("Delivering %.2f units at speed %d", static_cast<double>(units), speed);

    int err = setVref();
    if (err)
    {
        LOG_ERR("Failed to enable Vref: %d", err);
        return err;
    }
    // interval 100000 is the max speed, we take 10000000 as the convenient min
    int interval = 10000000 / speed;
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

    int steps = static_cast<int>(round(units * UNITS_PER_MICRO_STEP));

    LOG_DBG("Moving plunger by %d steps", steps);
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

int Motor::moveToPosition(float units, uint8_t speed)
{
    if (!mCurrentPosition.has_value())
    {
        LOG_ERR("Current position is not set");
        return -ENOENT;
    }

    if (units < 0 || speed > 100 || speed == 0)
    {
        return -EINVAL;
    }

    LOG_INF("Moving to position %.2f at speed %d", static_cast<double>(units), speed);

    int err = setVref(100);
    if (err)
    {
        LOG_ERR("Failed to enable Vref: %d", err);
        return err;
    }
    // interval 100000 is the max speed, we take 10000000 as the convenient min
    int interval = 10000000 / speed;
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

    err = stepper_move_to(mStepperDev, static_cast<int32_t>(round(units * UNITS_PER_MICRO_STEP)));
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
    if (position < 0)
    {
        return -EINVAL;
    }

    LOG_INF("Setting position to %.2f", static_cast<double>(position));

    int err = stepper_set_reference_position(mStepperDev, static_cast<int32_t>(round(position * UNITS_PER_MICRO_STEP)));
    if (err)
    {
        LOG_ERR("Failed to set reference position: %d", err);
        return err;
    }

    mCurrentPosition = position;
    return 0;
}

std::optional<float> Motor::getPosition() const
{
    return mCurrentPosition;
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
        motor->mCurrentPosition = static_cast<float>(stepperPos) / UNITS_PER_MICRO_STEP;
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

    stepper_disable(dev);
    motor->setVref(0);
    motor->mCallback.onMotorCompleted(motor->mCurrentPosition.value() - previousPosition,
                                      motor->mCurrentPosition.value(), stopped, error);
}
