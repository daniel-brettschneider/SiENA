#include "../../siena/scenario/Scenario.h"

namespace ns3 {

void Scenario::init() {
	MyConfig* config = MyConfig::Get();
	if(config->getBool("use_scenario"))
		this->init(config->getString("scenario"));
	else
		this->initRandom();
}

void Scenario::initRandom() {
	MyConfig* c = MyConfig::Get();
	int clusters = c->getInt("clusters");
	int homes = c->getInt("homes");
	int smartHomesInCluster = homes * c->getDouble("smarthome_share");
	int gridHomesInCluster = homes * c->getDouble("gridhome_share");
	int conventionalHomesInCluster = homes - smartHomesInCluster - gridHomesInCluster;
	DataBasis* data = DataBasis::Get();
	std::vector<std::vector<double>*>* space = data->get("scenario/living_space", "csv", CSV_PER_COLUMN, false, 2, 10);
	std::vector<std::vector<double>*>* residents = data->get("scenario/residents_dep_living_space", "csv", CSV_PER_COLUMN, false, 7, 10);
	std::vector<std::vector<double>*>* efficiency = data->get("heat/building_efficiency", "csv", CSV_PER_COLUMN, false, 2, 16);
	std::vector<std::vector<double>*>* type = data->get("scenario/building_type", "csv", CSV_PER_COLUMN, false, 2, 1);
	std::vector<std::vector<double>*>* cars = data->get("scenario/number_of_cars", "csv", CSV_PER_COLUMN, false, 5, 5);
	std::map<int, HomeConfig*> ownRod, ownHeatpump, ownCHPS;

	c->set("conventional_homes", conventionalHomesInCluster * clusters);
	c->set("smart_homes", smartHomesInCluster * clusters);
	c->set("grid_homes", gridHomesInCluster * clusters);

	bool addBackgroundTraffic = c->getDouble("background_traffic") > 0;

	int count = 0;
	for(int i = 0; i < clusters; i++) {
		ClusterConfig* cluster = new ClusterConfig;
		for(int j = 0; j < homes; j++) {
			HomeConfig* home = new HomeConfig(random->get());
			home->setId(count++);
			// type
			if(j < conventionalHomesInCluster)
				home->setType(CONFIG_TYPE_CONVENTIONAL);
			else if(j < conventionalHomesInCluster + smartHomesInCluster)
				home->setType(CONFIG_TYPE_SMART);
			else
				home->setType(CONFIG_TYPE_GRID);
			// space and residents
			std::pair<double, int> spaceCat = getCategory(space);
			home->setSpace(spaceCat.first);
			// residents
			double r = random->getD();
			double sum = 0;
			for(uint i = 1; i < (*residents)[0]->size(); i++) {
				sum += (*(*residents)[i])[spaceCat.second];
				if(r <= sum) {
					home->setResidents(i);
					break;
				}
			}
			// unit
			r = random->getD();
			if(r <= (*(*type)[0])[0])
				home->setUnit(CONFIG_UNIT_SINGLE);
			else
				home->setUnit(CONFIG_UNIT_MULTI);
			// heat efficiency
			home->setHeatEfficiency(getCategory(efficiency).first);
			// devices
			initSimpleDevice("dishwasher", "", home, c);
			initSimpleDevice("drier", "", home, c);
			initSimpleDevice("pv", "", home, c);
			initSimpleDevice("battery", chooseRandomDeviceConfig("battery", random->get()), home, c);
			initSimpleDevice("washingmachine", "", home, c);
			//r = random->getD();
			std::vector<int> rInt = random->getIntList(3);
			if(random->getDev("heat")) {
				home->addDevice("heatingrod", "", rInt[0]);
				ownRod.insert(std::pair<int, HomeConfig*>(home->getId(), home));
				std::string conf = devConfig->getHeatpump(home->getHeatDemand());
				if(conf != "") {
					home->addDevice("heatpump", conf, rInt[1]);
					ownHeatpump.insert(std::pair<int, HomeConfig*>(home->getId(), home));
				}
				conf = devConfig->getCHPS(home->getHeatDemand());
				if(conf != "") {
					home->addDevice("chps", conf, rInt[2]);
					ownCHPS.insert(std::pair<int, HomeConfig*>(home->getId(), home));
				}
			}
			int numberOfCars = c->getInt("own_car_override");
			//std::vector<double> rDouble = random->getDoubleList(2);
			r = random->getD();
			bool ownCar = random->getDev("car");
			rInt = random->getIntList(4);
			if(numberOfCars == -1) {
				if(ownCar) {
					double r2 = r;
					sum = 0;
					for(uint i = 0; i < (*cars)[0]->size(); i++) {
						sum += (*(*cars)[home->getResidents() - 1])[i];
						if(r2 <= sum) {
							numberOfCars = i;
							break;
						}
					}
				} else
					numberOfCars = 0;
			} else if(!ownCar)
				numberOfCars = 0;
			for(int j = 1; j <= numberOfCars; j++) {
				home->addDevice("car" + (numberOfCars > 1 ? ":" + Helper::toString(j) : ""), chooseRandomDeviceConfig("car", rInt[j-1]), rInt[j-1]);
			}
			if(addBackgroundTraffic && home->getType() == CONFIG_TYPE_GRID)
				home->addDevice("traffic", "", random->get());
			cluster->addHome(home);
		}
		this->clusters.push_back(cluster);
	}
	c->set("total_homes", homes * clusters);

	// purge heat devices
	//double fHeat = c->getDouble("own_heat");
	double targetRod = /*count * fHeat **/ c->getDouble("own_heatingrod");
	double targetPump = /*count * fHeat **/ c->getDouble("own_heatpump");
	double targetCHPS = /*count * fHeat **/ c->getDouble("own_chps");
	if(ownCHPS.size() < ownHeatpump.size()) {
		targetCHPS = c->getDouble("own_chps") / 0.654;
		purgeHeatDevice(&ownCHPS, targetCHPS, "chps", "heatpump", &ownHeatpump, "heatingrod", &ownRod);
		targetPump = c->getDouble("own_heatpump") * ((double) count / (double) ownHeatpump.size());
		purgeHeatDevice(&ownHeatpump, targetPump, "heatpump", "chps", &ownCHPS, "heatingrod", &ownRod);
	} else {
		targetPump = c->getDouble("own_heatpump") * ((double) count / (double) ownHeatpump.size());
		purgeHeatDevice(&ownHeatpump, targetPump, "heatpump", "chps", &ownCHPS, "heatingrod", &ownRod);
		targetCHPS = c->getDouble("own_chps") / (0.654 * (1.0 - c->getDouble("own_heatpump")));
		purgeHeatDevice(&ownCHPS, targetCHPS, "chps", "heatpump", &ownHeatpump, "heatingrod", &ownRod);
	}
	purgeHeatDevice(&ownRod, targetRod, "heatingrod", "heatpump", &ownHeatpump, "chps", &ownCHPS);


}

void Scenario::initSimpleDevice(std::string device, std::string devConfig, HomeConfig* home, MyConfig* config) {
	//double r = random->getD();
	int r2 = random->get();
	double own = config->getDouble("own_" + device);
	int i;
	for(i = 1; i <= (int) own; i++) {
		home->addDevice(device + (own > 1 ? ":" + Helper::toString(i) : ""), devConfig, r2);
	}
	own -= (int) own;
	if(random->getDev(device))
		home->addDevice(device + (own > 1 ? ":" + Helper::toString(i) : ""), devConfig, r2);
}

void Scenario::init(std::string scenario) {
	Log::i("Scenario", "parsing scenario file...");

	std::string file = DataBasis::Get()->getFileContent("data/scenario/" + scenario + ".json");
	Json::Reader reader;
	Json::Value json;
	int homeCount = 0;
	int clusterCount = 0;

	if(reader.parse(file, json, false)) {
		// clusters
		if(json.isMember("clusters")) {
			Json::Value clusters = json["clusters"];
			for(uint i = 0; i < clusters.size(); i++) {
				Json::Value cluster = clusters[i];

				ClusterConfig* clusterConfig = new ClusterConfig;

				// homes
				if(cluster.isMember("homes")) {
					Json::Value homes = cluster["homes"];
					for(uint j = 0; j < homes.size(); j++) {
						clusterConfig->addHome(parseHome(homes[j]));
						homeCount++;
					}
				} else
					throw("homes missing in cluster");

				// composites
				if(cluster.isMember("composites")) {
					Json::Value composites = cluster["composites"];
					for(uint j = 0; j < composites.size(); j++) {
						CompositeConfig* compositeConfig = new CompositeConfig;
						Json::Value composite = composites[j];

						// homes
						if(composite.isMember("homes")) {
							Json::Value homes = composite["homes"];
							for(uint j = 0; j < homes.size(); j++) {
								compositeConfig->addHome(parseHome(homes[j]));
								homeCount++;
							}
						} else
							throw("homes missing in composite");

						// devices
						if(composite.isMember("devices")) {
							std::vector<std::string> devices = composite["devices"].getMemberNames();
							for(uint i = 0; i < devices.size(); i++) {
								compositeConfig->addDevice(devices[i], composite["devices"][devices[i]].asString(), random->get());
							}
						} else
							throw("devices missing in composite");
						clusterConfig->addComposite(compositeConfig);
					}
				} else
					throw("composites missing in cluster");
                this->clusters.push_back(clusterConfig);
				clusterCount++;
			}
		} else
			throw("clusters missing");
	} else
		throw("could not parse scenario file " + scenario);
	MyConfig::Get()->set("total_homes", homeCount);
	MyConfig::Get()->set("clusters", clusterCount);
}

HomeConfig* Scenario::parseHome(Json::Value json) {
	HomeConfig* home = new HomeConfig(random->get());
	// name
	if(json.isMember("name"))
		home->setName(json["name"].asString());
	else
		throw("name of home missing");
	// type
	if(json.isMember("type"))
		home->setType(json["type"].asString());
	else
		throw("type of home missing");
	// residents
	if(json.isMember("residents"))
		home->setResidents(json["residents"].asInt());
	else
		throw("residents of home missing");
	// unit
	if(json.isMember("unit"))
		home->setUnit(json["unit"].asString());
	else
		throw("unit of home missing");
	// space
	if(json.isMember("space"))
		home->setSpace(json["space"].asInt());
	else
		throw("space of home missing");
	// heat efficiency
	if(json.isMember("heat_efficiency"))
		home->setHeatEfficiency(json["heat_efficiency"].asDouble());
	else
		throw("heat_efficiency of home missing");
	// devices
	if(json.isMember("devices")) {
		std::vector<std::string> devices = json["devices"].getMemberNames();
		for(uint i = 0; i < devices.size(); i++) {
			std::string devType = devices[i];
			std::string devConfig = json["devices"][devices[i]].asString();
			if(devConfig == "random") {
				if(devType == "heatpump") {
					devConfig = DeviceConfigHelper::Get()->getHeatpump(home->getHeatDemand());
					if(devConfig != "")
						home->addDevice(devType, devConfig, random->get());
				} else if(devType == "chps") {
					devConfig = DeviceConfigHelper::Get()->getCHPS(home->getHeatDemand());
					if(devConfig != "")
						home->addDevice(devType, devConfig, random->get());
				} else if(devType == "pv") {
					home->addDevice(devType, "", random->get());
				} else {
					devConfig = chooseRandomDeviceConfig(devType, random->get());
					home->addDevice(devType, devConfig, random->get());
				}
			} else
				home->addDevice(devType, devConfig, random->get());
		}
	} else
		throw("devices of home missing");
	return home;
}

std::string Scenario::chooseRandomDeviceConfig(std::string type, int r) {
	Json::Value all = devConfig->get(type);
	int id = r % (int) all.size();
	return all[id]["name"].asString();
}

void Scenario::purgeHeatDevice(std::map<int, HomeConfig*>* source, double target, std::string name, std::string other1Name,
		std::map<int, HomeConfig*>* other1, std::string other2Name, std::map<int, HomeConfig*>* other2) {
	std::vector<int> index;
	std::map<int, HomeConfig*>::iterator it;
	for(it = source->begin(); it != source->end(); ++it) {
		index.push_back(it->first);
	}
	std::random_shuffle(index.begin(), index.end());
	/*for(uint i = 0; i < index.size(); i++) {
		int id = index.at(i);
		double rest = target - (int) target;
		if(i < (uint) target || (i == (uint) target && rest > 0 && random->getD() <= rest)) {
			if(other1->find(id) != other1->end()) {
				other1->at(id)->removeDevice(other1Name);
				other1->erase(id);
			}
			if(other2->find(id) != other2->end()) {
				other2->at(id)->removeDevice(other2Name);
				other2->erase(id);
			}
		} else {
			source->at(id)->removeDevice(name);
			source->erase(id);
		}
	}*/
	for(uint i = 0; i < index.size(); i++) {
		int id = index[i];
		if(random->getD() <= target) {
			if(other1->find(id) != other1->end()) {
				(*other1)[id]->removeDevice(other1Name);
				other1->erase(id);
			}
			if(other2->find(id) != other2->end()) {
				(*other2)[id]->removeDevice(other2Name);
				other2->erase(id);
			}
		} else {
			(*source)[id]->removeDevice(name);
			source->erase(id);
		}
	}
}

std::pair<double, int> Scenario::getCategory(std::vector<std::vector<double>*>* data) {
	double r = random->getD();
	double sum = 0;
	for(uint i = 1; i < (*data)[0]->size(); i++) {
		sum += (*(*data)[1])[i];
		if(r <= sum)
			return std::pair<double, int>(random->get((*(*data)[0])[i-1] + 1, (*(*data)[0])[i]), i - 1);
	}
	return std::pair<double, int>(-1, -1);
}

}
