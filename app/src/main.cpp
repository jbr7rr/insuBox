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
    pumpService.init();
    return 0;
}