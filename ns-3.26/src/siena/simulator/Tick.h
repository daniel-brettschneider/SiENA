#ifndef TICK_H_
#define TICK_H_

#include <string>
#include <iomanip>
#include <sstream>
#include "ns3/simulator.h"
#include "ns3/MySingleton.h"
#include "ns3/MyConfig.h"
#include "ns3/Helper.h"
#include "ns3/nstime.h"

namespace ns3 {

class DataBasis;

#define SSams 0
#define SSonn 1
#define SWerk 2
#define USams 3
#define USonn 4
#define UWerk 5
#define WSams 6
#define WSonn 7
#define WWerk 8

class Tick : public MySingleton<Tick> {
	friend class MySingleton<Tick>;

private:
	int start;
	time_t start_t;
	int last;
	std::string timeString;
	std::vector<double>* dayClasses;
	int interval;
	int duration;
	int lastPeriod;
	int lastPeriodData;

public:
	virtual ~Tick() {}
	void init();
	int getMinute();
	int getSimMinute();
	int getTick();
	std::string getString();
	static std::string getTime(double sec);
	int getTimePeriod();
	int getTimePeriod(int tick);
	int getDayType(int tick);
	/*
	 * time_t!
	 */
	time_t getStart() { return start_t; }
	int getTimetoNextIntervalMilliSeconds();
	int getTimeInInterval();

	/*
	 * in minutes
	 */
	int getInterval() { return interval; }
	/*
	 * in minutes
	 */
	int getStartInt() { return start; }
	/*
	 * in minutes
	 */
	int getDuration() { return duration; }

private:
	Tick();

};

}

#endif
