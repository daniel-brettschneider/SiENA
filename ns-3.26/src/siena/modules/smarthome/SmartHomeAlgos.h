#ifndef NS_3_22_SRC_CITYGRID_MODULES_SMARTHOME_SMARTHOMEALGOS_H_
#define NS_3_22_SRC_CITYGRID_MODULES_SMARTHOME_SMARTHOMEALGOS_H_

#include <vector>
#include <string>
#include <climits>
#include "ns3/SmartHome.h"
#include "ns3/DataBasis.h"
#include "ns3/Tick.h"
#include "ns3/Device.h"
#include "ns3/DeviceEvent.h"
#include "ns3/MyConfig.h"
#include "ns3/MyStats.h"
#include "ns3/Car.h"
#include "ns3/Adaption.h"

namespace ns3 {

// TODO use devices/cost.csv (dsm/cost.csv is only for one day)

static void smarthome_shift(std::vector<void*>* params) {
	SmartHome* home = (SmartHome*) (*params)[0];
	DeviceEvent* event = (DeviceEvent*) (*params)[1];
	std::vector<double>* costList = (*DataBasis::Get()->get("dsm/cost"))[0];
	int duration = (*DataBasis::Get()->get("devices/avg_" + event->getDevice()->getCategory(), "txt", CSV_PER_LINE))[0]->size();
	int deadline = event->getData();
	int simStart = MyConfig::Get()->getInt("start");
	int interval = MyConfig::Get()->getInt("interval");
	double costBarrier = MyConfig::Get()->getDouble("smarthome_cost_barrier");

	// check cost for all ticks till deadline
	double cost = INT_MAX;
	int activation = (Tick::Get()->getMinute() - simStart) / interval + 1; // in ticks, without start offset, TODO +1 because of lasttick in conv home
	deadline = (Tick::Get()->getMinute() + 1440 - simStart) / interval;// - deviceConsumption->size(); TODO
	for(int i = activation; i <= deadline; i++) {
		int tickCost = 0;
		for(int j = 0; j < duration; j++) {
			double currentCost = (*costList)[(i + j) % costList->size()];
			if(currentCost <= costBarrier)
				tickCost += currentCost;
			else
				tickCost += costBarrier + (currentCost - costBarrier) * 10;
		}
		if(tickCost < cost) {
			cost = tickCost;
			activation = i;
		}
	}

	int activationSim = simStart + activation * interval;
	home->addEvent(new DeviceEvent(activationSim, event->getDevice(), EV_TURN_ON));
}

static void smarthome_battery(std::vector<void*>* params) {
	Battery* bat = (Battery*) (*params)[1];

	if(bat->isAdaptable()) {
		int interval = MyConfig::Get()->getInt("interval");
		std::vector<double>* cost = (*DataBasis::Get()->get("dsm/cost"))[0];
		double currentCost = (*cost)[(Tick::Get()->getMinute() / interval) % cost->size()];
		double costBarrier = MyConfig::Get()->getDouble("smarthome_cost_barrier");
		AdaptionFlex* adaption = bat->getAdaption().first;
		std::cout << currentCost << std::endl;
		if(currentCost > costBarrier +5) {
			bat->setAdaptedConsumption(adaption->getMinimum());
		} else if(currentCost <= costBarrier -5)
			bat->setAdaptedConsumption(adaption->getMaximum());
	}

}

static void smarthome_car(std::vector<void*>* params) {
	Car* car = (Car*) (*params)[1];

	if(car->isAdaptable()) {
		int interval = MyConfig::Get()->getInt("interval");
		std::vector<double>* cost = (*DataBasis::Get()->get("dsm/cost"))[0];
		double currentCost = (*cost)[(Tick::Get()->getMinute() / interval) % cost->size()];
		double costBarrier = MyConfig::Get()->getDouble("smarthome_cost_barrier");
		AdaptionFlex* adaption = car->getAdaption().first;

		if(currentCost > costBarrier +5) {
			car->setAdaptedConsumption(adaption->getMinimum());
		} else if(currentCost <= costBarrier -5)
			car->setAdaptedConsumption(adaption->getMaximum());
	}
}

static void smarthome_chps(std::vector<void*>* params) {
	CHPS* chps = (CHPS*) (*params)[1];

	if(chps->isAdaptable()) {
		int interval = MyConfig::Get()->getInt("interval");
		std::vector<double>* cost = (*DataBasis::Get()->get("dsm/cost"))[0];
		double currentCost = (*cost)[(Tick::Get()->getMinute() / interval) % cost->size()];
		double costBarrier = MyConfig::Get()->getDouble("smarthome_cost_barrier");

		if(chps->getState() == DEV_ON && currentCost <= costBarrier)
			chps->setState(DEV_OFF, 0);
		else if(chps->getState() == DEV_OFF && currentCost > costBarrier)
			chps->setState(DEV_ON, 0);
	}
}

static void smarthome_heatpump(std::vector<void*>* params) {
	HeatPump* hp = (HeatPump*) (*params)[1];

	if(hp->isAdaptable()) {
		int interval = MyConfig::Get()->getInt("interval");
		std::vector<double>* cost = (*DataBasis::Get()->get("dsm/cost"))[0];
		double currentCost = (*cost)[(Tick::Get()->getMinute() / interval) % cost->size()];
		double costBarrier = MyConfig::Get()->getDouble("smarthome_cost_barrier");

		if(hp->getState() == DEV_ON && currentCost > costBarrier)
			hp->setState(DEV_OFF, 0);
		else if(hp->getState() == DEV_OFF && currentCost <= costBarrier)
			hp->setState(DEV_ON, 0);
	}
}

}

#endif
