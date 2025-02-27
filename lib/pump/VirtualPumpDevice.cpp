#include <pump/VirtualPumpDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_virtual_pump_device);

VirtualPumpDevice::VirtualPumpDevice(IPumpServiceCallback &pumpServiceCallback)
    : mPumpServiceCallback(pumpServiceCallback)
{
    LOG_DBG("VirtualPumpDevice constructor");

    mSubContainer.pumpDevice = this;
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

void VirtualPumpDevice::_updateStatus()
{
    LOG_DBG("VirtualPumpDevice update status");

    PumpStatusUpdated status = {
        .therapyControlState = TherapyControlState::RUN,
        .operationalState = OperationalState::READY,
        .reservoirLevel = SFloat(101.0f),
        .reservoirAttached = true,
    };

    mPumpServiceCallback.pumpStatusUpdated(status);

    PumpAnnunciationStatusUpdated type = {
        .type = AnnunciationType::BATTERY_FULL,
    };

    mPumpServiceCallback.pumpAnnunciationStatusUpdated(type);

    k_work_reschedule(&mSubContainer.statusWork, K_SECONDS(60));
}
