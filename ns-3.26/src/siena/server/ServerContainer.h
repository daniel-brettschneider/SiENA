#ifndef NS_3_22_SRC_SIENA_SERVER_SERVERCONTAINER_H_
#define NS_3_22_SRC_SIENA_SERVER_SERVERCONTAINER_H_

#include <vector>
#include "ns3/Server.h"
#include "ns3/ClusterServer.h"
#include "ns3/MySingleton.h"

namespace ns3 {

class ServerContainer : public MySingleton<ServerContainer> {
	friend class MySingleton<ServerContainer>;

private:
	ClusterServer* clusterServer;
	std::vector<Server*> servers;

public:
	virtual ~ServerContainer() {
		delete clusterServer;
		for(uint i = 0; i < servers.size(); i++) {
			delete servers[i];
		}
	}

	ClusterServer* getClusterServer() { return clusterServer; }
	void setClusterServer(ClusterServer* clusterServer) { this->clusterServer = clusterServer; }
	void addServer(Server* server) { servers.push_back(server); }
	Server* getServer(int cluster) { return servers[cluster]; }
	std::vector<Server*>* getServers() { return &servers; }

private:
	ServerContainer() : clusterServer(NULL) {}

};

}

#endif
