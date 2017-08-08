#ifndef COMPOSITE_H_
#define COMPOSITE_H_

#include "ns3/GridHome.h"
#include "ns3/CompositeConfig.h"

namespace ns3 {

class Composite : public GridHome {

private:
	std::vector<ConventionalHome*> homes;

public:
	static TypeId GetTypeId();
	Composite() {};
	virtual ~Composite() {};

	virtual void init(CompositeConfig* config);
	void addHome(ConventionalHome* home) { homes.push_back(home); }
	std::vector<ConventionalHome*>* getHomes() { return &homes; }

};

}

#endif
