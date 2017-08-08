#include "../../siena/devices/CHPS.h"

namespace ns3 {

CHPS::CHPS(std::string id, WaterTank* tank) : HeatingRod(id, 1500, 3000, 0, tank), minHeat(4700/60), maxHeat(8000/60), forceDesired(true) {
	init();
}

CHPS::CHPS(std::string id, double minWork, double maxWork, double minHeat, double maxHeat, WaterTank* tank)
		: HeatingRod(id, -minWork, -maxWork, 0, tank), minHeat(minHeat/60), maxHeat(maxHeat/60), forceDesired(true) {
	init();
}

void CHPS::init() {
	HeatingRod::init();

	forceDesired = config->getBool("chps_force_desired");
	minHeat *= interval;
	maxHeat *= interval;
	minCritical = config->getDouble("chps_min_critical");
	minDesired = config->getDouble("chps_min_desired");
	if(config->getInt("chps_min_runtime") < interval)
		minRuntime = 1;
	else
		minRuntime = config->getInt("chps_min_runtime") / interval;
	runtime = Random::Get()->get(0, 30);
	if(Helper::contains(config->getString("switchable_devices"), ',', "chps"))
		adaptionEnabled = true;
}

void CHPS::init(Json::Value config) {
	if(config.isMember("name"))
		configName = config["name"].asString();
	else
		throw("name missing in chps config");
	if(config.isMember("min_el_power"))
		minWork = -config["min_el_power"].asDouble() / 60 * interval;
	else
		throw("min_el_power missing in chps config");
	if(config.isMember("max_el_power"))
		maxWork = -config["max_el_power"].asDouble() / 60 * interval;
	else
		throw("max_el_power missing in chps config");
	if(config.isMember("min_heat_power"))
		minHeat = config["min_heat_power"].asDouble() / 60 * interval;
	else
		throw("min_heat_power missing in chps config");
	if(config.isMember("max_heat_power"))
		maxHeat = config["max_heat_power"].asDouble() / 60 * interval;
	else
		throw("max_heat_power missing in chps config");
	Log::i(id, "\tinitialized with config " + configName);
}

void CHPS::tick() {
	tank->tick();

	if(lastTick != t->getTick()) {
		lastTick = t->getTick();
		lastRuntime = runtime;

		if(state == DEV_ON && tank->getChargePotential() < maxHeat) {
			setState(DEV_OFF);
			runtime = 0;
		} else if(state == DEV_OFF && tank->getChargeLevel() < minCritical)
			setState(DEV_ON);
	}

	if(state == DEV_ON) {
		tank->doCharge(id, maxHeat);
		runtime = lastRuntime + 1;
	} else if(state == DEV_OFF) {
		tank->doCharge(id, 0);
		runtime = 0;
	}

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
	stats->addStat(heatStatId, state == DEV_ON ? maxHeat : 0);
}

double CHPS::getConsumption() {
	if(state == DEV_ON)
		return -maxWork / interval * 60;
	return 0;
}

bool CHPS::isAdaptable() {
	if(adaptionEnabled) {
		if(state == DEV_OFF && tank->getChargePotential() >= minRuntime * maxHeat)
			return true;
		else if(state == DEV_ON && tank->getChargeLevel() > minCritical && runtime > minRuntime)
			return true;
	}
	return false;
}

std::pair<AdaptionFlex*, AdaptionOnOff*> CHPS::getAdaption() {
	if(lastAdaption < t->getTick()) {
		if(state == DEV_OFF && tank->getChargePotential() >= minRuntime * maxHeat && tank->getChargeLevel() >= minDesired)
			adaptionOnOff.set(-maxWork / interval * 60, false, 0);
		else if(state == DEV_ON && tank->getChargeLevel() > minCritical && runtime > minRuntime)
			adaptionOnOff.set(maxWork / interval * 60, false, -maxWork / interval * 60);
		else if(state == DEV_OFF && tank->getChargeLevel() < minDesired && tank->getChargeLevel() >= minCritical) {
			adaptionOnOff.set(-maxWork / interval * 60, true, 0);
			if(forceDesired)
				setState(DEV_ON);
		}
		lastAdaption = t->getTick();
	}
	return std::pair<AdaptionFlex*, AdaptionOnOff*>(&adaptionFlex, &adaptionOnOff);
}

void CHPS::setState(int state, int data) {
	if(state == DEV_OFF)
		runtime = 0;
	else if(state == DEV_ON)
		runtime = lastRuntime + 1;
	Device::setState(state, data);
}

double CHPS::getMaxHeat() {
	return maxHeat;
}

}
