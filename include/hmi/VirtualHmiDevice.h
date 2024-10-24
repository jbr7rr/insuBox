#ifndef VIRTUAL_HMI_DEVICE_H
#define VIRTUAL_HMI_DEVICE_H

#include "HmiService.h"
#include "IHmiDevice.h"

class VirtualHmiDevice : public IHmiDevice
{
public:
    VirtualHmiDevice(IHmiCallback &hmiCallback);
    ~VirtualHmiDevice() override;
    void init() override;

    void onUserBtPairingRequest(struct bt_conn *conn, uint32_t passkey) override;

private:
    IHmiCallback &mHmiCallback;
};

#endif // VIRTUAL_HMI_DEVICE_H
