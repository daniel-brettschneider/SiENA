#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_CHP_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_CHP_H_

#include "ns3/PM_Bid.h"
#include "ns3/CHPS.h"
#include "ns3/RunningAvg.h"

namespace ns3 {

PM_Bid* powermatcher_CHP_getBid(CHPS* chp, RunningAvg* avg) {
	PM_Bid* bid = new PM_Bid;
	if(chp->isAdaptable()) {
		WaterTank* tank = chp->getTank();
		int interval = MyConfig::Get()->getInt("interval");
		double maxHeat = chp->getMaxHeat() / interval * 60;
		int maxEl = -chp->getMaxWork()/ interval * 60;
		double minDes = MyConfig::Get()->getDouble("heatpump_min_desired");
		double chargelevel = tank->getChargeLevel();
//		AdaptionOnOff* adaption = chp->getAdaption().second;
//		maxEl = adaption->getCategory();
		double gasPrice = 0.07;
//		std::cout << "maxEl: " << maxEl << " maxHeat " << maxHeat << std::endl;
		//CHP should start at lower prices when the tank chargelevel is lower
		//double price = (gasPrice * ((double) std::abs(maxEl) + maxHeat) / (double) std::abs(maxEl));	//chp sollte immer früher angehen je leerer der Tank ist
		//price = price + (minDes - tank->getChargeLevel()) * price;
		double price = gasPrice * (chargelevel - minDes) / (std::abs(maxEl)/(std::abs(maxEl) + maxHeat));

		bid->addCoordinate(price, 0);
		bid->addCoordinate(price, maxEl);
	} else
		bid->addCoordinate(0, chp->getConsumption());
	return bid;
}

void powermatcher_CHP_setConsumption(CHPS* chp, double consumption) {
	if(consumption!=0)
		chp->setState(DEV_ON);
	else
		chp->setState(DEV_OFF);
	chp->setAdaptedConsumption(consumption);
}

}

#endif
