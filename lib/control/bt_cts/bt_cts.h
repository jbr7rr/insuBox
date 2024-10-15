#ifndef BT_CTS_H
#define BT_CTS_H

#include <zephyr/bluetooth/gatt.h>

#define BT_UUID_CTS_LOCAL_TIME_INFO_VAL 0x2A0F
#define BT_UUID_CTS_LOCAL_TIME_INFO BT_UUID_DECLARE_16(BT_UUID_CTS_LOCAL_TIME_INFO_VAL)

namespace bt_cts {

    void init();

} // namespace bt_cts

#endif // BT_CTS_H
