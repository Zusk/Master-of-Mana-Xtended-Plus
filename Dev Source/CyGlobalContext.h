#pragma once

#ifndef CyGlobalContext_h
#define CyGlobalContext_h

//
// Python wrapper class for global vars and fxns
// Passed to Python
//

#include "CvGlobals.h"
#include "CvArtFileMgr.h"

// lfgr UI 11/2020: Allow cycling through units in plot help
#include "PlotHelpCycling.h"

class CyGame;
class CyMap;
class CyPlayer;
class CvRandom;
class CyEngine;
class CyTeam;
class CyArtFileMgr;
class CyUserProfile;
class CyVariableSystem;

/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
class CyWinamp;
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/

class CyGlobalContext
{
public:
	CyGlobalContext();
	virtual ~CyGlobalContext();

	static CyGlobalContext& getInstance();		// singleton accessor

	bool isDebugBuild() const;
	CyGame* getCyGame() const;
	CyMap* getCyMap() const;
	CyPlayer* getCyPlayer(int idx);
	CyPlayer* getCyActivePlayer();
	CvRandom& getCyASyncRand() const;
	CyTeam* getCyTeam(int i);
	CyArtFileMgr* getCyArtFileMgr() const;

	CvEffectInfo* getEffectInfo(int i) const;
	CvTerrainInfo* getTerrainInfo(int i) const;
	CvBonusClassInfo* getBonusClassInfo(int i) const;
	CvBonusInfo* getBonusInfo(int i) const;
	CvFeatureInfo* getFeatureInfo(int i) const;
	CvCivilizationInfo* getCivilizationInfo(int idx) const;
	CvLeaderHeadInfo* getLeaderHeadInfo(int i) const;
	CvTraitInfo* getTraitInfo(int i) const;
	CvUnitInfo* getUnitInfo(int i) const;
	CvSpecialUnitInfo* getSpecialUnitInfo(int i) const;
	CvYieldInfo* getYieldInfo(int i) const;
	CvCommerceInfo* getCommerceInfo(int i) const;
	CvRouteInfo* getRouteInfo(int i) const;
	CvImprovementInfo* getImprovementInfo(int i) const;
	CvGoodyInfo* getGoodyInfo(int i) const;
	CvBuildInfo* getBuildInfo(int i) const;
	CvHandicapInfo* getHandicapInfo(int i) const;
	CvGameSpeedInfo* getGameSpeedInfo(int i) const;
	CvTurnTimerInfo* getTurnTimerInfo(int i) const;
	CvBuildingClassInfo* getBuildingClassInfo(int i) const;
	CvMissionInfo* getMissionInfo(int i) const;
	CvCommandInfo* getCommandInfo(int i) const;
	CvAutomateInfo* getAutomateInfo(int i) const;
	CvActionInfo* getActionInfo(int i) const;
	CvUnitClassInfo* getUnitClassInfo(int i) const;
	CvInfoBase* getUnitCombatInfo(int i) const;
	CvInfoBase* getDomainInfo(int i) const;
	CvBuildingInfo* getBuildingInfo(int i) const;
	CvCivicOptionInfo* getCivicOptionInfo(int i) const;
	CvCivicInfo* getCivicInfo(int i) const;
	CvDiplomacyInfo* getDiplomacyInfo(int i) const;
	CvProjectInfo* getProjectInfo(int i) const;
	CvVoteInfo* getVoteInfo(int i) const;
	CvProcessInfo* getProcessInfo(int i) const;
	CvSpecialistInfo* getSpecialistInfo(int i) const;
	CvReligionInfo* getReligionInfo(int i) const;
	CvCorporationInfo* getCorporationInfo(int i) const;
	CvControlInfo* getControlInfo(int i) const;
	CvTechInfo* getTechInfo(int i) const;
	CvSpecialBuildingInfo* getSpecialBuildingInfo(int i) const;
	CvPromotionInfo* getPromotionInfo(int i) const;
	CvAnimationPathInfo * getAnimationPathInfo(int i) const;
	CvEmphasizeInfo * getEmphasizeInfo(int i) const;
	CvUpkeepInfo * getUpkeepInfo(int i) const;
	CvCultureLevelInfo * getCultureLevelInfo(int i) const;
	CvEraInfo * getEraInfo(int i) const;
	CvVictoryInfo * getVictoryInfo(int i) const;
	CvWorldInfo * getWorldInfo(int i) const;
	CvClimateInfo * getClimateInfo(int i) const;
	CvSeaLevelInfo * getSeaLevelInfo(int i) const;
	CvInfoBase * getUnitAIInfo(int i) const;
	CvColorInfo* getColorInfo(int i) const;
/*************************************************************************************************/
/**	ADDON (Better AI) Sephi                                      					**/
/*************************************************************************************************/
	CvInfoBase * getAIGroupInfo(int i) const;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

//FfH Spell System: Added by Kael 07/23/2007
	CvSpellInfo* getSpellInfo(int i) const;
//FfH: End Add
/*************************************************************************************************/
/**	BETTER AI (Better Promotions) Sephi                                      					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	CvAIPromotionSpecializationInfo* getAIPromotionSpecializationInfo(int i) const;
/**	ADDON (Adventure) Sephi                                                      				**/
	CvAdventureInfo* getAdventureInfo(int i) const;
	CvAdventureStepInfo* getAdventureStepInfo(int i) const;
/**	ADDON (Mana Schools) Sephi                                                      			**/
	CvManaschoolInfo* getManaschoolInfo(int i) const;
/**	ADDON (Combatauras) Sephi																	**/
	CvCombatAuraInfo* getCombatAuraInfo(int i) const;
	CvCombatAuraClassInfo* getCombatAuraClassInfo(int i) const;

	CvEquipmentCategoryInfo* getEquipmentCategoryInfo(int i) const;
	CvDurabilityInfo* getDurabilityInfo(int i) const;
	CvDungeonInfo* getDungeonInfo(int i) const;
	CvDungeonEventInfo* getDungeonEventInfo(int i) const;
	CvLairResultInfo* getLairResultInfo(int i) const;
	CvCityStateInfo* getCityStateInfo(int i) const;
	CvPlaneInfo* getPlaneInfo(int i) const;
	CvTerrainClassInfo* getTerrainClassInfo(int i) const;
	CvFeatureClassInfo* getFeatureClassInfo(int i) const;
	CvCitySpecializationInfo* getCitySpecializationInfo(int i) const;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (WINAMP) merged Sephi																	**/
/*************************************************************************************************/
	CyWinamp* getCyWinamp() const;
/*************************************************************************************************/
/**	ADDON (WINAMP)						END														**/
/*************************************************************************************************/


    CvUnitArtStyleTypeInfo* getUnitArtStyleTypeInfo(int i) const;

	int getInfoTypeForString(const char* szInfoType) const;
	int getTypesEnum(const char* szType) const;

	int getNumPlayerColorInfos() const { return GC.getNumPlayerColorInfos(); }
	CvPlayerColorInfo* getPlayerColorInfo(int i) const;

	CvInfoBase* getHints(int i) const;
	CvMainMenuInfo* getMainMenus(int i) const;
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**																								**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
// Python Modular Loading
	CvPythonModulesInfo* getPythonModulesInfo(int i) const;
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
	CvInfoBase* getInvisibleInfo(int i) const;
	CvVoteSourceInfo* getVoteSourceInfo(int i) const;
	CvInfoBase* getAttitudeInfo(int i) const;
	CvInfoBase* getMemoryInfo(int i) const;
	CvInfoBase* getConceptInfo(int i) const;
	CvInfoBase* getNewConceptInfo(int i) const;
/*************************************************************************************************/
/**	ADDON (New Pedia Categories) Sephi                                      					**/
/*************************************************************************************************/
	CvInfoBase*  getWildmanaConceptInfo(int i) const;
	CvInfoBase*  getWildmanaGuideInfo(int i) const;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	CvInfoBase* getCityTabInfo(int i) const;
	CvInfoBase* getCalendarInfo(int i) const;
	CvInfoBase* getGameOptionInfo(int i) const;
	CvInfoBase* getMPOptionInfo(int i) const;
	CvInfoBase* getForceControlInfo(int i) const;
	CvInfoBase* getSeasonInfo(int i) const;
	CvInfoBase* getMonthInfo(int i) const;
	CvInfoBase* getDenialInfo(int i) const;
	CvQuestInfo* getQuestInfo(int i) const;
	CvTutorialInfo* getTutorialInfo(int i) const;
	CvEventTriggerInfo* getEventTriggerInfo(int i) const;
	CvEventInfo* getEventInfo(int i) const;
	CvEspionageMissionInfo* getEspionageMissionInfo(int i) const;
	CvHurryInfo* getHurryInfo(int i) const;
	CvPlayerOptionInfo* getPlayerOptionInfo(int i) const;
	CvPlayerOptionInfo* getPlayerOptionsInfoByIndex(int i) const;

	CvGraphicOptionInfo* getGraphicOptionInfo(int i) const;
	CvGraphicOptionInfo* getGraphicOptionsInfoByIndex(int i) const;

	// ArtInfos
	CvArtInfoInterface* getInterfaceArtInfo(int i) const;
	CvArtInfoMovie* getMovieArtInfo(int i) const;
	CvArtInfoMisc* getMiscArtInfo(int i) const;
	CvArtInfoUnit* getUnitArtInfo(int i) const;
	CvArtInfoBuilding* getBuildingArtInfo(int i) const;
	CvArtInfoCivilization* getCivilizationArtInfo(int i) const;
	CvArtInfoLeaderhead* getLeaderheadArtInfo(int i) const;
	CvArtInfoBonus* getBonusArtInfo(int i) const;
	CvArtInfoImprovement* getImprovementArtInfo(int i) const;
	CvArtInfoTerrain* getTerrainArtInfo(int i) const;
	CvArtInfoFeature* getFeatureArtInfo(int i) const;


	// Structs

	const char* getEntityEventTypes(int i) const { return GC.getEntityEventTypes((EntityEventTypes) i); }
	const char* getAnimationOperatorTypes(int i) const { return GC.getAnimationOperatorTypes((AnimationOperatorTypes) i); }
	const char* getFunctionTypes(int i) const { return GC.getFunctionTypes((FunctionTypes) i); }
	const char* getFlavorTypes(int i) const { return GC.getFlavorTypes((FlavorTypes) i); }
	const char* getArtStyleTypes(int i) const { return GC.getArtStyleTypes((ArtStyleTypes) i); }
	const char* getCitySizeTypes(int i) const { return GC.getCitySizeTypes(i); }
	const char* getContactTypes(int i) const { return GC.getContactTypes((ContactTypes) i); }
	const char* getDiplomacyPowerTypes(int i) const { return GC.getDiplomacyPowerTypes((DiplomacyPowerTypes) i); }
	const char *getFootstepAudioTypes(int i) { return GC.getFootstepAudioTypes(i); }
	const char *getFootstepAudioTags(int i) { return GC.getFootstepAudioTags(i); }

	int getNumEffectInfos() const { return GC.getNumEffectInfos(); }
	int getNumTerrainInfos() const { return GC.getNumTerrainInfos(); }
	int getNumSpecialBuildingInfos() const { return GC.getNumSpecialBuildingInfos(); }
	int getNumBonusInfos() const { return GC.getNumBonusInfos(); };
	int getNumPlayableCivilizationInfos() const { return GC.getNumPlayableCivilizationInfos(); }
	int getNumCivilizatonInfos() const { return GC.getNumCivilizationInfos(); }
	int getNumLeaderHeadInfos() const { return GC.getNumLeaderHeadInfos(); }
	int getNumTraitInfos() const { return GC.getNumTraitInfos(); }
	int getNumUnitInfos() const { return GC.getNumUnitInfos(); }
	int getNumSpecialUnitInfos() const { return GC.getNumSpecialUnitInfos(); }
	int getNumRouteInfos() const { return GC.getNumRouteInfos(); }
	int getNumFeatureInfos() const { return GC.getNumFeatureInfos(); }
	int getNumImprovementInfos() const { return GC.getNumImprovementInfos(); }
	int getNumGoodyInfos() const { return GC.getNumGoodyInfos(); }
	int getNumBuildInfos() const { return GC.getNumBuildInfos(); }
	int getNumHandicapInfos() const { return GC.getNumHandicapInfos(); }
	int getNumGameSpeedInfos() const { return GC.getNumGameSpeedInfos(); }
	int getNumTurnTimerInfos() const { return GC.getNumTurnTimerInfos(); }
	int getNumBuildingClassInfos() const { return GC.getNumBuildingClassInfos(); }
	int getNumBuildingInfos() const { return GC.getNumBuildingInfos(); }
	int getNumUnitClassInfos() const { return GC.getNumUnitClassInfos(); }
	int getNumUnitCombatInfos() const { return GC.getNumUnitCombatInfos(); }
	int getNumAutomateInfos() const { return GC.getNumAutomateInfos(); }
	int getNumCommandInfos() const { return GC.getNumCommandInfos(); }
	int getNumControlInfos() const { return GC.getNumControlInfos(); }
	int getNumMissionInfos() const { return GC.getNumMissionInfos(); }
	int getNumActionInfos() const { return GC.getNumActionInfos(); }
	int getNumPromotionInfos() const { return GC.getNumPromotionInfos(); }

//FfH Spell System: Added by Kael 07/23/2007
	int getNumSpellInfos() const { return GC.getNumSpellInfos(); }
//FfH: End Add
/*************************************************************************************************/
/**	BETTER AI (Better Promotions) Sephi                                      					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	int getNumAIPromotionSpecializationInfos() const { return GC.getNumAIPromotionSpecializationInfos(); }
/**	ADDON (Adventure) Sephi                                                      					**/
	int getNumAdventureInfos() const { return GC.getNumAdventureInfos(); }
	int getNumAdventureStepInfos() const { return GC.getNumAdventureStepInfos(); }
/**	ADDON (Mana Schools) Sephi                                                      			**/
	int getNumManaschoolInfos() const { return GC.getNumManaschoolInfos(); }
/**	ADDON (Combatauras) Sephi																	**/
	int getNumCombatAuraInfos() const { return GC.getNumCombatAuraInfos(); }
	int getNumCombatAuraClassInfos() const { return GC.getNumCombatAuraClassInfos(); }

	int getNumEquipmentCategoryInfos() const { return GC.getNumEquipmentCategoryInfos(); }
	int getNumDurabilityInfos() const { return GC.getNumDurabilityInfos(); }
	int getNumDungeonInfos() const { return GC.getNumDungeonInfos(); }
	int getNumDungeonEventInfos() const { return GC.getNumDungeonEventInfos(); }
	int getNumLairResultInfos() const { return GC.getNumLairResultInfos(); }
	int getNumCityStateInfos() const { return GC.getNumCityStateInfos(); }
	int getNumPlaneInfos() const { return GC.getNumPlaneInfos(); }
	int getNumTerrainClassInfos() const { return GC.getNumTerrainClassInfos(); }
	int getNumFeatureClassInfos() const { return GC.getNumFeatureClassInfos(); }
	int getNumCitySpecializationInfos() const { return GC.getNumCitySpecializationInfos(); }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	int getNumTechInfos() const { return GC.getNumTechInfos(); }
	int getNumReligionInfos() const { return GC.getNumReligionInfos(); }
	int getNumCorporationInfos() const { return GC.getNumCorporationInfos(); }
	int getNumSpecialistInfos() const { return GC.getNumSpecialistInfos(); }
	int getNumCivicInfos() const { return GC.getNumCivicInfos(); }
	int getNumDiplomacyInfos() const { return GC.getNumDiplomacyInfos(); }
	int getNumCivicOptionInfos() const { return GC.getNumCivicOptionInfos(); }
	int getNumProjectInfos() const { return GC.getNumProjectInfos(); }
	int getNumVoteInfos() const { return GC.getNumVoteInfos(); }
	int getNumProcessInfos() const { return GC.getNumProcessInfos(); }
	int getNumEmphasizeInfos() const { return GC.getNumEmphasizeInfos(); }
	int getNumHurryInfos() const { return GC.getNumHurryInfos(); }
	int getNumUpkeepInfos() const { return GC.getNumUpkeepInfos(); }
	int getNumCultureLevelInfos() const { return GC.getNumCultureLevelInfos(); }
	int getNumEraInfos() const { return GC.getNumEraInfos(); }
	int getNumVictoryInfos() const { return GC.getNumVictoryInfos(); }
	int getNumWorldInfos() const { return GC.getNumWorldInfos(); }
	int getNumSeaLevelInfos() const { return GC.getNumSeaLevelInfos(); }
	int getNumClimateInfos() const { return GC.getNumClimateInfos(); }
	int getNumConceptInfos() const { return GC.getNumConceptInfos(); }
	int getNumNewConceptInfos() const { return GC.getNumNewConceptInfos(); }
/*************************************************************************************************/
/**	ADDON (New Pedia Categories) Sephi                                      					**/
/*************************************************************************************************/
	int getNumWildmanaConceptInfos() const { return GC.getNumWildmanaConceptInfos(); }
	int getNumWildmanaGuideInfos() const { return GC.getNumWildmanaGuideInfos(); }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	int getNumCityTabInfos() const { return GC.getNumCityTabInfos(); }
	int getNumCalendarInfos() const { return GC.getNumCalendarInfos(); }
	int getNumPlayerOptionInfos() const { return GC.getNumPlayerOptionInfos(); }
	int getNumGameOptionInfos() const { return GC.getNumGameOptionInfos(); }
	int getNumMPOptionInfos() const { return GC.getNumMPOptionInfos(); }
	int getNumForceControlInfos() const { return GC.getNumForceControlInfos(); }
	int getNumSeasonInfos() const { return GC.getNumSeasonInfos(); }
	int getNumMonthInfos() const { return GC.getNumMonthInfos(); }
	int getNumDenialInfos() const { return GC.getNumDenialInfos(); }
	int getNumQuestInfos() const { return GC.getNumQuestInfos(); }
	int getNumTutorialInfos() const { return GC.getNumTutorialInfos(); }
	int getNumEventTriggerInfos() const { return GC.getNumEventTriggerInfos(); }
	int getNumEventInfos() const { return GC.getNumEventInfos(); }
	int getNumEspionageMissionInfos() const { return GC.getNumEspionageMissionInfos(); }
	int getNumHints() const { return GC.getNumHints(); }
	int getNumMainMenus() const { return GC.getNumMainMenus(); }
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**																								**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
// Python Modular Loading
	int getNumPythonModulesInfos() const { return GC.getNumPythonModulesInfos(); }
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
	int getNumInvisibleInfos() const { return GC.getNumInvisibleInfos(); }
	int getNumVoteSourceInfos() const { return GC.getNumVoteSourceInfos(); }

	// ArtInfos
	int getNumInterfaceArtInfos() const { return ARTFILEMGR.getNumInterfaceArtInfos(); }
	int getNumMovieArtInfos() const { return ARTFILEMGR.getNumMovieArtInfos(); }
	int getNumMiscArtInfos() const { return ARTFILEMGR.getNumMiscArtInfos(); }
	int getNumUnitArtInfos() const { return ARTFILEMGR.getNumUnitArtInfos(); }
	int getNumBuildingArtInfos() const { return ARTFILEMGR.getNumBuildingArtInfos(); }
	int getNumCivilizationArtInfos() const { return ARTFILEMGR.getNumCivilizationArtInfos(); }
	int getNumLeaderheadArtInfos() const { return ARTFILEMGR.getNumLeaderheadArtInfos(); }
	int getNumImprovementArtInfos() const { return ARTFILEMGR.getNumImprovementArtInfos(); }
	int getNumBonusArtInfos() const { return ARTFILEMGR.getNumBonusArtInfos(); }
	int getNumTerrainArtInfos() const { return ARTFILEMGR.getNumTerrainArtInfos(); }
	int getNumFeatureArtInfos() const { return ARTFILEMGR.getNumFeatureArtInfos(); }
	int getNumAnimationPathInfos() const { return GC.getNumAnimationPathInfos(); }
	int getNumAnimationCategoryInfos() const { return GC.getNumAnimationCategoryInfos(); }
    int getNumUnitArtStyleTypeInfos() const { return GC.getNumUnitArtStyleTypeInfos(); }


	int getNumEntityEventTypes() const { return GC.getNumEntityEventTypes(); }
	int getNumAnimationOperatorTypes() const { return GC.getNumAnimationOperatorTypes(); }
	int getNumArtStyleTypes() const { return GC.getNumArtStyleTypes(); }
	int getNumFlavorTypes() const { return GC.getNumFlavorTypes(); }
	int getNumCitySizeTypes() const { return GC.getNumCitySizeTypes(); }
	int getNumFootstepAudioTypes() const { return GC.getNumFootstepAudioTypes(); }

	//////////////////////
	// Globals Defines
	//////////////////////

	CyVariableSystem* getCyDefinesVarSystem();
/*************************************************************************************************/
/**	TrueModular								05/26/09	Written: Mr. Genie	Imported: Xienwolf	**/
/**																								**/
/**	Properly links Modular modifications to previous elements, and allows partial overwriting	**/
/*************************************************************************************************/
	bool getDefineBOOL( const char * szName ) const { return GC.getDefineBOOL( szName ); }
/*************************************************************************************************/
/**	TrueModular								END													**/
/*************************************************************************************************/
	int getDefineINT( const char * szName ) const { return GC.getDefineINT( szName ); }
	float getDefineFLOAT( const char * szName ) const { return GC.getDefineFLOAT( szName ); }
	const char * getDefineSTRING( const char * szName ) const { return GC.getDefineSTRING( szName ); }
	void setDefineINT( const char * szName, int iValue ) { return GC.setDefineINT( szName, iValue ); }
	void setDefineFLOAT( const char * szName, float fValue ) { return GC.setDefineFLOAT( szName, fValue ); }
	void setDefineSTRING( const char * szName, const char * szValue ) { return GC.setDefineSTRING( szName, szValue ); }

	int getMOVE_DENOMINATOR() const { return GC.getMOVE_DENOMINATOR(); }
	int getNUM_UNIT_PREREQ_OR_BONUSES() const { return GC.getNUM_UNIT_PREREQ_OR_BONUSES(); }
	int getNUM_BUILDING_PREREQ_OR_BONUSES() const { return GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); }
	int getFOOD_CONSUMPTION_PER_POPULATION() const { return GC.getFOOD_CONSUMPTION_PER_POPULATION(); }
	int getMAX_HIT_POINTS() const { return GC.getMAX_HIT_POINTS(); }
	int getHILLS_EXTRA_DEFENSE() const { return GC.getHILLS_EXTRA_DEFENSE(); }
	int getRIVER_ATTACK_MODIFIER() const { return GC.getRIVER_ATTACK_MODIFIER(); }
	int getAMPHIB_ATTACK_MODIFIER() const { return GC.getAMPHIB_ATTACK_MODIFIER(); }
	int getHILLS_EXTRA_MOVEMENT() const { return GC.getHILLS_EXTRA_MOVEMENT(); }
	int getMAX_PLOT_LIST_ROWS() const { return GC.getMAX_PLOT_LIST_ROWS(); }
	int getUNIT_MULTISELECT_MAX() const { return GC.getUNIT_MULTISELECT_MAX(); }
	int getPERCENT_ANGER_DIVISOR() const { return GC.getPERCENT_ANGER_DIVISOR(); }
	int getEVENT_MESSAGE_TIME() const { return GC.getEVENT_MESSAGE_TIME(); }
	int getROUTE_FEATURE_GROWTH_MODIFIER() const { return GC.getROUTE_FEATURE_GROWTH_MODIFIER(); }
	int getFEATURE_GROWTH_MODIFIER() const { return GC.getFEATURE_GROWTH_MODIFIER(); }
	int getMIN_CITY_RANGE() const { return GC.getMIN_CITY_RANGE(); }
	int getCITY_MAX_NUM_BUILDINGS() const { return GC.getCITY_MAX_NUM_BUILDINGS(); }
	int getNUM_UNIT_AND_TECH_PREREQS() const { return GC.getNUM_UNIT_AND_TECH_PREREQS(); }
	int getNUM_AND_TECH_PREREQS() const { return GC.getNUM_AND_TECH_PREREQS(); }
	int getNUM_OR_TECH_PREREQS() const { return GC.getNUM_OR_TECH_PREREQS(); }
	int getLAKE_MAX_AREA_SIZE() const { return GC.getLAKE_MAX_AREA_SIZE(); }
	int getNUM_ROUTE_PREREQ_OR_BONUSES() const { return GC.getNUM_ROUTE_PREREQ_OR_BONUSES(); }
	int getNUM_BUILDING_AND_TECH_PREREQS() const { return GC.getNUM_BUILDING_AND_TECH_PREREQS(); }
	int getMIN_WATER_SIZE_FOR_OCEAN() const { return GC.getMIN_WATER_SIZE_FOR_OCEAN(); }
	int getFORTIFY_MODIFIER_PER_TURN() const { return GC.getFORTIFY_MODIFIER_PER_TURN(); }
	int getMAX_CITY_DEFENSE_DAMAGE() const { return GC.getMAX_CITY_DEFENSE_DAMAGE(); }
	int getNUM_CORPORATION_PREREQ_BONUSES() const { return GC.getNUM_CORPORATION_PREREQ_BONUSES(); }
	int getPEAK_SEE_THROUGH_CHANGE() const { return GC.getPEAK_SEE_THROUGH_CHANGE(); }
	int getHILLS_SEE_THROUGH_CHANGE() const { return GC.getHILLS_SEE_THROUGH_CHANGE(); }
	int getSEAWATER_SEE_FROM_CHANGE() const { return GC.getSEAWATER_SEE_FROM_CHANGE(); }
	int getPEAK_SEE_FROM_CHANGE() const { return GC.getPEAK_SEE_FROM_CHANGE(); }
	int getHILLS_SEE_FROM_CHANGE() const { return GC.getHILLS_SEE_FROM_CHANGE(); }
	int getUSE_SPIES_NO_ENTER_BORDERS() const { return GC.getUSE_SPIES_NO_ENTER_BORDERS(); }

	float getCAMERA_MIN_YAW() const { return GC.getCAMERA_MIN_YAW(); }
	float getCAMERA_MAX_YAW() const { return GC.getCAMERA_MAX_YAW(); }
	float getCAMERA_FAR_CLIP_Z_HEIGHT() const { return GC.getCAMERA_FAR_CLIP_Z_HEIGHT(); }
	float getCAMERA_MAX_TRAVEL_DISTANCE() const { return GC.getCAMERA_MAX_TRAVEL_DISTANCE(); }
	float getCAMERA_START_DISTANCE() const { return GC.getCAMERA_START_DISTANCE(); }
	float getAIR_BOMB_HEIGHT() const { return GC.getAIR_BOMB_HEIGHT(); }
	float getPLOT_SIZE() const { return GC.getPLOT_SIZE(); }
	float getCAMERA_SPECIAL_PITCH() const { return GC.getCAMERA_SPECIAL_PITCH(); }
	float getCAMERA_MAX_TURN_OFFSET() const { return GC.getCAMERA_MAX_TURN_OFFSET(); }
	float getCAMERA_MIN_DISTANCE() const { return GC.getCAMERA_MIN_DISTANCE(); }
	float getCAMERA_UPPER_PITCH() const { return GC.getCAMERA_UPPER_PITCH(); }
	float getCAMERA_LOWER_PITCH() const { return GC.getCAMERA_LOWER_PITCH(); }
	float getFIELD_OF_VIEW() const { return GC.getFIELD_OF_VIEW(); }
	float getSHADOW_SCALE() const { return GC.getSHADOW_SCALE(); }
	float getUNIT_MULTISELECT_DISTANCE() const { return GC.getUNIT_MULTISELECT_DISTANCE(); }

	int getMAX_CIV_PLAYERS() const { return GC.getMAX_CIV_PLAYERS(); }
	int getMAX_PLAYERS() const { return GC.getMAX_PLAYERS(); }
	int getMAX_CIV_TEAMS() const { return GC.getMAX_CIV_TEAMS(); }
	int getMAX_TEAMS() const { return GC.getMAX_TEAMS(); }
	int getBARBARIAN_PLAYER() const { return GC.getBARBARIAN_PLAYER(); }
	int getBARBARIAN_TEAM() const { return GC.getBARBARIAN_TEAM(); }
/*************************************************************************************************/
/**	ADDON (MultiBarb) merged Sephi		12/23/08									Xienwolf	**/
/**																								**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
	int getANIMAL_PLAYER() const		{ return GC.getANIMAL_PLAYER(); }
	int getANIMAL_TEAM() const		    { return GC.getANIMAL_TEAM(); }
	int getWILDMANA_PLAYER() const		{ return GC.getWILDMANA_PLAYER(); }
	int getWILDMANA_TEAM() const		{ return GC.getWILDMANA_TEAM(); }
	int getPIRATES_PLAYER() const		{ return GC.getPIRATES_PLAYER(); }
	int getPIRATES_TEAM() const		    { return GC.getPIRATES_TEAM(); }
	int getWHITEHAND_PLAYER() const		{ return GC.getWHITEHAND_PLAYER(); }
	int getWHITEHAND_TEAM() const		{ return GC.getWHITEHAND_TEAM(); }
	int getDEVIL_PLAYER() const		{ return GC.getDEVIL_PLAYER(); }
	int getDEVIL_TEAM() const		{ return GC.getDEVIL_TEAM(); }
	int getORC_PLAYER() const		{ return GC.getORC_PLAYER(); }
	int getORC_TEAM() const		{ return GC.getORC_TEAM(); }
	int getMERCENARIES_PLAYER() const   { return GC.getMERCENARIES_PLAYER(); }
	int getMERCENARIES_TEAM() const		{ return GC.getMERCENARIES_TEAM(); }
/**	ADDON (Adventure) Sephi                                                    					**/
	int getMAX_ADVENTURESTEPS() const		    { return GC.getMAX_ADVENTURESTEPS(); }
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/

	int getINVALID_PLOT_COORD() const { return GC.getINVALID_PLOT_COORD(); }
	int getNUM_CITY_PLOTS() const { return GC.getNUM_CITY_PLOTS(); }
	int getCITY_HOME_PLOT() const { return GC.getCITY_HOME_PLOT(); }

// lfgr UI 11/2020: Allow cycling through units in plot help
	void changePlotHelpCycleIdx( int iChange ) { PlotHelpCyclingManager::getInstance().changeCycleIdx( iChange ); } // Exposed to Python
};

#endif	// CyGlobalContext_h
