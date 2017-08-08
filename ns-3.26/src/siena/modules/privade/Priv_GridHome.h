#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_GRIDHOME_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_GRIDHOME_H_

#include "ns3/GridHome.h"
#include "ns3/Priv_Helper.h"
#include "ns3/Priv_TokenData.h"
#include "ns3/Priv_TokenAdapt.h"

namespace ns3 {

static void priv_GridHomeHandleDataToken(GridHome* home, Priv_TokenData* token) {
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

	home->scheduleSend(token->next(), token->getPacket());
}

static void priv_GridHomeHandleAdaptToken(GridHome* home, Priv_TokenAdapt* token) {
	Adaption* adaption = home->getAdaption();

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
		token->addResidual(share - take);
		#ifndef TURBO
				std::stringstream s;
				s << "\tshare: " << share << ", take: " << take << ", residual: " << share - take << " (" << flex->getMinimum() << "/" << flex->getMaximum() << "/" << flex->getAssigned() << ")";
				Log::i(home->getId(), s.str());
		#endif
		if(flex->isAdaptable())
			token->addFlex();

		// set devices
		priv_setAdaptionResult(home, flex);
//		MMF_Data mmfSum;
//		std::vector<MMF_Data> mmfData;
//		std::map<std::string, Device*>::iterator it;
//		std::map<std::string, Device*>* devices = home->getDevices();
//		for(it = devices->begin(); it != devices->end(); ++it) {
//			AdaptionFlex* a = it->second->getAdaption().first;
//			if(a->isAdaptable()) {
//				MMF_Data d(it->first, a->isAdaptableUp() ? 1 : 0, a->isAdaptableDown() ? 1 : 0, a->getPotentialUp(), a->getPotentialDown(), a->getAssigned());
//				mmfSum.add(&d);
//				mmfData.push_back(d);
//			}
//		}
//		priv_MaxMinFairShare(&mmfData, take, &mmfSum);
//		for(uint i = 0; i < mmfData.size(); i++) {
//			devices->at(mmfData[i].id)->setAdaptedConsumption(mmfData[i].assigned);
//		}
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

	home->scheduleSend(token->next(), token->getPacket());
}

static void priv_GridHomeHandleToken(std::vector<void*>* params) {
	GridHome* home = (GridHome*) (*params)[0];
	Token* token = (Token*) (*params)[1];
	Priv_TokenData* tokenData = dynamic_cast<Priv_TokenData*>(token);
	if(tokenData != NULL)
		priv_GridHomeHandleDataToken(home, tokenData);
	else
		priv_GridHomeHandleAdaptToken(home, (Priv_TokenAdapt*) token);
}

}

#endif
