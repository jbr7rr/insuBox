#include "gtest/gtest.h"

extern "C" int main(void) {
    ::testing::InitGoogleTest();
    int ret = RUN_ALL_TESTS();
    return ret;
}
