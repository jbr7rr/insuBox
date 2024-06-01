#ifndef SYNCHRONIZE_PACKET_H
#define SYNCHRONIZE_PACKET_H

#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>

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
