#ifndef NS_3_22_SRC_SIENA_NETWORK_BACKGROUNDTRAFFIC_APP_H_
#define NS_3_22_SRC_SIENA_NETWORK_BACKGROUNDTRAFFIC_APP_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/Communicator.h"
#include "ns3/Log.h"
#include "ns3/DataBasis.h"
#include "ns3/Random.h"
#include "ns3/AttributeContainer.h"

namespace ns3 {

class BackgroundTraffic_App : public Application, public Recipient {

protected:
	std::string id;
	Ptr<AttributeContainer<std::string> > homeAddresses;
	Ptr<AttributeContainer<int> > homeDataRates;
	Communicator* communicator;
	Tick* t;
	int duration;
	Ptr<Socket> listenerSocket;
	MyConfig* config;
	int interval;
	int simEnd;
	double load;
	std::vector<double>* frameSize;
	std::vector<double>* frameDistribution;
	Random* random;
	int maxMs;
	static std::vector<std::vector<double>*>* frameSizeDistributionHelper;

public:
	static TypeId GetTypeId();
	BackgroundTraffic_App();
	virtual ~BackgroundTraffic_App() {}
	virtual void StartApplication();
	virtual void StopApplication();
	void scheduleSend(Time t, std::string destination, Ptr<Packet> packet);
	virtual void tick();

protected:
	void send(std::string destination, Ptr<Packet> packet);
	void handleRead(Ptr<Socket> socket);
	void handlePacket(Ptr<Packet> packet);
	static std::vector<std::vector<double>*>* getFrameSizeDistribution();

};

}

#endif
