#include "../../siena/simulator/DataBasis.h"

namespace ns3 {

const std::string DataBasis::TAG = "DataBasis";

DataBasis::~DataBasis() {
	std::map<std::string, std::vector<std::vector<double>*>*>::iterator it;
	for(it = data.begin(); it != data.end(); ++it) {
		for(uint i = 0; i < it->second->size(); i++) {
			delete (*it->second)[i];
		}
		delete it->second;
	}
}

std::string DataBasis::getFileContent(std::string filename) {
	Log::f(TAG, "reading file " + filename + "...");
	std::ifstream file(filename.c_str());
	std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return str;
}

void DataBasis::writeFile(std::string filename, std::string content) {
	Log::f(TAG, "writing file " + filename + "...");
	std::ofstream file(filename.c_str());
	file << content;
}

void DataBasis::printAvailableFiles() {
	Log::i(TAG, "available files:");
	std::map<std::string, std::vector<std::vector<double>*>*>::iterator it;
	for(it = data.begin(); it != data.end(); ++it) {
		Log::i(TAG, "\t" + it->first);
	}
}

std::vector<std::vector<double>*>* DataBasis::get(std::string filename, std::string extension, int mode, bool createMean, int lengthHintX, int lengthHintY, bool log) {
	if(data.find(filename) == data.end())
		readCsv(filename, extension, mode, createMean, lengthHintX, lengthHintY, log);
	if(mode != CSV_PER_COLUMN_SPLIT)
		return data[filename];
	return NULL;
}

void DataBasis::readCsv(std::string filename, std::string extension, int mode, bool createMean, int lengthHintX, int lengthHintY, bool log) {
	if(log)
		Log::f(TAG, "reading file " + filename + "." + extension + "...");

	std::ifstream file(("data/" + filename + "." + extension).c_str());
	if(file) {
		if(mode == CSV_PER_COLUMN)
			readCsvPerColumn(filename, &file, false, createMean, lengthHintX, lengthHintY);
		else if(mode == CSV_PER_COLUMN_SPLIT)
			readCsvPerColumn(filename, &file, true, createMean, lengthHintX, lengthHintY);
		else
			readCsvPerLine(filename, &file, createMean, lengthHintX, lengthHintY);
		file.close();
//		Log::i(TAG, "...done");
	} else
		throw("could not read file " + filename);
}

void DataBasis::readCsvPerColumn(std::string name, std::istream* file, bool split, bool createMean, int lengthHintX, int lengthHintY) {
	// create data table
	std::vector<std::vector<double>*>* tmp = new std::vector<std::vector<double>*>;
	std::vector<double> sums;
	std::vector<std::string> headers;
	if(lengthHintX > 0) {
		tmp->reserve(lengthHintX);
		headers.reserve(lengthHintX);
		sums.reserve(lengthHintX);
	}

	// read file
	std::string line;
	bool first = true;
	int interval = 0;
	int maxInterval = MyConfig::Get()->getInt("interval");
	if(!createMean)
		maxInterval = 1;
	while(getline(*file, line)) {
		// parse line
		std::stringstream s(line);
		int column = 0;
		if(first) {
			std::string header;
			while(getline(s, header, ',')) {
				header = Helper::rtrim(header);
				headers.push_back(header);
				std::vector<double>* tmpColumn = new std::vector<double>;
				if(lengthHintY > 0)
					tmpColumn->reserve(lengthHintY / maxInterval);
				tmp->push_back(tmpColumn);
			}
			for(uint i = 0; i < tmp->size(); i++) {
				sums.push_back(0);
			}
			first = false;
		} else {
			double d;
			while(s >> d) {
				//tmp->at(column++)->push_back(d);
				sums[column++] += d;
				if(s.peek() == ',')
					s.ignore();
			}
			if(++interval >= maxInterval) {
				for(uint i = 0; i < tmp->size(); i++) {
					(*tmp)[i]->push_back(sums[i] / maxInterval);
					sums[i] = 0;
				}
				interval = 0;
			}
		}
	}
	if(interval > 0) {
		for(uint i = 0; i < tmp->size(); i++) {
			(*tmp)[i]->push_back(sums[i] / maxInterval);
			sums[i] = 0;
		}
	}


	// save
	if(split) {
		for(uint i = 0; i < headers.size(); i++) {
			std::vector<std::vector<double>*>* tmp2 = new std::vector<std::vector<double>*>;
			if(lengthHintY > 0)
				tmp2->reserve(lengthHintY / maxInterval);
			tmp2->push_back((*tmp)[i]);
			data.insert(std::pair<std::string, std::vector<std::vector<double>*>*>(name + "_" + headers[i], tmp2));
		}
		// delete tmp
		delete tmp;
	} else
		data.insert(std::pair<std::string, std::vector<std::vector<double>*>*>(name, tmp));
}

void DataBasis::readCsvPerLine(std::string name, std::istream* file, bool createMean, int lengthHintX, int lengthHintY) {
	// create data table
	std::vector<std::vector<double>*>* tmp = new std::vector<std::vector<double>*>;
	if(lengthHintY > 0)
		tmp->reserve(lengthHintY);

	// read file
	int maxInterval = MyConfig::Get()->getInt("interval");
	if(!createMean)
		maxInterval = 1;
	std::string line;
	while(getline(*file, line)) {
		// parse line
		std::stringstream s(line);
		std::vector<double>* tmpLine = new std::vector<double>;
		if(lengthHintX > 0)
			tmpLine->reserve(lengthHintX);
		int interval = 0;
		double sum = 0, d;
		while(s >> d) {
			sum += d;
			if(s.peek() == ',')
				s.ignore();
			if(++interval >= maxInterval) {
				tmpLine->push_back(sum / maxInterval);
				sum = 0;
				interval = 0;
			}
		}
		if(interval > 0) {
			tmpLine->push_back(sum / maxInterval);
			interval = 0;
		}
		tmp->push_back(tmpLine);
	}

	// save
	data.insert(std::pair<std::string, std::vector<std::vector<double>*>*>(name, tmp));
}

double DataBasis::parseDouble(std::string s) {
	std::stringstream stream(s);
	double d;
	if(!(stream >> d))
		return 0;
	return d;
}

void DataBasis::createBinaryCSV(std::string filename) {
	std::ifstream file(filename.c_str());
	std::ofstream ofs((filename + ".bin").c_str(), std::ios::binary);
	std::string line;
	while(getline(file, line)) {
		double d;
		std::stringstream s(line);
		while(s >> d) {
			ofs.write(reinterpret_cast<char*>(&d), sizeof(d));
			if(s.peek() == ',')
				s.ignore();
		}
	}
}

void DataBasis::preloadBinaryLineFile(std::string filename, std::string dataName, int x, int y) {
	Log::f(TAG, "preloading binary file " + filename);
	std::ifstream file(filename.c_str(), std::ios::binary);
	int interval = MyConfig::Get()->getInt("interval");
	std::vector<std::vector<double>*>* tmp = new std::vector<std::vector<double>*>;
	tmp->reserve(y);
	std::vector<double>* line = new std::vector<double>;
	line->reserve(x / interval);
	double sum = 0;
	double d;
	int i;
	for(i = 0; i < x * y; i++) {
		file.read(reinterpret_cast<char*>(&d), sizeof(d));
		sum += d;
		if(i > 0 && i % interval == 0) {
			line->push_back(sum / interval);
			sum = 0;
		}
		if(i > 0 && i % x == 0) {
			if(i % interval != 0) {
				line->push_back(sum / interval);
				sum = 0;
			}
			tmp->push_back(line);
			if(i < x * y - 1) {
				line = new std::vector<double>;
				line->reserve(x / interval);
			}
		}
	}
	if(i % interval == 0)
		line->push_back(sum / interval);
	tmp->push_back(line);
	data.insert(std::pair<std::string, std::vector<std::vector<double>*>*>(dataName, tmp));
}

void DataBasis::convertFilesToBinary() {
	createBinaryCSV("data/devices/baseload_SSams.csv");
	createBinaryCSV("data/devices/baseload_SSonn.csv");
	createBinaryCSV("data/devices/baseload_SWerk.csv");
	createBinaryCSV("data/devices/baseload_USams.csv");
	createBinaryCSV("data/devices/baseload_USonn.csv");
	createBinaryCSV("data/devices/baseload_UWerk.csv");
	createBinaryCSV("data/devices/baseload_WSams.csv");
	createBinaryCSV("data/devices/baseload_WSonn.csv");
	createBinaryCSV("data/devices/baseload_WWerk.csv");
	createBinaryCSV("data/devices/costs.csv");
	createBinaryCSV("data/devices/pv_all.csv");
	createBinaryCSV("data/devices/pv_single.csv");
	createBinaryCSV("data/dsm/h0.csv");
	throw("finished");
}

bool DataBasis::isFileAvailable(std::string filename) {
	std::ifstream file(filename.c_str());
	return file.good();
}

}
