#ifndef COHDA_ADAPT_FLEX_H_
#define COHDA_ADAPT_FLEX_H_

#include <vector>
#include <string>
#include <climits>
#include "ns3/GridHome.h"
#include "ns3/MyConfig.h"
#include "ns3/DeviceEvent.h"
#include "ns3/DataBasis.h"
#include "ns3/Turbo.h"
#include "ns3/COHDA_WorkingMemory.h"
//#include "ns3/COHDA_Configuration.h"

namespace ns3 {

static void COHDA_AdaptFlex(double thetaOnOff, std::vector<std::vector<double> >* thetasFlex, double zeta, COHDA_Configuration* gOmegaTemp, std::string adrString/*, std::vector<double>* thetasFlexSolution*/, std::vector<double>* thetasFlexSolutionNew) {
	std::map<std::string, COHDA_ScheduleSelection>::iterator it;
	double currentConsumption = 0;
	for(it = gOmegaTemp->getGOmegaNice()->begin(); it != gOmegaTemp->getGOmegaNice()->end(); ++it) {
//		std::cout << "WM ScheduleSelection of " << it->first << ": " << it->second.getTheta() << " W"<< std::endl;
		currentConsumption = currentConsumption + it->second.getTheta();
	}
//	std::cout << "current Total Consumption: " << currentConsumption << std::endl;
	currentConsumption = currentConsumption - (*gOmegaTemp->getGOmegaNice())[adrString].getTheta();
//	std::cout << "current own Consumption: " << temp.at(adrString).getTheta() << std::endl;
	double targetFlex = zeta - thetaOnOff - currentConsumption;
//	std::cout << "thetaOnOff: " << thetaOnOff << std::endl;
//	std::cout << "targetFlex1: " << targetFlex << std::endl;
	for(uint j = 0; j < thetasFlex->size(); j++) {	//for each flexible Device
//		std::cout << "thetasFlexSolution->at(j) : " << thetasFlexSolution->at(j) << std::endl;
//		targetFlex = targetFlex + thetasFlexSolution->at(j);
//		std::cout << "targetFlex2: " << targetFlex << std::endl;
		double min = (*thetasFlex)[j][0];//0 = minimum, 1 = maximum, 2 = desired
		double max = (*thetasFlex)[j][1];
		if(targetFlex < 0) {
			if(min < targetFlex) {		//kann alleine mit diesem device befriedigt werden
				thetasFlexSolutionNew->push_back(targetFlex);
			} else {
				thetasFlexSolutionNew->push_back(min);
			}
		} else {
			if(max > targetFlex) {
				thetasFlexSolutionNew->push_back(targetFlex);
			} else {
				thetasFlexSolutionNew->push_back(max);
			}
		}
//		std::cout << "Is set to thetasFlexSolutionNew->at(j) = " << thetasFlexSolutionNew->at(j) << std::endl;
		gOmegaTemp->getScheduleSelectionNice(adrString)->setTheta(gOmegaTemp->getScheduleSelectionNice(adrString)->getTheta() + (*thetasFlexSolutionNew)[j]);
		targetFlex = targetFlex - (*thetasFlexSolutionNew)[j];
//		std::cout << "gOmegaTemp->getTotalConsumption(): " << gOmegaTemp->getTotalConsumption() << std::endl;
	}
//	if(targetFlex == 0) {
////		std::cout << "Best Solution found" << std::endl;
//	}
}
}

#endif
