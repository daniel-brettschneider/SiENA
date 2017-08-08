#ifndef NS_3_22_SRC_SIENA_NETWORK_PACKETLOG_H_
#define NS_3_22_SRC_SIENA_NETWORK_PACKETLOG_H_

#include "ns3/MyConfig.h"
#include "ns3/MySingleton.h"
#include <set>
#include <fstream>

namespace ns3 {

class PacketLog : public MySingleton<PacketLog> {
	friend class MySingleton<PacketLog>;

private:
	int max;
	int threshold;
	std::set<uint64_t> uids;

public:

	/**
	 * true: ok, false: duplicate
	 */
	bool check(Ptr<Packet> packet) {
		uint64_t uid = packet->GetUid();
		// check
		std::set<uint64_t>::iterator it = uids.find(uid);
		if(it != uids.end())
			return false;
		else
			uids.insert(uids.end(), uid);
		// clean
		if(uids.size() >= (uint) max + threshold) {
			for(int i = 0; i < threshold; i++) {
				uids.erase(uids.begin());
			}
		}
		return true;
	}

private:
	PacketLog() {
		MyConfig* config = MyConfig::Get();
		max = config->getInt("check_duplicate_packets");
		threshold = max * 0.1;
		if(threshold == 0)
			threshold = 1;
	}

};

}

#endif
