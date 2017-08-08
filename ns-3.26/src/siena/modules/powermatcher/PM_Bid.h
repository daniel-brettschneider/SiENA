#ifndef NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_BID_H_
#define NS_3_22_SRC_SIENA_MODULES_POWERMATCHER_PM_BID_H_

#include "ns3/Token.h"
#include <vector>
#include <string>
#include <sstream>

namespace ns3 {

class PM_Bid : public Token {

protected:
	std::vector<std::pair<double, double> >* coords;
	int tick;

public:
	PM_Bid(std::vector<std::pair<double, double> >* coords) : Token(), coords(coords) { tick = Tick::Get()->getTick(); }
	PM_Bid(double consumption) : Token() {
		 tick = Tick::Get()->getTick();
		 addCoordinate(0, consumption);
	}
	PM_Bid() : Token() {
		coords = new std::vector<std::pair<double, double> >;
		tick = Tick::Get()->getTick();
	}
	PM_Bid(std::vector<std::string>* participants) : Token(participants) {
		coords = new std::vector<std::pair<double, double> >;
		tick = Tick::Get()->getTick();
	}
	virtual ~PM_Bid() { delete coords; }
	int getTick() { return tick; }

	virtual int getSize() { return coords->size() * 8 + 4; }
	std::vector<std::pair<double, double> >* getCoords() { return coords; }

	virtual double getConsumption(double price, bool second = false) {	//if second == true, the second value of a price will be chosen
//		std::cout << "price " << price << " and bid " << this->toString();
		if(coords->empty()) {//size == 0
//			std::cout << " 1leads to consumption of 0" << std::endl;
			return 0;
		}
		for(uint i = 0; i < coords->size(); i++) {
			std::pair<double, double> current = (*coords)[i];
			if (current.first < price) {

			} else if(current.first == price) {
				if(second == true && coords->size() > i + 1 && (*coords)[i + 1].first == current.first) {
//					std::cout << " 2leads to consumption of " << coords.at(i + 1).second << std::endl;
					return (*coords)[i + 1].second;
				}
//				std::cout << " 3leads to consumption of " << current.second << std::endl;
				return current.second;
			} else if(current.first > price) {
				if(i >= 1) {
					std::pair<double, double> pre = (*coords)[i - 1];
					if(pre.second == current.second) {
//						std::cout << " 4leads to consumption of " << pre.second << std::endl;
						return pre.second;
					}
					double tmp = pre.second + ((pre.second - current.second) / (pre.first - current.first)) * (price - pre.first);
//					std::cout << " 5leads to consumption of " << tmp << std::endl;
					return tmp;
				} else {//return the first due to the horizontal line to the left
//					std::cout << " 6leads to consumption of " << current.second << std::endl;
					return current.second;
				}
			}
		}
//		std::cout << " 7leads to consumption of " << coords.at(coords.size() - 1).second << std::endl;
		return (*coords)[coords->size() - 1].second;
	}

	virtual void addBid(PM_Bid* bid) {
		std::vector<std::pair<double, double> >* coords2 = bid->getCoords();
		std::list<double> pricesSorted;
		uint c = 0, c2 = 0;
		double coord, coord2;
		while(c<coords->size()||c2<coords2->size()) {
			if(c<coords->size())
				coord = (*coords)[c].first;
			else
				coord = (*coords2)[c2].first;
			if(c2<coords2->size())
				coord2 = (*coords2)[c2].first;
			else
				coord2 = (*coords)[c].first;
			if(coord < coord2) {
				pricesSorted.push_back(coord);
				c++;
			} else if (coord > coord2) {
				pricesSorted.push_back(coord2);
				c2++;
			} else {
				pricesSorted.push_back(coord);
				c2++;
				c++;
			}
		}
		pricesSorted.unique();
		std::pair<double, double> p;
		std::vector<std::pair<double, double> >* coordsTmp = new std::vector<std::pair<double, double> >;
		for(std::list<double>::iterator it = pricesSorted.begin(); it != pricesSorted.end(); ++it) {
			p.first = *it;
			p.second = this->getConsumption(p.first) + bid->getConsumption(p.first);
			coordsTmp->push_back(p);
			double con1 = this->getConsumption(p.first, true);
			double con2 = bid->getConsumption(p.first, true);
			if(p.second != con1 + con2) {
				p.second = con1 + con2;
				coordsTmp->push_back(p);
			}
		}
//		coords->clear();
		delete coords;
		coords = coordsTmp;
//		cleanup();
//		selfcheck();
	}

	virtual void addCoordinate(double price, double consumption) {
		coords->push_back(std::pair<double, double>(price, consumption));
	}

	virtual double findEquilibrium(double goal, double* minPrice, double* maxPrice) {//assumed a monotone falling consumption
		if(coords->size() < (uint) 2)
			return 0;	//no adaptable devices (price could be an other)
		for(uint i = 0; i < coords->size(); i++) {
			std::pair<double, double> current = (*coords)[i];
			if(i == 0 && current.second <= goal)
				return current.first;
			else if(current.second < goal) {
				std::pair<double, double> pre = (*coords)[i - 1];
				return (goal - pre.second) * (current.first - pre.first) / (current.second - pre.second) + pre.first;
				//return (pre.second * (current.first - pre.first)) / (pre.second - current.second) + pre.first; //without goal
			}
		}
		//return *maxPrice;
		return coords->at(coords->size() - 1).first;	//if not found return the most expensable price
	}

	virtual std::string toString() {
		std::stringstream s;
		#ifndef	TURBO
			s << "#" << coords->size();
			if(coords->size() > 0) {
				s << ":";
				for(uint i = 0; i < coords->size(); i++) {
					s << " (" << coords->at(i).first << "/" << coords->at(i).second << ")";
				}
			}
		#endif
		return s.str();
	}

	void selfcheck() {
		if(coords->size() == 0)
			return;
		double tmp = (*coords)[0].first;
		double tmp2 = (*coords)[0].second;
		for(uint i = 1; i < coords->size(); i++) {
			if((*coords)[i].first >= tmp)
				tmp = (*coords)[i].first;
			else
				throw("coords not consistent (time)");
			if((*coords)[i].second <= tmp2)
				tmp2 = (*coords)[i].second;
			else
				throw("coords not consistent (consumption)");
		}
	}

	void cleanup() {
		if(coords->size() >= 3) {
			for(uint i = 2; i < coords->size(); i++) {
				if((*coords)[i].second == (*coords)[i-1].second && (*coords)[i].second == (*coords)[i-2].second) {
					coords->erase(coords->begin() + i - 1);
				}
			}
		}
		if(coords->size() >= 2) {
			std::vector<std::pair<double, double> >::iterator it = coords->begin()+1;
			std::vector<std::pair<double, double> >::iterator itp = coords->begin();
			if((*coords)[0].second == (*coords)[1].second) {
				coords->erase(coords->begin());
			}
		}
		if(coords->size() >= 2) {
			if((*coords)[coords->size()-1].second == (*coords)[coords->size()-2].second) {
				coords->erase(coords->end());
			}
		}
	}

protected:


};

}

#endif
