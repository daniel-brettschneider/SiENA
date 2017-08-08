#include "../../siena/simulator/Tick.h"

#include "ns3/DataBasis.h"

namespace ns3 {

Tick::Tick() : start(-1), start_t(-1), last(-1), timeString("--:--:--/-"), lastPeriod(-1) {
	init();
	dayClasses = (*DataBasis::Get()->get("heat/Classification", "csv", CSV_PER_COLUMN, false, 1, 1096, false))[0];
}

void Tick::init() {
	MyConfig* config = MyConfig::Get();
	start_t = Helper::parseDate(config->getString("start_date"));
	time_t data_start = Helper::parseDate(config->getString("data_start_date"));
	start = difftime(start_t, data_start) / 60;
	config->set("start", start);
	Time duration(config->getString("duration"));
	config->set("duration", (int) duration.GetMinutes());
	Time interval(config->getString("interval"));
	config->set("interval", (int) interval.GetMinutes());
	this->interval = interval.GetMinutes();
	this->duration = duration.GetMinutes();
}

int Tick::getMinute() {
	return start + Simulator::Now().GetSeconds() / 60;
}

int Tick::getSimMinute() {
	return Simulator::Now().GetSeconds() / 60;
}

std::string Tick::getString() {
	int tmp = Simulator::Now().GetSeconds();
	if(tmp != last) {
		last = tmp;
		int d = tmp / 86400;
		tmp -= d * 86400;
		int h = tmp / 3600;
		tmp -= h * 3600;
		int min = tmp / 60;
		tmp -= min * 60;
		std::stringstream s;
		s << std::setw(3) << std::setfill('0') << d << "-"<< std::setw(2) << h << ":" << std::setw(2) << min << ":"
				<< std::setw(2) << tmp << "/" << getSimMinute();
		timeString = s.str();
	}
	return timeString;
}

std::string Tick::getTime(double sec) {
	int h = sec / 3600;
	sec -= h * 3600;
	int min = sec / 60;
	sec -= min * 60;
	std::stringstream s;
	s << (h < 10 ? "0" : "") << h << ":" << (min < 10 ? "0" : "") << min << ":" << (sec < 10 ? "0" : "") << sec;
	return s.str();
}

int Tick::getTimePeriod() {
	int tmp = getSimMinute();
//	std::cout << "tmp:" << tmp << std::endl;
	if(tmp != lastPeriod) {
		lastPeriod = tmp;
		lastPeriodData = getTimePeriod(tmp);
	}
	return lastPeriodData;
}

int Tick::getTimePeriod(int tick) {
	time_t tmp = start_t + tick * 60;
	struct tm* t = localtime(&tmp);
	bool sundayHoliday = false;
	int dayClass = (*dayClasses)[(start + tick) / 1440];
	if(dayClass == 3 || dayClass == 4 || dayClass == 6 || dayClass == 9 || dayClass == 10)
		sundayHoliday = true;
	if(t->tm_mon < 2 || (t->tm_mon == 2 && t->tm_mday < 21)) { // 01.01 - 20.03.
		if(sundayHoliday || t->tm_wday == 0) return WSonn;
		else if(t->tm_wday == 6) return WSams;
		else return WWerk;
	} else if((t->tm_mon == 2 && t->tm_mday >= 21) || t->tm_mon == 3 || (t->tm_mon == 4 && t->tm_mday < 15)) { // 21.03. - 14.05.
		if(sundayHoliday || t->tm_wday == 0) return USonn;
		else if(t->tm_wday == 6) return USams;
		else return UWerk;
	} else if((t->tm_mon == 4 && t->tm_mday >= 15) || (t->tm_mon > 4 && t->tm_mon < 8) || (t->tm_mon == 8 && t->tm_mday < 15)) { // 15.05. - 14.09.
		if(sundayHoliday || t->tm_wday == 0) return SSonn;
		else if(t->tm_wday == 6) return SSams;
		else return SWerk;
	} else if((t->tm_mon == 8 && t->tm_mday >= 15) || t->tm_mon == 9) { // 15.09. - 31.10.
		if(sundayHoliday || t->tm_wday == 0) return USonn;
		else if(t->tm_wday == 6) return USams;
		else return UWerk;
	} else { // 01.11. - 31.12.
		if(sundayHoliday || t->tm_wday == 0) return WSonn;
		else if(t->tm_wday == 6) return WSams;
		else return WWerk;
	}
}

int Tick::getTimetoNextIntervalMilliSeconds() {
	int timetoNextInterval;
	int time = Simulator::Now().GetMilliSeconds();
//	std::cout << "time: " << time << std::endl;
	//get time next interval
	int timeInterval = MyConfig::Get()->getInt("interval") * 60000;
//	std::cout << timeInterval << std::endl;
	timetoNextInterval = timeInterval - (time%timeInterval);
	return timetoNextInterval;
}

int Tick::getTimeInInterval() {
	return Simulator::Now().GetMilliSeconds() % (interval * 60000);
}

int Tick::getDayType(int tick) {
	time_t tmp = start_t + tick * 60;
	struct tm* t = localtime(&tmp);
	return t->tm_wday;
}

int Tick::getTick() {
	return Simulator::Now().GetMinutes() / interval;
}

}
