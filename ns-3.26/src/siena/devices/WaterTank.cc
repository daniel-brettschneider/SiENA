#include "../../siena/devices/WaterTank.h"

namespace ns3 {

WaterTank::WaterTank(std::string id) : Device(id, DEV_MODE_CONTINUOUS, NULL),
		capacity(7000), lastCharge(-1), lastChargeTick(-1) {
	init();
}

WaterTank::WaterTank(std::string id, double capacity) : Device(id, DEV_MODE_CONTINUOUS, NULL),
		capacity(capacity), lastCharge(-1), lastChargeTick(-1) {
	init();
}

void WaterTank::init() {
	this->charge = Random::Get()->get(capacity * 0.5, capacity);
	zeroConsumption.push_back(0);
	this->consumption = &zeroConsumption;
	setState(DEV_OFF);
	chargeStatId = stats->registerStat("charge", id);
	pChargeStatId = stats->registerStat("pcharge", id);
	configName = Helper::toString(capacity);
}

void WaterTank::doCharge(std::string id, double amount) {
	tickReset();
	if(increments.insert(std::pair<std::string, double>(id, amount)).second == false)
		increments[id] = amount;
	internalSetCharge();
}

void WaterTank::take(std::string id, double amount) {
	tickReset();
	if(decrements.insert(std::pair<std::string, double>(id, amount)).second == false)
		decrements[id] = amount;
	internalSetCharge();
}

void WaterTank::tick() {
	Device::tick();
	tickReset();
	stats->addStat(chargeStatId, charge);
	stats->addStat(pChargeStatId, charge / capacity);
}

void WaterTank::tickReset() {
	if(lastChargeTick != t->getTick()) {
		lastCharge = charge;
		increments.clear();
		decrements.clear();
		lastChargeTick = t->getTick();
	}
}

void WaterTank::internalSetCharge() {
	std::map<std::string, double>::iterator it;
	double inc = 0;
	for(it = increments.begin(); it != increments.end(); ++it) {
		inc += it->second;
	}
	double dec = 0;
	for(it = decrements.begin(); it != decrements.end(); ++it) {
		dec += it->second;
	}
	charge = lastCharge + inc - dec;
	if(charge < 0)
		throw("thermal storage tank " + id + " capacity below 0");
	else if(charge > capacity) {
		std::cerr << "Try to load " << inc << " and consume " << dec << " to the thermal storage tank " << id << "; Old charge level: " << lastCharge << " new charge level: " << charge << " capacity: " << capacity << std::endl;
		//throw("thermal storage tank " + id + " is overcharged!");
		charge = capacity;
	}

	stats->addStat(chargeStatId, charge);
	stats->addStat(pChargeStatId, charge / capacity);
}

}
