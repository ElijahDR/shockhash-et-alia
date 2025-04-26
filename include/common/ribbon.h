#ifndef RIBBON_H
#define RIBBON_H

#include <cmath>
#include <string>
#include <vector>
#include <bitset>
#include <cstdint>
#include <memory>
#include <algorithm>

struct BuRRSpace {
    int total_bits;
    std::vector<std::pair<std::string, int>> space_usage;
    int total_Z;
    int total_metadata;
};

class BasicRibbon {
public:
    BasicRibbon() = default;
    BasicRibbon(uint64_t n, uint64_t w, uint64_t r, double epsilon);
    BasicRibbon(std::vector<std::string> &keys, std::vector<std::uint64_t> &values, uint64_t r, double epsilon, uint64_t w = 64);
    ~BasicRibbon() = default;  
    void build(std::vector<std::string> &keys, std::vector<std::uint8_t> values);
    // uint8_t query(std::string &key);
    uint64_t query(const std::string &key);
    uint32_t space();
    std::vector<uint64_t> Z;
    std::vector<uint64_t> compact_Z;
    bool solve();
    void make_compact_z();
private:
    bool insert(std::string &key, std::uint8_t value, uint32_t seed);

    uint64_t n;
    uint64_t w;
    uint64_t r;
    double e;
    uint64_t m;

    uint32_t seed_;

    std::vector<uint64_t> table;
    std::vector<uint8_t> b;
};

class BuRR {
public:
    BuRR() = default;
    BuRR(uint64_t n, uint64_t w, uint64_t r, double epsilon);
    BuRR(std::vector<std::string> &keys, std::vector<std::uint64_t> &values, 
        uint64_t r, double epsilon, uint64_t bucket_size, int num_layers=4, uint64_t w = 64);

    void build(std::vector<std::string> &keys, std::vector<std::uint8_t> values);
    // uint8_t query(std::string &key);
    uint64_t query(const std::string &key);
    BuRRSpace space();
    std::vector<uint64_t> Z;
    std::vector<uint64_t> compact_Z;
private:
    bool insert(std::string &key, std::uint8_t value, uint32_t seed);
    bool solve();
    void make_compact_z();
    void reset(double epsilon);

    std::vector<uint64_t> threshold_values;
    std::vector<uint16_t> metadata;
    std::unique_ptr<BuRR> fallback_burr_ptr;
    std::unique_ptr<BasicRibbon> fallback_ribbon_ptr;
    bool used_fallback = false;
    std::vector<std::pair<uint64_t, uint64_t>> previous_insertions;
    
    uint64_t n;
    uint64_t w;
    uint64_t r;
    double e;
    uint64_t m;
    int num_layers_;
    int bucket_size_;


    uint32_t seed_;

    std::vector<uint64_t> table;
    std::vector<uint8_t> b;
};

#endif