#include "gtest/gtest.h"
#include <pump/medtrum/MedtrumPumpSync.h>
#include <pump/medtrum/comm/WriteCommandPackets.h>
#include <pump/medtrum/comm/packets/AuthPacket.h>

class AuthPacketTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        mPumpSync.init();
    }
    virtual void TearDown() override {}

    MedtrumPumpSync mPumpSync;
};

TEST_F(AuthPacketTest, getRequest_Given_PacketAndSN_Expect_RequestPacket)
{
    // arange
    uint32_t pumpSerial = 0xAA76F9D9;
    uint32_t sessionToken = 667;

    // act
    mPumpSync.setSessionToken(sessionToken);
    AuthPacket authPacket(mPumpSync, pumpSerial);
    auto requestPacket = authPacket.getRequest();

    // assert
    uint32_t expectedkey = 3364239851;
    uint8_t type = 2;

    // assert
    std::vector<uint8_t> expectedPacket = {5,
                                           type,
                                           static_cast<uint8_t>(sessionToken & 0xFF),
                                           static_cast<uint8_t>((sessionToken >> 8) & 0xFF),
                                           static_cast<uint8_t>((sessionToken >> 16) & 0xFF),
                                           static_cast<uint8_t>((sessionToken >> 24) & 0xFF),
                                           static_cast<uint8_t>(expectedkey & 0xFF),
                                           static_cast<uint8_t>((expectedkey >> 8) & 0xFF),
                                           static_cast<uint8_t>((expectedkey >> 16) & 0xFF),
                                           static_cast<uint8_t>((expectedkey >> 24) & 0xFF)};
    EXPECT_EQ(requestPacket, expectedPacket);
}

TEST_F(AuthPacketTest, onIndication_Given_Response_When_MessageIsTooShort_Then_DataIsNotStored)
{
    // arrange
    uint8_t opcode = 5;
    uint16_t responseCode = 0;
    uint8_t deviceType = 80;
    uint8_t swVerX = 12;
    uint8_t swVerY = 1;
    uint8_t swVerZ = 3;

    std::vector<uint8_t> response = {opcode,
                                     static_cast<uint8_t>(responseCode & 0xFF),
                                     static_cast<uint8_t>((responseCode >> 8) & 0xFF),
                                     0,
                                     deviceType,
                                     swVerX,
                                     swVerY};

    // act
    AuthPacket authPacket(mPumpSync, 0);

    uint8_t buffer[WriteCommandPackets::PACKET_SIZE];
    WriteCommandPackets command(response.data(), response.size(), 0);
    size_t length = command.getNextPacket(buffer);
    authPacket.onIndication(buffer, length);

    // assert
    ASSERT_TRUE(authPacket.isFailed());
    ASSERT_TRUE(authPacket.isReady());
    ASSERT_NE(mPumpSync.getDeviceType(), deviceType);
}

TEST_F(AuthPacketTest, onIndication_Given_Response_When_MessageIsCorrect_Then_DataIsStored)
{
    // arrange
    uint8_t opcode = 5;
    uint16_t responseCode = 0;
    uint8_t deviceType = 80;
    uint8_t swVerX = 12;
    uint8_t swVerY = 1;
    uint8_t swVerZ = 3;

    std::vector<uint8_t> response = {opcode,
                                     static_cast<uint8_t>(responseCode & 0xFF),
                                     static_cast<uint8_t>((responseCode >> 8) & 0xFF),
                                     0,
                                     deviceType,
                                     swVerX,
                                     swVerY,
                                     swVerZ};

    // act
    AuthPacket authPacket(mPumpSync, 0);

    uint8_t buffer[WriteCommandPackets::PACKET_SIZE];
    WriteCommandPackets command(response.data(), response.size(), 0);
    size_t length = command.getNextPacket(buffer);
    authPacket.onIndication(buffer, length);


    // assert
    std::string swString = std::to_string(swVerX) + "." + std::to_string(swVerY) + "." + std::to_string(swVerZ);
    ASSERT_TRUE(authPacket.isReady());
    ASSERT_FALSE(authPacket.isFailed());
    ASSERT_EQ(mPumpSync.getDeviceType(), deviceType);
    ASSERT_EQ(mPumpSync.getSwVersion(), swString);
}
