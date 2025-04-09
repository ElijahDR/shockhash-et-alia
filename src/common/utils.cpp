#include "common/utils.h"
#include <unordered_set>
#include <random>

int highest_bit(int x)
{
    int index = -1;
    while (x > 0)
    {
        x >>= 1;
        index++;
    }
    return index;
}

int log2_approx(int x)
{
    int y = x + (x >> 1);

    return highest_bit(y);
}
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
    std::unordered_set<std::string> unique_keys;
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