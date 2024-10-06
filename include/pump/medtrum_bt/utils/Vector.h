#ifndef UTILS_VECTOR_H
#define UTILS_VECTOR_H

#include <cstdint>
#include <vector>

void vector_add_le32(std::vector<uint8_t> &vec, uint32_t value);
void vector_add_le16(std::vector<uint8_t> &vec, uint16_t value);

#endif // UTILS_VECTOR_H