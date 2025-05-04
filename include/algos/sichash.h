#ifndef SICHASH_H
#define SICHASH_H

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "algos/hash_function.h"
#include "common/ribbon.h"
#include "common/elias_fano.h"
#include "common/broadword.h"

struct BucketSeedGr {
    std::vector<bool> unary;
    std::vector<bool> fixed;
};


class SicHash : public HashFunction {
public:
    SicHash(uint32_t bucket_size, double p1, double p2, double alpha, double burr_epsilon=0, int layers=4, uint32_t bucket_seed=42, uint32_t class_seed=1);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;
    uint32_t perfect_hash(const std::string &key);
    uint32_t naive_hash(const std::string &key);

    std::string name() override { return "SicHash"; };
    HashFunctionSpace space() override;
    uint32_t base_seed = 0;

private:
    void create_buckets();
    void estimated_golomb_rice();
    void make_minimal();
    uint32_t assign_bucket(const std::string &key);
    void assign_classes();
    std::vector<uint32_t> generate_hash(size_t index, uint32_t base_seed);
    uint32_t extract_class_assignment(size_t index);
    void build_cuckoo_hash_table(const std::vector<size_t> &bucket);
    bool insert_into_hash_table(size_t index, uint32_t base_seed, std::vector<int> &hash_table);
    uint64_t key_class(const std::string &key);
    uint64_t key_n_hash(const std::string &key);
    uint64_t get_hash_index(std::string &key);

    std::vector<bool> class_assignments_;
    std::vector<uint16_t> keys_n_hashes_;
    double p1_;
    double p2_;
    double alpha_;
    double burr_epsilon_;
    int burr_layers_;
    uint32_t class_seed_;

    std::vector<int> rattle_counters_;
    std::vector<std::vector<bool>> hash_indexes_;
    std::map<std::string, std::vector<bool>> hash_index_map_;
    std::map<std::string, uint32_t> hash_index_map_raw_;

    std::vector<BuRR> ribbons;

    SimpleSelect perfect_rank_;
    EliasFanoDoubleEncodedData holes_ef_;
    std::vector<uint32_t> holes_;
    uint32_t n_holes_;
    std::vector<uint64_t> counts_;
    std::vector<uint64_t> taken_ranks_;

    Rank9 minimal_rank_;

    uint32_t bucket_seed_;
    uint32_t bucket_size_;
    std::vector<std::string> keys_;

    std::vector<std::vector<size_t>> buckets_;
    std::vector<uint32_t> bucket_sizes_;
    std::vector<uint32_t> hash_table_sizes_;

    std::vector<std::vector<std::string>> keys_classes_;
    std::vector<std::vector<uint64_t>> hash_indexes_per_class_;

    std::vector<uint32_t> bucket_seeds_;
    BucketSeedGr bucket_seeds_encoded;
    std::vector<uint32_t> bucket_prefixes_;
    EliasFanoDoubleEncodedData bucket_prefixes_ef_;
};


#endif