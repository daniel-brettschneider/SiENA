#ifndef SIENASIMULATOR_H_
#define SIENASIMULATOR_H_

#include <string>
#include <vector>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/Log.h"
#include "ns3/MyConfig.h"
#include "ns3/ConventionalHome.h"
#include "ns3/SmartHome.h"
#include "ns3/Progress.h"
#include "ns3/Modules.h"
#include "ns3/application-container.h"
#include "ns3/Turbo.h"
#include "ns3/Pandapower.h"

namespace ns3 {

class SiENASimulator {

private:
	static const std::string TAG;
	Progress* progress;
	Pandapower* pandapower;

public:
	SiENASimulator();
	virtual ~SiENASimulator();

};

}

#endif
