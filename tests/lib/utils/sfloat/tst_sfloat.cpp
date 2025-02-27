#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <utils/sfloat.h>

struct SFloatTestParams {
    float floatVal;
    uint16_t sFloatVal;
};

class SFloatFromFloatTest : public ::testing::TestWithParam<SFloatTestParams> {};

TEST_P(SFloatFromFloatTest, Should_ConvertFloatToSFloatCorrectly)
{
    SFloatTestParams params = GetParam();
    uint16_t result = SFloat(params.floatVal).value();
    EXPECT_EQ(result, params.sFloatVal)
        << "Failed for input float: " << params.floatVal;
}

INSTANTIATE_TEST_SUITE_P(
    SFloatTests,
    SFloatFromFloatTest,
    ::testing::Values(
        SFloatTestParams{0.0f, 0x0000},
        SFloatTestParams{-0.0f, 0x0000},
        SFloatTestParams{0.000678f, 0xA2A6},
        SFloatTestParams{-0.000678f, 0xAD5A},
        SFloatTestParams{0.00123f, 0xA4CE},
        SFloatTestParams{-0.00123f, 0xAB32},
        SFloatTestParams{0.0475f, 0xC1DB},
        SFloatTestParams{-0.0475f, 0xCE25},
        SFloatTestParams{0.827f, 0xD33B},
        SFloatTestParams{-0.827f, 0xDCC5},
        SFloatTestParams{2.75f, 0xE113},
        SFloatTestParams{-2.75f, 0xEEED},
        SFloatTestParams{10.3f, 0xE406},
        SFloatTestParams{-10.3f, 0xEBFA},
        SFloatTestParams{391.0f, 0x0187},
        SFloatTestParams{-391.0f, 0x0E79},
        SFloatTestParams{5070.0f, 0x11FB},
        SFloatTestParams{-5070.0f, 0x1E05},
        SFloatTestParams{96100.0f, 0x23C1},
        SFloatTestParams{-96100.0f, 0x2C3F},
        SFloatTestParams{278000.0f, 0x3116},
        SFloatTestParams{-278000.0f, 0x3EEA},
        SFloatTestParams{9510000.0f, 0x43B7},
        SFloatTestParams{-9510000.0f, 0x4C49},
        
        // Rounding test cases
        SFloatTestParams{105.899f, 0xF423},
        SFloatTestParams{-105.899f, 0xFBDD},
        SFloatTestParams{197.85f, 0xF7BB},
        SFloatTestParams{-197.85f, 0xF845},
        SFloatTestParams{153.249f, 0xF5FC},
        SFloatTestParams{-153.249f, 0xFA04},
        SFloatTestParams{171.601f, 0xF6B4},
        SFloatTestParams{-171.601f, 0xF94C},

        // Very low and high values near limits
        SFloatTestParams{0.000001f, 0x8064},
        SFloatTestParams{-0.000001f, 0x8F9C},
        SFloatTestParams{0.00000101f, 0x8065},
        SFloatTestParams{-0.00000101f, 0x8F9B},
        SFloatTestParams{20470000000.0f, 0x77FF}, // Max positive representable
        SFloatTestParams{-20480000000.0f, 0x7800}, // Max negative representable

        // Edge cases for mantissa
        SFloatTestParams{20470.0f, 0x17FF},
        SFloatTestParams{20480.0f, 0x20CD},
        SFloatTestParams{-20470.0f, 0x1801},
        SFloatTestParams{-20480.0f, 0x1800},

        // Avoiding special values boundaries
        SFloatTestParams{2045.0f, 0x07FD},
        SFloatTestParams{2046.0f, 0x10CD},
        SFloatTestParams{2047.0f, 0x10CD},
        SFloatTestParams{2048.0f, 0x10CD},
        SFloatTestParams{-2045.0f, 0x0803},
        SFloatTestParams{-2046.0f, 0x1F33},
        SFloatTestParams{-2047.0f, 0x1F33},
        SFloatTestParams{-2048.0f, 0x1F33},

        // Special values
        SFloatTestParams{INFINITY, 0x07FE},
        SFloatTestParams{-INFINITY, 0x0802},
        SFloatTestParams{NAN, 0x07FF}
    )
);

class SFloatToFloatTest : public ::testing::TestWithParam<SFloatTestParams> {};

TEST_P(SFloatToFloatTest, Should_ConvertSFloatToFloatCorrectly)
{
    SFloatTestParams params = GetParam();
    float result = SFloat(params.sFloatVal).toFloat();

    if (std::isnan(params.floatVal))
    {
        EXPECT_TRUE(std::isnan(result)) << "Expected NaN, got: " << result;
    }
    else if (std::isinf(params.floatVal))
    {
        EXPECT_TRUE(std::isinf(result) && (std::signbit(result) == std::signbit(params.floatVal)))
            << "Expected Inf with correct sign, got: " << result;
    }
    else
    {
        EXPECT_NEAR(result, params.floatVal, 0.0001)
            << "Failed for SFloat value: " << std::hex << params.sFloatVal;
    }
}

INSTANTIATE_TEST_SUITE_P(
    SFloatTests,
    SFloatToFloatTest,
    ::testing::Values(
        SFloatTestParams{0.0f, 0x0000},
        SFloatTestParams{0.000678f, 0xA2A6},
        SFloatTestParams{-0.000678f, 0xAD5A},
        SFloatTestParams{0.00123f, 0xA4CE},
        SFloatTestParams{-0.00123f, 0xAB32},
        SFloatTestParams{INFINITY, 0x07FE},
        SFloatTestParams{-INFINITY, 0x0802},
        SFloatTestParams{NAN, 0x07FF}
    )
);
