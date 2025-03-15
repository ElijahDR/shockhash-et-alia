#include "common/golomb_rice.h"
#include "common/utils.h"
#include <cmath>
#include <iostream>


// https://michaeldipperstein.github.io/rice.html
GolombEncodedData golomb_rice_encode(uint32_t value, uint32_t r)
{
    uint32_t quotient = value >> r;
    uint32_t remainder = value & ((1 << r) - 1);

    std::vector<bool> fixed;
    std::vector<bool> unary;

    for (int i = 0; i < quotient; i++)
    {
        unary.push_back(false);
    }
    unary.push_back(true);

    for (int i = r - 1; i >= 0; i--)
    {
        fixed.push_back((remainder >> i) & 1);
    }

    return GolombEncodedData{fixed, unary};
}

uint32_t golomb_rice_decode(GolombEncodedData data, uint32_t r)
{
    uint32_t quotient = data.unary.size() - 1;

    int pos = 0;
    uint32_t remainder = 0;
    for (int i = 0; i < r; ++i)
    {
        remainder = (remainder << 1) | data.fixed[pos++];
    }
    return (quotient << r) | remainder;
}

uint32_t compute_golomb_rice_parameter(float p) {
    double log_phi = std::log(GOLDEN_RATIO);
    double log_1_minus_p = std::log(1.0 - p);

    double r_p = std::ceil(std::log2(-log_phi / log_1_minus_p));

    uint32_t param = std::max(0, (int)(r_p));
    DEBUG_LOG("Golomb Rice Parameter Calculated: " << param);
    return param;
}

uint32_t compute_grp_bijection(uint32_t m) {
    double trials = pow(exp(1), m) / sqrt(2 * PI * m);
    double p = 1 / trials;
    DEBUG_LOG("GRP BIJECTION --- Size: " << m << " Est. Trials: " << trials << " Prob: " << p);

    return compute_golomb_rice_parameter(p);
}

uint32_t compute_grp_buckets(std::vector<uint32_t> sizes) {
    uint32_t k = 1;
    uint32_t m = 0;
    for (auto size : sizes) {
        k *= size;
        m += size;
    }

    uint32_t s = sizes.size();
    double trials = std::sqrt((pow((2*PI), s-1) * k) / m);
    double p = 1 / trials;

    DEBUG_LOG("GRP BUCKETS --- Size: " << m << " Est. Trials: " << trials << " Prob: " << p);
    DEBUG_VECTOR_LOG("GRP BUCKETS --- Sizes: ", sizes);
    return compute_golomb_rice_parameter(p);
}

std::vector<std::vector<uint32_t>> generate_all_grp() {
    // Generate up to bucket_size 3000 and leaf size up to 24?
    const uint32_t max_bucket_size = 3000;
    const uint32_t max_leaf_size = 23;
    std::vector<std::vector<int>> grp_table(max_bucket_size, std::vector<int>(max_leaf_size, 0));
    for (size_t bucket_size = 0; bucket_size < max_bucket_size; bucket_size++) {
        for (size_t leaf_size = 0; leaf_size < max_leaf_size; leaf_size++) {

        }
    }
}