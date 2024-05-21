#ifndef SYNCHRONIZE_PACKET_H
#define SYNCHRONIZE_PACKET_H

#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>

class SynchronizePacket : public MedtrumBasePacket
{
public:
    SynchronizePacket();
    virtual ~SynchronizePacket() = default;

protected:
    bool handleResponse() override;
};

#endif // SYNCHRONIZE_PACKET_H
