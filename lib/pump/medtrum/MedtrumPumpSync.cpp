#include <pump/medtrum/MedtrumPumpSync.h>

#include <string>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

namespace
{
    constexpr char subKey[] = "mt";
    constexpr char pumpStateKey[] = "state";
    constexpr char reservoirLevelKey[] = "level";
    constexpr char sessionTokenKey[] = "token";
    constexpr char patchIdKey[] = "patchId";
}

LOG_MODULE_REGISTER(ib_medtrum_pump_sync);

MedtrumPumpSync::MedtrumPumpSync()
{
    LOG_DBG("Initializing MedtrumPumpSync");
}

MedtrumPumpSync::~MedtrumPumpSync() {}

void MedtrumPumpSync::init()
{
    LOG_DBG("Initializing MedtrumPumpSync");
    
    k_mutex_init(&mSwVersionMutex);
    k_mutex_init(&mBasalStartTimeMutex);

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

void MedtrumPumpSync::setReservoirLevel(float level)
{
    mReservoirLevel.store(level);
    std::string key = std::string(subKey) + "/" + reservoirLevelKey;
    settings_save_one(key.c_str(), &level, sizeof(level));
}

float MedtrumPumpSync::getReservoirLevel()
{
    return mReservoirLevel.load();
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

void MedtrumPumpSync::handleBasalStatusUpdate(BasalType type, float rate, uint16_t sequence, uint16_t patchId, time_t startTime)
{
    LOG_DBG("Handling basal status update");
    LOG_DBG("Basal type: %u, Basal rate: %f, Basal sequence: %d, Basal patch id: %d, Basal start time: %lld",
            static_cast<uint8_t>(type), static_cast<double>(rate), sequence, patchId, startTime);

    if (patchId != getPatchId())
    {
        LOG_ERR("Patch id mismatch!!!");
        // For now we update anyway as this is the most recent basal reported by pump
        // PatchId update should be handled by NotificationPacket
    }

    mBasalType.store(type);
    mBasalRate.store(rate);
    mBasalSequence.store(sequence);

    k_mutex_lock(&mBasalStartTimeMutex, K_FOREVER);
    mBasalStartTime = startTime;
    k_mutex_unlock(&mBasalStartTimeMutex);
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
    else if (strcmp(key, reservoirLevelKey) == 0)
    {
        LOG_DBG("Loading reservoir level");
        float level;
        int len = read_cb(cb_arg, &level, sizeof(level));
        if (len != sizeof(level))
        {
            LOG_ERR("Failed to read reservoir level from settings");
            return -1;
        }
        mReservoirLevel.store(level);
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
    else
    {
        LOG_ERR("Unknown key: %s", key);
        return -1;
    }

    return 0;
}
