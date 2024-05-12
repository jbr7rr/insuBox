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

bool BLEComm::connect(bt_addr_le_t &peer, BleConnection *connection)
{
    if (connection == nullptr || connection->callback == nullptr)
    {
        return false;
    }
    struct bt_conn *conn;
    int err = bt_conn_le_create(&peer, create_param, param, &conn);
    if (err)
    {
        LOG_ERR("Connection failed (err %d)", err);
        return false;
    }
    // Unref the connection object here as we will use the one we get when connected
    bt_conn_unref(conn);
    // Save the connection object to the connection map
    mConnections[peer] = connection;
    return true;
}

void BLEComm::disconnect(BleConnection *connection)
{
    if (connection == nullptr)
    {
        return;
    }
    bt_conn_disconnect(connection->conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
    bt_conn_unref(connection->conn);
}

bool BLEComm::discoverServices(BleConnection *connection)
{
    LOG_DBG("Discovering services");
    if (connection == nullptr || connection->conn == nullptr)
    {
        LOG_ERR("Connection is null");
        return false;
    }
    static struct bt_gatt_discover_params discover_params;
    discover_params.uuid = NULL;
    discover_params.func = discover_func;
    discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
    discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
    discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

    int err = bt_gatt_discover(connection->conn, &discover_params);
    if (err)
    {
        LOG_ERR("Discover failed (err %d)", err);
        return false;
    }
    return true;
}

uint8_t BLEComm::discover_func(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                               struct bt_gatt_discover_params *params)
{
    if (!attr)
    {
        LOG_INF("Discover complete");
        return BT_GATT_ITER_STOP;
    }
    LOG_DBG("Discovered attribute handle %u", attr->handle);
    auto connection = mConnections[*bt_conn_get_dst(conn)];
    if (connection != nullptr && connection->callback != nullptr)
    {
        connection->callback->onAttributeDiscovered(attr);
    }
    return BT_GATT_ITER_CONTINUE;
}

void BLEComm::connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        LOG_ERR("Connection failed (err %u)", err);
    }
    else
    {
        LOG_INF("Connected");
    }
    // get connection object from mConnections map
    auto connection = mConnections[*bt_conn_get_dst(conn)];
    if (connection != nullptr && connection->callback != nullptr)
    {
        LOG_DBG("Connection object found");
        connection->conn = conn;
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
        connection->callback->onDisconnected(conn, reason);
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
