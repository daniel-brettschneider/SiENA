#ifndef NS_3_22_SRC_CITYGRID_MODULES_PRIVADE_PRIV_TOKENCLUSTER_H_
#define NS_3_22_SRC_CITYGRID_MODULES_PRIVADE_PRIV_TOKENCLUSTER_H_

#include "ns3/Token.h"

namespace ns3 {

class Priv_TokenCluster : public Token {

private:
	std::string ip;
	// flex
	int numberFlexUp, numberFlexDown;
	double amountFlexUp, amountFlexDown;
	double amountCurrent;
	double assigned;
	// TODO add switchable

public:
	Priv_TokenCluster(double assigned) : Token(), numberFlexUp(0), numberFlexDown(0), amountFlexUp(0), amountFlexDown(0), amountCurrent(0), assigned(assigned) {}
	Priv_TokenCluster(std::string sender, int numberFlexUp, int numberFlexDown, double amountFlexUp, double amountFlexDown, double amountCurrent) : Token(), ip(sender),
			numberFlexUp(numberFlexUp), numberFlexDown(numberFlexDown), amountFlexUp(amountFlexUp), amountFlexDown(amountFlexDown), amountCurrent(amountCurrent), assigned(0) {}
	virtual ~Priv_TokenCluster() {}

	int getNumberFlexUp() { return numberFlexUp; }
	int getNumberFlexDown() { return numberFlexDown; }
	double getAmountFlexUp() { return amountFlexUp; }
	double getAmountFlexDown() { return amountFlexDown; }
	double getAmountCurrent() { return amountCurrent; }
	double getAssigned() { return assigned; }
	std::string getIp() { return ip; }

protected:
	virtual int getSize() {
		return 2 * 512 / 8;  // 1 Block 512 Bit * 2 homomorphe Verschl. in Byte
	}
};

}

#endif
