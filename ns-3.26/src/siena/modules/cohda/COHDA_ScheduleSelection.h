#ifndef COHDA_SCHEDULESELECTION_H_
#define COHDA_SCHEDULESELECTION_H_

#include <vector>
#include "ns3/Token.h"
#include "ns3/Adaption.h"
#include "ns3/AdaptionCategories.h"

namespace ns3 {

class COHDA_ScheduleSelection {			//Fahrplanauswahl (omega)

private:
	double theta;		//Consumption
	uint lambda;		//counter

public:
	COHDA_ScheduleSelection () : theta(0.0), lambda(0) {}
	COHDA_ScheduleSelection (double theta, uint lambda) : theta(theta), lambda(lambda) {}
	virtual ~COHDA_ScheduleSelection() {}

	uint getLambda() { return this->lambda; }
	double getTheta() { return theta; }

	void setTheta(double t) { this->theta = t; }
	void setLambda(uint l) { this->lambda = l; }
	void increaseLambda() { lambda++; }

};

}

#endif
