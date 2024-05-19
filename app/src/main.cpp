#include <ble/BLEComm.h>
#include <pump/PumpService.h>

namespace
{
    BLEComm bleComm = BLEComm();
    PumpService pumpService;
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