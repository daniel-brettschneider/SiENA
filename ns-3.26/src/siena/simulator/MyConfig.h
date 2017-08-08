#ifndef MYCONFIG_H_
#define MYCONFIG_H_

#include <map>
#include <string>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include "jsoncpp/json/json.h"
#include "jsoncpp/json/value.h"
#include "ns3/MySingleton.h"
#include <cstdlib>
#include "ns3/Helper.h"

namespace ns3 {

class MyConfig : public MySingleton<MyConfig> {
	friend class MySingleton<MyConfig>;

private:
	Json::Value data;

public:
	virtual ~MyConfig() {}

	void init(int iargc, char* argv[]) {
		// init all .json files in ./data
		DIR *dir;
		struct dirent *ent;
		if((dir = opendir("data/config")) != NULL) {
			while ((ent = readdir (dir)) != NULL) {
				std::string filename(ent->d_name);
				if(filename.length() > 5 && filename.compare(filename.length() - 5, 5, ".json") == 0)
					init("data/config/" + filename);
			}
			closedir (dir);
		} else
			throw("could not access data folder");

		// parse command line
		int argc = iargc;
		for(argc--, argv++; argc > 0; argc--, argv++) {
			// check for -- or -
			std::string param = *argv;
			std::string::size_type cur = param.find("--");
			if(cur == 0)
				param = param.substr(2, param.size() - 2);
			else {
				cur = param.find("-");
				if(cur == 0)
					param = param.substr(1, param.size() - 1);
				else { // invalid argument. ignore.
					std::cerr << "strange config parameter" << std::endl;
				}
			}
			cur = param.find("=");
			std::string name, value;
			if(cur == std::string::npos) {
				name = param;
				value = "";
			} else {
				name = param.substr(0, cur);
				value = param.substr(cur + 1, param.size() - (cur+1));
			}
			// check whether param in config and edit
			Json::Value v = data[name];
			if(v != Json::Value::null) {
				if(v.isString())
					data[name] = value;
				else if(v.isInt())
					data[name] = atoi(value.c_str());
				else if(v.isDouble())
					data[name] = atof(value.c_str());
				else if(v.isBool())
					data[name] = value.compare("true") == 0 ? true : false;
			}
		}
	}

	void init(std::string name) {
		std::ifstream f(name.c_str());
		std::string file((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
		Json::Reader reader;
		Json::Value json;
		if(reader.parse(file, json, false)) {
			for(Json::ValueIterator it = json.begin(); it != json.end(); ++it) {
				data[it.name()] = json[it.name()];
			}
		} else
			throw("could not parse config file " + name);
	}

	std::string getString(std::string name) {
		Json::Value v = data[name];
		if(v == Json::Value::null)
			throw("config parameter " + name + " not available");
		if(v.isConvertibleTo(Json::stringValue))
			return v.asString();
		throw("could not convert config entry " + name + " to string");
	}

	int getInt(std::string name) {
		Json::Value v = data[name];
		if(v == Json::Value::null)
			throw("config parameter " + name + " not available");
		if(v.isConvertibleTo(Json::intValue))
			return v.asInt();
		throw("could not convert config entry " + name + " to int");
	}

	double getDouble(std::string name) {
		Json::Value v = data[name];
		if(v == Json::Value::null)
			throw("config parameter " + name + " not available");
		if(v.isConvertibleTo(Json::realValue))
			return v.asDouble();
		throw("could not convert config entry " + name + " to double");
	}

	bool getBool(std::string name) {
		Json::Value v = data[name];
		if(v == Json::Value::null)
			throw("config parameter " + name + " not available");
		if(v.isConvertibleTo(Json::booleanValue))
			return v.asBool();
		throw("could not convert config entry " + name + " to bool");
	}

	void set(std::string what, std::string s) {
		data[what] = s;
	}

	void set(std::string what, int i) {
		data[what] = i;
	}

	void set(std::string what, double d) {
		data[what] = d;
	}

	void set(std::string what, bool b) {
		data[what] = b;
	}

	void print() {
		for(Json::ValueIterator it = data.begin(); it != data.end(); ++it) {
			std::cout << it.name() << ": " << data[it.name()];
		}
	}


private:
	MyConfig() {};

};

}

#endif
