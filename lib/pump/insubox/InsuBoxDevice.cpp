#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_pump_device);

// TODO: move low level stuff to its own files
#include <cmath>
#include <zephyr/drivers/pwm.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

// Stepper device
#include <zephyr/drivers/stepper.h>
static const struct pwm_dt_spec pwm_step_vref = PWM_DT_SPEC_GET(DT_ALIAS(pwm_step_vref));
static const struct device *stepper_dev = DEVICE_DT_GET(DT_NODELABEL(motor_1));

// Sensor devices
static const struct device *sensor0 = DEVICE_DT_GET(DT_ALIAS(magn0));
static const struct device *sensor1 = DEVICE_DT_GET(DT_ALIAS(magn1));

// Buzzer device
static const struct pwm_dt_spec pwm_buzzer = PWM_DT_SPEC_GET(DT_ALIAS(pwm_buzzer));

#include <zephyr/drivers/gpio.h>
// Buttons (sw1 - sw3)
static const struct gpio_dt_spec button_sw1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw1), gpios, {0});
static const struct gpio_dt_spec button_sw2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw2), gpios, {0});
static const struct gpio_dt_spec button_sw3 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw3), gpios, {0});

// Stat1, stat2
static const struct gpio_dt_spec stat1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(stat1), gpios, {0});
static const struct gpio_dt_spec stat2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(stat2), gpios, {0});

// Define note frequencies (in Hz)
enum notes
{
    NOTE_C6 = 1047,
    NOTE_D6 = 1175,
    NOTE_E6 = 1319,
    NOTE_F6 = 1397,
    NOTE_G6 = 1568,
    NOTE_A6 = 1760,
    NOTE_B6 = 1976,
    NOTE_C7 = 2093,
    NOTE_D7 = 2349,
    NOTE_E7 = 2637,
    NOTE_F7 = 2794,
    NOTE_G7 = 3136,
    NOTE_A7 = 3520,
    NOTE_B7 = 3951,
    NOTE_C8 = 4186,
};

// Example tune: {note, duration_in_ms}
const struct
{
    int frequency;
    int duration;
} tune[] = {
    {NOTE_C6, 500}, {NOTE_E6, 500}, {NOTE_G6, 500}, {NOTE_B6, 500},
    {NOTE_D7, 500}, {NOTE_F7, 500}, {NOTE_A7, 500}, {NOTE_C8, 500},
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

// static void gpio_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
// {
//     // Print the state of the button, and name of the button
//     LOG_DBG("Button %s pressed", dev->name);
// }

static void setup_gpio_demo()
{
    // Initialize buttons
    if (!gpio_is_ready_dt(&button_sw1))
    {
        LOG_ERR("Error: GPIO device %s is not ready\n", button_sw1.port->name);
        return;
    }
    if (!gpio_is_ready_dt(&button_sw2))
    {
        LOG_ERR("Error: GPIO device %s is not ready\n", button_sw2.port->name);
        return;
    }
    if (!gpio_is_ready_dt(&button_sw3))
    {
        LOG_ERR("Error: GPIO device %s is not ready\n", button_sw3.port->name);
        return;
    }
    if (!gpio_is_ready_dt(&stat1))
    {
        LOG_ERR("Error: GPIO device %s is not ready\n", stat1.port->name);
        return;
    }
    if (!gpio_is_ready_dt(&stat2))
    {
        LOG_ERR("Error: GPIO device %s is not ready\n", stat2.port->name);
        return;
    }

    int err = gpio_pin_configure_dt(&button_sw1, GPIO_INPUT);
    if (err)
    {
        LOG_ERR("Error %d: failed to configure pin %d\n", err, button_sw1.pin);
        return;
    }
    err = gpio_pin_configure_dt(&button_sw2, GPIO_INPUT);
    if (err)
    {
        LOG_ERR("Error %d: failed to configure pin %d\n", err, button_sw2.pin);
        return;
    }
    err = gpio_pin_configure_dt(&button_sw3, GPIO_INPUT);
    if (err)
    {
        LOG_ERR("Error %d: failed to configure pin %d\n", err, button_sw3.pin);
        return;
    }
    err = gpio_pin_configure_dt(&stat1, GPIO_INPUT);
    if (err)
    {
        LOG_ERR("Error %d: failed to configure pin %d\n", err, stat1.pin);
        return;
    }
    err = gpio_pin_configure_dt(&stat2, GPIO_INPUT);
    if (err)
    {
        LOG_ERR("Error %d: failed to configure pin %d\n", err, stat2.pin);
        return;
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
    uint32_t pulse = probe ? period / 5 : period / 3;
    int ret = pwm_set_dt(&pwm_step_vref, period, pulse);
    if (ret)
    {
        LOG_ERR("Error %d: failed to set pulse width\n", ret);
        return;
    }

    // stepper_set_reference_position(stepper_dev, 0);
    stepper_set_micro_step_res(stepper_dev, STEPPER_MICRO_STEP_4);

    int interval = probe ? 250000 : 1000000;
    stepper_set_microstep_interval(stepper_dev, interval);

    stepper_enable(stepper_dev);

    // TODO: For now just to show it works
    int steps = static_cast<int>(round(units * 2 * 380));

    LOG_DBG("Moving plunger by %d steps", steps);
    // Need to reverse direction, as the motor is mounted in reverse
    stepper_move_by(stepper_dev, steps);
}

void InsuBoxDevice::init()
{
    LOG_DBG("InsuBoxDevice init");


    k_sleep(K_MSEC(1000));

    if (!pwm_is_ready_dt(&pwm_step_vref))
    {
        LOG_ERR("Error: PWM device %s is not ready\n", pwm_step_vref.dev->name);
        return;
    }

    if (!pwm_is_ready_dt(&pwm_buzzer))
    {
        LOG_ERR("Error: PWM device %s is not ready\n", pwm_buzzer.dev->name);
        return;
    }

    stepper_set_event_callback(stepper_dev, drv_callback, NULL);

    // // TEST BUZZER
    // int ret = 0;
    // int period = 5000;
    // int pulse = 0;

    // for (size_t i = 0; i < ARRAY_SIZE(tune); i++)
    // {
    //     period = 1000000000 / tune[i].frequency;
    //     pulse = period / 2;
    //     LOG_DBG("Playing note %d, period %d, pulse %d", tune[i].frequency, period, pulse);
    //     ret = pwm_set_dt(&pwm_buzzer, period, pulse);
    //     if (ret)
    //     {
    //         LOG_ERR("Error %d: failed to set pulse width\n", ret);
    //         return;
    //     }

    //     k_sleep(K_MSEC(tune[i].duration));
    // }

    // pwm_set_dt(&pwm_buzzer, period, 0);


    // movePlunger(10, false);
    // k_sleep(K_SECONDS(30));
    // movePlunger(-350, true);
    // k_sleep(K_SECONDS(90));
    movePlunger(30, false);
    // k_sleep(K_SECONDS(30));
    // movePlunger(10, false);

    // setup_gpio_demo();
    // k_work_reschedule(&mSubContainer.sensorWork, K_NO_WAIT);
}

void read_sensor(const struct device *sensor) {
    if (!device_is_ready(sensor)) {
        printk("Device %s is not ready\n", sensor->name);
        return;
    }

    struct sensor_value mag_x, mag_y, mag_z;

    if (sensor_sample_fetch(sensor) < 0) {
        printk("Failed to fetch samples\n");
        return;
    }

    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_X, &mag_x);
    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_Y, &mag_y);
    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_Z, &mag_z);

    printk("%s Magnetic field (uT): X=%d, Y=%d, Z=%d\n",
           sensor->name, mag_x.val1, mag_y.val1, mag_z.val1);
}

void InsuBoxDevice::sensorWork()
{
    read_sensor(sensor0);
    read_sensor(sensor1);

    int val = 0;
    val = gpio_pin_get_dt(&button_sw1);
    LOG_DBG("Button sw1 state: %d", val);
    val = gpio_pin_get_dt(&button_sw2);
    LOG_DBG("Button sw2 state: %d", val);
    val = gpio_pin_get_dt(&button_sw3);
    LOG_DBG("Button sw3 state: %d", val);

    val = gpio_pin_get_dt(&stat1);
    LOG_DBG("Button stat1 state: %d", val);
    val = gpio_pin_get_dt(&stat2);
    LOG_DBG("Button stat2 state: %d", val);

    k_work_reschedule(&mSubContainer.sensorWork, K_MSEC(1000));
}
