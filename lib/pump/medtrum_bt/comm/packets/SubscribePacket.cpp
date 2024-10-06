#include <pump/medtrum_bt/comm/enums/CommandType.h>
#include <pump/medtrum_bt/comm/packets/SubscribePacket.h>
#include <pump/medtrum_bt/utils/Vector.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ib_medtrum_subscribe_packet);

SubscribePacket::SubscribePacket()
{
    // Initialize member variables
    mOpCode = CommandType::SUBSCRIBE;
}

std::vector<uint8_t> &SubscribePacket::getRequest()
{
    mRequest.push_back(mOpCode);
    vector_add_le16(mRequest, 0x0FFF); // Mask ?
    return mRequest;
}
