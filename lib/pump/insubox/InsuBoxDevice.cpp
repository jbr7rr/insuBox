#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_pump_device);

// TODO: move low level stuff to its own files
#include <cmath>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/stepper.h>
#include <zephyr/kernel.h>

// Stepper device
static const struct pwm_dt_spec pwm_step_vref = PWM_DT_SPEC_GET(DT_ALIAS(pwm_step_vref));
static const struct device *stepper_dev = DEVICE_DT_GET(DT_ALIAS(stepper));

// Sensor devices
static const struct device *sensor0 = DEVICE_DT_GET(DT_ALIAS(magn0));
static const struct device *sensor1 = DEVICE_DT_GET(DT_ALIAS(magn1));

static void drv_callback(const struct device *dev, enum stepper_event event, void *dummy)
{
    LOG_DBG("drv_callback: %p, event: %d", dev, event);
    switch (event)
    {
    case STEPPER_EVENT_STEPS_COMPLETED:
        LOG_DBG("STEPPER_EVENT_STEPS_COMPLETED");
        stepper_disable(dev);
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
    uint32_t pulse = probe ? period / 3 : period / 3;
    int ret = pwm_set_dt(&pwm_step_vref, period, pulse);
    if (ret)
    {
        LOG_ERR("Error %d: failed to set pulse width", ret);
        return;
    }

    // stepper_set_reference_position(stepper_dev, 0);
    stepper_set_micro_step_res(stepper_dev, STEPPER_MICRO_STEP_4);

    int interval = probe ? 100000 : 500000;
    stepper_set_microstep_interval(stepper_dev, interval);

    stepper_enable(stepper_dev);
    stepper_set_event_callback(stepper_dev, drv_callback, NULL);
    int steps = static_cast<int>(round(units * 2 * 380));

    LOG_DBG("Moving plunger by %d steps", steps);
    stepper_move_by(stepper_dev, steps);
}

void InsuBoxDevice::init()
{
    LOG_DBG("InsuBoxDevice init");
    if (!pwm_is_ready_dt(&pwm_step_vref))
    {
        LOG_ERR("Error: PWM device %s is not ready", pwm_step_vref.dev->name);
        return;
    }
    k_work_reschedule(&mSubContainer.sensorWork, K_NO_WAIT);
}

void InsuBoxDevice::onBolusRequest(float amount, time_t timestamp)
{
    LOG_DBG("Bolus request: %.2f units at %lld", static_cast<double>(amount), timestamp);
    movePlunger(amount, false);
    // TODO: Update etc
}

void InsuBoxDevice::onStopBolus()
{
    LOG_DBG("Stop bolus");
    LOG_WRN("Stop bolus not implemented yet");
}

void read_sensor(const struct device *sensor)
{
    if (!device_is_ready(sensor))
    {
        LOG_ERR("Device %s is not ready", sensor->name);
        return;
    }

    struct sensor_value mag_x, mag_y, mag_z;

    if (sensor_sample_fetch(sensor) < 0)
    {
        LOG_ERR("Failed to fetch samples");
        return;
    }

    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_X, &mag_x);
    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_Y, &mag_y);
    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_Z, &mag_z);

    LOG_DBG("%s Magnetic field (uT): X=%d, Y=%d, Z=%d", sensor->name, mag_x.val1, mag_y.val1, mag_z.val1);
}

void InsuBoxDevice::sensorWork()
{
    read_sensor(sensor0);
    read_sensor(sensor1);

    k_work_reschedule(&mSubContainer.sensorWork, K_MSEC(10000));
}
