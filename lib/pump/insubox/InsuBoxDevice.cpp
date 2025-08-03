#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_pump_device);

#include <string>

// TODO: move low level stuff to its own files
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

// Sensor devices
static const struct device *sensor0 = DEVICE_DT_GET(DT_ALIAS(mag_bottom));
static const struct device *sensor1 = DEVICE_DT_GET(DT_ALIAS(mag_top));

namespace
{
    constexpr char settingsSubKey[] = "ib";
    constexpr char settingsPlungerPosKey[] = "plPos";
}

InsuBoxDevice::InsuBoxDevice(IPumpDeviceCallback &pumpDeviceCallback)
    : mPumpDeviceCallback(pumpDeviceCallback), mMotor(createMotorInstance(*this))
{
    LOG_DBG("InsuBoxDevice constructor");

    mSubContainer.mDevice = this;
    k_work_init_delayable(&mSubContainer.sensorWork, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SubContainer, sensorWork);
        container->mDevice->sensorWork();
    });

    mBolusTask.device = this;
    mBolusTask.completed = true;
    k_work_init_delayable(&mBolusTask.bolusWork, [](struct k_work *work) {
        auto *task = CONTAINER_OF(work, BolusTask, bolusWork);
        task->device->bolusWork();
    });

    settings_subsys_init();
    settings_load_subtree_direct(
        settingsSubKey,
        [](const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param) {
            return static_cast<InsuBoxDevice *>(param)->loadCb(key, len, read_cb, cb_arg, param);
        },
        this);
}

InsuBoxDevice::~InsuBoxDevice()
{
    LOG_DBG("InsuBoxDevice destructor");
}

void InsuBoxDevice::init()
{
    LOG_DBG("InsuBoxDevice init");

    k_work_reschedule(&mSubContainer.sensorWork, K_NO_WAIT);
}

void InsuBoxDevice::onBolusRequest(float amount, time_t timestamp)
{
    LOG_DBG("Bolus request: %.2f units at %lld", static_cast<double>(amount), timestamp);
    if (mBolusTask.completed == false)
    {
        LOG_ERR("Bolus already in progress");
        return;
    }
    mBolusTask.requestedBolus = amount;
    mBolusTask.requestedTimestamp = timestamp;
    mBolusTask.deliveredBolus = 0.0f;
    mBolusTask.completed = false;
    k_work_reschedule(&mBolusTask.bolusWork, K_NO_WAIT);
}

void InsuBoxDevice::onStopBolusRequest()
{
    LOG_DBG("Stop bolus");
    k_work_cancel_delayable(&mBolusTask.bolusWork);
    mMotor.stop();
}

void InsuBoxDevice::onRetractRequest()
{
    LOG_DBG("Retract request");
    if (mBolusTask.completed == false)
    {
        LOG_ERR("Motor busy, what did you do :') ????");
        return;
    }

    // TODO: Maybe want to add a priming/retracting flag to the bolus status?
    std::optional<float> position = mMotor.getPosition();
    if (position.has_value())
    {
        // TODO: Plunger position detection
        // For now add 50 units to ensure we fully retract
        position = position.value() + 50.0f;
    }
    else
    {
        // Position not initialized, assume first assembly/install
        position = 350.0f;
    }

    mMotor.setPosition(position.value());
    mBolusTask.requestedBolus = -position.value();
    mBolusTask.requestedTimestamp = 0;
    mBolusTask.deliveredBolus = 0.0f;
    mBolusTask.completed = false;
    mMotor.moveToPosition(0.0f, 80);
}

void InsuBoxDevice::onMotorCompleted(float delivered, float position, bool stopped, bool error)
{
    LOG_DBG("Deliver completed: units: %.2f, position: %.2f, stopped: %d, error: %d", static_cast<double>(delivered),
            static_cast<double>(position), stopped, error);

    std::string key = std::string(settingsSubKey) + "/" + settingsPlungerPosKey;
    settings_save_one(key.c_str(), &position, sizeof(position));

    mBolusTask.deliveredBolus += delivered;
    if (stopped || error)
    {
        mBolusTask.completed = true;
        // TODO: Report error somehow
    }

    k_work_reschedule(&mBolusTask.bolusWork, K_MSEC(500));
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

void InsuBoxDevice::bolusWork()
{
    LOG_DBG("Bolus work");

    // TODO: Verify plunger pos here?
    float remainingBolus = mBolusTask.requestedBolus - mBolusTask.deliveredBolus;
    if (mBolusTask.completed || remainingBolus <= 0.01f)
    {
        LOG_DBG("Bolus completed");
        mBolusTask.completed = true;
        sendBolusProgressUpdate();
        return;
    }

    // Deliver 0.5 per time, and update the delivered amount
    float bolusToDeliver = (remainingBolus > 0.5f) ? 0.5f : remainingBolus;
    constexpr int BOLUS_SPEED = 2;
    int err = mMotor.deliver(bolusToDeliver, BOLUS_SPEED);
    if (err)
    {
        LOG_ERR("Failed to deliver bolus: %d", err);
        mBolusTask.completed = true;
    }

    sendBolusProgressUpdate();
}

void InsuBoxDevice::sendBolusProgressUpdate()
{
    LOG_DBG("Sending bolus progress update");
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);
    BolusProgressUpdate progress = {
        .requestedAmount = mBolusTask.requestedBolus,
        .requestedTimestamp = mBolusTask.requestedTimestamp,
        .deliveredAmount = mBolusTask.deliveredBolus,
        .deliveredTimestamp = currentTime.tv_sec,
        .completed = mBolusTask.completed,
    };
    mPumpDeviceCallback.bolusProgressUpdate(progress);
}

int InsuBoxDevice::loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
{
    // Load the settings from the settings subsystem
    if (strcmp(key, settingsPlungerPosKey) == 0)
    {
        LOG_DBG("Loading plunger position");
        float position;
        int len = read_cb(cb_arg, &position, sizeof(position));
        if (len != sizeof(position))
        {
            LOG_ERR("Failed to read plunger position from settings");
            return -1;
        }
        mMotor.setPosition(position);
    }
    else
    {
        LOG_ERR("Unknown key: %s", key);
        return -1;
    }

    return 0;
}

Motor &InsuBoxDevice::createMotorInstance(IMotorCallback &callback)
{
    static Motor motor(callback);
    return motor;
}
