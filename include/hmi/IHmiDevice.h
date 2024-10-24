#ifndef IHMI_DEVICE_H
#define IHMI_DEVICE_H

#include <cstdint>

class IHmiDevice
{
public:
    virtual ~IHmiDevice() = default;
    virtual void init() = 0;

    /**
     * @brief Handle the pairing request from the BLE stack
     *
     * @param conn Connection object
     * @param passkey Passkey to be displayed to the user
     */
    virtual void onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey) = 0;
};

#endif // IHMI_DEVICE_H
