// AI_HERO.cpp

#include "CvGameCoreDLL.h"
#include "CvUnitAI.h"
#include "CvMap.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvGameCoreUtils.h"
#include "CvRandom.h"
#include "CyUnit.h"
#include "CyArgsList.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "FAStarNode.h"

// interface uses
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"

//selects Units that have the Hero AI
void CvPlayerAI::AI_updateHeroUnits()
{
	if(getNumUnits()<10 || isBarbarian())
	{
		return;
	}
	//TODO::remove Heroes

	//TODO::count Heroes
	int iExistingHeroes=AI_totalUnitAIs(UNITAI_HERO);
	int iHeroTarget=std::max(1,getUnitSupportUsed()/10);
	//TODO::add more Heroes if useful
	while(iExistingHeroes<iHeroTarget)
	{
		if(!AI_chooseAnotherHeroUnit())
		{
			break;
		}
		iExistingHeroes++;
	}
}

//returns true if we found another Unit we can use as a Hero
bool CvPlayerAI::AI_chooseAnotherHeroUnit()
{
	int iLoop;
	int iBestValue=0;
	CvUnit* pBestUnit=NULL;

	for(CvUnit* pLoopUnit=firstUnit(&iLoop);pLoopUnit!=NULL;pLoopUnit=nextUnit(&iLoop))
	{
		if(pLoopUnit->canAttack())
		{
			if(pLoopUnit->isGarrision())
			{
				continue;
			}

			if(pLoopUnit->getLevel() < 3)
				continue;

			if((pLoopUnit->AI_getUnitAIType() == UNITAI_HERO)
				|| (pLoopUnit->AI_getUnitAIType() == UNITAI_EXPLORE))
			{
				continue;
			}

			if(AI_isUnitCategory(pLoopUnit->getUnitType(), UNITCATEGORY_SIEGE)
				|| AI_isUnitCategory(pLoopUnit->getUnitType(), UNITCATEGORY_WARWIZARD))
			{
				continue;
			}

			if(pLoopUnit->getPower() > iBestValue)
			{
				iBestValue = pLoopUnit->getPower();
				pBestUnit = pLoopUnit;
			}
		}
	}

	if(pBestUnit!=NULL)
	{
		pBestUnit->AI_setUnitAIType(UNITAI_HERO);
		if(isOOSLogging())
		{
			oosLog("AI_Heroes"
				,"Turn:%d,PlayerID:%d,Player:%S,New Hero Unit::%S"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getID()
				,getName()
				,GC.getUnitInfo(pBestUnit->getUnitType()).getDescription()
			);
		}

		return true;
	}

	return false;
}
