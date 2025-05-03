#include "common/elias_fano.h"

EliasFanoEncodedData elias_fano_encode(std::vector<uint32_t>& data) {
    int n = data.size();
    uint32_t u = data.back();

    uint32_t q = (u + n - 1) / n; 
    int l = (q == 0) ? 0 : (int)(std::log2(q)) + 1;

    EliasFanoEncodedData result;
    result.m = u;
    result.l = l;

    int current_upper = 0;
    for (int i = 0; i < data.size(); i++) {
        uint32_t x = data[i];
        int quotient = x >> l;
        int delta = quotient - current_upper;
        result.upper.insert(result.upper.end(), delta, 0);
        result.upper.push_back(1);
        current_upper = quotient;

        int lower_value = x & ((1 << l) - 1);
        for (int j = l - 1; j >= 0; --j) {
            result.lower.push_back((lower_value >> j) & 1);
        }
    }

    return result;
}

std::vector<uint32_t> elias_fano_decode(const EliasFanoEncodedData &encoded, uint32_t expected_n) {
    std::vector<uint32_t> result;

    int l = encoded.l;
    size_t lower_index = 0;

    int current_upper = 0;
    for (bool bit : encoded.upper) {
        if (bit) {
            if (lower_index + l > encoded.lower.size()) break;
            uint32_t lower_val = 0;
            for (int j = 0; j < l; j++) {
                lower_val = (lower_val << 1) | encoded.lower[lower_index++];
            }

            result.push_back((current_upper << l) | lower_val);
        } else {
            current_upper++;
        }
    }

    return result;
}

uint32_t elias_fano_space(EliasFanoEncodedData &data) {
    // return (sizeof(data.lower) + sizeof(data.upper) + sizeof(data.m)) * 8 + data.upper.size() + data.lower.size();
    return data.upper.size() + data.lower.size();
}