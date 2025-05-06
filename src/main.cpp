#include "algos/recsplit.h"
#include "algos/sichash.h"
#include "algos/shockhash.h"
#include "algos/shockhash_rs.h"
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

void run_recsplit_random_keys(int n=1000000, uint32_t bucket_size=5, uint32_t leaf_size=5) {
    std::vector<std::string> keys = generate_random_keys(n);

    for (int i = 0; i < 1; i++) {
        RecSplit recsplit(bucket_size, leaf_size, i);
        // test_perfect_hashing(keys, recsplit);
        HashFunctionTime time = time_hashing(keys, recsplit);
        std::cout << time << std::endl;
    
#ifdef STATS
        std::cout << "Time Spent Bijection: " << recsplit.time_bijection << std::endl;
        std::cout << "Time Spent Splitting: " << recsplit.time_splitting << std::endl;
        std::cout << "Time Spent Buckets: " << recsplit.time_buckets << std::endl;
#endif
        HashFunctionSpace space = recsplit.space();
        std::cout << space << std::endl;
    }
    }

void run_sichash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(1000000);

    SicHash sichash(5000, 0.39, 0.22, 0.9768, 0);
    // test_perfect_hashing(keys, sichash);
    HashFunctionTime hash_time = time_hashing(keys, sichash);
    std::cout << hash_time << std::endl;
    std::cout << sichash.space() << std::endl;
}

void run_sichash_build() {
    std::vector<std::string> keys = generate_random_keys(100);
    SicHash sichash(100, 0.5, 0.5, 0.99);
    sichash.build(keys);

    test_perfect_hashing(keys, sichash);
}

void run_shockhash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(7);
    ShockHash shockhash(7);
    shockhash.build(keys);
}

void run_bipartite_shockhash_random_keys() {
    std::vector<std::string> keys = generate_random_keys(9);
    BipartiteShockHash bipartite_shockhash;
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

HashTestResult run_hash_function(std::vector<std::string> &keys, HashTestParameters &p, int test_runs=1, uint32_t seed=0) {
    HashFunctionTime time;
    HashFunctionSpace space;
    std::vector<HashFunctionTime> times;
    std::vector<HashFunctionSpace> spaces;
    std::vector<std::string> tmp_keys(keys.begin(), keys.begin() + p.params["n_keys"]);
    DEBUG_LOG(tmp_keys);
    if (p.hash_function == "RecSplit") {
        for (int i = 0; i < test_runs; i++) {
            RecSplit recsplit = RecSplit(p.params["bucket_size"], p.params["leaf_size"], seed+i);
            times.push_back(time_hashing(tmp_keys, recsplit));
            spaces.push_back(recsplit.space());
        }
    } else if (p.hash_function == "SicHash") {
        for (int i = 0; i < test_runs; i++) {
            SicHash sichash = SicHash(p.params["bucket_size"], p.params["p1"], p.params["p2"], p.params["alpha"], seed+i);
            times.push_back(time_hashing(tmp_keys, sichash));
            spaces.push_back(sichash.space());
        }
    } else if (p.hash_function == "ShockHash") {
        for (int i = 0; i < test_runs; i++) {
            ShockHashRS shockhash = ShockHashRS(p.params["bucket_size"], p.params["leaf_size"], seed+i);
            times.push_back(time_hashing(tmp_keys, shockhash));
            spaces.push_back(shockhash.space());
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

void record_recsplit_data() {
    std::unordered_map<std::string, std::vector<double>> param_ranges_recsplit = {
        {"n_keys", std::vector<double>{1000000}},
        {"bucket_size", std::vector<double>{5, 9, 100, 2000}},
        {"leaf_size", std::vector<double>{5, 8, 12, 16}},
    };
    std::vector<std::string> keys = generate_random_keys_random_length(1000000);
    std::vector<HashTestParameters> parameters_recsplit = generate_test_params("RecSplit", param_ranges_recsplit);
    std::vector<HashTestResult> results;
    std::ofstream file;
    std::string file_name = "data/results/recsplit-main-main.csv";
    // file.open(file_name);
    // file << "num_keys,bucket_size,leaf_size,total_bits,build_time,hashing_time,bijection_time,splitting_time,bucket_time,splitting_tree,unary,fixed,bucket_prefixes\n";
    // file.close();
    for (auto p : parameters_recsplit) {
        std::cout << p << std::endl;
        std::vector<HashFunctionTime> times;
        std::vector<HashFunctionSpace> spaces;
        std::vector<uint64_t> bijection_times;
        std::vector<uint64_t> splitting_times;
        for (int i = 0; i < 1; i++) {
            file.open(file_name, std::ios::app);
            // RecSplit recsplit0 = RecSplit(p.params["bucket_size"], p.params["leaf_size"], i);

            // test_perfect_hashing(keys, recsplit0);
            RecSplit recsplit = RecSplit(p.params["bucket_size"], p.params["leaf_size"], i);
            HashFunctionTime time = time_hashing(keys, recsplit);
            HashFunctionSpace space = recsplit.space();

            std::cout << "Space: " << space << std::endl;
            std::cout << "Time: " << time << std::endl;
            std::cout << "Time Splitting: " << recsplit.time_splitting << std::endl;
            std::cout << "Time Bijecting: " << recsplit.time_bijection << std::endl;
            std::cout << "Time Creating Buckets: " << recsplit.time_buckets << std::endl;

            file << p.params["n_keys"] << "," << p.params["bucket_size"] << "," << p.params["leaf_size"] << ",";
            file << space.total_bits << "," << time.build_time << "," << time.hashing_time << ",";
            file << recsplit.time_bijection << ",";
            file << recsplit.time_splitting << ",";
            file << recsplit.time_buckets << ",";
            for (auto x : space.space_usage) {
                if (x.first == "Total Splitting Tree") {
                    file << x.second << ",";
                }
            }
            for (auto x : space.space_usage) {
                if (x.first == "Splitting Tree Unary") {
                    file << x.second << ",";
                }
            }
            for (auto x : space.space_usage) {
                if (x.first == "Splitting Tree Fixed") {
                    file << x.second << ",";
                }
            }
            for (auto x : space.space_usage) {
                if (x.first == "Bucket Prefixes") {
                    file << x.second << "\n";
                }
            }
            file.close();
        }
    }
}

void record_shockhash_data() {
    std::vector<std::string> keys = generate_random_keys_random_length(1000000);
    std::unordered_map<std::string, std::vector<double>> param_ranges_recsplit = {
        {"n_keys", std::vector<double>{1000000}},
        {"bucket_size", std::vector<double>{8}},
        {"leaf_size", std::vector<double>{8}},
    };
    std::vector<HashTestParameters> parameters_recsplit = generate_test_params("ShockHash", param_ranges_recsplit);
    std::vector<HashTestResult> results;
    std::ofstream file;
    std::string filename = "data/results/shockhash-6-6.csv";
    file.open(filename);
    file << "num_keys,bucket_size,leaf_size,total_bits,build_time,hashing_time,bijection_time,splitting_time,splitting_tree,unary,fixed,bucket_prefixes,burr\n";
    file.close();
    for (auto p : parameters_recsplit) {
        std::cout << p << std::endl;
        std::vector<HashFunctionTime> times;
        std::vector<HashFunctionSpace> spaces;
        std::vector<uint64_t> bijection_times;
        std::vector<uint64_t> splitting_times;
        for (int i = 0; i < 10; i++) {
            file.open(filename, std::ios::app);
            ShockHashRS recsplit = ShockHashRS(p.params["bucket_size"], p.params["leaf_size"], i);

            HashFunctionTime time = time_hashing(keys, recsplit);
            HashFunctionSpace space = recsplit.space();

            std::cout << "Space: " << space << std::endl;
            std::cout << "Time: " << time << std::endl;
            std::cout << "Time Splitting: " << recsplit.time_splitting << std::endl;
            std::cout << "Time Bijecting: " << recsplit.time_bijection << std::endl;

            file << p.params["n_keys"] << "," << p.params["bucket_size"] << "," << p.params["leaf_size"] << ",";
            file << space.total_bits << "," << time.build_time << "," << time.hashing_time << ",";
            file << recsplit.time_bijection << ",";
            file << recsplit.time_splitting << ",";
            for (auto x : space.space_usage) {
                if (x.first == "Total Splitting Tree") {
                    file << x.second << ",";
                }
            }
            for (auto x : space.space_usage) {
                if (x.first == "Splitting Tree Unary") {
                    file << x.second << ",";
                }
            }
            for (auto x : space.space_usage) {
                if (x.first == "Splitting Tree Fixed") {
                    file << x.second << ",";
                }
            }
            for (auto x : space.space_usage) {
                if (x.first == "Bucket Prefixes") {
                    file << x.second << ",";
                }
            }
            for (auto x : space.space_usage) {
                if (x.first == "Retrieval Structure") {
                    file << x.second << "\n";
                }
            }
            file.close();
        }
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
    std::vector<uint32_t> decoded_data = elias_fano_decode(encoded);
    std::cout << encoded.lower << encoded.lower.size() << std::endl;
    std::cout << encoded.upper << encoded.upper.size() << std::endl;
}

void test_broadword() {
    std::bitset<64> first = 10488084004835287442ULL;
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

void run_sichash_simple(){
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
    SicHash sichash(keys.size(), 0.33, 0.33, 0.9, 0, 4, 42, 3);
    // sichash.build(keys);
    test_perfect_hashing(keys, sichash);
    // for (auto key : keys) {
    //     auto hash = sichash.hash(key);
    //     DEBUG_LOG("Key: " << key << " index: " << hash);
    // }
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

void test_shockhash_rs() {
    std::vector<std::string> keys = generate_random_keys_random_length(100000);

    ShockHashRS shockhashrs(2000, 50);
    test_perfect_hashing(keys, shockhashrs);
    // HashFunctionTime hash_time = time_hashing(keys, shockhashrs);
    // std::cout << hash_time << std::endl;
#ifdef STATS    
    std::cout << "Time Spent Bijection: " << shockhashrs.time_bijection << std::endl;
    std::cout << "Time Spent Splitting: " << shockhashrs.time_splitting << std::endl;
    std::cout << "Time Spent Ribbon Construction: " << shockhashrs.time_ribbon << std::endl;
#endif
    std::cout << shockhashrs.space() << std::endl;
}

void test_triangular() {
    std::pair<uint32_t, uint32_t> pair = std::make_pair(4, 1);
    uint32_t x = triangular_pairing_function(pair);
    DEBUG_LOG("X: " << x);
    std::pair<uint32_t, uint32_t> decode_pair = triangular_pairing_function_undo(x);
    DEBUG_LOG("Decoded: " << decode_pair);
}

void run_burr() {
    std::vector<int> rs = {1,2,3,4,5};
    std::vector<int> layers = {2,3,4};
    std::vector<double> epsilons = {-0.2, -0.15, -0.1, -0.05, 0, 0.05, 0.1, 0.15, 0.2};
    std::vector<int> buckets = {32, 64, 128, 256};
    std::vector<std::string> keys = generate_random_keys(10000000);
    std::vector<uint64_t> values(keys.size());

    std::ofstream file;
    std::string filename = "data/results/burr-10000000.csv";
    // file.open(filename);
    // file << "num_keys,r,layers,epsilon,bucket,build_time,query_time,space,metadata,Z\n";
    // file.close();
    for (auto r : rs) {
        std::cout << "Generating Values" << std::endl;
        for (int i = 0; i < keys.size(); i++) {
            values[i] = murmur32(keys[i], 0) % (int)std::pow(2, r);
        }      
        std::cout << "Generating Values Done" << std::endl;
        for (auto layer : layers) {
            for (auto epsilon : epsilons) {
                for (auto bucket : buckets) {
                    file.open(filename, std::ios::app);
                    file << keys.size() << ",";
                    file << r << "," << layer << "," << epsilon << "," << bucket << ",";
                    std::cout << "R: " << r << " layers: " << layer << " e: " << epsilon << " bucket: " << bucket << std::endl;
                    auto start_time = std::chrono::steady_clock::now();
                    BuRR burr(keys, values, r, epsilon, bucket, layer);
                    auto end_time = std::chrono::steady_clock::now();
                    auto duration = end_time - start_time;
                    auto build_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
                    uint64_t building_time = build_duration.count();
                    file << building_time << ",";
                    std::cout << "Building Time: " << building_time << std::endl;
                    start_time = std::chrono::steady_clock::now();
                    for (int i = 0; i < keys.size(); i++) {
                        uint64_t query_val = burr.query(keys[i]);
                    }
                    end_time = std::chrono::steady_clock::now();
                    duration = end_time - start_time;
                    build_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
                    uint64_t query_time = build_duration.count();
                    std::cout << "Query Time: " << query_time << std::endl;
                    file << query_time << ",";
                    BuRRSpace space = burr.space();
                    std::cout << "Space: " << space.total_bits << std::endl;
                    file << space.total_bits << "," << space.total_metadata << "," << space.total_Z << "\n";
                    file.close();
                }
            }
        }
    }


}

void run_sichash_tests() {
    std::vector<std::string> keys = generate_random_keys_random_length(1000000);
    std::vector<std::vector<double>> probs = {
        {0, 1, 0.976},
        {0.1, 0.8, 0.981},
        {0.33, 0.34, 0.988},
        {0.50, 0, 0.991},
        {0.21, 0.78, 0.9},
        {0.45, 0.031, 0.97},
        {0.49, 0.22, 0.9768},
    };


    std::vector<uint32_t> buckets = {50, 500, 5000};

    std::ofstream file;
    std::string filename = "data/results/sichash-1000000-4.csv";
    file.open(filename);
    file << "SicHash varying bucket sizes and probabilities. BuRR: e=0, l=4, b=128" << "\n";
    file << "num_keys,b,p1,p2,alpha,total_bits,build_time,query_time,minimal_overhead,bucket_seeds,total_ribbon,bucket_prefixes\n";
    file.close();
    int i = 0;
    for (auto prob : probs) {
        for (auto b : buckets) {
            for (int i = 0; i < 5; i++){
                file.open(filename, std::ios::app);
                file << keys.size() << "," << b << "," << prob[0] << "," << prob[1] << "," << prob[2] << ",";
                std::cout << "PRobs: " << prob << std::endl;
                std::cout << "B: " << b << std::endl;
                SicHash sichash(b, prob[0], prob[1], prob[2], 0, 4, 128, i);
                HashFunctionTime time = time_hashing(keys, sichash);
                HashFunctionSpace space = sichash.space();
                file << space.total_bits << "," << time.build_time << "," << time.hashing_time << ",";
                std::cout << time << std::endl;
                std::cout << space << std::endl;

                for (auto x : space.space_usage) {
                    if (x.first == "To Make Minimal") {
                        file << x.second << ",";
                    }
                }
                for (auto x : space.space_usage) {
                    if (x.first == "Bucket Seeds") {
                        file << x.second << ",";
                    }
                }
                for (auto x : space.space_usage) {
                    if (x.first == "Total Ribbon") {
                        file << x.second << ",";
                    }
                }
                for (auto x : space.space_usage) {
                    if (x.first == "Bucket Prefixes") {
                        file << x.second << "\n";
                    }
                }
                i++;
                file.close();
            }
        }
    }
}

void run_sichash_space_budget() {
    std::vector<std::string> keys = generate_random_keys_random_length(1000000);
    std::vector<double> space_budgets = {1.8};
    std::vector<double> alphas = {0.984};
    std::vector<uint32_t> buckets = {5000};

    std::ofstream file;
    std::string filename = "data/results/sichash-1000000-budget-2.csv";
    // file.open(filename);
    // file << "num_keys,b,p1,p2,alpha,total_bits,build_time,query_time,minimal_overhead,bucket_seeds,total_ribbon,bucket_prefixes\n";
    // file.close();
    int i = 0;
    for (int j = 0; j < space_budgets.size(); j++) {
        auto sb = space_budgets[j];
        auto alpha = alphas[j];
        for (auto b : buckets) {
            for (double x = 0.7; x < 0.8; x+=0.1) {
                file.open(filename, std::ios::app);
                std::cout << "Space Budget: " << sb << std::endl;
                std::cout << "x: " << x << std::endl;
                std::cout << "B: " << b << std::endl;
                std::pair<double, double> probs = space_budget(sb, x);
                std::cout << "probs: " << probs << std::endl;
                file << keys.size() << "," << b << "," << probs.first << "," << probs.second << "," << alpha << ",";
                SicHash sichash(b, probs.first, probs.second, alpha, 0, 4, 128, 0);
                HashFunctionTime time = time_hashing(keys, sichash);
                // std::cout << time << std::endl;
                // test_perfect_hashing(keys, sichash);
                HashFunctionSpace space = sichash.space();
                file << space.total_bits << "," << time.build_time << "," << time.hashing_time << ",";
                std::cout << time << std::endl;
                std::cout << space << std::endl;

                for (auto x : space.space_usage) {
                    if (x.first == "To Make Minimal") {
                        file << x.second << ",";
                    }
                }
                for (auto x : space.space_usage) {
                    if (x.first == "Bucket Seeds") {
                        file << x.second << ",";
                    }
                }
                for (auto x : space.space_usage) {
                    if (x.first == "Total Ribbon") {
                        file << x.second << ",";
                    }
                }
                for (auto x : space.space_usage) {
                    if (x.first == "Bucket Prefixes") {
                        file << x.second << "\n";
                    }
                }
                i++;
                file.close();
            }
        }
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
    // run_sichash_simple();
    // run_bipartite_shockhash_random_keys();
    // generate_random_bit_array();
    // record_recsplit_bits_3d();
    // test_shockhash_rs();
    record_recsplit_data();
    // run_burr();
    // run_sichash_tests();
    // record_shockhash_data();
    // run_sichash_space_budget();
    // test_triangular();
    return 0;
}