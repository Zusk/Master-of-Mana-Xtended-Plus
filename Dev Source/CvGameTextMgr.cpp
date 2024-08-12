//---------------------------------------------------------------------------------------
//
//  *****************   Civilization IV   ********************
//
//  FILE:    CvGameTextMgr.cpp
//
//  PURPOSE: Interfaces with GameText XML Files to manage the paths of art files
//
//---------------------------------------------------------------------------------------
//  Copyright (c) 2004 Firaxis Games, Inc. All rights reserved.
//---------------------------------------------------------------------------------------

#include "CvGameCoreDLL.h"
#include "CvGameTextMgr.h"
#include "CvGameCoreUtils.h"
#include "CvDLLUtilityIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLSymbolIFaceBase.h"
#include "CvInfos.h"
#include "CvXMLLoadUtility.h"
#include "CvCity.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CvGameAI.h"
#include "CvSelectionGroup.h"
#include "CvMap.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"
#include "CyArgsList.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvStructs.h"
#include "CvDiploParameters.h"

// lfgr UI 11/2020: Allow cycling through units in plot help
#include "PlotHelpCycling.h"


int shortenID(int iId)
{
	return iId;
}

// For displaying Asserts and error messages
static char* szErrorMsg;

//----------------------------------------------------------------------------
//
//	FUNCTION:	GetInstance()
//
//	PURPOSE:	Get the instance of this class.
//
//----------------------------------------------------------------------------
CvGameTextMgr& CvGameTextMgr::GetInstance()
{
	static CvGameTextMgr gs_GameTextMgr;
	return gs_GameTextMgr;
}

//----------------------------------------------------------------------------
//
//	FUNCTION:	CvGameTextMgr()
//
//	PURPOSE:	Constructor
//
//----------------------------------------------------------------------------
CvGameTextMgr::CvGameTextMgr()
{

}

CvGameTextMgr::~CvGameTextMgr()
{
}

//----------------------------------------------------------------------------
//
//	FUNCTION:	Initialize()
//
//	PURPOSE:	Allocates memory
//
//----------------------------------------------------------------------------
void CvGameTextMgr::Initialize()
{

}

//----------------------------------------------------------------------------
//
//	FUNCTION:	DeInitialize()
//
//	PURPOSE:	Clears memory
//
//----------------------------------------------------------------------------
void CvGameTextMgr::DeInitialize()
{
	for (int i = 0; i < (int)m_apbPromotion.size(); i++)
	{
		delete[] m_apbPromotion[i];
	}
}

//----------------------------------------------------------------------------
//
//	FUNCTION:	Reset()
//
//	PURPOSE:	Accesses CvXMLLoadUtility to clean global text memory and
//				reload the XML files
//
//----------------------------------------------------------------------------
void CvGameTextMgr::Reset()
{
	CvXMLLoadUtility pXML;
	pXML.LoadGlobalText();
}


// Returns the current language
int CvGameTextMgr::getCurrentLanguage()
{
	return gDLL->getCurrentLanguage();
}

void CvGameTextMgr::setYearStr(CvWString& szString, int iGameTurn, bool bSave, CalendarTypes eCalendar, int iStartYear, GameSpeedTypes eSpeed)
{
	int iTurnYear = getTurnYearForGame(iGameTurn, iStartYear, eCalendar, eSpeed);

	if (iTurnYear < 0)
	{
		if (bSave)
		{
			szString = gDLL->getText("TXT_KEY_TIME_BC_SAVE", CvWString::format(L"%04d", -iTurnYear).GetCString());
		}
		else
		{
			szString = gDLL->getText("TXT_KEY_TIME_BC", -(iTurnYear));
		}
	}
	else if (iTurnYear > 0)
	{
		if (bSave)
		{
			szString = gDLL->getText("TXT_KEY_TIME_AD_SAVE", CvWString::format(L"%04d", iTurnYear).GetCString());
		}
		else
		{
			szString = gDLL->getText("TXT_KEY_TIME_AD", iTurnYear);
		}
	}
	else
	{
		if (bSave)
		{
			szString = gDLL->getText("TXT_KEY_TIME_AD_SAVE", L"0001");
		}
		else
		{
			szString = gDLL->getText("TXT_KEY_TIME_AD", 1);
		}
	}
}


void CvGameTextMgr::setDateStr(CvWString& szString, int iGameTurn, bool bSave, CalendarTypes eCalendar, int iStartYear, GameSpeedTypes eSpeed)
{
	CvWString szYearBuffer;
	CvWString szWeekBuffer;

	setYearStr(szYearBuffer, iGameTurn, bSave, eCalendar, iStartYear, eSpeed);

	switch (eCalendar)
	{
	case CALENDAR_DEFAULT:
		if (0 == (getTurnMonthForGame(iGameTurn + 1, iStartYear, eCalendar, eSpeed) - getTurnMonthForGame(iGameTurn, iStartYear, eCalendar, eSpeed)) % GC.getNumMonthInfos())
		{
			szString = szYearBuffer;
		}
		else
		{
			int iMonth = getTurnMonthForGame(iGameTurn, iStartYear, eCalendar, eSpeed);
			if (bSave)
			{
				szString = (szYearBuffer + "-" + GC.getMonthInfo((MonthTypes)(iMonth % GC.getNumMonthInfos())).getDescription());
			}
			else
			{
				szString = (GC.getMonthInfo((MonthTypes)(iMonth % GC.getNumMonthInfos())).getDescription() + CvString(", ") + szYearBuffer);
			}
		}
		break;
	case CALENDAR_YEARS:
	case CALENDAR_BI_YEARLY:
		szString = szYearBuffer;
		break;

	case CALENDAR_TURNS:
		szString = gDLL->getText("TXT_KEY_TIME_TURN", (iGameTurn + 1));
		break;

	case CALENDAR_SEASONS:
		if (bSave)
		{
			szString = (szYearBuffer + "-" + GC.getSeasonInfo((SeasonTypes)(iGameTurn % GC.getNumSeasonInfos())).getDescription());
		}
		else
		{
			szString = (GC.getSeasonInfo((SeasonTypes)(iGameTurn % GC.getNumSeasonInfos())).getDescription() + CvString(", ") + szYearBuffer);
		}
		break;

	case CALENDAR_MONTHS:
		if (bSave)
		{
			szString = (szYearBuffer + "-" + GC.getMonthInfo((MonthTypes)(iGameTurn % GC.getNumMonthInfos())).getDescription());
		}
		else
		{
			szString = (GC.getMonthInfo((MonthTypes)(iGameTurn % GC.getNumMonthInfos())).getDescription() + CvString(", ") + szYearBuffer);
		}
		break;

	case CALENDAR_WEEKS:
		szWeekBuffer = gDLL->getText("TXT_KEY_TIME_WEEK", ((iGameTurn % GC.getDefineINT("WEEKS_PER_MONTHS")) + 1));

		if (bSave)
		{
			szString = (szYearBuffer + "-" + GC.getMonthInfo((MonthTypes)((iGameTurn / GC.getDefineINT("WEEKS_PER_MONTHS")) % GC.getNumMonthInfos())).getDescription() + "-" + szWeekBuffer);
		}
		else
		{
			szString = (szWeekBuffer + ", " + GC.getMonthInfo((MonthTypes)((iGameTurn / GC.getDefineINT("WEEKS_PER_MONTHS")) % GC.getNumMonthInfos())).getDescription() + ", " + szYearBuffer);
		}
		break;

	default:
		FAssert(false);
	}
}


void CvGameTextMgr::setTimeStr(CvWString& szString, int iGameTurn, bool bSave)
{
	setDateStr(szString, iGameTurn, bSave, GC.getGameINLINE().getCalendar(), GC.getGameINLINE().getStartYear(), GC.getGameINLINE().getGameSpeedType());
}


void CvGameTextMgr::setInterfaceTime(CvWString& szString, PlayerTypes ePlayer)
{
	CvWString szTempBuffer;

	if (GET_PLAYER(ePlayer).isGoldenAge())
	{
		szString.Format(L"%c(%d) ", gDLL->getSymbolID(GOLDEN_AGE_CHAR), GET_PLAYER(ePlayer).getGoldenAgeTurns());
	}
	else
	{
		szString.clear();
	}

	setTimeStr(szTempBuffer, GC.getGameINLINE().getGameTurn(), false);
	szString += CvWString(szTempBuffer);
}


void CvGameTextMgr::setGoldStr(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).getGold() < 0)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getCommerceInfo(COMMERCE_GOLD).getChar(), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GET_PLAYER(ePlayer).getGold());
	}
	else
	{
		szString.Format(L"%c: %d", GC.getCommerceInfo(COMMERCE_GOLD).getChar(), GET_PLAYER(ePlayer).getGold());
	}

	int iGoldRate = GET_PLAYER(ePlayer).calculateGoldRate();
	if (iGoldRate < 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_NEG_GOLD_PER_TURN", iGoldRate);
	}
	else if (iGoldRate > 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_POS_GOLD_PER_TURN", iGoldRate);
	}

	if (GET_PLAYER(ePlayer).isStrike())
	{
		szString += gDLL->getText("TXT_KEY_MISC_STRIKE");
	}
}


void CvGameTextMgr::setResearchStr(CvWString& szString, PlayerTypes ePlayer)
{
	CvWString szTempBuffer;

	//	szString = gDLL->getText("TXT_KEY_MISC_RESEARCH_STRING", GC.getTechInfo(GET_PLAYER(ePlayer).getCurrentResearch()).getTextKeyWide());
	szString = GC.getTechInfo(GET_PLAYER(ePlayer).getCurrentResearch()).getDescription();

	if (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getTechCount(GET_PLAYER(ePlayer).getCurrentResearch()) > 0)
	{
		szTempBuffer.Format(L" %d", (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getTechCount(GET_PLAYER(ePlayer).getCurrentResearch()) + 1));
		szString += szTempBuffer;
	}

	szTempBuffer.Format(L" (%d)", GET_PLAYER(ePlayer).getResearchTurnsLeft(GET_PLAYER(ePlayer).getCurrentResearch(), true));
	szString += szTempBuffer;
}


void CvGameTextMgr::setOOSSeeds(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).isHuman())
	{
		int iNetID = GET_PLAYER(ePlayer).getNetID();
		if (gDLL->isConnected(iNetID))
		{
			szString = gDLL->getText("TXT_KEY_PLAYER_OOS", gDLL->GetSyncOOS(iNetID), gDLL->GetOptionsOOS(iNetID));
		}
	}
}

void CvGameTextMgr::setNetStats(CvWString& szString, PlayerTypes ePlayer)
{
	if (ePlayer != GC.getGameINLINE().getActivePlayer())
	{
		if (GET_PLAYER(ePlayer).isHuman())
		{
			if (gDLL->getInterfaceIFace()->isNetStatsVisible())
			{
				int iNetID = GET_PLAYER(ePlayer).getNetID();
				if (gDLL->isConnected(iNetID))
				{
					szString = gDLL->getText("TXT_KEY_MISC_NUM_MS", gDLL->GetLastPing(iNetID));
				}
				else
				{
					szString = gDLL->getText("TXT_KEY_MISC_DISCONNECTED");
				}
			}
		}
		else
		{
			szString = gDLL->getText("TXT_KEY_MISC_AI");
		}
	}
}


void CvGameTextMgr::setMinimizePopupHelp(CvWString& szString, const CvPopupInfo& info)
{
	CvCity* pCity;
	UnitTypes eTrainUnit;
	BuildingTypes eConstructBuilding;
	ProjectTypes eCreateProject;
	ReligionTypes eReligion;
	CivicTypes eCivic;

	switch (info.getButtonPopupType())
	{
	case BUTTONPOPUP_CHOOSEPRODUCTION:
		pCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCity(info.getData1());
		if (pCity != NULL)
		{
			eTrainUnit = NO_UNIT;
			eConstructBuilding = NO_BUILDING;
			eCreateProject = NO_PROJECT;

			switch (info.getData2())
			{
			case (ORDER_TRAIN):
				eTrainUnit = (UnitTypes)info.getData3();
				break;
			case (ORDER_CONSTRUCT):
				eConstructBuilding = (BuildingTypes)info.getData3();
				break;
			case (ORDER_CREATE):
				eCreateProject = (ProjectTypes)info.getData3();
				break;
			default:
				break;
			}

			if (eTrainUnit != NO_UNIT)
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION_UNIT", GC.getUnitInfo(eTrainUnit).getTextKeyWide(), pCity->getNameKey());
			}
			else if (eConstructBuilding != NO_BUILDING)
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION_BUILDING", GC.getBuildingInfo(eConstructBuilding).getTextKeyWide(), pCity->getNameKey());
			}
			else if (eCreateProject != NO_PROJECT)
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION_PROJECT", GC.getProjectInfo(eCreateProject).getTextKeyWide(), pCity->getNameKey());
			}
			else
			{
				szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_PRODUCTION", pCity->getNameKey());
			}
		}
		break;

	case BUTTONPOPUP_CHANGERELIGION:
		eReligion = ((ReligionTypes)(info.getData1()));
		if (eReligion != NO_RELIGION)
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHANGE_RELIGION", GC.getReligionInfo(eReligion).getTextKeyWide());
		}
		break;

	case BUTTONPOPUP_CHOOSETECH:
		if (info.getData1() > 0)
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_TECH_FREE");
		}
		else
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHOOSE_TECH");
		}
		break;

	case BUTTONPOPUP_CHANGECIVIC:
		eCivic = ((CivicTypes)(info.getData2()));
		if (eCivic != NO_CIVIC)
		{
			szString += gDLL->getText("TXT_KEY_MINIMIZED_CHANGE_CIVIC", GC.getCivicInfo(eCivic).getTextKeyWide());
		}
		break;
	}
}

void CvGameTextMgr::setEspionageMissionHelp(CvWStringBuffer& szBuffer, const CvUnit* pUnit)
{
	if (pUnit->isSpy())
	{
		PlayerTypes eOwner = pUnit->plot()->getOwnerINLINE();
		if (NO_PLAYER != eOwner && GET_PLAYER(eOwner).getTeam() != pUnit->getTeam())
		{
			if (!pUnit->canEspionage(pUnit->plot()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HELP_NO_ESPIONAGE"));

				if (pUnit->hasMoved() || pUnit->isMadeAttack())
				{
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HELP_NO_ESPIONAGE_REASON_MOVED"));
				}
				else if (!pUnit->isInvisible(GET_PLAYER(eOwner).getTeam(), false))
				{
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HELP_NO_ESPIONAGE_REASON_VISIBLE", GET_PLAYER(eOwner).getNameKey()));
				}
			}
			else if (pUnit->getFortifyTurns() > 0)
			{
				int iModifier = -(pUnit->getFortifyTurns() * GC.getDefineINT("ESPIONAGE_EACH_TURN_UNIT_COST_DECREASE"));
				if (0 != iModifier)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_COST", iModifier));
				}
			}
		}
	}
}


void CvGameTextMgr::setUnitHelp(CvWStringBuffer& szString, CvUnit* pUnit, bool bOneLine, bool bShort)
{
	PROFILE_FUNC();

	CvWString szTempBuffer;
	BuildTypes eBuild;
	int iCurrMoves;
	int iI;
	bool bFirst;
	bool bShift = gDLL->shiftKey();
	bool bAlt = gDLL->altKey();
	bool bCtrl = gDLL->ctrlKey();	//SpyFanatic: use ctrl button to reduce some display for CombatAura
	/*************************************************************************************************/
	/**	ADDON (Ranged Combat) Sephi								                     				**/
	/**																								**/
	/**				Displays the Unit Religion, Combat Type, Ranged Damage of the Unit				**/
	/*************************************************************************************************/
	/**								---- Start Original Code ----									**
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), pUnit->getName().GetCString());
		szString.append(szTempBuffer);

		szString.append(L", ");

		if (pUnit->getDomainType() == DOMAIN_AIR)
		{
			if (pUnit->airBaseCombatStr() > 0)
			{
				if (pUnit->isFighting())
				{
					szTempBuffer.Format(L"?/%d%c, ", pUnit->airBaseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
				}
				else if (pUnit->isHurt())
				{
					szTempBuffer.Format(L"%.1f/%d%c, ", (((float)(pUnit->airBaseCombatStr() * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))), pUnit->airBaseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
				}
				else
				{
					szTempBuffer.Format(L"%d%c, ", pUnit->airBaseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
				}
				szString.append(szTempBuffer);
			}
		}
	/**								----  End Original Code  ----									**/


	bool bFirstBlessing = false;
	bool bFirstCurse = false;

	if (pUnit->getBlessing1() != NULL)
	{
		CvCombatAuraClassInfo& kClass = GC.getCombatAuraClassInfo((CombatAuraClassTypes)GC.getCombatAuraInfo((CombatAuraTypes)pUnit->getBlessing1()->getCombatAuraType()).getCombatAuraClassType());
		bFirstBlessing = true;
		szTempBuffer.Format(L"<img=%S size=16></img> ", kClass.getButton());
		szString.append(szTempBuffer);
	}

	if (pUnit->getBlessing2() != NULL)
	{
		CvCombatAuraClassInfo& kClass = GC.getCombatAuraClassInfo((CombatAuraClassTypes)GC.getCombatAuraInfo((CombatAuraTypes)pUnit->getBlessing2()->getCombatAuraType()).getCombatAuraClassType());
		bFirstBlessing = true;
		szTempBuffer.Format(L"<img=%S size=16></img> ", kClass.getButton());
		szString.append(szTempBuffer);
	}

	if (pUnit->getCurse1() != NULL)
	{
		if (bFirstBlessing && !bFirstCurse)
			szString.append(L"|");
		bFirstCurse = true;
		CvCombatAuraClassInfo& kClass = GC.getCombatAuraClassInfo((CombatAuraClassTypes)GC.getCombatAuraInfo((CombatAuraTypes)pUnit->getCurse1()->getCombatAuraType()).getCombatAuraClassType());
		szTempBuffer.Format(L"<img=%S size=16></img> ", kClass.getButton());
		szString.append(szTempBuffer);
	}

	if (pUnit->getCurse2() != NULL)
	{
		if (bFirstBlessing && !bFirstCurse)
			szString.append(L"|");
		bFirstCurse = true;
		CvCombatAuraClassInfo& kClass = GC.getCombatAuraClassInfo((CombatAuraClassTypes)GC.getCombatAuraInfo((CombatAuraTypes)pUnit->getCurse2()->getCombatAuraType()).getCombatAuraClassType());
		szTempBuffer.Format(L"<img=%S size=16></img> ", kClass.getButton());
		szString.append(szTempBuffer);
	}

	szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), pUnit->getName().GetCString());
	szString.append(szTempBuffer);

	szString.append(gDLL->getText("TXT_KEY_AGE", (GC.getGameINLINE().getGameTurn() - pUnit->getGameTurnCreated()) * 100 / GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent()));

	szString.append(L", ");

	if (pUnit->airBaseCombatStr() > 0)
	{
		if (pUnit->isFighting())
		{
			szTempBuffer.Format(L"?/%d", pUnit->airBaseCombatStr());
			szString.append(szTempBuffer);
		}
		else if (pUnit->isHurt())
		{
			szTempBuffer.Format(L"%.1f/%d", (((float)(pUnit->airBaseCombatStr() * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))), pUnit->airBaseCombatStr());
			szString.append(szTempBuffer);
		}
		else
		{
			szTempBuffer.Format(L"%d", pUnit->airBaseCombatStr());
			szString.append(szTempBuffer);
		}
		if (pUnit->airCombatLimit() == 100)
		{
			szTempBuffer.Format(L"%c, ", gDLL->getSymbolID(RANGED_CHAR));
		}
		else
		{
			szTempBuffer.Format(L"%c" SETCOLR L"%d%%" ENDCOLR L", ", gDLL->getSymbolID(RANGED_CHAR), TEXT_COLOR("COLOR_UNIT_TEXT"), pUnit->airCombatLimit());
		}
		szString.append(szTempBuffer);
	}

	if (pUnit->canFight())
	{
		/*************************************************************************************************/
		/**	    										END												**/
		/*************************************************************************************************/
		if (pUnit->canFight())
		{

			//FfH: Modified by Kael 08/18/2007
			//			if (pUnit->isFighting())
			//			{
			//				szTempBuffer.Format(L"?/%d%c, ", pUnit->baseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			//			}
			//			else if (pUnit->isHurt())
			//			{
			//				szTempBuffer.Format(L"%.1f/%d%c, ", (((float)(pUnit->baseCombatStr() * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))), pUnit->baseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			//			}
			//			else
			//			{
			//				szTempBuffer.Format(L"%d%c, ", pUnit->baseCombatStr(), gDLL->getSymbolID(STRENGTH_CHAR));
			//			}
			//			szString.append(szTempBuffer);

			/*************************************************************************************************/
			/**	FFHBUG denev																				**/
			/**	ADDON (FFHBUG) 																	**/
			/**																								**/
			/*************************************************************************************************/
			/** Orig
						int iDif = pUnit->getTotalDamageTypeCombat();
						if (pUnit->isFighting())
						{
							szTempBuffer.Format(L"?/%d", pUnit->baseCombatStr() - iDif);
						}
						else if (pUnit->isHurt())
						{
							if (pUnit->baseCombatStr() == pUnit->baseCombatStrDefense())
							{
								szTempBuffer.Format(L"%.1f/%d", (((float)((pUnit->baseCombatStr() - iDif) * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))), pUnit->baseCombatStr());
							}
							else
							{
								szTempBuffer.Format(L"%.1f/%.lf", (((float)((pUnit->baseCombatStr() - iDif) * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))), (((float)((pUnit->baseCombatStrDefense() - iDif) * pUnit->currHitPoints())) / ((float)(pUnit->maxHitPoints()))));
							}
						}
						else
						{
							if (pUnit->baseCombatStr() == pUnit->baseCombatStrDefense())
							{
								szTempBuffer.Format(L"%d", pUnit->baseCombatStr() - iDif);
							}
							else
							{
								szTempBuffer.Format(L"%d/%d", pUnit->baseCombatStr() - iDif, pUnit->baseCombatStrDefense() - iDif);
							}
						}
						szString.append(szTempBuffer);
						for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
						{
							if (pUnit->getDamageTypeCombat((DamageTypes)iI) != 0)
							{
								szTempBuffer.Format(L" (+%d %s)", pUnit->getDamageTypeCombat((DamageTypes)iI), GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
								szString.append(szTempBuffer);
							}
						}
						szTempBuffer.Format(L"%c, ", gDLL->getSymbolID(STRENGTH_CHAR));
						szString.append(szTempBuffer);
			**/
			const int iDif = pUnit->getTotalDamageTypeCombat();

			//wounded color
			const int iNumDmgLv = 3;
			const int iDmgLv = std::max(0, std::min((iNumDmgLv - 1), iNumDmgLv * pUnit->getDamage() / pUnit->maxHitPoints()));
			const char* szColor[] =
			{
				"COLOR_WHITE",
				"COLOR_PLAYER_LIGHT_ORANGE_TEXT",
				"COLOR_WARNING_TEXT",
			};

			if (pUnit->isFighting())
			{
				if (pUnit->baseCombatStr() == pUnit->baseCombatStrDefense())
				{
					szTempBuffer.Format(L"?(%d", pUnit->baseCombatStr() - iDif);
				}
				else
				{
					szTempBuffer.Format(L"?/?(%d/%d", pUnit->baseCombatStr() - iDif, pUnit->baseCombatStrDefense() - iDif);
				}
			}
			else if (pUnit->baseCombatStr() == pUnit->baseCombatStrDefense())
			{
				if (pUnit->isHurt())
				{
					const float fAttStr = (float)pUnit->baseCombatStr() * (float)pUnit->currHitPoints() / (float)pUnit->maxHitPoints();
					szTempBuffer.Format(SETCOLR L"%.1f" ENDCOLR L" (%d",
						TEXT_COLOR(szColor[iDmgLv]), fAttStr, pUnit->baseCombatStr() - iDif);
				}
				else
				{
					if (iDif != 0)
					{
						szTempBuffer.Format(L"%d(%d", pUnit->baseCombatStr(), pUnit->baseCombatStr() - iDif);
					}
					else
					{
						szTempBuffer.Format(L"%d", pUnit->baseCombatStr());
					}
				}
			}
			else
			{
				if (pUnit->isHurt())
				{
					const float fAttStr = (float)pUnit->baseCombatStr() * (float)pUnit->currHitPoints() / (float)pUnit->maxHitPoints();
					const float fDefStr = (float)pUnit->baseCombatStrDefense() * (float)pUnit->currHitPoints() / (float)pUnit->maxHitPoints();
					szTempBuffer.Format(SETCOLR L"%.1f" ENDCOLR L"/" SETCOLR L"%.1f" ENDCOLR L"(%d/%d",
						TEXT_COLOR(szColor[iDmgLv]), fAttStr, TEXT_COLOR(szColor[iDmgLv]), fDefStr, pUnit->baseCombatStr() - iDif, pUnit->baseCombatStrDefense() - iDif);
				}
				else
				{
					if (iDif != 0)
					{
						szTempBuffer.Format(L"%d/%d(%d/%d", pUnit->baseCombatStr(), pUnit->baseCombatStrDefense(), pUnit->baseCombatStr() - iDif, pUnit->baseCombatStrDefense() - iDif);
					}
					else
					{
						szTempBuffer.Format(L"%d/%d", pUnit->baseCombatStr(), pUnit->baseCombatStrDefense());
					}
				}
			}
			szString.append(szTempBuffer);
			for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
			{
				if (pUnit->getDamageTypeCombat((DamageTypes)iI) != 0)
				{
					szTempBuffer.Format(L", +%d %s", pUnit->getDamageTypeCombat((DamageTypes)iI), GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
					szString.append(szTempBuffer);
				}
			}
			if (pUnit->isHurt() || iDif != 0)
			{
				szString.append(L")");
			}
			szTempBuffer.Format(L"%c, ", gDLL->getSymbolID(STRENGTH_CHAR));
			szString.append(szTempBuffer);
			/*************************************************************************************************/
			/**	END																							**/
			/*************************************************************************************************/

			//FfH: End Modify

		}
	}

	iCurrMoves = ((pUnit->movesLeft() / GC.getMOVE_DENOMINATOR()) + (((pUnit->movesLeft() % GC.getMOVE_DENOMINATOR()) > 0) ? 1 : 0));
	if ((pUnit->baseMoves() == iCurrMoves) || (pUnit->getTeam() != GC.getGameINLINE().getActiveTeam()))
	{
		szTempBuffer.Format(L"%d%c", pUnit->baseMoves(), gDLL->getSymbolID(MOVES_CHAR));
	}
	else
	{
		szTempBuffer.Format(L"%d/%d%c", iCurrMoves, pUnit->baseMoves(), gDLL->getSymbolID(MOVES_CHAR));
	}
	szString.append(szTempBuffer);

	if (pUnit->airRange() > 0)
	{
		szString.append(gDLL->getText("TXT_KEY_UNIT_HELP_AIR_RANGE", pUnit->airRange()));
	}

	eBuild = pUnit->getBuildType();

	if (eBuild != NO_BUILD)
	{
		szString.append(L", ");
		szTempBuffer.Format(L"%s (%d)", GC.getBuildInfo(eBuild).getDescription(), pUnit->plot()->getBuildTurnsLeft(eBuild, 0, 0));
		szString.append(szTempBuffer);
	}

	if (pUnit->getImmobileTimer() > 0)
	{
		szString.append(L", ");

		//FfH: Modified by Kael 09/15/2008
		//		szString.append(gDLL->getText("TXT_KEY_UNIT_HELP_IMMOBILE", pUnit->getImmobileTimer()));
		if (pUnit->isHeld())
		{
			szString.append(gDLL->getText("TXT_KEY_UNIT_HELP_HELD"));
		}
		/*************************************************************************************************/
		/**	Xienwolf Tweak							02/01/09											**/
		/**	ADDON (reminder for spells being cast) merged Sephi											**/
		/**						Reminds you what spell the unit is trying to cast						**/
		/*************************************************************************************************/
		else if (pUnit->getDelayedSpell() != NO_SPELL)
		{
			szString.append(gDLL->getText("TXT_KEY_UNIT_HELP_DELAYED_CASTING", GC.getSpellInfo((SpellTypes)pUnit->getDelayedSpell()).getDescription(), pUnit->getImmobileTimer()));
		}
		/*************************************************************************************************/
		/**	Tweak									END													**/
		/*************************************************************************************************/
		else
		{
			szString.append(gDLL->getText("TXT_KEY_UNIT_HELP_IMMOBILE", pUnit->getImmobileTimer()));
		}
		//FfH: End Modify

	}

	/*if (!bOneLine)
	{
		if (pUnit->getUnitCombatType() != NO_UNITCOMBAT)
		{
			szTempBuffer.Format(L" (%s)", GC.getUnitCombatInfo(pUnit->getUnitCombatType()).getDescription());
			szString += szTempBuffer;
		}
	}*/

	if (GC.getGameINLINE().isDebugMode() && !bAlt && !bShift)
	{
		FAssertMsg(pUnit->AI_getUnitAIType() != NO_UNITAI, "pUnit's AI type expected to != NO_UNITAI");

		// Better AI Debug (Skyre)
				/*
				szTempBuffer.Format(L" (%s)", GC.getUnitAIInfo(pUnit->AI_getUnitAIType()).getDescription());
				*/
		if (pUnit->AI_getGroupflag() != GROUPFLAG_NONE)
		{
			szTempBuffer.Format(L" (%s)", getGroupflagName(pUnit->AI_getGroupflag()));
		}
		else
		{
			szTempBuffer.Format(L" (%s)", GC.getUnitAIInfo(pUnit->AI_getUnitAIType()).getDescription());
		}
		// End Better AI Debug
		szString.append(szTempBuffer);
		if (pUnit->getAIGroup() != NULL)
		{
			szTempBuffer.Format(L" (%s)", GC.getAIGroupInfo(pUnit->getAIGroup()->getGroupType()).getDescription());
			szString.append(szTempBuffer);
			if (pUnit->getAIGroup()->getGroupType() == AIGROUP_CITY_INVASION)
			{
				szTempBuffer.clear();
				getAI_InvasionStatusString(szTempBuffer, pUnit->getAIGroup()->getMissionStatus());
				szString.append(szTempBuffer);
			}
		}
		if (pUnit->AI_getPromotionSpecialization() != NO_AIPROMOTIONSPECIALIZATION)
		{
			szTempBuffer.Format(L" (%s)", GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)pUnit->AI_getPromotionSpecialization()).getDescription());
			szString.append(szTempBuffer);
		}
	}

	if ((pUnit->getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
	{
		if ((pUnit->getExperience() > 0) && !(pUnit->isFighting()))
		{
			szString.append(gDLL->getText("TXT_KEY_UNIT_HELP_LEVEL", pUnit->getExperience(), pUnit->experienceNeeded()));
		}
	}

	//FfH: Modified by Kael 08/27/2007
	//	if (pUnit->getOwnerINLINE() != GC.getGameINLINE().getActivePlayer() && !pUnit->isAnimal() && !pUnit->getUnitInfo().isHiddenNationality())
	if (pUnit->getOwnerINLINE() != GC.getGameINLINE().getActivePlayer() && !pUnit->isAnimal() && !pUnit->isHiddenNationality())
		//FfH: End Modify

	{
		szString.append(L", ");
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorA(), GET_PLAYER(pUnit->getOwnerINLINE()).getName());
		szString.append(szTempBuffer);
	}

	for (iI = 0; iI < GC.getNumPromotionInfos(); ++iI)
	{
		if (pUnit->isHasPromotion((PromotionTypes)iI))
		{
			if (!GC.getPromotionInfo((PromotionTypes)iI).isDisplayAsEquipment()) {
				szTempBuffer.Format(L"<img=%S size=16></img>", GC.getPromotionInfo((PromotionTypes)iI).getButton());
				szString.append(szTempBuffer);
			}
		}
	}

	if (bAlt && (gDLL->getChtLvl() > 0))
	{
		CvSelectionGroup* eGroup = pUnit->getGroup();
		if (eGroup != NULL)
		{
			if (pUnit->isGroupHead())
				szString.append(CvWString::format(L"\nLeading "));
			else
				szString.append(L"\n");

			szTempBuffer.Format(L"Group(%d), %d units", eGroup->getID(), eGroup->getNumUnits());
			szString.append(szTempBuffer);
		}
	}

	// Better Unit Power (Added by Skyre)
	if (GC.getGameINLINE().isDebugMode() && !bAlt && !bShift)
	{
		szTempBuffer.Format(L" (Power: %d)", pUnit->getPower());
		szString.append(szTempBuffer);
	}
	// End Better Unit Power

	if (!bOneLine)
	{
if(!bAlt && !bShift && !bCtrl){
		if (!bShort) {
			//SpyFanatic: show level
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_LEVEL", pUnit->getLevel()));
			//SpyFanatic: TEMP to show free promotion
			if(pUnit->getFreePromotionPick() > 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_FREEPROMOTION", pUnit->getFreePromotionPick()));
			}

			//////////////////////////////////////////////////
			// general Combat Bonuses that are the most useful
			//////////////////////////////////////////////////

			if (pUnit->getExtraCombatPercent() != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_STRENGTH_TEXT", pUnit->getExtraCombatPercent()));
			}

			if (pUnit->maxFirstStrikes() > 0)
			{
				if (pUnit->firstStrikes() == pUnit->maxFirstStrikes())
				{
					if (pUnit->firstStrikes() == 1)
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE"));
					}
					else
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", pUnit->firstStrikes()));
					}
				}
				else
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", pUnit->firstStrikes(), pUnit->maxFirstStrikes()));
				}
			}

			if (pUnit->immuneToFirstStrikes())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_IMMUNE_FIRST_STRIKES"));
			}

			if (pUnit->isDefensiveStrikes())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_DEFENSIVE_STRIKES"));
			}

			//////////////////////////////////////////////////
			// Grouped Information
			//////////////////////////////////////////////////

			//Terrain Strength Bonuses
			StringList TerrainList;

			if (pUnit->cityAttackModifier() == pUnit->cityDefenseModifier())
			{
				if (pUnit->cityAttackModifier() != 0)
				{
					TerrainList.append(gDLL->getText("TXT_KEY_UNIT_CITY_STRENGTH_MODIS", pUnit->cityAttackModifier()));
				}
			}
			else
			{
				if (pUnit->cityAttackModifier() != 0)
				{
					TerrainList.append(gDLL->getText("TXT_KEY_UNIT_CITY_ATTACK_MODIS", pUnit->cityAttackModifier()));
				}

				if (pUnit->cityDefenseModifier() != 0)
				{
					TerrainList.append(gDLL->getText("TXT_KEY_UNIT_CITY_DEFENSE_MODIS", pUnit->cityDefenseModifier()));
				}
			}

			if (pUnit->hillsAttackModifier() == pUnit->hillsDefenseModifier())
			{
				if (pUnit->hillsAttackModifier() != 0)
				{
					TerrainList.append(gDLL->getText("TXT_KEY_UNIT_HILLS_STRENGTH_MODIS", pUnit->cityAttackModifier()));
				}
			}
			else
			{
				if (pUnit->hillsAttackModifier() != 0)
				{
					TerrainList.append(gDLL->getText("TXT_KEY_UNIT_HILLS_ATTACK_MODIS", pUnit->cityAttackModifier()));
				}

				if (pUnit->hillsDefenseModifier() != 0)
				{
					TerrainList.append(gDLL->getText("TXT_KEY_UNIT_HILLS_DEFENSE_MODIS", pUnit->cityAttackModifier()));
				}
			}

			for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
			{
				if (pUnit->terrainAttackModifier((TerrainTypes)iI) == pUnit->terrainDefenseModifier((TerrainTypes)iI))
				{
					if (pUnit->terrainAttackModifier((TerrainTypes)iI) != 0)
					{
						TerrainList.append(gDLL->getText("TXT_KEY_UNIT_STRENGTH_MODIS", pUnit->terrainAttackModifier((TerrainTypes)iI), getLinkedText((TerrainTypes)iI).c_str()));
					}
				}
				else
				{
					if (pUnit->terrainAttackModifier((TerrainTypes)iI) != 0)
					{
						TerrainList.append(gDLL->getText("TXT_KEY_UNIT_ATTACK_MODIS", pUnit->terrainAttackModifier((TerrainTypes)iI), getLinkedText((TerrainTypes)iI).c_str()));
					}

					if (pUnit->terrainDefenseModifier((TerrainTypes)iI) != 0)
					{
						TerrainList.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE_MODIS", pUnit->terrainDefenseModifier((TerrainTypes)iI), getLinkedText((TerrainTypes)iI).c_str()));
					}
				}
			}

			for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
			{
				if (pUnit->featureAttackModifier((FeatureTypes)iI) == pUnit->featureDefenseModifier((FeatureTypes)iI))
				{
					if (pUnit->featureAttackModifier((FeatureTypes)iI) != 0)
					{
						TerrainList.append(gDLL->getText("TXT_KEY_UNIT_STRENGTH_MODIS", pUnit->featureAttackModifier((FeatureTypes)iI), getLinkedText((FeatureTypes)iI).c_str()));
					}
				}
				else
				{
					if (pUnit->featureAttackModifier((FeatureTypes)iI) != 0)
					{
						TerrainList.append(gDLL->getText("TXT_KEY_UNIT_ATTACK_MODIS", pUnit->featureAttackModifier((FeatureTypes)iI), getLinkedText((FeatureTypes)iI).c_str()));
					}

					if (pUnit->featureDefenseModifier((FeatureTypes)iI) != 0)
					{
						TerrainList.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE_MODIS", pUnit->featureDefenseModifier((FeatureTypes)iI), getLinkedText((FeatureTypes)iI).c_str()));
					}
				}
			}

			if (!TerrainList.isEmpty()) {
				szString.append(gDLL->getText("TXT_KEY_UNIT_TERRAIN_MODIS"));
				szString.append(NEWLINE);
				szString.append(TerrainList.get());
			}

			StringList MagicList;

			bFirst = true;
			for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
			{
				if (pUnit->getDamageTypeResist((DamageTypes)iI) < 0)
				{
					if (bFirst) {
						MagicList.newEntry(L"Weakness: ");
						szTempBuffer.assign(GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
						MagicList.append(szTempBuffer);
						bFirst = false;
					}
					else {
						szTempBuffer.assign(GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
						MagicList.append(szTempBuffer);
					}
				}
			}

			bFirst = true;
			for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
			{
				if (pUnit->getDamageTypeResist((DamageTypes)iI) < 100 && pUnit->getDamageTypeResist((DamageTypes)iI) > 0)
				{
					if (bFirst) {
						MagicList.newEntry(L"Resistance: ");
						szTempBuffer.assign(GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
						MagicList.append(szTempBuffer);
						bFirst = false;
					}
					else {
						szTempBuffer.assign(GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
						MagicList.append(szTempBuffer);
					}
				}
			}

			bFirst = true;
			for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
			{
				if (pUnit->getDamageTypeResist((DamageTypes)iI) >= 100)
				{
					if (bFirst) {
						MagicList.newEntry(L"Immunity: ");
						szTempBuffer.assign(GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
						MagicList.append(szTempBuffer);
						bFirst = false;
					}
					else {
						szTempBuffer.assign(GC.getDamageTypeInfo((DamageTypes)iI).getDescription());
						MagicList.append(szTempBuffer);
					}
				}
			}

			if (pUnit->getSpellDamageModify() != 0)
			{
				MagicList.newEntry(gDLL->getText("TXT_KEY_PROMOTION_SPELL_DAMAGE_MODIFY", pUnit->getSpellDamageModify()));
			}
			if (pUnit->getResistModify() != 0)
			{
				MagicList.newEntry(gDLL->getText("TXT_KEY_PROMOTION_CASTER_RESIST_MODIFY", pUnit->getResistModify()));
			}
			if (pUnit->getResist() != 0)
			{
				MagicList.newEntry(gDLL->getText("TXT_KEY_PROMOTION_RESIST", pUnit->getResist()));
			}

			if (!MagicList.isEmpty()) {
				szString.append(gDLL->getText("TXT_KEY_UNIT_MAGIC_MODIS"));
				szString.append(NEWLINE);
				szString.append(MagicList.get());
			}

			//Line Between Grouped information and other information
			szString.append(NEWLINE);
		}

		//FfH: Added by Kael 07/23/2007
		if (pUnit->getReligion() != NO_RELIGION)
		{
			szString.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/10/03
			//			szString.append(gDLL->getText("TXT_KEY_UNIT_RELIGION", GC.getReligionInfo((ReligionTypes)pUnit->getReligion()).getDescription()));
			szString.append(gDLL->getText("TXT_KEY_UNIT_RELIGION", getLinkedText((ReligionTypes)pUnit->getReligion()).c_str()));
			//<<<<	BUGFfH: End Modify
		}
		if (!pUnit->isAlive())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_NOT_ALIVE"));
		}
		if (pUnit->isHasCasted())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_HAS_CASTED"));
		}
		if (pUnit->getDuration() != 0)
		{
			szString.append(NEWLINE);
			szString.append(CvWString::format(SETCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));
			szString.append(gDLL->getText("TXT_KEY_UNIT_DURATION", pUnit->getDuration()));
			szString.append(CvWString::format(ENDCOLR));
		}
		/*************************************************************************************************/
		/**	FFHBUG denev																				**/
		/**	ADDON (FFHBUG) 																	**/
		/**	Moved to below(*1) by Denev 2009/09/05														**/
		/*************************************************************************************************/
		/**
				for (int iJ=0;iJ<GC.getNumPromotionInfos();iJ++)
				{
					if (pUnit->isHasPromotion((PromotionTypes)iJ) && GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod()>0)
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod(), GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()).getTextKeyWide()));
					}
				}
		**/
		/*************************************************************************************************/
		/**	END																							**/
		/*************************************************************************************************/
		//FfH: End Add

		if (pUnit->cargoSpace() > 0)
		{
			if (pUnit->getTeam() == GC.getGameINLINE().getActiveTeam())
			{
				szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_UNIT_HELP_CARGO_SPACE", pUnit->getCargo(), pUnit->cargoSpace());
			}
			else
			{
				szTempBuffer = NEWLINE + gDLL->getText("TXT_KEY_UNIT_CARGO_SPACE", pUnit->cargoSpace());
			}
			szString.append(szTempBuffer);

			if (pUnit->specialCargo() != NO_SPECIALUNIT)
			{
				szString.append(gDLL->getText("TXT_KEY_UNIT_CARRIES", GC.getSpecialUnitInfo(pUnit->specialCargo()).getTextKeyWide()));
			}
		}

		if (pUnit->fortifyModifier() != 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_HELP_FORTIFY_BONUS", pUnit->fortifyModifier()));
		}

		/*************************************************************************************************/
		/**	FFHBUG denev																				**/
		/**	ADDON (FFHBUG) 																	**/
		/**	Added by Denev 2009/09/06 get Abilities from promotions										**/
		/*************************************************************************************************/
				/*------------------------------------------------------*/
				/*	BUGFfH comment: get Abilities from promotions		*/
				/*------------------------------------------------------*/
		int iTotalBetrayalChance = 0;
		std::list<CvWString> aszPromotionRandomApply;
		std::list<CvWString> aszPromotionImmune;
		std::list<CvWString> aszPromotionSummonPerk;
		std::list<CvWString> aszPromotionCombatApply;
		std::list<CvWString> aszCaptureUnitCombat;

		int iTotalCombatCapturePercent = 0;
		int iTotalFreeXPFromCombat = 0;
		int iTotalGoldFromCombat = 0;
		int iTotalModifyGlobalCounter = 0;
		int iTotalModifyGlobalCounterOnCombat = 0;
		int iTotalFreeXPPerTurn = 0;
		int iUnitPromotionEnslavementChance = 0;
		std::vector<int> aiPromotionCombatMod;

		//Initialize
		for (int iPromotionCombatType = 0; iPromotionCombatType < GC.getNumPromotionInfos(); iPromotionCombatType++)
		{
			aiPromotionCombatMod.push_back(0);
		}

		//get ModifyGlobalCounter from UnitTypes
		iTotalModifyGlobalCounter += GC.getUnitInfo(pUnit->getUnitType()).getModifyGlobalCounter();

		//get FreeXPFromCombat from Traits
		for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
		{
			if (GET_PLAYER(pUnit->getOwnerINLINE()).hasTrait((TraitTypes)iI))
			{
				const int iFreeXPFromCombat = GC.getTraitInfo((TraitTypes)iI).getFreeXPFromCombat();
				if (iFreeXPFromCombat != 0)
				{
					iTotalFreeXPFromCombat += iFreeXPFromCombat;
				}
			}
		}

		//Whether the unit has summoning ability or not
		bool bSummoningAbility = false;
		if (pUnit->isHasPromotion((PromotionTypes)GC.getInfoTypeForString("PROMOTION_CHANNELING1")))
		{
			bSummoningAbility = true;
		}
		else
		{
			for (iI = 0; iI < GC.getNumSpellInfos(); ++iI)
			{
				if (GC.getSpellInfo((SpellTypes)iI).getUnitClassPrereq() == pUnit->getUnitClassType()
					&& GC.getSpellInfo((SpellTypes)iI).getCreateUnitType() != NO_UNIT)
				{
					bSummoningAbility = true;
					break;
				}
			}
		}

		for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
		{
			if (pUnit->isHasPromotion((PromotionTypes)iI))
			{
				CvPromotionInfo& pPromotion = GC.getPromotionInfo((PromotionTypes)iI);
				PromotionTypes ePromotionTemp = NO_PROMOTION;

				if (pPromotion.getBetrayalChance() != 0)
				{
					iTotalBetrayalChance += pPromotion.getBetrayalChance();
				}

				ePromotionTemp = (PromotionTypes)pPromotion.getPromotionRandomApply();
				if (ePromotionTemp != NO_PROMOTION)
				{
					szTempBuffer = getLinkedText(ePromotionTemp);
					aszPromotionRandomApply.push_back(szTempBuffer);
				}

				ePromotionTemp = (PromotionTypes)pPromotion.getPromotionImmune1();
				if (ePromotionTemp != NO_PROMOTION)
				{
					szTempBuffer = getLinkedText(ePromotionTemp);
					aszPromotionImmune.push_back(szTempBuffer);
				}
				ePromotionTemp = (PromotionTypes)pPromotion.getPromotionImmune2();
				if (ePromotionTemp != NO_PROMOTION)
				{
					szTempBuffer = getLinkedText(ePromotionTemp);
					aszPromotionImmune.push_back(szTempBuffer);
				}
				ePromotionTemp = (PromotionTypes)pPromotion.getPromotionImmune3();
				if (ePromotionTemp != NO_PROMOTION)
				{
					szTempBuffer = getLinkedText(ePromotionTemp);
					aszPromotionImmune.push_back(szTempBuffer);
				}

				ePromotionTemp = (PromotionTypes)pPromotion.getPromotionSummonPerk();
				if (ePromotionTemp != NO_PROMOTION)
				{
					szTempBuffer = getLinkedText(ePromotionTemp);
					aszPromotionSummonPerk.push_back(szTempBuffer);
				}

				ePromotionTemp = (PromotionTypes)pPromotion.getPromotionCombatApply();
				if (ePromotionTemp != NO_PROMOTION)
				{
					//szTempBuffer = getLinkedText(ePromotionTemp);
					szTempBuffer = gDLL->getText("TXT_KEY_PROMOTION_COMBAT_APPLY", getLinkedText((PromotionTypes)pPromotion.getPromotionCombatApply()).c_str(),
						(PromotionTypes)pPromotion.getPromotionCombatApplyChance());

					aszPromotionCombatApply.push_back(szTempBuffer);
				}

				const UnitCombatTypes eCaptureUnitCombat = (UnitCombatTypes)pPromotion.getCaptureUnitCombat();
				if (eCaptureUnitCombat != NO_UNITCOMBAT)
				{
					szTempBuffer = getLinkedText(eCaptureUnitCombat);
					aszCaptureUnitCombat.push_back(szTempBuffer);
				}

				if (pPromotion.getCombatCapturePercent() != 0)
				{
					iTotalCombatCapturePercent += pPromotion.getCombatCapturePercent();
				}

				//get FreeXPFromCombat from Promotions
				if (pPromotion.getFreeXPFromCombat() != 0)
				{
					iTotalFreeXPFromCombat += pPromotion.getFreeXPFromCombat();
				}

				if (pPromotion.getPromotionCombatMod() > 0)
				{
					int iPromotionCombatType = pPromotion.getPromotionCombatType();
					aiPromotionCombatMod[iPromotionCombatType] += pPromotion.getPromotionCombatMod();
				}

				//get ModifyGlobalCounter from Promotions
				if (pPromotion.getModifyGlobalCounter() != 0)
				{
					iTotalModifyGlobalCounter += pPromotion.getModifyGlobalCounter();
				}

				if (pPromotion.getModifyGlobalCounterOnCombat() != 0)
				{
					iTotalModifyGlobalCounterOnCombat += pPromotion.getModifyGlobalCounterOnCombat();
				}

				if (pPromotion.getFreeXPPerTurn() != 0)
				{
					iTotalFreeXPPerTurn += pPromotion.getFreeXPPerTurn();
				}

				if (pPromotion.getEnslavementChance() > 0)
					iUnitPromotionEnslavementChance += pPromotion.getEnslavementChance();
			}
		}
		//sort and erase repeated items
		aszPromotionRandomApply.sort();		aszPromotionRandomApply.unique();
		aszPromotionImmune.sort();			aszPromotionImmune.unique();
		aszPromotionSummonPerk.sort();		aszPromotionSummonPerk.unique();
		aszPromotionCombatApply.sort();		aszPromotionCombatApply.unique();
		aszCaptureUnitCombat.sort();		aszCaptureUnitCombat.unique();
		/*************************************************************************************************/
		/**	END																							**/
		/*************************************************************************************************/

		if (!bShort)
		{
			/*************************************************************************************************/
			/**	FFHBUG denev																				**/
			/**	ADDON (FFHBUG) 																	**/
			/**	Added by Denev 2009/09/05 about Unfavorable effect																	**/
			/*************************************************************************************************/
			if (iTotalBetrayalChance != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_BETRAYAL_CHANCE", iTotalBetrayalChance));
			}
			/**
						for (std::list<CvWString>::iterator it = aszPromotionRandomApply.begin(); it != aszPromotionRandomApply.end(); it++)
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_RANDOM_APPLY", (*it).GetCString()));
						}
			**/
			if (pUnit->baseCombatStr() + pUnit->getTotalDamageTypeCombat() > 0)
			{
				if (pUnit->isOnlyDefensive())
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_ONLY_DEFENSIVE"));
				}
				if (GC.getUnitInfo(pUnit->getUnitType()).isNoCapture())
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_CANNOT_CAPTURE"));
				}
				if (GC.getUnitInfo(pUnit->getUnitType()).getCaptureDifficulty() > 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_DIFFICULT_TO_CAPTURE"));
				}
				if (GC.getUnitInfo(pUnit->getUnitType()).isNoPillage())
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_CANNOT_PILLAGE"));
				}
				if (!GC.getUnitInfo(pUnit->getUnitType()).isMilitaryHappiness())
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_CANNOT_PROTECT_CITY"));
				}
			}
			/*************************************************************************************************/
			/**	END																							**/
			/*************************************************************************************************/

			if (pUnit->nukeRange() >= 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_CAN_NUKE"));
			}

			if (pUnit->alwaysInvisible())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_INVISIBLE_ALL"));
			}
			else if (pUnit->getInvisibleType() != NO_INVISIBLE)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_INVISIBLE_MOST"));
			}

			for (iI = 0; iI < pUnit->getNumSeeInvisibleTypes(); ++iI)
			{
				if (pUnit->getSeeInvisibleType(iI) != pUnit->getInvisibleType())
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_SEE_INVISIBLE", GC.getInvisibleInfo(pUnit->getSeeInvisibleType(iI)).getTextKeyWide()));
				}
			}
			/*************************************************************************************************/
			/**	FFHBUG denev																				**/
			/**	ADDON (FFHBUG) 																	**/
			/**																								**/
			/*************************************************************************************************/
			// Moved from below(*7) by Denev 2009/09/06
			if (pUnit->getExtraVisibilityRange() != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_VISIBILITY_TEXT", pUnit->getExtraVisibilityRange()));
			}
			//BUGFfH: End Move
			//BUGFfH: Added by Denev 2009/09/07
			if (GC.getUnitInfo(pUnit->getUnitType()).isInvestigate())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_INVESTIGATE_CITY"));
			}
			/*************************************************************************************************/
			/**	END																							**/
			/*************************************************************************************************/

			if (pUnit->canMoveImpassable())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_CAN_MOVE_IMPASSABLE"));
			}
			/*************************************************************************************************/
			/**	FFHBUG denev																				**/
			/**	ADDON (FFHBUG) 																	**/
			/**																								**/
			/*************************************************************************************************/
			if (GC.getUnitInfo(pUnit->getUnitType()).isRivalTerritory())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_EXPLORE_RIVAL"));
			}

			//BUGFfH: Moved from below(*2) by Denev 2009/09/06
			if (pUnit->flatMovementCost())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_FLAT_MOVEMENT"));
			}
			else
			{
				if (pUnit->ignoreTerrainCost())
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_IGNORE_TERRAIN"));
				}
				//BUGFfH: End Move

				//BUGFfH: Moved from below(*8) by Denev 2009/09/06
				else if (pUnit->getExtraMoveDiscount() != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_MOVE_DISCOUNT_TEXT", -(pUnit->getExtraMoveDiscount())));
				}
				//BUGFfH: End Move


				bFirst = true;
				if (pUnit->isHillsDoubleMove())
				{
					bFirst = false;
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_HILLS_MOVE_TEXT"));
				}

				// Double Movement in Terrain / Features

				for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
				{
					if (pUnit->isTerrainDoubleMove((TerrainTypes)iI))
					{
						if (bFirst) {
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", getLinkedText((TerrainTypes)iI).c_str()));
							bFirst = false;
						}
						else {
							szTempBuffer.Format(L", %s", getLinkedText((TerrainTypes)iI).c_str());
							szString.append(szTempBuffer);
						}
					}
				}

				for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
				{
					if (pUnit->isFeatureDoubleMove((FeatureTypes)iI))
					{
						if (bFirst) {
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", getLinkedText((FeatureTypes)iI).c_str()));
							bFirst = false;
						}
						else {
							szTempBuffer.Format(L", %s", getLinkedText((FeatureTypes)iI).c_str());
							szString.append(szTempBuffer);
						}
					}
				}
				//BUGFfH: End Move

				//>>>>	BUGFfH: Added by Denev 2009/09/07
				bFirst = true;
				for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
				{
					if (GC.getUnitInfo(pUnit->getUnitType()).getTerrainImpassable((TerrainTypes)iI))
					{
						const int iTerrainPassableTech = GC.getUnitInfo(pUnit->getUnitType()).getTerrainPassableTech(iI);
						if (NO_TECH == (TechTypes)iTerrainPassableTech
							|| !GET_PLAYER(pUnit->getOwnerINLINE()).isHasTech(iTerrainPassableTech))
						{
							szTempBuffer.Format(L"%s%s ", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CANNOT_ENTER").GetCString());
							setListHelp(szString, szTempBuffer, getLinkedText((TerrainTypes)iI), L", ", bFirst);
							bFirst = false;
						}
					}
				}

				bFirst = true;
				for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
				{
					if (GC.getUnitInfo(pUnit->getUnitType()).getFeatureImpassable((FeatureTypes)iI))
					{
						const int iFeaturePassableTech = GC.getUnitInfo(pUnit->getUnitType()).getFeaturePassableTech(iI);
						if (NO_TECH == (TechTypes)iFeaturePassableTech
							|| !GET_PLAYER(pUnit->getOwnerINLINE()).isHasTech(iFeaturePassableTech))
						{
							szTempBuffer.Format(L"\n%s ", gDLL->getText("TXT_KEY_UNIT_CANNOT_ENTER").GetCString());
							setListHelp(szString, szTempBuffer, getLinkedText((FeatureTypes)iI), L", ", bFirst);
							bFirst = false;
						}
					}
				}
			}
			//<<<<	BUGFfH: End Add

			//BUGFfH: Moved from below(*3) by Denev 2009/09/06
			if (pUnit->isEnemyRoute())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_ENEMY_ROADS_TEXT"));
			}
			//BUGFfH: End Move

			//BUGFfH: Added by Denev 2009/09/06
			if (pUnit->isWaterWalking())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_WATER_WALKING_PEDIA"));
			}
			//BUGFfH: End Add
			/*************************************************************************************************/
			/**	END																							**/
			/*************************************************************************************************/
		}

		if (pUnit->isImmuneToDefensiveStrike())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_DEFENSIVE_STRIKE_PEDIA"));
		}
		//<<<<	More Detailed Civilopedia: End Add
		if (!bShort)
		{
			if (pUnit->noDefensiveBonus())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_NO_DEFENSE_BONUSES"));
			}
			/*************************************************************************************************/
			/**	FFHBUG denev																				**/
			/**	ADDON (FFHBUG) 																	**/
			/**	Added by Denev 2009/09/05																	**/
			/*************************************************************************************************/
			if (pUnit->isDoubleFortifyBonus())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_FORTIFY_BONUS"));
			}
			if (pUnit->isTargetWeakestUnitCounter())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_TARGET_WEAKEST_UNIT_COUNTER"));
			}
			const int iDefaultPercent = 100;
			if (pUnit->getBetterDefenderThanPercent() != iDefaultPercent)
			{
				szString.append(NEWLINE);
				if (pUnit->getBetterDefenderThanPercent() < iDefaultPercent)
				{
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_BETTER_DEFENDER_THAN_PERCENT_LESS"));
				}
				if (pUnit->getBetterDefenderThanPercent() > iDefaultPercent)
				{
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_BETTER_DEFENDER_THAN_PERCENT_MORE"));
				}
			}

			//BUGFfH: Moved to above(*2) by Denev 2009/09/06
			/*
						if (pUnit->flatMovementCost())
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_UNIT_FLAT_MOVEMENT"));
						}

						if (pUnit->ignoreTerrainCost())
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_UNIT_IGNORE_TERRAIN"));
						}
			*/
			//BUGFfH: End Move
			/*************************************************************************************************/
			/**	END																							**/
			/*************************************************************************************************/



			if (pUnit->isBlitz())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_BLITZ_TEXT"));
			}

			if (pUnit->isAmphib())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_AMPHIB_TEXT"));
			}

			if (pUnit->isRiver())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_RIVER_ATTACK_TEXT"));
			}

			/*************************************************************************************************/
			/**	FFHBUG denev																				**/
			/**	ADDON (FFHBUG) 																	**/
			/**																								**/
			/*************************************************************************************************/
			//BUGFfH: Moved to above(*3) by Denev 2009/09/06
			/*
						if (pUnit->isEnemyRoute())
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_ENEMY_ROADS_TEXT"));
						}
			*/
			//BUGFfH: End Move

			//BUGFfH: Moved to below(*4) by Denev 2009/09/06
			/*
						if (pUnit->isAlwaysHeal())
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_ALWAYS_HEAL_TEXT"));
						}
			*/
			//BUGFfH: End Move

			//BUGFfH: Moved to above(*5) by Denev 2009/09/06
			/*
						if (pUnit->isHillsDoubleMove())
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_HILLS_MOVE_TEXT"));
						}
			*/
			//BUGFfH: End Move
			//BUGFfH: Added by Denev 2009/09/06
			if (pUnit->ignoreBuildingDefense())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_IGNORE_BUILDING_DEFENSE"));
			}
			if (pUnit->isBoarding())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_BOARDING_PEDIA"));
			}

			if (pUnit->getFear() >= 100)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_FEAR_PEDIA"));
			}
			if (pUnit->isTargetWeakestUnit())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_TARGET_WEAKEST_UNIT"));
			}

			/********************************************************/
			/*	BUGFfH comment: about Immune ability				*/
			/********************************************************/
			if (pUnit->isImmuneToCapture())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_CAPTURE_PEDIA"));
			}
			if (pUnit->isImmuneToMagic())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_MAGIC_PEDIA"));
			}
			if (pUnit->isImmuneToFear())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_FEAR_PEDIA"));
			}

			bFirst = true;
			for (std::list<CvWString>::iterator it = aszPromotionImmune.begin(); it != aszPromotionImmune.end(); it++)
			{
				if (bFirst) {
					bFirst = false;
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE", (*it).GetCString()));
				}
				else {
					szTempBuffer.Format(L", %s", (*it).GetCString());
					szString.append(szTempBuffer);
				}
			}

			/********************************************************/
			/*	BUGFfH comment: about Spell ability					*/
			/********************************************************/

			/*------------------------------------------------------*/
			/*	BUGFfH comment: about After combat					*/
			/*------------------------------------------------------*/
			for (std::list<CvWString>::iterator it = aszPromotionCombatApply.begin(); it != aszPromotionCombatApply.end(); it++)
			{
				szString.append(NEWLINE);
				//szString.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_APPLY", (*it).GetCString()));
				szString.append((*it).GetCString());
			}
			for (std::list<CvWString>::iterator it = aszCaptureUnitCombat.begin(); it != aszCaptureUnitCombat.end(); it++)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_CAPTURE_UNITCOMBAT", (*it).GetCString()));
			}
			if (iTotalCombatCapturePercent != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_CAPTURE_PERCENT", iTotalCombatCapturePercent));
			}
			if (pUnit->getGoldFromCombat() != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_GOLD_FROM_COMBAT", pUnit->getGoldFromCombat()));
			}

			const int iDurationFromCombat = GC.getUnitInfo(pUnit->getUnitType()).getDurationFromCombat();
			if (iDurationFromCombat != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_DURATION_FROM_COMBAT", iDurationFromCombat));
			}

			const UnitTypes eUnitConvertFromCombat = (UnitTypes)GC.getUnitInfo(pUnit->getUnitType()).getUnitConvertFromCombat();
			const int iUnitConvertFromCombatChance = GC.getUnitInfo(pUnit->getUnitType()).getUnitConvertFromCombatChance();
			if (eUnitConvertFromCombat != NO_UNIT)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_CONVERT_FROM_COMBAT", iUnitConvertFromCombatChance, getLinkedText(eUnitConvertFromCombat).c_str()));
			}

			const UnitTypes eUnitCreateFromCombat = (UnitTypes)GC.getUnitInfo(pUnit->getUnitType()).getUnitCreateFromCombat();
			const int iUnitCreateFromCombatChance = GC.getUnitInfo(pUnit->getUnitType()).getUnitCreateFromCombatChance();
			if (eUnitCreateFromCombat != NO_UNIT)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_CREATE_FROM_COMBAT", iUnitCreateFromCombatChance, getLinkedText(eUnitCreateFromCombat).c_str()));
			}

			int iEnslavementChance = 0;
			iEnslavementChance += GC.getUnitInfo(pUnit->getUnitType()).getEnslavementChance();
			iEnslavementChance += GET_PLAYER(pUnit->getOwnerINLINE()).getEnslavementChance();
			iEnslavementChance += iUnitPromotionEnslavementChance;
			if (iEnslavementChance > 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_ENSLAVEMENT_CHANCE", iEnslavementChance));
			}

			const PromotionTypes ePromotionFromCombat = (PromotionTypes)GC.getUnitInfo(pUnit->getUnitType()).getPromotionFromCombat();
			if (ePromotionFromCombat != NO_PROMOTION)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_PROMOTION_FROM_COMBAT", getLinkedText(ePromotionFromCombat).c_str()));
			}

			if (GC.getUnitInfo(pUnit->getUnitType()).isNoWarWeariness())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_NO_WAR_WEARINESS"));
			}
			if (GC.getUnitInfo(pUnit->getUnitType()).isExplodeInCombat())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_EXPLODE_IN_COMBAT"));
			}
			if (pUnit->isImmortal())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_IMMORTAL_PEDIA"));
			}
			if (pUnit->getCombatHealPercent() != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_HEAL_PERCENT", pUnit->getCombatHealPercent()));
			}
			//<<<<	BUGFfH: End Add

			//BUGFfH: Moved to above(*6) by Denev 2009/09/06
			/*
						for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
						{
							if (pUnit->isTerrainDoubleMove((TerrainTypes)iI))
							{
								szString.append(NEWLINE);
								szString.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide()));
							}
						}

						for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
						{
							if (pUnit->isFeatureDoubleMove((FeatureTypes)iI))
							{
								szString.append(NEWLINE);
								szString.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide()));
							}
						}
			*/
			//BUGFfH: End Move

			//BUGFfH: Moved to above(*7) by Denev 2009/09/06
			/*
						if (pUnit->getExtraVisibilityRange() != 0)
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_VISIBILITY_TEXT", pUnit->getExtraVisibilityRange()));
						}
			*/
			//BUGFfH: End Move

			//BUGFfH: Moved to above(*8) by Denev 2009/09/06
			/*
						if (pUnit->getExtraMoveDiscount() != 0)
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_PROMOTION_MOVE_DISCOUNT_TEXT", -(pUnit->getExtraMoveDiscount())));
						}
			*/
			//BUGFfH: End Move

			//BUGFfH: Moved from above(*4) by Denev 2009/09/06
			if (pUnit->isAlwaysHeal())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_ALWAYS_HEAL_TEXT"));
			}
			//BUGFfH: End Move

			//BUGFfH: Added by Denev 2009/09/06
			if (pUnit->getExtraEnemyHeal() == pUnit->getExtraNeutralHeal() && pUnit->getExtraNeutralHeal() == pUnit->getExtraFriendlyHeal())
			{
				if (pUnit->getExtraEnemyHeal() != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", pUnit->getExtraEnemyHeal()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT"));
				}
			}
			else
			{
				//BUGFfH: End Add
				if (pUnit->getExtraEnemyHeal() != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", pUnit->getExtraEnemyHeal()) + gDLL->getText("TXT_KEY_PROMOTION_ENEMY_LANDS_TEXT"));
				}

				if (pUnit->getExtraNeutralHeal() != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", pUnit->getExtraNeutralHeal()) + gDLL->getText("TXT_KEY_PROMOTION_NEUTRAL_LANDS_TEXT"));
				}

				if (pUnit->getExtraFriendlyHeal() != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", pUnit->getExtraFriendlyHeal()) + gDLL->getText("TXT_KEY_PROMOTION_FRIENDLY_LANDS_TEXT"));
				}
				//BUGFfH: Added by Denev 2009/09/06
			}
			/*************************************************************************************************/
			/**	END																							**/
			/*************************************************************************************************/

			if (pUnit->getSameTileHeal() != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_SAME_TEXT", pUnit->getSameTileHeal()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT"));
			}

			if (pUnit->getAdjacentTileHeal() != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_ADJACENT_TEXT", pUnit->getAdjacentTileHeal()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT"));
			}
		}

		if (pUnit->currInterceptionProbability() > 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_INTERCEPT_AIRCRAFT", pUnit->currInterceptionProbability()));
		}

		if (pUnit->evasionProbability() > 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_EVADE_INTERCEPTION", pUnit->evasionProbability()));
		}

		if (pUnit->withdrawalProbability() > 0)
		{
			if (bShort)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY_SHORT", pUnit->withdrawalProbability()));
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY", pUnit->withdrawalProbability()));
			}

		}

		//>>>>	BUGFfH: Added by Denev 2009/09/10
		if (pUnit->getWithdrawlProbDefensive() > 0)
		{
			if (bShort)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY_DEFENSIVE_SHORT", pUnit->getWithdrawlProbDefensive()));
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY_DEFENSIVE", pUnit->getWithdrawlProbDefensive()));
			}
		}
		//<<<<	BUGFfH: End Add

		if (pUnit->combatLimit() < GC.getMAX_HIT_POINTS() && pUnit->canAttack())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_COMBAT_LIMIT", (100 * pUnit->combatLimit()) / GC.getMAX_HIT_POINTS()));
		}

		if (pUnit->collateralDamage() > 0)
		{
			szString.append(NEWLINE);
			if (pUnit->getExtraCollateralDamage() == 0)
			{
				szString.append(gDLL->getText("TXT_KEY_UNIT_COLLATERAL_DAMAGE", (100 * pUnit->getUnitInfo().getCollateralDamageLimit() / GC.getMAX_HIT_POINTS())));
			}
			else
			{
				szString.append(gDLL->getText("TXT_KEY_UNIT_COLLATERAL_DAMAGE_EXTRA", pUnit->getExtraCollateralDamage()));
			}
		}

		for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
		{
			if (pUnit->getUnitInfo().getUnitCombatCollateralImmune(iI))
			{
				szString.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szString.append(gDLL->getText("TXT_KEY_UNIT_COLLATERAL_IMMUNE", GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide()));
				szString.append(gDLL->getText("TXT_KEY_UNIT_COLLATERAL_IMMUNE", getLinkedText((UnitCombatTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify

			}
		}

		if (pUnit->getCollateralDamageProtection() > 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PROMOTION_COLLATERAL_PROTECTION_TEXT", pUnit->getCollateralDamageProtection()));
		}

		if (pUnit->animalCombatModifier() != 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_ANIMAL_COMBAT_MOD", pUnit->animalCombatModifier()));
		}

		if (pUnit->getDropRange() > 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_PARADROP_RANGE", pUnit->getDropRange()));
		}

		/*************************************************************************************************/
		/**	FFHBUG denev																				**/
		/**	ADDON (FFHBUG) 																	**/
		/**																								**/
		/*************************************************************************************************/

		//BUGFfH: Moved from above(*1) by Denev 2009/09/05
		/*
				for (int iJ=0;iJ<GC.getNumPromotionInfos();iJ++)
				{
					if (pUnit->isHasPromotion((PromotionTypes)iJ) && GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod()>0)
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod(), GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()).getTextKeyWide()));
					}
				}
		*/
		for (int iPromotionCombatType = 0; iPromotionCombatType < GC.getNumPromotionInfos(); iPromotionCombatType++)
		{
			if (aiPromotionCombatMod[iPromotionCombatType] != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", aiPromotionCombatMod[iPromotionCombatType], getLinkedText((PromotionTypes)iPromotionCombatType).c_str()));
			}
		}
		//BUGFfH: End Move
		/*************************************************************************************************/
		/**	END																							**/
		/*************************************************************************************************/
		for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
		{
			if (pUnit->getUnitInfo().getUnitClassAttackModifier(iI) == GC.getUnitInfo(pUnit->getUnitType()).getUnitClassDefenseModifier(iI))
			{
				if (pUnit->getUnitInfo().getUnitClassAttackModifier(iI) != 0)
				{
					szString.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/03
					//					szString.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->getUnitInfo().getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide()));
					szString.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->getUnitInfo().getUnitClassAttackModifier(iI), getLinkedText((UnitClassTypes)iI).c_str()));
					//<<<<	BUGFfH: End Modify

				}
			}
			else
			{
				if (pUnit->getUnitInfo().getUnitClassAttackModifier(iI) != 0)
				{
					szString.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/03
					//					szString.append(gDLL->getText("TXT_KEY_UNIT_ATTACK_MOD_VS_CLASS", pUnit->getUnitInfo().getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide()));
					szString.append(gDLL->getText("TXT_KEY_UNIT_ATTACK_MOD_VS_CLASS", pUnit->getUnitInfo().getUnitClassAttackModifier(iI), getLinkedText((UnitClassTypes)iI).c_str()));
					//<<<<	BUGFfH: End Modify

				}

				if (pUnit->getUnitInfo().getUnitClassDefenseModifier(iI) != 0)
				{
					szString.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/03
					//					szString.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE_MOD_VS_CLASS", pUnit->getUnitInfo().getUnitClassDefenseModifier(iI), GC.getUnitClassInfo((UnitClassTypes) iI).getTextKeyWide()));
					szString.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE_MOD_VS_CLASS", pUnit->getUnitInfo().getUnitClassDefenseModifier(iI), getLinkedText((UnitClassTypes)iI).c_str()));
					//<<<<	BUGFfH: End Modify

				}
			}
		}

		for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
		{
			if (pUnit->unitCombatModifier((UnitCombatTypes)iI) != 0)
			{
				szString.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szString.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->unitCombatModifier((UnitCombatTypes)iI), GC.getUnitCombatInfo((UnitCombatTypes) iI).getTextKeyWide()));
				szString.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->unitCombatModifier((UnitCombatTypes)iI), getLinkedText((UnitCombatTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify

			}
		}

		for (iI = 0; iI < NUM_DOMAIN_TYPES; ++iI)
		{
			if (pUnit->domainModifier((DomainTypes)iI) != 0)
			{
				szString.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szString.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->domainModifier((DomainTypes)iI), GC.getDomainInfo((DomainTypes)iI).getTextKeyWide()));
				szString.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", pUnit->domainModifier((DomainTypes)iI), getLinkedText((DomainTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify

			}
		}
		/*************************************************************************************************/
		/**	FFHBUG denev																				**/
		/**	ADDON (FFHBUG) 																	**/
		/**																								**/
		/*************************************************************************************************/
		//BUGFfH: Added by Denev 2009/09/05
				/********************************************************/
				/*	BUGFfH comment: about Damage types ability			*/
				/********************************************************/

		if (!bShort)
		{
			for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (pUnit->getBonusAffinity((BonusTypes)iI) != 0)
				{
					szString.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/03
					//					szString.append(gDLL->getText("TXT_KEY_BONUS_AFFINITY", pUnit->getBonusAffinity((BonusTypes)iI), GC.getBonusInfo((BonusTypes)iI).getTextKeyWide()));
					szString.append(gDLL->getText("TXT_KEY_BONUS_AFFINITY", pUnit->getBonusAffinity((BonusTypes)iI), getLinkedText((BonusTypes)iI).c_str()));
					//<<<<	BUGFfH: End Modify

				}
			}

			/********************************************************/
			/*	BUGFfH comment: about Armageddon counter ability	*/
			/********************************************************/
			if (iTotalModifyGlobalCounter != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_MESSAGE_MODIFY_GLOBAL_COUNTER", iTotalModifyGlobalCounter));
			}
			if (iTotalModifyGlobalCounterOnCombat != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_MESSAGE_MODIFY_GLOBAL_COUNTER_ON_COMBAT", iTotalModifyGlobalCounterOnCombat));
			}

			/********************************************************/
			/*	BUGFfH comment: about Miscellaneous					*/
			/********************************************************/
			if (pUnit->isHiddenNationality())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_ALWAYS_HOSTILE"));
			}
			if (iTotalFreeXPFromCombat != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_FREE_XP_FROM_COMBAT", iTotalFreeXPFromCombat));
			}

			const int iXPGainChance = std::min(100, pUnit->getSpellCasterXP() - pUnit->getExperience());	//This must be reflected from CvUnit::doTurn()
			if (iXPGainChance > 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_FREE_XP_CHANCE_PER_TURN", iXPGainChance));
			}

			if (iTotalFreeXPPerTurn != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PROMOTION_FREE_XP_PER_TURN", iTotalFreeXPPerTurn, GC.getDefineINT("FREE_XP_MAX")));
			}
		}

		//BUGFfH: Moved from below(*9) by Denev 2009/09/07
		if (pUnit->bombardRate() > 0 && pUnit->getUnitInfo().getBombardRange() > 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_BOMBARD_RANGE", pUnit->getUnitInfo().getBombardRange()));

			if (bShort)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE_NEW", ((pUnit->bombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE())));
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE_NEW", ((pUnit->bombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE())));
			}
		}
		//BUGFfH: End Move

				/********************************************************/
				/*	BUGFfH comment: about Work rate modifier			*/
				/********************************************************/
		if (pUnit->getWorkRateModify() != 0)
		{
			for (iI = 0; iI < GC.getNumBuildInfos(); ++iI)
			{
				if (GC.getUnitInfo(pUnit->getUnitType()).getBuilds(iI))
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_PROMOTION_WORK_RATE_MODIFY", pUnit->getWorkRateModify()));
					break;
				}
			}
		}
		//BUGFfH: End Add
		/*************************************************************************************************/
		/**	END																							**/
		/*************************************************************************************************/
		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
		{
			if (pUnit->getUnitInfo().getTargetUnitClass(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
				szTempBuffer += getLinkedText((UnitClassTypes)iI);
				//<<<<	BUGFfH: End Modify

			}
		}

		if (!bFirst)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString()));
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
		{
			if (pUnit->getUnitInfo().getDefenderUnitClass(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
				szTempBuffer += getLinkedText((UnitClassTypes)iI);
				//<<<<	BUGFfH: End Modify

			}
		}

		if (!bFirst)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString()));
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
		{
			if (pUnit->getUnitInfo().getTargetUnitCombat(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
				szTempBuffer += getLinkedText((UnitCombatTypes)iI);
				//<<<<	BUGFfH: End Modify

			}
		}

		if (!bFirst)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString()));
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
		{
			if (pUnit->getUnitInfo().getDefenderUnitCombat(iI))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
				szTempBuffer += getLinkedText((UnitCombatTypes)iI);
				//<<<<	BUGFfH: End Modify

			}
		}

		if (!bFirst)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString()));
		}

		szTempBuffer.clear();
		bFirst = true;
		for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
		{
			if (pUnit->getUnitInfo().getFlankingStrikeUnitClass(iI) > 0)
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szTempBuffer += L", ";
				}

				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
				szTempBuffer += getLinkedText((UnitClassTypes)iI);
				//<<<<	BUGFfH: End Modify

			}
		}

		if (!bFirst)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_FLANKING_STRIKES", szTempBuffer.GetCString()));
		}

		/*************************************************************************************************/
		/**	FFHBUG denev																				**/
		/**	ADDON (FFHBUG) 																	**/
		/**																								**/
		/*************************************************************************************************/
		//BUGFfH: Moved to above(*9) by Denev 2009/09/07
		/*

				if (pUnit->bombardRate() > 0)
				{
					if (bShort)
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE_SHORT", ((pUnit->bombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE())));
					}
					else
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE", ((pUnit->bombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE())));
					}
				}
		*/
		//BUGFfH: End Move
		/*************************************************************************************************/
		/**	END																							**/
		/*************************************************************************************************/
		if (pUnit->getUnitInfo().isNoRevealMap())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_VISIBILITY_MOVE_RANGE"));
		}

		if (!CvWString(pUnit->getUnitInfo().getHelp()).empty())
		{
			szString.append(NEWLINE);
			szString.append(pUnit->getUnitInfo().getHelp());
		}

		if (pUnit->getUnitInfo().getPopulationCost() > 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_BUILDING_POPULATION_COST_HELP", pUnit->getUnitInfo().getPopulationCost()));
		}


		if (pUnit->getUnitInfo().getBonusNearUF() != 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_UNIT_BONUS_NEAR_UF", pUnit->getUnitInfo().getBonusNearUF()));
		}

		bool bFirst = true;
		for (int i = 0; i < NUM_COMMERCE_TYPES; ++i) {
			int iCommerceChange = pUnit->getUnitInfo().getCityCommerceChanges(i);
			if (!pUnit->plot()->isCity() || pUnit->plot()->getOwnerINLINE() != pUnit->plot()->getOwnerINLINE()) {
				iCommerceChange = 0;
			}
			else {
				iCommerceChange *= 100 + pUnit->plot()->getPlotCity()->getCommerceRateModifier((CommerceTypes)i);
				iCommerceChange /= 100;
			}
			if (iCommerceChange != 0) {
				if (bFirst) {
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_BUILDING_CITYCOMMERCE_BASE_HELP"));
					szString.append(CvWString::format(L"%d%c", iCommerceChange, GC.getCommerceInfo((CommerceTypes)i).getChar()));
					bFirst = false;
				}
				else {
					szString.append(L", ");
					szString.append(CvWString::format(L"%d%c", iCommerceChange, GC.getCommerceInfo((CommerceTypes)i).getChar()));
				}
			}
		}
}
		if (bShift && (gDLL->getChtLvl() > 0))
		{
			szTempBuffer.Format(L"\nUnitAI Type = %s.", GC.getUnitAIInfo(pUnit->AI_getUnitAIType()).getDescription());
			szString.append(szTempBuffer);
			szTempBuffer.Format(L"\nSacrifice Value = %d.", pUnit->AI_sacrificeValue(NULL));
			szString.append(szTempBuffer);
		}

		// Combat Auras, Sephi

		if (pUnit->getCombatAura() != NULL)
		{
			if(!bAlt && !bShift && bCtrl)
			{
				setCombatAuraStr(szTempBuffer, pUnit->getCombatAura(), pUnit, 0);
				szString.append(NEWLINE);
				szString.append(szTempBuffer);
				szTempBuffer.clear();
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(L"Use CTRL to see Combat Aura");
				szTempBuffer.clear();
			}
		}
		if (pUnit->getBlessing1() != NULL)
		{
			if(!bAlt && bShift && !bCtrl)
			{
				setCombatAuraStr(szTempBuffer, pUnit->getBlessing1(), pUnit, 3);
				szString.append(NEWLINE);
				szString.append(szTempBuffer);
				szTempBuffer.clear();
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(L"Use SHIFT to see applied first Blessings");
				szTempBuffer.clear();
			}
		}
		if (pUnit->getBlessing2() != NULL)
		{
			if(bAlt && !bShift && !bCtrl)
			{
				setCombatAuraStr(szTempBuffer, pUnit->getBlessing2(), pUnit, 4);
				szString.append(NEWLINE);
				szString.append(szTempBuffer);
				szTempBuffer.clear();
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(L"Use ALT to see applied second Blessings");
				szTempBuffer.clear();
			}
		}
		if (pUnit->getCurse1() != NULL)
		{
			if(bAlt && !bShift && bCtrl)
			{
				setCombatAuraStr(szTempBuffer, pUnit->getCurse1(), pUnit, 1);
				szString.append(NEWLINE);
				szString.append(szTempBuffer);
				szTempBuffer.clear();
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(L"Use CTRL + ALT to see applied first Curses");
				szTempBuffer.clear();
			}
		}
		if (pUnit->getCurse2() != NULL)
		{
			if(!bAlt && bShift && bCtrl)
			{
				setCombatAuraStr(szTempBuffer, pUnit->getCurse2(), pUnit, 2);
				szString.append(NEWLINE);
				szString.append(szTempBuffer);
				szTempBuffer.clear();
			}
			else
			{
				szString.append(NEWLINE);
				szString.append(L"Use CTRL + SHIFT to see applied second Curses");
				szTempBuffer.clear();
			}
		}
	}

	for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++) {
		if (pUnit->getUnitInfo().getBuildings(iI) == 1) {
			bool foundCity = false;
			int cityCanBuildCounter = 0;
			int iLoop = 0;
			for (CvCity* pLoopCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).nextCity(&iLoop))
			{
				if (pLoopCity->canConstruct((BuildingTypes)iI, true, false, true)) {
					if (foundCity == false) {
						szString.append(CvWString::format(NEWLINE L"%cCan build %s in ", gDLL->getSymbolID(BULLET_CHAR), GC.getBuildingInfo((BuildingTypes)iI).getDescription()));
						szString.append(pLoopCity->getName());
						foundCity = true;
					} 
					cityCanBuildCounter++;
				}

			}
			if (cityCanBuildCounter > 2) {
				szString.append(CvWString::format(L" and %d other cities.", cityCanBuildCounter - 1));
			}
			else if (cityCanBuildCounter > 1) {
				szString.append(CvWString::format(L" and %d other city.", cityCanBuildCounter - 1));
			}
		}
	}

	for (int iI = 0; iI < GC.getNumSpellInfos(); iI++) {
		if (GC.getSpellInfo((SpellTypes) iI).getCreateBuildingType() != NO_BUILDING) {
			bool foundCity = false;
			int cityCanBuildCounter = 0;
			int iLoop = 0;
			for (CvCity* pLoopCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).nextCity(&iLoop))
			{
				if (pUnit->canCast(iI, false, pLoopCity->plot())) {
					if (foundCity == false) {
						szString.append(CvWString::format(NEWLINE L"%cCan build %s in ", gDLL->getSymbolID(BULLET_CHAR), GC.getBuildingInfo((BuildingTypes) GC.getSpellInfo((SpellTypes)iI).getCreateBuildingType()).getDescription()));
						szString.append(pLoopCity->getName());
						foundCity = true;
					}
					cityCanBuildCounter++;
				}

			}
			if (cityCanBuildCounter > 2) {
				szString.append(CvWString::format(L" and %d other cities.", cityCanBuildCounter - 1));
			}
			else if (cityCanBuildCounter > 1) {
				szString.append(CvWString::format(L" and %d other city.", cityCanBuildCounter - 1));
			}
		}
	}
}


void CvGameTextMgr::setPlotListHelp(CvWStringBuffer& szString, CvPlot* pPlot, bool bOneLine, bool bShort)
{
	PROFILE_FUNC();

	int numPromotionInfos = GC.getNumPromotionInfos();

	// if cheatmode and ctrl, display grouping info instead
	if ((gDLL->getChtLvl() > 0) && gDLL->ctrlKey())
	{
		if (pPlot->isVisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
		{
			CvWString szTempString;

			CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
			while (pUnitNode != NULL)
			{
				CvUnit* pHeadUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pPlot->nextUnitNode(pUnitNode);

				// is this unit the head of a group, not cargo, and visible?
				if (pHeadUnit && pHeadUnit->isGroupHead() && !pHeadUnit->isCargo() && !pHeadUnit->isInvisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
				{
					// head unit name and unitai
					szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR, 255, 190, 0, 255, pHeadUnit->getName().GetCString()));
					szString.append(CvWString::format(L" (%d)", shortenID(pHeadUnit->getID())));
					getUnitAIString(szTempString, pHeadUnit->AI_getUnitAIType());
					szString.append(CvWString::format(SETCOLR L" %s " ENDCOLR, GET_PLAYER(pHeadUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pHeadUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pHeadUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pHeadUnit->getOwnerINLINE()).getPlayerTextColorA(), szTempString.GetCString()));

					// promotion icons
					for (int iPromotionIndex = 0; iPromotionIndex < numPromotionInfos; iPromotionIndex++)
					{
						PromotionTypes ePromotion = (PromotionTypes)iPromotionIndex;
						if (pHeadUnit->isHasPromotion(ePromotion))
						{
							szString.append(CvWString::format(L"<img=%S size=16></img>", GC.getPromotionInfo(ePromotion).getButton()));
						}
					}

					// group
					CvSelectionGroup* pHeadGroup = pHeadUnit->getGroup();
					FAssertMsg(pHeadGroup != NULL, "unit has NULL group");
					if (pHeadGroup->getNumUnits() > 1)
					{
						szString.append(CvWString::format(L"\nGroup:%d [%d units]", shortenID(pHeadGroup->getID()), pHeadGroup->getNumUnits()));

						// get average damage
						int iAverageDamage = 0;
						CLLNode<IDInfo>* pUnitNode = pHeadGroup->headUnitNode();
						while (pUnitNode != NULL)
						{
							CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = pHeadGroup->nextUnitNode(pUnitNode);

							iAverageDamage += (pLoopUnit->getDamage() * pLoopUnit->maxHitPoints()) / 100;
						}
						iAverageDamage /= pHeadGroup->getNumUnits();
						if (iAverageDamage > 0)
						{
							szString.append(CvWString::format(L" %d%%", 100 - iAverageDamage));
						}
					}

					// mission ai
					MissionAITypes eMissionAI = pHeadGroup->AI_getMissionAIType();
					if (eMissionAI != NO_MISSIONAI)
					{
						getMissionAIString(szTempString, eMissionAI);
						szString.append(CvWString::format(SETCOLR L"\n%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), szTempString.GetCString()));
					}

					// mission
					MissionTypes eMissionType = (MissionTypes)pHeadGroup->getMissionType(0);
					if (eMissionType != NO_MISSION)
					{
						getMissionTypeString(szTempString, eMissionType);
						szString.append(CvWString::format(SETCOLR L"\n%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), szTempString.GetCString()));
					}

					// mission unit
					CvUnit* pMissionUnit = pHeadGroup->AI_getMissionAIUnit();
					if (pMissionUnit != NULL && (eMissionAI != NO_MISSIONAI || eMissionType != NO_MISSION))
					{
						// mission unit
						szString.append(L"\n to ");
						szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorA(), pMissionUnit->getName().GetCString()));
						szString.append(CvWString::format(L"(%d) G:%d", shortenID(pMissionUnit->getID()), shortenID(pMissionUnit->getGroupID())));
						getUnitAIString(szTempString, pMissionUnit->AI_getUnitAIType());
						szString.append(CvWString::format(SETCOLR L" %s" ENDCOLR, GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pMissionUnit->getOwnerINLINE()).getPlayerTextColorA(), szTempString.GetCString()));
					}

					// mission plot
					if (eMissionAI != NO_MISSIONAI || eMissionType != NO_MISSION)
					{
						// first try the plot from the missionAI
						CvPlot* pMissionPlot = pHeadGroup->AI_getMissionAIPlot();

						// if MissionAI does not have a plot, get one from the mission itself
						if (pMissionPlot == NULL && eMissionType != NO_MISSION)
						{
							switch (eMissionType)
							{
							case MISSION_MOVE_TO:
							case MISSION_ROUTE_TO:
								pMissionPlot = GC.getMapINLINE().plotINLINE(pHeadGroup->getMissionData1(0), pHeadGroup->getMissionData2(0));
								break;

							case MISSION_MOVE_TO_UNIT:
								if (pMissionUnit != NULL)
								{
									pMissionPlot = pMissionUnit->plot();
								}
								break;
							}
						}

						if (pMissionPlot != NULL)
						{
							szString.append(CvWString::format(L"\n [%d,%d]", pMissionPlot->getX_INLINE(), pMissionPlot->getY_INLINE()));

							CvCity* pCity = pMissionPlot->getWorkingCity();
							if (pCity != NULL)
							{
								szString.append(L" (");

								if (!pMissionPlot->isCity())
								{
									DirectionTypes eDirection = estimateDirection(dxWrap(pMissionPlot->getX_INLINE() - pCity->plot()->getX_INLINE()), dyWrap(pMissionPlot->getY_INLINE() - pCity->plot()->getY_INLINE()));

									getDirectionTypeString(szTempString, eDirection);
									szString.append(CvWString::format(L"%s of ", szTempString.GetCString()));
								}

								szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR L")", GET_PLAYER(pCity->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pCity->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pCity->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pCity->getOwnerINLINE()).getPlayerTextColorA(), pCity->getName().GetCString()));
							}
							else
							{
								if (pMissionPlot != pPlot)
								{
									DirectionTypes eDirection = estimateDirection(dxWrap(pMissionPlot->getX_INLINE() - pPlot->getX_INLINE()), dyWrap(pMissionPlot->getY_INLINE() - pPlot->getY_INLINE()));

									getDirectionTypeString(szTempString, eDirection);
									szString.append(CvWString::format(L" (%s)", szTempString.GetCString()));
								}

								PlayerTypes eMissionPlotOwner = pMissionPlot->getOwnerINLINE();
								if (eMissionPlotOwner != NO_PLAYER)
								{
									szString.append(CvWString::format(L", " SETCOLR L"%s" ENDCOLR, GET_PLAYER(eMissionPlotOwner).getPlayerTextColorR(), GET_PLAYER(eMissionPlotOwner).getPlayerTextColorG(), GET_PLAYER(eMissionPlotOwner).getPlayerTextColorB(), GET_PLAYER(eMissionPlotOwner).getPlayerTextColorA(), GET_PLAYER(eMissionPlotOwner).getName()));
								}
							}
						}
					}

					// display cargo for head unit
					std::vector<CvUnit*> aCargoUnits;
					pHeadUnit->getCargoUnits(aCargoUnits);
					for (uint i = 0; i < aCargoUnits.size(); ++i)
					{
						CvUnit* pCargoUnit = aCargoUnits[i];
						if (!pCargoUnit->isInvisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
						{
							// name and unitai
							szString.append(CvWString::format(SETCOLR L"\n %s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), pCargoUnit->getName().GetCString()));
							szString.append(CvWString::format(L"(%d)", shortenID(pCargoUnit->getID())));
							getUnitAIString(szTempString, pCargoUnit->AI_getUnitAIType());
							szString.append(CvWString::format(SETCOLR L" %s " ENDCOLR, GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorA(), szTempString.GetCString()));

							// promotion icons
							for (int iPromotionIndex = 0; iPromotionIndex < numPromotionInfos; iPromotionIndex++)
							{
								PromotionTypes ePromotion = (PromotionTypes)iPromotionIndex;
								if (pCargoUnit->isHasPromotion(ePromotion))
								{
									szString.append(CvWString::format(L"<img=%S size=16></img>", GC.getPromotionInfo(ePromotion).getButton()));
								}
							}
						}
					}

					// display grouped units
					CLLNode<IDInfo>* pUnitNode3 = pPlot->headUnitNode();
					while (pUnitNode3 != NULL)
					{
						CvUnit* pUnit = ::getUnit(pUnitNode3->m_data);
						pUnitNode3 = pPlot->nextUnitNode(pUnitNode3);

						// is this unit not head, in head's group and visible?
						if (pUnit && (pUnit != pHeadUnit) && (pUnit->getGroupID() == pHeadUnit->getGroupID()) && !pUnit->isInvisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
						{
							FAssertMsg(!pUnit->isCargo(), "unit is cargo but head unit is not cargo");
							// name and unitai
							szString.append(CvWString::format(SETCOLR L"\n-%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), pUnit->getName().GetCString()));
							szString.append(CvWString::format(L" (%d)", shortenID(pUnit->getID())));
							getUnitAIString(szTempString, pUnit->AI_getUnitAIType());
							szString.append(CvWString::format(SETCOLR L" %s " ENDCOLR, GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pUnit->getOwnerINLINE()).getPlayerTextColorA(), szTempString.GetCString()));

							// promotion icons
							for (int iPromotionIndex = 0; iPromotionIndex < numPromotionInfos; iPromotionIndex++)
							{
								PromotionTypes ePromotion = (PromotionTypes)iPromotionIndex;
								if (pUnit->isHasPromotion(ePromotion))
								{
									szString.append(CvWString::format(L"<img=%S size=16></img>", GC.getPromotionInfo(ePromotion).getButton()));
								}
							}

							// display cargo for loop unit
							std::vector<CvUnit*> aLoopCargoUnits;
							pUnit->getCargoUnits(aLoopCargoUnits);
							for (uint i = 0; i < aLoopCargoUnits.size(); ++i)
							{
								CvUnit* pCargoUnit = aLoopCargoUnits[i];
								if (!pCargoUnit->isInvisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
								{
									// name and unitai
									szString.append(CvWString::format(SETCOLR L"\n %s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), pCargoUnit->getName().GetCString()));
									szString.append(CvWString::format(L"(%d)", shortenID(pCargoUnit->getID())));
									getUnitAIString(szTempString, pCargoUnit->AI_getUnitAIType());
									szString.append(CvWString::format(SETCOLR L" %s " ENDCOLR, GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorR(), GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorG(), GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorB(), GET_PLAYER(pCargoUnit->getOwnerINLINE()).getPlayerTextColorA(), szTempString.GetCString()));

									// promotion icons
									for (int iPromotionIndex = 0; iPromotionIndex < numPromotionInfos; iPromotionIndex++)
									{
										PromotionTypes ePromotion = (PromotionTypes)iPromotionIndex;
										if (pCargoUnit->isHasPromotion(ePromotion))
										{
											szString.append(CvWString::format(L"<img=%S size=16></img>", GC.getPromotionInfo(ePromotion).getButton()));
										}
									}
								}
							}
						}
					}

					// double space non-empty groups
					if (pHeadGroup->getNumUnits() > 1 || pHeadUnit->hasCargo())
					{
						szString.append(NEWLINE);
					}

					szString.append(NEWLINE);
				}
			}
		}

		return;
	}

	// lfgr UI 11/2020: Allow cycling through units in plot help
	PlotHelpCyclingManager::getInstance().updateCurrentPlot( GC.getMapINLINE().plotNumINLINE( pPlot->getX_INLINE(), pPlot->getY_INLINE() ) );

	CvUnit* pLoopUnit;
	static const uint iMaxNumUnits = GC.getDefineINT("PLOT_HELP_NUM_UNITS");
//	static std::vector<CvUnit*> apUnits;
	static std::vector<int> aiUnitNumbers;
	static std::vector<int> aiUnitStrength;
	static std::vector<int> aiUnitMaxStrength;
	static std::vector<CvUnit*> plotUnits;
	std::vector<std::pair<int, std::vector<std::pair<int, int> > > > unitInfoPromotionCounts;	//number of promotions for the unit info; unit info number, promotion number, count of promotions

	GC.getGameINLINE().getPlotUnits(pPlot, plotUnits);

	//int iNumVisibleUnits = 0;
	uint iNumVisibleUnits = 0;
	if (pPlot->isVisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
	{
		CLLNode<IDInfo>* pUnitNode5 = pPlot->headUnitNode();
		while (pUnitNode5 != NULL)
		{
			CvUnit* pUnit = ::getUnit(pUnitNode5->m_data);
			pUnitNode5 = pPlot->nextUnitNode(pUnitNode5);

			if (pUnit && !pUnit->isInvisible(GC.getGameINLINE().getActiveTeam(), GC.getGameINLINE().isDebugMode()))
			{
				++iNumVisibleUnits;
			}
		}
	}

//	apUnits.erase(apUnits.begin(), apUnits.end());

	if (iNumVisibleUnits > iMaxNumUnits)
	{
		aiUnitNumbers.erase(aiUnitNumbers.begin(), aiUnitNumbers.end());
		aiUnitStrength.erase(aiUnitStrength.begin(), aiUnitStrength.end());
		aiUnitMaxStrength.erase(aiUnitMaxStrength.begin(), aiUnitMaxStrength.end());

/*		if (m_apbPromotion.size() == 0)
		{
			for (int iI = 0; iI < (GC.getNumUnitInfos() * MAX_PLAYERS); ++iI)
			{
				m_apbPromotion.push_back(new int[numPromotionInfos]);
			}
		}*/

		for (int iI = 0; iI < (GC.getNumUnitInfos() * MAX_PLAYERS); ++iI)
		{
			aiUnitNumbers.push_back(0);
			aiUnitStrength.push_back(0);
			aiUnitMaxStrength.push_back(0);
			/*for (int iJ = 0; iJ < numPromotionInfos; iJ++)	//This executes something like 37m times and hurts performance unacceptably.  We only need to initialize what is actually used.
			{
				m_apbPromotion[iI][iJ] = 0;
			}*/
		}
	}

	int iCount = 0;
	for (uint iI = iMaxNumUnits; iI < iNumVisibleUnits && iI < (int)plotUnits.size(); ++iI)
	//for (int iI = iMaxNumUnits; iI < iNumVisibleUnits && iI < (int)plotUnits.size(); ++iI)
	{
		pLoopUnit = plotUnits[iI];

		if (pLoopUnit != NULL && pLoopUnit != pPlot->getCenterUnit())
		{
			int iIndex = pLoopUnit->getUnitType() * MAX_PLAYERS + pLoopUnit->getOwner();
			if (aiUnitNumbers[iIndex] == 0)
			{
				++iCount;
			}
			++aiUnitNumbers[iIndex];

			int iBase = (DOMAIN_AIR == pLoopUnit->getDomainType() ? pLoopUnit->airBaseCombatStr() : pLoopUnit->baseCombatStr());
			if (iBase > 0 && pLoopUnit->maxHitPoints() > 0)
			{
				aiUnitMaxStrength[iIndex] += 100 * iBase;
				aiUnitStrength[iIndex] += (100 * iBase * pLoopUnit->currHitPoints()) / pLoopUnit->maxHitPoints();
			}

			for (int iJ = 0; iJ < numPromotionInfos; iJ++)
			{
				if (pLoopUnit->isHasPromotion((PromotionTypes)iJ))
				{
					bool foundUnit = false;
					for (size_t iUnit = 0; iUnit < unitInfoPromotionCounts.size(); iUnit++) {
						std::pair<int, std::vector<std::pair<int, int> > >& promotions = unitInfoPromotionCounts[iUnit];
						if (promotions.first == iIndex) {
							foundUnit = true;
							bool foundPromotion = false;
							for (size_t iPromotion = 0; iPromotion < promotions.second.size(); iPromotion++) {
								std::pair<int, int>& promotionCount = promotions.second[iPromotion];
								if (promotionCount.first == iJ) {
									promotionCount.second = promotionCount.second > 0 ? promotionCount.second + 1 : 1;
									foundPromotion = true;
								}
							}
								
							if (!foundPromotion)
								promotions.second.push_back(std::make_pair(iJ, 1));
						}
					}
					if (!foundUnit) {
						std::vector<std::pair<int, int> > noPromotion(1, std::make_pair(iJ, 1));
						std::pair<int, std::vector<std::pair<int, int> > > unit = std::make_pair(iIndex, noPromotion);
						unitInfoPromotionCounts.push_back(unit);
					}
				}
			}
		}
	}


	if (iNumVisibleUnits > 0)
	{
		if (pPlot->getCenterUnit())
		{
			setUnitHelp(szString, pPlot->getCenterUnit(), iNumVisibleUnits > iMaxNumUnits, true);
		}

		uint iNumShown = std::min<uint>(iMaxNumUnits, iNumVisibleUnits);
		for (uint iI = 0; iI < iNumShown && iI < (int)plotUnits.size(); ++iI)
		{
			CvUnit* pLoopUnit = plotUnits[iI];
			if (pLoopUnit != pPlot->getCenterUnit())
			{
				szString.append(NEWLINE);
				setUnitHelp(szString, pLoopUnit, true, true);
			}
		}

		bool bFirst = true;
		if (iNumVisibleUnits > iMaxNumUnits)
		{
			for (int iI = 0; iI < GC.getNumUnitInfos(); ++iI)
			{
				for (int iJ = 0; iJ < MAX_PLAYERS; iJ++)
				{
					int iIndex = iI * MAX_PLAYERS + iJ;

					if (aiUnitNumbers[iIndex] > 0)
					{
						if (iCount < 5 || bFirst)
						{
							szString.append(NEWLINE);
							bFirst = false;
						}
						else
						{
							szString.append(L", ");
						}
						szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo((UnitTypes)iI).getDescription()));

						szString.append(CvWString::format(L" (%d)", aiUnitNumbers[iIndex]));

						if (aiUnitMaxStrength[iIndex] > 0)
						{
							int iBase = (aiUnitMaxStrength[iIndex] / aiUnitNumbers[iIndex]) / 100;
							int iCurrent = (aiUnitStrength[iIndex] / aiUnitNumbers[iIndex]) / 100;
							int iCurrent100 = (aiUnitStrength[iIndex] / aiUnitNumbers[iIndex]) % 100;
							if (0 == iCurrent100)
							{
								if (iBase == iCurrent)
								{
									szString.append(CvWString::format(L" %d", iBase));
								}
								else
								{
									szString.append(CvWString::format(L" %d/%d", iCurrent, iBase));
								}
							}
							else
							{
								szString.append(CvWString::format(L" %d.%02d/%d", iCurrent, iCurrent100, iBase));
							}
							szString.append(CvWString::format(L"%c", gDLL->getSymbolID(STRENGTH_CHAR)));
						}


						for (size_t iK = 0; iK < unitInfoPromotionCounts.size(); iK++)
						{
							if (unitInfoPromotionCounts[iK].first == iIndex)
							{
								for (size_t iL = 0; iL < unitInfoPromotionCounts[iK].second.size(); iL++) {
									szString.append(CvWString::format(L"%d<img=%S size=16></img>", unitInfoPromotionCounts[iK].second[iL].second, GC.getPromotionInfo((PromotionTypes)unitInfoPromotionCounts[iK].second[iL].first).getButton()));
								}
								iK = unitInfoPromotionCounts.size();
							}
						}

						if (iJ != GC.getGameINLINE().getActivePlayer() && !GC.getUnitInfo((UnitTypes)iI).isAnimal() && !GC.getUnitInfo((UnitTypes)iI).isHiddenNationality())
						{
							szString.append(L", ");
							szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR, GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorR(), GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorG(), GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorB(), GET_PLAYER((PlayerTypes)iJ).getPlayerTextColorA(), GET_PLAYER((PlayerTypes)iJ).getName()));
						}
					}
				}
			}
		}
	}
}


// Returns true if help was given...
bool CvGameTextMgr::setCombatPlotHelp(CvWStringBuffer& szString, CvPlot* pPlot)
{
	PROFILE_FUNC();

	CvUnit* pAttacker;
	CvUnit* pDefender;
	CvWString szTempBuffer;
	CvWString szOffenseOdds;
	CvWString szDefenseOdds;
	bool bValid;
	int iModifier;

	if (gDLL->getInterfaceIFace()->getLengthSelectionList() == 0)
	{
		return false;
	}

	bValid = false;

	//FfH: Added by Kael 12/23/2008
	int iOdds;
	pAttacker = gDLL->getInterfaceIFace()->getSelectionList()->AI_getBestGroupAttacker(pPlot, false, iOdds);
	//FfH: End Add

	switch (gDLL->getInterfaceIFace()->getSelectionList()->getDomainType())
	{
	case DOMAIN_SEA:
		bValid = pPlot->isWater();
		break;

	case DOMAIN_AIR:
		bValid = true;
		break;

	case DOMAIN_LAND:
		bValid = !(pPlot->isWater());

		//FfH: Added by Kael 12/23/2008
		if (pAttacker != NULL)
		{
			if (pAttacker->canMoveAllTerrain())
			{
				bValid = true;
			}
			if (pAttacker->isBoarding())
			{
				bValid = true;
			}
		}
		//FfH: End Add

		break;

	case DOMAIN_IMMOBILE:
		break;

	default:
		FAssert(false);
		break;
	}

	if (!bValid)
	{
		return false;
	}

	//FfH: Modified by Kael 12/23/2008 (moved up to earlier in this function)
	//	int iOdds;
	//	pAttacker = gDLL->getInterfaceIFace()->getSelectionList()->AI_getBestGroupAttacker(pPlot, false, iOdds);
	//FfH: End Modify

	if (pAttacker == NULL)
	{
		pAttacker = gDLL->getInterfaceIFace()->getSelectionList()->AI_getBestGroupAttacker(pPlot, false, iOdds, true);
	}

	if (pAttacker != NULL)
	{

		//FfH: Modified by Kael 03/29/2009
		//		pDefender = pPlot->getBestDefender(NO_PLAYER, pAttacker->getOwnerINLINE(), pAttacker, false, NO_TEAM == pAttacker->getDeclareWarMove(pPlot));
		if (pAttacker->isAlwaysHostile(pPlot))
		{
			pDefender = pPlot->getBestDefender(NO_PLAYER, pAttacker->getOwnerINLINE(), pAttacker);
		}
		else
		{
			pDefender = pPlot->getBestDefender(NO_PLAYER, pAttacker->getOwnerINLINE(), pAttacker, false, NO_TEAM == pAttacker->getDeclareWarMove(pPlot));
		}
		//FfH: End Modify

		if (pDefender != NULL && pDefender != pAttacker && pDefender->canDefend(pPlot) && pAttacker->canAttack(*pDefender))
		{
			if (pAttacker->getDomainType() != DOMAIN_AIR)
			{
				int iCombatOdds = getCombatOdds(pAttacker, pDefender);

				if (pAttacker->combatLimit() >= GC.getMAX_HIT_POINTS())
				{
					szTempBuffer.Format(L"%.1f", ((float)iCombatOdds) / 10.0f);
					szString.append("Offensive ");
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_ODDS", szTempBuffer.GetCString()));
				}


				int iWithdrawal = 0;

				if (pAttacker->combatLimit() < GC.getMAX_HIT_POINTS())
				{
					iWithdrawal += 100 * iCombatOdds;
				}

				iWithdrawal += std::min(100, pAttacker->withdrawalProbability()) * (1000 - iCombatOdds);

				if (iWithdrawal > 0 || pAttacker->combatLimit() < GC.getMAX_HIT_POINTS())
				{
					if (iWithdrawal > 99900)
					{
						szTempBuffer = L"&gt; 99.9";
					}
					else if (iWithdrawal < 100)
					{
						szTempBuffer = L"&lt; 0.1";
					}
					else
					{
						szTempBuffer.Format(L"%.1f", iWithdrawal / 1000.0f);
					}

					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_ODDS_RETREAT", szTempBuffer.GetCString()));
				}

				//szTempBuffer.Format(L"AI odds: %d%%", iOdds);
				//szString += NEWLINE + szTempBuffer;
			}

			//FfH: Modified by Kael 09/02/2007
			//			szOffenseOdds.Format(L"%.2f", ((pAttacker->getDomainType() == DOMAIN_AIR) ? pAttacker->airCurrCombatStrFloat(pDefender) : pAttacker->currCombatStrFloat(NULL, NULL)));
			szOffenseOdds.Format(L"%.2f", ((pAttacker->getDomainType() == DOMAIN_AIR) ? pAttacker->airCurrCombatStrFloat(pDefender) : pAttacker->currCombatStrFloat(NULL, pDefender)));
			//FfH: End Modify

			szDefenseOdds.Format(L"%.2f", pDefender->currCombatStrFloat(pPlot, pAttacker));
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_ODDS_VS", szOffenseOdds.GetCString(), szDefenseOdds.GetCString()));

			szString.append(L' ');//XXX

			szString.append(gDLL->getText("TXT_KEY_COLOR_POSITIVE"));

			szString.append(L' ');//XXX

			iModifier = pAttacker->getExtraCombatPercent();

			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EXTRA_STRENGTH", iModifier));
			}

			iModifier = pAttacker->unitClassAttackModifier(pDefender->getUnitClassType());

			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitClassInfo(pDefender->getUnitClassType()).getTextKeyWide()));
			}

			for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
			{
				if (pDefender->getDamageTypeCombat((DamageTypes)iI) > 0 && pAttacker->getDamageTypeResist((DamageTypes)iI) > 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_DAMAGE_TYPE_RESIST_ATTACKER", GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
				}

				if (pAttacker->getDamageTypeCombat((DamageTypes)iI) > 0 && pDefender->getDamageTypeResist((DamageTypes)iI) < 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_DAMAGE_TYPE_WEAKNESS_DEFENDER", GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
				}
			}
			if (pDefender->getUnitCombatType() != NO_UNITCOMBAT)
			{
				iModifier = pAttacker->unitCombatModifier(pDefender->getUnitCombatType());

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitCombatInfo(pDefender->getUnitCombatType()).getTextKeyWide()));
				}
			}

			//CombatAuras
			CvCombatAura* pAura = NULL;
			for (int i = 0; i < 4; i++)
			{
				if (i == 0)
					pAura = pAttacker->getBlessing1();
				if (i == 1)
					pAura = pAttacker->getBlessing2();
				if (i == 2)
					pAura = pAttacker->getCurse1();
				if (i == 3)
					pAura = pAttacker->getCurse2();

				if (pAura != NULL)
				{
					//Bonus vs. Promotion
					if (GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType() != NO_PROMOTION && pAura->getAttackPromotionBonus() != 0)
					{
						if (pDefender->isHasPromotion((PromotionTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()))
						{
							iModifier = pAura->getAttackPromotionBonus();

							if (iModifier != 0)
							{
								szString.append(NEWLINE);
								szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getPromotionInfo((PromotionTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()).getTextKeyWide()));
							}
						}
					}

					//Bonus vs. Alignment
					if (GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType() != NO_ALIGNMENT && pAura->getAlignmentAttackBonus() != 0)
					{
						if (GET_PLAYER(pDefender->getOwner()).getAlignment() == GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType())
						{
							iModifier = pAura->getAlignmentAttackBonus();

							if (iModifier != 0)
							{
								szString.append(NEWLINE);
								szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getAlignmentInfo((AlignmentTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType()).getTextKeyWide()));
							}
						}
					}
				}
			}

			if (pDefender->getDamage() == 0)
			{
				iModifier = pAttacker->getBonusVSUnitsFullHealth();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_FULL_HEALTH", iModifier));
				}
			}

			pAttacker->getDefensiveStrikeUnit(iModifier, pDefender);
			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_RANGED_ATTACK_BONUS", iModifier));
			}

			iModifier = GET_PLAYER(pAttacker->getOwnerINLINE()).getCombatBonusFromEquipment(pAttacker);
			if (iModifier != 0)
			{
				if (iModifier < GC.getDefineINT("MAX_EQUIPMENT_BONUS") / 3)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EQUIPMENT_BONUS_LOW", iModifier));
				}

				else if (iModifier < GC.getDefineINT("MAX_EQUIPMENT_BONUS") * 2 / 3)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EQUIPMENT_BONUS_MEDIUM", iModifier));
				}

				else
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EQUIPMENT_BONUS_HIGH", iModifier));
				}
			}

			if (pAttacker->plot()->getWorkingCity() != NULL && pAttacker->plot()->getWorkingCity()->getCultureLevel() > 0)
			{
				for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
				{
					if (GC.getCivicInfo((CivicTypes)GET_PLAYER(pAttacker->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism() != 0)
					{
						if (pAttacker->plot()->getWorkingCity()->getOwnerINLINE() == pAttacker->getOwnerINLINE())
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_NATIONALISM", GC.getCivicInfo((CivicTypes)GET_PLAYER(pAttacker->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism() * pAttacker->plot()->getWorkingCity()->getCultureLevel()));
						}
					}
				}
			}

			if (calculateFlankingBonus(pAttacker, pDefender) != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_PER_TILE_FLANKING", calculateFlankingBonus(pAttacker, pDefender)));
			}

			iModifier = pAttacker->domainModifier(pDefender->getDomainType());

			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getDomainInfo(pDefender->getDomainType()).getTextKeyWide()));
			}

			if (pPlot->isCity(true, pDefender->getTeam()))
			{
				iModifier = pAttacker->cityAttackModifier();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_CITY_MOD", iModifier));
				}
			}

			if (pPlot->isHills())
			{
				iModifier = pAttacker->hillsAttackModifier();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_HILLS_MOD", iModifier));
				}
			}

			if (pPlot->getFeatureType() != NO_FEATURE)
			{
				iModifier = pAttacker->featureAttackModifier(pPlot->getFeatureType());

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_MOD", iModifier, GC.getFeatureInfo(pPlot->getFeatureType()).getTextKeyWide()));
				}
			}
			if (pPlot->getTerrainType() != NO_TERRAIN)
			{
				iModifier = pAttacker->terrainAttackModifier(pPlot->getTerrainType());

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_MOD", iModifier, GC.getTerrainInfo(pPlot->getTerrainType()).getTextKeyWide()));
				}
			}

			//FfH: Added by Kael 0813/2007
			for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if (pAttacker->isHasPromotion((PromotionTypes)iJ) && GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod() > 0)
				{
					if (pDefender->isHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()))
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod(), GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()).getTextKeyWide()));
					}
				}
			}

			iModifier = GC.getGameINLINE().getGlobalCounter() * pAttacker->getCombatPercentGlobalCounter() / 100;
			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_STIGMATA", iModifier));
			}
			//FfH: End Add

			iModifier = pAttacker->getKamikazePercent();
			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_KAMIKAZE_MOD", iModifier));
			}

			if (pDefender->isAnimal())
			{
				iModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAnimalCombatModifier();

				iModifier += pAttacker->getUnitInfo().getAnimalCombatModifier();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_ANIMAL_COMBAT_MOD", iModifier));
				}
			}

			if (pDefender->isBarbarian())
			{
				iModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianCombatModifier();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_UNIT_BARBARIAN_COMBAT_MOD", iModifier));
				}
			}

			if (!(pDefender->immuneToFirstStrikes()))
			{
				if (pAttacker->maxFirstStrikes() > 0)
				{
					if (pAttacker->firstStrikes() == pAttacker->maxFirstStrikes())
					{
						if (pAttacker->firstStrikes() == 1)
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE"));
						}
						else
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", pAttacker->firstStrikes()));
						}
					}
					else
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", pAttacker->firstStrikes(), pAttacker->maxFirstStrikes()));
					}
				}
			}

			if (pAttacker->isHurt())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_HP", pAttacker->currHitPoints(), pAttacker->maxHitPoints()));
			}

			if (pAttacker->isHasSecondChance())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_SECOND_CHANCE"));
			}

			if (pAttacker->getExtraLives() > 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EXTRA_LIVES_LEFT", pAttacker->getExtraLives()));
			}

			if (pAttacker->getBonusNearUF() > 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_NEAR_UF", pAttacker->getBonusNearUF()));
			}

			szString.append(gDLL->getText("TXT_KEY_COLOR_REVERT"));

			szString.append(L' ');//XXX

			szString.append(gDLL->getText("TXT_KEY_COLOR_NEGATIVE"));

			szString.append(L' ');//XXX

			if (!(pAttacker->isRiver()))
			{
				if (pAttacker->plot()->isRiverCrossing(directionXY(pAttacker->plot(), pPlot)))
				{
					iModifier = GC.getRIVER_ATTACK_MODIFIER();

					if (iModifier != 0)
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_RIVER_MOD", -(iModifier)));
					}
				}
			}

			if (!(pAttacker->isAmphib()))
			{
				if (!(pPlot->isWater()) && pAttacker->plot()->isWater())
				{
					iModifier = GC.getAMPHIB_ATTACK_MODIFIER();

					if (iModifier != 0)
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_AMPHIB_MOD", -(iModifier)));
					}
				}
			}

			iModifier = pDefender->getCombatPercentDefense();

			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EXTRA_STRENGTH", iModifier));
			}

			iModifier = pDefender->unitClassDefenseModifier(pAttacker->getUnitClassType());

			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitClassInfo(pAttacker->getUnitClassType()).getTextKeyWide()));
			}

			for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
			{
				if (pAttacker->getDamageTypeCombat((DamageTypes)iI) > 0 && pDefender->getDamageTypeResist((DamageTypes)iI) > 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_DAMAGE_TYPE_RESIST_DEFENDER", GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
				}

				if (pDefender->getDamageTypeCombat((DamageTypes)iI) > 0 && pAttacker->getDamageTypeResist((DamageTypes)iI) < 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_DAMAGE_TYPE_WEAKNESS_ATTACKER", GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
				}
			}

			if (pAttacker->getUnitCombatType() != NO_UNITCOMBAT)
			{
				iModifier = pDefender->unitCombatModifier(pAttacker->getUnitCombatType());

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getUnitCombatInfo(pAttacker->getUnitCombatType()).getTextKeyWide()));
				}
			}

			//CombatAuras
			pAura = NULL;
			for (int i = 0; i < 4; i++)
			{
				if (i == 0)
					pAura = pDefender->getBlessing1();
				if (i == 1)
					pAura = pDefender->getBlessing2();
				if (i == 2)
					pAura = pDefender->getCurse1();
				if (i == 3)
					pAura = pDefender->getCurse2();

				if (pAura != NULL)
				{
					//Bonus vs. Promotion
					if (GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType() != NO_PROMOTION && pAura->getAttackPromotionBonus() != 0)
					{
						if (pAttacker->isHasPromotion((PromotionTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()))
						{
							iModifier = pAura->getAttackPromotionBonus();

							if (iModifier != 0)
							{
								szString.append(NEWLINE);
								szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getPromotionInfo((PromotionTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()).getTextKeyWide()));
							}
						}
					}

					//Bonus vs. Alignment
					if (GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType() != NO_ALIGNMENT && pAura->getAlignmentAttackBonus() != 0)
					{
						if (GET_PLAYER(pAttacker->getOwner()).getAlignment() == GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType())
						{
							iModifier = pAura->getAlignmentAttackBonus();

							if (iModifier != 0)
							{
								szString.append(NEWLINE);
								szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getAlignmentInfo((AlignmentTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType()).getTextKeyWide()));
							}
						}
					}
				}
			}

			if (pAttacker->getDamage() == 0)
			{
				iModifier = pDefender->getBonusVSUnitsFullHealth();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_FULL_HEALTH", iModifier));
				}
			}

			pDefender->getDefensiveStrikeUnit(iModifier, pAttacker);
			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_DEFENSIVE_STRIKE_BONUS", iModifier));
			}

			iModifier = GET_PLAYER(pDefender->getOwnerINLINE()).getCombatBonusFromEquipment(pDefender);
			if (iModifier != 0)
			{
				if (iModifier < GC.getDefineINT("MAX_EQUIPMENT_BONUS") / 3)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EQUIPMENT_BONUS_LOW", iModifier));
				}

				else if (iModifier < GC.getDefineINT("MAX_EQUIPMENT_BONUS") * 2 / 3)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EQUIPMENT_BONUS_MEDIUM", iModifier));
				}

				else
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EQUIPMENT_BONUS_HIGH", iModifier));
				}
			}

			if (pDefender->getWallOfFireBonus(pAttacker) != 0)
			{
				iModifier = pDefender->getWallOfFireBonus(pAttacker);

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_WALL_OF_FIRE", iModifier));
				}
			}

			if (pDefender->plot()->getWorkingCity() != NULL && pDefender->plot()->getWorkingCity()->getCultureLevel() > 0)
			{
				for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
				{
					if (GC.getCivicInfo((CivicTypes)GET_PLAYER(pDefender->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism() != 0)
					{
						if (pDefender->plot()->getWorkingCity()->getOwnerINLINE() == pDefender->getOwnerINLINE())
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_NATIONALISM", GC.getCivicInfo((CivicTypes)GET_PLAYER(pDefender->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism() * pDefender->plot()->getWorkingCity()->getCultureLevel()));
						}
					}
				}
			}

			iModifier = calculateFlankingBonus(pDefender, pAttacker);
			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_PER_TILE_FLANKING", iModifier));
			}

			iModifier = GC.getDefineINT("GARRISION_DEFENSE_BONUS");
			if (pDefender->isGarrision())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_GARRISION_DEFENSE_BONUS", iModifier));
			}

			iModifier = pDefender->domainModifier(pAttacker->getDomainType());

			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", iModifier, GC.getDomainInfo(pAttacker->getDomainType()).getTextKeyWide()));
			}

			//>>>>Unofficial Bug Fix: Modified by Denev 2010/02/21
			//*** Negative defense modifier from plot can apply to any units.
			/*
						if (!(pDefender->noDefensiveBonus()))
						{
							iModifier = pPlot->defenseModifier(pDefender->getTeam(), (pAttacker != NULL) ? pAttacker->ignoreBuildingDefense() : true);
			*/
			iModifier = pPlot->defenseModifier(pDefender->getTeam(), (pAttacker != NULL) ? pAttacker->ignoreBuildingDefense() : true);

			if (pPlot->isDefensiveBonuses(pDefender, iModifier))
			{
				//<<<<Unofficial Bug Fix: End Add
				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_TILE_MOD", iModifier));
				}
			}

			iModifier = pDefender->fortifyModifier();

			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_FORTIFY_MOD", iModifier));
			}

			if (pPlot->isCity(true, pDefender->getTeam()))
			{
				iModifier = pDefender->cityDefenseModifier();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_CITY_MOD", iModifier));
				}
			}

			if (pPlot->isHills())
			{
				iModifier = pDefender->hillsDefenseModifier();

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_HILLS_MOD", iModifier));
				}
			}

			if (pPlot->getFeatureType() != NO_FEATURE)
			{
				iModifier = pDefender->featureDefenseModifier(pPlot->getFeatureType());

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_MOD", iModifier, GC.getFeatureInfo(pPlot->getFeatureType()).getTextKeyWide()));
				}
			}
			if (pPlot->getTerrainType() != NO_TERRAIN)
			{
				iModifier = pDefender->terrainDefenseModifier(pPlot->getTerrainType());

				if (iModifier != 0)
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_UNIT_MOD", iModifier, GC.getTerrainInfo(pPlot->getTerrainType()).getTextKeyWide()));
				}
			}

			//FfH Promotions: Added by Kael 0813/2007
			for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if (pDefender->isHasPromotion((PromotionTypes)iJ) && GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod() > 0)
				{
					if (pAttacker->isHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()))
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_MOD_VS_TYPE", GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod(), GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()).getTextKeyWide()));
					}
				}
			}

			iModifier = GC.getGameINLINE().getGlobalCounter() * pDefender->getCombatPercentGlobalCounter() / 100;
			if (iModifier != 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_STIGMATA", iModifier));
			}
			//FfH: End Add

			if (!(pAttacker->immuneToFirstStrikes()))
			{
				if (pDefender->maxFirstStrikes() > 0)
				{
					if (pDefender->firstStrikes() == pDefender->maxFirstStrikes())
					{
						if (pDefender->firstStrikes() == 1)
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE"));
						}
						else
						{
							szString.append(NEWLINE);
							szString.append(gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", pDefender->firstStrikes()));
						}
					}
					else
					{
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", pDefender->firstStrikes(), pDefender->maxFirstStrikes()));
					}
				}
			}

			if (pDefender->isHurt())
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_HP", pDefender->currHitPoints(), pDefender->maxHitPoints()));
			}

			if (pDefender->getExtraLives() > 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_EXTRA_LIVES_LEFT", pDefender->getExtraLives()));
			}

			if (pDefender->getBonusNearUF() > 0)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_NEAR_UF", pDefender->getBonusNearUF()));
			}

			if ((gDLL->getChtLvl() > 0))
			{
				szTempBuffer.Format(L"\nStack Compare Value = %d",
					gDLL->getInterfaceIFace()->getSelectionList()->AI_compareStacks(pPlot, false));
				szString.append(szTempBuffer);

				int iOurStrengthDefense = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_getOurPlotStrength(pPlot, 1, true, false);
				int iOurStrengthOffense = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_getOurPlotStrength(pPlot, 1, false, false);
				szTempBuffer.Format(L"\nPlot Strength(Ours)= d%d, o%d", iOurStrengthDefense, iOurStrengthOffense);
				szString.append(szTempBuffer);
				int iEnemyStrengthDefense = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_getEnemyPlotStrength(pPlot, 1, true, false);
				int iEnemyStrengthOffense = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_getEnemyPlotStrength(pPlot, 1, false, false);
				szTempBuffer.Format(L"\nPlot Strength(Enemy)= d%d, o%d", iEnemyStrengthDefense, iEnemyStrengthOffense);
				szString.append(szTempBuffer);
			}

			szString.append(gDLL->getText("TXT_KEY_COLOR_REVERT"));

			setDefensiveCombatOddsHelp(szString, pDefender, pAttacker);

			return true;
		}
	}

	return false;
}

bool CvGameTextMgr::setDefensiveCombatOddsHelp(CvWStringBuffer& szString, CvUnit* pAttacker, CvUnit* pDefender) {
	
	//TODO: This could be so much better.  It gives the combat odds if a defending unit were attacked by the unit it would attack on the defending unit's current position.  
	//While that is incredibly useful, it for instance doesn't account for stacks.  So another unit in a stack might be a better attacker against this unit.  For instance this unit might target arcane units first, but
	//in a stack would be targeted by some much more powerful unit.  Additionally, again for stacks it is very hard to understand just what will be doing damage to this unit.  
	// Finally, it doesn't explain just what is going into defense, such as what was the impetus of creating this method, which was the the difficulty malus of -55% to defense (not offense).
	//This is better than what was there, and will cover a lot of cases since stacks with complicated attack orders only tend to become relevant in the mid-late game.
	PROFILE_FUNC();

	CvWString szTempBuffer;
	bool bValid;

	CvPlot* pPlot = pDefender->plot();

	if (gDLL->getInterfaceIFace()->getLengthSelectionList() == 0)
	{
		return false;
	}

	bValid = false;

	switch (pAttacker->getDomainType())
	{
	case DOMAIN_SEA:
		bValid = pPlot->isWater();
		break;

	case DOMAIN_AIR:
		bValid = true;
		break;

	case DOMAIN_LAND:
		bValid = !(pPlot->isWater());

		if (pAttacker != NULL)
		{
			if (pAttacker->canMoveAllTerrain())
			{
				bValid = true;
			}
			if (pAttacker->isBoarding())
			{
				bValid = true;
			}
		}

		break;

	case DOMAIN_IMMOBILE:
		break;

	default:
		FAssert(false);
		break;
	}

	if (!bValid)
	{
		return false;
	}

	if (pAttacker != NULL)
	{
		if (pDefender != NULL && pDefender != pAttacker && pDefender->canDefend(pPlot) && pAttacker->canAttack(*pDefender))
		{
			if (pAttacker->getDomainType() != DOMAIN_AIR)
			{
				int iCombatOdds = 1000 - getCombatOdds(pAttacker, pDefender);

				if (pAttacker->combatLimit() >= GC.getMAX_HIT_POINTS())
				{
					szTempBuffer.Format(L"%.1f", ((float)iCombatOdds) / 10.0f);

					szString.append(NEWLINE);
					szString.append("Defensive ");
					szString.append(gDLL->getText("TXT_KEY_COMBAT_PLOT_ODDS", szTempBuffer.GetCString()));
				}
			}
			return true;
		}
	}
	return false;
}

bool CvGameTextMgr::setSpellFactorsPlotHelp(CvWStringBuffer& szString, CvPlot* pTargetplot)
{
	CvUnit* pSelectedUnit;
	CLLNode<IDInfo>* pSelectedUnitNode;

	int maxDisplayUnits = 6;

	pSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
	pSelectedUnitNode = gDLL->getInterfaceIFace()->headSelectionListNode();	//Select the group. Spin through the selection and show the cumulative effects of the various spell castings
	if(pSelectedUnit == NULL)
		return false;
	PlayerTypes ePlayer = pSelectedUnit->getOwner();
	if(ePlayer == NO_PLAYER)
		return false;
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

	SpellTypes spellType = (SpellTypes)pSelectedUnit->getMissionSpell();
	if (spellType == NO_SPELL) {
		while (pSelectedUnitNode != NULL && spellType == NO_SPELL) {
			if (pSelectedUnit->getMissionSpell() == NO_SPELL) {			//If you have a group with say a non-caster, don't include that unit in the group.
				pSelectedUnitNode = gDLL->getInterfaceIFace()->nextSelectionListNode(pSelectedUnitNode);
				if(pSelectedUnitNode != NULL){ //SpyFanatic: fix possible null pointer exception
					pSelectedUnit = ::getUnit(pSelectedUnitNode->m_data);
				}
				continue;
			}

			spellType = (SpellTypes)pSelectedUnit->getMissionSpell();	//Mission spells are added per unit, so not all units might have the mission spell, but all those that do will have the same one.
		}
	}

	std::list<AttackerSpellInfo> attackingUnitsInformation = calculateAttackerSpellInfo(pSelectedUnitNode, pSelectedUnit, pTargetplot, kPlayer);

	if (attackingUnitsInformation.size() == 0)
		return false;


	if (attackingUnitsInformation.size() == 1) {
		AttackerSpellInfo attacker = attackingUnitsInformation.front();
		std::list<TargetUnitSpellInfo> targets = attacker.targetUnits;

		szString.append(CvWString::format(L"%s casting %s on ", GC.getUnitInfo(attacker.m_pSelectedUnit->getUnitType()).getDescription(), attacker.kSpell->getDescription()));

		if (attacker.targetUnits.size() == 1) {
			szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getUnitInfo(attacker.targetUnits.front().m_pTargetedUnit->getUnitType()).getDescription()));
		}
		else if (attacker.targetUnits.size() < 6) {

			int targetUnitCounter = 0;
			for (std::list<TargetUnitSpellInfo>::iterator it = targets.begin(); it != targets.end(); it++) {
				if (targetUnitCounter == attacker.m_pSelectedUnit->getSpellTargetNumber(*attacker.kSpell) && attacker.m_pSelectedUnit->getSpellTargetIncrease() % 100 > 0) {
					szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_YELLOW"), GC.getUnitInfo(it->m_pTargetedUnit->getUnitType()).getDescription()));

				}
				else {
					szString.append(CvWString::format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getUnitInfo(it->m_pTargetedUnit->getUnitType()).getDescription()));
				}
				if (++it != targets.end())
					szString.append(", ");
				--it;
				targetUnitCounter++;
			}
		}
		else {
			szString.append(CvWString::format(SETCOLR L"%d" ENDCOLR L" units", TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.targetUnits.size()));
		}

		int targetCounter = 0;
		for (std::list<TargetUnitSpellInfo>::iterator target = targets.begin(); target != targets.end(); target++) {
			CvUnit* targetUnit = target->m_pTargetedUnit;
			if (targetCounter < maxDisplayUnits) {
				szString.append(CvWString::format(NEWLINE L"%s" SETCOLR L" (%d%%, %d%%, %d%%)" ENDCOLR, GC.getUnitInfo(targetUnit->getUnitType()).getDescription(), TEXT_COLOR("COLOR_POSITIVE_TEXT"), target->lowDamage, target->expectedDamage, target->highDamage));
			}
			targetCounter++;

			if (target == targets.begin()) {
				if (attacker.kSpell->getDamage() > 0 && !targetUnit->isImmuneToMagic()) {
					szString.append(CvWString::format(NEWLINE L"%cTotal spell power: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), target->totalDamage));

					szString.append(CvWString::format(NEWLINE L"%cTarget " SETCOLR L"%s" ENDCOLR L": " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR(target->resistance > 0 ? "COLOR_NEGATIVE_TEXT" : "COLOR_POSITIVE_TEXT"),
						target->resistance > 0 ? std::wstring(L"Resistance").c_str() : std::wstring(L"Weakness").c_str(), TEXT_COLOR(target->resistance > 0 ? "COLOR_NEGATIVE_TEXT" : "COLOR_POSITIVE_TEXT"), target->resistance));

					if (targetUnit->getUnitInfo().getTier() - 1 > 0)
						szString.append(CvWString::format(NEWLINE L"%c    Tier: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), (targetUnit->getUnitInfo().getTier() - 1) * 10));
					if (attacker.kSpell->getDamageType() != DAMAGE_PHYSICAL && targetUnit->getDamageTypeResist((DamageTypes)attacker.kSpell->getDamageType()) != 0)
						szString.append(CvWString::format(NEWLINE L"%c    Resistance:" SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), targetUnit->getDamageTypeResist((DamageTypes)attacker.kSpell->getDamageType())));
					szString.append(CvWString::format(NEWLINE L"%c    Unit level: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), targetUnit->getLevel()));
					if (targetUnit->getCityResistance() > 0)
						szString.append(CvWString::format(NEWLINE L"%c    City: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), targetUnit->getCityResistance()));

					if (target->groupMemberAdjustment == (target->totalDamage - target->resistance) / 10 && target->groupMemberAdjustment + targetUnit->getDamage() <= attacker.kSpell->getDamageLimit())
						szString.append(CvWString::format(NEWLINE SETCOLR L"%cUnit damage: (%d %s %d)/10 = %d%% damage" ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), gDLL->getSymbolID(BULLET_CHAR), target->totalDamage, target->resistance > 0 ? "-" : "+", target->resistance, (target->totalDamage - target->resistance) / 10));
					else {
						szString.append(CvWString::format(NEWLINE L"%cUnit damage: (%d %s %d)/10 = %d%% damage", gDLL->getSymbolID(BULLET_CHAR), target->totalDamage, target->resistance > 0 ? "-" : "+", target->resistance, (target->totalDamage - target->resistance) / 10));
						if (target->groupMemberAdjustment + targetUnit->getDamage() <= attacker.kSpell->getDamageLimit())
							szString.append(CvWString::format(NEWLINE SETCOLR L"%c+-33%% per group size above/below 3: " L"%d%%" ENDCOLR, TEXT_COLOR(target->groupMemberAdjustment > ((target->iDmg - target->resistance) / 10) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getSymbolID(BULLET_CHAR), target->groupMemberAdjustment));
						else
							szString.append(CvWString::format(NEWLINE L"%c+-33%% per group size above/below 3: " SETCOLR L"%d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR(target->groupMemberAdjustment > ((target->iDmg - target->resistance) / 10) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), target->groupMemberAdjustment));
						szString.append(CvWString::format(NEWLINE L"%c    Base group members: " SETCOLR L"%d" ENDCOLR L"; Current: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR((targetUnit->getGroupSize() > 3) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), targetUnit->getGroupSize(),
							TEXT_COLOR((targetUnit->getGroupSize() - (targetUnit->getDamage() / (100.0 / targetUnit->getGroupSize()))) == targetUnit->getGroupSize() ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"),
							(int)(targetUnit->getGroupSize() + (((int)(targetUnit->getDamage() / (100.0 / targetUnit->getGroupSize()) * 100) % 100) > 0 ? 1 : 0) - (targetUnit->getDamage() / (100.0 / targetUnit->getGroupSize())))));
						if (targetUnit->getDamage() != 0)
							szString.append(CvWString::format(NEWLINE L"%c    Current Damage: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR(targetUnit->getDamage() == 0 ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), targetUnit->getDamage()));
					}

					if (target->groupMemberAdjustment + targetUnit->getDamage() > attacker.kSpell->getDamageLimit()) {
						szString.append(CvWString::format(NEWLINE L"%c" SETCOLR L"Spell exceeds max damage of %d%%: %d - %d = -%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.kSpell->getDamageLimit(), target->groupMemberAdjustment + targetUnit->getDamage(), attacker.kSpell->getDamageLimit(), target->lowDamageCap));
					}
					else {
						szString.append(CvWString::format(NEWLINE L"%c    Spell may deal an extra 1%% damage.", gDLL->getSymbolID(BULLET_CHAR)));
					}

					if (target->lowDamageCap == 0) {
						szString.append(CvWString::format(NEWLINE L"%c" SETCOLR L"Expected damage: %d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), target->expectedDamage));

						if (target->lowDamageCap == 0) {
							szString.append(CvWString::format(NEWLINE L"%c    Crit chance: " SETCOLR L"%d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.m_pSelectedUnit->getSpellCritChance(*attacker.kSpell)));
							szString.append(CvWString::format(NEWLINE L"%c        Tier crit chance " SETCOLR L"%d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.m_pSelectedUnit->getSpellCritChance(*attacker.kSpell)));
							if (attacker.kSpell->isHighCrit())
								szString.append(CvWString::format(NEWLINE L"%c    High crit spell; 2x crit damage (total 6x)", gDLL->getSymbolID(BULLET_CHAR)));
							szString.append(CvWString::format(NEWLINE L"%c    Crit damage multiplier: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), 3));
							if (target->maxDamageTotalCap > 0)
								szString.append(CvWString::format(NEWLINE L"%c    Ignored damage above the cap: " SETCOLR L"-%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), target->maxDamageTotalCap));
						}
					}

					if (target->maxDamageTotalCap != 0)
						szString.append(CvWString::format(NEWLINE L"%c" SETCOLR L"Max damage: %d - %d over cap = %d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), target->highDamage + target->maxDamageTotalCap, target->maxDamageTotalCap, target->highDamage));
					else
						szString.append(CvWString::format(NEWLINE L"%c" SETCOLR L"Max damage: %d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), target->highDamage));

				}
				else if (attacker.kSpell->getDamage() == 0) {
					szString.append(CvWString::format(NEWLINE SETCOLR L"%cSpell does no damage." ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), gDLL->getSymbolID(BULLET_CHAR)));
				}
				else if (targetUnit->isImmuneToMagic()) {
					szString.append(CvWString::format(NEWLINE SETCOLR L"%cTarget is immune to magic." ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), gDLL->getSymbolID(BULLET_CHAR)));
				}

//				-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
				if (attacker.kSpell->isResistable() && !targetUnit->isImmuneToSpell(attacker.m_pSelectedUnit, spellType)) {

					szString.append(CvWString::format(NEWLINE SETCOLR L"%cResist chance: %d%% %s" ENDCOLR, TEXT_COLOR(target->resistChance > 0 ? "COLOR_NEGATIVE_TEXT" : "COLOR_POSITIVE_TEXT"), gDLL->getSymbolID(BULLET_CHAR), target->resistChance, target->resistChance >= 100 ? L"(Immune)" : L""));

					if (GC.getDefineINT("SPELL_RESIST_CHANCE_BASE") != 0)
						szString.append(CvWString::format(NEWLINE L"%c    Base: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GC.getDefineINT("SPELL_RESIST_CHANCE_BASE") > 0 ? GC.getDefineINT("SPELL_RESIST_CHANCE_BASE") : -GC.getDefineINT("SPELL_RESIST_CHANCE_BASE")));

					szString.append(CvWString::format(NEWLINE L"%c    Level * 5: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), targetUnit->getLevel() * 5));

					if (targetUnit->getResist() != 0)
						szString.append(CvWString::format(NEWLINE L"%c    Unit additional resistance: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), targetUnit->getResist() > 0 ? targetUnit->getResist() : -targetUnit->getResist()));

					if (attacker.m_pSelectedUnit->getResistModify() != 0)
						szString.append(CvWString::format(NEWLINE L"%c    Caster target resistance: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.m_pSelectedUnit->getResistModify() > 0 ? -attacker.m_pSelectedUnit->getResistModify() : attacker.m_pSelectedUnit->getResistModify()));

					if (GC.getSpellInfo(spellType).getResistModify() != 0)
						szString.append(CvWString::format(NEWLINE L"%c    Base chance to resist this spell: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GC.getSpellInfo(spellType).getResistModify() > 0 ? GC.getSpellInfo(spellType).getResistModify() : -GC.getSpellInfo(spellType).getResistModify()));

					if (GET_PLAYER(attacker.m_pSelectedUnit->getOwnerINLINE()).getResistEnemyModify() != 0)
						szString.append(CvWString::format(NEWLINE L"%c    Attacking civ: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR(GET_PLAYER(attacker.m_pSelectedUnit->getOwnerINLINE()).getResistModify() > 0 ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), GET_PLAYER(attacker.m_pSelectedUnit->getOwnerINLINE()).getResistEnemyModify() > 0 ? GET_PLAYER(attacker.m_pSelectedUnit->getOwnerINLINE()).getResistEnemyModify() : -GET_PLAYER(attacker.m_pSelectedUnit->getOwnerINLINE()).getResistEnemyModify()));

					if (GET_PLAYER(targetUnit->getOwnerINLINE()).getResistModify() != 0)
						szString.append(CvWString::format(NEWLINE L"%c    Defending civ: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR(GET_PLAYER(targetUnit->getOwnerINLINE()).getResistModify() > 0 ? "COLOR_NEGATIVE_TEXT" : "COLOR_POSITIVE_TEXT"), GET_PLAYER(targetUnit->getOwnerINLINE()).getResistModify() > 0 ? GET_PLAYER(targetUnit->getOwnerINLINE()).getResistModify() : -GET_PLAYER(targetUnit->getOwnerINLINE()).getResistModify()));

					if (pTargetplot->isCity())
						szString.append(CvWString::format(NEWLINE L"%c    City: " SETCOLR L"+10 (base) + %d: %d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), pTargetplot->getPlotCity()->getResistMagic(), 10 + pTargetplot->getPlotCity()->getResistMagic()));

					if (target->resistChance >= GC.getDefineINT("SPELL_RESIST_CHANCE_MAX"))
						szString.append(CvWString::format(NEWLINE L"%c    Max " SETCOLR L"%d" ENDCOLR L"; Over by: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getDefineINT("SPELL_RESIST_CHANCE_MAX"), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), target->nonAdjustedResistChance - target->resistChance));

					if (target->resistChance <= GC.getDefineINT("SPELL_RESIST_CHANCE_MIN"))
						szString.append(CvWString::format(NEWLINE L"%c    Cannot be less than " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GC.getDefineINT("SPELL_RESIST_CHANCE_MIN")));
				}

				//Currently nothing uses miscast.  Hippus light ponies have it in the XML but nothing seems to check for it.
				//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
				if (attacker.kSpell->getDoT() != NO_PROMOTION || attacker.kSpell->getImmobileTurns() > 0 || (attacker.kSpell->getForcedTeleport() > 0 && !pTargetplot->isCity()) || (PromotionTypes)attacker.kSpell->getAddPromotionType1() != NO_PROMOTION ||
					(PromotionTypes)attacker.kSpell->getAddPromotionType2() != NO_PROMOTION || (PromotionTypes)attacker.kSpell->getAddPromotionType3() != NO_PROMOTION || (PromotionTypes)attacker.kSpell->getAddPromotionType4() != NO_PROMOTION || (PromotionTypes)attacker.kSpell->getAddPromotionType5() != NO_PROMOTION || attacker.kSpell->isDomination() ||
					(GC.getUnitInfo(targetUnit->getUnitType()).getUnitCombatType() != NO_UNITCOMBAT && attacker.kSpell->getUnitCombatCapture() == GC.getUnitInfo(targetUnit->getUnitType()).getUnitCombatType()) || target->totalDamageExpectedInflictedBySecondaryDamage > 0 ||
					(attacker.kSpell->isBombard() && pTargetplot->isCity())) {
					szString.append(CvWString::format(NEWLINE L"%cSecondary effects: ", gDLL->getSymbolID(BULLET_CHAR)));

					if (attacker.kSpell->getDoT() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Damage over time", gDLL->getSymbolID(BULLET_CHAR)));

					if (attacker.kSpell->getImmobileTurns() > 0)
						szString.append(CvWString::format(NEWLINE L"%c    Held for " SETCOLR L"%d" ENDCOLR L" turns", gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.kSpell->getImmobileTurns()));

					if (attacker.kSpell->getForcedTeleport() > 0) {
						if (pTargetplot->isCity())
							szString.append(CvWString::format(NEWLINE L"%c    Forced teleport doesn't affect cities", gDLL->getSymbolID(BULLET_CHAR)));
						else
							szString.append(CvWString::format(NEWLINE L"%c    Force teleport in a range of " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), std::min(5, attacker.kSpell->getForcedTeleport())));
					}

					if ((PromotionTypes)attacker.kSpell->getAddPromotionType1() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType1()).getDescription()));
					if ((PromotionTypes)attacker.kSpell->getAddPromotionType2() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType2()).getDescription()));
					if ((PromotionTypes)attacker.kSpell->getAddPromotionType3() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType3()).getDescription()));
					if ((PromotionTypes)attacker.kSpell->getAddPromotionType4() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType4()).getDescription()));
					if ((PromotionTypes)attacker.kSpell->getAddPromotionType5() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType5()).getDescription()));


					if (attacker.kSpell->getRemovePromotionType1() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Removes promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getRemovePromotionType1()).getDescription()));
					if (attacker.kSpell->getRemovePromotionType2() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Removes promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getRemovePromotionType2()).getDescription()));
					if (attacker.kSpell->getRemovePromotionType3() != NO_PROMOTION)
						szString.append(CvWString::format(NEWLINE L"%c    Removes promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getRemovePromotionType2()).getDescription()));

					if (attacker.kSpell->isDomination() || (GC.getUnitInfo(targetUnit->getUnitType()).getUnitCombatType() != NO_UNITCOMBAT && attacker.kSpell->getUnitCombatCapture() == GC.getUnitInfo(targetUnit->getUnitType()).getUnitCombatType()))
						szString.append(CvWString::format(NEWLINE L"%c    Converts unit to your cause", gDLL->getSymbolID(BULLET_CHAR)));

					if (attacker.kSpell->isBombard() && pTargetplot->isCity())
						szString.append(CvWString::format(NEWLINE L"%c    Reduces city defenses by " SETCOLR L"%d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), pTargetplot->getPlotCity()->getDefenseModifier(false) / 7));

					if (attacker.kSpell->isFlameFeatures() && pTargetplot->getFeatureType() != NO_FEATURE && pTargetplot->getImprovementType() == NO_IMPROVEMENT)
					{
						if (pTargetplot->getFeatureType() == GC.getInfoTypeForString("FEATURE_JUNGLE"))
						{
							szString.append(CvWString::format(NEWLINE L"%c    Place smoke - may turn into flames and burn down the jungle.  Can spread to nearby tiles.", gDLL->getSymbolID(BULLET_CHAR)));
						}
						else if (pTargetplot->getFeatureType() == GC.getInfoTypeForString("FEATURE_FOREST")) {
							szString.append(CvWString::format(NEWLINE L"%c    Place smoke - may turn into flames and burn down the forest.  Can spread to nearby tiles.", gDLL->getSymbolID(BULLET_CHAR)));
						}
					}
					if (target->totalDamageExpectedInflictedBySecondaryDamage > 0) {
						szString.append(CvWString::format(NEWLINE L"%c    A total of " SETCOLR L"%d%%" ENDCOLR L" damage is expected to be taken by units on adjacent plots", gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), target->totalDamageExpectedInflictedBySecondaryDamage));
						szString.append(CvWString::format(NEWLINE L"%c        Chance to affect nearby tiles: " SETCOLR L"%d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.kSpell->getTriggerSecondaryPlotChance()));
						szString.append(CvWString::format(NEWLINE L"%c        A total of " SETCOLR L"%d" ENDCOLR L" units can be affected", gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), target->totalUnitsAffectedBySecondaryDamage));
					}
				}
			}
		}

		if (targetCounter > maxDisplayUnits) {
			szString.append(CvWString::format(NEWLINE L"%cAn additional " SETCOLR L"%d" ENDCOLR L" unit(s) will be affected", gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), targetCounter - maxDisplayUnits));
		}

		if (attacker.lowDamageTotal > 0 || attacker.expectedDamageTotal > 0 || attacker.highDamageTotal > 0) {
			szString.append(CvWString::format(NEWLINE SETCOLR L"(Total, Expected, Max) damage: (%d, %d, %d)" ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.lowDamageTotal, attacker.expectedDamageTotal, attacker.highDamageTotal));
		}
	}
	else {
		AttackerSpellInfo attacker = attackingUnitsInformation.front();

		szString.append(CvWString::format(L"Casting %s", attacker.kSpell->getDescription()));
		//TODO: There is a lot of work that could be done to parse out how a selection group will cast a spell.  However, the idea was to demystify exactly how the spell damage was calculated so that a player could use that information
		//to properly use and build arcane classes.  Additionally, with the demystification, we can add potentially add promotions and new disciplines which tie into the various aspects of spell damage.  
		//
		//When a group of units casts a spell there are many more unknowns than if a single unit casts a spell.  For instance the first unit can crit, which could make all future casts of a spell vary in the amount of damage
		//or even change the targets.  Beyond crits there is also an element of damage randomization based on the number of spelltargets and leftover damage amounts which could affect the number of units in a group for example.  
		//
		//This could maybe be done, or at least show who might attack and who might be attacked, but the idea behind the information tooltip is to explain what numbers will be seen, and the numbers deviate too much beyond the first cast
		//to add much more than confusion.  Further, if the player wants to just mass cast a spell on a stack then a certain amount of inefficiency can be accepted.  So with all that in mind just print out some basic information on 
		//what the spell will do - trying to avoid equations and if someone wants to min/max they can cast a spell one unit at a time.
		if (attacker.kSpell->getDamage() == 0) {
			szString.append(CvWString::format(NEWLINE SETCOLR L"%cSpell does no damage." ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), gDLL->getSymbolID(BULLET_CHAR)));
		}
		else {
			szString.append(CvWString::format(NEWLINE L"%c    Crit damage multiplier: " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), 3));
			if (attacker.kSpell->isHighCrit())
				szString.append(CvWString::format(NEWLINE L"%c    High crit spell; 2x crit damage (total 6x)", gDLL->getSymbolID(BULLET_CHAR)));
		}
		//TODO Spyfanatic
/*
		if (attacker.kSpell->getDoT() != NO_PROMOTION || attacker.kSpell->getImmobileTurns() > 0 || (attacker.kSpell->getForcedTeleport() > 0 && !pTargetplot->isCity()) || (PromotionTypes)attacker.kSpell->getAddPromotionType1() != NO_PROMOTION ||
			(PromotionTypes)attacker.kSpell->getAddPromotionType2() != NO_PROMOTION || (PromotionTypes)attacker.kSpell->getAddPromotionType3() != NO_PROMOTION || (PromotionTypes)attacker.kSpell->getAddPromotionType4() != NO_PROMOTION || (PromotionTypes)attacker.kSpell->getAddPromotionType5() != NO_PROMOTION ||
			attacker.kSpell->isDomination() > 0 || attacker.kSpell->getUnitCombatCapture() > 0 || attacker.kSpell->getTriggerSecondaryPlotChance() > 0 || (attacker.kSpell->isBombard() && pTargetplot->isCity()))
		{
			szString.append(CvWString::format(NEWLINE L"%cSecondary effects: ", gDLL->getSymbolID(BULLET_CHAR)));

			if (attacker.kSpell->getDoT() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Damage over time", gDLL->getSymbolID(BULLET_CHAR)));

			if (attacker.kSpell->getImmobileTurns() > 0)
				szString.append(CvWString::format(NEWLINE L"%c    Held for " SETCOLR L"%d" ENDCOLR L" turns", gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.kSpell->getImmobileTurns()));

			if (attacker.kSpell->getForcedTeleport() > 0) {
				if (pTargetplot->isCity())
					szString.append(CvWString::format(NEWLINE L"%c    Forced teleport doesn't affect cities", gDLL->getSymbolID(BULLET_CHAR)));
				else
					szString.append(CvWString::format(NEWLINE L"%c    Force teleport in a range of " SETCOLR L"%d" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), std::min(5, attacker.kSpell->getForcedTeleport())));
			}

			if ((PromotionTypes)attacker.kSpell->getAddPromotionType1() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType1()).getDescription()));
			if ((PromotionTypes)attacker.kSpell->getAddPromotionType2() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType2()).getDescription()));
			if ((PromotionTypes)attacker.kSpell->getAddPromotionType3() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType3()).getDescription()));
			if ((PromotionTypes)attacker.kSpell->getAddPromotionType4() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType4()).getDescription()));
			if ((PromotionTypes)attacker.kSpell->getAddPromotionType5() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Adds promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getAddPromotionType5()).getDescription()));


			if (attacker.kSpell->getRemovePromotionType1() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Removes promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getRemovePromotionType1()).getDescription()));
			if (attacker.kSpell->getRemovePromotionType2() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Removes promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getRemovePromotionType2()).getDescription()));
			if (attacker.kSpell->getRemovePromotionType3() != NO_PROMOTION)
				szString.append(CvWString::format(NEWLINE L"%c    Removes promotion: " SETCOLR L"%s" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), GC.getPromotionInfo((PromotionTypes)attacker.kSpell->getRemovePromotionType2()).getDescription()));

			if (attacker.kSpell->isDomination() > 0 || attacker.kSpell->getUnitCombatCapture() > 0)
				szString.append(CvWString::format(NEWLINE L"%c    Can convert units to your cause", gDLL->getSymbolID(BULLET_CHAR)));

			if (attacker.kSpell->isBombard() && pTargetplot->isCity())
				szString.append(CvWString::format(NEWLINE L"%c    Reduces city defenses", gDLL->getSymbolID(BULLET_CHAR)));

			if (attacker.kSpell->isFlameFeatures() && pTargetplot->getFeatureType() != NO_FEATURE && pTargetplot->getImprovementType() == NO_IMPROVEMENT)
			{
				if (pTargetplot->getFeatureType() == GC.getInfoTypeForString("FEATURE_JUNGLE"))
				{
					szString.append(CvWString::format(NEWLINE L"%c    Place smoke - may turn into flames and burn down the jungle.  Can spread to nearby tiles.", gDLL->getSymbolID(BULLET_CHAR)));
				}
				else if (pTargetplot->getFeatureType() == GC.getInfoTypeForString("FEATURE_FOREST")) {
					szString.append(CvWString::format(NEWLINE L"%c    Place smoke - may turn into flames and burn down the forest.  Can spread to nearby tiles.", gDLL->getSymbolID(BULLET_CHAR)));
				}
			}
			if (attacker.kSpell->getTriggerSecondaryPlotChance() > 0) {
				szString.append(CvWString::format(NEWLINE L"%c    Chance to affect nearby tiles: " SETCOLR L"%d%%" ENDCOLR, gDLL->getSymbolID(BULLET_CHAR), TEXT_COLOR("COLOR_POSITIVE_TEXT"), attacker.kSpell->getTriggerSecondaryPlotChance()));
			}
		}*/

	}

	return true;
}

// DO NOT REMOVE - needed for font testing - Moose
void createTestFontString(CvWStringBuffer& szString)
{
	int iI;
	szString.assign(L"!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[×]^_`abcdefghijklmnopqrstuvwxyz\n");
	szString.append(L"{}~\\ßÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞŸßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ¿¡«»°ŠŒŽšœž™©®€£¢”‘“…’");
	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
		szString.append(CvWString::format(L"%c", GC.getYieldInfo((YieldTypes)iI).getChar()));

	szString.append(L"\n");
	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
		szString.append(CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes)iI).getChar()));
	szString.append(L"\n");
	for (iI = 0; iI < GC.getNumReligionInfos(); ++iI)
	{
		szString.append(CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes)iI).getChar()));
		szString.append(CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes)iI).getHolyCityChar()));
	}
	for (iI = 0; iI < GC.getNumCorporationInfos(); ++iI)
	{
		szString.append(CvWString::format(L"%c", GC.getCorporationInfo((CorporationTypes)iI).getChar()));
		szString.append(CvWString::format(L"%c", GC.getCorporationInfo((CorporationTypes)iI).getHeadquarterChar()));
	}
	szString.append(L"\n");
	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
		szString.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)iI).getChar()));
	for (iI = 0; iI < MAX_NUM_SYMBOLS; ++iI)
		szString.append(CvWString::format(L"%c", gDLL->getSymbolID(iI)));
}

void CvGameTextMgr::setPlotHelp(CvWStringBuffer& szString, CvPlot* pPlot)
{
	PROFILE_FUNC();

	int iI;

	CvWString szTempBuffer;
	ImprovementTypes eImprovement;
	PlayerTypes eRevealOwner;
	BonusTypes eBonus;
	bool bShift;
	bool bAlt;
	bool bCtrl;
	bool bFound;
	int iDefenseModifier;
	int iYield;
	int iTurns;

	bShift = gDLL->shiftKey();
	bAlt = gDLL->altKey();
	bCtrl = gDLL->ctrlKey();

	if (bCtrl && (gDLL->getChtLvl() > 0))
	{
		if (bShift && pPlot->headUnitNode() != NULL)
		{
			return;
		}

		if (pPlot->getOwnerINLINE() != NO_PLAYER)
		{
			int iPlotDanger = GET_PLAYER(pPlot->getOwnerINLINE()).AI_getPlotDanger(pPlot, 2);
			if (iPlotDanger > 0)
			{
				szString.append(CvWString::format(L"\nPlot Danger = %d", iPlotDanger));
			}
		}

		CvCity* pPlotCity = pPlot->getPlotCity();
		if (pPlotCity != NULL)
		{
			PlayerTypes ePlayer = pPlot->getOwnerINLINE();
			CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

			int iCityDefenders = pPlot->plotCount(PUF_canDefendGroupHead, -1, -1, ePlayer, NO_TEAM, PUF_isCityAIType);
			int iAttackGroups = pPlot->plotCount(PUF_isUnitAIType, UNITAI_ATTACK, -1, ePlayer);
			szString.append(CvWString::format(L"\nDefenders [D+A]/N ([%d + %d] / %d)", iCityDefenders, iAttackGroups, pPlotCity->AI_neededDefenders()));

			szString.append(CvWString::format(L"\nFloating Defenders H/N (%d / %d)", kPlayer.AI_getTotalFloatingDefenders(pPlotCity->area()), kPlayer.AI_getTotalFloatingDefendersNeeded(pPlotCity->area())));
			szString.append(CvWString::format(L"\nAir Defenders H/N (%d / %d)", pPlotCity->plot()->plotCount(PUF_canAirDefend, -1, -1, pPlotCity->getOwnerINLINE(), NO_TEAM, PUF_isDomainType, DOMAIN_AIR), pPlotCity->AI_neededAirDefenders()));
			//			int iHostileUnits = kPlayer.AI_countNumAreaHostileUnits(pPlotCity->area());
			//			if (iHostileUnits > 0)
			//			{
			//				szString+=CvWString::format(L"\nHostiles = %d", iHostileUnits);
			//			}

			szString.append(CvWString::format(L"\nThreat C/P (%d / %d)", pPlotCity->AI_cityThreat(), kPlayer.AI_getTotalAreaCityThreat(pPlotCity->area())));

			bool bFirst = true;
			for (int iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				PlayerTypes eLoopPlayer = (PlayerTypes)iI;
				CvPlayerAI& kLoopPlayer = GET_PLAYER(eLoopPlayer);
				if ((eLoopPlayer != ePlayer) && kLoopPlayer.isAlive())
				{
					int iCloseness = pPlotCity->AI_playerCloseness(eLoopPlayer, 7);
					if (iCloseness != 0)
					{
						if (bFirst)
						{
							bFirst = false;

							szString.append(CvWString::format(L"\n\nCloseness:"));
						}

						szString.append(CvWString::format(L"\n%s(7) : %d", kLoopPlayer.getName(), iCloseness));
						szString.append(CvWString::format(L" (%d, ", kPlayer.AI_playerCloseness(eLoopPlayer, 7)));
						if (kPlayer.getTeam() != kLoopPlayer.getTeam())
						{
							szString.append(CvWString::format(L"%d)", GET_TEAM(kPlayer.getTeam()).AI_teamCloseness(kLoopPlayer.getTeam(), 7)));
						}
						else
						{
							szString.append(CvWString::format(L"-)"));
						}
					}
				}
			}

			int iWorkersHave = pPlotCity->AI_getWorkersHave();
			int iWorkersNeeded = pPlotCity->AI_getWorkersNeeded();
			szString.append(CvWString::format(L"\n\nWorkers H/N (%d , %d)", iWorkersHave, iWorkersNeeded));

			int iWorkBoatsNeeded = pPlotCity->AI_neededSeaWorkers();
			szString.append(CvWString::format(L"\n\nWorkboats Needed = %d", iWorkBoatsNeeded));

			int iAreaSiteBestValue = 0;
			int iNumAreaCitySites = kPlayer.AI_getNumAreaCitySites(pPlot->getArea(), iAreaSiteBestValue);
			int iOtherSiteBestValue = 0;
			int iNumOtherCitySites = (pPlot->waterArea() == NULL) ? 0 : kPlayer.AI_getNumAdjacentAreaCitySites(pPlot->waterArea()->getID(), pPlot->getArea(), iOtherSiteBestValue);

			szString.append(CvWString::format(L"\n\nArea Sites = %d (%d)", iNumAreaCitySites, iAreaSiteBestValue));
			szString.append(CvWString::format(L"\nOther Sites = %d (%d)", iNumOtherCitySites, iOtherSiteBestValue));

			szString.append(CvWString::format(L"\nCity Food income: %d", pPlotCity->getYieldRate(YIELD_FOOD)));
			szString.append(CvWString::format(L"\nCity Production income: %d", pPlotCity->getYieldRate(YIELD_PRODUCTION)));
			szString.append(CvWString::format(L"\nCity Commerce income: %d", pPlotCity->getYieldRate(YIELD_COMMERCE)));
			szString.append(CvWString::format(L"\nCity Lumber income: %d", pPlotCity->getYieldRate(YIELD_LUMBER)));
			szString.append(CvWString::format(L"\nCity Leather income: %d", pPlotCity->getYieldRate(YIELD_LEATHER)));
			szString.append(CvWString::format(L"\nCity Metal income: %d", pPlotCity->getYieldRate(YIELD_METAL)));
			szString.append(CvWString::format(L"\nCity Herbs income: %d", pPlotCity->getYieldRate(YIELD_HERB)));
			szString.append(CvWString::format(L"\nCity Stone income: %d", pPlotCity->getYieldRate(YIELD_STONE)));
		}
		else {
			if (pPlot->getOwner() == NO_PLAYER) {
				return;
			}

			CvPlayer& kPlayer = GET_PLAYER(pPlot->getOwner());
			CvCity* pLoopCity;
			int iLoop;
			int iTotalIncome = 0;

			for (int eYield = 0; eYield < GC.getNUM_YIELD_TYPES(); eYield++) {
				iTotalIncome = 0;
				for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
				{
					if (pLoopCity->getYieldRate((YieldTypes)eYield) > 0)
					{
						iTotalIncome += pLoopCity->getYieldRate((YieldTypes)eYield);
					}
				}
				if (eYield != YIELD_COMMERCE && eYield != YIELD_FOOD && eYield != YIELD_PRODUCTION)
					szString.append(CvWString::format(L"\nTotal %s income, treasury: %d, %d", GC.getYieldInfo((YieldTypes) eYield).getDescription(), iTotalIncome, kPlayer.getGlobalYield((YieldTypes)eYield)));
			}
			szString.append(CvWString::format(L"\nTotal gold: %d", kPlayer.getGold()));
			szString.append(CvWString::format(L"\nTotal unit support: %d/%d", kPlayer.getUnitSupportUsed(), kPlayer.getUnitSupportLimitTotal()));
			szString.append(CvWString::format(L"\nTotal mana: %d", kPlayer.getMana()));
			szString.append(CvWString::format(L"\nTotal faith: %d", kPlayer.getFaith()));
			szString.append(CvWString::format(L"\nTotal global culture: %d", kPlayer.getGlobalCulture()));
		}
		/** Sephi
		/** Disable cause it is very annoying
				else if (pPlot->getOwner() != NO_PLAYER)
				{
					for (int iI = 0; iI < GC.getNumCivicInfos(); iI++)
					{
						szString.append(CvWString::format(L"\n %s = %d", GC.getCivicInfo((CivicTypes)iI).getDescription(), GET_PLAYER(pPlot->getOwner()).AI_civicValue((CivicTypes)iI)));
					}
				}
		**/
		return;

	}
	else if (bShift && !bAlt && (gDLL->getChtLvl() > 0))
	{
		szString.append(GC.getTerrainInfo(pPlot->getTerrainType()).getDescription());

		FAssert((0 < GC.getNumBonusInfos()) && "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvInterface::updateHelpStrings");
		for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
		{
			if (pPlot->isPlotGroupConnectedBonus(GC.getGameINLINE().getActivePlayer(), ((BonusTypes)iI)))
			{
				szString.append(NEWLINE);
				szString.append(GC.getBonusInfo((BonusTypes)iI).getDescription());
				szString.append(CvWString::format(L" (%d)", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_bonusVal((BonusTypes)iI)));
			}
		}

		if (pPlot->getPlotGroup(GC.getGameINLINE().getActivePlayer()) != NULL)
		{
			szTempBuffer.Format(L"\n(%d, %d) group: %d", pPlot->getX_INLINE(), pPlot->getY_INLINE(), pPlot->getPlotGroup(GC.getGameINLINE().getActivePlayer())->getID());
		}
		else
		{
			szTempBuffer.Format(L"\n(%d, %d) group: (-1, -1)", pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		szString.append(szTempBuffer);

		szTempBuffer.Format(L"\nArea: %d", pPlot->getArea());
		szString.append(szTempBuffer);

		char tempChar = 'x';
		if (pPlot->getRiverNSDirection() == CARDINALDIRECTION_NORTH)
		{
			tempChar = 'N';
		}
		else if (pPlot->getRiverNSDirection() == CARDINALDIRECTION_SOUTH)
		{
			tempChar = 'S';
		}
		szTempBuffer.Format(L"\nNSRiverFlow: %c", tempChar);
		szString.append(szTempBuffer);

		tempChar = 'x';
		if (pPlot->getRiverWEDirection() == CARDINALDIRECTION_WEST)
		{
			tempChar = 'W';
		}
		else if (pPlot->getRiverWEDirection() == CARDINALDIRECTION_EAST)
		{
			tempChar = 'E';
		}
		szTempBuffer.Format(L"\nWERiverFlow: %c", tempChar);
		szString.append(szTempBuffer);

		if (pPlot->getRouteType() != NO_ROUTE)
		{
			szTempBuffer.Format(L"\nRoute: %s", GC.getRouteInfo(pPlot->getRouteType()).getDescription());
			szString.append(szTempBuffer);
		}

		if (pPlot->getRouteSymbol() != NULL)
		{
			szTempBuffer.Format(L"\nConnection: %i", gDLL->getRouteIFace()->getConnectionMask(pPlot->getRouteSymbol()));
			szString.append(szTempBuffer);
		}

		for (iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (pPlot->getCulture((PlayerTypes)iI) > 0)
				{
					szTempBuffer.Format(L"\n%s Culture: %d", GET_PLAYER((PlayerTypes)iI).getName(), pPlot->getCulture((PlayerTypes)iI));
					szString.append(szTempBuffer);
				}
			}
		}

		PlayerTypes eActivePlayer = GC.getGameINLINE().getActivePlayer();
		int iActualFoundValue = pPlot->getFoundValue(eActivePlayer);
		int iCalcFoundValue = GET_PLAYER(eActivePlayer).AI_foundValue(pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1, false);
		int iStartingFoundValue = GET_PLAYER(eActivePlayer).AI_foundValue(pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1, true);

		szTempBuffer.Format(L"\nFound Value: %d, (%d, %d)", iActualFoundValue, iCalcFoundValue, iStartingFoundValue);
		szString.append(szTempBuffer);

		/** Sephi newFoundValue
		/** added for Debug **/
		szTempBuffer.Format(L"\nPlot Value: %d", GET_PLAYER(eActivePlayer).AI_plotValue(pPlot));
		szString.append(szTempBuffer);
		szTempBuffer.Format(L"\nWilderness Value: %d", pPlot->getWilderness());
		szString.append(szTempBuffer);
		szTempBuffer.Format(L"\nPlanes Value: %d", pPlot->getPlane());
		szString.append(szTempBuffer);
		if (pPlot->isPortal()) {
			szTempBuffer.Format(L"\nPortal Exit: %d, %d", pPlot->getPortalExit()->getX_INLINE(), pPlot->getPortalExit()->getY_INLINE());
			szString.append(szTempBuffer);
		}
		szTempBuffer.Format(L"\nStartingPlotValue: %d", pPlot->StartingPlotValue((CivilizationTypes)0, 5));
		szString.append(szTempBuffer);
		//Pathfinding
		//		szTempBuffer.Format(L"\nSector: %d, Region: %d", pPlot->getSector(), pPlot->getRegion());
		//		szString.append(szTempBuffer);
		/** end **/

		CvCity* pWorkingCity = pPlot->getWorkingCity();
		if (NULL != pWorkingCity)
		{
			int iPlotIndex = pWorkingCity->getCityPlotIndex(pPlot);
			int iBuildValue = pWorkingCity->AI_getBestBuildValue(iPlotIndex);
			BuildTypes eBestBuild = pWorkingCity->AI_getBestBuild(iPlotIndex);

			if (NO_BUILD != eBestBuild)
			{
				szTempBuffer.Format(L"\nBest Build: %s (%d)", GC.getBuildInfo(eBestBuild).getDescription(), iBuildValue);
				szString.append(szTempBuffer);
			}
		}

		{
			szTempBuffer.Format(L"\nStack Str: land=%d(%d), sea=%d(%d), air=%d(%d)",
				pPlot->AI_sumStrength(NO_PLAYER, NO_PLAYER, DOMAIN_LAND, false, false, false),
				pPlot->AI_sumStrength(NO_PLAYER, NO_PLAYER, DOMAIN_LAND, true, false, false),
				pPlot->AI_sumStrength(NO_PLAYER, NO_PLAYER, DOMAIN_SEA, false, false, false),
				pPlot->AI_sumStrength(NO_PLAYER, NO_PLAYER, DOMAIN_SEA, true, false, false),
				pPlot->AI_sumStrength(NO_PLAYER, NO_PLAYER, DOMAIN_AIR, false, false, false),
				pPlot->AI_sumStrength(NO_PLAYER, NO_PLAYER, DOMAIN_AIR, true, false, false));
			szString.append(szTempBuffer);
		}

		if (pPlot->getPlotCity() != NULL)
		{
			PlayerTypes ePlayer = pPlot->getOwnerINLINE();
			CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

			szString.append(CvWString::format(L"\n\nI spy with my little eye..."));
			for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
			{
				if (kPlayer.AI_getUnitClassWeight((UnitClassTypes)iI) != 0)
				{
					szString.append(CvWString::format(L"\n%s = %d", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription(), kPlayer.AI_getUnitClassWeight((UnitClassTypes)iI)));
				}
			}
			szString.append(CvWString::format(L"\n\nalso unit combat types..."));
			for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
			{
				if (kPlayer.AI_getUnitCombatWeight((UnitCombatTypes)iI) != 0)
				{
					szString.append(CvWString::format(L"\n%s = % d", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription(), kPlayer.AI_getUnitCombatWeight((UnitCombatTypes)iI)));
				}
			}
		}
	}
	else if (!bShift && bAlt && (gDLL->getChtLvl() > 0))
	{
		if (pPlot->isOwned())
		{
			szTempBuffer.Format(L"\nThis player has %d area cities", pPlot->area()->getCitiesPerPlayer(pPlot->getOwnerINLINE()));
			szString.append(szTempBuffer);
			for (int iI = 0; iI < GC.getNumReligionInfos(); ++iI)
			{
				int iNeededMissionaries = GET_PLAYER(pPlot->getOwnerINLINE()).AI_neededMissionaries(pPlot->area(), ((ReligionTypes)iI));
				if (iNeededMissionaries > 0)
				{
					szTempBuffer.Format(L"\nNeeded %c missionaries = %d", GC.getReligionInfo((ReligionTypes)iI).getChar(), iNeededMissionaries);
					szString.append(szTempBuffer);
				}
			}

			int iOurDefense = GET_PLAYER(pPlot->getOwnerINLINE()).AI_getOurPlotStrength(pPlot, 0, true, false);
			int iEnemyOffense = GET_PLAYER(pPlot->getOwnerINLINE()).AI_getEnemyPlotStrength(pPlot, 2, false, false);
			if (iEnemyOffense > 0)
			{
				szString.append(CvWString::format(SETCOLR L"\nDanger: %.2f (%d/%d)" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"),
					(iEnemyOffense * 1.0f) / std::max(1, iOurDefense), iEnemyOffense, iOurDefense));
			}

			CvCity* pCity = pPlot->getPlotCity();
			if (pCity != NULL)
			{
				szTempBuffer.Format(L"\n\nCulture Pressure Value = %d", pCity->AI_calculateCulturePressure());
				szString.append(szTempBuffer);

				szTempBuffer.Format(L"\nWater World Percent = %d", pCity->AI_calculateWaterWorldPercent());
				szString.append(szTempBuffer);

				CvPlayerAI& kPlayer = GET_PLAYER(pCity->getOwnerINLINE());
				int iUnitCost = kPlayer.calculateUnitCost();
				int iTotalCosts = kPlayer.calculatePreInflatedCosts();
				int iUnitCostPercentage = (iUnitCost * 100) / std::max(1, iTotalCosts);
				szString.append(CvWString::format(L"\nUnit cost percentage: %d (%d / %d)", iUnitCostPercentage, iUnitCost, iTotalCosts));

				szString.append(CvWString::format(L"\nUpgrade all units: %d gold", kPlayer.AI_goldToUpgradeAllUnits()));

				szString.append(CvWString::format(L"\n\nRanks:"));
				szString.append(CvWString::format(L"\nPopulation:%d", pCity->findPopulationRank()));

				szString.append(CvWString::format(L"\nFood:%d(%d), ", pCity->findYieldRateRank(YIELD_FOOD), pCity->findBaseYieldRateRank(YIELD_FOOD)));
				szString.append(CvWString::format(L"Prod:%d(%d), ", pCity->findYieldRateRank(YIELD_PRODUCTION), pCity->findBaseYieldRateRank(YIELD_PRODUCTION)));
				szString.append(CvWString::format(L"Commerce:%d(%d)", pCity->findYieldRateRank(YIELD_COMMERCE), pCity->findBaseYieldRateRank(YIELD_COMMERCE)));

				szString.append(CvWString::format(L"\nGold:%d, ", pCity->findCommerceRateRank(COMMERCE_GOLD)));
				szString.append(CvWString::format(L"Research:%d, ", pCity->findCommerceRateRank(COMMERCE_RESEARCH)));
				szString.append(CvWString::format(L"Culture:%d", pCity->findCommerceRateRank(COMMERCE_CULTURE)));
			}
			szString.append(NEWLINE);

			//AI strategies
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_DAGGER))
			{
				szTempBuffer.Format(L"Dagger, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_CULTURE1))
			{
				szTempBuffer.Format(L"Culture1, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_CULTURE2))
			{
				szTempBuffer.Format(L"Culture2, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_CULTURE3))
			{
				szTempBuffer.Format(L"Culture3, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_CULTURE4))
			{
				szTempBuffer.Format(L"Culture4, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_MISSIONARY))
			{
				szTempBuffer.Format(L"Missionary, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_CRUSH))
			{
				szTempBuffer.Format(L"Crush, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_PRODUCTION))
			{
				szTempBuffer.Format(L"Production, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_PEACE))
			{
				szTempBuffer.Format(L"Peace, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_GET_BETTER_UNITS))
			{
				szTempBuffer.Format(L"GetBetterUnits, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_FASTMOVERS))
			{
				szTempBuffer.Format(L"FastMovers, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_LAND_BLITZ))
			{
				szTempBuffer.Format(L"LandBlitz, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_AIR_BLITZ))
			{
				szTempBuffer.Format(L"AirBlitz, ");
				szString.append(szTempBuffer);
			}
			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_OWABWNW))
			{
				szTempBuffer.Format(L"OWABWNW, ");
				szString.append(szTempBuffer);
			}

			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_FINAL_WAR))
			{
				szTempBuffer.Format(L"FinalWar, ");
				szString.append(szTempBuffer);
			}

			if (GET_PLAYER(pPlot->getOwner()).AI_isDoStrategy(AI_STRATEGY_BIG_ESPIONAGE))
			{
				szTempBuffer.Format(L"BigEspionage, ");
				szString.append(szTempBuffer);
			}

			//Area battle plans.
			if (pPlot->area()->getAreaAIType(pPlot->getTeam()) == AREAAI_OFFENSIVE)
			{
				szTempBuffer.Format(L"\n Area AI = OFFENSIVE");
			}
			else if (pPlot->area()->getAreaAIType(pPlot->getTeam()) == AREAAI_DEFENSIVE)
			{
				szTempBuffer.Format(L"\n Area AI = DEFENSIVE");
			}
			else if (pPlot->area()->getAreaAIType(pPlot->getTeam()) == AREAAI_MASSING)
			{
				szTempBuffer.Format(L"\n Area AI = MASSING");
			}
			else if (pPlot->area()->getAreaAIType(pPlot->getTeam()) == AREAAI_ASSAULT)
			{
				szTempBuffer.Format(L"\n Area AI = ASSAULT");
			}
			else if (pPlot->area()->getAreaAIType(pPlot->getTeam()) == AREAAI_ASSAULT_MASSING)
			{
				szTempBuffer.Format(L"\n Area AI = ASSAULT_MASSING");
			}
			else if (pPlot->area()->getAreaAIType(pPlot->getTeam()) == AREAAI_NEUTRAL)
			{
				szTempBuffer.Format(L"\n Area AI = NEUTRAL");
			}

			szString.append(szTempBuffer);
		}

		if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
		{
			szTempBuffer.Format(L"\nChokeValue %d CanalValue %d", pPlot->getChokeValue() , pPlot->getCanalValue());
			szString.append(szTempBuffer);
		}
		bool bFirst = true;
		for (iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			PlayerTypes ePlayer = (PlayerTypes)iI;
			CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

			if (kPlayer.isAlive())
			{
				int iActualFoundValue = pPlot->getFoundValue(ePlayer);
				int iCalcFoundValue = kPlayer.AI_foundValue(pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1, false);
				int iStartingFoundValue = kPlayer.AI_foundValue(pPlot->getX_INLINE(), pPlot->getY_INLINE(), -1, true);
				int iBestAreaFoundValue = pPlot->area()->getBestFoundValue(ePlayer);
				int iCitySiteBestValue;
				int iNumAreaCitySites = kPlayer.AI_getNumAreaCitySites(pPlot->getArea(), iCitySiteBestValue);

				if ((iActualFoundValue > 0 || iCalcFoundValue > 0 || iStartingFoundValue > 0)
					|| ((pPlot->getOwner() == iI) && (iBestAreaFoundValue > 0)))
				{
					if (bFirst)
					{
						szString.append(CvWString::format(SETCOLR L"\nFound Values:" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT")));
						bFirst = false;
					}

					szString.append(NEWLINE);

					bool bIsRevealed = pPlot->isRevealed(kPlayer.getTeam(), false);

					szString.append(CvWString::format(SETCOLR, TEXT_COLOR(bIsRevealed ? (((iActualFoundValue > 0) && (iActualFoundValue == iBestAreaFoundValue)) ? "COLOR_UNIT_TEXT" : "COLOR_ALT_HIGHLIGHT_TEXT") : "COLOR_HIGHLIGHT_TEXT")));

					if (!bIsRevealed)
					{
						szString.append(CvWString::format(L"("));
					}

					szString.append(CvWString::format(L"%s: %d", kPlayer.getName(), iActualFoundValue));

					if (!bIsRevealed)
					{
						szString.append(CvWString::format(L")"));
					}

					szString.append(CvWString::format(ENDCOLR));

					if (iCalcFoundValue > 0 || iStartingFoundValue > 0)
					{
						szTempBuffer.Format(L" (%d,%ds)", iCalcFoundValue, iStartingFoundValue);
						szString.append(szTempBuffer);
					}

					int iDeadlockCount = kPlayer.AI_countDeadlockedBonuses(pPlot);
					if (iDeadlockCount > 0)
					{
						szTempBuffer.Format(L", " SETCOLR L"d=%d" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), iDeadlockCount);
						szString.append(szTempBuffer);
					}

					if (kPlayer.AI_isPlotCitySite(pPlot))
					{
						szTempBuffer.Format(L", " SETCOLR L"X" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"));
						szString.append(szTempBuffer);
					}

					if ((iBestAreaFoundValue > 0) || (iNumAreaCitySites > 0))
					{
						int iBestFoundValue = kPlayer.findBestFoundValue();

						szTempBuffer.Format(L"\n  Area Best = %d, Best = %d, Sites = %d", iBestAreaFoundValue, iBestFoundValue, iNumAreaCitySites);
						szString.append(szTempBuffer);
					}
				}
			}
		}
	}
	else if (bShift && bAlt && (gDLL->getChtLvl() > 0))
	{
		CvCity* pCity = pPlot->getWorkingCity();
		if (pCity != NULL)
		{
			// some functions we want to call are not in CvCity, worse some are protected, so made us a friend
			CvCityAI* pCityAI = static_cast<CvCityAI*>(pCity);

			bool bAvoidGrowth = pCity->AI_avoidGrowth();
			bool bIgnoreGrowth = pCityAI->AI_ignoreGrowth();

			// if we over the city, then do an array of all the plots
			if (pPlot->getPlotCity() != NULL)
			{

				// check avoid growth
				if (bAvoidGrowth || bIgnoreGrowth)
				{
					// red color
					szString.append(CvWString::format(SETCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));

					if (bAvoidGrowth)
					{
						szString.append(CvWString::format(L"AvoidGrowth"));

						if (bIgnoreGrowth)
							szString.append(CvWString::format(L", "));
					}

					if (bIgnoreGrowth)
						szString.append(CvWString::format(L"IgnoreGrowth"));

					// end color
					szString.append(CvWString::format(ENDCOLR L"\n"));
				}

				// if control key is down, ignore food
				bool bIgnoreFood = gDLL->ctrlKey();

				// line one is: blank, 20, 9, 10, blank
				setCityPlotYieldValueString(szString, pCity, -1, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 20, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 9, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 10, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				szString.append(L"\n");

				// line two is: 19, 8, 1, 2, 11
				setCityPlotYieldValueString(szString, pCity, 19, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 8, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 1, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 2, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 11, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				szString.append(L"\n");

				// line three is: 18, 7, 0, 3, 12
				setCityPlotYieldValueString(szString, pCity, 18, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 7, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 0, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 3, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 12, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				szString.append(L"\n");

				// line four is: 17, 6, 5, 4, 13
				setCityPlotYieldValueString(szString, pCity, 17, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 6, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 5, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 4, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 13, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				szString.append(L"\n");

				// line five is: blank, 16, 15, 14, blank
				setCityPlotYieldValueString(szString, pCity, -1, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 16, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 15, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);
				setCityPlotYieldValueString(szString, pCity, 14, bAvoidGrowth, bIgnoreGrowth, bIgnoreFood);

				// show specialist values too
				for (int iI = 0; iI < GC.getNumSpecialistInfos(); ++iI)
				{
					int iMaxThisSpecialist = pCity->getMaxSpecialistCount((SpecialistTypes)iI);
					int iSpecialistCount = pCity->getSpecialistCount((SpecialistTypes)iI);
					bool bUsingSpecialist = (iSpecialistCount > 0);
					bool bIsDefaultSpecialist = (iI == GC.getDEFAULT_SPECIALIST());

					// can this city have any of this specialist?
					if (iMaxThisSpecialist > 0 || bIsDefaultSpecialist)
					{
						// start color
						if (pCity->getForceSpecialistCount((SpecialistTypes)iI) > 0)
							szString.append(CvWString::format(L"\n" SETCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));
						else if (bUsingSpecialist)
							szString.append(CvWString::format(L"\n" SETCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT")));
						else
							szString.append(CvWString::format(L"\n" SETCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT")));

						// add name
						szString.append(GC.getSpecialistInfo((SpecialistTypes)iI).getDescription());

						// end color
						szString.append(CvWString::format(ENDCOLR));

						// add usage
						szString.append(CvWString::format(L": (%d/%d) ", iSpecialistCount, iMaxThisSpecialist));

						// add value
						int iValue = pCityAI->AI_specialistValue(((SpecialistTypes)iI), bAvoidGrowth, /*bRemove*/ bUsingSpecialist);
						setYieldValueString(szString, iValue, /*bActive*/ bUsingSpecialist);
					}
				}
				{
					int iFood = GET_PLAYER(pCity->getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_FOOD);
					int iHammer = GET_PLAYER(pCity->getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_PRODUCTION);
					int iCommerce = GET_PLAYER(pCity->getOwnerINLINE()).AI_averageYieldMultiplier(YIELD_COMMERCE);

					szString.append(CvWString::format(L"\nPlayer:(f%d, h%d, c%d)", iFood, iHammer, iCommerce));

					iFood = pCity->AI_yieldMultiplier(YIELD_FOOD);
					iHammer = pCity->AI_yieldMultiplier(YIELD_PRODUCTION);
					iCommerce = pCity->AI_yieldMultiplier(YIELD_COMMERCE);

					szString.append(CvWString::format(L"\nCityBa:(f%d, h%d, c%d)", iFood, iHammer, iCommerce));

					iFood = pCityAI->AI_specialYieldMultiplier(YIELD_FOOD);
					iHammer = pCityAI->AI_specialYieldMultiplier(YIELD_PRODUCTION);
					iCommerce = pCityAI->AI_specialYieldMultiplier(YIELD_COMMERCE);

					szString.append(CvWString::format(L"\nCitySp:(f%d, h%d, c%d)", iFood, iHammer, iCommerce));

					szString.append(CvWString::format(L"\nExchange"));
					for (int iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
					{
						iCommerce = GET_PLAYER(pCity->getOwnerINLINE()).AI_averageCommerceExchange((CommerceTypes)iI);
						szTempBuffer.Format(L", %d%c", iCommerce, GC.getCommerceInfo((CommerceTypes)iI).getChar());
						szString.append(szTempBuffer);
					}

					if (GET_PLAYER(pCity->getOwnerINLINE()).AI_isFinancialTrouble())
					{
						szTempBuffer.Format(L"$$$!!!");
						szString.append(szTempBuffer);
					}
				}
			}
			else
			{
				bool bWorkingPlot = pCity->isWorkingPlot(pPlot);

				if (bWorkingPlot)
					szTempBuffer.Format(SETCOLR L"%s is working" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), pCity->getName().GetCString());
				else
					szTempBuffer.Format(SETCOLR L"%s not working" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), pCity->getName().GetCString());
				szString.append(szTempBuffer);

				int iValue = pCityAI->AI_plotValue(pPlot, bAvoidGrowth, /*bRemove*/ bWorkingPlot, /*bIgnoreFood*/ false, bIgnoreGrowth);
				int iJuggleValue = pCityAI->AI_plotValue(pPlot, bAvoidGrowth, /*bRemove*/ bWorkingPlot, false, bIgnoreGrowth, true);
				int iMagicValue = pCityAI->AI_getPlotMagicValue(pPlot, pCityAI->healthRate() == 0);

				szTempBuffer.Format(L"\nvalue = %d\njuggle value = %d\nmagic value = %d", iValue, iJuggleValue, iMagicValue);
				szString.append(szTempBuffer);
			}
		}

		// calc some bonus info
		if (GC.getGameINLINE().isDebugMode())
		{
			eBonus = pPlot->getBonusType();
		}
		else
		{
			eBonus = pPlot->getBonusType(GC.getGameINLINE().getActiveTeam());
		}
		if (eBonus != NO_BONUS)
		{
			szString.append(CvWString::format(L"\n%s values:", GC.getBonusInfo(eBonus).getDescription()));

			for (int iPlayerIndex = 0; iPlayerIndex < MAX_PLAYERS; iPlayerIndex++)
			{
				CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayerIndex);
				if (kLoopPlayer.isAlive())
				{
					szString.append(CvWString::format(L"\n %s: %d", kLoopPlayer.getName(), kLoopPlayer.AI_bonusVal(eBonus)));
				}
			}
		}
	}
	else
	{
		eRevealOwner = pPlot->getRevealedOwner(GC.getGameINLINE().getActiveTeam(), true);

		if (eRevealOwner != NO_PLAYER)
		{
			if (pPlot->isActiveVisible(true))
			{
				szTempBuffer.Format(L"%d%% " SETCOLR L"%s" ENDCOLR, pPlot->calculateCulturePercent(eRevealOwner), GET_PLAYER(eRevealOwner).getPlayerTextColorR(), GET_PLAYER(eRevealOwner).getPlayerTextColorG(), GET_PLAYER(eRevealOwner).getPlayerTextColorB(), GET_PLAYER(eRevealOwner).getPlayerTextColorA(), GET_PLAYER(eRevealOwner).getCivilizationAdjective());
				szString.append(szTempBuffer);
				szString.append(NEWLINE);

				for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
				{
					if (iPlayer != eRevealOwner)
					{
						CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iPlayer);
						if (kPlayer.isAlive() && pPlot->getCulture((PlayerTypes)iPlayer) > 0)
						{
							szTempBuffer.Format(L"%d%% " SETCOLR L"%s" ENDCOLR, pPlot->calculateCulturePercent((PlayerTypes)iPlayer), kPlayer.getPlayerTextColorR(), kPlayer.getPlayerTextColorG(), kPlayer.getPlayerTextColorB(), kPlayer.getPlayerTextColorA(), kPlayer.getCivilizationAdjective());
							szString.append(szTempBuffer);
							szString.append(NEWLINE);
						}
					}
				}

			}
			else
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, GET_PLAYER(eRevealOwner).getPlayerTextColorR(), GET_PLAYER(eRevealOwner).getPlayerTextColorG(), GET_PLAYER(eRevealOwner).getPlayerTextColorB(), GET_PLAYER(eRevealOwner).getPlayerTextColorA(), GET_PLAYER(eRevealOwner).getCivilizationDescription());
				szString.append(szTempBuffer);
				szString.append(NEWLINE);
			}
		}

		iDefenseModifier = pPlot->defenseModifier((eRevealOwner != NO_PLAYER ? GET_PLAYER(eRevealOwner).getTeam() : NO_TEAM), true, true);
		/** BUGFIX SEPHI
		/** Include Defense Modifier from Improvement
		**/
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			iDefenseModifier += GC.getImprovementInfo(pPlot->getImprovementType()).getDefenseModifier();
		}
		/** BUGFIX END **/
		if (iDefenseModifier != 0)
		{
			szString.append(gDLL->getText("TXT_KEY_PLOT_BONUS", iDefenseModifier));
			szString.append(NEWLINE);
		}

		if (pPlot->getTerrainType() != NO_TERRAIN)
		{
			if (pPlot->isPeak())
			{
				szString.append(gDLL->getText("TXT_KEY_PLOT_PEAK"));
			}
			else
			{
				if (pPlot->isWater())
				{
					szTempBuffer.Format(SETCOLR, TEXT_COLOR("COLOR_WATER_TEXT"));
					szString.append(szTempBuffer);
				}

				if (pPlot->isHills())
				{
					szString.append(gDLL->getText("TXT_KEY_PLOT_HILLS"));
				}

				if (pPlot->getFeatureType() != NO_FEATURE)
				{
					szTempBuffer.Format(L"%s/", GC.getFeatureInfo(pPlot->getFeatureType()).getDescription());
					szString.append(szTempBuffer);
				}

				szString.append(GC.getTerrainInfo(pPlot->getTerrainType()).getDescription());

				if (pPlot->isWater())
				{
					szString.append(ENDCOLR);
				}
			}
		}

		if (pPlot->hasYield())
		{
			for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
			{
				iYield = pPlot->calculateYield(((YieldTypes)iI), true);

				if (iYield != 0)
				{
					szTempBuffer.Format(L", %d%c", iYield, GC.getYieldInfo((YieldTypes)iI).getChar());
					szString.append(szTempBuffer);
				}
			}
		}

		if (pPlot->isFreshWater())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PLOT_FRESH_WATER"));
		}

		if (pPlot->isLake())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PLOT_FRESH_WATER_LAKE"));
		}

		if (pPlot->isImpassable())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PLOT_IMPASSABLE"));
		}

		if (GC.getGameINLINE().isDebugMode())
		{
			eBonus = pPlot->getBonusType();
		}
		else
		{
			eBonus = pPlot->getBonusType(GC.getGameINLINE().getActiveTeam());
		}

		if (eBonus != NO_BONUS)
		{
			//>>>>	BUGFfH: Modified by Denev 2009/10/02
			//			szTempBuffer.Format(L"%c " SETCOLR L"%s" ENDCOLR, GC.getBonusInfo(eBonus).getChar(), TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo(eBonus).getDescription());
			szTempBuffer.assign(getLinkedText(eBonus));
			//<<<<	BUGFfH: End Modify

			szString.append(NEWLINE);
			szString.append(szTempBuffer);

			if (GC.getBonusInfo(eBonus).getHealth() != 0)
			{
				szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo(eBonus).getHealth()), ((GC.getBonusInfo(eBonus).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
				szString.append(szTempBuffer);
			}

			if (GC.getBonusInfo(eBonus).getHappiness() != 0)
			{
				szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo(eBonus).getHappiness()), ((GC.getBonusInfo(eBonus).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
				szString.append(szTempBuffer);
			}

			if ((pPlot->getImprovementType() == NO_IMPROVEMENT) || !(GC.getImprovementInfo(pPlot->getImprovementType()).isImprovementBonusTrade(eBonus)))
			{
				if (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)GC.getBonusInfo(eBonus).getTechCityTrade())))
				{
					szString.append(gDLL->getText("TXT_KEY_PLOT_RESEARCH", GC.getTechInfo((TechTypes)GC.getBonusInfo(eBonus).getTechCityTrade()).getTextKeyWide()));
				}

				/** disabled, show Bonus to nearby City instead
				if (!pPlot->isCity())
				{
					for (iI = 0; iI < GC.getNumBuildInfos(); ++iI)
					{
						if (GC.getBuildInfo((BuildTypes) iI).getImprovement() != NO_IMPROVEMENT)
						{
							CvImprovementInfo& kImprovementInfo = GC.getImprovementInfo((ImprovementTypes) GC.getBuildInfo((BuildTypes) iI).getImprovement());
							if (kImprovementInfo.isImprovementBonusTrade(eBonus))
							{
								if (pPlot->canHaveImprovement(((ImprovementTypes)(GC.getBuildInfo((BuildTypes) iI).getImprovement())), GC.getGameINLINE().getActiveTeam(), true))
								{
									if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)GC.getBuildInfo((BuildTypes) iI).getTechPrereq()))
									{
										szString.append(gDLL->getText("TXT_KEY_PLOT_REQUIRES", kImprovementInfo.getTextKeyWide()));
									}
									else if (GC.getBonusInfo(eBonus).getTechCityTrade() != GC.getBuildInfo((BuildTypes) iI).getTechPrereq())
									{
										szString.append(gDLL->getText("TXT_KEY_PLOT_RESEARCH", GC.getTechInfo((TechTypes) GC.getBuildInfo((BuildTypes) iI).getTechPrereq()).getTextKeyWide()));
									}

									bool bFirst = true;

									for (int k = 0; k < NUM_YIELD_TYPES; k++)
									{
										int iYieldChange = kImprovementInfo.getImprovementBonusYield(eBonus, k) + kImprovementInfo.getYieldChange(k);
										if (iYieldChange != 0)
										{
											if (iYieldChange > 0)
											{
												szTempBuffer.Format(L"+%d%c", iYieldChange, GC.getYieldInfo((YieldTypes)k).getChar());
											}
											else
											{
												szTempBuffer.Format(L"%d%c", iYieldChange, GC.getYieldInfo((YieldTypes)k).getChar());
											}
											setListHelp(szString, L"\n", szTempBuffer, L", ", bFirst);
											bFirst = false;
										}
									}

									if (!bFirst)
									{
										szString.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", kImprovementInfo.getTextKeyWide()));
									}

									break;
								}
							}
						}
					}
				}
				**/
				int iCityBonusModifier = GC.getBonusInfo(eBonus).getCityBonusModifier();
				if (iCityBonusModifier != 0) {

					if (GC.getBonusInfo(eBonus).getCityYieldModifierType() != NO_YIELD) {
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_BONUS_CITY_MODIFIER", iCityBonusModifier, GC.getYieldInfo((YieldTypes)GC.getBonusInfo(eBonus).getCityYieldModifierType()).getChar()));
					}

					if (GC.getBonusInfo(eBonus).getCityCommerceModifierType() != NO_COMMERCE) {
						szString.append(NEWLINE);
						szString.append(gDLL->getText("TXT_KEY_BONUS_CITY_MODIFIER", iCityBonusModifier, GC.getCommerceInfo((CommerceTypes)GC.getBonusInfo(eBonus).getCityCommerceModifierType()).getChar()));
					}
				}
			}
			else if (!(pPlot->isBonusNetwork(GC.getGameINLINE().getActiveTeam())))
			{
				szString.append(gDLL->getText("TXT_KEY_PLOT_REQUIRES_ROUTE"));
			}

			if (!CvWString(GC.getBonusInfo(eBonus).getHelp()).empty())
			{
				szString.append(NEWLINE);
				szString.append(GC.getBonusInfo(eBonus).getHelp());
			}
		}

		eImprovement = pPlot->getRevealedImprovementType(GC.getGameINLINE().getActiveTeam(), true);

		if (eImprovement != NO_IMPROVEMENT)
		{
			szString.append(NEWLINE);
			szString.append(GC.getImprovementInfo(eImprovement).getDescription());

			bFound = false;

			for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
			{
				if (GC.getImprovementInfo(eImprovement).getIrrigatedYieldChange(iI) != 0)
				{
					bFound = true;
					break;
				}
			}

			if (bFound)
			{
				if (pPlot->isIrrigationAvailable())
				{
					szString.append(gDLL->getText("TXT_KEY_PLOT_IRRIGATED"));
				}
				else
				{
					szString.append(gDLL->getText("TXT_KEY_PLOT_NOT_IRRIGATED"));
				}
			}

			if (pPlot->isPlundered()) {
				szString.append(gDLL->getText("TXT_KEY_PLOT_PLUNDERED", pPlot->getPlundered()));
			}

			if (GC.getImprovementInfo(eImprovement).getImprovementUpgrade() != NO_IMPROVEMENT)
			{

				//FfH: Modified by Kael 05/24/2008
				//				if ((pPlot->getUpgradeProgress() > 0) || pPlot->isBeingWorked())
				//				{
				//					iTurns = pPlot->getUpgradeTimeLeft(eImprovement, eRevealOwner);
				//
				//					szString.append(gDLL->getText("TXT_KEY_PLOT_IMP_UPGRADE", iTurns, GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide()));
				//				}
				//				else
				//				{
				//					szString.append(gDLL->getText("TXT_KEY_PLOT_WORK_TO_UPGRADE", GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide()));
				//				}
				if (GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getPrereqCivilization() == NO_CIVILIZATION ||
					GC.getGameINLINE().getActivePlayer() == NO_PLAYER ||
					GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType() == GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getPrereqCivilization())
				{
					// Super Forts begin *text* *upgrade*
					if ((pPlot->getUpgradeProgress() > 0) || (pPlot->isBeingWorked() && (
						!GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) ||
						!GC.getImprovementInfo(eImprovement).isUpgradeRequiresFortify()
						)
						)
					)
					// if ((pPlot->getUpgradeProgress() > 0) || pPlot->isBeingWorked()) - Original Code
					// Super Forts end
					{
						iTurns = pPlot->getUpgradeTimeLeft(eImprovement, eRevealOwner);
						szString.append(gDLL->getText("TXT_KEY_PLOT_IMP_UPGRADE", iTurns, GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide()));
					}
					else
					{
						if (GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getPrereqCivilization() == NO_CIVILIZATION)
						{
							// Super Forts begin *text* *upgrade*
							if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(eImprovement).getCulture() > 0)
							{
								szString.append(gDLL->getText("TXT_KEY_PLOT_FORTIFY_TO_UPGRADE", GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide()));
							}
							else
							{
								szString.append(gDLL->getText("TXT_KEY_PLOT_WORK_TO_UPGRADE", GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide()));
							}
							// Super Forts end
						}
						else
						{
							szString.append(gDLL->getText("TXT_KEY_PLOT_WORK_TO_UPGRADE_PREREQ_CIV", GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide(), GC.getCivilizationInfo((CivilizationTypes)GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getPrereqCivilization()).getDescription()));
						}
					}
				}
				//FfH: End Modify

			}

			getDiscoverChanceText(pPlot, szString, szTempBuffer);
		}

		if (pPlot->getRevealedRouteType(GC.getGameINLINE().getActiveTeam(), true) != NO_ROUTE)
		{
			szString.append(NEWLINE);
			szString.append(GC.getRouteInfo(pPlot->getRevealedRouteType(GC.getGameINLINE().getActiveTeam(), true)).getDescription());
		}

		if (pPlot->getBlockadedCount(GC.getGameINLINE().getActiveTeam()) > 0)
		{
			szString.append(CvWString::format(SETCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PLOT_BLOCKADED"));
			szString.append(CvWString::format(ENDCOLR));
		}

	}

	if (pPlot->isWater()) {
		getDiscoverChanceText(pPlot, szString, szTempBuffer);
	}

	if (pPlot->getFeatureType() != NO_FEATURE)
	{
		int iDamage = GC.getFeatureInfo(pPlot->getFeatureType()).getTurnDamage();

		if (iDamage > 0)
		{
			szString.append(CvWString::format(SETCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PLOT_DAMAGE", iDamage));
			szString.append(CvWString::format(ENDCOLR));
		}
	}

	if (!(bShift && !bAlt && (gDLL->getChtLvl() > 0)) && GC.getDefineINT("SHOW_WILDERNESS_INFO") == 1)
	{
		szTempBuffer.Format(L"\nWilderness Value: %d", pPlot->getWilderness());
		szString.append(NEWLINE);
		szString.append(szTempBuffer);
	}

	/** Improvement Slots
		if (pPlot->getWorkingCity() != NULL && pPlot->getOwnerINLINE()==GC.getGameINLINE().getActivePlayer())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_PLOT_INFRASTRUCTURE_HELP",pPlot->getWorkingCity()->getNameKey(),pPlot->getWorkingCity()->calculateImprovementInfrastructureCostFreeLeft()/100));
			int iAdditionalFavoredImps=GC.getDefineINT("FREE_FAVORED_IMPROVEMENTS_PER_CITY")-pPlot->getWorkingCity()->getNumFavoredImprovements();
			int iFavoredImp=GC.getCivilizationInfo(GET_PLAYER(pPlot->getWorkingCity()->getOwnerINLINE()).getCivilizationType()).getImpInfrastructureHalfCost();
			if(iAdditionalFavoredImps>0 && iFavoredImp!=NO_IMPROVEMENT)
			{
				szString.append(gDLL->getText("TXT_KEY_PLOT_FAV_INFRASTRUCTURE_HELP",iAdditionalFavoredImps,getLinkedText((ImprovementTypes)iFavoredImp,true,false).c_str()));
			}
		}
	**/
	//FfH: Added by Kael 05/10/2008
	//SpyFanatic: avoid to show minimum level -1 to enter....
	//if (pPlot->getMinLevel() != 0)
	if (pPlot->getMinLevel() > 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_PLOT_MIN_LEVEL", pPlot->getMinLevel()));
	}
	//FfH: End Add

}

void CvGameTextMgr::getDiscoverChanceText(CvPlot* pPlot, CvWStringBuffer& szString, CvWString szTempBuffer) {
	bool bFirst = true;

	if (pPlot->getOwner() != GC.getGameINLINE().getActivePlayer())
		return;

	for (int iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		int iChance = 0;
		if (pPlot->isWater())
			iChance += pPlot->getPossibleWaterBonusDiscoverChance((BonusTypes)iI, GC.getGameINLINE().getActivePlayer());
		else
			iChance += pPlot->getLandBonusDiscoverChance((BonusTypes)iI, GC.getGameINLINE().getActivePlayer());

		if (iChance > 0) {
			if (bFirst) {
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_PLOT_BONUS_DISCOVERCHANCE"));
				bFirst = false;
			}
			else {
				szString.append(L", ");
			}

			szTempBuffer.assign(getLinkedText((BonusTypes)iI));
			szString.append(szTempBuffer);
			szTempBuffer.Format(L"(%d.%03d%%)", iChance / 1000, iChance % 1000);
			szString.append(szTempBuffer);
		}
	}
}

void CvGameTextMgr::setCityPlotYieldValueString(CvWStringBuffer& szString, CvCity* pCity, int iIndex, bool bAvoidGrowth, bool bIgnoreGrowth, bool bIgnoreFood)
{
	PROFILE_FUNC();

	CvPlot* pPlot = NULL;

	if (iIndex >= 0 && iIndex < NUM_CITY_PLOTS)
		pPlot = pCity->getCityIndexPlot(iIndex);

	if (pPlot != NULL)
	{
		CvCityAI* pCityAI = static_cast<CvCityAI*>(pCity);
		bool bWorkingPlot = pCity->isWorkingPlot(iIndex);

		int iValue = pCityAI->AI_plotValue(pPlot, bAvoidGrowth, /*bRemove*/ bWorkingPlot, bIgnoreFood, bIgnoreGrowth);

		setYieldValueString(szString, iValue, /*bActive*/ bWorkingPlot);
	}
	else
		setYieldValueString(szString, 0, /*bActive*/ false, /*bMakeWhitespace*/ true);
}

void CvGameTextMgr::setYieldValueString(CvWStringBuffer& szString, int iValue, bool bActive, bool bMakeWhitespace)
{
	PROFILE_FUNC();

	static bool bUseFloats = false;

	if (bActive)
		szString.append(CvWString::format(SETCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT")));
	else
		szString.append(CvWString::format(SETCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT")));

	if (!bMakeWhitespace)
	{
		if (bUseFloats)
		{
			float fValue = ((float)iValue) / 10000;
			szString.append(CvWString::format(L"%2.3f " ENDCOLR, fValue));
		}
		else
			szString.append(CvWString::format(L"%04d  " ENDCOLR, iValue / 10));
	}
	else
		szString.append(CvWString::format(L"         " ENDCOLR));
}

void CvGameTextMgr::setCityBarHelp(CvWStringBuffer& szString, CvCity* pCity)
{
	PROFILE_FUNC();

	CvWString szTempBuffer;
	bool bFirst;
	int iFoodDifference;
	int iProductionDiffNoFood;
	int iProductionDiffJustFood;
	int iRate;
	int iI;

	iFoodDifference = pCity->foodDifference();

	szString.append(pCity->getName());

	szString.append(NEWLINE);
	szString.append(gDLL->getText("TXT_KEY_CITY_BAR_STATUS", pCity->unhappyLevel(), pCity->happyLevel(), pCity->badHealth(), pCity->goodHealth()));

	if (iFoodDifference <= 0)
	{
		szString.append(gDLL->getText("TXT_KEY_CITY_BAR_GROWTH", pCity->getFood(), pCity->growthThreshold()));
	}
	else
	{
		szString.append(gDLL->getText("TXT_KEY_CITY_BAR_FOOD_GROWTH", iFoodDifference, pCity->getFood(), pCity->growthThreshold(), pCity->getFoodTurnsLeft()));
	}
	if (pCity->getProductionNeeded() != MAX_INT)
	{
		iProductionDiffNoFood = pCity->getCurrentProductionDifference(true, true);
		iProductionDiffJustFood = (pCity->getCurrentProductionDifference(false, true) - iProductionDiffNoFood);

		if (iProductionDiffJustFood > 0)
		{
			szString.append(gDLL->getText("TXT_KEY_CITY_BAR_FOOD_HAMMER_PRODUCTION", iProductionDiffJustFood, iProductionDiffNoFood, pCity->getProductionName(), pCity->getProduction(), pCity->getProductionNeeded(), pCity->getProductionTurnsLeft()));
		}
		else if (iProductionDiffNoFood > 0)
		{
			szString.append(gDLL->getText("TXT_KEY_CITY_BAR_HAMMER_PRODUCTION", iProductionDiffNoFood, pCity->getProductionName(), pCity->getProduction(), pCity->getProductionNeeded(), pCity->getProductionTurnsLeft()));
		}
		else
		{
			szString.append(gDLL->getText("TXT_KEY_CITY_BAR_PRODUCTION", pCity->getProductionName(), pCity->getProduction(), pCity->getProductionNeeded()));
		}

		// BUG - Building Actual Effects - start
		// merged Sephi
		if (pCity->getOwnerINLINE() == GC.getGame().getActivePlayer())
		{
			if (pCity->isProductionBuilding())
			{
				BuildingTypes eBuilding = pCity->getProductionBuilding();
				CvWString szStart;

				// szStart.Format(NEWLINE L"<img=%S size=24></img>", GC.getBuildingInfo(eBuilding).getButton());
				setBuildingActualEffects(szString, szStart, eBuilding, pCity, false);
			}
		}
		// BUG - Building Actual Effects - end

	}

	bFirst = true;

	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		iRate = pCity->getCommerceRateTimes100((CommerceTypes)iI);

		if (iRate != 0)
		{
			szTempBuffer.Format(L"%d.%02d %c", iRate / 100, iRate % 100, GC.getCommerceInfo((CommerceTypes)iI).getChar());
			setListHelp(szString, NEWLINE, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}

	iRate = pCity->getGreatPeopleRate();

	if (iRate != 0)
	{
		szTempBuffer.Format(L"%d%c", iRate, gDLL->getSymbolID(GREAT_PEOPLE_CHAR));
		setListHelp(szString, NEWLINE, szTempBuffer, L", ", bFirst);
		bFirst = false;
	}

	if (!bFirst)
	{
		szString.append(gDLL->getText("TXT_KEY_PER_TURN"));
	}

	szString.append(NEWLINE);
	szString.append(gDLL->getText("INTERFACE_CITY_MAINTENANCE"));
	int iMaintenance = pCity->getMaintenanceTimes100();
	szString.append(CvWString::format(L" -%d.%02d %c", iMaintenance / 100, iMaintenance % 100, GC.getCommerceInfo(COMMERCE_GOLD).getChar()));

	bFirst = true;
	for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
	{
		if (pCity->getNumRealBuilding((BuildingTypes)iI) > 0)
		{
			setListHelp(szString, NEWLINE, GC.getBuildingInfo((BuildingTypes)iI).getDescription(), L", ", bFirst);
			bFirst = false;
		}
	}

	if (pCity->getCultureLevel() != NO_CULTURELEVEL)
	{
		szString.append(gDLL->getText("TXT_KEY_CITY_BAR_CULTURE", pCity->getCulture(pCity->getOwnerINLINE()), pCity->getCultureThreshold(), GC.getCultureLevelInfo(pCity->getCultureLevel()).getTextKeyWide()));
	}

	if (pCity->getGreatPeopleProgress() > 0)
	{
		szString.append(gDLL->getText("TXT_KEY_CITY_BAR_GREAT_PEOPLE", pCity->getGreatPeopleProgress(), GET_PLAYER(pCity->getOwnerINLINE()).greatPeopleThreshold(false)));
	}

	int iNumUnits = pCity->plot()->countNumAirUnits(GC.getGameINLINE().getActiveTeam());
	if (pCity->getAirUnitCapacity(GC.getGameINLINE().getActiveTeam()) > 0 && iNumUnits > 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_CITY_BAR_AIR_UNIT_CAPACITY", iNumUnits, pCity->getAirUnitCapacity(GC.getGameINLINE().getActiveTeam())));
	}

	//FfH: Added by Kael 12/02/2007
	szString.append(NEWLINE);
	szString.append(gDLL->getText("TXT_KEY_CITY_BAR_CRIME", pCity->getCrime()));
	//FfH: End Add
	/*************************************************************************************************/
	/**	ADDON (New Building tags) Sephi                                                             **/
	/*************************************************************************************************/
	if (GET_PLAYER(pCity->getOwnerINLINE()).getPopulationLimit() > -1)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_CITY_POPULATION_LIMIT", GET_PLAYER(pCity->getOwnerINLINE()).getPopulationLimit() + pCity->getPopulationLimit()));
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	/*************************************************************************************************/
	/**	New Gameoption (Passive XP Training) Sephi                                				    **/
	/*************************************************************************************************/


	bFirst = true;
	szTempBuffer.clear();
	for (int iLoopCombat = 0; iLoopCombat < GC.getNumUnitCombatInfos(); iLoopCombat++)
	{
		// City can train the UnitCombat?
		int iTrainCap = pCity->getTrainXPCap(iLoopCombat) + GET_PLAYER(pCity->getOwnerINLINE()).getTrainXPCap(iLoopCombat);
		if (iTrainCap > 0)
		{
			szTempBuffer.clear();
			if (bFirst)
			{
				szTempBuffer += gDLL->getText("TXT_KEY_CITY_BAR_PASSIVE_TRAINING");
			}
			szTempBuffer += GC.getUnitCombatInfo((UnitCombatTypes)iLoopCombat).getDescription();
			szTempBuffer += CvWString::format(L" %d ", iTrainCap);
			setListHelp(szString, NEWLINE, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}

	//Cheat Debug
	if (GC.getGameINLINE().isDebugMode())
	{
		CvCityAI* pCityAI = static_cast<CvCityAI*>(pCity);
		if (pCityAI->getAIGroup_Defense() != NULL)
		{
			szString.append(NEWLINE);
			szTempBuffer.clear();
			szTempBuffer += CvWString::format(L"City Defense Units: %d/%d ", pCityAI->getAIGroup_Defense()->getNumUnits(), pCityAI->AI_neededPermDefense());
			szString.append(szTempBuffer);
			szString.append(NEWLINE);
			szTempBuffer.clear();
			szTempBuffer += CvWString::format(L"City Danger Value: %d ", pCityAI->AI_dangerValue());
			szString.append(szTempBuffer);
		}
		szString.append(NEWLINE);
		if (pCityAI->AI_getCitySpecialization() != NO_CITYSPECIALIZATION)
		{
			szString.append(GC.getCitySpecializationInfo(pCityAI->AI_getCitySpecialization()).getDescription());
		}
		else
		{
			szTempBuffer.clear();
			szTempBuffer += CvWString::format(L"NO_CITYSPECIALIZATION");
			szString.append(szTempBuffer);
		}
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	szString.append(NEWLINE);

	szString.append(gDLL->getText("TXT_KEY_CITY_BAR_SELECT", pCity->getNameKey()));
	szString.append(gDLL->getText("TXT_KEY_CITY_BAR_SELECT_CTRL"));
	szString.append(gDLL->getText("TXT_KEY_CITY_BAR_SELECT_ALT"));
}


//>>>>	BUGFfH: Modified by Denev 2009/09/11
//void CvGameTextMgr::parseTraits(CvWStringBuffer &szHelpString, TraitTypes eTrait, CivilizationTypes eCivilization, bool bDawnOfMan)
void CvGameTextMgr::parseTraits(CvWStringBuffer& szHelpString, TraitTypes eTrait, CivilizationTypes eCivilization, bool bTemporary, bool bCivilopediaText)
//<<<<	BUGFfH: End Modify

{
	PROFILE_FUNC();

	CvWString szTempBuffer;
	BuildingTypes eLoopBuilding;
	UnitTypes eLoopUnit;
	int iLast;
	int iI, iJ;
	bool bFirst;
	CvWString szText;
	CvTraitInfo& kTrait = GC.getTraitInfo(eTrait);
	// Trait Name
//>>>>	BUGFfH: Modified by Denev 2009/09/27
/*	dawn of man screen doesn't call this function no longer	*/
/*
	szText = GC.getTraitInfo(eTrait).getDescription();
	if (bDawnOfMan)
	{
		szTempBuffer.Format(L"%s", szText.GetCString());
	}
	else
	{
		szTempBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
	}
*/
	if (!bCivilopediaText)
	{
		szTempBuffer.assign(getLinkedText(eTrait, true, false, bTemporary));
	}
	//<<<<	BUGFfH: End Modify
	szHelpString.append(szTempBuffer);

	//>>>>	BUGFfH: Deleted by Denev 2009/09/27
	/*
		if (!bDawnOfMan)
		{
	*/
	//<<<<	BUGFfH: End Delete
	if (!CvWString(GC.getTraitInfo(eTrait).getHelp()).empty())
	{
		szHelpString.append(GC.getTraitInfo(eTrait).getHelp());
	}

	// iHealth
	if (GC.getTraitInfo(eTrait).getHealth() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_HEALTH", GC.getTraitInfo(eTrait).getHealth()));
	}

	// iHappiness
	if (GC.getTraitInfo(eTrait).getHappiness() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_HAPPINESS", GC.getTraitInfo(eTrait).getHappiness()));
	}

	// iMaxAnarchy
	if (GC.getTraitInfo(eTrait).getMaxAnarchy() != -1)
	{
		if (GC.getTraitInfo(eTrait).getMaxAnarchy() == 0)
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_NO_ANARCHY"));
		}
		else
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_MAX_ANARCHY", GC.getTraitInfo(eTrait).getMaxAnarchy()));
		}
	}

	// iUpkeepModifier
	if (GC.getTraitInfo(eTrait).getUpkeepModifier() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_CIVIC_UPKEEP_MODIFIER", GC.getTraitInfo(eTrait).getUpkeepModifier()));
	}

	// iLevelExperienceModifier
	if (GC.getTraitInfo(eTrait).getLevelExperienceModifier() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_CIVIC_LEVEL_MODIFIER", GC.getTraitInfo(eTrait).getLevelExperienceModifier()));
	}

	// iGreatPeopleRateModifier
	if (GC.getTraitInfo(eTrait).getGreatPeopleRateModifier() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_GREAT_PEOPLE_MODIFIER", GC.getTraitInfo(eTrait).getGreatPeopleRateModifier()));
	}

	// iGreatGeneralRateModifier
	if (GC.getTraitInfo(eTrait).getGreatGeneralRateModifier() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_GREAT_GENERAL_MODIFIER", GC.getTraitInfo(eTrait).getGreatGeneralRateModifier()));
	}

	if (GC.getTraitInfo(eTrait).getDomesticGreatGeneralRateModifier() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_DOMESTIC_GREAT_GENERAL_MODIFIER", GC.getTraitInfo(eTrait).getDomesticGreatGeneralRateModifier()));
	}

	// Wonder Production Effects
	if ((GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() != 0)
		|| (GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier() != 0)
		|| (GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier() != 0))
	{
		if ((GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() == GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier())
			&& (GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() == GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier()))
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier()));
		}
		else
		{
			if (GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier() != 0)
			{
				szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_WORLD_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxGlobalBuildingProductionModifier()));
			}

			if (GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier() != 0)
			{
				szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_TEAM_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxTeamBuildingProductionModifier()));
			}

			if (GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier() != 0)
			{
				szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_NATIONAL_WONDER_PRODUCTION_MODIFIER", GC.getTraitInfo(eTrait).getMaxPlayerBuildingProductionModifier()));
			}
		}
	}

	// ExtraYieldThresholds
	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		if (GC.getTraitInfo(eTrait).getExtraYieldThreshold(iI) > 0)
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_EXTRA_YIELD_THRESHOLDS", GC.getYieldInfo((YieldTypes)iI).getChar(), GC.getTraitInfo(eTrait).getExtraYieldThreshold(iI), GC.getYieldInfo((YieldTypes)iI).getChar()));
		}
		// Trade Yield Modifiers
		if (GC.getTraitInfo(eTrait).getTradeYieldModifier(iI) != 0)
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_TRADE_YIELD_MODIFIERS", GC.getTraitInfo(eTrait).getTradeYieldModifier(iI), GC.getYieldInfo((YieldTypes)iI).getChar(), "YIELD"));
		}
	}

	// CommerceChanges
	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		if (GC.getTraitInfo(eTrait).getCommerceChange(iI) != 0)
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_COMMERCE_CHANGES", GC.getTraitInfo(eTrait).getCommerceChange(iI), GC.getCommerceInfo((CommerceTypes)iI).getChar(), "COMMERCE"));
		}

		if (GC.getTraitInfo(eTrait).getCommerceModifier(iI) != 0)
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_COMMERCE_MODIFIERS", GC.getTraitInfo(eTrait).getCommerceModifier(iI), GC.getCommerceInfo((CommerceTypes)iI).getChar(), "COMMERCE"));
		}
	}

	// Free Promotions
	bool bFoundPromotion = false;
	szTempBuffer.clear();
	for (iI = 0; iI < GC.getNumPromotionInfos(); ++iI)
	{
		if (GC.getTraitInfo(eTrait).isFreePromotion(iI))
		{
			if (bFoundPromotion)
			{
				szTempBuffer += L", ";
			}

			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//				szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getPromotionInfo((PromotionTypes) iI).getDescription());
			szTempBuffer += getLinkedText((PromotionTypes)iI, false);
			//<<<<	BUGFfH: End Modify
			bFoundPromotion = true;
		}
	}

	if (bFoundPromotion)
	{
		bFirst = true;
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_FREE_PROMOTIONS", szTempBuffer.GetCString()));

		for (iJ = 0; iJ < GC.getNumUnitCombatInfos(); iJ++)
		{
			if (GC.getTraitInfo(eTrait).isFreePromotionUnitCombat(iJ))
			{
				if (bFirst) {
					bFirst = false;
					szTempBuffer.Format(L"%s	%c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR));
					szHelpString.append(szTempBuffer);
					szHelpString.append(getLinkedText((UnitCombatTypes)iJ));
				}
				else {
					szHelpString.append(L", ");
					szHelpString.append(getLinkedText((UnitCombatTypes)iJ));
				}
			}
		}
	}

	// No Civic Maintenance
	for (iI = 0; iI < GC.getNumCivicOptionInfos(); ++iI)
	{
		if (GC.getCivicOptionInfo((CivicOptionTypes)iI).getTraitNoUpkeep(eTrait))
		{
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_NO_UPKEEP", GC.getCivicOptionInfo((CivicOptionTypes)iI).getTextKeyWide()));
		}
	}

	// Increase Building/Unit Production Speeds
	iLast = 0;
	for (iI = 0; iI < GC.getNumSpecialUnitInfos(); ++iI)
	{
		if (GC.getSpecialUnitInfo((SpecialUnitTypes)iI).getProductionTraits(eTrait) != 0)
		{
			if (GC.getSpecialUnitInfo((SpecialUnitTypes)iI).getProductionTraits(eTrait) == 100)
			{
				szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
			}
			else
			{
				szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getSpecialUnitInfo((SpecialUnitTypes)iI).getProductionTraits(eTrait));
			}
			setListHelp(szHelpString, szText.GetCString(), GC.getSpecialUnitInfo((SpecialUnitTypes)iI).getDescription(), L", ", (GC.getSpecialUnitInfo((SpecialUnitTypes)iI).getProductionTraits(eTrait) != iLast));
			iLast = GC.getSpecialUnitInfo((SpecialUnitTypes)iI).getProductionTraits(eTrait);
		}
	}

	// Unit Classes
	iLast = 0;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
	{
		if (eCivilization == NO_CIVILIZATION)
		{
			eLoopUnit = ((UnitTypes)(GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()));
		}
		else
		{
			eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationUnits(iI)));
		}

		if (eLoopUnit != NO_UNIT && !isWorldUnitClass((UnitClassTypes)iI))
		{
			if (GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait) != 0)
			{
				if (GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait) == 100)
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
				}
				else
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait));
				}
				CvWString szUnit;
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//					szUnit.Format(L"<link=literal>%s</link>", GC.getUnitInfo(eLoopUnit).getDescription());
				szUnit = getLinkedText(eLoopUnit, false);
				//<<<<	BUGFfH: End Modify
				setListHelp(szHelpString, szText.GetCString(), szUnit, L", ", (GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait) != iLast));
				iLast = GC.getUnitInfo(eLoopUnit).getProductionTraits(eTrait);
			}
		}
	}

	// SpecialBuildings
	iLast = 0;
	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); ++iI)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getProductionTraits(eTrait) != 0)
		{
			if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getProductionTraits(eTrait) == 100)
			{
				szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
			}
			else
			{
				szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getProductionTraits(eTrait));
			}
			setListHelp(szHelpString, szText.GetCString(), GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getDescription(), L", ", (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getProductionTraits(eTrait) != iLast));
			iLast = GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getProductionTraits(eTrait);
		}
	}

	// Buildings
	iLast = 0;
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		if (eCivilization == NO_CIVILIZATION)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()));
		}
		else
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
		}

		if (eLoopBuilding != NO_BUILDING && !isWorldWonderClass((BuildingClassTypes)iI))
		{
			if (GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait) != 0)
			{
				if (GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait) == 100)
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_DOUBLE_SPEED");
				}
				else
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_PRODUCTION_MODIFIER", GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait));
				}

				CvWString szBuilding;
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//					szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingInfo(eLoopBuilding).getDescription());
				szBuilding = getLinkedText(eLoopBuilding, false);
				//<<<<	BUGFfH: End Modify
				setListHelp(szHelpString, szText.GetCString(), szBuilding, L", ", (GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait) != iLast));
				iLast = GC.getBuildingInfo(eLoopBuilding).getProductionTraits(eTrait);
			}
		}
	}

	iLast = 0;
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		if (eCivilization == NO_CIVILIZATION)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()));
		}
		else
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
		}

		if (eLoopBuilding != NO_BUILDING && !isWorldWonderClass((BuildingClassTypes)iI))
		{
			if (kTrait.isIgnoreBuildingGYCost(iI)) {
				szText = gDLL->getText("TXT_KEY_TRAIT_IGNORE_GYCOST");

				CvWString szBuilding;
				szBuilding = getLinkedText(eLoopBuilding, false);

				setListHelp(szHelpString, szText.GetCString(), szBuilding, L", ", (kTrait.isIgnoreBuildingGYCost(iI) != (bool)iLast));
				iLast = kTrait.isIgnoreBuildingGYCost(iI);
			}
		}
	}

	// Buildings
	iLast = 0;
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		if (eCivilization == NO_CIVILIZATION)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()));
		}
		else
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
		}

		if (eLoopBuilding != NO_BUILDING && !isWorldWonderClass((BuildingClassTypes)iI))
		{
			int iHappiness = GC.getBuildingInfo(eLoopBuilding).getHappinessTraits(eTrait);
			if (iHappiness != 0)
			{
				if (iHappiness > 0)
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_BUILDING_HAPPINESS", iHappiness, gDLL->getSymbolID(HAPPY_CHAR));
				}
				else
				{
					szText = gDLL->getText("TXT_KEY_TRAIT_BUILDING_HAPPINESS", -iHappiness, gDLL->getSymbolID(UNHAPPY_CHAR));
				}

				CvWString szBuilding;
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//					szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingInfo(eLoopBuilding).getDescription());
				szBuilding = getLinkedText(eLoopBuilding, false);
				//<<<<	BUGFfH: End Modify
				setListHelp(szHelpString, szText.GetCString(), szBuilding, L", ", (iHappiness != iLast));
				iLast = iHappiness;
			}
		}
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/24
	bFirst = true;
	for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); iBuilding++)
	{
		if (eTrait == GC.getBuildingInfo((BuildingTypes)iBuilding).getPrereqTrait())
		{
			szText = gDLL->getText("TXT_KEY_TRAIT_CAN_BUILD");
			setListHelp(szHelpString, szText, getLinkedText((BuildingTypes)iBuilding, false), L", ", bFirst);
			bFirst = false;
		}
	}
	//<<<<	BUGFfH: End Add

	//FfH: Added by Kael 08/02/2007
	if (GC.getTraitInfo(eTrait).isAdaptive())
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_ADAPTIVE_HELP"));
		//>>>>	BUGFfH: Added by Denev 2009/09/11
		if (bCivilopediaText)
		{
			szTempBuffer.Format(L"%s\t", NEWLINE);
			szHelpString.append(szTempBuffer);
			szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_SELECTABLE"));
			for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
			{
				if (GC.getTraitInfo((TraitTypes)iI).isSelectable())
				{
					szTempBuffer.Format(L"%s\t\t%c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR));
					szTempBuffer.append(getLinkedText((TraitTypes)iI));
					szHelpString.append(szTempBuffer);
				}
			}
		}
		//<<<<	BUGFfH: End Add
	}
	if (GC.getTraitInfo(eTrait).isIgnoreUnHealthFromImprovements()) {
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_IGNORE_UNHEALTH_IMPROVEMENTS_HELP"));
	}

	if (kTrait.getExtraImprovementPerCity() != 0) {
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_EXTRA_IMPROVEMENT_CITY_HELP", kTrait.getExtraImprovementPerCity()));
	}
	if (kTrait.getExtraManaNodesPercent() != 0) {
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_EXTRA_MANA_NODES_HELP", kTrait.getExtraManaNodesPercent()));
	}
	if (kTrait.getReducedHurryCostBuilding() != 0) {
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_REDUCED_HURRY_COST_BUILDINGS_HELP", kTrait.getReducedHurryCostBuilding()));
	}
	if (kTrait.getExtraSupportUnitLimit() != 0) {
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_EXTRA_SUPPORT_UNITLIMIT_HELP", kTrait.getExtraSupportUnitLimit()));
	}
	if (kTrait.getExtraBuildingCapturedChance() != 0) {
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_EXTRA_BUILDING_CAPTURE_CHANCE_HELP", kTrait.getExtraBuildingCapturedChance()));
	}

	if (GC.getTraitInfo(eTrait).isAgnostic()) {
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_AGNOSTIC_HELP"));
	}

	if (GC.getTraitInfo(eTrait).isAssimilation()) {
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_ASSIMILATION_HELP"));
	}
	if (GC.getTraitInfo(eTrait).isBarbarianAlly()) {
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_BARBARIAN_ALLY_HELP"));
	}
	if (GC.getTraitInfo(eTrait).isIgnoreFood()) {
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_IGNORE_FOOD_HELP"));
	}
	if (GC.getTraitInfo(eTrait).isInsane()) {
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_INSANE_HELP"));
	}
	if (GC.getTraitInfo(eTrait).isSprawling()) {
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_SPRAWLING_HELP"));
	}
	if (GC.getTraitInfo(eTrait).getMaxCities() != -1)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_MAX_CITIES_HELP", (GC.getTraitInfo(eTrait).getMaxCities() + GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getMaxCitiesMod())));
	}
	if (GC.getTraitInfo(eTrait).getFreeXPFromCombat() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_FREE_XP_FROM_COMBAT_HELP", GC.getTraitInfo(eTrait).getFreeXPFromCombat()));
	}
	if (GC.getTraitInfo(eTrait).getPillagingGold() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_PILLAGING_GOLD_HELP", GC.getTraitInfo(eTrait).getPillagingGold()));
	}
	if (GC.getTraitInfo(eTrait).getStartingGold() != 0)
	{
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_STARTING_GOLD_HELP", GC.getTraitInfo(eTrait).getStartingGold()));
	}
	if (GC.getTraitInfo(eTrait).getSummonCostReduction() != 0)
	{
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_SUMMON_COST_REDUCTION_HELP", -GC.getTraitInfo(eTrait).getSummonCostReduction()));
	}
	if (GC.getTraitInfo(eTrait).getTaxesModifier() != 0)
	{
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_TAXES_MODIFIER_HELP", GC.getTraitInfo(eTrait).getTaxesModifier()));
	}
	if (GC.getTraitInfo(eTrait).getFoodPerPopulationModifier() != 0)
	{
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_FOOD_PER_POPULATION_HELP", GC.getTraitInfo(eTrait).getFoodPerPopulationModifier()));
	}
	if (GC.getTraitInfo(eTrait).getUpgradeCostModifier() != 0)
	{
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_UPGRADE_COST_MODIFIER_HELP", GC.getTraitInfo(eTrait).getUpgradeCostModifier()));
	}
	if (GC.getTraitInfo(eTrait).getProductionCostReductionCapitalBuilding() != 0)
	{
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_REDUCED_COST_BY_CAPITAL_BUILDING_HELP", GC.getTraitInfo(eTrait).getProductionCostReductionCapitalBuilding()));
	}
	if (GC.getTraitInfo(eTrait).getUnitSupportModifier() != 0)
	{
		szHelpString.append(NEWLINE);
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_UNIT_SUPPORT_MODIFIER_HELP", GC.getTraitInfo(eTrait).getUnitSupportModifier()));
	}
	//FfH: End Add
	if (GC.getTraitInfo(eTrait).getManaFromImprovementType() != NO_IMPROVEMENT)
	{
		CvWString szMana;
		szMana.Format(L"%.2f", 0.01f * GC.getTraitInfo(eTrait).getManaFromImprovement());
		szHelpString.append(gDLL->getText("TXT_KEY_TRAIT_MANA_FROM_IMPROVEMENT_HELP", getLinkedText((ImprovementTypes)GC.getTraitInfo(eTrait).getManaFromImprovementType()).c_str(), szMana.GetCString()));
	}

	//	Improvement upgrade rate modifier
	if (GC.getTraitInfo(eTrait).getImprovementUpgradeRateModifier() != 0)
	{
		bFirst = true;
		CvWString szFirstBuffer;

		for (iI = 0; iI < GC.getNumImprovementInfos(); ++iI)
		{
			if (GC.getImprovementInfo((ImprovementTypes)iI).getImprovementUpgrade() != NO_IMPROVEMENT)
			{
				if (!GC.getImprovementInfo((ImprovementTypes)iI).isGraphicalOnly()
					&& (GC.getImprovementInfo((ImprovementTypes)iI).getPrereqCivilization() == NO_CIVILIZATION
						|| (GC.getGame().getActiveCivilizationType() == GC.getImprovementInfo((ImprovementTypes)iI).getPrereqCivilization()))) {
					szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_IMPROVEMENT_UPGRADE", GC.getTraitInfo(eTrait).getImprovementUpgradeRateModifier()).c_str());
					CvWString szImprovement;
					szImprovement = getLinkedText((ImprovementTypes)iI);
					setListHelp(szHelpString, szFirstBuffer, szImprovement, L", ", bFirst);
					bFirst = false;
				}
			}
		}
	}

	if (!bCivilopediaText)
	{	// Display trait cost
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER) {
			//check because PLAY NOW runs this code without an valid instance of getActivePlayer
			if (GC.getGame().getElapsedGameTurns() > 0) {
				CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
				if (kPlayer.canGetTrait(eTrait)) {
					szTempBuffer.Format(L"\n%d%c", kPlayer.getTraitCost(eTrait), GC.getCommerceInfo(COMMERCE_CULTURE).getChar());
				}
				szHelpString.append(szTempBuffer);
			}
		}
	}

	//	return szHelpString;
}


//
// parseLeaderTraits - SimpleCivPicker							// LOCALIZATION READY
//
void CvGameTextMgr::parseLeaderTraits(CvWStringBuffer& szHelpString, LeaderHeadTypes eLeader, CivilizationTypes eCivilization, bool bDawnOfMan, bool bCivilopediaText)
{
	PROFILE_FUNC();

	CvWString szTempBuffer;	// Formatting
	int iI;

	//	Build help string
	if (eLeader != NO_LEADER)
	{
		if (!bDawnOfMan && !bCivilopediaText)
		{
			//>>>>	BUGFfH: Modified by Denev 2009/10/02
			//			szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getLeaderHeadInfo(eLeader).getDescription());
			szTempBuffer.assign(getLinkedText(eLeader));
			//<<<<	BUGFfH: End Modify
			szHelpString.append(szTempBuffer);
		}

		FAssert((GC.getNumTraitInfos() > 0) &&
			"GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvSimpleCivPicker::setLeaderText");

		//FfH: Added by Kael 11/03/2007
		if (!bDawnOfMan)
		{
			szHelpString.append(NEWLINE);
		}
		szHelpString.append(gDLL->getText("TXT_KEY_ALIGNMENT", GC.getAlignmentInfo((AlignmentTypes)GC.getLeaderHeadInfo(eLeader).getAlignment()).getDescription()));
		if (eCivilization != NO_CIVILIZATION)
		{
			if (GC.getCivilizationInfo(eCivilization).getHero() != NO_UNIT)
			{
				szHelpString.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/05
				//				szHelpString.append(gDLL->getText("TXT_KEY_HERO", GC.getUnitInfo((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero()).getDescription()));
				szTempBuffer.Format(L"%s: ", gDLL->getText("TXT_KEY_HERO_PEDIA").c_str());
				if (bDawnOfMan)
				{
					szTempBuffer.append(GC.getUnitInfo((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero()).getDescription());
				}
				else
				{
					szTempBuffer.append(getLinkedText((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero(), false));
				}
				szHelpString.append(szTempBuffer);
				//FfH: End Modify
			}
			for (iI = 0; iI < GC.getNumSpellInfos(); ++iI)
			{
				if (GC.getSpellInfo((SpellTypes)iI).isGlobal())
				{
					if (GC.getSpellInfo((SpellTypes)iI).getCivilizationPrereq() == eCivilization)
					{
						szHelpString.append(NEWLINE);
						//>>>>	BUGFfH: Modified by Denev 2009/10/05
						//						szHelpString.append(gDLL->getText("TXT_KEY_GLOBAL_SPELL", GC.getSpellInfo((SpellTypes)iI).getDescription()));
						szTempBuffer.Format(L"%s: ", gDLL->getText("TXT_KEY_GLOBAL_SPELL_PEDIA").c_str());
						if (bDawnOfMan)
						{
							szTempBuffer.append(GC.getSpellInfo((SpellTypes)iI).getDescription());
						}
						else
						{
							szTempBuffer.append(getLinkedText((SpellTypes)iI, false));
						}
						szHelpString.append(szTempBuffer);
						//FfH: End Modify
					}
				}
			}
			if (bDawnOfMan)
			{
				szHelpString.append(NEWLINE);
			}
		}
		//FfH: End Add

		bool bFirst = true;
		//>>>>	BUGFfH: Added by Denev 2009/10/05
		/*	Does the leader have a adaptive or insane?	*/
		bool bLeaderAdaptive = false;
		bool bLeaderInsane = false;
		for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); iTrait++)
		{
			if (GC.getLeaderHeadInfo(eLeader).hasTrait(iTrait))
			{
				if (GC.getTraitInfo((TraitTypes)iTrait).isAdaptive())
				{
					bLeaderAdaptive = true;
				}
				if (GC.getTraitInfo((TraitTypes)iTrait).isInsane())
				{
					bLeaderInsane = true;
				}
			}
		}
		//<<<<	BUGFfH: End Add
		for (iI = 0; iI < GC.getNumTraitInfos(); ++iI)
		{
			if (GC.getLeaderHeadInfo(eLeader).hasTrait(iI))
			{
				//>>>>	BUGFfH: Modified by Denev 2009/10/05
				/*
								if (!bFirst)
								{
									if (bDawnOfMan)
									{
										szHelpString.append(L", ");
									}
								}
								else
								{
									bFirst = false;
								}
								parseTraits(szHelpString, ((TraitTypes)iI), eCivilization, bDawnOfMan);
				*/
				if (bDawnOfMan)
				{
					if (!bFirst)
					{
						szHelpString.append(L", ");
					}
					bFirst = false;
					szHelpString.append(GC.getTraitInfo((TraitTypes)iI).getDescription());
				}
				else
				{
					//if the leader has adaptive or insane, shows temporary trait.
					bool bTemporary = false;
					if (GC.getTraitInfo((TraitTypes)iI).isSelectable())
					{
						if (bLeaderAdaptive)
						{
							if (iI != GC.getLeaderHeadInfo(eLeader).getPermanentTrait())
							{
								bTemporary = true;
							}
						}
						if (bLeaderInsane)
						{
							bTemporary = true;
						}
					}
					szHelpString.append(NEWLINE);
					parseTraits(szHelpString, (TraitTypes)iI, eCivilization, bTemporary);
				}
				//<<<<	BUGFfH: End Modify
			}
		}
	}
	else
	{
		//	Random leader
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_TRAIT_PLAYER_UNKNOWN").c_str());
		szHelpString.append(szTempBuffer);
	}

	//	return szHelpString;
}

//FfH: Added by Kael 08/08/2007
void CvGameTextMgr::parseLeaderTraits(CvWStringBuffer& szHelpString, PlayerTypes ePlayer)
{
	//	CvWString szHelpString;		// Final String Storage
	wchar szTempBuffer[1024];	// Formatting
	int iI;
	LeaderHeadTypes eLeader;
	CivilizationTypes eCivilization;

	//	Build help string
	if (GET_PLAYER(ePlayer).getLeaderType() != NO_LEADER)
	{
		eLeader = GET_PLAYER(ePlayer).getLeaderType();
		eCivilization = GET_PLAYER(ePlayer).getCivilizationType();

		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		swprintf(szTempBuffer,  SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getLeaderHeadInfo(eLeader).getDescription());
		swprintf(szTempBuffer, getLinkedText(eLeader).c_str());
		//<<<<	BUGFfH: End Modify
		szHelpString.append(szTempBuffer);

		FAssert((GC.getNumTraitInfos() > 0) &&
			"GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvSimpleCivPicker::setLeaderText");

		for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
		{
			if (GET_PLAYER(ePlayer).hasTrait((TraitTypes)iI))
			{
				//>>>>	BUGFfH: Modified by Denev 2009/09/27
				//				parseTraits(szHelpString, ((TraitTypes)iI), eCivilization, false);
								//if the leader has adaptive or insane, shows temporary trait.
				bool bTemporary = false;
				if (GC.getTraitInfo((TraitTypes)iI).isSelectable())
				{
					if (GET_PLAYER(ePlayer).isAdaptive())
					{
						if (iI != GC.getLeaderHeadInfo(eLeader).getPermanentTrait())
						{
							bTemporary = true;
						}
					}
					if (GET_PLAYER(ePlayer).isInsane())
					{
						bTemporary = true;
					}
				}
				szHelpString.append(NEWLINE);
				parseTraits(szHelpString, (TraitTypes)iI, eCivilization, bTemporary);
				//<<<<	BUGFfH: End Modify
			}
		}
	}
	else
	{
		//	Random leader
		swprintf(szTempBuffer, SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_TRAIT_PLAYER_UNKNOWN").c_str());
		szHelpString.append(szTempBuffer);
	}

	//	return szHelpString;
}
//FfH: End Add

void CvGameTextMgr::parseCivilizationAbilities(CvWStringBuffer& szHelpString, PlayerTypes ePlayer)
{
	//	wchar szTempBuffer[1024];	// Formatting
	CivilizationTypes eCivilization;

	//	Build help string
	if (ePlayer != NO_PLAYER)
	{
		eCivilization = GET_PLAYER(ePlayer).getCivilizationType();

		CivilizationAbilititesHelp(szHelpString, eCivilization);
	}
}

void CvGameTextMgr::CivilizationAbilititesHelp(CvWStringBuffer& szHelpString, CivilizationTypes eCivilization)
{
	//	wchar szTempBuffer[1024];	// Formatting
	int iI;
	bool bFirst;

	//	Build help string
	if (eCivilization != NO_CIVILIZATION)
	{
		CvCivilizationInfo& kCiv = GC.getCivilizationInfo(eCivilization);

		bFirst = true;
		for (iI = 0; iI < GC.getNumFeatureInfos(); iI++) {
			if (kCiv.isMaintainFeatures(iI)) {
				if (bFirst) {
					bFirst = false;
					szHelpString.append(gDLL->getText("TXT_KEY_CIV_ABILITIES_MAINTAIN_FEATURES"));
					szHelpString.append(L" ");
					szHelpString.append(getLinkedText((FeatureTypes)iI));
				}
				else {
					szHelpString.append(L", ");
					szHelpString.append(getLinkedText((FeatureTypes)iI));
				}
			}
		}

		bFirst = true;
		if (kCiv.isSpellResearchFromMaintainFeatures()) {
			for (iI = 0; iI < GC.getNumFeatureInfos(); iI++) {
				if (kCiv.isMaintainFeatures(iI)) {
					if (bFirst) {
						bFirst = false;
						szHelpString.append(NEWLINE);
						szHelpString.append(gDLL->getText("TXT_KEY_CIV_ABILITIES_SPELLRESEARCH_FEATURES"));
						szHelpString.append(L" ");
						szHelpString.append(getLinkedText((FeatureTypes)iI));
					}
					else {
						szHelpString.append(L", ");
						szHelpString.append(getLinkedText((FeatureTypes)iI));
					}
				}
			}
		}

		if (kCiv.getReducedManaCostInGoldenAge() != 0) {
			szHelpString.append(NEWLINE);
			szHelpString.append(gDLL->getText("TXT_KEY_CIV_ABILITIES_REDUCED_SPELLCOST_IN_GA", kCiv.getReducedManaCostInGoldenAge()));
		}
	}
	//	return szHelpString;
}

//
// parseLeaderTraits - SimpleCivPicker							// LOCALIZATION READY
//
void CvGameTextMgr::parseLeaderShortTraits(CvWStringBuffer& szHelpString, LeaderHeadTypes eLeader)
{
	PROFILE_FUNC();

	int iI;

	//	Build help string
	if (eLeader != NO_LEADER)
	{
		FAssert((GC.getNumTraitInfos() > 0) &&
			"GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvSimpleCivPicker::setLeaderText");

		bool bFirst = true;
		for (iI = 0; iI < GC.getNumTraitInfos(); ++iI)
		{
			if (GC.getLeaderHeadInfo(eLeader).hasTrait(iI))
			{
				if (!bFirst)
				{
					szHelpString.append(L"/");
				}
				szHelpString.append(gDLL->getText(GC.getTraitInfo((TraitTypes)iI).getShortDescription()));
				bFirst = false;
			}
		}
	}
	else
	{
		//	Random leader
		szHelpString.append(CvWString("???/???"));
	}

	//	return szHelpString;
}

//
// Build Civilization Info Help Text
//
void CvGameTextMgr::parseCivInfos(CvWStringBuffer& szInfoText, CivilizationTypes eCivilization, bool bDawnOfMan, bool bLinks)
{
	PROFILE_FUNC();

	CvWString szBuffer;
	CvWString szTempString;
	CvWString szText;
	UnitTypes eDefaultUnit;
	UnitTypes eUniqueUnit;
	BuildingTypes eDefaultBuilding;
	BuildingTypes eUniqueBuilding;

	if (eCivilization != NO_CIVILIZATION)
	{
		if (!bDawnOfMan)
		{
			// Civ Name
//>>>>	BUGFfH: Modified by Denev 2009/10/02
//			szBuffer.Format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivilizationInfo(eCivilization).getDescription());
			szBuffer.assign(getLinkedText(eCivilization));
			//<<<<	BUGFfH: End Modify
			szInfoText.append(szBuffer);

			// Free Techs
			szBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_FREE_TECHS").GetCString());
			szInfoText.append(szBuffer);

			bool bFound = false;
			for (int iI = 0; iI < GC.getNumTechInfos(); ++iI)
			{
				if (GC.getCivilizationInfo(eCivilization).isCivilizationFreeTechs(iI))
				{
					bFound = true;
					// Add Tech
//>>>>	BUGFfH: Modified by Denev 2009/09/13
//					szText.Format((bLinks ? L"<link=literal>%s</link>" : L"%s"), GC.getTechInfo((TechTypes)iI).getDescription());
					szText.assign(bLinks ? getLinkedText((TechTypes)iI, false) : GC.getTechInfo((TechTypes)iI).getDescription());
					//<<<<	BUGFfH: End Modify
					szBuffer.Format(L"%s  %c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szText.GetCString());
					szInfoText.append(szBuffer);
				}
			}

			if (!bFound)
			{
				szBuffer.Format(L"%s  %s", NEWLINE, gDLL->getText("TXT_KEY_FREE_TECHS_NO").GetCString());
				szInfoText.append(szBuffer);
			}
		}

		//FfH: Added by Kael 09/02/2008
				//Civ Trait
		if (GC.getCivilizationInfo(eCivilization).getCivTrait() != NO_TRAIT)
		{
			szText = gDLL->getText("TXT_KEY_CIV_TRAIT");
			if (bDawnOfMan)
			{
				szTempString.Format(L"%s:", szText.GetCString());
				szInfoText.append(szTempString);
				//>>>>	BUGFfH: Modified by Denev 2009/09/29
				//				szBuffer.Format(L" %s\n", GC.getTraitInfo((TraitTypes)GC.getCivilizationInfo(eCivilization).getCivTrait()).getDescription());
				szBuffer.Format(L" %s%s", getLinkedText((TraitTypes)GC.getCivilizationInfo(eCivilization).getCivTrait(), false).c_str(), NEWLINE);
				//<<<<	BUGFfH: End Modify
				szInfoText.append(szBuffer);
			}
			else
			{
				szBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_CIV_TRAIT").GetCString());
				szInfoText.append(szBuffer);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szText.Format((bLinks ? L"%s" : L"%s"), GC.getTraitInfo((TraitTypes)GC.getCivilizationInfo(eCivilization).getCivTrait()).getDescription());
				szText.assign(bLinks ? getLinkedText((TraitTypes)GC.getCivilizationInfo(eCivilization).getCivTrait(), false) : GC.getTraitInfo((TraitTypes)GC.getCivilizationInfo(eCivilization).getCivTrait()).getDescription());
				//<<<<	BUGFfH: End Modify
				szBuffer.Format(L"%s  %c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szText.GetCString());
				szInfoText.append(szBuffer);
			}
		}

		//Civ Hero
		if (GC.getCivilizationInfo(eCivilization).getHero() != NO_UNIT)
		{
			szText = gDLL->getText("TXT_KEY_HERO_PEDIA");
			if (bDawnOfMan)
			{
				szTempString.Format(L"%s:", szText.GetCString());
				szInfoText.append(szTempString);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szBuffer.Format(L" <link=literal>%s</link>\n", GC.getUnitInfo((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero()).getDescription());
				szBuffer.Format(L" %s%s", getLinkedText((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero()).c_str(), NEWLINE);
				//<<<<	BUGFfH: End Modify
				szInfoText.append(szBuffer);
			}
			else
			{
				szBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_HERO_PEDIA").GetCString());
				szInfoText.append(szBuffer);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szText.Format((bLinks ? L"<link=literal>%s</link>" : L"%s"), GC.getUnitInfo((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero()).getDescription());
				szText.assign(bLinks ? getLinkedText((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero(), false) : GC.getUnitInfo((UnitTypes)GC.getCivilizationInfo(eCivilization).getHero()).getDescription());
				//<<<<	BUGFfH: End Modify
				szBuffer.Format(L"%s  %c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szText.GetCString());
				szInfoText.append(szBuffer);
			}
		}

		//World Spell
		szText = gDLL->getText("TXT_KEY_GLOBAL_SPELL_PEDIA");
		if (bDawnOfMan)
		{
			szTempString.Format(L"%s:", szText.GetCString());
			szInfoText.append(szTempString);
			for (int iI = 0; iI < GC.getNumSpellInfos(); ++iI)
			{
				if (GC.getSpellInfo((SpellTypes)iI).isGlobal())
				{
					if (GC.getSpellInfo((SpellTypes)iI).getCivilizationPrereq() == eCivilization)
					{
						//>>>>	BUGFfH: Modified by Denev 2009/09/13
						//						szBuffer.Format(L" <link=literal>%s</link>\n", GC.getSpellInfo((SpellTypes)iI).getDescription());
						szBuffer.Format(L" %s%s", getLinkedText((SpellTypes)iI).c_str(), NEWLINE);
						//<<<<	BUGFfH: End Modify
						szInfoText.append(szBuffer);
					}
				}
			}
		}
		else
		{
			szBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_GLOBAL_SPELL_PEDIA").GetCString());
			szInfoText.append(szBuffer);
			for (int iI = 0; iI < GC.getNumSpellInfos(); ++iI)
			{
				if (GC.getSpellInfo((SpellTypes)iI).isGlobal())
				{
					if (GC.getSpellInfo((SpellTypes)iI).getCivilizationPrereq() == eCivilization)
					{
						//>>>>	BUGFfH: Modified by Denev 2009/09/13
						//						szText.Format((bLinks ? L"<link=literal>%s</link>" : L"%s"), GC.getSpellInfo((SpellTypes)iI).getDescription());
						szText.assign(bLinks ? getLinkedText((SpellTypes)iI, false) : GC.getSpellInfo((SpellTypes)iI).getDescription());
						//<<<<	BUGFfH: End Modify
						szBuffer.Format(L"%s  %c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szText.GetCString());
						szInfoText.append(szBuffer);
					}
				}
			}
		}

		//Civ Terrain Modifiers
		bool bFirst = true;
		for (int iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
		{
			if (GC.getTerrainInfo((TerrainTypes)iI).getCivilizationYieldType() == eCivilization)
			{
				for (int iJ = 0; iJ < NUM_YIELD_TYPES; ++iJ)
				{
					if (GC.getTerrainInfo((TerrainTypes)iI).getCivilizationYieldChange(YieldTypes(iJ)) != 0)
					{
						if (bFirst)
						{
							szText = gDLL->getText("TXT_KEY_MISC_CIV_TERRAIN");
							if (bDawnOfMan)
							{
								szTempString.Format(L"%s:", szText.GetCString());
								szInfoText.append(szTempString);
							}
							else
							{
								szBuffer.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), gDLL->getText("TXT_KEY_MISC_CIV_TERRAIN").GetCString());
								szInfoText.append(szBuffer);
							}
							bFirst = false;
						}
						//>>>>	BUGFfH: Modified by Denev 2009/10/05
						//						szText = gDLL->getText("TXT_KEY_MISC_CIV_TERRAIN_MOD", GC.getTerrainInfo((TerrainTypes)iI).getCivilizationYieldChange(YieldTypes(iJ)), GC.getYieldInfo((YieldTypes) iJ).getChar(), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide());
						szText = gDLL->getText("TXT_KEY_MISC_CIV_TERRAIN_MOD", GC.getTerrainInfo((TerrainTypes)iI).getCivilizationYieldChange(YieldTypes(iJ)), GC.getYieldInfo((YieldTypes)iJ).getChar(), getLinkedText((TerrainTypes)iI).c_str());
						//<<<<	BUGFfH: End Modify
						if (bDawnOfMan)
						{
							szBuffer.Format(L" %s\n", szText.GetCString());
							szInfoText.append(szBuffer);
						}
						else
						{
							szBuffer.Format(L"%s  %c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szText.GetCString());
							szInfoText.append(szBuffer);
						}
					}
				}
			}
		}
		//FfH: End Add

				// Free Units
		szText = gDLL->getText("TXT_KEY_FREE_UNITS");
		if (bDawnOfMan)
		{
			szTempString.Format(L"%s: ", szText.GetCString());
		}
		else
		{
			szTempString.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
		}
		szInfoText.append(szTempString);

		bool bFound = false;
		for (int iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
		{
			eDefaultUnit = ((UnitTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationUnits(iI)));
			eUniqueUnit = ((UnitTypes)(GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()));
			if ((eDefaultUnit != NO_UNIT) && (eUniqueUnit != NO_UNIT))
			{
				if (eDefaultUnit != eUniqueUnit

					//FfH: Added by Kael 08/27/2009
					|| (!isWorldUnitClass((UnitClassTypes)iI)
						&& GC.getUnitInfo(eUniqueUnit).getPrereqCiv() == eCivilization)
					//FfH: End Add

					)
				{
					// Add Unit
					if (bDawnOfMan)
					{
						if (bFound)
						{
							szInfoText.append(L", ");
						}

						//FfH: Modified by Kael 08/27/2009
						//						szBuffer.Format((bLinks ? L"<link=literal>%s</link> - (<link=literal>%s</link>)" : L"%s - (%s)"),
						//							GC.getUnitInfo(eDefaultUnit).getDescription(),
						//							GC.getUnitInfo(eUniqueUnit).getDescription());
						if (eDefaultUnit != eUniqueUnit)
						{
							//>>>>	BUGFfH: Modified by Denev 2009/09/13
							//							szBuffer.Format((bLinks ? L"<link=literal>%s</link> - (<link=literal>%s</link>)" : L"%s - (%s)"),
							//								GC.getUnitInfo(eDefaultUnit).getDescription(),
							//								GC.getUnitInfo(eUniqueUnit).getDescription());
							szBuffer.Format(L"%s",
								bLinks ? getLinkedText(eDefaultUnit).c_str() : GC.getUnitInfo(eDefaultUnit).getDescription());
							//<<<<	BUGFfH: End Modify
						}
						else
						{
							//>>>>	BUGFfH: Modified by Denev 2009/09/13
							//							szBuffer.Format((bLinks ? L"<link=literal>%s</link>" : L"%s"),
							//								GC.getUnitInfo(eDefaultUnit).getDescription());
							szBuffer.assign(bLinks ? getLinkedText(eDefaultUnit) : GC.getUnitInfo(eDefaultUnit).getDescription());
							//<<<<	BUGFfH: End Modify
						}
						//FfH: End Modify

					}
					else
					{

						//FfH: Modified by Kael 08/27/2009
						//						szBuffer.Format(L"\n  %c%s - (%s)", gDLL->getSymbolID(BULLET_CHAR),
						//							GC.getUnitInfo(eDefaultUnit).getDescription(),
						//							GC.getUnitInfo(eUniqueUnit).getDescription());
						if (eDefaultUnit != eUniqueUnit)
						{
							//>>>>	BUGFfH: Modified by Denev 2009/10/02
							/*
														szBuffer.Format(L"\n  %c%s - (%s)", gDLL->getSymbolID(BULLET_CHAR),
															GC.getUnitInfo(eDefaultUnit).getDescription(),
															GC.getUnitInfo(eUniqueUnit).getDescription());
							*/
							szBuffer.Format(L"%s  %c%s", NEWLINE,
								gDLL->getSymbolID(BULLET_CHAR),
								getLinkedText(eDefaultUnit, true).c_str());
							//<<<<	BUGFfH: End Modify
						}
						else
						{
							//>>>>	BUGFfH: Modified by Denev 2009/10/02
							/*
														szBuffer.Format(L"\n  %c%s", gDLL->getSymbolID(BULLET_CHAR),
															GC.getUnitInfo(eDefaultUnit).getDescription());
							*/
							szBuffer.Format(L"%s  %c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR));
							szBuffer.append(getLinkedText(eDefaultUnit));
							//<<<<	BUGFfH: End Modify
						}
						//FfH: End Modify

					}
					szInfoText.append(szBuffer);
					bFound = true;
				}
			}
		}

		if (!bFound)
		{
			szText = gDLL->getText("TXT_KEY_FREE_UNITS_NO");
			if (bDawnOfMan)
			{
				szTempString.Format(L"%s", szText.GetCString());
			}
			else
			{
				szTempString.Format(L"%s  %s", NEWLINE, szText.GetCString());
			}
			szInfoText.append(szTempString);
			bFound = true;
		}

		//FfH: Added by Kael 08/27/2009
				// Blocked Units
		szText = gDLL->getText("TXT_KEY_MISC_CIV_BLOCKED_UNITS");
		if (bDawnOfMan)
		{
			//>>>>	BUGFfH: Added by Denev 2009/09/11
			if (bFound)
			{
				szInfoText.append(NEWLINE);
			}
			//<<<<	BUGFfH: End Add
			szTempString.Format(L"%s: ", szText.GetCString());
		}
		else
		{
			szTempString.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
		}
		szInfoText.append(szTempString);

		bFound = false;
		for (int iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
		{
			eUniqueUnit = ((UnitTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationUnits(iI)));
			eDefaultUnit = ((UnitTypes)(GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex()));
			if (eDefaultUnit != NO_UNIT
				&& eUniqueUnit == NO_UNIT
				&& !isWorldUnitClass((UnitClassTypes)iI)
				&& GC.getUnitInfo(eDefaultUnit).getPrereqCiv() == NO_CIVILIZATION)
			{
				if (bDawnOfMan)
				{
					if (bFound)
					{
						szInfoText.append(L", ");
					}
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.Format((bLinks ? L"<link=literal>%s</link> - (<link=literal>%s</link>)" : L"%s"),
					//						GC.getUnitInfo(eDefaultUnit).getDescription());
					szBuffer.assign(bLinks ? getLinkedText(eDefaultUnit) : GC.getUnitInfo(eDefaultUnit).getDescription());
					//<<<<	BUGFfH: End Modify
				}
				else
				{
					//>>>>	BUGFfH: Modified by Denev 2009/10/02
					//					szBuffer.Format(L"\n  %c%s", gDLL->getSymbolID(BULLET_CHAR),
					//						GC.getUnitInfo(eDefaultUnit).getDescription());
					szBuffer.Format(L"%s  %c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR));
					szBuffer.append(getLinkedText(eDefaultUnit, false));
					//<<<<	BUGFfH: End Modify
				}
				szInfoText.append(szBuffer);
				bFound = true;
			}
		}

		if (!bFound)
		{
			szText = gDLL->getText("TXT_KEY_MISC_CIV_BLOCKED_UNITS_NO");
			if (bDawnOfMan)
			{
				szTempString.Format(L"%s", szText.GetCString());
			}
			else
			{
				szTempString.Format(L"%s  %s", NEWLINE, szText.GetCString());
			}
			szInfoText.append(szTempString);
			bFound = true;
		}
		//FfH: End Add

				// Free Buildings
		szText = gDLL->getText("TXT_KEY_UNIQUE_BUILDINGS");
		if (bDawnOfMan)
		{
			if (bFound)
			{
				szInfoText.append(NEWLINE);
			}
			szTempString.Format(L"%s: ", szText.GetCString());
		}
		else
		{
			szTempString.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
		}
		szInfoText.append(szTempString);

		bFound = false;
		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
		{
			eDefaultBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
			eUniqueBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()));

			//FfH: Modified by Kael 08/27/2009
			//			if ((eDefaultBuilding != NO_BUILDING) && (eUniqueBuilding != NO_BUILDING))
			if (eDefaultBuilding != NO_BUILDING
				&& (eUniqueBuilding != NO_BUILDING
					|| GC.getBuildingInfo(eDefaultBuilding).getPrereqCiv() == eCivilization))
				//FfH: End Modify

			{
				if (eDefaultBuilding != eUniqueBuilding)
				{
					// Add Building
					if (bDawnOfMan)
					{
						if (bFound)
						{
							szInfoText.append(L", ");
						}

						//FfH: Modified by Kael 08/27/2009
						//						szBuffer.Format((bLinks ? L"<link=literal>%s</link> - (<link=literal>%s</link>)" : L"%s - (%s)"),
						//							GC.getBuildingInfo(eDefaultBuilding).getDescription(),
						//							GC.getBuildingInfo(eUniqueBuilding).getDescription());
						if (GC.getBuildingInfo(eDefaultBuilding).getPrereqCiv() != eCivilization)
						{
							//>>>>	BUGFfH: Modified by Denev 2009/09/13
							//							szBuffer.Format((bLinks ? L"<link=literal>%s</link> - (<link=literal>%s</link>)" : L"%s - (%s)"),
							//								GC.getBuildingInfo(eDefaultBuilding).getDescription(),
							//								GC.getBuildingInfo(eUniqueBuilding).getDescription());
							szBuffer.Format(L"%s - (%s)",
								(bLinks ? getLinkedText(eDefaultBuilding).c_str() : GC.getBuildingInfo(eDefaultBuilding).getDescription()),
								(bLinks ? getLinkedText(eUniqueBuilding).c_str() : GC.getBuildingInfo(eUniqueBuilding).getDescription()));
							//<<<<	BUGFfH: End Modify
						}
						else
						{
							//>>>>	BUGFfH: Modified by Denev 2009/09/13
							//							szBuffer.Format((bLinks ? L"<link=literal>%s</link>" : L"%s"),
							//								GC.getBuildingInfo(eDefaultBuilding).getDescription());
							szBuffer.assign(bLinks ? getLinkedText(eDefaultBuilding) : GC.getBuildingInfo(eDefaultBuilding).getDescription());
							//<<<<	BUGFfH: End Modify
						}
						//FfH: End Modify

					}
					else
					{

						//FfH: Modified by Kael 08/27/2009
						//						szBuffer.Format(L"\n  %c%s - (%s)", gDLL->getSymbolID(BULLET_CHAR),
						//							GC.getBuildingInfo(eDefaultBuilding).getDescription(),
						//							GC.getBuildingInfo(eUniqueBuilding).getDescription());
						if (GC.getBuildingInfo(eDefaultBuilding).getPrereqCiv() != eCivilization)
						{
							//>>>>	BUGFfH: Modified by Denev 2009/10/02
							/*
														szBuffer.Format(L"\n  %c%s - (%s)", gDLL->getSymbolID(BULLET_CHAR),
															GC.getBuildingInfo(eDefaultBuilding).getDescription(),
															GC.getBuildingInfo(eUniqueBuilding).getDescription());
							*/
							szBuffer.Format(L"%s  %c%s - (%s)", NEWLINE,
								gDLL->getSymbolID(BULLET_CHAR),
								getLinkedText(eDefaultBuilding, true).c_str(),
								getLinkedText(eUniqueBuilding, false).c_str());
							//<<<<	BUGFfH: End Modify
						}
						else
						{
							//>>>>	BUGFfH: Modified by Denev 2009/10/02
							/*
														szBuffer.Format(L"\n  %c%s", gDLL->getSymbolID(BULLET_CHAR),
															GC.getBuildingInfo(eDefaultBuilding).getDescription());
							*/
							szBuffer.Format(L"%s  %c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR));
							szBuffer.append(getLinkedText(eDefaultBuilding));
							//<<<<	BUGFfH: End Modify
						}
						//FfH: End Modify

					}
					szInfoText.append(szBuffer);
					bFound = true;
				}
			}
		}
		if (!bFound)
		{
			szText = gDLL->getText("TXT_KEY_UNIQUE_BUILDINGS_NO");
			if (bDawnOfMan)
			{
				szTempString.Format(L"%s", szText.GetCString());
			}
			else
			{
				szTempString.Format(L"%s  %s", NEWLINE, szText.GetCString());
			}
			szInfoText.append(szTempString);
		}

		//FfH: Added by Kael 08/27/2009
				// Blocked Buildings
		szText = gDLL->getText("TXT_KEY_MISC_CIV_BLOCKED_BUILDINGS");
		if (bDawnOfMan)
		{
			if (bFound)
			{
				szInfoText.append(NEWLINE);
			}
			szTempString.Format(L"%s: ", szText.GetCString());
		}
		else
		{
			szTempString.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
		}
		szInfoText.append(szTempString);
		bFound = false;
		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
		{
			eDefaultBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(iI)));
			eUniqueBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex()));
			if ((eDefaultBuilding == NO_BUILDING) && (eUniqueBuilding != NO_BUILDING))
			{
				if (eDefaultBuilding != eUniqueBuilding)
				{
					// Add Building
					if (bDawnOfMan)
					{
						if (bFound)
						{
							szInfoText.append(L", ");
						}
						//>>>>	BUGFfH: Modified by Denev 2009/09/13
						//						szBuffer.Format((bLinks ? L"<link=literal>%s</link>" : L"%s"),
						//							GC.getBuildingInfo(eUniqueBuilding).getDescription());
						szBuffer.assign(bLinks ? getLinkedText(eUniqueBuilding) : GC.getBuildingInfo(eUniqueBuilding).getDescription());
						//<<<<	BUGFfH: End Modify
					}
					else
					{
						//>>>>	BUGFfH: Modified by Denev 2009/10/02
						//						szBuffer.Format(L"\n  %c%s", gDLL->getSymbolID(BULLET_CHAR),
						//							GC.getBuildingInfo(eUniqueBuilding).getDescription());
						szBuffer.Format(L"%s  %c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR));
						szBuffer.append(getLinkedText(eUniqueBuilding, false));
						//<<<<	BUGFfH: End Modify
					}
					szInfoText.append(szBuffer);
					bFound = true;
				}
			}
		}
		if (!bFound)
		{
			szText = gDLL->getText("TXT_KEY_MISC_CIV_BLOCKED_BUILDINGS_NO");
			if (bDawnOfMan)
			{
				szTempString.Format(L"%s", szText.GetCString());
			}
			else
			{
				szTempString.Format(L"%s  %s", NEWLINE, szText.GetCString());
			}
			szInfoText.append(szTempString);
		}

		//Special Abilities
		if (!CvWString(GC.getCivilizationInfo(eCivilization).getHelp()).empty())
		{
			szText = gDLL->getText("TXT_KEY_MISC_CIV_SPECIAL_ABILITIES");
			if (bDawnOfMan)
			{
				szInfoText.append(NEWLINE);
				szTempString.Format(L"%s: ", szText.GetCString());
			}
			else
			{
				szTempString.Format(NEWLINE SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_ALT_HIGHLIGHT_TEXT"), szText.GetCString());
			}
			szInfoText.append(szTempString);
			if (bDawnOfMan)
			{
				szBuffer.Format((bLinks ? L"%s" : L"%s"),
					GC.getCivilizationInfo(eCivilization).getHelp());
			}
			else
			{
				szBuffer.Format(L"\n  %c%s", gDLL->getSymbolID(BULLET_CHAR),
					GC.getCivilizationInfo(eCivilization).getHelp());
			}
			szInfoText.append(szBuffer);
		}
		//FfH: End Add

	}
	else
	{
		//	This is a random civ, let us know here...
		szInfoText.append(gDLL->getText("TXT_KEY_CIV_UNKNOWN"));
	}

	//	return szInfoText;
}



void CvGameTextMgr::parseSpecialistHelp(CvWStringBuffer& szHelpString, SpecialistTypes eSpecialist, CvCity* pCity, bool bCivilopediaText)
{
	PROFILE_FUNC();

	CvWString szText;
	int aiYields[NUM_YIELD_TYPES];
	int aiCommerces[NUM_COMMERCE_TYPES];
	int iI;

	if (eSpecialist != NO_SPECIALIST)
	{
		if (!bCivilopediaText)
		{
			//>>>>	BUGFfH: Modified by Denev 2009/10/03
			//			szHelpString.append(GC.getSpecialistInfo(eSpecialist).getDescription());
			szHelpString.append(getLinkedText(eSpecialist));
			//<<<<	BUGFfH: End Modify

		}

		for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
		{
			if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER)
			{
				aiYields[iI] = GC.getSpecialistInfo(eSpecialist).getYieldChange(iI);
			}
			else
			{
				aiYields[iI] = GET_PLAYER((pCity != NULL) ? pCity->getOwnerINLINE() : GC.getGameINLINE().getActivePlayer()).specialistYield(eSpecialist, ((YieldTypes)iI));
			}
		}

		setYieldChangeHelp(szHelpString, L"", L"", L"", aiYields);

		for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
		{
			if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER)
			{
				aiCommerces[iI] = GC.getSpecialistInfo(eSpecialist).getCommerceChange(iI);
			}
			else
			{
				aiCommerces[iI] = GET_PLAYER((pCity != NULL) ? pCity->getOwnerINLINE() : GC.getGameINLINE().getActivePlayer()).specialistCommerce(((SpecialistTypes)eSpecialist), ((CommerceTypes)iI));
			}
		}

		setCommerceChangeHelp(szHelpString, L"", L"", L"", aiCommerces);

		if (GC.getSpecialistInfo(eSpecialist).getExperience() > 0)
		{
			szHelpString.append(NEWLINE);
			szHelpString.append(gDLL->getText("TXT_KEY_SPECIALIST_EXPERIENCE", GC.getSpecialistInfo(eSpecialist).getExperience()));
		}

		if (GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange() != 0)
		{
			szHelpString.append(NEWLINE);
			szHelpString.append(gDLL->getText("TXT_KEY_SPECIALIST_BIRTH_RATE", GC.getSpecialistInfo(eSpecialist).getGreatPeopleRateChange()));
		}

		if (!CvWString(GC.getSpecialistInfo(eSpecialist).getHelp()).empty() && !bCivilopediaText)
		{
			szHelpString.append(NEWLINE);
			szHelpString.append(GC.getSpecialistInfo(eSpecialist).getHelp());
		}
	}
}

void CvGameTextMgr::parseFreeSpecialistHelp(CvWStringBuffer& szHelpString, const CvCity& kCity)
{
	PROFILE_FUNC();

	for (int iLoopSpecialist = 0; iLoopSpecialist < GC.getNumSpecialistInfos(); iLoopSpecialist++)
	{
		SpecialistTypes eSpecialist = (SpecialistTypes)iLoopSpecialist;
		int iNumSpecialists = kCity.getFreeSpecialistCount(eSpecialist);

		if (iNumSpecialists > 0)
		{
			int aiYields[NUM_YIELD_TYPES];
			int aiCommerces[NUM_COMMERCE_TYPES];

			szHelpString.append(NEWLINE);
			szHelpString.append(CvWString::format(L"%s (%d): ", GC.getSpecialistInfo(eSpecialist).getDescription(), iNumSpecialists));

			for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
			{
				aiYields[iI] = iNumSpecialists * GET_PLAYER(kCity.getOwnerINLINE()).specialistYield(eSpecialist, ((YieldTypes)iI));
			}

			CvWStringBuffer szYield;
			setYieldChangeHelp(szYield, L"", L"", L"", aiYields, false, false);
			szHelpString.append(szYield);

			for (int iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
			{
				aiCommerces[iI] = iNumSpecialists * GET_PLAYER(kCity.getOwnerINLINE()).specialistCommerce(eSpecialist, ((CommerceTypes)iI));
			}

			CvWStringBuffer szCommerceString;
			setCommerceChangeHelp(szCommerceString, L"", L"", L"", aiCommerces, false, false);
			if (!szYield.isEmpty() && !szCommerceString.isEmpty())
			{
				szHelpString.append(L", ");
			}
			szHelpString.append(szCommerceString);

			if (GC.getSpecialistInfo(eSpecialist).getExperience() > 0)
			{
				if (!szYield.isEmpty() || !szYield.isEmpty())
				{
					szHelpString.append(L", ");
				}
				szHelpString.append(gDLL->getText("TXT_KEY_SPECIALIST_EXPERIENCE_SHORT", iNumSpecialists * GC.getSpecialistInfo(eSpecialist).getExperience()));
			}
		}
	}
}


//
// Promotion Help
//
void CvGameTextMgr::parsePromotionHelp(CvWStringBuffer& szBuffer, PromotionTypes ePromotion, const wchar* pcNewline)
{
	PROFILE_FUNC();

	CvWString szText, szText2;
	CvWString szTempBuffer;
	int iI;
	bool bFirst;
	CvPromotionInfo& kPromotion = GC.getPromotionInfo(ePromotion);
	PlayerTypes eActivePlayer = GC.getGameINLINE().getActivePlayer();
	CvUnit* pSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (NO_PROMOTION == ePromotion)
	{
		return;
	}
/*
	if (kPromotion.getGoldCost() > 0)
	{
		szBuffer.append(pcNewline);

		CvWString szTempBuffer;
		szTempBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_GOLD_COST", GC.getPromotionInfo(ePromotion).getGoldCost()));
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER && GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getGold() < GC.getPromotionInfo(ePromotion).getGoldCost() && gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL)
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), szTempBuffer.c_str());
		szBuffer.append(szTempBuffer);
	}
*/
	if (kPromotion.getGoldCost() > 0)
	{
		bool bPopup = gDLL->getInterfaceIFace()->isPopupUp();
		int iGold = GC.getPromotionInfo(ePromotion).getGoldCost();
		if(NO_PLAYER != eActivePlayer){
			iGold = (iGold * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		}
		if (bPopup)
		{
			iGold *= std::max(50, 100 - (GC.getGameINLINE().getActivePlayer() == NO_PLAYER ? 0 : GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getReducedEquipmentCost()));
			iGold /= 100;
		}
		szBuffer.append(pcNewline);
		CvWString szTempBuffer;
		szTempBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_GOLD_COST", iGold));
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER && GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getGold() < iGold && gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL)
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), szTempBuffer.c_str());
		szBuffer.append(szTempBuffer);
	}

	if (kPromotion.getManaCost() > 0)
	{
		bool bPopup = gDLL->getInterfaceIFace()->isPopupUp();
		int iMana = GC.getPromotionInfo(ePromotion).getManaCost();
		if(NO_PLAYER != eActivePlayer){
			iMana = (iMana * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		}
		if (bPopup)
		{
			iMana *= std::max(50, 100 - (GC.getGameINLINE().getActivePlayer() == NO_PLAYER ? 0 : GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getReducedEquipmentCost()));
			iMana /= 100;
		}
		szBuffer.append(pcNewline);
		CvWString szTempBuffer;
		szTempBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MANA_COST", iMana));
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER && GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getMana() < iMana && gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL)
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), szTempBuffer.c_str());
		szBuffer.append(szTempBuffer);
	}

	if (kPromotion.getFaithCost() > 0)
	{
		bool bPopup = gDLL->getInterfaceIFace()->isPopupUp();
		int iFaith = GC.getPromotionInfo(ePromotion).getFaithCost();
		if(NO_PLAYER != eActivePlayer){
			iFaith = (iFaith * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		}
		if (bPopup)
		{
			iFaith *= std::max(50, 100 - (GC.getGameINLINE().getActivePlayer() == NO_PLAYER ? 0 : GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getReducedEquipmentCost()));
			iFaith /= 100;
		}
		szBuffer.append(pcNewline);
		CvWString szTempBuffer;
		szTempBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FAITH_COST", iFaith));
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER && GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getFaith() < iFaith && gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL)
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), szTempBuffer.c_str());
		szBuffer.append(szTempBuffer);
	}

	if (kPromotion.getYieldCostType() != NO_YIELD && kPromotion.getYieldCost(1) > 0)
	{
		bool bPopup = gDLL->getInterfaceIFace()->isPopupUp();
/*
		int iYieldCost = GC.getPromotionInfo(ePromotion).getYieldCost(1);
		if(NO_PLAYER != eActivePlayer){
			iYieldCost = (iYieldCost * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		}
		if (bPopup)
		{
			iYieldCost *= std::max(50, 100 - (GC.getGameINLINE().getActivePlayer() == NO_PLAYER ? 0 : GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getReducedEquipmentCost()));
			iYieldCost /= 100;
			if (ePromotion == GC.getInfoTypeForString("PROMOTION_POTION_STRONG"))
				iYieldCost *= (pSelectedUnit->getStrengthPotionsUsed() + 1);
		}
*/
		int iYieldCost = GC.getPromotionInfo(ePromotion).getYieldCost(1);

		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			iYieldCost *= GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getEquipmentCostModifier(ePromotion);
			iYieldCost /= 100;
		}

		iYieldCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
		iYieldCost /= 100;

		szTempBuffer.clear();
		szTempBuffer.append(gDLL->getText("TXT_KEY_UNIT_YIELD_COST", GC.getYieldInfo((YieldTypes)GC.getPromotionInfo(ePromotion).getYieldCostType()).getDescription(), iYieldCost));
		if (bPopup && iYieldCost > (GC.getGameINLINE().getActivePlayer() == NO_PLAYER ? 0 : GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getGlobalYield(GC.getPromotionInfo(ePromotion).getYieldCostType())))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_RED"), szTempBuffer.c_str());
		}
		szBuffer.append(pcNewline);
		szBuffer.append(szTempBuffer);
	}

	/*************************************************************************************************/
	/**	ADDON (Promotions can be unlocked by Gameoptions) Sephi                     					**/
	/*************************************************************************************************/
	if (kPromotion.getRequiredGameOption() != NO_GAMEOPTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES_GAMEOPTION"));
		szBuffer.append(GC.getGameOptionInfo((GameOptionTypes)kPromotion.getRequiredGameOption()).getDescription());
	}

	if (kPromotion.getMinLevel() > 4)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MIN_LEVEL", GC.getPromotionInfo(ePromotion).getMinLevel()));
	}

	if (kPromotion.getMaxLevel() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MAX_LEVEL", GC.getPromotionInfo(ePromotion).getMaxLevel()));
	}

	/** Equipment **/


	bFirst = true;
	for (iI = 0; iI < GC.getNumDurabilityInfos(); iI++)
	{
		if (kPromotion.isMakeEquipmentDurabilityValid(iI) && (pSelectedUnit == NULL || !GC.getUnitInfo(pSelectedUnit->getUnitType()).isNoEquipment()))
		{
			if (!bFirst)
			{
				szBuffer.append(L", ");
			}
			if (bFirst)
			{
				bFirst = false;
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MAKE_EQUIPMENT_VALID"));
			}
			szTempBuffer.clear();
			szTempBuffer.append(GC.getDurabilityInfo((DurabilityTypes)iI).getDescription());
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_GREEN"), szTempBuffer.c_str());
			szBuffer.append(szTempBuffer);
		}
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (GC.getPromotionInfo(ePromotion).isBlitz())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_BLITZ_TEXT"));
	}

	if (GC.getPromotionInfo(ePromotion).isAmphib())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AMPHIB_TEXT"));
	}

	if (GC.getPromotionInfo(ePromotion).isRiver())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_RIVER_ATTACK_TEXT"));
	}

	if (GC.getPromotionInfo(ePromotion).isEnemyRoute())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ENEMY_ROADS_TEXT"));
	}

	if (GC.getPromotionInfo(ePromotion).isAlwaysHeal())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ALWAYS_HEAL_TEXT"));
	}

	if (GC.getPromotionInfo(ePromotion).isImmuneToFirstStrikes())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_FIRST_STRIKES_TEXT"));
	}

	bFirst = true;
	if (GC.getPromotionInfo(ePromotion).isHillsDoubleMove())
	{
		bFirst = false;
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HILLS_MOVE_TEXT"));
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
	{
		if (GC.getPromotionInfo(ePromotion).getTerrainDoubleMove(iI))
		{
			if (bFirst) {
				bFirst = false;
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", getLinkedText((TerrainTypes)iI).c_str()));
			}
			else {
				szTempBuffer.Format(L", %s", getLinkedText((TerrainTypes)iI).c_str());
				szBuffer.append(szTempBuffer);
			}
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
	{
		if (GC.getPromotionInfo(ePromotion).getFeatureDoubleMove(iI))
		{
			if (bFirst) {
				bFirst = false;
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_MOVE_TEXT", getLinkedText((FeatureTypes)iI).c_str()));
			}
			else {
				szTempBuffer.Format(L", %s", getLinkedText((FeatureTypes)iI).c_str());
				szBuffer.append(szTempBuffer);
			}
		}
	}

	if (GC.getPromotionInfo(ePromotion).getVisibilityChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_VISIBILITY_TEXT", GC.getPromotionInfo(ePromotion).getVisibilityChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getMovesChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MOVE_TEXT", GC.getPromotionInfo(ePromotion).getMovesChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getMoveDiscountChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MOVE_DISCOUNT_TEXT", -(GC.getPromotionInfo(ePromotion).getMoveDiscountChange())));
	}

	if (GC.getPromotionInfo(ePromotion).getAirRangeChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AIR_RANGE_TEXT", GC.getPromotionInfo(ePromotion).getAirRangeChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getInterceptChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_INTERCEPT_TEXT", GC.getPromotionInfo(ePromotion).getInterceptChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getEvasionChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EVASION_TEXT", GC.getPromotionInfo(ePromotion).getEvasionChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getWithdrawalChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_WITHDRAWAL_TEXT", GC.getPromotionInfo(ePromotion).getWithdrawalChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getCargoChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CARGO_TEXT", GC.getPromotionInfo(ePromotion).getCargoChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getCollateralDamageChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_COLLATERAL_DAMAGE_TEXT", GC.getPromotionInfo(ePromotion).getCollateralDamageChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getBombardRateChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_BOMBARD_TEXT", GC.getPromotionInfo(ePromotion).getBombardRateChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getFirstStrikesChange() != 0)
	{
		if (GC.getPromotionInfo(ePromotion).getFirstStrikesChange() == 1)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKE_TEXT", GC.getPromotionInfo(ePromotion).getFirstStrikesChange()));
		}
		else
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKES_TEXT", GC.getPromotionInfo(ePromotion).getFirstStrikesChange()));
		}
	}

	if (GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange() != 0)
	{
		if (GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange() == 1)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKE_CHANCE_TEXT", GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange()));
		}
		else
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FIRST_STRIKES_CHANCE_TEXT", GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange()));
		}
	}
	/*************************************************************************************************/
	/**	FFHBUG denev																				**/
	/**	ADDON (FFHBUG) 																	**/
	/**																								**/
	/*************************************************************************************************/
	//BUGFfH: Added by Denev 2009/09/06
	if (GC.getPromotionInfo(ePromotion).getEnemyHealChange() == GC.getPromotionInfo(ePromotion).getNeutralHealChange()
		&& GC.getPromotionInfo(ePromotion).getEnemyHealChange() == GC.getPromotionInfo(ePromotion).getFriendlyHealChange())
	{
		if (GC.getPromotionInfo(ePromotion).getEnemyHealChange() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", GC.getPromotionInfo(ePromotion).getEnemyHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT"));
		}
	}
	else
	{
		//BUGFfH: End Add
		if (GC.getPromotionInfo(ePromotion).getEnemyHealChange() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", GC.getPromotionInfo(ePromotion).getEnemyHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_ENEMY_LANDS_TEXT"));
		}


		if (GC.getPromotionInfo(ePromotion).getNeutralHealChange() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", GC.getPromotionInfo(ePromotion).getNeutralHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_NEUTRAL_LANDS_TEXT"));
		}

		if (GC.getPromotionInfo(ePromotion).getFriendlyHealChange() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_EXTRA_TEXT", GC.getPromotionInfo(ePromotion).getFriendlyHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_FRIENDLY_LANDS_TEXT"));
		}
		//BUGFfH: Added by Denev 2009/09/06
	}
	/*************************************************************************************************/
	/**	END																							**/
	/*************************************************************************************************/

	if (GC.getPromotionInfo(ePromotion).getSameTileHealChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_SAME_TEXT", GC.getPromotionInfo(ePromotion).getSameTileHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT"));
	}

	if (GC.getPromotionInfo(ePromotion).getAdjacentTileHealChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HEALS_ADJACENT_TEXT", GC.getPromotionInfo(ePromotion).getAdjacentTileHealChange()) + gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TURN_TEXT"));
	}

	//>>>>	BUGFfH: Moved from below(*10) by Denev 2009/09/10
	if (GC.getPromotionInfo(ePromotion).getExtraCombatStr() == GC.getPromotionInfo(ePromotion).getExtraCombatDefense())
	{
		if (GC.getPromotionInfo(ePromotion).getExtraCombatStr() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EXTRA_COMBAT_STR_BOTH", GC.getPromotionInfo(ePromotion).getExtraCombatStr()));
		}
	}
	else
	{
		if (GC.getPromotionInfo(ePromotion).getExtraCombatStr() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EXTRA_COMBAT_STR", GC.getPromotionInfo(ePromotion).getExtraCombatStr()));
		}
		if (GC.getPromotionInfo(ePromotion).getExtraCombatDefense() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EXTRA_COMBAT_STR_DEFENSE", GC.getPromotionInfo(ePromotion).getExtraCombatDefense()));
		}
	}
	//<<<<	BUGFfH: End Move

	if (GC.getPromotionInfo(ePromotion).getCombatPercent() == GC.getPromotionInfo(ePromotion).getCombatPercentDefense())
	{
		if (GC.getPromotionInfo(ePromotion).getCombatPercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_STRENGTH_TEXT", GC.getPromotionInfo(ePromotion).getCombatPercent()));
		}
	}
	else
	{
		if (GC.getPromotionInfo(ePromotion).getCombatPercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_STRENGTH_TEXT_ATTACK", GC.getPromotionInfo(ePromotion).getCombatPercent()));
		}

		if (GC.getPromotionInfo(ePromotion).getCombatPercentDefense() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_STRENGTH_TEXT_DEFENSE", GC.getPromotionInfo(ePromotion).getCombatPercentDefense()));
		}
	}

	/*************************************************************************************************/
	/**	FFHBUG denev																				**/
	/**	ADDON (FFHBUG) 																	**/
	/**																								**/
	/*************************************************************************************************/
	//BUGFfH: Added by Denev 2009/09/05
	if (GC.getPromotionInfo(ePromotion).getCityAttackPercent() == GC.getPromotionInfo(ePromotion).getCityDefensePercent())
	{
		if (GC.getPromotionInfo(ePromotion).getCityAttackPercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CITY_STRENGTH_MOD", GC.getPromotionInfo(ePromotion).getCityAttackPercent()));
		}
	}
	else
	{
		//BUGFfH: End Add


		if (GC.getPromotionInfo(ePromotion).getCityAttackPercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CITY_ATTACK_TEXT", GC.getPromotionInfo(ePromotion).getCityAttackPercent()));
		}

		if (GC.getPromotionInfo(ePromotion).getCityDefensePercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CITY_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getCityDefensePercent()));
		}
		//BUGFfH: Added by Denev 2009/09/05
	}
	//BUGFfH: End Add

	//BUGFfH: Added by Denev 2009/09/05
	if (GC.getPromotionInfo(ePromotion).getHillsAttackPercent() == GC.getPromotionInfo(ePromotion).getHillsDefensePercent())
	{
		if (GC.getPromotionInfo(ePromotion).getHillsAttackPercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HILLS_STRENGTH", GC.getPromotionInfo(ePromotion).getHillsAttackPercent()));
		}
	}
	else
	{
		//BUGFfH: End Add
		if (GC.getPromotionInfo(ePromotion).getHillsAttackPercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HILLS_ATTACK", GC.getPromotionInfo(ePromotion).getHillsAttackPercent()));
		}

		if (GC.getPromotionInfo(ePromotion).getHillsDefensePercent() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HILLS_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getHillsDefensePercent()));
		}
		//BUGFfH: Added by Denev 2009/09/05
	}
	//BUGFfH: End Add
	/*************************************************************************************************/
	/**	END																							**/
	/*************************************************************************************************/

	if (GC.getPromotionInfo(ePromotion).getRevoltProtection() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_REVOLT_PROTECTION_TEXT", GC.getPromotionInfo(ePromotion).getRevoltProtection()));
	}

	if (GC.getPromotionInfo(ePromotion).getCollateralDamageProtection() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_COLLATERAL_PROTECTION_TEXT", GC.getPromotionInfo(ePromotion).getCollateralDamageProtection()));
	}

	if (GC.getPromotionInfo(ePromotion).getPillageChange() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_PILLAGE_CHANGE_TEXT", GC.getPromotionInfo(ePromotion).getPillageChange()));
	}

	if (GC.getPromotionInfo(ePromotion).getUpgradeDiscount() != 0)
	{
		if (100 == GC.getPromotionInfo(ePromotion).getUpgradeDiscount())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_UPGRADE_DISCOUNT_FREE_TEXT"));
		}
		else
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_UPGRADE_DISCOUNT_TEXT", GC.getPromotionInfo(ePromotion).getUpgradeDiscount()));
		}
	}

	if (GC.getPromotionInfo(ePromotion).getExperiencePercent() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FASTER_EXPERIENCE_TEXT", GC.getPromotionInfo(ePromotion).getExperiencePercent()));
	}

	if (GC.getPromotionInfo(ePromotion).getKamikazePercent() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_KAMIKAZE_TEXT", GC.getPromotionInfo(ePromotion).getKamikazePercent()));
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
	{
		/*************************************************************************************************/
		/**	FFHBUG denev																				**/
		/**	ADDON (FFHBUG) 																	**/
		/**																								**/
		/*************************************************************************************************/
		//>>>>	BUGFfH: Modified by Denev 2009/09/15
		/*
				if (GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI) != 0)
				{
					szBuffer.append(pcNewline);
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ATTACK_TEXT", GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide()));
				}

				if (GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI) != 0)
				{
					szBuffer.append(pcNewline);
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide()));
				}
		*/
		if (GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI) == GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI))
		{
			if (GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI) != 0)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_STRENGTH",
					GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI),
					getLinkedText((TerrainTypes)iI).c_str()));
			}
		}
		else
		{
			if (GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI) != 0)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ATTACK_TEXT",
					GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI),
					getLinkedText((TerrainTypes)iI).c_str()));
			}

			if (GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI) != 0)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSE_TEXT",
					GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI),
					getLinkedText((TerrainTypes)iI).c_str()));
			}
		}
		//<<<<	BUGFfH: End Modify
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
	{
		/*************************************************************************************************/
		/**	FFHBUG denev																				**/
		/**	ADDON (FFHBUG) 																	**/
		/**																								**/
		/*************************************************************************************************/
		//>>>>	BUGFfH: Modified by Denev 2009/09/15
		/*
				if (GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI) != 0)
				{
					szBuffer.append(pcNewline);
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ATTACK_TEXT", GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI), GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide()));
				}

				if (GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI) != 0)
				{
					szBuffer.append(pcNewline);
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSE_TEXT", GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI), GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide()));
				}
		*/
		if (GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI) == GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI))
		{
			if (GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI) != 0)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_STRENGTH",
					GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI),
					getLinkedText((FeatureTypes)iI).c_str()));
			}
		}
		else
		{
			if (GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI) != 0)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ATTACK_TEXT",
					GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI),
					getLinkedText((FeatureTypes)iI).c_str()));
			}

			if (GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI) != 0)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSE_TEXT",
					GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI),
					getLinkedText((FeatureTypes)iI).c_str()));
			}
		}
		//<<<<	BUGFfH: End Modify
	}
	/*************************************************************************************************/
	/**	END																							**/
	/*************************************************************************************************/

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
	{
		if (GC.getPromotionInfo(ePromotion).getUnitCombatModifierPercent(iI) != 0)
		{
			szBuffer.append(pcNewline);
			//>>>>	BUGFfH: Modified by Denev 2009/09/14
			//			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_VERSUS_TEXT", GC.getPromotionInfo(ePromotion).getUnitCombatModifierPercent(iI), GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide()));
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_VERSUS_TEXT",
				GC.getPromotionInfo(ePromotion).getUnitCombatModifierPercent(iI),
				getLinkedText((UnitCombatTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify

		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; ++iI)
	{
		if (GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI) != 0)
		{
			szBuffer.append(pcNewline);
			//>>>>	BUGFfH: Modified by Denev 2009/09/14
			//			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_VERSUS_TEXT", GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI), GC.getDomainInfo((DomainTypes)iI).getTextKeyWide()));
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_VERSUS_TEXT",
				GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI),
				getLinkedText((DomainTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify

		}
	}

	if (kPromotion.isFlamingArrows())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FLAMING_ARROWS_PEDIA"));
	}

	if (kPromotion.getCaptureAnimalChance() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CAPTURE_ANIMAL_PEDIA", kPromotion.getCaptureAnimalChance()));
	}

	if (kPromotion.getCaptureBeastChance() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CAPTURE_BEAST_PEDIA", kPromotion.getCaptureBeastChance()));
	}

	if (kPromotion.isNoXP())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_NO_XP_PEDIA"));
	}

	if (kPromotion.isLimitedByNumberBonuses() && kPromotion.getBonusPrereq() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_LIMITED_BY_BONUSES", getLinkedText((BonusTypes)kPromotion.getBonusPrereq()).c_str()));
	}

	//FfH: Added by Kael 07/30/2007
	if (GC.getPromotionInfo(ePromotion).isRace())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_RACE_PEDIA", GC.getPromotionInfo(ePromotion).getDescription()));
	}
	for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getDamageTypeCombat((DamageTypes)iI) != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_DAMAGE_TYPE_COMBAT", GC.getPromotionInfo(ePromotion).getDamageTypeCombat((DamageTypes)iI), GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
		}
		if (GC.getPromotionInfo(ePromotion).getDamageTypeResist((DamageTypes)iI) != 0)
		{
			szBuffer.append(pcNewline);
			if (GC.getPromotionInfo(ePromotion).getDamageTypeResist((DamageTypes)iI) == 1000)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TYPE_IMMUNE", GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
			}
			else if (GC.getPromotionInfo(ePromotion).getDamageTypeResist((DamageTypes)iI) > 0)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TYPE_RESIST", GC.getPromotionInfo(ePromotion).getDamageTypeResist((DamageTypes)iI), GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TYPE_VULNERABILITY", -GC.getPromotionInfo(ePromotion).getDamageTypeResist((DamageTypes)iI), GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
			}
		}

		if (GC.getPromotionInfo(ePromotion).getCombatAuraDamageTypeResist((DamageTypes)iI) != 0)
		{
			szBuffer.append(pcNewline);
			if (GC.getPromotionInfo(ePromotion).getCombatAuraDamageTypeResist((DamageTypes)iI) == 100)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TYPE_IMMUNE", GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DAMAGE_TYPE_RESIST_AURA", GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
			}
		}
	}
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).getBonusAffinity((BonusTypes)iI) != 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/09/15
			//			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_AFFINITY", GC.getPromotionInfo(ePromotion).getBonusAffinity((BonusTypes)iI), GC.getBonusInfo((BonusTypes)iI).getDescription()));
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_AFFINITY",
				GC.getPromotionInfo(ePromotion).getBonusAffinity((BonusTypes)iI),
				getLinkedText((BonusTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify

		}
	}
	if (GC.getPromotionInfo(ePromotion).getDefensiveStrikeChance() == 0)
	{
		if (GC.getPromotionInfo(ePromotion).getDefensiveStrikeDamage() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSIVE_STRIKE_DAMAGE_PEDIA", GC.getPromotionInfo(ePromotion).getDefensiveStrikeDamage()));
		}
	}
	else
	{
		if (GC.getPromotionInfo(ePromotion).getDefensiveStrikeDamage() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSIVE_STRIKE_PEDIA", GC.getPromotionInfo(ePromotion).getDefensiveStrikeChance(), GC.getPromotionInfo(ePromotion).getDefensiveStrikeDamage()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSIVE_STRIKE_CHANCE_PEDIA", GC.getPromotionInfo(ePromotion).getDefensiveStrikeChance()));
		}
	}
	if (GC.getPromotionInfo(ePromotion).isImmuneToDefensiveStrike())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_DEFENSIVE_STRIKE_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).getCombatPercentInBorders() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_PERCENT_IN_BORDERS", GC.getPromotionInfo(ePromotion).getCombatPercentInBorders()));
	}
	/*************************************************************************************************/
	/**	FFHBUG denev																				**/
	/**	ADDON (FFHBUG) 																	**/
	/**																								**/
	/*************************************************************************************************/
	//>>>>	BUGFfH: Moved to above(*10) by Denev 2009/09/10
	/*
		if (GC.getPromotionInfo(ePromotion).getExtraCombatStr() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EXTRA_COMBAT_STR", GC.getPromotionInfo(ePromotion).getExtraCombatStr()));
		}
		if (GC.getPromotionInfo(ePromotion).getExtraCombatDefense() != 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EXTRA_COMBAT_STR_DEFENSE", GC.getPromotionInfo(ePromotion).getExtraCombatDefense()));
		}
	*/
	//<<<<	BUGFfH: End Move
	/*************************************************************************************************/
	/**	END																							**/
	/*************************************************************************************************/

	if (GC.getPromotionInfo(ePromotion).isTargetWeakestUnit())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_TARGET_WEAKEST_UNIT"));
	}
	if (GC.getPromotionInfo(ePromotion).isTargetWeakestUnitCounter())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_TARGET_WEAKEST_UNIT_COUNTER"));
	}
	if (GC.getPromotionInfo(ePromotion).getBetterDefenderThanPercent() != 0)
	{
		szBuffer.append(pcNewline);
		if (GC.getPromotionInfo(ePromotion).getBetterDefenderThanPercent() < 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_BETTER_DEFENDER_THAN_PERCENT_LESS"));
		}
		if (GC.getPromotionInfo(ePromotion).getBetterDefenderThanPercent() > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_BETTER_DEFENDER_THAN_PERCENT_MORE"));
		}
	}
	if (GC.getPromotionInfo(ePromotion).getPromotionCombatApply() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_APPLY", getLinkedText((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionCombatApply()).c_str(), GC.getPromotionInfo(ePromotion).getPromotionCombatApplyChance()));
	}
	if (GC.getPromotionInfo(ePromotion).getPromotionRandomApply() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_RANDOM_APPLY", getLinkedText((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionRandomApply()).c_str(), GC.getPromotionInfo(ePromotion).getPromotionRandomApplyChance()));

	}
	if (GC.getPromotionInfo(ePromotion).isBoarding())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_BOARDING_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).getCaptureUnitCombat() != NO_UNITCOMBAT)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CAPTURE_UNITCOMBAT", getLinkedText((UnitCombatTypes)GC.getPromotionInfo(ePromotion).getCaptureUnitCombat()).c_str()));
	}
	if (GC.getPromotionInfo(ePromotion).getCombatCapturePercent() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_CAPTURE_PERCENT", GC.getPromotionInfo(ePromotion).getCombatCapturePercent()));
	}
	if (GC.getPromotionInfo(ePromotion).getCombatHealPercent() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_HEAL_PERCENT", GC.getPromotionInfo(ePromotion).getCombatHealPercent()));
	}
	if (GC.getPromotionInfo(ePromotion).getCombatManaGained() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_COMBAT_MANA_GAINED", GC.getPromotionInfo(ePromotion).getCombatManaGained()));
	}
	if (GC.getPromotionInfo(ePromotion).getCombatLimit() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_COMBAT_LIMIT", (100 * GC.getPromotionInfo(ePromotion).getCombatLimit()) / GC.getMAX_HIT_POINTS()));
	}
	if (GC.getPromotionInfo(ePromotion).getFear() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FEAR_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).getFreeXPPerTurn() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FREE_XP_PER_TURN", GC.getPromotionInfo(ePromotion).getFreeXPPerTurn(), GC.getDefineINT("FREE_XP_MAX")));
	}
	if (GC.getPromotionInfo(ePromotion).getFreeXPFromCombat() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FREE_XP_FROM_COMBAT", GC.getPromotionInfo(ePromotion).getFreeXPFromCombat()));
	}
	if (GC.getPromotionInfo(ePromotion).getGoldFromCombat() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_GOLD_FROM_COMBAT", GC.getPromotionInfo(ePromotion).getGoldFromCombat()));
	}
	if (GC.getPromotionInfo(ePromotion).getModifyGlobalCounter() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_MESSAGE_MODIFY_GLOBAL_COUNTER", GC.getPromotionInfo(ePromotion).getModifyGlobalCounter()));
	}
	if (GC.getPromotionInfo(ePromotion).getModifyGlobalCounterOnCombat() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_MESSAGE_MODIFY_GLOBAL_COUNTER_ON_COMBAT", GC.getPromotionInfo(ePromotion).getModifyGlobalCounterOnCombat()));
	}
	if (GC.getPromotionInfo(ePromotion).getSpellCasterXP() != 0)
	{
		szBuffer.append(pcNewline);
		//>>>>	BUGFfH: Modified by Denev 2009/09/27
		//		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_SPELL_CASTER_XP"));
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FREE_XP_CHANCE_PER_TURN", GC.getPromotionInfo(ePromotion).getSpellCasterXP()));
		//<<<<	BUGFfH: End Modify

	}
	if (GC.getPromotionInfo(ePromotion).isFlying())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FLYING_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isHeld())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HELD_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isHiddenNationality())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_HIDDEN_NATIONALITY_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isIgnoreBuildingDefense())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IGNORE_BUILDING_DEFENSE"));
	}
	if (GC.getPromotionInfo(ePromotion).isImmortal())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMORTAL_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isImmuneToCapture())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_CAPTURE_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isImmuneToMagic())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_MAGIC_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isImmuneToFear())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_FEAR_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).getPromotionImmune1() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		const PromotionTypes ePromotionImmune1 = (PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionImmune1();
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE", getLinkedText(ePromotionImmune1).c_str()));
	}
	if (GC.getPromotionInfo(ePromotion).getPromotionImmune2() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		const PromotionTypes ePromotionImmune2 = (PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionImmune2();
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE", getLinkedText(ePromotionImmune2).c_str()));
	}
	if (GC.getPromotionInfo(ePromotion).getPromotionImmune3() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		const PromotionTypes ePromotionImmune3 = (PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionImmune3();
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE", getLinkedText(ePromotionImmune3).c_str()));
	}
	if (GC.getPromotionInfo(ePromotion).isInvisible())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_INVISIBLE_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isSeeInvisible())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_SEE_INVISIBLE_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isDoubleFortifyBonus())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DOUBLE_FORTIFY_BONUS"));
	}
	if (GC.getPromotionInfo(ePromotion).isTwincast())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_TWINCAST_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isWaterWalking())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_WATER_WALKING_PEDIA"));
	}
	if (GC.getPromotionInfo(ePromotion).isNotAlive())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_NOT_ALIVE"));
	}
	if (GC.getPromotionInfo(ePromotion).getResistMagic() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_RESIST", GC.getPromotionInfo(ePromotion).getResistMagic()));
	}
	if (GC.getPromotionInfo(ePromotion).getSpellDamageModify() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_SPELL_DAMAGE_MODIFY", GC.getPromotionInfo(ePromotion).getSpellDamageModify()));
	}
	if (GC.getPromotionInfo(ePromotion).getCasterResistModify() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CASTER_RESIST_MODIFY", GC.getPromotionInfo(ePromotion).getCasterResistModify()));
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumSpellInfos(); iI++)
	{
		if (GC.getSpellInfo((SpellTypes)iI).getPromotionPrereq1() == ePromotion)
		{
			if (!GC.getSpellInfo((SpellTypes)iI).isGraphicalOnly())
			{
				if (GC.getSpellInfo((SpellTypes)iI).getPromotionPrereq2() == NO_PROMOTION)
				{
					if (bFirst)
					{
						szBuffer.append(pcNewline);
						szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ALLOWS_TEXT", getLinkedText((SpellTypes)iI).c_str()));
						bFirst = false;
					}
					else
					{
						szTempBuffer.Format(L", %s", getLinkedText((SpellTypes)iI).c_str());
						szBuffer.append(szTempBuffer);
					}
				}
			}
		}
	}

	for (iI = 0; iI < GC.getNumSpellInfos(); iI++)
	{
		if (GC.getSpellInfo((SpellTypes)iI).getPromotionPrereq1() == ePromotion)
		{
			if (!GC.getSpellInfo((SpellTypes)iI).isGraphicalOnly())
			{
				if (GC.getSpellInfo((SpellTypes)iI).getCivilizationPrereq() == NO_CIVILIZATION || eActivePlayer == NO_PLAYER
					|| GET_PLAYER(eActivePlayer).getCivilizationType() == GC.getSpellInfo((SpellTypes)iI).getCivilizationPrereq())
				{
					if (GC.getSpellInfo((SpellTypes)iI).getPromotionPrereq2() != NO_PROMOTION)
					{
						szBuffer.append(pcNewline);
						szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ALLOWS_WITH_TEXT",
							getLinkedText((SpellTypes)iI).c_str(),
							getLinkedText((PromotionTypes)GC.getSpellInfo((SpellTypes)iI).getPromotionPrereq2()).c_str()));
					}
				}
			}
		}
	}

	if (GC.getPromotionInfo(ePromotion).getExpireChance() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EXPIRE_CHANCE", GC.getPromotionInfo(ePromotion).getExpireChance()));
	}
	if (GC.getPromotionInfo(ePromotion).isRemovedByCasting())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_REMOVED_BY_CASTING"));
	}
	if (GC.getPromotionInfo(ePromotion).isRemovedByCombat())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_REMOVED_BY_COMBAT"));
	}
	if (GC.getPromotionInfo(ePromotion).isRemovedWhenHealed())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_REMOVED_WHEN_HEALED"));
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/29
	if (GC.getPromotionInfo(ePromotion).isDispellable())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_REMOVED_BY_DISPEL"));
	}
	//<<<<	BUGFfH: End Add

	if (GC.getPromotionInfo(ePromotion).getPromotionSummonPerk() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_SUMMON_PERK", getLinkedText((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionSummonPerk()).c_str()));
	}
	if (GC.getPromotionInfo(ePromotion).getPromotionCombatMod() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_VERSUS_TEXT",
			GC.getPromotionInfo(ePromotion).getPromotionCombatMod(),
			getLinkedText((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionCombatType()).c_str()));
	}
	if (GC.getPromotionInfo(ePromotion).getBetrayalChance() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_BETRAYAL_CHANCE", GC.getPromotionInfo(ePromotion).getBetrayalChance()));
	}
	if (GC.getPromotionInfo(ePromotion).getWorkRateModify() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_WORK_RATE_MODIFY", GC.getPromotionInfo(ePromotion).getWorkRateModify()));
	}
	//FfH: End Add
	/*************************************************************************************************/
	/**	ADDON (Ranged Combat) Sephi								                     				**/
	/*************************************************************************************************/
	if (GC.getPromotionInfo(ePromotion).getAirCombat() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AIR_COMBAT", GC.getPromotionInfo(ePromotion).getAirCombat()));
	}

	if (GC.getPromotionInfo(ePromotion).getAirCombatLimitBoost() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AIR_COMBAT_LIMIT", GC.getPromotionInfo(ePromotion).getAirCombatLimitBoost()));
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/
	//>>>>	More Detailed Civilopedia: Added by Denev 2009/11/10
	if (GC.getPromotionInfo(ePromotion).isDefensiveStrikes())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSIVE_STRIKES"));
	}
	//<<<<	More Detailed Civilopedia: End Add
	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/*************************************************************************************************/
	for (int i = 0; i < GC.getNumCorporationInfos(); i++)
	{
		if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isHasCorporationSupport((CorporationTypes)i))
		{
			if (GC.getPromotionInfo(ePromotion).getCorporationSupport((CorporationTypes)i) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getPromotionInfo(ePromotion).getCorporationSupport((CorporationTypes)i)));
			}

			if (GC.getPromotionInfo(ePromotion).getCorporationSupportMultiplier((CorporationTypes)i) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT_MODIFIER", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getPromotionInfo(ePromotion).getCorporationSupportMultiplier((CorporationTypes)i)));
			}
		}
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/
	/*************************************************************************************************/
	/**	ADDON (new Functions Definitions) Sephi					                     				**/
	/*************************************************************************************************/
	if (GC.getPromotionInfo(ePromotion).getCombatAuraEffect() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_EFFECT"));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraRange() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_RANGE", GC.getPromotionInfo(ePromotion).getCombatAuraRange()));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraTargets() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_TARGETS", GC.getPromotionInfo(ePromotion).getCombatAuraTargets()));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraType() != NO_COMBATAURA)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_NEW_COMBATAURA", GC.getCombatAuraClassInfo((CombatAuraClassTypes)GC.getCombatAuraInfo((CombatAuraTypes)GC.getPromotionInfo(ePromotion).getCombatAuraType()).getCombatAuraClassType()).getDescription()));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraExperienceChange() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_EXPERIENCE"));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraFearEffectChange() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_FEAREFFECT"));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraIgnoreFirstStrikes() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_IGNORE_FIRST_STRIKES"));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraIncreaseFirstStrikes() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_INCREASE_FIRST_STRIKES"));
	}

	if (GC.getPromotionInfo(ePromotion).isCombatAuraLoyaltyChange())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_LOYALTY"));
	}

	if (GC.getPromotionInfo(ePromotion).isCombatAuraImmuneToFearChange())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNITY_TO_FEAR"));
	}

	if (GC.getPromotionInfo(ePromotion).isCombatAuraWaterwalking())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_WATER_WALKING"));
	}

	if (GC.getPromotionInfo(ePromotion).isCombatAuraEnchantedWeapons())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_ENCHANTED_BLADES"));
		bFirst = true;
		for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			if (GC.getBonusInfo((BonusTypes)iI).getEnchantedWeaponBonus() > 0)
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szBuffer.append(L", ");
				}
				szBuffer.append(getLinkedText((BonusTypes)iI));
				szTempBuffer.format(L" (%d)", GC.getBonusInfo((BonusTypes)iI).getEnchantedWeaponBonus());
				szBuffer.append(szTempBuffer);
			}
		}
	}

	if (kPromotion.getEquipmentYieldType() != NO_YIELD)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EQUIPMENT_YIELD", GC.getYieldInfo((YieldTypes)kPromotion.getEquipmentYieldType()).getDescription()));
	}

	if (GC.getPromotionInfo(ePromotion).isCombatAuraImmunityRangedStrike())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_IMMUNITY_RANGED_STRIKE"));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraCityAttackChange() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_CITY_ATTACK"));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraHealthRegenerationChange() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_HEALTH_REGENERATION"));
	}

	if (GC.getPromotionInfo(ePromotion).getCombatAuraStrengthChange() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_STRENGTH_EFFECT"));
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).isCombatAuraApplyPromotion(iI))
		{
			if (bFirst)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_APPLY_PROMOTION"));
				bFirst = false;
			}
			else
			{
				szBuffer.append(L", ");
			}
			szBuffer.append(getLinkedText((PromotionTypes)iI).c_str());
		}
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).isCombatAuraRemovePromotion(iI))
		{
			if (bFirst)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_AURA_REMOVE_PROMOTION"));
				bFirst = false;
			}
			else
			{
				szBuffer.append(L", ");
			}
			szBuffer.append(getLinkedText((PromotionTypes)iI).c_str());
		}
	}



	if (GC.getPromotionInfo(ePromotion).getIDWPercent() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_IDWPERCENT", GC.getPromotionInfo(ePromotion).getIDWPercent()));
	}

	if (GC.getPromotionInfo(ePromotion).getUnitClassCreateFromCombat() != NO_UNITCLASS && GC.getPromotionInfo(ePromotion).getUnitClassCreateFromCombatChance() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CREATE_UNITCLASS_FROM_COMBAT", getLinkedText((UnitClassTypes)GC.getPromotionInfo(ePromotion).getUnitClassCreateFromCombat()).c_str()));
	}

	if (GC.getPromotionInfo(ePromotion).getEnslavementChance() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ENSLAVEMENT_CHANCE", GC.getPromotionInfo(ePromotion).getEnslavementChance()));
	}

	/**
		if (GC.getPromotionInfo(ePromotion).getEquipmentCategory() !=NO_EQUIPMENTCATEGORY && GC.getPromotionInfo(ePromotion).getEquipmentLevel() > 0)
		{
			EquipmentCategoryTypes eCategory=(EquipmentCategoryTypes)GC.getPromotionInfo(ePromotion).getEquipmentCategory();

			if (GC.getPromotionInfo(ePromotion).getDurabilityType() !=NO_DURABILITY)
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DURABILITY", GC.getDurabilityInfo((DurabilityTypes)GC.getPromotionInfo(ePromotion).getDurabilityType()).getDescription()));

				if(GC.getPromotionInfo(ePromotion).getYieldCostType()!=NO_YIELD)
				{
					int eYieldUpkeep=GC.getDurabilityInfo((DurabilityTypes)GC.getPromotionInfo(ePromotion).getDurabilityType()).getValue();
					szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EQUIPMENT_UPKEEP", eYieldUpkeep,GC.getYieldInfo((YieldTypes)GC.getPromotionInfo(ePromotion).getYieldCostType()).getDescription()));
				}

				//Display of Current Durability if a Unit is Selected
				if(gDLL->getInterfaceIFace()->getHeadSelectedUnit()!=NULL && gDLL->getInterfaceIFace()->getHeadSelectedUnit()->isHasPromotion(ePromotion))
				{
					CvWString szTempBuffer;
					int iMaxDurability=GC.getDurabilityInfo((DurabilityTypes)GC.getPromotionInfo(ePromotion).getDurabilityType()).getValue();
					int iCurrentDurability=gDLL->getInterfaceIFace()->getHeadSelectedUnit()->getDurability(eCategory);
					if(iCurrentDurability<10000)
					{
						szTempBuffer.Format(L"(%d / %d)", iCurrentDurability,iMaxDurability);
						szBuffer.append(szTempBuffer);
					}
				}
			}
		}
	**/
	bFirst = true;
	szTempBuffer.clear();

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (GC.getPromotionInfo(ePromotion).isEquipmentPromotion(iI))
		{
			if (!bFirst)
			{
				szTempBuffer.append(L", ");
			}
			else
			{
				bFirst = false;
			}
			szTempBuffer.append(getLinkedText((PromotionTypes)iI));
		}
	}

	if (!bFirst)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_EQUIPMENT_PROMOTIONS", szTempBuffer.c_str()));
	}

	if ((GC.getGameINLINE().isDebugMode()) && gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL)
	{
		int iPromotionValue = GET_PLAYER(gDLL->getInterfaceIFace()->getHeadSelectedUnit()->getOwnerINLINE()).AI_EquipmentValue(gDLL->getInterfaceIFace()->getHeadSelectedUnit(), ePromotion);
		szBuffer.append(CvWString::format(L"\nAI Promotion Value = %d", iPromotionValue));
	}

	if (GC.getPromotionInfo(ePromotion).getBonusVSUnitsFullHealth() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_BONUS_VS_UNIT_FULL_HEALTH", GC.getPromotionInfo(ePromotion).getBonusVSUnitsFullHealth()));
	}

	if (GC.getPromotionInfo(ePromotion).getReducedManaCost() > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REDUCED_MANA_COST", GC.getPromotionInfo(ePromotion).getReducedManaCost()));
	}

	int iNumPrereqs;

	iNumPrereqs = GC.getPromotionInfo(ePromotion).getNumPrereqCivilizations();
	bFirst = true;
	if (iNumPrereqs > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MISC_PREREQ_ITEM"));
		for (iI = 0; iI < iNumPrereqs; iI++)
		{
			if (!bFirst)
			{
				szBuffer.append(" ");
				szBuffer.append(gDLL->getText("TXT_KEY_OR"));
				szBuffer.append(" ");
			}
			szBuffer.append(getLinkedText((CivilizationTypes)GC.getPromotionInfo(ePromotion).getPrereqCivilization(iI)));
			//			GC.getCivilizationInfo((CivilizationTypes)GC.getPromotionInfo(ePromotion).getPrereqCivilization(iI)).getDescription());
			bFirst = false;
		}
	}

	iNumPrereqs = GC.getPromotionInfo(ePromotion).getNumPrereqBuildingANDs();
	bFirst = true;
	if (iNumPrereqs > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MISC_PREREQ_ITEM"));
		for (iI = 0; iI < iNumPrereqs; iI++)
		{
			if (!bFirst)
			{
				szBuffer.append(" ");
				szBuffer.append(gDLL->getText("TXT_KEY_AND"));
				szBuffer.append(" ");
			}
			szBuffer.append(getLinkedText((BuildingTypes)GC.getPromotionInfo(ePromotion).getPrereqBuildingAND(iI)));
			bFirst = false;
		}
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_PREREQ_CITY"));
	}

	iNumPrereqs = GC.getPromotionInfo(ePromotion).getNumPrereqBuildingORs();
	bFirst = true;
	if (iNumPrereqs > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MISC_PREREQ_ITEM"));
		for (iI = 0; iI < iNumPrereqs; iI++)
		{
			if (!bFirst)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_OR"));
			}
			szBuffer.append(getLinkedText((BuildingTypes)GC.getPromotionInfo(ePromotion).getPrereqBuildingOR(iI)));
			bFirst = false;
		}
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_PREREQ_CITY"));
	}

	int iNumPromotions = GC.getPromotionInfo(ePromotion).getNumPromotionExcludes();
	bFirst = true;
	if (iNumPromotions > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EXCLUDES_PRE"));
		for (iI = 0; iI < iNumPromotions; iI++)
		{
			if (!bFirst)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_AND"));
			}
			szBuffer.append(CvWString::format(L"<link=literal>%s</link>", GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionExcludes(iI)).getDescription()));
			bFirst = false;
		}
	}

	iNumPromotions = GC.getPromotionInfo(ePromotion).getNumPromotionMustHave();
	bFirst = true;
	if (iNumPromotions > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_MUST_HAVE_PRE"));
		for (iI = 0; iI < iNumPromotions; iI++)
		{
			if (!bFirst)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_AND"));
			}
			szBuffer.append(CvWString::format(L"<link=literal>%s</link>", GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionMustHave(iI)).getDescription()));
			bFirst = false;
		}
	}

	iNumPromotions = GC.getPromotionInfo(ePromotion).getNumPromotionReplacedBy();
	bFirst = true;
	if (iNumPromotions > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_REPLACED_PRE"));
		for (iI = 0; iI < iNumPromotions; iI++)
		{
			if (!bFirst)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_OR"));
			}
			szBuffer.append(CvWString::format(L"<link=literal>%s</link>", GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionReplacedBy(iI)).getDescription()));
			bFirst = false;
		}
	}

	if (GC.getPromotionInfo(ePromotion).getUnitCombatTarget() != NO_UNITCOMBAT)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_TARGETS_UNITCOMBAT", GC.getUnitCombatInfo((UnitCombatTypes)GC.getPromotionInfo(ePromotion).getUnitCombatTarget()).getDescription()));
	}

	if (GC.getPromotionInfo(ePromotion).getPromotionFromCombat() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_FROM_COMBAT_PRE", getLinkedText((PromotionTypes)GC.getPromotionInfo(ePromotion).getPromotionFromCombat()).c_str(), GC.getPromotionInfo(ePromotion).getPromotionCombatApplyChance()));
	}

	if (GC.getPromotionInfo(ePromotion).getSpellTargetIncrease() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_SPELL_TARGET_INCREASE", GC.getPromotionInfo(ePromotion).getSpellTargetIncrease()));
	}

	if (kPromotion.isDiscipline())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DISCIPLINE_HELP"));
	}

	if (eActivePlayer != NO_PLAYER && pSelectedUnit != NULL) {
		for (int iI = 0; iI < GC.getNumSpellInfos(); iI++) {
			if (GC.getSpellInfo((SpellTypes)iI).getCreateBuildingType() != NO_BUILDING && GC.getSpellInfo((SpellTypes)iI).getPromotionPrereq1() == ePromotion) {
				bool foundCity = false;
				int cityCanBuildCounter = 0;
				int iLoop = 0;
				for (CvCity* pLoopCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).nextCity(&iLoop))
				{	
					if (pLoopCity->getNumBuilding((BuildingTypes)GC.getSpellInfo((SpellTypes)iI).getCreateBuildingType()) <= 0) {
						if (foundCity == false) {
							szBuffer.append(CvWString::format(NEWLINE L"%cCan build %s in ", gDLL->getSymbolID(BULLET_CHAR), GC.getBuildingInfo((BuildingTypes)GC.getSpellInfo((SpellTypes)iI).getCreateBuildingType()).getDescription()));
							szBuffer.append(pLoopCity->getName());
							foundCity = true;
						}
						cityCanBuildCounter++;
					}
				}
				if (cityCanBuildCounter > 2) {
					szBuffer.append(CvWString::format(L" and %d other cities.", cityCanBuildCounter - 1));
				}
				else if (cityCanBuildCounter > 1) {
					szBuffer.append(CvWString::format(L" and %d other city.", cityCanBuildCounter - 1));
				}
			}
		}
	}

	if (gDLL->getInterfaceIFace()->getHeadSelectedUnit() != NULL)
	{
		//List Possible Class Promotions a Discipline allows
		if (kPromotion.isDiscipline())
		{
			bFirst = true;
			UnitCombatTypes eUnitCombat = gDLL->getInterfaceIFace()->getHeadSelectedUnit()->getUnitCombatType();
			if (eUnitCombat != NO_UNITCOMBAT)
			{
				for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
				{
					if (GC.getPromotionInfo((PromotionTypes)iI).isClassPromotion())
					{
						if (GC.getPromotionInfo((PromotionTypes)iI).isPromotionAPrereq(ePromotion))
						{
							if (GC.getPromotionInfo((PromotionTypes)iI).isAnyUnitCombat() ||
								GC.getPromotionInfo((PromotionTypes)iI).getUnitCombat(eUnitCombat))
							{
								if (bFirst)
								{
									bFirst = false;
									//add text
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DISCIPLINE_ALLOWS_CLASS_FIRST", getLinkedText((PromotionTypes)iI).c_str()));
								}
								else
								{
									szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DISCIPLINE_ALLOWS_CLASS_NEXT", getLinkedText((PromotionTypes)iI).c_str()));
									//add text
								}
							}
						}
					}
				}
			}
		}
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (wcslen(GC.getPromotionInfo(ePromotion).getHelp()) > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(GC.getPromotionInfo(ePromotion).getHelp());
	}
}

//
// Equipment Help
//
void CvGameTextMgr::parseEquipmentHelp(CvWStringBuffer& szBuffer, PromotionTypes ePromotion, const wchar* pcNewline)
{
	PROFILE_FUNC();

	CvWString szText, szText2;

	if (NO_PROMOTION == ePromotion)
	{
		return;
	}

	parsePromotionHelp(szBuffer, ePromotion, pcNewline);

	//SpyFanatic: Equipment Level seems partially implemented, and it does not come in effect while removing the existing category
	if (GC.getPromotionInfo(ePromotion).getEquipmentCategory() == NO_EQUIPMENTCATEGORY /*|| GC.getPromotionInfo(ePromotion).getEquipmentLevel() <= 0*/
		|| GC.getPromotionInfo(ePromotion).getDurabilityType() == NO_DURABILITY)
	{
		return;
	}

	EquipmentCategoryTypes eCategory = (EquipmentCategoryTypes)GC.getPromotionInfo(ePromotion).getEquipmentCategory();

	CvUnit* pUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
	//Display if new Equipment Removes old Equipment
	if (pUnit != NULL)
	{
		CvWString szTempBuffer;
		if (pUnit->getEquipment(eCategory) != NO_PROMOTION && pUnit->getEquipment(eCategory) != NO_PROMOTION)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_EQUIPMENT_OLD_REMOVED", getLinkedText((PromotionTypes)pUnit->getEquipment(eCategory)).c_str()));
		}
	}
}

//FfH: Added by Kael 07/23/2007
void CvGameTextMgr::parseSpellHelp(CvWStringBuffer& szBuffer, SpellTypes eSpell, const wchar* pcNewline)
{
	PROFILE_FUNC();

	CvWString szText, szText2;
	CvWString szFirstBuffer;
	CvWString szTempBuffer;

	CvUnit* pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

	if (NO_SPELL == eSpell)
	{
		return;
	}

	CvSpellInfo& kSpell = GC.getSpellInfo(eSpell);
	if (kSpell.isGlobal())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_GLOBAL"));
	}
	/*************************************************************************************************/
	/**	ADDON (New Esus) Sephi                                                    					**/
	/**						                                            							**/
	/*************************************************************************************************/
	/** New Definitions **/
	if (kSpell.getManaCost() != 0)
	{
		if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_MANA_COST", GC.getSpellInfo(eSpell).getManaCost()));
		}
		else
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_MANA_COST", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getSpellManaCost(eSpell, gDLL->getInterfaceIFace()->getHeadSelectedUnit())));
		}
	}

	if (kSpell.getFaithCost() != 0)
	{
		if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_FAITH_COST", GC.getSpellInfo(eSpell).getFaithCost()));
		}
		else
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_FAITH_COST", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getSpellFaithCost(eSpell, gDLL->getInterfaceIFace()->getHeadSelectedUnit())));
		}
	}

	if (kSpell.isManaCostReductionBonus() && kSpell.getTechPrereq() != NO_TECH)
	{
		BonusTypes eBonus = (BonusTypes)GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus();
		if (eBonus != NO_BONUS)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_ENCHANTMENT_BONUS_REDUCED_COST", getLinkedText(eBonus).c_str(), GC.getGameINLINE().getActivePlayer() == NO_PLAYER ? 0 : range(((GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getNumAvailableBonuses((BonusTypes)GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus()) - 1) * 10), 0, 30)));
		}
	}

	if (kSpell.isThiefMission())
	{
		if (pHeadSelectedUnit != NULL)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_THIEF_SUCCESS", 100 - pHeadSelectedUnit->getThiefMissionSuccessChance(eSpell)));
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_THIEF_ARRESTED", pHeadSelectedUnit->getThiefMissionArrestedChance(eSpell)));
		}
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_THIEF_EXPERIENCE", kSpell.getThiefExperienceChange()));
	}

	if (kSpell.isBombard())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_BOMBARD"));
	}

	if (kSpell.isPrereqSlaveTrade())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_PREREQ_SLAVE_TRADE"));
	}

	//<<<<	BUGFfH: End Modify
	if (kSpell.getCreateUnitType() != NO_UNIT)
	{
		szBuffer.append(pcNewline);
		if (kSpell.getCreateUnitNum() == 1)
		{
			const UnitTypes eCreateUnitType = (UnitTypes)kSpell.getCreateUnitType();
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_SUMMON_UNIT_NO_LINK", getLinkedText(eCreateUnitType).c_str()));
		}
		else
		{
			const UnitTypes eCreateUnitType = (UnitTypes)kSpell.getCreateUnitType();
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_SUMMON_UNIT_MULTIPLE_NO_LINK",
				kSpell.getCreateUnitNum(),
				getLinkedText(eCreateUnitType).c_str()));
		}
		if (kSpell.isPermanentUnitCreate())
		{
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_SUMMON_UNIT_PERMANENT"));
		}
		if (kSpell.isCopyCastersPromotions())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_COPY_CASTERS_PROMOTIONS"));
		}
	}
	if (kSpell.getCreateUnitPromotion() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		const PromotionTypes eCreateUnitPromotion = (PromotionTypes)GC.getSpellInfo(eSpell).getCreateUnitPromotion();
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CREATE_UNIT_PROMOTION_NO_LINK", getLinkedText(eCreateUnitPromotion).c_str()));
	}
	if (kSpell.getDamage() != 0)
	{
		if (!kSpell.isTargetedHostile())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DAMAGE", kSpell.getDamage(), kSpell.getDamageLimit()));
		}
	}

	if (kSpell.isTargetedHostile())
	{
		if (kSpell.getDamage() != 0 && kSpell.getDamageType() != NO_DAMAGE)
		{
			int iManaBonus = 0;
			int iPromotionBonus = 0;
			int iBaseDamage = GC.getSpellInfo(eSpell).getDamage();
			if (pHeadSelectedUnit != NULL) {
				iManaBonus = GET_PLAYER(pHeadSelectedUnit->getOwner()).getBonusSpellDamage(eSpell);
				iPromotionBonus = pHeadSelectedUnit->getSpellDamageModify() / 10;
			}
			if (pHeadSelectedUnit == NULL || (iManaBonus == 0 && iPromotionBonus == 0))
			{
				//	szTempBuffer.Format(L"%c%d%% %s %s %d%%", gDLL->getSymbolID(BULLET_CHAR), GC.getSpellInfo(eSpell).getDamage(), GC.getDamageTypeInfo((DamageTypes)GC.getSpellInfo(eSpell).getDamageType()).getDescription(), gDLL->getText("TXT_KEY_SPELL_LIMITED_DAMAGE_DETAIL").c_str(),GC.getSpellInfo(eSpell).getDamageLimit());
				//	szBuffer.append(szTempBuffer);
				/*	if(kSpell.isManaCostReductionBonus() &&
						kSpell.getTechPrereq()!=NO_TECH && GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus()!=NO_BONUS)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_SPELL_BONUS_DAMAGE_MANA", getLinkedText((BonusTypes)GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus()).c_str()));
					}*/
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TARGETED_HOSTILE_DAMAGE_DETAIL", iBaseDamage + iManaBonus + iPromotionBonus, GC.getDamageTypeInfo((DamageTypes)GC.getSpellInfo(eSpell).getDamageType()).getDescription()));
			}
			else
			{
				szBuffer.append(pcNewline);
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TARGETED_HOSTILE_DAMAGE_DETAIL", iBaseDamage + iManaBonus + (GC.getSpellInfo(eSpell).getDamageType() != DAMAGE_PHYSICAL ? iPromotionBonus : 0), GC.getDamageTypeInfo((DamageTypes)GC.getSpellInfo(eSpell).getDamageType()).getDescription()));
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_BASE_DAMAGE", iBaseDamage));
				if (iManaBonus != 0) {
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TARGETED_HOSTILE_DAMAGE_DETAIL1", iManaBonus));
				}
				if (iPromotionBonus != 0 && GC.getSpellInfo(eSpell).getDamageType() != DAMAGE_PHYSICAL) {
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TARGETED_HOSTILE_DAMAGE_DETAIL2", iPromotionBonus));
				}
			}
		}

		if (kSpell.getDamageLimit() > 0 && kSpell.getDamageLimit() != 100) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_LIMITED_DAMAGE_DETAIL", kSpell.getDamageLimit()));
		}

		szBuffer.append(pcNewline);
		szTempBuffer.clear();
		szText2.clear();
		if (kSpell.getNumTargets() != -1)
		{
			int spellTargets = kSpell.getNumTargets();
			if (pHeadSelectedUnit != NULL) {
				spellTargets += ((spellTargets * pHeadSelectedUnit->getSpellTargetIncrease()) / 100);
				if (spellTargets < 1) {
					spellTargets = kSpell.getNumTargets();
				}
				szTempBuffer.Format(L"%d", spellTargets);

				if ((pHeadSelectedUnit->getSpellTargetIncrease() % 100) > 0)
					szText2.Format(L"-%d", ++spellTargets);
			}
		}
		else
		{
			szTempBuffer.Format(L"all");
		}
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TARGETED_HOSTILE", szTempBuffer.c_str(), szText2.c_str()));

		if (kSpell.isHighCrit())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_HIGHCRIT"));
		}

		if (kSpell.getDoT() != NO_PROMOTION)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DAMAGE_OVER_TIME"));
		}

		if (kSpell.getTriggerSecondaryPlotChance() > 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TRIGGER_SECONDARY_PLOT"));
		}

		if (kSpell.getForcedTeleport() > 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_FORCED_TELEPORT", GC.getSpellInfo(eSpell).getForcedTeleport()));
		}

		if (kSpell.getReduceDurability() > 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REDUCE_DURABILITY", GC.getSpellInfo(eSpell).getReduceDurability()));
		}

		if (kSpell.getDestroyEnemyEquipment() > 0)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DESTROY_ENEMY_EQUIPMENT_PEDIA", GC.getSpellInfo(eSpell).getDestroyEnemyEquipment()));
		}

		if (kSpell.getDamageSpellPrereqPromotion() != NO_PROMOTION)
		{
			const PromotionTypes eDamageSpellPrereqPromotion = (PromotionTypes)kSpell.getDamageSpellPrereqPromotion();
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DAMAGE_TARGET_PROMOTION", getLinkedText(eDamageSpellPrereqPromotion).c_str()));
		}

		if (kSpell.getDamageSpellPrereqUnitCombat() != NO_UNITCOMBAT)
		{
			const UnitCombatTypes eDamageSpellPrereqUnitCombat = (UnitCombatTypes)kSpell.getDamageSpellPrereqUnitCombat();
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DAMAGE_TARGET_UNITCOMBAT", getLinkedText(eDamageSpellPrereqUnitCombat).c_str()));
		}

		if (pHeadSelectedUnit != NULL && pHeadSelectedUnit->getSpellCastingRange() > 1)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELLCAST_RANGE", pHeadSelectedUnit->getSpellCastingRange()));
		}
	}

	if (kSpell.getAddPromotionType1() != NO_PROMOTION)
	{
		if (kSpell.isBuffCasterOnly())
		{
			szBuffer.append(pcNewline);
			const PromotionTypes eAddPromotion1 = (PromotionTypes)kSpell.getAddPromotionType1();
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_CASTER_NO_LINK", getLinkedText(eAddPromotion1).c_str()));
			if (kSpell.getAddPromotionType2() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				const PromotionTypes eAddPromotion2 = (PromotionTypes)kSpell.getAddPromotionType2();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_CASTER_NO_LINK", getLinkedText(eAddPromotion2).c_str()));
			}
			if (kSpell.getAddPromotionType3() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				const PromotionTypes eAddPromotion3 = (PromotionTypes)kSpell.getAddPromotionType3();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_CASTER_NO_LINK", getLinkedText(eAddPromotion3).c_str()));
			}
			if (kSpell.getAddPromotionType4() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				const PromotionTypes eAddPromotion4 = (PromotionTypes)kSpell.getAddPromotionType4();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_CASTER_NO_LINK", getLinkedText(eAddPromotion4).c_str()));
			}
			if (kSpell.getAddPromotionType5() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				const PromotionTypes eAddPromotion5 = (PromotionTypes)kSpell.getAddPromotionType5();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_CASTER_NO_LINK", getLinkedText(eAddPromotion5).c_str()));
			}
		}
		else
		{
			if (kSpell.getRange() == 0)
			{
				szBuffer.append(pcNewline);
				const PromotionTypes eAddPromotion1 = (PromotionTypes)kSpell.getAddPromotionType1();
				if (kSpell.getUnitCombatPrereq() == NO_UNITCOMBAT)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_NO_LINK", getLinkedText(eAddPromotion1).c_str()));
				}
				else
				{
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_REQ_UNITCOMBAT", getLinkedText(eAddPromotion1).c_str(), getLinkedText((UnitCombatTypes)GC.getSpellInfo(eSpell).getUnitCombatPrereq()).c_str()));
				}

				if (GC.getSpellInfo(eSpell).getAddPromotionType2() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType2()).getDescription()));
					const PromotionTypes eAddPromotion2 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType2();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_NO_LINK", getLinkedText(eAddPromotion2).c_str()));
					//<<<<	BUGFfH: End Modify
				}
				if (GC.getSpellInfo(eSpell).getAddPromotionType3() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3()).getDescription()));
					const PromotionTypes eAddPromotion3 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_NO_LINK", getLinkedText(eAddPromotion3).c_str()));
					//<<<<	BUGFfH: End Modify
				}
				if (GC.getSpellInfo(eSpell).getAddPromotionType4() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3()).getDescription()));
					const PromotionTypes eAddPromotion4 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType4();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_NO_LINK", getLinkedText(eAddPromotion4).c_str()));
					//<<<<	BUGFfH: End Modify
				}
				if (GC.getSpellInfo(eSpell).getAddPromotionType5() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3()).getDescription()));
					const PromotionTypes eAddPromotion5 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType5();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_NO_LINK", getLinkedText(eAddPromotion5).c_str()));
					//<<<<	BUGFfH: End Modify
				}
			}
			else
			{
				szBuffer.append(pcNewline);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType1()).getDescription(), GC.getSpellInfo(eSpell).getRange()));
				const PromotionTypes eAddPromotion1 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType1();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE_NO_LINK",
					getLinkedText(eAddPromotion1).c_str(),
					GC.getSpellInfo(eSpell).getRange()));
				//<<<<	BUGFfH: End Modify
				if (GC.getSpellInfo(eSpell).getAddPromotionType2() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType2()).getDescription(), GC.getSpellInfo(eSpell).getRange()));
					const PromotionTypes eAddPromotion2 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType2();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE_NO_LINK",
						getLinkedText(eAddPromotion2).c_str(),
						GC.getSpellInfo(eSpell).getRange()));
					//<<<<	BUGFfH: End Modify
				}
				if (GC.getSpellInfo(eSpell).getAddPromotionType3() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3()).getDescription(), GC.getSpellInfo(eSpell).getRange()));
					const PromotionTypes eAddPromotion3 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE_NO_LINK",
						getLinkedText(eAddPromotion3).c_str(),
						GC.getSpellInfo(eSpell).getRange()));
					//<<<<	BUGFfH: End Modify
				}
				if (GC.getSpellInfo(eSpell).getAddPromotionType4() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3()).getDescription(), GC.getSpellInfo(eSpell).getRange()));
					const PromotionTypes eAddPromotion4 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType4();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE_NO_LINK",
						getLinkedText(eAddPromotion4).c_str(),
						GC.getSpellInfo(eSpell).getRange()));
					//<<<<	BUGFfH: End Modify
				}
				if (GC.getSpellInfo(eSpell).getAddPromotionType5() != NO_PROMOTION)
				{
					szBuffer.append(pcNewline);
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType3()).getDescription(), GC.getSpellInfo(eSpell).getRange()));
					const PromotionTypes eAddPromotion5 = (PromotionTypes)GC.getSpellInfo(eSpell).getAddPromotionType5();
					szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADD_PROMOTION_AT_RANGE_NO_LINK",
						getLinkedText(eAddPromotion5).c_str(),
						GC.getSpellInfo(eSpell).getRange()));
					//<<<<	BUGFfH: End Modify
				}

			}
		}
	}
	if (GC.getSpellInfo(eSpell).getRemovePromotionType1() != NO_PROMOTION)
	{
		if (GC.getSpellInfo(eSpell).isBuffCasterOnly())
		{
			szBuffer.append(pcNewline);
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_CASTER", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType1()).getDescription()));
			const PromotionTypes eRemovePromotion1 = (PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType1();
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_CASTER_NO_LINK", getLinkedText(eRemovePromotion1).c_str()));
			//<<<<	BUGFfH: End Modify
			if (GC.getSpellInfo(eSpell).getRemovePromotionType2() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_CASTER", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType2()).getDescription()));
				const PromotionTypes eRemovePromotion2 = (PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType2();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_CASTER_NO_LINK", getLinkedText(eRemovePromotion2).c_str()));
				//<<<<	BUGFfH: End Modify
			}
			if (GC.getSpellInfo(eSpell).getRemovePromotionType3() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_CASTER", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType3()).getDescription()));
				const PromotionTypes eRemovePromotion3 = (PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType3();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_CASTER_NO_LINK", getLinkedText(eRemovePromotion3).c_str()));
				//<<<<	BUGFfH: End Modify
			}
		}
		else
		{
			szBuffer.append(pcNewline);
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType1()).getDescription()));
			const PromotionTypes eRemovePromotion1 = (PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType1();
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_NO_LINK", getLinkedText(eRemovePromotion1).c_str()));
			//<<<<	BUGFfH: End Modify
			if (GC.getSpellInfo(eSpell).getRemovePromotionType2() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION", GC.getPromotionInfo((PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType2()).getDescription()));
				const PromotionTypes eRemovePromotion2 = (PromotionTypes)GC.getSpellInfo(eSpell).getRemovePromotionType2();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_NO_LINK", getLinkedText(eRemovePromotion2).c_str()));
				//<<<<	BUGFfH: End Modify
			}
			if (kSpell.getRemovePromotionType3() != NO_PROMOTION)
			{
				szBuffer.append(pcNewline);
				const PromotionTypes eRemovePromotion3 = (PromotionTypes)kSpell.getRemovePromotionType3();
				szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_PROMOTION_NO_LINK", getLinkedText(eRemovePromotion3).c_str()));
			}
		}
	}

	if (kSpell.getConvertUnitType() != NO_UNIT)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CONVERT_UNIT_NO_LINK", getLinkedText((UnitTypes)kSpell.getConvertUnitType()).c_str()));
	}
	if (kSpell.getCreateBuildingType() != NO_BUILDING)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CREATE_BUILDING_NO_LINK", getLinkedText((BuildingTypes)kSpell.getCreateBuildingType()).c_str()));
	}
	if (kSpell.getCreateBuildingType1() != NO_BUILDING)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CREATE_BUILDING_NO_LINK", getLinkedText((BuildingTypes)kSpell.getCreateBuildingType1()).c_str()));
	}
	if (kSpell.getCreateBuildingType2() != NO_BUILDING)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CREATE_BUILDING_NO_LINK", getLinkedText((BuildingTypes)kSpell.getCreateBuildingType2()).c_str()));
	}

	if (kSpell.getRemoveBuildingType() != NO_BUILDING)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_BUILDING_NO_LINK", getLinkedText((BuildingTypes)kSpell.getRemoveBuildingType()).c_str()));
	}

	if (kSpell.getCreateFeatureType() != NO_FEATURE)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CREATE_FEATURE_NO_LINK", getLinkedText((FeatureTypes)kSpell.getCreateFeatureType()).c_str()));
	}
	if (kSpell.getCreateImprovementType() != NO_IMPROVEMENT)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CREATE_IMPROVEMENT_NO_LINK", getLinkedText((ImprovementTypes)kSpell.getCreateImprovementType()).c_str()));
	}
	if (kSpell.getSpreadReligion() != NO_RELIGION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_SPREAD_RELIGION_NO_LINK", getLinkedText((ReligionTypes)kSpell.getSpreadReligion()).c_str()));
	}
	if (kSpell.isDispel())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DISPEL"));
	}

	if (kSpell.getUnitCombatCapture() != NO_UNITCOMBAT)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_UNITCOMBAT_CAPTURE", getLinkedText((UnitCombatTypes)kSpell.getUnitCombatCapture()).c_str()));
	}

	if (kSpell.isAdjacentToWaterOnly())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_ADJACENT_TO_WATER_ONLY"));
	}
	if (kSpell.isInBordersOnly() && kSpell.isInCityOnly())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IN_BORDERS_AND_CITY_ONLY"));
	}
	else
	{
		if (kSpell.isInBordersOnly())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IN_BORDERS_ONLY"));
		}
		if (kSpell.isInCityOnly())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IN_CITY_ONLY"));
		}
	}
	/**
		if (kSpell.isImmuneTeam() && !kSpell.isImmuneNeutral() && !kSpell.isImmuneEnemy())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_TEAM"));
		}
		if (!kSpell.isImmuneTeam() && kSpell.isImmuneNeutral() && !kSpell.isImmuneEnemy())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_NEUTRAL"));
		}
		if (!kSpell.isImmuneTeam() && !kSpell.isImmuneNeutral() && kSpell.isImmuneEnemy())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_ENEMY"));
		}
		if (kSpell.isImmuneTeam() && kSpell.isImmuneNeutral() && !kSpell.isImmuneEnemy())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_TEAM_NEUTRAL"));
		}
		if (kSpell.isImmuneTeam() && !kSpell.isImmuneNeutral() && kSpell.isImmuneEnemy())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_TEAM_ENEMY"));
		}
		if (!kSpell.isImmuneTeam() && kSpell.isImmuneNeutral() && kSpell.isImmuneEnemy())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_NEUTRAL_ENEMY"));
		}
		if (kSpell.isImmuneTeam() && GC.getSpellInfo(eSpell).isImmuneNeutral() && GC.getSpellInfo(eSpell).isImmuneEnemy())
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_TEAM_NEUTRAL_ENEMY"));
		}
	**/
	if (kSpell.isImmuneFlying())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_FLYING"));
	}
	if (kSpell.isImmuneNotAlive())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMUNE_NOT_ALIVE"));
	}
	if (kSpell.isRemoveHasCasted())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_REMOVE_HAS_CASTED"));
	}
	int iCost = GC.getSpellInfo(eSpell).getCost();
	if (iCost != 0)
	{
		/*************************************************************************************************/
		/**	BUGFIX (Spell Costs scale with Gamespeed) Sephi                                	            **/
		/**																								**/
		/**						                                            							**/
		/*************************************************************************************************/
		iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
		iCost /= 100;
		/*************************************************************************************************/
		/**	END	                                        												**/
		/*************************************************************************************************/

		if (kSpell.getConvertUnitType() != NO_UNIT)
		{
			if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
			{
				iCost += (iCost * GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getUpgradeCostModifier()) / 100;
			}
		}
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_COST", iCost));
	}
	if (kSpell.getDelay() != 0)
	{
		/*************************************************************************************************/
		/**	BUGFIX (SpellDelay scales with Gamespeed) Sephi                                	            **/
		/*************************************************************************************************/
		//  	szBuffer.append(pcNewline);
		//  	szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DELAY", GC.getSpellInfo(eSpell).getDelay()));

		int iDelayTurns = kSpell.getDelay();
		iDelayTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
		iDelayTurns /= 100;
		iDelayTurns = std::max(iDelayTurns, 2);
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DELAY", iDelayTurns));
		/*************************************************************************************************/
		/**	END	                                        												**/
		/*************************************************************************************************/
	}
	if (kSpell.getMiscastChance() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_MISCAST_CHANCE", GC.getSpellInfo(eSpell).getMiscastChance()));
	}
	if (kSpell.getImmobileTurns() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMMOBILE_TURNS", GC.getSpellInfo(eSpell).getImmobileTurns()));
	}
	if (kSpell.isDomination())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_DOMINATION_EFFECT"));
	}
	if (kSpell.isSacrificeCaster())
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_SACRIFICE_CASTER"));
	}

	if (kSpell.getPrereqAlignment() != NO_ALIGNMENT)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_PREREQ_ALIGNMENT", GC.getAlignmentInfo((AlignmentTypes)GC.getSpellInfo(eSpell).getPrereqAlignment()).getDescription()));
	}
	if (kSpell.getCasterMinLevel() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CASTER_MIN_LEVEL", GC.getSpellInfo(eSpell).getCasterMinLevel()));
	}
	if (kSpell.getHeal() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_HEAL_PEDIA", kSpell.getHeal()));
		if (kSpell.isManaCostReductionBonus()
			&& kSpell.getTechPrereq() != NO_TECH && GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus() != NO_BONUS)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_HEAL_PER_BONUS", getLinkedText((BonusTypes)GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus()).c_str()));
		}
	}
	if (kSpell.getCivilizationPrereq() != NO_CIVILIZATION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CIVILIZATION_PREREQ_NO_LINK", getLinkedText((CivilizationTypes)GC.getSpellInfo(eSpell).getCivilizationPrereq()).c_str()));
	}
	if (kSpell.getTechPrereq() != NO_TECH)
	{
		if (pHeadSelectedUnit == NULL)	//only show in Pedia
		{
			szBuffer.append(pcNewline);
			const TechTypes eTechPrereq = (TechTypes)GC.getSpellInfo(eSpell).getTechPrereq();
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TECH_PREREQ_NO_LINK", getLinkedText(eTechPrereq).c_str()));
		}
	}
	if (kSpell.getReligionPrereq() != NO_RELIGION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_RELIGION_PREREQ", getLinkedText((ReligionTypes)GC.getSpellInfo(eSpell).getReligionPrereq()).c_str()));
	}
	if (kSpell.getStateReligionPrereq() != NO_RELIGION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_STATE_RELIGION_PREREQ", getLinkedText((ReligionTypes)GC.getSpellInfo(eSpell).getStateReligionPrereq()).c_str()));
	}
	if (kSpell.getCorporationPrereq() != NO_CORPORATION)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_CORPORATION_PREREQ", getLinkedText((CorporationTypes)GC.getSpellInfo(eSpell).getCorporationPrereq()).c_str()));
	}
	if (kSpell.getBuildingClassOwnedPrereq() != NO_BUILDINGCLASS)
	{
		szBuffer.append(pcNewline);
		const BuildingClassTypes eBuildingClassOwnedPrereq = (BuildingClassTypes)GC.getSpellInfo(eSpell).getBuildingClassOwnedPrereq();
		const BuildingTypes eBuildingOwnedPrereq = (BuildingTypes)GC.getBuildingClassInfo(eBuildingClassOwnedPrereq).getDefaultBuildingIndex();
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_BUILDINGCLASS_OWNED_PREREQ_NO_LINK", getLinkedText(eBuildingOwnedPrereq).c_str()));
	}
	if (kSpell.getBuildingPrereq() != NO_BUILDING)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_BUILDING_PREREQ_NO_LINK", getLinkedText((BuildingTypes)GC.getSpellInfo(eSpell).getBuildingPrereq()).c_str()));
	}
	if (kSpell.getUnitCombatPrereq() != NO_UNITCOMBAT)
	{
		szBuffer.append(pcNewline);
		const UnitCombatTypes eUnitCombatTypePrereq = (UnitCombatTypes)GC.getSpellInfo(eSpell).getUnitCombatPrereq();
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_UNIT_PREREQ_NO_LINK", getLinkedText(eUnitCombatTypePrereq).c_str()));
	}
	if (kSpell.getUnitClassPrereq() != NO_UNITCLASS)
	{
		szBuffer.append(pcNewline);
		const UnitClassTypes eUnitClassPrereq = (UnitClassTypes)GC.getSpellInfo(eSpell).getUnitClassPrereq();
		const UnitTypes eDefaultUnitTypesPrereq = (UnitTypes)GC.getUnitClassInfo(eUnitClassPrereq).getDefaultUnitIndex();
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_UNIT_PREREQ_NO_LINK",
			getLinkedText(eDefaultUnitTypesPrereq).c_str(),
			getLinkedText(eUnitClassPrereq).c_str()));
	}
	if (kSpell.getUnitPrereq() != NO_UNIT)
	{
		szBuffer.append(pcNewline);
		const UnitTypes eUnitTypePrereq = (UnitTypes)GC.getSpellInfo(eSpell).getUnitPrereq();
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_UNIT_PREREQ_NO_LINK", getLinkedText(eUnitTypePrereq).c_str()));
	}
	if (kSpell.getUnitInStackPrereq() != NO_UNIT)
	{
		szBuffer.append(pcNewline);
		const UnitTypes eUnitInStackPrereq = (UnitTypes)GC.getSpellInfo(eSpell).getUnitInStackPrereq();
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_UNIT_IN_STACK_PREREQ", getLinkedText(eUnitInStackPrereq).c_str()));
	}
	if (kSpell.getPromotionInStackPrereq() != NO_PROMOTION)
	{
		szBuffer.append(pcNewline);
		const PromotionTypes ePromotionPrereq = (PromotionTypes)GC.getSpellInfo(eSpell).getPromotionInStackPrereq();
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_PROMOTION_IN_STACK_PREREQ_NO_LINK", getLinkedText(ePromotionPrereq).c_str()));
	}
	if (kSpell.getTerrainOrPrereq1() != NO_TERRAIN)
	{
		if (kSpell.getTerrainOrPrereq2() == NO_TERRAIN)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TERRAIN_PREREQ_NO_LINK",
				getLinkedText((TerrainTypes)GC.getSpellInfo(eSpell).getTerrainOrPrereq1()).c_str()));
		}
		else
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_TERRAIN2_PREREQ_NO_LINK",
				getLinkedText((TerrainTypes)GC.getSpellInfo(eSpell).getTerrainOrPrereq1()).c_str(),
				getLinkedText((TerrainTypes)GC.getSpellInfo(eSpell).getTerrainOrPrereq2()).c_str()));
		}
	}
	if (GC.getSpellInfo(eSpell).getFeatureOrPrereq1() != NO_FEATURE)
	{
		if (GC.getSpellInfo(eSpell).getFeatureOrPrereq2() == NO_FEATURE)
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_FEATURE_PREREQ_NO_LINK",
				getLinkedText((FeatureTypes)GC.getSpellInfo(eSpell).getFeatureOrPrereq1()).c_str()));
		}
		else
		{
			szBuffer.append(pcNewline);
			szBuffer.append(gDLL->getText("TXT_KEY_SPELL_FEATURE2_PREREQ_NO_LINK",
				getLinkedText((FeatureTypes)GC.getSpellInfo(eSpell).getFeatureOrPrereq1()).c_str(),
				getLinkedText((FeatureTypes)GC.getSpellInfo(eSpell).getFeatureOrPrereq2()).c_str()));
		}
	}
	if (GC.getSpellInfo(eSpell).getImprovementPrereq() != NO_IMPROVEMENT)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELL_IMPROVEMENT_PREREQ",
			getLinkedText((ImprovementTypes)GC.getSpellInfo(eSpell).getImprovementPrereq()).c_str()));
	}

	if (kSpell.getChangePopulation() != 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(kSpell.getChangePopulation() > 0 ? gDLL->getText("TXT_KEY_SPELL_CHANGE_POPULATION_ADD", kSpell.getChangePopulation()) : 
			gDLL->getText("TXT_KEY_SPELL_CHANGE_POPULATION_REMOVE", -1 * kSpell.getChangePopulation()));
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/
	if (wcslen(kSpell.getHelp()) > 0)
	{
		szBuffer.append(pcNewline);
		szBuffer.append(kSpell.getHelp());
	}
}
//FfH: End Add

//	Function:			parseCivicInfo()
//	Description:	Will parse the civic info help
//	Parameters:		szHelpText -- the text to put it into
//								civicInfo - what to parse
//	Returns:			nothing
void CvGameTextMgr::parseCivicInfo(CvWStringBuffer& szHelpText, CivicTypes eCivic, bool bCivilopediaText, bool bPlayerContext, bool bSkipName)
{
	PROFILE_FUNC();

	CvWString szFirstBuffer;
	bool bFound;
	bool bFirst;
	int iLast;
	//>>>>	BUGFfH: Modified by Denev 2009/09/27
	//	int iI, iJ;
	int iI;
	//<<<<	BUGFfH: End Modify

	if (NO_CIVIC == eCivic)
	{
		return;
	}

	szHelpText.clear();

	FAssert(GC.getGameINLINE().getActivePlayer() != NO_PLAYER || !bPlayerContext);

	if (!bSkipName)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/03
		//		szHelpText.append(GC.getCivicInfo(eCivic).getDescription());
		szHelpText.append(getLinkedText(eCivic));
		//<<<<	BUGFfH: End Modify
	}

	if (!bCivilopediaText)
	{
		if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canDoCivics(eCivic)))
		{
			if (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)(GC.getCivicInfo(eCivic).getTechPrereq()))))
			{
				if (GC.getCivicInfo(eCivic).getTechPrereq() != NO_TECH)
				{
					szHelpText.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/03
					//					szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", getLinkedText((TechTypes)GC.getCivicInfo(eCivic).getTechPrereq()).c_str()));
					szHelpText.append(gDLL->getText("TXT_KEY_REQUIRES"));
					szHelpText.append(getLinkedText((TechTypes)GC.getCivicInfo(eCivic).getTechPrereq(), false));
					szHelpText.append(ENDCOLR);
					//<<<<	BUGFfH: End Modify
				}
			}
			//>>>>	BUGFfH: Moved from below(*16) by Denev 2009/09/22
			if (GC.getCivicInfo(eCivic).getPrereqCivilization() != NO_CIVILIZATION)
			{
				if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType() != GC.getCivicInfo(eCivic).getPrereqCivilization())
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_REQUIRES"));
					szHelpText.append(getLinkedText((CivilizationTypes)GC.getCivicInfo(eCivic).getPrereqCivilization(), false));
					szHelpText.append(ENDCOLR);
				}
			}
			if (GC.getCivicInfo(eCivic).getPrereqReligion() != NO_RELIGION)
			{
				if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != GC.getCivicInfo(eCivic).getPrereqReligion())
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_MISC_REQ_STATE_RELIGION", GC.getReligionInfo((ReligionTypes)GC.getCivicInfo(eCivic).getPrereqReligion()).getChar()));
				}
			}
			if (GC.getCivicInfo(eCivic).getPrereqAlignment() != NO_ALIGNMENT)
			{
				if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getAlignment() != GC.getCivicInfo(eCivic).getPrereqAlignment())
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_REQUIRES"));
					szHelpText.append(GC.getAlignmentInfo((AlignmentTypes)GC.getCivicInfo(eCivic).getPrereqAlignment()).getDescription());
					szHelpText.append(ENDCOLR);
				}
			}
			if (GC.getCivicInfo(eCivic).getBlockAlignment() != NO_ALIGNMENT)
			{
				if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getAlignment() == GC.getCivicInfo(eCivic).getBlockAlignment())
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BLOCKS", GC.getAlignmentInfo((AlignmentTypes)GC.getCivicInfo(eCivic).getBlockAlignment()).getDescription()));
				}
			}

			if (bPlayerContext && GC.getLeaderHeadInfo((LeaderHeadTypes)GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getLeaderType()).getHatedCivic() == eCivic)
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_HATED_BLOCKED"));
				return;
			}

			if (GC.getCivicInfo(eCivic).isPrereqWar())
			{
				if (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).getAtWarCount(true) > 0))
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_PREREQ_WAR"));
				}
			}
			//<<<<	BUGFfH: End Move
		}
	}

	// Special Building Not Required...
	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); ++iI)
	{
		if (GC.getCivicInfo(eCivic).isSpecialBuildingNotRequired(iI))
		{
			// XXX "Missionaries"??? - Now in XML
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BUILD_MISSIONARIES", GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getTextKeyWide()));
		}
	}

	// Valid Specialists...

	bFirst = true;

	for (iI = 0; iI < GC.getNumSpecialistInfos(); ++iI)
	{
		if (GC.getCivicInfo(eCivic).isSpecialistValid(iI))
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_UNLIMTED").c_str());
			CvWString szSpecialist;
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szSpecialist.Format(L"<link=literal>%s</link>", GC.getSpecialistInfo((SpecialistTypes)iI).getDescription());
			szSpecialist = getLinkedText((SpecialistTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szHelpText, szFirstBuffer, szSpecialist, L", ", bFirst);
			bFirst = false;
		}
	}

	//	Great People Modifier...
	if (GC.getCivicInfo(eCivic).getGreatPeopleRateModifier() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_GREAT_PEOPLE_MOD", GC.getCivicInfo(eCivic).getGreatPeopleRateModifier()));
	}

	//	Great General Modifier...
	if (GC.getCivicInfo(eCivic).getGreatGeneralRateModifier() != 0)
	{
		szHelpText.append(NEWLINE);
		//		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_GREAT_GENERAL_MOD", GC.getCivicInfo(eCivic).getGreatGeneralRateModifier()));
		if (GC.getCivicInfo(eCivic).getGreatGeneralRateModifier() > 0)
		{
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_GREAT_GENERAL_ENABLED"));
		}
	}

	if (GC.getCivicInfo(eCivic).getDomesticGreatGeneralRateModifier() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_DOMESTIC_GREAT_GENERAL_MODIFIER", GC.getCivicInfo(eCivic).getDomesticGreatGeneralRateModifier()));
	}

	//	State Religion Great People Modifier...
	if (GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_GREAT_PEOPLE_MOD_RELIGION", GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier(), GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar()));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_GREAT_PEOPLE_MOD_STATE_RELIGION", GC.getCivicInfo(eCivic).getStateReligionGreatPeopleRateModifier(), gDLL->getSymbolID(RELIGION_CHAR)));
		}
	}

	//	Distance Maintenance Modifer...
	if (GC.getCivicInfo(eCivic).getDistanceMaintenanceModifier() != 0)
	{
		if (GC.getCivicInfo(eCivic).getDistanceMaintenanceModifier() <= -100)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_DISTANCE_MAINT"));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_DISTANCE_MAINT_MOD", GC.getCivicInfo(eCivic).getDistanceMaintenanceModifier()));
		}
	}

	//	Num Cities Maintenance Modifer...
	if (GC.getCivicInfo(eCivic).getNumCitiesMaintenanceModifier() != 0)
	{
		if (GC.getCivicInfo(eCivic).getNumCitiesMaintenanceModifier() <= -100)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_MAINT_NUM_CITIES"));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_MAINT_NUM_CITIES_MOD", GC.getCivicInfo(eCivic).getNumCitiesMaintenanceModifier()));
		}
	}

	//	Corporations Maintenance Modifer...
	if (GC.getCivicInfo(eCivic).getCorporationMaintenanceModifier() != 0)
	{
		if (GC.getCivicInfo(eCivic).getCorporationMaintenanceModifier() <= -100)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_MAINT_CORPORATION"));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_MAINT_CORPORATION_MOD", GC.getCivicInfo(eCivic).getCorporationMaintenanceModifier()));
		}
	}

	//	Extra Health
	if (GC.getCivicInfo(eCivic).getExtraHealth() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_EXTRA_HEALTH", abs(GC.getCivicInfo(eCivic).getExtraHealth()), ((GC.getCivicInfo(eCivic).getExtraHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR))));
	}

	//	Free Experience
	if (GC.getCivicInfo(eCivic).getFreeExperience() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_FREE_XP", GC.getCivicInfo(eCivic).getFreeExperience()));
	}

	//	Worker speed modifier
	if (GC.getCivicInfo(eCivic).getWorkerSpeedModifier() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_WORKER_SPEED", GC.getCivicInfo(eCivic).getWorkerSpeedModifier()));
	}

	//	Improvement upgrade rate modifier
	if (GC.getCivicInfo(eCivic).getImprovementUpgradeRateModifier() != 0)
	{
		bFirst = true;

		for (iI = 0; iI < GC.getNumImprovementInfos(); ++iI)
		{
			if (GC.getImprovementInfo((ImprovementTypes)iI).getImprovementUpgrade() != NO_IMPROVEMENT)
			{
				if (!GC.getImprovementInfo((ImprovementTypes)iI).isGraphicalOnly()
					&& (GC.getImprovementInfo((ImprovementTypes)iI).getPrereqCivilization() == NO_CIVILIZATION
						|| (GC.getGame().getActiveCivilizationType() == GC.getImprovementInfo((ImprovementTypes)iI).getPrereqCivilization()))) {

					szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_IMPROVEMENT_UPGRADE", GC.getCivicInfo(eCivic).getImprovementUpgradeRateModifier()).c_str());
					CvWString szImprovement;
					szImprovement = getLinkedText((ImprovementTypes)iI);
					setListHelp(szHelpText, szFirstBuffer, szImprovement, L", ", bFirst);
					bFirst = false;
				}
			}
		}
	}

	//	Military unit production modifier
	if (GC.getCivicInfo(eCivic).getMilitaryProductionModifier() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_MILITARY_PRODUCTION", GC.getCivicInfo(eCivic).getMilitaryProductionModifier()));
	}

	//	Free units population percent
	if ((GC.getCivicInfo(eCivic).getBaseFreeUnits() != 0) || (GC.getCivicInfo(eCivic).getFreeUnitsPopulationPercent() != 0))
	{
		if (bPlayerContext)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_FREE_UNITS", (GC.getCivicInfo(eCivic).getBaseFreeUnits() + ((GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTotalPopulation() * GC.getCivicInfo(eCivic).getFreeUnitsPopulationPercent()) / 100))));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_UNIT_SUPPORT"));
		}
	}

	//Sephi - Unit Cost disabled
	//	Free military units population percent
	if ((GC.getCivicInfo(eCivic).getFreeMilitaryUnitsPopulationPercent() != 0))
	{
		if (bPlayerContext)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_INCREASED_UNIT_LIMIT_CONTEXT",
				(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getNumCities() * GC.getCivicInfo(eCivic).getFreeMilitaryUnitsPopulationPercent())));
		}
	}
	//Sephi - Unit Cost disabled

	if (GC.getCivicInfo(eCivic).getFreeMilitaryUnitsPopulationPercent() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_INCREASED_UNIT_LIMIT", GC.getCivicInfo(eCivic).getFreeMilitaryUnitsPopulationPercent()));
	}

	//	Happiness per military unit
	if (GC.getCivicInfo(eCivic).getHappyPerMilitaryUnit() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_UNIT_HAPPINESS", GC.getCivicInfo(eCivic).getHappyPerMilitaryUnit(), ((GC.getCivicInfo(eCivic).getHappyPerMilitaryUnit() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
	}

	//	Military units produced with food
	if (GC.getCivicInfo(eCivic).isMilitaryFoodProduction())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_MILITARY_FOOD"));
	}

	//	Conscription
	if (getWorldSizeMaxConscript(eCivic) != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_CONSCRIPTION", getWorldSizeMaxConscript(eCivic)));
	}

	//	Population Unhealthiness
	if (GC.getCivicInfo(eCivic).isNoUnhealthyPopulation())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_POP_UNHEALTHY"));
	}

	//	Building Unhealthiness
	if (GC.getCivicInfo(eCivic).isBuildingOnlyHealthy())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_BUILDING_UNHEALTHY"));
	}

	//	Population Unhealthiness
	if (0 != GC.getCivicInfo(eCivic).getExpInBorderModifier())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_EXPERIENCE_IN_BORDERS", GC.getCivicInfo(eCivic).getExpInBorderModifier()));
	}

	//	War Weariness
	if (GC.getCivicInfo(eCivic).getWarWearinessModifier() != 0)
	{
		if (GC.getCivicInfo(eCivic).getWarWearinessModifier() <= -100)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_WAR_WEARINESS"));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_EXTRA_WAR_WEARINESS", GC.getCivicInfo(eCivic).getWarWearinessModifier()));
		}
	}

	//	Free specialists
	if (GC.getCivicInfo(eCivic).getFreeSpecialist() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_FREE_SPECIALISTS", GC.getCivicInfo(eCivic).getFreeSpecialist()));
	}

	//	Trade routes
	if (GC.getCivicInfo(eCivic).getTradeRoutes() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_TRADE_ROUTES", GC.getCivicInfo(eCivic).getTradeRoutes()));
	}

	//	No Foreign Trade
	if (GC.getCivicInfo(eCivic).isNoForeignTrade())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_FOREIGN_TRADE"));
	}

	//	No Corporations
	if (GC.getCivicInfo(eCivic).isNoCorporations())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_CORPORATIONS"));
	}

	//	No Foreign Corporations
	if (GC.getCivicInfo(eCivic).isNoForeignCorporations())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_FOREIGN_CORPORATIONS"));
	}

	//	Freedom Anger
	if (GC.getCivicInfo(eCivic).getCivicPercentAnger() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_FREEDOM_ANGER", GC.getCivicInfo(eCivic).getTextKeyWide()));
	}

	if (!(GC.getCivicInfo(eCivic).isStateReligion()))
	{
		bFound = false;

		for (iI = 0; iI < GC.getNumCivicInfos(); ++iI)
		{
			if ((GC.getCivicInfo((CivicTypes)iI).getCivicOptionType() == GC.getCivicInfo(eCivic).getCivicOptionType()) && (GC.getCivicInfo((CivicTypes)iI).isStateReligion()))
			{
				bFound = true;
			}
		}

		if (bFound)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_STATE_RELIGION"));
		}
	}

	if (GC.getCivicInfo(eCivic).getStateReligionHappiness() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_STATE_RELIGION_HAPPINESS", abs(GC.getCivicInfo(eCivic).getStateReligionHappiness()), ((GC.getCivicInfo(eCivic).getStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar()));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_RELIGION_HAPPINESS", abs(GC.getCivicInfo(eCivic).getStateReligionHappiness()), ((GC.getCivicInfo(eCivic).getStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
		}
	}

	if (GC.getCivicInfo(eCivic).getNonStateReligionHappiness() != 0)
	{
		if (GC.getCivicInfo(eCivic).isStateReligion())
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NON_STATE_REL_HAPPINESS_NO_STATE"));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NON_STATE_REL_HAPPINESS_WITH_STATE", abs(GC.getCivicInfo(eCivic).getNonStateReligionHappiness()), ((GC.getCivicInfo(eCivic).getNonStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
		}
	}

	//	State Religion Unit Production Modifier
	if (GC.getCivicInfo(eCivic).getStateReligionUnitProductionModifier() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REL_TRAIN_BONUS", GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar(), GC.getCivicInfo(eCivic).getStateReligionUnitProductionModifier()));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_STATE_REL_TRAIN_BONUS", GC.getCivicInfo(eCivic).getStateReligionUnitProductionModifier()));
		}
	}

	//	State Religion Building Production Modifier
	if (GC.getCivicInfo(eCivic).getStateReligionBuildingProductionModifier() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REL_BUILDING_BONUS", GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar(), GC.getCivicInfo(eCivic).getStateReligionBuildingProductionModifier()));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_STATE_REL_BUILDING_BONUS", GC.getCivicInfo(eCivic).getStateReligionBuildingProductionModifier()));
		}
	}

	//	State Religion Free Experience
	if (GC.getCivicInfo(eCivic).getStateReligionFreeExperience() != 0)
	{
		if (bPlayerContext && (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != NO_RELIGION))
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REL_FREE_XP", GC.getCivicInfo(eCivic).getStateReligionFreeExperience(), GC.getReligionInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion()).getChar()));
		}
		else
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_STATE_REL_FREE_XP", GC.getCivicInfo(eCivic).getStateReligionFreeExperience()));
		}
	}

	if (GC.getCivicInfo(eCivic).isNoNonStateReligionSpread())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_NON_STATE_SPREAD"));
	}

	//	Yield Modifiers
	setYieldChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_ALL_CITIES").GetCString(), GC.getCivicInfo(eCivic).getYieldModifierArray(), true);

	//	Capital Yield Modifiers
	setYieldChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_CAPITAL").GetCString(), GC.getCivicInfo(eCivic).getCapitalYieldModifierArray(), true);

	//	Trade Yield Modifiers
	setYieldChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_FROM_TRADE_ROUTES").GetCString(), GC.getCivicInfo(eCivic).getTradeYieldModifierArray(), true);

	//	Commerce Modifier
	setCommerceChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_ALL_CITIES").GetCString(), GC.getCivicInfo(eCivic).getCommerceModifierArray(), true);

	//	Capital Commerce Modifiers
	setCommerceChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_CAPITAL").GetCString(), GC.getCivicInfo(eCivic).getCapitalCommerceModifierArray(), true);

	//	Specialist Commerce
	setCommerceChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_PER_SPECIALIST").GetCString(), GC.getCivicInfo(eCivic).getSpecialistExtraCommerceArray());

	//	Largest City Happiness
	if (GC.getCivicInfo(eCivic).getLargestCityHappiness() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_LARGEST_CITIES_HAPPINESS", GC.getCivicInfo(eCivic).getLargestCityHappiness(), ((GC.getCivicInfo(eCivic).getLargestCityHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getTargetNumCities()));
	}

	//	Improvement Yields
//>>>>	BUGFfH: Modified by Denev 2009/09/22
/*
	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		iLast = 0;

		for (iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++)
		{
			if (GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI) != 0)
			{
				szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_IMPROVEMENT_YIELD_CHANGE", GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI), GC.getYieldInfo((YieldTypes)iI).getChar()).c_str());
				CvWString szImprovement;
				szImprovement.Format(L"<link=literal>%s</link>", GC.getImprovementInfo((ImprovementTypes)iJ).getDescription());
				setListHelp(szHelpText, szFirstBuffer, szImprovement, L", ", (GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI) != iLast));
				iLast = GC.getCivicInfo(eCivic).getImprovementYieldChanges(iJ, iI);
			}
		}
	}
*/
	for (int iImprovement = 0; iImprovement < GC.getNumImprovementInfos(); iImprovement++)
	{
		int aiYieldAmount[NUM_YIELD_TYPES];
		bool bYields = false;
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
		{
			aiYieldAmount[iYield] = GC.getCivicInfo(eCivic).getImprovementYieldChanges((ImprovementTypes)iImprovement, iYield);
			bYields = bYields || aiYieldAmount[iYield] != 0;
		}
		if (bYields)
		{
			setYieldChangeHelp(szHelpText, L"", L"",
				gDLL->getText("TXT_KEY_CIVIC_YIELD_FROM_IMPROVEMENT", getLinkedText((ImprovementTypes)iImprovement).c_str()),
				aiYieldAmount);
		}
	}
	//<<<<	BUGFfH: End Modify

	if (GC.getCivicInfo(eCivic).getImpInfrastructureHalfCost() != NO_IMPROVEMENT)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_IMPINFRASTRUCECOST_REDUCED", getLinkedText((ImprovementTypes)GC.getCivicInfo(eCivic).getImpInfrastructureHalfCost()).c_str()));
	}

	if (GC.getCivicInfo(eCivic).getNationalism() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_NATIONALISM", GC.getCivicInfo(eCivic).getNationalism()));
	}

	if (GC.getCivicInfo(eCivic).isPermanent())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_PERMANENT"));
	}

	if (GC.getCivicInfo(eCivic).isBanditLords())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_BANDIT_LORDS"));
	}

	if (GC.getCivicInfo(eCivic).isLumberFromAF())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_LUMBER_FROM_AF"));
	}

	if (GC.getCivicInfo(eCivic).getReducedEquipmentCost() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_EQUIPMENTCOST", GC.getCivicInfo(eCivic).getReducedEquipmentCost()));
	}

	if (GC.getCivicInfo(eCivic).getGreatPersonRatePerCulture() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_GPP_PER_CULTURE", GC.getCivicInfo(eCivic).getGreatPersonRatePerCulture()));
	}

	if (GC.getCivicInfo(eCivic).getHealthRatePerCulture() != 0)
	{
		CvWString szHealthString;
		if (GC.getCivicInfo(eCivic).getHealthRatePerCulture() * 0.01f == int(GC.getCivicInfo(eCivic).getHealthRatePerCulture() * 0.01f))
		{
			szHealthString.Format(L"%.0f", 0.01f * GC.getCivicInfo(eCivic).getHealthRatePerCulture());
		}
		else
		{
			szHealthString.Format(L"%.2f", 0.01f * GC.getCivicInfo(eCivic).getHealthRatePerCulture());
		}
		szHelpText.append(NEWLINE);
		szHelpText.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
		szHelpText.append(CvWString::format(L"%s", szHealthString.c_str()));
		szHelpText.append(CvWString::format(L"%c", gDLL->getSymbolID(HEALTHY_CHAR)));
		szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_COMMERCE_PER_CULTURE"));

	}

	bFirst = false;
	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getBuildingInfo((BuildingTypes)iI).getPrereqCivic() == eCivic)
		{
			if (!bFirst)
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_ALLOW_BUILDING"));
				bFirst = true;
			}
			else
			{
				szHelpText.append(L", ");
			}
			szHelpText.append(getLinkedText((BuildingTypes)iI));
		}
	}

	for (iI = 0; iI < GC.getNUM_COMMERCE_TYPES(); iI++)
	{
		if (GC.getCivicInfo(eCivic).getCommerceChangesPerCultureLevel(iI) != 0)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szHelpText.append(CvWString::format(L"%d", GC.getCivicInfo(eCivic).getCommerceChangesPerCultureLevel(iI)));
			szHelpText.append(CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes)iI).getChar()));
			szHelpText.append(gDLL->getText("TXT_KEY_HELP_CIVIC_COMMERCE_PER_CULTURE"));
		}
	}

	//	Building Happiness
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		if (GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI) != 0)
		{
			if (bPlayerContext && NO_PLAYER != GC.getGameINLINE().getActivePlayer())
			{
				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);
				if (NO_BUILDING != eBuilding)
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BUILDING_HAPPINESS", GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI), ((GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getBuildingInfo(eBuilding).getTextKeyWide()));
				}
			}
			else
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BUILDING_HAPPINESS", GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI), ((GC.getCivicInfo(eCivic).getBuildingHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getBuildingClassInfo((BuildingClassTypes)iI).getTextKeyWide()));
			}
		}

		if (GC.getCivicInfo(eCivic).getBuildingHealthChanges(iI) != 0)
		{
			if (bPlayerContext && NO_PLAYER != GC.getGameINLINE().getActivePlayer())
			{
				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);
				if (NO_BUILDING != eBuilding)
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BUILDING_HAPPINESS", GC.getCivicInfo(eCivic).getBuildingHealthChanges(iI), ((GC.getCivicInfo(eCivic).getBuildingHealthChanges(iI) > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)), GC.getBuildingInfo(eBuilding).getTextKeyWide()));
				}
			}
			else
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BUILDING_HAPPINESS", GC.getCivicInfo(eCivic).getBuildingHealthChanges(iI), ((GC.getCivicInfo(eCivic).getBuildingHealthChanges(iI) > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)), GC.getBuildingClassInfo((BuildingClassTypes)iI).getTextKeyWide()));
			}
		}
	}

	//	Feature Happiness
	iLast = 0;

	for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
	{
		if (GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) != 0)
		{
			CvWString szHappyString;
			if (GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) * 0.01f == int(GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) * 0.01f))
			{
				szHappyString.Format(L"%.0f", 0.01f * GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI));
			}
			else
			{
				szHappyString.Format(L"%.2f", 0.01f * GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI));
			}
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_CIVIC_FEATURE_HAPPINESS", szHappyString.GetCString(), ((GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))).c_str());
			CvWString szFeature;
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szFeature.Format(L"<link=literal>%s</link>", GC.getFeatureInfo((FeatureTypes)iI).getDescription());
			szFeature = getLinkedText((FeatureTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szHelpText, szFirstBuffer, szFeature, L", ", (GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI) != iLast));
			iLast = GC.getCivicInfo(eCivic).getFeatureHappinessChanges(iI);
		}
	}

	//	Hurry types
	for (iI = 0; iI < GC.getNumHurryInfos(); ++iI)
	{
		if (GC.getCivicInfo(eCivic).isHurry(iI))
		{
			szHelpText.append(CvWString::format(L"%s%c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), GC.getHurryInfo((HurryTypes)iI).getDescription()));
		}
	}

	//>>>>	BUGFfH: Moved to above(*16) by Denev 2009/09/22
	/*
	//FfH: Added by Kael 08/11/2007
		if (GC.getCivicInfo(eCivic).isPrereqWar())
		{
			if (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).getAtWarCount(true) > 0))
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_PREREQ_WAR"));
			}
		}
		if (GC.getCivicInfo(eCivic).getBlockAlignment() != NO_ALIGNMENT)
		{
			if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getAlignment() == GC.getCivicInfo(eCivic).getBlockAlignment())
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BLOCKS", GC.getAlignmentInfo((AlignmentTypes)GC.getCivicInfo(eCivic).getBlockAlignment()).getDescription()));
			}
		}
		if (GC.getCivicInfo(eCivic).getPrereqAlignment() != NO_ALIGNMENT)
		{
			if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getAlignment() != GC.getCivicInfo(eCivic).getPrereqAlignment())
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getAlignmentInfo((AlignmentTypes)GC.getCivicInfo(eCivic).getPrereqAlignment()).getDescription()));
			}
		}
		if (GC.getCivicInfo(eCivic).getPrereqCivilization() != NO_CIVILIZATION)
		{
			if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType() != GC.getCivicInfo(eCivic).getPrereqCivilization())
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getCivilizationInfo((CivilizationTypes)GC.getCivicInfo(eCivic).getPrereqCivilization()).getDescription()));
			}
		}
		if (GC.getCivicInfo(eCivic).getPrereqReligion() != NO_RELIGION)
		{
			if (!bPlayerContext || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != GC.getCivicInfo(eCivic).getPrereqReligion())
			{
				szHelpText.append(NEWLINE);
				szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getReligionInfo((ReligionTypes)GC.getCivicInfo(eCivic).getPrereqReligion()).getDescription()));
			}
		}
	*/
	//<<<<	BUGFfH: End Move
	if (GC.getCivicInfo(eCivic).isNoDiplomacyWithEnemies())
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_NO_DIPLOMACY_WITH_ENEMIES"));
	}
	if (GC.getCivicInfo(eCivic).getCoastalTradeRoutes() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_COASTAL_TRADE_ROUTES", GC.getCivicInfo(eCivic).getCoastalTradeRoutes()));
	}
	if (GC.getCivicInfo(eCivic).getEnslavementChance() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_UNIT_ENSLAVEMENT_CHANCE", GC.getCivicInfo(eCivic).getEnslavementChance()));
	}
	if (GC.getCivicInfo(eCivic).getFoodConsumptionPerPopulation() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_FOOD_CONSUMPTION_PER_POPULATION", GC.getCivicInfo(eCivic).getFoodConsumptionPerPopulation(), GC.getFOOD_CONSUMPTION_PER_POPULATION()));
	}
	//FfH: End Add

		//	Gold cost per unit
	if (GC.getCivicInfo(eCivic).getGoldPerUnit() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_SUPPORT_COSTS", (GC.getCivicInfo(eCivic).getGoldPerUnit() > 0), GC.getCommerceInfo(COMMERCE_GOLD).getChar()));
	}

	//	Gold cost per military unit
	if (GC.getCivicInfo(eCivic).getGoldPerMilitaryUnit() != 0)
	{
		szHelpText.append(NEWLINE);
		szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_MILITARY_SUPPORT_COSTS", (GC.getCivicInfo(eCivic).getGoldPerMilitaryUnit() > 0), GC.getCommerceInfo(COMMERCE_GOLD).getChar()));
	}

	if (!CvWString(GC.getCivicInfo(eCivic).getHelp()).empty())
	{
		szHelpText.append(CvWString::format(L"%s%s", NEWLINE, GC.getCivicInfo(eCivic).getHelp()).c_str());
	}
	//>>>>	BUGFfH: Added by Denev 2009/09/22
	if (bCivilopediaText)
	{
		if (GC.getCivicInfo(eCivic).isPrereqWar())
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_PREREQ_WAR"));
		}
		if (GC.getCivicInfo(eCivic).getBlockAlignment() != NO_ALIGNMENT)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_BLOCKS", GC.getAlignmentInfo((AlignmentTypes)GC.getCivicInfo(eCivic).getBlockAlignment()).getDescription()));
		}
		if (GC.getCivicInfo(eCivic).getPrereqAlignment() != NO_ALIGNMENT)
		{
			szHelpText.append(NEWLINE);
			szHelpText.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getAlignmentInfo((AlignmentTypes)GC.getCivicInfo(eCivic).getPrereqAlignment()).getDescription()));
		}
	}
	//<<<<	BUGFfH: End Add
	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/*************************************************************************************************/
	if (bCivilopediaText)
	{
		for (int i = 0; i < GC.getNumCorporationInfos(); i++)
		{
			if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isHasCorporationSupport((CorporationTypes)i))
			{
				if (GC.getCivicInfo(eCivic).getCorporationSupport((CorporationTypes)i) != 0)
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getCivicInfo(eCivic).getCorporationSupport((CorporationTypes)i)));
				}

				if (GC.getCivicInfo(eCivic).getCorporationSupportMultiplier((CorporationTypes)i) != 0)
				{
					szHelpText.append(NEWLINE);
					szHelpText.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT_MODIFIER", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getCivicInfo(eCivic).getCorporationSupportMultiplier((CorporationTypes)i)));
				}
			}
		}
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/
}

void CvGameTextMgr::setTechHelp(CvWStringBuffer& szBuffer, TechTypes eTech, bool bCivilopediaText, bool bPlayerContext, bool bStrategyText, bool bTreeInfo, TechTypes eFromTech)
{
	PROFILE_FUNC();

	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	BuildingTypes eLoopBuilding;
	UnitTypes eLoopUnit;
	bool bFirst;
	int iI;

	// show debug info if cheat level > 0 and alt down
	bool bAlt = gDLL->altKey();
	if (bAlt && (gDLL->getChtLvl() > 0))
	{
		szBuffer.clear();

		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			CvPlayerAI* playerI = &GET_PLAYER((PlayerTypes)iI);
			CvTeamAI* teamI = &GET_TEAM(playerI->getTeam());
			if (playerI->isAlive())
			{
				szTempBuffer.Format(L"%s: ", playerI->getName());
				szBuffer.append(szTempBuffer);

				TechTypes ePlayerTech = playerI->getCurrentResearch();
				if (ePlayerTech == NO_TECH)
					szTempBuffer.Format(L"-\n");
				else
					szTempBuffer.Format(L"%s (%d->%dt)(%d/%d)\n", GC.getTechInfo(ePlayerTech).getDescription(), playerI->calculateResearchRate(ePlayerTech), playerI->getResearchTurnsLeft(ePlayerTech, true), teamI->getResearchProgress(ePlayerTech), teamI->getResearchCost(ePlayerTech));

				szBuffer.append(szTempBuffer);
			}
		}

		return;
	}


	if (NO_TECH == eTech)
	{
		return;
	}

	// DEBUG Sephi
	if (GC.getGameINLINE().isDebugMode() && GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
	{
		if (GC.getTechInfo(eTech).isSpellResearch())
		{
			szTempBuffer.Format(L"\n AI SpellResearchValue: %d", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_getSpellResearchValue(eTech));
		}
		else
		{
			szTempBuffer.Format(L"\n AI TechValue: %d", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).AI_techValueNew(eTech, false));
		}
		szBuffer.append(szTempBuffer);
	}
	//


		//	Tech Name
	if (!bCivilopediaText && (!bTreeInfo || (NO_TECH == eFromTech)))
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTech).getDescription());
		szBuffer.append(szTempBuffer);
	}

	FAssert(GC.getGameINLINE().getActivePlayer() != NO_PLAYER || !bPlayerContext);

	if (bTreeInfo && (NO_TECH != eFromTech))
	{
		buildTechTreeString(szBuffer, eTech, bPlayerContext, eFromTech);
	}

	if (!bCivilopediaText && GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
	{
		if (GC.getTechInfo(eTech).isSpellResearch())
		{
			CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
			if (!(kPlayer.isCanEverSpellResearch(eTech) || GET_TEAM(kPlayer.getTeam()).isHasTech(eTech)))
			{
				if (GC.getTechInfo(eTech).getPrereqOrTechs(0) == NO_TECH || GET_TEAM(kPlayer.getTeam()).isHasTech((TechTypes)GC.getTechInfo(eTech).getPrereqOrTechs(0)))
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TECH_NOT_EVER_SPELLRESEARCH"));
					return;
				}
			}
		}
	}

	//	Obsolete Buildings
	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		if (!bPlayerContext || (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getBuildingClassCount((BuildingClassTypes)iI) > 0))
		{
			if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
			{
				eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);
			}
			else
			{
				eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
			}

			if (eLoopBuilding != NO_BUILDING)
			{
				//	Obsolete Buildings Check...
				if (GC.getBuildingInfo(eLoopBuilding).getObsoleteTech() == eTech)
				{
					buildObsoleteString(szBuffer, eLoopBuilding, true);
				}
			}
		}
	}

	//	Obsolete Bonuses
	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		if (GC.getBonusInfo((BonusTypes)iI).getTechObsolete() == eTech)
		{
			buildObsoleteBonusString(szBuffer, iI, true);
		}
	}

	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); ++iI)
	{
		if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iI).getObsoleteTech() == eTech)
		{
			buildObsoleteSpecialString(szBuffer, iI, true);
		}
	}

	//	Route movement change...
	buildMoveString(szBuffer, eTech, true, bPlayerContext);

	//	Creates a free unit...
	buildFreeUnitString(szBuffer, eTech, true, bPlayerContext);

	//	Increases feature production...
	buildFeatureProductionString(szBuffer, eTech, true, bPlayerContext);

	//	Increases worker build rate...
	buildWorkerRateString(szBuffer, eTech, true, bPlayerContext);

	//	Trade Routed per city change...
	buildTradeRouteString(szBuffer, eTech, true, bPlayerContext);

	//	Health increase...
	buildHealthRateString(szBuffer, eTech, true, bPlayerContext);

	//	Happiness increase...
	buildHappinessRateString(szBuffer, eTech, true, bPlayerContext);

	//	Free Techs...
	buildFreeTechString(szBuffer, eTech, true, bPlayerContext);

	//	Line of Sight Bonus across water...
	buildLOSString(szBuffer, eTech, true, bPlayerContext);

	//	Centers world map...
	buildMapCenterString(szBuffer, eTech, true, bPlayerContext);

	//	Reveals World Map...
	buildMapRevealString(szBuffer, eTech, true);

	//	Enables map trading...
	buildMapTradeString(szBuffer, eTech, true, bPlayerContext);

	//	Enables tech trading...
	buildTechTradeString(szBuffer, eTech, true, bPlayerContext);

	//	Enables gold trading...
	buildGoldTradeString(szBuffer, eTech, true, bPlayerContext);

	//	Enables open borders...
	buildOpenBordersString(szBuffer, eTech, true, bPlayerContext);

	//	Enables defensive pacts...
	buildDefensivePactString(szBuffer, eTech, true, bPlayerContext);

	//	Enables permanent alliances...
	buildPermanentAllianceString(szBuffer, eTech, true, bPlayerContext);

	//	Enables bridge building...
	buildBridgeString(szBuffer, eTech, true, bPlayerContext);

	//	Can spread irrigation...
	buildIrrigationString(szBuffer, eTech, true, bPlayerContext);

	//	Ignore irrigation...
	buildIgnoreIrrigationString(szBuffer, eTech, true, bPlayerContext);

	//	Coastal work...
	buildWaterWorkString(szBuffer, eTech, true, bPlayerContext);

	//	Enables permanent alliances...
	buildVassalStateString(szBuffer, eTech, true, bPlayerContext);

	//	Build farm, irrigation, etc...
	for (iI = 0; iI < GC.getNumBuildInfos(); ++iI)
	{
		buildImprovementString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Extra moves for certain domains...
	for (iI = 0; iI < NUM_DOMAIN_TYPES; ++iI)
	{
		buildDomainExtraMovesString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Adjusting culture, science, etc
	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		buildAdjustString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Enabling trade routes on water...?
	for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
	{
		buildTerrainTradeString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	buildRiverTradeString(szBuffer, eTech, true, bPlayerContext);

	//	Special Buildings
	for (iI = 0; iI < GC.getNumSpecialBuildingInfos(); ++iI)
	{
		buildSpecialBuildingString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	//	Build farm, mine, etc...
	for (iI = 0; iI < GC.getNumImprovementInfos(); ++iI)
	{
		buildYieldChangeString(szBuffer, eTech, iI, true, bPlayerContext);
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		bFirst = buildBonusRevealString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/23
	bFirst = true;
	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		bFirst = buildBonusEnableString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}
	//<<<<	BUGFfH: End Add

	bFirst = true;

	for (iI = 0; iI < GC.getNumCivicInfos(); ++iI)
	{
		/*************************************************************************************************/
		/**	ADDON (FFHBUG) Sephi    																	**/
		/**	Techtree doesn't show features only available to other civs									**/
		/*************************************************************************************************/
		/** orig
				bFirst = buildCivicRevealString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
		**/
		if (bCivilopediaText || GC.getCivicInfo((CivicTypes)iI).getPrereqCivilization() == NO_CIVILIZATION ||
			GC.getCivicInfo((CivicTypes)iI).getPrereqCivilization() == GC.getGameINLINE().getActiveCivilizationType())
		{
			bFirst = buildCivicRevealString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
		}
		/*************************************************************************************************/
		/**	END																							**/
		/*************************************************************************************************/
	}

	if (!bCivilopediaText)
	{
		bFirst = true;

		for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
		{
			if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isProductionMaxedUnitClass((UnitClassTypes)iI)))
			{
				if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
				{
					eLoopUnit = (UnitTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(iI);
				}
				else
				{
					eLoopUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex();
				}
				/*************************************************************************************************/
				/**	ADDON (FFHBUG) Sephi    																	**/
				/**	Techtree doesn't show features only available to other civs									**/
				/*************************************************************************************************/
				/** orig
								if (eLoopUnit != NO_UNIT)
				**/
				if (eLoopUnit != NO_UNIT &&
					(GC.getUnitInfo(eLoopUnit).getPrereqCiv() == NO_CIVILIZATION ||
						GC.getUnitInfo(eLoopUnit).getPrereqCiv() == GC.getGameINLINE().getActiveCivilizationType()))
					/*************************************************************************************************/
					/**	END																							**/
					/*************************************************************************************************/

					if (eLoopUnit != NO_UNIT)
					{
						if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canTrain(eLoopUnit)))
						{
							if (GC.getUnitInfo(eLoopUnit).getPrereqAndTech() == eTech)
							{
								szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_TRAIN").c_str());
								szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eLoopUnit).getDescription());
								setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
								bFirst = false;
							}
							else
							{
								for (int iJ = 0; iJ < GC.getNUM_UNIT_AND_TECH_PREREQS(); iJ++)
								{
									if (GC.getUnitInfo(eLoopUnit).getPrereqAndTechs(iJ) == eTech)
									{
										szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_TRAIN").c_str());
										szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eLoopUnit).getDescription());
										setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
										bFirst = false;
										break;
									}
								}
							}
						}
					}
			}
		}

		bFirst = true;

		for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
		{
			if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isProductionMaxedBuildingClass((BuildingClassTypes)iI)))
			{
				if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
				{
					eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationBuildings(iI);
				}
				else
				{
					eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
				}
				/*************************************************************************************************/
				/**	ADDON (FFHBUG) Sephi    																	**/
				/**	Techtree doesn't show features only available to other civs									**/
				/*************************************************************************************************/
				/** orig
								if (eLoopBuilding != NO_BUILDING)
				**/
				if (eLoopBuilding != NO_BUILDING &&
					(GC.getBuildingInfo(eLoopBuilding).getPrereqCiv() == NO_CIVILIZATION ||
						GC.getBuildingInfo(eLoopBuilding).getPrereqCiv() == GC.getGameINLINE().getActiveCivilizationType()))
					/*************************************************************************************************/
					/**	END																							**/
					/*************************************************************************************************/
				{
					if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canConstruct(eLoopBuilding, false, true)))
					{
						if (GC.getBuildingInfo(eLoopBuilding).getPrereqAndTech() == eTech)
						{
							szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_CONSTRUCT").c_str());
							//>>>>	BUGFfH: Modified by Denev 2009/09/13
							//							szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eLoopBuilding).getDescription());
							szTempBuffer.assign(getLinkedText(eLoopBuilding));
							//<<<<	BUGFfH: End Modify

							setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
							bFirst = false;
						}
						else
						{
							for (int iJ = 0; iJ < GC.getNUM_BUILDING_AND_TECH_PREREQS(); iJ++)
							{
								if (GC.getBuildingInfo(eLoopBuilding).getPrereqAndTechs(iJ) == eTech)
								{
									szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_CONSTRUCT").c_str());
									//>>>>	BUGFfH: Modified by Denev 2009/09/13
									//									szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eLoopBuilding).getDescription());
									szTempBuffer.assign(getLinkedText(eLoopBuilding));
									//<<<<	BUGFfH: End Modify
									setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
									bFirst = false;
									break;
								}
							}
						}
					}
				}
			}
		}

		bFirst = true;

		for (iI = 0; iI < GC.getNumProjectInfos(); ++iI)
		{
			if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isProductionMaxedProject((ProjectTypes)iI)))
			{
				if (!bPlayerContext || !(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canCreate(((ProjectTypes)iI), false, true)))
				{
					if (GC.getProjectInfo((ProjectTypes)iI).getTechPrereq() == eTech)
					{

						//FfH: Added by Kael 07/30/2009
						//						szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_CREATE").c_str());
						//						szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo((ProjectTypes)iI).getDescription());
						//						setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
						//						bFirst = false;
						if (!bPlayerContext ||
							GC.getProjectInfo((ProjectTypes)iI).getPrereqCivilization() == NO_CIVILIZATION ||
							GC.getProjectInfo((ProjectTypes)iI).getPrereqCivilization() == GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType())
						{
							szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_TECH_CAN_CREATE").c_str());
							szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo((ProjectTypes)iI).getDescription());
							setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
							bFirst = false;
						}
						//FfH: End Add

					}
				}
			}
		}
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumProcessInfos(); ++iI)
	{
		bFirst = buildProcessInfoString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumReligionInfos(); ++iI)
	{
		if (!bPlayerContext || !(GC.getGameINLINE().isReligionSlotTaken((ReligionTypes)iI)))
		{
			bFirst = buildFoundReligionString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
		}
	}
	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/**	Corporations aren't founded by Techs    													**/
	/*************************************************************************************************/
	/** BTS code disabled
		bFirst = true;
		for (iI = 0; iI < GC.getNumCorporationInfos(); ++iI)
		{
			if (!bPlayerContext || !(GC.getGameINLINE().isCorporationFounded((CorporationTypes)iI)))
			{
				bFirst = buildFoundCorporationString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
			}
		}
	**/
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	bFirst = true;
	for (iI = 0; iI < GC.getNumPromotionInfos(); ++iI)
	{
		bFirst = buildPromotionString(szBuffer, eTech, iI, bFirst, true, bPlayerContext);
	}

	if (bTreeInfo && NO_TECH == eFromTech)
	{
		buildSingleLineTechTreeString(szBuffer, eTech, bPlayerContext);
	}

	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/*************************************************************************************************/
	if (bCivilopediaText)
	{
		for (int i = 0; i < GC.getNumCorporationInfos(); i++)
		{
			if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isHasCorporationSupport((CorporationTypes)i))
			{
				if (GC.getTechInfo(eTech).getCorporationSupport((CorporationTypes)i) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getTechInfo(eTech).getCorporationSupport((CorporationTypes)i)));
				}

				if (GC.getTechInfo(eTech).getCorporationSupportMultiplier((CorporationTypes)i) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT_MODIFIER", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getTechInfo(eTech).getCorporationSupportMultiplier((CorporationTypes)i)));
				}
			}
		}
	}

	if (GC.getTechInfo(eTech).isAllowAnimalTaming())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TECH_ALLOW_ANIMAL_TAMING"));
	}

	if (GC.getTechInfo(eTech).isAllowGuild())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TECH_ALLOW_GUILD"));
	}

	if (GC.getTechInfo(eTech).isAllowReligion())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TECH_ALLOW_RELIGION"));
	}

	if (GC.getTechInfo(eTech).getArcaneAuraPromotion() != NO_PROMOTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_ARCANE_PROMOTION", getLinkedText((PromotionTypes)GC.getTechInfo(eTech).getArcaneAuraPromotion()).c_str()));
	}

	if (GC.getTechInfo(eTech).getNumPrereqSpellResearchFromSameSchool() != 0 && GC.getTechInfo(eTech).getSpellSchool() != NO_MANASCHOOL)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_PREREQ_NUM_SAME_SCHOOL", GC.getTechInfo(eTech).getNumPrereqSpellResearchFromSameSchool(), GC.getManaschoolInfo((ManaschoolTypes)GC.getTechInfo(eTech).getSpellSchool()).getDescription()));
	}

	if (GC.getTechInfo(eTech).getWillpowerChange() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_INCREASES_WILLPOWER_CHANCE"));
	}

	if (GC.getTechInfo(eTech).getReducedTerraformCost() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_REDUCES_TERRAFORM_COST", GC.getTechInfo(eTech).getReducedTerraformCost()));
	}

	if (GC.getTechInfo(eTech).getReducedEnchantmentCost() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_REDUCES_ENCHANTMENT_COST", GC.getTechInfo(eTech).getReducedEnchantmentCost()));
	}

	if (GC.getTechInfo(eTech).getReducedSummonCost() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_REDUCES_SUMMON_COST", GC.getTechInfo(eTech).getReducedSummonCost()));
	}

	if (GC.getTechInfo(eTech).isIncreaseTerraformTiles())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_INCREASES_TERRAFORM_TILES"));
	}

	if (GC.getTechInfo(eTech).isIncreaseHostileTerraformTiles())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_INCREASES_HOSTILE_TERRAFORM_TILES"));
	}

	if (GC.getTechInfo(eTech).getIncreaseSummonXP() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_INCREASES_SUMMON_XP", GC.getTechInfo(eTech).getIncreaseSummonXP()));
	}

	if (GC.getTechInfo(eTech).getIncreaseMaxMana() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_INCREASES_MAX_MANA", GC.getTechInfo(eTech).getIncreaseMaxMana()));
	}

	if (GC.getTechInfo(eTech).getIncreaseMana() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_INCREASES_MANA_INCOME", GC.getTechInfo(eTech).getIncreaseMana()));
	}

	if (GC.getTechInfo(eTech).getIncreaseManaFromNodes() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_INCREASES_MANA_FROM_NODES", GC.getTechInfo(eTech).getIncreaseManaFromNodes()));
	}



	if (GC.getTechInfo(eTech).getPrereqBonus() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_PREREQ_BONUS", getLinkedText((BonusTypes)GC.getTechInfo(eTech).getPrereqBonus()).c_str()));
	}

	if (GC.getTechInfo(eTech).getBlockedbyAlignment() != NO_ALIGNMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_BLOCKED_BY_ALIGNMENT", GC.getAlignmentInfo((AlignmentTypes)GC.getTechInfo(eTech).getBlockedbyAlignment()).getDescription()));
	}

	if (GC.getTechInfo(eTech).getPreferredAlignment() != NO_ALIGNMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELLRESEARCH_REQUIRED_ALIGNMENT_PEDIA", GC.getAlignmentInfo((AlignmentTypes)GC.getTechInfo(eTech).getPreferredAlignment()).getDescription()));
	}

	UnitClassTypes eUnitClass = (UnitClassTypes)GC.getTechInfo(eTech).getFreeUnitClass();
	if (eUnitClass != NO_UNITCLASS)
	{
		UnitTypes eUnit = (UnitTypes)GC.getUnitClassInfo(eUnitClass).getDefaultUnitIndex();

		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
			eUnit = (UnitTypes)GC.getCivilizationInfo(GC.getGame().getActiveCivilizationType()).getCivilizationUnits(eUnitClass);

		if (eUnit != NO_UNIT) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_TECH_FREE_UNITCLASS", getLinkedText(eUnit).c_str()));
		}
	}

	for (int iI = 0; iI < GC.getNumSpellInfos(); ++iI) {
		if (GC.getSpellInfo((SpellTypes)iI).getTechPrereq() == eTech) {
			if (GC.getSpellInfo((SpellTypes)iI).getCivilizationPrereq() == NO_CIVILIZATION
				|| (GC.getGame().getActiveCivilizationType() == GC.getSpellInfo((SpellTypes)iI).getCivilizationPrereq())) {

				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ALLOWS_TEXT", getLinkedText((SpellTypes)iI).c_str()));
			}
		}
	}

	for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
	{
		if (GC.getTechInfo(eTech).getSpellDamageChange(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_SPELLDAMAGE_CHANGE", GC.getTechInfo(eTech).getSpellDamageChange(iI), GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
		}
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (!CvWString(GC.getTechInfo(eTech).getHelp()).empty())
	{
		szBuffer.append(CvWString::format(L"%s%s", NEWLINE, GC.getTechInfo(eTech).getHelp()).c_str());
	}

	if (!bCivilopediaText)
	{	// Display tech cost
		if (GC.getTechInfo(eTech).getResearchCost() > 0) {
			if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER)
			{
				if (GC.getTechInfo((TechTypes)eTech).isSpellResearch())
				{
					szTempBuffer.Format(L"\n%d%c", GC.getTechInfo(eTech).getResearchCost(), GC.getCommerceInfo(COMMERCE_ARCANE).getChar());
				}
				else if (GC.getTechInfo(eTech).isHolyKnowledge())
				{
					szTempBuffer.Format(L"\n%d%c", GC.getTechInfo(eTech).getResearchCost(), GC.getCommerceInfo(COMMERCE_FAITH).getChar());
				}
				else
				{
					szTempBuffer.Format(L"\n%d%c", GC.getTechInfo(eTech).getResearchCost(), GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
				}

				szBuffer.append(szTempBuffer);
			}
			else if (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech(eTech))
			{
				if (GC.getTechInfo((TechTypes)eTech).isSpellResearch())
				{
					szTempBuffer.Format(L"\n%d%c", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech), GC.getCommerceInfo(COMMERCE_ARCANE).getChar());
				}
				else if (GC.getTechInfo(eTech).isHolyKnowledge())
				{
					szTempBuffer.Format(L"\n%d%c", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech), GC.getCommerceInfo(COMMERCE_FAITH).getChar());
				}
				else
				{
					szTempBuffer.Format(L"\n%d%c", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech), GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
				}

				szBuffer.append(szTempBuffer);
			}
			else
			{
				//added Sephi
				if (GC.getTechInfo(eTech).isHolyKnowledge())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TECH_COST_HOLY_KNOWLEDGE", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech)));
				}
				else if (GC.getTechInfo(eTech).isCulturalAchievement())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TECH_COST_CULTURAL_ACHIEVEMENT", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech)));
				}
				else if (GC.getTechInfo((TechTypes)eTech).isSpellResearch())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TECH_COST_SPELL_RESEARCH", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech)));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TECH_NUM_TURNS", GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getResearchTurnsLeft(eTech, (gDLL->ctrlKey() || !(gDLL->shiftKey())))));

					szTempBuffer.Format(L" (%d/%d %c)", GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchProgress(eTech), GET_TEAM(GC.getGameINLINE().getActiveTeam()).getResearchCost(eTech), (GC.getTechInfo(eTech).isHolyKnowledge()) ? GC.getCommerceInfo(COMMERCE_ARCANE).getChar() : GC.getCommerceInfo(COMMERCE_RESEARCH).getChar());
					szBuffer.append(szTempBuffer);
				}
			}
		}  // if Researchcost > 0
	}

	//Sephi
	//Block non Important Information
	/**
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canResearch(eTech))
			{
				for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
				{
					CvUnitInfo& kUnit = GC.getUnitInfo((UnitTypes)iI);

					if (kUnit.getBaseDiscover() > 0 || kUnit.getDiscoverMultiplier() > 0)
					{
						if (::getDiscoveryTech((UnitTypes)iI, GC.getGameINLINE().getActivePlayer()) == eTech)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_TECH_GREAT_PERSON_DISCOVER", kUnit.getTextKeyWide()));
						}
					}
				}

				if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCurrentEra() < GC.getTechInfo(eTech).getEra())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TECH_ERA_ADVANCE", GC.getEraInfo((EraTypes)GC.getTechInfo(eTech).getEra()).getTextKeyWide()));
				}
			}
		}
	**/

	//FfH: Added by Kael 08/09/2007
	if (GC.getTechInfo(eTech).getPrereqReligion() != NO_RELIGION)
	{
		if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getStateReligion() != GC.getTechInfo(eTech).getPrereqReligion())
		{
			//>>>>	BUGFfH: Modified by Denev 2009/09/17
			/*
						szBuffer.append(CvWString::format(SETCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT")));
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_TECH_PREREQ_RELIGION", GC.getReligionInfo((ReligionTypes)GC.getTechInfo(eTech).getPrereqReligion()).getDescription()));
						szBuffer.append(CvWString::format(ENDCOLR));
			*/
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_REQ_STATE_RELIGION", GC.getReligionInfo((ReligionTypes)GC.getTechInfo(eTech).getPrereqReligion()).getChar()));
			//<<<<	BUGFfH: End Modify
		}
	}
	//FfH: End Add

	if (bStrategyText)
	{
		if (!CvWString(GC.getTechInfo(eTech).getStrategy()).empty())
		{
			if ((GC.getGameINLINE().getActivePlayer() == NO_PLAYER) || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isOption(PLAYEROPTION_ADVISOR_HELP))
			{
				szBuffer.append(SEPARATOR);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_SIDS_TIPS"));
				szBuffer.append(L'\"');
				szBuffer.append(GC.getTechInfo(eTech).getStrategy());
				szBuffer.append(L'\"');
			}
		}
	}
}


void CvGameTextMgr::setBasicUnitHelp(CvWStringBuffer& szBuffer, UnitTypes eUnit, bool bCivilopediaText)
{
	PROFILE_FUNC();

	CvWString szTempBuffer;
	bool bFirst;
	int iCount;
	int iI;

	if (NO_UNIT == eUnit)
	{
		return;
	}

	CvUnitInfo& kUnit = GC.getUnitInfo(eUnit);

	if (!bCivilopediaText)
	{
		szBuffer.append(NEWLINE);

		/*************************************************************************************************/
		/**	ADDON (Ranged Combat) Sephi								                     				**/
		/**	Xienwolf Tweak							02/01/09											**/
		/**																								**/
		/**							Allows display of ranged attack on ground units						**/
		/*************************************************************************************************/
		/**								---- Start Original Code ----									**
				if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_AIR)
				{
					if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
					{
						szTempBuffer.Format(L"%d%c, ", GC.getUnitInfo(eUnit).getAirCombat(), gDLL->getSymbolID(STRENGTH_CHAR));
						szBuffer.append(szTempBuffer);
					}
				}
				else
				{
					if (GC.getUnitInfo(eUnit).getCombat() > 0)
					{
						szTempBuffer.Format(L"%d%c, ", GC.getUnitInfo(eUnit).getCombat(), gDLL->getSymbolID(STRENGTH_CHAR));
						szBuffer.append(szTempBuffer);
					}
				}
		/**								----  End Original Code  ----									**/
		/** disabled Ranged Attacks - replaced by Support Fire
				if (GC.getUnitInfo(eUnit).getAirCombat() > 0)
				{
					szTempBuffer.Format(L"%d%c, ", GC.getUnitInfo(eUnit).getAirCombat(), gDLL->getSymbolID(RANGED_CHAR));
					szBuffer.append(szTempBuffer);
				}
		**/
		if ((GC.getUnitInfo(eUnit).getCombat() > 0 || GC.getUnitInfo(eUnit).getCombatDefense() > 0) && GC.getUnitInfo(eUnit).getCombat() != GC.getUnitInfo(eUnit).getCombatDefense())
		{
			szTempBuffer.Format(L"%d/%d%c, ", GC.getUnitInfo(eUnit).getCombat(), GC.getUnitInfo(eUnit).getCombatDefense(), gDLL->getSymbolID(STRENGTH_CHAR));
			szBuffer.append(szTempBuffer);
		}

		else if (GC.getUnitInfo(eUnit).getCombat() > 0)
		{
			szTempBuffer.Format(L"%d%c, ", GC.getUnitInfo(eUnit).getCombat(), gDLL->getSymbolID(STRENGTH_CHAR));
			szBuffer.append(szTempBuffer);
		}

		/*************************************************************************************************/
		/**	Tweak									END													**/
		/*************************************************************************************************/

		szTempBuffer.Format(L"%d%c", GC.getUnitInfo(eUnit).getMoves(), gDLL->getSymbolID(MOVES_CHAR));
		szBuffer.append(szTempBuffer);

		if (GC.getUnitInfo(eUnit).getAirRange() > 0)
		{
			szBuffer.append(L", ");
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_AIR_RANGE", GC.getUnitInfo(eUnit).getAirRange()));
		}
	}

	/*************************************************************************************************/
	/**	ADDON (Units can be unlocked by Gameoptions) Sephi                     					**/
	/*************************************************************************************************/
	GameOptionTypes eGameOption = (GameOptionTypes)GC.getUnitInfo(eUnit).getRequiredGameOption();
	if (eGameOption != NO_GAMEOPTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES_GAMEOPTION"));
		szBuffer.append(" ");
		szBuffer.append(GC.getGameOptionInfo(eGameOption).getDescription());
	}

	// New Yields
	YieldTypes ReducedCostBy = (YieldTypes)GC.getUnitInfo(eUnit).getReducedCostByYield();
	if (ReducedCostBy != NO_YIELD)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_REDUCED_COST_BY_YIELD"));
		szTempBuffer.Format(L"%c", GC.getYieldInfo(ReducedCostBy).getChar());
		szBuffer.append(szTempBuffer);
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	/*************************************************************************************************/
	/**	ADDON (New tag Definitions) Sephi                                          					**/
	/*************************************************************************************************/
	if (GC.getUnitInfo(eUnit).getFaithUpkeep() != 0)
	{
		int iFaithCost = 0;
		for (int iI = 0; iI < GC.getNumProjectInfos(); iI++)
		{
			if (GC.getProjectInfo((ProjectTypes)iI).getSummonUnitType() == eUnit)
			{
				iFaithCost = GC.getProjectInfo((ProjectTypes)iI).getFaithCost();
				if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
				{
					iFaithCost = GET_PLAYER((PlayerTypes)GC.getGameINLINE().getActivePlayer()).getSpecificFaithRitualCost((ProjectTypes)iI);
				}
				break;
			}
		}

		szBuffer.append(NEWLINE);
		if (iFaithCost == 0 && GC.getUnitInfo(eUnit).getFaithUpkeep() < 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_FAITH_INCOME", -GC.getUnitInfo(eUnit).getFaithUpkeep()));
		}
		else
		{
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_FAITH_COST", iFaithCost, GC.getUnitInfo(eUnit).getFaithUpkeep()));
		}
	}

	if (GC.getUnitInfo(eUnit).getManaUpkeep() != 0)
	{
		ProjectTypes eProject = NO_PROJECT;
		int iManaCost = 0;
		for (int iI = 0; iI < GC.getNumProjectInfos(); iI++)
		{
			if (GC.getProjectInfo((ProjectTypes)iI).getSummonUnitType() == eUnit)
			{
				eProject = (ProjectTypes)iI;
				break;
			}
		}

		if (eProject != NO_PROJECT)
		{
			iManaCost = GC.getProjectInfo(eProject).getManaCost();
			if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
			{
				//SpyFanatic: correct discount for Balsph
				//iManaCost = GET_PLAYER((PlayerTypes)GC.getGameINLINE().getActivePlayer()).getSummonManaCost(eProject);
				iManaCost = GET_PLAYER((PlayerTypes)GC.getGameINLINE().getActivePlayer()).getSpecificMagicRitualCost(eProject);
			}

			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MANA_COST", iManaCost, GC.getUnitInfo(eUnit).getManaUpkeep()));
		}
	}

	if (kUnit.getGlobalYieldTypeCost() != NO_YIELD && kUnit.getGlobalYieldCost() > 0)
	{
		int iYieldCost = kUnit.getGlobalYieldCost();
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			iYieldCost = (iYieldCost * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		}
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GLOBAL_YIELD_COST", iYieldCost, GC.getYieldInfo((YieldTypes)kUnit.getGlobalYieldTypeCost()).getChar()));
	}


	if (kUnit.isNoXP())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_NO_XP_PEDIA"));
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (GC.getUnitInfo(eUnit).isGoldenAge())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_GOLDEN_AGE"));
	}

	if (GC.getUnitInfo(eUnit).getLeaderExperience() > 0)
	{
		if (!GC.getUnitClassInfo((UnitClassTypes)GC.getUnitInfo(eUnit).getUnitClassType()).isBattleCaptain())
		{
			if (0 == GC.getDefineINT("WARLORD_EXTRA_EXPERIENCE_PER_UNIT_PERCENT"))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_LEADER", GC.getUnitInfo(eUnit).getLeaderExperience()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_LEADER_EXPERIENCE", GC.getUnitInfo(eUnit).getLeaderExperience()));
			}
		}
	}

	if (NO_PROMOTION != GC.getUnitInfo(eUnit).getLeaderPromotion())
	{
		szBuffer.append(CvWString::format(L"%s%c%s", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), gDLL->getText("TXT_KEY_PROMOTION_WHEN_LEADING").GetCString()));
		parsePromotionHelp(szBuffer, (PromotionTypes)GC.getUnitInfo(eUnit).getLeaderPromotion(), L"\n   ");
	}

	if ((GC.getUnitInfo(eUnit).getBaseDiscover() > 0) || (GC.getUnitInfo(eUnit).getDiscoverMultiplier() > 0))
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DISCOVER_TECH"));
	}

	if ((GC.getUnitInfo(eUnit).getBaseHurry() > 0) || (GC.getUnitInfo(eUnit).getHurryMultiplier() > 0))
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HURRY_PRODUCTION"));
	}

	if ((GC.getUnitInfo(eUnit).getBaseTrade() > 0) || (GC.getUnitInfo(eUnit).getTradeMultiplier() > 0))
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_TRADE_MISSION"));
	}

	if (GC.getUnitInfo(eUnit).getGreatWorkCulture() > 0)
	{
		int iCulture = GC.getUnitInfo(eUnit).getGreatWorkCulture();
		if (NO_GAMESPEED != GC.getGameINLINE().getGameSpeedType())
		{
			iCulture *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent();
			iCulture /= 100;
		}

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_GREAT_WORK", iCulture));
	}

	if (GC.getUnitInfo(eUnit).getEspionagePoints() > 0)
	{
		int iEspionage = GC.getUnitInfo(eUnit).getEspionagePoints();
		if (NO_GAMESPEED != GC.getGameINLINE().getGameSpeedType())
		{
			iEspionage *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent();
			iEspionage /= 100;
		}

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ESPIONAGE_MISSION", iEspionage));
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumReligionInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getReligionSpreads(iI) > 0)
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN_SPREAD").c_str());
			CvWString szReligion;
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szReligion.Format(L"<link=literal>%s</link>", GC.getReligionInfo((ReligionTypes) iI).getDescription());
			szReligion = getLinkedText((ReligionTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szTempBuffer, szReligion, L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumCorporationInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getCorporationSpreads(iI) > 0)
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN_EXPAND").c_str());
			CvWString szCorporation;
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szCorporation.Format(L"<link=literal>%s</link>", GC.getCorporationInfo((CorporationTypes) iI).getDescription());
			szCorporation = getLinkedText((CorporationTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szTempBuffer, szCorporation, L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumSpecialistInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getGreatPeoples(iI))
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN_JOIN").c_str());
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			CvWString szSpecialistLink = CvWString::format(L"<link=literal>%s</link>", GC.getSpecialistInfo((SpecialistTypes) iI).getDescription());
			CvWString szSpecialistLink = getLinkedText((SpecialistTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szTempBuffer, szSpecialistLink.GetCString(), L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;
	//>>>>	BUGFfH: Modified by Denev 2009/09/24
	/*
		for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
		{
			if (GC.getUnitInfo(eUnit).getBuildings(iI) || GC.getUnitInfo(eUnit).getForceBuildings(iI))
			{
				szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN_CONSTRUCT").c_str());
				CvWString szBuildingLink = CvWString::format(L"<link=literal>%s</link>", GC.getBuildingInfo((BuildingTypes) iI).getDescription());
				setListHelp(szBuffer, szTempBuffer, szBuildingLink.GetCString(), L", ", bFirst);
				bFirst = false;
			}
		}
	*/
	szTempBuffer.assign(NEWLINE);
	szTempBuffer.append(gDLL->getText("TXT_KEY_UNIT_CAN_CONSTRUCT"));
	for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getBuildings(iI))
		{
			setListHelp(szBuffer, szTempBuffer, getLinkedText((BuildingTypes)iI).c_str(), L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getForceBuildings(iI))
		{
			setListHelp(szBuffer, szTempBuffer, getLinkedText((BuildingTypes)iI).c_str(), L", ", bFirst);
			bFirst = false;
		}
	}
	if (!bFirst)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_IGNORE_REQUIREMENT"));
	}
	//<<<<	BUGFfH: End Modify

	if (GC.getUnitInfo(eUnit).getCargoSpace() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CARGO_SPACE", GC.getUnitInfo(eUnit).getCargoSpace()));

		if (GC.getUnitInfo(eUnit).getSpecialCargo() != NO_SPECIALUNIT)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CARRIES", GC.getSpecialUnitInfo((SpecialUnitTypes)GC.getUnitInfo(eUnit).getSpecialCargo()).getTextKeyWide()));
		}
	}

	szTempBuffer.Format(L"%s%s ", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CANNOT_ENTER").GetCString());

	bFirst = true;

	for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getTerrainImpassable(iI))
		{
			CvWString szTerrain;
			TechTypes eTech = (TechTypes)GC.getUnitInfo(eUnit).getTerrainPassableTech(iI);
			if (NO_TECH == eTech)
			{
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTerrain.Format(L"<link=literal>%s</link>", GC.getTerrainInfo((TerrainTypes)iI).getDescription());
				szTerrain = getLinkedText((TerrainTypes)iI);
				//<<<<	BUGFfH: End Modify
			}
			else
			{
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szTerrain = gDLL->getText("TXT_KEY_TERRAIN_UNTIL_TECH", GC.getTerrainInfo((TerrainTypes)iI).getTextKeyWide(), GC.getTechInfo(eTech).getTextKeyWide());
				szTerrain = gDLL->getText("TXT_KEY_TERRAIN_UNTIL_TECH", getLinkedText((TerrainTypes)iI).c_str(), getLinkedText(eTech).c_str());
				//<<<<	BUGFfH: End Modify
			}
			setListHelp(szBuffer, szTempBuffer, szTerrain, L", ", bFirst);
			bFirst = false;
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getFeatureImpassable(iI))
		{
			CvWString szFeature;
			TechTypes eTech = (TechTypes)GC.getUnitInfo(eUnit).getTerrainPassableTech(iI);
			if (NO_TECH == eTech)
			{
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szFeature.Format(L"<link=literal>%s</link>", GC.getFeatureInfo((FeatureTypes)iI).getDescription());
				szFeature = getLinkedText((FeatureTypes)iI);
				//<<<<	BUGFfH: End Modify
			}
			else
			{
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szFeature = gDLL->getText("TXT_KEY_TERRAIN_UNTIL_TECH", GC.getFeatureInfo((FeatureTypes)iI).getTextKeyWide(), GC.getTechInfo(eTech).getTextKeyWide());
				szFeature = gDLL->getText("TXT_KEY_TERRAIN_UNTIL_TECH", getLinkedText((FeatureTypes)iI).c_str(), getLinkedText(eTech).c_str());
				//<<<<	BUGFfH: End Modify
			}
			setListHelp(szBuffer, szTempBuffer, szFeature, L", ", bFirst);
			bFirst = false;
		}
	}

	if (GC.getUnitInfo(eUnit).isInvisible())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_INVISIBLE_ALL"));
	}
	else if (GC.getUnitInfo(eUnit).getInvisibleType() != NO_INVISIBLE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_INVISIBLE_MOST"));
	}

	for (iI = 0; iI < GC.getUnitInfo(eUnit).getNumSeeInvisibleTypes(); ++iI)
	{
		if (bCivilopediaText || (GC.getUnitInfo(eUnit).getSeeInvisibleType(iI) != GC.getUnitInfo(eUnit).getInvisibleType()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_SEE_INVISIBLE", GC.getInvisibleInfo((InvisibleTypes)GC.getUnitInfo(eUnit).getSeeInvisibleType(iI)).getTextKeyWide()));
		}
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/08
	if (GC.getUnitInfo(eUnit).isInvestigate())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_INVESTIGATE_CITY"));
	}
	//<<<<	BUGFfH: End Add

	if (GC.getUnitInfo(eUnit).isCanMoveImpassable())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CAN_MOVE_IMPASSABLE"));
	}


	if (GC.getUnitInfo(eUnit).isNoBadGoodies())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NO_BAD_GOODIES"));
	}

	if (GC.getUnitInfo(eUnit).isHiddenNationality())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HIDDEN_NATIONALITY"));
	}

	if (GC.getUnitInfo(eUnit).isAlwaysHostile())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ALWAYS_HOSTILE"));
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/24
	int iTotalAttackStr = GC.getUnitInfo(eUnit).getCombat();
	for (int iDamageType = 0; iDamageType < NUM_DAMAGE_TYPES; iDamageType++)
	{
		iTotalAttackStr += GC.getUnitInfo(eUnit).getDamageTypeCombat(iDamageType);
	}
	if (iTotalAttackStr > 0)
	{
		//<<<<	BUGFfH: End Add
		if (GC.getUnitInfo(eUnit).isOnlyDefensive())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ONLY_DEFENSIVE"));
		}

		if (GC.getUnitInfo(eUnit).isNoCapture())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CANNOT_CAPTURE"));
		}

		//>>>>	BUGFfH: Added by Denev 2009/09/24
		if (GC.getUnitInfo(eUnit).isNoPillage())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CANNOT_PILLAGE"));
		}

		if (!GC.getUnitInfo(eUnit).isMilitaryHappiness())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CANNOT_PROTECT_CITY"));
		}
	}
	//<<<<	BUGFfH: End Add

	if (GC.getUnitInfo(eUnit).isRivalTerritory())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_EXPLORE_RIVAL"));
	}

	if (GC.getUnitInfo(eUnit).isFound())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_FOUND_CITY"));
	}

	iCount = 0;

	for (iI = 0; iI < GC.getNumBuildInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getBuilds(iI))
		{
			iCount++;
		}
	}

	if (iCount > ((GC.getNumBuildInfos() * 3) / 4))
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_IMPROVE_PLOTS"));
	}
	else
	{
		bFirst = true;

		for (iI = 0; iI < GC.getNumBuildInfos(); ++iI)
		{
			if (GC.getUnitInfo(eUnit).getBuilds(iI))
			{
				szTempBuffer.Format(L"%s%s ", NEWLINE, gDLL->getText("TXT_KEY_UNIT_CAN").c_str());
				setListHelp(szBuffer, szTempBuffer, GC.getBuildInfo((BuildTypes)iI).getDescription(), L", ", bFirst);
				bFirst = false;
			}
		}
	}

	if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CAN_NUKE"));
	}

	if (GC.getUnitInfo(eUnit).isCounterSpy())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_EXPOSE_SPIES"));
	}

	if ((GC.getUnitInfo(eUnit).getFirstStrikes() + GC.getUnitInfo(eUnit).getChanceFirstStrikes()) > 0)
	{
		if (GC.getUnitInfo(eUnit).getChanceFirstStrikes() == 0)
		{
			if (GC.getUnitInfo(eUnit).getFirstStrikes() == 1)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ONE_FIRST_STRIKE"));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NUM_FIRST_STRIKES", GC.getUnitInfo(eUnit).getFirstStrikes()));
			}
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_FIRST_STRIKE_CHANCES", GC.getUnitInfo(eUnit).getFirstStrikes(), GC.getUnitInfo(eUnit).getFirstStrikes() + GC.getUnitInfo(eUnit).getChanceFirstStrikes()));
		}
	}

	if (GC.getUnitInfo(eUnit).isFirstStrikeImmune())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_IMMUNE_FIRST_STRIKES"));
	}
	//>>>>	More Detailed Civilopedia: Added by Denev 2009/11/10
	if (GC.getUnitInfo(eUnit).getUnitCombatType() != NO_UNITCOMBAT) {
		if (GC.getUnitCombatInfo((UnitCombatTypes)GC.getUnitInfo(eUnit).getUnitCombatType()).isArcherSupport())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSIVE_STRIKES"));
		}
	}

	if (GC.getUnitInfo(eUnit).getUnitCombatType() != NO_UNITCOMBAT) {
		if (GC.getUnitCombatInfo((UnitCombatTypes)GC.getUnitInfo(eUnit).getUnitCombatType()).isDefensiveBonuses())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSIVE_BONUSES_WILDERNESS"));
		}
	}

	if (GC.getUnitInfo(eUnit).getUnitCombatType() != NO_UNITCOMBAT) {
		if (GC.getUnitCombatInfo((UnitCombatTypes)GC.getUnitInfo(eUnit).getUnitCombatType()).isCityDefensiveBonuses())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSIVE_BONUSES_CITIES"));
		}
	}

	//<<<<	More Detailed Civilopedia: End Add

	//FfH: Added by Kael 10/22/2008
		/**
		if (GC.getUnitInfo(eUnit).getDefensiveStrikeChance() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_DEFENSIVE_STRIKE_PEDIA", GC.getUnitInfo(eUnit).getDefensiveStrikeChance(), GC.getUnitInfo(eUnit).getDefensiveStrikeDamage()));
		}
		**/
	if (GC.getUnitInfo(eUnit).isImmuneToDefensiveStrike())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMUNE_TO_DEFENSIVE_STRIKE_PEDIA"));
	}
	//FfH: End Add

	if (GC.getUnitInfo(eUnit).isNoDefensiveBonus())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NO_DEFENSE_BONUSES"));
	}

	if (GC.getUnitInfo(eUnit).isFlatMovementCost())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_FLAT_MOVEMENT"));
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/15
	else
	{
		//<<<<	BUGFfH: End Add
		if (GC.getUnitInfo(eUnit).isIgnoreTerrainCost())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_IGNORE_TERRAIN"));
		}
		//>>>>	BUGFfH: Added by Denev 2009/09/15
	}
	//<<<<	BUGFfH: End Add

	if (GC.getUnitInfo(eUnit).getInterceptionProbability() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_INTERCEPT_AIRCRAFT", GC.getUnitInfo(eUnit).getInterceptionProbability()));
	}

	if (GC.getUnitInfo(eUnit).getEvasionProbability() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_EVADE_INTERCEPTION", GC.getUnitInfo(eUnit).getEvasionProbability()));
	}

	if (GC.getUnitInfo(eUnit).getWithdrawalProbability() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY", GC.getUnitInfo(eUnit).getWithdrawalProbability()));
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/10
	if (GC.getUnitInfo(eUnit).getWithdrawlProbDefensive() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_WITHDRAWL_PROBABILITY_DEFENSIVE", GC.getUnitInfo(eUnit).getWithdrawlProbDefensive()));
	}
	//<<<<	BUGFfH: End Add

	if (GC.getUnitInfo(eUnit).getCombatLimit() < GC.getMAX_HIT_POINTS() && GC.getUnitInfo(eUnit).getCombat() > 0 && !GC.getUnitInfo(eUnit).isOnlyDefensive())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_COMBAT_LIMIT", (100 * GC.getUnitInfo(eUnit).getCombatLimit()) / GC.getMAX_HIT_POINTS()));
	}

	if (GC.getUnitInfo(eUnit).getCollateralDamage() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_COLLATERAL_DAMAGE", (100 * GC.getUnitInfo(eUnit).getCollateralDamageLimit() / GC.getMAX_HIT_POINTS())));
	}

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getUnitCombatCollateralImmune(iI))
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/10/03
			//			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_COLLATERAL_IMMUNE", GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide()));
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_COLLATERAL_IMMUNE", getLinkedText((UnitCombatTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify
		}
	}

	if (GC.getUnitInfo(eUnit).getCityAttackModifier() == GC.getUnitInfo(eUnit).getCityDefenseModifier())
	{
		if (GC.getUnitInfo(eUnit).getCityAttackModifier() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CITY_STRENGTH_MOD", GC.getUnitInfo(eUnit).getCityAttackModifier()));
		}
	}
	else
	{
		if (GC.getUnitInfo(eUnit).getCityAttackModifier() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CITY_ATTACK_MOD", GC.getUnitInfo(eUnit).getCityAttackModifier()));
		}

		if (GC.getUnitInfo(eUnit).getCityDefenseModifier() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CITY_DEFENSE_MOD", GC.getUnitInfo(eUnit).getCityDefenseModifier()));
		}
	}

	if (GC.getUnitInfo(eUnit).getAnimalCombatModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ANIMAL_COMBAT_MOD", GC.getUnitInfo(eUnit).getAnimalCombatModifier()));
	}

	if (GC.getUnitInfo(eUnit).getDropRange() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_PARADROP_RANGE", GC.getUnitInfo(eUnit).getDropRange()));
	}

	if (GC.getUnitInfo(eUnit).getHillsDefenseModifier() == GC.getUnitInfo(eUnit).getHillsAttackModifier())
	{
		if (GC.getUnitInfo(eUnit).getHillsAttackModifier() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HILLS_STRENGTH", GC.getUnitInfo(eUnit).getHillsAttackModifier()));
		}
	}
	else
	{
		if (GC.getUnitInfo(eUnit).getHillsAttackModifier() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HILLS_ATTACK", GC.getUnitInfo(eUnit).getHillsAttackModifier()));
		}

		if (GC.getUnitInfo(eUnit).getHillsDefenseModifier() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_HILLS_DEFENSE", GC.getUnitInfo(eUnit).getHillsDefenseModifier()));
		}
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); ++iI)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/09/14
		/*
				if (GC.getUnitInfo(eUnit).getTerrainDefenseModifier(iI) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE", GC.getUnitInfo(eUnit).getTerrainDefenseModifier(iI), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide()));
				}

				if (GC.getUnitInfo(eUnit).getTerrainAttackModifier(iI) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ATTACK", GC.getUnitInfo(eUnit).getTerrainAttackModifier(iI), GC.getTerrainInfo((TerrainTypes) iI).getTextKeyWide()));
				}
		*/
		if (GC.getUnitInfo(eUnit).getTerrainAttackModifier(iI) == GC.getUnitInfo(eUnit).getTerrainDefenseModifier(iI))
		{
			if (GC.getUnitInfo(eUnit).getTerrainAttackModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_STRENGTH",
					GC.getUnitInfo(eUnit).getTerrainAttackModifier(iI),
					getLinkedText((TerrainTypes)iI).c_str()));
			}
		}
		else
		{
			if (GC.getUnitInfo(eUnit).getTerrainDefenseModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE",
					GC.getUnitInfo(eUnit).getTerrainDefenseModifier(iI),
					getLinkedText((TerrainTypes)iI).c_str()));
			}

			if (GC.getUnitInfo(eUnit).getTerrainAttackModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ATTACK",
					GC.getUnitInfo(eUnit).getTerrainAttackModifier(iI),
					getLinkedText((TerrainTypes)iI).c_str()));
			}
		}
		//<<<<	BUGFfH: End Modify
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/09/14
		/*
				if (GC.getUnitInfo(eUnit).getFeatureDefenseModifier(iI) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE", GC.getUnitInfo(eUnit).getFeatureDefenseModifier(iI), GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide()));
				}

				if (GC.getUnitInfo(eUnit).getFeatureAttackModifier(iI) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ATTACK", GC.getUnitInfo(eUnit).getFeatureAttackModifier(iI), GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide()));
				}
		*/
		if (GC.getUnitInfo(eUnit).getFeatureAttackModifier(iI) == GC.getUnitInfo(eUnit).getFeatureDefenseModifier(iI))
		{
			if (GC.getUnitInfo(eUnit).getFeatureAttackModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_STRENGTH",
					GC.getUnitInfo(eUnit).getFeatureAttackModifier(iI),
					getLinkedText((FeatureTypes)iI).c_str()));
			}
		}
		else
		{
			if (GC.getUnitInfo(eUnit).getFeatureDefenseModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE",
					GC.getUnitInfo(eUnit).getFeatureDefenseModifier(iI),
					getLinkedText((FeatureTypes)iI).c_str()));
			}

			if (GC.getUnitInfo(eUnit).getFeatureAttackModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ATTACK",
					GC.getUnitInfo(eUnit).getFeatureAttackModifier(iI),
					getLinkedText((FeatureTypes)iI).c_str()));
			}
		}
		//<<<<	BUGFfH: End Modify
	}

	for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) == GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI))
		{
			if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI), getLinkedText((UnitClassTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify
			}
		}
		else
		{
			if (GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ATTACK_MOD_VS_CLASS", GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI), GC.getUnitClassInfo((UnitClassTypes)iI).getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ATTACK_MOD_VS_CLASS", GC.getUnitInfo(eUnit).getUnitClassAttackModifier(iI), getLinkedText((UnitClassTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify
			}

			if (GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI) != 0)
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE_MOD_VS_CLASS", GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI), GC.getUnitClassInfo((UnitClassTypes) iI).getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENSE_MOD_VS_CLASS", GC.getUnitInfo(eUnit).getUnitClassDefenseModifier(iI), getLinkedText((UnitClassTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify
			}
		}
	}

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getUnitCombatModifier(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/10/03
			//			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", GC.getUnitInfo(eUnit).getUnitCombatModifier(iI), GC.getUnitCombatInfo((UnitCombatTypes) iI).getTextKeyWide()));
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE", GC.getUnitInfo(eUnit).getUnitCombatModifier(iI), getLinkedText((UnitCombatTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; ++iI)
	{
		if (GC.getUnitInfo(eUnit).getDomainModifier(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/10/03
			//			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE_NO_LINK", GC.getUnitInfo(eUnit).getDomainModifier(iI), GC.getDomainInfo((DomainTypes)iI).getTextKeyWide()));
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MOD_VS_TYPE_NO_LINK", GC.getUnitInfo(eUnit).getDomainModifier(iI), getLinkedText((DomainTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify
		}
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getTargetUnitClass(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
			szTempBuffer += getLinkedText((UnitClassTypes)iI);
			//<<<<	BUGFfH: End Modify
		}
	}

	if (!bFirst)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString()));
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getDefenderUnitClass(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
			szTempBuffer += getLinkedText((UnitClassTypes)iI);
			//<<<<	BUGFfH: End Modify
		}
	}

	if (!bFirst)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString()));
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getTargetUnitCombat(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
			szTempBuffer += getLinkedText((UnitCombatTypes)iI);
			//<<<<	BUGFfH: End Modify
		}
	}

	if (!bFirst)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_TARGETS_UNIT_FIRST", szTempBuffer.GetCString()));
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getDefenderUnitCombat(iI))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitCombatInfo((UnitCombatTypes)iI).getDescription());
			szTempBuffer += getLinkedText((UnitCombatTypes)iI);
			//<<<<	BUGFfH: End Modify
		}
	}

	if (!bFirst)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DEFENDS_UNIT_FIRST", szTempBuffer.GetCString()));
	}

	szTempBuffer.clear();
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitClassInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getFlankingStrikeUnitClass(iI) > 0)
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szTempBuffer += L", ";
			}

			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer += CvWString::format(L"<link=literal>%s</link>", GC.getUnitClassInfo((UnitClassTypes)iI).getDescription());
			szTempBuffer += getLinkedText((UnitClassTypes)iI);
			//<<<<	BUGFfH: End Modify
		}
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/14
	if (GC.getUnitInfo(eUnit).isIgnoreBuildingDefense())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IGNORE_BUILDING_DEFENSE"));
	}
	//<<<<	BUGFfH: End Add

	if (GC.getUnitInfo(eUnit).getBonusNearUF() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_BONUS_NEAR_UF", GC.getUnitInfo(eUnit).getBonusNearUF()));
	}

	if (GC.getUnitInfo(eUnit).isNoEquipment())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NO_EQUIPMENT"));
	}

	if (GC.getUnitInfo(eUnit).isScaleWithTech())
	{
		bool bFound = false;

		for (int i = 0; i < GC.getNumTechInfos(); ++i) {
			if (GC.getTechInfo((TechTypes)i).getIncreaseStrengthOfRegularUnits() > 0) {
				if (!bFound) {
					bFound = true;
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_SCALE_WITH_TECHS", GC.getTechInfo((TechTypes)i).getIncreaseStrengthOfRegularUnits()));
					szBuffer.append(getLinkedText((TechTypes)i));

				}
				else {
					szTempBuffer.Format(L", +%d ", GC.getTechInfo((TechTypes)i).getIncreaseStrengthOfRegularUnits());
					szBuffer.append(szTempBuffer);
					szBuffer.append(getLinkedText((TechTypes)i));
				}
			}
		}
	}

	if (!bFirst)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_FLANKING_STRIKES", szTempBuffer.GetCString()));
	}

	if (GC.getUnitInfo(eUnit).getBombRate() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_BOMB_RATE", ((GC.getUnitInfo(eUnit).getBombRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE())));
	}

	if (GC.getUnitInfo(eUnit).getBombardRate() > 0 && GC.getUnitInfo(eUnit).getBombardRange() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_BOMBARD_RANGE", GC.getUnitInfo(eUnit).getBombardRange()));

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_BOMBARD_RATE_NEW", ((GC.getUnitInfo(eUnit).getBombardRate() * 100) / GC.getMAX_CITY_DEFENSE_DAMAGE())));
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumPromotionInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getFreePromotions(iI))
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_STARTS_WITH").c_str());
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			setListHelp(szBuffer, szTempBuffer, CvWString::format(L"<link=literal>%s</link>", GC.getPromotionInfo((PromotionTypes) iI).getDescription()), L", ", bFirst);
			setListHelp(szBuffer, szTempBuffer, getLinkedText((PromotionTypes)iI), L", ", bFirst);
			//<<<<	BUGFfH: End Modify
			bFirst = false;
		}
	}
	/** disabled
		if (GC.getUnitInfo(eUnit).getExtraCost() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_EXTRA_COST", GC.getUnitInfo(eUnit).getExtraCost()));
		}
	**/
	if (bCivilopediaText)
	{
		// Trait
		for (int i = 0; i < GC.getNumTraitInfos(); ++i)
		{
			if (GC.getUnitInfo(eUnit).getProductionTraits((TraitTypes)i) != 0)
			{
				if (GC.getUnitInfo(eUnit).getProductionTraits((TraitTypes)i) == 100)
				{
					szBuffer.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/09/15
					//					szBuffer.append(gDLL->getText("TXT_KEY_DOUBLE_SPEED_TRAIT", GC.getTraitInfo((TraitTypes)i).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_DOUBLE_SPEED_TRAIT", getLinkedText((TraitTypes)i).c_str()));
					//<<<<	BUGFfH: End Modify
				}
				else
				{
					szBuffer.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/09/15
					//					szBuffer.append(gDLL->getText("TXT_KEY_PRODUCTION_MODIFIER_TRAIT", GC.getUnitInfo(eUnit).getProductionTraits((TraitTypes)i), GC.getTraitInfo((TraitTypes)i).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_PRODUCTION_MODIFIER_TRAIT",
						GC.getUnitInfo(eUnit).getProductionTraits((TraitTypes)i),
						getLinkedText((TraitTypes)i).c_str()));
					//<<<<	BUGFfH: End Modify
				}
			}
		}
	}

	//FfH: Added by Kael 08/04/2007
	for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getDamageTypeCombat((DamageTypes)iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_DAMAGE_TYPE_COMBAT", GC.getUnitInfo(eUnit).getDamageTypeCombat((DamageTypes)iI), GC.getDamageTypeInfo((DamageTypes)iI).getDescription()));
		}
	}
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (GC.getUnitInfo(eUnit).getBonusAffinity((BonusTypes)iI) != 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/09/15
			//			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_AFFINITY", GC.getUnitInfo(eUnit).getBonusAffinity((BonusTypes)iI), GC.getBonusInfo((BonusTypes)iI).getDescription()));
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_AFFINITY",
				GC.getUnitInfo(eUnit).getBonusAffinity((BonusTypes)iI),
				getLinkedText((BonusTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify
		}
	}

	/*************************************************************************************************/
	/**	ADDON (New Functions) Sephi                                     					        **/
	/*************************************************************************************************/

	if (bCivilopediaText)
	{
		if (GC.getUnitInfo(eUnit).getCombatAuraType() != NO_COMBATAURA)
		{
			CombatAuraClassTypes eCombatClass = (CombatAuraClassTypes)GC.getCombatAuraInfo((CombatAuraTypes)GC.getUnitInfo(eUnit).getCombatAuraType()).getCombatAuraClassType();
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_COMBAT_AURA", GC.getCombatAuraClassInfo(eCombatClass).getDescription()));
		}

		if (GC.getUnitInfo(eUnit).isArcaneAura())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_UNIT_ARCANE_AURA"));
		}
	}
	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/*************************************************************************************************/
	if (GC.getUnitInfo(eUnit).getCorporationSupportNeeded() != NO_CORPORATION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PEDIA_CORPORATIONSUPPORT_NEEDED", GC.getCorporationInfo((CorporationTypes)GC.getUnitInfo(eUnit).getCorporationSupportNeeded()).getDescription()));
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (GC.getUnitInfo(eUnit).getMinLevel() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CANT_BUILD"));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MIN_LEVEL", GC.getUnitInfo(eUnit).getMinLevel()));
	}
	if (GC.getUnitInfo(eUnit).isDisableUpgradeTo())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DISABLE_UPGRADE_TO"));
	}
	if (GC.getUnitInfo(eUnit).getFreePromotionPick() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_FREE_PROMOTION_PICK", GC.getUnitInfo(eUnit).getFreePromotionPick()));
	}

	if (GC.getUnitInfo(eUnit).getArcaneRange() > 1)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_SPELLCAST_RANGE", GC.getUnitInfo(eUnit).getArcaneRange()));
	}

	if (GC.getUnitInfo(eUnit).getGoldFromCombat() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_GOLD_FROM_COMBAT", GC.getUnitInfo(eUnit).getGoldFromCombat()));
	}
	if (GC.getUnitInfo(eUnit).getModifyGlobalCounter() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MESSAGE_MODIFY_GLOBAL_COUNTER", GC.getUnitInfo(eUnit).getModifyGlobalCounter()));
	}
	//>>>>	BUGFfH: Added by Denev 2009/09/10
	if (GC.getUnitInfo(eUnit).getDurationFromCombat() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DURATION_FROM_COMBAT", GC.getUnitInfo(eUnit).getDurationFromCombat()));
	}
	//<<<<	BUGFfH: End Add
	if (GC.getUnitInfo(eUnit).getUnitConvertFromCombat() != NO_UNIT)
	{
		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/09/19
		//		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CONVERT_FROM_COMBAT", GC.getUnitInfo(eUnit).getUnitConvertFromCombatChance(), GC.getUnitInfo((UnitTypes)GC.getUnitInfo(eUnit).getUnitConvertFromCombat()).getDescription()));
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CONVERT_FROM_COMBAT", GC.getUnitInfo(eUnit).getUnitConvertFromCombatChance(), getLinkedText((UnitTypes)GC.getUnitInfo(eUnit).getUnitConvertFromCombat()).c_str()));
		//<<<<	BUGFfH: End Modify
	}
	if (GC.getUnitInfo(eUnit).getUnitCreateFromCombat() != NO_UNIT)
	{
		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/09/19
		//		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CREATE_FROM_COMBAT", GC.getUnitInfo(eUnit).getUnitCreateFromCombatChance(), GC.getUnitInfo((UnitTypes)GC.getUnitInfo(eUnit).getUnitCreateFromCombat()).getDescription()));
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_CREATE_FROM_COMBAT", GC.getUnitInfo(eUnit).getUnitCreateFromCombatChance(), getLinkedText((UnitTypes)GC.getUnitInfo(eUnit).getUnitCreateFromCombat()).c_str()));
		//<<<<	BUGFfH: End Modify
	}
	if (GC.getUnitInfo(eUnit).getEnslavementChance() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ENSLAVEMENT_CHANCE", GC.getUnitInfo(eUnit).getEnslavementChance()));
	}
	if (GC.getUnitInfo(eUnit).getPromotionFromCombat() != NO_PROMOTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_PROMOTION_FROM_COMBAT", GC.getPromotionInfo((PromotionTypes)GC.getUnitInfo(eUnit).getPromotionFromCombat()).getDescription()));
	}
	if (GC.getUnitInfo(eUnit).isImmortal())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_IMMORTAL_PEDIA"));
	}
	//>>>>	BUGFfH: Added by Denev 2009/09/08
	if (GC.getUnitInfo(eUnit).isExplodeInCombat())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_EXPLODE_IN_COMBAT"));
	}
	//<<<<	BUGFfH: End Add
	if (GC.getUnitInfo(eUnit).isNoWarWeariness())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NO_WAR_WEARINESS"));
	}
	if (GC.getUnitInfo(eUnit).getDiploVoteType() != NO_VOTESOURCE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DIPLO_VOTE_TYPE", GC.getVoteSourceInfo((VoteSourceTypes)GC.getUnitInfo(eUnit).getDiploVoteType()).getDescription()));
	}
	if (GC.getUnitInfo(eUnit).isAbandon())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_ABANDON"));
	}
	for (int iJ = 0; iJ < GC.getNumSpellInfos(); iJ++)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/09/16
		/*
				if (GC.getSpellInfo((SpellTypes)iJ).getCivilizationPrereq() == NO_CIVILIZATION)
				{
					if (GC.getSpellInfo((SpellTypes)iJ).getReligionPrereq() == NO_RELIGION)
					{
						if (GC.getSpellInfo((SpellTypes)iJ).getUnitPrereq() == eUnit)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_UNIT_SPELL_ABILITY", GC.getSpellInfo((SpellTypes)iJ).getDescription()));
						}
						if (GC.getSpellInfo((SpellTypes)iJ).getUnitClassPrereq() == GC.getUnitInfo(eUnit).getUnitClassType())
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_UNIT_SPELL_ABILITY", GC.getSpellInfo((SpellTypes)iJ).getDescription()));
						}
					}
				}
		*/
		/**
				if (GC.getSpellInfo((SpellTypes)iJ).getUnitPrereq() == eUnit)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_SPELL_ABILITY_NO_LINK", getLinkedText((SpellTypes)iJ).c_str()));
				}
		**/
		const int iUnitClass = GC.getUnitInfo(eUnit).getUnitClassType();
		if (GC.getSpellInfo((SpellTypes)iJ).getUnitClassPrereq() == iUnitClass || GC.getSpellInfo((SpellTypes)iJ).getUnitPrereq() == eUnit)
		{
			do
			{
				//civilization requirement
				const CivilizationTypes eActiveCiv = GC.getGameINLINE().getActiveCivilizationType();
				const CivilizationTypes ePrereqCiv = (CivilizationTypes)GC.getSpellInfo((SpellTypes)iJ).getCivilizationPrereq();

				if (ePrereqCiv != NO_CIVILIZATION)
				{
					if (eActiveCiv != NO_CIVILIZATION && eActiveCiv != ePrereqCiv)
					{
						continue;
					}
					if (eUnit != GC.getCivilizationInfo(ePrereqCiv).getCivilizationUnits(iUnitClass))
					{
						continue;
					}
				}


				//race requirement
				const PromotionTypes ePromotionPrereq1 = (PromotionTypes)GC.getSpellInfo((SpellTypes)iJ).getPromotionPrereq1();
				const PromotionTypes ePromotionPrereq2 = (PromotionTypes)GC.getSpellInfo((SpellTypes)iJ).getPromotionPrereq2();
				const PromotionTypes ePromotionPrereq3 = (PromotionTypes)GC.getSpellInfo((SpellTypes)iJ).getPromotionPrereq3();
				const PromotionTypes ePromotionPrereq4 = (PromotionTypes)GC.getSpellInfo((SpellTypes)iJ).getPromotionPrereq4();
				PromotionTypes eRacePrereq = NO_PROMOTION;
				if (ePromotionPrereq4 != NO_PROMOTION)
				{
					eRacePrereq = GC.getPromotionInfo(ePromotionPrereq4).isRace() ? ePromotionPrereq4 : eRacePrereq;
				}
				if (ePromotionPrereq3 != NO_PROMOTION)
				{
					eRacePrereq = GC.getPromotionInfo(ePromotionPrereq3).isRace() ? ePromotionPrereq3 : eRacePrereq;
				}
				if (ePromotionPrereq2 != NO_PROMOTION)
				{
					eRacePrereq = GC.getPromotionInfo(ePromotionPrereq2).isRace() ? ePromotionPrereq2 : eRacePrereq;
				}
				if (ePromotionPrereq1 != NO_PROMOTION)
				{
					eRacePrereq = GC.getPromotionInfo(ePromotionPrereq1).isRace() ? ePromotionPrereq1 : eRacePrereq;
				}

				if (eRacePrereq != NO_PROMOTION)
				{
					if (!GC.getUnitInfo(eUnit).getFreePromotions(eRacePrereq))
					{
						continue;
					}
				}

				//State Religion Prereq
				const PlayerTypes eActivePlayer = GC.getGameINLINE().getActivePlayer();
				const ReligionTypes ePrereqStateReligion = (ReligionTypes)GC.getSpellInfo((SpellTypes)iJ).getStateReligionPrereq();

				if (ePrereqStateReligion != NO_RELIGION)
				{
					if (eActivePlayer != NO_PLAYER && GET_PLAYER(eActivePlayer).getStateReligion() != ePrereqStateReligion)
					{
						continue;
					}
				}
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_SPELL_ABILITY_NO_LINK", getLinkedText((SpellTypes)iJ).c_str()));
			} while (false);
		}
		//<<<<	BUGFfH: End Modify
	}
	//FfH: End Add

	bFirst = true;
	for (int i = 0; i < NUM_COMMERCE_TYPES; ++i) {
		int iCommerceChange = GC.getUnitInfo(eUnit).getCityCommerceChanges(i);
		if (iCommerceChange != 0) {
			if (bFirst) {
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_CITYCOMMERCE_BASE_HELP"));
				szBuffer.append(CvWString::format(L"%s%d%c", (iCommerceChange > 0) ? "+" : "", iCommerceChange, GC.getCommerceInfo((CommerceTypes)i).getChar()));
				bFirst = false;
			}
			else {
				szBuffer.append(L", ");
				szBuffer.append(CvWString::format(L"%s%d%c", (iCommerceChange > 0) ? "+" : "", iCommerceChange, GC.getCommerceInfo((CommerceTypes)i).getChar()));
			}
		}
	}
	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/*************************************************************************************************/
	for (int i = 0; i < GC.getNumCorporationInfos(); i++)
	{
		if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isHasCorporationSupport((CorporationTypes)i))
		{
			if (GC.getUnitInfo(eUnit).getCorporationSupport((CorporationTypes)i) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getUnitInfo(eUnit).getCorporationSupport((CorporationTypes)i)));
			}

			if (GC.getUnitInfo(eUnit).getCorporationSupportMultiplier((CorporationTypes)i) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT_MODIFIER", GC.getCorporationInfo((CorporationTypes)i).getDescription(), GC.getUnitInfo(eUnit).getCorporationSupportMultiplier((CorporationTypes)i)));
			}
		}
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/
	if (!CvWString(GC.getUnitInfo(eUnit).getHelp()).empty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(GC.getUnitInfo(eUnit).getHelp());
	}
}


void CvGameTextMgr::setUnitHelp(CvWStringBuffer& szBuffer, UnitTypes eUnit, bool bCivilopediaText, bool bStrategyText, bool bTechChooserText, CvCity* pCity)
{
	PROFILE_FUNC();

	CvWString szTempBuffer;
	PlayerTypes ePlayer;
	bool bFirst;
	int iProduction;
	int iI;

	if (NO_UNIT == eUnit)
	{
		return;
	}

	if (pCity != NULL)
	{
		ePlayer = pCity->getOwnerINLINE();
	}
	else
	{
		ePlayer = GC.getGameINLINE().getActivePlayer();
	}

	if (!bCivilopediaText)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eUnit).getDescription());
		szTempBuffer.assign(getLinkedText(eUnit));
		//<<<<	BUGFfH: End Modify
		szBuffer.append(szTempBuffer);

		if (GC.getUnitInfo(eUnit).getUnitCombatType() != NO_UNITCOMBAT)
		{
			//>>>>	BUGFfH: Modified by Denev 2009/10/03
			//			szTempBuffer.Format(L" (%s)", GC.getUnitCombatInfo((UnitCombatTypes) GC.getUnitInfo(eUnit).getUnitCombatType()).getDescription());
			szTempBuffer.Format(L" (%s)", getLinkedText((UnitCombatTypes)GC.getUnitInfo(eUnit).getUnitCombatType()).c_str());
			//<<<<	BUGFfH: End Modify
			szBuffer.append(szTempBuffer);
		}
	}

	// test for unique unit
	UnitClassTypes eUnitClass = (UnitClassTypes)GC.getUnitInfo(eUnit).getUnitClassType();
	UnitTypes eDefaultUnit = (UnitTypes)GC.getUnitClassInfo(eUnitClass).getDefaultUnitIndex();

	//FfH: Modified by Kael 01/31/2009
	//	if (NO_UNIT != eDefaultUnit && eDefaultUnit != eUnit)
	//	{
	//		for (iI  = 0; iI < GC.getNumCivilizationInfos(); ++iI)
	//		{
	//			UnitTypes eUniqueUnit = (UnitTypes)GC.getCivilizationInfo((CivilizationTypes)iI).getCivilizationUnits((int)eUnitClass);
	//			if (eUniqueUnit == eUnit)
	//			{
	//				szBuffer.append(NEWLINE);
	//				szBuffer.append(gDLL->getText("TXT_KEY_UNIQUE_UNIT", GC.getCivilizationInfo((CivilizationTypes)iI).getTextKeyWide()));
	//			}
	//		}
	//		szBuffer.append(NEWLINE);
	//		szBuffer.append(gDLL->getText("TXT_KEY_REPLACES_UNIT", GC.getUnitInfo(eDefaultUnit).getTextKeyWide()));
	//>>>>	BUGFfH: Modified by Denev 2009/09/16
	/*
		bool bUnique = false;
		int iCount = 0;
		for (iI  = 0; iI < GC.getNumCivilizationInfos(); ++iI)
		{
			UnitTypes eUniqueUnit = (UnitTypes)GC.getCivilizationInfo((CivilizationTypes)iI).getCivilizationUnits((int)eUnitClass);
			if (eUniqueUnit == NO_UNIT)
			{
				iCount += 1;
			}
		}
		if (iCount > 19)
		{
			bUnique = true;
		}
		if (bUnique || (NO_UNIT != eDefaultUnit && eDefaultUnit != eUnit))
		{
			for (iI = 0; iI < GC.getNumCivilizationInfos(); ++iI)
			{
				UnitTypes eUniqueUnit = (UnitTypes)GC.getCivilizationInfo((CivilizationTypes)iI).getCivilizationUnits((int)eUnitClass);
				if (eUniqueUnit == eUnit)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIQUE_UNIT", GC.getCivilizationInfo((CivilizationTypes)iI).getTextKeyWide()));
				}
			}

			if (!bUnique)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_REPLACES_UNIT", GC.getUnitInfo(eDefaultUnit).getTextKeyWide()));
			}
	*/
	CivilizationTypes eUniqueCivilization = (CivilizationTypes)GC.getUnitInfo(eUnit).getPrereqCiv();
	if (eUniqueCivilization == NO_CIVILIZATION)
	{
		int iCount = 0;
		for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); ++iCiv)
		{
			UnitTypes eUniqueUnit = (UnitTypes)GC.getCivilizationInfo((CivilizationTypes)iCiv).getCivilizationUnits((int)eUnitClass);
			if (eUniqueUnit != NO_UNIT)
			{
				eUniqueCivilization = (CivilizationTypes)iCiv;
				++iCount;
			}
		}
		if (iCount != 1)
		{
			eUniqueCivilization = NO_CIVILIZATION;
		}
	}
	if (eUniqueCivilization != NO_CIVILIZATION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIQUE_UNIT", getLinkedText(eUniqueCivilization).c_str()));
	}
	else if (NO_UNIT != eDefaultUnit && eDefaultUnit != eUnit)
	{
		for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); ++iCiv)
		{
			UnitTypes eUniqueUnit = (UnitTypes)GC.getCivilizationInfo((CivilizationTypes)iCiv).getCivilizationUnits((int)eUnitClass);
			if (eUniqueUnit == eUnit)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIQUE_UNIT", getLinkedText((CivilizationTypes)iCiv).c_str()));
			}
		}
		//		szBuffer.append(NEWLINE);
		//		szBuffer.append(gDLL->getText("TXT_KEY_REPLACES_UNIT", getLinkedText(eDefaultUnit).c_str()));
		//<<<<	BUGFfH: End Modify
		//FfH: End Modify

	}

	//SpyFanatic: show Tier
	//if (bCivilopediaText)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_TIER_PEDIA", GC.getUnitInfo(eUnit).getTier()));
	}

	if (isWorldUnitClass(eUnitClass))
	{
		if (pCity == NULL)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_WORLD_UNIT_ALLOWED", GC.getUnitClassInfo(eUnitClass).getMaxGlobalInstances()));
		}
		else
		{
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_WORLD_UNIT_LEFT", (GC.getUnitClassInfo(eUnitClass).getMaxGlobalInstances() - (ePlayer != NO_PLAYER ? GC.getGameINLINE().getUnitClassCreatedCount(eUnitClass) + GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getUnitClassMaking(eUnitClass) : 0))));
		}
	}

	if (isTeamUnitClass(eUnitClass))
	{
		if (pCity == NULL)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_TEAM_UNIT_ALLOWED", GC.getUnitClassInfo(eUnitClass).getMaxTeamInstances()));
		}
		else
		{
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_TEAM_UNIT_LEFT", (GC.getUnitClassInfo(eUnitClass).getMaxTeamInstances() - (ePlayer != NO_PLAYER ? GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getUnitClassCountPlusMaking(eUnitClass) : 0))));
		}
	}

	if (isNationalUnitClass(eUnitClass))
	{
		if (pCity == NULL)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NATIONAL_UNIT_ALLOWED", GC.getUnitClassInfo(eUnitClass).getMaxPlayerInstances()));
		}
		else
		{
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NATIONAL_UNIT_LEFT", (GC.getUnitClassInfo(eUnitClass).getMaxPlayerInstances() - (ePlayer != NO_PLAYER ? GET_PLAYER(ePlayer).getUnitClassCountPlusMaking(eUnitClass) : 0))));
		}
	}

	if (0 != GC.getUnitClassInfo(eUnitClass).getInstanceCostModifier())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_INSTANCE_COST_MOD", GC.getUnitClassInfo(eUnitClass).getInstanceCostModifier()));
	}

	setBasicUnitHelp(szBuffer, eUnit, bCivilopediaText);

	if (GC.getGameINLINE().isDebugMode())
	{
		if (NULL != pCity)
		{
			szTempBuffer.Format(L"\n AI UnitValue: %d", pCity->AI_getUnitValue(eUnit));
			szBuffer.append(szTempBuffer);
		}
	}

	if ((pCity == NULL) || !(pCity->canTrain(eUnit)))
	{
		if (pCity != NULL)
		{
			if (GC.getGameINLINE().isNoNukes())
			{
				if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_NO_NUKES"));
				}
			}
		}

		if (GC.getUnitInfo(eUnit).getHolyCity() != NO_RELIGION)
		{
			if ((pCity == NULL) || !(pCity->isHolyCity((ReligionTypes)(GC.getUnitInfo(eUnit).getHolyCity()))))
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/09/18
				//				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_HOLY_CITY", GC.getReligionInfo((ReligionTypes)(GC.getUnitInfo(eUnit).getHolyCity())).getChar()));
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_HOLY_CITY", GC.getReligionInfo((ReligionTypes)(GC.getUnitInfo(eUnit).getHolyCity())).getHolyCityChar()));
				//<<<<	BUGFfH: End Modify
			}
		}

		bFirst = true;

		if (GC.getUnitInfo(eUnit).getSpecialUnitType() != NO_SPECIALUNIT)
		{
			if ((pCity == NULL) || !(GC.getGameINLINE().isSpecialUnitValid((SpecialUnitTypes)(GC.getUnitInfo(eUnit).getSpecialUnitType()))))
			{
				for (iI = 0; iI < GC.getNumProjectInfos(); ++iI)
				{
					if (GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialUnit() == GC.getUnitInfo(eUnit).getSpecialUnitType())
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						CvWString szProject;
						//>>>>	BUGFfH: Modified by Denev 2009/09/13
						//						szProject.Format(L"<link=literal>%s</link>", GC.getProjectInfo((ProjectTypes)iI).getDescription());
						szProject = getLinkedText((ProjectTypes)iI, false);
						//<<<<	BUGFfH: End Modify
						setListHelp(szBuffer, szTempBuffer, szProject, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}
			}
		}

		if (!bFirst)
		{
			szBuffer.append(ENDCOLR);
		}

		bFirst = true;

		if (GC.getUnitInfo(eUnit).getNukeRange() != -1)
		{
			if (NULL == pCity || !GC.getGameINLINE().isNukesValid())
			{
				for (iI = 0; iI < GC.getNumProjectInfos(); ++iI)
				{
					if (GC.getProjectInfo((ProjectTypes)iI).isAllowsNukes())
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						CvWString szProject;
						//>>>>	BUGFfH: Modified by Denev 2009/09/13
						//						szProject.Format(L"<link=literal>%s</link>", GC.getProjectInfo((ProjectTypes)iI).getDescription());
						szProject = getLinkedText((ProjectTypes)iI, false);
						//<<<<	BUGFfH: End Modify
						setListHelp(szBuffer, szTempBuffer, szProject, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}

				for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
				{
					if (GC.getBuildingInfo((BuildingTypes)iI).isAllowsNukes())
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						CvWString szBuilding;
						//>>>>	BUGFfH: Modified by Denev 2009/09/13
						//						szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingInfo((BuildingTypes)iI).getDescription());
						szBuilding = getLinkedText((BuildingTypes)iI, false);
						//<<<<	BUGFfH: End Modify
						setListHelp(szBuffer, szTempBuffer, szBuilding, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}
			}
		}

		if (!bFirst)
		{
			szBuffer.append(ENDCOLR);
		}

		if (!bCivilopediaText)
		{
			if (GC.getUnitInfo(eUnit).getPrereqBuilding() != NO_BUILDING)
			{
				if ((pCity == NULL) || (pCity->getNumBuilding((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding())) <= 0))
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getBuildingInfo((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding())).getTextKeyWide()));
				}
			}

			//FfH: Added by Kael 08/04/2007
			if (GC.getUnitInfo(eUnit).getPrereqBuildingClass() != NO_BUILDINGCLASS)
			{
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				/*
								if ((pCity == NULL) || (!pCity->isHasBuildingClass(GC.getUnitInfo(eUnit).getPrereqBuildingClass())))
								{
									szBuffer.append(NEWLINE);
									szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getBuildingClassInfo((BuildingClassTypes)(GC.getUnitInfo(eUnit).getPrereqBuildingClass())).getTextKeyWide()));
								}
				*/
				//get list of all available civilizations which can use the unit.
				std::list<CivilizationTypes> aeAvailableCiv;
				CivilizationTypes eActiveCiv = GC.getGameINLINE().getActiveCivilizationType();
				if (pCity != NULL && pCity->getCivilizationType() != NO_CIVILIZATION)
				{
					//Assimilated city requires original civilization's building.
					eActiveCiv = pCity->getCivilizationType();
				}
				getAvailableCivs(aeAvailableCiv, eUnit, eActiveCiv);

				//collect prereq buildings on each available civilization
				std::list<BuildingTypes> aeBuildingPrereq;
				const int iBuildingClassPrereq = GC.getUnitInfo(eUnit).getPrereqBuildingClass();
				for (std::list<CivilizationTypes>::iterator it = aeAvailableCiv.begin(); it != aeAvailableCiv.end(); it++)
				{
					aeBuildingPrereq.push_back((BuildingTypes)GC.getCivilizationInfo(*it).getCivilizationBuildings(iBuildingClassPrereq));
				}
				//sort and erase repeated items
				aeBuildingPrereq.sort();
				aeBuildingPrereq.unique();
				aeBuildingPrereq.remove(NO_BUILDING);

				if (aeBuildingPrereq.size() == 1)
				{
					szTempBuffer.assign(GC.getBuildingInfo(*(aeBuildingPrereq.begin())).getDescription());
				}
				else
				{
					szTempBuffer.assign(GC.getBuildingClassInfo((BuildingClassTypes)iBuildingClassPrereq).getDescription());
				}

				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", szTempBuffer.c_str()));
				//<<<<	BUGFfH: End Modify
			}
			if (GC.getUnitInfo(eUnit).getPrereqAlignment() != NO_ALIGNMENT)
			{
				if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_PLAYER(ePlayer).getAlignment() != GC.getUnitInfo(eUnit).getPrereqAlignment())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_PREREQ_ALIGNMENT", GC.getAlignmentInfo((AlignmentTypes)GC.getUnitInfo(eUnit).getPrereqAlignment()).getDescription()));
				}
			}
			if (GC.getUnitInfo(eUnit).getPrereqCivic() != NO_CIVIC)
			{
				bool bValid = false;
				if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
				{
					for (iI = 0; iI < GC.getDefineINT("MAX_CIVIC_OPTIONS"); iI++)
					{
						if (GET_PLAYER(ePlayer).getCivics((CivicOptionTypes)iI) == GC.getUnitInfo(eUnit).getPrereqCivic())
						{
							bValid = true;
						}
					}
				}
				if (bValid == false)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getCivicInfo((CivicTypes)(GC.getUnitInfo(eUnit).getPrereqCivic())).getTextKeyWide()));
				}
			}
			//>>>>	BUGFfH: Moved to below(*18) by Denev 2009/10/03
			/*
						if (GC.getUnitInfo(eUnit).getPrereqGlobalCounter() != 0)
						{
							if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GC.getGameINLINE().getGlobalCounter() < GC.getUnitInfo(eUnit).getPrereqGlobalCounter())
							{
								szBuffer.append(NEWLINE);
								szBuffer.append(gDLL->getText("TXT_KEY_UNIT_PREREQ_GLOBAL_COUNTER", GC.getUnitInfo(eUnit).getPrereqGlobalCounter()));
							}
						}
			*/
			//<<<<	BUGFfH: End Move
			//FfH: End Add

			if (!bTechChooserText)
			{
				if (GC.getUnitInfo(eUnit).getPrereqAndTech() != NO_TECH)
				{
					if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTech()))))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getTechInfo((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTech())).getTextKeyWide()));
					}
				}
			}

			bFirst = true;

			for (iI = 0; iI < GC.getNUM_UNIT_AND_TECH_PREREQS(); ++iI)
			{
				if (GC.getUnitInfo(eUnit).getPrereqAndTechs(iI) != NO_TECH)
				{
					if (bTechChooserText || GC.getGameINLINE().getActivePlayer() == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTechs(iI)))))
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						setListHelp(szBuffer, szTempBuffer, GC.getTechInfo(((TechTypes)(GC.getUnitInfo(eUnit).getPrereqAndTechs(iI)))).getDescription(), gDLL->getText("TXT_KEY_AND").c_str(), bFirst);
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer.append(ENDCOLR);
			}

			if (GC.getUnitInfo(eUnit).getPrereqAndBonus() != NO_BONUS)
			{
				if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)GC.getUnitInfo(eUnit).getPrereqAndBonus())))
				{
					szBuffer.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/01
					//					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getBonusInfo((BonusTypes)(GC.getUnitInfo(eUnit).getPrereqAndBonus())).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES"));
					szBuffer.append(ENDCOLR);
					szBuffer.append(GC.getBonusInfo((BonusTypes)GC.getUnitInfo(eUnit).getPrereqAndBonus()).getChar());
					//<<<<	BUGFfH: End Modify
				}
			}

			bFirst = true;

			for (iI = 0; iI < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); ++iI)
			{
				if (GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI) != NO_BONUS)
				{
					if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI))))
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						//>>>>	BUGFfH: Modified by Denev 2009/10/01
						//						setListHelp(szBuffer, szTempBuffer, GC.getBonusInfo((BonusTypes) GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI)).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						CvWString szListItem;
						szListItem.Format(L"%c", GC.getBonusInfo((BonusTypes)GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI)).getChar());
						setListHelp(szBuffer, szTempBuffer, szListItem, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						//<<<<	BUGFfH: End Modify
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer.append(ENDCOLR);
			}
			//>>>>	BUGFfH: Added by Denev 2009/09/25
			if (NO_RELIGION != GC.getUnitInfo(eUnit).getStateReligion()
				&& (NO_PLAYER == ePlayer || GC.getUnitInfo(eUnit).getStateReligion() != GET_PLAYER(ePlayer).getStateReligion()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_REQ_STATE_RELIGION", GC.getReligionInfo((ReligionTypes)GC.getUnitInfo(eUnit).getStateReligion()).getChar()));
			}
			else if (NO_RELIGION != GC.getUnitInfo(eUnit).getPrereqReligion()
				&& (pCity == NULL || !(pCity->isHasReligion((ReligionTypes)GC.getUnitInfo(eUnit).getPrereqReligion()))))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_REQ_CITY_HAS_RELIGION", GC.getReligionInfo((ReligionTypes)GC.getUnitInfo(eUnit).getPrereqReligion()).getChar()));
			}
			//<<<<	BUGFfH: End Add
		}

		//>>>>	BUGFfH: Moved from above(*18) by Denev 2009/10/03
		if (GC.getUnitInfo(eUnit).getPrereqGlobalCounter() != 0)
		{
			if (bCivilopediaText
				|| GC.getGameINLINE().getActivePlayer() == NO_PLAYER
				|| GC.getGameINLINE().getGlobalCounter() < GC.getUnitInfo(eUnit).getPrereqGlobalCounter())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_PREREQ_GLOBAL_COUNTER", GC.getUnitInfo(eUnit).getPrereqGlobalCounter()));
			}
		}
		//<<<<	BUGFfH: End Move
	}

	//>>>>	BUGFfH: Modified by Denev 2009/09/24
	/*	Civilopedia unit tooltip shows unit production cost.(1/2)
		if (!bCivilopediaText && GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
	*/
	if (!bCivilopediaText && GC.getUnitInfo(eUnit).getProductionCost() >= 0)
	{
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			//<<<<	BUGFfH: End Modify
			if (pCity == NULL)
			{
				szTempBuffer.Format(L"%s%d%c", NEWLINE, GET_PLAYER(ePlayer).getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				szBuffer.append(szTempBuffer);
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_TURNS", pCity->getProductionTurnsLeft(eUnit, ((gDLL->ctrlKey() || !(gDLL->shiftKey())) ? 0 : pCity->getOrderQueueLength())), pCity->getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar()));

				iProduction = pCity->getUnitProduction(eUnit);

				if (iProduction > 0)
				{
					szTempBuffer.Format(L" - %d/%d%c", iProduction, pCity->getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer.append(szTempBuffer);
				}
				else
				{
					szTempBuffer.Format(L" - %d%c", pCity->getProductionNeeded(eUnit), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer.append(szTempBuffer);
				}
			}
			//>>>>	BUGFfH: Added by Denev 2009/09/24
			/*	Civilopedia unit tooltip shows unit production cost.(2/2)	*/
		}
		else
		{
			szTempBuffer.Format(L"%s%d%c", NEWLINE, GC.getUnitInfo(eUnit).getProductionCost(), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
			szBuffer.append(szTempBuffer);
		}
		//<<<<	BUGFfH: End Add
	}

	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		if (GC.getUnitInfo(eUnit).getBonusProductionModifier(iI) != 0)
		{
			if (pCity != NULL)
			{
				if (pCity->hasBonus((BonusTypes)iI))
				{
					szBuffer.append(gDLL->getText("TXT_KEY_COLOR_POSITIVE"));
				}
				else
				{
					szBuffer.append(gDLL->getText("TXT_KEY_COLOR_NEGATIVE"));
				}
			}
			if (!bCivilopediaText)
			{
				szBuffer.append(L" (");
			}
			else
			{
				szTempBuffer.Format(L"%s%c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szTempBuffer);
				szBuffer.append(szTempBuffer);
			}
			if (GC.getUnitInfo(eUnit).getBonusProductionModifier(iI) == 100)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_DOUBLE_SPEED", GC.getBonusInfo((BonusTypes)iI).getTextKeyWide()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_UNIT_BUILDS_FASTER", GC.getUnitInfo(eUnit).getBonusProductionModifier(iI), GC.getBonusInfo((BonusTypes)iI).getTextKeyWide()));
			}
			if (!bCivilopediaText)
			{
				szBuffer.append(L")");
			}
			if (pCity != NULL)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_COLOR_REVERT"));
			}
		}
	}

	if (bStrategyText)
	{
		if (!CvWString(GC.getUnitInfo(eUnit).getStrategy()).empty())
		{
			if ((ePlayer == NO_PLAYER) || GET_PLAYER(ePlayer).isOption(PLAYEROPTION_ADVISOR_HELP))
			{
				szBuffer.append(SEPARATOR);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_SIDS_TIPS"));
				szBuffer.append(L'\"');
				szBuffer.append(GC.getUnitInfo(eUnit).getStrategy());
				szBuffer.append(L'\"');
			}
		}
	}

	if (bCivilopediaText)
	{
		if (eDefaultUnit == eUnit)
		{
			for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
			{
				if (iI != eUnit)
				{
					if (eUnitClass == GC.getUnitInfo((UnitTypes)iI).getUnitClassType())
					{
						szBuffer.append(NEWLINE);
						//>>>>	BUGFfH: Modified by Denev 2009/09/13
						//						szBuffer.append(gDLL->getText("TXT_KEY_REPLACED_BY_UNIT", GC.getUnitInfo((UnitTypes)iI).getTextKeyWide()));
						szBuffer.append(gDLL->getText("TXT_KEY_REPLACED_BY_UNIT", getLinkedText((UnitTypes)iI).c_str()));
						//<<<<	BUGFfH: End Modify
					}
				}
			}
		}
	}

	if (pCity != NULL)
	{
		if ((gDLL->getChtLvl() > 0) && gDLL->ctrlKey())
		{
			szBuffer.append(NEWLINE);
			for (int iUnitAI = 0; iUnitAI < NUM_UNITAI_TYPES; iUnitAI++)
			{
				int iTempValue = GET_PLAYER(pCity->getOwner()).AI_unitValue(eUnit, (UnitAITypes)iUnitAI, pCity->area());
				if (iTempValue != 0)
				{
					CvWString szTempString;
					getUnitAIString(szTempString, (UnitAITypes)iUnitAI);
					szBuffer.append(CvWString::format(L"(%s : %d) ", szTempString.GetCString(), iTempValue));
				}
			}
		}
	}
}

void CvGameTextMgr::setBuildingHelp(CvWStringBuffer& szBuffer, BuildingTypes eBuilding, bool bCivilopediaText, bool bStrategyText, bool bTechChooserText, CvCity* pCity)
{
	PROFILE_FUNC();

	CvWString szFirstBuffer;
	CvWString szTempBuffer;
	BuildingTypes eLoopBuilding;
	UnitTypes eGreatPeopleUnit;
	PlayerTypes ePlayer;
	bool bFirst;
	int iProduction;
	int iLast;
	int iI;

	if (NO_BUILDING == eBuilding)
	{
		return;
	}

	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);


	if (pCity != NULL)
	{
		ePlayer = pCity->getOwnerINLINE();
	}
	else
	{
		ePlayer = GC.getGameINLINE().getActivePlayer();
	}

	if (!bCivilopediaText)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_BUILDING_TEXT"), kBuilding.getDescription());
		szTempBuffer.assign(getLinkedText(eBuilding));
		//<<<<	BUGFfH: End Modify

		szBuffer.append(szTempBuffer);

		int iHappiness;
		if (NULL != pCity)
		{
			iHappiness = pCity->getBuildingHappiness(eBuilding);
		}
		else
		{
			iHappiness = kBuilding.getHappiness();
		}

		if (iHappiness != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(iHappiness), ((iHappiness > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer.append(szTempBuffer);
		}

		int iHealth;
		if (NULL != pCity)
		{
			iHealth = pCity->getBuildingGoodHealth(eBuilding);
		}
		else
		{
			iHealth = kBuilding.getHealth();
			if (ePlayer != NO_PLAYER)
			{
				if (eBuilding == GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(kBuilding.getBuildingClassType()))
				{
					iHealth += GET_PLAYER(ePlayer).getExtraBuildingHealth(eBuilding);
				}
			}
		}
		if (iHealth != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(iHealth), ((iHealth > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer.append(szTempBuffer);
		}

		iHealth = 0;
		if (NULL != pCity)
		{
			iHealth = pCity->getBuildingBadHealth(eBuilding);
		}
		if (iHealth != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(iHealth), ((iHealth > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer.append(szTempBuffer);
		}

		int aiYields[NUM_YIELD_TYPES];
		for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
		{
			aiYields[iI] = kBuilding.getYieldChange(iI);

			if (NULL != pCity)
			{
				aiYields[iI] += pCity->getBuildingYieldChange((BuildingClassTypes)kBuilding.getBuildingClassType(), (YieldTypes)iI);
			}
		}
		setYieldChangeHelp(szBuffer, L", ", L"", L"", aiYields, false, false);

		int aiCommerces[NUM_COMMERCE_TYPES];
		for (int iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
		{
			if ((NULL != pCity) && (pCity->getNumBuilding(eBuilding) > 0))
			{
				aiCommerces[iI] = pCity->getBuildingCommerceByBuilding((CommerceTypes)iI, eBuilding);
			}
			else
			{
				aiCommerces[iI] = kBuilding.getCommerceChange(iI);
				aiCommerces[iI] += kBuilding.getObsoleteSafeCommerceChange(iI);
			}
		}
		setCommerceChangeHelp(szBuffer, L", ", L"", L"", aiCommerces, false, false);

		setYieldChangeHelp(szBuffer, L", ", L"", L"", kBuilding.getYieldModifierArray(), true, bCivilopediaText);
		setCommerceChangeHelp(szBuffer, L", ", L"", L"", kBuilding.getCommerceModifierArray(), true, bCivilopediaText);

		if (kBuilding.getGreatPeopleRateChange() != 0)
		{
			szTempBuffer.Format(L", %s%d%c", ((kBuilding.getGreatPeopleRateChange() > 0) ? "+" : ""), kBuilding.getGreatPeopleRateChange(), gDLL->getSymbolID(GREAT_PEOPLE_CHAR));
			szBuffer.append(szTempBuffer);

			if (kBuilding.getGreatPeopleUnitClass() != NO_UNITCLASS)
			{
				if (ePlayer != NO_PLAYER)
				{
					eGreatPeopleUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationUnits(kBuilding.getGreatPeopleUnitClass())));
				}
				else
				{
					eGreatPeopleUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)kBuilding.getGreatPeopleUnitClass()).getDefaultUnitIndex();
				}

				if (eGreatPeopleUnit != NO_UNIT)
				{
					szTempBuffer.Format(L" (%s)", GC.getUnitInfo(eGreatPeopleUnit).getDescription());
					szBuffer.append(szTempBuffer);
				}
			}
		}
	}

	// BUG - Building Actual Effects - start
	// merged Sephi
	//    if (NULL != pCity && pCity->getOwnerINLINE() == GC.getGame().getActivePlayer())
	if (NULL != pCity)
	{
		CvWString szStart = gDLL->getText("TXT_KEY_ACTUAL_EFFECTS");
		setBuildingActualEffects(szBuffer, szStart, eBuilding, pCity);
	}
	// BUG - Building Actual Effects - end

	// DEBUG Sephi
	if (GC.getGameINLINE().isDebugMode())
	{
		if (NULL != pCity)
		{
			szTempBuffer.Format(L"\n AI BuildingValue: %d", pCity->AI_getBuildingValue(eBuilding));
			szBuffer.append(szTempBuffer);
		}
	}

	if (kBuilding.getLastTurnPossibleBuild() > 0)
	{
		int iMaxTurns = kBuilding.getLastTurnPossibleBuild();

		iMaxTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getBuildPercent();
		iMaxTurns /= 100;

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_LAST_TURN_POSSIBLE_BUILD", iMaxTurns));
	}

	/*************************************************************************************************/
	/**	ADDON (Buildings can be unlocked by Gameoptions) Sephi                     					**/
	/*************************************************************************************************/
	GameOptionTypes eGameOption = (GameOptionTypes)kBuilding.getRequiredGameOption();
	if (eGameOption != NO_GAMEOPTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES_GAMEOPTION"));
		szBuffer.append(GC.getGameOptionInfo(eGameOption).getDescription());
	}
	// New Yields
	YieldTypes ReducedCostBy = (YieldTypes)kBuilding.getReducedCostByYield();
	if (ReducedCostBy != NO_YIELD)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_REDUCED_COST_BY_YIELD"));
		szTempBuffer.Format(L"%c", GC.getYieldInfo(ReducedCostBy).getChar());
		szBuffer.append(szTempBuffer);
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

		// test for unique building
	BuildingClassTypes eBuildingClass = (BuildingClassTypes)kBuilding.getBuildingClassType();
	BuildingTypes eDefaultBuilding = (BuildingTypes)GC.getBuildingClassInfo(eBuildingClass).getDefaultBuildingIndex();

	if (NO_BUILDING != eDefaultBuilding && eDefaultBuilding != eBuilding)
	{
		for (iI = 0; iI < GC.getNumCivilizationInfos(); ++iI)
		{
			BuildingTypes eUniqueBuilding = (BuildingTypes)GC.getCivilizationInfo((CivilizationTypes)iI).getCivilizationBuildings((int)eBuildingClass);
			if (eUniqueBuilding == eBuilding)
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szBuffer.append(gDLL->getText("TXT_KEY_UNIQUE_BUILDING", GC.getCivilizationInfo((CivilizationTypes)iI).getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_UNIQUE_BUILDING", getLinkedText((CivilizationTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify

			}
		}

		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/09/13
		//		szBuffer.append(gDLL->getText("TXT_KEY_REPLACES_UNIT", GC.getBuildingInfo(eDefaultBuilding).getTextKeyWide()));
		szBuffer.append(gDLL->getText("TXT_KEY_REPLACES_BUILDING", getLinkedText(eDefaultBuilding).c_str()));
		//<<<<	BUGFfH: End Modify

	}

	if (eDefaultBuilding == eBuilding && kBuilding.getPrereqCiv() != NO_CIVILIZATION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIQUE_BUILDING", getLinkedText((CivilizationTypes)kBuilding.getPrereqCiv()).c_str()));
	}

	if (bCivilopediaText)
	{
		setYieldChangeHelp(szBuffer, L"", L"", L"", kBuilding.getYieldModifierArray(), true, bCivilopediaText);

		setCommerceChangeHelp(szBuffer, L"", L"", L"", kBuilding.getCommerceModifierArray(), true, bCivilopediaText);
	}
	else
	{
		if (isWorldWonderClass((BuildingClassTypes)(kBuilding.getBuildingClassType())))
		{
			if (pCity == NULL || ePlayer == NO_PLAYER)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_WORLD_WONDER_ALLOWED", GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getBuildingClassType()).getMaxGlobalInstances()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_WORLD_WONDER_LEFT", (GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getBuildingClassType()).getMaxGlobalInstances() - GC.getGameINLINE().getBuildingClassCreatedCount((BuildingClassTypes)(kBuilding.getBuildingClassType())) - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getBuildingClassMaking((BuildingClassTypes)(kBuilding.getBuildingClassType())))));
			}
		}

		if (isTeamWonderClass((BuildingClassTypes)(kBuilding.getBuildingClassType())))
		{
			if (pCity == NULL || ePlayer == NO_PLAYER)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TEAM_WONDER_ALLOWED", GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getBuildingClassType()).getMaxTeamInstances()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TEAM_WONDER_LEFT", (GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getBuildingClassType()).getMaxTeamInstances() - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getBuildingClassCountPlusMaking((BuildingClassTypes)(kBuilding.getBuildingClassType())))));
			}
		}

		if (isNationalWonderClass((BuildingClassTypes)(kBuilding.getBuildingClassType())))
		{
			if (pCity == NULL || ePlayer == NO_PLAYER)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NATIONAL_WONDER_ALLOWED", GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getBuildingClassType()).getMaxPlayerInstances()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NATIONAL_WONDER_LEFT", (GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getBuildingClassType()).getMaxPlayerInstances() - GET_PLAYER(ePlayer).getBuildingClassCountPlusMaking((BuildingClassTypes)(kBuilding.getBuildingClassType())))));
			}
		}
	}

	if (kBuilding.getGlobalReligionCommerce() != NO_RELIGION)
	{
		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_PER_CITY_WITH", GC.getReligionInfo((ReligionTypes)kBuilding.getGlobalReligionCommerce()).getChar());
		setCommerceChangeHelp(szBuffer, L"", L"", szFirstBuffer, GC.getReligionInfo((ReligionTypes)kBuilding.getGlobalReligionCommerce()).getGlobalReligionCommerceArray());
	}

	if (NO_CORPORATION != kBuilding.getFoundsCorporation())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_FOUNDS_CORPORATION", GC.getCorporationInfo((CorporationTypes)kBuilding.getFoundsCorporation()).getTextKeyWide()));
	}

	if (kBuilding.getGlobalCorporationCommerce() != NO_CORPORATION)
	{
		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_PER_CITY_WITH", GC.getCorporationInfo((CorporationTypes)kBuilding.getGlobalCorporationCommerce()).getChar());
		setCommerceChangeHelp(szBuffer, L"", L"", szFirstBuffer, GC.getCorporationInfo((CorporationTypes)kBuilding.getGlobalCorporationCommerce()).getHeadquarterCommerceArray());
	}

	if (kBuilding.getNoBonus() != NO_BONUS)
	{
		CvBonusInfo& kBonus = GC.getBonusInfo((BonusTypes)kBuilding.getNoBonus());
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_DISABLES", kBonus.getTextKeyWide(), kBonus.getChar()));
	}

	if (kBuilding.getFreeBonus() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES", GC.getGameINLINE().getNumFreeBonuses(eBuilding), GC.getBonusInfo((BonusTypes) kBuilding.getFreeBonus()).getTextKeyWide(), GC.getBonusInfo((BonusTypes) kBuilding.getFreeBonus()).getChar()));
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES", GC.getGameINLINE().getNumFreeBonuses(eBuilding), getLinkedText((BonusTypes)kBuilding.getFreeBonus()).c_str()));
		//<<<<	BUGFfH: End Modify


		if (GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus())).getHealth() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)kBuilding.getFreeBonus()).getHealth()), ((GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus())).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer.append(szTempBuffer);
		}

		if (GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus())).getHappiness() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)kBuilding.getFreeBonus()).getHappiness()), ((GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus())).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer.append(szTempBuffer);
		}
	}

	//FfH: Added by Kael 08/04/2007
	if (kBuilding.getFreeBonus2() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES", GC.getGameINLINE().getNumFreeBonuses(eBuilding), GC.getBonusInfo((BonusTypes) kBuilding.getFreeBonus2()).getTextKeyWide(), GC.getBonusInfo((BonusTypes) kBuilding.getFreeBonus2()).getChar()));
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES", GC.getGameINLINE().getNumFreeBonuses(eBuilding), getLinkedText((BonusTypes)kBuilding.getFreeBonus2()).c_str()));
		//<<<<	BUGFfH: End Modify

		if (GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus2())).getHealth() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)kBuilding.getFreeBonus2()).getHealth()), ((GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus2())).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer.append(szTempBuffer);
		}
		if (GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus2())).getHappiness() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)kBuilding.getFreeBonus2()).getHappiness()), ((GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus2())).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer.append(szTempBuffer);
		}
	}
	if (kBuilding.getFreeBonus3() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES", GC.getGameINLINE().getNumFreeBonuses(eBuilding), GC.getBonusInfo((BonusTypes) kBuilding.getFreeBonus3()).getTextKeyWide(), GC.getBonusInfo((BonusTypes) kBuilding.getFreeBonus3()).getChar()));
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES", GC.getGameINLINE().getNumFreeBonuses(eBuilding), getLinkedText((BonusTypes)kBuilding.getFreeBonus3()).c_str()));
		//<<<<	BUGFfH: End Modify

		if (GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus3())).getHealth() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)kBuilding.getFreeBonus3()).getHealth()), ((GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus3())).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer.append(szTempBuffer);
		}
		if (GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus3())).getHappiness() != 0)
		{
			szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)kBuilding.getFreeBonus3()).getHappiness()), ((GC.getBonusInfo((BonusTypes)(kBuilding.getFreeBonus3())).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer.append(szTempBuffer);
		}
	}
	/*************************************************************************************************/
	/**	ADDON (New Building tags) Sephi                                                             **/
	/*************************************************************************************************/
	if (kBuilding.getPrereqCivic() != NO_CIVIC)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_PREREQ_CIVIC", getLinkedText((CivicTypes)kBuilding.getPrereqCivic()).c_str()));
	}

	if (kBuilding.getMistDensity() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_MIST_DENSITY_HELP", kBuilding.getMistDensity()));
	}

	if (kBuilding.getManaFromImprovement() != 0 && kBuilding.getManaFromImprovementType() != NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		CvWString szMana;
		szMana.Format(L"%.2f", 0.01f * kBuilding.getManaFromImprovement());
		szBuffer.append(gDLL->getText("TXT_KEY_TRAIT_MANA_FROM_IMPROVEMENT_HELP", getLinkedText((ImprovementTypes)kBuilding.getManaFromImprovementType()).c_str(), szMana.GetCString()));
	}

	if (kBuilding.getTaxesModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TAXES_MODIFIER_HELP", kBuilding.getTaxesModifier()));
	}

	if (kBuilding.getBuildingMaintenance() != 0)
	{
		szBuffer.append(NEWLINE);
		CvWString szFloatString;
		szFloatString.Format(L"%.2f", 0.01f * kBuilding.getBuildingMaintenance());
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_MAINTENANCE_HELP", szFloatString.GetCString()));

	}

	if (kBuilding.getGPPThresholdReductionPercent() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GPP_THRESHOLD_HELP", kBuilding.getGPPThresholdReductionPercent()));
	}

	if (kBuilding.getMaxManaChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_MAX_MANA_HELP", kBuilding.getMaxManaChange()));
	}

	if (kBuilding.getUnitSupportChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_UNITSUPPORT", kBuilding.getUnitSupportChange()));
	}
	if (kBuilding.getCommercePercentTypeForced() != NO_COMMERCE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_COMMERCEPERCENT_FORCED", GC.getCommerceInfo((CommerceTypes)kBuilding.getCommercePercentTypeForced()).getDescription()));
	}
	if (kBuilding.getPopulationCost() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_POPULATION_COST_HELP", kBuilding.getPopulationCost()));
	}

	if (kBuilding.getGlobalYieldTypeCost() != NO_YIELD && kBuilding.getGlobalYieldCost() > 0)
	{
		int iYieldCost = kBuilding.getGlobalYieldCost();
		if (pCity != NULL) {
			iYieldCost = pCity->getGlobalYieldCost(eBuilding);
		}
		szBuffer.append(NEWLINE);
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER && bCivilopediaText)
		{
			iYieldCost = (iYieldCost * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		}
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GLOBAL_YIELD_COST", iYieldCost, GC.getYieldInfo((YieldTypes)kBuilding.getGlobalYieldTypeCost()).getChar()));
	}

	if (kBuilding.getPopulationLimitChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_POPULATION_LIMIT_CHANGE_HELP", kBuilding.getPopulationLimitChange()));
	}

	if (kBuilding.isAdjacentPeak())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_ADJACENT_PEAK"));
	}

	if (kBuilding.isAdjacentUF())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_ADJACENT_UF"));
	}

	bFirst = false;
	for (int iJ = 0; iJ < GC.getNumBuildingClassInfos(); iJ++)
	{
		if (kBuilding.isBuildingClassBlocked(iJ))
		{
			for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
			{
				if (GC.getBuildingInfo((BuildingTypes)iI).getBuildingClassType() == iJ)
				{
					if (!bFirst)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_BLOCKS_BUILDINGCLASS", getLinkedText((BuildingTypes)iI).c_str()));
						bFirst = true;
					}
					else
					{
						szBuffer.append(L", ");
						szBuffer.append(getLinkedText((BuildingTypes)iI));
					}
				}
			}
		}
	}

	if (kBuilding.getPrereqBuildingClass() != NO_BUILDINGCLASS)
	{
		if (GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getPrereqBuildingClass()).getDefaultBuildingIndex() != NO_BUILDING) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_PREREQ_BUILDINGCLASS", getLinkedText((BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getPrereqBuildingClass()).getDefaultBuildingIndex()).c_str()));
		}
	}

	if (kBuilding.getPrereqBuildingClassInAllCities() != NO_BUILDINGCLASS)
	{
		if (GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getPrereqBuildingClassInAllCities()).getDefaultBuildingIndex() != NO_BUILDING) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_PREREQ_BUILDINGCLASS_ALL_CITIES", getLinkedText((BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getPrereqBuildingClassInAllCities()).getDefaultBuildingIndex()).c_str()));
		}
	}

	if (kBuilding.getPrereqImprovement() != NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_PREREQ_IMPROVEMENT_NEARBY", getLinkedText((ImprovementTypes)kBuilding.getPrereqImprovement()).c_str()));
	}

	bFirst = false;
	for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
	{
		if (GC.getBuildingInfo((BuildingTypes)iI).getPrereqBuildingClass() == kBuilding.getBuildingClassType())
		{
			CivilizationTypes eActiveCiv = NO_CIVILIZATION;
			if (GC.getGame().getActivePlayer() != NO_PLAYER)
				eActiveCiv = GET_PLAYER(GC.getGame().getActivePlayer()).getCivilizationType();
			if (eActiveCiv == NO_CIVILIZATION || GC.getBuildingInfo((BuildingTypes)iI).getPrereqCiv() == NO_CIVILIZATION
				|| eActiveCiv == GC.getBuildingInfo((BuildingTypes)iI).getPrereqCiv())
			{
				if (eActiveCiv == NO_CIVILIZATION ||
					GC.getCivilizationInfo(eActiveCiv).getCivilizationBuildings(GC.getBuildingInfo((BuildingTypes)iI).getBuildingClassType()) == iI)
				{
					if (!bFirst)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_ALLOWS_BUILDINGCLASS", getLinkedText((BuildingTypes)iI).c_str()));
						bFirst = true;
					}
					else
					{
						szBuffer.append(L", ");
						szBuffer.append(getLinkedText((BuildingTypes)iI));
					}
				}
			}
		}
	}

	for (iI = 0; iI < GC.getNUM_COMMERCE_TYPES(); iI++)
	{
		if (kBuilding.getCommercePerCultureLevel(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szBuffer.append(CvWString::format(L"%d", kBuilding.getCommercePerCultureLevel(iI)));
			szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes)iI).getChar()));
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_CIVIC_COMMERCE_PER_CULTURE"));
		}
	}

	for (iI = 0; iI < GC.getNUM_COMMERCE_TYPES(); iI++)
	{
		if (kBuilding.getCommercePerPopulation(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szBuffer.append(CvWString::format(L"%d", kBuilding.getCommercePerPopulation(iI)));
			szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes)iI).getChar()));
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_COMMERCE_PER_POPULATION"));
		}
	}

	for (iI = 0; iI < GC.getNUM_YIELD_TYPES(); iI++)
	{
		if (kBuilding.getYieldPerCultureLevel(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szBuffer.append(CvWString::format(L"%d", kBuilding.getYieldPerCultureLevel(iI)));
			szBuffer.append(CvWString::format(L"%c", GC.getYieldInfo((YieldTypes)iI).getChar()));
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_CIVIC_COMMERCE_PER_CULTURE"));
		}
	}

	if (kBuilding.getBonusCommerceType() != NO_COMMERCE)
	{
		if (kBuilding.getInitialValue() != 0)
		{
			CvWString szFloatString;
			szFloatString.Format(L"%.2f", 0.01f * kBuilding.getInitialValue());

			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes)kBuilding.getBonusCommerceType()).getChar()));
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_BONUSCOMMERCE_INITIAL", szFloatString.GetCString()));

			bool bFirst = true;
			for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (kBuilding.isBonusLinked(iI))
				{
					if (bFirst)
					{
						bFirst = false;
					}
					else
					{
						szBuffer.append(CvWString::format(L","));
					}
					szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)iI).getChar()));
				}
			}
		}

		if (kBuilding.getPerBonusValue() != 0)
		{
			CvWString szFloatString;
			szFloatString.Format(L"%.2f", 0.01f * kBuilding.getPerBonusValue());

			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes)kBuilding.getBonusCommerceType()).getChar()));
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_BONUSCOMMERCE_PERBONUS", szFloatString.GetCString()));
			bool bFirst = true;
			for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (kBuilding.isBonusLinked(iI))
				{
					if (bFirst)
					{
						bFirst = false;
					}
					else
					{
						szBuffer.append(CvWString::format(L","));
					}
					szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)iI).getChar()));
				}
			}
		}
	}

	if (kBuilding.getBonusYieldType() != NO_YIELD)
	{
		if (kBuilding.getInitialValue() != 0)
		{
			CvWString szFloatString;
			szFloatString.Format(L"%.2f", 0.01f * kBuilding.getInitialValue());

			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szBuffer.append(CvWString::format(L"%c", GC.getYieldInfo((YieldTypes)kBuilding.getBonusYieldType()).getChar()));

			szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_BONUSCOMMERCE_INITIAL", szFloatString.GetCString()));
			bFirst = true;
			for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (kBuilding.isBonusLinked(iI))
				{
					if (bFirst)
					{
						bFirst = false;
					}
					else
					{
						szBuffer.append(CvWString::format(L","));
					}
					szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)iI).getChar()));
				}
			}
		}

		if (kBuilding.getPerBonusValue() != 0)
		{
			CvWString szFloatString;
			szFloatString.Format(L"%.2f", 0.01f * kBuilding.getPerBonusValue());

			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			szBuffer.append(CvWString::format(L"%c", GC.getYieldInfo((YieldTypes)kBuilding.getBonusYieldType()).getChar()));
			szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_BONUSCOMMERCE_PERBONUS", szFloatString.GetCString()));
			bFirst = true;
			for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
			{
				if (kBuilding.isBonusLinked(iI))
				{
					if (bFirst)
					{
						bFirst = false;
					}
					else
					{
						szBuffer.append(CvWString::format(L","));
					}
					szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)iI).getChar()));
				}
			}
		}
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (kBuilding.isBonusConsumed(iI))
		{
			if (bFirst)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_BONUS_CONSUMED"));
				bFirst = false;
			}

			szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)iI).getChar()));
		}
	}

	bFirst = true;
	for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		if (kBuilding.isBonusNearbyNeeded(iI))
		{
			if (bFirst)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_HELP_BUILDING_BONUS_NEEDED_NEARBY_CITY"));
				bFirst = false;
			}

			szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)iI).getChar()));
		}
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	/*************************************************************************************************/
	/**	Change Building Bonuses  Snarko                                   					        **/
	/**																								**/
	/**						                                            							**/
	/*************************************************************************************************/
	if (NULL != pCity)
	{
		int iNumBonus;
		for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
		{
			iNumBonus = pCity->getBuildingBonusChange((BuildingClassTypes)kBuilding.getBuildingClassType(), (BonusTypes)iI);
			if (iNumBonus != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES", iNumBonus, getLinkedText((BonusTypes)iI).c_str()));

				if (GC.getBonusInfo((BonusTypes)iI).getHealth() != 0)
				{
					szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)iI).getHealth()), ((GC.getBonusInfo((BonusTypes)iI).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
					szBuffer.append(szTempBuffer);
				}
				if (GC.getBonusInfo((BonusTypes)iI).getHappiness() != 0)
				{
					szTempBuffer.Format(L", +%d%c", abs(GC.getBonusInfo((BonusTypes)iI).getHappiness()), ((GC.getBonusInfo((BonusTypes)iI).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
					szBuffer.append(szTempBuffer);
				}
			}
		}
	}
	/*************************************************************************************************/
	/**	END Change Building Bonuses	                                        						**/
	/*************************************************************************************************/
	/*************************************************************************************************/
	/**	ADDON (New Items for Buildinghelp) Sephi                                					**/
	/*************************************************************************************************/
	if (kBuilding.isRiver())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_RIVERSIDE_HELP"));
	}

	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/*************************************************************************************************/
	if (kBuilding.getCorporationSupportNeeded() != NO_CORPORATION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PEDIA_CORPORATIONSUPPORT_NEEDED", GC.getCorporationInfo((CorporationTypes)kBuilding.getCorporationSupportNeeded()).getDescription()));
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (kBuilding.getCrime() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_CRIME", kBuilding.getCrime()));
	}
	if (kBuilding.getFreePromotionPick() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_PROMOTION_PICK", kBuilding.getFreePromotionPick()));
	}
	if (kBuilding.getModifyGlobalCounter() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MESSAGE_MODIFY_GLOBAL_COUNTER", kBuilding.getModifyGlobalCounter()));
	}
	if (kBuilding.getRemovePromotion() > NO_PROMOTION)
	{
		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/09/14
		//		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REMOVE_PROMOTION", GC.getPromotionInfo((PromotionTypes)kBuilding.getRemovePromotion()).getDescription()));
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REMOVE_PROMOTION", getLinkedText((PromotionTypes)kBuilding.getRemovePromotion()).c_str()));
		//<<<<	BUGFfH: End Modify

	}
	if (kBuilding.getResistMagic() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_RESIST_MAGIC", kBuilding.getResistMagic()));
	}
	if (kBuilding.getGlobalResistEnemyModify() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GLOBAL_RESIST_ENEMY_MODIFY", kBuilding.getGlobalResistEnemyModify()));
	}
	if (kBuilding.getGlobalResistModify() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GLOBAL_RESIST_MODIFY", kBuilding.getGlobalResistModify()));
	}
	if (kBuilding.isHideUnits())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HIDE_UNITS"));
	}
	if (kBuilding.isUnhappyProduction())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_UNHAPPY_PRODUCTION_NEW"));
	}
	if (kBuilding.isSeeInvisible())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_SEE_INVISIBLE"));
	}
	for (iI = 0; iI < GC.getNumSpecialistInfos(); ++iI)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/09/14
		//		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_FROM_IN_ALL_CITIES", GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide());
		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_FROM_IN_ALL_CITIES", getLinkedText((SpecialistTypes)iI).c_str());
		//<<<<	BUGFfH: End Modify
		setCommerceChangeHelp(szBuffer, L"", L"", szFirstBuffer, kBuilding.getSpecialistCommerceChangeArray(iI));
		//		setCommerceChangeHelp(szHelpText, L"", L"", gDLL->getText("TXT_KEY_CIVIC_IN_ALL_CITIES").GetCString(), GC.getCivicInfo(eCivic).getCommerceModifierArray(), true);

		//>>>>	BUGFfH: Moved from below(*20) by Denev 2009/10/05
		szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_FROM_IN_ALL_CITIES", getLinkedText((SpecialistTypes)iI).c_str());
		setYieldChangeHelp(szBuffer, L"", L"", szFirstBuffer, kBuilding.getSpecialistYieldChangeArray(iI));
		//<<<<	BUGFfH: End Move
	}
	if (kBuilding.isNoCivicAnger())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NO_CIVIC_ANGER"));
	}
	if (kBuilding.getPlotRadius() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PLOT_RADIUS", kBuilding.getPlotRadius()));
	}
	//FfH: End Add

	if (kBuilding.getFreeBuildingClass() != NO_BUILDINGCLASS)
	{
		BuildingTypes eFreeBuilding;
		if (ePlayer != NO_PLAYER)
		{
			eFreeBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(kBuilding.getFreeBuildingClass())));
		}
		else
		{
			eFreeBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)kBuilding.getFreeBuildingClass()).getDefaultBuildingIndex();
		}

		if (NO_BUILDING != eFreeBuilding)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_IN_CITY", GC.getBuildingInfo(eFreeBuilding).getTextKeyWide()));
		}
	}
	//modified Sephi
//	if (kBuilding.getFreePromotion() != NO_PROMOTION)

	if (kBuilding.getFreePromotion() != NO_PROMOTION &&
		(GC.getPromotionInfo((PromotionTypes)kBuilding.getFreePromotion()).getRequiredGameOption() == NO_GAMEOPTION
			|| GC.getGameINLINE().isOption((GameOptionTypes)GC.getPromotionInfo((PromotionTypes)kBuilding.getFreePromotion()).getRequiredGameOption())))
	{
		szBuffer.append(NEWLINE);
		//>>>>	BUGFfH: Modified by Denev 2009/10/03
		/*
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_PROMOTION", GC.getPromotionInfo((PromotionTypes)(kBuilding.getFreePromotion())).getTextKeyWide()));

		//FfH: Added by Kael 08/04/2007
				if (kBuilding.isApplyFreePromotionOnMove())
				{
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_APPLY_FREE_PROMOTION_ON_MOVE"));
				}
		//FfH: End Add
		*/
		if (kBuilding.isApplyFreePromotionOnMove())
		{
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_PROMOTION_STAYED", getLinkedText((PromotionTypes)kBuilding.getFreePromotion()).c_str()));
		}
		else
		{
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_PROMOTION", getLinkedText((PromotionTypes)kBuilding.getFreePromotion()).c_str()));
		}
		//<<<<	BUGFfH: End Modify

	}

	if (kBuilding.getCivicOption() != NO_CIVICOPTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_ENABLES_CIVICS", GC.getCivicOptionInfo((CivicOptionTypes)(kBuilding.getCivicOption())).getTextKeyWide()));
	}

	if (kBuilding.isPower())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES_POWER"));

		if (kBuilding.isDirtyPower() && (GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") != 0))
		{
			szTempBuffer.Format(L" (+%d%c)", abs(GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE")), ((GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer.append(szTempBuffer);
		}
	}

	if (kBuilding.isAreaCleanPower())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES_AREA_CLEAN_POWER"));
	}

	if (kBuilding.isAreaBorderObstacle())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_BORDER_OBSTACLE"));
	}

	for (iI = 0; iI < GC.getNumVoteSourceInfos(); ++iI)
	{
		if (kBuilding.getVoteSourceType() == (VoteSourceTypes)iI)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_DIPLO_VOTE", GC.getVoteSourceInfo((VoteSourceTypes)iI).getTextKeyWide()));
		}
	}

	if (kBuilding.isForceTeamVoteEligible())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_ELECTION_ELIGIBILITY"));
	}

	if (kBuilding.isCapital())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_CAPITAL"));
	}

	if (kBuilding.isGovernmentCenter())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REDUCES_MAINTENANCE"));
	}

	if (kBuilding.isGoldenAge())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GOLDEN_AGE"));
	}

	if (kBuilding.isAllowsNukes())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_ENABLES_NUKES"));
	}

	if (kBuilding.isMapCentering())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_CENTERS_MAP"));
	}

	if (kBuilding.isNoUnhappiness())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NO_UNHAPPY"));
	}

	if (kBuilding.isNoUnhealthyPopulation())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NO_UNHEALTHY_POP"));
	}

	if (kBuilding.isBuildingOnlyHealthy())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NO_UNHEALTHY_BUILDINGS"));
	}

	if (kBuilding.getGreatPeopleRateModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_BIRTH_RATE_MOD", kBuilding.getGreatPeopleRateModifier()));
	}

	if (kBuilding.getGreatGeneralRateModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GENERAL_RATE_MOD", kBuilding.getGreatGeneralRateModifier()));
	}

	if (kBuilding.getDomesticGreatGeneralRateModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_DOMESTIC_GREAT_GENERAL_MODIFIER", kBuilding.getDomesticGreatGeneralRateModifier()));
	}

	if (kBuilding.getGlobalGreatPeopleRateModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_BIRTH_RATE_MOD_ALL_CITIES", kBuilding.getGlobalGreatPeopleRateModifier()));
	}

	if (kBuilding.getAnarchyModifier() != 0)
	{
		if (-100 == kBuilding.getAnarchyModifier())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NO_ANARCHY"));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_ANARCHY_MOD", kBuilding.getAnarchyModifier()));

			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_ANARCHY_TIMER_MOD", kBuilding.getAnarchyModifier()));
		}
	}

	if (kBuilding.getGoldenAgeModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GOLDENAGE_MOD", kBuilding.getGoldenAgeModifier()));
	}

	if (kBuilding.getGlobalHurryModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HURRY_MOD", kBuilding.getGlobalHurryModifier()));
	}

	if (kBuilding.getFreeExperience() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_XP_UNITS", kBuilding.getFreeExperience()));
	}

	if (kBuilding.getGlobalFreeExperience() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_XP_ALL_CITIES", kBuilding.getGlobalFreeExperience()));
	}

	if (kBuilding.getFoodKept() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_STORES_FOOD", kBuilding.getFoodKept()));
	}

	if (kBuilding.getAirlift() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_AIRLIFT", kBuilding.getAirlift()));
	}

	if (kBuilding.getAirModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_AIR_DAMAGE_MOD", kBuilding.getAirModifier()));
	}

	if (kBuilding.getAirUnitCapacity() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_AIR_UNIT_CAPACITY", kBuilding.getAirUnitCapacity()));
	}

	if (kBuilding.getNukeModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NUKE_DAMAGE_MOD", kBuilding.getNukeModifier()));
	}

	if (kBuilding.getNukeExplosionRand() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NUKE_EXPLOSION_CHANCE"));
	}

	if (kBuilding.getFreeSpecialist() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALISTS", kBuilding.getFreeSpecialist()));
	}

	if (kBuilding.getAreaFreeSpecialist() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALISTS_CONT", kBuilding.getAreaFreeSpecialist()));
	}

	if (kBuilding.getGlobalFreeSpecialist() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALISTS_ALL_CITIES", kBuilding.getGlobalFreeSpecialist()));
	}

	if (kBuilding.getMaintenanceModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_MAINT_MOD", kBuilding.getMaintenanceModifier()));
	}

	if (kBuilding.getHurryAngerModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HURRY_ANGER_MOD", kBuilding.getHurryAngerModifier()));
	}

	if (kBuilding.getWarWearinessModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_WAR_WEAR_MOD", kBuilding.getWarWearinessModifier()));
	}

	if (kBuilding.getGlobalWarWearinessModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_WAR_WEAR_MOD_ALL_CITIES", kBuilding.getGlobalWarWearinessModifier()));
	}

	if (kBuilding.getEnemyWarWearinessModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_ENEMY_WAR_WEAR", kBuilding.getEnemyWarWearinessModifier()));
	}

	if (kBuilding.getHealRateChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HEAL_MOD", kBuilding.getHealRateChange()));
	}

	if (kBuilding.getAreaHealth() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HEALTH_CHANGE_CONT", abs(kBuilding.getAreaHealth()), ((kBuilding.getAreaHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR))));
	}

	if (kBuilding.getGlobalHealth() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HEALTH_CHANGE_ALL_CITIES", abs(kBuilding.getGlobalHealth()), ((kBuilding.getGlobalHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR))));
	}

	if (kBuilding.getAreaHappiness() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HAPPY_CHANGE_CONT", kBuilding.getAreaHappiness(), ((kBuilding.getAreaHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
	}

	if (kBuilding.getGlobalHappiness() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HAPPY_CHANGE_ALL_CITIES", kBuilding.getGlobalHappiness(), ((kBuilding.getGlobalHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
	}

	if (kBuilding.getStateReligionHappiness() > 0)
	{
		if (kBuilding.getReligionType() != NO_RELIGION)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_RELIGION_HAPPINESS", kBuilding.getStateReligionHappiness(), ((kBuilding.getStateReligionHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), GC.getReligionInfo((ReligionTypes)(kBuilding.getReligionType())).getChar()));
		}
	}

	if (kBuilding.getWorkerSpeedModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_WORKER_MOD", kBuilding.getWorkerSpeedModifier()));
	}

	if (kBuilding.getMilitaryProductionModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_MILITARY_MOD", kBuilding.getMilitaryProductionModifier()));
	}

	if (kBuilding.getSpaceProductionModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_SPACESHIP_MOD", kBuilding.getSpaceProductionModifier()));
	}

	if (kBuilding.getGlobalSpaceProductionModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_SPACESHIP_MOD_ALL_CITIES", kBuilding.getGlobalSpaceProductionModifier()));
	}

	if (kBuilding.getTradeRoutes() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TRADE_ROUTES", kBuilding.getTradeRoutes()));
	}

	if (kBuilding.getCoastalTradeRoutes() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_COASTAL_TRADE_ROUTES", kBuilding.getCoastalTradeRoutes()));
	}

	if (kBuilding.getGlobalTradeRoutes() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TRADE_ROUTES_ALL_CITIES", kBuilding.getGlobalTradeRoutes()));
	}

	if (kBuilding.getTradeRouteModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TRADE_ROUTE_MOD", kBuilding.getTradeRouteModifier()));
	}

	if (kBuilding.getForeignTradeRouteModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FOREIGN_TRADE_ROUTE_MOD", kBuilding.getForeignTradeRouteModifier()));
	}

	if (kBuilding.getGlobalPopulationChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_GLOBAL_POP", kBuilding.getGlobalPopulationChange()));
	}

	if (kBuilding.getFreeTechs() != 0)
	{
		if (kBuilding.getFreeTechs() == 1)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_TECH"));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_TECHS", kBuilding.getFreeTechs()));
		}
	}

	if (kBuilding.getDefenseModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_DEFENSE_MOD", kBuilding.getDefenseModifier()));
	}

	if (kBuilding.getBombardDefenseModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_BOMBARD_DEFENSE_MOD", -kBuilding.getBombardDefenseModifier()));
	}

	if (kBuilding.getAllCityDefenseModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_DEFENSE_MOD_ALL_CITIES", kBuilding.getAllCityDefenseModifier()));
	}

	if (kBuilding.getEspionageDefenseModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_ESPIONAGE_DEFENSE_MOD", kBuilding.getEspionageDefenseModifier()));

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_EXPOSE_SPIES"));
	}

	setYieldChangeHelp(szBuffer, gDLL->getText("TXT_KEY_BUILDING_WATER_PLOTS").c_str(), L": ", L"", kBuilding.getSeaPlotYieldChangeArray());

	setYieldChangeHelp(szBuffer, gDLL->getText("TXT_KEY_BUILDING_RIVER_PLOTS").c_str(), L": ", L"", kBuilding.getRiverPlotYieldChangeArray());

	setYieldChangeHelp(szBuffer, gDLL->getText("TXT_KEY_BUILDING_WATER_PLOTS_ALL_CITIES").c_str(), L": ", L"", kBuilding.getGlobalSeaPlotYieldChangeArray());

	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_WITH_POWER").c_str(), kBuilding.getPowerYieldModifierArray(), true);

	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_ALL_CITIES_THIS_CONTINENT").c_str(), kBuilding.getAreaYieldModifierArray(), true);

	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_ALL_CITIES").c_str(), kBuilding.getGlobalYieldModifierArray(), true);

	setCommerceChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_ALL_CITIES").c_str(), kBuilding.getGlobalCommerceModifierArray(), true);

	setCommerceChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_BUILDING_PER_SPECIALIST_ALL_CITIES").c_str(), kBuilding.getSpecialistExtraCommerceArray());

	if (ePlayer != NO_PLAYER && GET_PLAYER(ePlayer).getStateReligion() != NO_RELIGION)
	{
		szTempBuffer = gDLL->getText("TXT_KEY_BUILDING_FROM_ALL_REL_BUILDINGS", GC.getReligionInfo(GET_PLAYER(ePlayer).getStateReligion()).getChar());
	}
	else
	{
		szTempBuffer = gDLL->getText("TXT_KEY_BUILDING_STATE_REL_BUILDINGS");
	}
	setCommerceChangeHelp(szBuffer, L"", L"", szTempBuffer, kBuilding.getStateReligionCommerceArray());

	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		if (kBuilding.getCommerceHappiness(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PER_LEVEL", ((kBuilding.getCommerceHappiness(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), (100 / kBuilding.getCommerceHappiness(iI)), GC.getCommerceInfo((CommerceTypes)iI).getChar()));
		}

		if (kBuilding.isCommerceFlexible(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_ADJUST_COMM_RATE", GC.getCommerceInfo((CommerceTypes)iI).getChar()));
		}
	}

	//>>>>	BUGFfH: Moved to above(*20) by Denev 2009/10/05
	/*
		for (iI = 0; iI < GC.getNumSpecialistInfos(); ++iI)
		{
			szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_FROM_IN_ALL_CITIES", GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide());
			setYieldChangeHelp(szBuffer, L"", L"", szFirstBuffer, kBuilding.getSpecialistYieldChangeArray(iI));
		}
	*/
	//<<<<	BUGFfH: End Move

	//>>>>	BUGFfH: Modified by Denev 2009/09/24
	/*
		for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
		{
			szFirstBuffer = gDLL->getText("TXT_KEY_BUILDING_WITH_BONUS", GC.getBonusInfo((BonusTypes) iI).getTextKeyWide());
			setYieldChangeHelp(szBuffer, L"", L"", szFirstBuffer, kBuilding.getBonusYieldModifierArray(iI), true);
		}
	*/
	int aiYieldChange[NUM_YIELD_TYPES] = { 0 };
	int* aiYieldChangeLast = aiYieldChange;
	bool bCompare;
	for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
	{
		CvWStringBuffer szTempBuffer2;
		setYieldChangeHelp(szTempBuffer2, L"", L"", gDLL->getText("TXT_KEY_MISC_WITH"), kBuilding.getBonusYieldModifierArray(iBonus), true);
		if (!szTempBuffer2.isEmpty())
		{
			szTempBuffer.assign(szTempBuffer2.getCString());
			bCompare = memcmp(kBuilding.getBonusYieldModifierArray(iBonus), aiYieldChangeLast, sizeof(int) * NUM_YIELD_TYPES);
			setListHelp(szBuffer, szTempBuffer, getLinkedText((BonusTypes)iBonus), L", ", bCompare);
			aiYieldChangeLast = kBuilding.getBonusYieldModifierArray(iBonus);
		}
	}
	//<<<<	BUGFfH: End Modify
	for (iI = 0; iI < GC.getNumReligionInfos(); ++iI)
	{
		if (kBuilding.getReligionChange(iI) > 0)
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_SPREADS_RELIGION", GC.getReligionInfo((ReligionTypes)iI).getChar()).c_str());
			szBuffer.append(szTempBuffer);
		}
	}

	for (iI = 0; iI < GC.getNumSpecialistInfos(); ++iI)
	{
		if (kBuilding.getSpecialistCount(iI) > 0)
		{
			if (kBuilding.getSpecialistCount(iI) == 1)
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/02
				//				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TURN_CITIZEN_INTO", GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TURN_CITIZEN_INTO", getLinkedText((SpecialistTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify

			}
			else
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/02
				//				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TURN_CITIZENS_INTO", kBuilding.getSpecialistCount(iI), GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TURN_CITIZENS_INTO", kBuilding.getSpecialistCount(iI), getLinkedText((SpecialistTypes)iI).c_str()));
				//<<<<	BUGFfH: End Modify

			}
		}

		if (kBuilding.getFreeSpecialistCount(iI) > 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/10/02
			//			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALIST", kBuilding.getFreeSpecialistCount(iI), GC.getSpecialistInfo((SpecialistTypes) iI).getTextKeyWide()));
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_SPECIALIST", kBuilding.getFreeSpecialistCount(iI), getLinkedText((SpecialistTypes)iI).c_str()));
			//<<<<	BUGFfH: End Modify

		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumImprovementInfos(); ++iI)
	{
		if (kBuilding.getImprovementFreeSpecialist(iI) > 0)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_IMPROVEMENT_FREE_SPECIALISTS", kBuilding.getImprovementFreeSpecialist(iI)).GetCString());
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer.Format(L"<link=literal>%s</link>", GC.getImprovementInfo((ImprovementTypes)iI).getDescription());
			szTempBuffer = getLinkedText((ImprovementTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (kBuilding.getImprovementFreeSpecialist(iI) != iLast));
			iLast = kBuilding.getImprovementFreeSpecialist(iI);
		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		if (kBuilding.getBonusHealthChanges(iI) != 0)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_HEALTH_HAPPINESS_CHANGE", abs(kBuilding.getBonusHealthChanges(iI)), ((kBuilding.getBonusHealthChanges(iI) > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR))).c_str());
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer.Format(L"<link=literal>%s</link>", GC.getBonusInfo((BonusTypes)iI).getDescription());
			szTempBuffer = getLinkedText((BonusTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (kBuilding.getBonusHealthChanges(iI) != iLast));
			iLast = kBuilding.getBonusHealthChanges(iI);
		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumCivicInfos(); ++iI)
	{
		int iChange = GC.getCivicInfo((CivicTypes)iI).getBuildingHealthChanges(kBuilding.getBuildingClassType());
		if (0 != iChange)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_CIVIC_HEALTH_HAPPINESS_CHANGE", abs(iChange), ((iChange > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR))).c_str());
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer.Format(L"<link=literal>%s</link>", GC.getCivicInfo((CivicTypes)iI).getDescription());
			szTempBuffer = getLinkedText((CivicTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (iChange != iLast));
			iLast = iChange;
		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		if (kBuilding.getBonusHappinessChanges(iI) != 0)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_HEALTH_HAPPINESS_CHANGE", abs(kBuilding.getBonusHappinessChanges(iI)), ((kBuilding.getBonusHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))).c_str());
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer.Format(L"<link=literal>%s</link>", GC.getBonusInfo((BonusTypes)iI).getDescription());
			szTempBuffer = getLinkedText((BonusTypes)iI);
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (kBuilding.getBonusHappinessChanges(iI) != iLast));
			iLast = kBuilding.getBonusHappinessChanges(iI);
		}
	}

	iLast = 0;

	for (iI = 0; iI < GC.getNumCivicInfos(); ++iI)
	{
		int iChange = GC.getCivicInfo((CivicTypes)iI).getBuildingHappinessChanges(kBuilding.getBuildingClassType());
		if (0 != iChange)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_CIVIC_HEALTH_HAPPINESS_CHANGE", abs(iChange), ((iChange > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))).c_str());
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer.Format(L"<link=literal>%s</link>", GC.getCivicInfo((CivicTypes)iI).getDescription());
			szTempBuffer = getLinkedText((CivicTypes)iI);
			//<<<<	BUGFfH: End Modify

			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", (iChange != iLast));
			iLast = iChange;
		}
	}

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); ++iI)
	{
		if (kBuilding.getUnitCombatFreeExperience(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/09/15
			//			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_XP", GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide(), kBuilding.getUnitCombatFreeExperience(iI)));
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_XP",
				getLinkedText((UnitCombatTypes)iI).c_str(),
				kBuilding.getUnitCombatFreeExperience(iI)));
			//<<<<	BUGFfH: End Modify

		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; ++iI)
	{
		if (kBuilding.getDomainFreeExperience(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/10/03
			//			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_XP", GC.getDomainInfo((DomainTypes)iI).getTextKeyWide(), kBuilding.getDomainFreeExperience(iI)));
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_XP",
				getLinkedText((DomainTypes)iI).c_str(),
				kBuilding.getDomainFreeExperience(iI)));
			//<<<<	BUGFfH: End Modify

		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; ++iI)
	{
		if (kBuilding.getDomainProductionModifier(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/09/15
			//			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_BUILDS_FASTER_DOMAIN", GC.getDomainInfo((DomainTypes)iI).getTextKeyWide(), kBuilding.getDomainProductionModifier(iI)));
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_BUILDS_FASTER_DOMAIN",
				getLinkedText((DomainTypes)iI).c_str(),
				kBuilding.getDomainProductionModifier(iI)));
			//<<<<	BUGFfH: End Modify

		}
	}

	bFirst = true;

	//>>>>	BUGFfH: Modified by Denev 2009/09/26
	/*
	//FfH: Modified by Kael 11/02/2006
	//	for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
	//	{
	//		if (GC.getUnitInfo((UnitTypes)iI).getPrereqBuilding() == eBuilding)
	//		{
	//			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRED_TO_TRAIN").c_str());
	//			szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo((UnitTypes)iI).getDescription());
	//			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
	//			bFirst = false;
	//		}
	//	}
		UnitTypes eLoopUnit;
		for (iI = 0; iI < GC.getNumUnitClassInfos(); iI++)
		{
			if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
			{
				eLoopUnit = (UnitTypes)GC.getCivilizationInfo(GC.getGameINLINE().getActiveCivilizationType()).getCivilizationUnits(iI);
			}
			else
			{
				eLoopUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)iI).getDefaultUnitIndex();
			}
			if (eLoopUnit != NO_UNIT)
			{
				if (GC.getUnitInfo(eLoopUnit).getPrereqBuilding() == eBuilding || GC.getUnitInfo(eLoopUnit).getPrereqBuildingClass() == eBuildingClass)
				{
					szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRED_TO_TRAIN").c_str());
					szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo(eLoopUnit).getDescription());
					setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
					bFirst = false;
				}
			}
		}
	//FfH: End Modify
	*/
	//get list of all available civilizations which can use the building.
	std::list<CivilizationTypes> aeAvailableCiv;
	CivilizationTypes eActiveCiv = GC.getGameINLINE().getActiveCivilizationType();
	if (pCity != NULL && pCity->getCivilizationType() != NO_CIVILIZATION)
	{
		//Assimilated city requires original civilization's building.
		eActiveCiv = pCity->getCivilizationType();
	}
	getAvailableCivs(aeAvailableCiv, eBuilding, eActiveCiv);

	for (int iUnit = 0; iUnit < GC.getNumUnitInfos(); iUnit++)
	{
		//When unit cost is negative (can not be built), skip the unit.
		if (GC.getUnitInfo((UnitTypes)iUnit).getProductionCost() < 0)
		{
			continue;
		}

		//added Sephi
		if (pCity != NULL && pCity->getOwnerINLINE() != NO_PLAYER)
		{
			if (GC.getUnitInfo((UnitTypes)iUnit).getPrereqCiv() != NO_CIVILIZATION &&
				GC.getUnitInfo((UnitTypes)iUnit).getPrereqCiv() != GET_PLAYER(pCity->getOwnerINLINE()).getCivilizationType())
			{
				continue;
			}
		}

		//Building requirement and BuildingClass requirement are "OR".
		if (GC.getUnitInfo((UnitTypes)iUnit).getPrereqBuilding() == eBuilding
			|| GC.getUnitInfo((UnitTypes)iUnit).getPrereqBuildingClass() == eBuildingClass)
		{
			//When unit or building is limited by civilization, they need matching types of available civilization each other.
			//For example, Arena can train only Balseraph units.
			bool bMatchCivilizationType = false;
			const int iUnitClass = GC.getUnitInfo((UnitTypes)iUnit).getUnitClassType();
			for (std::list<CivilizationTypes>::iterator it = aeAvailableCiv.begin(); it != aeAvailableCiv.end(); it++)
			{
				if (iUnit == GC.getCivilizationInfo(*it).getCivilizationUnits(iUnitClass)
					&& eBuilding == (BuildingTypes)GC.getCivilizationInfo(*it).getCivilizationBuildings((int)eBuildingClass))
				{
					bMatchCivilizationType = true;
					break;
				}
			}
			if (bMatchCivilizationType)
			{
				szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRED_TO_TRAIN").c_str());
				//>>>>	BUGFfH: Modified by Denev 2009/10/02
				//				szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), getLinkedText((UnitTypes)iUnit).c_str());
				szTempBuffer.assign(getLinkedText((UnitTypes)iUnit));
				//<<<<	BUGFfH: End Modify
				setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
				bFirst = false;
			}
		}
	}
	//<<<<	BUGFfH: End Modify

	bFirst = true;

	//>>>>	BUGFfH: Modified by Denev 2009/09/24
	/*
		for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
		{
			if (GC.getUnitInfo((UnitTypes)iI).getBuildings(eBuilding) || GC.getUnitInfo((UnitTypes)iI).getForceBuildings(eBuilding))
			{
				szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_UNIT_REQUIRED_TO_BUILD").c_str());
				szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), GC.getUnitInfo((UnitTypes)iI).getDescription());
				setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
				bFirst = false;
			}
		}
	*/
	szFirstBuffer.assign(NEWLINE);
	szFirstBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRED_TO_BUILD"));
	for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
	{
		if (GC.getUnitInfo((UnitTypes)iI).getBuildings(eBuilding))
		{
			//>>>>	BUGFfH: Modified by Denev 2009/10/02
			//			szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), getLinkedText((UnitTypes)iI).c_str());
			szTempBuffer.assign(getLinkedText((UnitTypes)iI));
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
	{
		if (GC.getUnitInfo((UnitTypes)iI).getForceBuildings(eBuilding))
		{
			//>>>>	BUGFfH: Modified by Denev 2009/10/02
			//			szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_UNIT_TEXT"), getLinkedText((UnitTypes)iI).c_str());
			szTempBuffer.assign(getLinkedText((UnitTypes)iI));
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}
	if (!bFirst)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_IGNORE_REQUIREMENT"));
	}
	//<<<<	BUGFfH: End Modify

	iLast = 0;

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		if (kBuilding.getBuildingHappinessChanges(iI) != 0)
		{
			szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_HAPPINESS_CHANGE", kBuilding.getBuildingHappinessChanges(iI),
				((kBuilding.getBuildingHappinessChanges(iI) > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))).c_str());
			CvWString szBuilding;
			if (NO_PLAYER != ePlayer)
			{
				BuildingTypes ePlayerBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI)));
				if (NO_BUILDING != ePlayerBuilding)
				{
					//>>>>	BUGFfH: Modified by Denev 2009/09/13
					//					szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingClassInfo((BuildingClassTypes)iI).getDescription());
					szBuilding = getLinkedText((BuildingClassTypes)iI);
					//<<<<	BUGFfH: End Modify

				}
			}
			else
			{
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szBuilding.Format(L"<link=literal>%s</link>", GC.getBuildingClassInfo((BuildingClassTypes)iI).getDescription());
				szBuilding = getLinkedText((BuildingClassTypes)iI);
				//<<<<	BUGFfH: End Modify

			}
			setListHelp(szBuffer, szTempBuffer, szBuilding, L", ", (kBuilding.getBuildingHappinessChanges(iI) != iLast));
			iLast = kBuilding.getBuildingHappinessChanges(iI);
		}
	}

	if (kBuilding.getPowerBonus() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_PROVIDES_POWER_WITH", GC.getBonusInfo((BonusTypes)kBuilding.getPowerBonus()).getTextKeyWide()));

		if (kBuilding.isDirtyPower() && (GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") != 0))
		{
			szTempBuffer.Format(L" (+%d%c)", abs(GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE")), ((GC.getDefineINT("DIRTY_POWER_HEALTH_CHANGE") > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)));
			szBuffer.append(szTempBuffer);
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
	{
		if (ePlayer != NO_PLAYER)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI)));
		}
		else
		{
			eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
		}

		if (eLoopBuilding != NO_BUILDING)
		{
			if (GC.getBuildingInfo(eLoopBuilding).isBuildingClassNeededInCity(kBuilding.getBuildingClassType()))
			{
				if ((pCity == NULL) || pCity->canConstruct(eLoopBuilding, false, true))
				{
					szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRED_TO_BUILD").c_str());
					//>>>>	BUGFfH: Modified by Denev 2009/10/02
					//					szTempBuffer.Format(SETCOLR L"<link=literal>%s</link>" ENDCOLR, TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eLoopBuilding).getDescription());
					szTempBuffer.assign(getLinkedText(eLoopBuilding));
					//<<<<	BUGFfH: End Modify
					setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
					bFirst = false;
				}
			}
		}
	}

	if (bCivilopediaText)
	{
		// Trait
		for (int i = 0; i < GC.getNumTraitInfos(); ++i)
		{
			if (kBuilding.getProductionTraits((TraitTypes)i) != 0)
			{
				if (kBuilding.getProductionTraits((TraitTypes)i) == 100)
				{
					szBuffer.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/09/15
					//					szBuffer.append(gDLL->getText("TXT_KEY_DOUBLE_SPEED_TRAIT", GC.getTraitInfo((TraitTypes)i).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_DOUBLE_SPEED_TRAIT", getLinkedText((TraitTypes)i).c_str()));
					//<<<<	BUGFfH: End Modify

				}
				else
				{
					szBuffer.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/09/15
					//					szBuffer.append(gDLL->getText("TXT_KEY_PRODUCTION_MODIFIER_TRAIT", kBuilding.getProductionTraits((TraitTypes)i), GC.getTraitInfo((TraitTypes)i).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_PRODUCTION_MODIFIER_TRAIT",
						kBuilding.getProductionTraits((TraitTypes)i),
						getLinkedText((TraitTypes)i).c_str()));
					//<<<<	BUGFfH: End Modify

				}
			}
		}

		for (int i = 0; i < GC.getNumTraitInfos(); ++i)
		{
			if (kBuilding.getHappinessTraits((TraitTypes)i) != 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_HAPPINESS_TRAIT", kBuilding.getHappinessTraits((TraitTypes)i), GC.getTraitInfo((TraitTypes)i).getTextKeyWide()));
			}
		}
	}

	if (bCivilopediaText)
	{
		if (kBuilding.getGreatPeopleUnitClass() != NO_UNITCLASS)
		{
			if (ePlayer != NO_PLAYER)
			{
				eGreatPeopleUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationUnits(kBuilding.getGreatPeopleUnitClass())));
			}
			else
			{
				eGreatPeopleUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)kBuilding.getGreatPeopleUnitClass()).getDefaultUnitIndex();
			}

			if (eGreatPeopleUnit != NO_UNIT)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_LIKELY_TO_GENERATE", GC.getUnitInfo(eGreatPeopleUnit).getTextKeyWide()));
			}
		}

		if (kBuilding.getFreeStartEra() != NO_ERA)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_FREE_START_ERA", GC.getEraInfo((EraTypes)kBuilding.getFreeStartEra()).getTextKeyWide()));
		}
	}

	/*************************************************************************************************/
	/**	New Gameoption (Passive XP Training) Sephi                                				    **/
	/*************************************************************************************************/

	szTempBuffer.clear();
	bool bFound = false;
	iLast = 0;
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++) {
		if (kBuilding.getTrainXPCap(iI) != 0) {
			if (kBuilding.getTrainXPCap(iI) != iLast) {

				if (!bFirst) {
					szTempBuffer += gDLL->getText("TXT_KEY_UNTIL");
					szTempBuffer += CvWString::format(L" %d ", iLast);
					szTempBuffer += gDLL->getText("TXT_KEY_XP");
					szTempBuffer += L", ";
				}
				szTempBuffer += CvWString::format(L" %s ", gDLL->getText(GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide()).c_str());
			}
			else {
				szTempBuffer += gDLL->getText("TXT_KEY_AND");
				szTempBuffer += CvWString::format(L" %s ", gDLL->getText(GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide()).c_str());
			}
			bFound = true;
			iLast = kBuilding.getTrainXPCap(iI);
			bFirst = false;
		}
	}

	if (bFound) {
		szTempBuffer += gDLL->getText("TXT_KEY_UNTIL");
		szTempBuffer += CvWString::format(L" %d ", iLast);
		szTempBuffer += gDLL->getText("TXT_KEY_XP");
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TRAIN_CAP"));
		szBuffer.append(szTempBuffer);
	}

	szTempBuffer.clear();
	bFound = false;
	iLast = 0;
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++) {
		if (kBuilding.getGlobalTrainXPCap(iI) != 0) {
			if (kBuilding.getGlobalTrainXPCap(iI) != iLast) {

				if (!bFirst) {
					szTempBuffer += gDLL->getText("TXT_KEY_UNTIL");
					szTempBuffer += CvWString::format(L" %d ", iLast);
					szTempBuffer += gDLL->getText("TXT_KEY_XP");
					szTempBuffer += L", ";
				}
				szTempBuffer += CvWString::format(L" %s ", getLinkedText((UnitCombatTypes)iI).c_str());
			}
			else {
				szTempBuffer += gDLL->getText("TXT_KEY_AND");
				szTempBuffer += CvWString::format(L" %s ", getLinkedText((UnitCombatTypes)iI).c_str());
			}
			bFound = true;
			iLast = kBuilding.getGlobalTrainXPCap(iI);
			bFirst = false;
		}
	}

	if (bFound) {
		szTempBuffer += gDLL->getText("TXT_KEY_UNTIL");
		szTempBuffer += CvWString::format(L" %d ", iLast);
		szTempBuffer += gDLL->getText("TXT_KEY_XP");
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TRAIN_CAP_GLOBAL"));
		szBuffer.append(szTempBuffer);
	}

	szTempBuffer.clear();
	bFound = false;
	iLast = 0;
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (kBuilding.getTrainXPRate(iI) != 0)
		{
			if (kBuilding.getTrainXPRate(iI) != iLast)
			{
				if (!bFirst)
				{
					szTempBuffer += L", ";
				}
				szTempBuffer += CvWString::format(L" %d%% (%s) ", kBuilding.getTrainXPRate(iI) / 100, getLinkedText((UnitCombatTypes)iI).c_str());
				iLast = kBuilding.getTrainXPRate(iI);
			}
			else
			{
				szTempBuffer += gDLL->getText("TXT_KEY_AND");
				szTempBuffer += CvWString::format(L" (%s) ", getLinkedText((UnitCombatTypes)iI).c_str());
			}
			bFound = true;
			iLast = kBuilding.getTrainXPRate(iI);
			bFirst = false;
		}
	}
	if (bFound)
	{
		szTempBuffer += gDLL->getText("TXT_KEY_XP");
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TRAIN_RATE"));
		szBuffer.append(szTempBuffer);
	}
	szTempBuffer.clear();
	bFound = false;
	iLast = 0;
	bFirst = true;
	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		if (kBuilding.getGlobalTrainXPRate(iI) != 0)
		{
			if (kBuilding.getGlobalTrainXPRate(iI) != iLast)
			{
				if (!bFirst)
				{
					szTempBuffer += L", ";
				}
				szTempBuffer += CvWString::format(L" %d%% (%s) ", kBuilding.getGlobalTrainXPRate(iI) / 100, gDLL->getText(GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide()).c_str());
				iLast = kBuilding.getGlobalTrainXPRate(iI);
			}
			else
			{
				szTempBuffer += gDLL->getText("TXT_KEY_AND");
				szTempBuffer += CvWString::format(L" (%s) ", gDLL->getText(GC.getUnitCombatInfo((UnitCombatTypes)iI).getTextKeyWide()).c_str());
			}
			bFound = true;
			iLast = kBuilding.getGlobalTrainXPRate(iI);
			bFirst = false;
		}
	}
	if (bFound)
	{
		szTempBuffer += gDLL->getText("TXT_KEY_XP");
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TRAIN_RATE_GLOBAL"));
		szBuffer.append(szTempBuffer);
	}

	for (int iImprovement = 0; iImprovement < GC.getNumImprovementInfos(); iImprovement++) {
		int aiYieldAmount[NUM_YIELD_TYPES];

		bool bYields = false;
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++) {
			aiYieldAmount[iYield] = GC.getBuildingInfo(eBuilding).getImprovementYieldChanges((ImprovementTypes)iImprovement, iYield);
			bYields = bYields || aiYieldAmount[iYield] != 0;
		}

		if (bYields) {
			setYieldChangeHelp(szBuffer, L"", L"",
				gDLL->getText("TXT_KEY_CIVIC_YIELD_FROM_IMPROVEMENT", getLinkedText((ImprovementTypes)iImprovement).c_str()),
				aiYieldAmount);
		}
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/
	/*************************************************************************************************/
	/**	ADDON (Houses of Erebus) Sephi			                                 					**/
	/*************************************************************************************************/
	if (bCivilopediaText)
	{
		for (int i = 0; i < GC.getNumCorporationInfos(); i++)
		{
			if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isHasCorporationSupport((CorporationTypes)i))
			{
				if (kBuilding.getCorporationSupport((CorporationTypes)i) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT", GC.getCorporationInfo((CorporationTypes)i).getDescription(), kBuilding.getCorporationSupport((CorporationTypes)i)));
				}

				if (kBuilding.getCorporationSupportMultiplier((CorporationTypes)i) != 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_SUPPORT_MODIFIER", GC.getCorporationInfo((CorporationTypes)i).getDescription(), kBuilding.getCorporationSupportMultiplier((CorporationTypes)i)));
				}
			}
		}
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/


	if (!CvWString(kBuilding.getHelp()).empty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(kBuilding.getHelp());
	}

	buildBuildingRequiresString(szBuffer, eBuilding, bCivilopediaText, bTechChooserText, pCity);

	if (pCity != NULL)
	{
		if (!(GC.getBuildingClassInfo((BuildingClassTypes)(kBuilding.getBuildingClassType())).isNoLimit()))
		{
			if (isWorldWonderClass((BuildingClassTypes)(kBuilding.getBuildingClassType())))
			{
				if (pCity->isWorldWondersMaxed())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_WORLD_WONDERS_PER_CITY", GC.getDefineINT("MAX_WORLD_WONDERS_PER_CITY")));
				}
			}
			else if (isTeamWonderClass((BuildingClassTypes)(kBuilding.getBuildingClassType())))
			{
				if (pCity->isTeamWondersMaxed())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_TEAM_WONDERS_PER_CITY", GC.getDefineINT("MAX_TEAM_WONDERS_PER_CITY")));
				}
			}
			else if (isNationalWonderClass((BuildingClassTypes)(kBuilding.getBuildingClassType())))
			{
				if (pCity->isNationalWondersMaxed())
				{
					int iMaxNumWonders = (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && GET_PLAYER(pCity->getOwnerINLINE()).isHuman()) ? GC.getDefineINT("MAX_NATIONAL_WONDERS_PER_CITY_FOR_OCC") : GC.getDefineINT("MAX_NATIONAL_WONDERS_PER_CITY");
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NATIONAL_WONDERS_PER_CITY", iMaxNumWonders));
				}
			}
			else
			{
				if (pCity->isBuildingsMaxed())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NUM_PER_CITY", GC.getDefineINT("MAX_BUILDINGS_PER_CITY")));
				}
			}
		}
	}

	if ((pCity == NULL) || pCity->getNumRealBuilding(eBuilding) < GC.getCITY_MAX_NUM_BUILDINGS())
	{
		if (!bCivilopediaText)
		{
			if (pCity == NULL)
			{
				if (kBuilding.getProductionCost() > 0)
				{
					szTempBuffer.Format(L"\n%d%c", (ePlayer != NO_PLAYER ? GET_PLAYER(ePlayer).getProductionNeeded(eBuilding) : kBuilding.getProductionCost()), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer.append(szTempBuffer);
				}
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_NUM_TURNS", pCity->getProductionTurnsLeft(eBuilding, ((gDLL->ctrlKey() || !(gDLL->shiftKey())) ? 0 : pCity->getOrderQueueLength()))));

				iProduction = pCity->getBuildingProduction(eBuilding);

				int iProductionNeeded = pCity->getProductionNeeded(eBuilding);
				if (iProduction > 0)
				{
					szTempBuffer.Format(L" - %d/%d%c", iProduction, iProductionNeeded, GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer.append(szTempBuffer);
				}
				else
				{
					szTempBuffer.Format(L" - %d%c", iProductionNeeded, GC.getYieldInfo(YIELD_PRODUCTION).getChar());
					szBuffer.append(szTempBuffer);
				}
			}
		}

		for (int iI = 0; iI < GC.getNumBonusInfos(); ++iI)
		{
			if (kBuilding.getBonusProductionModifier(iI) != 0)
			{
				if (pCity != NULL)
				{
					if (pCity->hasBonus((BonusTypes)iI))
					{
						szBuffer.append(gDLL->getText("TXT_KEY_COLOR_POSITIVE"));
					}
					else
					{
						szBuffer.append(gDLL->getText("TXT_KEY_COLOR_NEGATIVE"));
					}
				}
				if (!bCivilopediaText)
				{
					szBuffer.append(L" (");
				}
				else
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR), szTempBuffer);
					szBuffer.append(szTempBuffer);
				}
				if (kBuilding.getBonusProductionModifier(iI) == 100)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_DOUBLE_SPEED_WITH", GC.getBonusInfo((BonusTypes)iI).getTextKeyWide()));
				}
				else
				{
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_BUILDS_FASTER_WITH", kBuilding.getBonusProductionModifier(iI), GC.getBonusInfo((BonusTypes)iI).getTextKeyWide()));
				}
				if (!bCivilopediaText)
				{
					szBuffer.append(L')');
				}
				if (pCity != NULL)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_COLOR_REVERT"));
				}
			}
		}

		if (kBuilding.getObsoleteTech() != NO_TECH)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_OBSOLETE_WITH", GC.getTechInfo((TechTypes)kBuilding.getObsoleteTech()).getTextKeyWide()));

			if (kBuilding.getDefenseModifier() != 0 || kBuilding.getBombardDefenseModifier() != 0)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_OBSOLETE_EXCEPT"));
			}
		}

		if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
		{
			if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)kBuilding.getSpecialBuildingType()).getObsoleteTech() != NO_TECH)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_OBSOLETE_WITH", GC.getTechInfo((TechTypes)GC.getSpecialBuildingInfo((SpecialBuildingTypes)kBuilding.getSpecialBuildingType()).getObsoleteTech()).getTextKeyWide()));
			}
		}

		if ((gDLL->getChtLvl() > 0) && gDLL->ctrlKey() && (pCity != NULL))
		{
			int iBuildingValue = pCity->AI_buildingValue(eBuilding);
			szBuffer.append(CvWString::format(L"\nAI Building Value = %d", iBuildingValue));
		}
	}

	if (bStrategyText)
	{
		if (!CvWString(kBuilding.getStrategy()).empty())
		{
			if ((ePlayer == NO_PLAYER) || GET_PLAYER(ePlayer).isOption(PLAYEROPTION_ADVISOR_HELP))
			{
				szBuffer.append(SEPARATOR);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_SIDS_TIPS"));
				szBuffer.append(L'\"');
				szBuffer.append(kBuilding.getStrategy());
				szBuffer.append(L'\"');
			}
		}
	}

	if (bCivilopediaText)
	{
		if (eDefaultBuilding == eBuilding)
		{
			for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
			{
				if (iI != eBuilding)
				{
					if (eBuildingClass == GC.getBuildingInfo((BuildingTypes)iI).getBuildingClassType())
					{
						szBuffer.append(NEWLINE);
						//>>>>	BUGFfH: Modified by Denev 2009/09/16
						//						szBuffer.append(gDLL->getText("TXT_KEY_REPLACED_BY_BUILDING", GC.getBuildingInfo((BuildingTypes)iI).getTextKeyWide()));
						szBuffer.append(gDLL->getText("TXT_KEY_REPLACED_BY_BUILDING", getLinkedText((BuildingTypes)iI).c_str()));
						//<<<<	BUGFfH: End Modify

					}
				}
			}
		}
	}

}

void CvGameTextMgr::buildBuildingRequiresString(CvWStringBuffer& szBuffer, BuildingTypes eBuilding, bool bCivilopediaText, bool bTechChooserText, const CvCity* pCity)
{
	bool bFirst;
	PlayerTypes ePlayer;
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);
	BuildingTypes eLoopBuilding;

	if (pCity != NULL)
	{
		ePlayer = pCity->getOwnerINLINE();
	}
	else
	{
		ePlayer = GC.getGameINLINE().getActivePlayer();
	}

	if (NULL == pCity || !pCity->canConstruct(eBuilding))
	{
		if (kBuilding.getHolyCity() != NO_RELIGION)
		{
			if (pCity == NULL || !pCity->isHolyCity((ReligionTypes)(kBuilding.getHolyCity())))
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/09/15
				//				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ONLY_HOLY_CONSTRUCT", GC.getReligionInfo((ReligionTypes) kBuilding.getHolyCity()).getChar()));
				szBuffer.append(gDLL->getText("TXT_KEY_ACTION_ONLY_HOLY_CONSTRUCT", GC.getReligionInfo((ReligionTypes)kBuilding.getHolyCity()).getHolyCityChar()));
				//<<<<	BUGFfH: End Modify
			}
		}

		bFirst = true;

		if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
		{
			if ((pCity == NULL) || !(GC.getGameINLINE().isSpecialBuildingValid((SpecialBuildingTypes)(kBuilding.getSpecialBuildingType()))))
			{
				for (int iI = 0; iI < GC.getNumProjectInfos(); ++iI)
				{
					if (GC.getProjectInfo((ProjectTypes)iI).getEveryoneSpecialBuilding() == kBuilding.getSpecialBuildingType())
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						CvWString szProject;
						//>>>>	BUGFfH: Modified by Denev 2009/10/02
						//						szProject.Format(L"<link=literal>%s</link>", GC.getProjectInfo((ProjectTypes)iI).getDescription());
						szProject = getLinkedText((ProjectTypes)iI, false);
						//<<<<	BUGFfH: End Modify
						setListHelp(szBuffer, szTempBuffer, szProject, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
						bFirst = false;
					}
				}
			}
		}

		if (!bFirst)
		{
			szBuffer.append(ENDCOLR);
		}

		if (kBuilding.getSpecialBuildingType() != NO_SPECIALBUILDING)
		{
			if ((pCity == NULL) || !(GC.getGameINLINE().isSpecialBuildingValid((SpecialBuildingTypes)(kBuilding.getSpecialBuildingType()))))
			{
				TechTypes eTech = (TechTypes)GC.getSpecialBuildingInfo((SpecialBuildingTypes)kBuilding.getSpecialBuildingType()).getTechPrereqAnyone();
				if (NO_TECH != eTech)
				{
					szBuffer.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/01
					//					szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES_TECH_ANYONE", GC.getTechInfo(eTech).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES_TECH_ANYONE", getLinkedText(eTech, false).c_str()));
					//<<<<	BUGFfH: End Modify
				}
			}
		}

		for (int iI = 0; iI < GC.getNumBuildingClassInfos(); ++iI)
		{
			if (ePlayer == NO_PLAYER && kBuilding.getPrereqNumOfBuildingClass((BuildingClassTypes)iI) > 0)
			{
				eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
				szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS_NO_CITY", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide(), kBuilding.getPrereqNumOfBuildingClass((BuildingClassTypes)iI)).c_str());

				szBuffer.append(szTempBuffer);
			}
			else if (ePlayer != NO_PLAYER && GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI)) > 0)
			{
				//				if ((pCity == NULL) || (GET_PLAYER(ePlayer).getBuildingClassCount((BuildingClassTypes)iI) < GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI))))
				//				{
				eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI)));

				if (eLoopBuilding != NO_BUILDING)
				{
					if (pCity != NULL)
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide(), GET_PLAYER(ePlayer).getBuildingClassCount((BuildingClassTypes)iI), GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI))).c_str());
					}
					else
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS_NO_CITY", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide(), GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI))).c_str());
					}

					szBuffer.append(szTempBuffer);
				}
				//				}
			}
			else if (kBuilding.isBuildingClassNeededInCity(iI))
			{
				if (NO_PLAYER != ePlayer)
				{
					eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI)));
				}
				else
				{
					eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
				}

				if (eLoopBuilding != NO_BUILDING)
				{
					if ((pCity == NULL) || (pCity->getNumBuilding(eLoopBuilding) <= 0))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide()));
					}
				}
			}
			/** Denev Code
			if (kBuilding.getPrereqNumOfBuildingClass((BuildingClassTypes)iI) > 0)
			{
				if (ePlayer == NO_PLAYER)
				{
					eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS_NO_CITY",
									getLinkedText(eLoopBuilding, false).c_str(),
									kBuilding.getPrereqNumOfBuildingClass((BuildingClassTypes)iI)));

					szBuffer.append(szTempBuffer);
				}
				else
				{
					const int iNumPreReqs = GET_PLAYER(ePlayer).getBuildingClassPrereqBuilding(eBuilding, ((BuildingClassTypes)iI));
					const int iNumTotal = GET_PLAYER(ePlayer).getBuildingClassCount((BuildingClassTypes)iI);
					if (iNumPreReqs > iNumTotal)
					{
						eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI);

						if (eLoopBuilding != NO_BUILDING)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_SPECIAL_BUILDINGS",
											getLinkedText(eLoopBuilding, false).c_str(),
											iNumTotal,
											iNumPreReqs));

							szBuffer.append(szTempBuffer);
						}
					}
				}
			}
			**/

			if (!bCivilopediaText)
			{
				if (kBuilding.isBuildingClassNeededInCity(iI))
				{
					bool bBuildingClassNeeded = true;

					if (pCity == NULL)
					{
						eLoopBuilding = (BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)iI).getDefaultBuildingIndex();
					}
					else
					{
						eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(GET_PLAYER(ePlayer).getCivilizationType()).getCivilizationBuildings(iI);
						//Assimilated city requires original civilization's building
						if (pCity->getCivilizationType() != NO_CIVILIZATION)
						{
							eLoopBuilding = (BuildingTypes)GC.getCivilizationInfo(pCity->getCivilizationType()).getCivilizationBuildings(iI);
						}

						for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); iBuilding++)
						{
							if (GC.getBuildingInfo((BuildingTypes)iBuilding).getBuildingClassType() == iI
								&& pCity->getNumBuilding((BuildingTypes)iBuilding) > 0)
							{
								bBuildingClassNeeded = false;
								break;
							}
						}
					}

					if (bBuildingClassNeeded)
					{
						if (eLoopBuilding != NO_BUILDING)	//added Sephi
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_MISC_REQ_CITY_HAS", GC.getBuildingInfo(eLoopBuilding).getTextKeyWide()));
						}
					}
				}
			}
			//<<<<	BUGFfH: End Modify
		}

		if (kBuilding.isStateReligion())
		{
			if (NULL == pCity || NO_PLAYER == ePlayer || NO_RELIGION == GET_PLAYER(ePlayer).getStateReligion() || !pCity->isHasReligion(GET_PLAYER(ePlayer).getStateReligion()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES_STATE_RELIGION"));
			}
		}

		if (kBuilding.getNumCitiesPrereq() > 0)
		{
			if (NO_PLAYER == ePlayer || GET_PLAYER(ePlayer).getNumCities() < kBuilding.getNumCitiesPrereq())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_CITIES", kBuilding.getNumCitiesPrereq()));
			}
		}

		if (kBuilding.getUnitLevelPrereq() > 0)
		{
			if (NO_PLAYER == ePlayer || GET_PLAYER(ePlayer).getHighestUnitLevel() < kBuilding.getUnitLevelPrereq())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_UNIT_LEVEL", kBuilding.getUnitLevelPrereq()));
			}
		}

		if (kBuilding.getMinLatitude() > 0)
		{
			if (NULL == pCity || pCity->plot()->getLatitude() < kBuilding.getMinLatitude())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MIN_LATITUDE", kBuilding.getMinLatitude()));
			}
		}

		if (kBuilding.getMaxLatitude() < 90)
		{
			if (NULL == pCity || pCity->plot()->getLatitude() > kBuilding.getMaxLatitude())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MAX_LATITUDE", kBuilding.getMaxLatitude()));
			}
		}

		if (pCity != NULL)
		{
			if (GC.getGameINLINE().isNoNukes())
			{
				if (kBuilding.isAllowsNukes())
				{
					for (int iI = 0; iI < GC.getNumUnitInfos(); ++iI)
					{
						if (GC.getUnitInfo((UnitTypes)iI).getNukeRange() != -1)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_NO_NUKES"));
							break;
						}
					}
				}
			}
		}

		if (bCivilopediaText)
		{
			if (kBuilding.getVictoryPrereq() != NO_VICTORY)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_VICTORY", GC.getVictoryInfo((VictoryTypes)(kBuilding.getVictoryPrereq())).getTextKeyWide()));
			}

			if (kBuilding.getMaxStartEra() != NO_ERA)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_MAX_START_ERA", GC.getEraInfo((EraTypes)kBuilding.getMaxStartEra()).getTextKeyWide()));
			}

			if (kBuilding.getNumTeamsPrereq() > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_NUM_TEAMS", kBuilding.getNumTeamsPrereq()));
			}
		}
		else
		{
			if (!bTechChooserText)
			{
				if (kBuilding.getPrereqAndTech() != NO_TECH)
				{
					if (ePlayer == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(kBuilding.getPrereqAndTech()))))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes)(kBuilding.getPrereqAndTech())).getTextKeyWide()));
					}
				}
			}

			bFirst = true;

			for (int iI = 0; iI < GC.getNUM_BUILDING_AND_TECH_PREREQS(); ++iI)
			{
				if (kBuilding.getPrereqAndTechs(iI) != NO_TECH)
				{
					if (bTechChooserText || ePlayer == NO_PLAYER || !(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isHasTech((TechTypes)(kBuilding.getPrereqAndTechs(iI)))))
					{
						szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
						setListHelp(szBuffer, szTempBuffer, GC.getTechInfo(((TechTypes)(kBuilding.getPrereqAndTechs(iI)))).getDescription(), gDLL->getText("TXT_KEY_AND").c_str(), bFirst);
						bFirst = false;
					}
				}
			}

			if (!bFirst)
			{
				szBuffer.append(ENDCOLR);
			}

			if (kBuilding.getPrereqAndBonus() != NO_BONUS)
			{
				if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)kBuilding.getPrereqAndBonus())))
				{
					szBuffer.append(NEWLINE);
					//>>>>	BUGFfH: Modified by Denev 2009/10/01
					//					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getBonusInfo((BonusTypes)kBuilding.getPrereqAndBonus()).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_REQUIRES"));
					szBuffer.append(ENDCOLR);
					szBuffer.append(GC.getBonusInfo((BonusTypes)kBuilding.getPrereqAndBonus()).getChar());
					//<<<<	BUGFfH: End Modify
				}
			}

			CvWStringBuffer szBonusList;
			bFirst = true;
			bool bNeedAnyBonus = true;

			for (int iI = 0; iI < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); ++iI)
			{
				if (kBuilding.getPrereqOrBonuses(iI) != NO_BONUS)
				{
					if ((pCity != NULL) && (pCity->hasBonus((BonusTypes)kBuilding.getPrereqOrBonuses(iI))))
					{
						bNeedAnyBonus = false;
					}
				}
			}

			if (bNeedAnyBonus) {
				for (int iI = 0; iI < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); ++iI)
				{
					if (kBuilding.getPrereqOrBonuses(iI) != NO_BONUS)
					{
						if ((pCity == NULL) || !(pCity->hasBonus((BonusTypes)kBuilding.getPrereqOrBonuses(iI))))
						{
							szTempBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());

							//FfH: Modified by Kael 07/28/2008
							//						setListHelp(szBonusList, szTempBuffer, GC.getBonusInfo((BonusTypes)kBuilding.getPrereqOrBonuses(iI)).getDescription(), gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
							//						bFirst = false;
							//					}
							//					else if (NULL != pCity)
							//					{
							//						bFirst = true;
							//						break;
							//					}
							//>>>>	BUGFfH: Modified by Denev 2009/10/01
							//						setListHelp(szBonusList, szTempBuffer, GC.getBonusInfo((BonusTypes)kBuilding.getPrereqOrBonuses(iI)).getDescription(), gDLL->getText("TXT_KEY_AND").c_str(), bFirst);
							CvWString szListItem;
							szListItem.Format(L"%c", GC.getBonusInfo((BonusTypes)kBuilding.getPrereqOrBonuses(iI)).getChar());
							setListHelp(szBonusList, szTempBuffer, szListItem, gDLL->getText("TXT_KEY_OR").c_str(), bFirst);
							//<<<<	BUGFfH: End Modify
							bFirst = false;
						}
						//FfH: End Modify

					}
				}
			}

			//FfH: Added by Kael 08/04/2007
			if (kBuilding.getPrereqTrait() != NO_TRAIT)
			{
				if (NO_PLAYER == ePlayer || !GET_PLAYER(ePlayer).hasTrait((TraitTypes)kBuilding.getPrereqTrait()))
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_UNIT_REQUIRES_STRING", GC.getTraitInfo((TraitTypes)kBuilding.getPrereqTrait()).getTextKeyWide()));
				}
			}
			//FfH: End Add

			if (!bFirst)
			{
				szBonusList.append(ENDCOLR);
				szBuffer.append(szBonusList);
			}

			if (NO_CORPORATION != kBuilding.getFoundsCorporation())
			{
				bFirst = true;
				szBonusList.clear();
				for (int iI = 0; iI < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++iI)
				{
					BonusTypes eBonus = (BonusTypes)GC.getCorporationInfo((CorporationTypes)kBuilding.getFoundsCorporation()).getPrereqBonus(iI);
					if (NO_BONUS != eBonus)
					{
						if ((pCity == NULL) || !(pCity->hasBonus(eBonus)))
						{
							szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_REQUIRES").c_str());
							//>>>>	BUGFfH: Modified by Denev 2009/10/02
							//							szTempBuffer.Format(L"<link=literal>%s</link>", GC.getBonusInfo(eBonus).getDescription());
							szTempBuffer = getLinkedText(eBonus, false);
							//<<<<	BUGFfH: End Modify
							setListHelp(szBonusList, szFirstBuffer, szTempBuffer, gDLL->getText("TXT_KEY_OR"), bFirst);
							bFirst = false;
						}
						else if (NULL != pCity)
						{
							bFirst = true;
							break;
						}
					}
				}

				if (!bFirst)
				{
					szBonusList.append(ENDCOLR);
					szBuffer.append(szBonusList);
				}
			}
			//>>>>	BUGFfH: Added by Denev 2009/09/25
			if (NO_RELIGION != kBuilding.getStateReligion()
				&& (NO_PLAYER == ePlayer || kBuilding.getStateReligion() != GET_PLAYER(ePlayer).getStateReligion()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_REQ_STATE_RELIGION", GC.getReligionInfo((ReligionTypes)kBuilding.getStateReligion()).getChar()));
			}
			else if (NO_RELIGION != kBuilding.getPrereqReligion()
				&& (NULL == pCity || !(pCity->isHasReligion((ReligionTypes)kBuilding.getPrereqReligion()))))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_REQ_CITY_HAS_RELIGION", GC.getReligionInfo((ReligionTypes)kBuilding.getPrereqReligion()).getChar()));
			}
			//<<<<	BUGFfH: End Add
		}
	}
}


void CvGameTextMgr::setProjectHelp(CvWStringBuffer& szBuffer, ProjectTypes eProject, bool bCivilopediaText, CvCity* pCity)
{
	PROFILE_FUNC();

	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	PlayerTypes ePlayer;
	bool bFirst;
	int iProduction;
	int iI;

	if (NO_PROJECT == eProject)
	{
		return;
	}

	CvProjectInfo& kProject = GC.getProjectInfo(eProject);

	if (pCity != NULL)
	{
		ePlayer = pCity->getOwnerINLINE();
	}
	else
	{
		ePlayer = GC.getGameINLINE().getActivePlayer();
	}

	if (!bCivilopediaText)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_PROJECT_TEXT"), kProject.getDescription());
		szTempBuffer.assign(getLinkedText(eProject));
		//<<<<	BUGFfH: End Modify
		szBuffer.append(szTempBuffer);

		if (isWorldProject(eProject))
		{
			if (pCity == NULL)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_WORLD_NUM_ALLOWED", kProject.getMaxGlobalInstances()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_WORLD_NUM_LEFT", (kProject.getMaxGlobalInstances() - GC.getGameINLINE().getProjectCreatedCount(eProject) - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectMaking(eProject))));
			}
		}

		if (isTeamProject(eProject))
		{
			if (pCity == NULL)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TEAM_NUM_ALLOWED", kProject.getMaxTeamInstances()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TEAM_NUM_LEFT", (kProject.getMaxTeamInstances() - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount(eProject) - GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectMaking(eProject))));
			}
		}
	}

	/*************************************************************************************************/
	/**	ADDON (New tag Definitions) Sephi                                          					**/
	/*************************************************************************************************/

	int iProjectManaCost = kProject.getManaCost();
	int iProjectManaUpkeep = kProject.getManaUpkeep();
	int iProjectFaithCost = kProject.getFaithCost();
	int iProjectFaithUpkeep = kProject.getFaithUpkeep();

	if (kProject.getSummonUnitType() != NO_UNIT)
	{
		iProjectManaUpkeep = GC.getUnitInfo((UnitTypes)kProject.getSummonUnitType()).getManaUpkeep();
		iProjectFaithUpkeep = GC.getUnitInfo((UnitTypes)kProject.getSummonUnitType()).getFaithUpkeep();
	}
	if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
	{
		iProjectManaCost = GET_PLAYER((PlayerTypes)GC.getGameINLINE().getActivePlayer()).getSpecificMagicRitualCost(eProject);
		iProjectFaithCost = GET_PLAYER((PlayerTypes)GC.getGameINLINE().getActivePlayer()).getSpecificFaithRitualCost(eProject);
	}

	if (iProjectManaCost != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_MANA_COST", iProjectManaCost));
	}

	if (iProjectFaithCost != 0 || iProjectFaithUpkeep != 0)
	{
		if (iProjectFaithUpkeep >= 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_FAITH_COST", iProjectFaithCost, iProjectFaithUpkeep));
		}

		if (iProjectFaithUpkeep < 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_FAITH_INCOME", iProjectFaithCost, -iProjectFaithUpkeep));
		}
	}

	if (kProject.getSummonUnitType() != NO_UNIT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_SUMMON_UNIT"));
		szBuffer.append(CvWString::format(L"<link=literal>%s</link>", GC.getUnitInfo((UnitTypes)kProject.getSummonUnitType()).getDescription()));
	}

	if (kProject.getLastTurnPossibleBuild() > 0)
	{
		int iMaxTurns = kProject.getLastTurnPossibleBuild();

		iMaxTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getBuildPercent();
		iMaxTurns /= 100;

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_LAST_TURN_POSSIBLE_BUILD", iMaxTurns));
	}

	if (kProject.getTerrainToType1() != NO_TERRAIN)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM", getLinkedText((TerrainTypes)kProject.getTerrainType1(), true).c_str(), getLinkedText((TerrainTypes)kProject.getTerrainToType1(), true).c_str()));
	}

	if (kProject.getTerrainToType2() != NO_TERRAIN)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM", getLinkedText((TerrainTypes)kProject.getTerrainType2(), true).c_str(), getLinkedText((TerrainTypes)kProject.getTerrainToType2(), true).c_str()));
	}

	if (kProject.getTerrainToType3() != NO_TERRAIN)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM", getLinkedText((TerrainTypes)kProject.getTerrainType3(), true).c_str(), getLinkedText((TerrainTypes)kProject.getTerrainToType3(), true).c_str()));
	}

	if (kProject.getTerrainToType4() != NO_TERRAIN)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM", getLinkedText((TerrainTypes)kProject.getTerrainType4(), true).c_str(), getLinkedText((TerrainTypes)kProject.getTerrainToType4(), true).c_str()));
	}

	if (kProject.getTerrainToType5() != NO_TERRAIN)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM", getLinkedText((TerrainTypes)kProject.getTerrainType5(), true).c_str(), getLinkedText((TerrainTypes)kProject.getTerrainToType5(), true).c_str()));
	}

	if (kProject.getFeatureTypePre() != NO_FEATURE && kProject.getFeatureTypePost() == NO_FEATURE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_REMOVE_FEATURE", getLinkedText((FeatureTypes)kProject.getFeatureTypePre(), true).c_str()));
	}

	if (kProject.getFeatureTypePre() == NO_FEATURE && kProject.getFeatureTypePost() != NO_FEATURE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_FEATURE", getLinkedText((FeatureTypes)kProject.getFeatureTypePost(), true).c_str()));
	}

	if (kProject.getFeatureTypePost() != NO_FEATURE && kProject.getFeatureTypePre() != NO_FEATURE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_TRANSFORM_FEATURE", getLinkedText((FeatureTypes)kProject.getFeatureTypePre(), true).c_str(), getLinkedText((FeatureTypes)kProject.getFeatureTypePost(), true).c_str()));
	}

	if (kProject.getFeatureType2Pre() != NO_FEATURE && kProject.getFeatureType2Post() == NO_FEATURE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_REMOVE_FEATURE", getLinkedText((FeatureTypes)kProject.getFeatureType2Pre(), true).c_str()));
	}

	if (kProject.getFeatureType2Pre() == NO_FEATURE && kProject.getFeatureType2Post() != NO_FEATURE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_FEATURE", getLinkedText((FeatureTypes)kProject.getFeatureType2Post(), true).c_str()));
	}

	if (kProject.getFeatureType2Post() != NO_FEATURE && kProject.getFeatureType2Pre() != NO_FEATURE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_TRANSFORM_FEATURE", getLinkedText((FeatureTypes)kProject.getFeatureType2Pre(), true).c_str(), getLinkedText((FeatureTypes)kProject.getFeatureType2Post(), true).c_str()));
	}

	if (kProject.isCreateFloodPlains())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_FLOODPLAINS"));
	}

	if (kProject.isCreateHellTerrain())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_HELL_TERRAIN"));
	}

	if (kProject.isCreateRandom()) {
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_RANDOM_TERRAIN"));
	}

	if (kProject.isCreateHills())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_HILLS"));
	}

	if (kProject.isRemovePeak())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_REMOVE_PEAK"));
	}

	if (kProject.isRemoveHellTerrain())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_REMOVE_HELL_TERRAIN"));
	}

	if (kProject.isHostileTerraform())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_HOSTILE"));
	}

	if (kProject.isCreateRiver())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_RIVER"));
	}

	if (kProject.isCreateLand())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_LAND"));
	}

	if (kProject.isDrownLand())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_OCEAN"));
	}

	if (kProject.isTransmutationManaBonus() && kProject.getTechPrereq() != NO_TECH
		&& GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_BONUS_INCREASED_EFFECT", getLinkedText((BonusTypes)GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus()).c_str()));
	}

	if (kProject.isGlobalEnchantmentManaBonus() && kProject.getTechPrereq() != NO_TECH
		&& GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_ENCHANTMENT_BONUS_REDUCED_COST", getLinkedText((BonusTypes)GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus()).c_str(), GC.getGameINLINE().getActivePlayer() == NO_PLAYER ? 0 : range((GET_PLAYER(ePlayer).getNumAvailableBonuses((BonusTypes)GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus()) - 1) * 10, 0, 30)));
	}

	if (kProject.getImprovementTypePre() == NO_IMPROVEMENT && kProject.getImprovementTypePost() != NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_FEATURE", getLinkedText((ImprovementTypes)kProject.getImprovementTypePost(), true).c_str()));
	}

	if (kProject.getImprovementTypePre() != NO_IMPROVEMENT && kProject.getImprovementTypePost() != NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_TRANSFORM_FEATURE", getLinkedText((ImprovementTypes)kProject.getImprovementTypePre(), true).c_str(), getLinkedText((ImprovementTypes)kProject.getImprovementTypePost(), true).c_str()));
	}

	if (kProject.getImprovementTypePre() != NO_IMPROVEMENT && kProject.getImprovementTypePost() == NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_REMOVE_FEATURE", getLinkedText((ImprovementTypes)kProject.getImprovementTypePre(), true).c_str()));
	}

	if (kProject.getImprovementType2Pre() == NO_IMPROVEMENT && kProject.getImprovementType2Post() != NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_CREATE_FEATURE", getLinkedText((ImprovementTypes)kProject.getImprovementType2Post(), true).c_str()));
	}

	if (kProject.getImprovementType2Pre() != NO_IMPROVEMENT && kProject.getImprovementType2Post() != NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_TRANSFORM_FEATURE", getLinkedText((ImprovementTypes)kProject.getImprovementType2Pre(), true).c_str(), getLinkedText((ImprovementTypes)kProject.getImprovementType2Post(), true).c_str()));
	}

	if (kProject.getImprovementType2Pre() != NO_IMPROVEMENT && kProject.getImprovementType2Post() == NO_IMPROVEMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_REMOVE_FEATURE", getLinkedText((ImprovementTypes)kProject.getImprovementType2Pre(), true).c_str()));
	}

	if (kProject.isBanish())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_BANISH"));
	}

	if (kProject.isCallTheVoid())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_CALL_THE_VOID"));
	}

	if (kProject.isChaosRift())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_CHAOS_RIFT"));
	}

	if (kProject.isBirth() && kProject.getManaBonusLinked() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_BIRTH_PEDIA", getLinkedText(kProject.getManaBonusLinked()).c_str()));
	}

	if (kProject.isRessurection())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_RESSURECTION"));
	}

	if (kProject.isDeathWish() && kProject.getManaBonusLinked() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_DEATH_WISH", getLinkedText(kProject.getManaBonusLinked()).c_str()));
	}

	if (kProject.isEarthQuake())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_EARTH_QUAKE"));
	}

	if (kProject.isNatureWrath())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_NATURE_WRATH"));
	}

	if (kProject.getAwarenessLevel() == 1)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_AWARENESS_LEVEL1"));
	}

	if (kProject.getAwarenessLevel() == 2)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_AWARENESS_LEVEL2"));
	}

	if (kProject.getAwarenessLevel() == 3)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_AWARENESS_LEVEL3"));
	}

	if (kProject.getClimateRitual() != NO_PROJECT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_CLIMATE_RITUAL", getLinkedText((ProjectTypes)kProject.getClimateRitual(), true).c_str()));
	}

	if (kProject.getDevilSummon() != NO_PROJECT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_DEVIL_SUMMON", getLinkedText((UnitTypes)kProject.getDevilSummon(), true).c_str()));
	}

	if (kProject.getDiploCharmAlignmentPrereq() != NO_ALIGNMENT
		&& kProject.getManaBonusLinked() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_DIPLOCHARM", GC.getAlignmentInfo((AlignmentTypes)kProject.getDiploCharmAlignmentPrereq()).getDescription(), getLinkedText(kProject.getManaBonusLinked()).c_str()));
	}
	if (kProject.getDisjunctionPower() > 0)
	{
		szBuffer.append(NEWLINE);
		//szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_DISJUNCTION"));
		//SpyFanatic: show percentage
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_DISJUNCTION",10+kProject.getDisjunctionPower()));
	}
	if (kProject.getDoubleCostOfOtherManaSchools() != NO_MANASCHOOL)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_DOUBLECOST_OTHER_MANASCHOOL", GC.getManaschoolInfo((ManaschoolTypes)kProject.getDoubleCostOfOtherManaSchools()).getDescription()));
	}

	bFirst = true;
	if (kProject.getPromotionType() != NO_PROMOTION && kProject.isAddPromotionToUnits())
	{
		szTempBuffer.clear();
		if (GC.getPromotionInfo((PromotionTypes)kProject.getPromotionType()).isAnyUnitCombat())
		{
			szTempBuffer.append(L"all");
		}
		else
		{
			for (int iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
			{
				if (GC.getPromotionInfo((PromotionTypes)kProject.getPromotionType()).getUnitCombat(iI))
				{
					if (bFirst)
					{
						bFirst = false;
					}
					else
					{
						szTempBuffer.append(L", ");
					}
					szTempBuffer.append(getLinkedText((UnitCombatTypes)iI));
				}
			}
		}

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_PROMOTION", getLinkedText((PromotionTypes)kProject.getPromotionType(), true).c_str(), szTempBuffer.c_str()));
	}
	if (kProject.getReduceManaCreation() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_REDUCE_MANA_CREATION", kProject.getReduceManaCreation()));
	}
	if (kProject.getResistHostileTerraforming() > 0)
	{
		szBuffer.append(NEWLINE);
		//szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_RESISTHOSTILE_TERRAFORMING"));
		//SpyFanatic: showing values
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_RESISTHOSTILE_TERRAFORMING",kProject.getResistHostileTerraforming()));
	}

	if (kProject.getDevilSummon() != NO_UNIT)
	{
		if (GC.getDEVIL_PLAYER() == NO_PLAYER || GET_PLAYER((PlayerTypes)GC.getDEVIL_PLAYER()).getNumUnits() == 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_DEVIL_GAME_REQ"));
		}
	}

	if (kProject.getProjectPrereq() != NO_PROJECT)
	{
		if (GC.getGameINLINE().getActivePlayer() == NO_PLAYER || GET_TEAM(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTeam()).getProjectCount(eProject) == 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_REQUIRES_PROJECT", getLinkedText((ProjectTypes)kProject.getProjectPrereq(), true).c_str()));
		}
	}

	if (kProject.getTemporaryTech() != NO_TECH)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_TEMPORARY_TECH", getLinkedText((TechTypes)kProject.getTemporaryTech(), true).c_str()));
	}

	if (kProject.getCommerceChangeType() != NO_YIELD && kProject.getCommerceChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_COMMERCECHANGE", GC.getCommerceInfo((CommerceTypes)kProject.getCommerceChangeType()).getDescription(), kProject.getCommerceChange()));
	}

	if (kProject.getYieldChangeType() != NO_YIELD && kProject.getYieldChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_COMMERCECHANGE", GC.getYieldInfo((YieldTypes)kProject.getYieldChangeType()).getDescription(), kProject.getYieldChange()));
		if (kProject.getBonusPerMana() != 0 && kProject.getManaBonusLinked() != NO_BONUS)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_COMMERCECHANGE_PER_MANA", kProject.getBonusPerMana(), getLinkedText(kProject.getManaBonusLinked()).c_str()));
		}
	}

	if (kProject.getPopulationToExperienceRate() != 0 && kProject.getPromotionType() != NO_PROMOTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_POPULATION_TO_EXP", getLinkedText((PromotionTypes)kProject.getPromotionType()).c_str()));
	}

	if (kProject.getCreateUnitFromExperience() != 0 && kProject.getUnitType() != NO_UNIT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_CREATE_UNIT_FROM_EXP", getLinkedText((UnitTypes)kProject.getUnitType()).c_str()));
	}

	if (kProject.getPrereqReligion() != NO_RELIGION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_PREREQ_RELIGION", getLinkedText((ReligionTypes)kProject.getPrereqReligion()).c_str()));
	}

	if (kProject.getPrereqAlignment() != NO_ALIGNMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_PREREQ_ALIGNMENT", GC.getAlignmentInfo((AlignmentTypes)kProject.getPrereqAlignment()).getDescription()));
	}

	if (kProject.getBlockedAlignment() != NO_ALIGNMENT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_BLOCKED_ALIGNMENT", GC.getAlignmentInfo((AlignmentTypes)kProject.getBlockedAlignment()).getDescription()));
	}

	// DEBUG Sephi
	if (GC.getGameINLINE().isDebugMode())
	{
		if (kProject.getNumTerrainChanges() != 0)
		{
			PlayerTypes ePlayer = GC.getGameINLINE().getActivePlayer();
			if (ePlayer != NO_PLAYER)
			{
				szTempBuffer.Format(L"\n AI TerraformValue: %d", GET_PLAYER(ePlayer).AI_getTerraformRitualValue(eProject));
				szBuffer.append(szTempBuffer);
			}
		}

		if (kProject.getSummonUnitType() != NO_UNIT)
		{
			PlayerTypes ePlayer = GC.getGameINLINE().getActivePlayer();
			if (ePlayer != NO_PLAYER && GET_PLAYER(ePlayer).getCapitalCity() != NULL)
			{
				CvCityAI* pCityAI = static_cast<CvCityAI*>(GET_PLAYER(ePlayer).getCapitalCity());
				szTempBuffer.Format(L"\n AI SummenValue: %d", pCityAI->AI_getUnitValue((UnitTypes)kProject.getSummonUnitType()));
				szBuffer.append(szTempBuffer);
			}
		}
	}

	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (kProject.getNukeInterception() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_CHANCE_INTERCEPT_NUKES", kProject.getNukeInterception()));
	}

	if (kProject.getTechShare() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TECH_SHARE", kProject.getTechShare()));
	}

	if (kProject.isAllowsNukes())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_ENABLES_NUKES"));
	}

	if (kProject.getEveryoneSpecialUnit() != NO_SPECIALUNIT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_ENABLES_SPECIAL", GC.getSpecialUnitInfo((SpecialUnitTypes)(kProject.getEveryoneSpecialUnit())).getTextKeyWide()));
	}

	if (kProject.getEveryoneSpecialBuilding() != NO_SPECIALBUILDING)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_ENABLES_SPECIAL", GC.getSpecialBuildingInfo((SpecialBuildingTypes)(kProject.getEveryoneSpecialBuilding())).getTextKeyWide()));
	}

	for (iI = 0; iI < GC.getNumVictoryInfos(); ++iI)
	{
		if (kProject.getVictoryThreshold(iI) > 0)
		{
			if (kProject.getVictoryThreshold(iI) == kProject.getVictoryMinThreshold(iI))
			{
				szTempBuffer.Format(L"%d", kProject.getVictoryThreshold(iI));
			}
			else
			{
				szTempBuffer.Format(L"%d-%d", kProject.getVictoryMinThreshold(iI), kProject.getVictoryThreshold(iI));
			}

			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_REQUIRED_FOR_VICTORY", szTempBuffer.GetCString(), GC.getVictoryInfo((VictoryTypes)iI).getTextKeyWide()));
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumProjectInfos(); ++iI)
	{
		if (GC.getProjectInfo((ProjectTypes)iI).getAnyoneProjectPrereq() == eProject)
		{
			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_PROJECT_REQUIRED_TO_CREATE_ANYONE").c_str());
			//>>>>	BUGFfH: Modified by Denev 2009/10/02
			//			szTempBuffer.Format(SETCOLR L"<link=literal>%s</link>" ENDCOLR, TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo((ProjectTypes)iI).getDescription());
			szTempBuffer.assign(getLinkedText((ProjectTypes)iI, false));
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}

	bFirst = true;

	for (iI = 0; iI < GC.getNumProjectInfos(); ++iI)
	{
		if (GC.getProjectInfo((ProjectTypes)iI).getProjectsNeeded(eProject) > 0)
		{
			if ((pCity == NULL) || pCity->canCreate(((ProjectTypes)iI), false, true))
			{
				szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_PROJECT_REQUIRED_TO_CREATE").c_str());
				//>>>>	BUGFfH: Modified by Denev 2009/10/02
				//				szTempBuffer.Format(SETCOLR L"<link=literal>%s</link>" ENDCOLR, TEXT_COLOR("COLOR_PROJECT_TEXT"), GC.getProjectInfo((ProjectTypes)iI).getDescription());
				szTempBuffer.assign(getLinkedText((ProjectTypes)iI));
				//<<<<	BUGFfH: End Modify
				setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", bFirst);
				bFirst = false;
			}
		}
	}

	//FfH: Added by Kael 08/26/2008
	if (kProject.getModifyGlobalCounter() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MESSAGE_MODIFY_GLOBAL_COUNTER", kProject.getModifyGlobalCounter()));
	}
	if (kProject.getPrereqCivilization() != NO_CIVILIZATION)
	{
		if (pCity == NULL || pCity->getCivilizationType() != kProject.getPrereqCivilization())
		{
			szBuffer.append(NEWLINE);
			//>>>>	BUGFfH: Modified by Denev 2009/09/15
			//			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PREREQ_CIVILIZATION", GC.getCivilizationInfo((CivilizationTypes)kProject.getPrereqCivilization()).getDescription()));
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PREREQ_CIVILIZATION", getLinkedText((CivilizationTypes)kProject.getPrereqCivilization(), false).c_str()));
			//<<<<	BUGFfH: End Modify
		}
	}
	if (kProject.getPrereqGlobalCounter() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_PREREQ_GLOBAL_COUNTER", kProject.getPrereqGlobalCounter()));
	}
	//FfH: End Add

	if ((pCity == NULL) || !(pCity->canCreate(eProject)))
	{
		if (pCity != NULL)
		{
			if (GC.getGameINLINE().isNoNukes())
			{
				if (kProject.isAllowsNukes())
				{
					for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
					{
						if (GC.getUnitInfo((UnitTypes)iI).getNukeRange() != -1)
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_NO_NUKES"));
							break;
						}
					}
				}
			}
		}

		if (kProject.getAnyoneProjectPrereq() != NO_PROJECT)
		{
			if ((pCity == NULL) || (GC.getGameINLINE().getProjectCreatedCount((ProjectTypes)(kProject.getAnyoneProjectPrereq())) == 0))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_REQUIRES_ANYONE", GC.getProjectInfo((ProjectTypes)kProject.getAnyoneProjectPrereq()).getTextKeyWide()));
			}
		}

		for (iI = 0; iI < GC.getNumProjectInfos(); ++iI)
		{
			if (kProject.getProjectsNeeded(iI) > 0)
			{
				if ((pCity == NULL) || (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount((ProjectTypes)iI) < kProject.getProjectsNeeded(iI)))
				{
					if (pCity != NULL)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_REQUIRES", GC.getProjectInfo((ProjectTypes)iI).getTextKeyWide(), GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getProjectCount((ProjectTypes)iI), kProject.getProjectsNeeded(iI)));
					}
					else
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_REQUIRES_NO_CITY", GC.getProjectInfo((ProjectTypes)iI).getTextKeyWide(), kProject.getProjectsNeeded(iI)));
					}
				}
			}
		}

		if (bCivilopediaText)
		{
			if (kProject.getVictoryPrereq() != NO_VICTORY)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_REQUIRES_STRING_VICTORY", GC.getVictoryInfo((VictoryTypes)(kProject.getVictoryPrereq())).getTextKeyWide()));
			}
		}
	}

	if (!kProject.isGlobalEnchantment())
	{
		if (!bCivilopediaText)
		{
			if (pCity == NULL)
			{
				if (ePlayer != NO_PLAYER)
				{
					szTempBuffer.Format(L"\n%d%c", GET_PLAYER(ePlayer).getProductionNeeded(eProject), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				}
				else
				{
					szTempBuffer.Format(L"\n%d%c", kProject.getProductionCost(), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				}
				szBuffer.append(szTempBuffer);
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_NUM_TURNS", pCity->getProductionTurnsLeft(eProject, ((gDLL->ctrlKey() || !(gDLL->shiftKey())) ? 0 : pCity->getOrderQueueLength()))));

				iProduction = pCity->getProjectProduction(eProject);

				if (iProduction > 0)
				{
					szTempBuffer.Format(L" - %d/%d%c", iProduction, pCity->getProductionNeeded(eProject), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				}
				else
				{
					szTempBuffer.Format(L" - %d%c", pCity->getProductionNeeded(eProject), GC.getYieldInfo(YIELD_PRODUCTION).getChar());
				}
				szBuffer.append(szTempBuffer);
			}
		}
	}

	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		if (kProject.getBonusProductionModifier(iI) != 0)
		{
			if (pCity != NULL)
			{
				if (pCity->hasBonus((BonusTypes)iI))
				{
					szBuffer.append(gDLL->getText("TXT_KEY_COLOR_POSITIVE"));
				}
				else
				{
					szBuffer.append(gDLL->getText("TXT_KEY_COLOR_NEGATIVE"));
				}
			}
			if (!bCivilopediaText)
			{
				szBuffer.append(L" (");
			}
			else
			{
				szTempBuffer.Format(L"%s%c", NEWLINE, gDLL->getSymbolID(BULLET_CHAR), szTempBuffer);
				szBuffer.append(szTempBuffer);
			}
			if (kProject.getBonusProductionModifier(iI) == 100)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_DOUBLE_SPEED_WITH", GC.getBonusInfo((BonusTypes)iI).getTextKeyWide()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_BUILDS_FASTER_WITH", kProject.getBonusProductionModifier(iI), GC.getBonusInfo((BonusTypes)iI).getTextKeyWide()));
			}
			if (!bCivilopediaText)
			{
				szBuffer.append(L')');
			}
			if (pCity != NULL)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_COLOR_REVERT"));
			}
		}
	}

	if (kProject.isGlobalEnchantment() && !kProject.isEffectOnly())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_LAST_UNTIL_CANCELLED"));
	}

	if (kProject.isHostile())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_TERRAFORM_HOSTILE"));
	}

	int iProjectManaUpkeepPerCities = kProject.getManaUpkeepPerCityTimes100();
	if (iProjectManaUpkeep != 0 || iProjectManaUpkeepPerCities != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_MANA_COST_UPKEEP"));

		if (iProjectManaUpkeep != 0)
		{
			if (ePlayer != NO_PLAYER && kProject.isGlobalEnchantmentManaBonus()
				&& kProject.getTechPrereq() != NO_TECH && GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus() != NO_BONUS
				&& (GET_PLAYER(ePlayer).getNumAvailableBonuses((BonusTypes)GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus()) - 1) > 1)
				iProjectManaUpkeep = (iProjectManaUpkeep * (100 - (range((GET_PLAYER(ePlayer).getNumAvailableBonuses((BonusTypes)GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus()) - 1), 1, 3) * 10))) / 100;

			szBuffer.append(CvWString::format(L"%d", iProjectManaUpkeep));
		}

		if (iProjectManaUpkeep != 0 && iProjectManaUpkeepPerCities != 0)
		{
			szBuffer.append(CvWString::format(L" + "));
		}

		if (iProjectManaUpkeepPerCities != 0)
		{
			if (ePlayer != NO_PLAYER && kProject.isGlobalEnchantmentManaBonus()
				&& kProject.getTechPrereq() != NO_TECH && GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus() != NO_BONUS
				&& (GET_PLAYER(ePlayer).getNumAvailableBonuses((BonusTypes)GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus()) - 1) > 1)
				iProjectManaUpkeepPerCities = (iProjectManaUpkeepPerCities * (100 - (range((GET_PLAYER(ePlayer).getNumAvailableBonuses((BonusTypes)GC.getTechInfo((TechTypes)kProject.getTechPrereq()).getPrereqBonus()) - 1), 1, 3) * 10))) / 100;
			
			szBuffer.append(CvWString::format(L"%.2f", 0.01f * iProjectManaUpkeepPerCities));
			szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_MANA_COST_UPKEEP_PER_CITIES"));
			
			if (ePlayer != NO_PLAYER)
				szBuffer.append(CvWString::format(L"(%.2f total)", (0.01f * iProjectManaUpkeepPerCities * (ePlayer != NO_PLAYER ? GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getNumCities() : 1))));
		}
	}

	if (wcslen(kProject.getHelp()) > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(kProject.getHelp());
	}

	if (kProject.getBuildingType() != NO_BUILDING)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROJECT_GLOBAL_BUILDING", getLinkedText((BuildingTypes)kProject.getBuildingType(), true).c_str()));
		szBuffer.append(NEWLINE);
		CvWStringBuffer szNewBuffer;
		setBuildingHelp(szNewBuffer, (BuildingTypes)kProject.getBuildingType());
		szBuffer.append(szNewBuffer.getCString());
	}
}


void CvGameTextMgr::setProcessHelp(CvWStringBuffer& szBuffer, ProcessTypes eProcess)
{
	int iI;

	szBuffer.append(GC.getProcessInfo(eProcess).getDescription());

	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		if (GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI) != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROCESS_CONVERTS", GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iI), GC.getYieldInfo(YIELD_PRODUCTION).getChar(), GC.getCommerceInfo((CommerceTypes)iI).getChar()));
		}
	}

	CvCity* pCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
	if (pCity != NULL)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_PROCESS_INCREASES_SPECIALIZATION", pCity->calculateCitySpecializationPointsPerTurn(eProcess)));
	}
}

void CvGameTextMgr::setBadHealthHelp(CvWStringBuffer& szBuffer, CvCity& city)
{
	CvPlot* pLoopPlot;
	FeatureTypes eFeature;
	ImprovementTypes eImprovement;
	int iHealth;
	int iI;

	int iCityBadHealth = city.badHealth();
	if (iCityBadHealth > 0)
	{
		iHealth = -(city.getFreshWaterBadHealth());
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FROM_FRESH_WATER", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = -(city.getFeatureBadHealth());
		if (iHealth > 0)
		{
			eFeature = NO_FEATURE;

			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				pLoopPlot = plotCity(city.getX_INLINE(), city.getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getHealthPercent() < 0)
						{
							if (eFeature == NO_FEATURE)
							{
								eFeature = pLoopPlot->getFeatureType();
							}
							else if (eFeature != pLoopPlot->getFeatureType())
							{
								eFeature = NO_FEATURE;
								break;
							}
						}
					}
				}
			}

			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FEAT_HEALTH", iHealth, ((eFeature == NO_FEATURE) ? L"TXT_KEY_MISC_FEATURES" : GC.getFeatureInfo(eFeature).getTextKeyWide())));
			szBuffer.append(NEWLINE);
		}

		iHealth = -(city.getImprovementHealthChange());
		if (iHealth > 0)
		{
			eImprovement = NO_IMPROVEMENT;

			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				pLoopPlot = plotCity(city.getX_INLINE(), city.getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
					{
						if (GC.getImprovementInfo(pLoopPlot->getImprovementType()).getHealth() < 0)
						{
							if (eImprovement == NO_IMPROVEMENT)
							{
								eImprovement = pLoopPlot->getImprovementType();
							}
							else if (eImprovement != pLoopPlot->getImprovementType())
							{
								eImprovement = NO_IMPROVEMENT;
								break;
							}
						}
					}
				}
			}

			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FEAT_HEALTH", iHealth, ((eImprovement == NO_IMPROVEMENT) ? L"TXT_KEY_MISC_IMPROVEMENTS" : GC.getImprovementInfo(eImprovement).getTextKeyWide())));
			szBuffer.append(NEWLINE);
		}


		iHealth = city.getEspionageHealthCounter();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_ESPIONAGE", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = -(city.getPowerBadHealth());
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_POWER", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = -(city.getBonusBadHealth());
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_BONUSES", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = -(city.totalBadBuildingHealth());
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_BUILDINGS", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = -(GET_PLAYER(city.getOwnerINLINE()).getExtraHealth());
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_CIV", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = -city.getExtraHealth();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_UNHEALTH_EXTRA", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = -(GC.getHandicapInfo(city.getHandicapType()).getHealthBonus());
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_HANDICAP", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = city.unhealthyPopulation();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_POP", iHealth));
			szBuffer.append(NEWLINE);
		}

		szBuffer.append(L"-----------------------\n");

		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TOTAL_UNHEALTHY", iCityBadHealth));
	}
}

void CvGameTextMgr::setGoodHealthHelp(CvWStringBuffer& szBuffer, CvCity& city)
{
	CvPlot* pLoopPlot;
	FeatureTypes eFeature;
	int iHealth;
	int iI;

	int iCityGoodHealth = city.goodHealth();
	if (iCityGoodHealth > 0)
	{
		iHealth = city.getFreshWaterGoodHealth();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_FROM_FRESH_WATER", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = city.getFeatureGoodHealth();
		if (iHealth > 0)
		{
			eFeature = NO_FEATURE;

			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				pLoopPlot = plotCity(city.getX_INLINE(), city.getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->getFeatureType() != NO_FEATURE)
					{
						if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getHealthPercent() > 0)
						{
							if (eFeature == NO_FEATURE)
							{
								eFeature = pLoopPlot->getFeatureType();
							}
							else if (eFeature != pLoopPlot->getFeatureType())
							{
								eFeature = NO_FEATURE;
								break;
							}
						}
					}
				}
			}

			szBuffer.append(gDLL->getText("TXT_KEY_MISC_FEAT_GOOD_HEALTH", iHealth, ((eFeature == NO_FEATURE) ? L"TXT_KEY_MISC_FEATURES" : GC.getFeatureInfo(eFeature).getTextKeyWide())));
			szBuffer.append(NEWLINE);
		}

		iHealth = city.getPowerGoodHealth();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_POWER", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = city.getBonusGoodHealth();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_BONUSES", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = city.totalGoodBuildingHealth();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_BUILDINGS", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = GET_PLAYER(city.getOwnerINLINE()).getExtraHealth();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_CIV", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = city.getExtraHealth();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_EXTRA", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth=0; //SpyFanatic: reset iHealth
		if (!city.isHuman()) {
			int iInterval = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIFreeHappinessInterval();
			if (iInterval > 0) {
				int iAIBonus = GC.getGame().getElapsedGameTurns();
				iAIBonus *= GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getGrowthPercent();
				iAIBonus /= 100;
				iAIBonus /= iInterval;
				iHealth = std::max(0, iAIBonus);
			}
		}

		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_HEALTHY_YEAH", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = GC.getHandicapInfo(city.getHandicapType()).getHealthBonus();
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_HANDICAP", iHealth));
			szBuffer.append(NEWLINE);
		}

		iHealth = (GET_PLAYER((PlayerTypes)city.getOwner()).getHealthRatePerCulture() * city.getCultureLevel()) / 100;
		if (iHealth > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOOD_HEALTH_FROM_CULTURE", iHealth));
			szBuffer.append(NEWLINE);
		}

		szBuffer.append(L"-----------------------\n");

		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TOTAL_HEALTHY", iCityGoodHealth));
	}
}

void CvGameTextMgr::setAngerHelp(CvWStringBuffer& szBuffer, CvCity& city)
{
	int iOldAngerPercent;
	int iNewAngerPercent;
	int iOldAnger;
	int iNewAnger;
	int iAnger;
	int iI;

	if (city.isOccupation())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_ANGER_RESISTANCE"));
	}
	else if (GET_PLAYER(city.getOwnerINLINE()).isAnarchy())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_ANGER_ANARCHY"));
	}
	else if (city.unhappyLevel() > 0)
	{
		iOldAngerPercent = 0;
		iNewAngerPercent = 0;

		iOldAnger = 0;
		iNewAnger = 0;

		// XXX decomp these???
		iNewAngerPercent += city.getOvercrowdingPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_OVERCROWDING", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getNoMilitaryPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_MILITARY_PROTECTION", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getCulturePercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_OCCUPIED", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getReligionPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_RELIGION_FIGHT", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getHurryPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_OPPRESSION", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getConscriptPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_DRAFT", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getDefyResolutionPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_DEFY_RESOLUTION", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAngerPercent += city.getWarWearinessPercentAnger();
		iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_WAR_WEAR", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAngerPercent = iNewAngerPercent;
		iOldAnger = iNewAnger;

		iNewAnger += std::max(0, city.getVassalUnhappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_UNHAPPY_VASSAL", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger += std::max(0, city.getEspionageHappinessCounter());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_ESPIONAGE", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		for (iI = 0; iI < GC.getNumCivicInfos(); ++iI)
		{
			iNewAngerPercent += GET_PLAYER(city.getOwnerINLINE()).getCivicPercentAnger((CivicTypes)iI);
			iNewAnger += (((iNewAngerPercent * city.getPopulation()) / GC.getPERCENT_ANGER_DIVISOR()) - ((iOldAngerPercent * city.getPopulation()) / GC.getDefineINT("PERCENT_ANGER_DIVISOR")));
			iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
			if (iAnger > 0)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ANGER_DEMAND_CIVIC", iAnger, GC.getCivicInfo((CivicTypes)iI).getTextKeyWide()));
				szBuffer.append(NEWLINE);
			}
			iOldAngerPercent = iNewAngerPercent;
			iOldAnger = iNewAnger;
		}

		iNewAnger -= std::min(0, city.getLargestCityHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_BIG_CITY", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, city.getMilitaryHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_MILITARY_PRESENCE", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, city.getCurrentStateReligionHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_STATE_RELIGION", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, (city.getBuildingBadHappiness() + city.getExtraBuildingBadHappiness()));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_BUILDINGS", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, city.getFeatureBadHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_FEATURES_NEW", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, city.getBonusBadHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_BONUS", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, city.getReligionBadHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_RELIGIOUS_FREEDOM", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, city.getCommerceHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_BAD_ENTERTAINMENT", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, city.area()->getBuildingHappiness(city.getOwnerINLINE()));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_BUILDINGS", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, GET_PLAYER(city.getOwnerINLINE()).getBuildingHappiness());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_BUILDINGS", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, (city.getExtraHappiness() + GET_PLAYER(city.getOwnerINLINE()).getExtraHappiness()));
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_ARGH", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		iNewAnger -= std::min(0, GC.getHandicapInfo(city.getHandicapType()).getHappyBonus());
		iAnger = ((iNewAnger - iOldAnger) + std::min(0, iOldAnger));
		if (iAnger > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ANGER_HANDICAP", iAnger));
			szBuffer.append(NEWLINE);
		}
		iOldAnger = iNewAnger;

		szBuffer.append(L"-----------------------\n");

		szBuffer.append(gDLL->getText("TXT_KEY_ANGER_TOTAL_UNHAPPY", iOldAnger));

		FAssert(iOldAnger == city.unhappyLevel());
	}
}


void CvGameTextMgr::setHappyHelp(CvWStringBuffer& szBuffer, CvCity& city)
{
	int iHappy;
	int iTotalHappy = 0;

	if (city.isOccupation() || GET_PLAYER(city.getOwnerINLINE()).isAnarchy())
	{
		return;
	}
	if (city.happyLevel() > 0)
	{
		iHappy = city.getLargestCityHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_BIG_CITY", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.getMilitaryHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_MILITARY_PRESENCE", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.getVassalHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_VASSAL", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.getCurrentStateReligionHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_STATE_RELIGION", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = (city.getBuildingGoodHappiness() + city.getExtraBuildingGoodHappiness());
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_BUILDINGS", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.getFeatureGoodHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_FEATURES", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.getBonusGoodHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_BONUS", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.getReligionGoodHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_RELIGIOUS_FREEDOM", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.getCommerceHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_ENTERTAINMENT", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = city.area()->getBuildingHappiness(city.getOwnerINLINE());
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_BUILDINGS", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = GET_PLAYER(city.getOwnerINLINE()).getBuildingHappiness();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_BUILDINGS", iHappy));
			szBuffer.append(NEWLINE);
		}

		iHappy = ((city.getExtraHappiness() > 0 ? city.getExtraHappiness() : 0) + (GET_PLAYER(city.getOwnerINLINE()).getExtraHappiness() > 0 ? GET_PLAYER(city.getOwnerINLINE()).getExtraHappiness() : 0) + city.getAITurnHappinessBonus());
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_YEAH", iHappy));
			szBuffer.append(NEWLINE);
		}

		if (city.getHappinessTimer() > 0)
		{
			iHappy = GC.getDefineINT("TEMP_HAPPY");
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_TEMP", iHappy, city.getHappinessTimer()));
			szBuffer.append(NEWLINE);
		}

		iHappy = GC.getHandicapInfo(city.getHandicapType()).getHappyBonus();
		if (iHappy > 0)
		{
			iTotalHappy += iHappy;
			szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_HANDICAP", iHappy));
			szBuffer.append(NEWLINE);
		}

		szBuffer.append(L"-----------------------\n");

		szBuffer.append(gDLL->getText("TXT_KEY_HAPPY_TOTAL_HAPPY", iTotalHappy));

		FAssert(iTotalHappy == city.happyLevel())
	}
}

/** Moved to BUG Mod below Sephi
void CvGameTextMgr::setYieldChangeHelp(CvWStringBuffer &szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piYieldChange, bool bPercent, bool bNewLine)
{
	CvWString szTempBuffer;
	bool bStarted;
	int iI;

	bStarted = false;

	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		if (piYieldChange[iI] != 0)
		{
			if (!bStarted)
			{
				if (bNewLine)
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
				}
				szTempBuffer += CvWString::format(L"%s%s%s%d%s%c",
					szStart.GetCString(),
					szSpace.GetCString(),
					piYieldChange[iI] > 0 ? L"+" : L"",
					piYieldChange[iI],
					bPercent ? L"%" : L"",
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
			else
			{
				szTempBuffer.Format(L", %s%d%s%c",
					piYieldChange[iI] > 0 ? L"+" : L"",
					piYieldChange[iI],
					bPercent ? L"%" : L"",
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
			szBuffer.append(szTempBuffer);

			bStarted = true;
		}
	}

	if (bStarted)
	{
		szBuffer.append(szEnd);
	}
}

void CvGameTextMgr::setCommerceChangeHelp(CvWStringBuffer &szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piCommerceChange, bool bPercent, bool bNewLine)
{
	CvWString szTempBuffer;
	bool bStarted;
	int iI;

	bStarted = false;

	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		if (piCommerceChange[iI] != 0)
		{
			if (!bStarted)
			{
				if (bNewLine)
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
				}
				szTempBuffer += CvWString::format(L"%s%s%s%d%s%c", szStart.GetCString(), szSpace.GetCString(), ((piCommerceChange[iI] > 0) ? L"+" : L""), piCommerceChange[iI], ((bPercent) ? L"%" : L""), GC.getCommerceInfo((CommerceTypes) iI).getChar());
			}
			else
			{
				szTempBuffer.Format(L", %s%d%s%c", ((piCommerceChange[iI] > 0) ? L"+" : L""), piCommerceChange[iI], ((bPercent) ? L"%" : L""), GC.getCommerceInfo((CommerceTypes) iI).getChar());
			}
			szBuffer.append(szTempBuffer);

			bStarted = true;
		}
	}

	if (bStarted)
	{
		szBuffer.append(szEnd);
	}
}
 End Move Sephi **/

void CvGameTextMgr::setBonusHelp(CvWStringBuffer& szBuffer, BonusTypes eBonus, bool bCivilopediaText)
{
	if (NO_BONUS == eBonus)
	{
		return;
	}

	if (!bCivilopediaText)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szBuffer.append(CvWString::format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo(eBonus).getDescription()));
		szBuffer.append(getLinkedText(eBonus));
		//<<<<	BUGFfH: End Modify

		if (NO_PLAYER != GC.getGameINLINE().getActivePlayer())
		{
			CvPlayer& kActivePlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_AVAILABLE_PLAYER", kActivePlayer.getNumAvailableBonuses(eBonus), kActivePlayer.getNameKey()));

			for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
			{
				bool bFound = false;
				if (kActivePlayer.isActiveCorporation((CorporationTypes)iCorp))
				{
					for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
					{
						if (eBonus == GC.getCorporationInfo((CorporationTypes)iCorp).getPrereqBonus(i))
						{
							int iLoop;
							for (CvCity* pCity = kActivePlayer.firstCity(&iLoop); NULL != pCity; pCity = kActivePlayer.nextCity(&iLoop))
							{
								if (pCity->isHasCorporation((CorporationTypes)iCorp))
								{
									bFound = true;
									break;
								}
							}
						}

						if (bFound)
						{
							break;
						}
					}
				}

				if (bFound)
				{
					szBuffer.append(GC.getCorporationInfo((CorporationTypes)iCorp).getChar());
				}
			}
		}

		//>>>>	BUGFfH: Modified by Denev 2009/09/23
		/*	Resources modify ONLY tile yields, unlike happiness or health.	*/
		//		setYieldChangeHelp(szBuffer, L"", L"", L"", GC.getBonusInfo(eBonus).getYieldChangeArray());
		const CvWString szStart = CvWString::format(L", %s", gDLL->getText("TXT_KEY_BONUS_TILE_MODIFIER").c_str());
		setYieldChangeHelp(szBuffer, szStart, L"", L"", GC.getBonusInfo(eBonus).getYieldChangeArray(), false, false);
		//<<<<	BUGFfH: End Modify

		if (GC.getBonusInfo(eBonus).getTechReveal() != NO_TECH)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_REVEALED_BY", GC.getTechInfo((TechTypes)GC.getBonusInfo(eBonus).getTechReveal()).getTextKeyWide()));
		}
	}

	//>>>>	BUGFfH: Deleted by Denev 2009/09/23
	/*
		ImprovementTypes eImprovement = NO_IMPROVEMENT;
		for (int iLoopImprovement = 0; iLoopImprovement < GC.getNumImprovementInfos(); iLoopImprovement++)
		{
			if (GC.getImprovementInfo((ImprovementTypes)iLoopImprovement).isImprovementBonusMakesValid(eBonus))
			{
				eImprovement = (ImprovementTypes)iLoopImprovement;
				break;
			}
		}
	*/
	//<<<<	BUGFfH: End Delete

	if (GC.getBonusInfo(eBonus).getHealth() != 0)
	{
		if (GC.getBonusInfo(eBonus).getHealth() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_HEALTHY", GC.getBonusInfo(eBonus).getHealth()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_UNHEALTHY", -GC.getBonusInfo(eBonus).getHealth()));
		}

		//>>>>	BUGFfH: Deleted by Denev 2009/09/23
		/*	Improvement is necessary for supplying resources in itself, rather than for granting extra happiness.
				if (eImprovement != NO_IMPROVEMENT)
				{
		//>>>>	BUGFfH: Modified by Denev 2009/09/14
		//			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", GC.getImprovementInfo(eImprovement).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", getLinkedText(eImprovement).c_str()));
		//<<<<	BUGFfH: End Modify
				}
		*/
		//<<<<	BUGFfH: End Delete
	}

	if (GC.getBonusInfo(eBonus).getHappiness() != 0)
	{
		if (GC.getBonusInfo(eBonus).getHappiness() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_HAPPY", GC.getBonusInfo(eBonus).getHappiness()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_UNHAPPY", -GC.getBonusInfo(eBonus).getHappiness()));
		}

		//>>>>	BUGFfH: Deleted by Denev 2009/09/23
		/*	Improvement is necessary for supplying resources in itself, rather than for granting extra happiness.
				if (eImprovement != NO_IMPROVEMENT)
				{
		//>>>>	BUGFfH: Modified by Denev 2009/09/14
		//			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", GC.getImprovementInfo(eImprovement).getTextKeyWide()));
					szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", getLinkedText(eImprovement).c_str()));
		//<<<<	BUGFfH: End Modify
				}
		*/
		//<<<<	BUGFfH: End Delete
	}

	int iCityBonusModifier = GC.getBonusInfo(eBonus).getCityBonusModifier();

	if (iCityBonusModifier != 0) {

		if (GC.getBonusInfo(eBonus).getCityYieldModifierType() != NO_YIELD) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_CITY_MODIFIER", iCityBonusModifier, GC.getYieldInfo((YieldTypes)GC.getBonusInfo(eBonus).getCityYieldModifierType()).getChar()));
		}

		if (GC.getBonusInfo(eBonus).getCityCommerceModifierType() != NO_COMMERCE) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_CITY_MODIFIER", iCityBonusModifier, GC.getCommerceInfo((CommerceTypes)GC.getBonusInfo(eBonus).getCityCommerceModifierType()).getChar()));
		}
	}


	//FfH: Added by Kael 08/21/2007
	if (GC.getBonusInfo(eBonus).getDiscoverRandModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		if (GC.getBonusInfo(eBonus).getDiscoverRandModifier() > 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_DISCOVER_RAND_MODIFIER_INCREASE"));
		}
		if (GC.getBonusInfo(eBonus).getDiscoverRandModifier() < 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_DISCOVER_RAND_MODIFIER_DECREASE"));
		}
	}
	if (GC.getBonusInfo(eBonus).getGreatPeopleRateModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_GREAT_PEOPLE_RATE_MODIFIER", GC.getBonusInfo(eBonus).getGreatPeopleRateModifier()));
	}
	if (GC.getBonusInfo(eBonus).getHealChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_HEAL_CHANGE", GC.getBonusInfo(eBonus).getHealChange()));
	}
	if (GC.getBonusInfo(eBonus).getHealChangeEnemy() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_HEAL_CHANGE_ENEMY", GC.getBonusInfo(eBonus).getHealChangeEnemy()));
	}
	if (GC.getBonusInfo(eBonus).getMaintenanceModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MAINTENANCE_MODIFIER", GC.getBonusInfo(eBonus).getMaintenanceModifier()));
	}
	if (GC.getBonusInfo(eBonus).getResearchModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_RESEARCH_MODIFIER", GC.getBonusInfo(eBonus).getResearchModifier()));
	}
	//>>>>	BUGFfH: Added by Denev 2009/09/24
	if (GC.getBonusInfo(eBonus).getFreePromotion() != NO_PROMOTION)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_FREE_PROMOTION", getLinkedText((PromotionTypes)GC.getBonusInfo(eBonus).getFreePromotion()).c_str()));
	}

	if (GC.getBonusInfo(eBonus).getMutateChance() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_FREE_PROMOTION_CHANCE",
			GC.getBonusInfo(eBonus).getMutateChance(),
			getLinkedText((PromotionTypes)GC.getDefineINT("MUTATED_PROMOTION")).c_str()));
	}

	if (GC.getBonusInfo(eBonus).getBadAttitude() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_FORBIDDEN_MANA_PENALTY", GC.getBonusInfo(eBonus).getBadAttitude()));
	}
	//<<<<	BUGFfH: End Add
	if (GC.getBonusInfo(eBonus).isModifierPerBonus())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MODIFIER_PER_BONUS"));
	}
	//FfH: End Add
	for (int iI = 0; iI < GC.getNumManaschoolInfos(); iI++)
	{
		if (GC.getManaschoolInfo((ManaschoolTypes)iI).isBonusLinked(eBonus))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MANASCHOOL_LINKED", GC.getManaschoolInfo((ManaschoolTypes)iI).getDescription()));
		}

		if (GC.getManaschoolInfo((ManaschoolTypes)iI).getBonusIncreasedMana() == eBonus)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MORE_MANA"));
		}

		if (GC.getManaschoolInfo((ManaschoolTypes)iI).getBonusIncreasedSpellResearch() == eBonus)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MORE_SPELL_RESEARCH"));
		}

		if (GC.getManaschoolInfo((ManaschoolTypes)iI).getBonusIncreasedTerraformEffect() == eBonus)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_MORE_TERRAFORM_EFFECT", GC.getManaschoolInfo((ManaschoolTypes)iI).getDescription()));
		}

		if (GC.getManaschoolInfo((ManaschoolTypes)iI).getBonusReducedGlobalEnchantment() == eBonus)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_REDUCED_ENCHANTMENT", GC.getManaschoolInfo((ManaschoolTypes)iI).getDescription()));
		}

		if (GC.getManaschoolInfo((ManaschoolTypes)iI).getBonusReducedTerraformCost() == eBonus)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_REDUCED_TERRAFORM", GC.getManaschoolInfo((ManaschoolTypes)iI).getDescription()));
		}

		if (GC.getManaschoolInfo((ManaschoolTypes)iI).getBonusSummonStrength() == eBonus)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BONUS_SUMMON_STRENGTH", GC.getManaschoolInfo((ManaschoolTypes)iI).getDescription()));
		}
	}

	CivilizationTypes eCivilization = GC.getGameINLINE().getActiveCivilizationType();

	for (int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
		BuildingTypes eLoopBuilding;
		if (eCivilization == NO_CIVILIZATION)
		{
			eLoopBuilding = ((BuildingTypes)(GC.getBuildingClassInfo((BuildingClassTypes)i).getDefaultBuildingIndex()));
		}
		else
		{
			eLoopBuilding = ((BuildingTypes)(GC.getCivilizationInfo(eCivilization).getCivilizationBuildings(i)));
		}

		if (eLoopBuilding != NO_BUILDING)
		{
			CvBuildingInfo& kBuilding = GC.getBuildingInfo(eLoopBuilding);
			if (kBuilding.getBonusHappinessChanges(eBonus) != 0)
			{
				if (kBuilding.getBonusHappinessChanges(eBonus) > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BONUS_HAPPY", kBuilding.getBonusHappinessChanges(eBonus)));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BONUS_UNHAPPY", -kBuilding.getBonusHappinessChanges(eBonus)));
				}

				//>>>>	BUGFfH: Modified by Denev 2009/09/14
				//				szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", kBuilding.getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", getLinkedText(eLoopBuilding).c_str()));
				//<<<<	BUGFfH: End Modify
			}

			if (kBuilding.getBonusHealthChanges(eBonus) != 0)
			{
				if (kBuilding.getBonusHealthChanges(eBonus) > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BONUS_HEALTHY", kBuilding.getBonusHealthChanges(eBonus)));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_BONUS_UNHEALTHY", -kBuilding.getBonusHealthChanges(eBonus)));
				}

				//>>>>	BUGFfH: Modified by Denev 2009/09/14
				//				szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", kBuilding.getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_BONUS_WITH_IMPROVEMENT", getLinkedText(eLoopBuilding).c_str()));
				//<<<<	BUGFfH: End Modify
			}
		}
	}

	if (!CvWString(GC.getBonusInfo(eBonus).getHelp()).empty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(GC.getBonusInfo(eBonus).getHelp());
	}
}

void CvGameTextMgr::setReligionHelp(CvWStringBuffer& szBuffer, ReligionTypes eReligion, bool bCivilopedia)
{
	UnitTypes eFreeUnit;

	if (NO_RELIGION == eReligion)
	{
		return;
	}
	CvReligionInfo& religion = GC.getReligionInfo(eReligion);

	if (!bCivilopedia)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szBuffer.append(CvWString::format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), religion.getDescription()));
		szBuffer.append(getLinkedText(eReligion));
		//<<<<	BUGFfH: End Modify
	}

	setCommerceChangeHelp(szBuffer, gDLL->getText("TXT_KEY_RELIGION_HOLY_CITY").c_str(), L": ", L"", religion.getHolyCityCommerceArray());
	setCommerceChangeHelp(szBuffer, gDLL->getText("TXT_KEY_RELIGION_ALL_CITIES").c_str(), L": ", L"", religion.getStateReligionCommerceArray());

	if (!bCivilopedia)
	{
		if (religion.getTechPrereq() != NO_TECH)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FOUNDED_FIRST", GC.getTechInfo((TechTypes)religion.getTechPrereq()).getTextKeyWide()));
		}
	}

	if (religion.getFreeUnitClass() != NO_UNITCLASS)
	{
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			eFreeUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType()).getCivilizationUnits(religion.getFreeUnitClass())));
		}
		else
		{
			eFreeUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)religion.getFreeUnitClass()).getDefaultUnitIndex();
		}

		if (eFreeUnit != NO_UNIT)
		{
			if (religion.getNumFreeUnits() > 1)
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FOUNDER_RECEIVES_NUM", GC.getUnitInfo(eFreeUnit).getTextKeyWide(), religion.getNumFreeUnits()));
				szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FOUNDER_RECEIVES_NUM", getLinkedText(eFreeUnit).c_str(), religion.getNumFreeUnits()));
				//<<<<	BUGFfH: End Modify
			}
			else if (religion.getNumFreeUnits() > 0)
			{
				szBuffer.append(NEWLINE);
				//>>>>	BUGFfH: Modified by Denev 2009/10/03
				//				szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FOUNDER_RECEIVES", GC.getUnitInfo(eFreeUnit).getTextKeyWide()));
				szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FOUNDER_RECEIVES", getLinkedText(eFreeUnit).c_str()));
				//<<<<	BUGFfH: End Modify
			}
		}
	}

	//>>>>	BUGFfH: Added by Denev 2009/10/03
	int iAlignmentBest = 0;
	int iAlignmentWorst = NUM_ALIGNMENT_TYPES - 1;

	if (religion.getAlignmentBest() != NO_ALIGNMENT)
	{
		if (religion.getAlignmentBest() == iAlignmentWorst)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_ALIGNMENT_CONVERT_ANY", GC.getAlignmentInfo((AlignmentTypes)iAlignmentWorst).getTextKeyWide()));
		}
		else
		{
			for (int iAlignment = iAlignmentBest; iAlignment < religion.getAlignmentBest(); iAlignment++)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_ALIGNMENT_CONVERT",
					GC.getAlignmentInfo((AlignmentTypes)iAlignment).getTextKeyWide(),
					GC.getAlignmentInfo((AlignmentTypes)religion.getAlignmentBest()).getTextKeyWide()));
			}
		}
	}

	if (religion.getAlignmentWorst() != NO_ALIGNMENT)
	{
		if (religion.getAlignmentWorst() == iAlignmentBest)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_ALIGNMENT_CONVERT_ANY", GC.getAlignmentInfo((AlignmentTypes)iAlignmentBest).getTextKeyWide()));
		}
		else
		{
			for (int iAlignment = iAlignmentWorst; iAlignment > religion.getAlignmentWorst(); iAlignment--)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_ALIGNMENT_CONVERT",
					GC.getAlignmentInfo((AlignmentTypes)iAlignment).getTextKeyWide(),
					GC.getAlignmentInfo((AlignmentTypes)religion.getAlignmentWorst()).getTextKeyWide()));
			}
		}
	}

	if (religion.getGlobalCounterFound() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_MODIFY_GLOBAL_COUNTER_ON_FOUND",
			religion.getGlobalCounterFound(),
			GC.getReligionInfo(eReligion).getChar()));
	}
	if (religion.getGlobalCounterSpread() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_MODIFY_GLOBAL_COUNTER_ON_SPREAD",
			religion.getGlobalCounterSpread(),
			GC.getReligionInfo(eReligion).getChar()));
	}

	if (religion.isHidden())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_HIDE_STATE_RELIGION"));
	}

	if (religion.isSneakAttack())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_SNEAK_ATTACK"));
	}

	if (religion.isUpdateSight())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_UPDATE_SIGHT",
			GC.getReligionInfo(eReligion).getHolyCityChar(),
			GC.getReligionInfo(eReligion).getChar()));
	}
	//<<<<	BUGFfH: End Add
}

void CvGameTextMgr::setReligionHelpCity(CvWStringBuffer& szBuffer, ReligionTypes eReligion, CvCity* pCity, bool bCityScreen, bool bForceReligion, bool bForceState, bool bNoStateReligion)
{
	int i;
	CvWString szTempBuffer;
	bool bHandled = false;
	int iCommerce;
	int iHappiness;
	int iProductionModifier;
	int iFreeExperience;
	int iGreatPeopleRateModifier;

	if (pCity == NULL)
	{
		return;
	}

	ReligionTypes eStateReligion = (bNoStateReligion ? NO_RELIGION : GET_PLAYER(pCity->getOwnerINLINE()).getStateReligion());

	if (bCityScreen)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szBuffer.append(CvWString::format(SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getReligionInfo(eReligion).getDescription()));
		szBuffer.append(CvWString::format(L"%s", getLinkedText(eReligion).c_str()));
		//<<<<	BUGFfH: End Modify
		szBuffer.append(NEWLINE);

		if (!(GC.getGameINLINE().isReligionFounded(eReligion)) && !GC.getGameINLINE().isOption(GAMEOPTION_PICK_RELIGION))
		{
			if (GC.getReligionInfo(eReligion).getTechPrereq() != NO_TECH)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FOUNDED_FIRST", GC.getTechInfo((TechTypes)(GC.getReligionInfo(eReligion).getTechPrereq())).getTextKeyWide()));
			}
		}
	}

	if (!bForceReligion)
	{
		if (!(pCity->isHasReligion(eReligion)))
		{
			return;
		}
	}

	if (eStateReligion == eReligion || eStateReligion == NO_RELIGION || bForceState)
	{
		for (i = 0; i < NUM_COMMERCE_TYPES; i++)
		{
			iCommerce = GC.getReligionInfo(eReligion).getStateReligionCommerce((CommerceTypes)i);

			if (pCity->isHolyCity(eReligion))
			{
				iCommerce += GC.getReligionInfo(eReligion).getHolyCityCommerce((CommerceTypes)i);
			}

			if (iCommerce != 0)
			{
				if (bHandled)
				{
					szBuffer.append(L", ");
				}

				szTempBuffer.Format(L"%s%d%c", iCommerce > 0 ? "+" : "", iCommerce, GC.getCommerceInfo((CommerceTypes)i).getChar());
				szBuffer.append(szTempBuffer);
				bHandled = true;
			}
		}
	}

	if (eStateReligion == eReligion || bForceState)
	{
		iHappiness = (pCity->getStateReligionHappiness(eReligion) + GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionHappiness());

		if (iHappiness != 0)
		{
			if (bHandled)
			{
				szBuffer.append(L", ");
			}

			szTempBuffer.Format(L"%d%c", iHappiness, ((iHappiness > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)));
			szBuffer.append(szTempBuffer);
			bHandled = true;
		}

		iProductionModifier = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionBuildingProductionModifier();
		if (iProductionModifier != 0)
		{
			if (bHandled)
			{
				szBuffer.append(L", ");
			}

			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_BUILDING_PROD_MOD", iProductionModifier));
			bHandled = true;
		}

		iProductionModifier = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionUnitProductionModifier();
		if (iProductionModifier != 0)
		{
			if (bHandled)
			{
				szBuffer.append(L", ");
			}

			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_UNIT_PROD_MOD", iProductionModifier));
			bHandled = true;
		}

		iFreeExperience = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionFreeExperience();
		if (iFreeExperience != 0)
		{
			if (bHandled)
			{
				szBuffer.append(L", ");
			}

			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FREE_XP", iFreeExperience));
			bHandled = true;
		}

		iGreatPeopleRateModifier = GET_PLAYER(pCity->getOwnerINLINE()).getStateReligionGreatPeopleRateModifier();
		if (iGreatPeopleRateModifier != 0)
		{
			if (bHandled)
			{
				szBuffer.append(L", ");
			}

			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_BIRTH_RATE_MOD", iGreatPeopleRateModifier));
			bHandled = true;
		}
	}
}

void CvGameTextMgr::setCorporationHelp(CvWStringBuffer& szBuffer, CorporationTypes eCorporation, bool bCivilopedia)
{
	UnitTypes eFreeUnit;
	CvWString szTempBuffer;

	if (NO_CORPORATION == eCorporation)
	{
		return;
	}
	CvCorporationInfo& kCorporation = GC.getCorporationInfo(eCorporation);

	if (!bCivilopedia)
	{
		szBuffer.append(CvWString::format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), kCorporation.getDescription()));
	}

	szTempBuffer.clear();
	for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		int iYieldProduced = GC.getCorporationInfo(eCorporation).getYieldProduced((YieldTypes)iI);
		if (NO_PLAYER != GC.getGameINLINE().getActivePlayer())
		{
			iYieldProduced *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getCorporationMaintenancePercent();
			iYieldProduced /= 100;
		}

		if (iYieldProduced != 0)
		{
			if (!szTempBuffer.empty())
			{
				szTempBuffer += L", ";
			}

			if (iYieldProduced % 100 == 0)
			{
				szTempBuffer += CvWString::format(L"%s%d%c",
					iYieldProduced > 0 ? L"+" : L"",
					iYieldProduced / 100,
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
			else
			{
				szTempBuffer += CvWString::format(L"%s%.2f%c",
					iYieldProduced > 0 ? L"+" : L"",
					0.01f * abs(iYieldProduced),
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
		}
	}

	if (!szTempBuffer.empty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_ALL_CITIES", szTempBuffer.GetCString()));
	}

	szTempBuffer.clear();
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		int iCommerceProduced = GC.getCorporationInfo(eCorporation).getCommerceProduced((CommerceTypes)iI);
		if (NO_PLAYER != GC.getGameINLINE().getActivePlayer())
		{
			iCommerceProduced *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getCorporationMaintenancePercent();
			iCommerceProduced /= 100;
		}
		if (iCommerceProduced != 0)
		{
			if (!szTempBuffer.empty())
			{
				szTempBuffer += L", ";
			}

			if (iCommerceProduced % 100 == 0)
			{
				szTempBuffer += CvWString::format(L"%s%d%c",
					iCommerceProduced > 0 ? L"+" : L"",
					iCommerceProduced / 100,
					GC.getCommerceInfo((CommerceTypes)iI).getChar());
			}
			else
			{
				szTempBuffer += CvWString::format(L"%s%.2f%c",
					iCommerceProduced > 0 ? L"+" : L"",
					0.01f * abs(iCommerceProduced),
					GC.getCommerceInfo((CommerceTypes)iI).getChar());
			}

		}
	}

	if (!szTempBuffer.empty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_ALL_CITIES", szTempBuffer.GetCString()));
	}

	if (!bCivilopedia)
	{
		if (kCorporation.getTechPrereq() != NO_TECH)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_FOUNDED_FIRST", GC.getTechInfo((TechTypes)kCorporation.getTechPrereq()).getTextKeyWide()));
		}
	}

	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_BONUS_REQUIRED"));
	bool bFirst = true;
	for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
	{
		if (NO_BONUS != kCorporation.getPrereqBonus(i))
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				szBuffer.append(L", ");
			}

			szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)kCorporation.getPrereqBonus(i)).getChar()));
		}
	}

	if (kCorporation.getBonusProduced() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_BONUS_PRODUCED", GC.getBonusInfo((BonusTypes)kCorporation.getBonusProduced()).getChar()));
	}

	if (kCorporation.getFreeUnitClass() != NO_UNITCLASS)
	{
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
		{
			eFreeUnit = ((UnitTypes)(GC.getCivilizationInfo(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType()).getCivilizationUnits(kCorporation.getFreeUnitClass())));
		}
		else
		{
			eFreeUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)kCorporation.getFreeUnitClass()).getDefaultUnitIndex();
		}

		if (eFreeUnit != NO_UNIT)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_RELIGION_FOUNDER_RECEIVES", GC.getUnitInfo(eFreeUnit).getTextKeyWide()));
		}
	}

	std::vector<CorporationTypes> aCompetingCorps;
	bFirst = true;
	for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); ++iCorporation)
	{
		if (iCorporation != eCorporation)
		{
			bool bCompeting = false;

			CvCorporationInfo& kLoopCorporation = GC.getCorporationInfo((CorporationTypes)iCorporation);
			for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
			{
				if (kCorporation.getPrereqBonus(i) != NO_BONUS)
				{
					for (int j = 0; j < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++j)
					{
						if (kLoopCorporation.getPrereqBonus(j) == kCorporation.getPrereqBonus(i))
						{
							bCompeting = true;
							break;
						}
					}
				}

				if (bCompeting)
				{
					break;
				}
			}

			if (bCompeting)
			{
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				CvWString szTemp = CvWString::format(L"<link=literal>%s</link>", kLoopCorporation.getDescription());
				CvWString szTemp = getLinkedText((CorporationTypes)iCorporation);
				//<<<<	BUGFfH: End Modify
				setListHelp(szBuffer, gDLL->getText("TXT_KEY_CORPORATION_COMPETES").c_str(), szTemp.GetCString(), L", ", bFirst);
				bFirst = false;
			}
		}
	}
}

void CvGameTextMgr::setCorporationHelpCity(CvWStringBuffer& szBuffer, CorporationTypes eCorporation, CvCity* pCity, bool bCityScreen, bool bForceCorporation)
{
	if (pCity == NULL)
	{
		return;
	}

	CvCorporationInfo& kCorporation = GC.getCorporationInfo(eCorporation);

	if (bCityScreen)
	{
		szBuffer.append(CvWString::format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), kCorporation.getDescription()));
		szBuffer.append(NEWLINE);

		if (!(GC.getGameINLINE().isCorporationFounded(eCorporation)))
		{
			if (GC.getCorporationInfo(eCorporation).getTechPrereq() != NO_TECH)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_FOUNDED_FIRST", GC.getTechInfo((TechTypes)(kCorporation.getTechPrereq())).getTextKeyWide()));
			}
		}
	}

	if (!bForceCorporation)
	{
		if (!(pCity->isHasCorporation(eCorporation)))
		{
			return;
		}
	}

	int iNumResources = 0;
	for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
	{
		BonusTypes eBonus = (BonusTypes)kCorporation.getPrereqBonus(i);
		if (NO_BONUS != eBonus)
		{
			iNumResources += pCity->getNumBonuses(eBonus);
		}
	}

	bool bActive = (pCity->isActiveCorporation(eCorporation) || (bForceCorporation && iNumResources > 0));

	bool bHandled = false;
	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		int iYield = 0;

		if (bActive)
		{
			iYield += (kCorporation.getYieldProduced(i) * iNumResources * GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getCorporationMaintenancePercent()) / 100;
		}

		if (iYield != 0)
		{
			if (bHandled)
			{
				szBuffer.append(L", ");
			}

			CvWString szTempBuffer;
			szTempBuffer.Format(L"%s%d%c", iYield > 0 ? "+" : "", (iYield + 99) / 100, GC.getYieldInfo((YieldTypes)i).getChar());
			szBuffer.append(szTempBuffer);
			bHandled = true;
		}
	}

	bHandled = false;
	for (int i = 0; i < NUM_COMMERCE_TYPES; ++i)
	{
		int iCommerce = 0;

		if (bActive)
		{
			iCommerce += (kCorporation.getCommerceProduced(i) * iNumResources * GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getCorporationMaintenancePercent()) / 100;
		}

		if (iCommerce != 0)
		{
			if (bHandled)
			{
				szBuffer.append(L", ");
			}

			CvWString szTempBuffer;
			szTempBuffer.Format(L"%s%d%c", iCommerce > 0 ? "+" : "", (iCommerce + 99) / 100, GC.getCommerceInfo((CommerceTypes)i).getChar());
			szBuffer.append(szTempBuffer);
			bHandled = true;
		}
	}

	int iMaintenance = 0;

	if (bActive)
	{
		iMaintenance += pCity->calculateCorporationMaintenanceTimes100(eCorporation);
		iMaintenance *= 100 + pCity->getMaintenanceModifier();
		iMaintenance /= 100;
	}

	if (0 != iMaintenance)
	{
		if (bHandled)
		{
			szBuffer.append(L", ");
		}

		CvWString szTempBuffer;
		szTempBuffer.Format(L"%d%c", -iMaintenance / 100, GC.getCommerceInfo(COMMERCE_GOLD).getChar());
		szBuffer.append(szTempBuffer);
		bHandled = true;
	}

	if (bCityScreen)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_BONUS_REQUIRED"));
		bool bFirst = true;
		for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
		{
			if (NO_BONUS != kCorporation.getPrereqBonus(i))
			{
				if (bFirst)
				{
					bFirst = false;
				}
				else
				{
					szBuffer.append(L", ");
				}

				szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)kCorporation.getPrereqBonus(i)).getChar()));
			}
		}

		if (bActive)
		{
			if (kCorporation.getBonusProduced() != NO_BONUS)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_CORPORATION_BONUS_PRODUCED", GC.getBonusInfo((BonusTypes)kCorporation.getBonusProduced()).getChar()));
			}
		}
	}
	else
	{
		if (kCorporation.getBonusProduced() != NO_BONUS)
		{
			if (bActive)
			{
				if (bHandled)
				{
					szBuffer.append(L", ");
				}

				szBuffer.append(CvWString::format(L"%c", GC.getBonusInfo((BonusTypes)kCorporation.getBonusProduced()).getChar()));
			}
		}
	}
}

void CvGameTextMgr::buildObsoleteString(CvWStringBuffer& szBuffer, int iItem, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (bList)
	{
		szBuffer.append(NEWLINE);
	}
	szBuffer.append(gDLL->getText("TXT_KEY_TECH_OBSOLETES", GC.getBuildingInfo((BuildingTypes)iItem).getTextKeyWide()));
}

void CvGameTextMgr::buildObsoleteBonusString(CvWStringBuffer& szBuffer, int iItem, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (bList)
	{
		szBuffer.append(NEWLINE);
	}
	szBuffer.append(gDLL->getText("TXT_KEY_TECH_OBSOLETES", GC.getBonusInfo((BonusTypes)iItem).getTextKeyWide()));
}

void CvGameTextMgr::buildObsoleteSpecialString(CvWStringBuffer& szBuffer, int iItem, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (bList)
	{
		szBuffer.append(NEWLINE);
	}
	szBuffer.append(gDLL->getText("TXT_KEY_TECH_OBSOLETES_NO_LINK", GC.getSpecialBuildingInfo((SpecialBuildingTypes)iItem).getTextKeyWide()));
}

void CvGameTextMgr::buildMoveString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	int iI;
	int iMoveDiff;

	for (iI = 0; iI < GC.getNumRouteInfos(); ++iI)
	{
		iMoveDiff = ((GC.getMOVE_DENOMINATOR() / std::max(1, (GC.getRouteInfo((RouteTypes)iI).getMovementCost() + ((bPlayerContext) ? GET_TEAM(GC.getGameINLINE().getActiveTeam()).getRouteChange((RouteTypes)iI) : 0)))) - (GC.getMOVE_DENOMINATOR() / std::max(1, (GC.getRouteInfo((RouteTypes)iI).getMovementCost() + ((bPlayerContext) ? GET_TEAM(GC.getGameINLINE().getActiveTeam()).getRouteChange((RouteTypes)iI) : 0) + GC.getRouteInfo((RouteTypes)iI).getTechMovementChange(eTech)))));

		if (iMoveDiff != 0)
		{
			if (bList)
			{
				szBuffer.append(NEWLINE);
			}
			szBuffer.append(gDLL->getText("TXT_KEY_UNIT_MOVEMENT", -(iMoveDiff), GC.getRouteInfo((RouteTypes)iI).getTextKeyWide()));
			bList = true;
		}
	}
}

void CvGameTextMgr::buildFreeUnitString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	UnitTypes eFreeUnit = NO_UNIT;
	if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
	{
		eFreeUnit = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getTechFreeUnit(eTech);
	}
	else
	{
		if (GC.getTechInfo(eTech).getFirstFreeUnitClass() != NO_UNITCLASS)
		{
			eFreeUnit = (UnitTypes)GC.getUnitClassInfo((UnitClassTypes)GC.getTechInfo(eTech).getFirstFreeUnitClass()).getDefaultUnitIndex();
		}
	}

	if (eFreeUnit != NO_UNIT)
	{
		if (!bPlayerContext || (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0))
		{
			if (bList)
			{
				szBuffer.append(NEWLINE);
			}
			szBuffer.append(gDLL->getText("TXT_KEY_TECH_FIRST_RECEIVES", GC.getUnitInfo(eFreeUnit).getTextKeyWide()));
		}
	}
}

void CvGameTextMgr::buildFeatureProductionString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getFeatureProductionModifier() != 0)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_PRODUCTION_MODIFIER", GC.getTechInfo(eTech).getFeatureProductionModifier()));
	}
}

void CvGameTextMgr::buildWorkerRateString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getWorkerSpeedModifier() != 0)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_WORKERS_FASTER", GC.getTechInfo(eTech).getWorkerSpeedModifier()));
	}
}

void CvGameTextMgr::buildTradeRouteString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getTradeRoutes() != 0)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_TRADE_ROUTES", GC.getTechInfo(eTech).getTradeRoutes()));
	}
}

void CvGameTextMgr::buildHealthRateString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getHealth() != 0)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HEALTH_ALL_CITIES", abs(GC.getTechInfo(eTech).getHealth()), ((GC.getTechInfo(eTech).getHealth() > 0) ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR))));
	}
}

void CvGameTextMgr::buildHappinessRateString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getHappiness() != 0)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HAPPINESS_ALL_CITIES", abs(GC.getTechInfo(eTech).getHappiness()), ((GC.getTechInfo(eTech).getHappiness() > 0) ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
	}
}

void CvGameTextMgr::buildFreeTechString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getFirstFreeTechs() > 0)
	{
		if (!bPlayerContext || (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0))
		{
			if (bList)
			{
				szBuffer.append(NEWLINE);
			}

			if (GC.getTechInfo(eTech).getFirstFreeTechs() == 1)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_TECH_FIRST_FREE_TECH"));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_TECH_FIRST_FREE_TECHS", GC.getTechInfo(eTech).getFirstFreeTechs()));
			}
		}
	}
}

void CvGameTextMgr::buildLOSString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isExtraWaterSeeFrom() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isExtraWaterSeeFrom())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_UNIT_EXTRA_SIGHT"));
	}
}

void CvGameTextMgr::buildMapCenterString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isMapCentering() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isMapCentering())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_CENTERS_MAP"));
	}
}

void CvGameTextMgr::buildMapRevealString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList)
{
	if (GC.getTechInfo(eTech).isMapVisible())
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_REVEALS_MAP"));
	}
}

void CvGameTextMgr::buildMapTradeString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isMapTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isMapTrading())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_MAP_TRADING"));
	}
}

void CvGameTextMgr::buildTechTradeString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isTechTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isTechTrading())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_TECH_TRADING"));
	}
}

void CvGameTextMgr::buildGoldTradeString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isGoldTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isGoldTrading())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_GOLD_TRADING"));
	}
}

void CvGameTextMgr::buildOpenBordersString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isOpenBordersTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isOpenBordersTrading())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_OPEN_BORDERS"));
	}
}

void CvGameTextMgr::buildDefensivePactString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isDefensivePactTrading() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isDefensivePactTrading())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_DEFENSIVE_PACTS"));
	}
}

void CvGameTextMgr::buildPermanentAllianceString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isPermanentAllianceTrading() && (!bPlayerContext || (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isPermanentAllianceTrading()) && GC.getGameINLINE().isOption(GAMEOPTION_PERMANENT_ALLIANCES))))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_PERM_ALLIANCES"));
	}
}

void CvGameTextMgr::buildVassalStateString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isVassalStateTrading() && (!bPlayerContext || (!(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isVassalStateTrading()) && GC.getGameINLINE().isOption(GAMEOPTION_NO_VASSAL_STATES))))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_VASSAL_STATES"));
	}
}

void CvGameTextMgr::buildBridgeString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isBridgeBuilding() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isBridgeBuilding())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_BRIDGE_BUILDING"));
	}
}

void CvGameTextMgr::buildIrrigationString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isIrrigation() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isIrrigation())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPREAD_IRRIGATION"));
	}
}

void CvGameTextMgr::buildIgnoreIrrigationString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isIgnoreIrrigation() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isIgnoreIrrigation())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_IRRIGATION_ANYWHERE"));
	}
}

void CvGameTextMgr::buildWaterWorkString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isWaterWork() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isWaterWork())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_WATER_WORK"));
	}
}

void CvGameTextMgr::buildImprovementString(CvWStringBuffer& szBuffer, TechTypes eTech, int iImprovement, bool bList, bool bPlayerContext)
{
	bool bTechFound;
	int iJ;

	bTechFound = false;

	if (GC.getBuildInfo((BuildTypes)iImprovement).getTechPrereq() == NO_TECH)
	{
		/** Improvements that have no Prereq
		/** Sephi	**/
		if (GC.getBuildInfo((BuildTypes)iImprovement).getImprovement() == NO_IMPROVEMENT)
		{
			/**	 end **/
			for (iJ = 0; iJ < GC.getNumFeatureInfos(); iJ++)
			{
				if (GC.getBuildInfo((BuildTypes)iImprovement).getFeatureTech(iJ) == eTech)
				{
					bTechFound = true;
				}
			}
		}
	}
	else
	{
		if (GC.getBuildInfo((BuildTypes)iImprovement).getTechPrereq() == eTech)
		{
			bTechFound = true;
		}
	}

	/** Civ prepreq can block Improvements **/
	if (bPlayerContext) {
		ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo((BuildTypes)iImprovement).getImprovement();
		if (eImprovement != NO_IMPROVEMENT && GC.getGameINLINE().getActivePlayer() != NO_PLAYER) {
			CvPlayer& kPlayer = GET_PLAYER(GC.getGame().getActivePlayer());
			if (!GC.getCivilizationInfo(kPlayer.getCivilizationType()).canBuildImprovement(eImprovement)) {
				bTechFound = false;
			}
		}
	}

	/** Civ prepreq can block Improvements **/

	if (bTechFound)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}

		szBuffer.append(gDLL->getText("TXT_KEY_MISC_CAN_BUILD_IMPROVEMENT", GC.getBuildInfo((BuildTypes)iImprovement).getTextKeyWide()));
	}
}

void CvGameTextMgr::buildDomainExtraMovesString(CvWStringBuffer& szBuffer, TechTypes eTech, int iDomainType, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).getDomainExtraMoves(iDomainType) != 0)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}

		szBuffer.append(gDLL->getText("TXT_KEY_MISC_EXTRA_MOVES", GC.getTechInfo(eTech).getDomainExtraMoves(iDomainType), GC.getDomainInfo((DomainTypes)iDomainType).getTextKeyWide()));
	}
}

void CvGameTextMgr::buildAdjustString(CvWStringBuffer& szBuffer, TechTypes eTech, int iCommerceType, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isCommerceFlexible(iCommerceType) && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isCommerceFlexible((CommerceTypes)iCommerceType))))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ADJUST_COMMERCE_RATE", GC.getCommerceInfo((CommerceTypes)iCommerceType).getChar()));
	}
}

void CvGameTextMgr::buildTerrainTradeString(CvWStringBuffer& szBuffer, TechTypes eTech, int iTerrainType, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isTerrainTrade(iTerrainType) && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isTerrainTrade((TerrainTypes)iTerrainType))))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_ON_TERRAIN", gDLL->getSymbolID(TRADE_CHAR), GC.getTerrainInfo((TerrainTypes)iTerrainType).getTextKeyWide()));
	}
}

void CvGameTextMgr::buildRiverTradeString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bList, bool bPlayerContext)
{
	if (GC.getTechInfo(eTech).isRiverTrade() && (!bPlayerContext || !(GET_TEAM(GC.getGameINLINE().getActiveTeam()).isRiverTrade())))
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ENABLES_ON_TERRAIN", gDLL->getSymbolID(TRADE_CHAR), gDLL->getText("TXT_KEY_MISC_RIVERS").GetCString()));
	}
}

void CvGameTextMgr::buildSpecialBuildingString(CvWStringBuffer& szBuffer, TechTypes eTech, int iBuildingType, bool bList, bool bPlayerContext)
{
	if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iBuildingType).getTechPrereq() == eTech)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_CAN_CONSTRUCT_BUILDING", GC.getSpecialBuildingInfo((SpecialBuildingTypes)iBuildingType).getTextKeyWide()));
	}

	if (GC.getSpecialBuildingInfo((SpecialBuildingTypes)iBuildingType).getTechPrereqAnyone() == eTech)
	{
		if (bList)
		{
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_CAN_CONSTRUCT_BUILDING_ANYONE", GC.getSpecialBuildingInfo((SpecialBuildingTypes)iBuildingType).getTextKeyWide()));
	}
}

void CvGameTextMgr::buildYieldChangeString(CvWStringBuffer& szBuffer, TechTypes eTech, int iYieldType, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;
	if (bList)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/09/13
		//		szTempBuffer.Format(L"<link=literal>%s</link>", GC.getImprovementInfo((ImprovementTypes)iYieldType).getDescription());
		szTempBuffer = getLinkedText((ImprovementTypes)iYieldType);
		//<<<<	BUGFfH: End Modify
	}
	else
	{
		//>>>>	BUGFfH: Modified by Denev 2009/09/13
		//		szTempBuffer.Format(L"%c<link=literal>%s</link>", gDLL->getSymbolID(BULLET_CHAR), GC.getImprovementInfo((ImprovementTypes)iYieldType).getDescription());
		szTempBuffer.Format(L"%c%s", gDLL->getSymbolID(BULLET_CHAR), getLinkedText((ImprovementTypes)iYieldType).c_str());
		//<<<<	BUGFfH: End Modify
	}

	setYieldChangeHelp(szBuffer, szTempBuffer, L": ", L"", GC.getImprovementInfo((ImprovementTypes)iYieldType).getTechYieldChangesArray(eTech), false, bList);
}

bool CvGameTextMgr::buildBonusRevealString(CvWStringBuffer& szBuffer, TechTypes eTech, int iBonusType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getBonusInfo((BonusTypes)iBonusType).getTechReveal() == eTech)
	{
		if (bList && bFirst)
		{
			szBuffer.append(NEWLINE);
		}
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getBonusInfo((BonusTypes) iBonusType).getDescription());
		szTempBuffer.assign(getLinkedText((BonusTypes)iBonusType));
		//<<<<	BUGFfH: End Modify
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_REVEALS").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}

//>>>>	BUGFfH: Added by Denev 2009/09/23
bool CvGameTextMgr::buildBonusEnableString(CvWStringBuffer& szBuffer, TechTypes eTech, int iBonusType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getBonusInfo((BonusTypes)iBonusType).getTechCityTrade() == eTech)
	{
		if (bList && bFirst)
		{
			szBuffer.append(NEWLINE);
		}
		szTempBuffer.assign(getLinkedText((BonusTypes)iBonusType));
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_ENABLES").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}
//<<<<	BUGFfH: End Add

bool CvGameTextMgr::buildCivicRevealString(CvWStringBuffer& szBuffer, TechTypes eTech, int iCivicType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getCivicInfo((CivicTypes)iCivicType).getTechPrereq() == eTech)
	{

		//FfH: Modified by Kael 07/30/2009
		//		if (bList && bFirst)
		//		{
		//			szBuffer.append(NEWLINE);
		//		}
		//		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivicInfo((CivicTypes) iCivicType).getDescription());
		//		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_ENABLES").c_str(), szTempBuffer, L", ", bFirst);
		//		bFirst = false;
		if (!bPlayerContext ||
			GC.getCivicInfo((CivicTypes)iCivicType).getPrereqCivilization() == NO_CIVILIZATION ||
			GC.getCivicInfo((CivicTypes)iCivicType).getPrereqCivilization() == GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getCivilizationType())
		{
			if (bList && bFirst)
			{
				szBuffer.append(NEWLINE);
			}
			//>>>>	BUGFfH: Modified by Denev 2009/10/02
			//			szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivicInfo((CivicTypes) iCivicType).getDescription());
			szTempBuffer.assign(getLinkedText((CivicTypes)iCivicType));
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_ENABLES").c_str(), szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
		//FfH: End Modify

	}
	return bFirst;
}

bool CvGameTextMgr::buildProcessInfoString(CvWStringBuffer& szBuffer, TechTypes eTech, int iProcessType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getProcessInfo((ProcessTypes)iProcessType).getTechPrereq() == eTech)
	{
		if (bList && bFirst)
		{
			szBuffer.append(NEWLINE);
		}
		//>>>>	BUGFfH: Modified by Denev 2009/09/13
		//		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getProcessInfo((ProcessTypes) iProcessType).getDescription());
		szTempBuffer.assign(getLinkedText((ProcessTypes)iProcessType));
		//<<<<	BUGFfH: End Modify
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_CAN_BUILD").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}

bool CvGameTextMgr::buildFoundReligionString(CvWStringBuffer& szBuffer, TechTypes eTech, int iReligionType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getReligionInfo((ReligionTypes)iReligionType).getTechPrereq() == eTech)
	{
		if (!bPlayerContext || (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0))
		{
			if (bList && bFirst)
			{
				szBuffer.append(NEWLINE);
			}

			if (GC.getGameINLINE().isOption(GAMEOPTION_PICK_RELIGION))
			{
				szTempBuffer = gDLL->getText("TXT_KEY_RELIGION_UNKNOWN");
			}
			else
			{
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getReligionInfo((ReligionTypes) iReligionType).getDescription());
				szTempBuffer.assign(getLinkedText((ReligionTypes)iReligionType));
				//<<<<	BUGFfH: End Modify
			}
			setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_FIRST_DISCOVER_FOUNDS").c_str(), szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}
	return bFirst;
}

bool CvGameTextMgr::buildFoundCorporationString(CvWStringBuffer& szBuffer, TechTypes eTech, int iCorporationType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	if (GC.getCorporationInfo((CorporationTypes)iCorporationType).getTechPrereq() == eTech)
	{
		if (!bPlayerContext || (GC.getGameINLINE().countKnownTechNumTeams(eTech) == 0))
		{
			if (bList && bFirst)
			{
				szBuffer.append(NEWLINE);
			}
			//>>>>	BUGFfH: Modified by Denev 2009/09/13
			//			szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCorporationInfo((CorporationTypes) iCorporationType).getDescription());
			szTempBuffer.assign(getLinkedText((CorporationTypes)iCorporationType));
			//<<<<	BUGFfH: End Modify
			setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_FIRST_DISCOVER_INCORPORATES").c_str(), szTempBuffer, L", ", bFirst);
			bFirst = false;
		}
	}
	return bFirst;
}

bool CvGameTextMgr::buildPromotionString(CvWStringBuffer& szBuffer, TechTypes eTech, int iPromotionType, bool bFirst, bool bList, bool bPlayerContext)
{
	CvWString szTempBuffer;

	/*************************************************************************************************/
	/**	ADDON (FFHBUG) Sephi    																	**/
	/**	Techtree doesn't show features only available to other civs									**/
	/*************************************************************************************************/
	int iCiv = GC.getGameINLINE().getActiveCivilizationType();
	int iNumPrereqs = GC.getPromotionInfo((PromotionTypes)iPromotionType).getNumPrereqCivilizations();

	if (iNumPrereqs > 0)
	{
		bool bValid = false;

		for (int iI = 0; iI < iNumPrereqs; ++iI)
		{
			if (GC.getPromotionInfo((PromotionTypes)iPromotionType).getPrereqCivilization(iI) == iCiv)
			{
				bValid = true;
			}
		}

		if (!bValid)
			return bFirst;
	}
	/*************************************************************************************************/
	/**	END																							**/
	/*************************************************************************************************/

	if (GC.getPromotionInfo((PromotionTypes)iPromotionType).getTechPrereq() == eTech)
	{
		if (bList && bFirst)
		{
			szBuffer.append(NEWLINE);
		}
		//>>>>	BUGFfH: Modified by Denev 2009/09/13
		//		szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getPromotionInfo((PromotionTypes) iPromotionType).getDescription());
		szTempBuffer.assign(getLinkedText((PromotionTypes)iPromotionType));
		//<<<<	BUGFfH: End Modify
		setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_ENABLES").c_str(), szTempBuffer, L", ", bFirst);
		bFirst = false;
	}
	return bFirst;
}

// Displays a list of derived technologies - no distinction between AND/OR prerequisites
void CvGameTextMgr::buildSingleLineTechTreeString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bPlayerContext)
{
	CvWString szTempBuffer;	// Formatting

	if (NO_TECH == eTech)
	{
		// you need to specify a tech of origin for this method to do anything
		return;
	}

	bool bFirst = true;
	for (int iI = 0; iI < GC.getNumTechInfos(); ++iI)
	{
		bool bTechAlreadyAccessible = false;
		if (bPlayerContext)
		{
			bTechAlreadyAccessible = (GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech((TechTypes)iI) || GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canResearch((TechTypes)iI));
		}
		if (!bTechAlreadyAccessible)
		{
			bool bTechFound = false;

			if (!bTechFound)
			{
				for (int iJ = 0; iJ < GC.getNUM_OR_TECH_PREREQS(); iJ++)
				{
					if (GC.getTechInfo((TechTypes)iI).getPrereqOrTechs(iJ) == eTech)
					{
						bTechFound = true;
						break;
					}
				}
			}

			if (!bTechFound)
			{
				for (int iJ = 0; iJ < GC.getNUM_AND_TECH_PREREQS(); iJ++)
				{
					if (GC.getTechInfo((TechTypes)iI).getPrereqAndTechs(iJ) == eTech)
					{
						bTechFound = true;
						break;
					}
				}
			}

			if (bTechFound)
			{
				//>>>>	BUGFfH: Modified by Denev 2009/09/13
				//				szTempBuffer.Format( SETCOLR L"<link=literal>%s</link>" ENDCOLR , TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo((TechTypes) iI).getDescription());
				szTempBuffer.assign(getLinkedText((TechTypes)iI));
				//<<<<	BUGFfH: End Modify
				setListHelp(szBuffer, gDLL->getText("TXT_KEY_MISC_LEADS_TO").c_str(), szTempBuffer, L", ", bFirst);
				bFirst = false;
			}
		}
	}
}

// Information about other prerequisite technologies to eTech besides eFromTech
void CvGameTextMgr::buildTechTreeString(CvWStringBuffer& szBuffer, TechTypes eTech, bool bPlayerContext, TechTypes eFromTech)
{
	CvWString szTempBuffer;	// Formatting

	if (NO_TECH == eTech || NO_TECH == eFromTech)
	{
		return;
	}

	szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTech).getDescription());
	szBuffer.append(szTempBuffer);

	// Loop through OR prerequisites to make list
	CvWString szOtherOrTechs;
	int nOtherOrTechs = 0;
	bool bOrTechFound = false;
	for (int iJ = 0; iJ < GC.getNUM_OR_TECH_PREREQS(); iJ++)
	{
		TechTypes eTestTech = (TechTypes)GC.getTechInfo(eTech).getPrereqOrTechs(iJ);
		if (eTestTech >= 0)
		{
			bool bTechAlreadyResearched = false;
			if (bPlayerContext)
			{
				bTechAlreadyResearched = GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech(eTestTech);
			}
			if (!bTechAlreadyResearched)
			{
				if (eTestTech == eFromTech)
				{
					bOrTechFound = true;
				}
				else
				{
					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTestTech).getDescription());
					setListHelp(szOtherOrTechs, L"", szTempBuffer, gDLL->getText("TXT_KEY_OR").c_str(), 0 == nOtherOrTechs);
					nOtherOrTechs++;
				}
			}
		}
	}

	// Loop through AND prerequisites to make list
	CvWString szOtherAndTechs;
	int nOtherAndTechs = 0;
	bool bAndTechFound = false;
	for (int iJ = 0; iJ < GC.getNUM_AND_TECH_PREREQS(); iJ++)
	{
		TechTypes eTestTech = (TechTypes)GC.getTechInfo(eTech).getPrereqAndTechs(iJ);
		if (eTestTech >= 0)
		{
			bool bTechAlreadyResearched = false;
			if (bPlayerContext)
			{
				bTechAlreadyResearched = GET_TEAM(GC.getGameINLINE().getActiveTeam()).isHasTech(eTestTech);
			}
			if (!bTechAlreadyResearched)
			{
				if (eTestTech == eFromTech)
				{
					bAndTechFound = true;
				}
				else
				{
					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), GC.getTechInfo(eTestTech).getDescription());
					setListHelp(szOtherAndTechs, L"", szTempBuffer, L", ", 0 == nOtherAndTechs);
					nOtherAndTechs++;
				}
			}
		}
	}

	if (bOrTechFound || bAndTechFound)
	{
		if (nOtherAndTechs > 0 || nOtherOrTechs > 0)
		{
			szBuffer.append(L' ');

			if (nOtherAndTechs > 0)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_WITH_SPACE"));
				szBuffer.append(szOtherAndTechs);
			}

			if (nOtherOrTechs > 0)
			{
				if (bAndTechFound)
				{
					if (nOtherAndTechs > 0)
					{
						szBuffer.append(gDLL->getText("TXT_KEY_AND_SPACE"));
					}
					else
					{
						szBuffer.append(gDLL->getText("TXT_KEY_WITH_SPACE"));
					}
					szBuffer.append(szOtherOrTechs);
				}
				else if (bOrTechFound)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALTERNATIVELY_DERIVED", GC.getTechInfo(eTech).getTextKeyWide(), szOtherOrTechs.GetCString()));
				}
			}
		}
	}
}

void CvGameTextMgr::setPromotionHelp(CvWStringBuffer& szBuffer, PromotionTypes ePromotion, bool bCivilopediaText)
{
	if (!bCivilopediaText)
	{
		CvWString szTempBuffer;

		if (NO_PROMOTION == ePromotion)
		{
			return;
		}
		CvPromotionInfo& promo = GC.getPromotionInfo(ePromotion);

		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), promo.getDescription());
		szTempBuffer.assign(getLinkedText(ePromotion));
		//<<<<	BUGFfH: End Modify
				//added Sephi
		EquipmentCategoryTypes eCategory = (EquipmentCategoryTypes)promo.getEquipmentCategory();
		if (eCategory != NO_EQUIPMENTCATEGORY && promo.getEquipmentLevel() > 0)
		{
			szTempBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_EQUIPMENTCATEGORY", GC.getEquipmentCategoryInfo(eCategory).getDescription()));
		}
		//end added
		szBuffer.append(szTempBuffer);
	}

	parsePromotionHelp(szBuffer, ePromotion);

	CvPromotionInfo& kPromotion = GC.getPromotionInfo(ePromotion);

	if (bCivilopediaText)
	{
		if (kPromotion.isClassPromotion())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_CLASS_PROMOTION_HELP"));
		}

		if (kPromotion.getAdventurePrereq() != NO_ADVENTURE)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_PROMOTION_ADVENTURE_PREREQ"));
		}
	}
}

//FfH: Added by Kael 07/23/2007
void CvGameTextMgr::setSpellHelp(CvWStringBuffer& szBuffer, SpellTypes eSpell, bool bCivilopediaText)
{
	if (!bCivilopediaText)
	{
		CvWString szTempBuffer;

		if (NO_SPELL == eSpell)
		{
			return;
		}
		CvSpellInfo& spell = GC.getSpellInfo(eSpell);
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), spell.getDescription());
		szTempBuffer.assign(getLinkedText(eSpell));
		//<<<<	BUGFfH: End Modify
		szBuffer.append(szTempBuffer);
	}

	parseSpellHelp(szBuffer, eSpell);
}
//FfH: End Add

void CvGameTextMgr::setUnitCombatHelp(CvWStringBuffer& szBuffer, UnitCombatTypes eUnitCombat)
{
	//>>>>	BUGFfH: Modified by Denev 2009/10/03
	//	szBuffer.append(GC.getUnitCombatInfo(eUnitCombat).getDescription());
	szBuffer.append(getLinkedText(eUnitCombat));
	//<<<<	BUGFfH: End Modify
}

void CvGameTextMgr::setImprovementHelp(CvWStringBuffer& szBuffer, ImprovementTypes eImprovement, bool bCivilopediaText)
{
	CvWString szTempBuffer;
	CvWString szFirstBuffer;
	int iTurns;
	bool bFirst = false;

	if (NO_IMPROVEMENT == eImprovement)
	{
		return;
	}

	CvImprovementInfo& info = GC.getImprovementInfo(eImprovement);
	if (!bCivilopediaText)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/02
		//		szTempBuffer.Format( SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), info.getDescription());
		szTempBuffer.assign(getLinkedText(eImprovement));
		//<<<<	BUGFfH: End Modify
		szBuffer.append(szTempBuffer);

		//>>>>	BUGFfH: Modified by Denev 2009/09/21
		/*
				setYieldChangeHelp(szBuffer, L", ", L"", L"", info.getYieldChangeArray(), false, false);

				setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_WITH_IRRIGATION").c_str(), info.getIrrigatedYieldChangeArray());
				setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_ON_HILLS").c_str(), info.getHillsYieldChangeArray());
				setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_ALONG_RIVER").c_str(), info.getRiverSideYieldChangeArray());

				for (int iTech = 0; iTech < GC.getNumTechInfos(); iTech++)
				{
					for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
					{
						if (0 != info.getTechYieldChanges(iTech, iYield))
						{
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_WITH_TECH", info.getTechYieldChanges(iTech, iYield), GC.getYieldInfo((YieldTypes)iYield).getChar(), GC.getTechInfo((TechTypes)iTech).getTextKeyWide()));
						}
					}
				}

				//	Civics
				for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
				{
					for (int iCivic = 0; iCivic < GC.getNumCivicInfos(); iCivic++)
					{
						int iChange = GC.getCivicInfo((CivicTypes)iCivic).getImprovementYieldChanges(eImprovement, iYield);
						if (0 != iChange)
						{
							szTempBuffer.Format( SETCOLR L"%s" ENDCOLR , TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getCivicInfo((CivicTypes)iCivic).getDescription());
							szBuffer.append(NEWLINE);
							szBuffer.append(gDLL->getText("TXT_KEY_CIVIC_IMPROVEMENT_YIELD_CHANGE", iChange, GC.getYieldInfo((YieldTypes)iYield).getChar()));
							szBuffer.append(szTempBuffer);
						}
					}
				}
		*/
		setImprovementYieldHelp(szBuffer, eImprovement);
		//<<<<	BUGFfH: End Modify
	}

	//>>>>	BUGFfH: Moved from below(*12)(*13)(*14)(*15) by Denev 2009/09/21
	if (info.isUnique())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_UNIQUE"));
	}
	if (0 != info.getDefenseModifier())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_DEFENSE_MODIFIER", info.getDefenseModifier()));
	}
	if (info.getRangeDefenseModifier() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_RANGE_DEFENSE_MODIFIER", info.getRangeDefenseModifier(), info.getRange()));
	}
	if (info.getHealRateChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_HEAL_RATE_CHANGE", info.getHealRateChange()));
	}
	if (info.getVisibilityChange() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_VISIBILITY_CHANGE", info.getVisibilityChange()));
	}
	if (info.isActsAsCity())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_DEFENSE_MODIFIER_EXTRA"));
	}

	if (0 != info.getHappiness())
	{
		szBuffer.append(NEWLINE);
		CvWString szFloatString;
		float fHappiness = 0.01f * (float)info.getHappiness();
		szFloatString.Format(L"%.02f", fHappiness);

		CvWString szString1;
		CvWString szString2;
		szString1.append(gDLL->getText("TXT_KEY_IMPROVEMENT_HAPPINESS_PEDIA_1"));
		szString2.append(gDLL->getText("TXT_KEY_IMPROVEMENT_HAPPINESS_PEDIA_2"));

		szTempBuffer.Format(L"%s%s%s%c%s", szString1.GetCString(), ((fHappiness > 0) ? "+" : ""), szFloatString.GetCString(), (info.getHappiness() > 0 ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR)), szString2.GetCString());
		szBuffer.append(szTempBuffer);

		//		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ICON_CHANGE_NEARBY_CITIES", info.getHappiness()/100, (info.getHappiness() > 0 ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
	}

	int iHealth = info.getHealth();

	if ((iHealth < 0) && !bCivilopediaText) {
		if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER) {
			if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isIgnoreUnHealthFromImprovements()) {
				iHealth = 0;
			}
		}
	}

	if (0 != iHealth)
	{
		szBuffer.append(NEWLINE);
		CvWString szFloatString;
		float fHealth = 0.01f * (float)info.getHealth();
		szFloatString.Format(L"%.02f", fHealth);

		CvWString szString1;
		CvWString szString2;
		szString1.append(gDLL->getText("TXT_KEY_IMPROVEMENT_HAPPINESS_PEDIA_1"));
		szString2.append(gDLL->getText("TXT_KEY_IMPROVEMENT_HAPPINESS_PEDIA_2"));

		szTempBuffer.Format(L"%s%s%s%c%s", szString1.GetCString(), ((fHealth > 0) ? "+" : ""), szFloatString.GetCString(), (info.getHappiness() > 0 ? gDLL->getSymbolID(HEALTHY_CHAR) : gDLL->getSymbolID(UNHEALTHY_CHAR)), szString2.GetCString());
		szBuffer.append(szTempBuffer);
	}

	if (info.getAdjacentFindResourceModify() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_ADJACENT_RESOURCE_FIND_INCREASE"));
	}

	if (info.getCultureChange() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_CULTURE_CHANGE", info.getCultureChange()));
	}

	bFirst = true;
	for (int iSpell = 0; iSpell < GC.getNumSpellInfos(); iSpell++)
	{
		if (!GC.getSpellInfo((SpellTypes)iSpell).isGraphicalOnly())
		{
			if (eImprovement == GC.getSpellInfo((SpellTypes)iSpell).getImprovementPrereq())
			{
				szFirstBuffer.assign(NEWLINE);
				szFirstBuffer.append(gDLL->getText("TXT_KEY_MISC_ALLOWS_LIST"));
				setListHelp(szBuffer, szFirstBuffer, getLinkedText((SpellTypes)iSpell), L", ", bFirst);
				bFirst = false;
			}
		}
	}
	if (info.getSpawnUnitType() != NO_UNIT)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_SPAWN_UNIT_TYPE", getLinkedText((UnitTypes)info.getSpawnUnitType()).c_str()));
	}
	if (bCivilopediaText)
	{
		// Super Forts begin *text*
		if (info.getCulture() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PLOT_CULTURE", info.getCulture()));
		}
		if (info.getCultureRange() > 0 && ((info.getCulture() > 0) || info.isActsAsCity()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_CULTURE_RANGE", info.getCultureRange()));
		}
		if (info.getVisibilityChange() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_VISIBILITY_RANGE", info.getVisibilityChange()));
		}
		if (info.getSeeFrom() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_SEE_FROM", info.getSeeFrom()));
		}
		// Super Forts end

		if (info.getPillageGold() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PILLAGE_YIELDS", info.getPillageGold()));
		}
	}
	if (!CvWString(info.getHelp()).empty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(info.getHelp());
	}
	if (info.getImprovementUpgrade() != NO_IMPROVEMENT)
	{
		iTurns = GC.getGameINLINE().getImprovementUpgradeTime(eImprovement);
		szBuffer.append(NEWLINE);
		if (GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getPrereqCivilization() == NO_CIVILIZATION)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_EVOLVES",
				getLinkedText((ImprovementTypes)info.getImprovementUpgrade()).c_str(),
				iTurns));

				// Super Forts begin *text* *upgrade*
				if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && info.isUpgradeRequiresFortify())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_FORTIFY_TO_UPGRADE"));
				}
				// Super Forts end
		}
		else
		{
			const ImprovementTypes eImprovementUpgrade = (ImprovementTypes)info.getImprovementUpgrade();
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_EVOLVES_PREREQ_CIV_NO_LINK",
				getLinkedText(eImprovementUpgrade).c_str(),
				iTurns,
				getLinkedText((CivilizationTypes)GC.getImprovementInfo(eImprovementUpgrade).getPrereqCivilization()).c_str()));
		}
	}
	//<<<<	BUGFfH: End Move

	//>>>>	BUGFfH: Added by Denev 2009/10/07
	if (info.getBonusConvert() != NO_BONUS)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_GENERATES_BONUS", getLinkedText((BonusTypes)info.getBonusConvert()).c_str()));
	}

	//<<<<	BUGFfH: End Add

	if (info.isRequiresRiverSide())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_REQUIRES_RIVER"));
	}
	if (info.isCarriesIrrigation())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_CARRIES_IRRIGATION"));
	}
	if (bCivilopediaText)
	{
		if (info.isNoFreshWater())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_NO_BUILD_FRESH_WATER"));
		}
		if (info.isWater())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_BUILD_ONLY_WATER"));
		}
		if (info.isRequiresFlatlands())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_ONLY_BUILD_FLATLANDS"));
		}
	}

	//>>>>	BUGFfH: Moved to above(*12) by Denev 2009/09/21
	/*
		if (info.getImprovementUpgrade() != NO_IMPROVEMENT)
		{
			iTurns = GC.getGameINLINE().getImprovementUpgradeTime(eImprovement);

			szBuffer.append(NEWLINE);

	//FfH: Modified by Kael 05/24/2008
	//		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_EVOLVES", GC.getImprovementInfo((ImprovementTypes) info.getImprovementUpgrade()).getTextKeyWide(), iTurns));
			if (GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getPrereqCivilization() == NO_CIVILIZATION)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_EVOLVES", GC.getImprovementInfo((ImprovementTypes) info.getImprovementUpgrade()).getTextKeyWide(), iTurns));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_EVOLVES_PREREQ_CIV", GC.getImprovementInfo((ImprovementTypes) GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getTextKeyWide(), iTurns, GC.getCivilizationInfo((CivilizationTypes)GC.getImprovementInfo((ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade()).getPrereqCivilization()).getDescription()));
			}
	//FfH: End Modify

		}
	*/
	//<<<<	BUGFfH: End Move

	int iLast = -1;
	for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
	{
		int iRand = info.getImprovementBonusDiscoverRand(iBonus);
		if (iRand > 0)
		{
			//>>>>	BUGFfH: Modified by Denev 2009/10/07
			//			szFirstBuffer.Format(L"%s%s", NEWLINE, gDLL->getText("TXT_KEY_IMPROVEMENT_CHANCE_DISCOVER").c_str());
			szFirstBuffer.append(NEWLINE);
			szFirstBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_CHANCE_DISCOVER", info.getImprovementBonusDiscoverRand(iBonus)));
			//<<<<	BUGFfH: End Move
			szTempBuffer.Format(L"%c", GC.getBonusInfo((BonusTypes)iBonus).getChar());
			setListHelp(szBuffer, szFirstBuffer, szTempBuffer, L", ", iRand != iLast);
			iLast = iRand;
		}
	}
	if (iLast != -1) {
		if (info.getPrereqTerrainSpreadResources() != NO_TERRAIN) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PREREQ_TERRAIN_DISCOVER", getLinkedText((TerrainTypes)info.getPrereqTerrainSpreadResources()).c_str()));
		}
	}

	//>>>>	BUGFfH: Moved to above(*13) by Denev 2009/09/21
	/*
		if (0 != info.getDefenseModifier())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_DEFENSE_MODIFIER", info.getDefenseModifier()));
		}

		if (0 != info.getHappiness())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_ICON_CHANGE_NEARBY_CITIES", info.getHappiness(), (info.getHappiness() > 0 ? gDLL->getSymbolID(HAPPY_CHAR) : gDLL->getSymbolID(UNHAPPY_CHAR))));
		}

		if (info.isActsAsCity())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_DEFENSE_MODIFIER_EXTRA"));
		}
	*/
	//<<<<	BUGFfH: End Move

	if (info.getFeatureGrowthProbability() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_MORE_GROWTH"));
	}
	else if (info.getFeatureGrowthProbability() < 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_LESS_GROWTH"));
	}

	// Super Forts begin *text* *bombard*
	if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
	{
		if (info.isBombardable() && (info.getDefenseModifier() > 0))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_BOMBARD"));
		}
		if (info.getUniqueRange() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_UNIQUE_RANGE", info.getUniqueRange()));
		}
	}
	// Super Forts end

	if (info.getGoldCost() > 0) {
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_COST_BASE"));
		szTempBuffer.Format(L"%d %c", info.getGoldCost(), GC.getCommerceInfo(COMMERCE_GOLD).getChar());
		szBuffer.append(szTempBuffer);
	}

	bool bDisplayYieldCosts = false;
	for (int i = 0; i < NUM_YIELD_TYPES; ++i) {
		if (info.getYieldCost(i) > 0) {
			bDisplayYieldCosts = true;
		}
	}
	if (bDisplayYieldCosts) {
		bFirst = false;
		if (info.getGoldCost() > 0)
			bFirst = true;

		if (!bFirst) {
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_COST_BASE"));
		}

		for (int i = 0; i < NUM_YIELD_TYPES; ++i) {
			if (info.getYieldCost(i) > 0) {
				if (!bFirst) {
					bFirst = true;
				}
				else {
					szTempBuffer.Format(L", ");
					szBuffer.append(szTempBuffer);
				}

				szTempBuffer.Format(L"%d %c", info.getYieldCost(i), GC.getYieldInfo((YieldTypes)i).getChar());
				szBuffer.append(szTempBuffer);
			}
		}
	}


	bFirst = false;
	if (info.isDefaultBuildableAllCivs()) {
		for (int i = 0; i < GC.getNumCivilizationInfos(); ++i) {
			if (GC.getCivilizationInfo((CivilizationTypes)i).isImprovementNonDefault(eImprovement)) {
				if (!bFirst) {
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_CIV_RESTRICTED"));
					szBuffer.append(getLinkedText((CivilizationTypes)i).c_str());
					bFirst = true;
				}
				else {
					szTempBuffer.Format(L", %s", getLinkedText((CivilizationTypes)i).c_str());
					szBuffer.append(szTempBuffer);
				}
			}
		}
	}
	else {
		for (int i = 0; i < GC.getNumCivilizationInfos(); ++i) {
			if (GC.getCivilizationInfo((CivilizationTypes)i).isImprovementNonDefault(eImprovement)) {
				if (!bFirst) {
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_CIV_ENABLED"));
					szBuffer.append(getLinkedText((CivilizationTypes)i).c_str());
					bFirst = true;
				}
				else {
					szTempBuffer.Format(L", %s", getLinkedText((CivilizationTypes)i).c_str());
					szBuffer.append(szTempBuffer);
				}
			}
		}
	}


	//FfH: Added by Kael 05/12/2008
	//>>>>	BUGFfH: Moved to above(*14) by Denev 2009/09/21
	/*
		if (info.isUnique())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_UNIQUE"));
		}
	*/
	//<<<<	BUGFfH: End Move

	//>>>>	BUGFfH: Modified by Denev 2009/10/01
	/*
		if (info.getPrereqCivilization() != NO_CIVILIZATION)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PREREQ_CIVILIZATION", GC.getCivilizationInfo((CivilizationTypes)info.getPrereqCivilization()).getDescription()));
		}
	*/
	if (!bCivilopediaText)
	{
		if (info.getPrereqCivilization() != NO_CIVILIZATION)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PREREQ_CIVILIZATION", getLinkedText((CivilizationTypes)info.getPrereqCivilization(), false).c_str()));
		}
	}
	//<<<<	BUGFfH: End Modify

	//>>>>	BUGFfH: Moved to above(*15) by Denev 2009/09/21
	/*
		if (info.getHealRateChange() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_HEAL_RATE_CHANGE", info.getHealRateChange()));
		}
		if (info.getRangeDefenseModifier() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_RANGE_DEFENSE_MODIFIER", info.getRangeDefenseModifier(), info.getRange()));
		}
		if (info.getVisibilityChange() != 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_VISIBILITY_CHANGE", info.getVisibilityChange()));
		}
		if (info.getSpawnUnitType() != NO_UNIT)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_SPAWN_UNIT_TYPE", GC.getUnitInfo((UnitTypes)info.getSpawnUnitType()).getDescription()));
		}
		if (!CvWString(info.getHelp()).empty())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(info.getHelp());
		}
	//FfH: End Add

		if (bCivilopediaText)
		{
			if (info.getPillageGold() > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_IMPROVEMENT_PILLAGE_YIELDS", info.getPillageGold()));
			}
		}
	*/
	//<<<<	BUGFfH: End Move
}


//>>>>	BUGFfH: Added by Denev 2009/09/05
void CvGameTextMgr::setImprovementYieldHelp(CvWStringBuffer& szBuffer, ImprovementTypes eImprovement, bool bCivilopediaText)
{
	if (NO_IMPROVEMENT == eImprovement)
	{
		return;
	}

	CvImprovementInfo& pImprovement = GC.getImprovementInfo(eImprovement);

	const CvWString szStart = bCivilopediaText ? L"" : L", ";
	setYieldChangeHelp(szBuffer, szStart, L"", L"", pImprovement.getYieldChangeArray(), false, bCivilopediaText);

	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_WITH_IRRIGATION").c_str(), pImprovement.getIrrigatedYieldChangeArray());
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_ON_HILLS").c_str(), pImprovement.getHillsYieldChangeArray());
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_MISC_ALONG_RIVER").c_str(), pImprovement.getRiverSideYieldChangeArray());

	//	Tech
	for (int iTech = 0; iTech < GC.getNumTechInfos(); iTech++)
	{
		int aiYieldAmount[NUM_YIELD_TYPES];
		bool bYields = false;
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
		{
			aiYieldAmount[iYield] = pImprovement.getTechYieldChanges(iTech, iYield);
			bYields = bYields || aiYieldAmount[iYield] != 0;
		}
		if (bYields)
		{
			setYieldChangeHelp(szBuffer, L"", L"",
				gDLL->getText("TXT_KEY_IMPROVEMENT_YIELD_WITH_TECH", getLinkedText((TechTypes)iTech).c_str()),
				aiYieldAmount);
		}
	}

	//	Civics
	for (int iCivic = 0; iCivic < GC.getNumCivicInfos(); iCivic++)
	{
		int aiYieldAmount[NUM_YIELD_TYPES];
		bool bYields = false;
		for (int iYield = 0; iYield < NUM_YIELD_TYPES; iYield++)
		{
			aiYieldAmount[iYield] = GC.getCivicInfo((CivicTypes)iCivic).getImprovementYieldChanges(eImprovement, iYield);
			bYields = bYields || aiYieldAmount[iYield] != 0;
		}
		if (bYields)
		{
			setYieldChangeHelp(szBuffer, L"", L"",
				gDLL->getText("TXT_KEY_IMPROVEMENT_YIELD_FROM_CIVIC", getLinkedText((CivicTypes)iCivic).c_str()),
				aiYieldAmount);
		}
	}

	//	Routes
	for (int iRoute = 0; iRoute < GC.getNumRouteInfos(); iRoute++)
	{
		setYieldChangeHelp(szBuffer, L"", L"",
			gDLL->getText("TXT_KEY_IMPROVEMENT_YIELD_WITH_ROUTE", GC.getRouteInfo((RouteTypes)iRoute).getTextKeyWide()),
			pImprovement.getRouteYieldChangesArray((RouteTypes)iRoute));
	}
}
//<<<<	BUGFfH: End Add


void CvGameTextMgr::getDealString(CvWStringBuffer& szBuffer, CvDeal& deal, PlayerTypes ePlayerPerspective)
{
	PlayerTypes ePlayer1 = deal.getFirstPlayer();
	PlayerTypes ePlayer2 = deal.getSecondPlayer();

	const CLinkList<TradeData>* pListPlayer1 = deal.getFirstTrades();
	const CLinkList<TradeData>* pListPlayer2 = deal.getSecondTrades();

	getDealString(szBuffer, ePlayer1, ePlayer2, pListPlayer1, pListPlayer2, ePlayerPerspective);
}

void CvGameTextMgr::getDealString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer1, PlayerTypes ePlayer2, const CLinkList<TradeData>* pListPlayer1, const CLinkList<TradeData>* pListPlayer2, PlayerTypes ePlayerPerspective)
{
	if (NO_PLAYER == ePlayer1 || NO_PLAYER == ePlayer2)
	{
		FAssertMsg(false, "Deal needs two parties");
		return;
	}

	CvWStringBuffer szDealOne;
	if (NULL != pListPlayer1 && pListPlayer1->getLength() > 0)
	{
		CLLNode<TradeData>* pTradeNode;
		bool bFirst = true;
		for (pTradeNode = pListPlayer1->head(); pTradeNode; pTradeNode = pListPlayer1->next(pTradeNode))
		{
			CvWStringBuffer szTrade;
			getTradeString(szTrade, pTradeNode->m_data, ePlayer1, ePlayer2);
			setListHelp(szDealOne, L"", szTrade.getCString(), L", ", bFirst);
			bFirst = false;
		}
	}

	CvWStringBuffer szDealTwo;
	if (NULL != pListPlayer2 && pListPlayer2->getLength() > 0)
	{
		CLLNode<TradeData>* pTradeNode;
		bool bFirst = true;
		for (pTradeNode = pListPlayer2->head(); pTradeNode; pTradeNode = pListPlayer2->next(pTradeNode))
		{
			CvWStringBuffer szTrade;
			getTradeString(szTrade, pTradeNode->m_data, ePlayer2, ePlayer1);
			setListHelp(szDealTwo, L"", szTrade.getCString(), L", ", bFirst);
			bFirst = false;
		}
	}

	if (!szDealOne.isEmpty())
	{
		if (!szDealTwo.isEmpty())
		{
			if (ePlayerPerspective == ePlayer1)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_OUR_DEAL", szDealOne.getCString(), GET_PLAYER(ePlayer2).getNameKey(), szDealTwo.getCString()));
			}
			else if (ePlayerPerspective == ePlayer2)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_OUR_DEAL", szDealTwo.getCString(), GET_PLAYER(ePlayer1).getNameKey(), szDealOne.getCString()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEAL", GET_PLAYER(ePlayer1).getNameKey(), szDealOne.getCString(), GET_PLAYER(ePlayer2).getNameKey(), szDealTwo.getCString()));
			}
		}
		else
		{
			if (ePlayerPerspective == ePlayer1)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_OURS", szDealOne.getCString(), GET_PLAYER(ePlayer2).getNameKey()));
			}
			else if (ePlayerPerspective == ePlayer2)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_THEIRS", szDealOne.getCString(), GET_PLAYER(ePlayer1).getNameKey()));
			}
			else
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED", GET_PLAYER(ePlayer1).getNameKey(), szDealOne.getCString(), GET_PLAYER(ePlayer2).getNameKey()));
			}
		}
	}
	else if (!szDealTwo.isEmpty())
	{
		if (ePlayerPerspective == ePlayer1)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_THEIRS", szDealTwo.getCString(), GET_PLAYER(ePlayer2).getNameKey()));
		}
		else if (ePlayerPerspective == ePlayer2)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED_OURS", szDealTwo.getCString(), GET_PLAYER(ePlayer1).getNameKey()));
		}
		else
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEAL_ONESIDED", GET_PLAYER(ePlayer2).getNameKey(), szDealTwo.getCString(), GET_PLAYER(ePlayer1).getNameKey()));
		}
	}
}

void CvGameTextMgr::getWarplanString(CvWStringBuffer& szString, WarPlanTypes eWarPlan)
{
	switch (eWarPlan)
	{
	case WARPLAN_ATTACKED_RECENT: szString.assign(L"new defensive war"); break;
	case WARPLAN_ATTACKED: szString.assign(L"defensive war"); break;
	case WARPLAN_PREPARING_LIMITED: szString.assign(L"preparing limited war"); break;
	case WARPLAN_PREPARING_TOTAL: szString.assign(L"preparing total war"); break;
	case WARPLAN_LIMITED: szString.assign(L"limited war"); break;
	case WARPLAN_TOTAL: szString.assign(L"total war"); break;
	case WARPLAN_DOGPILE: szString.assign(L"dogpile war"); break;
	case NO_WARPLAN: szString.assign(L"unplanned war"); break;
	default:  szString.assign(L"unknown war"); break;
	}
}

void CvGameTextMgr::getAttitudeString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer, PlayerTypes eTargetPlayer)
{
	CvWString szTempBuffer;
	int iAttitudeChange;
	int iPass;
	int iI;
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	TeamTypes eTeam = (TeamTypes)kPlayer.getTeam();
	CvTeamAI& kTeam = GET_TEAM(eTeam);

	szBuffer.append(gDLL->getText("TXT_KEY_ATTITUDE_TOWARDS", GC.getAttitudeInfo(GET_PLAYER(ePlayer).AI_getAttitude(eTargetPlayer)).getTextKeyWide(), GET_PLAYER(eTargetPlayer).getNameKey()));

	for (int iTeam = 0; iTeam < MAX_TEAMS; iTeam++)
	{
		CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iTeam);
		if (kLoopTeam.isAlive())
		{
			if (NO_PLAYER != eTargetPlayer)
			{
				CvTeam& kTargetTeam = GET_TEAM(GET_PLAYER(eTargetPlayer).getTeam());
				if (kTargetTeam.isHasMet((TeamTypes)iTeam))
				{
					if (kTeam.isVassal((TeamTypes)iTeam))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ATTITUDE_VASSAL_OF", kLoopTeam.getName().GetCString()));

						setVassalRevoltHelp(szBuffer, (TeamTypes)iTeam, kTeam.getID());
					}
					else if (kLoopTeam.isVassal(kTeam.getID()))
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_ATTITUDE_MASTER_OF", kLoopTeam.getName().GetCString()));
					}
				}
			}
		}
	}

	for (iPass = 0; iPass < 2; iPass++)
	{
		iAttitudeChange = kPlayer.AI_getCloseBordersAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_LAND_TARGET", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getWarAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_WAR", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getPeaceAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_PEACE", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getSameReligionAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_SAME_RELIGION", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getDifferentReligionAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_DIFFERENT_RELIGION", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getBonusTradeAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_BONUS_TRADE", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getOpenBordersAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_OPEN_BORDERS", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getDefensivePactAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_DEFENSIVE_PACT", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getRivalDefensivePactAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_RIVAL_DEFENSIVE_PACT", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getRivalVassalAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_RIVAL_VASSAL", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getShareWarAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_SHARE_WAR", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getFavoriteCivicAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_FAVORITE_CIVIC", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getTradeAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_TRADE", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = kPlayer.AI_getRivalTradeAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_RIVAL_TRADE", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getColonyAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_FREEDOM", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		iAttitudeChange = GET_PLAYER(ePlayer).AI_getAttitudeExtra(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText(((iAttitudeChange > 0) ? "TXT_KEY_MISC_ATTITUDE_EXTRA_GOOD" : "TXT_KEY_MISC_ATTITUDE_EXTRA_BAD"), iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		/*************************************************************************************************/
		/**	New Gameoption (Show Hidden Diplomatic Modifiers) Sephi                    				    **/
		/*************************************************************************************************/

		/**	Hated Civic **/

		iAttitudeChange = kPlayer.AI_getHatedCivicAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_HATED_CIVIC", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}

		if (GC.getGameINLINE().isOption(GAMEOPTION_SHOW_HIDDEN_DIPLO_VALUES))
		{
			// BEGIN: Show Hidden Attitude Mod 9/19/2009
			iAttitudeChange = kPlayer.AI_getRankDifferenceAttitude(eTargetPlayer);
			if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText(((iAttitudeChange > 0) ? "TXT_KEY_MISC_ATTITUDE_BETTER_RANK" : "TXT_KEY_MISC_ATTITUDE_WORSE_RANK"), iAttitudeChange).GetCString());
				szBuffer.append(NEWLINE);
				szBuffer.append(szTempBuffer);
			}

			iAttitudeChange = kPlayer.AI_getLowRankAttitude(eTargetPlayer);
			if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_LOW_RANK", iAttitudeChange).GetCString());
				szBuffer.append(NEWLINE);
				szBuffer.append(szTempBuffer);
			}

			iAttitudeChange = kPlayer.AI_getLostWarAttitude(eTargetPlayer);
			if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_LOST_WAR", iAttitudeChange).GetCString());
				szBuffer.append(NEWLINE);
				szBuffer.append(szTempBuffer);
			}

			iAttitudeChange = kPlayer.AI_getTeamSizeAttitude(eTargetPlayer);
			if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_TEAM_SIZE", iAttitudeChange).GetCString());
				szBuffer.append(NEWLINE);
				szBuffer.append(szTempBuffer);
			}

			iAttitudeChange = kPlayer.AI_getFirstImpressionAttitude(eTargetPlayer);
			if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_FIRST_IMPRESSION", iAttitudeChange).GetCString());
				szBuffer.append(NEWLINE);
				szBuffer.append(szTempBuffer);
			}
			// END: Show Hidden Attitude Mod
		}
		/*************************************************************************************************/
		/**	END	                                        												**/
		/*************************************************************************************************/

		//FfH: Added by Kael 08/15/2007
		iAttitudeChange = kPlayer.AI_getAlignmentAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_ALIGNMENT", iAttitudeChange, GC.getAlignmentInfo((AlignmentTypes)GET_PLAYER(eTargetPlayer).getAlignment()).getDescription()).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
		for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			if (GET_PLAYER(eTargetPlayer).hasBonus((BonusTypes)iI))
			{
				iAttitudeChange = GC.getBonusInfo((BonusTypes)iI).getBadAttitude() * GC.getLeaderHeadInfo(kPlayer.getPersonalityType()).getAttitudeBadBonus();
				if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
				{
					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_BAD_BONUS", iAttitudeChange, GC.getBonusInfo((BonusTypes)iI).getDescription()).GetCString());
					szBuffer.append(NEWLINE);
					szBuffer.append(szTempBuffer);
				}
			}
		}
		iAttitudeChange = GET_PLAYER(ePlayer).AI_getFavoriteWonderAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_FAVORITE_WONDER", iAttitudeChange, GC.getBuildingInfo((BuildingTypes)GC.getLeaderHeadInfo(GET_PLAYER(ePlayer).getPersonalityType()).getFavoriteWonder()).getTextKeyWide()).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
		iAttitudeChange = GET_PLAYER(ePlayer).AI_getGenderAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText(((iAttitudeChange > 0) ? "TXT_KEY_MISC_ATTITUDE_GENDER_GOOD" : "TXT_KEY_MISC_ATTITUDE_GENDER_BAD"), iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
		iAttitudeChange = GET_PLAYER(ePlayer).AI_getTrustAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_TRUST", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
		iAttitudeChange = GET_PLAYER(ePlayer).AI_getCivicShareAttitude(eTargetPlayer);
		if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_CIVIC_SHARE", iAttitudeChange).GetCString());
			szBuffer.append(NEWLINE);
			szBuffer.append(szTempBuffer);
		}
		//FfH: End Add

		for (iI = 0; iI < NUM_MEMORY_TYPES; ++iI)
		{
			iAttitudeChange = kPlayer.AI_getMemoryAttitude(eTargetPlayer, ((MemoryTypes)iI));
			//FfH Card Game: Added by Sto 08/17/2008
			if (iI == MEMORY_SOMNIUM_POSITIVE)
			{
				if ((iAttitudeChange > 0) && (iPass == 0))
				{
					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), gDLL->getText("TXT_KEY_MEMORY_SOMNIUM_POSITIVE", iAttitudeChange).GetCString());
					szBuffer.append(NEWLINE);
					szBuffer.append(szTempBuffer);
				}
			}
			else if (iI == MEMORY_SOMNIUM_NEGATIVE)
			{
				if ((iAttitudeChange < 0) && (iPass == 1))
				{
					szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MEMORY_SOMNIUM_NEGATIVE", iAttitudeChange).GetCString());
					szBuffer.append(NEWLINE);
					szBuffer.append(szTempBuffer);
				}
			}
			else if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
				//if ((iPass == 0) ? (iAttitudeChange > 0) : (iAttitudeChange < 0))
	//FfH: End Add
			{
				szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR((iAttitudeChange > 0) ? "COLOR_POSITIVE_TEXT" : "COLOR_NEGATIVE_TEXT"), gDLL->getText("TXT_KEY_MISC_ATTITUDE_MEMORY", iAttitudeChange, GC.getMemoryInfo((MemoryTypes)iI).getDescription()).GetCString());
				szBuffer.append(NEWLINE);
				szBuffer.append(szTempBuffer);
			}
		}
	}

	/*************************************************************************************************/
	/**	Better Diplomacy (Show Diplo Modifiers) Sephi			                 				    **/
	/*************************************************************************************************/
	if (ePlayer != NO_PLAYER && eTargetPlayer != NO_PLAYER)
	{
		CvLeaderHeadInfo& kLeader = GC.getLeaderHeadInfo(GET_PLAYER(ePlayer).getLeaderType());
		if (kLeader.getBonusTradeAttitudeChangeLimit() >= 3)
		{
			if (kPlayer.AI_getTradeAttitude(eTargetPlayer) < 3)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_BONUSTRADE_HELP"));
			}
		}

		if (kLeader.getOpenBordersAttitudeChangeLimit() >= 3)
		{
			if (kPlayer.AI_getOpenBordersAttitude(eTargetPlayer) < 3)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_OPEN_BORDERS_HELP"));
			}
		}

		if (kLeader.getDefensivePactAttitudeChangeLimit() >= 3)
		{
			if (kPlayer.AI_getDefensivePactAttitude(eTargetPlayer) < 3)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_DEFENSIVE_PACT_HELP"));
			}
		}

		if (kLeader.getFavoriteCivic() != NO_CIVIC && kLeader.getFavoriteCivicAttitudeChangeLimit() >= 3)
		{
			if (kPlayer.AI_getFavoriteCivicAttitude(eTargetPlayer) < 3)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_FAVORITE_CIVIC_HELP", GC.getCivicInfo((CivicTypes)kLeader.getFavoriteCivic()).getDescription()));
			}
		}

		if (kLeader.getWorseRankDifferenceAttitudeChange() >= 3)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_WORSERANK_RESPECT_HELP"));
		}

		if (kLeader.getWorseRankDifferenceAttitudeChange() <= -3)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_WORSERANK_DISRESPECT_HELP"));
		}

		if (kLeader.getBetterRankDifferenceAttitudeChange() >= 3)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_BETTERRANK_RESPECT_HELP"));
		}

		if (kLeader.getBetterRankDifferenceAttitudeChange() <= -3)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_BETTERRANK_DISRESPECT_HELP"));
		}

		if (kLeader.getSameReligionAttitudeChangeLimit() >= 3)
		{
			if (GET_PLAYER(ePlayer).getStateReligion() != GET_PLAYER(eTargetPlayer).getStateReligion())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_SAME_RELIGION_HELP"));
			}
		}

		if (kLeader.getAtPeaceAttitudeChangeLimit() >= 3)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_DIPLO_ATPEACE_HELP"));
		}
	}
	/*************************************************************************************************/
	/**	END	                                        												**/
	/*************************************************************************************************/

	if (NO_PLAYER != eTargetPlayer)
	{
		int iWarWeariness = GET_PLAYER(eTargetPlayer).getModifiedWarWearinessPercentAnger(GET_TEAM(GET_PLAYER(eTargetPlayer).getTeam()).getWarWeariness(eTeam) * std::max(0, 100 + kTeam.getEnemyWarWearinessModifier()));
		if (iWarWeariness / 10000 > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_WAR_WEAR_HELP", iWarWeariness / 10000));
		}
	}

}

void CvGameTextMgr::getEspionageString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer, PlayerTypes eTargetPlayer)
{
	if (!GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		CvPlayer& kPlayer = GET_PLAYER(ePlayer);
		TeamTypes eTeam = (TeamTypes)kPlayer.getTeam();
		CvTeam& kTeam = GET_TEAM(eTeam);
		CvPlayer& kTargetPlayer = GET_PLAYER(eTargetPlayer);

		szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_AGAINST_PLAYER", kTargetPlayer.getNameKey(), kTeam.getEspionagePointsAgainstTeam(kTargetPlayer.getTeam()), GET_TEAM(kTargetPlayer.getTeam()).getEspionagePointsAgainstTeam(kPlayer.getTeam())));
	}
}

void CvGameTextMgr::getTradeString(CvWStringBuffer& szBuffer, const TradeData& tradeData, PlayerTypes ePlayer1, PlayerTypes ePlayer2)
{
	switch (tradeData.m_eItemType)
	{
	case TRADE_GOLD:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOLD", tradeData.m_iData));
		break;
	case TRADE_GOLD_PER_TURN:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_GOLD_PER_TURN", tradeData.m_iData));
		break;
	case TRADE_MAPS:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_WORLD_MAP"));
		break;
	case TRADE_SURRENDER:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_CAPITULATE"));
		break;
	case TRADE_VASSAL:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_VASSAL"));
		break;
	case TRADE_OPEN_BORDERS:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_OPEN_BORDERS"));
		break;
	case TRADE_DEFENSIVE_PACT:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEFENSIVE_PACT"));
		break;
	case TRADE_PERMANENT_ALLIANCE:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_PERMANENT_ALLIANCE"));
		break;
	case TRADE_PEACE_TREATY:
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_PEACE_TREATY", GC.getDefineINT("PEACE_TREATY_LENGTH")));
		break;
	case TRADE_TECHNOLOGIES:
		szBuffer.assign(CvWString::format(L"%s", GC.getTechInfo((TechTypes)tradeData.m_iData).getDescription()));
		break;
	case TRADE_RESOURCES:
		szBuffer.assign(CvWString::format(L"%s", GC.getBonusInfo((BonusTypes)tradeData.m_iData).getDescription()));
		break;
	case TRADE_CITIES:
		szBuffer.assign(CvWString::format(L"%s", GET_PLAYER(ePlayer1).getCity(tradeData.m_iData)->getName().GetCString()));
		break;
	case TRADE_PEACE:
	case TRADE_WAR:
	case TRADE_EMBARGO:
		szBuffer.assign(CvWString::format(L"%s", GET_TEAM((TeamTypes)tradeData.m_iData).getName().GetCString()));
		break;
	case TRADE_CIVIC:
		szBuffer.assign(CvWString::format(L"%s", GC.getCivicInfo((CivicTypes)tradeData.m_iData).getDescription()));
		break;
	case TRADE_RELIGION:
		szBuffer.assign(CvWString::format(L"%s", GC.getReligionInfo((ReligionTypes)tradeData.m_iData).getDescription()));
		break;
	default:
		FAssert(false);
		break;
	}
}

void CvGameTextMgr::setFeatureHelp(CvWStringBuffer& szBuffer, FeatureTypes eFeature, bool bCivilopediaText)
{
	if (NO_FEATURE == eFeature)
	{
		return;
	}
	CvFeatureInfo& feature = GC.getFeatureInfo(eFeature);

	int aiYields[NUM_YIELD_TYPES];
	if (!bCivilopediaText)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/03
		//		szBuffer.append(feature.getDescription());
		szBuffer.append(getLinkedText(eFeature));
		//<<<<	BUGFfH: End Modify

		for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
		{
			aiYields[iI] = feature.getYieldChange(iI);
		}
		setYieldChangeHelp(szBuffer, L"", L"", L"", aiYields);
	}
	for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		aiYields[iI] = feature.getRiverYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_NEXT_TO_RIVER"), aiYields);

	for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		aiYields[iI] = feature.getHillsYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_ON_HILLS"), aiYields);

	if (feature.getMovementCost() != 1)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_MOVEMENT_COST", feature.getMovementCost()));
	}

	CvWString szHealth;
	szHealth.Format(L"%.2f", 0.01f * abs(feature.getHealthPercent()));
	if (feature.getHealthPercent() > 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_GOOD_HEALTH", szHealth.GetCString()));
	}
	else if (feature.getHealthPercent() < 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_BAD_HEALTH", szHealth.GetCString()));
	}

	if (feature.getDefenseModifier() != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_DEFENSE_MODIFIER", feature.getDefenseModifier()));
	}

	if (feature.isAddsFreshWater())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_ADDS_FRESH_WATER"));
	}

	//>>>>	BUGFfH: Added by Denev 2009/10/07
	if (feature.getGrowthProbability() != 0)
	{
		int iGrowthProbability, iGrowthProbabilityOnRoute;
		iGrowthProbability = feature.getGrowthProbability() * (GC.getFEATURE_GROWTH_MODIFIER() + 100);
		iGrowthProbability /= 100;
		iGrowthProbabilityOnRoute = feature.getGrowthProbability() * (GC.getROUTE_FEATURE_GROWTH_MODIFIER() + 100);
		iGrowthProbabilityOnRoute /= 100;
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_GROWING_TO_NEAR_TILE",
			10000 / iGrowthProbability,
			NUM_CARDINALDIRECTION_TYPES,
			10000 / iGrowthProbabilityOnRoute));
	}

	if (feature.isFlammable())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_FLAMMABLE"));
	}

	if (feature.getRequireResist() != NO_DAMAGE)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_REQUIRE_RESIST",
			GC.getDefineINT("FEATURE_REQUIRE_RESIST_AMOUNT"),
			GC.getDamageTypeInfo((DamageTypes)feature.getRequireResist()).getTextKeyWide()));
	}
	//<<<<	BUGFfH: End Add

	if (feature.isImpassable())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_IMPASSABLE"));
	}

	if (feature.isNoCity())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_NO_CITIES"));
	}

	if (feature.isNoImprovement())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_FEATURE_NO_IMPROVEMENT"));
	}

}


void CvGameTextMgr::setTerrainHelp(CvWStringBuffer& szBuffer, TerrainTypes eTerrain, bool bCivilopediaText)
{
	if (NO_TERRAIN == eTerrain)
	{
		return;
	}
	CvTerrainInfo& terrain = GC.getTerrainInfo(eTerrain);

	int aiYields[NUM_YIELD_TYPES];
	if (!bCivilopediaText)
	{
		//>>>>	BUGFfH: Modified by Denev 2009/10/03
		//		szBuffer.append(terrain.getDescription());
		szBuffer.append(getLinkedText(eTerrain));
		//<<<<	BUGFfH: End Modify

		for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
		{
			aiYields[iI] = terrain.getYield(iI);
		}
		setYieldChangeHelp(szBuffer, L"", L"", L"", aiYields);
	}
	for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		aiYields[iI] = terrain.getRiverYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_NEXT_TO_RIVER"), aiYields);

	for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		aiYields[iI] = terrain.getHillsYieldChange(iI);
	}
	setYieldChangeHelp(szBuffer, L"", L"", gDLL->getText("TXT_KEY_TERRAIN_ON_HILLS"), aiYields);

	if (terrain.getMovementCost() != 1)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_MOVEMENT_COST", terrain.getMovementCost()));
	}

	if (terrain.getBuildModifier() != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_BUILD_MODIFIER", terrain.getBuildModifier()));
	}

	if (terrain.getDefenseModifier() != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_DEFENSE_MODIFIER", terrain.getDefenseModifier()));
	}

	if (terrain.isImpassable())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_IMPASSABLE"));
	}
	if (!terrain.isFound())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_NO_CITIES"));
		bool bFirst = true;
		if (terrain.isFoundCoast())
		{
			szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_COASTAL_CITIES"));
			bFirst = false;
		}
		if (!bFirst)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_OR"));
		}
		if (terrain.isFoundFreshWater())
		{
			szBuffer.append(gDLL->getText("TXT_KEY_TERRAIN_FRESH_WATER_CITIES"));
			bFirst = false;
		}
	}
}

void CvGameTextMgr::buildFinanceInflationString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	int iInflationRate = kPlayer.calculateInflationRate();
	if (iInflationRate != 0)
	{
		int iPreInflation = kPlayer.calculatePreInflatedCosts();
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_INFLATION", iPreInflation, iInflationRate, iInflationRate, iPreInflation, (iPreInflation * iInflationRate) / 100));
	}
}

void CvGameTextMgr::buildFinanceUnitCostString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);

	int iFreeUnits = 0;
	int iFreeMilitaryUnits = 0;
	int iUnits = player.getNumUnits();
	int iMilitaryUnits = player.getNumMilitaryUnits();
	int iPaidUnits = iUnits;
	int iPaidMilitaryUnits = iMilitaryUnits;
	int iMilitaryCost = 0;
	int iBaseUnitCost = 0;
	int iExtraCost = 0;
	int iCost = player.calculateUnitCost(iFreeUnits, iFreeMilitaryUnits, iPaidUnits, iPaidMilitaryUnits, iBaseUnitCost, iMilitaryCost, iExtraCost);
	int iHandicap = iCost - iBaseUnitCost - iMilitaryCost - iExtraCost;

	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST", iPaidUnits, iFreeUnits, iBaseUnitCost));

	if (iPaidMilitaryUnits != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST_2", iPaidMilitaryUnits, iFreeMilitaryUnits, iMilitaryCost));
	}
	if (iExtraCost != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST_3", iExtraCost));
	}
	if (iHandicap != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_HANDICAP_COST", iHandicap));
	}
	szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_UNIT_COST_4", iCost));
}

void CvGameTextMgr::buildFinanceAwaySupplyString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);

	int iPaidUnits = 0;
	int iBaseCost = 0;
	int iCost = player.calculateUnitSupply(iPaidUnits, iBaseCost);
	int iHandicap = iCost - iBaseCost;

	CvWString szHandicap;
	if (iHandicap != 0)
	{
		szHandicap = gDLL->getText("TXT_KEY_FINANCE_ADVISOR_HANDICAP_COST", iHandicap);
	}

	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_SUPPLY_COST", iPaidUnits, GC.getDefineINT("INITIAL_FREE_OUTSIDE_UNITS"), iBaseCost, szHandicap.GetCString(), iCost));
}

void CvGameTextMgr::buildFinanceCityMaintString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	int iLoop;
	int iDistanceMaint = 0;
	int iColonyMaint = 0;
	int iCorporationMaint = 0;

	CvPlayer& player = GET_PLAYER(ePlayer);
	for (CvCity* pLoopCity = player.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = player.nextCity(&iLoop))
	{
		iDistanceMaint += (pLoopCity->calculateDistanceMaintenanceTimes100() * std::max(0, (pLoopCity->getMaintenanceModifier() + 100))) / 100;
		iColonyMaint += (pLoopCity->calculateColonyMaintenanceTimes100() * std::max(0, (pLoopCity->getMaintenanceModifier() + 100))) / 100;
		iCorporationMaint += (pLoopCity->calculateCorporationMaintenanceTimes100() * std::max(0, (pLoopCity->getMaintenanceModifier() + 100))) / 100;
	}
	iDistanceMaint /= 100;
	iColonyMaint /= 100;
	iCorporationMaint /= 100;

	int iNumCityMaint = player.getTotalMaintenance() - iDistanceMaint - iColonyMaint - iCorporationMaint;

	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_CITY_MAINT_COST", iDistanceMaint, iNumCityMaint, iColonyMaint, iCorporationMaint, player.getTotalMaintenance()));
}

void CvGameTextMgr::buildFinanceCivicUpkeepString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);
	CvWString szCivicOptionCosts;
	for (int iI = 0; iI < GC.getNumCivicOptionInfos(); ++iI)
	{
		CivicTypes eCivic = player.getCivics((CivicOptionTypes)iI);
		if (NO_CIVIC != eCivic)
		{
			CvWString szTemp;
			szTemp.Format(L"%d%c: %s", player.getSingleCivicUpkeep(eCivic), GC.getCommerceInfo(COMMERCE_GOLD).getChar(), GC.getCivicInfo(eCivic).getDescription());
			szCivicOptionCosts += NEWLINE + szTemp;
		}
	}

	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_CIVIC_UPKEEP_COST", szCivicOptionCosts.GetCString(), player.getCivicUpkeep()));
}

void CvGameTextMgr::buildFinanceForeignIncomeString(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	if (NO_PLAYER == ePlayer)
	{
		return;
	}
	CvPlayer& player = GET_PLAYER(ePlayer);

	CvWString szPlayerIncome;
	for (int iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		CvPlayer& otherPlayer = GET_PLAYER((PlayerTypes)iI);
		if (otherPlayer.isAlive() && player.getGoldPerTurnByPlayer((PlayerTypes)iI) != 0)
		{
			CvWString szTemp;
			szTemp.Format(L"%d%c: %s", player.getGoldPerTurnByPlayer((PlayerTypes)iI), GC.getCommerceInfo(COMMERCE_GOLD).getChar(), otherPlayer.getCivilizationShortDescription());
			szPlayerIncome += NEWLINE + szTemp;
		}
	}
	if (!szPlayerIncome.empty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_FINANCE_ADVISOR_FOREIGN_INCOME", szPlayerIncome.GetCString(), player.getGoldPerTurn()));
	}
}

void CvGameTextMgr::setProductionHelp(CvWStringBuffer& szBuffer, CvCity& city)
{
	FAssertMsg(NO_PLAYER != city.getOwnerINLINE(), "City must have an owner");

	bool bIsProcess = city.isProductionProcess();
	int iPastOverflow = (bIsProcess ? 0 : city.getOverflowProduction());
	if (iPastOverflow != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_OVERFLOW", iPastOverflow));
		szBuffer.append(NEWLINE);
	}

	int iFromChops = (city.isProductionProcess() ? 0 : city.getFeatureProduction());
	if (iFromChops != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_CHOPS", iFromChops));
		szBuffer.append(NEWLINE);
	}

	/*************************************************************************************************/
	/**	GAMEPLAY CHANGE (Balance UnhappyProduction) Sephi                        					**/
	/**																								**/
	/**	                                                                   							**/
	/*************************************************************************************************/
	//FfH: Added by Kael 10/13/2007
	int iUnhappyProd = 0;
	if (city.isUnhappyProduction())
	{
		iUnhappyProd += city.unhappyLevel(0) / 2;

		if (iUnhappyProd > 5)
		{
			iUnhappyProd = std::max(5, city.unhappyLevel(0) / 3);
		}
		iUnhappyProd += city.angryPopulation(0);
	}

	if (iUnhappyProd != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_UNHAPPY_PROD", iUnhappyProd));
		szBuffer.append(NEWLINE);
	}


	//FfH: End Add
	/*************************************************************************************************/
	/**	END                                                                  						**/
	/*************************************************************************************************/

	if (city.getCurrentProductionDifference(false, true) == 0)
	{
		return;
	}

	setYieldHelp(szBuffer, city, YIELD_PRODUCTION);

	//FfH: Modified by Kael 10/13/2007
	//	int iBaseProduction = city.getBaseYieldRate(YIELD_PRODUCTION) + iPastOverflow + iFromChops;
	int iBaseProduction = city.getBaseYieldRate(YIELD_PRODUCTION) + iPastOverflow + iFromChops + iUnhappyProd;
	//FfH: End Modify

	int iBaseModifier = city.getBaseYieldRateModifier(YIELD_PRODUCTION);

	UnitTypes eUnit = city.getProductionUnit();
	if (NO_UNIT != eUnit)
	{
		CvUnitInfo& unit = GC.getUnitInfo(eUnit);

		// Domain
		int iDomainMod = city.getDomainProductionModifier((DomainTypes)unit.getDomainType());
		if (0 != iDomainMod)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_DOMAIN", iDomainMod, GC.getDomainInfo((DomainTypes)unit.getDomainType()).getTextKeyWide()));
			szBuffer.append(NEWLINE);
			iBaseModifier += iDomainMod;
		}

		// Military
		if (unit.isMilitaryProduction())
		{
			int iMilitaryMod = city.getMilitaryProductionModifier() + GET_PLAYER(city.getOwnerINLINE()).getMilitaryProductionModifier();
			if (0 != iMilitaryMod)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_MILITARY", iMilitaryMod));
				szBuffer.append(NEWLINE);
				iBaseModifier += iMilitaryMod;
			}
		}

		// Bonus
		for (int i = 0; i < GC.getNumBonusInfos(); i++)
		{
			if (city.hasBonus((BonusTypes)i))
			{
				int iBonusMod = unit.getBonusProductionModifier(i);
				if (0 != iBonusMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_MOD_BONUS", iBonusMod, unit.getTextKeyWide(), GC.getBonusInfo((BonusTypes)i).getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iBaseModifier += iBonusMod;
				}
			}
		}

		// Trait
		for (int i = 0; i < GC.getNumTraitInfos(); i++)
		{
			if (city.hasTrait((TraitTypes)i))
			{
				int iTraitMod = unit.getProductionTraits(i);

				if (unit.getSpecialUnitType() != NO_SPECIALUNIT)
				{
					iTraitMod += GC.getSpecialUnitInfo((SpecialUnitTypes)unit.getSpecialUnitType()).getProductionTraits(i);
				}
				if (0 != iTraitMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_TRAIT", iTraitMod, unit.getTextKeyWide(), GC.getTraitInfo((TraitTypes)i).getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iBaseModifier += iTraitMod;
				}
			}
		}

		// Religion
		if (NO_PLAYER != city.getOwnerINLINE() && NO_RELIGION != GET_PLAYER(city.getOwnerINLINE()).getStateReligion())
		{
			if (city.isHasReligion(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()))
			{
				int iReligionMod = GET_PLAYER(city.getOwnerINLINE()).getStateReligionUnitProductionModifier();
				if (0 != iReligionMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_RELIGION", iReligionMod, GC.getReligionInfo(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()).getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iBaseModifier += iReligionMod;
				}
			}
		}
	}

	BuildingTypes eBuilding = city.getProductionBuilding();
	if (NO_BUILDING != eBuilding)
	{
		CvBuildingInfo& building = GC.getBuildingInfo(eBuilding);

		// Bonus
		for (int i = 0; i < GC.getNumBonusInfos(); i++)
		{
			if (city.hasBonus((BonusTypes)i))
			{
				int iBonusMod = building.getBonusProductionModifier(i);
				if (0 != iBonusMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_MOD_BONUS", iBonusMod, building.getTextKeyWide(), GC.getBonusInfo((BonusTypes)i).getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iBaseModifier += iBonusMod;
				}
			}
		}

		// Trait
		for (int i = 0; i < GC.getNumTraitInfos(); i++)
		{
			if (city.hasTrait((TraitTypes)i))
			{
				int iTraitMod = building.getProductionTraits(i);

				if (building.getSpecialBuildingType() != NO_SPECIALBUILDING)
				{
					iTraitMod += GC.getSpecialBuildingInfo((SpecialBuildingTypes)building.getSpecialBuildingType()).getProductionTraits(i);
				}
				if (0 != iTraitMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_TRAIT", iTraitMod, building.getTextKeyWide(), GC.getTraitInfo((TraitTypes)i).getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iBaseModifier += iTraitMod;
				}
			}
		}

		// Wonder
		if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) && NO_PLAYER != city.getOwnerINLINE())
		{
			int iWonderMod = GET_PLAYER(city.getOwnerINLINE()).getMaxGlobalBuildingProductionModifier();
			if (0 != iWonderMod)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_WONDER", iWonderMod));
				szBuffer.append(NEWLINE);
				iBaseModifier += iWonderMod;
			}
		}

		// Team Wonder
		if (isTeamWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) && NO_PLAYER != city.getOwnerINLINE())
		{
			int iWonderMod = GET_PLAYER(city.getOwnerINLINE()).getMaxTeamBuildingProductionModifier();
			if (0 != iWonderMod)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_TEAM_WONDER", iWonderMod));
				szBuffer.append(NEWLINE);
				iBaseModifier += iWonderMod;
			}
		}

		// National Wonder
		if (isNationalWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())) && NO_PLAYER != city.getOwnerINLINE())
		{
			int iWonderMod = GET_PLAYER(city.getOwnerINLINE()).getMaxPlayerBuildingProductionModifier();
			if (0 != iWonderMod)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_NATIONAL_WONDER", iWonderMod));
				szBuffer.append(NEWLINE);
				iBaseModifier += iWonderMod;
			}
		}

		// Religion
		if (NO_PLAYER != city.getOwnerINLINE() && NO_RELIGION != GET_PLAYER(city.getOwnerINLINE()).getStateReligion())
		{
			if (city.isHasReligion(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()))
			{
				int iReligionMod = GET_PLAYER(city.getOwnerINLINE()).getStateReligionBuildingProductionModifier();
				if (0 != iReligionMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_RELIGION", iReligionMod, GC.getReligionInfo(GET_PLAYER(city.getOwnerINLINE()).getStateReligion()).getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iBaseModifier += iReligionMod;
				}
			}
		}

		//Capital Bonus
		if (NO_PLAYER != city.getOwnerINLINE())
		{
			CvPlayer& kOwner = GET_PLAYER(city.getOwnerINLINE());
			int iReducedCostMod = kOwner.getProductionCostReductionCapitalBuilding();
			if (iReducedCostMod != 0) {
				if (kOwner.getCapitalCity() != NULL && kOwner.getCapitalCity()->getNumRealBuilding(eBuilding) > 0) {
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_CAPITAL_BUILDING_BONUS", iReducedCostMod));
					szBuffer.append(NEWLINE);
					iBaseModifier += iReducedCostMod;
				}
			}
		}
	}

	ProjectTypes eProject = city.getProductionProject();
	if (NO_PROJECT != eProject)
	{
		CvProjectInfo& project = GC.getProjectInfo(eProject);

		// Spaceship

//FfH: Modified by Kael 12/17/2008
//		if (project.isSpaceship())
//		{
//			int iSpaceshipMod = city.getSpaceProductionModifier();
//			if (NO_PLAYER != city.getOwnerINLINE())
//			{
//				iSpaceshipMod += GET_PLAYER(city.getOwnerINLINE()).getSpaceProductionModifier();
//			}
//			if (0 != iSpaceshipMod)
//			{
//				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_SPACESHIP", iSpaceshipMod));
//				szBuffer.append(NEWLINE);
//				iBaseModifier += iSpaceshipMod;
//			}
//		}
		int iSpaceshipMod = city.getSpaceProductionModifier();
		if (NO_PLAYER != city.getOwnerINLINE())
		{
			iSpaceshipMod += GET_PLAYER(city.getOwnerINLINE()).getSpaceProductionModifier();
		}
		if (0 != iSpaceshipMod)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_RITUAL", iSpaceshipMod));
			szBuffer.append(NEWLINE);
			iBaseModifier += iSpaceshipMod;
		}
		//FfH: End Modify

				// Bonus
		for (int i = 0; i < GC.getNumBonusInfos(); i++)
		{
			if (city.hasBonus((BonusTypes)i))
			{
				int iBonusMod = project.getBonusProductionModifier(i);
				if (0 != iBonusMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_MOD_BONUS", iBonusMod, project.getTextKeyWide(), GC.getBonusInfo((BonusTypes)i).getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iBaseModifier += iBonusMod;
				}
			}
		}
	}

	int iFoodProduction = (city.isFoodProduction() ? std::max(0, (city.getYieldRate(YIELD_FOOD) - city.foodConsumption(true))) : 0);
	if (iFoodProduction > 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_FOOD", iFoodProduction, iFoodProduction));
		szBuffer.append(NEWLINE);
	}

	//added Sephi New Yields
	YieldTypes eYield = NO_YIELD;
	if (city.isProductionBuilding())
	{
		eYield = (YieldTypes)GC.getBuildingInfo(city.getProductionBuilding()).getReducedCostByYield();
	}

	if (city.isProductionUnit())
	{
		eYield = (YieldTypes)GC.getUnitInfo(city.getProductionUnit()).getReducedCostByYield();
	}

	if (eYield != NO_YIELD)
	{
		int iIncrease = GET_PLAYER(city.getOwner()).getProductionIncreaseByYield(eYield);
		if (iIncrease != 0)
		{
			CvWString szTempBuffer;
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_REDUCED_COST_BY_YIELD_HELP", iIncrease));
			szTempBuffer.Format(L"%c", GC.getYieldInfo(eYield).getChar());
			szBuffer.append(szTempBuffer);
			szBuffer.append(NEWLINE);
			iBaseModifier += iIncrease;
		}
	}
	//end added

/*************************************************************************************************/
/**	ADDON (Better Rounding of City Production) Sephi                        					**/
/**																								**/
/**	                                                                   							**/
/*************************************************************************************************/
//	int iModProduction = iFoodProduction + (iBaseModifier * iBaseProduction) / 100;
	int iModProduction = iFoodProduction + ((iBaseModifier * iBaseProduction) + 50) / 100;
	/*************************************************************************************************/
	/**	END                                                                  						**/
	/*************************************************************************************************/

	FAssertMsg(iModProduction == city.getCurrentProductionDifference(false, !bIsProcess), "Modified Production does not match actual value");

	szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PROD_FINAL_YIELD", iModProduction));
	szBuffer.append(NEWLINE);

}

void CvGameTextMgr::setFoodHelp(CvWStringBuffer& szBuffer, CvCity& city)
{
	FAssertMsg(NO_PLAYER != city.getOwnerINLINE(), "City must have an owner");

	CvPlayer& kPlayer = GET_PLAYER(city.getOwnerINLINE());

	if (kPlayer.isIgnoreFood()) {
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_FOOD_IGNORE"));
		szBuffer.append(NEWLINE);
		return;
	}

	setYieldHelp(szBuffer, city, YIELD_FOOD);
}

void CvGameTextMgr::parsePlayerTraits(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	bool bFirst = true;

	for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); ++iTrait)
	{
		if (GET_PLAYER(ePlayer).hasTrait((TraitTypes)iTrait))
		{
			if (bFirst)
			{
				szBuffer.append(L" (");
				bFirst = false;
			}
			else
			{
				szBuffer.append(L", ");
			}
			szBuffer.append(GC.getTraitInfo((TraitTypes)iTrait).getDescription());
		}
	}

	if (!bFirst)
	{
		szBuffer.append(L")");
	}
}

void CvGameTextMgr::parseLeaderHeadHelp(CvWStringBuffer& szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer)
{
	if (NO_PLAYER == eThisPlayer)
	{
		return;
	}

	szBuffer.append(CvWString::format(L"%s", GET_PLAYER(eThisPlayer).getName()));

	parsePlayerTraits(szBuffer, eThisPlayer);

	szBuffer.append(L"\n");

	if (eOtherPlayer != NO_PLAYER)
	{
		CvTeam& kThisTeam = GET_TEAM(GET_PLAYER(eThisPlayer).getTeam());
		if (eOtherPlayer != eThisPlayer && kThisTeam.isHasMet(GET_PLAYER(eOtherPlayer).getTeam()))
		{

			//FfH: Modified by Kael 06/25/2008
			//			getEspionageString(szBuffer, eThisPlayer, eOtherPlayer);
			//FfH: End Modify

			getAttitudeString(szBuffer, eThisPlayer, eOtherPlayer);

			getActiveDealsString(szBuffer, eThisPlayer, eOtherPlayer);

			getOtherRelationsString(szBuffer, eThisPlayer, eOtherPlayer);
		}
	}
}


void CvGameTextMgr::parseLeaderLineHelp(CvWStringBuffer& szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer)
{
	if (NO_PLAYER == eThisPlayer || NO_PLAYER == eOtherPlayer)
	{
		return;
	}
	CvTeam& thisTeam = GET_TEAM(GET_PLAYER(eThisPlayer).getTeam());
	CvTeam& otherTeam = GET_TEAM(GET_PLAYER(eOtherPlayer).getTeam());

	if (thisTeam.getID() == otherTeam.getID())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_PERMANENT_ALLIANCE"));
		szBuffer.append(NEWLINE);
	}
	else if (thisTeam.isAtWar(otherTeam.getID()))
	{
		szBuffer.append(gDLL->getText("TXT_KEY_CONCEPT_WAR"));
		szBuffer.append(NEWLINE);
	}
	else
	{
		if (thisTeam.isDefensivePact(otherTeam.getID()))
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_DEFENSIVE_PACT"));
			szBuffer.append(NEWLINE);
		}
		if (thisTeam.isOpenBorders(otherTeam.getID()))
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_OPEN_BORDERS"));
			szBuffer.append(NEWLINE);
		}
		if (thisTeam.isVassal(otherTeam.getID()))
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_VASSAL"));
			szBuffer.append(NEWLINE);
		}
	}
}


void CvGameTextMgr::getActiveDealsString(CvWStringBuffer& szBuffer, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer)
{
	int iIndex;
	CvDeal* pDeal = GC.getGameINLINE().firstDeal(&iIndex);
	while (NULL != pDeal)
	{
		if ((pDeal->getFirstPlayer() == eThisPlayer && pDeal->getSecondPlayer() == eOtherPlayer)
			|| (pDeal->getFirstPlayer() == eOtherPlayer && pDeal->getSecondPlayer() == eThisPlayer))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(BULLET_CHAR)));
			getDealString(szBuffer, *pDeal, eThisPlayer);
		}
		pDeal = GC.getGameINLINE().nextDeal(&iIndex);
	}
}

void CvGameTextMgr::getOtherRelationsString(CvWStringBuffer& szString, PlayerTypes eThisPlayer, PlayerTypes eOtherPlayer)
{
	if (eThisPlayer == NO_PLAYER || eOtherPlayer == NO_PLAYER)
	{
		return;
	}
	CvPlayer& kThisPlayer = GET_PLAYER(eThisPlayer);
	CvPlayer& kOtherPlayer = GET_PLAYER(eOtherPlayer);

	for (int iTeam = 0; iTeam < MAX_CIV_TEAMS; ++iTeam)
	{
		CvTeamAI& kTeam = GET_TEAM((TeamTypes)iTeam);
		if (kTeam.isAlive() && !kTeam.isMinorCiv() && iTeam != kThisPlayer.getTeam() && iTeam != kOtherPlayer.getTeam())
		{
			if (kTeam.isHasMet(kOtherPlayer.getTeam()))
			{
				if (::atWar((TeamTypes)iTeam, kThisPlayer.getTeam()))
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText(L"TXT_KEY_AT_WAR_WITH", kTeam.getName().GetCString()));
				}

				if (!kTeam.isHuman() && kTeam.AI_getWorstEnemy() == kThisPlayer.getTeam())
				{
					szString.append(NEWLINE);
					szString.append(gDLL->getText(L"TXT_KEY_WORST_ENEMY_OF", kTeam.getName().GetCString()));
				}
			}
		}
	}
}

void CvGameTextMgr::buildHintsList(CvWStringBuffer& szBuffer)
{
	for (int i = 0; i < GC.getNumHints(); i++)
	{
		szBuffer.append(CvWString::format(L"%c%s", gDLL->getSymbolID(BULLET_CHAR), GC.getHints(i).getText()));
		szBuffer.append(NEWLINE);
		szBuffer.append(NEWLINE);
	}
}

void CvGameTextMgr::setCommerceHelp(CvWStringBuffer& szBuffer, CvCity& city, CommerceTypes eCommerceType)
{
	if (NO_COMMERCE == eCommerceType || 0 == city.getCommerceRateTimes100(eCommerceType))
	{
		return;
	}
	CvCommerceInfo& info = GC.getCommerceInfo(eCommerceType);

	if (NO_PLAYER == city.getOwnerINLINE())
	{
		return;
	}
	CvPlayer& owner = GET_PLAYER(city.getOwnerINLINE());

	setYieldHelp(szBuffer, city, YIELD_COMMERCE);

	// Slider
	int iBaseCommerceRate = city.getCommerceFromPercent(eCommerceType, city.getYieldRate(YIELD_COMMERCE) * 100);
	CvWString szRate = CvWString::format(L"%d.%02d", iBaseCommerceRate / 100, iBaseCommerceRate % 100);
	szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_SLIDER_PERCENT_FLOAT", city.getCityCommercePercent(eCommerceType), city.getYieldRate(YIELD_COMMERCE), szRate.GetCString(), info.getChar()));
	szBuffer.append(NEWLINE);

	//FfH: Modified by Kael 12/19/2007
	//	int iSpecialistCommerce = city.getSpecialistCommerce(eCommerceType) + (city.getSpecialistPopulation() + city.getNumGreatPeople()) * owner.getSpecialistExtraCommerce(eCommerceType);
	int iSpecialistCommerce = city.getSpecialistCommerce(eCommerceType) + city.getExtraSpecialistCommerce(eCommerceType) + (city.getSpecialistPopulation() + city.getNumGreatPeople()) * owner.getSpecialistExtraCommerce(eCommerceType);
	//FfH: End Modify

	if (0 != iSpecialistCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_SPECIALIST_COMMERCE", iSpecialistCommerce, info.getChar(), L"TXT_KEY_CONCEPT_SPECIALISTS"));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += 100 * iSpecialistCommerce;
	}

	int iReligionCommerce = city.getReligionCommerce(eCommerceType);
	if (0 != iReligionCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_RELIGION_COMMERCE", iReligionCommerce, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += 100 * iReligionCommerce;
	}

	int iCorporationCommerce = city.getCorporationCommerce(eCommerceType);
	if (0 != iCorporationCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CORPORATION_COMMERCE", iCorporationCommerce, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += 100 * iCorporationCommerce;
	}

	int iBuildingCommerce = city.getBuildingCommerce(eCommerceType);
	if (0 != iBuildingCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_BUILDING_COMMERCE", iBuildingCommerce, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += 100 * iBuildingCommerce;
	}

	//added Sephi
	int iCultureCommerce = GET_PLAYER(city.getOwnerINLINE()).getCommerceChangesPerCultureLevel(eCommerceType) * city.getCultureLevel();
	iCultureCommerce += city.getCommercePerCultureLevel(eCommerceType) * city.getCultureLevel();
	if (0 != iCultureCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CULTURE_COMMERCE", iCultureCommerce, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += 100 * iCultureCommerce;
	}

	int iPopulationCommerce = city.getCommercePerPopulation(eCommerceType) * city.getPopulation();

	if (0 != iPopulationCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_POPULATION_COMMERCE", iPopulationCommerce, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += 100 * iPopulationCommerce;
	}

	if (eCommerceType == COMMERCE_CULTURE) {
		int iCultureFromImprovements = city.getCultureFromImprovements();
		if (0 != iCultureFromImprovements) {
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CULTURE_FROM_IMPROVEMENTS_COMMERCE", iCultureFromImprovements, info.getChar()));
			szBuffer.append(NEWLINE);
		}
		iBaseCommerceRate += (iCultureFromImprovements * 100);
	}

	int iBonusCommerce = city.getCommerceFromBonus(eCommerceType);


	if (0 != iBonusCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_BONUS_COMMERCE", iBonusCommerce / 100, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += iBonusCommerce;
	}

	int iUnitsCommerce = city.getCommerceFromUnits(eCommerceType);


	if (0 != iUnitsCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_UNITS_COMMERCE", iUnitsCommerce / 100, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += iUnitsCommerce;
	}

	if (eCommerceType == COMMERCE_ARCANE)
	{
		if (city.calculateSpellResearchFromFeaturesTimes100() != 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_SPELLRESEARCH_FROM_FEATURES", city.calculateSpellResearchFromFeaturesTimes100() / 100, info.getChar()));
			szBuffer.append(NEWLINE);
			iBaseCommerceRate += city.calculateSpellResearchFromFeaturesTimes100();
		}
	}


	//added Sephi

	int iFreeCityCommerce = owner.getFreeCityCommerce(eCommerceType);
	if (0 != iFreeCityCommerce)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_FREE_CITY_COMMERCE", iFreeCityCommerce, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseCommerceRate += 100 * iFreeCityCommerce;
	}


	FAssertMsg(city.getBaseCommerceRateTimes100(eCommerceType) == iBaseCommerceRate, "Base Commerce rate does not agree with actual value");

	int iModifier = 100;

	// Buildings
	int iBuildingMod = 0;
	for (int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		CvBuildingInfo& infoBuilding = GC.getBuildingInfo((BuildingTypes)i);
		if (city.getNumBuilding((BuildingTypes)i) > 0 && !GET_TEAM(city.getTeam()).isObsoleteBuilding((BuildingTypes)i))
		{
			for (int iLoop = 0; iLoop < city.getNumBuilding((BuildingTypes)i); iLoop++)
			{
				iBuildingMod += infoBuilding.getCommerceModifier(eCommerceType);
			}
		}
		for (int j = 0; j < MAX_PLAYERS; j++)
		{
			if (GET_PLAYER((PlayerTypes)j).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)j).getTeam() == owner.getTeam())
				{
					int iLoop;
					for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)j).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)j).nextCity(&iLoop))
					{
						if (pLoopCity->getNumBuilding((BuildingTypes)i) > 0 && !GET_TEAM(pLoopCity->getTeam()).isObsoleteBuilding((BuildingTypes)i))
						{
							for (int iLoop = 0; iLoop < pLoopCity->getNumBuilding((BuildingTypes)i); iLoop++)
							{
								iBuildingMod += infoBuilding.getGlobalCommerceModifier(eCommerceType);
							}
						}
					}
				}
			}
		}
	}
	if (0 != iBuildingMod)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BUILDINGS", iBuildingMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iModifier += iBuildingMod;
	}


	// Trait
	for (int i = 0; i < GC.getNumTraitInfos(); i++)
	{
		if (city.hasTrait((TraitTypes)i))
		{
			CvTraitInfo& trait = GC.getTraitInfo((TraitTypes)i);
			int iTraitMod = trait.getCommerceModifier(eCommerceType);
			if (0 != iTraitMod)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_TRAIT", iTraitMod, info.getChar(), trait.getTextKeyWide()));
				szBuffer.append(NEWLINE);
				iModifier += iTraitMod;
			}
		}
	}

	// Capital
	int iCapitalMod = city.isCapital() ? owner.getCapitalCommerceRateModifier(eCommerceType) : 0;
	if (iCapitalMod != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CAPITAL", iCapitalMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iModifier += iCapitalMod;
	}


	// Civics
	int iCivicMod = 0;
	for (int i = 0; i < GC.getNumCivicOptionInfos(); i++)
	{
		if (NO_CIVIC != owner.getCivics((CivicOptionTypes)i))
		{
			iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getCommerceModifier(eCommerceType);
		}
	}
	if (0 != iCivicMod)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CIVICS", iCivicMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iModifier += iCivicMod;
	}

	int iSpecializationMod = 0;
	switch (eCommerceType)
	{
	case COMMERCE_GOLD:
		iSpecializationMod = city.getCitySpecializationLevel(CITYSPECIALIZATION_GOLD) * GC.getDefineINT("GOLD_PER_SPECIALIZATION_LEVEL");
		break;
	case COMMERCE_RESEARCH:
		iSpecializationMod = city.getCitySpecializationLevel(CITYSPECIALIZATION_RESEARCH) * GC.getDefineINT("RESEARCH_PER_SPECIALIZATION_LEVEL");
		break;
	case COMMERCE_CULTURE:
		iSpecializationMod = city.getCitySpecializationLevel(CITYSPECIALIZATION_CULTURE) * GC.getDefineINT("CULTURE_PER_SPECIALIZATION_LEVEL");
		break;
	case COMMERCE_ARCANE:
		iSpecializationMod = city.getCitySpecializationLevel(CITYSPECIALIZATION_ARCANE) * GC.getDefineINT("SPELLRESEARCH_PER_SPECIALIZATION_LEVEL");
		break;
	case COMMERCE_FAITH:
		iSpecializationMod = city.getCitySpecializationLevel(CITYSPECIALIZATION_FAITH) * GC.getDefineINT("FAITH_PER_SPECIALIZATION_LEVEL");
		break;
	default:
		break;
	}

	if (0 != iSpecializationMod)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_CITY_SPECIALIZATION", iSpecializationMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iModifier += iSpecializationMod;
	}

	//>>>>	BUGFfH: Added by Denev 2009/09/29
		// Resources
	int iBonusMod = 0;
	for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
	{
		if (owner.hasBonus((BonusTypes)iBonus))
		{
			if (eCommerceType == COMMERCE_RESEARCH)
			{
				iBonusMod += GC.getBonusInfo((BonusTypes)iBonus).getResearchModifier();
			}
			iBonusMod += GC.getBonusInfo((BonusTypes)iBonus).getResearchModifier();
		}
	}
	iBonusMod += city.getCityBonusCommerceRateModifier(eCommerceType);

	if (0 != iBonusMod)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BONUS", iBonusMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iModifier += iBonusMod;
	}
	
	if (owner.getStateReligion() != NO_RELIGION && owner.getStateReligionCommerceTechRateModifier(eCommerceType) != 0) {
		szBuffer.append(gDLL->getText("TXT_KEY_STATE_RELIGION_COMMERCE_FROM_TECH", owner.getStateReligionCommerceTechRateModifier(eCommerceType)));
		szBuffer.append(NEWLINE);
		iModifier += owner.getStateReligionCommerceTechRateModifier(eCommerceType);
	}
	//<<<<	BUGFfH: End Add

	int iModYield = (iModifier * iBaseCommerceRate) / 100;

	int iProductionToCommerce = city.getProductionToCommerceModifier(eCommerceType) * city.getYieldRate(YIELD_PRODUCTION);
	if (0 != iProductionToCommerce)
	{
		if (iProductionToCommerce % 100 == 0)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PRODUCTION_TO_COMMERCE", iProductionToCommerce / 100, info.getChar()));
			szBuffer.append(NEWLINE);
		}
		else
		{
			szRate = CvWString::format(L"+%d.%02d", iProductionToCommerce / 100, iProductionToCommerce % 100);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_PRODUCTION_TO_COMMERCE_FLOAT", szRate.GetCString(), info.getChar()));
			szBuffer.append(NEWLINE);
		}
		iModYield += iProductionToCommerce;
	}
	/**
		if (eCommerceType == COMMERCE_CULTURE && GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
		{
			int iEspionageToCommerce = city.getCommerceRateTimes100(COMMERCE_CULTURE) - iModYield;
			if (0 != iEspionageToCommerce)
			{
				if (iEspionageToCommerce%100 == 0)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_TO_COMMERCE", iEspionageToCommerce/100, info.getChar(), GC.getCommerceInfo(COMMERCE_ESPIONAGE).getChar()));
					szBuffer.append(NEWLINE);
				}
				else
				{
					szRate = CvWString::format(L"+%d.%02d", iEspionageToCommerce/100, iEspionageToCommerce%100);
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_TO_COMMERCE_FLOAT", szRate.GetCString(), info.getChar(), GC.getCommerceInfo(COMMERCE_ESPIONAGE).getChar()));
					szBuffer.append(NEWLINE);
				}
				iModYield += iEspionageToCommerce;
			}
		}
	**/

	if (eCommerceType == COMMERCE_GOLD)
	{
		if (city.calculateTaxesTimes100() != 0)	//Certain traits change tax rate per pop.
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_TAXES_FROM_POPULATION", city.calculateTaxesTimes100() / 100, city.calculateTaxesTimes100() % 100));
			szBuffer.append(NEWLINE);
			iModYield += city.calculateTaxesTimes100();
		}
	}

	FAssertMsg(iModYield == city.getCommerceRateTimes100(eCommerceType), "Commerce yield does not match actual value");

	CvWString szYield = CvWString::format(L"%d.%02d", iModYield / 100, iModYield % 100);
	szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_COMMERCE_FINAL_YIELD_FLOAT", info.getTextKeyWide(), szYield.GetCString(), info.getChar()));
	szBuffer.append(NEWLINE);
}

void CvGameTextMgr::setYieldHelp(CvWStringBuffer& szBuffer, CvCity& city, YieldTypes eYieldType)
{
	FAssertMsg(NO_PLAYER != city.getOwnerINLINE(), "City must have an owner");

	if (NO_YIELD == eYieldType)
	{
		return;
	}
	CvYieldInfo& info = GC.getYieldInfo(eYieldType);

	if (NO_PLAYER == city.getOwnerINLINE())
	{
		return;
	}
	CvPlayer& owner = GET_PLAYER(city.getOwnerINLINE());

	int iBaseProduction = city.getBaseYieldRate(eYieldType);
	szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_BASE_YIELD", info.getTextKeyWide(), iBaseProduction, info.getChar()));
	szBuffer.append(NEWLINE);

	int iBaseModifier = 100;

	// Buildings
	int iBuildingMod = 0;
	for (int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		CvBuildingInfo& infoBuilding = GC.getBuildingInfo((BuildingTypes)i);
		if (city.getNumBuilding((BuildingTypes)i) > 0 && !GET_TEAM(city.getTeam()).isObsoleteBuilding((BuildingTypes)i))
		{
			for (int iLoop = 0; iLoop < city.getNumBuilding((BuildingTypes)i); iLoop++)
			{
				iBuildingMod += infoBuilding.getYieldModifier(eYieldType);
			}
		}
		for (int j = 0; j < MAX_PLAYERS; j++)
		{
			if (GET_PLAYER((PlayerTypes)j).isAlive())
			{
				if (GET_PLAYER((PlayerTypes)j).getTeam() == owner.getTeam())
				{
					int iLoop;
					for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)j).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)j).nextCity(&iLoop))
					{
						if (pLoopCity->getNumBuilding((BuildingTypes)i) > 0 && !GET_TEAM(pLoopCity->getTeam()).isObsoleteBuilding((BuildingTypes)i))
						{
							for (int iLoop = 0; iLoop < pLoopCity->getNumBuilding((BuildingTypes)i); iLoop++)
							{
								iBuildingMod += infoBuilding.getGlobalYieldModifier(eYieldType);
							}
						}
					}
				}
			}
		}
	}
	if (NULL != city.area())
	{
		iBuildingMod += city.area()->getYieldRateModifier(city.getOwnerINLINE(), eYieldType);
	}
	if (0 != iBuildingMod)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BUILDINGS", iBuildingMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseModifier += iBuildingMod;
	}

	// Power
	if (city.isPower())
	{
		int iPowerMod = city.getPowerYieldRateModifier(eYieldType);
		if (0 != iPowerMod)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_POWER", iPowerMod, info.getChar()));
			szBuffer.append(NEWLINE);
			iBaseModifier += iPowerMod;
		}
	}

	// Resources
	int iCityBonusMod = city.getCityBonusYieldRateModifier(eYieldType);
	if (0 != iCityBonusMod)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_BONUS", iCityBonusMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseModifier += iCityBonusMod;
	}

	// Capital
	if (city.isCapital())
	{
		int iCapitalMod = owner.getCapitalYieldRateModifier(eYieldType);
		if (0 != iCapitalMod)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CAPITAL", iCapitalMod, info.getChar()));
			szBuffer.append(NEWLINE);
			iBaseModifier += iCapitalMod;
		}
	}

	// Civics
	int iCivicMod = 0;
	for (int i = 0; i < GC.getNumCivicOptionInfos(); i++)
	{
		if (NO_CIVIC != owner.getCivics((CivicOptionTypes)i))
		{
			iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getYieldModifier(eYieldType);
		}
	}
	if (0 != iCivicMod)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_YIELD_CIVICS", iCivicMod, info.getChar()));
		szBuffer.append(NEWLINE);
		iBaseModifier += iCivicMod;
	}

	FAssertMsg((iBaseModifier * iBaseProduction) / 100 == city.getYieldRate(eYieldType), "Yield Modifier in setProductionHelp does not agree with actual value");
}

void CvGameTextMgr::setConvertHelp(CvWStringBuffer& szBuffer, PlayerTypes ePlayer, ReligionTypes eReligion)
{
	CvWString szReligion = L"TXT_KEY_MISC_NO_STATE_RELIGION";

	if (eReligion != NO_RELIGION)
	{
		szReligion = GC.getReligionInfo(eReligion).getTextKeyWide();
	}

	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CANNOT_CONVERT_TO", szReligion.GetCString()));

	if (GET_PLAYER(ePlayer).isAnarchy())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_WHILE_IN_ANARCHY"));
	}
	else if (GET_PLAYER(ePlayer).getStateReligion() == eReligion)
	{
		szBuffer.append(L". ");
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ALREADY_STATE_REL"));
	}
	else if (GET_PLAYER(ePlayer).getConversionTimer() > 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ANOTHER_REVOLUTION_RECENTLY"));
		szBuffer.append(L". ");
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_WAIT_MORE_TURNS", GET_PLAYER(ePlayer).getConversionTimer()));
	}
}

void CvGameTextMgr::setRevolutionHelp(CvWStringBuffer& szBuffer, PlayerTypes ePlayer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_CANNOT_CHANGE_CIVICS"));

	if (GET_PLAYER(ePlayer).isAnarchy())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_WHILE_IN_ANARCHY"));
	}
	else if (GET_PLAYER(ePlayer).getRevolutionTimer() > 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_ANOTHER_REVOLUTION_RECENTLY"));
		szBuffer.append(L" : ");
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_WAIT_MORE_TURNS", GET_PLAYER(ePlayer).getRevolutionTimer()));
	}
}

void CvGameTextMgr::setVassalRevoltHelp(CvWStringBuffer& szBuffer, TeamTypes eMaster, TeamTypes eVassal)
{
	if (NO_TEAM == eMaster || NO_TEAM == eVassal)
	{
		return;
	}

	if (!GET_TEAM(eVassal).isCapitulated())
	{
		return;
	}

	if (GET_TEAM(eMaster).isParent(eVassal))
	{
		return;
	}

	CvTeam& kMaster = GET_TEAM(eMaster);
	CvTeam& kVassal = GET_TEAM(eVassal);

	int iMasterLand = kMaster.getTotalLand(false);
	int iVassalLand = kVassal.getTotalLand(false);
	if (iMasterLand > 0 && GC.getDefineINT("FREE_VASSAL_LAND_PERCENT") >= 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_VASSAL_LAND_STATS", (iVassalLand * 100) / iMasterLand, GC.getDefineINT("FREE_VASSAL_LAND_PERCENT")));
	}

	int iMasterPop = kMaster.getTotalPopulation(false);
	int iVassalPop = kVassal.getTotalPopulation(false);
	if (iMasterPop > 0 && GC.getDefineINT("FREE_VASSAL_POPULATION_PERCENT") >= 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_VASSAL_POPULATION_STATS", (iVassalPop * 100) / iMasterPop, GC.getDefineINT("FREE_VASSAL_POPULATION_PERCENT")));
	}

	if (GC.getDefineINT("VASSAL_REVOLT_OWN_LOSSES_FACTOR") > 0 && kVassal.getVassalPower() > 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_VASSAL_AREA_LOSS", (iVassalLand * 100) / kVassal.getVassalPower(), GC.getDefineINT("VASSAL_REVOLT_OWN_LOSSES_FACTOR")));
	}

	if (GC.getDefineINT("VASSAL_REVOLT_MASTER_LOSSES_FACTOR") > 0 && kVassal.getMasterPower() > 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_MASTER_AREA_LOSS", (iMasterLand * 100) / kVassal.getMasterPower(), GC.getDefineINT("VASSAL_REVOLT_MASTER_LOSSES_FACTOR")));
	}
}

void CvGameTextMgr::parseGreatPeopleHelp(CvWStringBuffer& szBuffer, CvCity& city)
{
	int iTotalGreatPeopleUnitProgress;
	int iI;

	if (NO_PLAYER == city.getOwnerINLINE())
	{
		return;
	}
	CvPlayer& owner = GET_PLAYER(city.getOwnerINLINE());

	int iReductionTotal = 0;
	/**
		if(city.getGreatPeopleThreshold()>0)
		{
			iReductionTotal+=GC.getDefineINT("CITY_GPPTHRESHOLD_REDUCTION_PER_TURN_BASE");
			iReductionTotal+=city.calculateGPPThresholdReductionByPopulation();
			iReductionTotal+=city.calculateGPPThresholdReductionByCulture();

			iReductionTotal*=100+city.getGPPThresholdReductionPercent();
			iReductionTotal/=100;
		}
	**/
	/** DEBUG
	if(city.getGreatPeopleThreshold()>0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CURRENT_GPPTHRESHOLD", city.getGreatPeopleThreshold()));

		int iReduction=GC.getDefineINT("CITY_GPPTHRESHOLD_REDUCTION_PER_TURN_BASE");
		if(iReduction!=0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CURRENT_GPPTHRESHOLD_DECREASE_BASE",iReduction));
			iReductionTotal+=iReduction;
		}
		iReduction=city.calculateGPPThresholdReductionByPopulation();
		if(iReduction!=0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CURRENT_GPPTHRESHOLD_DECREASE_POP",iReduction));
			iReductionTotal+=iReduction;
		}
		iReduction=city.calculateGPPThresholdReductionByCulture();
		if(iReduction!=0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CURRENT_GPPTHRESHOLD_DECREASE_CULTURE",iReduction));
			iReductionTotal+=iReduction;
		}

		iReductionTotal*=100+city.getGPPThresholdReductionPercent();
		iReductionTotal/=100;
		if(iReductionTotal!=0)
		{
			szBuffer.append(SEPARATOR);
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CURRENT_GPPTHRESHOLD_DECREASE_TOTAL",iReductionTotal));
			szBuffer.append(NEWLINE);
		}
	}


	if(owner.greatPeopleThreshold(false)>0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CURRENT_PLAYER_GPPTHRESHOLD", owner.greatPeopleThreshold(false)));
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_CURRENT_TOTAL_GPPTHRESHOLD", owner.greatPeopleThreshold(false)+city.getGreatPeopleThreshold()));

		szBuffer.append(SEPARATOR);
		szBuffer.append(NEWLINE);
	}
	**/
	int iBestRate = MAX_INT;
	int iValue;
	UnitTypes eNextGreatPerson = NO_UNIT;
	for (iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		if (city.getGreatPeopleUnitRateTotal((UnitTypes)iI) > 0)
		{
			iValue = owner.greatPeopleThreshold(false) + city.getGreatPeopleThreshold() - city.getGreatPeopleUnitProgress((UnitTypes)iI);
			if (iValue > 0)
			{
				int iRate = city.getGreatPeopleUnitRateTotal((UnitTypes)iI);
				if (city.getGreatPeopleThreshold() > 0 && iReductionTotal > 0)
				{
					iRate += iReductionTotal;
				}
				iValue /= iRate;

				if (iValue < iBestRate)
				{
					iBestRate = iValue;
					eNextGreatPerson = (UnitTypes)iI;
				}
			}
		}
	}
	iTotalGreatPeopleUnitProgress = 0;

	for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
	{
		iTotalGreatPeopleUnitProgress += city.getGreatPeopleUnitProgress((UnitTypes)iI);
	}

	if (iTotalGreatPeopleUnitProgress > 0)
	{
		//szBuffer.append(SEPARATOR);
		//szBuffer.append(NEWLINE);

		for (iI = 0; iI < GC.getNumUnitInfos(); ++iI)
		{
			if (city.getGreatPeopleUnitProgress((UnitTypes)iI) > 0)
			{
				int iProgress = city.getGreatPeopleUnitProgress((UnitTypes)iI);
				int iThreshold = city.getGreatPeopleThreshold() + owner.greatPeopleThreshold(false);
				int iRate = city.getGreatPeopleUnitRateTotal((UnitTypes)iI);
				szBuffer.append(CvWString::format(L"%s%s - %d/%d (+%d/turn)", NEWLINE, GC.getUnitInfo((UnitTypes)iI).getDescription(), iProgress, iThreshold, iRate));
			}
		}
	}

	szBuffer.append(SEPARATOR);
	szBuffer.append(NEWLINE);

	if (eNextGreatPerson != NO_UNIT && iBestRate != 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_NEXT_GREAT_PERSON", iBestRate, GC.getUnitInfo(eNextGreatPerson).getDescription()));
		szBuffer.append(NEWLINE);
	}

	bool bCtrl = gDLL->ctrlKey();
	if (iTotalGreatPeopleUnitProgress > 0 && !bCtrl)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_MISC_GREAT_PERSON_SHOW_DETAILS"));
	}
	else if (iTotalGreatPeopleUnitProgress > 0 && bCtrl)
	{
		int iModifier = 100;

		// Buildings
		int iBuildingMod = 0;
		for (int i = 0; i < GC.getNumBuildingInfos(); i++)
		{
			CvBuildingInfo& infoBuilding = GC.getBuildingInfo((BuildingTypes)i);
			if (city.getNumBuilding((BuildingTypes)i) > 0 && !GET_TEAM(city.getTeam()).isObsoleteBuilding((BuildingTypes)i))
			{
				for (int iLoop = 0; iLoop < city.getNumBuilding((BuildingTypes)i); iLoop++)
				{
					iBuildingMod += infoBuilding.getGreatPeopleRateModifier();
				}
			}
			for (int j = 0; j < MAX_PLAYERS; j++)
			{
				if (GET_PLAYER((PlayerTypes)j).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)j).getTeam() == owner.getTeam())
					{
						int iLoop;
						for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)j).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)j).nextCity(&iLoop))
						{
							if (pLoopCity->getNumBuilding((BuildingTypes)i) > 0 && !GET_TEAM(pLoopCity->getTeam()).isObsoleteBuilding((BuildingTypes)i))
							{
								for (int iLoop = 0; iLoop < pLoopCity->getNumBuilding((BuildingTypes)i); iLoop++)
								{
									iBuildingMod += infoBuilding.getGlobalGreatPeopleRateModifier();
								}
							}
						}
					}
				}
			}
		}
		if (0 != iBuildingMod)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_BUILDINGS", iBuildingMod));
			szBuffer.append(NEWLINE);
			iModifier += iBuildingMod;
		}

		// Civics
		int iCivicMod = 0;
		for (int i = 0; i < GC.getNumCivicOptionInfos(); i++)
		{
			if (NO_CIVIC != owner.getCivics((CivicOptionTypes)i))
			{
				iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getGreatPeopleRateModifier();
				iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getGreatPersonRatePerCulture() * city.getCultureLevel();
				if (owner.getStateReligion() != NO_RELIGION && city.isHasReligion(owner.getStateReligion()))
				{
					iCivicMod += GC.getCivicInfo(owner.getCivics((CivicOptionTypes)i)).getStateReligionGreatPeopleRateModifier();
				}
			}
		}
		if (0 != iCivicMod)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_CIVICS", iCivicMod));
			szBuffer.append(NEWLINE);
			iModifier += iCivicMod;
		}

		// Trait
		for (int i = 0; i < GC.getNumTraitInfos(); i++)
		{
			if (city.hasTrait((TraitTypes)i))
			{
				CvTraitInfo& trait = GC.getTraitInfo((TraitTypes)i);
				int iTraitMod = trait.getGreatPeopleRateModifier();
				if (0 != iTraitMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_TRAIT", iTraitMod, trait.getTextKeyWide()));
					szBuffer.append(NEWLINE);
					iModifier += iTraitMod;
				}
			}
		}

		//>>>>	BUGFfH: Added by Denev 2009/09/29
			// Resources
		int iBonusMod = 0;
		for (int iBonus = 0; iBonus < GC.getNumBonusInfos(); iBonus++)
		{
			if (GC.getBonusInfo((BonusTypes)iBonus).getGreatPeopleRateModifier() != 0)
			{
				iBonusMod += GC.getBonusInfo((BonusTypes)iBonus).getGreatPeopleRateModifier() * city.getNumBonuses((BonusTypes)iBonus);
			}
		}
		if (0 != iBonusMod)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_BONUS", iBonusMod));
			szBuffer.append(NEWLINE);
			iModifier += iBonusMod;
		}
		//<<<<	BUGFfH: End Add

		if (owner.isGoldenAge())
		{
			int iGoldenAgeMod = GC.getDefineINT("GOLDEN_AGE_GREAT_PEOPLE_MODIFIER");

			if (0 != iGoldenAgeMod)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_GOLDEN_AGE", iGoldenAgeMod));
				szBuffer.append(NEWLINE);
				iModifier += iGoldenAgeMod;
			}
		}
		/**
				int iReductionMod=city.calculateGPPThresholdReductionByPopulation();
				iReductionMod*=100+city.getGPPThresholdReductionPercent();
				iReductionMod/=city.getGreatPeopleUnitRateTotal(eNextGreatPerson);
				if (0 != iReductionMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_POPULATION", iReductionMod));
					szBuffer.append(NEWLINE);
					iModifier += iReductionMod;
				}

				iReductionMod=city.calculateGPPThresholdReductionByCulture();
				iReductionMod*=100+city.getGPPThresholdReductionPercent();
				iReductionMod/=city.getGreatPeopleUnitRateTotal(eNextGreatPerson);
				if (0 != iReductionMod)
				{
					szBuffer.append(gDLL->getText("TXT_KEY_MISC_HELP_GREATPEOPLE_CULTURE", iReductionMod));
					szBuffer.append(NEWLINE);
					iModifier += iReductionMod;
				}
		**/
	}
}


void CvGameTextMgr::parseGreatGeneralHelp(CvWStringBuffer& szBuffer, CvPlayer& kPlayer)
{
	szBuffer.assign(gDLL->getText("TXT_KEY_MISC_GREAT_GENERAL", kPlayer.getCombatExperience(), kPlayer.greatPeopleThreshold(true)));
}


//------------------------------------------------------------------------------------------------

void CvGameTextMgr::buildCityBillboardIconString(CvWStringBuffer& szBuffer, CvCity* pCity)
{
	szBuffer.clear();

	// government center icon
	if (pCity->isGovernmentCenter() && !(pCity->isCapital()))
	{
		szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(SILVER_STAR_CHAR)));
	}

	// happiness, healthiness, superlative icons
	if (pCity->canBeSelected())
	{
		if (pCity->angryPopulation() > 0)
		{
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(UNHAPPY_CHAR)));
		}

		if (pCity->healthRate() < 0)
		{
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(UNHEALTHY_CHAR)));
		}
		/** Will display Districts now
				if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_DETAIL))
				{
					if (GET_PLAYER(pCity->getOwnerINLINE()).getNumCities() > 2)
					{
						if (pCity->findYieldRateRank(YIELD_PRODUCTION) == 1)
						{
							szBuffer.append(CvWString::format(L"%c", GC.getYieldInfo(YIELD_PRODUCTION).getChar()));
						}
						if (pCity->findCommerceRateRank(COMMERCE_GOLD) == 1)
						{
							szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_GOLD).getChar()));
						}
						if (pCity->findCommerceRateRank(COMMERCE_RESEARCH) == 1)
						{
							szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_RESEARCH).getChar()));
						}
					}
				}
				Will display Districts now **/

		for (int iI = 0; iI < GC.getNumBuildingInfos(); iI++) {
			for (int iPass = 0; iPass < 2; iPass++) {
				if (GC.getBuildingInfo((BuildingTypes)iI).isDistrict() && pCity->getNumBuilding((BuildingTypes)iI)) {
					bool bTier2 = ((GC.getBuildingInfo((BuildingTypes)iI).getPrereqBuildingClass() != NO_BUILDINGCLASS)
						&& (GC.getBuildingInfo((BuildingTypes)GC.getBuildingClassInfo((BuildingClassTypes)GC.getBuildingInfo((BuildingTypes)iI).getPrereqBuildingClass()).getDefaultBuildingIndex()).isDistrict()));
					if ((iPass == 0 && !bTier2)
						|| (iPass == 1 && bTier2)) {
						if (GC.getBuildingInfo((BuildingTypes)iI).getCommerceChange(COMMERCE_MANA) > 0) {
							szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_MANA).getChar()));
						}
						else if (GC.getBuildingInfo((BuildingTypes)iI).getCommerceChange(COMMERCE_FAITH) > 0) {
							szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_FAITH).getChar()));
						}
						else if (GC.getBuildingInfo((BuildingTypes)iI).getCommerceChange(COMMERCE_GOLD) > 0) {
							szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_GOLD).getChar()));
						}
						else if (GC.getBuildingInfo((BuildingTypes)iI).getCommerceChange(COMMERCE_RESEARCH) > 0) {
							szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_RESEARCH).getChar()));
						}
						else if (GC.getBuildingInfo((BuildingTypes)iI).getCommerceChange(COMMERCE_CULTURE) > 0) {
							szBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo(COMMERCE_CULTURE).getChar()));
						}
						else if (GC.getBuildingInfo((BuildingTypes)iI).getMilitaryProductionModifier() > 0) {
							szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(STRENGTH_CHAR)));
						}
					}
				}
			}
		}

		if (pCity->isConnectedToCapital())
		{
			if (GET_PLAYER(pCity->getOwnerINLINE()).countNumCitiesConnectedToCapital() > 1)
			{
				szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(TRADE_CHAR)));
			}
		}
	}

	// religion icons
	for (int iI = 0; iI < GC.getNumReligionInfos(); ++iI)
	{
		if (pCity->isHasReligion((ReligionTypes)iI))
		{

			//FfH: Modified by Kael 11/03/2007
			//			if (pCity->isHolyCity((ReligionTypes)iI))
			//			{
			//				szBuffer.append(CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes) iI).getHolyCityChar()));
			//			}
			//			else
			//			{
			//				szBuffer.append(CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes) iI).getChar()));
			//			}
			if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canSeeReligion(iI, pCity))
			{
				if (pCity->isHolyCity((ReligionTypes)iI))
				{
					szBuffer.append(CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes)iI).getHolyCityChar()));
				}
				else
				{
					szBuffer.append(CvWString::format(L"%c", GC.getReligionInfo((ReligionTypes)iI).getChar()));
				}
			}
			//FfH: End Modify

		}
	}

	//modify Sephi (added check for gameoption)

		// corporation icons
	for (int iI = 0; iI < GC.getNumCorporationInfos(); ++iI)
	{
		//TEMPFIX Sephi
		//if (pCity->isHeadquarters((CorporationTypes)iI))
		bool bFound = false;
		for (int iBLoop = 0; iBLoop < GC.getNumBuildingInfos(); ++iBLoop)
		{
			if (pCity->getNumBuilding((BuildingTypes)iBLoop) > 0)
			{
				if (GC.getBuildingInfo((BuildingTypes)iBLoop).getGlobalCorporationCommerce() == iI)
				{
					bFound = true;
					break;
				}
			}
		}
		if (bFound)
		{
			if (pCity->isHasCorporation((CorporationTypes)iI))
			{
				szBuffer.append(CvWString::format(L"%c", GC.getCorporationInfo((CorporationTypes)iI).getHeadquarterChar()));
			}
		}
		else
		{
			if (pCity->isActiveCorporation((CorporationTypes)iI))
			{
				szBuffer.append(CvWString::format(L"%c", GC.getCorporationInfo((CorporationTypes)iI).getChar()));
			}
		}
	}

	if (pCity->getTeam() == GC.getGameINLINE().getActiveTeam())
	{
		if (pCity->isPower())
		{
			szBuffer.append(CvWString::format(L"%c", gDLL->getSymbolID(POWER_CHAR)));
		}
	}

	// XXX out this in bottom bar???
	if (pCity->isOccupation())
	{
		szBuffer.append(CvWString::format(L" (%c:%d)", gDLL->getSymbolID(OCCUPATION_CHAR), pCity->getOccupationTimer()));
	}

	// defense icon and text
	//if (pCity->getTeam() != GC.getGameINLINE().getActiveTeam())
	{
		if (pCity->isVisible(GC.getGameINLINE().getActiveTeam(), true))
		{
			int iDefenseModifier = pCity->getDefenseModifier(GC.getGameINLINE().selectionListIgnoreBuildingDefense());

			if (iDefenseModifier != 0)
			{
				szBuffer.append(CvWString::format(L" %c:%s%d%%", gDLL->getSymbolID(DEFENSE_CHAR), ((iDefenseModifier > 0) ? "+" : ""), iDefenseModifier));
			}
		}
	}

	//FfH: Added by Kael 07/02/2008
	if (pCity->getCivilizationType() != GET_PLAYER(pCity->getOwnerINLINE()).getCivilizationType())
	{
		szBuffer.append(CvWString::format(L" (%s)", GC.getCivilizationInfo(pCity->getCivilizationType()).getShortDescription()));
	}
	//FfH: End Add

}

void CvGameTextMgr::buildCityBillboardCityNameString(CvWStringBuffer& szBuffer, CvCity* pCity)
{
	szBuffer.assign(pCity->getName());

	if (pCity->canBeSelected())
	{
		if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_DETAIL))
		{
			if (pCity->foodDifference() > 0)
			{
				int iTurns = pCity->getFoodTurnsLeft();

				if ((iTurns > 1) || !(pCity->AI_isEmphasizeAvoidGrowth()))
				{
					if (iTurns < MAX_INT)
					{
						szBuffer.append(CvWString::format(L" (%d)", iTurns));
					}
				}
			}
		}
	}

	//FfH: Added by Kael 11/07/2007
	if (pCity->isSettlement())
	{
		szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_SETTLEMENT_TAG"));
	}
	//FfH: End Add

}

void CvGameTextMgr::buildCityBillboardProductionString(CvWStringBuffer& szBuffer, CvCity* pCity)
{
	if (pCity->getOrderQueueLength() > 0)
	{
		szBuffer.assign(pCity->getProductionName());

		if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_DETAIL))
		{
			int iTurns = pCity->getProductionTurnsLeft();

			if (iTurns < MAX_INT)
			{
				szBuffer.append(CvWString::format(L" (%d)", iTurns));
			}
		}
	}
	else
	{
		szBuffer.clear();
	}
}


void CvGameTextMgr::buildCityBillboardCitySizeString(CvWStringBuffer& szBuffer, CvCity* pCity, const NiColorA& kColor)
{
#define CAPARAMS(c) (int)((c).r * 255.0f), (int)((c).g * 255.0f), (int)((c).b * 255.0f), (int)((c).a * 255.0f)
	szBuffer.assign(CvWString::format(SETCOLR L"%d" ENDCOLR, CAPARAMS(kColor), pCity->getPopulation()));
#undef CAPARAMS
}

void CvGameTextMgr::getCityBillboardFoodbarColors(CvCity* pCity, std::vector<NiColorA>& aColors)
{
	aColors.resize(NUM_INFOBAR_TYPES);
	aColors[INFOBAR_STORED] = GC.getColorInfo((ColorTypes)(GC.getYieldInfo(YIELD_FOOD).getColorType())).getColor();
	aColors[INFOBAR_RATE] = aColors[INFOBAR_STORED];
	aColors[INFOBAR_RATE].a = 0.5f;
	aColors[INFOBAR_RATE_EXTRA] = GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_NEGATIVE_RATE")).getColor();
	aColors[INFOBAR_EMPTY] = GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_EMPTY")).getColor();
}

void CvGameTextMgr::getCityBillboardProductionbarColors(CvCity* pCity, std::vector<NiColorA>& aColors)
{
	aColors.resize(NUM_INFOBAR_TYPES);
	aColors[INFOBAR_STORED] = GC.getColorInfo((ColorTypes)(GC.getYieldInfo(YIELD_PRODUCTION).getColorType())).getColor();
	aColors[INFOBAR_RATE] = aColors[INFOBAR_STORED];
	aColors[INFOBAR_RATE].a = 0.5f;
	aColors[INFOBAR_RATE_EXTRA] = GC.getColorInfo((ColorTypes)(GC.getYieldInfo(YIELD_FOOD).getColorType())).getColor();
	aColors[INFOBAR_RATE_EXTRA].a = 0.5f;
	aColors[INFOBAR_EMPTY] = GC.getColorInfo((ColorTypes)GC.getInfoTypeForString("COLOR_EMPTY")).getColor();
}


void CvGameTextMgr::setScoreHelp(CvWStringBuffer& szString, PlayerTypes ePlayer)
{
	if (NO_PLAYER != ePlayer)
	{
		CvPlayer& player = GET_PLAYER(ePlayer);

		int iPop = player.getPopScore();
		int iMaxPop = GC.getGameINLINE().getMaxPopulation();
		int iPopScore = 0;
		if (iMaxPop > 0)
		{
			iPopScore = (GC.getDefineINT("SCORE_POPULATION_FACTOR") * iPop) / iMaxPop;
		}
		int iLand = player.getLandScore();
		int iMaxLand = GC.getGameINLINE().getMaxLand();
		int iLandScore = 0;
		if (iMaxLand > 0)
		{
			iLandScore = (GC.getDefineINT("SCORE_LAND_FACTOR") * iLand) / iMaxLand;
		}
		int iTech = player.getTechScore();
		int iMaxTech = GC.getGameINLINE().getMaxTech();
		int iTechScore = (GC.getDefineINT("SCORE_TECH_FACTOR") * iTech) / iMaxTech;
		int iWonders = player.getWondersScore();
		int iMaxWonders = GC.getGameINLINE().getMaxWonders();
		int iWondersScore = (GC.getDefineINT("SCORE_WONDER_FACTOR") * iWonders) / iMaxWonders;
		int iTotalScore = iPopScore + iLandScore + iTechScore + iWondersScore;
		int iVictoryScore = player.calculateScore(true, true);
		if (iTotalScore == player.calculateScore())
		{
			szString.append(gDLL->getText("TXT_KEY_SCORE_BREAKDOWN", iPopScore, iPop, iMaxPop, iLandScore, iLand, iMaxLand, iTechScore, iTech, iMaxTech, iWondersScore, iWonders, iMaxWonders, iTotalScore, iVictoryScore));
		}
	}
}

void CvGameTextMgr::setEventHelp(CvWStringBuffer& szBuffer, EventTypes eEvent, int iEventTriggeredId, PlayerTypes ePlayer)
{
	if (NO_EVENT == eEvent || NO_PLAYER == ePlayer)
	{
		return;
	}

	CvEventInfo& kEvent = GC.getEventInfo(eEvent);
	CvPlayer& kActivePlayer = GET_PLAYER(ePlayer);
	EventTriggeredData* pTriggeredData = kActivePlayer.getEventTriggered(iEventTriggeredId);

	if (NULL == pTriggeredData)
	{
		return;
	}

	CvCity* pCity = kActivePlayer.getCity(pTriggeredData->m_iCityId);
	CvCity* pOtherPlayerCity = NULL;
	CvPlot* pPlot = GC.getMapINLINE().plot(pTriggeredData->m_iPlotX, pTriggeredData->m_iPlotY);
	CvUnit* pUnit = kActivePlayer.getUnit(pTriggeredData->m_iUnitId);

	if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
	{
		pOtherPlayerCity = GET_PLAYER(pTriggeredData->m_eOtherPlayer).getCity(pTriggeredData->m_iOtherPlayerCityId);
	}

	CvWString szCity = gDLL->getText("TXT_KEY_EVENT_THE_CITY");
	if (NULL != pCity && kEvent.isCityEffect())
	{
		szCity = pCity->getNameKey();
	}
	else if (NULL != pOtherPlayerCity && kEvent.isOtherPlayerCityEffect())
	{
		szCity = pOtherPlayerCity->getNameKey();
	}

	CvWString szUnit = gDLL->getText("TXT_KEY_EVENT_THE_UNIT");
	if (NULL != pUnit)
	{
		szUnit = pUnit->getNameKey();
	}

	CvWString szReligion = gDLL->getText("TXT_KEY_EVENT_THE_RELIGION");
	if (NO_RELIGION != pTriggeredData->m_eReligion)
	{
		szReligion = GC.getReligionInfo(pTriggeredData->m_eReligion).getTextKeyWide();
	}

	eventGoldHelp(szBuffer, eEvent, ePlayer, pTriggeredData->m_eOtherPlayer);

	eventTechHelp(szBuffer, eEvent, kActivePlayer.getBestEventTech(eEvent, pTriggeredData->m_eOtherPlayer), ePlayer, pTriggeredData->m_eOtherPlayer);

	if (NO_PLAYER != pTriggeredData->m_eOtherPlayer && NO_BONUS != kEvent.getBonusGift())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GIFT_BONUS_TO_PLAYER", GC.getBonusInfo((BonusTypes)kEvent.getBonusGift()).getTextKeyWide(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
	}

	if (kEvent.getHappy() != 0)
	{
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			if (kEvent.getHappy() > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY_FROM_PLAYER", kEvent.getHappy(), kEvent.getHappy(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY_TO_PLAYER", -kEvent.getHappy(), -kEvent.getHappy(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
			}
		}
		else
		{
			if (kEvent.getHappy() > 0)
			{
				if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY_CITY", kEvent.getHappy(), szCity.GetCString()));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY", kEvent.getHappy()));
				}
			}
			else
			{
				if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNHAPPY_CITY", -kEvent.getHappy(), szCity.GetCString()));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNHAPPY", -kEvent.getHappy()));
				}
			}
		}
	}

	if (kEvent.getHealth() != 0)
	{
		if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
		{
			if (kEvent.getHealth() > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HEALTH_FROM_PLAYER", kEvent.getHealth(), kEvent.getHealth(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HEALTH_TO_PLAYER", -kEvent.getHealth(), -kEvent.getHealth(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
			}
		}
		else
		{
			if (kEvent.getHealth() > 0)
			{
				if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HEALTH_CITY", kEvent.getHealth(), szCity.GetCString()));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HEALTH", kEvent.getHealth()));
				}
			}
			else
			{
				if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNHEALTH", -kEvent.getHealth(), szCity.GetCString()));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNHEALTH_CITY", -kEvent.getHealth()));
				}
			}
		}
	}

	if (kEvent.getHurryAnger() != 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HURRY_ANGER_CITY", kEvent.getHurryAnger(), szCity.GetCString()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HURRY_ANGER", kEvent.getHurryAnger()));
		}
	}

	if (kEvent.getHappyTurns() > 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_TEMP_HAPPY_CITY", GC.getDefineINT("TEMP_HAPPY"), kEvent.getHappyTurns(), szCity.GetCString()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_TEMP_HAPPY", GC.getDefineINT("TEMP_HAPPY"), kEvent.getHappyTurns()));
		}
	}

	if (kEvent.getFood() != 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FOOD_CITY", kEvent.getFood(), szCity.GetCString()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FOOD", kEvent.getFood()));
		}
	}

	if (kEvent.getFoodPercent() != 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FOOD_PERCENT_CITY", kEvent.getFoodPercent(), szCity.GetCString()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FOOD_PERCENT", kEvent.getFoodPercent()));
		}
	}

	if (kEvent.getRevoltTurns() > 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_REVOLT_TURNS", kEvent.getRevoltTurns(), szCity.GetCString()));
		}
	}

	if (0 != kEvent.getSpaceProductionModifier())
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_SPACE_PRODUCTION_CITY", kEvent.getSpaceProductionModifier(), szCity.GetCString()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_SPACESHIP_MOD_ALL_CITIES", kEvent.getSpaceProductionModifier()));
		}
	}

	if (kEvent.getMaxPillage() > 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			if (kEvent.getMaxPillage() == kEvent.getMinPillage())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_PILLAGE_CITY", kEvent.getMinPillage(), szCity.GetCString()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_PILLAGE_RANGE_CITY", kEvent.getMinPillage(), kEvent.getMaxPillage(), szCity.GetCString()));
			}
		}
		else
		{
			if (kEvent.getMaxPillage() == kEvent.getMinPillage())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_PILLAGE", kEvent.getMinPillage()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_PILLAGE_RANGE", kEvent.getMinPillage(), kEvent.getMaxPillage()));
			}
		}
	}

	for (int i = 0; i < GC.getNumSpecialistInfos(); ++i)
	{
		if (kEvent.getFreeSpecialistCount(i) > 0)
		{
			if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FREE_SPECIALIST", kEvent.getFreeSpecialistCount(i), GC.getSpecialistInfo((SpecialistTypes)i).getTextKeyWide(), szCity.GetCString()));
			}
		}
	}

	if (kEvent.getPopulationChange() != 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_POPULATION_CHANGE_CITY", kEvent.getPopulationChange(), szCity.GetCString()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_POPULATION_CHANGE", kEvent.getPopulationChange()));
		}
	}

	if (kEvent.getCulture() != 0)
	{
		if (kEvent.isCityEffect() || kEvent.isOtherPlayerCityEffect())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_CULTURE_CITY", kEvent.getCulture(), szCity.GetCString()));
		}
		else
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_CULTURE", kEvent.getCulture()));
		}
	}

	if (kEvent.getUnitClass() != NO_UNITCLASS)
	{
		CivilizationTypes eCiv = kActivePlayer.getCivilizationType();
		if (NO_CIVILIZATION != eCiv)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(eCiv).getCivilizationUnits(kEvent.getUnitClass());
			if (eUnit != NO_UNIT)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_BONUS_UNIT", kEvent.getNumUnits(), GC.getUnitInfo(eUnit).getTextKeyWide()));
			}
		}
	}

	if (kEvent.getBuildingClass() != NO_BUILDINGCLASS)
	{
		CivilizationTypes eCiv = kActivePlayer.getCivilizationType();
		if (NO_CIVILIZATION != eCiv)
		{
			BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(eCiv).getCivilizationBuildings(kEvent.getBuildingClass());
			if (eBuilding != NO_BUILDING)
			{
				if (kEvent.getBuildingChange() > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_BONUS_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide()));
				}
				else if (kEvent.getBuildingChange() < 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_REMOVE_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide()));
				}
			}
		}
	}

	if (kEvent.getNumBuildingYieldChanges() > 0)
	{
		CvWStringBuffer szYield;
		for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
		{
			CivilizationTypes eCiv = kActivePlayer.getCivilizationType();
			if (NO_CIVILIZATION != eCiv)
			{
				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(eCiv).getCivilizationBuildings(iBuildingClass);
				if (eBuilding != NO_BUILDING)
				{
					int aiYields[NUM_YIELD_TYPES];
					for (int iYield = 0; iYield < NUM_YIELD_TYPES; ++iYield)
					{
						aiYields[iYield] = kEvent.getBuildingYieldChange(iBuildingClass, iYield);
					}

					szYield.clear();
					setYieldChangeHelp(szYield, L"", L"", L"", aiYields, false, false);
					if (!szYield.isEmpty())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_EVENT_YIELD_CHANGE_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide(), szYield.getCString()));
					}
				}
			}
		}
	}

	if (kEvent.getNumBuildingCommerceChanges() > 0)
	{
		CvWStringBuffer szCommerce;
		for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
		{
			CivilizationTypes eCiv = kActivePlayer.getCivilizationType();
			if (NO_CIVILIZATION != eCiv)
			{
				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(eCiv).getCivilizationBuildings(iBuildingClass);
				if (eBuilding != NO_BUILDING)
				{
					int aiCommerces[NUM_COMMERCE_TYPES];
					for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; ++iCommerce)
					{
						aiCommerces[iCommerce] = kEvent.getBuildingCommerceChange(iBuildingClass, iCommerce);
					}

					szCommerce.clear();
					setCommerceChangeHelp(szCommerce, L"", L"", L"", aiCommerces, false, false);
					if (!szCommerce.isEmpty())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_EVENT_YIELD_CHANGE_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide(), szCommerce.getCString()));
					}
				}
			}
		}
	}

	if (kEvent.getNumBuildingHappyChanges() > 0)
	{
		for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
		{
			CivilizationTypes eCiv = kActivePlayer.getCivilizationType();
			if (NO_CIVILIZATION != eCiv)
			{
				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(eCiv).getCivilizationBuildings(iBuildingClass);
				if (eBuilding != NO_BUILDING)
				{
					int iHappy = kEvent.getBuildingHappyChange(iBuildingClass);
					if (iHappy > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide(), iHappy, gDLL->getSymbolID(HAPPY_CHAR)));
					}
					else if (iHappy < 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide(), -iHappy, gDLL->getSymbolID(UNHAPPY_CHAR)));
					}
				}
			}
		}
	}

	if (kEvent.getNumBuildingHealthChanges() > 0)
	{
		for (int iBuildingClass = 0; iBuildingClass < GC.getNumBuildingClassInfos(); ++iBuildingClass)
		{
			CivilizationTypes eCiv = kActivePlayer.getCivilizationType();
			if (NO_CIVILIZATION != eCiv)
			{
				BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(eCiv).getCivilizationBuildings(iBuildingClass);
				if (eBuilding != NO_BUILDING)
				{
					int iHealth = kEvent.getBuildingHealthChange(iBuildingClass);
					if (iHealth > 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide(), iHealth, gDLL->getSymbolID(HEALTHY_CHAR)));
					}
					else if (iHealth < 0)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_EVENT_HAPPY_BUILDING", GC.getBuildingInfo(eBuilding).getTextKeyWide(), -iHealth, gDLL->getSymbolID(UNHEALTHY_CHAR)));
					}
				}
			}
		}
	}

	if (kEvent.getFeatureChange() > 0)
	{
		if (kEvent.getFeature() != NO_FEATURE)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FEATURE_GROWTH", GC.getFeatureInfo((FeatureTypes)kEvent.getFeature()).getTextKeyWide()));
		}
	}
	else if (kEvent.getFeatureChange() < 0)
	{
		if (NULL != pPlot && NO_FEATURE != pPlot->getFeatureType())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FEATURE_REMOVE", GC.getFeatureInfo(pPlot->getFeatureType()).getTextKeyWide()));
		}
	}

	if (kEvent.getImprovementChange() > 0)
	{
		if (kEvent.getImprovement() != NO_IMPROVEMENT)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_IMPROVEMENT_GROWTH", GC.getImprovementInfo((ImprovementTypes)kEvent.getImprovement()).getTextKeyWide()));
		}
	}
	else if (kEvent.getImprovementChange() < 0)
	{
		if (NULL != pPlot && NO_IMPROVEMENT != pPlot->getImprovementType())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_IMPROVEMENT_REMOVE", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide()));
		}
	}

	if (kEvent.getBonusChange() > 0)
	{
		if (kEvent.getBonus() != NO_BONUS)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_BONUS_GROWTH", GC.getBonusInfo((BonusTypes)kEvent.getBonus()).getTextKeyWide()));
		}
	}
	else if (kEvent.getBonusChange() < 0)
	{
		if (NULL != pPlot && NO_BONUS != pPlot->getBonusType())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_BONUS_REMOVE", GC.getBonusInfo(pPlot->getBonusType()).getTextKeyWide()));
		}
	}

	if (kEvent.getRouteChange() > 0)
	{
		if (kEvent.getRoute() != NO_ROUTE)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ROUTE_GROWTH", GC.getRouteInfo((RouteTypes)kEvent.getRoute()).getTextKeyWide()));
		}
	}
	else if (kEvent.getRouteChange() < 0)
	{
		if (NULL != pPlot && NO_ROUTE != pPlot->getRouteType())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ROUTE_REMOVE", GC.getRouteInfo(pPlot->getRouteType()).getTextKeyWide()));
		}
	}

	int aiYields[NUM_YIELD_TYPES];
	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		aiYields[i] = kEvent.getPlotExtraYield(i);
	}

	CvWStringBuffer szYield;
	setYieldChangeHelp(szYield, L"", L"", L"", aiYields, false, false);
	if (!szYield.isEmpty())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_YIELD_CHANGE_PLOT", szYield.getCString()));
	}

	if (NO_BONUS != kEvent.getBonusRevealed())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_BONUS_REVEALED", GC.getBonusInfo((BonusTypes)kEvent.getBonusRevealed()).getTextKeyWide()));
	}

	if (0 != kEvent.getUnitExperience())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNIT_EXPERIENCE", kEvent.getUnitExperience(), szUnit.GetCString()));
	}

	if (0 != kEvent.isDisbandUnit())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNIT_DISBAND", szUnit.GetCString()));
	}

	if (NO_PROMOTION != kEvent.getUnitPromotion())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNIT_PROMOTION", szUnit.GetCString(), GC.getPromotionInfo((PromotionTypes)kEvent.getUnitPromotion()).getTextKeyWide()));
	}

	for (int i = 0; i < GC.getNumUnitCombatInfos(); ++i)
	{
		if (NO_PROMOTION != kEvent.getUnitCombatPromotion(i))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNIT_COMBAT_PROMOTION", GC.getUnitCombatInfo((UnitCombatTypes)i).getTextKeyWide(), GC.getPromotionInfo((PromotionTypes)kEvent.getUnitCombatPromotion(i)).getTextKeyWide()));
		}
	}

	for (int i = 0; i < GC.getNumUnitClassInfos(); ++i)
	{
		if (NO_PROMOTION != kEvent.getUnitClassPromotion(i))
		{
			UnitTypes ePromotedUnit = ((UnitTypes)(GC.getCivilizationInfo(kActivePlayer.getCivilizationType()).getCivilizationUnits(i)));
			if (NO_UNIT != ePromotedUnit)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_UNIT_CLASS_PROMOTION", GC.getUnitInfo(ePromotedUnit).getTextKeyWide(), GC.getPromotionInfo((PromotionTypes)kEvent.getUnitClassPromotion(i)).getTextKeyWide()));
			}
		}
	}

	if (kEvent.getConvertOwnCities() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_CONVERT_OWN_CITIES", kEvent.getConvertOwnCities(), szReligion.GetCString()));
	}

	if (kEvent.getConvertOtherCities() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_CONVERT_OTHER_CITIES", kEvent.getConvertOtherCities(), szReligion.GetCString()));
	}

	if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
	{
		if (kEvent.getAttitudeModifier() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ATTITUDE_GOOD", kEvent.getAttitudeModifier(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
		}
		else if (kEvent.getAttitudeModifier() < 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ATTITUDE_BAD", kEvent.getAttitudeModifier(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
		}
	}

	if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
	{
		TeamTypes eWorstEnemy = GET_TEAM(GET_PLAYER(pTriggeredData->m_eOtherPlayer).getTeam()).AI_getWorstEnemy();
		if (NO_TEAM != eWorstEnemy)
		{
			if (kEvent.getTheirEnemyAttitudeModifier() > 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ATTITUDE_GOOD", kEvent.getTheirEnemyAttitudeModifier(), GET_TEAM(eWorstEnemy).getName().GetCString()));
			}
			else if (kEvent.getTheirEnemyAttitudeModifier() < 0)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ATTITUDE_BAD", kEvent.getTheirEnemyAttitudeModifier(), GET_TEAM(eWorstEnemy).getName().GetCString()));
			}
		}
	}

	if (NO_PLAYER != pTriggeredData->m_eOtherPlayer)
	{
		if (kEvent.getEspionagePoints() > 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ESPIONAGE_POINTS", kEvent.getEspionagePoints(), GET_PLAYER(pTriggeredData->m_eOtherPlayer).getNameKey()));
		}
		else if (kEvent.getEspionagePoints() < 0)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ESPIONAGE_COST", -kEvent.getEspionagePoints()));
		}
	}

	if (kEvent.isGoldenAge())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLDEN_AGE"));
	}

	if (0 != kEvent.getFreeUnitSupport())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_FREE_UNIT_SUPPORT", kEvent.getFreeUnitSupport()));
	}

	if (0 != kEvent.getInflationModifier())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_INFLATION_MODIFIER", kEvent.getInflationModifier()));
	}

	if (kEvent.isDeclareWar())
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_DECLARE_WAR", GET_PLAYER(pTriggeredData->m_eOtherPlayer).getCivilizationAdjectiveKey()));
	}

	if (kEvent.getUnitImmobileTurns() > 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_IMMOBILE_UNIT", kEvent.getUnitImmobileTurns(), szUnit.GetCString()));
	}

	//FfH: Added by Kael 01/21/2008
	if (kEvent.getCrime() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_CRIME", kEvent.getCrime()));
	}
	if (kEvent.getGlobalCounter() != 0)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GLOBAL_COUNTER", kEvent.getGlobalCounter()));
	}
	//FfH: End Add

	if (!CvWString(kEvent.getPythonHelp()).empty())
	{
		CvWString szHelp;
		CyArgsList argsList;
		argsList.add(eEvent);
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pTriggeredData));

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kEvent.getPythonHelp(), argsList.makeFunctionArgs(), &szHelp);

		szBuffer.append(NEWLINE);
		szBuffer.append(szHelp);
	}

	CvWStringBuffer szTemp;
	for (int i = 0; i < GC.getNumEventInfos(); ++i)
	{
		if (0 == kEvent.getAdditionalEventTime(i))
		{
			if (kEvent.getAdditionalEventChance(i) > 0)
			{
				if (GET_PLAYER(GC.getGameINLINE().getActivePlayer()).canDoEvent((EventTypes)i, *pTriggeredData))
				{
					szTemp.clear();
					setEventHelp(szTemp, (EventTypes)i, iEventTriggeredId, ePlayer);

					if (!szTemp.isEmpty())
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ADDITIONAL_CHANCE", kEvent.getAdditionalEventChance(i), L""));
						szBuffer.append(NEWLINE);
						szBuffer.append(szTemp);
					}
				}
			}
		}
		else
		{
			szTemp.clear();
			setEventHelp(szTemp, (EventTypes)i, iEventTriggeredId, ePlayer);

			if (!szTemp.isEmpty())
			{
				CvWString szDelay = gDLL->getText("TXT_KEY_EVENT_DELAY_TURNS", (GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getGrowthPercent() * kEvent.getAdditionalEventTime((EventTypes)i)) / 100);

				if (kEvent.getAdditionalEventChance(i) > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_ADDITIONAL_CHANCE", kEvent.getAdditionalEventChance(i), szDelay.GetCString()));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_DELAY", szDelay.GetCString()));
				}

				szBuffer.append(NEWLINE);
				szBuffer.append(szTemp);
			}
		}
	}

	if (NO_TECH != kEvent.getPrereqTech())
	{
		if (!GET_TEAM(kActivePlayer.getTeam()).isHasTech((TechTypes)kEvent.getPrereqTech()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getTechInfo((TechTypes)(kEvent.getPrereqTech())).getTextKeyWide()));
		}
	}

	//FfH: Added by Kael 01/21/2008
	if (kEvent.getPrereqAlignment() != NO_ALIGNMENT)
	{
		if (kActivePlayer.getAlignment() != kEvent.getPrereqAlignment())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getAlignmentInfo((AlignmentTypes)kEvent.getPrereqAlignment()).getDescription()));
		}
	}
	if (kEvent.getPrereqBonus() != NO_BONUS)
	{
		if (!kActivePlayer.hasBonus((BonusTypes)kEvent.getPrereqBonus()))
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getBonusInfo((BonusTypes)kEvent.getPrereqBonus()).getDescription()));
		}
	}
	if (kEvent.getPrereqCivilization() != NO_CIVILIZATION)
	{
		if (kActivePlayer.getCivilizationType() != kEvent.getPrereqCivilization())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getCivilizationInfo((CivilizationTypes)kEvent.getPrereqCivilization()).getDescription()));
		}
	}
	if (kEvent.getPrereqCorporation() != NO_CORPORATION)
	{
		if (pCity != NULL)
		{
			if (!pCity->isHasCorporation((CorporationTypes)kEvent.getPrereqCorporation()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getCorporationInfo((CorporationTypes)kEvent.getPrereqCorporation()).getDescription()));
			}
		}
	}
	if (kEvent.getPrereqReligion() != NO_RELIGION)
	{
		if (pCity != NULL)
		{
			if (!pCity->isHasReligion((ReligionTypes)kEvent.getPrereqReligion()))
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_BUILDING_REQUIRES_STRING", GC.getReligionInfo((ReligionTypes)(kEvent.getPrereqReligion())).getDescription()));
			}
		}
	}
	if (kEvent.getPrereqStateReligion() != NO_RELIGION)
	{
		if (kActivePlayer.getStateReligion() != kEvent.getPrereqStateReligion())
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_CIVIC_REQUIRES", GC.getReligionInfo((ReligionTypes)kEvent.getPrereqStateReligion()).getDescription()));
		}
	}
	//FfH: End Add

	bool done = false;
	while (!done)
	{
		done = true;
		if (!szBuffer.isEmpty())
		{
			const wchar* wideChar = szBuffer.getCString();
			if (wideChar[0] == L'\n')
			{
				CvWString tempString(&wideChar[1]);
				szBuffer.clear();
				szBuffer.append(tempString);
				done = false;
			}
		}
	}
}

void CvGameTextMgr::eventTechHelp(CvWStringBuffer& szBuffer, EventTypes eEvent, TechTypes eTech, PlayerTypes eActivePlayer, PlayerTypes eOtherPlayer)
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (eTech != NO_TECH)
	{
		if (100 == kEvent.getTechPercent())
		{
			if (NO_PLAYER != eOtherPlayer)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_TECH_GAINED_FROM_PLAYER", GC.getTechInfo(eTech).getTextKeyWide(), GET_PLAYER(eOtherPlayer).getNameKey()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_TECH_GAINED", GC.getTechInfo(eTech).getTextKeyWide()));
			}
		}
		else if (0 != kEvent.getTechPercent())
		{
			CvTeam& kTeam = GET_TEAM(GET_PLAYER(eActivePlayer).getTeam());
			int iBeakers = (kTeam.getResearchCost(eTech) * kEvent.getTechPercent()) / 100;
			if (kEvent.getTechPercent() > 0)
			{
				iBeakers = std::min(kTeam.getResearchLeft(eTech), iBeakers);
			}
			else if (kEvent.getTechPercent() < 0)
			{
				iBeakers = std::max(kTeam.getResearchLeft(eTech) - kTeam.getResearchCost(eTech), iBeakers);
			}

			if (NO_PLAYER != eOtherPlayer)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_TECH_GAINED_FROM_PLAYER_PERCENT", iBeakers, GC.getTechInfo(eTech).getTextKeyWide(), GET_PLAYER(eOtherPlayer).getNameKey()));
			}
			else
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_EVENT_TECH_GAINED_PERCENT", iBeakers, GC.getTechInfo(eTech).getTextKeyWide()));
			}
		}
	}
}

void CvGameTextMgr::eventGoldHelp(CvWStringBuffer& szBuffer, EventTypes eEvent, PlayerTypes ePlayer, PlayerTypes eOtherPlayer)
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	int iGold1 = kPlayer.getEventCost(eEvent, eOtherPlayer, false);
	int iGold2 = kPlayer.getEventCost(eEvent, eOtherPlayer, true);

	if (0 != iGold1 || 0 != iGold2)
	{
		if (iGold1 == iGold2)
		{
			if (NO_PLAYER != eOtherPlayer && kEvent.isGoldToPlayer())
			{
				if (iGold1 > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_FROM_PLAYER", iGold1, GET_PLAYER(eOtherPlayer).getNameKey()));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_TO_PLAYER", -iGold1, GET_PLAYER(eOtherPlayer).getNameKey()));
				}
			}
			else
			{
				if (iGold1 > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_GAINED", iGold1));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_LOST", -iGold1));
				}
			}
		}
		else
		{
			if (NO_PLAYER != eOtherPlayer && kEvent.isGoldToPlayer())
			{
				if (iGold1 > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_RANGE_FROM_PLAYER", iGold1, iGold2, GET_PLAYER(eOtherPlayer).getNameKey()));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_RANGE_TO_PLAYER", -iGold1, -iGold2, GET_PLAYER(eOtherPlayer).getNameKey()));
				}
			}
			else
			{
				if (iGold1 > 0)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_RANGE_GAINED", iGold1, iGold2));
				}
				else
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_EVENT_GOLD_RANGE_LOST", -iGold1, iGold2));
				}
			}
		}
	}
}

void CvGameTextMgr::setTradeRouteHelp(CvWStringBuffer& szBuffer, int iRoute, CvCity* pCity)
{
	if (NULL != pCity)
	{
		CvCity* pOtherCity = pCity->getTradeCity(iRoute);

		if (NULL != pOtherCity)
		{
			szBuffer.append(pOtherCity->getName());

			int iProfit = pCity->getBaseTradeProfit(pOtherCity);

			szBuffer.append(NEWLINE);
			CvWString szBaseProfit;
			szBaseProfit.Format(L"%d.%02d", iProfit / 100, iProfit % 100);
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_HELP_BASE", szBaseProfit.GetCString()));

			int iModifier = 100;
			int iNewMod;

			for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); ++iBuilding)
			{
				if (pCity->getNumActiveBuilding((BuildingTypes)iBuilding) > 0)
				{
					iNewMod = pCity->getNumActiveBuilding((BuildingTypes)iBuilding) * GC.getBuildingInfo((BuildingTypes)iBuilding).getTradeRouteModifier();
					if (0 != iNewMod)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_MOD_BUILDING", GC.getBuildingInfo((BuildingTypes)iBuilding).getTextKeyWide(), iNewMod));
						iModifier += iNewMod;
					}
				}
			}

			iNewMod = pCity->getPopulationTradeModifier();
			if (0 != iNewMod)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_MOD_POPULATION", iNewMod));
				iModifier += iNewMod;
			}

			if (pCity->isConnectedToCapital())
			{
				iNewMod = GC.getDefineINT("CAPITAL_TRADE_MODIFIER");
				if (0 != iNewMod)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_MOD_CAPITAL", iNewMod));
					iModifier += iNewMod;
				}
			}

			if (NULL != pOtherCity)
			{
				if (pCity->area() != pOtherCity->area())
				{
					iNewMod = GC.getDefineINT("OVERSEAS_TRADE_MODIFIER");
					if (0 != iNewMod)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_MOD_OVERSEAS", iNewMod));
						iModifier += iNewMod;
					}
				}

				if (pCity->getTeam() != pOtherCity->getTeam())
				{
					iNewMod = pCity->getForeignTradeRouteModifier();
					if (0 != iNewMod)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_MOD_FOREIGN", iNewMod));
						iModifier += iNewMod;
					}

					iNewMod = pCity->getPeaceTradeModifier(pOtherCity->getTeam());
					if (0 != iNewMod)
					{
						szBuffer.append(NEWLINE);
						szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_MOD_PEACE", iNewMod));
						iModifier += iNewMod;
					}
				}
			}

			FAssert(pCity->totalTradeModifier(pOtherCity) == iModifier);

			iProfit *= iModifier;
			iProfit /= 10000;

			FAssert(iProfit == pCity->calculateTradeProfit(pOtherCity));

			szBuffer.append(SEPARATOR);

			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_TRADE_ROUTE_TOTAL", iProfit));
		}
	}
}

void CvGameTextMgr::setEspionageCostHelp(CvWStringBuffer& szBuffer, EspionageMissionTypes eMission, PlayerTypes eTargetPlayer, const CvPlot* pPlot, int iExtraData, const CvUnit* pSpyUnit)
{
	CvPlayer& kPlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());
	CvEspionageMissionInfo& kMission = GC.getEspionageMissionInfo(eMission);

	//szBuffer.assign(kMission.getDescription());

	int iMissionCost = kPlayer.getEspionageMissionBaseCost(eMission, eTargetPlayer, pPlot, iExtraData, pSpyUnit);

	if (kMission.isDestroyImprovement())
	{
		if (NULL != pPlot && NO_IMPROVEMENT != pPlot->getImprovementType())
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_DESTROY_IMPROVEMENT", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide()));
			szBuffer.append(NEWLINE);
		}
	}

	if (kMission.getDestroyBuildingCostFactor() > 0)
	{
		BuildingTypes eTargetBuilding = (BuildingTypes)iExtraData;

		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_DESTROY_IMPROVEMENT", GC.getBuildingInfo(eTargetBuilding).getTextKeyWide()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getDestroyProjectCostFactor() > 0)
	{
		ProjectTypes eTargetProject = (ProjectTypes)iExtraData;

		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_DESTROY_IMPROVEMENT", GC.getProjectInfo(eTargetProject).getTextKeyWide()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getDestroyProductionCostFactor() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_DESTROY_PRODUCTION", pCity->getProduction()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getDestroyUnitCostFactor() > 0)
	{
		if (NO_PLAYER != eTargetPlayer)
		{
			int iTargetUnitID = iExtraData;

			CvUnit* pUnit = GET_PLAYER(eTargetPlayer).getUnit(iTargetUnitID);

			if (NULL != pUnit)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_DESTROY_UNIT", pUnit->getNameKey()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getBuyUnitCostFactor() > 0)
	{
		if (NO_PLAYER != eTargetPlayer)
		{
			int iTargetUnitID = iExtraData;

			CvUnit* pUnit = GET_PLAYER(eTargetPlayer).getUnit(iTargetUnitID);

			if (NULL != pUnit)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_BRIBE", pUnit->getNameKey()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getBuyCityCostFactor() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_BRIBE", pCity->getNameKey()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getCityInsertCultureCostFactor() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity && pPlot->getCulture(GC.getGameINLINE().getActivePlayer()) > 0)
			{
				int iCultureAmount = kMission.getCityInsertCultureAmountFactor() * pCity->countTotalCultureTimes100();
				iCultureAmount /= 10000;
				iCultureAmount = std::max(1, iCultureAmount);

				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_INSERT_CULTURE", pCity->getNameKey(), iCultureAmount, kMission.getCityInsertCultureAmountFactor()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getCityPoisonWaterCounter() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_POISON", kMission.getCityPoisonWaterCounter(), gDLL->getSymbolID(UNHEALTHY_CHAR), pCity->getNameKey(), kMission.getCityPoisonWaterCounter()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getCityUnhappinessCounter() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_POISON", kMission.getCityUnhappinessCounter(), gDLL->getSymbolID(UNHAPPY_CHAR), pCity->getNameKey(), kMission.getCityUnhappinessCounter()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getCityRevoltCounter() > 0)
	{
		if (NULL != pPlot)
		{
			CvCity* pCity = pPlot->getPlotCity();

			if (NULL != pCity)
			{
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_REVOLT", pCity->getNameKey(), kMission.getCityRevoltCounter()));
				szBuffer.append(NEWLINE);
			}
		}
	}

	if (kMission.getStealTreasuryTypes() > 0)
	{
		if (NO_PLAYER != eTargetPlayer)
		{
			int iNumTotalGold = (GET_PLAYER(eTargetPlayer).getGold() * kMission.getStealTreasuryTypes()) / 100;

			if (NULL != pPlot)
			{
				CvCity* pCity = pPlot->getPlotCity();

				if (NULL != pCity)
				{
					iNumTotalGold *= pCity->getPopulation();
					iNumTotalGold /= std::max(1, GET_PLAYER(eTargetPlayer).getTotalPopulation());
				}
			}

			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_STEAL_TREASURY", iNumTotalGold, GET_PLAYER(eTargetPlayer).getCivilizationAdjectiveKey()));
			szBuffer.append(NEWLINE);
		}
	}

	if (kMission.getBuyTechCostFactor() > 0)
	{
		szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_STEAL_TECH", GC.getTechInfo((TechTypes)iExtraData).getTextKeyWide()));
		szBuffer.append(NEWLINE);
	}

	if (kMission.getSwitchCivicCostFactor() > 0)
	{
		if (NO_PLAYER != eTargetPlayer)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_SWITCH_CIVIC", GET_PLAYER(eTargetPlayer).getNameKey(), GC.getCivicInfo((CivicTypes)iExtraData).getTextKeyWide()));
			szBuffer.append(NEWLINE);
		}
	}

	if (kMission.getSwitchReligionCostFactor() > 0)
	{
		if (NO_PLAYER != eTargetPlayer)
		{
			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_SWITCH_CIVIC", GET_PLAYER(eTargetPlayer).getNameKey(), GC.getReligionInfo((ReligionTypes)iExtraData).getTextKeyWide()));
			szBuffer.append(NEWLINE);
		}
	}

	if (kMission.getPlayerAnarchyCounter() > 0)
	{
		if (NO_PLAYER != eTargetPlayer)
		{
			int iTurns = (kMission.getPlayerAnarchyCounter() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getAnarchyPercent()) / 100;
			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_ANARCHY", GET_PLAYER(eTargetPlayer).getNameKey(), iTurns));
			szBuffer.append(NEWLINE);
		}
	}

	if (kMission.getCounterespionageNumTurns() > 0 && kMission.getCounterespionageMod() > 0)
	{
		if (NO_PLAYER != eTargetPlayer)
		{
			int iTurns = (kMission.getCounterespionageNumTurns() * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent()) / 100;

			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_HELP_COUNTERESPIONAGE", kMission.getCounterespionageMod(), GET_PLAYER(eTargetPlayer).getCivilizationAdjectiveKey(), iTurns));
			szBuffer.append(NEWLINE);
		}
	}

	szBuffer.append(NEWLINE);
	szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_BASE_COST", iMissionCost));

	if (kPlayer.getEspionageMissionCost(eMission, eTargetPlayer, pPlot, iExtraData, pSpyUnit) > 0)
	{
		int iModifier = 100;
		int iTempModifier = 0;
		CvCity* pCity = NULL;
		if (NULL != pPlot)
		{
			pCity = pPlot->getPlotCity();
		}

		if (pCity != NULL && GC.getEspionageMissionInfo(eMission).isTargetsCity())
		{
			// City Population
			iTempModifier = (GC.getDefineINT("ESPIONAGE_CITY_POP_EACH_MOD") * (pCity->getPopulation() - 1));
			if (0 != iTempModifier)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_POPULATION_MOD", iTempModifier));
				iModifier *= 100 + iTempModifier;
				iModifier /= 100;
			}

			// Trade Route
			if (pCity->isTradeRoute(kPlayer.getID()))
			{
				iTempModifier = GC.getDefineINT("ESPIONAGE_CITY_TRADE_ROUTE_MOD");
				if (0 != iTempModifier)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_TRADE_ROUTE_MOD", iTempModifier));
					iModifier *= 100 + iTempModifier;
					iModifier /= 100;
				}
			}

			ReligionTypes eReligion = kPlayer.getStateReligion();
			if (NO_RELIGION != eReligion)
			{
				iTempModifier = 0;

				if (pCity->isHasReligion(eReligion))
				{
					if (GET_PLAYER(eTargetPlayer).getStateReligion() != eReligion)
					{
						iTempModifier += GC.getDefineINT("ESPIONAGE_CITY_RELIGION_STATE_MOD");
					}

					if (kPlayer.hasHolyCity(eReligion))
					{
						iTempModifier += GC.getDefineINT("ESPIONAGE_CITY_HOLY_CITY_MOD");;
					}
				}

				if (0 != iTempModifier)
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_RELIGION_MOD", iTempModifier));
					iModifier *= 100 + iTempModifier;
					iModifier /= 100;
				}
			}

			// City's culture affects cost
			iTempModifier = -(pCity->getCultureTimes100(kPlayer.getID()) * GC.getDefineINT("ESPIONAGE_CULTURE_MULTIPLIER_MOD")) / std::max(1, pCity->getCultureTimes100(eTargetPlayer) + pCity->getCultureTimes100(kPlayer.getID()));
			if (0 != iTempModifier)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_CULTURE_MOD", iTempModifier));
				iModifier *= 100 + iTempModifier;
				iModifier /= 100;
			}

			iTempModifier = pCity->getEspionageDefenseModifier();
			if (0 != iTempModifier)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_DEFENSE_MOD", iTempModifier));
				iModifier *= 100 + iTempModifier;
				iModifier /= 100;
			}
		}

		// Distance mod
		if (pPlot != NULL)
		{
			int iDistance = GC.getMap().maxPlotDistance();

			CvCity* pOurCapital = kPlayer.getCapitalCity();
			if (NULL != pOurCapital)
			{
				if (kMission.isSelectPlot() || kMission.isTargetsCity())
				{
					iDistance = plotDistance(pOurCapital->getX_INLINE(), pOurCapital->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE());
				}
				else
				{
					CvCity* pTheirCapital = GET_PLAYER(eTargetPlayer).getCapitalCity();
					if (NULL != pTheirCapital)
					{
						iDistance = plotDistance(pOurCapital->getX_INLINE(), pOurCapital->getY_INLINE(), pTheirCapital->getX_INLINE(), pTheirCapital->getY_INLINE());
					}
				}
			}

			iTempModifier = (iDistance + GC.getMapINLINE().maxPlotDistance()) * GC.getDefineINT("ESPIONAGE_DISTANCE_MULTIPLIER_MOD") / GC.getMapINLINE().maxPlotDistance() - 100;
			if (0 != iTempModifier)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_DISTANCE_MOD", iTempModifier));
				iModifier *= 100 + iTempModifier;
				iModifier /= 100;
			}
		}

		// Spy presence mission cost alteration
		if (NULL != pSpyUnit)
		{
			iTempModifier = -(pSpyUnit->getFortifyTurns() * GC.getDefineINT("ESPIONAGE_EACH_TURN_UNIT_COST_DECREASE"));
			if (0 != iTempModifier)
			{
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_SPY_STATIONARY_MOD", iTempModifier));
				iModifier *= 100 + iTempModifier;
				iModifier /= 100;
			}
		}

		// My points VS. Your points to mod cost
		iTempModifier = ::getEspionageModifier(kPlayer.getTeam(), GET_PLAYER(eTargetPlayer).getTeam()) - 100;
		if (0 != iTempModifier)
		{
			szBuffer.append(SEPARATOR);
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_EP_RATIO_MOD", iTempModifier));
			iModifier *= 100 + iTempModifier;
			iModifier /= 100;
		}

		// Counterespionage Mission Mod
		CvTeam& kTargetTeam = GET_TEAM(GET_PLAYER(eTargetPlayer).getTeam());
		if (kTargetTeam.getCounterespionageModAgainstTeam(kPlayer.getTeam()) > 0)
		{
			iTempModifier = kTargetTeam.getCounterespionageModAgainstTeam(kPlayer.getTeam()) - 100;
			if (0 != iTempModifier)
			{
				szBuffer.append(SEPARATOR);
				szBuffer.append(NEWLINE);
				szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_COUNTERESPIONAGE_MOD", iTempModifier));
				iModifier *= 100 + iTempModifier;
				iModifier /= 100;
			}
		}

		FAssert(iModifier == kPlayer.getEspionageMissionCostModifier(eMission, eTargetPlayer, pPlot, iExtraData, pSpyUnit));

		iMissionCost *= iModifier;
		iMissionCost /= 100;

		FAssert(iMissionCost == kPlayer.getEspionageMissionCost(eMission, eTargetPlayer, pPlot, iExtraData, pSpyUnit));

		szBuffer.append(SEPARATOR);

		szBuffer.append(NEWLINE);
		szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_COST_TOTAL", iMissionCost));


		if (NULL != pSpyUnit)
		{
			int iInterceptChance = (pSpyUnit->getSpyInterceptPercent(GET_PLAYER(eTargetPlayer).getTeam()) * (100 + kMission.getDifficultyMod())) / 100;

			szBuffer.append(NEWLINE);
			szBuffer.append(NEWLINE);
			szBuffer.append(gDLL->getText("TXT_KEY_ESPIONAGE_CHANCE_OF_SUCCESS", std::min(100, std::max(0, 100 - iInterceptChance))));
		}
	}
}

void CvGameTextMgr::getTradeScreenTitleIcon(CvString& szButton, CvWidgetDataStruct& widgetData, PlayerTypes ePlayer)
{
	szButton.clear();

	ReligionTypes eReligion = GET_PLAYER(ePlayer).getStateReligion();
	if (eReligion != NO_RELIGION)
	{
		szButton = GC.getReligionInfo(eReligion).getButton();
		widgetData.m_eWidgetType = WIDGET_HELP_RELIGION;
		widgetData.m_iData1 = eReligion;
		widgetData.m_iData2 = -1;
		widgetData.m_bOption = false;
	}
}

void CvGameTextMgr::getTradeScreenIcons(std::vector< std::pair<CvString, CvWidgetDataStruct> >& aIconInfos, PlayerTypes ePlayer)
{
	aIconInfos.clear();
	for (int i = 0; i < GC.getNumCivicOptionInfos(); i++)
	{
		CivicTypes eCivic = GET_PLAYER(ePlayer).getCivics((CivicOptionTypes)i);
		CvWidgetDataStruct widgetData;
		widgetData.m_eWidgetType = WIDGET_PEDIA_JUMP_TO_CIVIC;
		widgetData.m_iData1 = eCivic;
		widgetData.m_iData2 = -1;
		widgetData.m_bOption = false;
		aIconInfos.push_back(std::make_pair(GC.getCivicInfo(eCivic).getButton(), widgetData));
	}

}

void CvGameTextMgr::getTradeScreenHeader(CvWString& szHeader, PlayerTypes ePlayer, PlayerTypes eOtherPlayer, bool bAttitude)
{
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	szHeader.Format(L"%s - %s", CvWString(kPlayer.getName()).GetCString(), CvWString(kPlayer.getCivilizationDescription()).GetCString());
	if (bAttitude)
	{
		szHeader += CvWString::format(L" (%s)", GC.getAttitudeInfo(kPlayer.AI_getAttitude(eOtherPlayer)).getDescription());
	}
}

void CvGameTextMgr::getGlobeLayerName(GlobeLayerTypes eType, int iOption, CvWString& strName)
{
	switch (eType)
	{
	case GLOBE_LAYER_STRATEGY:
		switch (iOption)
		{
		case 0:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_STRATEGY_VIEW");
			break;
		case 1:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_STRATEGY_NEW_LINE");
			break;
		case 2:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_STRATEGY_NEW_SIGN");
			break;
		case 3:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_STRATEGY_DELETE");
			break;
		case 4:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_STRATEGY_DELETE_LINES");
			break;
		}
		break;
	case GLOBE_LAYER_UNIT:
		switch (iOption)
		{
		case SHOW_ALL_MILITARY:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_UNITS_ALLMILITARY");
			break;
		case SHOW_TEAM_MILITARY:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_UNITS_TEAMMILITARY");
			break;
		case SHOW_ENEMIES_IN_TERRITORY:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_UNITS_ENEMY_TERRITORY_MILITARY");
			break;
		case SHOW_ENEMIES:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_UNITS_ENEMYMILITARY");
			break;
		case SHOW_PLAYER_DOMESTICS:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_UNITS_DOMESTICS");
			break;
		}
		break;
	case GLOBE_LAYER_RESOURCE:
		switch (iOption)
		{
		case SHOW_ALL_RESOURCES:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_RESOURCES_EVERYTHING");
			break;
		case SHOW_STRATEGIC_RESOURCES:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_RESOURCES_GENERAL");
			break;
		case SHOW_HAPPY_RESOURCES:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_RESOURCES_LUXURIES");
			break;
		case SHOW_HEALTH_RESOURCES:
			strName = gDLL->getText("TXT_KEY_GLOBELAYER_RESOURCES_FOOD");
			break;
		}
		break;
	case GLOBE_LAYER_RELIGION:
		strName = GC.getReligionInfo((ReligionTypes)iOption).getDescription();
		break;
	case GLOBE_LAYER_CULTURE:
	case GLOBE_LAYER_TRADE:
		// these have no sub-options
		strName.clear();
		break;
	}
}

void CvGameTextMgr::getPlotHelp(CvPlot* pMouseOverPlot, CvCity* pCity, CvPlot* pFlagPlot, bool bAlt, CvWStringBuffer& strHelp)
{
	if (gDLL->getInterfaceIFace()->isCityScreenUp())
	{
		if (pMouseOverPlot != NULL)
		{
			CvCity* pHeadSelectedCity = gDLL->getInterfaceIFace()->getHeadSelectedCity();
			if (pHeadSelectedCity != NULL)
			{
				if (pMouseOverPlot->getWorkingCity() == pHeadSelectedCity)
				{
					if (pMouseOverPlot->isRevealed(GC.getGameINLINE().getActiveTeam(), true))
					{
						setPlotHelp(strHelp, pMouseOverPlot);
					}
				}
			}
		}
	}
	else
	{
		if (pCity != NULL)
		{
			setCityBarHelp(strHelp, pCity);
		}
		else if (pFlagPlot != NULL)
		{
			setPlotListHelp(strHelp, pFlagPlot, false, true);
		}

		if (strHelp.isEmpty())
		{
			if (pMouseOverPlot != NULL)
			{
				if ((pMouseOverPlot == gDLL->getInterfaceIFace()->getGotoPlot()) || bAlt)
				{
					if (pMouseOverPlot->isActiveVisible(true))
					{
						setCombatPlotHelp(strHelp, pMouseOverPlot);
					}
				}
			}
		}

		if (strHelp.isEmpty())
		{
			if (pMouseOverPlot != NULL)
			{
				if (pMouseOverPlot->isRevealed(GC.getGameINLINE().getActiveTeam(), true))
				{
					if (pMouseOverPlot->isActiveVisible(true))
					{
						if (pMouseOverPlot->headUnitNode() != NULL && gDLL->getInterfaceIFace()->getInterfaceMode() == INTERFACEMODE_SPELL_OFFENSIVE) {
							if (::getUnit(pMouseOverPlot->headUnitNode()->m_data)->getTeam() != gDLL->getInterfaceIFace()->getHeadSelectedUnit()->getTeam())
							{
								setSpellFactorsPlotHelp(strHelp, pMouseOverPlot);
							}
						}
						else if (pMouseOverPlot->isActiveVisible(true)) {
							setPlotListHelp(strHelp, pMouseOverPlot, true, false);

							if (!strHelp.isEmpty())
							{
								strHelp.append(L"\n");
							}
						}
					}
					if (gDLL->getInterfaceIFace()->getInterfaceMode() != INTERFACEMODE_SPELL_OFFENSIVE) {
						setPlotHelp(strHelp, pMouseOverPlot);
					}
				}
			}
		}

		InterfaceModeTypes eInterfaceMode = gDLL->getInterfaceIFace()->getInterfaceMode();
		if (eInterfaceMode != INTERFACEMODE_SELECTION)
		{
			CvWString szTempBuffer;
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR NEWLINE, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), GC.getInterfaceModeInfo(eInterfaceMode).getDescription());

			switch (eInterfaceMode)
			{
			case INTERFACEMODE_REBASE:
				getRebasePlotHelp(pMouseOverPlot, szTempBuffer);
				break;

			case INTERFACEMODE_NUKE:
				getNukePlotHelp(pMouseOverPlot, szTempBuffer);
				break;

			default:
				break;
			}

			szTempBuffer += strHelp.getCString();
			strHelp.assign(szTempBuffer);
		}
	}
}

void CvGameTextMgr::getRebasePlotHelp(CvPlot* pPlot, CvWString& strHelp)
{
	if (NULL != pPlot)
	{
		CvUnit* pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();
		if (NULL != pHeadSelectedUnit)
		{
			if (pPlot->isFriendlyCity(*pHeadSelectedUnit, true))
			{
				CvCity* pCity = pPlot->getPlotCity();
				if (NULL != pCity)
				{
					int iNumUnits = pCity->plot()->countNumAirUnits(GC.getGameINLINE().getActiveTeam());
					bool bFull = (iNumUnits >= pCity->getAirUnitCapacity(GC.getGameINLINE().getActiveTeam()));

					if (bFull)
					{
						strHelp += CvWString::format(SETCOLR, TEXT_COLOR("COLOR_WARNING_TEXT"));
					}

					strHelp += NEWLINE + gDLL->getText("TXT_KEY_CITY_BAR_AIR_UNIT_CAPACITY", iNumUnits, pCity->getAirUnitCapacity(GC.getGameINLINE().getActiveTeam()));

					if (bFull)
					{
						strHelp += ENDCOLR;
					}

					strHelp += NEWLINE;
				}
			}
		}
	}
}

void CvGameTextMgr::getNukePlotHelp(CvPlot* pPlot, CvWString& strHelp)
{
	if (NULL != pPlot)
	{
		CvUnit* pHeadSelectedUnit = gDLL->getInterfaceIFace()->getHeadSelectedUnit();

		if (NULL != pHeadSelectedUnit)
		{
			for (int iI = 0; iI < MAX_TEAMS; iI++)
			{
				if (pHeadSelectedUnit->isNukeVictim(pPlot, ((TeamTypes)iI)))
				{
					if (!pHeadSelectedUnit->isEnemy((TeamTypes)iI))
					{
						strHelp += NEWLINE + gDLL->getText("TXT_KEY_CANT_NUKE_FRIENDS");
						break;
					}
				}
			}
		}
	}
}

void CvGameTextMgr::getInterfaceCenterText(CvWString& strText)
{
	strText.clear();
	if (!gDLL->getInterfaceIFace()->isCityScreenUp())
	{
		if (GC.getGameINLINE().getWinner() != NO_TEAM)
		{
			strText = gDLL->getText("TXT_KEY_MISC_WINS_VICTORY", GET_TEAM(GC.getGameINLINE().getWinner()).getName().GetCString(), GC.getVictoryInfo(GC.getGameINLINE().getVictory()).getTextKeyWide());
		}
		else if (!(GET_PLAYER(GC.getGameINLINE().getActivePlayer()).isAlive()))
		{
			strText = gDLL->getText("TXT_KEY_MISC_DEFEAT");
		}
	}
}

void CvGameTextMgr::getTurnTimerText(CvWString& strText)
{
	strText.clear();
	if (gDLL->getInterfaceIFace()->getShowInterface() == INTERFACE_SHOW || gDLL->getInterfaceIFace()->getShowInterface() == INTERFACE_ADVANCED_START)
	{
		if (GC.getGameINLINE().isMPOption(MPOPTION_TURN_TIMER))
		{
			// Get number of turn slices remaining until end-of-turn
			int iTurnSlicesRemaining = GC.getGameINLINE().getTurnSlicesRemaining();

			if (iTurnSlicesRemaining > 0)
			{
				// Get number of seconds remaining
				int iTurnSecondsRemaining = ((int)floorf((float)(iTurnSlicesRemaining - 1) * ((float)gDLL->getMillisecsPerTurn() / 1000.0f)) + 1);
				int iTurnMinutesRemaining = (int)(iTurnSecondsRemaining / 60);
				iTurnSecondsRemaining = (iTurnSecondsRemaining % 60);
				int iTurnHoursRemaining = (int)(iTurnMinutesRemaining / 60);
				iTurnMinutesRemaining = (iTurnMinutesRemaining % 60);

				// Display time remaining
				CvWString szTempBuffer;
				szTempBuffer.Format(L"%d:%02d:%02d", iTurnHoursRemaining, iTurnMinutesRemaining, iTurnSecondsRemaining);
				strText += szTempBuffer;
			}
			else
			{
				// Flash zeroes
				if (iTurnSlicesRemaining % 2 == 0)
				{
					// Display 0
					strText += L"0:00";
				}
			}
		}

		if (GC.getGameINLINE().getGameState() == GAMESTATE_ON)
		{
			int iMinVictoryTurns = MAX_INT;
			for (int i = 0; i < GC.getNumVictoryInfos(); ++i)
			{
				TeamTypes eActiveTeam = GC.getGameINLINE().getActiveTeam();
				if (NO_TEAM != eActiveTeam)
				{
					int iCountdown = GET_TEAM(eActiveTeam).getVictoryCountdown((VictoryTypes)i);
					if (iCountdown > 0 && iCountdown < iMinVictoryTurns)
					{
						iMinVictoryTurns = iCountdown;
					}
				}
			}

			if (GC.getGameINLINE().isOption(GAMEOPTION_ADVANCED_START) && !GC.getGameINLINE().isOption(GAMEOPTION_ALWAYS_WAR) && GC.getGameINLINE().getElapsedGameTurns() <= GC.getDefineINT("PEACE_TREATY_LENGTH"))
			{
				if (!strText.empty())
				{
					strText += L" -- ";
				}

				strText += gDLL->getText("TXT_KEY_MISC_ADVANCED_START_PEACE_REMAINING", GC.getDefineINT("PEACE_TREATY_LENGTH") - GC.getGameINLINE().getElapsedGameTurns());
			}
			else if (iMinVictoryTurns < MAX_INT)
			{
				if (!strText.empty())
				{
					strText += L" -- ";
				}

				strText += gDLL->getText("TXT_KEY_MISC_TURNS_LEFT_TO_VICTORY", iMinVictoryTurns);
			}
			else if (GC.getGameINLINE().getMaxTurns() > 0)
			{
				if ((GC.getGameINLINE().getElapsedGameTurns() >= (GC.getGameINLINE().getMaxTurns() - 100)) && (GC.getGameINLINE().getElapsedGameTurns() < GC.getGameINLINE().getMaxTurns()))
				{
					if (!strText.empty())
					{
						strText += L" -- ";
					}

					strText += gDLL->getText("TXT_KEY_MISC_TURNS_LEFT", (GC.getGameINLINE().getMaxTurns() - GC.getGameINLINE().getElapsedGameTurns()));
				}
			}
		}
	}
}


void CvGameTextMgr::getFontSymbols(std::vector< std::vector<wchar> >& aacSymbols, std::vector<int>& aiMaxNumRows)
{
	aacSymbols.push_back(std::vector<wchar>());
	aiMaxNumRows.push_back(1);
	for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
	{
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)GC.getYieldInfo((YieldTypes)iI).getChar());
	}

	aacSymbols.push_back(std::vector<wchar>());
	aiMaxNumRows.push_back(2);
	for (int iI = 0; iI < NUM_COMMERCE_TYPES; iI++)
	{
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)GC.getCommerceInfo((CommerceTypes)iI).getChar());
	}

	aacSymbols.push_back(std::vector<wchar>());
	aiMaxNumRows.push_back(2);
	for (int iI = 0; iI < GC.getNumReligionInfos(); iI++)
	{
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)GC.getReligionInfo((ReligionTypes)iI).getChar());
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)GC.getReligionInfo((ReligionTypes)iI).getHolyCityChar());
	}
	for (int iI = 0; iI < GC.getNumCorporationInfos(); iI++)
	{
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)GC.getCorporationInfo((CorporationTypes)iI).getChar());
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)GC.getCorporationInfo((CorporationTypes)iI).getHeadquarterChar());
	}

	aacSymbols.push_back(std::vector<wchar>());
	aiMaxNumRows.push_back(3);

	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)GC.getBonusInfo((BonusTypes)iI).getChar());
	}

	aacSymbols.push_back(std::vector<wchar>());
	aiMaxNumRows.push_back(3);
	for (int iI = 0; iI < MAX_NUM_SYMBOLS; iI++)
	{
		aacSymbols[aacSymbols.size() - 1].push_back((wchar)gDLL->getSymbolID(iI));
	}
}

void CvGameTextMgr::assignFontIds(int iFirstSymbolCode, int iPadAmount)
{
	int iCurSymbolID = iFirstSymbolCode;

	// set yield symbols
	for (int i = 0; i < NUM_YIELD_TYPES; i++)
	{
		GC.getYieldInfo((YieldTypes)i).setChar(iCurSymbolID);
		++iCurSymbolID;
	}

	do
	{
		++iCurSymbolID;
	} while (iCurSymbolID % iPadAmount != 0);

	// set commerce symbols
	for (i = 0; i < GC.getNUM_COMMERCE_TYPES(); i++)
	{
		GC.getCommerceInfo((CommerceTypes)i).setChar(iCurSymbolID);
		++iCurSymbolID;
	}

	do
	{
		++iCurSymbolID;
	} while (iCurSymbolID % iPadAmount != 0);

	if (NUM_COMMERCE_TYPES < iPadAmount)
	{
		do
		{
			++iCurSymbolID;
		} while (iCurSymbolID % iPadAmount != 0);
	}

	for (int i = 0; i < GC.getNumReligionInfos(); i++)
	{
		GC.getReligionInfo((ReligionTypes)i).setChar(iCurSymbolID);
		++iCurSymbolID;
		GC.getReligionInfo((ReligionTypes)i).setHolyCityChar(iCurSymbolID);
		++iCurSymbolID;
	}
	for (i = 0; i < GC.getNumCorporationInfos(); i++)
	{
		GC.getCorporationInfo((CorporationTypes)i).setChar(iCurSymbolID);
		++iCurSymbolID;
		GC.getCorporationInfo((CorporationTypes)i).setHeadquarterChar(iCurSymbolID);
		++iCurSymbolID;
	}

	do
	{
		++iCurSymbolID;
	} while (iCurSymbolID % iPadAmount != 0);

	if (2 * (GC.getNumReligionInfos() + GC.getNumCorporationInfos()) < iPadAmount)
	{
		do
		{
			++iCurSymbolID;
		} while (iCurSymbolID % iPadAmount != 0);
	}
	// modified Sephi
	// Symbol loading adjusted to WoC.
	int iSavePosition = iCurSymbolID;

	// set bonus symbols
	int bonusBaseID = iCurSymbolID;
	++iCurSymbolID;
	for (int i = 0; i < GC.getNumBonusInfos(); i++)
	{
		int bonusID = bonusBaseID + GC.getBonusInfo((BonusTypes)i).getArtInfo()->getFontButtonIndex();
		GC.getBonusInfo((BonusTypes)i).setChar(bonusID);
		++iCurSymbolID;
	}

	iCurSymbolID = iSavePosition + 125;
	iCurSymbolID -= MAX_NUM_SYMBOLS;
	do
	{
		--iCurSymbolID;
	} while (iCurSymbolID % iPadAmount != 0);

	// modified Sephi

		// set extra symbols
	for (int i = 0; i < MAX_NUM_SYMBOLS; i++)
	{
		gDLL->setSymbolID(i, iCurSymbolID);
		++iCurSymbolID;
	}
}

void CvGameTextMgr::getCityDataForAS(std::vector<CvWBData>& mapCityList, std::vector<CvWBData>& mapBuildingList, std::vector<CvWBData>& mapAutomateList)
{
	CvPlayer& kActivePlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	CvWString szHelp;
	int iCost = kActivePlayer.getAdvancedStartCityCost(true);
	if (iCost > 0)
	{
		szHelp = gDLL->getText("TXT_KEY_CITY");
		szHelp += gDLL->getText("TXT_KEY_AS_UNREMOVABLE");
		mapCityList.push_back(CvWBData(0, szHelp, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_BUTTONS_CITYSELECTION")->getPath()));
	}

	iCost = kActivePlayer.getAdvancedStartPopCost(true);
	if (iCost > 0)
	{
		szHelp = gDLL->getText("TXT_KEY_WB_AS_POPULATION");
		mapCityList.push_back(CvWBData(1, szHelp, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_ANGRYCITIZEN_TEXTURE")->getPath()));
	}

	iCost = kActivePlayer.getAdvancedStartCultureCost(true);
	if (iCost > 0)
	{
		szHelp = gDLL->getText("TXT_KEY_ADVISOR_CULTURE");
		szHelp += gDLL->getText("TXT_KEY_AS_UNREMOVABLE");
		mapCityList.push_back(CvWBData(2, szHelp, ARTFILEMGR.getInterfaceArtInfo("CULTURE_BUTTON")->getPath()));
	}

	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(kActivePlayer.getCivilizationType()).getCivilizationBuildings(i);

		if (eBuilding != NO_BUILDING)
		{
			if (GC.getBuildingInfo(eBuilding).getFreeStartEra() == NO_ERA || GC.getGameINLINE().getStartEra() < GC.getBuildingInfo(eBuilding).getFreeStartEra())
			{
				// Building cost -1 denotes unit which may not be purchased
				iCost = kActivePlayer.getAdvancedStartBuildingCost(eBuilding, true);
				if (iCost > 0)
				{
					szBuffer.clear();
					setBuildingHelp(szBuffer, eBuilding);
					mapBuildingList.push_back(CvWBData(eBuilding, szBuffer.getCString(), GC.getBuildingInfo(eBuilding).getButton()));
				}
			}
		}
	}

	szHelp = gDLL->getText("TXT_KEY_ACTION_AUTOMATE_BUILD");
	mapAutomateList.push_back(CvWBData(0, szHelp, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_AUTOMATE")->getPath()));
}

void CvGameTextMgr::getUnitDataForAS(std::vector<CvWBData>& mapUnitList)
{
	CvPlayer& kActivePlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumUnitClassInfos(); i++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(kActivePlayer.getCivilizationType()).getCivilizationUnits(i);
		if (eUnit != NO_UNIT)
		{
			// Unit cost -1 denotes unit which may not be purchased
			int iCost = kActivePlayer.getAdvancedStartUnitCost(eUnit, true);
			if (iCost > 0)
			{
				szBuffer.clear();
				setUnitHelp(szBuffer, eUnit);

				int iMaxUnitsPerCity = GC.getDefineINT("ADVANCED_START_MAX_UNITS_PER_CITY");
				if (iMaxUnitsPerCity >= 0 && GC.getUnitInfo(eUnit).isMilitarySupport())
				{
					szBuffer.append(NEWLINE);
					szBuffer.append(gDLL->getText("TXT_KEY_WB_AS_MAX_UNITS_PER_CITY", iMaxUnitsPerCity));
				}
				mapUnitList.push_back(CvWBData(eUnit, szBuffer.getCString(), kActivePlayer.getUnitButton(eUnit)));
			}
		}
	}
}

void CvGameTextMgr::getImprovementDataForAS(std::vector<CvWBData>& mapImprovementList, std::vector<CvWBData>& mapRouteList)
{
	CvPlayer& kActivePlayer = GET_PLAYER(GC.getGameINLINE().getActivePlayer());

	for (int i = 0; i < GC.getNumRouteInfos(); i++)
	{
		RouteTypes eRoute = (RouteTypes)i;
		if (eRoute != NO_ROUTE)
		{
			// Route cost -1 denotes route which may not be purchased
			int iCost = kActivePlayer.getAdvancedStartRouteCost(eRoute, true);
			if (iCost > 0)
			{
				mapRouteList.push_back(CvWBData(eRoute, GC.getRouteInfo(eRoute).getDescription(), GC.getRouteInfo(eRoute).getButton()));
			}
		}
	}

	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumImprovementInfos(); i++)
	{
		ImprovementTypes eImprovement = (ImprovementTypes)i;
		if (eImprovement != NO_IMPROVEMENT)
		{
			// Improvement cost -1 denotes Improvement which may not be purchased
			int iCost = kActivePlayer.getAdvancedStartImprovementCost(eImprovement, true);
			if (iCost > 0)
			{
				szBuffer.clear();
				setImprovementHelp(szBuffer, eImprovement);
				mapImprovementList.push_back(CvWBData(eImprovement, szBuffer.getCString(), GC.getImprovementInfo(eImprovement).getButton()));
			}
		}
	}
}

void CvGameTextMgr::getVisibilityDataForAS(std::vector<CvWBData>& mapVisibilityList)
{
	// Unit cost -1 denotes unit which may not be purchased
	int iCost = GET_PLAYER(GC.getGameINLINE().getActivePlayer()).getAdvancedStartVisibilityCost(true);
	if (iCost > 0)
	{
		CvWString szHelp = gDLL->getText("TXT_KEY_WB_AS_VISIBILITY");
		szHelp += gDLL->getText("TXT_KEY_AS_UNREMOVABLE", iCost);
		mapVisibilityList.push_back(CvWBData(0, szHelp, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_TECH_LOS")->getPath()));
	}
}

void CvGameTextMgr::getTechDataForAS(std::vector<CvWBData>& mapTechList)
{
	mapTechList.push_back(CvWBData(0, gDLL->getText("TXT_KEY_WB_AS_TECH"), ARTFILEMGR.getInterfaceArtInfo("INTERFACE_BTN_TECH")->getPath()));
}

void CvGameTextMgr::getUnitDataForWB(std::vector<CvWBData>& mapUnitData)
{
	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumUnitInfos(); i++)
	{
		szBuffer.clear();
		setUnitHelp(szBuffer, (UnitTypes)i);
		mapUnitData.push_back(CvWBData(i, szBuffer.getCString(), GC.getUnitInfo((UnitTypes)i).getButton()));
	}
}

void CvGameTextMgr::getBuildingDataForWB(bool bStickyButton, std::vector<CvWBData>& mapBuildingData)
{
	int iCount = 0;
	if (!bStickyButton)
	{
		mapBuildingData.push_back(CvWBData(iCount++, GC.getMissionInfo(MISSION_FOUND).getDescription(), GC.getMissionInfo(MISSION_FOUND).getButton()));
	}

	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumBuildingInfos(); i++)
	{
		szBuffer.clear();
		setBuildingHelp(szBuffer, (BuildingTypes)i);
		mapBuildingData.push_back(CvWBData(iCount++, szBuffer.getCString(), GC.getBuildingInfo((BuildingTypes)i).getButton()));
	}
}

void CvGameTextMgr::getTerrainDataForWB(std::vector<CvWBData>& mapTerrainData, std::vector<CvWBData>& mapFeatureData, std::vector<CvWBData>& mapPlotData, std::vector<CvWBData>& mapRouteData)
{
	CvWStringBuffer szBuffer;

	for (int i = 0; i < GC.getNumTerrainInfos(); i++)
	{
		if (!GC.getTerrainInfo((TerrainTypes)i).isGraphicalOnly())
		{
			szBuffer.clear();
			setTerrainHelp(szBuffer, (TerrainTypes)i);
			mapTerrainData.push_back(CvWBData(i, szBuffer.getCString(), GC.getTerrainInfo((TerrainTypes)i).getButton()));
		}
	}

	for (int i = 0; i < GC.getNumFeatureInfos(); i++)
	{
		for (int k = 0; k < GC.getFeatureInfo((FeatureTypes)i).getArtInfo()->getNumVarieties(); k++)
		{
			szBuffer.clear();
			setFeatureHelp(szBuffer, (FeatureTypes)i);
			mapFeatureData.push_back(CvWBData(i + GC.getNumFeatureInfos() * k, szBuffer.getCString(), GC.getFeatureInfo((FeatureTypes)i).getArtInfo()->getVariety(k).getVarietyButton()));
		}
	}

	mapPlotData.push_back(CvWBData(0, gDLL->getText("TXT_KEY_WB_PLOT_TYPE_MOUNTAIN"), ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_PLOT_TYPE_MOUNTAIN")->getPath()));
	mapPlotData.push_back(CvWBData(1, gDLL->getText("TXT_KEY_WB_PLOT_TYPE_HILL"), ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_PLOT_TYPE_HILL")->getPath()));
	mapPlotData.push_back(CvWBData(2, gDLL->getText("TXT_KEY_WB_PLOT_TYPE_PLAINS"), ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_PLOT_TYPE_PLAINS")->getPath()));
	mapPlotData.push_back(CvWBData(3, gDLL->getText("TXT_KEY_WB_PLOT_TYPE_OCEAN"), ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_PLOT_TYPE_OCEAN")->getPath()));

	for (int i = 0; i < GC.getNumRouteInfos(); i++)
	{
		mapRouteData.push_back(CvWBData(i, GC.getRouteInfo((RouteTypes)i).getDescription(), GC.getRouteInfo((RouteTypes)i).getButton()));
	}
	mapRouteData.push_back(CvWBData(GC.getNumRouteInfos(), gDLL->getText("TXT_KEY_WB_RIVER_PLACEMENT"), ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_RIVER_PLACEMENT")->getPath()));
}

void CvGameTextMgr::getTerritoryDataForWB(std::vector<CvWBData>& mapTerritoryData)
{
	for (int i = 0; i <= MAX_CIV_PLAYERS; i++)
	{
		CvWString szName = gDLL->getText("TXT_KEY_MAIN_MENU_NONE");
		CvString szButton = GC.getCivilizationInfo(GET_PLAYER(BARBARIAN_PLAYER).getCivilizationType()).getButton();

		if (GET_PLAYER((PlayerTypes)i).isEverAlive())
		{
			szName = GET_PLAYER((PlayerTypes)i).getName();
			szButton = GC.getCivilizationInfo(GET_PLAYER((PlayerTypes)i).getCivilizationType()).getButton();
		}
		mapTerritoryData.push_back(CvWBData(i, szName, szButton));
	}
}


void CvGameTextMgr::getTechDataForWB(std::vector<CvWBData>& mapTechData)
{
	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumTechInfos(); i++)
	{
		szBuffer.clear();
		setTechHelp(szBuffer, (TechTypes)i);
		mapTechData.push_back(CvWBData(i, szBuffer.getCString(), GC.getTechInfo((TechTypes)i).getButton()));
	}
}

void CvGameTextMgr::getPromotionDataForWB(std::vector<CvWBData>& mapPromotionData)
{
	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumPromotionInfos(); i++)
	{
		szBuffer.clear();
		setPromotionHelp(szBuffer, (PromotionTypes)i, false);
		mapPromotionData.push_back(CvWBData(i, szBuffer.getCString(), GC.getPromotionInfo((PromotionTypes)i).getButton()));
	}
}

void CvGameTextMgr::getBonusDataForWB(std::vector<CvWBData>& mapBonusData)
{
	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumBonusInfos(); i++)
	{
		szBuffer.clear();
		setBonusHelp(szBuffer, (BonusTypes)i);
		mapBonusData.push_back(CvWBData(i, szBuffer.getCString(), GC.getBonusInfo((BonusTypes)i).getButton()));
	}
}

void CvGameTextMgr::getImprovementDataForWB(std::vector<CvWBData>& mapImprovementData)
{
	CvWStringBuffer szBuffer;
	for (int i = 0; i < GC.getNumImprovementInfos(); i++)
	{
		CvImprovementInfo& kInfo = GC.getImprovementInfo((ImprovementTypes)i);
		if (!kInfo.isGraphicalOnly())
		{
			szBuffer.clear();
			setImprovementHelp(szBuffer, (ImprovementTypes)i);
			mapImprovementData.push_back(CvWBData(i, szBuffer.getCString(), kInfo.getButton()));
		}
	}
}

void CvGameTextMgr::getReligionDataForWB(bool bHolyCity, std::vector<CvWBData>& mapReligionData)
{
	for (int i = 0; i < GC.getNumReligionInfos(); ++i)
	{
		CvReligionInfo& kInfo = GC.getReligionInfo((ReligionTypes)i);
		CvWString strDescription = kInfo.getDescription();
		if (bHolyCity)
		{
			strDescription = gDLL->getText("TXT_KEY_WB_HOLYCITY", strDescription.GetCString());
		}
		mapReligionData.push_back(CvWBData(i, strDescription, kInfo.getButton()));
	}
}


void CvGameTextMgr::getCorporationDataForWB(bool bHeadquarters, std::vector<CvWBData>& mapCorporationData)
{
	for (int i = 0; i < GC.getNumCorporationInfos(); ++i)
	{
		CvCorporationInfo& kInfo = GC.getCorporationInfo((CorporationTypes)i);
		CvWString strDescription = kInfo.getDescription();
		if (bHeadquarters)
		{
			strDescription = gDLL->getText("TXT_KEY_CORPORATION_HEADQUARTERS", strDescription.GetCString());
		}
		mapCorporationData.push_back(CvWBData(i, strDescription, kInfo.getButton()));
	}
}
/*************************************************************************************************/
/**	ADDON (New Mana) Sephi                                                   					**/
/*************************************************************************************************/
void CvGameTextMgr::setManaStr(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).getMana() < 0)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getCommerceInfo(COMMERCE_MANA).getChar(), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GET_PLAYER(ePlayer).getMana());
	}
	else
	{
		szString.Format(L"%c: %d", GC.getCommerceInfo(COMMERCE_MANA).getChar(), GET_PLAYER(ePlayer).getMana());
	}

	int iManaRate = GET_PLAYER(ePlayer).CalculateManaByBuildings();
	iManaRate += GET_PLAYER(ePlayer).CalculateManaByNodes();
	iManaRate += GET_PLAYER(ePlayer).CalculateFromSpellResearch();
	iManaRate -= GET_PLAYER(ePlayer).getManaChanneledIntoRitual();
	iManaRate -= GET_PLAYER(ePlayer).getManaUpkeepTotal();
	iManaRate -= GET_PLAYER(ePlayer).CalculateManaUpkeepGE();

	iManaRate = std::min(iManaRate, GET_PLAYER(ePlayer).getMaxMana() - GET_PLAYER(ePlayer).getMana());

	if (iManaRate < 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_NEG_GOLD_PER_TURN", iManaRate);
	}
	else if (iManaRate > 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_POS_GOLD_PER_TURN", iManaRate);
	}

}

void CvGameTextMgr::setUnitSupportStr(CvWString& szString, PlayerTypes ePlayer)
{
	int iSupportNeeded = GET_PLAYER(ePlayer).getUnitSupportUsed();
	int iSupportLimit = GET_PLAYER(ePlayer).getUnitSupportLimitTotal();
	szString.Format(L"%d/%d %c", iSupportNeeded, iSupportLimit, gDLL->getSymbolID(STRENGTH_CHAR));
}

void CvGameTextMgr::setFaithStr(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).getFaith() < 0)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getCommerceInfo(COMMERCE_FAITH).getChar(), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GET_PLAYER(ePlayer).getFaith());
	}
	else
	{
		szString.Format(L"%c: %d", GC.getCommerceInfo(COMMERCE_FAITH).getChar(), GET_PLAYER(ePlayer).getFaith());
	}

	int iFaithRate = GET_PLAYER(ePlayer).getFaithIncome();
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	bool hasAllSacredKnowledge = true;

	for (int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		CvTechInfo& kTech = GC.getTechInfo((TechTypes)iI);
		if (kTech.isHolyKnowledge() && !kPlayer.isHasTech(iI) && kTech.getReligionType() == kPlayer.getStateReligion()
			&& (kTech.getPrereqOrTechs(0) == NO_TECH || kPlayer.isHasTech(kTech.getPrereqOrTechs(0)))) {
			hasAllSacredKnowledge = false;
		}
	}

	if(kPlayer.getStateReligion() != NO_RELIGION && !hasAllSacredKnowledge)
		iFaithRate -= (iFaithRate / 2);

	if (iFaithRate < 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_NEG_GOLD_PER_TURN", iFaithRate);
	}
	else if (iFaithRate > 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_POS_GOLD_PER_TURN", iFaithRate);
	}
}

void CvGameTextMgr::setArcaneStr(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).getArcane() < 0 || GET_PLAYER(ePlayer).getSpellResearchFocus() == NO_TECH)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getCommerceInfo(COMMERCE_ARCANE).getChar(), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GET_PLAYER(ePlayer).getArcane());
	}
	else
	{
		szString.Format(L"%c: %d / %d", GC.getCommerceInfo(COMMERCE_ARCANE).getChar(), GET_PLAYER(ePlayer).getArcane(), GET_PLAYER(ePlayer).getArcaneNeeded(GET_PLAYER(ePlayer).getSpellResearchFocus()));
	}
}

void CvGameTextMgr::setGlobalYieldStr(CvWString& szString, PlayerTypes ePlayer, int iYieldType)
{
	CvWString szTempBuffer;
	if (iYieldType <= YIELD_COMMERCE || iYieldType > GC.getNUM_YIELD_TYPES())
	{
		return;
	}
	int iYield = GET_PLAYER(ePlayer).getGlobalYield(iYieldType);
	int iYieldIncome = GET_PLAYER(ePlayer).calculateGlobalYieldIncome(iYieldType) - GET_PLAYER(ePlayer).getGlobalYieldUpkeep(iYieldType);
	int iYieldNeeded = 0;
	switch (iYieldType)
	{
	case YIELD_LUMBER:
	case YIELD_METAL:
	case YIELD_LEATHER:
		iYieldNeeded = GET_PLAYER(ePlayer).getGlobalYieldNeededForEquipmentBonus(iYieldType);
		break;
	default:
		break;
	}
	if (iYield < iYieldNeeded / 3)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getYieldInfo((YieldTypes)iYieldType).getChar(), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), iYield);
	}
	else if (iYield < 2 * iYieldNeeded / 3)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getYieldInfo((YieldTypes)iYieldType).getChar(), TEXT_COLOR("COLOR_YELLOW"), iYield);
	}
	else if (iYield < iYieldNeeded)
	{
		szString.Format(L"%c%d", GC.getYieldInfo((YieldTypes)iYieldType).getChar(), iYield);
	}
	else
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getYieldInfo((YieldTypes)iYieldType).getChar(), TEXT_COLOR("COLOR_GREEN"), iYield);
	}

	if (iYieldIncome > 0)
	{
		szTempBuffer.Format(SETCOLR L"+%d" SETCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), iYieldIncome);
		szString.append(szTempBuffer);
	}
	else if (iYieldIncome < 0)
	{
		szTempBuffer.Format(SETCOLR L"%d" SETCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), iYieldIncome);
		szString.append(szTempBuffer);
	}

	/**
		int iIncome = GET_PLAYER(ePlayer).calculateGlobalYieldIncome(iYieldType);
		if (iIncome < 0)
		{
			szString += gDLL->getText("TXT_KEY_MISC_NEG_GOLD_PER_TURN", iIncome);
		}
		else if (iIncome >= 0)
		{
			szString += gDLL->getText("TXT_KEY_MISC_POS_GOLD_PER_TURN", iIncome);
		}
	**/
}

void CvGameTextMgr::setGlobalCultureStr(CvWString& szString, PlayerTypes ePlayer)
{
	if (GET_PLAYER(ePlayer).getGlobalCulture() < 0)
	{
		szString.Format(L"%c: " SETCOLR L"%d" SETCOLR, GC.getCommerceInfo(COMMERCE_CULTURE).getChar(), TEXT_COLOR("COLOR_NEGATIVE_TEXT"), GET_PLAYER(ePlayer).getGlobalCulture());
	}
	else
	{
		szString.Format(L"%c: %d", GC.getCommerceInfo(COMMERCE_CULTURE).getChar(), GET_PLAYER(ePlayer).getGlobalCulture());
	}

	int iRate = GET_PLAYER(ePlayer).calculateGlobalCultureIncome();

	if (iRate < 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_NEG_GOLD_PER_TURN", iRate);
	}
	else if (iRate > 0)
	{
		szString += gDLL->getText("TXT_KEY_MISC_POS_GOLD_PER_TURN", iRate);
	}
}

void CvGameTextMgr::setCombatAuraStr(CvWString& szString, CvCombatAura* pAura, const CvUnit* pUnit, int iType)
{
	if (pAura == NULL || pUnit == NULL)
	{
		return;
	}

	CvWString szTempBuffer;
	CvCombatAuraInfo& kAura = GC.getCombatAuraInfo(pAura->getCombatAuraType());
	CvCombatAuraClassInfo& kClass = GC.getCombatAuraClassInfo((CombatAuraClassTypes)kAura.getCombatAuraClassType());

	if (iType == 0)
	{
		szTempBuffer.Format(SETCOLR L"CombatAura (%s)" ENDCOLR, TEXT_COLOR("COLOR_HIGHLIGHT_TEXT"), kClass.getDescription());
		szString.append(szTempBuffer);

		szTempBuffer.Format(L"<img=%S size=16></img> ", kClass.getButton());
		szString.append(szTempBuffer);

		if (pAura->getRange() != -1)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_AURA_RANGE", pAura->getRange()));
		}

		if (pAura->getTargets() != -1)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_AURA_TARGETS", pAura->getTargets()));
		}

		if (kAura.getPrereqPromotion() != NO_PROMOTION)
		{
			szString.append(NEWLINE);
			szTempBuffer.Format(L"%s%s", gDLL->getText("TXT_KEY_AURA_REQUIRES_PROMOTION").c_str(), GC.getPromotionInfo((PromotionTypes)kAura.getPrereqPromotion()).getDescription());
			szString.append(szTempBuffer);
		}

		/** not implemented currently
		if(pAura->getTier()!=-1 && pAura->getTier()<4)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_AURA_TIER_REDUCED_EFFECT", (pAura->getTier())+1));
		}
		**/
	}
	if (iType == 1)
	{
		szTempBuffer.Format(SETCOLR L"Curse I (%s)" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), kClass.getDescription());
		szString.append(szTempBuffer);
	}

	if (iType == 2)
	{
		szTempBuffer.Format(SETCOLR L"Curse II (%s)" ENDCOLR, TEXT_COLOR("COLOR_NEGATIVE_TEXT"), kClass.getDescription());
		szString.append(szTempBuffer);
	}

	if (iType == 3)
	{
		szTempBuffer.Format(SETCOLR L"Blessing I (%s)" ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), kClass.getDescription());
		szString.append(szTempBuffer);
	}

	if (iType == 4)
	{
		szTempBuffer.Format(SETCOLR L"Blessing II (%s)" ENDCOLR, TEXT_COLOR("COLOR_POSITIVE_TEXT"), kClass.getDescription());
		szString.append(szTempBuffer);
	}

	if (iType > 0)
	{
		szTempBuffer.Format(L"<img=%S size=16></img> ", kClass.getButton());
		szString.append(szTempBuffer);
	}

	if (pAura->getFear() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_FEAR", pAura->getFear()));
	}

	if (pAura->getStrength() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_STRENGTH", pAura->getStrength() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getAlignmentAttackBonus() != 0)
	{
		szString.append(NEWLINE);
		szTempBuffer.Format(L"%s%s:%d%%", gDLL->getText("TXT_KEY_AURA_BONUS_VS_ALIGNMENT").c_str(), GC.getAlignmentInfo((AlignmentTypes)kAura.getAlignmentType()).getDescription(), pAura->getAlignmentAttackBonus() * (pAura->isBlessing() ? 1 : -1));
		szString.append(szTempBuffer);
	}

	if (pAura->getAttackPromotionBonus() != 0)
	{
		szString.append(NEWLINE);
		szTempBuffer.Format(L"%s%s:%d%%", gDLL->getText("TXT_KEY_AURA_BONUS_VS_PROMOTION").c_str(), GC.getPromotionInfo((PromotionTypes)kAura.getPromotionType()).getDescription(), pAura->getAttackPromotionBonus() * (pAura->isBlessing() ? 1 : -1));
		szString.append(szTempBuffer);
	}

	if (pAura->getCityAttack() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_CITY_ATTACK", pAura->getCityAttack() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getDamage() != 0)
	{
		szString.append(NEWLINE);
		szTempBuffer.Format(L"%c%d%% %s %s", gDLL->getSymbolID(BULLET_CHAR), pAura->getDamage(), GC.getDamageTypeInfo((DamageTypes)kAura.getDamageType()).getDescription(), gDLL->getText("TXT_KEY_AURA_DAMAGE").c_str());
		szString.append(szTempBuffer);
	}

	for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
	{
		if (pAura->getDamageTypeResist((DamageTypes)iI) > 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_AURA_RESIST", GC.getDamageTypeInfo((DamageTypes)iI).getDescription(), pAura->getDamageTypeResist((DamageTypes)iI)));
		}
		else if (pAura->getDamageTypeResist((DamageTypes)iI) < 0)
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_AURA_VULNERABILITY", GC.getDamageTypeInfo((DamageTypes)iI).getDescription(), -pAura->getDamageTypeResist((DamageTypes)iI)));
		}
	}

	if (pAura->isEnchantedWeapons())
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_ENCHANTED_WEAPONS", pAura->getEnchantedWeaponBonus()));
	}

	if (pAura->getExperience() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_EXPERIENCE", pAura->getExperience() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getFirststrikes() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_FIRSTSTRIKES", (pAura->getFirststrikes() / 100) * (pAura->isBlessing() ? 1 : -1)));
	}

	if (kAura.getGoldFromCombat() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_GOLD_FROM_COMBAT", kAura.getGoldFromCombat()));
	}

	if (pAura->getGuardianAngel() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_GUARDIAN_ANGEL", (pAura->getGuardianAngel() * (100 + pAura->getPower()) / 100)));
	}

	if (pAura->getHealing() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_HEALING", pAura->getHealing() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getHealthRegeneration() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_HEALTH_REGENERATION", pAura->getHealthRegeneration() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getIgnoreFirststrikes() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_IGNORE_FIRSTSTRIKES", (pAura->getIgnoreFirststrikes() / 50) * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->isImmunityRangedStrike())
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_IMMUNITY_TO_RANGED_STRIKES"));
	}

	if (pAura->isLoyalty())
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_IMMUNE_TO_CAPTURE"));
	}

	if (pAura->isImmuneToFear())
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_IMMUNE_TO_FEAR"));
	}

	if (pAura->getMagicResistance() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_MAGIC_RESISTANCE", pAura->getMagicResistance() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getSpellPower() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_MAGIC_SPELLPOWER", pAura->getSpellPower() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getRust() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_RUST", pAura->getRust() * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getSpellcasterXP() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_SPELLCASTER_XP", (pAura->getSpellcasterXP() / 50) * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->getUpgradePercent() != 0)
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_UPGRADE_PERCENT", (pAura->getUpgradePercent()) * (pAura->isBlessing() ? 1 : -1)));
	}

	if (pAura->isWaterwalking())
	{
		szString.append(NEWLINE);
		szString.append(gDLL->getText("TXT_KEY_AURA_WATER_WALKING"));
	}

	bool bFirst = true;
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		bool bValid = false;
		if (kAura.isPromotionApply((PromotionTypes)iI))
		{
			bValid = true;
		}
		else
		{
			for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if (pAura->getUnit()->isHasPromotion((PromotionTypes)iJ))
				{
					if (GC.getPromotionInfo((PromotionTypes)iJ).isCombatAuraApplyPromotion(iI))
					{
						bValid = true;
						break;
					}
				}
			}
		}

		if (bValid)
		{
			if (bFirst)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_AURA_PROMOTION_APPLY"));
				szString.append(GC.getPromotionInfo((PromotionTypes)iI).getDescription());
				bFirst = false;
			}
			else
			{
				szString.append(L", ");
				szString.append(GC.getPromotionInfo((PromotionTypes)iI).getDescription());
			}
		}
	}

	bFirst = true;
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		bool bValid = false;
		if (kAura.isPromotionRemove((PromotionTypes)iI))
		{
			bValid = true;
		}
		else
		{
			for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if (pUnit->isHasPromotion((PromotionTypes)iJ))
				{
					if (GC.getPromotionInfo((PromotionTypes)iJ).isCombatAuraRemovePromotion(iI))
					{
						bValid = true;
						break;
					}
				}
			}
		}

		if (bValid)
		{
			if (bFirst)
			{
				szString.append(NEWLINE);
				szString.append(gDLL->getText("TXT_KEY_AURA_PROMOTION_REMOVE"));
				szString.append(GC.getPromotionInfo((PromotionTypes)iI).getDescription());
				bFirst = false;
			}
			else
			{
				szString.append(L", ");
				szString.append(GC.getPromotionInfo((PromotionTypes)iI).getDescription());
			}
		}
	}

	/**
	if(iType>0)
	{
		if(pAura->getTier()<GC.getUnitInfo(pUnit->getUnitType()).getTier())
		{
			szString.append(NEWLINE);
			szString.append(gDLL->getText("TXT_KEY_AURA_TIER_REDUCED_EFFECT_ON_UNIT"));
		}
	}
	**/
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


//>>>>	BUGFfH: Added by Denev 2009/09/13
const CvWString CvGameTextMgr::getLinkedText(BonusTypes eBonus, bool bColor, bool bLinks, bool bFontIcon) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getBonusInfo(eBonus).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getBonusInfo(eBonus).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_BONUS_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bFontIcon)
	{
		szTempBuffer.Format(L"%c%s", GC.getBonusInfo(eBonus).getChar(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(BuildingClassTypes eBuildingClass, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	const BuildingTypes eBuilding = (BuildingTypes)GC.getBuildingClassInfo(eBuildingClass).getDefaultBuildingIndex();
	szBuffer.assign(GC.getBuildingClassInfo(eBuildingClass).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getBuildingClassInfo(eBuildingClass).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_BUILDING_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(BuildingTypes eBuilding, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getBuildingInfo(eBuilding).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getBuildingInfo(eBuilding).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_BUILDING_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(CivicTypes eCivic, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getCivicInfo(eCivic).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getCivicInfo(eCivic).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_CIVIC_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(CivilizationTypes eCivilization, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getCivilizationInfo(eCivilization).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getCivilizationInfo(eCivilization).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		const PlayerColorTypes ePlayerColor = (PlayerColorTypes)GC.getCivilizationInfo(eCivilization).getDefaultPlayerColor();
		const NiColorA color(GC.getColorInfo((ColorTypes)GC.getPlayerColorInfo(ePlayerColor).getTextColorType()).getColor());
		const int iRed = (int)(color.r * 255);
		const int iGreen = (int)(color.g * 255);
		const int iBlue = (int)(color.b * 255);
		const int iAlpha = (int)(color.a * 255);

		szTempBuffer.Format(L"<color=%d,%d,%d,%d>%s</color>", iRed, iGreen, iBlue, iAlpha, szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(CorporationTypes eCorporation, bool bColor, bool bLinks, bool bFontIcon) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getCorporationInfo(eCorporation).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getCorporationInfo(eCorporation).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_RELIGION_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bFontIcon)
	{
		szTempBuffer.Format(L"%c%s", GC.getCorporationInfo(eCorporation).getChar(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(DomainTypes eDomain, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getDomainInfo(eDomain).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getDomainInfo(eDomain).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNITCOMBAT_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(FeatureTypes eFeature, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getFeatureInfo(eFeature).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getFeatureInfo(eFeature).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TERRAIN_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(ImprovementTypes eImprovement, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getImprovementInfo(eImprovement).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getImprovementInfo(eImprovement).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_IMPROVEMENT_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(LeaderHeadTypes eLeaderHead, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getLeaderHeadInfo(eLeaderHead).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getLeaderHeadInfo(eLeaderHead).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_LEADERHEAD_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(ProcessTypes eProcess, bool bColor, bool bLinks, bool bFontIcon) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getProcessInfo(eProcess).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getProcessInfo(eProcess).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bFontIcon)
	{
		for (int iCommerce = 0; iCommerce < NUM_COMMERCE_TYPES; iCommerce++)
		{
			if (0 != GC.getProcessInfo(eProcess).getProductionToCommerceModifier(iCommerce))
			{
				szTempBuffer.append(CvWString::format(L"%c", GC.getCommerceInfo((CommerceTypes)iCommerce).getChar()));
			}
		}
		szTempBuffer.append(szBuffer);
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(ProjectTypes eProject, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getProjectInfo(eProject).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getProjectInfo(eProject).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_PROJECT_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(PromotionTypes ePromotion, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getPromotionInfo(ePromotion).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getPromotionInfo(ePromotion).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_PROMOTION_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(ReligionTypes eReligion, bool bColor, bool bLinks, bool bFontIcon) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getReligionInfo(eReligion).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getReligionInfo(eReligion).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_RELIGION_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bFontIcon)
	{
		szTempBuffer.Format(L"%c%s", GC.getReligionInfo(eReligion).getChar(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(SpecialistTypes eSpecialist, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getSpecialistInfo(eSpecialist).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getSpecialistInfo(eSpecialist).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_SPECIALIST_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(SpellTypes eSpell, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getSpellInfo(eSpell).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getSpellInfo(eSpell).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_SPELL_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(TechTypes eTech, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getTechInfo(eTech).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getTechInfo(eTech).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TECH_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(TerrainTypes eTerrain, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getTerrainInfo(eTerrain).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getTerrainInfo(eTerrain).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TERRAIN_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(TraitTypes eTrait, bool bColor, bool bLinks, bool bTemp) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getTraitInfo(eTrait).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getTraitInfo(eTrait).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		if (bTemp)
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR L" %s", TEXT_COLOR("COLOR_PLAYER_ORANGE_TEXT"), szBuffer.c_str(), gDLL->getText("TXT_KEY_TEMPORARY_TRAIT").c_str());
			szBuffer = szTempBuffer;
		}
		else
		{
			szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_TRAIT_TEXT"), szBuffer.c_str());
			szBuffer = szTempBuffer;
		}
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(UnitCombatTypes eUnitCombat, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getUnitCombatInfo(eUnitCombat).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getUnitCombatInfo(eUnitCombat).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNITCOMBAT_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(UnitClassTypes eUnitClass, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getUnitClassInfo(eUnitClass).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getUnitClassInfo(eUnitClass).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

const CvWString CvGameTextMgr::getLinkedText(UnitTypes eUnit, bool bColor, bool bLinks) const
{
	CvWString szBuffer;
	CvWString szTempBuffer;
	szBuffer.assign(GC.getUnitInfo(eUnit).getDescription());
	if (bLinks)
	{
		szTempBuffer.Format(L"<link=%s>%s</link>", ((CvWString)GC.getUnitInfo(eUnit).getType()).c_str(), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	if (bColor)
	{
		szTempBuffer.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_UNIT_TEXT"), szBuffer.c_str());
		szBuffer = szTempBuffer;
	}
	return szBuffer;
}

void CvGameTextMgr::getAvailableCivs(std::list<CivilizationTypes>& aeAvailableCiv, BuildingTypes eBuilding, CivilizationTypes eActiveCiv)
{
	const int iBuildingClass = GC.getBuildingInfo(eBuilding).getBuildingClassType();

	aeAvailableCiv.clear();

	//if active civilization can use the building, adopt it only as an avilable civ.
	if (eActiveCiv != NO_CIVILIZATION
		&& eBuilding == GC.getCivilizationInfo(eActiveCiv).getCivilizationBuildings(iBuildingClass))
	{
		aeAvailableCiv.push_back(eActiveCiv);
	}
	else
	{
		if (GC.getBuildingInfo(eBuilding).getPrereqCiv() != NO_CIVILIZATION)
		{
			aeAvailableCiv.push_back((CivilizationTypes)GC.getBuildingInfo(eBuilding).getPrereqCiv());
		}
		else
		{
			for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); iCiv++)
			{
				if (eBuilding == GC.getCivilizationInfo((CivilizationTypes)iCiv).getCivilizationBuildings(iBuildingClass))
				{
					aeAvailableCiv.push_back((CivilizationTypes)iCiv);
				}
			}
		}
	}
}

void CvGameTextMgr::getAvailableCivs(std::list<CivilizationTypes>& aeAvailableCiv, UnitTypes eUnit, CivilizationTypes eActiveCiv)
{
	const int iUnitClass = GC.getUnitInfo(eUnit).getUnitClassType();

	aeAvailableCiv.clear();

	//if active civilization can use the building, adopt it only as an avilable civ.
	if (eActiveCiv != NO_CIVILIZATION
		&& eUnit == GC.getCivilizationInfo(eActiveCiv).getCivilizationUnits(iUnitClass))
	{
		aeAvailableCiv.push_back(eActiveCiv);
	}
	else
	{
		if (GC.getUnitInfo(eUnit).getPrereqCiv() != NO_CIVILIZATION)
		{
			aeAvailableCiv.push_back((CivilizationTypes)GC.getUnitInfo(eUnit).getPrereqCiv());
		}
		else
		{
			for (int iCiv = 0; iCiv < GC.getNumCivilizationInfos(); iCiv++)
			{
				if (eUnit == GC.getCivilizationInfo((CivilizationTypes)iCiv).getCivilizationUnits(iUnitClass))
				{
					aeAvailableCiv.push_back((CivilizationTypes)iCiv);
				}
			}
		}
	}
}
//<<<<	BUGFfH: End Add

// BUG - Building Actual Effects - start
/*
 * Adds the actual effects of adding a building to the city.
 */
void CvGameTextMgr::setBuildingActualEffects(CvWStringBuffer& szBuffer, CvWString& szStart, BuildingTypes eBuilding, CvCity* pCity, bool bNewLine)
{
	if (NULL != pCity)
	{
		bool bStarted = false;

		// Happiness
		int iGood = 0;
		int iBad = 0;
		int iHappiness = pCity->getAdditionalHappinessByBuilding(eBuilding, iGood, iBad);
		bStarted = setResumableGoodBadChangeHelp(szBuffer, szStart, L": ", L"", iGood, gDLL->getSymbolID(HAPPY_CHAR), iBad, gDLL->getSymbolID(UNHAPPY_CHAR), false, bNewLine, bStarted);

		// Health
		iGood = 0;
		iBad = 0;
		int iHealth = pCity->getAdditionalHealthByBuilding(eBuilding, iGood, iBad);
		bStarted = setResumableGoodBadChangeHelp(szBuffer, szStart, L": ", L"", iGood, gDLL->getSymbolID(HEALTHY_CHAR), iBad, gDLL->getSymbolID(UNHEALTHY_CHAR), false, bNewLine, bStarted);

		// Yield
		int aiYields[NUM_YIELD_TYPES];
		for (int iI = 0; iI < NUM_YIELD_TYPES; ++iI)
		{
			aiYields[iI] = pCity->getAdditionalYieldByBuilding((YieldTypes)iI, eBuilding);
		}
		bStarted = setResumableYieldChangeHelp(szBuffer, szStart, L": ", L"", aiYields, false, bNewLine, bStarted);

		// Commerce
		int aiCommerces[NUM_COMMERCE_TYPES];
		for (int iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
		{
			aiCommerces[iI] = pCity->getAdditionalCommerceTimes100ByBuilding((CommerceTypes)iI, eBuilding);
		}
		// Maintenance - add to gold
		aiCommerces[COMMERCE_GOLD] += pCity->getSavedMaintenanceTimes100ByBuilding(eBuilding);
		bStarted = setResumableCommerceTimes100ChangeHelp(szBuffer, szStart, L": ", L"", aiCommerces, bNewLine, bStarted);

		// Great People
		int iGreatPeopleRate = pCity->getAdditionalGreatPeopleRateByBuilding(eBuilding);
		bStarted = setResumableValueChangeHelp(szBuffer, szStart, L": ", L"", iGreatPeopleRate, gDLL->getSymbolID(GREAT_PEOPLE_CHAR), false, bNewLine, bStarted);
	}
}
// BUG - Building Actual Effects - end

// BUG - Building Additional Happiness - start
bool CvGameTextMgr::setBuildingAdditionalHappinessHelp(CvWStringBuffer& szBuffer, const CvCity& city, const CvWString& szStart, bool bStarted)
{
	CvWString szLabel;

	// BUG jdog5000 - need to verify building type is correct for this player to avoid asserts - start
	for (int i = 0; i < GC.getNumBuildingClassInfos(); i++)
	{

		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(GET_PLAYER(city.getOwnerINLINE()).getCivilizationType()).getCivilizationBuildings((BuildingClassTypes)i);

		if (eBuilding != NO_BUILDING && city.canConstruct(eBuilding, false, true, false))
		{
			int iGood = 0, iBad = 0, iChange = city.getAdditionalHappinessByBuilding(eBuilding, iGood, iBad);

			if (iGood != 0 || iBad != 0)
			{
				if (!bStarted)
				{
					szBuffer.append(szStart);
					bStarted = true;
				}

				szLabel.Format(SETCOLR L"%s" ENDCOLR, TEXT_COLOR("COLOR_BUILDING_TEXT"), GC.getBuildingInfo(eBuilding).getDescription());
				setResumableGoodBadChangeHelp(szBuffer, szLabel, L": ", L"", iGood, gDLL->getSymbolID(HAPPY_CHAR), iBad, gDLL->getSymbolID(UNHAPPY_CHAR), false, true);
			}
		}
	}

	return bStarted;
}
// BUG - Building Additional Happiness - end


// BUG - Resumable Value Change Help - start
void CvGameTextMgr::setYieldChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piYieldChange, bool bPercent, bool bNewLine)
{
	setResumableYieldChangeHelp(szBuffer, szStart, szSpace, szEnd, piYieldChange, bPercent, bNewLine);
}

/*
 * Adds the ability to pass in and get back the value of bStarted so that
 * it can be used with other setResumable<xx>ChangeHelp() calls on a single line.
 */
bool CvGameTextMgr::setResumableYieldChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piYieldChange, bool bPercent, bool bNewLine, bool bStarted)
{
	CvWString szTempBuffer;
	//	bool bStarted;
	int iI;

	//	bStarted = false;

	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		if (piYieldChange[iI] != 0)
		{
			if (!bStarted)
			{
				if (bNewLine)
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
				}
				szTempBuffer += CvWString::format(L"%s%s%s%d%s%c",
					szStart.GetCString(),
					szSpace.GetCString(),
					piYieldChange[iI] > 0 ? L"+" : L"",
					piYieldChange[iI],
					bPercent ? L"%" : L"",
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
			else
			{
				szTempBuffer.Format(L", %s%d%s%c",
					piYieldChange[iI] > 0 ? L"+" : L"",
					piYieldChange[iI],
					bPercent ? L"%" : L"",
					GC.getYieldInfo((YieldTypes)iI).getChar());
			}
			szBuffer.append(szTempBuffer);

			bStarted = true;
		}
	}

	if (bStarted)
	{
		szBuffer.append(szEnd);
	}

	// added
	return bStarted;
}

void CvGameTextMgr::setCommerceChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piCommerceChange, bool bPercent, bool bNewLine)
{
	setResumableCommerceChangeHelp(szBuffer, szStart, szSpace, szEnd, piCommerceChange, bPercent, bNewLine);
}

/*
 * Adds the ability to pass in and get back the value of bStarted so that
 * it can be used with other setResumable<xx>ChangeHelp() calls on a single line.
 */
bool CvGameTextMgr::setResumableCommerceChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piCommerceChange, bool bPercent, bool bNewLine, bool bStarted)
{
	CvWString szTempBuffer;
	//	bool bStarted;
	int iI;

	//	bStarted = false;

	for (iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		if (piCommerceChange[iI] != 0)
		{
			if (!bStarted)
			{
				if (bNewLine)
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
				}
				szTempBuffer += CvWString::format(L"%s%s%s%d%s%c", szStart.GetCString(), szSpace.GetCString(), ((piCommerceChange[iI] > 0) ? L"+" : L""), piCommerceChange[iI], ((bPercent) ? L"%" : L""), GC.getCommerceInfo((CommerceTypes)iI).getChar());
			}
			else
			{
				szTempBuffer.Format(L", %s%d%s%c", ((piCommerceChange[iI] > 0) ? L"+" : L""), piCommerceChange[iI], ((bPercent) ? L"%" : L""), GC.getCommerceInfo((CommerceTypes)iI).getChar());
			}
			szBuffer.append(szTempBuffer);

			bStarted = true;
		}
	}

	if (bStarted)
	{
		szBuffer.append(szEnd);
	}

	// added
	return bStarted;
}

/*
 * Displays float values by dividing each value by 100.
 */
void CvGameTextMgr::setCommerceTimes100ChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piCommerceChange, bool bNewLine, bool bStarted)
{
	setResumableCommerceTimes100ChangeHelp(szBuffer, szStart, szSpace, szEnd, piCommerceChange, bNewLine);
}

/*
 * Adds the ability to pass in and get back the value of bStarted so that
 * it can be used with other setResumable<xx>ChangeHelp() calls on a single line.
 */
bool CvGameTextMgr::setResumableCommerceTimes100ChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, const int* piCommerceChange, bool bNewLine, bool bStarted)
{
	CvWString szTempBuffer;

	for (int iI = 0; iI < NUM_COMMERCE_TYPES; ++iI)
	{
		int iChange = piCommerceChange[iI];
		if (iChange != 0)
		{
			if (!bStarted)
			{
				if (bNewLine)
				{
					szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
				}
				szTempBuffer += CvWString::format(L"%s%s", szStart.GetCString(), szSpace.GetCString());
				bStarted = true;
			}
			else
			{
				szTempBuffer.Format(L", ");
			}
			szBuffer.append(szTempBuffer);

			if (iChange % 100 == 0)
			{
				szTempBuffer.Format(L"%s%d%c", iChange >= 0 ? L"+" : L"-", iChange / 100, GC.getCommerceInfo((CommerceTypes)iI).getChar());
			}
			else
			{
				if (iChange >= 0)
				{
					szBuffer.append(L"+");
				}
				else
				{
					iChange = -iChange;
					szBuffer.append(L"-");
				}
				szTempBuffer.Format(L"%d.%02d%c", iChange / 100, iChange % 100, GC.getCommerceInfo((CommerceTypes)iI).getChar());
			}
			szBuffer.append(szTempBuffer);
		}
	}

	if (bStarted)
	{
		szBuffer.append(szEnd);
	}

	return bStarted;
}

/*
 * Adds the ability to pass in and get back the value of bStarted so that
 * it can be used with other setResumable<xx>ChangeHelp() calls on a single line.
 */
bool CvGameTextMgr::setResumableGoodBadChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, int iGood, int iGoodSymbol, int iBad, int iBadSymbol, bool bPercent, bool bNewLine, bool bStarted)
{
	bStarted = setResumableValueChangeHelp(szBuffer, szStart, szSpace, szEnd, iGood, iGoodSymbol, bPercent, bNewLine, bStarted);
	bStarted = setResumableValueChangeHelp(szBuffer, szStart, szSpace, szEnd, iBad, iBadSymbol, bPercent, bNewLine, bStarted);

	return bStarted;
}

/*
 * Adds the ability to pass in and get back the value of bStarted so that
 * it can be used with other setResumable<xx>ChangeHelp() calls on a single line.
 */
bool CvGameTextMgr::setResumableValueChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, int iValue, int iSymbol, bool bPercent, bool bNewLine, bool bStarted)
{
	CvWString szTempBuffer;

	if (iValue != 0)
	{
		if (!bStarted)
		{
			if (bNewLine)
			{
				szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
			}
			szTempBuffer += CvWString::format(L"%s%s", szStart.GetCString(), szSpace.GetCString());
		}
		else
		{
			szTempBuffer = L", ";
		}
		szBuffer.append(szTempBuffer);

		szTempBuffer.Format(L"%+d%s%c", iValue, bPercent ? L"%" : L"", iSymbol);
		szBuffer.append(szTempBuffer);

		bStarted = true;
	}

	return bStarted;
}

/*
 * Adds the ability to pass in and get back the value of bStarted so that
 * it can be used with other setResumable<xx>ChangeHelp() calls on a single line.
 */
bool CvGameTextMgr::setResumableValueTimes100ChangeHelp(CvWStringBuffer& szBuffer, const CvWString& szStart, const CvWString& szSpace, const CvWString& szEnd, int iValue, int iSymbol, bool bNewLine, bool bStarted)
{
	CvWString szTempBuffer;

	if (iValue != 0)
	{
		if (!bStarted)
		{
			if (bNewLine)
			{
				szTempBuffer.Format(L"\n%c", gDLL->getSymbolID(BULLET_CHAR));
			}
			szTempBuffer += CvWString::format(L"%s%s", szStart.GetCString(), szSpace.GetCString());
		}
		else
		{
			szTempBuffer = L", ";
		}
		szBuffer.append(szTempBuffer);

		if (iValue % 100 == 0)
		{
			szTempBuffer.Format(L"%+d%c", iValue / 100, iSymbol);
		}
		else
		{
			if (iValue >= 0)
			{
				szBuffer.append(L"+");
			}
			else
			{
				iValue = -iValue;
				szBuffer.append(L"-");
			}
			szTempBuffer.Format(L"%d.%02d%c", iValue / 100, iValue % 100, iSymbol);
		}
		szBuffer.append(szTempBuffer);

		bStarted = true;
	}

	return bStarted;
}
// BUG - Resumable Value Change Help - end

// Better AI Debug (Skyre)

const wchar* CvGameTextMgr::getGroupflagName(int iGroupflag) const
{
	FAssert(iGroupflag != GROUPFLAG_NONE);

	switch (iGroupflag)
	{
	case GROUPFLAG_PERMDEFENSE:
		return L"GROUPFLAG_PERMDEFENSE";

	case GROUPFLAG_PERMDEFENSE_NEW:
		return L"GROUPFLAG_PERMDEFENSE_NEW";

	case GROUPFLAG_PATROL:
		return L"GROUPFLAG_PATROL";

	case GROUPFLAG_PATROL_NEW:
		return L"GROUPFLAG_PATROL_NEW";

	case GROUPFLAG_EXPLORE:
		return L"GROUPFLAG_EXPLORE";

	case GROUPFLAG_DEFENSE_NEW:
		return L"GROUPFLAG_DEFENSE_NEW";

	case GROUPFLAG_DEFENSE:
		return L"GROUPFLAG_DEFENSE";

	case GROUPFLAG_HERO:
		return L"GROUPFLAG_HERO";

	case GROUPFLAG_CONQUEST:
		return L"GROUPFLAG_CONQUEST";

	case GROUPFLAG_SETTLERGROUP:
		return L"GROUPFLAG_SETTLERGROUP";

	case GROUPFLAG_HNGROUP:
		return L"GROUPFLAG_HNGROUP";

	case GROUPFLAG_PICKUP_EQUIPMENT:
		return L"GROUPFLAG_PICKUP_EQUIPMENT";

	case GROUPFLAG_FEASTING:
		return L"GROUPFLAG_FEASTING";

	case GROUPFLAG_INQUISITION:
		return L"GROUPFLAG_INQUISITION";

	case GROUPFLAG_PILLAGE:
		return L"GROUPFLAG_PILLAGE";

	case GROUPFLAG_NAVAL_SETTLE:
		return L"GROUPFLAG_NAVAL_SETTLE";

	case GROUPFLAG_NAVAL_SETTLE_NEW:
		return L"GROUPFLAG_NAVAL_SETTLE_NEW";

	case GROUPFLAG_NAVAL_SETTLE_PICKUP:
		return L"GROUPFLAG_NAVAL_SETTLE_PICKUP";

	case GROUPFLAG_NAVAL_SETTLE_REINFORCEMENTS:
		return L"GROUPFLAG_NAVAL_SETTLE_REINFORCEMENTS";

	case GROUPFLAG_NAVAL_INVASION:
		return L"GROUPFLAG_NAVAL_INVASION";

	case GROUPFLAG_NAVAL_INVASION_NEW:
		return L"GROUPFLAG_NAVAL_INVASION_NEW";

	case GROUPFLAG_ANIMAL_CONSTRUCTBUILDING:
		return L"GROUPFLAG_ANIMAL_CONSTRUCTBUILDING";

	case GROUPFLAG_SVARTALFAR_KIDNAP:
		return L"GROUPFLAG_SVARTALFAR_KIDNAP";

	case GROUPFLAG_ESUS_MISSIONARY:
		return L"GROUPFLAG_ESUS_MISSIONARY";

	case GROUPFLAG_SUICIDE_SUMMON:
		return L"GROUPFLAG_SUICIDE_SUMMON";

	case GROUPFLAG_AWAKENED:
		return L"GROUPFLAG_AWAKENED";

	case GROUPFLAG_CREEPER:
		return L"GROUPFLAG_CREEPER";

	case GROUPFLAG_MERCENARY_HEADHUNTER:
		return L"GROUPFLAG_MERCENARY_HEADHUNTER";

	case GROUPFLAG_MERCENARY_RAIDER:
		return L"GROUPFLAG_MERCENARY_RAIDER";

	case GROUPFLAG_MERCENARY_PILLAGER:
		return L"GROUPFLAG_MERCENARY_PILLAGER";

	default:
		return L"GROUPFLAG_NONE";
	}
}

// End Better AI Debug

