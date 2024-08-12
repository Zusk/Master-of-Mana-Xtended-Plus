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


// Public Functions...

int CvCityAI::AI_getBuildingValue(BuildingTypes eBuilding)
{
	int iValue=0;

	iValue+=AI_buildingGrowthValue(eBuilding);
	iValue+=AI_buildingProductionValue(eBuilding);
	iValue+=AI_buildingCommerceValue(eBuilding);
	iValue+=AI_buildingGreatPersonValue(eBuilding);
	iValue+=AI_buildingHappinessValue(eBuilding);
	iValue+=AI_buildingMilitaryValue(eBuilding);
	iValue+=AI_buildingReligionValue(eBuilding);
	iValue+=AI_buildingRevolutionValue(eBuilding);
	iValue+=AI_buildingWorldWonderValue(eBuilding);
	iValue+=AI_buildingMiscValue(eBuilding);
	iValue+=GC.getBuildingInfo(eBuilding).getAIWeight();

	if(iValue < 0)
		return -1000;

	iValue*=100;
	iValue/=std::max(1,getProductionTurnsLeft(eBuilding,0));
	//block Palace
	if(GC.getBuildingClassInfo((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType()).getMaxPlayerInstances()==1)
	{
		if(GC.getBuildingClassInfo((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType()).getExtraPlayerInstances()==1)
		{
			iValue=-100;
		}
	}

	return iValue;
}

int CvCityAI::AI_buildingGrowthValue(BuildingTypes eBuilding)
{
	int YieldValue=GC.getDefineINT("AI_BUILDINGVALUE_YIELD");
	int iValue=0;
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	iValue+=YieldValue*getAdditionalYieldByBuilding(YIELD_FOOD,eBuilding);

	int iGood = 0;
	int iBad = 0;
	int iHealth = getAdditionalHealthByBuilding(eBuilding, iGood, iBad);

	int iHealthReserve=goodHealth();

	//in Cottage Economy produce some Health Reserve for cottages
	if(kPlayer.AI_getEconomyType()==AIECONOMY_COTTAGE)
	{
		iHealthReserve=std::max(0,iHealthReserve-3);
	}

	if(badHealth(false)>iHealthReserve)
	{
		iValue+=2*YieldValue*std::min(iGood,badHealth(false)-iHealthReserve);
	}

	if(iHealthReserve<badHealth(false)+iBad)
	{
		iValue+=-2*YieldValue*std::min(iBad,badHealth(false)+iBad-iHealthReserve);
	}

	iValue+=YieldValue*kBuilding.getFoodKept()*growthThreshold()/2000;

	//deactivate for now - food can be spend in other ways than just population
	/**
	if(happyLevel()<=unhappyLevel(0))
	{
		iValue=std::min(iValue,0);
	}
	**/

	return iValue;
}

int CvCityAI::AI_buildingGreatPersonValue(BuildingTypes eBuilding)
{
	int GreatPersonValue=GC.getDefineINT("AI_BUILDINGVALUE_GREATPERSON");
	int iValue=0;
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);
	
	iValue+=GreatPersonValue*getAdditionalBaseGreatPeopleRateByBuilding(eBuilding);

	iValue*=getGreatPeopleRateModifier();
	iValue/=100;

	return iValue;
}

int CvCityAI::AI_buildingReligionValue(BuildingTypes eBuilding)
{
	int ReligionValue=GC.getDefineINT("AI_BUILDINGVALUE_RELIGION");
	int iValue=0;
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);

	int iNumBuildingsOwned=GET_PLAYER(getOwnerINLINE()).getBuildingClassCount((BuildingClassTypes)kBuilding.getBuildingClassType());

	//Building Allows Missionaries
	if(iNumBuildingsOwned==0 || iNumBuildingsOwned<GET_PLAYER(getOwnerINLINE()).getNumCities()/5)
	{
		ReligionTypes eReligion=GET_PLAYER(getOwnerINLINE()).getStateReligion();

		if(eReligion!=NO_RELIGION)
		{
			for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
			{
				UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);

				if(eUnit!=NO_UNIT && GC.getUnitInfo(eUnit).getReligionSpreads(eReligion)>0 && GC.getUnitInfo(eUnit).getPrereqBuildingClass()==kBuilding.getBuildingClassType())
				{
					iValue+=ReligionValue*100;
				}
			}
		}
	}	

	return iValue;
}

int CvCityAI::AI_buildingProductionValue(BuildingTypes eBuilding)
{
	int YieldValue=GC.getDefineINT("AI_BUILDINGVALUE_PRODUCTION");
	int iValue=0;
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);

	iValue+=YieldValue*getAdditionalYieldByBuilding(YIELD_PRODUCTION,eBuilding);

	for(int iI = YIELD_LUMBER; iI < NUM_YIELD_TYPES; iI++)
	{		
		iValue += AI_valueYield((YieldTypes)iI, getAdditionalYieldByBuilding((YieldTypes)iI,eBuilding)) * std::max(10, GET_PLAYER(getOwnerINLINE()).AI_getYieldValue((YieldTypes)iI)) * 5;
	}
	return iValue;
}

int CvCityAI::AI_buildingMilitaryValue(BuildingTypes eBuilding)
{
	int MilitaryValue=GC.getDefineINT("AI_BUILDINGVALUE_MILITARY");
	int iValue=0;
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);
	CvPlayer& kPlayer=GET_PLAYER(getOwnerINLINE());

	if(kBuilding.getDefenseModifier()!=0)
	{
		iValue+=AI_dangerValue()*kBuilding.getDefenseModifier()*MilitaryValue/2000;
	}

	iValue += kBuilding.getGlobalFreeExperience() * 100;

	for(int iI=0;iI<GC.getNumUnitCombatInfos();iI++)
	{
		if(kBuilding.getUnitCombatFreeExperience(iI)!=0)
		{
			int iBestUnitValue=0;
			for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
			{
				UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);

				if(eUnit!=NO_UNIT && GC.getUnitInfo(eUnit).getUnitCombatType()==iI && canTrain(eUnit,true))
				{
					iBestUnitValue=std::max(iBestUnitValue,AI_getUnitValue(eUnit));
				}
			}
			iValue+=kBuilding.getUnitCombatFreeExperience(iI)*iBestUnitValue/20;
		}
	}

	//Unit SupportLimit Change
	if(kBuilding.getUnitSupportChange()!=0)
	{
		int iMod = 1;
		if(kPlayer.getUnitSupportUsed() * 5 > kPlayer.getUnitSupportLimitTotal() * 4)
		{
			if(kPlayer.getCommercePercent(COMMERCE_GOLD) < 70)
			{
				iMod = 10;
			}
		}
		iValue+=100 * kBuilding.getUnitSupportChange() * iMod;		
	}

	//Military Production Increase
	if(kBuilding.getMilitaryProductionModifier()!=0)
	{
		if(AI_getCitySpecialization()==CITYSPECIALIZATION_MILITARY)
		{
			iValue += kBuilding.getMilitaryProductionModifier()*100;
		}
	}

	return iValue;
}

int CvCityAI::AI_buildingHappinessValue(BuildingTypes eBuilding)
{
	int PopulationValue=GC.getDefineINT("AI_BUILDINGVALUE_HAPPINESS");
	int iValue=0;
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);

	int iGood = 0, iBad = 0, iChange = getAdditionalHappinessByBuilding(eBuilding, iGood, iBad);

	if(happyLevel()<unhappyLevel(0)+2)
	{
		iValue+=PopulationValue*std::min(iGood,unhappyLevel(0)+2-happyLevel());
	}

	if(happyLevel()<=unhappyLevel(0))
	{
		iValue+=-PopulationValue*iBad;
	}

	return iValue;
}

int CvCityAI::AI_buildingCommerceValue(BuildingTypes eBuilding)
{
	int CommerceValue=GC.getDefineINT("AI_BUILDINGVALUE_COMMERCE");
	int MagicValue=GC.getDefineINT("AI_BUILDINGVALUE_MAGIC");

	int iValue = 0;
	int ResearchValue = CommerceValue;
	int GoldValue = CommerceValue;

	switch(AI_getCitySpecialization())
	{
		case CITYSPECIALIZATION_GOLD:
			GoldValue *= 4;
			break;
		case CITYSPECIALIZATION_RESEARCH:
		case CITYSPECIALIZATION_ARCANE:
			ResearchValue *= 4;
			break;
		default:
			break;
	}
	if(GET_PLAYER(getOwnerINLINE()).AI_isFinancialTrouble())
	{
		GoldValue *= 10;
	}
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);

	iValue+=CommerceValue*getAdditionalYieldByBuilding(YIELD_COMMERCE,eBuilding);
	iValue+=CommerceValue*getSavedMaintenanceTimes100ByBuilding(eBuilding)/100;
	iValue+=ResearchValue*getAdditionalCommerceTimes100ByBuilding(COMMERCE_RESEARCH,eBuilding)/100;
	iValue+=ResearchValue*getAdditionalCommerceTimes100ByBuilding(COMMERCE_CULTURE,eBuilding)/400;
	iValue+=GoldValue*getAdditionalCommerceTimes100ByBuilding(COMMERCE_GOLD,eBuilding)/100;
	iValue+=MagicValue*getAdditionalCommerceTimes100ByBuilding(COMMERCE_ARCANE,eBuilding)/100;
	iValue+=MagicValue*getAdditionalCommerceTimes100ByBuilding(COMMERCE_MANA,eBuilding)/100;

	return iValue;
}

int CvCityAI::AI_buildingRevolutionValue(BuildingTypes eBuilding)
{
	//set to 0 for now, later we can use it to teach AI how to use Holy City States
	return 0;
	
}

int CvCityAI::AI_buildingWorldWonderValue(BuildingTypes eBuilding)
{
	int iValue=0;
	CvBuildingInfo &kBuilding=GC.getBuildingInfo(eBuilding);
	int iLoop;
	CvCity* pLoopCity;
	CvPlayerAI &kPlayer = GET_PLAYER(getOwnerINLINE());

	if(kBuilding.getFreeBuildingClass()!=NO_BUILDINGCLASS)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(kPlayer.getCivilizationType()).getCivilizationBuildings(kBuilding.getFreeBuildingClass());
		if(eBuilding != NO_BUILDING)
		{
			for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				eBuilding = (BuildingTypes)GC.getCivilizationInfo(pLoopCity->getCivilizationType()).getCivilizationBuildings(kBuilding.getFreeBuildingClass());
				if(eBuilding != NO_BUILDING && pLoopCity->getNumRealBuilding(eBuilding) == 0)
				{
					iValue += pLoopCity->AI_getBuildingValue(eBuilding);
				}
			}
		}
	}

	if(kBuilding.getGlobalHappiness()!=0 || kBuilding.getAreaHappiness()!=0)
	{
		int PopulationValue=GC.getDefineINT("AI_BUILDINGVALUE_HAPPINESS");

		for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
		{
			if(pLoopCity!=this)
			{
				int iChange=kBuilding.getGlobalHappiness();
				if(pLoopCity->getArea()==getArea())
				{
					iChange+=kBuilding.getAreaHappiness();
				}
				int iGood=std::max(iChange,0);
				int iBad=std::min(iChange,0);
				if(pLoopCity->happyLevel()<pLoopCity->unhappyLevel(0)+2)
				{
					iValue+=PopulationValue*std::min(iGood,pLoopCity->unhappyLevel(0)+2-pLoopCity->happyLevel());
				}

				if(pLoopCity->happyLevel()<=pLoopCity->unhappyLevel(0))
				{
					iValue+=-PopulationValue*iBad;
				}
			}
		}
	}

	if(kBuilding.getGlobalHealth()!=0 || kBuilding.getAreaHealth()!=0)
	{
		int YieldValue=GC.getDefineINT("AI_BUILDINGVALUE_YIELD");

		for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
		{
			if(pLoopCity!=this)
			{
				int iChange=kBuilding.getGlobalHealth();
				if(pLoopCity->getArea()==getArea())
				{
					iChange+=kBuilding.getAreaHealth();
				}
				int iGood=std::max(iChange,0);
				int iBad=std::min(iChange,0);

				if(pLoopCity->badHealth(false)>pLoopCity->goodHealth())
				{
					iValue+=YieldValue*std::min(iGood,pLoopCity->badHealth(false)-pLoopCity->goodHealth());
				}

				if(pLoopCity->goodHealth()<pLoopCity->badHealth(false)+iBad)
				{
					iValue+=-YieldValue*std::min(iBad,pLoopCity->badHealth(false)+iBad-pLoopCity->goodHealth());
				}
			}
		}
	}

	if(kBuilding.getGlobalTradeRoutes()!=0 || kBuilding.getCoastalTradeRoutes()!=0)
	{
		int iNewTotalTradeYield=0;
		for(int iI=0;iI<GC.getNUM_YIELD_TYPES();iI++)
		{
			YieldTypes eIndex=(YieldTypes)iI;
			int iPlayerTradeYieldModifier = kPlayer.getTradeYieldModifier(eIndex);

#ifdef _MOD_FRACTRADE
			int iTradeProfitDivisor = 100;
#else
			int iTradeProfitDivisor = 10000;
#endif

			for (pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				int iWorstTradeRouteProfit=MAX_INT;
				for (int iI = 0; iI < pLoopCity->getTradeRoutes(); ++iI)
				{
					CvCity* pCity = pLoopCity->getTradeCity(iI);
					if (pCity)
					{
						int iTradeProfit = pLoopCity->getBaseTradeProfit(pCity);
						int iTradeModifier = pLoopCity->totalTradeModifier(pCity);
						int iTradeYield = iTradeProfit * iTradeModifier / iTradeProfitDivisor * iPlayerTradeYieldModifier;

						iWorstTradeRouteProfit=std::min(iWorstTradeRouteProfit,iTradeYield);
					}
				}

				if(iWorstTradeRouteProfit!=MAX_INT)
				{
					int iNumTradeRouteChange=kBuilding.getGlobalTradeRoutes();
					if(pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
					{
						iNumTradeRouteChange+=kBuilding.getCoastalTradeRoutes();
					}
					iNumTradeRouteChange=std::max(-getTradeRoutes(),iNumTradeRouteChange);
					iNumTradeRouteChange=std::min(iNumTradeRouteChange,GC.getDefineINT("MAX_TRADE_ROUTES")-getTradeRoutes());

					iNewTotalTradeYield += iWorstTradeRouteProfit*iNumTradeRouteChange;
				}
				iNewTotalTradeYield /= 100;
			}
		}
		iValue+=iNewTotalTradeYield*GC.getDefineINT("AI_BUILDINGVALUE_COMMERCE");
	}

	iValue+=kBuilding.getFreeTechs()*10000;

	if(kBuilding.getFreeBonus()!=NO_BONUS)
	{
		if(kPlayer.getNumAvailableBonuses((BonusTypes)kBuilding.getFreeBonus())==0)
		{
			iValue+=1000;	//need to write an AI_Weight for Bonuses Later
		}
	}

	return iValue;
}

int CvCityAI::AI_buildingMiscValue(BuildingTypes eBuilding)
{
//District AI
	CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);
	if(kBuilding.isDistrict() && AI_getCitySpecialization() != NO_CITYSPECIALIZATION)
	{
		if((kBuilding.getAISpecialization1() != AI_getCitySpecialization())
			&& (kBuilding.getAISpecialization2() != AI_getCitySpecialization())
			)
			return -100000;
	}

	TerrainTypes eTerrainSnow = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW");
	if(eBuilding == GC.getInfoTypeForString("BUILDING_TEMPLE_OF_THE_HAND") && eTerrainSnow != NO_TERRAIN)
	{
		int iTempValue=0;
		for (int iI = 0; iI < getNumCityPlots(); iI++)
		{
			CvPlot* pLoopPlot = getCityIndexPlot(iI);

			if (pLoopPlot != NULL)
			{
				if(!(pLoopPlot->isWater() || pLoopPlot->isPeak()))
				{
					if(pLoopPlot->isRiver())
					{
						iTempValue-=100;
					}
					for(int iJ=0;iJ<GC.getNUM_YIELD_TYPES();iJ++)
					{
						iTempValue+=100*GC.getTerrainInfo(eTerrainSnow).getYield((YieldTypes)iJ)-GC.getTerrainInfo(pLoopPlot->getTerrainType()).getYield((YieldTypes)iJ);
					}
				}
			}
		}

		if(iTempValue>500)
		{
			return 1000;
		}

	}

	return 0;
}


int CvCityAI::AI_getValueAnyUnit()
{
	int iValue=GC.getDefineINT("AI_BUILDINGVALUE_ANYUNIT");
	CvPlayerAI &kPlayer = GET_PLAYER(getOwnerINLINE());

	if(isCapital() || area()->getCitiesPerPlayer(getOwnerINLINE())>3)
	{
	 	if(area()->AI_getPotentialCounterUnits(getOwnerINLINE())<4)
		{
			return MAX_INT;
		}
	}

	//Determines whether we need to build more conquest units in conquest mode
	//TODO: fix exploit that war can be declared just to make enemy build units
	if(GET_TEAM(getTeam()).getAtWarCount(false)>0 || GET_TEAM(getTeam()).AI_getWarPlanTarget() != NO_TEAM)
	{
		iValue*=5;
	}
	else if(GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI)) {
		iValue *= 2;
	}

	return iValue;
}

int CvCityAI::AI_getUnitValue(UnitTypes eUnit)
{
	int iValue;
	CvPlayerAI &kPlayer = GET_PLAYER(getOwnerINLINE());
	CvUnitInfo &kUnit = GC.getUnitInfo(eUnit);

	iValue=AI_ValueUnitBaseCombat(eUnit);
	
	for(int iI=0;iI<GC.getNumReligionInfos();iI++)
	{
		if(kUnit.getReligionSpreads(iI)>0)
		{			
			if(kPlayer.getUnitClassCountPlusMaking((UnitClassTypes)kUnit.getUnitClassType())==0)
			{
				iValue += GC.getDefineINT("AI_BUILDINGVALUE_RELIGION")*kPlayer.AI_missionaryValue(area(), (ReligionTypes)iI);
			}
		}
	}

	for(int iI=0;iI<GC.getNumCorporationInfos();iI++)
	{
		if(kUnit.getCorporationSpreads(iI)>0  && kPlayer.countCorporationSpreadUnits(area(),(CorporationTypes)iI))
		{			
			iValue += GC.getDefineINT("AI_BUILDINGVALUE_REVOLUTION")*100*kPlayer.getAI_lostCitiesRevolutionMemory();
		}	
	}
/**
//need AI first to use this
	for(int iI=0;iI<GC.getNumBuildingInfos();iI++)
	{
		if(kUnit.getBuildings((BuildingTypes)iI) && getNumRealBuilding((BuildingTypes)iI)==0)
		{
			iValue+=AI_getBuildingValue((BuildingTypes)iI);
		}
	}
**/
	int iTempValue=100;
	for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
	{
		if(kUnit.getFreePromotions(iI))
		{
			iTempValue+=GC.getPromotionInfo((PromotionTypes)iI).getAIWeight();
		}
	}

	iValue*=iTempValue;
	iValue/=100;

	iTempValue=100;
	for(int iI=0;iI<GC.getNumSpellInfos();iI++)
	{
		CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)iI);
		if(kSpell.getUnitCombatPrereq()==NO_UNITCOMBAT && kSpell.getUnitClassPrereq()==NO_UNITCLASS && kSpell.getUnitPrereq()==NO_UNIT && kSpell.getPromotionPrereq1()==NO_PROMOTION)
		{
			continue;
		}
		if(kSpell.getUnitPrereq()!=NO_UNIT && kSpell.getUnitPrereq()!=eUnit)
		{
			continue;			
		}
		if (kSpell.getUnitClassPrereq()!=NO_UNITCLASS && kSpell.getUnitClassPrereq()!=kUnit.getUnitClassType())
		{
			continue;
		}
		if (kSpell.getUnitCombatPrereq()!=NO_UNITCOMBAT && kSpell.getUnitCombatPrereq()!=kUnit.getUnitCombatType()) 
		{
			continue;
		}
		bool bValid=false;
		for(int iJ=0;iJ<4;iJ++)
		{
			PromotionTypes ePromotion=NO_PROMOTION;
			if(iJ==0)
				ePromotion=(PromotionTypes)kSpell.getPromotionPrereq1();
			if(iJ==1)
				ePromotion=(PromotionTypes)kSpell.getPromotionPrereq2();
			if(iJ==2)
				ePromotion=(PromotionTypes)kSpell.getPromotionPrereq3();
			if(iJ==3)
				ePromotion=(PromotionTypes)kSpell.getPromotionPrereq4();

			if(ePromotion!=NO_PROMOTION && GC.getPromotionInfo(ePromotion).getMinLevel()==-1 && GC.getUnitInfo(eUnit).getFreePromotions(ePromotion))
			{
				bValid=true;
			}
		}
		if(bValid)
		{
			iTempValue+=GC.getSpellInfo((SpellTypes)iI).getAIWeight();		
		}
	}
	iTempValue=std::min(iTempValue,130);

	iValue*=iTempValue;
	iValue/=100;

	return iValue;
}

int CvCityAI::AI_ValueUnitBaseCombat(UnitTypes eUnit, bool bOnlyDefense, bool bWater)
{
	CvUnitInfo &kUnit = GC.getUnitInfo(eUnit);

	if(bWater && (kUnit.getDomainType()!=DOMAIN_SEA))
	{
		return -100;
	}

	if(!bWater && (kUnit.getDomainType()==DOMAIN_SEA))
	{
		return -100;
	}

	int iValue=kUnit.getCombatDefense();
	iValue+=(bOnlyDefense) ? 0 : kUnit.getCombat();

	for(int iI=0;iI<GC.getNumBonusInfos();iI++)
	{
		if(kUnit.getBonusAffinity(iI)!=0)
		{
			iValue+=kUnit.getBonusAffinity(iI)*((bOnlyDefense)? 1 : 2) * GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses((BonusTypes)iI);
		}
	}

    for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
    {
        iValue += GC.getUnitInfo(eUnit).getDamageTypeCombat((DamageTypes) iI) *((bOnlyDefense)? 1 : 2);
    }

    if(GC.getUnitInfo(eUnit).isScaleWithTech())
	{
		for(int iI=0;iI<GC.getNumTechInfos();iI++)
		{
			if(GC.getTechInfo((TechTypes)iI).getIncreaseStrengthOfRegularUnits() > 0)
			{
				if ((GET_TEAM(GET_PLAYER(getOwnerINLINE()).getTeam()).isHasTech((TechTypes)iI)))
				{
					if(GC.getTechInfo((TechTypes)iI).getPrereqReligion()==NO_RELIGION || GC.getTechInfo((TechTypes)iI).getPrereqReligion()==GET_PLAYER(getOwnerINLINE()).getStateReligion())
					{
						iValue+=(bOnlyDefense) ? 1 : 2;
					}
				}
			}
		}
	}

	if(GC.getUnitInfo(eUnit).isImmortal()) {
		iValue *= 3;
		iValue /= 2;
	}

	return iValue*iValue*100;
}

int CvCityAI::AI_ValueCityCounter(UnitTypes eUnit)
{
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);
	if(isWorldUnitClass((UnitClassTypes)kUnit.getUnitClassType()))
	{
		return -MAX_INT;
	}

	if(kUnit.isAIblockPermDefense())
	{
		return -MAX_INT;
	}

	if(GC.getUnitClassInfo((UnitClassTypes)kUnit.getUnitClassType()).getMaxPlayerInstances()!=-1)
	{
		return -MAX_INT;
	}

	//no Unique ranged Units aren't picked as counters so City defense is less prone to Cover Promotions
	if(eUnit==GC.getUnitClassInfo((UnitClassTypes)kUnit.getUnitClassType()).getDefaultUnitIndex())
	{
		if(kUnit.getUnitCombatType() != NO_UNITCOMBAT && GC.getUnitCombatInfo((UnitCombatTypes)kUnit.getUnitCombatType()).isArcherSupport())
			return -MAX_INT;
	}

	int iValue;
	iValue=10*AI_ValueUnitBaseCombat(eUnit,true);
	if(GC.getUnitInfo(eUnit).isNoDefensiveBonus())
	{
		iValue/=2;
	}
	else
	{
		iValue*=(100+GC.getUnitInfo(eUnit).getCityDefenseModifier());
		iValue/=100;
		if(plot()->isHills())
		{
			iValue*=(100+GC.getUnitInfo(eUnit).getHillsDefenseModifier());
			iValue/=100;
		}
	}

	return iValue;
}

int CvCityAI::AI_ValueCityDefender(UnitTypes eUnit, CvAIGroup* pAIGroup)
{
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);
	if(isWorldUnitClass((UnitClassTypes)kUnit.getUnitClassType()))
	{
		return -MAX_INT;
	}

	if(kUnit.isAIblockPermDefense())
	{
		return -MAX_INT;
	}

	if(GC.getUnitClassInfo((UnitClassTypes)kUnit.getUnitClassType()).getMaxPlayerInstances()!=-1)
	{
		return -MAX_INT;
	}

	int iValue;
	iValue=10*AI_ValueUnitBaseCombat(eUnit,true);
	if(kUnit.isNoDefensiveBonus())
	{
		iValue/=2;
	}
	else
	{
		iValue*=(100+kUnit.getCityDefenseModifier());
		iValue/=100;
		if(plot()->isHills())
		{
			iValue*=(100+kUnit.getHillsDefenseModifier());
			iValue/=100;
		}
	}

	if(!kUnit.isMilitaryHappiness())
	{
		if(pAIGroup==NULL)
		{
			iValue=0;
		}
		else
		{
			int iCount=0;
			for (CLLNode<IDInfo>* pUnitNode = pAIGroup->headUnitNode(); pUnitNode != NULL; pUnitNode = pAIGroup->nextUnitNode(pUnitNode))
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

				if(GC.getUnitInfo(pLoopUnit->getUnitType()).isMilitaryHappiness())
				{
					iCount++;
				}
			}

			if(iCount<1)
			{
				iValue=0;
			}
		}
	}

	return iValue;
}

int CvCityAI::AI_ValueCitySiegeUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	if(pAIGroup->getNumUnitCategoryUnits(UNITCATEGORY_SIEGE)<3)
	{
		if(!GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(eUnit,UNITCATEGORY_SIEGE))
		{
			return -MAX_INT;
		}

		int iValue=100;
		iValue+=GC.getUnitInfo(eUnit).getBombardRate();
		iValue*=GC.getUnitInfo(eUnit).getBombardRange();
		iValue*=(1+GC.getUnitInfo(eUnit).getMoves());

		return iValue;
	}

	return AI_ValueCounterUnit(pAIGroup,eUnit);
}

int CvCityAI::AI_ValueCityInvasionUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);

	if(kUnit.getMoves()<1)
	{
		return -MAX_INT;
	}
	if (kUnit.isOnlyDefensive())
	{
		return -MAX_INT;
	}

	int iValue;
	iValue=10*AI_ValueUnitBaseCombat(eUnit);

	iValue*=100+kUnit.getCityAttackModifier();
	iValue/=100;

	//give Melee Units a 20% bonus
	if(kUnit.getUnitCombatType() != NO_UNITCOMBAT && GC.getUnitCombatInfo((UnitCombatTypes)kUnit.getUnitCombatType()).getCityAttack() > 0)
	{
		iValue*=120;
		iValue/=100;
	}

	//Wizards
	if(pAIGroup->getNumUnits()>5)
	{
		if(pAIGroup->getNumWarwizardsNeeded()>0)
		{
			if(GET_PLAYER(getOwnerINLINE()).AI_canUseMoreOfUnitCategory(UNITCATEGORY_WARWIZARD,getArea()))
			{
				if(GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(eUnit,UNITCATEGORY_WARWIZARD))
				{
					iValue*=1000;
				}
			}
		}
	}

	return iValue;
}

int CvCityAI::AI_ValueWarwizardUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);

	if(kUnit.getMoves()<1)
	{
		return -MAX_INT;
	}

	if(!GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(eUnit,UNITCATEGORY_WARWIZARD))
	{
		return -MAX_INT;
	}

	int iValue;
	iValue=10*AI_ValueUnitBaseCombat(eUnit);

	return iValue;
}


int CvCityAI::AI_ValueCounterUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);
	if(isWorldUnitClass((UnitClassTypes)kUnit.getUnitClassType()))
	{
		return -MAX_INT;
	}

	if(kUnit.getMoves()<1)
	{
		return -MAX_INT;
	}

	int iValue;
	iValue=10*AI_ValueUnitBaseCombat(eUnit);

	iValue*=(60+40*kUnit.getMoves());
	iValue/=100;

	if(pAIGroup->getMissionTargetUnit()!=NULL && pAIGroup->getMissionTargetUnit()->isInvisible(getTeam(),false))
	{
		InvisibleTypes iInvisible=pAIGroup->getMissionTargetUnit()->getInvisibleType();
		if(!pAIGroup->isSeeInvisible())
		{
			bool bValid=false;
			for(int iI=0;iI<kUnit.getNumSeeInvisibleTypes();iI++)
			{
				if(kUnit.getSeeInvisibleType(iI)==iInvisible)
				{
					bValid=true;
					break;
				}
			}
			if(bValid)
			{
				iValue*=100;
			}
		}
	}

	return iValue;
}

bool CvCityAI::AI_chooseProductionBarbarian()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvWString szBuffer;

	bool bDefenderNeeded=true;

	if(plot()->getNumUnits()>0)
	{
        CLLNode<IDInfo>* pUnitNode;
        CvUnit* pLoopUnit;
        pUnitNode = plot()->headUnitNode();

        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = plot()->nextUnitNode(pUnitNode);
            if (pLoopUnit!=NULL && pLoopUnit->getOwnerINLINE()==getOwnerINLINE())
            {
				if(pLoopUnit->AI_getUnitAIType()==UNITAI_CITY_DEFENSE)
				{
					bDefenderNeeded=false;
					break;
				}
			}
		}
	}


	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit,true))
		{
			if(GC.getUnitClassInfo((UnitClassTypes)iUnitIndex).getMaxGlobalInstances()==-1 || GET_PLAYER(getOwnerINLINE()).getUnitClassCountPlusMaking((UnitClassTypes)iUnitIndex)<GC.getUnitClassInfo((UnitClassTypes)iUnitIndex).getMaxGlobalInstances())
			{
				if(bDefenderNeeded)
				{
					iValue=AI_ValueCityDefender(eUnit,NULL);
				}
				else
				{
					iValue=10*AI_ValueUnitBaseCombat(eUnit);
				}

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestUnit=eUnit;
				}
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,(bDefenderNeeded) ? UNITAI_CITY_DEFENSE : UNITAI_ATTACK,false,false,false);
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseBuildingHealth()
{
	BuildingTypes eBestBuilding=NO_BUILDING;
	int iBestValue=0;
	int iValue;

	//Only if unhealth is preventing grow
	if(!GET_PLAYER(getOwnerINLINE()).isIgnoreFood() && (goodHealth() - badHealth() <= 0) && foodDifference() < 3 )
	{
		for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
		{
			BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);

			if(canConstruct(eBuilding,true))
			{
				if(!GC.getBuildingInfo(eBuilding).isDistrict())
				{
					int iProductionTurns = getProductionTurnsLeft(eBuilding,0);
					if(iProductionTurns < 10)
					{
						int iGood = 0, iBad = 0, iChange = getAdditionalHealthByBuilding(eBuilding, iGood, iBad);
						if(iChange > 0)
						{
							iValue = std::max(0,iChange * 100);
							if(iValue > 0)
							{
								//The cheaper the better
								iValue += std::max(0,10 - getProductionTurnsLeft(eBuilding,0)) * 100;

							}
							if(iValue > iBestValue)
							{
								iBestValue=iValue;
								eBestBuilding=eBuilding;
							}
						}
					}
				}
			}
		}
	}

	if(eBestBuilding!=NO_BUILDING)
	{
		pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,AI_chooseBuildingHealth,Happy:%d,Health:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestBuilding
				,GC.getBuildingInfo(eBestBuilding).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,happyLevel() - unhappyLevel()
				,goodHealth() - badHealth()
			);
		}
		return true;
	}

	return false;
}
bool CvCityAI::AI_chooseBuildingHappiness()
{
	BuildingTypes eBestBuilding=NO_BUILDING;
	int iBestValue=0;
	int iValue;

	//Only if there is no much happiness left and we are growing...
	if (happyLevel() - unhappyLevel() < 2 && (GET_PLAYER(getOwnerINLINE()).isIgnoreFood() || foodDifference() > 0))
	{
		for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
		{
			BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);

			if(canConstruct(eBuilding,true))
			{
				if(!GC.getBuildingInfo(eBuilding).isDistrict())
				{
					int iProductionTurns = getProductionTurnsLeft(eBuilding,0);
					if(iProductionTurns < 10)
					{
						int iGood = 0, iBad = 0, iChange = getAdditionalHappinessByBuilding(eBuilding, iGood, iBad);
						if(iChange > 0)
						{
							iValue = std::max(0,iChange * 100);
							if(iValue > 0)
							{
								//The cheaper the better
								iValue += std::max(0,10 - getProductionTurnsLeft(eBuilding,0)) * 100;

							}
/*
 *
					foodDifference()
					int iGood = 0, iBad = 0, iChange = getAdditionalHappinessByBuilding(eBuilding, iGood, iBad);



					int PopulationValue=GC.getDefineINT("AI_BUILDINGVALUE_HAPPINESS");
					int iValue=0;
					CvBuildingInfo &kBuilding = GC.getBuildingInfo(eBuilding);

					int iGood = 0, iBad = 0, iChange = getAdditionalHappinessByBuilding(eBuilding, iGood, iBad);

					if(happyLevel()<unhappyLevel(0)+2)
					{
						iValue+=PopulationValue*std::min(iGood,unhappyLevel(0)+2-happyLevel());
					}

					if(happyLevel()<=unhappyLevel(0))
					{
						iValue+=-PopulationValue*iBad;
					}

					return iValue;
*/


							if(iValue > iBestValue)
							{
								iBestValue=iValue;
								eBestBuilding=eBuilding;
							}
						}
					}
				}
			}
		}
	}

	if(eBestBuilding!=NO_BUILDING)
	{
		pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,AI_chooseBuildingHappiness,Happy:%d,Health:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestBuilding
				,GC.getBuildingInfo(eBestBuilding).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,happyLevel() - unhappyLevel()
				,goodHealth() - badHealth()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseCheapGoldBuilding()
{
	BuildingTypes eBestBuilding=NO_BUILDING;
	int iBestValue=100;
	int iValue;

	for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);
		if(canConstruct(eBuilding,true))
		{
			if(!GC.getBuildingInfo(eBuilding).isDistrict() && getProductionTurnsLeft(eBuilding,0) < 3) {
				iValue = getAdditionalCommerceTimes100ByBuilding(COMMERCE_GOLD,eBuilding);

				if(iValue > iBestValue)
				{
					iBestValue=iValue;
					eBestBuilding=eBuilding;
				}
			}
		}
	}
	if(eBestBuilding!=NO_BUILDING)
	{
		pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,AI_chooseCheapGoldBuilding,Happy:%d,Health:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestBuilding
				,GC.getBuildingInfo(eBestBuilding).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,happyLevel() - unhappyLevel()
				,goodHealth() - badHealth()
			);
		}
		return true;
	}

	return false;
}
bool CvCityAI::AI_chooseCheapProductionBuilding()
{
	BuildingTypes eBestBuilding=NO_BUILDING;
	int iBestValue=100;
	int iValue;

	int YieldValue=GC.getDefineINT("AI_BUILDINGVALUE_PRODUCTION");
	for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);
		if(canConstruct(eBuilding,true))
		{
			if(!GC.getBuildingInfo(eBuilding).isDistrict() && getProductionTurnsLeft(eBuilding,0) < 6) {
				iValue = YieldValue * getAdditionalYieldByBuilding(YIELD_PRODUCTION,eBuilding);

				if(iValue > iBestValue)
				{
					iBestValue=iValue;
					eBestBuilding=eBuilding;
				}
			}
		}
	}
	if(eBestBuilding!=NO_BUILDING)
	{
		pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,AI_chooseCheapProductionBuilding,Happy:%d,Health:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestBuilding
				,GC.getBuildingInfo(eBestBuilding).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,happyLevel() - unhappyLevel()
				,goodHealth() - badHealth()
			);
		}
		return true;
	}

	return false;
}
bool CvCityAI::AI_chooseCheapCultureBuilding()
{
	BuildingTypes eBestBuilding=NO_BUILDING;
	int iBestValue=100;
	int iValue;

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("CvCity::AI_chooseCheapCulture");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

	for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);
		iValue = 200;

		if(canConstruct(eBuilding,true))
		{
			if(!GC.getBuildingInfo(eBuilding).isDistrict()) {
				if(getAdditionalCommerceTimes100ByBuilding(COMMERCE_CULTURE,eBuilding)>50)
				{
					iValue=getProductionTurnsLeft(eBuilding,0);
				}

				if(iValue<iBestValue)
				{
					iBestValue=iValue;
					eBestBuilding=eBuilding;
				}
			}
		}
	}		
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("CvCity::AI_chooseCheapCulture");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

	if(eBestBuilding!=NO_BUILDING)
	{
		pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,AI_chooseCheapCultureBuilding,Happy:%d,Health:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestBuilding
				,GC.getBuildingInfo(eBestBuilding).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,happyLevel() - unhappyLevel()
				,goodHealth() - badHealth()
			);
		}
		return true;
	}

	return false;
}


bool CvCityAI::AI_chooseEarlyDefenseGroupUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;

/**
	if(!isCapital() || GET_PLAYER(getOwnerINLINE()).calculateUnitCost()>0)
	{
		return false;
	}
**/
	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			// maybe Replace later with different function
			iValue=AI_getUnitValue(eUnit);

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestUnit=eUnit;
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseEarlyDefenseGroupUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseHero()
{
	ReligionTypes eReligion = (ReligionTypes)GET_PLAYER((PlayerTypes)getOwnerINLINE()).getFavoriteReligion();
	if(eReligion!=NO_RELIGION)
	{
		if (GC.getReligionInfo(eReligion).getReligionHero1()!=NO_UNITCLASS)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(GC.getReligionInfo(eReligion).getReligionHero1());
			if(eUnit!=NO_UNIT && canTrain(eUnit))
			{
				pushOrder(ORDER_TRAIN,eUnit,NO_UNITAI,false,false,false);
				if(isOOSLogging())
				{
					oosLog("AIBestProductionValue"
						,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:0,CitySpecialization:%d,ReligiousHero1,SupportUsed:%d,Limit:%d"
						,GC.getGameINLINE().getElapsedGameTurns()
						,getOwner()
						,getID()
						,eUnit
						,GC.getUnitInfo(eUnit).getDescription()
						,AI_getCitySpecialization()
						,GET_PLAYER(getOwner()).getUnitSupportUsed()
						,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
					);
				}
				return true;
			}
		}

		if (GC.getReligionInfo(eReligion).getReligionHero2()!=NO_UNITCLASS)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(GC.getReligionInfo(eReligion).getReligionHero2());
			if(eUnit!=NO_UNIT && canTrain(eUnit))
			{
				pushOrder(ORDER_TRAIN,eUnit,NO_UNITAI,false,false,false);
				if(isOOSLogging())
				{
					oosLog("AIBestProductionValue"
						,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:0,CitySpecialization:%d,ReligiousHero2,SupportUsed:%d,Limit:%d"
						,GC.getGameINLINE().getElapsedGameTurns()
						,getOwner()
						,getID()
						,eUnit
						,GC.getUnitInfo(eUnit).getDescription()
						,AI_getCitySpecialization()
						,GET_PLAYER(getOwner()).getUnitSupportUsed()
						,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
					);
				}
				return true;
			}
		}
	}

	UnitTypes eHero = (UnitTypes)GC.getCivilizationInfo(GET_PLAYER((PlayerTypes)getOwnerINLINE()).getCivilizationType()).getHero();
	if(eHero!=NO_UNIT && canTrain(eHero))
	{
		pushOrder(ORDER_TRAIN,eHero,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:0,CitySpecialization:%d,CivHero,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eHero
				,GC.getUnitInfo(eHero).getDescription()
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseCityInvasionUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_CITY_INVASION)
		{
			if(getArea()==pAIGroup->getMissionArea())
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					pGroup=pAIGroup;
				}
			}
		}
	}

	if(pGroup==NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			if(kPlayer.AI_isUnitCategory(eUnit,UNITCATEGORY_COMBAT))
			{
				iValue=AI_ValueCityInvasionUnit(pGroup, eUnit);

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestUnit=eUnit;
				}
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseCityInvasionUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseCityLairUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_DESTROY_LAIR)
		{
			if(getArea()==pAIGroup->getMissionArea())
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					pGroup=pAIGroup;
				}
			}
		}
	}

	if(pGroup==NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			if(kPlayer.AI_isUnitCategory(eUnit,UNITCATEGORY_COMBAT))
			{
				iValue=AI_ValueCityInvasionUnit(pGroup, eUnit);

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestUnit=eUnit;
				}
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseCityLairUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseWarwizardUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;
	int iCounter=0;
	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_CITY_INVASION || pAIGroup->getGroupType()==AIGROUP_COUNTER || pAIGroup->getGroupType()==AIGROUP_RESERVE)
		{
			if(getArea()==pAIGroup->getMissionArea())
			{
				iCounter+=pAIGroup->getNumUnitCategoryUnits(UNITCATEGORY_WARWIZARD);
			}
		}
	}

	if(iCounter>=2)
	{
		return false;
	}

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			if(kPlayer.AI_isUnitCategory(eUnit,UNITCATEGORY_WARWIZARD))
			{
				iValue=AI_ValueWarwizardUnit(pGroup, eUnit);

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestUnit=eUnit;
				}
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseWarwizardUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseMagicNodeUnit()
{
	UnitTypes eBestUnit = NO_UNIT;
	int iBestValue = 0;
	int iValue;
	CvPlayerAI &kPlayer = GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup = NULL;
	int iLoop;
	int iCounter = 0;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_MANA_NODE)
		{
			if(getArea()==pAIGroup->getMissionArea())
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					pGroup=pAIGroup;
				}
			}
		}
	}

	if(pGroup == NULL)
		return false;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType() == AIGROUP_RESERVE)
		{
			if(getArea()==pAIGroup->getMissionArea())
			{
				iCounter += pAIGroup->getNumUnitCategoryUnits(UNITCATEGORY_WARWIZARD);
			}
		}
	}

	if(iCounter >= 2)
	{
		return false;
	}

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			if(kPlayer.AI_isUnitCategory(eUnit,UNITCATEGORY_WARWIZARD))
			{
				iValue=AI_ValueWarwizardUnit(pGroup, eUnit);

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestUnit=eUnit;
				}
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseMagicNodeUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseCitySiegeUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_SIEGE)
		{
			if(getArea()==pAIGroup->getMissionArea())
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					pGroup=pAIGroup;
				}
			}
		}
	}

	if(pGroup==NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			iValue=AI_ValueCitySiegeUnit(pGroup, eUnit);

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestUnit=eUnit;
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseCitySiegeUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseCounterUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_COUNTER)
		{
			if(getArea()==pAIGroup->getMissionArea())
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					if(stepDistance(getX_INLINE(),getY_INLINE(),pAIGroup->getMissionPlot()->getX_INLINE(),pAIGroup->getMissionPlot()->getY_INLINE())<20)
					{
						pGroup=pAIGroup;
						break;
					}
				}
			}
		}
	}

	if(pGroup==NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			iValue=AI_ValueCounterUnit(pGroup, eUnit);

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestUnit=eUnit;
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseCounterUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_choosePermDefenseReserveUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	bool bNoDefenseNeeded=true;

    int iSearchRange=10;
    int defvalue=0;
    int iDX,iDY;
    CvPlot* pLoopPlot;
	CvCity* pBestCity=NULL;
	int iPathTurns;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
			    if(pLoopPlot->isCity())
			    {
                    if(pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && pLoopPlot->getArea()==getArea())
                    {
						CvCityAI* pCity=static_cast<CvCityAI*>(pLoopPlot->getPlotCity());
						if(pCity->getAIGroup_Defense()==NULL || pCity->getAIGroup_Defense()->UnitsNeeded()>0)
						{						
//							if(getAIGroup_Defense()->getHeadUnit()->generatePath(pLoopPlot,0,false,&iPathTurns)) //would be better to check if a path exists?
//     						{
								iValue=pCity->AI_dangerValue();
								iValue/=std::max(1,iPathTurns);
								if(iValue>iBestValue)
								{
									iBestValue=iValue;
									pBestCity=pLoopPlot->getPlotCity();
								}
//							}
						}
					}
				}
			}
		}
	}

	if(pBestCity!=NULL)
	{
		CvCityAI* pCity=static_cast<CvCityAI*>(pBestCity);
		iBestValue=0;
		for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
			iValue = -1;

			if(eUnit!=NO_UNIT && canTrain(eUnit,true))
			{
				iValue=pCity->AI_ValueCityDefender(eUnit,getAIGroup_Defense());

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestUnit=eUnit;
				}
			}
		}		

		if(eBestUnit!=NO_UNIT)
		{
			pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
			if(isOOSLogging())
			{
				oosLog("AIBestProductionValue"
					,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_choosePermDefenseReserveUnit,SupportUsed:%d,Limit:%d"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwner()
					,getID()
					,eBestUnit
					,GC.getUnitInfo(eBestUnit).getDescription()
					,iBestValue
					,AI_getCitySpecialization()
					,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
				);
			}
			return true;
		}
	}

	return false;
}

bool CvCityAI::AI_chooseBestPermDefenseUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			iValue=AI_ValueCityDefender(eUnit,getAIGroup_Defense());

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestUnit=eUnit;
			}
		}
	}		

	//Search for Unit to Replace
	if(eBestUnit!=NO_UNIT)
	{
		int iLoop;
        for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
        {
	        if(pLoopCity->getArea()==getArea())
            {
                CLLNode<IDInfo>* pUnitNode;
                CvUnit* pLoopUnit;
                pUnitNode = pLoopCity->plot()->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pLoopCity->plot()->nextUnitNode(pUnitNode);
                    if(pLoopUnit!=NULL && pLoopUnit->AI_getGroupflag()==GROUPFLAG_PERMDEFENSE && pLoopUnit->AI_getUnitAIType()==UNITAI_CITY_DEFENSE)
                    {
						if(AI_ValueCityDefender(pLoopUnit->getUnitType(),getAIGroup_Defense())*120<iBestValue*100)
						{
							pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}

bool CvCityAI::AI_choosePermDefenseUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("CvCity::AI_choosePermDefenseUnit");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/


	if(getAIGroup_Defense()==NULL || getAIGroup_Defense()->UnitsNeeded()>0)
	{
		for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
			iValue = -1;

			if(canTrain(eUnit,true))
			{
				iValue=AI_ValueCityDefender(eUnit, getAIGroup_Defense());

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestUnit=eUnit;
				}
			}
		}		
	}

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("CvCity::AI_choosePermDefenseUnit");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_choosePermDefenseUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseSettlerUnit()
{
	//can't build another City
	if(GET_PLAYER((PlayerTypes)getOwnerINLINE()).getMaxCities()>0 && GET_PLAYER((PlayerTypes)getOwnerINLINE()).getMaxCities()<=GET_PLAYER((PlayerTypes)getOwnerINLINE()).getNumCities())
	{
		return false;
	}

	UnitClassTypes eSettler = (UnitClassTypes)GC.getUNITCLASS_SETTLER();
	UnitTypes eUnit = NO_UNIT;
	if(eSettler != NO_UNITCLASS) {
		eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eSettler);
	}

	if(eUnit == NO_UNIT || canTrain(eUnit,true) == false)
	{
		return false;
	}

	//move into cannottrain later
	if(GC.getUnitInfo(eUnit).getProductionCost() > 200) {
		if(getCivilizationType() == GC.getInfoTypeForString("CIVILIZATION_SCIONS"))
			return false;
	}

	//not enough Production for a Settler
	if(isCapital()) {
		if(happyLevel() > unhappyLevel(0)) {
			if(getFoodTurnsLeft() > 0 && getFoodTurnsLeft() * 2 < getProductionTurnsLeft(eUnit, 0)) {
				return false;
			}
		}
	}
	else if(getCurrentProductionDifference(false,false) < 10) {
		return false;
	}

	if(GET_PLAYER((PlayerTypes)getOwnerINLINE()).getUnitClassCountPlusMaking(eSettler) >= std::max(2, 1 + GET_PLAYER((PlayerTypes)getOwnerINLINE()).getNumCities() / 5))
	{
		return false;
	}



	//SpyFanatic: A settler may have been already created and not yet associated to a AIGROUP_SETTLE, thus wait at a minimum that it can be associated to a AIGroup and start to move to found a City
	int iExistingSettlers = 0;
	int iLoop;
	for (CvCity* pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).nextCity(&iLoop))
	{
		CvPlot* pCityPlot = GC.getMapINLINE().plotINLINE(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());
		for (CLLNode<IDInfo>* pUnitNode = pCityPlot->headUnitNode(); pUnitNode != NULL; pUnitNode = pCityPlot->nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			/*oosLog("AIBestProductionValueSettler"
					,"Turn: %d, PlayerID: %d, City [%d,%d] pLoopUnit:%d TurnCreated:%d UnitArea:%d ThisCityArea:%d"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwner()
					,getX_INLINE()
					,getY_INLINE()
					,pLoopUnit != NULL ? pLoopUnit->getUnitClassType() == (UnitClassTypes)GC.getUNITCLASS_SETTLER() : -1
					,pLoopUnit != NULL ? pLoopUnit->getGameTurnCreated() : -1
					,pLoopUnit->getArea()
					,getArea()
				);*/
			if(pLoopUnit != NULL && pLoopUnit->getUnitClassType() == (UnitClassTypes)GC.getUNITCLASS_SETTLER())
			{
				iExistingSettlers++;
				if(pLoopUnit->getGameTurnCreated() == GC.getGameINLINE().getElapsedGameTurns() && pLoopUnit->getArea() == getArea())
				{
					return false; //There is a Settler created this turn in one of the player city in the same area
				}
			}
		}
	}
	//SpyFanatic: Or check that if an AIGROUP exists (which will have a settler) its at a minimum complete
	for(CvAIGroup* pAIGroup = GET_PLAYER((PlayerTypes)getOwnerINLINE()).firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = GET_PLAYER((PlayerTypes)getOwnerINLINE()).nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_SETTLE && pAIGroup->getNumUnits() > 0 && (pAIGroup->getMissionPlot() == NULL || pAIGroup->getMissionPlot()->getArea() == getArea()) && pAIGroup->UnitsNeeded() > 0)
		{
			//If there is already a settler group in the area with not enough unit build another defense unit instead
			bool bPermDefense = AI_choosePermDefenseReserveUnit();
			/*oosLog("AIBestProductionValueSettler"
				,"Turn: %d, PlayerID: %d, City [%d,%d] bPermDefense:%d iMissionArea:%d iGroupArea:%d numunits:%d needed:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getX_INLINE()
				,getY_INLINE()
				,bPermDefense
				,pAIGroup->getMissionPlot()->getArea()
				,getArea()
				,pAIGroup->getNumUnits()
				,pAIGroup->UnitsNeeded()
			);*/
			if(bPermDefense)
			{
				return true; //If a reserve unit for an existing AIGROUP_SETTLE has been pushed to production, then return true to stop main production loop
			}
			return false; //No unit thus build something else instead
		}
	}


/*
	//Use world size?
	int iSpaceForCity = 0;
	//iSpaceForCity = GC.getMapINLINE().getWorldSize() <= 2 ? 1 : 2;
	int iReserveGroupUnit = 0;
	int iSettleGroupUnit = 0;
	for(CvAIGroup* pAIGroup = GET_PLAYER((PlayerTypes)getOwnerINLINE()).firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = GET_PLAYER((PlayerTypes)getOwnerINLINE()).nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_RESERVE && pAIGroup->getNumUnits() > 0)
		{
			iReserveGroupUnit+=pAIGroup->getNumUnits();
		}
		if(pAIGroup->getGroupType()==AIGROUP_SETTLE && pAIGroup->getNumUnits() > 0)
		{
			iSettleGroupUnit+=pAIGroup->getNumUnits();
		}
	}

	int iCityRange = GC.getMIN_CITY_RANGE();
	std::vector<CvPlot*> vPossibleCitySites;
	for (int iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
	{
		CvPlot* pCitySitePlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);
		if (GET_PLAYER(getOwnerINLINE()).canFound(pCitySitePlot->getX_INLINE(),pCitySitePlot->getY_INLINE()))
		{
			if(vPossibleCitySites.size() == 0)
			{
				oosLog("AIBestProductionValueSettler"
					,"Turn: %d, PlayerID: %d, City [%d,%d] canFound [%d,%d] area:%d iReserveGroupUnit:%d iSettleGroupUnit:%d grouparea:%d "
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwner()
					,getX_INLINE()
					,getY_INLINE()
					,pCitySitePlot->getX_INLINE()
					,pCitySitePlot->getY_INLINE()
					,pCitySitePlot->getArea()
					,iReserveGroupUnit
					,iSettleGroupUnit
					,getArea()
					//,getMissionPlot()!=NULL ? getMissionPlot()->getArea() : -1
				);
				vPossibleCitySites.push_back(pCitySitePlot);
				iSpaceForCity++;
			}
			else
			{
				bool bSeparateCityPlot = true;
				for (std::vector<CvPlot*>::iterator it = vPossibleCitySites.begin(); it != vPossibleCitySites.end(); it++)
				{
					int iXDist = abs((*it)->getX_INLINE() - pCitySitePlot->getX_INLINE());
					int iYDist = abs((*it)->getY_INLINE() - pCitySitePlot->getY_INLINE());
					if (iXDist <= iCityRange || iYDist <= iCityRange)
					{
						//oosLog("AIBestProductionValueSettler"
						//	,"Turn: %d, PlayerID: %d, City [%d,%d] canFound [%d,%d] area:%d near [%d,%d]"
						//	,GC.getGameINLINE().getElapsedGameTurns()
						//	,getOwner()
						//	,getX_INLINE()
						//	,getY_INLINE()
						//	,pCitySitePlot->getX_INLINE()
						//	,pCitySitePlot->getY_INLINE()
						//	,pCitySitePlot->getArea()
						//	,(*it)->getX_INLINE()
						//	,(*it)->getY_INLINE()
						//);
						bSeparateCityPlot = false;
						break;
					}
				}
				if(bSeparateCityPlot)
				{
					oosLog("AIBestProductionValueSettler"
						,"Turn: %d, PlayerID: %d, City [%d,%d] canFound [%d,%d] area:%d range:%d iReserveGroupUnit:%d iSettleGroupUnit:%d"
						,GC.getGameINLINE().getElapsedGameTurns()
						,getOwner()
						,getX_INLINE()
						,getY_INLINE()
						,pCitySitePlot->getX_INLINE()
						,pCitySitePlot->getY_INLINE()
						,pCitySitePlot->getArea()
						,iCityRange
						,iReserveGroupUnit
						,iSettleGroupUnit
					);
					vPossibleCitySites.push_back(pCitySitePlot);
					iSpaceForCity++;
				}
			}
		}
	}
	//iSpaceForCity=1; //TODO: SpyFanatic in the meantime let's limit settler to max 1
	if(GET_PLAYER((PlayerTypes)getOwnerINLINE()).getUnitClassCountPlusMaking(eSettler) >= iSpaceForCity)
	{
		return false;
	}
*/

	pushOrder(ORDER_TRAIN,eUnit,NO_UNITAI,false,false,false);
	if(isOOSLogging())
	{
		oosLog("AIBestProductionValue"
			,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:0,CitySpecialization:%d,NeedSettler:%d,SupportUsed:%d,Limit:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getOwner()
			,getID()
			,eUnit
			,GC.getUnitInfo(eUnit).getDescription()
			,AI_getCitySpecialization()
			,iExistingSettlers
			,GET_PLAYER(getOwner()).getUnitSupportUsed()
			,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
		);
	}
	return true;
}

//TODO Make Function a lot smarter
bool CvCityAI::AI_chooseAnyProject()
{
	int iMaxTurns = 40;

	iMaxTurns *= GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getCreatePercent();
	iMaxTurns /= 100;

	for(int i = 0; i < GC.getNumProjectInfos(); ++i) {
		if(canCreate((ProjectTypes)i)) {
			if(GC.getProjectInfo((ProjectTypes)i).getAIWeight() > -1) {
				if(getProductionTurnsLeft((ProjectTypes)i, 0) < iMaxTurns)	{
					pushOrder(ORDER_CREATE, i, -1, false, false, false);
					return true;
				}
			}
		}
	}
	return false;
}

bool CvCityAI::AI_chooseTrainingBuilding(bool bPatrol)
{
	bool bTraining=false;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	BuildingTypes eBestBuilding=NO_BUILDING;
	int iBestValue=0;
	int iValue;

	//disable for now
	return false;

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("CvCity::AI_chooseTrainingBuilding");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/


	//Doviello need no Training Buildings
	//move into cannotconstruct later
	//if(getCivilizationType()==GC.getInfoTypeForString("CIVILIZATION_DOVIELLO"))
	//	return false;

	//Check if we already have a Training Building
	for(int iI = 0;iI < GC.getNumUnitInfos(); iI++)
	{
		UnitTypes eUnit = (UnitTypes)iI;
		if(canTrain(eUnit, false, false))
		{
			//do not count arcane or siege units here
			if(GC.getUnitInfo(eUnit).getCombat()>0)
			{
				if(GC.getUnitInfo(eUnit).getPrereqBuildingClass() != NO_BUILDINGCLASS)
				{
					BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getUnitInfo(eUnit).getPrereqBuildingClass());

					if(eBuilding!=NO_BUILDING && getNumRealBuilding(eBuilding)>0)
					{
						bTraining=true;
					}
				}
			}
		}
	}
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("CvCity::AI_chooseTrainingBuilding");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

	if(!bTraining)
	{
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("CvCity::AI_chooseTrainingBuilding");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

		for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
		{
			BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);
			iValue = -1;

			if(canConstruct(eBuilding,true))
			{
				for(int iI=0;iI<GC.getNumUnitInfos();iI++)
				{
					UnitTypes eUnit=(UnitTypes)iI;
					if(GC.getUnitInfo((UnitTypes)iI).getPrereqAndTech()==NO_TECH || GET_TEAM(getTeam()).isHasTech((TechTypes)GC.getUnitInfo((UnitTypes)iI).getPrereqAndTech()))
					{
						if(GC.getUnitInfo(eUnit).getPrereqBuildingClass()==iBuildingIndex)
						{
							if(GC.getUnitInfo(eUnit).getCombat()>0)
							{
								iValue=AI_getUnitValue(eUnit);
							}
						}
					}
				}

				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					eBestBuilding=eBuilding;
				}
			}
		}		

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("CvCity::AI_chooseTrainingBuilding");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

		if(eBestBuilding!=NO_BUILDING)
		{
			pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
			if(isOOSLogging())
			{
				oosLog("AIBestProductionValue"
					,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,AI_chooseTrainingBuilding1,Happy:%d,Health:%d"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwner()
					,getID()
					,eBestBuilding
					,GC.getBuildingInfo(eBestBuilding).getDescription()
					,iBestValue
					,AI_getCitySpecialization()
					,happyLevel() - unhappyLevel()
					,goodHealth() - badHealth()
				);
			}
			return true;
		}
	}

	//construct mage guild/siege workshop
	iBestValue=0;
	eBestBuilding=NO_BUILDING;

	for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);
		iValue = -1;

		if(canConstruct(eBuilding,true) && GC.getBuildingClassInfo((BuildingClassTypes)iBuildingIndex).getMaxPlayerInstances()==-1)
		{
			for(int iI=0;iI<GC.getNumUnitInfos();iI++)
			{
				UnitTypes eUnit=(UnitTypes)iI;
				if(GC.getUnitInfo(eUnit).getPrereqAndTech()==NO_TECH || GET_TEAM(getTeam()).isHasTech((TechTypes)GC.getUnitInfo(eUnit).getPrereqAndTech()))
				{
					if(GC.getUnitInfo(eUnit).getPrereqBuildingClass()==iBuildingIndex || GC.getUnitInfo(eUnit).getPrereqBuilding()==eBuilding)
					{
						if(kPlayer.CountBuildingPlusMakinginArea(eBuilding,getArea())<1+kPlayer.getNumCities()/3)
						{
							iValue=AI_getUnitValue(eUnit);
						}
					}
				}
			}

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestBuilding=eBuilding;
			}
		}
	}		

	if(eBestBuilding!=NO_BUILDING)
	{
		pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,AI_chooseTrainingBuilding2,Happy:%d,Health:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestBuilding
				,GC.getBuildingInfo(eBestBuilding).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,happyLevel() - unhappyLevel()
				,goodHealth() - badHealth()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseWorkerUnit()
{
	//Workboat
	if(!waterArea()==NULL)
	{
		if(GET_PLAYER((PlayerTypes)getOwnerINLINE()).AI_totalWaterAreaUnitAIs(waterArea(),UNITAI_WORKER_SEA)==0)
		{
			if(AI_neededSeaWorkers()>0)
			{
				UnitClassTypes eWorkboat=(UnitClassTypes)GC.getUNITCLASS_WORKBOAT();
				if(eWorkboat!=NO_UNITCLASS)
				{
					UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eWorkboat);
					if(eUnit!=NO_UNIT && canTrain(eUnit,true))
					{
						pushOrder(ORDER_TRAIN,eUnit,NO_UNITAI,false,false,false);
						if(isOOSLogging())
						{
							oosLog("AIBestProductionValue"
								,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:0,CitySpecialization:%d,NeedSeaWorker,SupportUsed:%d,Limit:%d"
								,GC.getGameINLINE().getElapsedGameTurns()
								,getOwner()
								,getID()
								,eUnit
								,GC.getUnitInfo(eUnit).getDescription()
								,AI_getCitySpecialization()
								,GET_PLAYER(getOwner()).getUnitSupportUsed()
								,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
							);
						}
						return true;
					}
				}
			}
		}
	}

	UnitClassTypes eWorker=(UnitClassTypes)GC.getUNITCLASS_WORKER();
	if(eWorker!=NO_UNITCLASS)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(eWorker);
		if(eUnit!=NO_UNIT && canTrain(eUnit,true))
		{			
			if(GET_PLAYER(getOwnerINLINE()).isWorkerNeeded(getArea()))
			{
//				if(getAIGroup_Worker()->getNumUnits()==0)
//				{
					pushOrder(ORDER_TRAIN,eUnit,NO_UNITAI,false,false,false);
					if(isOOSLogging())
					{
						oosLog("AIBestProductionValue"
							,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:0,CitySpecialization:%d,NeedWorker,SupportUsed:%d,Limit:%d"
							,GC.getGameINLINE().getElapsedGameTurns()
							,getOwner()
							,getID()
							,eUnit
							,GC.getUnitInfo(eUnit).getDescription()
							,AI_getCitySpecialization()
							,GET_PLAYER(getOwner()).getUnitSupportUsed()
							,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
						);
					}
					return true;
//				}
			}
		}
	}

	return false;
}

bool CvCityAI::AI_chooseProductionDistrict()
{
	//Trying to force a specialization on district for cities as AI seems to build constantly
	//troops, which eventually force them to be behind tech tree and building more troops
	//Check AI_doCitySpecialization

	GET_PLAYER(getOwner()).AI_updateCitySpecializationCount();

	BuildingTypes eDistrictBuilding = NO_BUILDING;
	switch(AI_getCitySpecialization()){
		case CITYSPECIALIZATION_RESEARCH:{
			eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_SAGE_DISTRICT());
			if (eDistrictBuilding != NO_BUILDING && isHasBuildingClass(GC.getBuildingInfo(eDistrictBuilding).getBuildingClassType()))
			{
				eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_SALON());
			}
			break;
		}
		case CITYSPECIALIZATION_ARCANE:{
			eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_SAGE_DISTRICT());
			if (eDistrictBuilding != NO_BUILDING && isHasBuildingClass(GC.getBuildingInfo(eDistrictBuilding).getBuildingClassType()))
			{
				if(GET_PLAYER(getOwner()).isHasTech((TechTypes)GC.getInfoTypeForString("TECH_FISHING_GUILD")))
				{
					eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_SCHOLA_ARCANA());
				}
				else
				{
					eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_SALON());
				}
			}
			break;
		}
		case CITYSPECIALIZATION_MILITARY:{
			eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_NOBLE_DISTRICT());
			if (eDistrictBuilding != NO_BUILDING && isHasBuildingClass(GC.getBuildingInfo(eDistrictBuilding).getBuildingClassType()))
			{
				eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_WARRIOR_DISTRICT());
			}
			break;
		}
		case CITYSPECIALIZATION_FAITH:{
			eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_NOBLE_DISTRICT());
			if (eDistrictBuilding != NO_BUILDING && isHasBuildingClass(GC.getBuildingInfo(eDistrictBuilding).getBuildingClassType()))
			{
				eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_TEMPLE_DISTRICT());
			}
			break;
		}
		case CITYSPECIALIZATION_CULTURE:{
			eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_NOBLE_DISTRICT());
			if (eDistrictBuilding != NO_BUILDING && isHasBuildingClass(GC.getBuildingInfo(eDistrictBuilding).getBuildingClassType()))
			{
				eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_BARD_DISTRICT());
			}
			break;
		}
		case CITYSPECIALIZATION_GOLD:{
			eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_MERCHANT_DISTRICT());
			if (eDistrictBuilding != NO_BUILDING && isHasBuildingClass(GC.getBuildingInfo(eDistrictBuilding).getBuildingClassType()))
			{
				eDistrictBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(GC.getBUILDINGCLASS_FOREIGN_TRADE_DISTRICT());
			}
			//Those 2 second level district may not be really needed...
			//m_iBUILDINGCLASS_LUXURY_DISTRICT = GC.getDefineINT("BUILDINGCLASS_LUXURY_DISTRICT");
			//m_iBUILDINGCLASS_RESOURCE_DISTRICT = GC.getDefineINT("BUILDINGCLASS_RESOURCE_DISTRICT");
			break;
		}
		default:{

		}
	}

	if(eDistrictBuilding != NO_BUILDING && canConstruct(eDistrictBuilding) && getProductionTurnsLeft(eDistrictBuilding,0) < GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent()/3)
	{
		// DEBUG
		if(isOOSLogging())
		{
			oosLog("AISpecialization","Turn:%d,PlayerID:%d,Player:%S,City:%S,X:%d,Y:%d,Count:%d,District:%S,CitySpecialization:%S"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,GC.getCivilizationInfo(GET_PLAYER(getOwner()).getCivilizationType()).getDescription()
				,getName().c_str()//,getNameKey()
				,getX()
				,getY()
				,GET_PLAYER(getOwner()).getNumCities()
				,GC.getBuildingInfo(eDistrictBuilding).getDescription()
				,GC.getCitySpecializationInfo(AI_getCitySpecialization()).getDescription()
			);
		}
		//DEBUG

		pushOrder(ORDER_CONSTRUCT,eDistrictBuilding,-1,false,false,false);
		return true;
	}
	return false;
}
bool CvCityAI::AI_chooseEarlyProject()
{
	int iMaxTurns = 40;

	iMaxTurns *= GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getCreatePercent();
	iMaxTurns /= 100;

	for(int i = 0; i < GC.getNumProjectInfos(); ++i) {
		ProjectTypes eProject = (ProjectTypes)i;
		//Only project that will expire soon
		if(GC.getProjectInfo(eProject).getLastTurnPossibleBuild() > 0){

			int iLastTurns = GC.getProjectInfo(eProject).getLastTurnPossibleBuild();

			iLastTurns *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getBuildPercent();
			iLastTurns /= 100;

			if(canCreate(eProject,true,true)) {
				if(getProductionTurnsLeft(eProject, 0) < iMaxTurns){
					if(GC.getGame().getElapsedGameTurns() + getProductionTurnsLeft(eProject, 0) <= iLastTurns){
						if(isOOSLogging())
						{
							oosLog("AIBestProductionValue"
								,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:0,CitySpecialization:%d,AI_chooseEarlyProject,Happy:%d,Health:%d"
								,GC.getGameINLINE().getElapsedGameTurns()
								,getOwner()
								,getID()
								,eProject
								,GC.getProjectInfo(eProject).getDescription()
								,AI_getCitySpecialization()
								,happyLevel() - unhappyLevel()
								,goodHealth() - badHealth()
							);
						}
						pushOrder(ORDER_CREATE, i, -1, false, false, false);
						return true;
					}
				}
			}
		}
	}
	return false;
}
bool CvCityAI::AI_chooseProductionNew()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());

	if(isHuman())
	{
		return false;
	}

	if(isBarbarian())
	{
		if (AI_chooseProductionBarbarian())
		{
			return true;
		}
	}

/*
	Here AI may be able to build stronger units in:
	AI_chooseEarlyDefenseGroupUnit
		AI_chooseCityInvasionUnit
		AI_chooseCityLairUnit
	AI_chooseWarwizardUnit
	AI_chooseMagicNodeUnit
	AI_chooseCitySiegeUnit
	AI_chooseCounterUnit
	AI_choosePermDefenseReserveUnit
	AI_chooseBestPermDefenseUnit
	AI_choosePermDefenseUnit
	But maybe cannot due to not enough space to produce units...

*/
//SpyFanatic: not needed as of now, AI is now building better units
/*
	if(!kPlayer.canSupportMoreUnits(1))
	{
		int iLoop;
		for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
		{
			if(
				true
				//pAIGroup->getGroupType()==AIGROUP_CITY_INVASION
				//|| pAIGroup->getGroupType()==AIGROUP_DESTROY_LAIR
					//|| pAIGroup->getGroupType()==AIGROUP_COUNTER //Maybe kill used as a counter during invasion is not a great idea...
				//|| pAIGroup->getGroupType()==AIGROUP_RESERVE
					//AIGROUP_CITY_DEFENSE this should not be replaced....
					//AIGROUP_SETTLE maybe not replace also this
					//AIGROUP_CITY_DEFENSE
			)
			{
				if(getArea()==pAIGroup->getMissionArea()) //Maybe move it at CvPlayer level? Now let's only crosscheck group whos area is the same of the city...
				{
					if(true) //pAIGroup->UnitsNeeded()>0
					{
						for (int iI = 1; iI < 4; iI++)
						//for (int iI = 1; iI < 3; iI++) //Let's only replace tier 0,1,2 with 2,3,4
						{
							UnitTypes eUnitFound = NO_UNIT;
							UnitCombatTypes eUnitFoundCombatType = NO_UNITCOMBAT;
							for (CLLNode<IDInfo>* pUnitNode = pAIGroup->headUnitNode(); pUnitNode != NULL; pUnitNode = pAIGroup->nextUnitNode(pUnitNode))
							{
								CvUnit* pLoopUnit = getUnit(pUnitNode->m_data);
								CvUnitInfo& pLoopUnitInfo = GC.getUnitInfo(pLoopUnit->getUnitType());
								if(pLoopUnitInfo.getTier() <= iI && pLoopUnitInfo.getCombat() > 0 && pLoopUnitInfo.getProductionCost() > 0)
								{
									eUnitFound = pLoopUnit->getUnitType();
									eUnitFoundCombatType = pLoopUnit->getUnitCombatType();
								}
							}
							if(eUnitFound != NO_UNIT)
							{
								for(int iJ=0;iJ<GC.getNumUnitClassInfos();iJ++)
								{
									UnitTypes eUnit=(UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iJ);
									if(eUnit != NO_UNIT)
									{
										CvUnitInfo& kUnitInfo = GC.getUnitInfo(eUnit);
										if(
											kUnitInfo.getTier() > iI
											&& kUnitInfo.getProductionCost() > 0
											&& kUnitInfo.getCombat() > 0
											//&& kUnitInfo.getCombatDefense() > 0 Combat defense not needed
											&& eUnitFoundCombatType == kUnitInfo.getUnitCombatType()
										)
										{
											if(canTrain(eUnit,true))
											{
												//TODO: Check if for best unit to build (first does not mean its the best)
												oosLog(
													"AIStack"
													,"Turn:%d,PlayerID:%d,CityID:%d,CanTrain:%S[%d %S],InsteadOf:%S[%d %S],Group:%S,NeedsUnit:%d,X:%d,Y:%d\n"
													,GC.getGameINLINE().getElapsedGameTurns()
													,getOwner()
													,getID()
													,GC.getUnitInfo(eUnit).getDescription()
													,GC.getUnitInfo(eUnit).getTier()
													,GC.getUnitInfo(eUnit).getUnitCombatType()>=0?GC.getUnitCombatInfo((UnitCombatTypes)GC.getUnitInfo(eUnit).getUnitCombatType()).getDescription():L""
													,GC.getUnitInfo(eUnitFound).getDescription()
													,GC.getUnitInfo(eUnitFound).getTier()
													,GC.getUnitInfo(eUnitFound).getUnitCombatType()>=0?GC.getUnitCombatInfo((UnitCombatTypes)GC.getUnitInfo(eUnitFound).getUnitCombatType()).getDescription():L""
													,GC.getAIGroupInfo(pAIGroup->getGroupType()).getDescription()
													,pAIGroup->UnitsNeeded()
													,pAIGroup->getHeadUnit()->plot()->getX_INLINE()
													,pAIGroup->getHeadUnit()->plot()->getY_INLINE()
												);
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
*/
	if(AI_choosePermDefenseUnit())
	{
		return true;
	}
	if(AI_chooseCheapProductionBuilding())
	{
		return true;
	}

	if(kPlayer.AI_getCitySpecializationCount(CITYSPECIALIZATION_GOLD) <= 0 && kPlayer.getCommercePercent(COMMERCE_RESEARCH) < 100)
	//if(kPlayer.getNumCities() < 4 && kPlayer.getCommercePercent(COMMERCE_RESEARCH) < 100)
	{
		//If player does not yet have a merchant district and research below 100%
		if(AI_chooseCheapGoldBuilding())
		{
			return true;
		}
	}

	if(getCulture((PlayerTypes)getOwnerINLINE())==0 && getCommerceRate(COMMERCE_CULTURE)==0)
	{
		if(AI_chooseCheapCultureBuilding())
		{
			return true;
		}
	}

	if(AI_chooseEarlyProject()){
		return true;
	}
	//need to add code to make AI specialize Cities for Military per Area
	if(GET_TEAM(getTeam()).getAtWarCount(true)>0 || 
		getProduction()*(100+getMilitaryProductionModifier())/100>15)
	{

		if(AI_chooseTrainingBuilding(true))
		{
			return true;
		}

		if(AI_choosePermDefenseReserveUnit())
		{
			return true;
		}

		if(AI_chooseCounterUnit())
		{
			return true;
		}
	}

	if(AI_chooseWorkerUnit())
	{
		return true;
	}

	if(!kPlayer.isLackMoneyForExpansion())
	{
		if(AI_chooseSettlerUnit())
		{
			return true;
		}
	}

	if(AI_chooseProductionDistrict()){
		return true;
	}
//		if ((!kPlayer.isConquestMode()) && kPlayer.getNumCities()>2)
	//SpyFanatic: add 2 to this till cities are <= 3
	//if(kPlayer.getUnitSupportUsed()<6)
	if(kPlayer.getUnitSupportUsed()<6 || (kPlayer.getUnitSupportUsed()<8 && kPlayer.getNumCities() < 3))
	{
		if(AI_chooseTrainingBuilding())
		{
			return true;
		}

		if(AI_chooseCitySiegeUnit())
		{
			return true;
		}

		if(AI_chooseEarlyDefenseGroupUnit())
		{
			return true;
		}
	}

	if(findBaseYieldRateRank(YIELD_PRODUCTION)<4)
	{
		if(AI_chooseHero())
		{
			return true;
		}
	}

	//function for Religion Missionaries
	//function for Rituals
	//function for Naval Units
	//function for Mages

	//TODO: Only if AI wants to build a Unit
	if(2>1)
	{
		if(AI_chooseBuildingHappiness())
		{
			return true;
		}
		if(AI_chooseBuildingHealth())
		{
			return true;
		}
		if(AI_chooseTrainingBuilding())
		{
			return true;
		}

		//SpyFanatic: moved down the list to prioritize unit for city invasion over lair....

		//Need a Unit to take out a Lair?
		if(AI_chooseCityLairUnit())
		{
			return true;
		}

		if(AI_chooseCitySiegeUnit())
		{
			return true;
		}

		if(AI_chooseCityInvasionUnit())
		{
			return true;
		}

		if(AI_chooseNavalInvasionUnit())
		{
			return true;
		}

		if(AI_chooseNavalSettleUnit())
		{
			return true;
		}

		if(AI_chooseNavalCounterUnit())
		{
			return true;
		}

		if(AI_chooseMagicNodeUnit())
		{
			return true;
		}

		if(AI_chooseNavalExplorerUnit())
		{
			return true;
		}

		if(AI_chooseWarwizardUnit())
		{
			return true;
		}
		/*
		//Need a Unit to take out a Lair?
		if(AI_chooseCityLairUnit())
		{
			return true;
		}*/
	}

	int iValue=0;
	int iBestBuildingValue=0;
	BuildingTypes eBestBuilding=NO_BUILDING;
	int iMaxTurnTime=GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent()/3;

	for (int iBuildingIndex = 0; iBuildingIndex < GC.getNumBuildingClassInfos(); iBuildingIndex++)
	{
		BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iBuildingIndex);
		iValue = 0;

		if(canConstruct(eBuilding))
		{
			if(getProductionTurnsLeft(eBuilding,0)<iMaxTurnTime)
			{
				iValue=AI_getBuildingValue(eBuilding);
				
				if(iValue>iBestBuildingValue)
				{
					eBestBuilding = eBuilding;
					iBestBuildingValue=iValue;
				}
			}
		}
	}

	//DEBUG
	if(eBestBuilding != NO_BUILDING){
		if(isOOSLogging())	
		{
			oosLog(
				"AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestBuildingValue:%d,CitySpecialization:%d,Building,Happy:%d,Health:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestBuilding
				,GC.getBuildingInfo(eBestBuilding).getDescription()
				,iBestBuildingValue
				,AI_getCitySpecialization()
				,happyLevel() - unhappyLevel()
				,goodHealth() - badHealth()
			);
		}
	}
	//DEBUG

	if(iBestBuildingValue>AI_getValueAnyUnit())
	{
		//Process for specialized Cities?
		if(iBestBuildingValue < 1000)
		{
			for(int iI=0;iI<GC.getNumProcessInfos();iI++)
			{
				if(AI_getCitySpecialization()==CITYSPECIALIZATION_GOLD)
				{
					if(GC.getProcessInfo((ProcessTypes)iI).getProductionToCommerceModifier(COMMERCE_GOLD)>0)
					{
						pushOrder(ORDER_MAINTAIN,iI,-1,false,false,false);
						return true;
					}
				}

				if(AI_getCitySpecialization() == CITYSPECIALIZATION_RESEARCH
					|| AI_getCitySpecialization() == CITYSPECIALIZATION_ARCANE)
				{
					if(GC.getProcessInfo((ProcessTypes)iI).getProductionToCommerceModifier(COMMERCE_RESEARCH) > 0)
					{
						pushOrder(ORDER_MAINTAIN,iI,-1,false,false,false);
						return true;
					}
				}
			}
		}
		if(eBestBuilding!=NO_BUILDING)
		{
			pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
			return true;
		}
	}

	if(AI_chooseTrainingBuilding())
	{
		return true;
	}

	int iBestUnitValue=0;
	UnitTypes eBestUnit=NO_UNIT;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = 0;

		if(canTrain(eUnit,true))
		{
			iValue=AI_getUnitValue(eUnit);

			iValue*=100;
			iValue/=std::max(1,getProductionTurnsLeft(eUnit,0));

			if(iValue>iBestUnitValue)
			{
				eBestUnit = eUnit;
				iBestUnitValue=iValue;
			}
		}
	}

	if(eBestUnit != NO_UNIT) {
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		return true;
	}

	if(AI_chooseAnyProject()) {
		return true;
	}

	//just pick any process if we cannot build anything useful
	if(iBestBuildingValue > 0)
		if(eBestBuilding!=NO_BUILDING) {
			pushOrder(ORDER_CONSTRUCT,eBestBuilding,-1,false,false,false);
			return true;
		}

	for(int iI = 0; iI < GC.getNumProcessInfos(); iI++)
	{
		if(AI_getCitySpecialization() == CITYSPECIALIZATION_RESEARCH ||
			AI_getCitySpecialization() == CITYSPECIALIZATION_ARCANE) {
			if(GC.getProcessInfo((ProcessTypes)iI).getProductionToCommerceModifier(COMMERCE_RESEARCH) > 0) {
				pushOrder(ORDER_MAINTAIN, iI, -1, false, false,  false);
				return true;
			}
		}
		else
			if(GC.getProcessInfo((ProcessTypes)iI).getProductionToCommerceModifier(COMMERCE_GOLD) > 0) {
				pushOrder(ORDER_MAINTAIN, iI, -1, false, false,  false);
				return true;
			}
	}

	return false;
}

bool CvCityAI::AI_chooseNavalInvasionUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_INVASION && pAIGroup->getMissionStatus()!=STATUS_NAVAL_IN_TRANSIT)
		{
			if(plot()->isAdjacentToArea(pAIGroup->getMissionArea()))
			{
				//TODO add Siege for Naval Invasions
				if((pAIGroup->UnitsNeeded(UNITCATEGORY_COMBAT)>0)
					||(pAIGroup->UnitsNeeded(UNITCATEGORY_COMBAT_NAVAL)>0)
					||(pAIGroup->UnitsNeeded(UNITCATEGORY_TRANSPORT_NAVAL)>0))
				{
					pGroup=pAIGroup;
					break;
				}
			}
		}
	}

	if(pGroup==NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			iValue=AI_ValueNavalInvasionUnit(pGroup, eUnit);

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestUnit=eUnit;
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseNavalInvasionUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}
bool CvCityAI::AI_chooseNavalSettleUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	//TODO: transform this into a local condition
	UnitClassTypes eSettler = (UnitClassTypes)GC.getUNITCLASS_SETTLER();
	if(GET_PLAYER((PlayerTypes)getOwnerINLINE()).getUnitClassCountPlusMaking(eSettler) >= std::max(2, 1 + GET_PLAYER((PlayerTypes)getOwnerINLINE()).getNumCities() / 5)) {
		return false;
	}

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_SETTLE && pAIGroup->getMissionStatus()!=STATUS_NAVAL_IN_TRANSIT)
		{
			if(plot()->isAdjacentToArea(pAIGroup->getMissionArea()))
			{
				if((pAIGroup->UnitsNeeded(UNITCATEGORY_COMBAT)>0)
					||(pAIGroup->UnitsNeeded(UNITCATEGORY_SETTLE)>0)
					||(pAIGroup->UnitsNeeded(UNITCATEGORY_TRANSPORT_NAVAL)>0))
				{
					pGroup=pAIGroup;
					break;
				}
			}
		}
	}

	if(pGroup == NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			iValue = AI_ValueNavalSettleUnit(pGroup, eUnit);

			if(iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestUnit = eUnit;
			}
		}
	}		

	if(eBestUnit != NO_UNIT) {
		pushOrder(ORDER_TRAIN, eBestUnit, NO_UNITAI, false, false, false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseNavalSettleUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}
bool CvCityAI::AI_chooseNavalExplorerUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_EXPLORER)
		{
			if(plot()->isAdjacentToArea(pAIGroup->getMissionArea()))
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					pGroup=pAIGroup;
					break;
				}
			}
		}
	}

	if(pGroup==NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			iValue=AI_ValueNavalExplorerUnit(pGroup, eUnit);

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestUnit=eUnit;
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseNavalExplorerUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}

bool CvCityAI::AI_chooseNavalCounterUnit()
{
	UnitTypes eBestUnit=NO_UNIT;
	int iBestValue=0;
	int iValue;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvAIGroup* pGroup=NULL;
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_COUNTER)
		{
			if(plot()->isAdjacentToArea(pAIGroup->getMissionArea()))
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					pGroup=pAIGroup;
					break;
				}
			}
		}
	}

	if(pGroup==NULL)
		return false;

	for (int iUnitIndex = 0; iUnitIndex < GC.getNumUnitClassInfos(); iUnitIndex++)
	{
		UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iUnitIndex);
		iValue = -1;

		if(canTrain(eUnit))
		{
			iValue=AI_ValueNavalCounterUnit(pGroup, eUnit);

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				eBestUnit=eUnit;
			}
		}
	}		

	if(eBestUnit!=NO_UNIT)
	{
		pushOrder(ORDER_TRAIN,eBestUnit,NO_UNITAI,false,false,false);
		if(isOOSLogging())
		{
			oosLog("AIBestProductionValue"
				,"Turn:%d,PlayerID:%d,CityID:%d,ItemID:%d,ItemName:%S,AIBestUnitValue:%d,CitySpecialization:%d,AI_chooseNavalCounterUnit,SupportUsed:%d,Limit:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,eBestUnit
				,GC.getUnitInfo(eBestUnit).getDescription()
				,iBestValue
				,AI_getCitySpecialization()
				,GET_PLAYER(getOwner()).getUnitSupportUsed()
				,GET_PLAYER(getOwner()).getUnitSupportLimitTotal()
			);
		}
		return true;
	}

	return false;
}


int CvCityAI::AI_ValueNavalCounterUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);
	if(isWorldUnitClass((UnitClassTypes)kUnit.getUnitClassType()))
	{
		return -MAX_INT;
	}

	if(kUnit.getMoves()<1)
	{
		return -MAX_INT;
	}

	int iValue;
	iValue=10*AI_ValueUnitBaseCombat(eUnit,false,true);

	iValue*=(60+40*kUnit.getMoves());
	iValue/=100;

	if(pAIGroup->getMissionTargetUnit()!=NULL && pAIGroup->getMissionTargetUnit()->isInvisible(getTeam(),false))
	{
		InvisibleTypes iInvisible=pAIGroup->getMissionTargetUnit()->getInvisibleType();
		if(!pAIGroup->isSeeInvisible())
		{
			bool bValid=false;
			for(int iI=0;iI<kUnit.getNumSeeInvisibleTypes();iI++)
			{
				if(kUnit.getSeeInvisibleType(iI)==iInvisible)
				{
					bValid=true;
					break;
				}
			}
			if(bValid)
			{
				iValue*=100;
			}
		}
	}

	return iValue;
}

int CvCityAI::AI_ValueNavalSettleUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	int iValue=0;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);
	//do we Already have/build a settler?
	if(pAIGroup->UnitsNeeded(UNITCATEGORY_SETTLE)>kPlayer.CountUnitCategoryinMaking(UNITCATEGORY_SETTLE,getArea()))
	{
		if(kPlayer.AI_isUnitCategory(eUnit, UNITCATEGORY_SETTLE))
		{
			return MAX_INT;
		}
	}

	//Do we need Transport Units?
	if(pAIGroup->UnitsNeeded(UNITCATEGORY_TRANSPORT_NAVAL)>kPlayer.CountUnitCategoryinMaking(UNITCATEGORY_TRANSPORT_NAVAL,getArea()))
	{
		if(kPlayer.AI_isUnitCategory(eUnit, UNITCATEGORY_TRANSPORT_NAVAL))
		{
			iValue=200*AI_ValueUnitBaseCombat(eUnit,false,true);

			iValue*=(60+40*kUnit.getMoves());
			iValue*=(60+80*kUnit.getCargoSpace());
			iValue/=100;
		}
	}

	//Do we need Naval Combat Units?
	if(iValue==0)
	{
		if(pAIGroup->UnitsNeeded(UNITCATEGORY_COMBAT_NAVAL)>kPlayer.CountUnitCategoryinMaking(UNITCATEGORY_COMBAT_NAVAL,getArea()))
		{
			if(kPlayer.AI_isUnitCategory(eUnit, UNITCATEGORY_COMBAT_NAVAL))
			{
				iValue=50*AI_ValueUnitBaseCombat(eUnit,false,true);

				iValue*=(60+40*kUnit.getMoves());
				iValue/=100;
			}
		}
	}

	//Do we need Combat Units?
	if(iValue==0)
	{
		if(pAIGroup->UnitsNeeded(UNITCATEGORY_COMBAT)>kPlayer.CountUnitCategoryinMaking(UNITCATEGORY_COMBAT,getArea()))
		{
			if(kPlayer.AI_isUnitCategory(eUnit, UNITCATEGORY_COMBAT))
			{
				iValue=5*AI_ValueUnitBaseCombat(eUnit,false,false);

				iValue*=(60+40*kUnit.getMoves());
				iValue/=100;
			}
		}
	}

	return 0;
}

int CvCityAI::AI_ValueNavalInvasionUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	int iValue=0;
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);

	//Do we need Transport Units?
	if(pAIGroup->UnitsNeeded(UNITCATEGORY_TRANSPORT_NAVAL)>kPlayer.CountUnitCategoryinMaking(UNITCATEGORY_TRANSPORT_NAVAL,getArea()))
	{
		if(kPlayer.AI_isUnitCategory(eUnit, UNITCATEGORY_TRANSPORT_NAVAL))
		{
			iValue=200*AI_ValueUnitBaseCombat(eUnit,false,true);

			iValue*=(60+40*kUnit.getMoves());
			iValue*=(60+80*kUnit.getCargoSpace());
			iValue/=100;
		}
	}

	//Do we need Naval Combat Units?
	if(iValue==0)
	{
		if(pAIGroup->UnitsNeeded(UNITCATEGORY_COMBAT_NAVAL)>kPlayer.CountUnitCategoryinMaking(UNITCATEGORY_COMBAT_NAVAL,getArea()))
		{
			if(kPlayer.AI_isUnitCategory(eUnit, UNITCATEGORY_COMBAT_NAVAL))
			{
				iValue=50*AI_ValueUnitBaseCombat(eUnit,false,true);

				iValue*=(60+40*kUnit.getMoves());
				iValue/=100;
			}
		}
	}

	//Do we need Combat Units?
	if(iValue==0)
	{
		if(pAIGroup->UnitsNeeded(UNITCATEGORY_COMBAT)>kPlayer.CountUnitCategoryinMaking(UNITCATEGORY_COMBAT,getArea()))
		{
			if(kPlayer.AI_isUnitCategory(eUnit, UNITCATEGORY_COMBAT))
			{
				iValue=5*AI_ValueUnitBaseCombat(eUnit,false,false);

				iValue*=(60+40*kUnit.getMoves());
				iValue/=100;
			}
		}
	}

	return 0;
}

int CvCityAI::AI_ValueNavalExplorerUnit(CvAIGroup* pAIGroup, UnitTypes eUnit)
{
	CvUnitInfo &kUnit=GC.getUnitInfo(eUnit);
	if(isWorldUnitClass((UnitClassTypes)kUnit.getUnitClassType()))
	{
		return -MAX_INT;
	}

	if(kUnit.getMoves()<1)
	{
		return -MAX_INT;
	}

	int iValue;
	iValue=10*AI_ValueUnitBaseCombat(eUnit,false,true);

	iValue*=(100*kUnit.getMoves());
	iValue/=100;

	return iValue;
}
