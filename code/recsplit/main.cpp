#include <iostream>
#include <random>
#include <fstream>
#include <bit>
#include <vector>
#include <set>
#include <numeric>
using namespace std;

constexpr int LEAF_SIZE = 2;
float GOLDEN_RATIO = (sqrt(5) + 1) / 2;
// string keys[] = {"Hello", "World", "RecSplit", "Nelson", "Horatio", 
//                     "Napoleon", "Alexander", "Victory", "Great", "Nile",
//                     "Vincent", "Dock", "Longbow", "Whistle", "Thyme"};
constexpr int constants[] = {1, 2, 3, 5, 8, 13, 21}; 
bool little_endian = endian::native == endian::little;

// constexpr float computer_golden_ratio() {
//     return (sqrt(5) + 1) / 2;
// }
// constexpr float golden_ratio = (sqrt(5) + 1) / 2;

uint32_t ROL32(uint32_t k, uint32_t n) {
    uint32_t l = k << n;
    uint32_t r = k >> (32 - n);
    return l | r;
}

// https://en.wikipedia.org/wiki/MurmurHash#Algorithm
uint32_t murmur_hash_3_uint32(uint32_t seed, string key) {
    constexpr uint32_t c1 = 0xcc9e2d51;
    constexpr uint32_t c2 = 0x1b873593;
    constexpr int r1 = 15;
    constexpr int r2 = 13;
    constexpr int m = 5;
    constexpr uint32_t n = 0xe6546b64;

    uint32_t hash = seed;

    unsigned char buffer[key.length()];
    memcpy(buffer, key.data(), key.length());
    // cout << buffer;
    // cout << "\n";
    // cout << key.length();
    // cout << "\n";

    for (int i = 0; i <= key.length() - 4; i += 4) {
        if (key.length() < 4) {
            break;
        }
        uint32_t chunk = 0;
        memcpy(&chunk, buffer + i, 4);

        uint32_t k = chunk;
        k *= c1;
        k = ROL32(k, r1);
        k *= c2;

        hash ^= k;
        hash = ROL32(hash, r2);
        hash = (hash * m) + n;
    }

    int leftover = key.length() % 4;
    if (leftover != 0) {
        uint32_t chunk = 0;
        std::memcpy(&chunk, buffer + (key.length() - leftover), leftover);

        if (!little_endian) {
            chunk = __builtin_bswap32(chunk);
        }

        chunk *= c1;
        chunk = ROL32(chunk, r1);
        chunk *= c2;

        hash ^= chunk;
    }

    hash ^= key.length();

    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

uint32_t hash_key(string key) {
    hash<string> hasher;
    return hasher(key);
}

int highest_bit(int x) {
    int index = -1;
    while (x > 0) {
        x >>= 1;
        index++;
    }
    return index;
}

int nearest_integer_log(int x) {
    int y = x + (x >> 1);

    return highest_bit(y);
}

bool test_murmur3_32() {
    string test_keys[4] = {"Hello", "World", "Horatio", "Nelson"};
    uint32_t expected_hash[4] = {1466740371, 3789324275, 2689083821, 2244112232};

    bool test = true;
    for (int i = 0; i < 4; i++) {
        test &= (murmur_hash_3_uint32(42, test_keys[i]) == expected_hash[i]);
    }

    return test;
}

template <typename T>
void print_vector(const vector<T>& vec) {
    for (const auto& element : vec) {
        cout << element << " ";
    }
    cout << endl;
}

uint32_t assign_bucket(string key, uint32_t bucket_seed, uint32_t bucket_count) {
    uint32_t hash = murmur_hash_3_uint32(bucket_seed, key) >> 16;
    uint32_t bucket = floor((hash * bucket_count) >> 16);

    return bucket;
}

vector<uint32_t> assign_buckets(vector<string> keys, uint32_t bucket_seed, uint32_t bucket_size) {
    vector<uint32_t> bucket_assignments(keys.size());
    uint32_t bucket_count = ceil(keys.size() / bucket_size);
    for (int i = 0; i < keys.size(); i++) {
        bucket_assignments[i] = assign_bucket(keys[i], bucket_seed, bucket_count);
    } 

    return bucket_assignments;
}

struct bucket_data {
    vector<vector<string>> buckets;
    vector<uint32_t> bucket_sizes;
};

bucket_data create_buckets(vector<string> keys, uint32_t bucket_size, uint32_t bucket_seed) {
    uint32_t bucket_count = ceil((float)keys.size() / (float)bucket_size);

    vector<vector<string>> buckets(bucket_count);
    vector<uint32_t> bucket_sizes(bucket_count);
    for (int i = 0; i < keys.size(); i++) {
        // extract top 16 bits of the 32 bit hash
        uint32_t hash = murmur_hash_3_uint32(bucket_seed, keys[i]) >> 16;
        // Perform the bucket assigment
        uint32_t bucket = floor((hash * bucket_count) >> 16);
        buckets[bucket].push_back(keys[i]);
        bucket_sizes[bucket] += 1;
    } 

    return bucket_data{buckets, bucket_sizes};
    
}

uint64_t factorial(int n) {
    uint64_t ans = 1;
    while (n > 0) {
        ans *= n;
        n--;
    }

    return ans;
} 

void test_assign_buckets() {
    string addressFilePath = "addresses.txt";

    std::ifstream inputFile(addressFilePath);
    vector<string> addresses;
    string line;

    while (getline(inputFile, line)) {
        addresses.push_back(line);
    }

    int bucket_size = 50;
    inputFile.close();
    uint32_t bucket_count = ceil(addresses.size() / bucket_size);
    
    vector<int> counts(bucket_count);
    for (int i = 0; i < 10000; i++) {
        vector<uint32_t> bucket_assignments = assign_buckets(addresses, i, bucket_size);
        for (int j = 0; j < addresses.size(); j++) {
            counts[bucket_assignments[j]]++;
        }
    }

    print_vector(counts);
    uint64_t top = factorial(addresses.size() * 100);
    uint64_t bottom = 1;
    for (uint16_t i = 0; i < counts.size(); i++){
        bottom *= factorial(counts[i]);
    }

    float prob = top/bottom;
    prob *= (pow((1/bucket_count), (addresses.size() * 100))) * 100;
    cout << prob;
    cout<<endl;

}

uint32_t find_bijection(vector<string> keys) {
    uint32_t seed = 0;
    while (true) {
        bool bijection = true;
        set<uint32_t> indexes;
        print_vector(keys);
        for (string key : keys) {
            uint32_t hash = murmur_hash_3_uint32(seed, key) % keys.size();
            cout << "Hash for Key: " << key << " " << hash << endl;
            if (indexes.contains(hash)) {
                bijection = false;
                break;
            }

            indexes.insert(hash);
        }

        if (bijection) {
            return seed;
        }

        // cout << seed << endl;
        seed++;
    }
}

// https://github.com/thomasmueller/minperf/blob/master/src/test/java/org/minperf/simple/recsplit.md
void split(vector<string> keys, uint32_t leaf_size, vector<uint32_t>& splitting_tree, uint32_t depth) {
    cout << "Key Size: " << keys.size() << endl;
    if (keys.size() <= leaf_size) {
        if (keys.size() == 1) {
            return;
        }
        uint32_t bijection_index = find_bijection(keys);
        splitting_tree.push_back(bijection_index);
        return;
    }

    uint32_t parts;
    if (depth == 0) {
        parts = max(2, (int)ceil(0.35 * leaf_size + 0.5));
    } else if (depth == 1 && leaf_size >= 7) {
        parts = ceil(0.21 * leaf_size + 0.9);
    } else {
        parts = 2;
    }

    cout << "Parts: " << parts << endl;
    cout << "Depth: " << depth << endl;
    vector<uint32_t> expected_counts(parts);
    uint16_t bigger = keys.size() % parts;

    // cout << "Parts: " << parts << endl;
    // cout << "Keys Size: " << keys.size() << endl;
    // cout << "N Bigger: " << bigger << endl;
    // cout << (float)keys.size() / (float)parts << endl;
    // cout << floor(keys.size() / parts)

    for (int i = 0; i < parts; i++) {
        if (i < bigger) {
            expected_counts[i] = ceil((float)keys.size() / (float)parts);
        } else {
            expected_counts[i] = floor((float)keys.size() / (float)parts);
        }
    }


    uint32_t seed = 0;
    while (true) {
        vector<uint32_t> counts(parts);
        for (string key : keys) {
            uint32_t hash = murmur_hash_3_uint32(seed, key);
            counts[hash % parts] += 1;
        }

        // print_vector(expected_counts);
        // print_vector(counts);
        if (counts == expected_counts) {
            break;
        }

        seed++;
    }

    splitting_tree.push_back(seed);

    vector<vector<string>> keys_split(parts);
    for (string key : keys) {
        uint32_t hash = murmur_hash_3_uint32(seed, key);
        keys_split[hash % parts].push_back(key);
    }

    for (vector<string> key_part : keys_split) {
        split(key_part, leaf_size, splitting_tree, depth+1);
    }
}


struct recsplit_data {
    vector<uint32_t> splitting_tree;
    vector<uint32_t> bucket_prefixes;
    uint32_t n_keys;
    uint32_t leaf_size;
    uint32_t bucket_seed;
    uint32_t bucket_size;
};

int lookup(string key, recsplit_data data) {
    auto [splitting_tree, bucket_prefixes, n_keys, leaf_size, bucket_seed, bucket_size] = data;

    int depth = 0;

    uint32_t bucket_count = ceil((float)n_keys / (float)bucket_size);
    uint32_t bucket = assign_bucket(key, bucket_seed, bucket_count);

    uint32_t index = bucket_prefixes[bucket];
    uint32_t tree_index = 0;
    uint32_t part_size = bucket_prefixes[bucket+1] - bucket_prefixes[bucket];

    while (true) {
        if (part_size <= leaf_size) {
            if (part_size == 1) {
                return index;
            }
            uint32_t bijection_index = murmur_hash_3_uint32(splitting_tree[tree_index], key) % part_size;
            return index + bijection_index;
        }

        uint32_t parts;
        if (depth == 0) {
            parts = max(2, (int)ceil(0.35 * leaf_size + 0.5));
        } else if (depth == 1 && leaf_size >= 7) {
            parts = ceil(0.21 * leaf_size + 0.9);
        } else {
            parts = 2;
        }
        vector<uint32_t> expected_counts(parts);
        uint16_t bigger = part_size % parts;

        for (int i = 0; i < parts; i++) {
            if (i < bigger) {
                expected_counts[i] = ceil((float)part_size / (float)parts);
            } else {
                expected_counts[i] = floor((float)part_size / (float)parts);
            }
        }

        uint32_t part_index = murmur_hash_3_uint32(splitting_tree[tree_index], key) % parts;
        for (int i = 0; i < part_index; i++) {
            index += expected_counts[i];
        }
        part_size = expected_counts[part_index];
        depth++;
    }
}

int lookup2(string key, recsplit_data data) {
    auto [splitting_tree, bucket_prefixes, n_keys, leaf_size, bucket_seed, bucket_size] = data;
    cout << "Lookup for Key: " << key << endl;
    cout << "Bucket Prefixes: ";
    print_vector(bucket_prefixes);

    // Step 1: Assign the key to a bucket
    uint32_t bucket_count = ceil((float)n_keys / (float)bucket_size);
    uint32_t bucket = assign_bucket(key, bucket_seed, bucket_count);
    cout << "Bucket Assigned: " << bucket << endl;

    // Step 2: Initialize variables for tree traversal
    uint32_t index = bucket_prefixes[bucket];
    uint32_t tree_index = 0;
    uint32_t part_size = bucket_prefixes[bucket + 1] - bucket_prefixes[bucket];
    cout << "Part Size Calculated: " << part_size << endl;

    while (true) {
        // Base case: If the part size is small enough, return the index
        if (part_size <= leaf_size) {
            if (part_size == 1) {
                return index;
            }
            uint32_t bijection_index = murmur_hash_3_uint32(splitting_tree[tree_index], key) % part_size;
            cout << "Bijection Index: " << bijection_index << endl;
            cout << "Index: " << index << endl;
            return bijection_index + index;
        }

        // Step 3: Determine the number of parts and their sizes
        uint32_t parts;
        if (part_size >= 7) {
            parts = ceil(0.21 * leaf_size + 0.9);
        } else {
            parts = 2;
        }

        vector<uint32_t> expected_counts(parts);
        uint16_t bigger = part_size % parts;

        for (uint32_t i = 0; i < parts; i++) {
            if (i < bigger) {
                expected_counts[i] = ceil((float)part_size / (float)parts);
            } else {
                expected_counts[i] = floor((float)part_size / (float)parts);
            }
        }

        // Step 4: Determine which part the key belongs to
        uint32_t part_index = murmur_hash_3_uint32(splitting_tree[tree_index], key) % parts;

        // Update the index and part size
        for (uint32_t i = 0; i < part_index; i++) {
            index += expected_counts[i];
        }
        part_size = expected_counts[part_index];

        // Move to the next level in the splitting tree
        tree_index++;
    }
}

uint32_t leaf_size = 2;
uint32_t bucket_size = 4;
uint32_t bucket_seed = 1;

recsplit_data recsplit(vector<string> keys) {
    auto [buckets, bucket_sizes] = create_buckets(keys, bucket_size, bucket_seed);
    print_vector(bucket_sizes);
    // vector<vector<uint32_t>> splitting_trees(ceil(keys.size() / bucket_size));
    // for (vector<string> bucket : buckets) {
    //     print_vector(bucket);
    //     vector<uint32_t> splitting_tree;
    //     split(bucket, leaf_size, splitting_tree, 0);
    //     print_vector(splitting_tree);

    //     splitting_trees.push_back(splitting_tree);
    // }

    vector<uint32_t> splitting_tree;
    for (vector<string> bucket : buckets) {
        print_vector(bucket);
        split(bucket, leaf_size, splitting_tree, 0);
    }

    vector<uint32_t> bucket_prefixes(bucket_sizes.size()+1);
    uint32_t index = 0;
    for (int i = 0; i < bucket_sizes.size(); i++) {
        bucket_prefixes[i] = index;
        index += bucket_sizes[i];
    }
    bucket_prefixes[bucket_sizes.size()] = index;
    

    return recsplit_data{splitting_tree, bucket_prefixes, (uint32_t)keys.size(), leaf_size, bucket_seed, bucket_size};
}

void run_test_keys() { 
    leaf_size = 2;
    bucket_size = 4;
    bucket_seed = 5;
    
    vector<string> test_keys = {"Hello", "World", "RecSplit", "Nelson", "Horatio", 
                    "Napoleon", "Alexander", "Victory", "Great", "Nile",
                    "Vincent", "Dock", "Longbow", "Whistle", "Thyme"};

    recsplit_data data = recsplit(test_keys);
    print_vector(data.splitting_tree);
    for (int i = 0; i < 15; i++) {
        cout << "MPHF Hash: " << lookup2(test_keys[i], data) << endl;
    }
}

void run_test_words() { 
    leaf_size = 8;
    bucket_size = 600;

    string wordFilePath = "words.txt";
    std::ifstream inputFile(wordFilePath);
    vector<string> words;
    string line;

    while (getline(inputFile, line)) {
        words.push_back(line);
    }

    inputFile.close();
    recsplit_data data = recsplit(words);
    print_vector(data.splitting_tree);
    for (int i = 0; i < 15; i++) {
        cout << lookup2(words[i], data) << endl;
    }
}

int main() {
    cout << "Murmur3 32 hash test: ";
    cout << (test_murmur3_32() ? "Passed" : "!!! FAILED !!!");
    cout << endl;

    // test_assign_buckets();

    run_test_keys();
    // run_test_words();

    return 0;
}
