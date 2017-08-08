#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PM_DEVICEAGENT_CAR_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PM_DEVICEAGENT_CAR_H_

#include "ns3/PM_Bid.h"
#include "ns3/Car.h"
#include "ns3/MyConfig.h"
#include "ns3/RunningAvg.h"

namespace ns3 {

PM_Bid* powermatcher_Car_getBid(Car* car, RunningAvg* avg) {	//Currently no histeresis, no use of desired
	double capacity = car->getCapacity();
	double lastCharge = car->getLastCharge();
	MyConfig* config = MyConfig::Get();
	int interval = config->getInt("interval");
	int minCharge = config->getDouble("car_min_charge") * capacity;
	int desiredCharge = config->getDouble("car_desired_charge") * capacity;
	PM_Bid* bid = new PM_Bid;
	double power = car->getPower();
//	double elPrice = avg->getValue();
	double elPrice = 0.27;
//	double hist = elPrice / 2;	//TODO modulate the costs of the device
	double hist = 2 * std::abs(elPrice) * (1 - car->getEfficiency());
	if(car->isAdaptable()) {
		if(lastCharge + power * interval / 60 >= capacity) {	// full
			if(lastCharge <= capacity) {
				power = (capacity - lastCharge) * 60 / interval;
			} else
				std::cout << "!!!!ERROR!!!! Car Overload" << std::endl;
		} else {
			//power = 0;
		}
		bid->addCoordinate(0, power);
		//4 abfragen
		if((desiredCharge - lastCharge) * 60 /interval >= car->getPower()) {//Wenn genug luft nach oben
			power = car->getPower();
		} else if ((desiredCharge - lastCharge) * 60 /interval < car->getPower() && (desiredCharge - lastCharge) * 60 /interval > -car->getPower()) { //Wenn unter desired und Leistung muss begrenzt werden
			power = (desiredCharge - lastCharge) * 60 /interval;
		} else if ((desiredCharge - lastCharge) * 60 /interval <= -car->getPower()) {	//genug luft nach unten
			power = -car->getPower();
		} else {
			std::cout << "Hier ist wohl was schief gelaufen!" << std::endl;
		}
		bid->addCoordinate(elPrice - hist, power);
		bid->addCoordinate(elPrice + hist, power);
		//bid->addCoordinate(elPrice - hist, 0);
		//bid->addCoordinate(elPrice + hist, 0);
		if((lastCharge - minCharge) * 60 / interval < car->getPower()) // empty
			power = (minCharge - lastCharge) * 60 / interval;
		else {
			power = -car->getPower();
		}
		//if(power < 0)
			//power = 0;
		bid->addCoordinate(2*elPrice + hist, power);
	} else {
		bid->addCoordinate(0, car->getConsumption());
	}
	return bid;
}

void powermatcher_Car_setConsumption(Car* car, double consumption) {
	car->setAdaptedConsumption(consumption);
}

}

#endif
