#include "../../siena/network/Communicator.h"

namespace ns3 {

Communicator::~Communicator() {
	std::map<uint32_t, std::map<std::string, Ptr<Socket> >*>::iterator it;
	for(it = sockets.begin(); it != sockets.end(); ++it) {
		delete it->second;
	}
}

void Communicator::send(std::string destination, Ptr<Packet> packet, Ptr<Node> node) {
	if(simulated)
		this->sendOverNetwork(destination, packet, node);
	else
		this->sendDirectly(destination, packet);
}

void Communicator::sendOverNetwork(std::string destination, Ptr<Packet> packet, Ptr<Node> node) {
	std::map<uint32_t, std::map<std::string, Ptr<Socket> >*>::iterator it = sockets.find(node->GetId());
	if(it != sockets.end()) {
		std::map<std::string, Ptr<Socket> >::iterator jt = it->second->find(destination);
		if(jt != it->second->end())
			jt->second->Send(packet);
		else {
			Ptr<Socket> socket = createSocket(destination, node);
			it->second->insert(std::pair<std::string, Ptr<Socket> >(destination, socket));
			socket->Send(packet);
		}
	} else {
		std::map<std::string, Ptr<Socket> >* tmp = new std::map<std::string, Ptr<Socket> >;
		Ptr<Socket> socket = createSocket(destination, node);
		tmp->insert(std::pair<std::string, Ptr<Socket> >(destination, socket));
		sockets.insert(std::pair<uint32_t, std::map<std::string, Ptr<Socket> >*>(node->GetId(), tmp));
		socket->Send(packet);
	}
}

void Communicator::sendDirectly(std::string destination, Ptr<Packet> packet) {
	std::map<std::string, Recipient*>::iterator it = recipients.find(destination);
	if(it != recipients.end())
		it->second->handlePacket(packet);
	else
		throw("destination " + destination + " not registered at communicator");
}

Ptr<Socket> Communicator::createSocket(std::string destination, Ptr<Node> node) {
	TypeId id = TypeId::LookupByName(socketFactory);
	Ptr<Socket> sendSocket = Socket::CreateSocket(node, id);
	sendSocket->Bind();
	sendSocket->Connect(InetSocketAddress(Ipv4Address(destination.c_str()), 80));
	sendSocket->ShutdownRecv();
	return sendSocket;
}

}
