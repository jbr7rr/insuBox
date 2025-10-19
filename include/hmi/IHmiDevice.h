#ifndef IHMI_DEVICE_H
#define IHMI_DEVICE_H

#include <ble/BLEComm.h>
#include <cstdint>
#include <pump/PumpService.h>

class IHmiCallback
{
public:
    /**
     * @brief Send a response to a user Bluetooth pairing request
     *
     * @param conn Connection object
     * @param accepted True if the user accepted the pairing request, false otherwise
     */
    virtual void userBtPairingResponse(struct bt_conn *conn, bool accepted) = 0;

    /**
     * @brief Request a bolus from the pump
     *
     * @param amount Amount of insulin to be delivered
     * @param timestamp Timestamp of the request
     */
    virtual void bolusRequest(float amount, time_t timestamp) = 0;

    /**
     * @brief Request to stop the current bolus delivery
     */
    virtual void stopBolusRequest() = 0;

    /**
     * @brief Request to retract the last bolus delivery
     */
    virtual void retractRequest() = 0;

    /**
     * @brief Request to prime the pump
     */
    virtual void primeRequest() = 0;
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

    /**
     * @brief Handle changes in Bluetooth state
     *
     * @param conn Connection object
     * @param state New Bluetooth state
     */
    virtual void onBtBluetoothStateChanged(struct bt_conn *conn, BtState state) = 0;

    /**
     * @brief Handle updates to the bolus progress
     *
     * @param update Bolus progress update containing requested and delivered amounts and timestamps
     */
    virtual void onBolusProgressUpdate(BolusProgressUpdate &update) = 0;
};

#endif // IHMI_DEVICE_H
