#include <pump/VirtualPumpDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_pump_device);

VirtualPumpDevice::VirtualPumpDevice(IPumpServiceCallback &pumpServiceCallback)
    : mPumpServiceCallback(pumpServiceCallback)
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

void VirtualPumpDevice::onStopBolus()
{
    LOG_DBG("Stop bolus request");
    BolusProgressUpdate progress = {
        .requestedAmount = mSubContainer.requestedBolus,
        .requestedTimestamp = mSubContainer.requestedTimestamp,
        .deliveredAmount = mSubContainer.deliveredBolus,
        .deliveredTimestamp = 0,
        .completed = true,
    };
    mPumpServiceCallback.onBolusProgressUpdate(progress);

    mSubContainer.requestedBolus = 0.0f;
    k_work_reschedule(&mSubContainer.statusWork, K_NO_WAIT);
}

void VirtualPumpDevice::_updateStatus()
{
    LOG_DBG("VirtualPumpDevice update status");

    uint8_t intervalSec = 15;

    PumpStatusUpdated status = {
        .therapyControlState = TherapyControlState::RUN,
        .operationalState = OperationalState::READY,
        .reservoirLevel = SFloat(101.0f),
        .reservoirAttached = true,
    };

    mPumpServiceCallback.pumpStatusUpdated(status);

    if (mSubContainer.deliveredBolus < mSubContainer.requestedBolus)
    {
        intervalSec = 2;
        mSubContainer.deliveredBolus += 0.1f;
        struct timespec currentTime;
        clock_gettime(CLOCK_REALTIME, &currentTime);
        BolusProgressUpdate progress = {
            .requestedAmount = mSubContainer.requestedBolus,
            .requestedTimestamp = mSubContainer.requestedTimestamp,
            .deliveredAmount = mSubContainer.deliveredBolus,
            .deliveredTimestamp = currentTime.tv_sec,
            .completed = (mSubContainer.deliveredBolus >= mSubContainer.requestedBolus),
        };
        mPumpServiceCallback.onBolusProgressUpdate(progress);
    }

    k_work_reschedule(&mSubContainer.statusWork, K_SECONDS(intervalSec));
}
