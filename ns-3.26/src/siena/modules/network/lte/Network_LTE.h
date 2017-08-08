#ifndef NS_3_22_SRC_CITYGRID_MODULES_NETWORK_NETWORK_LTE_H_
#define NS_3_22_SRC_CITYGRID_MODULES_NETWORK_NETWORK_LTE_H_

#include "ns3/application-container.h"
#include "ns3/ClusterConfig.h"
#include "ns3/MyConfig.h"
#include "ns3/MySingleton.h"
#include "ns3/Scenario.h"
#include "ns3/DataBasis.h"
#include "ns3/Helper.h"
#include "ns3/Network_Utilities.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"

#include <algorithm>
#include <vector>

#include "ns3/Network_Utilities.h"
#include "ns3/Network_HomeHelper.h"
#include "ns3/Network_LTE_Helper.h"
#include "ns3/PacketDelay.h"

using namespace ns3;

static void LTE_create(std::vector<void*>* params) {

    InternetStackHelper stack;
    ApplicationContainer* apps = (ApplicationContainer*) params->at(0);
    MyConfig* c = MyConfig::Get();
//    DataBasis* db = DataBasis::Get();
//    std::string briteConfigPath = "data/brite/BRITE_Config.conf";

// get number of composites
    Scenario* scenario = Scenario::Get();
    int numberOfComposites = 0;
    std::vector<ClusterConfig*>* clusterConfigs = scenario->getClusters();
    for (uint c = 0; c < clusterConfigs->size(); c++) {
        numberOfComposites += clusterConfigs->at(c)->getComposites()->size();
    }

    // network
    // homes -- server -- clusterserver
    uint32_t numberOfHomes = c->getInt("total_homes") + numberOfComposites;
    uint32_t numberOfServer = c->getInt("clusters") + 1;//(c->getInt("clusters") == 1 ? 0 : 1);
    uint32_t nodesPerEnbNode = c->getInt("nodes_per_enbNode");
    uint32_t numberOfEnbNodes = numberOfHomes / nodesPerEnbNode + (numberOfHomes % nodesPerEnbNode == 0 ? 0 : 1);
    uint32_t distanceBetweenEnbNodes = c->getInt("distance_between_enbNodes");
    uint32_t maxDistanceToEnbNode = c->getInt("max_distance_to_enbNode");
    double_t minAngle = c->getDouble("minAngle");
    double_t maxAngle = c->getDouble("maxAngle");

   // Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));

//    uint32_t numberOfNodes = numberOfHomes + numberOfServer + numberOfComposites;
//    int nAs = numberOfNodes / 50 + (numberOfNodes % 50 > 0 ? 1 : 0) + c->getInt("brite_nodes_extra");
//    if (nAs < 2) nAs = 2;
//
//    std::string config = db->getFileContent(briteConfigPath);
//    config = config.replace(config.find("[x]"), 3, Helper::toString(nAs));
//    db->writeFile(briteConfigPath + ".tmp", config);

    NodeContainer serverNodes;
    NodeContainer homeNodes;
    NodeContainer enbNodes;

    serverNodes.Create(numberOfServer);
    homeNodes.Create(numberOfHomes);
    enbNodes.Create(numberOfEnbNodes);
    stack.Install(serverNodes);
    stack.Install(homeNodes);

    ConfigStore inputConfig;
    inputConfig.ConfigureDefaults();

    Ptr<LteHelper> lteHelper = NetworkLTEHelper::Get()->getLteHelper();
//    lteHelper->EnableTraces();
//    lteHelper->EnableLogComponents();

    lteHelper->SetSchedulerType (c->getString("lte_scheduler"));

    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);


    NodeContainer pgwNodeContainer;
    NetDeviceContainer pgwDevices;
    Ipv4InterfaceContainer pgwInterfaces;
    Ipv4StaticRoutingHelper staticRouting;

    pgwNodeContainer.Add(epcHelper->GetPgwNode());
    pgwNodeContainer.Add(serverNodes);

    // Create Connection between PGW Node and all Server Nodes (Star Topology)
    Ipv4AddressHelper address;
    Ipv4Address ipv4Network("176.0.0.0");
    Ipv4Mask ipv4Mask("255.255.255.252");
    address.SetBase(ipv4Network, ipv4Mask);
    for (uint32_t i = 0; i < numberOfServer; i++) {
        NodeContainer tmpPgwNodeContainer;
        PointToPointHelper pgwP2pHelper;
        NetDeviceContainer tmpPgwDevices;
        Ipv4InterfaceContainer tmpPgwInterfaces;

        tmpPgwNodeContainer.Add(pgwNodeContainer.Get(0));
        tmpPgwNodeContainer.Add(pgwNodeContainer.Get(i + 1));
        StringValue datarate("1Gbps");
        StringValue delay("2ms");
        UintegerValue mtu(9000);

        Network_Utilities::createPointToPointTopology(&tmpPgwNodeContainer, &pgwP2pHelper, &tmpPgwDevices, &datarate,
                &delay, &mtu);

        tmpPgwInterfaces = address.Assign(tmpPgwDevices);

        if (i == 0) {
            pgwDevices.Add(tmpPgwDevices.Get(0));
            pgwInterfaces.Add(tmpPgwInterfaces.Get(0));
        }
        pgwDevices.Add(tmpPgwDevices.Get(1));
        pgwInterfaces.Add(tmpPgwInterfaces.Get(1));

        ipv4Network = address.NewNetwork();
        address.SetBase(ipv4Network, ipv4Mask);
    }

    // Home position
    ObjectFactory homePositionhelper;
    homePositionhelper.SetTypeId("ns3::RandomDiscPositionAllocator");
    std::string s = "ns3::" + c->getString("random_function") + "RandomVariable[Min=0|Max=" + Helper::toString(maxDistanceToEnbNode) + "]";
    //homePositionhelper.Set("Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=360]"));
    homePositionhelper.Set("Rho", StringValue(s));
    //homePositionhelper.Set("Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=360]"));
    s = "ns3::UniformRandomVariable[Min="+ Helper::toString(minAngle) + "|Max=" + Helper::toString(maxAngle) + "]";
    //homePositionhelper.Set("Theta", StringValue("ns3::UniformRandomVariable[Min=0|Max=10]"));
    homePositionhelper.Set("Theta", StringValue(s)); //Min=0|Max=10
    homePositionhelper.Set("Y", DoubleValue(50));

    PacketDelay* pd = PacketDelay::Get();

    std::cout << "numberOfEnbNodes: " << numberOfEnbNodes << std::endl;
    for (uint32_t i = 0; i < numberOfEnbNodes; i++) {
        // enbNode Position
        Ptr<ConstantPositionMobilityModel> enbNodePosition = CreateObject<ConstantPositionMobilityModel>();
        enbNodePosition->SetPosition(Vector((i + 1) * distanceBetweenEnbNodes, 100, 0));
        enbNodes.Get(i)->AggregateObject(enbNodePosition);
        homePositionhelper.Set("X", DoubleValue((i + 1) * distanceBetweenEnbNodes));
        Ptr<PositionAllocator> homePositionAllocator = homePositionhelper.Create<PositionAllocator>();
        for (uint32_t j = i * nodesPerEnbNode; j < i * nodesPerEnbNode + nodesPerEnbNode && j < numberOfHomes; j++) {
            // position
            Ptr<ConstantPositionMobilityModel> homePosition = CreateObject<ConstantPositionMobilityModel>();
            homePosition->SetPosition(homePositionAllocator->GetNext());
            homeNodes.Get(j)->AggregateObject(homePosition);
        }

        pd->addNode(enbNodes.Get(i)->GetId());
    }
    pd->addNode(pgwNodeContainer.Get(0)->GetId());

//    Ptr<ListPositionAllocator> positionAllocator = CreateObject<ListPositionAllocator>();
//    for (uint32_t i = 0; i < totalEnbNodes; i++) {
//        positionAllocator->Add(Vector(distanceBetweenEnbNodes * (i+1), 100, 0));
//    }

//    MobilityHelper mobility;
//    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
//    mobility.SetPositionAllocator(positionAllocator);
//    mobility.Install(enbNodes);
//    mobility.Install(homeNodes);


    lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (MyConfig::Get()->getInt("download")));
    lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (MyConfig::Get()->getInt("upload")));
    // Install LTE Devices to the nodes
    NetDeviceContainer enbDevices = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer homeDevices = lteHelper->InstallUeDevice(homeNodes);

    Ipv4InterfaceContainer ueInterfaces;
    ueInterfaces = epcHelper->AssignUeIpv4Address(homeDevices);

    // Attach one homeNode per eNodeB | side effect: the default EPS bearer will be activated
    lteHelper->AttachToClosestEnb(homeDevices, enbDevices);

//    for (uint32_t i = 0; i < numberOfHomes; i++) {
//        lteHelper->Attach(homeDevices.Get(i), enbDevices.Get(i / nodesPerEnbNode));
//    }

    // Assign IP address to UEs, and set default route to pgw
    for (uint32_t i = 0; i < homeNodes.GetN(); ++i) {
        Ptr<Node> homeNode = homeNodes.Get(i);
        // Set the default gateway for the UE
        Ptr<Ipv4StaticRouting> homeStaticRouting = staticRouting.GetStaticRouting(homeNode->GetObject<Ipv4>());

        // Add all other Homes address to this home
        for (uint j = 0; j < homeNodes.GetN(); j++) {
            if (i == j) continue;
            Ptr<Node> tmpHomeNode = homeNodes.Get(j);
            Ptr<Ipv4> tmpHomeNodeIpv4 = tmpHomeNode->GetObject<Ipv4>();
            Ipv4InterfaceAddress tmpHomeNodeIpv4Address = tmpHomeNodeIpv4->GetAddress(1, 0);

            homeStaticRouting->AddHostRouteTo(tmpHomeNodeIpv4Address.GetLocal(), 1);
        }

        homeStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
    }

    // set routing Information between PGW Node and corresponding Server Node
    for (uint32_t i = 0; i < numberOfServer; i++) {
        Ptr<Ipv4StaticRouting> serverStaticRouting = staticRouting.GetStaticRouting(
                pgwNodeContainer.Get(i + 1)->GetObject<Ipv4>());
        Ptr<Ipv4> ipv4 = pgwNodeContainer.Get(0)->GetObject<Ipv4>();
        Ipv4InterfaceAddress pgwIpv4Address = ipv4->GetAddress(i + 2, 0);
//        std::cout << "IP: " << pgwIpv4Address.GetLocal() << std::endl;
        serverStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), pgwIpv4Address.GetLocal(),
                1);

        for (uint32_t j = 0; j < numberOfServer; j++) {
            if (j == i) continue;
            Ptr<Ipv4> serverIpv4 = pgwNodeContainer.Get(j + 1)->GetObject<Ipv4>();
            Ipv4InterfaceAddress serverIpv4Address = serverIpv4->GetAddress(1, 0);
            Ipv4InterfaceAddress tmpPgwIpv4Address = ipv4->GetAddress(j + 2, 0);
//            std::cout << "Server " << j <<" IP: " << serverIpv4Address.GetLocal();
//            std::cout << "| PGW " << j << " IP: " << tmpPgwIpv4Address.GetLocal() << std::endl;

            serverStaticRouting->AddHostRouteTo(serverIpv4Address.GetLocal(), tmpPgwIpv4Address.GetLocal(), 1);
        }
    }

//    serverNodes = NodeContainer();
//    for(uint32_t i=1; i<pgwNodeContainer.GetN(); i++) {
//        serverNodes.Add(pgwNodeContainer.Get(i));
//    }

    network_assignHomes(apps, &homeNodes, false, &serverNodes);

    // register mac traces and background traffic
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

#endif
