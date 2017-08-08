#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_BASIC_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_DEVICEAGENT_BASIC_H_

#include "ns3/PM_Bid.h"
#include "ns3/Device.h"

namespace ns3 {

PM_Bid* powermatcher_Basic_getBid(Device* dev) {	//Currently no histeresis
	PM_Bid* bid = new PM_Bid;
	bid->addCoordinate(0, dev->getConsumption());
	return bid;
}

}

#endif
