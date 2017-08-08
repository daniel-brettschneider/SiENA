#ifndef NS_3_22_SRC_SIENA_MODULES_NETWORK_NETWORK_HOMEHELPER_H_
#define NS_3_22_SRC_SIENA_MODULES_NETWORK_NETWORK_HOMEHELPER_H_

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
#include "ns3/Scenario.h"
#include "ns3/ServerContainer.h"
#include <algorithm>
#include "ns3/AttributeContainer.h"

namespace ns3 {

static ConventionalHome* network_createHome(HomeConfig* config, NodeContainer* nodes, int nodeId, int h, int c, ApplicationContainer* apps) {
	std::stringstream id;
	// conventional homes
	if(config->getType() == 0) {
		if(config->getName() == "")
			id << "home_" << c << "_" << h;
		else
			id << config->getName();
		ConventionalHome* home = new ConventionalHome(id.str());
		home->init(config);
		return home;
	// smart homes
	} else if(config->getType() == 1) {
		if(config->getName() == "")
			id << "smarthome_" << c << "_" << h;
		else
			id << config->getName();
		ConventionalHome* home = new SmartHome(id.str());
		home->init(config);
		return home;
	// grid home
	} else {
		if(config->getName() == "")
			id << "gridhome_" << c << "_" << h;
		else
			id << config->getName();
		ObjectFactory homeFactory;
		homeFactory.SetTypeId(GridHome::GetTypeId());
		homeFactory.Set("id", StringValue(id.str()));
		Ptr<Application> app = homeFactory.Create<GridHome>();
		nodes->Get(nodeId)->AddApplication(app);
		apps->Add(app);
		GridHome* home = (GridHome*) GetPointer(app);
		home->init(config);
		return home;
	}
}

/*
 * use server nodes container to separate home and server nodes, otherwise leave NULL and only use home container
 * use random to associate homes and servers to random nodes
 */
static void network_assignHomes(ApplicationContainer* apps, NodeContainer* homeNodes, bool randomizeNodeOrder = true, NodeContainer* serverNodes = NULL) {
	MyConfig* config = MyConfig::Get();
	Scenario* scenario = Scenario::Get();
	std::vector<ClusterConfig*>* clusterConfigs = scenario->getClusters();
	HomeContainer* homes = HomeContainer::Get();
	ServerContainer* servers = ServerContainer::Get();

	// randomize
	std::vector<int> homeIds;
	for(uint i = 0; i < homeNodes->GetN(); i++) {
		homeIds.push_back(i);
	}
	if(randomizeNodeOrder)
		std::random_shuffle(homeIds.begin(), homeIds.end());
	std::vector<int> serverIds;
	if(serverNodes != NULL) {
		for(uint i = 0; i < serverNodes->GetN(); i++) {
			serverIds.push_back(i);
		}
		if(randomizeNodeOrder)
			std::random_shuffle(serverIds.begin(), serverIds.end());
	}

	ObjectFactory serverFactory;
	serverFactory.SetTypeId(Server::GetTypeId());
	Ptr<AttributeContainer<Ipv4Address> > serverAddresses = CreateObject<AttributeContainer<Ipv4Address> >();
	int homeId = 0, serverId = 0;
	bool hasGridHome = false;
	for(uint c = 0; c < clusterConfigs->size(); c++) {
		// homes
		std::vector<HomeConfig*>* homeConfigs = (*clusterConfigs)[c]->getHomes();
		Ptr<AttributeContainer<Ipv4Address> > gridhomeAddresses = CreateObject<AttributeContainer<Ipv4Address> >();
		bool clusterHasGridHome = false;
		for(uint h = 0; h < homeConfigs->size(); h++) {
			ConventionalHome* home = network_createHome((*homeConfigs)[h], homeNodes, homeIds[homeId], h, c, apps);
			homes->addHome(home, (*homeConfigs)[h]->getType(), c);
			if((*homeConfigs)[h]->getType() == 2) {
				gridhomeAddresses->push_back(homeNodes->Get(homeIds[homeId])->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
				clusterHasGridHome = true;
				hasGridHome = true;
			}
			homeId++;
		}
		// composites
		std::vector<CompositeConfig*>* compositeConfigs = (*clusterConfigs)[c]->getComposites();
		for(uint comp = 0; comp < compositeConfigs->size(); comp++) {
			std::stringstream id;
			id << "composite_" << c << "_" << comp;
			CompositeConfig* compositeConfig = (*compositeConfigs)[comp];
			ObjectFactory compositeFactory;
			compositeFactory.SetTypeId(Composite::GetTypeId());
			compositeFactory.Set("id", StringValue(id.str()));
			Ptr<Application> app = compositeFactory.Create<Composite>();
			homeNodes->Get(homeIds[homeId])->AddApplication(app);
			apps->Add(app);
			gridhomeAddresses->push_back(homeNodes->Get(homeIds[homeId])->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
			homeId++;
			clusterHasGridHome = true;
			hasGridHome = true;
			Composite* composite = (Composite*) GetPointer(app);
			std::vector<HomeConfig*>* compHomeConfigs = compositeConfig->getHomes();
			for(uint h = 0; h < compHomeConfigs->size(); h++) {
				ConventionalHome* home = network_createHome((*compHomeConfigs)[h], homeNodes, homeIds[homeId], h + homeConfigs->size(), c, apps);
				homes->addHome(home, (*compHomeConfigs)[h]->getType(), c);
				if((*compHomeConfigs)[h]->getType() == 2) {
					gridhomeAddresses->push_back(homeNodes->Get(homeIds[homeId])->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
				}
				composite->addHome(home);
				homeId++;
			}
			composite->init(compositeConfig);
		}
		// server
		if(clusterHasGridHome) {
			// check for separate servers
			Ptr<Node> thisServerNode;
			if(serverNodes != NULL)
				thisServerNode = serverNodes->Get(serverIds[serverId++]);
			else
				thisServerNode = homeNodes->Get(homeIds[homeId++]);
			std::stringstream id;
			id << "server_" << c;
			serverFactory.Set("homeAddresses", PointerValue(gridhomeAddresses));
			serverFactory.Set("id", StringValue(id.str()));
			if(serverNodes != NULL)
				serverFactory.Set("server", AddressValue(serverNodes->Get(serverIds[serverIds.size()-1])->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()));
			else
				serverFactory.Set("server", AddressValue(homeNodes->Get(homeIds[homeIds.size()-1])->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal()));
			serverFactory.Set("cluster", IntegerValue(c));
			Ptr<Application> app = serverFactory.Create<Server>();
			thisServerNode->AddApplication(app);
			apps->Add(app);
			serverAddresses->push_back(thisServerNode->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal());
			servers->addServer((Server*) GetPointer(app));
		}
	}
	// cluster server
	if(config->getBool("enable_clustering") && hasGridHome) {
		serverFactory.SetTypeId(ClusterServer::GetTypeId());
		serverFactory.Set("id", StringValue("clusterserver"));
		serverFactory.Set("children", PointerValue(serverAddresses));
		Ptr<Application> app = serverFactory.Create<ClusterServer>();
		if(serverNodes != NULL)
			serverNodes->Get(serverIds[serverId])->AddApplication(app);
		else
			homeNodes->Get(homeIds[homeId])->AddApplication(app);
		apps->Add(app);
		servers->setClusterServer((ClusterServer*) GetPointer(app));
	}
}

}

#endif
