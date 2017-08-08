#!/usr/bin/env python
import sys, os

if len(sys.argv) != 2:
	print("usage: ./initProject.py module")
	sys.exit(0)
	
module = sys.argv[1]
print("initializing module " + module + "...")

print("fetching wscript...")
f = open("../src/" + module + "/wscript", "r")
lines = f.readlines()
f.close()
print("\tok")

print("fetching header and source files...")
headers = []
sources = []
for dir_, _, files in os.walk("../src/" + module):
	for file in files:
		if file.endswith(".h"):
			relFile = os.path.join(os.path.relpath(dir_, "../src/" + module), file)
			print("\theader: " + relFile)
			headers.append(relFile)
		elif file.endswith(".cc"):
			relFile = os.path.join(os.path.relpath(dir_, "../src/" + module), file)
			print("\tsource: " + relFile)
			sources.append(relFile)
			
print("creating new wscript...")
f = open("../src/" + module + "/wscript", "w")
skip = False
for line in lines:
	if "module.source" in line:
		f.write(line)
		for source in sources[:-1]:
			f.write("\t\t\'" + source + "\',\n")
		f.write("\t\t\'" + sources[-1] + "\'\n")
		skip = True
	elif "headers.source" in line:
		f.write(line)
		for header in headers[:-1]:
			f.write("\t\t\'" + header + "\',\n")
		f.write("\t\t\'" + headers[-1] + "\'\n")
		skip = True
	else:
		if skip and "]" in line:
			skip = False
		if not skip:
			f.write(line)
print("\tdone")
