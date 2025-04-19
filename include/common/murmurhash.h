#ifndef MURMURHASH_H
#define MURMURHASH_H

#include <cstdint>
#include <string>
#include "common/utils.h"

uint32_t murmur32(const std::string_view &key, const uint32_t seed);
uint32_t murmur32_old(const std::string_view &key, const uint32_t seed);
inline uint32_t ROL32(uint32_t k, uint32_t n);
inline uint64_t fmix64 (uint64_t k);
uint64_t murmur64(const std::string &key, const uint32_t &seed);
uint128_t murmur128(const std::string &key, const uint32_t &seed);

#endif