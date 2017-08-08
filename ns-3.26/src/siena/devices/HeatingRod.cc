#include "../../siena/devices/HeatingRod.h"

namespace ns3 {

HeatingRod::HeatingRod(std::string id, WaterTank* tank) : Device(id, DEV_MODE_CONTINUOUS, NULL),
		minWork(0), maxWork(10000/60), cop(1), heat(-1), tank(tank), chargeState(HEAT_CHARGE), minRuntime(1),
		runtime(0), lastRuntime(0), maxTank(-1) {
	init();
}

HeatingRod::HeatingRod(std::string id, double minWork, double maxWork, double cop, WaterTank* tank)
		: Device(id, DEV_MODE_CONTINUOUS, NULL), minWork(minWork / 60), maxWork(maxWork / 60), cop(cop), tank(tank), minRuntime(1),
		runtime(0), lastRuntime(0), maxTank(-1) {
	init();
}

void HeatingRod::init() {
	heatStatId = stats->registerStat("heat", id);
	minWork *= interval;
	maxWork *= interval;
	minCritical = config->getDouble("heatingrod_min_critical");
	maxTank = config->getDouble("heatingrod_max");
	heat = maxWork * cop;

	if(Helper::contains(config->getString("switchable_devices"), ',', "heatingrod"))
		adaptionEnabled = true;

	setState(DEV_ON);
}

void HeatingRod::init(Json::Value config) {
	if(config.isMember("name"))
		configName = config["name"].asString();
	else
		throw("name missing in energy device config");
	if(config.isMember("min_el_power"))
		minWork = config["min_el_power"].asDouble() / 60 * interval;
	else
		throw("min_el_power missing in energy device config");
	if(config.isMember("max_el_power"))
		maxWork = config["max_el_power"].asDouble() / 60 * interval;
	else
		throw("max_el_power missing in energy device config");
	if(config.isMember("cop"))
		cop = config["cop"].asDouble();
	else
		throw("cop missing in energy device config");
	heat = maxWork * cop;
	Log::i(id, "\tinitialized with config " + configName);
}

void HeatingRod::tick() {
	tank->tick();

	if(lastTick != t->getTick()) {
		lastTick = t->getTick();
		lastRuntime = runtime;

		if(state == DEV_ON && tank->getChargeLevel() >= maxTank) {
			setState(DEV_OFF);
			runtime = 0;
		} else if(state == DEV_OFF && tank->getChargeLevel() < minCritical)
			setState(DEV_ON);
	}

	if(state == DEV_ON) {
		tank->doCharge(id, heat);
		runtime = lastRuntime + 1;
	} else {
		tank->doCharge(id, 0);
		runtime = 0;
	}

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
	stats->addStat(heatStatId, state == DEV_ON ? heat : 0);
}

double HeatingRod::getConsumption() {
	if(state == DEV_ON)
		return heat / cop / interval * 60;
	return 0;
}

bool HeatingRod::isAdaptable() {
	if(adaptionEnabled) {
		if(tank->getChargeLevel() >= minCritical && tank->getChargeLevel() < maxTank)
			return true;
	}
	return false;
}

std::pair<AdaptionFlex*, AdaptionOnOff*> HeatingRod::getAdaption() {
	if(lastAdaption < t->getTick()) {
		double potentialHeat = tank->getCapacity() * maxTank - tank->getCharge() / interval;
		if(potentialHeat < heat)
			potentialHeat = 0;
		else if(potentialHeat >= heat)
			potentialHeat = heat;
		if(state == DEV_OFF)
			adaptionOnOff.set(potentialHeat / cop / interval * 60, false, 0);
		else
			adaptionOnOff.set(-potentialHeat / cop / interval * 60, false, potentialHeat / cop / interval * 60);
		lastAdaption = t->getTick();
	}
	return std::pair<AdaptionFlex*, AdaptionOnOff*>(&adaptionFlex, &adaptionOnOff);
}

void HeatingRod::setAdaptedConsumption(double consumption) {
	this->adaptedConsumption = consumption;
	tick();
}

void HeatingRod::setState(int state, int data) {
	if(state == DEV_OFF)
		runtime = 0;
	else if(state == DEV_ON)
		runtime = lastRuntime + 1;
	Device::setState(state, data);
}

}
