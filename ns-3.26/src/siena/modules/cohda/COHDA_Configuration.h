#ifndef COHDA_CONFIGURATION_H_
#define COHDA_CONFIGURATION_H_

#include <vector>
#include "ns3/Token.h"
#include "ns3/Adaption.h"
#include "ns3/AdaptionCategories.h"
#include "ns3/COHDA_ScheduleSelection.h"

namespace ns3 {

class COHDA_Configuration {		//gOmega

private:
	std::map<std::string, COHDA_ScheduleSelection> gOmega;

public:
	COHDA_Configuration () {gOmega = std::map<std::string, COHDA_ScheduleSelection>(); }
	COHDA_Configuration (std::map<std::string, COHDA_ScheduleSelection> gOmega) : gOmega(gOmega) {}
	COHDA_Configuration (const COHDA_Configuration &c) {
		// gOmega.clear();
		gOmega = c.gOmega;
	}
	virtual ~COHDA_Configuration() {}

	uint getSize() { return gOmega.size(); }
	std::map<std::string, COHDA_ScheduleSelection>* getGOmegaNice() { return &gOmega; }
	COHDA_ScheduleSelection* getScheduleSelectionNice(std::string s) { return &this->gOmega[s]; }

	void setConfiguration(std::map<std::string, COHDA_ScheduleSelection>* gOmega) { this->gOmega = *gOmega; }
	void setScheduleSelection(std::string str, COHDA_ScheduleSelection* scheduleselection) { gOmega[str] = *scheduleselection; }

	bool contains(std::string adr) { return gOmega.find(adr) != gOmega.end(); }

	bool isProperSubsetOf(COHDA_Configuration* c){	// returns true, if this is a subset of surrending Configuration
		for(std::map<std::string, COHDA_ScheduleSelection>::iterator it = this->gOmega.begin(); it != this->gOmega.end(); ++it) {
			if(c->getGOmegaNice()->find(it->first) == c->getGOmegaNice()->end()) // this nicht in c gefunen
				return false;
		}
		if(c->getGOmegaNice()->size() == this->gOmega.size())
			return false;
		return true;
	}

	double getTotalConsumption() {
		if(gOmega.size() > 0) {
			double gTheta = 0;
			for(std::map<std::string,COHDA_ScheduleSelection>::iterator it = gOmega.begin(); it != gOmega.end(); ++it) {
				gTheta += it->second.getTheta();
			}
			return gTheta;
		}
		return 0;
	}

	std::string getAddresses() {
		std::stringstream ss;
		for(std::map<std::string,COHDA_ScheduleSelection>::iterator it = gOmega.begin(); it != gOmega.end(); ++it) {
			ss << it->first << ", ";
		}
		return ss.str();
	}

};

}

#endif
