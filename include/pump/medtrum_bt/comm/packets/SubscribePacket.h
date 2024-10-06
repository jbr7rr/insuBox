#ifndef SUBSCRIBE_PACKET_H
#define SUBSCRIBE_PACKET_H

#include <pump/medtrum_bt/comm/packets/MedtrumBasePacket.h>

class SubscribePacket : public MedtrumBasePacket
{
public:
    SubscribePacket();
    virtual ~SubscribePacket() = default;

    std::vector<uint8_t> &getRequest() override;
};

#endif // SUBSCRIBE_PACKET_H
