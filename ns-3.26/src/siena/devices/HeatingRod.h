#ifndef HEATINGROD_H_
#define HEATINGROD_H_

#include "ns3/Device.h"
#include "ns3/WaterTank.h"

namespace ns3 {

#define HEAT_CHARGE 0
#define HEAT_DISCHARGE 1

class HeatingRod : public Device {

protected:
	double minWork;
	double maxWork;
	double cop;
	double heat;
	WaterTank* tank;
	int heatStatId;
	int chargeState;
	double minCritical;
	double minDesired;
	int minRuntime;
	int runtime;
	int lastRuntime;

private:
	double maxTank;

public:
	HeatingRod(std::string id, WaterTank* tank);
	HeatingRod(std::string id, double minWork, double maxWork, double cop, WaterTank* tank);
	virtual ~HeatingRod() {}
	virtual void init(Json::Value config);
	virtual void tick();
	virtual double getConsumption();
	virtual bool isAdaptable();
	virtual std::pair<AdaptionFlex*, AdaptionOnOff*> getAdaption();
	virtual void setAdaptedConsumption(double consumption);
	virtual void setState(int state, int data = 0);
	virtual WaterTank* getTank() { return tank; }
	double getMaxWork() { return maxWork; }
	double getHeat() { return heat; }
	double getCOP() { return cop; }

protected:
	virtual void init();

};

}

#endif
