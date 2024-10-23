#ifndef HMI_SERVICE_H
#define HMI_SERVICE_H

#include <hmi/IHmiDevice.h>
#include <events/EventDispatcher.h>
#include <ble/BLEComm.h>

class HmiService
{
public:
    HmiService(EventDispatcher &dispatcher, IHmiDevice &hmiDevice = HmiService::getHmiDevice());
    ~HmiService();
    void init();

private:
    EventDispatcher &mDispatcher;
    IHmiDevice &mHmiDevice;

    /**
     * @brief Get the hmi internal hmi device object of the selected type in Kconfig
     *
     * @return IHmiDevice&
     */
    static IHmiDevice &getHmiDevice();

    void onBtPassKeyConfirmRequest(const BtPassKeyConfirmRequest &request);

};

#endif // HMI_SERVICE_H
