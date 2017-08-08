#ifndef MYSINGLETON_H_
#define MYSINGLETON_H_

namespace ns3 {

template<typename T>
class MySingleton {

public:
	static T* Get() {
		static T instance;
		return &instance;
	}

protected:
	MySingleton<T>() {}

};

}

#endif
