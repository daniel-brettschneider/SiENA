#ifndef NS_3_22_SRC_SIENA_STATS_TRACEHELPER_H_
#define NS_3_22_SRC_SIENA_STATS_TRACEHELPER_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/Log.h"
#include "ns3/MySingleton.h"
#include "ns3/MyStats.h"
#include <map>
#include <string>
#include "ns3/TraceHelperTag.h"
#include "ns3/PacketLogger.h"

namespace ns3 {

class TraceHelper : public MySingleton<TraceHelper> {
	friend class MySingleton<TraceHelper>;

private:
	bool shouldTrace;
	MyStats* stats;
	int duration;
	double rx, tx;
	int statsIdRx, statsIdTx;
	std::map<Ipv4Address, std::pair<int,int> > statsIds;
	PacketLogger* packetLogger;

public:
	virtual ~TraceHelper() {}

	void addRx(int rx, Ipv4Address adr) {
		this->rx += rx;
		stats->increaseStat(statsIdRx, rx);
		int id = statsIds.at(adr).first;
		stats->increaseStat(id, rx);
		packetLogger->logMacRx(rx);
	}
	void addTx(int tx, Ipv4Address adr) {
		this->tx += tx;
		stats->increaseStat(statsIdTx, tx);
		int id = statsIds.at(adr).second;
		stats->increaseStat(id, tx);
		packetLogger->logMacTx(tx);
	}

	void registerTracesP2P(Ptr<Node> node) {
		if(shouldTrace) {
			for(uint i = 0; i < node->GetNDevices(); i++) {
				Ptr<NetDevice> dev = node->GetDevice(i);
				dev->TraceConnectWithoutContext("MacRx", MakeCallback(&TraceHelper::trace_MacRx));
				dev->TraceConnectWithoutContext("MacTx", MakeCallback(&TraceHelper::trace_MacTx));
			}
		}
	}

	void registerTracesStats(std::string id, Ipv4Address ip) {
		if(shouldTrace) {
//			std::cout << id << " " << Helper::toString(ip) << std::endl;
			int idRx = stats->registerStat("mac_rx", id, 0);
			int idTx = stats->registerStat("mac_tx", id, 0);
			statsIds.insert(std::pair<Ipv4Address, std::pair<int,int> >(ip, std::pair<int, int>(idRx, idTx)));
		}
	}

	void addTag(Ptr<Packet> packet, Ipv4Address source, std::string destination) {
		TraceHelperTag tag;
		tag.set(source, Ipv4Address(destination.c_str()));
		packet->AddPacketTag(tag);
	}

	void print() {
		Log::i("TraceHelper", "total mac bytes received: ", rx);
		double throughput = rx / 1024.0 / duration;
		std::stringstream s;
		s << "avg throughput: " << throughput << " kB/s";
		Log::i("TraceHelper", s.str());
	}

	static void trace_MacRx(Ptr<const Packet> packet) {;
		TraceHelperTag tag;
		bool found = packet->PeekPacketTag(tag);
		if(found)
			TraceHelper::Get()->addRx(packet->GetSize(), tag.getDestination());
//		Ipv4Address adr = tag.getDestination();
//		std::cout << "rx " << Helper::toString(adr) << " " << packet->GetSize() << std::endl;
	}

	static void trace_MacTx(Ptr<const Packet> packet) {
		TraceHelperTag tag;
		bool found = packet->PeekPacketTag(tag);
		if(found)
			TraceHelper::Get()->addTx(packet->GetSize(), tag.getSource());
//		Ipv4Address adr = tag.getSource();
//		std::cout << "tx " << Helper::toString(adr) << " " << packet->GetSize() << std::endl;
	}

private:
	TraceHelper() : rx(0), tx(0) {
		duration = Tick::Get()->getDuration() * 60;
		stats = MyStats::Get();
		MyConfig* config = MyConfig::Get();
		std::vector<std::string> saveIds = Helper::split(config->getString("keep_only_these_stats_in_ram"), ',');
		std::vector<std::string> saveIds2 = Helper::split(config->getString("save_only_these_stats"), ',');
		bool filter = config->getBool("filter_stats");
		bool com = config->getBool("simulate_communication");
		shouldTrace = false;
		if(com) {
			if(!filter || Helper::contains(&saveIds, "mac_rx") || Helper::contains(&saveIds, "mac_tx")) {
				shouldTrace = true;
				statsIdRx = stats->registerStat("mac_rx", "total", 0);
				statsIdTx = stats->registerStat("mac_tx", "total", 0);
			}
		}
		packetLogger = PacketLogger::Get();
	}
};

}

#endif
