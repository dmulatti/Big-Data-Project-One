#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mach/mach.h>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

inline uint64_t get_current_memory_usage()
{
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
    task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count);
    return t_info.resident_size;
}

inline uint32_t hash1(const uint16_t first, const uint16_t second)
{
    constexpr uint32_t FNV_prime = 16777619;
    constexpr uint32_t FNV_offset_basis = 2166136261;
    uint32_t hash = FNV_offset_basis;
    hash ^= (first & 0x00ff);
    hash *= FNV_prime;
    hash ^= ((first & 0x00ff) >> 8);
    hash *= FNV_prime;
    hash ^= (second & 0x00ff);
    hash *= FNV_prime;
    hash ^= ((second & 0x00ff) >> 8);
    hash *= FNV_prime;
    return hash;
}

inline uint32_t hash2(const uint16_t first, const uint16_t second)
{
    constexpr uint32_t FNV_prime = 16777619;
    constexpr uint32_t FNV_offset_basis = 2166136261;
    uint32_t hash = FNV_offset_basis;
    hash *= FNV_prime;
    hash ^= (first & 0x00ff);
    hash *= FNV_prime;
    hash ^= ((first & 0x00ff) >> 8);
    hash *= FNV_prime;
    hash ^= (second & 0x00ff);
    hash *= FNV_prime;
    hash ^= ((second & 0x00ff) >> 8);
    return hash;
}

inline uint32_t pair_16s_to_32(uint16_t a, uint16_t b)
{
    return ((uint32_t)a << 16) | b;
}

inline vector<bool> counts_to_frequent_bitmap(const vector<uint32_t> &counts, const unsigned support_threshold)
{
    vector<bool> freq(counts.size(), false);
    for (unsigned i = 0; i < counts.size(); ++i)
    {
		if (counts[i] >= support_threshold)
		{
	    	freq[i] = true;
		}
    }
    return freq;
}

void PCY_basic(const vector<vector<uint16_t>> &data, const double support_percentage, const double file_percentage)
{
    vector<uint32_t> item_counts(16500, 0);
    vector<uint32_t> pair_counts(data.size() * file_percentage * 4, 0);

    for (unsigned k = 0; k < data.size() * file_percentage; ++k)
    {
		for (const auto &i : data[k])
		{
			++item_counts[i];
		}
		for (unsigned i = 0; i < data[k].size(); ++i)
		{
			for (unsigned j = i + 1; j < data[k].size(); ++j)
			{
				++pair_counts[hash1(data[k][i], data[k][j]) & pair_counts.size()];
			}
		}
    }

    // The bool vector is implemented as a bitset
    const vector<bool> frequent_items = counts_to_frequent_bitmap(item_counts, data.size() * support_percentage);
    item_counts.resize(0); // deallocations memory used by vector
    const vector<bool> frequent_pairs = counts_to_frequent_bitmap(pair_counts, data.size() * support_percentage);
    pair_counts.resize(0);

    unordered_map<uint32_t, uint32_t> candidate_pairs;

    for (unsigned k = 0; k < data.size() * file_percentage; ++k)
    {
		for (unsigned i = 0; i < data[k].size() && frequent_items[data[k][i]]; ++i)
		{
			for (unsigned j = i + 1; j < data[k].size(); ++j)
			{
				if (frequent_items[data[k][j]] && frequent_pairs[hash1(data[k][i], data[k][j]) & frequent_pairs.size()])
				{
					candidate_pairs[pair_16s_to_32(data[k][i], data[k][j])] += 1;
				}
			}
		}
    }

    for (const auto &p : candidate_pairs)
    {
		if ((double)p.second / data.size() >= support_percentage)
		{
			cout << (p.first >> 16) << ' ' << (p.first & 0xffff) << '\n';
		}
    }
}

void PCY_multihash(const vector<vector<uint16_t>> &data, const double support_percentage, const double file_percentage)
{
    vector<uint32_t> item_counts(16500, 0);
    vector<uint32_t> pair_counts_1(data.size() * file_percentage * 2, 0);
	vector<uint32_t> pair_counts_2(data.size() * file_percentage * 2, 0);

    for (unsigned k = 0; k < data.size() * file_percentage; ++k)
    {
		for (const auto &i : data[k])
		{
			++item_counts[i];
		}
		for (unsigned i = 0; i < data[k].size(); ++i)
		{
			for (unsigned j = i + 1; j < data[k].size(); ++j)
			{
				++pair_counts_1[hash1(data[k][i], data[k][j]) % pair_counts_1.size()];
				++pair_counts_2[hash2(data[k][i], data[k][j]) % pair_counts_2.size()];
			}
		}
    }

    // The bool vector is implemented as a bitset
    const vector<bool> frequent_items = counts_to_frequent_bitmap(item_counts, data.size() * support_percentage);
    item_counts.resize(0); // deallocations memory used by vector
    const vector<bool> frequent_pairs_1 = counts_to_frequent_bitmap(pair_counts_1, data.size() * support_percentage);
    pair_counts_1.resize(0);
	const vector<bool> frequent_pairs_2 = counts_to_frequent_bitmap(pair_counts_2, data.size() * support_percentage);
    pair_counts_2.resize(0);

    unordered_map<uint32_t, uint32_t> candidate_pairs;

    for (unsigned k = 0; k < data.size() * file_percentage; ++k)
    {
		for (unsigned i = 0; i < data[k].size() && frequent_items[data[k][i]]; ++i)
		{
			for (unsigned j = i + 1; j < data[k].size(); ++j)
			{
				if (frequent_items[data[k][j]] 
					&& frequent_pairs_1[hash1(data[k][i], data[k][j]) % frequent_pairs_1.size()] 
					&& frequent_pairs_2[hash2(data[k][i], data[k][j]) % frequent_pairs_2.size()])
				{
					candidate_pairs[pair_16s_to_32(data[k][i], data[k][j])] += 1;
				}
			}
		}
    }

    for (const auto &p : candidate_pairs)
    {
		if ((double)p.second / data.size() >= support_percentage)
		{
			cout << (p.first >> 16) << ' ' << (p.first & 0xffff) << '\n';
		}
    }
}

void apriori(const vector<vector<uint16_t>> &data, const double support_percentage, const double file_percentage)
{
    unsigned baskets_to_process = file_percentage * data.size();
    unsigned threshold = support_percentage * baskets_to_process;

    unordered_map<unsigned, unsigned> product_counts;

    for (unsigned i = 0; i < baskets_to_process; ++i)
    {
		for (auto const &product : data[i])
		{
			++product_counts[product];
		}
    }

    unordered_set<unsigned> frequent_items;

    for (auto const &count : product_counts)
    {
		if (count.second >= threshold)
		{
			frequent_items.insert(count.first);
		}
    }

    unordered_map<uint32_t, uint32_t> frequent_pairs;

    for (unsigned i = 0; i < baskets_to_process; ++i)
    {
		auto basket = data[i];
		for (unsigned j = 0; frequent_items.count(basket[j]) && j < basket.size(); ++j)
		{
			for (unsigned k = j + 1; k < basket.size(); ++k)
			{
				if (frequent_items.count(basket[k]))
				{
					++frequent_pairs[(pair_16s_to_32(basket[j], basket[k]))];
				}
			}
		}
    }

    for (auto const &pair : frequent_pairs)
    {
		if (pair.second >= threshold)
			cout << (pair.first >> 16) << ' ' << (pair.first & 0xffff) << '\n';
    }
}

vector<vector<uint16_t>> read_baskets_from_file(const string filename)
{
    vector<vector<uint16_t>> data;
    string buf;
    ifstream fin(filename, istream::in);
    while (getline(fin, buf))
    {
		istringstream line(buf);
		data.push_back(vector<uint16_t>((istream_iterator<uint16_t>(line)),
						istream_iterator<uint16_t>()));
		sort(data.back().begin(), data.back().end());
    }
    return data;
}

inline uint64_t benchmark(void (*func)(const vector<vector<uint16_t>> &, const double, const double),
		  const vector<vector<uint16_t>> &data, const double support, const double file_percentage)
{
    const auto start = chrono::high_resolution_clock::now();
    func(data, support, file_percentage);
    const auto end = chrono::high_resolution_clock::now();
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
		cout << "Usage: " << argv[0] << " [filename]\n";
		return 1;
    }
    
	vector<vector<uint16_t>> data = read_baskets_from_file(argv[1]);
	double support_threshold = 0.05;
	double file_percentage = 1;

    cout << "PCY_basic: " << benchmark(&PCY_basic, data, support_threshold, file_percentage) << endl;
    cout << "apriori: " << benchmark(&apriori, data, support_threshold, file_percentage) << endl;
	cout << "PCY_multihash: " << benchmark(&PCY_multihash, data, support_threshold, file_percentage) << endl;
}
