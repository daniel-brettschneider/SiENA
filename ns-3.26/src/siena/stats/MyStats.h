#ifndef MYSTATS_H_
#define MYSTATS_H_

#include <string>
#include <map>
#include <stdexcept>
#include <climits>
#include <fstream>
#include "ns3/MySingleton.h"
#include "ns3/Log.h"
#include "ns3/MyConfig.h"
#include "ns3/Tick.h"
#include "ns3/Helper.h"

#define STAT_CONSUMPTION "consumption"
#define STAT_STATE "state"
#define STAT_OTHER "other"
#define STAT_COMMUNICATION "communication"
#define STAT_ACTIVATION "activation"
#define STAT_RUN "run"
#define STAT_ACTIVATION_LATE "late"

namespace ns3 {

class MyStats : public MySingleton<MyStats> {
	friend class MySingleton<MyStats>;

private:
	static const std::string TAG;
	std::map<std::string, std::map<std::string, int>*> statIds;
	std::map<std::string, int> counterIds;
	std::vector<std::vector<float>*> stats;
	std::vector<std::vector<float>*> counters;
	int statIdCounter;
	Tick* t;
	int duration;
	int interval;
	std::vector<std::string> saveIds;
	bool filter;

public:
	virtual ~MyStats();
	int registerStat(std::string group, std::string name, float defaultValue = INT_MIN);
	int registerCounter(std::string group, std::string name);
	void addStat(int id, float value);
	void increaseStat(int id, float amount);
	void updateCounter(int id, float value);
	void exportCsv();
	void exportCsv(std::string filename, std::vector<std::string>* headers, std::vector<std::vector<float>*>* data, std::vector<int>* dataIds = NULL);

	std::map<std::string, std::map<std::string, int>*>* getStatIds() { return & statIds; }
	std::map<std::string, int>* getCounterIds() { return &counterIds; }
	std::vector<std::vector<float>*>* getStats() { return &stats; }
	std::vector<std::vector<float>*>* getCounters() { return &counters; }
	float getStat(int tick, int id) { return (*stats[id])[tick]; }

private:
	MyStats();


};

}

#endif
