#ifndef NS_3_22_SRC_SIENA_SIMULATOR_RUNNINGAVG_H_
#define NS_3_22_SRC_SIENA_SIMULATOR_RUNNINGAVG_H_

namespace ns3 {

class RunningAvg {

private:
	double value;
	int count;

public:
	RunningAvg() : value(0), count(0) {}
	virtual ~RunningAvg() {}

	double getValue() {
		if(count == 0) {
			return 0.27;
		} else {
			return value;
		}
	}
	int getCount() { return count; }

	void add(double add) {
		value = ((value * count) + add) / (count + 1);
		count++;
	}

};

}

#endif
