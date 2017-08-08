#include "../../siena/home/GridHome.h"

namespace ns3 {

TypeId GridHome::GetTypeId() {
	static TypeId id = TypeId("ns3::GridHome")
		.SetParent<Application>()
		.AddConstructor<GridHome>()
		.AddAttribute("id", "id", StringValue("gridhome"), MakeStringAccessor(&GridHome::id), MakeStringChecker());
	return id;
}

GridHome::GridHome() : ConventionalHome("error"), token(NULL), lastAdaption(-1) {
	communicator = Communicator::Get();
	algo = Modules::Get();
	moduleShift = c->getString("gridhome_shift_device");
	moduleToken = c->getString("gridhome_handle_token");
	moduleTick = c->getString("gridhome_tick");
	suppressTick = c->getBool("suppress_auto_home_tick");
	traceHelper = TraceHelper::Get();
	packetLog = PacketLog::Get();
	packetLogger = PacketLogger::Get();
}

void GridHome::StartApplication() {
	#ifndef TURBO
	Log::i(id, "starting application...");
	#endif
	TypeId id = TypeId::LookupByName(communicator->getSocketFactory());
	listenerSocket = Socket::CreateSocket(GetNode(), id);
	listenerSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 80));
	listenerSocket->Listen();
	listenerSocket->ShutdownSend();
	listenerSocket->SetRecvCallback(MakeCallback(&GridHome::handleRead, this));
	listenerSocket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address &> (), MakeCallback (&GridHome::handleAccept, this));

	ip = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	communicator->registerRecipient(this, Helper::toString(ip));
}

void GridHome::StopApplication() {
	listenerSocket->Close();
}

void GridHome::tick() {
	if(t->getTick() > simEnd) // catch late packets after sim end
		return;
	ConventionalHome::tick();

	std::vector<void*> params;
	params.push_back(this);
	algo->execute(moduleTick, &params);
}

void GridHome::schedule(DeviceEvent* event) {
	std::vector<void*> params;
	params.push_back(this);
	params.push_back(event);
	params.push_back(token);
	algo->execute(moduleShift, &params);
}

void GridHome::scheduleSend(std::string destination, Ptr<Packet> packet) {
	Simulator::Schedule(Seconds(0), &GridHome::send, this, destination, packet);
}

void GridHome::scheduleSend(Time t, std::string destination, Ptr<Packet> packet) {
	Simulator::Schedule(t, &GridHome::send, this, destination, packet);
}

void GridHome::send(std::string destination, Ptr<Packet> packet) {
	#ifndef	TURBO
		std::stringstream s;
		s << "Sending Packet to " << destination << " (Uid: " << packet->GetUid() << ")";
		Log::i(id, s.str());
	#endif
    traceHelper->addTag(packet, ip, destination);
    packetLogger->logPacketTx(packet);
	this->communicator->send(destination, packet, this->GetNode());
}

void GridHome::handleAccept(Ptr<Socket> socket, const Address& address) {
	socket->SetRecvCallback(MakeCallback(&GridHome::handleRead, this));
}

void GridHome::handleRead(Ptr<Socket> socket) {
	Ptr<Packet> packet;
	Address sender;
	while((packet = socket->RecvFrom(sender))) {
		if(packetLog->check(packet))
			handlePacket(packet);
		#ifndef TURBO
		else
			Log::i(id, "received duplicate packet, discarding, uid: ", packet->GetUid());
		#endif
	}
}

void GridHome::handlePacket(Ptr<Packet> packet) {
	Log::i(id, "received packet, uid: ", packet->GetUid());
	if(t->getTick() > simEnd) // catch late packets after sim end
		return;

	// handle packet
	packetLogger->logPacketRx(packet);
	token = Token::extractToken(packet);

	// tick
	this->tick();

	// adaption
	setAdaption();

	// handle token
	std::vector<void*> params;
	params.push_back(this);
	params.push_back(token);
	algo->execute(moduleToken, &params);
}

Adaption* GridHome::getAdaption() {
	setAdaption();
	return &adaption;
}

void GridHome::setAdaption() {
	if(t->getTick() > lastAdaption) {
		Log::i(id, "\tadaption:");
		adaption = Adaption();
		double minimum = 0, maximum = 0, desired = 0, base = 0;
		std::map<std::string, Device*>::iterator it;
		for(it = devices.begin(); it != devices.end(); ++it) {
			if(it->second->isAdaptable()) {
				std::pair<AdaptionFlex*, AdaptionOnOff*> a = it->second->getAdaption();
				if(a.first->isAdaptable()) {
					minimum += a.first->getMinimum();
					maximum += a.first->getMaximum();
					desired += a.first->getDesired();
				} else if(a.second != NULL) {
					adaption.addOnOff(a.second);
					base += a.second->getCurrent();
				}
				#ifndef TURBO
					if(a.second != NULL)
						Log::i(id, "\t\t" + it->first + " on/off: " + a.second->toString());
					if(a.first->isAdaptable())
						Log::i(id, "\t\t" + it->first + " flex: " + a.first->toString());
				#else
					Log::i(id, "\toptimized out");
				#endif
			} else
				base += it->second->getConsumption();
		}
		adaption.setFlex(AdaptionFlex(minimum, maximum, desired, INT_MIN, desired));
		adaption.setBase(base);
		#ifndef TURBO
			if(adaption.getFlex()->isAdaptable())
				Log::i(id, "\t\t-> total flex: " + adaption.getFlex()->toString());
			Log::i(id, "\t\t-> base: ", base);
		#endif
		lastAdaption = t->getTick();
	}
}

}
