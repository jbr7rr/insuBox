#ifndef HMI_SERVICE_H
#define HMI_SERVICE_H

#include <ble/BLEComm.h>
#include <events/EventDispatcher.h>
#include <hmi/IHmiDevice.h>
#include <pump/PumpService.h>

class HmiService : public IHmiCallback
{
public:
    HmiService(EventDispatcher &dispatcher);
    HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice);

    ~HmiService();
    void init();

protected:
    void userBtPairingResponse(struct bt_conn *conn, bool accepted) override;
    void bolusRequest(float amount, time_t timestamp) override;
    void stopBolusRequest() override;
    void retractRequest() override;
    void primeRequest() override;

private:
    EventDispatcher &mDispatcher;
    IHmiDevice &mHmiDevice;

    struct SimpleTask
    {
        HmiService *service;
        struct k_work work;
    };
    SimpleTask mInitTask;
    SimpleTask mBtBluetoothStateChangedTask;

    struct PassKeyDisplayTask
    {
        HmiService *service;
        struct k_work work;
        struct bt_conn *conn;
        unsigned int passkey;
    };
    PassKeyDisplayTask mPassKeyDisplayTask;

    struct BolusProgressUpdateTask
    {
        HmiService *service;
        struct k_work work;
        BolusProgressUpdate update;
    };
    BolusProgressUpdateTask mBolusProgressUpdateTask;

    static k_work_q mWorkQueue;
    K_KERNEL_STACK_MEMBER(mWorkQueueBuffer, CONFIG_IB_HMI_STACK_SIZE);

    /**
     * @brief Get the hmi internal hmi device object of the selected type in Kconfig
     *
     * @param hmiCallback The hmi callback object to initialize the hmi device with
     * @return IHmiDevice&
     */
    static IHmiDevice &getHmiDevice(IHmiCallback &hmiCallback);
};

#endif // HMI_SERVICE_H
