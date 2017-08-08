#include "../../siena/devices/PV.h"

namespace ns3 {

PV::PV(std::string id) : Device(id, DEV_MODE_CONTINUOUS, NULL) {
	size = 9600;
	std::vector<std::vector<double>*>* data = DataBasis::Get()->get("devices/pv");
	int r = Random::Get()->get() % data->size();
	consumption = (*data)[r];
	this->setState(DEV_ON);
	if(Helper::contains(config->getString("adaptable_devices"), ',', "pv"))
		adaptionEnabled = true;
}

void PV::init(Json::Value config) {
	if(config.isMember("name"))
		configName = config["name"].asString();
	else
		throw("name missing in pv config");
	if(config.isMember("peak_power"))
		size = config["peak_power"].asDouble() * 1000;
	else
		throw("peak_power missing in pv config");

	Log::i(id, "\tinitialized with config " + configName);
}

void PV::tick() {
	if(lastTick != t->getTick()) {
		if(state == DEV_ON)
			adaptedConsumption = -round((*consumption)[(t->getMinute() / interval) % consumption->size()] * size);
		else
			adaptedConsumption = 0;
	}
	lastTick = t->getTick();

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
}

double PV::getConsumption() {
	if(state == DEV_ON)
		return adaptedConsumption;
	return 0;
}

bool PV::isAdaptable() {
	if(adaptionEnabled && state == DEV_ON && getConsumption() < 0)
		return true;
	return false;
}

std::pair<AdaptionFlex*, AdaptionOnOff*> PV::getAdaption() {
	double current = getConsumption();
	adaptionFlex.setMinimum(current);
	adaptionFlex.setMaximum(0);
	adaptionFlex.setDesired(current);
	adaptionFlex.setAssigned(current);
	return std::pair<AdaptionFlex*, AdaptionOnOff*>(&adaptionFlex, NULL);
}

}
