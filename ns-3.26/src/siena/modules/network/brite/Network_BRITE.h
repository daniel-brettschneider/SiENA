#ifndef NS_3_22_SRC_CITYGRID_MODULES_NETWORK_NETWORK_BRITE_H_
#define NS_3_22_SRC_CITYGRID_MODULES_NETWORK_NETWORK_BRITE_H_

#include <sys/types.h>
#include <vector>
#include <string>
#include <stdlib.h>
#include <time.h>

#include "ns3/application-container.h"
#include "ns3/ClusterConfig.h"
#include "ns3/MyConfig.h"
#include "ns3/MySingleton.h"
#include "ns3/Scenario.h"
#include "ns3/DataBasis.h"
#include "ns3/Helper.h"
#include "ns3/Network_Utilities.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include <algorithm>

namespace ns3 {

static void BRITE_create(std::vector<void*>* params) {

    InternetStackHelper stack;
    NodeContainer nodes;
    ApplicationContainer* apps = (ApplicationContainer*) params->at(0);
    MyConfig* c = MyConfig::Get();
    DataBasis* db = DataBasis::Get();
    std::string briteConfigPath = "data/brite/BRITE_Config.conf";

    // get number of composites
    Scenario* scenario = Scenario::Get();
    int numberOfComposites = 0;
    std::vector<ClusterConfig*>* clusterConfigs = scenario->getClusters();
    for (uint c = 0; c < clusterConfigs->size(); c++) {
        numberOfComposites += clusterConfigs->at(c)->getComposites()->size();
    }

    // network
    // homes -- server -- clusterserver
    uint32_t numberOfNodes = c->getInt("total_homes") + c->getInt("clusters") + 1 + numberOfComposites;
    int nAs = numberOfNodes / 50 + (numberOfNodes % 50 > 0 ? 1 : 0) + c->getInt("brite_nodes_extra");
    if (nAs < 2) nAs = 2;

    std::string config = db->getFileContent(briteConfigPath);
    config = config.replace(config.find("[x]"), 3, Helper::toString(nAs));
    db->writeFile(briteConfigPath + ".tmp", config);

    std::cout << "=================== BRITE Generation =====================" << std::endl;
    BriteTopologyHelper bth(briteConfigPath + ".tmp");
    Network_Utilities::generateDSLTopology(&stack, &bth);
    std::cout << "==========================================================" << std::endl;

    uint32_t briteLeafNodes = 0;
    for (u_int32_t i = 0; i < bth.GetNAs(); i++) {
        briteLeafNodes += bth.GetNLeafNodesForAs(i);
    }

//	stack.EnablePcapIpv4All("pcap/Network_BRITE");

    if (briteLeafNodes < numberOfNodes) {
        std::string message = "Not enough Nodes! Please edit 'brite_nodes_extra ' in network_brite.json";
        throw(message);
    }

    std::vector<Ptr<Node> > leafNodes;
    for (uint i = 0; i < bth.GetNAs(); i++) {
        for (uint j = 0; j < bth.GetNLeafNodesForAs(i); j++) {
            leafNodes.push_back(bth.GetLeafNodeForAs(i, j));
        }
    }
    std::random_shuffle(leafNodes.begin(), leafNodes.end());

    int minDelay = c->getInt("min_delay");
    int maxDelay = c->getInt("max_delay");
    int minDatarate = c->getInt("min_datarate");
    int maxDatarate = c->getInt("max_datarate");

    Ipv4AddressHelper address;
    Ipv4Address ipv4Network("176.0.0.0");
    Ipv4Mask ipv4Mask("255.255.255.252");

    address.SetBase(ipv4Network, ipv4Mask);
    nodes.Create(numberOfNodes);
    for (u_int32_t i = 0; i < numberOfNodes; i++) {
        PointToPointHelper pointToPoint;
        NodeContainer p2pNodes;
        NetDeviceContainer p2pDevices;
        Ipv4InterfaceContainer p2pInterfaces;

        p2pNodes.Add(nodes.Get(i));
        /**
         * zufaelliger Leaf Node irgendeines AS
         */
        p2pNodes.Add(leafNodes.at(i));

        /**
         * zufaellige Datarate and zufaelliger Delay abrufen
         */
        Random* random = Random::Get();
        int randomValue = random->get(minDatarate, maxDatarate);
        StringValue datarate(Helper::toString(randomValue) + "Mbps");
        randomValue = random->get(minDelay, maxDelay);
        StringValue delay(Helper::toString(randomValue) + "ms");

        Network_Utilities::createPointToPointTopology(&p2pNodes, &pointToPoint, &p2pDevices, &datarate, &delay);

        // install the node & assign IPv4 Addresses to the created Interfaces of each Net Device
        stack.Install(p2pNodes.Get(0));
        p2pInterfaces.Add(address.Assign(p2pDevices));

        ipv4Network = address.NewNetwork();
        address.SetBase(ipv4Network, ipv4Mask);
    }

    if (c->getBool("simulate_communication")) {
        Log::f("BRITENetwork", "populating routing tables... (this may take a while)");
        Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    }

    network_assignHomes(apps, &nodes);


// register mac traces
    TraceHelper* traceHelper = TraceHelper::Get();
    std::vector<ConventionalHome*>* homes = HomeContainer::Get()->getHomes();

    for(uint i = 0; i < homes->size(); i++) {
        GridHome* home = dynamic_cast<GridHome*>((*homes)[i]);
        if(home != NULL) {
            Ipv4Address adr = home->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            traceHelper->registerTracesStats(home->getId(), adr);
            traceHelper->registerTracesP2P(home->GetNode());
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
}
}

#endif
