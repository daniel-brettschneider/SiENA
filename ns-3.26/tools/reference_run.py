#!/usr/bin/python
import sys
import os
import subprocess
import csv

def main():
	if len(sys.argv) != 4:
		print("usage: [identifier] [interval] [quality_start_day]")
		return
	name = sys.argv[1]
	interval = sys.argv[2]
	day = sys.argv[3]
	
	# ref run
	print ("starting reference run...")
	subprocess.call('./waf --run "siena --identifier=%s --smarthome_share=0.0 --gridhome_share=0.0 --save_only_these_stats=consumption,grouped --filter_stats=true --keep_only_these_stats_in_ram=consumption,grouped --graphs= --analyze=false --simulate_communication=false"' % (name), shell=True)
	
	# copy consumption
	print ("\ncopying consumption results...")
	con = []
	with open("results/%s_consumption_grouped.csv" % (name), 'r') as f:
		reader = csv.reader(f, delimiter=',')
		first = True
		for row in reader:
			if first:
				first = False
				continue
			tmp = 0
			for value in row:
				tmp += float(value)
			con.append(tmp)
	with open("data/dsm/quality_compare.csv", 'w') as f:
		f.write("compare\n")
		for c in con:
			f.write(str(c) + '\n')
	
	# sim run
	print ("\nstarting sim run...")
	subprocess.call('./waf --run "siena --identifier=%s"' % (name), shell=True)
	
	# r
	print ("\nrunning r script...")
	subprocess.call('Rscript --slave --vanilla data/graphs/quality.R %s %s %s' % (name, interval, day), shell=True)
	
if __name__ == "__main__":
	main()	
