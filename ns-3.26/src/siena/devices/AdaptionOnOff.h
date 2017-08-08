#ifndef ADAPTIONONOFF_H_
#define ADAPTIONONOFF_H_

#include <climits>

namespace ns3 {

class AdaptionOnOff {

private:
	int value;
	bool desired;
	int current;
	int category;
	int categoryValue;

public:
	AdaptionOnOff() : value(0), desired(false), current(0), category(INT_MIN), categoryValue(INT_MIN) {}
	AdaptionOnOff(int value, bool desired) : value(value), desired(desired), current(0), category(INT_MIN), categoryValue(INT_MIN) {}
	virtual ~AdaptionOnOff() {}

	int getValue() { return value; }
	bool isDesired() { return desired; }

	void setValue(int value) { this->value = value; }
	void setDesired(bool desired) { this->desired = desired; }
	void set(int value, bool desired, int current) { this->value = value; this->desired = desired; this->current = current; }
	void setCategory(int cat) { this->category = cat; }
	void setCategoryValue(int value) { this->categoryValue = value; }
	int getCategory() { if(category == INT_MIN) throw("category of AdaptionOnOff not set"); return category; }
	int getCategoryValue() { if(categoryValue == INT_MIN) throw("category value of AdaptionOnOff not set"); return categoryValue; }
	int getCurrent() { return current; }

	std::string toString() {
		#ifndef TURBO
			std::stringstream s;
			s << "#" << value << " ♥" << (desired ? "true" : "false") << " →" << current;
			return s.str();
		#else
			return "optimized out";
		#endif
	}
};

}

#endif
