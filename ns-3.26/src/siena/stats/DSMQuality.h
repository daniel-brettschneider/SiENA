#ifndef DSMQUALITY_H_
#define DSMQUALITY_H_

#include <vector>
#include <climits>
#include <cmath>
#include "ns3/Log.h"
#include "ns3/MyConfig.h"
#include "ns3/DataBasis.h"
#include "ns3/MyStats.h"
#include "ns3/DsmMode.h"

namespace ns3 {

static float cut(float result) {
	if (result < -1.0)
		result = -1;
	else if (result > 1.0)
		result = 1;
	return result;
}

static float max(std::vector<float>* data, int duration) {
	float max = INT_MIN;
	for(int i = 0; i < duration; i++) {
		if((*data)[i] > max)
			max = (*data)[i];
	}
	return max;
}

static float min(std::vector<float>* data, int duration) {
	float min = INT_MAX;
	for(int i = 0; i < duration; i++) {
		if((*data)[i] < min)
			min = (*data)[i];
	}
	return min;
}

static float sum(std::vector<float>* data, int duration) {
	float sum = 0;
	for(int i = 0; i < duration; i++) {
		sum += (*data)[i];
	}
	return sum;
}

static float avg(std::vector<float>* data, int duration) {
	return sum(data, duration) / (float) duration;
}

//static float qPC1(float CoMin, float CsMin, float CoMax, float CsMax) {
//	int min = 0;
//	if(CoMin < CsMin) //To get only positive values
//		min = CoMin;
//	else
//		min = CsMin;
//	CoMax = CoMax - min;
//	CsMax = CsMax - min;
//
//	if(CsMax < CoMax)
//		return cut(1 - (CsMax / CoMax));
//	else if(CsMax == CoMax)
//		return 0;
//	else
//		return cut((CoMax / CsMax) - 1);
//}
//
//static float qPC2(std::vector<float>* Co, std::vector<float>* Cs, float CoMax, float CsMax, int duration) {
//	if(CsMax < CoMax) {		//CsMax >= 0
//		float sumP1 = 0, sumEval = 0;
//		for(int i = 0; i < duration; i++) {
//			if((*Co)[i] > CsMax)
//				sumP1 += (*Co)[i] - CsMax;
//			sumEval += (*Co)[i];
//		}
//		return cut(sumP1 / sumEval);
//	} else if(CsMax > CoMax) { //CsMax >= 0
//		float sumP2 = 0, sumEval = 0;
//		for(int i = 0; i < duration; i++) {
//			if((*Cs)[i] > CoMax)
//				sumP2 += (*Cs)[i] - CoMax;
//			sumEval += (*Cs)[i];
//		}
//		return cut(- sumP2 / sumEval);
//	} else
//		return 0;
//}

static float qPC(float CoMin, float CsMin, float CoMax, float CsMax) {
	if (CoMax > 0 && CsMax <= 0 ) {
		return 1;
	} else if (CoMax > CsMax && CsMax > 0) {
		return (1- CsMax/CoMax);
	} else if (CsMax > CoMax && CoMax > 0) {
		return CoMax/CsMax - 1;
	} else if (CsMax > 0 && CoMax <= 0) {
		return -1;
	}
	return 0;
}

//static float qVF1(float CoMin, float CsMin, float CoAvg, float CsAvg) {
//	int min = 0;
//	if(CoMin < CsMin) //To get only positive values
//		min = CoMin;
//	else
//		min = CsMin;
//	CoAvg = CoAvg - min;
//	CsAvg = CsAvg - min;
//	CoMin = CsMin - min;
//	CsMin = CsMin - min;
//
//
//	float rs = 0;
//	if(CsAvg > 0)
//		rs = CsMin / CsAvg;
//	float ro = 1;
//	if(CoAvg > 0)
//		ro = CoMin / CoAvg;
//	if(ro <= rs && ro < 1)
//		return cut((rs - ro) / (1 - ro));
//	else
//		return cut((rs / ro) - 1);
//}
//
//static float qVF2(std::vector<float>* Co, std::vector<float>* Cs, float CoMin, float CsMin, int duration) {
//	if(CsMin > CoMin) {
//		float sumV1 = 0, sumEval = 0;
//		for(int i = 0; i < duration; i++) {
//			if(CsMin > (*Co)[i])
//				sumV1 += CsMin - (*Co)[i];
//			sumEval += (*Cs)[i];
//		}
//		return cut(sumV1 / sumEval);
//	} else if(CsMin < CoMin) {
//		float sumV2 = 0, sumEval = 0;
//		for(int i = 0; i < duration; i++) {
//			if(CoMin > (*Cs)[i])
//				sumV2 += CoMin - (*Cs)[i];
//			sumEval += (*Co)[i];
//		}
//		return cut(- sumV2 / sumEval);
//	} else
//		return 0;
//}

static float qVF(float CoMin, float CsMin, float CoAvg, float CsAvg) {
	if(CsMin == CoMin) {
		return 0;
	} else if (CsMin > CoMin) {
		return (CsMin - CoMin)/(CsAvg - CoMin);
	} else {
		return (CoMin - CsMin)/(CoAvg - CsMin);
	}
	return -1;
}

static float qEE(std::vector<float>* Co, std::vector<float>* Cs, int duration) {
	float sumCoCs = 0, sumCo = 0, sumCs = 0;
	for(int i = 0; i < duration; i++) {
		//sumCoCs += Co->at(i) - Cs->at(i);
		sumCo += (*Co)[i];
		sumCs += (*Cs)[i];
	}
	sumCoCs = sumCo - sumCs;
	if((sumCo <= 0 && sumCs > 0) || (sumCo < 0 && sumCs >= 0))
		return -1;
	else if((sumCo >= 0 && sumCs < 0) || (sumCo > 0 && sumCs <= 0))
		return 1;
	if (sumCo > 0 && sumCs > 0) {
		if(sumCo > sumCs)
			return sumCoCs / sumCo;
		else if(sumCo < sumCs)
			return sumCoCs / sumCs;
		else
			return 0;
	} else if (sumCo < 0 && sumCs < 0) {
		if(sumCo < sumCs)
			return sumCoCs / sumCo;
		else if(sumCo > sumCs)
			return sumCoCs / sumCs;
		else
			return 0;
	} else
		return 0;
}

static float qLG(float qEE) {
	return - qEE;
}

static float qLSH(std::vector<float>* Co1, std::vector<float>* Co2, std::vector<float>* Cs1, std::vector<float>* Cs2, double (*f)(double), int duration) {
	float sumCo = 0, sumCoQ = 0, sumCs = 0, sumCsQ = 0;
	for(int i = 0; i < duration; i++) {
		float addCo = 0, addCs = 0;
		if(f != NULL) {
			if(f((*Co1)[i] - (*Co2)[i]) > 0)
				addCo = f((*Co1)[i] - (*Co2)[i]);
			if(f((*Cs1)[i] - (*Cs2)[i]) > 0)
				addCs = f((*Cs1)[i] - (*Cs2)[i]);
		} else {
			if((*Co1)[i] - (*Co2)[i] > 0)
				addCo = (*Co1)[i] - (*Co2)[i];
			if((*Cs1)[i] - (*Cs2)[i] > 0)
				addCs = (*Cs1)[i] - (*Cs2)[i];
		}
		sumCo += addCo;
		sumCoQ += addCo * addCo;
		sumCs += addCs;
		sumCsQ += addCs * addCs;
	}
	//std::cout << "sumCs:" << sumCs << " sumCo:" << sumCo << std::endl;
	//std::cout << "sumCsQ:" << sumCsQ << " sumCoQ:" << sumCoQ << std::endl;
	if(sumCs < sumCo)
		return cut(1 - (sumCs / sumCo));
		//return 1 - (sumCsQ / sumCoQ);
	else if(sumCs > sumCo)
		return cut((sumCo / sumCs) - 1);
		//return (sumCoQ / sumCsQ) - 1;
	else
		return 0;
}

static float qLS1(std::vector<float>* Co, std::vector<float>* Cs, std::vector<float>* Cg, int duration) {
	return qLSH(Co, Cg, Cs, Cg, NULL, duration);
}

static float qLS2(std::vector<float>* Co, std::vector<float>* Cs, std::vector<float>* Cg, int duration) {
	return qLSH(Cg, Co, Cg, Cs, NULL, duration);
}

static float qLS3(std::vector<float>* Co, std::vector<float>* Cs, std::vector<float>* Cg, int duration) {
	return qLSH(Co, Cg, Cs, Cg, fabs, duration);
}

float* analyzeQuality(std::vector<std::vector<float>*>* grouped) {
	MyConfig* c = MyConfig::Get();
	DataBasis* data = DataBasis::Get();
	int interval = c->getInt("interval");
	int start = (c->getInt("quality_start") - 1) * 1440 / interval;
	int duration = c->getInt("duration") / interval - start;
	if(duration <= 0) {
		std::cerr << "quality start day after simulation end... set to 0..." << std::endl;
		start = 0;
		duration = c->getInt("duration") / interval;
	}

	std::string compareFile = "dsm/quality_compare";
	if(data->isFileAvailable("data/dsm/quality_compare_" + c->getString("identifier") + ".csv"))
		compareFile += "_" + c->getString("identifier");
	else
		Log::f("StatsAnalyzer", "compare file " + compareFile + " not available, fallback to standard file");
	std::vector<double>* compare = (*data->get(compareFile, "csv", CSV_PER_COLUMN, false, 1))[0];
	std::vector<double>* goal = DsmMode::Get()->getQualityGoal();
	float* values = new float[7];
	for (uint i = 0; i < (sizeof(values)); i++) {
		values[i] = 0;
	}
	int division = c->getInt("quality_division");
	if(division == 0) division = duration*interval;
	std::vector<float> Co(division/interval, 0);
	std::vector<float> Cs(division/interval, 0);
	std::vector<float> Cg(division/interval, 0);
	uint durationRemaining = division/interval;
	std::vector<std::vector<float>*> qualityTable;
	for(uint i = 0; i < (sizeof(values)); i++) {
		qualityTable.push_back(new std::vector<float>);
	}
//		std::stringstream s;
//		Log::f("Quality", s.str());
	//s << std::left << std::setw(16) << "compare: " << std::right << std::setw(4) << CoSum * 219 / (25 * 1440) << "kWh/y";

	for(uint j = 0; j < uint(duration); j = j + division / interval) {
		if(uint(duration) - j < durationRemaining)
			durationRemaining = uint(duration) - j;
		float CoSum = 0;
		for(uint i = 0; i < durationRemaining; i++) {
			Co[i] = (*compare)[(start + i + j) % compare->size()];
			Cg[i] = (*goal)[(start + i + j) % goal->size()];
			Cs[i] = 0;
			for(uint k = 0; k < grouped->size(); k++) {
				Cs[i] += (*(*grouped)[k])[start + i + j];
			}
			CoSum += Co[i];
		}
		float CoMax = max(&Co, durationRemaining);
		float CsMax = max(&Cs, durationRemaining);
		float CoMin = min(&Co, durationRemaining);
		float CsMin = min(&Cs, durationRemaining);
		float CoAvg = avg(&Co, durationRemaining);
		float CsAvg = avg(&Cs, durationRemaining);
		//std::cout << "C'max:" << CsMax << " C'avg:" << CsAvg << " C'min:" << CsMin << std::endl;
		//std::cout << "Cmax:" << CoMax << " Cavg:" << CoAvg << " Cmin:" << CoMin << std::endl;

		double weighting = (double) durationRemaining / (double) duration;
		float valuesTemp[7];
		//valuesTemp[0] = qPC1(CoMin, CsMin, CoMax, CsMax);
		//valuesTemp[1] = qPC2(&Co, &Cs, CoMax, CsMax, durationRemaining);
		valuesTemp[0] = qPC(CoMin, CsMin, CoMax, CsMax);
		//valuesTemp[2] = qVF1(CoMin, CsMin, CoAvg, CsAvg);
		//valuesTemp[3] = qVF2(&Co, &Cs, CoMin, CsMin, durationRemaining);
		valuesTemp[1] = qVF(CoMin, CsMin, CoAvg, CsAvg);
		valuesTemp[2] = qEE(&Co, &Cs, durationRemaining);
		valuesTemp[3] = qLG(qEE(&Co, &Cs, durationRemaining));
		valuesTemp[4] = qLS1(&Co, &Cs, &Cg, durationRemaining);
		valuesTemp[5] = qLS2(&Co, &Cs, &Cg, durationRemaining);
		valuesTemp[6] = qLS3(&Co, &Cs, &Cg, durationRemaining);
		//for (uint i = 0; i < 9; i++) {
		for (uint i = 0; i < sizeof(values)-1; i++) {
			qualityTable[i]->push_back(valuesTemp[i]);
			values[i] = values[i] + valuesTemp[i] * weighting;
		}
		qualityTable[sizeof(values)-1]->push_back(weighting);
	}
	//Special for EE and LG
//		std::vector<float> CoAll(duration, 0);
//		std::vector<float> CsAll(duration, 0);
//		for(uint j = 0; j < uint(duration); j++) {
//			CoAll[j] = (float) (*compare)[j];
//			for(uint k = 0; k < grouped->size(); k++) {
//				CsAll[j] += (*(*grouped)[k])[start + j];
//			}
//			std::cout << CsAll[j] << " " ;
//		}
//		std::cout << std::endl;
//		for(uint j = 0; j < uint(duration); j++) {
//			std::cout << CoAll[j] << " " ;
//		}
//		std::cout << std::endl;
//		values[2] = qEE(&CoAll, &CsAll, duration);
//		values[3] = qLG(qEE(&CoAll, &CsAll, duration));

	std::vector<std::string> saveIds = Helper::split(c->getString("save_only_these_stats"), ',');
	if(!c->getBool("filter_stats") || Helper::contains(&saveIds, "quality")) {
		std::vector<std::string> headers;
		//headers.push_back("PC1");
		//headers.push_back("PC2");
		headers.push_back("PC");
		//headers.push_back("VF1");
		//headers.push_back("VF2");
		headers.push_back("VF");
		headers.push_back("EE");
		headers.push_back("LG");
		headers.push_back("LSp");
		headers.push_back("LSn");
		headers.push_back("LS");
		headers.push_back("weighting");
		MyStats::Get()->exportCsv("quality", &headers, &qualityTable);
		for(uint i = 0; i < sizeof(values); i++) {
			delete qualityTable[i];
		}
	}

	return values;
}

}

#endif
