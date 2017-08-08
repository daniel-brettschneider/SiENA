#include "../../siena/devices/ShiftableDevice.h"

namespace ns3 {

void ShiftableDevice::tick() {
	if(state == DEV_ON && (lastTick != t->getTick() || lastState != state)) {
		consumptionStep++;
		// turn off if end of list reached
		if(consumptionStep > consumption->size()) {
			if(additionalRuns == 0) {
				state = DEV_OFF;
				consumptionStep = 0;
			} else if(additionalRuns > 0) {
				additionalRuns--;
				consumptionStep = 1;
			}
		}
	}
	lastTick = t->getTick();
	lastState = state;

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
}

void ShiftableDevice::setState(int state, int data) {
	if(this->state == DEV_ON && state == DEV_ON)
		additionalRuns++;
	this->state = state;
	if(state == DEV_OFF)
		consumptionStep = 0;
	else if(state == DEV_ON)
		stats->updateCounter(activationCounterId, 1);
}

}
