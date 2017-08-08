#include "StatsAnalyzer.h"

namespace ns3 {

const std::string StatsAnalyzer::TAG = "StatsAnalyzer";
const std::string StatsAnalyzer::DEPENDENCIES = "consumption,grouped,counters,heat,state";

void StatsAnalyzer::analyze() {
	Log::f("StatsAnalyzer", "stats:");

	MyConfig* config = MyConfig::Get();
	std::vector<std::string> saveIds = Helper::split(config->getString("keep_only_these_stats_in_ram"), ',');
	std::vector<std::string> exportIds = Helper::split(config->getString("keep_only_these_stats_in_ram"), ','); // TODO check
	bool filter = config->getBool("filter_stats");
	std::vector<std::string> deps = Helper::split(DEPENDENCIES, ',');
	for(uint i = 0; i < deps.size(); i++) {
		bool found = false;
		for(uint j = 0; j < saveIds.size(); j++) {
			if(deps[i] == saveIds[j]) {
				found = true;
				break;
			}
		}
		if(!found) {
			Log::f(TAG, "\tsome dependencies are missing, aborting (" + DEPENDENCIES + ")");
			return;
		}
	}



	int start = 0;
	int interval = config->getInt("interval");
	int duration = config->getInt("duration") / interval;
	double homes = config->getInt("total_homes");
	int startDay = config->getInt("quality_start");
	time_t startDate = Helper::parseDate(config->getString("start_date"));
	if(duration > (startDay - 1) * 1440 / interval) {
		start = (startDay - 1) * 1440 / interval;
		duration -= start;
	}

	// stats
	std::vector<std::string> csvHeaders;
	std::vector<std::vector<float>*> csvData;

	MyStats* stats = MyStats::Get();
	std::vector<std::vector<float>*>* data = stats->getStats();
	std::map<std::string, std::map<std::string, int>*>* dataIds = stats->getStatIds();
	std::vector<std::vector<float>*>* counters = stats->getCounters();
	std::map<std::string, int>* counterIds = stats->getCounterIds();
	std::vector<std::vector<float>*>* groupedConsumption = StatsExporter::Get()->getGroupedConsumption();

	// generated stats
	// table 1
	std::map<std::string, double> consumptionTypes;
	// table 2
	std::map<std::string, double> numberOfDevices;
	std::map<std::string, double> numberOfDevicesPercent;
	std::map<std::string, double> totalConsumptionOfDevices;
	std::map<std::string, double> consumptionOfDevices;
	std::map<std::string, double> feedOfDevices;
	std::map<std::string, double> consumptionOfDevicesPercent;
	std::map<std::string, double> feedOfDevicesPercent;
	std::map<std::string, double> activationsOfDevices;
	// table 3
	std::map<std::string, double> heatDemand;
	// table 4
	std::map<std::string, double> heatDeviceRuntimes;
	std::map<std::string, double> heatDeviceRuntimesMin;
	std::map<std::string, double> heatDeviceRuntimesMax;
	// table 5
	std::map<std::string, double> selfSuffE;
	std::map<std::string, double> selfSuffT;
	std::map<std::string, double> ownCons;
	//table 7
	std::map<std::string, double> communication;
	//table 8
	std::map<std::string, double> car;

	// consumption and number of devices
	std::map<std::string, int>* ids = (*dataIds)["consumption"];
	std::map<std::string, int>::iterator it;
	double consumption = 0, generation1 = 0;

	for(it = ids->begin(); it != ids->end(); ++it) {
		// sum up consumption
		double consumptionCounter = 0;
		double feedCounter = 0;
		std::vector<float>* consumptionData = (*data)[it->second];
		for(int i = start; i < start + duration; i++) {
			if((*consumptionData)[i] < 0)
				feedCounter -= (*consumptionData)[i];
			else
				consumptionCounter += (*consumptionData)[i];
		}
		double totalConsumption = consumptionCounter - feedCounter;
		consumption += kWhpy(consumptionCounter, duration, interval, homes);
		generation1 += kWhpy(feedCounter, duration, interval, homes);

		// stats
		std::string deviceType = Helper::getDeviceType(it->first);
		addValueToMap(&totalConsumptionOfDevices, deviceType, kWhpy(totalConsumption, duration, interval, homes));
		addValueToMap(&numberOfDevices, deviceType, 1);
		addValueToMap(&consumptionOfDevices, deviceType, kWhpy(consumptionCounter, duration, interval, homes));
		addValueToMap(&feedOfDevices, deviceType, kWhpy(feedCounter, duration, interval, homes));
	}

	// percents
	std::map<std::string, double>::iterator jt;
	for(jt = numberOfDevices.begin(); jt != numberOfDevices.end(); ++jt) {
		numberOfDevicesPercent.insert(std::pair<std::string, double>(jt->first, jt->second / homes * 100));
	}
	for(jt = consumptionOfDevices.begin(); jt != consumptionOfDevices.end(); ++jt) {
		consumptionOfDevicesPercent.insert(std::pair<std::string, double>(jt->first, jt->second / consumption * 100));
	}
	for(jt = feedOfDevices.begin(); jt != feedOfDevices.end(); ++jt) {
		feedOfDevicesPercent.insert(std::pair<std::string, double>(jt->first, jt->second / generation1 * 100));
	}

	// TODO web
	if(!filter || Helper::contains(&exportIds, "web")) {
		std::string filename = "stats_devices";
		if(!config->getString("identifier").empty())
			filename = MyConfig::Get()->getString("identifier") + "_" + filename;
		std::ofstream file(("results/" + filename + ".csv").c_str());
		if(file) {
			// headers
			file << "device,amount,amountP,con,conP,feed,feedP,total\n";
			// data
			for(jt = numberOfDevices.begin(); jt != numberOfDevices.end(); ++jt) {
				std::string type = Helper::getDeviceType(jt->first);
				if(type.compare("tank") != 0) {
					file << type << ","
						<< numberOfDevices[type] << ","
						<< numberOfDevicesPercent[type] << ","
						<< consumptionOfDevices[type] << ","
						<< consumptionOfDevicesPercent[type] << ","
						<< feedOfDevices[type] << ","
						<< feedOfDevicesPercent[type] << ","
						<< totalConsumptionOfDevices[type] << "\n";
				}
			}
			file.close();
			Log::f("StatsExporter", "\tfile " + filename + " written");
		} else
			throw("could not create file " + filename);
	}

	// consumption types
	double adaptable = 0, switchable = 0, base = 0, movable = 0, other[5] = {0}, total = 0, sum_own_con_pv[5] = {0}, self_consumption[5] = {0}, grid_consumption[5] = {0};
	int count_feed[5] = {0}, count_consumption[5] = {0}, season = 0;
	long time;
	double max_power = -9999999;
	double min_power = 9999999;
	for(int i = start; i < start + duration; i++) {
		time = startDate + i * interval * 60;
		//std::cout << time << std::endl;
		if((time >= 1292886000 && time < 1300662000) || (time >= 1324422000 && time < 1332284400) || (time >= 1356044400  && time < 1363820400) || (time >= 1387580400 && time < 1395356400) || (time >= 1419116400 && time < 1426892400)) //winter
			season = 0;
		else if((time >= 1300662000 && time < 1308610800) || (time >= 1332284400 && time < 1340233200) || (time >= 1363820400  && time < 1371769200) || (time >= 1395356400 && time < 1403305200) || (time >= 1426892400 && time < 1434841200)) //spring
			season = 1;
		else if((time >= 1308610800 && time < 1316559600) || (time >= 1340233200 && time < 1348182000) || (time >= 1371769200  && time < 1379718000) || (time >= 1403305200 && time < 1411254000) || (time >= 1434841200 && time < 1442790000)) //summer
			season = 2;
		else if((time >= 1316559600 && time < 1324422000) || (time >= 1348182000 && time < 1356044400) || (time >= 1379718000  && time < 1387580400) || (time >= 1411254000 && time < 1419116400) || (time >= 1442790000 && time < 1450652400)) //autumn
			season = 3;
		double consumption_temp = 0, generation_temp = 0;
		for(it = ids->begin(); it != ids->end(); ++it) {
			std::vector<float>* consumptionData = (*data)[it->second];
			if((*consumptionData)[i] >= 0)
				consumption_temp += (*consumptionData)[i];
			else
				generation_temp -= (*consumptionData)[i];
		}
		if (generation_temp >= consumption_temp) {
			self_consumption[season] += consumption_temp;
			self_consumption[4] += consumption_temp;
		} else {
			self_consumption[season] += generation_temp;
			self_consumption[4] += generation_temp;
			grid_consumption[season] += (consumption_temp - generation_temp);
			grid_consumption[4] += (consumption_temp - generation_temp);
		}

		double a = (*(*groupedConsumption)[0])[i]; //adaptable
		double b = (*(*groupedConsumption)[1])[i]; //baseload
		double m = (*(*groupedConsumption)[2])[i]; //movable
		double os = (*(*groupedConsumption)[3])[i];//other (PV)
		double o4 = (*(*groupedConsumption)[3])[i];//
		double s = (*(*groupedConsumption)[4])[i];//shiftable
		adaptable += a;
		switchable += s;
		base += b;
		movable += m;
		other[season] += os;
		other[4] += o4;
		total = a + b + m + os + s;//TODO check +=
		if(total < min_power)
			min_power = total;
		if(total > max_power)
			max_power = total;
		if(total <= 1.0) {		//feed in, 1.0 due to flex-Algorithm tollerance
			count_feed[season] += interval;
			count_feed[4] += interval;
			if(a > 0) {
				if(-os < b + m + a) {//Einspeisung kleiner als verbrauch?
					sum_own_con_pv[season] += os;	//only PV
					sum_own_con_pv[4] += os;	//only PV
				} else {
					sum_own_con_pv[season] -= (a + b + s + m); //could leed to negative own_consumption due to chps
					sum_own_con_pv[4] -= (a + b + s + m);
				}
			} else {
				if(-os < b + m) {//Einspeisung kleiner als verbrauch?
					sum_own_con_pv[season] += os;	//only PV
					sum_own_con_pv[4] += os;	//only PV
				} else {
					sum_own_con_pv[season] -= (b + s + m); //could leed to negative own_consumption due to chps
					sum_own_con_pv[4] -= (b + s + m);
				}
			}
		} else {					// consumption
			count_consumption[season] += interval;
			count_consumption[4] += interval;
			sum_own_con_pv[season] += os;	//only PV
			sum_own_con_pv[4] += os;
		}
	}
	double self_suff_t[5] = {0};
	double self_suff_e[5] = {0};
	double own_con_pv[5] = {0};
	for (int i = 0; i < 5; i++){
		self_suff_t[i] = 100 * ((double)count_feed[i]) / (count_consumption[i] + count_feed[i]);
		self_suff_e[i] = 100 * self_consumption[i] / (self_consumption[i] + grid_consumption[i]);
		own_con_pv[i] = 100 * sum_own_con_pv[i] / other[i];
		/*if (self_suff_t[i] >= 0)
			self_suff_t[4] = self_suff_t[4] + self_suff_t[i];
		if (self_suff_e[i] >= 0)
			self_suff_e[4] = self_suff_e[4] + self_suff_e[i];
		if (own_con_pv[i] >= 0)
			own_con_pv[4] = own_con_pv[4] + own_con_pv[i];
		if (other[i] < 0 || other[i] > 0)
			other[4] = other[4] + other[i];*/
	}

	double totalConsumption = adaptable + switchable + base + movable + other[4];
	addValueToMap(&consumptionTypes, "adaptable", kWhpy(adaptable, duration, interval, 1));
	addValueToMap(&consumptionTypes, "base", kWhpy(base, duration, interval, 1));
	addValueToMap(&consumptionTypes, "movable", kWhpy(movable, duration, interval, 1));
	addValueToMap(&consumptionTypes, "other", kWhpy(other[4], duration, interval, 1));
	addValueToMap(&consumptionTypes, "switchable", kWhpy(switchable, duration, interval, 1));
	addValueToMap(&consumptionTypes, "total", kWhpy(totalConsumption, duration, interval, 1));
	addValueToMap(&consumptionTypes, "total consumption", consumption);
	addValueToMap(&consumptionTypes, "total generation", generation1);

	if(!filter || Helper::contains(&exportIds, "web")) {
		std::string filename = "stats_con";
		if(!config->getString("identifier").empty())
			filename = MyConfig::Get()->getString("identifier") + "_" + filename;
		std::ofstream file2(("results/" + filename + ".csv").c_str());
		if(file2) {
			// headers
			file2 << "type,value\n";
			// data
			file2 << "adaptable," << kWhpy(adaptable, duration, interval, 1) << "\n";
			file2 << "base," << kWhpy(base, duration, interval, 1) << "\n";
			file2 << "movable," << kWhpy(movable, duration, interval, 1) << "\n";
			file2 << "pv," << kWhpy(other[4], duration, interval, 1) << "\n";
			file2 << "switchable," << kWhpy(switchable, duration, interval, 1) << "\n";
			file2 << "total," << kWhpy(totalConsumption, duration, interval, 1) << "\n";
			file2 << "total consumption," << consumption << "\n";
			file2 << "total generation," << generation1 << "\n";
			file2.close();
			Log::f("StatsExporter", "\tfile " + filename + " written");
		} else
			throw("could not create file " + filename);
	}

	std::string sso_names[] = {"winter", "spring", "summer" ,"autumn", "total"};
	for (int i = 0; i < 5; i++) {
		//std::cout << self_suff_t[i] << std::endl;
		//std::cout << self_suff_e[i] << std::endl;
		//std::cout << own_con_pv[i] << std::endl;
		addValueToMap(&selfSuffE, sso_names[i], self_suff_e[i]);
		addValueToMap(&selfSuffT, sso_names[i], self_suff_t[i]);
		addValueToMap(&ownCons, sso_names[i], own_con_pv[i]);
	}
	//addValueToMap(&selfSuff_OwnCon, "selfSuff_time", self_suff_t[4]);
	//addValueToMap(&selfSuff_OwnCon, "selfSuff_energy", self_suff_e[4]);
	//addValueToMap(&selfSuff_OwnCon, "own_con_PV", own_con_pv[4]);

	// heat
	double demand = -1; // TODO web
	if(dataIds->find("heat") != dataIds->end()) {
		ids = (*dataIds)["heat"];
		double unsatisfied = 0;
		for(it = ids->begin(); it != ids->end(); ++it) {
			std::string type = Helper::getDeviceType(it->first);
			bool isDemand = type == "demand";
			bool isUnsatisfied = type == "unsatisfied";
			std::vector<float>* heat = (*data)[it->second];
			for(int i = start; i < start + duration; i++) {
				if(isDemand)
					demand += (*heat)[i];
				else if(isUnsatisfied)
					unsatisfied += (*heat)[i];
			}
		}
		addValueToMap(&heatDemand, "demand", kWhpy(demand / interval, duration, interval, homes));
		//saveCSV(&csvHeaders, &csvData, "demand_heat", kWhpy(unsatisfied, duration, interval, homes));
		addValueToMap(&heatDemand, "unsatisfied", kWhpy(unsatisfied / interval, duration, interval, homes));
		//saveCSV(&csvHeaders, &csvData, "demand_heat_unsatisfied", kWhpy(unsatisfied, duration, interval, homes));

		// heat device runtimes
		std::string heatDevices[] = {"heatpump", "chps", "heatingrod"};
		ids = (*dataIds)["state"];
		for(it = ids->begin(); it != ids->end(); ++it) {
			std::string deviceType = Helper::getDeviceType(it->first);
			if(Helper::contains(heatDevices, deviceType, 3)) {
				int amount = numberOfDevices[deviceType];
				double runtime = 0;
				for(int i = start; i < start + duration; i++) {
					if((*(*data)[it->second])[i] == 1)
						runtime++;
				}
				runtime = runtime / duration * 8760;
				addValueToMap(&heatDeviceRuntimes, deviceType, runtime / amount);
				if(heatDeviceRuntimesMin.insert(std::pair<std::string, double>(deviceType, runtime)).second == false) {
					if(runtime < heatDeviceRuntimesMin[deviceType])
						heatDeviceRuntimesMin[deviceType] = runtime;
				}
				if(heatDeviceRuntimesMax.insert(std::pair<std::string, double>(deviceType, runtime)).second == false) {
					if(runtime > heatDeviceRuntimesMax[deviceType])
						heatDeviceRuntimesMax[deviceType] = runtime;
				}
			}
		}
	}

	if(!filter || Helper::contains(&exportIds, "web")) {
		std::string filename = "stats_autarchy";
		if(!config->getString("identifier").empty())
			filename = MyConfig::Get()->getString("identifier") + "_" + filename;
		std::ofstream file3(("results/" + filename + ".csv").c_str());
		if(file3) {
			file3 << "energy,time,owncon" << (demand != -1 ? ",demand\n" : "\n")
				<< self_suff_e[4] << "," << self_suff_t[4] << "," << own_con_pv[4];
			if(demand != -1)
				file3 << "," << kWhpy(demand / interval, duration, interval, homes);
			file3 << "\n";
			file3.close();
			Log::f("StatsExporter", "\tfile " + filename + " written");
		} else
			throw("could not write file " + filename);
	}

	// activations
	for(it = counterIds->begin(); it != counterIds->end(); ++it) {
		if(Helper::contains(it->first, '_', "activation"))
			addValueToMap(&activationsOfDevices, Helper::getDeviceType(it->first), (int) (*(*counters)[it->second])[0]);
	}


	if(dataIds->find("msgs") != dataIds->end()) {
		ids = (*dataIds)["msgs"];
		double sum = 0;
		int duration = (*data)[ids->begin()->second]->size();
		for(it = ids->begin(); it != ids->end(); ++it) {
			std::vector<float>* tmp = (*data)[it->second];
			for(uint i = 0; i < tmp->size(); i++) {//TODO negative Werte raus filtern
				if (((*tmp)[i]) > 0)
					sum += (*tmp)[i];
			}
		}
		addValueToMap(&communication, "messages_avg", sum/duration);
	} else
		std::cout << "no messages found" << std::endl;

	if(dataIds->find("time") != dataIds->end()) {
		ids = (*dataIds)["time"];
		double sum = 0;
		double sumconverged = 0;
		int duration = (*data)[ids->begin()->second]->size();
		for(it = ids->begin(); it != ids->end(); ++it) {
			std::vector<float>* tmp = (*data)[it->second];
			for(uint i = 0; i < tmp->size(); i++) {
				sum += (*tmp)[i];
				if((*tmp)[i] < MyConfig::Get()->getInt("interval") * 60000 - 5000 && (*tmp)[i] > 0) {
					sumconverged++;
				} else
					std::cout << "not converged!!! time: " << (*tmp)[i] << std::endl;
			}
		}
		addValueToMap(&communication, "time_avg", sum/(duration*1000)/config->getInt("clusters"));
		addValueToMap(&communication, "converged", (sumconverged*100)/duration);
	} else
		std::cout << "no time and convergation data found" << std::endl;


	// print tables and save stats to csv
	std::map<std::string, double>::iterator it2;
	// table 1
	std::string tableHeaders1[] = {"load_kWh/y"};
	std::map<std::string, double>* tableData1[] = {&consumptionTypes};
	printTable("consumption types", tableHeaders1, tableData1, 1);
	saveCSV(&csvHeaders, &csvData, tableHeaders1, tableData1, 1);

	// table 2
	std::string tableHeaders2[] = {"number_total", "number_%", "activations", "tot_con_kWh/y", "con_kWh/y", "con_%", "feedIn_kWh/y", "feedIn_%"};
	std::map<std::string, double>* tableData2[] = {&numberOfDevices, &numberOfDevicesPercent, &activationsOfDevices, &totalConsumptionOfDevices,
			&consumptionOfDevices, &consumptionOfDevicesPercent, &feedOfDevices, &feedOfDevicesPercent};
	printTable("device stats", tableHeaders2, tableData2, 8);
	saveCSV(&csvHeaders, &csvData, tableHeaders2, tableData2, 8);

	// table 3
	std::string tableHeaders3[] = {"heat_kWh/y"};
	std::map<std::string, double>* tableData3[] = {&heatDemand};
	printTable("heat demand", tableHeaders3, tableData3, 1);
	saveCSV(&csvHeaders, &csvData, tableHeaders3, tableData3, 1);

	// table 4
	std::string tableHeaders4[] = {"runtime_h/a", "min_h/a", "max_h/a"};
	std::map<std::string, double>* tableData4[] = {&heatDeviceRuntimes, &heatDeviceRuntimesMin, &heatDeviceRuntimesMax};
	printTable("heat device runtimes", tableHeaders4, tableData4, 3);
	saveCSV(&csvHeaders, &csvData, tableHeaders4, tableData4, 3);

	// table 5
	//std::string tableHeaders5[] = {"winter", "spring", "summer" ,"autumn", "total"};
	std::string tableHeaders5[] = {"selfSuff_energy", "selfSuff_time", "ownConsumption"};
	std::map<std::string, double>* tableData5[] = {&selfSuffE, &selfSuffT, &ownCons};
	printTable("self-suff / own consumption", tableHeaders5, tableData5, 3);
	saveCSV(&csvHeaders, &csvData, tableHeaders5, tableData5, 3);

	// table 6
	std::string tableHeaders6[] = {"Quality"};
	//std::string valueNames[] = {"PC1", "PC2", "VF1", "VF2", "EE", "LG", "LS1", "LS2", "LS3"};
	std::string valueNames[] = {"PC", "VF", "EE", "LG", "LSp", "LSn", "LS"};
	float* values = analyzeQuality(groupedConsumption);
	if(values != NULL) {
		std::map<std::string, double> valueMap;
		for (uint i = 0; i < 7; i++) {
			valueMap.insert(std::pair<std::string, double>(valueNames[i], values[i]));
		}
		std::map<std::string, double>* tableData6[] = {&valueMap};
		printTable("DSM-Quality", tableHeaders6, tableData6, 1, 3);
		saveCSV(&csvHeaders, &csvData, tableHeaders6, tableData6, 1);
		delete [] values;
	}

	// table 7 (communication)
	std::string tableHeaders7[] = {"Communication"};
	std::map<std::string, double>* tableData7[] = {&communication};
	printTable("communication", tableHeaders7, tableData7, 1);
	saveCSV(&csvHeaders, &csvData, tableHeaders7, tableData7, 1);

	// table 8 (car rides)
	int numRides = 0;
	int numFailedRides = 0;
	std::vector<ConventionalHome*>* homesContainer = HomeContainer::Get()->getHomes();
	for(uint i = 0; i < homesContainer->size(); i++) {
		std::map<std::string, Device*>* devices = (*homesContainer)[i]->getDevices();
		std::map<std::string, Device*>::iterator it;
		for(it = devices->begin(); it != devices->end(); ++it) {
			if(Helper::getDeviceType(it->second->getId()) == "car") {
				Car* c = (Car*) it->second;
				numRides += c->getNumRides();
				numFailedRides += c->getNumFailedRides();
			}
		}
	}
	addValueToMap(&car, "rides", numRides);
	addValueToMap(&car, "failed rides", numFailedRides);
	addValueToMap(&car, "successful", (1. - ((double) numFailedRides / (double) numRides)) * 100.);
	std::string tableHeaders8[] = {"Car Rides"};
	std::map<std::string, double>* tableData8[] = {&car};
	printTable("car rides", tableHeaders8, tableData8, 1);
	saveCSV(&csvHeaders, &csvData, tableHeaders8, tableData8, 1);

	// extra stats in stats.csv
	saveCSV(&csvHeaders, &csvData, "homes_count", homes);
	saveCSV(&csvHeaders, &csvData, "max_power", max_power);
	saveCSV(&csvHeaders, &csvData, "min_power", min_power);
	saveCSV(&csvHeaders, &csvData, "seed", config->getInt("seed"));


	std::vector<std::string> tmp = Helper::split(config->getString("save_only_these_stats"), ',');
	if(!config->getBool("filter_stats") || Helper::contains(&tmp, "stats"))
		MyStats::Get()->exportCsv("stats", &csvHeaders, &csvData);
	for(uint i = 0; i < csvData.size(); i++) {
		delete csvData[i];
	}
}

void StatsAnalyzer::addValueToMap(std::map<std::string, double>* map, std::string key, double value) {
	if(map->insert(std::pair<std::string, double>(key, value)).second == false)
		(*map)[key] += value;
}

void StatsAnalyzer::printTable(std::string name, std::string* headers, std::map<std::string, double>** data, int n, int decimalPlaces /* = 1*/) {
	Log::f(TAG, name + ":");
	// headers
	std::cout << std::setw(20) << " ";
	for(int i = 0; i < n; i++) {
		std::cout << " | " << std::setw(15) << headers[i];
	}
	std::cout << std::endl;
	std::cout << std::string(20 + n * 15 + n * 3, '-') << std::endl;
	// data
	std::map<std::string, double>::iterator it;
	for(it = data[0]->begin(); it != data[0]->end(); ++it) {
		std::cout << std::setw(20) << it->first;
		for(int i = 0; i < n; i++) {
			std::cout << " | " << std::setw(15) << std::fixed << std::setprecision(decimalPlaces) << (*data[i])[it->first];
		}
		std::cout << std::endl;
	}
}

double StatsAnalyzer::kWhpy(double value, double duration, double interval, double amount) {
	return value / duration * 525600 / 60000 / amount; //Intervall is already in the duration
}

void StatsAnalyzer::saveCSV(std::vector<std::string>* csvHeaders, std::vector<std::vector<float>*>* csvData, std::string* headers,
		std::map<std::string, double>** data, int n) {
	std::map<std::string, double>::iterator it;
	for(it = data[0]->begin(); it != data[0]->end(); ++it) {
		for(int i = 0; i < n; i++) {
			saveCSV(csvHeaders, csvData, it->first + "_" + headers[i], (*data[i])[it->first]);
		}
	}
}

void StatsAnalyzer::saveCSV(std::vector<std::string>* csvHeaders, std::vector<std::vector<float>*>* csvData, std::string name, double value) {
	std::vector<float>* dataTemp = new std::vector<float>;
	csvHeaders->push_back(name);
	dataTemp->push_back((float)value);
	csvData->push_back(dataTemp);
}

}
