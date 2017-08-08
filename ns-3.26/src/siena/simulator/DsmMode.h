#ifndef NS_3_22_SRC_SIENA_SIMULATOR_DSMMODE_H_
#define NS_3_22_SRC_SIENA_SIMULATOR_DSMMODE_H_

#include "ns3/MyConfig.h"
#include "ns3/DataBasis.h"
#include "ns3/MySingleton.h"
#include <vector>
#include <string>

namespace ns3 {

#define DSM_Peak_Clipping 0
#define DSM_Load_Shaping 1
#define DSM_Undefinded 2
#define DSM_Own_Consumption 3

class DsmMode : public MySingleton<DsmMode> {
	friend class MySingleton<DsmMode>;

private:
	bool overwritten;
	std::vector<double>* goal;

public:
	virtual ~DsmMode() {
		if(overwritten)
			delete goal;
	}

	static int get() {
		std::string s = MyConfig::Get()->getString("dsm_mode");
		if(s == "peak_clipping")
			return DSM_Peak_Clipping;
		else if(s == "load_shaping")
			return DSM_Load_Shaping;
		else if(s == "own_consumption")
			return DSM_Own_Consumption;
		else
			return DSM_Undefinded;
	}

	std::vector<double>* getQualityGoal() { return goal; }

private:
	DsmMode() : overwritten(false) {
		MyConfig* config = MyConfig::Get();
		if(config->getBool("overwrite_quality_goal")) {
			overwritten = true;
			double tmp = config->getDouble("quality_goal");
			goal = new std::vector<double>(1440 / Tick::Get()->getInterval(), tmp);
		} else
			goal = (*DataBasis::Get()->get("dsm/quality_goal", "csv", CSV_PER_COLUMN, true, 1, 1440))[0];
	}

};

}

#endif
