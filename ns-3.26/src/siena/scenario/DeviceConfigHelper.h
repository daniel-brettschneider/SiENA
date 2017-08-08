#ifndef DEVICECONFIGHELPER_H_
#define DEVICECONFIGHELPER_H_

#include "ns3/MySingleton.h"
#include "jsoncpp/json/json.h"
#include "jsoncpp/json/value.h"
#include <string>
#include <vector>
#include "ns3/DataBasis.h"
#include <ctime>
#include "ns3/MyConfig.h"

namespace ns3 {

class DeviceConfigHelper : public MySingleton<DeviceConfigHelper> {
	friend class MySingleton<DeviceConfigHelper>;

private:
	Json::Value configs;
	DataBasis* data;
	Random* random;
	double minHeatpump;
	int heatpumpHours;
	double minCHPS;
	int chpsHours;

public:
	virtual ~DeviceConfigHelper() {}

	Json::Value get(std::string type) {
		Json::Value config = configs[type];
		if(config == Json::nullValue) {
			std::string file = data->getFileContent("data/scenario/" + type + ".json");
			Json::Reader reader;
			Json::Value tmp;
			if(!reader.parse(file, tmp, false))
				throw("could not parse data/scenario/" + type + ".json");
			configs[type] = tmp[type];
			config = configs[type];
		}
		return config;
	}

	Json::Value get(std::string type, std::string name) {
		Json::Value config = get(type);
		for(uint i = 0; i < config.size(); i++) {
			if(config[i]["name"].asString() == name) {
				return config[i];
			}
		}
		throw("device config " + type + " " + name + " not found");
	}

	std::string getHeatpump(double heatDemand) {
		double max = heatDemand * 1000 / heatpumpHours;
		double min = minHeatpump * max;
		Json::Value config = get("heatpump");
		std::vector<int> index = random->randomList(0, config.size());
		for(uint i = 0; i < config.size(); i++) {
			Json::Value c = config[index[i]];
			double power =  c["max_el_power"].asDouble() * c["cop"].asDouble();
			if(power >= min && power <= max) {
				return c["name"].asString();
			}
		}
		return "";
	}

	std::string getCHPS(double heatDemand) {
		double max = heatDemand * 1000 / chpsHours;
		double min = minCHPS * max;
		Json::Value config = get("chps");
		std::vector<int> index = random->randomList(0, config.size());
		for(uint i = 0; i < config.size(); i++) {
			Json::Value c = config[index[i]];
			double power = c["max_heat_power"].asDouble();
			if(power >= min && power <= max)
				return c["name"].asString();
		}
		return "";
	}


private:
	DeviceConfigHelper() {
		data = DataBasis::Get();
		MyConfig* config = MyConfig::Get();
		random = Random::Get();
		minHeatpump = config->getDouble("heatpump_min_demand");
		heatpumpHours = config->getInt("heatpump_min_hours");
		minCHPS = config->getDouble("chps_min_demand");
		chpsHours = config->getInt("chps_min_hours");
	}

};

}

#endif
