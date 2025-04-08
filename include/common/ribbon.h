#ifndef RIBBON_H
#define RIBBON_H

#include <cmath>
#include <string>
#include <vector>
#include <bitset>

class BasicRibbon {
public:
    BasicRibbon(uint64_t n, uint64_t w, uint64_t r, double epsilon);

    void build(std::vector<std::string> &keys, std::vector<std::uint8_t> values);
    uint8_t query(std::string &key);
    std::vector<uint8_t> Z;
    std::vector<uint64_t> compact_Z;
private:
    bool insert(std::string &key, std::uint8_t value, uint32_t seed);
    bool solve();

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