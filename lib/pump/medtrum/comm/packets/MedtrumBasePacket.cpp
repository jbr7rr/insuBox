#include "../../utils/CrcUtil.h"
#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>

#include <zephyr/sys/byteorder.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(ib_medtrum_base_packet);

namespace
{
    constexpr size_t HEADER_LENGTH_OFFSET = 0;
    constexpr size_t HEADER_COMMAND_OFFSET = 1;
    constexpr size_t HEADER_SEQUENCE_NO_OFFSET = 2;
    constexpr size_t HEADER_PACKAGE_IDX_OFFSET = 3;
    constexpr size_t HEADER_SIZE = 4;
    constexpr size_t RESULT_SIZE = 2;

    constexpr uint16_t RESPONSE_SUCCESS = 0x0000;
    constexpr uint16_t RESPONSE_WAITING = 0x4000;
}

std::vector<uint8_t> &MedtrumBasePacket::getRequest()
{
    mRequest.push_back(mOpCode);
    return mRequest;
}

bool MedtrumBasePacket::onNotification(const uint8_t *data, size_t dataSize)
{
    LOG_DBG("onNotification");
    // Response: Header: 4 bytes, Data: 15 bytes, CRC: 1 byte
    // Header: Length, Command, Sequence number, Package index

    if (dataSize < HEADER_SIZE)
    {
        LOG_ERR("Invalid data size");
        mFailed = true;
        return true;
    }

    // Check crc of data
    uint8_t crc = CrcUtil::crc8(data, dataSize - 1);
    if (crc != data[dataSize - 1])
    {
        LOG_ERR("CRC failed");
        mFailed = true;
        return true;
    }

    if (mOpCode != data[HEADER_COMMAND_OFFSET])
    {
        LOG_ERR("Invalid opcode");
        mFailed = true;
        return true;
    }

    // Total length = 1x header + total data size excluding other headers and crc's
    uint8_t responseSize = data[HEADER_LENGTH_OFFSET];

    if (responseSize + 1 > 20)
    {
        // Packet with one message has pkgIndex of 0, otherwise start with 1
        mPkgIndex++;
    }

    if (mPkgIndex != data[HEADER_PACKAGE_IDX_OFFSET])
    {
        LOG_ERR("Invalid package index");
        mFailed = true;
        return true;
    }

    if (mPkgIndex < 2)
    {
        // For first packet don't strip header
        mResponse.insert(mResponse.end(), data, data + dataSize - 1);
    } else
    {
        // Strip header and crc
        mResponse.insert(mResponse.end(), &data[HEADER_SIZE], &data[dataSize - 1]);
    }
    if (responseSize >= mResponse.size())
    {
        return handleResponse();
    }
    return false;
}

bool MedtrumBasePacket::isFailed() const { return mFailed; }

bool MedtrumBasePacket::handleResponse()
{
    LOG_DBG("Handling response");
    bool ready = false;
    if (mResponse.size() < HEADER_SIZE + RESULT_SIZE)
    {
        LOG_ERR("Invalid response size");
        mFailed = true;
        return true;
    }

    uint16_t responseCode = sys_get_le16(&mResponse[HEADER_SIZE]);
    switch (responseCode)
    {
        case RESPONSE_SUCCESS:
            LOG_DBG("Response success");
            if (mResponse.size() < mExpectedLength)
            {
                mFailed = true;
                LOG_ERR("Invalid response length");
                LOG_HEXDUMP_DBG(mResponse.data(), mResponse.size(), "Response: ");
            }
            ready = true;
            break;
        case RESPONSE_WAITING:
            LOG_DBG("Response waiting");
            ready = false;
            break;
        default:
            LOG_ERR("Response error");
            LOG_HEXDUMP_DBG(mResponse.data(), mResponse.size(), "Response: ");
            mFailed = true;
            ready = true;
            break;
    }

    return ready;
}
