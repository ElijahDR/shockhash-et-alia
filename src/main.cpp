#include "algos/recsplit.h"
#include <iostream>
#include <vector>

int main()
{
    // Input keys
    std::vector<std::string> keys = {"apple", "banana", "cherry", "date", "elderberry"};

    // Create a RecSplit instance
    RecSplit recsplit(4, 3);

    // Build the hash function
    recsplit.build(keys);

    return 0;
}