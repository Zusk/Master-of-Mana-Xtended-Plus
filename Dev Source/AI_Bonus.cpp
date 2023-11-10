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
int CvPlayerAI::AI_bonusVal_new(BonusTypes eBonus, int iChange) const
{
	int iValue=0;
	int iBonusCount = getNumAvailableBonuses(eBonus);

	iValue+=AI_bonusHealthValue(eBonus,iChange,iBonusCount);
	iValue+=AI_bonusHappinessValue(eBonus,iChange,iBonusCount);
	iValue+=AI_bonusYieldCommerceValue(eBonus,iChange,iBonusCount);
//	iValue+=AI_bonusAlignmentValue(eBonus);
//	iValue+=AI_bonusMiscValue(eBonus);
	return iValue;
}

int CvPlayerAI::AI_bonusHealthValue(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

	//we had zero before or we have zero after
	if(iChange>0 && iBonusCount==0 || iBonusCount+iChange<=0 && iBonusCount>0)
	{
		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			int iActualHealthChangeInCity=pLoopCity->getBonusHealth(eBonus);

			iActualHealthChangeInCity=std::min(iActualHealthChangeInCity,pLoopCity->goodHealth()+3-pLoopCity->badHealth());
			iActualHealthChangeInCity=std::max(0,iActualHealthChangeInCity);
			iValue+=150*iActualHealthChangeInCity;
		}
	}

	return iValue;
}

int CvPlayerAI::AI_bonusHappinessValue(BonusTypes eBonus, int iChange, int iBonusCount) const
{
	int iValue=0;
	int iLoop;

	//we had zero before or we have zero after
	if(iChange>0 && iBonusCount==0 || iBonusCount+iChange<=0 && iBonusCount>0)
	{
		for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
		{
			int iActualHappinessChangeInCity=pLoopCity->getBonusHappiness(eBonus);

			iActualHappinessChangeInCity=std::min(iActualHappinessChangeInCity,pLoopCity->happyLevel()+3-pLoopCity->unhappyLevel());
			iActualHappinessChangeInCity=std::max(0,iActualHappinessChangeInCity);
			iValue+=150*iActualHappinessChangeInCity;
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

	for (CvCity* pLoopCity = firstCity(&iLoop); pLoopCity != NULL; pLoopCity = nextCity(&iLoop))
	{
		for(int iI=0;iI<GC.getNumBuildingInfos();iI++)
		{
			if(pLoopCity->getNumBuilding((BuildingTypes)iI)>0)
			{
				if(GC.getBuildingInfo((BuildingTypes)iI).getBonusYieldType()!=NO_YIELD && GC.getBuildingInfo((BuildingTypes)iI).isBonusLinked(eBonus))
				{
					iValue+=GC.getBuildingInfo((BuildingTypes)iI).getPerBonusValue()* iChange;
					if(bFirst)
					{
						iValue+=(GC.getBuildingInfo((BuildingTypes)iI).getInitialValue()* iChange *3)/2;
					}
				}

				else if(GC.getBuildingInfo((BuildingTypes)iI).getBonusCommerceType()!=NO_COMMERCE && GC.getBuildingInfo((BuildingTypes)iI).isBonusLinked(eBonus))
				{
					iValue+=GC.getBuildingInfo((BuildingTypes)iI).getPerBonusValue()* iChange;
					if(bFirst)
					{
						iValue+=GC.getBuildingInfo((BuildingTypes)iI).getInitialValue()* iChange;
					}
				}

			}
		}
	}

	return iValue;
}

