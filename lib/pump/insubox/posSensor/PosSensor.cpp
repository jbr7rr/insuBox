#include <pump/insubox/posSensor/PosSensor.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <cmath>
#include <limits>
#include <string>

LOG_MODULE_REGISTER(PosSensor, LOG_LEVEL_DBG);

namespace
{
    constexpr char settingsSubKey[] = "ibs";
    constexpr char settingsPosLUTKey[] = "posLUT";
}

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
    // TODO: Load and validate LUT from persistent storage if available
    settings_subsys_init();
    settings_load_subtree_direct(
        settingsSubKey,
        [](const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param) {
            return static_cast<PosSensor *>(param)->loadCb(key, len, read_cb, cb_arg, param);
        },
        this);
}

PosSensor::~PosSensor() {}

std::optional<float> PosSensor::getPosition() const
{
    if (!mLUTReady)
    {
        LOG_ERR("LUT is not ready, cannot get position");
        return std::nullopt;
    }

    auto currentTopVals = readSensor(mTopSensor);
    auto currentBottomVals = readSensor(mBottomSensor);

    LOG_DBG("Current sensor values - Top: X=%d, Z=%d; Bottom: X=%d, Z=%d", currentTopVals.first, currentTopVals.second,
            currentBottomVals.first, currentBottomVals.second);

    if (currentTopVals == std::pair<int16_t, int16_t>{0, 0} || currentBottomVals == std::pair<int16_t, int16_t>{0, 0})
    {
        LOG_ERR("Failed to read sensor values");
        return std::nullopt;
    }

    SensorVals currentVals = {currentTopVals.first, currentTopVals.second, currentBottomVals.first,
                              currentBottomVals.second};

    // Find the nearest key in the LUT
    int nearestIndex = -1;
    float nearestDistance = std::numeric_limits<float>::max();

    for (size_t i = 0; i < mSensorLUT.size(); ++i)
    {
        const auto &lutVals = mSensorLUT[i];
        float distance = std::sqrt(std::pow(currentVals.x1 - lutVals.x1, 2) + std::pow(currentVals.z1 - lutVals.z1, 2) +
                                   std::pow(currentVals.x2 - lutVals.x2, 2) + std::pow(currentVals.z2 - lutVals.z2, 2));

        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearestIndex = static_cast<int>(i);
        }
    }

    if (nearestIndex == -1)
    {
        LOG_ERR("No nearest position found in LUT");
        return std::nullopt;
    }

    int secondNearestIndex = -1;
    float secondNearestDistance = std::numeric_limits<float>::max();

    // Find second-nearest
    for (size_t i = 0; i < mSensorLUT.size(); ++i)
    {
        if (static_cast<int>(i) == nearestIndex)
            continue;

        const auto &lutVals = mSensorLUT[i];
        float distance = std::sqrt(std::pow(currentVals.x1 - lutVals.x1, 2) + std::pow(currentVals.z1 - lutVals.z1, 2) +
                                   std::pow(currentVals.x2 - lutVals.x2, 2) + std::pow(currentVals.z2 - lutVals.z2, 2));

        if (distance < secondNearestDistance)
        {
            secondNearestDistance = distance;
            secondNearestIndex = static_cast<int>(i);
        }
    }

    if (secondNearestIndex == -1)
    {
        LOG_ERR("No second-nearest position found in LUT");
        return static_cast<float>(nearestIndex);
    }

    LOG_DBG("Nearest position index: %d, distance: %f", nearestIndex, static_cast<double>(nearestDistance));
    LOG_DBG("Second nearest position index: %d, distance: %f", secondNearestIndex,
            static_cast<double>(secondNearestDistance));

    // Inverse distance weighting
    float d1 = nearestDistance;
    float d2 = secondNearestDistance;

    float w1 = (d1 + d2 - d1) / (d1 + d2); // same as d2 / (d1 + d2)
    float w2 = 1.0f - w1;

    LOG_DBG("Weights: w1 = %f, w2 = %f", static_cast<double>(w1), static_cast<double>(w2));

    float interpolatedPosition = nearestIndex * w1 + secondNearestIndex * w2;
    LOG_DBG("Interpolated position: %f", static_cast<double>(interpolatedPosition));
    return interpolatedPosition;
}

bool PosSensor::storePositionToLUT(int position)
{
    if (position < 0 || position >= static_cast<int>(mSensorLUT.size()))
    {
        LOG_ERR("Position %d is out of bounds for LUT", position);
        return false;
    }

    auto topVals = readSensor(mTopSensor);
    auto bottomVals = readSensor(mBottomSensor);

    mSensorLUT[position] = {topVals.first, topVals.second, bottomVals.first, bottomVals.second};

    LOG_DBG("Stored position %d: Top(X=%d, Z=%d), Bottom(X=%d, Z=%d)", position, topVals.first, topVals.second,
            bottomVals.first, bottomVals.second);

    if (position == 300)
    {
        // Save the LUT to persistent storage
        std::string key = std::string(settingsSubKey) + "/" + settingsPosLUTKey;
        if (settings_save_one(key.c_str(), mSensorLUT.data(), mSensorLUT.size() * sizeof(SensorVals)) < 0)
        {
            LOG_ERR("Failed to save position LUT to settings");
            return false;
        }
        mLUTReady = true;
        LOG_INF("Position sensor LUT saved successfully");
    }

    return true;
}

std::pair<int16_t, int16_t> PosSensor::readSensor(const struct device *sensor) const
{
    if (!device_is_ready(sensor))
    {
        LOG_ERR("Device %s is not ready", sensor->name);
        return {0, 0};
    }

    if (sensor_sample_fetch(sensor) < 0)
    {
        LOG_ERR("Failed to fetch samples");
        return {0, 0};
    }

    struct sensor_value mag_x, mag_z;
    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_X, &mag_x);
    sensor_channel_get(sensor, SENSOR_CHAN_MAGN_Z, &mag_z);

    return {mag_x.val1, mag_z.val1};
}

int PosSensor::loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
{
    if (strcmp(key, settingsPosLUTKey) == 0)
    {
        LOG_DBG("Loading position LUT from settings");
        size_t len = read_cb(cb_arg, &mSensorLUT[0], mSensorLUT.size() * sizeof(SensorVals));
        if (len != mSensorLUT.size() * sizeof(SensorVals))
        {
            LOG_ERR("Failed to read position LUT from settings");
            return -EIO;
        }
        mLUTReady = true;
        LOG_INF("Position sensor LUT loaded successfully");
        return 0;
    }

    return -ENOENT; // Key not found
}
