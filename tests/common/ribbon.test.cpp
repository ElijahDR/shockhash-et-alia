#include <vector>
#include <gtest/gtest.h>
#include <iostream>
#include "common/ribbon.h"
#include "common/utils.h"
#include "common/murmurhash.h"

TEST(BasicRibbonTest, SingleBitTest) {
    std::vector<std::string> keys = generate_random_keys(64);
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
    std::vector<std::string> keys = generate_random_keys(1000);
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

TEST(BurrTest, OneBitTest) {
    int n = 1000000;
    std::vector<std::string> keys = generate_random_keys(n);
    std::vector<uint64_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 2;
    }

    int w = 64; 
    int b = 128;
    double e = -(double)0;
    BuRR burr(keys, values, 1, e, b, 4, w);

    ProgressBar pbar(100);
    for (int i = 0; i < keys.size(); i++) {
        uint64_t query_val = burr.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<3>(query_val) << " original value: " << std::bitset<3>(values[i]));
        EXPECT_EQ(query_val, values[i]);
        if (i % (n / 100) == 0) {
            pbar.update();
        }
    }

    DEBUG_LOG("Finished Querying");
    std::cout << burr.space() << std::endl;;
}


TEST(BurrTest, TwoBitTest) {
    int n = 1000000;
    std::vector<std::string> keys = generate_random_keys(n);
    std::vector<uint64_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 4;
    }

    int w = 64; 
    int b = 64;
    double e = (double)-4 / 64;
    BuRR burr(keys, values, 2, e, b, 5, w);

    ProgressBar pbar(100);
    for (int i = 0; i < keys.size(); i++) {
        uint64_t query_val = burr.query(keys[i]);
        DEBUG_LOG("Query Value: " << std::bitset<3>(query_val) << " original value: " << std::bitset<3>(values[i]));
        EXPECT_EQ(query_val, values[i]);
        if (i % (n / 100) == 0) {
            pbar.update();
        }
    }

    DEBUG_LOG("Finished Querying");
    std::cout << burr.space() << std::endl;;
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}