#include "common/murmurhash.h"

uint32_t ROL32(uint32_t k, uint32_t n)
{
    uint32_t l = k << n;
    uint32_t r = k >> (32 - n);
    return l | r;
}

// https://en.wikipedia.org/wiki/MurmurHash#Algorithm
uint32_t murmur_hash(uint32_t seed, std::string key)
{
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

    for (int i = 0; i <= key.length() - 4; i += 4)
    {
        if (key.length() < 4)
        {
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
    if (leftover != 0)
    {
        uint32_t chunk = 0;
        std::memcpy(&chunk, buffer + (key.length() - leftover), leftover);

        if (!endian::native == endian::little)
        {
            chunk = __builtin_bswap32(chunk);
        }

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