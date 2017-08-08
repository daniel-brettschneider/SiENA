//TODO does it work for more than one flexible device in one household?

#ifndef COHDA_GRIDHOME_H_
#define COHDA_GRIDHOME_H_

#include <vector>
#include <string>
#include <climits>
#include "ns3/GridHome.h"
#include "ns3/MyConfig.h"
#include "ns3/DeviceEvent.h"
#include "ns3/COHDA_Token.h"
#include "ns3/DataBasis.h"
#include "ns3/Turbo.h"
#include "ns3/HomeContainer.h"
#include "ns3/AdaptionCategories.h"
#include "ns3/COHDA_Configuration.h"
#include "ns3/COHDA_WorkingMemory.h"
#include "ns3/COHDA_AdaptOnOff.h"
#include "ns3/COHDA_AdaptFlex.h"
#include "ns3/COHDA_Helper.h"
#include "ns3/COHDA_ScheduleSelectionStore.h"
#include "ns3/ServerContainer.h"
#include "ns3/Turbo.h"
#include "ns3/DsmMode.h"

namespace ns3 {

static void COHDA_GridHomeHandleToken(std::vector<void*>* params);
static double COHDA_error(double desired, double consumption);
void COHDA_GridHomeOptimise(ConventionalHome* home);
static void COHDA_Shift(std::vector<void*>* params);
static void COHDA_scheduler_init(std::vector<void*>* params);
static void calcThetaOnOff(std::vector<int> consumptions, std::vector<int> states, int devNumber, std::vector<Device*>* devices, std::vector<std::vector<int> >* matrixStates, std::vector<std::vector<int> >* matrixConsumptions, std::vector<double>* thetas, double totalConsumption);
static void calcThetaFlex(std::vector<Device*>* devices, std::vector<std::vector<double> >* thetasFlex);
void COHDA_Gridhomefinaltick(GridHome* home);


static void COHDA_GridHomeHandleToken(std::vector<void*>* params) {
	GridHome* home = (GridHome*) (*params)[0];
	COHDA_Token* token = (COHDA_Token*) (*params)[1];

	Log::i(home->getId(), "token->getTick(): ", token->getTick());
	Log::i(home->getId(), "Tick::Get()->getTick(): ", Tick::Get()->getTick());
	if (token->getTick() != Tick::Get()->getTick()) {
		delete token;
		return;
	}

	int* convStatsId = home->getData<int>("conv_stats", false);
	if(convStatsId == NULL) {
		convStatsId = new int(MyStats::Get()->registerStat("convergence", "piep", 1));
		home->addDataValue("conv_stats", convStatsId);
	}

	std::vector<COHDA_Token*>* tokens = home->getData<std::vector<COHDA_Token*> >("tokens");
	tokens->push_back(token);

	// stats
	int interval = MyConfig::Get()->getInt("interval");
	MyStats* stats = MyStats::Get();
	Tick* tick = Tick::Get();

	if(token->getMsgStatsId() > -1) {
		int cnt = stats->getStat(tick->getSimMinute() / interval, token->getMsgStatsId());
		stats->addStat(token->getMsgStatsId(), cnt < 0 ? 1 : cnt + 1);
	}
//	token->getWorkingMemoryNice()->getZeta();

	bool* isScheduled = home->getData<bool>("isScheduled", false);
	if(isScheduled == NULL) {
		isScheduled = new bool(false);
		home->addDataValue("isScheduled", isScheduled);
	}
	double randomFactor = MyConfig::Get()->getDouble("random_schedule_factor");
	int scheduleTime = MyConfig::Get()->getInt("schedule_time_ms");
	double randomTime = (double)scheduleTime + Random::Get()->get(-randomFactor*scheduleTime, randomFactor*scheduleTime);
	if(Simulator::Now().GetSeconds() >= (tick->getTick() + 1) * interval * 60 - 5 - (randomTime)/1000) {
		stats->addStat(token->getTimeStatsId(), interval * 60000 - 5000);
		MyStats::Get()->addStat(*convStatsId, 0);
		Log::i(home->getId(), "will not be handled");
		for(uint i = 0; i < tokens->size(); i++) {
			delete (*tokens)[i];
		}
		tokens->clear();
	} else if(*isScheduled == false){
		Simulator::Schedule(MilliSeconds(randomTime), &COHDA_GridHomeOptimise, home);
		*isScheduled = true;
		stats->addStat(token->getTimeStatsId(), tick->getTimeInInterval() + randomTime);
	}
}

void COHDA_GridHomeOptimise(ConventionalHome* home) {
	GridHome* gridhome = (GridHome*) home;
	// get data
	bool increasedLambda = false;
	std::vector<COHDA_Token*>* tokens = gridhome->getData<std::vector<COHDA_Token*> >("tokens");
	int* lastTick = gridhome->getData<int>("LastTick", false);
	if(lastTick == NULL) {
		lastTick = new int(-1000);// must be less than -interval
		gridhome->addDataValue("LastTick", lastTick);
	}
	int* convStatsId = gridhome->getData<int>("conv_stats", false);
	if(convStatsId == NULL) {
		convStatsId = new int(MyStats::Get()->registerStat("convergence", "piep", 1));
		gridhome->addDataValue("conv_stats", convStatsId);
	}

	std::vector<double>* thetasFlexSolution = gridhome->getData<std::vector<double> >("thetasFlexSolution");
	std::vector<double>* thetasOnOffSolution = gridhome->getData<std::vector<double> >("thetasOnOffSolution");
	COHDA_ScheduleSelectionStore* sssFlex = gridhome->getData<COHDA_ScheduleSelectionStore>("lambdaFlex");
	COHDA_ScheduleSelectionStore* sssOnOff = gridhome->getData<COHDA_ScheduleSelectionStore>("lambdaOnOff");
	std::vector<Device*>* adaptableDevicesOnOff = gridhome->getData<std::vector<Device*> >("adaptableDevicesOnOff");
	std::vector<Device*>* adaptableDevicesFlex = gridhome->getData<std::vector<Device*> >("adaptableDevicesFlex");
	COHDA_WorkingMemory* kappaOwn = gridhome->getData<COHDA_WorkingMemory>("WorkingMemory", false);
	std::vector<std::vector<int> >* thetaMatrixStates = gridhome->getData<std::vector<std::vector<int> > >("thetaMatrixStates");
	std::vector<std::vector<int> >* thetaMatrixConsumptions = gridhome->getData<std::vector<std::vector<int> > >("thetaMatrixConsumptions");
	std::vector<double>* thetasOnOff = gridhome->getData<std::vector<double> >("thetasOnOff");
	std::vector<std::vector<double> >* thetasFlex = gridhome->getData<std::vector<std::vector<double> > >("thetasFlex");
	bool* isScheduled = gridhome->getData<bool>("isScheduled");
	*isScheduled = false;
	if(kappaOwn == NULL) {
		Log::i(gridhome->getId(), "leeres WM wird erstellt");
		kappaOwn = new COHDA_WorkingMemory();
		gridhome->addDataValue("WorkingMemory", kappaOwn);
	}
	bool workingMemoryRefreshed = false;
	double consumption = gridhome->getConsumption();
	bool wasComplete = true;
	bool wasBest = true;
	uint gridHomeCount = MyConfig::Get()->getDouble("homes") * MyConfig::Get()->getDouble("gridhome_share");

	Ipv4Address adr = gridhome->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	std::string adrString = Helper::toString(adr);

	COHDA_Token* firstToken = NULL;
	if(Tick::Get()->getTick() > *lastTick) {	//find possible consumptions (thetas)
		Log::i(gridhome->getId(), "delete WorkingMemory");
		delete kappaOwn;
		kappaOwn = new COHDA_WorkingMemory();
		gridhome->addDataValue("WorkingMemory", kappaOwn);
		adaptableDevicesFlex->clear();
		adaptableDevicesOnOff->clear();
		std::map<std::string, Device*>::iterator devIt;
		std::map<std::string, Device*>* devices = gridhome->getDevices();
		for(devIt = devices->begin(); devIt != devices->end(); ++devIt) {
			if(devIt->second->isAdaptable()) {
				if(devIt->second->getAdaption().second != NULL) {
					adaptableDevicesOnOff->push_back(devIt->second);
					Log::i(gridhome->getId(), "device " + devIt->first + " is OnOff adaptable");
				} else if(devIt->second->getAdaption().first != NULL) {
					adaptableDevicesFlex->push_back(devIt->second);
					Log::i(gridhome->getId(), "device " + devIt->first + " is Flex adaptable");
				}
			}
		}

		// reset data
		thetasOnOff->clear();
		thetasFlex->clear();
		thetaMatrixStates->clear();
		thetaMatrixConsumptions->clear();
		sssFlex->clear();
		sssOnOff->clear();


		if(adaptableDevicesFlex->size() > 0)
			calcThetaFlex(adaptableDevicesFlex, thetasFlex);
		else 
			thetasFlex->push_back(std::vector<double>(3, 0));
		for(uint i = 0; i < thetasFlex->size(); i++) {
			consumption = consumption - (*thetasFlex)[i][2];
		}
		if(adaptableDevicesOnOff->size() > 0)
			calcThetaOnOff(std::vector<int>(), std::vector<int>(), 0, adaptableDevicesOnOff, thetaMatrixStates, thetaMatrixConsumptions, thetasOnOff, consumption);
		else
			thetasOnOff->push_back(consumption);

		if(sssOnOff->getLambdaAmount() == 0)
			sssOnOff->addScheduleStorage(0, new std::vector<double>(adaptableDevicesOnOff->size(), 0));
		if(sssFlex->getLambdaAmount() == 0)
			sssFlex->addScheduleStorage(0, new std::vector<double>(adaptableDevicesFlex->size(), 0));
		#ifndef	TURBO
			std::stringstream ss;
			ss << "possible Schedules (On/Off): ";
			for(uint i = 0; i < thetasOnOff->size(); i++) {
				ss << thetasOnOff->at(i) << ", ";
			}
			Log::i(gridhome->getId(), ss.str());
		#endif
		thetasFlexSolution->clear();
		for(uint i = 0; i < adaptableDevicesFlex->size(); i++) {
			thetasFlexSolution->push_back((*adaptableDevicesFlex)[i]->getConsumption());
		}
		thetasOnOffSolution->clear();
		for(uint i = 0; i < adaptableDevicesOnOff->size(); i++) {
			thetasOnOffSolution->push_back((*adaptableDevicesOnOff)[i]->getState());
		}
		(*lastTick) = Tick::Get()->getTick();
		gridhome->addDataValue("LastTick", lastTick);
	}

	bool sendToken = false;
	if(!kappaOwn->getConfigurationNice()->contains(adrString)) {	// Eigenes theta hinzufügen wenn es noch nicht drinn ist.
		Log::i(gridhome->getId(), "eigenen Fahrplan hinzufügen");
		COHDA_ScheduleSelection omegaOwn;
		omegaOwn.setLambda(0);				//counter = 0
		omegaOwn.setTheta((*thetasOnOff)[0]);		//the first theta
		kappaOwn->getConfigurationNice()->setScheduleSelection(adrString, &omegaOwn);
		kappaOwn->getSolutionCandidateNice()->setAdrString(adrString);
		kappaOwn->getSolutionCandidateNice()->setConfiguration(kappaOwn->getConfigurationNice());
//		kappaOwn->setZeta(kappaOwn->);//TODO darf so nicht gemacht werden, da zeta vom Server kommen muss.
		wasComplete = false;
		wasBest = false;
		workingMemoryRefreshed = true;
	} else if(gridHomeCount > kappaOwn->getConfigurationNice()->getSize()) {
		#ifndef	TURBO
			std::stringstream ss;
			ss << "homes " << gridHomeCount << ", Configuration.size: " << kappaOwn->getConfigurationNice()->getSize();
			Log::i(gridhome->getId(), ss.str());
		#endif
		wasComplete = false;
	}
	std::string oldSolutionCreator = kappaOwn->getSolutionCandidateNice()->getAdrString();
	if(tokens->size() != 0) {
		if(firstToken == NULL) {
			firstToken = (*tokens)[0];
		}
		kappaOwn->setZeta(tokens->at(0)->getWorkingMemoryNice()->getZeta());
		Log::i(gridhome->getId(), "tokens->size(): ", tokens->size());
		std::map<std::string, COHDA_ScheduleSelection>* gOmegaOwn = kappaOwn->getConfigurationNice()->getGOmegaNice();
		COHDA_SolutionCandidate* gammaOwn = kappaOwn->getSolutionCandidateNice();
		double errorOwnSC = -1;
		errorOwnSC = COHDA_error(kappaOwn->getZeta()*kappaOwn->getConfigurationNice()->getSize(), gammaOwn->getConfigurationNice()->getTotalConsumption());
		if(errorOwnSC == -1)
			throw("invalid error function chosen");
		double errorTokenSC;
		for(uint i = 0; i < tokens->size(); i++) {

			std::map<std::string, COHDA_ScheduleSelection>* temp = (*tokens)[i]->getWorkingMemoryNice()->getConfigurationNice()->getGOmegaNice();
			std::map<std::string, COHDA_ScheduleSelection>::iterator it;
			int nf = 0, wo = 0;
			for(it = temp->begin(); it != temp->end(); ++it) {//Integration beider Configurationen
				bool wasOlder = false;
				bool notFound = false;
				if(gOmegaOwn->find(it->first) == gOmegaOwn->end()) {
					notFound = true;
//					Log::i(gridhome->getId(), "Not Found");
					wasComplete = false;
					nf++;
				} else if(it->second.getLambda() > (*gOmegaOwn)[it->first].getLambda()) {
					wasOlder = true;
					wo++;
//					Log::i(gridhome->getId(), "was Older");
				}
				if(notFound == true || wasOlder == true) {
//					Log::i(gridhome->getId(), "will be replaced");
					kappaOwn->getConfigurationNice()->setScheduleSelection(it->first, &it->second);
//					std::cout << it->first << ": " << kappaOwn->getConfigurationNice()->getScheduleSelectionNice(it->first)->getLambda() << std::endl;
					workingMemoryRefreshed = true;
				}
			}
			#ifndef	TURBO
				std::stringstream ss;
				ss << "Refreshed gOmega and handle token " << i << ", sc " << (*tokens)[i]->getWorkingMemoryNice()->getSolutionCandidateNice()->getAdrString() << ": " << nf << " not found, " << wo << " were older. New size " << gOmegaOwn->size();
				Log::i(gridhome->getId(), ss.str());
				Log::i(gridhome->getId(), "Refresh Solution Candidate"); //Compare solution candidates
			#endif


			COHDA_SolutionCandidate* gammaToken = (*tokens)[i]->getWorkingMemoryNice()->getSolutionCandidateNice();
			errorTokenSC = COHDA_error(kappaOwn->getZeta()*kappaOwn->getConfigurationNice()->getSize(), gammaToken->getConfigurationNice()->getTotalConsumption());
			bool complete = false;
			if(gammaToken->getConfigurationNice()->getSize() == gridHomeCount)
				complete = true;
			bool isBigger = false;
			if(gammaToken->getConfigurationNice()->getSize() > gammaOwn->getConfigurationNice()->getSize())
				isBigger = true;
			bool isBetter = false;
			if(errorTokenSC < errorOwnSC)
				isBetter = true;
			bool isEqual = false;
			if(errorTokenSC == errorOwnSC)
				isEqual = true;
			bool hasHigherIP = false;
			if(gammaToken->getAdrString().empty() || Ipv4Address(gammaToken->getAdrString().c_str()).Get() > Ipv4Address(gammaOwn->getAdrString().c_str()).Get())
				hasHigherIP = true;

			#ifndef	TURBO
				std::stringstream sstr;
				(complete == true) ? sstr << "complete, " : sstr << "";
				(isBigger == true) ? sstr << "isBigger, " : sstr << "";
				(isBetter == true) ? sstr << "isBetter, " : sstr << "";
				(isEqual == true) ? sstr << "isEqual (Error: " << errorTokenSC << ", " : sstr << "";
				(hasHigherIP == true) ? sstr << "hasHigherIP (" << gammaToken->getAdrString() << " compared to own: " << kappaOwn->getSolutionCandidateNice()->getAdrString() << ") " : sstr << "";
				Log::i(gridhome->getId(), sstr.str());
			#endif
			if(complete && (isBigger || isBetter || (isEqual && hasHigherIP))) {
				kappaOwn->setSolutionCandidate(gammaToken);
				errorOwnSC = errorTokenSC;
				std::vector<double>* ss = sssFlex->getStorage(gammaToken->getConfigurationNice()->getScheduleSelectionNice(adrString)->getLambda());
				thetasFlexSolution->clear();
				for(uint j = 0; j < adaptableDevicesFlex->size(); j++) {
					thetasFlexSolution->push_back((*ss)[j]);
				}

				ss = sssOnOff->getStorage(gammaToken->getConfigurationNice()->getScheduleSelectionNice(adrString)->getLambda());
				thetasOnOffSolution->clear();
				for(uint j = 0; j < adaptableDevicesOnOff->size(); j++) {
					thetasOnOffSolution->push_back((*ss)[j]);
				}
				sendToken = true;

			} else if(complete) {	//If same solution candidate
				if(Ipv4Address(gammaToken->getAdrString().c_str()).Get() == Ipv4Address(gammaOwn->getAdrString().c_str()).Get()) {//same address
					std::map<std::string, COHDA_ScheduleSelection>::iterator it;
					if(gammaToken->getConfigurationNice()->getScheduleSelectionNice(gammaToken->getAdrString())->getLambda() != gammaOwn->getConfigurationNice()->getScheduleSelectionNice(gammaOwn->getAdrString())->getLambda()) {
						Log::i(gridhome->getId(), "Another Solution Candidate");
					}
				}
			}
		}
		if(workingMemoryRefreshed == true) {
			Log::i(gridhome->getId(), "Optimize");
			double error = 0; //kappaOwn->getZeta(),
			if(kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getSize() == gridHomeCount) {	//TODO kontrollieren, ob wm größer geworden ist (
				error = COHDA_error(kappaOwn->getZeta()*kappaOwn->getConfigurationNice()->getSize(), kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getTotalConsumption());
				Log::i(gridhome->getId(), "was complete and the error was ", error);
			} else
				error = 1.79769e+308;
			double error_temp = 1.79769e+308;
			COHDA_ScheduleSelection omega_temp;
			COHDA_Configuration* gOmega_temp = new COHDA_Configuration();
			std::vector<double> thetasFlexSolutionTemp;
			std::vector<double> thetasOnOffSolutionTemp;

			gOmega_temp->setConfiguration(kappaOwn->getConfigurationNice()->getGOmegaNice());

			//Search vor the best two Solutions
			if(adaptableDevicesOnOff->size() > 0)
				COHDA_AdaptOnOff(gridhome->getId(), thetasOnOff, thetaMatrixStates, thetaMatrixConsumptions, kappaOwn->getZeta()*kappaOwn->getConfigurationNice()->getSize(), kappaOwn, adrString);
			#ifndef	TURBO
				std::stringstream ss;
				ss << "possible On/Off Schedules (amount " << thetasOnOff->size() << "): ";
				for(uint i = 0; i < thetasOnOff->size(); i++) {
					ss << thetasOnOff->at(i) << ", ";
				}
				Log::i(gridhome->getId(), ss.str());
			#endif
			double thetaOnOffMin = 0;
			for(uint k = 0; k < thetasOnOff->size(); k++) {	//theta is reduzed to min. one and max. two solutions
				Log::i(gridhome->getId(), "thetasOnOff: ", (*thetasOnOff)[k]);
				if(k==0)
					thetaOnOffMin = (*thetasOnOff)[k];
				gOmega_temp->getScheduleSelectionNice(adrString)->setTheta((*thetasOnOff)[k]);
				if(adaptableDevicesFlex->size() > 0) {
					#ifndef	TURBO
						std::stringstream ss;
						ss << "possible Schedules (Flex): ";
						for(uint j = 0; j < thetasFlex->at(0).size(); j++) {
							ss << thetasFlex->at(0).at(j) << ", ";
						}
						Log::i(gridhome->getId(), ss.str());
					#endif
					//gOmega_temp->getScheduleSelectionNice(adrString)->setTheta((*thetasOnOff)[k]);
					COHDA_AdaptFlex((*thetasOnOff)[k], thetasFlex, kappaOwn->getZeta()*gOmega_temp->getSize(), gOmega_temp, adrString, &thetasFlexSolutionTemp);
				} else {
					Log::i(gridhome->getId(), "no optimisation in this household");
				}
				error_temp = COHDA_error(kappaOwn->getZeta()*gOmega_temp->getSize(), gOmega_temp->getTotalConsumption());
				if(wasComplete == false || error_temp < error || (error_temp == error && (*thetasOnOff)[k] < thetaOnOffMin)) {//neue Lösung gefunden -> neuer Solution Candidate
					Log::i(gridhome->getId(), "new solution! error = ", error_temp);
					if(increasedLambda == false) {
						gOmega_temp->getScheduleSelectionNice(adrString)->increaseLambda();
						increasedLambda = true;
					}
					wasBest = false;
					error = error_temp;
					kappaOwn->getSolutionCandidateNice()->setAdrString(adrString);
					kappaOwn->getSolutionCandidateNice()->setConfiguration(gOmega_temp);
					kappaOwn->setConfiguration(gOmega_temp);
					//Geräten Fahrplan mitteilen
					#ifndef	TURBO
						std::stringstream ss;
					#endif
					if(adaptableDevicesOnOff->size() > 0) {
						std::vector<int> states = (*thetaMatrixStates)[k];
						std::vector<int> consumptions = (*thetaMatrixConsumptions)[k];
						thetasOnOffSolution->clear();
						#ifndef	TURBO
							ss << "Store On/Off Devices: ";
						#endif
						for(uint j = 0; j < adaptableDevicesOnOff->size(); j++) {
							#ifndef	TURBO
								ss << "Device" << j << ": state " << states[j] << "; ";
							#endif
							thetasOnOffSolution->push_back(states[j]);
						}
						#ifndef	TURBO
							ss.seekp(-2, ss.cur);
							ss << "; ";
						#endif
					}
					if(adaptableDevicesFlex->size() > 0) {
						thetasFlexSolution->clear();
						#ifndef	TURBO
							ss << "Store Flex Devices: ";
						#endif
						for(uint j = 0; j < adaptableDevicesFlex->size(); j++) {
							#ifndef	TURBO
								ss << "Device" << j << ": " << thetasFlexSolutionTemp.at(j) << "W, ";
							#endif
							thetasFlexSolution->push_back(thetasFlexSolutionTemp[j]);
						}
						#ifndef	TURBO
							ss.seekp(-2, ss.cur);
							ss << "; ";
						#endif
					}
					#ifndef	TURBO
						ss << "consumption of solution candidate: " << kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getTotalConsumption() << "W";
						Log::i(gridhome->getId(), ss.str());
					#endif
				} else {	//own Theta = theta from the solution candidate
					//kappaOwn->getConfigurationNice()->setScheduleSelection(adrString, kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getScheduleSelectionNice(adrString));
				}
				if((*thetasOnOff)[k] < thetaOnOffMin)
					thetaOnOffMin = (*thetasOnOff)[k];
			}
			delete gOmega_temp;

			if(!wasComplete) {
				sendToken = true;
				Log::i(gridhome->getId(), "was older or not complete");
			} else if(!wasBest) {
				sendToken = true;
				Log::i(gridhome->getId(), "was not best");
			} else {
				Log::i(gridhome->getId(), "no better solution found;");// old SolutionAlreadySent = ", *SolutionAlreadySent);
			}
			sendToken = true;
			if(Simulator::Now().GetSeconds() >= (((*lastTick + 1) * MyConfig::Get()->getInt("interval")) * 60) - 5) {//5 seconds before the next tick, no token will be send
				sendToken = false;
				MyStats::Get()->addStat(*convStatsId, 0);
			}
			Log::i(gridhome->getId(), "new solution creator: ", kappaOwn->getSolutionCandidateNice()->getAdrString());
		} else
			sendToken = false;
	}
	if(firstToken != NULL && sendToken == true) {
		//only once refresh the sssOnOff and sssFlex
		std::vector<double>* ssFlexTemp = new std::vector<double>;
		for(uint i = 0; i < adaptableDevicesFlex->size(); i++) {
			ssFlexTemp->push_back((*thetasFlexSolution)[i]);
		}
		std::vector<double>* ssOnOffTemp = new std::vector<double>;
		for(uint i = 0; i < adaptableDevicesOnOff->size(); i++) {
			ssOnOffTemp->push_back((double) (*thetasOnOffSolution)[i]);
		}
		int lambda = kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getScheduleSelectionNice(adrString)->getLambda();
		sssFlex->addScheduleStorage(lambda, ssFlexTemp);
		sssOnOff->addScheduleStorage(lambda, ssOnOffTemp);
		#ifndef TURBO
			std::stringstream ssss;
			ssss << "store lambda: " << lambda << ", ssOnOffTemp: ";
			for (uint i = 0; i < ssOnOffTemp->size(); i++) {
				ssss << ssOnOffTemp->at(i) << ", ";
			}
			Log::i(gridhome->getId(), ssss.str());
		#endif

		std::vector<std::string>* recipients = firstToken->getRecipients(adrString);

		#ifndef TURBO
			std::stringstream ss;
			ss << "send " << recipients->size() << " messages to ";
		#endif
		//		Adress + lambda + theta				(Address is 32 bit (ip). In simulation, its a string)
		int ssSize = 4 + 4 + 8;
		//				zeta+	Anzahl der ScheduleSelections * (size(theta)+size(lambda)) + adress
		float tokenSize = 4 + (kappaOwn->getConfigurationNice()->getSize() + kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getSize()) * ssSize + 4;
		MyStats* stats = MyStats::Get();
		for(uint i = 0; i < recipients->size(); i++) {
			#ifndef	TURBO
				ss << recipients->at(i) << ", ";
			#endif
			COHDA_Token* newToken = new COHDA_Token(firstToken->getTimeStatsId(), firstToken->getMsgStatsId(), firstToken->getDataStatsId(), firstToken->getOverlayNet(), adrString, kappaOwn);
			stats->increaseStat(firstToken->getDataStatsId(), tokenSize);
			Time t = MilliSeconds(0);
			if(recipients->size()*newToken->getPacket()->GetSize() >= 192048)
				t = MilliSeconds(60 * i);
			else if(MyConfig::Get()->getString("com_net") == "lte" && newToken->getPacket()->GetSize() < (uint) MyConfig::Get()->getInt("homes")*40) {//otherwise, K>4 & homes=1000 does not converge wit hplc
				t = MicroSeconds(i * MyConfig::Get()->getInt("schedule_time_ms") * MyConfig::Get()->getInt("backoff_percentage") * 10);
			}
			gridhome->scheduleSend(t, (*recipients)[i], newToken->getPacket());
		}
		#ifndef TURBO
			ss << "(C: ";
			std::map<std::string, COHDA_ScheduleSelection>::iterator it;
			std::map<std::string, COHDA_ScheduleSelection>* temp = kappaOwn->getConfigurationNice()->getGOmegaNice();
			for(it = temp->begin(); it != temp->end(); ++it) {
				ss << it->first << ": " << it->second.getTheta() << " W, ";
			}
			ss << ")  (SC: ";
			temp = kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getGOmegaNice();
			for(it = temp->begin(); it != temp->end(); ++it) {
				ss << it->first << ": " << it->second.getTheta() << " W, ";
			}
			ss << ")";
			Log::i(gridhome->getId(), ss.str());
		#endif
	}
	for(uint i = 0; i < tokens->size(); i++) {
		delete (*tokens)[i];
	}
	tokens->clear();
}

static void COHDA_Shift(std::vector<void*>* params) {
	// just turn it on
	DeviceEvent* event = (DeviceEvent*) (*params)[1];
	event->getDevice()->setState(DEV_ON);
	event->getDevice()->tick();
}

static void calcThetaOnOff(std::vector<int> consumptions, std::vector<int> states, int devNumber, std::vector<Device*>* devices, std::vector<std::vector<int> >* matrixStates,
		std::vector<std::vector<int> >* matrixConsumptions, std::vector<double>* thetas, double totalConsumption) {
	if(devices->size() > 0) {
		std::pair<AdaptionFlex*, AdaptionOnOff*> adaption = (*devices)[devNumber]->getAdaption(); //TODO testen ob es mit mehreren Devices Funktioniert
		if((uint) devNumber < devices->size() - 1) { //Wenn es noch nicht das letzte Gerät ist
			if(adaption.second != NULL) {
				consumptions.push_back(adaption.second->getCurrent()); // keine anpassung
				states.push_back((*devices)[devNumber]->getState());
				calcThetaOnOff(consumptions, states, devNumber + 1, devices, matrixStates, matrixConsumptions, thetas, totalConsumption);
				consumptions.pop_back();
				consumptions.push_back(adaption.second->getValue()); // die anpassung um value
				states.pop_back();
				states.push_back(
						((*devices)[devNumber]->getState() == DEV_ON) ? DEV_OFF : DEV_ON);
				calcThetaOnOff(consumptions, states, devNumber + 1, devices, matrixStates, matrixConsumptions, thetas, totalConsumption);
			}
		} else {	//Wenn es das letzte Gerät ist
			if(adaption.second != NULL) {
				int devCons[] = { adaption.second->getCurrent(), adaption.second->getValue() };
				int devStates[] = { (*devices)[devNumber]->getState(),
						((*devices)[devNumber]->getState() == DEV_ON) ? DEV_OFF : DEV_ON };//Device is switchable between these stages at this point
				for(int i = 0; i < 2; i++) {
					if(i > 0) {
						consumptions.pop_back();
						consumptions.push_back(devCons[1] + devCons[0]);
						states.pop_back();
						states.push_back(devStates[1]);
					} else {
						consumptions.push_back(devCons[0]);
						states.push_back(devStates[0]);
					}
					matrixStates->push_back(states);
					matrixConsumptions->push_back(consumptions);
					double total = totalConsumption - devCons[0]; //because the Current State is already in the totalConsumption
					for(uint j = 0; j < consumptions.size(); j++) {
						total += consumptions[j];
					}
					thetas->push_back(total);
				}
			}
		}
	}
}

static void calcThetaFlex(std::vector<Device*>* devices, std::vector<std::vector<double> >* thetasFlex) {
	for(uint i = 0; i < devices->size(); i++) {
		std::vector<double> states;
		std::pair<AdaptionFlex*, AdaptionOnOff*> adaption = (*devices)[i]->getAdaption(); //TODO testen ob es mit mehreren Devices Funktioniert
		if(adaption.first->isAdaptable()) {	//Only Flexible Devices
			states.push_back(adaption.first->getMinimum()); // keine anpassung
			states.push_back(adaption.first->getMaximum()); // mit anpassung
			states.push_back(adaption.first->getDesired()); // mit gewollter anpassung //TODO wird derzeit ignoriert
		}
		thetasFlex->push_back(states);
	}
}

}

#endif
