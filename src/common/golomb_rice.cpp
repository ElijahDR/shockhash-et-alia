#include "common/golomb_rice.h"

// https://michaeldipperstein.github.io/rice.html
GolombEncodedData golomb_rice_encode(uint32_t value, uint32_t r)
{
    uint32_t quotient = value >> r;
    uint32_t remainder = value & ((1 << r) - 1);

    std::vector<bool> fixed;
    std::vector<bool> unary;

    for (int i = 0; i < quotient; i++)
    {
        unary.push_back(true);
    }
    unary.push_back(false);

    for (int i = r - 1; i >= 0; i--)
    {
        fixed.push_back((remainder >> i) & 1);
    }

    return GolombEncodedData{fixed, unary};
}

uint32_t golomb_rice_decode(GolombEncodedData data, uint32_t r)
{
    uint32_t quotient = data.unary.size() - 1;

    int pos = 0;
    uint32_t remainder = 0;
    for (int i = 0; i < r; ++i)
    {
        remainder = (remainder << 1) | data.fixed[pos++];
    }
    return (quotient << r) | remainder;
}