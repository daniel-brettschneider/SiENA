#include "../../siena/home/Composite.h"

namespace ns3 {

TypeId Composite::GetTypeId() {
	static TypeId id = TypeId("ns3::Composite")
		.SetParent<Application>()
		.AddConstructor<Composite>()
		.AddAttribute("id", "id", StringValue("composite"), MakeStringAccessor(&Composite::id), MakeStringChecker());
	return id;
}

void Composite::init(CompositeConfig* config) {
	Log::f(id, "initializing...");

	// devices
	DeviceHelper::Get()->initComposite(this, config);

	// schedule
	this->scheduleNextTick(0);
}

}
