#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include "algos/sichash.h"

#ifdef DEBUG
#define DEBUG_LOG(x) std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << "DEBUG: " << x << std::endl
#else
#define DEBUG_LOG(x)
#endif

const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

int highest_bit(int x);
int log2_approx(int x);

std::string generate_random_string(int length);
std::vector<std::string> generate_random_keys(int n);

const double PI = 3.14159265358979323846;
const double GOLDEN_RATIO = (std::sqrt(5.0) + 1.0) / 2.0;

typedef unsigned __int128 uint128_t;

enum class ConsoleColour {
    Reset = 0,
    Bold = 1,
    Black = 30,
    Red = 31,
    Green = 32,
    Yellow = 33,
    Blue = 34,
    Magenta = 35,
    Cyan = 36,
    White = 37
};

template <typename T>
inline size_t size_of_print(const T data) {
    std::cout << "Size: " << sizeof(T) * 8 << " bits" << std::endl;
    return sizeof(T);
}

template <typename T>
inline size_t size_of(const std::vector<T> data) {
    size_t overhead = sizeof(data) * 8;
    size_t data_size = data.size() * sizeof(T);

    return data_size + overhead;

}


inline std::vector<bool> int_to_bits(int number) {
    if (number == 0) {
        return std::vector<bool>{0};
    }
    std::vector<bool> bool_vec;
    while (number > 0) {
        bool_vec.push_back(number & 1);
        number >>= 1;
    }

    return bool_vec;
}

inline int bits_to_int(std::vector<bool> &bits) {
    int result = 0;
    for (int i = 0; i < bits.size(); i++) {
        if (bits[i]) {
            result |= 1 << i;
        }
    }

    return result;
}

inline void print_colour(const std::string &msg, ConsoleColour colour) {
    std::cout << "\033[" << (int)colour << "m" << msg << " \033[0m" << std::endl;
}

inline std::string special_string(const std::string &msg, ConsoleColour colour) {
    return "\033[" + std::to_string((int)colour) + "m" + msg + "\033[0m";
}



// Vector printing
template <typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T> &data) {
    for (const auto &element : data) {
        os << element << " ";
    }
    return os;
}

// Map Printing
template <typename K, typename V>
inline std::ostream& operator<<(std::ostream& os, const std::map<K, V> &data) {
    os << "{";
    for (const auto &[key, value] : data) {
        os << key << ": " << value << ", ";
    }
    os << "}";
    return os;
}



template <typename T>
void append_vector_to_vector(std::vector<T> &parent, const std::vector<T> &child) {
    parent.insert(
        parent.end(),
        child.begin(),
        child.end()
    );
}

template <typename T>
void print_vector(const std::vector<T> &vec) {
    for (const auto &element : vec) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

#endif