#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_PRICEUPDATE_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_PRICEUPDATE_H_

#include "ns3/Token.h"
#include "ns3/MyStats.h"

namespace ns3 {

class PM_PriceUpdate : public Token {

protected:
	double price;
	int statId;
	int tick;

public:
	PM_PriceUpdate(double price, int statId) : Token(), price(price), statId(statId) { tick = Tick::Get()->getTick(); }
	PM_PriceUpdate(std::vector<std::string>* participants, double price, int statId) : Token(participants), price(price), statId(statId) { tick = Tick::Get()->getTick(); }
	virtual ~PM_PriceUpdate() {}
	double getPrice() { return price; }
	int getStatId() { return statId; }
	void saveStat() { MyStats::Get()->addStat(statId, Tick::Get()->getTimeInInterval()); } //Simulator::Now().GetMicroSeconds() % (15 * 60000000));}//
	int getTick() { return tick; }
	virtual int getSize() { return 4 + 4; }

};

}

#endif
