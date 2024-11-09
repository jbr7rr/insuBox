#include <cmath>
#include <utils/sfloat.h>
#include <zephyr/sys/byteorder.h>

namespace
{
    static constexpr int16_t SFLOAT_MANTISSA_MAX = (1 << 11) - 1;
    static constexpr int16_t SFLOAT_MANTISSA_MIN = -(1 << 11);
    static constexpr int16_t SFLOAT_MANTISSA_MASK = 0x0FFF;

    static constexpr int8_t SFLOAT_EXP_MIN = -8;
    static constexpr int8_t SFLOAT_EXP_MAX = 7;

    static constexpr int16_t SFLOAT_EXP_MASK = 0xF000;
    static constexpr int8_t SFLOAT_EXP_BIT_POS = 12;

    static constexpr float FLOAT_ABS_MIN = 0.000001f;
    static constexpr float FLOAT_NEG_MIN = 10000000.0f * SFLOAT_MANTISSA_MIN;
    static constexpr float FLOAT_POS_MAX = 10000000.0f * SFLOAT_MANTISSA_MAX;

    static constexpr uint16_t SFLOAT_SPECIAL_NEG_INFINTY = 0x0802;
    static constexpr uint16_t SFLOAT_SPECIAL_POS_INFINTY = 0x07FE;
    static constexpr uint16_t SFLOAT_SPECIAL_NAN = 0x07FF;
    static constexpr uint16_t SFLOAT_SPECIAL_NOT_RES = 0x0800;
}

SFloat::SFloat(float value)
{
    val = sfloatFromFloat(value);
}

SFloat::SFloat(uint16_t rawValue) : val(rawValue) {}

float SFloat::toFloat() const
{
    int exponent = (val & SFLOAT_EXP_MASK) >> SFLOAT_EXP_BIT_POS;
    int mantissa = val & SFLOAT_MANTISSA_MASK;

    if (exponent > SFLOAT_EXP_MAX)
    {
        exponent = -((~exponent + 1) & 0x0F);
    }

    if (exponent == 0 && mantissa >= SFLOAT_SPECIAL_POS_INFINTY)
    {
        return mantissa == SFLOAT_MANTISSA_MAX ? INFINITY : NAN;
    }

    if (mantissa > SFLOAT_MANTISSA_MAX)
    {
        mantissa = -((~mantissa & SFLOAT_MANTISSA_MASK) + 1);
    }

    return static_cast<float>(mantissa * std::pow(10.0f, exponent));
}

uint16_t SFloat::value() const
{
    return val;
}

uint16_t SFloat::sfloatFromFloat(float floatInput)
{
    uint8_t exponent = 0;
    uint16_t mantissa = 0;
    union
    {
        uint32_t val;
        struct
        {
            uint32_t mantissa : 23;
            uint32_t exp : 8;
            uint32_t sign : 1;
        };
    } floatEnc;

    floatEnc.val = sys_get_le32(reinterpret_cast<uint8_t *>(&floatInput));

    if ((floatEnc.exp == 0) && (floatEnc.mantissa == 0))
    {
        return encode(0, 0);
    }

    if (floatEnc.exp == UINT8_MAX)
    {
        mantissa = floatEnc.mantissa == 0 ? (floatEnc.sign ? SFLOAT_SPECIAL_NEG_INFINTY : SFLOAT_SPECIAL_POS_INFINTY)
                                          : SFLOAT_SPECIAL_NAN;
        return encode(0, mantissa);
    }

    float floatAbs = floatEnc.sign ? -floatInput : floatInput;
    if (floatAbs < FLOAT_ABS_MIN || floatInput < FLOAT_NEG_MIN || floatInput > FLOAT_POS_MAX)
    {
        mantissa = SFLOAT_SPECIAL_POS_INFINTY;
        return encode(0, mantissa);
    }

    uint16_t mantissaMax = floatEnc.sign ? -SFLOAT_MANTISSA_MIN : SFLOAT_MANTISSA_MAX;
    bool incExp = floatAbs > mantissaMax;
    int8_t exp = 0;

    while (exp > SFLOAT_EXP_MIN && exp < SFLOAT_EXP_MAX)
    {
        if (incExp)
        {
            if (floatAbs <= mantissaMax)
                break;
            floatAbs /= 10;
            exp++;
        }
        else
        {
            if ((floatAbs * 10) > mantissaMax)
                break;
            floatAbs *= 10;
            exp--;
        }
    }

    mantissa = static_cast<uint16_t>(std::round(floatAbs));
    if (mantissa > mantissaMax)
    {
        mantissa = mantissaMax;
    }

    exponent = exp >= 0 ? (exp & 0x0F) : ((~(-exp) & 0x0F) + 1);
    mantissa = mantissa & SFLOAT_MANTISSA_MASK;
    if (floatEnc.sign)
    {
        mantissa = (~mantissa & SFLOAT_MANTISSA_MASK) + 1;
    }

    return encode(exponent, mantissa);
}

uint16_t SFloat::encode(uint8_t exponent, uint16_t mantissa)
{
    return ((exponent << SFLOAT_EXP_BIT_POS) & SFLOAT_EXP_MASK) | (mantissa & SFLOAT_MANTISSA_MASK);
}
