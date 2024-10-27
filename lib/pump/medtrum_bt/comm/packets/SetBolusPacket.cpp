#include <cmath>
#include <pump/medtrum_bt/comm/enums/CommandType.h>
#include <pump/medtrum_bt/comm/packets/SetBolusPacket.h>
#include <pump/medtrum_bt/utils/Vector.h>

SetBolusPacket::SetBolusPacket(float insulin) : mInsulin(insulin)
{
    // Initialize member variables
    mOpCode = CommandType::SET_BOLUS;
}

std::vector<uint8_t> &SetBolusPacket::getRequest()
{
    // Bolus types:
    // 1 = normal
    // 2 = Extended
    // 3 = Combi

    mRequest.push_back(mOpCode);
    mRequest.push_back(0x01); // Bolus type, only support for normal bolus for now
    vector_add_le16(mRequest, static_cast<uint16_t>(round(mInsulin / 0.05F)));
    mRequest.push_back(0x00);
    return mRequest;
}
