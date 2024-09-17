#ifndef SET_TEMP_BASAL_PACKET_H
#define SET_TEMP_BASAL_PACKET_H

#include <pump/medtrum/MedtrumPumpSync.h>
#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>

class SetTempBasalPacket : public MedtrumBasePacket
{
public:
    SetTempBasalPacket(MedtrumPumpSync &pumpSync, float absoluteRate, uint16_t durationInMinutes);
    virtual ~SetTempBasalPacket() = default;

    std::vector<uint8_t> &getRequest() override;

protected:
    void handleResponse() override;

private:
    MedtrumPumpSync &mPumpSync;

    float mAbsoluteRate;
    uint16_t mDurationInMinutes;
};

#endif // SET_TEMP_BASAL_PACKET_H
