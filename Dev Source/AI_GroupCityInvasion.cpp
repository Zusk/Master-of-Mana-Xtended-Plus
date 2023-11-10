#include "CvGameCoreDLL.h"
#include "CvAIGroup.h"

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

	int iEnemyPower=calculateEnemyStrength(getMissionTarget(), 2,true,true,false);
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
					if(stepDistance(pLoopCity->getX_INLINE(),pLoopCity->getY_INLINE(),getMissionTarget()->getX_INLINE(),getMissionTarget()->getY_INLINE())<15)
					{
						iValue=calculateEnemyStrength(getMissionTarget(), 2,true);

						if(iValue<iBestValue)
						{
							iBestValue=iValue;
							pBestCity=pLoopCity;
						}
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

	//lets move all units to the Plot adjacent to our target City
	if(pBestPlot!=NULL)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(pLoopUnit->canMove() && !pLoopUnit->atPlot(pBestPlot) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				if(pLoopUnit->generatePath(pBestPlot,MOVE_IGNORE_DANGER,true,&iPathTurns))
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_IGNORE_DANGER);
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

	/** DEBUG **/
	//TCHAR szOut[1024];
	CvWString szTempBuffer;
	getAI_InvasionStatusString(szTempBuffer, getMissionStatus());

	if(isOOSLogging())	
	{
	oosLog(
	"AI_Invasion"
	,"Turn: %d,PlayerID: %d,GroupID: %d,%S,TargetCity: %S,PowerRatio:%d,Already at War:%d,Current MissionStatus:%S,Units:%d,Units needed:%d\n"
	,GC.getGame().getElapsedGameTurns()
	,getOwnerINLINE()
	,getID()
	,GC.getAIGroupInfo(getGroupType()).getDescription()
	,(getMissionTarget()!=NULL && getMissionTarget()->isCity())?getMissionTarget()->getPlotCity()->getName().c_str():L""
	,GET_TEAM(getTeam()).getPower(true)*100/(1+GET_TEAM(getMissionTarget()->getTeam()).getPower(true))
	,GET_TEAM(getTeam()).isAtWar(getMissionTarget()->getTeam())
	,szTempBuffer.c_str()
	,getNumUnits()
	,UnitsNeeded()
	);
	}
/*
	TCHAR szFile[1024];
	sprintf(szFile, "AI_Invasion_%d_%d.log",getOwnerINLINE(),getID());

	TCHAR szOut[1024];
	sprintf(szOut, "CvAIInvasion %d,%d::update -- %S -- %d\n",getOwnerINLINE(),getID(),GC.getAIGroupInfo(getGroupType()).getDescription(),GC.getGame().getElapsedGameTurns());
	gDLL->logMsg(szFile,szOut, false, false);
	if(getMissionTarget()!=NULL && getMissionTarget()->isCity())
	{
		sprintf(szOut, "TargetCity -- %S\n",getMissionTarget()->getPlotCity()->getName().c_str());
		gDLL->logMsg(szFile,szOut, false, false);
	}
	sprintf(szOut, "Power Ratio -- %d\n",GET_TEAM(getTeam()).getPower(true)*100/(1+GET_TEAM(getMissionTarget()->getTeam()).getPower(true)));
	gDLL->logMsg(szFile,szOut, false, false);
	sprintf(szOut, "Already at War? -- %d\n",GET_TEAM(getTeam()).isAtWar(getMissionTarget()->getTeam()));
	gDLL->logMsg(szFile,szOut, false, false);
	sprintf(szOut, "Current MissionStatus -- %d\n",getMissionStatus());
	gDLL->logMsg(szFile,szOut, false, false);
	sprintf(szOut, "Units, Units needed -- %d,%d\n",getNumUnits(),UnitsNeeded());
	gDLL->logMsg(szFile,szOut, false, false);	
*/
	/** DEBUG **/

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

	//can we attack our target?
	int iEnemyStrength, iOurStrength;
	compareInvasionPower(getMissionTarget(),&iEnemyStrength,&iOurStrength);

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
