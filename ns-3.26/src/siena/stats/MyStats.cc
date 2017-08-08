#include "../../siena/stats/MyStats.h"

namespace ns3 {

const std::string MyStats::TAG = "Stats";

MyStats::MyStats() {
	t = Tick::Get();
	MyConfig* config = MyConfig::Get();
	duration = config->getInt("duration");
	interval = config->getInt("interval");
	statIdCounter = 0;
	saveIds = Helper::split(config->getString("keep_only_these_stats_in_ram"), ',');
	filter = config->getBool("filter_stats");
}

MyStats::~MyStats() {
	std::map<std::string, std::map<std::string, int>*>::iterator it;
	for(it = statIds.begin(); it != statIds.end(); ++it) {
		delete it->second;
	}
	for(uint i = 0; i < stats.size(); i++) {
		delete stats[i];
	}
	for(uint i = 0; i < counters.size(); i++) {
		delete counters[i];
	}
}

int MyStats::registerStat(std::string group, std::string name, float defaultValue) {
	// check whether to save this
	if(filter && !Helper::contains(&saveIds, group))
		return -1;

	// find group
	std::map<std::string, std::map<std::string, int>*>::iterator groupIt = statIds.find(group);
	if(groupIt != statIds.end()) {
		// find name
		std::map<std::string, int>::iterator nameIt = groupIt->second->find(name);
		if(nameIt != groupIt->second->end())
			// return id
			return nameIt->second;
		// create id for name
		else
			groupIt->second->insert(std::pair<std::string, int>(name, statIdCounter++));
	} else {
		// create group and id for name
		std::map<std::string, int>* tmp = new std::map<std::string, int>;
		tmp->insert(std::pair<std::string, int>(name, statIdCounter++));
		statIds.insert(std::pair<std::string, std::map<std::string, int>*>(group, tmp));
	}
	// create stat vector
	stats.push_back(new std::vector<float>(duration / interval, defaultValue));

	return statIdCounter - 1;
}

int MyStats::registerCounter(std::string group, std::string name) {
	// check whether to save this
	if(filter && !Helper::contains(&saveIds, "counters"))
		return -1;

	std::string id = group + "_" + name;
	std::map<std::string, int>::iterator it = counterIds.find(id);
	if(it != counterIds.end())
		return it->second;
	counterIds.insert(std::pair<std::string, int>(id, counterIds.size()));

	counters.push_back(new std::vector<float>(1, 0));

	return counterIds.size() - 1;
}

void MyStats::addStat(int id, float value) {
	if(id < 0)
		return;
	if((uint) id < stats.size()) {
		(*stats[id])[t->getTick()] = value;
	} else
		throw("stat maybe not registered?");
}

/*
 * attention: stats may be initialised with INT_MIN
 */
void MyStats::increaseStat(int id, float amount) {
	if(id < 0)
		return;
	if((uint) id < stats.size()) {
		(*stats[id])[t->getTick()] += amount;
	} else
		throw("stat maybe not registered?");
}

void MyStats::updateCounter(int id, float value) {
	if(id < 0)
		return;
	if((uint) id < counters.size()) {
		(*counters[id])[0] += value;
	} else
		throw("counter maybe not registered?");
}

void MyStats::exportCsv() {
	Log::f(TAG, "exporting stats...");
	std::vector<std::string> exportIds = Helper::split(MyConfig::Get()->getString("save_only_these_stats"), ',');

	// counters
	if(counterIds.size() > 0 && (!filter || Helper::contains(&exportIds, "counters"))) {
		std::vector<std::string> headers(counterIds.size(), "");
		std::map<std::string, int>::iterator it;
		for(it = counterIds.begin(); it != counterIds.end(); ++it) {
			headers[it->second] = it->first;
		}
		this->exportCsv("counters", &headers, &counters);
	}

	// stats
	std::map<std::string, std::map<std::string, int>*>::iterator groupIt;
	for(groupIt = statIds.begin(); groupIt != statIds.end(); ++groupIt) {
		if(!filter || Helper::contains(&exportIds, groupIt->first)) {
			std::vector<std::string> headers;
			std::vector<int> dataIds;
			std::map<std::string, int>::iterator nameIt;
			for(nameIt = groupIt->second->begin(); nameIt != groupIt->second->end(); ++nameIt) {
				headers.push_back(nameIt->first);
				dataIds.push_back(nameIt->second);
			}
			this->exportCsv(groupIt->first, &headers, &stats, &dataIds);
		}
	}
}

void MyStats::exportCsv(std::string filename, std::vector<std::string>* headers, std::vector<std::vector<float>*>* data, std::vector<int>* dataIds) {
	std::string nameWithId = filename;
	if(!MyConfig::Get()->getString("identifier").empty())
		nameWithId = MyConfig::Get()->getString("identifier") + "_" + filename;
	std::ofstream file(("results/" + nameWithId + ".csv").c_str());
	if(file) {
		// headers
		for(uint i = 0; i < headers->size(); i++) {
			file << (*headers)[i];
			if(i < headers->size() - 1)
				file << ",";
		}
		file << "\n";
		// data
		if(dataIds == NULL) {
			int lines = (*data)[0]->size();
			for(int i = 0; i < lines; i++) {
				for(uint j = 0; j < data->size(); j++) {
					file << (*(*data)[j])[i];
					if(j < data->size() - 1)
						file << ",";
				}
				file << "\n";
			}
		} else {
			int lines = (*data)[0]->size();
			for(int i = 0; i < lines; i++) {
				for(uint j = 0; j < dataIds->size(); j++) {
					file << (*(*data)[(*dataIds)[j]])[i];
					if(j < dataIds->size() - 1)
						file << ",";
				}
				file << "\n";
			}
		}
		file.close();
		Log::f(TAG, "\tfile " + filename + " written");
	} else
		throw("could not create file " + filename);
}

}
