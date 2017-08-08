#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PMP_AUCTIONEER_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PMP_AUCTIONEER_H_

#include <vector>
#include <string>

#include "ns3/ClusterServer.h"
#include "ns3/Log.h"
#include "ns3/PM_Bid.h"
#include "ns3/PM_PriceUpdate.h"
#include "ns3/MyStats.h"
#include "ns3/DataBasis.h"
#include "ns3/MyConfig.h"
#include "ns3/Tick.h"
#include "ns3/DsmMode.h"

namespace ns3 {

static void powermatcher_AuctioneerHandleToken(std::vector<void*>* params) {
	ClusterServer* server = (ClusterServer*) (*params)[0];
	PM_Bid* bid = (PM_Bid*) (*params)[1];
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
	if(goals == NULL || numberOfParticipants == NULL || minPrice == NULL || maxPrice == NULL || dataStatsId == NULL || msgStatsId == NULL) {
		lastTick = new int(-1);
		server->addDataValue("lastTick", lastTick);
		goals = DsmMode::Get()->getQualityGoal();
		server->addDataValue("goals", goals);
		int homes = MyConfig::Get()->getInt("homes");
		double gridhomeShare = MyConfig::Get()->getDouble("gridhome_share");
//		numberOfParticipants = &gridHomes;
		numberOfParticipants = new int(gridhomeShare * homes);
		server->addDataValue("numberOfParticipants", numberOfParticipants);
		minPrice = new double(MyConfig::Get()->getDouble("min_price"));
		server->addDataValue("minPrice", minPrice);
		maxPrice = new double(MyConfig::Get()->getDouble("max_price"));
		server->addDataValue("maxPrice", maxPrice);
		timeStatsId = new int(MyStats::Get()->registerStat("time", server->getId(), 0));
		server->addDataValue("time_stats", timeStatsId);
		dataStatsId = new int(MyStats::Get()->registerStat("data", server->getId()));
		server->addDataValue("data_stats", dataStatsId);
		msgStatsId = new int(MyStats::Get()->registerStat("msgs", server->getId()));
		server->addDataValue("msg_stats", msgStatsId);
		convStatsId = new int(MyStats::Get()->registerStat("convergence", server->getId(), 1));
		server->addDataValue("conv_stats", convStatsId);
	}


	Log::i(server->getId(), "received bid");
	std::vector<PM_Bid*>* bids = server->getData<std::vector<PM_Bid*> >("bids");
	Log::i(server->getId(), "\tbids: ", bids->size());
	if(*lastTick != Tick::Get()->getTick()) {
		if(bids->size()>0) {
			for(uint i = 0; i < bids->size(); i++) {
				delete (*bids)[i];
			}
			bids->clear();
			Log::f(server->getId(), "not converged");		//
			MyStats::Get()->addStat(*convStatsId, 0);
		}
		*lastTick = Tick::Get()->getTick();
	}
	bids->push_back(bid);
	if(bids->size() >= (uint) server->getChildren()->size()) {
		Log::i(server->getId(), "\tall bids received");
		PM_Bid total;
		for(uint i = 0; i < bids->size(); i++) {
			total.addBid((*bids)[i]);
			delete (*bids)[i];
		}
		bids->clear();
		Log::i(server->getId(), "\tconcentrated bid: ", total.toString());
		double goal = (*goals)[Tick::Get()->getMinute() % goals->size()] * *numberOfParticipants;
		Log::i(server->getId(), "\tgoal consumption: ", goal);
		double price = total.findEquilibrium(goal, minPrice, maxPrice);
//		std::cout.precision(17);
		Log::i(server->getId(), "\tprice: ", price);
		int* priceStatsId = server->getData<int>("price_stat", false);
		MyStats* stats = MyStats::Get();
		if(priceStatsId == NULL)
			priceStatsId = new int(stats->registerStat("powermatcher", "price"));
		stats->addStat(*priceStatsId, price);
		stats->addStat(*dataStatsId, 0);
		stats->addStat(*msgStatsId, 0);

		Log::i(server->getId(), "\tsending price updates to concentraters");
		Ptr<AttributeContainer<Ipv4Address> > adrs = server->getChildren();
		for(int i = 0; i < adrs->size(); i++) {
			Ipv4Address adr = adrs->at(i);
			PM_PriceUpdate* priceUpdate = new PM_PriceUpdate(price, *timeStatsId);
			stats->increaseStat(*dataStatsId, 4);
			stats->increaseStat(*msgStatsId, 1);
			server->scheduleSend(Helper::toString(adr), priceUpdate->getPacket());
		}
	}
}

}

#endif
