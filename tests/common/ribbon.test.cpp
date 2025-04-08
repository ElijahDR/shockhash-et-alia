#include <vector>
#include <gtest/gtest.h>
#include <iostream>
#include "common/ribbon.h"
#include "common/utils.h"
#include "common/murmurhash.h"

TEST(BasicRibbonTest, SingleBitTest) {
    std::vector<std::string> keys = generate_random_keys(10000);
    std::vector<uint8_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 2;
    }

    BasicRibbon basic_ribbon(keys.size(), 64, 1, 0.25);
    basic_ribbon.build(keys, values);

    for (int i = 0; i < keys.size(); i++) {
        uint8_t query_val = basic_ribbon.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<8>(query_val) << " original value: " << std::bitset<8>(values[i]));
        EXPECT_EQ(query_val, values[i]);
    }
}

TEST(BasicRibbonTest, TwoBitTest) {
    std::vector<std::string> keys = generate_random_keys(10000);
    std::vector<uint8_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 4;
    }

    BasicRibbon basic_ribbon(keys.size(), 64, 2, 0.25);
    basic_ribbon.build(keys, values);

    for (int i = 0; i < keys.size(); i++) {
        uint8_t query_val = basic_ribbon.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<8>(query_val) << " original value: " << std::bitset<8>(values[i]));
        EXPECT_EQ(query_val, values[i]);
    }
}

TEST(BasicRibbonTest, ThreeBitTest) {
    std::vector<std::string> keys = generate_random_keys(10000);
    std::vector<uint8_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 8;
    }

    BasicRibbon basic_ribbon(keys.size(), 64, 3, 0.25);
    basic_ribbon.build(keys, values);

    for (int i = 0; i < keys.size(); i++) {
        uint8_t query_val = basic_ribbon.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<8>(query_val) << " original value: " << std::bitset<8>(values[i]));
        EXPECT_EQ(query_val, values[i]);
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}