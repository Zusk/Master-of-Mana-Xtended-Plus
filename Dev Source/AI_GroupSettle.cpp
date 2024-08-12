#include "CvGameCoreDLL.h"
#include "CvAIGroup.h"

void CvAIGroup::update_Settle()
{
	CvUnitAI* pSettler = static_cast<CvUnitAI*>(getHeadUnit());
	CvPlayerAI &kPlayer = GET_PLAYER(getOwnerINLINE());
	bool bDebug = true;

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

	if(isOOSLogging())
	{
		oosLog("AI_SettleGroup"
			,"Turn:%d,PlayerID:%d,GroupID:%d::update,%S,NumUnits:%d,UnitsNeeded:%d,MissionX:%d,MissionY:%d,FoundValue:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getOwnerINLINE()
			,getID()
			,GC.getAIGroupInfo(getGroupType()).getDescription()
			,getNumUnits()
			,UnitsNeeded()
			,(getMissionPlot() != NULL)?getMissionPlot()->getX_INLINE():-1
			,(getMissionPlot() != NULL)?getMissionPlot()->getY_INLINE():-1
			,(getMissionPlot() != NULL)?getMissionPlot()->getFoundValue(getOwnerINLINE()):-1

		);
	}

	//logic for gamestart
	if (GET_PLAYER(getOwnerINLINE()).getNumCities() == 0)
	{
		if(pSettler->getOriginPlot() == NULL)		{
			pSettler->setOriginPlot(pSettler->plot());
		}

		//GET_PLAYER(getOwnerINLINE()).AI_updateFoundValues(false);
		GET_PLAYER(getOwnerINLINE()).AI_updateFoundValues(true);

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
			if(isOOSLogging()) {
				oosLog("AI_SettleGroup"
					,"Turn:%d,PlayerID:%d,GroupID:%d,Group Disbanded because of lack of gold,GoldRate:%d,target:%d,NumCities:%d,Gold:%d,CultivationRate:%d"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwnerINLINE()
					,getID()
					,GET_PLAYER(getOwnerINLINE()).calculateGoldRate()
					,5 + 5 * GET_PLAYER(getOwnerINLINE()).getNumCities()
					,GET_PLAYER(getOwnerINLINE()).getNumCities()
					,GET_PLAYER(getOwnerINLINE()).getGold()
					,GET_PLAYER(getOwnerINLINE()).getCultivationRate()
				);
			}
			setDeathDelayed(true);
			return;
		}
	}

	int MAX_UNITS_PER_PLOT=3;
	bool bUnitJoinedStack = false;

	//let other Units Join the Settler
	if(!GC.getGameINLINE().isOption(GAMEOPTION_UNIT_PER_TILE_LIMIT)) {
		if(pSettler->getGroup()->getNumUnits() < getNumUnits())
		{
			if(isOOSLogging()) {
				oosLog("AI_SettleGroup"
					,"Turn:%d,PlayerID:%d,GroupID:%d,Not all Units on same Plot"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwnerINLINE()
					,getID()
				);
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
						bUnitJoinedStack = true;
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
							bUnitJoinedStack = true;
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
			if(isOOSLogging())
			{
				oosLog("AI_SettleGroup"
					,"Turn:%d,PlayerID:%d,GroupID:%d,Settler Retreat because of no protection"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwnerINLINE()
					,getID()
				);
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
/*
		if(isOOSLogging())
		{
			oosLog("AI_SettleGroup"
				,"Turn:%d,PlayerID:%d,GroupID:%d,Pick new MissionPlot"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwnerINLINE()
				,getID()
			);
		}
*/
		setMissionPlot(NULL);
		GET_PLAYER(getOwnerINLINE()).AI_updateFoundValues(false);
/**
		for (int iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
		{
			CvPlot* pCitySitePlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);

**/
		for(int iPass = 0; iPass < 2; iPass++)
		{
			if(pBestPlot == NULL)
			{
				for(int iI = 0; iI < GC.getMapINLINE().numPlots(); iI++)
				{
					CvPlot* pCitySitePlot = GC.getMapINLINE().plotByIndex(iI);

					if (pCitySitePlot->getArea() == pSettler->getArea() || pSettler->isWaterWalking())
					{
						iValue = pCitySitePlot->getFoundValue(getOwnerINLINE());

						/*if(iValue > 0)
						{
						oosLog("AI_SettleGroup"
											,"Turn:%d,PlayerID:%d,GroupID:%d,Possible MissionPlot [%d,%d] found value:%d distance:%d"
											,GC.getGameINLINE().getElapsedGameTurns()
											,getOwnerINLINE()
											,getID()
											,pCitySitePlot->getX_INLINE()
											,pCitySitePlot->getY_INLINE()
											,pCitySitePlot->getFoundValue(getOwnerINLINE())
											,GC.getMapINLINE().calculatePathDistance(pSettler->plot(), pCitySitePlot)
										);
						}*/
						if(iValue>iBestValue)
						{
							if(kPlayer.getNumCities()==0 || pCitySitePlot->isRevealed(getTeam(), false))
							{
								//SpyFanatic: picking up a nearby city spot first
								//int iPathDistance = GC.getMapINLINE().calculatePathDistance(pSettler->plot(), pCitySitePlot);
								//if(iPathDistance > -1 && (iPathDistance < 10 || iPass > 0))
								int iPathDistance;
								bool pathOK = pSettler->getGroup()->generatePath(pSettler->plot(), pCitySitePlot, MOVE_SAFE_TERRITORY, false, &iPathDistance);
								if(pathOK && (iPathDistance < 10 || iPass > 0))
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
			}
		}
		if(pBestPlot != NULL) {
			setMissionPlot(pBestPlot);
			if(isOOSLogging())
			{
				oosLog("AI_SettleGroup"
					,"Turn:%d,PlayerID:%d,GroupID:%d,New MissionPlot [%d,%d] found value:%d"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwnerINLINE()
					,getID()
					,pBestPlot->getX_INLINE()
					,pBestPlot->getY_INLINE()
					,pBestPlot->getFoundValue(getOwnerINLINE())
				);
			}
		}
	}

	//Retreat to City because of no Target
	if(getMissionPlot() == NULL) {
		if(!pSettler->plot()->isCity()) {
			pSettler->AI_retreatToCity();
		}
		if(isOOSLogging())
		{
			oosLog("AI_SettleGroup"
				,"Turn:%d,PlayerID:%d,GroupID:%d,Retreat because still no target"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwnerINLINE()
				,getID()
			);
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
					/*if(pSettler->getGroup()->getNumUnits() > 0 && !pSettler->getGroup()->canAllMove() && !bUnitJoinedStack)
					{
						//TODO SpyFanatic: maybe remove settler from stack if no units added to it this turn and stack cannot move?
					}*/
					if(/*pSettler->getGroup()->getNumUnits() == 0 || */pSettler->getGroup()->canAllMove()) //SpyFanatic: retreat immediately only if unit/group can move
					{
						pSettler->AI_retreatToCity();
					}
				}
				return;
			}
		}
	}

	if (getMissionPlot() != NULL)
	{
		if (!pSettler->atPlot(getMissionPlot()))
		{
			//SpyFanatic: why not destroy some lair in the meantime?
			bool bMove = true;
			if(pSettler->getGroup()->getNumUnits() > 4 && GET_PLAYER((PlayerTypes)getOwnerINLINE()).AI_isPlotThreatened(pSettler->plot(), 1))
			{
				for (int iDX = -1; iDX <= 1; iDX++)
				{
					for (int iDY = -1; iDY <= 1; iDY++)
					{
						CvPlot* pLoopPlot = plotXY(pSettler->plot()->getX_INLINE(), pSettler->plot()->getY_INLINE(), iDX, iDY);
						if(pLoopPlot->isLair())
						{
							//Powerful Enough to Launch attack?
							//ToDo add in Calculation for Damage Spells pre Combat
							int iEnemyStrength, iOurStrength;
							compareAttackStrength(pLoopPlot,&iEnemyStrength,&iOurStrength);

							if(iOurStrength>iEnemyStrength*1.3 || isAllUnitsWithinRange(pLoopPlot,1))
							{
								launchAttack(pLoopPlot);
								if(isOOSLogging()) {
									oosLog("AI_SettleGroup"
										,"Turn:%d,PlayerID:%d,GroupID:%d,Attack a Lair during his journey [%d,%d]"
										,GC.getGameINLINE().getElapsedGameTurns()
										,getOwnerINLINE()
										,getID()
										,pLoopPlot->getX_INLINE()
										,pLoopPlot->getY_INLINE()
									);
								}
								bMove = false;
							}
							else //if(UnitPowerNeeded()<=0)
							{
								if(isOOSLogging()) {
									oosLog("AI_SettleGroup"
										,"Turn:%d,PlayerID:%d,GroupID:%d,Cannot attack the Lair during his journey [%d,%d] %d vs %d withinrange:%d"
										,GC.getGameINLINE().getElapsedGameTurns()
										,getOwnerINLINE()
										,getID()
										,pLoopPlot->getX_INLINE()
										,pLoopPlot->getY_INLINE()
										,iOurStrength
										,iEnemyStrength
										,isAllUnitsWithinRange(pLoopPlot,1)
									);
								}
								//prepareAttack(pLoopPlot,MAX_INT,1);
								//bMove = false;
							}

							if(!bMove)
							{
								int iExploreSpell=GC.getInfoTypeForString("SPELL_EXPLORE_DUNGEON");
								if(iExploreSpell!=NO_SPELL)
								{
									for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
									{
										CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
										if (pLoopUnit->plot()==getMissionPlot() )
										{
											if(pLoopUnit->canCast(iExploreSpell,false))
											{
												pLoopUnit->cast(iExploreSpell);
												if(isOOSLogging())
												{
													oosLog("AI_DestroyLairGroup"
														,"Turn:%d,PlayerID:%d,GroupID:%d,Lair:[%d,%d],Destroyed by Settle Group"
														,GC.getGameINLINE().getElapsedGameTurns()
														,getOwnerINLINE()
														,getID()
														,pLoopUnit->plot()->getX_INLINE()
														,pLoopUnit->plot()->getY_INLINE()
													);
												}
												break;
											}
										}
									}
								}
								/*
								if(UnitPowerNeeded()>0)
								{
									checkForPossibleUpgrade();
								}*/
								break;
							}
						}
					}
					if(!bMove)
					{
						break;
					}
				}
			}
			if(bMove)
			{
				if(isOOSLogging()) {
					oosLog("AI_SettleGroup"
						,"Turn:%d,PlayerID:%d,GroupID:%d,Move towards City [%d,%d]"
						,GC.getGameINLINE().getElapsedGameTurns()
						,getOwnerINLINE()
						,getID()
						,getMissionPlot()->getX_INLINE()
						,getMissionPlot()->getY_INLINE()
					);
				}
				pSettler->getGroup()->pushMission(MISSION_MOVE_TO, getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_FOUND, getMissionPlot());
			}
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
