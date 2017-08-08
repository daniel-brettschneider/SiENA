#!/usr/bin/python3
import sys
import os
import csv
from decimal import Decimal
from operator import add

def main():
	nargs = len(sys.argv)
	if nargs != 4 and nargs != 7:
		print("[id] [target] [repetitions] [from] [to] [step]\nExample: test consumption 100\nExample: test consumption 10 0 1 0.02 -> 0-100% pv in 2% steps, each repeated 10 times")
		return
	
	id = sys.argv[1]
	target = sys.argv[2]
	reps = int(sys.argv[3])
	fr = 0
	to = 0
	step = 1
	if nargs == 7:
		fr = float(sys.argv[4])
		to = float(sys.argv[5])
		step = Decimal(sys.argv[6])

	headers = 0
	duration = 0
	with open("results/%s_%s_mean.csv" % (id, target), 'w') as out:
		writer = csv.writer(out, delimiter=',')
		run = Decimal(fr)
		firstFile = True
		while run <= to:
			data = [0.0] * headers
			for i in range(0, reps):
				with open("results/%s_%f_%d_%s.csv" % (id, run, i, target), 'r') as f:
					reader = csv.reader(f, delimiter=',')
					first = True
					for row in reader:
						if first:
							if headers == 0:
								writer.writerow(["run"] + row)
								headers = len(row)
								data = [0] * headers
							first = False
							continue
						if firstFile:
							duration += 1
						tmp = []
						for value in row:
							tmp.append(float(value))
						data = list(map(add, data, tmp))
					print("results/%s_%f_%d_%s.csv" % (id, run, i, target))
					firstFile = False
			for i in range(len(data)):
				data[i] = data[i] / reps / duration
			writer.writerow([run] + data)
			run += step

if __name__ == "__main__":
	main()
