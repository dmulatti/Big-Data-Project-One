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

	return [k for k,v in dict(cnt).items() if v >= threshold]


def get_most_frequent_pairs(in_file, lines_to_process, item_pairs, threshold):
	in_file.seek(0)
	cnt = Counter()

	for _ in range(lines_to_process):
		basket = set(in_file.readline().split())
		for pair in item_pairs:
			if pair[0] in basket and pair[1] in basket:
				cnt[pair] += 1

	return [k for k,v in dict(cnt).items() if v >= threshold]


def main():
	retail = open('retail.txt')
	threshold = 1000

	num_lines = sum(1 for line in retail)
	retail.seek(0)
	lines_to_process = num_lines

	# If a command line arg 'x' is given, we only read 'x'% of the lines in
	# the 'retail.txt' file.
	if len(sys.argv) > 1:
		percent = float(sys.argv[1])
		if percent < 0 or percent > 100:
			print('percent must be between 0 and 100!')
			exit()
		lines_to_process = int((percent * num_lines) // 100)

	# Run the apriori algorithm..
	most_frequent_items = get_most_frequent_items(retail, lines_to_process, threshold)
	item_pairs = list(itertools.combinations(most_frequent_items, 2))
	most_frequent_pairs = get_most_frequent_pairs(retail, lines_to_process, item_pairs, threshold)

	print(most_frequent_pairs)


if __name__ == '__main__':
	main()
