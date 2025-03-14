#include <iostream>
#include <random>
#include <fstream>
#include <bit>
#include <vector>
#include <set>
#include <numeric>
#include <cstring>
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

uint32_t ROL32(uint32_t k, uint32_t n)
{
    uint32_t l = k << n;
    uint32_t r = k >> (32 - n);
    return l | r;
}

// https://en.wikipedia.org/wiki/MurmurHash#Algorithm
uint32_t murmur_hash_3_uint32(uint32_t seed, string key)
{
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

    for (int i = 0; i <= key.length() - 4; i += 4)
    {
        if (key.length() < 4)
        {
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
    if (leftover != 0)
    {
        uint32_t chunk = 0;
        std::memcpy(&chunk, buffer + (key.length() - leftover), leftover);

        if (!little_endian)
        {
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

struct GolombEncodedData
{
    vector<bool> fixed;
    vector<bool> unary;
};

// https://michaeldipperstein.github.io/rice.html
GolombEncodedData golomb_rice_encoding(uint32_t value, uint32_t r)
{
    uint32_t quotient = value >> r;
    uint32_t remainder = value & ((1 << r) - 1);

    vector<bool> fixed;
    vector<bool> unary;

    for (int i = 0; i < quotient; i++)
    {
        unary.push_back(true);
    }
    unary.push_back(false);

    for (int i = r - 1; i >= 0; i--)
    {
        fixed.push_back((remainder >> i) & 1);
    }

    return GolombEncodedData{fixed, unary};
}

uint32_t golomb_rice_decoding(GolombEncodedData data, uint32_t r)
{
    uint32_t quotient = data.unary.size() - 1;

    int pos = 0;
    uint32_t remainder = 0;
    for (int i = 0; i < r; ++i)
    {
        remainder = (remainder << 1) | data.fixed[pos++];
    }
    return (quotient << r) | remainder;
}

bool test_golomb_rice()
{
    struct GolombTestData
    {
        uint32_t value;
        uint32_t r;
    };

    vector<GolombTestData> test_cases = {
        {0, 1},
        {1, 1},
        {2, 1},
        {3, 1},
        {4, 1},
        {5, 1},
        {6, 1},
        {7, 1},
        {8, 1},
        {10, 2},
        {15, 2},
        {20, 2},
        {25, 2},
        {30, 2},
        {50, 3},
        {100, 3},
        {200, 4},
        {500, 5},
        {1024, 6},
        {2048, 7},
        {4096, 8},
        {8192, 9},
        {16384, 10},
        {32768, 11},
        {65536, 12},
        {131072, 13},
        {262144, 14},
        {12345, 5},
        {67890, 6},
        {987654, 10},
        {1234567, 12},
        {2345678, 14},
        {3456789, 16},
        {4567890, 18},
        {5678901, 20},
    };

    for (auto test_case : test_cases)
    {
        uint32_t value = test_case.value;
        uint32_t test_value = golomb_rice_decoding(golomb_rice_encoding(value, test_case.r), test_case.r);

        if (value != test_value)
        {
            cerr << "Test failed for value = " << value << ", r parameter = " << (int)test_case.r
                 << ". Decoded value = " << test_value << endl;
            exit(1);
        }
        // cout << "Test passed for value = " << value << ", k = " << (int)test_case.r << endl;
    }

    return true;
}

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

int nearest_integer_log(int x)
{
    int y = x + (x >> 1);

    return highest_bit(y);
}

bool test_murmur3_32()
{
    string test_keys[4] = {"Hello", "World", "Horatio", "Nelson"};
    uint32_t expected_hash[4] = {1466740371, 3789324275, 2689083821, 2244112232};

    bool test = true;
    for (int i = 0; i < 4; i++)
    {
        test &= (murmur_hash_3_uint32(42, test_keys[i]) == expected_hash[i]);
    }

    return test;
}

template <typename T>
void print_vector(const vector<T> &vec)
{
    for (const auto &element : vec)
    {
        cout << element << " ";
    }
    cout << endl;
}

uint32_t assign_bucket(string key, uint32_t bucket_seed, uint32_t bucket_count)
{
    uint32_t hash = murmur_hash_3_uint32(bucket_seed, key) >> 16;
    uint32_t bucket = floor((hash * bucket_count) >> 16);

    return bucket;
}

vector<uint32_t> assign_buckets(vector<string> keys, uint32_t bucket_seed, uint32_t bucket_size)
{
    vector<uint32_t> bucket_assignments(keys.size());
    uint32_t bucket_count = ceil(keys.size() / bucket_size);
    for (int i = 0; i < keys.size(); i++)
    {
        bucket_assignments[i] = assign_bucket(keys[i], bucket_seed, bucket_count);
    }

    return bucket_assignments;
}

struct bucket_data
{
    vector<vector<string>> buckets;
    vector<uint32_t> bucket_sizes;
};

bucket_data create_buckets(vector<string> keys, uint32_t bucket_size, uint32_t bucket_seed)
{
    uint32_t bucket_count = ceil((float)keys.size() / (float)bucket_size);

    vector<vector<string>> buckets(bucket_count);
    vector<uint32_t> bucket_sizes(bucket_count);
    for (int i = 0; i < keys.size(); i++)
    {
        // extract top 16 bits of the 32 bit hash
        uint32_t hash = murmur_hash_3_uint32(bucket_seed, keys[i]) >> 16;
        // Perform the bucket assigment
        uint32_t bucket = floor((hash * bucket_count) >> 16);
        buckets[bucket].push_back(keys[i]);
        bucket_sizes[bucket] += 1;
    }

    return bucket_data{buckets, bucket_sizes};
}

uint64_t factorial(int n)
{
    uint64_t ans = 1;
    while (n > 0)
    {
        ans *= n;
        n--;
    }

    return ans;
}

void test_assign_buckets()
{
    string addressFilePath = "addresses.txt";

    std::ifstream inputFile(addressFilePath);
    vector<string> addresses;
    string line;

    while (getline(inputFile, line))
    {
        addresses.push_back(line);
    }

    int bucket_size = 50;
    inputFile.close();
    uint32_t bucket_count = ceil(addresses.size() / bucket_size);

    vector<int> counts(bucket_count);
    for (int i = 0; i < 10000; i++)
    {
        vector<uint32_t> bucket_assignments = assign_buckets(addresses, i, bucket_size);
        for (int j = 0; j < addresses.size(); j++)
        {
            counts[bucket_assignments[j]]++;
        }
    }

    print_vector(counts);
    uint64_t top = factorial(addresses.size() * 100);
    uint64_t bottom = 1;
    for (uint16_t i = 0; i < counts.size(); i++)
    {
        bottom *= factorial(counts[i]);
    }

    float prob = top / bottom;
    prob *= (pow((1 / bucket_count), (addresses.size() * 100))) * 100;
    cout << prob;
    cout << endl;
}

uint32_t find_bijection(vector<string> keys)
{
    uint32_t seed = 0;
    while (true)
    {
        bool bijection = true;
        set<uint32_t> indexes;
        print_vector(keys);
        for (string key : keys)
        {
            uint32_t hash = murmur_hash_3_uint32(seed, key) % keys.size();
            cout << "Hash for Key: " << key << " " << hash << endl;
            if (indexes.contains(hash))
            {
                bijection = false;
                break;
            }

            indexes.insert(hash);
        }

        if (bijection)
        {
            return seed;
        }

        // cout << seed << endl;
        seed++;
    }
}

int calculate_parts(int depth, int leaf_size)
{

    if (depth == 0)
    {
        return max(2, (int)ceil(0.35 * leaf_size + 0.5));
    }
    else if (depth == 1 && leaf_size >= 7)
    {
        return ceil(0.21 * leaf_size + 0.9);
    }
    else
    {
        return 2;
    }
}

// https://github.com/thomasmueller/minperf/blob/master/src/test/java/org/minperf/simple/recsplit.md
void split(vector<string> keys, uint32_t leaf_size, vector<uint32_t> &splitting_tree, uint32_t depth)
{
    cout << "Key Size: " << keys.size() << endl;
    if (keys.size() <= leaf_size)
    {
        if (keys.size() == 1)
        {
            return;
        }
        uint32_t bijection_index = find_bijection(keys);
        splitting_tree.push_back(bijection_index);
        return;
    }

    uint32_t parts = calculate_parts(depth, leaf_size);
    cout << "Parts: " << parts << endl;
    cout << "Depth: " << depth << endl;
    vector<uint32_t> expected_counts(parts);
    uint16_t bigger = keys.size() % parts;

    // cout << "Parts: " << parts << endl;
    // cout << "Keys Size: " << keys.size() << endl;
    // cout << "N Bigger: " << bigger << endl;
    // cout << (float)keys.size() / (float)parts << endl;
    // cout << floor(keys.size() / parts)

    for (int i = 0; i < parts; i++)
    {
        if (i < bigger)
        {
            expected_counts[i] = ceil((float)keys.size() / (float)parts);
        }
        else
        {
            expected_counts[i] = floor((float)keys.size() / (float)parts);
        }
    }

    uint32_t seed = 0;
    while (true)
    {
        vector<uint32_t> counts(parts);
        for (string key : keys)
        {
            uint32_t hash = murmur_hash_3_uint32(seed, key);
            uint32_t part = hash % parts;
            counts[part] += 1;
            if (counts[part] > expected_counts[part])
            {
                seed++;
                continue;
            }
        }

        // print_vector(expected_counts);
        // print_vector(counts);
        if (counts == expected_counts)
        {
            break;
        }

        seed++;
    }

    splitting_tree.push_back(seed);

    vector<vector<string>> keys_split(parts);
    for (string key : keys)
    {
        uint32_t hash = murmur_hash_3_uint32(seed, key);
        keys_split[hash % parts].push_back(key);
    }

    for (vector<string> key_part : keys_split)
    {
        split(key_part, leaf_size, splitting_tree, depth + 1);
    }
}

struct recsplit_data
{
    // Data required for lookup
    vector<uint32_t> splitting_tree;
    vector<uint32_t> bucket_prefixes;
    vector<uint32_t> bucket_indexes;

    // Constants
    uint32_t n_keys;
    uint32_t leaf_size;
    uint32_t bucket_seed;
    uint32_t bucket_size;
};

uint32_t leaf_size = 2;
uint32_t bucket_size = 4;
uint32_t bucket_seed = 1;

recsplit_data recsplit(vector<string> keys)
{
    auto [buckets, bucket_sizes] = create_buckets(keys, bucket_size, bucket_seed);
    print_vector(bucket_sizes);

    vector<uint32_t> splitting_tree;
    vector<uint32_t> bucket_indexes;
    for (vector<string> bucket : buckets)
    {
        print_vector(bucket);
        split(bucket, leaf_size, splitting_tree, 0);
        bucket_indexes.push_back(splitting_tree.size());
    }

    vector<uint32_t> bucket_prefixes(bucket_sizes.size() + 1);
    uint32_t index = 0;
    for (int i = 0; i < bucket_sizes.size(); i++)
    {
        bucket_prefixes[i] = index;
        index += bucket_sizes[i];
    }
    bucket_prefixes[bucket_sizes.size()] = index;

    return recsplit_data{splitting_tree, bucket_prefixes, bucket_indexes, (uint32_t)keys.size(), leaf_size, bucket_seed, bucket_size};
}

void run_test_keys()
{
    leaf_size = 2;
    bucket_size = 4;
    bucket_seed = 5;

    vector<string> test_keys = {"Hello", "World", "RecSplit", "Nelson", "Horatio",
                                "Napoleon", "Alexander", "Victory", "Great", "Nile",
                                "Vincent", "Dock", "Longbow", "Whistle", "Thyme"};

    recsplit_data data = recsplit(test_keys);
    print_vector(data.splitting_tree);
    // for (int i = 0; i < 15; i++)
    // {
    //     cout << "MPHF Hash: " << lookup2(test_keys[i], data) << endl;
    // }
}

void run_test_words()
{
    leaf_size = 8;
    bucket_size = 600;

    string wordFilePath = "words.txt";
    std::ifstream inputFile(wordFilePath);
    vector<string> words;
    string line;

    while (getline(inputFile, line))
    {
        words.push_back(line);
    }

    inputFile.close();
    recsplit_data data = recsplit(words);
    print_vector(data.splitting_tree);
    for (int i = 0; i < 15; i++)
    {
        cout << lookup2(words[i], data) << endl;
    }
}

int main()
{
    cout << "Murmur3 32 hash test: ";
    cout << (test_murmur3_32() ? "Passed" : "!!! FAILED !!!");
    cout << endl;

    cout << "Golomb Rice Encoding test: ";
    cout << (test_golomb_rice() ? "Passed" : "!!! FAILED !!!");
    cout << endl;

    // test_assign_buckets();

    run_test_keys();
    // run_test_words();

    cout << "Press Enter to Exit...";
    cin.get();

    return 0;
}
