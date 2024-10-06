#ifndef WRITE_COMMAND_PACKETS_H
#define WRITE_COMMAND_PACKETS_H

#include <cstddef>
#include <cstdint>

class WriteCommandPackets
{
public:
    static const size_t PACKET_SIZE = 20; // Header (4 bytes) + 15 bytes data + 1 byte CRC
    WriteCommandPackets(const uint8_t *data, size_t dataSize, int sequenceNumber);

    size_t getNextPacket(uint8_t *packet);
    bool allPacketsConsumed() const;

private:
    const uint8_t *mData;
    size_t mDataSize;

    uint8_t mTotalCrc = 0;
    uint8_t mHeader[4] = {}; // Length + Command + Sequence number + Package index
    uint8_t mIndex = 0;
    uint8_t mPacketsConsumed = false;
};
#endif // WRITE_COMMAND_PACKETS_H
