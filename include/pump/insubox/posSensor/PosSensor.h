#ifndef POS_SENSOR_H
#define POS_SENSOR_H

#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include <optional>

class PosSensor
{
public:
    PosSensor();
    ~PosSensor();

    std::optional<float> getPosition() const;

private:
    const struct device *mTopSensor = DEVICE_DT_GET(DT_ALIAS(mag_top));
    const struct device *mBottomSensor = DEVICE_DT_GET(DT_ALIAS(mag_bottom));

    void readSensor(const struct device *sensor);
};

#endif // POS_SENSOR_H
