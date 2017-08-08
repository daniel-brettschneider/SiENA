#include "Pandapower.h"

namespace ns3 {

Pandapower::Pandapower() {
	MyConfig* c = MyConfig::Get();
	interval = c->getInt("interval") * int64_t(60000);
	duration = c->getInt("duration") * int64_t(60000);
	homeContainer = HomeContainer::Get();
	db = DataBasis::Get();
	streetSize = c->getInt("pp_street_size");
	net = c->getString("pp_net");
	python = c->getString("pp_python");
	stats = MyStats::Get();
}

void Pandapower::start() {
	Simulator::Schedule(MilliSeconds(interval - 1), &Pandapower::tick, this);
}

void Pandapower::tick() {
	Log::i("Pandapower", "tick");

	// write loads to file
	Log::i("pandapower", "writing loads file");
	std::vector<ConventionalHome*>* homes = homeContainer->getHomes();
	std::stringstream ss;
	for(uint i = 0; i < homes->size(); i++) {
		if(i > 0)
			ss << "\n";
		ss << (*homes)[i]->getConsumption();
	}
	std::ofstream out("data/pandapower/loads.tmp");
	out << ss.str();
	out.close();

	// run pandapower
	Log::i("pandapower", "running power flow");
	std::stringstream cmd;
	cmd << python << " data/pandapower/" << net << ".py " << streetSize << " data/pandapower/loads.tmp";
	FILE *in = popen(cmd.str().c_str(), "r");
    char buff[1024];
    if(!in)
    	throw("could not start pandapower");
    std::stringstream result;
    while(fgets(buff, sizeof(buff), in) != NULL){
    	result << buff;
    }
    pclose(in);

    // parse result
	Log::i("pandapower", "saving stats");
	std::string line;
	while(std::getline(result, line, '\n')) {
		std::vector<std::string> tokens = Helper::split(line, ' ');
		std::map<std::string, int>::iterator idsIt = statsIds.find(tokens[0]);
		if(idsIt == statsIds.end())
			idsIt = statsIds.insert(std::pair<std::string, int>(tokens[0], stats->registerStat("powergrid", tokens[0]))).first;
		stats->addStat(idsIt->second, atof(tokens[1].c_str()));
	}

	// delete loads file
	std::remove("data/pandapower/loads.tmp");

	if(Simulator::Now().GetMilliSeconds() + interval < duration)
		Simulator::Schedule(MilliSeconds(interval), &Pandapower::tick, this);
}

}
