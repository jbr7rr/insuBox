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
    constexpr uint8_t RESP_FIELDS_END = RESP_FIELDS_START + 2;
    constexpr uint8_t RESP_SYNC_DATA_START = 9;

    constexpr uint8_t MASK_SUSPEND = 0x01;
    constexpr uint8_t MASK_NORMAL_BOLUS = 0x02;
    constexpr uint8_t MASK_EXTENDED_BOLUS = 0x04;
}

SynchronizePacket::SynchronizePacket()
{
    mOpCode = CommandType::SYNCHRONIZE;
    mExpectedLength = RESP_SYNC_DATA_START + 1;
}

bool SynchronizePacket::handleResponse()
{
    LOG_DBG("Handling SynchronizePacket response");
    bool ready = MedtrumBasePacket::handleResponse();
    if (ready && !mFailed)
    {
        uint8_t state = mResponse[RESP_STATE_START];

        LOG_DBG("SynchronizePacket: state: %d", state);

        // uint16_t fieldMask = ByteArrayExtension::toInt(mResponse.data() + RESP_FIELDS_START, 2);
        // std::vector<uint8_t> syncData(mResponse.begin() + RESP_SYNC_DATA_START, mResponse.end());

        uint16_t fieldMask = sys_get_le16(mResponse.data() + RESP_FIELDS_START);

        LOG_DBG("SynchronizePacket: fieldMask: %d", fieldMask);

        // TODO: Extract data from sync data (Notification Packet)
    }

    return ready;
}
