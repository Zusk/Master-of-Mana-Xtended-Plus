#pragma once

// cityAI.h

#ifndef CIV4_CITY_AI_H
#define CIV4_CITY_AI_H

#include "CvCity.h"
#include "CvSelectionGroup.h"
typedef std::vector<std::pair<UnitAITypes, int> > UnitTypeWeightArray;

class CvCityAI : public CvCity
{

public:

	CvCityAI();
	virtual ~CvCityAI();

	void AI_init();
	void AI_uninit();
	void AI_reset();

	void AI_doTurn();

	void AI_assignWorkingPlots();
	void AI_updateAssignWork();

	bool AI_avoidGrowth();
	bool AI_ignoreGrowth();
	int AI_specialistValue(SpecialistTypes eSpecialist, bool bAvoidGrowth, bool bRemove);

	void AI_chooseProduction();

	UnitTypes AI_bestUnit(bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR, UnitAITypes* peBestUnitAI = NULL);
	UnitTypes AI_bestUnitAI(UnitAITypes eUnitAI, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR);

	BuildingTypes AI_bestBuilding(int iFocusFlags = 0, int iMaxTurns = 0, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR);
	BuildingTypes AI_bestBuildingThreshold(int iFocusFlags = 0, int iMaxTurns = 0, int iMinThreshold = 0, bool bAsync = false, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR);

	int AI_buildingValue(BuildingTypes eBuilding, int iFocusFlags = 0);
	int AI_buildingValueThreshold(BuildingTypes eBuilding, int iFocusFlags = 0, int iThreshold = 0);

	ProjectTypes AI_bestProject();
	int AI_projectValue(ProjectTypes eProject);

	ProcessTypes AI_bestProcess();
	ProcessTypes AI_bestProcess(CommerceTypes eCommerceType);
	int AI_processValue(ProcessTypes eProcess);
	int AI_processValue(ProcessTypes eProcess, CommerceTypes eCommerceType);

	int AI_neededSeaWorkers();

	bool AI_isDefended(int iExtra = 0);
	bool AI_isAirDefended(int iExtra = 0);
	bool AI_isDanger();

	int AI_neededDefenders();
	int AI_neededAirDefenders();
	int AI_minDefenders();
	int AI_neededFloatingDefenders();
	void AI_updateNeededFloatingDefenders();

	int AI_getEmphasizeAvoidGrowthCount();
	bool AI_isEmphasizeAvoidGrowth();

	int AI_getEmphasizeGreatPeopleCount();
	bool AI_isEmphasizeGreatPeople();

	bool AI_isAssignWorkDirty();
	void AI_setAssignWorkDirty(bool bNewValue);

	bool AI_isChooseProductionDirty();
	void AI_setChooseProductionDirty(bool bNewValue);

	CvCity* AI_getRouteToCity() const;
	void AI_updateRouteToCity();

	int AI_getEmphasizeYieldCount(YieldTypes eIndex);
	bool AI_isEmphasizeYield(YieldTypes eIndex);

	int AI_getEmphasizeCommerceCount(CommerceTypes eIndex);
	bool AI_isEmphasizeCommerce(CommerceTypes eIndex);

	bool AI_isEmphasize(EmphasizeTypes eIndex);
	void AI_setEmphasize(EmphasizeTypes eIndex, bool bNewValue);
	void AI_forceEmphasizeCulture(bool bNewValue);

	int AI_getBestBuildValue(int iIndex);
	int AI_totalBestBuildValue(CvArea* pArea);

	int AI_clearFeatureValue(int iIndex);
	BuildTypes AI_getBestBuild(int iIndex);
	int AI_countBestBuilds(CvArea* pArea);
	void AI_updateBestBuild();

	virtual int AI_cityValue() const;

    int AI_calculateWaterWorldPercent();

    int AI_getCityImportance(bool bEconomy, bool bMilitary);

    int AI_yieldMultiplier(YieldTypes eYield);
    void AI_updateSpecialYieldMultiplier();
    int AI_specialYieldMultiplier(YieldTypes eYield);

    int AI_countNumBonuses(BonusTypes eBonus, bool bIncludeOurs, bool bIncludeNeutral, int iOtherCultureThreshold, bool bLand = true, bool bWater = true);

	int AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance);
	int AI_cityThreat(bool bDangerPercent = false);

	int AI_getWorkersHave();
	int AI_getWorkersNeeded();
	void AI_changeWorkersHave(int iChange);
	BuildingTypes AI_bestAdvancedStartBuilding(int iPass);

/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/*************************************************************************************************/
    int AI_neededPermDefense();
	int AI_dangerValue(); //calculates passive Danger around a City, used for Permdefense
    int AI_CityDangerValue();

	int AI_PlotBuildValue(BuildTypes eBuild, CvPlot* pPlot);
	int AI_valueYield(YieldTypes eYield, int iAmount);
	int getAI_valueYield(YieldTypes eYield) const;
	void update_AI_valueYield();

	bool AI_isWorkerImprovement(BuildTypes eBuild) const;
    void AI_updateCitySpecializationValues();
    int AI_getCitySpecializationValue(CitySpecializationTypes eSpecialization) const;
    void AI_updateCitySpecializationValuesShortTerm();
    int AI_getCitySpecializationValueShortTerm(CitySpecializationTypes eSpecialization) const;
	CitySpecializationTypes AI_getCitySpecialization() const;
	void AI_setCitySpecialization(CitySpecializationTypes eSpecialization);

    void setAI_WorstPlotIndex(int iNewIndex);
	CvPlot* getAI_WorstPlot() const;
	void AI_updateWorstPlot();

    CvAIGroup* getAIGroup_Defense();
	void setAIGroup_Defense(CvAIGroup* pNewGroup);
    CvAIGroup* getAIGroup_Defense_Replace();
	void setAIGroup_Defense_Replace(CvAIGroup* pNewGroup);
    CvAIGroup* getAIGroup_Worker();
	void setAIGroup_Worker(CvAIGroup* pNewGroup);
	//new AI_chooseProduction functions
	bool AI_chooseEarlyProject();
	bool AI_chooseProductionDistrict();
	bool AI_chooseProductionNew();
	bool AI_chooseProductionBarbarian();
	bool AI_chooseCheapCultureBuilding();
	bool AI_chooseEarlyDefenseGroupUnit();
	bool AI_chooseHero();
	bool AI_chooseCityInvasionUnit();
	bool AI_chooseCityLairUnit();	
	bool AI_chooseWarwizardUnit();
	bool AI_chooseCitySiegeUnit();
	bool AI_chooseCounterUnit();
	bool AI_choosePermDefenseUnit();
	bool AI_chooseBestPermDefenseUnit();
	bool AI_choosePermDefenseReserveUnit();
	bool AI_chooseSettlerUnit();
	bool AI_chooseTrainingBuilding(bool bPatrol = false);
	bool AI_chooseAnyProject();
	bool AI_chooseWorkerUnit();
	bool AI_chooseMagicNodeUnit();
	bool AI_chooseNavalInvasionUnit();
	bool AI_chooseNavalSettleUnit();
	bool AI_chooseNavalCounterUnit();
	bool AI_chooseNavalExplorerUnit();

	int AI_specialistValueNew(SpecialistTypes eSpecialist);
	int AI_specialistGrowthValue(SpecialistTypes eSpecialist);
	int AI_specialistProductionValue(SpecialistTypes eSpecialist);
	int AI_specialistCommerceValue(SpecialistTypes eSpecialist);
	int AI_specialistGreatPersonValue(SpecialistTypes eSpecialist);
	int AI_specialistSpecializationValue(SpecialistTypes eSpecialist);
	int AI_specialistMiscValue(SpecialistTypes eSpecialist);

	int AI_getBuildingValue(BuildingTypes eBuilding);
	int AI_buildingGrowthValue(BuildingTypes eBuilding);
	int AI_buildingProductionValue(BuildingTypes eBuilding);
	int AI_buildingCommerceValue(BuildingTypes eBuilding);
	int AI_buildingMilitaryValue(BuildingTypes eBuilding);
	int AI_buildingGreatPersonValue(BuildingTypes eBuilding);
	int AI_buildingHappinessValue(BuildingTypes eBuilding);
	int AI_buildingReligionValue(BuildingTypes eBuilding);
	int AI_buildingMiscValue(BuildingTypes eBuilding);
	int AI_buildingRevolutionValue(BuildingTypes eBuilding);
	int AI_buildingWorldWonderValue(BuildingTypes eBuilding);
	int AI_getValueAnyUnit();
	int AI_getUnitValue(UnitTypes eUnit);
	int AI_ValueUnitBaseCombat(UnitTypes eUnit, bool bOnlyDefense = false, bool bWater = false);
	int AI_ValueCityCounter(UnitTypes eUnit);
	int AI_ValueCityDefender(UnitTypes eUnit, CvAIGroup* pAIGroup);
	int AI_ValueCityInvasionUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);
	int AI_ValueWarwizardUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);	
	int AI_ValueCitySiegeUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);	
	int AI_ValueNavalCounterUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);	
	int AI_ValueNavalSettleUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);	
	int AI_ValueNavalInvasionUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);	
	int AI_ValueNavalExplorerUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);	
	int AI_ValueCounterUnit(CvAIGroup* pAIGroup, UnitTypes eUnit);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	New Tag Defs	(EmphasizeInfos)				11/15/08						Jean Elcard	**/
/**	ADDON (GrowthControl) merged Sephi															**/
/**								Defines Function for Use in .cpp								**/
/*************************************************************************************************/
	bool AI_stopGrowth();
	int AI_getEmphasizeAvoidAngryCitizensCount();
	bool AI_isEmphasizeAvoidAngryCitizens();
	int AI_getEmphasizeAvoidUnhealthyCitizensCount();
	bool AI_isEmphasizeAvoidUnhealthyCitizens();
/*************************************************************************************************/
/**	New Tag Defs							END													**/
/*************************************************************************************************/
	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

protected:

	int m_iEmphasizeAvoidGrowthCount;
	int m_iEmphasizeGreatPeopleCount;

	bool m_bAssignWorkDirty;
	bool m_bChooseProductionDirty;

	IDInfo m_routeToCity;

	int* m_aiEmphasizeYieldCount;
	int* m_aiEmphasizeCommerceCount;
	bool m_bForceEmphasizeCulture;

	int m_aiBestBuildValue[NUM_CITY_PLOTS];

	BuildTypes m_aeBestBuild[NUM_CITY_PLOTS];

	bool* m_pbEmphasize;

	int* m_aiSpecialYieldMultiplier;

	int m_iCachePlayerClosenessTurn;
	int m_iCachePlayerClosenessDistance;
	int* m_aiPlayerCloseness;

	int m_iNeededFloatingDefenders;
	int m_iNeededFloatingDefendersCacheTurn;

	int m_iWorkersNeeded;
	int m_iWorkersHave;

/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/*************************************************************************************************/
	bool* m_abWorkerImprovement;
	int* m_aiAI_valueYield;
    int* m_aiCitySpecializationValue;
    int* m_aiCitySpecializationValueShortTerm;
	int m_iCitySpecialization;

    int m_iAI_WorstPlotIndex;

	int m_iAIGroup_Defense;
	int m_iAIGroup_Defense_Replace;
	int m_iAIGroup_Worker;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	New Tag Defs	(EmphasizeInfos)				11/15/08						Jean Elcard	**/
/**	ADDON (GrowthControl) merged Sephi															**/
/**								Defines Variable for Use in .cpp								**/
/*************************************************************************************************/
	int m_iEmphasizeAvoidAngryCitizensCount;
	int m_iEmphasizeAvoidUnhealthyCitizensCount;
/*************************************************************************************************/
/**	New Tag Defs							END													**/
/*************************************************************************************************/
	void AI_doDraft(bool bForce = false);
	void AI_doHurry(bool bForce = false);
	void AI_doEmphasize();
	int AI_getHappyFromHurry(HurryTypes eHurry);
	int AI_getHappyFromHurry(HurryTypes eHurry, UnitTypes eUnit, bool bIgnoreNew);
	int AI_getHappyFromHurry(HurryTypes eHurry, BuildingTypes eBuilding, bool bIgnoreNew);
	int AI_getHappyFromHurry(int iHurryPopulation);
	bool AI_doPanic();
	int AI_calculateCulturePressure(bool bGreatWork = false);


	bool AI_chooseUnit(UnitAITypes eUnitAI = NO_UNITAI);
	bool AI_chooseUnit(UnitTypes eUnit, UnitAITypes eUnitAI);

	bool AI_chooseDefender();
	bool AI_chooseLeastRepresentedUnit(UnitTypeWeightArray &allowedTypes);
	bool AI_chooseBuilding(int iFocusFlags = 0, int iMaxTurns = MAX_INT, int iMinThreshold = 0);
	bool AI_chooseProject();
	bool AI_chooseProcess(CommerceTypes eCommerceType = NO_COMMERCE);

	bool AI_bestSpreadUnit(bool bMissionary, bool bExecutive, int iBaseChance, UnitTypes* eBestSpreadUnit, int* iBestSpreadUnitValue);
	bool AI_addBestCitizen(bool bWorkers, bool bSpecialists, int* piBestPlot = NULL, SpecialistTypes* peBestSpecialist = NULL);
	bool AI_removeWorstCitizen(SpecialistTypes eIgnoreSpecialist = NO_SPECIALIST);
	void AI_juggleCitizens();

	bool AI_potentialPlot(short* piYields);
	bool AI_foodAvailable(int iExtra = 0);
	int AI_yieldValue(short* piYields, short* piCommerceYields, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false, bool bIgnoreStarvation = false, bool bWorkerOptimization = false);
	int AI_plotValue(CvPlot* pPlot, bool bAvoidGrowth, bool bRemove, bool bIgnoreFood = false, bool bIgnoreGrowth = false, bool bIgnoreStarvation = false);

	int AI_plotValueNew(CvPlot* pPlot);
	int AI_commerceValue(CommerceTypes eCommerce, int iValue);
	int AI_cityValue_Food(int iFoodAmount);
	int AI_plotValue_Food(CvPlot* pPlot);
	int AI_plotValue_Production(CvPlot* pPlot);
	int AI_plotValue_Commerce(CvPlot* pPlot);
	int AI_plotValue_Misc(CvPlot* pPlot);

	int AI_experienceWeight();
	int AI_buildUnitProb();

	void AI_bestPlotBuild(CvPlot* pPlot);
//	void AI_bestPlotBuild_parallel(void *arg);
	void AI_setBestPlotBuild(CvPlot* pPlot, int iBestValue, BuildTypes eBestBuild);	//added Sephi

	void AI_buildGovernorChooseProduction();

	int AI_getYieldMagicValue(const int* piYieldsTimes100, bool bHealthy);
	int AI_getPlotMagicValue(CvPlot* pPlot, bool bHealthy, bool bWorkerOptimization = false);
	int AI_countGoodTiles(bool bHealthy, bool bUnworkedOnly, int iThreshold = 50, bool bWorkerOptimization = false);
	int AI_countGoodSpecialists(bool bHealthy);
	int AI_calculateTargetCulturePerTurn();

	void AI_stealPlots();

	int AI_buildingSpecialYieldChangeValue(BuildingTypes kBuilding, YieldTypes eYield);

	void AI_cachePlayerCloseness(int iMaxDistance);
	void AI_updateWorkersNeededHere();

	// added so under cheat mode we can call protected functions for testing
	friend class CvGameTextMgr;
};

#endif
