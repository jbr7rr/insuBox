#include <pump/insubox/InsuBoxDevice.h>
#include <zephyr/kernel.h>

#include <cmath>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_insubox_pump_device);

InsuBoxDevice::InsuBoxDevice(IPumpDeviceCallback &pumpDeviceCallback)
    : mPumpDeviceCallback(pumpDeviceCallback), mMotor(createMotorInstance(*this)), mPosSensor(createPosSensorInstance())
{
    LOG_DBG("InsuBoxDevice constructor");

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

    mCalSensorTask.mDevice = this;
    k_work_init_delayable(&mCalSensorTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SimpleTask, work);
        container->mDevice->calSensorTask();
    });

    mPrimeTask.mDevice = this;
    k_work_init_delayable(&mPrimeTask.work, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SimpleTask, work);
        container->mDevice->primeTask();
    });
}

InsuBoxDevice::~InsuBoxDevice()
{
    LOG_DBG("InsuBoxDevice destructor");
}

void InsuBoxDevice::init()
{
    LOG_DBG("InsuBoxDevice init");
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

void InsuBoxDevice::onPrimeRequest()
{
    LOG_DBG("Prime request");
    if (mState != State::IDLE)
    {
        LOG_ERR("Cannot handle prime request, device is busy");
        return;
    }
    mState = State::PRIMING;
    k_work_reschedule(&mPrimeTask.work, K_NO_WAIT);
}

void InsuBoxDevice::onMotorCompleted(float delivered, float position, bool stopped, bool error)
{
    LOG_DBG("Deliver completed: units: %.2f, position: %.2f, stopped: %d, error: %d", static_cast<double>(delivered),
            static_cast<double>(position), stopped, error);

    // TODO: Move to motor class? Or ensure that we save it also when we set pos manually

    if (mState == State::DELIVERING_BOLUS)
    {
        mBolusTask.deliveredBolus += delivered;
        mBolusTask.completed = (stopped || error);
        k_work_reschedule(&mBolusTask.work, K_NO_WAIT);
    }
    else if (mState == State::RETRACTING)
    {
        k_work_reschedule(&mRetractTask.work, K_NO_WAIT);
    }
    else if (mState == State::CAL_SENSOR)
    {
        k_work_reschedule(&mCalSensorTask.work, K_NO_WAIT);
    }
    else if (mState == State::PRIMING)
    {
        k_work_reschedule(&mPrimeTask.work, K_NO_WAIT);
    }
    else
    {
        LOG_ERR("Whoops motor completed in unexpected state: %d", static_cast<int>(mState.load()));
        return;
    }
}

void InsuBoxDevice::bolusTask()
{
    LOG_DBG("Bolus work");

    // Validate sensor pos
    auto currentMotorposition = mMotor.getPosition();
    auto currentSensorPosition = mPosSensor.getPosition();
    constexpr float POSITION_TOLERANCE = 1.5f;

    float posDiff = std::fabs(currentMotorposition.value_or(0.0f) - currentSensorPosition.value_or(0.0f));
    if (posDiff > mMaxPlungerDifference)
    {
        LOG_WRN("Max plunger difference exceeded: %.2f > %.2f", static_cast<double>(posDiff),
                static_cast<double>(mMaxPlungerDifference));
        mMaxPlungerDifference = posDiff;
    }
    LOG_WRN("Motor pos: %.2f, sensor pos %.3f, diff: %.3f", static_cast<double>(currentMotorposition.value_or(0.0f)),
            static_cast<double>(currentSensorPosition.value_or(0.0f)), static_cast<double>(posDiff));

    if (posDiff > POSITION_TOLERANCE)
    {
        LOG_ERR("Motor position %.2f does not match sensor position %.2f, cannot deliver bolus",
                static_cast<double>(currentMotorposition.value_or(-99.0f)),
                static_cast<double>(currentSensorPosition.value_or(-95.0f)));
        // TODO: Error here
        mState = State::IDLE;
        mBolusTask.completed = true;
        sendBolusProgressUpdate();
        return;
    }

    // Check if bolus is completed
    float remainingBolus = mBolusTask.requestedBolus - mBolusTask.deliveredBolus;
    if (mBolusTask.completed || remainingBolus <= 0.01f)
    {
        LOG_DBG("Bolus completed");
        LOG_WRN("Max plunger difference: %.2f", static_cast<double>(mMaxPlungerDifference));

        mState = State::IDLE;
        mBolusTask.completed = true;
        sendBolusProgressUpdate();
        return;
    }

    constexpr int BOLUS_SPEED = 5;
    constexpr uint8_t BOLUS_POWER = 100;
    constexpr float BOLUS_DELIVERY_STEP = 0.25f;
    float bolusToDeliver = (remainingBolus > BOLUS_DELIVERY_STEP) ? BOLUS_DELIVERY_STEP : remainingBolus;
    int err = mMotor.deliver(bolusToDeliver, BOLUS_SPEED, BOLUS_POWER);
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

    constexpr float SLOW_RETRACT_POSITION = 10.0f;
    constexpr float FULL_RETRACT_POSITION = -0.25f;
    constexpr float FINAL_POSITION = 0.0f;
    constexpr float EXTRA_UNITS = 5.0f;

    if (currentPosition.value() > SLOW_RETRACT_POSITION)
    {
        // Start retract
        constexpr int RETRACT_SPEED = 100;
        constexpr uint8_t RETRACT_POWER = 110;
        int err = mMotor.moveToPosition(SLOW_RETRACT_POSITION, RETRACT_SPEED, RETRACT_POWER);
        if (err)
        {
            LOG_ERR("Failed to retract: %d", err);
            return;
        }
    }
    else if (currentPosition.value() > FINAL_POSITION)
    {
        // Add some unit to ensure we fully retract
        mMotor.setPosition(currentPosition.value() + EXTRA_UNITS);

        constexpr int RETRACT_SPEED = 95;
        constexpr uint8_t RETRACT_POWER = 75;
        int err = mMotor.moveToPosition(FULL_RETRACT_POSITION, RETRACT_SPEED, RETRACT_POWER);
        if (err)
        {
            LOG_ERR("Failed to retract: %d", err);
            mState = State::IDLE;
            return;
        }
    }
    else if (currentPosition.value() < FINAL_POSITION)
    {
        // Final adjust to 0 position
        constexpr int RETRACT_SPEED = 95;
        constexpr uint8_t RETRACT_POWER = 100;
        int err = mMotor.moveToPosition(FINAL_POSITION, RETRACT_SPEED, RETRACT_POWER);
        if (err)
        {
            LOG_ERR("Failed to retract: %d", err);
            mState = State::IDLE;
            return;
        }
    }
    else
    {
        constexpr float POSITION_TOLERANCE = 5.0f;
        auto position = mPosSensor.getPosition();
        if (!position.has_value())
        {
            LOG_DBG("Position sensor not calibrated, starting calibration");
            mState = State::CAL_SENSOR;
            k_work_reschedule(&mCalSensorTask.work, K_NO_WAIT);
        }
        else if (position.value() > POSITION_TOLERANCE)
        {
            LOG_WRN("Sensor position %.2f is not fully retracted. Updating motor position to match.",
                    static_cast<double>(position.value()));
            mMotor.setPosition(position.value());
            k_work_reschedule(&mRetractTask.work, K_NO_WAIT);
        }
        else
        {
            LOG_DBG("Retract completed");
            mState = State::IDLE;
        }
    }
}

void InsuBoxDevice::calSensorTask()
{
    LOG_DBG("Calibrate sensor work");

    if (mState != State::CAL_SENSOR)
    {
        LOG_ERR("Invalid state for calibration task: %d", static_cast<int>(mState.load()));
        return;
    }

    // This will build the LUT for the position sensor
    auto position = mMotor.getPosition();
    if (!position.has_value())
    {
        LOG_ERR("Failed to get position from motor, cannot calibrate");
        mState = State::IDLE;
        return;
    }

    k_sleep(K_MSEC(100)); // Wait for motor to stabilize
    if (!mPosSensor.storePositionToLUT(static_cast<int>(position.value())))
    {
        LOG_ERR("Failed to store position to LUT");
        mState = State::IDLE;
        return;
    }

    // Increment motor pos until we reach the end of reservoir
    constexpr float STEP_SIZE = 1.0f; // TODO: Get increment val from sensor config
    constexpr int SPEED = 90;         // Speed for calibration
    constexpr int POWER = 100;        // Power for calibration

    if (position.value() >= CONFIG_IB_PUMP_RESERVOIR_VOLUME)
    {
        LOG_DBG("Reached maximum position, calibration complete");
        mState = State::RETRACTING;
        k_work_reschedule(&mRetractTask.work, K_NO_WAIT);
        return;
    }

    int err = mMotor.deliver(STEP_SIZE, SPEED, POWER);
    if (err)
    {
        LOG_ERR("Failed to deliver during calibration: %d", err);
        mState = State::IDLE;
        return;
    }
}

void InsuBoxDevice::primeTask()
{
    // For now we only feel the plunger, and either need to manually prime the tube or deliver boli
    LOG_DBG("Prime work");

    auto currentMotorposition = mMotor.getPosition();
    auto currentSensorPosition = mPosSensor.getPosition();
    constexpr float POSITION_TOLERANCE = 4.0f;

    float posDiff = currentMotorposition.value_or(0.0f) - currentSensorPosition.value_or(0.0f);
    if (posDiff > POSITION_TOLERANCE)
    {
        // Assume plunger hit the reservoir end
        LOG_WRN("Plunger position difference too high: %.2f > %.2f, updating motor position to sensor position",
                static_cast<double>(posDiff), static_cast<double>(POSITION_TOLERANCE));

        k_sleep(K_SECONDS(5)); // Wait for any movement to settle
        currentSensorPosition = mPosSensor.getPosition();
        if (currentSensorPosition.has_value())
        {
            mMotor.setPosition(currentSensorPosition.value());
        }
        mState = State::IDLE;
        return;
    }
    if (posDiff < -POSITION_TOLERANCE)
    {
        // Reset motor position to sensor and continue priming
        LOG_WRN("Plunger position difference negative: %.2f < %.2f, updating motor position to sensor position",
                static_cast<double>(posDiff), static_cast<double>(POSITION_TOLERANCE));
        mMotor.setPosition(currentSensorPosition.value());
    }

    constexpr int SPEED = 97;
    constexpr uint8_t POWER = 100;
    int err = mMotor.deliver(1.0f, SPEED, POWER);
    if (err)
    {
        LOG_ERR("Failed to deliver during priming: %d", err);
        mState = State::IDLE;
        return;
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

Motor &InsuBoxDevice::createMotorInstance(IMotorCallback &callback)
{
    static Motor motor(callback);
    return motor;
}

PosSensor &InsuBoxDevice::createPosSensorInstance()
{
    static PosSensor posSensor;
    return posSensor;
}
