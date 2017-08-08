#ifndef NS_3_22_SRC_CITYGRID_MODULES_NETWORK_NETWORK_PLC_H_
#define NS_3_22_SRC_CITYGRID_MODULES_NETWORK_NETWORK_PLC_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/application-container.h"
#include "ns3/plc.h"
#include "ns3/Helper.h"
#include "ns3/Network_Utilities.h"


#include <vector>

namespace ns3 {

//void ReceivedACK(void) {
//    NS_LOG_UNCOND(Simulator::Now() << ": ACK received!");
//}

static void PLC_create(std::vector<void*>* params) {

//    LogComponentEnable("PLC_Mac", LOG_LEVEL_INFO);
//    Packet::EnablePrinting();

    ApplicationContainer* apps = (ApplicationContainer*) params->at(0);
    MyConfig* c = MyConfig::Get();

    Scenario* scenario = Scenario::Get();
    int numberOfComposites = 0;
    std::vector<ClusterConfig*>* clusterConfigs = scenario->getClusters();
    for (uint c = 0; c < clusterConfigs->size(); c++) {
        numberOfComposites += clusterConfigs->at(c)->getComposites()->size();
    }

    uint32_t numberOfNwCluster = c->getInt("plc_network_cluster");
    uint32_t numberOfHomes = c->getInt("total_homes") + numberOfComposites;
    uint32_t numberOfServer = c->getInt("clusters") + 1; //(c->getInt("clusters") == 1 ? 0 : 1);
    uint32_t homesPerNwCluster = numberOfHomes / numberOfNwCluster;
    uint32_t homesPerNwClusterRest = numberOfHomes % numberOfNwCluster;
    uint32_t branchSize = c->getInt("plc_branch_size");
    uint32_t numberOfMainJunctionNodes = homesPerNwCluster / branchSize + (homesPerNwCluster % branchSize == 0 ? 0 : 1);
    uint32_t gridFrequency = c->getInt("plc_grid_frequency");
    double minFrequency = c->getDouble("plc_frequency_min");
    double maxFrequency = c->getDouble("plc_frequency_max");
    size_t numberOfBands = c->getInt("plc_number_of_bands");
    uint32_t symbolLength = c->getInt("plc_symbol_length");
    double transmitPowerSpectralDensity = c->getDouble("plc_transmit_power_spectral_density");
    uint32_t nodeShuntImpedanz = c->getInt("plc_node_shunt_impedanz");
    uint32_t startpositionX = c->getInt("plc_startposition_x");
    uint32_t startpositionY = c->getInt("plc_startposition_y");
    uint32_t dist2MainJunctionNodesMin = c->getInt("plc_dist_betw_two_main_junction_nodes_min");
    uint32_t dist2MainJunctionNodesMax = c->getInt("plc_dist_betw_two_main_junction_nodes_max");
    if(dist2MainJunctionNodesMin>dist2MainJunctionNodesMax)
    	dist2MainJunctionNodesMax = dist2MainJunctionNodesMin;
    uint32_t dist2JunctionNodesMin = c->getInt("plc_dist_betw_two_junction_nodes_min");
    uint32_t dist2JunctionNodesMax = c->getInt("plc_dist_betw_two_junction_nodes_max");
    if(dist2JunctionNodesMin>dist2JunctionNodesMax)
    	dist2JunctionNodesMax = dist2JunctionNodesMin;
    uint32_t distJunctionHomeMin = c->getInt("plc_dist_betw_junction_node_and_home_node_min");
    uint32_t distJunctionHomeMax = c->getInt("plc_dist_betw_junction_node_and_home_node_max");
    if(distJunctionHomeMin>distJunctionHomeMax)
    	distJunctionHomeMax = distJunctionHomeMin;
    bool plcWorstCasteBackgroundNoise = c->getBool("plc_worst_case_background_noise");
    int headerModulation = c->getInt("pcl_header_modulation_and_coding_type");
    int payloadModulation = c->getInt("plc_payload_modulation_and_coding_type");

    NS_ASSERT_MSG(distJunctionHomeMax * 2 < dist2MainJunctionNodesMin,
            "Max. distance between 2 Junction Nodes has to be 2x smaller than min. distance between 2 Main Junction Nodes! Look into the network_plc.json.");

    NS_ASSERT_MSG(headerModulation >= BPSK_1_4 && headerModulation <= QAM64_16_21,
                "wrong value for 'header_modulation_and_coding_type'. Look into the network_plc.json.");

    NS_ASSERT_MSG(payloadModulation >= BPSK_1_4 && payloadModulation <= QAM64_RATELESS,
            "wrong value for 'payload_modulation_and_coding_type'. Look into the network_plc.json.");

    // network
    PLC_Time::SetTimeModel(gridFrequency, MicroSeconds(symbolLength)); // main frequency, symbol length (defines time granularity)

    // Define spectrum model
    PLC_SpectrumModelHelper plcSmHelper;
    Ptr<const SpectrumModel> plcSm;
    plcSm = plcSmHelper.GetSpectrumModel(minFrequency, maxFrequency, numberOfBands);

    // Define transmit power spectral density
    Ptr<SpectrumValue> txPsd = Create<SpectrumValue>(plcSm);
    (*txPsd) = transmitPowerSpectralDensity;

    // Create cable types
    Ptr<PLC_Cable> plcMainCable = CreateObject<PLC_NAYY150SE_Cable>(plcSm);
    Ptr<PLC_Cable> plcHouseCable = CreateObject<PLC_NAYY50SE_Cable>(plcSm);

    // Node shunt impedance
    Ptr<PLC_ConstImpedance> plcShuntImpedance = Create<PLC_ConstImpedance>(plcSm, PLC_Value(nodeShuntImpedanz, 0));

    PLC_NodeList plcHomeNodes;
    PLC_NodeList plcGatewayNodes;
    PLC_NodeList::iterator plcHomeIt;
    PLC_NodeList::iterator plcGatewayIt;

    for (uint i = 0; i < numberOfNwCluster; i++) {
        plcGatewayNodes.push_back(CreateObject<PLC_Node>());
    }
    for (uint i = 0; i < numberOfHomes; i++) {
        plcHomeNodes.push_back(CreateObject<PLC_Node>());
    }
    plcHomeIt = plcHomeNodes.begin();
    plcGatewayIt = plcGatewayNodes.begin();

    Random* random = Random::Get();
    int x, y, z = 0;
    uint homesDone = 0;
    std::vector<PLC_NodeList> plcJunctionNodesVector;
    std::vector<PLC_NodeList> plcHomeNodesVector;
    std::vector<PLC_NodeList> plcNodesVector;

    for (uint i = 0; i < numberOfNwCluster; i++) {
        PLC_NodeList plcClusterNodes;
        PLC_NodeList plcClusterJunctionNodes;
        PLC_NodeList plcClusterHomeNodes;
        Ptr<PLC_Node> plcGatewayNode = *plcGatewayIt;
        plcGatewayNode->SetImpedance(plcShuntImpedance);
        
        x = startpositionX;
        y = startpositionY + 0.5 * dist2JunctionNodesMax * branchSize * (i + 1) + startpositionY * (i == 0 ? 0 : 1);
        z = 0;
        
        plcGatewayNode->SetPosition(x, y, z);
        plcGatewayNode->SetName("GatewayNode_" + Helper::toString(i));

        uint homesToDo = homesPerNwCluster;
        if (homesPerNwClusterRest>0) {
        	homesToDo++;
        	homesPerNwClusterRest--;
        }
        Ptr<PLC_Node> lastMainJunctionNode = plcGatewayNode;
        for (uint j = 0; j < numberOfMainJunctionNodes; j++) {
            uint distanceMainJunctionNodes = random->get(dist2MainJunctionNodesMin, dist2MainJunctionNodesMax);
            x = x + distanceMainJunctionNodes + distanceMainJunctionNodes * 0.2;

            Ptr<PLC_Node> curMainJunctionNode = CreateObject<PLC_Node>();
            curMainJunctionNode->SetImpedance(plcShuntImpedance);
            curMainJunctionNode->SetPosition(x, y, z);
            curMainJunctionNode->SetName("MainJunctionNode_" + Helper::toString(i) + "_" + Helper::toString(j));

            Ptr<PLC_Line> line = CreateObject<PLC_Line>(plcMainCable, lastMainJunctionNode, curMainJunctionNode);
            if(line->GetLength() < 0.00001) {
                throw ("Cable lenght is 0 betw. Main Junction Nodes in Network_PLC.h");
            }
            plcClusterJunctionNodes.push_back(curMainJunctionNode);
            lastMainJunctionNode = curMainJunctionNode;

            Ptr<PLC_Node> lastJunctionNode = curMainJunctionNode;
            uint k=0;
            int junctionX, junctionY, junctionZ = 0;
            junctionX = x;
            junctionY = y;
            while(k < branchSize) {
                if (homesToDo == 0 || homesDone >= numberOfHomes) {
                    break;
                }

                uint distanceJunctionToJunction = random->get(dist2JunctionNodesMin, dist2JunctionNodesMax);
//                x = startpositionX + (distanceMainJunctionNodes + (distanceMainJunctionNodes * 0.2)) * (j + 1);
//                y = startpositionY * branchSize * (i + 1) - (k + 1) * distanceJunctionToJunction;
                junctionY = junctionY - distanceJunctionToJunction;

                Ptr<PLC_Node> curJunctionNode = CreateObject<PLC_Node>();
                curJunctionNode->SetImpedance(plcShuntImpedance);
                curJunctionNode->SetPosition(junctionX, junctionY, junctionZ);
                curJunctionNode->SetName(
                        "JunctionNode_" + Helper::toString(i) + "_" + Helper::toString(j) + "_" + Helper::toString(k));

                line = CreateObject<PLC_Line>(plcMainCable, lastJunctionNode, curJunctionNode);
                if(line->GetLength() < 0.00001) {
                    throw ("Cable lenght is 0 betw. 2 Nodes in Network_PLC.h");
                }
                plcClusterJunctionNodes.push_back(curJunctionNode);
                lastJunctionNode = curJunctionNode;

                for(uint f=0; f<2; f++) {
                    if (homesToDo == 0 || homesDone >= numberOfHomes || k >= branchSize) {
                        break;
                    }

                    Ptr<PLC_Node> curHomeNode = *plcHomeIt;
                    curHomeNode->SetImpedance(plcShuntImpedance);
                    int homeX, homeY, homeZ = 0;
                    uint distanceJunctionToHome = random->get(distJunctionHomeMin, distJunctionHomeMax);

                    if(f==0) homeX = junctionX - distanceJunctionToHome;
                    else homeX = junctionX + distanceJunctionToHome;
                    homeY = junctionY;

//                    x += distanceJunctionToJunction;
    //                x = startpositionX + distBetw2MainJunctionNodes * (j + 1) + distanceJunctionToHome;
                    curHomeNode->SetPosition(homeX, homeY, homeZ);
                    curHomeNode->SetName(
                            "HomeNode_" + Helper::toString(i) + "_" + Helper::toString(j) + "_" + Helper::toString(k));

                    line = CreateObject<PLC_Line>(plcHouseCable, curJunctionNode, curHomeNode);
                    if(line->GetLength() < 0.00001) {
                        throw ("Cable lenght is 0 betw. Node & Home in Network_PLC.h");
                    }
                    plcClusterHomeNodes.push_back(curHomeNode);

                    plcHomeIt++;
                    homesDone++;
                    homesToDo--;
                    k++;
                }
            }
        }

        plcHomeNodesVector.push_back(plcClusterHomeNodes);
        plcJunctionNodesVector.push_back(plcClusterJunctionNodes);

        plcClusterNodes.insert(plcClusterNodes.end(), plcClusterHomeNodes.begin(), plcClusterHomeNodes.end());
        plcClusterNodes.push_back(plcGatewayNode);
        plcClusterNodes.insert(plcClusterNodes.end(), plcClusterJunctionNodes.begin(), plcClusterJunctionNodes.end());

        plcNodesVector.push_back(plcClusterNodes);

        plcGatewayIt++;
    }

    // Define background noise
    Ptr<SpectrumValue> plcNoiseFloor;
    if (plcWorstCasteBackgroundNoise) {
        plcNoiseFloor = CreateWorstCaseBgNoise(plcSm)->GetNoisePsd();
    } else {
        plcNoiseFloor = CreateBestCaseBgNoise(plcSm)->GetNoisePsd();
    }

    // Set up channel
    std::vector<NodeContainer> ns3HomeNodesVector;
    NodeContainer ns3HomeNodes;
    NodeContainer ns3GatewayNodes;

    InternetStackHelper stack;
    Ipv4AddressHelper addressHelper;
    Mac48Address plcMacAddress("00:00:00:00:00:00");
    Ipv4Address plcIpv4Address("20.0.0.0");
    Ipv4Mask plcIpv4Mask("255.255.0.0");
    addressHelper.SetBase(plcIpv4Address, plcIpv4Mask);

    std::vector<Ptr<PLC_Channel> > plcChannelVector;
    std::vector<NetDeviceContainer> ns3HomeDevicesVector;
    std::vector<Ptr<NetDevice> > ns3GatewayDeviceVector;
    std::vector<Ipv4InterfaceContainer> ns3HomeNodesInterfaceVector;
    std::vector<Ipv4InterfaceContainer> ns3GatewayInterfaceVector;
    for (uint i = 0; i < numberOfNwCluster; i++) {
        PLC_NodeList plcNodes = plcNodesVector.at(i);
        PLC_ChannelHelper plcChannelHelper(plcSm);

        plcChannelHelper.Install(plcNodes);
        Ptr<PLC_Channel> plcChannel = plcChannelHelper.GetChannel();

        // Create net devices
        PLC_NetDeviceHelper plcDeviceHelper(plcSm, txPsd, plcNodes);
        plcDeviceHelper.SetNoiseFloor(plcNoiseFloor);
        ModulationAndCodingScheme hmcs = ModulationAndCodingScheme(
                static_cast<ModulationAndCodingType>(headerModulation), 0);
        ModulationAndCodingScheme pmcs = ModulationAndCodingScheme(
                static_cast<ModulationAndCodingType>(payloadModulation), 0);

        plcDeviceHelper.SetHeaderModulationAndCodingScheme(hmcs);
        plcDeviceHelper.SetPayloadModulationAndCodingScheme(pmcs);
        plcDeviceHelper.Setup(&plcMacAddress);

        // Calculate channels
        plcChannel->InitTransmissionChannels();
        plcChannel->CalcTransmissionChannels();
        plcChannelVector.push_back(plcChannel);

        NodeContainer ns3ClusterNodes = plcDeviceHelper.GetNS3Nodes();
        NetDeviceContainer ns3ClusterDevices = plcDeviceHelper.GetNetDevices();
        NodeContainer ns3ClusterHomeNodes;
        NetDeviceContainer ns3ClusterHomeDevices;
        Ptr<NetDevice> ns3ClusterGatewayDevice = plcDeviceHelper.GetDevice("GatewayNode_" + Helper::toString(i));
        Ptr<Node> ns3ClusterGatewayNode = ns3ClusterGatewayDevice->GetNode();

        uint clusterHomeNodesNumber = plcHomeNodesVector.at(i).size();
        for (uint j = 0; j < clusterHomeNodesNumber; j++) {
            ns3ClusterHomeNodes.Add(ns3ClusterNodes.Get(j));
            ns3ClusterHomeDevices.Add(ns3ClusterDevices.Get(j));
        }

        ns3HomeNodesVector.push_back(ns3ClusterHomeNodes);
        ns3HomeNodes.Add(ns3ClusterHomeNodes);
        ns3GatewayNodes.Add(ns3ClusterGatewayNode);

        stack.Install(ns3ClusterHomeNodes);
        stack.Install(ns3ClusterGatewayNode);

        Ipv4InterfaceContainer ns3ClusterHomeInterfaces;
        Ipv4InterfaceContainer ns3ClusterGatewayInterface;

        ns3ClusterGatewayInterface = addressHelper.Assign(ns3ClusterGatewayDevice);
        ns3GatewayInterfaceVector.push_back(ns3ClusterGatewayInterface);
        ns3ClusterHomeInterfaces = addressHelper.Assign(ns3ClusterHomeDevices);
        ns3HomeNodesInterfaceVector.push_back(ns3ClusterHomeInterfaces);

        addressHelper.SetBase(addressHelper.NewNetwork(), plcIpv4Mask);
    }

    //
    // Connection between Network Cluster & Server
    //
    Ptr<Node> rootNode = CreateObject<Node>();
    stack.Install(rootNode);

    NodeContainer rootToGatewayNodes;
    NodeContainer rootToServerNodes;
    NetDeviceContainer rootToGatewayDevices;
    NetDeviceContainer rootToServerDevices;
    Ipv4InterfaceContainer rootToGatewayInterfaces;
    Ipv4InterfaceContainer rootToServerInterfaces;
    Ipv4StaticRoutingHelper staticRouting;
    NodeContainer serverNodes;

    serverNodes.Create(numberOfServer);
    stack.Install(serverNodes);

    rootToServerNodes.Add(rootNode);
    rootToServerNodes.Add(serverNodes);

    rootToGatewayNodes.Add(rootNode);
    rootToGatewayNodes.Add(ns3GatewayNodes);

    // Create Connection between Root Node and all Server Nodes (Star Topology)
    Ipv4Address ipv4RootToGateway("80.0.0.0");
    Ipv4Address ipv4RootToServer("176.0.0.0");
    Ipv4Mask ipv4MaskRootToGateway("255.255.255.252");
    Ipv4Mask ipv4MaskRootToServer("255.255.255.252");

    Network_Utilities::createStarTopology(&rootToGatewayNodes, &rootToGatewayDevices, &rootToGatewayInterfaces,
            ipv4RootToGateway, ipv4MaskRootToGateway);
    Network_Utilities:: createStarTopology(&rootToServerNodes, &rootToServerDevices, &rootToServerInterfaces,
            ipv4RootToServer, ipv4MaskRootToServer);

    //
    // routing
    //

    #ifndef TURBO
        std::stringstream ss;

        ss << "Root Node:";
        for(uint i=1; i < rootToGatewayNodes.GetN() + rootToServerNodes.GetN() - 1; i++) {
            ss << " " << rootToGatewayNodes.Get(0)->GetObject<Ipv4>()->GetAddress(i, 0).GetLocal();
        }
        Log::i("PLC-Network", ss.str());

        ss.str(std::string());
        for(uint i=1; i < rootToGatewayNodes.GetN(); i++) {
                    ss << "Gateway " << i << ": " << rootToGatewayNodes.Get(i)->GetObject<Ipv4>()->GetAddress(2, 0)
                            .GetLocal() << "; ";
                }
        Log::i("PLC-Network", ss.str());

        ss.str(std::string());
        for(uint i=1; i < rootToServerNodes.GetN(); i++) {
                    ss << "Server " << i << ": " << rootToServerNodes.Get(i)->GetObject<Ipv4>()->GetAddress(1, 0)
                            .GetLocal() << "; ";
                }
        Log::i("PLC-Network", ss.str());
    #endif

    //set routing Information between Root Node and corresponding PLC Gateway
    for (uint i = 1; i < rootToGatewayNodes.GetN(); i++) {
        Ptr<Ipv4StaticRouting> gatewaySaticRouting = staticRouting.GetStaticRouting(
                rootToGatewayNodes.Get(i)->GetObject<Ipv4>());
        Ptr<Ipv4> ipv4Root = rootToGatewayNodes.Get(0)->GetObject<Ipv4>();
        Ipv4InterfaceAddress rootToGatewayInterfaceAddress = ipv4Root->GetAddress(i, 0);


        #ifndef TURBO
            ss.str(std::string());
            ss << "Gateway " << i;
            Log::i("PLC-Network", ss.str());
        #endif

        Ipv4AddressHelper tmpAddress(ipv4RootToServer, ipv4MaskRootToServer);
        Ipv4Address tmpIpv4Address(ipv4RootToServer);
        for (uint j = 0; j < numberOfServer; j++) {

            #ifndef TURBO
                ss.str(std::string());
                ss << "Dest. Network: " << tmpIpv4Address << " find at next Hop: "
                        << rootToGatewayInterfaceAddress.GetLocal();
                Log::i("PLC-Network", ss.str());
            #endif

            gatewaySaticRouting->AddNetworkRouteTo(tmpIpv4Address, ipv4MaskRootToServer,
                    rootToGatewayInterfaceAddress.GetLocal(), 2);

            tmpIpv4Address = tmpAddress.NewNetwork();
        }

        addressHelper.SetBase(plcIpv4Address, plcIpv4Mask);
        Ipv4Address tmpipv4NwPlc(plcIpv4Address);
        for (uint j = 1; j < rootToGatewayNodes.GetN(); j++) {
            if (j == i) {
                tmpipv4NwPlc = addressHelper.NewNetwork();
                addressHelper.SetBase(tmpipv4NwPlc, plcIpv4Mask);
                continue;
            }
            Ipv4InterfaceAddress rootIpv4InterfaceAddress = ipv4Root->GetAddress(i, 0);

            #ifndef TURBO
                ss.str(std::string());
                ss << "Corresponding Gateway " << j << " with Dest. Network IP: " << tmpipv4NwPlc
                        << " find at next Hop: " << rootIpv4InterfaceAddress.GetLocal();
                Log::i("PLC-Network", ss.str());

            #endif

            gatewaySaticRouting->AddNetworkRouteTo(tmpipv4NwPlc, plcIpv4Mask, rootIpv4InterfaceAddress.GetLocal(), 2);

            tmpipv4NwPlc = addressHelper.NewNetwork();
            addressHelper.SetBase(tmpipv4NwPlc, plcIpv4Mask);
        }
        gatewaySaticRouting->SetDefaultRoute(rootToGatewayInterfaceAddress.GetLocal(),2);
    }

    // set routing Information between Root Node and corresponding Server Node
    for (uint32_t i = 0; i < numberOfServer; i++) {
        Ptr<Ipv4StaticRouting> serverStaticRouting = staticRouting.GetStaticRouting(
                rootToServerNodes.Get(i + 1)->GetObject<Ipv4>());
        Ptr<Ipv4> ipv4Root = rootToServerNodes.Get(0)->GetObject<Ipv4>();
        Ipv4InterfaceAddress rootToServerInterfaceAddress = ipv4Root->GetAddress(
                i + 1 + (rootToGatewayNodes.GetN() - 1), 0);

        #ifndef TURBO
            ss.str(std::string());
            ss << "Server " << i;
            Log::i("PLC-Network", ss.str());
            ss.str(std::string());
            ss << "Dest. Network " << plcIpv4Address << " find at next Hop: "
                    << rootToServerInterfaceAddress.GetLocal();
            Log::i("PLC-Network", ss.str());
        #endif

        serverStaticRouting->AddNetworkRouteTo(plcIpv4Address, Ipv4Mask("255.0.0.0"),
                rootToServerInterfaceAddress.GetLocal(), 1);

        for (uint32_t j = 0; j < numberOfServer; j++) {
            if (j == i) {
                continue;
            }
            Ptr<Ipv4> tmpServerIpv4 = rootToServerNodes.Get(j + 1)->GetObject<Ipv4>();
            Ipv4InterfaceAddress serverInterfaceAddress = tmpServerIpv4->GetAddress(1, 0);
            Ipv4InterfaceAddress tmpRootToSerevrInterfaceAddress = ipv4Root->GetAddress(
                    i + 1 + (rootToGatewayNodes.GetN() - 1), 0);

            #ifndef TURBO
                ss.str(std::string());
                ss << "Corresponding Server " << j << " with IP: " << serverInterfaceAddress.GetLocal()
                        << " find at next Hop: " << tmpRootToSerevrInterfaceAddress.GetLocal();
                Log::i("PLC-Network", ss.str());
            #endif

            serverStaticRouting->AddHostRouteTo(serverInterfaceAddress.GetLocal(),
                    tmpRootToSerevrInterfaceAddress.GetLocal(), 1);
        }
    }

    // set routing information to root node
    addressHelper.SetBase(plcIpv4Address, plcIpv4Mask);
    Ipv4Address tmpIpv4NwPlc(plcIpv4Address);
    Ptr<Ipv4StaticRouting> rootStaticRouting = staticRouting.GetStaticRouting(rootNode->GetObject<Ipv4>());

    #ifndef TURBO
        ss.str(std::string());
        ss << "Root Node";
        Log::i("PLC-Network", ss.str());
    #endif

    for (uint i = 0; i < numberOfNwCluster; i++) {
        Ptr<Ipv4> tmpGwIpv4 = rootToGatewayNodes.Get(i + 1)->GetObject<Ipv4>();
        Ipv4Address tmpGwAddress = tmpGwIpv4->GetAddress(2, 0).GetLocal();

        #ifndef TURBO
            ss.str(std::string());
            ss << "Gateway " << i << " with dest. Network IP: " << tmpIpv4NwPlc << " fint at next Hop: "
                    << tmpGwAddress;
            Log::i("PLC-Network", ss.str());
        #endif

        rootStaticRouting->AddNetworkRouteTo(tmpIpv4NwPlc, plcIpv4Mask, tmpGwAddress, i + 1);

        tmpIpv4NwPlc = addressHelper.NewNetwork();
        addressHelper.SetBase(tmpIpv4NwPlc, plcIpv4Mask);
    }

    // set default route to home nodes
    for(uint i=0; i<numberOfNwCluster; i++) {
        NodeContainer tmpHomes = ns3HomeNodesVector.at(i);
        for(uint j=0; j<tmpHomes.GetN(); j++) {
            Ptr<Node> tmpNode = tmpHomes.Get(j);
            Ptr<Node> tmpGw = rootToGatewayNodes.Get(i+1);
            Ptr<Ipv4> tmpGwIpv4 = tmpGw->GetObject<Ipv4>();
            Ipv4Address tmpDefaultGwAddress = tmpGwIpv4->GetAddress(1,0).GetLocal();

            #ifndef TURBO
                ss.str(std::string());
                ss << "Home " << i << "," << j <<  " with Id " <<  tmpNode->GetId() << " | Default Route to "
                        << tmpDefaultGwAddress;
                Log::i("PLC-Network", ss.str());
            #endif

            Ptr<Ipv4StaticRouting> homeStaticRouting = staticRouting.GetStaticRouting(tmpNode->GetObject<Ipv4>());
            homeStaticRouting->SetDefaultRoute(tmpDefaultGwAddress, 1, 0);
        }
    }

//    stack.EnablePcapIpv4All("pcap/plc");

//    PcapHelperForDevice pcap;
//    pcap.EnablePcap("test", ns3HomeNodes);
//    PointToPointHelper p2p;
//    p2p.EnablePcap("test", ns3HomeNodes);
    network_assignHomes(apps, &ns3HomeNodes, true, &serverNodes);

    //
    // register mac traces
    //
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
} /* namespace ns3 */

#endif
