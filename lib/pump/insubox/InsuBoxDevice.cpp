#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_pump_device);

// TODO: move low level stuff to its own files
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>
static const struct pwm_dt_spec pwm_step_vref = PWM_DT_SPEC_GET(DT_ALIAS(pwm_step_vref));

// Stepper device
#include <zephyr/drivers/stepper.h>
static const struct device *stepper_dev = DEVICE_DT_GET(DT_NODELABEL(motor_1));

static void drv_callback(const struct device *dev, enum stepper_event event,
					     void *dummy)
{
	switch (event) {
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
}

InsuBoxDevice::~InsuBoxDevice()
{
    LOG_DBG("InsuBoxDevice destructor");
}

void InsuBoxDevice::init()
{
    LOG_DBG("InsuBoxDevice init");

    // k_sleep(K_MSEC(1000));

    if (!pwm_is_ready_dt(&pwm_step_vref))
    {
        LOG_ERR("Error: PWM device %s is not ready\n", pwm_step_vref.dev->name);
        return;
    }

    // We want the following parameters:
    // Freq: 100kHz
    // Duty cycle: 20% (360mv: I = 360/3 = 120mA)

    uint32_t period = 10000;
    uint32_t pulse = 2000;
    int ret = pwm_set_dt(&pwm_step_vref, period, pulse);
    if (ret)
    {
        LOG_ERR("Error %d: failed to set pulse width\n", ret);
        return;
    }

    LOG_DBG("PWM device %s is ready\n", pwm_step_vref.dev->name);

    stepper_enable(stepper_dev, true);
    stepper_set_max_velocity(stepper_dev, 1000u);
    stepper_set_event_callback(stepper_dev, drv_callback, NULL);

    // TODO: For now just to show it works
    stepper_move_by(stepper_dev, 6000);
}
