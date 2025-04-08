#include "common/ribbon.h"
#include "common/murmurhash.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

BasicRibbon::BasicRibbon(uint64_t number_items, uint64_t ribbon_width, uint64_t value_bits, double epsilon) {
    n = number_items;
    w = ribbon_width;
    r = value_bits;
    e = epsilon;
    m = (uint64_t)((1 + e) * n) + 1;

    if (w != 64) {
        throw std::invalid_argument("Ribbon Width 64 currently only one supported!");
    } else if (r > 8) {
        throw std::invalid_argument("R values above 8 not yet supported!");

    }

    table.resize(m, 0);
    b.resize(m, 0);
}

bool BasicRibbon::insert(std::string &key, std::uint8_t value, uint32_t seed) {
    DEBUG_LOG("Inserting Key: " << key << " with Value: " << std::bitset<8>(value) << " and Seed: " << seed);
    uint64_t row_vector = murmur64(key, seed);
    uint64_t start_pos = murmur64(key, seed) % (m - w);
    DEBUG_LOG("Current Row Vector (random bits): " << std::bitset<64>(row_vector));
    DEBUG_LOG("Current Start Pos: " << start_pos);

    uint64_t index = 0;
    while (row_vector > 0) {
        while (!(row_vector & (1))) {
            if (row_vector == 0) {
                return false;
            }
            row_vector >>= 1;
            index++;

        }

        if (table[start_pos + index] & 1) {
            row_vector ^= table[start_pos + index];
            value ^= b[start_pos + index];
        } else {
            table[start_pos + index] = row_vector;
            b[start_pos + index] = value;
            return true;
        }
    }

    return false;
}

void BasicRibbon::build(std::vector<std::string> &keys, std::vector<std::uint8_t> values) {
    DEBUG_LOG("Building Basic Ribbon with values w: " << w << " m: " << m << " n:" << n << " epsilon: " << e << " r: " << r);
    uint32_t seed = 0;
    while (true) {
        for (auto entry : table) {
            DEBUG_LOG(std::bitset<64>(entry));
        }
    
        for (auto entry : b) {
            DEBUG_LOG(std::bitset<8>(entry));
        }
        bool complete = true;
        for (int i = 0; i < keys.size(); i++) {
            if (!insert(keys[i], values[i], seed)) {
                complete = false;
                break;
            }
            // for (auto entry : table) {
            //     DEBUG_LOG(std::bitset<64>(entry));
            // }
        
            // for (auto entry : b) {
            //     DEBUG_LOG(std::bitset<8>(entry));
            // }
        }

        if (!complete) {
            seed++;
            continue;;
        }

        break;
    }

    seed_ = seed;

    // for (auto entry : table) {
    //     DEBUG_LOG(std::bitset<64>(entry));
    // }

    // for (auto entry : b) {
    //     DEBUG_LOG(std::bitset<8>(entry));
    // }

    solve();
}

bool BasicRibbon::solve() {
    Z.resize(m, 0);
    for (int i = m-1; i >= 0; i--) {
        if (table[i] == 0) {
            Z[i] = 0;
            continue;
        }

        uint64_t value = table[i];
        uint8_t z_value = b[i];
        for (int j = 1; j < w; j++) {
            value >>= 1;
            if (value & 1) {
                z_value ^= Z[i + j];
            }

            if (value == 0) {
                break;
            }

        }

        Z[i] = z_value;
    }

    for (auto entry : Z) {
        DEBUG_LOG(std::bitset<8>(entry));
    }

    return true;
}

uint8_t BasicRibbon::query(std::string &key) {
    uint64_t row_vector = murmur64(key, seed_);
    uint64_t start_pos = murmur64(key, seed_) % (m - w);

    uint8_t value = 0;
    for (int i = 0; i < w; i++) {
        if (row_vector & 1) {
            value ^= Z[start_pos + i];
        }

        if (row_vector == 0) {
            break;
        }

        row_vector >>= 1;
    }

    return value;
}
