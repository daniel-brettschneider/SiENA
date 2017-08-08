#ifndef COMPOSITECONFIG_H_
#define COMPOSITECONFIG_H_

#include <string>
#include <map>
#include <vector>
#include "ns3/HomeConfig.h"
#include "ns3/DeviceConfig.h"

namespace ns3 {

class CompositeConfig {

private:
	std::vector<HomeConfig*> homes;
	std::map<std::string, DeviceConfig> devices;

public:
	CompositeConfig() {}
	virtual ~CompositeConfig() {
		for(uint i = 0; i < homes.size(); i++) {
			delete homes[i];
		}
	}

	std::vector<HomeConfig*>* getHomes() { return &homes; }
	std::map<std::string, DeviceConfig>* getDevices() { return &devices; }

	void addHome(HomeConfig* config) { this->homes.push_back(config); }
	void addDevice(std::string device, std::string config, int seed) { this->devices.insert(std::pair<std::string, DeviceConfig>(device, DeviceConfig(config, seed))); }
	double getHeatDemand() {
		double heatDemand = 0;
		for(uint i = 0; i < homes.size(); i++) {
			heatDemand += homes.at(i)->getHeatDemand();
		}
		return heatDemand;
	}
};

}

#endif
