#ifndef ADAPTION_H_
#define ADAPTION_H_

#include <vector>
#include <string>
#include <sstream>
#include <climits>
#include "ns3/Turbo.h"
#include "ns3/DataBasis.h"
#include "ns3/AdaptionFlex.h"
#include "ns3/AdaptionOnOff.h"
#include "ns3/AdaptionCategories.h"
#include <cmath>

namespace ns3 {

class Adaption {

private:
	AdaptionFlex flex;
	//std::vector<int> onOff;
	//std::vector<int> onOffDesired;
	std::vector<AdaptionOnOff*> onOff;
	std::vector<AdaptionOnOff*> onOffDesired;
	bool adaptable;
	bool onOffAdaptable;
	double base;
	AdaptionCategories* cats;

public:
	Adaption() : adaptable(false), onOffAdaptable(false), base(0) {
		cats = AdaptionCategories::Get();
	}
	virtual ~Adaption() {}

	double getBase() { return base; }
	void setBase(double base) { this->base = base; }

	void setFlex(AdaptionFlex flex) {
		this->flex = flex;
		if(flex.isAdaptable())
			adaptable = true;
	}
	AdaptionFlex* getFlex() { return &flex; }

	void addOnOff(AdaptionOnOff* a) {
		if(a->isDesired())
			onOffDesired.push_back(a);
		else
			onOff.push_back(a);
		a->setCategory(cats->getOnOffCategory(a->getValue()));
		a->setCategoryValue(cats->getOnOffCategoryRating(a->getCategory()));
		adaptable = true;
		onOffAdaptable = true;
	}
	std::vector<AdaptionOnOff*>* getOnOff() { return &onOff; }
	std::vector<AdaptionOnOff*>* getOnOffDesired() { return &onOffDesired; }

//	int addOnOff(int value, bool desired) {
//		int category = cats->getOnOffCategory(value);
//		if(desired)
//			onOffDesired.at(category)++;
//		else
//			onOff.at(category)++;
//		adaptable = true;
//		onOffAdaptable = true;
//		return category;
//	}
//	void addOnOff(std::vector<int>* onOff, std::vector<int>* desired) {
//		for(uint i = 0; i < onOff->size(); i++) {
//			this->onOff.at(i) += onOff->at(i);
//			this->onOffDesired.at(i) += desired->at(i);
//		}
//		adaptable = true;
//		onOffAdaptable = true;
//	}
//	std::vector<int>* getOnOff() { return &onOff; }
//	std::vector<int>* getOnOffDesired() { return &onOffDesired; }

	bool isAdaptable() { return adaptable; }
	bool isOnOffAdaptable() { return onOffAdaptable; }

};

}

#endif
