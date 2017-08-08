#ifndef HEATHELPER_H_
#define HEATHELPER_H_

#include "ns3/MySingleton.h"
#include "ns3/DataBasis.h"
#include "ns3/MyConfig.h"
#include "ns3/Random.h"
#include "ns3/Scenario.h"
#include "ns3/Helper.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

namespace ns3 {

class HeatHelper : public MySingleton<HeatHelper> {
	friend class MySingleton<HeatHelper>;

private:
	DataBasis* data;
	MyConfig* config;
	Random* random;
	std::string classes[10];

public:
	virtual ~HeatHelper() {}

	void generateHeatDemand(std::vector<double>* demand, int unit, int residents, double heatEfficiency, int space) {
		int interval = config->getInt("interval");
		int start = config->getInt("start") / 1440;
		int duration = ceil((double) config->getInt("duration") / 1440.);

		std::string unitS = "_MFH";
		double Qtww = 1000;	// kWh/year
		int nPers = 1;
		if(unit == CONFIG_UNIT_SINGLE) {
			unitS = "_EFH";
			Qtww = residents * 500; // kWh/year
			nPers = residents;
		}
		double Qheiz = heatEfficiency * space;	// kWh/year

		std::vector<std::string> shiftTmp = Helper::split(config->getString("heatdemand_shift"), ',');
		int shift = random->get(atoi(shiftTmp[0].c_str()), atoi(shiftTmp[1].c_str())) / interval;

		std::string dayClass;
		std::string name;
		if(shift > 0) {	// >>
			for(int i = 0; i < shift; i++) {
				demand->push_back(0);
			}
		}

		for(int day = start; day < start + duration; day++) {
			dayClass = classes[(int) (*(*data->get("heat/Classification"))[0])[day] - 1];

			name = "F_W_" + dayClass + unitS;
			double Qheiztt = Qheiz * (*(*data->get("heat/Factors_" + name))[0])[0]; // kWh/year
			name = "F_WW_" + dayClass + unitS;
			double Qtwwtt = Qtww * (1. / 365. + nPers * (*(*data->get("heat/Factors_" + name))[0])[0]); // kWh/year
			if(Qtwwtt < 0)
				Qtwwtt = Qtww / 365.;

			name = "W_" + dayClass + unitS;
			std::vector<double>* heatData = (*data->get("heat/VDI4655_" + name))[0];
			name = "WW_" + dayClass + unitS;
			std::vector<double>* waterData = (*data->get("heat/VDI4655_" + name))[0];

			//std::cou << "Generate heat demand for " << residents << " residents" << std::endl;
			// create demand for whole day (ignore if sim ends during day)
			double sum = 0;
			for(int i = 0; i < 1440; i++) {
				if(i > 0 && i % interval == 0) {
					demand->push_back(sum);
					sum = 0;
				}
				double QheizttT = (*heatData)[i] * Qheiztt; // kWh/year
				double QtwwttT = (*waterData)[i] * Qtwwtt;
				sum += (QheizttT + QtwwttT) * 1000; // Wh
			}
			demand->push_back(sum);
		}

		if(shift < 0) { // <<
			for(uint i = 0; i < (uint) -shift; i++) {
				demand->push_back(0);
			}
		}

		Log::i("HeatHelper", "\theat demand, shift: ", shift);
	}

private:
	HeatHelper() {
		data = DataBasis::Get();
		config = MyConfig::Get();
		random = Random::Get();
		classes[0] = "UWH";
		classes[1] = "UWB";
		classes[2] = "USH";
		classes[3] = "USB";
		classes[4] = "SWX";
		classes[5] = "SSX";
		classes[6] = "WWH";
		classes[7] = "WWB";
		classes[8] = "WSH";
		classes[9] = "WSB";

		// preload files
		data->get("heat/VDI4655", "csv", CSV_PER_COLUMN_SPLIT, false, 40, 1440);
		data->get("heat/Factors", "csv", CSV_PER_COLUMN_SPLIT, false, 40, 1);
		data->get("heat/Classification", "csv", CSV_PER_COLUMN, false);
	}

};

}

#endif
