#ifndef NS_3_22_SRC_SIENA_NETWORK_ATTRIBUTECONTAINER_H_
#define NS_3_22_SRC_SIENA_NETWORK_ATTRIBUTECONTAINER_H_

#include <vector>
#include "ns3/object.h"

namespace ns3 {

template<typename T>
class AttributeContainer : public Object {

private:
	std::vector<T> list;

public:
	AttributeContainer() {}
	virtual ~AttributeContainer() {}
	void push_back(T t) { list.push_back(t); }
	T at(int i) { return list[i]; }
	int size() { return list.size(); }

};

}

#endif
