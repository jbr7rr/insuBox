#include "gtest/gtest.h"
#include <pump/medtrum_bt/crypt/Crypt.h>

TEST(CryptTest, Given_SN_Expect_Key)
{
    uint32_t input = 2859923929;
    uint32_t expected = 3364239851;
    uint32_t output = Crypt::keyGen(input);
    EXPECT_EQ(output, expected);
}

TEST(CryptTest, Given_SN_Expect_Real)
{
    uint32_t input = 2859923929;
    uint32_t expected = 126009121;
    uint32_t output = Crypt::simpleDecrypt(input);
    EXPECT_EQ(output, expected);
}
