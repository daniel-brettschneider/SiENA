#ifndef STATSEXPORTER_H_
#define STATSEXPORTER_H_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include "ns3/Log.h"
#include "ns3/MyConfig.h"
#include "ns3/MyStats.h"
#include "ns3/Helper.h"
#include "ns3/MySingleton.h"
#include "ns3/HomeContainer.h"
#include "ns3/ConventionalHome.h"
#include <climits>
#include "ns3/ServerContainer.h"

namespace ns3 {

class StatsExporter : public MySingleton<StatsExporter> {
	friend class MySingleton<StatsExporter>;

private:
	MyConfig* config;
	MyStats* stats;
	std::vector<std::vector<float>*> grouped;

public:
	virtual ~StatsExporter() {
		for(uint i = 0; i < grouped.size(); i++) {
			delete grouped[i];
		}
	}

	void exportAdditionalStatsToCsv();
	void createGroupedConsumption();
	void exportDeviceConfigs();
	void exportIps();
	void exportCarFailedRides();
	void exportWeb();
	std::vector<std::vector<float>*>* getGroupedConsumption() { return &grouped; }

private:
	StatsExporter() {
		config = MyConfig::Get();
		stats = MyStats::Get();
	}
	int getType(std::string id, std::vector<std::string>* mov, std::vector<std::string>* adapt, std::vector<std::string>* other, std::vector<std::string>* switchable);

	void exportCsv(std::string filename, std::vector<std::string>* headers, std::vector<std::vector<float>*>* data);

};

}

#endif
