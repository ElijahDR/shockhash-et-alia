#ifndef RECSPLIT_H
#define RECSPLIT_H

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include "common/utils.h"
#include "common/broadword.h"
#include "common/elias_fano.h"
#include "algos/hash_function.h"

struct EncodedSplittingTree { 
    std::vector<bool> fixed;
    std::vector<bool> unary;
};

struct EncodedSplittingTreeSelect { 
    std::vector<bool> fixed;
    SimpleSelect unary;
};

struct FanoutData {
    uint32_t size;
    uint32_t fanout;
    std::vector<uint16_t> part_sizes;
};

struct SubtreeData { 
    uint8_t parameter;
    uint16_t nodes;
    uint16_t fixed_code_length;
    uint16_t unary_code_length;
};

// Struct printings
inline std::ostream& operator<<(std::ostream& os, const SubtreeData& data) {
    os << "GolombRiceData {\n"
       << "  parameter: " << static_cast<int>(data.parameter) << "\n"
       << "  nodes: " << data.nodes << "\n"
       << "  fixed_code_length: " << data.fixed_code_length << "\n"
       << "  unary_code_length: " << data.unary_code_length << "\n"
       << "}";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const FanoutData &data) {
    os << "FanoutData {\n"
       << "  size: " << data.size << "\n"
       << "  fanout: " << data.fanout << "\n"
       << " part_sizes: " << data.part_sizes << "\n"
       << "}";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const EncodedSplittingTree &data) {
    os << "EncodedSplittingTree {\n"
       << "  Fixed: " << data.fixed << "\n"
       << "  Unary: " << data.unary << "\n";
    os << "}";
    return os;
}


class RecSplit : public HashFunction {
public:
    RecSplit(uint32_t bucket_size, uint32_t leaf_size, uint32_t bucket_seed=42);

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;

    std::string name() override { return "RecSplit"; };

    HashFunctionSpace space() override;
    uint64_t time_bijection = 0;
    uint64_t time_splitting = 0;

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

    std::vector<std::vector<SubtreeData>> grp_table_;

    uint32_t find_splitting(const std::vector<std::string> &keys, const FanoutData &fanout_data);
    uint32_t find_bijection(const std::vector<std::string> &keys);
};

FanoutData calculate_fanout(uint32_t size, uint32_t leaf_size);
std::vector<std::vector<SubtreeData>> generate_all_grp();
uint32_t map_key_to_split(const std::string &key, const uint32_t &seed, const FanoutData &fanout_data);

#endif