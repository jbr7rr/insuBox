#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_pump_device);

#include <cmath>
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

    mSensorTask.mDevice = this;
    k_work_init_delayable(&mSensorTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SimpleTask, work);
        container->mDevice->sensorTask();
    });

    mBolusTask.device = this;
    mBolusTask.completed = true;
    k_work_init_delayable(&mBolusTask.work, [](struct k_work *work) {
        auto *task = CONTAINER_OF(work, BolusTask, work);
        task->device->bolusTask();
    });

    mRetractTask.mDevice = this;
    k_work_init_delayable(&mRetractTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SimpleTask, work);
        container->mDevice->retractTask();
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

    k_work_reschedule(&mSensorTask.work, K_NO_WAIT);
}

void InsuBoxDevice::onBolusRequest(float amount, time_t timestamp)
{
    LOG_DBG("Bolus request: %.2f units at %lld", static_cast<double>(amount), timestamp);
    if (mState != State::IDLE)
    {
        LOG_ERR("Cannot handle bolus request, device is busy");
        return;
    }
    auto totalDelivered = mMotor.getPosition();
    if (totalDelivered.has_value() && totalDelivered.value() + amount > CONFIG_IB_PUMP_RESERVOIR_VOLUME)
    {
        amount = CONFIG_IB_PUMP_RESERVOIR_VOLUME - totalDelivered.value();
        LOG_WRN("Requested bolus exceeds reservoir volume, adjusting to %.2f units", static_cast<double>(amount));
    }

    mState = State::DELIVERING_BOLUS;
    mBolusTask.requestedBolus = amount;
    mBolusTask.requestedTimestamp = timestamp;
    mBolusTask.deliveredBolus = 0.0f;
    mBolusTask.completed = false;
    k_work_reschedule(&mBolusTask.work, K_NO_WAIT);
}

void InsuBoxDevice::onStopBolusRequest()
{
    // TODO: Check if this syncs properly
    LOG_DBG("Stop bolus");
    k_work_cancel_delayable(&mBolusTask.work);
    mMotor.stop();
}

void InsuBoxDevice::onRetractRequest()
{
    LOG_DBG("Retract request");
    if (mState != State::IDLE)
    {
        LOG_ERR("Cannot handle retract request, device is busy");
        return;
    }
    mState = State::RETRACTING;
    k_work_reschedule(&mRetractTask.work, K_NO_WAIT);
}

void InsuBoxDevice::onMotorCompleted(float delivered, float position, bool stopped, bool error)
{
    LOG_DBG("Deliver completed: units: %.2f, position: %.2f, stopped: %d, error: %d", static_cast<double>(delivered),
            static_cast<double>(position), stopped, error);

    std::string key = std::string(settingsSubKey) + "/" + settingsPlungerPosKey;
    settings_save_one(key.c_str(), &position, sizeof(position));

    if (mState == State::RETRACTING)
    {
        k_work_reschedule(&mRetractTask.work, K_NO_WAIT);
        return;
    }

    if (mState == State::DELIVERING_BOLUS)
    {
        mBolusTask.deliveredBolus += delivered;
        mBolusTask.completed = (stopped || error);
        k_work_reschedule(&mBolusTask.work, K_MSEC(500));
    }
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

void InsuBoxDevice::sensorTask()
{
    read_sensor(sensor0);
    read_sensor(sensor1);

    k_work_reschedule(&mSensorTask.work, K_MSEC(10000));
}

void InsuBoxDevice::bolusTask()
{
    LOG_DBG("Bolus work");

    // TODO: Verify plunger pos here?
    float remainingBolus = mBolusTask.requestedBolus - mBolusTask.deliveredBolus;
    if (mBolusTask.completed || remainingBolus <= 0.01f)
    {
        LOG_DBG("Bolus completed");
        mState = State::IDLE;
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
        mState = State::IDLE;
        mBolusTask.completed = true;
    }

    sendBolusProgressUpdate();
}

void InsuBoxDevice::retractTask()
{
    LOG_DBG("Retract work");

    if (mState != State::RETRACTING)
    {
        LOG_ERR("Invalid state for retract task: %d", static_cast<int>(mState.load()));
        return;
    }

    std::optional<float> currentPosition = mMotor.getPosition();
    if (!currentPosition.has_value())
    {
        // Assemble with retracted plunger
        // Maybe error here, and seperate assembly procedure?
        LOG_ERR("Current position is not set, assuming retracted position");
        mMotor.setPosition(5.0f);
        currentPosition = 5.0f;
    }

    constexpr float TOLERANCE = 0.0001f;
    constexpr float SLOW_RETRACT_POSITION = 15.0f;
    constexpr float FULL_RETRACT_POSITION = 0.0f;
    constexpr float EXTRA_UNITS = 5.0f;

    if (currentPosition.value() > SLOW_RETRACT_POSITION)
    {
        // Start retract at full speed
        constexpr int RETRACT_SPEED = 100;     // Set speed to 100% for retract
        constexpr uint8_t RETRACT_POWER = 100; // Set power to 80% for retract, to not block the motor
        int err = mMotor.moveToPosition(SLOW_RETRACT_POSITION, RETRACT_SPEED, RETRACT_POWER);
        if (err)
        {
            LOG_ERR("Failed to retract: %d", err);
            return;
        }
    }
    else if (currentPosition.value() > TOLERANCE)
    {
        // Add some unit to ensure we fully retract
        mMotor.setPosition(currentPosition.value() + EXTRA_UNITS);

        constexpr int RETRACT_SPEED = 20;
        constexpr uint8_t RETRACT_POWER = 50;
        int err = mMotor.moveToPosition(FULL_RETRACT_POSITION, RETRACT_SPEED, RETRACT_POWER);
        if (err)
        {
            LOG_ERR("Failed to bump forward: %d", err);
            return;
        }
    }
    else
    {
        // Update here?
        mState = State::IDLE;
    }
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
