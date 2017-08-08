#ifndef BASELOAD_H_
#define BASELOAD_H_

#include "ns3/Device.h"
#include "ns3/DataBasis.h"
#include "ns3/Random.h"

namespace ns3 {

class BaseLoad : public Device {

private:
	int dataIndex;
	static std::string files[9];
	std::vector<double>* filePointers[9];
	DataBasis* data;
	Random* random;
	std::vector<int> randomList;
	int randomCount;
	int maxIndex;

public:
	BaseLoad(std::string home, int dataIndex);
	virtual ~BaseLoad() {}
	virtual void tick();
	virtual double getConsumption();

};

}

#endif
