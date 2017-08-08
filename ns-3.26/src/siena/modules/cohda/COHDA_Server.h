#ifndef COHDA_BASESERVER_H_
#define COHDA_BASESERVER_H_

#include "ns3/Server.h"
#include "ns3/COHDA_Token.h"
#include "ns3/AttributeContainer.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/TokenTag.h"
#include "ns3/MyConfig.h"
#include <sstream>
#include "ns3/Log.h"
#include "ns3/HomeContainer.h"
#include "ns3/Adaption.h"
#include <algorithm>
#include "ns3/MyStats.h"
#include "ns3/Random.h"
#include "ns3/COHDA_Configuration.h"
#include "ns3/Modules.h"
#include "ns3/DsmMode.h"

namespace ns3 {


void COHDA_CheckConvergence() {
	HomeContainer* container = HomeContainer::Get();
	std::vector<ConventionalHome*>* homes = container->getHomes();
	double err = -1;
	std::string solutionCreator;

	for(uint i = 0; i < homes->size(); i++) {
		GridHome* home = dynamic_cast<GridHome*>((*homes)[i]);
		if(home != NULL) {
			int* convStatsId = home->getData<int>("conv_stats", false);
			if(convStatsId == NULL) {
				convStatsId = new int(MyStats::Get()->registerStat("convergence", "piep", 1));
				home->addDataValue("conv_stats", convStatsId);
			}
			COHDA_WorkingMemory* kappaOwn = home->getData<COHDA_WorkingMemory>("WorkingMemory", false);
			if(err == -1) {
				err = COHDA_error(kappaOwn->getZeta()*kappaOwn->getConfigurationNice()->getSize(), kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getTotalConsumption());
				solutionCreator = kappaOwn->getSolutionCandidateNice()->getAdrString();
			}
			if (kappaOwn->getConfigurationNice()->getSize() != homes->size()) {
				MyStats::Get()->addStat(*convStatsId, 0);
				Log::i(home->getId(), "Solution Candidate incomplete");
			}
			if (err != COHDA_error(kappaOwn->getZeta()*kappaOwn->getConfigurationNice()->getSize(), kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getTotalConsumption())) {
				MyStats::Get()->addStat(*convStatsId, 0);
				Log::i(home->getId(), "different Error");
			}
			if (solutionCreator != kappaOwn->getSolutionCandidateNice()->getAdrString()) {
				MyStats::Get()->addStat(*convStatsId, 0);
				Log::i(home->getId(), "different Solution Creator");
			}
		}
	}
}

static void COHDA_ServerStartInterval(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses = server->getHomeAddresses();
	if(homeAddresses->size()){
		std::map<std::string, std::vector<std::string>*>* overlayNetwork = server->getData<std::map<std::string, std::vector<std::string>*> >("overlay_net", false);
		if(overlayNetwork == NULL) {
			overlayNetwork = new std::map<std::string, std::vector<std::string>*>;
			params->push_back(overlayNetwork);
			Modules::Get()->execute(MyConfig::Get()->getString("generate_overlay_network"), params);
			server->addDataValue("overlay_net", overlayNetwork);
		}

		Log::i(server->getId(), "starting interval...");
		Ipv4Address adr = server->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
		std::string adrString = Helper::toString(adr);

		COHDA_WorkingMemory kappa;
		int* msgStatsId = server->getData<int>("msg_stats", false);
		int* timeStatsId = server->getData<int>("time_stats", false);
		int* dataStatsId = server->getData<int>("data_stats", false);
		MyStats* stats = MyStats::Get();
		if(msgStatsId == NULL) {
			msgStatsId = new int(stats->registerStat("msgs", server->getId()));
			server->addDataValue("msg_stats", msgStatsId);
			timeStatsId = new int(stats->registerStat("time", server->getId()));
			server->addDataValue("time_stats", timeStatsId);
			dataStatsId = new int(stats->registerStat("data", server->getId()));
			server->addDataValue("data_stats", dataStatsId);
		}

		COHDA_Token* token = new COHDA_Token(*timeStatsId, *msgStatsId, *dataStatsId, overlayNetwork, adrString, &kappa);
		std::vector<double>* goal = DsmMode::Get()->getQualityGoal();
		token->getWorkingMemoryNice()->setZeta(goal->at(Tick::Get()->getTick() % goal->size()));

		if(MyConfig::Get()->getBool("send_first_token_random") == true) {
			int adrIndexRandom =  Random::Get()->get(0,homeAddresses->size()) % (homeAddresses->size());
			adr = homeAddresses->at(adrIndexRandom);
		} else
			adr = homeAddresses->at(0);
		stats->addStat(*dataStatsId, 4);	//zeta only
		server->scheduleSend(Helper::toString(adr), token->getPacket());

		int interval = MyConfig::Get()->getInt("interval");
		Simulator::Schedule(Seconds(interval*60-1.1), &COHDA_CheckConvergence);
	}
//	COHDA_scheduler_init();
}


}


#endif
