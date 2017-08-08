#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_HELPER_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_HELPER_H_

#include <map>
#include <cstdlib>
#include "ns3/Priv_Server.h"

namespace ns3 {

static void priv_tidyUp(std::vector<void*>* params) {
	ServerContainer* serverContainer = ServerContainer::Get();
	std::vector<Server*>* servers = serverContainer->getServers();
	for(uint i = 0; i < servers->size(); i++) {
		Server* server = (*servers)[i];
		delete server->getData<std::vector<std::string> >("homeAddresses");
		delete server->getData<bool>("neplan");
		delete server->getData<bool>("performClustering");
		delete server->getData<int>("msgs_stats");
		delete server->getData<int>("rounds_stats");
		delete server->getData<int>("time_stats");
		delete server->getData<int>("dsm_mode");
//		delete server->getData<FlexHelper>("flex_helper");
		delete server->getData<bool>("clustering_done");
		delete server->getData<std::vector<double> >("futureConsumption");
		delete server->getData<int>("encryption_mode");
		delete server->getData<bool>("shift_expected");
	}

	if(serverContainer->getClusterServer() != NULL) {
		ClusterServer* server = serverContainer->getClusterServer();
		delete server->getData<std::vector<Priv_TokenCluster*> >("token_list");
		delete server->getData<int>("dsm_mode");
		delete server->getData<int>("homes");
	}
}

class MMF_Data {
public:
	std::string id;
	double numberUp, numberDown, amountUp, amountDown, amountCurrent;
	double assigned;
	MMF_Data() : id(""), numberUp(0), numberDown(0), amountUp(0), amountDown(0), amountCurrent(0), assigned(0) {}
	MMF_Data(std::string id, double numberUp, double numberDown, double amountUp, double amountDown, double amountCurrent)
				: id(id), numberUp(numberUp), numberDown(numberDown), amountUp(amountUp), amountDown(amountDown), amountCurrent(amountCurrent), assigned(amountCurrent) {}
	virtual ~MMF_Data() {}
	void add(MMF_Data* d) { amountCurrent += d->amountCurrent; amountDown += d->amountDown; amountUp += d->amountUp; numberDown += d->numberDown; numberUp += d->numberUp; }
};

//static void priv_MaxMinFairShare(std::vector<MMF_Data>* data, double error, MMF_Data* sum = NULL) {
//	MMF_Data tmp;
//	if(sum == NULL) {
//		sum = &tmp;
//		for(uint i = 0; i < data->size(); i++) {
//			sum->add(&(*data)[i]);
//		}
//	}
//
//	Log::i("MaxMinFair", "error: ", error);
//
//	double share = 0;
//	int adaptable = 0;
//	if(error < 0 && sum->numberDown > 0) {
//		adaptable = sum->numberDown;
//		share = error / sum->numberDown;
//	} else if(error > 0 && sum->numberUp > 0) {
//		adaptable = sum->numberUp;
//		share = error / sum->numberUp;
//	}
//
//	if(share != 0) {
//		int round = 1;
//		while(((share < 0 && error < -1) || (share > 0 && error > 1)) && adaptable > 0) {
//			#ifndef TURBO
//				std::stringstream s;
//				s << "round " << round << ", share: " << share << ", adaptable: " << adaptable;
//				Log::i("MaxMinFair", s.str());
//			#endif
//			int adaptableTmp = 0;
//			for(uint i = 0; i < data->size(); i++) {
//				MMF_Data a = (*data)[i];
//				double take = 0;
//				if(share < 0 && a.amountDown >= fabs(a.assigned - a.amountCurrent) - share) {
//					if(fabs(a.assigned - a.amountCurrent) - share > a.amountDown) {
//						std::cout << "piep" << std::endl;
//						std::cout << a.assigned << " " << a.amountCurrent << " " << a.amountDown << std::endl;
//						take = -fabs(a.amountDown - fabs(a.assigned - a.amountCurrent));
//					} else {
//						take = share;
//						adaptableTmp++;
//					}
//				} else if(share > 0 && a.amountUp >= a.assigned - a.amountCurrent + share) {
//					if(a.assigned - a.amountCurrent + share > a.amountUp)
//						take = fabs(a.amountUp - (a.assigned - a.amountCurrent));
//					else {
//						take = share;
//						adaptableTmp++;
//					}
//				}
//				a.assigned += take;
//				error -= take;
//				#ifndef TURBO
//					s.str("");
//					s << "\t" << a.id << " takes " << take << ": "<< a.amountCurrent << " => " << a.assigned << ", new error: " << error;
//					Log::i("MaxMinFair", s.str());
//				#endif
//				(*data)[i] = a;
//			}
//			adaptable = adaptableTmp;
//			share = error / adaptable;
//			round++;
//		}
//	}
//}

static void priv_setAdaptionResult(GridHome* home, AdaptionFlex* adaption) {
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
