#include <pump/medtrum/MedtrumPumpSync.h>
#include <pump/medtrum/comm/packets/NotificationPacket.h>
#include <pump/medtrum/comm/packets/SynchronizePacket.h>

#include <pump/medtrum/comm/enums/CommandType.h>

#include <zephyr/sys/byteorder.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ib_medtrum_synchronize_packet);

namespace
{
    constexpr uint8_t RESP_STATE_START = 6;
    constexpr uint8_t RESP_FIELDS_START = 7;

    constexpr uint8_t MASK_SUSPEND = 0x01;
    constexpr uint8_t MASK_NORMAL_BOLUS = 0x02;
    constexpr uint8_t MASK_EXTENDED_BOLUS = 0x04;
}

SynchronizePacket::SynchronizePacket(MedtrumPumpSync &pumpSync) : mPumpSync(pumpSync)
{
    mOpCode = CommandType::SYNCHRONIZE;
    mExpectedLength = RESP_FIELDS_START + 3;
}

void SynchronizePacket::handleResponse()
{
    LOG_DBG("Handling SynchronizePacket response");
    MedtrumBasePacket::handleResponse();
    if (mReady && !mFailed)
    {
        PumpState state = mResponse[RESP_STATE_START];

        LOG_DBG("SynchronizePacket: state: %d", static_cast<uint8_t>(state));

        if (mPumpSync.getPumpState() != state)
        {
            mPumpSync.setPumpState(state);
        }

        uint16_t fieldMask = sys_get_le16(mResponse.data() + RESP_FIELDS_START);

        LOG_DBG("SynchronizePacket: fieldMask: %d", fieldMask);
        bool success = NotificationPacket(mPumpSync).handleMaskedMessage(mResponse.data() + RESP_FIELDS_START,
                                                                         mResponse.size() - RESP_FIELDS_START);
    }

    return;
}
