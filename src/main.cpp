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

std::vector<HashTestParameters> generate_test_params(std::string hash_function_name, std::unordered_map<std::string, std::vector<double>> &param_ranges) {
    std::vector<int> indices(param_ranges.size(), 0);
    std::vector<int> sizes;
    for (auto kv : param_ranges) {
        // std::cout << kv.first << std::endl;
        // std::cout << kv.second << std::endl;
        sizes.push_back(kv.second.size());
    }
    std::vector<HashTestParameters> parameters;
    bool done = false;
    while (!done) {
        std::unordered_map<std::string, double> values;
        int indices_index = 0;
        for (auto kv : param_ranges) {
            values.insert(std::make_pair(kv.first, kv.second[indices[indices_index]]));
            indices_index++;
        }

        for (int i = indices.size() - 1; i >= 0; i--) {
            if (indices[i] + 1 < sizes[i]) {
                indices[i]++;
                break;
            } else {
                indices[i] = 0;
                if (i == 0) {
                    done = true;
                }
            }
        }

        HashTestParameters value = HashTestParameters{hash_function_name, values};
        parameters.push_back(value);
        // std::cout << "Indices: " << indices << std::endl;
        // std::cout << "Sizes: " << indices << std::endl;
        // std::cout << value << std::endl;
    }

    return parameters;
    
}

HashTestResult run_hash_function(std::vector<std::string> &keys, HashTestParameters &p) {
    HashFunctionTime time;
    HashFunctionSpace space;
    std::vector<std::string> tmp_keys(keys.begin(), keys.begin() + p.params["n_keys"]);
    DEBUG_LOG(tmp_keys);
    if (p.hash_function == "RecSplit") {
        RecSplit recsplit = RecSplit(p.params["bucket_size"], p.params["leaf_size"]);
        time = time_hashing(keys, recsplit);
        space = recsplit.space();
    }

    return HashTestResult{space, time};
}

void test_hashing_molecules() {
    // std::unordered_map<std::string, std::vector<double>> param_ranges_recsplit = {
    //     {"n_keys", std::vector<double>{100000}},
    //     {"bucket_size", std::vector<double>{500, 1000, 2000}},
    //     {"leaf_size", std::vector<double>{8, 10, 12, 14, 16, 18, 20, 22}},
    // };
    std::unordered_map<std::string, std::vector<double>> param_ranges_recsplit = {
        {"n_keys", std::vector<double>{100}},
        {"bucket_size", std::vector<double>{100}},
        {"leaf_size", std::vector<double>{8}},
    };
    std::vector<HashTestParameters> parameters = generate_test_params("RecSplit", param_ranges_recsplit);
    std::vector<std::string> molecules_keys = read_file("data/temp/molecules-1000.txt");
    for (auto p : parameters) {
        std::cout << p << std::endl;
        HashTestResult result = run_hash_function(molecules_keys, p);
        std::cout << result << std::endl;
    }
}

int main(int argc, char *argv[]) {
    test_hashing_molecules();

    return 0;
}