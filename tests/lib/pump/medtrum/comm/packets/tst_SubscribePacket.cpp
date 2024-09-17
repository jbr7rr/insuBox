#include "gtest/gtest.h"

#include <pump/medtrum/comm/packets/SubscribePacket.h>

TEST(SubscribePacketTest, GetRequest_Given_Packet_Expect_RequestPacket)
{
    // arange
    SubscribePacket subscribePacket;

    // act
    auto requestPacket = subscribePacket.getRequest();

    // assert
    std::vector<uint8_t> expectedPacket = {0x04, 0xFF, 0x0F};
    EXPECT_EQ(requestPacket, expectedPacket);
}
