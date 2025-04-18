#include "common/elias_fano.h"

EliasFanoEncodedData elias_fano_encode(std::vector<uint32_t> data) {
    const int n = data.size();
    const uint32_t m = *data.rbegin() + 1;

    int l = std::floor(std::log2(m/n));
    int r = std::ceil(std::log2(m)) - l; 

    std::vector<bool> lower;
    std::vector<bool> upper;
    std::vector<uint32_t> upper_counts(std::pow(2, r), 0);
    int prev = 0;
    for (auto x : data) {
        for (int i = l-1; i >= 0; i--) {
            lower.push_back(((1 << i) & x) != 0);
        }

        int a = x >> l;
        while (prev < a) {
            upper.push_back(0);
            prev++;
        }
        upper.push_back(1);
    }

    // upper.insert(upper.end(), lower.begin(), lower.end());

    return EliasFanoEncodedData{upper, lower, m};
}

std::vector<uint32_t> elias_fano_decode(EliasFanoEncodedData &data, uint32_t n) {
    int l = std::floor(std::log2(data.m/n));
    std::vector<uint32_t> data_decoded;

    uint32_t upper = 0, count = 0;
    for (auto upper_bit : data.upper) {
        if (upper_bit) {
            uint32_t lower = 0;
            for (int j = 0; j < l; j++) {
                lower |= (uint32_t)(data.lower[count * l + j]) << (l - j - 1);
            }
            data_decoded.push_back((upper << l) | lower);
            count += 1;
        } else {
            upper++;
        }
    }

    return data_decoded;
}

uint32_t elias_fano_space(EliasFanoEncodedData &data) {
    // return (sizeof(data.lower) + sizeof(data.upper) + sizeof(data.m)) * 8 + data.upper.size() + data.lower.size();
    return data.upper.size() + data.lower.size();
}