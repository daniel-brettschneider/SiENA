#ifndef COHDA_ADAPTONOFF_H_
#define COHDA_ADAPTONOFF_H_

#include <vector>
#include <string>
#include <climits>
#include "ns3/GridHome.h"
#include "ns3/MyConfig.h"
#include "ns3/DeviceEvent.h"
#include "ns3/DataBasis.h"
#include "ns3/Turbo.h"
#include "ns3/COHDA_WorkingMemory.h"
#include "ns3/COHDA_Configuration.h"


namespace ns3 {


//static void COHDA_AdaptOnOff(std::vector<std::vector<double> > thetasOnOff, std::vector<std::vector<int> > thetaMatrix, double zeta, COHDA_WorkingMemory* kappaOwn);
static double COHDA_errorOnOff(double desired, double consumption) {
	return consumption - desired;
}

static void COHDA_AdaptOnOff(std::string homeId, std::vector<double>* thetasOnOff, std::vector<std::vector<int> >* thetaMatrixStates, std::vector<std::vector<int> >* thetaMatrixConsumptions, double zeta, COHDA_WorkingMemory* kappaOwn, std::string adrString) {
	COHDA_ScheduleSelection omega_temp;
	COHDA_Configuration gOmega_temp;
	double bestNegSolution = -11.79769e+30;
	int bestNegSolutionTheta = -1;
	double bestPosSolution = 11.79769e+30;
	int bestPosSolutionTheta = -1;
	std::vector<double> thetasOnOffNew;
	std::vector<std::vector<int> > thetaMatrixStatesNew;
	std::vector<std::vector<int> > thetaMatrixConsumptionsNew;
	double error_temp;
	//kappaOwn->getConfigurationNice()->getScheduleSelectionNice(homeId)->getTheta(); kappaOwn->getConfigurationNice()->getTotalConsumption()
	//std::cout << "COHDA_AdaptOnOff: " << zeta << std::endl;
	double totalConsumption = kappaOwn->getConfigurationNice()->getTotalConsumption() - kappaOwn->getConfigurationNice()->getGOmegaNice()->at(adrString).getTheta();
	for(uint i = 0; i < thetasOnOff->size(); i++) {
		omega_temp.setTheta((*thetasOnOff)[i]);
		//gOmega_temp.omega.at(adrString) = omega_temp; //(std::pair<std::string, ScheduleSelection > (adrString, thetas.at(i)));
		gOmega_temp.setScheduleSelection(adrString, &omega_temp);
		error_temp = COHDA_errorOnOff(zeta,totalConsumption + gOmega_temp.getTotalConsumption());
		if(error_temp < 0 && bestNegSolution < error_temp) {
			bestNegSolutionTheta = i;
			bestNegSolution = error_temp;
		} else if (error_temp >= 0 && bestPosSolution > error_temp){
			bestPosSolutionTheta = i;
			bestPosSolution = error_temp;
		}
//				std::vector<double> temp = kappaOwn->getConfiguration().getTotalConsumption();
	}
//	std::vector<double> temp;
//	std::vector<std::vector<int> > temp2;
	if(bestNegSolutionTheta != -1){
		thetasOnOffNew.push_back((*thetasOnOff)[bestNegSolutionTheta]);
		thetaMatrixStatesNew.push_back((*thetaMatrixStates)[bestNegSolutionTheta]);
		thetaMatrixConsumptionsNew.push_back((*thetaMatrixConsumptions)[bestNegSolutionTheta]);
	}
	if(bestPosSolutionTheta != -1){
		thetasOnOffNew.push_back((*thetasOnOff)[bestPosSolutionTheta]);
		thetaMatrixStatesNew.push_back((*thetaMatrixStates)[bestPosSolutionTheta]);
		thetaMatrixConsumptionsNew.push_back((*thetaMatrixConsumptions)[bestPosSolutionTheta]);
	}
	*thetasOnOff = thetasOnOffNew;
	*thetaMatrixStates = thetaMatrixStatesNew;
	*thetaMatrixConsumptions = thetaMatrixConsumptionsNew;
//	for(uint i = 0; i < thetaMatrixStates->size(); i++) {
//		for(uint j = 0; j < thetaMatrixStates->at(i).size(); j++) {
//			std::cout << "(" << thetaMatrixStates->at(i).at(j) << "|" << thetaMatrixConsumptions->at(i).at(j) << ") ";
//		}
//		std::cout << std::endl;
//	}

//		//Ausgabe
//		std::stringstream ss;
//		ss << "Error" << i << " (" << gOmega_temp.getTotalConsumption()[0] << " W, " << gOmega_temp.getScheduleSelection(adrString).getTheta()[0] << "): " << error_temp << "; Current Error: " << error;
//		Log::i(homeId, ss.str());
}
}

#endif
