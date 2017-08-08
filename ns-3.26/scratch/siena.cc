#include "ns3/core-module.h"
#include "ns3/siena-module.h"
#include <string>
#include <ctime>
#include "ns3/MyStats.h"
#include "ns3/StatsExporter.h"
#include "ns3/StatsAnalyzer.h"
#include "ns3/Rscript.h"
#include "ns3/MyConfig.h"
#include "ns3/Tick.h"
#include "ns3/Registry.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/TraceHelper.h"
#include "ns3/plc-defs.h"

using namespace ns3;

int main(int argc, char* argv[]) {
	try {
		time_t start = time(NULL);

		Config::SetDefault("ns3::ArpCache::PendingQueueSize", UintegerValue(1000));
		Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (1024 * 100));
//		CommandLine cmd;
//		cmd.Parse(argc, argv);


//		LogComponentEnableAll(LOG_PREFIX_ALL);
//		LogComponentEnableAll(LOG_LEVEL_ALL);
//		LogComponentEnable("PLC_Mac", LOG_LEVEL_ALL);
//		LogComponentEnable("PLC_Phy", LOG_LEVEL_ALL);

		Packet::EnablePrinting(); // drin lassen!

		Time::SetResolution(Time::NS);
		MyConfig* config = MyConfig::Get();
		config->init(argc, argv);
		Random::Get()->init(config->getInt("seed"));

//		DataBasis::Get()->convertFilesToBinary();

		registerModules();

		SiENASimulator sim;
		time_t initEnd = time(NULL);

//		PointToPointHelper p2p;
//		p2p.EnablePcapAll("test");
//		FlowMonitorHelper flowmon;
//		Ptr<FlowMonitor> monitor = flowmon.InstallAll();

		Log::f("Simulator", "starting simulation...");
		Simulator::Run();

//		monitor->CheckForLostPackets ();
//		std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
//		std::map<FlowId, FlowMonitor::FlowStats>::iterator it;
//		double rx = 0;
//		for(it = stats.begin(); it != stats.end(); ++it) {
//			rx += it->second.rxBytes;
//		}
//		double throughput = (double) rx / stats.rbegin()->second.timeLastRxPacket.GetSeconds() / 1024.0;
//		std::cout << "sent: " << rx / 1024 / 1024 << " MB" << std::endl;
//		std::cout << "avg throughput: " << throughput << " kB/s" << std::endl;
//		std::cout << rx << std::endl;

		TraceHelper::Get()->print();

		Modules::Get()->execute(config->getString("tidy_up"), NULL);

		Simulator::Destroy();
		time_t simEnd = time(NULL);

		MyStats::Get()->exportCsv();
		StatsExporter::Get()->exportAdditionalStatsToCsv();
		if(config->getBool("analyze"))
			StatsAnalyzer::analyze();
		time_t statsEnd = time(NULL);

		Log::f("Simulator", "creating graphs...");
		Rscript::runRscripts();

		time_t graphsEnd = time(NULL);

		double initTime = difftime(initEnd, start);
		double simTime = difftime(simEnd, initEnd);
		double statsTime = difftime(statsEnd, simEnd);
		double graphsTime = difftime(graphsEnd, statsEnd);
		double totalTime = difftime(graphsEnd, start);
		std::cout << std::endl << "simulation finished in " << Tick::getTime(totalTime)
			<< " (init: " << Tick::getTime(initTime)
			<< ", simulation: " << Tick::getTime(simTime)
			<< ", stats: " << Tick::getTime(statsTime)
			<< ", graphs: " << Tick::getTime(graphsTime) << ")" << std::endl;
	} catch(const std::string& s) {
		std::cerr << s << std::endl;
	} catch(const char* s) {
		std::cerr << s << std::endl;
	}
	return 0;
}
