#ifndef SFLOAT_H
#define SFLOAT_H

#include <cstdint>

class SFloat
{
public:
    SFloat() = default;
    explicit SFloat(float value);
    explicit SFloat(uint16_t rawValue);

    float toFloat() const;
    uint16_t value() const;

private:
    uint16_t val;

    static uint16_t sfloatFromFloat(float value);
    static uint16_t encode(uint8_t exponent, uint16_t mantissa);
};

#endif // SFLOAT_H
