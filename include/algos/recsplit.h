#ifndef RECSPLIT_H
#define RECSPLIT_H

#include <cstdint>
#include <vector>
#include <string>

class RecSplit
{
public:
    RecSplit(uint32_t bucket_size, uint32_t leaf_size);

    void build(const std::vector<std::string> &keys);

    uint32_t hash(std::string &key);

private:
    void split(const std::vector<std::string> &keys, uint8_t depth);

    uint32_t bucket_size_;
    uint32_t leaf_size_;

    // Vector of length 3 to define part sizes.
    std::vector<uint32_t> part_sizes_;

    std::vector<std::string> keys_;

    std::vector<std::vector<std::string>> bucket_assignments_;
    std::vector<uint32_t> bucket_sizes_;
};

#endif