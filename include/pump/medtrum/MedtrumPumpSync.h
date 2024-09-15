#ifndef MEDTRUM_PUMP_SYNC_H
#define MEDTRUM_PUMP_SYNC_H

#include <pump/medtrum/comm/enums/BasalType.h>
#include <pump/medtrum/comm/enums/PumpState.h>

#include <atomic>
#include <stddef.h>
#include <stdint.h>
#include <string>

#include <zephyr/kernel.h>
#include <zephyr/settings/settings.h>

class MedtrumPumpSync
{
public:
    MedtrumPumpSync();
    ~MedtrumPumpSync();

    void init();

    void setPumpState(PumpState state);
    PumpState getPumpState();

    void setSessionToken(uint32_t token);
    uint32_t getSessionToken();

    void setPatchId(uint16_t patchId);
    uint16_t getPatchId();

    void setCurrentSequenceNumber(uint16_t sequenceNumber);
    uint16_t getCurrentSequenceNumber();

    void setSyncedSequenceNumber(uint16_t sequenceNumber);
    uint16_t getSyncedSequenceNumber();

    /** Settings that are NOT stored in settings sub system and not persistent */
    void setReservoirLevel(float level);
    float getReservoirLevel();

    void setDeviceType(uint8_t type);
    uint8_t getDeviceType();

    void setSwVersion(std::string version);
    const std::string &getSwVersion();

    void setPrimeProgress(uint8_t progress);
    uint8_t getPrimeProgress();

    void setPatchStartTime(time_t startTime);
    time_t getPatchStartTime();

    BasalType getBasalType();
    float getBasalRate();
    uint16_t getBasalSequence();
    time_t getBasalStartTime();

    time_t getBolusProgressLastTime();
    bool getBolusInProgress();
    float getBolusAmountDelivered();

    void handleBasalStatusUpdate(BasalType type, float rate, uint16_t sequence, uint16_t patchId, time_t startTime);

    void handleBolusStatusUpdate(uint8_t bolusType, bool bolusCompleted, float ammountDelivered);

    void handleNewPatch(uint16_t patchId, uint16_t sequence, time_t startTime);

private:
    int loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param);

    struct k_sem mInitSem;

    std::atomic<PumpState> mPumpState;
    std::atomic<float> mReservoirLevel;
    std::atomic<uint32_t> mSessionToken;
    std::atomic<uint8_t> mDeviceType;
    std::atomic<int16_t> mPatchId;
    std::atomic<uint16_t> mCurrentSequenceNumber;
    std::atomic<uint16_t> mSyncedSequenceNumber;

    k_mutex mSwVersionMutex;
    std::string mSwVersion;

    std::atomic<uint8_t> mPrimeProgress;

    k_mutex mPatchStartTimeMutex;
    time_t mPatchStartTime;

    std::atomic<BasalType> mBasalType;
    std::atomic<float> mBasalRate;
    std::atomic<uint16_t> mBasalSequence;
    k_mutex mBasalStartTimeMutex;
    time_t mBasalStartTime;

    k_mutex mBolusProgressMutex;
    time_t mBolusProgressLastTime;
    std::atomic<bool> mBolusInProgress;
    std::atomic<float> mBolusAmountDelivered;
};

#endif // MEDTRUM_PUMP_SYNC_H
