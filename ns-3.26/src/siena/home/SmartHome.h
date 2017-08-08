#ifndef SMARTHOME_H_
#define SMARTHOME_H_

#include <string>
#include "ns3/ConventionalHome.h"
#include "ns3/Modules.h"

namespace ns3 {

class SmartHome: public ConventionalHome {

private:
	std::string moduleBattery;
	std::string moduleCar;
	std::string moduleChps;
	std::string moduleHeatpump;
	std::string moduleShift;

public:
	SmartHome(std::string id) : ConventionalHome(id) {
		moduleBattery = c->getString("smarthome_battery");
		moduleCar = c->getString("smarthome_car");
		moduleChps = c->getString("smarthome_chps");
		moduleHeatpump = c->getString("smarthome_heatpump");
		moduleShift = c->getString("smarthome_shift_device");
	}
	virtual ~SmartHome() {}
	virtual void tick();

protected:
	virtual void schedule(DeviceEvent* event);

};

}

#endif
