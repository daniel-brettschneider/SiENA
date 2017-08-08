#include "../../siena/devices/Device.h"

namespace ns3 {

Device::Device(std::string id, int mode, std::vector<double>* consumption, double standbyConsumption)
	: id(id), state(DEV_OFF), mode(mode), consumption(consumption), consumptionStep(0), lastTick(-1), lastState(-10),
	  adaptedConsumption(0), standbyConsumption(standbyConsumption), adaptionEnabled(false), lastAdaption(-1) {
	t = Tick::Get();
	stats = MyStats::Get();
	consumptionStatId = stats->registerStat(STAT_CONSUMPTION, id);
	stateStatId = stats->registerStat(STAT_STATE, id);
	activationCounterId = stats->registerCounter(STAT_ACTIVATION, id);
	config = MyConfig::Get();
	interval = config->getInt("interval");
}

void Device::tick() {
	if(state == DEV_ON && (lastTick != t->getTick() || lastState != state)) {
		consumptionStep++;
		// single run, turn off if end of list reached
		if(mode == DEV_MODE_SINGLE_RUN && consumptionStep > consumption->size()) {
			state = DEV_OFF;
			consumptionStep = 0;
		// continuous run
		} else if(mode == DEV_MODE_CONTINUOUS && consumptionStep > consumption->size())
			consumptionStep = 1;
	}
	lastTick = t->getTick();
	lastState = state;

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
}

double Device::getConsumption() {
	if(state == DEV_ON)
		return (*consumption)[consumptionStep - 1];
	else
		return standbyConsumption;
}

void Device::setState(int state, int data) {
	this->state = state;
	if(state == DEV_OFF)
		consumptionStep = 0;
	else if(state == DEV_ON)
		stats->updateCounter(activationCounterId, 1);
}


}
