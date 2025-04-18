#ifndef HASHFUNCTION_H
#define HASHFUNCTION_H

#include <string>
#include <vector>
#include <unordered_map>

struct HashFunctionSpace {
    std::vector<std::pair<std::string, int>> space_usage;
    int total_bits;
    double bits_per_key;
};

struct HashFunctionTime {
    int build_time;
    int hashing_time;
    double throughput;
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


#endif