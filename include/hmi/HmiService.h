#ifndef HMI_SERVICE_H
#define HMI_SERVICE_H

#include <ble/BLEComm.h>
#include <events/EventDispatcher.h>
#include <hmi/IHmiDevice.h>

#include <zephyr/zbus/zbus.h>

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
};

class HmiService : public IHmiCallback
{
public:
    HmiService(EventDispatcher &dispatcher);
    HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice);

    ~HmiService();
    void init();

    void onUserBtPairingResponse(struct bt_conn *conn, bool accepted) override;

private:
    EventDispatcher &mDispatcher;
    IHmiDevice &mHmiDevice;

    struct PassKeyDisplayTask
    {
        HmiService *service;
        struct k_work work;
        struct bt_conn *conn;
        unsigned int passkey;
    };
    PassKeyDisplayTask mPassKeyDisplayTask;

    k_work_q mWorkQueue;
    K_KERNEL_STACK_MEMBER(mWorkQueueBuffer, KB(2));

    /**
     * @brief Get the hmi internal hmi device object of the selected type in Kconfig
     *
     * @param hmiCallback The hmi callback object to initialize the hmi device with
     * @return IHmiDevice&
     */
    static IHmiDevice &getHmiDevice(IHmiCallback &hmiCallback);
};

#endif // HMI_SERVICE_H
