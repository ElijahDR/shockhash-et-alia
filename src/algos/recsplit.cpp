#include "algos/recsplit.h"
#include "common/murmurhash.h"
#include "common/utils.h"
#include "common/golomb_rice.h"
#include <cmath>
#include <set>
#include <iostream>
#include <random>

RecSplit::RecSplit(uint32_t bucket_size, uint32_t leaf_size)
    : bucket_size_(bucket_size), leaf_size_(leaf_size), splitting_tree_() {
    part_sizes_ = {
        (uint32_t)std::max(2, (int)std::ceil(0.35 * leaf_size + 0.5)),
        leaf_size_ >= 7 ? (uint32_t)std::ceil(0.21 * leaf_size + 0.9) : 2,
        2,
    };

    bucket_seed_ = 43;

    golomb_rice_parameters_leaf_.resize(leaf_size+1);
    for (uint32_t i = 2; i <= leaf_size; i++) {
        golomb_rice_parameters_leaf_[i] = compute_grp_bijection(i);
    }
    DEBUG_VECTOR_LOG("Golomb Rice Parameter Table: ", golomb_rice_parameters_leaf_);
}

void RecSplit::build(const std::vector<std::string> &keys) {
    keys_ = keys;
    DEBUG_LOG("Creating Buckets...");
    create_buckets();

    for (std::vector<std::string> bucket : buckets_) {
        DEBUG_VECTOR_LOG("Bucket being sent to Split(): ", bucket);
        split(bucket, 0);
    }

    std::cout << "Fixed: ";
    print_vector(splitting_tree_.fixed);
    std::cout << "Unary: ";
    print_vector(splitting_tree_.unary);
    uint32_t total_bits = splitting_tree_.fixed.size() + splitting_tree_.unary.size();
    float bits_per_key = (float)total_bits / keys_.size();
    std::cout << "Total Bits: " << total_bits << std::endl;
    std::cout << "Bits per Key: " << bits_per_key << std::endl;
}

uint32_t RecSplit::hash(std::string &key) {
    uint32_t bucket = assign_bucket(key);

    return 0;
}

void RecSplit::split(const std::vector<std::string> &keys, uint32_t depth) {
    if (keys.size() <= leaf_size_) {
        if (keys.size() == 1) {
            return;
        }
        uint32_t bijection_seed = find_bijection(keys);
        // uint32_t bijection_seed = find_bijection_random(keys);
        append_to_splitting_tree(bijection_seed, golomb_rice_parameters_leaf_[keys.size()]);
        return;
    }

    uint32_t parts = part_sizes_[depth > 2 ? 2 : depth];
    DEBUG_VECTOR_LOG("Keys: ", keys);
    DEBUG_LOG("Parts: " << parts);
    DEBUG_LOG("Depth: " << depth);

    std::vector<uint32_t> expected_counts(parts);
    for (int i = 0; i < parts; i++) {
        if (i < keys.size() % parts) {
            expected_counts[i] = ceil((float)keys.size() / (float)parts);
        } else {
            expected_counts[i] = floor((float)keys.size() / (float)parts);
        }
    }

    DEBUG_VECTOR_LOG("Expected Counts: ", expected_counts);
    uint32_t seed = 0;
    while (true) {
        DEBUG_LOG("Seed: " << seed);
        std::vector<uint32_t> counts(parts);
        for (std::string key : keys) {
            uint32_t hash = murmur_hash(seed, key);
            uint32_t part = hash % parts;
            // DEBUG_LOG("Key: " << key << " Part: " << hash%parts << " Hash: " << hash);
            counts[part] += 1;
            if (counts[part] > expected_counts[part]) {
                break;
            }
        }
        DEBUG_VECTOR_LOG("Actual Counts: ", counts);
        if (counts == expected_counts) {
            break;
        }
        seed++;
    }

    // Split up the keys for next level of split

    const uint32_t golomb_rice_param = compute_grp_buckets(expected_counts);
    append_to_splitting_tree(seed, golomb_rice_param);

    std::vector<std::vector<std::string>> keys_split(parts);
    for (std::string key : keys) {
        uint32_t hash = murmur_hash(seed, key);
        keys_split[hash % parts].push_back(key);
    }

    for (std::vector<std::string> key_part : keys_split) {
        split(key_part, depth+1);
    }
}

void RecSplit::append_to_splitting_tree(const uint32_t &data, const uint32_t golomb_rice_param) {
    DEBUG_LOG("Value: " << data);
    DEBUG_LOG("Golomb Rice Param: " << golomb_rice_param);
    
    GolombEncodedData encoded_data = golomb_rice_encode(
        data, 
        golomb_rice_param
    );
    DEBUG_VECTOR_LOG("Fixed: ", encoded_data.fixed);
    DEBUG_VECTOR_LOG("Unary: ", encoded_data.unary);
    append_vector_to_vector(splitting_tree_.fixed, encoded_data.fixed);
    append_vector_to_vector(splitting_tree_.unary, encoded_data.unary);
}

uint32_t RecSplit::assign_bucket(const std::string &key) {
    uint32_t hash = murmur_hash(bucket_seed_, key) >> 16;
    uint32_t bucket = floor((hash * buckets_.size()) >> 16);

    return bucket;
}

void RecSplit::create_buckets() {
    uint32_t bucket_count = ceil((float)keys_.size() / (float)bucket_size_);
    buckets_.resize(bucket_count);
    bucket_sizes_.resize(bucket_count, 0);
    for (int i = 0; i < keys_.size(); i++) {
        // Perform the bucket assigment
        uint32_t bucket = assign_bucket(keys_[i]);
        buckets_[bucket].push_back(keys_[i]);
        bucket_sizes_[bucket] += 1;
    }

    DEBUG_VECTOR_LOG("Bucket Sizes:", bucket_sizes_);
    DEBUG_LOG("Buckets: ");
    for (auto bucket : buckets_) {
        DEBUG_VECTOR_LOG("", bucket);
    }

    uint32_t bucket_index = 0;
    for (auto bucket_size : bucket_sizes_) {
        bucket_prefixes_.push_back(bucket_index);
        bucket_index += bucket_size;
    }
    bucket_prefixes_.push_back(bucket_index);
}

uint32_t RecSplit::find_bijection(const std::vector<std::string> &keys) {
    uint32_t seed = 0;
    while (true) {
        bool bijection = true;
        std::set<uint32_t> indexes;
        for (std::string key : keys) {
            uint32_t hash = murmur_hash(seed, key) % keys.size();
            // DEBUG_LOG("FINDING BIJECTION -- Key: " << key << " Hash: " << hash);
            if (indexes.contains(hash)) {
                bijection = false;
                // DEBUG_LOG("Bijection Failed");
                break;
            }
            indexes.insert(hash);
        }
        if (bijection) {
            return seed;
        }
        seed++;
    }
}

uint32_t RecSplit::find_bijection_random(const std::vector<std::string> &keys) {
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<uint32_t> dist(0, UINT32_MAX);

    while (true) {
        uint32_t seed = dist(rng);
        bool bijection = true;
        std::set<uint32_t> indexes;
        for (std::string key : keys) {
            uint32_t hash = murmur_hash(seed, key) % keys.size();
            DEBUG_LOG("FINDING BIJECTION -- Key: " << key << " Hash: " << hash);
            if (indexes.contains(hash)) {
                bijection = false;
                DEBUG_LOG("Bijection Failed");
                break;
            }

            indexes.insert(hash);
        }

        if (bijection) {
            return seed;
        }
    }
}