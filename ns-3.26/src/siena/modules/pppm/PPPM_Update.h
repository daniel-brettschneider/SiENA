#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PRIV_PPPM_Update_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PRIV_PPPM_Update_H_

#include "ns3/PM_PriceUpdate.h"

namespace ns3 {

#define PPPM_MODE_PRICE 0
#define PPPM_MODE_STAGE 1

class PPPM_Update : public PM_PriceUpdate {

protected:
	double minPrice;
	double maxPrice;
	int mode;

public:
	PPPM_Update(double price, int statId) : PM_PriceUpdate(price, statId), minPrice(-1), maxPrice(-1), mode(PPPM_MODE_PRICE) {}
	PPPM_Update(std::vector<std::string>* participants, double price, int statId) : PM_PriceUpdate(participants, price, statId), minPrice(-1), maxPrice(-1), mode(PPPM_MODE_PRICE) {}
	PPPM_Update(double minPrice, double maxPrice, int statId) : PM_PriceUpdate(-1, statId), minPrice(minPrice), maxPrice(maxPrice), mode(PPPM_MODE_STAGE) {}
	virtual ~PPPM_Update() {}

	double getMinPrice() { return minPrice; }
	double getMaxPrice() { return maxPrice; }
	int getMode() { return mode; }

};

}

#endif
