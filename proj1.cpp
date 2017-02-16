#include <chrono>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

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

uint32_t pair_16s_to_32(uint16_t a, uint16_t b) {
	return ((uint32_t)a << 16) | b;
}

vector<bool> counts_to_frequent_bitmap(const vector<uint32_t>& counts, unsigned support_threshold) {
	vector<bool> freq(counts.size(), false);
	for(unsigned i = 0; i < counts.size(); ++i) {
		if(counts[i] >= support_threshold) {
			freq[i] = true;
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
	vector<bool> frequent_items = counts_to_frequent_bitmap(item_counts, (double)data.size()*support_percentage);
	//item_counts.resize(0); // deallocations memory used by vector
	vector<bool> frequent_pairs = counts_to_frequent_bitmap(pair_counts, (double)data.size()*support_percentage);
	//pair_counts.resize(0);

	unordered_map<uint32_t, uint32_t> candidate_pairs;

	for(const auto& v : data) {
		for(unsigned i = 0; i < v.size() && frequent_items[v[i]]; ++i) {
			for(unsigned j = i + 1; j < v.size(); ++j) {
				if(frequent_items[v[j]] && frequent_pairs[hash1(v[i], v[j]) & 0x00ffffff]) {
						candidate_pairs[pair_16s_to_32(v[i], v[j])] += 1;
				}
			}
		}
	}

	for(const auto& p : candidate_pairs) {
		if((double)p.second/data.size() > support_percentage) {
			cout << '(' << (p.first >> 16) << ", " << (p.first & 0xffff) << ")\n";
		}
	}
}

void apriori(const vector<vector<uint16_t>>& data, double support_percentage, double file_percentage) {
	unsigned baskets_to_process = file_percentage * data.size();
	unsigned threshold = support_percentage * baskets_to_process;

	unordered_map<unsigned, unsigned> product_counts;

	for (unsigned i=0; i < baskets_to_process; ++i) {
		for(auto const& product : data[i]) {
			++product_counts[product];
		}
	}

	unordered_set<unsigned> frequent_items;

	for (auto const& count : product_counts){
		if (count.second >= threshold){
			frequent_items.insert(count.first);
		}
	}

	cout << "baskets: " << baskets_to_process << '\n';
	cout << "threshold: " << threshold << '\n';

	unordered_map<uint32_t, uint32_t> frequent_pairs;

	for (unsigned i = 0; i<baskets_to_process; ++i) {
		auto basket = data[i];
		for (unsigned j = 0; frequent_items.count(basket[j]) && j < basket.size(); ++j) {
			for (unsigned k = j+1; k < basket.size(); ++k){
				if (frequent_items.count(basket[k])) {
					++frequent_pairs[(pair_16s_to_32(basket[j], basket[k]))];
				}
			}
		}
	}

	for (auto const& pair : frequent_pairs) {
		if (pair.second >= threshold)
			cout << '(' << (pair.first >> 16) << ", " << (pair.first & 0xffff) << ")\n";
	}



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
	return chrono::duration_cast<chrono::nanoseconds>(end - start).count();
}

int main(){
	vector<vector<uint16_t>> data = read_baskets_from_file("retail.txt");
	cout << benchmark(&PCY_basic, data, 0.01, 1) << endl;
	cout << benchmark(&apriori, data, 0.01, 1) << endl;
}
