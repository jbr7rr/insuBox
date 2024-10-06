#include "gtest/gtest.h"

#include <pump/medtrum_bt/comm/packets/NotificationPacket.h>
#include <pump/medtrum_bt/MedtrumPumpSync.h>

class NotificationPacketTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        mPumpSync = new MedtrumPumpSync();
        mPumpSync->init();
        
        // Reset all pumpsync values before each test
        mPumpSync->setPumpState(PumpState::NONE);
        mPumpSync->setPatchId(0);
        mPumpSync->setCurrentSequenceNumber(0);
        mPumpSync->setReservoirLevel(0);
    }
    virtual void TearDown() override {
        delete mPumpSync;
    }

    MedtrumPumpSync *mPumpSync;
};

TEST_F(NotificationPacketTest, onNotification_Given_FieldMaskButMessageTooShort_Then_NothingHandled)
{
    // arange
    uint8_t data[] = {67, 41, 67, 255, 122, 95, 18, 0, 73, 1, 19, 0, 1, 0, 20, 0, 0, 0, 0, 16};
    size_t dataSize = sizeof(data) - 1;

    NotificationPacket notificationPacket(*mPumpSync);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getBolusInProgress(), false);
    EXPECT_NEAR(mPumpSync->getBolusProgressLastTime(), 0, 1);
    EXPECT_NEAR(mPumpSync->getBolusAmountDelivered(), 0, 0.001);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 0);
}

TEST_F(NotificationPacketTest, onNotification_Given_ErrorenousData_Expect_NothingHandled)
{
    // arange
    uint8_t data[] = {32, 34, 17, 128, 128, 128, 167, 12, 176, 0, 14, 0, 0, 0, 0, 0, 0};
    size_t dataSize = sizeof(data);

    NotificationPacket notificationPacket(*mPumpSync);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getBolusInProgress(), false);
    EXPECT_NEAR(mPumpSync->getBolusProgressLastTime(), 0, 1);
    EXPECT_NEAR(mPumpSync->getBolusAmountDelivered(), 0, 0.001);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 0);
}

TEST_F(NotificationPacketTest, onNotification_Given_BolusOutOfRange_Expect_NothingHandled)
{
    // arange
    uint8_t data[] = {32, 34, 17, 128, 128, 128, 167, 12, 176, 0, 14, 0, 0, 0, 0, 0, 0};
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);
    // Set valid patchID (as in a started pump session)
    mPumpSync->setPatchId(14);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getBolusInProgress(), false);
    EXPECT_NEAR(mPumpSync->getBolusProgressLastTime(), 0, 1);
    EXPECT_NEAR(mPumpSync->getBolusAmountDelivered(), 0, 0.001);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 0);
}

TEST_F(NotificationPacketTest, onNotification_Given_WrongPatchIdInBasal_Expect_NothingHandled)
{
    // arange
    uint8_t data[] = {32, 40, 64, 6, 25, 0, 14, 0, 84, 163, 173, 17, 17, 64, 0, 152, 15, 0, 16}; 
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);
    // Set valid patchID (as in a started pump session)
    mPumpSync->setPatchId(15);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getBasalType(), BasalType::NONE);
    EXPECT_NEAR(mPumpSync->getBasalRate(), 0, 0.001);
    EXPECT_EQ(mPumpSync->getBasalSequence(), 0);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 0);
    EXPECT_EQ(mPumpSync->getBasalStartTime(), 1);
    EXPECT_NEAR(mPumpSync->getReservoirLevel(), 0, 0.001);
}

TEST_F(NotificationPacketTest, onNotification_Given_BasalOutOfRange_Expect_NothingHandled)
{
    // arange
    uint8_t data[] = {32, 40, 64, 6, 25, 0, 14, 0, 84, 163, 173, 17, 127, 127, 128, 152, 14, 0, 16}; 
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);
    // Set valid patchID (as in a started pump session)
    mPumpSync->setPatchId(14);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getBasalType(), BasalType::NONE);
    EXPECT_NEAR(mPumpSync->getBasalRate(), 0, 0.001);
    EXPECT_EQ(mPumpSync->getBasalSequence(), 0);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 0);
    EXPECT_EQ(mPumpSync->getBasalStartTime(), 1);
    EXPECT_NEAR(mPumpSync->getReservoirLevel(), 0, 0.001);
}

TEST_F(NotificationPacketTest, onNotification_Given_WrongPatchId_Expect_NothingHandled)
{
    // arange
    uint8_t data[] = {32, 34, 17, 128, 33, 0, 167, 12, 176, 0, 15, 0, 0, 0, 0, 0, 0};
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);
    // Set valid patchID (as in a started pump session)
    mPumpSync->setPatchId(14);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getBolusInProgress(), false);
    EXPECT_NEAR(mPumpSync->getBolusProgressLastTime(), 0, 1);
    EXPECT_NEAR(mPumpSync->getBolusAmountDelivered(), 0, 0.001);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 0);
}

TEST_F(NotificationPacketTest, onNotification_Given_StatusAndData_Expect_PumpStateSet)
{
    // arange
    uint8_t data[] = {1}; // IDLE
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getPumpState(), PumpState::IDLE);
}

TEST_F(NotificationPacketTest, onNotification_Given_BasalData_Expect_BasalDataHandled)
{
    // arange
    uint8_t data[] = {32, 40, 64, 6, 25, 0, 14, 0, 84, 163, 173, 17, 17, 64, 0, 152, 14, 0, 16}; 
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);
    mPumpSync->setCurrentSequenceNumber(0);
    // Set valid patch id
    mPumpSync->setPatchId(14);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getPumpState(), PumpState::ACTIVE);
    EXPECT_EQ(mPumpSync->getBasalType(), BasalType::ABSOLUTE_TEMP);
    EXPECT_NEAR(mPumpSync->getBasalRate(), 0.85, 0.001);
    EXPECT_EQ(mPumpSync->getBasalSequence(), 25);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 25);
    EXPECT_EQ(mPumpSync->getBasalStartTime(), 1685126612);
    EXPECT_NEAR(mPumpSync->getReservoirLevel(), 186.80, 0.001);
}

TEST_F(NotificationPacketTest, onNotification_Given_SequenceAndOtherData_Expect_DataHandled)
{
    // arange
    uint8_t data[] = {32, 0, 17, 167, 0, 14, 0, 0, 0, 0, 0, 0}; 
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);
    mPumpSync->setCurrentSequenceNumber(0);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getPumpState(), PumpState::ACTIVE);
    EXPECT_EQ(mPumpSync->getCurrentSequenceNumber(), 167);
}

TEST_F(NotificationPacketTest, onNotification_Given_BolusInProgress_Expect_BolusDataHandled)
{
    // arange
    uint8_t data[] = {32, 34, 16, 0, 3, 0, 198, 12, 0, 0, 0, 0, 0};
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);

    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getPumpState(), PumpState::ACTIVE);
    EXPECT_NEAR(mPumpSync->getBolusProgressLastTime(), currentTime.tv_sec, 1);
    EXPECT_EQ(mPumpSync->getBolusInProgress(), true);
    EXPECT_NEAR(mPumpSync->getBolusAmountDelivered(), 0.15, 0.001);
    EXPECT_NEAR(mPumpSync->getReservoirLevel(), 163.5, 0.001);
}

TEST_F(NotificationPacketTest, onNotification_Given_BolusDone_Expect_BolusDataHandled)
{
    // arange
    uint8_t data[] = {32, 34, 17, 128, 33, 0, 167, 12, 176, 0, 14, 0, 0, 0, 0, 0, 0};
    size_t dataSize = sizeof(data);
    NotificationPacket notificationPacket(*mPumpSync);

    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);

    // act
    notificationPacket.onNotification(data, dataSize);

    // assert
    EXPECT_EQ(mPumpSync->getPumpState(), PumpState::ACTIVE);
    EXPECT_NEAR(mPumpSync->getBolusProgressLastTime(), currentTime.tv_sec, 1);
    EXPECT_EQ(mPumpSync->getBolusInProgress(), false);
    EXPECT_NEAR(mPumpSync->getBolusAmountDelivered(), 1.65, 0.001);
    EXPECT_NEAR(mPumpSync->getReservoirLevel(), 161.95, 0.001);
}
