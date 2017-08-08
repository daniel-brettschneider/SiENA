#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_TOKENADAPT_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_TOKENADAPT_H_

#include "ns3/Token.h"
#include "ns3/AdaptionCategories.h"
#include "ns3/Priv_TokenData.h"
#include <vector>
#include <set>
#include <cmath>

namespace ns3 {

class Priv_TokenAdapt : public Token {

private:
	int encryptionMode;
	// flex
	int numberFlex;
	double amountResidual;
	double shareFlex;
	// switch
	bool isSwitchable;
	std::vector<std::set<int> > switchableCategories;

public:
	Priv_TokenAdapt(int encryptionMode, std::vector<std::string>* participants) : Token(participants), encryptionMode(encryptionMode), numberFlex(0), amountResidual(0), shareFlex(0), isSwitchable(false) {
		for(int i = 0; i < SWITCHABLE_PRIORITY_STAGES; i++) {
			switchableCategories.push_back(std::set<int>());
		}
	}
	virtual ~Priv_TokenAdapt() {}

	int getNumberFlex() { return numberFlex; }
	double getAmountResidual() { return amountResidual; }
	double getShareFlex() { return shareFlex; }
	std::vector<std::set<int> >* getSwitchableCategories() { return &switchableCategories; }

	void setShareFlex(double shareFlex) { this->shareFlex = shareFlex; }
	void addResidual(double amount) { amountResidual += amount; }
	void addFlex() { numberFlex++; }
	void addSwitchable(int category, int priority) { isSwitchable = true; switchableCategories[priority].insert(category); }

	bool hasToSwitch(int category, int priority) { return switchableCategories[priority].find(category) != switchableCategories[priority].end(); }
	bool hasSwitchable() { return isSwitchable; }

protected:
	virtual int getSize() {
		if(encryptionMode == PRIVADE_HES) {
			// 10 Bit per Categorie & 32 Bit per Counter
			if(switchableCategories.size() <= 44) {
				return 2 * 512 / 8;  // 1 Block 512 Bit * 2 homomorphe Verschl. in Byte
			} else {
				int rest = switchableCategories.size() - 44;
				int blocks = rest / 51 + (rest % 51 != 0 ? 1 : 0);
				return (blocks + 1) * 512 * 2 / 8; // s.o. + restliche 512er Bloecke
			}
		} else {
			return (participants->size()-1) * 16
				   + switchableCategories.size() * 1	/* switch */
				   + 5 * 4;								/* flex */
		}
	}

};

}

#endif
