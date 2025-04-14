#ifndef HASHFUNCTION_H
#define HASHFUNCTION_H

#include <string>
#include <vector>
#include <unordered_map>

struct HashFunctionSpace {
    std::unordered_map<std::string, int> data_usage;
    int overall;
    double bits_per_key;
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