#include "../../siena/network/Token.h"

#include "ns3/TokenTag.h"

namespace ns3 {

Token::~Token() {
	delete participants;
}

std::string Token::next() {
	return (*participants)[counter++];
}

int Token::getSize() {
	return 100;
}

bool Token::hasNext() {
	return (uint) counter < participants->size();
}

Ptr<Packet> Token::getPacket() {
	Ptr<Packet> packet = Create<Packet>(this->getSize());
	TokenTag tag;
	tag.setToken(this);
	packet->AddPacketTag(tag);
	packet->AddByteTag(tag);
	return packet;
}

Token* Token::extractToken(Ptr<Packet> packet) {
	TokenTag tag;
	bool found = packet->PeekPacketTag(tag);
	if(found)
		return tag.getToken();
	else {
		ByteTagIterator it = packet->GetByteTagIterator();
		while(it.HasNext()) {
			ByteTagIterator::Item i = it.Next();
			if(i.GetTypeId() == TokenTag::GetTypeId()) {
				i.GetTag(tag);
				return tag.getToken();
			}
		}
	}
	throw("token not found in packet");
}

}
