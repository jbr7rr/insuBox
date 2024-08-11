#ifndef CRCUTIL_H
#define CRCUTIL_H

#include <cstdint>
#include <cstring>

class CrcUtil
{
public:
    static uint8_t crc8(const uint8_t *value, size_t size, uint8_t initial = 0);

private:
};

#endif // CRCUTIL_H
