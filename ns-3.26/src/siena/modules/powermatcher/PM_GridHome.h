#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_GRIDHOME_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_GRIDHOME_H_

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
#include <iostream>
#include <string>

namespace ns3 {

static void powermatcher_GridHomeHandleToken(std::vector<void*>* params) {
	//std::cout << "3 " << Simulator::Now() << std::endl;
	//throw("Ende (PM_GridHome)");
	GridHome* home = (GridHome*) params->at(0);
	PM_PriceUpdate* up = (PM_PriceUpdate*) params->at(1);
	std::map<std::string, Device*>* devices = home->getDevices();
	std::map<std::string, PM_Bid*>* bidStorage = home->getData<std::map<std::string, PM_Bid*> >("bid_storage");

	MyStats* stats = MyStats::Get();
	int* convStatsId = home->getData<int>("conv_stats", false);
	int* msgStatsId = home->getData<int>("msg_stats", false);
	int* dataStatsId = home->getData<int>("data_stats", false);
	if(convStatsId == NULL) {
		convStatsId = new int(stats->registerStat("convergence", home->getId(), 0));
		home->addDataValue("conv_stats", convStatsId);
		msgStatsId = new int(stats->registerStat("msgs", home->getId()));
		home->addDataValue("msg_stats", msgStatsId);
		dataStatsId = new int(stats->registerStat("data", home->getId()));
		home->addDataValue("data_stats", dataStatsId);
	}

	if(up->getTick() != Tick::Get()->getTick()) {
		MyStats::Get()->getStats()->at(up->getStatId())->at(up->getTick()) = -1;
		return;
	}
	stats->addStat(*convStatsId, 1);
	stats->addStat(*dataStatsId, up->getSize());
	stats->addStat(*msgStatsId, 1);
	double price = up->getPrice();
	up->saveStat();
	Log::i(home->getId(), "received price update: ", price);
	//std::cout << "received price update: " << price << std::endl;
	RunningAvg* avg = home->getData<RunningAvg>("price_avg");
	avg->add(price);

	bool beyond = MyConfig::Get()->getBool("pm_consumption_beyond");
	//std::cout << "blabla: " << beyond << std::endl;
	for(std::map<std::string, Device*>::iterator it = devices->begin(); it != devices->end(); ++it) {
		std::string type = Helper::getDeviceType(it->first);
		if(it->first.find(':') != std::string::npos)
			type = Helper::split(it->first, '_').back();
		double consumption = bidStorage->at(type)->getConsumption(price, beyond);		//true = consumption shoud be beyond the goal, false = above
		Log::i(home->getId(), "\t" + type + " consumption: ", consumption);
		if(type == "battery")
			powermatcher_Battery_setConsumption((Battery*) it->second, consumption);
		else if(type == "chps") {
			powermatcher_CHP_setConsumption((CHPS*) it->second, consumption);
			//std::cout << "chp: " << consumption << std::endl;
		}
		else if(type == "heatingrod")
			powermatcher_Heatingrod_setConsumption((HeatingRod*) it->second, consumption);
		else if(type == "heatpump")
			powermatcher_Heatpump_setConsumption((HeatPump*) it->second, consumption);
		else if(type.find("car") != std::string::npos)
			powermatcher_Car_setConsumption((Car*) it->second, consumption);
	}
	delete up;
}

static void powermatcher_GridHomeTick(std::vector<void*>* params) {
	GridHome* home = (GridHome*) params->at(0);
	std::map<std::string, Device*>* devices = home->getDevices();
	Tick* tick = Tick::Get();
	int* lastTick = home->getData<int>("LastTick", false);
	Time t_add = Seconds(0);
	MyStats* stats = MyStats::Get();
	int* convStatsId = home->getData<int>("conv_stats", false);
	int* msgStatsId = home->getData<int>("msg_stats", false);
	int* dataStatsId = home->getData<int>("data_stats", false);

	if(lastTick == NULL) {
		lastTick = new int(-1000);
		home->addDataValue("LastTick", lastTick);
		t_add = Seconds(0.1);			//100 Miliseconds due to LTE Start
		if(convStatsId == NULL) {
			convStatsId = new int(stats->registerStat("convergence", home->getId(), 0));
			home->addDataValue("conv_stats", convStatsId);
			msgStatsId = new int(stats->registerStat("msgs", home->getId()));
			home->addDataValue("msg_stats", msgStatsId);
			dataStatsId = new int(stats->registerStat("data", home->getId()));
			home->addDataValue("data_stats", dataStatsId);
		}
	}

	if(tick->getTick() > *lastTick) {
		*lastTick = tick->getTick();
		PM_Bid* totalBid = new PM_Bid;
		std::map<std::string, PM_Bid*>* bidStorage = home->getData<std::map<std::string, PM_Bid*> >("bid_storage");
		RunningAvg* avg = home->getData<RunningAvg>("price_avg");
		for(std::map<std::string, Device*>::iterator it = devices->begin(); it != devices->end(); ++it) {
			std::string type = Helper::getDeviceType(it->first);
			if(it->first.find(':') != std::string::npos)
				type = Helper::split(it->first, '_').back();
			PM_Bid* deviceBid = NULL;
			if(type == "battery") {
				deviceBid = powermatcher_Battery_getBid((Battery*) it->second, avg);
//				std::cout << "battery: " << deviceBid->toString() << std::endl;
			} else if(type == "chps") {
				deviceBid = powermatcher_CHP_getBid((CHPS*) it->second, avg);
//				std::cout << "chps: " << deviceBid->toString() << std::endl;
			} else if(type == "heatingrod") {
				deviceBid = powermatcher_Heatingrod_getBid((HeatingRod*) it->second, avg);
//				std::cout << "heatingrod: " << deviceBid->toString() << std::endl;
			} else if(type == "heatpump") {
				deviceBid = powermatcher_HeatPump_getBid((HeatPump*) it->second, avg);
//				std::cout << "heatpump: " << deviceBid->toString() << std::endl;
			} else if(type.find("car") != std::string::npos) {
				deviceBid = powermatcher_Car_getBid((Car*) it->second, avg);
//				std::cout << "car: " << deviceBid->toString() << std::endl;
			} else
				deviceBid = powermatcher_Basic_getBid(it->second);
//			std::cout << deviceBid->toString() << std::endl;
//			deviceBid->selfcheck();
//			std::cout << totalBid->toString() << std::endl;
			totalBid->addBid(deviceBid);
//			std::cout << totalBid->toString() << std::endl;
			std::map<std::string, PM_Bid*>::iterator jt = bidStorage->find(type);
			if(jt != bidStorage->end()) {
				delete jt->second;
				bidStorage->at(type) = deviceBid;
			} else
				bidStorage->insert(std::pair<std::string, PM_Bid*>(type, deviceBid));
		}
		Log::i(home->getId(), "\tbid created: " + totalBid->toString());
		Log::i(home->getId(), "\tsending bid to concentrator");
		std::string serverIp = *((std::string*) home->getData<std::string>("serverIp"));
//		Time t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff")*MyConfig::Get()->getInt("total_homes")));	//random delay up to 100 ms
		Time t = MicroSeconds(Random::Get()->get(0, MyConfig::Get()->getInt("pm_backoff")));	//random delay up to 100 ms
		//int s = atof(home->getId().erase(0,11).c_str());
		//Time t = MilliSeconds(s*MyConfig::Get()->getInt("time_phousehold"));	//random delay up to 100 ms
		home->scheduleSend(t + t_add, serverIp, totalBid->getPacket());
	}
}

}

#endif
