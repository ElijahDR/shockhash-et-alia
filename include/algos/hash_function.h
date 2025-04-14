#ifndef HASHFUNCTION_H
#define HASHFUNCTION_H

#include <string>
#include <vector>

struct HashFunctionSpace {
    std::vector<std::pair<std::string, int>> space_usage;
    int total_bits;
    double bits_per_key;
    int n_keys;
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