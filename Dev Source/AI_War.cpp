#include "CvGameCoreDLL.h"
#include "CvTeamAI.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvGameAI.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "CyArgsList.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvArea.h"
#include "CvDiploParameters.h"
#include "CvInitCore.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvPopupInfo.h"
#include "CvDLLFAStarIFaceBase.h"
#include "FAStarNode.h"
#include "CvEventReporter.h"

void CvTeamAI::calculateWarValues()
{
	if(getLeaderID() != NO_PLAYER)
	{
		//bool bFirst = true;
		TCHAR szWar[1024];
		sprintf(szWar,"");
		for(int iI = 0; iI < GC.getMAX_CIV_TEAMS(); iI++)
		{
			if(iI != getID() && GET_TEAM((TeamTypes)iI).isAlive())
			{
				GET_PLAYER(getLeaderID()).calculateWarValue((TeamTypes)iI);

				if(isOOSLogging())
				{
					sprintf(szWar
						,"%s,[%d]:%6d/%6d"
						,szWar
						,iI
						,AI_getWarValueAccumulated((TeamTypes)iI)
						,AI_getWarValue((TeamTypes)iI)
					);
				}
			}
			else
			{
				if(isOOSLogging() && GET_TEAM((TeamTypes)iI).isEverAlive())
				{
					sprintf(szWar
						,"%s,[%d]:%S"
						,szWar
						,iI
						,iI == getID() ? L"             " : L"XXXXXX/XXXXXX"
					);
				}
			}

		}

		//is Current Target not good anymore?
		if(AI_getWarPlanTarget() != NO_TEAM)
		{
			if(AI_getWarValue(AI_getWarPlanTarget())<-500 || isAtWar(AI_getWarPlanTarget()))
			{
				AI_setWarPlanTarget(NO_TEAM);
			}
		}

		//SpyFanatic: aggressive AI pick valid target chosing less distance and weak
		if(/*AI_getWarPlanTarget() == NO_TEAM &&*/ GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
		{
			int iDistance1 = MAX_INT;
			TeamTypes eEnemyTeam1 = NO_TEAM;
			int iX1 = MAX_INT;
			int iY1 = MAX_INT;
			int iDistance2 = MAX_INT;
			TeamTypes eEnemyTeam2 = NO_TEAM;
			int iX2 = MAX_INT;
			int iY2 = MAX_INT;

			int iLoop, iLoop2;
			for(int iI = 0; iI < GC.getMAX_CIV_TEAMS(); iI++)
			{
				CvTeam& kEnemyTeam = GET_TEAM((TeamTypes)iI);
				if(iI != getID() && kEnemyTeam.isAlive() && kEnemyTeam.isHasMet(getID())) //Check all teams that has been met
				{
					for(int iJ = 0; iJ < kEnemyTeam.getPlayerMemberListSize(); iJ++)
					{
						CvPlayer& kEnemy = GET_PLAYER(kEnemyTeam.getPlayerMemberAt(iJ));
						//Get distance of nearest city of player
						for (CvCity* pLoopCity = kEnemy.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kEnemy.nextCity(&iLoop))
						{
							for(int iK = 0; iK < getPlayerMemberListSize(); iK++)
							{
								CvPlayer& kPlayer = GET_PLAYER(getPlayerMemberAt(iK));
								if(pLoopCity->area()->getCitiesPerPlayer(kPlayer.getID()) > 0) //for each player of enemy team check nearest city
								{
									for (CvCity* pLoopCity2 = kPlayer.firstCity(&iLoop2); pLoopCity2 != NULL; pLoopCity2 = kPlayer.nextCity(&iLoop2))
									{
										if(pLoopCity2->getArea() == pLoopCity->getArea())
										{
											int iTempDistance = pLoopCity->getStepDistance(pLoopCity2->plot());
											if(iTempDistance >= 0)
											{
												//iTempDistance = stepDistance(pLoopCity->getX_INLINE(),pLoopCity->getY_INLINE(),pLoopCity2->getX_INLINE(),pLoopCity2->getY_INLINE());
												if(iTempDistance < iDistance1)
												{
													if(eEnemyTeam1 != kEnemy.getTeam())
													{
														iDistance2 = iDistance1;
														eEnemyTeam2 = eEnemyTeam1;
														iX2 = iX1;
														iY2 = iY1;
													}
													iDistance1 = iTempDistance;
													eEnemyTeam1 = kEnemy.getTeam();
													iX1 = pLoopCity->getX_INLINE();
													iY1 = pLoopCity->getY_INLINE();
												}
												else if(iTempDistance < iDistance2 && eEnemyTeam1 != kEnemy.getTeam())
												{
													iDistance2 = iTempDistance;
													eEnemyTeam2 = kEnemy.getTeam();
													iX2 = pLoopCity->getX_INLINE();
													iY2 = pLoopCity->getY_INLINE();
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

			if(eEnemyTeam1 != NO_TEAM && eEnemyTeam2 != NO_TEAM)
			{
				int iOurPower = getPower(true)*100;
				int iEnemy1Power = eEnemyTeam1 != NO_TEAM ? 1+GET_TEAM(eEnemyTeam1).getPower(true) : 0;
				int iEnemy2Power = eEnemyTeam2 != NO_TEAM ? 1+GET_TEAM(eEnemyTeam2).getPower(true) : 0;
				int iEnemy1PowerRatio = eEnemyTeam1 != NO_TEAM ? iOurPower / iEnemy1Power : 0;
				int iEnemy2PowerRatio = eEnemyTeam2 != NO_TEAM ? iOurPower / iEnemy2Power : 0;
				if(eEnemyTeam2 == NO_TEAM || eEnemyTeam1 == eEnemyTeam2 || iEnemy1Power <= iEnemy2Power)
				{
					//If power ratio for both is above 5 then pick the nearest one instead
					if(iEnemy1PowerRatio >= 5 && iEnemy2PowerRatio >= 5)
					{
						AI_setWarPlanTarget(iDistance1 < iDistance2 ? eEnemyTeam1 : eEnemyTeam2);
					}
					else
					{
						AI_setWarPlanTarget(eEnemyTeam1);
					}
				}
				else
				{
					AI_setWarPlanTarget(eEnemyTeam2);
				}
				if(isOOSLogging())
				{
					if(eEnemyTeam2 == NO_TEAM || eEnemyTeam1 == eEnemyTeam2)
					{
						oosLog("AINearbyEnemy"
							,"Turn:%d,Team:%d,Power:%d,Enemy1:%d,X:%d,Y:%d,Distance:%d,Power:%d,PowerRatio:%d"
							,GC.getGameINLINE().getElapsedGameTurns()
							,getID()
							,iOurPower
							,eEnemyTeam1
							,iX1
							,iY1
							,iDistance1
							,iEnemy1Power
							,iEnemy1PowerRatio
						);
					}
					else
					{
						oosLog("AINearbyEnemy"
							,"Turn:%d,Team:%d,Power:%d,Enemy1:%d,X:%d,Y:%d,Distance:%d,Power:%d,PowerRatio:%d,Enemy2:%d,X:%d,Y:%d,Distance:%d,Power:%d,PowerRatio:%d"
							,GC.getGameINLINE().getElapsedGameTurns()
							,getID()
							,iOurPower
							,eEnemyTeam1
							,iX1
							,iY1
							,iDistance1
							,iEnemy1Power
							,iEnemy1PowerRatio
							,eEnemyTeam2
							,iX2
							,iY2
							,iDistance2
							,iEnemy2Power
							,iEnemy2PowerRatio
						);
					}
				}
			}
		}

		//pick a Target if there isn't one yet
		if(AI_getWarPlanTarget()==NO_TEAM)
		{
			TeamTypes eBestWarTarget=NO_TEAM;
			int iBestValue = 0;

			for(int iI=0;iI<GC.getMAX_CIV_TEAMS(); iI++)
			{
				if(iI!=getID() && GET_TEAM((TeamTypes)iI).isAlive() && !isAtWar((TeamTypes)iI))
				{
					int iAiWarValue = AI_getWarValue((TeamTypes)iI);
					if(iAiWarValue > iBestValue)
					{
						iBestValue = iAiWarValue;
						eBestWarTarget = (TeamTypes)iI;
					}
				}
			}

			if(eBestWarTarget!=NO_TEAM)
			{
				AI_setWarPlanTarget(eBestWarTarget);
			}
		}

		if(isOOSLogging())
		{
			oosLog(
			"AIWarValues"
			,"Turn:%d,Team[%d] Target:%d%s"
			,GC.getGameINLINE().getElapsedGameTurns()
			//,getLeaderID()
			,getID()
			,AI_getWarPlanTarget()
			,szWar
			);
		}
	}
}

void CvPlayerAI::calculateWarValue(TeamTypes eTeam)
{
	int iValue=0;
	CvTeamAI &kTeam = GET_TEAM(getTeam());

	//Team met yet?
	if(!kTeam.isHasMet(eTeam) || !kTeam.isAlive())
	{
		kTeam.AI_setWarValue(eTeam, -200);
		return;
	}

	UpdateWarValueAccumulated(eTeam);

	iValue -= calculateBaseWarCost(eTeam); //How much does it cost us to upgrade our Military to win the war
	iValue += kTeam.AI_getWarValueAccumulated(eTeam);

	//TEMPFIX
	//BLOCK WAR DECLARATION AT FRIENDLY/PEACEFUL
	if(!GC.getGame().isOption(GAMEOPTION_AGGRESSIVE_AI)) {
		for(int i = 0; i < MAX_CIV_PLAYERS; ++i) {
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)i);
			if(kPlayer.getTeam() == eTeam) {
				if(GET_TEAM(kPlayer.getTeam()).getLeaderID() == i) {
					if(AI_getAttitude((PlayerTypes)i) == ATTITUDE_FRIENDLY) {
						iValue = -200;
					}
					else if(AI_getAttitude((PlayerTypes)i) == ATTITUDE_PLEASED) {
						if(getAlignment() == kPlayer.getAlignment()) {
							iValue = -200;
						}
					}
				}
			}
		}
	}

	//TEMPFIX


	kTeam.AI_setWarValue(eTeam, iValue);
}
//Note, the bigger the most difficult to start war with eTeam!
int CvPlayerAI::calculateBaseWarCost(TeamTypes eTeam) const
{
	int iValue = 0;
	CvTeamAI &kTeam = GET_TEAM(getTeam());

	//Initial Attitude Modification
	int iCounter = 0;
	int iInitialValue = 0;
	if(!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI)) {
		for(int iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iI);
			//Player in enemy team?
			if(kPlayer.getTeam() == eTeam)
			{
				iInitialValue += std::abs(ATTITUDE_FURIOUS - AI_getAttitude((PlayerTypes)iI)) * 100;
				iCounter++;
			}
		}
	}

	if(iCounter > 0) {
		iValue += iInitialValue / iCounter;
	}

	if(AI_getExpansionPlot() != NULL && AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID()) > 0)
	{
		iValue += 1000;
	}

	bool bNoCityClose=true;
	bool bNoCityAround=true;
	bool bNoCitySameArea=true;
	bool bRunnaway=false;

	//Enemy has no Cities nearby?
	for(int iJ = 0; iJ < MAX_CIV_PLAYERS; iJ++)
	{
		CvPlayerAI& kPlayer = GET_PLAYER((PlayerTypes)iJ);
		if(kPlayer.getTeam() == eTeam)
		{
			if(hasCityinRange((PlayerTypes)iJ, 7))
			{
				bNoCityClose = false;
				bNoCityAround = false;
				bNoCitySameArea = false;
			}
			else if(hasCityinRange((PlayerTypes)iJ, 15))
			{
				bNoCityAround = false;
				bNoCitySameArea = false;
			}
			else if(hasCityinRange((PlayerTypes)iJ, MAX_INT))
			{
				bNoCitySameArea = false;
			}
		}
	}

	if(bNoCitySameArea)
	{
		iValue += 4000;
	}
	else if(bNoCityAround)
	{
		iValue += 3000;
	}
	else if(bNoCityClose)
	{
		iValue += 2000;
	}

	//calculate Power Difference
	int iOurPower = GET_TEAM(getTeam()).getPower(true);
	int iEnemyPower = (GET_TEAM(eTeam).getPower(true)*3)/2;
	for(int iI = 0; iI < GC.getMAX_CIV_TEAMS(); iI++)
	{
		if(iI != getTeam() && iI != eTeam)
		{
			if(GET_TEAM((TeamTypes)iI).isAlive())
			{
				if(GET_TEAM((TeamTypes)iI).isAtWar(eTeam))
				{
					//Adds to our Power if Enemies threaten our enemies
					iOurPower+=(GET_TEAM((TeamTypes)iI).getWarPower(eTeam)*2)/3;
				}

				if(GET_TEAM((TeamTypes)iI).isAtWar(getTeam()))
				{
					//Adds to enemy Power if Enemies threaten us
					iEnemyPower+=(GET_TEAM((TeamTypes)iI).getWarPower(getTeam())*2)/3;
				}
			}
		}
	}

	int iWarCost = (iEnemyPower - iOurPower) / (5 + getNumCities()); //TODO should reflect the power/cities of the whole team

	if(GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI)) {
		iWarCost *= 3;
	}

	if(isOOSLogging())
	{
		oosLog(
		"AIWarValuesDetails"
		,"Turn:%d,Team:%d,Target:%d,Expansion:%d,bNoCitySameArea:%d,bNoCityAround:%d,bNoCityClose:%d,iValue:%d,iEnemyPower:%d,iOurPower:%d,iWarCost:%d"
		,GC.getGameINLINE().getElapsedGameTurns()
		,getID()
		,eTeam
		,(AI_getExpansionPlot() != NULL && AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID()) > 0) ? 1000 : 0
		,bNoCitySameArea
		,bNoCityAround
		,bNoCityClose
		,iValue
		,iEnemyPower
		,iOurPower
		,iWarCost
		);
	}

	iValue += iWarCost;

	return iValue;
}

void CvPlayerAI::UpdateWarValueAccumulated(TeamTypes eTeam)
{
	CvTeam& kEnemyTeam = GET_TEAM(eTeam);
	int iNewValue = GET_TEAM(getTeam()).AI_getWarValueAccumulated(eTeam);

	int iAlignmentChange=0;
	int iAttitudeChange=0;
	int iHostileEnchantmentChange=0;
	for(int iI=0;iI<MAX_CIV_PLAYERS;iI++)
	{
		CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)iI);
		//Player in enemy team?
		if(kPlayer.getTeam()==eTeam)
		{
			//Alignment Change
			iAlignmentChange += 1*abs(kPlayer.getAlignment() - getAlignment());
			
			//AI_invalidateAttitudeCache((PlayerTypes)iI);
			//Attitude Change
			iAttitudeChange += ((ATTITUDE_PLEASED - AI_getAttitude((PlayerTypes)iI))*3)/2;
/*
//DEBUG
	TCHAR szOut[1024];
	sprintf(szOut, "Turn: %d,Leader[%d],vs Leader[%d],AI_getPeaceWeight: %d,AI_getPeaceWeightTheir: %d,getWarmongerRespect: %d,getWarmongerRespectTheir: %d,getWorseRankDifferenceAttitudeChange: %d,getBetterRankDifferenceAttitudeChange: %d,AI_getWarSuccessTheir: %d,AI_getWarSuccessMine: %d,getLostWarAttitudeChange: %d\n"
		,GC.getGameINLINE().getElapsedGameTurns()
		,getID()
		,GET_PLAYER((PlayerTypes)iI).getID()
,AI_getHatedCivicAttitude()

	);
	gDLL->logMsg("AIWarValues.log",szOut, false, false);
//DEBUG 
*/
		}
	}

	for(int iI=0;iI<GC.getNumProjectInfos();iI++)
	{
		if(GC.getProjectInfo((ProjectTypes)iI).isHostile())
		{
			iHostileEnchantmentChange+=(kEnemyTeam.getProjectCount((ProjectTypes)iI)>0) ? 5 : 0;
		}
	}
	iAlignmentChange*=100;
	iAlignmentChange/=100;

	iAttitudeChange*=100;
	iAttitudeChange/=100;

	iAlignmentChange/=kEnemyTeam.getNumMembers();
	iAttitudeChange/=kEnemyTeam.getNumMembers();
	iNewValue+=iAttitudeChange+iAlignmentChange;

	/*if(isOOSLogging())
	{
		oosLog(
			"AIWarValues"
			,"Turn: %d,Leader[%d]: %S,vs Leader[%d]: %S,iAttitudeChange: %d,iAlignmentChange: %d\n"
			,GC.getGameINLINE().getElapsedGameTurns()
			//,getLeaderID()
			,getID()
			,L""//GET_PLAYER(getLeaderID()).getName()
			,GET_TEAM((TeamTypes)eTeam).getLeaderID()
			//,iI
			,L""//GET_PLAYER(GET_TEAM((TeamTypes)iI).getLeaderID()).getName()
			,iAttitudeChange
			,iAlignmentChange
		);
	}*/
	
	GET_TEAM(getTeam()).AI_setWarValueAccumulated(eTeam, iNewValue);
}

int CvTeam::getWarPower(TeamTypes eTeam) const
{
	int iCount = 0;
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive() && kLoopPlayer.isThreatinWar(eTeam))
		{
			if (kLoopPlayer.getTeam() == getID() || GET_TEAM(kLoopPlayer.getTeam()).isVassal(getID()))
			{
				iCount += kLoopPlayer.getPower();
			}
		}
	}

	return iCount;
}

//can Player actually hurt eTeam in War?
bool CvPlayerAI::isThreatinWar(TeamTypes eTeam) const
{
	for(int iI=0;iI<MAX_CIV_PLAYERS;iI++)
	{
		if(GET_PLAYER((PlayerTypes)iI).getTeam()==eTeam)
		{
			if(isThreatinWar((PlayerTypes)iI))
			{
				return true;
			}
		}
	}
	return false;
}

bool CvPlayerAI::hasCityinRange(PlayerTypes ePlayer, int iRange) const
{
	CvPlayer& kEnemy = GET_PLAYER(ePlayer);
	int iLoop, iLoop2;

	//does player has City near us?
	for (CvCity* pLoopCity = kEnemy.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kEnemy.nextCity(&iLoop)) {
		if(pLoopCity->area()->getCitiesPerPlayer(getID()) > 0) {
			for (CvCity* pLoopCity2 = firstCity(&iLoop2); pLoopCity2 != NULL; pLoopCity2 = nextCity(&iLoop2)) {
				if(pLoopCity2->getArea() == pLoopCity->getArea()) {
					if(iRange >= stepDistance(pLoopCity->getX_INLINE(),pLoopCity->getY_INLINE(),pLoopCity2->getX_INLINE(),pLoopCity2->getY_INLINE())) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

//can Player actually hurt ePlayer in War?
bool CvPlayerAI::isThreatinWar(PlayerTypes ePlayer) const
{
	// TODO: add check if the Player can reach the target (via naval, planes, etc.)
	if(!isHuman())
		return true;
	//does player has City near us?
	return hasCityinRange(ePlayer, 25);
}

bool CvPlayerAI::isInvasionGroupNotFull(int iAIGroupType, int iAreaID) const
{
	int iLoop2;

	for(CvAIGroup* pAIGroup = firstAIGroup(&iLoop2); pAIGroup != NULL; pAIGroup = nextAIGroup(&iLoop2))
	{
		if(pAIGroup->getGroupType()==iAIGroupType && pAIGroup->getMissionArea()==iAreaID)
		{
			if(iAIGroupType==AIGROUP_CITY_INVASION && pAIGroup->UnitsNeeded()>0)
			{
				return true;
			}

			if(iAIGroupType==AIGROUP_NAVAL_INVASION && !pAIGroup->isAllUnitsForNavalGroup())
			{
				return true;
			}
		}
	}

	return false;
}

//called once a Turn in CvPlayerAI::AI_doWar()
void CvPlayerAI::updateWarTargets()
{
	int iLoop;
	int iLoop2;
	int iLoop3;
	CvArea* pLoopArea;

	GET_TEAM(getTeam()).calculateWarValues();

	//Look for new Targets
	for(pLoopArea = GC.getMapINLINE().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMapINLINE().nextArea(&iLoop))
	{
		// Land Targets
		// TODO: also allow Naval Targets
		if(pLoopArea->getCitiesPerPlayer(getID())>0)
		{
			//There is an invasion Group not full
			if(isInvasionGroupNotFull(AIGROUP_CITY_INVASION, pLoopArea->getID()))
			{
				continue;
			}

			//Find a City for a new Target on this Area
			int iValue;
			int iBestValue=0;
			CvCity* pBestCity=NULL;
			for(int iI=0;iI<GC.getMAX_PLAYERS();iI++)
			{
				CvPlayer &kEnemy=GET_PLAYER((PlayerTypes)iI);
				bool bAtWar=GET_TEAM(getTeam()).isAtWar(kEnemy.getTeam());
				if(kEnemy.isAlive() && pLoopArea->getCitiesPerPlayer((PlayerTypes)iI)>0)
				{
					if(bAtWar || GET_TEAM(getTeam()).canDeclareWar(kEnemy.getTeam()))
					{
						if(bAtWar || GET_TEAM(getTeam()).AI_getWarPlanTarget()== kEnemy.getTeam())
						{
							for (CvCity* pLoopCity = kEnemy.firstCity(&iLoop2); pLoopCity != NULL; pLoopCity = kEnemy.nextCity(&iLoop2))
							{
								if(pLoopCity->getArea()==pLoopArea->getID())
								{
									//City already Target of another Group?
									bool bAlreadyTarget=false;
									for(CvAIGroup* pAIGroup = firstAIGroup(&iLoop3); pAIGroup != NULL; pAIGroup = nextAIGroup(&iLoop3))
									{
										if(pAIGroup->getGroupType()==AIGROUP_CITY_INVASION && pAIGroup->getMissionTarget()==pLoopCity->plot())
										{
											bAlreadyTarget=true;
											break;
										}
									}
									if(!bAlreadyTarget)
									{
										if(AI_isValidCityforWar(pLoopCity, &iValue))
										{
											if(iValue>iBestValue)
											{
												iBestValue=iValue;
												pBestCity=pLoopCity;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			if(pBestCity!=NULL)
			{
				CvAIGroup* pNewGroup=initAIGroup(AIGROUP_CITY_INVASION);
				pNewGroup->setMissionPlot(NULL);
				pNewGroup->setMissionTarget(pBestCity->plot());	//MissionCity can only be used on cities that belong to the player
				pNewGroup->setMissionArea(pBestCity->getArea());
			}
		}
			//this is the Naval case
		else if(canPlayerReachAreaBySea(pLoopArea->getID()))
		{
			//Find a City for a new Target on this Area
			int iValue;
			int iBestValue=0;
			CvCity* pBestCity=NULL;
			for(int iI=0;iI<GC.getMAX_PLAYERS();iI++)
			{
				CvPlayer &kEnemy=GET_PLAYER((PlayerTypes)iI);
				bool bAtWar=GET_TEAM(getTeam()).isAtWar(kEnemy.getTeam());
				if(kEnemy.isAlive() && pLoopArea->getCitiesPerPlayer((PlayerTypes)iI)>0)
				{
					if(bAtWar || GET_TEAM(getTeam()).canDeclareWar(kEnemy.getTeam()))
					{
						if(bAtWar || GET_TEAM(getTeam()).AI_getWarPlanTarget()== kEnemy.getTeam())
						{
							for (CvCity* pLoopCity = kEnemy.firstCity(&iLoop2); pLoopCity != NULL; pLoopCity = kEnemy.nextCity(&iLoop2))
							{
								if(pLoopCity->getArea()==pLoopArea->getID() && pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
								{
									//City already Target of another Group?
									bool bAlreadyTarget=false;
									for(CvAIGroup* pAIGroup = firstAIGroup(&iLoop3); pAIGroup != NULL; pAIGroup = nextAIGroup(&iLoop3))
									{
										if(pAIGroup->getGroupType()==AIGROUP_CITY_INVASION && pAIGroup->getMissionTarget()==pLoopCity->plot())
										{
											bAlreadyTarget=true;
											break;
										}
									}
									if(!bAlreadyTarget)
									{
										if(AI_isValidCityforWar(pLoopCity, &iValue))
										{
											if(iValue>iBestValue)
											{
												iBestValue=iValue;
												pBestCity=pLoopCity;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			if(pBestCity!=NULL)
			{
				int iMissionArea=AI_bestAreaToReachPlotbySea(pBestCity->plot());
				//There is an invasion Group not full
				if(!isInvasionGroupNotFull(AIGROUP_NAVAL_INVASION, iMissionArea))
				{
					if(iMissionArea!=-1)
					{
						CvAIGroup* pNewGroup=initAIGroup(AIGROUP_NAVAL_INVASION);
						pNewGroup->setMissionPlot(pBestCity->plot());
						pNewGroup->setMissionArea(iMissionArea);
						pNewGroup->setMissionTarget(pBestCity->plot());
					}
				}
			}
		}
	}
}

//writes into piValue the Value to target pTargetCity
bool CvPlayerAI::AI_isValidCityforWar(CvCity* pTargetCity, int* piValue)
{
	if(pTargetCity==NULL || pTargetCity->getTeam()==getTeam())
	{
		return false;
	}

	int iValue;
	//int iDifX,iDifY;
	int iLoop;

	bool bSameArea = false;
	int iDistance = MAX_INT;

	//Calculate Distance from border
	for(int iI=0;iI<MAX_CIV_PLAYERS;iI++)
	{
		CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)iI);
		if(kPlayer.getTeam()==getTeam())
		{
			for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				if(pLoopCity->getArea()==pTargetCity->getArea())
				{
					bSameArea=true;
				}

				//iDifX = pLoopCity->getX_INLINE() - pTargetCity->getX_INLINE();
				//iDifY = pLoopCity->getY_INLINE() - pTargetCity->getY_INLINE();
				//iDistance = std::min(iDistance,std::max(iDifX * iDifX, iDifY * iDifY));

				int iTempDistance  = pLoopCity->getStepDistance(pTargetCity->plot());

				if(iTempDistance >= 0)
				{
					iDistance = std::min(iTempDistance,iDistance);
				}
				/*
				if(pTargetCity->plot()->area()->getCitiesPerPlayer(getID())>0)
				{
					//Method has been called to check for land target in updateWarTargets, use turn to reach
					iDistance =
				}
				else
				{
					//Across sea thus use linear distance
					iDistance = std::min(iDistance,std::max(iDifX * iDifX, iDifY * iDifY));
				}
				*/
			}
		}
	}

	//do not Target Barbs if we are allied to them
	if(!GET_TEAM(getTeam()).isAtWar(pTargetCity->getTeam()))
	{
		if(pTargetCity->isBarbarian())
		{
			return false;
		}
	}

	//don't target Barbs cities of size 1 that can still grow
	if(pTargetCity->isBarbarian())
	{
		if(pTargetCity->getPopulation() < 2 && pTargetCity->foodDifference() > 0)
		{
			return false;
		}

		if(GC.getGame().getElapsedGameTurns() < GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getTrainPercent())
		{
			return false;
		}

		//for now, eventually some AIs should hunt down Barbarians
		if(iDistance > 10)
		{
			return false;
		}
	}

	//aren't interested in the actual value
	if(piValue==NULL)
	{
		return true;
	}

	iValue=10000;

	if(GET_TEAM(getTeam()).isAtWar(pTargetCity->getTeam()))
	{
		iValue*=2;
	}

	iValue/=int(10+sqrt((float)iDistance)*3.0f);
	iValue*=100;
	iValue/=100+3*pTargetCity->getDefenseModifier(false);
//	int iTheirStrength=AI_getEnemyPlotStrength(pTargetCity->plot(),0,true,false);
	iValue/=std::max(1,pTargetCity->plot()->getNumDefenders(getID()));

	*piValue=iValue;

	return true;
}

/**
//returns false if we can't declare war
//returns false if we have no interest in war
//iMaxLandRange=Maximum Range in that we target Cities
//bNavalInvasion=also target Cities on different Areas?
bool CvPlayerAI::isPossibleWarTarget(PlayerTypes ePlayer, int iMaxLandRange, bool bNavalInvasion)
{
	CvPlayer &kEnemy=GET_PLAYER(ePlayer);
	CvPlayer &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop, iLoop2;

	//same team?
	if(kEnemy.getTeam()==getTeam())
	{
		return false;
	}

	//our Friend?
	if(AI_getAttitude(ePlayer,true)==ATTITUDE_FRIENDLY)
	{
		return false;
	}

	//recently fought war?
	if(AI_getWarSuccessValue(ePlayer)<0)
	{
		return false
	}

	for (CvCity* pLoopCity = kEnemy.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kEnemy.nextCity(&iLoop))
	{
		if(bNavalInvasion || pLoopCity->area()->getCitiesPerPlayer(getOwnerINLINE())>0)
		{
			int iDistance=MAX_INT;

			for (CvCity* pLoopCity2 = kPlayer.firstCity(&iLoop2); pLoopCity2 != NULL; pLoopCity2 = kPlayer.nextCity(&iLoop2))
			{
				if(pLoopCity2->getArea()==pLoopCity->getArea())
				{
					iDistance=std::min(iDistance,stepDistance(pLoopCity->getX_INLINE(),pLoopCity->getY_INLINE(),pLoopCity2->getX_INLINE(),pLoopCity2->getY_INLINE()));
				}
			}

			if(bNavalInvasion || iDistance<=iMaxLandRange)
			{
				//add bNavalInvasion check if we can reach city by sea
				return true;
			}
		}
	}

	return false;
}

	//are we already at war against a threatening target?
bool CvPlayerAI::pickWarTarget_alreadyAtWar()
{
	for(int iI=0;iI<GC.getMAX_CIV_PLAYERS;iI++)
	{
		CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)iI);
		if(kPlayer.isAlive())
		{
			if(kPlayer.getTeam()!=getTeam()
				&& GET_TEAM(kPlayer.getTeam()).isAtWar(getTeam()))
			{
				//kPlayer can actually hurt us in war
				if(isThreatInWar((PlayerTypes)iI))
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CvPlayerAI::pickWarTarget_instead_of_peaceful_Expansion()
{
	if(AI_getExpansionPlot()!=NULL && AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID())>0)
	{
		//is Enemy Weaker?
		for(int iI=0;iI<GC.getMAX_CIV_PLAYERS();iI++)
		{
			CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)iI);
			if(kPlayer.isAlive() && GET_PLAYER(getOwnerINLINE()).isPossibleWarTarget((PlayerTypes)iI,10,false))
			{
				if(AI_getAttitude((PlayerTypes)iI,true)<ATTITUDE_PLEASED)
				{
					int iEnemyPower=GET_TEAM(kPlayer.getTeam()).getWarPower(getTeam());
					int iOurPower=GET_TEAM(getTeam()).getWarPower(GET_TEAM(kPlayer.getTeam()));
					if(iOurPower*100>iEnemyPower*140)
					{
						//declare War
						return true;
					}
				}
			}
		}

		//can we outproduce Enemy?
		for(int iI=0;iI<GC.getMAX_CIV_PLAYERS();iI++)
		{
			CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)iI);
			if(kPlayer.isAlive() && GET_PLAYER(getOwnerINLINE()).isPossibleWarTarget((PlayerTypes)iI,10,false))
			{
				if(AI_getAttitude((PlayerTypes)iI,true)<ATTITUDE_PLEASED)
				{
					int iEnemyProduction=0;
					int iOurProduction=0;

					for(int iJ=0;iJ<GC.getMAX_CIV_PLAYERS();iJ++)
					{
						CvPlayer& kLoopPlayer=GET_PLAYER((PlayerTypes)iJ);
						if(kLoopPlayer.getTeam()==kPlayer.getTeam())
						{
							iEnemyProduction+=kLoopPlayer.calculateTotalYield(YIELD_PRODUCTION);
						}
						else if(kLoopPlayer.getTeam()==getTeam())
						{
							iOurProduction+=kLoopPlayer.calculateTotalYield(YIELD_PRODUCTION);
						}
					}

					if(iOurProduction*100>iEnemyProduction*140)
					{
						//declare War
						return true;
					}
				}
			}
		}
	}

	return false;
}

//returns the Value of our Target
int CvPlayerAI::getWarTargetValue(PlayerTypes eEnemy,int iMaxLandRange, bool bNavalInvasion)
{
	if(eEnemy==NO_PLAYER)
	{
		return -MAX_INT;
	}
	kEnemy=GET_PLAYER(eEnemy);
	kPlayer=GET_PLAYER(getOwnerINLINE());
	if(!kEnemy.isAlive() || !GET_PLAYER(getOwnerINLINE()).isPossibleWarTarget(eEnemy,iMaxLandRange,bNavalInvasion))
	{
		return -MAX_INT;
	}

	iValue=1000;
	bool bEnoughPower=false;
	int iEnemyPower=GET_TEAM(kEnemy.getTeam()).getWarPower(getTeam());
	int iOurPower=GET_TEAM(getTeam()).getWarPower(kEnemy.getTeam());
	if(iOurPower*100>iEnemyPower*140)
	{
		iValue*=iOurPower;
		iValue/=iEnemyPower;
	}

	int iEnemyProduction=0;
	int iOurProduction=0;

	for(int iJ=0;iJ<GC.getMAX_CIV_PLAYERS();iJ++)
	{
		CvPlayer& kLoopPlayer=GET_PLAYER((PlayerTypes)iJ);
		if(kLoopPlayer.getTeam()==kEnemy.getTeam())
		{
			iEnemyProduction+=kLoopPlayer.calculateTotalYield(YIELD_PRODUCTION);
		}
		else if(kLoopPlayer.getTeam()==getTeam())
		{
			iOurProduction+=kLoopPlayer.calculateTotalYield(YIELD_PRODUCTION);
		}
	}

	if(iOurProduction*100>iEnemyProduction*140)
	{
		iValue*=iOurProduction;
		iValue/=iEnemyProduction;
	}

	int iAttitudeMod=100+10*(ATTITUDE_FRIENDLY-AI_getAttitude(eEnemy,true));

	iValue*=iAttitudeMod;
	iValue/=100;

	return iValue;
}
bool CvPlayerAI::pickWarTarget_Landarea_nearby()
{
	//can still expand peacefully?
	if(AI_getExpansionPlot()!=NULL && AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID())>0)
	{
		return false;
	}

	//is Enemy Weaker?
	int iValue;
	int iBestValue=1000;
	int iBestTarget=-1;
	for(int iI=0;iI<GC.getMAX_CIV_PLAYERS();iI++)
	{
		iValue=getWarTargetValue(PlayerTypes(iI), 10, false)
		if(iValue>iBestValue)
		{
			iBestValue=iValue;
			iBestTarget=iI;
		}
	}

	if(iBestTarget!=NO_PLAYER)
	{
		//declare war
		return true;
	}

	return false;
}

bool CvPlayerAI::pickWarTarget_same_Landarea()
{
	//can still expand peacefully?
	if(AI_getExpansionPlot()!=NULL && AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID())>0)
	{
		return false;
	}

	//is Enemy Weaker?
	int iValue;
	int iBestValue=1000;
	int iBestTarget=-1;
	for(int iI=0;iI<GC.getMAX_CIV_PLAYERS();iI++)
	{
		iValue=getWarTargetValue(PlayerTypes(iI), 30, false)
		if(iValue>iBestValue)
		{
			iBestValue=iValue;
			iBestTarget=iI;
		}
	}

	if(iBestTarget!=NO_PLAYER)
	{
		//declare war
		return true;
	}

	return false;
}

bool CvPlayerAI::pickWarTarget_naval_Invasion()
{
	//can still expand peacefully?
	if(AI_getExpansionPlot()!=NULL && AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID())>0)
	{
		return false;
	}

	//is Enemy Weaker?
	int iValue;
	int iBestValue=1000;
	int iBestTarget=-1;
	for(int iI=0;iI<GC.getMAX_CIV_PLAYERS();iI++)
	{
		iValue=getWarTargetValue(PlayerTypes(iI), MAX_INT, true)
		if(iValue>iBestValue)
		{
			iBestValue=iValue;
			iBestTarget=iI;
		}
	}

	if(iBestTarget!=NO_PLAYER)
	{
		//declare war
		return true;
	}

	return false;
}

bool CvPlayerAI::pickWarTarget_Portal_Invasion()
{
	//can still expand peacefully?
	if(AI_getExpansionPlot()!=NULL && AI_getExpansionPlot()->area()->getCitiesPerPlayer(getID())>0)
	{
		return false;
	}

	//is Enemy Weaker?
	int iValue;
	int iBestValue=1000;
	int iBestTarget=-1;
	for(int iI=0;iI<GC.getMAX_CIV_PLAYERS();iI++)
	{
		iValue=getWarTargetValue(PlayerTypes(iI), 10, false)
		if(iValue>iBestValue)
		{
			iBestValue=iValue;
			iBestTarget=iI;
		}
	}

	if(iBestTarget!=NO_PLAYER)
	{
		//declare war
		return true;
	}

	return false;
}

void CvPlayerAI::pickWarTarget()
{
	bool bPeacefulExpansionPossible;

	if(pickWarTarget_alreadyAtWar())
	{
		return;
	}

	//can peacefully expand in area we already have a city?
	if(pickWarTarget_instead_of_peaceful_Expansion())
	{
		return;
	}

	//search targets same Landarea nearby?
	if(pickWarTarget_Landarea_nearby())
	{
		return;
	}

	//search targets same Landarea nearby?
	if(pickWarTarget_same_Landarea())
	{
		return;
	}

	//search targets for naval Invasion?
	if(pickWarTarget_naval_Invasion())
	{
		return;
	}

	//search targets for Portal Invasion?
	if(pickWarTarget_Portal_Invasion())
	{
		return;
	}

}


// Better War Diplomacy (Skyre)
bool CvTeamAI::AI_isConquestMode() const
{
	for (std::vector<PlayerTypes>::const_iterator iter = m_aePlayerMembers.begin();
         iter != m_aePlayerMembers.end();
		 ++iter)
	{
		if (GET_PLAYER(*iter).isConquestMode())
		{
			return true;
		}
	}

	return false;
}

void CvTeamAI::AI_startWar()
{
	typedef std::vector<std::pair<int, TeamTypes> > TargetList;

	TCHAR szMsgBuf[1024]; // For logging

	int iTotalPower = getPower(true);

	int iThreshold = 150;

	if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
	{
		iThreshold *= 2;
	}

	for (int iPrepare = 0; iPrepare < 2; iPrepare++)
	{
		for (int iPass = 0; iPass < 3; iPass++)
		{
			TargetList targets;

			for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
			{
				
				 // Eliminate any teams we can't go to war with
				 

				if (iI == getID())
				{
					continue;
				}

				const CvTeam& kTeam = GET_TEAM((TeamTypes)iI);

				if (!kTeam.isAlive())
				{
					continue;
				}

				if (!isHasMet((TeamTypes)iI))
				{
					continue;
				}

				if (!canDeclareWar((TeamTypes)iI))
				{
					continue;
				}

				
				// Calculate power ratios
				 

				int iOurPower = iTotalPower;
				int iTheirPower = kTeam.getDefensivePower();

				if (iPass != 2)
				{
					int iRatio;

					// Check power ratios are good enough
					if (iPass == 0)
					{
						if (AI_isLandTarget((TeamTypes)iI) || AI_isAnyCapitalAreaAlone())
						{
							iRatio = AI_maxWarNearbyPowerRatio();
						}
						else
						{
							iRatio = AI_maxWarDistantPowerRatio();
						}
					}
					else
					{
						iRatio = AI_limitedWarPowerRatio();
					}

					if (iOurPower < (iTheirPower * iRatio / 100))
					{
						continue;
					}
				}
				else
				{
					if (kTeam.getAtWarCount(true) == 0)
					{
						continue;
					}

					// Dogpile mechanic
					for (int iJ = 0; iJ < MAX_CIV_TEAMS; iJ++)
					{
						if (iJ != iI && atWar((TeamTypes)iJ, (TeamTypes)iI))
						{
							iOurPower += GET_TEAM((TeamTypes)iJ).getPower(false);
						}
					}
				}

				
				// Work out war priority for this player
				
				// Basic value
				int iValue = AI_startWarVal((TeamTypes)iI);

				// Adjust by power ratio (to maximum of 4X their power)
				iValue *= std::min(4, iOurPower / std::max(1, iTheirPower));

				// Attitude/niceness adjustment
				if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
				{
					iValue *= (100 - AI_noWarAttitudeProb(AI_getAttitude((TeamTypes)iI)));
					iValue /= 100;
				}

				// Prefer land targets
				if (!AI_isLandTarget((TeamTypes)iI))
				{
					if (iPass == 0)
					{
						// No max wars vs. overseas target - AI can't invade yet
						continue;
					}
					else
					{
						iValue /= 4;
					}
				}
				// Prefer civs whose borders we're 'touching'
				else if (AI_calculateAdjacentLandPlots((TeamTypes)iI) == 0)
				{
					if (iPass == 2)
					{
						iValue /= 2;
					}
					else
					{
						iValue /= 4;
					}
				}

				if (iValue > iThreshold)
				{
					targets.push_back(std::make_pair(iValue, (TeamTypes)iI));
				}
			}

			std::sort(targets.begin(), targets.end());

			// Cycle through targets array from best value to worst
			for (TargetList::reverse_iterator it = targets.rbegin();
				 it != targets.rend();
				 ++it)
			{
				if (!iPrepare && !AI_isReadyToAttack(it->second))
				{
					continue;
				}

				if (iPass == 0)
				{
					AI_setWarPlan(it->second, iPrepare ? WARPLAN_PREPARING_TOTAL : WARPLAN_TOTAL);

					if (GC.getDefineINT("USE_DEBUG_LOG") == 1)
					{
						if (iPrepare)
						{
							sprintf(szMsgBuf,
									"%d: *** %S prepares for max war vs. %S (%d) ***\n",
									GC.getGameINLINE().getGameTurn(),
									GET_PLAYER(getLeaderID()).getName(),
									GET_PLAYER(GET_TEAM(it->second).getLeaderID()).getName(),
									it->first);
						}
						else
						{
							sprintf(szMsgBuf,
									"%d: *** %S begins max war vs. %S (%d) ***\n",
									GC.getGameINLINE().getGameTurn(),
									GET_PLAYER(getLeaderID()).getName(),
									GET_PLAYER(GET_TEAM(it->second).getLeaderID()).getName(),
									it->first);
						}

						gDLL->logMsg("war.log", szMsgBuf);
					}
				}
				else if (iPass == 1)
				{
					AI_setWarPlan(it->second, iPrepare ? WARPLAN_PREPARING_LIMITED : WARPLAN_LIMITED);

					if (GC.getDefineINT("USE_DEBUG_LOG") == 1)
					{
						if (iPrepare)
						{
							sprintf(szMsgBuf,
									"%d: *** %S prepares for limited war vs. %S (%d) ***\n",
									GC.getGameINLINE().getGameTurn(),
									GET_PLAYER(getLeaderID()).getName(),
									GET_PLAYER(GET_TEAM(it->second).getLeaderID()).getName(),
									it->first);
						}
						else
						{
							sprintf(szMsgBuf,
									"%d: *** %S begins limited war vs. %S (%d) ***\n",
									GC.getGameINLINE().getGameTurn(),
									GET_PLAYER(getLeaderID()).getName(),
									GET_PLAYER(GET_TEAM(it->second).getLeaderID()).getName(),
									it->first);
						}

						gDLL->logMsg("war.log", szMsgBuf);
					}
				}
				else if (!iPrepare) // Don't 'prepare' for dogpiles
				{
					AI_setWarPlan(it->second, WARPLAN_DOGPILE);

					if (GC.getDefineINT("USE_DEBUG_LOG") == 1)
					{
						sprintf(szMsgBuf,
								"%d: *** %S begins dogpile war vs. %S (%d) ***\n",
								GC.getGameINLINE().getGameTurn(),
								GET_PLAYER(getLeaderID()).getName(),
								GET_PLAYER(GET_TEAM(it->second).getLeaderID()).getName(),
								it->first);

						gDLL->logMsg("war.log", szMsgBuf);
					}
				}

				return;
			}
		}
	}
}

bool CvTeamAI::AI_isReadyToAttack(TeamTypes eTarget) const
{
	for (std::vector<PlayerTypes>::const_iterator it = m_aePlayerMembers.begin();
		 it != m_aePlayerMembers.end();
		 ++it)
	{
		if (GET_PLAYER(*it).AI_isReadyToAttack(eTarget))
		{
			return true;
		}
	}

	return false;
}

// End Better War Diplomacy

// Better War AI (Skyre)
bool CvPlayerAI::AI_isReadyToAttack(TeamTypes eTarget, bool bLog) const
{
	int iI;

	for (CvSelectionGroup* pGroup = firstSelectionGroup(&iI);
		 pGroup != NULL;
		 pGroup = nextSelectionGroup(&iI))
	{
		if (pGroup->AI_getGroupflag() == GROUPFLAG_CONQUEST)
		{
			if (pGroup->AI_isReadyToAttack(eTarget, bLog))
			{
				return true;
			}
		}
	}

	return false;
}
// End Better War AI

// Better War AI (Skyre)
bool CvSelectionGroupAI::AI_isReadyToAttack(TeamTypes eTarget, bool bLog) const
{
    FAssert(getOwnerINLINE() != NO_PLAYER);
    FAssert(getTeam() != NO_TEAM);
    FAssert(plot() != NULL);
    FAssert(area() != NULL);
    FAssert(getNumUnits() >= 1);

	bool bOurTerritory = (getOwnerINLINE() == plot()->getOwnerINLINE());

	// Stack must contain at least 3 units
	// modified Sephi, increase Minimum # of Attack Stacks
	// atleast 6 always, more than 3 * NumCities, 30 is always enough
	if (bOurTerritory && 
		(getNumUnits() < 6 || ((getNumUnits() < 3 * GET_PLAYER(getOwnerINLINE()).getNumCities()) && getNumUnits() < 30)))
	{
		return false;
	}

	
	// Get total enemy power and number of cities
	

	int iEnemyPower = 0;
	int iEnemyCities = 0;

	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		CvTeam& kTeam = GET_TEAM((TeamTypes)iI);

		if (getTeam() == (TeamTypes)iI || !kTeam.isAlive() || kTeam.isBarbarian())
		{
			continue;
		}

		if (GET_TEAM(getTeam()).AI_getWarPlan((TeamTypes)iI) != NO_WARPLAN || eTarget == (TeamTypes)iI)
		{
			if (GET_TEAM(getTeam()).AI_isLandTarget((TeamTypes)iI))
			{
				iEnemyPower += kTeam.countPowerByArea(area());
				iEnemyCities += kTeam.countNumCitiesByArea(area());
			}
			else
			{
				iEnemyPower += kTeam.getPower(false);
				iEnemyCities += kTeam.getNumCities();
			}
		}
	}

	if (iEnemyCities == 0)
	{
		return true;
	}

	
    // Get total power of all units in the group
	
	int iGroupPower = 0;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode();
         pUnitNode != NULL;
		 pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

		iGroupPower += pLoopUnit->getPower();
	}

	if (bLog && GC.getDefineINT("USE_DEBUG_LOG") == 1)
	{
		TCHAR szLogMsg[1024];

		sprintf(szLogMsg,
				"%d: %S = %d vs. %d (%d / %d)\n",
				GC.getGameINLINE().getGameTurn(),
				GET_PLAYER(getOwner()).getName(),
				bOurTerritory ? (iGroupPower / 2) : (iGroupPower * 2 / 3),
				iEnemyPower / iEnemyCities,
				iEnemyPower,
				iEnemyCities);

		gDLL->logMsg("war.log", szLogMsg);
	}

	return (bOurTerritory ? (iGroupPower / 2) : (iGroupPower * 2 / 3)) > (iEnemyPower / iEnemyCities);
}
// End Better War AI

//writes into piValue the Value to target pTargetCity
bool CvPlayerAI::AI_isValidCityforWar(CvCity* pTargetCity, int* piValue)
{
	if(pTargetCity==NULL || pTargetCity->getTeam()==getTeam())
	{
		return false;
	}

	int iValue;
	int iDistance=MAX_INT;
	int iDifX,iDifY;
	int iLoop;
	//Calculate Distance from border
	for(int iI=0;iI<GC.getMAX_CIV_PLAYERS();iI++)
	{
		CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)iI);
		if(kPlayer.getTeam()==getTeam())
		{
			for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				if(pLoopCity->getArea()==pTargetCity->getArea())
				{
					iDifX=pLoopCity->getX_INLINE()-pTargetCity->getX_INLINE();
					iDifY=pLoopCity->getY_INLINE()-pTargetCity->getY_INLINE();
					iDistance=std::min(iDistance,std::max(iDifX*iDifX,iDifY*iDifY));
				}
			}
		}
	}

	if(iDistance==MAX_INT)
	{
		return false;
	}

	int iPowerRatio=(100*GET_TEAM(getTeam()).getPower(true))/std::max(1,GET_TEAM(pTargetCity->getTeam()).getPower(true));
	iPowerRatio=std::min(2000,iPowerRatio);

	if(!GET_TEAM(getTeam()).isAtWar(pTargetCity->getTeam()))
	{
		if(pTargetCity->isBarbarian())
		{
			return false;
		}

		if(AI_getAttitude(pTargetCity->getOwnerINLINE())==ATTITUDE_FRIENDLY)
		{
			return false;
		}

		if(GET_TEAM(getTeam()).AI_getAttitude(pTargetCity->getTeam())==ATTITUDE_FRIENDLY)
		{
			return false;
		}

		if(iDistance>7*7)
		{
			return false;
		}

		if(iPowerRatio<80 || (iPowerRatio<150 && !isConquestMode()))
		{
			return false;
		}
	}

	if(pTargetCity->isBarbarian())
	{
		int iMaxDistance=10*10;

		if(iDistance>iMaxDistance)
		{
			return false;
		}

		if(pTargetCity->getPopulation()<2 && pTargetCity->foodDifference()>0)
		{
			return false;
		}
	}

	//aren't interested in the actual value
	if(piValue==NULL)
	{
		return true;
	}

	iValue=0;

	if(GET_TEAM(getTeam()).isAtWar(pTargetCity->getTeam()))
	{
		iValue+=10000;
	}
	else
	{
		iValue+=5*iPowerRatio;
	}

	iValue/=10+iDistance*3;
//	int iTheirStrength=AI_getEnemyPlotStrength(pTargetCity->plot(),0,true,false);
	iValue/=std::max(1,pTargetCity->plot()->getNumDefenders(getID()));

	*piValue=iValue;

	return true;
}




**/
