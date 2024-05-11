#include <ble/BLEComm.h>
#include <pump/IPumpService.h>
#include <pump/medtrum/MedtrumService.h>

namespace
{
    BLEComm bleComm = BLEComm();
    MedtrumService medtrumService;
    IPumpService& pumpService = medtrumService;
}

extern "C" int main(void)
{
    bleComm.init();

    // Wait for the BLE stack to be ready
    // TODO: This should be done via callback or event
    k_sleep(K_SECONDS(1));

    pumpService.init();
    return 0;
}