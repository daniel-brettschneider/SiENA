#ifndef NS_3_22_SRC_SIENA_MODULES_NETWORK_LTE_NETWORK_LTE_HELPER_H_
#define NS_3_22_SRC_SIENA_MODULES_NETWORK_LTE_NETWORK_LTE_HELPER_H_

#include "ns3/MySingleton.h"
#include "ns3/lte-helper.h"
#include "ns3/MyConfig.h"

namespace ns3 {

class NetworkLTEHelper : public MySingleton<NetworkLTEHelper> {
	friend class MySingleton<NetworkLTEHelper>;

private:
	Ptr<LteHelper> lteHelper;

public:
	virtual ~NetworkLTEHelper() {}
	Ptr<LteHelper> getLteHelper() { return lteHelper; }

private:
	NetworkLTEHelper() {
		lteHelper = CreateObject<LteHelper>();
		MyConfig* config = MyConfig::Get();
		Simulator::Schedule(Seconds(config->getInt("duration") * 60), &NetworkLTEHelper::stopSim, this);
	}
	void stopSim() {
		Log::f("LTE Helper", "stopping simulator");
		if(!Simulator::IsFinished())
			Simulator::Stop();
	}
};

}

#endif
