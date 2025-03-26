#include "algos/recsplit.h"
#include "algos/sichash.h"
#include "common/utils.h"
#include "common/murmurhash.h"
#include <iostream>
#include <vector>
#include <set>
#include <fstream>
#include <random>

const std::vector<std::string> test_keys = {"Hello", "World", "RecSplit", "Nelson", "Horatio",
    "Napoleon", "Alexander", "Victory", "Great", "Nile",
    "Vincent", "Dock", "Longbow", "Whistle", "Thyme"};
const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

std::string generate_random_string(int length) {
    std::string result;
    result.reserve(length);

    // static std::mt19937 generator(std::time(nullptr));
    static std::mt19937 generator(1);
    std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

    for (int i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }

    return result;
}

std::vector<std::string> generate_random_keys(int n) {
    std::set<std::string> unique_keys;
    // const int length = std::ceil(std::log(n) / std::log(charset.size()));
    const int length = 10;
    std::cout << "Generating " << n << " keys with length " << length << std::endl;
    while (unique_keys.size() < n) {
        std::string key = generate_random_string(length);
        unique_keys.insert(key);
    }
    std::cout << "Generation complete!" << std::endl;

    return std::vector<std::string>(unique_keys.begin(), unique_keys.end());
}

bool test_perfect_hashing(std::vector<std::string> &keys, HashFunction &hash_function) {
    hash_function.build(keys);
    std::set<uint32_t> hashes;
    int collisions = 0;
    for (auto key : keys) {
        uint32_t hash = hash_function.hash(key);
        if (hashes.contains(hash)) {
            print_colour(key + " collided with another", ConsoleColour::Red);
            collisions++;
        }
        hashes.insert(hash);
    }

    if (collisions == 0) {
        if (*hashes.rbegin() == keys.size()) {
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

void run_recsplit_random_keys(int n=1000, uint32_t bucket_size=100, uint32_t leaf_size=8) {
    std::vector<std::string> keys = generate_random_keys(n);

    RecSplit recsplit(bucket_size, leaf_size);
    test_perfect_hashing(keys, recsplit);
    recsplit.space();
}

void run_sichash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(100000);

    SicHash sichash(5000, 0.3, 0.3, 0.99);
    test_perfect_hashing(keys, sichash);
}

void run_sichash_build() {
    std::vector<std::string> keys = generate_random_keys(15);
    SicHash sichash(15, 0.33, 0.34, 0.99);
    sichash.build(keys);
}

void run_mumur_64() {
    std::vector<std::string> keys = generate_random_keys(100000000);
    std::set<uint64_t> hashes;
    int collisions = 0;
    const uint32_t seed = 42;
    for (auto key : keys) {
        uint64_t hash = murmur64(key, seed);
        if (hashes.contains(hash)) {
            collisions++;
        }
        hashes.insert(hash);
    }

    if (collisions == 0) {
        print_colour("Murmur3 64 succeeded hashing without collisions!", ConsoleColour::Green);
    } else {
        print_colour(("Murmur3 64 failed hashing with " + std::to_string(collisions) + " collisions!"), ConsoleColour::Red);
    }
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

int main()
{
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
    run_sichash_random_keys();
    // run_sichash_build();
    // run_mumur_64();

    return 0;
}