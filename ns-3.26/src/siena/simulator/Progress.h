#ifndef PROGRESS_H_
#define PROGRESS_H_

#include <string>
#include <sstream>
#include <ctime>
#include "ns3/simulator.h"
#include "ns3/MyConfig.h"
#include "ns3/Log.h"
#include "ns3/Tick.h"

namespace ns3 {

class Progress {

private:
	MyConfig* c;
	Tick* t;
	int duration;
	time_t start;
	int last;

public:
	Progress() {
		c = MyConfig::Get();
		t = Tick::Get();
		duration = MyConfig::Get()->getInt("duration");
		start = time(NULL);
		last = 0;
		Simulator::Schedule(Seconds(0), &Progress::print, this);
	}

	virtual ~Progress() {}

	virtual void print() {
		int tick = t->getSimMinute();
		int progress = (double) (tick+1) / duration * 100;
		if(progress > last) {
			std::stringstream s;
			s << progress << "%, duration: " << Tick::getTime(difftime(time(NULL), start));
			Log::f("Progress", s.str());
			last = progress;
		}
		if(tick < duration - 1)
			Simulator::Schedule(Seconds(60), &Progress::print, this);
	}

};

}

#endif
