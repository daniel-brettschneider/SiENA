#include "SmartHome.h"

namespace ns3 {

void SmartHome::tick() {
	ConventionalHome::tick();

	std::map<std::string, Device*>::iterator it;
	for(it = devices.begin(); it != devices.end(); ++it) {
		std::string type = Helper::getDeviceType(it->first);
		std::string module = "";
		std::vector<void*> params;
		params.push_back(this);
		if(type == "car")
			module = moduleCar;
		else if(type == "battery")
			module = moduleBattery;
		else if(type == "chps")
			module = moduleChps;
		else if(type == "heatpump")
			module = moduleHeatpump;
		if(module != "") {
			params.push_back(it->second);
			Modules::Get()->execute(module, &params);
		}
	}
}

void SmartHome::schedule(DeviceEvent* event) {
	std::vector<void*> params;
	params.push_back(this);
	params.push_back(event);
	Modules::Get()->execute(moduleShift, &params);
}

}
