#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_SHIFT_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_SHIFT_H_

#include <vector>
#include <string>
#include <climits>
#include "ns3/GridHome.h"
#include "ns3/MyConfig.h"
#include "ns3/DeviceEvent.h"
#include "ns3/PrivadePlus_TokenData.h"
#include "ns3/DataBasis.h"
#include "ns3/Turbo.h"
#include "ns3/DsmMode.h"

namespace ns3 {

static void privadePlus_shiftBasis(std::vector<void*>* params, int (*costRating)(int, int)) {
	GridHome* home = (GridHome*) (*params)[0];
	DeviceEvent* event = (DeviceEvent*) (*params)[1];
	PrivadePlus_TokenData* token = (PrivadePlus_TokenData*) (*params)[2];

	// data
	std::vector<double>* barriers = home->getData<std::vector<double> >("barriers", false);
	if(barriers == NULL) {
		barriers = DsmMode::Get()->getQualityGoal();
		home->addDataValue("barriers", barriers);
	}

	MyConfig* config = MyConfig::Get();
	int simStart = config->getInt("start");
	int interval = config->getInt("interval");
	std::vector<double>* consumption = token->getExpectedConsumption();
	std::vector<double>* deviceConsumption = (*DataBasis::Get()->get("devices/avg_" + event->getDevice()->getCategory(), "txt", CSV_PER_LINE))[0];
	int homes = config->getInt("homes");
	int consumptionBarrier = (*barriers)[Tick::Get()->getMinute() % barriers->size()] * homes;

	double cost = INT_MAX;
	int activation = (Tick::Get()->getMinute() - simStart) / interval + 1; // in ticks, without start offset
	int deadline = (event->getData() - simStart) / interval;// - deviceConsumption->size();
	for(int i = activation; i <= deadline; i++) {
		int tickCost = 0;
		for(uint j = 0; j < deviceConsumption->size(); j++) {
			int tmpCost = (*consumption)[(i + j) % consumption->size()] + (*deviceConsumption)[j];
			tmpCost = costRating(tmpCost, consumptionBarrier);
			tickCost += tmpCost;
		}
		if(tickCost < cost) {
			cost = tickCost;
			activation = i;
		}
	}

	for(uint i = 0; i < deviceConsumption->size(); i++) {
		(*consumption)[(activation + i) % consumption->size()] += (*deviceConsumption)[i];
	}
	int activationSim = simStart + activation * interval;
	home->addEvent(new DeviceEvent(activationSim, event->getDevice(), EV_TURN_ON));
}

static int privadePlus_costRatingFactor(int consumption, int limit) {
	if(consumption <= limit)
		return consumption;
	else
		return limit + (consumption - limit) * 10;
}

static void privadePlus_GridHomeShiftFactor(std::vector<void*>* params) {
	privadePlus_shiftBasis(params, &privadePlus_costRatingFactor);
}

static int privadePlus_costRatingQuadratic(int consumption, int limit) {
	return (limit - consumption) * (limit - consumption);
}

static void privadePlus_GridHomeShiftQuadratic(std::vector<void*>* params) {
	privadePlus_shiftBasis(params, &privadePlus_costRatingQuadratic);
}

}

#endif
