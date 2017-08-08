#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_BATTERY_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_BATTERY_H_

#include "ns3/PM_Bid.h"
#include "ns3/Battery.h"
#include "ns3/MyConfig.h"
#include "ns3/RunningAvg.h"

namespace ns3 {

PM_Bid* powermatcher_Battery_getBid(Battery* battery, RunningAvg* avg) {	//Currently no histeresis
	double capacity = battery->getCapacity();
	double lastCharge = battery->getLastCharge();
	MyConfig* config = MyConfig::Get();
	int interval = config->getInt("interval");
	int minCharge = config->getDouble("battery_min_charge") * capacity;

	PM_Bid* bid = new PM_Bid;
	double power = battery->getPower();
//	double elPrice = avg->getValue();
	double elPrice = 0.27;
	double hist = 2 * std::abs(elPrice) * (1 - battery->getEfficiency());
	// hist = 2.7;

	if(lastCharge + power * interval / 60 >= capacity) {	// full
		power = (capacity - lastCharge) * 60 / interval;
	}
	bid->addCoordinate(0, power);
	bid->addCoordinate(elPrice - hist, 0);
	bid->addCoordinate(elPrice + hist, 0);

	if((lastCharge - minCharge) * 60 / interval < battery->getPower())
		power = lastCharge * 60 / interval;
	else
		power = battery->getPower();
	if(power < 0)
		power = 0;
	//bid->addCoordinate(elPrice + 2*hist, -power);
	bid->addCoordinate(2*elPrice + hist, -power);
	return bid;
}

void powermatcher_Battery_setConsumption(Battery* battery, double consumption) {
	battery->setAdaptedConsumption(consumption);
}

}

#endif
