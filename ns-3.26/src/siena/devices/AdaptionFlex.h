#ifndef ADAPTIONFLEX_H_
#define ADAPTIONFLEX_H_

#include <string>
#include <sstream>
#include <climits>
#include "ns3/Turbo.h"
#include <cmath>

namespace ns3 {

class AdaptionFlex {

private:
	double minimum;
	double maximum;
	double desired;
	double assigned;
	bool adaptable;

public:
	AdaptionFlex() : minimum(0), maximum(0), desired(0), assigned(0), adaptable(true) {}
	AdaptionFlex(double minimum, double maximum, double desired) : minimum(minimum), maximum(maximum), desired(desired),
		assigned(0), adaptable(true) {}
	AdaptionFlex(double minimum, double maximum, double desired, double base) : minimum(minimum), maximum(maximum), desired(desired),
		assigned(0), adaptable(true) {}
	AdaptionFlex(double minimum, double maximum, double desired, double base, double assigned) : minimum(minimum), maximum(maximum), desired(desired),
		assigned(assigned), adaptable(true) {}
	virtual ~AdaptionFlex() {}

	double getMinimum() { return minimum; }
	double getMaximum() { return maximum; }
	double getDesired() { return desired; }
	double getAssigned() { return assigned; }
	double getPotentialUp() { return fabs(assigned - maximum); }
	double getPotentialDown() { return fabs(assigned - minimum); }

	void setMinimum(double minimum) { this->minimum = minimum; }
	void setMaximum(double maximum) { this->maximum = maximum; }
	void setDesired(double desired) { this->desired = desired; }
	void setAssigned(double assigned) { this->assigned = assigned; }
	void setAdaptable(bool adaptable) { this->adaptable = adaptable; }

	bool isAdaptable() { return (minimum != maximum) && (desired >= minimum && desired <= maximum) && adaptable == true; }
	bool isAdaptableUp() { return assigned < maximum; }
	bool isAdaptableDown() { return assigned > minimum; }

	std::string toString() {
		#ifndef TURBO
			std::stringstream s;
			s  << "↓" << minimum << " ↑" << maximum << " ♥" << desired << " →" << assigned;
			return s.str();
		#else
			return "optimized out";
		#endif
	}

};

}

#endif
