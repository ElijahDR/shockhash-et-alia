#include "algos/sichash.h"
#include "common/murmurhash.h"
#include "common/utils.h"
#include "common/golomb_rice.h"
#include <cmath>
#include <set>
#include <iostream>
#include <random>
#include <stdexcept>

SicHash::SicHash(uint32_t bucket_size, double p1, double p2, double alpha, uint32_t bucket_seed, uint32_t class_seed) : 
bucket_size_(bucket_size), p1_(p1), p2_(p2), alpha_(alpha), 
bucket_seed_(bucket_seed), class_seed_(class_seed), class_assignments_() {
    
    return;
}

void SicHash::build(const std::vector<std::string> &keys) {
    keys_ = keys;
    hash_indexes_.resize(keys.size());
    keys_classes_.resize(3);
    hash_indexes_per_class_.resize(3);

    assign_classes();
    create_buckets();
    std::cout << "Bucket Sizes: " << bucket_sizes_ << std::endl;
    
    for (auto bucket : buckets_) {
        DEBUG_LOG("Building Bucket: " << bucket);
        build_cuckoo_hash_table(bucket);
    }

    ribbons.push_back(BasicRibbon(keys_classes_[0], hash_indexes_per_class_[0], 1, 0.25));
    ribbons.push_back(BasicRibbon(keys_classes_[1], hash_indexes_per_class_[1], 2, 0.25));
    ribbons.push_back(BasicRibbon(keys_classes_[2], hash_indexes_per_class_[2], 3, 0.25));

    std::cout << "Bucket Seeds: " << bucket_seeds_ << std::endl;
}

uint32_t SicHash::hash(const std::string &key) {
    DEBUG_LOG("===================");
    DEBUG_LOG("Key: " << key);
    // auto it = std::find(keys_.begin(), keys_.end(), key);
    // DEBUG_LOG("Index: " << it - keys_.begin());
    int bucket = assign_bucket(key);
    DEBUG_LOG("Bucket for key " << key << ": " << bucket);
    int global_index = bucket_prefixes_[bucket];
    DEBUG_LOG("Global Index for key " << key << ": " << global_index);
    uint32_t bucket_seed = bucket_seeds_[bucket];

    uint64_t class_for_key = key_class(key);
    uint64_t hash_index = ribbons[class_for_key].query(key);
    // int hash_index = bits_to_int(hash_index_map_[key]);
    // int hash_index = hash_index_map_raw_[key];
    DEBUG_LOG("Hash Index for key " << key << ": " << hash_index);
    int table_size = bucket_sizes_[bucket] / alpha_;
    int hash_table_index = murmur32(key, (bucket_seed * 8) + hash_index) % table_size;
    DEBUG_LOG("Hash Table Index for key " << key << ": " << hash_table_index);
    DEBUG_LOG("Overall Hash for key" << key << ": " << global_index + hash_table_index);

    return global_index + hash_table_index;
}

uint32_t SicHash::naive_hash(const std::string &key) {
    DEBUG_LOG("===================");
    DEBUG_LOG("Key: " << key);
    // auto it = std::find(keys_.begin(), keys_.end(), key);
    // DEBUG_LOG("Index: " << it - keys_.begin());
    int bucket = assign_bucket(key);
    DEBUG_LOG("Bucket for key " << key << ": " << bucket);
    int global_index = bucket_prefixes_[bucket];
    DEBUG_LOG("Global Index for key " << key << ": " << global_index);
    uint32_t bucket_seed = bucket_seeds_[bucket];

    int hash_index = bits_to_int(hash_index_map_[key]);
    DEBUG_LOG("Hash Index for key " << key << ": " << hash_index);
    int table_size = bucket_sizes_[bucket] / alpha_;
    int hash_table_index = murmur32(key, (bucket_seed * 8) + hash_index) % table_size;
    DEBUG_LOG("Hash Table Index for key " << key << ": " << hash_table_index);
    DEBUG_LOG("Overall Hash for key" << key << ": " << global_index + hash_table_index);

    return global_index + hash_table_index;
}


void SicHash::assign_classes() {
    // class_assignments_.resize(keys_.size() * 2);
    std::vector<size_t> class_counts(3, 0);
    // DEBUG_LOG("Class Assignments: " << class_assignments_);     
    for (int i = 0; i < keys_.size(); i++) {
        size_t bit_index = 2 * i;
        std::string key = keys_[i];
        uint32_t hash = murmur32(key, class_seed_);
        double class_val = (double)hash / UINT32_MAX;

        // DEBUG_LOG("Hash Value: " << hash);
        // DEBUG_LOG("Class Value: " << class_val);

        if (class_val < p1_) { 
            class_assignments_.insert(class_assignments_.end(), {0, 1});
            class_counts[0]++;
        } else if (class_val < (p1_ + p2_)) {
            class_assignments_.insert(class_assignments_.end(), {1, 0});
            class_counts[1]++;
        } else {
            class_assignments_.insert(class_assignments_.end(), {1, 1});
            class_counts[2]++;
        }
    }

    DEBUG_LOG("Class Counts: " << class_counts);
    // DEBUG_LOG("Class Assignments: " << class_assignments_);
}

uint64_t SicHash::key_class(const std::string &key) {
    uint32_t hash = murmur32(key, class_seed_);
    double class_val = (double)hash / UINT32_MAX;

    if (class_val < p1_) { 
        return 0;
    } else if (class_val < (p1_ + p2_)) {
        return 1;
    } else {
        return 2;
    }
}

std::vector<uint32_t> SicHash::generate_hash(size_t index, uint32_t base_seed) {
    std::string key = keys_[index];
    int n_hash = extract_class_assignment(index);

    std::vector<uint32_t> hashes(n_hash);
    for (int i = 0; i < n_hash; i++) {
        hashes[i] = murmur32(key, base_seed + i);
    }

    return hashes;
}

uint32_t SicHash::extract_class_assignment(size_t index) {
    std::vector<bool> class_assignment = {class_assignments_[index*2], class_assignments_[(index*2)+1]};
    int n_hash;
    if (class_assignment == std::vector<bool>{0,1}) {
        return 2;
    } else if (class_assignment == std::vector<bool>{1,0}) {
        return 4;
    } else if (class_assignment == std::vector<bool>{1,1}) {
        return 8;
    } else {
        throw std::runtime_error("Class Assignment exctraction failed");
    }
}

HashFunctionSpace SicHash::space() {
    // std::vector<std::pair<std::string, int>> space_usage;
    // int splitting_tree_overhead = (sizeof(splitting_tree_) + sizeof(splitting_tree_.fixed) + sizeof(splitting_tree_.unary)) * 8;
    // space_usage.push_back(std::make_pair("Splitting Tree Overhead", splitting_tree_overhead));
    // space_usage.push_back(std::make_pair("Splitting Tree Fixed", splitting_tree_.fixed.size()));
    // space_usage.push_back(std::make_pair("Splitting Tree Unary", splitting_tree_.unary.size()));
    // int total_splitting_tree = splitting_tree_overhead + splitting_tree_.fixed.size() + splitting_tree_.unary.size();
    // space_usage.push_back(std::make_pair("Total Splitting Tree", total_splitting_tree));

    // space_usage.push_back(std::make_pair("Bucket Node Prefixes", elias_fano_space(bucket_node_prefixes_ef_)));
    // space_usage.push_back(std::make_pair("Bucket Unary Prefixes", elias_fano_space(bucket_unary_prefixes_ef_)));
    // space_usage.push_back(std::make_pair("Bucket Fixed Prefixes", elias_fano_space(bucket_fixed_prefixes_ef_)));
    // int total_bucket_prefixes = elias_fano_space(bucket_node_prefixes_ef_) + elias_fano_space(bucket_unary_prefixes_ef_) + elias_fano_space(bucket_fixed_prefixes_ef_);
    // space_usage.push_back(std::make_pair("Total Bucket Prefixes", total_bucket_prefixes));

    // int total_bits = total_bucket_prefixes + total_splitting_tree;
    // double bits_per_key = total_bits / (double)keys_.size();
    // return HashFunctionSpace{space_usage, total_bits, bits_per_key, (int)keys_.size()};
    return HashFunctionSpace{};
}   

void SicHash::build_cuckoo_hash_table(const std::vector<size_t> &bucket) {
    uint32_t seed = 0;
    while (true) {
        rattle_counters_.assign(keys_.size(), 0);
        DEBUG_LOG("Alpha: " << alpha_);
        int table_size = bucket.size() / alpha_;
        DEBUG_LOG("Table Size: " << table_size);
        std::vector<int> table(table_size, -1);
        bool insert;
        for (int i = 0; i < bucket.size(); i++) {
            // if (i % 500 == 0) {
            //     std::cout << "Inserted " << i << " elements" << std::endl;
            // }
            DEBUG_LOG("Seed: " << seed << " index: " << i);
            insert = insert_into_hash_table(bucket[i], seed * 8, table);
            DEBUG_LOG("Current Hash Table: " << table);
            DEBUG_LOG("Current Rattle Counters: " << rattle_counters_);
            if (!insert) break;
        }

        if (insert) {
            break;
        }

        seed++;
    }

    bucket_seeds_.push_back(seed);

    for (auto index : bucket) {
        uint32_t n_hash = extract_class_assignment(index);
        size_t hash_index = rattle_counters_[index] % n_hash;

        uint64_t class_n = key_class(keys_[index]);
        DEBUG_LOG("Class N: " << class_n);
        keys_classes_[key_class(keys_[index])].push_back(keys_[index]);
        hash_indexes_per_class_[key_class(keys_[index])].push_back(hash_index);

        hash_indexes_[index] = int_to_bits(hash_index);

        hash_index_map_[keys_[index]] = int_to_bits(hash_index);
        hash_index_map_raw_[keys_[index]] = hash_index;
    }
    DEBUG_LOG("Hash Index Map: " << hash_index_map_);
}

bool SicHash::insert_into_hash_table(size_t key_index, uint32_t base_seed, std::vector<int> &hash_table) {
    int retries = 0;
    while (retries < hash_table.size() * 2) {
        DEBUG_LOG("Current Hash Table: " << hash_table);
        DEBUG_LOG("Try: " << retries << " inserting: " << key_index);
        DEBUG_LOG("Element being inserted: " << keys_[key_index]);
        DEBUG_LOG("Key Index being inserted: " << key_index);
        int n_hash = extract_class_assignment(key_index);
        DEBUG_LOG("N Hashes: " << n_hash);
        std::vector<uint32_t> hashes = generate_hash(key_index, base_seed);
        int hash_index = rattle_counters_[key_index] % n_hash;
        DEBUG_LOG("Hash Index: " << hash_index);
        size_t table_index = hashes[hash_index] % hash_table.size();
        DEBUG_LOG("Table Index: " << table_index);
        if (hash_table[table_index] == -1) {
            DEBUG_LOG("Straight In");
            hash_table[table_index] = key_index;
            DEBUG_LOG("Inserting element took " << retries << " tries");
            return true;
        }

        size_t current_index = hash_table[table_index];
        if (rattle_counters_[key_index] > rattle_counters_[current_index]) {
            DEBUG_LOG("Swap: New goes in");
            hash_table[table_index] = key_index;
            key_index = current_index;
            rattle_counters_[key_index]++;
        } else {
            DEBUG_LOG("Swap: Original stays");
            rattle_counters_[key_index]++;
        }

        retries++;
    }

    DEBUG_LOG("FAILED INSERTING ELEMENT");
    return false;
}

void SicHash::create_buckets() {
    uint32_t bucket_count = ceil((float)keys_.size() / (float)bucket_size_);
    buckets_.resize(bucket_count);
    bucket_sizes_.resize(bucket_count, 0);

    for (int i = 0; i < keys_.size(); i++) {
        uint32_t bucket = assign_bucket(keys_[i]);
        buckets_[bucket].push_back(i);
        bucket_sizes_[bucket] += 1;
    }

    DEBUG_LOG("Buckets: " << buckets_);

    int bucket_prefix = 0;
    for (int i = 0; i < bucket_count; i++) {
        bucket_prefixes_.push_back(bucket_prefix);
        int table_size = bucket_sizes_[i] / alpha_;
        bucket_prefix += table_size;
    }
    bucket_prefixes_.push_back(bucket_prefix);
}

uint32_t SicHash::assign_bucket(const std::string &key) {
    uint32_t bucket_count = ceil((float)keys_.size() / (float)bucket_size_);
    uint32_t hash = murmur32(key, bucket_seed_) >> 16;
    uint32_t bucket = floor((hash * bucket_count) >> 16);

    return bucket;
}