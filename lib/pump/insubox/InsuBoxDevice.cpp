#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_pump_device);

// TODO: move low level stuff to its own files
#include <cmath>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
static const struct pwm_dt_spec pwm_step_vref = PWM_DT_SPEC_GET(DT_ALIAS(pwm_step_vref));

// Stepper device
#include <zephyr/drivers/stepper.h>
static const struct device *stepper_dev = DEVICE_DT_GET(DT_NODELABEL(motor_1));

// Sensor inputs
#define PWN_SENSOR_NODE DT_INST(0, test_pwm_loopback)

#define PWM_BUZZER_OUT_CTRL DT_PWMS_CTLR_BY_IDX(PWN_SENSOR_NODE, 0)
#define PWM_BUZZER_OUT_CHANNEL DT_PWMS_CHANNEL_BY_IDX(PWN_SENSOR_NODE, 0)
#define PWM_BUZZER_OUT_FLAGS DT_PWMS_FLAGS_BY_IDX(PWN_SENSOR_NODE, 0)

#define PWM_SENSOR_0_IN_CTRL DT_PWMS_CTLR_BY_IDX(PWN_SENSOR_NODE, 1)
#define PWM_SENSOR_0_IN_CHANNEL DT_PWMS_CHANNEL_BY_IDX(PWN_SENSOR_NODE, 1)
#define PWM_SENSOR_0_IN_FLAGS DT_PWMS_FLAGS_BY_IDX(PWN_SENSOR_NODE, 1)

#define PWM_SENSOR_1_IN_CTRL DT_PWMS_CTLR_BY_IDX(PWN_SENSOR_NODE, 2)
#define PWM_SENSOR_1_IN_CHANNEL DT_PWMS_CHANNEL_BY_IDX(PWN_SENSOR_NODE, 2)
#define PWM_SENSOR_1_IN_FLAGS DT_PWMS_FLAGS_BY_IDX(PWN_SENSOR_NODE, 2)

// TODO: Move to header or whatever
struct pwm_sensor
{
    const struct device *dev;
    uint32_t pwm;
    pwm_flags_t flags;
};

struct pwm_callback_data
{
    uint32_t *buffer;
    size_t buffer_len;
    size_t count;
    int status;
    struct k_sem sem;
    bool pulse_capture;
};

struct pwm_sensor pwm_buzzer = {
    .dev = DEVICE_DT_GET(PWM_BUZZER_OUT_CTRL),
    .pwm = PWM_BUZZER_OUT_CHANNEL,
    .flags = PWM_BUZZER_OUT_FLAGS,
};

struct pwm_sensor pwm_sensor_0 = {
    .dev = DEVICE_DT_GET(PWM_SENSOR_0_IN_CTRL),
    .pwm = PWM_SENSOR_0_IN_CHANNEL,
    .flags = PWM_SENSOR_0_IN_FLAGS,
};

struct pwm_sensor pwm_sensor_1 = {
    .dev = DEVICE_DT_GET(PWM_SENSOR_1_IN_CTRL),
    .pwm = PWM_SENSOR_1_IN_CHANNEL,
    .flags = PWM_SENSOR_1_IN_FLAGS,
};

// Define note frequencies (in Hz)
enum notes
{
    NOTE_C4 = 261,
    NOTE_D4 = 294,
    NOTE_E4 = 329,
    NOTE_F4 = 349,
    NOTE_G4 = 392,
    NOTE_A4 = 440,
    NOTE_B4 = 494,
    NOTE_C5 = 523,
};

// Example tune: {note, duration_in_ms}
const struct
{
    int frequency;
    int duration;
} tune[] = {
    {NOTE_C4, 1000}, {NOTE_D4, 1000}, {NOTE_E4, 1000}, {NOTE_F4, 1000},
    {NOTE_G4, 1000}, {NOTE_A4, 1000}, {NOTE_B4, 1000}, {NOTE_C5, 1000},
};

static void drv_callback(const struct device *dev, enum stepper_event event, void *dummy)
{
    switch (event)
    {
    case STEPPER_EVENT_STEPS_COMPLETED:
        LOG_DBG("STEPPER_EVENT_STEPS_COMPLETED");
        break;
    case STEPPER_EVENT_LEFT_END_STOP_DETECTED:
        LOG_DBG("STEPPER_EVENT_LEFT_END_STOP_DETECTED");
        break;
    case STEPPER_EVENT_RIGHT_END_STOP_DETECTED:
        LOG_DBG("STEPPER_EVENT_RIGHT_END_STOP_DETECTED");
        break;
    case STEPPER_EVENT_STALL_DETECTED:
        LOG_DBG("STEPPER_EVENT_STALL_DETECTED");
        break;
    default:
        break;
    }
}

InsuBoxDevice::InsuBoxDevice()
{
    LOG_DBG("InsuBoxDevice constructor");

    mSubContainer.mDevice = this;
    k_work_init_delayable(&mSubContainer.sensorWork, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SubContainer, sensorWork);
        container->mDevice->sensorWork();
    });
}

InsuBoxDevice::~InsuBoxDevice()
{
    LOG_DBG("InsuBoxDevice destructor");
}

void movePlunger(double units, bool probe)
{
    // We want the following parameters:
    // Freq: 100kHz
    // Duty cycle: 20% (360mv: I = 360/3 = 120mA)

    uint32_t period = 10000;
    uint32_t pulse = probe ? period / 6 : period / 3;
    int ret = pwm_set_dt(&pwm_step_vref, period, pulse);
    if (ret)
    {
        LOG_ERR("Error %d: failed to set pulse width\n", ret);
        return;
    }

    // stepper_set_reference_position(stepper_dev, 0);
    stepper_set_micro_step_res(stepper_dev, STEPPER_MICRO_STEP_2);

    int interval = probe ? 1000000 : 5000000;
    stepper_set_microstep_interval(stepper_dev, interval);

    stepper_enable(stepper_dev, true);

    // TODO: For now just to show it works
    int steps = static_cast<int>(round(units * 380));

    LOG_DBG("Moving plunger by %d steps", steps);
    // Need to reverse direction, as the motor is mounted in reverse
    stepper_move_by(stepper_dev, -steps);
}

void InsuBoxDevice::init()
{
    LOG_DBG("InsuBoxDevice init");

    int ret = 0;

    k_sleep(K_MSEC(1000));

    if (!pwm_is_ready_dt(&pwm_step_vref))
    {
        LOG_ERR("Error: PWM device %s is not ready\n", pwm_step_vref.dev->name);
        return;
    }

    if (!device_is_ready(pwm_buzzer.dev))
    {
        LOG_ERR("Error: PWM buzzer device is not ready\n");
        return;
    }

    if (!device_is_ready(pwm_sensor_0.dev))
    {
        LOG_ERR("Error: PWM sensor 0 device is not ready\n");
        return;
    }

    if (!device_is_ready(pwm_sensor_1.dev))
    {
        LOG_ERR("Error: PWM sensor 1 device is not ready\n");
        return;
    }

    stepper_set_event_callback(stepper_dev, drv_callback, NULL);

    // TEST BUZZER
    int period = 0;
    int pulse = 0;

    for (size_t i = 0; i < ARRAY_SIZE(tune); i++)
    {
        period = 10000000000 / tune[i].frequency;
        pulse = period / 2;
        ret = pwm_set(pwm_buzzer.dev, pwm_buzzer.pwm, period, pulse, pwm_buzzer.flags);
        if (ret)
        {
            LOG_ERR("Error %d: failed to set pulse width\n", ret);
            return;
        }

        k_sleep(K_MSEC(tune[i].duration));
    }

    pwm_set(pwm_buzzer.dev, pwm_buzzer.pwm, period, 0, pwm_buzzer.flags);

    k_work_reschedule(&mSubContainer.sensorWork, K_NO_WAIT);

    // movePlunger(10, false);
    // k_sleep(K_SECONDS(30));
    // movePlunger(-350, true);
    // k_sleep(K_SECONDS(90));
    movePlunger(300, true);
    // k_sleep(K_SECONDS(30));
    // movePlunger(10, false);
}

void InsuBoxDevice::sensorWork()
{
    // Test PWM sensor
    uint64_t period_capture_0 = 0;
    uint64_t pulse_capture_0 = 0;
    uint64_t period_capture_1 = 0;
    uint64_t pulse_capture_1 = 0;
    int ret = 0;

    ret = pwm_capture_nsec(pwm_sensor_0.dev, pwm_sensor_0.pwm, (PWM_CAPTURE_TYPE_BOTH | pwm_sensor_0.flags),
                           &period_capture_0, &pulse_capture_0, K_MSEC(10));
    if (ret)
    {
        LOG_ERR("Error %d: failed to capture PWM sensor 0\n", ret);
    }

    ret = pwm_capture_nsec(pwm_sensor_1.dev, pwm_sensor_1.pwm, (PWM_CAPTURE_TYPE_BOTH | pwm_sensor_0.flags),
                           &period_capture_1, &pulse_capture_1, K_MSEC(10));
    if (ret)
    {
        LOG_ERR("Error %d: failed to capture PWM sensor 1\n", ret);
    }

    double duty_cycle_0 = (float)pulse_capture_0 / (float)period_capture_0;
    double duty_cycle_1 = (float)pulse_capture_1 / (float)period_capture_1;
    LOG_DBG("PWM sensor 0: %f, PWM sensor 1: %f", duty_cycle_0, duty_cycle_1);
    k_work_reschedule(&mSubContainer.sensorWork, K_MSEC(500));
}
