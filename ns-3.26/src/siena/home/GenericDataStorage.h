#ifndef NS_3_22_SRC_SIENA_HOME_GENERICDATASTORAGE_H_
#define NS_3_22_SRC_SIENA_HOME_GENERICDATASTORAGE_H_

#include <map>
#include <string>

namespace ns3 {

class GenericDataStorage {

protected:
	std::map<std::string,void*> data;

public:
	GenericDataStorage() {}
	virtual ~GenericDataStorage() {}

	std::map<std::string,void*>* getData() { return &data; }
	void addDataValue(std::string id, void* d) { data[id] = d; }

	// cannot implement in source file because of internal template
	template<typename T>
	T* getData(std::string id, bool createNewIfMissing = true) {
		std::map<std::string,void*>::iterator it = data.find(id);
		if(it == data.end()) {
			if(createNewIfMissing) {
				T* tmp = new T;
				data[id] = tmp;
				return tmp;
			}
			return NULL;
		} else
			return (T*) it->second;
	}

};

}

#endif
