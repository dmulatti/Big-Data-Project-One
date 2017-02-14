#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdint>
#include <iterator>
#include <map>
#include <unordered_set>
#include <utility>

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

void PCY_basic(const vector<vector<uint16_t>>& data, unsigned support_threshold) {
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
	vector<bool> frequent_items = counts_to_frequent_bitmap(item_counts, support_threshold);
	item_counts.resize(0); // deallocations memory used by vector
	vector<bool> frequent_pairs = counts_to_frequent_bitmap(pair_counts, support_threshold);
	pair_counts.resize(0);

	for(unsigned i = 0; i < frequent_items.size(); ++i) {
		if(frequent_items[i]) {
			cout << i << ' ';
		}
	}

}

void apriori(const vector<vector<uint16_t>>& data, double support_percentage, double file_percentage) {
	unsigned baskets_to_process = file_percentage * data.size();
	unsigned threshold = support_percentage * baskets_to_process;

	map<unsigned, unsigned> product_counts;

	for (unsigned i = 0; i < baskets_to_process; ++i) {
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


	for (unsigned i = 0; i<baskets_to_process; ++i) {
		auto basket = data[i];
		for (unsigned j = 0; j < basket.size(); ++j) {
			for (unsigned k = j+1; k < basket.size(); ++k){
				if (frequent_items.count(basket[j]) && frequent_items.count(basket[k])) {
					cout << "(" << basket[j] << ", " << basket[k] << ")\n";
				}
			}
		}
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

int main(){
	vector<vector<uint16_t>> data = read_baskets_from_file("retail.txt");
	apriori(data, 0.1, 1);
}
