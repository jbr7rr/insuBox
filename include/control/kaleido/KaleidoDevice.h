#ifndef KALEIDO_DEVICE_H
#define KALEIDO_DEVICE_H

#include <control/IControlDevice.h>
#include <events/EventDispatcher.h>

class KaleidoDevice : public IControlDevice
{
public:
    explicit KaleidoDevice(EventDispatcher &dispatcher);
    ~KaleidoDevice() override = default;

    void init() override;
    void onPumpStatusUpdated(const PumpStatus &status) override;
    void onAlarmStatusUpdated(const AnnunciationType &annunciation, bool cancel = false) override;

private:
    EventDispatcher &mDispatcher;
};

#endif // KALEIDO_DEVICE_H
