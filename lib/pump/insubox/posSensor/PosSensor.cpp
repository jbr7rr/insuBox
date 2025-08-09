#include <pump/insubox/posSensor/PosSensor.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(PosSensor, LOG_LEVEL_DBG);

PosSensor::PosSensor()
{
    if (!device_is_ready(mTopSensor))
    {
        LOG_ERR("Top sensor is not ready");
    }

    if (!device_is_ready(mBottomSensor))
    {
        LOG_ERR("Bottom sensor is not ready");
    }
}

PosSensor::~PosSensor() {}

std::optional<float> PosSensor::getPosition() const
{
    return std::nullopt; // Placeholder for actual position logic
}

void PosSensor::readSensor(const struct device *sensor)
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
