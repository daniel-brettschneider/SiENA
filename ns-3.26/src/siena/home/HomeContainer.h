#ifndef HOMECONTAINER_H_
#define HOMECONTAINER_H_

#include <vector>
#include "ns3/MySingleton.h"
#include "ns3/MyConfig.h"
#include "ns3/Composite.h"
#include "ns3/ConventionalHome.h"

namespace ns3 {

class HomeContainer : public MySingleton<HomeContainer> {
	friend class MySingleton<HomeContainer>;

private:
	std::vector<ConventionalHome*> homes;
	std::vector<std::vector<std::vector<ConventionalHome*>*>*> linker;
	std::vector<Composite*> composites;

public:
	virtual ~HomeContainer() {
		for(uint i = 0; i < linker.size(); i++) {
			for(uint j = 0; j < linker[i]->size(); j++) {
				for(uint k = 0; k < (*linker[i])[j]->size(); k++) {
					delete (*(*linker[i])[j])[k];
				}
				delete (*linker[i])[j];
			}
			delete linker[i];
		}
		for(uint i = 0; i < composites.size(); i++) {
			delete composites[i];
		}
	}

	void addHome(ConventionalHome* home, int type, int cluster) {
		homes.push_back(home);
		(*linker[cluster])[type]->push_back(home);
	}
	void addComposite(Composite* composite) { this->composites.push_back(composite); }

	std::vector<ConventionalHome*>* getHomes(int cluster, int type) { return (*linker[cluster])[type]; }
	std::vector<std::vector<ConventionalHome*>*>* getHomes(int cluster) { return linker[cluster]; }
	std::vector<ConventionalHome*>* getHomes() { return &homes; }

private:
	HomeContainer() {
		int clusters = MyConfig::Get()->getInt("clusters");
		for(int i = 0; i < clusters; i++) {
			std::vector<std::vector<ConventionalHome*>*>* types = new std::vector<std::vector<ConventionalHome*>*>;
			for(int j = 0; j < 3; j++) {
				std::vector<ConventionalHome*>* typeHomes = new std::vector<ConventionalHome*>;
				types->push_back(typeHomes);
			}
			linker.push_back(types);
		}
	}

};

}

#endif
