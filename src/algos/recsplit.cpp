#include "algos/recsplit.h"
#include "common/murmurhash.h"
#include "common/utils.h"
#include "common/golomb_rice.h"
#include <cmath>
#include <set>
#include <unordered_set>
#include <iostream>
#include <random>

std::unordered_map<std::string, uint32_t> memo_murmur;


RecSplit::RecSplit(uint32_t bucket_size, uint32_t leaf_size, uint32_t bucket_seed)
: bucket_size_(bucket_size), leaf_size_(leaf_size), splitting_tree_(), bucket_seed_(bucket_seed) {
    
    golomb_rice_parameters_leaf_.resize(leaf_size+1);
    for (uint32_t i = 2; i <= leaf_size; i++) {
        golomb_rice_parameters_leaf_[i] = compute_grp_bijection(i);
    }
    DEBUG_LOG("Golomb Rice Parameter Table: " << golomb_rice_parameters_leaf_);

    grp_table_ = generate_all_grp();
    DEBUG_LOG("Generating GRP Done");
}

void RecSplit::build(const std::vector<std::string> &keys) {
    DEBUG_LOG("Building RecSplit");
    keys_ = keys;
    DEBUG_LOG("Creating Buckets...");

    bucket_count_ = ceil((float)keys_.size() / (float)bucket_size_);
    create_buckets();

    for (std::vector<std::string> bucket : buckets_) {
        DEBUG_LOG("Bucket being sent to Split(): " << bucket);
        split(bucket);
    }

    splitting_tree_select_.unary.build(splitting_tree_.unary, 128, 8);
    splitting_tree_select_.fixed = splitting_tree_.fixed;
}

HashFunctionSpace RecSplit::space() {
    std::vector<std::pair<std::string, int>> space_usage;
    int splitting_tree_overhead = (sizeof(splitting_tree_) + sizeof(splitting_tree_.fixed) + sizeof(splitting_tree_.unary)) * 8;
    space_usage.push_back(std::make_pair("Splitting Tree Overhead", splitting_tree_overhead));
    space_usage.push_back(std::make_pair("Splitting Tree Fixed", splitting_tree_.fixed.size()));
    space_usage.push_back(std::make_pair("Splitting Tree Unary", splitting_tree_.unary.size()));
    // int total_splitting_tree = splitting_tree_overhead + splitting_tree_.fixed.size() + splitting_tree_.unary.size();
    int total_splitting_tree = splitting_tree_.fixed.size() + splitting_tree_.unary.size();
    space_usage.push_back(std::make_pair("Total Splitting Tree", total_splitting_tree));

    space_usage.push_back(std::make_pair("Bucket Node Prefixes", elias_fano_space(bucket_node_prefixes_ef_)));
    space_usage.push_back(std::make_pair("Bucket Unary Prefixes", elias_fano_space(bucket_unary_prefixes_ef_)));
    space_usage.push_back(std::make_pair("Bucket Fixed Prefixes", elias_fano_space(bucket_fixed_prefixes_ef_)));
    int total_bucket_prefixes = elias_fano_space(bucket_node_prefixes_ef_) + elias_fano_space(bucket_unary_prefixes_ef_) + elias_fano_space(bucket_fixed_prefixes_ef_);
    space_usage.push_back(std::make_pair("Total Bucket Prefixes", total_bucket_prefixes));

    int total_bits = total_bucket_prefixes + total_splitting_tree;
    double bits_per_key = total_bits / (double)keys_.size();
    return HashFunctionSpace{space_usage, total_bits, bits_per_key};
}   


// void RecSplit::space() {
//     uint32_t splitting_tree_size = sizeof(splitting_tree_) * 8;
//     uint32_t splitting_tree_fixed_size = sizeof(splitting_tree_.fixed) * 8;
//     uint32_t splitting_tree_unary_size = sizeof(splitting_tree_.unary) * 8;

//     std::cout << "###################### RecSplit Space Analysis ######################" << std::endl;
//     std::cout << "Total Keys: " << keys_.size() << std::endl;
//     std::cout << "Total Buckets: " << bucket_count_ << std::endl;

//     uint32_t splitting_tree_fixed_bits = splitting_tree_.fixed.size();
//     uint32_t splitting_tree_unary_bits = splitting_tree_.unary.size();

//     uint32_t total_splitting_tree_bits = splitting_tree_fixed_size + splitting_tree_unary_size + \
//              splitting_tree_fixed_bits + splitting_tree_unary_bits;

//     std::cout << "************************ Splitting Tree Data ************************" << std::endl;
//     std::cout << "Unary Component Overhead: " << splitting_tree_unary_size << " bits" << std::endl;
//     std::cout << "Fixed Component Overhead: " << splitting_tree_fixed_size << " bits" << std::endl;
//     std::cout << "Unary Bits: " << splitting_tree_unary_bits << " bits" << std::endl;
//     std::cout << "Fixed Bits: " << splitting_tree_fixed_bits << " bits" << std::endl;
//     std::cout << "====> Total Splitting Tree: " << special_string(std::to_string(total_splitting_tree_bits), ConsoleColour::Bold) << " bits" << std::endl;

    
//     std::cout << "*************************** Bucket Data ****************************" << std::endl;
// }

// uint64_t RecSplit::space_bits() {
//     uint64_t total_bits = splitting_tree_.fixed.size() + splitting_tree_.unary.size();
//     total_bits += bucket_node_prefixes_.size() * 32;
//     total_bits += bucket_unary_prefixes_.size() * 32;
//     total_bits += bucket_fixed_prefixes_.size() * 32;
//     total_bits += bucket_sizes_.size() * 32;

//     return total_bits;
// }

uint32_t RecSplit::hash(const std::string &key) {
    DEBUG_LOG(splitting_tree_);
    DEBUG_LOG("Finding Hash for Key: " << key);
    int n_buckets = buckets_.size();

    // Extract elias fano data
    std::vector<uint32_t> bucket_node_prefixes = elias_fano_decode(bucket_node_prefixes_ef_, n_buckets+1);
    std::vector<uint32_t> bucket_unary_prefixes = elias_fano_decode(bucket_unary_prefixes_ef_, n_buckets+1);
    std::vector<uint32_t> bucket_fixed_prefixes = elias_fano_decode(bucket_fixed_prefixes_ef_, n_buckets+1);
    DEBUG_LOG("Decoded Bucket Node Prefixes: " << bucket_node_prefixes);
    DEBUG_LOG("Decoded Bucket Unary Prefixes: " << bucket_unary_prefixes);
    DEBUG_LOG("Decoded Bucket Fixed Prefixes: " << bucket_fixed_prefixes);
    DEBUG_LOG("Bucket Node Prefixes: " << bucket_node_prefixes_);
    DEBUG_LOG("Bucket Unary Prefixes: " << bucket_unary_prefixes_);
    DEBUG_LOG("Bucket Fixed Prefixes: " << bucket_fixed_prefixes_);

    // Bucket finding and starting the unary pointer
    uint32_t bucket = assign_bucket(key, buckets_.size());


    size_t node_count = bucket_node_prefixes[bucket];
    size_t ones_count = 0;
    ones_count = bucket_unary_prefixes[bucket];
    size_t unary_pointer_select = splitting_tree_select_.unary.select(bucket_unary_prefixes[bucket]);

    // unless the first item, need to start on a 0
    if (bucket_unary_prefixes_[bucket] > 0) {
        unary_pointer_select++;
    }
    DEBUG_LOG("Unary Pointer Select: " << unary_pointer_select);

    size_t fixed_pointer = bucket_fixed_prefixes[bucket];
    size_t size = bucket_sizes_[bucket];
    DEBUG_LOG("Bucket Index: " << bucket);
    DEBUG_LOG("Bucket Size: " << size);
    DEBUG_LOG("Node Count: " << node_count);

    while (size > leaf_size_) {
        SubtreeData subtree_data = grp_table_[size][leaf_size_-2];
        DEBUG_LOG("Current Size: " << size);
        DEBUG_LOG("Subtree Data: " << subtree_data);
        DEBUG_LOG("Fixed Pointer Before: " << fixed_pointer);
        DEBUG_LOG("Unary Pointer Before: " << unary_pointer_select);
        std::vector<bool> fixed, unary;
        std::vector<bool> unary_select;     
        while (splitting_tree_.unary[unary_pointer_select] != 1) {
            unary_select.push_back(splitting_tree_.unary[unary_pointer_select]);
            unary_pointer_select++;
        }
        unary_pointer_select++;


        unary.push_back(1);
        unary_select.push_back(1);

        for (size_t i = 0; i < subtree_data.parameter; i++) {
            fixed.push_back(splitting_tree_.fixed[fixed_pointer]); 
            fixed_pointer++;
        }
        DEBUG_LOG("Fixed Pointer After: " << fixed_pointer);
        DEBUG_LOG("Unary Pointer After: " << unary_pointer_select);
        DEBUG_LOG("Fixed Data Extracted: " << fixed);
        DEBUG_LOG("Unary Data Extracted: " << unary);

        uint32_t seed = golomb_rice_decode(GolombEncodedData{fixed, unary_select}, subtree_data.parameter);
        DEBUG_LOG("Split Seed: " << seed);

        FanoutData fanout_data = calculate_fanout(size, leaf_size_);
        DEBUG_LOG("Fanout Data: " << fanout_data);
        uint32_t split_index = map_key_to_split(key, seed, fanout_data);
        DEBUG_LOG("Split Index: " << split_index);

        uint32_t nodes_to_skip = 0;
        for (int i = 0; i < split_index; i++) {
            SubtreeData subtree_data_skip = grp_table_[fanout_data.part_sizes[i]][leaf_size_-2];
            DEBUG_LOG("Subtree Data to Skip: " << subtree_data_skip);
            fixed_pointer += subtree_data_skip.fixed_code_length;
            nodes_to_skip += subtree_data_skip.unary_code_length;
            node_count += subtree_data_skip.nodes;
        }

        DEBUG_LOG("Ones Counts: " << ones_count);
        DEBUG_LOG("Nodes to Skip: " << nodes_to_skip);
        ones_count += nodes_to_skip + 1;
        unary_pointer_select = splitting_tree_select_.unary.select(ones_count) + 1;

        DEBUG_LOG("Unary Pointer: " << unary_pointer_select);
        DEBUG_LOG("Unary Pointer Select: " << unary_pointer_select);

        DEBUG_LOG("Node Count: " << node_count);

        size = fanout_data.part_sizes[split_index];
    }

    SubtreeData subtree_data = grp_table_[size][leaf_size_-2];
    DEBUG_LOG("Subtree Data at Bijection: " << subtree_data);
    std::vector<bool> fixed, unary;
    DEBUG_LOG("Fixed Pointer Before: " << fixed_pointer);
    DEBUG_LOG("Unary Pointer Before: " << unary_pointer_select);
    while (splitting_tree_.unary[unary_pointer_select] != 1) {
        unary.push_back(splitting_tree_.unary[unary_pointer_select]);
        unary_pointer_select++;
    }
    unary_pointer_select++;
    unary.push_back(1);

    for (size_t i = 0; i < subtree_data.parameter; i++) {
        fixed.push_back(splitting_tree_.fixed[fixed_pointer]); 
        fixed_pointer++;
    }

    DEBUG_LOG("Fixed Pointer After: " << fixed_pointer);
    DEBUG_LOG("Unary Pointer After: " << unary_pointer_select);
    DEBUG_LOG("Fixed Data Extracted: " << fixed);
    DEBUG_LOG("Unary Data Extracted: " << unary);
    uint32_t seed = golomb_rice_decode(GolombEncodedData{fixed, unary}, subtree_data.parameter);
    DEBUG_LOG("Bijection Seed: " << seed);
    uint32_t bijection_hash = murmur32(key, seed) % size;
    DEBUG_LOG("Bijection Hash: " << bijection_hash);
    uint32_t hash = node_count + 1 + bijection_hash;
    DEBUG_LOG(" ================ Found Hash: " << hash);
    DEBUG_LOG("Raw Splitting Tree: " << splitting_tree_raw_);
    return hash;
}

void RecSplit::split(const std::vector<std::string> &keys) {
    if (keys.size() <= leaf_size_) {
        if (keys.size() == 1) {
            return;
        }
        uint32_t bijection_seed = find_bijection(keys);
        // uint32_t bijection_seed = find_bijection_random(keys);
        append_to_splitting_tree(bijection_seed, golomb_rice_parameters_leaf_[keys.size()]);
        return;
    }

    FanoutData fanout_data = calculate_fanout(keys.size(), leaf_size_);
    uint32_t fanout = fanout_data.fanout;
    std::vector<uint32_t> part_sizes = fanout_data.part_sizes;
    DEBUG_LOG("Keys: " << keys);
    DEBUG_LOG("Fanout: " << fanout);

    DEBUG_LOG("Part Sizes: " << part_sizes);
    uint32_t seed = find_splitting(keys, fanout_data);

    const uint32_t golomb_rice_param = compute_grp_buckets(part_sizes);
    append_to_splitting_tree(seed, golomb_rice_param);

    std::vector<std::vector<std::string>> keys_split(fanout);
    for (std::string key : keys) {
        uint32_t hash = murmur32(key, seed);
        keys_split[map_key_to_split(key, seed, fanout_data)].push_back(key);
    }

    for (std::vector<std::string> key_part : keys_split) {
        split(key_part);
    }
}

void RecSplit::append_to_splitting_tree(const uint32_t &data, const uint32_t golomb_rice_param) {
    DEBUG_LOG("Value: " << data);
    DEBUG_LOG("Golomb Rice Param: " << golomb_rice_param);
    
    GolombEncodedData encoded_data = golomb_rice_encode(
        data, 
        golomb_rice_param
    );
    DEBUG_LOG("Fixed: " << encoded_data.fixed);
    DEBUG_LOG("Unary: " << encoded_data.unary);
    splitting_tree_raw_.push_back(data);
    append_vector_to_vector(splitting_tree_.fixed, encoded_data.fixed);
    append_vector_to_vector(splitting_tree_.unary, encoded_data.unary);
}

void RecSplit::create_buckets() {
    buckets_.resize(bucket_count_);
    bucket_sizes_.resize(bucket_count_, 0);
    for (int i = 0; i < keys_.size(); i++) {
        // Perform the bucket assigment
        uint32_t bucket = assign_bucket(keys_[i], bucket_count_);
        buckets_[bucket].push_back(keys_[i]);
        bucket_sizes_[bucket] += 1;
    }

    DEBUG_LOG("Bucket Sizes:" << bucket_sizes_);
    DEBUG_LOG("Buckets: ");
    for (auto bucket : buckets_) {
        DEBUG_LOG(bucket);
    }

    // uint32_t bucket_index = 0;
    // uint32_t bucket_offset = 0;
    // for (auto bucket_size : bucket_sizes_) {
    //     bucket_prefixes_.push_back(bucket_index);
    //     bucket_index += bucket_size;
    //     bucket_offsets_.push_back(bucket_offset);
    //     bucket_offset += grp_table_[bucket_size][leaf_size_-2].fixed_code_length;
    // }
    // bucket_prefixes_.push_back(bucket_index);
    // bucket_offsets_.push_back(bucket_offset);

    uint32_t bucket_node_prefix = 0, bucket_unary_prefix = 0, bucket_fixed_prefix = 0;
    for (auto bucket_size : bucket_sizes_) {
        bucket_node_prefixes_.push_back(bucket_node_prefix);
        bucket_unary_prefixes_.push_back(bucket_unary_prefix);
        bucket_fixed_prefixes_.push_back(bucket_fixed_prefix);
        DEBUG_LOG("Bucket Node Prefix: " << bucket_node_prefix);
        DEBUG_LOG("Bucket Unary Prefix: " << bucket_unary_prefix);
        DEBUG_LOG("Bucket Fixed Prefix: " << bucket_fixed_prefix);

        SubtreeData subtree_data = grp_table_[bucket_size][leaf_size_-2];

        bucket_node_prefix += subtree_data.nodes;
        bucket_unary_prefix += subtree_data.unary_code_length;
        bucket_fixed_prefix += subtree_data.fixed_code_length;
    }
    bucket_node_prefixes_.push_back(bucket_node_prefix);
    bucket_unary_prefixes_.push_back(bucket_unary_prefix);
    bucket_fixed_prefixes_.push_back(bucket_fixed_prefix);
    
    bucket_node_prefixes_ef_ = elias_fano_encode(bucket_node_prefixes_);
    bucket_unary_prefixes_ef_ = elias_fano_encode(bucket_unary_prefixes_);
    bucket_fixed_prefixes_ef_ = elias_fano_encode(bucket_fixed_prefixes_);

    DEBUG_LOG("Bucket Node Prefixes: " << bucket_node_prefixes_);
    DEBUG_LOG("Bucket Unary Prefixes: " << bucket_unary_prefixes_);
    DEBUG_LOG("Bucket Fixed Prefixes: " << bucket_fixed_prefixes_);
}



uint32_t RecSplit::assign_bucket(const std::string &key, uint32_t bucket_count) {
    uint32_t hash = murmur32(key, bucket_seed_) >> 16;
    uint32_t bucket = floor((hash * bucket_count) >> 16);

    return bucket;
}

uint32_t find_bijection(const std::vector<std::string> &keys) {
    uint32_t seed = 0;
    while (true) {
        bool bijection = true;
        std::bitset<30> used(0);
        for (const std::string &key : keys) {
            uint32_t hash = murmur32(key, seed) % keys.size();
            // DEBUG_LOG("FINDING BIJECTION -- Key: " << key << " Hash: " << hash);
            if (used[hash]) {
                bijection = false;
                // DEBUG_LOG("Bijection Failed");
                break;
            }
            used[hash] = 1;
        }
        if (bijection) {
            return seed;
        }
        seed++;
    }
}

uint32_t find_splitting(const std::vector<std::string> &keys, const FanoutData &fanout_data) {
    uint32_t seed = 0;
    while (true) {
        DEBUG_LOG("Seed: " << seed);
        std::vector<uint32_t> counts(fanout_data.fanout);
        for (const std::string &key : keys) {
            counts[map_key_to_split(key, seed, fanout_data)] +=1;
        }

        DEBUG_LOG("Actual Counts: " << counts);
        if (counts == fanout_data.part_sizes) {
            return seed;
        }

        seed++;
    }
}

uint32_t map_key_to_split(const std::string &key, const uint32_t &seed, const FanoutData &fanout_data) {
    uint32_t hash = murmur32(key, seed);
    uint32_t index = hash % fanout_data.size;

    int current = 0;
    for (int i = 0; i < fanout_data.fanout; i++) {
        current += fanout_data.part_sizes[i];
        if (index < current) {
            return i;
        }
    }

    // throw "Map Key to Split function didn't work...";
}

FanoutData calculate_fanout(uint32_t size, uint32_t leaf_size) { 
    uint32_t lower_aggr = leaf_size * (int32_t)std::max(2, (int)std::ceil(0.35 * leaf_size + 0.5));
    uint32_t upper_aggr = lower_aggr * (leaf_size >= 7 ? (uint32_t)std::ceil(0.21 * leaf_size + 0.9) : 2);
    uint32_t fanout, part_size;
    // DEBUG_LOG("Upper Aggr: " << upper_aggr);
    // DEBUG_LOG("Lower Aggr: " << lower_aggr);
    if (size > upper_aggr) {
        fanout = 2;
        part_size = upper_aggr * ((size / 2 + upper_aggr - 1) / upper_aggr);
    } else if (size > lower_aggr) {
        fanout = (size + lower_aggr - 1) / lower_aggr;
        part_size = lower_aggr;
    } else {
        part_size = leaf_size;
        fanout = (size + leaf_size - 1) / leaf_size;
    }

    std::vector<uint32_t> part_sizes(fanout);
    int total = 0;
    for (int i = 0; i < fanout - 1; i++) {
        part_sizes[i] = part_size;
        total+=part_size;
    }
    part_sizes[fanout - 1] = size - total;

    return FanoutData{size, fanout, part_sizes};
}

std::vector<std::vector<SubtreeData>> generate_all_grp() {
    // Generate up to bucket_size 3000 and leaf size up to 24?
    const uint32_t max_bucket_size = 3000;
    const uint32_t max_leaf_size = 24;
    std::vector<std::vector<SubtreeData>> grp_table(max_bucket_size, std::vector<SubtreeData>(max_leaf_size-1));
    for (size_t bucket_size = 1; bucket_size < max_bucket_size; bucket_size++) {
        for (size_t leaf_size = 2; leaf_size < max_leaf_size + 1; leaf_size++) {
            size_t leaf_index = leaf_size - 2;
            
            if (bucket_size <= leaf_size) { 
                uint8_t param = compute_grp_bijection(bucket_size);
                grp_table[bucket_size][leaf_index] = SubtreeData{
                    param, 
                    (uint16_t)bucket_size, 
                    param,
                    (uint16_t)(bucket_size > 1 ? 1 : 0)
                };
            } else {
                FanoutData fanout = calculate_fanout(bucket_size, leaf_size);
                uint8_t param = compute_grp_buckets(fanout.part_sizes);
                uint16_t nodes = 0;
                uint16_t fixed_code_length = param;
                uint16_t unary_code_length = 1;
                for (auto part_size : fanout.part_sizes) {
                    nodes += grp_table[part_size][leaf_index].nodes;
                    fixed_code_length += grp_table[part_size][leaf_index].fixed_code_length;
                    unary_code_length += grp_table[part_size][leaf_index].unary_code_length;
                }
    
                grp_table[bucket_size][leaf_index] = SubtreeData{
                    param, nodes, fixed_code_length, unary_code_length
                };
            }

            // std::cout << "Bucket Size: " << bucket_size << " Leaf Size: " << leaf_index << std::endl;
            // std::cout << grp_table[bucket_size][leaf_index] << std::endl;
        }
    }

    return grp_table;
}