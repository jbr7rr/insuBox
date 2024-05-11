#include <ble/BLEComm.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/settings/settings.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

/* Callbacks have to be C functions, might want to move these to a separate file */
#ifdef __cplusplus
extern "C" {
#endif

LOG_MODULE_REGISTER(ib_ble);

namespace
{
    /**
     *  @brief Callback for when a connection is established
     */
    static void connected(struct bt_conn *conn, uint8_t err)
    {
        LOG_DBG("Connected");
    }

    /**
     *  @brief Callback for when a connection is terminated
     */
    static void disconnected(struct bt_conn *conn, uint8_t reason)
    {
        LOG_INF("Disconnected (reason 0x%02x)", reason);
    }

    /**
     *  @brief Callback for when the security level of a connection is changed
     */
    static void securityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
    {
        if (!err)
        {
            LOG_INF("Security changed: level %u", level);
        }
        else
        {
            LOG_ERR("Security failed: level %u, err %d", level, err);
        }
    }

    BT_CONN_CB_DEFINE(conn_callbacks) = {
        .connected = connected, .disconnected = disconnected, .security_changed = securityChanged};

    /**
     * @brief Callback for when the Bluetooth stack is ready
     */
    static void btReady(int err)
    {
        if (err != 0)
        {
            LOG_ERR("Bluetooth failed to initialise: %d", err);
        }
        else
        {
            if (IS_ENABLED(CONFIG_SETTINGS))
            {
                settings_load();
            }
            LOG_DBG("Bluetooth initialized");
        }
    }
} // namespace
#ifdef __cplusplus
}
#endif

void BLEComm::init()
{
    LOG_INF("Bluetooth initialising");
    int err = 0;
    err = bt_enable(btReady);
    if (err)
    {
        LOG_ERR("Bluetooth enable failed: %d", err);
    }
}
