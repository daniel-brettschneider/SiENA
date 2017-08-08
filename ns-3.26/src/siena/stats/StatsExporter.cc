#include "../../siena/stats/StatsExporter.h"

namespace ns3 {

void StatsExporter::exportAdditionalStatsToCsv() {
	bool filter = config->getBool("filter_stats");
	std::vector<std::string> saveIds = Helper::split(config->getString("save_only_these_stats"), ',');
	std::vector<std::string> ramIds = Helper::split(config->getString("keep_only_these_stats_in_ram"), ',');
	if(!filter || Helper::contains(&ramIds, "grouped"))
		createGroupedConsumption();
	if(!filter || Helper::contains(&saveIds, "grouped")) {
		std::vector<std::string> headers;
		headers.push_back("adaptable");
		headers.push_back("base");
		headers.push_back("movable");
		headers.push_back("other");
		headers.push_back("switchable");
		stats->exportCsv("consumption_grouped", &headers, &grouped);
	}

	if(!filter || Helper::contains(&saveIds, "configs"))
		exportDeviceConfigs();

	if(!filter || Helper::contains(&saveIds, "ips"))
		exportIps();

	if(Helper::contains(&saveIds, "web"))
		exportWeb();
}

void StatsExporter::createGroupedConsumption() {
	Log::f("StatsExporter", "\tcreating and exporting grouped consumption...");

	// get devices
	std::vector<std::string> movDev = Helper::split(MyConfig::Get()->getString("movable_devices"), ',');
	std::vector<std::string> adaptDev = Helper::split(MyConfig::Get()->getString("adaptable_devices"), ',');
	std::vector<std::string> otherDev = Helper::split(MyConfig::Get()->getString("other_devices"), ',');
	std::vector<std::string> switchDev = Helper::split(MyConfig::Get()->getString("switchable_devices"), ',');

	// create data
	int duration = config->getInt("duration") / MyConfig::Get()->getInt("interval");
	for(int i = 0; i < 5; i++) {
		grouped.push_back(new std::vector<float>(duration, 0));
	}

	// add up data
	std::map<std::string, int>* ids = (*stats->getStatIds())[STAT_CONSUMPTION];
	std::vector<std::vector<float>*>* statsData = stats->getStats();
	std::map<std::string, int>::iterator it;
	double homes = config->getDouble("total_homes");
	for(it = ids->begin(); it != ids->end(); ++it) {
		std::vector<float>* tmp = grouped[getType(it->first, &movDev, &adaptDev, &otherDev, &switchDev)];
		std::vector<float>* add = (*statsData)[it->second];
		for(int i = 0; i < duration; i++) {
			(*tmp)[i] += (*add)[i] / homes;
		}
	}
}

int StatsExporter::getType(std::string id, std::vector<std::string>* mov, std::vector<std::string>* adapt, std::vector<std::string>* other, std::vector<std::string>* switchable) {
	std::string dev = Helper::getDeviceType(id);
	for(uint i = 0; i < mov->size(); i++) {
		if(dev == (*mov)[i])
			return 2;
	}
	for(uint i = 0; i < adapt->size(); i++) {
		if(dev == (*adapt)[i])
			return 0;
	}
	for(uint i = 0; i < other->size(); i++) {
		if(dev == (*other)[i])
			return 3;
	}
	for(uint i = 0; i < switchable->size(); i++) {
		if(dev == (*switchable)[i])
			return 4;
	}
	return 1;
}

void StatsExporter::exportDeviceConfigs() {
	std::vector<ConventionalHome*>* homes = HomeContainer::Get()->getHomes();
	std::string filename = "results/device_config.csv";
	if(!config->getString("identifier").empty())
		filename = "results/" + config->getString("identifier") + "_device_config.csv";
	std::ofstream file(filename.c_str());
	file << "device,config\n";
	for(uint i = 0; i < homes->size(); i++) {
		ConventionalHome* home = (*homes)[i];
		file << home->getId() << "," << home->getUnit() << "/" << home->getResidents() << "/" << home->getSpace() << "/" << home->getHeatEfficiency() << "\n";
		std::map<std::string, Device*>* devices = home->getDevices();
		std::map<std::string, Device*>::iterator it;
		for(it = devices->begin(); it != devices->end(); ++it) {
			file << it->first << "," << it->second->getConfigName() << "\n";
		}
	}
	Log::f("StatsExporter", "\tfile device configs exported");
}

void StatsExporter::exportIps() {
	std::vector<ConventionalHome*>* homes = HomeContainer::Get()->getHomes();
	std::string filename = "results/ips.csv";
	if(!config->getString("identifier").empty())
		filename = "results/" + config->getString("identifier") + "_ips.csv";
	std::ofstream file(filename.c_str());
	file << "node,ips\n";
	for(uint i = 0; i < homes->size(); i++) {
		ConventionalHome* home = (*homes)[i];
		GridHome* gridHome = dynamic_cast<GridHome*>(home);
		if(gridHome != NULL) {
			Ipv4Address ip = gridHome->getIp();
			file << home->getId() << "," << Helper::toString(ip) << "\n";
		} else {
			file << home->getId() << ",\n";
		}
	}
	std::vector<Server*>* servers = ServerContainer::Get()->getServers();
	for(uint i = 0; i < servers->size(); i++) {
		file << servers->at(i)->getId() << "," << servers->at(i)->getIp() << "\n";
	}
	ClusterServer* clusterServer = ServerContainer::Get()->getClusterServer();
	if(clusterServer != NULL) {
		Ipv4Address ip = clusterServer->getIp();
		file << clusterServer->getId() << "," << Helper::toString(ip) << "\n";
	}
	Log::f("StatsExporter", "\tfile ips exported");
}

void StatsExporter::exportWeb() {
	int interval = config->getInt("interval");
	int duration = config->getInt("duration") / interval;
	double homes = config->getDouble("total_homes");

	std::vector<std::string> headers;
	std::vector<std::vector<float>*> out;

	// total consumption
	std::map<std::string, int>* ids = (*stats->getStatIds())[STAT_CONSUMPTION];
	std::vector<std::vector<float>*>* data = stats->getStats();
	std::vector<float> tmp(duration, 0);
	std::map<std::string, int>::iterator it;
	for(it = ids->begin(); it != ids->end(); ++it) {
		std::vector<float>* d = (*data)[it->second];
		for(int i = 0; i < duration; i++) {
			tmp[i] += (*d)[i] / homes;
		}
	}
	out.push_back(&tmp);
	headers.push_back("total");

	// grouped
	for(uint i = 0; i < grouped.size(); i++) {
		out.push_back(grouped[i]);
	}
	headers.push_back("adaptable");
	headers.push_back("base");
	headers.push_back("movable");
	headers.push_back("other");
	headers.push_back("switchable");

	// car
	std::vector<float> carMin(duration, INT_MAX);
	std::vector<float> carMax(duration, INT_MIN);
	std::vector<float> carMean(duration, 0);
	ids = (*stats->getStatIds())["pcharge"];
	std::vector<int> carIds;
	for(it = ids->begin(); it != ids->end(); ++it) {
		if(Helper::getDeviceType(it->first).compare("car") == 0)
			carIds.push_back(it->second);
	}
	if(carIds.size() > 0) {
		for(uint i = 0; i < carIds.size(); i++) {
			std::vector<float>* d = (*data)[carIds[i]];
			for(int j = 0; j < duration; j++) {
				float f = (*d)[j] * 100;
				if(f < carMin[j])
					carMin[j] = f;
				if(f > carMax[j])
					carMax[j] = f;
				carMean[j] += f;
			}
		}
		for(int j = 0; j < duration; j++) {
			carMean[j] /= carIds.size();
		}
		headers.push_back("car_min");
		headers.push_back("car_max");
		headers.push_back("car_mean");
		out.push_back(&carMin);
		out.push_back(&carMax);
		out.push_back(&carMean);
	}

	exportCsv("consumption_all", &headers, &out);
}

void StatsExporter::exportCsv(std::string filename, std::vector<std::string>* headers, std::vector<std::vector<float>*>* data) {
	time_t now = Tick::Get()->getStart();
	int interval = config->getInt("interval");
	std::string nameWithId = filename;
	if(!config->getString("identifier").empty())
		nameWithId = MyConfig::Get()->getString("identifier") + "_" + filename;
	std::ofstream file(("results/" + nameWithId + ".csv").c_str());
	if(file) {
		// headers
		file << "time" << ",";
		for(uint i = 0; i < headers->size(); i++) {
			file << (*headers)[i];
			if(i < headers->size() - 1)
				file << ",";
		}
		file << "\n";
		// data
		int lines = (*data)[0]->size();
		for(int i = 0; i < lines; i++) {
			file << Helper::createTimestring(&now).c_str() << ",";
			now += interval * 60;
			for(uint j = 0; j < data->size(); j++) {
				file << (int) (*(*data)[j])[i];
				if(j < data->size() - 1)
					file << ",";
			}
			file << "\n";
		}
		file.close();
		Log::f("StatsExporter", "\tfile " + filename + " written");
	} else
		throw("could not create file " + filename);
}

}
