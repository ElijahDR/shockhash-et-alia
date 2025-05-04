#include "common/broadword.h"
#include <cmath>
#include <iostream>
#include <bit>
#include <algorithm> 

SimpleSelect::SimpleSelect() {
    return;
}

void SimpleSelect::build(std::vector<uint64_t> &data, uint64_t L, uint64_t M) {
    L_ = L;
    M_ = M;
    data_ = data;
    build_inventories();
    DEBUG_LOG("Inventories Built");
}

void SimpleSelect::build(std::vector<bool> &data, uint64_t L, uint64_t M) {
    L_ = L;
    M_ = M;

    DEBUG_LOG("Bool Data: " << data);
    DEBUG_LOG("Data Size: " << data.size());
    uint64_t n_words = std::ceil((double)data.size() / 64);
    DEBUG_LOG("N Words: " << n_words);
    // std::vector<uint64_t> data_(n_words, 0);
    for (int w = 0; w < n_words; w++) {
        uint64_t word = 0;
        for (int i = 0; i < 64; i++) {
            DEBUG_LOG("Word: " << (std::bitset<64>)word);
            uint64_t index = (64 * w) + i;
            DEBUG_LOG("Index: " << index);
            if (index > data.size()) {
                break;
            }
            
            uint64_t mask = (uint64_t)data[index] << i;
            DEBUG_LOG("Mask: " << (std::bitset<64>)mask);
            // DEBUG_LOG("Word and Mask: " << (word | mask));
            word |= mask;
        }
        data_.push_back(word);
    }

    build_inventories();
    DEBUG_LOG("Inventories Built");
}

void SimpleSelect::build_inventories() {
    DEBUG_LOG("Data: " << data_);
    DEBUG_LOG("L: " << L_);
    DEBUG_LOG("M: " << M_);

    uint64_t total_bits = 0, set_bits = 0;
    for (int w = 0; w < data_.size(); w++) {
        set_bits += popcount(data_[w]);
        total_bits += 64;
    }
    DEBUG_LOG("Set Bits: " << set_bits);
    DEBUG_LOG("Total Bits: " << total_bits);

    M_spacing_ = std::ceil((M_ * set_bits) / (double)total_bits);
    L_spacing_ = M_spacing_ * ((double)L_ / M_);
    LM_ratio_ = std::ceil((double)L_spacing_ / M_spacing_);
    DEBUG_LOG("L Spacing: " << L_spacing_);
    DEBUG_LOG("M Spacing: " << M_spacing_);

    uint64_t rank = 0;
    for (int w = 0; w < data_.size(); w++) {
        uint64_t word = data_[w];
        // DEBUG_LOG("Rank: " << rank);
        for (int i = 0; i < 64; i++) {
            uint64_t mask = ((uint64_t)1 << i);
            // DEBUG_LOG("Word: " << word);
            // DEBUG_LOG("Mask: " << mask);
            if (word & mask) {
                rank++;
            }
            uint64_t index = w * 64 + i;
            if (index % L_spacing_ == 0) {
                primary_inventory.push_back(rank);
            } 
            if (index % M_spacing_ == 0) {
                secondary_inventory.push_back(rank - primary_inventory.back());
            }
        }
    }
    primary_inventory.push_back(set_bits + 1);
    secondary_inventory.push_back(set_bits + 1);
    DEBUG_LOG("Primary Inventory: " << primary_inventory);
    DEBUG_LOG("Secondary Inventory: " << secondary_inventory);
}

uint64_t SimpleSelect::select(uint64_t r) {
    DEBUG_LOG("Selecting rank: " << r);
    for (auto w : data_) {
        DEBUG_LOG("Word: " << (std::bitset<64>)w);
    }
    if (r == 0) { 
        return 0;
    }
    // uint64_t primary_index = std::floor((float)r / L_spacing_);
    // uint64_t previous_primary_index = primary_index;
    // DEBUG_LOG("Primary Index: " << primary_index);
    // uint64_t current_rank = primary_inventory[primary_index];
    // int gap = 1;
    // DEBUG_LOG("Current Rank: " << current_rank);
    // while (current_rank < r) {
    //     primary_index++;
    //     current_rank = primary_inventory[primary_index];
    //     if (primary_inventory[primary_index-1] != primary_inventory[primary_index-2]) {
    //         gap = 1;
    //     } else {
    //         gap++;
    //     }
    //     DEBUG_LOG("Gap: " << gap);
    // }
    // primary_index = primary_index < gap ? 0 : primary_index - gap;
    // current_rank = primary_inventory[primary_index];
    // DEBUG_LOG("Primary Index: " << primary_index);
    // DEBUG_LOG("Current Rank: " << current_rank);

    // uint64_t rank_left = r - current_rank;
    // DEBUG_LOG("Rank Left: " << rank_left);
    // if (rank_left == 0) {
    //     return (primary_index * L_spacing_);
    // }

    auto it = std::lower_bound(primary_inventory.begin(), primary_inventory.end(), r);
    uint64_t primary_index = std::distance(primary_inventory.begin(), it);
    DEBUG_LOG("Primary Index Before Min: " << primary_index);
    primary_index = primary_index > 0 ? primary_index - 1 : 0;

    uint64_t current_rank = primary_inventory[primary_index];
    DEBUG_LOG("Primary Index: " << primary_index);
    DEBUG_LOG("Current Rank: " << current_rank);

    uint64_t rank_left = r - current_rank;
    DEBUG_LOG("Rank Left: " << rank_left);
    if (rank_left == 0) {
        return (primary_index * L_spacing_);
    }

    uint64_t secondary_index = LM_ratio_ * primary_index;
    uint64_t previous_secondary_index = secondary_index;
    DEBUG_LOG("Secondary Index: " << secondary_index);
    uint64_t secondary_rank = secondary_inventory[secondary_index];
    DEBUG_LOG("Secondary Rank: " << secondary_rank);
    
    int gap = 1;
    while (secondary_rank < rank_left && secondary_index < LM_ratio_ * (primary_index+1)) {
        secondary_index++;
        DEBUG_LOG("Secondary Index: " << secondary_index);
        secondary_rank = secondary_inventory[secondary_index];
        DEBUG_LOG("Secondary Rank: " << secondary_rank);
        if (secondary_inventory[secondary_index-1] != secondary_inventory[secondary_index-2]) {
            gap = 1;
        } else {
            gap++;
        }
        DEBUG_LOG("Gap: " << gap);
    }
    secondary_index = secondary_index < gap ? 0 : secondary_index - gap;
    secondary_rank = secondary_inventory[secondary_index];
    DEBUG_LOG("Secondary Index: " << secondary_index);
    DEBUG_LOG("Secondary Rank: " << secondary_rank);

    rank_left -= secondary_rank;

    uint64_t current_global_index = (primary_index * L_spacing_) + (secondary_index - LM_ratio_ * primary_index) * M_spacing_;
    uint64_t bit_index = bit_search(current_global_index, rank_left);

    return bit_index;
}

uint64_t SimpleSelect::bit_search(uint64_t curr_pos, uint64_t r) {
    DEBUG_LOG("Bit Search, Current Pos: " << curr_pos);
    DEBUG_LOG("Rank Left: " << r);
    uint64_t word_index = std::floor((double)curr_pos / 64);
    DEBUG_LOG("Word Index: " << word_index);
    uint64_t bit_index = curr_pos % 64;
    DEBUG_LOG("Bit Index: " << bit_index);
    if (r == 0) {
        return (word_index * 64) + bit_index;
    }

    uint64_t word = (data_[word_index] & ~(((uint64_t)1)));
    DEBUG_LOG("Masked Word: " << (std::bitset<64>)word);
    word = (word & (uint64_t)~(((uint128_t)1 << (bit_index+1)) - 1));
    DEBUG_LOG("Masked Word: " << (std::bitset<64>)word);

    while (r > 0) {
        uint64_t count = popcount(word);
        if (count >= r) {
            break;
        }
        r -= count;
        word_index++;
        word = data_[word_index];
    }



    for (int i = 0; i < 64; i++) {
        if (word & ((uint64_t)1 << i)) {
            DEBUG_LOG("i: " << i);
            r--;
            DEBUG_LOG("r: " << r);
            if (r == 0) {
                return (word_index * 64) + i;
            }
        }
    }

    DEBUG_LOG("SERIOUS FAILURE IN BIT SEARCH");
    return -1;
}

uint64_t SimpleSelect::rank(uint32_t i) {
    DEBUG_LOG("Rank for Index: " << i);
    int primary_index = std::floor((double)i / L_spacing_);
    DEBUG_LOG("Primary Index: " << primary_index);
    int secondary_index = (LM_ratio_ * primary_index) + std::floor((i - primary_index * L_spacing_) / (double)M_spacing_);
    DEBUG_LOG("Secondary Index: " << secondary_index);
    int bit_index = i - (secondary_index * M_spacing_);
    DEBUG_LOG("Bit Index: " << bit_index);
    int word_index = std::floor((double)i / 64);
    int counted_index = (secondary_index * M_spacing_) % 64;
    int leftover_index = i - counted_index;
    DEBUG_LOG("Counted Index: " << counted_index);
    DEBUG_LOG("Word: " << std::bitset<64>(data_[word_index]));
    uint64_t masked_word = data_[word_index] & ~(((uint128_t)1 << (counted_index+1)) - 1);
    DEBUG_LOG("Masked Word: " << std::bitset<64>(masked_word));
    masked_word = masked_word & (((uint128_t)1 << (bit_index+counted_index+1)) - 1);
    DEBUG_LOG("Masked Word: " << std::bitset<64>(masked_word));
    return primary_inventory[primary_index] + secondary_inventory[secondary_index] + popcount(masked_word);
}

SimpleSpace SimpleSelect::space() {
    return SimpleSpace{};
}

inline uint64_t popcount(uint64_t x) {
    #if defined(__POPCNT__) || defined(_MSC_VER)
        return __builtin_popcountll(x);
    #else
        DEBUG_LOG("Manual Popcount Used");
        x = x - ((x &  0xAAAAAAAAAAAAAAAA) >> 1);
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x = (x + (x >> 4)) &  0x0F0F0F0F0F0F0F0F;
        return x * L8 >> 56;
    #endif
}

void generate_rank_counts(std::vector<uint64_t> &data, std::vector<uint64_t> &counts) {
    // std::cout << data << std::endl;
    size_t basic_blocks = std::ceil((float)data.size() / 8);
    // std::cout << data.size() << std::endl;
    uint64_t current_rank = 0;

    for (int i = 0; i < basic_blocks; i++) {
        // First Level Count
        counts.push_back(current_rank);
        std::bitset<9> current_rank_bin(current_rank);
        // std::cout << "First Count: " << current_rank_bin << std::endl;

        uint64_t second_count = 0;
        // Second Level Count
        uint64_t total_count = 0;
        for (int k = 1; k <= 7; k++) {
            size_t data_index = (8 * i) + k - 1;
            total_count += data_index > data.size() ? 0 : popcount(data[data_index]);
            std::bitset<9> total_count_bin(total_count);
            // std::cout << "Total Count: " << total_count_bin << std::endl;
            second_count |= total_count << (9 * (k - 1));
            std::bitset<64> second_count_bin(second_count);
            // std::cout << "Second Count: " << second_count_bin << std::endl;
        }
        counts.push_back(second_count);
        current_rank += total_count;
        // Account for last word in block for next rank(p) calc
        current_rank += popcount(8 * (i + 1));
    }
}

Rank9::Rank9() {
    return ;
}

void Rank9::build(std::vector<bool> &bool_data) {
    // Convert to uint64_t
    uint64_t n_words = std::ceil((double)bool_data.size() / 64);
    DEBUG_LOG("N Words: " << n_words);
    // std::vector<uint64_t> data_(n_words, 0);
    for (int w = 0; w < n_words; w++) {
        uint64_t word = 0;
        for (int i = 0; i < 64; i++) {
            DEBUG_LOG("Word: " << (std::bitset<64>)word);
            uint64_t index = (64 * w) + i;
            DEBUG_LOG("Index: " << index);
            if (index > bool_data.size()) {
                break;
            }
            
            uint64_t mask = (uint64_t)bool_data[index] << i;
            DEBUG_LOG("Mask: " << (std::bitset<64>)mask);
            // DEBUG_LOG("Word and Mask: " << (word | mask));
            word |= mask;
        }
        data.push_back(word);
    }

    // Generate Counts
    // std::cout << data << std::endl;
    size_t basic_blocks = std::ceil((float)data.size() / 8);
    // std::cout << data.size() << std::endl;
    uint64_t current_rank = 0;

    for (int i = 0; i < basic_blocks; i++) {
        // First Level Count
        counts.push_back(current_rank);
        std::bitset<9> current_rank_bin(current_rank);
        // std::cout << "First Count: " << current_rank_bin << std::endl;

        uint64_t second_count = 0;
        // Second Level Count
        uint64_t total_count = 0;
        for (int k = 1; k <= 7; k++) {
            size_t data_index = (8 * i) + k - 1;
            total_count += data_index > data.size() ? 0 : popcount(data[data_index]);
            std::bitset<9> total_count_bin(total_count);
            // std::cout << "Total Count: " << total_count_bin << std::endl;
            second_count |= total_count << (9 * (k - 1));
            std::bitset<64> second_count_bin(second_count);
            // std::cout << "Second Count: " << second_count_bin << std::endl;
        }
        counts.push_back(second_count);
        current_rank += total_count;
        // Account for last word in block for next rank(p) calc
        current_rank += popcount(data[8 * (i + 1) - 1]);
    }
}

uint64_t Rank9::rank(uint32_t i){
    DEBUG_LOG("Rank for index: " << i);
    for (auto x : data) {
        DEBUG_LOG("Data: " << std::bitset<64>(x));
    }
    DEBUG_LOG("Counts: " << counts);
    size_t w = std::floor((float)i / 64);
    DEBUG_LOG("Word Location: " << w);
    const size_t position = i & 63;
    DEBUG_LOG("Position in Word: " << position);
    size_t s_i = (w & 7);
    DEBUG_LOG("Position in Word: " << position);
    size_t t = (s_i) - 1;

    size_t count_index = std::floor((float)w / 8) * 2;
    DEBUG_LOG("Count Index: " << count_index);
    // std::cout << "Counts: " << counts << std::endl;
    uint64_t first_count = counts[count_index];
    uint64_t second_count = counts[count_index+1];
    DEBUG_LOG("First Count: " << first_count);
    uint64_t second_count_wanted = second_count >> (t + (t >> 60 & 8)) * 9 & (0x1FF);
    DEBUG_LOG("second_count_wanted: " << (std::bitset<64>)second_count_wanted);
    DEBUG_LOG("Second Count: " << second_count_wanted);

    uint64_t count = first_count + second_count_wanted;
    DEBUG_LOG("Count: " << count);

    uint64_t mask = ((uint128_t)1 << (position)) - 1;
    DEBUG_LOG("Mask: " << (std::bitset<64>)mask);
    DEBUG_LOG("Masked Word: " << (std::bitset<64>)(data[w] & mask));
    uint64_t poopcnt = popcount(data[w] & mask);
    DEBUG_LOG("Pop Count From Word: " << poopcnt);
    count += popcount(data[w] & mask);

    return count;
}

SimpleSpace Rank9::space() {
    std::vector<std::pair<std::string, int>> usage = {
        std::make_pair("Data", data.size()*64),
        std::make_pair("Overhead", counts.size()*64),
    };
    return SimpleSpace{usage, (int)(counts.size() * 64 + data.size() * 64)};
}

uint64_t rank9(std::vector<uint64_t> &data, std::vector<uint64_t> &counts, size_t p){
    DEBUG_LOG("Data: " << data);
    DEBUG_LOG("Counts: " << counts);
    size_t w = std::floor((float)p / 64);
    DEBUG_LOG("Word Location: " << w);
    const size_t position = p & 63;
    DEBUG_LOG("Position in Word: " << position);
    size_t s_i = (w & 7);
    DEBUG_LOG("Position in Word: " << position);
    size_t t = (s_i) - 1;

    size_t count_index = std::floor((float)w / 8);
    // std::cout << "Counts: " << counts << std::endl;
    uint64_t first_count = counts[count_index];
    uint64_t second_count = counts[count_index+1];
    DEBUG_LOG("First Count: " << first_count);
    uint64_t second_count_wanted = second_count >> (t + (t >> 60 & 8)) * 9 & (0x1FF);
    DEBUG_LOG("second_count_wanted: " << (std::bitset<64>)second_count_wanted);

    uint64_t count = first_count + second_count_wanted;
    DEBUG_LOG("Count: " << count);

    uint64_t mask = ((uint128_t)1 << (position + 1)) - 1;
    DEBUG_LOG("Mask: " << (std::bitset<64>)mask);
    DEBUG_LOG("Masked Word: " << (std::bitset<64>)(data[w] & mask));
    uint64_t poopcnt = popcount(data[w] & mask);
    DEBUG_LOG("Pop Count From Work: " << poopcnt);
    count += popcount(data[w] & mask);

    return count;
}

std::vector<uint64_t> bool_to_uint64(std::vector<bool> &data) {
    std::vector<uint64_t> data_;
    uint64_t n_words = std::ceil((double)data.size() / 64);
    DEBUG_LOG("N Words: " << n_words);
    // std::vector<uint64_t> data_(n_words, 0);
    for (int w = 0; w < n_words; w++) {
        uint64_t word = 0;
        for (int i = 0; i < 64; i++) {
            DEBUG_LOG("Word: " << (std::bitset<64>)word);
            uint64_t index = (64 * w) + i;
            DEBUG_LOG("Index: " << index);
            if (index > data.size()) {
                break;
            }
            
            uint64_t mask = (uint64_t)data[index] << i;
            DEBUG_LOG("Mask: " << (std::bitset<64>)mask);
            // DEBUG_LOG("Word and Mask: " << (word | mask));
            word |= mask;
        }
        data_.push_back(word);
    }

    return data_;
}