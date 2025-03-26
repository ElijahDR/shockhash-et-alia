#include <vector>
#include <gtest/gtest.h>
#include "common/elias_fano.h"

TEST(EliasFanoTest, Encoding) {
    std::vector<uint32_t> data = {2,3,5,7,11,13,24};
    int n = data.size();
    EliasFanoEncodedData encoded = elias_fano_encode(data);
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded, n);
    EXPECT_EQ(data, decoded_data);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}