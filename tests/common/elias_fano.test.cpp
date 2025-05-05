#include <vector>
#include <gtest/gtest.h>
#include "common/elias_fano.h"

TEST(EliasFanoTest, EliasFanoClass_Test ){
    std::vector<uint32_t> data = {2,3,5,7,11,13,24};
    EliasFano ef(data);
    for (int i = 0; i < data.size(); i++) {
        uint32_t val = ef.get(i);
        EXPECT_EQ(data[i], val);
    }
}

TEST(EliasFanoTest, DoubleEncoding) {
    std::vector<uint32_t> data = {2,3,5,7,11,13,24};
    int n = data.size();
    EliasFanoDoubleEncodedData encoded = elias_fano_double_encode(data);
    std::vector<uint32_t> decoded_data = elias_fano_double_decode(encoded);
    EXPECT_EQ(data, decoded_data);
}

TEST(EliasFanoTest, Encoding) {
    std::vector<uint32_t> data = {2,3,5,7,11,13,24};
    int n = data.size();
    EliasFanoEncodedData encoded = elias_fano_encode(data);
    DEBUG_LOG(encoded.upper);
    DEBUG_LOG(encoded.lower);
    DEBUG_LOG(encoded.m);
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded);
    EXPECT_EQ(data, decoded_data);
}

TEST(EliasFanoTest, EncodingBig) {
    std::vector<uint32_t> data = {};
    for (int i = 0; i < 10; i++) {
        data.push_back(1234 + i);
    }
    int n = data.size();
    EliasFanoEncodedData encoded = elias_fano_encode(data);
    DEBUG_LOG(encoded.upper);
    DEBUG_LOG(encoded.upper.size());
    DEBUG_LOG(encoded.lower);
    DEBUG_LOG(encoded.lower.size());
    DEBUG_LOG(encoded.m);
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded);
    EXPECT_EQ(data, decoded_data);
}


TEST(EliasFanoTest, EncodingTwo) {
    std::vector<uint32_t> data = {1, 1000};
    int n = data.size();
    EliasFanoEncodedData encoded = elias_fano_encode(data);
    DEBUG_LOG(encoded.upper);
    DEBUG_LOG(encoded.upper.size());
    DEBUG_LOG(encoded.lower);
    DEBUG_LOG(encoded.lower.size());
    DEBUG_LOG(encoded.m);
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded);
    EXPECT_EQ(data, decoded_data);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}