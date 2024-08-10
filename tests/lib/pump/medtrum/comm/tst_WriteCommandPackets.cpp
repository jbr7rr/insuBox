#include "gtest/gtest.h"
#include <pump/medtrum/comm/WriteCommandPackets.h>
#include <vector>

TEST(WriteCommandPacketsTest, Given_14LongCommand_Expect_OnePacket)
{
    // Arrange
    std::vector<uint8_t> input = {5, 2, 0, 0, 0, 0, 235, 57, 134, 200};
    std::vector<uint8_t> expected = {14, 5, 0, 0, 2, 0, 0, 0, 0, 235, 57, 134, 200, 163, 0};
    int sequence = 0;
    WriteCommandPackets cmdPackets(input.data(), input.size(), sequence);
    std::vector<uint8_t> output(WriteCommandPackets::PACKET_SIZE);

    // Act
    size_t length = cmdPackets.getNextPacket(output.data());
    output.resize(length);

    // Assert
    EXPECT_EQ(output, expected);
}

TEST(WriteCommandPacketsTest, Given_41LongCommand_Expect_ThreePackets)
{
    // Arrange
    std::vector<uint8_t> input = {18,  0,  12, 0,   3,  0, 1,   30,  32, 3,   16, 14, 0,   0,  1, 7,   0,   160, 2,
                                  240, 96, 2,  104, 33, 2, 224, 225, 1,  192, 3,  2,  236, 36, 2, 100, 133, 2};
    std::vector<uint8_t> expected1 = {41, 18, 0, 1, 0, 12, 0, 3, 0, 1, 30, 32, 3, 16, 14, 0, 0, 1, 7, 135};
    std::vector<uint8_t> expected2 = {41, 18, 0, 2, 0, 160, 2, 240, 96, 2, 104, 33, 2, 224, 225, 1, 192, 3, 2, 253};
    std::vector<uint8_t> expected3 = {41, 18, 0, 3, 236, 36, 2, 100, 133, 2, 131, 167};
    int sequence = 0;
    size_t length = 0;
    WriteCommandPackets cmdPackets(input.data(), input.size(), sequence);
    std::vector<uint8_t> output(WriteCommandPackets::PACKET_SIZE);

    // Act
    length = cmdPackets.getNextPacket(output.data());
    output.resize(length);
    EXPECT_EQ(output, expected1);

    // Act
    length = cmdPackets.getNextPacket(output.data());
    output.resize(length);
    EXPECT_EQ(output, expected2);

    // Act
    length = cmdPackets.getNextPacket(output.data());
    output.resize(length);
    EXPECT_EQ(output, expected3);

    // Act
    size_t nextPacketSize = cmdPackets.getNextPacket(output.data());

    // Assert
    EXPECT_EQ(nextPacketSize, 0);
    EXPECT_TRUE(cmdPackets.allPacketsConsumed());
}
