#ifndef SCENARIO_H_
#define SCENARIO_H_

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "jsoncpp/json/json.h"
#include "jsoncpp/json/value.h"
#include "ns3/MySingleton.h"
#include "ns3/HomeConfig.h"
#include "ns3/MyConfig.h"
#include "ns3/DataBasis.h"
#include "ns3/Random.h"
#include "ns3/Helper.h"
#include "ns3/ClusterConfig.h"
#include "ns3/DeviceConfigHelper.h"

namespace ns3 {

class Scenario : public MySingleton<Scenario> {
	friend class MySingleton<Scenario>;

private:
	std::vector<ClusterConfig*> clusters;
	DeviceConfigHelper* devConfig;
	Random* random;

public:
	virtual ~Scenario() {
		for(uint i = 0; i < clusters.size(); i++) {
			delete clusters[i];
		}
	}
	void init();
	std::vector<ClusterConfig*>* getClusters() { return &clusters; }

private:
	Scenario() {
		devConfig = DeviceConfigHelper::Get();
		random = Random::Get();
	}
	void initRandom();
	void initSimpleDevice(std::string device, std::string devConfig, HomeConfig* home, MyConfig* config);
	void init(std::string filename);
	HomeConfig* parseHome(Json::Value json);
	std::string chooseRandomDeviceConfig(std::string type, int r);
	void purgeHeatDevice(std::map<int, HomeConfig*>* source, double target, std::string name, std::string other1Name,
			std::map<int, HomeConfig*>* other1, std::string other2Name, std::map<int, HomeConfig*>* other2);
	std::pair<double, int> getCategory(std::vector<std::vector<double>*>* data);

};

}

#endif
