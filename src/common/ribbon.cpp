#include "common/ribbon.h"
#include "common/murmurhash.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

BasicRibbon::BasicRibbon(std::vector<std::string> &keys, std::vector<std::uint64_t> &values, uint64_t value_bits, double epsilon, uint64_t ribbon_width) {
    n = keys.size();
    w = ribbon_width;
    r = value_bits;
    e = epsilon;
    m = (uint64_t)((1 + e) * n) + 1;
    if (m < 64) {
        m = 65;
    }

    if (w != 64) {
        throw std::invalid_argument("Ribbon Width 64 currently only one supported!");
    }

    table.resize(m, 0);
    b.resize(m, 0);

    DEBUG_LOG("Building Basic Ribbon with values w: " << w << " m: " << m << " n:" << n << " epsilon: " << e << " r: " << r);
    seed_ = 0;
    while (true) {
        bool complete = true;
        for (int i = 0; i < keys.size(); i++) {
            if (!insert(keys[i], values[i], seed_)) {
                complete = false;
                break;
            }
        }

        if (!complete) {
            seed_++;
            continue;;
        }

        break;
    }

    solve();
    make_compact_z();
}

uint32_t BasicRibbon::space() {

    return compact_Z.size() * 64;
}

void BasicRibbon::make_compact_z() {
    int number_compact = std::ceil((m * r) / 64);
    // compact_Z.resize(number_compact, 0);

    for (auto z : Z) {
        DEBUG_LOG(std::bitset<8>(z));
    }

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < m; j+=64) {
            uint64_t current_Z = 0;
            for (int k = 0; k < 64; k++) {
                if (k >= m) {
                    break;
                }

                uint64_t value = (Z[j+k] & (1 << i)) >> i;
                current_Z |= (value << k);
            }
            compact_Z.push_back(current_Z);
        }
    }
    for (auto z : compact_Z) {
        DEBUG_LOG(std::bitset<64>(z));
    }
}

uint64_t BasicRibbon::query(const std::string &key) {
    // DEBUG_LOG("Query of " << key);
    uint64_t start_pos = murmur64(key, seed_) % (m - w);
    // DEBUG_LOG("Start Pos: " << start_pos);

    int num_words_per_bit = std::ceil((double)m / 64);

    uint64_t value = 0;
    for (int i = 0; i < r; i++) {
        uint64_t row_vector = murmur64(key, seed_);
        // DEBUG_LOG("Row Vector: " << std::bitset<64>(row_vector));
        // DEBUG_LOG("Extracting bit " << i);
        for (int j = 0; j < w; j++) {
            // DEBUG_LOG("Trying: " << j);
            // DEBUG_LOG("Row Vector: " << std::bitset<64>(row_vector));
            if (row_vector & 1) {
                int index = start_pos + j;
                // DEBUG_LOG("Index: " << index);
                
                int word_index = (i * num_words_per_bit) + (index / 64);
                // DEBUG_LOG("Word Index: " << word_index);
                // DEBUG_LOG("Word: " << std::bitset<64>(compact_Z[word_index]));
                int bit_index = index % 64;
                // DEBUG_LOG("Bit Index: " << bit_index);

                uint64_t extracted = (compact_Z[word_index] & ((uint64_t)1 << bit_index)) >> bit_index;
                // DEBUG_LOG("Z Value: " << std::bitset<8>(Z[start_pos + j]));
                // DEBUG_LOG("Extracted Bit: " << extracted);
                value ^= (extracted << i);
            }

            if (row_vector == 0) {
                break;
            }
            row_vector >>= 1;
        }
    }

    // uint8_t value = 0;
    // for (int i = 0; i < w; i++) {
    //     if (row_vector & 1) {
    //         value ^= Z[start_pos + i];
    //     }

    //     if (row_vector == 0) {
    //         break;
    //     }

    //     row_vector >>= 1;
    // }

    return value;
}

BasicRibbon::BasicRibbon(uint64_t number_items, uint64_t ribbon_width, uint64_t value_bits, double epsilon) {
    n = number_items;
    w = ribbon_width;
    r = value_bits;
    e = epsilon;
    m = (uint64_t)((1 + e) * n) + 1;

    if (w != 64) {
        throw std::invalid_argument("Ribbon Width 64 currently only one supported!");
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

// uint8_t BasicRibbon::query(std::string &key) {
//     uint64_t row_vector = murmur64(key, seed_);
//     uint64_t start_pos = murmur64(key, seed_) % (m - w);

//     uint8_t value = 0;
//     for (int i = 0; i < w; i++) {
//         if (row_vector & 1) {
//             value ^= Z[start_pos + i];
//         }

//         if (row_vector == 0) {
//             break;
//         }

//         row_vector >>= 1;
//     }

//     return value;
// }
BuRR::BuRR(std::vector<std::string> &keys, std::vector<std::uint64_t> &values, 
    uint64_t value_bits, double epsilon, uint64_t bucket_size, int num_layers, uint64_t ribbon_width) {
    n = keys.size();
    w = ribbon_width;
    r = value_bits;
    e = epsilon;
    num_layers_ = num_layers;
    bucket_size_ = bucket_size;
    m = (uint64_t)((1 + e) * n) + 1;
    if (m < 64) {
        m = 65;
    }

    if (w != 64) {
        throw std::invalid_argument("Ribbon Width 64 currently only one supported!");
    }

    table.resize(m, 0);
    b.resize(m, 0);



    DEBUG_LOG("Building BuRR with values w: " << w << " m: " << m << " n:" << n << " epsilon: " << e << " r: " << r);
    seed_ = 0;
    while (seed_ < 10) {
        bool complete = true;
        std::vector<uint64_t> bumped_values;
        std::vector<std::string> bumped_keys;
        std::vector<std::pair<uint64_t, uint64_t>> start_positions;
        for (int i = 0; i < keys.size(); i++) {
            uint64_t start_pos= murmur64(keys[i], seed_) % (m - w);
            start_positions.push_back(std::make_pair(start_pos, i));
        }
        std::sort(start_positions.begin(), start_positions.end());

        metadata.clear();
        metadata.resize((m / bucket_size), 0);
        for (size_t i = 0; i < start_positions.size();) {
            size_t bucket_start = i;
            uint64_t bucket_index = start_positions[i].first / bucket_size;
            uint64_t first_start_pos_of_bucket = start_positions[i].first;

            bool bumped = false;
            // Find the end of the current bucket
            while (i < start_positions.size() && (start_positions[i].first / bucket_size) == bucket_index) {
                i++;
            }

            for (size_t j = i; j > bucket_start;) {
                j--;
                size_t key_idx = start_positions[j].second;
                if (!insert(keys[key_idx], values[key_idx], seed_)) {
                    bumped = true;
                    uint64_t start_pos = start_positions[j].first;
                    uint64_t bucket_idx = start_pos / bucket_size;

                    for (int k = metadata[bucket_idx]; k < 4; k++){
                        if (start_pos <= bucket_start + threshold_values[k]) {
                            metadata[bucket_idx] = k;
                        }
                    }

                    // Undo previous insertions
                    int threshold_start_pos = bucket_start + threshold_values[metadata[bucket_idx]];
                    for (int k = previous_insertions.size(); k >= 0; k--) {
                        if (previous_insertions[k].first > threshold_start_pos) {
                            break;
                        }

                        table[previous_insertions[k].second] = 0;
                        b[previous_insertions[k].second] = 0;
                    }

                    // Bump all keys start position > current, but less than bumping threshold
                    for (int k = j + 1; k < i; k++) {
                        if (start_positions[k].first > threshold_start_pos) {
                            break;
                        }
                        bumped_keys.push_back(keys[start_positions[k].second]);
                        bumped_values.push_back(values[start_positions[k].second]);
                    }

                    bumped_keys.push_back(keys[key_idx]);
                    bumped_values.push_back(values[key_idx]);

                    // All other elements in bucket will have lower start pos so just bump them
                    for (int k = j - 1; j > bucket_start; j++) {
                        bumped_keys.push_back(keys[start_positions[k].second]);
                        bumped_values.push_back(values[start_positions[k].second]);
                    }
                    break;
                }
            }
            if (bumped && num_layers > 0) {
                fallback_burr_ptr = std::make_unique<BuRR>(bumped_keys, bumped_values, r, e, bucket_size, num_layers-1);
            } else if (bumped) {

            }
        }


        if (!complete) {
            seed_++;
            continue;;
        }

        break;
    }

    solve();
    make_compact_z();
}

bool BuRR::insert(std::string &key, std::uint8_t value, uint32_t seed) {
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
            previous_insertions.push_back(std::make_pair(start_pos, start_pos + index));
            return true;
        }
    }

    return false;
}