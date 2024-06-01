#include <pump/medtrum/MedtrumPumpSync.h>

#include <string>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

namespace
{
    constexpr char subKey[] = "medtrum";
    constexpr char pumpStateKey[] = "pump_state";
    constexpr char reservoirLevelKey[] = "reservoir_level";
}

LOG_MODULE_REGISTER(ib_medtrum_pump_sync);

MedtrumPumpSync::MedtrumPumpSync() { LOG_DBG("Initializing MedtrumPumpSync"); }

void MedtrumPumpSync::init()
{
    LOG_DBG("Initializing MedtrumPumpSync");
    settings_subsys_init();

    // Load the settings on initialization
    settings_load_subtree_direct(
        subKey,
        [](const char *key, size_t len, settings_read_cb read_cb, void *cb_arg, void *param) {
            return static_cast<MedtrumPumpSync *>(param)->loadCb(key, len, read_cb, cb_arg, param);
        },
        this);
}

MedtrumPumpSync::~MedtrumPumpSync()
{
    // Optional: handle any cleanup if necessary
}

void MedtrumPumpSync::setPumpState(PumpState state)
{
    mPumpState.store(state);
    std::string key = std::string(subKey) + "/" + pumpStateKey;
    settings_save_one(key.c_str(), &state, sizeof(state));
}

PumpState MedtrumPumpSync::getPumpState() { return mPumpState.load(); }

void MedtrumPumpSync::setReservoirLevel(float level)
{
    mReservoirLevel.store(level);
    std::string key = std::string(subKey) + "/" + reservoirLevelKey;
    settings_save_one(key.c_str(), &level, sizeof(level));
}

float MedtrumPumpSync::getReservoirLevel() { return mReservoirLevel.load(); }

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
    else
    {
        LOG_ERR("Unknown key: %s", key);
        return -1;
    }

    return 0;
}
