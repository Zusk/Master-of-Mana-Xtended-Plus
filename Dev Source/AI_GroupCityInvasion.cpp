#include "CvGameCoreDLL.h"
#include "CvAIGroup.h"
#include "FAStarNode.h"
#include "CvDLLFAStarIFaceBase.h"

//checks if the Group can still has a city to target
bool CvAIGroup::update_City_Invasion_isTargetStillValid()
{
	bool bValid = true;

	if(getMissionTarget() == NULL || !getMissionTarget()->isCity())
	{
		bValid = false;
	}

	if(bValid) {
		if(!GET_TEAM(getTeam()).isAtWar(getMissionTarget()->getTeam())) {
			if(!GET_TEAM(getTeam()).canDeclareWar(getMissionTarget()->getTeam())) {

				bValid = false;
				//does City already belong to us? if yes, maybe we need to add some defenders?
				if(getMissionTarget()->getOwnerINLINE() == getOwnerINLINE()) {

					CvCityAI* pCity = static_cast<CvCityAI*>(getMissionTarget()->getPlotCity());
					CvAIGroup* pDefense = pCity->getAIGroup_Defense();
					while(pDefense->UnitsNeeded() > 0) {

						CvUnit* pUnitToAdd = NULL;
						int iValue;
						int iBestValue = 0;

						for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
						{
							CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

							if(pDefense->isUnitAllowed(pLoopUnit,AIGROUP_CITY_DEFENSE))
							{
								if(pLoopUnit->plot()==pCity->plot())
								{
									iValue=pCity->AI_ValueCityDefender(pLoopUnit->getUnitType(),pDefense);

									if(iValue>iBestValue)
									{
										iBestValue=iValue;
										pUnitToAdd=pLoopUnit;
									}
								}
							}
						}
						if(pUnitToAdd == NULL)
						{
							break;
						}
						else
						{
							pUnitToAdd->setAIGroup(pDefense);
						}
					}				
				}
			}

			if(GET_TEAM(getTeam()).AI_getWarPlanTarget() != getMissionTarget()->getTeam())
			{
				bValid = false;
			}
	/**
			if(!GET_PLAYER(getOwnerINLINE()).AI_isValidCityforWar(getMissionTarget()->getPlotCity(), NULL))
			{
				bValid=false;
			}
	**/
		}
	}

	if(!bValid)
	{
		setDeathDelayed(true);
		return false;
	}	

	return true;
}

//MissionPlot should always point to the plot where the AIGROUP "stack" is
void CvAIGroup::update_City_Invasion_InitMissionPlot()
{
	if(getNumUnits() == 0)
	{
		setMissionPlot(NULL);
		return;
	}

	CvPlot* pBestPlot=NULL;
	int iBestCount=MAX_INT;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		CvPlot* pLoopPlot = pLoopUnit->plot();

		if(pLoopPlot!=pBestPlot)
		{
			int iLoopCount=0;
			for (CLLNode<IDInfo>* pUnitNode1 = headUnitNode(); pUnitNode1 != NULL; pUnitNode1 = nextUnitNode(pUnitNode1))
			{
				CvUnit* pLoopUnitCount = ::getUnit(pUnitNode1->m_data);

				iLoopCount+=stepDistance(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),pLoopUnitCount->getX_INLINE(),pLoopUnitCount->getY_INLINE());
			}			

			if(iLoopCount<iBestCount)
			{
				iBestCount=iLoopCount;
				pBestPlot=pLoopPlot;
			}
		}
	}

	setMissionPlot(pBestPlot);
}

//Do we have enough Units to attack the Target City?
bool CvAIGroup::update_City_Invasion_enoughUnits()
{
	return UnitsNeeded_City_Invasion(130)<=0;
}

bool CvAIGroup::update_City_Invasion_doesEnemyOutNumberUs(double dMod) const
{
	if(getMissionTarget()==NULL)
	{
		return false;
	}

	//SpyFanatic: given that mission target is a non barbarian city, we should count defensive unit
	//int iEnemyPower=calculateEnemyStrength(getMissionTarget(), 2,true,true,false);
	int iEnemyPower=0;
	if(getMissionTarget()!=NULL && getMissionTarget()->getPlotCity()!=NULL && getMissionTarget()->getPlotCity()->getOwnerINLINE() < BARBARIAN_PLAYER)
	{
		iEnemyPower=calculateEnemyStrength(getMissionTarget(), 2,true,false,true);
	}
	else
	{
		iEnemyPower=calculateEnemyStrength(getMissionTarget(), 2,true,true,false);
	}
	int iOurPower=getGroupPowerWithinRange(getMissionTarget(),MAX_INT);

	return(iEnemyPower>iOurPower*dMod);
}

bool CvAIGroup::update_City_Invasion_doWeHaveHeroToTrain() const
{
	PromotionTypes eHero=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_HERO");
	if(eHero==NO_PROMOTION)
	{
		return false;
	}

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canAttack() && pLoopUnit->isHasPromotion(eHero))
		{
			return true;
		}
	}

	return false;
}

bool CvAIGroup::update_City_Invasion_canWePillageStuff() const
{
	if(getMissionTarget()==NULL || getMissionTarget()->getPlotCity()==NULL)
	{
		return false;
	}

	//do we have a unit to pillage stuff?
	int iCounter=0;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canPillage())
		{
			iCounter++;
		}
	}
	if(iCounter<3)
	{
		return false;
	}

	CvCity* pCity=getMissionTarget()->getPlotCity();

	for (int iI = 0; iI < pCity->getNumCityPlots(); iI++)
	{
		CvPlot* pLoopPlot = pCity->getCityIndexPlot(iI);
		if(pLoopPlot!=NULL && pLoopPlot->getOwnerINLINE()==pCity->getOwnerINLINE() && pLoopPlot!=getMissionTarget())
		{
			if(pLoopPlot->canBePillaged()) {
				return true;
			}
		}
	}

	return false;
}

bool CvAIGroup::update_City_Invasion_canBuildMoreUnits() const
{
	return GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits(0);
}

void CvAIGroup::update_City_Invasion_moveToSecurePlot()
{
	if(getMissionTarget()==NULL)
		return;

	CvPlot* pTarget=getMissionTarget()->getcloseSecurePlot(getTeam());
	int iPathTurns;

	if(pTarget!=NULL)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->canMove() && !pLoopUnit->atPlot(pTarget) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				if(pLoopUnit->generatePath(pTarget,0,true,&iPathTurns))
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
				}
			}
		}
	}
}

CvPlot* CvAIGroup::update_City_Invasion_TrainHero_GetTargetPlot()
{
	CvPlot* pTarget=getMissionTarget();
	//ok, now move towards our target
	CvPlot* pBestPlot=NULL;
	int iValue;
	int iBestValue=0;

	for (int iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
	{
		CvPlot* pAdjacentPlot = plotDirection(pTarget->getX_INLINE(), pTarget->getY_INLINE(), ((DirectionTypes)iJ));
		if(pAdjacentPlot!=NULL && canMoveIntoPlot(pAdjacentPlot,false))
		{
			iValue=10000-100*stepDistance(pAdjacentPlot->getX_INLINE(),pAdjacentPlot->getY_INLINE(),getMissionPlot()->getX_INLINE(),getMissionPlot()->getY_INLINE());
			if(pAdjacentPlot->isVisibleEnemyUnit(getOwnerINLINE()))
			{
				iValue-=1000;
			}
			iValue+=100*pAdjacentPlot->defenseModifier(getTeam(),false);
			if(iValue>iBestValue)
			{
				pBestPlot=pAdjacentPlot;
				iBestValue=iValue;
			}
		}
	}

	return pBestPlot;
}

void CvAIGroup::update_City_Invasion_TrainHero()
{
	if(getMissionPlot()==NULL || getMissionTarget()==NULL)
	{
		return;
	}

	CvPlot* pTarget=getMissionTarget();
	int iPathTurns;

	//first lets move all units to the MissionPlot if they aren't already
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canMove() && !pLoopUnit->atPlot(getMissionPlot()) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
		{
			if(pLoopUnit->generatePath(getMissionPlot(),0,true,&iPathTurns))
			{
				pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
			}
		}
	}

	CvPlot* pBestPlot=update_City_Invasion_TrainHero_GetTargetPlot();

	//break, if there is a unit on the missionplot that cannot move (wait for it). Do not wait if units are less than 10%
	if(getMissionPlot()!=pBestPlot)
	{
		int iCounter=0;
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(!pLoopUnit->canMove() && pLoopUnit->atPlot(getMissionPlot()))
			{
				iCounter++;
			}
		}
		if(iCounter*10>=getNumUnits())
		{
			return;
		}
	}

	//lets move all units to the Plot adjacent to our target City
	if(pBestPlot!=NULL)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->canMove() && !pLoopUnit->atPlot(pBestPlot) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				if(pLoopUnit->generatePath(pBestPlot,0,true,&iPathTurns))
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
				}
			}

			//can we attack now?
			if(pLoopUnit->isHasSecondChance() && (pLoopUnit->AI_attackOdds(pTarget,false)>90 || pLoopUnit->getDamage()<10))
			{
				if(pLoopUnit->canMove() && pLoopUnit->atPlot(pBestPlot) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pTarget->getX_INLINE(), pTarget->getY_INLINE(),MOVE_DIRECT_ATTACK);					
				}
			}
		}
	}
}

void CvAIGroup::update_City_Invasion_PillageStuff()
{
	if(getMissionPlot()==NULL || getMissionTarget()==NULL)
	{
		return;
	}

	CvPlot* pTarget=getMissionTarget();
	int iPathTurns;

	//first lets move all units to the MissionPlot if they aren't already
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->plot()->getWorkingCity()==getMissionTarget()->getPlotCity() && pLoopUnit->canPillage(pLoopUnit->plot()))
		{
			pLoopUnit->pillage();
		}

		if(pLoopUnit->canMove() && !pLoopUnit->atPlot(getMissionPlot()) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
		{
			if(pLoopUnit->generatePath(getMissionPlot(),0,true,&iPathTurns))
			{
				pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
			}
		}
	}

	//ok, now move towards our target
	CvPlot* pBestPlot=NULL;
	int iValue;
	int iBestValue=0;

	CvCity* pCity=getMissionTarget()->getPlotCity();

	for (int iI = 0; iI < pCity->getNumCityPlots(); iI++)
	{
		CvPlot* pLoopPlot = pCity->getCityIndexPlot(iI);
		if(pLoopPlot!=NULL && pLoopPlot->getOwnerINLINE()==pCity->getOwnerINLINE() && pLoopPlot!=getMissionTarget()
			 && canMoveIntoPlot(pLoopPlot,false))
		{
			if(pLoopPlot->canBePillaged())
			{
				iValue = 10000 - 100 * stepDistance(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),getMissionPlot()->getX_INLINE(),getMissionPlot()->getY_INLINE());
				if(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE()))
				{
					iValue-=1000;
				}
				if(pLoopPlot->getBonusType(getTeam())!=NO_BONUS)
				{
					iValue *= 2;
				}
				if(iValue>iBestValue)
				{
					pBestPlot = pLoopPlot;
					iBestValue = iValue;
				}
			}
		}
	}

	//lets move all units to the Plot adjacent to our target City
	if(pBestPlot != NULL)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->canMove() && !pLoopUnit->atPlot(pBestPlot) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				if(pLoopUnit->generatePath(pBestPlot,0,true,&iPathTurns))
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
				}
			}

			//can we pillage now?
			if(pLoopUnit->canMove() && pLoopUnit->atPlot(pBestPlot) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				pLoopUnit->pillage();
			}
		}
	}
	else
	{
		//we have nothing to pillage anymore, retreat from danger
		update_City_Invasion_moveToSecurePlot();
	}
}

void CvAIGroup::update_City_Invasion_LookForOtherTarget()
{
	CvCity* pLoopCity;
	CvCity* pBestCity=NULL;
	int iValue, iBestValue=MAX_INT;
	int iLoop;
	int iCityDistance = MAX_INT;
	CvPlayer& kPlayer=GET_PLAYER(getOwnerINLINE());

	//if we have another Invasion Group on this land area, disband
	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_CITY_INVASION && pAIGroup->getMissionTarget()!=NULL)
		{
			if(getMissionArea()==pAIGroup->getMissionArea() && pAIGroup!=this)
			{
				setDeathDelayed(true);
				return;
			}
		}
	}

	if(getMissionTarget()!=NULL && getMissionTarget()->isOwned())
	{
		CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)getMissionTarget()->getOwnerINLINE());
		if(GET_TEAM(kPlayer.getTeam()).isAtWar(getTeam()))
		{
			for(pLoopCity = kPlayer.firstCity(&iLoop);pLoopCity!=NULL;pLoopCity = kPlayer.nextCity(&iLoop))
			{
				if(pLoopCity->getArea()==getMissionTarget()->getArea())
				{
					int iTempDistance = pLoopCity->getStepDistance(getMissionTarget());
					if(iTempDistance >= 0 && iTempDistance < iCityDistance)
					//SpyFanatic: point to the nearest city as route, not in linear distance
					//if(stepDistance(pLoopCity->getX_INLINE(),pLoopCity->getY_INLINE(),getMissionTarget()->getX_INLINE(),getMissionTarget()->getY_INLINE())<15)
					{
						iValue=calculateEnemyStrength(getMissionTarget(), 2,true);

						if(iValue<iBestValue)
						{
							iBestValue=iValue;
							pBestCity=pLoopCity;
							iCityDistance=iTempDistance;
						}
					}
				}
			}
		}
	}

	//SpyFanatic: if not at war, check for barbs lairs or cities to cleanup
	if(!GET_TEAM(kPlayer.getTeam()).isAtWar(GET_PLAYER((PlayerTypes)getMissionTarget()->getOwnerINLINE()).getTeam()) && !GET_TEAM(kPlayer.getTeam()).isBarbarianAlly())
	{
/*
		//Train on Lair
		for(int iI = 0; iI < GC.getMapINLINE().numPlots(); ++iI) {
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(iI);
			if(pLoopPlot->isLair())
			{
				bool bValid=false;
				if(pLoopPlot->getOwnerINLINE()==getID())
				{
					oosLog("BarbLair"
					,"Turn:%d,PlayerID:%d,Lairx:%d,LairY:%d,is in cultural borders"
					,GC.getGame().getElapsedGameTurns()
					,getOwnerINLINE()
					,pLoopPlot->getX_INLINE()
					,pLoopPlot->getY_INLINE()
					);
					bValid=true;
				}
				else if(!pLoopPlot->isOwned())
				{
					int iSearchRange=1;
					for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
					{
						for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
						{
							CvPlot* pAdjacentPlot = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iDX, iDY);
							if(pAdjacentPlot!=NULL)
							{
								if(pAdjacentPlot->isVisible(getTeam(),false) && pAdjacentPlot->getOwnerINLINE()==getID())
								{
									oosLog("BarbLair"
									,"Turn:%d,PlayerID:%d,Lairx:%d,LairY:%d,is at distance 1 from cultural borders"
									,GC.getGame().getElapsedGameTurns()
									,getOwnerINLINE()
									,pLoopPlot->getX_INLINE()
									,pLoopPlot->getY_INLINE()
									);
									bValid=true;
									break;
								}
							}
						}
					}
				}
				if(bValid)
				//if(isAllUnitsWithinRange(pLoopCity->plot(),10))
				{
					oosLog("BarbLair"
					,"Turn:%d,PlayerID:%d,Lairx:%d,LairY:%d"
					,GC.getGame().getElapsedGameTurns()
					,getOwnerINLINE()
					,pLoopPlot->getX_INLINE()
					,pLoopPlot->getY_INLINE()
					);

					CvAIGroup* pNewGroup = NULL;
					for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
					{
						if(pAIGroup->getGroupType()==AIGROUP_DESTROY_LAIR && pAIGroup->getMissionPlot()==pLoopPlot)
						{
							if(pAIGroup->UnitsNeeded() > 0)
							{
								pNewGroup = pAIGroup; //Existing group already target lair
								break;
							}
						}
					}
					if(pNewGroup == NULL)
					{
						//Create new stack and move all units there...
						CvAIGroup* pNewGroup=kPlayer.initAIGroup(AIGROUP_DESTROY_LAIR);
						pNewGroup->setMissionPlot(pLoopPlot);
						pNewGroup->setMissionArea(pLoopPlot->getArea());
					}
					//Add Units to this stack
					while(pAIGroup->UnitsNeeded()>0)
					{
						CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionPlot(),pAIGroup, true);
						if(pUnit==NULL)
							break;
						if(isUnitAllowed(pUnit,pAIGroup->getGroupType()) && !pAIGroup->isFull(pUnit))
						{
							pUnit->setAIGroup(pAIGroup);
						}
					}
				}
			}
			if(getNumUnits() <= 0)
			{
				//No more units, stack is empty
				oosLog("BarbLair"
					,"Turn:%d,PlayerID:%d,Lairx:%d,LairY:%d,no more unit stack is empty"
					,GC.getGame().getElapsedGameTurns()
					,getOwnerINLINE()
					,pLoopPlot->getX_INLINE()
					,pLoopPlot->getY_INLINE()
					);
				break;
			}
		}
*/
		//if(getNumUnits() > 0)
		{
			//SpyFanatic: if cant take target city but not yet in war... check for some nearby barbs city to conquer and gain xp
			CvPlayer& kBarbPlayer=GET_PLAYER((PlayerTypes)GC.getBARBARIAN_PLAYER());
			for(pLoopCity = kBarbPlayer.firstCity(&iLoop);pLoopCity!=NULL;pLoopCity = kBarbPlayer.nextCity(&iLoop))
			{
				if(isAllUnitsWithinRange(pLoopCity->plot(),10))
				{
					iValue=calculateEnemyStrength(pLoopCity->plot(), 2,true);

					if(iValue<iBestValue) //Find a barb city easier than the original target
					{
						iBestValue=iValue;
						pBestCity=pLoopCity;
					}
				}
			}
		}
	}


	//TODO
	if(pBestCity==getMissionTarget()->getPlotCity())
	{
		//no other city is a better target, negotioate peace?
		update_City_Invasion_moveToSecurePlot();
	}
	if(pBestCity!=NULL)
	{
		setMissionTarget(pBestCity->plot());
	}
}

void CvAIGroup::update_City_Invasion_AttackCity()
{
	//Assault Enemy City

	if(getMissionPlot()==NULL || getMissionTarget()==NULL)
	{
		return;
	}

	CvPlot* pTarget=getMissionTarget();
	int iPathTurns;

	//retreat wounded units
	healUnits(30,pTarget);

	bool bAtWarWithTarget = getMissionTarget()->getTeam() != NO_TEAM ? GET_TEAM(getTeam()).isAtWar(getMissionTarget()->getTeam()) : false;
	bool bAtWarWithPlot = getMissionPlot()->getTeam() != NO_TEAM ? GET_TEAM(getTeam()).isAtWar(getMissionPlot()->getTeam()) : false;

	//first lets move all units to the MissionPlot if they aren't already
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

		if(pLoopUnit->canMove() && !pLoopUnit->atPlot(getMissionPlot()) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
		{
			if(pLoopUnit->generatePath(getMissionPlot(),0,true,&iPathTurns))
			{
				if(isOOSLogging())
				{
					oosLog(
						"AI_Invasion"
						,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,X:%d,Y:%d,update_City_Invasion_AttackCity MOVE TO TARGET AtWar:%d,UnitID:%d,%S,GroupID:%d,X:%d,Y:%d"
						,GC.getGame().getElapsedGameTurns()
						,getOwnerINLINE()
						,getID()
						,GC.getAIGroupInfo(getGroupType()).getDescription()
						,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
						,getMissionPlot() != NULL ? getMissionPlot()->getX_INLINE() : 0
						,getMissionPlot() != NULL ? getMissionPlot()->getY_INLINE() : 0
						,bAtWarWithPlot
						,pLoopUnit->getID()
						,pLoopUnit->getName().GetCString()
						,pLoopUnit->getGroupID()
						,pLoopUnit->getPathEndTurnPlot()->getX_INLINE()
						,pLoopUnit->getPathEndTurnPlot()->getY_INLINE()
					);
				}
				pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
			}
		}
	}

	//logic to gather troops at missionplot
	//if not enough,wait for more units
	//if also threatened, retreat to safe plot

	int iCounter=0;
	bool bNotEnoughUnits;
	bool bMissionPlotThreatened=GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(getMissionPlot());

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(!pLoopUnit->atPlot(getMissionPlot()))
		{
			iCounter++;
		}
	}
	bNotEnoughUnits=(iCounter*100>=getNumUnits()*20);

	//not enough units at missionplot
	if(bNotEnoughUnits && bMissionPlotThreatened)
	{
		if(isOOSLogging())
		{
			oosLog(
				"AI_Invasion"
				,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,X:%d,Y:%d,update_City_Invasion_AttackCity MOVE SECURE PLOT,PlotThreat:%d,NotEnoughUnit:%d,Count:%d,Num:%d"
				,GC.getGame().getElapsedGameTurns()
				,getOwnerINLINE()
				,getID()
				,GC.getAIGroupInfo(getGroupType()).getDescription()
				,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
				,getMissionPlot() != NULL ? getMissionPlot()->getX_INLINE() : 0
				,getMissionPlot() != NULL ? getMissionPlot()->getY_INLINE() : 0
				,bMissionPlotThreatened
				,bNotEnoughUnits
				,iCounter
				,getNumUnits()
			);
		}
		update_City_Invasion_moveToSecurePlot();
	}
	else
	{
		//first lets move all units to the MissionPlot if they aren't already
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

			if(pLoopUnit->canMove() && !pLoopUnit->atPlot(getMissionPlot()) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				if(pLoopUnit->generatePath(getMissionPlot(),0,true,&iPathTurns))
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
				}
			}
		}

		//count again
		iCounter=0;
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(!pLoopUnit->atPlot(getMissionPlot()) || !pLoopUnit->canMove())
			{
				iCounter++;
			}
		}

		//not enough units to march on
		if(iCounter*100>=getNumUnits()*20)
		{
			if(isOOSLogging())
			{
				oosLog(
					"AI_Invasion"
					,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,X:%d,Y:%d,update_City_Invasion_AttackCity WAIT,PlotThreat:%d,NotEnoughUnit:%d,Count:%d,Num:%d"
					,GC.getGame().getElapsedGameTurns()
					,getOwnerINLINE()
					,getID()
					,GC.getAIGroupInfo(getGroupType()).getDescription()
					,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
					,getMissionPlot() != NULL ? getMissionPlot()->getX_INLINE() : 0
					,getMissionPlot() != NULL ? getMissionPlot()->getY_INLINE() : 0
					,bMissionPlotThreatened
					,bNotEnoughUnits
					,iCounter
					,getNumUnits()
				);
			}
			return;
		}
	}

	//ok, now move towards our target
	CvPlot* pBestPlot=NULL;
	int iValue;
	int iBestValue=0;

	for (int iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
	{
		CvPlot* pAdjacentPlot = plotDirection(pTarget->getX_INLINE(), pTarget->getY_INLINE(), ((DirectionTypes)iJ));

		if(pAdjacentPlot!=NULL && canMoveIntoPlot(pAdjacentPlot,false))
		{
			//SpyFanatic: dont circumnavigate city and lose turn just to find a plot with higher defense
			bool nearbyX = (pAdjacentPlot->getX_INLINE() <= pTarget->getX_INLINE() && pAdjacentPlot->getX_INLINE() >= getMissionPlot()->getX_INLINE()) || (pAdjacentPlot->getX_INLINE() >= pTarget->getX_INLINE() && pAdjacentPlot->getX_INLINE() <= getMissionPlot()->getX_INLINE());
			bool nearbyY = (pAdjacentPlot->getY_INLINE() <= pTarget->getY_INLINE() && pAdjacentPlot->getY_INLINE() >= getMissionPlot()->getY_INLINE()) || (pAdjacentPlot->getY_INLINE() >= pTarget->getY_INLINE() && pAdjacentPlot->getY_INLINE() <= getMissionPlot()->getY_INLINE());
			if(nearbyX && nearbyY)
			{
			iValue=10000-100*stepDistance(pAdjacentPlot->getX_INLINE(),pAdjacentPlot->getY_INLINE(),getMissionPlot()->getX_INLINE(),getMissionPlot()->getY_INLINE());
			if(pAdjacentPlot->isVisibleEnemyUnit(getOwnerINLINE()))
			{
				iValue-=1000;
			}
			iValue+=1*pAdjacentPlot->defenseModifier(getTeam(),false);
			if(iValue>iBestValue)
			{
				pBestPlot=pAdjacentPlot;
				iBestValue=iValue;
			}
			}
		}
	}

	//lets move all units to the Plot adjacent to our target City
	if(pBestPlot!=NULL)
	{
		//SpyFanatic: if some units just move here wait for them
		iCounter = 0;
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->hasMoved() && pLoopUnit->atPlot(getMissionPlot()))
			{
				iCounter++;
			}
		}
		if(iCounter <= 0)
		{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->canMove() && !pLoopUnit->atPlot(pBestPlot) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				if(pLoopUnit->generatePath(pBestPlot,MOVE_IGNORE_DANGER,true,&iPathTurns))
				{
					if(isOOSLogging())
					{
						oosLog(
							"AI_Invasion"
							,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,X:%d,Y:%d,update_City_Invasion_AttackCity MOVE BEST PLOT AtWar:%d,UnitID:%d,%S,GroupID:%d,X:%d,Y:%d"
							,GC.getGame().getElapsedGameTurns()
							,getOwnerINLINE()
							,getID()
							,GC.getAIGroupInfo(getGroupType()).getDescription()
							,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
							,getMissionPlot() != NULL ? getMissionPlot()->getX_INLINE() : 0
							,getMissionPlot() != NULL ? getMissionPlot()->getY_INLINE() : 0
							,bAtWarWithPlot
							,pLoopUnit->getID()
							,pLoopUnit->getName().GetCString()
							,pLoopUnit->getGroupID()
							,pLoopUnit->getPathEndTurnPlot()->getX_INLINE()
							,pLoopUnit->getPathEndTurnPlot()->getY_INLINE()
						);
					}
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_IGNORE_DANGER);
				}
			}
		}
		}
	}

	//can we attack now?
	//Powerful Enough to Launch Final attack?
	//ToDo add in Calculation for Damage Spells pre Combat
	int iEnemyStrength, iOurStrength;
	compareInvasionPower(pTarget,&iEnemyStrength,&iOurStrength);

	if(iOurStrength>iEnemyStrength*1.2)
	{
		if(isOOSLogging())
		{
			oosLog(
				"AI_Invasion"
				,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,X:%d,Y:%d,update_City_Invasion_AttackCity LAUNCH ATTACK,iOurStrength:%d,iEnemyStrength:%d"
				,GC.getGame().getElapsedGameTurns()
				,getOwnerINLINE()
				,getID()
				,GC.getAIGroupInfo(getGroupType()).getDescription()
				,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
				,getMissionPlot() != NULL ? getMissionPlot()->getX_INLINE() : 0
				,getMissionPlot() != NULL ? getMissionPlot()->getY_INLINE() : 0
				,iOurStrength
				,iEnemyStrength
			);
		}
		launchAttack(pTarget);
	}
	else
	{
		//can still pillage stuff?
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->plot()->getWorkingCity()==getMissionTarget()->getPlotCity() && pLoopUnit->canPillage(pLoopUnit->plot()))
			{
				if(pLoopUnit->canMove())
				{
					if(isOOSLogging())
					{
						CvPlot* pEndOfTurnPlot = pLoopUnit->getPathEndTurnPlot();
						oosLog(
							"AI_Invasion"
							,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,X:%d,Y:%d,update_City_Invasion_AttackCity PILLAGE,UnitID:%d,X:%d,Y:%d"
							,GC.getGame().getElapsedGameTurns()
							,getOwnerINLINE()
							,getID()
							,GC.getAIGroupInfo(getGroupType()).getDescription()
							,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
							,getMissionPlot() != NULL ? getMissionPlot()->getX_INLINE() : 0
							,getMissionPlot() != NULL ? getMissionPlot()->getY_INLINE() : 0
							,pLoopUnit->getID()
							,pEndOfTurnPlot != NULL ? pEndOfTurnPlot->getX_INLINE() : 0
							,pEndOfTurnPlot != NULL ? pEndOfTurnPlot->getY_INLINE() : 0
						);
					}
					pLoopUnit->pillage();
				}
			}
		}
	}
}

void CvAIGroup::update_City_Invasion()
{
	if(!update_City_Invasion_isTargetStillValid())
	{
		return;
	}

	CvWString szTempBufferInitial;
	getAI_InvasionStatusString(szTempBufferInitial, getMissionStatus());

	//MissionPlot should always point to the plot where the AIGROUP "stack" is
	update_City_Invasion_InitMissionPlot();

	if(getMissionStatus()==NO_AIGROUP_STATUS)
	{
		setMissionStatus(STATUS_CITY_INVASION_WAIT_MORE_UNITS);
	}

	if(update_City_Invasion_enoughUnits())
	{
		setMissionStatus(STATUS_CITY_INVASION_TAKECITY);
	}
	else if(getMissionStatus()==STATUS_CITY_INVASION_TAKECITY)
	{
		setMissionStatus(STATUS_CITY_INVASION_WAIT_MORE_UNITS);
	}

	//Do we still want to train a Hero?
	if(getMissionStatus()==STATUS_CITY_INVASION_TRAIN_HERO)
	{
		if(update_City_Invasion_doesEnemyOutNumberUs(2.0))
		{
			setMissionStatus(STATUS_CITY_INVASION_WAIT_MORE_UNITS);
		}

		if(!update_City_Invasion_doWeHaveHeroToTrain())
		{
			setMissionStatus(STATUS_CITY_INVASION_WAIT_MORE_UNITS);
		}
	}

	//Do we still want to pillage Stuff?
	if(getMissionStatus()==STATUS_CITY_INVASION_PILLAGE_STUFF)
	{
		if(update_City_Invasion_doesEnemyOutNumberUs())
		{
			setMissionStatus(STATUS_CITY_INVASION_WAIT_MORE_UNITS);
		}

		if(!update_City_Invasion_canWePillageStuff())
		{
			setMissionStatus(STATUS_CITY_INVASION_WAIT_MORE_UNITS);
		}
	}

	CvWString szTempBuffer;
	getAI_InvasionStatusString(szTempBuffer, getMissionStatus());

	//can we attack our target?
	int iEnemyStrength, iOurStrength;
	compareInvasionPower(getMissionTarget(),&iEnemyStrength,&iOurStrength);

	if(isOOSLogging())
	{
		oosLog(
			"AI_Invasion"
			,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,PowerRatio:%d,Already at War:%d,AnyWarPlan:%d,Initial MissionStatus:%S,Current MissionStatus:%S,Units:%d,Units needed:%d,OurStrengthNearTarget:%d,GroupStrength:%d,EnemyStrength:%d"
			//,"Turn:%d,PlayerID:%d,GroupID:%d,%S,TargetCity:%S,PowerRatio:%d,Already at War:%d,AnyWarPlan:%d,Current MissionStatus:%S,Units:%d,Units needed:%d,OurStrength:%d,EnemyStrength:%d,iEnemyPower:%d (WithDefense: %d),iOurPowerGlobal:%d"
			,GC.getGame().getElapsedGameTurns()
			,getOwnerINLINE()
			,getID()
			,GC.getAIGroupInfo(getGroupType()).getDescription()
			,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
			,GET_TEAM(getTeam()).getPower(true)*100/(1+GET_TEAM(getMissionTarget()->getTeam()).getPower(true))
			,GET_TEAM(getTeam()).isAtWar(getMissionTarget()->getTeam())
			,GET_TEAM(getTeam()).getAnyWarPlanCount(true)
			,szTempBufferInitial.c_str()
			,szTempBuffer.c_str()
			,getNumUnits()
			,UnitsNeeded()
			,iOurStrength
			,getMissionTarget() != NULL ? getGroupPowerWithinRange(getMissionTarget(),MAX_INT) : -1 //This is the global power of the group, not only the one near the plot
			,iEnemyStrength
			//,getMissionTarget() != NULL ? calculateEnemyStrength(getMissionTarget(), 2,true,true,false) : -1
			//,getMissionTarget() != NULL ? calculateEnemyStrength(getMissionTarget(), 2,true,false,true) : -1
		);
	}

	if(iOurStrength > iEnemyStrength*1.2) {
		launchAttack(getMissionTarget());

		if(!update_City_Invasion_isTargetStillValid())
			return;
	}

	if(getMissionStatus() == STATUS_CITY_INVASION_WAIT_MORE_UNITS)
	{
		if(update_City_Invasion_canBuildMoreUnits() && getNumUnits()<5)
		{
			update_City_Invasion_moveToSecurePlot();
		}
		else
		{
			//we do not have enough units, but we also cannot build more units or have enough to have some fun, so use guerrila warfare
			if(update_City_Invasion_doesEnemyOutNumberUs())
			{
				setMissionStatus(STATUS_CITY_INVASION_LOOK_FOR_OTHER_TARGET);
			}
			else
			{
				if(update_City_Invasion_doWeHaveHeroToTrain())
				{
					setMissionStatus(STATUS_CITY_INVASION_TRAIN_HERO);
					update_City_Invasion_TrainHero();
				}
				else
				{
					if(update_City_Invasion_canWePillageStuff())
					{
						setMissionStatus(STATUS_CITY_INVASION_PILLAGE_STUFF);
						update_City_Invasion_PillageStuff();
					}
					else
					{
						setMissionStatus(STATUS_CITY_INVASION_LOOK_FOR_OTHER_TARGET);
						update_City_Invasion_LookForOtherTarget();
					}
				}
			}
		}
	}

	//now do actual commands

	attackAnyUnit(66);

	if(getMissionStatus() == STATUS_CITY_INVASION_TAKECITY)
	{
		//declare war if necessary
		/**
		if(!GET_TEAM(getTeam()).isAtWar(getMissionTarget()->getTeam()))
		{
			if(GET_TEAM(getTeam()).canDeclareWar(getMissionTarget()->getTeam()))
			{
				GET_TEAM(getTeam()).declareWar(getMissionTarget()->getTeam(), true, WARPLAN_LIMITED);
			}
		}
		**/

		update_City_Invasion_AttackCity();
	}

	if(getMissionStatus()==STATUS_CITY_INVASION_LOOK_FOR_OTHER_TARGET)
	{
		update_City_Invasion_LookForOtherTarget();
	}

	if(getMissionStatus()==STATUS_CITY_INVASION_TRAIN_HERO)
	{
		update_City_Invasion_TrainHero();
	}

	if(getMissionStatus()==STATUS_CITY_INVASION_PILLAGE_STUFF)
	{
		update_City_Invasion_PillageStuff();
	}

}
