#ifndef GOLOMB_RICE_H
#define GOLOMB_RICE_H

#include <cstdint>
#include <vector>

struct GolombEncodedData
{
    std::vector<bool> fixed;
    std::vector<bool> unary;
};
GolombEncodedData golomb_rice_encode(uint32_t value, uint32_t r);
uint32_t golomb_rice_decode(GolombEncodedData data, uint32_t r);

#endif