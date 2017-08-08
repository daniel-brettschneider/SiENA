#ifndef NS_3_22_SRC_COHDA_MODULES_CITYGRID_ALGO_CITYGRID_HELPER_H_
#define NS_3_22_SRC_COHDA_MODULES_CITYGRID_ALGO_CITYGRID_HELPER_H_

#include "ns3/HomeContainer.h"
#include "ns3/ServerContainer.h"
#include "ns3/GridHome.h"
//#include "ns3/COHDA_Configuration.h"
//#include "ns3/COHDA_WorkingMemory.h"
//#include "ns3/COHDA_AdaptOnOff.h"
//#include "ns3/COHDA_AdaptFlex.h"
#include "ns3/COHDA_ScheduleSelectionStore.h"
//#include "ns3/ServerContainer.h"
//#include "ns3/Turbo.h"
#include "ns3/DsmMode.h"

namespace ns3 {


static void COHDA_tidyUp(std::vector<void*>* params) {
	HomeContainer* homes = HomeContainer::Get();
	for(int i = 0; i < MyConfig::Get()->getInt("clusters"); i++) {
		std::vector<ConventionalHome*>* gridhomes = homes->getHomes(i, 2);
		for(uint j = 0; j < gridhomes->size(); j++) {
			GridHome* gridhome = (GridHome*) (*gridhomes)[j];
			delete gridhome->getData<bool>("isScheduled");
			delete gridhome->getData<int>("LastTick");
			delete gridhome->getData<std::vector<double> >("thetasFlexSolution");
			delete gridhome->getData<std::vector<double> >("thetasOnOffSolution");
			delete gridhome->getData<COHDA_ScheduleSelectionStore>("lambdaFlex");
			delete gridhome->getData<COHDA_ScheduleSelectionStore>("lambdaOnOff");
			delete gridhome->getData<COHDA_WorkingMemory>("WorkingMemory");
			delete gridhome->getData<std::vector<double> >("thetasOnOff");
			delete gridhome->getData<std::vector<std::vector<double> > >("thetasFlex");
			delete gridhome->getData<std::vector<std::vector<int> > >("thetaMatrixStates");
			delete gridhome->getData<std::vector<std::vector<int> > >("thetaMatrixConsumptions");
			delete gridhome->getData<std::vector<Device*> >("adaptableDevicesOnOff");
			delete gridhome->getData<std::vector<Device*> >("adaptableDevicesFlex");

			std::vector<COHDA_Token*>* tokens = gridhome->getData<std::vector<COHDA_Token*> >("tokens");
			for(uint k = 0; k < tokens->size(); k++) {
				delete (*tokens)[k];
			}
			delete tokens;
		}
	}

	std::vector<Server*>* servers = ServerContainer::Get()->getServers();
	for(uint i = 0; i < servers->size(); i++) {
		std::map<std::string, std::vector<std::string>*>* overlayNetwork = (*servers)[i]->getData<std::map<std::string, std::vector<std::string>*> >("overlay_net", false);
		std::map<std::string, std::vector<std::string>*>::iterator it;
		for(it = overlayNetwork->begin(); it != overlayNetwork->end(); ++it) {
			delete it->second;
		}
		delete overlayNetwork;
		delete (*servers)[i]->getData<int>("msg_stats");
		delete (*servers)[i]->getData<int>("time_stats");
		delete (*servers)[i]->getData<int>("data_stats");
	}
}

void COHDA_Gridhomefinaltick(GridHome* home) {
	std::vector<Device*>* adaptableDevicesOnOff = home->getData<std::vector<Device*> >("adaptableDevicesOnOff");
	std::vector<Device*>* adaptableDevicesFlex = home->getData<std::vector<Device*> >("adaptableDevicesFlex");
	COHDA_ScheduleSelectionStore* sssFlex = home->getData<COHDA_ScheduleSelectionStore>("lambdaFlex");
	COHDA_ScheduleSelectionStore* sssOnOff = home->getData<COHDA_ScheduleSelectionStore>("lambdaOnOff");
	COHDA_WorkingMemory* kappaOwn = home->getData<COHDA_WorkingMemory>("WorkingMemory");
	Ipv4Address adr = home->GetNode()->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
	std::string adrString = Helper::toString(adr);

	#ifndef TURBO
		std::stringstream ss;
	#endif
//		Log::i(gridhome->getId(), "adaptableDevicesFlex.size(): ", adaptableDevicesFlex.size());
	if(adaptableDevicesOnOff->size() > 0) {
		#ifndef	TURBO
			ss << "set On/Off: ";
		#endif
		for(uint j = 0; j < adaptableDevicesOnOff->size(); j++) {
			#ifndef TURBO
				ss << "Device" << j << " from " << adaptableDevicesOnOff->at(j)->getConsumption();
			#endif
//				if((int) adaptableDevicesOnOff[j]->getState() != thetasOnOffSolution->at(j)) { //nur für adaptable
			if((int) (*adaptableDevicesOnOff)[j]->getState() != (*sssOnOff->getStorage(kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getScheduleSelectionNice(adrString)->getLambda()))[j]) { //nur für adaptable
				(*adaptableDevicesOnOff)[j]->setState((*adaptableDevicesOnOff)[j]->getState() == DEV_ON ? DEV_OFF : DEV_ON);
				(*adaptableDevicesOnOff)[j]->tick();
			}
			#ifndef TURBO
				ss << "W to " << adaptableDevicesOnOff->at(j)->getConsumption() << "W, ";
			#endif
		}
		#ifndef TURBO
			ss.seekp(-2, ss.cur);
			ss << ";";
		#endif
	}  else {
		#ifndef TURBO
			ss << " no On/Off Device;";
		#endif
	}
	if(adaptableDevicesFlex->size() > 0) {
		#ifndef TURBO
			ss << " set Flex: ";
		#endif
		for(uint j = 0; j < adaptableDevicesFlex->size(); j++) {
			#ifndef TURBO
				ss << "Device" << j << " to " << (*sssFlex->getStorage(kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getScheduleSelectionNice(adrString)->getLambda()))[j] << "W, ";
			#endif
			(*adaptableDevicesFlex)[j]->setAdaptedConsumption((*sssFlex->getStorage(kappaOwn->getSolutionCandidateNice()->getConfigurationNice()->getScheduleSelectionNice(adrString)->getLambda()))[j]);
			Log::i(home->getId(), "set device to ", (*adaptableDevicesFlex)[j]->getConsumption());
			Log::i(home->getId(), "state: ", (*adaptableDevicesFlex)[j]->getState());
		}
		#ifndef TURBO
			ss.seekp(-2, ss.cur);
			ss << ";";
		#endif
	}else {
		#ifndef TURBO
			ss << " no Flex Device;";
		#endif
	}
	#ifndef TURBO
		Log::i(home->getId(), ss.str());
	#endif

	if(Tick::Get()->getTick() < home->getSimEnd()) {
		Simulator::Schedule(MilliSeconds(60000*MyConfig::Get()->getInt("interval")), &COHDA_Gridhomefinaltick, home);
	}
}

static void COHDA_scheduler_init(std::vector<void*>* params) {
	HomeContainer* homes = HomeContainer::Get();
	for(int i = 0; i < MyConfig::Get()->getInt("clusters"); i++) {
		std::vector<ConventionalHome*>* gridhomes = homes->getHomes(i, 2);
		for(uint j = 0; j < gridhomes->size(); j++) {
			Simulator::Schedule(MilliSeconds(MyConfig::Get()->getInt("interval") * 60000 - 5000), &COHDA_Gridhomefinaltick, (GridHome*) (*gridhomes)[j]);
		}
	}
}

static double COHDA_error(double desired, double consumption) {
	if (DsmMode::get()==DSM_Load_Shaping) {
		return (desired - consumption) * (desired - consumption);
	} else if (DsmMode::get()==DSM_Peak_Clipping){
		if (desired < consumption) {
			return (desired - consumption) * (desired - consumption) + 10000000000;
		} else {
			//return 0;
			return (desired - consumption) * (desired - consumption);
		}
	} else if (DsmMode::get()==DSM_Own_Consumption){
		if (desired < consumption) {
			return (desired - consumption) * (desired - consumption);
		} else {
			//return 0;
			return (desired - consumption) * (desired - consumption) + 10000000000;
		}
	} else {
		throw("no defined DSM Mode for COHDA");
	}
}

}

#endif
