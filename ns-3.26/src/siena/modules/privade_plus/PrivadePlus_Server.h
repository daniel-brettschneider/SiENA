#ifndef NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_SERVER_H_
#define NS_3_22_SRC_SIENA_MODULES_PRIVADE_PLUS_PRIVADEPLUS_SERVER_H_

#include "ns3/Server.h"
#include "ns3/DataBasis.h"
#include "ns3/PrivadePlus_TokenData.h"
#include "ns3/PrivadePlus_TokenAdapt.h"
#include "ns3/PrivadePlus_TokenShare.h"
#include "ns3/DsmMode.h"
#include <algorithm>

namespace ns3 {

static void privadePlus_ServerHandleDataToken(ClusterServer* server, PrivadePlus_TokenData* token) {
	// data, first entry
	MyStats* stats = MyStats::Get();
	std::vector<std::string>* homeAddresses = server->getData<std::vector<std::string> >("homeAddresses", false);
	std::vector<double>* futureConsumption = server->getData<std::vector<double> >("futureConsumption", false);
	std::vector<double>* limits = server->getData<std::vector<double> >("limits", false);
	int* msgsStatsId = server->getData<int>("msgs_stats", false);
	int* roundsStatsId = server->getData<int>("rounds_stats", false);
	int* timeStatsId = server->getData<int>("time_stats", false);
	int* dsmMode = server->getData<int>("dsm_mode", false);
	int* convStatsId = server->getData<int>("conv_stats", false);
	int* backoff = server->getData<int>("backoff", false);
	int* numberHomes = server->getData<int>("numberHomes", false);
	if(msgsStatsId == NULL) {
		MyConfig* config = MyConfig::Get();
		server->addDataValue("power", new bool(config->getBool("simulate_power")));
		limits = DsmMode::Get()->getQualityGoal();
		server->addDataValue("limits", limits);
		msgsStatsId = new int(stats->registerStat("msgs", server->getId(), 0));
		server->addDataValue("msgs_stats", msgsStatsId);
		roundsStatsId = new int(stats->registerStat("rounds", server->getId(), 0));
		server->addDataValue("rounds_stats", roundsStatsId);
		timeStatsId = new int(stats->registerStat("time", server->getId()));
		server->addDataValue("time_stats", timeStatsId);
		dsmMode = new int(DsmMode::get());
		server->addDataValue("dsm_mode", dsmMode);
		convStatsId = new int(stats->registerStat("convergence", server->getId(), 0));
		server->addDataValue("conv_stats", convStatsId);
		backoff = new int(config->getInt("privade_backoff2"));
		server->addDataValue("backoff", backoff);
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
		numberHomes = new int(HomeContainer::Get()->getHomes()->size());
		server->addDataValue("numberHomes", numberHomes);
	}

	// stats
	stats->increaseStat(*msgsStatsId, homeAddresses->size() + 1);
	stats->increaseStat(*roundsStatsId, 1);
	stats->addStat(*timeStatsId, Tick::Get()->getTimeInInterval());

	// limit
	Tick* tick = Tick::Get();
	double limit = (*limits)[(tick->getMinute() / tick->getInterval()) % limits->size()] * *numberHomes;

	HomeContainer* homes = HomeContainer::Get();
	double otherConsumption = 0;
	std::vector<ConventionalHome*>* others = homes->getHomes(0, 0);
	for(uint i = 0; i < others->size(); i++) {
		otherConsumption += (*others)[i]->getConsumption();
	}
	others = homes->getHomes(0, 1);
	for(uint i = 0; i < others->size(); i++) {
		otherConsumption += (*others)[i]->getConsumption();
	}
	token->addCurrent(otherConsumption);

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
		PrivadePlus_TokenShare* tokenShare = new PrivadePlus_TokenShare(*timeStatsId);
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
						tokenShare->addSwitchable(cat.first, i);
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
						tokenShare->addSwitchable(cat.first, categories->size()-1);
				}
				for(uint j = 0; j < catsNegative[categories->size()-1].size(); j++) { // add negative desired, they have to be switched
					std::pair<int, int> cat = catsNegative[categories->size()-1][j];
					tokenShare->addSwitchable(cat.first, categories->size()-1);
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
							tokenShare->addSwitchable(cat.first, i);
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
						tokenShare->addSwitchable(cat.first, categories->size()-1);
				}
				for(uint j = 0; j < catsPositive[categories->size()-1].size(); j++) { // add positive desired, they have to be switched
					std::pair<int, int> cat = catsPositive[categories->size()-1][j];
					tokenShare->addSwitchable(cat.first, categories->size()-1);
				}
				#ifndef TURBO
					Log::i(server->getId(), s.str()); s.str("");
				#endif
			} else { // within limits, add desired to token
				for(uint j = 0; j < (*categories)[categories->size()-1].size(); j++) {
					std::pair<int, int> cat = (*categories)[categories->size()-1][j];
					if(cat.first > 0)
						tokenShare->addSwitchable(j, categories->size()-1);
				}
			}
		}

		// flex
		double error2 = fabs(sumSwitchable - limit) * (sumSwitchable > limit ? -1 : 1);
		if(error2 < -1 && token->getNumberFlexDown() > 0)
			tokenShare->setShareFlex(error2 / token->getNumberFlexDown());
		else if(error2 > 1 && token->getNumberFlexUp() > 0)
			tokenShare->setShareFlex(error2 / token->getNumberFlexUp());

		#ifndef TURBO
			Log::i(server->getId(), "\t=> consumption is now: ", sumSwitchable);
			Log::i(server->getId(), "\tnew error: ", error2);
			s << "\tflex up: " << token->getNumberFlexUp() << "/" << token->getAmountFlexUp() << ", down: " << token->getNumberFlexDown() << "/" << token->getAmountFlexDown();
			Log::i(server->getId(), s.str()); s.str("");
			Log::i(server->getId(), "\t=> share for flex: ", tokenShare->getShareFlex());
		#endif

		// send token
		if(tokenShare->hasSwitchable() || tokenShare->getShareFlex() != 0) {
			Log::i(server->getId(), "starting next round");
			for(uint i = 0; i < homeAddresses->size(); i++) {
				Time t = MicroSeconds(Random::Get()->get(0, *backoff));
				PrivadePlus_TokenShare* send = new PrivadePlus_TokenShare(*tokenShare);
				server->scheduleSend(t, (*homeAddresses)[i], send->getPacket());
			}
			delete tokenShare;
		} else {
			Log::i(server->getId(), "finished, remaining error: ", error2);
			delete tokenShare;
			stats->addStat(*convStatsId, 1);
		}
	} else {
		// send token with desired switchable, if there is no error
		stats->increaseStat(*msgsStatsId, homeAddresses->size());
		stats->increaseStat(*roundsStatsId, 1);

		if(consumptionDesiredSwitchable != token->getAmountCurrent()) {
			PrivadePlus_TokenShare* tokenShare = new PrivadePlus_TokenShare(*timeStatsId);
			for(uint j = 0; j < (*categories)[categories->size()-1].size(); j++) {
				std::pair<int, int> cat = (*categories)[categories->size()-1][j];
				if(cat.first > 0)
					tokenShare->addSwitchable(j, categories->size()-1);
			}

			#ifndef TURBO
				std::stringstream s;
				s << "\tcurrent consumption: " << token->getAmountCurrent() << ", with all desired switchables: " << consumptionDesiredSwitchable;
				Log::i(server->getId(), s.str()); s.str("");
				s << "\terror: " << error << ", limit: " << limit;
				Log::i(server->getId(), s.str()); s.str("");
				Log::i(server->getId(), "\tswitching all desired categories");
				Log::i(server->getId(), "starting next round");
			#endif

			for(uint i = 0; i < homeAddresses->size(); i++) {
				Time t = MicroSeconds(Random::Get()->get(0, *backoff));
				PrivadePlus_TokenShare* send = new PrivadePlus_TokenShare(*tokenShare);
				server->scheduleSend(t, (*homeAddresses)[i], send->getPacket());
			}
			delete tokenShare;
		} else {
			#ifndef TURBO
				std::stringstream s;
				s << "\terror: " << error << ", limit: " << limit << " => finished";
				Log::i(server->getId(), s.str());
			#endif
			stats->addStat(*convStatsId, 1);
		}
	}

	delete token;
}

static void privadePlus_ServerHandleAdaptToken(ClusterServer* server, PrivadePlus_TokenAdapt* token) {
	// data
	MyStats* stats = MyStats::Get();
	std::vector<std::string>* homeAddresses = server->getData<std::vector<std::string> >("homeAddresses");
	int* msgsStatsId = server->getData<int>("msgs_stats");
	int* roundsStatsId = server->getData<int>("rounds_stats");
	int* timeStatsId = server->getData<int>("time_stats");
	int* backoff = server->getData<int>("backoff", false);

	// stats
	stats->increaseStat(*msgsStatsId, homeAddresses->size() * 2 + 1);
	stats->increaseStat(*roundsStatsId, 1);
	stats->addStat(*timeStatsId, Tick::Get()->getTimeInInterval());

	double error = token->getAmountResidual();
	std::stringstream s;
	s << "\terror: " << error << ", flex number: " << token->getNumberFlex();
	Log::i(server->getId(), s.str());

	if((error > 1 || error < -1) && token->getNumberFlex() > 0) {
		double share = error / token->getNumberFlex();

		PrivadePlus_TokenShare* tokenNew = new PrivadePlus_TokenShare(*timeStatsId);
		tokenNew->setShareFlex(share);
		Log::i(server->getId(), "\t\t=> share: ", share);
		Log::i(server->getId(), "starting next round");

		for(uint i = 0; i < homeAddresses->size(); i++) {
			Time t = MicroSeconds(Random::Get()->get(0, *backoff));
			PrivadePlus_TokenShare* send = new PrivadePlus_TokenShare(*tokenNew);
			server->scheduleSend(t, (*homeAddresses)[i], send->getPacket());
		}
		delete tokenNew;
	} else {
		Log::i(server->getId(), "finished, remaining error: ", error);
		int* convStatsId = server->getData<int>("conv_stats");
		stats->addStat(*convStatsId, 1);
	}
	delete token;
}

static void privadePlus_ServerHandleToken(std::vector<void*>* params) {
	ClusterServer* server = (ClusterServer*) (*params)[0];
	Token* token = (Token*) (*params)[1];
	PrivadePlus_TokenData* tokenData = dynamic_cast<PrivadePlus_TokenData*>(token);
	if(tokenData != NULL)
		privadePlus_ServerHandleDataToken(server, tokenData);
	else if(dynamic_cast<PrivadePlus_TokenAdapt*>(token) != NULL)
		privadePlus_ServerHandleAdaptToken(server, (PrivadePlus_TokenAdapt*) token);
}

}

#endif
