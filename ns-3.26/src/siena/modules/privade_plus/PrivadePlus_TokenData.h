#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_TOKENDATA_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_TOKENDATA_H_

#include "ns3/Token.h"
#include "ns3/AdaptionCategories.h"
#include <vector>

#define PRIVADE_BES 0
#define PRIVADE_HES 1

namespace ns3 {

class PrivadePlus_TokenData : public Token {

private:
	bool shiftExpected;
	// flex
	int numberFlexUp, numberFlexDown;
	double amountFlexUp, amountFlexDown;
	double amountCurrent;
	// switch
	int numberSwitchable;		// not in token, just a helper
	double amountSwitchable;	// not in token, just a helper
	std::vector<std::vector<std::pair<int, int> > > switchableCategories;
	// shift
	std::vector<double>* expectedConsumption;

public:
	PrivadePlus_TokenData(bool shiftExpected, std::vector<std::string>* participants, std::vector<double>* expectedConsumption = NULL) :
			Token(participants), shiftExpected(shiftExpected), numberFlexUp(0), numberFlexDown(0), amountFlexUp(0),
			amountFlexDown(0), amountCurrent(0), numberSwitchable(0), amountSwitchable(0), expectedConsumption(expectedConsumption) {
		AdaptionCategories* a = AdaptionCategories::Get();
		std::vector<double>* catValues = a->getCategories();
		std::vector<std::pair<int, int> > tmp;
		tmp.reserve(a->getNumberWithoutPrios());
		for(int i = 0; i < a->getNumberWithoutPrios(); i++) {
			tmp.push_back(std::pair<int, int>(0, (*catValues)[i]));
		}
		for(int i = 0; i < a->getPrios(); i++) {
			switchableCategories.push_back(std::vector<std::pair<int, int> >(tmp));
		}
	}
	PrivadePlus_TokenData(const PrivadePlus_TokenData& t) : Token(t), shiftExpected(t.shiftExpected), numberFlexUp(t.numberFlexUp),
			numberFlexDown(t.numberFlexDown), amountFlexUp(t.amountFlexUp), amountFlexDown(t.amountFlexDown), amountCurrent(t.amountCurrent),
			numberSwitchable(t.numberSwitchable), amountSwitchable(t.amountSwitchable), switchableCategories(t.switchableCategories),
			expectedConsumption(t.expectedConsumption) {}
	virtual ~PrivadePlus_TokenData() {}

	int getNumberFlexUp() { return numberFlexUp; }
	int getNumberFlexDown() { return numberFlexDown; }
	double getAmountFlexUp() { return amountFlexUp; }
	double getAmountFlexDown() { return amountFlexDown; }
	double getAmountCurrent() { return amountCurrent; }
	int getNumberSwitchable() { return numberSwitchable; }
	double getAmountSwitchable() { return amountSwitchable; }
	std::vector<std::vector<std::pair<int, int> > >* getSwitchableCategories() { return &switchableCategories; }
	std::vector<double>* getExpectedConsumption() { return expectedConsumption; }

	void addFlexUp(double amount) { amountFlexUp += amount; numberFlexUp++; }
	void addFlexDown(double amount) { amountFlexDown += amount; numberFlexDown++; }
	void addCurrent(double amount) { amountCurrent += amount; }
	void addSwitchable(int category, int categoryValue, int priority) {
		switchableCategories[priority][category].first++;
		numberSwitchable++;
		amountSwitchable += categoryValue;
	}

	void add(PrivadePlus_TokenData* d) {
		std::vector<std::vector<std::pair<int, int> > >* otherCats = d->getSwitchableCategories();
		for(uint i = 0; i < switchableCategories.size(); i++) {
			for(uint j = 0; j < switchableCategories[0].size(); j++) {
				std::pair<int,int> add = (*otherCats)[i][j];
				switchableCategories[i][j].first += add.first;
				numberSwitchable += add.first;
				amountSwitchable += add.second;
			}
		}
		amountFlexUp += d->getAmountFlexUp();
		numberFlexUp += d->getNumberFlexUp();
		amountFlexDown += d->getAmountFlexDown();
		numberFlexDown += d->getNumberFlexDown();
		amountCurrent += d->getAmountCurrent();
	}


	virtual int getSize() {
        // 8 Bit per Categorie & 32 Bit per Counter
	    int numberOfCategories = switchableCategories.at(0).size() * switchableCategories.size();
		if(numberOfCategories <= 40) {
			return 2 * 512 / 8;  // 1 Block 512 Bit * 2 homomorphe Verschl. in Byte
		} else {
			int rest = numberOfCategories - 40;
			int blocks = rest / 64 + (rest % 64 != 0 ? 1 : 0);
			return (blocks + 1) * 512 * 2 / 8; // s.o. + restliche 512er Bloecke
		}
	}

};

}

#endif
