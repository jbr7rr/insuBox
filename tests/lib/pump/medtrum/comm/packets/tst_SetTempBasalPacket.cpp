#include "gtest/gtest.h"
#include <pump/medtrum_bt/MedtrumPumpSync.h>
#include <pump/medtrum_bt/comm/WriteCommandPackets.h>
#include <pump/medtrum_bt/comm/packets/SetTempBasalPacket.h>

class SetTempBasalPacketTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        mPumpSync.init();
    }
    virtual void TearDown() override {}

    MedtrumPumpSync mPumpSync;
};

TEST_F(SetTempBasalPacketTest, getRequest_Given_PacketAndValues_Expect_RequestPacket)
{
    // arange
    float absoluteRate = 1.25;
    uint16_t durationInMinutes = 60;

    // act
    SetTempBasalPacket setTempBasalPacket(mPumpSync, absoluteRate, durationInMinutes);
    auto requestPacket = setTempBasalPacket.getRequest();

    // assert
    std::vector<uint8_t> expectedPacket = {24, 6, 25, 0, 60, 0};
    EXPECT_EQ(requestPacket, expectedPacket);
}

TEST_F(SetTempBasalPacketTest, onIndication_Given_Response_When_MessageIsTooShort_Then_DataIsNotStored)
{
    // arange
    float absoluteRate = 1.25;
    uint16_t durationInMinutes = 60;

    std::vector<uint8_t> response = {24, 0, 0, 6, 25, 0, 2, 0, 146, 0, 200, 237, 88};

    // act
    SetTempBasalPacket setTempBasalPacket(mPumpSync, absoluteRate, durationInMinutes);

    uint8_t buffer[WriteCommandPackets::PACKET_SIZE];
    WriteCommandPackets writeCommandPackets(response.data(), response.size(), 0);
    while (!writeCommandPackets.allPacketsConsumed())
    {
        size_t length = writeCommandPackets.getNextPacket(buffer);
        setTempBasalPacket.onIndication(buffer, length);
    }

    // assert
    ASSERT_TRUE(setTempBasalPacket.isReady());
    ASSERT_TRUE(setTempBasalPacket.isFailed());
    ASSERT_NE(mPumpSync.getBasalType(), BasalType::ABSOLUTE_TEMP);
    ASSERT_NE(mPumpSync.getBasalRate(), absoluteRate);
}

TEST_F(SetTempBasalPacketTest, onIndication_Given_Response_When_MessageIsCorrect_Then_DataIsStored)
{
    // arange
    float absoluteRate = 1.25;
    uint16_t durationInMinutes = 60;

    std::vector<uint8_t> response = {24, 0, 0, 6, 25, 0, 2, 0, 146, 0, 200, 237, 88, 17};

    // act
    SetTempBasalPacket setTempBasalPacket(mPumpSync, absoluteRate, durationInMinutes);

    uint8_t buffer[WriteCommandPackets::PACKET_SIZE];
    WriteCommandPackets writeCommandPackets(response.data(), response.size(), 0);
    while (!writeCommandPackets.allPacketsConsumed())
    {
        size_t length = writeCommandPackets.getNextPacket(buffer);
        setTempBasalPacket.onIndication(buffer, length);
    }

    // assert
    ASSERT_TRUE(setTempBasalPacket.isReady());
    ASSERT_FALSE(setTempBasalPacket.isFailed());
    ASSERT_EQ(mPumpSync.getBasalType(), BasalType::ABSOLUTE_TEMP);
    ASSERT_EQ(mPumpSync.getBasalRate(), absoluteRate);
    ASSERT_EQ(mPumpSync.getBasalSequence(), 2);
    ASSERT_EQ(mPumpSync.getBasalStartTime(), 1679575112);
}
