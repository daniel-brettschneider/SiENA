#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_

#include <string>
#include <map>
#include "ns3/MyConfig.h"
#include "ns3/Recipient.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/MySingleton.h"

namespace ns3 {

class Communicator : public MySingleton<Communicator> {
	friend class MySingleton<Communicator>;

private:
	bool simulated;
	std::string socketFactory;
	std::map<std::string, Recipient*> recipients;
	std::map<uint32_t, std::map<std::string, Ptr<Socket> >*> sockets;

public:
	virtual ~Communicator();
	std::string getSocketFactory() { return socketFactory; }
	void send(std::string destination, Ptr<Packet>, Ptr<Node> node);
	void registerRecipient(Recipient* recipient, std::string ip) {
		recipients.insert(std::pair<std::string, Recipient*>(ip, recipient));
	}

private:
	Communicator() {
		MyConfig* config = MyConfig::Get();
		simulated = config->getBool("simulate_communication");
		socketFactory = config->getString("socket_type") == "tcp" ? "ns3::TcpSocketFactory" : "ns3::UdpSocketFactory";
	}
	void sendOverNetwork(std::string destination, Ptr<Packet>, Ptr<Node> node);
	void sendDirectly(std::string destination, Ptr<Packet>);
	Ptr<Socket> createSocket(std::string destination, Ptr<Node> node);

};

}

#endif
