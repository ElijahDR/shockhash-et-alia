#include "common/broadword.h"
#include <cmath>
#include <iostream>

inline uint64_t sideways_addition(uint64_t x) {
    x = x - ((x & 0xAAAAAAAAAAAAAAA) >> 1);
    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0;
    return x * L8 >> 56;
}