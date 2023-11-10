#include "CvGameCoreDLL.h"
#include "CvGameCoreUtils.h"
#include "CvCity.h"
#include "CvUnit.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvGameCoreUtils.h"
#include "CyArgsList.h"
#include "CvGameTextMgr.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvEventReporter.h"
#include "CvDLLWidgetData.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"
#include "CvMessageControl.h"

CvDLLWidgetData* CvDLLWidgetData::m_pInst = NULL;

CvDLLWidgetData& CvDLLWidgetData::getInstance()
{
	if (m_pInst == NULL)
	{
		m_pInst = new CvDLLWidgetData;
	}
	return *m_pInst;
}

void CvDLLWidgetData::freeInstance()
{
	delete m_pInst;
	m_pInst = NULL;
}

void CvDLLWidgetData::parseHelp(CvWStringBuffer &szBuffer, CvWidgetDataStruct &widgetDataStruct)
{
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_PLOT_LIST:
		parsePlotListHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PLOT_LIST_SHIFT:
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CTRL_SHIFT", (GC.getDefineINT("MAX_PLOT_LIST_SIZE") - 1)));
		break;

	case WIDGET_CITY_SCROLL:
		break;

	case WIDGET_LIBERATE_CITY:
		parseLiberateCityHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITY_NAME:
		parseCityNameHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_NAME:
		szBuffer.append(gDLL->getText("TXT_KEY_CHANGE_NAME"));
		break;

	case WIDGET_CREATE_GROUP:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_CREATE_GROUP"));
		break;

	case WIDGET_DELETE_GROUP:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_DELETE_GROUP"));
		break;

	case WIDGET_TRAIN:
		parseTrainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CONSTRUCT:
		parseConstructHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CREATE:
		parseCreateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_MAINTAIN:
		parseMaintainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HURRY:
		parseHurryHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_MENU_ICON:
		szBuffer.append(gDLL->getText("TXT_KEY_MAIN_MENU"));

	case WIDGET_CONSCRIPT:
		parseConscriptHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_ACTION:
		parseActionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITIZEN:
		parseCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_FREE_CITIZEN:
		parseFreeCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_DISABLED_CITIZEN:
		parseDisabledCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_ANGRY_CITIZEN:
		parseAngryCitizenHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CHANGE_SPECIALIST:
		parseChangeSpecialistHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_RESEARCH:
		parseResearchHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_TECH_TREE:
		parseTechTreeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CHANGE_PERCENT:
		parseChangePercentHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CITY_TAB:
		{
			CvWString szTemp;
            szTemp.Format(L"%s", GC.getCityTabInfo((CityTabTypes)widgetDataStruct.m_iData1).getDescription());
			szBuffer.assign(szTemp);
		}
		break;

	case WIDGET_CONTACT_CIV:
		parseContactCivHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_SCORE_BREAKDOWN:
		parseScoreHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_ZOOM_CITY:
		szBuffer.append(gDLL->getText("TXT_KEY_ZOOM_CITY_HELP"));
		break;

	case WIDGET_END_TURN:

//FfH: Modified by Kael 09/16/2008
//		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_END_TURN"));
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_END_TURN_AC", GC.getGameINLINE().getTrueGlobalCounter(), GC.getGameINLINE().getGlobalCounterLimit(), GC.getGameINLINE().getGlobalCounter()));
//FfH: End Modify

		break;

	case WIDGET_LAUNCH_VICTORY:
		szBuffer.append(gDLL->getText("TXT_KEY_WIDGET_LAUNCH_VICTORY"));
		break;

	case WIDGET_AUTOMATE_CITIZENS:
		parseAutomateCitizensHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_AUTOMATE_PRODUCTION:
		parseAutomateProductionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_EMPHASIZE:
		parseEmphasizeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_TRADE_ITEM:
		parseTradeItem(widgetDataStruct, szBuffer);
		break;

	case WIDGET_UNIT_MODEL:
		parseUnitModelHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_FLAG:
		parseFlagHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAINTENANCE:
		parseMaintenanceHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CITY_SPECIALIZATION:
		parseCitySpecializationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CIVCOUNTER_ELOHIM:
		parseCivCounterElohimHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CIVCOUNTER_SCIONS:
		parseCivCounterScionsHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MANA:
		parseManaHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FAITH:
		parseFaithHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GOLD:
		parseGoldHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GLOBAL_CULTURE:
		parseGlobalCultureHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_LUMBER:
		parseGlobalYieldHelp(widgetDataStruct, szBuffer,YIELD_LUMBER);
		break;

	case WIDGET_HELP_LEATHER:
		parseGlobalYieldHelp(widgetDataStruct, szBuffer,YIELD_LEATHER);
		break;

	case WIDGET_HELP_METAL:
		parseGlobalYieldHelp(widgetDataStruct, szBuffer,YIELD_METAL);
		break;

	case WIDGET_HELP_HERB:
		parseGlobalYieldHelp(widgetDataStruct, szBuffer,YIELD_HERB);
		break;

	case WIDGET_HELP_STONE:
		parseGlobalYieldHelp(widgetDataStruct, szBuffer,YIELD_STONE);
		break;

	case WIDGET_HELP_UNITLIMIT:
		parseUnitLimitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_RELIGION:
		parseReligionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_RELIGION_CITY:
		parseReligionHelpCity(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CORPORATION_CITY:
		parseCorporationHelpCity(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_NATIONALITY:
		parseNationalityHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_DEFENSE:
		break;

	case WIDGET_HELP_HEALTH:
		parseHealthHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_HAPPINESS:
		parseHappinessHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_POPULATION:
		parsePopulationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_PRODUCTION:
		parseProductionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CULTURE:
		parseCultureHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GREAT_PEOPLE:
		parseGreatPeopleHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GREAT_GENERAL:
		parseGreatGeneralHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_SELECTED:
		parseSelectedHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_BUILDING:
		parseBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TRADE_ROUTE_CITY:
		parseTradeRouteCityHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_ESPIONAGE_COST:
		parseEspionageCostHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TECH_ENTRY:
		parseTechEntryHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TECH_PREPREQ:
		parseTechPrereqHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OBSOLETE:
		parseObsoleteHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OBSOLETE_BONUS:
		parseObsoleteBonusString(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OBSOLETE_SPECIAL:
		parseObsoleteSpecialHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MOVE_BONUS:
		parseMoveHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FREE_UNIT:
		parseFreeUnitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FEATURE_PRODUCTION:
		parseFeatureProductionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_WORKER_RATE:
		parseWorkerRateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TRADE_ROUTES:
		parseTradeRouteHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_HEALTH_RATE:
		parseHealthRateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_HAPPINESS_RATE:
		parseHappinessRateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FREE_TECH:
		parseFreeTechHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_LOS_BONUS:
		parseLOSHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAP_CENTER:
		parseMapCenterHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAP_REVEAL:
		parseMapRevealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_MAP_TRADE:
		parseMapTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TECH_TRADE:
		parseTechTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_GOLD_TRADE:
		parseGoldTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_OPEN_BORDERS:
		parseOpenBordersHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_DEFENSIVE_PACT:
		parseDefensivePactHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_PERMANENT_ALLIANCE:
		parsePermanentAllianceHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_VASSAL_STATE:
		parseVassalStateHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_BUILD_BRIDGE:
		parseBuildBridgeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_IRRIGATION:
		parseIrrigationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_IGNORE_IRRIGATION:
		parseIgnoreIrrigationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_WATER_WORK:
		parseWaterWorkHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_IMPROVEMENT:
		parseBuildHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_DOMAIN_EXTRA_MOVES:
		parseDomainExtraMovesHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_ADJUST:
		parseAdjustHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_TERRAIN_TRADE:
		parseTerrainTradeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_SPECIAL_BUILDING:
		parseSpecialBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_YIELD_CHANGE:
		parseYieldChangeHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_BONUS_REVEAL:
		parseBonusRevealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_CIVIC_REVEAL:
		parseCivicRevealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_PROCESS_INFO:
		parseProcessInfoHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FOUND_RELIGION:
		parseFoundReligionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FOUND_CORPORATION:
		parseFoundCorporationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_NUM_UNITS:
		parseFinanceNumUnits(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_UNIT_COST:
		parseFinanceUnitCost(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_AWAY_SUPPLY:
		parseFinanceAwaySupply(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_CITY_MAINT:
		parseFinanceCityMaint(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_CIVIC_UPKEEP:
		parseFinanceCivicUpkeep(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_FOREIGN_INCOME:
		parseFinanceForeignIncome(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_INFLATED_COSTS:
		parseFinanceInflatedCosts(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_GROSS_INCOME:
		parseFinanceGrossIncome(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_NET_GOLD:
		parseFinanceNetGold(widgetDataStruct, szBuffer);
		break;

	case WIDGET_HELP_FINANCE_GOLD_RESERVE:
		parseFinanceGoldReserve(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_TECH:
	case WIDGET_CULTURE_ACHIEVEMENT:		// added Sephi
		parseTechEntryHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_REQUIRED_TECH:
		parseTechTreePrereq(widgetDataStruct, szBuffer, false);
		break;

	case WIDGET_PEDIA_JUMP_TO_DERIVED_TECH:
		parseTechTreePrereq(widgetDataStruct, szBuffer, true);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT:
	case WIDGET_DO_SUMMON:		// added Sephi
		parseUnitHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		parseBuildingHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_BACK:
		// parsePediaBack(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_FORWARD:
		// parsePediaForward(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_BONUS:
		parseBonusHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_MAIN:
		break;

	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
	case WIDGET_HELP_PROMOTION:
		parsePromotionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_CHOOSE_EVENT:
		parseEventHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT:
		parseUnitCombatHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
		parseImprovementHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIVIC:
		parseCivicHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIV:
		parseCivilizationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_LEADER:
		parseLeaderHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_SPECIALIST:
/*************************************************************************************************/
/**	FFHBUG denev																				**/
/**	ADDON (FFHBUG) merged Sephi																	**/
/**																								**/
/*************************************************************************************************/
/*
		if (widgetDataStruct.m_iData1 != NO_SPECIALIST && widgetDataStruct.m_iData2 != 0)
		{
			CvWString szTemp;
			szTemp.Format(L"%s", GC.getSpecialistInfo((SpecialistTypes)widgetDataStruct.m_iData1).getDescription());
			szBuffer.assign(szTemp);
		}
*/
		parseSpecialistHelp(widgetDataStruct, szBuffer);
/*************************************************************************************************/
/**	END																							**/
/*************************************************************************************************/

		if (widgetDataStruct.m_iData1 != NO_SPECIALIST && widgetDataStruct.m_iData2 != 0)
		{
			CvWString szTemp;
			szTemp.Format(L"%s", GC.getSpecialistInfo((SpecialistTypes)widgetDataStruct.m_iData1).getDescription());
			szBuffer.assign(szTemp);
		}
		break;

	case WIDGET_PEDIA_JUMP_TO_PROJECT:
	case WIDGET_DO_TERRAFORM:		// added Sephi
	case WIDGET_DO_GLOBAL_ENCHANTMENT:		// added Sephi
	case WIDGET_REMOVE_GLOBAL_ENCHANTMENT:		// added Sephi
		parseProjectHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_RELIGION:
		parseReligionHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
		parseCorporationHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
		parseTerrainHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_JUMP_TO_FEATURE:
		parseFeatureHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_DESCRIPTION:
		parseDescriptionHelp(widgetDataStruct, szBuffer, false);
		break;

	case WIDGET_CLOSE_SCREEN:
		//parseCloseScreenHelp(szBuffer);
		break;

	case WIDGET_DEAL_KILL:
		parseKillDealHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
		//parseDescriptionHelp(widgetDataStruct, szBuffer, true);
		break;

	case WIDGET_MINIMAP_HIGHLIGHT:
		break;

	case WIDGET_PRODUCTION_MOD_HELP:
		parseProductionModHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_LEADERHEAD:
		parseLeaderheadHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_LEADER_LINE:
		parseLeaderLineHelp(widgetDataStruct, szBuffer);
		break;

	case WIDGET_COMMERCE_MOD_HELP:
		parseCommerceModHelp(widgetDataStruct, szBuffer);
		break;

/*************************************************************************************************/
/**	FFHBUG denev																				**/
/**	ADDON (FFHBUG) merged Sephi																	**/
/**																								**/
/*************************************************************************************************/
	case WIDGET_PEDIA_JUMP_TO_TRAIT:
	case WIDGET_DO_TRAIT_FROM_CULTURE:
	case WIDGET_HELP_TRAIT:
		parseTraitHelp(widgetDataStruct, szBuffer);
		break;
/*************************************************************************************************/
/**	END																							**/
/*************************************************************************************************/

//FfH: Added by Kael 07/23/2007
	case WIDGET_PEDIA_JUMP_TO_SPELL:
	case WIDGET_HELP_SPELL:
		parseSpellHelp(widgetDataStruct, szBuffer);
		break;
//FfH: End Add

	case WIDGET_HELP_EQUIPMENT:
		parseEquipmentHelp(widgetDataStruct, szBuffer);
		break;
	}
}

// Protected Functions...
bool CvDLLWidgetData::executeAction( CvWidgetDataStruct &widgetDataStruct )
{
	bool bHandled = false;			//	Right now general bHandled = false;  We can specific case this to true later.  Game will run with this = false;

	switch (widgetDataStruct.m_eWidgetType)
	{

	case WIDGET_PLOT_LIST:
		doPlotList(widgetDataStruct);
		break;

	case WIDGET_PLOT_LIST_SHIFT:
		gDLL->getInterfaceIFace()->changePlotListColumn(widgetDataStruct.m_iData1 * ((gDLL->ctrlKey()) ? (GC.getDefineINT("MAX_PLOT_LIST_SIZE") - 1) : 1));
		break;

	case WIDGET_CITY_SCROLL:
		if ( widgetDataStruct.m_iData1 > 0 )
		{
			GC.getGameINLINE().doControl(CONTROL_NEXTCITY);
		}
		else
		{
			GC.getGameINLINE().doControl(CONTROL_PREVCITY);
		}
		break;

	case WIDGET_LIBERATE_CITY:
		doLiberateCity();
		break;

	case WIDGET_CITY_NAME:
		doRenameCity();
		break;

	case WIDGET_UNIT_NAME:
		doRenameUnit();
		break;

	case WIDGET_CREATE_GROUP:
		doCreateGroup();
		break;

	case WIDGET_DELETE_GROUP:
		doDeleteGroup();
		break;

	case WIDGET_TRAIN:
		doTrain(widgetDataStruct);
		break;

	case WIDGET_CONSTRUCT:
		doConstruct(widgetDataStruct);
		break;

	case WIDGET_CREATE:
		doCreate(widgetDataStruct);
		break;

	case WIDGET_MAINTAIN:
		doMaintain(widgetDataStruct);
		break;

	case WIDGET_HURRY:
		doHurry(widgetDataStruct);
		break;

	case WIDGET_MENU_ICON:
		doMenu();

	case WIDGET_CONSCRIPT:
		doConscript();
		break;

	case WIDGET_ACTION:
		doAction(widgetDataStruct);
		break;

	case WIDGET_CITIZEN:
		break;

	case WIDGET_FREE_CITIZEN:
		break;

	case WIDGET_DISABLED_CITIZEN:
		break;

	case WIDGET_ANGRY_CITIZEN:
		break;

	case WIDGET_CHANGE_SPECIALIST:
		doChangeSpecialist(widgetDataStruct);
		break;

	case WIDGET_RESEARCH:
	case WIDGET_TECH_TREE:
		doResearch(widgetDataStruct);
		break;

	case WIDGET_CHANGE_PERCENT:
		doChangePercent(widgetDataStruct);
		break;

	case WIDGET_CITY_TAB:
		doCityTab(widgetDataStruct);
		break;

	case WIDGET_CONTACT_CIV:
		doContactCiv(widgetDataStruct);
		break;

	case WIDGET_END_TURN:
		GC.getGameINLINE().doControl(CONTROL_FORCEENDTURN);
		break;

	case WIDGET_LAUNCH_VICTORY:
		doLaunch(widgetDataStruct);
		break;

	case WIDGET_CONVERT:
		doConvert(widgetDataStruct);
		break;

	case WIDGET_REVOLUTION:
		// handled in Python
		break;

	case WIDGET_AUTOMATE_CITIZENS:
		doAutomateCitizens();
		break;

	case WIDGET_AUTOMATE_PRODUCTION:
		doAutomateProduction();
		break;

	case WIDGET_EMPHASIZE:
		doEmphasize(widgetDataStruct);
		break;

	case WIDGET_DIPLOMACY_RESPONSE:
		// CLEANUP -- PD
//		GC.getDiplomacyScreen().handleClick(m_pWidget);
		break;

	case WIDGET_TRADE_ITEM:
		break;

	case WIDGET_UNIT_MODEL:
		doUnitModel();
		break;

	case WIDGET_FLAG:
		doFlag();
		break;

	case WIDGET_HELP_SELECTED:
		doSelected(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_UNIT:
		doPediaUnitJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		doPediaBuildingJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_TECH:
	case WIDGET_PEDIA_JUMP_TO_REQUIRED_TECH:
	case WIDGET_PEDIA_JUMP_TO_DERIVED_TECH:
		doPediaTechJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_BACK:
		doPediaBack();
		break;
	case WIDGET_PEDIA_FORWARD:
		doPediaForward();
		break;

	case WIDGET_PEDIA_JUMP_TO_BONUS:
		doPediaBonusJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_MAIN:
		doPediaMain(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
		doPediaPromotionJump(widgetDataStruct);
		break;

//FfH: Added by Kael 07/23/2007
	case WIDGET_PEDIA_JUMP_TO_SPELL:
		doPediaSpellJump(widgetDataStruct);
		break;
//FfH: End Add

/*************************************************************************************************/
/**	FFHBUG denev																				**/
/**	ADDON (FFHBUG) merged Sephi																	**/
/**																								**/
/*************************************************************************************************/
	case WIDGET_PEDIA_JUMP_TO_TRAIT:
		doPediaTraitJump(widgetDataStruct);
		break;
/*************************************************************************************************/
/**	END																							**/
/*************************************************************************************************/

	case WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT:
		doPediaUnitCombatJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
		doPediaImprovementJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIVIC:
		doPediaCivicJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CIV:
		doPediaCivilizationJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_LEADER:
		doPediaLeaderJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_SPECIALIST:
		doPediaSpecialistJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_PROJECT:
		doPediaProjectJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_RELIGION:
		doPediaReligionJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
		doPediaCorporationJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
		doPediaTerrainJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_JUMP_TO_FEATURE:
		doPediaFeatureJump(widgetDataStruct);
		break;

	case WIDGET_PEDIA_DESCRIPTION:
	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
		doPediaDescription(widgetDataStruct);
		break;

	case WIDGET_TURN_EVENT:
		doGotoTurnEvent(widgetDataStruct);
		break;

	case WIDGET_FOREIGN_ADVISOR:
		doForeignAdvisor(widgetDataStruct);
		break;

/*************************************************************************************************/
/**	ADDON(Screen) Sephi					                                                        **/
/*************************************************************************************************/
	case WIDGET_TECH_SCREEN:
		doResearchScreen(widgetDataStruct);
		break;
	case WIDGET_SPELL_RESEARCH:
		doSpellResearchScreen(widgetDataStruct);
		break;
	case WIDGET_GUILD_SCREEN:
		doSpellGuildScreen(widgetDataStruct);
		break;
	case WIDGET_MAGIC_SCREEN:
		doMagicScreen(widgetDataStruct);
		break;
	case WIDGET_DO_TRAIT_FROM_CULTURE:
		doTraitFromCulture(widgetDataStruct);
		break;
	case WIDGET_DO_SUMMON:
		doSummon(widgetDataStruct);
		break;
	case WIDGET_DO_TERRAFORM:
		doTerraform(widgetDataStruct);
		break;
	case WIDGET_DO_GLOBAL_ENCHANTMENT:
		doGlobalenchantment(widgetDataStruct);
		break;
	case WIDGET_REMOVE_GLOBAL_ENCHANTMENT:
		RemoveGlobalenchantment(widgetDataStruct);
		break;
	case WIDGET_CULTURE_ACHIEVEMENT:
		doCultureAchievement(widgetDataStruct);
		break;
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/

	case WIDGET_DEAL_KILL:
		doDealKill(widgetDataStruct);
		break;

	case WIDGET_MINIMAP_HIGHLIGHT:
		doRefreshMilitaryAdvisor(widgetDataStruct);
		break;

	case WIDGET_CHOOSE_EVENT:
		break;

	case WIDGET_ZOOM_CITY:
		break;

	case WIDGET_HELP_TECH_PREPREQ:
	case WIDGET_HELP_OBSOLETE:
	case WIDGET_HELP_OBSOLETE_BONUS:
	case WIDGET_HELP_OBSOLETE_SPECIAL:
	case WIDGET_HELP_MOVE_BONUS:
	case WIDGET_HELP_FREE_UNIT:
	case WIDGET_HELP_FEATURE_PRODUCTION:
	case WIDGET_HELP_WORKER_RATE:
	case WIDGET_HELP_TRADE_ROUTES:
	case WIDGET_HELP_HEALTH_RATE:
	case WIDGET_HELP_HAPPINESS_RATE:
	case WIDGET_HELP_FREE_TECH:
	case WIDGET_HELP_LOS_BONUS:
	case WIDGET_HELP_MAP_CENTER:
	case WIDGET_HELP_MAP_REVEAL:
	case WIDGET_HELP_MAP_TRADE:
	case WIDGET_HELP_TECH_TRADE:
	case WIDGET_HELP_GOLD_TRADE:
	case WIDGET_HELP_OPEN_BORDERS:
	case WIDGET_HELP_DEFENSIVE_PACT:
	case WIDGET_HELP_PERMANENT_ALLIANCE:
	case WIDGET_HELP_VASSAL_STATE:
	case WIDGET_HELP_BUILD_BRIDGE:
	case WIDGET_HELP_IRRIGATION:
	case WIDGET_HELP_IGNORE_IRRIGATION:
	case WIDGET_HELP_WATER_WORK:
	case WIDGET_HELP_IMPROVEMENT:
	case WIDGET_HELP_DOMAIN_EXTRA_MOVES:
	case WIDGET_HELP_ADJUST:
	case WIDGET_HELP_TERRAIN_TRADE:
	case WIDGET_HELP_SPECIAL_BUILDING:
	case WIDGET_HELP_YIELD_CHANGE:
	case WIDGET_HELP_BONUS_REVEAL:
	case WIDGET_HELP_CIVIC_REVEAL:
	case WIDGET_HELP_PROCESS_INFO:
	case WIDGET_HELP_FINANCE_NUM_UNITS:
	case WIDGET_HELP_FINANCE_UNIT_COST:
	case WIDGET_HELP_FINANCE_AWAY_SUPPLY:
	case WIDGET_HELP_FINANCE_CITY_MAINT:
	case WIDGET_HELP_FINANCE_CIVIC_UPKEEP:
	case WIDGET_HELP_FINANCE_FOREIGN_INCOME:
	case WIDGET_HELP_FINANCE_INFLATED_COSTS:
	case WIDGET_HELP_FINANCE_GROSS_INCOME:
	case WIDGET_HELP_FINANCE_NET_GOLD:
	case WIDGET_HELP_FINANCE_GOLD_RESERVE:
	case WIDGET_HELP_RELIGION_CITY:
	case WIDGET_HELP_CORPORATION_CITY:
	case WIDGET_HELP_PROMOTION:
	case WIDGET_LEADERHEAD:
	case WIDGET_LEADER_LINE:
	case WIDGET_CLOSE_SCREEN:
	case WIDGET_SCORE_BREAKDOWN:
		//	Nothing on clicked

/*************************************************************************************************/
/**	FFHBUG denev																				**/
/**	ADDON (FFHBUG) merged Sephi																	**/
/**																								**/
/*************************************************************************************************/
	case WIDGET_HELP_TRAIT:
/*************************************************************************************************/
/**	END																							**/
/*************************************************************************************************/

//FfH: Added by Kael 07/23/2007
	case WIDGET_HELP_SPELL:
//FfH: End Add

		break;
	}

	return bHandled;
}

//	right clicking action
bool CvDLLWidgetData::executeAltAction( CvWidgetDataStruct &widgetDataStruct )
{
	CvWidgetDataStruct widgetData = widgetDataStruct;

	bool bHandled = true;
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_HELP_TECH_ENTRY:
	case WIDGET_HELP_TECH_PREPREQ:
	case WIDGET_RESEARCH:
	case WIDGET_TECH_TREE:
		doPediaTechJump(widgetDataStruct);
		break;
	case WIDGET_TRAIN:
		doPediaTrainJump(widgetDataStruct);
		break;
	case WIDGET_CONSTRUCT:
		doPediaConstructJump(widgetDataStruct);
		break;
	case WIDGET_CREATE:
		doPediaProjectJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_UNIT:
	case WIDGET_HELP_FREE_UNIT:
		doPediaUnitJump(widgetDataStruct);
		break;
	case WIDGET_HELP_FOUND_RELIGION:
		widgetData.m_iData1 = widgetData.m_iData2;
		//	Intentional fallthrough...
	case WIDGET_PEDIA_JUMP_TO_RELIGION:
		doPediaReligionJump(widgetData);
		break;
	case WIDGET_HELP_FOUND_CORPORATION:
		widgetData.m_iData1 = widgetData.m_iData2;
		//	Intentional fallthrough...
	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
		doPediaCorporationJump(widgetData);
		break;
	case WIDGET_PEDIA_JUMP_TO_BUILDING:
		doPediaBuildingJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_PROMOTION:
		doPediaPromotionJump(widgetDataStruct);
		break;
	case WIDGET_HELP_OBSOLETE:
		doPediaBuildingJump(widgetDataStruct);
		break;
	case WIDGET_HELP_IMPROVEMENT:
		doPediaBuildJump(widgetDataStruct);
		break;
	case WIDGET_HELP_YIELD_CHANGE:
		doPediaImprovementJump(widgetDataStruct, true);
		break;
	case WIDGET_HELP_BONUS_REVEAL:
	case WIDGET_HELP_OBSOLETE_BONUS:
		doPediaBonusJump(widgetDataStruct, true);
		break;
	case WIDGET_CITIZEN:
	case WIDGET_FREE_CITIZEN:
	case WIDGET_DISABLED_CITIZEN:
		doPediaSpecialistJump(widgetDataStruct);
		break;
	case WIDGET_PEDIA_JUMP_TO_PROJECT:
		doPediaProjectJump(widgetDataStruct);
		break;
	case WIDGET_HELP_CIVIC_REVEAL:
		widgetData.m_iData1 = widgetData.m_iData2;
		doPediaCivicJump(widgetData);
		break;
	case WIDGET_LEADERHEAD:
		doContactCiv(widgetDataStruct);
		break;

/*************************************************************************************************/
/**	FFHBUG denev																				**/
/**	ADDON (FFHBUG) merged Sephi																	**/
/**																								**/
/*************************************************************************************************/
	case WIDGET_PEDIA_JUMP_TO_TRAIT:
		doPediaTraitJump(widgetDataStruct);
		break;
/*************************************************************************************************/
/**	END																							**/
/*************************************************************************************************/

//FfH: Added by Kael 07/23/2007
	case WIDGET_PEDIA_JUMP_TO_SPELL:
		doPediaSpellJump(widgetDataStruct);
		break;
//FfH: End Add

	default:
		bHandled = false;
		break;
	}

	return (bHandled);
}

bool CvDLLWidgetData::isLink(const CvWidgetDataStruct &widgetDataStruct) const
{
	bool bLink = false;
	switch (widgetDataStruct.m_eWidgetType)
	{
	case WIDGET_PEDIA_JUMP_TO_TECH:
	case WIDGET_PEDIA_JUMP_TO_REQUIRED_TECH:
	case WIDGET_PEDIA_JUMP_TO_DERIVED_TECH:
	case WIDGET_PEDIA_JUMP_TO_BUILDING:
	case WIDGET_PEDIA_JUMP_TO_UNIT:
	case WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT:
	case WIDGET_PEDIA_JUMP_TO_PROMOTION:

//FfH: Added by Kael 07/23/2007
	case WIDGET_PEDIA_JUMP_TO_SPELL:
//FfH: End Add

//>>>>BUGFfH: Added by Denev 2009/09/10
	case WIDGET_PEDIA_JUMP_TO_TRAIT:
//<<<<BUGFfH: End Add

	case WIDGET_PEDIA_JUMP_TO_BONUS:
	case WIDGET_PEDIA_JUMP_TO_IMPROVEMENT:
	case WIDGET_PEDIA_JUMP_TO_CIVIC:
	case WIDGET_PEDIA_JUMP_TO_CIV:
	case WIDGET_PEDIA_JUMP_TO_LEADER:
	case WIDGET_PEDIA_JUMP_TO_SPECIALIST:
	case WIDGET_PEDIA_JUMP_TO_PROJECT:
	case WIDGET_PEDIA_JUMP_TO_RELIGION:
	case WIDGET_PEDIA_JUMP_TO_CORPORATION:
	case WIDGET_PEDIA_JUMP_TO_TERRAIN:
	case WIDGET_PEDIA_JUMP_TO_FEATURE:
	case WIDGET_PEDIA_FORWARD:
	case WIDGET_PEDIA_BACK:
	case WIDGET_PEDIA_MAIN:
	case WIDGET_TURN_EVENT:
	case WIDGET_FOREIGN_ADVISOR:
	case WIDGET_PEDIA_DESCRIPTION:
	case WIDGET_PEDIA_DESCRIPTION_NO_HELP:
/*************************************************************************************************/
/**	ADDON(Screen) Sephi					                                                        **/
/*************************************************************************************************/
	case WIDGET_SPELL_RESEARCH:
	case WIDGET_GUILD_SCREEN:
	case WIDGET_MAGIC_SCREEN:
	case WIDGET_DO_TRAIT_FROM_CULTURE:
	case WIDGET_DO_SUMMON:
	case WIDGET_DO_TERRAFORM:
	case WIDGET_DO_GLOBAL_ENCHANTMENT:
	case WIDGET_REMOVE_GLOBAL_ENCHANTMENT:
	case WIDGET_CULTURE_ACHIEVEMENT:
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/
	case WIDGET_MINIMAP_HIGHLIGHT:
		bLink = (widgetDataStruct.m_iData1 >= 0);
		break;
	case WIDGET_DEAL_KILL:
		{
			CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
			bLink = (NULL != pDeal && pDeal->isCancelable(GC.getGameINLINE().getActivePlayer()));
		}
		break;
	case WIDGET_CONVERT:
		bLink = (0 != widgetDataStruct.m_iData2);
		break;
	case WIDGET_GENERAL:
	case WIDGET_REVOLUTION:
		bLink = (1 == widgetDataStruct.m_iData1);
		break;
	}
	return (bLink);
}


void CvDLLWidgetData::doPlotList(CvWidgetDataStruct &widgetDataStruct)
{
	PROFILE_FUNC();

	CvUnit* pUnit;
	bool bWasCityScreenUp;

	int iUnitIndex = widgetDataStruct.m_iData1 + gDLL->getInterfaceIFace()->getPlotListColumn() - gDLL->getInterfaceIFace()->getPlotListOffset();

	CvPlot *selectionPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
	pUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(selectionPlot, iUnitIndex);

	if (pUnit != NULL)
	{
		if (pUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			bWasCityScreenUp = gDLL->getInterfaceIFace()->isCityScreenUp();

			gDLL->getInterfaceIFace()->selectGroup(pUnit, gDLL->shiftKey(), gDLL->ctrlKey(), gDLL->altKey());

			if (bWasCityScreenUp)
			{
				gDLL->getInterfaceIFace()->lookAtSelectionPlot();
			}
		}
	}
}


void CvDLLWidgetData::doLiberateCity()
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_LIBERATE, 0);

	gDLL->getInterfaceIFace()->clearSelectedCities();
}


void CvDLLWidgetData::doRenameCity()
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			CvEventReporter::getInstance().cityRename(pHeadSelectedCity);
		}
	}
}


void CvDLLWidgetData::doRenameUnit()
{
	CvUnit* pHeadSelectedUnit;

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (pHeadSelectedUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			CvEventReporter::getInstance().unitRename(pHeadSelectedUnit);
		}
	}
}


void CvDLLWidgetData::doCreateGroup()
{
	GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP);
}


void CvDLLWidgetData::doDeleteGroup()
{
	GC.getGameINLINE().selectionListGameNetMessage(GAMEMESSAGE_JOIN_GROUP, -1, -1, -1, 0, false, true);
}


void CvDLLWidgetData::doTrain(CvWidgetDataStruct &widgetDataStruct)
{
	UnitTypes eUnit;

//FfH: Modified by Kael 07/02/2008
//	eUnit = ((UnitTypes)(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1)));
//	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
//	{
//		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_TRAIN, eUnit, false, false, false);
//	}
//	else
//	{
//		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_TRAIN, eUnit, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
//	}
	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		eUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2)->getCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1)));
		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_TRAIN, eUnit, false, false, false);
	}
	else
	{
		eUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(gDLL->getInterfaceIFace()->headSelectedCitiesNode()->m_data.iID)->getCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1)));
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_TRAIN, eUnit, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}
//FfH: End Modify

	gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_UNITS);
}


void CvDLLWidgetData::doConstruct(CvWidgetDataStruct &widgetDataStruct)
{
	BuildingTypes eBuilding;

//FfH: Modified by Kael 07/02/2008
//	eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1)));
//	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
//	{
//		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_CONSTRUCT, eBuilding, false, false, false);
//	}
//	else
//	{
//		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CONSTRUCT, eBuilding, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
//	}
	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2)->getCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1)));
		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_CONSTRUCT, eBuilding, false, false, false);
	}
	else
	{
		eBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(gDLL->getInterfaceIFace()->headSelectedCitiesNode()->m_data.iID)->getCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1)));
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CONSTRUCT, eBuilding, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}
//FfH: End Modify

	if (isLimitedWonderClass((BuildingClassTypes)(widgetDataStruct.m_iData1)))
	{
		gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_WONDERS);
	}
	else
	{
		gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_BUILDINGS);
	}
}


void CvDLLWidgetData::doCreate(CvWidgetDataStruct &widgetDataStruct)
{
	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_CREATE, widgetDataStruct.m_iData1, false, false, false);
	}
	else
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_CREATE, widgetDataStruct.m_iData1, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}

	gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_WONDERS);
}


void CvDLLWidgetData::doMaintain(CvWidgetDataStruct &widgetDataStruct)
{
	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		CvMessageControl::getInstance().sendPushOrder(widgetDataStruct.m_iData2, ORDER_MAINTAIN, widgetDataStruct.m_iData1, false, false, false);
	}
	else
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_PUSH_ORDER, ORDER_MAINTAIN, widgetDataStruct.m_iData1, -1, false, gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}

	gDLL->getInterfaceIFace()->setCityTabSelectionRow(CITYTAB_WONDERS);
}


void CvDLLWidgetData::doHurry(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_HURRY, widgetDataStruct.m_iData1);
}


void CvDLLWidgetData::doConscript()
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_CONSCRIPT);
}


void CvDLLWidgetData::doAction(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().handleAction(widgetDataStruct.m_iData1);
}


void CvDLLWidgetData::doChangeSpecialist(CvWidgetDataStruct &widgetDataStruct)
{
	GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_CHANGE_SPECIALIST, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
}


void CvDLLWidgetData::doResearch(CvWidgetDataStruct &widgetDataStruct)
{
	bool bShift;

	bShift = gDLL->shiftKey();

	if (!bShift)
	{
		if ((GetKeyState(VK_LSHIFT) & 0x8000) || (GetKeyState(VK_RSHIFT) & 0x8000))
		{
			bShift = true;
		}
	}

	CvMessageControl::getInstance().sendResearch(((TechTypes)widgetDataStruct.m_iData1), widgetDataStruct.m_iData2, bShift);
}


void CvDLLWidgetData::doChangePercent(CvWidgetDataStruct &widgetDataStruct)
{
	CvMessageControl::getInstance().sendPercentChange(((CommerceTypes)widgetDataStruct.m_iData1), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::doCityTab(CvWidgetDataStruct &widgetDataStruct)
{
	gDLL->getInterfaceIFace()->setCityTabSelectionRow((CityTabTypes)widgetDataStruct.m_iData1);
}

void CvDLLWidgetData::doContactCiv(CvWidgetDataStruct &widgetDataStruct)
{
	if (gDLL->isDiplomacy() || gDLL->isMPDiplomacyScreenUp())
	{
		return;
	}

	//	Do not execute this if we are trying to contact ourselves...
	if (GC.getGameINLINE().getActivePlayer() == widgetDataStruct.m_iData1)
	{
		if (!gDLL->getInterfaceIFace()->isFocusedWidget())
		{
			gDLL->getInterfaceIFace()->toggleScoresMinimized();
		}

		return;
	}

	if (gDLL->shiftKey())
	{
		if (GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).isHuman())
		{
			if (widgetDataStruct.m_iData1 != GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->showTurnLog((ChatTargetTypes)widgetDataStruct.m_iData1);
			}
		}
		return;
	}

	if (gDLL->altKey())
	{
		if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()))
		{
			CvMessageControl::getInstance().sendChangeWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam(), true);
		}
		else if (GET_TEAM(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()).isVassal(GC.getGameINLINE().getActiveTeam()))
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_VASSAL_DEMAND_TRIBUTE, widgetDataStruct.m_iData1);
			if (pInfo)
			{
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		return;
	}

	GET_PLAYER(GC.getGameINLINE().getActivePlayer()).contact((PlayerTypes)widgetDataStruct.m_iData1);
}

void CvDLLWidgetData::doConvert(CvWidgetDataStruct &widgetDataStruct)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		CvMessageControl::getInstance().sendConvert((ReligionTypes)(widgetDataStruct.m_iData1));
	}
}

void CvDLLWidgetData::doAutomateCitizens()
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_AUTOMATED_CITIZENS, -1, -1, !(pHeadSelectedCity->isCitizensAutomated()));
	}
}

void CvDLLWidgetData::doAutomateProduction()
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_AUTOMATED_PRODUCTION, -1, -1, !pHeadSelectedCity->isProductionAutomated(), gDLL->altKey(), gDLL->shiftKey(), gDLL->ctrlKey());
	}
}

void CvDLLWidgetData::doEmphasize(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_DO_TASK, TASK_SET_EMPHASIZE, widgetDataStruct.m_iData1, -1, !(pHeadSelectedCity->AI_isEmphasize((EmphasizeTypes)(widgetDataStruct.m_iData1))));
	}
}

void CvDLLWidgetData::doUnitModel()
{
	if (gDLL->getInterfaceIFace()->isFocused())
	{
		//	Do NOT execute if a screen is up...
		return;
	}

	gDLL->getInterfaceIFace()->lookAtSelectionPlot();
}


void CvDLLWidgetData::doFlag()
{
	GC.getGameINLINE().doControl(CONTROL_SELECTCAPITAL);
}

void CvDLLWidgetData::doSelected(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GC.getGameINLINE().selectedCitiesGameNetMessage(GAMEMESSAGE_POP_ORDER, widgetDataStruct.m_iData1);
	}
}


void CvDLLWidgetData::doPediaTechJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToTech", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaUnitJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;

	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToUnit", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaBuildingJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToBuilding", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaProjectJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToProject", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaReligionJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToReligion", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCorporationJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToCorporation", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaTerrainJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToTerrain", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaFeatureJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToFeature", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaTrainJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1));

	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToUnit", argsList.makeFunctionArgs());
}


void CvDLLWidgetData::doPediaConstructJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1));

	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToBuilding", argsList.makeFunctionArgs());
}


void CvDLLWidgetData::doPediaBack()
{
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaBack");
}

void CvDLLWidgetData::doPediaForward()
{
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaForward");
}

void CvDLLWidgetData::doPediaBonusJump(CvWidgetDataStruct &widgetDataStruct, bool bData2)
{
	CyArgsList argsList;
	if (bData2)
	{
		argsList.add(widgetDataStruct.m_iData2);
	}
	else
	{
		argsList.add(widgetDataStruct.m_iData1);
	}
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToBonus", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaSpecialistJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToSpecialist", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaMain(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1 < 0 ? 0 : widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaMain", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaPromotionJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToPromotion", argsList.makeFunctionArgs());
}

//FfH: Added by Kael 07/23/2007
void CvDLLWidgetData::doPediaSpellJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToSpell", argsList.makeFunctionArgs());
}
//FfH: End Add

void CvDLLWidgetData::doPediaUnitCombatJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToUnitChart", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaImprovementJump(CvWidgetDataStruct &widgetDataStruct, bool bData2)
{
	CyArgsList argsList;
	if (bData2)
	{
		argsList.add(widgetDataStruct.m_iData2);
	}
	else
	{
		argsList.add(widgetDataStruct.m_iData1);
	}
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToImprovement", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCivicJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToCivic", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaCivilizationJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToCiv", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaLeaderJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToLeader", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaDescription(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	argsList.add(widgetDataStruct.m_iData2);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaShowHistorical", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doPediaBuildJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;

	ImprovementTypes eImprovement = NO_IMPROVEMENT;
	BuildTypes eBuild = (BuildTypes)widgetDataStruct.m_iData2;
	if (NO_BUILD != eBuild)
	{
		eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();
	}

	if (NO_IMPROVEMENT != eImprovement)
	{
		argsList.add(eImprovement);
		gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToImprovement", argsList.makeFunctionArgs());
	}
}

void CvDLLWidgetData::doGotoTurnEvent(CvWidgetDataStruct &widgetDataStruct)
{
	CvPlot* pPlot = GC.getMapINLINE().plotINLINE(widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);

	if (NULL != pPlot && !gDLL->getEngineIFace()->isCameraLocked())
	{
		if (pPlot->isRevealed(GC.getGameINLINE().getActiveTeam(), false))
		{
			gDLL->getEngineIFace()->cameraLookAt(pPlot->getPoint());
		}
	}
}

void CvDLLWidgetData::doMenu( void )
{
	if (!gDLL->isGameInitializing())
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_MAIN_MENU);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo, NO_PLAYER, true);
		}
	}
}

void CvDLLWidgetData::doLaunch(CvWidgetDataStruct &widgetDataStruct)
{
	if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canLaunch((VictoryTypes)widgetDataStruct.m_iData1) && GC.getGameINLINE().testVictory((VictoryTypes)widgetDataStruct.m_iData1, GC.getGameINLINE().getActiveTeam()))
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_LAUNCH, widgetDataStruct.m_iData1);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
	}
}

void CvDLLWidgetData::doForeignAdvisor(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "showForeignAdvisorScreen", argsList.makeFunctionArgs());
}

/*************************************************************************************************/
/**	ADDON(Screen) Sephi					                                                        **/
/*************************************************************************************************/
void CvDLLWidgetData::doResearchScreen(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList,argsList1;
	argsList.add(widgetDataStruct.m_iData1);
	argsList1.add(true);
//	gDLL->getPythonIFace()->callFunction("CvTechChooser", "updateTechRecords", argsList1.makeFunctionArgs());	
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "showTechChooser", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doSpellResearchScreen(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "showArcaneScreen", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doSpellGuildScreen(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "showGuildScreen", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doMagicScreen(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "showMagicScreen", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::doTerraform(CvWidgetDataStruct &widgetDataStruct)
{
	if(widgetDataStruct.m_iData1!=NO_PROJECT && !GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canDoTerraformRitual((ProjectTypes)widgetDataStruct.m_iData1))
	{
		return;
	}

	CyArgsList argsList;
	argsList.add(0);
	gDLL->getPythonIFace()->callFunction(PYGameModule, "Trigger_MagicScreenPopup", argsList.makeFunctionArgs());
	if(widgetDataStruct.m_iData1!=-1)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_TERRAFORM, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
	}
}

void CvDLLWidgetData::doTraitFromCulture(CvWidgetDataStruct &widgetDataStruct)
{
	TraitTypes eTrait = (TraitTypes)widgetDataStruct.m_iData1;
	CvPlayer& kPlayer = GET_PLAYER(GC.getGame().getActivePlayer());

	if(eTrait == NO_TRAIT || 
		!kPlayer.canGetTrait(eTrait) ||
		kPlayer.getGlobalCulture() < kPlayer.getTraitCost(eTrait))
		return;

	CyArgsList argsList;
	argsList.add(0);
	gDLL->getPythonIFace()->callFunction(PYGameModule, "Trigger_GuildScreenPopup", argsList.makeFunctionArgs());

	CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_NEW_TRAIT, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
	if (NULL != pInfo)
	{
		gDLL->getInterfaceIFace()->addPopup(pInfo);
	}
}

void CvDLLWidgetData::doSummon(CvWidgetDataStruct &widgetDataStruct)
{
	ProjectTypes eProject=NO_PROJECT;
	for(int iI=0;iI<GC.getNumProjectInfos();iI++)
	{
		if(GC.getProjectInfo((ProjectTypes)iI).getSummonUnitType()==widgetDataStruct.m_iData1)
		{
			eProject=(ProjectTypes)iI;
			break;
		}
	}

	if(eProject == NO_PROJECT || !GET_PLAYER((PlayerTypes)GC.getGame().getActivePlayer()).canCreateSummon(eProject))
	{
		return;
	}
	CyArgsList argsList;
	argsList.add(0);
	gDLL->getPythonIFace()->callFunction(PYGameModule, "Trigger_MagicScreenPopup", argsList.makeFunctionArgs());
	if(widgetDataStruct.m_iData1!=-1)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_SUMMON, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
	}
}

void CvDLLWidgetData::doGlobalenchantment(CvWidgetDataStruct &widgetDataStruct)
{
	if(widgetDataStruct.m_iData1!=NO_PROJECT && !GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canDoGlobalEnchantment((ProjectTypes)widgetDataStruct.m_iData1))
	{
		return;
	}
		
	CyArgsList argsList;
	argsList.add(0);
	gDLL->getPythonIFace()->callFunction(PYGameModule, "Trigger_MagicScreenPopup", argsList.makeFunctionArgs());
	if(widgetDataStruct.m_iData1!=-1)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_GLOBALENCHANTMENT, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
	}
}
void CvDLLWidgetData::RemoveGlobalenchantment(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(0);
	gDLL->getPythonIFace()->callFunction(PYGameModule, "Trigger_MagicScreenPopup", argsList.makeFunctionArgs());
	if(widgetDataStruct.m_iData1!=-1)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_GLOBALENCHANTMENT_CANCEL, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
	}
}

void CvDLLWidgetData::doCityInfrastructureIncrease(CvWidgetDataStruct &widgetDataStruct)
{
	CvCity* pCity=gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if(!GET_PLAYER((PlayerTypes)GC.getGame().getActivePlayer()).canIncreaseCityInfrastructureLevel(pCity))
	{
		return;
	}

	CvMessageControl::getInstance().sendDoTask(pCity->getID(),TASK_CITY_INFRASTRUCTURE_INCREASE,-1,-1,false,false,false,false);
	/**
	CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CITY_INFRASTRUCTURE_INCREASE, pCity->getID());
	if (NULL != pInfo)
	{
		gDLL->getInterfaceIFace()->addPopup(pInfo);
	}
	**/
}

void CvDLLWidgetData::doCultureAchievement(CvWidgetDataStruct &widgetDataStruct)
{
	TechTypes eTech=(TechTypes)widgetDataStruct.m_iData1;

	if(eTech == NO_TECH || !GET_PLAYER((PlayerTypes)GC.getGame().getActivePlayer()).canGetCulturalAchievement(eTech))
	{
		return;
	}

	CyArgsList argsList;
	argsList.add(0);
	gDLL->getPythonIFace()->callFunction(PYGameModule, "Trigger_GuildScreenPopup", argsList.makeFunctionArgs());

	if(widgetDataStruct.m_iData1!=-1)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_CULTURAL_ACHIEVEMENT, widgetDataStruct.m_iData1, widgetDataStruct.m_iData2);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo);
		}
	}
}

/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/

//
//	HELP PARSING FUNCTIONS
//

void CvDLLWidgetData::parsePlotListHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	PROFILE_FUNC();

	CvUnit* pUnit;

	int iUnitIndex = widgetDataStruct.m_iData1 + gDLL->getInterfaceIFace()->getPlotListColumn() - gDLL->getInterfaceIFace()->getPlotListOffset();

	CvPlot *selectionPlot = gDLL->getInterfaceIFace()->getSelectionPlot();
	pUnit = gDLL->getInterfaceIFace()->getInterfacePlotUnit(selectionPlot, iUnitIndex);

	if (pUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pUnit);

		if (pUnit->plot()->plotCount(PUF_isUnitType, pUnit->getUnitType(), -1, pUnit->getOwnerINLINE()) > 1)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CTRL_SELECT", GC.getUnitInfo(pUnit->getUnitType()).getTextKeyWide()));
		}

		if (pUnit->plot()->plotCount(NULL, -1, -1, pUnit->getOwnerINLINE()) > 1)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALT_SELECT"));
		}
	}
}


void CvDLLWidgetData::parseLiberateCityHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		PlayerTypes ePlayer = pHeadSelectedCity->getLiberationPlayer(false);
		if (NO_PLAYER != ePlayer)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_LIBERATE_CITY_HELP", pHeadSelectedCity->getNameKey(), GET_PLAYER(ePlayer).getNameKey()));
		}
	}
}

void CvDLLWidgetData::parseCityNameHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.append(pHeadSelectedCity->getName());

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CITY_POPULATION", pHeadSelectedCity->getRealPopulation()));

		GAMETEXT.setTimeStr(szTempBuffer, pHeadSelectedCity->getGameTurnFounded(), false);
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CITY_FOUNDED", szTempBuffer.GetCString()));

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CHANGE_NAME"));
	}
}



void CvDLLWidgetData::parseTrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	UnitTypes eUnit;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	if (pHeadSelectedCity != NULL)
	{
		eUnit = (UnitTypes)GC.getCivilizationInfo(pHeadSelectedCity->getCivilizationType()).getCivilizationUnits(widgetDataStruct.m_iData1);

		GAMETEXT.setUnitHelp(szBuffer, eUnit, false, widgetDataStruct.m_bOption, false, pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseConstructHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	BuildingTypes eBuilding;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	if (pHeadSelectedCity != NULL)
	{
		eBuilding = (BuildingTypes)GC.getCivilizationInfo(pHeadSelectedCity->getCivilizationType()).getCivilizationBuildings(widgetDataStruct.m_iData1);

		GAMETEXT.setBuildingHelp(szBuffer, eBuilding, false, widgetDataStruct.m_bOption, false, pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseCreateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	if (widgetDataStruct.m_iData2 != FFreeList::INVALID_INDEX)
	{
		pHeadSelectedCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(widgetDataStruct.m_iData2);
	}
	else
	{
		pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	}

	GAMETEXT.setProjectHelp(szBuffer, ((ProjectTypes)widgetDataStruct.m_iData1), false, pHeadSelectedCity);
}


void CvDLLWidgetData::parseMaintainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.setProcessHelp(szBuffer, ((ProcessTypes)(widgetDataStruct.m_iData1)));
}


void CvDLLWidgetData::parseHurryHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;
	bool bFirst;
	int iHurryGold;
	int iHurryStone;
	int iHurryPopulation;
	int iHurryAngerLength;
	int iI;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_HURRY_PROD", pHeadSelectedCity->getProductionNameKey()));

		iHurryGold = pHeadSelectedCity->hurryGold((HurryTypes)(widgetDataStruct.m_iData1));

		if (iHurryGold > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_GOLD", iHurryGold));
		}

		iHurryStone = pHeadSelectedCity->hurryStone((HurryTypes)(widgetDataStruct.m_iData1));

		if (iHurryStone > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_STONE", iHurryStone));
		}

		iHurryPopulation = pHeadSelectedCity->hurryPopulation((HurryTypes)(widgetDataStruct.m_iData1));

		if (iHurryPopulation > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_POP", iHurryPopulation));

			if (iHurryPopulation > pHeadSelectedCity->maxHurryPopulation())
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_MAX_POP_HURRY", pHeadSelectedCity->maxHurryPopulation()));
			}
		}

		iHurryAngerLength = pHeadSelectedCity->hurryAngerLength((HurryTypes)(widgetDataStruct.m_iData1));

		if (iHurryAngerLength > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_ANGER_TURNS", GC.getDefineINT("HURRY_POP_ANGER"), (iHurryAngerLength + pHeadSelectedCity->getHurryAngerTimer())));
		}

		if (!(pHeadSelectedCity->isProductionUnit()) && !(pHeadSelectedCity->isProductionBuilding()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_UNIT_BUILDING_HURRY"));
		}

		if (!(GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).canHurry((HurryTypes)(widgetDataStruct.m_iData1))))
		{
			bFirst = true;

			for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
			{
				if (GC.getCivicInfo((CivicTypes)iI).isHurry(widgetDataStruct.m_iData1))
				{
					szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
					setListHelp(szBuffer, szTempBuffer, GC.getCivicInfo((CivicTypes)iI).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
					bFirst = false;
				}
			}

			if (!bFirst)
			{
				szBuffer.append(ENDCOLR);
			}
		}
	}
}


void CvDLLWidgetData::parseConscriptHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;
	int iConscriptPopulation;
	int iConscriptAngerLength;
	int iMinCityPopulation;
	int iMinCulturePercent;
	int iI;
	bool bFirst;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getConscriptUnit() != NO_UNIT)
		{
			CvWString szTemp;
			szTemp.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(pHeadSelectedCity->getConscriptUnit()).getDescription());
			szBuffer.assign(szTemp);

			iConscriptPopulation = pHeadSelectedCity->getConscriptPopulation();

			if (iConscriptPopulation > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HURRY_POP", iConscriptPopulation));
			}

			iConscriptAngerLength = pHeadSelectedCity->flatConscriptAngerLength();

			if (iConscriptAngerLength > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_ANGER_TURNS", GC.getDefineINT("CONSCRIPT_POP_ANGER"), (iConscriptAngerLength + pHeadSelectedCity->getConscriptAngerTimer())));
			}

			iMinCityPopulation = pHeadSelectedCity->conscriptMinCityPopulation();

			if (pHeadSelectedCity->getPopulation() < iMinCityPopulation)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_MIN_CITY_POP", iMinCityPopulation));
			}

			iMinCulturePercent = GC.getDefineINT("CONSCRIPT_MIN_CULTURE_PERCENT");

			if (pHeadSelectedCity->plot()->calculateTeamCulturePercent(pHeadSelectedCity->getTeam()) < iMinCulturePercent)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_MIN_CULTURE_PERCENT", iMinCulturePercent));
			}

			if (GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getMaxConscript() == 0)
			{
				bFirst = true;

				for (iI = 0; iI < GC.getNumCivicInfos(); iI++)
				{
					if (getWorldSizeMaxConscript((CivicTypes)iI) > 0)
					{
						szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
						setListHelp(szBuffer, szTempBuffer, GC.getCivicInfo((CivicTypes)iI).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}

				if (!bFirst)
				{
					szBuffer.append(ENDCOLR);
				}
			}
		}
	}
}


void CvDLLWidgetData::parseActionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CLLNode<IDInfo>* pSelectedUnitNode;
	CvCity* pMissionCity;
	CvCity* pCity;
	CvUnit* pHeadSelectedUnit;
	CvUnit* pSelectedUnit;
	CvPlot* pMissionPlot;
	CvPlot* pLoopPlot;
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	ReligionTypes eReligion;
	BuildingTypes eBuilding;
	PlayerTypes eGiftPlayer;
	BuildTypes eBuild;
	RouteTypes eRoute;
	BonusTypes eBonus;
	TechTypes eTech;
	bool bAlt;
	bool bShift;
	bool bValid;
	int iYield;
	int iUnitConsume;
	int iUnitDiff;
	int iProduction;
	int iMovementCost;
	int iFlatMovementCost;
	int iMoves;
	int iFlatMoves;
	int iNowWorkRate;
	int iThenWorkRate;
	int iTurns;
	int iPrice;
	int iLow;
	int iHigh;
	int iLast;
	int iRange;
	int iDX, iDY;
	int iI;

	bAlt = gDLL->altKey();
	bShift = gDLL->shiftKey();

	CvWString szTemp;
	szTemp.Format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getActionInfo(widgetDataStruct.m_iData1).getHotKeyDescription().c_str());
	szBuffer.assign(szTemp);

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() != NO_MISSION)
		{
			if (bShift && gDLL->getInterfaceIFace()->mirrorsSelectionGroup())
			{
				pMissionPlot = pHeadSelectedUnit->getGroup()->lastMissionPlot();
			}
			else
			{
				pMissionPlot = pHeadSelectedUnit->plot();
			}

			pMissionCity = pMissionPlot->getPlotCity();

			if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_HEAL)
			{
				iTurns = 0;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
					iTurns = std::max(iTurns, pSelectedUnit->healTurns(pMissionPlot));

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_OR_TURNS", iTurns));
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_PILLAGE)
			{
				if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DESTROY_IMP", GC.getImprovementInfo(pMissionPlot->getImprovementType()).getTextKeyWide()));
				}
				else if (pMissionPlot->getRouteType() != NO_ROUTE)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DESTROY_IMP", GC.getRouteInfo(pMissionPlot->getRouteType()).getTextKeyWide()));
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_PLUNDER)
			{
				pMissionPlot = pHeadSelectedUnit->plot();

				if (pMissionPlot->getTeam() == pHeadSelectedUnit->getTeam())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PLUNDER_IN_BORDERS"));
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SABOTAGE)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canSabotage(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						iPrice = pSelectedUnit->sabotageCost(pMissionPlot);
						if (iPrice > 0)
						{
							szTempBuffer.Format(L"%d %c", iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}

						iLow = pSelectedUnit->sabotageProb(pMissionPlot, PROBABILITY_LOW);
						iHigh = pSelectedUnit->sabotageProb(pMissionPlot, PROBABILITY_HIGH);

						if (iLow == iHigh)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY", iHigh));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY_RANGE", iLow, iHigh));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_DESTROY)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canDestroy(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						iPrice = pSelectedUnit->destroyCost(pMissionPlot);
						if (iPrice > 0)
						{
							szTempBuffer.Format(L"%d %c", iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}

						iLow = pSelectedUnit->destroyProb(pMissionPlot, PROBABILITY_LOW);
						iHigh = pSelectedUnit->destroyProb(pMissionPlot, PROBABILITY_HIGH);

						if (iLow == iHigh)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY", iHigh));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY_RANGE", iLow, iHigh));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_STEAL_PLANS)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canStealPlans(pMissionPlot, true)) // XXX if queuing up this action, use the current plot along the goto...
					{
						iPrice = pSelectedUnit->stealPlansCost(pMissionPlot);
						if (iPrice > 0)
						{
							szTempBuffer.Format(L"%d %c", iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}

						iLow = pSelectedUnit->stealPlansProb(pMissionPlot, PROBABILITY_LOW);
						iHigh = pSelectedUnit->stealPlansProb(pMissionPlot, PROBABILITY_HIGH);

						if (iLow == iHigh)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY", iHigh));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROBABILITY_RANGE", iLow, iHigh));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_FOUND)
			{
				if (!(GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).canFound(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE())))
				{
					bValid = true;

					iRange = GC.getMIN_CITY_RANGE();

					for (iDX = -(iRange); iDX <= iRange; iDX++)
					{
						for (iDY = -(iRange); iDY <= iRange; iDY++)
						{
							pLoopPlot	= plotXY(pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE(), iDX, iDY);

							if (pLoopPlot != NULL)
							{
								if (pLoopPlot->isCity())
								{
									bValid = false;
								}
							}
						}
					}

					if (!bValid)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_FOUND", GC.getMIN_CITY_RANGE()));
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SPREAD)
			{
				eReligion = ((ReligionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));

				if (pMissionCity != NULL)
				{
					if (pMissionCity->getTeam() != pHeadSelectedUnit->getTeam()) // XXX still true???
					{
						if (GET_PLAYER(pMissionCity->getOwnerINLINE()).isNoNonStateReligionSpread())
						{
							if (eReligion != GET_PLAYER(pMissionCity->getOwnerINLINE()).getStateReligion())
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CANNOT_SPREAD_NON_STATE_RELIGION"));
							}
						}
					}

					szBuffer.append(NEWLINE);
					GAMETEXT.setReligionHelpCity(szBuffer, eReligion, pMissionCity, false, true);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_SPREAD_CORPORATION)
			{
				CorporationTypes eCorporation = ((CorporationTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));

				if (pMissionCity != NULL)
				{
					szBuffer.append(NEWLINE);
					GAMETEXT.setCorporationHelpCity(szBuffer, eCorporation, pMissionCity, false, true);

					for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
					{
						if (eCorporation != iCorp)
						{
							if (pMissionCity->isHasCorporation((CorporationTypes)iCorp))
							{
								if (GC.getGameINLINE().isCompetingCorporation(eCorporation, (CorporationTypes)iCorp))
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_WILL_ELIMINATE_CORPORATION", GC.getCorporationInfo((CorporationTypes)iCorp).getTextKeyWide()));
								}
							}
						}
					}

					szTempBuffer.Format(L"%s%d %c", NEWLINE, pHeadSelectedUnit->spreadCorporationCost(eCorporation, pMissionCity), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
					szBuffer.append(szTempBuffer);

					if (!pHeadSelectedUnit->canSpreadCorporation(pMissionPlot, eCorporation))
					{
						if (!GET_PLAYER(pMissionCity->getOwnerINLINE()).isActiveCorporation(eCorporation))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CORPORATION_NOT_ACTIVE", GC.getCorporationInfo(eCorporation).getTextKeyWide(), GET_PLAYER(pMissionCity->getOwnerINLINE()).getCivilizationAdjective()));
						}

						CorporationTypes eCompetition = NO_CORPORATION;
						for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); ++iCorporation)
						{
							if (pMissionCity->isHeadquarters((CorporationTypes)iCorporation))
							{
								if (GC.getGameINLINE().isCompetingCorporation((CorporationTypes)iCorporation, eCorporation))
								{
									eCompetition = (CorporationTypes)iCorporation;
									break;
								}
							}
						}

						if (NO_CORPORATION != eCompetition)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CORPORATION_COMPETING_HEADQUARTERS", GC.getCorporationInfo(eCorporation).getTextKeyWide(), GC.getCorporationInfo(eCompetition).getTextKeyWide()));
						}

						CvWStringBuffer szBonusList;
						bool bValid = false;
						bool bFirst = true;
						for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
						{
							BonusTypes eBonus = (BonusTypes)GC.getCorporationInfo(eCorporation).getPrereqBonus(i);
							if (NO_BONUS != eBonus)
							{
								if (!bFirst)
								{
									szBonusList.append(L", ");
								}
								else
								{
									bFirst = false;
								}
								szBonusList.append(GC.getBonusInfo(eBonus).getDescription());

								if (pMissionCity->hasBonus(eBonus))
								{
									bValid = true;
									break;
								}
							}
						}

						if (!bValid)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CORPORATION_NO_RESOURCES", pMissionCity->getNameKey(), szBonusList.getCString()));
						}
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_JOIN)
			{
				GAMETEXT.parseSpecialistHelp(szBuffer, ((SpecialistTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData())), pMissionCity, true);
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_CONSTRUCT)
			{
				eBuilding = ((BuildingTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));

				if (pMissionCity != NULL)
				{
					if (!pHeadSelectedUnit->getUnitInfo().getForceBuildings(eBuilding)  && !pMissionCity->canConstruct(eBuilding, false, false, true))
					{
						if (!(GC.getGameINLINE().isBuildingClassMaxedOut((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType()))))
						{
							GAMETEXT.buildBuildingRequiresString(szBuffer, ((BuildingTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData())), false, false, pMissionCity);
						}
					}
					else
					{
						szBuffer.append(NEWLINE);
						GAMETEXT.setBuildingHelp(szBuffer, ((BuildingTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData())), false, false, false, pMissionCity);
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_DISCOVER)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canDiscover(pMissionPlot))
					{
						eTech = pSelectedUnit->getDiscoveryTech();

						if (pSelectedUnit->getDiscoverResearch(eTech) >= GET_TEAM(pSelectedUnit->getTeam()).getResearchLeft(eTech))
						{
							szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTech).getDescription());
							szBuffer.append(NEWLINE);
							szBuffer.append(szTempBuffer);
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_EXTRA_RESEARCH", pSelectedUnit->getDiscoverResearch(eTech), GC.getTechInfo(eTech).getTextKeyWide()));
						}
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_HURRY)
			{
				if (pMissionCity != NULL)
				{
					if (!(pMissionCity->isProductionBuilding()))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_BUILDING_HURRY"));
					}
					else
					{
						pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

						while (pSelectedUnitNode != NULL)
						{
							pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

							if (pSelectedUnit->canHurry(pMissionPlot, true))
							{
								const wchar* pcKey = NULL;
								if (NO_PROJECT != pMissionCity->getProductionProject())
								{
									pcKey = GC.getProjectInfo(pMissionCity->getProductionProject()).getTextKeyWide();
								}
								else if (NO_BUILDING != pMissionCity->getProductionBuilding())
								{
									pcKey = GC.getBuildingInfo(pMissionCity->getProductionBuilding()).getTextKeyWide();
								}
								else if (NO_UNIT != pMissionCity->getProductionUnit())
								{
									pcKey = GC.getUnitInfo(pMissionCity->getProductionUnit()).getTextKeyWide();
								}
								if (NULL != pcKey && pSelectedUnit->getHurryProduction(pMissionPlot) >= pMissionCity->productionLeft())
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_FINISH_CONSTRUCTION", pcKey));
								}
								else
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_EXTRA_CONSTRUCTION", pSelectedUnit->getHurryProduction(pMissionPlot), pcKey));
								}
								break;
							}

							pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
						}
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_TRADE)
			{
				if (pMissionCity != NULL)
				{
					if (pMissionCity->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_TRADE_MISSION_FOREIGN"));
					}
					else
					{
						pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

						while (pSelectedUnitNode != NULL)
						{
							pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

							if (pSelectedUnit->canTrade(pMissionPlot, true))
							{
								szTempBuffer.Format(L"%s+%d%c", NEWLINE, pSelectedUnit->getTradeGold(pMissionPlot), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
								szBuffer.append(szTempBuffer);
								break;
							}

							pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
						}
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_GREAT_WORK)
			{
				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canGreatWork(pMissionPlot))
					{
						szTempBuffer.Format(L"%s+%d%c", NEWLINE, pSelectedUnit->getGreatWorkCulture(pMissionPlot), GC.getCommerceInfo(COMMERCE_CULTURE).getChar());
						szBuffer.append(szTempBuffer);
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_INFILTRATE)
			{
				if (pMissionCity != NULL)
				{
					if (pMissionCity->getOwnerINLINE() == pHeadSelectedUnit->getOwnerINLINE())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_INFILTRATE_MISSION_FOREIGN"));
					}
					else
					{
						pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

						while (pSelectedUnitNode != NULL)
						{
							pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

							if (pSelectedUnit->canEspionage(pMissionPlot))
							{
								szTempBuffer.Format(L"%s+%d%c", NEWLINE, pSelectedUnit->getEspionagePoints(pMissionPlot), GC.getCommerceInfo(COMMERCE_ESPIONAGE).getChar());
								szBuffer.append(szTempBuffer);
								break;
							}

							pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
						}
					}
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_GOLDEN_AGE)
			{
				iUnitConsume = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).unitsRequiredForGoldenAge();
				iUnitDiff = (iUnitConsume - GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).unitsGoldenAgeReady());

				if (iUnitDiff > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_MORE_GREAT_PEOPLE", iUnitDiff));
				}

				if (iUnitConsume > 1)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONSUME_GREAT_PEOPLE", iUnitConsume));
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_LEAD)
			{
				if (pHeadSelectedUnit->getUnitInfo().getLeaderExperience() > 0)
				{
					int iNumUnits = pHeadSelectedUnit->canGiveExperience(pHeadSelectedUnit->plot());
					if (iNumUnits > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_LEAD_TROOPS", pHeadSelectedUnit->getStackExperienceToGive(iNumUnits)));
					}
				}
				if (pHeadSelectedUnit->getUnitInfo().getLeaderPromotion() != NO_PROMOTION)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_WHEN_LEADING"));
					GAMETEXT.parsePromotionHelp(szBuffer, (PromotionTypes)pHeadSelectedUnit->getUnitInfo().getLeaderPromotion(), L"\n   ");
				}
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_ESPIONAGE)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ESPIONAGE_MISSION"));

				GAMETEXT.setEspionageMissionHelp(szBuffer, pHeadSelectedUnit);
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType() == MISSION_BUILD)
			{
				eBuild = ((BuildTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionData()));
				FAssert(eBuild != NO_BUILD);
				eImprovement = ((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement()));
				eRoute = ((RouteTypes)(GC.getBuildInfo(eBuild).getRoute()));
				eBonus = pMissionPlot->getBonusType(pHeadSelectedUnit->getTeam());

				for (iI = 0; iI < NUM_YIELD_TYPES; iI++)
				{
					iYield = 0;

					if (eImprovement != NO_IMPROVEMENT)
					{
						iYield += pMissionPlot->calculateImprovementYieldChange(eImprovement, ((YieldTypes)iI), pHeadSelectedUnit->getOwnerINLINE());
						if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
						{
							iYield -= pMissionPlot->calculateImprovementYieldChange(pMissionPlot->getImprovementType(), ((YieldTypes)iI), pHeadSelectedUnit->getOwnerINLINE());
						}
					}

					if (NO_FEATURE != pMissionPlot->getFeatureType())
					{
						if (GC.getBuildInfo(eBuild).isFeatureRemove(pMissionPlot->getFeatureType())

//FfH: Added by Kael 04/24/2008
                          && !GC.getCivilizationInfo(GET_PLAYER(pHeadSelectedUnit->getOwner()).getCivilizationType()).isMaintainFeatures(pMissionPlot->getFeatureType())
//FfH: End Add

						)
						{
							iYield -= GC.getFeatureInfo(pMissionPlot->getFeatureType()).getYieldChange(iI);
						}
					}

					if (iYield != 0)
					{
						szTempBuffer.Format(L", %s%d%c", ((iYield > 0) ? "+" : ""), iYield, GC.getYieldInfo((YieldTypes) iI).getChar());
						szBuffer.append(szTempBuffer);
					}
				}

				if (NO_IMPROVEMENT != eImprovement)
				{
					int iHappy = GC.getImprovementInfo(eImprovement).getHappiness();					

					if (iHappy != 0)
					{
						szBuffer.append(NEWLINE);
						float fHappiness=0.01f * (float)iHappy;

						szTempBuffer.Format(L"+%0.2f%c", abs(fHappiness), (fHappiness > 0 ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
						szBuffer.append(szTempBuffer);
					}

					int iHealth = GC.getImprovementInfo(eImprovement).getHealth();

					if (iHealth != 0)
					{
						szBuffer.append(NEWLINE);
						float fHealth=0.01f * (float)iHealth;

						szTempBuffer.Format(L"+%0.2f%c", abs(fHealth), (fHealth > 0 ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
						szBuffer.append(szTempBuffer);
					}
				}
//added magic help
				if (NO_IMPROVEMENT != eImprovement)
				{
					if(GC.getImprovementInfo(eImprovement).getBonusConvert() != NO_BONUS)
					{
						BonusTypes eBonus = (BonusTypes)GC.getImprovementInfo(eImprovement).getBonusConvert();
						if(GC.getBonusInfo(eBonus).getBonusClassType() == GC.getInfoTypeForString("BONUSCLASS_MANA"))
						{
							for(int iJ = 0; iJ < GC.getNumManaschoolInfos(); iJ++)
							{
								if (GC.getManaschoolInfo((ManaschoolTypes)iJ).isBonusLinked(eBonus))
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MANASCHOOL_LINKED", GC.getManaschoolInfo((ManaschoolTypes)iJ).getDescription()));
								}

								if (GC.getManaschoolInfo((ManaschoolTypes)iJ).getBonusIncreasedMana()==eBonus)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MORE_MANA"));
								}

								if (GC.getManaschoolInfo((ManaschoolTypes)iJ).getBonusIncreasedSpellResearch()==eBonus)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MORE_SPELL_RESEARCH"));
								}

								if (GC.getManaschoolInfo((ManaschoolTypes)iJ).getBonusIncreasedTerraformEffect()==eBonus)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MORE_TERRAFORM_EFFECT", GC.getManaschoolInfo((ManaschoolTypes)iJ).getDescription()));
								}

								if (GC.getManaschoolInfo((ManaschoolTypes)iJ).getBonusReducedGlobalEnchantment()==eBonus)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_BONUS_REDUCED_ENCHANTMENT", GC.getManaschoolInfo((ManaschoolTypes)iJ).getDescription()));
								}

								if (GC.getManaschoolInfo((ManaschoolTypes)iJ).getBonusReducedTerraformCost()==eBonus)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_BONUS_REDUCED_TERRAFORM", GC.getManaschoolInfo((ManaschoolTypes)iJ).getDescription()));
								}

								if (GC.getManaschoolInfo((ManaschoolTypes)iJ).getBonusSummonStrength()==eBonus)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_BONUS_SUMMON_STRENGTH", GC.getManaschoolInfo((ManaschoolTypes)iJ).getDescription()));
								}
							}
						}
					}
				}
//end added magic help
				bValid = false;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->canBuild(pMissionPlot, eBuild))
					{
						bValid = true;
						break;
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				if (!bValid)
				{
					if (eImprovement != NO_IMPROVEMENT)
					{
						if (pMissionPlot->getTeam() != pHeadSelectedUnit->getTeam())
						{
							if (GC.getImprovementInfo(eImprovement).isOutsideBorders())
							{
								if (pMissionPlot->getTeam() != NO_TEAM)
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NEEDS_OUT_RIVAL_CULTURE_BORDER"));
								}
							}
							else
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NEEDS_CULTURE_BORDER"));
							}
						}

						if ((eBonus == NO_BONUS) || !(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eBonus)))
						{
							if (!(GET_TEAM(pHeadSelectedUnit->getTeam()).isIrrigation()) && !(GET_TEAM(pHeadSelectedUnit->getTeam()).isIgnoreIrrigation()))
							{
								if (GC.getImprovementInfo(eImprovement).isRequiresIrrigation() && !(pMissionPlot->isIrrigationAvailable()))
								{
									for (iI = 0; iI < GC.getNumTechInfos(); iI++)
									{
										if (GC.getTechInfo((TechTypes)iI).isIrrigation())
										{
											szBuffer.append(NEWLINE);
											szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes)iI).getTextKeyWide()));
											break;
										}
									}
								}
							}
						}
					}

					if (!(GET_TEAM(pHeadSelectedUnit->getTeam()).isHasTech((TechTypes)GC.getBuildInfo(eBuild).getTechPrereq())))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes) GC.getBuildInfo(eBuild).getTechPrereq()).getTextKeyWide()));
					}

					if (eRoute != NO_ROUTE)
					{
						if (GC.getRouteInfo(eRoute).getPrereqBonus() != NO_BONUS)
						{
							if (!(pMissionPlot->isAdjacentPlotGroupConnectedBonus(pHeadSelectedUnit->getOwnerINLINE(), ((BonusTypes)(GC.getRouteInfo(eRoute).getPrereqBonus())))))
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getBonusInfo((BonusTypes) GC.getRouteInfo(eRoute).getPrereqBonus()).getTextKeyWide()));
							}
						}

						bool bFoundValid = true;
						std::vector<BonusTypes> aeOrBonuses;
						for (int i = 0; i < GC.getNUM_ROUTE_PREREQ_OR_BONUSES(); ++i)
						{
							if (NO_BONUS != GC.getRouteInfo(eRoute).getPrereqOrBonus(i))
							{
								aeOrBonuses.push_back((BonusTypes)GC.getRouteInfo(eRoute).getPrereqOrBonus(i));
								bFoundValid = false;

								if (pMissionPlot->isAdjacentPlotGroupConnectedBonus(pHeadSelectedUnit->getOwnerINLINE(), ((BonusTypes)(GC.getRouteInfo(eRoute).getPrereqOrBonus(i)))))
								{
									bFoundValid = true;
									break;
								}
							}
						}

						if (!bFoundValid)
						{
							bool bFirst = true;
							for (std::vector<BonusTypes>::iterator it = aeOrBonuses.begin(); it != aeOrBonuses.end(); ++it)
							{
								szFirstBuffer = NEWLINE + gDLL->getText("TXT_KEY_BUILDING_REQUIRES_LIST");
								szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo(*it).getDescription());
								setListHelp(szBuffer, szFirstBuffer.GetCString(), szTempBuffer, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
								bFirst = false;
							}
						}
					}

					if (pMissionPlot->getFeatureType() != NO_FEATURE

//FfH: Added by Kael 12/16/2007
                      && !GC.getCivilizationInfo(GET_PLAYER(pHeadSelectedUnit->getOwner()).getCivilizationType()).isMaintainFeatures(pMissionPlot->getFeatureType())
//FfH: End Add

					)
					{
						if (!(GET_TEAM(pHeadSelectedUnit->getTeam()).isHasTech((TechTypes)GC.getBuildInfo(eBuild).getFeatureTech(pMissionPlot->getFeatureType()))))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes) GC.getBuildInfo(eBuild).getFeatureTech(pMissionPlot->getFeatureType())).getTextKeyWide()));
						}
					}
				}

				if (eImprovement != NO_IMPROVEMENT)
				{
					if (pMissionPlot->getImprovementType() != NO_IMPROVEMENT)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_WILL_DESTROY_IMP", GC.getImprovementInfo(pMissionPlot->getImprovementType()).getTextKeyWide()));
					}
				}

				if (GC.getBuildInfo(eBuild).isKill())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONSUME_UNIT"));
				}

				if (pMissionPlot->getFeatureType() != NO_FEATURE)
				{
					if (GC.getBuildInfo(eBuild).isFeatureRemove(pMissionPlot->getFeatureType())

//FfH: Added by Kael 12/16/2007
                      && !GC.getCivilizationInfo(GET_PLAYER(pHeadSelectedUnit->getOwner()).getCivilizationType()).isMaintainFeatures(pMissionPlot->getFeatureType())
//FfH: End Add

					)
					{
						iProduction = pMissionPlot->getFeatureProduction(eBuild, pHeadSelectedUnit->getTeam(), &pCity);

						if (iProduction > 0)
						{
							szBuffer.append(NEWLINE);
/**
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CHANGE_PRODUCTION", iProduction, pCity->getNameKey()));
**/
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CHANGE_LUMBER", iProduction));
						}

						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_REMOVE_FEATURE", GC.getFeatureInfo(pMissionPlot->getFeatureType()).getTextKeyWide()));
					}

				}

				if (eImprovement != NO_IMPROVEMENT)
				{
					if (eBonus != NO_BONUS)
					{
						if (!GET_TEAM(pHeadSelectedUnit->getTeam()).isBonusObsolete(eBonus))
						{
							if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eBonus))
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROVIDES_BONUS", GC.getBonusInfo(eBonus).getTextKeyWide()));

								if (GC.getBonusInfo(eBonus).getHealth() != 0)
								{
									szTempBuffer.Format(L" (+%d%c)", abs(GC.getBonusInfo(eBonus).getHealth()), ((GC.getBonusInfo(eBonus).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
									szBuffer.append(szTempBuffer);
								}

								if (GC.getBonusInfo(eBonus).getHappiness() != 0)
								{
									szTempBuffer.Format(L" (+%d%c)", abs(GC.getBonusInfo(eBonus).getHappiness()), ((GC.getBonusInfo(eBonus).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
									szBuffer.append(szTempBuffer);
								}

								if(GC.getBonusInfo(eBonus).getTechCityTrade()!=NO_TECH && !GET_TEAM(pHeadSelectedUnit->getTeam()).isHasTech((TechTypes)GC.getBonusInfo(eBonus).getTechCityTrade()))
								{
									szBuffer.append(gDLL->getText("TXT_KEY_ACTION_PROVIDES_BONUS_NEW",GC.getTechInfo((TechTypes)GC.getBonusInfo(eBonus).getTechCityTrade()).getDescription()));
								}

							}
						}
					}
					else
					{
						iLast = 0;

						FAssert((0 < GC.getNumBonusInfos()) && "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvDLLWidgetData::parseActionHelp");
						for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
						{
							if (GET_TEAM(pHeadSelectedUnit->getTeam()).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes) iI).getTechReveal())))
							{
								if (GC.getImprovementInfo(eImprovement).getImprovementBonusDiscoverRand(iI) > 0)
								{
									szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_ACTION_CHANCE_DISCOVER").c_str());
									szTempBuffer.Format(L"%c", GC.getBonusInfo((BonusTypes) iI).getChar());
									setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (GC.getImprovementInfo(eImprovement).getImprovementBonusDiscoverRand(iI) != iLast));
									iLast = GC.getImprovementInfo(eImprovement).getImprovementBonusDiscoverRand(iI);
								}
							}
						}
					}

					if (!(pMissionPlot->isIrrigationAvailable()))
					{
						GAMETEXT.setYieldChangeHelp(szBuffer, gDLL->getText("TXT_KEY_ACTION_IRRIGATED").c_str(), L": ", L"", GC.getImprovementInfo(eImprovement).getIrrigatedYieldChangeArray());
					}

					if (eRoute == NO_ROUTE)
					{
						for (iI = 0; iI < GC.getNumRouteInfos(); iI++)
						{
							if (pMissionPlot->getRouteType() != ((RouteTypes)iI))
							{
								GAMETEXT.setYieldChangeHelp(szBuffer, GC.getRouteInfo((RouteTypes)iI).getDescription(), L": ", L"", GC.getImprovementInfo(eImprovement).getRouteYieldChangesArray((RouteTypes)iI));
							}
						}
					}

					if (GC.getImprovementInfo(eImprovement).getDefenseModifier() != 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_DEFENSE_MODIFIER", GC.getImprovementInfo(eImprovement).getDefenseModifier()));
					}

					if (GC.getImprovementInfo(eImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
					{
						iTurns = pMissionPlot->getUpgradeTimeLeft(eImprovement, pHeadSelectedUnit->getOwnerINLINE());

						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTION_BECOMES_IMP", GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide(), iTurns));
					}
				}

//DEBUG Sephi
				CvUnit* pWorker = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
				if(pWorker!=NULL)
				{
					CvCity* pWorkingCity=pWorker->plot()->getWorkingCity();

					if(eImprovement!=NO_IMPROVEMENT)
					{
						CvImprovementInfo &info=GC.getImprovementInfo(eImprovement);

						if (!CvWString(info.getHelp()).empty())
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(info.getHelp());
						}
					}


					BuildTypes eBuild=NO_BUILD;
					for(int iI=0;iI<GC.getNumBuildInfos();iI++)
					{
						if(GC.getBuildInfo((BuildTypes)iI).getImprovement()==eImprovement)
						{
							if(GC.getUnitInfo(pWorker->getUnitType()).getBuilds((BuildTypes)iI))
							{
								eBuild=(BuildTypes)iI;
								break;
							}
						}
					}

					if(eBuild!=NO_BUILD && eImprovement!=NO_IMPROVEMENT)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ACTUAL_PLOT_YIELDS"));
						for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
						{
							iYield = pWorker->plot()->ActualImprovementChanges(pWorker->getOwnerINLINE(),eBuild,(YieldTypes)iI,NO_COMMERCE,false);

							if (iYield != 0 && iI!=YIELD_COMMERCE)
							{
								CvWString szFloatString;
								float fYield=0.01f * (float)iYield;
								szFloatString.Format(L"%.02f", fYield);

								szTempBuffer.Format(L", %s%s%c", ((iYield > 0) ? "+" : ""), szFloatString.GetCString(), GC.getYieldInfo((YieldTypes) iI).getChar());
								szBuffer.append(szTempBuffer);
							}
						}

						for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
						{
							int iCommerce = pWorker->plot()->ActualImprovementChanges(pWorker->getOwnerINLINE(),eBuild,NO_YIELD,(CommerceTypes)iI,false);

							if (iCommerce != 0)
							{
								CvWString szFloatString;
								float fYield=0.01f * (float)iCommerce;
								szFloatString.Format(L"%2.02f", fYield);

								szTempBuffer.Format(L", %s%s%c", ((iCommerce > 0) ? "+" : ""), szFloatString.GetCString(), GC.getCommerceInfo((CommerceTypes)iI).getChar());
								szBuffer.append(szTempBuffer);
							}
						}

						if(pWorkingCity!=NULL)
						{
							CvCityAI* pCity=static_cast<CvCityAI*>(pWorkingCity);
							if (NO_IMPROVEMENT != eImprovement)
							{
								int iHappy = GC.getImprovementInfo(eImprovement).getHappiness();

								if (iHappy > 0)
								{
									float fHappiness=0.01f * (float)iHappy;

									szTempBuffer.Format(L", +%0.2f%c", abs(fHappiness), (fHappiness > 0 ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
									szBuffer.append(szTempBuffer);
								}
								else if(iHappy<0)
								{
									if(-iHappy-100*(pCity->happyLevel()-pCity->unhappyLevel(0))>0)
									{
										float fHappiness=0.01f * (iHappy+100*(pCity->happyLevel()-pCity->unhappyLevel(0)));

										szTempBuffer.Format(L", +%0.2f%c", abs(fHappiness), (fHappiness > 0 ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
										szBuffer.append(szTempBuffer);
									}
								}
							}

							if (GC.getGameINLINE().isDebugMode())
							{				
								szBuffer.append(NEWLINE);
								szTempBuffer.Format(L"BuildValue is %d",pCity->AI_PlotBuildValue(eBuild, pWorker->plot()));
								szBuffer.append(szTempBuffer);
							}
						}
/**
						if(pWorkingCity!=NULL && eImprovement!=NO_IMPROVEMENT)
						{
							int iFavoredImp=GC.getCivilizationInfo(GET_PLAYER(pWorkingCity->getOwnerINLINE()).getCivilizationType()).getImpInfrastructureHalfCost();
							if(eImprovement!=iFavoredImp || pWorkingCity->getNumFavoredImprovements()>=GC.getDefineINT("FREE_FAVORED_IMPROVEMENTS_PER_CITY"))
							{
								int iCost=pWorkingCity->calculateImprovementInfrastructureCost(eImprovement);
								if(iCost>0)
								{
									int iFree=pWorkingCity->calculateImprovementInfrastructureCostFreeLeft();
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_INFRASTRUCTURE_COST_NEXT_IMPROVEMENT",iCost/100,iCost%100,iFree/100,iFree%100));								
								}
							}
						}
**/
					}
				}

//DEBUG Sephi

				if (eRoute != NO_ROUTE)
				{
					eFinalImprovement = eImprovement;

					if (eFinalImprovement == NO_IMPROVEMENT)
					{
						eFinalImprovement = pMissionPlot->getImprovementType();
					}

					if (eFinalImprovement != NO_IMPROVEMENT)
					{
						GAMETEXT.setYieldChangeHelp(szBuffer, GC.getImprovementInfo(eFinalImprovement).getDescription(), L": ", L"", GC.getImprovementInfo(eFinalImprovement).getRouteYieldChangesArray(eRoute));
					}

					iMovementCost = GC.getRouteInfo(eRoute).getMovementCost() + GET_TEAM(pHeadSelectedUnit->getTeam()).getRouteChange(eRoute);
					iFlatMovementCost = GC.getRouteInfo(eRoute).getFlatMovementCost();

					if (iMovementCost > 0)
					{
						iMoves = (GC.getMOVE_DENOMINATOR() / iMovementCost);

						if ((iMoves * iMovementCost) < GC.getMOVE_DENOMINATOR())
						{
							iMoves++;
						}
					}
					else
					{
						iMoves = GC.getMOVE_DENOMINATOR();
					}

					if (iFlatMovementCost > 0)
					{
						iFlatMoves = (GC.getMOVE_DENOMINATOR() / iFlatMovementCost);

						if ((iFlatMoves * iFlatMovementCost) < GC.getMOVE_DENOMINATOR())
						{
							iFlatMoves++;
						}
					}
					else
					{
						iFlatMoves = GC.getMOVE_DENOMINATOR();
					}

					if ((iMoves > 1) || (iFlatMoves > 1))
					{
						if (iMoves >= iFlatMoves)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_MOVEMENT_COST", iMoves));
						}
						else
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_ACTION_FLAT_MOVEMENT_COST", iFlatMoves));
						}
					}

					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_CONNECTS_RESOURCES"));
				}

				iNowWorkRate = 0;
				iThenWorkRate = 0;

				pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

				if (NULL != pHeadSelectedUnit)
				{
					if (GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getBuildCost(pMissionPlot, eBuild) > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_BUILD_COST", GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getBuildCost(pMissionPlot, eBuild)));
					}

					if(eImprovement != NO_IMPROVEMENT && GC.getBuildInfo(eBuild).isApplyImprovementCost())
					{
						CvImprovementInfo& info = GC.getImprovementInfo(eImprovement);
						int iGoldCost = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getImprovementGoldCost(pMissionPlot, eBuild);
						int iCostMod = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getImprovementCostModifier(eImprovement);

						if (iGoldCost > 0) {
							if(iCostMod != 100) {
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_COST_MODIFIER", iCostMod - 100));
							}
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_COST_BASE"));
							szTempBuffer.Format(L"%d %c", iGoldCost, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
							szBuffer.append(szTempBuffer);
						}

						bool bDisplayYieldCosts = false;
						for(int i = 0; i < NUM_YIELD_TYPES; ++i) {
							if(info.getYieldCost(i) > 0) {
								bDisplayYieldCosts = true;
							}
						}
						if(bDisplayYieldCosts) {
							bool bFirst = false;
							if(iGoldCost > 0)
								bFirst = true;
							
							if(!bFirst) {
								if(iCostMod != 100) {
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_COST_MODIFIER", iCostMod - 100));
								}
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_COST_BASE"));
							}

							for(int i = 0; i < NUM_YIELD_TYPES; ++i) {
								int iYieldCost = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getImprovementYieldCost(pMissionPlot, eBuild, (YieldTypes)i);
								if(iYieldCost > 0) {
									if(!bFirst) {
										bFirst = true;
									} else {
										szTempBuffer.Format(L", ");
										szBuffer.append(szTempBuffer);
									}

									if(GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).getGlobalYield(i) < iYieldCost) {
										szTempBuffer.Format(SETCOLR L"%d %c" ENDCOLR, TEXT_COLOR("COLOR_WARNING_TEXT"), iYieldCost, GC.getYieldInfo((YieldTypes)i).getChar());
									} else {
										szTempBuffer.Format(SETCOLR L"%d %c" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), iYieldCost, GC.getYieldInfo((YieldTypes)i).getChar());
									}

									szBuffer.append(szTempBuffer);
								}
							}
						}
					}						 
				}

				while (pSelectedUnitNode != NULL)
				{
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

					if (pSelectedUnit->getBuildType() != eBuild)
					{
						iNowWorkRate += pSelectedUnit->workRate(false);
						iThenWorkRate += pSelectedUnit->workRate(true);
					}

					pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				}

				iTurns = pMissionPlot->getBuildTurnsLeft(eBuild, iNowWorkRate, iThenWorkRate);

				if(GC.getBuildInfo(eBuild).getTerrainPre() != NO_TERRAIN &&
					GC.getBuildInfo(eBuild).getTerrainPost() != NO_TERRAIN) {
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_TRANSFORM_TERRAIN", 
						CvGameTextMgr::GetInstance().getLinkedText((TerrainTypes)GC.getBuildInfo(eBuild).getTerrainPre()).c_str(), 
						CvGameTextMgr::GetInstance().getLinkedText((TerrainTypes)GC.getBuildInfo(eBuild).getTerrainPost()).c_str()));
				}

				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_NUM_TURNS", iTurns));

				if (!CvWString(GC.getBuildInfo(eBuild).getHelp()).empty())
				{
					szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getBuildInfo(eBuild).getHelp()).c_str());
				}
			}

			if (!CvWString(GC.getMissionInfo((MissionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType())).getHelp()).empty())
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getMissionInfo((MissionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getMissionType())).getHelp()).c_str());
			}
		}

		if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() != NO_COMMAND)
		{
			if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_PROMOTION)
			{
				GAMETEXT.parsePromotionHelp(szBuffer, ((PromotionTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));
			}

//FfH: Added by Kael 07/23/2007
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_CAST)
			{
				GAMETEXT.parseSpellHelp(szBuffer, ((SpellTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));
			}

/*************************************************************************************************/
/**	ADDON (automatic Spellcasting) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_AUTOMATE_SPELL)
			{
                szBuffer.append("automatically cast this spell at the begin of each turn (click to turn on/off)");
//				GAMETEXT.parseSpellHelp(szBuffer, ((SpellTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));
			}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//FfH: End Add

			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_UPGRADE)
			{
				GAMETEXT.setBasicUnitHelp(szBuffer, ((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())));

				if (bAlt && GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getAll())
				{
					iPrice = GET_PLAYER(pHeadSelectedUnit->getOwnerINLINE()).upgradeAllPrice(((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())), pHeadSelectedUnit->getUnitType());
				}
				else
				{
					iPrice = 0;

					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (pSelectedUnit->canUpgrade(((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData())), true))
						{
							iPrice += pSelectedUnit->upgradePrice((UnitTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandData()));
						}

						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}

				szTempBuffer.Format(L"%s%d %c", NEWLINE, iPrice, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
				szBuffer.append(szTempBuffer);
			}
			else if (GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType() == COMMAND_GIFT)
			{
				eGiftPlayer = pHeadSelectedUnit->plot()->getOwnerINLINE();

				if (eGiftPlayer != NO_PLAYER)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ACTION_GOES_TO_CIV"));

					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(eGiftPlayer).getPlayerTextColorR(), GET_PLAYER(eGiftPlayer).getPlayerTextColorG(), GET_PLAYER(eGiftPlayer).getPlayerTextColorB(), GET_PLAYER(eGiftPlayer).getPlayerTextColorA(), GET_PLAYER(eGiftPlayer).getCivilizationShortDescription());
					szBuffer.append(szTempBuffer);

					pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();

					while (pSelectedUnitNode != NULL)
					{
						pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);

						if (!(GET_PLAYER(eGiftPlayer).AI_acceptUnit(pSelectedUnit)))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_REFUSE_GIFT", GET_PLAYER(eGiftPlayer).getNameKey()));
							break;
						}

						pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
					}
				}
			}

			if (GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getAll())
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ALL_UNITS"));
			}

			if (!CvWString(GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getHelp()).empty())
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getCommandInfo((CommandTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getCommandType())).getHelp()).c_str());
			}
		}

		if (GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType() != NO_AUTOMATE)
		{
			if (!CvWString(GC.getAutomateInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType())).getHelp()).empty())
			{
				szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getAutomateInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getAutomateType())).getHelp()).c_str());
			}
		}
	}

	if (GC.getActionInfo(widgetDataStruct.m_iData1).getControlType() != NO_CONTROL)
	{
		if (!CvWString(GC.getControlInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getControlType())).getHelp()).empty())
		{
			szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getControlInfo((ControlTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getControlType())).getHelp()).c_str());
		}
	}

	if (GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType() != NO_INTERFACEMODE)
	{
		if (!CvWString(GC.getInterfaceModeInfo((InterfaceModeTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType())).getHelp()).empty())
		{
			szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getInterfaceModeInfo((InterfaceModeTypes)(GC.getActionInfo(widgetDataStruct.m_iData1).getInterfaceModeType())).getHelp()).c_str());
		}
	}
}


void CvDLLWidgetData::parseCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	int iCount;
	int iI;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (widgetDataStruct.m_iData1 != NO_SPECIALIST)
		{
			GAMETEXT.parseSpecialistHelp(szBuffer, ((SpecialistTypes)(widgetDataStruct.m_iData1)), pHeadSelectedCity);

			//added Debug
			CvWString szTempBuffer;
			szTempBuffer.Format(L"\n AI SpecialistValue old: %d", pHeadSelectedCity->AI_specialistValue((SpecialistTypes)widgetDataStruct.m_iData1,false,false));
			szBuffer.append(szTempBuffer);
			szTempBuffer.Format(L"\n AI SpecialistValue new: %d", pHeadSelectedCity->AI_specialistValueNew((SpecialistTypes)widgetDataStruct.m_iData1));
			szBuffer.append(szTempBuffer);
			
			//end added Debug

			if (widgetDataStruct.m_iData2 != -1)
			{
				iCount = 0;

				for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
				{
					if (iI < widgetDataStruct.m_iData1)
					{
						iCount += pHeadSelectedCity->getSpecialistCount((SpecialistTypes)iI);
					}
					else if (iI == widgetDataStruct.m_iData1)
					{
						iCount += widgetDataStruct.m_iData2;
					}
				}

				if (iCount < pHeadSelectedCity->totalFreeSpecialists())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_FREE_SPECIALIST"));
				}
			}
		}
	}
}


void CvDLLWidgetData::parseFreeCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		SpecialistTypes eSpecialist = (SpecialistTypes)widgetDataStruct.m_iData1;
		if (NO_SPECIALIST != eSpecialist)
		{
			GAMETEXT.parseSpecialistHelp(szBuffer, eSpecialist, pHeadSelectedCity);
		}
		if (widgetDataStruct.m_iData2 != -1)
		{
			szBuffer.append(SEPARATOR);
			GAMETEXT.parseFreeSpecialistHelp(szBuffer, *pHeadSelectedCity);
		}
	}
}


void CvDLLWidgetData::parseDisabledCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szTempBuffer;
	BuildingTypes eLoopBuilding;
	bool bFirst;
	int iI;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (widgetDataStruct.m_iData1 != NO_SPECIALIST)
		{
			GAMETEXT.parseSpecialistHelp(szBuffer, ((SpecialistTypes)(widgetDataStruct.m_iData1)), pHeadSelectedCity);

			if (!(pHeadSelectedCity->isSpecialistValid(((SpecialistTypes)(widgetDataStruct.m_iData1)), 1)))
			{
				bFirst = true;

				for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
				{
					eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);

					if (eLoopBuilding != NO_BUILDING)
					{
						if (GC.getBuildingInfo(eLoopBuilding).getSpecialistCount(widgetDataStruct.m_iData1) > 0)
						{
							if ((pHeadSelectedCity->getNumBuilding(eLoopBuilding) <= 0) && !isLimitedWonderClass((BuildingClassTypes)iI))
							{
								if ((GC.getBuildingInfo(eLoopBuilding).getSpecialBuildingType() == NO_SPECIALBUILDING) || pHeadSelectedCity->canConstruct(eLoopBuilding))
								{
									szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_REQUIRES");
									setListHelp(szBuffer, szTempBuffer, GC.getBuildingInfo(eLoopBuilding).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
									bFirst = false;
								}
							}
						}
					}
				}

				if (!bFirst)
				{
					szBuffer.append(ENDCOLR);
				}
			}
		}
	}
}


void CvDLLWidgetData::parseAngryCitizenHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ANGRY_CITIZEN"));
		szBuffer.append(NEWLINE);

		GAMETEXT.setAngerHelp(szBuffer, *pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseChangeSpecialistHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (widgetDataStruct.m_iData2 > 0)
		{
			GAMETEXT.parseSpecialistHelp(szBuffer, ((SpecialistTypes)(widgetDataStruct.m_iData1)), pHeadSelectedCity);

			if (widgetDataStruct.m_iData1 != GC.getDefineINT("DEFAULT_SPECIALIST"))
			{
				if (!(GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).isSpecialistValid((SpecialistTypes)(widgetDataStruct.m_iData1))))
				{
					if (pHeadSelectedCity->getMaxSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1)) > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_MISC_MAX_SPECIALISTS", pHeadSelectedCity->getMaxSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1))));
					}
				}
			}
		}
		else
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_REMOVE_SPECIALIST", GC.getSpecialistInfo((SpecialistTypes) widgetDataStruct.m_iData1).getTextKeyWide()));

			if (pHeadSelectedCity->getForceSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1)) > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FORCED_SPECIALIST", pHeadSelectedCity->getForceSpecialistCount((SpecialistTypes)(widgetDataStruct.m_iData1))));
			}
		}
	}
}


void CvDLLWidgetData::parseResearchHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	TechTypes eTech;

	eTech = ((TechTypes)(widgetDataStruct.m_iData1));

	if (eTech == NO_TECH)
	{
		//	No Technology
		if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCurrentResearch() != NO_TECH)
		{
			CvGameAI& game = GC.getGameINLINE();
			CvPlayer& activePlayer = GET_PLAYER(game.getActivePlayer());
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CHANGE_RESEARCH"));
			szBuffer.append(NEWLINE);
			GAMETEXT.setTechHelp(szBuffer, activePlayer.getCurrentResearch(), false, true);
		}
	}
	else
	{
		GAMETEXT.setTechHelp(szBuffer, eTech, false, true, widgetDataStruct.m_bOption);
	}
}


void CvDLLWidgetData::parseTechTreeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//FfH: Modified by Kael 07/30/2009
//	GAMETEXT.setTechHelp(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), false, false, false, false);
    bool bPlayerContext = true;
	if (GC.getGame().getActivePlayer() == NO_PLAYER)
	{
	    bPlayerContext = false;
	}
	GAMETEXT.setTechHelp(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), false, bPlayerContext, false, false);
//FfH: End Modify
}


void CvDLLWidgetData::parseChangePercentHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 > 0)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_INCREASE_RATE", GC.getCommerceInfo((CommerceTypes) widgetDataStruct.m_iData1).getTextKeyWide(), widgetDataStruct.m_iData2));
	}
	else
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_DECREASE_RATE", GC.getCommerceInfo((CommerceTypes) widgetDataStruct.m_iData1).getTextKeyWide(), -(widgetDataStruct.m_iData2)));
	}
}


void CvDLLWidgetData::parseContactCivHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	//	Do not execute this if we are trying to contact ourselves...
	if (widgetDataStruct.m_iData1 >= MAX_PLAYERS || GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getCivilizationType() == NO_CIVILIZATION)
	{
		return;
	}
	if (GC.getGameINLINE().getActivePlayer() == widgetDataStruct.m_iData1)
	{
		parseScoreHelp(widgetDataStruct, szBuffer);
		return;
	}

	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CONTACT_LEADER", GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getNameKey(), GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getCivilizationShortDescription()));

	szBuffer.append(NEWLINE);
	GAMETEXT.parsePlayerTraits(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);

	if (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasMet(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam())))
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HAVENT_MET_CIV"));
	}
	else
	{
		if (!(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).isHuman()))
		{
			if (!(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).AI_isWillingToTalk(GC.getGameINLINE().getActivePlayer())))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_REFUSES_TO_TALK"));
			}

			szBuffer.append(NEWLINE);
			GAMETEXT.getAttitudeString(szBuffer, ((PlayerTypes)widgetDataStruct.m_iData1), GC.getGameINLINE().getActivePlayer());

			szBuffer.append(NEWLINE);

//FfH: Modified by Kael 06/26/2008
//			GAMETEXT.getEspionageString(szBuffer, ((PlayerTypes)widgetDataStruct.m_iData1), GC.getGameINLINE().getActivePlayer());
            if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
            {
                GAMETEXT.getEspionageString(szBuffer, ((PlayerTypes)widgetDataStruct.m_iData1), GC.getGameINLINE().getActivePlayer());
            }
//FfH: End Modify

			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CTRL_TRADE"));
		}
		else
		{
			szBuffer.append(NEWLINE);

//FfH: Modified by Kael 06/26/2008
//			GAMETEXT.getEspionageString(szBuffer, ((PlayerTypes)widgetDataStruct.m_iData1), GC.getGameINLINE().getActivePlayer());
            if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
            {
                GAMETEXT.getEspionageString(szBuffer, ((PlayerTypes)widgetDataStruct.m_iData1), GC.getGameINLINE().getActivePlayer());
            }
//FfH: End Modify

		}

		if ((GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam() != GC.getGameINLINE().getActiveTeam()) && !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isAtWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam())))
		{
			if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).canDeclareWar(GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).getTeam()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALT_DECLARE_WAR"));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_CANNOT_DECLARE_WAR"));
			}
		}
	}

	if (GET_PLAYER((PlayerTypes)widgetDataStruct.m_iData1).isHuman())
	{
//		szBuffer += "\n(<SHIFT> to Send Chat Message)";
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SHIFT_SEND_CHAT"));
	}
}


void CvDLLWidgetData::parseConvertHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		if (widgetDataStruct.m_iData1 == NO_RELIGION)
		{
//			szBuffer.Format(L"No State %c", gDLL->getSymbolID(RELIGION_CHAR));
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_NO_STATE_REL"));
		}
		else
		{
//			szBuffer.Format(L"Convert to %s", GC.getReligionInfo((ReligionTypes) widgetDataStruct.m_iData1).getDescription());
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CONVERT_TO_REL", GC.getReligionInfo((ReligionTypes) widgetDataStruct.m_iData1).getTextKeyWide()));
		}
	}
	else
	{
		GAMETEXT.setConvertHelp(szBuffer, GC.getGameINLINE().getActivePlayer(), (ReligionTypes)widgetDataStruct.m_iData1);
	}
}


void CvDLLWidgetData::parseRevolutionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData1 != 0)
	{
//		szBuffer = "Change Civics";
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CHANGE_CIVICS"));
	}
	else
	{
		GAMETEXT.setRevolutionHelp(szBuffer, GC.getGameINLINE().getActivePlayer());
	}
}

/*void CvDLLWidgetData::parsePopupQueue(CvWidgetDataStruct &widgetDataStruct, CvWString &szBuffer)
{
	PopupEventTypes eEvent;

	if ( m_pPopup || m_pScreen )
	{
		if ( m_pPopup )
		{
			eEvent = m_pPopup->getPopupType();
		}
		else if ( m_pScreen )
		{
			eEvent = m_pScreen->getPopupType();
		}

		switch (eEvent)
		{
			case POPUPEVENT_NONE:
//				szBuffer = "***NO MOUSEOVER TEXT***.  Click to activate popup.";
				szBuffer = gDLL->getText("TXT_KEY_MISC_NO_MOUSEOVER_TEXT");
				break;

			case POPUPEVENT_PRODUCTION:
//				szBuffer = "Production Complete";
				szBuffer = gDLL->getText("TXT_KEY_MISC_PRODUCTION_COMPLETE");
				break;

			case POPUPEVENT_TECHNOLOGY:
//				szBuffer = "Technology Research Complete";
				szBuffer = gDLL->getText("TXT_KEY_MISC_TECH_RESEARCH_COMPLETE");
				break;

			case POPUPEVENT_RELIGION:
//				szBuffer = "New Religion Discovered";
				szBuffer = gDLL->getText("TXT_KEY_MISC_NEW_REL_DISCOVERED");
				break;

			case POPUPEVENT_WARNING:
//				szBuffer = "Warning!!!";
				szBuffer = gDLL->getText("TXT_KEY_MISC_WARNING");
				break;

			case POPUPEVENT_CIVIC:
//				szBuffer = "New Civic Accessible";
				szBuffer = gDLL->getText("TXT_KEY_MISC_NEW_CIVIC_ACCESSIBLE");
				break;
		}
	}
}*/

void CvDLLWidgetData::parseAutomateCitizensHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->isCitizensAutomated())
		{
//			szBuffer = "Turn Off Citizen Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_OFF_CITIZEN_AUTO"));
		}
		else
		{
//			szBuffer = "Turn On Citizen Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ON_CITIZEN_AUTO"));
		}
	}
}

void CvDLLWidgetData::parseAutomateProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->isProductionAutomated())
		{
//			szBuffer = "Turn Off Production Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_OFF_PROD_AUTO"));
		}
		else
		{
//			szBuffer = "Turn On Production Automation";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_ON_PROD_AUTO"));
		}
	}
}

void CvDLLWidgetData::parseEmphasizeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	szBuffer.clear();

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->AI_isEmphasize((EmphasizeTypes)widgetDataStruct.m_iData1))
		{
//			szBuffer += "Turn Off ";
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_OFF"));
		}
		else
		{
//			szBuffer += "Turn On ";
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_TURN_ON"));
		}
	}

	szBuffer.append(GC.getEmphasizeInfo((EmphasizeTypes)widgetDataStruct.m_iData1).getDescription());
}


void CvDLLWidgetData::parseTradeItem(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szTempBuffer;
	TradeData item;
	PlayerTypes eWhoFrom = NO_PLAYER;
	PlayerTypes eWhoTo = NO_PLAYER;
	DenialTypes eDenial;
	PlayerTypes eWhoDenies;

	szBuffer.clear();

	if (widgetDataStruct.m_bOption)
	{
		if ( gDLL->isDiplomacy())
		{
			eWhoFrom = (PlayerTypes) gDLL->getDiplomacyPlayer();
		}
		else if (gDLL->isMPDiplomacyScreenUp())
		{
			eWhoFrom = (PlayerTypes) gDLL->getMPDiplomacyPlayer();
		}
		eWhoTo = GC.getGameINLINE().getActivePlayer();
	}
	else
	{
		eWhoFrom = GC.getGameINLINE().getActivePlayer();
		if ( gDLL->isDiplomacy() )
		{
			eWhoTo = (PlayerTypes) gDLL->getDiplomacyPlayer();
		}
		else if (gDLL->isMPDiplomacyScreenUp())
		{
			eWhoTo = (PlayerTypes) gDLL->getMPDiplomacyPlayer();
		}
	}

	eWhoDenies = eWhoFrom;

	if ((eWhoFrom != NO_PLAYER) && (eWhoTo != NO_PLAYER))
	{
		//	Data1 is the heading
		switch (widgetDataStruct.m_iData1)
		{
		case TRADE_TECHNOLOGIES:
			GAMETEXT.setTechHelp(szBuffer, ((TechTypes)widgetDataStruct.m_iData2));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_RESOURCES:
			GAMETEXT.setBonusHelp(szBuffer, ((BonusTypes)widgetDataStruct.m_iData2));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_CITIES:
			szBuffer.assign(gDLL->getText("TXT_KEY_TRADE_CITIES"));
			eWhoDenies = (widgetDataStruct.m_bOption ? eWhoFrom : eWhoTo);
			break;
		case TRADE_PEACE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAKE_PEACE", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_WAR:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAKE_WAR", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_EMBARGO:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_STOP_TRADING", GET_TEAM(GET_PLAYER(eWhoFrom).getTeam()).getName().GetCString(), GET_TEAM((TeamTypes)widgetDataStruct.m_iData2).getName().GetCString()));
			break;
		case TRADE_CIVIC:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ADOPT_CIVIC", GC.getCivicInfo((CivicTypes)widgetDataStruct.m_iData2).getDescription()));
			break;
		case TRADE_RELIGION:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_CONVERT_RELIGION", GC.getReligionInfo((ReligionTypes)widgetDataStruct.m_iData2).getDescription()));
			break;
		case TRADE_GOLD:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_GOLD"));
			break;
		case TRADE_GOLD_PER_TURN:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_GOLD_PER_TURN"));
			break;
		case TRADE_MAPS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_MAPS"));
			break;
		case TRADE_SURRENDER:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_CAPITULATE"));
			break;
		case TRADE_VASSAL:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_VASSAL"));
			break;
		case TRADE_OPEN_BORDERS:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_OPEN_BORDERS"));
			break;
		case TRADE_DEFENSIVE_PACT:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_DEFENSIVE_PACT"));
			break;
		case TRADE_PERMANENT_ALLIANCE:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_PERMANENT_ALLIANCE"));
			break;
		case TRADE_PEACE_TREATY:
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_PEACE_TREATY", GC.getDefineINT("PEACE_TREATY_LENGTH")));
			break;
		}

		setTradeItem(&item, ((TradeableItems)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);

		eDenial = GET_PLAYER(eWhoFrom).getTradeDenial(eWhoTo, item);

		if (eDenial != NO_DENIAL)
		{
			szTempBuffer.Format(L"%s: " SETCOLR L"%s" ENDCOLR, GET_PLAYER(eWhoDenies).getName(), TEXT_COLOR("COLOR_WARNING_TEXT"), GC.getDenialInfo(eDenial).getDescription());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
	}
}


void CvDLLWidgetData::parseUnitModelHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pHeadSelectedUnit;

	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedUnit != NULL)
	{
		GAMETEXT.setUnitHelp(szBuffer, pHeadSelectedUnit);
	}
}


void CvDLLWidgetData::parseFlagHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szTempBuffer;

//>>>>	BUGFfH: Modified by Denev 2009/10/03
//	szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getDescription());
//	szBuffer.append(szTempBuffer);
	szBuffer.append(GAMETEXT.getLinkedText(GC.getGameINLINE().getActiveCivilizationType()));
//<<<<	BUGFfH: End Modify
	szBuffer.append(NEWLINE);

	GAMETEXT.parseCivilizationAbilities(szBuffer, GC.getGameINLINE().getActivePlayer());
	szBuffer.append(NEWLINE);

//FfH: Modified by Kael 08/08/2007
//	GAMETEXT.parseLeaderTraits(szBuffer, GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getLeaderType(), GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType());
	GAMETEXT.parseLeaderTraits(szBuffer, GC.getGameINLINE().getActivePlayer());
//FfH: End Modify

}


void CvDLLWidgetData::parseMaintenanceHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
//	wchar szTempBuffer[1024];
	int iMaintenanceValue;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->isWeLoveTheKingDay())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_WE_LOVE_KING_MAINT"));
		}
		else
		{
			//		szBuffer = "Maintenance represents the total cost of governing this city.\n";
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_MAINT_INFO"));
			szBuffer.append(NEWLINE);

			iMaintenanceValue = pHeadSelectedCity->calculateDistanceMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				//			swprintf(szTempBuffer, "\n%s%d%c: %s", ((iMaintenanceValue > 0) ?  "+" : ""), iMaintenanceValue, GC.getCommerceInfo(COMMERCE_GOLD).getChar(), ((GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getNumGovernmentCenters() > 0) ? "Distance from Palace" : "No Palace Penalty"));
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_NUM_MAINT_FLOAT", szMaint.GetCString()) + ((GET_PLAYER(pHeadSelectedCity->getOwnerINLINE()).getNumGovernmentCenters() > 0) ? gDLL->getText("TXT_KEY_MISC_DISTANCE_FROM_PALACE") : gDLL->getText("TXT_KEY_MISC_NO_PALACE_PENALTY")));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateNumCitiesMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				//			swprintf(szTempBuffer, "\n%s%d%c: Number of Cities", ((iMaintenanceValue > 0) ? "+" : ""), iMaintenanceValue, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_NUM_CITIES_FLOAT", szMaint.GetCString()));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateColonyMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_COLONY_MAINT_FLOAT", szMaint.GetCString()));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateCorporationMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_CORPORATION_MAINT_FLOAT", szMaint.GetCString()));
			}

			iMaintenanceValue = pHeadSelectedCity->calculateBuildingMaintenanceTimes100();
			if (iMaintenanceValue != 0)
			{
				CvWString szMaint = CvWString::format(L"%d.%02d", iMaintenanceValue/100, iMaintenanceValue%100);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_BUILDING_MAINT_FLOAT", szMaint.GetCString()));
			}

			szBuffer.append(SEPARATOR);

			iMaintenanceValue = pHeadSelectedCity->getMaintenanceModifier();

			if (iMaintenanceValue != 0)
			{
//>>>>BUGFfH: Added by Denev 2009/09/29
				// Buildings
				int iBuildingMod = 0;
				for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); iBuilding++)
				{
					CvBuildingInfo& pBuilding = GC.getBuildingInfo((BuildingTypes)iBuilding);
					if (pHeadSelectedCity->getNumBuilding((BuildingTypes)iBuilding) > 0 && !GET_TEAM(pHeadSelectedCity->getTeam()).isObsoleteBuilding((BuildingTypes)iBuilding))
					{
						for (int iLoop = 0; iLoop < pHeadSelectedCity->getNumBuilding((BuildingTypes)iBuilding); iLoop++)
						{
							iBuildingMod += pBuilding.getMaintenanceModifier();
						}
					}
				}
				if (0 != iBuildingMod)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BUILDINGS", iBuildingMod, GC.getCommerceInfo(COMMERCE_GOLD).getChar()));
				}

				// Resources
				int iBonusMod = 0;
				for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
				{
					if (pHeadSelectedCity->hasBonus((BonusTypes)iBonus))
					{
						iBonusMod += GC.getBonusInfo((BonusTypes)iBonus).getMaintenanceModifier();
					}
				}
				if (0 != iBonusMod)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BONUS", iBonusMod, GC.getCommerceInfo(COMMERCE_GOLD).getChar()));
				}

//<<<<BUGFfH: End Add

//				swprintf(szTempBuffer, L" (%s%d%%)", ((iMaintenanceValue > 0) ? L"+" : L""), iMaintenanceValue);
//				szBuffer.append(szTempBuffer);
			}

			//		swprintf(szTempBuffer, "\n%d%c Total Maintenance", pHeadSelectedCity->getMaintenance(), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
			CvWString szMaint = CvWString::format(L"%d.%02d", pHeadSelectedCity->getMaintenanceTimes100()/100, pHeadSelectedCity->getMaintenanceTimes100()%100);
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_TOTAL_MAINT_FLOAT", szMaint.GetCString()));
		}
	}
}

void CvDLLWidgetData::parseGoldHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	int iGoldFromCities = kPlayer.getCommerceRate(COMMERCE_GOLD);
	if (iGoldFromCities != 0) {
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GOLD_FROM_CITIES", iGoldFromCities));
	}

	int iGoldFromDeals = kPlayer.getGoldPerTurn();
	if (iGoldFromDeals != 0) {
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GOLD_FROM_DEALS", iGoldFromDeals));
	}
	szBuffer.append(NEWLINE);

	int iCityMaintenance = kPlayer.getTotalMaintenance();
	if (iCityMaintenance != 0) {
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GOLD_MAINTENANCE", -iCityMaintenance));
	}

	int iCivicMaintenance = kPlayer.getCivicUpkeep();
	if (iCivicMaintenance != 0) {
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GOLD_CIVIC_UPKEEP", -iCivicMaintenance));
	}

}

void CvDLLWidgetData::parseGlobalCultureHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CULTURE"));
}



void CvDLLWidgetData::parseCitySpecializationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvWString szMaint;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		bool bCtrl = gDLL->ctrlKey();
		CvPlayer &kPlayer=GET_PLAYER(GC.getGameINLINE().getActivePlayer());

		if(bCtrl)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_DETAILS"));
			return;
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_MAIN"));
		szBuffer.append(NEWLINE);
		szBuffer.append(NEWLINE);
		for(int iI=0;iI<NUM_CITYSPECIALIZATION_TYPES;iI++)
		{
			if(pHeadSelectedCity->getCitySpecializationLevel(iI)==0 && pHeadSelectedCity->getCitySpecialization(iI)==0 && pHeadSelectedCity->getCitySpecializationIncome(iI)==0)
				continue;
			szMaint = CvWString::format(L"%d: %d/%d +(%d/Turn)",pHeadSelectedCity->getCitySpecializationLevel(iI),pHeadSelectedCity->getCitySpecialization(iI),pHeadSelectedCity->getCitySpecializationThreshold(iI),pHeadSelectedCity->getCitySpecializationIncome(iI));
			switch(iI)
			{
				case 0:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_GOLD", szMaint.GetCString()));
					break;
				case 1:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_RESEARCH", szMaint.GetCString()));
					break;
				case 2:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_CULTURE", szMaint.GetCString()));
					break;
				case 3:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_ARCANE", szMaint.GetCString()));
					break;
				case 4:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_MILITARY", szMaint.GetCString()));
					break;
				case 5:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_METAL", szMaint.GetCString()));
					break;
				case 6:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_LUMBER", szMaint.GetCString()));
					break;
				case 7:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_LEATHER", szMaint.GetCString()));
					break;
				case 8:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_STONE", szMaint.GetCString()));
					break;
				case 9:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_FAITH", szMaint.GetCString()));					
					break;
				default:
					break;
			}
			szBuffer.append(NEWLINE);
		}
		bool bFirst=false;
		for(int iI=0;iI<NUM_CITYSPECIALIZATION_TYPES;iI++)
		{
			if(pHeadSelectedCity->getCitySpecializationLevel(iI)==0)
				continue;

			if(!bFirst)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_BONUSES_HELP"));
				szBuffer.append(NEWLINE);

				bFirst=true;
			}

			switch(iI)
			{
				case 0:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_GOLD_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("GOLD_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 1:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_RESEARCH_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("RESEARCH_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 2:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_CULTURE_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("CULTURE_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 3:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_ARCANE_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("SPELLRESEARCH_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 4:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_MILITARY_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("FREE_EXPERIENCE_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 5:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_METAL_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("GLOBALYIELD_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 6:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_LUMBER_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("GLOBALYIELD_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 7:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_LEATHER_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("GLOBALYIELD_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 8:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_STONE_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("GLOBALYIELD_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				case 9:
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_FAITH_HELP",pHeadSelectedCity->getCitySpecializationLevel(iI)*GC.getDefineINT("FAITH_PER_SPECIALIZATION_LEVEL")));
					szBuffer.append(NEWLINE);
					break;
				default:
					break;
			}
		}

		szBuffer.append(NEWLINE);
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CITY_SPECIALIZATION_DETAILS_HELP"));
	}
}

void CvDLLWidgetData::parseCivCounterElohimHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szMaint;

	if (GC.getGameINLINE().getActivePlayer()!=NO_PLAYER)
	{
		CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_MAIN"));
		szBuffer.append(SEPARATOR);
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_BUILDINGS",kPlayer.CalculateCivCounter(0,0)/100));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_MANA",kPlayer.CalculateCivCounter(1,0)));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_CIVIC",kPlayer.CalculateCivCounter(2,0)));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_UF",kPlayer.CalculateCivCounter(3,0)));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_RELIGION",kPlayer.CalculateCivCounter(4,0)));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_SEALED",kPlayer.CalculateCivCounter(5,0)));
		szBuffer.append(NEWLINE);
	}
}

void CvDLLWidgetData::parseCivCounterScionsHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvWString szMaint;

	if (GC.getGameINLINE().getActivePlayer()!=NO_PLAYER)
	{
		CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_MAIN"));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CIVCOUNTER_ELOHIM_BUILDINGS",kPlayer.CalculateCivCounter(0,0)));
		szBuffer.append(NEWLINE);
	}
}

void CvDLLWidgetData::parseManaHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{

	CvPlayer &kPlayer=GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	int iManaBuildings=kPlayer.CalculateManaByBuildings();
	int iManaNodes=kPlayer.CalculateManaByNodes();
	int iManaSpellResearch=kPlayer.CalculateFromSpellResearch();
	int iChange=iManaBuildings+iManaNodes+iManaSpellResearch;
	if(iChange>0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_INCOME"));
		szBuffer.append(NEWLINE);
	}

	if(iManaBuildings!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_FROM_BUILDINGS", iManaBuildings));
		szBuffer.append(NEWLINE);
	}
	if(iManaNodes!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_FROM_MANA_NODES", iManaNodes));
		szBuffer.append(NEWLINE);
	}
	if(iManaSpellResearch!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_FROM_MANA_SPELLRESEARCH", iManaSpellResearch));
		szBuffer.append(NEWLINE);
	}

	if(iChange>0)
	{
		szBuffer.append(NEWLINE);
	}

	int iUpkeep=kPlayer.getManaUpkeepTotal();
	int iManaUpkeepGE=kPlayer.CalculateManaUpkeepGE();
	int iManaChanneled=kPlayer.getManaChanneledIntoRitual();

	if(iUpkeep!=0 || iManaUpkeepGE!=0 || iManaChanneled!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_COSTS"));
		szBuffer.append(NEWLINE);
	}

	if(iUpkeep!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_UPKEEP",-iUpkeep));
		szBuffer.append(NEWLINE);
		iChange-=iUpkeep;
	}

	if(iManaUpkeepGE!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_UPKEEP_GE",-iManaUpkeepGE));
		szBuffer.append(NEWLINE);
		iChange-=iManaUpkeepGE;
	}

	if(iManaChanneled!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_CHANNELED",-iManaChanneled));
		szBuffer.append(NEWLINE);
		iChange-=iManaChanneled;
	}

	int iMaximumMana=kPlayer.getMaxMana();
	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText("TXT_KEY_MISC_MANA_TOTAL_CHANGE", iChange,iMaximumMana));
	szBuffer.append(NEWLINE);

}

void CvDLLWidgetData::parseUnitLimitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvPlayer &kPlayer=GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	int iChange=GC.getDefineINT("BASE_UNIT_SUPPORT");
	int iTotal=iChange;

	if(iChange!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_UNITSUPPORTLIMIT_BASE",iChange));
		szBuffer.append(NEWLINE);
	}

	iChange=kPlayer.getUnitSupportLimit();
	iTotal+=iChange;

	if(iChange!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_UNITSUPPORTLIMIT_BUILDINGS",iChange));
		szBuffer.append(NEWLINE);
	}

	iChange=kPlayer.getFreeMilitaryUnitsPopulationPercent();

	if(iChange!=0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_UNITSUPPORTLIMIT_FROM_CIVICS",(iChange*iTotal)/100));
		szBuffer.append(NEWLINE);
	}

}

void CvDLLWidgetData::parseFaithHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	bool bCtrl = gDLL->ctrlKey();
	CvPlayer &kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	ReligionTypes eStateReligion = kPlayer.getStateReligion();

	CvWString szTempBuffer1;
	CvWString szTempBuffer2;
	if(bCtrl && 1 > 2)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_GENERAL"));
		szBuffer.append(NEWLINE);

		switch(eStateReligion)
		{
			case RELIGION_THE_ORDER:
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_ORDER"));
				break;
			case RELIGION_THE_ASHEN_VEIL:
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_VEIL"));
				break;
			case RELIGION_FELLOWSHIP_OF_LEAVES:
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_FOL"));
				break;
			case RELIGION_OCTOPUS_OVERLORDS:
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_OVERLORDS"));
				break;
			case RELIGION_THE_EMPYREAN:
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_EMPYREAN"));
				break;
			case RELIGION_RUNES_OF_KILMORPH:
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_KILMORPH"));
				break;

			default:
				break;
		}

		if(eStateReligion!=NO_RELIGION)
		{
//			if(!GC.getGame().isReligionFounded(eStateReligion))
			if(GC.getGame().getGameTurn()<GC.getGame().getHolyCitiesCreatedTurn())
			{
				int iProb=kPlayer.getFoundHolyCityProb(eStateReligion)*100;
				int iTotalValue=0;
				for(int iJ=0;iJ<MAX_CIV_PLAYERS;iJ++)
				{
					iTotalValue+=GET_PLAYER((PlayerTypes)iJ).getFoundHolyCityProb(eStateReligion);
				}
				if(iTotalValue!=0)
				{
					iProb/=iTotalValue;
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_HELP_FOUND_HOLY_CITY",GC.getGame().getHolyCitiesCreatedTurn()+1,iProb));
				}
			}
		}

		return;
	}


	int iBase = 0;
	int iFaithBuildings = kPlayer.calculateFaithByBuildings();
	int iFaithLT = kPlayer.calculateFaithLostTemples();
	int iFaithAF = kPlayer.calculateFaithAncientForests();
	int iFaithUpkeep = kPlayer.getFaithUpkeep();
	int iLengthString = 10;

	if(kPlayer.getFaithUpkeep() > 0)
	{
		iBase -= iFaithUpkeep;

		szTempBuffer1.clear();
		szTempBuffer1.Format(L"%d %c: ", -iFaithUpkeep, GC.getCommerceInfo(COMMERCE_FAITH).getChar());
		for(int i = szTempBuffer1.length(); i < iLengthString; ++i) {
			szBuffer.append(L" ");
		}
		szBuffer.append(szTempBuffer1);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_UPKEEP"));
		szBuffer.append(NEWLINE);
	}

	if((iFaithBuildings > 0) 
		|| (iFaithLT > 0)
		|| (iFaithAF > 0)
		|| (iFaithUpkeep < 0)) {
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_INCOME"));
		szBuffer.append(NEWLINE);

		if(iFaithBuildings != 0)
		{
			iBase += iFaithBuildings;

			szTempBuffer1.clear();
			szTempBuffer1.Format(L"+%d %c: ", iFaithBuildings, GC.getCommerceInfo(COMMERCE_FAITH).getChar());
			for(int i = szTempBuffer1.length(); i < iLengthString; ++i) {
				szBuffer.append(L" ");
			}
			szBuffer.append(szTempBuffer1);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_FROM_TEMPLES"));
			szBuffer.append(NEWLINE);
		}
		if(iFaithLT != 0)
		{
			iBase += iFaithLT;

			szTempBuffer1.clear();
			szTempBuffer1.Format(L"+%d %c: ", iFaithLT, GC.getCommerceInfo(COMMERCE_FAITH).getChar());
			for(int i = szTempBuffer1.length(); i < iLengthString; ++i) {
				szBuffer.append(L" ");
			}
			szBuffer.append(szTempBuffer1);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_FROM_LOST_TEMPLES"));
			szBuffer.append(NEWLINE);
		}
		if(iFaithAF != 0)
		{
			iBase += iFaithAF;

			szTempBuffer1.clear();
			szTempBuffer1.Format(L"+%d %c: ", iFaithAF, GC.getCommerceInfo(COMMERCE_FAITH).getChar());
			for(int i = szTempBuffer1.length(); i < iLengthString; ++i) {
				szBuffer.append(L" ");
			}
			szBuffer.append(szTempBuffer1);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_FROM_ANCIENT_FORESTS"));
			szBuffer.append(NEWLINE);
		}

		if(iFaithUpkeep < 0)
		{
			iBase -= iFaithUpkeep;

			szTempBuffer1.clear();
			szTempBuffer1.Format(L"+%d %c: ", -iFaithUpkeep, GC.getCommerceInfo(COMMERCE_FAITH).getChar());
			for(int i = szTempBuffer1.length(); i < iLengthString; ++i) {
				szBuffer.append(L" ");
			}
			szBuffer.append(szTempBuffer1);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_UNIT_INCOME"));
			szBuffer.append(NEWLINE);
		}

	}

	if(iBase > 0)
	{
		if(kPlayer.isFaithIncomeDoubled())
		{
			iBase += 20;

			szTempBuffer1.clear();
			szTempBuffer1.Format(L"+%d %c: ", 20, GC.getCommerceInfo(COMMERCE_FAITH).getChar());
			for(int i = szTempBuffer1.length(); i < iLengthString; ++i) {
				szBuffer.append(L" ");
			}
			szBuffer.append(szTempBuffer1);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_INCOME_DOUBLED"));
			szBuffer.append(NEWLINE);
		}
	}

	if(kPlayer.getStateReligion() != NO_RELIGION) {
		if(iBase > 1) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_SACRED_KNOWLEDGE"));
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_CONVERTED_TO_SACRED_KNOWLEDGE", iBase / 2, kPlayer.getTurnSacredKnowledge()));

			szBuffer.append(NEWLINE);
			szTempBuffer1.clear();
			szTempBuffer1.Format(L"%d %c: ", iBase / 2, GC.getCommerceInfo(COMMERCE_FAITH).getChar());
			szBuffer.append(szTempBuffer1);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FAITH_TOTAL_CHANGE"));
			szBuffer.append(NEWLINE);
		}
	}
}

void CvDLLWidgetData::parseGlobalYieldHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer, YieldTypes eYield)
{
	CvPlayer &kPlayer=GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	CvCity* pLoopCity;
	int iLoop;
	CvWString szTempBuffer;

	int iTotalIncome=0;
/**
	switch(eYield)
	{
		case YIELD_METAL:
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD_METAL_TOOLTIP", GC.getYieldInfo(eYield).getDescription()));
			szBuffer.append(NEWLINE);
			break;
		case YIELD_LUMBER:
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD_LUMBER_TOOLTIP", GC.getYieldInfo(eYield).getDescription()));
			szBuffer.append(NEWLINE);
			break;
		case YIELD_LEATHER:
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD_LEATHER_TOOLTIP", GC.getYieldInfo(eYield).getDescription()));
			szBuffer.append(NEWLINE);
			break;
		case YIELD_HERB:
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD_HERB_TOOLTIP", GC.getYieldInfo(eYield).getDescription()));
			szBuffer.append(NEWLINE);
			break;
		case YIELD_STONE:
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD_STONE_TOOLTIP", GC.getYieldInfo(eYield).getDescription()));
			szBuffer.append(NEWLINE);
			break;
		default:
			break;
	}
**/
	int iYieldNeeded=kPlayer.getGlobalYieldNeededForEquipmentBonus(eYield)-kPlayer.getGlobalYield(eYield);
	if(iYieldNeeded>0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_GLOBALYIELD_NEEDED_TOOLTIP", GC.getYieldInfo(eYield).getDescription(),iYieldNeeded));
		szBuffer.append(NEWLINE);
	}

	szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD_TOOLTIP", GC.getYieldInfo(eYield).getDescription()));

	for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
	{
		if(pLoopCity->getYieldIncome(eYield)>0)
		{
			szTempBuffer.Format(L"%c %d",GC.getYieldInfo(eYield).getChar(), pLoopCity->getYieldIncome(eYield));
			szBuffer.append(szTempBuffer);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD",pLoopCity->getNameKey()));
			szBuffer.append(NEWLINE);

			iTotalIncome+=pLoopCity->getYieldIncome(eYield);
		}
	}
/**
	int iUpkeep=kPlayer.getGlobalYieldUpkeep(eYield);
	if(iUpkeep!=0)
	{
		szBuffer.append(NEWLINE);
		szTempBuffer.Format(L"%c %d",GC.getYieldInfo(eYield).getChar(), -iUpkeep);
		szBuffer.append(szTempBuffer);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_UPKEEP_GLOBALYIELD"));
		iTotalIncome-=iUpkeep;
	}
	szBuffer.append(NEWLINE);
	szTempBuffer.Format(L"%c %d",GC.getYieldInfo(eYield).getChar(), iTotalIncome);
	szBuffer.append(szTempBuffer);
	szBuffer.append(gDLL->getText("TXT_KEY_MISC_CITY_PRODUCE_GLOBALYIELD_TOTAL"));
	szBuffer.append(NEWLINE);
**/
}

void CvDLLWidgetData::parseHealthHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pHeadSelectedCity)
	{
		GAMETEXT.setBadHealthHelp(szBuffer, *pHeadSelectedCity);
		szBuffer.append(L"\n=======================\n");
		GAMETEXT.setGoodHealthHelp(szBuffer, *pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseNationalityHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	wchar szTempBuffer[1024];
	CvCity* pHeadSelectedCity;
	PlayerTypes eCulturalOwner;
	int iCulturePercent;
	int iCityStrength;
	int iGarrison;
	int iI;

	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CITY_NATIONALITY"));

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				iCulturePercent = pHeadSelectedCity->plot()->calculateCulturePercent((PlayerTypes)iI);

				if (iCulturePercent > 0)
				{
					swprintf(szTempBuffer, L"\n%d%% " SETCOLR L"%s" ENDCOLR, iCulturePercent, GET_PLAYER((PlayerTypes)iI).getPlayerTextColorR(), GET_PLAYER((PlayerTypes)iI).getPlayerTextColorG(), GET_PLAYER((PlayerTypes)iI).getPlayerTextColorB(), GET_PLAYER((PlayerTypes)iI).getPlayerTextColorA(), GET_PLAYER((PlayerTypes)iI).getCivilizationAdjective());
					szBuffer.append(szTempBuffer);
				}
			}
		}

		eCulturalOwner = pHeadSelectedCity->plot()->calculateCulturalOwner();

		if (eCulturalOwner != NO_PLAYER)
		{
			if (GET_PLAYER(eCulturalOwner).getTeam() != pHeadSelectedCity->getTeam())
			{
				iCityStrength = pHeadSelectedCity->cultureStrength(eCulturalOwner);
				iGarrison = pHeadSelectedCity->cultureGarrison(eCulturalOwner);

				if (iCityStrength > iGarrison)
				{
					swprintf(szTempBuffer, L"%.2f", std::max(0.0f, (1.0f - (((float)iGarrison) / ((float)iCityStrength)))) * ((float)(std::min(100.0f, ((float)pHeadSelectedCity->getRevoltTestProbability())))));
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_CHANCE_OF_REVOLT", szTempBuffer));
				}
			}
		}
	}
}


void CvDLLWidgetData::parseHappinessHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		GAMETEXT.setAngerHelp(szBuffer, *pHeadSelectedCity);
		szBuffer.append(L"\n=======================\n");
		GAMETEXT.setHappyHelp(szBuffer, *pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parsePopulationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		szBuffer.assign(gDLL->getText("TXT_KEY_MISC_FOOD_THRESHOLD", pHeadSelectedCity->getFood(), pHeadSelectedCity->growthThreshold()));
	}
}


void CvDLLWidgetData::parseProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		if (pHeadSelectedCity->getProductionNeeded() != MAX_INT)
		{
			CvWString szTemp;
			szTemp.Format(L"%s: %d/%d %c", pHeadSelectedCity->getProductionName(), pHeadSelectedCity->getProduction(), pHeadSelectedCity->getProductionNeeded(), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			szBuffer.assign(szTemp);
		}
	}
}


void CvDLLWidgetData::parseCultureHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();

	if (pHeadSelectedCity != NULL)
	{
		int iCultureTimes100 = pHeadSelectedCity->getCultureTimes100(pHeadSelectedCity->getOwnerINLINE());
		if (iCultureTimes100%100 == 0)
		{
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CULTURE", iCultureTimes100/100, pHeadSelectedCity->getCultureThreshold()));
		}
		else
		{
			CvWString szCulture = CvWString::format(L"%d.%02d", iCultureTimes100/100, iCultureTimes100%100);
			szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CULTURE_FLOAT", szCulture.GetCString(), pHeadSelectedCity->getCultureThreshold()));
		}

		int iCultureRateTimes100 = pHeadSelectedCity->getCommerceRateTimes100(COMMERCE_CULTURE);
		if (iCultureRateTimes100 > 0)
		{
			int iCultureLeftTimes100 = 100 * pHeadSelectedCity->getCultureThreshold() - iCultureTimes100;

			if (iCultureLeftTimes100 > 0)
			{
				int iTurnsLeft = (iCultureLeftTimes100  + iCultureRateTimes100 - 1) / iCultureRateTimes100;

				szBuffer.append(L' ');
				szBuffer.append(gDLL->getText("INTERFACE_CITY_TURNS", std::max(1, iTurnsLeft)));
			}
		}


		szBuffer.append(L"\n=======================\n");
		GAMETEXT.setCommerceHelp(szBuffer, *pHeadSelectedCity, COMMERCE_CULTURE);
	}
}


void CvDLLWidgetData::parseGreatPeopleHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pHeadSelectedCity)
	{
		GAMETEXT.parseGreatPeopleHelp(szBuffer, *pHeadSelectedCity);
	}
}


void CvDLLWidgetData::parseGreatGeneralHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (NO_PLAYER != GC.getGame().getActivePlayer())
	{
		GAMETEXT.parseGreatGeneralHelp(szBuffer, GET_PLAYER(GC.getGame().getActivePlayer()));
	}
}


void CvDLLWidgetData::parseSelectedHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pHeadSelectedCity;
	CvUnit* pHeadSelectedUnit;
	OrderData* pOrder;

	pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (pHeadSelectedCity != NULL)
	{
		pOrder = pHeadSelectedCity->getOrderFromQueue(widgetDataStruct.m_iData1);

		if (pOrder != NULL)
		{
			switch (pOrder->eOrderType)
			{
			case ORDER_TRAIN:
				GAMETEXT.setUnitHelp(szBuffer, ((UnitTypes)(pOrder->iData1)), false, false, false, pHeadSelectedCity);
				break;

			case ORDER_CONSTRUCT:
				GAMETEXT.setBuildingHelp(szBuffer, ((BuildingTypes)(pOrder->iData1)), false, false, false, pHeadSelectedCity);
				break;

			case ORDER_CREATE:
				GAMETEXT.setProjectHelp(szBuffer, ((ProjectTypes)(pOrder->iData1)), false, pHeadSelectedCity);
				break;

			case ORDER_MAINTAIN:
				GAMETEXT.setProcessHelp(szBuffer, ((ProcessTypes)(pOrder->iData1)));
				break;

			default:
				FAssertMsg(false, "eOrderType did not match valid options");
				break;
			}
		}
	}
}


void CvDLLWidgetData::parseTradeRouteCityHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setTradeRouteHelp(szBuffer, widgetDataStruct.m_iData1, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parseEspionageCostHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvUnit* pUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
	if (NULL != pUnit)
	{
		CvPlot* pPlot = pUnit->plot();
		if (NULL != pPlot)
		{
			GAMETEXT.setEspionageCostHelp(szBuffer, (EspionageMissionTypes)widgetDataStruct.m_iData1, pPlot->getOwnerINLINE(), pPlot, widgetDataStruct.m_iData2, pUnit);
		}
	}
}

void CvDLLWidgetData::parseBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setBuildingHelp(szBuffer, ((BuildingTypes)(widgetDataStruct.m_iData1)), false, false, widgetDataStruct.m_bOption, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parseProjectHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setProjectHelp(szBuffer, ((ProjectTypes)widgetDataStruct.m_iData1), false, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}


void CvDLLWidgetData::parseTerrainHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setTerrainHelp(szBuffer, (TerrainTypes)widgetDataStruct.m_iData1);
	}
}


void CvDLLWidgetData::parseFeatureHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setFeatureHelp(szBuffer, (FeatureTypes)widgetDataStruct.m_iData1);
	}
}


void CvDLLWidgetData::parseTechEntryHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setTechHelp(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
	}
}


void CvDLLWidgetData::parseTechPrereqHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer.Format(L"%cThis technology requires the knowledge of %s", gDLL->getSymbolID(BULLET_CHAR), GC.getTechInfo((TechTypes) widgetDataStruct.m_iData1).getDescription());
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_TECH_REQUIRES_KNOWLEDGE_OF", GC.getTechInfo((TechTypes) widgetDataStruct.m_iData1).getTextKeyWide()));
}

void CvDLLWidgetData::parseTechTreePrereq(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer, bool bTreeInfo)
{
	GAMETEXT.setTechHelp(szBuffer, (TechTypes)widgetDataStruct.m_iData1, false, false, false, bTreeInfo, (TechTypes)widgetDataStruct.m_iData2);
}


void CvDLLWidgetData::parseObsoleteHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoleteString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseObsoleteBonusString(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoleteBonusString(szBuffer, ((BonusTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseObsoleteSpecialHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildObsoleteSpecialString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMoveHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMoveString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseFreeUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFreeUnitString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData2)));
}

void CvDLLWidgetData::parseFeatureProductionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFeatureProductionString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseWorkerRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildWorkerRateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseTradeRouteHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildTradeRouteString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseHealthRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildHealthRateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseHappinessRateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildHappinessRateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseFreeTechHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFreeTechString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseLOSHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildLOSString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMapCenterHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMapCenterString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMapRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMapRevealString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseMapTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildMapTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseTechTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildTechTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseGoldTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildGoldTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseOpenBordersHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildOpenBordersString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseDefensivePactHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildDefensivePactString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parsePermanentAllianceHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildPermanentAllianceString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseVassalStateHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildVassalStateString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseBuildBridgeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildBridgeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseIrrigationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildIrrigationString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseIgnoreIrrigationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildIgnoreIrrigationString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseWaterWorkHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildWaterWorkString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
}

void CvDLLWidgetData::parseBuildHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildImprovementString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseDomainExtraMovesHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildDomainExtraMovesString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseAdjustHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildAdjustString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseTerrainTradeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 < GC.getNumTerrainInfos())
	{
		GAMETEXT.buildTerrainTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
	}
	else if (widgetDataStruct.m_iData2 == GC.getNumTerrainInfos())
	{
		GAMETEXT.buildRiverTradeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)));
	}
}

void CvDLLWidgetData::parseSpecialBuildingHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildSpecialBuildingString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseYieldChangeHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildYieldChangeString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, false);
}

void CvDLLWidgetData::parseBonusRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildBonusRevealString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseCivicRevealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildCivicRevealString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseProcessInfoHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildProcessInfoString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseFoundReligionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFoundReligionString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseFoundCorporationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.buildFoundCorporationString(szBuffer, ((TechTypes)(widgetDataStruct.m_iData1)), widgetDataStruct.m_iData2, true);
}

void CvDLLWidgetData::parseFinanceNumUnits(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Number of units you are currently supporting";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_NUM_UNITS_SUPPORTING"));
}

void CvDLLWidgetData::parseFinanceUnitCost(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent on unit upkeep";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_MONEY_SPENT_UPKEEP"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceUnitCostString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceAwaySupply(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent on units in enemy territory";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_UNITS_ENEMY_TERRITORY"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceAwaySupplyString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceCityMaint(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent doing city maintenance";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_CITY_MAINT"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceCityMaintString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceCivicUpkeep(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "The amount of money spent on Civics";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_CIVIC_UPKEEP"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceCivicUpkeepString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceForeignIncome(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_FOREIGN"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceForeignIncomeString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceInflatedCosts(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_AMOUNT_MONEY_AFTER_INFLATION"));
	if (widgetDataStruct.m_iData2 > 0)
	{
		GAMETEXT.buildFinanceInflationString(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseFinanceGrossIncome(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Your gross income";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_GROSS_INCOME"));
}

void CvDLLWidgetData::parseFinanceNetGold(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Net Gold per turn";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_NET_GOLD"));
}

void CvDLLWidgetData::parseFinanceGoldReserve(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Amount of money in your gold reserves";
	szBuffer.assign(gDLL->getText("TXT_KEY_ECON_GOLD_RESERVE"));
}

void CvDLLWidgetData::parseUnitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setUnitHelp(szBuffer, (UnitTypes)widgetDataStruct.m_iData1, false, false, widgetDataStruct.m_bOption, gDLL->getInterfaceIFace()->getHeadSelectedCity());
	}
}

void CvDLLWidgetData::parsePediaBack(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Back";
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_PEDIA_BACK"));
}

void CvDLLWidgetData::parsePediaForward(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Forward";
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_PEDIA_FORWARD"));
}

void CvDLLWidgetData::parseBonusHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setBonusHelp(szBuffer, (BonusTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseReligionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setReligionHelp(szBuffer, (ReligionTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseReligionHelpCity( CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer )
{
	GAMETEXT.setReligionHelpCity(szBuffer, (ReligionTypes)widgetDataStruct.m_iData1, gDLL->getInterfaceIFace()->getHeadSelectedCity(), true);
}

void CvDLLWidgetData::parseCorporationHelpCity( CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer )
{
	GAMETEXT.setCorporationHelpCity(szBuffer, (CorporationTypes)widgetDataStruct.m_iData1, gDLL->getInterfaceIFace()->getHeadSelectedCity(), true);
}

void CvDLLWidgetData::parseCorporationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setCorporationHelp(szBuffer, (CorporationTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parsePromotionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setPromotionHelp(szBuffer, (PromotionTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseEquipmentHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseEquipmentHelp(szBuffer, (PromotionTypes)widgetDataStruct.m_iData1);
	}
}

//FfH: Added by Kael 07/23/2007
void CvDLLWidgetData::parseSpellHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setSpellHelp(szBuffer, (SpellTypes)widgetDataStruct.m_iData1);
	}
}
//FfH: End Add

void CvDLLWidgetData::parseEventHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.setEventHelp(szBuffer, (EventTypes)widgetDataStruct.m_iData1, widgetDataStruct.m_iData2, GC.getGameINLINE().getActivePlayer());
}

void CvDLLWidgetData::parseUnitCombatHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setUnitCombatHelp(szBuffer, (UnitCombatTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseImprovementHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.setImprovementHelp(szBuffer, (ImprovementTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseCivicHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseCivicInfo(szBuffer, (CivicTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseCivilizationHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseCivInfos(szBuffer, (CivilizationTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseLeaderHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != -1)
	{
		GAMETEXT.parseLeaderTraits(szBuffer, (LeaderHeadTypes)widgetDataStruct.m_iData1, (CivilizationTypes)widgetDataStruct.m_iData2);
	}
}

void CvDLLWidgetData::parseCloseScreenHelp(CvWStringBuffer& szBuffer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CLOSE_SCREEN"));
}

void CvDLLWidgetData::parseDescriptionHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer, bool bMinimal)
{
	CivilopediaPageTypes eType = (CivilopediaPageTypes)widgetDataStruct.m_iData1;
	switch (eType)
	{
	case CIVILOPEDIA_PAGE_TECH:
		{
			TechTypes eTech = (TechTypes)widgetDataStruct.m_iData2;
			if (NO_TECH != eTech)
			{
				szBuffer.assign(bMinimal ? GC.getTechInfo(eTech).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getTechInfo(eTech).getTextKeyWide()));
			}
		}
		break;

//FfH: Added by Kael 07/23/2007
	case CIVILOPEDIA_PAGE_HERO:
//FfH: End Add

	case CIVILOPEDIA_PAGE_UNIT:
		{
			UnitTypes eUnit = (UnitTypes)widgetDataStruct.m_iData2;
			if (NO_UNIT != eUnit)
			{
				szBuffer.assign(bMinimal ? GC.getUnitInfo(eUnit).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getUnitInfo(eUnit).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_BUILDING:
	case CIVILOPEDIA_PAGE_WONDER:
		{
			BuildingTypes eBuilding = (BuildingTypes)widgetDataStruct.m_iData2;
			if (NO_BUILDING != eBuilding)
			{
				szBuffer.assign(bMinimal ? GC.getBuildingInfo(eBuilding).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getBuildingInfo(eBuilding).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_BONUS:
		{
			BonusTypes eBonus = (BonusTypes)widgetDataStruct.m_iData2;
			if (NO_BONUS != eBonus)
			{
				szBuffer.assign(bMinimal ? GC.getBonusInfo(eBonus).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getBonusInfo(eBonus).getTextKeyWide()));
			}
		}
		break;

//FfH: Added by Kael 08/26/2008
	case CIVILOPEDIA_PAGE_UNIQUE_FEATURE:
//FfH: End Add

	case CIVILOPEDIA_PAGE_IMPROVEMENT:
		{
			ImprovementTypes eImprovement = (ImprovementTypes)widgetDataStruct.m_iData2;
			if (NO_IMPROVEMENT != eImprovement)
			{
				szBuffer.assign(bMinimal ? GC.getImprovementInfo(eImprovement).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getImprovementInfo(eImprovement).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_UNIT_GROUP:
		{
			UnitCombatTypes eGroup = (UnitCombatTypes)widgetDataStruct.m_iData2;
			if (NO_UNITCOMBAT != eGroup)
			{
				szBuffer.assign(bMinimal ? GC.getUnitCombatInfo(eGroup).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getUnitCombatInfo(eGroup).getTextKeyWide()));
			}
		}
		break;

//FfH: Added by Kael 08/26/2008
	case CIVILOPEDIA_PAGE_EFFECT:
//>>>>BUGFfH: Modified by Denev 2009/10/14
//	case CIVILOPEDIA_PAGE_ITEM:
	case CIVILOPEDIA_PAGE_EQUIPMENT:
//>>>>BUGFfH: End Modifiy
	case CIVILOPEDIA_PAGE_RACE:
//FfH: End Add

	case CIVILOPEDIA_PAGE_PROMOTION:
		{
			PromotionTypes ePromo = (PromotionTypes)widgetDataStruct.m_iData2;
			if (NO_PROMOTION != ePromo)
			{
				szBuffer.assign(bMinimal ? GC.getPromotionInfo(ePromo).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getPromotionInfo(ePromo).getTextKeyWide()));
			}
		}
		break;

//FfH: Added by Kael 07/23/2007
	case CIVILOPEDIA_PAGE_SPELL:
		{
			SpellTypes eSpell = (SpellTypes)widgetDataStruct.m_iData2;
			if (NO_SPELL != eSpell)
			{
				szBuffer.assign(bMinimal ? GC.getSpellInfo(eSpell).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getSpellInfo(eSpell).getTextKeyWide()));
			}
		}
		break;
//FfH: End Add

	case CIVILOPEDIA_PAGE_CIV:
		{
			CivilizationTypes eCiv = (CivilizationTypes)widgetDataStruct.m_iData2;
			if (NO_CIVILIZATION != eCiv)
			{
				szBuffer.assign(bMinimal ? GC.getCivilizationInfo(eCiv).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCivilizationInfo(eCiv).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_LEADER:
		{
			LeaderHeadTypes eLeader = (LeaderHeadTypes)widgetDataStruct.m_iData2;
			if (NO_LEADER != eLeader)
			{
				szBuffer.assign(bMinimal ? GC.getLeaderHeadInfo(eLeader).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getLeaderHeadInfo(eLeader).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_RELIGION:
		{
			ReligionTypes eReligion = (ReligionTypes)widgetDataStruct.m_iData2;
			if (NO_RELIGION != eReligion)
			{
				szBuffer.assign(bMinimal ? GC.getReligionInfo(eReligion).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getReligionInfo(eReligion).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CORPORATION:
		{
			CorporationTypes eCorporation = (CorporationTypes)widgetDataStruct.m_iData2;
			if (NO_CORPORATION != eCorporation)
			{
				szBuffer.assign(bMinimal ? GC.getCorporationInfo(eCorporation).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCorporationInfo(eCorporation).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CIVIC:
		{
			CivicTypes eCivic = (CivicTypes)widgetDataStruct.m_iData2;
			if (NO_CIVIC != eCivic)
			{
				szBuffer.assign(bMinimal ? GC.getCivicInfo(eCivic).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getCivicInfo(eCivic).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_PROJECT:
		{
			ProjectTypes eProject = (ProjectTypes)widgetDataStruct.m_iData2;
			if (NO_PROJECT != eProject)
			{
				szBuffer.assign(bMinimal ? GC.getProjectInfo(eProject).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getProjectInfo(eProject).getTextKeyWide()));
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CONCEPT:
		{
			ConceptTypes eConcept = (ConceptTypes)widgetDataStruct.m_iData2;
			if (NO_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getConceptInfo(eConcept).getDescription());
			}
		}
		break;
	case CIVILOPEDIA_PAGE_CONCEPT_NEW:
		{
			NewConceptTypes eConcept = (NewConceptTypes)widgetDataStruct.m_iData2;
			if (NO_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getNewConceptInfo(eConcept).getDescription());
			}
		}
		break;
/*************************************************************************************************/
/**	ADDON (New Pedia Categories) Sephi                                      					**/
/*************************************************************************************************/
	case CIVILOPEDIA_PAGE_CONCEPT_WILDMANA:
		{
			WildmanaConceptTypes eConcept = (WildmanaConceptTypes)widgetDataStruct.m_iData2;
			if (NO_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getWildmanaConceptInfo(eConcept).getDescription());
			}
		}
		break;
	case CIVILOPEDIA_PAGE_WILDMANA_GUIDE:
		{
			WildmanaGuideTypes eConcept = (WildmanaGuideTypes)widgetDataStruct.m_iData2;
			if (NO_CONCEPT != eConcept)
			{
				szBuffer.assign(GC.getWildmanaGuideInfo(eConcept).getDescription());
			}
		}
		break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	case CIVILOPEDIA_PAGE_SPECIALIST:
		{
			SpecialistTypes eSpecialist = (SpecialistTypes)widgetDataStruct.m_iData2;
			if (NO_SPECIALIST != eSpecialist)
			{
				szBuffer.assign(bMinimal ? GC.getSpecialistInfo(eSpecialist).getDescription() : gDLL->getText("TXT_KEY_MISC_HISTORICAL_INFO", GC.getSpecialistInfo(eSpecialist).getTextKeyWide()));
			}
		}
		break;
	default:
		break;
	}
}

void CvDLLWidgetData::parseKillDealHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
//	szBuffer = "Click to cancel";
	CvWString szTemp;
	szTemp = szBuffer.getCString();
	CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
	if (NULL != pDeal)
	{
		if (pDeal->isCancelable(GC.getGameINLINE().getActivePlayer(), &szTemp))
		{
			szTemp = gDLL->getText("TXT_KEY_MISC_CLICK_TO_CANCEL");
		}
	}

	szBuffer.assign(szTemp);
}


void CvDLLWidgetData::doDealKill(CvWidgetDataStruct &widgetDataStruct)
{
	CvDeal* pDeal = GC.getGameINLINE().getDeal(widgetDataStruct.m_iData1);
	if (pDeal != NULL)
	{
		if (!pDeal->isCancelable(GC.getGameINLINE().getActivePlayer()))
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_TEXT);
			if (NULL != pInfo)
			{
				pInfo->setText(gDLL->getText("TXT_KEY_POPUP_CANNOT_CANCEL_DEAL"));
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
		else
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DEAL_CANCELED);
			if (NULL != pInfo)
			{
				pInfo->setData1(pDeal->getID());
				pInfo->setOption1(false);
				gDLL->getInterfaceIFace()->addPopup(pInfo, GC.getGameINLINE().getActivePlayer(), true);
			}
		}
	}
}


void CvDLLWidgetData::doRefreshMilitaryAdvisor(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	argsList.add(widgetDataStruct.m_iData2);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "refreshMilitaryAdvisor", argsList.makeFunctionArgs());
}

void CvDLLWidgetData::parseProductionModHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pCity)
	{
		GAMETEXT.setProductionHelp(szBuffer, *pCity);
	}
}

void CvDLLWidgetData::parseLeaderheadHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.parseLeaderHeadHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseLeaderLineHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	GAMETEXT.parseLeaderLineHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1, (PlayerTypes)widgetDataStruct.m_iData2);
}

void CvDLLWidgetData::parseCommerceModHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (NULL != pCity)
	{
		GAMETEXT.setCommerceHelp(szBuffer, *pCity, (CommerceTypes)widgetDataStruct.m_iData1);
	}
}

void CvDLLWidgetData::parseScoreHelp(CvWidgetDataStruct& widgetDataStruct, CvWStringBuffer& szBuffer)
{
	GAMETEXT.setScoreHelp(szBuffer, (PlayerTypes)widgetDataStruct.m_iData1);
}

//>>>>BUGFfH: Added by Denev 2009/09/10
void CvDLLWidgetData::doPediaTraitJump(CvWidgetDataStruct &widgetDataStruct)
{
	CyArgsList argsList;
	argsList.add(widgetDataStruct.m_iData1);
	gDLL->getPythonIFace()->callFunction(PYScreensModule, "pediaJumpToTrait", argsList.makeFunctionArgs());
}
//<<<<BUGFfH: End Add

//>>>>BUGFfH: Added by Denev 2009/10/03
void CvDLLWidgetData::parseSpecialistHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseSpecialistHelp(szBuffer, (SpecialistTypes)widgetDataStruct.m_iData1, NULL);
	}
}

void CvDLLWidgetData::parseTraitHelp(CvWidgetDataStruct &widgetDataStruct, CvWStringBuffer &szBuffer)
{
	if (widgetDataStruct.m_iData2 != 0)
	{
		GAMETEXT.parseTraits(szBuffer, (TraitTypes)widgetDataStruct.m_iData1);
	}
}
//<<<<BUGFfH: End Add
