#include <ble/BLEComm.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/settings/settings.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_ble);

namespace 
{
    static struct bt_conn_le_create_param *create_param = BT_CONN_LE_CREATE_CONN;
    static struct bt_le_conn_param *param = BT_LE_CONN_PARAM_DEFAULT;
}

struct bt_conn_cb BLEComm::connCallbacks = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = securityChanged,
};

bool BLEComm::connect(bt_addr_le_t &peer, BleConnection *connection)
{
    if (connection == nullptr || connection->callback == nullptr)
    {
        return false;
    }
    
    int err = bt_conn_le_create(&peer, create_param, param, &connection->conn);
    if (err)
    {
        LOG_ERR("Connection failed (err %d)", err);
        return false;
    }
    return true;
}

void BLEComm::disconnect(BleConnection *connection)
{
    if (connection == nullptr)
    {
        return;
    }
    bt_conn_disconnect(connection->conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);

}

void BLEComm::connected(struct bt_conn *conn, uint8_t err)
{
    LOG_DBG("Connected");
    if (err)
    {
        LOG_ERR("Connection failed (err %u)", err);
    }
    else
    {
        LOG_INF("Connected");
    }
    auto callback = CONTAINER_OF(conn, BleConnection, conn)->callback;
    if (callback != nullptr)
    {
        callback->onConnected(conn, err);
    }
}

void BLEComm::disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason 0x%02x)", reason);
    auto callback = CONTAINER_OF(conn, BleConnection, conn)->callback;
    if (callback != nullptr)
    {
        callback->onDisconnected(conn, reason);
    }
    bt_conn_unref(conn);
}

void BLEComm::securityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    if (!err)
    {
        LOG_INF("Security changed: level %u", level);
    }
    else
    {
        LOG_ERR("Security failed: level %u, err %d", level, err);
    }
    auto callback = CONTAINER_OF(conn, BleConnection, conn)->callback;
    if (callback != nullptr)
    {
        callback->onSecurityChanged(conn, level, err);
    }
}

void BLEComm::btReady(int err)
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
    bt_conn_cb_register(&connCallbacks);
}

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
