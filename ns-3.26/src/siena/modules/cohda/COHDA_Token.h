#ifndef COHDA_WORKINGMEMORY_H_
#define COHDA_WORKINGMEMORY_H_

#include <vector>
#include "ns3/Token.h"
#include "ns3/Adaption.h"
#include "ns3/AdaptionCategories.h"
#include "ns3/COHDA_Configuration.h"
#include "ns3/COHDA_SolutionCandidate.h"
#include "ns3/COHDA_WorkingMemory.h"

namespace ns3 {


class COHDA_Token : public Token {

private:
	std::map<std::string,std::vector<std::string>* >* overlayNet;
	std::string adrString;
	COHDA_WorkingMemory kappa;
	int msgStatsId;
	int timeStatsId;
	int dataStatsId;
	int tick;

public:
	COHDA_Token(int timeStatsId, int msgStatsId, int dataStatsId, std::map<std::string,std::vector<std::string>* >* overlayNet, std::string adrString, COHDA_WorkingMemory* kappa)
			: Token(), overlayNet(overlayNet), adrString(adrString), msgStatsId(msgStatsId), timeStatsId(timeStatsId), dataStatsId(dataStatsId) {
		this->kappa.setConfiguration(kappa->getConfigurationNice());
		this->kappa.setSolutionCandidate(kappa->getSolutionCandidateNice());
		this->kappa.setZeta(kappa->getZeta());
		this->tick = Tick::Get()->getTick();
	}
	virtual ~COHDA_Token() {}
	virtual std::string next() { throw("gibts nich"); return NULL; }
	std::vector<std::string>* getRecipients(std::string ip) { return (*overlayNet)[ip]; }
	std::map<std::string,std::vector<std::string>* >* getOverlayNet() { return overlayNet; }
	COHDA_WorkingMemory* getWorkingMemoryNice() { return &kappa; }
	int getMsgStatsId() { return msgStatsId; }
	int getTimeStatsId() { return timeStatsId; }
	int getDataStatsId() { return dataStatsId; }
	int getTick() { return tick; }

	void setWorkingMemory(COHDA_WorkingMemory k) { this->kappa = k; }

protected:
	virtual int getSize() {
		if(kappa.getConfigurationNice()->getSize() != kappa.getSolutionCandidateNice()->getConfigurationNice()->getSize())
			throw("kappa.size != solutionCandidate.size");
		//			home+ consumption + lambda
		int ssSize = 4 + 4 + 4;
		//			   zeta + Anzahl der ScheduleSelections * (size(theta)+size(lambda)) + adress
		float tokenSize;// = 4 + (kappa.getConfigurationNice()->getSize() + kappa.getSolutionCandidateNice()->getConfigurationNice()->getSize()) * ssSize + 4;
		if(kappa.getConfigurationNice()->getSize() == 0)
			tokenSize = 4 + 4;//time + goal
		else
			tokenSize = 4 + 4 +(2 * kappa.getConfigurationNice()->getSize()) * ssSize + 4;
		return tokenSize;
	}
};

}

#endif
