#!/usr/bin/python3
import sys
import os
from multiprocessing import Pool
import subprocess
import random
import csv
from decimal import Decimal

def run(params):
	id = params[0]
	rep = params[1]
	varRep = 0
	var = False
	refrun = False
	config = ""
	if len(params) == 5:
		varRep = params[2]
		var = params[3]
		refrun = params[4]
		config = "--seed=%d --%s=%f" % (rep, var, varRep)
	# reference run
	if refrun:
		subprocess.check_output('./waf --run "siena --identifier=%s_%f_%d %s --smarthome_share=0.0 --gridhome_share=0.0 --save_only_these_stats=consumption,grouped --graphs= --analyze=false --simulate_communication=false --simulate_neplan=false" > log/log_%s_%f_%d_refrun.txt' % (id, varRep, rep, config, id, varRep, rep), shell=True)
		# copy
		with open("data/dsm/quality_compare_%s_%f_%d.csv" % (id, varRep, rep), 'w') as fOut:
			fOut.write("compare\n")
			with open("results/%s_%f_%d_consumption_grouped.csv" % (id, varRep, rep), 'r') as fIn:
				reader = csv.reader(fIn, delimiter=',')
				first = True
				for row in reader:
					if first:
						first = False
						continue
					tmp = 0
					for value in row:
						tmp += float(value)
					fOut.write(str(tmp) + '\n')
	# sim run
	subprocess.check_output('./waf --run "siena --identifier=%s_%f_%d %s" > log/log_%s_%f_%d.txt' % (id, varRep, rep, config, id, varRep, rep), shell=True)
	# clean
	if refrun:
		os.remove("data/dsm/quality_compare_%s_%f_%d.csv" % (id, varRep, rep))

def main():
	nargs = len(sys.argv)
	if nargs != 4 and nargs != 9:
		print("[id] [repetitions] [processes] [variable] [from] [to] [step] [refrun]\nExample: test 100 10\nExample: test 10 10 own_pv 0 1 0.02 -> 0-100% pv in 2% steps, each repeated 10 times, 10 processes")
		return
	
	params = []
	id = sys.argv[1]
	reps = int(sys.argv[2])
	procs = int(sys.argv[3])
	if nargs == 4:
		for i in range(0, reps):
			params.append([id, i])
	else:
		var = sys.argv[4]
		fr = float(sys.argv[5])
		to = float(sys.argv[6])
		step = Decimal(sys.argv[7])
		refrun = sys.argv[8] == "true"
		for i in range(0, reps):
			count = Decimal(fr)
			while count <= to:
				params.append([id, i, count, var, refrun])
				count += step
			
	random.shuffle(params)
	pool = Pool(processes=procs)
	pool.map(run, params)
	
	

if __name__ == "__main__":
	main()
	
