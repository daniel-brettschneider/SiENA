#include "DeviceHelper.h"
#include "ns3/ConventionalHome.h"
#include "ns3/Composite.h"

namespace ns3 {

const std::string DeviceHelper::TAG = "DeviceCreator";
std::string DeviceHelper::dayTypes[9] = {"SSams", "SSonn", "SWerk", "USams", "USonn", "UWerk", "WSams", "WSonn", "WWerk"};

DeviceHelper::DeviceHelper() {
	data = DataBasis::Get();
	random = Random::Get();
	config = MyConfig::Get();
	stats = MyStats::Get();
	// preload files
	data->get("scenario/pdf", "csv", CSV_PER_COLUMN_SPLIT, false, 117, 1440);
	//data->get("devices/pv", "csv", CSV_PER_LINE);
	if(config->getBool("single_pv-system"))
		data->preloadBinaryLineFile("data/devices/pv_single.csv.bin", "devices/pv", 2599200, 1); // numbers musst be less or equal with number of columns/rows
	else
		data->preloadBinaryLineFile("data/devices/pv_all.csv.bin", "devices/pv", 2599200, 27);
	data->preloadBinaryLineFile("data/dsm/h0.csv.bin", "dsm/h0", 2607841, 1);
	data->preloadBinaryLineFile("data/dsm/cost.csv.bin", "devices/cost", 2607841, 1);
	data->preloadBinaryLineFile("data/devices/baseload_SSams.csv.bin", "devices/baseload_SSams", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_SSonn.csv.bin", "devices/baseload_SSonn", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_SWerk.csv.bin", "devices/baseload_SWerk", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_USams.csv.bin", "devices/baseload_USams", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_USonn.csv.bin", "devices/baseload_USonn", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_UWerk.csv.bin", "devices/baseload_UWerk", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_WSams.csv.bin", "devices/baseload_WSams", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_WSonn.csv.bin", "devices/baseload_WSonn", 1440, 1000);
	data->preloadBinaryLineFile("data/devices/baseload_WWerk.csv.bin", "devices/baseload_WWerk", 1440, 1000);
	data->get("scenario/number_of_departures_per_car", "csv", CSV_PER_COLUMN, false, 3, 1);
	data->get("scenario/dis_departure_duration_distance", "csv", CSV_PER_COLUMN, false, 9, 1440);
	data->get("scenario/pv_size_distribution", "csv", CSV_PER_COLUMN, false, 1, 151);
	// sum up activations
//	int start = config->getInt("start");
	int duration = config->getInt("duration");
	double totalActivations[] = {0, 0, 0};
	Tick* t = Tick::Get();
	std::vector<std::vector<double>*>* activations = data->get("scenario/device_activations", "csv", CSV_PER_COLUMN, false, 3, 9);
	for(int i = 0; i < duration; i += 1440) {
		for(uint j = 0; j < activations->size(); j++) {
			totalActivations[j] += (*(*activations)[j])[t->getTimePeriod(i)];
		}
	}
	config->set("act_dishwasher", totalActivations[0]); // TODO get names somehow from file
	config->set("act_washingmachine", totalActivations[1]);
	config->set("act_drier", totalActivations[2]);
}

void DeviceHelper::initHome(ConventionalHome* home, HomeConfig* homeConfig) {
	random->init(homeConfig->getSeed());

	std::string id = home->getId();

	// baseload
	int index = random->get() % data->get("devices/baseload_SSams")->size();
	Device* dev = new BaseLoad(id, index);
	dev->setState(DEV_ON);
	home->addDevice(dev);
	Log::i(TAG, "\tbaseload #", index);

	std::map<std::string, DeviceConfig>* deviceConfigs = homeConfig->getDevices();
	std::map<std::string, DeviceConfig>::iterator it;
	WaterTank* tank = NULL;
	for(it = deviceConfigs->begin(); it != deviceConfigs->end(); ++it) {
		std::string dev = Helper::split(it->first, ':')[0];
		if(dev == CONFIG_DEV_WASH)
			createMovableDevice(it->second.getSeed(), id, "washingmachine", it->first, 0, home);
		else if(dev == CONFIG_DEV_DISH)
			createMovableDevice(it->second.getSeed(), id, "dishwasher", it->first, 0, home);
		else if(dev == CONFIG_DEV_DRIER)
			createMovableDevice(it->second.getSeed(), id, "drier", it->first, 12, home);
		else if(dev == CONFIG_DEV_CAR)
			createCar(it->second.getSeed(), id + "_" + it->first, it->second.getType(), home);
		else if(dev == CONFIG_DEV_PV)
			createPV(it->second.getSeed(), id + "_" + it->first, it->second.getType(), home);
		else if(dev == CONFIG_DEV_HEATPUMP || dev == CONFIG_DEV_HEATINGROD || dev == CONFIG_DEV_CHPS)
			tank = createHeatDevice(it->second.getSeed(), dev, id + "_" + it->first, it->second.getType(), home, tank, 1, homeConfig->getHeatDemand());
		else if(dev == CONFIG_DEV_BATTERY)
			createBattery(it->second.getSeed(), id + "_" + it->first, it->second.getType(), home);
		else if(dev == CONFIG_DEV_TRAFFIC) {
			home->addDevice(new BackgroundTraffic_Device(id + "_" + it->first, (GridHome*) home));
		} else
			throw("devicehelper cannot create device for home (" + dev + ")");
	}
}

void DeviceHelper::initComposite(Composite* composite, CompositeConfig* config) {
	std::string id = composite->getId();
	std::map<std::string, DeviceConfig>* deviceConfigs = config->getDevices();
	std::map<std::string, DeviceConfig>::iterator it;
	WaterTank* tank = NULL;
	double demand = 0;
	std::vector<HomeConfig*>* homes = config->getHomes();
	for(uint i = 0; i < homes->size(); i++) {
		demand = demand + (*homes)[i]->getHeatDemand();
	}
	for(it = deviceConfigs->begin(); it != deviceConfigs->end(); ++it) {
		std::string dev = Helper::split(it->first, ':')[0];
		if(dev == CONFIG_DEV_HEATPUMP || dev == CONFIG_DEV_HEATINGROD || dev == CONFIG_DEV_CHPS)
			tank = createHeatDevice(it->second.getSeed(), dev, id + "_" + it->first, it->second.getType(), composite, tank, config->getHomes()->size(), demand);
		else
			throw("devicehelper cannot create device " + dev + " for composite");
	}
	// set heat source of homes
	if(tank != NULL) {
		composite->addDevice(tank);
		std::vector<ConventionalHome*>* homes = composite->getHomes();
		for(uint i = 0; i < homes->size(); i++) {
			(*homes)[i]->setHeatSource(tank);
		}
	}
	// delete heat demand of composite
	composite->removeHeatSource();
}

void DeviceHelper::createMovableDevice(int seed, std::string id, std::string type, std::string devId, double standby, ConventionalHome* home) {
	random->init(seed);

	// create device
	std::vector<std::vector<double>*>* loadcurves = data->get("devices/" + type, "txt", CSV_PER_LINE);
	Device* device = new ShiftableDevice(id + "_" + devId, (*loadcurves)[random->get() % loadcurves->size()], standby);
	device->setCategory(type);
	home->addDevice(device);

	// log
	std::stringstream s;
	s << "\t" << type << ": ";

	// activations
	int simStart = config->getInt("start");
	int duration = config->getInt("duration");
	double activations = config->getDouble("act_" + type);
	// 1.9 -> 90% 2, 10% 1
	if(random->getD() <= (activations - (int) activations)) // check random <= decimal places
		activations = ceil(activations); // round upwards
	else
		activations = floor(activations); // round downwards

	// activation probability integral
	std::vector<double> api(duration, 0);
	this->createIntegral(type, &api, simStart, duration, activations, true);

	// compute activation times
	int start = 0;
	for(int counter = 0; counter < activations; counter++) {
		// get activation tick
		int tick = this->integralGoTo(&api, start, duration, counter + random->getD());
		if(tick != -1) {
			// deadline probability integral
			std::vector<double> dpi(1440, 0); // TODO max
			this->createIntegral(type + "_end", &dpi, simStart + tick, 1440, 1, false);
			// get deadline tick
			int deadline = tick + this->integralGoTo(&dpi, 0, 1440, random->getD());
			// create event
			DeviceEvent* event = new DeviceEvent(simStart + tick, device, EV_SCHEDULE, simStart + deadline);
			home->addEvent(event);
			s << tick << "[" << deadline << "] ";
			start = tick;
		}
	}

	int runCounterId = stats->registerCounter(STAT_RUN, device->getId());
	stats->updateCounter(runCounterId, activations);
	Log::i(TAG, s.str());
}

void DeviceHelper::createCar(int seed, std::string id, std::string conf, ConventionalHome* home) {
	random->init(seed);

	// create device
	Car* car = new Car(id);
	if(conf != "")
		car->init(DeviceConfigHelper::Get()->get("car", conf));
	home->addDevice(car);

	// init departures
	std::vector<CarEvent*> carEvents;
	int simStart = config->getInt("start");
	int simDuration = config->getInt("duration");
	Tick* t = Tick::Get();
	for(int day = simStart; day < simStart + simDuration; day += 1440) {
		int dayType = t->getDayType(day);
		if(dayType == 0 || dayType == 6) {
			initCarDeparture(&carEvents, day, 2);
		} else {
			initCarDeparture(&carEvents, day, 0);
			initCarDeparture(&carEvents, day, 1);
		}
	}

	// create events
	for(uint i = 0; i < carEvents.size(); i++) {
		CarEvent* e = carEvents[i];
		home->addEvent(new DeviceEvent(e->departure, car, EV_GO_AWAY, 0));
		home->addEvent(new DeviceEvent(e->arrival, car, EV_COMING_BACK, e->distance));
		delete e;
	}
}

void DeviceHelper::initCarDeparture(std::vector<CarEvent*>* events, uint tick, int dataIndex) {
	std::vector<std::vector<double>*>* n = data->get("scenario/number_of_departures_per_car");
	std::vector<std::vector<double>*>* carData = data->get("scenario/dis_departure_duration_distance");

	// number of activations
	double activations = (*(*n)[dataIndex])[0];
	if(random->getD() <= (activations - (int) activations))
		activations = ceil(activations);
	else
		activations = floor(activations);

	// departure
	double r = random->getD();
	std::vector<double>* pdfDep = (*carData)[dataIndex * 3];
	double sum = 0;
	for(int i = 0; i < 1440; i++) {
		sum += (*pdfDep)[i] * activations;
		if(r <= sum) {
			int departure = tick + i;
			// duration
			int duration = 120;
			double r2 = random->getD();
			std::vector<double>* pdfDur = (*carData)[dataIndex * 3 + 1];
			double sum2 = 0;
			for(int j = 0; j < 1440; j++) {
				sum2 += (*pdfDur)[j];
				if(r2 <= sum2) {
					duration = j + 1;
					break;
				}
			}
			// distance
			int distance = 10;
			r2 = random->getD();
			std::vector<double>* pdfDist = (*carData)[dataIndex * 3 + 2];
			sum2 = 0;
			for(int j = 0; j < 1440; j++) {
				sum2 += (*pdfDist)[j];
				if(r2 <= sum2) {
					distance = j + 1;
					break;
				}
			}
			distance *= 2; // distance in both directions
			// create new event
			events->push_back(new CarEvent(departure, duration, distance));
			// check for overlappings
			int id = events->size() - 1;
			int compareId = events->size() - 2;
			while(events->size() > 1) {
				bool changed = false;
				CarEvent* current = (*events)[id];
				CarEvent* compare = (*events)[compareId];
				// front
				if(current->departure < compare->departure && current->arrival >= compare->departure && current->arrival <= compare->arrival) {
					compare->departure = current->departure;
					compare->duration = compare->arrival - compare->departure;
					compare->distance += current->distance;
					changed = true;
				}
				// inside
				else if(current->departure >= compare->departure && current->departure < compare->arrival && current->arrival <= compare->arrival) {
					compare->distance += current->distance;
					changed = true;
				}
				// end
				else if(current->departure <= compare->arrival && current->departure >= compare->departure && current->arrival > compare->arrival) {
					compare->arrival = current->arrival;
					compare->duration = compare->arrival - compare->departure;
					compare->distance += current->distance;
					changed = true;
				}
				// over
				else if(current->departure < compare->departure && current->arrival > compare->arrival) {
					compare->departure = current->departure;
					compare->arrival = current->arrival;
					compare->duration = compare->arrival - compare->departure;
					compare->distance += current->distance;
					changed = true;
				}
				if(changed) {
					delete current;
					events->erase(events->begin() + id);
					id = compareId;
					compareId = events->size() - 1;
					if(compareId == id)
						compareId--;
				} else if((uint) compare->departure > tick - 2880 && compareId > 1) {
					compareId--;
					if(compareId == id)
						compareId--;
				} else
					break;
			}
			// reset for next activation
			r = ceil(r) + random->getD();
			if(r > activations)
				break;
		}
	}
}

void DeviceHelper::createPV(int seed, std::string id, std::string config, ConventionalHome* home) {
	random->init(seed);

	PV* pv = new PV(id);
	home->addDevice(pv);

	// size
	double size = -1;
	if(config != "")
		size = atof(config.c_str());
	else {
		std::vector<double>* pdf = (*data->get("scenario/pv_size_distribution"))[0];
		double sum = 0;
		double r = random->getD();
		for(uint i = 0; i < pdf->size(); i++) {
			sum += (*pdf)[i];
			if(r <= sum) {
				size = (i + 1) * 100;
				break;
			}
		}
	}
	pv->setSize(size);

	Log::i(TAG, "\tpv, size: ", size);
}

WaterTank* DeviceHelper::createHeatDevice(int seed, std::string dev, std::string id, std::string config, ConventionalHome* home, WaterTank* tank, int consumers, double demand) {
	random->init(seed);
	if(tank == NULL) {
		int size = calcTankSize(consumers, demand);
		tank = new WaterTank(id + "_tank", size);
		home->addDevice(tank);
		home->setHeatSource(tank);
		Log::i(TAG, "\ttank, capacity: ", size);
	}
	if(dev == CONFIG_DEV_HEATPUMP) {
		HeatPump* heatpump = new HeatPump(id, tank);
		if(config != "")
			heatpump->init(DeviceConfigHelper::Get()->get("heatpump", config));
		home->addDevice(heatpump);
		Log::i(TAG, "\theatpump");
		HeatingRod* rod = new HeatingRod(id + "_heatingrod", -1, 10000 + random->get(0, 50), 1, tank);
		home->addDevice(rod);
		Log::i(TAG, "\theatingrod");
	} else if(dev == CONFIG_DEV_CHPS) {
		CHPS* chps = new CHPS(id, tank);
		if(config != "")
			chps->init(DeviceConfigHelper::Get()->get("chps", config));
		home->addDevice(chps);
		Log::i(TAG, "\tchps");
		HeatingRod* rod = new HeatingRod(id + "_heatingrod", -1, 10000 + random->get(0, 50), 1, tank);
		home->addDevice(rod);
		Log::i(TAG, "\theatingrod");
	} else if(dev == CONFIG_DEV_HEATINGROD) {
		HeatingRod* rod = new HeatingRod(id, tank);
		home->addDevice(rod);
		Log::i(TAG, "\theatingrod");
	}
	return tank;
}

void DeviceHelper::createBattery(int seed, std::string id, std::string config, ConventionalHome* home) {
	random->init(seed);
	Battery* bat = new Battery(id);
	if(config != "")
		bat->init(DeviceConfigHelper::Get()->get("battery", config));
	home->addDevice(bat);
}

void DeviceHelper::createNormalDistributionIntegral(double* integral, double m, double s) {
	double sum = 0;
	for(int i = 0; i < s; i++) {
		sum += (1 / (s * sqrt(2*M_PI))) * pow(M_E, -0.5 * pow(((double)i-m)/s, 2));
		integral[i] = sum;
	}
	for(int i = 0; i < s; i++) {
		integral[i] /= sum;
	}
}

// TODO opt
void DeviceHelper::createIntegral(std::string type, std::vector<double>* integral, int start, int duration, double activations, bool correct) {
	std::vector<double>* tmp = NULL;
	int period = -1;
	Tick* t = Tick::Get();
	double correction = 1;
	if(correct) // standard integral -> 1 activation per day
		correction = activations / ((double) config->getInt("duration") / 1440);
	double sum = 0;
	for(int i = start; i < start + duration; i++) {
		if(i == start || i % 1440 == 0) {
			period = t->getTimePeriod(i % 1811); // TODO 1811
			tmp = (*data->get("scenario/pdf_" + dayTypes[period] + "_" + type))[0];
		}
		sum += (*tmp)[i % tmp->size()];
		(*integral)[i - start] = sum * correction;
	}
}

// TODO opt
void DeviceHelper::createIntegral(std::vector<double>* integral, std::vector<double>* source, double times) {
	double sum = 0;
	for(uint i = 0; i < source->size(); i++) {
		sum += (*source)[i];
		(*integral)[i] = sum * times;
	}
}

int DeviceHelper::integralGoTo(std::vector<double>* data, int start, int end, double limit) {
	// go through array until limit value is reached
	for(int x = start; x < end; x++) {
		if((*data)[x] >= limit)
			return x;
	}
	return -1;
}

int DeviceHelper::calcTankSize(int consumers, double demand) {
	//std::cou << "Consumers: " << consumers << "; demand: " << demand << "; tankSize: " << (0.20 + (0.8 / std::sqrt(consumers))) * demand * 1.161 * 35 / 15 << std::endl;
	return (0.20 + (0.8 / std::sqrt(consumers))) * demand * 1.161 * 35 / 15;
	// consumers * (simultaneity factor) * demand / correction factor * Waterheat * deltatemperature
}

}
