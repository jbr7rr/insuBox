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
    pumpService.init();
    return 0;
}