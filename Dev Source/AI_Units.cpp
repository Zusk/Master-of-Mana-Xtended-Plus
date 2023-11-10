// unitAI.cpp

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


// AI_update returns true when we should abort the loop and wait until next slice
bool CvUnitAI::AI_update_main()
{
	AI_update_debug();
	if(AI_update_specialCases())
	{
		return false;
	}
    if(AI_update_AIControl())
    {
		return false;
	}
	if(AI_update_Automated())
	{
		return false;
	}

/*************************************************************************************************/
/**	BETTER AI (UnitAI::AI_update) Sephi                                 	    				**/
/*************************************************************************************************/
    if (!isBarbarian())
    {
		switch (AI_getUnitAIType())
		{			
			case UNITAI_WORKER:
			case UNITAI_WORKER_SEA:
			case UNITAI_PROPHET:
			case UNITAI_MISSIONARY:
			case UNITAI_ARTIST:
			case UNITAI_SCIENTIST:
			case UNITAI_MERCHANT:
			case UNITAI_ENGINEER:
				break;
			case UNITAI_ATTACK_SEA:
			case UNITAI_RESERVE_SEA:
			case UNITAI_ESCORT_SEA:
			case UNITAI_EXPLORE_SEA:
			case UNITAI_ASSAULT_SEA:
			case UNITAI_SETTLER_SEA:
			case UNITAI_MISSIONARY_SEA:
			case UNITAI_SPY_SEA:
			case UNITAI_CARRIER_SEA:
			case UNITAI_MISSILE_CARRIER_SEA:
			case UNITAI_PIRATE_SEA:
				AIGroup_Reserve_naval_join(getArea());
                getGroup()->pushMission(MISSION_SKIP);
				return false;
			default:
				AIGroup_Reserve_join(getArea());
                getGroup()->pushMission(MISSION_SKIP);
				return false;
		}
	}

    if (!isBarbarian())
    {
        switch (AI_getGroupflag())
        {
            case GROUPFLAG_CONQUEST:
                FAssertMsg(false,"GROUPFLAG shouldn't be called this way");
                return false;
                break;
            case GROUPFLAG_PERMDEFENSE:
                PermDefenseMove();
                return false;
                break;
            case GROUPFLAG_PERMDEFENSE_NEW:
                PermDefenseNewMove();
                return false;
                break;
            case GROUPFLAG_EXPLORE:
                AI_exploreMove();
                return false;
                break;
            case GROUPFLAG_DEFENSE_NEW:
                AI_barbsmashermove();
                return false;
                break;
            case GROUPFLAG_HERO:
                AI_heromove();
                return false;
                break;
            case GROUPFLAG_SETTLERGROUP:
                SettlergroupMove();
                return false;
                break;
            case GROUPFLAG_HNGROUP:
                HNgroupMove();
                return false;
			case GROUPFLAG_PICKUP_EQUIPMENT:
				PickupEquipmentMove();
				return false;
            case GROUPFLAG_FEASTING:
                AI_feastingmove();
                return false;
            case GROUPFLAG_NAVAL_SETTLE_NEW:
				AI_SettleNavalNewMove();
                return false;
            case GROUPFLAG_NAVAL_SETTLE:
                AI_SettleNavalMove();
                return false;
            case GROUPFLAG_NAVAL_SETTLE_PICKUP:
                AI_SettleNavalPickupMove();
                return false;
            case GROUPFLAG_SUICIDE_SUMMON:
                AI_summonAttackMove();
                return false;
            case GROUPFLAG_AWAKENED:
                AI_AwakenedMove();
                return false;
            case GROUPFLAG_CREEPER:
                AI_CreeperMove();
                return false;
            case GROUPFLAG_ANIMAL_CONSTRUCTBUILDING:
                if(AI_construct(10000))
                {
                    return false;
                }
                break;
			case GROUPFLAG_SVARTALFAR_KIDNAP:
				AI_SvartalfarKidnapMove();
				break;
			case GROUPFLAG_ESUS_MISSIONARY:
				AI_EsusMissionaryMove();
				break;
            case GROUPFLAG_PILLAGE:
            	AI_PillageGroupMove();
            	break;
            default:
                break;
        }

        if (isSuicideSummon())
        {
            AI_summonAttackMove();
            setSuicideSummon(false);
            return false;
        }
        switch (AI_getUnitAIType())
        {
            case UNITAI_UNKNOWN:
            case UNITAI_ANIMAL:
            case UNITAI_ATTACK:
            case UNITAI_WARWIZARD:
            case UNITAI_BARBSMASHER:
            case UNITAI_ATTACK_CITY:
            case UNITAI_COLLATERAL:
            case UNITAI_PILLAGE:
            case UNITAI_RESERVE:
            case UNITAI_COUNTER:
            case UNITAI_CITY_DEFENSE:
            case UNITAI_CITY_COUNTER:
                getGroup()->pushMission(MISSION_SKIP);
                return false;
                break;
            default:
                break;
        }
    }

	switch (AI_getUnitAIType())
	{
	case UNITAI_UNKNOWN:
		getGroup()->pushMission(MISSION_SKIP);
		break;

	case UNITAI_ANIMAL:
		AI_animalMove();
		break;

	case UNITAI_SETTLE:
		AI_settleMove();
		break;

	case UNITAI_WORKER:
		AI_workerMove();
		break;

	case UNITAI_ATTACK:

//Added by Kael 09/19/2007
        if (getDuration() > 0)
        {
            AI_summonAttackMove();
            break;
        }
//FfH: End Add

		if (isBarbarian())
		{
			AI_barbAttackMove();
		}
		else
		{
			AI_attackMove();
		}
		break;
	case UNITAI_MANADEFENDER:
		AI_manadefendermove();
		break;
	case UNITAI_ACHERON:
		AI_acheronMove();
		break;
	case UNITAI_ANIMALDEFENDER:
		AI_animalDefenderMove();
		break;
    case UNITAI_BARBSMASHER:
        AI_barbsmashermove();
        break;
    case UNITAI_HERO:
		if (isBarbarian())
		{
			AI_barbAttackMove();
		}
		else
		{
            AI_heromove();
		}
        break;
    case UNITAI_FEASTING:
        AI_feastingmove();
    case UNITAI_MANA_UPGRADE:
        AI_upgrademanaMove();
        break;
	case UNITAI_ATTACK_CITY:
		if (isBarbarian())
		{
			AI_barbAttackMove();
		}
		else
		{
            AI_attackCityMove();
		}
		break;
	case UNITAI_COLLATERAL:
		AI_collateralMove();
		break;

	case UNITAI_PILLAGE:
		AI_pillageMove();
		break;

	case UNITAI_RESERVE:
		AI_reserveMove();
		break;

    case UNITAI_MEDIC:
	case UNITAI_COUNTER:
		AI_counterMove();
		break;

	case UNITAI_CITY_DEFENSE:
		AI_cityDefenseMove();
		break;

	case UNITAI_CITY_COUNTER:
	case UNITAI_CITY_SPECIAL:
		AI_cityDefenseExtraMove();
		break;

	case UNITAI_EXPLORE:
		AI_exploreMove();
		break;

	case UNITAI_MISSIONARY:
		AI_missionaryMove();
		break;

	case UNITAI_PROPHET:
		AI_prophetMove();
		break;

	case UNITAI_ARTIST:
		AI_artistMove();
		break;

	case UNITAI_SCIENTIST:
		AI_scientistMove();
		break;

	case UNITAI_GENERAL:
		AI_generalMove();
		break;

	case UNITAI_MERCHANT:
		AI_merchantMove();
		break;

	case UNITAI_ENGINEER:
		AI_engineerMove();
		break;

	case UNITAI_SPY:
		AI_spyMove();
		break;

	case UNITAI_WORKER_SEA:
		AI_workerSeaMove();
		break;

	case UNITAI_ATTACK_SEA:
		if (isBarbarian())
		{
			AI_barbAttackSeaMove();
		}
		else
		{
			AI_attackSeaMove();
		}
		break;

	case UNITAI_RESERVE_SEA:
		AI_reserveSeaMove();
		break;

	case UNITAI_ESCORT_SEA:
		AI_escortSeaMove();
		break;

	case UNITAI_EXPLORE_SEA:
		AI_exploreSeaMove();
		break;

	case UNITAI_ASSAULT_SEA:
		AI_assaultSeaMove();
		break;

	case UNITAI_SETTLER_SEA:
		AI_settlerSeaMove();
		break;

	case UNITAI_MISSIONARY_SEA:
		AI_missionarySeaMove();
		break;

	case UNITAI_SPY_SEA:
		AI_spySeaMove();
		break;

	case UNITAI_PIRATE_SEA:
		AI_pirateSeaMove();
		break;

	case UNITAI_ATTACK_AIR:
		AI_attackAirMove();
		break;

	case UNITAI_DEFENSE_AIR:
		AI_defenseAirMove();
		break;

	case UNITAI_ATTACK_CITY_LEMMING:
		AI_attackCityLemmingMove();
		break;

	default:
		FAssert(false);
		break;
	}
	return false;
}

void CvUnitAI::AI_update_debug()
{
	//DEBUG
    if (GC.getDefineINT("USE_PERFORMANCE_LOG_LEVEL4")==1)
    {
        CvString szError="start CvUnitAI::AI_update()";
		gDLL->logMsg("pdebug.log", szError);

		TCHAR szOut[1024];
		int CombatType=this->getUnitCombatType();
		if (CombatType==-1)
		{
            sprintf(szOut, "UnitCombatType -- %d, %S\n",CombatType,"NO_UNITCOMBAT");
            gDLL->logMsg("pdebug.log",szOut);
		}
		else
		{
            sprintf(szOut, "UnitCombatType -- %d, %S\n",CombatType,GC.getUnitCombatInfo(this->getUnitCombatType()).getDescription());
            gDLL->logMsg("pdebug.log",szOut);
		}
		if (getUnitType()!=NO_UNIT)
		{
			sprintf(szOut, "UnitType -- %S\n",GC.getUnitInfo(this->getUnitType()).getDescription());
			gDLL->logMsg("pdebug.log",szOut);
		}
		if (AI_getUnitAIType()!=NO_UNITAI)
		{
			sprintf(szOut, "UnitAIType -- %S\n",GC.getUnitAIInfo(this->AI_getUnitAIType()).getDescription());
			gDLL->logMsg("pdebug.log",szOut);
		}
        sprintf(szOut, "GetX: %d GetY: %d\n",this->getX(), this->getY());
		gDLL->logMsg("pdebug.log",szOut);
        sprintf(szOut, "UnitID: %d, GroupID: %d, Groupsize: %d\n",this->getID(), this->getGroupID(), this->getGroup()->getNumUnits());
		gDLL->logMsg("pdebug.log",szOut);
    }
}

bool CvUnitAI::AI_update_specialCases()
{
	CvUnit* pTransportUnit;

	if(plot()->isCity())
	{
		if (getUnitClassType() == GC.getDefineINT("UNITCLASS_FREAK"))
		{
			if (canConstruct(plot(),(BuildingTypes)GC.getDefineINT("BUILDING_FREAK_SHOW")))
			{
				construct((BuildingTypes)GC.getDefineINT("BUILDING_FREAK_SHOW"));
				return true;
			}
		}
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		if (plot()->isWater() && !canMoveAllTerrain())
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			pTransportUnit = getTransportUnit();

			if (pTransportUnit != NULL)
			{
				if (pTransportUnit->getGroup()->hasMoved() || (pTransportUnit->getGroup()->headMissionQueueNode() != NULL))
				{
					getGroup()->pushMission(MISSION_SKIP);
					return true;
				}
			}
		}
	}

	if (AI_afterAttack())
	{
		return true;
	}

    if(getOwnerINLINE()==GC.getMERCENARIES_PLAYER())
    {
        switch (AI_getGroupflag())
        {
            case GROUPFLAG_MERCENARY_HEADHUNTER:
                AI_MercenariesHeadHunterMove();
                return true;
            case GROUPFLAG_MERCENARY_PILLAGER:
                AI_MercenariesPillagerMove();
                return true;
            case GROUPFLAG_MERCENARY_RAIDER:
                AI_MercenariesRaiderMove();
                return true;
            default:
                break;
        }
    }

	return false;
}

bool CvUnitAI::AI_update_AIControl()
{
    if(isAIControl())
    {
		if (getGroup()->getNumUnits()>1)
		{
			joinGroup(NULL);
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		//remove AI control from Defensive only Units
		else if(isOnlyDefensive())
		{
			changeAIControl(-1);
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else if(isAIControl())
		{
			if (AI_anyAttack(3, 0))
			{
				return true;
			}
			if (AI_anyAttack(10, 0))
			{
				return true;
			}

			if (AI_anyAttack(30, 0))
			{
				return true;
			}
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
	}
	return false;
}

bool CvUnitAI::AI_update_Automated()
{
	CvUnit* pTransportUnit;

	if (getGroup()->isAutomated())
	{
		switch (getGroup()->getAutomateType())
		{
		case AUTOMATE_BUILD:
			if (AI_getUnitAIType() == UNITAI_WORKER)
			{
				AI_workerMove();
			}
			else if (AI_getUnitAIType() == UNITAI_WORKER_SEA)
			{
				AI_workerSeaMove();
			}
			else
			{
				FAssert(false);
			}
			break;

		case AUTOMATE_NETWORK:
			AI_networkAutomated();
			// XXX else wake up???
			break;

		case AUTOMATE_CITY:
			AI_cityAutomated();
			// XXX else wake up???
			break;

		case AUTOMATE_EXPLORE:
			switch (getDomainType())
			{
			case DOMAIN_SEA:
				AI_exploreSeaMove();
				break;

			case DOMAIN_AIR:
				// if we are cargo (on a carrier), hold if the carrier is not done moving yet
				pTransportUnit = getTransportUnit();
				if (pTransportUnit != NULL)
				{
					if (pTransportUnit->isAutomated() && pTransportUnit->canMove() && pTransportUnit->getGroup()->getActivityType() != ACTIVITY_HOLD)
					{
						getGroup()->pushMission(MISSION_SKIP);
						break;
					}
				}
				// Have air units explore like AI units do
				AI_exploreAirMove();
				break;

			case DOMAIN_LAND:
				AI_exploreMove();
				break;

			default:
				FAssert(false);
				break;
			}

			// if we have air cargo (we are a carrier), and we done moving, explore with the aircraft as well
			if (hasCargo() && domainCargo() == DOMAIN_AIR && (!canMove() || getGroup()->getActivityType() == ACTIVITY_HOLD))
			{
				std::vector<CvUnit*> aCargoUnits;
				getCargoUnits(aCargoUnits);
				for (uint i = 0; i < aCargoUnits.size() && isAutomated(); ++i)
				{
					CvUnit* pCargoUnit = aCargoUnits[i];
					if (pCargoUnit->getDomainType() == DOMAIN_AIR)
					{
						if (pCargoUnit->canMove())
						{
							pCargoUnit->getGroup()->setAutomateType(AUTOMATE_EXPLORE);
							pCargoUnit->getGroup()->setActivityType(ACTIVITY_AWAKE);
						}
					}
				}
			}
			break;

		case AUTOMATE_RELIGION:
			if (AI_getUnitAIType() == UNITAI_MISSIONARY)
			{
				AI_missionaryMove();
			}
			break;

		default:
			FAssert(false);
			break;
		}

		// if no longer automated, then we want to bail
		return getGroup()->isAutomated();
	}
	return false;
}

void CvUnitAI::AIGroup_Reserve_join(int iArea)
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;
	bool bAdded=false;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_RESERVE && pAIGroup->getMissionArea()==iArea)
		{
			setAIGroup(pAIGroup);
			bAdded=true;
			break;
		}
	}

	if(!bAdded)
	{
		CvAIGroup* pNewGroup=kPlayer.initAIGroup(AIGROUP_RESERVE);
		setAIGroup(pNewGroup);
		pNewGroup->setMissionArea(getArea());
	}
}

void CvUnitAI::AIGroup_Reserve_naval_join(int iArea)
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;
	bool bAdded=false;

	int iNewArea=iArea;

	CvMap &kMap=GC.getMapINLINE();

	if(!kMap.getArea(iArea)->isWater())
	{
		int iBestArea;
		int iBestValue=0;
		int iValue;

		for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if(pAdjacentPlot!=NULL)
			{
				if(pAdjacentPlot->area()->isWater())
				{
					iValue=10000;

					for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
					{
						if(pAIGroup->getGroupType()==AIGROUP_NAVAL_RESERVE && pAIGroup->getMissionArea()==pAdjacentPlot->getArea())
						{
							iValue-=pAIGroup->getNumUnits();
						}
					}

					if(iValue>iBestValue)
					{
						iBestValue=iValue;
						iBestArea=pAdjacentPlot->getArea();
					}
				}
			}
		}
		if(iBestValue>0)
		{
			iNewArea=iBestArea;
		}
	}

	FAssertMsg(kMap.getArea(iNewArea)->isWater(),"Naval is somewhere on land");

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_RESERVE && pAIGroup->getMissionArea()==iNewArea)
		{
			setAIGroup(pAIGroup);
			bAdded=true;
			break;
		}
	}

	if(!bAdded)
	{
		CvAIGroup* pNewGroup=kPlayer.initAIGroup(AIGROUP_NAVAL_RESERVE);
		setAIGroup(pNewGroup);
		pNewGroup->setMissionArea(iNewArea);
	}
}