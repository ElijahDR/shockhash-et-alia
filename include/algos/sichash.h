#ifndef SICHASH_H
#define SICHASH_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "algos/hash_function.h"
#include "common/ribbon.h"


class SicHash : public HashFunction {
public:
    SicHash(uint32_t bucket_size, double p1, double p2, double alpha=0.6, uint32_t bucket_seed=42, uint32_t class_seed=1);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;
    uint32_t naive_hash(const std::string &key);

    std::string name() override { return "SicHash"; };

private:
    void create_buckets();
    uint32_t assign_bucket(const std::string &key);
    void assign_classes();
    std::vector<uint32_t> generate_hash(size_t index, uint32_t base_seed);
    uint32_t extract_class_assignment(size_t index);
    void build_cuckoo_hash_table(const std::vector<size_t> &bucket);
    bool insert_into_hash_table(size_t index, uint32_t base_seed, std::vector<int> &hash_table);
    uint64_t key_class(const std::string &key);
    uint64_t get_hash_index(std::string &key);

    std::vector<bool> class_assignments_;
    double p1_;
    double p2_;
    double alpha_;
    uint32_t class_seed_;

    std::vector<int> rattle_counters_;
    std::vector<std::vector<bool>> hash_indexes_;
    std::map<std::string, std::vector<bool>> hash_index_map_;
    std::map<std::string, uint32_t> hash_index_map_raw_;

    std::vector<BasicRibbon> ribbons;

    uint32_t bucket_seed_;
    uint32_t bucket_size_;
    std::vector<std::string> keys_;

    std::vector<std::vector<size_t>> buckets_;
    std::vector<uint32_t> bucket_sizes_;
    std::vector<uint32_t> hash_table_sizes_;

    std::vector<std::vector<std::string>> keys_classes_;
    std::vector<std::vector<uint64_t>> hash_indexes_per_class_;

    std::vector<uint32_t> bucket_seeds_;
    std::vector<size_t> bucket_prefixes_;
};


#endif