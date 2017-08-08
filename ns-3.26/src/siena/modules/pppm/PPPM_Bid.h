#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PRIV_PPPM_Bid_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PRIV_PPPM_Bid_H_

#include "ns3/PM_Bid.h"
#include "ns3/MyConfig.h"
#include <vector>
#include <string>
#include <sstream>

namespace ns3 {

class PPPM_Bid : public PM_Bid {

protected:
	std::vector<double> steps;
	double resolution;
	double minPrice, maxPrice, priceSteps;

public:
	PPPM_Bid(std::vector<std::string>* participants) : PM_Bid(participants), resolution(-1), priceSteps(0) {
		MyConfig* config = MyConfig::Get();
		minPrice = config->getDouble("min_price");
		maxPrice = config->getDouble("max_price");
		init();
		steps.resize(priceSteps, 0);
	}
	PPPM_Bid(std::vector<std::string>* participants, double minPrice, double maxPrice) : PM_Bid(participants), resolution(-1), minPrice(minPrice), maxPrice(maxPrice), priceSteps(0) {
		init();
		steps.resize(priceSteps, 0);
	}
	virtual ~PPPM_Bid() {}

	std::vector<double>* getSteps() { return &steps; }
	double getResolution() { return resolution; }
	int getPriceSteps() { return priceSteps; }
	double getMinPrice() { return minPrice; }
	double getMaxPrice() { return maxPrice; }

	virtual int getSize() {
		MyConfig* config = MyConfig::Get();
		int blocks = (4+4*priceSteps)/64;	//Vollständige Blöcke
		int rest = (4+4*(int)priceSteps)%64;
		int additional = (rest != 0 ? 1 : 0);	//ggf. ein weiterer Block
		int size = (blocks + additional) * 512 * 2 / 8;
		if(minPrice != config->getDouble("min_price") || maxPrice != config->getDouble("max_price"))
			size += 16;
		return size;
	}

	double getConsumption(double price, bool second = false) {
		return steps[getIndex(price)];
	}

	void addCoordinate(double price, double consumption) {
		steps[getIndex(price)] += consumption;
	}

	void addCoordinate(int t, double consumption) {
		steps[t] += consumption;
	}

	void addSteps(std::vector<double>* other) {
		for(uint i = 0; i < priceSteps; i++) {
			steps[i] += other->at(i);
		}
	}

	void addBid(PPPM_Bid* bid) {
		std::vector<double>* other = bid->getSteps();
		for(uint i = 0; i < priceSteps; i++) {
			steps[i] += other->at(i);
		}
	}

	double findEquilibrium(double goal) {
		for(uint i = 0; i < priceSteps; i++) {
			if(i == 0 && steps[i] <= goal)
				return getPrice(i);
			else if(steps[i] < goal) {
				return getPrice(i);
			}
		}
		return getPrice(priceSteps - 1);
	}

	std::pair<double,double> findNewRange(double goal) {
		for(uint i = 0; i < priceSteps; i++) {
			if(i == 0 && steps[i] <= goal)
				return std::pair<double,double>(1, 0);
			else if(steps[i] < goal)
				return std::pair<double, double>(getPrice(i-1), getPrice(i));
		}
		return std::pair<double,double>(1, 0);
	}

	std::string toString() {
		std::stringstream s;
		for(uint i = 0; i < priceSteps; i++) {
			s << steps[i] << " ";
		}
		return s.str();
	}

private:
	int getIndex(double price) {
		return (price - minPrice) / resolution;
	}

	double getPrice(int index) {
		return minPrice + index * resolution;
	}

	void init() {
		MyConfig* config = MyConfig::Get();
		priceSteps = config->getDouble("price_steps");
		if(priceSteps == 1)
			resolution = 1; // doesnt matter
		else
			resolution = (double) (maxPrice - minPrice) / (double) (priceSteps - 1);
	}
};

}

#endif
