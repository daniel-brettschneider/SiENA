#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_CONCENTRATOR_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_CONCENTRATOR_H_

#include <vector>
#include <string>
#include "ns3/Server.h"
#include "ns3/PM_Bid.h"
#include "ns3/Log.h"
#include "ns3/Helper.h"
#include "ns3/PM_PriceUpdate.h"

namespace ns3 {

static void powermatcher_handleBid(Server* server, PM_Bid* bid) {
	if (bid->getTick() != Tick::Get()->getTick())
		return;
	Log::i(server->getId(), "received bid");
	//vorher könnten anfragen gescheduled werden (2*max_time). diese müssten gelöscht werden, wenn ein bid erhalten wird.
	std::vector<PM_Bid*>* bids = server->getData<std::vector<PM_Bid*> >("bids");
	int* lastTick = server->getData<int>("lastTick", false);
	int* convStatsId = server->getData<int>("conv_stats", false);
	if(lastTick == NULL) {
		lastTick = new int(-1);
		server->addDataValue("lastTick", lastTick);
		convStatsId = new int(MyStats::Get()->registerStat("convergence", server->getId(), 1));
		server->addDataValue("conv_stats", convStatsId);
	}

	int* msgStatsId = server->getData<int>("msg_stats", false);
	int* dataStatsId = server->getData<int>("data_stats", false);
	MyStats* stats = MyStats::Get();
	if(msgStatsId == NULL) {
		msgStatsId = new int(stats->registerStat("msgs", server->getId()));
		server->addDataValue("msg_stats", msgStatsId);
		dataStatsId = new int(stats->registerStat("data", server->getId()));
		server->addDataValue("data_stats", dataStatsId);
	}

	if(*lastTick != Tick::Get()->getTick()) {
		if(bids->size()>0) {
			for(uint i = 0; i < bids->size(); i++) {
				delete (*bids)[i];
			}
			bids->clear();
			Log::f(server->getId(), "not converged");
			MyStats::Get()->addStat(*convStatsId, 0);
		}
		*lastTick = Tick::Get()->getTick();
		stats->addStat(*dataStatsId, 0);
		stats->addStat(*msgStatsId, 0);
	}
	bids->push_back(bid);
	Log::i(server->getId(), "\tbids: ", bids->size());

	stats->increaseStat(*dataStatsId, bid->getSize());
	stats->increaseStat(*msgStatsId, 1);


	if(bids->size() >= (uint) server->getNumberOfParticipants()) {
		Log::i(server->getId(), "\tall bids received, sending to auctioneer");
		PM_Bid* total = new PM_Bid;

		for(uint i = 0; i < bids->size(); i++) {
			total->addBid((*bids)[i]);
			delete (*bids)[i];
		}
		bids->clear();
		Log::i(server->getId(), "\tconcentrated bid: ", total->toString());
		Log::i(server->getId(), "\tsending to auctioneer: ", total->toString());
		Ipv4Address auctioAdr = server->getServerAddress();
		server->scheduleSend(Helper::toString(auctioAdr), total->getPacket());
	}
}

static void powermatcher_handlePriceUpdate(Server* server, PM_PriceUpdate* up) {
	Log::i(server->getId(), "\treceived price update, forwarding to homes");
	if(up->getTick() == Tick::Get()->getTick()) {
		int* msgStatsId = server->getData<int>("msg_stats", false);
		int* dataStatsId = server->getData<int>("data_stats", false);
		MyStats* stats = MyStats::Get();
		stats->increaseStat(*dataStatsId, up->getSize());
		stats->increaseStat(*msgStatsId, 1);
		Ptr<AttributeContainer<Ipv4Address> > adrs = server->getHomeAddresses();
		for(int i = 0; i < adrs->size(); i++) {
			Ipv4Address adr = adrs->at(i);
			PM_PriceUpdate* priceUpdate = new PM_PriceUpdate(up->getPrice(), up->getStatId());
			//Time t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff")*MyConfig::Get()->getInt("total_homes")));	//random delay up to 100 ms
			Time t = MicroSeconds(0);
			if (MyConfig::Get()->getInt("pm_backoff_price")<0)
				t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff")));	//random delay (reduced due to less data of the price bid)
			else
				t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff_price")));	//random delay (reduced due to less data of the price bid)
			//Time t = MilliSeconds(i*MyConfig::Get()->getInt("time_phousehold"));	//random delay up to 100 ms
			server->scheduleSend(t, Helper::toString(adr), priceUpdate->getPacket());

		}
	}
	delete up;
}

static void powermatcher_ConcentratorHandleToken(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	Token* token = (Token*) (*params)[1];
	PM_Bid* bid = dynamic_cast<PM_Bid*>(token);
	if(bid != NULL)
		powermatcher_handleBid(server, bid);
	else
		powermatcher_handlePriceUpdate(server, (PM_PriceUpdate*) token);
}

}

#endif
