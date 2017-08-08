#ifndef NS_3_22_SRC_SIENA_NETWORK_BACKGROUNDTRAFFIC_DEVICE_H_
#define NS_3_22_SRC_SIENA_NETWORK_BACKGROUNDTRAFFIC_DEVICE_H_

#include "ns3/Device.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/Random.h"

namespace ns3 {

class GridHome;

class BackgroundTraffic_Device : public Device {

protected:
	GridHome* home;
	double load;
	std::string server;
	std::vector<double>* frameSize;
	std::vector<double>* frameDistribution;
	Random* random;
	double maxData;
	int maxMs;

	static std::vector<std::vector<double>*>* frameSizeDistributionHelper;

public:
	BackgroundTraffic_Device(std::string id, GridHome* home);
	virtual ~BackgroundTraffic_Device() {}
	virtual void tick();
	virtual double getConsumption() { return 0; }
	void setServer(std::string server) { this->server = server; }

protected:
	static std::vector<std::vector<double>*>* getFrameSizeDistribution();

};

}

#endif
