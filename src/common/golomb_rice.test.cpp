#include "common/golomb_rice.h"
#include <iostream>

bool test_golomb_rice()
{
    struct GolombTestData
    {
        uint32_t value;
        uint32_t r;
    };

    std::vector<GolombTestData> test_cases = {
        {0, 1},
        {1, 1},
        {2, 1},
        {3, 1},
        {4, 1},
        {5, 1},
        {6, 1},
        {7, 1},
        {8, 1},
        {10, 2},
        {15, 2},
        {20, 2},
        {25, 2},
        {30, 2},
        {50, 3},
        {100, 3},
        {200, 4},
        {500, 5},
        {1024, 6},
        {2048, 7},
        {4096, 8},
        {8192, 9},
        {16384, 10},
        {32768, 11},
        {65536, 12},
        {131072, 13},
        {262144, 14},
        {12345, 5},
        {67890, 6},
        {987654, 10},
        {1234567, 12},
        {2345678, 14},
        {3456789, 16},
        {4567890, 18},
        {5678901, 20},
    };

    for (auto test_case : test_cases)
    {
        uint32_t value = test_case.value;
        uint32_t test_value = golomb_rice_decode(golomb_rice_encode(value, test_case.r), test_case.r);

        if (value != test_value)
        {
            std::cerr << "Test failed for value = " << value << ", r parameter = " << (int)test_case.r
                      << ". Decoded value = " << test_value << std::endl;
            exit(1);
        }
    }

    return true;
}