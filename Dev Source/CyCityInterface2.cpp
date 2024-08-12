#include "CvGameCoreDLL.h"
#include "CyCity.h"
#include "CyPlot.h"
#include "CyArea.h"
#include "CvInfos.h"

//# include <boost/python/manage_new_object.hpp>
//# include <boost/python/return_value_policy.hpp>

//
// published python interface for CyCity
//

void CyCityPythonInterface2(python::class_<CyCity>& x)
{
	OutputDebugString("Python Extension Module - CyCityPythonInterface2\n");

	x

	//FfH: Added by Kael 10/18/2007
		.def("applyBuildEffects", &CyCity::applyBuildEffects, "void (CyUnit* pUnit)")
		.def("changeCrime", &CyCity::changeCrime, "void (int iChange) - changes the Crime Rate for this city")
		.def("getCrime", &CyCity::getCrime, "int () - crime rate")
		.def("isHasBuildingClass", &CyCity::isHasBuildingClass, "bool (int /*BuildingClassTypes*/ iIndex) - has building class")
		.def("isSettlement", &CyCity::isSettlement, "bool () - is settlement")
		.def("setCivilizationType", &CyCity::setCivilizationType, "void (int iNewValue) - sets the Civilization Type of this city")
		.def("setPlotRadius", &CyCity::setPlotRadius, "void (int iNewValue) - sets the Plot Radius of this city")
		.def("setSettlement", &CyCity::setSettlement, "void (bool bNewValue) - sets city as a Settlement or not")
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (New Function Definitions) Sephi                                                      **/
/*************************************************************************************************/
        .def("swapAustrinCities", &CyCity::swapAustrinCities, "void (CyPlot* pNewPlot)")
        .def("getPopulationLimit", &CyCity::getPopulationLimit, "int ()")
		.def("getGreatPeopleThreshold", &CyCity::getGreatPeopleThreshold, "int ()")
		.def("getNextGreatPersonInfo", &CyCity::getNextGreatPersonInfo, "int (bool)")
		.def("getCommercePercentTypeForced", &CyCity::getCommercePercentTypeForced, "int ()")
		.def("changeBuildingCommerceChange", &CyCity::changeBuildingCommerceChange, "void (int, int, int)")
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
        .def("AI_neededPermDefense", &CyCity::AI_neededPermDefense, "int (int flag)")
		.def("AI_stopGrowth", &CyCity::AI_stopGrowth, "bool ()")
		.def("AI_neededSeaWorkers", &CyCity::AI_neededSeaWorkers, "int ()")
/**	ADDON (Better Traderoutes) Sephi                                                          	**/
		.def("calculateTradeProfitTimes100", &CyCity::calculateTradeProfitTimes100, "int (CyCity) - returns the trade profit created by CyCity")
/**	Change Building Bonuses  Snarko                                   					        **/
		.def("getBuildingBonusChange", &CyCity::getBuildingBonusChange, "int (int /*BuildingClassTypes*/ eBuildingClass, int /*BonusTypes*/ eBonus)")
		.def("setBuildingBonusChange", &CyCity::setBuildingBonusChange, "void (int /*BuildingClassTypes*/ eBuildingClass, int /*BonusTypes*/ eBonus, int iChange)")
/*************************************************************************************************/
/**	END Change Building Bonuses	                                        						**/
/*************************************************************************************************/
//>>>>BUGFfH: Added by Denev 2009/09/29
		.def("getUnitArtStyleButton", &CyCity::getUnitArtStyleButton, "string (int iUnit)")
		.def("getReligionCount", &CyCity::getReligionCount, "int ()")
//<<<<BUGFfH: End Add
		;
}
