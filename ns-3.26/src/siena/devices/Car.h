#ifndef CAR_H_
#define CAR_H_

#include "ns3/Battery.h"

//#define CAR_CHECK_RIDES

namespace ns3 {

class Car : public Battery {

protected:
	double fuel;
	std::vector<std::pair<int, double> > failedRides;
	int numRides;
	int numFailedRides;

public:
	Car(std::string id);
	virtual ~Car() {}
	virtual void init(Json::Value config);
	virtual void setState(int state, int data = 0);
	double getFuelEconomy() { return fuel; }
	std::vector<std::pair<int, double> >* getFailedRides() { return &failedRides; }
	int getNumRides() { return numRides; }
	int getNumFailedRides() { return numFailedRides; }

};

}

#endif
