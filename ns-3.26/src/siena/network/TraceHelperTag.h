#ifndef NS_3_22_SRC_SIENA_NETWORK_TRACEHELPERTAG_H_
#define NS_3_22_SRC_SIENA_NETWORK_TRACEHELPERTAG_H_

#include "ns3/tag.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

class TraceHelperTag : public Tag {

private:
	Ipv4Address source, destination;

public:
	static TypeId GetTypeId() {
		static TypeId tid = TypeId("ns3::TraceHelperTag")
			.SetParent<Tag>()
			.AddConstructor<TraceHelperTag>();
		return tid;
	}
	TraceHelperTag() {}
	virtual TypeId GetInstanceTypeId() const { return GetTypeId(); }
	virtual uint32_t GetSerializedSize() const { return 8; }
	virtual void Serialize(TagBuffer i) const {
		uint8_t buf[4];
		source.Serialize(buf);
		i.Write(buf, 4);
		destination.Serialize(buf);
		i.Write(buf, 4);
	}
	virtual void Deserialize(TagBuffer i) {
		uint8_t buf[4];
		i.Read(buf, 4);
		source = Ipv4Address::Deserialize(buf);
		i.Read(buf, 4);
		destination = Ipv4Address::Deserialize(buf);
	}
	virtual void Print(std::ostream &os) const { os << "kein bock was sinnvolles zu printen"; }

	void set(Ipv4Address source, Ipv4Address destination) {
		this->source = source;
		this->destination = destination;
	}

	Ipv4Address getSource() { return source; }
	Ipv4Address getDestination() { return destination; }

};

}

#endif
