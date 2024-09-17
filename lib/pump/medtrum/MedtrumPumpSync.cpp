#include <pump/medtrum/MedtrumPumpSync.h>
#include <pump/medtrum/utils/MedtrumTimeUtil.h>

#include <string>
#include <zephyr/kernel.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

namespace
{
    constexpr char subKey[] = "mt";
    constexpr char pumpStateKey[] = "state";
    constexpr char sessionTokenKey[] = "token";
    constexpr char patchIdKey[] = "patchId";
    constexpr char currentSequenceNumberKey[] = "seq";
    constexpr char syncedSequenceNumberKey[] = "syncSeq";
}

LOG_MODULE_REGISTER(ib_medtrum_pump_sync);

MedtrumPumpSync::MedtrumPumpSync()
{
    LOG_DBG("Constructing MedtrumPumpSync");
    k_sem_init(&mInitSem, 0, 1);
}

MedtrumPumpSync::~MedtrumPumpSync() {}

void MedtrumPumpSync::init()
{
    LOG_DBG("Initializing MedtrumPumpSync");

    k_mutex_init(&mSwVersionMutex);
    k_mutex_init(&mBasalStartTimeMutex);
    k_mutex_init(&mBolusProgressMutex);

    settings_subsys_init();

    // Load the settings on initialization
    settings_load_subtree_direct(
        subKey,
        [](const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param) {
            return static_cast<MedtrumPumpSync *>(param)->loadCb(key, len, read_cb, cb_arg, param);
        },
        this);
}

void MedtrumPumpSync::setPumpState(PumpState state)
{
    mPumpState.store(state);
    std::string key = std::string(subKey) + "/" + pumpStateKey;
    settings_save_one(key.c_str(), &state, sizeof(state));
}

PumpState MedtrumPumpSync::getPumpState()
{
    return mPumpState.load();
}

void MedtrumPumpSync::setSessionToken(uint32_t token)
{
    mSessionToken.store(token);
    std::string key = std::string(subKey) + "/" + sessionTokenKey;
    settings_save_one(key.c_str(), &token, sizeof(token));
}

uint32_t MedtrumPumpSync::getSessionToken()
{
    return mSessionToken.load();
}

void MedtrumPumpSync::setPatchId(uint16_t patchId)
{
    mPatchId.store(patchId);
    std::string key = std::string(subKey) + "/" + patchIdKey;
    settings_save_one(key.c_str(), &patchId, sizeof(patchId));
}

uint16_t MedtrumPumpSync::getPatchId()
{
    return mPatchId.load();
}

void MedtrumPumpSync::setCurrentSequenceNumber(uint16_t sequenceNumber)
{
    mCurrentSequenceNumber.store(sequenceNumber);
    std::string key = std::string(subKey) + "/" + currentSequenceNumberKey;
    settings_save_one(key.c_str(), &sequenceNumber, sizeof(sequenceNumber));
}

uint16_t MedtrumPumpSync::getCurrentSequenceNumber()
{
    return mCurrentSequenceNumber.load();
}

void MedtrumPumpSync::setSyncedSequenceNumber(uint16_t sequenceNumber)
{
    mSyncedSequenceNumber.store(sequenceNumber);
    std::string key = std::string(subKey) + "/" + syncedSequenceNumberKey;
    settings_save_one(key.c_str(), &sequenceNumber, sizeof(sequenceNumber));
}

uint16_t MedtrumPumpSync::getSyncedSequenceNumber()
{
    return mSyncedSequenceNumber.load();
}

void MedtrumPumpSync::setReservoirLevel(float level)
{
    mReservoirLevel.store(level);
}

float MedtrumPumpSync::getReservoirLevel()
{
    return mReservoirLevel.load();
}

void MedtrumPumpSync::setDeviceType(uint8_t type)
{
    mDeviceType.store(type);
}

uint8_t MedtrumPumpSync::getDeviceType()
{
    return mDeviceType.load();
}

void MedtrumPumpSync::setSwVersion(std::string version)
{
    k_mutex_lock(&mSwVersionMutex, K_FOREVER);
    mSwVersion = version;
    k_mutex_unlock(&mSwVersionMutex);
}

const std::string &MedtrumPumpSync::getSwVersion()
{
    k_mutex_lock(&mSwVersionMutex, K_FOREVER);
    const std::string &version = mSwVersion;
    k_mutex_unlock(&mSwVersionMutex);
    return version;
}

void MedtrumPumpSync::setPrimeProgress(uint8_t progress)
{
    mPrimeProgress.store(progress);
}

uint8_t MedtrumPumpSync::getPrimeProgress()
{
    return mPrimeProgress.load();
}

void MedtrumPumpSync::setPatchStartTime(time_t startTime)
{
    k_mutex_lock(&mPatchStartTimeMutex, K_FOREVER);
    mPatchStartTime = startTime;
    k_mutex_unlock(&mPatchStartTimeMutex);
}

time_t MedtrumPumpSync::getPatchStartTime()
{
    k_mutex_lock(&mPatchStartTimeMutex, K_FOREVER);
    time_t time = mPatchStartTime;
    k_mutex_unlock(&mPatchStartTimeMutex);
    return time;
}

BasalType MedtrumPumpSync::getBasalType()
{
    return mBasalType.load();
}

float MedtrumPumpSync::getBasalRate()
{
    return mBasalRate.load();
}

uint16_t MedtrumPumpSync::getBasalSequence()
{
    return mBasalSequence.load();
}

time_t MedtrumPumpSync::getBasalStartTime()
{
    k_mutex_lock(&mBasalStartTimeMutex, K_FOREVER);
    time_t time = mBasalStartTime;
    k_mutex_unlock(&mBasalStartTimeMutex);
    return time;
}

time_t MedtrumPumpSync::getBolusProgressLastTime()
{
    k_mutex_lock(&mBolusProgressMutex, K_FOREVER);
    time_t time = mBolusProgressLastTime;
    k_mutex_unlock(&mBolusProgressMutex);
    return time;
}

bool MedtrumPumpSync::getBolusInProgress()
{
    return mBolusInProgress.load();
}

float MedtrumPumpSync::getBolusAmountDelivered()
{
    return mBolusAmountDelivered.load();
}

void MedtrumPumpSync::handleBasalStatusUpdate(BasalType type, float rate, uint16_t sequence, uint16_t patchId,
                                              time_t startTime)
{
    LOG_DBG("Handling basal status update");
    LOG_DBG("Basal type: %u, Basal rate: %f, Basal sequence: %d, Basal patch id: %d, Basal start time: %lld",
            static_cast<uint8_t>(type), static_cast<double>(rate), sequence, patchId, startTime);

    if (patchId != getPatchId())
    {
        LOG_ERR("Patch id mismatch: %d vs stored patch id: %d", patchId, getPatchId());
        // PatchId update should be handled by NotificationPacket
    }

    mBasalType.store(type);
    mBasalRate.store(rate);
    mBasalSequence.store(sequence);
    if (sequence > getCurrentSequenceNumber())
    {
        LOG_DBG("Setting current sequence number to: %d", sequence);
        setCurrentSequenceNumber(sequence);
    }

    k_mutex_lock(&mBasalStartTimeMutex, K_FOREVER);
    mBasalStartTime = startTime;
    k_mutex_unlock(&mBasalStartTimeMutex);
}

void MedtrumPumpSync::handleBolusStatusUpdate(uint8_t bolusType, bool bolusCompleted, float ammountDelivered)
{
    LOG_DBG("Handling bolus status update");
    LOG_DBG("Bolus type: %u, Bolus completed: %d, Bolus ammount delivered: %f", bolusType, bolusCompleted,
            static_cast<double>(ammountDelivered));

    k_mutex_lock(&mBolusProgressMutex, K_FOREVER);
    mBolusProgressLastTime = MedtrumTimeUtil::getCurrentTime();
    k_mutex_unlock(&mBolusProgressMutex);

    mBolusInProgress.store(!bolusCompleted);
    mBolusAmountDelivered.store(ammountDelivered);
}

void MedtrumPumpSync::handleNewPatch(uint16_t patchId, uint16_t sequence, time_t startTime)
{
    LOG_DBG("Handling new patch");
    LOG_DBG("Patch id: %d, Patch sequence: %d, Patch start time: %lld", patchId, sequence, startTime);

    setPatchId(patchId);
    setPatchStartTime(startTime);
    setCurrentSequenceNumber(sequence);
    setSyncedSequenceNumber(1); // New patch, so we start from 1
}

int MedtrumPumpSync::loadCb(const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param)
{
    // Load the settings from the settings subsystem
    if (strcmp(key, pumpStateKey) == 0)
    {
        LOG_DBG("Loading pump state");
        PumpState state;
        int len = read_cb(cb_arg, &state, sizeof(state));
        if (len != sizeof(state))
        {
            LOG_ERR("Failed to read pump state from settings");
            return -1;
        }
        mPumpState.store(state);
    }
    else if (strcmp(key, sessionTokenKey) == 0)
    {
        LOG_DBG("Loading session token");
        uint32_t token;
        int len = read_cb(cb_arg, &token, sizeof(token));
        if (len != sizeof(token))
        {
            LOG_ERR("Failed to read session token from settings");
            return -1;
        }
        mSessionToken.store(token);
    }
    else if (strcmp(key, patchIdKey) == 0)
    {
        LOG_DBG("Loading patch id");
        uint16_t patchId;
        int len = read_cb(cb_arg, &patchId, sizeof(patchId));
        if (len != sizeof(patchId))
        {
            LOG_ERR("Failed to read patch id from settings");
            return -1;
        }
        mPatchId.store(patchId);
    }
    else if (strcmp(key, currentSequenceNumberKey) == 0)
    {
        LOG_DBG("Loading current sequence number");
        uint16_t sequence;
        int len = read_cb(cb_arg, &sequence, sizeof(sequence));
        if (len != sizeof(sequence))
        {
            LOG_ERR("Failed to read current sequence number from settings");
            return -1;
        }
        mCurrentSequenceNumber.store(sequence);
    }
    else if (strcmp(key, syncedSequenceNumberKey) == 0)
    {
        LOG_DBG("Loading sync sequence number");
        uint16_t sequence;
        int len = read_cb(cb_arg, &sequence, sizeof(sequence));
        if (len != sizeof(sequence))
        {
            LOG_ERR("Failed to read sync sequence number from settings");
            return -1;
        }
        mSyncedSequenceNumber.store(sequence);
    }
    else
    {
        LOG_ERR("Unknown key: %s", key);
        return -1;
    }

    return 0;
}
