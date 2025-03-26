#ifndef RECSPLIT_H
#define RECSPLIT_H

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include "algos/hash_function.h"

struct EncodedSplittingTree { 
    std::vector<bool> fixed;
    std::vector<bool> unary;
};

struct FanoutData {
    uint32_t size;
    uint32_t fanout;
    std::vector<uint32_t> part_sizes;
};

struct SubtreeData { 
    uint8_t parameter;
    uint16_t nodes;
    uint16_t fixed_code_length;
    uint16_t unary_code_length;
};


class RecSplit : public HashFunction {
public:
    RecSplit(uint32_t bucket_size, uint32_t leaf_size);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;

    std::string name() override { return "RecSplit"; };

    void space();

private:

    void split(const std::vector<std::string> &keys);
    
    void create_buckets();

    uint32_t find_bijection_random(const std::vector<std::string> &keys);

    void append_to_splitting_tree(const uint32_t &data, const uint32_t size);

    uint32_t bucket_size_;
    uint32_t bucket_count_;
    const uint32_t leaf_size_;

    std::vector<uint32_t> golomb_rice_parameters_leaf_;

    std::vector<uint32_t> part_sizes_;

    EncodedSplittingTree splitting_tree_;
    std::vector<uint32_t> splitting_tree_raw_;

    std::vector<std::string> keys_;

    std::vector<std::vector<std::string>> buckets_;
    std::vector<uint32_t> bucket_sizes_;
    std::vector<uint32_t> bucket_prefixes_;
    std::vector<uint32_t> bucket_offsets_;

    std::vector<uint32_t> bucket_node_prefixes_;
    std::vector<uint32_t> bucket_unary_prefixes_;
    std::vector<uint32_t> bucket_fixed_prefixes_;

    std::vector<std::vector<SubtreeData>> grp_table_;
};

uint32_t find_bijection(const std::vector<std::string> &keys);
uint32_t find_bijection(const std::vector<std::string> &keys);
FanoutData calculate_fanout(uint32_t size, uint32_t leaf_size);
std::vector<std::vector<SubtreeData>> generate_all_grp();
uint32_t assign_bucket(const std::string &key, uint32_t bucket_count);
uint32_t map_key_to_split(const std::string &key, const uint32_t &seed, const FanoutData &fanout_data);
uint32_t find_splitting(const std::vector<std::string> &keys, const FanoutData &fanout_data);

#endif