#ifndef REGISTRY_H_
#define REGISTRY_H_

#include "ns3/Modules.h"
#include "ns3/COHDA_GridHome.h"
#include "ns3/COHDA_Server.h"
#include "ns3/COHDA_Token.h"
#include "ns3/COHDA_Configuration.h"
#include "ns3/COHDA_SolutionCandidate.h"
#include "ns3/COHDA_WorkingMemory.h"
#include "ns3/COHDA_ScheduleSelection.h"
#include "ns3/COHDA_AdaptOnOff.h"
#include "ns3/COHDA_AdaptFlex.h"
#include "ns3/COHDA_Helper.h"
#include "ns3/COHDA_OverlayNetwork.h"
#include "ns3/NetworkTree_Module.h"
#include "ns3/Network_BRITE.h"
#include "ns3/Network_LTE.h"
#include "ns3/Network_PLC.h"
#include "ns3/Do_Not_Shift.h"
#include "ns3/Shift_H0.h"
#include "ns3/Priv_Shift.h"
#include "ns3/PPPM_Concentrator.h"
#include "ns3/PPPM_GridHome.h"
#include "ns3/PPPM_Auctioneer.h"
#include "ns3/SmartHomeAlgos.h"

namespace ns3 {

static void module_void(std::vector<void*>* params) {}

static void registerModules() {
	Modules* m = Modules::Get();

	// static
	m->registerFunction("empty", &module_void);

	// smarthome
	m->registerFunction("smarthome_shift_device", &smarthome_shift);
	m->registerFunction("smarthome_battery", &smarthome_battery);
	m->registerFunction("smarthome_car", &smarthome_car);
	m->registerFunction("smarthome_chps", &smarthome_chps);
	m->registerFunction("smarthome_heatpump", &smarthome_heatpump);

	//COHDA
	m->registerFunction("COHDA_GridHomeHandleToken", &COHDA_GridHomeHandleToken);
	m->registerFunction("COHDA_ServerStartInterval", &COHDA_ServerStartInterval);
	m->registerFunction("COHDA_Shift", &COHDA_Shift);
	m->registerFunction("COHDA_scheduler_init", &COHDA_scheduler_init);
	m->registerFunction("COHDA_tidyUp", &COHDA_tidyUp);
	m->registerFunction("COHDA_OverlaynetworkRing", &COHDA_OverlaynetworkRing);
	m->registerFunction("COHDA_OverlaynetworkTree", &COHDA_OverlaynetworkTree);
	m->registerFunction("COHDA_OverlaynetworkRandom", &COHDA_OverlaynetworkRandom);
	m->registerFunction("COHDA_OverlaynetworkClientServer", &COHDA_OverlaynetworkClientServer);
	m->registerFunction("COHDA_OverlaynetworkSmallWorld", &COHDA_OverlaynetworkSmallWorld);
	m->registerFunction("COHDA_OverlaynetworkSmallWorld_WS", &COHDA_OverlaynetworkSmallWorld_WS);

	// powermatcher
	m->registerFunction("powermatcher_SchedulerInit", &powermatcher_SchedulerInit);
	m->registerFunction("powermatcher_AuctioneerHandleToken", &powermatcher_AuctioneerHandleToken);
	m->registerFunction("powermatcher_ConcentratorHandleToken", &powermatcher_ConcentratorHandleToken);
	m->registerFunction("powermatcher_GridHomeHandleToken", &powermatcher_GridHomeHandleToken);
	m->registerFunction("powermatcher_GridHomeTick", &powermatcher_GridHomeTick);
	m->registerFunction("powermatcher_tidyUp", &powermatcher_tidyUp);
	// pppm
	m->registerFunction("pppm_ConcentratorHandleToken", &pppm_ConcentratorHandleToken);
	m->registerFunction("pppm_GridHomeHandleToken", &pppm_GridHomeHandleToken);
	m->registerFunction("pppm_GridHomeTick", &pppm_GridHomeTick);
	m->registerFunction("pppm_AuctioneerHandleToken", &pppm_AuctioneerHandleToken);

	// PrivADE
	m->registerFunction("priv_GridHomeHandleToken", &priv_GridHomeHandleToken);
	m->registerFunction("priv_ServerStartInterval", &priv_ServerStartInterval);
	m->registerFunction("priv_ServerHandleToken", &priv_ServerHandleToken);
	m->registerFunction("priv_ClusterServerHandleToken", &priv_ClusterServerHandleToken);
	m->registerFunction("priv_tidyUp", &priv_tidyUp);
	m->registerFunction("priv_shift_expected_factor", &priv_GridHomeShiftFactor);
	m->registerFunction("priv_shift_expected_quadratic", &priv_GridHomeShiftQuadratic);

	// PrivADE+
	m->registerFunction("privadePlus_GridHomeTick", &privadePlus_GridHomeTick);
	m->registerFunction("privadePlus_GridHomeHandleToken", &privadePlus_GridHomeHandleToken);
	m->registerFunction("privadePlus_ServerHandleToken", &privadePlus_ServerHandleToken);
	m->registerFunction("privadePlus_AggregatorHandleToken", &privadePlus_AggregatorHandleToken);
	m->registerFunction("privadePlus_tidyUp", &privadePlus_tidyUp);
	m->registerFunction("privadePlus_shift_expected_factor", &privadePlus_GridHomeShiftFactor);
	m->registerFunction("privadePlus_shift_expected_quadratic", &privadePlus_GridHomeShiftQuadratic);
	m->registerFunction("privadePlus_SchedulerInit", &privadePlus_SchedulerInit);

	// network
	m->registerFunction("tree", &networkTree_create);
	m->registerFunction("brite", &BRITE_create);
	m->registerFunction("lte", &LTE_create);
	m->registerFunction("plc", &PLC_create);
	
	// misc
	m->registerFunction("do_not_shift", &do_not_shift);
	m->registerFunction("shift_H0", &shift_H0);

}

}

#endif
