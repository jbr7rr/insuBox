#ifndef SYNCHRONIZE_PACKET_H
#define SYNCHRONIZE_PACKET_H

#include <pump/medtrum_bt/comm/packets/MedtrumBasePacket.h>
#include <pump/medtrum_bt/MedtrumPumpSync.h>

class MedtrumPumpSync;

class SynchronizePacket : public MedtrumBasePacket
{
public:
    SynchronizePacket(MedtrumPumpSync &pumpSync);
    virtual ~SynchronizePacket() = default;

protected:
    void handleResponse() override;

private:
    MedtrumPumpSync &mPumpSync;
};

#endif // SYNCHRONIZE_PACKET_H
