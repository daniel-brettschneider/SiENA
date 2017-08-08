#ifndef LOG_H_
#define LOG_H_

#include <string>
#include <iostream>
#include <iomanip>
#include "ns3/Tick.h"
#include "ns3/MyConfig.h"
#include "ns3/Turbo.h"


namespace ns3 {

class Log {

public:
	virtual ~Log() {}

	template<class T1, class T2>
	static void i(std::string tag, T1 msg1, T2 msg2, bool force = false) {
			if(force
				#ifndef	TURBO
					|| MyConfig::Get()->getBool("log_all")
				#endif
					)
				std::cout << std::setw(18) << std::left << Tick::Get()->getString() << " [" << std::setw(14) << std::right << tag << "] "
					<< msg1 << msg2 << std::endl;

	}

	static void i(std::string tag, std::string msg) {
		i(tag, msg, "");
	}

	static void f(std::string tag, std::string msg) {
		i(tag, msg, "", true);
	}

};

}

#endif
