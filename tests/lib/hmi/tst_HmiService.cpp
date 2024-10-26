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

    HmiServiceTest() : mHmiService(mEventDispatcher, mMockHmiDevice)
    {
        // Wait for some background stuff to initialize
        k_sleep(K_NSEC(1));
    }

    virtual ~HmiServiceTest() {
    }
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
    EXPECT_CALL(mMockHmiDevice, init()).Times(1);

    // Act
    mHmiService.init();
    mEventDispatcher.dispatch<BtPassKeyConfirmRequest>({conn, passkey});
    k_sleep(K_NSEC(10)); // Wait for the work to be executed

    // Assert
}

TEST_F(HmiServiceTest, onUserBtPairingResponse_Should_Dispatch_BtPassKeyConfirmResponse)
{
    // Arrange
    struct bt_conn *conn = nullptr;
    bool accepted = true;
    int count = 0;
    EXPECT_CALL(mMockHmiDevice, init()).Times(1);
    mEventDispatcher.subscribe<BtPassKeyConfirmResponse>([&](const BtPassKeyConfirmResponse &response) {
        count++;
        EXPECT_EQ(response.conn, conn);
        EXPECT_EQ(response.accept, accepted);
    });
    
    // Act
    mHmiService.init();
    mHmiService.onUserBtPairingResponse(conn, accepted);

    // Assert
    EXPECT_EQ(count, 1);
}