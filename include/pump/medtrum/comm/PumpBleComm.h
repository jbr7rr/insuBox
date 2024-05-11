#ifndef PUMP_BLE_COMM_H
#define PUMP_BLE_COMM_H

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

class PumpBleComm {
public:
    PumpBleComm();
    ~PumpBleComm();
    void init();

    void connect();
private:
    
};

#endif // PUMP_BLE_COMM_H