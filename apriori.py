#!/usr/bin/env python3
import sys
import itertools
from collections import Counter


def get_most_frequent_items(in_file, lines_to_process, threshold):
	in_file.seek(0)
	cnt = Counter()

	for _ in range(lines_to_process):
		basket = in_file.readline().split()
		for product in basket:
			cnt[product] += 1

	return [k for k,v in cnt.items() if v >= threshold]


def get_most_frequent_pairs(in_file, lines_to_process, item_pairs, threshold):
	in_file.seek(0)
	cnt = Counter()

	for _ in range(lines_to_process):
		basket = set(in_file.readline().split())
		for pair in item_pairs:
			if pair[0] in basket and pair[1] in basket:
				cnt[pair] += 1

	return [k for k,v in cnt.items() if v >= threshold]


def main():
	if len(sys.argv) < 2:
		print('usage: ' + sys.argv[0] + " 'percent of file to process' 'percent support threshold'")
		exit()

	percent_lines = float(sys.argv[1])
	percent_support = float(sys.argv[2])
	if percent_lines < 0 or percent_lines > 100 or percent_support < 0 or percent_support > 100:
		print('percent must be between 0 and 100!')
		exit()

	retail = open('retail.txt')

	num_lines = sum(1 for line in retail)
	retail.seek(0)
	lines_to_process = int((percent_lines * num_lines) // 100)
	threshold = int((percent_support * lines_to_process) // 100)

	print('Number of Baskets: ' + str(lines_to_process))
	print('Support Threshold: ' + str(threshold))
	print('Running apriori algorithm..')

	most_frequent_items = get_most_frequent_items(retail, lines_to_process, threshold)
	item_pairs = list(itertools.combinations(most_frequent_items, 2))
	most_frequent_pairs = get_most_frequent_pairs(retail, lines_to_process, item_pairs, threshold)

	print('The most frequent pairs are:')
	print(most_frequent_pairs)


if __name__ == '__main__':
	main()
