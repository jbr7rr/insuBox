#ifndef MEDTRUM_PUMP_SYNC_H
#define MEDTRUM_PUMP_SYNC_H

#include <pump/medtrum/comm/enums/PumpState.h>

#include <stdint.h>
#include <atomic>
#include <stddef.h>

#include <zephyr/settings/settings.h>

class MedtrumPumpSync {
public:
    MedtrumPumpSync();
    ~MedtrumPumpSync();

    void init();

    void setPumpState(PumpState state);
    PumpState getPumpState();

    void setReservoirLevel(float level);
    float getReservoirLevel();

private:
    int loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param);

    std::atomic<PumpState> mPumpState;
    std::atomic<float> mReservoirLevel;
};

#endif // MEDTRUM_PUMP_SYNC_H
