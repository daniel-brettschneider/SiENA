#ifndef MODULES_H_
#define MODULES_H_

#include <string>
#include <map>
#include "ns3/MySingleton.h"

namespace ns3 {

class ConventionalHome;

// function pointer
typedef void (*fp)(std::vector<void*>*);

class Modules : public MySingleton<Modules> {
	friend class MySingleton<Modules>;

protected:
	std::map<std::string, fp> functions;

public:
	virtual ~Modules() {}

	void registerFunction(std::string name, fp function) {
		if(functions.insert(std::pair<std::string, fp>(name, function)).second == false)
			throw("algorithm " + name + " already registered");
	}

	void execute(std::string algo, std::vector<void*>* params) {
		std::map<std::string, fp>::iterator it = functions.find(algo);
		if(it == functions.end())
			throw("algo " + algo + " not registered");
		it->second(params);
	}

private:
	Modules() {}

};

}

#endif
