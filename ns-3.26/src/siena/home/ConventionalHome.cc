#include "../../siena/home/ConventionalHome.h"

namespace ns3 {

ConventionalHome::ConventionalHome(std::string id) : id(id), lastTick(-1), residents(-1), unit(-1), space(-1),
		heatEfficiency(-1), heatSource(NULL), heatStatId(-1), heatUnsatisfiedStatId(-1), suppressTick(false) {
	c = MyConfig::Get();
	t = Tick::Get();
	stats = MyStats::Get();
	interval = c->getInt("interval");
	simEnd = c->getInt("duration") / interval - 1;
}

void ConventionalHome::init(HomeConfig* config) {
	Log::f(id, "initializing...");

	residents = config->getResidents();
	space = config->getSpace();
	unit = config->getUnit();
	heatEfficiency = config->getHeatEfficiency();
	std::stringstream s;
	s << "\tres: " << residents << ", space: " << space << ", unit: " << unit << ", heatEff: " << heatEfficiency;
	Log::i(id, s.str());

	// devices
	DeviceHelper::Get()->initHome(this, config);

	// schedule
	this->scheduleNextTick(0);
}

ConventionalHome::~ConventionalHome() {
	std::map<int, std::vector<DeviceEvent*>*>::iterator jt;
	for(jt = events.begin(); jt != events.end(); ++jt) {
		for(uint i = 0; i < jt->second->size(); i++) {
			DeviceEvent* e = (*jt->second)[i];
			delete e;
		}
		delete jt->second;
	}

	std::map<std::string, Device*>::iterator it;
	for(it = devices.begin(); it != devices.end(); ++it) {
		delete it->second;
	}
}

void ConventionalHome::tick() {
	Log::i(id, "tick");

	int tick = t->getTick();

	if(tick > simEnd) // catch late packets after sim end
		return;

	if(tick > lastTick) {

		// check events
		std::map<int, std::vector<DeviceEvent*>*>::iterator it;
		int minute = t->getStartInt() + t->getSimMinute();
		for(int i = 0; i < interval; i++) {
			it = events.find(minute + i);
			if(it != events.end()) {
				std::vector<DeviceEvent*>* list = it->second;
				for(uint j = 0; j < list->size(); j++) {
					DeviceEvent* e = (*list)[j];
					switch(e->getWhat()) {
						case EV_TURN_ON:
							e->getDevice()->setState(DEV_ON);
							e->getDevice()->tick();
							break;
						case EV_TURN_OFF:
							e->getDevice()->setState(DEV_OFF);
							e->getDevice()->tick();
							break;
						case EV_GO_AWAY:
							e->getDevice()->setState(DEV_AWAY);
							e->getDevice()->tick();
							break;
						case EV_COMING_BACK:
							e->getDevice()->setState(DEV_COME_BACK, e->getData());
							e->getDevice()->tick();
							break;
						case EV_SCHEDULE:
							this->schedule(e);
							break;
					}
				}
			}
		}
	}

	if(heatSource != NULL) {
		double demand = heatDemand[t->getSimMinute() / interval];
		double available = heatSource->getRemainingCapacity();
		if(available >= demand) {
			heatSource->take(id, demand);
			stats->addStat(heatStatId, demand);
			stats->addStat(heatUnsatisfiedStatId, 0);
			if(tick > lastTick)
				Log::i(id, "\theat demand: ", demand);
		} else {
			heatSource->take(id, available);
			stats->addStat(heatStatId, available);
			stats->addStat(heatUnsatisfiedStatId, available - demand);
			if(tick > lastTick) {
				#ifndef TURBO
					std::stringstream s;
					s << "\theat demand: " << demand << ", unsatisfied: " << available - demand;
					Log::i(id, s.str());
				#else
					Log::i(id, "\toptimized out");
				#endif
			}
		}
	}

	if(tick > lastTick)
		Log::i(id, "\tconsumption:");
	std::map<std::string, Device*>::iterator it;
	for(it = devices.begin(); it != devices.end(); ++it) {
		it->second->tick();
		if(tick > lastTick) {
			#ifndef TURBO
				std::stringstream s;
				s << "\t" << it->second->getId() << ": " << it->second->getConsumption() << ", state: " << it->second->getState();
				Log::i(id, s.str());
				s.str("");
			#else
				Log::i(id, "\toptimized out");
			#endif
		}
	}

	if(tick > lastTick) {
		lastTick = tick;
		scheduleNextTick(interval * 60);
	}

}

void ConventionalHome::scheduleNextTick(int seconds) {
	if(!suppressTick && lastTick < simEnd)
		Simulator::Schedule(Seconds(seconds), &ConventionalHome::tick, this);
}

void ConventionalHome::schedule(DeviceEvent* event) {
	event->getDevice()->setState(DEV_ON);
}

double ConventionalHome::getConsumption() {
	double consumption = 0;
	std::map<std::string, Device*>::iterator it;
	for(it = devices.begin(); it != devices.end(); ++it) {
		consumption += it->second->getConsumption();
	}
	return consumption;
}

void ConventionalHome::setHeatSource(HeatSource* heatSource) {
	this->heatSource = heatSource;
	HeatHelper::Get()->generateHeatDemand(&heatDemand, unit, residents, heatEfficiency, space);
	heatStatId = stats->registerStat("heat", id + "_demand");
	heatUnsatisfiedStatId = stats->registerStat("heat", id + "_unsatisfied");
}

void ConventionalHome::addEvent(DeviceEvent* event) {
	events.insert(std::pair<int, std::vector<DeviceEvent*>*>(event->getTick(), new std::vector<DeviceEvent*>));
	events[event->getTick()]->push_back(event);
}

}
