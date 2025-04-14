#include "algos/recsplit.h"
#include "algos/sichash.h"
#include "algos/shockhash.h"
#include "common/utils.h"
#include "common/murmurhash.h"
#include "common/broadword.h"
#include "common/ribbon.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include <unordered_set>
#include <fstream>
#include <random>
#include <chrono>

const std::vector<std::string> test_keys = {"Hello", "World", "RecSplit", "Nelson", "Horatio",
    "Napoleon", "Alexander", "Victory", "Great", "Nile",
    "Vincent", "Dock", "Longbow", "Whistle", "Thyme"};


std::vector<std::string> read_file(std::string file_name) {
    std::ifstream inputFile(file_name);
    std::vector<std::string> words;
    std::string line;

    while (getline(inputFile, line))
    {
        words.push_back(line);
    }

    inputFile.close();
    return words;
}

bool test_perfect_hashing(std::vector<std::string> &keys, HashFunction &hash_function) {
    auto start_time = std::chrono::steady_clock::now();
    hash_function.build(keys);
    auto end_time = std::chrono::steady_clock::now();
    auto duration = end_time - start_time;
    auto build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    std::cout << "Build took: " << build_duration.count() << " ms" << std::endl;
    std::unordered_set<uint32_t> hashes;
    int collisions = 0;
    start_time = std::chrono::steady_clock::now();
    for (auto key : keys) {
        uint32_t hash = hash_function.hash(key);
        if (hashes.contains(hash)) {
            print_colour(key + " collided with another", ConsoleColour::Red);
            collisions++;
        }
        hashes.insert(hash);
    }
    DEBUG_LOG("Hashing Finished");
    end_time = std::chrono::steady_clock::now();
    duration = end_time - start_time;
    auto hashing_duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    std::cout << "Hashing took: " << hashing_duration.count() << " ms" << std::endl;

    if (collisions == 0) {
        DEBUG_LOG("Here");
        if (hashes.size() == keys.size()) {
            print_colour(hash_function.name() + " succeeded minimal perfect hashing!", ConsoleColour::Green);
        } else {
            print_colour(hash_function.name() + " succeeded perfect hashing!", ConsoleColour::Green);
        }
        return true;
    } else {
        print_colour(hash_function.name() + " failed to correctly hash, with " + std::to_string(collisions) + " collisions", ConsoleColour::Red);
        return false;
    }
}

void run_recsplit_random_keys(int n=1000, uint32_t bucket_size=1000, uint32_t leaf_size=8) {
    std::vector<std::string> keys = generate_random_keys(n);

    RecSplit recsplit(bucket_size, leaf_size);
    test_perfect_hashing(keys, recsplit);

    HashFunctionSpace space = recsplit.space();
    std::cout << space << std::endl;
}

void run_sichash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(100000);

    SicHash sichash(1000, 0.3, 0.3, 0.99);
    test_perfect_hashing(keys, sichash);
}

void run_sichash_build() {
    std::vector<std::string> keys = generate_random_keys(15);
    SicHash sichash(15, 0.33, 0.34, 0.99);
    sichash.build(keys);
}

// void run_mumur_64() {
//     std::vector<std::string> keys = generate_random_keys(100000000);
//     std::set<uint64_t> hashes;
//     int collisions = 0;
//     const uint32_t seed = 42;
//     for (auto key : keys) {
//         uint64_t hash = murmur64(key, seed);
//         if (hashes.contains(hash)) {
//             collisions++;
//         }
//         hashes.insert(hash);
//     }

//     if (collisions == 0) {
//         print_colour("Murmur3 64 succeeded hashing without collisions!", ConsoleColour::Green);
//     } else {
//         print_colour(("Murmur3 64 failed hashing with " + std::to_string(collisions) + " collisions!"), ConsoleColour::Red);
//     }
// }

void time_recsplit() {
    run_recsplit_random_keys(100000, 1000, 8);
}

void run_ribbon() {
    // std::vector<std::string> keys = generate_random_keys(10000);
    // std::vector<uint8_t> values(keys.size());
    // for (int i = 0; i < keys.size(); i++) {
    //     values[i] = murmur32(keys[i], 0) % 4;
    // }

    // BasicRibbon basic_ribbon(keys.size(), 64, 2, 0.25);
    // basic_ribbon.build(keys, values);

    // for (int i = 0; i < keys.size(); i++) {
    //     uint8_t query_val = basic_ribbon.query(keys[i]);
    //     DEBUG_LOG("Query Value: " << std::bitset<8>(query_val) << " original value: " << std::bitset<8>(values[i]));
    //     if (query_val != values[i]) {
    //         DEBUG_LOG("########## NOT WORKING #############");
    //     }
    // }

    std::vector<std::string> keys = generate_random_keys(100);
    std::vector<uint64_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 8;
    }

    BasicRibbon basic_ribbon(keys, values, 3, 0.25);

    bool correct = true;
    for (int i = 0; i < keys.size(); i++) {
        uint8_t query_val = basic_ribbon.query(keys[i]);
        // DEBUG_LOG("Query Value: " << std::bitset<64>(query_val) << " original value: " << std::bitset<64>(values[i]));
        if (query_val != values[i]) {
            correct = false;
            DEBUG_LOG("########## NOT WORKING #############");
        }
    }

    DEBUG_LOG("Correct: " << correct);
}

void run_shockhash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(100);
    ShockHash shockhash(100);
    shockhash.build(keys);
}

void run_bipartite_shockhash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(50);
    BipartiteShockHash bipartite_shockhash(50);
    bipartite_shockhash.build(keys);
}

// void run_sichash_random_strings(int n=10000) {
//     SicHash sichash(std::min(5000, n / 5), 0.3, 0.3, 0.9, 42);

//     std::set<std::string> unique_keys;
//     while (unique_keys.size() < n) {
//         std::string key = generateRandomString(10);
//         unique_keys.insert(key);
//     }

//     std::vector<std::string> input_data_vector(unique_keys.begin(), unique_keys.end());

//     sichash.build(input_data_vector);
// }

int main(int argc, char *argv[]) {
    // test_recsplit_file("data/shakespeare.txt");
    // run_ribbon();
    // std::vector<std::string> test_keys = {"Hello",  "RecSplit", "Nelson", "Horatio", "Elijah", "World"};

    // RecSplit recsplit(4, 2);
    // recsplit.build(test_keys);
    // std::vector<uint32_t> hashes;
    // std::set<uint32_t> indexes;
    // int collisions = 0;
    // for (auto key : test_keys) {
    //     uint32_t hash = recsplit.hash(key);
    //     if (indexes.contains(hash)) {
    //         collisions++;
    //     }
    //     indexes.insert(hash);
    //     hashes.push_back(hash);
    // }
    // DEBUG_LOG(hashes);x
    // DEBUG_LOG("Collisions: " << collisions);

    // run_test_words("data/words.txt", 100, 8);
    // run_test_words("data/names.txt", 5, 8);
    // test_random_strings();

    // run_sichash_test_basic();
    // run_sichash_random_strings(1000000);
    run_recsplit_random_keys();
    // run_sichash_random_keys();
    // run_shockhash_random_keys();
    // run_bipartite_shockhash_random_keys();
    // run_sichash_build();
    // run_mumur_64();

    // std::vector<uint64_t> data = {0x0F0F0F0F0F0F0F0F, 0xF0F0F0F0F0F0F0F0};
    // std::cout << data << std::endl;
    // std::vector<uint64_t> counts;
    // generate_rank_counts(data, counts);
    // size_t p = 5;
    // uint64_t result = rank9(data, counts, p);
    // std::cout << "Result of Rank 9: " << result << std::endl;
    // p = 3;
    // result = rank9(data, counts, p);
    // std::cout << "Result of Rank 9: " << result << std::endl;

    return 0;
}