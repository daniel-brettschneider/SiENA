#ifndef COHDA_OVERLAYNETWORK_H_
#define COHDA_OVERLAYNETWORK_H_

#include "ns3/Server.h"
#include "ns3/COHDA_Token.h"
#include "ns3/AttributeContainer.h"
#include "ns3/ipv4-address.h"
#include "ns3/packet.h"
#include "ns3/TokenTag.h"
#include "ns3/MyConfig.h"
#include <sstream>
#include "ns3/Log.h"
#include "ns3/HomeContainer.h"
#include "ns3/Adaption.h"
#include <algorithm>
#include "ns3/MyStats.h"
#include "ns3/Random.h"
#include "ns3/COHDA_Configuration.h"

namespace ns3 {

static void COHDA_OverlaynetworkRing(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	std::map<std::string, std::vector<std::string>*>* overlayNetwork = (std::map<std::string, std::vector<std::string>*>*) (*params)[1];
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses = server->getHomeAddresses();

	for(int i = 0; i < homeAddresses->size(); i++) {
		Ipv4Address adr = homeAddresses->at(i);
		overlayNetwork->insert(std::pair<std::string, std::vector<std::string>*>(Helper::toString(adr),new std::vector<std::string>));
	}
	uint number = MyConfig::Get()->getInt("number_recipients");
	if(number<2)
		number = 2;

	for(int i = 0; i < homeAddresses->size(); i++) {
		//current IP
		Ipv4Address adr = homeAddresses->at(i);
		std::string adrString = Helper::toString(adr);

		//random IP
		std::vector<int> adrR = Random::Get()->randomList(i+2,i+homeAddresses->size()-1);
		if(adrR.size() != 0) {		//next IP
			Ipv4Address adrN = homeAddresses->at((i+1) % homeAddresses->size());
			(*overlayNetwork)[adrString]->push_back(Helper::toString(adrN));
			(*overlayNetwork)[Helper::toString(adrN)]->push_back(adrString);
			for(uint j = 0; j < number-2; j++) {
				Ipv4Address adrN = homeAddresses->at(adrR[j] % homeAddresses->size());
				std::string adrNString = Helper::toString(adrN);
				if(!(std::find((*overlayNetwork)[adrString]->begin(), (*overlayNetwork)[adrString]->end(), adrNString) != (*overlayNetwork)[adrString]->end()))
					(*overlayNetwork)[adrString]->push_back(adrNString);
				if(!(std::find((*overlayNetwork)[adrNString]->begin(), (*overlayNetwork)[adrNString]->end(), adrString) != (*overlayNetwork)[adrNString]->end()))
					(*overlayNetwork)[adrNString]->push_back(adrString);
			}
		} else {	//Connect all if not a lot of households
			for(int j = 0; j < homeAddresses->size()-1; j++) {
				Ipv4Address adrN = homeAddresses->at((i+j+1) % homeAddresses->size());
				(*overlayNetwork)[adrString]->push_back(Helper::toString(adrN));
			}
		}
	}
}

static void COHDA_OverlaynetworkTree(std::vector<void*>* params) {
	Server* server = (Server*) params->at(0);
	std::map<std::string, std::vector<std::string>*>* overlayNetwork = (std::map<std::string, std::vector<std::string>*>*) params->at(1);
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses = server->getHomeAddresses();

	std::vector<std::string>* listToConnectNew = new std::vector<std::string>;
	for(int i = 0; i < homeAddresses->size(); i++) {
		Ipv4Address adr = homeAddresses->at(i);
		std::string adrString = Helper::toString(adr);
		listToConnectNew->push_back(adrString);
		overlayNetwork->insert(std::pair<std::string, std::vector<std::string>*>(adrString,new std::vector<std::string>));
	}

	std::vector<std::string>* listToConnectOld = new std::vector<std::string>;
	listToConnectOld->push_back(listToConnectNew->at(0));
	uint i = 1;
	uint number = MyConfig::Get()->getInt("number_recipients");
	if(number == 0)
		number = 1;
	while(i < listToConnectNew->size()) {
		std::string adrOldString = listToConnectOld->at(0);
		for(uint j = 0; j < number; j++) {
			std::string adrString = listToConnectNew->at(i);
			overlayNetwork->at(adrOldString)->push_back(adrString);
			overlayNetwork->at(adrString)->push_back(adrOldString);
//			std::cout << adrOldString << " with " << adrString << std::endl;
			listToConnectOld->push_back(adrString);
			i++;
			if(i >= listToConnectNew->size()){
				break;
			}
		}
		listToConnectOld->erase(listToConnectOld->begin());
	}

	delete listToConnectNew;
	delete listToConnectOld;
}

static void COHDA_OverlaynetworkRandom(std::vector<void*>* params) {
	Server* server = (Server*) params->at(0);
	std::map<std::string, std::vector<std::string>*>* overlayNetwork = (std::map<std::string, std::vector<std::string>*>*) params->at(1);
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses = server->getHomeAddresses();

	std::vector<std::string>* listToConnectNew = new std::vector<std::string>;
	std::vector<std::string>* listToConnectOld = new std::vector<std::string>;
	for(int i = 0; i < homeAddresses->size(); i++) {
		Ipv4Address adr = homeAddresses->at(i);
		std::string adrString = Helper::toString(adr);
		listToConnectNew->push_back(adrString);
		overlayNetwork->insert(std::pair<std::string, std::vector<std::string>*>(adrString,new std::vector<std::string>));
	}
	int number = MyConfig::Get()->getInt("number_recipients");
	if(number == 0)
		number = 1;

	for(int i = 0; i < number; i++) {
		listToConnectOld->push_back(listToConnectNew->at(i));
		for(int j = i + 1; j < number; j++) {
			overlayNetwork->at(listToConnectNew->at(i))->push_back(listToConnectNew->at(j));
			overlayNetwork->at(listToConnectNew->at(j))->push_back(listToConnectNew->at(i));
//			std::cout << listToConnectNew->at(j) << " with1 " << listToConnectNew->at(i) << std::endl;
		}
	}

	for(uint i = number; i < listToConnectNew->size(); i++) {
		Random::Get()->getIntList(2);
		int j = 0;
		while(j < number) {
			int random = Random::Get()->get(0,listToConnectOld->size());
			if(!(std::find(overlayNetwork->at(listToConnectOld->at(random))->begin(), overlayNetwork->at(listToConnectOld->at(random))->end(), listToConnectNew->at(i)) != overlayNetwork->at(listToConnectOld->at(random))->end())) {
				overlayNetwork->at(listToConnectOld->at(random))->push_back(listToConnectNew->at(i));
				overlayNetwork->at(listToConnectNew->at(i))->push_back(listToConnectOld->at(random));
//				std::cout << listToConnectNew->at(i) << " with " << listToConnectOld->at(random) << std::endl;
				j++;
			}
		}
		listToConnectOld->push_back(listToConnectNew->at(i));
	}
	delete listToConnectNew;
	delete listToConnectOld;
}


static void COHDA_OverlaynetworkClientServer(std::vector<void*>* params) {
	Server* server = (Server*) params->at(0);
	std::map<std::string, std::vector<std::string>*>* overlayNetwork = (std::map<std::string, std::vector<std::string>*>*) params->at(1);
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses = server->getHomeAddresses();

	std::vector<std::string>* listToConnectNew = new std::vector<std::string>;
	std::vector<std::string>* listToConnectOld = new std::vector<std::string>;
	for(int i = 0; i < homeAddresses->size(); i++) {
		Ipv4Address adr = homeAddresses->at(i);
		std::string adrString = Helper::toString(adr);
		listToConnectNew->push_back(adrString);
		overlayNetwork->insert(std::pair<std::string, std::vector<std::string>*>(adrString,new std::vector<std::string>));
	}

	listToConnectOld->push_back(listToConnectNew->at(0));
	for(uint i = 1; i < listToConnectNew->size(); i++) {
		overlayNetwork->at(listToConnectOld->at(0))->push_back(listToConnectNew->at(i));
		overlayNetwork->at(listToConnectNew->at(i))->push_back(listToConnectOld->at(0));
//		std::cout << listToConnectNew->at(i) << " with " << listToConnectOld->at(0) << std::endl;
	}
	delete listToConnectNew;
	delete listToConnectOld;
}

static void COHDA_OverlaynetworkSmallWorld(std::vector<void*>* params) {
	//std::cerr << "building overlay network" << std::endl;
	Server* server = (Server*) (*params)[0];
	std::map<std::string, std::vector<std::string>*>* overlayNetwork = (std::map<std::string, std::vector<std::string>*>*) (*params)[1];
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses = server->getHomeAddresses();
	int n = homeAddresses->size();
	double phi = MyConfig::Get()->getDouble("phi");
	std::vector<std::string> addresses;

	for(int i = 0; i < n; i++) {
		Ipv4Address adr = homeAddresses->at(i);
		overlayNetwork->insert(std::pair<std::string, std::vector<std::string>*>(Helper::toString(adr),new std::vector<std::string>));
		addresses.push_back(Helper::toString(adr));
	}

	if(n==2) {
		Ipv4Address adr = homeAddresses->at(0);
		Ipv4Address adrN = homeAddresses->at(1);
		(*overlayNetwork)[Helper::toString(adr)]->push_back(Helper::toString(adrN));
		(*overlayNetwork)[Helper::toString(adrN)]->push_back(Helper::toString(adr));
	} else if(n > 2){
		for(int i = 0; i < n; i++) {
			Ipv4Address adr = homeAddresses->at(i);
			std::string adrString = Helper::toString(adr);
			Ipv4Address adrN = homeAddresses->at((i+1) % homeAddresses->size());
			(*overlayNetwork)[adrString]->push_back(Helper::toString(adrN));
			(*overlayNetwork)[Helper::toString(adrN)]->push_back(adrString); //Ring
//			std::cout << adrString << " with " << Helper::toString(adrN) << std::endl;
		}
	}

	if(addresses.size()>3) {
		std::vector<std::string> addressesNew = addresses;
		int i = 0;
		while(i < n*phi && addressesNew.size()>1) {
//			std::cout << "i = " << i << " < " << n*phi << std::endl;
			int random1 = (int) (Random::Get()->getD()*(addressesNew.size()));
			std::vector<std::string> addressesTemp = addresses;
			std::string adrString1 = addressesNew[random1];
			addressesTemp.erase(std::find(addressesTemp.begin(), addressesTemp.end(), adrString1));
			for(uint j=0; j < (*overlayNetwork)[adrString1]->size(); j++) {
				addressesTemp.erase(std::find(addressesTemp.begin(), addressesTemp.end(), (*overlayNetwork)[adrString1]->at(j)));
//				std::cout << adrString1 << " is already conected to " << overlayNetwork->at(adrString1)->at(j) << std::endl;
			}
			int random2 = (int) (Random::Get()->getD()*(addressesTemp.size()));
//			std::cout << "random1: " << random1 << " random2: " << random2 << std::endl;
//			std::cout << "addressesNew.size(): " << addressesNew.size() << std::endl;
//			std::cout << addressesTemp.size()<< std::endl;
			if(addressesTemp.size() != 0) {
				std::string adrString2 = addressesTemp.at(random2);
				(*overlayNetwork)[adrString1]->push_back(adrString2);
				(*overlayNetwork)[adrString2]->push_back(adrString1);
//				std::cout << adrString1 << " with " << adrString2 << std::endl;
				i++;
			}
//			std::cout << "i: " << i << std::endl;
			addressesNew.erase(std::find(addressesNew.begin(), addressesNew.end(), adrString1));
//			addresses.erase(std::find(addressesTemp.begin(), addressesTemp.end(), adrString2));
		}
	}
//		for(uint j = 0; j < overlayNetwork->at(adrString)->size(); j++) {
//			std::vector<std::string>::iterator tmp = std::find(addressesTemp.begin(), addressesTemp.end(), overlayNetwork->at(adrString)->at(j));
//			if (tmp != addressesTemp.end()) {
//				addressesTemp.erase(tmp);
//			}
//		}
//		if(random < phi) {
//			random = (random/phi)*addressesTemp.size(); //is new rand between 0 and addressesTemp.size() (without addressesTemp.size())
//			overlayNetwork->at(adrString)->push_back(addressesTemp.at((int)random));
//			overlayNetwork->at(addressesTemp.at((int)random))->push_back(adrString);
//			std::cout << adrString << " with " << addressesTemp.at((int)random) << std::endl;
//		}
}

static void COHDA_OverlaynetworkSmallWorld_WS(std::vector<void*>* params) {
	Server* server = (Server*) (*params)[0];
	std::map<std::string, std::vector<std::string>*>* overlayNetwork = (std::map<std::string, std::vector<std::string>*>*) (*params)[1];
	Ptr<AttributeContainer<Ipv4Address> > homeAddresses = server->getHomeAddresses();
	int n = homeAddresses->size();
	double phi = MyConfig::Get()->getDouble("phi");
	int K = MyConfig::Get()->getInt("K");
	if(K % 2 != 0 && K < n)
		throw("fix K!!");
	if(K == n-1) {
		std::cout << "setting phi to 0.0!!" << std::endl;
		phi = 0.0;
	}
	std::vector<std::string> addresses;
	Random* random = Random::Get();

	// get addresses of households
	for(int i = 0; i < n; i++) {
		Ipv4Address adr = homeAddresses->at(i);
		overlayNetwork->insert(std::pair<std::string, std::vector<std::string>*>(Helper::toString(adr),new std::vector<std::string>));
		addresses.push_back(Helper::toString(adr));
	}

	// create ring
	if(n==2) {
			Ipv4Address adr = homeAddresses->at(0);
			Ipv4Address adrN = homeAddresses->at(1);
			(*overlayNetwork)[Helper::toString(adr)]->push_back(Helper::toString(adrN));
			(*overlayNetwork)[Helper::toString(adrN)]->push_back(Helper::toString(adr));
	} else if(n > 2){
		for(int i = 0; i < n; i++) { // ring
			for(int j = 0; j < K/2; j++) { // K
				Ipv4Address adr = homeAddresses->at(i);
				std::string node = Helper::toString(adr);
				Ipv4Address next = homeAddresses->at((i+1+j) % homeAddresses->size());
				std::vector<std::string>* nodeNexts = overlayNetwork->at(node);
				// probability stuff
				if(random->getD() > phi && std::find(nodeNexts->begin(), nodeNexts->end(), Helper::toString(next)) == nodeNexts->end()) {
					(*overlayNetwork)[node]->push_back(Helper::toString(next));
					(*overlayNetwork)[Helper::toString(next)]->push_back(node);
					//std::cout << i << " with " << (i+1+j)%n << std::endl;
				} else {
					std::vector<int> nexts = random->randomList(0, n-1);
					uint x = 0;
					while(x<nexts.size()) {
						int nextNew = nexts[x++];
						if(nextNew == i)
							continue;
						Ipv4Address adrNextNew = homeAddresses->at(nextNew);
						std::string nodeNextNew = Helper::toString(adrNextNew);
						if(std::find(nodeNexts->begin(), nodeNexts->end(), nodeNextNew) == nodeNexts->end()) {
							(*overlayNetwork)[node]->push_back(nodeNextNew);
							(*overlayNetwork)[nodeNextNew]->push_back(node);
							//std::cout << i << " with " << nextNew << " (phi) " << std::endl;
							break;
						}
					}
				}
			}
		}
	}
}

}

#endif
