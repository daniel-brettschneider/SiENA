#ifndef HEATSOURCE_H_
#define HEATSOURCE_H_

namespace ns3 {

class HeatSource {

public:
	virtual ~HeatSource() {}
	virtual void take(std::string id, double amount) = 0;
	virtual double getRemainingCapacity() = 0;

};

}

#endif
