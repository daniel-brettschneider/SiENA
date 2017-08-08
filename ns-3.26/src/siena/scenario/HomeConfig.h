#ifndef HOMECONFIG_H_
#define HOMECONFIG_H_

#include <string>
#include <map>
#include "ns3/DeviceConfig.h"

namespace ns3 {

#define CONFIG_TYPE_CONVENTIONAL 0
#define CONFIG_TYPE_SMART 1
#define CONFIG_TYPE_GRID 2
#define CONFIG_UNIT_SINGLE 0
#define CONFIG_UNIT_MULTI 1
#define CONFIG_DEV_WASH "washingmachine"
#define CONFIG_DEV_DISH "dishwasher"
#define CONFIG_DEV_DRIER "drier"
#define CONFIG_DEV_CAR "car"
#define CONFIG_DEV_PV "pv"
#define CONFIG_DEV_HEATPUMP "heatpump"
#define CONFIG_DEV_CHPS "chps"
#define CONFIG_DEV_HEATINGROD "heatingrod"
#define CONFIG_DEV_BATTERY "battery"
#define CONFIG_DEV_TRAFFIC "traffic"

class HomeConfig {

private:
	std::string name;
	int id;
	int type;
	int residents;
	int unit;
	int space;
	double heatEfficiency;
	std::map<std::string, DeviceConfig> devices;
	int seed;

public:
	HomeConfig(int seed) : name(""), id(-1), type(-1), residents(-1), unit(-1), space(-1), heatEfficiency(-1), seed(seed) {}
	virtual ~HomeConfig() {}

	std::string getName() { return name; }
	int getId() { return id; }
	std::map<std::string, DeviceConfig>* getDevices() { return &devices; }
	double getHeatEfficiency() { return heatEfficiency; }
	void setHeatEfficiency(double heatEfficiency) { this->heatEfficiency = heatEfficiency; }
	int getResidents() const { return residents; }
	void setResidents(int residents) { this->residents = residents; }
	int getSpace() const { return space; }
	void setSpace(int space) { this->space = space; }
	int getType() const { return type; }
	int getUnit() const { return unit; }
	int getSeed() { return seed; }

	void setName(std::string name) { this->name = name; }
	void setId(int id) { this->id = id; }

	void setUnit(std::string unit) {
		if(unit == "single")
			this->unit = CONFIG_UNIT_SINGLE;
		else if(unit == "multi")
			this->unit = CONFIG_UNIT_MULTI;
		else
			throw("could not parse unit " + unit + " [single,multi]");
	}
	void setUnit(int unit) { this->unit = unit; }

	void setType(std::string type) {
		if(type == "conventional")
			this->type = CONFIG_TYPE_CONVENTIONAL;
		else if(type == "smart")
			this->type = CONFIG_TYPE_SMART;
		else if(type == "grid")
			this->type = CONFIG_TYPE_GRID;
		else
			throw("could not parse type " + type + " [conventional,smart,grid]");
	}
	void setType(int type) { this->type = type; }

	void addDevice(std::string device, std::string config, int seed) {
		this->devices.insert(std::pair<std::string, DeviceConfig>(device, DeviceConfig(config, seed)));
	}

	void removeDevice(std::string device) {
		this->devices.erase(device);
	}

	double getHeatDemand() {
		if(unit == CONFIG_UNIT_SINGLE)
			return residents * 500 + heatEfficiency * space;
		else
			return 1000 + heatEfficiency * space;
	}

};

}

#endif
