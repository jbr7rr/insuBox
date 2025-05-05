#include <pump/insubox/InsuBoxDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_pump_device);

// TODO: move low level stuff to its own files
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>

// Sensor devices
static const struct device *sensor0 = DEVICE_DT_GET(DT_ALIAS(magn0));
static const struct device *sensor1 = DEVICE_DT_GET(DT_ALIAS(magn1));

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

void InsuBoxDevice::onStopBolus()
{
    LOG_DBG("Stop bolus");
    LOG_WRN("Stop bolus not implemented yet");
    mMotor.stop();
}

void InsuBoxDevice::onMotorCompleted(float delivered, float position, bool stopped, bool error)
{
    LOG_DBG("Deliver completed: units: %.2f, position: %.2f, stopped: %d, error: %d", static_cast<double>(delivered),
            static_cast<double>(position), stopped, error);

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
    float remainingBolus = mBolusTask.requestedBolus - mBolusTask.deliveredBolus;

    // TODO: Verify plunger pos here?

    if (mBolusTask.completed || remainingBolus <= 0.01f)
    {
        LOG_DBG("Bolus completed");
        mBolusTask.completed = true;
        sendBolusProgressUpdate();
        return;
    }

    float bolusToPump = 0.0f;
    if (remainingBolus > 0.5f)
    {
        bolusToPump = 0.5f;
    }
    else
    {
        bolusToPump = remainingBolus;
    }

    sendBolusProgressUpdate();
    mMotor.deliver(bolusToPump, 2);
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
    mPumpDeviceCallback.onBolusProgressUpdate(progress);
}

Motor &InsuBoxDevice::createMotorInstance(IMotorCallback &callback)
{
    static Motor motor(callback);
    return motor;
}
