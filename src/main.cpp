#include "algos/recsplit.h"
#include <iostream>
#include <vector>
#include <fstream>

void run_test_words(const std::string file_path, const uint32_t bucket_size, const uint32_t leaf_size) {
    std::ifstream input_file(file_path);
    std::vector<std::string> input_data;

    std::string line;
    while (std::getline(input_file, line)) {
        input_data.push_back(line);
    }
    input_file.close();

    RecSplit recsplit(bucket_size, leaf_size);
    recsplit.build(input_data);
}

int main()
{
    std::vector<std::string> test_keys = {"Hello", "World", "RecSplit", "Nelson", "Horatio",
        "Napoleon", "Alexander", "Victory", "Great", "Nile",
        "Vincent", "Dock", "Longbow", "Whistle", "Thyme"};

    // RecSplit recsplit(4, 2);
    // recsplit.build(test_keys);

    run_test_words("data/words.txt", 100, 8);
    // run_test_words("data/names.txt", 50, 8);

    return 0;
}