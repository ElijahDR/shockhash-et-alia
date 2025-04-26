#ifndef TEST_H
#define TEST_H

#include <cmath>
#include <string>
#include <vector>
#include <bitset>
#include <cstdint>
#include <memory>
#include <algorithm>
#include "common/ribbon.h"
#include "common/murmurhash.h"

class BucketedRibbon {
public:
    BucketedRibbon(std::vector<std::string> &keys, std::vector<uint64_t> &values, int r, int bucket_size, double e=0.01);
    uint64_t query(std::string &key);
    uint64_t space();
private:
    std::vector<BuRR> ribbons;
    std::vector<std::vector<uint32_t>> buckets_;
    uint32_t bucket_seed_ = 0;
    double epsilon;
    int n_bits;
    int bucket_size_;
    uint32_t assign_bucket(std::string &key);
    uint32_t bucket_count_;
};

#endif