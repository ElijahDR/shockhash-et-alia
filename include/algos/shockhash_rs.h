#ifndef SHOCKHASHRS_H
#define SHOCKHASHRS_H

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "common/utils.h"
#include "common/broadword.h"
#include "common/elias_fano.h"
#include "algos/hash_function.h"
#include "algos/shockhash.h"
#include "algos/recsplit.h"



class ShockHashRS : public HashFunction {
public:
    ShockHashRS(uint32_t bucket_size, uint32_t leaf_size, uint32_t bucket_seed=42);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;

    std::string name() override { return "ShockHash-RS"; };

    HashFunctionSpace space() override;
    uint64_t time_bijection = 0;
    uint64_t time_splitting = 0;
    uint64_t time_ribbon = 0;

private:

    void split(const std::vector<std::string> &keys);
    
    void create_buckets();
    uint32_t assign_bucket(const std::string &key, uint32_t bucket_count);

    uint32_t find_bijection_random(const std::vector<std::string> &keys);

    void append_to_splitting_tree(const uint32_t &data, const uint32_t size);


    uint32_t bucket_size_;
    uint32_t bucket_count_;
    uint32_t bucket_seed_;
    const uint32_t leaf_size_;

    std::vector<uint32_t> golomb_rice_parameters_leaf_;

    std::vector<uint32_t> part_sizes_;

    EncodedSplittingTree splitting_tree_; 
    std::vector<uint32_t> splitting_tree_raw_;
    EncodedSplittingTreeSelect splitting_tree_select_;

    std::vector<std::string> keys_;

    std::vector<std::vector<std::string>> buckets_;
    std::vector<uint32_t> bucket_sizes_;
    std::vector<uint32_t> bucket_prefixes_;
    std::vector<uint32_t> bucket_offsets_;

    std::vector<uint32_t> bucket_node_prefixes_;
    std::vector<uint32_t> bucket_unary_prefixes_;
    std::vector<uint32_t> bucket_fixed_prefixes_;

    EliasFanoEncodedData bucket_node_prefixes_ef_;
    EliasFanoEncodedData bucket_unary_prefixes_ef_;
    EliasFanoEncodedData bucket_fixed_prefixes_ef_;
    EliasFanoDoubleEncodedData bucket_node_prefixes_ef_double;

    EliasFano fixed_prefixes;
    EliasFano unary_prefixes;
    EliasFano node_prefixes;

    std::vector<std::vector<SubtreeData>> grp_table_;

    std::vector<uint64_t> hash_indexes;
    std::vector<std::string> key_as_computed;
    BuRR hash_choices;

    uint32_t find_splitting(const std::vector<std::string> &keys, const FanoutData &fanout_data);
    uint32_t find_bijection(const std::vector<std::string> &keys);
};

FanoutData calculate_fanout_shockhash(uint32_t size, uint32_t leaf_size);
std::vector<std::vector<SubtreeData>> generate_all_grp_shockhash();
inline uint32_t map_key_to_split_new(const std::string &key, const uint32_t &seed, const std::vector<int> &cumulative_sums);

#endif