#ifndef SHIFTABLEDEVICE_H_
#define SHIFTABLEDEVICE_H_

#include "ns3/Device.h"

namespace ns3 {

class ShiftableDevice : public Device {

protected:
	int additionalRuns;

public:
	ShiftableDevice(std::string id, std::vector<double>* consumption, double standby) : Device(id, DEV_MODE_SINGLE_RUN, consumption, standby), additionalRuns(0) {}
	virtual ~ShiftableDevice() {}
	virtual void tick();
	virtual void setState(int state, int data);

};

}

#endif
