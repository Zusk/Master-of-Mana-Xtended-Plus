#pragma once

// playerAI.h

#ifndef CIV4_PLAYER_AI_H
#define CIV4_PLAYER_AI_H

#include "CvPlayer.h"
#include "AI_defines.h"

class CvEventTriggerInfo;

class CvPlayerAI : public CvPlayer
{

public:

	CvPlayerAI();
	virtual ~CvPlayerAI();

  // inlined for performance reasons
#ifdef _USRDLL
  static CvPlayerAI& getPlayer(PlayerTypes ePlayer) 
  {
	  FAssertMsg(ePlayer != NO_PLAYER, "Player is not assigned a valid value");
	  FAssertMsg(ePlayer < MAX_PLAYERS, "Player is not assigned a valid value");
	  return m_aPlayers[ePlayer]; 
  }
#endif
	DllExport static CvPlayerAI& getPlayerNonInl(PlayerTypes ePlayer);

	static void initStatics();
	static void freeStatics();
	DllExport static bool areStaticsInitialized();

	void AI_init();
	void AI_uninit();
	void AI_reset(bool bConstructor);

	int AI_getFlavorValue(FlavorTypes eFlavor) const;

	void AI_doTurnPre();
	void AI_doTurnPost();
	void AI_doTurnUnitsPre();
	void AI_doTurnUnitsPost();

	void AI_doPeace();

	void AI_updateFoundValues(bool bStartingLoc = false) const;
	void AI_updateAreaTargets();

	int AI_movementPriority(CvSelectionGroup* pGroup) const;
	void AI_unitUpdate();

	void AI_makeAssignWorkDirty();
	void AI_assignWorkingPlots();
	void AI_updateAssignWork();

	void AI_makeProductionDirty();

	void AI_conquerCity(CvCity* pCity);

	bool AI_acceptUnit(CvUnit* pUnit) const;
	bool AI_captureUnit(UnitTypes eUnit, CvPlot* pPlot) const;

	DomainTypes AI_unitAIDomainType(UnitAITypes eUnitAI) const;

	int AI_yieldWeight(YieldTypes eYield) const;
	int AI_commerceWeight(CommerceTypes eCommerce, CvCity* pCity = NULL) const;

	int AI_foundValue(int iX, int iY, int iMinRivalRange = -1, bool bStartingLoc = false) const;

	bool AI_isAreaAlone(CvArea* pArea) const;
	bool AI_isCapitalAreaAlone() const;
	bool AI_isPrimaryArea(CvArea* pArea) const;

	int AI_militaryWeight(CvArea* pArea) const;

	int AI_targetCityValue(CvCity* pCity, bool bRandomize, bool bIgnoreAttackers = false) const;
	CvCity* AI_findTargetCity(CvArea* pArea) const;

	bool AI_isCommercePlot(CvPlot* pPlot) const;
	int AI_getPlotDanger(CvPlot* pPlot, int iRange = -1, bool bTestMoves = true) const;
	int AI_getUnitDanger(CvUnit* pUnit, int iRange = -1, bool bTestMoves = true, bool bAnyDanger = true) const;
	int AI_getWaterDanger(CvPlot* pPlot, int iRange, bool bTestMoves = true) const;

	bool AI_avoidScience() const;
	bool AI_isFinancialTrouble() const;
	int AI_goldTarget() const;

	TechTypes AI_bestTech(int iMaxPathLength = 1, bool bIgnoreCost = false, bool bAsync = false, TechTypes eIgnoreTech = NO_TECH, AdvisorTypes eIgnoreAdvisor = NO_ADVISOR) const;
	void AI_chooseFreeTech();
	void AI_chooseResearch();

	DllExport DiploCommentTypes AI_getGreeting(PlayerTypes ePlayer) const;
	bool AI_isWillingToTalk(PlayerTypes ePlayer) const;
	bool AI_demandRebukedSneak(PlayerTypes ePlayer) const;
	bool AI_demandRebukedWar(PlayerTypes ePlayer) const;
	bool AI_hasTradedWithTeam(TeamTypes eTeam) const;

	AttitudeTypes AI_getAttitude(PlayerTypes ePlayer, bool bForced = true) const;
	int AI_getAttitudeVal(PlayerTypes ePlayer, bool bForced = true) const;
	static AttitudeTypes AI_getAttitudeFromValue(int iAttitudeVal);

	int AI_calculateStolenCityRadiusPlots(PlayerTypes ePlayer) const;
	int AI_getCloseBordersAttitude(PlayerTypes ePlayer) const;

	int AI_getWarAttitude(PlayerTypes ePlayer) const;
	int AI_getPeaceAttitude(PlayerTypes ePlayer) const;
	int AI_getSameReligionAttitude(PlayerTypes ePlayer) const;
	int AI_getDifferentReligionAttitude(PlayerTypes ePlayer) const;
	int AI_getBonusTradeAttitude(PlayerTypes ePlayer) const;
	int AI_getOpenBordersAttitude(PlayerTypes ePlayer) const;
	int AI_getDefensivePactAttitude(PlayerTypes ePlayer) const;
	int AI_getRivalDefensivePactAttitude(PlayerTypes ePlayer) const;
	int AI_getRivalVassalAttitude(PlayerTypes ePlayer) const;
	int AI_getShareWarAttitude(PlayerTypes ePlayer) const;
	int AI_getFavoriteCivicAttitude(PlayerTypes ePlayer) const;
	int AI_getTradeAttitude(PlayerTypes ePlayer) const;
	int AI_getRivalTradeAttitude(PlayerTypes ePlayer) const;
	int AI_getMemoryAttitude(PlayerTypes ePlayer, MemoryTypes eMemory) const;
	int AI_getColonyAttitude(PlayerTypes ePlayer) const;

	PlayerVoteTypes AI_diploVote(const VoteSelectionSubData& kVoteData, VoteSourceTypes eVoteSource, bool bPropose);

	int AI_dealVal(PlayerTypes ePlayer, const CLinkList<TradeData>* pList, bool bIgnoreAnnual = false, int iExtra = 1) const;
	bool AI_goldDeal(const CLinkList<TradeData>* pList) const;
	bool AI_considerOffer(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, int iChange = 1) const;
	bool AI_counterPropose(PlayerTypes ePlayer, const CLinkList<TradeData>* pTheirList, const CLinkList<TradeData>* pOurList, CLinkList<TradeData>* pTheirInventory, CLinkList<TradeData>* pOurInventory, CLinkList<TradeData>* pTheirCounter, CLinkList<TradeData>* pOurCounter) const;

	DllExport int AI_maxGoldTrade(PlayerTypes ePlayer) const;

	DllExport int AI_maxGoldPerTurnTrade(PlayerTypes ePlayer) const;
	int AI_goldPerTurnTradeVal(int iGoldPerTurn) const;

	int AI_bonusVal(BonusTypes eBonus, int iChange = 1) const;
	int AI_baseBonusVal(BonusTypes eBonus) const;
	int AI_bonusTradeVal(BonusTypes eBonus, PlayerTypes ePlayer, int iChange) const;
	DenialTypes AI_bonusTrade(BonusTypes eBonus, PlayerTypes ePlayer) const;
	int AI_corporationBonusVal(BonusTypes eBonus) const;

	int AI_bonusVal_new(BonusTypes eBonus, int iChange) const;
	int AI_bonusHealthValue(BonusTypes eBonus, int iChange, int iBonusCount) const;
	int AI_bonusHappinessValue(BonusTypes eBonus, int iChange, int iBonusCount) const;
	int AI_bonusYieldCommerceValue(BonusTypes eBonus, int iChange, int iBonusCount) const;

	int AI_cityTradeVal(CvCity* pCity) const;
	DenialTypes AI_cityTrade(CvCity* pCity, PlayerTypes ePlayer) const;

	int AI_stopTradingTradeVal(TeamTypes eTradeTeam, PlayerTypes ePlayer) const;
	DenialTypes AI_stopTradingTrade(TeamTypes eTradeTeam, PlayerTypes ePlayer) const;

	int AI_civicTradeVal(CivicTypes eCivic, PlayerTypes ePlayer) const;
	DenialTypes AI_civicTrade(CivicTypes eCivic, PlayerTypes ePlayer) const;

	int AI_religionTradeVal(ReligionTypes eReligion, PlayerTypes ePlayer) const;
	DenialTypes AI_religionTrade(ReligionTypes eReligion, PlayerTypes ePlayer) const;

	int AI_unitImpassableCount(UnitTypes eUnit) const;
	int AI_unitValue(UnitTypes eUnit, UnitAITypes eUnitAI, CvArea* pArea) const;
	int AI_totalUnitAIs(UnitAITypes eUnitAI) const;
	int AI_totalAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) const;
	int AI_totalWaterAreaUnitAIs(CvArea* pArea, UnitAITypes eUnitAI) const;
	int AI_countCargoSpace(UnitAITypes eUnitAI) const;

	int AI_neededExplorers(CvArea* pArea) const;
	int AI_neededWorkers(CvArea* pArea) const;
	int AI_neededMissionaries(CvArea* pArea, ReligionTypes eReligion) const;
	int AI_neededExecutives(CvArea* pArea, CorporationTypes eCorporation) const;
	
	int AI_missionaryValue(CvArea* pArea, ReligionTypes eReligion, PlayerTypes* peBestPlayer = NULL) const;
	int AI_executiveValue(CvArea* pArea, CorporationTypes eCorporation, PlayerTypes* peBestPlayer = NULL) const;
	
	int AI_corporationValue(CorporationTypes eCorporation, CvCity* pCity = NULL) const;
	
	int AI_adjacentPotentialAttackers(CvPlot* pPlot, bool bTestCanMove = false) const;
	int AI_totalMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL) const;
	int AI_areaMissionAIs(CvArea* pArea, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL) const;
	int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0) const;
	int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0) const;
	int AI_plotTargetMissionAIs(CvPlot* pPlot, MissionAITypes* aeMissionAI, int iMissionAICount, int& iClosestTargetRange, CvSelectionGroup* pSkipSelectionGroup = NULL, int iRange = 0) const;
	int AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL) const;
	int AI_unitTargetMissionAIs(CvUnit* pUnit, MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup = NULL) const;
	int AI_enemyTargetMissionAIs(MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL) const;
	int AI_enemyTargetMissionAIs(MissionAITypes* aeMissionAI, int iMissionAICount, CvSelectionGroup* pSkipSelectionGroup = NULL) const;
	int AI_wakePlotTargetMissionAIs(CvPlot* pPlot, MissionAITypes eMissionAI, CvSelectionGroup* pSkipSelectionGroup = NULL) const;
	

	CivicTypes AI_bestCivic(CivicOptionTypes eCivicOption) const;
	int AI_civicValue(CivicTypes eCivic) const;

	ReligionTypes AI_bestReligion() const;
	int AI_religionValue(ReligionTypes eReligion) const;

	EspionageMissionTypes AI_bestPlotEspionage(CvPlot* pSpyPlot, PlayerTypes& eTargetPlayer, CvPlot*& pPlot, int& iData) const;
	int AI_espionageVal(PlayerTypes eTargetPlayer, EspionageMissionTypes eMission, CvPlot* pPlot, int iData) const;

	int AI_getPeaceWeight() const;
	void AI_setPeaceWeight(int iNewValue);

	int AI_getEspionageWeight() const;
	void AI_setEspionageWeight(int iNewValue);

	int AI_getAttackOddsChange() const;
	void AI_setAttackOddsChange(int iNewValue);

	int AI_getCivicTimer() const;
	void AI_setCivicTimer(int iNewValue);
	void AI_changeCivicTimer(int iChange);

	int AI_getReligionTimer() const;
	void AI_setReligionTimer(int iNewValue);
	void AI_changeReligionTimer(int iChange);

	int AI_getExtraGoldTarget() const;
	void AI_setExtraGoldTarget(int iNewValue);

	int AI_getNumTrainAIUnits(UnitAITypes eIndex) const;
	void AI_changeNumTrainAIUnits(UnitAITypes eIndex, int iChange);

	int AI_getNumAIUnits(UnitAITypes eIndex) const;
	void AI_changeNumAIUnits(UnitAITypes eIndex, int iChange);

	int AI_getSameReligionCounter(PlayerTypes eIndex) const;
	void AI_changeSameReligionCounter(PlayerTypes eIndex, int iChange);

	int AI_getDifferentReligionCounter(PlayerTypes eIndex) const;
	void AI_changeDifferentReligionCounter(PlayerTypes eIndex, int iChange);

	int AI_getFavoriteCivicCounter(PlayerTypes eIndex) const;
	void AI_changeFavoriteCivicCounter(PlayerTypes eIndex, int iChange);

	int AI_getBonusTradeCounter(PlayerTypes eIndex) const;
	void AI_changeBonusTradeCounter(PlayerTypes eIndex, int iChange);

	int AI_getPeacetimeTradeValue(PlayerTypes eIndex) const;
	void AI_changePeacetimeTradeValue(PlayerTypes eIndex, int iChange);

	int AI_getPeacetimeGrantValue(PlayerTypes eIndex) const;
	void AI_changePeacetimeGrantValue(PlayerTypes eIndex, int iChange);

	int AI_getGoldTradedTo(PlayerTypes eIndex) const;
	void AI_changeGoldTradedTo(PlayerTypes eIndex, int iChange);

	int AI_getAttitudeExtra(PlayerTypes eIndex) const;
	void AI_setAttitudeExtra(PlayerTypes eIndex, int iNewValue);
	void AI_changeAttitudeExtra(PlayerTypes eIndex, int iChange);

	bool AI_isFirstContact(PlayerTypes eIndex) const;
	void AI_setFirstContact(PlayerTypes eIndex, bool bNewValue);

	int AI_getContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2) const;
	void AI_changeContactTimer(PlayerTypes eIndex1, ContactTypes eIndex2, int iChange);

	int AI_getMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2) const;
	void AI_changeMemoryCount(PlayerTypes eIndex1, MemoryTypes eIndex2, int iChange);

	int AI_calculateGoldenAgeValue() const;

	void AI_doCommerce();

	EventTypes AI_chooseEvent(int iTriggeredId) const;
	virtual void AI_launch(VictoryTypes eVictory);

	int AI_getCultureVictoryStage() const;
	
	int AI_cultureVictoryTechValue(TechTypes eTech) const;
	
	bool AI_isDoStrategy(int iStrategy) const;

	void AI_nowHasTech(TechTypes eTech);
	
    int AI_countDeadlockedBonuses(CvPlot* pPlot) const;
    
    int AI_getOurPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves) const;
    int AI_getEnemyPlotStrength(CvPlot* pPlot, int iRange, bool bDefensiveBonuses, bool bTestMoves) const;

	int AI_goldToUpgradeAllUnits(int iExpThreshold = 0) const;

	int AI_goldTradeValuePercent() const;
	
	int AI_averageYieldMultiplier(YieldTypes eYield) const;
	int AI_averageCommerceMultiplier(CommerceTypes eCommerce) const;
	int AI_averageGreatPeopleMultiplier() const;
	int AI_averageCommerceExchange(CommerceTypes eCommerce) const;
	
	int AI_playerCloseness(PlayerTypes eIndex, int iMaxDistance) const;
	
	int AI_getTotalCityThreat() const;
	int AI_getTotalFloatingDefenseNeeded() const;
	
	
	int AI_getTotalAreaCityThreat(CvArea* pArea) const;
	int AI_countNumAreaHostileUnits(CvArea* pArea, bool bPlayer, bool bTeam, bool bNeutral, bool bHostile) const;
	int AI_getTotalFloatingDefendersNeeded(CvArea* pArea) const;
	int AI_getTotalFloatingDefenders(CvArea* pArea) const;

	RouteTypes AI_bestAdvancedStartRoute(CvPlot* pPlot, int* piYieldValue = NULL) const;
	UnitTypes AI_bestAdvancedStartUnitAI(CvPlot* pPlot, UnitAITypes eUnitAI) const;
	CvPlot* AI_advancedStartFindCapitalPlot() const;
	
	bool AI_advancedStartPlaceExploreUnits(bool bLand);
	void AI_advancedStartRevealRadius(CvPlot* pPlot, int iRadius);
	bool AI_advancedStartPlaceCity(CvPlot* pPlot);
	bool AI_advancedStartDoRoute(CvPlot* pFromPlot, CvPlot* pToPlot);
	void AI_advancedStartRouteTerritory();
	void AI_doAdvancedStart(bool bNoExit = false);
	
	int AI_getMinFoundValue() const;
	
	void AI_recalculateFoundValues(int iX, int iY, int iInnerRadius, int iOuterRadius) const;
	
	void AI_updateCitySites(int iMinFoundValueThreshold, int iMaxSites) const;
	void AI_invalidateCitySites(int iMinFoundValueThreshold) const;
	bool AI_isPlotCitySite(CvPlot* pPlot) const;
	int AI_getNumAreaCitySites(int iAreaID, int& iBestValue) const;
	int AI_getNumAdjacentAreaCitySites(int iWaterAreaID, int iExcludeArea, int& iBestValue) const;
	
	int AI_getNumCitySites() const;
	CvPlot* AI_getCitySite(int iIndex) const;
	
	int AI_bestAreaUnitAIValue(UnitAITypes eUnitAI, CvArea* pArea, UnitTypes* peBestUnitType = NULL) const;
	int AI_bestCityUnitAIValue(UnitAITypes eUnitAI, CvCity* pCity, UnitTypes* peBestUnitType = NULL) const;
	
	int AI_calculateTotalBombard(DomainTypes eDomain) const;
	
	int AI_getUnitClassWeight(UnitClassTypes eUnitClass) const;
	int AI_getUnitCombatWeight(UnitCombatTypes eUnitCombat) const;
	int AI_calculateUnitAIViability(UnitAITypes eUnitAI, DomainTypes eDomain) const;
	
	void AI_updateBonusValue();
	void AI_updateBonusValue(BonusTypes eBonus);
	
	int AI_getAttitudeWeight(PlayerTypes ePlayer) const;

	ReligionTypes AI_chooseReligion();
	
	int AI_getPlotAirbaseValue(CvPlot* pPlot) const;
	int AI_getPlotCanalValue(CvPlot* pPlot) const;

	bool AI_isPlotThreatened(CvPlot* pPlot, int iRange = -1, bool bTestMoves = true) const;

	bool AI_isFirstTech(TechTypes eTech) const;

//FfH: Added by Kael 08/15/2007
    int AI_getAlignmentAttitude(PlayerTypes ePlayer) const;
    int AI_getBadBonusAttitude(PlayerTypes ePlayer) const;
    int AI_getFavoriteWonderAttitude(PlayerTypes ePlayer) const;
    int AI_getGenderAttitude(PlayerTypes ePlayer) const;
    int AI_getTrustAttitude(PlayerTypes ePlayer) const;
    int AI_getCivicShareAttitude(PlayerTypes ePlayer) const;
    int AI_trueCombatValue(UnitTypes eUnit) const;
    int AI_combatValue(UnitTypes eUnit) const;
//FfH: End Add
/*************************************************************************************************/
/**	New Gameoption (Show Hidden Diplomatic Modifiers) Sephi                    				    **/
/*************************************************************************************************/
	int AI_getFirstImpressionAttitude(PlayerTypes ePlayer) const;
	int AI_getTeamSizeAttitude(PlayerTypes ePlayer) const;
	int AI_getRankDifferenceAttitude(PlayerTypes ePlayer) const;
	int AI_getLowRankAttitude(PlayerTypes ePlayer) const;
	int AI_getLostWarAttitude(PlayerTypes ePlayer) const;

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
	void AI_invalidateAttitudeCache(PlayerTypes ePlayer);
	void AI_invalidatePlotDangerCache(int iPlotIndex);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/** BETTER AI (function definitions) Sephi                                                      **/
/*************************************************************************************************/
/** BETTER AI (HatedCivic) merged Sephi		Opera		31.05.09	    						**/
    int AI_getHatedCivicAttitude(PlayerTypes ePlayer) const;
    int AI_getHatedCivicCounter(PlayerTypes eIndex) const;
	void AI_changeHatedCivicCounter(PlayerTypes eIndex, int iChange) const;
/** BETTER AI (better AI gold management) Sephi                                                 **/
    int AI_getGoldTreasury(bool bVictoryHurry, bool bHurry, bool bTrading, bool bReserve) const;
/** BETTER AI (Naval AI) Sephi                                                                   **/
	bool canPlayerReachPlotBySea(CvPlot* pTarget, bool bNotAlreadySettled = true);
	bool canPlayerReachAreaBySea(int iArea);
	int AI_bestAreaToReachPlotbySea(CvPlot* pTarget);
	CvPlot* AI_getExpansionPlot() const;
	void AI_setExpansionPlot(CvPlot* newPlot);
	void AI_doExpansion();
	void AI_doCitySpecialization();
	int AI_getSiegeStrategy() const;
	void AI_setSiegeStrategy(int iNewStrategy);
	void AI_updateSiegeStrategy();
	void AI_updateVictoryStrategy();
	int AI_foundValueNew(int iX, int iY, int iMinRivalRange = -1) const;
	int AI_plotValue(CvPlot* pLoopPlot) const;
	void updateAIGroups();
	void updateWarTargets();
	bool isInvasionGroupNotFull(int iAIGroupType, int iAreaID) const;
	bool AI_isValidCityforWar(CvCity* pTargetCity, int* piValue);
	void AI_doGlobalCulture();
	void AI_doMagic();
	void AI_doFaith();
	ManaschoolTypes AI_getManaschoolFocus();
	ProjectTypes AI_getMagicTargetProject();
	void AI_setMagicTargetProject(ProjectTypes newProject);
	int AI_getMagicStrategy();
	int AI_ValueManaNode(int iBonusType);
	void update_BestManaNode();
	CvAIGroup* AI_getManaNodeGroup(int iArea);
	int AI_getEconomyType();
	void AI_chooseEquipment(CvUnit* pUnit);
	int AI_EquipmentValue(CvUnit* pUnit, PromotionTypes Equipment);
/** Better AI (Tech Choose) Sephi																**/
	TechTypes AI_bestTechNew(int iMaxPathLength, bool bIgnoreCost, bool bAsync, TechTypes eIgnoreTech, AdvisorTypes eIgnoreAdvisor) const;
	int AI_techValueNew(TechTypes iTech, bool bIgnoreCost) const;
	int CvPlayerAI::AI_getGuildValue(TechTypes eTech) const;
	bool CvPlayerAI::hasGuild(std::vector<TechTypes> eTech) const;
	TechTypes AI_needEarlyTech() const;
	TechTypes AI_nextTechForIncreaseStrengthOfRegularUnits() const;
	TechTypes AI_nextTechForTier(int iTier) const;
	TechTypes AI_needMilitaryTech() const;
	bool AI_isMilitaryTierNeeded(int iTier) const;
	int AI_techValueMelee(TechTypes iTech) const;
	int AI_techValueEngineering(TechTypes iTech) const;
	int AI_techValueRecon(TechTypes iTech) const;
	int AI_techValueRanged(TechTypes iTech) const;
	int AI_techValueMounted(TechTypes iTech) const;
	int AI_techValueMilitary(TechTypes iTech) const;
	int AI_techValueMagic(TechTypes iTech) const;
	int AI_techValueScience(TechTypes iTech) const;
	int AI_techValueHappy(TechTypes iTech) const;
	int AI_techValueNaval(TechTypes iTech) const;
	int AI_techValueMetal(TechTypes iTech) const;	
	int AI_techValueGold(TechTypes iTech) const;
	int AI_techValueReligion(TechTypes iTech) const;
	int AI_techValueResource(TechTypes iTech) const;
	int AI_techValueBuilds(TechTypes iTech) const;

	int AI_techValueMilitaryUnit(TechTypes iTech) const;
	int AI_techValueUniqueUnit(TechTypes iTech) const;

	int AI_getSpellResearchValue(TechTypes eSpellResearch);
	TechTypes AI_bestSpellResearch();

	int AI_getYieldValue(YieldTypes eYield);
	bool isLackMoneyForExpansion();
	int AI_getVictoryStrategyStage(int iAIVictoryStrategyType) const;
	void AI_setVictoryStrategyStage(int iAIVictoryStrategyType,int iNewValue);
	int AI_getCitySpecializationCount(int eIndex) const;
	void AI_setCitySpecializationCount(int eIndex, int iNewValue);
	void AI_updateCitySpecializationCount();
	bool isWorkerNeeded(int iArea) const;

	void calculateWarValue(TeamTypes eTeam);
	int calculateBaseWarCost(TeamTypes eTeam) const;
	void UpdateWarValueAccumulated(TeamTypes eTeam);
	bool isThreatinWar(TeamTypes eTeam) const;
	bool isThreatinWar(PlayerTypes ePlayer) const;
	bool hasCityinRange(PlayerTypes ePlayer, int iRange) const;

	void AI_updateHeroUnits();
	bool AI_chooseAnotherHeroUnit();
/*************************************************************************************************/
/** End																							**/
/*************************************************************************************************/
//>>>>BUGFfH: Added by Denev 2009/09/29
	UnitArtStyleTypes getUnitArtStyleType() const;
//<<<<BUGFfH: End Add

	// for serialization
  virtual void read(FDataStreamBase* pStream);
  virtual void write(FDataStreamBase* pStream);

protected:

	static CvPlayerAI* m_aPlayers;

	int m_iPeaceWeight;
	int m_iEspionageWeight;
	int m_iAttackOddsChange;
	int m_iCivicTimer;
	int m_iReligionTimer;
	int m_iExtraGoldTarget;
	
	mutable int m_iStrategyHash;
	mutable int m_iStrategyHashCacheTurn;
	
	
	mutable int m_iAveragesCacheTurn;
	
	mutable int m_iAverageGreatPeopleMultiplier;
	
	mutable int *m_aiAverageYieldMultiplier;
	mutable int *m_aiAverageCommerceMultiplier;
	mutable int *m_aiAverageCommerceExchange;
	
	mutable int m_iUpgradeUnitsCacheTurn;
	mutable int m_iUpgradeUnitsCachedExpThreshold;
	mutable int m_iUpgradeUnitsCachedGold;
	
	
	int *m_aiNumTrainAIUnits;
	int *m_aiNumAIUnits;
	int* m_aiSameReligionCounter;
	int* m_aiDifferentReligionCounter;
	int* m_aiFavoriteCivicCounter;
/*************************************************************************************************/
/** BETTER AI (HatedCivic) merged Sephi		Opera		31.05.09	    						**/
/*************************************************************************************************/
	int* m_aiHatedCivicCounter;
/** BETTER AI (Naval AI) Sephi                                                                   **/
    int m_iExpansionX;
    int m_iExpansionY;
    int m_iSiegeStrategy;
    int m_iAIManaschoolFocus;
	int m_iAIEconomy;
	int m_iAIMagicTargetProject;
	int m_iAIMagicStrategy;
	int* m_aiAIVictoryStrategyStage;
	int* m_aiAICitySpecializationCount;
/*************************************************************************************************/
/** End																								**/
/*************************************************************************************************/
	int* m_aiBonusTradeCounter;
	int* m_aiPeacetimeTradeValue;
	int* m_aiPeacetimeGrantValue;
	int* m_aiGoldTradedTo;
	int* m_aiAttitudeExtra;
	int* m_aiBonusValue;
	int* m_aiUnitClassWeights;
	int* m_aiUnitCombatWeights;

	mutable int* m_aiCloseBordersAttitudeCache;
/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
	mutable int* m_aiAttitudeCache;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	bool* m_abFirstContact;

	int** m_aaiContactTimer;
	int** m_aaiMemoryCount;
	
	mutable std::vector<int> m_aiAICitySites;
	
	bool m_bWasFinancialTrouble;
	int m_iTurnLastProductionDirty;


	void AI_doCounter();
	void AI_doMilitary();
	void AI_doResearch();
	void AI_doCivics();
	void AI_doReligion();
	void AI_doDiplo();
	void AI_doSplit();
	void AI_doCheckFinancialTrouble();
	
	bool AI_disbandUnit(int iExpThreshold, bool bObsolete);
	
	int AI_getStrategyHash() const;
	void AI_calculateAverages() const;
	
	int AI_getHappinessWeight(int iHappy, int iExtraPop) const;
	int AI_getHealthWeight(int iHealth, int iExtraPop) const;
	
	void AI_convertUnitAITypesForCrush();
	int AI_eventValue(EventTypes eEvent, const EventTriggeredData& kTriggeredData) const;
		
	void AI_doEnemyUnitData();
	void AI_invalidateCloseBordersAttitudeCache();
	
	friend class CvGameTextMgr;
};

// helper for accessing static functions
#ifdef _USRDLL
#define GET_PLAYER CvPlayerAI::getPlayer
#else
#define GET_PLAYER CvPlayerAI::getPlayerNonInl
#endif

#endif
