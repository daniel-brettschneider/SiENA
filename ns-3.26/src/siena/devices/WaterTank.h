#ifndef WATERTANK_H_
#define WATERTANK_H_

#include "ns3/Device.h"
#include "ns3/Random.h"
#include "ns3/HeatSource.h"

namespace ns3 {

class WaterTank : public Device, public HeatSource {

private:
	double capacity;	// Wh
	double charge;
	int chargeStatId;
	int pChargeStatId;
	std::vector<double> zeroConsumption;
	double lastCharge;
	int lastChargeTick;
	std::map<std::string, double> increments;
	std::map<std::string, double> decrements;

public:
	WaterTank(std::string id);
	WaterTank(std::string id, double capacity);
	virtual ~WaterTank() {}
	virtual void tick();

	void doCharge(std::string id, double amount);
	virtual void take(std::string id, double amount);
	virtual double getRemainingCapacity() { tickReset(); return charge; }
	double getChargePotential() { tickReset(); return capacity - lastCharge; }
	double getChargeLevel() { tickReset(); return lastCharge / capacity; }

	void setCapacity(double capacity) { this->capacity = capacity; }
	void setCharge(double charge) { this->charge = charge; }
	double getCharge() { tickReset(); return charge; }
	double getCapacity() { return capacity; }

private:
	void tickReset();
	void internalSetCharge();
	void init();

};

}

#endif
