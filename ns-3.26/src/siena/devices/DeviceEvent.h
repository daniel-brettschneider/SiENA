#ifndef DEVICEEVENT_H_
#define DEVICEEVENT_H_

#include <string>
#include "ns3/Device.h"

namespace ns3 {

#define EV_TURN_ON 0
#define EV_TURN_OFF 1
#define EV_GO_AWAY 2
#define EV_SCHEDULE 3
#define EV_COMING_BACK 4

class DeviceEvent {

private:
	int tick;
	Device* device;
	int what;
	int data;

public:
	DeviceEvent(int tick, Device* device, int what) : tick(tick), device(device), what(what), data(-1) {};
	DeviceEvent(int tick, Device* device, int what, int data) : tick(tick), device(device), what(what), data(data) {};
	virtual ~DeviceEvent() {}

	int getTick() { return tick; }
	Device* getDevice() { return device; }
	int getWhat() { return what; }
	int getData() { return data; }

};

}

#endif
