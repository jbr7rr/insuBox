#include "bt_kaleido/bt_kaleido.h"
#include <control/kaleido/KaleidoDevice.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_kaleido_device);

KaleidoDevice::KaleidoDevice(EventDispatcher &dispatcher) : mDispatcher(dispatcher) {}

void KaleidoDevice::init()
{
    LOG_DBG("Initializing KaleidoDevice");
    bt_kaleido::init();
}

void KaleidoDevice::onPumpStatusUpdated(const PumpStatus &status)
{
    LOG_DBG("KaleidoDevice::onPumpStatusUpdated");
    (void)status;
}

void KaleidoDevice::onAlarmStatusUpdated(const AnnunciationType &annunciation, bool cancel)
{
    LOG_DBG("KaleidoDevice::onAlarmStatusUpdated");
    (void)annunciation;
    (void)cancel;
}
