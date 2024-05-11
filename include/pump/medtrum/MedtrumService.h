#ifndef MEDTRUM_SERVICE_H
#define MEDTRUM_SERVICE_H

#include <pump/IPumpService.h>
#include <pump/medtrum/comm/PumpBleComm.h>


class MedtrumService : public IPumpService {
public:
    MedtrumService();
    ~MedtrumService();
    void init() override;

private:
    PumpBleComm pumpBleComm;
};

#endif // MEDTRUM_SERVICE_H
