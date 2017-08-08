#include "Car.h"

namespace ns3 {

Car::Car(std::string id) : Battery(id), numRides(0), numFailedRides(0) {
	fuel = 171.6;
	capacity = 24000;
//	charge = capacity / 2 + (Random::Get()->get() % ((int) capacity / 2));
	minCharge = capacity * MyConfig::Get()->getDouble("car_min_charge");
	charge = minCharge + (Random::Get()->get() % ((int) (capacity - minCharge)));
	desiredCharge = capacity * MyConfig::Get()->getDouble("car_desired_charge");

	// charge rates
	std::vector<double> chargeRates = Helper::splitToDouble(MyConfig::Get()->getString("car_charge_rate"), ',');
	double r = Random::Get()->getD();
	double sum = 0;
	for(uint i = 0; i < chargeRates.size(); i += 2) {
		sum += chargeRates[i];
		if(r <= sum) {
			chargeRateH = chargeRates[i + 1];
			power = chargeRateH;
			break;
		}
	}
	chargeRateInterval = chargeRateH / 60 * interval;

	if(Helper::contains(config->getString("adaptable_devices"), ',', "car"))
		adaptionEnabled = true;
}

void Car::init(Json::Value config) {
	if(config.isMember("name"))
		configName = config["name"].asString();
	else
		throw("name missing in car config");
	if(config.isMember("capacity"))
		capacity = config["capacity"].asDouble() * 1000;
	else
		throw("capacity missing in car config");
	if(config.isMember("fuel_economy_real"))
		fuel = config["fuel_economy_real"].asDouble() * 10;
	else
		throw("fuel_economy_real missing in car config");
//	this->charge = capacity / 2 + (Random::Get()->get() % ((int) capacity / 2));
//	this->charge = capacity;
	this->minCharge = capacity * MyConfig::Get()->getDouble("car_min_charge");
	this->desiredCharge = capacity * MyConfig::Get()->getDouble("car_desired_charge");
	this->charge = minCharge + (Random::Get()->get() % ((int) (capacity - minCharge)));
//	this->charge = minCharge;
	Log::i(id, "\tinitialized with config " + configName);
}

void Car::setState(int state, int data) {
	if(state == DEV_COME_BACK) {
		charge -= data * fuel;
		if(charge < 0) {
			#ifdef CAR_CHECK_RIDES
				std::cerr << id << " did not have enough charge left to drive " << data << " km" << std::endl;
				failedRides.push_back(std::pair<int, double>(t->getTick(), -charge));
			#endif
			numFailedRides++;
			charge = 0;
		}
		numRides++;
		Device::setState(DEV_ON);
	} else
		Device::setState(state, data);
}

}
