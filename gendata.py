#!/usr/bin/env python3
import random
import os

def main():
	f = open('bigdata.txt', 'w')
	while os.path.getsize('bigdata.txt') < 9000000000:
		for _ in range(random.randint(1, 20)):
			f.write(str(random.randint(1, 1000)) + ' ')
		f.write(str(random.randint(1, 1000)) + '\n')

if __name__ == '__main__':
	main()
