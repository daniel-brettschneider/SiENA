#ifndef NS_3_22_SRC_SIENA_MODULES_MISCELLANEOUS_SHIFT_H0_H_
#define NS_3_22_SRC_SIENA_MODULES_MISCELLANEOUS_SHIFT_H0_H_

#include "ns3/GridHome.h"
#include "ns3/Device.h"
#include "ns3/DataBasis.h"
#include "ns3/Tick.h"
#include "ns3/MySingleton.h"
#include "ns3/Log.h"
#include "ns3/Random.h"
#include "ns3/DsmMode.h"

namespace ns3 {

static void shift_H0(std::vector<void*>* params) {
	GridHome* home = (GridHome*) (*params)[0];
	DeviceEvent* event = (DeviceEvent*) (*params)[1];

	std::vector<double>* h0 = (*DataBasis::Get()->get("dsm/h0", "csv", CSV_PER_COLUMN, true, 1, 2607841))[0];
	std::vector<double>* deviceConsumption = (*DataBasis::Get()->get("devices/avg_" + event->getDevice()->getCategory(), "txt", CSV_PER_LINE, true))[0];
	std::vector<double>* goal = DsmMode::Get()->getQualityGoal();


	Tick* tick = Tick::Get();
	double cost = INT_MAX;
	int activation = tick->getMinute() / tick->getInterval() + 1; // start next tick
	int deadline = event->getData() / tick->getInterval();
	std::vector<std::pair<int, double> > tickCosts;

	// get cost for starting at each tick, and best one
	for(int i = activation; i <= deadline; i++) {
		int tickCost = 0;
		for(uint j = 0; j < deviceConsumption->size(); j++) {
			double c = (*h0)[(i + j) % h0->size()] + (*deviceConsumption)[j];
			double limit = (*goal)[(i + j) % goal->size()];
			if(c > limit)
				c = (c - limit) * 10 + limit;
			tickCost += c;
		}
		tickCosts.push_back(std::pair<int, double>(i, tickCost));
		if(tickCost < cost) {
			cost = tickCost;
			activation = i;
		}
	}

	// select only best ones
	std::vector<int> bests;
	for(uint i = 0; i < tickCosts.size(); i++) {
		if(tickCosts[i].second <= cost * MyConfig::Get()->getDouble("shift_h0_factor"))
			bests.push_back(tickCosts[i].first);
	}
	if(bests.size() > 0) {
		int r = Random::Get()->get(0, bests.size()-1);
		activation = bests[r];
	}

	home->addEvent(new DeviceEvent(activation * tick->getInterval(), event->getDevice(), EV_TURN_ON));
}

}

#endif
