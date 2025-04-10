#include <vector>
#include <gtest/gtest.h>
#include "common/elias_fano.h"

TEST(EliasFanoTest, Encoding) {
    std::vector<uint32_t> data = {2,3,5,7,11,13,24};
    int n = data.size();
    EliasFanoEncodedData encoded = elias_fano_encode(data);
    DEBUG_LOG(encoded.upper);
    DEBUG_LOG(encoded.lower);
    DEBUG_LOG(encoded.m);
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded, n);
    EXPECT_EQ(data, decoded_data);
    EXPECT_EQ(1, 0);
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
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded, n);
    EXPECT_EQ(data, decoded_data);
    EXPECT_EQ(1, 0);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}