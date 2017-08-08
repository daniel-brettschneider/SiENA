#ifndef NS_3_22_SRC_SIENA_MODULES_NETWORK_LTE_PACKETDELAY_H_
#define NS_3_22_SRC_SIENA_MODULES_NETWORK_LTE_PACKETDELAY_H_

#include <map>
#include <string>
#include "ns3/MySingleton.h"
#include "ns3/MyConfig.h"
#include "ns3/Random.h"
#include "ns3/DelayTag.h"

//#define PACKETDELAY

namespace ns3 {

class PacketDelay : public MySingleton<PacketDelay> {
	friend class MySingleton<PacketDelay>;

private:
	std::map<uint32_t,bool> nodes;
	int min, max;
	Random* random;

public:
	virtual ~PacketDelay() {}
	void addNode(uint32_t node) {
		nodes.insert(std::pair<uint32_t, bool>(node, true));
	}
	int getDelay(uint32_t node, Ptr<const Packet> packet) {
		if(nodes.find(node) != nodes.end()) {
			DelayTag tag;
			bool found = packet->PeekPacketTag(tag);
			if(found)
				return 0;
			else {
				ByteTagIterator it = packet->GetByteTagIterator();
				while(it.HasNext()) {
					ByteTagIterator::Item i = it.Next();
					if(i.GetTypeId() == DelayTag::GetTypeId()) {
						return 0;
					}
				}
			}
			DelayTag t;
			packet->AddPacketTag(tag);
			packet->AddByteTag(tag);
			return random->get(min, max);
		}
		return 0;
	}

private:
	PacketDelay() {
		MyConfig* config = MyConfig::Get();
		random = Random::Get();
		min = config->getInt("delay_min");
		max = config->getInt("delay_max");
	}

};

}

#endif
