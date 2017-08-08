#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PPPM_CONCENTRATOR_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PPPM_CONCENTRATOR_H_

#include <vector>
#include <string>
#include "ns3/Server.h"
#include "ns3/PM_Bid.h"
#include "ns3/Log.h"
#include "ns3/Helper.h"
#include "ns3/PPPM_Update.h"

namespace ns3 {

static void pppm_handleBid(Server* server, PPPM_Bid* bid) {
	if (bid->getTick() != Tick::Get()->getTick())
		return;
	Log::i(server->getId(), "received bid");
	std::vector<PPPM_Bid*>* bids = server->getData<std::vector<PPPM_Bid*> >("bids");
	int* lastTick = server->getData<int>("lastTick", false);
	int* convStatsId = server->getData<int>("conv_stats", false);
	if(lastTick == NULL) {
		lastTick = new int(-1);
		server->addDataValue("lastTick", lastTick);
		convStatsId = new int(MyStats::Get()->registerStat("convergence", server->getId(), 1));
		server->addDataValue("conv_stats", convStatsId);
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
	}
	bids->push_back(bid);
	Log::i(server->getId(), "\tbids: ", bids->size());

	if(bids->size() >= (uint) server->getNumberOfParticipants()) {
		Log::i(server->getId(), "\tall bids received, sending to auctioneer");
		PPPM_Bid* total = new PPPM_Bid(NULL, (*bids)[0]->getMinPrice(), (*bids)[0]->getMaxPrice());

		int* msgStatsId = server->getData<int>("msg_stats", false);
		int* dataStatsId = server->getData<int>("data_stats", false);
		MyStats* stats = MyStats::Get();
		if(msgStatsId == NULL) {
			msgStatsId = new int(stats->registerStat("msgs", server->getId()));
			server->addDataValue("msg_stats", msgStatsId);
			dataStatsId = new int(stats->registerStat("data", server->getId()));
			server->addDataValue("data_stats", dataStatsId);
		}
		stats->addStat(*dataStatsId, 0);
		stats->addStat(*msgStatsId, 0);
		for(uint i = 0; i < bids->size(); i++) {
			stats->increaseStat(*dataStatsId, (*bids)[i]->getSize());
			stats->increaseStat(*msgStatsId, 1);
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

static void pppm_handlePriceUpdate(Server* server, PPPM_Update* up) {
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
			PPPM_Update* priceUpdate = NULL;
			if(up->getMode() == PPPM_MODE_PRICE)
				priceUpdate = new PPPM_Update(up->getPrice(), up->getStatId());
			else
				priceUpdate = new PPPM_Update(up->getMinPrice(), up->getMaxPrice(), up->getStatId());
			Time t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff")));	//random delay up to 100 ms
			server->scheduleSend(t, Helper::toString(adr), priceUpdate->getPacket());
		}
	}
	delete up;
}

static void pppm_ConcentratorHandleToken(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	Token* token = (Token*) (*params)[1];
	PPPM_Bid* bid = dynamic_cast<PPPM_Bid*>(token);
	if(bid != NULL)
		pppm_handleBid(server, bid);
	else
		pppm_handlePriceUpdate(server, (PPPM_Update*) token);
}

}

#endif
