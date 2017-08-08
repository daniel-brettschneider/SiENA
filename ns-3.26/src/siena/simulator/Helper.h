#ifndef HELPER_H_
#define HELPER_H_

#include <string>
#include <sstream>
#include <vector>
#include "ns3/uinteger.h"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cmath>

namespace ns3 {

class Helper {

public:
	virtual ~Helper();

	static std::vector<std::string> split(std::string s, char delim) {
		std::vector<std::string> tokens;
		std::stringstream ss(s);
		std::string token;
		while(std::getline(ss, token, delim)) {
			tokens.push_back(token);
		}
		return tokens;
	}

	static std::vector<double> splitToDouble(std::string s, char delim) {
		std::vector<double> tokens;
		std::stringstream ss(s);
		std::string token;
		while(std::getline(ss, token, delim)) {
			tokens.push_back(atof(token.c_str()));
		}
		return tokens;
	}

	static bool contains(std::vector<std::string>* list, std::string target) {
		for(uint i = 0; i < list->size(); i++) {
			if(target.compare((*list)[i]) == 0)
				return true;
		}
		return false;
	}

	static bool contains(std::string s, char delim, std::string target) {
		std::stringstream ss(s);
		std::string token;
		while(std::getline(ss, token, delim)) {
			if(token == target)
				return true;
		}
		return false;
	}

	static bool contains(std::string* list, std::string target, int n) {
		for(int i = 0; i < n; i++) {
			if(target.compare(list[i]) == 0)
				return true;
		}
		return false;
	}

	template<class T>
	static std::string toString(T& obj) {
		std::stringstream stream;
		stream << obj;
		return stream.str();
	}

	static inline std::string &rtrim(std::string &s) {
	        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	        return s;
	}

	static time_t parseDate(std::string dateString) {
		int d, m, y;
		sscanf(dateString.c_str(), "%2d.%2d.%2d", &d, &m, &y);
		struct tm date = {0};
		date.tm_mday = d;
		date.tm_mon = m - 1;
		date.tm_year = 100 + y;
		return mktime(&date);
	}

	static double map(double x, double in_min, double in_max, double out_min, double out_max) {
		return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
	}

	static std::string getDeviceType(std::string id) {
		return Helper::split(Helper::split(id, '_').back(), ':').front();
	}

	static std::string createTimestring(time_t* time) {
		char buffer[16];
		struct tm* t = localtime(time);
		strftime(buffer, 16, "%y%m%d%H%M", t);
		return std::string(buffer);
	}

	static double parseDouble(std::string s) {
		size_t found = s.find(",");
		if(found != std::string::npos)
			s.replace(found, 1, ".");
		return atof(s.c_str());
	}

	static int ceilSignificant(int i) {
		if(i <= 0)
			return 0;
		int count = 0;
		while(i % 10 == 0) {
			i /= 10;
			count++;
		}
		double d = (double) i / 10;
		return ceil(d) * pow(10, ++count);
	}

};

}

#endif
