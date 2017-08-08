#ifndef STATSANALYZER_H_
#define STATSANALYZER_H_

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include "ns3/Log.h"
#include "ns3/MyStats.h"
#include "ns3/StatsExporter.h"
#include "ns3/MyConfig.h"
#include "ns3/DataBasis.h"
#include "ns3/DSMQuality.h"

namespace ns3 {

class StatsAnalyzer {

public:
	static const std::string TAG;
	static const std::string DEPENDENCIES;
	static void analyze();

private:
	static void addValueToMap(std::map<std::string, double>* map, std::string key, double value);
	static void printTable(std::string name, std::string* headers, std::map<std::string, double>** data, int n, int decimalPlaces = 1);
	static double kWhpy(double value, double duration, double interval, double amount);
	static void saveCSV(std::vector<std::string>* csvHeaders, std::vector<std::vector<float>*>* csvData, std::string* headers,
			std::map<std::string, double>** data, int n);
	static void saveCSV(std::vector<std::string>* header, std::vector<std::vector<float>*>* csvData, std::string name, double value);
};

}

#endif
