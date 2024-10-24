#include <gtest/gtest.h>
#include <hmi/HmiService.h>

#include "mocks/MockHmiDevice.h"

using namespace ::testing;

class HmiServiceTest : public ::testing::Test
{
protected:
    EventDispatcher mEventDispatcher;
    MockHmiDevice mMockHmiDevice;
    HmiService mHmiService;

    HmiServiceTest() : mHmiService(mEventDispatcher, mMockHmiDevice) { ; }

    virtual ~HmiServiceTest() {}
};

TEST_F(HmiServiceTest, init_Should_CallHmiDeviceInit)
{
    // Arrange
    EXPECT_CALL(mMockHmiDevice, init()).Times(1);
    
    // Act
    mHmiService.init();

    // Assert
}

TEST_F(HmiServiceTest, onUserBtPairingRequest_Should_Call_HmiDevice_OnUserBtPairingRequest)
{
    // Arrange
    struct bt_conn *conn = nullptr;
    uint32_t passkey = 123456;
    EXPECT_CALL(mMockHmiDevice, onUserBtPairingRequest(conn, passkey)).Times(1);
    
    // Act
    mEventDispatcher.dispatch<BtPassKeyConfirmRequest>({conn, passkey});
    k_sleep(K_MSEC(1)); // Wait for the work to be executed

    // Assert
}
