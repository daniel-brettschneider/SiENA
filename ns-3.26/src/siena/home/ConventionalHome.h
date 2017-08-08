#ifndef CONVENTIONALHOME_H_
#define CONVENTIONALHOME_H_

#include <string>
#include <map>
#include "ns3/Log.h"
#include "ns3/Tick.h"
#include "ns3/DataBasis.h"
#include "ns3/Random.h"
#include "ns3/Device.h"
#include "ns3/MyConfig.h"
#include "ns3/simulator.h"
#include "ns3/DeviceHelper.h"
#include "ns3/DeviceEvent.h"
#include "ns3/HomeConfig.h"
#include "ns3/HeatSource.h"
#include "ns3/HeatHelper.h"
#include "ns3/MyStats.h"
#include "ns3/Turbo.h"

namespace ns3 {

class ConventionalHome {

protected:
	std::string id;
	std::map<std::string, Device*> devices;
	std::map<int, std::vector<DeviceEvent*>*> events;
	MyConfig* c;
	Tick* t;
	int lastTick;
	int residents;
	int unit;
	int space;
	double heatEfficiency;
	std::vector<double> heatDemand;
	HeatSource* heatSource;
	int heatStatId;
	int heatUnsatisfiedStatId;
	MyStats* stats;
	int interval;
	int simEnd;
	bool suppressTick;

public:
	ConventionalHome(std::string id);
	virtual ~ConventionalHome();
	virtual void init(HomeConfig* config);
	virtual void tick();
	virtual double getConsumption();
	std::string getId() { return id; }
	std::map<std::string, Device*>* getDevices() { return &devices; }
	std::map<int, std::vector<DeviceEvent*>*>* getEvents() { return &events; }
	void setHeatSource(HeatSource* heatSource);
	std::vector<double>* getHeatDemand() { return &heatDemand; }
	int getResidents() { return residents; }
	int getUnit() { return unit; }
	int getSpace() { return space; }
	double getHeatEfficiency() { return heatEfficiency; }
	void addDevice(Device* device) { devices.insert(std::pair<std::string, Device*>(device->getId(), device)); }
	void addEvent(DeviceEvent* event);
	int getSimEnd() { return simEnd; }
	void removeHeatSource() {
		for(uint i = 0; i < heatDemand.size(); i++) {
			heatDemand[i] = 0;
		}
	}

protected:
	virtual void scheduleNextTick(int seconds);
	virtual void schedule(DeviceEvent* event);

};

}

#endif
