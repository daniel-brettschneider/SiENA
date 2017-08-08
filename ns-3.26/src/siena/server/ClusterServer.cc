#include "../../siena/server/ClusterServer.h"

namespace ns3 {

TypeId ClusterServer::GetTypeId() {
	static TypeId id = TypeId("ns3::ClusterServer")
		.SetParent<Application>()
		.AddConstructor<ClusterServer>()
		.AddAttribute("id", "id", StringValue("clusterserver"), MakeStringAccessor(&ClusterServer::id), MakeStringChecker())
		.AddAttribute("children", "children", PointerValue(NULL), MakePointerAccessor(&ClusterServer::children), MakePointerChecker<AttributeContainer<Ipv4Address> >());
	return id;
}

ClusterServer::ClusterServer() : id("error"), ip("") {
	communicator = Communicator::Get();
	t = Tick::Get();
	duration = MyConfig::Get()->getInt("duration");
	algo = Modules::Get();
	config = MyConfig::Get();
	moduleToken = config->getString("clusterserver_handle_token");
	moduleTick = config->getString("clusterserver_tick");
	interval = config->getInt("interval");
	simEnd = config->getInt("start") + config->getInt("duration") - interval;
	traceHelper = TraceHelper::Get();
	packetLog = PacketLog::Get();
	packetLogger = PacketLogger::Get();
}

void ClusterServer::StartApplication() {
	Log::f(id, "initializing...");
	TypeId id = TypeId::LookupByName(communicator->getSocketFactory());
	listenerSocket = Socket::CreateSocket(GetNode(), id);
	listenerSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 80));
	listenerSocket->Listen();
	listenerSocket->ShutdownSend();
	listenerSocket->SetRecvCallback(MakeCallback(&ClusterServer::handleRead, this));
	listenerSocket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address &> (), MakeCallback (&ClusterServer::handleAccept, this));

	ipv4 = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	ip = Helper::toString(ipv4);
	communicator->registerRecipient(this, ip);

	//Simulator::Schedule(Seconds(0), &ClusterServer::tick, this);
}

void ClusterServer::StopApplication() {
	listenerSocket->Close();
}

void ClusterServer::scheduleSend(std::string destination, Ptr<Packet> packet) {
	Simulator::Schedule(Seconds(0), &ClusterServer::send, this, destination, packet);
}

void ClusterServer::scheduleSend(Time t, std::string destination, Ptr<Packet> packet) {
	Simulator::Schedule(t, &ClusterServer::send, this, destination, packet);
}

void ClusterServer::send(std::string destination, Ptr<Packet> packet) {
	Log::i(id, "Sending Packet to " + destination);
	traceHelper->addTag(packet, ipv4, destination);
    packetLogger->logPacketTx(packet);
	this->communicator->send(destination, packet, this->GetNode());
}

void ClusterServer::handleAccept(Ptr<Socket> socket, const Address& address) {
	socket->SetRecvCallback(MakeCallback(&ClusterServer::handleRead, this));
}

void ClusterServer::handleRead(Ptr<Socket> socket) {
	Ptr<Packet> packet;
	Address sender;
	while((packet = socket->RecvFrom(sender))) {
		if(packetLog->check(packet))
			handlePacket(packet);
		#ifndef TURBO
		else {
			Log::i(id, "received duplicate packet, discarding, uid: ", packet->GetUid());
			//throw("asd");
		}
		#endif
	}
}

void ClusterServer::handlePacket(Ptr<Packet> packet) {
	Log::i(id, "received packet, uid: ", packet->GetUid());
	packetLogger->logPacketRx(packet);

	Token* token = Token::extractToken(packet);
	std::vector<void*> params;
	params.push_back(this);
	params.push_back(token);
	algo->execute(moduleToken, &params);
}

void ClusterServer::tick() {
	Log::i(id, "tick");

	std::vector<void*> params;
	params.push_back(this);
	algo->execute(moduleTick, &params);

	if(t->getMinute() < simEnd)
		Simulator::Schedule(Seconds(interval * 60), &ClusterServer::tick, this);
}

}
