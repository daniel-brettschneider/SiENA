#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include "ns3/Communicator.h"
#include "ns3/AttributeContainer.h"
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
#include <ctime>
#include "ns3/GenericDataStorage.h"
#include "ns3/TraceHelper.h"
#include "ns3/PacketLog.h"
#include "ns3/PacketLogger.h"

namespace ns3 {

class Server : public Application, public Recipient, public GenericDataStorage {

protected:
	std::string id;
	int cluster;
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses;
	Address serverAddress;
	Communicator* communicator;
	Tick* t;
	int duration;
	std::string ip;
	Ptr<Socket> listenerSocket;
	Modules* algo;
	MyConfig* config;
	MyStats* stats;
	double adaption;
	int stage;
	Token* token;
	std::string moduleToken;
	std::string moduleTick;
	int interval;
	int simEnd;
	TraceHelper* traceHelper;
	Ipv4Address ipv4;
	PacketLog* packetLog;
	PacketLogger* packetLogger;

public:
	static TypeId GetTypeId();
	Server();
	virtual ~Server() { if(token != NULL) delete token; }
	virtual void StartApplication();
	virtual void StopApplication();
	void scheduleSend(std::string destination, Ptr<Packet> packet);
	void scheduleSend(Time t, std::string destination, Ptr<Packet> packet);
	std::string getId() { return id; }
	double getAdaption() { return adaption; }
	void setAdaption(double adaption) { this->adaption = adaption; }
	void setStage(int stage) { this->stage = stage; }
	int getStage() { return stage; }
	Ipv4Address getServerAddress() { return Ipv4Address::ConvertFrom(serverAddress); }
	std::string getIp() { return ip; }
	Token* getToken() { return token; }
	void setToken(Token* token) { this->token = token; }
	int getCluster() { return cluster; }
	int getNumberOfParticipants() { return homeAddresses->size(); }
	Ptr<AttributeContainer<Ipv4Address> > getHomeAddresses() { return homeAddresses; }
	virtual void tick();

protected:
	void send(std::string destination, Ptr<Packet> packet);
	void handleRead(Ptr<Socket> socket);
	void handlePacket(Ptr<Packet> packet);
	void handleAccept(Ptr<Socket> socket, const Address& address);

};

}

#endif
