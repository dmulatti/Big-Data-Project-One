#!/usr/bin/env python3
import random

def main():
	f = open('bigdata.txt', 'w')
	for _ in range(200000):
		s = ''
		for _ in range(random.randint(1, 20)):
			s += str(random.randint(1, 1000)) + ' '
		s += str(random.randint(1, 1000)) + '\n'
		f.write(s)

if __name__ == '__main__':
	main()
