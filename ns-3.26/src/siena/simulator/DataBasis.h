#ifndef DATABASIS_H_
#define DATABASIS_H_

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include "ns3/Log.h"
#include "ns3/MySingleton.h"
#include "ns3/MyConfig.h"
#include <cstdlib>
#include <ctime>
#include <iostream>

#define CSV_PER_COLUMN 0
#define CSV_PER_COLUMN_SPLIT 1
#define CSV_PER_LINE 2

namespace ns3 {

class DataBasis : public MySingleton<DataBasis> {
	friend class MySingleton<DataBasis>;

private:
	static const std::string TAG;
	std::map<std::string, std::vector<std::vector<double>*>*> data;

public:
	virtual ~DataBasis();

	/**
	 * See filename!
	 * @param filename "data/[filename].[extension]"
	 * @param mode
	 * @return
	 */
	std::vector<std::vector<double>*>* get(std::string filename, std::string extension = "csv", int mode = CSV_PER_COLUMN, bool createMean = true,
			int lengthHintX = 0, int lengthHintY = 0, bool log = true);
	void preloadBinaryLineFile(std::string filename, std::string dataName, int x, int y);
	std::string getFileContent(std::string filename);
	void writeFile(std::string filename, std::string content);
	void printAvailableFiles();
	bool isFileAvailable(std::string filename);
	void convertFilesToBinary();

private:
	DataBasis() {
		//convertFilesToBinary();
	}
	void readCsv(std::string filename, std::string extension, int mode, bool createMean, int lengthHintX, int lengthHintY, bool log);
	void readCsvPerColumn(std::string name, std::istream* file, bool split, bool createMean, int lengthHintX, int lengthHintY);
	void readCsvPerLine(std::string name, std::istream* file, bool createMean, int lengthHintX, int lengthHintY);
	double parseDouble(std::string s);
	void createBinaryCSV(std::string filename);

};

}

#endif
