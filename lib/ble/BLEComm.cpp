#include <ble/BLEComm.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/settings/settings.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_ble);

std::map<bt_addr_le_t, BleConnection *, BLEComm::CompareBtAddr> BLEComm::mConnections;

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

int BLEComm::connect(bt_addr_le_t &peer, BleConnection *connection)
{
    if (connection == nullptr || connection->callback == nullptr)
    {
        return -EINVAL;
    }
    int err = bt_conn_le_create(&peer, create_param, param, &connection->conn);
    if (err)
    {
        LOG_ERR("Connection failed (err %d)", err);
        return err;
    }
    // Save the connection object to the connection map
    mConnections[peer] = connection;
    return err;
}

void BLEComm::disconnect(BleConnection *connection)
{
    LOG_DBG("Disconnecting");
    if (connection == nullptr)
    {
        return;
    }
    bt_conn_disconnect(connection->conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
}

int BLEComm::discover(BleConnection *connection, struct bt_gatt_discover_params *params)
{
    if (connection == nullptr || connection->conn == nullptr || params == nullptr)
    {
        LOG_ERR("Connection or params is null");
        return -EINVAL;
    }
    if (params->func == nullptr)
    {
        params->func = onDiscover;
    }
    return bt_gatt_discover(connection->conn, params);
}

uint8_t BLEComm::onDiscover(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                            struct bt_gatt_discover_params *params)
{
    auto connection = mConnections[*bt_conn_get_dst(conn)];
    if (connection != nullptr && connection->callback != nullptr)
    {
        return connection->callback->onDiscover(conn, attr, params);
    }
    else
    {
        return BT_GATT_ITER_STOP;
    }
}

int BLEComm::subscribe(BleConnection *connection, struct bt_gatt_subscribe_params *params)
{
    if (connection == nullptr || connection->conn == nullptr || params == nullptr)
    {
        LOG_ERR("Connection or params is null");
        return -EINVAL;
    }
    if (params->notify == nullptr)
    {
        params->notify = onGattChanged;
    }
    return bt_gatt_subscribe(connection->conn, params);
}

uint8_t BLEComm::onGattChanged(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data,
                               uint16_t length)
{
    LOG_DBG("Gatt changed");
    auto connection = mConnections[*bt_conn_get_dst(conn)];
    if (connection != nullptr && connection->callback != nullptr)
    {
        connection->callback->onGattChanged(conn, params, data, length);
    }
    else
    {
        LOG_ERR("Connection object not found");
    }
    if (!data)
    {
        return BT_GATT_ITER_STOP;
    }
    return BT_GATT_ITER_CONTINUE;
}

void BLEComm::connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        LOG_ERR("Connection failed (err %u)", err);
        bt_conn_unref(conn);
    }
    else
    {
        LOG_INF("Connected");
    }
    // get connection object from mConnections map
    auto connection = mConnections[*bt_conn_get_dst(conn)];
    if (connection != nullptr && connection->callback != nullptr)
    {
        connection->callback->onConnected(conn, err);
    }
    else
    {
        LOG_ERR("Connection object not found");
    }
}

void BLEComm::disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected (reason 0x%02x)", reason);
    auto connection = mConnections[*bt_conn_get_dst(conn)];
    if (connection != nullptr && connection->callback != nullptr)
    {
        bt_conn_unref(connection->conn);
        connection->callback->onDisconnected(conn, reason);
    }
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
    auto connection = mConnections[*bt_conn_get_dst(conn)];
    if (connection != nullptr && connection->callback != nullptr)
    {
        connection->callback->onSecurityChanged(conn, level, err);
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
