#ifndef NS_3_22_SRC_SIENA_PANDAPOWER_PANDAPOWER_H_
#define NS_3_22_SRC_SIENA_PANDAPOWER_PANDAPOWER_H_

#include <string>
#include "ns3/MyConfig.h"
#include "ns3/Log.h"
#include "ns3/DataBasis.h"
#include "ns3/HomeContainer.h"
#include <cstdio>
#include <iostream>
#include "ns3/Helper.h"

namespace ns3 {

class Pandapower {

private:
	std::map<std::string, int> statsIds;
	int64_t interval;
	int64_t duration;
	HomeContainer* homeContainer;
	DataBasis* db;
	int streetSize;
	std::string net;
	std::string python;
	MyStats* stats;

public:
	Pandapower();
	virtual ~Pandapower() {}
	void init();
	void start();
	void tick();

};

}

#endif
