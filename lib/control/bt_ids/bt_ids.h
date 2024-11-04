#ifndef BT_IDS_H
#define BT_IDS_H

#include <control/bt_ids/InsulinDeliveryDevice.h>

namespace bt_ids
{
    void init(IInsulinDeliveryDeviceCallback &insulinDeliveryDevice);
    const bt_gatt_service_static &getService();
};

#endif // BT_IDS_H