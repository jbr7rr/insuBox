#include "gtest/gtest.h"
#include <pump/medtrum/comm/packets/MedtrumBasePacket.h>

class TestMedtrumBasePacket : public MedtrumBasePacket
{
public:
    TestMedtrumBasePacket(uint8_t opcode = 0x00) { mOpCode = opcode; }
    std::vector<uint8_t> getResponse() const { return mResponse; }
};

TEST(MedtrumBasePacketTest, Given_CorrectBytes_Expect_PacketNotFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {51, 99, 10, 1, 0, 0, 170, 44, 1, 255, 171, 21, 148, 194, 1, 0, 22, 0, 1, 75};
    std::vector<uint8_t> chunk2 = {51, 99, 10, 2, 0, 0, 0, 176, 140, 84, 18, 10, 0, 10, 0, 0, 0, 0, 0, 246};
    std::vector<uint8_t> chunk3 = {51, 99, 10, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 195, 197, 136, 5};
    std::vector<uint8_t> chunk4 = {51, 99, 10, 4, 19, 174, 176};

    // act
    TestMedtrumBasePacket packet(99);
    packet.onIndication(chunk1.data(), chunk1.size());
    packet.onIndication(chunk2.data(), chunk2.size());
    packet.onIndication(chunk3.data(), chunk3.size());
    bool ready = packet.onIndication(chunk4.data(), chunk4.size());

    // assert
    std::vector<uint8_t> expectedData = {51, 99, 10, 1, 0, 0,   170, 44, 1,  255, 171, 21, 148, 194, 1,   0,  22,
                                         0,  1,  0,  0, 0, 176, 140, 84, 18, 10,  0,   10, 0,   0,   0,   0,  0,
                                         0,  0,  0,  0, 0, 0,   0,   0,  0,  0,   0,   0,  195, 197, 136, 19, 174};
    EXPECT_EQ(packet.getResponse(), expectedData);
    EXPECT_FALSE(packet.isFailed());
    EXPECT_TRUE(ready);
}

TEST(MedtrumBasePacketTest, Given_IncorrectCRCInFirstChunk_Expect_PacketFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {51, 99, 10, 1, 0, 0, 170, 44, 1, 255, 171, 21, 255, 194, 255, 255, 22, 0, 1, 75};
    std::vector<uint8_t> chunk2 = {51, 99, 10, 2, 0, 0, 0, 176, 140, 84, 18, 10, 0, 10, 0, 0, 0, 0, 0, 246};
    std::vector<uint8_t> chunk3 = {51, 99, 10, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 195, 197, 136, 5};
    std::vector<uint8_t> chunk4 = {51, 99, 10, 4, 19, 174, 176};

    // act
    TestMedtrumBasePacket packet(99);
    packet.onIndication(chunk1.data(), chunk1.size());
    packet.onIndication(chunk2.data(), chunk2.size());
    packet.onIndication(chunk3.data(), chunk3.size());
    bool ready = packet.onIndication(chunk4.data(), chunk4.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, Given_IncorrectCRCInSecondChunk_Expect_PacketFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {51, 99, 10, 1, 0, 0, 170, 44, 1, 255, 171, 21, 148, 194, 1, 0, 22, 0, 1, 75};
    std::vector<uint8_t> chunk2 = {51, 99, 10, 2, 0, 0, 0, 176, 255, 84, 18, 10, 0, 10, 255, 255, 0, 0, 0, 246};
    std::vector<uint8_t> chunk3 = {51, 99, 10, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 195, 197, 136, 5};
    std::vector<uint8_t> chunk4 = {51, 99, 10, 4, 19, 174, 176};

    // act
    TestMedtrumBasePacket packet(99);
    packet.onIndication(chunk1.data(), chunk1.size());
    packet.onIndication(chunk2.data(), chunk2.size());
    packet.onIndication(chunk3.data(), chunk3.size());
    bool ready = packet.onIndication(chunk4.data(), chunk4.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, Given_IncorrectCRCInThirdChunk_Expect_PacketFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {51, 99, 10, 1, 0, 0, 170, 44, 1, 255, 171, 21, 148, 194, 1, 0, 22, 0, 1, 75};
    std::vector<uint8_t> chunk2 = {51, 99, 10, 2, 0, 0, 0, 176, 140, 84, 18, 10, 0, 10, 0, 0, 0, 0, 0, 246};
    std::vector<uint8_t> chunk3 = {51, 99, 10, 3, 0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 195, 197, 136, 5};
    std::vector<uint8_t> chunk4 = {51, 99, 10, 4, 19, 174, 176};

    // act
    TestMedtrumBasePacket packet(99);
    packet.onIndication(chunk1.data(), chunk1.size());
    packet.onIndication(chunk2.data(), chunk2.size());
    packet.onIndication(chunk3.data(), chunk3.size());
    bool ready = packet.onIndication(chunk4.data(), chunk4.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, Given_IncorrectCRCInLastChunk_Expect_PacketFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {51, 99, 10, 1, 0, 0, 170, 44, 1, 255, 171, 21, 148, 194, 1, 0, 22, 0, 1, 75};
    std::vector<uint8_t> chunk2 = {51, 99, 10, 2, 0, 0, 0, 176, 140, 84, 18, 10, 0, 10, 0, 0, 0, 0, 0, 246};
    std::vector<uint8_t> chunk3 = {51, 99, 10, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 195, 197, 136, 5};
    std::vector<uint8_t> chunk4 = {51, 99, 10, 255, 255, 174, 176};

    // act
    TestMedtrumBasePacket packet(99);
    packet.onIndication(chunk1.data(), chunk1.size());
    packet.onIndication(chunk2.data(), chunk2.size());
    packet.onIndication(chunk3.data(), chunk3.size());
    bool ready = packet.onIndication(chunk4.data(), chunk4.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, Given_IncorrectSequence_Expect_PacketFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {51, 99, 10, 1, 0, 0, 170, 44, 1, 255, 171, 21, 148, 194, 1, 0, 22, 0, 1, 75};
    std::vector<uint8_t> chunk2 = {51, 99, 10, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 195, 197, 136, 5};
    std::vector<uint8_t> chunk3 = {51, 99, 10, 2, 0, 0, 0, 176, 140, 84, 18, 10, 0, 10, 0, 0, 0, 0, 0, 246};
    std::vector<uint8_t> chunk4 = {51, 99, 10, 4, 19, 174, 176};

    // act
    TestMedtrumBasePacket packet(99);
    packet.onIndication(chunk1.data(), chunk1.size());
    packet.onIndication(chunk2.data(), chunk2.size());
    packet.onIndication(chunk3.data(), chunk3.size());
    bool ready = packet.onIndication(chunk4.data(), chunk4.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, Given_IncorrectOpCode_Expect_PacketFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {51, 99, 10, 1, 0, 0, 170, 44, 1, 255, 171, 21, 148, 194, 1, 0, 22, 0, 1, 75};
    std::vector<uint8_t> chunk2 = {51, 99, 10, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 195, 197, 136, 5};
    std::vector<uint8_t> chunk3 = {51, 99, 10, 2, 0, 0, 0, 176, 140, 84, 18, 10, 0, 10, 0, 0, 0, 0, 0, 246};
    std::vector<uint8_t> chunk4 = {51, 99, 10, 4, 19, 174, 176};

    // act
    TestMedtrumBasePacket packet(90);
    packet.onIndication(chunk1.data(), chunk1.size());
    packet.onIndication(chunk2.data(), chunk2.size());
    packet.onIndication(chunk3.data(), chunk3.size());
    bool ready = packet.onIndication(chunk4.data(), chunk4.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}
TEST(MedtrumBasePacketTest, Given_CorrectBytesOneChunk_Expect_PacketNotFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {14, 5, 0, 0, 0, 0, 2, 80, 1, 74, 64, 4, 0, 240};

    // act
    TestMedtrumBasePacket packet(5);
    bool ready = packet.onIndication(chunk1.data(), chunk1.size());

    // assert
    std::vector<uint8_t> expectedData = {14, 5, 0, 0, 0, 0, 2, 80, 1, 74, 64, 4, 0};
    EXPECT_EQ(packet.getResponse(), expectedData);
    EXPECT_FALSE(packet.isFailed());
    EXPECT_TRUE(ready);
}

TEST(MedtrumBasePacketTest, Given_IncorrectCRCOneChunk_Expect_PacketFailed)
{
    // arrange
    std::vector<uint8_t> chunk1 = {14, 5, 0, 0, 0, 0, 2, 80, 1, 74, 64, 4, 255, 255};

    // act
    TestMedtrumBasePacket packet(5);
    bool ready = packet.onIndication(chunk1.data(), chunk1.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, GetRequest_Given_OpCodeCorrect_Expect_RequestPacket)
{
    // arrange
    TestMedtrumBasePacket packet(0x01);

    // act
    std::vector<uint8_t> request = packet.getRequest();

    // assert
    std::vector<uint8_t> expectedRequest = {0x01};
    EXPECT_EQ(request, expectedRequest);
}

TEST(MedtrumBasePacketTest, GetRequest_Given_MessageTooShort_Expect_Failed)
{
    // arrange
    TestMedtrumBasePacket packet(0x01);
    std::vector<uint8_t> message = {0x01, 0x02};

    // act
    bool ready = packet.onIndication(message.data(), message.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, GetRequest_Given_MessageTooShortAfterHeader_Expect_Failed)
{
    // arrange
    TestMedtrumBasePacket packet(0x01);
    std::vector<uint8_t> message = {0x01, 0x02, 0x03, 0x04, 0x05};

    // act
    bool ready = packet.onIndication(message.data(), message.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, GetRequest_Given_ResponseCodeError_Expect_Failed)
{
    // arrange
    TestMedtrumBasePacket packet(21);
    std::vector<uint8_t> message = {7, 21, 5, 0, 8, 0, 146, 0};

    // act
    bool ready = packet.onIndication(message.data(), message.size());

    // assert
    EXPECT_TRUE(ready);
    EXPECT_TRUE(packet.isFailed());
}

TEST(MedtrumBasePacketTest, GetRequest_Given_ResponseCodeWaiting_Expect_NotReady)
{
    // arrange
    TestMedtrumBasePacket packet(21);
    std::vector<uint8_t> message = {0x07, 0x15, 0x00, 0x00, 0x00, 0x40, 0xf4};

    // act
    bool ready = packet.onIndication(message.data(), message.size());

    // assert
    EXPECT_FALSE(ready);
    EXPECT_FALSE(packet.isFailed());
}
