#ifndef BROADWORD_H
#define BROADWORD_H

#include "common/utils.h"
#include <vector>

const uint64_t L8 = 0x0101010101010101;

// https://vigna.di.unimi.it/ftp/papers/Broadword.pdf
inline uint64_t sideways_addition(uint64_t x);
// uint64_t rank9(const uint64_t *bits, const uint64_t)

#endif