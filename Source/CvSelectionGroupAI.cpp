// selectionGroupAI.cpp

#include "CvGameCoreDLL.h"
#include "CvSelectionGroupAI.h"
#include "CvPlayerAI.h"
#include "CvMap.h"
#include "CvPlot.h"
#include "CvTeamAI.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvGameCoreUtils.h"
#include "FProfiler.h"
#include "CVInfos.h"

// Public Functions...

CvSelectionGroupAI::CvSelectionGroupAI()
{
	AI_reset();
}


CvSelectionGroupAI::~CvSelectionGroupAI()
{
	AI_uninit();
}


void CvSelectionGroupAI::AI_init()
{
	AI_reset();

	//--------------------------------
	// Init other game data
}


void CvSelectionGroupAI::AI_uninit()
{
}


void CvSelectionGroupAI::AI_reset()
{
	AI_uninit();

	m_iMissionAIX = INVALID_PLOT_COORD;
	m_iMissionAIY = INVALID_PLOT_COORD;

	m_bForceSeparate = false;

	m_eMissionAIType = NO_MISSIONAI;

	m_missionAIUnit.reset();

	m_bGroupAttack = false;
	m_iGroupAttackX = -1;
	m_iGroupAttackY = -1;
}


void CvSelectionGroupAI::AI_separate()
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);

		pLoopUnit->joinGroup(NULL);
		if (pLoopUnit->plot()->getTeam() == getTeam())
		{
			pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
		}
	}
}

void CvSelectionGroupAI::AI_seperateNonAI(UnitAITypes eUnitAI)
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);
		if (pLoopUnit->AI_getUnitAIType() != eUnitAI)
		{
			pLoopUnit->joinGroup(NULL);
			if (pLoopUnit->plot()->getTeam() == getTeam())
			{
				pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
			}
		}
	}
}

void CvSelectionGroupAI::AI_seperateAI(UnitAITypes eUnitAI)
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);
		if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
		{
			pLoopUnit->joinGroup(NULL);
			if (plot()->getTeam() == getTeam())
			{
				pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
			}
		}
	}
}
// Returns true if the group has become busy...
bool CvSelectionGroupAI::AI_update()
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;
	bool bDead;
	bool bFollow;

	PROFILE("CvSelectionGroupAI::AI_update");

	FAssert(getOwnerINLINE() != NO_PLAYER);

//FfH: Modified by Kael 12/28/2008
//	if (!AI_isControlled())
	if (!AI_isControlled() && !isAIControl())
//FfH: End Modify

	{
		return false;
	}

	if (getNumUnits() == 0)
	{
		return false;
	}

	if (isForceUpdate())
	{
		clearMissionQueue(); // XXX ???
		setActivityType(ACTIVITY_AWAKE);
		setForceUpdate(false);

		// if we are in the middle of attacking with a stack, cancel it
		AI_cancelGroupAttack();
	}

	FAssert(!(GET_PLAYER(getOwnerINLINE()).isAutoMoves()));

	int iTempHack = 0; // XXX

	bDead = false;

	bool bFailedAlreadyFighting = false;
	while ((m_bGroupAttack && !bFailedAlreadyFighting) || readyToMove())
	{
		iTempHack++;
		if (iTempHack > 100)
		{
			FAssert(false);
/*********************************************************************/
/** DEBUG **
    if (GC.getDefineINT("USE_DEBUG_LOG")==1)
    {
        CvString szError="Unit in Loop";
		gDLL->logMsg("loop.log", szError);
		TCHAR szOut[1024];
		int CombatType=getHeadUnit()->getUnitCombatType();
		if (CombatType==-1)
		{
            sprintf(szOut, "UnitCombatType -- %d, %S\n",CombatType,"NO_UNITCOMBAT");
            gDLL->logMsg("loop.log",szOut, false, false);
		}
		else
		{
            sprintf(szOut, "UnitCombatType -- %d, %S\n",CombatType,GC.getUnitCombatInfo(getHeadUnit()->getUnitCombatType()).getDescription());
            gDLL->logMsg("loop.log",szOut, false, false);
		}
		if (getHeadUnit()->getUnitType()!=NO_UNIT)
		{
			sprintf(szOut, "UnitType -- %S\n",GC.getUnitInfo(getHeadUnit()->getUnitType()).getDescription());
			gDLL->logMsg("loop.log",szOut, false, false);
		}
		if (getHeadUnit()->AI_getUnitAIType()!=NO_UNITAI)
		{
			sprintf(szOut, "UnitAIType -- %S\n",GC.getUnitAIInfo(getHeadUnit()->AI_getUnitAIType()).getDescription());
			gDLL->logMsg("loop.log",szOut, false, false);
		}
        sprintf(szOut, "GetX: %d GetY: %d\n",getHeadUnit()->getX(), getHeadUnit()->getY());
		gDLL->logMsg("loop.log",szOut, false, false);
        sprintf(szOut, "UnitID: %d, GroupID: %d, Groupsize: %d\n",getHeadUnit()->getID(), getHeadUnit()->getGroupID(), getHeadUnit()->getGroup()->getNumUnits());
		gDLL->logMsg("loop.log",szOut, false, false);
        sprintf(szOut, "Groupflag: %d\n",getHeadUnit()->AI_getGroupflag());
		gDLL->logMsg("loop.log",szOut, false, false);

    }
** DEBUG End **/
/*********************************************************************/

			CvUnit* pHeadUnit = getHeadUnit();
			if (NULL != pHeadUnit)
			{
				if (GC.getLogging())
				{
					TCHAR szOut[1024];
					CvWString szTempString;
					getUnitAIString(szTempString, pHeadUnit->AI_getUnitAIType());
					sprintf(szOut, "Unit stuck in loop: %S(%S)[%d, %d] (%S)\n", pHeadUnit->getName().GetCString(), GET_PLAYER(pHeadUnit->getOwnerINLINE()).getName(),
						pHeadUnit->getX_INLINE(), pHeadUnit->getY_INLINE(), szTempString.GetCString());
					gDLL->messageControlLog(szOut);
				}

				pHeadUnit->finishMoves();
			}
			break;
		}

		// if we want to force the group to attack, force another attack
		if (m_bGroupAttack)
		{
			m_bGroupAttack = false;

			groupAttack(m_iGroupAttackX, m_iGroupAttackY, MOVE_DIRECT_ATTACK, bFailedAlreadyFighting);
		}
		// else pick AI action
		else
		{
			CvUnit* pHeadUnit = getHeadUnit();

			if (pHeadUnit == NULL || pHeadUnit->isDelayedDeath())
			{
				break;
			}

			resetPath();

			if (pHeadUnit->AI_update())
			{
				// AI_update returns true when we should abort the loop and wait until next slice
				break;
			}

		}

		if (doDelayedDeath())
		{
			bDead = true;
			break;
		}

		// if no longer group attacking, and force separate is true, then bail, decide what to do after group is split up
		// (UnitAI of head unit may have changed)
		if (!m_bGroupAttack && AI_isForceSeparate())
		{
			AI_separate();	// pointers could become invalid...
			return true;
		}
	}

	if (!bDead)
	{
		if (!isHuman())
		{
			bFollow = false;

			// if we not group attacking, then check for follow action
			if (!m_bGroupAttack)
			{
				pEntityNode = headUnitNode();

				while ((pEntityNode != NULL) && readyToMove(true))
				{
					pLoopUnit = ::getUnit(pEntityNode->m_data);
					pEntityNode = nextUnitNode(pEntityNode);

					if (pLoopUnit->canMove())
					{
						resetPath();

						if (pLoopUnit->AI_follow())
						{
							bFollow = true;
							break;
						}
					}
				}
			}

			if (doDelayedDeath())
			{
				bDead = true;
			}

			if (!bDead)
			{
				if (!bFollow && readyToMove(true))
				{
					pushMission(MISSION_SKIP);
				}
			}
		}
	}

	if (bDead)
	{
		return true;
	}

	return (isBusy() || isCargoBusy());
}


// Returns attack odds out of 100 (the higher, the better...)
int CvSelectionGroupAI::AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const
{
	CvUnit* pAttacker;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), NULL, !bPotentialEnemy, bPotentialEnemy) == NULL)
	{
		return 100;
	}

	int iOdds = 0;
	pAttacker = AI_getBestGroupAttacker(pPlot, bPotentialEnemy, iOdds);

	if (pAttacker == NULL)
	{
		return 0;
	}

	return iOdds;
}


CvUnit* CvSelectionGroupAI::AI_getBestGroupAttacker(const CvPlot* pPlot, bool bPotentialEnemy, int& iUnitOdds, bool bForce, bool bNoBlitz) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPossibleTargets;
	int iValue;
	int iBestValue;
	int iOdds;
	int iBestOdds;

	iBestValue = 0;
	iBestOdds = 0;
	pBestUnit = NULL;

	pUnitNode = headUnitNode();

	bool bIsHuman = (pUnitNode != NULL) ? GET_PLAYER(::getUnit(pUnitNode->m_data)->getOwnerINLINE()).isHuman() : true;

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (!pLoopUnit->isDead())
		{
			bool bCanAttack = false;
			if (pLoopUnit->getDomainType() == DOMAIN_AIR)
			{
				bCanAttack = pLoopUnit->canAirAttack();
			}
			else
			{
				bCanAttack = pLoopUnit->canAttack();

				if (bCanAttack && bNoBlitz && pLoopUnit->isBlitz() && pLoopUnit->isMadeAttack())
				{
					bCanAttack = false;
				}
			}

			if (bCanAttack)
			{
				if (bForce || pLoopUnit->canMove())
				{
					if (bForce || pLoopUnit->canMoveInto(pPlot, /*bAttack*/ true, /*bDeclareWar*/ bPotentialEnemy))
					{
						iOdds = pLoopUnit->AI_attackOdds(pPlot, bPotentialEnemy);

						iValue = iOdds;
/*************************************************************************************************/
/**	BETTER AI (Block some Units from attacking at low odds) Sephi              					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

                        if (!GET_PLAYER(pLoopUnit->getOwnerINLINE()).isHuman())
                        {
                            if (pLoopUnit->AI_getUnitAIType()==UNITAI_WARWIZARD)
                            {
                                if (iOdds<95)
                                {
                                    iValue=0;	//Need to put the min to 1 to prevent WoC
                                }
                            }

                            if (pLoopUnit->AI_getUnitAIType()==UNITAI_HERO)
                            {
                                if (iOdds<97)
                                {
                                    iValue=0;
                                }
                            }
							if (pLoopUnit->getLevel()>4 && iOdds<95)
							{
								iValue/=2;
							}
                        }
						iValue=std::max(1,iValue);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
						FAssertMsg(iValue > -1, "iValue is expected to be greater than 0");

						if (pLoopUnit->collateralDamage() > 0)
						{
							iPossibleTargets = std::min((pPlot->getNumVisibleEnemyDefenders(pLoopUnit) - 1), pLoopUnit->collateralDamageMaxUnits());

							if (iPossibleTargets > 0)
							{
								iValue *= (100 + ((pLoopUnit->collateralDamage() * iPossibleTargets) / 5));
								iValue /= 100;
							}
						}

						// if non-human, prefer the last unit that has the best value (so as to avoid splitting the group)
						if (iValue > iBestValue || (!bIsHuman && iValue > 0 && iValue == iBestValue))
						{
							iBestValue = iValue;
							iBestOdds = iOdds;
							pBestUnit = pLoopUnit;
						}
					}
				}
			}
		}
	}

	iUnitOdds = iBestOdds;
	return pBestUnit;
}

CvUnit* CvSelectionGroupAI::AI_getBestGroupSacrifice(const CvPlot* pPlot, bool bPotentialEnemy, bool bForce, bool bNoBlitz) const
{
	int iBestValue = 0;
	CvUnit* pBestUnit = NULL;

	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (!pLoopUnit->isDead())
		{
			bool bCanAttack = false;
			if (pLoopUnit->getDomainType() == DOMAIN_AIR)
			{
				bCanAttack = pLoopUnit->canAirAttack();
			}
			else
			{
				bCanAttack = pLoopUnit->canAttack();

				if (bCanAttack && bNoBlitz && pLoopUnit->isBlitz() && pLoopUnit->isMadeAttack())
				{
					bCanAttack = false;
				}
			}

			if (bCanAttack)
			{
				if (bForce || pLoopUnit->canMove())
				{
					if (bForce || pLoopUnit->canMoveInto(pPlot, true))
					{
                        int iValue = pLoopUnit->AI_sacrificeValue(pPlot);
						FAssertMsg(iValue > 0, "iValue is expected to be greater than 0");
/*************************************************************************************************/
/**	BETTER AI (Summons make good groupattack sacrifice units) Sephi              				**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
						if (pLoopUnit->getDuration()>0)
						{
						    iValue+=10000;
						}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	BETTER AI (Block some Units from attacking at low odds) Sephi              					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
                        if (!GET_PLAYER(pLoopUnit->getOwnerINLINE()).isHuman())
                        {
                            if (pLoopUnit->AI_getUnitAIType()==UNITAI_WARWIZARD)
                            {
	                            iValue=1;
                            }

                            if (pLoopUnit->AI_getUnitAIType()==UNITAI_HERO)
                            {
	                            iValue=1;
                            }
							if (pLoopUnit->getLevel()>4)
							{
	                            iValue=1;
							}
                        }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

						// we want to pick the last unit of highest value, so pick the last unit with a good value
						if (iValue >= iBestValue)
						{
							iBestValue = iValue;
							pBestUnit = pLoopUnit;
						}
					}
				}
			}
		}
	}

	return pBestUnit;
}

// Returns ratio of strengths of stacks times 100
// (so 100 is even ratio, numbers over 100 mean this group is more powerful than the stack on a plot)
int CvSelectionGroupAI::AI_compareStacks(const CvPlot* pPlot, bool bPotentialEnemy, bool bCheckCanAttack, bool bCheckCanMove) const
{
	FAssert(pPlot != NULL);

	int	compareRatio;
	DomainTypes eDomainType = getDomainType();

	// if not aircraft, then choose based on the plot, not the head unit (mainly for transport carried units)
	if (eDomainType != DOMAIN_AIR)
	{
		if (pPlot->isWater())
			eDomainType = DOMAIN_SEA;
		else
			eDomainType = DOMAIN_LAND;

	}

	compareRatio = AI_sumStrength(pPlot, eDomainType, bCheckCanAttack, bCheckCanMove);
	compareRatio *= 100;

	PlayerTypes eOwner = getOwnerINLINE();
	if (eOwner == NO_PLAYER)
	{
		eOwner = getHeadOwner();
	}
	FAssert(eOwner != NO_PLAYER);

	int defenderSum = pPlot->AI_sumStrength(NO_PLAYER, getOwnerINLINE(), eDomainType, true, !bPotentialEnemy, bPotentialEnemy);
	compareRatio /= std::max(1, defenderSum);

	return compareRatio;
}

int CvSelectionGroupAI::AI_sumStrength(const CvPlot* pAttackedPlot, DomainTypes eDomainType, bool bCheckCanAttack, bool bCheckCanMove) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int	strSum = 0;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (!pLoopUnit->isDead())
		{
			bool bCanAttack = false;
			if (pLoopUnit->getDomainType() == DOMAIN_AIR)
				bCanAttack = pLoopUnit->canAirAttack();
			else
				bCanAttack = pLoopUnit->canAttack();

			if (!bCheckCanAttack || bCanAttack)
			{
				if (!bCheckCanMove || pLoopUnit->canMove())
					if (!bCheckCanMove || pAttackedPlot == NULL || pLoopUnit->canMoveInto(pAttackedPlot, /*bAttack*/ true, /*bDeclareWar*/ true))
						if (eDomainType == NO_DOMAIN || pLoopUnit->getDomainType() == eDomainType)
							strSum += pLoopUnit->currEffectiveStr(pAttackedPlot, pLoopUnit);
			}
		}
	}

	return strSum;
}

void CvSelectionGroupAI::AI_queueGroupAttack(int iX, int iY)
{
	m_bGroupAttack = true;

	m_iGroupAttackX = iX;
	m_iGroupAttackY = iY;
}

inline void CvSelectionGroupAI::AI_cancelGroupAttack()
{
	m_bGroupAttack = false;
}

inline bool CvSelectionGroupAI::AI_isGroupAttack()
{
	return m_bGroupAttack;
}

bool CvSelectionGroupAI::AI_isControlled()
{
	return (!isHuman() || isAutomated());
}


bool CvSelectionGroupAI::AI_isDeclareWar(const CvPlot* pPlot)
{
	FAssert(getHeadUnit() != NULL);

	if (isHuman())
	{
		return false;
	}
	else
	{		
// Better War AI (Sephi)
		// Conquest groups can always declare, no matter which unit AIs they contain
		if (getHeadUnit()->getAIGroup()!=NULL && getHeadUnit()->getAIGroup()->getGroupType()==AIGROUP_CITY_INVASION)
		{
			return true;
		}
// End Better War AI

		bool bLimitedWar = false;
		if (pPlot != NULL)
		{
			TeamTypes ePlotTeam = pPlot->getTeam();
			if (ePlotTeam != NO_TEAM)
			{
				WarPlanTypes eWarplan = GET_TEAM(getTeam()).AI_getWarPlan(ePlotTeam);
				if (eWarplan == WARPLAN_LIMITED)
				{
					bLimitedWar = true;
				}
			}
		}

		CvUnit* pHeadUnit = getHeadUnit();

		if (pHeadUnit != NULL)
		{
			switch (pHeadUnit->AI_getUnitAIType())
			{
			case UNITAI_UNKNOWN:
			case UNITAI_ANIMAL:
			case UNITAI_SETTLE:
			case UNITAI_WORKER:
				break;
			case UNITAI_ATTACK_CITY:
			case UNITAI_ATTACK_CITY_LEMMING:
/*************************************************************************************************/
/**	BETTER AI (New UNITAI) Sephi                                 					            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
			case UNITAI_WARWIZARD:
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
				return true;
				break;

			case UNITAI_ATTACK:
			case UNITAI_COLLATERAL:
			case UNITAI_PILLAGE:
				if (bLimitedWar)
				{
					return true;
				}
				break;

			case UNITAI_PARADROP:
			case UNITAI_RESERVE:
			case UNITAI_COUNTER:
			case UNITAI_CITY_DEFENSE:
			case UNITAI_CITY_COUNTER:
			case UNITAI_CITY_SPECIAL:
			case UNITAI_EXPLORE:
			case UNITAI_MISSIONARY:
			case UNITAI_PROPHET:
			case UNITAI_ARTIST:
			case UNITAI_SCIENTIST:
			case UNITAI_GENERAL:
			case UNITAI_MERCHANT:
			case UNITAI_ENGINEER:
			case UNITAI_SPY:
			case UNITAI_ICBM:
			case UNITAI_WORKER_SEA:
/*************************************************************************************************/
/**	WILDMANA (new UnitAI) Sephi                                               					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
            case UNITAI_MANADEFENDER:
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	BETTER AI (New UNITAI) Sephi                                 					            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
			case UNITAI_MAGE:
			case UNITAI_TERRAFORMER:
            case UNITAI_MANA_UPGRADE:
            case UNITAI_BARBSMASHER:
            case UNITAI_HERO:
            case UNITAI_FEASTING:
            case UNITAI_MEDIC:
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
				break;

			case UNITAI_ATTACK_SEA:
			case UNITAI_RESERVE_SEA:
			case UNITAI_ESCORT_SEA:
				if (bLimitedWar)
				{
					return true;
				}
				break;
			case UNITAI_EXPLORE_SEA:
				break;

			case UNITAI_ASSAULT_SEA:
				if (hasCargo())
				{
					return true;
				}
				break;

			case UNITAI_SETTLER_SEA:
			case UNITAI_MISSIONARY_SEA:
			case UNITAI_SPY_SEA:
			case UNITAI_CARRIER_SEA:
			case UNITAI_MISSILE_CARRIER_SEA:
			case UNITAI_PIRATE_SEA:
			case UNITAI_ATTACK_AIR:
			case UNITAI_DEFENSE_AIR:
			case UNITAI_CARRIER_AIR:
			case UNITAI_MISSILE_AIR:
				break;

			default:
				FAssert(false);
				break;
			}
		}
	}

	return false;
}


CvPlot* CvSelectionGroupAI::AI_getMissionAIPlot()
{
	return GC.getMapINLINE().plotSorenINLINE(m_iMissionAIX, m_iMissionAIY);
}


bool CvSelectionGroupAI::AI_isForceSeparate()
{
	return m_bForceSeparate;
}


void CvSelectionGroupAI::AI_makeForceSeparate()
{
	m_bForceSeparate = true;
}


MissionAITypes CvSelectionGroupAI::AI_getMissionAIType()
{
	return m_eMissionAIType;
}


void CvSelectionGroupAI::AI_setMissionAI(MissionAITypes eNewMissionAI, CvPlot* pNewPlot, CvUnit* pNewUnit)
{
	m_eMissionAIType = eNewMissionAI;

	if (pNewPlot != NULL)
	{
		m_iMissionAIX = pNewPlot->getX_INLINE();
		m_iMissionAIY = pNewPlot->getY_INLINE();
	}
	else
	{
		m_iMissionAIX = INVALID_PLOT_COORD;
		m_iMissionAIY = INVALID_PLOT_COORD;
	}

	if (pNewUnit != NULL)
	{
		m_missionAIUnit = pNewUnit->getIDInfo();
	}
	else
	{
		m_missionAIUnit.reset();
	}
}


CvUnit* CvSelectionGroupAI::AI_getMissionAIUnit()
{
	return getUnit(m_missionAIUnit);
}

bool CvSelectionGroupAI::AI_isFull()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (getNumUnits() > 0)
	{
		UnitAITypes eUnitAI = getHeadUnitAI();
		// do two passes, the first pass, we ignore units with speical cargo
		int iSpecialCargoCount = 0;
		int iCargoCount = 0;

		// first pass, count but ignore special cargo units
		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
			{
				if (pLoopUnit->cargoSpace() > 0)
				{
					iCargoCount++;
				}

				if (pLoopUnit->specialCargo() != NO_SPECIALUNIT)
				{
					iSpecialCargoCount++;
				}
				else if (!(pLoopUnit->isFull()))
				{
					return false;
				}
			}
		}

		// if every unit in the group has special cargo, then check those, otherwise, consider ourselves full
		if (iSpecialCargoCount >= iCargoCount)
		{
			pUnitNode = headUnitNode();
			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);

				if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
				{
					if (!(pLoopUnit->isFull()))
					{
						return false;
					}
				}
			}
		}

		return true;
	}

	return false;
}


CvUnit* CvSelectionGroupAI::AI_ejectBestDefender(CvPlot* pDefendPlot)
{
	CLLNode<IDInfo>* pEntityNode;
	CvUnit* pLoopUnit;

	pEntityNode = headUnitNode();

	CvUnit* pBestUnit = NULL;
	int iBestUnitValue = 0;

	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		pEntityNode = nextUnitNode(pEntityNode);

		if (!pLoopUnit->noDefensiveBonus())
		{
			int iValue = pLoopUnit->currEffectiveStr(pDefendPlot, NULL) * 100;

			if (pDefendPlot->isCity(true, getTeam()))
			{
				iValue *= 100 + pLoopUnit->cityDefenseModifier();
				iValue /= 100;
			}

			iValue *= 100;
			iValue /= (100 + pLoopUnit->cityAttackModifier() + pLoopUnit->getExtraCityAttackPercent());

			iValue /= 2 + pLoopUnit->getLevel();

			if (iValue > iBestUnitValue)
			{
				iBestUnitValue = iValue;
				pBestUnit = pLoopUnit;
			}
		}
	}

	if (NULL != pBestUnit && getNumUnits() > 1)
	{
		pBestUnit->joinGroup(NULL);
	}

	return pBestUnit;
}


// Protected Functions...

void CvSelectionGroupAI::read(FDataStreamBase* pStream)
{
	CvSelectionGroup::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iMissionAIX);
	pStream->Read(&m_iMissionAIY);

	pStream->Read(&m_bForceSeparate);

	pStream->Read((int*)&m_eMissionAIType);

	pStream->Read((int*)&m_missionAIUnit.eOwner);
	pStream->Read(&m_missionAIUnit.iID);

	pStream->Read(&m_bGroupAttack);
	pStream->Read(&m_iGroupAttackX);
	pStream->Read(&m_iGroupAttackY);
}


void CvSelectionGroupAI::write(FDataStreamBase* pStream)
{
	CvSelectionGroup::write(pStream);

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iMissionAIX);
	pStream->Write(m_iMissionAIY);

	pStream->Write(m_bForceSeparate);

	pStream->Write(m_eMissionAIType);

	pStream->Write(m_missionAIUnit.eOwner);
	pStream->Write(m_missionAIUnit.iID);

	pStream->Write(m_bGroupAttack);
	pStream->Write(m_iGroupAttackX);
	pStream->Write(m_iGroupAttackY);
}

// Private Functions...

/*************************************************************************************************/
/**	BETTER AI (New Functions) Sephi                                               				**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
int CvSelectionGroupAI::AI_getGroupflag() const
{
	if(getHeadUnit()==NULL)
	{
		return GROUPFLAG_NONE;
	}
	else
	{
		return getHeadUnit()->AI_getGroupflag();
	}
}

void CvSelectionGroupAI::AI_setGroupflag(int newflag)
{
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
    while (pUnitNode != NULL)
    {
        CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
        pUnitNode = nextUnitNode(pUnitNode);
        pLoopUnit->AI_setGroupflag(newflag);
    }
}

bool CvSelectionGroupAI::isHeadUnit(CvUnit* pUnit)
{
    if (getHeadUnit()==pUnit)
    {
        return true;
    }
    return false;
}
// uses the same scale as CvPlayerAI::AI_getOurPlotStrength
int CvSelectionGroupAI::AI_GroupPower(CvPlot* pPlot, bool bDefensiveBonuses) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iValue;

	iValue = 0;

	pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((bDefensiveBonuses && pLoopUnit->canDefend()) || pLoopUnit->canAttack())
		{
			if (!(pLoopUnit->isInvisible(getTeam(), false)))
			{
			    if (pLoopUnit->atPlot(pPlot) || pLoopUnit->canMoveInto(pPlot) || pLoopUnit->canMoveInto(pPlot, /*bAttack*/ true))
			    {
                    iValue += pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pPlot : NULL), NULL);
				}
			}
		}
	}
	return iValue;
}

/**	ADDON (Ranged Combat) Sephi								                     				**/
void CvSelectionGroupAI::AI_GroupRangeStrike()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if(pLoopUnit->canRangeStrike(false))
		{
			pLoopUnit->AI_rangeAttack(pLoopUnit->airRange());
		}
	}
}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/02/09                                jdog5000      */
/*                                                                                              */
/* General AI                                                                                   */
/************************************************************************************************/
//
// Approximate how many turns this group would take to reduce pCity's defense modifier to zero
//
int CvSelectionGroup::getBombardTurns(CvCity* pCity)
{
	PROFILE_FUNC();

	bool bHasBomber = (area()->getNumAIUnits(getOwner(),UNITAI_ATTACK_AIR) > 0);
	bool bIgnoreBuildingDefense = bHasBomber;
	int iTotalBombardRate = (bHasBomber ? 16 : 0);

	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if( pLoopUnit->bombardRate() > 0 )
		{
			iTotalBombardRate += pLoopUnit->bombardRate();
			bIgnoreBuildingDefense = (bIgnoreBuildingDefense || pLoopUnit->ignoreBuildingDefense());
		}
	}

	int iBombardTurns = pCity->getDefenseModifier(bIgnoreBuildingDefense);
	if( !bIgnoreBuildingDefense )
	{
		iBombardTurns *= 100;
		iBombardTurns /= std::max(25, (100 - pCity->getBuildingBombardDefense()));
	}
	iBombardTurns /= std::max(8, iTotalBombardRate);

	// added Sephi
	if (iTotalBombardRate==0)
	{
		return -1;
	}

	return iBombardTurns;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
