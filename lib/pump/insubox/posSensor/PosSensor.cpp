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

    SensorVals currentVals = readSensors();

    LOG_DBG("Current sensor values - Top: X=%d, Z=%d; Bottom: X=%d, Z=%d", currentVals.x1, currentVals.z1,
            currentVals.x2, currentVals.z2);

    if ((currentVals.x1 == 0 && currentVals.z1 == 0) || (currentVals.x2 == 0 && currentVals.z2 == 0))
    {
        LOG_ERR("Failed to read sensor values");
        return std::nullopt;
    }

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

    if (d1 == 0.0f)
    {
        LOG_DBG("Exact LUT match found at index: %d", nearestIndex);
        return static_cast<float>(nearestIndex);
    }

    float distanceSum = d1 + d2;
    if (distanceSum == 0.0f)
    {
        LOG_ERR("Invalid inverse-distance weights: d1 + d2 is zero, using nearest index");
        return static_cast<float>(nearestIndex);
    }

    float w1 = d2 / distanceSum;
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

    SensorVals sensorVals = readSensors();
    if ((sensorVals.x1 == 0 && sensorVals.z1 == 0) || (sensorVals.x2 == 0 && sensorVals.z2 == 0))
    {
        LOG_ERR("Failed to read sensor values");
        return false;
    }

    mSensorLUT[position] = sensorVals;

    LOG_DBG("Stored position %d: x1=%d, z1=%d; x2=%d, z2=%d", position, sensorVals.x1, sensorVals.z1, sensorVals.x2,
            sensorVals.z2);

    if (position == CONFIG_IB_PUMP_RESERVOIR_VOLUME)
    {
        return postProcessLUT();
    }

    return true;
}

uint16_t PosSensor::transformValue(int16_t value) const
{
    // Transform values from range -2000, 2000 to 0, 40000
    return (value + 2000) * 10; // Shift range to 0-40000
}

bool PosSensor::postProcessLUT()
{
    if (mLUTReady)
    {
        LOG_DBG("LUT is already ready, skipping post-processing");
        return true;
    }

    SensorVals prevVals = {0, 0, 0, 0};
    SensorVals currentVals = {0, 0, 0, 0};
    for (size_t i = 0; i < mSensorLUT.size(); ++i)
    {
        LOG_DBG("LUT before processing pos %d: x1=%d, z1=%d; x2=%d, z2=%d", static_cast<int>(i),
                mSensorLUT[i].x1, mSensorLUT[i].z1, mSensorLUT[i].x2, mSensorLUT[i].z2);
        // Check if any of the values are zero, which indicates no data
        if (mSensorLUT[i].x1 == 0 && mSensorLUT[i].z1 == 0 && mSensorLUT[i].x2 == 0 && mSensorLUT[i].z2 == 0)
        {
            // LUT not complete
            return false;
        }
        // Apply average smoothing to the LUT
        if (i == 0)
        {
            // Smooth first element using only center and next value
            constexpr float weight = 0.5f;
            mSensorLUT[i].x1 = std::round((mSensorLUT[i].x1 * weight + mSensorLUT[i + 1].x1 * weight));
            mSensorLUT[i].z1 = std::round((mSensorLUT[i].z1 * weight + mSensorLUT[i + 1].z1 * weight));
            mSensorLUT[i].x2 = std::round((mSensorLUT[i].x2 * weight + mSensorLUT[i + 1].x2 * weight));
            mSensorLUT[i].z2 = std::round((mSensorLUT[i].z2 * weight + mSensorLUT[i + 1].z2 * weight));
            prevVals = mSensorLUT[i];
            LOG_DBG("LUT after processing pos %d: x1=%d, z1=%d; x2=%d, z2=%d", static_cast<int>(i),
                    mSensorLUT[i].x1, mSensorLUT[i].z1, mSensorLUT[i].x2, mSensorLUT[i].z2);
            continue;
        }
        if (i == mSensorLUT.size() - 1)
        {
            // Smooth last element using only previous and center value
            constexpr float weight = 0.5f;
            currentVals = mSensorLUT[i];
            mSensorLUT[i].x1 = std::round((prevVals.x1 * weight + mSensorLUT[i].x1 * weight));
            mSensorLUT[i].z1 = std::round((prevVals.z1 * weight + mSensorLUT[i].z1 * weight));
            mSensorLUT[i].x2 = std::round((prevVals.x2 * weight + mSensorLUT[i].x2 * weight));
            mSensorLUT[i].z2 = std::round((prevVals.z2 * weight + mSensorLUT[i].z2 * weight));
            prevVals = currentVals;
            LOG_DBG("LUT after processing pos %d: x1=%d, z1=%d; x2=%d, z2=%d", static_cast<int>(i),
                    mSensorLUT[i].x1, mSensorLUT[i].z1, mSensorLUT[i].x2, mSensorLUT[i].z2);
            continue;
        }
        constexpr float centerWeight = 0.4f; // Weight for the center value
        constexpr float sideWeight = (1.0f - centerWeight) / 2.0f; // Weight for the side values

        currentVals = mSensorLUT[i];
        mSensorLUT[i].x1 = std::round((prevVals.x1 * sideWeight + mSensorLUT[i].x1 * centerWeight + mSensorLUT[i + 1].x1 * sideWeight));
        mSensorLUT[i].z1 = std::round((prevVals.z1 * sideWeight + mSensorLUT[i].z1 * centerWeight + mSensorLUT[i + 1].z1 * sideWeight));
        mSensorLUT[i].x2 = std::round((prevVals.x2 * sideWeight + mSensorLUT[i].x2 * centerWeight + mSensorLUT[i + 1].x2 * sideWeight));
        mSensorLUT[i].z2 = std::round((prevVals.z2 * sideWeight + mSensorLUT[i].z2 * centerWeight + mSensorLUT[i + 1].z2 * sideWeight));
        prevVals = currentVals;

        LOG_DBG("LUT after processing pos %d: x1=%d, z1=%d; x2=%d, z2=%d", static_cast<int>(i),
                mSensorLUT[i].x1, mSensorLUT[i].z1, mSensorLUT[i].x2, mSensorLUT[i].z2);
    }

    // Save the LUT to persistent storage
    std::string key = std::string(settingsSubKey) + "/" + settingsPosLUTKey;
    if (settings_save_one(key.c_str(), mSensorLUT.data(), mSensorLUT.size() * sizeof(SensorVals)) < 0)
    {
        LOG_ERR("Failed to save position LUT to settings");
    }

    // Mark as ready, even when storing fails, we have data at least for the duration when the device is powered
    mLUTReady = true;
    LOG_INF("Position sensor LUT post-processed successfully");
    return true;
}

PosSensor::SensorVals PosSensor::readSensors() const
{
    if (!device_is_ready(mTopSensor))
    {
        LOG_ERR("Top sensor is not ready");
        return {0, 0, 0, 0};
    }
    if (!device_is_ready(mBottomSensor))
    {
        LOG_ERR("Bottom sensor is not ready");
        return {0, 0, 0, 0};
    }
    if (sensor_sample_fetch(mTopSensor) < 0)
    {
        LOG_ERR("Failed to fetch samples from top sensor");
        return {0, 0, 0, 0};
    }
    if (sensor_sample_fetch(mBottomSensor) < 0)
    {
        LOG_ERR("Failed to fetch samples from bottom sensor");
        return {0, 0, 0, 0};
    }

    struct sensor_value top_mag_x, top_mag_z, bottom_mag_x, bottom_mag_z;
    sensor_channel_get(mTopSensor, SENSOR_CHAN_MAGN_X, &top_mag_x);
    sensor_channel_get(mTopSensor, SENSOR_CHAN_MAGN_Z, &top_mag_z);
    sensor_channel_get(mBottomSensor, SENSOR_CHAN_MAGN_X, &bottom_mag_x);
    sensor_channel_get(mBottomSensor, SENSOR_CHAN_MAGN_Z, &bottom_mag_z);
    return {transformValue(top_mag_x.val1), transformValue(top_mag_z.val1),
            transformValue(bottom_mag_x.val1), transformValue(bottom_mag_z.val1)};
}

int PosSensor::loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
{
    if (strcmp(key, settingsPosLUTKey) == 0)
    {
        LOG_DBG("Loading position LUT from settings");
        size_t read_len = read_cb(cb_arg, &mSensorLUT[0], mSensorLUT.size() * sizeof(SensorVals));
        if (read_len != mSensorLUT.size() * sizeof(SensorVals))
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
