#ifndef COHDA_SOLUTIONCANDIDATE_H_
#define COHDA_SOLUTIONCANDIDATE_H_

#include <vector>
#include "ns3/Token.h"
#include "ns3/Adaption.h"
#include "ns3/AdaptionCategories.h"
#include "ns3/COHDA_Configuration.h"

namespace ns3 {

class COHDA_SolutionCandidate {		//gamma

private:
	std::string adrString;
	COHDA_Configuration gOmega;

public:
	COHDA_SolutionCandidate() : adrString("") {}
	COHDA_SolutionCandidate(std::string adr, COHDA_Configuration gOmega) : adrString(adr), gOmega(gOmega) {}
	virtual ~COHDA_SolutionCandidate() {}

	std::string getAdrString() { return adrString; }
	COHDA_Configuration* getConfigurationNice() { return &gOmega; }

	void setAdrString(std::string adr) { this->adrString = adr; }
	void setConfiguration(COHDA_Configuration* c) { gOmega.setConfiguration(c->getGOmegaNice()); }

};

}

#endif
