#include "../../siena/network/BackgroundTraffic_Device.h"

#include "ns3/GridHome.h"

namespace ns3 {

std::vector<std::vector<double>*>* BackgroundTraffic_Device::frameSizeDistributionHelper = NULL;

BackgroundTraffic_Device::BackgroundTraffic_Device(std::string id, GridHome* home) : Device(id, DEV_MODE_CONTINUOUS, NULL), home(home) {
	load = config->getDouble("background_traffic");
	random = Random::Get();
	std::vector<std::vector<double>*>* frameData = getFrameSizeDistribution();
	frameSize = frameData->at(0);
	frameDistribution = frameData->at(1);
	DataRateValue v;
	home->GetNode()->GetDevice(1)->GetAttribute("DataRate", v);
	int bitrate = v.Get().GetBitRate(); // in bps
	maxData = (bitrate / 8) * 60 * interval; // in bytes
	maxMs = interval * 60 * 1000;
}

void BackgroundTraffic_Device::tick() {
	if(t->getTick() != lastTick) {
		lastTick = t->getTick();

		Log::i(id, "creating background traffic...");
		// create packets
		double dataCounter = 0;
		int packetCounter = 0;
		while(dataCounter < maxData * load - (*frameSize)[0]) {
			double r = random->getD();
			uint size;
			for(size = 0; size < frameDistribution->size(); size++) {
				if((*frameDistribution)[size] >= r)
					break;
			}
			int s = (*frameSize)[size];
			Ptr<Packet> packet = Create<Packet>(s);
			home->scheduleSend(MilliSeconds(random->get(0, maxMs)), server, packet);
			dataCounter += s;
			packetCounter++;
		}
		std::stringstream s;
		s << "\tpackets: " << packetCounter << ", data: " << dataCounter / 1000000 << " MB";
		Log::i(id, s.str());
	}

	// stats
	stats->addStat(consumptionStatId, getConsumption());
	stats->addStat(stateStatId, state);
}

std::vector<std::vector<double>*>* BackgroundTraffic_Device::getFrameSizeDistribution() {
	if(frameSizeDistributionHelper == NULL)
		frameSizeDistributionHelper = DataBasis::Get()->get("scenario/frame_size_distribution", "csv", CSV_PER_COLUMN, false, 1, 1455);
	return frameSizeDistributionHelper;
}

}
