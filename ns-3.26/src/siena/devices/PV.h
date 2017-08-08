#ifndef PV_H_
#define PV_H_

#include "ns3/Device.h"
#include "ns3/DataBasis.h"
#include "ns3/Random.h"
#include "ns3/Helper.h"

namespace ns3 {

class PV : public Device {

private:
	double size;

public:
	PV(std::string id);
	virtual ~PV() {}
	virtual void init(Json::Value config);
	virtual void tick();
	virtual double getConsumption();
	virtual bool isAdaptable();
	virtual std::pair<AdaptionFlex*, AdaptionOnOff*> getAdaption();
	void setSize(double size) { this->size = size; configName = Helper::toString(size); }

};

}

#endif
