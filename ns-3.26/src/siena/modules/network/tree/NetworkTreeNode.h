#ifndef NETWORKTREENODE_H_
#define NETWORKTREENODE_H_

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include <vector>

namespace ns3 {

class NetworkTreeNode {

private:
	Ptr<Node> node;
	std::vector<NetworkTreeNode*> children;
	Ptr<NetDevice> netDeviceRoot;
	std::vector<Ptr<NetDevice> > netDevicesChildren;

public:
	NetworkTreeNode(Ptr<Node> node) { this->node = node; }
	virtual ~NetworkTreeNode() {}

	void addChild(NetworkTreeNode* child) { children.push_back(child); }
	std::vector<NetworkTreeNode*>* getChildren() { return &children; }
	bool isLeaf() { return children.size() == 0; }
	Ptr<Node> getNode() { return node; }
	void addNetDeviceRoot(Ptr<NetDevice> dev) { netDeviceRoot = dev; }
	void addNetDeviceChild(Ptr<NetDevice> dev) { netDevicesChildren.push_back(dev); }
	Ptr<NetDevice> getNetDeviceRoot() { return netDeviceRoot; }
	Ptr<NetDevice> getNetDeviceChild(int id) { return netDevicesChildren[id]; }

};

}

#endif
