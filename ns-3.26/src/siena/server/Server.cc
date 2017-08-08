#include "../../siena/server/Server.h"

namespace ns3 {

TypeId Server::GetTypeId() {
	static TypeId id = TypeId("ns3::Server")
		.SetParent<Application>()
		.AddConstructor<Server>()
		.AddAttribute("id", "id", StringValue("server"), MakeStringAccessor(&Server::id), MakeStringChecker())
		.AddAttribute("homeAddresses", "homeAddresses", PointerValue(NULL), MakePointerAccessor(&Server::homeAddresses), MakePointerChecker<AttributeContainer<Ipv4Address> >())
		.AddAttribute("server", "server", AddressValue(), MakeAddressAccessor(&Server::serverAddress), MakeAddressChecker())
		.AddAttribute("cluster", "cluster", IntegerValue(-1), MakeIntegerAccessor(&Server::cluster), MakeIntegerChecker<uint16_t>());
	return id;
}

Server::Server() : id("error"), cluster(-1), ip(""), adaption(0), stage(0), token(NULL) {
	communicator = Communicator::Get();
	t = Tick::Get();
	duration = MyConfig::Get()->getInt("duration");
	algo = Modules::Get();
	config = MyConfig::Get();
	stats = MyStats::Get();
	moduleToken = config->getString("server_handle_token");
	moduleTick = config->getString("server_tick");
	interval = config->getInt("interval");
	simEnd = config->getInt("start") + config->getInt("duration") - interval;
	traceHelper = TraceHelper::Get();
	packetLog = PacketLog::Get();
	packetLogger = PacketLogger::Get();
}

void Server::StartApplication() {
	Log::f(id, "initializing...");
	TypeId id = TypeId::LookupByName(communicator->getSocketFactory());
	listenerSocket = Socket::CreateSocket(GetNode(), id);
	listenerSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 80));
	listenerSocket->Listen();
	listenerSocket->ShutdownSend();
	listenerSocket->SetRecvCallback(MakeCallback(&Server::handleRead, this));
	listenerSocket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address &> (), MakeCallback (&Server::handleAccept, this));

	ipv4 = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	ip = Helper::toString(ipv4);
	communicator->registerRecipient(this, ip);

	Simulator::Schedule(Seconds(0), &Server::tick, this);
}

void Server::StopApplication() {
	listenerSocket->Close();
}

void Server::scheduleSend(std::string destination, Ptr<Packet> packet) {
	Simulator::Schedule(Seconds(0), &Server::send, this, destination, packet);
}

void Server::scheduleSend(Time t, std::string destination, Ptr<Packet> packet) {
	Simulator::Schedule(t, &Server::send, this, destination, packet);
}

void Server::send(std::string destination, Ptr<Packet> packet) {
    //Log::i(id, "Sending Packet to " + destination);
	#ifndef	TURBO
		std::stringstream s;
		s << "Sending Packet to " << destination << " (Uid: " << packet->GetUid() << ")";
		Log::i(id, s.str());
	#endif
    traceHelper->addTag(packet, ipv4, destination);
    packetLogger->logPacketTx(packet);
	this->communicator->send(destination, packet, this->GetNode());
}

void Server::handleAccept(Ptr<Socket> socket, const Address& address) {
	socket->SetRecvCallback(MakeCallback(&Server::handleRead, this));
}

void Server::handleRead(Ptr<Socket> socket) {
	Ptr<Packet> packet;
	Address sender;
	while((packet = socket->RecvFrom(sender))) {
		if(packetLog->check(packet))
			handlePacket(packet);
        #ifndef TURBO
        else {
        	Log::i(id, "received duplicate packet, discarding, uid: ", packet->GetUid());
		}
        #endif
	}
}

void Server::handlePacket(Ptr<Packet> packet) {
	Log::i(id, "received packet, uid: ", packet->GetUid());
	packetLogger->logPacketRx(packet);

	Token* token = Token::extractToken(packet);
	std::vector<void*> params;
	params.push_back(this);
	params.push_back(token);
	algo->execute(moduleToken, &params);
}

void Server::tick() {
	Log::i(id, "tick");

	std::vector<void*> params;
	params.push_back(this);
	algo->execute(moduleTick, &params);

	if(t->getMinute() < simEnd)
		Simulator::Schedule(Seconds(interval * 60), &Server::tick, this);
}

}
