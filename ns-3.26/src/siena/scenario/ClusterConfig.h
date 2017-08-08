#ifndef CLUSTERCONFIG_H_
#define CLUSTERCONFIG_H_

#include <vector>
#include "ns3/CompositeConfig.h"
#include "ns3/HomeConfig.h"

namespace ns3 {

class ClusterConfig {

private:
	std::vector<HomeConfig*> homes;
	std::vector<CompositeConfig*> composites;

public:
	ClusterConfig() {}
	virtual ~ClusterConfig() {
		for(uint i = 0; i < homes.size(); i++) {
			delete homes[i];
		}
		for(uint i = 0; i < composites.size(); i++) {
			delete composites[i];
		}
	}

	std::vector<HomeConfig*>* getHomes() { return &homes; }
	std::vector<CompositeConfig*>* getComposites() { return &composites; }

	void addHome(HomeConfig* config) { this->homes.push_back(config); }
	void addComposite(CompositeConfig* config) { this->composites.push_back(config); }

};

}

#endif
