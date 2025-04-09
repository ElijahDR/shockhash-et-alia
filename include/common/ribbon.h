#ifndef RIBBON_H
#define RIBBON_H

#include <cmath>
#include <string>
#include <vector>
#include <bitset>

class BasicRibbon {
public:
    BasicRibbon() = default;
    BasicRibbon(uint64_t n, uint64_t w, uint64_t r, double epsilon);
    BasicRibbon(std::vector<std::string> &keys, std::vector<std::uint64_t> &values, uint64_t r, double epsilon, uint64_t w = 64);

    void build(std::vector<std::string> &keys, std::vector<std::uint8_t> values);
    // uint8_t query(std::string &key);
    uint64_t query(const std::string &key);
    std::vector<uint64_t> Z;
    std::vector<uint64_t> compact_Z;
private:
    bool insert(std::string &key, std::uint8_t value, uint32_t seed);
    bool solve();
    void make_compact_z();

    uint64_t n;
    uint64_t w;
    uint64_t r;
    double e;
    uint64_t m;

    uint32_t seed_;

    std::vector<uint64_t> table;
    std::vector<uint8_t> b;
};

#endif