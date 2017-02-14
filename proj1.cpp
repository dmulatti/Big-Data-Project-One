#include <chrono>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <iterator>

using namespace std;

uint32_t hash1(uint16_t first, uint16_t second) {
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

uint32_t hash2(uint16_t first, uint16_t second) {
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

vector<bool> counts_to_frequent_bitmap(const vector<uint32_t>& counts, unsigned support_threshold) {
	vector<bool> freq(counts.size(), false);
	for(unsigned i = 0; i < counts.size(); ++i) {
		if(counts[i] >= support_threshold) {
			freq[i].flip();
		}
	}
	return freq;
}

void PCY_basic(const vector<vector<uint16_t>>& data, double support_percentage, double file_percentage) {
	vector<uint32_t> item_counts(16500, 0);
	vector<uint32_t> pair_counts(0x00ffffff, 0);
	
	for(const auto& v : data) {
		for(const auto& i : v) {
			++item_counts[i];
		}
		for(unsigned i = 0; i < v.size(); ++i) {
			for(unsigned j = i + 1; j < v.size(); ++j) {
				++pair_counts[hash1(v[i], v[j]) & 0x00ffffff];
			}
		}
	}
	
	// The bool vector is implemented as a bitset
	vector<bool> frequent_items = counts_to_frequent_bitmap(item_counts, data.size()*support_percentage);
	item_counts.resize(0); // deallocations memory used by vector
	vector<bool> frequent_pairs = counts_to_frequent_bitmap(pair_counts, data.size()*support_percentage);
	pair_counts.resize(0);

		
}

vector<vector<uint16_t>> read_baskets_from_file(string filename) {
	vector<vector<uint16_t>> data;
	string buf;
	ifstream fin(filename, istream::in);
	while(getline(fin, buf)) {
		istringstream line(buf);
		data.push_back(vector<uint16_t>((istream_iterator<uint16_t>(line)),
						istream_iterator<uint16_t>()));
		sort(data.back().begin(), data.back().end());
	}
	return data;	
}

int64_t benchmark(void (*func)(const vector<vector<uint16_t>>&, double, double),
					const vector<vector<uint16_t>>& data, double support, double file_percentage) {
	auto start = chrono::high_resolution_clock::now();
	func(data, support, file_percentage);
	auto end = chrono::high_resolution_clock::now();
	return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

int main(){
	vector<vector<uint16_t>> data = read_baskets_from_file("retail.txt");
	cout << benchmark(&PCY_basic, data, 0.1, 0.1) << endl;
}
