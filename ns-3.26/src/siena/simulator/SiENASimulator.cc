#include "SiENASimulator.h"

namespace ns3 {

const std::string SiENASimulator::TAG = "Simulator";

SiENASimulator::SiENASimulator() {
	#ifdef TURBO
		std::cerr << "turbo mode activated" << std::endl << "a lot of log messages will be suppressed" << std::endl
				<< "see src/simulator/Turbo.h to disable turbo mode" << std::endl;
	#endif

	// ns3 fix
	Config::SetDefault("ns3::ArpCache::PendingQueueSize", UintegerValue(1000));
	Config::SetDefault ("ns3::PointToPointNetDevice::Mtu", UintegerValue(9000));

	MyConfig* c = MyConfig::Get();

	// init times
	Tick::Get();

	//c->print();

	// init modules
	Modules::Get()->execute(c->getString("init_modules"), NULL);

	// stuff
	progress = new Progress;

	// init homes and network
	ApplicationContainer apps;
	std::vector<void*> params;
	params.push_back(&apps);
	Log::f("Simulator", "creating scenario, network and homes...");
	Scenario::Get()->init();
	Modules::Get()->execute(c->getString("com_net"), &params);

	// neplan
	if(c->getBool("simulate_power")) {
		pandapower = new Pandapower;
		pandapower->start();
	} else
		pandapower = NULL;

	apps.Start(Seconds(0.1)); //100 Miliseconds due to LTE Start (synchronised to PowerMatcher)

	Modules::Get()->execute(c->getString("init_scheduler"), NULL);
}

SiENASimulator::~SiENASimulator() {
	delete progress;
	if(pandapower != NULL)
		delete pandapower;
}

}
