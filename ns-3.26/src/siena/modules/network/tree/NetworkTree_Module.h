#ifndef BASEBRITENETWORK_H_
#define BASEBRITENETWORK_H_

#include <string>
#include <vector>
#include "ns3/MyConfig.h"
#include "ns3/GridHome.h"
#include "ns3/SmartHome.h"
#include "ns3/Server.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/ConventionalHome.h"
#include "ns3/Log.h"
#include "ns3/ClusterServer.h"
#include "ns3/HomeContainer.h"
#include "ns3/NetworkTree.h"
#include "ns3/Scenario.h"
#include "ns3/ServerContainer.h"
#include <algorithm>
#include "ns3/Network_HomeHelper.h"
#include "ns3/Helper.h"
#include "ns3/AttributeContainer.h"
#include "ns3/BackgroundTraffic_Device.h"

namespace ns3 {

static void networkTree_create(std::vector<void*>* params) {
	ApplicationContainer* apps = (ApplicationContainer*) (*params)[0];

	MyConfig* c = MyConfig::Get();
	Scenario* scenario = Scenario::Get();

	// get number of composites
	int numberOfComposites = 0;
	std::vector<ClusterConfig*>* clusterConfigs = scenario->getClusters();
	for(uint c = 0; c < clusterConfigs->size(); c++) {
		numberOfComposites += (*clusterConfigs)[c]->getComposites()->size();
	}

	// network
	int numberOfNodes = c->getInt("total_homes") + c->getInt("clusters") + 1 + numberOfComposites; // homes -- server -- clusterserver
	NetworkTree tree;
	NodeContainer nodes = tree.create(numberOfNodes);
	tree.assignAddresses(Ipv4Address("10.0.0.0"), Ipv4Mask("/8"));
	if(c->getBool("simulate_communication")) {
		Log::f("TreeNetwork", "populating routing tables... (this may take a while)");
		Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	}

	network_assignHomes(apps, &nodes);

	// register mac traces and background traffic
	TraceHelper* traceHelper = TraceHelper::Get();
	std::vector<ConventionalHome*>* homes = HomeContainer::Get()->getHomes();
	Ptr<AttributeContainer<std::string> > homeAddresses = CreateObject<AttributeContainer<std::string> >();
	Ptr<AttributeContainer<int> > homeDataRates = CreateObject<AttributeContainer<int> >();
	Ptr<Node> trafficServer = tree.getRootNode();
	Ipv4Address trafficIpAdr = trafficServer->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	std::string trafficIp = Helper::toString(trafficIpAdr);
	for(uint i = 0; i < homes->size(); i++) {
		GridHome* home = dynamic_cast<GridHome*>((*homes)[i]);
		if(home != NULL) {
			if(c->getDouble("background_traffic") > 0) {
				std::map<std::string, Device*>* devices = home->getDevices();
				std::map<std::string, Device*>::iterator it = devices->find(home->getId() + "_traffic");
				if(it != devices->end())
					((BackgroundTraffic_Device*) it->second)->setServer(trafficIp);
			}
			Ipv4Address adr = home->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
			traceHelper->registerTracesStats(home->getId(), adr);
			traceHelper->registerTracesP2P(home->GetNode());
			homeAddresses->push_back(Helper::toString(adr));
			DataRateValue v;
			home->GetNode()->GetDevice(1)->GetAttribute("DataRate", v);
			int bitrate = v.Get().GetBitRate();
			homeDataRates->push_back(bitrate);
		}
	}
	ServerContainer* serverContainer = ServerContainer::Get();
	std::vector<Server*>* servers = serverContainer->getServers();
	for(uint i = 0; i < servers->size(); i++) {
		Server* server = (*servers)[i];
		traceHelper->registerTracesStats(server->getId(), server->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
		traceHelper->registerTracesP2P(server->GetNode());
	}
	if(serverContainer->getClusterServer() != NULL) {
		ClusterServer* server = serverContainer->getClusterServer();
		traceHelper->registerTracesStats(server->getId(), server->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
		traceHelper->registerTracesP2P(server->GetNode());
	}

	// background traffic
	if(c->getDouble("background_traffic") > 0) {
		ObjectFactory serverFactory;
		serverFactory.SetTypeId(BackgroundTraffic_App::GetTypeId());
		serverFactory.Set("homeAddresses", PointerValue(homeAddresses));
		serverFactory.Set("homeDataRates", PointerValue(homeDataRates));
		serverFactory.Set("id", StringValue("traffic_server"));
		Ptr<Application> app = serverFactory.Create<BackgroundTraffic_App>();
		trafficServer->AddApplication(app);
		apps->Add(app);
	}
}

}

#endif
