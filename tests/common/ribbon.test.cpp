#include <vector>
#include <gtest/gtest.h>
#include <iostream>
#include "common/ribbon.h"
#include "common/utils.h"
#include "common/murmurhash.h"

TEST(BasicRibbonTest, SingleBitTest) {
    std::vector<std::string> keys = generate_random_keys(10000);
    std::vector<uint64_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 2;
    }

    BasicRibbon basic_ribbon(keys, values, 1, 0.25);

    for (int i = 0; i < keys.size(); i++) {
        uint64_t query_val = basic_ribbon.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<1>(query_val) << " original value: " << std::bitset<1>(values[i]));
        EXPECT_EQ(query_val, values[i]);
    }
}

TEST(BasicRibbonTest, TwoBitTest) {
    std::vector<std::string> keys = generate_random_keys(10000);
    std::vector<uint64_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 4;
    }

    BasicRibbon basic_ribbon(keys, values, 2, 0.25);

    for (int i = 0; i < keys.size(); i++) {
        uint64_t query_val = basic_ribbon.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<2>(query_val) << " original value: " << std::bitset<2>(values[i]));
        EXPECT_EQ(query_val, values[i]);
    }
}

TEST(BasicRibbonTest, ThreeBitTest) {
    std::vector<std::string> keys = generate_random_keys(10000);
    std::vector<uint64_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 8;
    }

    BasicRibbon basic_ribbon(keys, values, 3, 0.25);

    for (int i = 0; i < keys.size(); i++) {
        uint64_t query_val = basic_ribbon.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<3>(query_val) << " original value: " << std::bitset<3>(values[i]));
        EXPECT_EQ(query_val, values[i]);
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}