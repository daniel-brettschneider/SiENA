#ifndef HEATPUMP_H_
#define HEATPUMP_H_

#include "ns3/HeatingRod.h"

namespace ns3 {

class HeatPump : public HeatingRod {

protected:
	bool forceDesired;

public:
	HeatPump(std::string id, WaterTank* tank);
	HeatPump(std::string id, double minWork, double maxWork, double cop, WaterTank* tank);
	virtual ~HeatPump() {}
	virtual void init(Json::Value config) { HeatingRod::init(config); }
	virtual void tick();
	virtual double getConsumption();
	virtual bool isAdaptable();
	virtual std::pair<AdaptionFlex*, AdaptionOnOff*> getAdaption();
	virtual void setState(int state, int data = 0);

protected:
	virtual void init();

};

}

#endif
