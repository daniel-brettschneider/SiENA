#ifndef TOKENTAG_H_
#define TOKENTAG_H_

#include "ns3/core-module.h"
#include "ns3/Token.h"

namespace ns3 {

class TokenTag : public Tag {

private:
	uint64_t token;

public:
	static TypeId GetTypeId() {
		static TypeId tid = TypeId("ns3::TokenTag")
			.SetParent<Tag>()
		    .AddConstructor<TokenTag> ()
		    .AddAttribute("Token", "token pointer", EmptyAttributeValue(), MakeUintegerAccessor(&TokenTag::token), MakeUintegerChecker<uint64_t>());
		return tid;
	}
	virtual TypeId GetInstanceTypeId() const { return GetTypeId(); }
	virtual uint32_t GetSerializedSize() const { return sizeof(token); }
	virtual void Serialize(TagBuffer buffer) const { buffer.WriteU64(token); }
	virtual void Deserialize(TagBuffer buffer) { token = buffer.ReadU64(); }
	virtual void Print(std::ostream &os) const { os << "token=" << token; }
	void setToken(Token* token) { this->token = reinterpret_cast<uint64_t>(token); }
	Token* getToken() { return reinterpret_cast<Token*>(token);	}

};

}

#endif
