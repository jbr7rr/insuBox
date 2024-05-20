#ifndef UTILS_VECTOR_H
#define UTILS_VECTOR_H

#include <vector>

void vector_add_le32(std::vector<uint8_t> &vec, uint32_t value)
{
    vec.push_back(static_cast<uint8_t>(value & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    vec.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

#endif // UTILS_VECTOR_H