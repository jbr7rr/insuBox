cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES $ENV{ZEPHYR_BASE}/../insuBox/modules/googletest)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(tst)

set(SOURCES
    src/main.cpp
    crypt/tst_Crypt.cpp
    comm/tst_WriteCommandPackets.cpp
    comm/packets/tst_AuthPacket.cpp
    comm/packets/tst_MedtrumBasePacket.cpp
    comm/packets/tst_NotificationPacket.cpp
    comm/packets/tst_SetBolusPacket.cpp
    comm/packets/tst_SetTempBasalPacket.cpp
    comm/packets/tst_SubscribePacket.cpp
    comm/packets/tst_SynchronizePacket.cpp
)

target_sources(app PRIVATE ${SOURCES})
target_link_libraries(app PUBLIC gtest_main)
