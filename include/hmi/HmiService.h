#ifndef HMI_SERVICE_H
#define HMI_SERVICE_H

#include <hmi/IHmiDevice.h>

class HmiService
{
public:
    HmiService(IHmiDevice &hmiDevice = HmiService::getHmiDevice());
    ~HmiService();
    void init();

private:
    IHmiDevice &mHmiDevice;

    /**
     * @brief Get the hmi internal hmi device object of the selected type in Kconfig
     *
     * @return IHmiDevice&
     */
    static IHmiDevice &getHmiDevice();
};

#endif // HMI_SERVICE_H
