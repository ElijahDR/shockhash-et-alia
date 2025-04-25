#include "common/elias_fano.h"

EliasFanoEncodedData elias_fano_encode(std::vector<uint32_t> &data) {
    const int n = data.size();
    const uint32_t m = data.back();

    int q = std::ceil((double)m / n);
    int l = std::ceil(std::log2(q));

    std::vector<bool> lower;
    std::vector<bool> upper;

    int prev = -1;
    for (auto x : data) {
        // Encode lower bits
        for (int i = 0; i < l; i++) {
            lower.push_back((x >> i) & 1);
        }

        // Encode upper bits
        int a = x >> l;
        while (prev < a - 1) {
            upper.push_back(0);
            prev++;
        }
        upper.push_back(1);
        prev = a;
    }

    return EliasFanoEncodedData{upper, lower, m};
}

std::vector<uint32_t> elias_fano_decode(EliasFanoEncodedData &data, uint32_t n) {
    int l = std::ceil(std::log2(std::ceil((double)data.m / n)));
    std::vector<uint32_t> data_decoded;

    uint32_t upper = 0, count = 0;
    for (auto upper_bit : data.upper) {
        if (upper_bit) {
            uint32_t lower = 0;
            for (int j = 0; j < l; j++) {
                lower |= (uint32_t)(data.lower[count * l + j]) << (l - j - 1);
            }
            data_decoded.push_back((upper << l) | lower);
            count++;
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