#ifndef RECSPLIT_H
#define RECSPLIT_H

#include <cstdint>
#include <vector>
#include <string>

struct EncodedSplittingTree { 
    std::vector<bool> fixed;
    std::vector<bool> unary;
};

class RecSplit {
public:
    RecSplit(uint32_t bucket_size, uint32_t leaf_size);

    void build(const std::vector<std::string> &keys);

    uint32_t hash(std::string &key);

private:
    void split(const std::vector<std::string> &keys, uint32_t depth);
    
    void create_buckets();
    uint32_t assign_bucket(const std::string &key);

    uint32_t find_bijection(const std::vector<std::string> &keys);
    uint32_t find_bijection_random(const std::vector<std::string> &keys);

    void append_to_splitting_tree(const uint32_t &data, const uint32_t size);

    uint32_t bucket_size_;
    const uint32_t leaf_size_;
    uint32_t bucket_seed_;

    std::vector<uint32_t> golomb_rice_parameters_leaf_;

    // Vector of length 3 to define part sizes.
    std::vector<uint32_t> part_sizes_;

    EncodedSplittingTree splitting_tree_;

    std::vector<std::string> keys_;

    std::vector<std::vector<std::string>> buckets_;
    std::vector<uint32_t> bucket_sizes_;
    std::vector<uint32_t> bucket_prefixes_;
    std::vector<uint32_t> bucket_offsets;

};

#endif