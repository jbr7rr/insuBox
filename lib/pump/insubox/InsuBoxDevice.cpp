#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_pump_device);

#include "motor/Motor.h"

// TODO: move low level stuff to its own files
#include <cmath>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

// Sensor devices
static const struct device *sensor0 = DEVICE_DT_GET(DT_ALIAS(magn0));
static const struct device *sensor1 = DEVICE_DT_GET(DT_ALIAS(magn1));

InsuBoxDevice::InsuBoxDevice() : mMotor(createMotorInstance())
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

void InsuBoxDevice::init()
{
    LOG_DBG("InsuBoxDevice init");

    // TODO: Store position in flash, and procedure to find the zero position
    mMotor.setPosition(0.0f);

    k_work_reschedule(&mSubContainer.sensorWork, K_NO_WAIT);
}

void InsuBoxDevice::onBolusRequest(float amount, time_t timestamp)
{
    LOG_DBG("Bolus request: %.2f units at %lld", static_cast<double>(amount), timestamp);
    int err = mMotor.deliver(amount, 10);
    if (err)
    {
        LOG_ERR("Failed to deliver bolus: %d", err);
        // TODO: event
        return;
    }
}

void InsuBoxDevice::onStopBolus()
{
    LOG_DBG("Stop bolus");
    LOG_WRN("Stop bolus not implemented yet");
    mMotor.stop();
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

Motor &InsuBoxDevice::createMotorInstance()
{
    static Motor motor;
    return motor;
}