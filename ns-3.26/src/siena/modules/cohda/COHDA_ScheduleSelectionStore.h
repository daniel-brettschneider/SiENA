#ifndef COHDA_SCHEDULESELECTIONSTORE_H_
#define COHDA_SCHEDULESELECTIONSTORE_H_

#include <vector>


namespace ns3 {

class COHDA_ScheduleSelectionStore {

private://lambda, vector mit geräten
	std::map<uint, std::vector<double>*> sss;

public:
	COHDA_ScheduleSelectionStore () {}
	COHDA_ScheduleSelectionStore (std::map<uint, std::vector<double>*>* sss): sss(*sss) {}
	virtual ~COHDA_ScheduleSelectionStore() {
		std::map<uint, std::vector<double>*>::iterator it;
		for(it = sss.begin(); it != sss.end(); ++it) {
			delete it->second;
		}
	}

	uint getLambdaAmount() { return sss.size();	}
	std::map<uint, std::vector<double>*>* getSSS() { return &sss; }
	std::vector<double>* getStorage(int lambda) { return this->sss[lambda]; }

	void setSSS(std::map<uint, std::vector<double>*>* sss) {
		// delete current
		std::map<uint, std::vector<double>*>::iterator it;
		for(it = this->sss.begin(); it != this->sss.end(); ++it) {
			delete it->second;
		}
		// set new
		this->sss = *sss;
	}

	void addScheduleStorage(uint lambda, std::vector<double>* ss) {
		std::pair<std::map<uint, std::vector<double>*>::iterator, bool> ret = sss.insert(std::pair<uint, std::vector<double>*>(lambda, ss));
		if(ret.second == false) {
			delete ret.first->second;
			ret.first->second = ss;
		}
	}

	bool contains(int lambda) { return sss.find(lambda) != sss.end(); }

	void clear() {
		// delete current
		std::map<uint, std::vector<double>*>::iterator it;
		for(it = this->sss.begin(); it != this->sss.end(); ++it) {
			delete it->second;
		}
		// clear
		sss.clear();
	}

};

}

#endif
