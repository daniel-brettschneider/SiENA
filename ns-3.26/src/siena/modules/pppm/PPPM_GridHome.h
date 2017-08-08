#ifndef NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PPPM_GRIDHOME_H_
#define NS_3_22_SRC_CITYGRID_MODULES_POWERMATCHER_PPPM_GRIDHOME_H_

#include "ns3/GridHome.h"
#include "ns3/PM_PriceUpdate.h"
#include "ns3/PM_Bid.h"
#include "ns3/Log.h"
#include "ns3/PM_DeviceAgent_Battery.h"
#include "ns3/PM_DeviceAgent_CHP.h"
#include "ns3/PM_DeviceAgent_Heatingrod.h"
#include "ns3/PM_DeviceAgent_HP.h"
#include "ns3/PM_DeviceAgent_Car.h"
#include "ns3/PM_DeviceAgent_Basic.h"
#include "ns3/Helper.h"
#include "ns3/RunningAvg.h"
#include "ns3/PPPM_Update.h"

namespace ns3 {

static void pppm_aggregate_devicebids(GridHome* home, PPPM_Bid* bid) {
	std::map<std::string, Device*>* devices = home->getDevices();
	std::map<std::string, PM_Bid*>* bidStorage = home->getData<std::map<std::string, PM_Bid*> >("bid_storage");
	RunningAvg* avg = home->getData<RunningAvg>("price_avg");
	double minPrice = bid->getMinPrice();
	double resolution = bid->getResolution();
	for(std::map<std::string, Device*>::iterator it = devices->begin(); it != devices->end(); ++it) {
		std::string type = Helper::getDeviceType(it->first);
		if(it->first.find(':') != std::string::npos)
			type = Helper::split(it->first, '_').back();
		PM_Bid* deviceBid = NULL;
		if(type == "battery") {
			deviceBid = powermatcher_Battery_getBid((Battery*) it->second, avg);
		} else if(type == "chps") {
			deviceBid = powermatcher_CHP_getBid((CHPS*) it->second, avg);
		} else if(type == "heatingrod") {
			deviceBid = powermatcher_Heatingrod_getBid((HeatingRod*) it->second, avg);
		} else if(type == "heatpump") {
			deviceBid = powermatcher_HeatPump_getBid((HeatPump*) it->second, avg);
		} else if(type.find("car") != std::string::npos) {
			deviceBid = powermatcher_Car_getBid((Car*) it->second, avg);
		} else
			deviceBid = powermatcher_Basic_getBid(it->second);

		for(int p = 0; p < bid->getPriceSteps(); p++) {
			bid->addCoordinate(p, deviceBid->getConsumption(minPrice+p*resolution));
		}

		std::map<std::string, PM_Bid*>::iterator jt = bidStorage->find(type);
		if(jt != bidStorage->end()) {
			delete jt->second;
			bidStorage->at(type) = deviceBid;
		} else
			bidStorage->insert(std::pair<std::string, PM_Bid*>(type, deviceBid));
		}
}

static void pppm_GridHomeTick(std::vector<void*>* params) {
	GridHome* home = (GridHome*) params->at(0);
	Tick* tick = Tick::Get();
	int* lastTick = home->getData<int>("LastTick", false);
	if(lastTick == NULL) {
		lastTick = new int(-1000);
		home->addDataValue("LastTick", lastTick);
	}

	if(tick->getTick() > *lastTick) {
		*lastTick = tick->getTick();

		PPPM_Bid* bid = new PPPM_Bid(NULL);
		pppm_aggregate_devicebids(home, bid);

		Log::i(home->getId(), "\tbid aggregated: " + bid->toString());
		Log::i(home->getId(), "\tsending bid to concentrator");
		std::string serverIp = *((std::string*) home->getData<std::string>("serverIp"));
		Time t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff")));
		if(*lastTick == 0)
			t += Seconds(0.1);
		home->scheduleSend(t, serverIp, bid->getPacket());
	}
}

static void pppm_home_handlePriceUpdate(GridHome* home, PPPM_Update* up) {
	if(up->getMode() == PPPM_MODE_PRICE) {
		std::map<std::string, Device*>* devices = home->getDevices();
		std::map<std::string, PM_Bid*>* bidStorage = home->getData<std::map<std::string, PM_Bid*> >("bid_storage");

		int* convStatsId = home->getData<int>("conv_stats", false);
		if(convStatsId == NULL) {
			convStatsId = new int(MyStats::Get()->registerStat("convergence", home->getId(), 0));
			home->addDataValue("conv_stats", convStatsId);
		}

		if(up->getTick() != Tick::Get()->getTick()) {
			MyStats::Get()->getStats()->at(up->getStatId())->at(up->getTick()) = -1;
			return;
		}
		MyStats::Get()->addStat(*convStatsId, 1);
		double price = up->getPrice();
		up->saveStat();
		Log::i(home->getId(), "received price update: ", price);
		RunningAvg* avg = home->getData<RunningAvg>("price_avg");
		avg->add(price);

		for(std::map<std::string, Device*>::iterator it = devices->begin(); it != devices->end(); ++it) {
			std::string type = Helper::getDeviceType(it->first);
			if(it->first.find(':') != std::string::npos)
				type = Helper::split(it->first, '_').back();
			double consumption = bidStorage->at(type)->getConsumption(price, true);		//true = consumption shoud be behond the goal, false = above
			Log::i(home->getId(), "\t" + type + " consumption: ", consumption);
			if(type == "battery")
				powermatcher_Battery_setConsumption((Battery*) it->second, consumption);
			else if(type == "chps")
				powermatcher_CHP_setConsumption((CHPS*) it->second, consumption);
			else if(type == "heatingrod")
				powermatcher_Heatingrod_setConsumption((HeatingRod*) it->second, consumption);
			else if(type == "heatpump")
				powermatcher_Heatpump_setConsumption((HeatPump*) it->second, consumption);
			else if(type.find("car") != std::string::npos)
				powermatcher_Car_setConsumption((Car*) it->second, consumption);
		}
	} else {
		Log::i(home->getId(), "received update, next round");
		PPPM_Bid* bid = new PPPM_Bid(NULL, up->getMinPrice(), up->getMaxPrice());
		pppm_aggregate_devicebids(home, bid);
		Log::i(home->getId(), "\tbid aggregated: " + bid->toString());
		Log::i(home->getId(), "\tsending bid to concentrator");
		std::string serverIp = *((std::string*) home->getData<std::string>("serverIp"));
		Time t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff")));
		home->scheduleSend(t, serverIp, bid->getPacket());
	}

	delete up;
}

static void pppm_GridHomeHandleToken(std::vector<void*>* params) {
	GridHome* home = (GridHome*) (*params)[0];
	Token* token = (Token*) (*params)[1];
	PPPM_Update* bid = dynamic_cast<PPPM_Update*>(token);
	pppm_home_handlePriceUpdate(home, bid);
}

}

#endif
