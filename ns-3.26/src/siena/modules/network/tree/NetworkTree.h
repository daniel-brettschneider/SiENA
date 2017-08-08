#ifndef NETWORKTREE_H_
#define NETWORKTREE_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include <string>
#include <vector>
#include "ns3/Random.h"
#include "ns3/NetworkTreeNode.h"
#include "ns3/MyConfig.h"
#include "ns3/TraceHelper.h"

namespace ns3 {

class NetworkTree {

private:
	NetworkTreeNode* root;
	std::vector<NetworkTreeNode*> leaves;
	std::vector<std::vector<NetworkTreeNode*>*> routers;
	PointToPointHelper p2pLeaf;
	PointToPointHelper p2pRouter;
	InternetStackHelper internet;
	MyConfig* config;

public:
	NetworkTree();
	virtual ~NetworkTree();
	NodeContainer create(int nodes);
	NodeContainer getLeaveNodes();
	void assignAddresses(Ipv4Address rootNetwork, Ipv4Mask rootMask);
	Ptr<Node> getRootNode() { return root->getNode(); }

private:
	void createLeafBranches(int nodes);
	void createRouterBranches(int layer);
	void createRoot(int layer);
	void assignAddresses(NetworkTreeNode* root, Ipv4Address rootNetwork, Ipv4Mask rootMask, int layer = 1);
	Ipv4Mask splitNetwork(Ipv4Mask mask, int subnets);

};

}

#endif
