#ifndef NS_3_22_SRC_SIENA_MODULES_NETWORK_NETWORK_UTILITIES_H_
#define NS_3_22_SRC_SIENA_MODULES_NETWORK_NETWORK_UTILITIES_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/brite-module.h"

using namespace ns3;

class Network_Utilities {
    public:
        static void generateDSLTopology(InternetStackHelper* stack, BriteTopologyHelper* bth) {
            // generate BRITE DSL Topology
            // Invoke the BriteTopologyHelper and pass in a BRITE
            // configuration file and a seed file. This will use
            // BRITE to build a graph from which we can build the ns-3 topology
            bth->AssignStreams(3);

            Ipv4ListRoutingHelper listRouting;
            Ipv4StaticRoutingHelper staticRouting;
            Ipv4GlobalRoutingHelper globalRouting;
            //    Ipv4NixVectorHelper nixRouting;

            listRouting.Add(staticRouting, 0);
            listRouting.Add(globalRouting, 10);
            stack->SetRoutingHelper(listRouting);

            Ipv4AddressHelper address;
            address.SetBase("10.0.0.0", "255.255.255.252");

            bth->BuildBriteTopology(*stack);
            bth->AssignIpv4Addresses(address);

            return;
        }

        static void createPointToPointTopology(NodeContainer* p2pNodes, PointToPointHelper* pointToPointHelper,
                NetDeviceContainer* p2pDevices, StringValue *datarate, StringValue *delay, UintegerValue *mtu = NULL) {

            //create P2P Helper & set attributes to P2P Helper
            pointToPointHelper->SetDeviceAttribute("DataRate", *datarate);
            pointToPointHelper->SetChannelAttribute("Delay", *delay);
            if (mtu) {
                pointToPointHelper->SetDeviceAttribute("Mtu", *mtu);
            }

            // create Net Devices for a P2P Connection between the 2 Nodes
            *p2pDevices = pointToPointHelper->Install(*p2pNodes);

            return;
        }

        static void createStarTopology(NodeContainer* nodeContainer, NetDeviceContainer* deviceContainer,
                Ipv4InterfaceContainer* interfaceContainer, Ipv4Address ipv4, Ipv4Mask mask) {

            Ipv4AddressHelper address;
            address.SetBase(ipv4, mask);
            for (uint32_t i = 1; i < nodeContainer->GetN(); i++) {
                NodeContainer tmpNodeContainer;
                PointToPointHelper tmpP2pHelper;
                NetDeviceContainer tmpDevices;
                Ipv4InterfaceContainer tmpInterfaces;

                tmpNodeContainer.Add(nodeContainer->Get(0));
                tmpNodeContainer.Add(nodeContainer->Get(i));
                StringValue datarate("1Gbps");
                StringValue delay("2ms");
                UintegerValue mtu(65000);

                createPointToPointTopology(&tmpNodeContainer, &tmpP2pHelper, &tmpDevices, &datarate, &delay,
                        &mtu);

                tmpInterfaces = address.Assign(tmpDevices);

                if (i == 1) {
                    deviceContainer->Add(tmpDevices.Get(0));
                    interfaceContainer->Add(tmpInterfaces.Get(0));
                }
                deviceContainer->Add(tmpDevices.Get(1));
                interfaceContainer->Add(tmpInterfaces.Get(1));

                address.SetBase(address.NewNetwork(), mask);
            }
        }
};

#endif
