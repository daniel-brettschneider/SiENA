#ifndef RECIPIENT_H_
#define RECIPIENT_H_

#include "ns3/packet.h"

namespace ns3 {

class Recipient {

public:
	virtual ~Recipient() {}
	virtual void handlePacket(Ptr<Packet> packet) = 0;

};

}

#endif
