#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_GRIDHOME_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_GRIDHOME_H_

#include "ns3/GridHome.h"
#include "ns3/PrivadePlus_Helper.h"
#include "ns3/PrivadePlus_TokenShare.h"

namespace ns3 {

static void privadePlus_GridHomeTick(std::vector<void*>* params) {
	GridHome* home = (GridHome*) params->at(0);
	MyConfig* config = MyConfig::Get();
	Tick* tick = Tick::Get();
	int* lastTick = home->getData<int>("LastTick", false);
	int* backoff = home->getData<int>("backoff", false);
	if(lastTick == NULL) {
		lastTick = new int(-1000);
		home->addDataValue("LastTick", lastTick);
		backoff = new int(config->getInt("privade_backoff"));
		home->addDataValue("backoff", backoff);
	}

	if(tick->getTick() > *lastTick) {
		*lastTick = tick->getTick();
		PrivadePlus_TokenData* token = new PrivadePlus_TokenData(false, NULL, NULL);
		Adaption* adaption = home->getAdaption();

		// gather flex data
		AdaptionFlex* flex = adaption->getFlex();
		if(flex->isAdaptable()) {
			if(flex->isAdaptableUp())
				token->addFlexUp(flex->getPotentialUp());
			if(flex->isAdaptableDown())
				token->addFlexDown(flex->getPotentialDown());
			token->addCurrent(adaption->getBase() + flex->getDesired());
		} else
			token->addCurrent(adaption->getBase());

		// switch devices
		if(adaption->isOnOffAdaptable()) {
			std::vector<AdaptionOnOff*>* prio = adaption->getOnOff();
			for(uint i = 0; i < prio->size(); i++) {
				AdaptionOnOff* a = (*prio)[i];
				token->addSwitchable(a->getCategory(), a->getCategoryValue(), 0);
			}
			prio = adaption->getOnOffDesired();
			for(uint i = 0; i < prio->size(); i++) {
				AdaptionOnOff* a = (*prio)[i];
				token->addSwitchable(a->getCategory(), a->getCategoryValue(), 1);
			}
		}

		Log::i(home->getId(), "\tsending token to aggregator");
		std::string serverIp = *home->getData<std::string>("serverIp");
		Time t = MicroSeconds(Random::Get()->get(0, *backoff));
		if(*lastTick == 0)
			t += Seconds(0.1);
		home->scheduleSend(t, serverIp, token->getPacket());
	}
}


static void privadePlus_GridHomeHandleToken(std::vector<void*>* params) {
	GridHome* home = (GridHome*) (*params)[0];
	PrivadePlus_TokenShare* token = (PrivadePlus_TokenShare*) (*params)[1];
	Adaption* adaption = home->getAdaption();

	PrivadePlus_TokenAdapt* tokenAdapt = new PrivadePlus_TokenAdapt(NULL);
	MyStats::Get()->addStat(token->getTimeStatsId(), Tick::Get()->getTimeInInterval());

	// gather flex data
	AdaptionFlex* flex = adaption->getFlex();
	double share = token->getShareFlex();
	if((share > 0 && flex->isAdaptableUp()) || (share < 0 && flex->isAdaptableDown())) {
		double take = 0;
		if(share < 0) { // reduce consumption
			double tmp = flex->getAssigned() + share;
			if(tmp <= flex->getMinimum()) {
				take = -fabs(flex->getMinimum() - flex->getAssigned());
				flex->setAssigned(flex->getMinimum());
				flex->setAdaptable(false);
			} else {
				flex->setAssigned(tmp);
				take = share;
			}
		} else { // increase consumption
			double tmp = flex->getAssigned() + share;
			if(tmp >= flex->getMaximum()) {
				take = fabs(flex->getMaximum() - flex->getAssigned());
				flex->setAssigned(flex->getMaximum());
				flex->setAdaptable(false);
			} else {
				flex->setAssigned(tmp);
				take = share;
			}
		}
		tokenAdapt->addResidual(share - take);
		#ifndef TURBO
				std::stringstream s;
				s << "\tshare: " << share << ", take: " << take << ", residual: " << share - take << " (" << flex->getMinimum() << "/" << flex->getMaximum() << "/" << flex->getAssigned() << ")";
				Log::i(home->getId(), s.str());
		#endif
		if(flex->isAdaptable())
			tokenAdapt->addFlex();

		// set devices
		privadePlus_setAdaptionResult(home, flex);
	}

	// switch devices
	if(adaption->isOnOffAdaptable()) {
		AdaptionCategories* cats = AdaptionCategories::Get();
		std::map<std::string, Device*>::iterator it;
		for(it = home->getDevices()->begin(); it != home->getDevices()->end(); ++it) {
			if(it->second->isAdaptable()) {
				AdaptionOnOff* a = it->second->getAdaption().second;
				if(a != NULL && token->hasToSwitch(a->getCategory(), a->isDesired() ? 1 : 0)) {
					Device* dev = it->second;
					if(cats->getOnOffCategory(dev->getConsumption()) != a->getCategory()) { // dev does not fulfill category currently -> toggle
						dev->setState(dev->getState() == DEV_ON ? DEV_OFF : DEV_ON);
						dev->tick();
						#ifndef TURBO
							std::stringstream s;
							s << "\tswitching device " << dev->getId() << " -> " << dev->getState() << ", category: " << a->getCategory() << ", desired: " << a->isDesired();
							Log::i(home->getId(), s.str());
						#endif
					} else {
						#ifndef TURBO
							std::stringstream s;
							s << "\tdevice " << dev->getId() << " already fulfills switching category " << a->getCategory() << ", desired: " << a->isDesired();
							Log::i(home->getId(), s.str());
						#endif
					}
				}
			}
		}
	}

	Log::i(home->getId(), "\tsending token to aggregator");
	std::string serverIp = *home->getData<std::string>("serverIp");
	home->scheduleSend(serverIp, tokenAdapt->getPacket());

	delete token;
}

}

#endif
