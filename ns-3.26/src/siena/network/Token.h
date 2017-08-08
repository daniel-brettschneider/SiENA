#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>
#include <vector>
#include "ns3/packet.h"
#include "ns3/Tick.h"
#include "ns3/MyStats.h"

namespace ns3 {

class TokenTag;

class Token {

protected:
	std::vector<std::string>* participants;
	int counter;

public:
	Token() : participants(NULL), counter(0) {}
	Token(std::vector<std::string>* participants) : participants(participants), counter(0) {}
	Token(const Token& t) : participants(t.participants), counter(t.counter) {}
	virtual ~Token();
	virtual std::string next();
	virtual Ptr<Packet> getPacket();
	virtual bool hasNext();
	static Token* extractToken(Ptr<Packet> packet);

protected:
	virtual int getSize();

};

}

#endif
