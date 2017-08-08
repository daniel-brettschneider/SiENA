import pandapower as pp
import sys
import math
import csv

if len(sys.argv) != 3:
	print("usage: street_size loads_file")
	sys.exit()
	
# params
STREET_SIZE = int(sys.argv[1])
loads = []
with open(sys.argv[2], 'r') as f:
    reader = csv.reader(f)
    for r in reader:
    	loads.append(float(r[0]))

# grid
net = pp.create_empty_network()
bus_grid = pp.create_bus(net, name="bus_grid", vn_kv=10.)
pp.create_ext_grid(net, bus_grid)

# transformer
bus_trans = pp.create_bus(net, name="bus_trans", vn_kv=.4)
pp.create_transformer(net, bus_grid, bus_trans, std_type="0.25 MVA 10/0.4 kV")

# streets
streets = math.ceil(len(loads) / STREET_SIZE)
for s in range(1, streets+1):
	# bus street
	bus_street = pp.create_bus(net, name="bus_street_%d" % (s), vn_kv=.4)
	# connect street bus to upper bus
	pp.create_line(net, bus_trans, bus_street, name="line_street_%d" % (s), length_km=0.1, std_type="NAYY 4x150 SE")
	# create houses
	last_house_bus = bus_street
	for h in range(1, STREET_SIZE+1):
		# create house bus and connect
		bus_house = pp.create_bus(net, name="bus_street_%d_house_%d" % (s, h), vn_kv=.4)
		pp.create_line(net, last_house_bus, bus_house, name="line_street_%d_house_%d" % (s, h), length_km=0.04, std_type="NAYY 4x150 SE")
		# load
		pp.create_load(net, bus_house, p_kw=loads[(s-1)*STREET_SIZE+h-1]/1000)
		last_house_bus = bus_house
		
		if (s-1)*STREET_SIZE+h-1 == len(loads)-1:
			break
	
# run power flow
pp.runpp(net)

# print load [%]
for i in range(len(net.bus)):
	print(net.bus.iloc[i]["name"] + "_vm", net.res_bus.iloc[i]["vm_pu"])
print("trafo_load", net.res_trafo.iloc[0]["loading_percent"])
for i in range(len(net.line)):
	print(net.line.iloc[i]["name"] + "_load", net.res_line.iloc[i]["loading_percent"])
	
