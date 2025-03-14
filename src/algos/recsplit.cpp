#include "algos/recsplit.h"

RecSplit::RecSplit(uint32_t bucket_size, uint32_t leaf_size)
    : bucket_size_(bucket_size), leaf_size_(leaf_size)
{
    part_sizes_ = {
        (uint32_t)std::max(2, (int)ceil(0.35 * leaf_size + 0.5)),
        leaf_size_ >= 7 ? (uint32_t)ceil(0.21 * leaf_size + 0.9) : 2,
        2,
    };
}

void RecSplit::build(const std::vector<std::string> &keys)
{
    return;
}