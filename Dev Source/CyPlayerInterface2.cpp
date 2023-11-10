#include "CvGameCoreDLL.h"
#include "CyPlayer.h"
#include "CyUnit.h"
#include "CyCity.h"
#include "CyPlot.h"
#include "CySelectionGroup.h"
#include "CyArea.h"
//# include <boost/python/manage_new_object.hpp>
//# include <boost/python/return_value_policy.hpp>
//# include <boost/python/scope.hpp>

//
// published python interface for CyPlayer
//

void CyPlayerPythonInterface2(python::class_<CyPlayer>& x)
{
	OutputDebugString("Python Extension Module - CyPlayerPythonInterface2\n");

	// set the docstring of the current module scope
	python::scope().attr("__doc__") = "Civilization IV Player Class";
	x
		.def("AI_updateFoundValues", &CyPlayer::AI_updateFoundValues, "void (bool bStartingLoc)")
		.def("AI_foundValue", &CyPlayer::AI_foundValue, "int (int, int, int, bool)")
		.def("AI_isFinancialTrouble", &CyPlayer::AI_isFinancialTrouble, "bool ()")
		.def("AI_demandRebukedWar", &CyPlayer::AI_demandRebukedWar, "bool (int /*PlayerTypes*/)")
		.def("AI_getAttitude", &CyPlayer::AI_getAttitude, "AttitudeTypes (int /*PlayerTypes*/) - Gets the attitude of the player towards the player passed in")
		.def("AI_unitValue", &CyPlayer::AI_unitValue, "int (int /*UnitTypes*/ eUnit, int /*UnitAITypes*/ eUnitAI, CyArea* pArea)")
		.def("AI_civicValue", &CyPlayer::AI_civicValue, "int (int /*CivicTypes*/ eCivic)")
		.def("AI_totalUnitAIs", &CyPlayer::AI_totalUnitAIs, "int (int /*UnitAITypes*/ eUnitAI)")
		.def("AI_totalAreaUnitAIs", &CyPlayer::AI_totalAreaUnitAIs, "int (CyArea* pArea, int /*UnitAITypes*/ eUnitAI)")
		.def("AI_totalWaterAreaUnitAIs", &CyPlayer::AI_totalWaterAreaUnitAIs, "int (CyArea* pArea, int /*UnitAITypes*/ eUnitAI)")
		.def("AI_getNumAIUnits", &CyPlayer::AI_getNumAIUnits, "int (UnitAIType) - Returns # of UnitAITypes the player current has of UnitAIType")
		.def("AI_getAttitudeExtra", &CyPlayer::AI_getAttitudeExtra, "int (int /*PlayerTypes*/ eIndex) - Returns the extra attitude for this player - usually scenario specific")
		.def("AI_setAttitudeExtra", &CyPlayer::AI_setAttitudeExtra, "void (int /*PlayerTypes*/ eIndex, int iNewValue) - Sets the extra attitude for this player - usually scenario specific")
		.def("AI_changeAttitudeExtra", &CyPlayer::AI_changeAttitudeExtra, "void (int /*PlayerTypes*/ eIndex, int iChange) - Changes the extra attitude for this player - usually scenario specific")
		.def("AI_getMemoryCount", &CyPlayer::AI_getMemoryCount, "int (/*PlayerTypes*/ eIndex1, /*MemoryTypes*/ eIndex2)")
		.def("AI_changeMemoryCount", &CyPlayer::AI_changeMemoryCount, "void (/*PlayerTypes*/ eIndex1, /*MemoryTypes*/ eIndex2, int iChange)")
		.def("AI_getExtraGoldTarget", &CyPlayer::AI_getExtraGoldTarget, "int ()")
		.def("AI_setExtraGoldTarget", &CyPlayer::AI_setExtraGoldTarget, "void (int)")

		.def("getScoreHistory", &CyPlayer::getScoreHistory, "int (int iTurn)")
		.def("getEconomyHistory", &CyPlayer::getEconomyHistory, "int (int iTurn)")
		.def("getIndustryHistory", &CyPlayer::getIndustryHistory, "int (int iTurn)")
		.def("getAgricultureHistory", &CyPlayer::getAgricultureHistory, "int (int iTurn)")
		.def("getPowerHistory", &CyPlayer::getPowerHistory, "int (int iTurn)")
		.def("getCultureHistory", &CyPlayer::getCultureHistory, "int (int iTurn)")
		.def("getEspionageHistory", &CyPlayer::getEspionageHistory, "int (int iTurn)")

		.def("getScriptData", &CyPlayer::getScriptData, "str () - Get stored custom data (via pickle)")
		.def("setScriptData", &CyPlayer::setScriptData, "void (str) - Set stored custom data (via pickle)")

		.def("chooseTech", &CyPlayer::chooseTech, "void (int iDiscover, wstring szText, bool bFront)")

		.def("AI_maxGoldTrade", &CyPlayer::AI_maxGoldTrade, "int (int)")
		.def("AI_maxGoldPerTurnTrade", &CyPlayer::AI_maxGoldPerTurnTrade, "int (int)")

		.def("splitEmpire", &CyPlayer::splitEmpire, "bool (int iAreaId)")
		.def("canSplitEmpire", &CyPlayer::canSplitEmpire, "bool ()")
		.def("canSplitArea", &CyPlayer::canSplitArea, "bool (int)")
		.def("canHaveTradeRoutesWith", &CyPlayer::canHaveTradeRoutesWith, "bool (int)")
		.def("forcePeace", &CyPlayer::forcePeace, "void (int)")

//FfH Alignment: Added by Kael 08/09/2007
		.def("canSeeCivic", &CyPlayer::canSeeCivic, "void (int iCivic)")
		.def("canSeeReligion", &CyPlayer::canSeeReligion, "void (int iReligion)")
		.def("changeSanctuaryTimer", &CyPlayer::changeSanctuaryTimer, "void (int iChange)")
		.def("getAlignment", &CyPlayer::getAlignment, "int ()")
        .def("setAlignment", &CyPlayer::setAlignment, "AlignmentTypes (iAlignment)")
		.def("changeDisableProduction", &CyPlayer::changeDisableProduction, "void (int iChange)")
		.def("getDisableProduction", &CyPlayer::getDisableProduction, "int ()")
		.def("changeDisableResearch", &CyPlayer::changeDisableResearch, "void (int iChange)")
		.def("getDisableResearch", &CyPlayer::getDisableResearch, "int ()")
		.def("changeDisableSpellcasting", &CyPlayer::changeDisableSpellcasting, "void (int iChange)")
		.def("getDisableSpellcasting", &CyPlayer::getDisableSpellcasting, "int ()")
		.def("getMaxCities", &CyPlayer::getMaxCities, "int ()")
		.def("changeNoDiplomacyWithEnemies", &CyPlayer::changeNoDiplomacyWithEnemies, "void (int iChange)")
		.def("getNumBuilding", &CyPlayer::getNumBuilding, "int (int iBuilding)")
		.def("getNumSettlements", &CyPlayer::getNumSettlements, "int ()")
		.def("getPlayersKilled", &CyPlayer::getPlayersKilled, "int ()")
		.def("isGamblingRing", &CyPlayer::isGamblingRing, "bool ()")
		.def("isHasTech", &CyPlayer::isHasTech, "bool (int iTech)")
		.def("isSlaveTrade", &CyPlayer::isSlaveTrade, "bool ()")
		.def("isSmugglingRing", &CyPlayer::isSmugglingRing, "bool ()")
        .def("setAlive", &CyPlayer::setAlive, "void (bool bNewValue)")
        .def("setFoundedFirstCity", &CyPlayer::setFoundedFirstCity, "void (bool bNewValue)")
		.def("setGreatPeopleCreated", &CyPlayer::setGreatPeopleCreated, "void (int iNewValue)")
		.def("setGreatPeopleThresholdModifier", &CyPlayer::setGreatPeopleThresholdModifier, "void (int iNewValue)")
        .def("setHasTrait", &CyPlayer::setHasTrait, "TraitTypes (eTrait), bool (bNewValue)")
//FfH: End Add
/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
        .def("isConquestMode", &CyPlayer::isConquestMode,  "bool ()")
        .def("startConquestMode", &CyPlayer::startConquestMode, "void ()")
        .def("getFavoriteReligion",&CyPlayer::getFavoriteReligion, "int ()")
        .def("setFavoriteReligion",&CyPlayer::setFavoriteReligion, "void (ReligionTypes (newvalue))")
        .def("countGroupFlagUnits",&CyPlayer::countGroupFlagUnits, "int (int)")
        .def("getConquestUnitClassCount",&CyPlayer::getConquestUnitClassCount, "int (int Unitclass, CvArea* pArea)")
        .def("canSupportMoreUnits",&CyPlayer::canSupportMoreUnits, "bool ()")		
        .def("AI_isWillingToTalk", &CyPlayer::AI_isWillingToTalk, "bool (int)")
        .def("isIgnoreFood", &CyPlayer::isIgnoreFood, "bool ()")
        .def("getPopulationLimit", &CyPlayer::getPopulationLimit, "int ()")
        .def("getCityStateType",&CyPlayer::getCityStateType, "int ()")
        .def("getPurityCounter",&CyPlayer::getPurityCounter, "int ()")
        .def("setPurityCounter",&CyPlayer::setPurityCounter, "void (int)")
        .def("getPurityCounterCache1",&CyPlayer::getPurityCounterCache1, "int ()")
        .def("setPurityCounterCache1",&CyPlayer::setPurityCounterCache1, "void (int)")
		.def("AI_getEconomyType",&CyPlayer::AI_getEconomyType, "int ()")
        .def("CalculateCivCounter",&CyPlayer::CalculateCivCounter, "int (int,int)")
        .def("setTerraformPlan",&CyPlayer::setTerraformPlan, "void (int)")
/** Allow Python to modify the CivilizationType of a Player **/
        .def("setCivilizationType",&CyPlayer::setCivilizationType, "void (int)")
        .def("canIncreaseCityInfrastructureLevel",&CyPlayer::canIncreaseCityInfrastructureLevel, "bool (CyCity* pCity)")
        .def("isAgnostic",&CyPlayer::isAgnostic, "bool ()")
        .def("getPrestige",&CyPlayer::getPrestige, "int ()")
        .def("changePrestige",&CyPlayer::changePrestige, "void (int)")
        .def("setCurrentMagicRitual",&CyPlayer::setCurrentMagicRitual, "void (int)")
/** Mana **/
        .def("getMana",&CyPlayer::getMana, "int ()")
        .def("changeMana",&CyPlayer::changeMana, "void (int)")
        .def("getFaith",&CyPlayer::getFaith, "int ()")
        .def("changeFaith",&CyPlayer::changeFaith, "void (int)")
        .def("getArcane",&CyPlayer::getArcane, "int ()")
        .def("changeArcane",&CyPlayer::changeArcane, "void (int)")
        .def("getArcaneNeeded",&CyPlayer::getArcaneNeeded, "int (int)")

        .def("calculateArcaneByBuildings",&CyPlayer::calculateArcaneByBuildings, "int ()")
        .def("calculateArcaneByManaSchool",&CyPlayer::calculateArcaneByManaSchool, "int (int)")
        .def("setSpellResearchFocus",&CyPlayer::setSpellResearchFocus, "void (int)")
        .def("getSpellResearchFocus",&CyPlayer::getSpellResearchFocus, "int ()")
        .def("canSpellResearchNext",&CyPlayer::canSpellResearchNext, "int (int)")

        .def("changeGlobalYield",&CyPlayer::changeGlobalYield, "void (int,int)")
        .def("getGlobalYield",&CyPlayer::getGlobalYield, "int (int)")
        .def("canDoGlobalEnchantment",&CyPlayer::canDoGlobalEnchantment, "bool (int)")
        .def("canDoTerraformRitual",&CyPlayer::canDoTerraformRitual, "bool (int)")
        .def("canCreateSummon",&CyPlayer::canCreateSummon, "bool (int)")
        .def("getMagicRitualTurnsNeeded",&CyPlayer::getMagicRitualTurnsNeeded, "int (int,bool)")
        .def("getCurrentMagicRitual",&CyPlayer::getCurrentMagicRitual, "int ()")
        .def("getSpecificMagicRitualCost",&CyPlayer::getSpecificMagicRitualCost, "int ()")
        .def("getCurrentMagicRitualManaNeeded",&CyPlayer::getCurrentMagicRitualManaNeeded, "int ()")
        .def("getManaIncome",&CyPlayer::getManaIncome, "int ()")
        .def("getArcaneIncome",&CyPlayer::getArcaneIncome, "int ()")
        .def("getArcaneIncomeTimes100", &CyPlayer::getArcaneIncomeTimes100, "int ()")
        .def("changeGlobalCulture",&CyPlayer::changeGlobalCulture, "void (int)")
        .def("getGlobalCulture",&CyPlayer::getGlobalCulture, "int ()")
        .def("setManaBonus2",&CyPlayer::setManaBonus2, "void (int)")
        .def("getManaBonus2",&CyPlayer::getManaBonus2, "int ()")
        .def("setManaBonus3",&CyPlayer::setManaBonus3, "void (int)")
        .def("getManaBonus3",&CyPlayer::getManaBonus3, "int ()")
        .def("setManaBonus4",&CyPlayer::setManaBonus4, "void (int)")
        .def("getManaBonus4",&CyPlayer::getManaBonus4, "int ()")
        .def("canGetCulturalAchievement",&CyPlayer::canGetCulturalAchievement, "bool (int)")
        .def("canGetTrait",&CyPlayer::canGetTrait, "bool (int)")
        .def("getTraitCost",&CyPlayer::getTraitCost, "int (int)")
        .def("getUnitSupportLimitTotal",&CyPlayer::getUnitSupportLimitTotal, "int (int)")
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Houses of Erebus) Sephi			                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    .def("getCorporationSupport",&CyPlayer::getCorporationSupport, "int (int)")
    .def("changeCorporationSupport",&CyPlayer::changeCorporationSupport, "void (int, int)")
    .def("getCorporationSupportMultiplier",&CyPlayer::getCorporationSupportMultiplier, "int (int)")
    .def("changeCorporationSupportMultiplier",&CyPlayer::changeCorporationSupportMultiplier, "void (int,int)")
    .def("isHasCorporationSupport",&CyPlayer::isHasCorporationSupport, "bool (int)")
    .def("setHasCorporationSupport",&CyPlayer::setHasCorporationSupport, "void (int, bool)")
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/*************************************************************************************************/
	.def("isAdventureEnabled",&CyPlayer::isAdventureEnabled, "bool (int)")
	.def("isAdventureFinished",&CyPlayer::isAdventureFinished,"bool (int)")
    .def("getAdventureStatus",&CyPlayer::getAdventureStatus,"int (int, int)")
    .def("getAdventureLimit",&CyPlayer::getAdventureLimit,"int (int, int)")

	.def("setAdventureEnabled",&CyPlayer::setAdventureEnabled,"void (int, bool)")
	.def("setAdventureFinished",&CyPlayer::setAdventureFinished,"void (int, bool)")
    .def("changeAdventureCounter",&CyPlayer::changeAdventureCounter,"void (int,int, int)")

    .def("VerifyEnabledAdventures",&CyPlayer::VerifyEnabledAdventures,"void ()")
    .def("VerifyFinishedAdventure",&CyPlayer::VerifyFinishedAdventure,"void (int)")
    .def("VerifyFinishedAdventures",&CyPlayer::VerifyFinishedAdventures,"void ()")

	.def("launchPickEquipmentPopup",&CyPlayer::launchPickEquipmentPopup,"void ()")
	.def("canEquipUnit",&CyPlayer::canEquipUnit,"bool (CyUnit* pUnit, int, bool, bool)")
	.def("isCanEverSpellResearch",&CyPlayer::isCanEverSpellResearch,"bool (int)")
/*************************************************************************************************/
/**	    									END													**/
/*************************************************************************************************/
		;
}
