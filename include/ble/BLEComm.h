#ifndef BLE_COMM_H
#define BLE_COMM_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#include <cstdint>
#include <map>

class IBLECallback
{
public:
    virtual void onConnected(struct bt_conn *conn, uint8_t err) = 0;
    virtual void onDisconnected(struct bt_conn *conn, uint8_t reason) = 0;
    virtual void onSecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) = 0;
};

struct BleConnection {
    IBLECallback *callback;
    struct bt_conn *conn;
};

class BLEComm
{
public:
    static void init();
    static bool connect(bt_addr_le_t &peer, BleConnection *connection);
    static void disconnect(BleConnection *connection);

private:
    static void connected(struct bt_conn *conn, uint8_t err);
    static void disconnected(struct bt_conn *conn, uint8_t reason);
    static void securityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);
    static void btReady(int err);
    static struct bt_conn_cb connCallbacks;
};

#endif // BLE_COMM_H
