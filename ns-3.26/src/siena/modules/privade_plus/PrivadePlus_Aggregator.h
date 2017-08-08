#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_AGGREGATOR_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_AGGREGATOR_H_

#include <vector>
#include "ns3/Log.h"
#include "ns3/Server.h"
#include "ns3/PrivadePlus_TokenData.h"
#include "ns3/PrivadePlus_TokenAdapt.h"

namespace ns3 {

static void privadePlus_AggregatorHandleToken(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	Token* token = (Token*) (*params)[1];

	// data
	Log::i(server->getId(), "received token");
	std::vector<Token*>* tokens = server->getData<std::vector<Token*> >("tokens");
	int* lastTick = server->getData<int>("lastTick", false);
	int* convStatsId = server->getData<int>("conv_stats", false);
	if(lastTick == NULL) {
		lastTick = new int(-1);
		server->addDataValue("lastTick", lastTick);
		convStatsId = new int(MyStats::Get()->registerStat("convergence", server->getId(), 1));
		server->addDataValue("conv_stats", convStatsId);
	}

	// convergence
	if(*lastTick != Tick::Get()->getTick()) {
		if(tokens->size() > 0) {
			for(uint i = 0; i < tokens->size(); i++) {
				delete (*tokens)[i];
			}
			tokens->clear();
			Log::f(server->getId(), "not converged");
			MyStats::Get()->addStat(*convStatsId, 0);
		}
		*lastTick = Tick::Get()->getTick();
	}

	// store token
	tokens->push_back(token);
	Log::i(server->getId(), "\ttokens: ", tokens->size());

	// all received
	if(tokens->size() >= (uint) server->getNumberOfParticipants()) {
		Log::i(server->getId(), "\tall tokens received, aggregating...");

		// aggregate and stats
		Token* aggregated = (*tokens)[0];
		PrivadePlus_TokenData* data = dynamic_cast<PrivadePlus_TokenData*>(aggregated);
		if(data != NULL) {
			for(uint i = 1; i < tokens->size(); i++) {
				PrivadePlus_TokenData* d = (PrivadePlus_TokenData*) (*tokens)[i];
				data->add(d);
				delete d;
			}
		} else {
			PrivadePlus_TokenAdapt* adapt = dynamic_cast<PrivadePlus_TokenAdapt*>(aggregated);
			for(uint i = 1; i < tokens->size(); i++) {
				PrivadePlus_TokenAdapt* a = (PrivadePlus_TokenAdapt*) (*tokens)[i];
				adapt->add(a);
				delete a;
			}
		}
		tokens->clear();

		Log::i(server->getId(), "\tsending to server");
		Ipv4Address serverAdr = server->getServerAddress();
		server->scheduleSend(Helper::toString(serverAdr), aggregated->getPacket());
	}
}

}

#endif
