#include "CvGameCoreDLL.h"
#include "CvAIGroup.h"

void CvAIGroup::update_Settle()
{
	CvUnitAI* pSettler = static_cast<CvUnitAI*>(getHeadUnit());
	CvPlayerAI &kPlayer = GET_PLAYER(getOwnerINLINE());
	bool bDebug = true;
	TCHAR szFile[1024];
	sprintf(szFile, "AI_SettleGroup_%d_%d.log",getOwnerINLINE(),getID());

	if(pSettler == NULL || pSettler->AI_getUnitAIType() != UNITAI_SETTLE) {
		setDeathDelayed(true);
		return;
	}

	//NAVAL SETTLE
	if(getNumUnits() == 1 && getMissionPlot() == NULL)  {
		CvAIGroup* pNaval = getNavalSettleGroupNeedSettler(pSettler->getArea());
		if(pNaval != NULL)  {
			pSettler->setAIGroup(pNaval);
			setDeathDelayed(true);
			return;
		}
	}

	if(bDebug) {
		TCHAR szOut[1024];
		sprintf(szOut, "CvAISettle %d, %d::update -- %S -- %d\n", getOwnerINLINE(), getID(), GC.getAIGroupInfo(getGroupType()).getDescription(), GC.getGame().getElapsedGameTurns());
		gDLL->logMsg(szFile, szOut, false, false);
		sprintf(szOut, "Num Units -- %d / %d\n", getNumUnits(), UnitsNeeded());
		gDLL->logMsg(szFile,szOut, false, false);		
		if(getMissionPlot() != NULL) {
			sprintf(szOut, "Mission Coords -- x: %d, y: %d,  foundvalue: %d\n", getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), getMissionPlot()->getFoundValue(getOwnerINLINE()));
			gDLL->logMsg(szFile,szOut, false, false);		
		}
	}

	//logic for gamestart
	if (GET_PLAYER(getOwnerINLINE()).getNumCities() == 0)
	{
		if(pSettler->getOriginPlot() == NULL)		{
			pSettler->setOriginPlot(pSettler->plot());
		}

		GET_PLAYER(getOwnerINLINE()).AI_updateFoundValues(false);

        CvPlot* pLoopPlot;
        CvPlot* pBestPlot;
        int iSearchRange;
        int iPathTurns;
        int iValue;
        int iBestValue;
        int iDX, iDY;

        iSearchRange = GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getAISettlerMove();
        int iRange = 6;


        iBestValue = 0;
        pBestPlot = NULL;

        for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
        {
            for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
            {
                pLoopPlot	= plotXY(pSettler->getOriginPlot()->getX_INLINE(), pSettler->getOriginPlot()->getY_INLINE(), iDX, iDY);

                if (pLoopPlot != NULL)
                {
                    if (pLoopPlot->getArea()==pSettler->getArea() && pSettler->canFound(pLoopPlot))
                    {
                        if (!pLoopPlot->isVisibleEnemyUnit(pSettler))
                        {
                            if (pSettler->generatePath(pLoopPlot, 0, true, &iPathTurns))
                            {
                                if (iPathTurns<3)
                                {
                                    iValue = pLoopPlot->getFoundValue(getOwnerINLINE());
                                    if (iValue > iBestValue)
                                    {
                                        iBestValue = iValue;
                                        pBestPlot = pLoopPlot;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (pBestPlot != NULL)
        {
            if(!pSettler->atPlot(pBestPlot))
            {
	            pSettler->getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			}
        }

		if (pSettler->canFound(getHeadUnit()->plot()))
		{
			pSettler->getGroup()->pushMission(MISSION_FOUND);
			return;
		}
    }

	if(GET_PLAYER(getOwnerINLINE()).isLackMoneyForExpansion())
	{
		//Allow Units to go to do other Tasks
		if(getNumUnits() > 2) {

			if(bDebug) {
				TCHAR szOut[1024];
				sprintf(szOut, "Group Disbanded because of lack of gold \n");
				gDLL->logMsg(szFile, szOut, false, false);
			}
			setDeathDelayed(true);
			return;
		}
	}

	int MAX_UNITS_PER_PLOT=3;

	//let other Units Join the Settler
	if(!GC.getGameINLINE().isOption(GAMEOPTION_UNIT_PER_TILE_LIMIT)) {
		if(pSettler->getGroup()->getNumUnits() < getNumUnits())
		{
			if(bDebug) {
				TCHAR szOut[1024];
				sprintf(szOut, "Not all Units on same Plot \n");
				gDLL->logMsg(szFile, szOut, false, false);
			}

			for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				if(pLoopUnit->getGroup()!=pSettler->getGroup())
				{
					if(pLoopUnit->atPlot(pSettler->plot()))
					{
						pLoopUnit->joinGroup(pSettler->getGroup());
					}
					else if(pLoopUnit->canMoveInto(pSettler->plot()))
					{
						pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pSettler->getX_INLINE(), pSettler->getY_INLINE());
						if(pLoopUnit->atPlot(pSettler->plot()))
						{
							pLoopUnit->joinGroup(pSettler->getGroup());
						}
					}
				}
			}
		}
	}
	else if(pSettler->getGroup()->getNumUnits()<2)
	{
		CvPlot* pNewTarget=NULL;
		CvPlot* pTarget=NULL;
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pAdjacentPlot = plotDirection(pSettler->getX_INLINE(), pSettler->getY_INLINE(), (DirectionTypes) iI);
			if (pAdjacentPlot != NULL)
			{
				if(pAdjacentPlot->getNumUnits() == 0 && pAdjacentPlot->getOwnerINLINE() == getOwnerINLINE())
				{
					pTarget = pAdjacentPlot;
				}
			}
		}

		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->getGroup()!=pSettler->getGroup())
			{
				if(pLoopUnit->canMoveInto(pSettler->plot()))
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pSettler->getX_INLINE(), pSettler->getY_INLINE());
					if(pLoopUnit->atPlot(pSettler->plot()))
					{
						pLoopUnit->joinGroup(pSettler->getGroup());
					}
				}
				else
				{
					//Move to a Different Plot so we can add units to the settler stack
					pNewTarget = pTarget;
				}
			}
		}
		if(pNewTarget != NULL)
		{
			for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

				if(!pLoopUnit->atPlot(pNewTarget))
				{
					if(pLoopUnit->canMoveInto(pNewTarget))
					{
						pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pNewTarget->getX_INLINE(), pNewTarget->getY_INLINE());
						if(pLoopUnit->atPlot(pSettler->plot()))
						{
							pLoopUnit->joinGroup(pSettler->getGroup());
						}
					}
				}
			}
		}
	}

	if(GC.getGameINLINE().isOption(GAMEOPTION_UNIT_PER_TILE_LIMIT)) {
		if(pSettler->plot()->getOwnerINLINE() == getOwnerINLINE() && pSettler->getGroup()->getNumUnits()<2)
		{
			return;
		}
	}

	bool bRetreat = false;

	if (!pSettler->getGroup()->canDefend() && 
		(!pSettler->plot()->isOwned() || GET_PLAYER((PlayerTypes)getOwnerINLINE()).AI_isPlotThreatened(pSettler->plot(), 3)))
	{
		bRetreat = true;
	}

	//settler retreat because of no protection
	if(bRetreat) {
		if(!pSettler->plot()->isCity())
		{
			if(bDebug) {
				TCHAR szOut[1024];
				sprintf(szOut, "Settler Retreat because of no protection \n");
				gDLL->logMsg(szFile, szOut, false, false);
			}

			if(pSettler->AI_retreatToCity()) {
				return;
			}
		}
	}

	CvPlot* pBestPlot = NULL;
	int iValue;
	int iPathTurns;
	int iBestValue = 0;

	//make sure we only move to plots that are already revealed
	if(kPlayer.getNumCities() > 0 && getMissionPlot() != NULL && !getMissionPlot()->isRevealed(getTeam(), false)) {
		setMissionPlot(NULL);
	}

	//current Missionplot isn't valid?
	if(getMissionPlot() == NULL || !pSettler->canFound(getMissionPlot()) || !pSettler->getGroup()->generatePath(pSettler->plot(),getMissionPlot(),0,false,&iPathTurns))
	{
		if(bDebug) {
			TCHAR szOut[1024];
			sprintf(szOut, "Pick new MissionPlot \n");
			gDLL->logMsg(szFile, szOut, false, false);
		}

		setMissionPlot(NULL);
		GET_PLAYER(getOwnerINLINE()).AI_updateFoundValues(false);
/**
		for (int iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
		{
			CvPlot* pCitySitePlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);

**/
		for(int iI = 0; iI < GC.getMapINLINE().numPlots(); iI++)
		{
			CvPlot* pCitySitePlot = GC.getMapINLINE().plotByIndex(iI);
			
			if (pCitySitePlot->getArea() == pSettler->getArea() || pSettler->isWaterWalking())
			{
				iValue = pCitySitePlot->getFoundValue(getOwnerINLINE());

				if(iValue>iBestValue)
				{
					if(kPlayer.getNumCities()==0 || pCitySitePlot->isRevealed(getTeam(), false))
					{
						if(GC.getMapINLINE().calculatePathDistance(pSettler->plot(), pCitySitePlot) > -1)
						{
							int iLoop;
							bool bValid = true;

							for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
							{
								if(pAIGroup != this && pAIGroup->getGroupType() == AIGROUP_SETTLE && pAIGroup->getMissionPlot() == pCitySitePlot)
								{
									bValid = false;
									break;
								}
							}
							if(bValid)
							{
								iBestValue = iValue;
								pBestPlot = pCitySitePlot;
							}
						}
					}
				}
			}
		}
		if(pBestPlot != NULL) {
			setMissionPlot(pBestPlot);
			if(bDebug) {
				TCHAR szOut[1024];
				sprintf(szOut, "New MissionPlot found: value %d\n", pBestPlot->getFoundValue(getOwnerINLINE()));
				gDLL->logMsg(szFile, szOut, false, false);
			}
		}
	}

	//Retreat to City because of no Target
	if(getMissionPlot() == NULL) {
		if(!pSettler->plot()->isCity()) {
			pSettler->AI_retreatToCity();
		}
		if(bDebug) {
			TCHAR szOut[1024];
			sprintf(szOut, "Retreat because still no target\n");
			gDLL->logMsg(szFile, szOut, false, false);
		}

		return;
	}

	//Retreat to City because we do not have enough Units
	if(!GC.getGameINLINE().isOption(GAMEOPTION_UNIT_PER_TILE_LIMIT))
	{
		if(pSettler->plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if(pSettler->getGroup()->getNumUnits() < pSettler->AI_getSettlerPatrolUnitsNeeded())
			{
				if(!pSettler->plot()->isCity())
				{
					pSettler->AI_retreatToCity();
				}
				return;
			}
		}
	}

	if (getMissionPlot() != NULL)
	{
		if(bDebug) {
			TCHAR szOut[1024];
			sprintf(szOut, "Move towards City\n");
			gDLL->logMsg(szFile, szOut, false, false);
		}

		if (!pSettler->atPlot(getMissionPlot()))
		{
			pSettler->getGroup()->pushMission(MISSION_MOVE_TO, getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_FOUND, getMissionPlot());
		}

		if (pSettler->atPlot(getMissionPlot()))
		{
			pSettler->getGroup()->pushMission(MISSION_FOUND, -1, -1, 0, false, false, MISSIONAI_FOUND, getMissionPlot());

			if(getMissionPlot()->isCity() && getMissionPlot()->getOwnerINLINE()==getOwnerINLINE())
			{
				for (CLLNode<IDInfo>* pUnitNode = getMissionPlot()->headUnitNode(); pUnitNode != NULL; pUnitNode = getMissionPlot()->nextUnitNode(pUnitNode))
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					if(pLoopUnit->getOwnerINLINE()==getOwnerINLINE() 
					&& (pLoopUnit->getAIGroup()==NULL || pLoopUnit->getAIGroup()->getGroupType()==AIGROUP_SETTLE || pLoopUnit->getAIGroup()->getGroupType()==AIGROUP_RESERVE) 
					&& GC.getUnitInfo(pLoopUnit->getUnitType()).getCombatDefense()>0)
					{
						if(pLoopUnit->plot()->getPlotCity()->getAIGroup_Defense()->UnitsNeeded()>0)
						{
							pLoopUnit->setAIGroup(pLoopUnit->plot()->getPlotCity()->getAIGroup_Defense());
						}
					}
				}
			}
		}
	}
}