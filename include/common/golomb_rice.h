#ifndef GOLOMB_RICE_H
#define GOLOMB_RICE_H

#include <vector>
#include <cstdint>

struct GolombEncodedData {
    std::vector<bool> fixed;
    std::vector<bool> unary;
};

GolombEncodedData golomb_rice_encode(uint32_t value, uint32_t r);
uint32_t golomb_rice_decode(GolombEncodedData data, uint32_t r);
uint32_t compute_golomb_rice_parameter(float p);
uint32_t compute_grp_bijection(uint32_t m);
uint32_t compute_grp_buckets(std::vector<uint32_t> sizes);

#endif