#ifndef IHMI_DEVICE_H
#define IHMI_DEVICE_H

#include <ble/BLEComm.h>
#include <cstdint>
#include <pump/PumpService.h>

class IHmiCallback
{
public:
    /**
     * @brief Handle the user response to the pairing request
     *
     * @param conn Connection object
     * @param accepted True if the user accepted the pairing request, false otherwise
     */
    virtual void onUserBtPairingResponse(struct bt_conn *conn, bool accepted) = 0;
    virtual void onBolusRequest(float amount, time_t timestamp) = 0;
    virtual void onStopBolus() = 0;
    virtual void onRetractRequest() = 0;
};

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
    virtual void onBtBluetoothStateChanged(struct bt_conn *conn, BtState state) = 0;
    virtual void onBolusProgressUpdate(BolusProgressUpdate &update) = 0;
};

#endif // IHMI_DEVICE_H
