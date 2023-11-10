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

	TechTypes eExploration=(TechTypes)GC.getInfoTypeForString("TECH_EXPLORATION");
	TechTypes eBarter=(TechTypes)GC.getInfoTypeForString("TECH_BARTER");
	TechTypes eChants = (TechTypes)GC.getInfoTypeForString("TECH_ANCIENT_CHANTS");

	if(eExploration!=NO_TECH && !GET_TEAM(getTeam()).isHasTech(eExploration))
	{
		return eExploration;
	}

	if(eBarter!=NO_TECH && !GET_TEAM(getTeam()).isHasTech(eBarter))
	{
		return eBarter;
	}

	if(eChants!=NO_TECH && !GET_TEAM(getTeam()).isHasTech(eChants))
	{
		return eChants;
	}

	

	//FirstPass: Access to Tier2 Military Units
	bool bTier2MilitaryNeeded = AI_isMilitaryTierNeeded(2);

	if(bTier2MilitaryNeeded)
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
	int iFlavorType=FLAVOR_MILITARY;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);

	// for now, hardcode it to 15 (every civ/leader needs this, similar to gold/research
	iFlavorValue = 15;

	iFlavorValue *= GC.getTechInfo(iTech).getFlavorValue(iFlavorType);
	return iFlavorValue;
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

	int iFlavorValue=getFlavorValue(iFlavorType);
	iFlavorValue*=GC.getTechInfo(iTech).getFlavorValue(iFlavorType);

	return iFlavorValue;
}

int CvPlayerAI::AI_techValueHappy(TechTypes iTech) const
{
	int iFlavorType=FLAVOR_HAPPY;
	int iLoop;
	if(GC.getTechInfo(iTech).getFlavorValue(iFlavorType)<=0)
		return 0;

	int iFlavorValue=getFlavorValue(iFlavorType);
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

	int iFlavorValue=getFlavorValue(iFlavorType);
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

	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "AIResearchPick %d, %S\n",getID(),GC.getCivilizationInfo(getCivilizationType()).getDescription());
    gDLL->logMsg("AIResearch.log",szOut, false, false);
	for(int iI=0;iI<MAX_TECH_VARIATION;iI++)
	{
		if(aiBestTechs[iI]>NO_TECH && aiBestTechs[iI]<=GC.getNumTechInfos() && iBestTechAmount!=0)
		{
			sprintf(szOut, "%S -- %d\n",GC.getTechInfo((TechTypes)aiBestTechs[iI]).getDescription(),(aiBestTechValues[iI]*100)/iBestTechAmount);
			gDLL->logMsg("AIResearch.log",szOut, false, false);
		}
	}
	/** DEBUG **/

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
		return (TechTypes)aiBestTechs[iBestTech];
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