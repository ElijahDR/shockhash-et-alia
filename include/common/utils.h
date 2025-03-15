#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <iostream>
#include <cmath>

#ifdef DEBUG
#define DEBUG_LOG(x) std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << "DEBUG: " << x << std::endl
#define DEBUG_VECTOR_LOG(msg, vec) debug_print_vector(msg, vec, __FILE__, __LINE__)
#else
#define DEBUG_LOG(x)
#define DEBUG_VECTOR_LOG(msg, vec)
#endif

int highest_bit(int x);
int log2_approx(int x);

const double PI = 3.14159265358979323846;
const double GOLDEN_RATIO = (std::sqrt(5.0) + 1.0) / 2.0;

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
template <typename T>
inline void debug_print_vector(const std::string msg, const std::vector<T> &vec, const char* file, int line) {
    std::cout << "[" << file << ":" << line << "] " << "DEBUG : " << msg;
    for (const auto &element : vec) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
}

#endif