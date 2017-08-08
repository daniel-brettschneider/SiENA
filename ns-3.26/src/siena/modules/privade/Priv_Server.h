#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_SERVER_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PRIV_SERVER_H_

#include "ns3/Server.h"
#include "ns3/DataBasis.h"
#include "ns3/Priv_TokenData.h"
#include "ns3/Priv_TokenAdapt.h"
#include "ns3/DsmMode.h"
#include <algorithm>

namespace ns3 {

//class FlexHelper {
//public:
//	double numberUp, numberDown, amountUp, amountDown, amountCurrent;
//	double share;
//	FlexHelper() : numberUp(0), numberDown(0), amountUp(0), amountDown(0), amountCurrent(0), share(0) {}
//	void set(Priv_TokenData* t) { numberUp = t->getNumberFlexUp(); numberDown = t->getNumberFlexDown(); amountUp = t->getAmountFlexUp(); amountDown = t->getAmountFlexDown();
//			amountCurrent = t->getAmountCurrent(); }
//};

static void priv_ServerStartInterval(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	std::string ip = server->getIp();
	HomeContainer* homes = HomeContainer::Get();
	MyStats* stats = MyStats::Get();

	// data
	std::vector<std::string>* homeAddresses = server->getData<std::vector<std::string> >("homeAddresses", false);
	std::vector<double>* futureConsumption = server->getData<std::vector<double> >("futureConsumption", false);;
	if(homeAddresses == NULL) {
		MyConfig* config = MyConfig::Get();
		homeAddresses = new std::vector<std::string>;
		Ptr<AttributeContainer<Ipv4Address> > tmp = server->getHomeAddresses();
		for(int i = 0; i < tmp->size(); i++) {
			Ipv4Address adr = tmp->at(i);
			homeAddresses->push_back(Helper::toString(adr));
		}
		server->addDataValue("homeAddresses", homeAddresses);
		server->addDataValue("neplan", new bool(config->getBool("simulate_power")));
		std::vector<double>* limits = DsmMode::Get()->getQualityGoal();
		server->addDataValue("limits", limits);
		bool* performClustering = new bool(config->getBool("enable_clustering"));
		server->addDataValue("performClustering", performClustering);
		int* msgsStatsId = new int(stats->registerStat("msgs", server->getId(), 0));
		server->addDataValue("msgs_stats", msgsStatsId);
		int* roundsStatsId = new int(stats->registerStat("rounds", server->getId(), 0));
		server->addDataValue("rounds_stats", roundsStatsId);
		int* timeStatsId = new int(stats->registerStat("time", server->getId()));
		server->addDataValue("time_stats", timeStatsId);
		int* dsmMode = new int(DsmMode::get());
		server->addDataValue("dsm_mode", dsmMode);
//		server->addDataValue("flex_helper", new FlexHelper());
		server->addDataValue("clustering_done", new bool(false));
		int* convStatsId = new int(stats->registerStat("convergence", server->getId(), 0));
		server->addDataValue("conv_stats", convStatsId);
		server->addDataValue("encryption_mode", new int(config->getString("encryption") == "bes" ? 0 : 1));
		std::string shift = config->getString("gridhome_shift_device");
		server->addDataValue("shift_expected", new bool(shift.find("priv_shift_expected") != std::string::npos));
		if(*server->getData<bool>("shift_expected")) {
			int interval = config->getInt("interval");
			int duration = config->getInt("duration") / interval;
			int start = config->getInt("start") / interval;
			int homes = config->getInt("total_homes");
			futureConsumption = new std::vector<double>;
			futureConsumption->reserve(duration);
			std::vector<double>* h0 = (*DataBasis::Get()->get("dsm/h0", "csv", CSV_PER_COLUMN, true, 1))[0];
			for(int i = 0; i < duration; i++) {
				futureConsumption->push_back((*h0)[start + i] * homes);
			}
			server->addDataValue("futureConsumption", futureConsumption);
		}
	}

	int encMode = *server->getData<int>("encryption_mode");
	bool shiftExpected = *server->getData<bool>("shift_expected");
	std::vector<std::string>* tempAddresses = new std::vector<std::string>(*homeAddresses);
	std::random_shuffle(tempAddresses->begin(), tempAddresses->end());
	tempAddresses->push_back(ip);
	Priv_TokenData* token = new Priv_TokenData(encMode, shiftExpected, tempAddresses, futureConsumption);
	bool* clusteringDone = server->getData<bool>("clustering_done");
	*clusteringDone = false;

	double otherConsumption = 0;
	std::vector<ConventionalHome*>* others = homes->getHomes(server->getCluster(), 0);
	for(uint i = 0; i < others->size(); i++) {
		otherConsumption += (*others)[i]->getConsumption();
	}
	others = homes->getHomes(server->getCluster(), 1);
	for(uint i = 0; i < others->size(); i++) {
		otherConsumption += (*others)[i]->getConsumption();
	}
	token->addCurrent(otherConsumption);

	server->scheduleSend(token->next(), token->getPacket());
}

static void priv_ServerHandleDataToken(Server* server, Priv_TokenData* token) {
	// data
	MyStats* stats = MyStats::Get();
	std::vector<std::string>* homeAddresses = server->getData<std::vector<std::string> >("homeAddresses");
	std::vector<double>* limits = server->getData<std::vector<double> >("limits");
	bool* performClustering = server->getData<bool>("performClustering");
	int* msgsStatsId = server->getData<int>("msgs_stats");
	int* roundsStatsId = server->getData<int>("rounds_stats");
	int* timeStatsId = server->getData<int>("time_stats");
	int* dsmMode = server->getData<int>("dsm_mode");
	int* convStatsId = server->getData<int>("conv_stats");
//	FlexHelper* flexHelper = server->getData<FlexHelper>("flex_helper");

	// stats
	stats->increaseStat(*msgsStatsId, server->getNumberOfParticipants() + 1);
	stats->increaseStat(*roundsStatsId, 1);
	stats->addStat(*timeStatsId, Tick::Get()->getTimeInInterval());

	// limit
	Tick* tick = Tick::Get();
	double limit = (*limits)[(tick->getMinute() / tick->getInterval()) % limits->size()] * server->getNumberOfParticipants();

	// get switchable data
	std::vector<std::vector<std::pair<int, int> > >* categories = token->getSwitchableCategories();
	std::vector<std::vector<std::pair<int, int> > > catsPositive(categories->size()), catsNegative(categories->size());
	double consumptionDesiredSwitchable = token->getAmountCurrent();
	#ifndef TURBO
		std::stringstream sss;
		sss << "\tcategories: ";
	#endif
	for(uint i = 0; i < categories->size(); i++) {
		for(uint j = 0; j < (*categories)[i].size(); j++) {
			std::pair<int, int> cat = (*categories)[i][j];
			if(cat.first > 0) {
				if(cat.second > 0)
					catsPositive[i].push_back(std::pair<int,int>(j, cat.first * cat.second));
				else if(cat.second < 0)
					catsNegative[i].push_back(std::pair<int,int>(j, cat.first * cat.second));
				if(i == categories->size() - 1)
					consumptionDesiredSwitchable += cat.first * cat.second;
				#ifndef TURBO
					sss << i << "/" << j << "/" << cat.first << "/" << cat.second << " ";
				#endif
			}
		}
	}
	double error = limit - consumptionDesiredSwitchable;
	if(error > 1 || error < -1) {
		int encMode = *server->getData<int>("encryption_mode");
		std::vector<std::string>* tempAddresses = new std::vector<std::string>(*homeAddresses);
		std::random_shuffle(tempAddresses->begin(), tempAddresses->end());
		tempAddresses->push_back(server->getIp());
		Priv_TokenAdapt* tokenAdapt = new Priv_TokenAdapt(encMode, tempAddresses);
		for(uint i = 0; i < categories->size(); i++) {
			std::random_shuffle(catsPositive[i].begin(), catsPositive[i].end());
			std::random_shuffle(catsNegative[i].begin(), catsNegative[i].end());
		}

		// switchable
		double sumSwitchable = consumptionDesiredSwitchable;
		double upperLimitSwitchable = limit + token->getAmountFlexDown(), lowerLimitSwitchable = limit - token->getAmountFlexUp();
		#ifndef TURBO
			std::stringstream s;
			s << "\tcurrent consumption: " << token->getAmountCurrent() << ", with all desired switchables: " << consumptionDesiredSwitchable;
			Log::i(server->getId(), s.str()); s.str("");
			s << "\terror: " << error << ", limit: " << limit;
			Log::i(server->getId(), s.str()); s.str("");
			Log::i(server->getId(), "\tswitchable number: ", token->getNumberSwitchable());
			Log::i(server->getId(), sss.str());
			s << "\t=> target for switchables: " << lowerLimitSwitchable << "/" << upperLimitSwitchable;
			Log::i(server->getId(), s.str()); s.str("");
		#endif
		if(token->getNumberSwitchable() > 0) {
			if(sumSwitchable > upperLimitSwitchable) { // reduce switchable
				#ifndef TURBO
					Log::i(server->getId(), "\ttrying to reduce consumption of switchables...");
					s << "\tadding negative undesired: ";
				#endif
				// add negative of undesired
				for(int i = (int) categories->size() - 2; i >= 0; i--) {
					for(uint j = 0; j < catsNegative[i].size(); j++) {
						std::pair<int, int> cat = catsNegative[i][j];
						sumSwitchable += cat.second;
						tokenAdapt->addSwitchable(cat.first, i);
						#ifndef TURBO
							s << cat.first << "/" << cat.second << " ";
						#endif
						if(sumSwitchable <= upperLimitSwitchable)
							break;
					}
					if(sumSwitchable <= upperLimitSwitchable)
						break;
				}
				#ifndef TURBO
					Log::i(server->getId(), s.str()); s.str("");
				// remove positive of desired
					s << "\tremoving positive desired: ";
				#endif
				for(uint j = 0; j < catsPositive[categories->size()-1].size(); j++) {
					std::pair<int, int> cat = catsPositive[categories->size()-1][j];
					if(sumSwitchable > upperLimitSwitchable) {
						sumSwitchable -= cat.second;
						#ifndef TURBO
							s << cat.first << "/" << cat.second << " ";
						#endif
					} else // add remaining desired, they have to be switched
						tokenAdapt->addSwitchable(cat.first, categories->size()-1);
				}
				for(uint j = 0; j < catsNegative[categories->size()-1].size(); j++) { // add negative desired, they have to be switched
					std::pair<int, int> cat = catsNegative[categories->size()-1][j];
					tokenAdapt->addSwitchable(cat.first, categories->size()-1);
				}
				#ifndef TURBO
					Log::i(server->getId(), s.str()); s.str("");
				#endif
			} else if(sumSwitchable < lowerLimitSwitchable) { // increase switchable
				#ifndef TURBO
					Log::i(server->getId(), "\ttrying to increase consumption of switchables...");
					s << "\tadding positive undesired: ";
				#endif
				// add positive of undesired
				for(int i = (int) categories->size() - 2; i >= 0; i--) {
					for(uint j = 0; j < catsPositive[i].size(); j++) {
						std::pair<int, int> cat = catsPositive[i][j];
						if(sumSwitchable < lowerLimitSwitchable && (*dsmMode != DSM_Peak_Clipping || (*dsmMode == DSM_Peak_Clipping && sumSwitchable + cat.second  <= upperLimitSwitchable))) {
							sumSwitchable += cat.second;
							tokenAdapt->addSwitchable(cat.first, i);
							#ifndef TURBO
								s << cat.first << "/" << cat.second << " ";
							#endif
						} else if(sumSwitchable >= lowerLimitSwitchable)
							break;
					}
					if(sumSwitchable >= lowerLimitSwitchable)
						break;
				}
				#ifndef TURBO
					Log::i(server->getId(), s.str()); s.str("");
				// remove negative of desired
					s << "\tremoving negative desired: ";
				#endif
				for(uint j = 0; j < catsNegative[categories->size()-1].size(); j++) {
					std::pair<int, int> cat = catsNegative[categories->size()-1][j];
					if(sumSwitchable < lowerLimitSwitchable && (*dsmMode != DSM_Peak_Clipping || (*dsmMode == DSM_Peak_Clipping && sumSwitchable - cat.second  <= upperLimitSwitchable))) {
						sumSwitchable -= cat.second;
						#ifndef TURBO
							s << cat.first << "/" << cat.second << " ";
						#endif
					} else // add remaining desired, they have to be switched
						tokenAdapt->addSwitchable(cat.first, categories->size()-1);
				}
				for(uint j = 0; j < catsPositive[categories->size()-1].size(); j++) { // add positive desired, they have to be switched
					std::pair<int, int> cat = catsPositive[categories->size()-1][j];
					tokenAdapt->addSwitchable(cat.first, categories->size()-1);
				}
				#ifndef TURBO
					Log::i(server->getId(), s.str()); s.str("");
				#endif
			} else { // within limits, add desired to token
				for(uint j = 0; j < (*categories)[categories->size()-1].size(); j++) {
					std::pair<int, int> cat = (*categories)[categories->size()-1][j];
					if(cat.first > 0)
						tokenAdapt->addSwitchable(j, categories->size()-1);
				}
			}
		}

		// flex
//		flexHelper->set(token);
//		flexHelper->amountCurrent = sumSwitchable;
		double error2 = fabs(sumSwitchable - limit) * (sumSwitchable > limit ? -1 : 1);
		if(error2 < -1 && token->getNumberFlexDown() > 0)
			tokenAdapt->setShareFlex(error2 / token->getNumberFlexDown());
		else if(error2 > 1 && token->getNumberFlexUp() > 0)
			tokenAdapt->setShareFlex(error2 / token->getNumberFlexUp());
//		flexHelper->share = error2;

		#ifndef TURBO
			Log::i(server->getId(), "\t=> consumption is now: ", sumSwitchable);
			Log::i(server->getId(), "\tnew error: ", error2);
			s << "\tflex up: " << token->getNumberFlexUp() << "/" << token->getAmountFlexUp() << ", down: " << token->getNumberFlexDown() << "/" << token->getAmountFlexDown();
			Log::i(server->getId(), s.str()); s.str("");
			Log::i(server->getId(), "\t=> share for flex: ", tokenAdapt->getShareFlex());
		#endif

		// send token
		if(tokenAdapt->hasSwitchable() || tokenAdapt->getShareFlex() != 0) {
			server->scheduleSend(tokenAdapt->next(), tokenAdapt->getPacket());
			Log::i(server->getId(), "starting next round");
		} else {
			Log::i(server->getId(), "finished, remaining error: ", error2);
			delete tokenAdapt;
			stats->addStat(*convStatsId, 1);
			if(*performClustering) {
//				Log::i(server->getId(), "reporting to cluster server");
//				Priv_TokenCluster* tokenCluster = NULL;
//				tokenCluster = new Priv_TokenCluster(server->getIp(), flexHelper->numberUp, flexHelper->numberDown, flexHelper->amountUp, flexHelper->amountDown, flexHelper->amountCurrent);
//				Ipv4Address next = server->getServerAddress();
//				server->scheduleSend(Helper::toString(next), tokenCluster->getPacket());
				throw("clustering not supported, turn off in PrivADE config");
			}
		}
	} else {
//		flexHelper->set(token);
		// send token with desired switchable, if there is no error
		if(consumptionDesiredSwitchable != token->getAmountCurrent()) {
			int encMode = *server->getData<int>("encryption_mode");
			std::vector<std::string>* tempAddresses = new std::vector<std::string>(*homeAddresses);
			std::random_shuffle(tempAddresses->begin(), tempAddresses->end());
			tempAddresses->push_back(server->getIp());
			Priv_TokenAdapt* tokenAdapt = new Priv_TokenAdapt(encMode, tempAddresses);
			for(uint j = 0; j < (*categories)[categories->size()-1].size(); j++) {
				std::pair<int, int> cat = (*categories)[categories->size()-1][j];
				if(cat.first > 0)
					tokenAdapt->addSwitchable(j, categories->size()-1);
			}

			server->scheduleSend(tokenAdapt->next(), tokenAdapt->getPacket());
			std::stringstream s;
			s << "\tcurrent consumption: " << token->getAmountCurrent() << ", with all desired switchables: " << consumptionDesiredSwitchable;
			Log::i(server->getId(), s.str()); s.str("");
			s << "\terror: " << error << ", limit: " << limit;
			Log::i(server->getId(), s.str()); s.str("");
			Log::i(server->getId(), "\tswitching all desired categories");
			Log::i(server->getId(), "starting next round");
		} else {
			#ifndef TURBO
				std::stringstream s;
				s << "\terror: " << error << ", limit: " << limit << " => finished";
				Log::i(server->getId(), s.str());
			#endif
			stats->addStat(*convStatsId, 1);
			if(*performClustering && (token->getNumberFlexUp() > 0 || token->getNumberFlexDown() > 0)) {
//				Log::i(server->getId(), "there are remaining flex loads => reporting to cluster server");
//				Priv_TokenCluster* tokenCluster = new Priv_TokenCluster(server->getIp(), token->getNumberFlexUp(), token->getNumberFlexDown(), token->getAmountFlexUp(),
//						token->getAmountFlexDown(), token->getAmountCurrent());
//				Ipv4Address next = server->getServerAddress();
//				server->scheduleSend(Helper::toString(next), tokenCluster->getPacket());
				throw("clustering not supported, turn off in PrivADE config");
			}
		}
	}

	delete token;
}

static void priv_ServerHandleAdaptToken(Server* server, Priv_TokenAdapt* token) {
	// data
	MyStats* stats = MyStats::Get();
	std::vector<std::string>* homeAddresses = server->getData<std::vector<std::string> >("homeAddresses");
	bool* performClustering = server->getData<bool>("performClustering");
	int* msgsStatsId = server->getData<int>("msgs_stats");
	int* roundsStatsId = server->getData<int>("rounds_stats");
	int* timeStatsId = server->getData<int>("time_stats");
//	FlexHelper* flexHelper = server->getData<FlexHelper>("flex_helper");
	bool* clusteringDone = server->getData<bool>("clustering_done");

	// stats
	stats->increaseStat(*msgsStatsId, server->getNumberOfParticipants() + 1);
	stats->increaseStat(*roundsStatsId, 1);
	stats->addStat(*timeStatsId, Tick::Get()->getTimeInInterval());

	double error = token->getAmountResidual();
	std::stringstream s;
	s << "\terror: " << error << ", flex number: " << token->getNumberFlex();
	Log::i(server->getId(), s.str());

//	flexHelper->amountCurrent += flexHelper->share - error;
//	if(error < 0) {
//			double tmp = fabs(flexHelper->share) - fabs(error);
//			flexHelper->amountDown -= tmp;
//			flexHelper->amountUp += tmp;
//			flexHelper->numberUp = flexHelper->numberUp - (flexHelper->numberDown - token->getNumberFlex());
//			flexHelper->numberDown = token->getNumberFlex();
//
//		} else {
//			double tmp = flexHelper->share - error;
//			flexHelper->amountUp -= tmp;
//			flexHelper->amountDown += tmp;
//			flexHelper->up = token->getNumberFlex() > 0;
//			flexHelper->down = true;
//		}
//	flexHelper->share = error;

	if((error > 1 || error < -1) && token->getNumberFlex() > 0) {
		double share = error / token->getNumberFlex();
		int encMode = *server->getData<int>("encryption_mode");
		std::vector<std::string>* tempAddresses = new std::vector<std::string>(*homeAddresses);
		std::random_shuffle(tempAddresses->begin(), tempAddresses->end());
		tempAddresses->push_back(server->getIp());
		Priv_TokenAdapt* tokenNew = new Priv_TokenAdapt(encMode, tempAddresses);
		tokenNew->setShareFlex(share);
		server->scheduleSend(tokenNew->next(), tokenNew->getPacket());
		Log::i(server->getId(), "\t\t=> share: ", share);
		Log::i(server->getId(), "starting next round");
	} else {
		Log::i(server->getId(), "finished, remaining error: ", error);
		int* convStatsId = server->getData<int>("conv_stats");
		stats->addStat(*convStatsId, 1);
		if(!*clusteringDone && *performClustering) {
//			Log::i(server->getId(), "reporting to cluster server");
//			Priv_TokenCluster* tokenCluster = NULL;
//			tokenCluster = new Priv_TokenCluster(server->getIp(), flexHelper->numberUp, flexHelper->numberDown, flexHelper->amountUp, flexHelper->amountDown, flexHelper->amountCurrent);
//			Ipv4Address next = server->getServerAddress();
//			server->scheduleSend(Helper::toString(next), tokenCluster->getPacket());
			throw("clustering not supported, turn off in PrivADE config");
		}
	}
	delete token;
}

static void priv_ServerHandleClusterToken(Server* server, Priv_TokenCluster* token) {
	/*bool* clusteringDone = server->getData<bool>("clustering_done");
	*clusteringDone = true;

	// data
	MyStats* stats = MyStats::Get();
	std::vector<std::string>* homeAddresses = server->getData<std::vector<std::string> >("homeAddresses");
	int* msgsStatsId = server->getData<int>("msgs_stats");
	int* roundsStatsId = server->getData<int>("rounds_stats");
	int* timeStatsId = server->getData<int>("time_stats");
	FlexHelper* flexHelper = server->getData<FlexHelper>("flex_helper");

	// stats
	stats->increaseStat(*msgsStatsId, server->getNumberOfParticipants() + 1);
	stats->increaseStat(*roundsStatsId, 1);
	stats->addStat(*timeStatsId, Tick::Get()->getTimeInInterval());

	double error = token->getAssigned();
	int numberFlex = 0;
	if(error < 0)
		numberFlex = flexHelper->numberDown;
	else
		numberFlex = flexHelper->numberUp;

	std::stringstream s;
	s << "\terror: " << error << ", flex number: " << numberFlex;
	Log::i(server->getId(), s.str());

	if((error > 1 || error < -1) && numberFlex > 0) {
		double share = error / numberFlex;
		Priv_TokenAdapt* tokenNew = new Priv_TokenAdapt(new std::vector<std::string>(*homeAddresses));
		tokenNew->setShareFlex(share);
		server->scheduleSend(tokenNew->next(), tokenNew->getPacket());
		Log::i(server->getId(), "\t\t=> share: ", share);
		Log::i(server->getId(), "starting next round");
	} else
		Log::i(server->getId(), "finished, remaining error: ", error);*/

	delete token;
}

static void priv_ServerHandleToken(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	Token* token = (Token*) (*params)[1];
	Priv_TokenData* tokenData = dynamic_cast<Priv_TokenData*>(token);
	if(tokenData != NULL)
		priv_ServerHandleDataToken(server, tokenData);
	else if(dynamic_cast<Priv_TokenAdapt*>(token) != NULL)
		priv_ServerHandleAdaptToken(server, (Priv_TokenAdapt*) token);
	else if(dynamic_cast<Priv_TokenCluster*>(token) != NULL)
		priv_ServerHandleClusterToken(server, (Priv_TokenCluster*) token);
}

}

#endif
