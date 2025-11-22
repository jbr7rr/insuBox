#ifndef MOTOR_H
#define MOTOR_H

#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/stepper.h>
#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

#include <optional>

class IMotorCallback
{
public:
    virtual void onMotorCompleted(float delivered, float position, bool stopped, bool error) = 0;
};

class Motor
{
public:
    Motor(IMotorCallback &callback);
    ~Motor();

    /**
     * @brief deliver the specified number of units, from the current position.
     *
     * @param units The number of units to move.
     * @param speed The speed of the motor. 0 - 100. (0 is slowest possible, 100 is fastest), for bolus it is
     * recommended to use 1 - 10%
     *
     * @return int 0 on success, negative error code on failure.
     */
    int deliver(float units, uint8_t speed = 10, uint8_t powerPct = 100);

    /**
     * @brief Move the motor to the specified position.
     *
     * @details This will move the motor to the specified position. Counting from the retracted position,
     * 0 meansplunger completely retracted.
     *
     * @param units The position to move to, in units. Can be negative in order to correct the position if it is
     * incorrect.
     * @param speed The speed of the motor. 0 - 100. (0 is slowest possible, 100 is fastest)
     * @param powerPct The power percentage to use for the motor. 0 - 150. (0 is off, 100 is nominal, 150 is overdrive)
     *
     * @return int 0 on success, negative error code on failure.
     */
    int moveToPosition(float units, uint8_t speed = 100, uint8_t powerPct = 100);

    /**
     * @brief Stop the motor.
     *
     * @details This will stop the current operation of the motor. It will stop the motor immediately.
     *
     */
    void stop();

    int setPosition(float position);
    std::optional<float> getPosition() const;

private:
    const struct pwm_dt_spec mPwmStepVref = PWM_DT_SPEC_GET(DT_ALIAS(pwm_step_vref));
    const struct device *mStepperDev = DEVICE_DT_GET(DT_ALIAS(stepper));
    std::optional<float> mCurrentPosition = std::nullopt;
    IMotorCallback &mCallback;

    int prepareForMove(uint8_t speed, uint8_t powerPct);

    int setVref(uint8_t powerPct);

    static void drvCallback(const struct device *dev, enum stepper_event event, void *userData);
    int loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param);
};

#endif // MOTOR_H
