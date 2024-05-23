#ifndef AUTH_PACKET_H
#define AUTH_PACKET_H

#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>

class AuthPacket : public MedtrumBasePacket
{
public:
    AuthPacket(uint32_t deviceSN);
    virtual ~AuthPacket() = default;

    std::vector<uint8_t> &getRequest() override;
protected:
    void handleResponse() override;
private:
    uint32_t mDeviceSN;
};

#endif // AUTH_PACKET_H
