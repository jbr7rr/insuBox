#ifndef SET_BOLUS_PACKET_H
#define SET_BOLUS_PACKET_H

#include <pump/medtrum_bt/comm/packets/MedtrumBasePacket.h>

class SetBolusPacket : public MedtrumBasePacket
{
public:
    SetBolusPacket(float insulin);
    virtual ~SetBolusPacket() = default;

    std::vector<uint8_t> &getRequest() override;

private:
    float mInsulin = 0;
};

#endif // SET_BOLUS_PACKET_H
