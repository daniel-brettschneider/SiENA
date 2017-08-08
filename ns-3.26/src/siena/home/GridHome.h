#ifndef GRIDHOME_H_
#define GRIDHOME_H_

#include "ns3/ConventionalHome.h"
#include "ns3/Communicator.h"
#include "ns3/Helper.h"
#include "ns3/Modules.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/Token.h"
#include "ns3/Adaption.h"
#include "ns3/TokenTag.h"
#include "ns3/GenericDataStorage.h"
#include "ns3/TraceHelper.h"
#include "ns3/PacketLog.h"
#include "ns3/PacketLogger.h"

namespace ns3 {

class GridHome : public ConventionalHome, public Application, public Recipient, public GenericDataStorage {

protected:
	Communicator* communicator;
	Ptr<Socket> listenerSocket;
	Modules* algo;
	Token* token;
	int lastAdaption;
	Adaption adaption;
	std::string moduleShift;
	std::string moduleToken;
	std::string moduleTick;
	Ipv4Address ip;
	TraceHelper* traceHelper;
	PacketLog* packetLog;
	PacketLogger* packetLogger;

public:
	static TypeId GetTypeId();
	GridHome();
	virtual ~GridHome() {}
	virtual void StartApplication();
	virtual void StopApplication();
	void scheduleSend(std::string destination, Ptr<Packet> packet);
	void scheduleSend(Time t, std::string destination, Ptr<Packet> packet);
	void setToken(Token* token) { this->token = token; }
	Adaption* getAdaption();
	int getSimEnd() { return simEnd; }
	virtual void tick();
	Ipv4Address getIp() { return ip; }

protected:
	virtual void schedule(DeviceEvent* event);
	void send(std::string destination, Ptr<Packet> packet);
	void handleRead(Ptr<Socket> socket);
	void handlePacket(Ptr<Packet> packet);
	void handleAccept(Ptr<Socket> socket, const Address& address);
	void setAdaption();

};

}

#endif
