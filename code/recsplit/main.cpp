#include <iostream>
#include <random>
#include <bit>
using namespace std;

constexpr int LEAF_SIZE = 2;
float GOLDEN_RATIO = (sqrt(5) + 1) / 2;
string keys[] = {"Hello", "World", "RecSplit", "Nelson", "Horatio", 
                    "Napoleon", "Alexander", "Victory", "Great", "Nile",
                    "Vincent", "Dock", "Longbow", "Whistle", "Thyme"};
constexpr int constants[] = {1, 2, 3, 5, 8, 13, 21}; 
bool little_endian = endian::native == endian::little;

// constexpr float computer_golden_ratio() {
//     return (sqrt(5) + 1) / 2;
// }
// constexpr float golden_ratio = (sqrt(5) + 1) / 2;

uint32_t ROL32(uint32_t k, uint32_t n) {
    uint32_t l = k << n;
    uint32_t r = k >> (32 - n);
    return l | r;
}

// https://en.wikipedia.org/wiki/MurmurHash#Algorithm
uint32_t murmur_hash_3_uint32(uint32_t seed, string key) {
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
        uint32_t chunk = 0;
        std::memcpy(&chunk, buffer + i, 4);

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

        if (!little_endian) {
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

uint32_t hash_key(string key) {
    hash<string> hasher;
    return hasher(key);
}

int highest_bit(int x) {
    int index = -1;
    while (x > 0) {
        x >>= 1;
        index++;
    }
    return index;
}

int nearest_integer_log(int x) {
    int y = x + (x >> 1);

    return highest_bit(y);
}

bool test_murmur3_32() {
    string test_keys[4] = {"Hello", "World", "Horatio", "Nelson"};
    uint32_t expected_hash[4] = {1466740371, 3789324275, 2689083821, 2244112232};

    bool test = true;
    for (int i = 0; i < 4; i++) {
        test &= (murmur_hash_3_uint32(42, test_keys[i]) == expected_hash[i]);
    }

    return test;
}

int main() {
    cout << "Murmur3 32 hash test: ";
    cout << (test_murmur3_32() ? "Passed" : "!!! FAILED !!!");
    cout << endl;

    for (string key : keys) {
        uint32_t hash = murmur_hash_3_uint32(42, key);
    }
    return 0;
}
