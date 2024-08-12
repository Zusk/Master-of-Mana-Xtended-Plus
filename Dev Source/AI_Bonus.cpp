#include "CvGameCoreDLL.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvCityAI.h"
#include "CvGameAI.h"
#include "CvPlot.h"
#include "CvArea.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CyCity.h"
#include "CyArgsList.h"
#include "CvInfos.h"
#include "FProfiler.h"

#include "CvDLLPythonIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"


//returns Bonus value in Gold/turn times 100
int CvPlayerAI::AI_bonusVal_new(BonusTypes eBonus, int iChange, bool bDebugValue, CvWStringBuffer &szBuffer) const
{
	/*
	oosLog("AITrade","*****************************************************");
	for(int iI=0;iI<GC.getNumBuildingInfos();iI++)
	{

		for(int iJ=0;iJ<GC.getNumBonusInfos();iJ++)
		{
			if(GC.getBuildingInfo((BuildingTypes)iI).isBonusLinked((BonusTypes)iJ))
			{
				oosLog("AITrade","AI_bonusYieldCommerceValue: %S: %S",GC.getBuildingInfo((BuildingTypes)iI).getDescription(),GC.getBonusInfo((BonusTypes)iJ).getDescription());
			}
			if(GC.getBuildingInfo((BuildingTypes)iI).getBonusProductionModifier((BonusTypes)iJ) > 0)
			{
				oosLog("AITrade","getBonusProductionModifier: %S: %S",GC.getBuildingInfo((BuildingTypes)iI).getDescription(),GC.getBonusInfo((BonusTypes)iJ).getDescription());
			}
			if(GC.getBuildingInfo((BuildingTypes)iI).isBonusConsumed((BonusTypes)iJ))
			{
				oosLog("AITrade","isBonusConsumed: %S: %S",GC.getBuildingInfo((BuildingTypes)iI).getDescription(),GC.getBonusInfo((BonusTypes)iJ).getDescription());
			}
			for(int iZ=0;iZ<GC.getNUM_BUILDING_PREREQ_OR_BONUSES();iZ++)
			{
				if(GC.getBuildingInfo((BuildingTypes)iI).getPrereqOrBonuses(iZ) == iJ)
				{
					oosLog("AITrade","Building getPrereqOrBonuses: %S: %S",GC.getBuildingInfo((BuildingTypes)iI).getDescription(),GC.getBonusInfo((BonusTypes)iJ).getDescription());
				}
			}
		}
	}
	for(int iI=0;iI<GC.getNumUnitInfos();iI++)
	{
		for(int iJ=0;iJ<GC.getNumBonusInfos();iJ++)
		{
			for(int iZ=0;iZ<GC.getNUM_UNIT_PREREQ_OR_BONUSES();iZ++)
			{
				if(GC.getUnitInfo((UnitTypes)iI).getPrereqOrBonuses(iZ) == iJ)
				{
					oosLog("AITrade","Unit getPrereqOrBonuses: %S: %S",GC.getUnitInfo((UnitTypes)iI).getDescription(),GC.getBonusInfo((BonusTypes)iJ).getDescription());
				}
			}
			if(GC.getUnitInfo((UnitTypes)iI).getBonusAffinity((BonusTypes)iJ) != 0)
			{
				oosLog("AITrade","getBonusAffinity: %S: %S",GC.getUnitInfo((UnitTypes)iI).getDescription(),GC.getBonusInfo((BonusTypes)iJ).getDescription());
			}
		}
	}
	for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
	{
		if (GC.getPromotionInfo((PromotionTypes)iI).isGear() || GC.getPromotionInfo((PromotionTypes)iI).getEquipmentCategory() != -1)
		{
			if (GC.getPromotionInfo((PromotionTypes)iI).getBonusPrereq() != NO_BONUS)
			{
				for(int iJ=0;iJ<GC.getNumBonusInfos();iJ++)
				{
					if (GC.getPromotionInfo((PromotionTypes)iI).getBonusPrereq() == (BonusTypes)iJ)
					{
						oosLog("AITrade","Equipment getPrereqOrBonuses: %S: %S",GC.getPromotionInfo((PromotionTypes)iI).getDescription(),GC.getBonusInfo((BonusTypes)iJ).getDescription());
					}
				}
			}
		}
	}
	//***********************************************
*/
	if (iChange == 0)
	{
		return 0;
	}
	int iValue=0;
	int iBonusCount = getNumAvailableBonuses(eBonus);
	bool bAppend = true; //TODO. how to check if szBuffer it is empty? or null? gDLL->isDiplo?

	int iValueHealth=AI_bonusHealthValue(eBonus,iChange,iBonusCount);
	if(iValueHealth != 0)
	{
		/*iValue+=50*iValueHealth;
		if(!szBuffer.isEmpty()){
			szBuffer.append(NEWLINE);
		}
		szBuffer.append(CvWString::format(L"+%d%c", iValueHealth, gDLL->getSymbolID(HEALTHY_CHAR)));
		*/
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" Health:%d", iValueHealth));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],Health:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueHealth
			);
		}
	}
	iValue+=iValueHealth;

	int iValueHappiness=AI_bonusHappinessValue(eBonus,iChange,iBonusCount);
	if(iValueHappiness != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" Happiness:%d", iValueHappiness));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],Happiness:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueHappiness
			);
		}
	}
	iValue+=iValueHappiness;

	int iValueCommerce=AI_bonusYieldCommerceValue(eBonus,iChange,iBonusCount);
	if(iValueCommerce != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" Commerce:%d", iValueCommerce));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],Commerce:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueCommerce
			);
		}
	}
	iValue+=iValueCommerce;

	int iValueBuildFaster=AI_bonusBuildFaster(eBonus, iChange, iBonusCount);
	if(iValueBuildFaster != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" BuildFaster:%d", iValueBuildFaster));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],BuildFaster:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueBuildFaster
			);
		}
	}
	iValue+=iValueBuildFaster;

	int iValueConsumeResource=AI_bonusBuildingConsumeResource(eBonus, iChange, iBonusCount);
	if(iValueConsumeResource != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" ConsumeResource:%d", iValueConsumeResource));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],ConsumeResource:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueConsumeResource
			);
		}
	}
	iValue+=iValueConsumeResource;

	int iValueAffinityUnit=AI_bonusGivesAffinityUnit(eBonus, iChange, iBonusCount);
	if(iValueAffinityUnit != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" Affinity:%d", iValueAffinityUnit));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],Affinity:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueAffinityUnit
			);
		}
	}
	iValue+=iValueAffinityUnit;

	int iValueAllowsEquipment=AI_bonusAllowsEquipment(eBonus, iChange, iBonusCount);
	if(iValueAllowsEquipment != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" AllowsEquipment:%d", iValueAllowsEquipment));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],AllowsEquipment:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueAllowsEquipment
			);
		}
	}
	iValue+=iValueAllowsEquipment;

	int iValueAllowsUnit=AI_bonusAllowsUnit(eBonus, iChange, iBonusCount);
	if(iValueAllowsUnit != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" AllowsUnit:%d", iValueAllowsUnit));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],AllowsUnit:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueAllowsUnit
			);
		}
	}
	iValue+=iValueAllowsUnit;

	int iValueAllowsBuilding=AI_bonusAllowsBuilding(eBonus, iChange, iBonusCount);
	if(iValueAllowsBuilding != 0)
	{
		if(bAppend)
		{
			szBuffer.append(NEWLINE);
			szBuffer.append(CvWString::format(L" AllowsBuilding:%d", iValueAllowsBuilding));
		}
		if(bDebugValue && isOOSLogging())
		{
			oosLog("AITrade"
				,"Turn:%d,Player:%S,BonusTypes:%S[%d],AllowsBuilding:%d\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,GC.getCivilizationInfo(getCivilizationType()).getDescription()
				,GC.getBonusInfo(eBonus).getDescription()
				,iChange
				,iValueAllowsBuilding
			);
		}
	}
	iValue+=iValueAllowsBuilding;

//	iValue+=AI_bonusAlignmentValue(eBonus);
//	iValue+=AI_bonusMiscValue(eBonus);
	if(bAppend)
	{
		szBuffer.append(NEWLINE);
		szBuffer.append(CvWString::format(L" Total:%d", iValue));
	}
	return iValue;
}

int CvPlayerAI::AI_bonusHealthValue(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

	//we had zero before or we have zero after
	if((iChange>0 && iBonusCount==0) || (iBonusCount+iChange<=0 && iBonusCount>0))
	{
		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			int iActualHealthChangeInCity=pLoopCity->getBonusHealth(eBonus);
			if(iActualHealthChangeInCity > 0)
			{
				int iCurrentHealth = pLoopCity->goodHealth()-pLoopCity->badHealth();
				int iCurrentHappiness = pLoopCity->happyLevel() - pLoopCity->unhappyLevel();
				if(iCurrentHappiness > 0)
				{
					//If city does not have happiness to grow, value is 0
					if(iCurrentHealth <= 0 || (iChange < 0 && iCurrentHealth <= 1))
					{
						//This healt change can allow/deny the city to grow faster
						iValue+=100*iActualHealthChangeInCity*iChange;
					}
					else
					{
						//City is already growing
						iValue+=50*iActualHealthChangeInCity*iChange;
					}
				}
				else if((iCurrentHappiness <= 0 && iChange < 0) && (iChange < 0 && iCurrentHealth <= 1))
				{
					//This healt change can deny the city to grow faster
					iValue+=100*iActualHealthChangeInCity*iChange;
				}
			}
			/*
			iActualHealthChangeInCity=std::min(iActualHealthChangeInCity,pLoopCity->goodHealth()+3-pLoopCity->badHealth());
			iActualHealthChangeInCity=std::max(0,iActualHealthChangeInCity);
			//iValue+=150*iActualHealthChangeInCity;
			iValue+=50*iActualHealthChangeInCity;
			//iValue+=iActualHealthChangeInCity;
			*/
		}
	}

	return iValue;
}

int CvPlayerAI::AI_bonusHappinessValue(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

	//we had zero before or we have zero after
	if((iChange>0 && iBonusCount==0) || (iBonusCount+iChange<=0 && iBonusCount>0))
	{
		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			int iActualHappinessChangeInCity=pLoopCity->getBonusHappiness(eBonus);
			if(iActualHappinessChangeInCity > 0)
			{
				int iCurrentHappiness = pLoopCity->happyLevel() - pLoopCity->unhappyLevel();
				if (iCurrentHappiness <= 0 || (iChange < 0 && iCurrentHappiness <= 0))
				{
					//This happiness change will allow/prevent city to grow or to use one worker more!
					iValue+=150*iActualHappinessChangeInCity*iChange;
				}
				else if((pLoopCity->growthThreshold() - pLoopCity->getFood()) > 0 || GET_PLAYER(pLoopCity->getOwner()).isIgnoreFood())
				{
					//If city is growing or civ does not use food
					iValue+=75*iActualHappinessChangeInCity*iChange;
				}
			}
			/*
			iActualHappinessChangeInCity=std::min(iActualHappinessChangeInCity,pLoopCity->happyLevel()+3-pLoopCity->unhappyLevel());
			iActualHappinessChangeInCity=std::max(0,iActualHappinessChangeInCity);
			//iValue+=150*iActualHappinessChangeInCity;
			iValue+=75*iActualHappinessChangeInCity;
			*/
		}
	}

	return iValue;
}

int CvPlayerAI::AI_bonusYieldCommerceValue(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

	bool bFirst=false;
	//we had zero before or we have zero after
	if((iChange>0 && iBonusCount==0) || (iBonusCount+iChange<=0 && iBonusCount>0))
	{
		bFirst=true;
	}
//TODO: this seems to not care for exotic market...
	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		for(int iI=0;iI<GC.getNumBuildingInfos();iI++)
		{
			if(pLoopCity->getNumBuilding((BuildingTypes)iI)>0)
			{
				if(GC.getBuildingInfo((BuildingTypes)iI).getBonusYieldType()!=NO_YIELD && GC.getBuildingInfo((BuildingTypes)iI).isBonusLinked(eBonus))
				{
					//iValue+=GC.getBuildingInfo((BuildingTypes)iI).getPerBonusValue()* iChange;
					iValue+=GC.getBuildingInfo((BuildingTypes)iI).getPerBonusValue() * (iChange/2);

					if(bFirst)
					{
						//iValue+=(GC.getBuildingInfo((BuildingTypes)iI).getInitialValue()* iChange *3)/2;
						iValue+=GC.getBuildingInfo((BuildingTypes)iI).getInitialValue() * iChange;
					}
					//SpyFanatic: some YIELD are more equals than others
					switch(GC.getBuildingInfo((BuildingTypes)iI).getBonusYieldType())
					{
						case YIELD_FOOD:
						case YIELD_PRODUCTION:
						case YIELD_COMMERCE:
							break;
						case YIELD_LUMBER:
						case YIELD_LEATHER:
						case YIELD_METAL:
						case YIELD_HERB:
						case YIELD_STONE:
							iValue/=2; //AI should not use a lot of those for improvement/buildings/troops...
							break;
						default:
							break;
					}
				}
				else if(GC.getBuildingInfo((BuildingTypes)iI).getBonusCommerceType()!=NO_COMMERCE && GC.getBuildingInfo((BuildingTypes)iI).isBonusLinked(eBonus))
				{
					//iValue+=GC.getBuildingInfo((BuildingTypes)iI).getPerBonusValue()* iChange;
					iValue+=GC.getBuildingInfo((BuildingTypes)iI).getPerBonusValue() * (iChange/2);

					if(bFirst)
					{
						iValue+=GC.getBuildingInfo((BuildingTypes)iI).getInitialValue() * iChange;
					}
					//SpyFanatic: some COMMERCE are more equals than others
					switch(GC.getBuildingInfo((BuildingTypes)iI).getBonusCommerceType())
					{
						case COMMERCE_GOLD:
							break;
						case COMMERCE_RESEARCH:
							break;
						case COMMERCE_CULTURE:
							break;
						case COMMERCE_ESPIONAGE:
							break;
						case COMMERCE_FAITH:
							break;
						case COMMERCE_MANA:
							break;
						case COMMERCE_ARCANE:
							break;
						default:
							break;
					}
				}

			}
		}
	}

	return iValue;
}

int CvPlayerAI::AI_bonusBuildFaster(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

/*
Form of the Titan	Copper
Summer Palace	Marble
Winter Palace	Marble
Summer Palace	Stone
Winter Palace	Stone
*/
	//we had zero before or we have zero after
	if((iChange>0 && iBonusCount==0) || (iBonusCount+iChange<=0 && iBonusCount>0))
	{
		//int iCityCanConstructWonder = 0;
		//int iCityCanConstructNational = 0;
		int iModifier = iChange > 0?1:-1;

		for(int iI=0;iI<GC.getNumBuildingInfos();iI++)
		{
			BuildingTypes eBuilding = (BuildingTypes)iI;
			if(GC.getBuildingInfo(eBuilding).getBonusProductionModifier(eBonus) > 0)
			{
				for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
				{
					if(pLoopCity->getNumBuilding(eBuilding) <= 0 && pLoopCity->canConstruct(eBuilding))
					{
						if(isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
						{
							iValue += 500*iModifier;
							//iCityCanConstructWonder++;
							break; //no need to iterate across other cities as its wonder
						}
						else if(isNationalWonderClass((BuildingClassTypes)(GC.getBuildingInfo(eBuilding).getBuildingClassType())))
						{
							iValue += 250*iModifier;
							//iCityCanConstructNational++;
							break; //no need to iterate across other cities as its wonder
						}
						else
						{
							iValue += 100*iModifier; //This should never be the case unless new buildings are added
						}
					}
				}
			}
		}/*
		if(iCityCanConstructWonder > 0)
		{
			iValue += 500*iModifier;
		}
		if(iCityCanConstructNational > 0)
		{
			iValue += 250*iModifier;
		}*/
	}

	return iValue;
}

int CvPlayerAI::AI_bonusBuildingConsumeResource(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

/*
Fur Trader	Fur
Gemsmith	Gems
Alabaster Garden	Ivory
Jeweler	Jewelry
eroic Epic	Marble
National Epic	Marble
*/
	for(int iI=0;iI<GC.getNumBuildingInfos();iI++)
	{
		//If we have bonusconsumed, then there is no prereq bonus in the current buildings
		if(GC.getBuildingInfo((BuildingTypes)iI).isBonusConsumed(eBonus))
		{
			int iCountBuildingCanConstruct = 0;
			int iCountBuildingExisting = 0;
			for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
			{
				if(pLoopCity->canConstruct((BuildingTypes)iI,false,false,false,true))
				{
					iCountBuildingCanConstruct++;
				}
				else if(pLoopCity->getNumBuilding((BuildingTypes)iI))
				{
					iCountBuildingExisting++;
				}
				if(iChange > 0 && iCountBuildingCanConstruct >= iChange)
				{
					break; //No need to go on with for loop once as 1 resource allows only 1 building
				}
			}
			if(iCountBuildingCanConstruct > 0)
			{
				int iModifier =  iChange > 0?1:-1;
				iValue=100*std::min(iCountBuildingCanConstruct,std::abs(iChange))*iModifier;
			}
		}
	}

	return iValue;
}

int CvPlayerAI::AI_bonusGivesAffinityUnit(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

/*
Gurid	Banana
Larincsidhe	Eternal Fruits
Bone Horde	Gunpowder
Bucaneer	Gunpowder
Cannon	Gunpowder
Dwarven Cannon	Gunpowder
Cannon	Iron
Dwarven Cannon	Iron
Cannon	Mithril
Dwarven Cannon	Mithril
Dwarven Shadow	Razorweed
Shadow	Razorweed
Catapult	Stone
Trebuchet	Stone
Catapult	Timber
Trebuchet	Timber
*/
	for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
	{
		int iAffinityBonus = pLoopUnit->getBonusAffinity(eBonus);
		if(iAffinityBonus != 0)
		{
			iValue+=iAffinityBonus * iChange * 100;
		}
	}
	if(iValue != 0)
	{
		if(iChange > 0)
		{
			iValue=std::max(100,iValue / 5);
		}
		else
		{
			iValue=std::min(-100,iValue / 5);
		}
	}
	//100 of Value for each +5 affinity it gives, minimum 100
	return iValue;
}

int CvPlayerAI::AI_bonusAllowsEquipment(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;
/*
Mithril Chainmail: Mithril
Mithril Platemail: Mithril
Fast Mount: Horse
Battle Mount: Horse
Hippus Heavy Warhorses: Horse
Hippus Medium Warhorses: Horse
Hippus Light Warhorses: Horse
Hippus Ponnies: Horse
Silver Shortsword: Silver
Mithril Sword: Mithril
Mithril Claymore: Mithril
Mithril Fullblade: Mithril
Silver Bow: Silver
Dwarven Coppercraft: Copper
Rifle BR0NZ: Copper
Rifle 01-R0N: Iron
Rifle GG-L0L: Mithril
Iron Ammo: Iron
Mithril Ammo: Mithril
*/
	//we had zero before or we have zero after
	if((iChange>0 && iBonusCount==0) || (iBonusCount+iChange<=0 && iBonusCount>0))
	{
		int iModifier = iChange > 0?1:-1;
		int iCountEquipment = 0;
		for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
		{
			PromotionTypes ePromotion = (PromotionTypes)iI;
			CvPromotionInfo& kPromotion = GC.getPromotionInfo(ePromotion);
			if (kPromotion.isGear() || kPromotion.getEquipmentCategory() != -1)
			{
				if (kPromotion.getBonusPrereq() == eBonus)
				{
					//put this here maybe should reduce the amount of looping
					if(kPromotion.getTechPrereq()==NO_TECH || GET_TEAM(getTeam()).isHasTech((TechTypes)kPromotion.getTechPrereq()))
					{
						bool bEquipmentFound = false;
						for (CvUnit* pLoopUnit = firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = nextUnit(&iLoop))
						{
							//No way of knowing if canEquipUnit return false just becouse bonus is missing, but with bVisible and bTreasure some checks are skipped
							if(canEquipUnit(pLoopUnit,ePromotion,false,false,true))
							{
								bEquipmentFound=true;
								break;
							}
						}
						if(bEquipmentFound)
						{
							iCountEquipment++;
							break;
						}
					}
				}
			}
		}
		iValue+=iCountEquipment*50*iModifier;
	}

	return iValue;
}

int CvPlayerAI::AI_bonusAllowsUnit(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;

	//we had zero before or we have zero after
	if((iChange>0 && iBonusCount==0) || (iBonusCount+iChange<=0 && iBonusCount>0))
	{
		int iModifier = iChange > 0?1:-1;
		int iAllowedUnits = 0;
		for(int iI=0;iI<GC.getNumUnitInfos();iI++)
		{
			UnitTypes eUnit = (UnitTypes)iI;
			CvUnitInfo& kUnit = GC.getUnitInfo(eUnit);

			bool bNeedBonus = false;
			for(int iZ=0;iZ<GC.getNUM_UNIT_PREREQ_OR_BONUSES();iZ++)
			{
				if(kUnit.getPrereqOrBonuses(iZ) == eBonus)
				{
					bNeedBonus = true;
				}
				else if (kUnit.getPrereqOrBonuses(iZ) != NO_BONUS && hasBonus((BonusTypes)kUnit.getPrereqOrBonuses(iZ)))
				{
					//We have another bonus for producing it, no value
					bNeedBonus = false;
					break;
				}
			}

			if(bNeedBonus)
			{
				int iLoop;
				bool bFound = false;
				for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
				{
					if(pLoopCity->canTrain(eUnit,false,false,false,false,true))
					{
						bFound = true;
						break;
					}
				}
				if(bFound)
				{
					iAllowedUnits+=std::min(1,kUnit.getTier());
					//iAllowedUnits++;
					break;
				}
			}
		}
		iValue+=iAllowedUnits*100*iModifier;
	}

	return iValue;
}

int CvPlayerAI::AI_bonusAllowsBuilding(BonusTypes eBonus, int iChange, int iBonusCount) const
{

/*
 Apothecary	Banana
 Apothecary	Sugar
 Apothecary	Wine
 Dry Salting House	Salt
 Farmers Market	Corn
 Farmers Market	Rice
 Farmers Market	Wheat
 Hall of the Covenant	Dye
 Huntsman's Cabin	Bison
 Huntsman's Cabin	Deer
 Huntsman's Cabin	Fur
 Mint	Amber
 Mint	Gold
 Mint	Silver
 Pearl Farm	Clam
 Smokehouse	Bison
 Smokehouse	Cow
 Smokehouse	Pig
 Smokehouse	Sheep
 Weavers Hut	Cotton
 Weavers Hut	Sheep

*/
	int iValue=0;
	//we had zero before or we have zero after
	if((iChange>0 && iBonusCount==0) || (iBonusCount+iChange<=0 && iBonusCount>0))
	{
		int iModifier = iChange > 0 ? 1 : -1;
		for(int iI=0;iI<GC.getNumBuildingInfos();iI++)
		{
			BuildingTypes eBuilding = (BuildingTypes)iI;
			CvBuildingInfo& kBuilding = GC.getBuildingInfo(eBuilding);

			bool bNeedBonus = (kBuilding.getPrereqAndBonus() == eBonus);
			if (!bNeedBonus)
			{
				for (int iJ = 0; iJ < GC.getNUM_BUILDING_PREREQ_OR_BONUSES(); iJ++)
				{
					if ((BonusTypes)kBuilding.getPrereqOrBonuses(iJ) != NO_BONUS && hasBonus((BonusTypes)kBuilding.getPrereqOrBonuses(iJ)))
					{
						bNeedBonus = false;
						break;
					}
					if (kBuilding.getPrereqOrBonuses(iJ) == eBonus)
					{
						bNeedBonus = true;
					}
				}
			}
			//If we have the other prereq bonus, no need of eBonus
			if(bNeedBonus)
			{
				//put this here maybe should reduce the amount of looping
				bool bNeedTech = kBuilding.getPrereqAndTech() != NO_TECH && !GET_TEAM(getTeam()).isHasTech((TechTypes)kBuilding.getPrereqAndTech());
				if(!bNeedTech)
				{
					for (int iJ = 0; iJ < GC.getNUM_BUILDING_AND_TECH_PREREQS(); iJ++)
					{
						if (kBuilding.getPrereqAndTechs(iJ) != NO_TECH && !GET_TEAM(getTeam()).isHasTech((TechTypes)kBuilding.getPrereqAndTechs(iJ)))
						{
							bNeedTech = false;
							break;
						}
					}
				}
				if(!bNeedTech)
				{
					int iLoop;
					int iCountBuilding = 0;
					for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
					{
						//Check if except bonus resources city can construct the building
						if(pLoopCity->canConstruct(eBuilding,false,false,false,true))
						{
							iCountBuilding++;
						}
					}
					iValue+=iCountBuilding*50*iModifier;
				}
			}
		}
	}
	return iValue;
}
