#ifndef MURMURHASH_H
#define MURMURHASH_H

#include <cstdint>
#include <string>
#include "common/utils.h"
#include "common/MurmurHash3.h"

uint32_t murmur32(const std::string_view &key, const uint32_t seed);
void murmur32_ref(const std::string_view &key, const uint32_t seed, uint32_t &value);
// void murmur128_range(const std::string_view &key, const uint32_t &seed, const uint32_t &range, uint128_t &value);
inline void murmur128_range(const std::string_view &key, const uint32_t &seed, const uint32_t &range, uint128_t &value){
    MurmurHash3_x64_128(key.data(), (int)key.size(), seed, &value);
    value >>= 64;
    value = (value * range) >> 64;
}
uint32_t murmur32_old(const std::string_view &key, const uint32_t seed);
inline uint32_t ROL32(uint32_t k, uint32_t n);
inline uint64_t fmix64 (uint64_t k);
uint64_t murmur64(const std::string &key, const uint32_t &seed);
uint128_t murmur128(const std::string &key, const uint32_t &seed);
uint128_t murmur128_old(const std::string &key, const uint32_t &seed);

#endif