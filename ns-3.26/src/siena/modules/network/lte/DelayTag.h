#ifndef NS_3_22_SRC_SIENA_MODULES_NETWORK_LTE_DELAYTAG_H_
#define NS_3_22_SRC_SIENA_MODULES_NETWORK_LTE_DELAYTAG_H_

#include "ns3/core-module.h"

namespace ns3 {

class DelayTag : public Tag {

private:
	uint16_t delay;

public:
	static TypeId GetTypeId() {
		static TypeId tid = TypeId("ns3::DelayTag")
			.SetParent<Tag>()
			.AddConstructor<DelayTag> ()
			.AddAttribute("Delay Tag", "asd", EmptyAttributeValue(), MakeIntegerAccessor(&DelayTag::delay), MakeUintegerChecker<uint16_t>());
		return tid;
	}
	virtual TypeId GetInstanceTypeId() const { return GetTypeId(); }
	virtual uint32_t GetSerializedSize() const { return sizeof(delay); }
	virtual void Serialize(TagBuffer buffer) const { buffer.WriteU16(delay); }
	virtual void Deserialize(TagBuffer buffer) { delay = buffer.ReadU16(); }
	virtual void Print(std::ostream &os) const { os << "delay=" << delay; }
	void setDelay() { this->delay = 1; }
	uint16_t getDelay() { return delay; }
};

}

#endif
