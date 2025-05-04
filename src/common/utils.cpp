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
    int length = std::ceil(std::log(n) / std::log(charset.size()));
    length = std::max(16, length);
    // std::cout << "Generating " << n << " keys with length " << length << std::endl;
    std::string name = "Generating " + std::to_string(n) + " keys with length " + std::to_string(length);
    ProgressBar pbar(100, name);

    while (unique_keys.size() < n) {
        std::string key = generate_random_string(length);
        unique_keys.insert(key);
        if (unique_keys.size() % (n /100) == 0) {
            pbar.update();
        }
    }
    std::cout << "Generation complete!" << std::endl;

    return std::vector<std::string>(unique_keys.begin(), unique_keys.end());
}

std::vector<std::string> read_file(std::string file_name) {
    std::cout << "Reading file: " << file_name << std::endl;
    std::ifstream input_file(file_name);
    std::unordered_set<std::string> keys;
    std::string line;

    while (getline(input_file, line)){
        keys.insert(line);
    }

    input_file.close();
    DEBUG_LOG(keys.size());
    std::cout << "Finished Reading in file" << std::endl;
    return std::vector<std::string>(keys.begin(), keys.end());
}


bool test_perfect_hashing(std::vector<std::string> &keys, HashFunction &hash_function) {
    auto start_time = std::chrono::steady_clock::now();
    hash_function.build(keys);
    auto end_time = std::chrono::steady_clock::now();
    auto duration = end_time - start_time;
    auto build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    std::cout << "Build took: " << build_duration.count() << " ms" << std::endl;
    std::unordered_set<uint32_t> hashes;
    int collisions = 0;
    start_time = std::chrono::steady_clock::now();
    for (auto key : keys) {
        uint32_t hash = hash_function.hash(key);
        if (hashes.contains(hash)) {
            print_colour(key + " collided with another", ConsoleColour::Red);
            collisions++;
        }
        hashes.insert(hash);
    }
    DEBUG_LOG("Hashing Finished");
    end_time = std::chrono::steady_clock::now();
    duration = end_time - start_time;
    auto hashing_duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    std::cout << "Hashing took: " << hashing_duration.count() << " ms" << std::endl;

    if (collisions == 0) {
        DEBUG_LOG("Here");
        if (hashes.size() == keys.size()) {
            print_colour(hash_function.name() + " succeeded minimal perfect hashing!", ConsoleColour::Green);
        } else {
            print_colour(hash_function.name() + " succeeded perfect hashing!", ConsoleColour::Green);
        }
        return true;
    } else {
        print_colour(hash_function.name() + " failed to correctly hash, with " + std::to_string(collisions) + " collisions", ConsoleColour::Red);
        return false;
    }
}

HashFunctionTime time_hashing(std::vector<std::string> &keys, HashFunction &hash_function) {
    HashFunctionTime result;
    auto start_time = std::chrono::steady_clock::now();
    hash_function.build(keys);
    auto end_time = std::chrono::steady_clock::now();
    auto duration = end_time - start_time;
    auto build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    result.build_time = build_duration.count();
    double throughput = (double)keys.size() / ((double)build_duration.count() / 1000);
    std::cout << "Build took: " << build_duration.count() << " ms" << std::endl;
    std::cout << "Build Throughput: " << throughput << " (keys/s)" << std::endl;
    result.build_throughput = throughput;
    start_time = std::chrono::steady_clock::now();
    // ProgressBar pbar(keys.size(), "Hashing Keys");
    for (auto key : keys) {
        uint32_t hash = hash_function.hash(key);
        // pbar.update();
    }
    DEBUG_LOG("Hashing Finished");
    end_time = std::chrono::steady_clock::now();
    duration = end_time - start_time;
    auto hashing_duration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    result.hashing_time = hashing_duration.count();
    std::cout << "Hashing took: " << hashing_duration.count() << " ms" << std::endl;
    throughput = (double)keys.size() / ((double)hashing_duration.count() / 1000);
    std::cout << "Hashing Throughput: " << throughput << " (keys/s)" << std::endl;
    result.hash_throughput = throughput;


    return result;
}

void write_result_to_file(HashTestResult &result, std::string file_name) {
    std::ofstream file(file_name);
    file << "hash_function,params,total_bits,bits_per_key,build_time,hashing_time,throughput,space_usage\n";
}

ProgressBar::ProgressBar(int total, std::string title, int bar_width) : 
total_(total), bar_width_(bar_width), title_(title) {
    start_time = std::chrono::steady_clock::now();
    return;
}

void ProgressBar::update() {
    progress_++;
    display();
}

void ProgressBar::display() {
    std::cout << title_ << " [";
    float progress = (double)progress_ / total_;
    int pos = bar_width_ * progress;
    for (int i = 0; i < bar_width_; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " % ";
    
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed_total = current_time - start_time;
    auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed_total).count();
    int elapsed_minutes = elapsed_seconds / 60;
    int elapsed_remaining_seconds = elapsed_seconds % 60;

    int remaining_seconds = static_cast<int>((1.0 - progress) * elapsed_seconds / progress);
    int remaining_minutes = remaining_seconds / 60;
    remaining_seconds %= 60;

    std::cout << "[" 
                << std::setw(2) << std::setfill('0') << elapsed_minutes << ":" 
                << std::setw(2) << std::setfill('0') << elapsed_remaining_seconds 
                << " | " 
                << std::setw(2) << std::setfill('0') << remaining_minutes << ":" 
                << std::setw(2) << std::setfill('0') << remaining_seconds 
                << "]";

    std::cout << "\r";
    std::cout.flush();

    if (progress == 1) {
        std::cout << std::endl;
    }
}