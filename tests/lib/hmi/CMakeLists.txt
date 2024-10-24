cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_COMPILER /usr/bin/g++ CACHE PATH "" FORCE)
set(BOARD native_posix)
set(ZEPHYR_TOOLCHAIN_VARIANT zephyr)
set(TOOLCHAIN_ROOT ${ZEPHYR_BASE})
set(ZEPHYR_EXTRA_MODULES $ENV{ZEPHYR_BASE}/../insuBox/modules/googletest)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(tst)

# Add test sources
set(SOURCES
    src/main.cpp
    tst_HmiService.cpp
)

# Add these sources to your target
target_sources(app PRIVATE ${SOURCES})

# Link libraries
target_link_libraries(app PUBLIC gtest_main gmock_main)
