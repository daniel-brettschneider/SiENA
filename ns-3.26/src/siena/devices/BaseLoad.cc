#include "../../siena/devices/BaseLoad.h"

namespace ns3 {

std::string BaseLoad::files[9] = {"devices/baseload_SSams", "devices/baseload_SSonn", "devices/baseload_SWerk",
								  "devices/baseload_USams", "devices/baseload_USonn", "devices/baseload_UWerk",
								  "devices/baseload_WSams", "devices/baseload_WSonn", "devices/baseload_WWerk"};

BaseLoad::BaseLoad(std::string home, int dataIndex) : Device(home + "_baseload", DEV_MODE_CONTINUOUS, NULL), dataIndex(dataIndex) {
	data = DataBasis::Get();
	random = Random::Get();
	for(int i = 0; i < 9; i++) {
		filePointers[i] = (*data->get(files[i]))[dataIndex];
	}
	randomList = random->getIntList(config->getInt("duration") / 1440 + 1);
	randomCount = 0;
	maxIndex = data->get("devices/baseload_SSams")->size();
}

void BaseLoad::tick() {
	if(t->getTick() != lastTick) {
		if(t->getMinute() % 1440 == 0) {
			dataIndex = randomList[randomCount++] % maxIndex;
			for(int i = 0; i < 9; i++) {
				filePointers[i] = (*data->get(files[i]))[dataIndex];
			}
		}
		lastTick = t->getTick();
		lastState = state;
	}


	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
}

double BaseLoad::getConsumption() {
	std::vector<double>* consumption = filePointers[t->getTimePeriod()];
	return (*consumption)[(t->getMinute() / interval) % consumption->size()];
}

}
