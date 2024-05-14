#include "gtest/gtest.h"
#include <pump/medtrum/crypt/Crypt.h>

TEST(CryptTest, GivenSNExpectKey) {
    uint32_t input = 2859923929;
    uint32_t expected = 3364239851;
    uint32_t output = Crypt::keyGen(input);
    EXPECT_EQ(output, expected);
}

TEST(CryptTest, GivenSNExpectReal) {
    uint32_t input = 2859923929;
    uint32_t expected = 126009121;
    uint32_t output = Crypt::simpleDecrypt(input);
    EXPECT_EQ(output, expected);
}
