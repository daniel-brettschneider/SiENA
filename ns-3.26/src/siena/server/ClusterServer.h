#ifndef CLUSTERSERVER_H_
#define CLUSTERSERVER_H_

#include <string>
#include "ns3/Communicator.h"
#include "ns3/Tick.h"
#include "ns3/Helper.h"
#include "ns3/Token.h"
#include "ns3/Modules.h"
#include "ns3/Log.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/TokenTag.h"
#include "ns3/MyStats.h"
#include "ns3/AttributeContainer.h"
#include "ns3/GenericDataStorage.h"
#include "ns3/TraceHelper.h"
#include "ns3/PacketLog.h"
#include "ns3/PacketLogger.h"

namespace ns3 {

class ClusterServer : public Application, public Recipient, public GenericDataStorage {

private:
	std::string id;
	Ptr<AttributeContainer<Ipv4Address> > children;
	Communicator* communicator;
	Tick* t;
	int duration;
	std::string ip;
	Ptr<Socket> listenerSocket;
	Modules* algo;
	MyConfig* config;
	std::string moduleToken;
	std::string moduleTick;
	int interval;
	int simEnd;
	Ipv4Address ipv4;
	TraceHelper* traceHelper;
	PacketLog* packetLog;
	PacketLogger* packetLogger;

public:
	static TypeId GetTypeId();
	ClusterServer();
	virtual ~ClusterServer() {}
	virtual void StartApplication();
	virtual void StopApplication();
	void scheduleSend(std::string destination, Ptr<Packet> packet);
	void scheduleSend(Time t, std::string destination, Ptr<Packet> packet);
	Ptr<AttributeContainer<Ipv4Address> > getChildren() { return children; }
	std::string getId() { return id; }
	Ipv4Address getIp() { return ipv4; }
	virtual void tick();

protected:
	void send(std::string destination, Ptr<Packet> packet);
	void handleRead(Ptr<Socket> socket);
	void handlePacket(Ptr<Packet> packet);
	void handleAccept(Ptr<Socket> socket, const Address& address);

};

}

#endif
