#ifndef NS_3_22_SRC_SIENA_MODULES_MISCELLANEOUS_DO_NOT_SHIFT_H_
#define NS_3_22_SRC_SIENA_MODULES_MISCELLANEOUS_DO_NOT_SHIFT_H_

#include "ns3/Device.h"

namespace ns3 {

static void do_not_shift(std::vector<void*>* params) {
	// just turn it on
	DeviceEvent* event = (DeviceEvent*) (*params)[1];
	event->getDevice()->setState(DEV_ON);
	event->getDevice()->tick();
}

}

#endif
