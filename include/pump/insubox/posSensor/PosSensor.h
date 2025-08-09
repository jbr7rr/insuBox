#ifndef POS_SENSOR_H
#define POS_SENSOR_H

#include <zephyr/device.h>
#include <zephyr/settings/settings.h>

#include <array>
#include <optional>

class PosSensor
{
public:
    PosSensor();
    ~PosSensor();

    std::optional<float> getPosition() const;

    /**
     * @brief Reads the top and bottom sensors and returns their values.
     * @param position The position to store in the lookup table (LUT).
     *
     * Only whole numbers are allowed for input, as the LUT is indexed by integers.
     *
     * @return True if the position was successfully stored in the LUT, false otherwise.
     */
    bool storePositionToLUT(int position);

private:
    const struct device *mTopSensor = DEVICE_DT_GET(DT_ALIAS(mag_top));
    const struct device *mBottomSensor = DEVICE_DT_GET(DT_ALIAS(mag_bottom));

    // Y is disregarded, as it has a very low signal
    struct SensorVals
    {
        int16_t x1;
        int16_t z1;
        int16_t x2;
        int16_t z2;
    };
    std::array<SensorVals, CONFIG_IB_PUMP_RESERVOIR_VOLUME + 1> mSensorLUT = {0};
    bool mLUTReady = false;

    std::pair<int16_t, int16_t> readSensor(const struct device *sensor) const;
    int loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param);
};

#endif // POS_SENSOR_H
