#include "gtest/gtest.h"
#include <pump/medtrum/MedtrumPumpSync.h>
#include <pump/medtrum/comm/WriteCommandPackets.h>
#include <pump/medtrum/comm/packets/SynchronizePacket.h>

class SynchronizePacketTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        mPumpSync.init();
    }
    virtual void TearDown() override {}

    MedtrumPumpSync mPumpSync;
};

TEST_F(SynchronizePacketTest, GetRequest_Given_Packet_Expect_RequestPacket)
{
    // arange
    SynchronizePacket synchronizePacket(mPumpSync);

    // act
    auto requestPacket = synchronizePacket.getRequest();

    // assert
    std::vector<uint8_t> expectedPacket = {0x03};
    EXPECT_EQ(requestPacket, expectedPacket);
}

// TODO: Add more tests when Notification packet is fully implemented
