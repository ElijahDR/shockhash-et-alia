#ifndef SHOCKHASH_H
#define SHOCKHASH_H

#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include "algos/hash_function.h"


class ShockHash : public HashFunction {
public:
    ShockHash();

    void build(const std::vector<std::string> &keys) override;

    uint32_t hash(const std::string &key) override;

    std::string name() override { return "ShockHash"; };

private:


    std::vector<std::string> keys_;
};


#endif