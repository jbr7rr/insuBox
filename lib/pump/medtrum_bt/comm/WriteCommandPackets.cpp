#include "../utils/CrcUtil.h"
#include <cstring>
#include <pump/medtrum_bt/comm/WriteCommandPackets.h>

namespace
{
    constexpr size_t LENGTH_OFFSET = 0;
    constexpr size_t COMMAND_OFFSET = 1;
    constexpr size_t SEQUENCE_NO_OFFSET = 2;
    constexpr size_t PACKAGE_IDX_OFFSET = 3;
    constexpr size_t DATA_OFFSET = 4;
}

WriteCommandPackets::WriteCommandPackets(const uint8_t *data, size_t dataSize, int sequenceNumber)
    : mData(data), mDataSize(dataSize)
{
    if (data == nullptr || dataSize == 0)
    {
        mPacketsConsumed = true;
        return;
    }

    mHeader[LENGTH_OFFSET] = static_cast<uint8_t>(dataSize + 4);        // Length
    mHeader[COMMAND_OFFSET] = data[0];                                  // Command
    mHeader[SEQUENCE_NO_OFFSET] = static_cast<uint8_t>(sequenceNumber); // Sequence number
    mHeader[PACKAGE_IDX_OFFSET] = 0x00;                                 // Package index

    uint8_t headerCrc = CrcUtil::crc8(mHeader, 4);
    mTotalCrc = CrcUtil::crc8(&data[1], dataSize - 1, headerCrc);
    mIndex = 1; // Extracted the command from the data

    // Account for CRC of total data
    mDataSize++;
}

size_t WriteCommandPackets::getNextPacket(uint8_t *packet)
{
    if (packet == nullptr || mPacketsConsumed)
    {
        return 0;
    }

    if (mDataSize > 15)
    {
        // Single packet has index of 0, multiple packets have index of 1, 2, 3, ...
        mHeader[PACKAGE_IDX_OFFSET]++; // Increment the package index
    }
    memcpy(packet, mHeader, sizeof(mHeader));

    size_t dataLength = mDataSize - mIndex;
    if (dataLength > 15)
    {
        dataLength = 15;
    }

    // Copy data
    memcpy(&packet[DATA_OFFSET], &mData[mIndex], dataLength);

    mIndex += dataLength;
    if (mIndex >= mDataSize)
    {
        // Add total CRC to the last packet
        packet[dataLength + 3] = mTotalCrc;
        mPacketsConsumed = true;
    }

    packet[dataLength + DATA_OFFSET] = CrcUtil::crc8(packet, dataLength + 4);
    return dataLength + 5; // Header + data + crc
}

bool WriteCommandPackets::allPacketsConsumed() const
{
    return mPacketsConsumed;
}
