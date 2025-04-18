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

void run_recsplit_random_keys(int n=1000000, uint32_t bucket_size=1000, uint32_t leaf_size=8) {
    std::vector<std::string> keys = generate_random_keys(n);

    for (int i = 0; i < 1; i++) {
        RecSplit recsplit(bucket_size, leaf_size, i);
        test_perfect_hashing(keys, recsplit);
        // HashFunctionTime time = time_hashing(keys, recsplit);
        // std::cout << time << std::endl;
    
        HashFunctionSpace space = recsplit.space();
        std::cout << space << std::endl;
    }
    }

void run_sichash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(100000);

    SicHash sichash(1000, 0.3, 0.3, 0.99);
    test_perfect_hashing(keys, sichash);
}

void run_sichash_build() {
    std::vector<std::string> keys = generate_random_keys(100);
    SicHash sichash(100, 0.33, 0.34, 0.99);
    sichash.build(keys);

    test_perfect_hashing(keys, sichash);
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

HashTestResult run_hash_function(std::vector<std::string> &keys, HashTestParameters &p, int test_runs=1) {
    HashFunctionTime time;
    HashFunctionSpace space;
    std::vector<HashFunctionTime> times;
    std::vector<HashFunctionSpace> spaces;
    std::vector<std::string> tmp_keys(keys.begin(), keys.begin() + p.params["n_keys"]);
    DEBUG_LOG(tmp_keys);
    if (p.hash_function == "RecSplit") {
        for (int i = 0; i < test_runs; i++) {
            RecSplit recsplit = RecSplit(p.params["bucket_size"], p.params["leaf_size"], i);
            times.push_back(time_hashing(tmp_keys, recsplit));
            spaces.push_back(recsplit.space());
        }
    } else if (p.hash_function == "SicHash") {
        for (int i = 0; i < test_runs; i++) {
            SicHash sichash = SicHash(p.params["bucket_size"], p.params["p1"], p.params["p2"], p.params["alpha"]);
            times.push_back(time_hashing(tmp_keys, sichash));
            spaces.push_back(sichash.space());
        }
    }

    return HashTestResult{average_space(spaces), average_time(times)};
}

void test_hashing_molecules() {
    // std::vector<std::string> molecules_keys = read_file("data/temp/molecules-100000.txt");
    std::vector<std::string> molecules_keys = generate_random_keys(1000000);
    // std::unordered_map<std::string, std::vector<double>> param_ranges_recsplit = {
    //     {"n_keys", std::vector<double>{100000}},
    //     {"bucket_size", std::vector<double>{500, 1000, 2000}},
    //     {"leaf_size", std::vector<double>{8, 10, 12, 14, 16, 18, 20, 22}},
    // };
    std::unordered_map<std::string, std::vector<double>> param_ranges_recsplit = {
        {"n_keys", std::vector<double>{1000000}},
        {"bucket_size", std::vector<double>{500, 1000, 2000}},
        {"leaf_size", std::vector<double>{8, 16, 24}},
    };
    std::vector<HashTestParameters> parameters_recsplit = generate_test_params("RecSplit", param_ranges_recsplit);
    for (auto p : parameters_recsplit) {
        std::cout << p << std::endl;
        HashTestResult result = run_hash_function(molecules_keys, p, 2);
        std::cout << result << std::endl;
    }
    std::unordered_map<std::string, std::vector<double>> param_ranges_sichash = {
        {"n_keys", std::vector<double>{100000}},
        {"bucket_size", std::vector<double>{500, 1000, 2000}},
        {"p1", std::vector<double>{0.33, 0.5}},
        {"p2", std::vector<double>{0.33, 0.5}},
        {"alpha", std::vector<double>{0.95}},
    };
    std::vector<HashTestParameters> parameters_sichash = generate_test_params("SicHash", param_ranges_sichash);
    for (auto p : parameters_sichash) {
        std::cout << p << std::endl;
        HashTestResult result = run_hash_function(molecules_keys, p, 2);
        std::cout << result << std::endl;
    }
}

int main(int argc, char *argv[]) {
    // test_hashing_molecules();
    // run_sichash_random_keys();
    run_recsplit_random_keys();

    return 0;
}