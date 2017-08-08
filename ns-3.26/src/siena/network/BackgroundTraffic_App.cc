#include "../../siena/network/BackgroundTraffic_App.h"

namespace ns3 {

std::vector<std::vector<double>*>* BackgroundTraffic_App::frameSizeDistributionHelper = NULL;

TypeId BackgroundTraffic_App::GetTypeId() {
	static TypeId id = TypeId("ns3::BackgroundTraffic_App")
		.SetParent<Application>()
		.AddConstructor<BackgroundTraffic_App>()
		.AddAttribute("id", "id", StringValue("server"), MakeStringAccessor(&BackgroundTraffic_App::id), MakeStringChecker())
		.AddAttribute("homeAddresses", "homeAddresses", PointerValue(NULL), MakePointerAccessor(&BackgroundTraffic_App::homeAddresses), MakePointerChecker<AttributeContainer<std::string> >())
		.AddAttribute("homeDataRates", "homeDataRates", PointerValue(NULL), MakePointerAccessor(&BackgroundTraffic_App::homeDataRates), MakePointerChecker<AttributeContainer<int> >());
	return id;
}

BackgroundTraffic_App::BackgroundTraffic_App() : id("error") {
	communicator = Communicator::Get();
	t = Tick::Get();
	duration = MyConfig::Get()->getInt("duration");
	config = MyConfig::Get();
	interval = config->getInt("interval");
	simEnd = config->getInt("start") + config->getInt("duration") - interval;
	load = config->getDouble("background_traffic");
	random = Random::Get();
	std::vector<std::vector<double>*>* frameData = getFrameSizeDistribution();
	frameSize = frameData->at(0);
	frameDistribution = frameData->at(1);
	maxMs = interval * 60 * 1000;
}

void BackgroundTraffic_App::StartApplication() {
	Log::f(id, "initializing...");
	TypeId id = TypeId::LookupByName("ns3::UdpSocketFactory");
	listenerSocket = Socket::CreateSocket(GetNode(), id);
	listenerSocket->Bind(InetSocketAddress(Ipv4Address::GetAny(), 80));
	listenerSocket->Listen();
	listenerSocket->ShutdownSend();
	listenerSocket->SetRecvCallback(MakeCallback(&BackgroundTraffic_App::handleRead, this));

	Ipv4Address ipv4 = GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	std::string ip = Helper::toString(ipv4);
	communicator->registerRecipient(this, ip);

	Simulator::Schedule(Seconds(0), &BackgroundTraffic_App::tick, this);
}

void BackgroundTraffic_App::StopApplication() {
	listenerSocket->Close();
}

void BackgroundTraffic_App::scheduleSend(Time t, std::string destination, Ptr<Packet> packet) {
	Simulator::Schedule(t, &BackgroundTraffic_App::send, this, destination, packet);
}

void BackgroundTraffic_App::send(std::string destination, Ptr<Packet> packet) {
    Log::i(id, "Sending Packet to " + destination + ", size: ", packet->GetSize());
	this->communicator->send(destination, packet, this->GetNode());
}

void BackgroundTraffic_App::handleRead(Ptr<Socket> socket) {
	Ptr<Packet> packet;
	Address sender;
	while((packet = socket->RecvFrom(sender))) {
		Log::i(id, "received packet");
	}
}

void BackgroundTraffic_App::handlePacket(Ptr<Packet> packet) {
	Log::i(id, "received packet");
}

void BackgroundTraffic_App::tick() {
	Log::i(id, "tick");

	Log::i(id, "creating background traffic...");
	// create packets
	double totalData = 0;
	int totalPackets = 0;
	for(int i = 0; i < homeAddresses->size(); i++) {
		double maxData = (homeDataRates->at(i) / 8) * 60 * interval;
		double dataCounter = 0;
		int packetCounter = 0;
		while(dataCounter < maxData * load - (*frameSize)[0]) {
			double r = random->getD();
			uint size;
			for(size = 0; size < frameDistribution->size(); size++) {
				if((*frameDistribution)[size] >= r)
					break;
			}
			int s = (*frameSize)[size];
			Ptr<Packet> packet = Create<Packet>(s);
			scheduleSend(MilliSeconds(random->get(0, maxMs)), homeAddresses->at(i), packet);
			dataCounter += s;
			packetCounter++;
		}
		std::stringstream s;
		s << "\t" << homeAddresses->at(i) << " => " << "packets: " << packetCounter << ", data: " << dataCounter / 1000000 << " MB";
		Log::i(id, s.str());
		totalData += dataCounter / 1000000;
		totalPackets += packetCounter;
	}
	std::stringstream s;
	s << "\t=> packets: " << totalPackets << ", data: " << totalData << " MB";
	Log::i(id, s.str());

	if(t->getMinute() < simEnd)
		Simulator::Schedule(Seconds(interval * 60), &BackgroundTraffic_App::tick, this);
}

std::vector<std::vector<double>*>* BackgroundTraffic_App::getFrameSizeDistribution() {
	if(frameSizeDistributionHelper == NULL)
		frameSizeDistributionHelper = DataBasis::Get()->get("scenario/frame_size_distribution", "csv", CSV_PER_COLUMN, false, 1, 1455);
	return frameSizeDistributionHelper;
}

}
