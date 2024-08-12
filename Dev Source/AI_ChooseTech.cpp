// player.cpp

#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvArea.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvRandom.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CvPlayerAI.h"
#include "CvPlayer.h"
#include "CvArtFileMgr.h"
#include "CvDiploParameters.h"
#include "CvInitCore.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "CvPopupInfo.h"
#include "FProfiler.h"
#include "CvGameTextMgr.h"
#include "CyCity.h"
#include "CyPlot.h"

#define MAX_TECH_VARIATION 3

int CvPlayerAI::AI_techValueNew(TechTypes iTech, bool bIgnoreCost) const
{
	int iValue = 0;
	int iResearchTurnsLeft=getResearchTurnsLeft(iTech,true);
	iResearchTurnsLeft*=100;
	iResearchTurnsLeft/=std::max(1,GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getResearchPercent());

	iValue+=AI_techValueMelee(iTech);
	iValue+=AI_techValueEngineering(iTech);
	iValue+=AI_techValueRecon(iTech);
	iValue+=AI_techValueRanged(iTech);
	iValue+=AI_techValueMounted(iTech);
	iValue+=AI_techValueMilitary(iTech);
	iValue+=AI_techValueMagic(iTech);
	iValue+=AI_techValueReligion(iTech);
	if(iValue>0)
	{
		iValue+=AI_techValueMilitaryUnit(iTech);
	}
	iValue+=AI_techValueHappy(iTech);
	iValue+=AI_techValueNaval(iTech);
	iValue+=AI_techValueMetal(iTech);	

	iValue+=AI_techValueUniqueUnit(iTech);
	iValue+=AI_techValueResource(iTech);
	if(iResearchTurnsLeft<10)
	{
		iValue+=AI_techValueBuilds(iTech);
	}

	if(GC.getGameINLINE().getElapsedGameTurns() < 250) {
		if(iResearchTurnsLeft > 50)
		{
			iValue /= 10;
		}
	}
	if(iResearchTurnsLeft > 100)
	{
		iValue /= 10;
	}

	iValue+=AI_techValueGold(iTech);
	iValue+=AI_techValueScience(iTech);

	iValue*=100;

	if(!bIgnoreCost)
	{
		iValue/=1+getResearchTurnsLeft(iTech,true);
	}

	return iValue;
}
TechTypes CvPlayerAI::AI_needMilitaryTech() const{
	bool bForceMilitaryTech = false;
	TechTypes iMilitaryTech = NO_TECH;
	//Need to defeat a lot of village
	int iCountGroup = 0;
	int iLoop;
	for(CvAIGroup* pAIGroup = firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_DESTROY_LAIR)
		{
			//if(getArea()==pAIGroup->getMissionArea()) //Maybe this means its nearby?
			//{
				if(pAIGroup->UnitsNeeded()>0) //This means is not strenght enough
				{
					iCountGroup+=1;
				}
			//}
		}
	}	

	int iPower = GET_TEAM(getTeam()).getPower(true);
	//int iThemPower = 0;
	//Other AI has more strength
	for (int iLoop = 0; iLoop < MAX_CIV_TEAMS; iLoop++)
	{
		if (getTeam() != iLoop)
		{
			if (GET_TEAM((TeamTypes)iLoop).isAlive())
			{
				if (GET_TEAM(getTeam()).isHasMet((TeamTypes)iLoop))
				{
					int iPowerValue = GET_TEAM((TeamTypes)iLoop).getPower(true);
					if (iPowerValue > iPower){
						iCountGroup+=1;
					}
				}
			}
		}
	}

	if(iCountGroup > 0){
		bForceMilitaryTech = true;
	}
	
	if(bForceMilitaryTech){
		int iLoop;
		int totalUnit = 0;
		int incUnit = 0;
		for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
		{
			if(GC.getUnitInfo(pLoopUnit->getUnitType()).getCombat()>0 && GC.getUnitInfo(pLoopUnit->getUnitType()).getCombatDefense()>0)
			{
				++totalUnit;
				if(GC.getUnitInfo(pLoopUnit->getUnitType()).isScaleWithTech())
				{
					++incUnit;
				}
			}
		}

		TechTypes iTechIncrease = AI_nextTechForIncreaseStrengthOfRegularUnits();
		int iValueTechIncrease = (iTechIncrease != NO_TECH)?getResearchTurnsLeft(iTechIncrease,true):-1;
		int iRandTechIncrease = iValueTechIncrease - GC.getGame().getSorenRandNum(10, "AI_needMilitaryTech");

		TechTypes iTechTier3 = AI_nextTechForTier(3);
		int iValueTechTier3 = (iTechTier3 != NO_TECH)?getResearchTurnsLeft(iTechTier3,true):-1;
		int iRandTier3 = iValueTechTier3 - GC.getGame().getSorenRandNum(10, "AI_needMilitaryTech");

		TechTypes iTechTier4 = AI_nextTechForTier(4);
		int iValueTechTier4 = (iTechTier4 != NO_TECH)?getResearchTurnsLeft(iTechTier4,true):-1;
		int iRandTier4 = iValueTechTier4 - GC.getGame().getSorenRandNum(10, "AI_needMilitaryTech");

		bool bConsiderTechIncrease = (incUnit >= (totalUnit / 2));

		if(iValueTechIncrease >= 0 &&
			bConsiderTechIncrease &&
			(iValueTechTier3 == -1 || iRandTechIncrease <= iRandTier3) &&
			(iValueTechTier4 == -1 || iRandTechIncrease <= iRandTier4) &&
			iValueTechIncrease <= (30 + iCountGroup)
		){
			//Buy a +2 tech if it cost reasonable and less than a tier3 or tier4 unit tech
			//And if it increase the strength of at least half of your combat units
			iMilitaryTech = iTechIncrease;
		}else if(iValueTechTier3 >= 0 &&
			(iValueTechIncrease == -1 || !bConsiderTechIncrease || iRandTier3 < iRandTechIncrease) &&
			(iValueTechTier4 == -1 || iRandTier3 < iRandTier4) &&
			iValueTechTier3 <= (30 + iCountGroup)
		){
			//Buy a tier3 unit tech if it cost reasonable and less than a +2 or tier4 unit tech
			iMilitaryTech = iTechTier3;
		}else if(iValueTechTier4 >= 0 &&
			(iValueTechIncrease == -1 || !bConsiderTechIncrease || iRandTier4 < iRandTechIncrease) &&
			(iValueTechTier3 == -1 || iRandTier4 < iRandTier3) &&
			iValueTechTier4 <= (30 + iCountGroup)
		){
			//Buy a tier4 unit tech if it cost reasonable and less than a +2 or tier3 unit tech
			iMilitaryTech = iTechTier4;
		}

/*
		int iMilitaryTechBestValue = MAX_INT;

		if(iValueTechIncrease >= 0 && 
			(iValueTechTier3 == -1 || iValueTechIncrease <= iValueTechTier3) && 
			(iValueTechTier4 == -1 || iValueTechIncrease <= iValueTechTier4) &&
			iValueTechIncrease <= (30 + iCountGroup)
		){
			//Buy a +2 tech if it cost reasonable and less than a tier3 or tier4 unit tech
			//And if it increase the strength of at least half of your combat units
			if (incUnit >= (totalUnit / 2))
			{
				iRandTechIncrease = iValueTechTier4 - GC.getGame().getSorenRandNum(10, "AI_needMilitaryTech");
				if(iRandTechIncrease < iMilitaryTechBestValue)
				{
					iMilitaryTechBestValue = iRandTechIncrease;
					iMilitaryTech = iTechIncrease;
				}
			}
		}else if(iValueTechTier3 >= 0 &&
			(iValueTechIncrease == -1 || iValueTechTier3 < iValueTechIncrease) &&
			(iValueTechIncrease == -1 || iValueTechTier4 < iValueTechIncrease) &&
			iValueTechTier3 <= (30 + iCountGroup)
		){
			//Buy a tier3 unit tech if it cost reasonable and less than a +2 or tier4 unit tech
			iRandTier3 = iValueTechTier4 - GC.getGame().getSorenRandNum(10, "AI_needMilitaryTech");
			//iMilitaryTech = iTechTier3;
			if(iRandTier3 < iMilitaryTechBestValue)
			{
				iMilitaryTechBestValue = iRandTier3;
				iMilitaryTech = iTechTier3;
			}
		}else if(iValueTechTier4 >= 0 &&
			(iValueTechTier4 == -1 || iValueTechTier4 < iValueTechTier3) &&
			(iValueTechTier4 == -1 || iValueTechTier4 < iValueTechIncrease) &&
			iValueTechTier4 <= (30 + iCountGroup)
		){
			//Buy a tier4 unit tech if it cost reasonable and less than a +2 or tier3 unit tech
			iRandTier4 = iValueTechTier4 - GC.getGame().getSorenRandNum(10, "AI_needMilitaryTech");
			//iMilitaryTech = iTechTier4;
			if(iRandTier4 < iMilitaryTechBestValue)
			{
				iMilitaryTechBestValue = iRandTier4;
				iMilitaryTech = iTechTier4;
			}
		}
*/

		/* DEBUG */
		if(isOOSLogging())
		{
			oosLog("AIResearch"
				,"Turn: %d,PlayerID: %d,Player: %S,MilitaryTech: %S,IncreaseTech: %S,IncreaseTechTurns: %d (%d),[%d/%d],TierTech3: %S,TierTechTurns3: %d (%d),TierTech4: %S,TierTechTurns4: %d (%d)\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getID()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,iMilitaryTech!=NO_TECH?GC.getTechInfo(iMilitaryTech).getDescription():L"NO_TECH"
				,iTechIncrease!=NO_TECH?GC.getTechInfo(iTechIncrease).getDescription():L"none"
				,iValueTechIncrease
				,iRandTechIncrease
				,incUnit
				,totalUnit
				,iTechTier3!=NO_TECH?GC.getTechInfo(iTechTier3).getDescription():L"none"
				,iValueTechTier3
				,iRandTier3
				,iTechTier4!=NO_TECH?GC.getTechInfo(iTechTier4).getDescription():L"none"
				,iValueTechTier4
				,iRandTier4
			);
		}
		/* DEBUG */
	}
	return iMilitaryTech;
}
TechTypes CvPlayerAI::AI_nextTechForIncreaseStrengthOfRegularUnits() const{
	int iBestValue = 10000;
	TechTypes iBestTech = NO_TECH;

	for(int iI=0;iI<GC.getNumTechInfos();iI++)
	{
		if(canEverResearch((TechTypes)iI))
		{
			if(GC.getTechInfo((TechTypes)iI).getIncreaseStrengthOfRegularUnits() > 0)
			{
				if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)iI)))
				{
					if(GC.getTechInfo((TechTypes)iI).getPrereqReligion()==NO_RELIGION || GC.getTechInfo((TechTypes)iI).getPrereqReligion()==getStateReligion())
					{
						int iValue=getResearchTurnsLeft((TechTypes)iI,true);
						if(iValue < iBestValue)
						{
							iBestValue=iValue;
							iBestTech=(TechTypes)iI;
						}
					}
				}
			}
		}
	}
	return iBestTech;
}
TechTypes CvPlayerAI::AI_nextTechForTier(int iTier) const{
	int iBestValue = 10000;
	TechTypes iBestTech = NO_TECH;

	for(int iI=0;iI<GC.getNumTechInfos();iI++)
	{
		if(canEverResearch((TechTypes)iI))
		{
			//SpyFanatic: evaluate all tech as some units get unlock new units even if does not give +2 bonus (e.g. Luchuirp)
			//if(GC.getTechInfo((TechTypes)iI).getIncreaseStrengthOfRegularUnits() > 0)
			//{
				if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)iI)))
				{
					if(GC.getTechInfo((TechTypes)iI).getPrereqReligion()==NO_RELIGION || GC.getTechInfo((TechTypes)iI).getPrereqReligion()==getStateReligion())
					{
						for(int iJ=0;iJ<GC.getNumUnitClassInfos();iJ++)
						{
							UnitTypes eUnit=(UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ);
							if(eUnit!=NO_UNIT && GC.getUnitInfo(eUnit).getPrereqAndTech()==iI)
							{
								if(GC.getUnitInfo(eUnit).getProductionCost()>0)
								{
									if(GC.getUnitInfo(eUnit).getCombat()>0 && GC.getUnitInfo(eUnit).getCombatDefense()>0)
									{
										ReligionTypes eReligion=(ReligionTypes)GC.getUnitInfo(eUnit).getPrereqReligion();
										CivilizationTypes eCivilization=(CivilizationTypes)GC.getUnitInfo(eUnit).getPrereqCiv();
										if((eReligion==NO_RELIGION || getStateReligion()==eReligion) &&
											(eCivilization==NO_CIVILIZATION || getCivilizationType()==eCivilization))
										{
											if(GC.getUnitInfo(eUnit).getTier()==iTier)  
											{
												int iValue=getResearchTurnsLeft((TechTypes)iI,true);
												if(iValue < iBestValue)
												{
													iBestValue=iValue;
													iBestTech=(TechTypes)iI;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			//}
		}
	}
	
	return iBestTech;
}

bool CvPlayerAI::AI_isMilitaryTierNeeded(int iTier) const
{
	for(int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if ((GET_TEAM(getTeam()).isHasTech((TechTypes)iI)))
		{
			for(int iJ=0;iJ<GC.getNumUnitClassInfos();iJ++)
			{
				UnitTypes eUnit=(UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ);
				if(eUnit!=NO_UNIT && GC.getUnitInfo(eUnit).getPrereqAndTech()==iI)
				{
					if(GC.getUnitInfo(eUnit).getProductionCost()>0)
					{
						if(GC.getUnitInfo(eUnit).getCombat()>0 && GC.getUnitInfo(eUnit).getCombatDefense()>0)
						{
							ReligionTypes eReligion=(ReligionTypes)GC.getUnitInfo(eUnit).getPrereqReligion();
							CivilizationTypes eCivilization=(CivilizationTypes)GC.getUnitInfo(eUnit).getPrereqCiv();
							if((eReligion==NO_RELIGION || getStateReligion()==eReligion) &&
								(eCivilization==NO_CIVILIZATION || getCivilizationType()==eCivilization))
							{
								if(GC.getUnitInfo(eUnit).getTier() >= iTier)  
								{
									return false;
								}
							}
						}
					}
				}
			}
		}
	}

	return true;
}

TechTypes CvPlayerAI::AI_needEarlyTech() const
{
	int iValue=0;
	int iBestValue=0;
	TechTypes iBestTech=NO_TECH;

	const char* earlyTech[4] = {"TECH_WARRIOR_CASTE", "TECH_EXPLORATION", "TECH_EDUCATION", "TECH_BARTER"};
	for(int iI=0;iI < 4;iI++)
	{
		TechTypes eEarlyTech = (TechTypes)GC.getInfoTypeForString(earlyTech[iI]);
		if(eEarlyTech != NO_TECH && !GET_TEAM(getTeam()).isHasTech(eEarlyTech))
		{
			iValue = GC.getGameINLINE().getSorenRandNum(1000,"AI_needEarlyTech");

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				iBestTech=eEarlyTech;
			}
		}
	}
	iBestValue = 0;
	iValue = 0;

	//FirstPass: Access to Tier2 Military Units
	//bool bTier2MilitaryNeeded = AI_isMilitaryTierNeeded(2);
	if(iBestTech == NO_TECH && AI_isMilitaryTierNeeded(2))
	//if(iBestTech == NO_TECH && bTier2MilitaryNeeded)
	{
		for(int iI=0;iI<GC.getNumTechInfos();iI++)
		{
			if(canEverResearch((TechTypes)iI))
			{
				if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)iI)))
				{
					if(GC.getTechInfo((TechTypes)iI).getPrereqReligion()==NO_RELIGION || GC.getTechInfo((TechTypes)iI).getPrereqReligion()==getStateReligion())
					{
						for(int iJ=0;iJ<GC.getNumUnitClassInfos();iJ++)
						{
							UnitTypes eUnit=(UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ);
							if(eUnit!=NO_UNIT && GC.getUnitInfo(eUnit).getPrereqAndTech()==iI)
							{
								if(GC.getUnitInfo(eUnit).getProductionCost()>0)
								{
									if(GC.getUnitInfo(eUnit).getCombat()>0 && GC.getUnitInfo(eUnit).getCombatDefense()>0)
									{
										ReligionTypes eReligion=(ReligionTypes)GC.getUnitInfo(eUnit).getPrereqReligion();
										CivilizationTypes eCivilization=(CivilizationTypes)GC.getUnitInfo(eUnit).getPrereqCiv();
										if((eReligion==NO_RELIGION || getStateReligion()==eReligion) &&
											(eCivilization==NO_CIVILIZATION || getCivilizationType()==eCivilization))
										{
											if(GC.getUnitInfo(eUnit).getTier()==2)  
											{
												iValue=GC.getUnitInfo(eUnit).getAIWeight();
												iValue+=AI_techValueMelee((TechTypes)iI);
												iValue+=AI_techValueRecon((TechTypes)iI);
												iValue+=AI_techValueRanged((TechTypes)iI);
												iValue+=AI_techValueMounted((TechTypes)iI);
												iValue+=AI_techValueMagic((TechTypes)iI);
												iValue+=AI_techValueEngineering((TechTypes)iI);
												iValue+=AI_techValueReligion((TechTypes)iI);

												iValue/=std::max(1,GC.getTechInfo((TechTypes)iI).getResearchCost());

												if(iValue>iBestValue)
												{
													iBestValue=iValue;
													iBestTech=(TechTypes)iI;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if(iBestTech!=NO_TECH)
	{
		if(isOOSLogging())
		{
			oosLog("AIResearch"
				,"Turn: %d,PlayerID: %d,Player: %S,Tech: %S,AI_needEarlyTech\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getID()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getTechInfo(iBestTech).getDescription()
			);
		}
		return iBestTech;
	}

	return NO_TECH;
}

int CvPlayerAI::AI_techValueMelee(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_MELEE;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueEngineering(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_ENGINEERING;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);
	if(AI_getSiegeStrategy()==STRATEGY_SIEGE_NORMAL)
	{
		iFlavorValue+=20;
	}
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueRecon(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_RECON;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueRanged(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_RANGED;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueMounted(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_MOUNTED;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueMilitary(TechTypes iTech) const
{
	int allUnitsStrongerValue = GC.getTechInfo(iTech).getIncreaseStrengthOfRegularUnits() * 1000;

	int iFlavorType=FLAVOR_MILITARY;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return allUnitsStrongerValue;

	int iFlavorValue = getFlavorValue(iFlavorType);

	// for now, hardcode it to 15 (every civ/leader needs this, similar to gold/research
	iFlavorValue = 15;

	iFlavorValue *= GC.getTechInfo(iTech).getFlavorValue(iFlavorType);
	return iFlavorValue + allUnitsStrongerValue;
}

int CvPlayerAI::AI_techValueMagic(TechTypes iTech) const
{
	int iFlavorType = FLAVOR_MAGIC;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType) <= 0)
		return 0;

	int iFlavorValue = getFlavorValue(iFlavorType);

	iFlavorValue = std::max(10, iFlavorValue); //every AI Leader should value Magic to some degree
	iFlavorValue *= GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	//Tower of Mastery
	/**
	if(AI_getVictoryStrategyStage(AIVICTORYSTRATEGY_TOWER_OF_MASTERY) >= AI_TOWER_OF_MASTERY_LESSER_TOWERS)
	{
		return 10000;
	}
	**/

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueScience(TechTypes iTech) const
{
	int iFlavorType = FLAVOR_SCIENCE;

	if(iFlavorType==-1 || GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
	{
		return 0;
	}

	int iFlavorValue = getFlavorValue(iFlavorType) + 20;
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	//TODO: less generic and more focused on Priviledge Class, as its only civicoption that gives malus are City Governours and Military State

	bool bHasOptionNoMalusResearch = false;
	int iCivicResearchValue = 0;
	int iCIVICOPTION_GOVERNMENT = GC.getInfoTypeForString("CIVICOPTION_GOVERNMENT");
	for (int iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
	{
		if (GC.getCivicInfo((CivicTypes)iJ).getCivicOptionType() == iCIVICOPTION_GOVERNMENT)
		{
			if(GC.getCivicInfo((CivicTypes)iJ).getCommerceModifier(COMMERCE_RESEARCH) >= 0)
			{
				if (canDoCivics((CivicTypes)iJ))
				{
					//There is already a civic that can give you no malus in research!
					bHasOptionNoMalusResearch = true;
					break;
				}
				else
				{
					//There are other possibilities for which player cannot do civic a part from the tech prereq and most hated civic, but currently there is only that!
					if(GC.getCivicInfo((CivicTypes)iJ).getTechPrereq() == iTech)
					{
						if(
							!(GC.getLeaderHeadInfo(getLeaderType()).getHatedCivic()==(CivicTypes)iJ)
						)
						{
							//The tech unlock a Civic that allows a non-negative research
							iCivicResearchValue += AI_civicValue((CivicTypes)iJ);
						}
					}
				}
			}
		}
	}
	if(!bHasOptionNoMalusResearch){
		iFlavorValue+=iCivicResearchValue;
	}
	//SpyFanatic: Add value of unlocked civic to tech value if all currently available civic gives a malus in research (and if unlocked civic does not)
	/*
	for (int iI = 0; iI < GC.getNumCivicOptionInfos(); iI++)
	{
		bool bHasOptionNoMalusResearch = false;
		int iCivicResearchValue = 0;
		for (int iJ = 0; iJ < GC.getNumCivicInfos(); iJ++)
		{
			if (GC.getCivicInfo((CivicTypes)iJ).getCivicOptionType() == (CivicOptionTypes)iI)
			{
				if (canDoCivics((CivicTypes)iJ))
				{
					//There is already a civic that can give you no malus in research!
					if(GC.getCivicInfo((CivicTypes)iJ).getCommerceModifier(COMMERCE_RESEARCH) >= 0)
					{
						bHasOptionNoMalusResearch = true;
						break;
					}
				}
				else
				{
					//TODO: there are other possibilities for which player cannot do civic a part from the tech prereq, for example most hated civic!!
					if(GC.getCivicInfo((CivicTypes)iJ).getTechPrereq() == iTech)
					{
						if(GC.getCivicInfo((CivicTypes)iJ).getCommerceModifier(COMMERCE_RESEARCH) >= 0)
						{
							//The tech unlock a Civic that allows a non-negative research
							iCivicResearchValue += AI_civicValue((CivicTypes)iJ);
						}
					}
				}
			}
		}
		if(!bHasOptionNoMalusResearch){
			iFlavorValue+=iCivicResearchValue;
		}
	}
	*/


	return iFlavorValue;
}

int CvPlayerAI::AI_techValueHappy(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_HAPPY;
	int iLoop;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue = getFlavorValue(iFlavorType) + 20;
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		if(pLoopCity->happyLevel()<pLoopCity->unhappyLevel()+2)
		{
			iFlavorValue+=1000;
		}
	}

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueNaval(TechTypes iTech) const
{
	int iFlavorType = FLAVOR_NAVAL;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType) <= 0)
		return 0;

	int iFlavorValue = getFlavorValue(iFlavorType);
	iFlavorValue *= GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	int iTempValue=0;
/**
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop)) {
		if(pLoopCity->isCoastal(5))  {
			iTempValue += 30000;
		}
	}
**/
	if(getNumCities() > 0)
		if(AI_getExpansionPlot() == NULL || AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID()) == 0)
			iTempValue += 30000;

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueMetal(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_METAL;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueGold(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_GOLD;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType) + 20;
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueReligion(TechTypes iTech) const
{
	int iValue = 0;
	int iFlavorType = FLAVOR_RELIGION;
	CvTechInfo& kTech = GC.getTechInfo(iTech);

	if(kTech.isAllowReligion() && !isAgnostic())
		iValue += 1000;

	if(kTech.getFlavorValue(iFlavorType) > 0)
		iValue += getFlavorValue(iFlavorType) * kTech.getFlavorValue(iFlavorType);

	return iValue;
}

int CvPlayerAI::AI_techValueMilitaryUnit(TechTypes iTech) const
{
	int iValue=0;
	bool bTier4Needed = false;

	if(AI_isMilitaryTierNeeded(4))
		bTier4Needed = true;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		if(GC.getGameINLINE().isUnitClassMaxedOut((UnitClassTypes)iUnitIndex))
		{
			continue;
		}

		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		if(eUnit!=NO_UNIT)
		{
			CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);
			if(kUnit.getPrereqAndTech() == iTech)
			{
				//Civ Unique Unit
				if(kUnit.getPrereqCiv()==NO_CIVILIZATION || kUnit.getPrereqCiv()==getCivilizationType())
				{
					if(kUnit.getCorporationSupportNeeded()!=NO_CORPORATION)
					{
						continue;
					}
					if(kUnit.getStateReligion()!=NO_RELIGION)
					{
						if(getFavoriteReligion()!=getStateReligion()
							||(kUnit.getStateReligion()!=getFavoriteReligion()))
						{
							continue;
						}
					}

					if(kUnit.getDomainType()==DOMAIN_AIR)
					{
						continue;
					}

					if(kUnit.getManaUpkeep()!=0 || kUnit.getFaithUpkeep()!=0)
					{
						continue;
					}

					if(kUnit.getPrereqAndBonus()!=NO_BONUS && getNumAvailableBonuses((BonusTypes)kUnit.getPrereqAndBonus())==0)
					{
						continue;
					}

					iValue+=500;

					if(bTier4Needed && kUnit.getTier() == 4) {
						if(getResearchTurnsLeft(iTech, true) < 100) {
							iValue += 10000;
						}
					}
				}
			}
		}
	}

	return iValue;
}

int CvPlayerAI::AI_techValueUniqueUnit(TechTypes iTech) const
{
	int iValue=0;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		if(eUnit!=NO_UNIT)
		{
			if(GC.getUnitInfo(eUnit).getPrereqAndTech()==iTech)
			{
				if(GC.getUnitInfo(eUnit).getPrereqCiv()==NO_CIVILIZATION || GC.getUnitInfo(eUnit).getPrereqCiv()==getCivilizationType())
				{
					if(eUnit!=GC.getUnitClassInfo((UnitClassTypes)GC.getUnitInfo(eUnit).getUnitClassType()).getDefaultUnitIndex())
					{
						if(GC.getUnitInfo(eUnit).getPrereqAndBonus()!=NO_BONUS && getNumAvailableBonuses((BonusTypes)GC.getUnitInfo(eUnit).getPrereqAndBonus())==0)
						{
							continue;
						}

						iValue+=2000;
					}
				}
			}
		}
	}

	return iValue;
}

int CvPlayerAI::AI_techValueResource(TechTypes iTech) const
{
	int iValue=0;

	for(int iI=0;iI<GC.getNumBonusInfos();iI++)
	{
		CvBonusInfo &kBonus=GC.getBonusInfo((BonusTypes)iI);
		if(kBonus.getTechCityTrade()==iTech && getNumAvailableBonuses((BonusTypes)iI)==0)
		{
			if(kBonus.getTechReveal()==NO_TECH || GET_TEAM(getTeam()).isHasTech((TechTypes)kBonus.getTechReveal()))
			{
				CvMap &kMap=GC.getMapINLINE();
				for(int iJ=0;iJ<kMap.numPlots();iJ++)
				{
					if(kMap.plotByIndex(iJ)->getBonusType(NO_TEAM)==iI)
					{
						if(kMap.plotByIndex(iJ)->getOwnerINLINE()==getID())
						{
							iValue+=2000;							
							break;
						}
					}
				}
			}
		}
	}

	return iValue;
}


int CvPlayerAI::AI_techValueBuilds(TechTypes iTech) const
{
	int iValue=0;

	for(int iI=0;iI<GC.getNumBuildInfos();iI++)
	{
		if(GC.getBuildInfo((BuildTypes)iI).getTechPrereq()==iTech)
		{
			iValue+=100;
		}
	}
	return iValue;
}

TechTypes CvPlayerAI::AI_bestTechNew(int iMaxPathLength, bool bIgnoreCost, bool bAsync, TechTypes eIgnoreTech, AdvisorTypes eIgnoreAdvisor) const
{
	int iBestTech=0;
	int iValue, iPathLength;
	int aiBestTechs[MAX_TECH_VARIATION];
	int aiBestTechValues[MAX_TECH_VARIATION];

	for(int iI = 0; iI < MAX_TECH_VARIATION; iI++)
	{
		aiBestTechValues[iI] = 0;
	}

	TechTypes eEarlyTech = AI_needEarlyTech();
	if(eEarlyTech!=NO_TECH)
	{
		return eEarlyTech;
	}

	TechTypes eMilitaryTech = AI_needMilitaryTech();
	if(eMilitaryTech!=NO_TECH)
	{
		return eMilitaryTech;
	}

	for (iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if ((eIgnoreTech == NO_TECH) || (iI != eIgnoreTech))
		{
			if ((eIgnoreAdvisor == NO_ADVISOR) || (GC.getTechInfo((TechTypes)iI).getAdvisorType() != eIgnoreAdvisor))
			{
				if (canEverResearch((TechTypes)iI))
				{
					if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)iI)))
					{
						iPathLength = findPathLength(((TechTypes)iI), false);
						if (iPathLength <= iMaxPathLength)
						{
							iValue=AI_techValueNew((TechTypes)iI,bIgnoreCost);

							if(iValue>aiBestTechValues[MAX_TECH_VARIATION-1])
							{
								for(int iJ=0;iJ<MAX_TECH_VARIATION;iJ++)
								{
									if(iValue>aiBestTechValues[iJ])
									{
										//aiTechValues[iI] must become the new aiBestTechValues[iJ]
										if(MAX_TECH_VARIATION>1)
										{
											for(int iK=MAX_TECH_VARIATION-1;iK>iJ;iK--)
											{
												aiBestTechValues[iK]=aiBestTechValues[iK-1];
												aiBestTechs[iK]=aiBestTechs[iK-1];
											}
										}

										aiBestTechValues[iJ]=iValue;
										aiBestTechs[iJ]=iI;
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	int iBestTechAmount=0;
	for(int iJ=0;iJ<MAX_TECH_VARIATION;iJ++)
	{
		iBestTechAmount+=std::max(0,aiBestTechValues[iJ]);
	}

	int iBestTechPick;
	

	if (bAsync)
	{
		iBestTechPick=GC.getASyncRand().get(iBestTechAmount,"CvAIChooseTech: random amoung BestTechs ASYNC");
	}
	else
	{
		iBestTechPick=GC.getGameINLINE().getSorenRandNum(iBestTechAmount,"CvAIChooseTech: random amoung BestTechs");
	}

	iBestTechAmount=0;
	for(int iI=0;iI<MAX_TECH_VARIATION;iI++)
	{
		iBestTechAmount+=aiBestTechValues[iI];
		if(iBestTechAmount>=iBestTechPick)
		{
			iBestTech=iI;
			break;
		}
	}		
	

	if(aiBestTechValues[iBestTech]>0)
	{
		TechTypes iChoosenTech = (TechTypes)aiBestTechs[iBestTech];
		if(iChoosenTech>NO_TECH && iChoosenTech<=GC.getNumTechInfos() && iBestTechAmount!=0)
		{
			//DEBUG
			if(isOOSLogging()){
				TCHAR szOut[1024];
				sprintf(szOut,"");
				for(int iI=0;iI<MAX_TECH_VARIATION;iI++)
				{
					sprintf(szOut,"%s,Tech%d:%S,Value:%d,AIValue:%d"
						,szOut
						,iI
						,(aiBestTechs[iI]>NO_TECH && aiBestTechs[iI]<=GC.getNumTechInfos() && iBestTechAmount!=0)?GC.getTechInfo((TechTypes)aiBestTechs[iI]).getDescription():L""
						,(aiBestTechs[iI]>NO_TECH && aiBestTechs[iI]<=GC.getNumTechInfos() && iBestTechAmount!=0)?aiBestTechValues[iI]:-1
						,(aiBestTechs[iI]>NO_TECH && aiBestTechs[iI]<=GC.getNumTechInfos() && iBestTechAmount!=0)?(aiBestTechValues[iI]*100)/iBestTechAmount:-1
					);
				}
				oosLog("AIResearch"
					,"Turn: %d,PlayerID: %d,Player: %S,Tech: %S(%d)%s\n"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getID()
					,GC.getCivilizationInfo(getCivilizationType()).getDescription()
					,GC.getTechInfo(iChoosenTech).getDescription()
					,getResearchTurnsLeft(iChoosenTech,true)*100 / std::max(1,GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getResearchPercent())
					,szOut
				);
			}
			//DEBUG
			return iChoosenTech;
		}
		/*
		//DEBUG
		if(isOOSLogging()){
			TCHAR szOut[1024];
			sprintf(szOut,"");
			for(int iI=0;iI<MAX_TECH_VARIATION;iI++)
			{
				sprintf(szOut,"%s,Tech%d:%S,Value:%d,AIValue:%d"
					,szOut
					,iI
					,(aiBestTechs[iI]>NO_TECH && aiBestTechs[iI]<=GC.getNumTechInfos() && iBestTechAmount!=0)?GC.getTechInfo((TechTypes)aiBestTechs[iI]).getDescription():L""
					,(aiBestTechs[iI]>NO_TECH && aiBestTechs[iI]<=GC.getNumTechInfos() && iBestTechAmount!=0)?aiBestTechValues[iI]:-1
					,(aiBestTechs[iI]>NO_TECH && aiBestTechs[iI]<=GC.getNumTechInfos() && iBestTechAmount!=0)?(aiBestTechValues[iI]*100)/iBestTechAmount:-1
				);
			}

			TechTypes iChoosenTech = (aiBestTechs[iBestTech]>NO_TECH && aiBestTechs[iBestTech]<=GC.getNumTechInfos() && iBestTechAmount!=0)?(TechTypes)aiBestTechValues[iBestTech]:NO_TECH;
			oosLog("AIResearch"
				,"Turn: %d,PlayerID: %d,Player: %S,Tech: %S(%d)%s\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getID()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,iChoosenTech!=NO_TECH?GC.getTechInfo(iChoosenTech).getDescription():L""
				,iChoosenTech!=NO_TECH?(getResearchTurnsLeft(iChoosenTech,true)*100)/std::max(1,GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getResearchPercent()):-1
				,szOut
			);
		}
		//DEBUG
		return (TechTypes)aiBestTechs[iBestTech];
		*/
	}

	return NO_TECH;
}

int CvPlayerAI::AI_getSpellResearchValue(TechTypes eSpellResearch) 
{
	if(eSpellResearch == NO_TECH)
		return 0;

	CvTechInfo &kResearch=GC.getTechInfo(eSpellResearch);

	/** new flavor formula **/
	int ePalaceClass = GC.getBUILDINGCLASS_PALACE();
	BuildingTypes ePalace = NO_BUILDING;
	BonusTypes eBonus = NO_BONUS;

	if(ePalaceClass != NO_BUILDINGCLASS)
		ePalace = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(ePalaceClass);
	if(ePalace != NO_BUILDING)
		eBonus = (BonusTypes)GC.getBuildingInfo(ePalace).getFreeBonus();
	
	if(kResearch.getPrereqBonus() == eBonus)
		return 10000;
	else
		return 100;


	int iValue=100;
	int iStrategy=AI_getMagicStrategy();

	iValue+=GC.getGame().getSorenRandNum(100,"AI Spellresearch");
	if(kResearch.getPrereqBonus()!=NO_BONUS)
	{
		iValue+=15*getNumAvailableBonuses((BonusTypes)kResearch.getPrereqBonus());
	}

	return iValue;
}

TechTypes CvPlayerAI::AI_bestSpellResearch()
{
	TechTypes eBestTech=NO_TECH;
	int iValue;
	int iBestValue=0;

	for(int iI = 0; iI < GC.getNumTechInfos(); iI++)
	{
		if(GC.getTechInfo((TechTypes)iI).isSpellResearch())
		{
			if(canSpellResearchNext((TechTypes)iI))
			{
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("Player::doTurn::AI_doTurnPre::AI_getSpellResearchValue");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

				iValue = AI_getSpellResearchValue((TechTypes)iI);
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("Player::doTurn::AI_doTurnPre::AI_getSpellResearchValue");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestTech=(TechTypes)iI;
				}
			}
		}
	}

	return eBestTech;
}
