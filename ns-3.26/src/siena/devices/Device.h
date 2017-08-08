#ifndef DEVICE_H_
#define DEVICE_H_

#include <string>
#include <vector>
#include "ns3/Log.h"
#include "ns3/Tick.h"
#include "ns3/MyStats.h"
#include "ns3/uinteger.h"
#include "ns3/Adaption.h"
#include "ns3/AdaptionFlex.h"
#include "ns3/AdaptionOnOff.h"
#include "ns3/MyConfig.h"
#include "jsoncpp/json/value.h"

#define DEV_ON 1
#define DEV_OFF 0
#define DEV_AWAY -1
#define DEV_COME_BACK -10
#define DEV_MODE_SINGLE_RUN 0
#define DEV_MODE_CONTINUOUS 1

namespace ns3 {

class Device {

protected:
	std::string id;
	int state;
	int mode;
	std::vector<double>* consumption;
	uint consumptionStep;
	Tick* t;
	int lastTick;
	int lastState;
	MyStats* stats;
	int consumptionStatId;
	int stateStatId;
	int activationCounterId;
	double adaptedConsumption;
	AdaptionFlex adaptionFlex;
	AdaptionOnOff adaptionOnOff;
	std::string category;
	std::string configName;
	int interval;
	MyConfig* config;
	double standbyConsumption;
	bool adaptionEnabled;
	int lastAdaption;

public:
	Device(std::string id, int mode, std::vector<double>* consumption, double standbyConsumption = 0);
	virtual ~Device() {}
	virtual void init(Json::Value config) {}
	virtual void tick();
	virtual double getConsumption();
	virtual void setState(int state, int data = 0);
	std::string getId() { return id; }
	int getState() { return state; }
	std::vector<double>* getConsumptionList() { return consumption; }
	virtual bool isAdaptable() { return false; }
	virtual std::pair<AdaptionFlex*, AdaptionOnOff*> getAdaption() { return std::pair<AdaptionFlex*, AdaptionOnOff*>(&adaptionFlex, &adaptionOnOff); }
	virtual void setAdaptedConsumption(double consumption) { adaptedConsumption = consumption; tick(); }
	void setCategory(std::string category) { this->category = category; }
	std::string getCategory() { return category; }
	std::string getConfigName() { return configName; }

};

}

#endif
