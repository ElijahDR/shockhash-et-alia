#include "common/murmurhash.h"
#include <bit>
#include <cstdint>  
#include <cstring>

uint32_t ROL32(uint32_t k, uint32_t n) {
    uint32_t l = k << n;
    uint32_t r = k >> (32 - n);
    return l | r;
}

// https://en.wikipedia.org/wiki/MurmurHash#Algorithm
uint32_t murmur32(const std::string &key, const uint32_t seed) {
    constexpr uint32_t c1 = 0xcc9e2d51;
    constexpr uint32_t c2 = 0x1b873593;
    constexpr int r1 = 15;
    constexpr int r2 = 13;
    constexpr int m = 5;
    constexpr uint32_t n = 0xe6546b64;

    uint32_t hash = seed;

    unsigned char buffer[key.length()];
    memcpy(buffer, key.data(), key.length());
    // cout << buffer;
    // cout << "\n";
    // cout << key.length();
    // cout << "\n";

    for (int i = 0; i <= key.length() - 4; i += 4) {
        if (key.length() < 4) {
            break;
        }
        uint32_t chunk = 0;
        memcpy(&chunk, buffer + i, 4);

        uint32_t k = chunk;
        k *= c1;
        k = ROL32(k, r1);
        k *= c2;

        hash ^= k;
        hash = ROL32(hash, r2);
        hash = (hash * m) + n;
    }

    int leftover = key.length() % 4;
    if (leftover != 0) {
        uint32_t chunk = 0;
        std::memcpy(&chunk, buffer + (key.length() - leftover), leftover);

        // if (!(std::endian::native == std::endian::little))
        // {
        //     chunk = __builtin_bswap32(chunk);
        // }

        chunk *= c1;
        chunk = ROL32(chunk, r1);
        chunk *= c2;

        hash ^= chunk;
    }

    hash ^= key.length();

    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

inline uint64_t fmix64 (uint64_t k) {
  k ^= k >> 33;
  k *= 0xff51afd7ed558ccd;
  k ^= k >> 33;
  k *= 0xc4ceb9fe1a85ec53;
  k ^= k >> 33;

  return k;
}


// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash3.cpp
uint128_t murmur128(const std::string &key, const uint32_t &seed) {
    uint64_t h1 = seed;
    uint64_t h2 = seed;

    const uint64_t c1 = 0x87c37b91114253d5;
    const uint64_t c2 = 0x4cf5ad432745937f;

    const size_t len = key.length();

    unsigned char buffer[key.length()];
    memcpy(buffer, key.data(), key.length());

    for (int i = 0; i <= key.length() - 16; i += 16) {
        if (key.length() < 16) {
            break;
        }
        uint64_t k1 = 0, k2 = 0;
        memcpy(&k1, buffer + i, 8);
        memcpy(&k2, buffer + i + 8, 8);

        k1 *= c1;
        k1  = std::rotl(k1,31);
        k1 *= c2;
        h1 ^= k1;
    
        h1 = std::rotl(h1,27); 
        h1 += h2;
        h1 = h1*5+0x52dce729;
    
        k2 *= c2; 
        k2 = std::rotl(k2,33);
        k2 *= c1;
        h2 ^= k2;
    
        h2 = std::rotl(h2,31); 
        h2 += h1; 
        h2 = h2*5+0x38495ab5;
    }


    int leftover = key.length() % 16;
    unsigned char tail[leftover];
    std::memcpy(&tail, buffer + (len - leftover), leftover);

    uint64_t k1 = 0;
    uint64_t k2 = 0;

    switch(len & 15)
    {
    case 15: k2 ^= ((uint64_t)tail[14]) << 48;
    case 14: k2 ^= ((uint64_t)tail[13]) << 40;
    case 13: k2 ^= ((uint64_t)tail[12]) << 32;
    case 12: k2 ^= ((uint64_t)tail[11]) << 24;
    case 11: k2 ^= ((uint64_t)tail[10]) << 16;
    case 10: k2 ^= ((uint64_t)tail[ 9]) << 8;
    case  9: k2 ^= ((uint64_t)tail[ 8]) << 0;
    k2 *= c2; k2  = std::rotl(k2,33); k2 *= c1; h2 ^= k2;

    case  8: k1 ^= ((uint64_t)tail[ 7]) << 56;
    case  7: k1 ^= ((uint64_t)tail[ 6]) << 48;
    case  6: k1 ^= ((uint64_t)tail[ 5]) << 40;
    case  5: k1 ^= ((uint64_t)tail[ 4]) << 32;
    case  4: k1 ^= ((uint64_t)tail[ 3]) << 24;
    case  3: k1 ^= ((uint64_t)tail[ 2]) << 16;
    case  2: k1 ^= ((uint64_t)tail[ 1]) << 8;
    case  1: k1 ^= ((uint64_t)tail[ 0]) << 0;
    k1 *= c1; k1  = std::rotl(k1,31); k1 *= c2; h1 ^= k1;
    };

    h1 ^= len; h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    return ((uint128_t)h1 << 64) | h2;
}

// https://github.com/aappleby/smhasher/issues/61
uint64_t murmur64(const std::string &key, const uint32_t &seed) {
    uint128_t hash128 = murmur128(key, seed);
    uint64_t h1 = (uint64_t)hash128;
    uint64_t h2 = (uint64_t)(hash128 >> 64);
    return h2 * ((fmix64(h1) & 3) + 1) + h1;
}