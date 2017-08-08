#include "../../siena/devices/HeatPump.h"

namespace ns3 {

HeatPump::HeatPump(std::string id, WaterTank* tank) : HeatingRod(id, 1500, 3000, 3, tank), forceDesired(true) {
	init();
}

HeatPump::HeatPump(std::string id, double minWork, double maxWork, double cop, WaterTank* tank) : HeatingRod(id, minWork, maxWork, cop, tank), forceDesired(true) {
	init();
}

void HeatPump::init() {
	HeatingRod::init();

	forceDesired = config->getBool("heatpump_force_desired");
	minCritical = config->getDouble("heatpump_min_critical");
	minDesired = config->getDouble("heatpump_min_desired");
	if(config->getInt("heatpump_min_runtime") < interval)
		minRuntime = 1;
	else
		minRuntime = config->getInt("heatpump_min_runtime") / interval;
	runtime = Random::Get()->get(0, 30);
	if(Helper::contains(config->getString("switchable_devices"), ',', "heatpump"))
		adaptionEnabled = true;
}

void HeatPump::tick() {
	tank->tick();

	if(lastTick != t->getTick()) {
		lastTick = t->getTick();
		lastRuntime = runtime;
		if(state == DEV_ON && tank->getChargePotential() < heat) {
			setState(DEV_OFF);
			runtime = 0;
		} else if(state == DEV_OFF && tank->getChargeLevel() < minCritical)
			setState(DEV_ON);
	}

	if(state == DEV_ON) {
		tank->doCharge(id, heat);
		runtime = lastRuntime + 1; // in ticks
	} else if(state == DEV_OFF) {
		tank->doCharge(id, 0);
		runtime = 0;
	}

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
	stats->addStat(heatStatId, state == DEV_ON ? heat : 0);
}

double HeatPump::getConsumption() {
	if(state == DEV_ON)
		return maxWork / interval * 60;
	return 0;
}

bool HeatPump::isAdaptable() {
	if(adaptionEnabled) {
		if(state == DEV_OFF && tank->getChargePotential() >= minRuntime * heat)
			return true;
		else if(state == DEV_ON && tank->getChargeLevel() > minCritical && runtime > minRuntime)
			return true;
	}
	return false;
}


std::pair<AdaptionFlex*, AdaptionOnOff*> HeatPump::getAdaption() {
	if(lastAdaption < t->getTick()) {
		if(state == DEV_OFF && tank->getChargePotential() >= minRuntime * heat && tank->getChargeLevel() >= minDesired)
			adaptionOnOff.set(maxWork / interval * 60, false, 0);
		else if(state == DEV_ON && tank->getChargeLevel() > minCritical && tank->getChargeLevel() <= minDesired && runtime > minRuntime)
			adaptionOnOff.set(-maxWork / interval * 60, false, maxWork / interval * 60);
		else if(state == DEV_ON && tank->getChargeLevel() > minDesired && runtime > minRuntime)
			adaptionOnOff.set(-maxWork / interval * 60, true, maxWork / interval * 60);
		else if(state == DEV_OFF && tank->getChargeLevel() < minDesired && tank->getChargeLevel() >= minCritical) {
			adaptionOnOff.set(maxWork / interval * 60, true, 0);
			if(forceDesired)
				setState(DEV_ON);
		}
		lastAdaption = t->getTick();
	}
	return std::pair<AdaptionFlex*, AdaptionOnOff*>(&adaptionFlex, &adaptionOnOff);
}

void HeatPump::setState(int state, int data) {
	if(state == DEV_OFF)
		runtime = 0;
	else if(state == DEV_ON)
		runtime = lastRuntime + 1;
	Device::setState(state, data);
}

}
