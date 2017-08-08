#ifndef NS_3_22_SRC_SIENA_STATS_PACKETLOGGER_H_
#define NS_3_22_SRC_SIENA_STATS_PACKETLOGGER_H_

#include "ns3/MySingleton.h"
#include <fstream>

namespace ns3 {

class PacketLogger : public MySingleton<PacketLogger> {
	friend class MySingleton<PacketLogger>;

private:
	bool shouldTrace;
	std::ofstream* macTx, *macRx;
	std::ofstream* packetTx, *packetRx;

public:
	virtual ~PacketLogger() {
		if(shouldTrace) {
			macTx->close();
			delete macTx;
			macRx->close();
			delete macRx;
			packetTx->close();
			delete packetTx;
			packetRx->close();
			delete packetRx;
		}
	}

	void logMacRx(int rx) {
		if(shouldTrace) {
			(*macRx) << Simulator::Now().GetNanoSeconds() << "," << rx  << "\n";
		}
	}

	void logMacTx(int tx) {
		if(shouldTrace) {
			(*macTx) << Simulator::Now().GetNanoSeconds() << "," << tx  << "\n";
		}
	}

	void logPacketRx(Ptr<Packet> p) {
		if(shouldTrace) {
			(*packetRx) << Simulator::Now().GetNanoSeconds() << "," << p->GetSize()  << "\n";
		}
	}

	void logPacketTx(Ptr<Packet> p) {
		if(shouldTrace) {
			(*packetTx) << Simulator::Now().GetNanoSeconds() << "," << p->GetSize()  << "\n";
		}
	}

private:
	PacketLogger() {
		MyConfig* config = MyConfig::Get();
		std::vector<std::string> saveIds = Helper::split(config->getString("keep_only_these_stats_in_ram"), ',');
		std::vector<std::string> saveIds2 = Helper::split(config->getString("save_only_these_stats"), ',');
		bool filter = config->getBool("filter_stats");
		bool com = config->getBool("simulate_communication");
		shouldTrace = false;
		macTx = NULL;
		macRx = NULL;
		packetTx = NULL;
		packetRx = NULL;
		if(com) {
			if(!filter || (Helper::contains(&saveIds, "mac_tx_time") && Helper::contains(&saveIds2, "mac_tx_time"))) {
				shouldTrace = true;
				std::string filename_tx = config->getString("identifier");
				macTx = new std::ofstream(("results/" + filename_tx + "_mac_tx_time.csv").c_str());
				(*macTx) << "time,data\n";
				packetTx = new std::ofstream(("results/" + filename_tx + "_packet_tx_time.csv").c_str());
				(*packetTx) << "time,data\n";
			}
			if(!filter || (Helper::contains(&saveIds, "mac_rx_time") && Helper::contains(&saveIds2, "mac_rx_time"))) {
				shouldTrace = true;
				std::string filename_rx = config->getString("identifier");
				macRx = new std::ofstream(("results/" + filename_rx + "_mac_rx_time.csv").c_str());
				(*macRx) << "time,data\n";
				packetRx = new std::ofstream(("results/" + filename_rx + "_packet_rx_time.csv").c_str());
				(*packetRx) << "time,data\n";
			}
		}
	}


};

}

#endif
