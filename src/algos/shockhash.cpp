#include "algos/shockhash.h"
#include "common/murmurhash.h"
#include "common/utils.h"
#include "common/golomb_rice.h"
#include <cmath>
#include <random>

ShockHash::ShockHash(uint32_t bucket_size, uint32_t bucket_seed) : 
bucket_size_(bucket_size), bucket_seed_(bucket_seed) {
    return;
}

void ShockHash::build(const std::vector<std::string> &keys) {
    keys_ = keys;
    n_keys_ = keys.size();
    hash_choices_.resize(n_keys_, 0);

    int n_buckets = std::ceil(n_keys_ / bucket_size_);
    buckets_.resize(n_buckets);
    DEBUG_LOG("Number of Buckets: " << n_buckets);
    for (int i = 0; i < n_keys_; i++) {
        int b = murmur128(keys[i], bucket_seed_) % n_buckets;
        buckets_[b].push_back(i);
    }

    hashing_data_.resize(n_buckets);
    for (int i = 0; i < n_buckets; i++) {
        hashing_data_[i] = create_cuckoo_table(i);
    }
}

CuckooHashData ShockHash::create_cuckoo_table(const int bucket_id) {
    DEBUG_LOG("Creating Hash Table for Bucket: " << bucket_id);
    uint32_t seed_ = 0;
    while (true) {
        if (seed_ % 1000000 == 0) {
            DEBUG_LOG("Seed: " << seed_);
        }
        if (!filter_bit_mask(buckets_[bucket_id], seed_)) {
            seed_++;
            continue;
        }

        DEBUG_LOG("Seed Passed BitMask: " << seed_);
        std::vector<int> hash_table(buckets_[bucket_id].size(), -1);
        hash_choices_.assign(n_keys_, 0);
        bool valid = true;
        for (const int &key_index : buckets_[bucket_id]) {
            if (!insert_into_hash_table(key_index, hash_table, seed_)) {
                valid = false;
                break;
            }
        }

        if (!valid) {
            seed_++;
            DEBUG_LOG("Seed Failed Cuckoo Hash Table");
            continue;
        }

        break;
    }

    DEBUG_LOG("Final Seed: " << seed_);
    return CuckooHashData{BasicRibbon(), 0};
}

bool ShockHash::insert_into_hash_table(const int key_index, std::vector<int> &hash_table, uint32_t seed) {
    int tries = 0;
    int current = key_index;
    int hash_table_size = hash_table.size();
    while (tries < hash_table_size) {
        uint32_t hash = murmur32(keys_[current], seed + hash_choices_[current]) % hash_table_size;
        // DEBUG_LOG("Inserting " << current << " into " << hash_table);
        // DEBUG_LOG("Current Choice: " << hash_choices_[current] << " current hash: " << hash);
        if (hash_table[hash] == -1) {
            hash_table[hash] = current;
            return true;
        }

        int new_current = hash_table[hash];
        hash_table[hash] = current;
        current = new_current;
        hash_choices_[current] = !hash_choices_[current];

        tries++;
    }

    return false;
}

HashFunctionSpace ShockHash::space() {
    return HashFunctionSpace{};
}   

uint32_t ShockHash::hash(const std::string &key) {
    return 1;
}

bool ShockHash::filter_bit_mask(const std::vector<uint32_t> &key_indexes, const uint32_t &seed) {
    int n_keys = key_indexes.size();
    int n_words = std::ceil((double)n_keys / 64);
    std::vector<uint64_t> bit_mask_vector(n_words, 0);
    int leftover = n_keys % 64;
    if (leftover > 0) {
        bit_mask_vector[n_words-1] |= ~(((uint64_t)1 << (leftover)) - 1);
    }

    for (const uint32_t &key_index : key_indexes) {
        int hash = murmur32(keys_[key_index], seed) % n_keys;
        int word_index = std::floor((double)hash / 64);
        int bit_index = hash % 64;
        bit_mask_vector[word_index] |= 1 << bit_index;

        hash = murmur32(keys_[key_index], seed+1) % n_keys;
        word_index = std::floor((double)hash / 64);
        bit_index = hash % 64;
        bit_mask_vector[word_index] |= 1 << bit_index;
    }

    // DEBUG_LOG("Trying Seed: " << seed);
    for (const uint64_t &bit_mask : bit_mask_vector) {
        // DEBUG_LOG("Seed: " << seed << " Bit Mask: " << std::bitset<64>(bit_mask));
        if (bit_mask != UINT64_MAX) {
            return false;
        }
    }

    return true;
}


BipartiteShockHash::BipartiteShockHash(uint32_t bucket_size, uint32_t bucket_seed) : 
bucket_size_(bucket_size), bucket_seed_(bucket_seed) {
    return;
}

void BipartiteShockHash::build(const std::vector<std::string> &keys) {
    keys_ = keys;
    n_keys_ = keys.size();
    hash_choices_.resize(n_keys_, 0);

    int n_buckets = std::ceil(n_keys_ / bucket_size_);
    buckets_.resize(n_buckets);
    DEBUG_LOG("Number of Buckets: " << n_buckets);
    for (int i = 0; i < n_keys_; i++) {
        int b = murmur128(keys[i], bucket_seed_) % n_buckets;
        buckets_[b].push_back(i);
    }

    hashing_data_.resize(n_buckets);
    for (int i = 0; i < n_buckets; i++) {
        hashing_data_[i] = create_cuckoo_table(i);
    }
}

uint32_t BipartiteShockHash::hash(const std::string &key) {
    return 1;
}

bool BipartiteShockHash::insert_into_hash_table(const int key_index, std::vector<int> &hash_table, std::pair<uint32_t, uint32_t> seeds) {
    int tries = 0;
    int current = key_index;
    int hash_table_size = hash_table.size();
    while (tries < 10 * hash_table_size) {
        uint32_t seed = hash_choices_[current] ? seeds.second : seeds.first;
        uint32_t hash = murmur32(keys_[current], seed) % (hash_table_size / 2);
        if (hash_choices_[current]) {
            hash += hash_table_size / 2;
        }
        DEBUG_LOG("Inserting " << current << " into " << hash_table);
        DEBUG_LOG("Current Choice: " << hash_choices_[current] << " current hash: " << hash);
        if (hash_table[hash] == -1) {
            hash_table[hash] = current;
            return true;
        }

        int new_current = hash_table[hash];
        hash_table[hash] = current;
        current = new_current;
        hash_choices_[current] = !hash_choices_[current];

        tries++;
    }

    return false;
}

HashFunctionSpace BipartiteShockHash::space() {
    return HashFunctionSpace{};
}   

bool BipartiteShockHash::filter_bit_mask_half(const std::vector<uint32_t> &key_indexes, const uint32_t &seed) {
    int n_keys = key_indexes.size() / 2;
    int n_words = std::ceil((double)n_keys / 64);
    std::vector<uint64_t> bit_mask_vector(n_words, 0);
    int leftover = n_keys % 64;
    if (leftover > 0) {
        bit_mask_vector[n_words-1] |= ~(((uint64_t)1 << (leftover)) - 1);
    }

    for (const uint32_t &key_index : key_indexes) {
        int hash = murmur32(keys_[key_index], seed) % (n_keys);
        int word_index = std::floor((double)hash / 64);
        int bit_index = hash % 64;
        bit_mask_vector[word_index] |= 1 << bit_index;
    }

    for (const uint64_t &bit_mask : bit_mask_vector) {
        if (bit_mask != UINT64_MAX) {
            return false;
        }
    }

    return true;
}

CuckooHashData BipartiteShockHash::create_cuckoo_table(const int bucket_id) {
    DEBUG_LOG("Creating Hash Table for Bucket: " << bucket_id);
    uint32_t seed = 0;
    std::pair<uint32_t, uint32_t> seeds;
    std::vector<uint32_t> surjective_candidates; 
    while (true) {
        if (!filter_bit_mask_half(buckets_[bucket_id], seed)) {
            seed++;
            continue;
        }

        DEBUG_LOG("Surjective Candidates: " << surjective_candidates);
        surjective_candidates.push_back(seed);
        if (surjective_candidates.size() < 2) {
            seed++;
            continue;
        }

        // DEBUG_LOG("Seed Passed BitMask: " << seed);
        bool seed_pair_worked = false;
        for (int i = 0; i < surjective_candidates.size() - 1; i++) {
            seeds = {surjective_candidates[surjective_candidates.size() - 1], surjective_candidates[i]};
            DEBUG_LOG("Trying Pair: " << seeds);
            // seeds = {surjective_candidates[i], surjective_candidates[surjective_candidates.size() - 1]};
            std::vector<int> hash_table(buckets_[bucket_id].size(), -1);
            hash_choices_.assign(n_keys_, 0);
            bool valid = true;
            for (const int &key_index : buckets_[bucket_id]) {
                if (!insert_into_hash_table(key_index, hash_table, seeds)) {
                    valid = false;
                    break;
                }
            }
    
            if (!valid) {
                continue;
            }
            
            seed_pair_worked = true;
            break;
        }

        if (seed_pair_worked) {
            break;
        }
        
        seed++;
    }

    DEBUG_LOG("Final Seeds: " << seeds);
    return CuckooHashData{BasicRibbon(), 0};
}