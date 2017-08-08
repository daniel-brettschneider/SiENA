#!/usr/bin/python3
import sys

if len(sys.argv) != 2:
	print("usage: [log true/false]")
	sys.exit()
log = sys.argv[1] == "true"

# set log in config
with open("data/config/config.json", "rt") as f:
	content = []
	for line in f.readlines():
		if "log_all" in line:
			rep = "true"
			if not log:
				rep = "false"
			content.append("\t\"log_all\": " + rep + ",\t\t\t\t\t// show log messages, slows down simulation, check src/simulation/Turbo.h for even more speed\n")
		else:
			content.append(line)
with open("data/config/config.json", "wt") as f:
    for line in content:
        f.write(line)
       
# set turbo 
with open("src/siena/simulator/Turbo.h", "rt") as f:
	content = []
	for line in f.readlines():
		if "#define TURBO" in line:
			rep = "//"
			if not log:
				rep = ""
			content.append(rep + "#define TURBO\n")
		else:
			content.append(line)
with open("src/siena/simulator/Turbo.h", "wt") as f:
    for line in content:
        f.write(line)
