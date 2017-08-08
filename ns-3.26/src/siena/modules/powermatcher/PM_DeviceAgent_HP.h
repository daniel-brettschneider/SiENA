#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PM_DEVICEAGENT_HEATPUMP_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PM_DEVICEAGENT_HEATPUMP_H_

#include "ns3/PM_Bid.h"
#include "ns3/HeatPump.h"
#include "ns3/MyConfig.h"
#include "ns3/RunningAvg.h"

namespace ns3 {

PM_Bid* powermatcher_HeatPump_getBid(HeatPump* heatPump, RunningAvg* avg) {
	PM_Bid* bid = new PM_Bid;
	if(heatPump->isAdaptable()) {
		WaterTank* tank = heatPump->getTank();
		int interval = MyConfig::Get()->getInt("interval");
		double maxEl = heatPump->getMaxWork()/ interval * 60;
		double elPrice = 0.27;			//avg->getValue();
		double minPrice = MyConfig::Get()->getDouble("min_price");
		//double maxPrice = MyConfig::Get()->getDouble("max_price");


		//Heatpump should stop at higher prices when the tank chargelevel is lower
		double minCrit = MyConfig::Get()->getDouble("heatpump_min_critical");
		//double minDes = MyConfig::Get()->getDouble("heatpump_min_desired");
		double cop = heatPump->getCOP();
		double chargeLevel = tank->getChargeLevel();
		if(minCrit < chargeLevel) {
			//double price = elPrice + cop*(minDes - chargeLevel) * elPrice; old!
			double price = elPrice * cop * (minCrit - chargeLevel) / (1 - minCrit);
			bid->addCoordinate(price, maxEl);
			bid->addCoordinate(price, 0);
		} else
			bid->addCoordinate(minPrice, maxEl);
	} else
		bid->addCoordinate(0, heatPump->getConsumption());
	return bid;
}

void powermatcher_Heatpump_setConsumption(HeatPump* heatpump, double consumption) {
	if(consumption!=0)
		heatpump->setState(DEV_ON);
	else
		heatpump->setState(DEV_OFF);
	heatpump->setAdaptedConsumption(consumption);
}

}

#endif
