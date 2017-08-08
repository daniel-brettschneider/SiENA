#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_HELPER_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_HELPER_H_

#include <map>
#include <cstdlib>
#include "ns3/PrivadePlus_Server.h"
#include "ns3/PrivadePlus_GridHome.h"

namespace ns3 {

static void privadePlus_SchedulerInit(std::vector<void*>* params) {
	HomeContainer* homes = HomeContainer::Get();
	ServerContainer* servers = ServerContainer::Get();

	MyConfig* config = MyConfig::Get();
	if(config->getDouble("gridhome_share") > 0) {
		std::vector<std::string>* homeAddresses = new std::vector<std::string>;
		for(int i = 0; i < config->getInt("clusters"); i++) {
			Server* server = servers->getServer(i);
			Ipv4Address serverIp = server->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
			std::string* serverAdr = new std::string(Helper::toString(serverIp));
			std::vector<ConventionalHome*>* gridhomes = homes->getHomes(0, 2);
			for(uint j = 0; j < gridhomes->size(); j++) {
				GridHome* home = (GridHome*) gridhomes->at(j);
				home->addDataValue("serverIp", serverAdr);
				Ipv4Address homeIp = home->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
				homeAddresses->push_back(Helper::toString(homeIp));
			}
		}
		servers->getClusterServer()->addDataValue("homeAddresses", homeAddresses);
	}
}

static void privadePlus_tidyUp(std::vector<void*>* params) {
	HomeContainer* homes = HomeContainer::Get();
	for(int i = 0; i < MyConfig::Get()->getInt("clusters"); i++) {
		std::vector<ConventionalHome*>* gridhomes = homes->getHomes(i, 2);
		for(uint j = 0; j < gridhomes->size(); j++) {
			GridHome* gridhome = (GridHome*) gridhomes->at(j);
			delete gridhome->getData<int>("LastTick");
			delete gridhome->getData<int>("backoff");
		}
	}

	ServerContainer* serverContainer = ServerContainer::Get();
	std::vector<Server*>* servers = serverContainer->getServers();
	for(uint i = 0; i < servers->size(); i++) {
		Server* server = (*servers)[i];
		delete server->getData<int>("conv_stats");
		delete server->getData<int>("lastTick");
	}

	if(serverContainer->getClusterServer() != NULL) {
		ClusterServer* server = serverContainer->getClusterServer();
		delete server->getData<std::vector<std::string> >("homeAddresses");
		delete server->getData<bool>("neplan");
		delete server->getData<int>("msgs_stats");
		delete server->getData<int>("rounds_stats");
		delete server->getData<int>("time_stats");
		delete server->getData<int>("dsm_mode");
		delete server->getData<std::vector<double> >("futureConsumption");
		delete server->getData<bool>("shift_expected");
		delete server->getData<int>("backoff");
	}
}

static void privadePlus_setAdaptionResult(GridHome* home, AdaptionFlex* adaption) {
	// get devices
	std::map<Device*, AdaptionFlex*> up, down;
	double desired = 0;
	std::map<std::string, Device*>::iterator it;
	for(it = home->getDevices()->begin(); it != home->getDevices()->end(); ++it) {
		if(it->second->isAdaptable()) {
			AdaptionFlex* a = it->second->getAdaption().first;
			if(a->isAdaptable()) {
				Log::i(home->getId(), "\t" + it->first + ": " + a->toString());
				if(a->getMaximum() > a->getAssigned())
					up.insert(std::pair<Device*, AdaptionFlex*>(it->second, a));
				if(a->getMinimum() < a->getAssigned())
					down.insert(std::pair<Device*, AdaptionFlex*>(it->second, a));
				desired += a->getAssigned();
			}
		}
	}
	// max-min
	std::map<Device*, AdaptionFlex*>::iterator jt;
	double error = adaption->getAssigned() - desired;
	std::map<Device*, AdaptionFlex*>* devices = error < 0 ? &down : &up;
	int adaptableCount = devices->size();
	#ifndef TURBO
		std::stringstream s;
		s << "\ttrying to adapt to: ♥" << desired << " →" << adaption->getAssigned() << " !" << error << " #" << adaptableCount;
		Log::i(home->getId(), s.str());
	#endif
	while((error > 1 || error < -1) && adaptableCount > 0) {
		double share = error / adaptableCount;
		adaptableCount = 0;
		for(jt = devices->begin(); jt != devices->end(); ++jt) {
			AdaptionFlex* a = jt->second;
			if(share < 0) { // reduce consumption
				double tmp = a->getAssigned() + share;
				if(tmp <= a->getMinimum()) {
					error += fabs(a->getMinimum() - a->getAssigned());
					a->setAssigned(a->getMinimum());
					jt->first->setAdaptedConsumption(a->getMinimum());
				} else {
					error -= share;
					a->setAssigned(tmp);
					jt->first->setAdaptedConsumption(tmp);
					adaptableCount++;
				}
			} else { // increase consumption
				double tmp = a->getAssigned() + share;
				if(tmp >= a->getMaximum()) {
					error -= fabs(a->getMaximum() - a->getAssigned());
					a->setAssigned(a->getMaximum());
					jt->first->setAdaptedConsumption(a->getMaximum());
				} else {
					error -= share;
					a->setAssigned(tmp);
					jt->first->setAdaptedConsumption(tmp);
					adaptableCount++;
				}
			}
			#ifndef TURBO
				Log::i(home->getId(), "\t" + jt->first->getId() + ": " + a->toString());
				Log::i(home->getId(), "\tremaing error: ", error);
			#endif
		}
	}
	if(error > 1 || error < -1)
		throw("argh");
}

}

#endif
