#ifndef CRYPT_H
#define CRYPT_H

#include <array>
#include <cstdint>

class Crypt
{
public:
    static uint32_t keyGen(uint32_t input);
    static uint32_t generateRandomToken();
    static uint32_t simpleDecrypt(uint32_t inputData);

private:
    static uint32_t simpleCrypt(uint32_t inputData);
    static uint32_t randomGen(uint32_t input);
    static uint32_t changeByTable(uint32_t inputData, const std::array<uint8_t, 256> &tableData);
    static uint32_t rotatoLeft(uint32_t x, int s, int n);
    static uint32_t rotatoRight(uint32_t x, int s, int n);
};

#endif // CRYPT_H
