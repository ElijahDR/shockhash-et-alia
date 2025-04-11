#ifndef SHOCKHASH_H
#define SHOCKHASH_H

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include "algos/hash_function.h"
#include "common/ribbon.h"

struct CuckooHashData { 
    BasicRibbon hash_choices;
    uint32_t seed;
};

struct BipartiteCuckooHashData { 
    BasicRibbon hash_choices;
    std::pair<uint32_t, uint32_t> seeds;
};


class ShockHash : public HashFunction {
public:
    ShockHash(uint32_t bucket_size, uint32_t bucket_seed=42);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;

    std::string name() override { return "ShockHash"; };

private:
    bool filter_bit_mask(const std::vector<uint32_t> &keys, const uint32_t &seed);

    CuckooHashData create_cuckoo_table(const int bucket_id);

    std::vector<uint32_t> key_hashes(const std::string &key, uint32_t seed);
    bool insert_into_hash_table(const int key, std::vector<int> &hash_table, uint32_t seed);

    BasicRibbon hash_index;
    std::vector<bool> hash_choices_;

    uint32_t n_keys_;
    uint32_t bucket_size_;
    uint32_t bucket_seed_;
    std::vector<std::vector<uint32_t>> buckets_;

    std::vector<CuckooHashData> hashing_data_;

    std::vector<std::string> keys_;
};

class BipartiteShockHash : public HashFunction {
public:
    BipartiteShockHash(uint32_t bucket_size, uint32_t bucket_seed=42);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;

    std::string name() override { return "ShockHash"; };

private:
    bool filter_bit_mask_half(const std::vector<uint32_t> &keys, const uint32_t &seed);

    CuckooHashData create_cuckoo_table(const int bucket_id);

    std::vector<uint32_t> key_hashes(const std::string &key, uint32_t seed);
    bool insert_into_hash_table(const int key, std::vector<int> &hash_table, std::pair<uint32_t, uint32_t> seeds);

    BasicRibbon hash_index;
    std::vector<bool> hash_choices_;

    uint32_t n_keys_;
    uint32_t bucket_size_;
    uint32_t bucket_seed_;
    std::vector<std::vector<uint32_t>> buckets_;

    std::vector<CuckooHashData> hashing_data_;

    std::vector<std::string> keys_;
};

class ShockHashRS : public HashFunction {
public:
    ShockHashRS(uint32_t bucket_size, uint32_t bucket_seed=42);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;

    std::string name() override { return "ShockHash"; };

private:
    bool filter_bit_mask(const std::vector<uint32_t> &keys, const uint32_t &seed);

    CuckooHashData create_cuckoo_table(const int bucket_id);

    std::vector<uint32_t> key_hashes(const std::string &key, uint32_t seed);
    bool insert_into_hash_table(const int key, std::vector<int> &hash_table, uint32_t seed);

    BasicRibbon hash_index;
    std::vector<bool> hash_choices_;

    uint32_t n_keys_;
    uint32_t bucket_size_;
    uint32_t bucket_seed_;
    std::vector<std::vector<uint32_t>> buckets_;

    std::vector<CuckooHashData> hashing_data_;

    std::vector<std::string> keys_;
};


#endif