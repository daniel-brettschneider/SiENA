#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_CLUSTERSERVER_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_CLUSTERSERVER_H_

#include "ns3/ClusterServer.h"
#include "ns3/Priv_TokenCluster.h"

namespace ns3 {

class ClusterHelper {
public:
	std::string ip;
	double numberUp, numberDown, amountUp, amountDown, amountCurrent;
	double assigned;
	ClusterHelper(Priv_TokenCluster* t) : ip(t->getIp()), numberUp(t->getNumberFlexUp()), numberDown(t->getNumberFlexDown()), amountUp(t->getAmountFlexUp()),
			amountDown(t->getAmountFlexDown()), amountCurrent(t->getAmountCurrent()), assigned(t->getAmountCurrent()) {}
	virtual ~ClusterHelper() {}
};

static void priv_ClusterServerHandleClusterToken(ClusterServer* server, Priv_TokenCluster* token) {
	// add token to list
	std::vector<ClusterHelper>* tokens = server->getData<std::vector<ClusterHelper> >("token_list");
	tokens->push_back(ClusterHelper(token));
	delete token;
	if(tokens->size() >= (uint) server->getChildren()->size()) {

		// data
		std::vector<double>* limits = server->getData<std::vector<double> >("limits", false);
		if(limits == NULL) {
			limits = DsmMode::Get()->getQualityGoal();
			server->addDataValue("limits", limits);
		}
		int* dsmMode = server->getData<int>("dsm_mode", false);
		if(dsmMode == NULL) {
			int* dsmMode = new int(DsmMode::get());
			server->addDataValue("dsm_mode", dsmMode);
		}
		int* homes = server->getData<int>("homes", false);
		if(homes == NULL) {
			homes = new int(MyConfig::Get()->getInt("total_homes"));
			server->addDataValue("homes", homes);
		}

		// max min
		double consumption = 0, up = 0, down = 0;
		for(uint i = 0; i < tokens->size(); i++) {
			consumption += (*tokens)[i].amountCurrent;
			if((*tokens)[i].numberUp > 0)
				up++;
			if((*tokens)[i].numberDown > 0)
				down++;
		}
		double barrier = (*limits)[Tick::Get()->getMinute() % limits->size()] * *homes;
		#ifndef TURBO
			std::stringstream s;
			s << "consumption: " << consumption << " (" << barrier << "), up: " << up << ", down: " << down;
			Log::i(server->getId(), s.str());
		#endif

		double error = 0;
		if(consumption > barrier)
			error = -fabs(consumption - barrier);
		else if(consumption < -barrier)
			error = fabs(consumption - -barrier);
		Log::i(server->getId(), "error: ", error);

		double share = 0;
		int adaptable = 0;
		if(error < 0 && down > 0) {
			adaptable = down;
			share = error / down;
		} else if(error > 0 && up > 0) {
			adaptable = up;
			share = error / up;
		}

		if(share != 0) {
			int round = 1;
			while(((share < 0 && error < -1) || (share > 0 && error > 1)) && adaptable > 0) {
				#ifndef TURBO
					std::stringstream s;
					s << "round " << round << ", share: " << share << ", adaptable: " << adaptable;
					Log::i(server->getId(), s.str());
				#endif
				int adaptableTmp = 0;
				for(uint i = 0; i < tokens->size(); i++) {
					ClusterHelper a = (*tokens)[i];
					double take = 0;
					std::cout << share << " " << a.assigned << " " << a.amountCurrent << " " << a.amountDown << " " << a.amountUp << std::endl;
					if(share < 0 && a.assigned > a.amountCurrent - a.amountDown) {
						if(a.assigned + share < a.amountCurrent - a.amountDown)
							take = -fabs(a.assigned - (a.amountCurrent - a.amountDown));
						else {
							take = share;
							adaptableTmp++;
						}
					} else if(share > 0 && a.assigned < a.amountCurrent + a.amountUp) {
						if(a.assigned + share > a.amountCurrent + a.amountUp)
							take = fabs((a.amountCurrent + a.amountUp) - a.assigned);
						else {
							take = share;
							adaptableTmp++;
						}
					}
					a.assigned += take;
					error -= take;
					#ifndef TURBO
						s.str("");
						s << "\t" << a.ip << " take:" << take << ", assigned: " << a.assigned << " => error: " << error;
						Log::i(server->getId(), s.str());
					#endif
					(*tokens)[i] = a;
				}
				adaptable = adaptableTmp;
				share = error / adaptable;
				round++;
			}

			for(uint i = 0; i < tokens->size(); i++) {
				ClusterHelper a = (*tokens)[i];
				if(a.assigned != a.amountCurrent) {
					Priv_TokenCluster* t = new Priv_TokenCluster(a.assigned - a.amountCurrent);
					server->scheduleSend(a.ip, t->getPacket());
				}
			}
		}

		tokens->clear();
	}
}

static void priv_ClusterServerHandleToken(std::vector<void*>* params) {
	ClusterServer* server = (ClusterServer*) (*params)[0];
	Priv_TokenCluster* token = (Priv_TokenCluster*) (*params)[1];
	priv_ClusterServerHandleClusterToken(server, token);
}

}

#endif
