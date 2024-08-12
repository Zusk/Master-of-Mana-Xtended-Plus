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

int CvCityAI::AI_specialistValueNew(SpecialistTypes eSpecialist)
{
	PROFILE_FUNC();

	int iValue=0;
	iValue+=AI_specialistGrowthValue(eSpecialist);
	iValue+=AI_specialistProductionValue(eSpecialist);
	iValue+=AI_specialistCommerceValue(eSpecialist);
	iValue+=AI_specialistGreatPersonValue(eSpecialist);
	iValue+=AI_specialistSpecializationValue(eSpecialist);
	iValue+=AI_specialistMiscValue(eSpecialist);
	if(AI_isEmphasizeGreatPeople())
	{
		iValue*=2;
	}
	return iValue;
}

int CvCityAI::AI_specialistGrowthValue(SpecialistTypes eSpecialist)
{
	CvSpecialistInfo& kSpecialist=GC.getSpecialistInfo(eSpecialist);
	int iValue=0;

	if(GET_PLAYER(getOwnerINLINE()).isIgnoreFood())
	{
		return 0;
	}

	if(AI_avoidGrowth())
	{
		return 0;
	}

	iValue=kSpecialist.getYieldChange(YIELD_FOOD);
	iValue+=GET_PLAYER(getOwnerINLINE()).getSpecialistExtraYield(eSpecialist,YIELD_FOOD);
	iValue*=100;
	iValue*=100+getYieldRateModifier(YIELD_FOOD);
	iValue/=100;

	return AI_cityValue_Food(iValue);
}

int CvCityAI::AI_specialistProductionValue(SpecialistTypes eSpecialist)
{
	CvSpecialistInfo& kSpecialist=GC.getSpecialistInfo(eSpecialist);
	int iValue=0;

	iValue=kSpecialist.getYieldChange(YIELD_PRODUCTION);
	iValue+=GET_PLAYER(getOwnerINLINE()).getSpecialistExtraYield(eSpecialist,YIELD_PRODUCTION);
	iValue*=200;
	iValue*=100+getYieldRateModifier(YIELD_PRODUCTION);
	iValue/=100;

	return iValue;
}

int CvCityAI::AI_specialistCommerceValue(SpecialistTypes eSpecialist)
{
	CvPlayerAI& kPlayer=GET_PLAYER(getOwnerINLINE());
	CvSpecialistInfo& kSpecialist=GC.getSpecialistInfo(eSpecialist);
	int iValue=0;

	for(int iI=0;iI<GC.getNUM_COMMERCE_TYPES();iI++)
	{
		int iTempValue = kSpecialist.getCommerceChange((CommerceTypes)iI);
		iTempValue += kPlayer.getSpecialistExtraCommerce((CommerceTypes)iI);
		iTempValue += kPlayer.getSpecialistTypeExtraCommerce(eSpecialist,(CommerceTypes)iI);

		if(iI == COMMERCE_CULTURE) {
			iTempValue *= 25;
		}
		else {
			iTempValue *= 100;	//at the moment Rate any Commerce equal
		}
		
		iValue += AI_commerceValue((CommerceTypes)iI,iTempValue);
	}

	return iValue;
}

int CvCityAI::AI_specialistGreatPersonValue(SpecialistTypes eSpecialist)
{
	CvSpecialistInfo& kSpecialist=GC.getSpecialistInfo(eSpecialist);
	int iValue=0;

	if(kSpecialist.getGreatPeopleUnitClass()==NO_UNITCLASS)
	{
		return 0;
	}
	iValue=kSpecialist.getGreatPeopleRateChange();
	iValue*=10;
	iValue*=100+getGreatPeopleUnitProgress((UnitTypes)GC.getUnitClassInfo((UnitClassTypes)kSpecialist.getGreatPeopleUnitClass()).getDefaultUnitIndex())*10+getGreatPeopleRateModifier();
	iValue/=100;

	iValue=std::min(iValue,300);

	return iValue;
}

int CvCityAI::AI_specialistSpecializationValue(SpecialistTypes eSpecialist)
{
	CvSpecialistInfo& kSpecialist=GC.getSpecialistInfo(eSpecialist);
	int iValue = 0;

	//disable CIty Specialization
	//return iValue;
	//disable CIty Specialization

	if(eSpecialist==GC.getSPECIALIST_MERCHANT())
	{
		if(getCitySpecializationLevel(CITYSPECIALIZATION_GOLD)*10<getCommerceRate(COMMERCE_GOLD))
		{
			return 200;
		}
	}

	if(eSpecialist == GC.getSPECIALIST_SCIENTIST())
	{
		if(getCommerceRate(COMMERCE_RESEARCH) || getCommerceRate(COMMERCE_ARCANE) > 30) {
			if(getCitySpecializationLevel(CITYSPECIALIZATION_RESEARCH)*10<getCommerceRate(COMMERCE_RESEARCH))
			{
				return 200;
			}
			if(getCitySpecializationLevel(CITYSPECIALIZATION_ARCANE)*10 < getCommerceRate(COMMERCE_ARCANE))
			{
				return 200;
			}
		}
	}

	if(eSpecialist == GC.getSPECIALIST_ARTIST())
	{
		if(getCitySpecializationLevel(CITYSPECIALIZATION_CULTURE)*10<getCommerceRate(COMMERCE_CULTURE))
		{
			return 0;
		}
	}

	if(eSpecialist == GC.getSPECIALIST_PRIEST())
	{
		if(getCitySpecializationLevel(CITYSPECIALIZATION_FAITH)*10<getCommerceRate(COMMERCE_FAITH))
		{
			return 200;
		}
	}

	if(eSpecialist == GC.getSPECIALIST_ENGINEER()) {
		if(getYieldRate(YIELD_METAL) > 20
		|| getYieldRate(YIELD_LEATHER) > 20
		|| getYieldRate(YIELD_STONE) > 20
		|| getYieldRate(YIELD_LUMBER) > 20) {
			if(getCitySpecializationLevel(CITYSPECIALIZATION_METAL)*10 < getYieldRate(YIELD_METAL)) {
				return 200;
			}

			if(getCitySpecializationLevel(CITYSPECIALIZATION_LEATHER)*10 < getYieldRate(YIELD_LEATHER))  {
				return 200;
			}

			if(getCitySpecializationLevel(CITYSPECIALIZATION_STONE)*10 < getYieldRate(YIELD_STONE))  {
				return 200;
			}

			if(getCitySpecializationLevel(CITYSPECIALIZATION_LUMBER)*10 < getYieldRate(YIELD_LUMBER)) {
				return 200;
			}
		}
	}

	return iValue;
}

int CvCityAI::AI_specialistMiscValue(SpecialistTypes eSpecialist)
{
	int iValue=0;

	return iValue;
}

// PlotValue
// Should scale similar to SpecialistValue
int CvCityAI::AI_plotValue_Food(CvPlot* pPlot)
{
	int iValue=0;
	iValue+=pPlot->getYield(YIELD_FOOD);

	iValue*=100;
	iValue*=100+this->getYieldRateModifier(YIELD_FOOD);
	iValue/=100;

	return AI_cityValue_Food(iValue);
}

int CvCityAI::AI_cityValue_Food(int iFoodAmount)
{
	bool bAvoidGrowth=AI_avoidGrowth();
	bool bCanGrow=(happyLevel()>unhappyLevel() && healthRate()==0);
	int iPopulation=getPopulation();
	int iFoodPerPopulationNeeded = GC.getFOOD_CONSUMPTION_PER_POPULATION() + GET_PLAYER(getOwnerINLINE()).getFoodPerPopulationModifier();  //adjust for Sacrifice the Weak/Slavery
	int iFoodDifference=foodDifference();
	int iFoodTargetForAlreadyDistributedPopulation=-foodConsumption()+iFoodPerPopulationNeeded*(iPopulation-extraPopulation());
	if(bCanGrow)
		iFoodTargetForAlreadyDistributedPopulation+=10;

	//we REALLY need Food or else we starve...
	if(iFoodDifference<iFoodTargetForAlreadyDistributedPopulation 
		|| (getFood()<10 && iFoodDifference<0))
	{
		return iFoodAmount*10;
	}
	else if(AI_isEmphasizeYield(YIELD_FOOD) || isFoodProduction())
	{
		return iFoodAmount*4;
	}
	else if(!bAvoidGrowth)
	{
		return (iFoodAmount*5)/2;
	}
	else if(foodDifference()>0)
	{
		return 0;
	}
	else if(foodDifference()>-(iFoodPerPopulationNeeded*extraPopulation())/100)
	{
		return iFoodAmount/4;
	}

	return (iFoodAmount*3)/2;
}

int CvCityAI::AI_plotValue_Production(CvPlot* pPlot)
{
	int iValue = 0;
	iValue += pPlot->getYield(YIELD_PRODUCTION);

	iValue *= 200;
	iValue *= 100+getYieldRateModifier(YIELD_PRODUCTION);
	iValue /= 100;

	if(AI_getCitySpecialization()==CITYSPECIALIZATION_MILITARY)
		iValue *= 2;

	if(AI_isEmphasizeYield(YIELD_PRODUCTION))
	{
		iValue *= 3;
	}

	return iValue;
}

int CvCityAI::AI_plotValue_Commerce(CvPlot* pPlot)
{
	int iCommerceAmount=pPlot->getYield(YIELD_COMMERCE);
	int iValue=0;

	iCommerceAmount*=100;
	iCommerceAmount*=100+getYieldRateModifier(YIELD_COMMERCE);
	iCommerceAmount/=100;

	for(int iI=0;iI<GC.getNUM_COMMERCE_TYPES();iI++)
	{
		iValue+=AI_commerceValue((CommerceTypes)iI,getCommerceFromPercent((CommerceTypes)iI,iCommerceAmount));
	}

	if(AI_isEmphasizeYield(YIELD_COMMERCE))
	{
		iValue*=3;
	}

	return iValue;
}

int CvCityAI::AI_plotValue_Misc(CvPlot* pPlot)
{
	int iValue = 0;

	if(pPlot->getImprovementType() != NO_IMPROVEMENT)
	{
		if(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementUpgrade() != NO_IMPROVEMENT)
		{
			iValue += 400;
		}
	}

	for(int i = YIELD_COMMERCE + 1; i < NUM_YIELD_TYPES; ++i) {
		iValue += pPlot->getYield((YieldTypes)i) * 100;
	}

	return iValue;
}


int CvCityAI::AI_plotValueNew(CvPlot* pPlot)
{
	int iValue=0;
	iValue+=AI_plotValue_Food(pPlot);
	iValue+=AI_plotValue_Production(pPlot);
	iValue+=AI_plotValue_Commerce(pPlot);
	iValue+=AI_plotValue_Misc(pPlot);

	return iValue;
}

//iValue is Commerce Times 100
int CvCityAI::AI_commerceValue(CommerceTypes eCommerce, int iValue)
{
	int iTempValue=iValue;
	iTempValue*=100+getCommerceRateModifier(eCommerce);
	iTempValue/=100;

	if(eCommerce==COMMERCE_GOLD)
	{
		if(GET_PLAYER(getOwnerINLINE()).getStrikeTurns()>0)
		{
			iTempValue*=5;
		}
	}

	if(AI_getCitySpecialization()==CITYSPECIALIZATION_GOLD && eCommerce==COMMERCE_GOLD)
	{
		iTempValue*=2;
	}

	if(AI_getCitySpecialization()==CITYSPECIALIZATION_RESEARCH && eCommerce==COMMERCE_RESEARCH)
	{
		iTempValue*=2;
	}

	if(AI_getCitySpecialization()==CITYSPECIALIZATION_CULTURE && eCommerce==COMMERCE_CULTURE)
	{
		iTempValue*=2;
	}

	return iTempValue;
}
