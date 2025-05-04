#ifndef ELIAS_FANO_H
#define ELIAS_FANO_H

#include "common/utils.h"

struct EliasFanoEncodedData {
    std::vector<bool> upper;
    std::vector<bool> lower;
    uint32_t m;
    int l;
};
struct EliasFanoDoubleEncodedData {
    EliasFanoEncodedData result;
    uint32_t delta;
};

// https://www.antoniomallia.it/sorted-integers-compression-with-elias-fano-encoding.html
EliasFanoEncodedData elias_fano_encode(std::vector<uint32_t> &data);
std::vector<uint32_t> elias_fano_decode(const EliasFanoEncodedData &data);
EliasFanoDoubleEncodedData elias_fano_double_encode(std::vector<uint32_t> data);
std::vector<uint32_t> elias_fano_double_decode(const EliasFanoDoubleEncodedData &data);
uint32_t elias_fano_space(EliasFanoEncodedData &data);
uint32_t elias_fano_space(EliasFanoDoubleEncodedData &data);

class EliasFanoWithEmpty {
public:
    EliasFanoWithEmpty();
private:
};

#endif