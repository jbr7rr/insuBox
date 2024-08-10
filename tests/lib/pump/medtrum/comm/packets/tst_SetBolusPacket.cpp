#include "gtest/gtest.h"

#include <pump/medtrum/comm/packets/SetBolusPacket.h>

TEST(SetBolusPacketTest, GetRequest_Given_Packet_Expect_RequestPacket)
{
    // arange
    SetBolusPacket setBolusPacket(2.35);

    // act
    auto requestPacket = setBolusPacket.getRequest();

    // assert
    std::vector<uint8_t> expectedPacket = {0x13, 0x01, 0x2F, 0x00, 0x00};
    EXPECT_EQ(requestPacket, expectedPacket);
}
