#include "../../../../siena/modules/network/tree/NetworkTree.h"

namespace ns3 {

NetworkTree::NetworkTree() : root(NULL) {
	config = MyConfig::Get();
	p2pLeaf.SetDeviceAttribute("DataRate", StringValue(config->getString("tree_leaf_data_rate")));
	p2pLeaf.SetChannelAttribute("Delay", StringValue(config->getString("tree_leaf_delay")));
	p2pRouter.SetDeviceAttribute("DataRate", StringValue(config->getString("tree_router_data_rate")));
	p2pRouter.SetChannelAttribute("Delay", StringValue(config->getString("tree_router_delay")));
}

NetworkTree::~NetworkTree() {
	delete root;
	for(uint i = 0; i < leaves.size(); i++) {
		delete leaves[i];
	}
	for(uint i = 0; i < routers.size(); i++) {
		for(uint j = 0; j < routers[i]->size(); j++) {
			delete (*routers[i])[j];
		}
		delete routers[i];
	}
}

NodeContainer NetworkTree::create(int nodes) {
	createLeafBranches(nodes);
	for(int i = 0; i < config->getInt("tree_layers"); i++) {
		createRouterBranches(i + 1);
	}
	createRoot(routers.size());
	return getLeaveNodes();
}

NodeContainer NetworkTree::getLeaveNodes() {
	NodeContainer c;
	for(uint i = 0; i < leaves.size(); i++) {
		c.Add(leaves[i]->getNode());
	}
	return c;
}

void NetworkTree::assignAddresses(Ipv4Address rootNetwork, Ipv4Mask rootMask) {
	assignAddresses(root, rootNetwork, rootMask);
}

void NetworkTree::createLeafBranches(int nodes) {
	// create leaf nodes with ip stack
	int numberOfLeaves = nodes;
	NodeContainer leafContainer;
	leafContainer.Create(numberOfLeaves);
	internet.Install(leafContainer);

	// create random order of leaf nodes
	std::vector<int> order = Random::Get()->randomList(0, numberOfLeaves);

	// position
	ObjectFactory leafPositionHelper;
	leafPositionHelper.SetTypeId("ns3::RandomDiscPositionAllocator");
	leafPositionHelper.Set("Rho", StringValue("ns3::UniformRandomVariable[Min=0|Max=45]"));
	leafPositionHelper.Set("Theta", StringValue("ns3::UniformRandomVariable[Min=0|Max=10]"));
	leafPositionHelper.Set("Y", DoubleValue(0));

	// create routers for leaf clusters
	int leafClusterSize = config->getInt("tree_leaf_cluster_size");
	int numberOfRouters = numberOfLeaves / leafClusterSize + (numberOfLeaves % leafClusterSize == 0? 0 : 1);
	NodeContainer routerContainer;
	routerContainer.Create(numberOfRouters);
	internet.Install(routerContainer);

	// create clusters
	std::vector<NetworkTreeNode*>* routerTreeNodes = new std::vector<NetworkTreeNode*>;
	for(int i = 0; i < numberOfRouters; i++) {
		// create tree node
		Ptr<Node> routerNode = routerContainer.Get(i);
		NetworkTreeNode* routerTreeNode = new NetworkTreeNode(routerNode);
		routerTreeNodes->push_back(routerTreeNode);

		// position
		Ptr<ConstantPositionMobilityModel> routerPosition = CreateObject<ConstantPositionMobilityModel>();
		routerPosition->SetPosition(Vector(i * 100, 0, 0));
		routerNode->AggregateObject(routerPosition);
		leafPositionHelper.Set("X", DoubleValue(i * 100));
		Ptr<PositionAllocator> leafPositionAllocator = leafPositionHelper.Create<PositionAllocator>();

		// leaf nodes
		for(int j = i * leafClusterSize; j < i * leafClusterSize + leafClusterSize && j < numberOfLeaves; j++) {
			// leaf
			Ptr<Node> leafNode = leafContainer.Get(order[j]);
			NetworkTreeNode* leafTreeNode = new NetworkTreeNode(leafNode);
			leaves.push_back(leafTreeNode);
			routerTreeNode->addChild(leafTreeNode);

			// create net devices
			NetDeviceContainer devs = p2pLeaf.Install(routerNode, leafNode);
			routerTreeNode->addNetDeviceChild(devs.Get(0));
			leafTreeNode->addNetDeviceRoot(devs.Get(1));

			// position
			Ptr<ConstantPositionMobilityModel> leafPosition = CreateObject<ConstantPositionMobilityModel>();
			leafPosition->SetPosition(leafPositionAllocator->GetNext());
			leafNode->AggregateObject(leafPosition);
		}
	}
	routers.push_back(routerTreeNodes);
}

void NetworkTree::createRouterBranches(int layer) {
	// get children
	std::vector<NetworkTreeNode*>* children = routers.back();
	int numberOfChildren = children->size();
	int numberOfRouters = numberOfChildren / 2 + (numberOfChildren % 2 == 0? 0 : 1);

	// create routers
	NodeContainer r;
	r.Create(numberOfRouters);
	internet.Install(r);

	// connect routers
	std::vector<NetworkTreeNode*>* routerTreeNodes = new std::vector<NetworkTreeNode*>;
	for(int i = 0; i < numberOfRouters; i++) {
		// tree node
		NetworkTreeNode* routerTreeNode = new NetworkTreeNode(r.Get(i));
		routerTreeNodes->push_back(routerTreeNode);

		// position
		Ptr<ConstantPositionMobilityModel> routerPosition = CreateObject<ConstantPositionMobilityModel>();
		routerPosition->SetPosition(Vector(-50 + 50 * pow(2, layer) + i * pow(2, layer) * 100, -layer * 100, 0));
		r.Get(i)->AggregateObject(routerPosition);

		// connect to children
		for(int j = i * 2; j < i * 2 + 2 && j < numberOfChildren; j++) {
			// create net devices
			NetDeviceContainer devs = p2pRouter.Install(r.Get(i), (*children)[j]->getNode());
			routerTreeNode->addChild((*children)[j]);
			routerTreeNode->addNetDeviceChild(devs.Get(0));
			(*children)[j]->addNetDeviceRoot(devs.Get(1));
		}
	}
	routers.push_back(routerTreeNodes);
}

void NetworkTree::createRoot(int layer) {
	// get children
	std::vector<NetworkTreeNode*>* children = routers.back();
	int numberOfChildren = children->size();

	// create root
	NodeContainer r;
	r.Create(1);
	internet.Install(r);
	root = new NetworkTreeNode(r.Get(0));

	// position
	Ptr<ConstantPositionMobilityModel> position = CreateObject<ConstantPositionMobilityModel>();
	position->SetPosition(Vector(-50 + 50 * pow(2, layer), -layer * 100, 0));
	r.Get(0)->AggregateObject(position);

	// connect to children
	for(int i = 0; i < numberOfChildren; i++) {
		// create net devices
		NetDeviceContainer devs = p2pRouter.Install(r.Get(0), (*children)[i]->getNode());
		root->addChild((*children)[i]);
		root->addNetDeviceChild(devs.Get(0));
		(*children)[i]->addNetDeviceRoot(devs.Get(1));
	}
}

void NetworkTree::assignAddresses(NetworkTreeNode* root, Ipv4Address rootNetwork, Ipv4Mask rootMask, int layer) {
	std::vector<NetworkTreeNode*>* children = root->getChildren();
	if(children->size() > 0) {
		Ipv4Mask outerMask = splitNetwork(rootMask, children->size());
		Ipv4AddressHelper outerHelper(rootNetwork, outerMask);
		Ipv4Address innerNetwork = rootNetwork;
		for(uint i = 0; i < children->size(); i++) {
			NetworkTreeNode* child = (*children)[i];
			Ipv4Mask innerMask = outerMask;
			if(!child->isLeaf())
				innerMask = splitNetwork(outerMask, child->getChildren()->size() + 1);
			Ipv4AddressHelper innerHelper(innerNetwork, innerMask);
			Ipv4AddressHelper innerHelperP2P(innerNetwork, Ipv4Mask("/30"));
			innerHelperP2P.Assign(NetDeviceContainer(root->getNetDeviceChild(i)));
			innerHelperP2P.Assign(child->getNetDeviceRoot());
			innerNetwork = outerHelper.NewNetwork();
			assignAddresses((*children)[i], innerHelper.NewNetwork(), innerMask, layer + 1);
		}
	}
}

Ipv4Mask NetworkTree::splitNetwork(Ipv4Mask mask, int subnets) {
	int add = ceil(log(subnets) / log(2));
	if(mask.GetPrefixLength() + add <= 31) {
		uint32_t tmp = 0x80000000 ;
		tmp >>= mask.GetPrefixLength ();
		uint32_t newMask = mask.Get();
		for(int i = 0; i < add; i++) {
			newMask |= (tmp >> i);
		}
		return Ipv4Mask(newMask);
	} else
		throw("cannot split ip mask any more");
}

}
