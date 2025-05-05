#ifndef HASHFUNCTION_H
#define HASHFUNCTION_H

#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>

struct HashFunctionSpace {
    std::vector<std::pair<std::string, int>> space_usage;
    int total_bits;
    double bits_per_key;
};

struct HashFunctionTime {
    uint64_t build_time;
    uint64_t hashing_time;
    double build_throughput;
    double hash_throughput;
};

struct HashTestParameters {
    std::string hash_function;
    std::unordered_map<std::string, double> params;
};

struct HashTestResult {
    HashFunctionSpace space;
    HashFunctionTime time;
};

class HashFunction {
public:
    virtual ~HashFunction() = default;
    virtual void build(const std::vector<std::string> &keys) = 0;
    virtual uint32_t hash(const std::string &key) = 0;

    virtual std::string name() = 0;
    virtual HashFunctionSpace space() = 0;
};

inline HashFunctionTime average_time(std::vector<HashFunctionTime> &times) {
    int total_build_time = 0;
    int total_hash_time = 0;
    double total_build_throughput = 0;
    double total_hash_throughput = 0;

    for (auto t : times) {
        total_build_time += t.build_time;
        total_hash_time += t.hashing_time;
        total_build_throughput += t.build_throughput;
        total_hash_throughput += t.hash_throughput;
    }

    return HashFunctionTime{
        (total_build_time / times.size()), 
        (total_hash_time / times.size()), 
        (double)(total_build_throughput / times.size()), 
        (double)(total_hash_throughput / times.size()), 
    };
}

inline HashFunctionSpace average_space(std::vector<HashFunctionSpace> &spaces) {
    int total_bits = 0;
    double total_bits_per_key = 0;
    std::vector<int> space_usages(spaces[0].space_usage.size(), 0);

    for (auto s : spaces) {
        total_bits += s.total_bits;
        total_bits_per_key += s.bits_per_key;
        for (int i = 0; i < s.space_usage.size(); i++) {
            space_usages[i] += s.space_usage[i].second;
        }
    }

    std::vector<std::pair<std::string, int>> total_space_usage;
    for (int i = 0; i < space_usages.size(); i++) {
        total_space_usage.push_back(std::make_pair(spaces[0].space_usage[i].first, space_usages[i] / spaces.size()));
    }

    return HashFunctionSpace{
        total_space_usage,
        (int)(total_bits / spaces.size()), 
        (double)((double)total_bits_per_key / (double)spaces.size()), 
    };
}


#endif