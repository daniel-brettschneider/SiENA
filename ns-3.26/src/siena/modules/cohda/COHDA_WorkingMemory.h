#ifndef COHDA_BASETOKEN_H_
#define COHDA_BASETOKEN_H_

#include <vector>
#include "ns3/Token.h"
#include "ns3/Adaption.h"
#include "ns3/AdaptionCategories.h"
#include "ns3/COHDA_Configuration.h"
#include "ns3/COHDA_SolutionCandidate.h"

namespace ns3 {

class COHDA_WorkingMemory {	//kappa

private:
	double zeta;		//Wirkleistungsprodukt (Sollkurve)
	COHDA_Configuration gOmega;
	COHDA_SolutionCandidate gamma;

public:
	COHDA_WorkingMemory() : zeta(0) {}
	COHDA_WorkingMemory (double zeta, COHDA_Configuration gOmega, COHDA_SolutionCandidate gamma) : zeta(zeta), gOmega(gOmega) {
		this->gamma.setAdrString(gamma.getAdrString());
		this->gamma.setConfiguration(gamma.getConfigurationNice());
	}

	double getZeta() { return zeta; }
	COHDA_Configuration* getConfigurationNice () { return &gOmega; }
	COHDA_SolutionCandidate* getSolutionCandidateNice () { return &gamma; }

	void setZeta(double z) { this->zeta = z; }
	void setConfiguration(COHDA_Configuration* gOmega) { this->gOmega.setConfiguration(gOmega->getGOmegaNice()); }
	void setSolutionCandidate(COHDA_SolutionCandidate* gamma) {
		this->gamma.setAdrString(gamma->getAdrString());
		this->gamma.setConfiguration(gamma->getConfigurationNice());
	}

};



}

#endif
