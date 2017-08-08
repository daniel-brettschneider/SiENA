#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_HELPER_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_HELPER_H_

#include "ns3/GridHome.h"
#include "ns3/Server.h"
#include "ns3/HomeContainer.h"
#include "ns3/ServerContainer.h"
#include <string>
#include <vector>
#include "ns3/Helper.h"

namespace ns3 {

class PM_Helper {
public:

};

// static void powermatcher_InitModules(std::vector<void*>* params) {}

static void powermatcher_SchedulerInit(std::vector<void*>* params) {
	HomeContainer* homes = HomeContainer::Get();
	ServerContainer* servers = ServerContainer::Get();

	MyConfig* config = MyConfig::Get();
	if(config->getDouble("gridhome_share") > 0) {
		for(int i = 0; i < config->getInt("clusters"); i++) {
			Ipv4Address ip = servers->getServer(i)->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
			std::string* serverAdr = new std::string(Helper::toString(ip));
			std::vector<ConventionalHome*>* gridhomes = homes->getHomes(i, 2);
			for(uint j = 0; j < gridhomes->size(); j++) {
				((GridHome*) gridhomes->at(j))->addDataValue("serverIp", serverAdr);
			}
		}
	}
}

static void powermatcher_tidyUp(std::vector<void*>* params) {
	HomeContainer* homes = HomeContainer::Get();
	for(int i = 0; i < MyConfig::Get()->getInt("clusters"); i++) {
		std::vector<ConventionalHome*>* gridhomes = homes->getHomes(i, 2);
		for(uint j = 0; j < gridhomes->size(); j++) {
			GridHome* gridhome = (GridHome*) gridhomes->at(j);
			delete gridhome->getData<int>("LastTick");
			delete gridhome->getData<std::map<std::string, PM_Bid*> >("bid_storage");
			delete gridhome->getData<RunningAvg>("price_avg");
		}
	}

	ServerContainer* servers = ServerContainer::Get();
	ClusterServer* cs = servers->getClusterServer();
	if(cs != NULL) {
		delete cs->getData<int>("price_stat");
		delete cs->getData<int>("time_stats");
	}
}

}

#endif
