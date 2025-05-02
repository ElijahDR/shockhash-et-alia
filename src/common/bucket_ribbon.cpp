#include "common/bucket_ribbon.h"
#include "common/murmurhash.h"

BucketedRibbon::BucketedRibbon(std::vector<std::string> &keys, std::vector<uint64_t> &values, int r, int bucket_size, double e) {
    bucket_count_ = std::ceil((double)keys.size() / bucket_size);
    buckets_.resize(bucket_count_);
    std::vector<int> bucket_sizes(bucket_count_, 0);
    DEBUG_LOG("Bucket Count: " << bucket_count_);
    for (int i = 0; i < keys.size(); i++) {
        buckets_[assign_bucket(keys[i])].push_back(i);
        bucket_sizes[assign_bucket(keys[i])]++;
    }

    std::cout << "Bucket Sizes: " << bucket_sizes << std::endl;

    for (auto bucket : buckets_) {
        std::vector<std::string> bucket_keys;
        std::vector<uint64_t> bucket_values;
        for (auto index : bucket) {
            bucket_keys.push_back(keys[index]);
            bucket_values.push_back(values[index]);
        }
        
        // BuRR burr = ;
        ribbons.push_back(BuRR(bucket_keys, bucket_values, r, e, 64, 1));
    }
}

uint32_t BucketedRibbon::assign_bucket(std::string &key) {
    uint128_t hash = murmur128(key, bucket_seed_) >> 64;
    uint32_t bucket = floor((hash * (uint128_t)bucket_count_) >> 64);
    return bucket;
}

uint64_t BucketedRibbon::query(std::string &key) {
    uint32_t bucket = assign_bucket(key);
    return ribbons[bucket].query(key);
}

uint64_t BucketedRibbon::space() {
    int total = 0;
    std::vector<int> spaces;
    for (int i = 0; i < ribbons.size(); i++) {
        auto space = ribbons[i].space();
        std::cout << "Space: " << space;
    }
    std::cout << "Spaces: " << spaces << std::endl;
    std::cout << "Total Bits: " << total << std::endl;

    return total;
}