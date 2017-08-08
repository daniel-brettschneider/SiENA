#ifndef FILEPRELOADER_H_
#define FILEPRELOADER_H_

#include "ns3/Log.h"
//#include <thread>
#include "ns3/DataBasis.h"

namespace ns3 {

class FilePreloader {

public:
	static void preload() {
		/*DataBasis* data = DataBasis::Get();
		std::thread t1(data->get("devices/baseload_SSams"));

		t1.join();*/
	}

};

}

#endif
