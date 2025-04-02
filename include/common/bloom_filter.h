#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include "common/utils.h"
#include <vector>

const uint64_t L8 = 0x0101010101010101;

// https://vigna.di.unimi.it/ftp/papers/Broadword.pdf
class SimpleSelect {
public:
    SimpleSelect(std::vector<uint64_t> &data, uint64_t L, uint64_t M);
    SimpleSelect(std::vector<bool> &data, uint64_t L, uint64_t M);

    uint64_t select(uint64_t r);

    uint64_t bit_search(uint64_t curr_pos, uint64_t r);
    
private:
    void build_inventories();

    std::vector<uint64_t> data_;
    std::vector<uint64_t> primary_inventory;
    std::vector<uint16_t> secondary_inventory;
    
    uint64_t L_;
    uint64_t M_;
    uint64_t L_spacing_;
    uint64_t M_spacing_;
};

void generate_rank_counts(std::vector<uint64_t> &data, std::vector<uint64_t> &counts);
inline uint64_t popcount(uint64_t x);
uint64_t rank9(std::vector<uint64_t> &data, std::vector<uint64_t> &counts, size_t p);


#endif