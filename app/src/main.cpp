#include <ble/BLEComm.h>

namespace
{
    BLEComm bleComm = BLEComm();
}

extern "C" int main(void)
{
    bleComm.init();
    return 0;
}
