#ifndef HMI_SERVICE_H
#define HMI_SERVICE_H

#include <events/EventDispatcher.h>
#include <ble/BLEComm.h>

class HmiService
{
public:
    HmiService(EventDispatcher &dispatcher);
    ~HmiService();
    void init();
private:
    EventDispatcher &mDispatcher;

    void onBtPassKeyConfirmRequest(const BtPassKeyConfirmRequest &request);

};

#endif // HMI_SERVICE_H