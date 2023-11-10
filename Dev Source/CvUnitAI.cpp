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

#define FOUND_RANGE				(7)

// Public Functions...

CvUnitAI::CvUnitAI()
{
	AI_reset();
}


CvUnitAI::~CvUnitAI()
{
	AI_uninit();
}


void CvUnitAI::AI_init(UnitAITypes eUnitAI)
{
	AI_reset(eUnitAI);

	//--------------------------------
	// Init other game data
	AI_setBirthmark(GC.getGameINLINE().getSorenRandNum(10000, "AI Unit Birthmark"));

	FAssertMsg(AI_getUnitAIType() != NO_UNITAI, "AI_getUnitAIType() is not expected to be equal with NO_UNITAI");
	area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), 1);
	GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), 1);
}


void CvUnitAI::AI_uninit()
{
}


void CvUnitAI::AI_reset(UnitAITypes eUnitAI)
{
	AI_uninit();

	m_iBirthmark = 0;
/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/*************************************************************************************************/
    m_iGroupflag=GROUPFLAG_NONE;
    m_iOriginX=-1;
    m_iOriginY=-1;
    m_bSuicideSummon=false;
    m_bPermanentSummon=false;

    m_bAllowedPatrol=true;
    m_bAllowedExplore=true;

    m_iWasAttacked=0;
	m_iPromotionSpecialization=-1;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	m_eUnitAIType = eUnitAI;

	m_iAutomatedAbortTurn = -1;
}

// AI_update returns true when we should abort the loop and wait until next slice
bool CvUnitAI::AI_update()
{
	PROFILE_FUNC();

	CvUnit* pTransportUnit;

	FAssertMsg(canMove(), "canMove is expected to be true");
	FAssertMsg(isGroupHead(), "isGroupHead is expected to be true"); // XXX is this a good idea???

/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/*************************************************************************************************/
	return AI_update_main();
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

// allow python to handle it

	CyUnit* pyUnit = new CyUnit(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_unitUpdate", argsList.makeFunctionArgs(), &lResult);
	delete pyUnit;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return false;
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		if (plot()->isWater() && !canMoveAllTerrain())
		{
			getGroup()->pushMission(MISSION_SKIP);
			return false;
		}
		else
		{
			pTransportUnit = getTransportUnit();

			if (pTransportUnit != NULL)
			{
				if (pTransportUnit->getGroup()->hasMoved() || (pTransportUnit->getGroup()->headMissionQueueNode() != NULL))
				{
					getGroup()->pushMission(MISSION_SKIP);
					return false;
				}
			}
		}
	}

	if (AI_afterAttack())
	{
		return false;
	}

//FfH: Added by Kael 10/26/2008
    if (!isBarbarian())
    {
        if (getLevel() < 2)
        {
            bool bDoesBuild = false;
            for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
            {
                BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);
                if (NO_BUILDING != eBuilding)
                {
                    if ((m_pUnitInfo->getForceBuildings(eBuilding)) || (m_pUnitInfo->getBuildings(eBuilding)))
                    {
                        bDoesBuild = true;
                    }
                }
            }
            if (bDoesBuild)
            {
                if (AI_construct())
                {
                    return false;
                }
            }
        }
    }
/*************************************************************************************************/
/**	BUGFIX (also AI Units can become Enraged) Sephi                             				**/
/**																			                    **/
/**	                                                                 							**/
/*************************************************************************************************/
/** FFH code
    if (isHuman())
    {
        if (getGroup()->getHeadUnit()->isAIControl())
        {
            if (AI_anyAttack(3, 20))
            {
                return true;
            }
            AI_barbAttackMove();
        }
    }
**/
    if(isAIControl())
    {
        if (getGroup()->getNumUnits()>1)
        {
            joinGroup(NULL);
            return true;
        }
        //remove AI control from Defensive only Units
        else if(isOnlyDefensive())
        {
            changeAIControl(-1);
            getGroup()->pushMission(MISSION_SKIP);
            return false;
        }
        else if(isAIControl())
        {
            if (AI_anyAttack(3, 0))
            {
                return false;
            }
            if (AI_anyAttack(10, 0))
            {
                return false;
            }

            if (AI_anyAttack(30, 0))
            {
                return false;
            }
            getGroup()->pushMission(MISSION_SKIP);
            return false;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//FfH: End Add

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

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      01/12/09                                jdog5000      */
/*                                                                                              */
/* Player Interface                                                                             */
/************************************************************************************************/
				// Have air units explore like AI units do
				AI_exploreAirMove();
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

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

/*************************************************************************************************/
/**	ADDON (automatic Terraforming) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
		case AUTOMATE_FREE_IMPROVEMENT:
			AI_workerSearchForFreeImprovementMove();
			break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

		default:
			FAssert(false);
			break;
		}

		// if no longer automated, then we want to bail
		return !getGroup()->isAutomated();
	}
	else
	{
/*************************************************************************************************/
/**	BETTER AI (UnitAI::AI_update) Sephi                                 	    				**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

        if (!isBarbarian())
	    {
            switch (AI_getGroupflag())
            {
                case GROUPFLAG_CONQUEST:
                    //ConquestMove();
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

/**	ADDON (Mercenaries) Sephi	        								                    	**/

	    if(getOwnerINLINE()==GC.getMERCENARIES_PLAYER())
	    {
            switch (AI_getGroupflag())
            {
                case GROUPFLAG_MERCENARY_HEADHUNTER:
                    AI_MercenariesHeadHunterMove();
                    return false;
                case GROUPFLAG_MERCENARY_PILLAGER:
                    AI_MercenariesPillagerMove();
                    return false;
                case GROUPFLAG_MERCENARY_RAIDER:
                    AI_MercenariesRaiderMove();
                    return false;
                default:
                    break;
            }
	    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
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
/*************************************************************************************************/
/**	WILDMANA (new UnitAI) Sephi                                               					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    	case UNITAI_MANADEFENDER:
			AI_manadefendermove();
			break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	BETTER AI (New UNITAI) Sephi                                 					            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
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
/** orig
		case UNITAI_ATTACK_CITY:
			AI_attackCityMove();
			break;
**/
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
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

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

		case UNITAI_PARADROP:
			AI_paratrooperMove();
			break;

		case UNITAI_CITY_DEFENSE:
			if (isBarbarian())
			{
				AI_barbAttackMove();
			}
			else
			{
				AI_cityDefenseMove();
			}
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

		case UNITAI_ICBM:
			AI_ICBMMove();
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

		case UNITAI_CARRIER_SEA:
			AI_carrierSeaMove();
			break;

		case UNITAI_MISSILE_CARRIER_SEA:
			AI_missileCarrierSeaMove();
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

		case UNITAI_CARRIER_AIR:
			AI_carrierAirMove();
			break;

		case UNITAI_MISSILE_AIR:
			AI_missileAirMove();
			break;

		case UNITAI_ATTACK_CITY_LEMMING:
			AI_attackCityLemmingMove();
			break;

		default:
			FAssert(false);
			break;
		}
	}

	return false;
}


// Returns true if took an action or should wait to move later...
bool CvUnitAI::AI_follow()
{
	if (AI_followBombard())
	{
		return true;
	}

	if (AI_cityAttack(1, 65, true))
	{
		return true;
	}

	if (isEnemy(plot()->getTeam()))
	{
		if (canPillage(plot()))
		{
			getGroup()->pushMission(MISSION_PILLAGE);
			return true;
		}
	}

	if (AI_anyAttack(1, 70, 2, true))
	{
		return true;
	}

	if (isFound())
	{
		if (area()->getBestFoundValue(getOwnerINLINE()) > 0)
		{
			if (AI_foundRange(FOUND_RANGE, true))
			{
				return true;
			}
		}
	}

	return false;
}


// XXX what if a unit gets stuck b/c of it's UnitAIType???
// XXX is this function costing us a lot? (it's recursive...)
bool CvUnitAI::AI_upgrade()
{
	PROFILE_FUNC();

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(AI_getUnitAIType() != NO_UNITAI, "AI_getUnitAIType() is not expected to be equal with NO_UNITAI");

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	UnitAITypes eUnitAI = AI_getUnitAIType();
	CvArea* pArea = area();

	int iCurrentValue = kPlayer.AI_unitValue(getUnitType(), eUnitAI, pArea);

	for (int iPass = 0; iPass < 2; iPass++)
	{
		int iBestValue = 0;
		UnitTypes eBestUnit = NO_UNIT;

/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
/** Orig
 		for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
		{
			if ((iPass > 0) || GC.getUnitInfo((UnitTypes)iI).getUnitAIType(AI_getUnitAIType()))
			{
				int iNewValue = kPlayer.AI_unitValue(((UnitTypes)iI), eUnitAI, pArea);
				if ((iPass == 0 || iNewValue > 0) && iNewValue > iCurrentValue)
				{
					if (canUpgrade((UnitTypes)iI))
					{
						int iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Upgrade"));

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestUnit = ((UnitTypes)iI);
						}
					}
				}
			}
		}
**/
		std::vector<int> aPotentialUnitClassTypes = GC.getUnitInfo(getUnitType()).getUpgradeUnitClassTypes();
		for (int iI = 0; iI < (int)aPotentialUnitClassTypes.size(); iI++)
		{
			UnitTypes eUnit = (UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits((UnitClassTypes)aPotentialUnitClassTypes[iI]);
			if (eUnit != NO_UNIT && ((iPass > 0) || GC.getUnitInfo(eUnit).getUnitAIType(AI_getUnitAIType())))
			{
				int iNewValue = kPlayer.AI_unitValue(eUnit, eUnitAI, pArea);
				if ((iPass == 0 || iNewValue > 0) && iNewValue > iCurrentValue)
				{
					if (canUpgrade(eUnit))
					{
						int iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Upgrade"));

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestUnit = eUnit;
						}
					}
				}
			}
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


		if (eBestUnit != NO_UNIT)
		{
			if (upgrade(eBestUnit))
			{
				doDelayedDeath();
				return true;
			}
			return false;
		}
	}
	return false;
}


void CvUnitAI::AI_promote()
{
	PROFILE_FUNC();

	PromotionTypes eBestPromotion;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	eBestPromotion = NO_PROMOTION;


/*************************************************************************************************/
/**	BETTER AI (Better Promotion selection) Sephi                               					**/
/**	values UNITAI and GROUPFLAG much higher when selecting Promotions							**/
/**						                                            							**/
/*************************************************************************************************/

	//if we have no Specialization, choose one via python (only Barbarians)
	if(getAIGroup()==NULL || isBarbarian())
	{
		if (AI_getPromotionSpecialization()==-1)
		{
			CyUnit* pyUnit = new CyUnit(this);
			CyArgsList argsList;
			argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
			long lResult=-1;
			gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_ChoosePromotionSpecialization", argsList.makeFunctionArgs(), &lResult);
			delete pyUnit;	// python fxn must not hold on to this pointer
		}
	}

	//temporary
	if(AI_getUnitAIType()==UNITAI_WARWIZARD)
	{
		AI_setPromotionSpecialization(0);
	}

	//even more temporary
	if (AI_getPromotionSpecialization()==-1)
	{
		AI_setPromotionSpecialization(0);
	}

	if (AI_getPromotionSpecialization()==-1)
	{
		return;
	}

	int iTemp;

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		iValue = 0;

		if (canPromote((PromotionTypes)iI, -1))
		{
		    //Combat Promotions
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getCombatPromotions()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getCombatPromotions();

				iValue += (iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getCombatPercent();
				iValue += (iTemp/4)*GC.getPromotionInfo((PromotionTypes)iI).getBonusVSUnitsFullHealth();
				for(int iJ=0;iJ<GC.getNumUnitCombatInfos();iJ++)
				{
					if(GC.getPromotionInfo((PromotionTypes)iI).getUnitCombatModifierPercent(iJ)>0)
					{
						iValue +=(iTemp*GET_PLAYER(getOwnerINLINE()).AI_getUnitCombatWeight((UnitCombatTypes)iJ)*GC.getPromotionInfo((PromotionTypes)iI).getUnitCombatModifierPercent(iJ))/100;
					}
				}
			}

			//City Defense
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getCityDefense()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getCityDefense();

			    iValue += (iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getCityDefensePercent();

			    if (plot()->isHills())
			    {
                    iValue += (iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getHillsDefensePercent();
			    }

                if (plot()->getTerrainType()!=NO_TERRAIN)
                    iValue +=GC.getPromotionInfo((PromotionTypes)iI).getTerrainDefensePercent(plot()->getTerrainType());
                if (plot()->getFeatureType()!=NO_FEATURE)
                    iValue +=GC.getPromotionInfo((PromotionTypes)iI).getFeatureDefensePercent(plot()->getFeatureType());

			}

			//City Attack
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getCityAttack()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getCityAttack();

			    iValue += (iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getCityAttackPercent();
			}

			//DefensiveStrikes
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getDefensiveStrikes()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getDefensiveStrikes();

			    iValue +=(iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getDefensiveStrikeChance();
			    iValue +=(iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getDefensiveStrikeDamage();
			}

            //FirstStrikes
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getFirstStrikes()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getFirstStrikes();

			    iValue +=iTemp*15*GC.getPromotionInfo((PromotionTypes)iI).getChanceFirstStrikesChange();
			    iValue +=iTemp*20*GC.getPromotionInfo((PromotionTypes)iI).getFirstStrikesChange();
			}

			//Guardsman
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getGuardsman()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getGuardsman();

                if (GC.getPromotionInfo((PromotionTypes)iI).isTargetWeakestUnitCounter())
                {
                    iValue +=iTemp*3/2;
                }
			}

            //Hunter
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getHunter()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getHunter();

                if (GC.getPromotionInfo((PromotionTypes)iI).getCaptureUnitCombat()!=NO_UNITCOMBAT)
                {
                    iValue +=iTemp*3/2;
                }
			}

            //Landscape
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getLandscape()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getLandscape();

                for (int i=0;i<GC.getNumTerrainInfos();i++)
                {
                    iValue+=(iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getTerrainAttackPercent(i);
                    iValue+=(iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getTerrainDefensePercent(i);
                }

			    if (plot()->isHills())
			    {
                    iValue += (iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getHillsDefensePercent();
			    }

                for (int i=0;i<GC.getNumFeatureInfos();i++)
                {
                    iValue+=(iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getFeatureAttackPercent(i);
                    iValue+=(iTemp/2)*GC.getPromotionInfo((PromotionTypes)iI).getFeatureDefensePercent(i);
                }
			}

            //Mobility
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getMobility()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getMobility();

			    iValue +=(iTemp*3/2)*GC.getPromotionInfo((PromotionTypes)iI).getMovesChange();
			}

            //Sight
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getSight()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getSight();

                if (GC.getPromotionInfo((PromotionTypes)iI).isSeeInvisible())
                {
                    iValue +=(iTemp*3/2);
                }

			    iValue +=(iTemp*3/2)*GC.getPromotionInfo((PromotionTypes)iI).getVisibilityChange();
			}


            //Withdraw
			if (GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getWithdraw()>0)
			{
			    iTemp=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getWithdraw();

			    iValue +=(iTemp*2/3)*GC.getPromotionInfo((PromotionTypes)iI).getWithdrawalChange();

            }

			//magic value - add special tag for this?
			if(GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraType()!=NO_COMBATAURA)
			{
				if(getCombatAura()==NULL)
				{
					iValue+=100;
				}
			}

			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraCityAttackChange();
			iValue+=100*GC.getPromotionInfo((PromotionTypes)iI).getSpellDamageModify();
			iValue+=100*GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraEffect();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraExperienceChange();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraFearEffectChange();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraHealthRegenerationChange();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraStrengthChange();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraIgnoreFirstStrikes();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraIncreaseFirstStrikes();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraRange();
			iValue+=GC.getPromotionInfo((PromotionTypes)iI).getCombatAuraTargets();

			//siege - add Special tag for it some day
			iValue+=100*GC.getPromotionInfo((PromotionTypes)iI).getBombardRateChange();

			//misc modifiers
			iValue+=GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)AI_getPromotionSpecialization()).getPromotionValue((PromotionTypes)iI);

			//TODO: make promotion selection mucho better
			iValue += GC.getGameINLINE().getSorenRandNum(100, "promotion pick");

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestPromotion = ((PromotionTypes)iI);
			}
		}
	}

	if (eBestPromotion != NO_PROMOTION)
	{
		promote(eBestPromotion, -1);
		AI_promote();
	}
	return;



    if (AI_promoteMagicUnit())
    {
        return;
    }

    if (!isBarbarian())
    {
        switch (AI_getGroupflag())
        {
            case GROUPFLAG_PERMDEFENSE:
                if (1<2)
                {
                    PromotionTypes eBestPromotion;
                    eBestPromotion = NO_PROMOTION;
                    CyUnit* pyUnit = new CyUnit(this);
                    CyArgsList argsList;
                    argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                    long lResult=-1;
                    gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_PermDefensePromotion", argsList.makeFunctionArgs(), &lResult);
                    delete pyUnit;	// python fxn must not hold on to this pointer

                    eBestPromotion = ((PromotionTypes)lResult);

                    if (eBestPromotion != NO_PROMOTION)
                    {
                        promote(eBestPromotion, -1);
                        AI_promote();
                    }
                }
                return;
                break;

            case GROUPFLAG_PATROL:
            case GROUPFLAG_DEFENSE_NEW:
                if (1<2)
                {
                    PromotionTypes eBestPromotion;
                    eBestPromotion = NO_PROMOTION;
                    CyUnit* pyUnit = new CyUnit(this);
                    CyArgsList argsList;
                    argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                    long lResult=-1;
                    gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_PatrolPromotion", argsList.makeFunctionArgs(), &lResult);
                    delete pyUnit;	// python fxn must not hold on to this pointer

                    eBestPromotion = ((PromotionTypes)lResult);

                    if (eBestPromotion != NO_PROMOTION)
                    {
                        promote(eBestPromotion, -1);
                        AI_promote();
                    }
                }
                return;
                break;

                case GROUPFLAG_CONQUEST:
                if (1<2)
                {
                    PromotionTypes eBestPromotion;
                    eBestPromotion = NO_PROMOTION;
                    CyUnit* pyUnit = new CyUnit(this);
                    CyArgsList argsList;
                    argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                    long lResult=-1;
                    gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_ConquestPromotion", argsList.makeFunctionArgs(), &lResult);
                    delete pyUnit;	// python fxn must not hold on to this pointer

                    eBestPromotion = ((PromotionTypes)lResult);

                    if (eBestPromotion != NO_PROMOTION)
                    {
                        promote(eBestPromotion, -1);
                        AI_promote();
                    }
                    return;
                }

            case GROUPFLAG_NONE:
            case GROUPFLAG_PERMDEFENSE_NEW:
            case GROUPFLAG_PATROL_NEW:
                return;
                break;
            default:
                break;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (canPromote((PromotionTypes)iI, -1))
		{
			iValue = AI_promotionValue((PromotionTypes)iI);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestPromotion = ((PromotionTypes)iI);
			}
		}
	}

	if (eBestPromotion != NO_PROMOTION)
	{
		promote(eBestPromotion, -1);
		AI_promote();
	}

}


int CvUnitAI::AI_groupFirstVal()
{
/*************************************************************************************************/
/**	BETTER AI (improved logic which unit becomes head of a group) Sephi                        	**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	if (getDuration()>0)
	{
	    return 1;
	}

	if (AI_getGroupflag()==GROUPFLAG_CONQUEST)
	{
	    return 25;
	}
/** Wildmana    **/
	int iWildmanaDefender=GC.getInfoTypeForString("PROMOTION_MANA_DEFENDER");

	if (isHasPromotion((PromotionTypes)iWildmanaDefender))
	{
	    return 100;
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	switch (AI_getUnitAIType())
	{
	case UNITAI_UNKNOWN:
	case UNITAI_ANIMAL:
		FAssert(AI_getUnitAIType()==UNITAI_ANIMAL);	//modified Sephi
		break;

	case UNITAI_SETTLE:
		return 21;
		break;

	case UNITAI_WORKER:
		return 20;
		break;

	case UNITAI_ATTACK:
		if (collateralDamage() > 0)
		{
			return 17;
		}
		else if (withdrawalProbability() > 0)
		{
			return 15;
		}
		else
		{
			return 13;
		}
		break;

	case UNITAI_ATTACK_CITY:
		if (bombardRate() > 0)
		{
			return 19;
		}
		else if (collateralDamage() > 0)
		{
			return 18;
		}
		else if (withdrawalProbability() > 0)
		{
			return 16;
		}
		else
		{
			return 14;
		}
		break;

	case UNITAI_COLLATERAL:
		return 7;
		break;

	case UNITAI_PILLAGE:
		return 12;
		break;

	case UNITAI_RESERVE:
		return 6;
		break;

	case UNITAI_COUNTER:
/*************************************************************************************************/
/**	WILDMANA (new UnitAI) Sephi                                               					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    case UNITAI_MANADEFENDER:
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
		return 5;
		break;

	case UNITAI_CITY_DEFENSE:
		return 3;
		break;

	case UNITAI_CITY_COUNTER:
		return 2;
		break;

	case UNITAI_CITY_SPECIAL:
/*************************************************************************************************/
/**	BETTER AI (New UNITAI) Sephi                                 					            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	case UNITAI_MAGE:
    case UNITAI_TERRAFORMER:
    case UNITAI_MANA_UPGRADE:
	case UNITAI_WARWIZARD:
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
		return 3;
		break;

/*************************************************************************************************/
/**	BETTER AI (New UNITAI) Sephi                                 					            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    case UNITAI_BARBSMASHER:
    case UNITAI_FEASTING:
    case UNITAI_MEDIC:
		return 3;
		break;
    case UNITAI_HERO:
        return 100; //Heroes don't like to get pushed around
        break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	case UNITAI_PARADROP:
		return 4;
		break;

	case UNITAI_EXPLORE:
		return 8;
		break;

	case UNITAI_MISSIONARY:
		return 10;
		break;

	case UNITAI_PROPHET:
	case UNITAI_ARTIST:
	case UNITAI_SCIENTIST:
	case UNITAI_GENERAL:
	case UNITAI_MERCHANT:
	case UNITAI_ENGINEER:
		return 11;
		break;

	case UNITAI_SPY:
		return 9;
		break;

	case UNITAI_ICBM:
		break;

	case UNITAI_WORKER_SEA:
		return 8;
		break;

	case UNITAI_ATTACK_SEA:
		return 3;
		break;

	case UNITAI_RESERVE_SEA:
		return 2;
		break;

	case UNITAI_ESCORT_SEA:
		return 1;
		break;

	case UNITAI_EXPLORE_SEA:
		return 5;
		break;

	case UNITAI_ASSAULT_SEA:
		return 11;
		break;

	case UNITAI_SETTLER_SEA:
		return 9;
		break;

	case UNITAI_MISSIONARY_SEA:
		return 9;
		break;

	case UNITAI_SPY_SEA:
		return 10;
		break;

	case UNITAI_CARRIER_SEA:
		return 7;
		break;

	case UNITAI_MISSILE_CARRIER_SEA:
		return 6;
		break;

	case UNITAI_PIRATE_SEA:
		return 4;
		break;

	case UNITAI_ATTACK_AIR:
	case UNITAI_DEFENSE_AIR:
	case UNITAI_CARRIER_AIR:
	case UNITAI_MISSILE_AIR:
		break;

	case UNITAI_ATTACK_CITY_LEMMING:
		return 1;
		break;

	default:
		FAssert(false);
		break;
	}

	return 0;
}


int CvUnitAI::AI_groupSecondVal()
{
	return ((getDomainType() == DOMAIN_AIR) ? airBaseCombatStr() : baseCombatStr());
}


// Returns attack odds out of 100 (the higher, the better...)
// Withdrawal odds included in returned value
int CvUnitAI::AI_attackOdds(const CvPlot* pPlot, bool bPotentialEnemy) const
{
	CvUnit* pDefender;
	int iOurStrength;
	int iTheirStrength;
	int iOurFirepower;
	int iTheirFirepower;
	int iBaseOdds;
	int iStrengthFactor;
	int iDamageToUs;
	int iDamageToThem;
	int iNeededRoundsUs;
	int iNeededRoundsThem;
	int iHitLimitThem;

	pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, !bPotentialEnemy, bPotentialEnemy);

	if (pDefender == NULL)
	{
		return 100;
	}

	iOurStrength = ((getDomainType() == DOMAIN_AIR) ? airCurrCombatStr(NULL) : currCombatStr(NULL, NULL));
	iOurFirepower = ((getDomainType() == DOMAIN_AIR) ? iOurStrength : currFirepower(NULL, NULL));

	if (iOurStrength == 0)
	{
		return 1;
	}

	iTheirStrength = pDefender->currCombatStr(pPlot, this);
	iTheirFirepower = pDefender->currFirepower(pPlot, this);


	FAssert((iOurStrength + iTheirStrength) > 0);
	FAssert((iOurFirepower + iTheirFirepower) > 0);

	iBaseOdds = (100 * iOurStrength) / (iOurStrength + iTheirStrength);
	if (iBaseOdds == 0)
	{
		return 1;
	}

	iStrengthFactor = ((iOurFirepower + iTheirFirepower + 1) / 2);

	iDamageToUs = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iTheirFirepower + iStrengthFactor)) / (iOurFirepower + iStrengthFactor)));
	iDamageToThem = std::max(1,((GC.getDefineINT("COMBAT_DAMAGE") * (iOurFirepower + iStrengthFactor)) / (iTheirFirepower + iStrengthFactor)));

	iHitLimitThem = pDefender->maxHitPoints() - combatLimit();

	iNeededRoundsUs = (std::max(0, pDefender->currHitPoints() - iHitLimitThem) + iDamageToThem - 1 ) / iDamageToThem;
	iNeededRoundsThem = (std::max(0, currHitPoints()) + iDamageToUs - 1 ) / iDamageToUs;

	if (getDomainType() != DOMAIN_AIR)
	{
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/30/09                      Mongoose & jdog5000     */
/*                                                                                              */
/* Unit AI                                                                                      */
/************************************************************************************************/
		// From Mongoose SDK
		//IgnoreFirstStrikes tag added, Sephi
		if (!pDefender->immuneToFirstStrikes()) {
			iNeededRoundsUs   -= (iBaseOdds * (std::max(0,2*firstStrikes() + chanceFirstStrikes() - 2*pDefender->getIgnoreFirstStrikes()))) / 200;
		}
		if (!immuneToFirstStrikes()) {
			iNeededRoundsThem -= ((100 - iBaseOdds) * (std::max(0,2*pDefender->firstStrikes() + pDefender->chanceFirstStrikes() - 2*getIgnoreFirstStrikes()))) / 200;
		}
		iNeededRoundsUs   = std::max(1, iNeededRoundsUs);
		iNeededRoundsThem = std::max(1, iNeededRoundsThem);
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
	}

	int iRoundsDiff = iNeededRoundsUs - iNeededRoundsThem;
	if (iRoundsDiff > 0)
	{
		iTheirStrength *= (1 + iRoundsDiff);
	}
	else
	{
		iOurStrength *= (1 - iRoundsDiff);
	}

	int iOdds = (((iOurStrength * 100) / (iOurStrength + iTheirStrength)));
	iOdds += ((100 - iOdds) * withdrawalProbability()) / 100;
/*************************************************************************************************/
/**	BETTER AI (attack odds) Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
/** orig
	iOdds += GET_PLAYER(getOwnerINLINE()).AI_getAttackOddsChange();
**/
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


	return std::max(1, std::min(iOdds, 99));
}


// Returns true if the unit found a build for this city...
bool CvUnitAI::AI_bestCityBuild(CvCity* pCity, CvPlot** ppBestPlot, BuildTypes* peBestBuild, CvPlot* pIgnorePlot, CvUnit* pUnit)
{
	PROFILE_FUNC();

	BuildTypes eBuild;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	BuildTypes eBestBuild = NO_BUILD;
	CvPlot* pBestPlot = NULL;


	for (int iPass = 0; iPass < 2; iPass++)
	{
		for (iI = 0; iI < pCity->getNumCityPlots(); iI++)
		{
			CvPlot* pLoopPlot = plotCity(pCity->getX_INLINE(), pCity->getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot != pIgnorePlot)
					{
						if ((pLoopPlot->getImprovementType() == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION) && !(pLoopPlot->getImprovementType() == (GC.getDefineINT("RUINS_IMPROVEMENT")))))
						{
							iValue = pCity->AI_getBestBuildValue(iI);

							if (iValue > iBestValue)
							{
								eBuild = pCity->AI_getBestBuild(iI);
								FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

								if (eBuild != NO_BUILD)
								{
									if (0 == iPass)
									{
										iBestValue = iValue;
										pBestPlot = pLoopPlot;
										eBestBuild = eBuild;
									}
									else if (canBuild(pLoopPlot, eBuild))
									{
										if (!(pLoopPlot->isVisibleEnemyUnit(this)))
										{
											int iPathTurns;
											if (generatePath(pLoopPlot, 0, true, &iPathTurns))
											{
												// XXX take advantage of range (warning... this could lead to some units doing nothing...)
												int iMaxWorkers = 1;
												if (getPathLastNode()->m_iData1 == 0)
												{
													iPathTurns++;
												}
												else if (iPathTurns <= 1)
												{
													iMaxWorkers = AI_calculatePlotWorkersNeeded(pLoopPlot, eBuild);
												}
												if (pUnit != NULL)
												{
													if (pUnit->plot()->isCity() && iPathTurns == 1 && getPathLastNode()->m_iData1 > 0)
													{
														iMaxWorkers += 10;
													}
												}
												if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup()) < iMaxWorkers)
												{
													//XXX this could be improved greatly by
													//looking at the real build time and other factors
													//when deciding whether to stack.
													iValue /= iPathTurns;

													iBestValue = iValue;
													pBestPlot = pLoopPlot;
													eBestBuild = eBuild;
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
		}

		if (0 == iPass)
		{
			if (eBestBuild != NO_BUILD)
			{
				FAssert(pBestPlot != NULL);
				int iPathTurns;
				if ((generatePath(pBestPlot, 0, true, &iPathTurns)) && canBuild(pBestPlot, eBestBuild)
					&& !(pBestPlot->isVisibleEnemyUnit(this)))
				{
					int iMaxWorkers = 1;
					if (pUnit != NULL)
					{
						if (pUnit->plot()->isCity())
						{
							iMaxWorkers += 10;
						}
					}
					if (getPathLastNode()->m_iData1 == 0)
					{
						iPathTurns++;
					}
					else if (iPathTurns <= 1)
					{
						iMaxWorkers = AI_calculatePlotWorkersNeeded(pBestPlot, eBestBuild);
					}
					int iWorkerCount = GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pBestPlot, MISSIONAI_BUILD, getGroup());
					if (iWorkerCount < iMaxWorkers)
					{
						//Good to go.
						break;
					}
				}
				eBestBuild = NO_BUILD;
				iBestValue = 0;
			}
		}
	}

	if (NO_BUILD != eBestBuild)
	{
		FAssert(NULL != pBestPlot);
		if (ppBestPlot != NULL)
		{
			*ppBestPlot = pBestPlot;
		}
		if (peBestBuild != NULL)
		{
			*peBestBuild = eBestBuild;
		}
	}


	return (NO_BUILD != eBestBuild);
}


bool CvUnitAI::AI_isCityAIType() const
{
	return ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		      (AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
					(AI_getUnitAIType() == UNITAI_CITY_SPECIAL) ||
						(AI_getUnitAIType() == UNITAI_RESERVE));
}


int CvUnitAI::AI_getBirthmark() const
{
	return m_iBirthmark;
}


void CvUnitAI::AI_setBirthmark(int iNewValue)
{
	m_iBirthmark = iNewValue;
	if (AI_getUnitAIType() == UNITAI_EXPLORE_SEA)
	{
		if (GC.getGame().circumnavigationAvailable())
		{
			m_iBirthmark -= m_iBirthmark % 4;
			int iExplorerCount = GET_PLAYER(getOwnerINLINE()).AI_getNumAIUnits(UNITAI_EXPLORE_SEA);
			iExplorerCount += getOwnerINLINE() % 4;
			if (GC.getMap().isWrapX())
			{
				if ((iExplorerCount % 2) == 1)
				{
					m_iBirthmark += 1;
				}
			}
			if (GC.getMap().isWrapY())
			{
				if (!GC.getMap().isWrapX())
				{
					iExplorerCount *= 2;
				}

				if (((iExplorerCount >> 1) % 2) == 1)
				{
					m_iBirthmark += 2;
				}
			}
		}
	}
}


UnitAITypes CvUnitAI::AI_getUnitAIType() const
{
	return m_eUnitAIType;
}


// XXX make sure this gets called...
void CvUnitAI::AI_setUnitAIType(UnitAITypes eNewValue)
{
	FAssertMsg(eNewValue != NO_UNITAI, "NewValue is not assigned a valid value");

	if (AI_getUnitAIType() != eNewValue)
	{
		area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), -1);
		GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), -1);

		m_eUnitAIType = eNewValue;

		area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), 1);
		GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), 1);

		joinGroup(NULL);
	}
}

int CvUnitAI::AI_sacrificeValue(const CvPlot* pPlot) const
{
    int iValue;
    int iCollateralDamageValue = 0;
    if (pPlot != NULL)
    {
        int iPossibleTargets = std::min((pPlot->getNumVisibleEnemyDefenders(this) - 1), collateralDamageMaxUnits());

        if (iPossibleTargets > 0)
        {
            iCollateralDamageValue = collateralDamage();
            iCollateralDamageValue += std::max(0, iCollateralDamageValue - 100);

// Improved Pyre Zombie AI (Skyre)
			if (getUnitInfo().isExplodeInCombat())
			{
				iCollateralDamageValue += 100;
			}
// End Improved Pyre Zombie AI

            iCollateralDamageValue *= iPossibleTargets;
            iCollateralDamageValue /= 5;
        }
    }

	if (getDomainType() == DOMAIN_AIR)
	{
		iValue = 128 * (100 + currInterceptionProbability());
		if (m_pUnitInfo->getNukeRange() != -1)
		{
			iValue += 25000;
		}
		iValue /= std::max(1, (1 + m_pUnitInfo->getProductionCost()));
		iValue *= (maxHitPoints() - getDamage());
		iValue /= 100;
	}
	else
	{
		iValue  = 128 * (currEffectiveStr(pPlot, ((pPlot == NULL) ? NULL : this)));
		iValue *= (100 + iCollateralDamageValue);
		iValue /= (100 + cityDefenseModifier());
		iValue *= (100 + withdrawalProbability());
		iValue /= std::max(1, (1 + m_pUnitInfo->getProductionCost()));
		iValue /= (10 + getExperience());

		if (m_pUnitInfo->getCombatLimit() < 100)
		{
			iValue *= 150;
			iValue /= 100;
		}
	}

    return iValue;
}

// Protected Functions...

void CvUnitAI::AI_animalMove()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	ADDON (multibarb) merged Sephi		                                                    	**/
/**	                     									**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
	/** TODO: activate Natures Wrath on Beasts
	bool bNatureWrathActive = false;
	for(int iI = 0; iI < GC.getNumProjectInfos(); iI++)
	{
		if(GC.getProjectInfo((ProjectTypes)iI).isNatureWrath())
		{
			for(int iJ=0;iJ<GC.getMAX_CIV_TEAMS(); iJ++)
			{
				if(GET_TEAM((TeamTypes)iJ).isAlive())
				{
					if(GET_TEAM((TeamTypes)iJ).getProjectCount((ProjectTypes)iI) > 0)
					{
						bool bNatureWrathActive = true;
						break;
					}
				}
			}
		}
	}

	if(bNatureWrathActive)
	{
        setBaseCombatStr(baseCombatStr() + 2);
        setBaseCombatStrDefense(baseCombatStrDefense() + 2);
		setAnimalInRage(true);
	}	
	**/
	castAnyTarget(0, true);

	//use special abilities
    int ispell = chooseSpell();
    if (ispell != NO_SPELL)  {
        cast(ispell);

		//quit if we died attacking
		if (isDelayedDeath())
			return;
    }

    if (plot()->isOwned())
    {
		//we are in Owned Land but not in Rage?
		CvPlot* pLoopPlot;
		CvPlot* pBestPlot=NULL;
		int iRange = 5;
		int iDX, iDY;
		int iPathTurns;
		int iBestvalue = 10;

		for (iDX = -(iRange); iDX <= iRange; iDX++)
		{
			for (iDY = -(iRange); iDY <= iRange; iDY++)
			{
				pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

				if (pLoopPlot && pLoopPlot!=plot())
				{
					if(!pLoopPlot->isOwned() && pLoopPlot->getArea() == getArea())
					{
						if(!pLoopPlot->isVisibleEnemyUnit(this))
						{
							if(generatePath(pLoopPlot,0,false,&iPathTurns))
							{
								if(iPathTurns < iBestvalue)
								{
									pBestPlot = pLoopPlot;
									iBestvalue = iPathTurns;
								}
							}
						}
					}
				}
			}
		}

		if (pBestPlot!=NULL)
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return;
		}
    }
    
    if (getDamage()>10)
    {
        if (GC.getGameINLINE().getSorenRandNum(100, "Healing") < 80)
        {
            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
    }


    if ((GC.getGameINLINE().getSorenRandNum(100, "Animal Patrol")<20))
    {
        if (AI_patrol())
        {
            return;
        }
    }

/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
    if (AI_anyAttack(2, 50))  {
        return;
    }

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_settleMove()
{
	PROFILE_FUNC();
/*************************************************************************************************/
/**	BETTER AI (UNITAI_SETTLE move) Sephi                        	                            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

    //build a city after leaving a ship
    if (GET_PLAYER(getOwnerINLINE()).getNumCities() > 1)
    {
        if (GET_PLAYER(getOwnerINLINE()).AI_getExpansionPlot()==plot())
        {
            if(canFound(plot()))
            {
                getGroup()->pushMission(MISSION_FOUND);
                return;
            }
        }
    }

    //reset values after first city is build
	if (GET_PLAYER(getOwnerINLINE()).getNumCities() == 1 && getGroup()->getNumUnits()==1)
	{
        GET_PLAYER(getOwnerINLINE()).AI_updateFoundValues(false);
	}

	if (GET_PLAYER(getOwnerINLINE()).getNumCities() == 0)
	{
	    if (GC.getGameINLINE().getGameTurn()==0)
	    {
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
                    pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                    if (pLoopPlot != NULL)
                    {
                        if ((AI_plotValid(pLoopPlot)) && canFound(pLoopPlot))
                        {
                            if (!pLoopPlot->isVisibleEnemyUnit(this))
                            {
                                if (generatePath(pLoopPlot, 0, true, &iPathTurns))
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
                if(atPlot(pBestPlot))
                {
                    getGroup()->pushMission(MISSION_FOUND);
                    return;
                }
                else
                {
                getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
                // Teleport other units to this plot
                if (GC.getGameINLINE().getGameTurn()<4)
                {
                    CvUnit* pLoopUnit;
                    int iLoop;
                    for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
                    {
                        if (pLoopUnit)
                        {
                            if (pLoopUnit!=this)
                            {
								if(!at(pBestPlot->getX_INLINE(),pBestPlot->getY_INLINE()))
								{
									pLoopUnit->setXY(pBestPlot->getX_INLINE(),pBestPlot->getY_INLINE());
								}
                            }
                        }
                    }
                }
                return;
                }

            }
	    }

        if (canFound(plot()))
        {
            getGroup()->pushMission(MISSION_FOUND);
            return;
        }

        //haven't found a City after 3 turns?
	    if (GC.getGameINLINE().getGameTurn()>3)
	    {
            CvPlot* pLoopPlot;
            CvPlot* pBestPlot;
            int iSearchRange;
            int iPathTurns;
            int iValue;
            int iBestValue;
            int iDX, iDY;

            iSearchRange = 6;
            int iRange = 6;


            iBestValue = 0;
            pBestPlot = NULL;

            for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
            {
                for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
                {
                    pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                    if (pLoopPlot != NULL)
                    {
                        if ((AI_plotValid(pLoopPlot)) && canFound(pLoopPlot))
                        {
                            if (!pLoopPlot->isVisibleEnemyUnit(this))
                            {
                                if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                {
                                    if (iPathTurns<3)
                                    {
                                        iValue = pLoopPlot->getFoundValue(getOwnerINLINE());
                                        if (iValue > iBestValue)
                                        {
                                            iBestValue = iValue;
                                            pBestPlot = pLoopPlot;
                                            FAssert(!atPlot(pBestPlot));
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
                if(atPlot(pBestPlot))
                {
                    getGroup()->pushMission(MISSION_FOUND);
                    return;
                }
                else
                {
                    getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
                    return;
                }
            }
	    }
    }

    if (plot()->isCity() && getGroup()->getNumUnits()<AI_getSettlerPatrolUnitsNeeded())
    {
        if (plot()->getOwnerINLINE()==getOwnerINLINE())
        {
            CLLNode<IDInfo>* pUnitNode;
            CvUnit* pLoopUnit;
            pUnitNode = plot()->headUnitNode();

            while (pUnitNode != NULL)
            {
                pLoopUnit = ::getUnit(pUnitNode->m_data);
                pUnitNode = plot()->nextUnitNode(pUnitNode);
                if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
                {
                    if (pLoopUnit->getGroup() != getGroup())
                    {
                        if (pLoopUnit->AI_getGroupflag()==GROUPFLAG_SETTLERGROUP)
                        {
                            pLoopUnit->joinGroup(NULL);
                            pLoopUnit->joinGroup(getGroup());
                            getGroup()->pushMission(MISSION_SKIP);
                        }
                    }
                }
            }

            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	int iAreaBestFoundValue = 0;
	int iOtherBestFoundValue = 0;

	for (int iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
	{
		CvPlot* pCitySitePlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       01/10/09                                jdog5000      */
/*                                                                                              */
/* Bugfix, settler AI                                                                           */
/************************************************************************************************/
/* original bts code
		if (pCitySitePlot->getArea() == getArea())
*/
		// BBAI TODO:  For all terrain settlers, would need to remove area check here
		// Only count city sites we can get to
		if ((pCitySitePlot->getArea() == getArea() || canMoveAllTerrain()) && generatePath(pCitySitePlot, 0, true))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		{
			if (plot() == pCitySitePlot)
			{
				if (canFound(plot()))
				{
					getGroup()->pushMission(MISSION_FOUND);
					return;
				}
			}
			iAreaBestFoundValue = std::max(iAreaBestFoundValue, pCitySitePlot->getFoundValue(getOwnerINLINE()));

		}
		else
		{
			iOtherBestFoundValue = std::max(iOtherBestFoundValue, pCitySitePlot->getFoundValue(getOwnerINLINE()));
		}
	}
/*************************************************************************************************/
/**	BETTER AI (UNITAI_SETTLE move) Sephi                        	                            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
/**
	if ((iAreaBestFoundValue == 0) && (iOtherBestFoundValue == 0))
	{
		if ((GC.getGame().getGameTurn() - getGameTurnCreated()) > 20)
		{
			if (NULL != getTransportUnit())
			{
				getTransportUnit()->unloadAll();
			}

			if (NULL == getTransportUnit())
			{
				//may seem wasteful, but settlers confuse the AI.
				scrap();
				return;
			}
		}
	}

	if ((iOtherBestFoundValue * 100) > (iAreaBestFoundValue * 110))
	{
		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, -1, -1, 0, MOVE_SAFE_TERRITORY))
			{
				return;
			}
		}
	}
**/
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if ((iAreaBestFoundValue > 0) && plot()->isBestAdjacentFound(getOwnerINLINE()))
	{
		if (canFound(plot()))
		{
			getGroup()->pushMission(MISSION_FOUND);
			return;
		}
	}

	if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI) && !getGroup()->canDefend())
	{
		if (AI_retreatToCity())
		{
			return;
		}
	}

	if (plot()->isCity() && (plot()->getOwnerINLINE() == getOwnerINLINE()))
	{
		if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
			&& (GC.getGameINLINE().getMaxCityElimination() > 0))
		{
			if (getGroup()->getNumUnits() < 3)
			{
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
		}
	}

	if (iAreaBestFoundValue > 0)
	{
		if (AI_found())
		{
			return;
		}
	}
/*************************************************************************************************/
/**	BETTER AI (UNITAI_SETTLE move) Sephi                        	                            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
/**

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, -1, -1, 0, MOVE_NO_ENEMY_TERRITORY))
		{
			return;
		}
	}
**/
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_workerMove()
{
	PROFILE_FUNC();

	CvCity* pCity;
	bool bCanRoute;
	bool bNextCity;

	bCanRoute = canBuildRoute();
	bNextCity = false;

	// XXX could be trouble...
	if (plot()->getOwnerINLINE() != getOwnerINLINE())
	{
		if (AI_retreatToCity())
		{
			return;
		}
	}

	if (!isHuman())
	{
		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 2, -1, -1, 0, MOVE_SAFE_TERRITORY))
			{
				return;
			}
		}
	}

    if (GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(plot(), 3))
    {
		if(!plot()->isCity() || (!isHuman() && plot()->getPlotCity()->getAIGroup_Defense()->getNumUnits()==0))
		{
			if (AI_retreatToCity())
			{
				return;
			}

			if (AI_safety())
			{
				return;
			}
		}
	}
//Added Sephi

	if (bCanRoute)
	{
		//this should connect BonusResources to the cities
		if (plot()->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
		{
			if(plot()->getWorkingCity()!=NULL)
			{
				CvCity* pCity=plot()->getWorkingCity();
				for (int iI = 0; iI < pCity->getNumCityPlots(); iI++)
				{
					CvPlot* pLoopPlot = pCity->getCityIndexPlot(iI);

					if(pLoopPlot)
					{
						if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
						{
							if(pLoopPlot->getWorkingCity()==pCity)
							{
								BonusTypes eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

								if (eNonObsoleteBonus != NO_BONUS)
								{
									if (!(pLoopPlot->isConnectedToCapital()))
									{
										if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT || (!GC.getImprovementInfo(pLoopPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus)))
										{
											//can we build the Improvement needed to trade the bonus yet?
											bool bTrade=true;
											BuildTypes eBonusBuild=NO_BUILD;
											if(!GET_TEAM(getTeam()).isHasTech((TechTypes)GC.getBonusInfo(eNonObsoleteBonus).getTechCityTrade()))
											{
												bTrade=false;
											}
											else
											{
												for(int iJ=0;iJ<GC.getNumBuildInfos();iJ++)
												{
													if(GC.getBuildInfo((BuildTypes)iJ).getImprovement()!=NO_IMPROVEMENT && GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo((BuildTypes)iJ).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus))
													{
														if(canBuild(pLoopPlot,(BuildTypes)iJ))
														{
															eBonusBuild=(BuildTypes)iJ;
														}
													}
												}
											}
											if(bTrade && eBonusBuild!=NO_BUILD)
											{
												if(!atPlot(pLoopPlot))
												{
													getGroup()->pushMission(MISSION_MOVE_TO, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pLoopPlot);
												}

												if(atPlot(pLoopPlot))
												{
													getGroup()->pushMission(MISSION_BUILD, eBonusBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pLoopPlot);												
													return;
												}
											}
										}
										else
										{
											//Improvement already there, connect it!
											if (AI_connectPlot(pLoopPlot))
											{
												return;
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
	}

	if (bCanRoute)
	{
		//connect only cities that aren't connected yet
		if (AI_routeCity(true))
		{
			return;
		}
	}

	pCity = NULL;

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		pCity = plot()->getPlotCity();
		if (pCity == NULL)
		{
			pCity = plot()->getWorkingCity();
		}
	}

	//build an Improvement in a city that is not at the max
	if(AI_createImprovement(false))
	{
		return;
	}

	//build an Improvement in a city that is already at the max
	if(AI_createImprovement(true))
	{
		return;
	}

	if (bCanRoute)
	{
		if (AI_routeTerritory(true))
		{
			return;
		}

		if (AI_connectBonus(false))
		{
			return;
		}

		if (AI_routeCity())
		{
			return;
		}
	}

	if (AI_retreatToCity(false, true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
//Added Sephi end

	CvPlot* pBestBonusPlot = NULL;
	BuildTypes eBestBonusBuild = NO_BUILD;
	int iBestBonusValue = 0;

    if (AI_improveBonus(25, &pBestBonusPlot, &eBestBonusBuild, &iBestBonusValue))
	{
		return;
	}

	if (bCanRoute)
	{
		if (plot()->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
		{
			BonusTypes eNonObsoleteBonus = plot()->getNonObsoleteBonusType(getTeam());
			if (NO_BONUS != eNonObsoleteBonus)
			{
				if (!(plot()->isConnectedToCapital()))
				{
					ImprovementTypes eImprovement = plot()->getImprovementType();
					if (NO_IMPROVEMENT != eImprovement && GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
					{
						if (AI_connectPlot(plot()))
						{
							return;
						}
					}
				}
			}
		}
	}

    if (AI_improveBonus(25, &pBestBonusPlot, &eBestBonusBuild, &iBestBonusValue))
	{
		return;
	}

	if (bCanRoute && !isBarbarian())
	{
		if (AI_connectCity())
		{
			return;
		}
	}

	pCity = NULL;

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		pCity = plot()->getPlotCity();
		if (pCity == NULL)
		{
			pCity = plot()->getWorkingCity();
		}
	}
//	if (pCity != NULL)
//	{
//		bool bMoreBuilds = false;
//		for (iI = 0; iI < NUM_CITY_PLOTS; iI++)
//		{
//			CvPlot* pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);
//			if ((iI != CITY_HOME_PLOT) && (pLoopPlot != NULL))
//			{
//				if (pLoopPlot->getWorkingCity() == pCity)
//				{
//					if (pLoopPlot->isBeingWorked())
//					{
//						if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
//						{
//							if (pCity->AI_getBestBuildValue(iI) > 0)
//							{
//								ImprovementTypes eImprovement;
//								eImprovement = (ImprovementTypes)GC.getBuildInfo((BuildTypes)pCity->AI_getBestBuild(iI)).getImprovement();
//								if (eImprovement != NO_IMPROVEMENT)
//								{
//									bMoreBuilds = true;
//									break;
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//
//		if (bMoreBuilds)
//		{
//			if (AI_improveCity(pCity))
//			{
//				return;
//			}
//		}
//	}
	if (pCity != NULL)
	{
		if ((pCity->AI_getWorkersNeeded() > 0) && (plot()->isCity() || (pCity->AI_getWorkersNeeded() < ((1 + pCity->AI_getWorkersHave() * 2) / 3))))
		{
			if (AI_improveCity(pCity))
			{
				return;
			}
		}
	}

	if (AI_improveLocalPlot(2, pCity))
	{
		return;
	}

//FfH: Modified by Kael 11/11/2007
//	bool bBuildFort = false;
//
//	if (GC.getGame().getSorenRandNum(5, "AI Worker build Fort with Priority"))
//	{
//		bool bCanal = ((100 * area()->getNumCities()) / std::max(1, GC.getGame().getNumCities()) < 85);
//		CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
//		bool bAirbase = false;
//		bAirbase = (kPlayer.AI_totalUnitAIs(UNITAI_PARADROP) || kPlayer.AI_totalUnitAIs(UNITAI_ATTACK_AIR) || kPlayer.AI_totalUnitAIs(UNITAI_MISSILE_AIR));
//
//		if (bCanal || bAirbase)
//		{
//			if (AI_fortTerritory(bCanal, bAirbase))
//			{
//				return;
//			}
//		}
//		bBuildFort = true;
//	}
//FfH: End Modify

	if (bCanRoute && isBarbarian())
	{
		if (AI_connectCity())
		{
			return;
		}
	}

	if ((pCity == NULL) || (pCity->AI_getWorkersNeeded() == 0) || ((pCity->AI_getWorkersHave() > (pCity->AI_getWorkersNeeded() + 1))))
	{
		if ((pBestBonusPlot != NULL) && (iBestBonusValue >= 15))
		{
			if (AI_improvePlot(pBestBonusPlot, eBestBonusBuild))
			{
				return;
			}
		}

//		if (pCity == NULL)
//		{
//			pCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE()); // XXX do team???
//		}

		if (AI_nextCityToImprove(pCity))
		{
			return;
		}

		bNextCity = true;
	}

	if (pBestBonusPlot != NULL)
	{
		if (AI_improvePlot(pBestBonusPlot, eBestBonusBuild))
		{
			return;
		}
	}

	if (pCity != NULL)
	{
		if (AI_improveCity(pCity))
		{
			return;
		}
	}

	if (!bNextCity)
	{
		if (AI_nextCityToImprove(pCity))
		{
			return;
		}
	}

	if (bCanRoute)
	{
		if (AI_routeTerritory(true))
		{
			return;
		}

		if (AI_connectBonus(false))
		{
			return;
		}

		if (AI_routeCity())
		{
			return;
		}
	}

	if (AI_irrigateTerritory())
	{
		return;
	}

//FfH: Modified by Kael 11/11/2007
//		if (!bBuildFort)
//		{
//			bool bCanal = ((100 * area()->getNumCities()) / std::max(1, GC.getGame().getNumCities()) < 85);
//			CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
//			bool bAirbase = false;
//			bAirbase = (kPlayer.AI_totalUnitAIs(UNITAI_PARADROP) || kPlayer.AI_totalUnitAIs(UNITAI_ATTACK_AIR) || kPlayer.AI_totalUnitAIs(UNITAI_MISSILE_AIR));
//
//			if (bCanal || bAirbase)
//			{
//				if (AI_fortTerritory(bCanal, bAirbase))
//				{
//					return;
//				}
//			}
//		}
//FfH: End Modify

	if (bCanRoute)
	{
		if (AI_routeTerritory())
		{
			return;
		}
	}

	if (!isHuman() || (isAutomated() && GET_TEAM(getTeam()).getAtWarCount(true) == 0))
	{
		if (!isHuman() || (getGameTurnCreated() < GC.getGame().getGameTurn()))
		{
			if (AI_nextCityToImproveAirlift())
			{
				return;
			}
		}
		if (!isHuman())
		{
		    /** TEMPFIX Sephi
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, NO_UNITAI, -1, -1, -1, -1, MOVE_SAFE_TERRITORY))
			{
				return;
			}
			**/
		}
	}

	if (AI_improveLocalPlot(3, NULL))
	{
		return;
	}
/*************************************************************************************************/
/**	BETTER AI (stop AI from deleting workers) Sephi                            					**/
/**	                        																	**/
/**						                                            							**/
/*************************************************************************************************/
/**
	if (!(isHuman()) && (AI_getUnitAIType() == UNITAI_WORKER))
	{
		if (GC.getGameINLINE().getElapsedGameTurns() > 10)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs(UNITAI_WORKER) > GET_PLAYER(getOwnerINLINE()).getNumCities())
			{
				if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
				{
					scrap();
					return;
				}
			}
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (AI_retreatToCity(false, true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

//returns true if a Mission was pushed
//if bSwitch=false, only look for cities that can support more improvements
bool CvUnitAI::AI_createImprovement(bool bSwitch)
{
	CvPlot* pBestPlot=NULL;
	BuildTypes eBestBuild=NO_BUILD;
	int iBestValue=0;
	CvPlayer &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop, iValue, iPathTurns;
	for(CvCity* pLoopCity=kPlayer.firstCity(&iLoop);pLoopCity!=NULL;pLoopCity=kPlayer.nextCity(&iLoop))
	{
		if(pLoopCity->getArea()==getArea())
		{
			if(bSwitch || pLoopCity->calculateImprovementInfrastructureCostFreeLeft()>0)
			{
				if(pLoopCity->getAIGroup_Worker())
				{
					CvPlot* pLoopPlot=pLoopCity->getAIGroup_Worker()->getMissionPlot();
					if(pLoopPlot!=NULL && pLoopCity->AI_getBestBuild(pLoopCity->getCityPlotIndex(pLoopPlot))!=NO_BUILD)
					{
						if(canBuild(pLoopPlot,pLoopCity->AI_getBestBuild(pLoopCity->getCityPlotIndex(pLoopPlot))))
						{
							if (!GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pLoopPlot, 3))
							{
								if(generatePath(pLoopPlot,0,false,&iPathTurns))
								{
									iValue=pLoopCity->AI_getBestBuildValue(pLoopCity->getCityPlotIndex(pLoopPlot));
									if(pLoopCity->getAIGroup_Worker()->getNumUnits()==0)
									{
										iValue*=1000;
										if(iValue<0)
											iValue=MAX_INT;
									}
									iValue/=iPathTurns;
									if (iValue> iBestValue)
									{
										eBestBuild=pLoopCity->AI_getBestBuild(pLoopCity->getCityPlotIndex(pLoopPlot));
										iBestValue=iValue;
										pBestPlot=pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if(pBestPlot!=NULL && eBestBuild!=NO_BUILD && pBestPlot->getWorkingCity()!=NULL)
	{
		setAIGroup(pBestPlot->getWorkingCity()->getAIGroup_Worker());

		if(!atPlot(pBestPlot))
		{
			if(isOOSLogging())
			{
				oosLog("AIImprovement"
					,"Turn: %d,PlayerID:%d,UnitID:%d,X:%d,Y:%d,MoveTo:%d,%d\n"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwner()
					,getID()
					,getX()
					,getY()
					,pBestPlot->getX_INLINE()
					,pBestPlot->getY_INLINE()
				);
			}
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		}
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);
		if(isOOSLogging())
			{
				oosLog("AIImprovement"
					,"Turn: %d,PlayerID:%d,UnitID:%d,X:%d,Y:%d,MoveTo:%d,%d,doBuild\n"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwner()
					,getID()
					,getX()
					,getY()
					,pBestPlot->getX_INLINE()
					,pBestPlot->getY_INLINE()
				);
			}
		return true;
	}
	return false;
}

void CvUnitAI::AI_barbAttackMove()
{
	PROFILE_FUNC();
/*************************************************************************************************/
/**	BETTER AI (faster barbs ) Sephi                                            					**/
/**	                        																	**/
/** will hopefully run faster now                                   							**/
/*************************************************************************************************/
	castAnyTarget(0,true);

	//leashed barbs
	if(getOriginPlot() != NULL)
	{
		if(plot()->getNumUnits() > 1 && plot() == getOriginPlot())
		{
			AI_anyAttack(1, 20);
		}
		if(!atPlot(getOriginPlot()))
		{
			getGroup()->pushMission(MISSION_MOVE_TO, getOriginPlot()->getX_INLINE(), getOriginPlot()->getY_INLINE(), MOVE_DIRECT_ATTACK);
		}
        getGroup()->pushMission(MISSION_SKIP);
        return;
	}

	if (plot()->isCity())
	{
        if (plot()->getNumUnits()==1)
        {
            //archers
            getGroup()->AI_GroupRangeStrike();

            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
	}
    //Units attacked by ranged Strikes will attack back
    if (AI_getWasAttacked()>0)
    {
        if (AI_anyAttack(3, 0))
        {
            return;
        }
    }

	//Orthus (and other Heroes) will wait until they have enough XP

	if (AI_getUnitAIType()==UNITAI_HERO)
	{
	    if (getLevel()<7)
	    {
            getGroup()->pushMission(MISSION_SKIP);
            return;
	    }
	}

    if (AI_getUnitAIType()==UNITAI_ATTACK_CITY)
	{
		//wait for Reinforcements?
		if(getGroup()->getNumUnits()<4 && plot()->isCity())
		{
			if (AI_group(UNITAI_ATTACK_CITY,-1,-1,1,false,false,true,1,true))
			{
				return;
			}
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
		//Looks for targets (preference of Cities)
		else
		{
			if (AI_heal())
			{
				return;
			}

			if (AI_anyAttack(1, 20))
			{
				return;
			}

			if (AI_cityAttack(3, 20))
			{
				return;
			}

			if (AI_cityAttack(3, 0))
			{
				return;
			}

			if (AI_cityAttack(10, 40))
			{
				return;
			}

			if (AI_cityAttack(10, 20))
			{
				return;
			}

			if (AI_cityAttack(10, 0))
			{
				return;
			}

			if (AI_cityAttack(20, 40))
			{
				return;
			}

			if (AI_cityAttack(20, 20))
			{
				return;
			}

			if (AI_cityAttack(20, 0))
			{
				return;
			}
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}
    if (AI_getUnitAIType()==UNITAI_ATTACK || AI_getUnitAIType()==UNITAI_HERO)
	{
		//Guard a Lair?
		if (AI_getUnitAIType()!=UNITAI_HERO)
		{
            if (plot()->getImprovementType()!=NO_IMPROVEMENT && GC.getImprovementInfo(plot()->getImprovementType()).getSpawnUnitCiv()==GC.getDefineINT("BARBARIAN_CIVILIZATION"))
            {
                if (plot()->getNumUnits()<3)
                {
                    getGroup()->pushMission(MISSION_SKIP);
                    return;
                }
            }
		}

        if(canPillage(plot()))
        {
			int iPillageChance = 50;

			if (plot()->getOwnerINLINE()!=NO_PLAYER)
			{
				if (!GET_PLAYER(plot()->getOwnerINLINE()).isHuman())
				{
					iPillageChance=10;
				}
			}

            if (GC.getGameINLINE().getSorenRandNum(100, "AI Pillage") < iPillageChance)
            {
                getGroup()->pushMission(MISSION_PILLAGE, -1, -1, 0, false, false, MISSIONAI_PILLAGE, plot());
                return;
            }
        }

        if (getDamage()>10)
        {
			int iHealchance=80;
			//Barbs heal less in AI Land
			if (plot()->getOwnerINLINE()!=NO_PLAYER)
			{
				if (!GET_PLAYER(plot()->getOwnerINLINE()).isHuman())
				{
					iHealchance=20;
				}
			}
            if (GC.getGameINLINE().getSorenRandNum(100, "Healing") < iHealchance)
            {
                getGroup()->pushMission(MISSION_SKIP);
                return;
            }
        }

        //Special Barb Units
        if(AI_barbSpecialActionMove())
        {
            return;
        }

        //normal barb units

		if (AI_anyAttack(1, 20))
		{
			return;
		}

        if (AI_cityAttack(2, 0))
        {
            return;
        }


		//Look for a nice target close by
		CvPlot* pLoopPlot;
		CvPlot* pBestPlot=NULL;
		int iRange=5;
		int iDX, iDY;
		int iPathTurns;
		int iBestvalue=10000;

		bool bTimeToPillage=false;

		int iPillageChance = 33;

		if (plot()->getOwnerINLINE()!=NO_PLAYER)
		{
			if (!GET_PLAYER(plot()->getOwnerINLINE()).isHuman())
			{
				iPillageChance=8;
			}
		}

		if (GC.getGameINLINE().getSorenRandNum(100, "AI Pillage") < iPillageChance)
		{
			bTimeToPillage=true;
		}

		if (bTimeToPillage)
		{
			iBestvalue=3;

			for (iDX = -(iRange); iDX <= iRange; iDX++)
			{
				for (iDY = -(iRange); iDY <= iRange; iDY++)
				{
					pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

					if (pLoopPlot && pLoopPlot!=plot())
					{
						if(pLoopPlot->getArea()==getArea())
						{
							if(canPillage(pLoopPlot))
							{
								if(generatePath(pLoopPlot,0,false,&iPathTurns))
								{
									if(iPathTurns<iBestvalue)
									{
										pBestPlot=getPathEndTurnPlot();
										iBestvalue=iPathTurns;
									}
								}
							}
						}
					}
				}
			}
		}
		if (pBestPlot!=NULL)
		{
			if (atPlot(pBestPlot))
			{
				getGroup()->pushMission(MISSION_PILLAGE, -1, -1, 0, false, false, MISSIONAI_PILLAGE, plot());
				return;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
				return;
			}
		}
		else
		// found nothing to pillage
		{
			iBestvalue=10000;
			for (iDX = -(iRange); iDX <= iRange; iDX++)
			{
				for (iDY = -(iRange); iDY <= iRange; iDY++)
				{
					pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

					if (pLoopPlot && pLoopPlot!=plot())
					{
						if(pLoopPlot->getArea()==getArea())
						{
							if(pLoopPlot->isVisibleEnemyUnit(this))
							{
								if(generatePath(pLoopPlot,0,false,&iPathTurns))
								{
									int iVictimValue=GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(pLoopPlot,0,true,false);

									iVictimValue+=1000;
									iVictimValue=iVictimValue/(iPathTurns+1);

									if(iVictimValue<iBestvalue)
									{
										pBestPlot=getPathEndTurnPlot();
										iBestvalue=iVictimValue;
									}
								}
							}
						}
					}
				}
			}
		}

		if (pBestPlot!=NULL)
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
			return;
		}

		if (AI_anyAttack(10, 0))
		{
			return;
		}

		getGroup()->pushMission(MISSION_SKIP);
		return;

	}

	AI_rangeAttack(5);

	getGroup()->pushMission(MISSION_SKIP);
	return;

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->isGoody())
	{
		if (plot()->plotCount(PUF_isUnitAIType, UNITAI_ATTACK, -1, getOwnerINLINE()) == 1)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (GC.getGameINLINE().getSorenRandNum(2, "AI Barb") == 0)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}

	if (AI_anyAttack(1, 20))
	{
		return;
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		if (AI_pillageRange(4))
		{
			return;
		}

		if (AI_cityAttack(3, 10))
		{
			return;
		}

		if (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
		{
			if (AI_targetCity())
			{
				return;
			}
		}
	}

//FfH: Modified by Kael 10/11/2008
//	else if (GC.getGameINLINE().getNumCivCities() > (GC.getGameINLINE().countCivPlayersAlive() * 3))
//	{
//		if (AI_cityAttack(1, 15))
//		{
//			return;
//		}
//		if (AI_pillageRange(3))
//		{
//			return;
//		}
//		if (AI_cityAttack(2, 10))
//		{
//			return;
//		}
//		if (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
//		{
//			if (AI_targetCity())
//			{
//				return;
//			}
//		}
//	}
//	else if (GC.getGameINLINE().getNumCivCities() > (GC.getGameINLINE().countCivPlayersAlive() * 2))
//	{
//		if (AI_pillageRange(2))
//		{
//			return;
//		}
//		if (AI_cityAttack(1, 10))
//		{
//			return;
//		}
//	}
//	if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 1))
//	{
//		return;
//	}
//	if (AI_heal())
//	{
//		return;
//	}
//	if (AI_guardCity(false, true, 2))
//	{
//		return;
//	}
	if (AI_cityAttack(1, 15))
	{
		return;
	}
	if (AI_pillageRange(3))
	{
		return;
	}
	if (AI_cityAttack(2, 10))
	{
		return;
	}
	if (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE)
	{
		if (AI_targetCity())
		{
			return;
		}
	}
	if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 1))
	{
		return;
	}
    if (!isWorldUnitClass((UnitClassTypes)(m_pUnitInfo->getUnitClassType())))
    {
        if (AI_guardCity(false, true, 2))
        {
            return;
        }
    }
//FfH: End Modify

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_attackMove()
{
	PROFILE_FUNC();
	bool bDanger = (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3) > 0);
	{
		PROFILE("CvUnitAI::AI_attackMove() 1");

		if (AI_guardCity(true))
		{
			return;
		}

		if (AI_heal(30, 1))
		{
			return;
		}

		if (AI_guardCityAirlift())
		{
			return;
		}

		if (AI_guardCity(false, true, 1))
		{
			return;
		}

		//join any city attacks in progress
		if (plot()->getOwnerINLINE() != getOwnerINLINE())
		{
			if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 1, true, true))
			{
				return;
			}
		}

		AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
        if (plot()->isCity())
        {
            if (plot()->getOwnerINLINE() == getOwnerINLINE())
            {
                if ((eAreaAIType == AREAAI_ASSAULT) || (eAreaAIType == AREAAI_ASSAULT_ASSIST))
                {
                    if (AI_offensiveAirlift())
                    {
                        return;
                    }
                }
            }
        }

		if (bDanger)
		{
			if (AI_cityAttack(1, 55))
			{
				return;
			}

			if (AI_anyAttack(1, 65))
			{
				return;
			}

			if (collateralDamage() > 0)
			{
				if (AI_anyAttack(1, 45, 3))
				{
					return;
				}
			}
		}

		if (!noDefensiveBonus())
		{
			if (AI_guardCity(false, false))
			{
				return;
			}
		}

		if (!bDanger)
		{
			if (plot()->getOwnerINLINE() == getOwnerINLINE())
			{
				if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 3, -1, -1, -1, MOVE_SAFE_TERRITORY, 3))
				{
					return;
				}

				if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, 2, -1, -1, MOVE_SAFE_TERRITORY, 4))
				{
					return;
				}

				if (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_GROUP) > 0)
				{
					getGroup()->pushMission(MISSION_SKIP);
					return;
				}
			}
		}

		if (bDanger)
		{
			if (AI_pillageRange(1, 20))
			{
				return;
			}

			if (AI_cityAttack(1, 35))
			{
				return;
			}

			if (AI_anyAttack(1, 45))
			{
				return;
			}

			if (AI_pillageRange(3, 20))
			{
				return;
			}

			if (AI_choke(1))
			{
				return;
			}
		}

		if (AI_goody(3))
		{
			return;
		}

		if (AI_anyAttack(1, 70))
		{
			return;
		}
	}

	{
		PROFILE("CvUnitAI::AI_attackMove() 2");

		if (bDanger)
		{
			if (AI_cityAttack(4, 30))
			{
				return;
			}

			if (AI_anyAttack(2, 40))
			{
				return;
			}
		}

		if (!isEnemy(plot()->getTeam()))
		{
			if (AI_heal())
			{
				return;
			}
		}

		if ((GET_PLAYER(getOwnerINLINE()).AI_getNumAIUnits(UNITAI_CITY_DEFENSE) > 0) || (GET_TEAM(getTeam()).getAtWarCount(true) > 0))
		{
				if (AI_group(UNITAI_ATTACK_CITY, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, true, true, true, /*iMaxPath*/ 5))
				{
					return;
				}

			if (AI_group(UNITAI_ATTACK, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, true, true, false, /*iMaxPath*/ 4))
			{
				return;
			}

			if ((getMoves() > 1) && GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_FASTMOVERS))
			{
				if (AI_group(UNITAI_ATTACK, /*iMaxGroup*/ 4, /*iMaxOwnUnitAI*/ 1, -1, true, false, false, /*iMaxPath*/ 3))
				{
					return;
				}
			}
		}

		if (area()->getAreaAIType(getTeam()) != AREAAI_OFFENSIVE)
		{
/*************************************************************************************************/
/**	ADDON (MultiBarb) merged Sephi		12/23/08									Xienwolf	**/
/**	adjusted for more barb teams																**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
            if (area()->getCitiesPerPlayer(BARBARIAN_PLAYER) > 0)
/**								----  End Original Code  ----									**/
            if (area()->getCitiesPerPlayer(BARBARIAN_PLAYER)
            + area()->getCitiesPerPlayer(ANIMAL_PLAYER)
            + area()->getCitiesPerPlayer(WILDMANA_PLAYER)> 0)
            {
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/

				if (AI_targetBarbCity())
				{
					return;
				}
			}
		}
		else
		{
			if (getGroup()->getNumUnits() > 1)
			{
				if (AI_targetCity())
				{
					return;
				}
			}
		}

		if (AI_guardCity(false, true, 3))
		{
			return;
		}

		if ((GET_PLAYER(getOwnerINLINE()).getNumCities() > 1) && (getGroup()->getNumUnits() == 1))
		{
			if (area()->getAreaAIType(getTeam()) != AREAAI_DEFENSIVE)
			{
				if (area()->getNumUnrevealedTiles(getTeam()) > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_areaMissionAIs(area(), MISSIONAI_EXPLORE, getGroup()) < (GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area()) + 1))
					{
						if (AI_exploreRange(3))
						{
							return;
						}

						if (AI_explore())
						{
							return;
						}
					}
				}
			}
		}

		if (AI_protect(35))
		{
			return;
		}

		if (AI_offensiveAirlift())
		{
			return;
		}

		if (AI_defend())
		{
			return;
		}

		if (AI_travelToUpgradeCity())
		{
			return;
		}

        if (GC.getGameINLINE().getSorenRandNum(200, "CheckUnitforUpgrade")<2)
        {
            if (AI_travelToUpgradeCity())
            {
                return;
            }
        }

		if (AI_patrol())
		{
			return;
		}

		if (AI_retreatToCity())
		{
			return;
		}

		if (AI_safety())
		{
			return;
		}
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_paratrooperMove()
{
	PROFILE_FUNC();

	{
		PROFILE("CvUnitAI::AI_paratrooperMove()");
		bool bHostile = (plot()->isOwned() && isPotentialEnemy(plot()->getTeam()));
		if (!bHostile)
		{
			if (AI_guardCity(true))
			{
				return;
			}

			if (plot()->getTeam() == getTeam())
			{
				if (plot()->isCity())
				{
					if (AI_heal(30, 1))
					{
						return;
					}
				}

				AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
				bool bLandWar = ((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING));
				if (!bLandWar)
				{
					if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, -1, -1, 0, MOVE_SAFE_TERRITORY, 4))
					{
						return;
					}
				}
			}

			if (AI_guardCity(false, true, 1))
			{
				return;
			}
		}

		if (AI_cityAttack(1, 45))
		{
			return;
		}

		if (AI_anyAttack(1, 55))
		{
			return;
		}

		if (!bHostile)
		{
			if (AI_paradrop(getDropRange()))
			{
				return;
			}

			if (AI_offensiveAirlift())
			{
				return;
			}

			if (AI_moveToStagingCity())
			{
				return;
			}

			if (AI_guardFort(true))
			{
				return;
			}

			if (AI_guardCityAirlift())
			{
				return;
			}
		}

		if (collateralDamage() > 0)
		{
			if (AI_anyAttack(1, 45, 3))
			{
				return;
			}
		}

		if (AI_pillageRange(1, 15))
		{
			return;
		}

		if (bHostile)
		{
			if (AI_choke(1))
			{
				return;
			}
		}

		if (AI_heal())
		{
			return;
		}

		if (AI_retreatToCity())
		{
			return;
		}

		if (AI_protect(35))
		{
			return;
		}

		if (AI_safety())
		{
			return;
		}
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_attackCityMove()
{
	PROFILE_FUNC();

	bool bIgnoreFaster = false;
	if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_FASTMOVERS))
	{
		if (area()->getAreaAIType(getTeam()) != AREAAI_ASSAULT)
		{
			bIgnoreFaster = true;
		}
	}


	// force heal if we in our own city and damaged
	// can we remove this or call AI_heal here?
	if ((getGroup()->getNumUnits() == 1) && (getDamage() > 0) &&
        plot()->getOwnerINLINE() == getOwnerINLINE() && plot()->isCity())
    {
        getGroup()->pushMission(MISSION_HEAL);
		return;
    }

    AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
    bool bLandWar = !isBarbarian() && ((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING));

	if (plot()->isCity())
	{
		if ((GC.getGame().getGameTurn() - plot()->getPlotCity()->getGameTurnAcquired()) <= 1)
		{
			CvSelectionGroup* pOldGroup = getGroup();

			pOldGroup->AI_seperateNonAI(UNITAI_ATTACK_CITY);

			if (pOldGroup != getGroup())
			{
				return;
			}
		}

		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
		    if ((eAreaAIType == AREAAI_ASSAULT) || (eAreaAIType == AREAAI_ASSAULT_ASSIST))
		    {
		        if (AI_offensiveAirlift())
		        {
		            return;
		        }
		    }
		}
	}

	if (AI_guardCity(false, false, 1))
	{
		return;
	}

	if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 0, true, true, bIgnoreFaster))
	{
		return;
	}

	bool bCity = plot()->isCity();

	if (!bCity)
	{
		if (AI_bombardCity())
		{
			return;
		}

/** original code
		//stack attack
		if (getGroup()->getNumUnits() > 1)
		{
			if (AI_stackAttackCity(1, 250, true))
			{
				return;
			}
		}

		//stack attack
		if (getGroup()->getNumUnits() > 1)
		{
			if (AI_stackAttackCity(1, 110, true))
			{
				return;
			}
		}
**/
		// Attack with just 1 unit if overwhelming odds (Skyre)
		if (AI_stackAttackCity(1, 250, false))
		{
			return;
		}

		//stack attack
		if (getGroup()->getNumUnits() > 1)
		{
			if (AI_stackAttackCity(1, 110, false))
			{
				return;
			}
		}
/** end modifiy **/

	}

	if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 2, true, true, bIgnoreFaster))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	bool bHuntBarbs = false;
/*************************************************************************************************/
/**	ADDON (MultiBarb) merged Sephi		12/23/08									Xienwolf	**/
/**	adjusted for more barb teams																**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
    if (area()->getCitiesPerPlayer(BARBARIAN_PLAYER) > 0)
/**								----  End Original Code  ----									**/
    if (area()->getCitiesPerPlayer(BARBARIAN_PLAYER)
    + area()->getCitiesPerPlayer(ANIMAL_PLAYER)
    + area()->getCitiesPerPlayer(WILDMANA_PLAYER)> 0)
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
	{
		if ((area()->getAreaAIType(getTeam()) != AREAAI_OFFENSIVE) && (area()->getAreaAIType(getTeam()) != AREAAI_DEFENSIVE))
		{
			bHuntBarbs = true;
		}
	}
	bool bReadyToAttack = ((getGroup()->getNumUnits() >= (bHuntBarbs ? 3 : AI_stackOfDoomExtra())));
	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (!bLandWar)
		{
			if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, NO_UNITAI, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
			{
				return;
			}
		}

		if (!bReadyToAttack)
		{
			int iTargetCount = GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_GROUP);
			if ((iTargetCount * 5) > getGroup()->getNumUnits())
			{
				if (AI_moveToStagingCity())
				{
					return;
				}
			}
		}
	}

	if (collateralDamage() > 0)
	{
		if (AI_anyAttack(1, 45, 3))
		{
			return;
		}
	}

	if (AI_anyAttack(1, 60))
	{
		return;
	}

	bool bAtWar = isEnemy(plot()->getTeam());

	if (bAtWar && (getGroup()->getNumUnits() <= 2))
	{
		if (AI_pillageRange(3, 11))
		{
			return;
		}

		if (AI_pillageRange(1))
		{
			return;
		}
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (!isEnemy(plot()->getTeam()))
	{
		if (AI_heal())
		{
			return;
		}

		if ((getGroup()->getNumUnits() == 1) && (getTeam() != plot()->getTeam()))
		{
			if (AI_retreatToCity())
			{
				return;
			}
		}
	}

	if (!bReadyToAttack && !noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	//XXX more sophisticated logic for attacking is long overdue here
//	if ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) ||
//		  (atWar(getTeam(), plot()->getTeam())) ||
//			((area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE) && (getGroup()->getNumUnits() >= AI_stackOfDoomExtra())))
	if (bReadyToAttack)
	{
		if (bHuntBarbs && AI_targetBarbCity())
		{
			return;
		}
		else if (bLandWar)
		{
			if (AI_targetCity())
			{
				return;
			}
			CvTeamAI& kTeam = GET_TEAM(getTeam());
			if (kTeam.getAnyWarPlanCount(true) > 0)
			{
				CvCity* pTargetCity = area()->getTargetCity(getOwnerINLINE());

				if (pTargetCity != NULL)
				{
					if (AI_solveBlockageProblem(pTargetCity->plot(), (kTeam.getAtWarCount(true) == 0)))
					{
						return;
					}
				}
			}
		}
	}
	else
	{
		if ((bombardRate() > 0) && noDefensiveBonus())
		{
			if (AI_group(UNITAI_ATTACK_CITY, -1, -1, -1, bIgnoreFaster, true, true, /*iMaxPath*/ 10, /*bAllowRegrouping*/ true))
			{
				return;
			}
		}
		else
		{
			if (AI_group(UNITAI_ATTACK_CITY, AI_stackOfDoomExtra() * 2, -1, -1, bIgnoreFaster, true, true, /*iMaxPath*/ 10, /*bAllowRegrouping*/ false))
			{
				return;
			}
		}
	}

	if (AI_moveToStagingCity())
	{
		return;
	}

	if (AI_offensiveAirlift())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_attackCityLemmingMove()
{
	if (AI_cityAttack(1, 80))
	{
		return;
	}

	if (AI_bombardCity())
	{
		return;
	}

	if (AI_cityAttack(1, 40))
	{
		return;
	}

	if (AI_targetCity(MOVE_THROUGH_ENEMY))
	{
		return;
	}

	if (AI_anyAttack(1, 70))
	{
		return;
	}

	if (AI_anyAttack(1, 0))
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
}


void CvUnitAI::AI_collateralMove()
{
	PROFILE_FUNC();

	if (AI_leaveAttack(1, 20, 100))
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_cityAttack(1, 35))
	{
		return;
	}

	if (AI_anyAttack(1, 45, 3))
	{
		return;
	}

	if (AI_anyAttack(1, 55, 2))
	{
		return;
	}

	if (AI_anyAttack(1, 35, 3))
	{
		return;
	}

	if (AI_anyAttack(1, 30, 4))
	{
		return;
	}

	if (AI_anyAttack(1, 20, 5))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	if (AI_anyAttack(2, 55, 3))
	{
		return;
	}

	if (AI_cityAttack(2, 50))
	{
		return;
	}

	if (AI_anyAttack(2, 60))
	{
		return;
	}

	if (AI_protect(50))
	{
		return;
	}

	if (AI_guardCity(false, true, 3))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_pillageMove()
{
	PROFILE_FUNC();

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	//join any city attacks in progress
	if (plot()->getOwnerINLINE() != getOwnerINLINE())
	{
		if (AI_groupMergeRange(UNITAI_ATTACK_CITY, 1, true, true))
		{
			return;
		}
	}

	if (AI_cityAttack(1, 55))
	{
		return;
	}

	if (AI_anyAttack(1, 65))
	{
		return;
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	if (AI_pillageRange(3, 11))
	{
		return;
	}

	if (AI_choke(1))
	{
		return;
	}

	if (AI_pillageRange(1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
		{
			return;
		}
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (!isEnemy(plot()->getTeam()))
	{
		if (AI_heal())
		{
			return;
		}
	}

	if (AI_group(UNITAI_PILLAGE, /*iMaxGroup*/ 1, /*iMaxOwnUnitAI*/ 1, -1, /*bIgnoreFaster*/ true, false, false, /*iMaxPath*/ 3))
	{
		return;
	}

	if ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || isEnemy(plot()->getTeam()))
	{
		if (AI_pillage(20))
		{
			return;
		}
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_guardCity(false, true, 3))
	{
		return;
	}

	if (AI_offensiveAirlift())
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_reserveMove()
{
	PROFILE_FUNC();

	bool bDanger = (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3) > 0);


	if (bDanger && AI_leaveAttack(2, 55, 130))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, -1, -1, 1, -1, MOVE_SAFE_TERRITORY))
		{
			return;
		}
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_WORKER, -1, -1, 1, -1, MOVE_SAFE_TERRITORY))
		{
			return;
		}
	}

	if (!bDanger)
	{
		if (AI_group(UNITAI_SETTLE, 2, -1, -1, false, false, false, 3, true))
		{
			return;
		}
	}

	if (AI_guardCity(true))
	{
		return;
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardFort(false))
		{
			return;
		}
	}

	if (AI_guardCityAirlift())
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (AI_guardCitySite())
	{
		return;
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardFort(true))
		{
			return;
		}

		if (AI_guardBonus(15))
		{
			return;
		}
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (bDanger)
	{
		if (AI_cityAttack(1, 55))
		{
			return;
		}

		if (AI_anyAttack(1, 60))
		{
			return;
		}
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	if (bDanger)
	{
		if (AI_cityAttack(3, 45))
		{
			return;
		}

		if (AI_anyAttack(3, 50))
		{
			return;
		}
	}

	if (AI_protect(45))
	{
		return;
	}

	if (AI_guardCity(false, true, 3))
	{
		return;
	}

	if (AI_defend())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_counterMove()
{
	PROFILE_FUNC();

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (getSameTileHeal() > 0)
	{
		if (!canAttack())
		{
			if (AI_shadow(UNITAI_ATTACK_CITY))
			{
				return;
			}
		}
	}

    AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
    if (plot()->isCity())
    {
        if (plot()->getOwnerINLINE() == getOwnerINLINE())
        {
            if ((eAreaAIType == AREAAI_ASSAULT) || (eAreaAIType == AREAAI_ASSAULT_ASSIST))
            {
                if (AI_offensiveAirlift())
                {
                    return;
                }
            }
        }
    }

	bool bDanger = (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3) > 0);

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK_CITY, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
		{
			return;
		}

		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK, -1, -1, -1, -1, MOVE_SAFE_TERRITORY, 4))
		{
			return;
		}
	}

	if (!noDefensiveBonus())
	{
		if (AI_guardCity(false, false))
		{
			return;
		}
	}

	if (bDanger)
	{
		if (AI_cityAttack(1, 30))
		{
			return;
		}

		if (AI_anyAttack(1, 40))
		{
			return;
		}
	}

			if (AI_group(UNITAI_ATTACK_CITY, /*iMaxGroup*/ -1, 2, -1, false, /*bIgnoreOwnUnitType*/ true, /*bStackOfDoom*/ true, /*iMaxPath*/ 6))
			{
				return;
			}

	bool bFastMovers = (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_FASTMOVERS));

	if (AI_group(UNITAI_ATTACK, /*iMaxGroup*/ 2, -1, -1, bFastMovers, /*bIgnoreOwnUnitType*/ true, /*bStackOfDoom*/ true, /*iMaxPath*/ 5))
	{
		return;
	}

	if (AI_group(UNITAI_ATTACK_CITY, /*iMaxGroup*/ -1, 2, -1, false, /*bIgnoreOwnUnitType*/ true, /*bStackOfDoom*/ true, /*iMaxPath*/ 6))
	{
		return;
	}

	if (AI_guardCity(false, true, 3))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_offensiveAirlift())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_cityDefenseMove()
{
	PROFILE_FUNC();

	bool bDanger = (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3) > 0);

/*************************************************************************************************/
/**	BETTER AI (first city ) Sephi                                              					**/
/**	                        																	**/
/**						                                            							**/
/*************************************************************************************************/
    if (GET_PLAYER(getOwnerINLINE()).getNumCities() == 0 && (GC.getGameINLINE().getGameTurn()<10))
	{
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


	if (bDanger)
	{
		if (AI_leaveAttack(1, 70, 175))
		{
			return;
		}

		if (AI_chokeDefend())
		{
			return;
		}
	}

	if (AI_guardCityBestDefender())
	{
		return;
	}

	if (!bDanger)
	{
		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, -1, -1, 1, -1, MOVE_SAFE_TERRITORY, 1))
			{
				return;
			}
		}
	}

	if (AI_guardCityMinDefender(true))
	{
		return;
	}

	if (AI_guardCity(true))
	{
		return;
	}

	if (!bDanger)
	{
		if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 1, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
		{
			return;
		}

		if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 2, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
		{
			return;
		}

		if (plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, -1, -1, 1, -1, MOVE_SAFE_TERRITORY))
			{
				return;
			}
		}
	}

	AreaAITypes eAreaAI = area()->getAreaAIType(getTeam());
	if ((eAreaAI == AREAAI_ASSAULT) || (eAreaAI == AREAAI_ASSAULT_MASSING) || (eAreaAI == AREAAI_ASSAULT_ASSIST))
	{
		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK_CITY, -1, -1, -1, 0, MOVE_SAFE_TERRITORY))
		{
			return;
		}
	}

	if ((AI_getBirthmark() % 4) == 0)
	{
		if (AI_guardFort())
		{
			return;
		}
	}

	if (AI_guardCityAirlift())
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 3, -1, -1, -1, MOVE_SAFE_TERRITORY))
		{
			// will enter here if in danger
			return;
		}
	}

	if (AI_guardCity(false, true))
	{
		return;
	}
	if (!isBarbarian() && ((area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE) || (area()->getAreaAIType(getTeam()) == AREAAI_MASSING)))
	{
			if (AI_group(UNITAI_ATTACK_CITY, -1, 2, 4, /*bIgnoreFaster*/ true))
			{
				return;
			}
		}

	if (area()->getAreaAIType(getTeam()) == AREAAI_ASSAULT)
	{
		if (AI_load(UNITAI_ASSAULT_SEA, MISSIONAI_LOAD_ASSAULT, UNITAI_ATTACK_CITY, 1, 2, -1, 1, MOVE_SAFE_TERRITORY))
		{
			// does this ever occur? the previous settler check is less strict, this one should never be true (I think)
			FAssertMsg(false, "unexpected settler load (non-fatal)");
			return;
		}
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_cityDefenseExtraMove()
{
	PROFILE_FUNC();

	CvCity* pCity;

	if (AI_leaveAttack(2, 55, 150))
	{
		return;
	}

	if (AI_chokeDefend())
	{
		return;
	}

	if (AI_guardCityBestDefender())
	{
		return;
	}

	if (AI_guardCity(true))
	{
		return;
	}

	if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 1, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_SETTLE, /*iMaxGroup*/ 2, -1, -1, false, false, false, /*iMaxPath*/ 2, /*bAllowRegrouping*/ true))
	{
		return;
	}

	pCity = plot()->getPlotCity();

	if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE())) // XXX check for other team?
	{
		if (plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isUnitAIType, AI_getUnitAIType()) == 1)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (AI_guardCityAirlift())
	{
		return;
	}

	if (AI_guardCity(false, true, 1))
	{
		return;
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		if (AI_load(UNITAI_SETTLER_SEA, MISSIONAI_LOAD_SETTLER, UNITAI_SETTLE, 3, -1, -1, -1, MOVE_SAFE_TERRITORY, 3))
		{
			return;
		}
	}

	if (AI_guardCity(false, true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_exploreMove()
{
	PROFILE_FUNC();

	if (!isHuman() && canAttack())
	{
		if (AI_cityAttack(1, 60))
		{
			return;
		}

		if (AI_anyAttack(1, 70))
		{
			return;
		}
	}

	if (getDamage() > 0)
	{
		if (!plot()->isCity())
		{

			if (AI_retreatToCity())
			{
				return;
			}

		}
		getGroup()->pushMission(MISSION_HEAL);
		return;		
	}

	if (!isHuman())
	{
		if (AI_pillageRange(1))
		{
			return;
		}

		if (AI_cityAttack(3, 80))
		{
			return;
		}
	}

	if (AI_goody(4))
	{
		return;
	}

	if (AI_exploreRange(3))
	{
		return;
	}

	if (!isHuman())
	{
		if (AI_pillageRange(3))
		{
			return;
		}
	}

	if (AI_explore())
	{
		return;
	}

	if (!isHuman())
	{
		if (AI_pillage())
		{
			return;
		}
	}

	if (!isHuman())
	{
		if (AI_travelToUpgradeCity())
		{
			return;
		}
	}

//FfH: Modified by Kael 08/21/2008
//	if (!isHuman() && (AI_getUnitAIType() == UNITAI_EXPLORE))
//	{
//		if (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(area(), UNITAI_EXPLORE) > GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(area()))
//		{
//			if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
//			{
//				scrap();
//				return;
//			}
//		}
//	}
//FfH: End Add

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_missionaryMove()
{
	PROFILE_FUNC();

	if (AI_spreadReligion())
	{
		return;
	}

	if (AI_spreadCorporation())
	{
		return;
	}

	if (!isHuman() || (isAutomated() && GET_TEAM(getTeam()).getAtWarCount(true) == 0))
	{
		if (!isHuman() || (getGameTurnCreated() < GC.getGame().getGameTurn()))
		{
			if (AI_spreadReligionAirlift())
			{
				return;
			}
			if (AI_spreadCorporationAirlift())
			{
				return;
			}
		}

		if (!isHuman())
		{
			if (AI_load(UNITAI_MISSIONARY_SEA, MISSIONAI_LOAD_SPECIAL, NO_UNITAI, -1, -1, -1, 0, MOVE_SAFE_TERRITORY))
			{
				return;
			}

			if (AI_load(UNITAI_MISSIONARY_SEA, MISSIONAI_LOAD_SPECIAL, NO_UNITAI, -1, -1, -1, 0, MOVE_NO_ENEMY_TERRITORY))
			{
				return;
			}
		}
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_prophetMove()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	BETTER AI (first city) Sephi                                          		                **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    if(GC.getLeaderHeadInfo(GET_PLAYER(getOwnerINLINE()).getLeaderType()).isAltarVictory())
    {
		if(!plot()->isCity() || !plot()->getPlotCity()->isCapital())
		{
			if(GET_PLAYER((PlayerTypes)getOwnerINLINE()).getCapitalCity()!=NULL)
			{
				CvPlot* pTarget=GET_PLAYER((PlayerTypes)getOwnerINLINE()).getCapitalCity()->plot();
				if(!atPlot(pTarget) && generatePath(pTarget))
				{
					getGroup()->pushMission(MISSION_MOVE_TO, pTarget->getX_INLINE(), pTarget->getY_INLINE(),MOVE_SAFE_TERRITORY);
					return;
				}
				if(atPlot(pTarget))
				{
					for (int iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
					{
						BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);

						if (NO_BUILDING != eBuilding)
						{
							bool bDoesBuild = false;
							if ((m_pUnitInfo->getForceBuildings(eBuilding))
								|| (m_pUnitInfo->getBuildings(eBuilding)))
							{
								bDoesBuild = true;
							}

							if (canConstruct(plot(), eBuilding))
							{
								getGroup()->pushMission(MISSION_CONSTRUCT, eBuilding);
								return;
							}
						}
					}
				}
			}
		}
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (AI_construct(1))
	{
		return;
	}

	if (AI_discover(true, true))
	{
		return;
	}

	if (AI_construct(3))
	{
		return;
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = std::max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
		{
			return;
		}
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_artistMove()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/** Skyre Mod                                                                                   **/
/** BETTER AI (use Artists to end Disorder) merged Sephi                                        **/
/**						                                            							**/
/*************************************************************************************************/

    CvCity* pCity = plot()->getPlotCity();

    if (pCity != NULL && pCity->isDisorder())
    {
        getGroup()->pushMission(MISSION_GREAT_WORK);
        return;
    }

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


	if (AI_artistCultureVictoryMove())
	{
	    return;
	}

	if (AI_construct())
	{
		return;
	}

	if (AI_discover(true, true))
	{
		return;
	}

	if (AI_greatWork())
	{
		return;
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = std::max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
		{
			return;
		}
	}

	if (AI_join())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_scientistMove()
{
	PROFILE_FUNC();

	if (AI_discover(true, true))
	{
		return;
	}

	if (AI_construct(MAX_INT, 1))
	{
		return;
	}
	if (GET_PLAYER(getOwnerINLINE()).getCurrentEra() < 3)
	{
		if (AI_join(2))
		{
			return;
		}
	}

	if (GET_PLAYER(getOwnerINLINE()).getCurrentEra() <= (GC.getNumEraInfos() / 2))
	{
		if (AI_construct())
		{
			return;
		}
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = std::max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
		{
			return;
		}
	}

	if (AI_join())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_generalMove()
{
	PROFILE_FUNC();

	std::vector<UnitAITypes> aeUnitAITypes;
	int iDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2);

	bool bOffenseWar = (area()->getAreaAIType(getTeam()) == AREAAI_OFFENSIVE);


	if (iDanger > 0)
	{
		aeUnitAITypes.clear();
		aeUnitAITypes.push_back(UNITAI_ATTACK);
		aeUnitAITypes.push_back(UNITAI_COUNTER);
		if (AI_lead(aeUnitAITypes))
		{
			return;
		}
	}

	if (AI_construct(1))
	{
		return;
	}
	if (AI_join(1))
	{
		return;
	}

	if (bOffenseWar && (GC.getGameINLINE().getSorenRandNum(2, "AI General Lead") == 0))
	{
		aeUnitAITypes.clear();
		aeUnitAITypes.push_back(UNITAI_ATTACK_CITY);
		if (AI_lead(aeUnitAITypes))
		{
			return;
		}
	}


	if (AI_join(2))
	{
		return;
	}

	if (AI_construct(2))
	{
		return;
	}
	if (AI_join(4))
	{
		return;
	}

	if (GC.getGameINLINE().getSorenRandNum(3, "AI General Construct") == 0)
	{
		if (AI_construct())
		{
			return;
		}
	}

	if (AI_join())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_merchantMove()
{
	PROFILE_FUNC();

	//temporary Sephi
	//make all specialists join the city
	if (AI_join())
	{
		return;
	}

	if (AI_construct())
	{
		return;
	}

	if (AI_discover(true, true))
	{
		return;
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = std::max(1, getDiscoverResearch(NO_TECH));

	if (AI_trade(iGoldenAgeValue * 2))
	{
	    return;
	}

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (AI_trade(iGoldenAgeValue))
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
		{
			return;
		}
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_engineerMove()
{
	PROFILE_FUNC();

	if (AI_construct())
	{
		return;
	}

	if (AI_switchHurry())
	{
		return;
	}

	if (AI_hurry())
	{
		return;
	}

	if (AI_join())
	{
		return;
	}

	if (AI_discover(true, true))
	{
		return;
	}

	int iGoldenAgeValue = (GET_PLAYER(getOwnerINLINE()).AI_calculateGoldenAgeValue() / (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge()));
	int iDiscoverValue = std::max(1, getDiscoverResearch(NO_TECH));

	if (((iGoldenAgeValue * 100) / iDiscoverValue) > 60)
	{
        if (AI_goldenAge())
        {
            return;
        }

        if (iDiscoverValue > iGoldenAgeValue)
        {
            if (AI_discover())
            {
                return;
            }
            if (GET_PLAYER(getOwnerINLINE()).getUnitClassCount(getUnitClassType()) > 1)
            {
                if (AI_join())
                {
                    return;
                }
            }
        }
	}
	else
	{
		if (AI_discover())
		{
			return;
		}

		if (AI_join())
		{
			return;
		}
	}

	if ((GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) ||
		  (getGameTurnCreated() < (GC.getGameINLINE().getGameTurn() - 25)))
	{
		if (AI_discover())
		{
			return;
		}
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_spyMove()
{
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	int iEspionageChance = 0;
	if (plot()->isOwned() && (plot()->getTeam() != getTeam()))
	{
		switch (kTeam.AI_getAttitude(plot()->getTeam()))
		{
		case ATTITUDE_FURIOUS:
			iEspionageChance = 100;
			break;

		case ATTITUDE_ANNOYED:
			iEspionageChance = 50;
			break;

		case ATTITUDE_CAUTIOUS:
			iEspionageChance = 0;
			break;

		case ATTITUDE_PLEASED:
			iEspionageChance = 0;
			break;

		case ATTITUDE_FRIENDLY:
			iEspionageChance = 0;
			break;

		default:
			FAssert(false);
			break;
		}

		WarPlanTypes eWarPlan = kTeam.AI_getWarPlan(plot()->getTeam());
		if (eWarPlan != NO_WARPLAN)
		{
			if (eWarPlan == WARPLAN_LIMITED)
			{
				iEspionageChance += 50;
			}
			else
			{
				iEspionageChance += 10;
			}
		}

		if (plot()->isCity() && plot()->getTeam() != getTeam())
		{
			if (getFortifyTurns() >= GC.getDefineINT("MAX_FORTIFY_TURNS"))
			{
				if (AI_espionageSpy())
				{
					return;
				}
			}
			if (GC.getGame().getSorenRandNum(100, "AI Spy Skip Turn") > 5)
			{
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
		}

		if (GC.getGameINLINE().getSorenRandNum(100, "AI Spy Espionage") < iEspionageChance)
		{
			if (AI_espionageSpy())
			{
				return;
			}
		}
	}

	if (plot()->getTeam() == getTeam())
	{
		if (kTeam.getAnyWarPlanCount(true) == 0)
		{
			if (AI_guardSpy(0))
			{
				return;
			}
		}

		if (GC.getGame().getSorenRandNum(100, "AI Spy pillage improvement") < 25)
		{
			if (AI_bonusOffenseSpy(3))
			{
				return;
			}
		}
		else
		{
			if (AI_cityOffenseSpy(10))
			{
				return;
			}
		}
	}

	if (iEspionageChance > 0 && (plot()->isCity() || (plot()->getNonObsoleteBonusType(getTeam()) != NO_BONUS)))
	{
		if (GC.getGame().getSorenRandNum(7, "AI Spy Skip Turn") > 0)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}


	if (GC.getGame().getSorenRandNum(4, "AI Spy Choose Movement"))
	{
		if (AI_reconSpy(3))
		{
			return;
		}
	}
	else
	{
		if (AI_cityOffenseSpy(10))
		{
			return;
		}
	}

//	int iInfiltrateChance = 25;
//	if (plot()->getPlotCity() != NULL)
//	{
//		if (plot()->getTeam() != getTeam())
//		{
//			iInfiltrateChance += 50;
//		}
//	}

//	if (GC.getGameINLINE().getSorenRandNum(100, "AI Spy Infiltrate") < iInfiltrateChance)
//	{
//		if (AI_infiltrate())
//		{
//			return;
//		}
//	}



	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_ICBMMove()
{
//	CvCity* pCity = plot()->getPlotCity();

//	if (pCity != NULL)
//	{
//		if (pCity->AI_isDanger())
//		{
//			if (!(pCity->AI_isDefended()))
//			{
//				if (AI_airCarrier())
//				{
//					return;
//				}
//			}
//		}
//	}

	if (airRange() > 0)
	{
		if (AI_nukeRange(airRange()))
		{
			return;
		}
	}
	else if (AI_nuke())
	{
		return;
	}

	if (isCargo())
	{
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	if (airRange() > 0)
	{
		if (AI_missileLoad(UNITAI_MISSILE_CARRIER_SEA, 2, true))
		{
			return;
		}

		if (AI_missileLoad(UNITAI_MISSILE_CARRIER_SEA, 1, false))
		{
			return;
		}

		if (AI_getBirthmark() % 3 == 0)
		{
			if (AI_missileLoad(UNITAI_ATTACK_SEA, 0, false))
			{
				return;
			}
		}

		if (AI_airOffensiveCity())
		{
			return;
		}
	}

	getGroup()->pushMission(MISSION_SKIP);
}


void CvUnitAI::AI_workerSeaMove()
{
	PROFILE_FUNC();

	int iI;

	if (!(getGroup()->canDefend()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
		{
			if (AI_retreatToCity())
			{
				return;
			}
		}
	}
/*************************************************************************************************/
/** Skyre Mod                                                                                   **/
/** BETTER AI (Lanun Pirate Coves) merged Sephi                                                 **/
/**						                                            							**/
/*************************************************************************************************/
    if (GET_PLAYER(getOwnerINLINE()).isPirate())
    {
        if (AI_buildPirateCove())
        {
            return;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	if (AI_improveBonus(20))
	{
		return;
	}

	if (AI_improveBonus(10))
	{
		return;
	}

	if (AI_improveBonus())
	{
		return;
	}

	if (isHuman())
	{
		FAssert(isAutomated());
		if (plot()->getBonusType() != NO_BONUS)
		{
			if ((plot()->getOwnerINLINE() == getOwnerINLINE()) || (!plot()->isOwned()))
			{
				getGroup()->pushMission(MISSION_SKIP);
				return;
			}
		}

		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes)iI);
			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getBonusType() != NO_BONUS)
				{
					if (pLoopPlot->isValidDomainForLocation(*this))
					{
						getGroup()->pushMission(MISSION_SKIP);
						return;
					}
				}
			}
		}
	}
/** Disabled Sephi
	CvCity* pCity;

	if (!(isHuman()) && (AI_getUnitAIType() == UNITAI_WORKER_SEA))
	{
		pCity = plot()->getPlotCity();

		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				if (pCity->AI_neededSeaWorkers() == 0)
				{
					if (GC.getGameINLINE().getElapsedGameTurns() > 10)
					{
						if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
						{
							scrap();
							return;
						}
					}
				}
				else
				{
					//Probably icelocked since it can't perform actions.
					scrap();
					return;
				}
			}
		}
	}
**/
	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_barbAttackSeaMove()
{
	PROFILE_FUNC();

	if (GC.getGameINLINE().getSorenRandNum(2, "AI Barb") == 0)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}

	if (AI_anyAttack(2, 25))
	{
		return;
	}

	if (AI_pillageRange(4))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_pirateSeaMove()
{
	PROFILE_FUNC();

	CvArea* pWaterArea;

/*************************************************************************************************/
/** BETTER AI (Naval AI) Sephi                                                                  **/
/**						                                            							**/
/*************************************************************************************************/
/*********************************************************************/
/** DEBUG **/
    if (GC.getDefineINT("USE_PERFORMANCE_LOG_LEVEL1")==1)
    {
		CvString szError="CvUnitAI::AI_pirateSeaMove Begin 1";
		gDLL->logMsg("debug.log", szError,true,true);
	}
/** DEBUG End **/
/*********************************************************************/

    if (getOwnerINLINE()==PIRATES_PLAYER)
    {
        if(getCargo()>0)
        {
            //time to plunder
            if (AI_assaultSeaTransport(false))
            {
                return;
            }
        }
    }

/*********************************************************************/
/** DEBUG **/
    if (GC.getDefineINT("USE_PERFORMANCE_LOG_LEVEL1")==1)
    {
		CvString szError="CvUnitAI::AI_pirateSeaMove Begin 2";
		gDLL->logMsg("debug.log", szError,true,true);
	}
/** DEBUG End **/
/*********************************************************************/

	if (AI_pillageRange(3))
	{
		return;
	}
/*********************************************************************/
/** DEBUG **/
    if (GC.getDefineINT("USE_PERFORMANCE_LOG_LEVEL1")==1)
    {
		CvString szError="CvUnitAI::AI_pirateSeaMove Begin 3";
		gDLL->logMsg("debug.log", szError,true,true);
	}
/** DEBUG End **/
/*********************************************************************/

	if (AI_heal())
	{
		return;
	}
/*********************************************************************/
/** DEBUG **/
    if (GC.getDefineINT("USE_PERFORMANCE_LOG_LEVEL1")==1)
    {
		CvString szError="CvUnitAI::AI_pirateSeaMove Begin 4";
		gDLL->logMsg("debug.log", szError,true,true);
	}
/** DEBUG End **/
/*********************************************************************/

	if (AI_anyAttack(3, 30))
	{
		return;
	}
/*********************************************************************/
/** DEBUG **/
    if (GC.getDefineINT("USE_PERFORMANCE_LOG_LEVEL1")==1)
    {
		CvString szError="CvUnitAI::AI_pirateSeaMove Begin 5";
		gDLL->logMsg("debug.log", szError,true,true);
	}
/** DEBUG End **/
/*********************************************************************/

	if (AI_anyAttack(20, 0))
	{
		return;
	}
/*********************************************************************/
/** DEBUG **/
    if (GC.getDefineINT("USE_PERFORMANCE_LOG_LEVEL1")==1)
    {
		CvString szError="CvUnitAI::AI_pirateSeaMove Begin 6";
		gDLL->logMsg("debug.log", szError,true,true);
	}
/** DEBUG End **/
/*********************************************************************/

	getGroup()->pushMission(MISSION_SKIP);
	return;

/*************************************************************************************************/
/**	END                                                                  						**/
/*************************************************************************************************/

	if (plot()->isCity())
	{
		if (AI_heal())
		{
			return;
		}
	}

	if (plot()->isOwned() && (plot()->getTeam() == getTeam()))
	{
		if (AI_anyAttack(2, 30))
		{
			return;
		}

		if (AI_protect(30))
		{
			return;
		}

		if (((AI_getBirthmark() / 8) % 2) == 0)
		{
			if (AI_group(UNITAI_PIRATE_SEA, 1, 0))
			{
				return;
			}
		}
	}
	else
	{
		if (AI_anyAttack(2, 51))
		{
			return;
		}
	}


	if (GC.getGame().getSorenRandNum(10, "AI Pirate Explore") == 0)
	{
		pWaterArea = plot()->waterArea();

		if (pWaterArea != NULL)
		{
			if (pWaterArea->getNumUnrevealedTiles(getTeam()) > 0)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_areaMissionAIs(pWaterArea, MISSIONAI_EXPLORE, getGroup()) < (GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea)))
				{
					if (AI_exploreRange(2))
					{
						return;
					}
				}
			}
		}
	}

	if (GC.getGame().getSorenRandNum(11, "AI Pirate Pillage") == 0)
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}

	//Includes heal and retreat to sea routines.
	if (AI_pirateBlockade())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_attackSeaMove()
{
	PROFILE_FUNC();

	if (AI_seaRetreatFromCityDanger())
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_anyAttack(1, 35))
	{
		return;
	}

	if (AI_anyAttack(2, 40))
	{
		return;
	}

	if (AI_seaBombardRange(6))
	{
		return;
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_group(UNITAI_CARRIER_SEA, /*iMaxGroup*/ 4, 1, -1, true, false, false, /*iMaxPath*/ 5))
	{
		return;
	}

	if (AI_group(UNITAI_ATTACK_SEA, /*iMaxGroup*/ 1, -1, -1, true, false, false, /*iMaxPath*/ 3))
	{
		return;
	}

	if (!plot()->isOwned() || !isEnemy(plot()->getTeam()))
	{
		if (AI_shadow(UNITAI_ASSAULT_SEA, 4, 34))
		{
			return;
		}

		if (AI_shadow(UNITAI_CARRIER_SEA, 4, 51))
		{
			return;
		}
		if (AI_group(UNITAI_ASSAULT_SEA, -1, 4, -1, false, false, false))
		{
			return;
		}
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, 1, -1, false, false, false))
	{
		return;
	}

	if (plot()->isOwned() && (isEnemy(plot()->getTeam())))
	{
		if (AI_blockade())
		{
			return;
		}
	}

	if (AI_pillageRange(4))
	{
		return;
	}

	if (AI_protect(35))
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_reserveSeaMove()
{
	PROFILE_FUNC();

	if (AI_seaRetreatFromCityDanger())
	{
		return;
	}

	if (AI_guardBonus(30))
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_anyAttack(1, 55))
	{
		return;
	}

	if (AI_seaBombardRange(6))
	{
		return;
	}

	if (AI_protect(40))
	{
		return;
	}

	if (AI_shadow(UNITAI_SETTLER_SEA, 1, -1, true))
	{
		return;
	}

	if (AI_group(UNITAI_RESERVE_SEA, 1))
	{
		return;
	}

	if (bombardRate() > 0)
	{
		if (AI_shadow(UNITAI_ASSAULT_SEA, 2, 30, true))
		{
			return;
		}
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (AI_anyAttack(3, 45))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (!isNeverInvisible())
	{
		if (AI_anyAttack(5, 35))
		{
			return;
		}
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_escortSeaMove()
{
	PROFILE_FUNC();

//	// if we have cargo, possibly convert to UNITAI_ASSAULT_SEA (this will most often happen with galleons)
//	// note, this should not happen when we are not the group head, so escort galleons are fine joining a group, just not as head
//	if (hasCargo() && (getUnitAICargo(UNITAI_ATTACK_CITY) > 0 || getUnitAICargo(UNITAI_ATTACK) > 0))
//	{
//		// non-zero AI_unitValue means that UNITAI_ASSAULT_SEA is valid for this unit (that is the check used everywhere)
//		if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), UNITAI_ASSAULT_SEA, NULL) > 0)
//		{
//			// save old group, so we can merge it back in
//			CvSelectionGroup* pOldGroup = getGroup();
//
//			// this will remove this unit from the current group
//			AI_setUnitAIType(UNITAI_ASSAULT_SEA);
//
//			// merge back the rest of the group into the new group
//			CvSelectionGroup* pNewGroup = getGroup();
//			if (pOldGroup != pNewGroup)
//			{
//				pOldGroup->mergeIntoGroup(pNewGroup);
//			}
//
//			// perform assault sea action
//			AI_assaultSeaMove();
//			return;
//		}
//	}

	if (AI_seaRetreatFromCityDanger())
	{
		return;
	}

	if (AI_heal(30, 1))
	{
		return;
	}

	if (AI_anyAttack(1, 55))
	{
		return;
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, /*iMaxOwnUnitAI*/ 0, -1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 0, -1, /*bIgnoreFaster*/ true, false, false, /*iMaxPath*/ 3))
	{
		return;
	}

	if (AI_heal(50, 3))
	{
		return;
	}

	if (AI_pillageRange(2))
	{
		return;
	}

	if (AI_group(UNITAI_MISSILE_CARRIER_SEA, 1, 1, true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, 1, /*iMaxOwnUnitAI*/ 0, /*iMinUnitAI*/ -1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 2, /*iMinUnitAI*/ -1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, /*iMaxOwnUnitAI*/ 2, /*iMinUnitAI*/ -1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ 4, /*iMinUnitAI*/ -1, /*bIgnoreFaster*/ true))
	{
		return;
	}

	if (AI_heal())
	{
		return;
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_exploreSeaMove()
{
	PROFILE_FUNC();

	if (AI_seaRetreatFromCityDanger())
	{
		return;
	}

	CvArea* pWaterArea = plot()->waterArea();

	if (!isHuman())
	{
		if (AI_anyAttack(1, 60))
		{
			return;
		}
	}

	if (!isHuman() && !isBarbarian()) //XXX move some of this into a function? maybe useful elsewhere
	{
		//Obsolete?
		int iValue = GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), AI_getUnitAIType(), area());
		int iBestValue = GET_PLAYER(getOwnerINLINE()).AI_bestAreaUnitAIValue(AI_getUnitAIType(), area());

		if (iValue < iBestValue)
		{
			//Transform
			if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), UNITAI_WORKER_SEA, area()) > 0)
			{
				AI_setUnitAIType(UNITAI_WORKER_SEA);
				return;
			}

			if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), UNITAI_PIRATE_SEA, area()) > 0)
			{
				AI_setUnitAIType(UNITAI_PIRATE_SEA);
				return;
			}

			if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), UNITAI_MISSIONARY_SEA, area()) > 0)
			{
				AI_setUnitAIType(UNITAI_MISSIONARY_SEA);
				return;
			}

			if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), UNITAI_RESERVE_SEA, area()) > 0)
			{
				AI_setUnitAIType(UNITAI_RESERVE_SEA);
				return;
			}
			scrap();
		}
	}

	if (getDamage() > 0)
	{
		if ((plot()->getFeatureType() == NO_FEATURE) || (GC.getFeatureInfo(plot()->getFeatureType()).getTurnDamage() == 0))
		{
			getGroup()->pushMission(MISSION_HEAL);
			return;
		}
	}

	if (!isHuman())
	{
		if (AI_pillageRange(1))
		{
			return;
		}
	}

	if (AI_exploreRange(4))
	{
		return;
	}

	if (!isHuman())
	{
		if (AI_pillageRange(4))
		{
			return;
		}
	}

	if (AI_explore())
	{
		return;
	}

	if (!isHuman())
	{
		if (AI_pillage())
		{
			return;
		}
	}

	if (!isHuman())
	{
		if (AI_travelToUpgradeCity())
		{
			return;
		}
	}

	if (!(isHuman()) && (AI_getUnitAIType() == UNITAI_EXPLORE_SEA))
	{
		pWaterArea = plot()->waterArea();

		if (pWaterArea != NULL)
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_EXPLORE_SEA) > GET_PLAYER(getOwnerINLINE()).AI_neededExplorers(pWaterArea))
			{
				if (GET_PLAYER(getOwnerINLINE()).calculateUnitCost() > 0)
				{
					scrap();
					return;
				}
			}
		}
	}

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_assaultSeaMove()
{
	PROFILE_FUNC();

	FAssert(AI_getUnitAIType() == UNITAI_ASSAULT_SEA);

	bool bEmpty = !getGroup()->hasCargo();
	if (bEmpty)
	{
		if (AI_anyAttack(1, 65))
		{
			return;
		}
		if (AI_anyAttack(1, 40))
		{
			return;
		}
	}
	bool bAttack = false;
	bool bLandWar = false;
	bool bIsCity = plot()->isCity();
	int iTargetStackSize = std::max(4, 1 + (AI_stackOfDoomExtra()));
	int iCargo = getGroup()->getCargo();
	bool bFull = getGroup()->AI_isFull();

	AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
	bLandWar = !isBarbarian() && ((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING));

	if (bIsCity)
	{
		if (eAreaAIType == AREAAI_ASSAULT)
		{
			if (iCargo >= iTargetStackSize)
			{
				bAttack = true;
			}
		}
		if (!bAttack)
		{
			if (eAreaAIType == AREAAI_ASSAULT && iCargo > 0)
			{
				int iAttackers = GET_PLAYER(getOwnerINLINE()).AI_enemyTargetMissionAIs(MISSIONAI_ASSAULT, getGroup());
				if (iAttackers >= iTargetStackSize)
				{
					if (bFull)
					{
						//Join the attack
						bAttack = true;
					}
				}
			}
			else if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2))
			{
				if (getGroup()->hasCargo())
				{
					getGroup()->unloadAll();
					getGroup()->pushMission(MISSION_SKIP);
					return;
				}
			}
			else if (bLandWar)
			{
				if (bFull && (eAreaAIType != AREAAI_DEFENSIVE))
				{
					if (AI_assaultSeaTransport(false))
					{
						return;
					}
				}
				else if (iCargo > 0)
				{
					getGroup()->unloadAll();
					getGroup()->pushMission(MISSION_SKIP);
					return;
				}
			}
			else if (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_LOAD_ASSAULT) > 0)
			{
				if (!bFull)
				{
					getGroup()->pushMission(MISSION_SKIP);
					return;
				}
			}
			else
			{
				if (!bLandWar && iCargo > 0)
				{
					if (AI_group(UNITAI_ASSAULT_SEA))
					{
						return;
					}
				}
			}
		}
	}

	if (plot()->getTeam() == getTeam())
	{
		if ((iCargo == 0) && getGroup()->getNumUnits() > 1)
		{
			getGroup()->AI_makeForceSeparate();
		}
	}

	if (!bIsCity)
	{
	 	if (plot()->isOwned() && isEnemy(plot()->getTeam()))
		{
			if (iCargo == 0)
			{
				// if we just made a dropoff, bombard the city if we can
				if ((getGroup()->countNumUnitAIType(UNITAI_ATTACK_SEA) + getGroup()->countNumUnitAIType(UNITAI_RESERVE_SEA)) > 0)
				{
					bool bMissionPushed = false;

					if (AI_seaBombardRange(1))
					{
						bMissionPushed = true;
					}

					CvSelectionGroup* pOldGroup = getGroup();

						//Release any Warships to finish the job.
						getGroup()->AI_seperateAI(UNITAI_ATTACK_SEA);
						getGroup()->AI_seperateAI(UNITAI_RESERVE_SEA);

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       05/11/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
					if (pOldGroup == getGroup() && getUnitType() == UNITAI_ASSAULT_SEA)
					{
						if (AI_retreatToCity(true))
						{
							bMissionPushed = true;
						}
					}
*/
						// Fixed bug in next line with checking unit type instead of unit AI
						if (pOldGroup == getGroup() && AI_getUnitAIType() == UNITAI_ASSAULT_SEA)
						{
							// Need to be sure all units can move
							if( getGroup()->canAllMove() )
							{
								if (AI_retreatToCity(true))
								{
									bMissionPushed = true;
								}
							}
						}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

					if (bMissionPushed)
					{
						return;
					}
				}
			}
		}

		if ((iCargo > iTargetStackSize) || bFull)
		{
			bAttack = true;
		}
	}

	if (isBarbarian())
	{
		if (getGroup()->isFull())
		{
			if (AI_assaultSeaTransport(false))
			{
				return;
			}
		}
		else
		{
			if (AI_pickup(UNITAI_ATTACK_CITY))
			{
				return;
			}

			if (AI_pickup(UNITAI_ATTACK))
			{
				return;
			}
		}
	}
	else
	{
		bool bAttackBarbarian = false;

		if (GET_TEAM(getTeam()).getAtWarCount(true) == 0)
		{
			bAttackBarbarian = true;
		}

		if (bAttack)
		{
			FAssert(getGroup()->hasCargo());
			if (AI_assaultSeaTransport(bAttackBarbarian))
			{
				return;
			}
		}
	}

	if (bFull)
	{
		if (AI_group(UNITAI_ASSAULT_SEA, -1, /*iMaxOwnUnitAI*/ -1, -1, true))
		{
			return;
		}
	}
	else
	{
		if (AI_pickup(UNITAI_ATTACK_CITY))
		{
			return;
		}

		if (AI_pickup(UNITAI_ATTACK))
		{
			return;
		}

		if (AI_pickup(UNITAI_COUNTER))
		{
			return;
		}
	}

	// if we are in a city, and at/preparing land war, and we have cargo, unload, even if full, since AI_assaultSeaTransport already had its chance
	if (bIsCity && bLandWar && getGroup()->hasCargo())
	{
		getGroup()->unloadAll();
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	if (AI_retreatToCity(true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_missionarySeaMove()
{
	PROFILE_FUNC();

	if (getUnitAICargo(UNITAI_MISSIONARY) > 0)
	{
		if (AI_specialSeaTransportMissionary())
		{
			return;
		}
	}
	else if (!(getGroup()->hasCargo()))
	{
		if (AI_pillageRange(4))
		{
			return;
		}
	}

/*************************************************************************************************/
/** BETTER_BTS_AI_MOD merged Sephi         01/14/09                                jdog5000      */
/**                                                                                              */
/** Naval AI                                                                                     */
/*************************************************************************************************/
	if( !(getGroup()->isFull()) )
	{
		if( GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_LOAD_SPECIAL) > 0 )
		{
			// Wait for units on the way
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if (AI_pickup(UNITAI_MISSIONARY))
	{
		return;
	}

/*************************************************************************************************/
/** BETTER_BTS_AI_MOD                       END                                                  */
/*************************************************************************************************/

	if (AI_explore())
	{
		return;
	}

	if (AI_retreatToCity(true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::AI_settlerSeaMove()
{
	PROFILE_FUNC();

	bool bEmpty = !getGroup()->hasCargo();

	if (bEmpty)
	{
		if (AI_anyAttack(1, 65))
		{
			return;
		}
		if (AI_anyAttack(1, 40))
		{
			return;
		}
	}

	int iSettlerCount = getUnitAICargo(UNITAI_SETTLE);
	int iWorkerCount = getUnitAICargo(UNITAI_WORKER);

/*************************************************************************************************/
/** BETTER_BTS_AI_MOD                      12/07/08                                jdog5000      */
/**                                                                                              */
/** Naval AI merged Sephi                                                                        */
/*************************************************************************************************/
	if( hasCargo() && (iSettlerCount == 0) && (iWorkerCount == 0))
	{
		// Dump troop load at first oppurtunity after pick up
		if( plot()->isCity() && plot()->getOwnerINLINE() == getOwnerINLINE() )
		{
			getGroup()->unloadAll();
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
		else
		{
			if (AI_retreatToCity(true))
			{
				return;
			}

			if (AI_retreatToCity())
			{
				return;
			}
		}
	}
/*************************************************************************************************/
/** BETTER_BTS_AI_MOD                       END                                                  */
/*************************************************************************************************/

/*************************************************************************************************/
/** BETTER_BTS_AI_MOD                      06/02/09                                jdog5000      */
/**                                                                                              */
/** Settler AI merged Sephi                                                                      */
/*************************************************************************************************/
	// Don't send transport with settler and no defense
	if( (iSettlerCount > 0) && (iSettlerCount + iWorkerCount == cargoSpace()) )
	{
		// No defenders for settler
		if( plot()->isCity() && plot()->getOwnerINLINE() == getOwnerINLINE() )
		{
			getGroup()->unloadAll();
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	// Don't send transport with too many workers
	if(iSettlerCount==0 && (iWorkerCount > 0))
	{
		// No defenders for settler
		if( plot()->isCity() && plot()->getOwnerINLINE() == getOwnerINLINE() )
		{
			getGroup()->unloadAll();
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}

	if ((iSettlerCount > 0) && (isFull() ||
			((getUnitAICargo(UNITAI_CITY_DEFENSE) > 0) &&
			 (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_LOAD_SETTLER) == 0))))
/*************************************************************************************************/
/** BETTER_BTS_AI_MOD                       END                                                  */
/*************************************************************************************************/
	{
		if (AI_settlerSeaTransport())
		{
			return;
		}
	}
	else if ((getTeam() != plot()->getTeam()) && bEmpty)
	{
		if (AI_pillageRange(3))
		{
			return;
		}
	}

/**
	if (plot()->isCity() && !hasCargo())
	{
		AreaAITypes eAreaAI = area()->getAreaAIType(getTeam());
		if ((eAreaAI == AREAAI_ASSAULT) || (eAreaAI == AREAAI_ASSAULT_MASSING))
		{
			CvArea* pWaterArea = plot()->waterArea();
			FAssert(pWaterArea != NULL);
			if (pWaterArea != NULL)
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_totalWaterAreaUnitAIs(pWaterArea, UNITAI_SETTLER_SEA) > 1)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_unitValue(getUnitType(), UNITAI_ASSAULT_SEA, pWaterArea) > 0)
					{
						AI_setUnitAIType(UNITAI_ASSAULT_SEA);
						AI_assaultSeaMove();
						return;
					}
				}
			}
		}
	}
**/

	if ((iWorkerCount > 0)
		&& GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_LOAD_SETTLER) == 0)
	{
		if (isFull() || (iSettlerCount == 0))
		{
			if (AI_settlerSeaFerry())
			{
				return;
			}
		}
	}

/*************************************************************************************************/
/** BETTER_BTS_AI_MOD merged Sephi                     01/14/09                    jdog5000      */
/**                                                                                              */
/** Settler AI                                                                                   */
/*************************************************************************************************/
/* original bts code
	if (AI_pickup(UNITAI_SETTLE))
	{
		return;
	}
*/

	if( !(getGroup()->isFull()) )
	{
		if( GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_LOAD_SETTLER) > 0 )
		{
			// Wait for units on the way
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}

		if( iSettlerCount > 0 )
		{
			if (AI_pickup(UNITAI_COUNTER))
			{
				return;
			}
		}
		else
		{
			if (AI_pickup(UNITAI_SETTLE))
			{
				return;
			}
		}
	}
/*************************************************************************************************/
/** BETTER_BTS_AI_MOD                       END                                                  */
/*************************************************************************************************/

	if ((GC.getGame().getGameTurn() - getGameTurnCreated()) < 8)
	{
		if ((plot()->getPlotCity() == NULL) || GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(plot()->area(), UNITAI_SETTLE) == 0)
		{
			if (AI_explore())
			{
				return;
			}
		}
	}
/**
	if (AI_pickup(UNITAI_WORKER))
	{
		return;
	}
**/
	if (AI_retreatToCity(true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_spySeaMove()
{
	PROFILE_FUNC();

	CvCity* pCity;

	if (getUnitAICargo(UNITAI_SPY) > 0)
	{
		if (AI_specialSeaTransportSpy())
		{
			return;
		}

		pCity = plot()->getPlotCity();

		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_ATTACK_SPY, pCity->plot());
				return;
			}
		}
	}
	else if (!(getGroup()->hasCargo()))
	{
		if (AI_pillageRange(5))
		{
			return;
		}
	}

	if (AI_pickup(UNITAI_SPY))
	{
		return;
	}

	if (AI_retreatToCity(true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_carrierSeaMove()
{
	if (AI_seaRetreatFromCityDanger())
	{
		return;
	}

	if (AI_heal(50))
	{
		return;
	}

	if (!isEnemy(plot()->getTeam()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(this, MISSIONAI_GROUP) > 0)
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}
	else
	{
		if (AI_seaBombardRange(1))
		{
			return;
		}
	}

	if (AI_group(UNITAI_CARRIER_SEA, -1, /*iMaxOwnUnitAI*/ 1))
	{
		return;
	}

	if (getGroup()->countNumUnitAIType(UNITAI_ATTACK_SEA) + getGroup()->countNumUnitAIType(UNITAI_ESCORT_SEA) == 0)
	{
		if (plot()->isCity() && plot()->getOwnerINLINE() == getOwnerINLINE())
		{
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
		if (AI_retreatToCity())
		{
			return;
		}
	}

	if (getCargo() > 0)
	{
		if (AI_carrierSeaTransport())
		{
			return;
		}

		if (AI_blockade())
		{
			return;
		}

		if (AI_shadow(UNITAI_ASSAULT_SEA))
		{
			return;
		}
	}

	if (AI_travelToUpgradeCity())
	{
		return;
	}

	if (AI_retreatToCity(true))
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_missileCarrierSeaMove()
{
	bool bIsStealth = (getInvisibleType() != NO_INVISIBLE);

	if (AI_seaRetreatFromCityDanger())
	{
		return;
	}

	if (plot()->isCity() && plot()->getTeam() == getTeam())
	{
		if (AI_heal())
		{
			return;
		}
	}

	if (((plot()->getTeam() != getTeam()) && getGroup()->hasCargo()) || getGroup()->AI_isFull())
	{
		if (bIsStealth)
		{
			if (AI_carrierSeaTransport())
			{
				return;
			}
		}
		else
		{
			if (AI_shadow(UNITAI_ASSAULT_SEA, 1, 50))
			{
				return;
			}

			if (AI_carrierSeaTransport())
			{
				return;
			}
		}
	}
//	if (AI_pickup(UNITAI_ICBM))
//	{
//		return;
//	}
//
//	if (AI_pickup(UNITAI_MISSILE_AIR))
//	{
//		return;
//	}
	if (AI_retreatToCity())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
}


void CvUnitAI::AI_attackAirMove()
{
	if (AI_airRetreatFromCityDanger())
	{
		return;
	}

	if (AI_airAttackDamagedSkip())
	{
		return;
	}

	if (getDamage() > 0)
	{
		if (AI_airBombPlots())
		{
			return;
		}
		if (AI_airStrike())
		{
			return;
		}
	}

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvArea* pArea = area();
	int iAttackValue = kPlayer.AI_unitValue(getUnitType(), UNITAI_ATTACK_AIR, pArea);
	int iCarrierValue = kPlayer.AI_unitValue(getUnitType(), UNITAI_CARRIER_AIR, pArea);
	if (iCarrierValue > 0)
	{
		int iCarriers = kPlayer.AI_totalUnitAIs(UNITAI_CARRIER_SEA);
		if (iCarriers > 0)
		{
			UnitTypes eBestCarrierUnit = NO_UNIT;
			kPlayer.AI_bestAreaUnitAIValue(UNITAI_CARRIER_SEA, NULL, &eBestCarrierUnit);
			if (eBestCarrierUnit != NO_UNIT)
			{
				int iCarrierAirNeeded = iCarriers * GC.getUnitInfo(eBestCarrierUnit).getCargoSpace();
				if (kPlayer.AI_totalUnitAIs(UNITAI_CARRIER_AIR) < iCarrierAirNeeded)
				{
					AI_setUnitAIType(UNITAI_CARRIER_AIR);
					getGroup()->pushMission(MISSION_SKIP);
					return;
				}
			}
		}
	}

	int iDefenseValue = kPlayer.AI_unitValue(getUnitType(), UNITAI_DEFENSE_AIR, pArea);
	if (iDefenseValue > iAttackValue)
	{
		if (kPlayer.AI_bestAreaUnitAIValue(UNITAI_ATTACK_AIR, pArea) > iAttackValue)
		{
			AI_setUnitAIType(UNITAI_DEFENSE_AIR);
			getGroup()->pushMission(MISSION_SKIP);
			return;
		}
	}


	if (AI_airBombDefenses())
	{
		return;
	}

	if (GC.getGameINLINE().getSorenRandNum(4, "AI Air Attack Move") == 0)
	{
		if (AI_airBombPlots())
		{
			return;
		}
	}

	if (AI_airStrike())
	{
		return;
	}

	if (canAirAttack())
	{
		if (AI_airOffensiveCity())
		{
			return;
		}
	}

	if (canRecon(plot()))
	{
		if (AI_exploreAir())
		{
			return;
		}
	}

	if (canAirDefend())
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_defenseAirMove()
{
	if (AI_airRetreatFromCityDanger())
	{
		return;
	}

	if (AI_airAttackDamagedSkip())
	{
		return;
	}

	if ((GC.getGameINLINE().getSorenRandNum(2, "AI Air Defense Move") == 0))
	{
		CvCity* pCity = plot()->getPlotCity();

		if ((pCity != NULL) && pCity->AI_isDanger())
		{
			if (AI_airStrike())
			{
				return;
			}
		}
		else
		{
			if (AI_airBombDefenses())
			{
				return;
			}

			if (AI_airStrike())
			{
				return;
			}

			if (AI_getBirthmark() % 2 == 0)
			{
				if (AI_airBombPlots())
				{
					return;
				}
			}
		}

		if (AI_travelToUpgradeCity())
		{
			return;
		}
	}

	bool bNoWar = (GET_TEAM(getTeam()).getAtWarCount(false) == 0);

	if (canRecon(plot()))
	{
		if (GC.getGame().getSorenRandNum(bNoWar ? 2 : 4, "AI defensive air recon") == 0)
		{
			if (AI_exploreAir())
			{
				return;
			}
		}
	}

	if (AI_airDefensiveCity())
	{
		return;
	}

	if (canAirDefend())
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_carrierAirMove()
{
	// XXX maybe protect land troops?

	if (getDamage() > 0)
	{
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	if (isCargo())
	{
		int iRand = GC.getGameINLINE().getSorenRandNum(3, "AI Air Carrier Move");

		if (iRand == 2 && canAirDefend())
		{
			getGroup()->pushMission(MISSION_AIRPATROL);
			return;
		}
		else if (AI_airBombDefenses())
		{
			return;
		}
		else if (iRand == 1)
		{
			if (AI_airBombPlots())
			{
				return;
			}

			if (AI_airStrike())
			{
				return;
			}
		}
		else
		{
			if (AI_airStrike())
			{
				return;
			}

			if (AI_airBombPlots())
			{
				return;
			}
		}

		if (AI_travelToUpgradeCity())
		{
			return;
		}

		if (canAirDefend())
		{
			getGroup()->pushMission(MISSION_AIRPATROL);
			return;
		}
		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	if (AI_airCarrier())
	{
		return;
	}

	if (AI_airDefensiveCity())
	{
		return;
	}

	if (canAirDefend())
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_missileAirMove()
{
	CvCity* pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->AI_isDanger())
		{
			if (!(pCity->AI_isDefended()))
			{
				if (AI_airOffensiveCity())
				{
					return;
				}
			}
		}
	}

	if (isCargo())
	{
		int iRand = GC.getGameINLINE().getSorenRandNum(3, "AI Air Missile plot bombing");
		if (iRand != 0)
		{
			if (AI_airBombPlots())
			{
				return;
			}
		}

		iRand = GC.getGameINLINE().getSorenRandNum(3, "AI Air Missile Carrier Move");
		if (iRand == 0)
		{
			if (AI_airBombDefenses())
			{
				return;
			}

			if (AI_airStrike())
			{
				return;
			}
		}
		else
		{
			if (AI_airStrike())
			{
				return;
			}

			if (AI_airBombDefenses())
			{
				return;
			}
		}

		if (AI_airBombPlots())
		{
			return;
		}

		getGroup()->pushMission(MISSION_SKIP);
		return;
	}

	if (AI_airStrike())
	{
		return;
	}

	if (AI_missileLoad(UNITAI_MISSILE_CARRIER_SEA))
	{
		return;
	}

	if (AI_missileLoad(UNITAI_RESERVE_SEA, 1))
	{
		return;
	}

	if (AI_missileLoad(UNITAI_ATTACK_SEA, 1))
	{
		return;
	}

	if (AI_airBombDefenses())
	{
		return;
	}

	if (!isCargo())
	{
		if (AI_airOffensiveCity())
		{
			return;
		}
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_networkAutomated()
{
	FAssertMsg(canBuildRoute(), "canBuildRoute is expected to be true");

	if (!(getGroup()->canDefend()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
		{
			if (AI_retreatToCity()) // XXX maybe not do this??? could be working productively somewhere else...
			{
				return;
			}
		}
	}

	if (AI_improveBonus(20))
	{
		return;
	}

	if (AI_improveBonus(10))
	{
		return;
	}

	if (AI_connectBonus())
	{
		return;
	}

	if (AI_connectCity())
	{
		return;
	}

	if (AI_improveBonus())
	{
		return;
	}

	if (AI_routeTerritory(true))
	{
		return;
	}

	if (AI_connectBonus(false))
	{
		return;
	}

	if (AI_routeCity())
	{
		return;
	}

	if (AI_routeTerritory())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


void CvUnitAI::AI_cityAutomated()
{
	CvCity* pCity;

	if (!(getGroup()->canDefend()))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot()) > 0)
		{
			if (AI_retreatToCity()) // XXX maybe not do this??? could be working productively somewhere else...
			{
				return;
			}
		}
	}

	pCity = NULL;

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		pCity = plot()->getWorkingCity();
	}

	if (pCity == NULL)
	{
		pCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE()); // XXX do team???
	}

	if (pCity != NULL)
	{
		if (AI_improveCity(pCity))
		{
			return;
		}
	}

	if (AI_retreatToCity())
	{
		return;
	}

	if (AI_safety())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}


// XXX make sure we include any new UnitAITypes...
int CvUnitAI::AI_promotionValue(PromotionTypes ePromotion)
{
	int iValue;
	int iTemp;
	int iExtra;
	int iI;

	iValue = 0;

	if (GC.getPromotionInfo(ePromotion).isLeader())
	{
		// Don't consume the leader as a regular promotion
		return 0;
	}

	if (GC.getPromotionInfo(ePromotion).isBlitz())
	{
		if ((AI_getUnitAIType() == UNITAI_RESERVE  && baseMoves() > 1) ||
			AI_getUnitAIType() == UNITAI_PARADROP)
		{
			iValue += 10;
		}
		else
		{

//FfH: Modified by Kael 06/28/2008
//			iValue += 2;
			iValue += 3 * baseMoves();
//FfH: End Modify

		}
	}

	if (GC.getPromotionInfo(ePromotion).isAmphib())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			  (AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 5;
		}
		else
		{
			iValue++;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isRiver())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			  (AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 5;
		}
		else
		{
			iValue++;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isEnemyRoute())
	{
		if (AI_getUnitAIType() == UNITAI_PILLAGE)
		{
			iValue += 40;
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			       (AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 20;
		}
		else if (AI_getUnitAIType() == UNITAI_PARADROP)
		{
			iValue += 10;
		}
		else
		{
			iValue += 4;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isAlwaysHeal())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			  (AI_getUnitAIType() == UNITAI_ATTACK_CITY) ||
				(AI_getUnitAIType() == UNITAI_PILLAGE) ||
				(AI_getUnitAIType() == UNITAI_COUNTER) ||
				(AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
				(AI_getUnitAIType() == UNITAI_PIRATE_SEA) ||
				(AI_getUnitAIType() == UNITAI_ESCORT_SEA) ||
				(AI_getUnitAIType() == UNITAI_PARADROP))
		{
			iValue += 10;
		}
		else
		{
			iValue += 8;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isHillsDoubleMove())
	{
		if (AI_getUnitAIType() == UNITAI_EXPLORE)
		{
			iValue += 20;
		}
		else
		{
			iValue += 10;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isImmuneToFirstStrikes()
		&& !immuneToFirstStrikes())
	{
		if ((AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += 12;
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK))
		{
			iValue += 8;
		}
		else
		{
			iValue += 4;
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getVisibilityChange();
	if ((AI_getUnitAIType() == UNITAI_EXPLORE_SEA) ||
		(AI_getUnitAIType() == UNITAI_EXPLORE))
	{
		iValue += (iTemp * 40);
	}
	else if (AI_getUnitAIType() == UNITAI_PIRATE_SEA)
	{
		iValue += (iTemp * 20);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getMovesChange();
	if ((AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
		(AI_getUnitAIType() == UNITAI_PIRATE_SEA) ||
		  (AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
		  (AI_getUnitAIType() == UNITAI_ESCORT_SEA) ||
			(AI_getUnitAIType() == UNITAI_EXPLORE_SEA) ||
			(AI_getUnitAIType() == UNITAI_ASSAULT_SEA) ||
			(AI_getUnitAIType() == UNITAI_SETTLER_SEA) ||
			(AI_getUnitAIType() == UNITAI_PILLAGE) ||
			(AI_getUnitAIType() == UNITAI_ATTACK) ||
			(AI_getUnitAIType() == UNITAI_PARADROP))
	{
		iValue += (iTemp * 20);
	}
	else
	{
		iValue += (iTemp * 4);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getMoveDiscountChange();
	if (AI_getUnitAIType() == UNITAI_PILLAGE)
	{
		iValue += (iTemp * 10);
	}
	else
	{
		iValue += (iTemp * 2);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getAirRangeChange();
	if (AI_getUnitAIType() == UNITAI_ATTACK_AIR ||
		AI_getUnitAIType() == UNITAI_CARRIER_AIR)
	{
		iValue += (iTemp * 20);
	}
	else if (AI_getUnitAIType() == UNITAI_DEFENSE_AIR)
	{
		iValue += (iTemp * 10);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getInterceptChange();
	if (AI_getUnitAIType() == UNITAI_DEFENSE_AIR)
	{
		iValue += (iTemp * 3);
	}
	else if (AI_getUnitAIType() == UNITAI_CITY_SPECIAL || AI_getUnitAIType() == UNITAI_CARRIER_AIR)
	{
		iValue += (iTemp * 2);
	}
	else
	{
		iValue += (iTemp / 10);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getEvasionChange();
	if (AI_getUnitAIType() == UNITAI_ATTACK_AIR || AI_getUnitAIType() == UNITAI_CARRIER_AIR)
	{
		iValue += (iTemp * 3);
	}
	else
	{
		iValue += (iTemp / 10);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getFirstStrikesChange() * 2;
	iTemp += GC.getPromotionInfo(ePromotion).getChanceFirstStrikesChange();
	if ((AI_getUnitAIType() == UNITAI_RESERVE) ||
		  (AI_getUnitAIType() == UNITAI_COUNTER) ||
			(AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
			(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
			(AI_getUnitAIType() == UNITAI_CITY_SPECIAL) ||
			(AI_getUnitAIType() == UNITAI_ATTACK))
	{
		iTemp *= 8;
		iExtra = getExtraChanceFirstStrikes() + getExtraFirstStrikes() * 2;
		iTemp *= 100 + iExtra * 15;
		iTemp /= 100;
		iValue += iTemp;
	}
	else
	{
		iValue += (iTemp * 5);
	}


	iTemp = GC.getPromotionInfo(ePromotion).getWithdrawalChange();
	if (iTemp != 0)
	{
		iExtra = (m_pUnitInfo->getWithdrawalProbability() + (getExtraWithdrawal() * 4));
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		if ((AI_getUnitAIType() == UNITAI_ATTACK_CITY))
		{
			iValue += (iTemp * 4) / 3;
		}
		else if ((AI_getUnitAIType() == UNITAI_COLLATERAL) ||
			  (AI_getUnitAIType() == UNITAI_RESERVE) ||
			  (AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
			  getLeaderUnitType() != NO_UNIT)
		{
			iValue += iTemp * 1;
		}
		else
		{
			iValue += (iTemp / 4);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCollateralDamageChange();
	if (iTemp != 0)
	{
		iExtra = (getExtraCollateralDamage());//collateral has no strong synergy (not like retreat)
		iTemp *= (100 + iExtra);
		iTemp /= 100;

		if (AI_getUnitAIType() == UNITAI_COLLATERAL)
		{
			iValue += (iTemp * 1);
		}
		else if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
		{
			iValue += ((iTemp * 2) / 3);
		}
		else
		{
			iValue += (iTemp / 8);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getBombardRateChange();
	if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
	{
		iValue += (iTemp * 2);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getEnemyHealChange();
	if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		(AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
		(AI_getUnitAIType() == UNITAI_PARADROP) ||
		(AI_getUnitAIType() == UNITAI_PIRATE_SEA))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getNeutralHealChange();
	iValue += (iTemp / 8);

	iTemp = GC.getPromotionInfo(ePromotion).getFriendlyHealChange();
	if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		  (AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		  (AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}


    if (getDamage() > 0)
    {
        iTemp = GC.getPromotionInfo(ePromotion).getSameTileHealChange() + getSameTileHeal();
        iExtra = getSameTileHeal();

        iTemp *= (100 + iExtra * 5);
        iTemp /= 100;

        if (iTemp > 0)
        {
            if (healRate(plot()) < iTemp)
            {
                iValue += iTemp * ((getGroup()->getNumUnits() > 4) ? 4 : 2);
            }
            else
            {
                iValue += (iTemp / 8);
            }
        }

        iTemp = GC.getPromotionInfo(ePromotion).getAdjacentTileHealChange();
        iExtra = getAdjacentTileHeal();
        iTemp *= (100 + iExtra * 5);
        iTemp /= 100;
        if (getSameTileHeal() >= iTemp)
        {
            iValue += (iTemp * ((getGroup()->getNumUnits() > 9) ? 4 : 2));
        }
        else
        {
            iValue += (iTemp / 4);
        }
    }

	iTemp = GC.getPromotionInfo(ePromotion).getCombatPercent();
	if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		(AI_getUnitAIType() == UNITAI_COUNTER) ||
		(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		  (AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
		  (AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
			(AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
			(AI_getUnitAIType() == UNITAI_PARADROP) ||
			(AI_getUnitAIType() == UNITAI_PIRATE_SEA) ||
			(AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
			(AI_getUnitAIType() == UNITAI_ESCORT_SEA) ||
			(AI_getUnitAIType() == UNITAI_CARRIER_SEA) ||
			(AI_getUnitAIType() == UNITAI_ATTACK_AIR) ||
			(AI_getUnitAIType() == UNITAI_CARRIER_AIR))
	{
		iValue += (iTemp * 2);
	}
	else
	{
		iValue += (iTemp * 1);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCityAttackPercent();
	if (iTemp != 0)
	{
		if (m_pUnitInfo->getUnitAIType(UNITAI_ATTACK) || m_pUnitInfo->getUnitAIType(UNITAI_ATTACK_CITY) || m_pUnitInfo->getUnitAIType(UNITAI_ATTACK_CITY_LEMMING))
		{
			iExtra = (m_pUnitInfo->getCityAttackModifier() + (getExtraCityAttackPercent() * 2));
			iTemp *= (100 + iExtra);
			iTemp /= 100;
			if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
			{
				iValue += (iTemp * 1);
			}
			else
			{
				iValue -= iTemp / 4;
			}
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCityDefensePercent();
	if (iTemp != 0)
	{
		if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
			  (AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
		{
			iExtra = m_pUnitInfo->getCityDefenseModifier() + (getExtraCityDefensePercent() * 2);
			iValue += ((iTemp * (100 + iExtra)) / 100);
		}
		else
		{
			iValue += (iTemp / 4);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getHillsAttackPercent();
	if (iTemp != 0)
	{
		iExtra = getExtraHillsAttackPercent();
		iTemp *= (100 + iExtra * 2);
		iTemp /= 100;
		if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			(AI_getUnitAIType() == UNITAI_COUNTER))
		{
			iValue += (iTemp / 4);
		}
		else
		{
			iValue += (iTemp / 16);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getHillsDefensePercent();
	if (iTemp != 0)
	{
		iExtra = (m_pUnitInfo->getHillsDefenseModifier() + (getExtraHillsDefensePercent() * 2));
		iTemp *= (100 + iExtra);
		iTemp /= 100;
		if (AI_getUnitAIType() == UNITAI_CITY_DEFENSE)
		{
			if (plot()->isCity() && plot()->isHills())
			{
				iValue += (iTemp * 4) / 3;
			}
		}
		else if (AI_getUnitAIType() == UNITAI_COUNTER)
		{
			if (plot()->isHills())
			{
				iValue += (iTemp / 4);
			}
			else
			{
				iValue++;
			}
		}
		else
		{
			iValue += (iTemp / 16);
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getRevoltProtection();
	if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		(AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
	{
		if (iTemp > 0)
		{
			PlayerTypes eOwner = plot()->calculateCulturalOwner();
			if (eOwner != NO_PLAYER && GET_PLAYER(eOwner).getTeam() != GET_PLAYER(getOwnerINLINE()).getTeam())
			{
				iValue += (iTemp / 2);
			}
		}
	}

	iTemp = GC.getPromotionInfo(ePromotion).getCollateralDamageProtection();
	if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		(AI_getUnitAIType() == UNITAI_CITY_SPECIAL))
	{
		iValue += (iTemp / 3);
	}
	else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		(AI_getUnitAIType() == UNITAI_COUNTER))
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 8);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getPillageChange();
	if (AI_getUnitAIType() == UNITAI_PILLAGE ||
		AI_getUnitAIType() == UNITAI_ATTACK_SEA ||
		AI_getUnitAIType() == UNITAI_PIRATE_SEA)
	{
		iValue += (iTemp / 4);
	}
	else
	{
		iValue += (iTemp / 16);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getUpgradeDiscount();
	iValue += (iTemp / 16);

	iTemp = GC.getPromotionInfo(ePromotion).getExperiencePercent();
	if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
		(AI_getUnitAIType() == UNITAI_ATTACK_SEA) ||
		(AI_getUnitAIType() == UNITAI_PIRATE_SEA) ||
		(AI_getUnitAIType() == UNITAI_RESERVE_SEA) ||
		(AI_getUnitAIType() == UNITAI_ESCORT_SEA) ||
		(AI_getUnitAIType() == UNITAI_CARRIER_SEA) ||
		(AI_getUnitAIType() == UNITAI_MISSILE_CARRIER_SEA))
	{
		iValue += (iTemp * 1);
	}
	else
	{
		iValue += (iTemp / 2);
	}

	iTemp = GC.getPromotionInfo(ePromotion).getKamikazePercent();
	if (AI_getUnitAIType() == UNITAI_ATTACK_CITY)
	{
		iValue += (iTemp / 16);
	}
	else
	{
		iValue += (iTemp / 64);
	}

	for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
	{
		iTemp = GC.getPromotionInfo(ePromotion).getTerrainAttackPercent(iI);
		if (iTemp != 0)
		{
			iExtra = getExtraTerrainAttackPercent((TerrainTypes)iI);
			iTemp *= (100 + iExtra * 2);
			iTemp /= 100;
			if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
				(AI_getUnitAIType() == UNITAI_COUNTER))
			{
				iValue += (iTemp / 4);
			}
			else
			{
				iValue += (iTemp / 16);
			}
		}

		iTemp = GC.getPromotionInfo(ePromotion).getTerrainDefensePercent(iI);
		if (iTemp != 0)
		{
			iExtra =  getExtraTerrainDefensePercent((TerrainTypes)iI);
			iTemp *= (100 + iExtra);
			iTemp /= 100;
			if (AI_getUnitAIType() == UNITAI_COUNTER)
			{
				if (plot()->getTerrainType() == (TerrainTypes)iI)
				{
					iValue += (iTemp / 4);
				}
				else
				{
					iValue++;
				}
			}
			else
			{
				iValue += (iTemp / 16);
			}
		}

		if (GC.getPromotionInfo(ePromotion).getTerrainDoubleMove(iI))
		{
			if (AI_getUnitAIType() == UNITAI_EXPLORE)
			{
				iValue += 20;
			}
			else if ((AI_getUnitAIType() == UNITAI_ATTACK) || (AI_getUnitAIType() == UNITAI_PILLAGE))
			{
				iValue += 10;
			}
			else
			{
			    iValue += 1;
			}
		}
	}

	for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
	{
		iTemp = GC.getPromotionInfo(ePromotion).getFeatureAttackPercent(iI);
		if (iTemp != 0)
		{
			iExtra = getExtraFeatureAttackPercent((FeatureTypes)iI);
			iTemp *= (100 + iExtra * 2);
			iTemp /= 100;
			if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
				(AI_getUnitAIType() == UNITAI_COUNTER))
			{
				iValue += (iTemp / 4);
			}
			else
			{
				iValue += (iTemp / 16);
			}
		}

		iTemp = GC.getPromotionInfo(ePromotion).getFeatureDefensePercent(iI);;
		if (iTemp != 0)
		{
			iExtra = getExtraFeatureDefensePercent((FeatureTypes)iI);
			iTemp *= (100 + iExtra * 2);
			iTemp /= 100;

			if (!noDefensiveBonus())
			{
				if (AI_getUnitAIType() == UNITAI_COUNTER)
				{
					if (plot()->getFeatureType() == (FeatureTypes)iI)
					{
						iValue += (iTemp / 4);
					}
					else
					{
						iValue++;
					}
				}
				else
				{
					iValue += (iTemp / 16);
				}
			}
		}

		if (GC.getPromotionInfo(ePromotion).getFeatureDoubleMove(iI))
		{
			if (AI_getUnitAIType() == UNITAI_EXPLORE)
			{
				iValue += 20;
			}
			else if ((AI_getUnitAIType() == UNITAI_ATTACK) || (AI_getUnitAIType() == UNITAI_PILLAGE))
			{
				iValue += 10;
			}
			else
			{
			    iValue += 1;
			}
		}
	}

    int iOtherCombat = 0;
    int iSameCombat = 0;

    for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
    {
        if ((UnitCombatTypes)iI == getUnitCombatType())
        {
            iSameCombat += unitCombatModifier((UnitCombatTypes)iI);
        }
        else
        {
            iOtherCombat += unitCombatModifier((UnitCombatTypes)iI);
        }
    }

	for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
	{
		iTemp = GC.getPromotionInfo(ePromotion).getUnitCombatModifierPercent(iI);
		int iCombatWeight = 0;
        //Fighting their own kind
        if ((UnitCombatTypes)iI == getUnitCombatType())
        {
            if (iSameCombat >= iOtherCombat)
            {
                iCombatWeight = 70;//"axeman takes formation"
            }
            else
            {
                iCombatWeight = 30;
            }
        }
        else
        {
            //fighting other kinds
            if (unitCombatModifier((UnitCombatTypes)iI) > 10)
            {
                iCombatWeight = 70;//"spearman takes formation"
            }
            else
            {
                iCombatWeight = 30;
            }
        }

		iCombatWeight *= GET_PLAYER(getOwnerINLINE()).AI_getUnitCombatWeight((UnitCombatTypes)iI);
		iCombatWeight /= 100;

		if ((AI_getUnitAIType() == UNITAI_COUNTER) || (AI_getUnitAIType() == UNITAI_CITY_COUNTER))
		{
		    iValue += (iTemp * iCombatWeight) / 50;
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			       (AI_getUnitAIType() == UNITAI_RESERVE))
		{
			iValue += (iTemp * iCombatWeight) / 100;
		}
		else
		{
			iValue += (iTemp * iCombatWeight) / 200;
		}
	}

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		//WTF? why float and cast to int?
		//iTemp = ((int)((GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI) + getExtraDomainModifier((DomainTypes)iI)) * 100.0f));
		iTemp = GC.getPromotionInfo(ePromotion).getDomainModifierPercent(iI);
		if (AI_getUnitAIType() == UNITAI_COUNTER)
		{
			iValue += (iTemp * 1);
		}
		else if ((AI_getUnitAIType() == UNITAI_ATTACK) ||
			       (AI_getUnitAIType() == UNITAI_RESERVE))
		{
			iValue += (iTemp / 2);
		}
		else
		{
			iValue += (iTemp / 8);
		}
	}

//FfH: Added by Kael 07/30/2007
	iTemp = GC.getPromotionInfo(ePromotion).getDefensiveStrikeChance() + GC.getPromotionInfo(ePromotion).getDefensiveStrikeDamage();
	iTemp /= 2;
	if ((AI_getUnitAIType() == UNITAI_CITY_DEFENSE) ||
		(AI_getUnitAIType() == UNITAI_CITY_COUNTER) ||
		(AI_getUnitAIType() == UNITAI_COUNTER))
    {
        iTemp *= 2;
    }
    iValue += iTemp;
    iValue += GC.getPromotionInfo(ePromotion).getAIWeight();
//FfH: End Add

	if (iValue > 0)
	{
		iValue += GC.getGameINLINE().getSorenRandNum(15, "AI Promote");
	}

	return iValue;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_shadow(UnitAITypes eUnitAI, int iMax, int iMaxRatio, bool bWithCargoOnly)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestUnit = NULL;

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (pLoopUnit != this)
		{
			if (AI_plotValid(pLoopUnit->plot()))
			{
				if (pLoopUnit->isGroupHead())
				{
					if (!(pLoopUnit->isCargo()))
					{
						if (pLoopUnit->AI_getUnitAIType() == eUnitAI)
						{
							if (pLoopUnit->getGroup()->baseMoves() <= getGroup()->baseMoves())
							{
								if (!bWithCargoOnly || pLoopUnit->getGroup()->hasCargo())
								{
									int iShadowerCount = GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, MISSIONAI_SHADOW, getGroup());
									if (((-1 == iMax) || (iShadowerCount < iMax)) &&
										 ((-1 == iMaxRatio) || (iShadowerCount == 0) || (((100 * iShadowerCount) / std::max(1, pLoopUnit->getGroup()->countNumUnitAIType(eUnitAI))) <= iMaxRatio)))
									{
										if (!(pLoopUnit->plot()->isVisibleEnemyUnit(this)))
										{
											if (generatePath(pLoopUnit->plot(), 0, true, &iPathTurns))
											{
												//if (iPathTurns <= iMaxPath) XXX
												{
													iValue = 1 + pLoopUnit->getGroup()->getCargo();
													iValue *= 1000;
													iValue /= 1 + iPathTurns;

													if (iValue > iBestValue)
													{
														iBestValue = iValue;
														pBestUnit = pLoopUnit;
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
			}
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_SHADOW, NULL, pBestUnit);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), 0, false, false, MISSIONAI_SHADOW, NULL, pBestUnit);
			return true;
		}
	}

	return false;
}


// Returns true if a group was joined or a mission was pushed...
bool CvUnitAI::AI_group(UnitAITypes eUnitAI, int iMaxGroup, int iMaxOwnUnitAI, int iMinUnitAI, bool bIgnoreFaster, bool bIgnoreOwnUnitType, bool bStackOfDoom, int iMaxPath, bool bAllowRegrouping)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	// if we are on a transport, then do not regroup
	if (isCargo())
	{
		return false;
	}

	if (!bAllowRegrouping)
	{
		if (getGroup()->getNumUnits() > 1)
		{
			return false;
		}
	}

	if ((getDomainType() == DOMAIN_LAND) && !canMoveAllTerrain())
	{
		if (area()->getNumAIUnits(getOwnerINLINE(), eUnitAI) == 0)
		{
			return false;
		}
	}

	if (!AI_canGroupWithAIType(eUnitAI))
	{
		return false;
	}

	iBestValue = MAX_INT;
	pBestUnit = NULL;

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		CvSelectionGroup* pLoopGroup = pLoopUnit->getGroup();
		CvPlot* pPlot = pLoopUnit->plot();
		if (AI_plotValid(pPlot))
		{
			if (iMaxPath > 0 || pPlot == plot())
			{
				if (!isEnemy(pPlot->getTeam()))
				{
					if (AI_allowGroup(pLoopUnit, eUnitAI))
					{
						if ((iMaxGroup == -1) || ((pLoopGroup->getNumUnits() + GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, MISSIONAI_GROUP, getGroup())) <= (iMaxGroup + ((bStackOfDoom) ? AI_stackOfDoomExtra() : 0))))
						{
							if ((iMaxOwnUnitAI == -1) || (pLoopGroup->countNumUnitAIType(AI_getUnitAIType()) <= (iMaxOwnUnitAI + ((bStackOfDoom) ? AI_stackOfDoomExtra() : 0))))
							{
								if ((iMinUnitAI == -1) || (pLoopGroup->countNumUnitAIType(eUnitAI) >= iMinUnitAI))
								{
									if (!bIgnoreFaster || (pLoopUnit->getGroup()->baseMoves() <= baseMoves()))
									{
										if (!bIgnoreOwnUnitType || (pLoopUnit->getUnitType() != getUnitType()))
										{
											if (!(pPlot->isVisibleEnemyUnit(this)))
											{
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We don't have to check for a path to distant shores if we want to move only short distance  **/
/**	anyway, so approx maximum distance for a possible path by iMaxPath                			**/
/*************************************************************************************************/
/** Orig Code Start
												if (generatePath(pPlot, 0, true, &iPathTurns))
												{
													if (iPathTurns <= iMaxPath)
													{
														iValue = 1000 * (iPathTurns + 1);
														iValue *= 4 + pLoopGroup->getCargo();
														iValue /= pLoopGroup->getNumUnits();


														if (iValue < iBestValue)
														{
															iBestValue = iValue;
															pBestUnit = pLoopUnit;
														}
													}
												}
/** Orig Code End **/

                                                int XDist=pPlot->getX_INLINE() - plot()->getX_INLINE();
                                                int YDist=pPlot->getY_INLINE() - plot()->getY_INLINE();
                                                if (((XDist*XDist)+(YDist*YDist))<iMaxPath*iMaxPath*4)
                                                {
                                                    if (generatePath(pPlot, 0, true, &iPathTurns))
                                                    {
                                                        if (iPathTurns <= iMaxPath)
                                                        {
                                                            iValue = 1000 * (iPathTurns + 1);
                                                            iValue *= 4 + pLoopGroup->getCargo();
                                                            iValue /= pLoopGroup->getNumUnits();


                                                            if (iValue < iBestValue)
                                                            {
                                                                iBestValue = iValue;
                                                                pBestUnit = pLoopUnit;
                                                            }
                                                        }
                                                    }
                                                }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

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
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			joinGroup(pBestUnit->getGroup());
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), 0, false, false, MISSIONAI_GROUP, NULL, pBestUnit);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_groupMergeRange(UnitAITypes eUnitAI, int iMaxRange, bool bBiggerOnly, bool bAllowRegrouping, bool bIgnoreFaster)
{
	PROFILE_FUNC();


 	// if we are on a transport, then do not regroup
	if (isCargo())
	{
		return false;
	}

   if (!bAllowRegrouping)
	{
		if (getGroup()->getNumUnits() > 1)
		{
			return false;
		}
	}

	if ((getDomainType() == DOMAIN_LAND) && !canMoveAllTerrain())
	{
		if (area()->getNumAIUnits(getOwnerINLINE(), eUnitAI) == 0)
		{
			return false;
		}
	}

	if (!AI_canGroupWithAIType(eUnitAI))
	{
		return false;
	}

	// cached values
	CvPlot* pPlot = plot();
	CvSelectionGroup* pGroup = getGroup();

	// best match
	CvUnit* pBestUnit = NULL;
	int iBestValue = MAX_INT;
	// iterate over plots at each range
	for (int iDX = -(iMaxRange); iDX <= iMaxRange; iDX++)
	{
		for (int iDY = -(iMaxRange); iDY <= iMaxRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL && pLoopPlot->getArea() == pPlot->getArea())
			{
				CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

					CvSelectionGroup* pLoopGroup = pLoopUnit->getGroup();

					if (AI_allowGroup(pLoopUnit, eUnitAI))
					{
						if (!bIgnoreFaster || (pLoopUnit->getGroup()->baseMoves() <= baseMoves()))
						{
							if (!bBiggerOnly || (pLoopGroup->getNumUnits() >= pGroup->getNumUnits()))
							{
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We don't have to check for a path to distant shores if we want to move only short distance  **/
/**	anyway, so approx maximum distance for a possible path by iMaxPath                			**/
/*************************************************************************************************/
/** Orig Code Start
								int iPathTurns;
								if (generatePath(pLoopPlot, 0, true, &iPathTurns))
								{
									if (iPathTurns <= (iMaxRange + 2))
									{
										int iValue = 1000 * (iPathTurns + 1);
										iValue /= pLoopGroup->getNumUnits();

										if (iValue < iBestValue)
										{
											iBestValue = iValue;
											pBestUnit = pLoopUnit;
										}
									}
								}
/** Orig Code End **/
                                int XDist=pLoopPlot->getX_INLINE() - plot()->getX_INLINE();
                                int YDist=pLoopPlot->getY_INLINE() - plot()->getY_INLINE();
                                if (((XDist*XDist)+(YDist*YDist))<(iMaxRange + 2)*(iMaxRange + 2)*4)
                                {
                                    int iPathTurns;
                                    if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                    {
                                        if (iPathTurns <= (iMaxRange + 2))
                                        {
                                            int iValue = 1000 * (iPathTurns + 1);
                                            iValue /= pLoopGroup->getNumUnits();

                                            if (iValue < iBestValue)
                                            {
                                                iBestValue = iValue;
                                                pBestUnit = pLoopUnit;
                                            }
                                        }
                                    }
                                }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
							}
						}
					}
				}
			}
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			pGroup->mergeIntoGroup(pBestUnit->getGroup());
			return true;
		}
		else
		{
			pGroup->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), 0, false, false, MISSIONAI_GROUP, NULL, pBestUnit);
			return true;
		}
	}

	return false;
}

// Returns true if we loaded onto a transport or a mission was pushed...
bool CvUnitAI::AI_load(UnitAITypes eUnitAI, MissionAITypes eMissionAI, UnitAITypes eTransportedUnitAI, int iMinCargo, int iMinCargoSpace, int iMaxCargoSpace, int iMaxCargoOurUnitAI, int iFlags, int iMaxPath)
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	// XXX what to do about groups???
	/*if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}*/

	if (getCargo() > 0)
	{
		return false;
	}

	if (isCargo())
	{
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}

	if ((getDomainType() == DOMAIN_LAND) && !canMoveAllTerrain())
	{
		if (area()->getNumAIUnits(getOwnerINLINE(), eUnitAI) == 0)
		{
			return false;
		}
	}

	// do not load transports if we are already in a land war
	AreaAITypes eAreaAIType = area()->getAreaAIType(getTeam());
	bool bLandWar = ((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING));
	if (!isBarbarian() && bLandWar && (eMissionAI != MISSIONAI_LOAD_SETTLER))
	{
		return false;
	}

	iBestValue = MAX_INT;
	pBestUnit = NULL;

	const int iLoadMissionAICount = 4;
	MissionAITypes aeLoadMissionAI[iLoadMissionAICount] = {MISSIONAI_LOAD_ASSAULT, MISSIONAI_LOAD_SETTLER, MISSIONAI_LOAD_SPECIAL, MISSIONAI_ATTACK_SPY};

	int iCurrentGroupSize = getGroup()->getNumUnits();

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (pLoopUnit != this)
		{
			if (AI_plotValid(pLoopUnit->plot()))
			{
				if (canLoadUnit(pLoopUnit, pLoopUnit->plot()))
				{
					// special case ASSAULT_SEA UnitAI, so that, if a unit is marked escort, but can load units, it will load them
					// transport units might have been built as escort, this most commonly happens with galleons
					UnitAITypes eLoopUnitAI = pLoopUnit->AI_getUnitAIType();
					if (eLoopUnitAI == eUnitAI)// || (eUnitAI == UNITAI_ASSAULT_SEA && eLoopUnitAI == UNITAI_ESCORT_SEA))
					{
						int iCargoSpaceAvailable = pLoopUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType());
						iCargoSpaceAvailable -= GET_PLAYER(getOwnerINLINE()).AI_unitTargetMissionAIs(pLoopUnit, aeLoadMissionAI, iLoadMissionAICount, getGroup());
						if (iCargoSpaceAvailable > 0)
						{
							if ((eTransportedUnitAI == NO_UNITAI) || (pLoopUnit->getUnitAICargo(eTransportedUnitAI) > 0))
							{
								if ((iMinCargo == -1) || (pLoopUnit->getCargo() >= iMinCargo))
								{
									if ((iMinCargoSpace == -1) || (pLoopUnit->cargoSpaceAvailable() >= iMinCargoSpace))
									{
										if ((iMaxCargoSpace == -1) || (pLoopUnit->cargoSpaceAvailable() <= iMaxCargoSpace))
										{
											if ((iMaxCargoOurUnitAI == -1) || (pLoopUnit->getUnitAICargo(AI_getUnitAIType()) <= iMaxCargoOurUnitAI))
											{
												if (getGroup()->getHeadUnitAI() != UNITAI_CITY_DEFENSE || !plot()->isCity() || (plot()->getTeam() != getTeam()))
												{
													if (!(pLoopUnit->plot()->isVisibleEnemyUnit(this)))
													{
														CvPlot* pUnitTargetPlot = pLoopUnit->getGroup()->AI_getMissionAIPlot();
														if ((pUnitTargetPlot == NULL) || (pUnitTargetPlot->getTeam() == getTeam()) || (!pUnitTargetPlot->isOwned() || !isPotentialEnemy(pUnitTargetPlot->getTeam(), pUnitTargetPlot)))
														{
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We don't have to check for a path to distant shores if we want to move only short distance  **/
/**	anyway, so approx maximum distance for a possible path by iMaxPath                			**/
/*************************************************************************************************/
/** Orig Code Start
                                                        if (generatePath(pLoopUnit->plot(), iFlags, true, &iPathTurns))
                                                        {
                                                            if (iPathTurns <= iMaxPath)
                                                            {
                                                                // prefer a transport that can hold as much of our group as possible
                                                                iValue = (std::max(0, iCurrentGroupSize - iCargoSpaceAvailable) * 5) + iPathTurns;

                                                                if (iValue < iBestValue)
                                                                {
                                                                    iBestValue = iValue;
                                                                    pBestUnit = pLoopUnit;
                                                                }
                                                            }
                                                        }
/** Orig Code End **/

                                                        int XDist=pLoopUnit->plot()->getX_INLINE() - plot()->getX_INLINE();
                                                        int YDist=pLoopUnit->plot()->getY_INLINE() - plot()->getY_INLINE();
                                                        if (((XDist*XDist)+(YDist*YDist))<iMaxPath*iMaxPath*4)
                                                        {
															if (generatePath(pLoopUnit->plot(), iFlags, true, &iPathTurns))
															{
																if (iPathTurns <= iMaxPath)
																{
																	// prefer a transport that can hold as much of our group as possible
																	iValue = (std::max(0, iCurrentGroupSize - iCargoSpaceAvailable) * 5) + iPathTurns;

																	if (iValue < iBestValue)
																	{
																		iBestValue = iValue;
																		pBestUnit = pLoopUnit;
																	}
																}
															}
                                                        }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

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
				}
			}
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			getGroup()->setTransportUnit(pBestUnit); // XXX is this dangerous (not pushing a mission...) XXX air units?
			return true;
		}
		else
		{
			int iCargoSpaceAvailable = pBestUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType());
			FAssertMsg(iCargoSpaceAvailable > 0, "best unit has no space");

			// split our group to fit on the transport
			CvSelectionGroup* pOtherGroup = NULL;
			CvSelectionGroup* pSplitGroup = getGroup()->splitGroup(iCargoSpaceAvailable, this, &pOtherGroup);
			FAssertMsg(pSplitGroup != NULL, "splitGroup failed");
			FAssertMsg(m_iGroupID == pSplitGroup->getID(), "splitGroup failed to put unit in the new group");

			if (pSplitGroup != NULL)
			{
				CvPlot* pOldPlot = pSplitGroup->plot();
				pSplitGroup->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), iFlags, false, false, eMissionAI, NULL, pBestUnit);
				bool bMoved = (pSplitGroup->plot() != pOldPlot);
				if (!bMoved && pOtherGroup != NULL)
				{
					joinGroup(pOtherGroup);
				}
				return bMoved;
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardCityBestDefender()
{
	CvCity* pCity;
	CvPlot* pPlot;

	pPlot = plot();
	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pPlot->getBestDefender(getOwnerINLINE()) == this)
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return true;
			}
		}
	}

	return false;
}

bool CvUnitAI::AI_guardCityMinDefender(bool bSearch)
{
	PROFILE_FUNC();

	CvCity* pPlotCity = plot()->getPlotCity();
	if ((pPlotCity != NULL) && (pPlotCity->getOwnerINLINE() == getOwnerINLINE()))
	{
		int iCityDefenderCount = pPlotCity->plot()->plotCount(PUF_isUnitAIType, UNITAI_CITY_DEFENSE, -1, getOwnerINLINE());
		if ((iCityDefenderCount - 1) < pPlotCity->AI_minDefenders())
		{
			if ((iCityDefenderCount <= 2) || (GC.getGame().getSorenRandNum(5, "AI shuffle defender") != 0))
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return true;
			}
		}
	}

	if (bSearch)
	{
		int iBestValue = 0;
		CvPlot* pBestPlot = NULL;
		CvPlot* pBestGuardPlot = NULL;

		CvCity* pLoopCity;
		int iLoop;

		int iCurrentTurn = GC.getGame().getGameTurn();
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (AI_plotValid(pLoopCity->plot()))
			{
				int iDefendersHave = pLoopCity->plot()->plotCount(PUF_isUnitAIType, UNITAI_CITY_DEFENSE, -1, getOwnerINLINE());
				int iDefendersNeed = pLoopCity->AI_minDefenders();
				if (iDefendersHave < iDefendersNeed)
				{
					if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
					{
						iDefendersHave += GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_GUARD_CITY, getGroup());
						if (iDefendersHave < iDefendersNeed + 1)
						{
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We don't have to check for a path to distant shores if we want to move only short distance  **/
/**	anyway, so approx maximum distance for a possible path by iMaxPath                			**/
/*************************************************************************************************/
/** Orig Code Start
							int iPathTurns;
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
							{
								if (iPathTurns <= 10)
								{
									int iValue = (iDefendersNeed - iDefendersHave) * 20;
									iValue += 2 * std::min(15, iCurrentTurn - pLoopCity->getGameTurnAcquired());
									if (pLoopCity->isOccupation())
									{
										iValue += 5;
									}
									iValue -= iPathTurns;

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestGuardPlot = pLoopCity->plot();
									}
								}
							}
/** Orig Code End **/

                            int XDist=pLoopCity->plot()->getX_INLINE() - plot()->getX_INLINE();
                            int YDist=pLoopCity->plot()->getY_INLINE() - plot()->getY_INLINE();
                            if (((XDist*XDist)+(YDist*YDist))<200)
                            {
                                int iPathTurns;
                                if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
                                {
                                    if (iPathTurns <= 10)
                                    {
                                        int iValue = (iDefendersNeed - iDefendersHave) * 20;
                                        iValue += 2 * std::min(15, iCurrentTurn - pLoopCity->getGameTurnAcquired());
                                        if (pLoopCity->isOccupation())
                                        {
                                            iValue += 5;
                                        }
                                        iValue -= iPathTurns;

                                        if (iValue > iBestValue)
                                        {
                                            iBestValue = iValue;
                                            pBestPlot = getPathEndTurnPlot();
                                            pBestGuardPlot = pLoopCity->plot();
                                        }
                                    }
                                }
							}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

						}
					}
				}
			}
		}
		if (pBestPlot != NULL)
		{
			if (atPlot(pBestGuardPlot))
			{
				FAssert(pBestGuardPlot == pBestPlot);
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				return true;
			}
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, pBestGuardPlot);
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardCity(bool bLeave, bool bSearch, int iMaxPath)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvCity* pLoopCity;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestGuardPlot;
	bool bDefend;
	int iExtra;
	int iCount;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	FAssert(getDomainType() == DOMAIN_LAND);
	FAssert(canDefend());

	pPlot = plot();
	pCity = pPlot->getPlotCity();

	if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE())) // XXX check for other team?
	{
		if (bLeave && !(pCity->AI_isDanger()))
		{
			iExtra = 1;
		}
		else
		{
			iExtra = (bSearch ? 0 : -GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pPlot, 2));
		}

		bDefend = false;

		if (pPlot->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE()) == 1) // XXX check for other team's units?
		{
			bDefend = true;
		}
		else if (!(pCity->AI_isDefended(((AI_isCityAIType()) ? -1 : 0) + iExtra))) // XXX check for other team's units?
		{
			if (AI_isCityAIType())
			{
				bDefend = true;
			}
			else
			{
				iCount = 0;

				pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
					{
						if (pLoopUnit->isGroupHead())
						{
							if (!(pLoopUnit->isCargo()))
							{
								if (pLoopUnit->canDefend())
								{
									if (!(pLoopUnit->AI_isCityAIType()))
									{
										if (!(pLoopUnit->isHurt()))
										{
											if (pLoopUnit->isWaiting())
											{
												FAssert(pLoopUnit != this);
												iCount++;
											}
										}
									}
									else
									{
										if (pLoopUnit->getGroup()->getMissionType(0) != MISSION_SKIP)
										{
											iCount++;
										}
									}
								}
							}
						}
					}
				}

				if (!(pCity->AI_isDefended(iCount + iExtra))) // XXX check for other team's units?
				{
					bDefend = true;
				}
			}
		}

		if (bDefend)
		{
			CvSelectionGroup* pOldGroup = getGroup();
			CvUnit* pEjectedUnit = getGroup()->AI_ejectBestDefender(pPlot);

			if (pEjectedUnit != NULL)
			{
				if (pPlot->plotCount(PUF_isCityAIType, -1, -1, getOwnerINLINE()) == 0)
				{
					if (pEjectedUnit->cityDefenseModifier() > 0)
					{
						pEjectedUnit->AI_setUnitAIType(UNITAI_CITY_DEFENSE);
					}
				}
				pEjectedUnit->getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				if (pEjectedUnit->getGroup() == pOldGroup || pEjectedUnit == this)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				//This unit is not suited for defense, skip the mission
				//to protect this city but encourage others to defend instead.
				getGroup()->pushMission(MISSION_SKIP);
				if (!isHurt())
				{
					finishMoves();
				}
			}
			return true;
		}
	}

	if (bSearch)
	{
		iBestValue = MAX_INT;
		pBestPlot = NULL;
		pBestGuardPlot = NULL;

		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (AI_plotValid(pLoopCity->plot()))
			{
				if (!(pLoopCity->AI_isDefended((!AI_isCityAIType()) ? pLoopCity->plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isNotCityAIType) : 0)))	// XXX check for other team's units?
				{
					if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
					{
						if ((GC.getGame().getGameTurn() - pLoopCity->getGameTurnAcquired() < 10) || GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_GUARD_CITY, getGroup()) < 2)
						{
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We don't have to check for a path to distant shores if we want to move only short distance  **/
/**	anyway, so approx maximum distance for a possible path by iMaxPath                			**/
/*************************************************************************************************/
/** Orig Code Start
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
							{
								if (iPathTurns <= iMaxPath)
								{
									iValue = iPathTurns;

									if (iValue < iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestGuardPlot = pLoopCity->plot();
										FAssert(!atPlot(pBestPlot));
									}
								}
							}
/** Orig Code End **/

                            int XDist=pLoopCity->plot()->getX_INLINE() - plot()->getX_INLINE();
                            int YDist=pLoopCity->plot()->getY_INLINE() - plot()->getY_INLINE();
                            if (((XDist*XDist)+(YDist*YDist))<iMaxPath*iMaxPath*4)
                            {
                                if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
                                {
                                    if (iPathTurns <= iMaxPath)
                                    {
                                        iValue = iPathTurns;

                                        if (iValue < iBestValue)
                                        {
                                            iBestValue = iValue;
                                            pBestPlot = getPathEndTurnPlot();
                                            pBestGuardPlot = pLoopCity->plot();
                                            FAssert(!atPlot(pBestPlot));
                                        }
                                    }
                                }
							}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
						}
					}
				}

				if (pBestPlot != NULL)
				{
					break;
				}
			}
		}

		if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
		{
			FAssert(!atPlot(pBestPlot));
			// split up group if we are going to defend, so rest of group has opportunity to do something else
//			if (getGroup()->getNumUnits() > 1)
//			{
//				getGroup()->AI_separate();	// will change group
//			}
//
//			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, pBestGuardPlot);
//			return true;

			CvSelectionGroup* pOldGroup = getGroup();
			CvUnit* pEjectedUnit = getGroup()->AI_ejectBestDefender(pPlot);

			if (pEjectedUnit != NULL)
			{
				pEjectedUnit->getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				if (pEjectedUnit->getGroup() == pOldGroup || pEjectedUnit == this)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				//This unit is not suited for defense, skip the mission
				//to protect this city but encourage others to defend instead.
				if (atPlot(pBestGuardPlot))
				{
					getGroup()->pushMission(MISSION_SKIP);
				}
				else
				{
					getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, NULL);
				}
				return true;
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardCityAirlift()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity != pCity)
		{
			if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
			{
				if (!(pLoopCity->AI_isDefended((!AI_isCityAIType()) ? pLoopCity->plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isNotCityAIType) : 0)))	// XXX check for other team's units?
				{
					iValue = pLoopCity->getPopulation();

					if (pLoopCity->AI_isDanger())
					{
						iValue *= 2;
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopCity->plot();
						FAssert(pLoopCity != pCity);
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardBonus(int iMinValue)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestGuardPlot;
	ImprovementTypes eImprovement;
	BonusTypes eNonObsoleteBonus;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestGuardPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
			{
				eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

//FfH: Modified by Kael 03/22/2008
//				if (eNonObsoleteBonus != NO_BONUS)
//				{
//					eImprovement = pLoopPlot->getImprovementType();
//
//					if ((eImprovement != NO_IMPROVEMENT) && GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
//					{
//						iValue = GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus);
//						iValue += std::max(0, 200 * GC.getBonusInfo(eNonObsoleteBonus).getAIObjective());
//
//						if (pLoopPlot->getPlotGroupConnectedBonus(getOwnerINLINE(), eNonObsoleteBonus) == 1)
//						{
//							iValue *= 2;
//						}
                iValue = 0;
				eImprovement = pLoopPlot->getImprovementType();
                if (eImprovement != NO_IMPROVEMENT)
                {
                    if (eNonObsoleteBonus != NO_BONUS && GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
                    {
						iValue += GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus);
						iValue += std::max(0, 200 * GC.getBonusInfo(eNonObsoleteBonus).getAIObjective());
						if (pLoopPlot->getPlotGroupConnectedBonus(getOwnerINLINE(), eNonObsoleteBonus) == 1)
						{
							iValue *= 2;
						}
                    }
                    iValue += GC.getImprovementInfo(eImprovement).getRangeDefenseModifier() * GC.getImprovementInfo(eImprovement).getRange() * 200;
//FfH: End Modify

						if (iValue > iMinValue)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_GUARD_BONUS, getGroup()) == 0)
								{
									if (generatePath(pLoopPlot, 0, true, &iPathTurns))
									{
										iValue *= 1000;

										iValue /= (iPathTurns + 1);

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											pBestGuardPlot = pLoopPlot;
										}
									}
								}
							}
						}
					}
				}
			}
		}

//FfH: Modified by Kael 10/29/2007
//	}
//FfH: End Modify

	if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
	{
		if (atPlot(pBestGuardPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_BONUS, pBestGuardPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_BONUS, pBestGuardPlot);
			return true;
		}
	}

	return false;
}

int CvUnitAI::AI_getPlotDefendersNeeded(CvPlot* pPlot, int iExtra)
{
	int iNeeded = iExtra;
	BonusTypes eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(getTeam());
	if (eNonObsoleteBonus != NO_BONUS)
	{
		iNeeded += (GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus) + 10) / 19;
	}

	int iDefense = pPlot->defenseModifier(getTeam(), true);

	iNeeded += (iDefense + 25) / 50;

	if (iNeeded == 0)
	{
		return 0;
	}

	iNeeded += GET_PLAYER(getOwnerINLINE()).AI_getPlotAirbaseValue(pPlot) / 50;

	int iNumHostiles = 0;
	int iNumPlots = 0;

	int iRange = 2;
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				iNumHostiles += pLoopPlot->getNumVisibleEnemyDefenders(this);
				if ((pLoopPlot->getTeam() != getTeam()) || pLoopPlot->isCoastalLand())
				{
				    iNumPlots++;
                    if (isEnemy(pLoopPlot->getTeam()))
                    {
                        iNumPlots += 4;
                    }
				}
			}
		}
	}

	if ((iNumHostiles == 0) && (iNumPlots < 4))
	{
		if (iNeeded > 1)
		{
			iNeeded = 1;
		}
		else
		{
			iNeeded = 0;
		}
	}

	return iNeeded;
}

bool CvUnitAI::AI_guardFort(bool bSearch)
{
	PROFILE_FUNC();

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		ImprovementTypes eImprovement = plot()->getImprovementType();
		if (eImprovement != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(eImprovement).isActsAsCity())
			{
				if (plot()->plotCount(PUF_isCityAIType, -1, -1, getOwnerINLINE()) <= AI_getPlotDefendersNeeded(plot(), 0))
				{
					getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_BONUS, plot());
					return true;
				}
			}
		}
	}

	if (!bSearch)
	{
		return false;
	}

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestGuardPlot = NULL;

	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot) && !atPlot(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
			{
				ImprovementTypes eImprovement = pLoopPlot->getImprovementType();
				if (eImprovement != NO_IMPROVEMENT)
				{
					if (GC.getImprovementInfo(eImprovement).isActsAsCity())
					{
						int iValue = AI_getPlotDefendersNeeded(pLoopPlot, 0);

						if (iValue > 0)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_GUARD_BONUS, getGroup()) < iValue)
								{
									int iPathTurns;
									if (generatePath(pLoopPlot, 0, true, &iPathTurns))
									{
										iValue *= 1000;

										iValue /= (iPathTurns + 2);

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											pBestGuardPlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
	{
		if (atPlot(pBestGuardPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_BONUS, pBestGuardPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_BONUS, pBestGuardPlot);
			return true;
		}
	}

	return false;
}
// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardCitySite()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestGuardPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestGuardPlot = NULL;

	for (iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
	{
		pLoopPlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);
		if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_GUARD_CITY, getGroup()) == 0)
		{
			if (generatePath(pLoopPlot, 0, true, &iPathTurns))
			{
				iValue = pLoopPlot->getFoundValue(getOwnerINLINE());
				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestPlot = getPathEndTurnPlot();
					pBestGuardPlot = pLoopPlot;
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
	{
		if (atPlot(pBestGuardPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_CITY, pBestGuardPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_CITY, pBestGuardPlot);
			return true;
		}
	}

	return false;
}



// Returns true if a mission was pushed...
bool CvUnitAI::AI_guardSpy(int iRandomPercent)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestGuardPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestGuardPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
			{
				iValue = 0;
				if (pLoopCity->isProductionUnit())
				{
					if (isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(pLoopCity->getProductionUnit()).getUnitClassType())))
					{
						iValue = 4;
					}
				}
				else if (pLoopCity->isProductionBuilding())
				{
					if (isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pLoopCity->getProductionBuilding()).getBuildingClassType())))
					{
						iValue = 5;
					}
				}
				else if (pLoopCity->isProductionProject())
				{
					if (isLimitedProject(pLoopCity->getProductionProject()))
					{
						iValue = 6;
					}
				}
				if (iValue > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_GUARD_SPY, getGroup()) == 0)
					{
						int iPathTurns;
						if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
						{
							iValue *= 100 + GC.getGameINLINE().getSorenRandNum(iRandomPercent, "AI Guard Spy");
							//iValue /= 100;
							iValue /= iPathTurns + 1;

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestGuardPlot = pLoopCity->plot();
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestGuardPlot != NULL))
	{
		if (atPlot(pBestGuardPlot))
		{
			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_GUARD_SPY, pBestGuardPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GUARD_SPY, pBestGuardPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_destroySpy()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) <= ATTITUDE_ANNOYED)
				{
					for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
					{
						if (AI_plotValid(pLoopCity->plot()))
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_ATTACK_SPY, getGroup()) == 0)
							{
								if (generatePath(pLoopCity->plot(), 0, true))
								{
									iValue = (pLoopCity->getPopulation() * 2);

									iValue += pLoopCity->getYieldRate(YIELD_PRODUCTION);

									if (atPlot(pLoopCity->plot()))
									{
										iValue *= 4;
										iValue /= 3;
									}

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestCity = pLoopCity;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestCity != NULL))
	{
		if (atPlot(pBestCity->plot()))
		{
			if (canDestroy(pBestCity->plot()))
			{
				if (pBestCity->getProduction() > ((pBestCity->getProductionNeeded() * 2) / 3))
				{
					if (pBestCity->isProductionUnit())
					{
						if (isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(pBestCity->getProductionUnit()).getUnitClassType())))
						{
							getGroup()->pushMission(MISSION_DESTROY);
							return true;
						}
					}
					else if (pBestCity->isProductionBuilding())
					{
						if (isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pBestCity->getProductionBuilding()).getBuildingClassType())))
						{
							getGroup()->pushMission(MISSION_DESTROY);
							return true;
						}
					}
					else if (pBestCity->isProductionProject())
					{
						if (isLimitedProject(pBestCity->getProductionProject()))
						{
							getGroup()->pushMission(MISSION_DESTROY);
							return true;
						}
					}
				}
			}

			getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_ATTACK_SPY, pBestCity->plot());
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestCity->plot());
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_sabotageSpy()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestSabotagePlot;
	bool abPlayerAngry[MAX_PLAYERS];
	ImprovementTypes eImprovement;
	BonusTypes eNonObsoleteBonus;
	int iValue;
	int iBestValue;
	int iI;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		abPlayerAngry[iI] = false;

		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) <= ATTITUDE_ANNOYED)
				{
					abPlayerAngry[iI] = true;
				}
			}
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestSabotagePlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->isOwned())
			{
				if (pLoopPlot->getTeam() != getTeam())
				{
					if (abPlayerAngry[pLoopPlot->getOwnerINLINE()])
					{
						eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(pLoopPlot->getTeam());

						if (eNonObsoleteBonus != NO_BONUS)
						{
							eImprovement = pLoopPlot->getImprovementType();

							if ((eImprovement != NO_IMPROVEMENT) && GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
							{
								if (canSabotage(pLoopPlot))
								{
									iValue = GET_PLAYER(pLoopPlot->getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus);

									if (pLoopPlot->isConnectedToCapital() && (pLoopPlot->getPlotGroupConnectedBonus(pLoopPlot->getOwnerINLINE(), eNonObsoleteBonus) == 1))
									{
										iValue *= 3;
									}

									if (iValue > 25)
									{
										if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ATTACK_SPY, getGroup()) == 0)
										{
											if (generatePath(pLoopPlot, 0, true))
											{
												if (iValue > iBestValue)
												{
													iBestValue = iValue;
													pBestPlot = getPathEndTurnPlot();
													pBestSabotagePlot = pLoopPlot;
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
		}
	}

	if ((pBestPlot != NULL) && (pBestSabotagePlot != NULL))
	{
		if (atPlot(pBestSabotagePlot))
		{
			getGroup()->pushMission(MISSION_SABOTAGE);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestSabotagePlot);
			return true;
		}
	}

	return false;
}


bool CvUnitAI::AI_pickupTargetSpy()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestPickupPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_ATTACK_SPY, pCity->plot());
				return true;
			}
		}
	}

	iBestValue = MAX_INT;
	pBestPlot = NULL;
	pBestPickupPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
			{
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
				{
					if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
					{
						iValue = iPathTurns;

						if (iValue < iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
							pBestPickupPlot = pLoopCity->plot();
							FAssert(!atPlot(pBestPlot));
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestPickupPlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestPickupPlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_chokeDefend()
{
	CvCity* pCity;
	int iPlotDanger;

	FAssert(AI_isCityAIType());

	// XXX what about amphib invasions?

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->AI_neededDefenders() > 1)
			{
				if (pCity->AI_isDefended(pCity->plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE(), NO_TEAM, PUF_isNotCityAIType)))
				{
					iPlotDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3);

					if (iPlotDanger <= 4)
					{
						if (AI_anyAttack(1, 65, std::max(0, (iPlotDanger - 1))))
						{
							return true;
						}
					}
				}
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_heal(int iDamagePercent, int iMaxPath)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pEntityNode;
	std::vector<CvUnit*> aeDamagedUnits;
	CvSelectionGroup* pGroup;
	CvUnit* pLoopUnit;
	int iTotalDamage;
	int iTotalHitpoints;
	int iHurtUnitCount;
	bool bRetreat;

	if (plot()->getFeatureType() != NO_FEATURE)
	{
		if (GC.getFeatureInfo(plot()->getFeatureType()).getTurnDamage() != 0)
		{
			//Pass through
			//(actively seeking a safe spot may result in unit getting stuck)
			return false;
		}
	}

//FfH: Added by Kael 10/01/2007 (so the AI won't try to sit and heal in areas where they cant heal)
    if (healRate(plot()) == 0)
    {
        return false;
    }
//FfH: End Add

	pGroup = getGroup();

	if (iDamagePercent == 0)
	{
	    iDamagePercent = 10;
	}

	bRetreat = false;

    if (getGroup()->getNumUnits() == 1)
	{
	    if (getDamage() > 0)
        {

            if (plot()->isCity() || (healTurns(plot()) == 1))
            {
                if (!(isAlwaysHeal()))
                {
                    getGroup()->pushMission(MISSION_HEAL);
                    return true;
                }
            }
        }
        return false;
	}

	iMaxPath = std::min(iMaxPath, 2);

	pEntityNode = getGroup()->headUnitNode();

    iTotalDamage = 0;
    iTotalHitpoints = 0;
    iHurtUnitCount = 0;
	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		FAssert(pLoopUnit != NULL);
		pEntityNode = pGroup->nextUnitNode(pEntityNode);

		int iDamageThreshold = (pLoopUnit->maxHitPoints() * iDamagePercent) / 100;

		if (NO_UNIT != getLeaderUnitType())
		{
			iDamageThreshold /= 2;
		}

		if (pLoopUnit->getDamage() > 0)
		{
		    iHurtUnitCount++;
		}
		iTotalDamage += pLoopUnit->getDamage();
		iTotalHitpoints += pLoopUnit->maxHitPoints();


		if (pLoopUnit->getDamage() > iDamageThreshold)
		{
			bRetreat = true;

			if (!(pLoopUnit->hasMoved()))
			{
				if (!(pLoopUnit->isAlwaysHeal()))
				{
					if (pLoopUnit->healTurns(pLoopUnit->plot()) <= iMaxPath)
					{
					    aeDamagedUnits.push_back(pLoopUnit);
					}
				}
			}
		}
	}
	if (iHurtUnitCount == 0)
	{
	    return false;
	}

	bool bPushedMission = false;
    if (plot()->isCity() && (plot()->getOwnerINLINE() == getOwnerINLINE()))
	{
		FAssertMsg(((int) aeDamagedUnits.size()) <= iHurtUnitCount, "damaged units array is larger than our hurt unit count");

		for (unsigned int iI = 0; iI < aeDamagedUnits.size(); iI++)
		{
			CvUnit* pUnitToHeal = aeDamagedUnits[iI];
			pUnitToHeal->joinGroup(NULL);
			pUnitToHeal->getGroup()->pushMission(MISSION_HEAL);

			// note, removing the head unit from a group will force the group to be completely split if non-human
			if (pUnitToHeal == this)
			{
				bPushedMission = true;
			}

			iHurtUnitCount--;
		}
	}

	if ((iHurtUnitCount * 2) > pGroup->getNumUnits())
	{
		FAssertMsg(pGroup->getNumUnits() > 0, "group now has zero units");

	    if (AI_moveIntoCity(2))
		{
			return true;
		}
		else if (healRate(plot()) > 10)
	    {
            pGroup->pushMission(MISSION_HEAL);
            return true;
	    }
	}

	return bPushedMission;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_afterAttack()
{
	if (!isMadeAttack())
	{
		return false;
	}

	if (!canFight())
	{
		return false;
	}

	if (isBlitz())
	{
		return false;
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		if (AI_guardCity(false, true, 1))
		{
			return true;
		}
	}

	if (AI_pillageRange(1))
	{
		return true;
	}

	if (AI_retreatToCity(false, false, 1))
	{
		return true;
	}

	if (AI_hide())
	{
		return true;
	}

	if (AI_goody(1))
	{
		return true;
	}

	if (AI_pillageRange(2))
	{
		return true;
	}

	if (AI_defend())
	{
		return true;
	}

	if (AI_safety())
	{
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_goldenAge()
{
	if (canGoldenAge(plot()))
	{
		getGroup()->pushMission(MISSION_GOLDEN_AGE);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_spreadReligion()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestSpreadPlot;
	ReligionTypes eReligion;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iPlayerMultiplierPercent;
	int iLoop;
	int iI;


    bool bCultureVictory = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE2);
	eReligion = NO_RELIGION;

	if (eReligion == NO_RELIGION)
	{
		if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != NO_RELIGION)
		{
			if (m_pUnitInfo->getReligionSpreads(GET_PLAYER(getOwnerINLINE()).getStateReligion()) > 0)
			{
				eReligion = GET_PLAYER(getOwnerINLINE()).getStateReligion();
			}
		}
	}

	if (eReligion == NO_RELIGION)
	{
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			//if (bCultureVictory || GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
			{
				if (m_pUnitInfo->getReligionSpreads((ReligionTypes)iI) > 0)
				{
					eReligion = ((ReligionTypes)iI);
					break;
				}
			}
		}
	}

	if (eReligion == NO_RELIGION)
	{
		return false;
	}

	bool bHasHolyCity = GET_TEAM(getTeam()).hasHolyCity(eReligion);
	bool bHasAnyHolyCity = bHasHolyCity;
	if (!bHasAnyHolyCity)
	{
		for (iI = 0; !bHasAnyHolyCity && iI < GC.getNumReligionInfos(); iI++)
		{
			bHasAnyHolyCity = GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI);
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestSpreadPlot = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
		    iPlayerMultiplierPercent = 0;

			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (bHasHolyCity)
				{
					iPlayerMultiplierPercent = 100;
					if (!bCultureVictory || ((eReligion == GET_PLAYER(getOwnerINLINE()).getStateReligion()) && bHasHolyCity))
					{
						if (GET_PLAYER((PlayerTypes)iI).getStateReligion() == NO_RELIGION)
						{
							if (0 == (GET_PLAYER((PlayerTypes)iI).getNonStateReligionHappiness()))
							{
								iPlayerMultiplierPercent += 600;
							}
						}
						else if (GET_PLAYER((PlayerTypes)iI).getStateReligion() == eReligion)
						{
							iPlayerMultiplierPercent += 300;
						}
						else
						{
							if (GET_PLAYER((PlayerTypes)iI).hasHolyCity(GET_PLAYER((PlayerTypes)iI).getStateReligion()))
							{
								iPlayerMultiplierPercent += 50;
							}
							else
							{
								iPlayerMultiplierPercent += 300;
							}
						}

						int iReligionCount = GET_PLAYER((PlayerTypes)iI).countTotalHasReligion();
						int iCityCount = GET_PLAYER(getOwnerINLINE()).getNumCities();
						//magic formula to produce normalized adjustment factor based on religious infusion
						int iAdjustment = (100 * (iCityCount + 1));
						iAdjustment /= ((iCityCount + 1) + iReligionCount);
						iAdjustment = (((iAdjustment - 25) * 4) / 3);

						iAdjustment = (std::max(10, iAdjustment));

						iPlayerMultiplierPercent *= iAdjustment;
						iPlayerMultiplierPercent /= 100;

						// if we have a holy city, but not this holy city, then we will likely switch
						// religions soon, so try to spread this religion internally, not externally
						if (bHasAnyHolyCity && !bHasHolyCity)
						{
							iPlayerMultiplierPercent /= 10;
						}
					}
				}
			}
			else if (iI == getOwnerINLINE())
			{
				iPlayerMultiplierPercent = 100;
			}
			else if (bHasHolyCity)
			{
				iPlayerMultiplierPercent = 80;
			}

			if (iPlayerMultiplierPercent > 0)
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{

					if (AI_plotValid(pLoopCity->plot()) && pLoopCity->area() == area())
					{
						if (canSpread(pLoopCity->plot(), eReligion))
						{
							if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_SPREAD, getGroup()) == 0)
								{
									if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
									{
										iValue = (7 + (pLoopCity->getPopulation() * 4));

										bool bOurCity = false;
										if (pLoopCity->getOwnerINLINE() == getOwnerINLINE())
										{
											iValue *= (bCultureVictory ? 16 : 4);
											bOurCity = true;
										}
										else if (pLoopCity->getTeam() == getTeam())
										{
											iValue *= 3;
											bOurCity = true;
										}
										else
										{
											iValue *= iPlayerMultiplierPercent;
											iValue /= 100;
										}

										int iCityReligionCount = pLoopCity->getReligionCount();
										int iReligionCountFactor = iCityReligionCount;

										if (bOurCity)
										{
											// count cities with no religion the same as cities with 2 religions
											// prefer a city with exactly 1 religion already
											if (iCityReligionCount == 0)
											{
												iReligionCountFactor = 2;
											}
											else if (iCityReligionCount == 1)
											{
												iValue *= 2;
											}
										}
										else
										{
											// absolutely prefer cities with zero religions
											if (iCityReligionCount == 0)
											{
												iValue *= 2;
											}

											// not our city, so prefer the lowest number of religions (increment so no divide by zero)
											iReligionCountFactor++;
										}

										iValue /= iReligionCountFactor;

										FAssert(iPathTurns > 0);

										bool bForceMove = false;
										if (isHuman())
										{
											//If human, prefer to spread to the player where automated from.
											if (plot()->getOwnerINLINE() == pLoopCity->getOwnerINLINE())
											{
												iValue *= 10;
												if (pLoopCity->isRevealed(getTeam(), false))
												{
													bForceMove = true;
												}
											}
										}

										iValue *= 1000;

										iValue /= (iPathTurns + 2);

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = bForceMove ? pLoopCity->plot() : getPathEndTurnPlot();
											pBestSpreadPlot = pLoopCity->plot();
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

	if ((pBestPlot != NULL) && (pBestSpreadPlot != NULL))
	{
		if (atPlot(pBestSpreadPlot))
		{
			getGroup()->pushMission(MISSION_SPREAD, eReligion);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_SPREAD, pBestSpreadPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_spreadCorporation()
{
	PROFILE_FUNC();

	CorporationTypes eCorporation = NO_CORPORATION;

	for (int iI = 0; iI < GC.getNumCorporationInfos(); ++iI)
	{
		if (m_pUnitInfo->getCorporationSpreads((CorporationTypes)iI) > 0)
		{
			eCorporation = ((CorporationTypes)iI);
			break;
		}
	}

	if (NO_CORPORATION == eCorporation)
	{
		return false;
	}
	bool bHasHQ = (GET_TEAM(getTeam()).hasHeadquarters((CorporationTypes)iI));

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestSpreadPlot = NULL;

	CvTeam& kTeam = GET_TEAM(getTeam());
	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive() && (bHasHQ || (getTeam() == kLoopPlayer.getTeam())))
		{
			int iLoopPlayerCorpCount = kLoopPlayer.countCorporations(eCorporation);
			CvTeam& kLoopTeam = GET_TEAM(kLoopPlayer.getTeam());
			int iLoop;
			for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); NULL != pLoopCity; pLoopCity = kLoopPlayer.nextCity(&iLoop))
			{
				if (AI_plotValid(pLoopCity->plot()))
				{
					if (canSpreadCorporation(pLoopCity->plot(), eCorporation))
					{
						if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_SPREAD_CORPORATION, getGroup()) == 0)
							{
								int iPathTurns;
								if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
								{
									int iValue = (10 + pLoopCity->getPopulation() * 2);

									if (pLoopCity->getOwnerINLINE() == getOwnerINLINE())
									{
										iValue *= 4;
									}
									else if (kLoopTeam.isVassal(getTeam()))
									{
										iValue *= 3;
									}
									else if (kTeam.isVassal(kLoopTeam.getID()))
									{
										if (iLoopPlayerCorpCount == 0)
										{
											iValue *= 10;
										}
										else
										{
											iValue *= 3;
											iValue /= 2;
										}
									}
									else if (pLoopCity->getTeam() == getTeam())
									{
										iValue *= 2;
									}

									if (iLoopPlayerCorpCount == 0)
									{
										//Generally prefer to heavily target one player
										iValue /= 2;
									}


									bool bForceMove = false;
									if (isHuman())
									{
										//If human, prefer to spread to the player where automated from.
										if (plot()->getOwnerINLINE() == pLoopCity->getOwnerINLINE())
										{
											iValue *= 10;
											if (pLoopCity->isRevealed(getTeam(), false))
											{
												bForceMove = true;
											}
										}
									}

									FAssert(iPathTurns > 0);

									iValue *= 1000;

									iValue /= (iPathTurns + 1);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = bForceMove ? pLoopCity->plot() : getPathEndTurnPlot();
										pBestSpreadPlot = pLoopCity->plot();
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestSpreadPlot != NULL))
	{
		if (atPlot(pBestSpreadPlot))
		{
			if (canSpreadCorporation(pBestSpreadPlot, eCorporation))
			{
				getGroup()->pushMission(MISSION_SPREAD_CORPORATION, eCorporation);
			}
			else
			{
				getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_SPREAD_CORPORATION, pBestSpreadPlot);
			}
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_SPREAD_CORPORATION, pBestSpreadPlot);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_spreadReligionAirlift()
{
	PROFILE_FUNC();

	CvPlot* pBestPlot;
	ReligionTypes eReligion;
	int iValue;
	int iBestValue;
	int iI;

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	CvCity* pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	//bool bCultureVictory = GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE2);
	eReligion = NO_RELIGION;

	if (eReligion == NO_RELIGION)
	{
		if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != NO_RELIGION)
		{
			if (m_pUnitInfo->getReligionSpreads(GET_PLAYER(getOwnerINLINE()).getStateReligion()) > 0)
			{
				eReligion = GET_PLAYER(getOwnerINLINE()).getStateReligion();
			}
		}
	}

	if (eReligion == NO_RELIGION)
	{
		for (iI = 0; iI < GC.getNumReligionInfos(); iI++)
		{
			//if (bCultureVictory || GET_TEAM(getTeam()).hasHolyCity((ReligionTypes)iI))
			{
				if (m_pUnitInfo->getReligionSpreads((ReligionTypes)iI) > 0)
				{
					eReligion = ((ReligionTypes)iI);
					break;
				}
			}
		}
	}

	if (eReligion == NO_RELIGION)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive() && (getTeam() == kLoopPlayer.getTeam()))
		{
			int iLoop;
			for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); NULL != pLoopCity; pLoopCity = kLoopPlayer.nextCity(&iLoop))
			{
				if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
				{
					if (canSpread(pLoopCity->plot(), eReligion))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_SPREAD, getGroup()) == 0)
						{
							iValue = (7 + (pLoopCity->getPopulation() * 4));

							int iCityReligionCount = pLoopCity->getReligionCount();
							int iReligionCountFactor = iCityReligionCount;

							// count cities with no religion the same as cities with 2 religions
							// prefer a city with exactly 1 religion already
							if (iCityReligionCount == 0)
							{
								iReligionCountFactor = 2;
							}
							else if (iCityReligionCount == 1)
							{
								iValue *= 2;
							}

							iValue /= iReligionCountFactor;
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopCity->plot();
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_SPREAD, pBestPlot);
		return true;
	}

	return false;
}

bool CvUnitAI::AI_spreadCorporationAirlift()
{
	PROFILE_FUNC();

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	CvCity* pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	CorporationTypes eCorporation = NO_CORPORATION;

	for (int iI = 0; iI < GC.getNumCorporationInfos(); ++iI)
	{
		if (m_pUnitInfo->getCorporationSpreads((CorporationTypes)iI) > 0)
		{
			eCorporation = ((CorporationTypes)iI);
			break;
		}
	}

	if (NO_CORPORATION == eCorporation)
	{
		return false;
	}

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
		if (kLoopPlayer.isAlive() && (getTeam() == kLoopPlayer.getTeam()))
		{
			int iLoop;
			for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); NULL != pLoopCity; pLoopCity = kLoopPlayer.nextCity(&iLoop))
			{
				if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
				{
					if (canSpreadCorporation(pLoopCity->plot(), eCorporation))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_SPREAD_CORPORATION, getGroup()) == 0)
						{
							int iValue = (pLoopCity->getPopulation() * 4);

							if (pLoopCity->getOwnerINLINE() == getOwnerINLINE())
							{
								iValue *= 4;
							}
							else
							{
								iValue *= 3;
							}

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopCity->plot();
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_SPREAD, pBestPlot);
		return true;
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_discover(bool bThisTurnOnly, bool bFirstResearchOnly)
{
	TechTypes eDiscoverTech;
	bool bIsFirstTech;
	int iPercentWasted = 0;

	if (canDiscover(plot()))
	{
		eDiscoverTech = getDiscoveryTech();
		bIsFirstTech = (GET_PLAYER(getOwnerINLINE()).AI_isFirstTech(eDiscoverTech));

        if (bFirstResearchOnly && !bIsFirstTech)
        {
            return false;
        }

		iPercentWasted = (100 - ((getDiscoverResearch(eDiscoverTech) * 100) / getDiscoverResearch(NO_TECH)));
		FAssert(((iPercentWasted >= 0) && (iPercentWasted <= 100)));


        if (getDiscoverResearch(eDiscoverTech) >= GET_TEAM(getTeam()).getResearchLeft(eDiscoverTech))
        {
            if ((iPercentWasted < 51) && bFirstResearchOnly && bIsFirstTech)
            {
                getGroup()->pushMission(MISSION_DISCOVER);
                return true;
            }

            if (iPercentWasted < (bIsFirstTech ? 31 : 11))
            {
                //I need a good way to assess if the tech is actually valuable...
                //but don't have one.
                getGroup()->pushMission(MISSION_DISCOVER);
                return true;
            }
        }
        else if (bThisTurnOnly)
        {
            return false;
        }

        if (iPercentWasted <= 11)
        {
            if (GET_PLAYER(getOwnerINLINE()).getCurrentResearch() == eDiscoverTech)
            {
                getGroup()->pushMission(MISSION_DISCOVER);
                return true;
            }
        }
    }
	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_lead(std::vector<UnitAITypes>& aeUnitAITypes)
{
	PROFILE_FUNC();

	FAssertMsg(!isHuman(), "isHuman did not return false as expected");
	FAssertMsg(AI_getUnitAIType() != NO_UNITAI, "AI_getUnitAIType() is not expected to be equal with NO_UNITAI");
	FAssert(NO_PLAYER != getOwnerINLINE());

	CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());

	bool bNeedLeader = false;
	for (int iI = 0; iI < MAX_CIV_TEAMS; iI++)
	{
		CvTeamAI& kLoopTeam = GET_TEAM((TeamTypes)iI);
		if (isEnemy((TeamTypes)iI))
		{
			if (kLoopTeam.countNumUnitsByArea(area()) > 0)
			{
				bNeedLeader = true;
				break;
			}
		}
	}

	CvUnit* pBestUnit = NULL;
	CvPlot* pBestPlot = NULL;

	// AI may use Warlords to create super-medic units
	CvUnit* pBestStrUnit = NULL;
	CvPlot* pBestStrPlot = NULL;

	CvUnit* pBestHealUnit = NULL;
	CvPlot* pBestHealPlot = NULL;

	if (bNeedLeader)
	{
		int iBestStrength = 0;
		int iBestHealing = 0;
		int iLoop;
		for (CvUnit* pLoopUnit = kOwner.firstUnit(&iLoop); pLoopUnit; pLoopUnit = kOwner.nextUnit(&iLoop))
		{
			for (uint iI = 0; iI < aeUnitAITypes.size(); iI++)
			{
				if (pLoopUnit->AI_getUnitAIType() == aeUnitAITypes[iI] || NO_UNITAI == aeUnitAITypes[iI])
				{
					if (canLead(pLoopUnit->plot(), pLoopUnit->getID()))
					{
						if (AI_plotValid(pLoopUnit->plot()))
						{
							if (!(pLoopUnit->plot()->isVisibleEnemyUnit(this)))
							{
								if (generatePath(pLoopUnit->plot(), 0, true))
								{
									// pick the unit with the highest current strength
									int iCombatStrength = pLoopUnit->currCombatStr(NULL, NULL);
									if (iCombatStrength > iBestStrength)
									{
										iBestStrength = iCombatStrength;
										pBestStrUnit = pLoopUnit;
										pBestStrPlot = getPathEndTurnPlot();
									}

									// or the unit with the best healing ability
									int iHealing = pLoopUnit->getSameTileHeal() + pLoopUnit->getAdjacentTileHeal();
									if (iHealing > iBestHealing)
									{
										iBestHealing = iHealing;
										pBestHealUnit = pLoopUnit;
										pBestHealPlot = getPathEndTurnPlot();
									}

									if (GC.getGame().getSorenRandNum(3, "AI Warlord mash unit") != 0)
									{
										pBestPlot = pBestStrPlot;
										pBestUnit = pBestStrUnit;
									}
									else
									{
										pBestPlot = pBestHealPlot;
										pBestUnit = pBestHealUnit;
									}
								}
							}
						}
					}
					break;
				}
			}
		}
	}

	if (pBestPlot)
	{
		if (atPlot(pBestPlot) && pBestUnit)
		{
			getGroup()->pushMission(MISSION_LEAD, pBestUnit->getID());
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
// iMaxCounts = 1 would mean join a city if there's no existing joined GP of that type.
bool CvUnitAI::AI_join(int iMaxCount)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	SpecialistTypes eBestSpecialist;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;
	int iCount;

	iBestValue = 0;
	pBestPlot = NULL;
	eBestSpecialist = NO_SPECIALIST;
	iCount = 0;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
			{
				if (generatePath(pLoopCity->plot(), 0, true))
				{
					for (iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
					{
						bool bDoesJoin = false;
						SpecialistTypes eSpecialist = (SpecialistTypes)iI;
						if (m_pUnitInfo->getGreatPeoples(eSpecialist))
						{
							bDoesJoin = true;
						}
						if (bDoesJoin)
						{
							iCount += pLoopCity->getSpecialistCount(eSpecialist);
							if (iCount >= iMaxCount)
							{
								return false;
							}
						}

						if (canJoin(pLoopCity->plot(), ((SpecialistTypes)iI)))
						{
							if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopCity->plot(), 2) == 0)
							{
								iValue = pLoopCity->AI_specialistValue(((SpecialistTypes)iI), pLoopCity->AI_avoidGrowth(), false);
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									eBestSpecialist = ((SpecialistTypes)iI);
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (eBestSpecialist != NO_SPECIALIST))
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_JOIN, eBestSpecialist);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
// iMaxCount = 1 would mean construct only if there are no existing buildings
//   constructed by this GP type.
bool CvUnitAI::AI_construct(int iMaxCount, int iMaxSingleBuildingCount, int iThreshold)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestConstructPlot;
	BuildingTypes eBestBuilding;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;
	int iCount;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestConstructPlot = NULL;
	eBestBuilding = NO_BUILDING;
	iCount = 0;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()) && pLoopCity->area() == area())
		{
			if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_CONSTRUCT, getGroup()) == 0)
				{
					if (generatePath(pLoopCity->plot(), 0, true))
					{
						for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
						{
							BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);

							if (NO_BUILDING != eBuilding)
						{
							bool bDoesBuild = false;
							if ((m_pUnitInfo->getForceBuildings(eBuilding))
								|| (m_pUnitInfo->getBuildings(eBuilding)))
							{
								bDoesBuild = true;
							}

							if (bDoesBuild && (pLoopCity->getNumBuilding(eBuilding) > 0))
							{
								iCount++;
								if (iCount >= iMaxCount)
								{
									return false;
								}
							}

							if (bDoesBuild && GET_PLAYER(getOwnerINLINE()).getBuildingClassCount((BuildingClassTypes)GC.getBuildingInfo(eBuilding).getBuildingClassType()) < iMaxSingleBuildingCount)
							{
								if (canConstruct(pLoopCity->plot(), eBuilding))
								{
									iValue = pLoopCity->AI_buildingValue(eBuilding);
/*************************************************************************************************/
/**	BETTER AI (Religion Victory) Sephi                                          		                **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
                                    if(AI_getUnitAIType()==UNITAI_PROPHET)
                                    {
                                        if(pLoopCity->isCapital())
                                        {
                                            iValue+=10000;
                                        }
                                    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

									if ((iValue > iThreshold) && (iValue > iBestValue))
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestConstructPlot = pLoopCity->plot();
											eBestBuilding = eBuilding;
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

	if ((pBestPlot != NULL) && (pBestConstructPlot != NULL) && (eBestBuilding != NO_BUILDING))
	{
		if (atPlot(pBestConstructPlot))
		{
			getGroup()->pushMission(MISSION_CONSTRUCT, eBestBuilding);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_CONSTRUCT, pBestConstructPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_switchHurry()
{
	CvCity* pCity;
	BuildingTypes eBestBuilding;
	int iValue;
	int iBestValue;
	int iI;

	pCity = plot()->getPlotCity();

	if ((pCity == NULL) || (pCity->getOwnerINLINE() != getOwnerINLINE()))
	{
		return false;
	}

	iBestValue = 0;
	eBestBuilding = NO_BUILDING;

	for (iI = 0; iI < GC.getNumBuildingClassInfos(); iI++)
	{
		if (isWorldWonderClass((BuildingClassTypes)iI))
		{
			BuildingTypes eBuilding = (BuildingTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationBuildings(iI);

			if (NO_BUILDING != eBuilding)
		{
				if (pCity->canConstruct(eBuilding))
			{
					if (pCity->getBuildingProduction(eBuilding) == 0)
				{
						if (getMaxHurryProduction(pCity) >= pCity->getProductionNeeded(eBuilding))
					{
							iValue = pCity->AI_buildingValue(eBuilding);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
								eBestBuilding = eBuilding;
							}
						}
					}
				}
			}
		}
	}

	if (eBestBuilding != NO_BUILDING)
	{
		pCity->pushOrder(ORDER_CONSTRUCT, eBestBuilding, -1, false, false, false);

		if (pCity->getProductionBuilding() == eBestBuilding)
		{
			if (canHurry(plot()))
			{
				getGroup()->pushMission(MISSION_HURRY);
				return true;
			}
		}

		FAssert(false);
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_hurry()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestHurryPlot;
	bool bHurry;
	int iTurnsLeft;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestHurryPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (canHurry(pLoopCity->plot()))
			{
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_HURRY, getGroup()) == 0)
					{
						if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
						{
							bHurry = false;

							if (pLoopCity->isProductionBuilding())
							{
								if (isWorldWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pLoopCity->getProductionBuilding()).getBuildingClassType())))
								{
									bHurry = true;
								}
							}

							if (bHurry)
							{
								iTurnsLeft = pLoopCity->getProductionTurnsLeft();

								iTurnsLeft -= iPathTurns;

								if (iTurnsLeft > 8)
								{
									iValue = iTurnsLeft;

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestHurryPlot = pLoopCity->plot();
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestHurryPlot != NULL))
	{
		if (atPlot(pBestHurryPlot))
		{
			getGroup()->pushMission(MISSION_HURRY);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_HURRY, pBestHurryPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_greatWork()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestGreatWorkPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestGreatWorkPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (canGreatWork(pLoopCity->plot()))
			{
				if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_GREAT_WORK, getGroup()) == 0)
					{
						if (generatePath(pLoopCity->plot(), 0, true))
						{
							iValue = pLoopCity->AI_calculateCulturePressure(true);
							iValue -= ((100 * pLoopCity->getCulture(pLoopCity->getOwnerINLINE())) / std::max(1, getGreatWorkCulture(pLoopCity->plot())));
							if (iValue > 0)
							{
								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestGreatWorkPlot = pLoopCity->plot();
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestGreatWorkPlot != NULL))
	{
		if (atPlot(pBestGreatWorkPlot))
		{
			getGroup()->pushMission(MISSION_GREAT_WORK);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_GREAT_WORK, pBestGreatWorkPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_offensiveAirlift()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pTargetCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	if (area()->getTargetCity(getOwnerINLINE()) != NULL)
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity->area() != pCity->area())
		{
			if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
			{
				pTargetCity = pLoopCity->area()->getTargetCity(getOwnerINLINE());

				if (pTargetCity != NULL)
				{
					AreaAITypes eAreaAIType = pTargetCity->area()->getAreaAIType(getTeam());
					if (((eAreaAIType == AREAAI_OFFENSIVE) || (eAreaAIType == AREAAI_DEFENSIVE) || (eAreaAIType == AREAAI_MASSING))
						|| pTargetCity->AI_isDanger())
					{
						iValue = 10000;

						iValue *= (GET_PLAYER(getOwnerINLINE()).AI_militaryWeight(pLoopCity->area()) + 10);
						iValue /= (GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(pLoopCity->area(), AI_getUnitAIType()) + 10);

						iValue += std::max(1, ((GC.getMapINLINE().maxStepDistance() * 2) - GC.getMapINLINE().calculatePathDistance(pLoopCity->plot(), pTargetCity->plot())));

						if (AI_getUnitAIType() == UNITAI_PARADROP)
						{
							CvCity* pNearestEnemyCity = GC.getMapINLINE().findCity(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), NO_PLAYER, NO_TEAM, false, false, getTeam());

							if (pNearestEnemyCity != NULL)
							{
								int iDistance = plotDistance(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), pNearestEnemyCity->getX_INLINE(), pNearestEnemyCity->getY_INLINE());
								if (iDistance <= getDropRange())
								{
									iValue *= 5;
								}
							}
						}

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopCity->plot();
							FAssert(pLoopCity != pCity);
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_paradrop(int iRange)
{
	PROFILE_FUNC();

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}
	int iParatrooperCount = plot()->plotCount(PUF_isUnitAIType, UNITAI_PARADROP, -1, getOwnerINLINE());
	FAssert(iParatrooperCount > 0);

	CvPlot* pPlot = plot();

	if (!canParadrop(pPlot))
	{
		return false;
	}

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	int iSearchRange = AI_searchRange(iRange);

	for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
	{
		for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (isPotentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
				{
					if (canParadropAt(pPlot, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()))
					{
						int iValue = 0;

						PlayerTypes eTargetPlayer = pLoopPlot->getOwnerINLINE();
						FAssert(NO_PLAYER != eTargetPlayer);

						if (NO_BONUS != pLoopPlot->getBonusType())
						{
							iValue += GET_PLAYER(eTargetPlayer).AI_bonusVal(pLoopPlot->getBonusType()) - 10;
						}

						for (int i = -1; i <= 1; ++i)
						{
							for (int j = -1; j <= 1; ++j)
							{
								CvPlot* pAdjacentPlot = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), i, j);
								if (NULL != pAdjacentPlot)
								{
									CvCity* pAdjacentCity = pAdjacentPlot->getPlotCity();

									if (NULL != pAdjacentCity)
									{
										if (pAdjacentCity->getOwnerINLINE() == eTargetPlayer)
										{
											int iAttackerCount = GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pAdjacentPlot, true);
											int iDefenderCount = pAdjacentPlot->getNumVisibleEnemyDefenders(this);
											iValue += 20 * (AI_attackOdds(pAdjacentPlot, true) - ((50 * iDefenderCount) / (iParatrooperCount + iAttackerCount)));
										}
									}
								}
							}
						}

						if (iValue > 0)
						{
							iValue += pLoopPlot->defenseModifier(getTeam(), ignoreBuildingDefense());

							CvUnit* pInterceptor = bestInterceptor(pLoopPlot);
							if (NULL != pInterceptor)
							{
								int iInterceptProb = isSuicide() ? 100 : pInterceptor->currInterceptionProbability();

								iInterceptProb *= std::max(0, (100 - evasionProbability()));
								iInterceptProb /= 100;

								iValue *= std::max(0, 100 - iInterceptProb / 2);
								iValue /= 100;
							}
						}

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;

							FAssert(pBestPlot != pPlot);
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_PARADROP, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_protect(int iOddsThreshold)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
			{
				if (pLoopPlot->isVisibleEnemyUnit(this))
				{
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We don't have to check for a path to distant shores if we want to move only short distance  **/
/**	anyway, so approx maximum distance for a possible path by iMaxPath                			**/
/*************************************************************************************************/
/** Orig Code Start
					if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, 0, true))
					{
						iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

						if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
						{
							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								FAssert(!atPlot(pBestPlot));
							}
						}
					}
/** Orig Code End **/
                    int XDist=pLoopPlot->getX_INLINE() - plot()->getX_INLINE();
                    int YDist=pLoopPlot->getY_INLINE() - plot()->getY_INLINE();
                    if (((XDist*XDist)+(YDist*YDist))<10)
                    {
                        if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, 0, true))
                        {
                            iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

                            if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
                            {
                                if (iValue > iBestValue)
                                {
                                    iBestValue = iValue;
                                    pBestPlot = getPathEndTurnPlot();
                                    FAssert(!atPlot(pBestPlot));
                                }
                            }
                        }
                    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_patrol()
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (AI_plotValid(pAdjacentPlot))
			{
				if (!(pAdjacentPlot->isVisibleEnemyUnit(this)))
				{
					if (generatePath(pAdjacentPlot, 0, true))
					{
						iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Patrol"));

						if (isBarbarian())
						{
							if (!(pAdjacentPlot->isOwned()))
							{
								iValue += 20000;
							}

							if (!(pAdjacentPlot->isAdjacentOwned()))
							{
								iValue += 10000;
							}
						}
						else
						{
							if (pAdjacentPlot->isRevealedGoody(getTeam()))
							{
								iValue += 100000;
							}

							if (pAdjacentPlot->getOwnerINLINE() == getOwnerINLINE())
							{
								iValue += 10000;
							}
						}

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = getPathEndTurnPlot();
							FAssert(!atPlot(pBestPlot));
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_defend()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	if (AI_defendPlot(plot()))
	{
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}

	iSearchRange = AI_searchRange(1);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (AI_defendPlot(pLoopPlot))
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								if (iPathTurns <= 1)
								{
									iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Defend"));

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
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_safety()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pHeadUnit;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iCount;
	int iPass;
	int iDX, iDY;

	iSearchRange = AI_searchRange(1);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iPass = 0; iPass < 2; iPass++)
	{
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (AI_plotValid(pLoopPlot))
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pLoopPlot, ((iPass > 0) ? MOVE_IGNORE_DANGER : 0), true, &iPathTurns))
							{
								if (iPathTurns <= 1)
								{
									iCount = 0;

									pUnitNode = pLoopPlot->headUnitNode();

									while (pUnitNode != NULL)
									{
										pLoopUnit = ::getUnit(pUnitNode->m_data);
										pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

										if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
										{
											if (pLoopUnit->canDefend())
											{
												pHeadUnit = pLoopUnit->getGroup()->getHeadUnit();
												FAssert(pHeadUnit != NULL);
												FAssert(getGroup()->getHeadUnit() == this);

												if (pHeadUnit != this)
												{
													if (pHeadUnit->isWaiting() || !(pHeadUnit->canMove()))
													{
														FAssert(pLoopUnit != this);
														FAssert(pHeadUnit != getGroup()->getHeadUnit());
														iCount++;
													}
												}
											}
										}
									}

									iValue = (iCount * 100);

									iValue += pLoopPlot->defenseModifier(getTeam(), false);

									if (atPlot(pLoopPlot))
									{
										iValue += 50;
									}
									else
									{
										iValue += GC.getGameINLINE().getSorenRandNum(50, "AI Safety");
									}

									//added Sephi
									//Workers shouldn't move to tiles that are threatened
									if(!canDefend() && iCount==0)
									{
										if(GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pLoopPlot, 3))
										{
											iValue=-1000;
										}
									}
									//end added

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
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((iPass > 0) ? MOVE_IGNORE_DANGER : 0));
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_hide()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pHeadUnit;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	bool bValid;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iCount;
	int iDX, iDY;
	int iI;

	if (getInvisibleType() == NO_INVISIBLE)
	{
		return false;
	}

	iSearchRange = AI_searchRange(1);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					bValid = true;

					for (iI = 0; iI < MAX_TEAMS; iI++)
					{
						if (GET_TEAM((TeamTypes)iI).isAlive())
						{
							if (pLoopPlot->isInvisibleVisible(((TeamTypes)iI), getInvisibleType()))
							{
								bValid = false;
								break;
							}
						}
					}

					if (bValid)
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								if (iPathTurns <= 1)
								{
									iCount = 1;

									pUnitNode = pLoopPlot->headUnitNode();

									while (pUnitNode != NULL)
									{
										pLoopUnit = ::getUnit(pUnitNode->m_data);
										pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

										if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
										{
											if (pLoopUnit->canDefend())
											{
												pHeadUnit = pLoopUnit->getGroup()->getHeadUnit();
												FAssert(pHeadUnit != NULL);
												FAssert(getGroup()->getHeadUnit() == this);

												if (pHeadUnit != this)
												{
													if (pHeadUnit->isWaiting() || !(pHeadUnit->canMove()))
													{
														FAssert(pLoopUnit != this);
														FAssert(pHeadUnit != getGroup()->getHeadUnit());
														iCount++;
													}
												}
											}
										}
									}

									iValue = (iCount * 100);

									iValue += pLoopPlot->defenseModifier(getTeam(), false);

									if (atPlot(pLoopPlot))
									{
										iValue += 50;
									}
									else
									{
										iValue += GC.getGameINLINE().getSorenRandNum(50, "AI Hide");
									}

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
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_goody(int iRange)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
//	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;
//	int iI;

	if (isBarbarian())
	{
		return false;
	}

	iSearchRange = AI_searchRange(iRange);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isRevealedGoody(getTeam()))
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								if (iPathTurns <= iRange)
								{
									iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "AI Goody"));

									iValue /= (iPathTurns + 1);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
									}
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
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_explore()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestExplorePlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI, iJ;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestExplorePlot = NULL;

	bool bNoContact = (GC.getGameINLINE().countCivTeamsAlive() > GET_TEAM(getTeam()).getHasMetCivCount(true));

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		PROFILE("AI_explore 1");

		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			iValue = 0;

			if (pLoopPlot->isRevealedGoody(getTeam()))
			{
				iValue += 100000;
			}

			if (iValue > 0 || GC.getGameINLINE().getSorenRandNum(4, "AI make explore faster ;)") == 0)
			{
				if (!(pLoopPlot->isRevealed(getTeam(), false)))
				{
					iValue += 10000;
				}
				// XXX is this too slow?
				for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
				{
					PROFILE("AI_explore 2");

					pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iJ));

					if (pAdjacentPlot != NULL)
					{
						if (!(pAdjacentPlot->isRevealed(getTeam(), false)))
						{
							iValue += 1000;
						}
						else if (bNoContact)
						{
							if (pAdjacentPlot->getRevealedTeam(getTeam(), false) != pAdjacentPlot->getTeam())
							{
								iValue += 100;
							}
						}
					}
				}

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
						{
							if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
							{
								iValue += GC.getGameINLINE().getSorenRandNum(250 * abs(xDistance(getX_INLINE(), pLoopPlot->getX_INLINE())) + abs(yDistance(getY_INLINE(), pLoopPlot->getY_INLINE())), "AI explore");

								if (pLoopPlot->isAdjacentToLand())
								{
									iValue += 10000;
								}

								if (pLoopPlot->isOwned())
								{
									iValue += 5000;
								}

								iValue /= 3 + std::max(1, iPathTurns);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = pLoopPlot->isRevealedGoody(getTeam()) ? getPathEndTurnPlot() : pLoopPlot;
									pBestExplorePlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestExplorePlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_exploreRange(int iRange)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestExplorePlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;
	int iI;

	iSearchRange = AI_searchRange(iRange);

	iBestValue = 0;
	pBestPlot = NULL;
	pBestExplorePlot = NULL;

	int iImpassableCount = GET_PLAYER(getOwnerINLINE()).AI_unitImpassableCount(getUnitType());

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			PROFILE("AI_exploreRange 1");

			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					iValue = 0;

					if (pLoopPlot->isRevealedGoody(getTeam()))
					{
						iValue += 100000;
					}

					if (!(pLoopPlot->isRevealed(getTeam(), false)))
					{
						iValue += 10000;
					}

					for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
					{
						PROFILE("AI_exploreRange 2");

						pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));

						if (pAdjacentPlot != NULL)
						{
							if (!(pAdjacentPlot->isRevealed(getTeam(), false)))
							{
								iValue += 1000;
							}
						}
					}

					if (iValue > 0)
					{
						if (!(pLoopPlot->isVisibleEnemyUnit(this)))
						{
							PROFILE("AI_exploreRange 3");

							if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_EXPLORE, getGroup(), 3) == 0)
							{
								PROFILE("AI_exploreRange 4");

								if (!atPlot(pLoopPlot) && generatePath(pLoopPlot, MOVE_NO_ENEMY_TERRITORY, true, &iPathTurns))
								{
									if (iPathTurns <= iRange)
									{
										iValue += GC.getGameINLINE().getSorenRandNum(10000, "AI Explore");

										if (pLoopPlot->isAdjacentToLand())
										{
											iValue += 10000;
										}

										if (pLoopPlot->isOwned())
										{
											iValue += 5000;
										}

										if (!isHuman())
										{
											int iDirectionModifier = 100;

											if (AI_getUnitAIType() == UNITAI_EXPLORE_SEA && iImpassableCount == 0)
											{
												iDirectionModifier += (50 * (abs(iDX) + abs(iDY))) / iSearchRange;
												if (GC.getGame().circumnavigationAvailable())
												{
													if (GC.getMap().isWrapX())
													{
														if ((iDX * ((AI_getBirthmark() % 2 == 0) ? 1 : -1)) > 0)
														{
															iDirectionModifier *= 150 + ((iDX * 100) / iSearchRange);
														}
														else
														{
															iDirectionModifier /= 2;
														}
													}
													if (GC.getMap().isWrapY())
													{
														if ((iDY * (((AI_getBirthmark() >> 1) % 2 == 0) ? 1 : -1)) > 0)
														{
															iDirectionModifier *= 150 + ((iDY * 100) / iSearchRange);
														}
														else
														{
															iDirectionModifier /= 2;
														}
													}
												}
												iValue *= iDirectionModifier;
												iValue /= 100;
											}
										}

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											if (getDomainType() == DOMAIN_LAND)
											{
												pBestPlot = getPathEndTurnPlot();
											}
											else
											{
												pBestPlot = pLoopPlot;
											}
											pBestExplorePlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestExplorePlot != NULL))
	{
		PROFILE("AI_exploreRange 5");

		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_NO_ENEMY_TERRITORY, false, false, MISSIONAI_EXPLORE, pBestExplorePlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_targetCity(int iFlags)
{
	PROFILE_FUNC();

	CvCity* pTargetCity;
	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestCity = NULL;

	pTargetCity = area()->getTargetCity(getOwnerINLINE());

	if (pTargetCity != NULL)
	{
		if (AI_potentialEnemy(pTargetCity->getTeam(), pTargetCity->plot()))
		{
			if (!atPlot(pTargetCity->plot()) && generatePath(pTargetCity->plot(), iFlags, true))
			{
				pBestCity = pTargetCity;
			}
		}
	}

	if (pBestCity == NULL)
	{
		for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					// BBAI efficiency: check area for land units before generating path
					if (AI_plotValid(pLoopCity->plot()) && (pLoopCity->area() == area()))
					{
						if (AI_potentialEnemy(GET_PLAYER((PlayerTypes)iI).getTeam(), pLoopCity->plot()))
						{
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), iFlags, true, &iPathTurns))
							{
								iValue = 0;
								if (AI_getUnitAIType() == UNITAI_ATTACK_CITY) //lemming?
								{
									iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, false, false);
								}
								else
								{
									iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, true, true);
								}

								iValue *= 1000;

								if ((area()->getAreaAIType(getTeam()) == AREAAI_DEFENSIVE))
								{
									if (pLoopCity->calculateCulturePercent(getOwnerINLINE()) < 75)
									{
										iValue /= 2;
									}
								}

								// If city is minor civ, less interesting
								if( GET_PLAYER(pLoopCity->getOwnerINLINE()).isMinorCiv() )
								{
									iValue /= 2;
								}

								// If stack has poor bombard, direct towards lower defense cities
								iPathTurns += std::min(16, getGroup()->getBombardTurns(pLoopCity))/2;

// Better War AI (Skyre)
								// Try to make sure we always find a target, even against far away civs
								iValue *= 100;
// End Better War AI
								iValue /= (4 + iPathTurns*iPathTurns);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestCity = pLoopCity;
								}
							}
						}
					}
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		iBestValue = 0;
		pBestPlot = NULL;

		if (0 == (iFlags & MOVE_THROUGH_ENEMY))
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pAdjacentPlot = plotDirection(pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), ((DirectionTypes)iI));

				if (pAdjacentPlot != NULL)
				{
					if (AI_plotValid(pAdjacentPlot))
					{
						if (!(pAdjacentPlot->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pAdjacentPlot, iFlags, true, &iPathTurns))
							{
								iValue = std::max(0, (pAdjacentPlot->defenseModifier(getTeam(), false) + 100));

								if (!(pAdjacentPlot->isRiverCrossing(directionXY(pAdjacentPlot, pBestCity->plot()))))
								{
									iValue += (12 * -(GC.getRIVER_ATTACK_MODIFIER()));
								}

								if (!isEnemy(pAdjacentPlot->getTeam(), pAdjacentPlot))
								{
									iValue += 100;
								}

								iValue = std::max(1, iValue);

								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
								}
							}
						}
					}
				}
			}
		}


		else
		{
			pBestPlot =  pBestCity->plot();
		}

		if (pBestPlot != NULL)
		{
/** logging added by Skyre **/
			if (GC.getDefineINT("USE_DEBUG_LOG") == 1)
			{
				TCHAR szMsgLog[1024];

				sprintf(szMsgLog,
						"*** %d: %S is targeting %S (%d) ***\n",
						GC.getGameINLINE().getGameTurn(),
						(const wchar*)GET_PLAYER(getOwnerINLINE()).getName(),
						(const wchar*)pBestCity->getName(),
						iBestValue);

				gDLL->logMsg("war.log", szMsgLog);
			}
/** logging added by Skyre **/

			FAssert(!(pBestCity->at(pBestPlot)) || 0 != (iFlags & MOVE_THROUGH_ENEMY)); // no suicide missions...
			if (!atPlot(pBestPlot))
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), iFlags);
				return true;
			}
		}
	}

/** logging added by Skyre **/
	if (GC.getDefineINT("USE_DEBUG_LOG") == 1)
	{
		if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)
		{
			TCHAR szMsgLog[1024];

			sprintf(szMsgLog,
					"%d: %S failed to find a target\n",
					GC.getGameINLINE().getGameTurn(),
					(const wchar*)GET_PLAYER(getOwnerINLINE()).getName());

			gDLL->logMsg("war.log", szMsgLog);
		}
	}
/** logging added by Skyre **/

	return false;
}

/*************************************************************************************************/
/** Skyre Mod                                                                                   **/
/** BETTER AI (Don't attack Archeron) merged Sephi                                              **/
/**						                                            							**/
/*************************************************************************************************/

// Returns true if a mission was pushed...
bool CvUnitAI::AI_targetBarbCity()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pAdjacentPlot;
	CvPlot* pBestPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	if (isBarbarian())
	{
		return false;
	}

	iBestValue = 0;
	pBestCity = NULL;

	for (pLoopCity = GET_PLAYER(BARBARIAN_PLAYER).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(BARBARIAN_PLAYER).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (pLoopCity->isRevealed(getTeam(), false))
			{
				if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
				{
					if (iPathTurns < 10)
					{
// Acheron Hack (Skyre)
                        CvUnit* pHeadUnit = ::getUnit(getGroup()->headUnitNode()->m_data);
                        CvUnit* pBestDefender = pLoopCity->plot()->getBestDefender(BARBARIAN_PLAYER, getOwnerINLINE(), pHeadUnit);

                        // Avoid dragon city unless we're very powerful
                        if (pBestDefender != NULL &&
							pBestDefender->getUnitType() == GC.getInfoTypeForString("UNIT_ACHERON") &&
                            pHeadUnit->baseCombatStr() <= pBestDefender->baseCombatStrDefense())
                        {
							continue;
						}
// End Acheron Hack
						iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, false);

						iValue *= 1000;

						iValue /= (iPathTurns + 1);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestCity = pLoopCity;
						}
					}
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		iBestValue = 0;
		pBestPlot = NULL;

		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pAdjacentPlot = plotDirection(pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if (AI_plotValid(pAdjacentPlot))
				{
					if (!(pAdjacentPlot->isVisibleEnemyUnit(this)))
					{
						if (generatePath(pAdjacentPlot, 0, true, &iPathTurns))
						{
							iValue = std::max(0, (pAdjacentPlot->defenseModifier(getTeam(), false) + 100));

							if (!(pAdjacentPlot->isRiverCrossing(directionXY(pAdjacentPlot, pBestCity->plot()))))
							{
								iValue += (10 * -(GC.getRIVER_ATTACK_MODIFIER()));
							}

							iValue = std::max(1, iValue);

							iValue *= 1000;

							iValue /= (iPathTurns + 1);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
							}
						}
					}
				}
			}
		}

		if (pBestPlot != NULL)
		{
			FAssert(!(pBestCity->at(pBestPlot))); // no suicide missions...
			if (atPlot(pBestPlot))
			{
				getGroup()->pushMission(MISSION_SKIP);
				return true;
			}
			else
			{
/** logging added by Skyre **/
				if (GC.getDefineINT("USE_DEBUG_LOG") == 1)
				{
					TCHAR szMsgLog[1024];

					sprintf(szMsgLog,
							"*** %d: %S is targeting %S (Barbarian) ***\n",
							GC.getGameINLINE().getGameTurn(),
							(const wchar*)GET_PLAYER(getOwnerINLINE()).getName(),
							(const wchar*)pBestCity->getName());

					gDLL->logMsg("war.log", szMsgLog);
				}
/** logging added by Skyre **/

				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
				return true;
			}
		}
	}

	return false;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


// Returns true if a mission was pushed...
bool CvUnitAI::AI_bombardCity()
{
	CvCity* pBombardCity;

	if (canBombard(plot()))
	{
		pBombardCity = bombardTarget(plot());
		FAssertMsg(pBombardCity != NULL, "BombardCity is not assigned a valid value");

		// do not bombard cities with no defenders
		int iDefenderStrength = pBombardCity->plot()->AI_sumStrength(NO_PLAYER, getOwnerINLINE(), DOMAIN_LAND, /*bDefensiveBonuses*/ true, /*bTestAtWar*/ true, false);
		if (iDefenderStrength == 0)
		{
			return false;
		}

		// do not bombard cities if we have overwelming odds
		int iAttackOdds = getGroup()->AI_attackOdds(pBombardCity->plot(), /*bPotentialEnemy*/ true);
		if (iAttackOdds > 95)
		{
			return false;
		}

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/28/09                                jdog5000      */
/*                                                                                              */
/* War Tactics AI                                                                               */
/************************************************************************************************/
		// If we have reasonable odds, check for attacking without waiting for bombards
		if( iAttackOdds > std::max(1, GC.getDefineINT("BBAI_SKIP_BOMBARD_BEST_ATTACK_ODDS")) )
		{
			int iBase = GC.getDefineINT("BBAI_SKIP_BOMBARD_BASE_STACK_RATIO");
			iBase = ((iBase == 0) ? 300 : iBase);
			int iComparison = getGroup()->AI_compareStacks(pBombardCity->plot(), /*bPotentialEnemy*/ true, /*bCheckCanAttack*/ true, /*bCheckCanMove*/ true);

			// Big troop advantage plus pretty good starting odds, don't wait to allow reinforcements
			if( iComparison > (iBase - 2*iAttackOdds) )
			{
				return false;
			}

			int iMin = GC.getDefineINT("BBAI_SKIP_BOMBARD_MIN_STACK_RATIO");
			iMin = ((iMin == 0) ? 140 : iMin);
			// Bombard at least one turn to allow bombers/ships to get some shots in too
			if( (getFortifyTurns() > 0) && (pBombardCity->getDefenseDamage() > ((GC.getMAX_CITY_DEFENSE_DAMAGE()) / 5)) )
			{
				int iBombardTurns = getGroup()->getBombardTurns(pBombardCity);
				if( iComparison > std::max(iMin, iBase - 2*iAttackOdds - 3*iBombardTurns) )
				{
					return false;
				}
			}
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/


		// could also do a compare stacks call here if we wanted, the downside of that is that we may just have a lot more units
		// we may not want to suffer high casualties just to save a turn
		//getGroup()->AI_compareStacks(pBombardCity->plot(), /*bPotentialEnemy*/ true, /*bCheckCanAttack*/ true, /*bCheckCanMove*/ true);
		//int iOurStrength = pBombardCity->plot()->AI_sumStrength(getOwnerINLINE(), NO_PLAYER, DOMAIN_LAND, false, false, false)

		if (pBombardCity->getDefenseDamage() < ((GC.getMAX_CITY_DEFENSE_DAMAGE() * 3) / 4))
		{
			getGroup()->pushMission(MISSION_BOMBARD);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_cityAttack(int iRange, int iOddsThreshold, bool bFollow)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	if (bFollow)
	{
		iSearchRange = 1;
	}
	else
	{
		iSearchRange = AI_searchRange(iRange);
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true, getTeam()) && pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
						{
							if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
							{
								iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

								if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
								{
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
										FAssert(!atPlot(pBestPlot));
									}
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
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_anyAttack(int iRange, int iOddsThreshold, int iMinStack, bool bFollow)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

/*************************************************************************************************/
/**	BETTER AI (Ranged Attacks) Sephi                                           					**/
/*************************************************************************************************/
/** orig    **/
	if (AI_rangeAttack(iRange))
	{
		return true;
	}
/**
    //before we make a range attack, check if we can attack at very high odds
    if (canRangeStrike(true))
    {
        if (iOddsThreshold<80)
        {
            if (AI_anyAttack(1,80))
            {
                return true;
            }
        }

        if (AI_rangeAttack(iRange))
        {
            return true;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	if (bFollow)
	{
		iSearchRange = 1;
	}
	else
	{
		iSearchRange = AI_searchRange(iRange);
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isVisibleEnemyUnit(this) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam())))
					{
						if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
						{
							if (pLoopPlot->getNumVisibleEnemyDefenders(this) >= iMinStack)
							{								
								iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

//								if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
								if(!isBarbarian() || !isAnimal() || iValue < 90) {		//quick fix to make animals ignore workers and other great people
									if (isHasSecondChance() || iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
									{
										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
											FAssert(!atPlot(pBestPlot));
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

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
		return true;
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_rangeAttack(int iRange)
{
	PROFILE_FUNC();

	FAssert(canMove());

	if (!canRangeStrike())
	{
		return false;
	}

	int iSearchRange = AI_searchRange(iRange);

	int iBestValue = -100;
	CvPlot* pBestPlot = NULL;

	for (int iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (int iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->isVisibleEnemyUnit(this) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam())))
				{
					if (!atPlot(pLoopPlot) && canRangeStrikeAt(plot(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()))
					{
						int iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

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

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_RANGE_ATTACK, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0);
		return true;
	}

	return false;
}

bool CvUnitAI::AI_leaveAttack(int iRange, int iOddsThreshold, int iStrengthThreshold)
{
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvCity* pCity;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	iSearchRange = iRange;

	iBestValue = 0;
	pBestPlot = NULL;


	pCity = plot()->getPlotCity();

	if ((pCity != NULL) && (pCity->getOwnerINLINE() == getOwnerINLINE()))
	{
		int iOurStrength = GET_PLAYER(getOwnerINLINE()).AI_getOurPlotStrength(plot(), 0, false, false);
    	int iEnemyStrength = GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(plot(), 2, false, false);
		if (iEnemyStrength > 0)
		{
    		if (((iOurStrength * 100) / iEnemyStrength) < iStrengthThreshold)
    		{
    			return false;
    		}
    		if (plot()->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE()) <= getGroup()->getNumUnits())
    		{
    			return false;
    		}
		}
	}

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isVisibleEnemyUnit(this) || (pLoopPlot->isCity() && AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot)))
					{
						if (!atPlot(pLoopPlot) && (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange)))
						{
							if (pLoopPlot->getNumVisibleEnemyDefenders(this) > 0)
							{
								iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

								if (iValue >= AI_finalOddsThreshold(pLoopPlot, iOddsThreshold))
								{
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										FAssert(!atPlot(pBestPlot));
									}
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
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0);
		return true;
	}

	return false;

}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_blockade()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestBlockadePlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestBlockadePlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (potentialWarAction(pLoopPlot))
			{
				pCity = pLoopPlot->getWorkingCity();

				if (pCity != NULL)
				{
					if (pCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
					{
						if (!(pCity->isBarbarian()))
						{
							FAssert(isEnemy(pCity->getTeam()) || GET_TEAM(getTeam()).AI_getWarPlan(pCity->getTeam()) != NO_WARPLAN);

							if (!(pLoopPlot->isVisibleEnemyUnit(this)) && canPlunder(pLoopPlot))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BLOCKADE, getGroup(), 2) == 0)
								{
									if (generatePath(pLoopPlot, 0, true, &iPathTurns))
									{
										iValue = 1;

										iValue += std::min(pCity->getPopulation(), pCity->countNumWaterPlots());

										iValue += GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pCity->plot());

										iValue += (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pCity->plot(), MISSIONAI_ASSAULT, getGroup(), 2) * 3);

										if (canBombard(pLoopPlot))
										{
											iValue *= 2;
										}

										iValue *= 1000;

										iValue /= (iPathTurns + 1);

										if (iPathTurns == 1)
										{
											//Prefer to have movement remaining to Bombard + Plunder
											iValue *= 1 + std::min(2, getPathLastNode()->m_iData1);
										}

										// if not at war with this plot owner, then devalue plot if we already inside this owner's borders
										// (because declaring war will pop us some unknown distance away)
										if (!isEnemy(pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
										{
											iValue /= 10;
										}

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											pBestBlockadePlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestBlockadePlot != NULL))
	{
		FAssert(canPlunder(pBestBlockadePlot));
		if (atPlot(pBestBlockadePlot) && !isEnemy(pBestBlockadePlot->getTeam(), pBestBlockadePlot))
		{
			getGroup()->groupDeclareWar(pBestBlockadePlot, true);
		}

		if (atPlot(pBestBlockadePlot))
		{
			if (canBombard(plot()))
			{
				getGroup()->pushMission(MISSION_BOMBARD, -1, -1, 0, false, false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
			}

			getGroup()->pushMission(MISSION_PLUNDER, -1, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BLOCKADE, pBestBlockadePlot);

			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_pirateBlockade()
{
	PROFILE_FUNC();

	int iPathTurns;
	int iValue;
	int iI;

	std::vector<int> aiDeathZone(GC.getMapINLINE().numPlotsINLINE(), 0);

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if (AI_plotValid(pLoopPlot) || (pLoopPlot->isCity() && pLoopPlot->isAdjacentToArea(area())))
		{
			if (pLoopPlot->isOwned() && (pLoopPlot->getTeam() != getTeam()))
			{
				int iBestHostileMoves = 0;
				CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
					if (isEnemy(pLoopUnit->getTeam(), pLoopUnit->plot()))
					{
						if (pLoopUnit->getDomainType() == DOMAIN_SEA && !pLoopUnit->isInvisible(getTeam(), false))
						{
							if (pLoopUnit->canAttack())
							{
								if (pLoopUnit->currEffectiveStr(NULL, NULL, NULL) > currEffectiveStr(pLoopPlot, pLoopUnit, NULL))
								{
									iBestHostileMoves = std::max(iBestHostileMoves, pLoopUnit->getMoves());
								}
							}
						}
					}
				}
				if (iBestHostileMoves > 0)
				{
					for (int iX = -iBestHostileMoves; iX <= iBestHostileMoves; iX++)
					{
						for (int iY = -iBestHostileMoves; iY <= iBestHostileMoves; iY++)
						{
							CvPlot * pRangePlot = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iX, iY);
							if (pRangePlot != NULL)
							{
								aiDeathZone[GC.getMap().plotNumINLINE(pRangePlot->getX_INLINE(), pRangePlot->getY_INLINE())]++;
							}
						}
					}
				}
			}
		}
	}

	bool bIsInDanger = aiDeathZone[GC.getMap().plotNumINLINE(getX_INLINE(), getY_INLINE())] > 0;

	if (!bIsInDanger)
	{
		if (getDamage() > 0)
		{
			if (!plot()->isOwned() && !plot()->isAdjacentOwned())
			{
				if (AI_retreatToCity(false, false, 1 + getDamage() / 20))
				{
					return true;
				}
				getGroup()->pushMission(MISSION_SKIP);
				return true;
			}
		}
	}

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestBlockadePlot = NULL;
	bool bBestIsForceMove = false;
	bool bBestIsMove = false;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (!(pLoopPlot->isVisibleEnemyUnit(this)) && canPlunder(pLoopPlot))
			{
				if (GC.getGame().getSorenRandNum(4, "AI Pirate Blockade") == 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BLOCKADE, getGroup(), 3) == 0)
					{
						if (generatePath(pLoopPlot, 0, true, &iPathTurns))
						{
							int iBlockadedCount = 0;
							int iPopulationValue = 0;
							int iRange = GC.getDefineINT("SHIP_BLOCKADE_RANGE") - 1;
							for (int iX = -iRange; iX <= iRange; iX++)
							{
								for (int iY = -iRange; iY <= iRange; iY++)
								{
									CvPlot* pRangePlot = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iX, iY);
									if (pRangePlot != NULL)
									{
										bool bPlotBlockaded = false;
										if (pRangePlot->isWater() && pRangePlot->isOwned() && isEnemy(pRangePlot->getTeam(), pLoopPlot))
										{
											bPlotBlockaded = true;
											iBlockadedCount += pRangePlot->getBlockadedCount(pRangePlot->getTeam());
										}

										if (!bPlotBlockaded)
										{
											CvCity* pPlotCity = pRangePlot->getPlotCity();
											if (pPlotCity != NULL)
											{
												if (isEnemy(pPlotCity->getTeam(), pLoopPlot))
												{
													int iCityValue = 3 + pPlotCity->getPopulation();
													iCityValue *= (atWar(getTeam(), pPlotCity->getTeam()) ? 1 : 3);
													if (GET_PLAYER(pPlotCity->getOwnerINLINE()).isNoForeignTrade())
													{
														iCityValue /= 2;
													}
													iPopulationValue += iCityValue;

												}
											}
										}
									}
								}
							}
							iValue = iPopulationValue;

							iValue *= 1000;

							iValue /= 16 + iBlockadedCount;

							bool bMove = ((getPathLastNode()->m_iData2 == 1) && getPathLastNode()->m_iData1 > 0);
							if (atPlot(pLoopPlot))
							{
								iValue *= 3;
							}
							else if (bMove)
							{
								iValue *= 2;
							}

							int iDeath = aiDeathZone[GC.getMap().plotNumINLINE(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE())];

							bool bForceMove = false;
							if (iDeath)
							{
								iValue /= 10;
							}
							else if (bIsInDanger && (iPathTurns <= 2) && (0 == iPopulationValue))
							{
								if (getPathLastNode()->m_iData1 == 0)
								{
									if (!pLoopPlot->isAdjacentOwned())
									{
										int iRand = GC.getGame().getSorenRandNum(2500, "AI Pirate Retreat");
										iValue += iRand;
										if (iRand > 1000)
										{
											iValue += GC.getGame().getSorenRandNum(2500, "AI Pirate Retreat");
											bForceMove = true;
										}
									}
								}
							}

							if (!bForceMove)
							{
								iValue /= iPathTurns + 1;
							}

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = bForceMove ? pLoopPlot : getPathEndTurnPlot();
								pBestBlockadePlot = pLoopPlot;
								bBestIsForceMove = bForceMove;
								bBestIsMove = bMove;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestBlockadePlot != NULL))
	{
		FAssert(canPlunder(pBestBlockadePlot));

		if (atPlot(pBestBlockadePlot))
		{
			getGroup()->pushMission(MISSION_PLUNDER, -1, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			if (bBestIsForceMove)
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
				if (bBestIsMove)
				{
					getGroup()->pushMission(MISSION_PLUNDER, -1, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BLOCKADE, pBestBlockadePlot);
				}
				return true;
			}
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_seaBombardRange(int iMaxRange)
{
	PROFILE_FUNC();

	// cached values
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvPlot* pPlot = plot();
	CvSelectionGroup* pGroup = getGroup();

	// can any unit in this group bombard?
	bool bHasBombardUnit = false;
	bool bBombardUnitCanBombardNow = false;
	CLLNode<IDInfo>* pUnitNode = pGroup->headUnitNode();
	while (pUnitNode != NULL && !bBombardUnitCanBombardNow)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pGroup->nextUnitNode(pUnitNode);

		if (pLoopUnit->bombardRate() > 0)
		{
			bHasBombardUnit = true;

			if (pLoopUnit->canMove() && !pLoopUnit->isMadeAttack())
			{
				bBombardUnitCanBombardNow = true;
			}
		}
	}

	if (!bHasBombardUnit)
	{
		return false;
	}

	// best match
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestBombardPlot = NULL;
	int iBestValue = 0;

	// iterate over plots at each range
	for (int iDX = -(iMaxRange); iDX <= iMaxRange; iDX++)
	{
		for (int iDY = -(iMaxRange); iDY <= iMaxRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL && AI_plotValid(pLoopPlot))
			{
				CvCity* pBombardCity = bombardTarget(pLoopPlot);

				if (pBombardCity != NULL && isEnemy(pBombardCity->getTeam(), pLoopPlot) && pBombardCity->getDefenseDamage() < GC.getMAX_CITY_DEFENSE_DAMAGE())
				{
					int iPathTurns;
					if (generatePath(pLoopPlot, 0, true, &iPathTurns))
					{
						int iValue = (AI_getUnitAIType() == UNITAI_ASSAULT_SEA) ? 0 : 1;

						iValue += (kPlayer.AI_plotTargetMissionAIs(pBombardCity->plot(), MISSIONAI_ASSAULT, NULL, 2) * 3);
						iValue += (kPlayer.AI_adjacentPotentialAttackers(pBombardCity->plot(), true));

						if (iValue > 0)
						{
							iValue *= 1000;

							iValue /= (iPathTurns + 1);

							if (iPathTurns == 1)
							{
								//Prefer to have movement remaining to Bombard + Plunder
								iValue *= 1 + std::min(2, getPathLastNode()->m_iData1);
							}

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestBombardPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestBombardPlot != NULL))
	{
		if (atPlot(pBestBombardPlot))
		{
			// if we are at the plot from which to bombard, and we have a unit that can bombard this turn, do it
			if (bBombardUnitCanBombardNow && pGroup->canBombard(pBestBombardPlot))
			{
				getGroup()->pushMission(MISSION_BOMBARD, -1, -1, 0, false, false, MISSIONAI_BLOCKADE, pBestBombardPlot);

				// if city bombarded enough, wake up any units that were waiting to bombard this city
				CvCity* pBombardCity = bombardTarget(pBestBombardPlot); // is NULL if city cannot be bombarded any more
				if (pBombardCity == NULL || pBombardCity->getDefenseDamage() < ((GC.getMAX_CITY_DEFENSE_DAMAGE()*5)/6))
				{
					kPlayer.AI_wakePlotTargetMissionAIs(pBestBombardPlot, MISSIONAI_BLOCKADE, getGroup());
				}
			}
			// otherwise, skip until next turn, when we will surely bombard
			else if (canPlunder(pBestBombardPlot))
			{
				getGroup()->pushMission(MISSION_PLUNDER, -1, -1, 0, false, false, MISSIONAI_BLOCKADE, pBestBombardPlot);
			}
			else
			{
				getGroup()->pushMission(MISSION_SKIP);
			}

			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BLOCKADE, pBestBombardPlot);
			return true;
		}
	}

	return false;
}



// Returns true if a mission was pushed...
bool CvUnitAI::AI_pillage(int iBonusValueThreshold)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestPillagePlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;
	pBestPillagePlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot) && !(pLoopPlot->isBarbarian()))
		{
			if (potentialWarAction(pLoopPlot))
			{
			    CvCity * pWorkingCity = pLoopPlot->getWorkingCity();

			    if (pWorkingCity != NULL)
			    {
                    if (!(pWorkingCity == area()->getTargetCity(getOwnerINLINE())) && canPillage(pLoopPlot))
                    {
                        if (!(pLoopPlot->isVisibleEnemyUnit(this)))
                        {
                            if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_PILLAGE, getGroup(), 1) == 0)
                            {
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We don't have to check for a path to distant shores if we want to move only short distance  **/
/**	anyway, so approx maximum distance for a possible path by iMaxPath                			**/
/*************************************************************************************************/
/** Start Orig Code
                               if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                {
                                    iValue = AI_pillageValue(pLoopPlot, iBonusValueThreshold);

                                    iValue *= 1000;

                                    iValue /= (iPathTurns + 1);

                                    // if not at war with this plot owner, then devalue plot if we already inside this owner's borders
                                    // (because declaring war will pop us some unknown distance away)
                                    if (!isEnemy(pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
                                    {
                                        iValue /= 10;
                                    }

                                    if (iValue > iBestValue)
                                    {
                                        iBestValue = iValue;
                                        pBestPlot = getPathEndTurnPlot();
                                        pBestPillagePlot = pLoopPlot;
                                    }
                                }
/** End Orig Code **/
                                int XDist=pLoopPlot->getX_INLINE() - plot()->getX_INLINE();
                                int YDist=pLoopPlot->getY_INLINE() - plot()->getY_INLINE();
                                if (((XDist*XDist)+(YDist*YDist))<200)
                                {
                                    if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                    {
                                        iValue = AI_pillageValue(pLoopPlot, iBonusValueThreshold);

                                        iValue *= 1000;

                                        iValue /= (iPathTurns + 1);

                                        // if not at war with this plot owner, then devalue plot if we already inside this owner's borders
                                        // (because declaring war will pop us some unknown distance away)
                                        if (!isEnemy(pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
                                        {
                                            iValue /= 10;
                                        }

                                        if (iValue > iBestValue)
                                        {
                                            iBestValue = iValue;
                                            pBestPlot = getPathEndTurnPlot();
                                            pBestPillagePlot = pLoopPlot;
                                        }
                                    }
                                }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

                            }
                        }
                    }
			    }
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestPillagePlot != NULL))
	{
		if (atPlot(pBestPillagePlot) && !isEnemy(pBestPillagePlot->getTeam()))
		{
			//getGroup()->groupDeclareWar(pBestPillagePlot, true);
			// rather than declare war, just find something else to do, since we may already be deep in enemy territory
			return false;
		}

		if (atPlot(pBestPillagePlot))
		{
			if (isEnemy(pBestPillagePlot->getTeam()))
			{
				getGroup()->pushMission(MISSION_PILLAGE, -1, -1, 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_canPillage(CvPlot& kPlot) const
{
	if (isEnemy(kPlot.getTeam(), &kPlot))
	{
		return true;
	}

	if (!kPlot.isOwned())
	{
		bool bPillageUnowned = true;

		for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS && bPillageUnowned; ++iPlayer)
		{
			int iIndx;
			CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
			if (!isEnemy(kLoopPlayer.getTeam(), &kPlot))
			{
				for (CvCity* pCity = kLoopPlayer.firstCity(&iIndx); NULL != pCity; pCity = kLoopPlayer.nextCity(&iIndx))
				{
					if (kPlot.getPlotGroup((PlayerTypes)iPlayer) == pCity->plot()->getPlotGroup((PlayerTypes)iPlayer))
					{
						bPillageUnowned = false;
						break;
					}

				}
			}
		}

		if (bPillageUnowned)
		{
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_pillageRange(int iRange, int iBonusValueThreshold)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestPillagePlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = AI_searchRange(iRange);

	iBestValue = 0;
	pBestPlot = NULL;
	pBestPillagePlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && !(pLoopPlot->isBarbarian()))
				{
					if (potentialWarAction(pLoopPlot))
					{
                        CvCity * pWorkingCity = pLoopPlot->getWorkingCity();

                        if (pWorkingCity != NULL)
                        {
                            if (!(pWorkingCity == area()->getTargetCity(getOwnerINLINE())) && canPillage(pLoopPlot))
                            {
                                if (!(pLoopPlot->isVisibleEnemyUnit(this)))
                                {
                                    if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_PILLAGE, getGroup()) == 0)
                                    {
                                        if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                        {
                                            if (getPathLastNode()->m_iData1 == 0)
                                            {
                                                iPathTurns++;
                                            }

                                            if (iPathTurns <= iRange)
                                            {
                                                iValue = AI_pillageValue(pLoopPlot, iBonusValueThreshold);

                                                iValue *= 1000;

                                                iValue /= (iPathTurns + 1);

												// if not at war with this plot owner, then devalue plot if we already inside this owner's borders
												// (because declaring war will pop us some unknown distance away)
												if (!isEnemy(pLoopPlot->getTeam()) && plot()->getTeam() == pLoopPlot->getTeam())
												{
													iValue /= 10;
												}

                                                if (iValue > iBestValue)
                                                {
                                                    iBestValue = iValue;
                                                    pBestPlot = getPathEndTurnPlot();
                                                    pBestPillagePlot = pLoopPlot;
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
		}
	}

	if ((pBestPlot != NULL) && (pBestPillagePlot != NULL))
	{
		if (atPlot(pBestPillagePlot) && !isEnemy(pBestPillagePlot->getTeam()))
		{
			//getGroup()->groupDeclareWar(pBestPillagePlot, true);
			// rather than declare war, just find something else to do, since we may already be deep in enemy territory
			return false;
		}

		if (atPlot(pBestPillagePlot))
		{
			if (isEnemy(pBestPillagePlot->getTeam()))
			{
				getGroup()->pushMission(MISSION_PILLAGE, -1, -1, 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PILLAGE, pBestPillagePlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_found()
{
	PROFILE_FUNC();
//
//	CvPlot* pLoopPlot;
//	CvPlot* pBestPlot;
//	CvPlot* pBestFoundPlot;
//	int iPathTurns;
//	int iValue;
//	int iBestValue;
//	int iI;
//
//	iBestValue = 0;
//	pBestPlot = NULL;
//	pBestFoundPlot = NULL;
//
//	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
//	{
//		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
//
//		if (AI_plotValid(pLoopPlot) && (pLoopPlot != plot() || GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopPlot, 1) <= pLoopPlot->plotCount(PUF_canDefend, -1, -1, getOwnerINLINE())))
//		{
//			if (canFound(pLoopPlot))
//			{
//				iValue = pLoopPlot->getFoundValue(getOwnerINLINE());
//
//				if (iValue > 0)
//				{
//					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
//					{
//						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup(), 3) == 0)
//						{
//							if (generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
//							{
//								iValue *= 1000;
//
//								iValue /= (iPathTurns + 1);
//
//								if (iValue > iBestValue)
//								{
//									iBestValue = iValue;
//									pBestPlot = getPathEndTurnPlot();
//									pBestFoundPlot = pLoopPlot;
//								}
//							}
//						}
//					}
//				}
//			}
//		}
//	}

	int iPathTurns;
	int iValue;
	int iBestFoundValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestFoundPlot = NULL;

	for (int iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
	{
		CvPlot* pCitySitePlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);
		if (pCitySitePlot->getArea() == getArea())
		{
			if (canFound(pCitySitePlot))
			{
				if (!(pCitySitePlot->isVisibleEnemyUnit(this)))
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pCitySitePlot, MISSIONAI_FOUND, getGroup()) == 0)
					{
						if (getGroup()->canDefend() || GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pCitySitePlot, MISSIONAI_GUARD_CITY) > 0)
						{
							if (generatePath(pCitySitePlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
							{
								iValue = pCitySitePlot->getFoundValue(getOwnerINLINE());
								iValue *= 1000;
								iValue /= (iPathTurns + 1);
								if (iValue > iBestFoundValue)
								{
									iBestFoundValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestFoundPlot = pCitySitePlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		if (atPlot(pBestFoundPlot))
		{
			getGroup()->pushMission(MISSION_FOUND, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_foundRange(int iRange, bool bFollow)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestFoundPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = AI_searchRange(iRange);

	iBestValue = 0;
	pBestPlot = NULL;
	pBestFoundPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && (pLoopPlot != plot() || GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopPlot, 1) <= pLoopPlot->plotCount(PUF_canDefend, -1, -1, getOwnerINLINE())))
				{
					if (canFound(pLoopPlot))
					{
						iValue = pLoopPlot->getFoundValue(getOwnerINLINE());

						if (iValue > iBestValue)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup(), 3) == 0)
								{
									if (generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
									{
										if (iPathTurns <= iRange)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											pBestFoundPlot = pLoopPlot;
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

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		if (atPlot(pBestFoundPlot))
		{
			getGroup()->pushMission(MISSION_FOUND, -1, -1, 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
		else if (!bFollow)
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_assaultSeaTransport(bool bBarbarian)
{
	PROFILE_FUNC();

	bool bIsAttackCity = (getUnitAICargo(UNITAI_ATTACK_CITY) > 0);

	FAssert(getGroup()->hasCargo());
	//FAssert(bIsAttackCity || getGroup()->getUnitAICargo(UNITAI_ATTACK) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	std::vector<CvUnit*> aGroupCargo;
	CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);
		CvUnit* pTransport = pLoopUnit->getTransportUnit();
		if (pTransport != NULL && pTransport->getGroup() == getGroup())
		{
			aGroupCargo.push_back(pLoopUnit);
		}
	}

	int iCargo = getGroup()->getCargo();
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	CvPlot* pBestAssaultPlot = NULL;

	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			if (pLoopPlot->isOwned())
			{
				if (((bBarbarian || !pLoopPlot->isBarbarian())) || GET_PLAYER(getOwnerINLINE()).isMinorCiv())
				{
					if (isPotentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
					{
						int iTargetCities = pLoopPlot->area()->getCitiesPerPlayer(pLoopPlot->getOwnerINLINE());
						if (iTargetCities > 0)
						{
							bool bCanCargoAllUnload = true;
							int iVisibleEnemyDefenders = pLoopPlot->getNumVisibleEnemyDefenders(this);
							if (iVisibleEnemyDefenders > 0)
							{
								for (uint i = 0; i < aGroupCargo.size(); ++i)
								{
									CvUnit* pAttacker = aGroupCargo[i];
									CvUnit* pDefender = pLoopPlot->getBestDefender(NO_PLAYER, pAttacker->getOwnerINLINE(), pAttacker, true);
									if (pDefender == NULL || !pAttacker->canAttack(*pDefender))
									{
										bCanCargoAllUnload = false;
										break;
									}
								}
							}

							if (bCanCargoAllUnload)
							{
								int iPathTurns;
								if (generatePath(pLoopPlot, 0, true, &iPathTurns))
								{
									int iValue = 1;

									if (!bIsAttackCity)
									{
										iValue += (AI_pillageValue(pLoopPlot, 15) * 10);
									}

									int iAssaultsHere = GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ASSAULT, getGroup());

									iValue += (iAssaultsHere * 100);

									CvCity* pCity = pLoopPlot->getPlotCity();

									if (pCity == NULL)
									{
										for (int iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
										{
											CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iJ));

											if (pAdjacentPlot != NULL)
											{
												pCity = pAdjacentPlot->getPlotCity();

												if (pCity != NULL)
												{
													if (pCity->getOwnerINLINE() == pLoopPlot->getOwnerINLINE())
													{
														break;
													}
													else
													{
														pCity = NULL;
													}
												}
											}
										}
									}

									if (pCity != NULL)
									{
										FAssert(isPotentialEnemy(pCity->getTeam(), pLoopPlot));

										if (!(pLoopPlot->isRiverCrossing(directionXY(pLoopPlot, pCity->plot()))))
										{
											iValue += (50 * -(GC.getRIVER_ATTACK_MODIFIER()));
										}

										iValue += 15 * (pLoopPlot->defenseModifier(getTeam(), false));
										iValue += 1000;
										iValue += (GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pCity->plot()) * 200);

										if (iPathTurns == 1)
										{
											iValue += GC.getGameINLINE().getSorenRandNum(50, "AI Assault");
										}
									}

									FAssert(iPathTurns > 0);

									if (iPathTurns == 1)
									{
										if (pCity != NULL)
										{
											if (pCity->area()->getNumCities() > 1)
											{
												iValue *= 2;
											}
										}
									}

									iValue *= 1000;

									if (iTargetCities <= iAssaultsHere)
									{
										iValue /= 2;
									}

									if (iTargetCities == 1)
									{
										if (iCargo > 7)
										{
											iValue *= 3;
											iValue /= iCargo - 4;
										}
									}

									if (pLoopPlot->isCity())
									{
										if (iVisibleEnemyDefenders * 3 > iCargo)
										{
											iValue /= 10;
										}
										else
										{
											iValue *= iCargo;
											iValue /= std::max(1, (iVisibleEnemyDefenders * 3));
										}
									}
									else
									{
										if (0 == iVisibleEnemyDefenders)
										{
											iValue *= 4;
											iValue /= 3;
										}
										else
										{
											iValue /= iVisibleEnemyDefenders;
										}
									}

									// if more than 3 turns to get there, then put some randomness into our preference of distance
									// +/- 33%
									if (iPathTurns > 3)
									{
										int iPathAdjustment = GC.getGameINLINE().getSorenRandNum(67, "AI Assault Target");

										iPathTurns *= 66 + iPathAdjustment;
										iPathTurns /= 100;
									}

									iValue /= (iPathTurns + 1);

									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = getPathEndTurnPlot();
										pBestAssaultPlot = pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestAssaultPlot != NULL))
	{
		FAssert(!(pBestPlot->isImpassable()));

		if ((pBestPlot == pBestAssaultPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestAssaultPlot->getX_INLINE(), pBestAssaultPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestAssaultPlot))
			{
				getGroup()->unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestAssaultPlot->getX_INLINE(), pBestAssaultPlot->getY_INLINE(), 0, false, false, MISSIONAI_ASSAULT, pBestAssaultPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ASSAULT, pBestAssaultPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_settlerSeaTransport()
{

	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestFoundPlot;
	CvArea* pWaterArea;
	bool bValid;
	int iValue;
	int iBestValue;
	int iI;

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_SETTLE) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	//New logic should allow some new tricks like
	//unloading settlers when a better site opens up locally
	//and delivering settlers
	//to inland sites

	pWaterArea = plot()->waterArea();
	FAssertMsg(pWaterArea != NULL, "Ship out of water?");

	CvUnit* pSettlerUnit = NULL;
	pPlot = plot();
	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (pLoopUnit->AI_getUnitAIType() == UNITAI_SETTLE)
			{
				pSettlerUnit = pLoopUnit;
				break;
			}
		}
	}

	FAssert(pSettlerUnit != NULL);

	int iAreaBestFoundValue = 0;
	CvPlot* pAreaBestPlot = NULL;

	int iOtherAreaBestFoundValue = 0;
	CvPlot* pOtherAreaBestPlot = NULL;

	for (iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
	{
		CvPlot* pCitySitePlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);
		if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pCitySitePlot, MISSIONAI_FOUND, getGroup()) == 0)
		{
			iValue = pCitySitePlot->getFoundValue(getOwnerINLINE());
/*************************************************************************************************/
/** BETTER_BTS_AI_MOD merged Sephi         01/13/09                                jdog5000      */
/**                                                                                              */
/** Settler AI                                                                                   */
/*************************************************************************************************/
/* original bts code
			if (pCitySitePlot->getArea() == getArea())
			{
				if (iValue > iAreaBestFoundValue)
				{
*/
			// Only count city sites we can get to
			if (pCitySitePlot->getArea() == getArea() && pSettlerUnit->generatePath(pCitySitePlot, MOVE_SAFE_TERRITORY, true))
			{
				if (iValue > iAreaBestFoundValue)
				{
/*************************************************************************************************/
/** BETTER_BTS_AI_MOD                       END                                                  */
/*************************************************************************************************/
					iAreaBestFoundValue = iValue;
					pAreaBestPlot = pCitySitePlot;
				}
			}
			else
			{
				if (iValue > iOtherAreaBestFoundValue)
				{
					iOtherAreaBestFoundValue = iValue;
					pOtherAreaBestPlot = pCitySitePlot;
				}
			}
		}
	}
	if ((0 == iAreaBestFoundValue) && (0 == iOtherAreaBestFoundValue))
	{
		return false;
	}

	if (iAreaBestFoundValue > iOtherAreaBestFoundValue)
	{
		//let the settler walk.
		getGroup()->unloadAll();
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestFoundPlot = NULL;

	for (iI = 0; iI < GET_PLAYER(getOwnerINLINE()).AI_getNumCitySites(); iI++)
	{
		CvPlot* pCitySitePlot = GET_PLAYER(getOwnerINLINE()).AI_getCitySite(iI);
		if (!(pCitySitePlot->isVisibleEnemyUnit(this)))
		{
			if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pCitySitePlot, MISSIONAI_FOUND, getGroup(), 4) == 0)
			{
				int iPathTurns;
				if (generatePath(pCitySitePlot, 0, true, &iPathTurns))
				{
					iValue = pCitySitePlot->getFoundValue(getOwnerINLINE());
					iValue *= 1000;
					iValue /= (2 + iPathTurns);

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = getPathEndTurnPlot();
						pBestFoundPlot = pCitySitePlot;
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		FAssert(!(pBestPlot->isImpassable()));

		if ((pBestPlot == pBestFoundPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestFoundPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
	}

	//Try original logic
	//(sometimes new logic breaks)
	pPlot = plot();

	iBestValue = 0;
	pBestPlot = NULL;
	pBestFoundPlot = NULL;

	int iMinFoundValue = GET_PLAYER(getOwnerINLINE()).AI_getMinFoundValue();

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			iValue = pLoopPlot->getFoundValue(getOwnerINLINE());

			if ((iValue > iBestValue) && (iValue >= iMinFoundValue))
			{
				bValid = false;

				pUnitNode = pPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->getTransportUnit() == this)
					{
						if (pLoopUnit->canFound(pLoopPlot))
						{
							bValid = true;
							break;
						}
					}
				}

				if (bValid)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_FOUND, getGroup(), 4) == 0)
						{
							if (generatePath(pLoopPlot, 0, true))
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestFoundPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestFoundPlot != NULL))
	{
		FAssert(!(pBestPlot->isImpassable()));

		if ((pBestPlot == pBestFoundPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestFoundPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestFoundPlot->getX_INLINE(), pBestFoundPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestFoundPlot);
			return true;
		}
	}
	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_settlerSeaFerry()
{
	PROFILE_FUNC();

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_WORKER) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	CvArea* pWaterArea = plot()->waterArea();
	FAssertMsg(pWaterArea != NULL, "Ship out of water?");

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	CvCity* pLoopCity;
	int iLoop;
	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		int iValue = pLoopCity->AI_getWorkersNeeded();
		if (iValue > 0)
		{
			iValue -= GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_FOUND, getGroup());
			if (iValue > 0)
			{
				int iPathTurns;
				if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
				{
					iValue += std::max(0, (GET_PLAYER(getOwnerINLINE()).AI_neededWorkers(pLoopCity->area()) - GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(pLoopCity->area(), UNITAI_WORKER)));
					iValue *= 1000;
					iValue /= 4 + iPathTurns;
					if (atPlot(pLoopCity->plot()))
					{
						iValue += 100;
					}
					else
					{
						iValue += GC.getGame().getSorenRandNum(100, "AI settler sea ferry");
					}
					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopCity->plot();
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pBestPlot);
			return true;
		}
	}
	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_specialSeaTransportMissionary()
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvUnit* pMissionaryUnit;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestSpreadPlot;
	int iPathTurns;
	int iValue;
	int iCorpValue;
	int iBestValue;
	int iI, iJ;
	bool bExecutive = false;

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_MISSIONARY) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	pPlot = plot();

	pMissionaryUnit = NULL;

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (pLoopUnit->AI_getUnitAIType() == UNITAI_MISSIONARY)
			{
				pMissionaryUnit = pLoopUnit;
				break;
			}
		}
	}

	if (pMissionaryUnit == NULL)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestSpreadPlot = NULL;

	// XXX what about non-coastal cities?
	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			pCity = pLoopPlot->getPlotCity();

			if (pCity != NULL)
			{
				iValue = 0;
				iCorpValue = 0;

				for (iJ = 0; iJ < GC.getNumReligionInfos(); iJ++)
				{
					if (pMissionaryUnit->canSpread(pLoopPlot, ((ReligionTypes)iJ)))
					{
						if (GET_PLAYER(getOwnerINLINE()).getStateReligion() == ((ReligionTypes)iJ))
						{
							iValue += 3;
						}

						if (GET_PLAYER(getOwnerINLINE()).hasHolyCity((ReligionTypes)iJ))
						{
							iValue++;
						}
					}
				}

				for (iJ = 0; iJ < GC.getNumCorporationInfos(); iJ++)
				{
					if (pMissionaryUnit->canSpreadCorporation(pLoopPlot, ((CorporationTypes)iJ)))
					{
						if (GET_PLAYER(getOwnerINLINE()).hasHeadquarters((CorporationTypes)iJ))
						{
							iCorpValue += 3;
						}
					}
				}

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_SPREAD, getGroup()) == 0)
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								iValue *= pCity->getPopulation();

								if (pCity->getOwnerINLINE() == getOwnerINLINE())
								{
									iValue *= 4;
								}
								else if (pCity->getTeam() == getTeam())
								{
									iValue *= 3;
								}

								if (pCity->getReligionCount() == 0)
								{
									iValue *= 2;
								}

								iValue /= (pCity->getReligionCount() + 1);

								FAssert(iPathTurns > 0);

								if (iPathTurns == 1)
								{
									iValue *= 2;
								}

								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestSpreadPlot = pLoopPlot;
									bExecutive = false;
								}
							}
						}
					}
				}

				if (iCorpValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_SPREAD_CORPORATION, getGroup()) == 0)
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								iCorpValue *= pCity->getPopulation();

								FAssert(iPathTurns > 0);

								if (iPathTurns == 1)
								{
									iValue *= 2;
								}

								iCorpValue *= 1000;

								iCorpValue /= (iPathTurns + 1);

								if (iCorpValue > iBestValue)
								{
									iBestValue = iCorpValue;
									pBestPlot = getPathEndTurnPlot();
									pBestSpreadPlot = pLoopPlot;
									bExecutive = true;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestSpreadPlot != NULL))
	{
		FAssert(!(pBestPlot->isImpassable()) || canMoveImpassable());

		if ((pBestPlot == pBestSpreadPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestSpreadPlot->getX_INLINE(), pBestSpreadPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestSpreadPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestSpreadPlot->getX_INLINE(), pBestSpreadPlot->getY_INLINE(), 0, false, false, bExecutive ? MISSIONAI_SPREAD_CORPORATION : MISSIONAI_SPREAD, pBestSpreadPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, bExecutive ? MISSIONAI_SPREAD_CORPORATION : MISSIONAI_SPREAD, pBestSpreadPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_specialSeaTransportSpy()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvPlot* pBestSpyPlot;
	PlayerTypes eBestPlayer;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI;

	FAssert(getCargo() > 0);
	FAssert(getUnitAICargo(UNITAI_SPY) > 0);

	if (!canCargoAllMove())
	{
		return false;
	}

	iBestValue = 0;
	eBestPlayer = NO_PLAYER;

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) <= ATTITUDE_ANNOYED)
				{
					iValue = GET_PLAYER((PlayerTypes)iI).getTotalPopulation();

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestPlayer = ((PlayerTypes)iI);
					}
				}
			}
		}
	}

	if (eBestPlayer == NO_PLAYER)
	{
		return false;
	}

	pBestPlot = NULL;
	pBestSpyPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCoastalLand())
		{
			if (pLoopPlot->getOwnerINLINE() == eBestPlayer)
			{
				iValue = pLoopPlot->area()->getCitiesPerPlayer(eBestPlayer);

				if (iValue > 0)
				{
					if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ATTACK_SPY, getGroup(), 4) == 0)
					{
						if (generatePath(pLoopPlot, 0, true, &iPathTurns))
						{
							iValue *= 1000;

							iValue /= (iPathTurns + 1);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = getPathEndTurnPlot();
								pBestSpyPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestSpyPlot != NULL))
	{
		FAssert(!(pBestPlot->isImpassable()));

		if ((pBestPlot == pBestSpyPlot) || (stepDistance(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), pBestSpyPlot->getX_INLINE(), pBestSpyPlot->getY_INLINE()) == 1))
		{
			if (atPlot(pBestSpyPlot))
			{
				unloadAll(); // XXX is this dangerous (not pushing a mission...) XXX air units?
				return true;
			}
			else
			{
				getGroup()->pushMission(MISSION_MOVE_TO, pBestSpyPlot->getX_INLINE(), pBestSpyPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestSpyPlot);
				return true;
			}
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_ATTACK_SPY, pBestSpyPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_carrierSeaTransport()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pLoopPlotAir;
	CvPlot* pBestPlot;
	CvPlot* pBestCarrierPlot;
	int iMaxAirRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;
	int iI;

	// XXX maybe protect land troops?

	iMaxAirRange = 0;

	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		iMaxAirRange = std::max(iMaxAirRange, aCargoUnits[i]->airRange());
	}

	if (iMaxAirRange == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestCarrierPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->isAdjacentToLand())
			{
				iValue = 0;

				for (iDX = -(iMaxAirRange); iDX <= iMaxAirRange; iDX++)
				{
					for (iDY = -(iMaxAirRange); iDY <= iMaxAirRange; iDY++)
					{
						pLoopPlotAir = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iDX, iDY);

						if (pLoopPlotAir != NULL)
						{
							if (plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pLoopPlotAir->getX_INLINE(), pLoopPlotAir->getY_INLINE()) <= iMaxAirRange)
							{
								if (!(pLoopPlotAir->isBarbarian()))
								{
									if (potentialWarAction(pLoopPlotAir))
									{
										if (pLoopPlotAir->isCity())
										{
											iValue++;
										}

										if (pLoopPlotAir->getImprovementType() != NO_IMPROVEMENT)
										{
											iValue ++;
										}
									}
								}
							}
						}
					}
				}

				if (iValue > 0)
				{
					if (!(pLoopPlot->isVisibleEnemyUnit(this)))
					{
						bool bStealth = (getInvisibleType() != NO_INVISIBLE);
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_CARRIER, getGroup(), bStealth ? 5 : 3) <= (bStealth ? 0 : 3))
						{
							if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								iValue *= 1000;

								for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
								{
									CvPlot* pDirectionPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes)iDirection);
									if (pDirectionPlot != NULL)
									{
										if (pDirectionPlot->isCity() && isEnemy(pDirectionPlot->getTeam(), pLoopPlot))
										{
											iValue /= 2;
											break;
										}
									}
								}

								iValue += (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ASSAULT, getGroup(), 2) * 2000);

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = getPathEndTurnPlot();
									pBestCarrierPlot = pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestCarrierPlot != NULL))
	{
		if (atPlot(pBestCarrierPlot))
		{
			if (getGroup()->hasCargo())
			{
				CvPlot* pPlot = plot();

				int iNumUnits = pPlot->getNumUnits();

				for (int i = 0; i < iNumUnits; ++i)
				{
					bool bDone = true;
				CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					CvUnit* pCargoUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pPlot->nextUnitNode(pUnitNode);

					if (pCargoUnit->isCargo())
					{
						FAssert(pCargoUnit->getTransportUnit() != NULL);
							if (pCargoUnit->getOwnerINLINE() == getOwnerINLINE() && (pCargoUnit->getTransportUnit()->getGroup() == getGroup()) && (pCargoUnit->getDomainType() == DOMAIN_AIR))
						{
								if (pCargoUnit->canMove() && pCargoUnit->isGroupHead())
								{
									// careful, this might kill the cargo group
									if (pCargoUnit->getGroup()->AI_update())
							{
										bDone = false;
										break;
							}
						}
					}
							}
						}

					if (bDone)
					{
						break;
					}
				}
			}

			if (canPlunder(pBestCarrierPlot))
			{
			getGroup()->pushMission(MISSION_PLUNDER, -1, -1, 0, false, false, MISSIONAI_CARRIER, pBestCarrierPlot);
			}
			else
			{
				getGroup()->pushMission(MISSION_SKIP);
			}
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_CARRIER, pBestCarrierPlot);
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_connectPlot(CvPlot* pPlot, int iRange)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	int iLoop;

	FAssert(canBuildRoute());

	if (!(pPlot->isVisibleEnemyUnit(this)))
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pPlot, MISSIONAI_BUILD, getGroup(), iRange) == 0)
		{
			if (generatePath(pPlot, MOVE_SAFE_TERRITORY, true))
			{
				for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
				{
					if (!(pPlot->isConnectedTo(pLoopCity)))
					{
						FAssertMsg(pPlot->getPlotCity() != pLoopCity, "pPlot->getPlotCity() is not expected to be equal with pLoopCity");

						if (plot()->getPlotGroup(getOwnerINLINE()) == pLoopCity->plot()->getPlotGroup(getOwnerINLINE()))
						{
							getGroup()->pushMission(MISSION_ROUTE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pPlot);
							return true;
						}
					}
				}

				for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
				{
					if (!(pPlot->isConnectedTo(pLoopCity)))
					{
						FAssertMsg(pPlot->getPlotCity() != pLoopCity, "pPlot->getPlotCity() is not expected to be equal with pLoopCity");

						if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
						{
							if (generatePath(pLoopCity->plot(), MOVE_SAFE_TERRITORY, true))
							{
								if (atPlot(pPlot)) // need to test before moving...
								{
									getGroup()->pushMission(MISSION_ROUTE_TO, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pPlot);
								}
								else
								{
									getGroup()->pushMission(MISSION_ROUTE_TO, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pPlot);
									getGroup()->pushMission(MISSION_ROUTE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pPlot);
								}

								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_improveCity(CvCity* pCity)
{
	PROFILE_FUNC();

	CvPlot* pBestPlot;
	BuildTypes eBestBuild;
	MissionTypes eMission;

	if (AI_bestCityBuild(pCity, &pBestPlot, &eBestBuild, NULL, this))
	{
		FAssertMsg(pBestPlot != NULL, "BestPlot is not assigned a valid value");
		FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
		FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");
		if ((plot()->getWorkingCity() != pCity) || (GC.getBuildInfo(eBestBuild).getRoute() != NO_ROUTE))
		{
			eMission = MISSION_ROUTE_TO;
		}
		else
		{
			eMission = MISSION_MOVE_TO;
			if (NULL != pBestPlot && generatePath(pBestPlot) && (getPathLastNode()->m_iData2 == 1) && (getPathLastNode()->m_iData1 == 0))
			{
				if (pBestPlot->getRouteType() != NO_ROUTE)
				{
					eMission = MISSION_ROUTE_TO;
				}
			}
			else if (plot()->getRouteType() == NO_ROUTE)
			{
				int iPlotMoveCost = 0;
				iPlotMoveCost = ((plot()->getFeatureType() == NO_FEATURE) ? GC.getTerrainInfo(plot()->getTerrainType()).getMovementCost() : GC.getFeatureInfo(plot()->getFeatureType()).getMovementCost());

				if (plot()->isHills())
				{
					iPlotMoveCost += GC.getHILLS_EXTRA_MOVEMENT();
				}
				if (iPlotMoveCost > 1)
				{
					eMission = MISSION_ROUTE_TO;
				}
			}
		}

		eBestBuild = AI_betterPlotBuild(pBestPlot, eBestBuild);

		getGroup()->pushMission(eMission, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);

		return true;
	}

	return false;
}

bool CvUnitAI::AI_improveLocalPlot(int iRange, CvCity* pIgnoreCity)
{

	int iX, iY;

	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;
	BuildTypes eBestBuild = NO_BUILD;

	for (iX = -iRange; iX <= iRange; iX++)
	{
		for (iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if ((pLoopPlot != NULL) && (pLoopPlot->isCityRadius()))
			{
				CvCity* pCity = pLoopPlot->getWorkingCity();
				if ((NULL != pCity) && (pCity->getOwnerINLINE() == getOwnerINLINE()))
				{
					if ((NULL == pIgnoreCity) || (pCity != pIgnoreCity))
					{
						if (AI_plotValid(pLoopPlot))
						{
							int iIndex = pCity->getCityPlotIndex(pLoopPlot);
							if (iIndex != CITY_HOME_PLOT)
							{
								if (((NULL == pIgnoreCity) || ((pCity->AI_getWorkersNeeded() > 0) && (pCity->AI_getWorkersHave() < (1 + pCity->AI_getWorkersNeeded() * 2 / 3)))) && (pCity->AI_getBestBuild(iIndex) != NO_BUILD))
								{
									if (canBuild(pLoopPlot, pCity->AI_getBestBuild(iIndex)))
									{
										bool bAllowed = true;

										if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
										{
											if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT && pLoopPlot->getImprovementType() != GC.getDefineINT("RUINS_IMPROVEMENT"))
											{
												bAllowed = false;
											}
										}

										if (bAllowed)
										{
											if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT && GC.getBuildInfo(pCity->AI_getBestBuild(iIndex)).getImprovement() != NO_IMPROVEMENT)
											{
												bAllowed = false;
											}
										}

										if (bAllowed)
										{
											int iValue = pCity->AI_getBestBuildValue(iIndex);
											int iPathTurns;
											if (generatePath(pLoopPlot, 0, true, &iPathTurns))
											{
												int iMaxWorkers = 1;
												if (plot() == pLoopPlot)
												{
													iValue *= 3;
													iValue /= 2;
												}
												else if (getPathLastNode()->m_iData1 == 0)
												{
													iPathTurns++;
												}
												else if (iPathTurns <= 1)
												{
													iMaxWorkers = AI_calculatePlotWorkersNeeded(pLoopPlot, pCity->AI_getBestBuild(iIndex));
												}

												if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup()) < iMaxWorkers)
												{
													iValue *= 1000;
													iValue /= 1 + iPathTurns;

													if (iValue > iBestValue)
													{
														iBestValue = iValue;
														pBestPlot = pLoopPlot;
														eBestBuild = pCity->AI_getBestBuild(iIndex);
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
			}
		}
	}

	if (pBestPlot != NULL)
	{
	    FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
	    FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");

		FAssert(pBestPlot->getWorkingCity() != NULL);
		if (NULL != pBestPlot->getWorkingCity())
		{
			pBestPlot->getWorkingCity()->AI_changeWorkersHave(+1);

			if (plot()->getWorkingCity() != NULL)
			{
				plot()->getWorkingCity()->AI_changeWorkersHave(-1);
			}
		}
		MissionTypes eMission = MISSION_MOVE_TO;

		int iPathTurns;
		if (generatePath(pBestPlot, 0, true, &iPathTurns) && (getPathLastNode()->m_iData2 == 1) && (getPathLastNode()->m_iData1 == 0))
		{
			if (pBestPlot->getRouteType() != NO_ROUTE)
			{
				eMission = MISSION_ROUTE_TO;
			}
		}
		else if (plot()->getRouteType() == NO_ROUTE)
		{
			int iPlotMoveCost = 0;
			iPlotMoveCost = ((plot()->getFeatureType() == NO_FEATURE) ? GC.getTerrainInfo(plot()->getTerrainType()).getMovementCost() : GC.getFeatureInfo(plot()->getFeatureType()).getMovementCost());

			if (plot()->isHills())
			{
				iPlotMoveCost += GC.getHILLS_EXTRA_MOVEMENT();
			}
			if (iPlotMoveCost > 1)
			{
				eMission = MISSION_ROUTE_TO;
			}
		}

		eBestBuild = AI_betterPlotBuild(pBestPlot, eBestBuild);

		getGroup()->pushMission(eMission, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);
		return true;
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_nextCityToImprove(CvCity* pCity)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvPlot* pPlot;
	CvPlot* pBestPlot;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	eBestBuild = NO_BUILD;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity != pCity)
		{
			//iValue = pLoopCity->AI_totalBestBuildValue(area());
			int iWorkersNeeded = pLoopCity->AI_getWorkersNeeded();
			int iWorkersHave = pLoopCity->AI_getWorkersHave();

			iValue = std::max(0, iWorkersNeeded - iWorkersHave) * 100;
			iValue += iWorkersNeeded * 10;
			iValue *= (iWorkersNeeded + 1);
			iValue /= (iWorkersHave + 1);

			if (iValue > 0)
			{
				if (AI_bestCityBuild(pLoopCity, &pPlot, &eBuild, NULL, this))
				{
					FAssert(pPlot != NULL);
					FAssert(eBuild != NO_BUILD);

					iValue *= 1000;

					if (pLoopCity->isCapital())
					{
					    iValue *= 2;
					}

					generatePath(pPlot, 0, true, &iPathTurns);
					iValue /= (iPathTurns + 1);

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						eBestBuild = eBuild;
						pBestPlot = pPlot;
						FAssert(!atPlot(pBestPlot) || NULL == pCity || pCity->AI_getWorkersNeeded() == 0 || pCity->AI_getWorkersHave() > pCity->AI_getWorkersNeeded() + 1);
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
	    FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
	    FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");
	    if (plot()->getWorkingCity() != NULL)
	    {
			plot()->getWorkingCity()->AI_changeWorkersHave(-1);
	    }

		FAssert(pBestPlot->getWorkingCity() != NULL || GC.getBuildInfo(eBestBuild).getImprovement() == NO_IMPROVEMENT);
		if (NULL != pBestPlot->getWorkingCity())
		{
			pBestPlot->getWorkingCity()->AI_changeWorkersHave(+1);
		}

		eBestBuild = AI_betterPlotBuild(pBestPlot, eBestBuild);

		getGroup()->pushMission(MISSION_ROUTE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_nextCityToImproveAirlift()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getGroup()->getNumUnits() > 1)
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getMaxAirlift() == 0)
	{
		return false;
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (pLoopCity != pCity)
		{
			if (canAirliftAt(pCity->plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
			{
				iValue = pLoopCity->AI_totalBestBuildValue(pLoopCity->area());

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestPlot = pLoopCity->plot();
					FAssert(pLoopCity != pCity);
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRLIFT, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_irrigateTerritory()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	ImprovementTypes eImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eNonObsoleteBonus;
	bool bValid;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iI, iJ;

	iBestValue = 0;
	eBestBuild = NO_BUILD;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				if (pLoopPlot->getWorkingCity() == NULL)
				{
					eImprovement = pLoopPlot->getImprovementType();

					if ((eImprovement == NO_IMPROVEMENT) || !(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION) && !(eImprovement == (GC.getDefineINT("RUINS_IMPROVEMENT")))))
					{
						if ((eImprovement == NO_IMPROVEMENT) || !(GC.getImprovementInfo(eImprovement).isCarriesIrrigation()))
						{
							eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

							if ((eImprovement == NO_IMPROVEMENT) || (eNonObsoleteBonus == NO_BONUS) || !(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
							{
								if (pLoopPlot->isIrrigationAvailable(true))
								{
									iBestTempBuildValue = MAX_INT;
									eBestTempBuild = NO_BUILD;

									for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
									{
										eBuild = ((BuildTypes)iJ);
										FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

										if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
										{
											if (GC.getImprovementInfo((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement())).isCarriesIrrigation())
											{
												if (canBuild(pLoopPlot, eBuild))
												{
													iValue = 10000;

													iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

													// XXX feature production???

													if (iValue < iBestTempBuildValue)
													{
														iBestTempBuildValue = iValue;
														eBestTempBuild = eBuild;
													}
												}
											}
										}
									}

									if (eBestTempBuild != NO_BUILD)
									{
										bValid = true;

										if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
										{
											if (pLoopPlot->getFeatureType() != NO_FEATURE)
											{
												if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pLoopPlot->getFeatureType())

//FfH: Added by Kael 04/24/2008
                                                  && !GC.getCivilizationInfo(getCivilizationType()).isMaintainFeatures(pLoopPlot->getFeatureType())
//FfH: End Add

												)
												{
													if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
													{
														bValid = false;
													}
												}
											}
										}

										if (bValid)
										{
											if (!(pLoopPlot->isVisibleEnemyUnit(this)))
											{
												if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup(), 1) == 0)
												{
													if (generatePath(pLoopPlot, 0, true, &iPathTurns)) // XXX should this actually be at the top of the loop? (with saved paths and all...)
													{
														iValue = 10000;

														iValue /= (iPathTurns + 1);

														if (iValue > iBestValue)
														{
															iBestValue = iValue;
															eBestBuild = eBestTempBuild;
															pBestPlot = pLoopPlot;
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
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
		FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");

		getGroup()->pushMission(MISSION_ROUTE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);

		return true;
	}

	return false;
}

bool CvUnitAI::AI_fortTerritory(bool bCanal, bool bAirbase)
{
	int iBestValue = 0;
	BuildTypes eBestBuild = NO_BUILD;
	CvPlot* pBestPlot = NULL;

	CvPlayerAI& kOwner = GET_PLAYER(getOwnerINLINE());
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
				{
					int iValue = 0;
					iValue += bCanal ? kOwner.AI_getPlotCanalValue(pLoopPlot) : 0;
					iValue += bAirbase ? kOwner.AI_getPlotAirbaseValue(pLoopPlot) : 0;

					if (iValue > 0)
					{
						int iBestTempBuildValue = MAX_INT;
						BuildTypes eBestTempBuild = NO_BUILD;

						for (int iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
						{
							BuildTypes eBuild = ((BuildTypes)iJ);
							FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

							if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
							{
								if (GC.getImprovementInfo((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement())).isActsAsCity())
								{
								    if (GC.getImprovementInfo((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement())).getDefenseModifier() > 0)
								    {
                                        if (canBuild(pLoopPlot, eBuild))
                                        {
                                            iValue = 10000;

                                            iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

                                            if (iValue < iBestTempBuildValue)
                                            {
                                                iBestTempBuildValue = iValue;
                                                eBestTempBuild = eBuild;
                                            }
                                        }
                                    }
								}
							}
						}

						if (eBestTempBuild != NO_BUILD)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								bool bValid = true;

								if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS))
								{
									if (pLoopPlot->getFeatureType() != NO_FEATURE)
									{
										if (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pLoopPlot->getFeatureType())

//FfH: Added by Kael 04/24/2008
                                          && !GC.getCivilizationInfo(getCivilizationType()).isMaintainFeatures(pLoopPlot->getFeatureType())
//FfH: End Add

										)
										{
											if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).getYieldChange(YIELD_PRODUCTION) > 0)
											{
												bValid = false;
											}
										}
									}
								}

								if (bValid)
								{
									if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup(), 3) == 0)
									{
										int iPathTurns;
										if (generatePath(pLoopPlot, 0, true, &iPathTurns))
										{
											iValue *= 1000;
											iValue /= (iPathTurns + 1);

											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestBuild = eBestTempBuild;
												pBestPlot = pLoopPlot;
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
	}

	if (pBestPlot != NULL)
	{
		FAssertMsg(eBestBuild != NO_BUILD, "BestBuild is not assigned a valid value");
		FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");

		getGroup()->pushMission(MISSION_ROUTE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
		getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);

		return true;
	}
	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_improveBonus(int iMinValue, CvPlot** ppBestPlot, BuildTypes* peBestBuild, int* piBestValue)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	ImprovementTypes eImprovement;
	BuildTypes eBuild;
	BuildTypes eBestBuild;
	BuildTypes eBestTempBuild;
	BonusTypes eNonObsoleteBonus;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iBestTempBuildValue;
	int iBestResourceValue;
	int iI, iJ;
	bool bBestBuildIsRoute = false;

	bool bCanRoute;
	bool bIsConnected;

	iBestValue = 0;
	iBestResourceValue = 0;
	eBestBuild = NO_BUILD;
	pBestPlot = NULL;

	bCanRoute = canBuildRoute();

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE() && AI_plotValid(pLoopPlot))
		{
			bool bCanImprove = (pLoopPlot->area() == area());
			if (!bCanImprove)
			{
				if (DOMAIN_SEA == getDomainType() && pLoopPlot->isWater() && plot()->isAdjacentToArea(pLoopPlot->area()))
				{
					bCanImprove = true;
				}
			}

//FfH: Added by Kael 12/20/2008
            if (pLoopPlot->isVisibleEnemyUnit(this))
            {
                bCanImprove = false;
            }
            if (!atPlot(pLoopPlot))
            {
                if (!canMoveInto(pLoopPlot))
                {
                    bCanImprove = false;
                }
            }
//FfH: End Add

			if (bCanImprove)
			{
				eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

				if (eNonObsoleteBonus != NO_BONUS)
				{
				    bIsConnected = pLoopPlot->isConnectedToCapital(getOwnerINLINE());
                    if ((pLoopPlot->getWorkingCity() != NULL) || (bIsConnected || bCanRoute))
                    {
                        eImprovement = pLoopPlot->getImprovementType();

                        bool bDoImprove = false;

                        if (eImprovement == NO_IMPROVEMENT)
                        {
                            bDoImprove = true;
                        }
                        else if (GC.getImprovementInfo(eImprovement).isActsAsCity() || GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
                        {
                        	bDoImprove = false;
                        }
                        else if (eImprovement == (ImprovementTypes)(GC.getDefineINT("RUINS_IMPROVEMENT")))
                        {
                            bDoImprove = true;
                        }
                        else if (!GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
                        {
                        	bDoImprove = true;
                        }

                        iBestTempBuildValue = MAX_INT;
                        eBestTempBuild = NO_BUILD;

                        if (bDoImprove)
                        {
                            for (iJ = 0; iJ < GC.getNumBuildInfos(); iJ++)
                            {
                                eBuild = ((BuildTypes)iJ);

                                if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
                                {
                                    if (GC.getImprovementInfo((ImprovementTypes) GC.getBuildInfo(eBuild).getImprovement()).isImprovementBonusTrade(eNonObsoleteBonus) || (!pLoopPlot->isCityRadius() && GC.getImprovementInfo((ImprovementTypes) GC.getBuildInfo(eBuild).getImprovement()).isActsAsCity()))
                                    {
                                        if (canBuild(pLoopPlot, eBuild))
                                        {
                                        	if ((pLoopPlot->getFeatureType() == NO_FEATURE) || !GC.getBuildInfo(eBuild).isFeatureRemove(pLoopPlot->getFeatureType()) || !GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_LEAVE_FORESTS)

//FfH: Added by Kael 04/24/2008
                                              || GC.getCivilizationInfo(getCivilizationType()).isMaintainFeatures(pLoopPlot->getFeatureType())
//FfH: End Add

                                        	)
                                        	{
												iValue = 10000;

												iValue /= (GC.getBuildInfo(eBuild).getTime() + 1);

/*FfH: Added by Chalid AiManaAndBonus 06/10/2006*/
                                                if (!isHuman())
                                                {
                                                    iValue /= 100;
                                                    iValue *= std::max(0, (100-GC.getLeaderHeadInfo(GET_PLAYER(getOwnerINLINE()).getPersonalityType()).getImprovementWeightModifier((ImprovementTypes) GC.getBuildInfo(eBuild).getImprovement())));
                                                }
                                                iValue -= GC.getGameINLINE().getSorenRandNum(4000, "AIBonus");
//FfH: End Add

												// XXX feature production???

												if (iValue < iBestTempBuildValue)
												{
													iBestTempBuildValue = iValue;
													eBestTempBuild = eBuild;
												}
                                        	}
                                        }
                                    }
                                }
                            }
                        }
                        if (eBestTempBuild == NO_BUILD)
                        {
                        	bDoImprove = false;
                        }

                        if ((eBestTempBuild != NO_BUILD) || (bCanRoute && !bIsConnected))
                        {
                        	if (generatePath(pLoopPlot, 0, true, &iPathTurns))
							{
								iValue = GET_PLAYER(getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus);

								if (bDoImprove)
								{
									eImprovement = (ImprovementTypes)GC.getBuildInfo(eBestTempBuild).getImprovement();
									FAssert(eImprovement != NO_IMPROVEMENT);
									//iValue += (GC.getImprovementInfo((ImprovementTypes) GC.getBuildInfo(eBestTempBuild).getImprovement()))
									iValue += 5 * pLoopPlot->calculateImprovementYieldChange(eImprovement, YIELD_FOOD, getOwner(), false);
									iValue += 5 * pLoopPlot->calculateNatureYield(YIELD_FOOD, getTeam(), (pLoopPlot->getFeatureType() == NO_FEATURE) ? true : (GC.getBuildInfo(eBestTempBuild).isFeatureRemove(pLoopPlot->getFeatureType())

//FfH: Added by Kael 04/24/2008
                                      && !GC.getCivilizationInfo(getCivilizationType()).isMaintainFeatures(pLoopPlot->getFeatureType()))
//FfH: End Add

									);
								}

								iValue += std::max(0, 100 * GC.getBonusInfo(eNonObsoleteBonus).getAIObjective());

								if (GET_PLAYER(getOwnerINLINE()).getNumTradeableBonuses(eNonObsoleteBonus) == 0)
								{
									iValue *= 2;
								}

								int iMaxWorkers = 1;
								if ((eBestTempBuild != NO_BUILD) && (!GC.getBuildInfo(eBestTempBuild).isKill()))
								{
									//allow teaming.
									iMaxWorkers = AI_calculatePlotWorkersNeeded(pLoopPlot, eBestTempBuild);
									if (getPathLastNode()->m_iData1 == 0)
									{
										iMaxWorkers = std::min((iMaxWorkers + 1) / 2, 1 + GET_PLAYER(getOwnerINLINE()).AI_baseBonusVal(eNonObsoleteBonus) / 20);
									}
								}

								if ((GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup()) < iMaxWorkers)
									&& (!bDoImprove || (pLoopPlot->getBuildTurnsLeft(eBestTempBuild, 0, 0) > (iPathTurns * 2 - 1))))
								{
									if (bDoImprove)
									{
										iValue *= 1000;

										//added Sephi, for Sea workers, all that matter is Distance
										if (DOMAIN_SEA == getDomainType() && pLoopPlot->isWater())
										{
											iValue=10000;
										}
										//end added Sephi

										if (atPlot(pLoopPlot))
										{
											iValue *= 3;
										}

										iValue /= (iPathTurns + 1);

										if (pLoopPlot->isCityRadius())
										{
											iValue *= 2;
										}

										if (iValue > iBestValue)
										{
											iBestValue = iValue;
											eBestBuild = eBestTempBuild;
											pBestPlot = pLoopPlot;
											bBestBuildIsRoute = false;
											iBestResourceValue = iValue;
										}
									}
									else
									{
										FAssert(bCanRoute && !bIsConnected);
										eImprovement = pLoopPlot->getImprovementType();
										if ((eImprovement != NO_IMPROVEMENT) && (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus)))
										{
											iValue *= 1000;
											iValue /= (iPathTurns + 1);

											if (iValue > iBestValue)
											{
												iBestValue = iValue;
												eBestBuild = NO_BUILD;
												pBestPlot = pLoopPlot;
												bBestBuildIsRoute = true;
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
	}

	if ((iBestValue < iMinValue) && (NULL != ppBestPlot))
	{
		FAssert(NULL != peBestBuild);
		FAssert(NULL != piBestValue);

		*ppBestPlot = pBestPlot;
		*peBestBuild = eBestBuild;
		*piBestValue = iBestResourceValue;
	}

	if (pBestPlot != NULL)
	{
		if (eBestBuild != NO_BUILD)
		{
			FAssertMsg(!bBestBuildIsRoute, "BestBuild should not be a route");
			FAssertMsg(eBestBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");


			MissionTypes eBestMission = MISSION_MOVE_TO;

			if ((pBestPlot->getWorkingCity() == NULL) || !pBestPlot->getWorkingCity()->isConnectedToCapital())
			{
				eBestMission = MISSION_ROUTE_TO;
			}
			else
			{
				int iDistance = stepDistance(getX_INLINE(), getY_INLINE(), pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
				int iPathTurns;
				if (generatePath(pBestPlot, 0, false, &iPathTurns))
				{
					if (iPathTurns >= iDistance)
					{
						eBestMission = MISSION_ROUTE_TO;
					}
				}
			}

			eBestBuild = AI_betterPlotBuild(pBestPlot, eBestBuild);
			getGroup()->pushMission(eBestMission, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
			getGroup()->pushMission(MISSION_BUILD, eBestBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pBestPlot);

			return true;
		}
		else if (bBestBuildIsRoute)
		{
			if (AI_connectPlot(pBestPlot))
			{
				return true;
			}
			/*else
			{
				// the plot may be connected, but not connected to capital, if capital is not on same area, or if civ has no capital (like barbarians)
				FAssertMsg(false, "Expected that a route could be built to eBestPlot");
			}*/
		}
		else
		{
			FAssert(false);
		}
	}

	return false;
}

//returns true if a mission is pushed
//if eBuild is NO_BUILD, assumes a route is desired.
bool CvUnitAI::AI_improvePlot(CvPlot* pPlot, BuildTypes eBuild)
{
	FAssert(pPlot != NULL);

	if (eBuild != NO_BUILD)
	{
		FAssertMsg(eBuild < GC.getNumBuildInfos(), "BestBuild is assigned a corrupt value");

		eBuild = AI_betterPlotBuild(pPlot, eBuild);
		if (!atPlot(pPlot))
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pPlot->getX_INLINE(), pPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pPlot);
		}
		getGroup()->pushMission(MISSION_BUILD, eBuild, -1, 0, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pPlot);

		return true;
	}
	else if (canBuildRoute())
	{
		if (AI_connectPlot(pPlot))
		{
			return true;
		}
	}

	return false;

}

BuildTypes CvUnitAI::AI_betterPlotBuild(CvPlot* pPlot, BuildTypes eBuild)
{
	FAssert(pPlot != NULL);
	FAssert(eBuild != NO_BUILD);
	bool bBuildRoute = false;
	bool bClearFeature = false;

	FeatureTypes eFeature = pPlot->getFeatureType();

	CvBuildInfo& kOriginalBuildInfo = GC.getBuildInfo(eBuild);

	if (kOriginalBuildInfo.getRoute() != NO_ROUTE)
	{
		return eBuild;
	}

	int iWorkersNeeded = AI_calculatePlotWorkersNeeded(pPlot, eBuild);

	if ((pPlot->getBonusType() == NO_BONUS) && (pPlot->getWorkingCity() != NULL))
	{
		iWorkersNeeded = std::max(1, std::min(iWorkersNeeded, pPlot->getWorkingCity()->AI_getWorkersHave()));
	}

	if (eFeature != NO_FEATURE)
	{
		CvFeatureInfo& kFeatureInfo = GC.getFeatureInfo(eFeature);
		if (kOriginalBuildInfo.isFeatureRemove(eFeature)

//FfH: Added by Kael 04/24/2008
          && !GC.getCivilizationInfo(getCivilizationType()).isMaintainFeatures(pPlot->getFeatureType())
//FfH: End Add

		)
		{
			if ((kOriginalBuildInfo.getImprovement() == NO_IMPROVEMENT) || (!pPlot->isBeingWorked() || (kFeatureInfo.getYieldChange(YIELD_FOOD) + kFeatureInfo.getYieldChange(YIELD_PRODUCTION)) <= 0))
			{
				bClearFeature = true;
			}
		}

		if ((kFeatureInfo.getMovementCost() > 1) && (iWorkersNeeded > 1))
		{
			bBuildRoute = true;
		}
	}

	if (pPlot->getBonusType() != NO_BONUS)
	{
		bBuildRoute = true;
	}
	else if (pPlot->isHills())
	{
		if ((GC.getHILLS_EXTRA_MOVEMENT() > 0) && (iWorkersNeeded > 1))
		{
			bBuildRoute = true;
		}
	}

	if (pPlot->getRouteType() != NO_ROUTE)
	{
		bBuildRoute = false;
	}

	BuildTypes eBestBuild = NO_BUILD;
	int iBestValue = 0;
	for (int iBuild = 0; iBuild < GC.getNumBuildInfos(); iBuild++)
	{
		BuildTypes eBuild = ((BuildTypes)iBuild);
		CvBuildInfo& kBuildInfo = GC.getBuildInfo(eBuild);


		RouteTypes eRoute = (RouteTypes)kBuildInfo.getRoute();

		if ((bBuildRoute && (eRoute != NO_ROUTE)) || (bClearFeature && kBuildInfo.isFeatureRemove(eFeature)

//FfH: Added by Kael 04/24/2008
          && !GC.getCivilizationInfo(getCivilizationType()).isMaintainFeatures(pPlot->getFeatureType())
//FfH: End Add

		))
		{
			if (canBuild(pPlot, eBuild))
			{
				int iValue = 10000;
				if (bBuildRoute && (eRoute != NO_ROUTE))
				{
					iValue *= (1 + GC.getRouteInfo(eRoute).getValue());
					iValue /= 2;

					if (pPlot->getBonusType() != NO_BONUS)
					{
						iValue *= 2;
					}

					if (pPlot->getWorkingCity() != NULL)
					{
						iValue *= 2 + iWorkersNeeded + ((pPlot->isHills() && (iWorkersNeeded > 1)) ? 2 * GC.getHILLS_EXTRA_MOVEMENT() : 0);
						iValue /= 3;
					}
					ImprovementTypes eImprovement = (ImprovementTypes)kOriginalBuildInfo.getImprovement();

					if (eImprovement != NO_IMPROVEMENT)
					{
						int iRouteMultiplier = ((GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eRoute, YIELD_FOOD)) * 100);
						iRouteMultiplier += ((GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eRoute, YIELD_PRODUCTION)) * 100);
						iRouteMultiplier += ((GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eRoute, YIELD_COMMERCE)) * 60);
						iValue *= 100 + iRouteMultiplier;
						iValue /= 100;
					}

					int iPlotGroupId = -1;
					for (int iDirection = 0; iDirection < NUM_DIRECTION_TYPES; iDirection++)
					{
						CvPlot* pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), (DirectionTypes)iDirection);
						if (pLoopPlot != NULL)
						{
							if (pPlot->isRiver() || (pLoopPlot->getRouteType() != NO_ROUTE))
							{
								CvPlotGroup* pLoopGroup = pLoopPlot->getPlotGroup(getOwnerINLINE());
								if (pLoopGroup != NULL)
								{
									if (pLoopGroup->getID() != -1)
									{
										if (pLoopGroup->getID() != iPlotGroupId)
										{
											//This plot bridges plot groups, so route it.
											iValue *= 4;
											break;
										}
										else
										{
											iPlotGroupId = pLoopGroup->getID();
										}
									}
								}
							}
						}
					}
				}

				iValue /= (kBuildInfo.getTime() + 1);

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					eBestBuild = eBuild;
				}
			}
		}
	}

	if (eBestBuild == NO_BUILD)
	{
		return eBuild;
	}
	return eBestBuild;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_connectBonus(bool bTestTrade)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	BonusTypes eNonObsoleteBonus;
	int iI;

	// XXX how do we make sure that we can build roads???

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				eNonObsoleteBonus = pLoopPlot->getNonObsoleteBonusType(getTeam());

				if (eNonObsoleteBonus != NO_BONUS)
				{
					if (!(pLoopPlot->isConnectedToCapital()))
					{
						if (!bTestTrade || ((pLoopPlot->getImprovementType() != NO_IMPROVEMENT) && (GC.getImprovementInfo(pLoopPlot->getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus))))
						{
							if (AI_connectPlot(pLoopPlot))
							{
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_connectCity()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	int iLoop;

	// XXX how do we make sure that we can build roads???

    pLoopCity = plot()->getWorkingCity();
    if (pLoopCity != NULL)
    {
        if (AI_plotValid(pLoopCity->plot()))
        {
            if (!(pLoopCity->isConnectedToCapital()))
            {
                if (AI_connectPlot(pLoopCity->plot(), 1))
                {
                    return true;
                }
            }
        }
    }

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			if (!(pLoopCity->isConnectedToCapital()))
			{
				if (AI_connectPlot(pLoopCity->plot(), 1))
				{
					return true;
				}
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_routeCity(bool bUnconnectedOnly)
{
	PROFILE_FUNC();

	CvCity* pRouteToCity;
	CvCity* pLoopCity;
	int iLoop;

	FAssert(canBuildRoute());

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			pRouteToCity = pLoopCity->AI_getRouteToCity();

			if (pRouteToCity != NULL)
			{
				//added Sephi
				if (!bUnconnectedOnly || !pLoopCity->isConnectedToCapital())
				{
					if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
					{
						if (generatePath(pLoopCity->plot(), MOVE_SAFE_TERRITORY, true))
						{
							if (!(pRouteToCity->plot()->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pRouteToCity->plot(), MISSIONAI_BUILD, getGroup()) == 0)
								{
									if (generatePath(pRouteToCity->plot(), MOVE_SAFE_TERRITORY, true))
									{
										getGroup()->pushMission(MISSION_ROUTE_TO, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pRouteToCity->plot());
										getGroup()->pushMission(MISSION_ROUTE_TO, pRouteToCity->getX_INLINE(), pRouteToCity->getY_INLINE(), MOVE_SAFE_TERRITORY, (getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, pRouteToCity->plot());

										return true;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_routeTerritory(bool bImprovementOnly)
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	ImprovementTypes eImprovement;
	RouteTypes eBestRoute;
	bool bValid;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iI, iJ;

	// XXX how do we make sure that we can build roads???

	FAssert(canBuildRoute());

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (AI_plotValid(pLoopPlot))
		{
			if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE()) // XXX team???
			{
				eBestRoute = GET_PLAYER(getOwnerINLINE()).getBestRoute(pLoopPlot);

				if (eBestRoute != NO_ROUTE)
				{
					if (eBestRoute != pLoopPlot->getRouteType())
					{
						if (bImprovementOnly)
						{
							bValid = false;

							eImprovement = pLoopPlot->getImprovementType();

							if (eImprovement != NO_IMPROVEMENT)
							{
								for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
								{
									if (GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eBestRoute, iJ) > 0)
									{
										bValid = true;
										break;
									}
								}
							}
						}
						else
						{
							bValid = true;
						}

						if (bValid)
						{
							if (!(pLoopPlot->isVisibleEnemyUnit(this)))
							{
								if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_BUILD, getGroup(), 1) == 0)
								{
									if (generatePath(pLoopPlot, MOVE_SAFE_TERRITORY, true, &iPathTurns))
									{
										iValue = 10000;

										iValue /= (iPathTurns + 1);

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
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_ROUTE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_SAFE_TERRITORY, false, false, MISSIONAI_BUILD, pBestPlot);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_travelToUpgradeCity()
{
	// is there a city which can upgrade us?
	CvCity* pUpgradeCity = getUpgradeCity(/*bSearch*/ true);
	if (pUpgradeCity != NULL)
	{
		// cache some stuff
		CvPlot* pPlot = plot();
		bool bSeaUnit = (getDomainType() == DOMAIN_SEA);
		bool bCanAirliftUnit = (getDomainType() == DOMAIN_LAND);
		bool bShouldSkipToUpgrade = (getDomainType() != DOMAIN_AIR);

		// if we at the upgrade city, stop, wait to get upgraded
		if (pUpgradeCity->plot() == pPlot)
		{
			if (!bShouldSkipToUpgrade)
			{
				return false;
			}

			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}

		if (DOMAIN_AIR == getDomainType())
		{
			FAssert(!atPlot(pUpgradeCity->plot()));
			getGroup()->pushMission(MISSION_MOVE_TO, pUpgradeCity->getX_INLINE(), pUpgradeCity->getY_INLINE());
			return true;
		}

		// find the closest city
		CvCity* pClosestCity = pPlot->getPlotCity();
		bool bAtClosestCity = (pClosestCity != NULL);
		if (pClosestCity == NULL)
		{
			pClosestCity = pPlot->getWorkingCity();
		}
		if (pClosestCity == NULL)
		{
			pClosestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), NO_PLAYER, getTeam(), true, bSeaUnit);
		}

		// can we path to the upgrade city?
		int iUpgradeCityPathTurns;
		CvPlot* pThisTurnPlot = NULL;
		bool bCanPathToUpgradeCity = generatePath(pUpgradeCity->plot(), 0, true, &iUpgradeCityPathTurns);
		if (bCanPathToUpgradeCity)
		{
			pThisTurnPlot = getPathEndTurnPlot();
		}

		// if we close to upgrade city, head there
		if (NULL != pThisTurnPlot && NULL != pClosestCity && (pClosestCity == pUpgradeCity || iUpgradeCityPathTurns < 4))
		{
			FAssert(!atPlot(pThisTurnPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlot->getX_INLINE(), pThisTurnPlot->getY_INLINE());
			return true;
		}

		// check for better airlift choice
		if (bCanAirliftUnit && NULL != pClosestCity && pClosestCity->getMaxAirlift() > 0)
		{
			// if we at the closest city, then do the airlift, or wait
			if (bAtClosestCity)
			{
				// can we do the airlift this turn?
				if (canAirliftAt(pClosestCity->plot(), pUpgradeCity->getX_INLINE(), pUpgradeCity->getY_INLINE()))
				{
					getGroup()->pushMission(MISSION_AIRLIFT, pUpgradeCity->getX_INLINE(), pUpgradeCity->getY_INLINE());
					return true;
				}
				// wait to do it next turn
				else
				{
					getGroup()->pushMission(MISSION_SKIP);
					return true;
				}
			}

			int iClosestCityPathTurns;
			CvPlot* pThisTurnPlotForAirlift = NULL;
			bool bCanPathToClosestCity = generatePath(pClosestCity->plot(), 0, true, &iClosestCityPathTurns);
			if (bCanPathToClosestCity)
			{
				pThisTurnPlotForAirlift = getPathEndTurnPlot();
			}

			// is the closest city closer pathing? If so, move toward closest city
			if (NULL != pThisTurnPlotForAirlift && (!bCanPathToUpgradeCity || iClosestCityPathTurns < iUpgradeCityPathTurns))
			{
				FAssert(!atPlot(pThisTurnPlotForAirlift));
				getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlotForAirlift->getX_INLINE(), pThisTurnPlotForAirlift->getY_INLINE());
				return true;
			}
		}

		// did not have better airlift choice, go ahead and path to the upgrade city
		if (NULL != pThisTurnPlot)
		{
			FAssert(!atPlot(pThisTurnPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pThisTurnPlot->getX_INLINE(), pThisTurnPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_retreatToCity(bool bPrimary, bool bAirlift, int iMaxPath)
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot = NULL;
	int iPathTurns;
	int iValue;
	int iBestValue = MAX_INT;
	int iPass;
	int iLoop;
	int iCurrentDanger = GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot());

/*************************************************************************************************/
/**	ADDON (MultiBarb) Sephi																		**/
/**	units from new barb teams should never try to reatreat to a city							**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
	if (getOwnerINLINE()==ANIMAL_PLAYER || getOwnerINLINE()==WILDMANA_PLAYER || getOwnerINLINE()==PIRATES_PLAYER
        || getOwnerINLINE()==WHITEHAND_PLAYER || getOwnerINLINE()==DEVIL_PLAYER || getOwnerINLINE()==MERCENARIES_PLAYER)
	{
		return false;
	}
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/


	pCity = plot()->getPlotCity();


	if (0 == iCurrentDanger)
	{
		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				if (!bPrimary || GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pCity->area()))
				{
					if (!bAirlift || (pCity->getMaxAirlift() > 0))
					{
						if (!(pCity->plot()->isVisibleEnemyUnit(this)))
						{
							getGroup()->pushMission(MISSION_SKIP);
							return true;
						}
					}
				}
			}
		}
	}

	for (iPass = 0; iPass < 4; iPass++)
	{
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (AI_plotValid(pLoopCity->plot()))
			{
				if (!bPrimary || GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pLoopCity->area()))
				{
					if (!bAirlift || (pLoopCity->getMaxAirlift() > 0))
					{
						if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
						{
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), ((iPass > 1) ? MOVE_IGNORE_DANGER : 0), true, &iPathTurns))
							{
								if (iPathTurns <= ((iPass == 2) ? 1 : iMaxPath))
								{
									if ((iPass > 0) || (getGroup()->canFight() || GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopCity->plot()) < iCurrentDanger))
									{
										iValue = iPathTurns;

										if (AI_getUnitAIType() == UNITAI_SETTLER_SEA)
										{
											iValue *= 1 + std::max(0, GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(pLoopCity->area(), UNITAI_SETTLE) - GET_PLAYER(getOwnerINLINE()).AI_totalAreaUnitAIs(pLoopCity->area(), UNITAI_SETTLER_SEA));
										}
/*************************************************************************************************/
/**	BETTER AI (Worker only retreat where they are 100% safe) Sephi            					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
/** orig
										if (iValue < iBestValue)
										{
											iBestValue = iValue;
											pBestPlot = getPathEndTurnPlot();
											FAssert(!atPlot(pBestPlot));
										}
**/
										CvPlot* pNewPlot = getPathEndTurnPlot();

										if (iValue < iBestValue && (pNewPlot->isCity() || canDefend() || (!GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pNewPlot, 3))))
										{
											iBestValue = iValue;
											pBestPlot = pNewPlot;

											//TODO: is iCurrentDanger == 0 means no danger???
											if(isOOSLogging() && !(!atPlot(pBestPlot) || canDefend()))
											{
												oosLog("AIplot","Turn: %d, Player: %d,UnitID: %d,X: %d Y: %d,dstX: %d dstY: %d,iCurrentDanger:%d,isCity:%d,canDefend:%d,isPlotThreatened:%d"
												,GC.getGameINLINE().getElapsedGameTurns()
												,getOwnerINLINE()
												,getID()
												,plot()->getX()
												,plot()->getY()
												,pBestPlot->getX()
												,pBestPlot->getY()
												,iCurrentDanger
												,pNewPlot->isCity()
												,canDefend()
												,(!GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pNewPlot, 3))
												);
											}
											//FAssert(!atPlot(pBestPlot));
											FAssert(!atPlot(pBestPlot) || canDefend()); //SpyFanatic: prevent triggering if unit remain on same plot but can defend
										}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
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
			break;
		}
		else if (iPass == 0)
		{
			if (pCity != NULL)
			{
				if (pCity->getOwnerINLINE() == getOwnerINLINE())
				{
					if (!bPrimary || GET_PLAYER(getOwnerINLINE()).AI_isPrimaryArea(pCity->area()))
					{
						if (!bAirlift || (pCity->getMaxAirlift() > 0))
						{
							if (!(pCity->plot()->isVisibleEnemyUnit(this)))
							{
								getGroup()->pushMission(MISSION_SKIP);
								return true;
							}
						}
					}
				}
			}
		}

		if (getGroup()->alwaysInvisible())
		{
			break;
		}
	}

	if (pBestPlot != NULL)
	{
		//FAssert(!atPlot(pBestPlot)); //SpyFanatic: redundant
		//FAssert(!atPlot(pBestPlot) || canDefend()); //SpyFanatic: prevent triggering if unit remain on same plot but can defend
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((iPass > 0) ? MOVE_IGNORE_DANGER : 0));
		return true;
	}

	if (pCity != NULL)
	{
		if (pCity->getTeam() == getTeam())
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
	}

	return false;
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_pickup(UnitAITypes eUnitAI)
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestPickupPlot;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	FAssert(cargoSpace() > 0);
	if (0 == cargoSpace())
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->plot()->plotCount(PUF_isUnitAIType, eUnitAI, -1, getOwnerINLINE()) > 0)
			{
				if ((AI_getUnitAIType() != UNITAI_ASSAULT_SEA) || pCity->AI_isDefended(-1))
				{
					getGroup()->pushMission(MISSION_SKIP, -1, -1, 0, false, false, MISSIONAI_PICKUP, pCity->plot());
					return true;
				}
			}
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;
	pBestPickupPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{

			if ((AI_getUnitAIType() != UNITAI_ASSAULT_SEA) || pLoopCity->AI_isDefended(-1))
			{
				int iCount = pLoopCity->plot()->plotCount(PUF_isUnitAIType, eUnitAI, -1, getOwnerINLINE(), NO_TEAM, PUF_isFiniteRange);
				iValue = iCount * 10;

				if (pLoopCity->getProductionUnitAI() == eUnitAI)
				{
					CvUnitInfo& kUnitInfo = GC.getUnitInfo(pLoopCity->getProductionUnit());
					if ((kUnitInfo.getDomainType() != DOMAIN_AIR) || kUnitInfo.getAirRange() > 0)
					{
						iValue++;
						iCount++;
					}
				}

				if (iValue > 0)
				{
					iValue += pLoopCity->getPopulation();

					if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
					{
						if (GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_PICKUP, getGroup()) < ((iCount + (cargoSpace() - 1)) / cargoSpace()))
						{
							if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
							{
								iValue *= 1000;

								iValue /= (iPathTurns + 1);

								if (iValue > iBestValue)
								{
									iBestValue = iValue;
									pBestPlot = pLoopCity->plot();
									pBestPickupPlot = pLoopCity->plot();
								}
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestPickupPlot != NULL))
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_PICKUP, pBestPickupPlot);
		return true;
	}

	return false;
}



// Returns true if a mission was pushed...
bool CvUnitAI::AI_airOffensiveCity()
{
	PROFILE_FUNC();

	CvCity* pNearestEnemyCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	FAssert(canAirAttack() || nukeRange() >= 0);

	iBestValue = 0;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isCity(true, getTeam()))
		{
			if (pLoopPlot->getTeam() == getTeam()) // XXX team???
			{
				CvCity* pLoopCity = pLoopPlot->getPlotCity();
				bool bValid = false;

				int iAirBaseValue = (pLoopCity != NULL) ? 0 : GET_PLAYER(getOwnerINLINE()).AI_getPlotAirbaseValue(pLoopPlot);
				iAirBaseValue /= 6;

				int iDefenders = pLoopPlot->plotCount(PUF_canDefend, -1, -1, getOwnerINLINE());

				if (pLoopCity != NULL)
				{
					if (iDefenders > 2)
					{
						bValid = true;
						if (!pLoopCity->AI_isDanger())
						{
							iDefenders += 2;
						}
					}
				}
				else if (iAirBaseValue > 0)
				{
					bValid = true;
				}



				if (bValid)
				{
					if (atPlot(pLoopPlot) || canMoveInto(pLoopPlot))
					{
						// XXX is in danger?
						iValue = iAirBaseValue;
						if (pLoopCity != NULL)
						{
							iValue = (pLoopCity->getPopulation() + 20);
							iValue += pLoopCity->AI_cityThreat();
						}

						if (pLoopPlot->getOwnerINLINE() == getOwnerINLINE())
						{
							iValue *= 3;
							iValue /= 2;
						}

						iValue *= 1000;

						pNearestEnemyCity = GC.getMapINLINE().findCity(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), NO_PLAYER, NO_TEAM, false, false, getTeam());

						if (pNearestEnemyCity != NULL)
						{
							int iDistance = plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pNearestEnemyCity->getX_INLINE(), pNearestEnemyCity->getY_INLINE());
							if (iDistance > airRange())
							{
								iValue /= 10 * (2 + airRange());
							}
							else
							{
								iValue /= 2 + iDistance;
							}
						}

						int iAttackAirCount = pLoopPlot->plotCount(PUF_canAirAttack, -1, -1, NO_PLAYER, getTeam());
						iAttackAirCount += 2 * pLoopPlot->plotCount(PUF_isUnitAIType, UNITAI_ICBM, -1, NO_PLAYER, getTeam());
						if (atPlot(pLoopPlot))
						{
							iAttackAirCount += canAirAttack() ? -1 : 0;
							iAttackAirCount += (nukeRange() >= 0) ? -2 : 0;
						}


						if (iAttackAirCount <= iDefenders)
						{
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

	if (pBestPlot != NULL)
	{
		if (!atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_airDefensiveCity()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iLoop;

	FAssert(getDomainType() == DOMAIN_AIR);
	FAssert(canAirDefend());

	if (canAirDefend())
	{
		pCity = plot()->getPlotCity();

		if (pCity != NULL)
		{
			if (pCity->getOwnerINLINE() == getOwnerINLINE())
			{
				if (!(pCity->AI_isAirDefended(-1)))
				{
					getGroup()->pushMission(MISSION_AIRPATROL);
					return true;
				}
			}
		}
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (canAirDefend(pLoopCity->plot()))
		{
			if (!(pLoopCity->AI_isAirDefended()))
			{
				if (!atPlot(pLoopCity->plot()) && canMoveInto(pLoopCity->plot()))
				{
					iValue = pLoopCity->getPopulation() + pLoopCity->AI_cityThreat();

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopCity->plot();
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_airCarrier()
{
	PROFILE_FUNC();

	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iValue;
	int iBestValue;
	int iLoop;

	if (getCargo() > 0)
	{
		return false;
	}

	if (isCargo())
	{
		if (canAirDefend())
		{
			getGroup()->pushMission(MISSION_AIRPATROL);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
	}

	iBestValue = 0;
	pBestUnit = NULL;

	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (canLoadUnit(pLoopUnit, pLoopUnit->plot()))
		{
			iValue = 10;

			if (!(pLoopUnit->plot()->isCity()))
			{
				iValue += 20;
			}

			if (pLoopUnit->plot()->isOwned())
			{
				if (isEnemy(pLoopUnit->plot()->getTeam(), pLoopUnit->plot()))
				{
					iValue += 20;
				}
			}
			else
			{
				iValue += 10;
			}

			iValue /= (pLoopUnit->getCargo() + 1);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				pBestUnit = pLoopUnit;
			}
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			setTransportUnit(pBestUnit); // XXX is this dangerous (not pushing a mission...) XXX air units?
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestUnit->getX_INLINE(), pBestUnit->getY_INLINE());
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_missileLoad(UnitAITypes eTargetUnitAI, int iMaxOwnUnitAI, bool bStealthOnly)
{
	PROFILE_FUNC();

	CvUnit* pBestUnit = NULL;
	int iBestValue = 0;
	int iLoop;
	CvUnit* pLoopUnit;
	for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
	{
		if (!bStealthOnly || pLoopUnit->getInvisibleType() != NO_INVISIBLE)
		{
			if (pLoopUnit->AI_getUnitAIType() == eTargetUnitAI)
			{
				if ((iMaxOwnUnitAI == -1) || (pLoopUnit->getUnitAICargo(AI_getUnitAIType()) <= iMaxOwnUnitAI))
				{
					if (canLoadUnit(pLoopUnit, pLoopUnit->plot()))
					{
						int iValue = 100;

						iValue += GC.getGame().getSorenRandNum(100, "AI missile load");

						iValue *= 1 + pLoopUnit->getCargo();

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestUnit = pLoopUnit;
						}
					}
				}
			}
		}
	}

	if (pBestUnit != NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			setTransportUnit(pBestUnit); // XXX is this dangerous (not pushing a mission...) XXX air units?
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestUnit->getX_INLINE(), pBestUnit->getY_INLINE());
			setTransportUnit(pBestUnit);
			return true;
		}
	}

	return false;

}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_airStrike()
{
	PROFILE_FUNC();

	CvUnit* pDefender;
	CvUnit* pInterceptor;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iDamage;
	int iPotentialAttackers;
	int iInterceptProb;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = airRange();

	iBestValue = (isSuicide() && m_pUnitInfo->getProductionCost() > 0) ? (5 * m_pUnitInfo->getProductionCost()) / 6 : 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (canMoveInto(pLoopPlot, true))
				{
					iValue = 0;
					iPotentialAttackers = GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pLoopPlot);
					if (pLoopPlot->isCity())
					{
						iPotentialAttackers += GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pLoopPlot, MISSIONAI_ASSAULT, getGroup(), 1) * 2;
					}
					if (pLoopPlot->isWater() || (iPotentialAttackers > 0) || pLoopPlot->isAdjacentTeam(getTeam()))
					{
						pDefender = pLoopPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);

						FAssert(pDefender != NULL);
						FAssert(pDefender->canDefend());

						// XXX factor in air defenses...

						iDamage = airCombatDamage(pDefender);

						iValue = std::max(0, (std::min((pDefender->getDamage() + iDamage), airCombatLimit()) - pDefender->getDamage()));

						iValue += ((((iDamage * collateralDamage()) / 100) * std::min((pLoopPlot->getNumVisibleEnemyDefenders(this) - 1), collateralDamageMaxUnits())) / 2);

						iValue *= (3 + iPotentialAttackers);
						iValue /= 4;

						pInterceptor = bestInterceptor(pLoopPlot);

						if (pInterceptor != NULL)
						{
							iInterceptProb = isSuicide() ? 100 : pInterceptor->currInterceptionProbability();

							iInterceptProb *= std::max(0, (100 - evasionProbability()));
							iInterceptProb /= 100;

							iValue *= std::max(0, 100 - iInterceptProb / 2);
							iValue /= 100;
						}

						if (pLoopPlot->isWater())
						{
							iValue *= 3;
						}

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopPlot;
							FAssert(!atPlot(pBestPlot));
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}

bool CvUnitAI::AI_airBombPlots()
{
	PROFILE_FUNC();

	CvUnit* pInterceptor;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iInterceptProb;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = airRange();

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (!pLoopPlot->isCity() && pLoopPlot->isOwned() && pLoopPlot != plot())
				{
					if (canAirBombAt(plot(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()))
					{
						iValue = 0;

						if (pLoopPlot->getBonusType(pLoopPlot->getTeam()) != NO_BONUS)
						{
							iValue += AI_pillageValue(pLoopPlot, 15);

							iValue += GC.getGameINLINE().getSorenRandNum(10, "AI Air Bomb");
						}
						else if (isSuicide())
						{
							//This should only be reached when the unit is desperate to die
							iValue += AI_pillageValue(pLoopPlot);
							// Guided missiles lean towards destroying resource-producing tiles as opposed to improvements like Towns
							if (pLoopPlot->getBonusType(pLoopPlot->getTeam()) != NO_BONUS)
							{
								//and even more so if it's a resource
								iValue += GET_PLAYER(pLoopPlot->getOwnerINLINE()).AI_bonusVal(pLoopPlot->getBonusType(pLoopPlot->getTeam()));
							}
						}

						if (iValue > 0)
						{

							pInterceptor = bestInterceptor(pLoopPlot);

							if (pInterceptor != NULL)
							{
								iInterceptProb = isSuicide() ? 100 : pInterceptor->currInterceptionProbability();

								iInterceptProb *= std::max(0, (100 - evasionProbability()));
								iInterceptProb /= 100;

								iValue *= std::max(0, 100 - iInterceptProb / 2);
								iValue /= 100;
							}

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopPlot;
								FAssert(!atPlot(pBestPlot));
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRBOMB, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}


bool CvUnitAI::AI_airBombDefenses()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvUnit* pInterceptor;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPotentialAttackers;
	int iInterceptProb;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	iSearchRange = airRange();

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				pCity = pLoopPlot->getPlotCity();
				if (pCity != NULL)
				{
					iValue = 0;

					if (canAirBombAt(plot(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()))
					{
						iPotentialAttackers = GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pLoopPlot);
						iPotentialAttackers += std::max(0, GET_PLAYER(getOwnerINLINE()).AI_plotTargetMissionAIs(pCity->plot(), NO_MISSIONAI, getGroup(), 2) - 4);

						if (iPotentialAttackers > 1)
						{
							iValue += std::max(0, (std::min((pCity->getDefenseDamage() + airBombCurrRate()), GC.getMAX_CITY_DEFENSE_DAMAGE()) - pCity->getDefenseDamage()));

							iValue *= 4 + iPotentialAttackers;

							if (pCity->AI_isDanger())
							{
								iValue *= 2;
							}

							if (pCity == pCity->area()->getTargetCity(getOwnerINLINE()))
							{
								iValue *= 2;
							}
						}

						if (iValue > 0)
						{
							pInterceptor = bestInterceptor(pLoopPlot);

							if (pInterceptor != NULL)
							{
								iInterceptProb = isSuicide() ? 100 : pInterceptor->currInterceptionProbability();

								iInterceptProb *= std::max(0, (100 - evasionProbability()));
								iInterceptProb /= 100;

								iValue *= std::max(0, 100 - iInterceptProb / 2);
								iValue /= 100;
							}

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopPlot;
								FAssert(!atPlot(pBestPlot));
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_AIRBOMB, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;

}

bool CvUnitAI::AI_exploreAir()
{
	PROFILE_FUNC();

	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
	int iLoop;
	CvCity* pLoopCity;
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;

	for (int iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && !GET_PLAYER((PlayerTypes)iI).isBarbarian())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
			{
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (!pLoopCity->isVisible(getTeam(), false))
					{
						if (canReconAt(plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
						{
							int iValue = 1 + GC.getGame().getSorenRandNum(15, "AI explore air");
							if (isEnemy(GET_PLAYER((PlayerTypes)iI).getTeam()))
							{
								iValue += 10;
								iValue += std::min(10,  pLoopCity->area()->getNumAIUnits(getOwnerINLINE(), UNITAI_ATTACK_CITY));
								iValue += 10 * kPlayer.AI_plotTargetMissionAIs(pLoopCity->plot(), MISSIONAI_ASSAULT);
							}

							iValue *= plotDistance(getX_INLINE(), getY_INLINE(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopCity->plot();
							}
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_RECON, pBestPlot->getX(), pBestPlot->getY());
		return true;
	}

	return false;
}


// Returns true if a mission was pushed...
bool CvUnitAI::AI_nuke()
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	pBestCity = NULL;

	iBestValue = 0;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive() && !GET_PLAYER((PlayerTypes)iI).isBarbarian())
		{
			if (isEnemy(GET_PLAYER((PlayerTypes)iI).getTeam()))
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude((PlayerTypes)iI) == ATTITUDE_FURIOUS)
				{
					for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
					{
						if (canNukeAt(plot(), pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE()))
						{
							iValue = AI_nukeValue(pLoopCity);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestCity = pLoopCity;
								FAssert(pBestCity->getTeam() != getTeam());
							}
						}
					}
				}
			}
		}
	}

	if (pBestCity != NULL)
	{
		getGroup()->pushMission(MISSION_NUKE, pBestCity->getX_INLINE(), pBestCity->getY_INLINE());
		return true;
	}

	return false;
}

bool CvUnitAI::AI_nukeRange(int iRange)
{
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;
	for (int iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (int iDY = -(iRange); iDY <= iRange; iDY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (canNukeAt(plot(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()))
				{
					int iValue = -99;

					for (int iDX2 = -(nukeRange()); iDX2 <= nukeRange(); iDX2++)
					{
						for (int iDY2 = -(nukeRange()); iDY2 <= nukeRange(); iDY2++)
						{
							CvPlot* pLoopPlot2 = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iDX2, iDY2);

							if (pLoopPlot2 != NULL)
							{
								int iEnemyCount = 0;
								int iTeamCount = 0;
								int iNeutralCount = 0;
								int iDamagedEnemyCount = 0;

								CLLNode<IDInfo>* pUnitNode;
								CvUnit* pLoopUnit;
								pUnitNode = pLoopPlot2->headUnitNode();
								while (pUnitNode != NULL)
								{
									pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = pLoopPlot2->nextUnitNode(pUnitNode);

									if (!pLoopUnit->isNukeImmune())
									{
										if (pLoopUnit->getTeam() == getTeam())
										{
											iTeamCount++;
										}
										else if (!pLoopUnit->isInvisible(getTeam(), false))
										{
											if (isEnemy(pLoopUnit->getTeam()))
											{
												iEnemyCount++;
												if (pLoopUnit->getDamage() * 2 > pLoopUnit->maxHitPoints())
												{
													iDamagedEnemyCount++;
												}
											}
											else
											{
												iNeutralCount++;
											}
										}
									}
								}

								iValue += (iEnemyCount + iDamagedEnemyCount) * (pLoopPlot2->isWater() ? 25 : 12);
								iValue -= iTeamCount * 15;
								iValue -= iNeutralCount * 20;


								int iMultiplier = 1;
								if (pLoopPlot2->getTeam() == getTeam())
								{
									iMultiplier = -2;
								}
								else if (isEnemy(pLoopPlot2->getTeam()))
								{
									iMultiplier = 1;
								}
								else if (!pLoopPlot2->isOwned())
								{
									iMultiplier = 0;
								}
								else
								{
									iMultiplier = -10;
								}

								if (pLoopPlot2->getImprovementType() != NO_IMPROVEMENT)
								{
									iValue += iMultiplier * 10;
								}

								if (pLoopPlot2->getBonusType() != NO_BONUS)
								{
									iValue += iMultiplier * 20;
								}

								if (pLoopPlot2->isCity())
								{
									iValue += std::max(0, iMultiplier * (-20 + 15 * pLoopPlot2->getPlotCity()->getPopulation()));
								}
							}
						}
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopPlot;
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_NUKE, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}

bool CvUnitAI::AI_trade(int iValueThreshold)
{
	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvPlot* pBestTradePlot;

	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;


	iBestValue = 0;
	pBestPlot = NULL;
	pBestTradePlot = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (AI_plotValid(pLoopCity->plot()))
				{
                    if (getTeam() != pLoopCity->getTeam())
				    {
                        iValue = getTradeGold(pLoopCity->plot());

                        if ((iValue >= iValueThreshold) && canTrade(pLoopCity->plot(), true))
                        {
                            if (!(pLoopCity->plot()->isVisibleEnemyUnit(this)))
                            {
                                if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
                                {
                                    FAssert(iPathTurns > 0);

                                    iValue /= (4 + iPathTurns);

                                    if (iValue > iBestValue)
                                    {
                                        iBestValue = iValue;
                                        pBestPlot = getPathEndTurnPlot();
                                        pBestTradePlot = pLoopCity->plot();
                                    }
                                }

                            }
                        }
				    }
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestTradePlot != NULL))
	{
		if (atPlot(pBestTradePlot))
		{
			getGroup()->pushMission(MISSION_TRADE);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_infiltrate()
{
	CvCity* pLoopCity;
	CvPlot* pBestPlot;

	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestPlot = NULL;

	if (canInfiltrate(plot()))
	{
		getGroup()->pushMission(MISSION_INFILTRATE);
		return true;
	}

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if ((GET_PLAYER((PlayerTypes)iI).isAlive()) && GET_PLAYER((PlayerTypes)iI).getTeam() != getTeam())
		{
			for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
			{
				if (canInfiltrate(pLoopCity->plot()))
				{
					iValue = getEspionagePoints(pLoopCity->plot());

					if (iValue > 0)
					{
						if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
						{
							FAssert(iPathTurns > 0);

							if (getPathLastNode()->m_iData1 == 0)
							{
								iPathTurns++;
							}

							iValue /= 1 + iPathTurns;

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestPlot = pLoopCity->plot();
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL))
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_INFILTRATE);
			return true;
		}
		else
		{
			FAssert(!atPlot(pBestPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			getGroup()->pushMission(MISSION_INFILTRATE, -1, -1, 0, (getGroup()->getLengthMissionQueue() > 0));
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_reconSpy(int iRange)
{
	PROFILE_FUNC();
	CvPlot* pLoopPlot;
	int iX, iY;

	CvPlot* pBestPlot = NULL;
	CvPlot* pBestTargetPlot = NULL;
	int iBestValue = 0;

	int iSearchRange = AI_searchRange(iRange);

	for (iX = -iSearchRange; iX <= iSearchRange; iX++)
	{
		for (iY = -iSearchRange; iY <= iSearchRange; iY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			int iDistance = stepDistance(0, 0, iX, iY);
			if ((iDistance > 0) && (pLoopPlot != NULL) && AI_plotValid(pLoopPlot))
			{
				int iValue = 0;
				if (pLoopPlot->getPlotCity() != NULL)
				{
					iValue += GC.getGameINLINE().getSorenRandNum(4000, "AI Spy Scout City");
				}

				if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
				{
					iValue += GC.getGameINLINE().getSorenRandNum(1000, "AI Spy Recon Bonus");
				}

				for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
				{
					CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), ((DirectionTypes)iI));

					if (pAdjacentPlot != NULL)
					{
						if (!pAdjacentPlot->isRevealed(getTeam(), false))
						{
							iValue += 500;
						}
						else if (!pAdjacentPlot->isVisible(getTeam(), false))
						{
							iValue += 200;
						}
					}
				}


				if (iValue > 0)
				{
					int iPathTurns;
					if (generatePath(pLoopPlot, 0, true, &iPathTurns))
					{
						if (iPathTurns <= iRange)
						{
							// don't give each and every plot in range a value before generating the patch (performance hit)
							iValue += GC.getGameINLINE().getSorenRandNum(250, "AI Spy Scout Best Plot");

							iValue *= iDistance;

							/* Can no longer perform missions after having moved
							if (getPathLastNode()->m_iData2 == 1)
							{
								if (getPathLastNode()->m_iData1 > 0)
								{
									//Prefer to move and have movement remaining to perform a kill action.
									iValue *= 2;
								}
							} */

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								pBestTargetPlot = getPathEndTurnPlot();
								pBestPlot = pLoopPlot;
							}
						}
					}
				}
			}
		}
	}

	if ((pBestPlot != NULL) && (pBestTargetPlot != NULL))
	{
		if (atPlot(pBestTargetPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestTargetPlot->getX_INLINE(), pBestTargetPlot->getY_INLINE());
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
	}

	return false;
}

int CvUnitAI::AI_getEspionageTargetValue(CvPlot* pPlot, int iMaxPath)
{
	CvTeamAI& kTeam = GET_TEAM(getTeam());
	int iValue = 0;

	if (pPlot->isOwned() && pPlot->getTeam() != getTeam() && !GET_TEAM(getTeam()).isVassal(pPlot->getTeam()))
	{
		if (AI_plotValid(pPlot))
		{
			if (pPlot->isCity())
			{
				iValue += 10;
				int iRand = GC.getGame().getSorenRandNum(8, "AI spy choose city");
				iValue += iRand * iRand;
			}
			else
			{
				BonusTypes eBonus = pPlot->getNonObsoleteBonusType(getTeam());
				if (eBonus != NO_BONUS)
				{
					iValue += GET_PLAYER(pPlot->getOwnerINLINE()).AI_baseBonusVal(eBonus) - 10;
				}
			}

			int iPathTurns;
			if (generatePath(pPlot, 0, true, &iPathTurns))
			{
				if (iPathTurns <= iMaxPath)
				{
					if (kTeam.AI_getWarPlan(pPlot->getTeam()) == NO_WARPLAN)
					{
						iValue *= 1;
					}
					else if (kTeam.AI_isSneakAttackPreparing(pPlot->getTeam()))
					{
						iValue *= (pPlot->isCity()) ? 15 : 10;
					}
					else
					{
						iValue *= 3;
					}
				}
			}
		}
	}

	return iValue;
}


bool CvUnitAI::AI_cityOffenseSpy(int iMaxPath)
{
	int iBestValue = 0;
	CvPlot* pBestPlot = NULL;

	for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive() && kLoopPlayer.getTeam() != getTeam() && !GET_TEAM(getTeam()).isVassal(kLoopPlayer.getTeam()))
		{
			int iLoop;
			for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); NULL != pLoopCity; pLoopCity = kLoopPlayer.nextCity(&iLoop))
			{
				if (pLoopCity->area() == area())
				{
					CvPlot* pLoopPlot = pLoopCity->plot();
					if (AI_plotValid(pLoopPlot))
					{
						int iValue = AI_getEspionageTargetValue(pLoopPlot, iMaxPath);
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

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			getGroup()->pushMission(MISSION_SKIP);
		}
		return true;
	}

	return false;
}

bool CvUnitAI::AI_bonusOffenseSpy(int iRange)
{
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;

	int iSearchRange = AI_searchRange(iRange);

	for (int iX = -iSearchRange; iX <= iSearchRange; iX++)
	{
		for (int iY = -iSearchRange; iY <= iSearchRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);

			if (NULL != pLoopPlot && pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
			{
				int iValue = AI_getEspionageTargetValue(pLoopPlot, iRange);
				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestPlot = pLoopPlot;
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
	}

	return false;
}

//Returns true if the spy performs espionage.
bool CvUnitAI::AI_espionageSpy()
{
	if (!canEspionage(plot()))
	{
		return false;
	}

	EspionageMissionTypes eBestMission = NO_ESPIONAGEMISSION;
	CvPlot* pTargetPlot = NULL;
	PlayerTypes eTargetPlayer = NO_PLAYER;
	int iExtraData = -1;

	eBestMission = GET_PLAYER(getOwnerINLINE()).AI_bestPlotEspionage(plot(), eTargetPlayer, pTargetPlot, iExtraData);
	if (NO_ESPIONAGEMISSION == eBestMission)
	{
		return false;
	}

	if (!GET_PLAYER(getOwnerINLINE()).canDoEspionageMission(eBestMission, eTargetPlayer, pTargetPlot, iExtraData, this))
	{
		return false;
	}

	if (!espionage(eBestMission, iExtraData))
	{
		return false;
	}

	return true;
}

bool CvUnitAI::AI_moveToStagingCity()
{
	CvCity* pLoopCity;
	CvPlot* pBestPlot;

	int iPathTurns;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestPlot = NULL;

	int iWarCount = 0;
	TeamTypes eTargetTeam = NO_TEAM;
	CvTeam& kTeam = GET_TEAM(getTeam());
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if ((iI != getTeam()) && GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (kTeam.AI_isSneakAttackPreparing((TeamTypes)iI))
			{
				eTargetTeam = (TeamTypes)iI;
				iWarCount++;
			}
		}
	}
	if (iWarCount > 1)
	{
		eTargetTeam = NO_TEAM;
	}


	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
		if (AI_plotValid(pLoopCity->plot()))
		{
			iValue = pLoopCity->AI_cityThreat();
			if (iValue > 0)
			{
				if (generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
				{
					iValue *= 1000;
					iValue /= (5 + iPathTurns);
					if ((pLoopCity->plot() != plot()) && pLoopCity->isVisible(eTargetTeam, false))
					{
						iValue /= 2;
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = getPathEndTurnPlot();
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
			return true;
		}
	}

	return false;
}

bool CvUnitAI::AI_seaRetreatFromCityDanger()
{
	if (plot()->isCity(true) && GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0) //prioritize getting outta there
	{
		if (AI_anyAttack(2, 40))
		{
			return true;
		}

		if (AI_anyAttack(4, 50))
		{
			return true;
		}

		if (AI_retreatToCity())
		{
			return true;
		}

		if (AI_safety())
		{
			return true;
		}
	}
	return false;
}

bool CvUnitAI::AI_airRetreatFromCityDanger()
{
	if (plot()->isCity(true))
	{
		CvCity* pCity = plot()->getPlotCity();
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 2) > 0 || (pCity != NULL && !pCity->AI_isDefended()))
		{
			if (AI_airOffensiveCity())
			{
				return true;
			}

			if (canAirDefend() && AI_airDefensiveCity())
			{
				return true;
			}
		}
	}
	return false;
}

bool CvUnitAI::AI_airAttackDamagedSkip()
{
	if (getDamage() == 0)
	{
		return false;
	}

	bool bSkip = (currHitPoints() * 100 / maxHitPoints() < 40);
	if (!bSkip)
	{
		int iSearchRange = airRange();
		bool bSkiesClear = true;
		for (int iDX = -iSearchRange; iDX <= iSearchRange && bSkiesClear; iDX++)
		{
			for (int iDY = -iSearchRange; iDY <= iSearchRange && bSkiesClear; iDY++)
			{
				CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
				if (pLoopPlot != NULL)
				{
					if (bestInterceptor(pLoopPlot) != NULL)
					{
						bSkiesClear = false;
						break;
					}
				}
			}
		}
		bSkip = !bSkiesClear;
	}

	if (bSkip)
	{
		getGroup()->pushMission(MISSION_SKIP);
		return true;
	}

	return false;
}


// Returns true if a mission was pushed or we should wait for another unit to bombard...
bool CvUnitAI::AI_followBombard()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pAdjacentPlot1;
	CvPlot* pAdjacentPlot2;
	int iI, iJ;

	if (canBombard(plot()))
	{
		getGroup()->pushMission(MISSION_BOMBARD);
		return true;
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pAdjacentPlot1 = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot1 != NULL)
			{
				if (pAdjacentPlot1->isCity())
				{
					if (AI_potentialEnemy(pAdjacentPlot1->getTeam(), pAdjacentPlot1))
					{
						for (iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
						{
							pAdjacentPlot2 = plotDirection(pAdjacentPlot1->getX_INLINE(), pAdjacentPlot1->getY_INLINE(), ((DirectionTypes)iJ));

							if (pAdjacentPlot2 != NULL)
							{
								pUnitNode = pAdjacentPlot2->headUnitNode();

								while (pUnitNode != NULL)
								{
									pLoopUnit = ::getUnit(pUnitNode->m_data);
									pUnitNode = pAdjacentPlot2->nextUnitNode(pUnitNode);

									if (pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
									{
										if (pLoopUnit->canBombard(pAdjacentPlot2))
										{
											if (pLoopUnit->isGroupHead())
											{
												if (pLoopUnit->getGroup() != getGroup())
												{
													if (pLoopUnit->getGroup()->readyToMove())
													{
														return true;
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
			}
		}
	}

	return false;
}


// Returns true if the unit has found a potential enemy...
bool CvUnitAI::AI_potentialEnemy(TeamTypes eTeam, const CvPlot* pPlot)
{
	if (getGroup()->AI_isDeclareWar(pPlot))
	{
		return isPotentialEnemy(eTeam, pPlot);
	}
	else
	{
		return isEnemy(eTeam, pPlot);
	}
}


// Returns true if this plot needs some defense...
bool CvUnitAI::AI_defendPlot(CvPlot* pPlot)
{
	CvCity* pCity;

	if (!canDefend(pPlot))
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getOwnerINLINE() == getOwnerINLINE())
		{
			if (pCity->AI_isDanger())
			{
				return true;
			}
		}
	}
	else
	{
		if (pPlot->plotCount(PUF_canDefendGroupHead, -1, -1, getOwnerINLINE()) <= ((atPlot(pPlot)) ? 1 : 0))
		{
			if (pPlot->plotCount(PUF_cannotDefend, -1, -1, getOwnerINLINE()) > 0)
			{
				return true;
			}

//			if (pPlot->defenseModifier(getTeam(), false) >= 50 && pPlot->isRoute() && pPlot->getTeam() == getTeam())
//			{
//				return true;
//			}
		}
	}

	return false;
}


int CvUnitAI::AI_pillageValue(CvPlot* pPlot, int iBonusValueThreshold)
{
	CvPlot* pAdjacentPlot;
	ImprovementTypes eImprovement;
	BonusTypes eNonObsoleteBonus;
	int iValue;
	int iTempValue;
	int iBonusValue;
	int iI;

	FAssert(canPillage(pPlot) || canAirBombAt(plot(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) || (getGroup()->getCargo() > 0));

	if (!(pPlot->isOwned()))
	{
		return 0;
	}

	iBonusValue = 0;
	eNonObsoleteBonus = pPlot->getNonObsoleteBonusType(pPlot->getTeam());
	if (eNonObsoleteBonus != NO_BONUS)
	{
		iBonusValue = (GET_PLAYER(pPlot->getOwnerINLINE()).AI_bonusVal(eNonObsoleteBonus));
	}

	if (iBonusValueThreshold > 0)
	{
		if (eNonObsoleteBonus == NO_BONUS)
		{
			return 0;
		}
		else if (iBonusValue < iBonusValueThreshold)
		{
			return 0;
		}
	}

	iValue = 0;

	if (getDomainType() != DOMAIN_AIR)
	{
		if (pPlot->isRoute())
		{
			iValue++;
			if (eNonObsoleteBonus != NO_BONUS)
			{
				iValue += iBonusValue * 4;
			}

			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

				if (pAdjacentPlot != NULL && pAdjacentPlot->getTeam() == pPlot->getTeam())
				{
					if (pAdjacentPlot->isCity())
					{
						iValue += 10;
					}

					if (!(pAdjacentPlot->isRoute()))
					{
						if (!(pAdjacentPlot->isWater()) && !(pAdjacentPlot->isImpassable()))
						{
							iValue += 2;
						}
					}
				}
			}
		}
	}

	if (pPlot->getImprovementDuration() > ((pPlot->isWater()) ? 20 : 5))
	{
		eImprovement = pPlot->getImprovementType();
	}
	else
	{
		eImprovement = pPlot->getRevealedImprovementType(getTeam(), false);
	}

	if (eImprovement != NO_IMPROVEMENT)
	{
		if (pPlot->getWorkingCity() != NULL)
		{
			iValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_FOOD, pPlot->getOwnerINLINE()) * 5);
			iValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_PRODUCTION, pPlot->getOwnerINLINE()) * 4);
			iValue += (pPlot->calculateImprovementYieldChange(eImprovement, YIELD_COMMERCE, pPlot->getOwnerINLINE()) * 3);
		}

		if (getDomainType() != DOMAIN_AIR)
		{
			iValue += GC.getImprovementInfo(eImprovement).getPillageGold();
		}

		if (eNonObsoleteBonus != NO_BONUS)
		{
			if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eNonObsoleteBonus))
			{
				iTempValue = iBonusValue * 4;

				if (pPlot->isConnectedToCapital() && (pPlot->getPlotGroupConnectedBonus(pPlot->getOwnerINLINE(), eNonObsoleteBonus) == 1))
				{
					iTempValue *= 2;
				}

				iValue += iTempValue;
			}
		}
	}

	return iValue;
}


int CvUnitAI::AI_nukeValue(CvCity* pCity)
{
	PROFILE_FUNC();
	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		CvTeam& kLoopTeam = GET_TEAM((TeamTypes)iI);
		if (kLoopTeam.isAlive() && !isEnemy((TeamTypes)iI))
		{
			if (isNukeVictim(pCity->plot(), ((TeamTypes)iI)))
			{
				// Don't start wars with neutrals
				return 0;
			}
		}
	}

	int iValue = 1;

	iValue += GC.getGameINLINE().getSorenRandNum((pCity->getPopulation() + 1), "AI Nuke City Value");
	iValue += std::max(0, pCity->getPopulation() - 10);

	iValue += ((pCity->getPopulation() * (100 + pCity->calculateCulturePercent(pCity->getOwnerINLINE()))) / 100);

	iValue += -(GET_PLAYER(getOwnerINLINE()).AI_getAttitudeVal(pCity->getOwnerINLINE()) / 3);

	for (int iDX = -(nukeRange()); iDX <= nukeRange(); iDX++)
	{
		for (int iDY = -(nukeRange()); iDY <= nukeRange(); iDY++)
		{
			CvPlot* pLoopPlot = plotXY(pCity->getX_INLINE(), pCity->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					iValue++;
				}

				if (pLoopPlot->getBonusType() != NO_BONUS)
				{
					iValue++;
				}
			}
		}
	}

	if (!(pCity->isEverOwned(getOwnerINLINE())))
	{
		iValue *= 3;
		iValue /= 2;
	}

	if (!GET_TEAM(pCity->getTeam()).isAVassal())
	{
		iValue *= 2;
	}

	if (pCity->plot()->isVisible(getTeam(), false))
	{
		iValue += 2 * pCity->plot()->getNumVisibleEnemyDefenders(this);
	}
	else
	{
		iValue += 6;
	}

	return iValue;
}


int CvUnitAI::AI_searchRange(int iRange)
{
	if (iRange == 0)
	{
		return 0;
	}

	if (flatMovementCost() || (getDomainType() == DOMAIN_SEA))
	{
		return (iRange * baseMoves());
	}
	else
	{
		return ((iRange + 1) * (baseMoves() + 1));
	}
}


// XXX at some point test the game with and without this function...
bool CvUnitAI::AI_plotValid(CvPlot* pPlot)
{
	PROFILE_FUNC();

	if (m_pUnitInfo->isNoRevealMap() && willRevealByMove(pPlot))
	{
		return false;
	}

	switch (getDomainType())
	{
	case DOMAIN_SEA:
		if (pPlot->isWater() || canMoveAllTerrain())
		{
			return true;
		}
		else if (pPlot->isFriendlyCity(*this, true) && pPlot->isCoastalLand())
		{
			return true;
		}
		break;

	case DOMAIN_AIR:
		FAssert(false);
		break;

	case DOMAIN_LAND:
		if (pPlot->getArea() == getArea() || canMoveAllTerrain())
		{
			return true;
		}
		break;

	case DOMAIN_IMMOBILE:
		FAssert(false);
		break;

	default:
		FAssert(false);
		break;
	}

	return false;
}


int CvUnitAI::AI_finalOddsThreshold(CvPlot* pPlot, int iOddsThreshold)
{
	PROFILE_FUNC();

	CvCity* pCity;

	int iFinalOddsThreshold;

	iFinalOddsThreshold = iOddsThreshold;

	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (pCity->getDefenseDamage() < ((GC.getMAX_CITY_DEFENSE_DAMAGE() * 3) / 4))
		{
			iFinalOddsThreshold += std::max(0, (pCity->getDefenseDamage() - pCity->getLastDefenseDamage() - (GC.getDefineINT("CITY_DEFENSE_DAMAGE_HEAL_RATE") * 2)));
		}
	}

	if (pPlot->getNumVisiblePotentialEnemyDefenders(this) == 1)
	{
		if (pCity != NULL)
		{
			iFinalOddsThreshold *= 2;
			iFinalOddsThreshold /= 3;
		}
		else
		{
			iFinalOddsThreshold *= 7;
			iFinalOddsThreshold /= 8;
		}
	}

	if ((getDomainType() == DOMAIN_SEA) && !getGroup()->hasCargo())
	{
		iFinalOddsThreshold *= 3;
		iFinalOddsThreshold /= 2 + getGroup()->getNumUnits();
	}
	else
	{
		iFinalOddsThreshold *= 6;
		iFinalOddsThreshold /= (3 + GET_PLAYER(getOwnerINLINE()).AI_adjacentPotentialAttackers(pPlot, true) + ((stepDistance(getX_INLINE(), getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) > 1) ? 1 : 0) + ((AI_isCityAIType()) ? 2 : 0));
	}

	return range(iFinalOddsThreshold, 1, 99);
}


int CvUnitAI::AI_stackOfDoomExtra()
{
	return ((AI_getBirthmark() % (1 + GET_PLAYER(getOwnerINLINE()).getCurrentEra())) + 4);
}

bool CvUnitAI::AI_stackAttackCity(int iRange, int iPowerThreshold, bool bFollow)
{
    PROFILE_FUNC();
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	if (bFollow)
	{
		iSearchRange = 1;
	}
	else
	{
		iSearchRange = AI_searchRange(iRange);
	}

	iBestValue = 0;
	pBestPlot = NULL;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true) && pLoopPlot->isVisibleEnemyUnit(this)))
					{
						if (AI_potentialEnemy(pLoopPlot->getTeam(), pLoopPlot))
						{
							if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, /*bAttack*/ true, /*bDeclareWar*/ true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
							{
/*************************************************************************************************/
/** BETTER AI (attack odds) Sephi				                                                **/
/**	AI_compareStacks seems broken					                   							**/
/*************************************************************************************************/
// orig							iValue = getGroup()->AI_compareStacks(pLoopPlot, /*bPotentialEnemy*/ true, /*bCheckCanAttack*/ true, /*bCheckCanMove*/ true);

								int iOurStrength=getGroup()->AI_GroupPower(pLoopPlot,false);
								int iTheirStrength=GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(pLoopPlot,0,true,false);

								//If we aren't at war yet
								if(pLoopPlot->isOwned() && (!GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam())))
								{
									iTheirStrength=GET_PLAYER(pLoopPlot->getOwnerINLINE()).AI_getOurPlotStrength(pLoopPlot,0,true,false);
								}

								iValue = 100;
								iValue *=iOurStrength;
								iValue /=std::max(1,iTheirStrength);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
								if (iValue >= iPowerThreshold)
								{
									if (iValue > iBestValue)
									{
										iBestValue = iValue;
										pBestPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
										FAssert(!atPlot(pBestPlot));
									}
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
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
		return true;
	}

	return false;
}

bool CvUnitAI::AI_moveIntoCity(int iRange)
{
    PROFILE_FUNC();

	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iSearchRange = iRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

	FAssert(canMove());

	iBestValue = 0;
	pBestPlot = NULL;

	if (plot()->isCity())
	{
	    return false;
	}

	iSearchRange = AI_searchRange(iRange);

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (AI_plotValid(pLoopPlot) && (!isEnemy(pLoopPlot->getTeam(), pLoopPlot)))
				{
					if (pLoopPlot->isCity() || (pLoopPlot->isCity(true)))
					{
                        if (canMoveInto(pLoopPlot, false) && (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= 1)))
                        {
                            iValue = 1;
                            if (pLoopPlot->getPlotCity() != NULL)
                            {
                                 iValue += pLoopPlot->getPlotCity()->getPopulation();
                            }

                            if (iValue > iBestValue)
                            {
                                iBestValue = iValue;
                                pBestPlot = getPathEndTurnPlot();
                                FAssert(!atPlot(pBestPlot));
                            }
						}
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		FAssert(!atPlot(pBestPlot));
		getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
		return true;
	}

	return false;
}

//bolsters the culture of the weakest city.
//returns true if a mission is pushed.
bool CvUnitAI::AI_artistCultureVictoryMove()
{
    bool bGreatWork = false;
    bool bJoin = true;

    if (!GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE1))
    {
        return false;
    }

    if (GET_PLAYER(getOwnerINLINE()).AI_isDoStrategy(AI_STRATEGY_CULTURE3))
    {
        //Great Work
        bGreatWork = true;
    }

	int iCultureCitiesNeeded = GC.getGameINLINE().culturalVictoryNumCultureCities();
	FAssertMsg(iCultureCitiesNeeded > 0, "CultureVictory Strategy should not be true");

	CvCity* pLoopCity;
	CvPlot* pBestPlot;
	CvCity* pBestCity;
	SpecialistTypes eBestSpecialist;
	int iLoop, iValue, iBestValue;

	pBestPlot = NULL;
	eBestSpecialist = NO_SPECIALIST;

	pBestCity = NULL;

	iBestValue = 0;
	iLoop = 0;

	int iTargetCultureRank = iCultureCitiesNeeded;
	while (iTargetCultureRank > 0 && pBestCity == NULL)
	{
		for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
		{
			if (AI_plotValid(pLoopCity->plot()))
			{
				// instead of commerce rate rank should use the culture on tile...
				if (pLoopCity->findCommerceRateRank(COMMERCE_CULTURE) == iTargetCultureRank)
				{
					// if the city is a fledgling, probably building culture, try next higher city
					if (pLoopCity->getCultureLevel() < 2)
					{
						break;
					}

					// if we cannot path there, try the next higher culture city
					if (!generatePath(pLoopCity->plot(), 0, true))
					{
						break;
					}

					pBestCity = pLoopCity;
					pBestPlot = pLoopCity->plot();
					if (bGreatWork)
					{
						if (canGreatWork(pBestPlot))
						{
							break;
						}
					}

					for (int iI = 0; iI < GC.getNumSpecialistInfos(); iI++)
					{
						if (canJoin(pBestPlot, ((SpecialistTypes)iI)))
						{
							iValue = pLoopCity->AI_specialistValue(((SpecialistTypes)iI), pLoopCity->AI_avoidGrowth(), false);

							if (iValue > iBestValue)
							{
								iBestValue = iValue;
								eBestSpecialist = ((SpecialistTypes)iI);
							}
						}
					}

					if (eBestSpecialist == NO_SPECIALIST)
					{
						bJoin = false;
						if (canGreatWork(pBestPlot))
						{
							bGreatWork = true;
							break;
						}
						bGreatWork = false;
					}
					break;
				}
			}
		}

		iTargetCultureRank--;
	}


	FAssertMsg(bGreatWork || bJoin, "This wasn't a Great Artist");

	if (pBestCity == NULL)
	{
	    //should try to airlift there...
	    return false;
	}


    if (atPlot(pBestPlot))
    {
        if (bGreatWork)
        {
            getGroup()->pushMission(MISSION_GREAT_WORK);
            return true;
        }
        if (bJoin)
        {
            getGroup()->pushMission(MISSION_JOIN, eBestSpecialist);
            return true;
        }
        FAssert(false);
        return false;
    }
    else
    {
        FAssert(!atPlot(pBestPlot));
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
        return true;
    }
}

bool CvUnitAI::AI_poach()
{
	CvPlot* pLoopPlot;
	int iX, iY;

	int iBestPoachValue = 0;
	CvPlot* pBestPoachPlot = NULL;
	TeamTypes eBestPoachTeam = NO_TEAM;

	if (!GC.getGameINLINE().isOption(GAMEOPTION_AGGRESSIVE_AI))
	{
		return false;
	}

	if (GET_TEAM(getTeam()).getNumMembers() > 1)
	{
		return false;
	}

	int iNoPoachRoll = GET_PLAYER(getOwnerINLINE()).AI_totalUnitAIs(UNITAI_WORKER);
	iNoPoachRoll += GET_PLAYER(getOwnerINLINE()).getNumCities();
	iNoPoachRoll = std::max(0, (iNoPoachRoll - 1) / 2);
	if (GC.getGameINLINE().getSorenRandNum(iNoPoachRoll, "AI Poach") > 0)
	{
		return false;
	}

	if (GET_TEAM(getTeam()).getAnyWarPlanCount(true) > 0)
	{
		return false;
	}

	FAssert(canAttack());



	int iRange = 1;
	//Look for a unit which is non-combat
	//and has a capture unit type
	for (iX = -iRange; iX <= iRange; iX++)
	{
		for (iY = -iRange; iY <= iRange; iY++)
		{
			if (iX != 0 && iY != 0)
			{
				pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
				if ((pLoopPlot != NULL) && (pLoopPlot->getTeam() != getTeam()) && pLoopPlot->isVisible(getTeam(), false))
				{
					int iPoachCount = 0;
					int iDefenderCount = 0;
					CvUnit* pPoachUnit = NULL;
					CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
					while (pUnitNode != NULL)
					{
						CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
						if ((pLoopUnit->getTeam() != getTeam())
							&& GET_TEAM(getTeam()).canDeclareWar(pLoopUnit->getTeam()))
						{
							if (!pLoopUnit->canDefend())
							{
								if (pLoopUnit->getCaptureUnitType(getCivilizationType()) != NO_UNIT)
								{
									iPoachCount++;
									pPoachUnit = pLoopUnit;
								}
							}
							else
							{
								iDefenderCount++;
							}
						}
					}

					if (pPoachUnit != NULL)
					{
						if (iDefenderCount == 0)
						{
							int iValue = iPoachCount * 100;
							iValue -= iNoPoachRoll * 25;
							if (iValue > iBestPoachValue)
							{
								iBestPoachValue = iValue;
								pBestPoachPlot = pLoopPlot;
								eBestPoachTeam = pPoachUnit->getTeam();
							}
						}
					}
				}
			}
		}
	}

	if (pBestPoachPlot != NULL)
	{
		//No war roll.
		if (!GET_TEAM(getTeam()).AI_performNoWarRolls(eBestPoachTeam))
		{
			GET_TEAM(getTeam()).declareWar(eBestPoachTeam, true, WARPLAN_LIMITED);

			FAssert(!atPlot(pBestPoachPlot));
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPoachPlot->getX_INLINE(), pBestPoachPlot->getY_INLINE(), MOVE_DIRECT_ATTACK);
			return true;
		}

	}

	return false;
}

bool CvUnitAI::AI_choke(int iRange)
{
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;
	for (int iX = -iRange; iX <= iRange; iX++)
	{
		for (int iY = -iRange; iY <= iRange; iY++)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);
			if (pLoopPlot != NULL)
			{
				if (isEnemy(pLoopPlot->getTeam()))
				{
					CvCity* pWorkingCity = pLoopPlot->getWorkingCity();
					if ((pWorkingCity != NULL) && (pWorkingCity->getTeam() == pLoopPlot->getTeam()))
					{
						int iValue = -15;
						if (pLoopPlot->getBonusType(getTeam()) != NO_BONUS)
						{
							iValue += GET_PLAYER(pLoopPlot->getOwnerINLINE()).AI_bonusVal(pLoopPlot->getBonusType(), 0);
						}

						iValue += pLoopPlot->getYield(YIELD_PRODUCTION) * 10;
						iValue += pLoopPlot->getYield(YIELD_FOOD) * 10;
						iValue += pLoopPlot->getYield(YIELD_COMMERCE) * 5;

						if (noDefensiveBonus())
						{
							iValue *= std::max(0, ((baseCombatStr() * 120) - GC.getGame().getBestLandUnitCombat()));
						}
						else
						{
							iValue *= pLoopPlot->defenseModifier(getTeam(), false);
						}

						if (iValue > 0)
						{
							iValue *= 10;

							iValue /= std::max(1, (pLoopPlot->getNumDefenders(getOwnerINLINE()) + ((pLoopPlot == plot()) ? 0 : 1)));

							if (generatePath(pLoopPlot))
							{
								pBestPlot = getPathEndTurnPlot();
								iBestValue = iValue;
							}
						}
					}
				}
			}
		}
	}
	if (pBestPlot != NULL)
	{
		if (atPlot(pBestPlot))
		{
			getGroup()->pushMission(MISSION_SKIP);
			return true;
		}
		else
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX(), pBestPlot->getY());
			return true;
		}
	}



	return false;
}

bool CvUnitAI::AI_solveBlockageProblem(CvPlot* pDestPlot, bool bDeclareWar)
{
	FAssert(pDestPlot != NULL);


	if (pDestPlot != NULL)
	{
		FAStarNode* pStepNode;

		CvPlot* pSourcePlot = plot();

		if (gDLL->getFAStarIFace()->GeneratePath(&GC.getStepFinder(), pSourcePlot->getX_INLINE(), pSourcePlot->getY_INLINE(), pDestPlot->getX_INLINE(), pDestPlot->getY_INLINE(), false, 0, true))
		{
			pStepNode = gDLL->getFAStarIFace()->GetLastNode(&GC.getStepFinder());

			while (pStepNode != NULL)
			{
				CvPlot* pStepPlot = GC.getMapINLINE().plotSorenINLINE(pStepNode->m_iX, pStepNode->m_iY);
				if (canMoveOrAttackInto(pStepPlot) && generatePath(pStepPlot, 0, true))
				{
					if (bDeclareWar && pStepNode->m_pPrev != NULL)
					{
						CvPlot* pPlot = GC.getMapINLINE().plotSorenINLINE(pStepNode->m_pPrev->m_iX, pStepNode->m_pPrev->m_iY);
						if (pPlot->getTeam() != NO_TEAM)
						{
							if (!canMoveInto(pPlot, true, true))
							{
								if (!isPotentialEnemy(pPlot->getTeam(), pPlot))
								{
									CvTeamAI& kTeam = GET_TEAM(getTeam());
									if (kTeam.canDeclareWar(pPlot->getTeam()))
									{
										WarPlanTypes eWarPlan = WARPLAN_LIMITED;
										WarPlanTypes eExistingWarPlan = kTeam.AI_getWarPlan(pDestPlot->getTeam());
										if (eExistingWarPlan != NO_WARPLAN)
										{
											if ((eExistingWarPlan == WARPLAN_TOTAL) || (eExistingWarPlan == WARPLAN_PREPARING_TOTAL))
											{
												eWarPlan = WARPLAN_TOTAL;
											}

											if (!kTeam.isAtWar(pDestPlot->getTeam()))
											{
												kTeam.AI_setWarPlan(pDestPlot->getTeam(), NO_WARPLAN);
											}
										}
										kTeam.AI_setWarPlan(pPlot->getTeam(), eWarPlan, true);
										return (AI_targetCity());
									}
								}
							}
						}
					}
					if (pStepPlot->isVisibleEnemyUnit(this))
					{
						FAssert(canAttack());
						CvPlot* pBestPlot = pStepPlot;
						//To prevent puppeteering attempt to barge through
						//if quite close
						if (getPathLastNode()->m_iData2 > 3)
						{
							pBestPlot = getPathEndTurnPlot();
						}

						FAssert(!atPlot(pBestPlot));
						getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_DIRECT_ATTACK);
						return true;
					}
				}
				pStepNode = pStepNode->m_pParent;
			}
		}
	}

	return false;
}

int CvUnitAI::AI_calculatePlotWorkersNeeded(CvPlot* pPlot, BuildTypes eBuild)
{
	int iBuildTime = pPlot->getBuildTime(eBuild) - pPlot->getBuildProgress(eBuild);
	int iWorkRate = workRate(true);

	if (iWorkRate <= 0)
	{
		FAssert(false);
		return 1;
	}
	int iTurns = iBuildTime / iWorkRate;

	if (iBuildTime > (iTurns * iWorkRate))
	{
		iTurns++;
	}

	int iNeeded = std::max(1, (iTurns + 2) / 3);

	if (pPlot->getBonusType() != NO_BONUS)
	{
		iNeeded *= 2;
	}
	return iNeeded;

}

bool CvUnitAI::AI_canGroupWithAIType(UnitAITypes eUnitAI) const
{
	if (eUnitAI != AI_getUnitAIType())
	{
		switch (eUnitAI)
		{
		case (UNITAI_ATTACK_CITY):
			if (plot()->isCity() && (GC.getGame().getGameTurn() - plot()->getPlotCity()->getGameTurnAcquired()) <= 1)
			{
				return false;
			}
			break;
/*************************************************************************************************/
/**	BETTER AI (first city) Sephi                                          		                **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

		case (UNITAI_SETTLE):
            if (GC.getGame().getGameTurn()<10)
            {
                return false;
            }
            break;
		case (UNITAI_BARBSMASHER):
            if (AI_getUnitAIType()!=UNITAI_BARBSMASHER)
            {
                return false;
            }
            break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

		default:
			break;
		}
	}

	return true;
}



bool CvUnitAI::AI_allowGroup(const CvUnit* pUnit, UnitAITypes eUnitAI) const
{
	CvSelectionGroup* pGroup = pUnit->getGroup();
	CvPlot* pPlot = pUnit->plot();

/*************************************************************************************************/
/**	BETTER AI (Group with Conquest stack) Sephi                                          		**/
/*************************************************************************************************/
    CvUnit* tempUnit=getGroup()->getHeadUnit();
    if (tempUnit!=NULL)
    {
		if (tempUnit->AI_getGroupflag()==GROUPFLAG_CONQUEST || tempUnit->AI_getUnitAIType()==UNITAI_HERO)
        {

            if (pUnit == this)
            {
                return false;
            }

            if (!pUnit->isGroupHead())
            {
                return false;
            }

            if (pGroup == getGroup())
            {
                return false;
            }

            if (pUnit->isCargo())
            {
                return false;
            }

        //FfH: Added by Kael 08/18/2008
            if ((plot() != pPlot) && (pPlot->isVisibleEnemyUnit((PlayerTypes)getOwnerINLINE())))
            {
                return false;
            }
        //FfH: End Add

            switch (pGroup->AI_getMissionAIType())
            {
            case MISSIONAI_GUARD_CITY:
                // do not join groups that are guarding cities
                // intentional fallthrough
            case MISSIONAI_LOAD_SETTLER:
            case MISSIONAI_LOAD_ASSAULT:
            case MISSIONAI_LOAD_SPECIAL:
                // do not join groups that are loading into transports (we might not fit and get stuck in loop forever)
                return false;
                break;
            default:
                break;
            }

            if (!canJoinGroup(pPlot, pGroup))
            {
                return false;
            }

            if (eUnitAI == UNITAI_ASSAULT_SEA)
            {
                if (!pGroup->hasCargo())
                {
                    return false;
                }
            }

            if (pUnit->getInvisibleType() != NO_INVISIBLE)
            {
                if (getInvisibleType() == NO_INVISIBLE)
                {
                    return false;
                }
            }

			if (tempUnit->AI_getGroupflag()==GROUPFLAG_CONQUEST)
			{
				if (!pUnit->getGroup()->getHeadUnit())
				{
					return false;
				}
				else if (pUnit->getGroup()->getHeadUnit()->AI_getGroupflag()!=GROUPFLAG_CONQUEST)
				{
					return false;
				}
			}

            return true;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	if (pUnit == this)
	{
		return false;
	}

	if (!pUnit->isGroupHead())
	{
		return false;
	}

	if (pGroup == getGroup())
	{
		return false;
	}

	if (pUnit->isCargo())
	{
		return false;
	}

	if (pUnit->AI_getUnitAIType() != eUnitAI)
	{
		return false;
	}

//FfH: Added by Kael 08/18/2008
	if ((plot() != pPlot) && (pPlot->isVisibleEnemyUnit((PlayerTypes)getOwnerINLINE())))
	{
		return false;
	}
//FfH: End Add

	switch (pGroup->AI_getMissionAIType())
	{
	case MISSIONAI_GUARD_CITY:
		// do not join groups that are guarding cities
		// intentional fallthrough
	case MISSIONAI_LOAD_SETTLER:
	case MISSIONAI_LOAD_ASSAULT:
	case MISSIONAI_LOAD_SPECIAL:
		// do not join groups that are loading into transports (we might not fit and get stuck in loop forever)
		return false;
		break;
	default:
		break;
	}

	if (pGroup->getActivityType() == ACTIVITY_HEAL)
	{
		// do not attempt to join groups which are healing this turn
		// (healing is cleared every turn for automated groups, so we know we pushed a heal this turn)
		return false;
	}

	if (!canJoinGroup(pPlot, pGroup))
	{
		return false;
	}

	if (eUnitAI == UNITAI_SETTLE)
	{
		if (GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pPlot, 3) > 0)
		{
			return false;
		}
	}
	else if (eUnitAI == UNITAI_ASSAULT_SEA)
	{
		if (!pGroup->hasCargo())
		{
			return false;
		}
	}

	if ((getGroup()->getHeadUnitAI() == UNITAI_CITY_DEFENSE))
	{
		if (plot()->isCity() && (plot()->getTeam() == getTeam()) && plot()->getBestDefender(getOwnerINLINE())->getGroup() == getGroup())
		{
			return false;
		}
	}

	if (plot()->getOwnerINLINE() == getOwnerINLINE())
	{
		CvPlot* pTargetPlot = pGroup->AI_getMissionAIPlot();

		if (pTargetPlot != NULL)
		{
			if (pTargetPlot->isOwned())
			{
				if (isPotentialEnemy(pTargetPlot->getTeam(), pTargetPlot))
				{
					//Do not join groups which have debarked on an offensive mission
					return false;
				}
			}
		}
	}

	if (pUnit->getInvisibleType() != NO_INVISIBLE)
	{
		if (getInvisibleType() == NO_INVISIBLE)
		{
			return false;
		}
	}

	return true;
}


void CvUnitAI::read(FDataStreamBase* pStream)
{
	CvUnit::read(pStream);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iBirthmark);
/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                          		**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    pStream->Read(&m_bAllowedPatrol);
    pStream->Read(&m_bAllowedExplore);
    pStream->Read(&m_bPermanentSummon);
    pStream->Read(&m_bSuicideSummon);

    pStream->Read(&m_iGroupflag);
    pStream->Read(&m_iOriginX);
    pStream->Read(&m_iOriginY);
    pStream->Read(&m_iPromotionSpecialization);
    pStream->Read(&m_iWasAttacked);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	pStream->Read((int*)&m_eUnitAIType);
	pStream->Read(&m_iAutomatedAbortTurn);
}


void CvUnitAI::write(FDataStreamBase* pStream)
{
	CvUnit::write(pStream);

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iBirthmark);
/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                          		**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    pStream->Write(m_bAllowedPatrol);
    pStream->Write(m_bAllowedExplore);
    pStream->Write(m_bPermanentSummon);
    pStream->Write(m_bSuicideSummon);

    pStream->Write(m_iGroupflag);
    pStream->Write(m_iOriginX);
    pStream->Write(m_iOriginY);
	pStream->Write(m_iPromotionSpecialization);
    pStream->Write(m_iWasAttacked);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	pStream->Write(m_eUnitAIType);
	pStream->Write(m_iAutomatedAbortTurn);
}

// Private Functions...

//FfH: Added by Kael 09/19/2007
void CvUnitAI::AI_summonAttackMove()
{
	PROFILE_FUNC();

    if(getDuration()>0)
    {
        if (AI_anyAttack(getDuration()*baseMoves(), 0))
        {
            return;
        }
    }
    else
    {
        if (AI_anyAttack(baseMoves(), 0))
        {
            return;
        }
    }

	if (AI_patrol())
	{
		return;
	}

	if (AI_retreatToCity())
	{
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}
//FfH: End Add

/*************************************************************************************************/
/**	BETTER AI (New Functions) Sephi                                          					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

CvPlot* CvUnitAI::getOriginPlot() const
{
   if (m_iOriginX==-1 && m_iOriginY==-1)
   {
        return NULL;
   }
   return plotXY(m_iOriginX,m_iOriginY,0,0);
}

void CvUnitAI::setOriginPlot(CvPlot* newplot)
{
    if (newplot==NULL)
    {
        m_iOriginX=-1;
        m_iOriginY=-1;
    }
    else
    {
        m_iOriginX=newplot->getX_INLINE();
        m_iOriginY=newplot->getY_INLINE();
    }
}

int CvUnitAI::AI_getPromotionSpecialization() const
{
    return m_iPromotionSpecialization;
}

void CvUnitAI::AI_setPromotionSpecialization(int newSpecial)
{
    m_iPromotionSpecialization=newSpecial;
}


int CvUnitAI::AI_getGroupflag() const
{
    return m_iGroupflag;
}

void CvUnitAI::AI_setGroupflag(int newflag)
{
    m_iGroupflag=newflag;
}
// Chooses the Groupflag for AI Units in CvUnit::DoTurn()
void CvUnitAI::AI_chooseGroupflag()
{
    if(AI_getGroupflag()!=GROUPFLAG_NONE || getAIGroup()!=NULL)
    {
        return;
    }
    //Don't Choose a Groupflag if we haven't already build a city
    if(GET_PLAYER(getOwnerINLINE()).getNumCities()==0)
    {
        return;
    }

    if (isHiddenNationality())
    {
        AI_setGroupflag(GROUPFLAG_HNGROUP);
        return;
    }

    switch (AI_getUnitAIType())
    {
        case UNITAI_MAGE:
            AI_setGroupflag(GROUPFLAG_PERMDEFENSE_NEW);
            return;
            break;
        case UNITAI_HERO:
            AI_setGroupflag(GROUPFLAG_HERO);
            return;
            break;
        case UNITAI_SETTLER_SEA:
            AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE_NEW);
            return;
            break;
		case UNITAI_ANIMAL:
			AI_setGroupflag(GROUPFLAG_DEFENSE_NEW);
			return;
			break;
        default:
            break;
    }

    switch (AI_getUnitAIType())
    {
        case UNITAI_FEASTING:
            AI_setGroupflag(GROUPFLAG_FEASTING);
            return;
            break;
        default:
            break;
    }

    // move to subfunction later
    if(GC.getLeaderHeadInfo(GET_PLAYER(getOwnerINLINE()).getLeaderType()).isReligionVictory())
    {
//        PromotionTypes iPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_INQUISITOR");
//        if (iPromotion!=NO_PROMOTION && isHasPromotion(iPromotion))
//  Must use UNITAI, since PromotionSelection peneds on Groupflag
        if(AI_getUnitAIType()==UNITAI_MEDIC)
        {
            if ((GET_PLAYER(getOwnerINLINE()).countGroupFlagUnits(GROUPFLAG_INQUISITION)-2)<(GET_PLAYER(getOwnerINLINE()).getNumCities()/5))
            {
                AI_setGroupflag(GROUPFLAG_INQUISITION);
                return;
            }
        }
    }

	//Svartalfar Kidnap
	CivilizationTypes iSvartal=(CivilizationTypes)GC.getInfoTypeForString("CIVILIZATION_SVARTALFAR");
	if (iSvartal!=NO_CIVILIZATION && getCivilizationType()==iSvartal)
	{
		UnitTypes iRanger=(UnitTypes)GC.getInfoTypeForString("UNIT_RANGER");
		if(iRanger!=NO_UNIT && getUnitType()==iRanger)
		{
			if(GET_PLAYER(getOwnerINLINE()).countGroupFlagUnits(GROUPFLAG_SVARTALFAR_KIDNAP)==0)
			{
                AI_setGroupflag(GROUPFLAG_SVARTALFAR_KIDNAP);
                return;
            }
        }
    }

	//Esus Missionary
	ReligionTypes iEsus =(ReligionTypes)GC.getInfoTypeForString("RELIGION_COUNCIL_OF_ESUS");
	if (iEsus!=NO_RELIGION && GET_PLAYER(getOwnerINLINE()).getStateReligion()==iEsus)
	{
		if(GET_PLAYER(getOwnerINLINE()).getFavoriteReligion()==iEsus)
		{
			if(GC.getUnitInfo(getUnitType()).getTier()==3 && getReligion()==iEsus)
			{
				int iNumMiss=GET_PLAYER(getOwnerINLINE()).countGroupFlagUnits(GROUPFLAG_ESUS_MISSIONARY);
				if(baseMoves()>1 || iNumMiss==0)
				{
					if(iNumMiss<3)
					{
						AI_setGroupflag(GROUPFLAG_ESUS_MISSIONARY);
						return;
					}
				}
			}
        }
    }

 //   if (isUnitAllowedDefenseGroup())
	if(1<2)
    {
        AI_setGroupflag(GROUPFLAG_DEFENSE_NEW);
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
}

// Returns true if a mission was pushed...
bool CvUnitAI::AI_groupheal(int iDamagePercent, int iMaxPath)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pEntityNode;
	std::vector<CvUnit*> aeDamagedUnits;
	CvSelectionGroup* pGroup;
	CvUnit* pLoopUnit;
	int iTotalDamage;
	int iTotalHitpoints;
	int iHurtUnitCount;

	if (plot()->getFeatureType() != NO_FEATURE)
	{
		if (GC.getFeatureInfo(plot()->getFeatureType()).getTurnDamage() != 0)
		{
			//Pass through
			//(actively seeking a safe spot may result in unit getting stuck)
            if (!plot()->isCity() && AI_retreatToCity(false,false,1))
            {
                return true;
            }

			return false;
		}
	}

//FfH: Added by Kael 10/01/2007 (so the AI won't try to sit and heal in areas where they cant heal)
    if (healRate(plot()) <= 0)
    {
        if (!plot()->isCity() && AI_retreatToCity(false,false,1))
        {
            return true;
        }

        return false;
    }
//FfH: End Add

	pGroup = getGroup();

	if (iDamagePercent == 0)
	{
	    iDamagePercent = 10;
	}


	iMaxPath = std::min(iMaxPath, 1);

	pEntityNode = getGroup()->headUnitNode();

    iTotalDamage = 0;
    iTotalHitpoints = 0;
    iHurtUnitCount = 0;
	while (pEntityNode != NULL)
	{
		pLoopUnit = ::getUnit(pEntityNode->m_data);
		FAssert(pLoopUnit != NULL);
		pEntityNode = pGroup->nextUnitNode(pEntityNode);

		int iDamageThreshold = (pLoopUnit->maxHitPoints() * iDamagePercent) / 100;

		if (pLoopUnit->getDamage() > iDamageThreshold)
		{
		    iHurtUnitCount++;
		}
		iTotalDamage += pLoopUnit->getDamage();
		iTotalHitpoints += pLoopUnit->maxHitPoints();
	}
	if (iHurtUnitCount*3>pGroup->getNumUnits())
	{
        if (!plot()->isCity() && AI_retreatToCity(false,false,1))
        {
            return true;
        }
        if(canHeal(plot()))
        {
            pGroup->pushMission(MISSION_HEAL);
            return true;
        }
	}
	return false;
}

void CvUnitAI::AI_barbsmashermove()
{
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	CvCity* pLoopCity;
	int iLoop;
	int iSearchRange;
	int iPathTurns;
	int iValue;
	int iBestValue;
	int iDX, iDY;

    int iOddsThreshold=30;
    int iMinStack=1;
    int iRange=20;

    bool bFollow=false;
	FAssert(canMove());

	iBestValue = 0;
	pBestPlot = NULL;

    switch (AI_getUnitAIType())
    {
        case UNITAI_HERO:
        case UNITAI_WARWIZARD:
            break;
        default:
            AI_setUnitAIType(UNITAI_BARBSMASHER);
            break;
    }

	//Look for Heros on Same plot
    if (getGroup()->getNumUnits()>1)
    {
        CLLNode<IDInfo>* pUnitNode;
        CvUnit* pLoopUnit;
        pUnitNode = plot()->headUnitNode();
        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = plot()->nextUnitNode(pUnitNode);
            if (pLoopUnit!=NULL && pLoopUnit->getOwnerINLINE()==getOwnerINLINE() && pLoopUnit->getGroup()!=getGroup())
            {
				if(pLoopUnit->AI_getUnitAIType()==UNITAI_HERO && pLoopUnit->AI_getGroupflag()==GROUPFLAG_DEFENSE_NEW)
                {
					if(pLoopUnit->getGroup()->getActivityType()==ACTIVITY_AWAKE)
					{
						pLoopUnit->joinGroup(getGroup());
						if(!getGroup()->canAllMove())
						{
							finishMoves();
							getGroup()->pushMission(MISSION_SKIP);
							return;
						}
					}
                }
            }
        }
    }

	//If player is in ConquestMode, seperate
    if (GET_PLAYER(getOwnerINLINE()).isConquestMode())
    {
		CvSelectionGroup* pOldSelectionGroup = getGroup();

        CLLNode<IDInfo>* pUnitNode;
        CvUnit* pLoopUnit;
        pUnitNode = plot()->headUnitNode();
        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = plot()->nextUnitNode(pUnitNode);
            if (pLoopUnit!=NULL && pLoopUnit->getOwnerINLINE()==getOwnerINLINE())
            {
                if(pLoopUnit->AI_getGroupflag()==GROUPFLAG_DEFENSE_NEW)
                {
                    pLoopUnit->joinGroup(NULL);
                    pLoopUnit->AI_setGroupflag(GROUPFLAG_CONQUEST);
                    pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
                }
            }
        }
		pOldSelectionGroup->pushMission(MISSION_SKIP);
        return;
    }

    if (getGroup()->getNumUnits()==1)
    {
        if (AI_group(UNITAI_BARBSMASHER, -1, -1, -1, false, false, false, 20, false))
        {
			//TEMPFIX to stop WoC
            if (getGroup()->getLengthMissionQueue()==0) //Make sure we push a Mission if joining a group failed
            {
                getGroup()->pushMission(MISSION_SKIP);
            }
        }
    }

    if (getGroup()->getNumUnits()==1)
    {
        iBestValue=1;
        CvUnit* pBestUnit = NULL;

        for(CvSelectionGroup* pLoopSelectionGroup = GET_PLAYER(getOwnerINLINE()).firstSelectionGroup(&iLoop); pLoopSelectionGroup != NULL; pLoopSelectionGroup = GET_PLAYER(getOwnerINLINE()).nextSelectionGroup(&iLoop))
        {
            if (pLoopSelectionGroup->getHeadUnit() != NULL)
            {
                if (pLoopSelectionGroup->getHeadUnit()->AI_getGroupflag()==GROUPFLAG_DEFENSE_NEW)
                {
                    if (pLoopSelectionGroup!=getGroup())
                    {
                        pLoopPlot = pLoopSelectionGroup->getHeadUnit()->plot();
                        if (AI_plotValid(pLoopPlot))
                        {
                            if (!isEnemy(pLoopPlot->getTeam()))
                            {
                                if (AI_allowGroup(pLoopSelectionGroup->getHeadUnit(), UNITAI_UNKNOWN))
                                {
                                    if (!(pLoopPlot->isVisibleEnemyUnit(this)))
                                    {
                                        if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                        {
                                            iValue = pLoopSelectionGroup->getNumUnits();
                                            if (iValue >= iBestValue)
                                            {
                                                iBestValue = iValue;
                                                pBestUnit = pLoopSelectionGroup->getHeadUnit();
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
        if (pBestUnit != NULL)
        {
            if (atPlot(pBestUnit->plot()))
            {
                joinGroup(pBestUnit->getGroup());
                if (getGroup()->getLengthMissionQueue()==0) //Make sure we push a Mission if joining a group failed
                {
                    getGroup()->pushMission(MISSION_SKIP);
                }
                return;
            }
            else
            {
                //getGroup()->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(), pBestUnit->getID(), 0, false, false, MISSIONAI_GROUP, NULL, pBestUnit);
				getGroup()->pushMission(MISSION_MOVE_TO, pBestUnit->getX_INLINE(), pBestUnit->getY_INLINE(), MOVE_DIRECT_ATTACK);
                return;
            }
        }
    }

	//When not at war, maybe City needs defense?
	//Are we in A City that needs Defense?
	if (plot()->isCity() && GET_TEAM(getTeam()).getAtWarCount(true)==0)
	{
	    if(plot()->getOwnerINLINE()==getOwnerINLINE())
	    {
	        if (plot()->getPlotCity()->AI_neededPermDefense()>0)
	        {
	            //Split so we can join the Defense

                bool bWaitAfterSplit=false;

                if(getGroup()->getNumUnits()==1)
                    bWaitAfterSplit=true;

                CLLNode<IDInfo>* pUnitNode;
                CvUnit* pLoopUnit;
                pUnitNode = getGroup()->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = getGroup()->nextUnitNode(pUnitNode);
                    if (pLoopUnit)
                    {
//                        if (pLoopUnit->isUnitAllowedPermDefense())
						if(1<2)
                        {
                            if (!(pLoopUnit->getGroup()->getHeadUnit()==pLoopUnit) || pLoopUnit->getGroup()->getNumUnits()==1)
                            {
                                if (AI_getUnitAIType()==UNITAI_BARBSMASHER)
                                {
                                    pLoopUnit->joinGroup(NULL);
                                    pLoopUnit->AI_setGroupflag(GROUPFLAG_PERMDEFENSE_NEW);
                                    pLoopUnit->AI_setUnitAIType(UNITAI_CITY_DEFENSE);
                                    pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
                                }
                            }
                        }
                    }
                }
                if(bWaitAfterSplit)
                {
                    getGroup()->pushMission(MISSION_SKIP);
                    return;
                }
	        }
	    }
	}

    if (AI_groupheal(30))
    {
        return;
    }

	pBestPlot=NULL;
    iSearchRange=20;
	iBestValue=-100;
	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if ((AI_plotValid(pLoopPlot)) && pLoopPlot->getOwnerINLINE()==getOwnerINLINE())
				{
					if (pLoopPlot->isVisibleEnemyUnit(this))
					{
                        if (!atPlot(pLoopPlot) && ((bFollow) ? canMoveInto(pLoopPlot, true) : (generatePath(pLoopPlot, 0, true, &iPathTurns) && (iPathTurns <= iRange))))
                        {
                            if (pLoopPlot->getNumVisibleEnemyDefenders(this) >= iMinStack)
                            {
								int iOurStrength=getGroup()->AI_GroupPower(pLoopPlot,false);
								int iTheirStrength=GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(pLoopPlot,0,true,false);
								if (iOurStrength>(iTheirStrength*1.2) && (iTheirStrength>0))
								{
									iValue = getGroup()->AI_attackOdds(pLoopPlot, true);

                                    iValue *=(100/(1+iPathTurns));
                                    iValue *=(3+pLoopPlot->getNumVisibleEnemyDefenders(this));
                                    if (iValue > iBestValue)
                                    {
                                        iBestValue = iValue;
                                        pBestPlot = ((bFollow) ? pLoopPlot : getPathEndTurnPlot());
                                        FAssert(!atPlot(pBestPlot));
                                    }
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
        FAssert(!atPlot(pBestPlot));
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), ((bFollow) ? MOVE_DIRECT_ATTACK : 0));
        return;
    }

//look for barb cities
    iSearchRange=5;

	iBestValue = 0;
	pBestPlot = NULL;

    for (pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).nextCity(&iLoop))
    {
        if (pLoopCity)
        {
            for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
            {
                for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
                {
                    pLoopPlot = plotXY(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), iDX, iDY);
                    if (pLoopPlot)
                    {
                        if (AI_plotValid(pLoopPlot))
                        {
                            if(pLoopPlot->isCity())
                            {
                                if (GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam()))
                                {
                                    // Avoid hopeless causes
                                    CvUnit* pBestDefender = pLoopCity->plot()->getBestDefender(pLoopCity->getOwnerINLINE(), getOwnerINLINE(), this);

                                    if (pBestDefender == NULL ||
                                        !(pBestDefender->getFear()>=100) ||
                                        isImmuneToFear() ||
                                        baseCombatStr() > pBestDefender->baseCombatStrDefense())
                                    {
                                        if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                        {
											int iOurStrength=getGroup()->AI_GroupPower(pLoopPlot,false);
											int iTheirStrength=GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(pLoopPlot,0,true,false);

											if (pBestDefender!=NULL && getFear()>=100 && pBestDefender->isBarbarian())
												iTheirStrength*=10;

											if (getGroup()->getNumUnits()>=pLoopPlot->getNumVisibleEnemyDefenders(this)*4)
											{
												iOurStrength=((int)(iOurStrength*1.7));
											}

											if (iOurStrength>(iTheirStrength*1.2)  && (iTheirStrength>0))
											{
												iValue=(iOurStrength*100/iTheirStrength)/(1+iPathTurns);
												if (iValue>iBestValue)
												{
													iBestValue=iValue;
													pBestPlot=pLoopPlot;
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
        }
    }
	if (pBestPlot!=NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO,pBestPlot->getX_INLINE(),pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
        return;
	}


    //Look for Mana nodes to Capture
    if ((getGroup()->getNumUnits()>6) && AI_CaptureManaNodes())
    {
        return;
    }

	//pick up reinforcements
    if (AI_group(UNITAI_BARBSMASHER, -1, -1, -1, false, false, true, 3, true))
	{
		//TEMPFIX to stop WoC
        if (getGroup()->getLengthMissionQueue()==0) //Make sure we push a Mission if joining a group failed
        {
            getGroup()->pushMission(MISSION_SKIP);
        }
	    return;
    }

    if (AI_groupheal(1))
    {
        return;
    }

    if (plot()->getOwnerINLINE()==getOwnerINLINE())
    {
        if (AI_exploreLair())
        {
            return;
        }

        if (AI_goody(1))
        {
            return;
        }

        if (GC.getGameINLINE().getSorenRandNum(100, "CheckUnitforUpgrade")<30)
        {
            if (AI_travelToUpgradeCity())
            {
                return;
            }
        }

        if (AI_patrol())
        {
            return;
        }
    }

	if (!plot()->isCity())
	{
		if (AI_retreatToCity())
		{
			return;
		}
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}
// returns true if a Mission was pushed
bool CvUnitAI::AI_CaptureManaNodes()
{
    if (!GET_TEAM(getTeam()).isAtWar(WILDMANA_TEAM))
    {
        return false;
    }

    CvPlot* pBestplot=NULL;
    int iBestValue=10000;
    int iPathTurns;

    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
    {
        CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if (pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && pLoopPlot->getUnitPower(WILDMANA_PLAYER)>0 && pLoopPlot->getArea()==getArea())
        {
            int iOurStrength=getGroup()->AI_GroupPower(pLoopPlot,false);
            int iTheirStrength=GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(pLoopPlot,0,true,false);

            if (iOurStrength>(iTheirStrength*1.2))
            {
                if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                {
                    if(iPathTurns<iBestValue)
                    {
                        pBestplot=pLoopPlot;
                        iBestValue=iPathTurns;
                    }
                }
            }
		}
	}

	if (pBestplot!=NULL)
	{
        getGroup()->pushMission(MISSION_MOVE_TO,pBestplot->getX_INLINE(),pBestplot->getY_INLINE(),MOVE_DIRECT_ATTACK);
        return true;
    }

    return false;
}

//checks if Player needs Units to pick up Equipment and changes Groupflag of groupmembers if necessary
void CvUnitAI::CheckForEquipment()
{
	bool bNeedMage=false;
	bool bNeedPickup=false;

	int countbooks=0;
	countbooks+=GET_PLAYER(getOwnerINLINE()).getUnitClassCount((UnitClassTypes)GC.getInfoTypeForString("EQUIPMENTCLASS_SPELLBOOK_ALTERATION"));
	countbooks+=GET_PLAYER(getOwnerINLINE()).getUnitClassCount((UnitClassTypes)GC.getInfoTypeForString("EQUIPMENTCLASS_SPELLBOOK_DIVINATION"));
	countbooks+=GET_PLAYER(getOwnerINLINE()).getUnitClassCount((UnitClassTypes)GC.getInfoTypeForString("EQUIPMENTCLASS_SPELLBOOK_ELEMENTALISM"));
	countbooks+=GET_PLAYER(getOwnerINLINE()).getUnitClassCount((UnitClassTypes)GC.getInfoTypeForString("EQUIPMENTCLASS_SPELLBOOK_NECROMANCY"));

	if (countbooks>0)
	{
		bNeedMage=true;
	}

    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
    {
        CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if (pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && pLoopPlot->getNumUnits()==1)
		{
		    CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
			if (pUnitNode!=NULL)
			{
				CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
				if(GC.getUnitInfo(pUnit->getUnitType()).getUnitCaptureClassType()!=NO_UNITCLASS)
				{
					bNeedPickup=true;
				}
			}
		}
	}

    CLLNode<IDInfo>* pUnitNode;
    CvUnit* pLoopUnit;
    pUnitNode = getGroup()->headUnitNode();
    while (pUnitNode != NULL)
    {
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = getGroup()->nextUnitNode(pUnitNode);
        if (pLoopUnit!=NULL)
        {
			if(bNeedMage && pLoopUnit!=this && pLoopUnit->getUnitCombatType()==GC.getUNITCOMBAT_ADEPT())
			{
                pLoopUnit->joinGroup(NULL);
                pLoopUnit->AI_setGroupflag(GROUPFLAG_PICKUP_EQUIPMENT);
                pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
				bNeedMage=false;
            }
			else if(bNeedPickup && pLoopUnit!=this && pLoopUnit->getUnitCombatType()==GC.getUNITCOMBAT_ADEPT())
			{
                pLoopUnit->joinGroup(NULL);
                pLoopUnit->AI_setGroupflag(GROUPFLAG_PICKUP_EQUIPMENT);
                pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
				bNeedPickup=false;
            }
		}
		if (!(bNeedMage || bNeedPickup))
		{
			break;
		}
    }
}

void CvUnitAI::AI_feastingmove()
{
    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::PermDefenseNewMove()
{
/**
    if (AI_getGroupflag()!=GROUPFLAG_PERMDEFENSE_NEW)
    {
        return;
    }
    if (getGroup()->getNumUnits()>1)
    {
        getGroup()->pushMission(MISSION_SKIP);
        joinGroup(NULL);
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
    //Unit in a City that needs Defense?
    if (plot()->isCity() && plot()->getOwnerINLINE()==getOwnerINLINE())
    {
        bool bvalid=false;
        if (AI_getUnitAIType()==UNITAI_CITY_DEFENSE && plot()->getPlotCity()->AI_neededPermDefense(0)>0)
        {
            bvalid=true;
        }

        if (AI_getUnitAIType()==UNITAI_CITY_COUNTER && plot()->getPlotCity()->AI_neededPermDefense(1)>0)
        {
            bvalid=true;
        }

        if (AI_getUnitAIType()==UNITAI_MAGE && plot()->getPlotCity()->AI_neededPermDefense(2)>0)
        {
            bvalid=true;
        }

        if (AI_getUnitAIType()==UNITAI_MEDIC && plot()->getPlotCity()->AI_neededPermDefense(3)>0)
        {
            bvalid=true;
        }

        if (getDomainType() == DOMAIN_AIR && airRange()>0 && plot()->getPlotCity()->AI_neededPermDefense(4)>0)
        {
            bvalid=true;
        }

        if (bvalid)
        {
            CLLNode<IDInfo>* pUnitNode;
            CvUnit* pLoopUnit;
            pUnitNode = plot()->headUnitNode();

            while (pUnitNode != NULL)
            {
                pLoopUnit = ::getUnit(pUnitNode->m_data);
                pUnitNode = plot()->nextUnitNode(pUnitNode);
                if (pLoopUnit)
                {
                    if (pLoopUnit->AI_getGroupflag()==GROUPFLAG_PERMDEFENSE)
                    {
                        AI_setGroupflag(GROUPFLAG_PERMDEFENSE);
                        joinGroup(pLoopUnit->getGroup());
                        getGroup()->pushMission(MISSION_SKIP);
                        return;
                    }
                }
            }
            AI_setGroupflag(GROUPFLAG_PERMDEFENSE);
            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
    }

    //Check for Units specialized in Defense
    bool reinforcement=false;
    CivilizationTypes Civili=GET_PLAYER(getOwnerINLINE()).getCivilizationType();

    if (getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender1())
    {
        reinforcement=true;
    }

    if (getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender2())
    {
        reinforcement=true;
    }

    if (getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender3())
    {
        reinforcement=true;
    }

    if(reinforcement)
    {
        CvUnit* pReplacement=NULL;
        int iSearchRange=8;
        int iDX,iDY;
        CvPlot* pLoopPlot;
        CvPlot* pBestPlot=NULL;
        int iBestValue=10;
        int iPathTurns;
        for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
        {
            for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
            {
                pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                if (pLoopPlot != NULL)
                {
                    if(pLoopPlot->isCity())
                    {
                        if(pLoopPlot->getOwnerINLINE()==getOwnerINLINE())
                        {
                            CLLNode<IDInfo>* pUnitNode;
                            CvUnit* pLoopUnit;
                            pUnitNode = pLoopPlot->headUnitNode();
                            while (pUnitNode != NULL)
                            {
                                pLoopUnit = ::getUnit(pUnitNode->m_data);
                                pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                                if (pLoopUnit!=NULL)
                                {
                                    if(pLoopUnit->AI_getGroupflag()==GROUPFLAG_PERMDEFENSE)
                                    {
                                        if(pLoopUnit->AI_getUnitAIType()==AI_getUnitAIType())
                                        {
                                            if (getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender1())
                                            {
                                                if( (pLoopUnit->getUnitClassType()!=GC.getCivilizationInfo(Civili).getBestDefender1()) &&
                                                    (pLoopUnit->getUnitClassType()!=GC.getCivilizationInfo(Civili).getBestDefender2()) &&
                                                    (pLoopUnit->getUnitClassType()!=GC.getCivilizationInfo(Civili).getBestDefender3()))
                                                {
                                                    if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                                    {
                                                        if(iPathTurns<iBestValue)
                                                        {
                                                            pReplacement=pLoopUnit;
                                                            iBestValue=iPathTurns;
                                                        }
                                                    }
                                                }
                                            }

                                            if (getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender2())
                                            {
                                                if( pLoopUnit->getUnitClassType()!=GC.getCivilizationInfo(Civili).getBestDefender2() &&
                                                    pLoopUnit->getUnitClassType()!=GC.getCivilizationInfo(Civili).getBestDefender3())
                                                {
                                                    if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                                    {
                                                        if(iPathTurns<iBestValue)
                                                        {
                                                            pReplacement=pLoopUnit;
                                                            iBestValue=iPathTurns;
                                                        }
                                                    }
                                                }
                                            }

                                            if (getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender3())
                                            {
                                                if( pLoopUnit->getUnitClassType()!=GC.getCivilizationInfo(Civili).getBestDefender3())
                                                {
                                                    if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                                    {
                                                        if(iPathTurns<iBestValue)
                                                        {
                                                            pReplacement=pLoopUnit;
                                                            iBestValue=iPathTurns;
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
                }
            }
        }
        if(pReplacement!=NULL)
        {
            if (atPlot(pReplacement->plot()))
            {
                AI_setGroupflag(GROUPFLAG_PERMDEFENSE);
                joinGroup(pReplacement->getGroup());
                getGroup()->pushMission(MISSION_SKIP);
                pReplacement->joinGroup(NULL);
                pReplacement->AI_setGroupflag(GROUPFLAG_NONE);
                pReplacement->getGroup()->pushMission(MISSION_SKIP);
                return;
            }
            else
            {
                getGroup()->pushMission(MISSION_MOVE_TO, pReplacement->getX_INLINE(), pReplacement->getY_INLINE(), MOVE_DIRECT_ATTACK);
                return;
            }
        }
        //Look for Cities around
    }
//Look for Cities around that need DefHelp
    int iSearchRange=10;
    int iDX,iDY;
    CvPlot* pLoopPlot;
    CvPlot* pBestPlot=NULL;
    int iValue;
    int iMod=1;
    int iBestValue=0;
    int iPathTurns;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
			    if(pLoopPlot->isCity())
			    {
                    if(pLoopPlot->getOwnerINLINE()==getOwnerINLINE())
                    {
                        bool bvalid=false;
                        if (AI_getUnitAIType()==UNITAI_CITY_DEFENSE && pLoopPlot->getPlotCity()->AI_neededPermDefense(0)>0)
                        {
                            bvalid=true;
                            iMod=pLoopPlot->getPlotCity()->AI_neededPermDefense(0);
                        }

                        if (AI_getUnitAIType()==UNITAI_CITY_COUNTER && pLoopPlot->getPlotCity()->AI_neededPermDefense(1)>0)
                        {
                            bvalid=true;
                            iMod=pLoopPlot->getPlotCity()->AI_neededPermDefense(1);
                        }

                        if (AI_getUnitAIType()==UNITAI_MAGE && pLoopPlot->getPlotCity()->AI_neededPermDefense(2)>0)
                        {
                            bvalid=true;
                            iMod=pLoopPlot->getPlotCity()->AI_neededPermDefense(2);
                        }

                        if (AI_getUnitAIType()==UNITAI_MEDIC && pLoopPlot->getPlotCity()->AI_neededPermDefense(3)>0)
                        {
                            bvalid=true;
                            iMod=pLoopPlot->getPlotCity()->AI_neededPermDefense(3);
                        }
                        if (bvalid)
                        {
                            if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                            {
                                iValue=(iMod*100)/iPathTurns;
                                if (iValue>iBestValue)
                                {
                                    pBestPlot=pLoopPlot;
                                    iBestValue=iValue;
                                }
                            }
                        }
                    }
			    }
			}
		}
	}
    if (pBestPlot!=NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_DIRECT_ATTACK);
        return;
    }
    AI_setGroupflag(GROUPFLAG_NONE);
    if (AI_getUnitAIType()==UNITAI_MAGE)
    {
        AI_setUnitAIType(UNITAI_WARWIZARD);
    }
**/
    getGroup()->pushMission(MISSION_SKIP);
    return;
}


void CvUnitAI::PermDefenseMove()
{
    // Hawks
    if(getDomainType()==DOMAIN_AIR && airRange()>0)
    {
        AI_exploreAirMove();
    }

    if (!plot()->isCity() || (plot()->getOwnerINLINE()!=getOwnerINLINE()))
    {
        AI_setGroupflag(GROUPFLAG_NONE);
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

//archers
    getGroup()->AI_GroupRangeStrike();

	getGroup()->pushMission(MISSION_SKIP);
	return;
}

//returns true if a Mission was pushed
bool CvUnitAI::RejoinPatrolGroup()
{
    int iBestValue=-1;
    CvUnit* pBestUnit=NULL;
	int iLoop;

    for (CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); NULL != pUnit; pUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
    {
        if (pUnit->AI_getGroupflag()==GROUPFLAG_PATROL && AI_canJoinGroup(pUnit->getGroup()))
        {
            if (pUnit->getOriginPlot() && pUnit!=this)
            {
                if (pUnit->getOriginPlot()==getOriginPlot())
                {
					if(pUnit->getGroup()->getNumUnits()>iBestValue)
					{
						pBestUnit=pUnit;
						iBestValue=pUnit->getGroup()->getNumUnits();
					}
				}
			}
		}
	}
	if (pBestUnit!=NULL)
	{
		if (atPlot(pBestUnit->plot()))
		{
			joinGroup(pBestUnit->getGroup());
            if (getGroup()->getLengthMissionQueue()==0) //Make sure we push a Mission if joining a group failed
            {
                getGroup()->pushMission(MISSION_SKIP);
            }
            return true;
		}
		else
		{
			// getGroup()->pushMission(MISSION_MOVE_TO_UNIT, pBestUnit->getOwnerINLINE(),pBestUnit->getID(),0,false,false, MISSIONAI_GROUP, NULL, pBestUnit);
			getGroup()->pushMission(MISSION_MOVE_TO, pBestUnit->getX_INLINE(), pBestUnit->getY_INLINE(), MOVE_DIRECT_ATTACK);
		}
	    return true;
	}
	return false;
}

void CvUnitAI::HNgroupMove()
{
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

//called in CvUnitAI::AI_update
//Look around for equipment, then join Defense/Conqueststack
void CvUnitAI::PickupEquipmentMove()
{
	//is there other equipment we can pick up?
    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
    {
        CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if (pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && pLoopPlot->getNumUnits()==1)
		{
		    CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
			if (pUnitNode!=NULL)
			{
				CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
				if(GC.getUnitInfo(pUnit->getUnitType()).getUnitCaptureClassType()!=NO_UNITCLASS)
				{
					if(!atPlot(pLoopPlot))
					{
				        getGroup()->pushMission(MISSION_MOVE_TO, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), MOVE_DIRECT_ATTACK);
						return;
					}
				}
			}
		}
	}
	AI_setGroupflag(GROUPFLAG_DEFENSE_NEW);
	getGroup()->pushMission(MISSION_SKIP);
	return;
}

void CvUnitAI::SettlergroupMove()
{
    if (!plot()->isCity() || (plot()->getOwnerINLINE()!=getOwnerINLINE()))
    {
        joinGroup(NULL);
        AI_setGroupflag(GROUPFLAG_NONE);
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
  	//Are we in A City that needs Defense?
    else if (plot()->getPlotCity()->AI_neededPermDefense()>0)
    {
        //Split so we can join the Defense

        CLLNode<IDInfo>* pUnitNode;
        CvUnit* pLoopUnit;
        pUnitNode = plot()->headUnitNode();
        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = plot()->nextUnitNode(pUnitNode);
            if (pLoopUnit)
            {
                if (pLoopUnit->getOwnerINLINE()==getOwnerINLINE())
                {
                    if (pLoopUnit->AI_getGroupflag()==GROUPFLAG_SETTLERGROUP)
                    {
                        pLoopUnit->AI_setGroupflag(GROUPFLAG_PERMDEFENSE_NEW);
                        pLoopUnit->AI_setUnitAIType(UNITAI_CITY_DEFENSE);
                        pLoopUnit->joinGroup(NULL);
                        pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
                    }
                }
            }
        }
        return;
    }
    //settler around that we can join?
    else
    {
        CLLNode<IDInfo>* pUnitNode;
        CvUnit* pLoopUnit;
        pUnitNode = plot()->headUnitNode();
        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = plot()->nextUnitNode(pUnitNode);
            if (pLoopUnit)
            {
                if (pLoopUnit->getOwnerINLINE()==getOwnerINLINE())
                {
                    if (pLoopUnit->AI_getUnitAIType()==UNITAI_SETTLE)
                    {
                        joinGroup(NULL);
                        joinGroup(pLoopUnit->getGroup());
                        pLoopUnit->getGroup()->pushMission(MISSION_SKIP);
                    }
                }
            }
        }
        return;
    }
    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_heromove()
{

    if (getUnitClassType()==GC.getDefineINT("UNITCLASS_GOVANNON"))
    {
        if (AI_Govannonmove())
        {
            return;
        }
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
    if (getUnitClassType()==GC.getDefineINT("UNITCLASS_LOKI"))
    {
        if (AI_Lokimove())
        {
            return;
        }
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

    if (getUnitClassType()==GC.getDefineINT("UNITCLASS_RANTINE"))
    {
        if (AI_Rantinemove())
        {
            return;
        }
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

    if (GET_PLAYER(getOwnerINLINE()).isConquestMode())
    {
        AI_setGroupflag(GROUPFLAG_CONQUEST);
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
    else
    {
//        if (GET_PLAYER(getOwnerINLINE()).countGroupFlagUnits(GROUPFLAG_DEFENSE_NEW)>4)
        if (GET_PLAYER(getOwnerINLINE()).countGroupFlagUnits(GROUPFLAG_DEFENSE_NEW)>0)
        {
            AI_setGroupflag(GROUPFLAG_DEFENSE_NEW);
            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
        if (AI_guardCity(true))
        {
            return;
        }
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
	return;
}

bool CvUnitAI::AI_Govannonmove()
{
	return false;
}

bool CvUnitAI::AI_Lokimove()
{
    CvCity* pLoopCity;
    int iLoop;
    int iSearchRange=6;
    int icount=0;
    int iDX, iDY;
    int iPathTurns;
    CvPlot* pLoopPlot;
    if (plot()->isCity())
    {
        if (canCast(GC.getDefineINT("SPELL_DISRUPT"),false))
            cast(GC.getDefineINT("SPELL_DISRUPT"));
    }
    for (pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).nextCity(&iLoop))
    {
        if (pLoopCity)
        {
            for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
            {
                for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
                {
                    pLoopPlot = plotXY(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), iDX, iDY);
                    if (pLoopPlot)
                    {
                        if (AI_plotValid(pLoopPlot))
                        {
                            if(pLoopPlot->isCity() && pLoopPlot->getTeam()!=getTeam())
                            {
                                if (!GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam()))
                                {
                                    if (pLoopPlot->getPlotCity()->getCulture(pLoopPlot->getOwnerINLINE())==0)
                                    {
                                        if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                        {
                                            getGroup()->pushMission(MISSION_MOVE_TO,pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),MOVE_THROUGH_ENEMY);
                                            return true;
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
    return false;
}

bool CvUnitAI::AI_Rantinemove()
{
   if (getGroup()->getNumUnits()<4)
    {
        if (!(plot()->isCity() && plot()->getOwnerINLINE()==getOwnerINLINE()))
        {
            if (AI_retreatToCity())
            {
                return true;
            }
        }
        int iSearchRange=5;
        int icount=0;
        int iDX, iDY;
        CvPlot* pLoopPlot;
        CvUnit* pLoopUnit;
        int iPathTurns;
        CLLNode<IDInfo>* pUnitNode;

        for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
        {
            for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
            {
                pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                if (pLoopPlot != NULL)
                {
                    if (pLoopPlot->getOwnerINLINE()==getOwnerINLINE())
                    {
                        if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                        {
                            pUnitNode = pLoopPlot->headUnitNode();
                            while (pUnitNode != NULL)
                            {
                                pLoopUnit = ::getUnit(pUnitNode->m_data);
                                pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                                if (pLoopUnit)
                                {
                                    if (!(pLoopUnit->getGroup()->getHeadUnit()==pLoopUnit) || pLoopUnit->getGroup()->getNumUnits()==1)
                                    {
                                        if (pLoopUnit->AI_getUnitAIType()==UNITAI_COUNTER && pLoopUnit->AI_getGroupflag()==GROUPFLAG_PATROL)
                                        {
                                            if(pLoopUnit->getGroup()->getHeadUnit()!=pLoopUnit || pLoopUnit->getGroup()->getNumUnits()==1)
                                            {
                                                if(pLoopUnit->atPlot(plot()))
                                                {
                                                    pLoopUnit->joinGroup(NULL);
                                                    pLoopUnit->AI_setGroupflag(GROUPFLAG_NONE);
                                                    pLoopUnit->AI_setUnitAIType(UNITAI_ATTACK);
                                                    pLoopUnit->setOriginPlot(NULL);
                                                    pLoopUnit->joinGroup(getGroup());
                                                    return false;
                                                }
                                                else
                                                {
                                                    //pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO_UNIT, getOwnerINLINE(), getID(), 0, false, false, MISSIONAI_GROUP, NULL, this);
													pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, getX_INLINE(), getY_INLINE(), MOVE_DIRECT_ATTACK);
                                                    return true;
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
		}
	}
	else
	{
        CvCity* pLoopCity;
        int iLoop;
        int iSearchRange=8;
        int icount=0;
        int iDX, iDY;
        int iPathTurns;
        CvPlot* pLoopPlot;
        if (plot()->isCity())
        {
            if (canCast(GC.getDefineINT("SPELL_CONVERT_CITY_RANTINE"),false))
                cast(GC.getDefineINT("SPELL_CONVERT_CITY_RANTINE"));
        }
        for (pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)getOwnerINLINE()).nextCity(&iLoop))
        {
            if (pLoopCity)
            {
                for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
                {
                    for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
                    {
                        pLoopPlot = plotXY(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), iDX, iDY);
                        if (pLoopPlot)
                        {
                            if (AI_plotValid(pLoopPlot))
                            {
                                if(pLoopPlot->isCity() && pLoopPlot->getTeam()!=getTeam())
                                {
                                    if (!GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam()))
                                    {
                                        if (pLoopPlot->getPlotCity()->isBarbarian())
                                        {
                                            if (generatePath(pLoopPlot, 0, true, &iPathTurns))
                                            {
                                                getGroup()->pushMission(MISSION_MOVE_TO,pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),MOVE_THROUGH_ENEMY);
                                                return true;
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
	}
	return false;
}

bool CvUnitAI::AI_promoteMagicUnit()
{
    PromotionTypes MagicTalent=(PromotionTypes)GC.getDefineINT("PROMOTION_CHANNELING1");

    if (!isHasPromotion(MagicTalent))
    {
        return false;
    }

    switch (AI_getGroupflag())
    {

        case GROUPFLAG_PATROL:
        case GROUPFLAG_CONQUEST:
            if (1<2)
            {
                PromotionTypes eBestPromotion;
                eBestPromotion = NO_PROMOTION;
                CyUnit* pyUnit = new CyUnit(this);
                CyArgsList argsList;
                argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                long lResult=-1;
                gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_MagePromotion", argsList.makeFunctionArgs(), &lResult);
                delete pyUnit;	// python fxn must not hold on to this pointer

                eBestPromotion = ((PromotionTypes)lResult);

                if (eBestPromotion != NO_PROMOTION)
                {
                    promote(eBestPromotion, -1);
                    AI_promote();
                }
            }
            return true;
            break;

            case GROUPFLAG_NONE:
                switch (AI_getUnitAIType())
                {
                    case UNITAI_TERRAFORMER:
                    case UNITAI_MANA_UPGRADE:
                        if (1<2)
                        {
                            PromotionTypes eBestPromotion;
                            eBestPromotion = NO_PROMOTION;
                            CyUnit* pyUnit = new CyUnit(this);
                            CyArgsList argsList;
                            argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                            long lResult=-1;
                            gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_MagePromotion", argsList.makeFunctionArgs(), &lResult);
                            delete pyUnit;	// python fxn must not hold on to this pointer

                            eBestPromotion = ((PromotionTypes)lResult);

                            if (eBestPromotion != NO_PROMOTION)
                            {
                                promote(eBestPromotion, -1);
                                AI_promote();
                            }
                        }
                        return true;
                        break;
                    default:
                        break;
                }
    }
    return true;
}

/*************************************************************************************************/
/**	END                                                                  						**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (Mercenaries) Sephi	        								                    	**/
/*************************************************************************************************/
void CvUnitAI::AI_MercenariesHeadHunterMove()
{
    if(GC.getGameINLINE().getMercenariesWarTarget()!=NO_TEAM)
    {
        int iRange=15;
        CvPlot* pLoopPlot;
        int iDX,iDY;
        int iBestvalue=0;
        CvPlot* pBestPlot=NULL;
        int iPathTurns;

        for (iDX = -(iRange); iDX <= iRange; iDX++)
        {
            for (iDY = -(iRange); iDY <= iRange; iDY++)
            {
                pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                if (pLoopPlot && pLoopPlot!=plot())
                {
                    if(pLoopPlot->getArea()==getArea())
                    {
                        if(pLoopPlot->isVisibleEnemyUnit(this))
                        {
                            if(pLoopPlot->getBestDefender(NO_PLAYER,getOwnerINLINE(),this, false, false, false)->getTeam()==GC.getGameINLINE().getMercenariesWarTarget())
                            {
                                if(generatePath(pLoopPlot,0,false,&iPathTurns))
                                {
                                    int iVictimValue=getGroup()->AI_attackOdds(pLoopPlot, true);
                                    iVictimValue*=(2+iPathTurns);

                                    if(iVictimValue>iBestvalue)
                                    {
                                        pBestPlot=getPathEndTurnPlot();
                                        iBestvalue=iVictimValue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

		if (pBestPlot!=NULL)
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
			return;
		}

		//no Target Found yet

    }

    AI_MercenariesMoveToSupport();
}

void CvUnitAI::AI_MercenariesPillagerMove()
{
    if(GC.getGameINLINE().getMercenariesWarTarget()!=NO_TEAM)
    {
        if(plot()->getTeam()==GC.getGameINLINE().getMercenariesWarTarget())
        {
            if (canPillage(plot()))
            {
                getGroup()->pushMission(MISSION_PILLAGE);
                return;
            }
        }
        int iRange=15;
        CvPlot* pLoopPlot;
        int iDX,iDY;
        int iBestvalue=15;
        CvPlot* pBestPlot=NULL;
        int iPathTurns;

        for (iDX = -(iRange); iDX <= iRange; iDX++)
        {
            for (iDY = -(iRange); iDY <= iRange; iDY++)
            {
                pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                if (pLoopPlot && pLoopPlot!=plot())
                {
                    if(pLoopPlot->getArea()==getArea())
                    {
                        if(pLoopPlot->getTeam()==GC.getGameINLINE().getMercenariesWarTarget())
                        {
                            if(canPillage(pLoopPlot))
                            {
                                if(generatePath(pLoopPlot,0,false,&iPathTurns))
                                {
                                    if(iPathTurns<iBestvalue)
                                    {
                                        pBestPlot=getPathEndTurnPlot();
                                        iBestvalue=iPathTurns;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

		if (pBestPlot!=NULL)
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
			return;
		}

		//no Target Found yet

    }

    AI_MercenariesMoveToSupport();
}

void CvUnitAI::AI_MercenariesRaiderMove()
{
    if(plot()->isCity())
    {
        if(getGroup()->getNumUnits()<10)
        {
            bool bUnitJoined=false;
            CLLNode<IDInfo>* pUnitNode;
            CvUnit* pLoopUnit;
            pUnitNode = plot()->headUnitNode();

            while (pUnitNode != NULL)
            {
                pLoopUnit = ::getUnit(pUnitNode->m_data);
                pUnitNode = plot()->nextUnitNode(pUnitNode);
                if (pLoopUnit && pLoopUnit->getGroup()->getNumUnits()==1)
                {
                    if(pLoopUnit->AI_canJoinGroup(getGroup()))
                    {
                        if(pLoopUnit->AI_getGroupflag()==AI_getGroupflag())
                        {
                            pLoopUnit->joinGroup(getGroup());
                            bUnitJoined=true;
                            if(getGroup()->getNumUnits()>=10)
                            {
                                break;
                            }
                        }
                    }
                }
            }
            if (bUnitJoined)
            {
                getGroup()->pushMission(MISSION_SKIP);
                return;
            }
        }
    }

    if(GC.getGameINLINE().getMercenariesWarTarget()!=NO_TEAM)
    {
        CvPlot* pBestPlot=NULL;
        CvCity* pLoopCity;
        int iBestValue=100;
        int iPathTurns;
        int iLoop;
        int iValue;

        for(int iI=0; iI<MAX_CIV_PLAYERS; iI++)
        {
            if(GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).getTeam()==GC.getGameINLINE().getMercenariesSupportTarget())
            {
                pLoopCity = area()->getTargetCity((PlayerTypes)iI);

                if (pLoopCity != NULL && pLoopCity->getTeam()==GC.getGameINLINE().getMercenariesWarTarget())
                {
                    if (!atPlot(pLoopCity->plot()) && generatePath(pLoopCity->plot(), 0, false,&iPathTurns))
                    {
                        if(iPathTurns<iBestValue)
                        {
                            pBestPlot = pLoopCity->plot();
                            iBestValue=iPathTurns;
                        }
                    }
                }
            }
        }

		if (pBestPlot!=NULL)
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
			return;
		}

		iBestValue=0;
        for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
        {
            if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).getTeam()==GC.getGameINLINE().getMercenariesSupportTarget())
            {
				for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
				{
					if (pLoopCity->getArea() == getArea())
					{
						iValue = GET_PLAYER(getOwnerINLINE()).AI_targetCityValue(pLoopCity, true);

						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							pBestPlot = pLoopCity->plot();
						}
					}
				}
			}
		}

		if (pBestPlot!=NULL)
		{
			getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
			return;
		}
    }
    AI_MercenariesMoveToSupport();
}

void CvUnitAI::AI_MercenariesMoveToSupport()
{
    if(GC.getGameINLINE().getMercenariesWarTarget()==NO_TEAM || GC.getGameINLINE().getMercenariesSupportTarget()==NO_TEAM)
    {
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

    CvCity* pLoopCity;
    int iPathTurns;
    int iLoop;
    int iBestValue=100;
    CvPlot* pBestPlot=NULL;
    int iDX, iDY;
    int iRange = 7;
    CvPlot* pLoopPlot;
    if(plot()->isOwned())
    {
        if(plot()->getTeam()==GC.getGameINLINE().getMercenariesSupportTarget())
        {
            for (iDX = -(iRange); iDX <= iRange; iDX++)
            {
                for (iDY = -(iRange); iDY <= iRange; iDY++)
                {
                    pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                    if (pLoopPlot && pLoopPlot->isCity())
                    {
                        if(pLoopPlot->getArea()==getArea() && pLoopPlot->getTeam()==GC.getGameINLINE().getMercenariesSupportTarget())
                        {
                            if(!pLoopPlot->isVisibleEnemyUnit(this))
                            {
                               if(generatePath(pLoopPlot,0,false,&iPathTurns))
                               {
                                    getGroup()->pushMission(MISSION_MOVE_TO, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
                                    return;
                               }
                            }
                        }
                    }
                }
            }
        }
    }

    for(int iI=0; iI<MAX_CIV_PLAYERS; iI++)
    {
        if(GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).getTeam()==GC.getGameINLINE().getMercenariesSupportTarget())
        {
            for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
            {
                if (pLoopCity->getArea()==getArea())
                {
                    if(generatePath(pLoopCity->plot(),0,false,&iPathTurns))
                    {
                        if(iBestValue>iPathTurns)
                        {
                            pBestPlot=pLoopCity->plot();
                            iBestValue=iPathTurns;
                        }
                    }
                }
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
        return;
    }

    getGroup()->pushMission(MISSION_SKIP);
    return;
}

/*************************************************************************************************/
/**			        						END													**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	WILDMANA (new Functions) Sephi                                             					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

void CvUnitAI::AI_manadefenderEffect()
{
    if (getOriginPlot()==NULL)
    {
        return;
    }
//Casting is fun
/**
    for (int iSpell = 0; iSpell < GC.getNumSpellInfos(); iSpell++)
    {
        if (canCast(iSpell, false))
        {
            cast(iSpell);
        }
    }
**/
    int Effectchance=GC.getDefineINT("WILD_MANA_TERRAIN_CHANCE");
    Effectchance*= (100/GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getBarbPercent());
    if((getOriginPlot()!= NULL) && (GC.getGameINLINE().getGameTurn() - getGameTurnCreated()==1))
    {
        int iSearchRange =5;
        int iDX, iDY;
        CvPlot* pLoopPlot;
        for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
        {
            for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
            {
                pLoopPlot= plotXY(getOriginPlot()->getX_INLINE(), getOriginPlot()->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL && !pLoopPlot->isWildmanaImmune())
                {
                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_FIRE"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(300, "Fireitup")<Effectchance)
                        {
                            FeatureTypes iFeature = pLoopPlot->getFeatureType();

                            if ((iFeature == GC.getInfoTypeForString("FEATURE_FOREST")) || (iFeature == GC.getInfoTypeForString("FEATURE_JUNGLE")) || (iFeature == GC.getInfoTypeForString("FEATURE_FOREST_NEW")))
                            {
                                bool bvalid=true;
                                if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT)
                                {
                                    if (!GC.getImprovementInfo(pLoopPlot->getImprovementType()).isPermanent())
                                    {
                                        bvalid=false;

                                    }
                                }
                                if (bvalid)
                                {
                                    pLoopPlot->setImprovementType((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_SMOKE"));
                                }
                            }
                        }
                    }
                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_WATER"))
                    {
                        if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT") && pLoopPlot->getFeatureType()==NO_FEATURE)
                        {
                            pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"), true, true);
                        }
                        if (GC.getGameINLINE().getSorenRandNum(1000, "Wateritup")<Effectchance)
                        {
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"), true, true);
                            }
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_EARTH"))
                    {
                        if (!pLoopPlot->isHills() && GC.getGameINLINE().getSorenRandNum(1000, "Turn to Hills")<Effectchance)
                        {
                            if ((!pLoopPlot->isWater()) && (!pLoopPlot->isPeak()))
                            {
                                pLoopPlot->setPlotType(PLOT_HILLS, true, true);
                            }
                        }

                        if (pLoopPlot->isHills() && (pLoopPlot->getBonusType()==NO_BONUS) && GC.getGameINLINE().getSorenRandNum(100, "Discover Minerals")<200)
                        {
                            int mineralchance=2000/Effectchance;
                            int imineraltyp=GC.getGameINLINE().getSorenRandNum(150, "mineraltype");
                            switch (imineraltyp)
                            {
                            case 0:
                                pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_GOLD"));
                                break;
                            case 1:
                                pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_MITHRIL"));
                                break;
                            case 2:
                                pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_BRONZE"));
                                break;
                            case 3:
                                pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_GEMS"));
                                break;
                            case 4:
                                pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_IRON"));
                                break;
                            default:
                                break;
                            }
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_SUN"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(1000, "Scorchit")<Effectchance)
                        {
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW"))
                            {
                                 pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"), true, true);
                            }
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_ICE"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(1000, "freezeit")<Effectchance)
                        {
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"), true, true);
                            }
                            if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT"))
                            {
                                pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"), true, true);
                            }

                            if (pLoopPlot->isWater())
                            {
                                pLoopPlot->setFeatureType((FeatureTypes)GC.getInfoTypeForString("FEATURE_ICE"));
                            }
                        }
                    }

                    if ((getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_DEATH")) || (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_ENTROPY")))
                    {
                        if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"))
                        {
                            pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT"), true, true);
                        }
                        if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"))
                        {
                            pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH"), true, true);
                        }
                        if (pLoopPlot->getTerrainType()==(TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"))
                        {
                            pLoopPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW"), true, true);
                        }

                        bool bvalid=true;
                        if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT)
                        {
                            if (!GC.getImprovementInfo(pLoopPlot->getImprovementType()).isPermanent())
                            {
                                bvalid=false;
                            }
                        }
                        if (bvalid)
                        {
                            pLoopPlot->setFeatureType((FeatureTypes)GC.getFEATURE_HAUNTED_LANDS());
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_NATURE"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(1000, "Fireitup")<Effectchance)
                        {
                            FeatureTypes iFeature = pLoopPlot->getFeatureType();

                            if ((iFeature == NO_FEATURE) || (iFeature ==GC.getInfoTypeForString("FEATURE_FOREST")) || (iFeature == GC.getInfoTypeForString("FEATURE_JUNGLE")) || (iFeature == GC.getInfoTypeForString("FEATURE_FOREST_NEW")) || (iFeature ==GC.getInfoTypeForString("FEATURE_SCRUB")))
                            {
                                bool bvalid=true;
                                if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT)
                                {
                                    if (!GC.getImprovementInfo(pLoopPlot->getImprovementType()).isPermanent())
                                    {
                                        bvalid=false;
                                    }
                                }
                                if (bvalid &&(GC.getGameINLINE().getSorenRandNum(100, "Oasisscores")<3))
                                {
                                    pLoopPlot->setFeatureType((FeatureTypes)GC.getInfoTypeForString("FEATURE_FOREST_ANCIENT"));
                                }
                            }
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_CREATION"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Fireitup")<Effectchance)
                        {
                            FeatureTypes iFeature = pLoopPlot->getFeatureType();

                            if ((iFeature == NO_FEATURE) || (iFeature ==GC.getInfoTypeForString("FEATURE_FOREST")) || (iFeature == GC.getInfoTypeForString("FEATURE_JUNGLE")) || (iFeature == GC.getInfoTypeForString("FEATURE_FOREST_NEW")) || (iFeature ==GC.getInfoTypeForString("FEATURE_SCRUB")))
                            {
                                bool bvalid=true;
                                if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT)
                                {
                                    if (!GC.getImprovementInfo(pLoopPlot->getImprovementType()).isPermanent())
                                    {
                                        bvalid=false;
                                    }
                                }
                                if (pLoopPlot->isWater() || pLoopPlot->isHills() || pLoopPlot->isPeak())
                                    bvalid=false;
                                if (pLoopPlot->getBonusType()!=NO_BONUS)
                                    bvalid=false;
                                if (bvalid)
                                {
                                    if (GC.getGameINLINE().getSorenRandNum(60, "Oase")<1)
                                    {
                                        if (GC.getInfoTypeForString("FEATURE_OASIS")!=NO_FEATURE)
                                        {
                                            pLoopPlot->setFeatureType((FeatureTypes)GC.getInfoTypeForString("FEATURE_OASIS"));
                                        }
                                    }
                                }
                            }

                            if (!pLoopPlot->isWater() && !pLoopPlot->isPeak() && (pLoopPlot->getBonusType()==NO_BONUS) && GC.getGameINLINE().getSorenRandNum(100, "Discover food")<200)
                            {
                                int ifood=GC.getGameINLINE().getSorenRandNum(180, "foodresource");
                                switch (ifood)
                                {
                                case 0:
                                    if(GC.getInfoTypeForString("BONUS_COW")!=NO_BONUS)
                                    {
                                        pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_COW"));
                                    }
                                    break;
                                case 1:
                                    if(GC.getInfoTypeForString("BONUS_PIG")!=NO_BONUS)
                                    {
                                        pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_PIG"));
                                    }
                                    break;
                                case 2:
                                    if(GC.getInfoTypeForString("BONUS_SHEEP")!=NO_BONUS)
                                    {
                                        pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_SHEEP"));
                                    }
                                    break;
                                case 3:
                                    if(GC.getInfoTypeForString("BONUS_CORN")!=NO_BONUS)
                                    {
                                        pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_CORN"));
                                    }
                                    break;
                                case 4:
                                    if(GC.getInfoTypeForString("BONUS_WHEAT")!=NO_BONUS)
                                    {
                                        pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_WHEAT"));
                                    }
                                    break;
                                case 5:
                                    if(GC.getInfoTypeForString("BONUS_RICE")!=NO_BONUS)
                                    {
                                        pLoopPlot->setBonusType((BonusTypes)GC.getInfoTypeForString("BONUS_RICE"));
                                    }
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                    }

                    PromotionTypes SpecialPromotion=NO_PROMOTION;
                    CvWString szBuffer;

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_AIR"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_FLYING");
                            szBuffer=("Because of a strange magical node our unit can walk on air now");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_BODY"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_WILD_REGENERATION");
                            szBuffer=("Because of a strange magical node our unit heals much faster now");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_CHAOS"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_MUTATED");
                            szBuffer=("Because of a strange magical node our unit was mutated");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_ENCHANTMENT"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_ENCHANTED_BLADE_WILD");
                            szBuffer=("Because of a strange magical node our unit has magic weapons now");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_LAW"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_WILD_VALOR");
                            szBuffer=("Because of a strange magical node our unit is more devoted to our cause now");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_MIND"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_LOYALTY");
                            szBuffer=("Because of a strange magical node our unit is more devoted to our cause now");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_SHADOW"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_WILD_SHADOWWALK");
                            szBuffer=("Because of a strange magical node our unit can walk in the shadows now");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_SPIRIT"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_WILD_VALOR");
                            szBuffer=("Because of a strange magical node our unit is more devoted to our cause now");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_FORCE"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_WILD_VALOR");
                            szBuffer=("Because of a strange magical node our unit is now strong with the force");
                        }
                    }

                    if (getOriginPlot()->getBonusType()==(BonusTypes)GC.getInfoTypeForString("BONUS_MANA_DIMENSIONAL"))
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Special")<100)
                        {
                            SpecialPromotion=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_MOBILITY2");
                            szBuffer=("Because of a strange magical node our unit now moves much faster");
                        }
                    }

                    if (SpecialPromotion!=NO_PROMOTION)
                    {
                        CLLNode<IDInfo>* pUnitNode;
                        CvUnit* pLoopUnit;
                        CvUnit* pSpecialUnit=NULL;
                        pUnitNode = pLoopPlot->headUnitNode();
                        while (pUnitNode != NULL)
                        {
                            pLoopUnit = ::getUnit(pUnitNode->m_data);
                            pUnitNode = plot()->nextUnitNode(pUnitNode);

                            if (pLoopUnit!=NULL)
                            {
                                pSpecialUnit=pLoopUnit;
                                if (GC.getGameINLINE().getSorenRandNum(pLoopPlot->getNumUnits(), "Special")==0)
                                    break;
                            }
                        }

                        if (pSpecialUnit!=NULL)
                        {
                            pSpecialUnit->setHasPromotion(SpecialPromotion,true);
                            gDLL->getInterfaceIFace()->addMessage(pSpecialUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTUREEXPANDS", MESSAGE_TYPE_MINOR_EVENT, GC.getCommerceInfo(COMMERCE_CULTURE).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pSpecialUnit->getX_INLINE(), pSpecialUnit->getY_INLINE(), true, true);
                        }
                    }
                }
            }
        }
    }//Defendergroup finished
}

// UnitAI::doTurn function for UNITAI_MANADEFENDER
void CvUnitAI::AI_manadefendermove()
{

    //function run by the Guardians
    if (isHasPromotion((PromotionTypes)GC.getInfoTypeForString("PROMOTION_MANA_DEFENDER")))
    {
        setOriginPlot(plot());
        AI_setGroupflag(GROUPFLAG_MANADEFENDER_DEF);

//        AI_manadefenderEffect(); // should be run once a turn
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

	getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_upgrademanaMove()
{
    CyUnit* pyUnit1 = new CyUnit(this);
    CyArgsList argsList1;
    argsList1.add(gDLL->getPythonIFace()->makePythonObject(pyUnit1));	// pass in unit class
    long lResult=0;
    gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_Mage_UPGRADE_MANA", argsList1.makeFunctionArgs(), &lResult);
    delete pyUnit1;	// python fxn must not hold on to this pointer

    if (lResult == 1)
    {
        return;
    }
    if (lResult == 2)
    {
        AI_setUnitAIType(UNITAI_TERRAFORMER);
    }
    getGroup()->pushMission(MISSION_SKIP);
    return;
}

//returns true if the Unit can Summon stuff
bool CvUnitAI::isSummoner()
{
    return false;
}

//Make sure iNumSummonSpells is big enough
void CvUnitAI::AI_SummonCast()
{
    return;
}

//returns true if the Unit can Summon stuff
bool CvUnitAI::isDirectDamageCaster()
{
    return false;
}

//Make sure iNumSummonSpells is big enough
//Spell will only be Cast if it can damage Threshold Units
void CvUnitAI::AI_DirectDamageCast(int Threshold)
{
	return;
}

//returns true if the Unit can Summon stuff
bool CvUnitAI::isDeBuffer()
{
    return false;
}

void CvUnitAI::AI_DeBuffCast()
{
	return;
}

bool CvUnitAI::isMovementCaster()
{
    return false;
}

void CvUnitAI::AI_MovementCast()
{
/**
    if (canCast(GC.getInfoTypeForString("SPELL_ACCELERATE"),false))
        cast(GC.getInfoTypeForString("SPELL_ACCELERATE"));

    if (canCast(GC.getInfoTypeForString("SPELL_HASTE"),false))
        cast(GC.getInfoTypeForString("SPELL_HASTE"));
**/
}

bool CvUnitAI::isBuffer()
{
    return false;
}

// This is run often, so lets keep things simple
void CvUnitAI::AI_BuffCast()
{
	return;
}

int CvUnitAI::AI_getSettlerPatrolUnitsNeeded()
{
    if(AI_getUnitAIType()!=UNITAI_SETTLE)
    {
        return 0;
    }

    int iNeededPatrol=4;
    if (GET_TEAM(getTeam()).isBarbarianAlly())
    {
        iNeededPatrol=3;
    }

    return iNeededPatrol;
}
bool CvUnitAI::isSuicideSummon()
{
    return m_bSuicideSummon;
}

void CvUnitAI::setSuicideSummon(bool newvalue)
{
    m_bSuicideSummon=newvalue;
}

bool CvUnitAI::isPermanentSummon()
{
    return m_bPermanentSummon;
}

void CvUnitAI::setPermanentSummon(bool newvalue)
{
    m_bPermanentSummon=newvalue;
}

void CvUnitAI::changeAllowedPatrol(bool bNewValue)
{
    m_bAllowedPatrol=bNewValue;
}

void CvUnitAI::changeAllowedExplore(bool bNewValue)
{
    m_bAllowedExplore=bNewValue;
}

void CvUnitAI::AI_setWasAttacked(int iNewValue)
{
    m_iWasAttacked=iNewValue;
}

int CvUnitAI::AI_getWasAttacked()
{
    return m_iWasAttacked;
}

/** BETTER AI (Naval AI) Sephi                                                                   **/
void CvUnitAI::AI_SettleNavalNewMove()
{
    CvUnit* pLoopUnit;
    int iLoop;
    for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
    {
        if (pLoopUnit)
        {
            if (pLoopUnit!=this)
            {
                if (pLoopUnit->AI_getGroupflag()==GROUPFLAG_NAVAL_SETTLE || pLoopUnit->AI_getGroupflag()==GROUPFLAG_NAVAL_SETTLE_PICKUP )
                {
                    if(atPlot(pLoopUnit->plot()))
                    {
                        joinGroup(pLoopUnit->getGroup());
                        AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE);
                        return;
                    }
                    else
                    {
                        int iPathTurns;
                        if(generatePath(pLoopUnit->plot(), 0, true, &iPathTurns))
                        {
                            //getGroup()->pushMission(MISSION_MOVE_TO_UNIT, pLoopUnit->getOwnerINLINE(), pLoopUnit->getID(), 0, false, false, MISSIONAI_GROUP, NULL, pLoopUnit);
							getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), MOVE_DIRECT_ATTACK);
                            return;
                        }

                        AI_setUnitAIType(UNITAI_EXPLORE_SEA);
                        return;
                    }
                }
            }
        }
    }
    AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE);
    return;
}

void CvUnitAI::AI_SettleNavalPickupMove()
{
    if (!plot()->isCity() || GET_PLAYER(getOwnerINLINE()).AI_getExpansionPlot()==NULL)
    {
        AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE);
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

    //Count Settlers and Workers
	int iSettlerCount = 0;
	int iWorkerCount = 0;
	int iTotalCargo = 0;
	int iTotalCargoSpace = 0;

    CLLNode<IDInfo>* pEntityNode = getGroup()->headUnitNode();
    CvUnit* pLoopUnit;
    while (pEntityNode != NULL)
    {
        pLoopUnit = ::getUnit(pEntityNode->m_data);
        pEntityNode = getGroup()->nextUnitNode(pEntityNode);

        iSettlerCount += pLoopUnit->getUnitAICargo(UNITAI_SETTLE);
        iWorkerCount += pLoopUnit->getUnitAICargo(UNITAI_WORKER);
        iTotalCargo += pLoopUnit->getCargo();
        iTotalCargoSpace += std::max(0,pLoopUnit->cargoSpace()-pLoopUnit->getCargo());
    }

    //Need to Pick up a Settler
    if(iSettlerCount==0)
    {
        if(iTotalCargoSpace<1)
        {
            getGroup()->unloadAll();
        }
        if (plot()->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_SETTLE)==0)
        {
            AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE);
            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
        int iLoop;

        for(CvUnit* pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
        {
            if (pLoopUnit->getArea()==getArea())
            {
                if (pLoopUnit->AI_getUnitAIType()==UNITAI_SETTLE)
                {
                    if(getArea()!=GET_PLAYER(getOwnerINLINE()).AI_getExpansionPlot()->getArea())
                    {
                        int iPathTurns;
                        if (pLoopUnit->generatePath(plot(),0,true,&iPathTurns))
                        {
                            if(pLoopUnit->atPlot(plot()))
                            {
                                CLLNode<IDInfo>* pEntityNode = pLoopUnit->getGroup()->headUnitNode();
                                CvUnit* pLoopUnit1;
                                while (pEntityNode != NULL)
                                {
                                    pLoopUnit1 = ::getUnit(pEntityNode->m_data);
                                    pEntityNode = pLoopUnit->getGroup()->nextUnitNode(pEntityNode);

                                    pLoopUnit1->joinGroup(NULL);

                                    //Load Units into Transporters
                                    bool bUnitLoaded=false;

                                    CLLNode<IDInfo>* pEntityNode = getGroup()->headUnitNode();
                                    CvUnit* pLoopTrans;
                                    while (pEntityNode != NULL)
                                    {
                                        pLoopTrans = ::getUnit(pEntityNode->m_data);
                                        pEntityNode = getGroup()->nextUnitNode(pEntityNode);

                                        //Settler(pLoopUnit) has Priority
                                        if(!pLoopTrans->isFull())
                                        {
                                            if (!pLoopUnit->isCargo())
                                            {
												pLoopUnit->getGroup()->setTransportUnit(pLoopTrans);
                                            }
                                        }
                                        if(!pLoopTrans->isFull())
                                        {
											pLoopUnit1->getGroup()->setTransportUnit(pLoopTrans);
                                            bUnitLoaded=true;
                                        }

                                        if(bUnitLoaded)
                                        {
                                            break;
                                        }
                                    }
                                }
                                getGroup()->pushMission(MISSION_SKIP);
                                return;
                            }
                            else
                            {
                                pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, getX_INLINE(), getY_INLINE(), 0, false, false, NO_MISSIONAI, plot());
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    //Need to Pick up Guarding Units
    if(iTotalCargoSpace>0)
    {
        if (plot()->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_BARBSMASHER)+plot()->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_ATTACK_CITY)==0)
        {
            AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE);
            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
        int iLoop;
        int UnitsNeeded=iTotalCargoSpace;

        for(CvUnit* pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
        {
            if (pLoopUnit->getArea()==getArea())
            {
                if (pLoopUnit->AI_getUnitAIType()==UNITAI_BARBSMASHER || pLoopUnit->AI_getUnitAIType()==UNITAI_ATTACK_CITY)
                {
                    if(pLoopUnit->getGroup()->getNumUnits()==1)  //for now only pick up newly build units
                    {
                        int iPathTurns;
                        if (pLoopUnit->generatePath(plot(),0,true,&iPathTurns))
                        {
                            if(pLoopUnit->atPlot(plot()))
                            {
                                pLoopUnit->joinGroup(NULL);
                                bool bUnitLoaded=false;

                                CLLNode<IDInfo>* pEntityNode = getGroup()->headUnitNode();
                                CvUnit* pLoopTrans;
                                while (pEntityNode != NULL)
                                {
                                    pLoopTrans = ::getUnit(pEntityNode->m_data);
                                    pEntityNode = getGroup()->nextUnitNode(pEntityNode);

                                    if(!pLoopTrans->isFull())
                                    {
                                        pLoopUnit->getGroup()->setTransportUnit(pLoopTrans);
                                        bUnitLoaded=true;
                                    }

                                    if(bUnitLoaded)
                                    {
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, getX_INLINE(), getY_INLINE(), 0, false, false, NO_MISSIONAI, plot());
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
	//can start the Settle Mission
	if(iSettlerCount>0 && iTotalCargoSpace==0)
	{
        AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE);
        getGroup()->pushMission(MISSION_SKIP);
        return;
	}
}

bool CvUnitAI::AI_exploreLair()
{
    int iValue;
    int iBestValue=10;
    int iSearchRange=4;
    int iDX,iDY;
    CvPlot* pLoopPlot;
    CvPlot* pBestPlot=NULL;

    for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
    {
        for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
        {
            pLoopPlot = plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), iDX, iDY);

            if (pLoopPlot != NULL)
            {
                if (AI_plotValid(pLoopPlot))
                {
                    if(pLoopPlot->getArea()==getArea() && pLoopPlot->getTeam()==getTeam())
                    {
                        if (!pLoopPlot->isVisibleEnemyUnit(this))
                        {
                            bool bValid=false;

                            int iImprovement=pLoopPlot->getImprovementType();

                            if(iImprovement==GC.getInfoTypeForString("IMPROVEMENT_DUNGEON"))
                            {
                                bValid=true;
                            }

                            if(iImprovement!=NO_IMPROVEMENT)
                            {
                                if(GC.getImprovementInfo((ImprovementTypes)iImprovement).getSpawnUnitCiv()==GC.getInfoTypeForString("CIVILIZATION_BARBARIAN")
                                && !GET_TEAM(getTeam()).isBarbarianAlly())
                                {
                                    bValid=true;
                                }
                            }

                            if(bValid)
                            {
                                if (generatePath(pLoopPlot,0,true,&iValue))
                                {
                                    if(iValue<iBestValue)
                                    {
                                        pBestPlot=pLoopPlot;
                                        iBestValue=iValue;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        if(atPlot(pBestPlot))
        {
            SpellTypes eExploreBarrow=(SpellTypes)GC.getInfoTypeForString("SPELL_EXPLORE_LAIR_BARROW");
            SpellTypes eExploreDungeon=(SpellTypes)GC.getInfoTypeForString("SPELL_EXPLORE_LAIR_DUNGEON");
            SpellTypes eExploreRuins=(SpellTypes)GC.getInfoTypeForString("SPELL_EXPLORE_LAIR_RUINS");
            SpellTypes eExploreGoblinFort=(SpellTypes)GC.getInfoTypeForString("SPELL_EXPLORE_LAIR_GOBLIN_FORT");

            if(canCast(eExploreBarrow,false))
            {
                cast(eExploreBarrow);
            }

            else if(canCast(eExploreDungeon,false))
            {
                cast(eExploreDungeon);
            }

            else if(canCast(eExploreRuins,false))
            {
                cast(eExploreRuins);
            }

            else if(canCast(eExploreGoblinFort,false))
            {
                cast(eExploreGoblinFort);
            }

            return false;
        }
        else
        {
            getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
            return true;
        }
    }

    return false;
}

void CvUnitAI::AI_InquisitionMove()
{
    CvPlot* pBestPlot=NULL;
    int iValue;
    int iBestValue=100;

    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
    {
        CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

        if(pLoopPlot)
        {
            if (AI_plotValid(pLoopPlot))
            {
                if(pLoopPlot->isCity())
                {
                    if(pLoopPlot->getArea()==getArea())
                    {
                        if (!pLoopPlot->isVisibleEnemyUnit(this))
                        {
                            bool bValidTargetForInquisition=false;
                            int iStateBelief = GET_PLAYER(getOwnerINLINE()).getStateReligion();
                            CvCity* pCity=pLoopPlot->getPlotCity();

                            for (int iTarget=0;iTarget<GC.getNumReligionInfos();iTarget++)
                            {
                                if (iStateBelief != iTarget && pCity->isHasReligion((ReligionTypes)iTarget) && (!pCity->isHolyCity((ReligionTypes)iTarget)))
                                {
                                    bValidTargetForInquisition=true;
                                }
                            }

                            if (iStateBelief == NO_RELIGION)
                            {
                                if (getOwnerINLINE()!=pCity->getOwnerINLINE())
                                {
                                    bValidTargetForInquisition=false;
                                }
                            }

                            if (iStateBelief != GET_PLAYER(pCity->getOwner()).getStateReligion())
                            {
                                bValidTargetForInquisition=false;
                            }

                            if (bValidTargetForInquisition && generatePath(pLoopPlot,0,true,&iValue))
                            {
                                if(iValue<iBestValue)
                                {
                                    pBestPlot=pLoopPlot;
                                    iBestValue=iValue;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
        return;
    }

    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_SvartalfarKidnapMove()
{
	int iSpell=GC.getInfoTypeForString("SPELL_KIDNAP");

	if(iSpell!=NO_SPELL && canCast(iSpell,false))
	{
		cast(iSpell);
	}

    CvPlot* pBestPlot=NULL;
    int iValue;
    int iBestValue=100;

    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
    {
        CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

        if(pLoopPlot)
        {
            if (AI_plotValid(pLoopPlot))
            {
                if(pLoopPlot->isCity())
                {
                    if(pLoopPlot->getArea()==getArea() && pLoopPlot->getTeam()!=getTeam())
                    {
                        if (!pLoopPlot->isVisibleEnemyUnit(this))
                        {
							CvCity* pLoopCity=pLoopPlot->getPlotCity();
                            bool bValidTargetForKidnap=true;

							//too bad this is hardcoded in the Spell
						    SpecialistTypes iGreatPriest=(SpecialistTypes)GC.getInfoTypeForString("SPECIALIST_GREAT_PRIEST");
							SpecialistTypes iGreatArtist=(SpecialistTypes)GC.getInfoTypeForString("SPECIALIST_GREAT_ARTIST");
							SpecialistTypes iGreatMerchant=(SpecialistTypes)GC.getInfoTypeForString("SPECIALIST_GREAT_MERCHANT");
							SpecialistTypes iGreatEngineer=(SpecialistTypes)GC.getInfoTypeForString("SPECIALIST_GREAT_ENGINEER");
							SpecialistTypes iGreatScientist=(SpecialistTypes)GC.getInfoTypeForString("SPECIALIST_GREAT_SCIENTIST");

							int iCountSpecialists=0;
							if(iGreatPriest!=-1) iCountSpecialists+=pLoopCity->getFreeSpecialistCount(iGreatPriest);
							if(iGreatArtist!=-1) iCountSpecialists+=pLoopCity->getFreeSpecialistCount(iGreatArtist);
							if(iGreatMerchant!=-1) iCountSpecialists+=pLoopCity->getFreeSpecialistCount(iGreatMerchant);
							if(iGreatEngineer!=-1) iCountSpecialists+=pLoopCity->getFreeSpecialistCount(iGreatEngineer);
							if(iGreatScientist!=-1) iCountSpecialists+=pLoopCity->getFreeSpecialistCount(iGreatScientist);

							if (iCountSpecialists==0)
							{
								bValidTargetForKidnap=false;
							}
							else if (GET_PLAYER(getOwnerINLINE()).AI_getAttitude(pLoopPlot->getOwnerINLINE())>=ATTITUDE_PLEASED)
							{
								if((GET_TEAM(getTeam()).getPower(true)*2)<(GET_TEAM(pLoopPlot->getTeam()).getPower(true)*3))
								{
									//not enough Power to risk declare war
									bValidTargetForKidnap=false;
								}
							}
							else
							{
								if((GET_TEAM(getTeam()).getPower(true)*3)<(GET_TEAM(pLoopPlot->getTeam()).getPower(true)*2))
								{
									//not enough Power to risk declare war
									bValidTargetForKidnap=false;
								}
							}

                            if (bValidTargetForKidnap && generatePath(pLoopPlot,0,true,&iValue))
                            {
                                if(iValue<iBestValue)
                                {
                                    pBestPlot=pLoopPlot;
                                    iBestValue=iValue;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
        return;
    }

    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_EsusMissionaryMove()
{
	int iSpell=GC.getInfoTypeForString("SPELL_SPREAD_THE_COUNCIL_OF_ESUS");

	if(iSpell!=NO_SPELL && canCast(iSpell,false))
	{
		cast(iSpell);
	}

    CvPlot* pBestPlot=NULL;
    int iValue;
    int iBestValue=100;

    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
    {
        CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

        if(pLoopPlot)
        {
            if (AI_plotValid(pLoopPlot))
            {
                if(pLoopPlot->isCity())
                {
                    if(pLoopPlot->getArea()==getArea() && pLoopPlot->getOwnerINLINE()==getOwnerINLINE())
                    {
						CvCity* pLoopCity=pLoopPlot->getPlotCity();
					    ReligionTypes iEsus=(ReligionTypes)GC.getInfoTypeForString("RELIGION_COUNCIL_OF_ESUS");
						if ((!pLoopCity->isHasReligion(iEsus)) && generatePath(pLoopPlot,0,true,&iValue))
						{
                            if(iValue<iBestValue)
                            {
                                pBestPlot=pLoopPlot;
                                iBestValue=iValue;
                            }
                        }
                    }
                }
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
        return;
    }

    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_AwakenedMove()
{
    //already in a City we can join
    if(plot()->isCity() && plot()->getOwnerINLINE()==getOwner())
    {
        if(plot()->getPlotCity()->happyLevel()-plot()->getPlotCity()->unhappyLevel(0)>0)
        {
            getGroup()->pushMission(MISSION_SKIP);
            return;
        }
    }

    CvPlot* pBestPlot=NULL;
    int iValue;
    int iBestValue=100;

   	CvCity* pLoopCity;
	int iLoop;

	for (pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
	{
        if(pLoopCity->happyLevel()-pLoopCity->unhappyLevel(0)>0)
        {
            if (generatePath(pLoopCity->plot(),0,true,&iValue))
			{
			    if(iValue<iBestValue)
                {
                    pBestPlot=pLoopCity->plot();
                    iBestValue=iValue;
                }
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
        return;
    }
    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_CreeperMove()
{
    CvPlot* pBestPlot=NULL;
    int iValue;
    int iBestValue=0;

    int iSearchRange=15;

    int iDX, iDY;
    CvPlot* pLoopPlot;
    int iPathTurns;

    for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
    {
        for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
        {
            pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

            if (pLoopPlot != NULL)
            {
				if (AI_plotValid(pLoopPlot))
				{
					if (pLoopPlot->getArea()==getArea())
					{
					    if((pLoopPlot->isOwned() && pLoopPlot->getTeam()!=getTeam() && pLoopPlot->getImprovementType()!=NO_IMPROVEMENT)
                           || pLoopPlot->isOwned() && pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && pLoopPlot->getFeatureType()==GC.getInfoTypeForString("FEATURE_HAUNTED_LANDS"))
                        {
                            if (generatePath(pLoopPlot,0,true,&iPathTurns))
                            {
                                iValue =0;
                                if(pLoopPlot->getTeam()!=getTeam())
                                {
                                    if(pLoopPlot->getImprovementType()==GC.getInfoTypeForString("IMPROVEMENT_FARM"))
                                    {
                                        iValue+=500;
                                        if(pLoopPlot->getNonObsoleteBonusType(getTeam())!=NO_BONUS)
                                        {
                                            iValue+=5000;
                                        }
                                    }

                                    if(pLoopPlot->getImprovementType()==GC.getInfoTypeForString("IMPROVEMENT_PASTURE"))
                                    {
                                        iValue+=500;
                                        if(pLoopPlot->getNonObsoleteBonusType(getTeam())!=NO_BONUS)
                                        {
                                            iValue+=5000;
                                        }
                                    }

                                    if(pLoopPlot->getImprovementType()==GC.getInfoTypeForString("IMPROVEMENT_PLANTATION"))
                                    {
                                        iValue+=500;
                                        if(pLoopPlot->getNonObsoleteBonusType(getTeam())!=NO_BONUS)
                                        {
                                            iValue+=5000;
                                        }
                                    }

                                    if(pLoopPlot->getImprovementType()==GC.getInfoTypeForString("IMPROVEMENT_CAMP"))
                                    {
                                        iValue+=500;
                                        if(pLoopPlot->getNonObsoleteBonusType(getTeam())!=NO_BONUS)
                                        {
                                            iValue+=5000;
                                        }
                                    }
                                }
                                else
                                {
                                    if(pLoopPlot->getFeatureType()==-1)
                                    {
                                        iValue+=200;
                                        if(pLoopPlot->isBeingWorked())
                                        {
                                            iValue += 500;
                                        }
                                    }
                                }

                                iValue /= (iPathTurns + 1);

                                if (iValue > iBestValue)
                                {
                                    iBestValue = iValue;
                                    pBestPlot = getPathEndTurnPlot();
                                }
                            }
                        }
					}
				}
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        if(!atPlot(pBestPlot))
        {
            getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
            return;
        }
        else
        {
            if(getTeam()!=plot()->getTeam())
            {
                SpellTypes iSpell=(SpellTypes)GC.getInfoTypeForString("SPELL_ARAWNS_DUST");
                if(iSpell!=NO_SPELL && canCast(iSpell,false))
                {
                    cast(iSpell);
                }
            }
            else
            {
                SpellTypes iSpell=(SpellTypes)GC.getInfoTypeForString("SPELL_GHOSTWALKER_MEND_NATURE");
                if(iSpell!=NO_SPELL && canCast(iSpell,false))
                {
                    cast(iSpell);
                }
            }
        }
    }

    getGroup()->pushMission(MISSION_SKIP);
    return;
}
    //already in a City we can join
void CvUnitAI::AI_PillageGroupMove()
{
    //we are not in Enemy land yet
    if(!plot()->isOwned() || getTeam()==plot()->getTeam() || (!GET_TEAM(getTeam()).isAtWar(plot()->getTeam())))
    {
        if (AI_moveToStagingCity())
        {
            return;
        }
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

    CvPlot* pBestPlot=NULL;
    int iValue;
    int iBestValue=100;

    int iSearchRange=5;

    //raider units
    if (isEnemyRoute())
    {
        iSearchRange*=3;
    }

    int iDX, iDY;
    CvPlot* pLoopPlot;
    CvPlot* pBestPillagePlot;
    int iPathTurns;

    for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
    {
        for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
        {
            pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

            if (pLoopPlot != NULL)
            {
				if (AI_plotValid(pLoopPlot) && !(pLoopPlot->isBarbarian()) && pLoopPlot->isOwned())
				{
					if (pLoopPlot->getArea()==getArea() && potentialWarAction(pLoopPlot))
					{
                        if (!pLoopPlot->isVisibleEnemyUnit(this) || getGroup()->AI_attackOdds(pLoopPlot, true)>80)
                        {
                            if (generatePath(pLoopPlot,0,true,&iPathTurns))
                            {
                                if(canPillage(pLoopPlot))
                                {
                                    iValue = AI_pillageValue(pLoopPlot,0);

                                    if (pLoopPlot->getNonObsoleteBonusType((pLoopPlot->getTeam()))!= NO_BONUS)
                                    {
                                        iValue*=3;
                                    }

                                    iValue *= 1000;
                                    iValue /= (iPathTurns + 1);

                                    if (iValue > iBestValue)
                                    {
                                        iBestValue = iValue;
                                        pBestPlot = getPathEndTurnPlot();
                                        pBestPillagePlot = pLoopPlot;
                                    }
                                }
                            }
                        }
					}
				}
            }
        }
    }

    if (pBestPlot!=NULL)
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
        return;
    }

    getGroup()->pushMission(MISSION_SKIP);
    return;
}

void CvUnitAI::AI_SettleNavalMove()
{
    //Do we have a target?
    CvPlot* pTargetPlot=GET_PLAYER(getOwnerINLINE()).AI_getExpansionPlot();

    if (pTargetPlot==NULL)
    {
        //wait until we get a new Target
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }

    //Count Settlers and Workers
	int iSettlerCount = 0;
	int iWorkerCount = 0;
	int iTotalCargo = 0;
	int iTotalCargoSpace = 0;



    CLLNode<IDInfo>* pEntityNode = getGroup()->headUnitNode();
    CvUnit* pLoopUnit;
    while (pEntityNode != NULL)
    {
        pLoopUnit = ::getUnit(pEntityNode->m_data);
        pEntityNode = getGroup()->nextUnitNode(pEntityNode);

        iSettlerCount += pLoopUnit->getUnitAICargo(UNITAI_SETTLE);
        iWorkerCount += pLoopUnit->getUnitAICargo(UNITAI_WORKER);
        iTotalCargo += pLoopUnit->getCargo();
        iTotalCargoSpace += std::max(0,pLoopUnit->cargoSpace()-pLoopUnit->getCargo());
    }

    int iGuardingUnits = iTotalCargo-iTotalCargoSpace-iSettlerCount-iWorkerCount;

    //Transport needs Settler?
    if(iSettlerCount==0)
    {
        if(plot()->isCity())
        {
            //if units are around to pick up
            if (plot()->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_SETTLE)>0)
            {
                AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE_PICKUP);
                getGroup()->pushMission(MISSION_SKIP);
                return;
            }
        }

        CvCity* pBestCity=NULL;
        int iBestValue=0;
        int iValue,iLoop;

        for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
        {
            if(pLoopCity->isCoastal(-1))
            {
                int iPathTurns;
                if(generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
                {
                    iValue=pLoopCity->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_SETTLE);
                    iValue*=1000;
                    int iDivisor=abs(pLoopCity->getX_INLINE()-getX_INLINE())+abs(pLoopCity->getY_INLINE()-getY_INLINE());
                    iValue/=(20+iDivisor);

                    if (iValue>iBestValue)
                    {
                        iBestValue=iValue;
                        pBestCity=pLoopCity;
                    }
                }
            }
        }

        if (pBestCity!=NULL)
        {
            getGroup()->pushMission(MISSION_MOVE_TO, pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), 0, false, false, NO_MISSIONAI, pBestCity->plot());
            return;
        }
        //no Reinforcements Available
        if (AI_retreatToCity())
        {
            return;
        }

        getGroup()->pushMission(MISSION_SKIP);
        return;
    }


    //Transport is not Full?
    if(iTotalCargoSpace>0 && (5>iGuardingUnits))
    {
        if(plot()->isCity())
        {
            //if units are around to pick up
            if (plot()->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_ATTACK_CITY)+area()->getNumAIUnits(getOwnerINLINE(),UNITAI_BARBSMASHER)>0)
            {
                AI_setGroupflag(GROUPFLAG_NAVAL_SETTLE_PICKUP);
                getGroup()->pushMission(MISSION_SKIP);
                return;
            }
        }
        //move to potential units to pick up

        CvCity* pBestCity=NULL;
        int iBestValue=0;
        int iValue,iLoop;

        for (CvCity* pLoopCity = GET_PLAYER(getOwnerINLINE()).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER(getOwnerINLINE()).nextCity(&iLoop))
        {
            if(pLoopCity->isCoastal(-1))
            {
                int iPathTurns;
                if(generatePath(pLoopCity->plot(), 0, true, &iPathTurns))
                {
                    iValue=pLoopCity->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_ATTACK_CITY);
                    iValue+=pLoopCity->area()->getNumAIUnits(getOwnerINLINE(),UNITAI_BARBSMASHER);
                    iValue*=1000;
                    int iDivisor=abs(pLoopCity->getX_INLINE()-getX_INLINE())+abs(pLoopCity->getY_INLINE()-getY_INLINE());
                    iValue/=(20+iDivisor);

                    if (iValue>iBestValue)
                    {
                        iBestValue=iValue;
                        pBestCity=pLoopCity;
                    }
                }
            }
        }

        if (pBestCity!=NULL)
        {
            getGroup()->pushMission(MISSION_MOVE_TO, pBestCity->getX_INLINE(), pBestCity->getY_INLINE(), 0, false, false, NO_MISSIONAI, pBestCity->plot());
            return;
        }
        //no Reinforcements Available
        if (AI_retreatToCity())
        {
            return;
        }

        getGroup()->pushMission(MISSION_SKIP);
        return;

    }

    //Move Closer to TargetPlot
    if(atPlot(pTargetPlot))
    {
        getGroup()->unloadAll();
        getGroup()->pushMission(MISSION_SKIP);
        return;
    }
    else
    {
        getGroup()->pushMission(MISSION_MOVE_TO, pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE(), 0, false, false, MISSIONAI_FOUND, pTargetPlot);
        return;
    }

}

//called in CvUnitAI::AI_barbAttackMove()
//returns true if a Mission is pushed
bool CvUnitAI::AI_barbSpecialActionMove()
{
	return false;
}
/*************************************************************************************************/
/**	END                                                                  						**/
/*************************************************************************************************/



/*************************************************************************************************/
/** Skyre Mod                                                                                   **/
/** BETTER AI (Lanun Pirate Coves) merged Sephi                                                 **/
/**						                                            							**/
/*************************************************************************************************/
bool CvUnitAI::AI_buildPirateCove()
{
    PROFILE_FUNC();

    SpellTypes eCoveSpell = (SpellTypes)GC.getDefineINT("PIRATE_COVE_SPELL");

    if (eCoveSpell == NO_SPELL)
    {
        return false;
    }

    std::vector<CvPlot*> apGoodPlots;
    int iBestPlotValue = 0;

    for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
    {
        CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

        if (AI_plotValid(pLoopPlot))
        {
            if (pLoopPlot->getOwnerINLINE() != getOwnerINLINE() || pLoopPlot->getWorkingCity() == NULL)
            {
                continue;
            }

            if (pLoopPlot->area() != area() && !plot()->isAdjacentToArea(pLoopPlot->area()))
            {
                continue;
            }

            if (pLoopPlot->isVisibleEnemyUnit(this))
            {
                continue;
            }

            if (!pLoopPlot->isPirateCoveValid(getOwnerINLINE()))
            {
                continue;
            }

            if (!atPlot(pLoopPlot) && !canMoveInto(pLoopPlot))
            {
                continue;
            }

            int iDistance = 0;

            for (int iX = -5; iX <= 5; iX++)
            {
                for (int iY = -5; iY <= 5; iY++)
                {
                    CvPlot* pSearchPlot = plotXY(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), iX, iY);

                    if (pSearchPlot != NULL && pSearchPlot->isPirateCove())
                    {
                        iDistance = std::min(iDistance, std::max(std::abs(iX), std::abs(iY)) * 10 + std::min(std::abs(iX), std::abs(iY)));
                    }
                }
            }

            int iPlotValue = (iDistance > 0) ? (100 - iDistance) : 70;

            if (iPlotValue > iBestPlotValue)
            {
                apGoodPlots.clear();
                apGoodPlots.push_back(pLoopPlot);

                iBestPlotValue = iPlotValue;
            }
            else if (iPlotValue == iBestPlotValue)
            {
                apGoodPlots.push_back(pLoopPlot);
            }
        }
    }

    CvPlot* pBestPlot = NULL;

    if (!apGoodPlots.empty())
    {
        int iShortestDistance = MAX_INT;
        std::vector<CvPlot*>::iterator it;

        for (it = apGoodPlots.begin(); it != apGoodPlots.end(); ++it)
        {
            int iPathTurns;

            generatePath(*it, 0, true, &iPathTurns);

            if (iPathTurns < iShortestDistance)
            {
                pBestPlot = *it;
                iShortestDistance = iPathTurns;
            }
        }
    }

    if (pBestPlot)
    {
        if (plot() != pBestPlot)
        {
            getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, pBestPlot);
        }
        else
        {
            if (canCast(eCoveSpell, false))
            {
                cast(eCoveSpell);
            }

            getGroup()->pushMission(MISSION_SKIP);
        }

        return true;
    }

    return false;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/02/09                                jdog5000      */
/*                                                                                              */
/* Player Interface                                                                             */
/************************************************************************************************/
int CvUnitAI::AI_exploreAirPlotValue( CvPlot* pPlot )
{
	int iValue = 0;
	if (pPlot->isVisible(getTeam(), false))
	{
		iValue++;

		if (!pPlot->isOwned())
		{
			iValue++;
		}

		if (!pPlot->isImpassable())
		{
			iValue *= 4;

			if (pPlot->isWater() || pPlot->getArea() == getArea())
			{
				iValue *= 2;
			}
		}
	}

	return iValue;
}

bool CvUnitAI::AI_exploreAir2()
{
	PROFILE_FUNC();

	CvPlayer& kPlayer = GET_PLAYER(getOwner());
	CvPlot* pLoopPlot = NULL;
	CvPlot* pBestPlot = NULL;
	int iBestValue = 0;

	int iDX, iDY;
	int iSearchRange = airRange();
	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if( pLoopPlot != NULL )
			{
				if (canReconAt(plot(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()))
				{
					int iValue = AI_exploreAirPlotValue( pLoopPlot );

					for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
					{
						DirectionTypes eDirection = (DirectionTypes) iI;
						CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), eDirection);
						if (pAdjacentPlot != NULL)
						{
							if( !pAdjacentPlot->isVisible(getTeam(),false) )
							{
								iValue += AI_exploreAirPlotValue( pAdjacentPlot );
							}
						}
					}

					iValue += GC.getGame().getSorenRandNum(25, "AI explore air");
					iValue *= std::min(7, plotDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()));

					if(!pLoopPlot->isVisible(getTeam(),false))
					{
						iValue*=3;
					}

					if (iValue > iBestValue)
					{
						iBestValue = iValue;
						pBestPlot = pLoopPlot;
					}
				}
			}
		}
	}

	if (pBestPlot != NULL)
	{
		getGroup()->pushMission(MISSION_RECON, pBestPlot->getX(), pBestPlot->getY());
		return true;
	}

	return false;
}

void CvUnitAI::AI_exploreAirMove()
{
/** disable for now
	if( AI_exploreAir() )
	{
		return;
	}
**/
	if( AI_exploreAir2() )
	{
		return;
	}

	if( canAirDefend() )
	{
		getGroup()->pushMission(MISSION_AIRPATROL);
		return;
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/

void CvUnitAI::castAnyTarget(int iThreshold, bool bOtherArea)
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());

	int iSearchRange=getUnitInfo().getArcaneRange();
	int iDX,iDY;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			CvPlot* pLoopPlot	= plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), iDX, iDY);
			if(pLoopPlot!=NULL)
			{
				if(bOtherArea || pLoopPlot->getArea()==getArea())
				{
					if(pLoopPlot->getNumUnits()>0)
					{
						for(int iI=0;iI<GC.getNumSpellInfos();iI++)
						{
							if(GC.getSpellInfo((SpellTypes)iI).isTargetedHostile())
							{
								if(canCastAt((SpellTypes)iI,pLoopPlot))
								{
									if(castDamage((SpellTypes)iI,pLoopPlot,true)/std::max(1,GC.getSpellInfo((SpellTypes)iI).getManaCost())>=iThreshold)
									{
										cast((SpellTypes)iI,pLoopPlot);
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

void CvUnitAI::AI_workerSearchForFreeImprovementMove()
{
	CvPlot* pTarget=NULL;
	CvPlayer& kPlayer=GET_PLAYER(getOwnerINLINE());
	if(plot()->getWorkingCity()!=NULL)
	{
		if(plot()->getWorkingCity()->calculateImprovementInfrastructureCostFreeLeft()>25)
		{
			pTarget=plot()->getWorkingCity()->plot();
		}
	}

	if(pTarget!=NULL)
	{
		int iLoop,iPathTurns;
		int iValue, iBestValue=0;
		for(CvCity* pLoopCity=kPlayer.firstCity(&iLoop);pLoopCity!=NULL;pLoopCity=kPlayer.nextCity(&iLoop))
		{
			if(pLoopCity->getArea()==getArea())
			{
				if(pLoopCity->calculateImprovementInfrastructureCostFreeLeft()>25)
				{
					if(generatePath(pLoopCity->plot(),0,false,&iPathTurns))
					{
						iValue=1000/(3+iPathTurns);
						if(iValue>iBestValue)
						{
							iBestValue=iValue;
							pTarget=pLoopCity->plot();
						}
					}
				}
			}
		}
	}

	if(pTarget!=NULL)
	{
		getGroup()->pushMission(MISSION_MOVE_TO,plot()->getWorkingCity()->plot()->getX_INLINE(),plot()->getWorkingCity()->plot()->getY_INLINE());
		if(atPlot(pTarget))
		{
			getGroup()->setAutomateType(NO_AUTOMATE);
		}
	}

	getGroup()->pushMission(MISSION_SKIP);
	return;
}
