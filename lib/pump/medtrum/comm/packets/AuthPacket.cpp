#include <pump/medtrum/comm/packets/AuthPacket.h>

#include <pump/medtrum/crypt/Crypt.h>
#include <pump/medtrum/utils/Vector.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ib_medtrum_auth_packet);

namespace
{
    constexpr uint8_t OPCODE = 0x05;
    constexpr size_t RESP_DEVICE_TYPE_START = 7;
    constexpr size_t RESP_VERSION_X_START = 8;
    constexpr size_t RESP_VERSION_Y_START = 9;
    constexpr size_t RESP_VERSION_Z_START = 10;
}

AuthPacket::AuthPacket(uint32_t deviceSN) : mDeviceSN(deviceSN)
{
    mOpCode = OPCODE;
    mExpectedLength = RESP_VERSION_Z_START + 1;
}

std::vector<uint8_t> &AuthPacket::getRequest()
{
    // TODO: Save token somewhere, and ability to get external token
    uint32_t sessionToken = 0; //Crypt::generateRandomToken();
    uint32_t key = Crypt::keyGen(mDeviceSN);

    mRequest.push_back(OPCODE);
    mRequest.push_back(0x02); // Role, 0x02 for pump
    vector_add_le32(mRequest, sessionToken);
    vector_add_le32(mRequest, key);

    return mRequest;
}

bool AuthPacket::handleResponse()
{
    LOG_DBG("Handling AuthPacket response");
    bool ready = MedtrumBasePacket::handleResponse();
    if (!mFailed && ready)
    {
        uint8_t deviceType = mResponse[RESP_DEVICE_TYPE_START];
        uint8_t versionX = mResponse[RESP_VERSION_X_START];
        uint8_t versionY = mResponse[RESP_VERSION_Y_START];
        uint8_t versionZ = mResponse[RESP_VERSION_Z_START];
        LOG_DBG("Device type: %d, Version: %d.%d.%d", deviceType, versionX, versionY, versionZ);
        // TODO: Do somethign with this or remove it
    }
    return ready;
}
