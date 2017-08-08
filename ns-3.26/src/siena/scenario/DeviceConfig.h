#ifndef DEVICECONFIG_H_
#define DEVICECONFIG_H_

#include <string>

namespace ns3 {

class DeviceConfig {

private:
	std::string type;
	int seed;

public:
	DeviceConfig(std::string type, int seed) : type(type), seed(seed) {}
	virtual ~DeviceConfig() {}

	std::string getType() { return this->type; }
	int getSeed() { return this->seed; }

};

}

#endif
