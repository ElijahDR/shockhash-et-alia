#include <vector>
#include <gtest/gtest.h>
#include <iostream>
#include "common/broadword.h"

// TEST(BroadwordTest_RankCounts, SingleEmpty) {
//     std::vector<uint64_t> data = {0, 0, 0, 0, 0, 0, 0, 0};
//     std::vector<uint64_t> counts;
//     generate_rank_counts(data, counts);
//     ASSERT_EQ(counts[0], 0);
//     ASSERT_EQ(counts[1], 0);
// }

// TEST(BroadwordTest_RankCounts, SingleFull) {
//     std::vector<uint64_t> data = {
//         0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
//         0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF
//     };
//     std::vector<uint64_t> counts;
//     generate_rank_counts(data, counts);
//     ASSERT_EQ(counts[0], 0);
//     uint64_t expected_second = 0;
//     for (uint64_t k = 1; k <= 7; ++k) {
//         expected_second |= (64 * k) << (9 * (k - 1));
//     }
//     std::bitset<9> ex_bin(expected_second);
//     std::cerr << "Total Count: " << ex_bin << std::endl;
//     ASSERT_EQ(counts[1], expected_second);
// }

// TEST(BroadwordTest_RankCounts, SingleMixed) {
//     std::vector<uint64_t> data = {
//         0x000000000000000F,
//         0x00000000000000F0,
//         0x0000000000000F00,
//         0x000000000000F000,
//         0x00000000000F0000,
//         0x0000000000F00000,
//         0x000000000F000000,
//         0x00000000F0000000
//     };
//     std::vector<uint64_t> counts;
//     generate_rank_counts(data, counts);
//     ASSERT_EQ(counts[0], 0);
//     uint64_t expected_second = 0;
//     for (uint64_t k = 1; k <= 7; ++k) {
//         expected_second |= (4 * k) << (9 * (k - 1));
//     }
//     ASSERT_EQ(counts[1], expected_second);
// }

// TEST(BroadwordTest_RankCounts, MultipleMixed) {
//     std::vector<uint64_t> data = {
//         1, 1, 1, 1, 1, 1, 1, 1,
//         // Block 1: 8 words of 2-bits each (total 16)
//         3, 3, 3, 3, 3, 3, 3, 3
//     };
//     std::vector<uint64_t> counts;
//     generate_rank_counts(data, counts);
//     ASSERT_EQ(counts[0], 0);
//     uint64_t expected_second_block0 = 0;
//     for (uint64_t k = 1; k <= 7; ++k) {
//         expected_second_block0 |= k << (9 * (k - 1));
//     }
//     ASSERT_EQ(counts[1], expected_second_block0);
//     uint64_t expected_second_block1 = 0;
//     for (uint64_t k = 1; k <= 7; ++k) {
//         expected_second_block1 |= (2 * k) << (9 * (k - 1));
//     }
//     ASSERT_EQ(counts[3], expected_second_block1);
//     ASSERT_EQ(counts[2], 8);
// }

// TEST(BroadwordTest_Rank9, MixedBitsInSingleWord) {
//     std::vector<uint64_t> data = {0x0F0F0F0F0F0F0F0F, 0xF0F0F0F0F0F0F0F0};
//     std::vector<uint64_t> counts;
//     generate_rank_counts(data, counts);
//     size_t p = 5;
//     uint64_t result = rank9(data, counts, p);
//     EXPECT_EQ(result, 4);
// }

// // // Test case: Boundary between words
// TEST(Rank9Test, BoundaryBetweenWords) {
//     std::vector<uint64_t> data = {0x0F0F0F0F0F0F0F0F, 0xF0F0F0F0F0F0F0F0};
//     std::vector<uint64_t> counts;
//     generate_rank_counts(data, counts);
//     size_t p = 64;
//     uint64_t result = rank9(data, counts, p);
//     EXPECT_EQ(result, 32);
// }

// // Test case: All zeros
// TEST_F(Rank9Test, AllZeros) {
//     data = {0, 0, 0}; // Bitmap with all zeros
//     counts = {0, 0, 0}; // Precomputed counts for all zeros
//     size_t p = 10; // Query rank at position 10
//     uint64_t result = rank9(data, counts, p);
//     EXPECT_EQ(result, 0); // Expected rank: 0 ones
// }

TEST(SimpleSelectTest, AllSetBitSearch) {
    std::vector<uint64_t> data = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    SimpleSelect simple;
    simple.build(data, 64, 4);

    uint64_t result = simple.select(3);
    EXPECT_EQ(result, 2);
}

TEST(SimpleSelectTest, AllSetZero) {
    std::vector<uint64_t> data = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    SimpleSelect simple;
    simple.build(data, 64, 4);

    uint64_t result = simple.select(0);
    EXPECT_EQ(result, 0);
}

TEST(SimpleSelectTest, AllSetSecondarySearch) {
    std::vector<uint64_t> data = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    SimpleSelect simple;
    simple.build(data, 64, 4);

    uint64_t result = simple.select(13);
    EXPECT_EQ(result, 12);
}

TEST(SimpleSelectTest, AllSetPrimarySearch) {
    std::vector<uint64_t> data = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    SimpleSelect simple;
    simple.build(data, 64, 4);

    uint64_t result = simple.select(65);
    EXPECT_EQ(result, 64);
}

TEST(SimpleSelectTest, AllSetPrimarySearchSecondary) {
    std::vector<uint64_t> data = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    SimpleSelect simple;
    simple.build(data, 64, 4);

    uint64_t result = simple.select(111);
    EXPECT_EQ(result, 110);
}

TEST(SimpleSelectTest, AlternatingBitsBitSearch) {
    std::vector<uint64_t> data = {0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F};
    SimpleSelect simple;
    simple.build(data, 64, 4);
    uint64_t result = simple.select(1);
    EXPECT_EQ(result, 0);
}

TEST(SimpleSelectTest, AlternatingBitsSecondarySearch) {
    std::vector<uint64_t> data = {0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F};
    SimpleSelect simple;
    simple.build(data, 64, 4);
    uint64_t result = simple.select(1);
    EXPECT_EQ(result, 0);
}

TEST(SimpleSelectTest, AlternatingBits) {
    std::vector<uint64_t> data = {0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F};
    SimpleSelect simple;
    simple.build(data, 64, 4);
    uint64_t result = simple.select(3);
    EXPECT_EQ(result, 2);
    result = simple.select(5);
    EXPECT_EQ(result, 8);
    result = simple.select(8);
    EXPECT_EQ(result, 11);
    result = simple.select(32);
    EXPECT_EQ(result, 59);
    result = simple.select(36);
    EXPECT_EQ(result, 67);
    result = simple.select(48);
    EXPECT_EQ(result, 91);
}


TEST(SimpleRankTest, AllSetBitSearch) {
    std::vector<uint64_t> data = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF};
    SimpleSelect simple;
    simple.build(data, 64, 4);

    uint64_t result = simple.rank(3);
    EXPECT_EQ(result, 4);
    result = simple.rank(13);
    EXPECT_EQ(result, 14);
    result = simple.rank(64);
    EXPECT_EQ(result, 65);
}

TEST(SimpleRankTest, AlternatingBits) {
    std::vector<uint64_t> data = {0x0F0F0F0F0F0F0F0F, 0x0F0F0F0F0F0F0F0F};
    SimpleSelect simple;
    simple.build(data, 64, 4);
    uint64_t result = simple.rank(2);
    EXPECT_EQ(result, 3);
    result = simple.rank(8);
    EXPECT_EQ(result, 5);
    result = simple.rank(11);
    EXPECT_EQ(result, 8);
    result = simple.rank(59);
    EXPECT_EQ(result, 32);
    result = simple.rank(67);
    EXPECT_EQ(result, 36);
    result = simple.rank(91);
    EXPECT_EQ(result, 48);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}