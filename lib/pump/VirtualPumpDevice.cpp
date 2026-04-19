#include <pump/VirtualPumpDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_pump_device);

VirtualPumpDevice::VirtualPumpDevice(IPumpDeviceCallback &pumpDeviceCallback) : mPumpDeviceCallback(pumpDeviceCallback)
{
    LOG_DBG("VirtualPumpDevice constructor");

    mSubContainer.pumpDevice = this;
    mSubContainer.requestedBolus = 0.0f;
    mSubContainer.deliveredBolus = 0.0f;
    k_work_init_delayable(&mSubContainer.statusWork, [](struct k_work *work) {
        auto *container = CONTAINER_OF(work, SubContainer, statusWork);
        container->pumpDevice->_updateStatus();
    });

    k_work_reschedule(&mSubContainer.statusWork, K_SECONDS(5));
}

VirtualPumpDevice::~VirtualPumpDevice()
{
    LOG_DBG("VirtualPumpDevice destructor");
}

void VirtualPumpDevice::init()
{
    LOG_DBG("VirtualPumpDevice init");
}

void VirtualPumpDevice::onBolusRequest(float amount, time_t timestamp)
{
    LOG_DBG("Bolus request: %.2f at %lld", static_cast<double>(amount), static_cast<long long>(timestamp));
    mSubContainer.requestedBolus = amount;
    mSubContainer.deliveredBolus = 0.0f;
    mSubContainer.requestedTimestamp = timestamp;

    k_work_reschedule(&mSubContainer.statusWork, K_NO_WAIT);
}

void VirtualPumpDevice::onStopBolusRequest()
{
    LOG_DBG("Stop bolus request");
    BolusProgressUpdate progress = {
        .requestedAmount = mSubContainer.requestedBolus,
        .requestedTimestamp = mSubContainer.requestedTimestamp,
        .deliveredAmount = mSubContainer.deliveredBolus,
        .deliveredTimestamp = 0,
        .completed = true,
    };
    mPumpDeviceCallback.bolusProgressUpdate(progress);

    mSubContainer.requestedBolus = 0.0f;
    k_work_reschedule(&mSubContainer.statusWork, K_NO_WAIT);
}

void VirtualPumpDevice::onRetractRequest()
{
    LOG_DBG("Retract request");
    mReservoirLevel = 300.0f;
    mSubContainer.requestedBolus = 0.0f;
    mSubContainer.deliveredBolus = 0.0f;
    k_work_reschedule(&mSubContainer.statusWork, K_NO_WAIT);
}

void VirtualPumpDevice::onPrimeRequest()
{
    LOG_DBG("Prime request");
    constexpr float kPrimeReservoirConsumption = 10.0f;
    mReservoirLevel = (mReservoirLevel > kPrimeReservoirConsumption) ? (mReservoirLevel - kPrimeReservoirConsumption) : 0.0f;
    mSubContainer.requestedBolus = 0.0f;
    mSubContainer.deliveredBolus = 0.0f;
    k_work_reschedule(&mSubContainer.statusWork, K_NO_WAIT);
}

void VirtualPumpDevice::_updateStatus()
{
    LOG_DBG("VirtualPumpDevice update status");

    uint8_t intervalSec = 15;

    PumpStatus status = {
        .therapyControlState = TherapyControlState::RUN,
        .operationalState = OperationalState::READY,
        .reservoirLevel = SFloat(mReservoirLevel),
        .reservoirAttached = true,
    };

    mPumpDeviceCallback.pumpStatusUpdate(status);

    if (mSubContainer.deliveredBolus < mSubContainer.requestedBolus)
    {
        intervalSec = 2;
        mSubContainer.deliveredBolus += 0.1f;
        mReservoirLevel -= 0.1f;
        struct timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);
        BolusProgressUpdate progress = {
            .requestedAmount = mSubContainer.requestedBolus,
            .requestedTimestamp = mSubContainer.requestedTimestamp,
            .deliveredAmount = mSubContainer.deliveredBolus,
            .deliveredTimestamp = currentTime.tv_sec,
            .completed = (mSubContainer.deliveredBolus >= mSubContainer.requestedBolus),
        };
        mPumpDeviceCallback.bolusProgressUpdate(progress);
    }

    k_work_reschedule(&mSubContainer.statusWork, K_SECONDS(intervalSec));
}
