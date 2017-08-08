#ifndef DEVICEHELPER_H_
#define DEVICEHELPER_H_

#include <string>
#include <map>
#include <cmath>
#include <vector>
#include "ns3/MySingleton.h"
#include "ns3/Log.h"
#include "ns3/DataBasis.h"
#include "ns3/MyConfig.h"
#include "ns3/Random.h"
#include "ns3/Device.h"
#include "ns3/DeviceEvent.h"
#include "ns3/MyStats.h"
#include "ns3/PV.h"
#include "ns3/Car.h"
#include "ns3/Scenario.h"
#include "ns3/BaseLoad.h"
#include "ns3/WaterTank.h"
#include "ns3/HeatingRod.h"
#include "ns3/Tick.h"
#include "ns3/DeviceConfigHelper.h"
#include <cstdlib>
#include "ns3/CHPS.h"
#include "ns3/HeatPump.h"
#include "ns3/Battery.h"
#include "ns3/ShiftableDevice.h"
#include "ns3/BackgroundTraffic_Device.h"

namespace ns3 {

class ConventionalHome;
class Composite;

class CarEvent {
public:
	int departure, duration, distance, arrival;
	CarEvent(int departure, int duration, int distance) : departure(departure), duration(duration), distance(distance) {
		arrival = departure + duration;
	}
	virtual ~CarEvent() {}
};

class DeviceHelper : public MySingleton<DeviceHelper> {
	friend class MySingleton<DeviceHelper>;

private:
	static const std::string TAG;
	DataBasis* data;
	Random* random;
	MyConfig* config;
	MyStats* stats;
	static std::string dayTypes[9];

public:
	virtual ~DeviceHelper() {}
	void initHome(ConventionalHome* home, HomeConfig* homeConfig);
	void initComposite(Composite* composite, CompositeConfig* config);
	void createMovableDevice(int seed, std::string id, std::string type, std::string devId, double standby, ConventionalHome* home);
	void createCar(int seed, std::string id, std::string config, ConventionalHome* home);
	void createPV(int seed, std::string id, std::string config, ConventionalHome* home);
	WaterTank* createHeatDevice(int seed, std::string dev, std::string id, std::string config, ConventionalHome* home, WaterTank* tank, int consumers, double demand);
	void createBattery(int seed, std::string id, std::string config, ConventionalHome* home);

private:
	DeviceHelper();
	void createNormalDistributionIntegral(double* integral, double m, double s);
	void createIntegral(std::string type, std::vector<double>* integral, int start, int duration, double activations, bool correct);
	void createIntegral(std::vector<double>* integral, std::vector<double>* source, double times);
	int integralGoTo(std::vector<double>* data, int start, int end, double limit);
	void initCarDeparture(std::vector<CarEvent*>* events, uint tick, int dataIndex);
	int calcTankSize(int count, double heatDemandYear);
};

}

#endif
