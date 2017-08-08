#ifndef CHPS_H_
#define CHPS_H_

#include "ns3/HeatingRod.h"

namespace ns3 {

class CHPS : public HeatingRod {

private:
	double minHeat;
	double maxHeat;
	bool forceDesired;

public:
	CHPS(std::string id, WaterTank* tank);
	CHPS(std::string id, double minWork, double maxWork, double minHeat, double maxHeat, WaterTank* tank);
	virtual ~CHPS() {}
	virtual void init(Json::Value config);
	virtual void tick();
	virtual double getConsumption();
	virtual bool isAdaptable();
	virtual std::pair<AdaptionFlex*, AdaptionOnOff*> getAdaption();
	virtual void setState(int state, int data = 0);
	virtual double getMaxHeat();

private:
	virtual void init();

};

}

#endif
