cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES $ENV{ZEPHYR_BASE}/../insuBox/modules/googletest)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(tst)

set(SOURCES
    src/main.cpp
    tst_HmiService.cpp
)

target_sources(app PRIVATE ${SOURCES})
target_link_libraries(app PUBLIC gtest_main gmock_main)
