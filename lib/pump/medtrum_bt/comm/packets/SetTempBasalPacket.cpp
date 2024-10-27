#include <pump/medtrum_bt/comm/enums/BasalType.h>
#include <pump/medtrum_bt/comm/enums/CommandType.h>
#include <pump/medtrum_bt/comm/packets/SetTempBasalPacket.h>
#include <pump/medtrum_bt/utils/MedtrumTimeUtil.h>
#include <pump/medtrum_bt/utils/Vector.h>

#include <cmath>
#include <zephyr/sys/byteorder.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ib_medtrum_set_temp_basal_packet);

namespace
{
    constexpr size_t RESP_BASAL_TYPE_START = 6;
    constexpr size_t RESP_BASAL_RATE_START = 7;
    constexpr size_t RESP_BASAL_SEQUENCE_START = 9;
    constexpr size_t RESP_BASAL_PATCH_ID_START = 11;
    constexpr size_t RESP_BASAL_START_TIME_START = 13;
}

SetTempBasalPacket::SetTempBasalPacket(MedtrumPumpSync &pumpSync, float absoluteRate, uint16_t durationInMinutes)
    : mPumpSync(pumpSync), mAbsoluteRate(absoluteRate), mDurationInMinutes(durationInMinutes)
{
    mOpCode = CommandType::SET_TEMP_BASAL;
    mExpectedLength = RESP_BASAL_START_TIME_START + 4;
}

std::vector<uint8_t> &SetTempBasalPacket::getRequest()
{
    /**
     * byte 0: opCode
     * byte 1: tempBasalType
     * byte 2-3: tempBasalRate
     * byte 4-5: tempBasalDuration
     */

    mRequest.push_back(mOpCode);
    mRequest.push_back(0x06); // Absolute tbr
    vector_add_le16(mRequest, static_cast<uint16_t>(round(mAbsoluteRate / 0.05F)));
    vector_add_le16(mRequest, mDurationInMinutes);
    return mRequest;
}

void SetTempBasalPacket::handleResponse()
{
    LOG_DBG("Handling SetTempBasalPacket response");
    MedtrumBasePacket::handleResponse();
    if (!mFailed && mReady)
    {
        BasalType basalType = mResponse[RESP_BASAL_TYPE_START];
        float basalRate = sys_get_le16(mResponse.data() + RESP_BASAL_RATE_START) * 0.05;
        uint16_t basalSequence = sys_get_le16(mResponse.data() + RESP_BASAL_SEQUENCE_START);
        uint16_t basalPatchId = sys_get_le16(mResponse.data() + RESP_BASAL_PATCH_ID_START);

        uint32_t rawTime = sys_get_le32(mResponse.data() + RESP_BASAL_START_TIME_START);
        int64_t basalStartTime = MedtrumTimeUtil::convertPumpTimeToSystemTime(rawTime);

        mPumpSync.handleBasalStatusUpdate(basalType, basalRate, basalSequence, basalPatchId, basalStartTime);
    }
    return;
}
