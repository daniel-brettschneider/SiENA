#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PPPM_AUCTIONEER_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PPPM_AUCTIONEER_H_

#include <vector>
#include <string>

#include "ns3/ClusterServer.h"
#include "ns3/Log.h"
#include "ns3/PPPM_Bid.h"
#include "ns3/PPPM_Update.h"
#include "ns3/MyStats.h"
#include "ns3/DataBasis.h"
#include "ns3/MyConfig.h"
#include "ns3/Tick.h"
#include "ns3/DsmMode.h"

namespace ns3 {

static void pppm_AuctioneerHandleToken(std::vector<void*>* params) {
	ClusterServer* server = (ClusterServer*) (*params)[0];
	PPPM_Bid* bid = (PPPM_Bid*) (*params)[1];
	if (bid->getTick() != Tick::Get()->getTick())
		return;
	std::vector<double>* goals = server->getData<std::vector<double> >("goals", false);
	int* numberOfParticipants = server->getData<int>("numberOfParticipants", false);
	double* minPrice = server->getData<double>("minPrice", false);
	double* maxPrice = server->getData<double>("maxPrice", false);
	int* timeStatsId = server->getData<int>("time_stats", false);
	int* dataStatsId = server->getData<int>("data_stats", false);
	int* msgStatsId = server->getData<int>("msg_stats", false);
	int* lastTick = server->getData<int>("lastTick", false);
	int* convStatsId = server->getData<int>("conv_stats", false);
	int* stages = server->getData<int>("stages", false);
	int* stageCounter = server->getData<int>("stage_counter", false);
	if(goals == NULL || numberOfParticipants == NULL || minPrice == NULL || maxPrice == NULL || dataStatsId == NULL || msgStatsId == NULL) {
		MyConfig* config = MyConfig::Get();
		lastTick = new int(-1);
		server->addDataValue("lastTick", lastTick);
		goals = DsmMode::Get()->getQualityGoal();
		server->addDataValue("goals", goals);
		int homes = config->getInt("homes");
		double gridhomeShare = config->getDouble("gridhome_share");
//		numberOfParticipants = &gridHomes;
		numberOfParticipants = new int(gridhomeShare * homes);
		server->addDataValue("numberOfParticipants", numberOfParticipants);
		minPrice = new double(config->getDouble("min_price"));
		server->addDataValue("minPrice", minPrice);
		maxPrice = new double(config->getDouble("max_price"));
		server->addDataValue("maxPrice", maxPrice);
		timeStatsId = new int(MyStats::Get()->registerStat("time", server->getId(), 0));
		server->addDataValue("time_stats", timeStatsId);
		dataStatsId = new int(MyStats::Get()->registerStat("data", server->getId()));
		server->addDataValue("data_stats", dataStatsId);
		msgStatsId = new int(MyStats::Get()->registerStat("msgs", server->getId()));
		server->addDataValue("msg_stats", msgStatsId);
		convStatsId = new int(MyStats::Get()->registerStat("convergence", server->getId(), 1));
		server->addDataValue("conv_stats", convStatsId);
		stages = new int(config->getInt("stages"));
		server->addDataValue("stages", stages);
		stageCounter = new int(0);
		server->addDataValue("stage_counter", stageCounter);
	}

	Log::i(server->getId(), "received bid");
	std::vector<PPPM_Bid*>* bids = server->getData<std::vector<PPPM_Bid*> >("bids");
	if(*lastTick != Tick::Get()->getTick()) {
		if(bids->size()>0) {
			for(uint i = 0; i < bids->size(); i++) {
				delete (*bids)[i];
			}
			bids->clear();
			Log::f(server->getId(), "not converged");		//
			MyStats::Get()->addStat(*convStatsId, 0);
		}
		MyConfig* config = MyConfig::Get();
		*minPrice = config->getDouble("min_price");
		*maxPrice = config->getDouble("max_price");
		*stageCounter = 0;
		*lastTick = Tick::Get()->getTick();
	}
	bids->push_back(bid);
	Log::i(server->getId(), "\tbids: ", bids->size());
	if(bids->size() >= (uint) server->getChildren()->size()) {
		Log::i(server->getId(), "\tall bids received");
		(*stageCounter)++;
		Log::i(server->getId(), "\tstage ", *stageCounter);
		PPPM_Bid total(NULL, *minPrice, *maxPrice);
		for(uint i = 0; i < bids->size(); i++) {
			total.addBid((*bids)[i]);
			delete (*bids)[i];
		}
		bids->clear();

		Log::i(server->getId(), "\tconcentrated bid: ", total.toString());
		double goal = (*goals)[Tick::Get()->getMinute() % goals->size()] * *numberOfParticipants;
		Log::i(server->getId(), "\tgoal consumption: ", goal);

		double price = total.findEquilibrium(goal);
		int* priceStatsId = server->getData<int>("price_stat", false);
		MyStats* stats = MyStats::Get();
		if(priceStatsId == NULL)
			priceStatsId = new int(stats->registerStat("powermatcher", "price"));
		stats->addStat(*priceStatsId, price);
		stats->addStat(*dataStatsId, 0);
		stats->addStat(*msgStatsId, 0);


		// check stage
		std::pair<double, double> range = std::pair<double, double>(1, 0);
		if(*stageCounter < *stages) {
			range = total.findNewRange(goal);
			if(range.first < range.second) {
				*minPrice = range.first;
				*maxPrice = range.second;
				std::stringstream s;
				s << "\tsetting new stage: [" << *minPrice << ", " << *maxPrice << "]";
				Log::i(server->getId(), s.str());
				for(uint i = 0; i < bids->size(); i++) {
					delete (*bids)[i];
				}
				bids->clear();
			} else
				Log::i(server->getId(), "\tnew stage unnecessary, price: ", price);
		} else {
			Log::i(server->getId(), "\tprice: ", price);
			Log::i(server->getId(), "\t=> consumption: ", total.getConsumption(price));
		}

		Log::i(server->getId(), "\tsending update to concentrators");
		Ptr<AttributeContainer<Ipv4Address> > adrs = server->getChildren();
		for(int i = 0; i < adrs->size(); i++) {
			Ipv4Address adr = adrs->at(i);
			PPPM_Update* update = NULL;
			if(range.first < range.second && *stageCounter < *stages)
				update = new PPPM_Update(*minPrice, *maxPrice, *timeStatsId);
			else
				update = new PPPM_Update(price, *timeStatsId);
			stats->increaseStat(*dataStatsId, 4);
			stats->increaseStat(*msgStatsId, 1);
			server->scheduleSend(Helper::toString(adr), update->getPacket());
		}
	}
}

}

#endif
