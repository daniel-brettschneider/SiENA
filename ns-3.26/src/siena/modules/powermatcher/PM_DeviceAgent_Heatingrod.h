#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_HEATINGROD_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_HEATINGROD_H_

#include "ns3/PM_Bid.h"
#include "ns3/HeatingRod.h"
#include "ns3/MyConfig.h"
#include "ns3/RunningAvg.h"

namespace ns3 {

PM_Bid* powermatcher_Heatingrod_getBid(HeatingRod* heatingrod, RunningAvg* avg) {
	PM_Bid* bid = new PM_Bid;
	if(heatingrod->isAdaptable()) {
		WaterTank* tank = heatingrod->getTank();
		//double maxHeat = heatingrod->getHeat();
		double maxEl = heatingrod->getMaxWork();
		double elPrice = 0.27;

		double minCrit = MyConfig::Get()->getDouble("heatingrod_min_critical");
		double max = MyConfig::Get()->getDouble("heatingrod_max");
		double chargeLevel = tank->getChargeLevel();

		double price = elPrice * (1 - chargeLevel/(max-minCrit));

		bid->addCoordinate(price,maxEl);
		bid->addCoordinate(price,0);
//		bid->addCoordinate(0, maxEl);
//		if(minCrit < chargeLevel)
//			bid->addCoordinate(0, 0);
	} else
		bid->addCoordinate(0, heatingrod->getConsumption());
	return bid;
}

void powermatcher_Heatingrod_setConsumption(HeatingRod* heater, double consumption) {
	if(consumption!=0)
		heater->setState(DEV_ON);
	else
		heater->setState(DEV_OFF);
	heater->setAdaptedConsumption(consumption);
}

}

#endif
