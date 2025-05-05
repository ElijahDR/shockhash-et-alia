#include "common/elias_fano.h"

EliasFano::EliasFano(std::vector<uint32_t> &data) {
    DEBUG_LOG("Elias Fano Encoding: " << data);
    EliasFanoDoubleEncodedData ef = elias_fano_double_encode(data);
    upper.build(ef.result.upper, 128, 8);
    lower = ef.result.lower;
    delta = ef.delta;
    m = ef.result.m;
    l = ef.result.l;
}

uint32_t EliasFano::get(int index) {
    std::vector<bool> lower_part(lower.begin() + index * l, lower.begin() + ((index + 1) * l));
    uint32_t quotient = upper.select(index+1) - index;
    DEBUG_LOG("Quotient: " << quotient);

    uint32_t lower_val = 0;
    for (int j = 0; j < l; j++) {
        lower_val = (lower_val << 1) | lower_part[j];
    }

    return ((quotient << l) | lower_val) + delta * index;
}

EliasFanoEncodedData elias_fano_encode(std::vector<uint32_t> &data) {
    DEBUG_LOG("Elias Fano Encoding: " << data);
    int n = data.size();
    uint32_t m = data.back();

    uint32_t q = std::ceil((double)m / n);
    int upper = std::ceil(std::log2((double)n));
    DEBUG_LOG("Upper: " << upper);
    int lower = std::ceil(std::log2(m == 0 ? 1 : m)) - upper;
    DEBUG_LOG("Lower: " << lower);

    EliasFanoEncodedData result;
    result.m = m;
    result.l = lower;

    int last_quotient = 0;
    for (int i = 0; i < data.size(); i++) {
        uint32_t x = data[i];
        int quotient = x >> lower;
        int delta = quotient - last_quotient;
        result.upper.insert(result.upper.end(), delta, 0);
        result.upper.push_back(1);
        last_quotient = quotient;

        int lower_value = x & ((1 << lower) - 1);
        for (int j = lower - 1; j >= 0; j--) {
            result.lower.push_back((lower_value >> j) & 1);
        }
    }

    DEBUG_LOG("Lower: " << result.lower);
    DEBUG_LOG("Upper: " << result.upper);
    return result;
}

std::vector<uint32_t> elias_fano_decode(const EliasFanoEncodedData &encoded) {
    std::vector<uint32_t> result;

    int lower = encoded.l;
    size_t lower_index = 0;

    int current_quotient = 0;
    for (bool bit : encoded.upper) {
        if (bit) {
            if (lower_index + lower > encoded.lower.size()) break;
            uint32_t lower_val = 0;
            for (int j = 0; j < lower; j++) {
                lower_val = (lower_val << 1) | encoded.lower[lower_index++];
            }

            DEBUG_LOG("Quotient: " << current_quotient);
            result.push_back((current_quotient << lower) | lower_val);
        } else {
            current_quotient++;
        }
    }

    return result;
}

EliasFanoDoubleEncodedData elias_fano_double_encode(std::vector<uint32_t> data) {
    uint32_t min_delta = data.back();
    for (int i = 0; i < data.size() - 1; i++){
        if (data[i+1] - data[i] < min_delta) {
            min_delta = data[i+1] - data[i];
        }
    }

    for (int i = 0; i < data.size(); i++) {
        data[i] -= i * min_delta;
    }
    
    EliasFanoDoubleEncodedData result;
    EliasFanoEncodedData single_result = elias_fano_encode(data);
    result.delta = min_delta;
    result.result = single_result;

    return result;
}

std::vector<uint32_t> elias_fano_double_decode(const EliasFanoDoubleEncodedData &encoded) {
    std::vector<uint32_t> result = elias_fano_decode(encoded.result);

    for (int i = 0; i < result.size(); i++) {
        result[i] += encoded.delta * i;
    }

    return result;
}

uint32_t elias_fano_space(EliasFanoEncodedData &data) {
    // return (sizeof(data.lower) + sizeof(data.upper) + sizeof(data.m)) * 8 + data.upper.size() + data.lower.size();
    return data.upper.size() + data.lower.size();
}
uint32_t elias_fano_space(EliasFanoDoubleEncodedData &data) {
    // return (sizeof(data.lower) + sizeof(data.upper) + sizeof(data.m)) * 8 + data.upper.size() + data.lower.size();
    return data.result.upper.size() + data.result.lower.size() + 32;
}