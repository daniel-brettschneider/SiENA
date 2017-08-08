#include "Battery.h"

namespace ns3 {

Battery::Battery(std::string id) : Device(id, DEV_MODE_CONTINUOUS, NULL), lastCharge (0) {
	capacity = 6000;
	power = 2000;
	efficiency = 0.95;
	charge = capacity / 2 + (Random::Get()->get() % ((int) capacity / 2));
	minCharge = capacity * MyConfig::Get()->getDouble("battery_min_charge");
	desiredCharge = capacity * MyConfig::Get()->getDouble("battery_desired_charge");

	// charge rate
	chargeRateH = power;
	chargeRateInterval = chargeRateH / 60 * interval;

	if(Helper::contains(config->getString("adaptable_devices"), ',', "battery"))
		adaptionEnabled = true;

	setState(DEV_ON);
	chargeStatId = stats->registerStat("charge", id);
	pChargeStatId = stats->registerStat("pcharge", id);
}

void Battery::init(Json::Value config) {
	if(config.isMember("name"))
		configName = config["name"].asString();
	else
		throw("name missing in battery config");
	if(config.isMember("capacity"))
		capacity = config["capacity"].asDouble() * 1000;
	else
		throw("capacity missing in battery config");
	if(config.isMember("power"))
		power = config["power"].asDouble();
	else
		throw("power missing in battery config");
	if(config.isMember("efficiency"))
		efficiency = config["efficiency"].asDouble();
	else
		throw("efficiency missing in battery config");
//	this->charge = capacity / 2 + (Random::Get()->get() % ((int) capacity / 2));

	this->minCharge = capacity * MyConfig::Get()->getDouble("battery_min_charge");
	this->desiredCharge = capacity * MyConfig::Get()->getDouble("battery_desired_charge");
	this->charge = minCharge + (Random::Get()->get() % (int)(capacity - minCharge));
	chargeRateH = power;
	chargeRateInterval = chargeRateH / 60 * interval;
	Log::i(id, "\tinitialized with config " + configName);
}

void Battery::tick() {
	if((state == DEV_FULL || state == DEV_ON || state == DEV_OFF) && lastTick != t->getTick()) { //da bei 2 min tick die 59 min sonst der 59. tick und nicht 58 ist. //TODO testen mit citygrid algo
		lastCharge = charge;
		if(charge >= capacity) {
			setState(DEV_FULL);
			adaptedConsumption = 0;
		} else {
			if(charge + chargeRateInterval >= capacity)
				adaptedConsumption = (capacity - charge) * 60 / interval;
			else
				adaptedConsumption = chargeRateH;
			setState(DEV_ON);
		}
	}

	charge = lastCharge + adaptedConsumption / 60 * interval;
	lastTick = t->getTick();
	if(state==DEV_AWAY) {
		adaptedConsumption = 0;
		charge = lastCharge;
	}

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
	stats->addStat(chargeStatId, charge);
	stats->addStat(pChargeStatId, charge / capacity);
}

double Battery::getConsumption() {
	return adaptedConsumption;
}

//void Battery::setAdaptedConsumption(double consumption) {
//	adaptedConsumption = consumption;
//	adaptionFlex.setAssigned(consumption);
//	tick();
//}

bool Battery::isAdaptable() {
	if(adaptionEnabled && (state == DEV_FULL || (state == DEV_ON && lastCharge >= minCharge) || (state == DEV_ON && lastCharge + chargeRateInterval > minCharge)))
		return true;
	return false;
}

std::pair<AdaptionFlex*, AdaptionOnOff*> Battery::getAdaption() {
	if(lastAdaption < t->getTick()) {
		double fullChargeRate = chargeRateH;
		double residualUp = fullChargeRate, residualDown = fullChargeRate, residualDesiredCharge = 0;
		if(lastCharge + chargeRateInterval > capacity)
			residualUp = (capacity - lastCharge) * 60 / interval;
		if(lastCharge - chargeRateInterval < minCharge)
			residualDown = (lastCharge - minCharge) * 60 / interval;
		if(lastCharge > desiredCharge) {	//damit auf exakt desiredCharge geladen werden kann
			if(chargeRateInterval >= lastCharge - desiredCharge)
				residualDesiredCharge = -(lastCharge - desiredCharge) * 60 / interval;
			else
				residualDesiredCharge = -fullChargeRate;
			//residualDesiredCharge = min(chargeRateintervall,lastcharge-desiredcharge)* 60 / interval;
		}
		if(lastCharge < desiredCharge) {
			if(chargeRateInterval >= desiredCharge - lastCharge)
				residualDesiredCharge = (desiredCharge - lastCharge) * 60 / interval;
			else
				residualDesiredCharge = fullChargeRate;
		}
		if(state == DEV_FULL) {	//100%
			//std::cout << "1set minimum to " << -residualDown << std::endl;
			adaptionFlex.setMinimum(-residualDown);
			adaptionFlex.setMaximum(0);//Check
			adaptionFlex.setDesired(residualDesiredCharge);
		} else if(state == DEV_ON && lastCharge >= desiredCharge) {	//90-100%
			//std::cout << "2set minimum to " << -residualDown << std::endl;
			adaptionFlex.setMinimum(-residualDown);
			adaptionFlex.setMaximum(residualUp);
			adaptionFlex.setDesired(residualDesiredCharge);
		} else if(state == DEV_ON && lastCharge >= minCharge) { // 80-90%
			//std::cout << "3set minimum to " << -residualDown << std::endl;
			adaptionFlex.setMinimum(-residualDown);
			adaptionFlex.setMaximum(residualUp);
			adaptionFlex.setDesired(residualDesiredCharge);
		} else if(state == DEV_ON && lastCharge < minCharge){ //<80%
			//std::cout << "4set minimum to " << (minCharge - lastCharge) * 60 / interval << std::endl;
			//std::cout << "bat: " << this->getId() << std::endl;
			adaptionFlex.setMinimum((minCharge - lastCharge) * 60 / interval);//Problem for COHDA
			adaptionFlex.setMaximum(residualUp);
			adaptionFlex.setDesired(residualDesiredCharge);
		}
		if(isAdaptable()==false) {
			//std::cerr << "Gerät versucht sich anzupassen obwohl nicht Adaptable" << std::endl;
			adaptionFlex.setMinimum(residualUp);
			adaptionFlex.setMaximum(residualUp);
			adaptionFlex.setDesired(residualUp);
		}
		adaptionFlex.setAssigned(adaptionFlex.getDesired());
		adaptedConsumption = adaptionFlex.getDesired();
		/*if(this->getId()=="gridhome_0_1_car")
			std::cout << state << " " << this->getId() << " desired: " << adaptionFlex.getDesired() << " maximum: " << adaptionFlex.getMaximum()<< " minimum: " << adaptionFlex.getMinimum() << std::endl;*/
		lastAdaption = t->getTick();
	}
	return std::pair<AdaptionFlex*, AdaptionOnOff*>(&adaptionFlex, NULL);
}

}
