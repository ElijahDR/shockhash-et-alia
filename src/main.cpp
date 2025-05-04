#include "algos/recsplit.h"
#include "algos/sichash.h"
#include "algos/shockhash.h"
#include "common/utils.h"
#include "common/murmurhash.h"
#include "common/broadword.h"
#include "common/ribbon.h"
#include "common/bucket_ribbon.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <set>
#include <unordered_set>
#include <fstream>
#include <random>
#include <chrono>

void run_recsplit_random_keys(int n=10000, uint32_t bucket_size=2000, uint32_t leaf_size=14) {
    std::vector<std::string> keys = generate_random_keys(n);

    for (int i = 0; i < 1; i++) {
        RecSplit recsplit(bucket_size, leaf_size, i);
        test_perfect_hashing(keys, recsplit);
        // HashFunctionTime time = time_hashing(keys, recsplit);
        // std::cout << time << std::endl;
    
#ifdef STATS
        std::cout << "Time Spent Bijection: " << recsplit.time_bijection << std::endl;
        std::cout << "Time Spent Splitting: " << recsplit.time_splitting << std::endl;
#endif
        HashFunctionSpace space = recsplit.space();
        std::cout << space << std::endl;
    }
    }

void run_sichash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(1000000);

    SicHash sichash(5000, 0.49, 0.22, 0.9768);
    test_perfect_hashing(keys, sichash);
    // HashFunctionTime time = time_hashing(keys, sichash);
    std::cout << time << std::endl;
    std::cout << sichash.space() << std::endl;
}

void run_sichash_build() {
    std::vector<std::string> keys = generate_random_keys(100);
    SicHash sichash(100, 0.5, 0.5, 0.99);
    sichash.build(keys);

    test_perfect_hashing(keys, sichash);
}

void run_shockhash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(100);
    ShockHash shockhash(100);
    shockhash.build(keys);
}

void run_bipartite_shockhash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(20);
    BipartiteShockHash bipartite_shockhash(keys.size());
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

void record_recsplit_bits_3d() {
    std::vector<std::string> keys = generate_random_keys(100000);
    std::unordered_map<std::string, std::vector<double>> param_ranges_recsplit = {
        {"n_keys", std::vector<double>{100000}},
        {"bucket_size", std::vector<double>{100, 500, 1000, 1500, 2000}},
        {"leaf_size", std::vector<double>{2,3,4,5,6,7,8,9,10,11,12,13,14,15,16}},
    };
    std::vector<HashTestParameters> parameters_recsplit = generate_test_params("RecSplit", param_ranges_recsplit);
    std::vector<HashTestResult> results;
    std::ofstream file;
    file.open("data/results/recsplit-3d-bpk-b-l-100000.csv");
    file << "num_keys,bucket_size,leaf_size,bits_per_key,build_time,hashing_time,splitting_tree,bucket_prefixes,\n";
    file.close();
    for (auto p : parameters_recsplit) {
        file.open("data/results/recsplit-3d-bpk-b-l-100000.csv", std::ios::app);
        std::cout << p << std::endl;
        HashTestResult result = run_hash_function(keys, p, 5);
        std::cout << result << std::endl;
        results.push_back(result);
        file << p.params["n_keys"] << "," << p.params["bucket_size"] << "," << p.params["leaf_size"] << ",";
        file << result.space.bits_per_key << "," << result.time.build_time << "," << result.time.hashing_time << ",";
        for (auto x : result.space.space_usage) {
            if (x.first == "Total Splitting Tree") {
                file << x.second << ",";
            }
        }
        for (auto x : result.space.space_usage) {
            if (x.first == "Bucket Prefixes") {
                file << x.second << "\n";
            }
        }
        file.close();
    }
}

void test_elias_fano() {
    std::vector<uint32_t> data = {};
    for (int i = 0; i < 10; i++) {
        data.push_back(1234);
    }
    data.push_back(1300);
    int n = data.size();
    std::cout << data << std::endl;
    EliasFanoEncodedData encoded = elias_fano_encode(data);
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded, n);
    std::cout << encoded.lower << encoded.lower.size() << std::endl;
    std::cout << encoded.upper << encoded.upper.size() << std::endl;
}

void test_broadword() {
    std::bitset<64> first = 10488084004835287442;
    std::bitset<36> second = 45739767294;
    std::vector<bool> data;
    for (int i = 0; i < first.size(); i++) {
        data.push_back(first[i]);
    }
    for (int i = 0; i < second.size(); i++) {
        data.push_back(second[i]);
    }

    // std::reverse(data.begin(), data.end());
    DEBUG_LOG("Data: " << data);

    SimpleSelect select;
    select.build(data, 64, 16);
    //  0 1 0 0 1 0 0 1 1 0 0 0 0 1 1 1 1 0 0 1 0 1  0  1  0  1  0  1  0  0  0  1  1  0  1  0  1  0  1  1  0  0  0  1  0  1  0  0  1  0  1  1  0  0  0  1  1  0  0  0  1  0  0  1  0  1  1  1  1  1  1  1  1  0  1  0  1  1  1  0  1  0  1  1  0  0  1  0  0  1  1  0  0  1  0  1  0  1  0  1  
    
    
    std::vector<int> ranks = {
        0,1,1,1,2,2,2,3,4,4,4,4,4,5,6,7,8,8,8,9,9,10,10,11,11,12,12,13,13,13,13,14,15,15,16,16,17,17,18,19,19,19,19,20,20,21,21,21,22,22,23,24,24,24,24,25,26,26,26,26,27,27,27,28,28,29,30,31,32,33,34,35,36,36,37,37,38,39,40,40,41,41,42,43,43,43,44,44,44,45,46,46,46,47,47,48,48,49,49,50
    };

    for (int i = 0; i < 100; i++) {
        int rank = select.rank(i);
        DEBUG_LOG("Rank: " << rank << " at index " << i);
        if (rank != ranks[i]) {
            DEBUG_LOG("################# ERROR at index " << i);
            DEBUG_LOG("Expected: " << ranks[i]);
        }
    }

    int rank = select.rank(10);
    DEBUG_LOG("Rank: " << rank);
    rank = select.rank(50);
    DEBUG_LOG("Rank: " << rank);
    rank = select.rank(76);
    DEBUG_LOG("Rank: " << rank);
    rank = select.rank(92);
    DEBUG_LOG("Rank: " << rank);

    std::vector<uint64_t> counts;
    std::vector<uint64_t> new_data = bool_to_uint64(data);
    generate_rank_counts(new_data, counts);

    for (int i = 0; i < 100; i++) {
        int rank = rank9(new_data, counts, i);
        DEBUG_LOG("Rank: " << rank << " at index " << i);
        if (rank != ranks[i]) {
            DEBUG_LOG("################# ERROR at index " << i);
            DEBUG_LOG("Expected: " << ranks[i]);
        }
    }
}

void test_bucketed_ribbon() {
    int n = 1024;
    std::vector<std::string> keys = generate_random_keys(n);
    std::vector<uint64_t> values(keys.size());
    for (int i = 0; i < keys.size(); i++) {
        values[i] = murmur32(keys[i], 0) % 2;
    }

    BucketedRibbon br(keys, values, 1, 64, 0.01);

    ProgressBar pbar(100);
    for (int i = 0; i < keys.size(); i++) {
        uint64_t query_val = br.query(keys[i]);
        if (query_val != values[i]) {
            std::cout << "Not Working" << std::endl;
        }
        if (i % (n / 100) == 0) {
            pbar.update();
        }
    }

    DEBUG_LOG("Finished Querying");
    std::cout << br.space() << std::endl;;
}

void run_recsplit_simple(){
    std::vector<std::string> keys = {
        "Babbage",
        "Churchill",
        "Cromwell",
        "Darwin",
        "Hawking",
        "Lionheart",
        "Lovelace",
        "Nelson",
        "Newton",
        "Nightingale",
        "Shakespeare",
        "Tolkien",
        "Turing",
        "Thatcher",
        "Wellington",
        // "Wilberforce",
        // "A",
        // "B",
        // "C",
        // "D",
        // "E",
        // "F",
        // "G"
    };
    DEBUG_LOG("Keys Size: " << keys.size());
    RecSplit recsplit(keys.size(), 4);
    recsplit.build(keys);
    // test_perfect_hashing(keys, recsplit);
    for (auto key : keys) {
        auto hash = recsplit.hash(key);
        DEBUG_LOG("Key: " << key << " index: " << hash);
    }
}

void generate_random_bit_array() {
    const int num_rows = 40; // Number of rows in the matrix
    const int width = 40;    // Width of each row (number of bits)
    const int ribbon_width = 8; // Width of the "ribbon" (set bits)

    std::vector<std::bitset<width>> bits(num_rows); // Initialize vector with 100 rows
    std::vector<int> starts;

    // Generate random starting positions for each row
    for (int i = 0; i < num_rows; i++) {
        int start = murmur128("elijah", i) % (width - ribbon_width);
        starts.push_back(start);
    }

    // Sort starting positions
    // std::sort(starts.begin(), starts.end(), std::greater<>());
    std::sort(starts.begin(), starts.end());

    // Populate the bit array
    for (int i = 0; i < num_rows; i++) {
        uint128_t set_bits = murmur128("elijah", i) >> (128 - ribbon_width); // Extract ribbon_width bits
        bits[i] = std::bitset<width>{static_cast<unsigned long long>(set_bits)} << starts[i];    
    }

    // Print the generated bit array
    for (int j = 0; j < width; j++) {
        std::bitset<width> b = bits[j];
        for (int i = 0; i < width; i++){
            if (i >= starts[j] && i <= starts[j] + ribbon_width) {
                std::cout << b[i];
            } else {
                std::cout << " ";
            }

        }
        std::cout <<std::endl;
    }
}


int main(int argc, char *argv[]) {

#if defined(__POPCNT__) || defined(_MSC_VER)
    std::cout << "POPCNT Instruction Found" << std::endl;
#else
    std::cout << "Non POPCNT Instruction" << std::endl;
#endif
#if INTPTR_MAX == INT64_MAX
    std::cout << "64 bit" << std::endl;
#elif INTPTR_MAX == INT32_MAX
    std::cout << "32 bit" << std::endl;
#else
    #error Unknown pointer size or missing size macros!
#endif
    // test_hashing_molecules();
    // run_sichash_random_keys();
    // run_recsplit_random_keys();
    // test_elias_fano();
    // test_broadword();
    // test_bucketed_ribbon();
    // run_recsplit_simple();
    // run_bipartite_shockhash_random_keys();
    // generate_random_bit_array();
    record_recsplit_bits_3d();
    return 0;
}