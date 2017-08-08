#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_TOKENSHARE_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_TOKENSHARE_H_

#include "ns3/Token.h"
#include <cmath>
#include "ns3/AdaptionCategories.h"

namespace ns3 {

class PrivadePlus_TokenShare : public Token {

private:
	std::vector<std::set<int> > switchableCategories;
	bool isSwitchable;
	double shareFlex;
	int timeStatsId;

public:
	PrivadePlus_TokenShare(int timeStatsId) : Token(NULL), isSwitchable(false), shareFlex(0), timeStatsId(timeStatsId) {
		for(int i = 0; i < SWITCHABLE_PRIORITY_STAGES; i++) {
			switchableCategories.push_back(std::set<int>());
		}
	}
	PrivadePlus_TokenShare(const PrivadePlus_TokenShare& t) : Token(t), switchableCategories(t.switchableCategories), isSwitchable(t.isSwitchable),
			shareFlex(t.shareFlex), timeStatsId(t.timeStatsId) {
		for(uint i = 0; i < t.switchableCategories.size(); i++) {
			switchableCategories.push_back(t.switchableCategories[i]);
		}
	}
	virtual ~PrivadePlus_TokenShare() {}

	double getShareFlex() { return shareFlex; }
	std::vector<std::set<int> >* getSwitchableCategories() { return &switchableCategories; }
	int getTimeStatsId() { return timeStatsId; }

	void setShareFlex(double shareFlex) { this->shareFlex = shareFlex; }
	void addSwitchable(int category, int priority) { isSwitchable = true; switchableCategories[priority].insert(category); }

	bool hasToSwitch(int category, int priority) { return switchableCategories[priority].find(category) != switchableCategories[priority].end(); }
	bool hasSwitchable() { return isSwitchable; }

	virtual int getSize() {
		return ceil((switchableCategories.size() * 10 + 32) / 8);
	}
};

}

#endif
