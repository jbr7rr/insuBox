#ifndef MEDTRUM_DEVICE_H
#define MEDTRUM_DEVICE_H

#include <pump/IPumpDevice.h>
#include <pump/medtrum/comm/PumpBleComm.h>


class MedtrumDevice : public IPumpDevice {
public:
    MedtrumDevice();
    ~MedtrumDevice();
    void init() override;

private:
    PumpBleComm pumpBleComm;
};

#endif // MEDTRUM_DEVICE_H
