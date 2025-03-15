#ifndef MURMURHASH_H
#define MURMURHASH_H

#include <cstdint>
#include <string>

uint32_t murmur_hash(uint32_t seed, std::string key);
uint32_t ROL32(uint32_t k, uint32_t n);

#endif