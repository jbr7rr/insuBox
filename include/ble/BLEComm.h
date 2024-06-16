#ifndef BLE_COMM_H
#define BLE_COMM_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include <cstdint>
#include <map>
#include <string>
#include <vector>

class IBLECallback
{
public:
    virtual void onConnected(struct bt_conn *conn, uint8_t err) = 0;
    virtual void onDisconnected(struct bt_conn *conn, uint8_t reason) = 0;
    virtual void onSecurityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) = 0;

    virtual uint8_t onDiscover(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                               struct bt_gatt_discover_params *params) = 0;
    virtual uint8_t onGattChanged(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data,
                                  uint16_t length) = 0;
};

struct BleConnection
{
    IBLECallback *callback = nullptr;
    struct bt_conn *conn = nullptr;
};

/*
 * @brief BLEComm class
 *
 * This class is responsible for handling the BLE communication.
 * It wraps the Zephyr BLE stack and provides the necessary functionality to C++ classes,
 * so multiple classes can connect to different BLE devices, without themselves having to
 * handle the static BLE callback functions.
 *
 * For now this class doesn't handle the pointers, it expects the user to handle the memory if needed.
 *
 * @note This class is a singleton as zephyr BLE stack requires static interface.
 * Using the callback structure of this class it is possible to handle multiple connections
 * from different non-static classes.
 *
 */

class BLEComm
{
public:
    static void init();
    static int connect(bt_addr_le_t &peer, BleConnection *connection);
    static void disconnect(BleConnection *connection);

    // Maybe make this part of a BT gatt class ?
    static int discover(BleConnection *connection, struct bt_gatt_discover_params *params);
    static int subscribe(BleConnection *connection, struct bt_gatt_subscribe_params *params);

private:
    struct CompareBtAddr
    {
        bool operator()(const bt_addr_le_t &lhs, const bt_addr_le_t &rhs) const
        {
            return bt_addr_le_cmp(&lhs, &rhs) < 0;
        }
    };

    static std::map<bt_addr_le_t, BleConnection *, CompareBtAddr> mConnections;

    static void connected(struct bt_conn *conn, uint8_t err);
    static void disconnected(struct bt_conn *conn, uint8_t reason);
    static void securityChanged(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);
    static void btReady(int err);
    static struct bt_conn_cb connCallbacks;

    static uint8_t onDiscover(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                              struct bt_gatt_discover_params *params);
    static uint8_t onGattChanged(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data,
                                 uint16_t length);
};

#endif // BLE_COMM_H
