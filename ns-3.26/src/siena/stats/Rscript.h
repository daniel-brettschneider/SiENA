#ifndef RSCRIPT_H_
#define RSCRIPT_H_

#include <string>
#include <cstdlib>
#include "ns3/Log.h"
#include "ns3/MyConfig.h"
#include "ns3/Helper.h"

namespace ns3 {

class Rscript {

public:
	Rscript() {}
	virtual ~Rscript() {}

	static void runRscripts() {
		std::vector<std::string> files = Helper::split(MyConfig::Get()->getString("graphs"), ',');
		for(uint i = 0; i < files.size(); i++) {
			runRscript(files[i]);
		}
	}

	static void runRscript(std::string filename) {
		Log::f("Rscript", "running rscript " + filename + "...");
		std::stringstream cmd;
		cmd << "Rscript --slave --vanilla data/graphs/" << filename << ".R "
				<< MyConfig::Get()->getString("identifier").c_str() << " "
				<< MyConfig::Get()->getInt("interval");
		int result = std::system(cmd.str().c_str());
		result++; // suppress warning
	}

};

}

#endif
