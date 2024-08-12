// unit.cpp

#include "CvGameCoreDLL.h"
#include "CvUnit.h"
#include "CvArea.h"
#include "CvPlot.h"
#include "CvCity.h"
#include "CvGlobals.h"
#include "CvGameCoreUtils.h"
#include "CvGameAI.h"
#include "CvMap.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CyUnit.h"
#include "CyArgsList.h"
#include "CyPlot.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvEventReporter.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "CvPopupInfo.h"
#include "CvArtFileMgr.h"
#include "CvGameTextMgr.h"


//TEMPFIX SEPHI
#include "math.h"
//TEMP PARALLEL TEST
/**
#include "tbb/include/parallel_for.h"
static const size_t N = 23;

class ApplyFoo {
	float *const my_a;
public:
	void operator()( const tbb::blocked_range<size_t>& r ) const {
		float *a = my_a;
		for( size_t i=10; i!=10000; ++i)
			perFormanceTest();
	}
	ApplyFoo( float a[] ):
		my_a(a)
	{}
};

void perFormanceTest()
{
	CvPlayer &kPlayer=GET_PLAYER((PlayerTypes)0);

	int iLoop;

	CvUnit* pUnit=kPlayer.firstUnit(&iLoop);
	if(pUnit!=NULL)
	{
		for(int iI=0;iI<MAX_INT;iI++)
		{
			for(int iJ=0;iJ<MAX_INT;iJ++)
			{
				for(int iL=0;iL<MAX_INT;iL++)
				{
					for(int iK=0;iK<MAX_INT;iK++)
					{
						int i=iI*(iJ+iL)-iK;
						i=i*i;
						i=sqrt((double)i);
					}
				}
			}
		}
	}
}
**/
//TEMPFIX SEPHI

// Public Functions...


CvUnit::CvUnit()
{
	m_aiExtraDomainModifier = new int[NUM_DOMAIN_TYPES];

/*************************************************************************************************/
/**	ADDON (New Functions) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	m_piDenyPromotion = NULL;
	m_piDurability = NULL;
	m_piEquipment = NULL;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

//FfH Damage Types: Added by Kael 08/23/2007
	m_paiBonusAffinity = NULL;
	m_paiBonusAffinityAmount = NULL;
	m_paiDamageTypeCombat = NULL;
	m_paiDamageTypeResist = NULL;
	m_paiEquipmentDurabilityValidityCounter = NULL;
//FfH: End Add

	m_pabHasPromotion = NULL;

	m_paiTerrainDoubleMoveCount = NULL;
	m_paiFeatureDoubleMoveCount = NULL;
	m_paiExtraTerrainAttackPercent = NULL;
	m_paiExtraTerrainDefensePercent = NULL;
	m_paiExtraFeatureAttackPercent = NULL;
	m_paiExtraFeatureDefensePercent = NULL;
	m_paiExtraUnitCombatModifier = NULL;

	CvDLLEntity::createUnitEntity(this);		// create and attach entity to unit

	reset(0, NO_UNIT, NO_PLAYER, true);
}


CvUnit::~CvUnit()
{
	if (!gDLL->GetDone() && GC.IsGraphicsInitialized())						// don't need to remove entity when the app is shutting down, or crash can occur
	{
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		CvDLLEntity::removeEntity();		// remove entity from engine
	}

	CvDLLEntity::destroyEntity();			// delete CvUnitEntity and detach from us

	uninit();

	SAFE_DELETE_ARRAY(m_aiExtraDomainModifier);
}

void CvUnit::reloadEntity()
{

//FfH: Added by Kael 07/05/2009 (fixes a crash when promotions are applid before the game is loaded)
	bool bSelected = IsSelected();
//FfH: End Add

	//destroy old entity
	if (!gDLL->GetDone() && GC.IsGraphicsInitialized())						// don't need to remove entity when the app is shutting down, or crash can occur
	{
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		CvDLLEntity::removeEntity();		// remove entity from engine
	}

	CvDLLEntity::destroyEntity();			// delete CvUnitEntity and detach from us

	//creat new one
	CvDLLEntity::createUnitEntity(this);		// create and attach entity to unit
	setupGraphical();


//FfH: Added by Kael 07/05/2009 (fixes a crash when promotions are applid before the game is loaded)
	if (bSelected)
	{
		gDLL->getInterfaceIFace()->selectUnit(this, false, false, false);
	}
//FfH: End Add

}


void CvUnit::init(int iID, UnitTypes eUnit, UnitAITypes eUnitAI, PlayerTypes eOwner, int iX, int iY, DirectionTypes eFacingDirection)
{
	CvWString szBuffer;
	int iUnitName;
	int iI, iJ;

	FAssert(NO_UNIT != eUnit);

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	//	If the current viewport is not yet initialized center it on the first unit created for the active player
	if ( GC.getGameINLINE().getActivePlayer() == eOwner && GC.getCurrentViewport().getState() == VIEWPORT_MODE_UNINITIALIZED )
	{
		GC.getCurrentViewport().setOffsetToShow(iX, iY);
	}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	//--------------------------------
	// Init saved data
	reset(iID, eUnit, eOwner);

	if(eFacingDirection == NO_DIRECTION)
		m_eFacingDirection = DIRECTION_SOUTH;
	else
		m_eFacingDirection = eFacingDirection;

	//--------------------------------
	// Init containers

	//--------------------------------
	// Init pre-setup() data
	setXY(iX, iY, false, false);

	//--------------------------------
	// Init non-saved data
	setupGraphical();

	//--------------------------------
	// Init other game data
	plot()->updateCenterUnit();

	plot()->setFlagDirty(true);

	iUnitName = GC.getGameINLINE().getUnitCreatedCount(getUnitType());
	int iNumNames = m_pUnitInfo->getNumUnitNames();
	if (iUnitName < iNumNames)
	{
		int iOffset = GC.getGameINLINE().getSorenRandNum(iNumNames, "Unit name selection");

		for (iI = 0; iI < iNumNames; iI++)
		{
			int iIndex = (iI + iOffset) % iNumNames;
			CvWString szName = gDLL->getText(m_pUnitInfo->getUnitNames(iIndex));
			if (!GC.getGameINLINE().isGreatPersonBorn(szName))
			{
				setName(szName);
				GC.getGameINLINE().addGreatPersonBornName(szName);
				break;
			}
		}
	}

	setGameTurnCreated(GC.getGameINLINE().getGameTurn());

	GC.getGameINLINE().incrementUnitCreatedCount(getUnitType());

	GC.getGameINLINE().incrementUnitClassCreatedCount((UnitClassTypes)(m_pUnitInfo->getUnitClassType()));
	GET_TEAM(getTeam()).changeUnitClassCount(((UnitClassTypes)(m_pUnitInfo->getUnitClassType())), 1);
	GET_PLAYER(getOwnerINLINE()).changeUnitClassCount(((UnitClassTypes)(m_pUnitInfo->getUnitClassType())), 1);

	GET_PLAYER(getOwnerINLINE()).changeExtraUnitCost(m_pUnitInfo->getExtraCost());

	if (m_pUnitInfo->getNukeRange() != -1)
	{
		GET_PLAYER(getOwnerINLINE()).changeNumNukeUnits(1);
	}

	if (m_pUnitInfo->isMilitarySupport())
	{
		GET_PLAYER(getOwnerINLINE()).changeNumMilitaryUnits(1);
	}
/*************************************************************************************************/
/**	ADDON (New Mana) Sephi                                                   					**/
/*************************************************************************************************/
    GET_PLAYER(getOwnerINLINE()).changeFaithUpkeep(m_pUnitInfo->getFaithUpkeep());
    GET_PLAYER(getOwnerINLINE()).changeManaUpkeep(m_pUnitInfo->getManaUpkeep());
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	doCitySupport(false);

	GET_PLAYER(getOwnerINLINE()).changeAssets(m_pUnitInfo->getAssetValue());

	updateBaseCombatStrength();

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (m_pUnitInfo->getFreePromotions(iI))
		{
			setHasPromotion(((PromotionTypes)iI), true);
		}
	}

	if (isSummon())
	{
		/*oosLog(
		"AIsetXY"
		,"Turn:%d,PlayerID:%d,UnitID:%d,init,isSummon:%d"
		,GC.getGameINLINE().getElapsedGameTurns()
		,getOwnerINLINE()
		,getID()
		,isSummon()
		);*/
		GET_PLAYER(getOwnerINLINE()).changeNumSummonUnits(1);
	}

	FAssertMsg((GC.getNumTraitInfos() > 0), "GC.getNumTraitInfos() is less than or equal to zero but is expected to be larger than zero in CvUnit::init");
	for (iI = 0; iI < GC.getNumTraitInfos(); iI++)
	{
		if (GET_PLAYER(getOwnerINLINE()).hasTrait((TraitTypes)iI))
		{
			for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if (GC.getTraitInfo((TraitTypes) iI).isFreePromotion(iJ))
				{
					if ((getUnitCombatType() != NO_UNITCOMBAT) && GC.getTraitInfo((TraitTypes) iI).isFreePromotionUnitCombat(getUnitCombatType()))
					{
						setHasPromotion(((PromotionTypes)iJ), true);
					}
				}
			}
		}
	}

	if (NO_UNITCOMBAT != getUnitCombatType())
	{
		for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
		{
			if (GET_PLAYER(getOwnerINLINE()).isFreePromotion(getUnitCombatType(), (PromotionTypes)iJ))
			{
				setHasPromotion(((PromotionTypes)iJ), true);
			}
		}
	}

	if (NO_UNITCLASS != getUnitClassType())
	{
		for (iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
		{
			if (GET_PLAYER(getOwnerINLINE()).isFreePromotion(getUnitClassType(), (PromotionTypes)iJ))
			{
				setHasPromotion(((PromotionTypes)iJ), true);
			}
		}
	}

	if (getDomainType() == DOMAIN_LAND)
	{
		if (baseCombatStr() > 0)
		{
			if ((GC.getGameINLINE().getBestLandUnit() == NO_UNIT) || (baseCombatStr() > GC.getGameINLINE().getBestLandUnitCombat()))
			{
				GC.getGameINLINE().setBestLandUnit(getUnitType());
			}
		}
	}

	if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		gDLL->getInterfaceIFace()->setDirty(GameData_DIRTY_BIT, true);
	}

	if (isWorldUnitClass((UnitClassTypes)(m_pUnitInfo->getUnitClassType()))

//FfH: Added by Kael 11/05/2007
      && GC.getGameINLINE().getUnitClassCreatedCount((UnitClassTypes)(m_pUnitInfo->getUnitClassType())) == 1
//FfH: End Add

	)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				if (GET_TEAM(getTeam()).isHasMet(GET_PLAYER((PlayerTypes)iI).getTeam()))
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_CREATED_UNIT", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
				}
				else
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_UNKNOWN_CREATED_UNIT", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));
				}
			}
		}

		szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_CREATED_UNIT", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey());
		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getOwnerINLINE(), szBuffer, getX_INLINE(), getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"));
	}

	AI_init(eUnitAI);
//FfH Units: Added by Kael 04/18/2008
	if (m_pUnitInfo->getFreePromotionPick() > 0)
	{
	    changeFreePromotionPick(m_pUnitInfo->getFreePromotionPick());
		//testPromotionReady();
        setPromotionReady(true);
    }
    GC.getGameINLINE().changeGlobalCounter(m_pUnitInfo->getModifyGlobalCounter());
	m_iReligion = m_pUnitInfo->getReligionType();
    for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
    {
        changeBonusAffinity((BonusTypes)iI, m_pUnitInfo->getBonusAffinity((BonusTypes)iI));
    }

    if (m_pUnitInfo->isMechUnit())
    {
        changeAlive(1);
    }
    if (GC.getCivilizationInfo(getCivilizationType()).getDefaultRace() != NO_PROMOTION)
    {
        if (getRace() == NO_PROMOTION)
        {
            if (!::isWorldUnitClass(getUnitClassType()) && !isAnimal() && isAlive() && getDomainType() == DOMAIN_LAND)
            {
                setHasPromotion((PromotionTypes)GC.getCivilizationInfo(getCivilizationType()).getDefaultRace(), true);
            }
        }
    }
//FfH: End Add

/*************************************************************************************************/
/**	ADDON (New Functions) Sephi                                     					**/
/*************************************************************************************************/

	if(getUnitType()==GC.getEQUIPMENT_PIECES_OF_BARNAXUS())
	{
		GC.getGameINLINE().changePiecesOfBarnaxusCounter(1);
	}

	if(getCombatAuraType()==NO_COMBATAURA)
	{
		setCombatAuraType(m_pUnitInfo->getCombatAuraType());
	}

	if(getUnitCombatType()==GC.getUNITCOMBAT_ADEPT() || m_pUnitInfo->isArcaneAura())
	{
		if(GC.getInfoTypeForString("COMBATAURA_ARCANE_AURA")!=NO_COMBATAURA)
		{
			if(getCombatAuraType()==NO_COMBATAURA)
			{
				setCombatAuraType(GC.getInfoTypeForString("COMBATAURA_ARCANE_AURA"));
			}

			for(int iI=0;iI<GC.getNumTechInfos();iI++)
			{
				if(GC.getTechInfo((TechTypes)iI).getArcaneAuraPromotion()!=NO_PROMOTION)
				{
					if(GET_TEAM(getTeam()).isHasTech((TechTypes)iI))
					{
						setHasPromotion((PromotionTypes)GC.getTechInfo((TechTypes)iI).getArcaneAuraPromotion(),true);
					}
				}
			}
		}
	}

	changeExtraLives(m_pUnitInfo->getExtraLives());
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Houses of Erebus) Sephi			                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	for (int i = 0; i < GC.getNumCorporationInfos(); i++)
	{
		if (GC.getUnitInfo(getUnitType()).getCorporationSupport((CorporationTypes)i)!=0)
		{
			GET_PLAYER(getOwnerINLINE()).changeCorporationSupport((CorporationTypes)i,GC.getUnitInfo(getUnitType()).getCorporationSupport((CorporationTypes)i));
		}
	}

	for (int i=0;i<GC.getNumCorporationInfos();i++)
	{
		if (GC.getUnitInfo(getUnitType()).getCorporationSupportMultiplier((CorporationTypes)i)!=0)
		{
			GET_PLAYER(getOwnerINLINE()).changeCorporationSupportMultiplier((CorporationTypes)i,GC.getUnitInfo(getUnitType()).getCorporationSupportMultiplier((CorporationTypes)i));
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bUnitCreate                                                                             **/
/*************************************************************************************************/
        for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
        {
            if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
            {
                CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
                for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
                {
					if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
					{
						if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isUnitCreated() && GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getUnitType()==eUnit)
						{
							GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
						}
					}
                }
            }
        }
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/
	CvEventReporter::getInstance().unitCreated(this);

// Better Unit Power (Added by Skyre)
	// Initialise power rating
	updatePower();
// End Better Unit Power
}


void CvUnit::uninit()
{
/*************************************************************************************************/
/**	ADDON (New Functions) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	SAFE_DELETE_ARRAY(m_piDenyPromotion);
	SAFE_DELETE_ARRAY(m_piDurability);	
	SAFE_DELETE_ARRAY(m_piEquipment);	
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//FfH Damage Types: Added by Kael 08/23/2007
	SAFE_DELETE_ARRAY(m_paiBonusAffinity);
	SAFE_DELETE_ARRAY(m_paiBonusAffinityAmount);
	SAFE_DELETE_ARRAY(m_paiDamageTypeCombat);
	SAFE_DELETE_ARRAY(m_paiDamageTypeResist);
	SAFE_DELETE_ARRAY(m_paiEquipmentDurabilityValidityCounter);
//FfH: End Add

	SAFE_DELETE_ARRAY(m_pabHasPromotion);

	SAFE_DELETE_ARRAY(m_paiTerrainDoubleMoveCount);
	SAFE_DELETE_ARRAY(m_paiFeatureDoubleMoveCount);
	SAFE_DELETE_ARRAY(m_paiExtraTerrainAttackPercent);
	SAFE_DELETE_ARRAY(m_paiExtraTerrainDefensePercent);
	SAFE_DELETE_ARRAY(m_paiExtraFeatureAttackPercent);
	SAFE_DELETE_ARRAY(m_paiExtraFeatureDefensePercent);
	SAFE_DELETE_ARRAY(m_paiExtraUnitCombatModifier);
}


// FUNCTION: reset()
// Initializes data members that are serialized.
void CvUnit::reset(int iID, UnitTypes eUnit, PlayerTypes eOwner, bool bConstructorCall)
{
	int iI;

	//--------------------------------
	// Uninit class
	uninit();

	m_iID = iID;
	m_iGroupID = FFreeList::INVALID_INDEX;
	m_iHotKeyNumber = -1;
	m_iX = INVALID_PLOT_COORD;
	m_iY = INVALID_PLOT_COORD;
	m_iLastMoveTurn = 0;
	m_iReconX = INVALID_PLOT_COORD;
	m_iReconY = INVALID_PLOT_COORD;
	m_iGameTurnCreated = 0;
	m_iDamage = 0;
	m_iMoves = 0;
	m_iExperience = 0;
	m_iLevel = 1;
	m_iCargo = 0;
	m_iAttackPlotX = INVALID_PLOT_COORD;
	m_iAttackPlotY = INVALID_PLOT_COORD;
	m_iCombatTimer = 0;
	m_iCombatFirstStrikes = 0;
	m_iFortifyTurns = 0;
	m_iBlitzCount = 0;
	m_iAmphibCount = 0;
	m_iRiverCount = 0;
	m_iEnemyRouteCount = 0;
	m_iAlwaysHealCount = 0;
	m_iHillsDoubleMoveCount = 0;
	m_iImmuneToFirstStrikesCount = 0;
	m_iExtraVisibilityRange = 0;
	m_iExtraMoves = 0;
	m_iExtraMoveDiscount = 0;
	m_iExtraAirRange = 0;
	m_iExtraIntercept = 0;
	m_iExtraEvasion = 0;
	m_iExtraFirstStrikes = 0;
	m_iExtraChanceFirstStrikes = 0;
	m_iExtraWithdrawal = 0;
	m_iExtraCollateralDamage = 0;
	m_iExtraBombardRate = 0;
	m_iExtraEnemyHeal = 0;
	m_iExtraNeutralHeal = 0;
	m_iExtraFriendlyHeal = 0;
	m_iSameTileHeal = 0;
	m_iAdjacentTileHeal = 0;
	m_iExtraCombatPercent = 0;
	m_iExtraCityAttackPercent = 0;
	m_iExtraCityDefensePercent = 0;
	m_iExtraHillsAttackPercent = 0;
	m_iExtraHillsDefensePercent = 0;
	m_iRevoltProtection = 0;
	m_iCollateralDamageProtection = 0;
	m_iPillageChange = 0;
	m_iUpgradeDiscount = 0;
	m_iExperiencePercent = 0;
	m_iKamikazePercent = 0;
	m_eFacingDirection = DIRECTION_SOUTH;
	m_iImmobileTimer = 0;

	m_bMadeAttack = false;
	m_bMadeInterception = false;
	m_bPromotionReady = false;
	m_bDeathDelay = false;
	m_bCombatFocus = false;
	m_bInfoBarDirty = false;
	m_bBlockading = false;
	m_bAirCombat = false;

	m_eOwner = eOwner;
	m_eCapturingPlayer = NO_PLAYER;
	m_eUnitType = eUnit;
	m_pUnitInfo = (NO_UNIT != m_eUnitType) ? &GC.getUnitInfo(m_eUnitType) : NULL;
	m_iBaseCombat = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getCombat() : 0;
	m_iBaseCombatFromPromotion = 0;
	m_iBaseCombatFromPotion = 0;
	m_iBaseCombatDefense = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getCombatDefense() : 0;
	m_iBaseCombatDefenseFromPromotion = 0;
	m_iBaseCombatDefenseFromPotion = 0;
	m_eLeaderUnitType = NO_UNIT;
	m_iCargoCapacity = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getCargoSpace() : 0;

//FfH Spell System: Added by Kael 07/23/2007
	m_bFleeWithdrawl = false;
	m_bHasCasted = false;
	m_bIgnoreHide = false;
	m_iAlive = 0;
	m_iAIControl = 0;
	m_iBoarding = 0;
	m_iDefensiveStrikeChance = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getDefensiveStrikeChance() : 0;
	m_iDefensiveStrikeDamage = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getDefensiveStrikeDamage() : 0;
	m_iDoubleFortifyBonus = 0;
	m_iFear = 0;
	m_iFlying = 0;
	m_iHeld = 0;
//>>>>Advanced Rules: Modified by Denev 2009/12/17
//*** fix implicit casting from boolean to integer.
/*
	m_iHiddenNationality = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->isHiddenNationality() : 0;
	m_iIgnoreBuildingDefense = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->isIgnoreBuildingDefense() : 0;
	m_iImmortal = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->isImmortal() : 0;
*/
	m_iHiddenNationality = (NO_UNIT != m_eUnitType && m_pUnitInfo->isHiddenNationality()) ? 1 : 0;
	m_iIgnoreBuildingDefense = (NO_UNIT != m_eUnitType && m_pUnitInfo->isIgnoreBuildingDefense()) ? 1 : 0;
	m_iImmortal = (NO_UNIT != m_eUnitType && m_pUnitInfo->isImmortal()) ? 1 : 0;
//<<<<Advanced Rules: End Modify
	m_iImmuneToCapture = 0;
	m_iImmuneToRangeStrike = 0;
//>>>>Advanced Rules: Modified by Denev 2009/12/17
//*** fix implicit casting from boolean to integer.
//	m_iImmuneToDefensiveStrike = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->isImmuneToDefensiveStrike() : 0;
	m_iImmuneToDefensiveStrike = (NO_UNIT != m_eUnitType && m_pUnitInfo->isImmuneToDefensiveStrike()) ? 1 : 0;
//<<<<Advanced Rules: End Modify

	m_iImmuneToFear = 0;
	m_iImmuneToMagic = 0;
	m_iInvisible = 0;
	m_iOnlyDefensive = 0;
	m_iSeeInvisible = 0;
	m_iTargetWeakestUnit = 0;
	m_iTargetWeakestUnitCounter = 0;
	m_iTwincast = 0;
	m_iWaterWalking = 0;
	m_iBetterDefenderThanPercent = 100;
	m_iCombatHealPercent = 0;
	m_iCombatManaGained = 0;
	m_iCombatLimit = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getCombatLimit() : 0;
	m_iCombatPercentInBorders = 0;
	m_iCombatPercentDefense = 0;
	m_iCombatPercentGlobalCounter = 0;
	m_iDelayedSpell = NO_SPELL;
	m_iDuration = 0;
	m_iFreePromotionPick = 0;
	m_iGoldFromCombat = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getGoldFromCombat() : 0;
	m_iGroupSize = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getGroupSize() : 0;
	m_iInvisibleType = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getInvisibleType() : 0;
	m_iRace = NO_PROMOTION;
    m_iReligion = NO_RELIGION;
	m_iResist = 0;
	m_iResistModify = 0;
	m_iScenarioCounter = -1;
	m_iSpellCasterXP = 0;
	m_iSpellDamageModify = 0;
	m_iSummoner = -1;
	m_iTotalDamageTypeCombat = 0;
    m_iUnitArtStyleType = NO_UNIT_ARTSTYLE;
	m_iWorkRateModify = 0;

	if (!bConstructorCall)
	{
        m_paiDamageTypeCombat = new int[GC.getNumDamageTypeInfos()];
        m_paiDamageTypeResist = new int[GC.getNumDamageTypeInfos()];
        for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
        {
            int iChange = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getDamageTypeCombat(iI) : 0;
            m_paiDamageTypeCombat[iI] = iChange;
            m_paiDamageTypeResist[iI] = 0;
            m_iTotalDamageTypeCombat += iChange;
        }
        m_paiBonusAffinity = new int[GC.getNumBonusInfos()];
        m_paiBonusAffinityAmount = new int[GC.getNumBonusInfos()];
        for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
        {
            m_paiBonusAffinity[iI] = 0;
            m_paiBonusAffinityAmount[iI] = 0;
        }

        m_paiEquipmentDurabilityValidityCounter = new int[GC.getNumDurabilityInfos()];
        for (iI = 0; iI < GC.getNumDurabilityInfos(); iI++)
        {
            m_paiEquipmentDurabilityValidityCounter[iI] = 0;
        }

	}
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (New Functions Definition) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	m_iFearEffect = 0;
	m_iBonusVSUnitsFullHealth = 0;
    m_iCannotLeaveCulture = 0;
    m_iDefensiveStrikesLeft = 2;
    m_iNoXP = (NO_UNIT == m_eUnitType || !m_pUnitInfo->isNoXP()) ?  0 : 1;
	m_iSpellTargetIncrease = 0;
	m_iBonusSummonStrength = 0;
    m_iAutoSpellCast = NO_SPELL;
	m_iMissionSpell = NO_SPELL;
	m_iExtraLives = 0;
	m_iStrengthPotionsUsed = 0;
	m_iCombatAuraID = FFreeList::INVALID_INDEX;
	m_iCombatAuraType = NO_COMBATAURA;
	m_iBlessing1 = FFreeList::INVALID_INDEX;
	m_iBlessing2 = FFreeList::INVALID_INDEX;
	m_iCurse1 = FFreeList::INVALID_INDEX;
	m_iCurse2 = FFreeList::INVALID_INDEX;

	m_iCurse1Owner = NO_PLAYER;
	m_iCurse2Owner = NO_PLAYER;
	m_iBlessing1Owner = NO_PLAYER;
	m_iBlessing2Owner = NO_PLAYER;

	m_iIgnoreFirstStrikes = 0;
	m_iCaptureAnimalChance = 0;
	m_iCaptureBeastChance = 0;
    m_bGarrision = false;
	m_bFlamingArrows = false;
	m_iEquipmentYieldType = NO_YIELD;

	m_iAIGroupID = FFreeList::INVALID_INDEX;

	if (!bConstructorCall)
	{
        m_piDenyPromotion = new int[GC.getNumPromotionInfos()];
        for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
        {
            m_piDenyPromotion[iI] = 0;
		}

		m_piDurability = new int[GC.getNumEquipmentCategoryInfos()];
		for (iI = 0; iI< GC.getNumEquipmentCategoryInfos(); iI++)
		{
			m_piDurability[iI] = 0;
		}

		m_piEquipment = new int[GC.getNumEquipmentCategoryInfos()];
		for (iI = 0; iI< GC.getNumEquipmentCategoryInfos(); iI++)
		{
			m_piEquipment[iI] = NO_PROMOTION;
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/*************************************************************************************************/
    m_iAirCombat = (NO_UNIT != m_eUnitType) ? m_pUnitInfo->getAirCombat() : 0;
    m_iAirCombatLimitBoost = 0;
    m_bAutomatedRangeStrike = false;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//>>>>Refined Defensive Strikes: Added by Denev 2009/11/10
	m_iCombatDefensiveStrikes = -1;
//<<<<Refined Defensive Strikes: End Add
//>>>>Refined Defensive Strikes: Added by Denev 2009/11/10
	m_iDefensiveStrikes = (NO_UNIT != m_eUnitType && m_pUnitInfo->isDefensiveStrikes()) ? 1 : 0;
//<<<<Refined Defensive Strikes: End Add

// Better Unit Power (Added by Skyre)
	m_iPower = 0;
// End Better Unit Power

	m_combatUnit.reset();
	m_transportUnit.reset();

	for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
	{
		m_aiExtraDomainModifier[iI] = 0;
	}

	m_szName.clear();
	m_szScriptData ="";

	if (!bConstructorCall)
	{
		FAssertMsg((0 < GC.getNumPromotionInfos()), "GC.getNumPromotionInfos() is not greater than zero but an array is being allocated in CvUnit::reset");
		m_pabHasPromotion = new bool[GC.getNumPromotionInfos()];
		for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
		{
			m_pabHasPromotion[iI] = false;
		}

		FAssertMsg((0 < GC.getNumTerrainInfos()), "GC.getNumTerrainInfos() is not greater than zero but a float array is being allocated in CvUnit::reset");
		m_paiTerrainDoubleMoveCount = new int[GC.getNumTerrainInfos()];
		m_paiExtraTerrainAttackPercent = new int[GC.getNumTerrainInfos()];
		m_paiExtraTerrainDefensePercent = new int[GC.getNumTerrainInfos()];
		for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
		{
			m_paiTerrainDoubleMoveCount[iI] = 0;
			m_paiExtraTerrainAttackPercent[iI] = 0;
			m_paiExtraTerrainDefensePercent[iI] = 0;
		}

		FAssertMsg((0 < GC.getNumFeatureInfos()), "GC.getNumFeatureInfos() is not greater than zero but a float array is being allocated in CvUnit::reset");
		m_paiFeatureDoubleMoveCount = new int[GC.getNumFeatureInfos()];
		m_paiExtraFeatureDefensePercent = new int[GC.getNumFeatureInfos()];
		m_paiExtraFeatureAttackPercent = new int[GC.getNumFeatureInfos()];
		for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
		{
			m_paiFeatureDoubleMoveCount[iI] = 0;
			m_paiExtraFeatureAttackPercent[iI] = 0;
			m_paiExtraFeatureDefensePercent[iI] = 0;
		}

		FAssertMsg((0 < GC.getNumUnitCombatInfos()), "GC.getNumUnitCombatInfos() is not greater than zero but an array is being allocated in CvUnit::reset");
		m_paiExtraUnitCombatModifier = new int[GC.getNumUnitCombatInfos()];
		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			m_paiExtraUnitCombatModifier[iI] = 0;
		}

		AI_reset();
	}
}


//////////////////////////////////////
// graphical only setup
//////////////////////////////////////
void CvUnit::setupGraphical()
{
	if (!GC.IsGraphicsInitialized())
	{
		return;
	}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	if (!isInViewport())
	{
		return;
	}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	CvDLLEntity::setup();

	if (getGroup()->getActivityType() == ACTIVITY_INTERCEPT)
	{
		airCircle(true);
	}
}


void CvUnit::convert(CvUnit* pUnit)
{
	CvPlot* pPlot = plot();

//FfH: Modified by Kael 08/21/2008
//	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
//	{
//		setHasPromotion(((PromotionTypes)iI), (pUnit->isHasPromotion((PromotionTypes)iI) || m_pUnitInfo->getFreePromotions(iI)));
//  }
    if (getRace() != NO_PROMOTION)
    {
		if(pUnit->getRace() != NO_PROMOTION)
		{
			if (!m_pUnitInfo->getFreePromotions(getRace()))
			{
				setHasPromotion(((PromotionTypes)getRace()), false);
			}
			else
			{
				pUnit->setHasPromotion(((PromotionTypes)pUnit->getRace()), false);
			}
		}
    }
    if (pUnit->isHasPromotion((PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION")))
    {
        CvUnit* pLoopUnit;
        CLLNode<IDInfo>* pUnitNode;
        pUnitNode = pPlot->headUnitNode();
        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = pPlot->nextUnitNode(pUnitNode);
            if (pLoopUnit->getTeam() != getTeam())
            {
                pUnit->setHasPromotion((PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION"), false);
            }
        }
    }
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
        if (pUnit->isHasPromotion((PromotionTypes)iI))
        {
            setHasPromotion(((PromotionTypes)iI), true);
			CvPromotionInfo &kPromotion=GC.getPromotionInfo((PromotionTypes)iI);
            if (kPromotion.isEquipment())
            {
                pUnit->setHasPromotion((PromotionTypes)iI, false);
            }
            if (kPromotion.isValidate())
            {
				bool bValid=true;

				int iNumPrereqs = kPromotion.getNumPrereqCivilizations();

				if (iNumPrereqs > 0)
				{
					bValid = false;
					for (int iI = 0; iI < iNumPrereqs; iI++)
					{
						if (GET_PLAYER(getOwner()).getCivilizationType() == (CivilizationTypes)kPromotion.getPrereqCivilization(iI))
						{
							bValid = true;
							break;
						}
					}
				}

				if(getUnitCombatType()==NO_UNITCOMBAT)
				{
					if(!GC.getPromotionInfo((PromotionTypes)iI).isAnyUnitCombat())
					{
						bValid = false;
					}
				}
				else
				{
					if (!GC.getPromotionInfo((PromotionTypes)iI).getUnitCombat(getUnitCombatType()))
					{
						bValid = false;
					}
				}

				if(!bValid)
				{
					setHasPromotion(((PromotionTypes)iI), false);
				}
            }
        }
    }
    if (m_pUnitInfo->getFreePromotionPick() > 0 && getGameTurnCreated() == GC.getGameINLINE().getGameTurn())
	{
		//testPromotionReady();
        setPromotionReady(true);
    }
    setDuration(pUnit->getDuration());
    if (pUnit->getReligion() != NO_RELIGION && getReligion() == NO_RELIGION)
    {
        setReligion(pUnit->getReligion());
    }
    if (pUnit->isImmortal())
    {
        pUnit->changeImmortal(-1);
    }
    if (pUnit->isHasCasted())
    {
        setHasCasted(true);
    }
    if (pUnit->getScenarioCounter() != -1)
    {
        setScenarioCounter(pUnit->getScenarioCounter());
    }
//FfH: End Modify

	for(int iI=0;iI<GC.getNumEquipmentCategoryInfos();iI++)
	{
		PromotionTypes ePromotion=pUnit->getEquipment((EquipmentCategoryTypes)iI);
		if(ePromotion!=NO_PROMOTION && isHasPromotion(ePromotion))
		{
			setEquipment((EquipmentCategoryTypes)iI,pUnit->getEquipment((EquipmentCategoryTypes)iI));
		}
	}	

	setGameTurnCreated(pUnit->getGameTurnCreated());
	setDamage(pUnit->getDamage());
	setMoves(pUnit->getMoves());

	setLevel(pUnit->getLevel());
	int iOldModifier = std::max(1, 100 + GET_PLAYER(pUnit->getOwnerINLINE()).getLevelExperienceModifier());
	int iOurModifier = std::max(1, 100 + GET_PLAYER(getOwnerINLINE()).getLevelExperienceModifier());
	setExperience(std::max(0, (pUnit->getExperience() * iOurModifier) / iOldModifier));

	setName(pUnit->getNameNoDesc());
	setLeaderUnitType(pUnit->getLeaderUnitType());

/*************************************************************************************************/
/**	BETTER AI (New UNITAI) Sephi                                 					            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

    AI_setGroupflag(pUnit->AI_getGroupflag());
    AI_setUnitAIType(pUnit->AI_getUnitAIType());

    //If Wizards upgrade to Immortals
    if(pUnit->AI_getUnitAIType()==UNITAI_WARWIZARD &&
        GC.getUnitInfo(getUnitType()).getDefaultUnitAIType()!=UNITAI_MAGE)
    {
        AI_setUnitAIType(UNITAI_BARBSMASHER);
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//FfH: Added by Kael 10/03/2008
	if (!isWorldUnitClass((UnitClassTypes)(m_pUnitInfo->getUnitClassType())) && isWorldUnitClass((UnitClassTypes)(pUnit->getUnitClassType())))
	{
	    setName(pUnit->getName());
	}
//FfH: End Add

	CvUnit* pTransportUnit = pUnit->getTransportUnit();
	if (pTransportUnit != NULL)
	{
		pUnit->setTransportUnit(NULL);
		setTransportUnit(pTransportUnit);
	}

	std::vector<CvUnit*> aCargoUnits;
	pUnit->getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		aCargoUnits[i]->setTransportUnit(this);
	}

	pUnit->kill(true);
}


void CvUnit::kill(bool bDelay, PlayerTypes ePlayer)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pTransportUnit;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	CvWString szBuffer;
	PlayerTypes eOwner;
	PlayerTypes eCapturingPlayer;
	UnitTypes eCaptureUnitType;

	pPlot = plot();
	FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

	static std::vector<IDInfo> oldUnits;
	oldUnits.clear();
	pUnitNode = pPlot->headUnitNode();

	/*if(isOOSLogging())
	{
		oosLog(
			"Kill"
			,"Turn:%d,Player:%d,UnitID:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getOwnerINLINE()
			,getID()
		);
	}*/
	//Extra Lives
	if(getExtraLives() > 0)
	{
		changeExtraLives(-1);
		setDamage(0, NO_PLAYER);
		m_bDeathDelay = false;
		return;
	}

	while (pUnitNode != NULL)
	{
		oldUnits.push_back(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
	}

	for (uint i = 0; i < oldUnits.size(); i++)
	{
		pLoopUnit = ::getUnit(oldUnits[i]);

		if (pLoopUnit != NULL)
		{
			if (pLoopUnit->getTransportUnit() == this)
			{
				//save old units because kill will clear the static list
				std::vector<IDInfo> tempUnits = oldUnits;

				if (pPlot->isValidDomainForLocation(*pLoopUnit))
				{
					pLoopUnit->setCapturingPlayer(NO_PLAYER);
				}

				pLoopUnit->kill(false, ePlayer);

				oldUnits = tempUnits;
			}
		}
	}

	if (ePlayer != NO_PLAYER)
	{

//FfH: Modified by Kael 02/05/2009
//		CvEventReporter::getInstance().unitKilled(this, ePlayer);
        if (!isImmortal())
        {
			CvEventReporter::getInstance().unitKilled(this, ePlayer);

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag isUnitKill, isMazatlAmbushDeepJungle                                                    **/
/*************************************************************************************************/
            UnitTypes eUnit=getUnitType();
            for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
            {
                if(GET_PLAYER(ePlayer).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
                {
                    CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
                    for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
                    {
						if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
						{
							if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isUnitKill())
							{
							    if (GET_PLAYER(ePlayer).VerifyAdventureProgress(kAdv.getAdventureStep(iJ),this))
							    {
									GET_PLAYER(ePlayer).changeAdventureCounter(iAdvLoop,iJ,1);
								}
							}

							if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isMazatlAmbushDeepJungle())
							{
							    if (plot()->getFeatureType()==GC.getInfoTypeForString("FEATURE_LIZ_JUNGLE"))
							    {
									GET_PLAYER(ePlayer).changeAdventureCounter(iAdvLoop,iJ,1);
								}
							}
						}
                    }
                }
            }
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/
        }
//FfH: End Modify

		if (NO_UNIT != getLeaderUnitType())
		{
			for (int iI = 0; iI < MAX_PLAYERS; iI++)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_GENERAL_KILLED", getNameKey());
					gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_MAJOR_EVENT);
				}
			}
		}
	}

	if (bDelay)
	{
		startDelayedDeath();
		return;
	}

	if (isSummon())
	{
		/*oosLog(
		"AIsetXY"
		,"Turn:%d,PlayerID:%d,UnitID:%d,startDelayedDeath:%d,isSummon:%d"
		,GC.getGameINLINE().getElapsedGameTurns()
		,ePlayer
		,getID()
		,bDelay
		,isSummon()
		);*/

		GET_PLAYER(getOwnerINLINE()).changeNumSummonUnits(-1);
	}

//FfH: Added by Kael 07/23/2008
    if (isImmortal())
    {
		if(isImmortalRebirthDestinationExisting())
        //if (GET_PLAYER(getOwnerINLINE()).getCapitalCity() != NULL)
        {
            m_bDeathDelay = false;
            doImmortalRebirth();
            return;
        }
    }

	//resolveEquipment();	//converts Equipment to Global Yields
    GC.getGameINLINE().changeGlobalCounter(-1 * m_pUnitInfo->getModifyGlobalCounter());
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
	    if (isHasPromotion((PromotionTypes)iI))
	    {
            GC.getGameINLINE().changeGlobalCounter(-1 * GC.getPromotionInfo((PromotionTypes)iI).getModifyGlobalCounter());
            if (GC.getPromotionInfo((PromotionTypes)iI).isEquipment())
            {
                for (int iJ = 0; iJ < GC.getNumUnitInfos(); iJ++)
                {
                    if (GC.getUnitInfo((UnitTypes)iJ).getEquipmentPromotion() == iI)
                    {
						//DEBUG
						if(isOOSLogging())
						{
							oosLog(
								"EquipmentPromotion"
								,"Turn: %d,PlayerID: %d, UnitID: %d,EquipmentPromotionCreated: %S,Type: %d"
								,GC.getGameINLINE().getElapsedGameTurns()
								,getOwner()
								,getID()
								,GC.getUnitInfo((UnitTypes)iJ).getDescription()
								,AI_getUnitAIType()
							);
						}
						//DEBUG
                        GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)iJ, getX_INLINE(), getY_INLINE(), AI_getUnitAIType());
                        setHasPromotion((PromotionTypes)iI, false);
                    }
                }
            }
			if(GC.getPromotionInfo((PromotionTypes)iI).getYieldCostType()!=NO_YIELD)
			{
				setHasPromotion((PromotionTypes)iI, false);
			}
	    }
	}
	if (isWorldUnitClass((UnitClassTypes)(m_pUnitInfo->getUnitClassType())) && GC.getGameINLINE().getUnitClassCreatedCount((UnitClassTypes)(m_pUnitInfo->getUnitClassType())) == 1)
	{
		for (int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive() && GET_PLAYER((PlayerTypes)iI).isHuman() && getOwner() != iI)
			{
                szBuffer = gDLL->getText("TXT_KEY_MISC_SOMEONE_KILLED_UNIT", getNameKey());
                gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
			}
		}
	}
	int iLoop;
    for (pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
    {
        if (pLoopUnit->getSummoner() == getID())
        {
            pLoopUnit->setSummoner(-1);
        }
    }
//FfH: End Add

	if (isMadeAttack() && nukeRange() != -1)
	{
		CvPlot* pTarget = getAttackPlot();
		if (pTarget)
		{
			pTarget->nukeExplosion(nukeRange(), this);
			setAttackPlot(NULL, false);
		}
	}

	finishMoves();

	if (IsSelected())
	{
		if (gDLL->getInterfaceIFace()->getLengthSelectionList() == 1)
		{
			if (!(gDLL->getInterfaceIFace()->isFocused()) && !(gDLL->getInterfaceIFace()->isCitySelection()) && !(gDLL->getInterfaceIFace()->isDiploOrPopupWaiting()))
			{
				GC.getGameINLINE().updateSelectionList();
			}

			if (IsSelected())
			{
				gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);
			}
			else
			{
				gDLL->getInterfaceIFace()->setDirty(SelectionCamera_DIRTY_BIT, true);
			}
		}
	}

	gDLL->getInterfaceIFace()->removeFromSelectionList(this);

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/

	// XXX this is NOT a hack, without it, the game crashes.
	gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
/**
	if (isInViewport())
	{
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
	}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/


	FAssertMsg(!isCombat(), "isCombat did not return false as expected");

	pTransportUnit = getTransportUnit();

	if (pTransportUnit != NULL)
	{
		setTransportUnit(NULL);
	}

	setReconPlot(NULL);
	setBlockading(false);

	FAssertMsg(getAttackPlot() == NULL, "The current unit instance's attack plot is expected to be NULL");
	FAssertMsg(getCombatUnit() == NULL, "The current unit instance's combat unit is expected to be NULL");

	GET_TEAM(getTeam()).changeUnitClassCount((UnitClassTypes)m_pUnitInfo->getUnitClassType(), -1);
	GET_PLAYER(getOwnerINLINE()).changeUnitClassCount((UnitClassTypes)m_pUnitInfo->getUnitClassType(), -1);

	GET_PLAYER(getOwnerINLINE()).changeExtraUnitCost(-(m_pUnitInfo->getExtraCost()));

	if (m_pUnitInfo->getNukeRange() != -1)
	{
		GET_PLAYER(getOwnerINLINE()).changeNumNukeUnits(-1);
	}

	if (m_pUnitInfo->isMilitarySupport())
	{
		GET_PLAYER(getOwnerINLINE()).changeNumMilitaryUnits(-1);
	}
/*************************************************************************************************/
/**	ADDON (New Mana) Sephi                                                   					**/
/*************************************************************************************************/
	int iFaithUpkeep=m_pUnitInfo->getFaithUpkeep();
    GET_PLAYER(getOwnerINLINE()).changeFaithUpkeep(-iFaithUpkeep);
	if(iFaithUpkeep<0)
	{
		GET_PLAYER(getOwnerINLINE()).changeFaith(iFaithUpkeep*50);
	}
    GET_PLAYER(getOwnerINLINE()).changeManaUpkeep(-m_pUnitInfo->getManaUpkeep());
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	setEquipmentYieldType(NO_YIELD);
	setGarrision(false);
	doCitySupport(true);

	GET_PLAYER(getOwnerINLINE()).changeAssets(-(m_pUnitInfo->getAssetValue()));

/*************************************************************************************************/
/**	BETTER AI (better UnitPower calculation) Sephi                        	                    **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
/** orig
	GET_PLAYER(getOwnerINLINE()).changePower(-(m_pUnitInfo->getPowerValue()));
**/
	GET_PLAYER(getOwnerINLINE()).changePower(-(getPower()));
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	GET_PLAYER(getOwnerINLINE()).AI_changeNumAIUnits(AI_getUnitAIType(), -1);

	eOwner = getOwnerINLINE();
	eCapturingPlayer = getCapturingPlayer();

//FfH: Modified by Kael 09/01/2007
//	eCaptureUnitType = ((eCapturingPlayer != NO_PLAYER) ? getCaptureUnitType(GET_PLAYER(eCapturingPlayer).getCivilizationType()) : NO_UNIT);
    eCaptureUnitType = ((eCapturingPlayer != NO_PLAYER) ? getCaptureUnitType(GET_PLAYER(getOwnerINLINE()).getCivilizationType()) : NO_UNIT);
    if (m_pUnitInfo->getUnitCaptureClassType() == getUnitClassType())
    {
        eCaptureUnitType = (UnitTypes)getUnitType();
    }
    int iRace = getRace();
//FfH: End Modify

	setXY(INVALID_PLOT_COORD, INVALID_PLOT_COORD, true);

	joinGroup(NULL, false, false);

	CvEventReporter::getInstance().unitLost(this);
/*************************************************************************************************/
/**	ADDON (Combatauras) Sephi																	**/
/*************************************************************************************************/
	if(getBlessing1()!=NULL)
		getBlessing1()->Remove(this);
	if(getBlessing2()!=NULL)
		getBlessing2()->Remove(this);
	if(getCurse1()!=NULL)
		getCurse1()->Remove(this);
	if(getCurse2()!=NULL)
		getCurse2()->Remove(this);
	setCombatAuraType(NO_COMBATAURA);
	
	setAIGroup(NULL);

	if(ePlayer!=NO_PLAYER && ePlayer<GC.getMAX_CIV_PLAYERS())
	{
		TeamTypes eOtherTeam=GET_PLAYER(ePlayer).getTeam();
		if(GET_TEAM(getTeam()).isAtWar(eOtherTeam))
		{
			GET_TEAM(getTeam()).AI_changeWarValueAccumulated(eOtherTeam,-getPower()/30);
			GET_TEAM(eOtherTeam).AI_changeWarValueAccumulated(GET_PLAYER(ePlayer).getTeam(),getPower()/30);
		}
	}

	if(getUnitType()==GC.getEQUIPMENT_PIECES_OF_BARNAXUS())
	{
		GC.getGameINLINE().changePiecesOfBarnaxusCounter(-1);
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	GET_PLAYER(getOwnerINLINE()).deleteUnit(getID());

//FfH: Modified by Kael 01/19/2008
//	if ((eCapturingPlayer != NO_PLAYER) && (eCaptureUnitType != NO_UNIT) && !(GET_PLAYER(eCapturingPlayer).isBarbarian()))
	if ((eCapturingPlayer != NO_PLAYER) && (eCaptureUnitType != NO_UNIT))
//FfH: End Modify

	{
		if (GET_PLAYER(eCapturingPlayer).isHuman() || GET_PLAYER(eCapturingPlayer).AI_captureUnit(eCaptureUnitType, pPlot) || 0 == GC.getDefineINT("AI_CAN_DISBAND_UNITS"))
		{
			CvUnit* pkCapturedUnit = GET_PLAYER(eCapturingPlayer).initUnit(eCaptureUnitType, pPlot->getX_INLINE(), pPlot->getY_INLINE());

//FfH: Added by Kael 08/18/2008
            if (pkCapturedUnit->getRace() != NO_PROMOTION)
            {
                pkCapturedUnit->setHasPromotion((PromotionTypes)pkCapturedUnit->getRace(), false);
            }
            if (iRace != NO_PROMOTION)
            {
                pkCapturedUnit->setHasPromotion((PromotionTypes)iRace, true);
            }
//FfH: End Add

			if (pkCapturedUnit != NULL)
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_CAPTURED_UNIT", GC.getUnitInfo(eCaptureUnitType).getTextKeyWide());
				gDLL->getInterfaceIFace()->addMessage(eCapturingPlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNITCAPTURE", MESSAGE_TYPE_INFO, pkCapturedUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

				// Add a captured mission
				CvMissionDefinition kMission;
				kMission.setMissionTime(GC.getMissionInfo(MISSION_CAPTURED).getTime() * gDLL->getSecsPerTurn());
				kMission.setUnit(BATTLE_UNIT_ATTACKER, pkCapturedUnit);
				kMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
				kMission.setPlot(pPlot);
				kMission.setMissionType(MISSION_CAPTURED);
				gDLL->getEntityIFace()->AddMission(&kMission);

				pkCapturedUnit->finishMoves();

				if (!GET_PLAYER(eCapturingPlayer).isHuman())
				{
					CvPlot* pPlot = pkCapturedUnit->plot();
					if (pPlot && !pPlot->isCity(false))
					{
						if (GET_PLAYER(eCapturingPlayer).AI_getPlotDanger(pPlot) && GC.getDefineINT("AI_CAN_DISBAND_UNITS")

//FfH: Added by Kael 12/02/2007
                          && pkCapturedUnit->canScrap()
//FfH: End Add

						  )
						{
							pkCapturedUnit->kill(false);
						}
					}
				}
			}
		}
	}
}


void CvUnit::NotifyEntity(MissionTypes eMission)
{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	if (!isInViewport())
		return;
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	gDLL->getEntityIFace()->NotifyEntity(getUnitEntity(), eMission);
}


void CvUnit::doTurn()
{
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("Unit::doTurn");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

	PROFILE("CvUnit::doTurn()")

	FAssertMsg(!isDead(), "isDead did not return false as expected");
	FAssertMsg(getGroup() != NULL, "getGroup() is not expected to be equal with NULL");

/*********************************************************************/
/** DEBUG **

    if (GC.getDefineINT("USE_DEBUG_LOG")==1)
    {
        CvString szError="start CvUnit::doTurn()";
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
		sprintf(szOut, "UnitType -- %S\n",GC.getUnitInfo(this->getUnitType()).getDescription());
		gDLL->logMsg("pdebug.log",szOut);
		if (AI_getUnitAIType()!=NO_UNITAI)
		{
            sprintf(szOut, "UnitAIType -- %S\n",GC.getUnitAIInfo(this->AI_getUnitAIType()).getDescription());
            gDLL->logMsg("pdebug.log",szOut);
		}
        sprintf(szOut, "GetX: %d GetY: %d\n",this->getX(), this->getY());
		gDLL->logMsg("pdebug.log",szOut);
        sprintf(szOut, "Groupflag:  %d",this->AI_getGroupflag());
		gDLL->logMsg("pdebug.log",szOut);
        sprintf(szOut, "UnitID: %d, GroupID: %d, Groupsize: %d\n",this->getID(), this->getGroupID(), this->getGroup()->getNumUnits());
		gDLL->logMsg("pdebug.log",szOut);

    }
/** DEBUG **/
/*********************************************************************/

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("Unit::doTurn::zero");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

    int iI;
    CvPlot* pPlot = plot();
	if (hasMoved())
	{
		if (isAlwaysHeal() || isBarbarian())
		{
			doHeal();
		}
	}
	else
	{
		if (isHurt())
		{
			doHeal();
		}

		if (!isCargo())
		{
			changeFortifyTurns(1);
		}
	}

    if (m_pUnitInfo->isAbandon())
    {
        if (!isBarbarian())
        {
            bool bValid = true;
            if (m_pUnitInfo->getPrereqCivic() != NO_CIVIC)
            {
                bValid = false;
                for (int iI = 0; iI < GC.getDefineINT("MAX_CIVIC_OPTIONS"); iI++)
                {
                    if (GET_PLAYER(getOwnerINLINE()).getCivics((CivicOptionTypes)iI) == m_pUnitInfo->getPrereqCivic())
                    {
                        bValid = true;
                    }
                }
                if (GET_PLAYER(getOwnerINLINE()).isAnarchy())
                {
                    bValid = true;
                }
            }
            if (bValid == true)
            {
                if (m_pUnitInfo->getStateReligion() != NO_RELIGION)
                {
                    bValid = false;
                    if (GET_PLAYER(getOwnerINLINE()).getStateReligion() == m_pUnitInfo->getStateReligion())
                    {
                        bValid = true;
                    }
                }
            }
            if (bValid == false)
            {
                gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), gDLL->getText("TXT_KEY_MESSAGE_UNIT_ABANDON", getNameKey()), GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, m_pUnitInfo->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), plot()->getX_INLINE(), plot()->getY_INLINE());
                kill(true);
                GC.getGameINLINE().decrementUnitCreatedCount(getUnitType());
                GC.getGameINLINE().decrementUnitClassCreatedCount((UnitClassTypes)(m_pUnitInfo->getUnitClassType()));
		/*
		if(isOOSLogging())
		{
			oosLog("UnitClassCount"
				,"Turn:%d,Player:%d,X:%d,Y:%d,ClassType:%S,Count:%d"
				,GC.getGameINLINE().getGameTurn()
				,getOwner()
				,getX()
				,getY()
				,GC.getUnitClassInfo((UnitClassTypes)(m_pUnitInfo->getUnitClassType())).getDescription()
				,GET_PLAYER(getOwnerINLINE()).getUnitClassCount(((UnitClassTypes)(m_pUnitInfo->getUnitClassType())))
			);
		}*/
                //GET_TEAM(getTeam()).changeUnitClassCount(((UnitClassTypes)(m_pUnitInfo->getUnitClassType())), -1);				//SpyFanatic: already called during kill
                //GET_PLAYER(getOwnerINLINE()).changeUnitClassCount(((UnitClassTypes)(m_pUnitInfo->getUnitClassType())), -1);		//SpyFanatic: already called during kill
                //GET_PLAYER(getOwnerINLINE()).changeExtraUnitCost(m_pUnitInfo->getExtraCost() * -1);								//SpyFanatic: already called during kill
				return;
            }
        }
    }

	//TODO: add new booleans to CvUnitinfo to increase performance
	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (isHasPromotion((PromotionTypes)iI))
		{
		    if (GC.getPromotionInfo((PromotionTypes)iI).getFreeXPPerTurn() != 0)
		    {
                if (getExperience() < GC.getDefineINT("FREE_XP_MAX"))
                {
                    changeExperience(GC.getPromotionInfo((PromotionTypes)iI).getFreeXPPerTurn(), -1, false, false, false);
                }
		    }
		    if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionRandomApply() != NO_PROMOTION)
		    {
                if (!isHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionRandomApply()))
                {
					int iPromotionChance=std::max(3,GC.getPromotionInfo((PromotionTypes)iI).getPromotionRandomApplyChance());

                    if (GC.getGameINLINE().getSorenRandNum(100, "Promotion Random Apply") <= iPromotionChance)
                    {
                        setHasPromotion(((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionRandomApply()), true);
                    }
                }
		    }
		    if (GC.getPromotionInfo((PromotionTypes)iI).getBetrayalChance() != 0)
		    {
                if (!isImmuneToCapture() && !isBarbarian() && !GC.getGameINLINE().isOption(GAMEOPTION_NO_BARBARIANS))
                {
                    if (GC.getGameINLINE().getSorenRandNum(100, "Betrayal Chance") <= GC.getPromotionInfo((PromotionTypes)iI).getBetrayalChance())
                    {
                        betray(BARBARIAN_PLAYER);
						return;
                    }
                }
		    }
            if (!CvString(GC.getPromotionInfo((PromotionTypes)iI).getPyPerTurn()).empty())
            {
                CyUnit* pyUnit = new CyUnit(this);
                CyArgsList argsList;
                argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                argsList.add(iI);//the promotion #
                gDLL->getPythonIFace()->callFunction(PYSpellModule, "effect", argsList.makeFunctionArgs()); //, &lResult
                delete pyUnit; // python fxn must not hold on to this pointer
            }
		    if (GC.getPromotionInfo((PromotionTypes)iI).getExpireChance() != 0)
		    {
                if (GC.getGameINLINE().getSorenRandNum(100, "Promotion Expire") <= GC.getPromotionInfo((PromotionTypes)iI).getExpireChance())
                {
                    setHasPromotion(((PromotionTypes)iI), false);
					if(isDelayedDeath())
					{
						return;
					}
                }
		    }
            if (!isHurt())
            {
                if (GC.getPromotionInfo((PromotionTypes)iI).isRemovedWhenHealed())
                {
                    setHasPromotion(((PromotionTypes)iI), false);
					if(isDelayedDeath())
					{
						return;
					}
                }
		    }
		}
	}

	setHasCasted(false);

    if (getSpellCasterXP() > 0)
    {
		int iChance=GC.getGameINLINE().getSorenRandNum(100, "SpellCasterXP");

        iChance *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
		iChance /=100;

        if (iChance < getSpellCasterXP() - getExperience())
        {
            changeExperience(1, -1, false, false, false);
        }
    }

	if (getDuration() > 0)
	{
	    changeDuration(-1);
	    if (getDuration() == 0)
	    {
	        if (isImmortal())
	        {
	            changeImmortal(-1);
	        }
            kill(true);
			return; 
	    }
	}

    if (pPlot->isCity())
    {
        for (iI = 0; iI < GC.getNumBuildingInfos(); iI++)
        {
            if (pPlot->getPlotCity()->getNumBuilding((BuildingTypes)iI) > 0)
            {
                if (GC.getBuildingInfo((BuildingTypes)iI).getRemovePromotion() != NO_PROMOTION)
                {
                    if (isHasPromotion((PromotionTypes)GC.getBuildingInfo((BuildingTypes)iI).getRemovePromotion()))
                    {
                        setHasPromotion((PromotionTypes)GC.getBuildingInfo((BuildingTypes)iI).getRemovePromotion(), false);
                    }
                }
                if (GC.getBuildingInfo((BuildingTypes)iI).isApplyFreePromotionOnMove())
                {
                    if (GC.getBuildingInfo((BuildingTypes)iI).getFreePromotion() != NO_PROMOTION)
                    {
                        if ((getUnitCombatType() != NO_UNITCOMBAT) && GC.getPromotionInfo((PromotionTypes)GC.getBuildingInfo((BuildingTypes)iI).getFreePromotion()).getUnitCombat(getUnitCombatType()))
                        {
                            setHasPromotion((PromotionTypes)GC.getBuildingInfo((BuildingTypes)iI).getFreePromotion(), true);
                        }
                    }
                }
            }
        }

		if (isBarbarian())
        {
            if (m_pUnitInfo->isAutoRaze())
            {
                if (pPlot->getOwner() == getOwnerINLINE())
                {
                    pPlot->getPlotCity()->kill(true);
                }
            }
        }
    }

    if (m_pUnitInfo->isImmortal())
    {
        if (!isImmortal())
        {
            changeImmortal(1);
        }
    }
//FfH: End Add

	if((getLevel()==1) && (getExperience() >= experienceNeeded()))
	{
        setPromotionReady(true);
		promote((PromotionTypes)GC.getInfoTypeForString("PROMOTION_COMBAT2"),-1);
	}

	testPromotionReady();

	if((getLevel()==2) && (getExperience() >= experienceNeeded()))
	{
        setPromotionReady(true);
		promote((PromotionTypes)GC.getInfoTypeForString("PROMOTION_COMBAT3"),-1);
	}
	testPromotionReady();

	if (isBlockading())
	{
		collectBlockadeGold();
	}

//FfH: Modified by Kael 02/03/2009 (spy intercept and feature damage commented out for performance, healing moved to earlier in the function)
//	if (isSpy() && isIntruding() && !isCargo())
//	{
//		TeamTypes eTeam = plot()->getTeam();
//		if (NO_TEAM != eTeam)
//		{
//			if (GET_TEAM(getTeam()).isOpenBorders(eTeam))
//			{
//				testSpyIntercepted(plot()->getOwnerINLINE(), GC.getDefineINT("ESPIONAGE_SPY_NO_INTRUDE_INTERCEPT_MOD"));
//			}
//			else
//			{
//				testSpyIntercepted(plot()->getOwnerINLINE(), GC.getDefineINT("ESPIONAGE_SPY_INTERCEPT_MOD"));
//			}
//		}
//	}
//	if (baseCombatStr() > 0)
//	{
//		FeatureTypes eFeature = plot()->getFeatureType();
//		if (NO_FEATURE != eFeature)
//		{
//			if (0 != GC.getFeatureInfo(eFeature).getTurnDamage())
//			{
//				changeDamage(GC.getFeatureInfo(eFeature).getTurnDamage(), NO_PLAYER);
//			}
//		}
//	}
//	if (hasMoved())
//	{
//		if (isAlwaysHeal())
//		{
//			doHeal();
//		}
//	}
//	else
//	{
//		if (isHurt())
//		{
//			doHeal();
//		}
//		if (!isCargo())
//		{
//			changeFortifyTurns(1);
//		}
//	}
//FfH:End Modify

	changeImmobileTimer(-1);

	setMadeAttack(false);
	setMadeInterception(false);

	setReconPlot(NULL);
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("Unit::doTurn::zero");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("Unit::doTurn::setMoves");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

	setMoves(0);
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("Unit::doTurn::setMoves");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("Unit::doTurn::zero");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

//>>>>Refined Defensive Strikes: Added by Denev 2009/11/10
	setCombatDefensiveStrikes(-1);
//<<<<Refined Defensive Strikes: End Add
	setDefensiveStrikesLeft(2);


	if(isGarrision() && (!plot()->isCity() || plot()->getOwnerINLINE()!=getOwnerINLINE()))
	{
		//SpyFanatic: do not ungarrison if they are in fort and gameoption is enabled
		if(!GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) || plot()->getImprovementType() == NO_IMPROVEMENT || GC.getImprovementInfo(plot()->getImprovementType()).getCulture() <= 0)
		{
			setGarrision(false);
			setAIGroup(NULL);
		}
	}

	//added Sephi
	//Better AI. Units with a lot of Experience are more valuable
	if(isHasSecondChance())
	{
		AI_setUnitAIType(UNITAI_HERO);
	}

/*************************************************************************************************/
/**	ADDON (Combatauras) Sephi																	**/
/*************************************************************************************************/

	if(getBlessing1() != NULL || getBlessing2() != NULL 
		|| getCurse1() != NULL || getCurse2() != NULL) {
		CvCombatAura* pAura=NULL;
		for(int i=0;i<4;i++)
		{
			pAura=NULL;
			if(i==0)
			{
				pAura=getBlessing1();
			}
			else if(i==1)
			{
				pAura=getBlessing2();
			}
			else if(i==2)
			{
				pAura=getCurse1();
			}
			else if(i==3)
			{
				pAura=getCurse2();
			}

			if(pAura!=NULL)
			{
				if(pAura->isEnchantedWeapons())
				{
					pAura->updateEnchantedWeapons(this);
				}

				if(pAura->getDamage()>0)
				{
					doDamage(pAura->getDamage(), pAura->getDamage()/2, pAura->getUnit(), GC.getCombatAuraInfo(pAura->getCombatAuraType()).getDamageType(), false);
				}

				if(pAura->getHealing()>0)
				{
					changeDamage(-pAura->getHealing());
				}

				//Apply && Remove Promotions

				CvUnit* pOwner=pAura->getUnit();
/**
				for(int iJ=0;iJ<GC.getNumPromotionInfos();iJ++)
				{
					if(!isHasPromotion((PromotionTypes)iJ))
					{
						if(pAura->isPromotionApply((PromotionTypes)iJ))
						{
							if(GC.getPromotionInfo((PromotionTypes)iJ).isAnyUnitCombat() || getUnitCombatType()!=NO_UNITCOMBAT && GC.getPromotionInfo((PromotionTypes)iJ).getUnitCombat(getUnitCombatType()))
							{
								setHasPromotion((PromotionTypes)iJ,true);
							}
						}
					}
				}
**/
				pAura->ApplyPromotionsToUnit(this);

				for(int iJ=0;iJ<GC.getNumPromotionInfos();iJ++)
				{
					if(isHasPromotion((PromotionTypes)iJ))
					{
						if(pAura->isPromotionRemove((PromotionTypes)iJ))
						{
							setHasPromotion((PromotionTypes)iJ,false);
						}
					}
				}
			}
		}
	}

	PromotionTypes eDoT=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_SPELL_ON_FIRE");
	if(eDoT!=NO_PROMOTION && isHasPromotion(eDoT))
	{
		doDamage(10,100,NULL,GC.getInfoTypeForString("DAMAGE_FIRE"),false);
	}

	if(!isBarbarian() && getEquipmentYieldType()==NO_YIELD)
	{
		setEquipmentYieldType(GC.getCivilizationInfo(getCivilizationType()).getFavoredYield());
	}

	//Affinity for Summons
	updateBonusSummonStrength();
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	BETTER AI (Choose Groupflag) Sephi                             					            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    if (!GET_PLAYER(getOwnerINLINE()).isHuman())
    {
		/** Disable for now, mostly not needed anymore
        if (!isBarbarian())
        {
            if (AI_getGroupflag()==GROUPFLAG_NONE)
            {
                AI_chooseGroupflag();
			}
        }
		**/
    }

    if(AI_getUnitAIType()==UNITAI_WORKER)
    {
        if(getOwnerINLINE()!=NO_PLAYER && GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(plot(), 3)>0)
        {
			if(!plot()->isCity())
			{
				if (getGroup())
				{
					getGroup()->setActivityType(ACTIVITY_AWAKE);
				}
			}
        }
    }

/** AI Memory decay for being Attacked   **/
    if(AI_getWasAttacked()>0)
    {
        AI_setWasAttacked(AI_getWasAttacked()-1);
    }

	if(getExtraLives() < m_pUnitInfo->getExtraLives())
	{
		changeExtraLives(-getExtraLives() + m_pUnitInfo->getExtraLives());
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

// Better Unit Power (Added by Skyre)
	// TODO: Is calculating this every turn efficient enough?
	updatePower();
// End Better Unit Power
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("Unit::doTurn::zero");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("Unit::doTurn");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/
}


void CvUnit::updateAirStrike(CvPlot* pPlot, bool bQuick, bool bFinish)
{
	bool bVisible = false;

	if (!bFinish)
	{
		if (isFighting())
		{
			return;
		}

		if (!bQuick)
		{
			bVisible = isCombatVisible(NULL);
		}

		if (!airStrike(pPlot))
		{
			return;
		}

		if (bVisible)
		{
			CvAirMissionDefinition kAirMission;
			kAirMission.setMissionType(MISSION_AIRSTRIKE);
			kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
			kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
			kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
			kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
			kAirMission.setPlot(pPlot);
			setCombatTimer(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime());
			GC.getGameINLINE().incrementTurnTimer(getCombatTimer());
			kAirMission.setMissionTime(getCombatTimer() * gDLL->getSecsPerTurn());

			if (pPlot->isActiveVisible(false))
			{
				gDLL->getEntityIFace()->AddMission(&kAirMission);
			}

			return;
		}
	}

	CvUnit *pDefender = getCombatUnit();
	if (pDefender != NULL)
	{
		pDefender->setCombatUnit(NULL);
	}
	setCombatUnit(NULL);
	setAttackPlot(NULL, false);

	getGroup()->clearMissionQueue();

	if (isSuicide() && !isDead())
	{
		kill(true);
	}
}

void CvUnit::resolveAirCombat(CvUnit* pInterceptor, CvPlot* pPlot, CvAirMissionDefinition& kBattle)
{
	CvWString szBuffer;

	int iTheirStrength = (DOMAIN_AIR == pInterceptor->getDomainType() ? pInterceptor->airCurrCombatStr(this) : pInterceptor->currCombatStr(NULL, NULL));
	int iOurStrength = (DOMAIN_AIR == getDomainType() ? airCurrCombatStr(pInterceptor) : currCombatStr(NULL, NULL));
	int iTotalStrength = iOurStrength + iTheirStrength;
	if (0 == iTotalStrength)
	{
		FAssert(false);
		return;
	}

	int iOurOdds = (100 * iOurStrength) / std::max(1, iTotalStrength);

	int iOurRoundDamage = (pInterceptor->currInterceptionProbability() * GC.getDefineINT("MAX_INTERCEPTION_DAMAGE")) / 100;
	int iTheirRoundDamage = (currInterceptionProbability() * GC.getDefineINT("MAX_INTERCEPTION_DAMAGE")) / 100;
	if (getDomainType() == DOMAIN_AIR)
	{
		iTheirRoundDamage = std::max(GC.getDefineINT("MIN_INTERCEPTION_DAMAGE"), iTheirRoundDamage);
	}

	int iTheirDamage = 0;
	int iOurDamage = 0;

	for (int iRound = 0; iRound < GC.getDefineINT("INTERCEPTION_MAX_ROUNDS"); ++iRound)
	{
		if (GC.getGameINLINE().getSorenRandNum(100, "Air combat") < iOurOdds)
		{
			if (DOMAIN_AIR == pInterceptor->getDomainType())
			{
				iTheirDamage += iTheirRoundDamage;
				pInterceptor->changeDamage(iTheirRoundDamage, getOwnerINLINE());
				if (pInterceptor->isDead())
				{
					break;
				}
			}
		}
		else
		{
			iOurDamage += iOurRoundDamage;
			changeDamage(iOurRoundDamage, pInterceptor->getOwnerINLINE());
			if (isDead())
			{
				break;
			}
		}
	}

	if (isDead())
	{
		if (iTheirRoundDamage > 0)
		{
			int iExperience = attackXPValue();
			iExperience = (iExperience * iOurStrength) / std::max(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), iTheirStrength);
			iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
			pInterceptor->changeExperience(iExperience, maxXPValue(), true, pPlot->getOwnerINLINE() == pInterceptor->getOwnerINLINE(), !isBarbarian());
		}
	}
	else if (pInterceptor->isDead())
	{
		int iExperience = pInterceptor->defenseXPValue();
		iExperience = (iExperience * iTheirStrength) / std::max(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), iOurStrength);
		iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
		changeExperience(iExperience, pInterceptor->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pInterceptor->isBarbarian());
	}
	else if (iOurDamage > 0)
	{
		if (iTheirRoundDamage > 0)
		{
			pInterceptor->changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), maxXPValue(), true, pPlot->getOwnerINLINE() == pInterceptor->getOwnerINLINE(), !isBarbarian());
		}
	}
	else if (iTheirDamage > 0)
	{
		changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pInterceptor->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pInterceptor->isBarbarian());
	}

	kBattle.setDamage(BATTLE_UNIT_ATTACKER, iOurDamage);
	kBattle.setDamage(BATTLE_UNIT_DEFENDER, iTheirDamage);
}


void CvUnit::updateAirCombat(bool bQuick)
{
	CvUnit* pInterceptor = NULL;
	bool bFinish = false;

	FAssert(getDomainType() == DOMAIN_AIR || getDropRange() > 0);

	if (getCombatTimer() > 0)
	{
		changeCombatTimer(-1);

		if (getCombatTimer() > 0)
		{
			return;
		}
		else
		{
			bFinish = true;
		}
	}

	CvPlot* pPlot = getAttackPlot();
	if (pPlot == NULL)
	{
		return;
	}

	if (bFinish)
	{
		pInterceptor = getCombatUnit();
	}
	else
	{
		pInterceptor = bestInterceptor(pPlot);
	}


	if (pInterceptor == NULL)
	{
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);

		getGroup()->clearMissionQueue();

		return;
	}

	//check if quick combat
	bool bVisible = false;
	if (!bQuick)
	{
		bVisible = isCombatVisible(pInterceptor);
	}

	//if not finished and not fighting yet, set up combat damage and mission
	if (!bFinish)
	{
		if (!isFighting())
		{
			if (plot()->isFighting() || pPlot->isFighting())
			{
				return;
			}

			setMadeAttack(true);

			setCombatUnit(pInterceptor, true);
			pInterceptor->setCombatUnit(this, false);
		}

		FAssertMsg(pInterceptor != NULL, "Defender is not assigned a valid value");

		FAssertMsg(plot()->isFighting(), "Current unit instance plot is not fighting as expected");
		FAssertMsg(pInterceptor->plot()->isFighting(), "pPlot is not fighting as expected");

		CvAirMissionDefinition kAirMission;
		if (DOMAIN_AIR != getDomainType())
		{
			kAirMission.setMissionType(MISSION_PARADROP);
		}
		else
		{
			kAirMission.setMissionType(MISSION_AIRSTRIKE);
		}
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, pInterceptor);

		resolveAirCombat(pInterceptor, pPlot, kAirMission);

		if (!bVisible)
		{
			bFinish = true;
		}
		else
		{
			kAirMission.setPlot(pPlot);
			kAirMission.setMissionTime(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime() * gDLL->getSecsPerTurn());
			setCombatTimer(GC.getMissionInfo(MISSION_AIRSTRIKE).getTime());
			GC.getGameINLINE().incrementTurnTimer(getCombatTimer());

			if (pPlot->isActiveVisible(false))
			{
				gDLL->getEntityIFace()->AddMission(&kAirMission);
			}
		}

		changeMoves(GC.getMOVE_DENOMINATOR());
		if (DOMAIN_AIR != pInterceptor->getDomainType())
		{
			pInterceptor->setMadeInterception(true);
		}

		if (isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SHOT_DOWN_ENEMY", pInterceptor->getNameKey(), getNameKey(), getVisualCivAdjective(pInterceptor->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_SHOT_DOWN", getNameKey(), pInterceptor->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		else if (kAirMission.getDamage(BATTLE_UNIT_ATTACKER) > 0)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_HURT_ENEMY_AIR", pInterceptor->getNameKey(), getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_ATTACKER)), getVisualCivAdjective(pInterceptor->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_AIR_UNIT_HURT", getNameKey(), pInterceptor->getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_ATTACKER)));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}

		if (pInterceptor->isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SHOT_DOWN_ENEMY", getNameKey(), pInterceptor->getNameKey(), pInterceptor->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_SHOT_DOWN", pInterceptor->getNameKey(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
		else if (kAirMission.getDamage(BATTLE_UNIT_DEFENDER) > 0)
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DAMAGED_ENEMY_AIR", getNameKey(), pInterceptor->getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_DEFENDER)), pInterceptor->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_AIR_UNIT_DAMAGED", pInterceptor->getNameKey(), getNameKey(), -(kAirMission.getDamage(BATTLE_UNIT_DEFENDER)));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}

		if (0 == kAirMission.getDamage(BATTLE_UNIT_ATTACKER) + kAirMission.getDamage(BATTLE_UNIT_DEFENDER))
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ABORTED_ENEMY_AIR", pInterceptor->getNameKey(), getNameKey(), getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pInterceptor->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPT", MESSAGE_TYPE_INFO, pInterceptor->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_AIR_UNIT_ABORTED", getNameKey(), pInterceptor->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_INTERCEPTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
	}

	if (bFinish)
	{
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);
		pInterceptor->setCombatUnit(NULL);

		if (!isDead() && isSuicide())
		{
			kill(true);
		}
	}
}

void CvUnit::resolveCombat(CvUnit* pDefender, CvPlot* pPlot, CvBattleDefinition& kBattle)
{
	CombatDetails cdAttackerDetails;
	CombatDetails cdDefenderDetails;

//FfH: Modified by Kael 01/14/2009
//	int iAttackerStrength = currCombatStr(NULL, NULL, &cdAttackerDetails);
//	int iAttackerFirepower = currFirepower(NULL, NULL);
	int iAttackerStrength = currCombatStr(NULL, pDefender, &cdAttackerDetails);
	int iAttackerFirepower = currFirepower(NULL, pDefender);
//FfH: End Modify

	int iDefenderStrength;
	int iAttackerDamage;
	int iDefenderDamage;
	int iDefenderOdds;
/*************************************************************************************************/
/**	ADDON (Promotions in Combat) killmeplease / merged Sephi                   					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	bool bDefenderWithdrawn = false;
	bool bAttackerWithdrawn = false;
	bool bAttackerHasLostNoHP = true;
	int iAttackerInitialDamage = getDamage();
	int iDefenderInitialDamage = pDefender->getDamage();
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	bool bAttackerMustWin = (getCombatOdds(this, pDefender)==1000);
	bool bDefenderMustWin = (getCombatOdds(this, pDefender)==0);

	getDefenderCombatValues(*pDefender, pPlot, iAttackerStrength, iAttackerFirepower, iDefenderOdds, iDefenderStrength, iAttackerDamage, iDefenderDamage, &cdDefenderDetails);
	int iAttackerKillOdds = iDefenderOdds * (100 - withdrawalProbability()) / 100;

//FfH: Modified by Kael 08/02/2008
//	if (isHuman() || pDefender->isHuman())
//	{
//		//Added ST
//		CyArgsList pyArgsCD;
//		pyArgsCD.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
//		pyArgsCD.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
//		pyArgsCD.add(getCombatOdds(this, pDefender));
//		gDLL->getEventReporterIFace()->genericEvent("combatLogCalc", pyArgsCD.makeFunctionArgs());
//	}
    if(GC.getUSE_COMBAT_RESULT_CALLBACK())
    {
        if (isHuman() || pDefender->isHuman())
        {
            CyArgsList pyArgsCD;
            pyArgsCD.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
            pyArgsCD.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
            pyArgsCD.add(getCombatOdds(this, pDefender));
			CvEventReporter::getInstance().genericEvent("combatLogCalc", pyArgsCD.makeFunctionArgs());
        }
	}
//FfH: End Modify
/*************************************************************************************************/
/**	ADDON (Promotions in Combat) killmeplease / merged Sephi                   					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	//@MOD OP: calculate odds (copied from CvUnit::AI_attackOdds)
	int iWinningOdds = getCombatOdds(this, pDefender);
	int iCombatRounds = 0;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	collateralCombat(pPlot, pDefender);

	while (true)
	{
		++iCombatRounds;
		bool bDefenderWonRound=(GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("COMBAT_DIE_SIDES"), "Combat") < iDefenderOdds); 

		if(bDefenderWonRound && bAttackerMustWin)
		{
			if(getDamage()+ iAttackerDamage>= maxHitPoints())
			{
				//next round would kill attacker
				bDefenderWonRound = false;
			}
		}

		if(!bDefenderWonRound && bDefenderMustWin)
		{
			if(pDefender->getDamage() + iDefenderDamage >= maxHitPoints())
			{
				//next round would kill Defender
				bDefenderWonRound = true;
			}
		}

		if (bDefenderWonRound)
		{
			if (getCombatFirstStrikes() == 0)
			{
				if (getDamage() + iAttackerDamage >= maxHitPoints() && GC.getGameINLINE().getSorenRandNum(100, "Withdrawal") < withdrawalProbability())
				{
/*************************************************************************************************/
/**	ADDON (Promotions in Combat) killmeplease / merged Sephi                   					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
					bAttackerWithdrawn = true;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

					flankingStrikeCombat(pPlot, iAttackerStrength, iAttackerFirepower, iAttackerKillOdds, iDefenderDamage, pDefender);

					changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pDefender->isBarbarian());

//FfH Promotions: Added by Kael 08/12/2007
                    setFleeWithdrawl(true);
//FfH: End Add

					break;
				}

/*************************************************************************************************/
/**	ADDON (Guardian Angel tag) Sephi						                   					**/
/*************************************************************************************************/
				int iGuardianProbability=((getBlessing1() == NULL) ? 0 : (getBlessing1()->getGuardianAngel()*(100+getBlessing1()->getPower())/100)) + ((getBlessing2() == NULL) ? 0 : (getBlessing2()->getGuardianAngel()*(100+getBlessing2()->getPower())/100));
				if (getDamage() + iAttackerDamage >= maxHitPoints() && GC.getGameINLINE().getSorenRandNum(100, "Guardian Angel") < iGuardianProbability)
				{
					flankingStrikeCombat(pPlot, iAttackerStrength, iAttackerFirepower, iAttackerKillOdds, iDefenderDamage, pDefender);
					changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pDefender->isBarbarian());
                    setFleeWithdrawl(true);
					break;
				}

				PromotionTypes eSecondChance=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_SECOND_CHANCE");
				if(eSecondChance!=NO_PROMOTION && isHasPromotion(eSecondChance))
				{
					if (getDamage() + iAttackerDamage >= maxHitPoints())
					{
						changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pDefender->isBarbarian());
						setHasPromotion(eSecondChance,false);
						setDamage(iAttackerInitialDamage);
						finishMoves();
						setFleeWithdrawl(true);
						break;
					}
				}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

				changeDamage(iAttackerDamage, pDefender->getOwnerINLINE());
/*************************************************************************************************/
/**	ADDON (Promotions in Combat) killmeplease / merged Sephi                   					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
				bAttackerHasLostNoHP = false;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

				if (pDefender->getCombatFirstStrikes() > 0 && pDefender->isRanged())
				{
					kBattle.addFirstStrikes(BATTLE_UNIT_DEFENDER, 1);
					kBattle.addDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, iAttackerDamage);
				}

				cdAttackerDetails.iCurrHitPoints = currHitPoints();

//FfH: Modified by Kael 08/02/2008
//				if (isHuman() || pDefender->isHuman())
//				{
//					CyArgsList pyArgs;
//					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
//					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
//					pyArgs.add(1);
//					pyArgs.add(iAttackerDamage);
//					gDLL->getEventReporterIFace()->genericEvent("combatLogHit", pyArgs.makeFunctionArgs());
//				}
                if(GC.getUSE_COMBAT_RESULT_CALLBACK())
                {
                    if (isHuman() || pDefender->isHuman())
                    {
                        CyArgsList pyArgs;
                        pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
                        pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
                        pyArgs.add(1);
                        pyArgs.add(iAttackerDamage);
						CvEventReporter::getInstance().genericEvent("combatLogHit", pyArgs.makeFunctionArgs());
                    }
				}
//FfH: End Modify

			}
		}
		else
		{
			if (pDefender->getCombatFirstStrikes() == 0)
			{
                if (pDefender->getDamage() + iDefenderDamage >= pDefender->maxHitPoints())
                {
/*************************************************************************************************/
/** GAMEPLAY CHANGE (less annoying Withdrawal) Sephi                                            **/
/**						                                            							**/
/**	allow withdrawal only when enemy unit has movement left            							**/
/*************************************************************************************************/
/** orig
                    if (!pPlot->isCity())
                    {
                        if (GC.getGameINLINE().getSorenRandNum(100, "Withdrawal") < pDefender->getWithdrawlProbDefensive())
                        {
                            pDefender->setFleeWithdrawl(true);
                            break;
                        }
                    }
**/
                    if (!pPlot->isCity())
                    {
                        UnitClassTypes eUnitClass = ((UnitClassTypes)(GC.getUnitInfo(pDefender->getUnitType()).getUnitClassType()));
                        if(pDefender->getMoves()<maxMoves() || ::isWorldUnitClass(eUnitClass))
                        {
                            if (GC.getGameINLINE().getSorenRandNum(100, "Withdrawal") < pDefender->getWithdrawlProbDefensive())
                            {
                                pDefender->setFleeWithdrawl(true);

                                if(!::isWorldUnitClass(eUnitClass))
                                {
                                    pDefender->changeMoves((1+pDefender->maxMoves()-pDefender->getMoves())/2);
                                }
                                break;
                            }
                        }
                    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
                }
//FfH: End Add

/*************************************************************************************************/
/**	ADDON (Guardian Angel tag) Sephi						                   					**/
/*************************************************************************************************/
				int iGuardianProbability=((pDefender->getBlessing1() == NULL) ? 0 : (pDefender->getBlessing1()->getGuardianAngel()*(100+pDefender->getBlessing1()->getPower())/100)) + ((pDefender->getBlessing2() == NULL) ? 0 : (pDefender->getBlessing2()->getGuardianAngel()*(100+pDefender->getBlessing2()->getPower())/100));
                if (pDefender->getDamage() + iDefenderDamage >= pDefender->maxHitPoints() && GC.getGameINLINE().getSorenRandNum(100, "Guardian Angel") < iGuardianProbability)
				{
                    pDefender->setFleeWithdrawl(true);
					break;
				}

				PromotionTypes eSecondChance=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_SECOND_CHANCE");
				if(eSecondChance!=NO_PROMOTION && pDefender->isHasPromotion(eSecondChance))
				{
					if (pDefender->getDamage() + iDefenderDamage >= pDefender->maxHitPoints())
					{
						pDefender->setHasPromotion(eSecondChance,false);
						pDefender->setDamage(iDefenderInitialDamage/2);
						pDefender->setFleeWithdrawl(true);
						break;
					}
				}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

				if (std::min(GC.getMAX_HIT_POINTS(), pDefender->getDamage() + iDefenderDamage) > combatLimit())
				{
					changeExperience(GC.getDefineINT("EXPERIENCE_FROM_WITHDRAWL"), pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pDefender->isBarbarian());
					pDefender->setDamage(combatLimit(), getOwnerINLINE());

//FfH: Added by Kael 05/27/2008
                    setMadeAttack(true);
                    changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
//FfH: End Add

					break;
				}

				pDefender->changeDamage(iDefenderDamage, getOwnerINLINE());

				if (getCombatFirstStrikes() > 0 && isRanged())
				{
					kBattle.addFirstStrikes(BATTLE_UNIT_ATTACKER, 1);
					kBattle.addDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, iDefenderDamage);
				}

				cdDefenderDetails.iCurrHitPoints=pDefender->currHitPoints();

//FfH: Modified by Kael 08/02/2008
//				if (isHuman() || pDefender->isHuman())
//				{
//					CyArgsList pyArgs;
//					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
//					pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
//					pyArgs.add(0);
//					pyArgs.add(iDefenderDamage);
//					CvEventReporter::getInstance().genericEvent("combatLogHit", pyArgs.makeFunctionArgs());
//				}
                if(GC.getUSE_COMBAT_RESULT_CALLBACK())
                {
                    if (isHuman() || pDefender->isHuman())
                    {
                        CyArgsList pyArgs;
                        pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdAttackerDetails));
                        pyArgs.add(gDLL->getPythonIFace()->makePythonObject(&cdDefenderDetails));
                        pyArgs.add(0);
                        pyArgs.add(iDefenderDamage);
                        CvEventReporter::getInstance().genericEvent("combatLogHit", pyArgs.makeFunctionArgs());
                    }
				}
//FfH: End Modify

			}
		}

		if (getCombatFirstStrikes() > 0)
		{
			changeCombatFirstStrikes(-1);
		}

		if (pDefender->getCombatFirstStrikes() > 0)
		{
			pDefender->changeCombatFirstStrikes(-1);
		}

		if (isDead() || pDefender->isDead())
		{
			if (isDead())
			{
				int iExperience = defenseXPValue();
				iExperience = (iDefenderStrength == 0) ? 1 : ((iExperience * iAttackerStrength) / iDefenderStrength);
				iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
				pDefender->changeExperience(iExperience, maxXPValue(), true, pPlot->getOwnerINLINE() == pDefender->getOwnerINLINE(), !isBarbarian());
			}
			else
			{
				flankingStrikeCombat(pPlot, iAttackerStrength, iAttackerFirepower, iAttackerKillOdds, iDefenderDamage, pDefender);

				int iExperience = pDefender->attackXPValue();
				iExperience = (iAttackerStrength == 0) ? 1 : ((iExperience * iDefenderStrength) / iAttackerStrength);
				iExperience = range(iExperience, GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), GC.getDefineINT("MAX_EXPERIENCE_PER_COMBAT"));
				changeExperience(iExperience, pDefender->maxXPValue(), true, pPlot->getOwnerINLINE() == getOwnerINLINE(), !pDefender->isBarbarian());
			}

			break;
		}
	}


// Blitz will use and extra move if you have more than 2 movement.
/*	if (isBlitz())
	{
		if (maxMoves() <= 2 * GC.getMOVE_DENOMINATOR())
		{}
		else if (getMoves() >= maxMoves() - GC.getMOVE_DENOMINATOR())
			finishMoves();
		else
			setMoves(getMoves() + GC.getMOVE_DENOMINATOR());
	}*/

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bKillHeroNetherblade                                                                    **/
/*************************************************************************************************/
    PromotionTypes iHero = (PromotionTypes)GC.getInfoTypeForString("PROMOTION_HERO");
    PromotionTypes iNetherBlade = (PromotionTypes)GC.getInfoTypeForString("PROMOTION_NETHER_BLADE");

    for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
    {
        if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
        {
            CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
            for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
            {
                if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
                {
                    if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isKillHeroNetherblade())
                    {
                        if(iHero!=NO_PROMOTION && iNetherBlade!=NO_PROMOTION)
                        {
                            if(pDefender->isDead() && pDefender->isHasPromotion(iHero))
                            {
                                if (isHasPromotion(iNetherBlade))
                                {
                                    GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
                                }
                            }
                        }
                    }

                    if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isWorldUnitDefeated())
                    {
                        if(pDefender->isDead() && ::isWorldUnitClass((UnitClassTypes)GC.getUnitInfo(pDefender->getUnitType()).getUnitClassType()))
                        {
                            GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
                        }
                    }
                }
            }
        }
    }

    for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
    {
        if(GET_PLAYER(pDefender->getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(pDefender->getOwnerINLINE()).isAdventureFinished(iAdvLoop))
        {
            CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
            for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
            {
                if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
                {
                    if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isKillHeroNetherblade())
                    {
                        if(iHero!=NO_PROMOTION && iNetherBlade!=NO_PROMOTION)
                        {
                            if(isDead() && isHasPromotion(iHero))
                            {
                                if (pDefender->isHasPromotion(iNetherBlade))
                                {
                                    GET_PLAYER(pDefender->getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
                                }
                            }
                        }
                    }

                    if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isWorldUnitDefeated())
                    {
                        if(isDead() && ::isWorldUnitClass((UnitClassTypes)GC.getUnitInfo(getUnitType()).getUnitClassType()))
                        {
                            GET_PLAYER(pDefender->getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
                        }
                    }
                }
            }
        }
    }

/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/



/*************************************************************************************************/
/**	ADDON (Promotions in Combat) killmeplease / merged Sephi                   					**/
/*************************************************************************************************/
	std::vector<PromotionTypes> aAttackerAvailablePromotions;
	std::vector<PromotionTypes> aDefenderAvailablePromotions;
	int iI;
	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)	//loop through promotions
	{
		if (pDefender->isDead() || bDefenderWithdrawn)
		{
			if(!GC.getPromotionInfo((PromotionTypes)iI).isGainedInBattle())
			{
				continue;
			}

			//* defender withdrawn, give him withdrawal promo
			if (bDefenderWithdrawn && GC.getPromotionInfo((PromotionTypes)iI).getWithdrawalChange() > 0)
			{
				if(!pDefender->isHasPromotion((PromotionTypes)iI))
				{
					aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
				}
			}

			if ((isDenyPromotion((PromotionTypes)iI))
				||(isHasPromotion((PromotionTypes)iI)))
			{
				continue;
			}

			if(GC.getPromotionInfo((PromotionTypes)iI).getNumPromotionExcludes()>0)
			{
				continue;
			}
            //added Sephi

			//* attacker was crossing river
			if (GC.getPromotionInfo((PromotionTypes)iI).isRiver() && cdDefenderDetails.iRiverAttackModifier != 0)	//this bonus is being applied to defender
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* attack from water
			else if (GC.getPromotionInfo((PromotionTypes)iI).isAmphib() && cdDefenderDetails.iAmphibAttackModifier != 0)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* attack terrain
			else if (GC.getPromotionInfo((PromotionTypes)iI).getTerrainAttackPercent((int)pPlot->getTerrainType()) > 0)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* attack feature
			else if (pPlot->getFeatureType() != NO_FEATURE &&
				GC.getPromotionInfo((PromotionTypes)iI).getFeatureAttackPercent((int)pPlot->getFeatureType()) > 0)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* attack hills
			else if (GC.getPromotionInfo((PromotionTypes)iI).getHillsAttackPercent() > 0 && pPlot->isHills())
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* attack city
			else if (GC.getPromotionInfo((PromotionTypes)iI).getCityAttackPercent() > 0 && pPlot->isCity(true))	//count forts too
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* first strikes/chances promotions
			/** disabled Sephi, too strong in MoM
			else if ((GC.getPromotionInfo((PromotionTypes)iI).getFirstStrikesChange() > 0 ||
				GC.getPromotionInfo((PromotionTypes)iI).getChanceFirstStrikesChange() > 0) && (firstStrikes() > 0 || chanceFirstStrikes() > 0)	)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			**/
			//* unit combat mod
			else if (pDefender->getUnitCombatType()!=NO_UNITCOMBAT && GC.getPromotionInfo((PromotionTypes)iI).getUnitCombatModifierPercent((int)pDefender->getUnitCombatType()) > 0)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* combat strength promotions
			/** disabled Sephi, too strong in MoM
			else if (GC.getPromotionInfo((PromotionTypes)iI).getCombatPercent() > 0)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			**/
			//* domain mod
			else if (GC.getPromotionInfo((PromotionTypes)iI).getDomainModifierPercent((int)pDefender->getDomainType()))
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* blitz
			else if (GC.getPromotionInfo((PromotionTypes)iI).isBlitz() && bAttackerHasLostNoHP)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* Bonus vs. Full Health
			else if (GC.getPromotionInfo((PromotionTypes)iI).getBonusVSUnitsFullHealth()>0 && iDefenderInitialDamage==0)
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* Bonus vs. Promotion
			else if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatType()!=NO_PROMOTION && pDefender->isHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatType()))
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//Homeland Bonus
			else if(GC.getPromotionInfo((PromotionTypes)iI).getCombatPercentInBorders()!=0 && plot()->getOwnerINLINE()==getOwnerINLINE())
			{
				aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* Bonus vs. Damage Type
			for(int iJ=0;iJ<GC.getNumDamageTypeInfos();iJ++)
			{
				if(GC.getPromotionInfo((PromotionTypes)iI).getDamageTypeResist(iJ)>0)
				{
					if(pDefender->getDamageTypeCombat((DamageTypes)iJ)>0  && getDamageTypeResist((DamageTypes)iJ)<100)
					{
						aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
					}
				}
			}
		}	//if defender is dead or withdrawn
		else	//attacker is dead or withdrawn
		{
			if(!GC.getPromotionInfo((PromotionTypes)iI).isGainedInBattle())
			{
				continue;
			}

			//* attacker withdrawn
			if (bAttackerWithdrawn && GC.getPromotionInfo((PromotionTypes)iI).getWithdrawalChange() > 0)
			{
				if(!isHasPromotion((PromotionTypes)iI))
				{
					aAttackerAvailablePromotions.push_back((PromotionTypes)iI);
				}
			}

			if ((pDefender->isDenyPromotion((PromotionTypes)iI))
				||(pDefender->isHasPromotion((PromotionTypes)iI)))
			{
				continue;
			}

			if(GC.getPromotionInfo((PromotionTypes)iI).getNumPromotionExcludes()>0)
			{
				continue;
			}

			//* defend terrain
			if (GC.getPromotionInfo((PromotionTypes)iI).getTerrainDefensePercent((int)pPlot->getTerrainType()) > 0)
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* defend feature
			else if (pPlot->getFeatureType() != NO_FEATURE &&
				GC.getPromotionInfo((PromotionTypes)iI).getFeatureDefensePercent((int)pPlot->getFeatureType()) > 0)
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* defend hills
			else if (GC.getPromotionInfo((PromotionTypes)iI).getHillsDefensePercent() > 0 && pPlot->isHills())
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* defend city
			else if (GC.getPromotionInfo((PromotionTypes)iI).getCityDefensePercent() > 0 && pPlot->isCity(true))	//count forts too
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* first strikes/chanses promotions
			/** disabled Sephi, too strong in MoM
			else if ((GC.getPromotionInfo((PromotionTypes)iI).getFirstStrikesChange() > 0 ||
				GC.getPromotionInfo((PromotionTypes)iI).getChanceFirstStrikesChange() > 0) &&
				(pDefender->firstStrikes() > 0 || pDefender->chanceFirstStrikes() > 0))
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}**/
			//* unit combat mod vs attacker unit type
			else if (getUnitCombatType()!=NO_UNITCOMBAT && GC.getPromotionInfo((PromotionTypes)iI).getUnitCombatModifierPercent((int)getUnitCombatType()) > 0)
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* combat strength promotions
			//* combat strength promotions
			/** disabled Sephi, too strong in MoM
			else if (GC.getPromotionInfo((PromotionTypes)iI).getCombatPercent() > 0)
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}**/
			//* domain mod
			else if (GC.getPromotionInfo((PromotionTypes)iI).getDomainModifierPercent((int)getDomainType()))
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* Bonus vs. Promotion
			else if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatType()!=NO_PROMOTION && isHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatType()))
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//* Defensive Bonus
			else if (GC.getPromotionInfo((PromotionTypes)iI).isDoubleFortifyBonus() && pDefender->fortifyModifier()>0)
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}
			//Homeland Bonus
			else if(GC.getPromotionInfo((PromotionTypes)iI).getCombatPercentInBorders()!=0 && pDefender->plot()->getOwnerINLINE()==pDefender->getOwnerINLINE())
			{
				aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
			}

			//* Bonus vs. Damage Type
			for(int iJ=0;iJ<GC.getNumDamageTypeInfos();iJ++)
			{
				if(GC.getPromotionInfo((PromotionTypes)iI).getDamageTypeResist(iJ)>0)
				{
					if(getDamageTypeCombat((DamageTypes)iJ)>0 && pDefender->getDamageTypeResist((DamageTypes)iJ)<100)
					{
						aDefenderAvailablePromotions.push_back((PromotionTypes)iI);
					}
				}
			}

		}	//if attacker withdrawn
	}	//end promotion types cycle

	//promote attacker:
	if (!isDead() && aAttackerAvailablePromotions.size() > 0)
	{
		int iHealthPercent = (maxHitPoints() - getDamage()) * 100 / (maxHitPoints() - iAttackerInitialDamage);
		int iPromotionChanseModifier = iHealthPercent * iHealthPercent / maxHitPoints();
		int iPromotionChanse = (GC.getDefineINT("COMBAT_DIE_SIDES") - iWinningOdds) * (100 + iPromotionChanseModifier) / 100;

		iPromotionChanse = std::max(iPromotionChanse,50);

		if (GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("COMBAT_DIE_SIDES"), "Occasional Promotion") < iPromotionChanse)
		{
			//select random promotion from available
			PromotionTypes ptPromotion = aAttackerAvailablePromotions[
				GC.getGameINLINE().getSorenRandNum(aAttackerAvailablePromotions.size(), "Select Promotion Type")];
			//promote
			setHasPromotion(ptPromotion, true);
			//show message
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNIT_PROMOTED_IN_BATTLE", getNameKey(), GC.getPromotionInfo(ptPromotion).getText());
			gDLL->getInterfaceIFace()->addMessage(
				getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer,
				GC.getPromotionInfo((PromotionTypes)0).getSound(), MESSAGE_TYPE_INFO, NULL,
				(ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), this->plot()->getX_INLINE(), this->plot()->getY_INLINE());
		}
	}
	//promote defender:
	if (!pDefender->isDead() && aDefenderAvailablePromotions.size() > 0)
	{
		int iHealthPercent = (maxHitPoints() - pDefender->getDamage()) * 100 / (maxHitPoints() - iDefenderInitialDamage);
		int iPromotionChanseModifier = iHealthPercent * iHealthPercent / maxHitPoints();
		int iPromotionChanse = iWinningOdds * (100 + iPromotionChanseModifier) / 100;

		iPromotionChanse = std::max(iPromotionChanse,50);

		if (GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("COMBAT_DIE_SIDES"), "Occasional Promotion") < iPromotionChanse)
		{
			//select random promotion from available
			PromotionTypes ptPromotion = aDefenderAvailablePromotions[
				GC.getGameINLINE().getSorenRandNum(aDefenderAvailablePromotions.size(), "Select Promotion Type")];
			//promote
			pDefender->setHasPromotion(ptPromotion, true);
			//show message
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNIT_PROMOTED_IN_BATTLE", pDefender->getNameKey(),
				GC.getPromotionInfo(ptPromotion).getText());
			gDLL->getInterfaceIFace()->addMessage(
				pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer,
				GC.getPromotionInfo((PromotionTypes)0).getSound(), MESSAGE_TYPE_INFO, NULL,
				(ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/**	
	//Equipment decay
	CvUnit* pLoopUnit = this;
	for(int iPass = 0; iPass < 2; iPass++)  {
		if(iPass == 1)
			pLoopUnit = pDefender;

		if(pLoopUnit->getEquipmentYieldType() != NO_YIELD)
		{
			CvPlayer& kPlayer = GET_PLAYER(pLoopUnit->getOwnerINLINE());
			kPlayer.changeGlobalYield(pLoopUnit->getEquipmentYieldType(), - (kPlayer.getCombatBonusFromEquipment(pLoopUnit) * pLoopUnit->baseCombatStr()) / 10);				
		}

		if(!pLoopUnit->isDead()) {
			int iStamina = 100 - getDamage();
			iStamina *= std::min(iCombatRounds * 3, 50);
			iStamina /= 100;
			setDamage(getDamage() + iStamina, NO_PLAYER, false);
		}
	}
**/
	//Glorious Victory!
	if(!isDead() && pDefender->isDead())
	{
		doGloriousVictory(iWinningOdds, pDefender);
	}
	else if(isDead() && !pDefender->isDead())
	{
		pDefender->doGloriousVictory((GC.getDefineINT("COMBAT_DIE_SIDES") - iWinningOdds),this);
	}

	//Adjust Defensive/Range Strikes
	int iRangedBonus;
	CvUnit* pRangedSupport = pDefender->getDefensiveStrikeUnit(iRangedBonus,this);
	if(iRangedBonus > 0 && pRangedSupport != NULL)
	{
		pRangedSupport->changeDefensiveStrikesLeft(-1);
	}
	pRangedSupport = getDefensiveStrikeUnit(iRangedBonus,pDefender);
	if(iRangedBonus > 0 && pRangedSupport != NULL)
	{
		pRangedSupport->changeDefensiveStrikesLeft(-1);
	}
}


void CvUnit::updateCombat(bool bQuick)
{
	CvWString szBuffer;

	bool bFinish = false;
	bool bVisible = false;

	if (getCombatTimer() > 0)
	{
		changeCombatTimer(-1);

		if (getCombatTimer() > 0)
		{
			return;
		}
		else
		{
			bFinish = true;
		}
	}

	CvPlot* pPlot = getAttackPlot();

	if (pPlot == NULL)
	{
		return;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		updateAirStrike(pPlot, bQuick, bFinish);
		return;
	}

	CvUnit* pDefender = NULL;
	if (bFinish)
	{
		pDefender = getCombatUnit();
	}
	else
	{
		pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
	}

	if (pDefender == NULL)
	{
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);

		getGroup()->groupMove(pPlot, true, ((canAdvance(pPlot, 0)) ? this : NULL));

		getGroup()->clearMissionQueue();

		return;
	}

	//check if quick combat
	if (!bQuick)
	{
		bVisible = isCombatVisible(pDefender);
	}

	//FAssertMsg((pPlot == pDefender->plot()), "There is not expected to be a defender or the defender's plot is expected to be pPlot (the attack plot)");

//FfH: Added by Kael 07/30/2007
    if (!isImmuneToDefensiveStrike())
    {
//>>>>Refined Defensive Strikes: Modified by Denev 2009/11/07
//		pDefender->doDefensiveStrike(this);
		//incurDefensiveStrike(pDefender); disabled Sephi -- TODO: Begs to be rewritten
//<<<<Refined Defensive Strikes: End Modify
    }


    if (!isImmuneToFear())
    {
		bool bFear=false;

		if (pDefender->getFear() > 0)
		{
            int iChance = baseCombatStr() + 20 + getLevel() - pDefender->baseCombatStr() - pDefender->getLevel();
            if (iChance < 4)
            {
                iChance = 4;
            }
            /*oosLog("FearEffect","Turn:%d,Player:%d,Unit:%S,X:%d,Y:%d,Fear:%d",GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,GC.getUnitInfo(getUnitType()).getDescription()
				,getX_INLINE()
				,getY_INLINE()
				,iChance
			);*/
            if (GC.getGameINLINE().getSorenRandNum(40, "Im afeared!") > iChance)
            {
				bFear=true;
			}
		}

		//SpyFanatic: corrected management of fear effect of combat aura, to affect enemy units
		//if(getFearEffect()>0)
		if(pDefender->getFearEffect()>0)
		{
			//int iChance = getFearEffect() - pDefender->getLevel() - GC.getUnitInfo(pDefender->getUnitType()).getTier()*4;
            int iChance = pDefender->getFearEffect() - getLevel() - GC.getUnitInfo(getUnitType()).getTier()*4;
            if (iChance < 4)
            {
                iChance = 4;
            }

			/*oosLog("FearEffect","Turn:%d,Player:%d,Unit:%S,Defender:%S,X:%d,Y:%d,FearEffect:%d,DefenderFearEffect:%d,level:%d,tier:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,GC.getUnitInfo(getUnitType()).getDescription()
				,GC.getUnitInfo(pDefender->getUnitType()).getDescription()
				,getX_INLINE()
				,getY_INLINE()
				,iChance
				,pDefender->getFearEffect()
				,getLevel()
				,GC.getUnitInfo(getUnitType()).getTier()
			);*/

            if (iChance<GC.getGameINLINE().getSorenRandNum(100, "Im afeared!"))
            {
				bFear=true;
			}
		}

		if(bFear)
		{
            setMadeAttack(true);
            changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
            szBuffer = gDLL->getText("TXT_KEY_MESSAGE_IM_AFEARED", getNameKey());
            gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)getOwner()), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, "Art/Interface/Buttons/Promotions/Fear.dds", (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
            bFinish = true;
        }
    }
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (City MistDensity Combat effect ) Sephi                                               **/
/*************************************************************************************************/
    if (pDefender->plot()->isCity() && pDefender->plot()->getPlotCity()->getMistDensity()>0)
    {
        int iChance = pDefender->plot()->getPlotCity()->getMistDensity()/2 - getLevel()*4;
        if (iChance > 90)
        {
            iChance = 90;
        }

        if (GC.getGameINLINE().getSorenRandNum(100, "Im Lost in the Mist!") < iChance)
        {
            setMadeAttack(true);
            changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
            szBuffer = gDLL->getText("TXT_KEY_LOST_IN_THE_MIST", getNameKey());
            gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)getOwner()), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, ", ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,2,6", (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
            bFinish = true;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	//if not finished and not fighting yet, set up combat damage and mission
	if (!bFinish)
	{
		if (!isFighting())
		{
			if (plot()->isFighting() || pPlot->isFighting())
			{
				return;
			}

			setMadeAttack(true);

			//rotate to face plot
			DirectionTypes newDirection = estimateDirection(this->plot(), pDefender->plot());
			if (newDirection != NO_DIRECTION)
			{
				setFacingDirection(newDirection);
			}

			//rotate enemy to face us
			newDirection = estimateDirection(pDefender->plot(), this->plot());
			if (newDirection != NO_DIRECTION)
			{
				pDefender->setFacingDirection(newDirection);
			}

			setCombatUnit(pDefender, true);
			pDefender->setCombatUnit(this, false);

			pDefender->getGroup()->clearMissionQueue();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
			bool bFocused = (bVisible && isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus());
//			bool bFocused = (bVisible && isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus() && plot()->isInViewport() && pDefender->isInViewport());
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

			if (bFocused)
			{
				DirectionTypes directionType = directionXY(plot(), pPlot);
				//								N			NE				E				SE					S				SW					W				NW
				NiPoint2 directions[8] = {NiPoint2(0, 1), NiPoint2(1, 1), NiPoint2(1, 0), NiPoint2(1, -1), NiPoint2(0, -1), NiPoint2(-1, -1), NiPoint2(-1, 0), NiPoint2(-1, 1)};
				NiPoint3 attackDirection = NiPoint3(directions[directionType].x, directions[directionType].y, 0);
				float plotSize = GC.getPLOT_SIZE();
				NiPoint3 lookAtPoint(plot()->getPoint().x + plotSize / 2 * attackDirection.x, plot()->getPoint().y + plotSize / 2 * attackDirection.y, (plot()->getPoint().z + pPlot->getPoint().z) / 2);
				attackDirection.Unitize();
				gDLL->getInterfaceIFace()->lookAt(lookAtPoint, (((getOwnerINLINE() != GC.getGameINLINE().getActivePlayer()) || gDLL->getGraphicOption(GRAPHICOPTION_NO_COMBAT_ZOOM)) ? CAMERALOOKAT_BATTLE : CAMERALOOKAT_BATTLE_ZOOM_IN), attackDirection);
			}
			else
			{
				PlayerTypes eAttacker = getVisualOwner(pDefender->getTeam());
				CvWString szMessage;

				if (BARBARIAN_PLAYER != eAttacker)
				{
					szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK", GET_PLAYER(getOwnerINLINE()).getNameKey());
				}
				else
				{
					szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK_UNKNOWN");
				}

				gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true);
			}
		}

		FAssertMsg(pDefender != NULL, "Defender is not assigned a valid value");

		FAssertMsg(plot()->isFighting(), "Current unit instance plot is not fighting as expected");
		FAssertMsg(pPlot->isFighting(), "pPlot is not fighting as expected");

		if (!pDefender->canDefend())
		{
			if (!bVisible)
			{
				bFinish = true;
			}
			else
			{
				CvMissionDefinition kMission;
				kMission.setMissionTime(getCombatTimer() * gDLL->getSecsPerTurn());
				kMission.setMissionType(MISSION_SURRENDER);
				kMission.setUnit(BATTLE_UNIT_ATTACKER, this);
				kMission.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
				kMission.setPlot(pPlot);
				gDLL->getEntityIFace()->AddMission(&kMission);

				// Surrender mission
				setCombatTimer(GC.getMissionInfo(MISSION_SURRENDER).getTime());

				GC.getGameINLINE().incrementTurnTimer(getCombatTimer());
			}

			// Kill them!
			pDefender->setDamage(GC.getMAX_HIT_POINTS());
		}
		else
		{
			CvBattleDefinition kBattle;
			kBattle.setUnit(BATTLE_UNIT_ATTACKER, this);
			kBattle.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
			kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN, getDamage());
			kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN, pDefender->getDamage());

			resolveCombat(pDefender, pPlot, kBattle);

			if (!bVisible)
			{
				bFinish = true;
			}
			else
			{
				kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END, getDamage());
				kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END, pDefender->getDamage());
				kBattle.setAdvanceSquare(canAdvance(pPlot, 1));

				if (isRanged() && pDefender->isRanged())
				{
					kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END));
					kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END));
				}
				else
				{
					kBattle.addDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN));
					kBattle.addDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN));
				}

				int iTurns = planBattle( kBattle);
				kBattle.setMissionTime(iTurns * gDLL->getSecsPerTurn());
				setCombatTimer(iTurns);

				GC.getGameINLINE().incrementTurnTimer(getCombatTimer());

				if (pPlot->isActiveVisible(false))
				{
					ExecuteMove(0.5f, true);
					gDLL->getEntityIFace()->AddMission(&kBattle);
				}
			}
		}
	}

	if (bFinish)
	{
		if (bVisible)
		{
			if (isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus())
			{
				if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
				{
					gDLL->getInterfaceIFace()->releaseLockedCamera();
				}
			}
		}

		//end the combat mission if this code executes first

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		gDLL->getEntityIFace()->RemoveUnitFromBattle(pDefender);
/**
		if (isInViewport())
		{
			gDLL->getEntityIFace()->RemoveUnitFromBattle(this);
		}
		if (pDefender->isInViewport())
		{
			gDLL->getEntityIFace()->RemoveUnitFromBattle(pDefender);
		}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		setAttackPlot(NULL, false);
		setCombatUnit(NULL);
		pDefender->setCombatUnit(NULL);
		NotifyEntity(MISSION_DAMAGE);
		pDefender->NotifyEntity(MISSION_DAMAGE);

		if (isDead())
		{
			if (isBarbarian())
			{
				GET_PLAYER(pDefender->getOwnerINLINE()).changeWinsVsBarbs(1);
			}

//FfH Hidden Nationality: Modified by Kael 08/27/2007
//			if (!m_pUnitInfo->isHiddenNationality() && !pDefender->getUnitInfo().isHiddenNationality())
			if (!isHiddenNationality() && !pDefender->isHiddenNationality() && getDuration() == 0 && !m_pUnitInfo->isNoWarWeariness())
//FfH: End Modify

			{
				GET_TEAM(getTeam()).changeWarWeariness(pDefender->getTeam(), *pPlot, GC.getDefineINT("WW_UNIT_KILLED_ATTACKING"));
				GET_TEAM(pDefender->getTeam()).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_KILLED_UNIT_DEFENDING"));
				GET_TEAM(pDefender->getTeam()).AI_changeWarSuccess(getTeam(), GC.getDefineINT("WAR_SUCCESS_DEFENDING"));
			}

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DIED_ATTACKING", getNameKey(), pDefender->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
/*************************************************************************************************/
/**	New Gameoption (InfluenceDrivenWar) merged Sephi                                         	**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
            float fInfluenceRatio = 0.0;
            if (GC.getGameINLINE().isOption(GAMEOPTION_INFLUENCE_DRIVEN_WAR))
			{
				if(CanDeathInfluenceWar() && pDefender->CanInfluenceWar())
				{
					fInfluenceRatio = pDefender->doVictoryInfluence(this, false, false);
					CvWString szTempBuffer;
					szTempBuffer.Format(L" Influence: -%.1f%%", fInfluenceRatio);
					szBuffer += szTempBuffer;
				}
			}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_KILLED_ENEMY_UNIT", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(pDefender->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

//FfH: Added by Kael 07/30/2007
            pDefender->combatWon(this, false);
//FfH: End Add

			// report event to Python, along with some other key state

//FfH: Modified by Kael 08/02/2008
//			CvEventReporter::getInstance().combatResult(pDefender, this);
            if(GC.getUSE_COMBAT_RESULT_CALLBACK())
            {
				CvEventReporter::getInstance().combatResult(pDefender, this);
            }
//FfH: End Modify

		}
		else if (pDefender->isDead())
		{
			if (pDefender->isBarbarian())
			{
				GET_PLAYER(getOwnerINLINE()).changeWinsVsBarbs(1);
			}

//FfH Hidden Nationality: Modified by Kael 08/27/2007
//			if (!m_pUnitInfo->isHiddenNationality() && !pDefender->getUnitInfo().isHiddenNationality())
			if (!isHiddenNationality() && !pDefender->isHiddenNationality() && pDefender->getDuration() == 0 && !pDefender->getUnitInfo().isNoWarWeariness())
//FfH: End Modify

			{
				GET_TEAM(pDefender->getTeam()).changeWarWeariness(getTeam(), *pPlot, GC.getDefineINT("WW_UNIT_KILLED_DEFENDING"));
				GET_TEAM(getTeam()).changeWarWeariness(pDefender->getTeam(), *pPlot, GC.getDefineINT("WW_KILLED_UNIT_ATTACKING"));
				GET_TEAM(getTeam()).AI_changeWarSuccess(pDefender->getTeam(), GC.getDefineINT("WAR_SUCCESS_ATTACKING"));
			}

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_ENEMY", getNameKey(), pDefender->getNameKey());
/*************************************************************************************************/
/**	New Gameoption (InfluenceDrivenWar) merged Sephi                                         	**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
			float fInfluenceRatio = 0.0;
            if (GC.getGameINLINE().isOption(GAMEOPTION_INFLUENCE_DRIVEN_WAR))
			{
				if(pDefender->CanDeathInfluenceWar() && CanInfluenceWar())
				{
					fInfluenceRatio = doVictoryInfluence(pDefender, true, false);
					CvWString szTempBuffer;
					szTempBuffer.Format(L" Influence: +%.1f%%", fInfluenceRatio);
					szBuffer += szTempBuffer;
				}
			}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			if (getVisualOwner(pDefender->getTeam()) != getOwnerINLINE())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED_UNKNOWN", pDefender->getNameKey(), getNameKey());
			}
			else
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WAS_DESTROYED", pDefender->getNameKey(), getNameKey(), getVisualCivAdjective(pDefender->getTeam()));
			}
/*************************************************************************************************/
/**	New Gameoption (InfluenceDrivenWar) merged Sephi                                         	**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
            if (GC.getGameINLINE().isOption(GAMEOPTION_INFLUENCE_DRIVEN_WAR))
			{
				CvWString szTempBuffer;
				szTempBuffer.Format(L" Influence: -%.1f%%", fInfluenceRatio);
				szBuffer += szTempBuffer;
			}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer,GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

//FfH: Added by Kael 05/15/2007
            combatWon(pDefender, true);
//FfH: End Add

			// report event to Python, along with some other key state

//FfH: Modified by Kael 08/02/2008
//			CvEventReporter::getInstance().combatResult(this, pDefender);
            if(GC.getUSE_COMBAT_RESULT_CALLBACK())
            {
				CvEventReporter::getInstance().combatResult(this, pDefender);
            }
//FfH: End Modify

			bool bAdvance = false;

			if (isSuicide())
			{
				kill(true);

				pDefender->kill(false);
				pDefender = NULL;
			}
			else
			{
				bAdvance = canAdvance(pPlot, ((pDefender->canDefend()) ? 1 : 0));

				if (bAdvance)
				{
					if (!isNoCapture()

//FfH: Added by Kael 11/14/2007
					 || GC.getUnitInfo((UnitTypes)pDefender->getUnitType()).getEquipmentPromotion() != NO_PROMOTION
//FfH: End Add

					 )
					{
						pDefender->setCapturingPlayer(getOwnerINLINE());
					}
				}


				pDefender->kill(false);
				pDefender = NULL;

//FfH Fear: Added by Kael 07/30/2007
				//SpyFanatic: corrected management of fear effect of combat aura, to affect enemy units
				if ((getFear() >= 10 || getFearEffect() > 0) && pPlot->isCity() == false)
                //if (getFear()>=10 && pPlot->isCity() == false)
                {
					int iChance = getFear()>=10 ? getFear() : getFearEffect();

                    CvUnit* pLoopUnit;
                    CLLNode<IDInfo>* pUnitNode;
                    pUnitNode = pPlot->headUnitNode();
                    while (pUnitNode != NULL)
                    {
                        pLoopUnit = ::getUnit(pUnitNode->m_data);
                        pUnitNode = pPlot->nextUnitNode(pUnitNode);
                        if (pLoopUnit->isEnemy(getTeam()))
                        {
                            if (!pLoopUnit->isImmuneToFear() && !pLoopUnit->isGarrision())
                            {
								//if (GC.getGameINLINE().getSorenRandNum(20, "Im afeared!") <= (getFear()*(baseCombatStr() + 10 - pLoopUnit->baseCombatStr())/100))

								/*oosLog("FearEffect","Turn:%d,Player:%d,Unit:%S,X:%d,Y:%d,FearEffect:%d,%d,isCity:%d,Chance:%d,baseCombatStr:%d,defenderbasecombat:%d,result:%d"
									,GC.getGameINLINE().getElapsedGameTurns()
									,getOwner()
									,GC.getUnitInfo(getUnitType()).getDescription()
									,getX_INLINE()
									,getY_INLINE()
									,getFear()
									,getFearEffect()
									,pPlot->isCity()
									,iChance
									,baseCombatStr()
									,pLoopUnit->baseCombatStr()
									,iChance * (baseCombatStr() + 10 - pLoopUnit->baseCombatStr())/100
								);*/

                                if (GC.getGameINLINE().getSorenRandNum(20, "Im afeared!") <= iChance * (baseCombatStr() + 10 - pLoopUnit->baseCombatStr())/100)
                                {
                                    pLoopUnit->joinGroup(NULL);
                                    pLoopUnit->withdrawlToNearestValidPlot();
                                }
                            }
                        }
                    }
                }
                bAdvance = canAdvance(pPlot, 0);
//FfH: End Add

				if (!bAdvance)
				{
					changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
					checkRemoveSelectionAfterAttack();
				}
			}

			if (pPlot->getNumVisibleEnemyDefenders(this) == 0)
			{
				getGroup()->groupMove(pPlot, true, ((bAdvance) ? this : NULL));
			}

			// This is is put before the plot advancement, the unit will always try to walk back
			// to the square that they came from, before advancing.

			//modified Sephi, Barbarians might be deleted after successful combat
			//getGroup()->clearMissionQueue();
			if(getGroup() !=NULL)
				getGroup()->clearMissionQueue();
		}
		else
		{

//FfH Promotions: Modified by Kael 08/12/2007
//			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
//			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
//			szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
//			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
//
//			changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
//			checkRemoveSelectionAfterAttack();
//
//			getGroup()->clearMissionQueue();
            if (pDefender->isFleeWithdrawl())
            {
                pDefender->joinGroup(NULL);
                pDefender->setFleeWithdrawl(false);
                pDefender->withdrawlToNearestValidPlot();
//>>>>BUGFfH: Modified by Denev 2009/10/14
/*	When defender fleed, attacker loses movement point as same as wining	*/
/*
				checkRemoveSelectionAfterAttack();
				if (pPlot->getNumVisibleEnemyDefenders(this) == 0)
				{
					getGroup()->groupMove(pPlot, true, ((canAdvance(pPlot, 0)) ? this : NULL));
				}
*/
				if (canAdvance(pPlot, 0))
				{
					getGroup()->groupMove(pPlot, true, this);
				}
				else
				{
					changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
					checkRemoveSelectionAfterAttack();
				}
//<<<<BUGFfH: End Modify
                getGroup()->clearMissionQueue();
/*************************************************************************************************/
/**	BUGFIX (Surrounded Defender already died)	                             				    **/
/*************************************************************************************************/
/** orig
                szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_FLED", pDefender->getNameKey(), getNameKey());
                gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
                szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_UNIT_FLED", pDefender->getNameKey(), getNameKey());
                gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
**/
				if(pDefender!=NULL)
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_FLED", pDefender->getNameKey(), getNameKey());
					gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
					szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_UNIT_FLED", pDefender->getNameKey(), getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
				}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
            }
            if (isFleeWithdrawl())
            {
                joinGroup(NULL);
                setFleeWithdrawl(false);
                szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
                gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_OUR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
                szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_UNIT_WITHDRAW", getNameKey(), pDefender->getNameKey());
                gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_THEIR_WITHDRAWL", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
                changeMoves(std::max(GC.getMOVE_DENOMINATOR(), pPlot->movementCost(this, plot())));
                checkRemoveSelectionAfterAttack();
                getGroup()->clearMissionQueue();
            }
//FfH: End Modify

		}
	}
}

void CvUnit::checkRemoveSelectionAfterAttack()
{
	if (!canMove() || !isBlitz())
	{
		if (IsSelected())
		{
			if (gDLL->getInterfaceIFace()->getLengthSelectionList() > 1)
			{
				gDLL->getInterfaceIFace()->removeFromSelectionList(this);
			}
		}
	}
}


bool CvUnit::isActionRecommended(int iAction)
{
	CvCity* pWorkingCity;
	CvPlot* pPlot;
	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovement;
	BuildTypes eBuild;
	RouteTypes eRoute;
	BonusTypes eBonus;
	int iIndex;

	if (getOwnerINLINE() != GC.getGameINLINE().getActivePlayer())
	{
		return false;
	}

	if (GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_NO_UNIT_RECOMMENDATIONS))
	{
		return false;
	}

	CyUnit* pyUnit = new CyUnit(this);
	CyArgsList argsList;
	argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
	argsList.add(iAction);
	long lResult=0;
	gDLL->getPythonIFace()->callFunction(PYGameModule, "isActionRecommended", argsList.makeFunctionArgs(), &lResult);
	delete pyUnit;	// python fxn must not hold on to this pointer
	if (lResult == 1)
	{
		return true;
	}

	pPlot = gDLL->getInterfaceIFace()->getGotoPlot();

	if (pPlot == NULL)
	{
		if (gDLL->shiftKey())
		{
			pPlot = getGroup()->lastMissionPlot();
		}
	}

	if (pPlot == NULL)
	{
		pPlot = plot();
	}

	if (GC.getActionInfo(iAction).getMissionType() == MISSION_FORTIFY)
	{
		if (pPlot->isCity(true, getTeam()))
		{
			if (canDefend(pPlot))
			{
				if (pPlot->getNumDefenders(getOwnerINLINE()) < ((atPlot(pPlot)) ? 2 : 1))
				{
					return true;
				}
			}
		}
	}

	if (GC.getActionInfo(iAction).getMissionType() == MISSION_HEAL)
	{
		if (isHurt())
		{
			if (!hasMoved())
			{
				if ((pPlot->getTeam() == getTeam()) || (healTurns(pPlot) < 4))
				{
					return true;
				}
			}
		}
	}

	if (GC.getActionInfo(iAction).getMissionType() == MISSION_FOUND)
	{
		if (canFound(pPlot))
		{
			if (pPlot->isBestAdjacentFound(getOwnerINLINE()))
			{
				return true;
			}
		}
	}

	if (GC.getActionInfo(iAction).getMissionType() == MISSION_BUILD)
	{
		if (pPlot->getOwnerINLINE() == getOwnerINLINE())
		{
			eBuild = ((BuildTypes)(GC.getActionInfo(iAction).getMissionData()));
			FAssert(eBuild != NO_BUILD);
			FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

			if (canBuild(pPlot, eBuild))
			{
				eImprovement = ((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement()));
				eRoute = ((RouteTypes)(GC.getBuildInfo(eBuild).getRoute()));
				eBonus = pPlot->getBonusType(getTeam());
				pWorkingCity = pPlot->getWorkingCity();

				if (pPlot->getImprovementType() == NO_IMPROVEMENT)
				{
					if (pWorkingCity != NULL)
					{
						iIndex = pWorkingCity->getCityPlotIndex(pPlot);

						if (iIndex != -1)
						{
							if (pWorkingCity->AI_getBestBuild(iIndex) == eBuild)
							{
								return true;
							}
						}
					}

					if (eImprovement != NO_IMPROVEMENT)
					{
						if (eBonus != NO_BONUS)
						{
							if (GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(eBonus))
							{
								return true;
							}
						}

						if (pPlot->getImprovementType() == NO_IMPROVEMENT)
						{
							if (!(pPlot->isIrrigated()) && pPlot->isIrrigationAvailable(true))
							{
								if (GC.getImprovementInfo(eImprovement).isCarriesIrrigation())
								{
									return true;
								}
							}

							if (pWorkingCity != NULL)
							{
								if (GC.getImprovementInfo(eImprovement).getYieldChange(YIELD_FOOD) > 0)
								{
									return true;
								}

								if (pPlot->isHills())
								{
									if (GC.getImprovementInfo(eImprovement).getYieldChange(YIELD_PRODUCTION) > 0)
									{
										return true;
									}
								}
								else
								{
									if (GC.getImprovementInfo(eImprovement).getYieldChange(YIELD_COMMERCE) > 0)
									{
										return true;
									}
								}
							}
						}
					}
				}

				if (eRoute != NO_ROUTE)
				{
					if (!(pPlot->isRoute()))
					{
						if (eBonus != NO_BONUS)
						{
							return true;
						}

						if (pWorkingCity != NULL)
						{
							if (pPlot->isRiver())
							{
								return true;
							}
						}
					}

					eFinalImprovement = eImprovement;

					if (eFinalImprovement == NO_IMPROVEMENT)
					{
						eFinalImprovement = pPlot->getImprovementType();
					}

					if (eFinalImprovement != NO_IMPROVEMENT)
					{
						if ((GC.getImprovementInfo(eFinalImprovement).getRouteYieldChanges(eRoute, YIELD_FOOD) > 0) ||
							(GC.getImprovementInfo(eFinalImprovement).getRouteYieldChanges(eRoute, YIELD_PRODUCTION) > 0) ||
							(GC.getImprovementInfo(eFinalImprovement).getRouteYieldChanges(eRoute, YIELD_COMMERCE) > 0))
						{
							return true;
						}
					}
				}
			}
		}
	}

	if (GC.getActionInfo(iAction).getCommandType() == COMMAND_PROMOTION)
	{
		return true;
	}

	return false;
}


bool CvUnit::isBetterDefenderThan(const CvUnit* pDefender, const CvUnit* pAttacker) const
{
	int iOurDefense;
	int iTheirDefense;

	if (pDefender == NULL)
	{
		return true;
	}

	TeamTypes eAttackerTeam = NO_TEAM;
	if (NULL != pAttacker)
	{
		eAttackerTeam = pAttacker->getTeam();
	}

	if (canCoexistWithEnemyUnit(eAttackerTeam))
	{
		return false;
	}

	if (!canDefend())
	{
		return false;
	}

	if (canDefend() && !(pDefender->canDefend()))
	{
		return true;
	}

	if (pAttacker)
	{
		if (isTargetOf(*pAttacker) && !pDefender->isTargetOf(*pAttacker))
		{
			return true;
		}

		if (!isTargetOf(*pAttacker) && pDefender->isTargetOf(*pAttacker))
		{
			return false;
		}

		if (pAttacker->canAttack(*pDefender) && !pAttacker->canAttack(*this))
		{
			return false;
		}

		if (pAttacker->canAttack(*this) && !pAttacker->canAttack(*pDefender))
		{
			return true;
		}
	}

	iOurDefense = currCombatStr(plot(), pAttacker);
	if (::isWorldUnitClass(getUnitClassType()))
	{
		iOurDefense /= 2;
	}

	if (NULL == pAttacker)
	{
		if (pDefender->collateralDamage() > 0)
		{
			iOurDefense *= (100 + pDefender->collateralDamage());
			iOurDefense /= 100;
		}

		if (pDefender->currInterceptionProbability() > 0)
		{
			iOurDefense *= (100 + pDefender->currInterceptionProbability());
			iOurDefense /= 100;
		}
	}
	else
	{
		if (!(pAttacker->immuneToFirstStrikes()))
		{
			//IgnoreFirstStrikes tag added, Sephi
			//iOurDefense *= ((((firstStrikes() * 2) + chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iOurDefense *= ((((std::max(0,firstStrikes()-pAttacker->getIgnoreFirstStrikes()) * 2) + chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iOurDefense /= 100;
		}

		if (immuneToFirstStrikes())
		{
			iOurDefense *= ((((pAttacker->firstStrikes() * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iOurDefense /= 100;
		}

		//IgnoreFirstStrikes tag added, Sephi
		else if(getIgnoreFirstStrikes()>0)
		{
			iOurDefense *= ((((std::max(0,std::min(pAttacker->firstStrikes(),getIgnoreFirstStrikes())) * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iOurDefense /= 100;
		}
	}

	int iAssetValue = std::max(1, getUnitInfo().getAssetValue());
	int iCargoAssetValue = 0;
	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		iCargoAssetValue += aCargoUnits[i]->getUnitInfo().getAssetValue();
	}
	iOurDefense = iOurDefense * iAssetValue / std::max(1, iAssetValue + iCargoAssetValue);

	iTheirDefense = pDefender->currCombatStr(plot(), pAttacker);
	if (::isWorldUnitClass(pDefender->getUnitClassType()))
	{
		iTheirDefense /= 2;
	}

	if (NULL == pAttacker)
	{
		if (collateralDamage() > 0)
		{
			iTheirDefense *= (100 + collateralDamage());
			iTheirDefense /= 100;
		}

		if (currInterceptionProbability() > 0)
		{
			iTheirDefense *= (100 + currInterceptionProbability());
			iTheirDefense /= 100;
		}
	}
	else
	{
		if (!(pAttacker->immuneToFirstStrikes()))
		{
			//IgnoreFirstStrikes tag added, Sephi
			//iTheirDefense *= ((((pDefender->firstStrikes() * 2) + pDefender->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iTheirDefense *= ((((std::max(0,pDefender->firstStrikes()-pAttacker->getIgnoreFirstStrikes()) * 2) + pDefender->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iTheirDefense /= 100;
		}

		if (pDefender->immuneToFirstStrikes())
		{
			iTheirDefense *= ((((pAttacker->firstStrikes() * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iTheirDefense /= 100;
		}

		//IgnoreFirstStrikes tag added, Sephi
		else if(pDefender->getIgnoreFirstStrikes()>0)
		{
			iTheirDefense *= ((((std::max(0,std::min(pAttacker->firstStrikes(),pDefender->getIgnoreFirstStrikes())) * 2) + pAttacker->chanceFirstStrikes()) * ((GC.getDefineINT("COMBAT_DAMAGE") * 2) / 5)) + 100);
			iTheirDefense /= 100;
		}

	}

	iAssetValue = std::max(1, pDefender->getUnitInfo().getAssetValue());
	iCargoAssetValue = 0;
	pDefender->getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		iCargoAssetValue += aCargoUnits[i]->getUnitInfo().getAssetValue();
	}
	iTheirDefense = iTheirDefense * iAssetValue / std::max(1, iAssetValue + iCargoAssetValue);

//FfH Promotions: Added by Kael 07/30/2007
    iOurDefense *= getBetterDefenderThanPercent();
    iOurDefense /= 100;
    iTheirDefense *= pDefender->getBetterDefenderThanPercent();
    iTheirDefense /= 100;
//FfH Promotions: End Add

	if (iOurDefense == iTheirDefense)
	{
		if (NO_UNIT == getLeaderUnitType() && NO_UNIT != pDefender->getLeaderUnitType())
		{
			++iOurDefense;
		}
		else if (NO_UNIT != getLeaderUnitType() && NO_UNIT == pDefender->getLeaderUnitType())
		{
			++iTheirDefense;
		}
		else if (isBeforeUnitCycle(this, pDefender))
		{
			++iOurDefense;
		}
	}

	return (iOurDefense > iTheirDefense);
}


bool CvUnit::canDoCommand(CommandTypes eCommand, int iData1, int iData2, bool bTestVisible, bool bTestBusy, CvPlot* pPlot)
{
	CvUnit* pUnit;

	if (bTestBusy && getGroup()->isBusy())
	{
		return false;
	}

	switch (eCommand)
	{
	case COMMAND_PROMOTION:
		if (canPromote((PromotionTypes)iData1, iData2))
		{
			return true;
		}
		break;

	case COMMAND_UPGRADE:
		if (canUpgrade(((UnitTypes)iData1), bTestVisible))
		{
			return true;
		}
		break;

	case COMMAND_AUTOMATE:
		if (canAutomate((AutomateTypes)iData1))
		{
			return true;
		}
		break;
/*************************************************************************************************/
/**	ADDON (automatic Spellcasting) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    case COMMAND_AUTOMATE_SPELL:
        if (canSpellAutomate(iData1))
        {
            return true;
        }
        break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	case COMMAND_RANGESTRIKE:
        if(canRangeStrike(true))
        {
            return true;
        }
		break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	case COMMAND_WAKE:
		if (!isAutomated() && isWaiting())
		{
			return true;
		}
		break;

	case COMMAND_CANCEL:
	case COMMAND_CANCEL_ALL:
		if (!isAutomated() && (getGroup()->getLengthMissionQueue() > 0))
		{
			return true;
		}
		break;

	case COMMAND_STOP_AUTOMATION:
		if (isAutomated())
		{
			return true;
		}
		break;

	case COMMAND_DELETE:
		if (canScrap())
		{
			return true;
		}
		break;

	case COMMAND_GIFT:
		if (canGift(bTestVisible))
		{
			return true;
		}
		break;

	case COMMAND_LOAD:
		if (canLoad(plot()))
		{
			return true;
		}
		break;

	case COMMAND_LOAD_UNIT:
		pUnit = ::getUnit(IDInfo(((PlayerTypes)iData1), iData2));
		if (pUnit != NULL)
		{
			if (canLoadUnit(pUnit, plot()))
			{
				return true;
			}
		}
		break;

	case COMMAND_UNLOAD:
		if (canUnload())
		{
			return true;
		}
		break;

	case COMMAND_UNLOAD_ALL:
		if (canUnloadAll())
		{
			return true;
		}
		break;

	case COMMAND_HOTKEY:
		if (isGroupHead())
		{
			return true;
		}
		break;

//FfH Spell System: Added by Kael 07/23/2007
	case COMMAND_CAST:
		if(canCast(iData1, bTestVisible, pPlot))
		{
			return true;
		}
		break;
	
//FfH: End Add
	case COMMAND_PROJECT:
		return true;
		break;

	case COMMAND_PICK_EQUIPMENT:
		return true;
		break;

	case COMMAND_GARRISION:
		if(canGarrision())
		{
			return true;
		}
		break;
	case COMMAND_MOBILIZE:
		if(canMobilize())
		{
			return true;
		}
		break;
	case COMMAND_USE_PORTAL:
		if(canMove()) {
			if(plot()->isPortal()) {
/**
				//TODO: allow Units to fight through Portals
				if(plot()->getPortalExit()->getNumUnits() == 0) {
					return true;
				}

				if(plot()->getPortalExit()->isVisible(getTeam(), false)) {
					if(!plot()->getPortalExit()->isVisibleEnemyUnit(getOwner())) {
						return true;
					}
				}			
**/
				return true;
			}
		}
		break;
	default:
		FAssert(false);
		break;
	}

	return false;
}


void CvUnit::doCommand(CommandTypes eCommand, int iData1, int iData2)
{
	CvUnit* pUnit;
	bool bCycle;
	int iValue;

	bCycle = false;
	bool bFailed = false;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (canDoCommand(eCommand, iData1, iData2))
	{
		switch (eCommand)
		{
		case COMMAND_PROMOTION:
			promote((PromotionTypes)iData1, iData2);
			break;

		case COMMAND_UPGRADE:
			upgrade((UnitTypes)iData1);
			bCycle = true;
			break;

		case COMMAND_AUTOMATE:
			automate((AutomateTypes)iData1);
			bCycle = true;
			break;
/*************************************************************************************************/
/**	ADDON (automatic spellcasting) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

        case COMMAND_AUTOMATE_SPELL:
            if (getAutoSpellCast()==iData1)
                setAutoSpellCast(NO_SPELL);
            else
                setAutoSpellCast(iData1);
            break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
        case COMMAND_RANGESTRIKE:
            if (isAutomatedRangeStrike())
                setAutomatedRangeStrike(false);
            else
                setAutomatedRangeStrike(true);
            break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

		case COMMAND_WAKE:
			getGroup()->setActivityType(ACTIVITY_AWAKE);
			break;

		case COMMAND_CANCEL:
			getGroup()->popMission();
			break;

		case COMMAND_CANCEL_ALL:
			getGroup()->clearMissionQueue();
			break;

		case COMMAND_STOP_AUTOMATION:
			getGroup()->setAutomateType(NO_AUTOMATE);
			break;

		case COMMAND_DELETE:
			scrap();
			bCycle = true;
			break;

		case COMMAND_GIFT:
			gift();
			bCycle = true;
			break;

		case COMMAND_LOAD:
			load();
			bCycle = true;
			break;

		case COMMAND_LOAD_UNIT:
			pUnit = ::getUnit(IDInfo(((PlayerTypes)iData1), iData2));
			if (pUnit != NULL)
			{
				loadUnit(pUnit);
				bCycle = true;
			}
			break;

		case COMMAND_UNLOAD:
			unload();
			bCycle = true;
			break;

		case COMMAND_UNLOAD_ALL:
			unloadAll();
			bCycle = true;
			break;

		case COMMAND_HOTKEY:
			setHotKeyNumber(iData1);
			break;

//FfH Spell System: Added by Kael 07/23/2007
		case COMMAND_CAST:
/*************************************************************************************************/
/**	ADDON(INTERFACEMODE_SPELL_OFFENSIVE) Sephi					                                **/
/*************************************************************************************************/
			if(GC.getSpellInfo((SpellTypes)iData1).isTargetedHostile())
			{
				setMissionSpell(iData1);
			}
			else
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/

			{
			cast(iData1);
			}
			break;

//FfH: End Add
		case COMMAND_PROJECT:
			if(GET_PLAYER(getOwnerINLINE()).getTerraformPlan()!=NO_PROJECT && GC.getMapINLINE().plot(iData1,iData2)!=NULL && GC.getMapINLINE().plot(iData1,iData2)->doTerraformRitual((ProjectTypes)GET_PLAYER(getOwnerINLINE()).getTerraformPlan(),getOwnerINLINE(), iValue,true))
			{
				GET_PLAYER(getOwnerINLINE()).doTerraformRitual((ProjectTypes)GET_PLAYER(getOwnerINLINE()).getTerraformPlan(),false,GC.getMapINLINE().plot(iData1, iData2));						
			}
			GET_PLAYER(getOwnerINLINE()).setTerraformPlan(NO_PROJECT);
			break;

		case COMMAND_PICK_EQUIPMENT:
			if(iData1!=NO_PROMOTION)
			{
				equip((PromotionTypes)iData1);
			}
			break;
		case COMMAND_GARRISION:
			setGarrision(true);
			if(getOwnerINLINE()==GC.getGameINLINE().getActivePlayer())
			{
				gDLL->getInterfaceIFace()->changeCycleSelectionCounter(1);
			}
			break;
		case COMMAND_MOBILIZE:
			setGarrision(false);
			GET_PLAYER(getOwnerINLINE()).changeGold(-300);
			break;
		case COMMAND_USE_PORTAL:
			getGroup()->groupAttack(plot()->getPortalExit()->getX_INLINE(), plot()->getPortalExit()->getY_INLINE(), -1, bFailed, true);
			getGroup()->groupMove(plot()->getPortalExit(), true, ((canAdvance(plot()->getPortalExit(), 0)) ? this : NULL));
//			move(plot()->getPortalExit(), false);
//			setXY(plot()->getPortalExit()->getX_INLINE(), plot()->getPortalExit()->getY_INLINE(), true);
			finishMoves();
			break;
		default:
			FAssert(false);
			break;
		}
	}

	if (bCycle)
	{
		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setCycleSelectionCounter(1);
		}
	}

	if(!isDelayedDeath())	//added Sephi, Units afer COMMAND_CAST may be dead
	{
		getGroup()->doDelayedDeath();
	}
}


FAStarNode* CvUnit::getPathLastNode() const
{
	return getGroup()->getPathLastNode();
}


CvPlot* CvUnit::getPathEndTurnPlot() const
{
	return getGroup()->getPathEndTurnPlot();
}


bool CvUnit::generatePath(const CvPlot* pToPlot, int iFlags, bool bReuse, int* piPathTurns) const
{
	return getGroup()->generatePath(plot(), pToPlot, iFlags, bReuse, piPathTurns);
}


bool CvUnit::canEnterTerritory(TeamTypes eTeam, bool bIgnoreRightOfPassage) const
{
	if (GET_TEAM(getTeam()).isFriendlyTerritory(eTeam))
	{
		return true;
	}

	if (eTeam == NO_TEAM)
	{
		return true;
	}

	if (isEnemy(eTeam))
	{
		return true;
	}

	if (isRivalTerritory())
	{
		return true;
	}

	if (alwaysInvisible())
	{
		return true;
	}

	if (!bIgnoreRightOfPassage)
	{
		if (GET_TEAM(getTeam()).isOpenBorders(eTeam))
		{
			return true;
		}
	}

//FfH: Added by Kael 09/02/2007 (so hidden nationality units can enter all territories)
    if (isHiddenNationality())
    {
        return true;
    }
    if (GET_TEAM(eTeam).isBarbarian()) // (so barbarians can enter player areas they are at peace with an vice versa)
    {
        return true;
    }
    if (GET_TEAM(getTeam()).isBarbarian())
    {
        return true;
    }
//FfH: End Add

	return false;
}


bool CvUnit::canEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage) const
{
	if (!canEnterTerritory(eTeam, bIgnoreRightOfPassage))
	{
		return false;
	}

	if (isBarbarian() && DOMAIN_LAND == getDomainType())
	{
		if (eTeam != NO_TEAM && eTeam != getTeam())
		{
			if (pArea && pArea->isBorderObstacle(eTeam))
			{
				return false;
			}
		}
	}

	return true;
}


// Returns the ID of the team to declare war against
TeamTypes CvUnit::getDeclareWarMove(const CvPlot* pPlot) const
{
	CvUnit* pUnit;
	TeamTypes eRevealedTeam;

	FAssert(isHuman());

	if (getDomainType() != DOMAIN_AIR)
	{
		eRevealedTeam = pPlot->getRevealedTeam(getTeam(), false);

		if (eRevealedTeam != NO_TEAM)
		{
			if (!canEnterArea(eRevealedTeam, pPlot->area()) || (getDomainType() == DOMAIN_SEA && !canCargoEnterArea(eRevealedTeam, pPlot->area(), false) && getGroup()->isAmphibPlot(pPlot)))
			{

//FfH: Modified by Kael 03/29/2009
//				if (GET_TEAM(getTeam()).canDeclareWar(pPlot->getTeam()))
				if (isAlwaysHostile(pPlot) || GET_TEAM(getTeam()).canDeclareWar(pPlot->getTeam()))
//FfH: End Modify

				{
					return eRevealedTeam;
				}
			}
		}
		else
		{
			if (pPlot->isActiveVisible(false))
			{
				if (canMoveInto(pPlot, true, true, true))
				{
					pUnit = pPlot->plotCheck(PUF_canDeclareWar, getOwnerINLINE(), isAlwaysHostile(pPlot), NO_PLAYER, NO_TEAM, PUF_isVisible, getOwnerINLINE());

					if (pUnit != NULL)
					{
						return pUnit->getTeam();
					}
				}
			}
		}
	}

	return NO_TEAM;
}

bool CvUnit::willRevealByMove(const CvPlot* pPlot) const
{
	int iRange = visibilityRange() + 1;
	for (int i = -iRange; i <= iRange; ++i)
	{
		for (int j = -iRange; j <= iRange; ++j)
		{
			CvPlot* pLoopPlot = ::plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), i, j);
			if (NULL != pLoopPlot)
			{
				if (!pLoopPlot->isRevealed(getTeam(), false) && pPlot->canSeePlot(pLoopPlot, getTeam(), visibilityRange(), NO_DIRECTION))
				{
					return true;
				}
			}
		}
	}

	return false;
}

// bool CvUnit::canMoveInto(const CvPlot* pPlot, bool bAttack, bool bDeclareWar, bool bIgnoreLoad) const
bool CvUnit::canMoveInto(const CvPlot* pPlot, bool bAttack, bool bDeclareWar, bool bIgnoreLoad, bool bTestDanger) const
{
	FAssertMsg(pPlot != NULL, "Plot is not assigned a valid value");

	if (atPlot(pPlot))
	{
		return false;
	}
	/*if(!bAttack && pPlot->getX_INLINE() == 6 && pPlot->getY_INLINE() == 16){
		FAssert(false);
	}*/
	if (pPlot->isImpassable())
	{
		if (!canMoveImpassable())
		{
			return false;
		}
	}

	if(pPlot->isNonPlanar()) {
		return false;
	}

	// Cannot move around in unrevealed land freely
	if (m_pUnitInfo->isNoRevealMap() && willRevealByMove(pPlot))
	{
		return false;
	}

	if (GC.getUSE_SPIES_NO_ENTER_BORDERS())
	{
		if (isSpy() && NO_PLAYER != pPlot->getOwnerINLINE())
		{
			if (!GET_PLAYER(getOwnerINLINE()).canSpiesEnterBorders(pPlot->getOwnerINLINE()))
			{
				return false;
			}
		}
	}

	CvArea *pPlotArea = pPlot->area();
	TeamTypes ePlotTeam = pPlot->getTeam();
	bool bCanEnterArea = canEnterArea(ePlotTeam, pPlotArea);

	if (bCanEnterArea)
	{
		if (pPlot->getFeatureType() != NO_FEATURE)
		{
			if (m_pUnitInfo->getFeatureImpassable(pPlot->getFeatureType()))
			{
				TechTypes eTech = (TechTypes)m_pUnitInfo->getFeaturePassableTech(pPlot->getFeatureType());
				if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech))
				{
					if (DOMAIN_SEA != getDomainType() || pPlot->getTeam() != getTeam())  // sea units can enter impassable in own cultural borders
					{
						return false;
					}
				}
			}
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/17/09                         TC01 & jdog5000      */
/*                                                                                              */
/* Bugfix				                                                                         */
/************************************************************************************************/
/* original bts code
		else
*/
		// always check terrain also
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		{
			if (m_pUnitInfo->getTerrainImpassable(pPlot->getTerrainType()))
			{
				TechTypes eTech = (TechTypes)m_pUnitInfo->getTerrainPassableTech(pPlot->getTerrainType());
				if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech))
				{
					if (DOMAIN_SEA != getDomainType() || pPlot->getTeam() != getTeam())  // sea units can enter impassable in own cultural borders
					{
						if (bIgnoreLoad || !canLoad(pPlot))
						{
							return false;
						}
					}
				}
			}
		}
	}

	switch (getDomainType())
	{
	case DOMAIN_SEA:
		if (!pPlot->isWater() && !canMoveAllTerrain())
		{
			if (!pPlot->isFriendlyCity(*this, true) || !pPlot->isCoastalLand())
			{
				//SpyFanatic: allow boats to pass through forts if gameoption is enabled, maybe isActsAsCity should have been better... but it can mess with existing MoM code
				if(!GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) || pPlot->getImprovementType() == NO_IMPROVEMENT || GC.getImprovementInfo(pPlot->getImprovementType()).getCulture() <= 0)
				{
					return false;
				}
				//return false;
			}
			//SpyFanatic: block ship from passing through Austrin Fortified Outpost
			if(!GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && pPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_FORTIFIED_DUNGEON"))
			{
				return false;
			}
		}
		break;

	case DOMAIN_AIR:
		if (!bAttack)
		{
			bool bValid = false;

			if (pPlot->isFriendlyCity(*this, true))
			{
				bValid = true;

				if (m_pUnitInfo->getAirUnitCap() > 0)
				{
					if (pPlot->airUnitSpaceAvailable(getTeam()) <= 0)
					{
						bValid = false;
					}
				}
			}

			if (!bValid)
			{
				if (bIgnoreLoad || !canLoad(pPlot))
				{
					return false;
				}
			}

/*************************************************************************************************/
/**	ADDON (Block Hawks from rebasing to non teammember Cities) Sephi                            **/
/*************************************************************************************************/
            if (GC.getUnitInfo(getUnitType()).getUnitClassType()==GC.getInfoTypeForString("UNITCLASS_HAWK"))
            {
                if (pPlot && getGroup())
                {
                    if ((pPlot->getTeam() != getTeam()) && pPlot->isCity() && (getGroup()->getNumUnits()==1))
                    {
                        return false;
                    }
                }
            }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

		}

		break;

	case DOMAIN_LAND:
		if (pPlot->isWater() && !canMoveAllTerrain()

//FfH: Added by Kael 08/27/2007 (for boarding)
              && !(bAttack && isBoarding())
//FfH: End Add

		)
		{
			if (!pPlot->isCity() || 0 == GC.getDefineINT("LAND_UNITS_CAN_ATTACK_WATER_CITIES"))
			{
				if (bIgnoreLoad || !isHuman() || plot()->isWater() || !canLoad(pPlot))
				{
					return false;
				}
			}
		}
		break;

	case DOMAIN_IMMOBILE:
		return false;
		break;

	default:
		FAssert(false);
		break;
	}

/*************************************************************************************************/
/**	ADDON (New Promotion CannotLeaveCulture) Sephi        				                        **/
/*************************************************************************************************/
	if (isCannotLeaveCulture())
	{
	    if (plot()->getTeam() == getTeam() && pPlot->getTeam() != getTeam())
	    {
	        return false;
	    }
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (MultibarbAnimals) Sephi   		                                                    **/
/**	                                                         									**/
/*************************************************************************************************/

/** barbs shouldn't attack Wildmana Guardians                                                   **/
/**
	if(getOriginPlot()!=NULL)
	{
		if(getCivilizationType()==GC.getDefineINT("BARBARIAN_CIVILIZATION")
			|| getCivilizationType()==GC.getDefineINT("ANIMAL_CIVILIZATION"))
		{
			if(abs(dxWrap(getOriginPlot()->getX_INLINE() - pPlot->getX_INLINE()))>3)
			{
				return false;
			}

			if(abs(dyWrap(getOriginPlot()->getY_INLINE() - pPlot->getY_INLINE()))>3)
			{
				return false;
			}
		}
	}
**/
    if(isAnimal() && getOwnerINLINE()==PIRATES_PLAYER)
    {
        if(!(pPlot->getTerrainType()==GC.getInfoTypeForString("TERRAIN_OCEAN_DEEP")))
        {
            return false;
        }
    }

	//Animals can only enter their own Lairs
	if (isAnimal() && isBarbarian())
	{
	    if(pPlot->getImprovementType()!=NO_IMPROVEMENT)
	    {
	        UnitTypes eUnit=(UnitTypes)GC.getImprovementInfo(pPlot->getImprovementType()).getSpawnUnitType();

	        if(eUnit!=NO_UNIT && eUnit!=getUnitType())
	        {
	            return false;
	        }
	    }
	}
/**	give enraged animals no movement limits                    									**/
/** orig
//FfH: Modified by Kael 08/04/2007 (So owned animals dont have limited movement)
//	if (isAnimal())
	if (isAnimal() && isBarbarian())
//FfH: End Add
**/
	bool bCanMoveInCulture=false;

	if (isAnimal() && isBarbarian())
	{
		if (isHasPromotion((PromotionTypes)GC.getInfoTypeForString("PROMOTION_RAGE_ANIMAL")))
		{
			bCanMoveInCulture=true;
		}

		if (isHasPromotion((PromotionTypes)GC.getInfoTypeForString("PROMOTION_WEAK")))
		{
			bCanMoveInCulture=true;
		}

		if (isHasPromotion((PromotionTypes)GC.getInfoTypeForString("PROMOTION_CAUTIOUS_ANIMAL")))
		{
			bCanMoveInCulture=true;
		}
	}
	if ((isAnimal() || (AI_getUnitAIType()==UNITAI_ANIMAL)) && isBarbarian() && !bCanMoveInCulture)
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
	{
		if (pPlot->isOwned())
		{
			return false;
		}
/**
/** No idea what this is good for / causes AI to calculate Danger from Animals wrong Sephi
		if (!bAttack)
		{
			if (pPlot->getBonusType() != NO_BONUS)
			{
				return false;
			}

			if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				return false;
			}

			if (pPlot->getNumUnits() > 0)
			{
				return false;
			}
		}
**/
	}

	if (isNoCapture())
	{
		if (!bAttack)
		{
			if (pPlot->isEnemyCity(*this))
			{
				return false;
			}
		}
	}

	if (bAttack && !bTestDanger)
	{
		if (isMadeAttack() && !isBlitz())
		{
			return false;
		}
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		if (bAttack)
		{
			if (!canAirStrike(pPlot))
			{
				return false;
			}
		}
	}
	else
	{
		if (canAttack())
		{
			if (bAttack || !canCoexistWithEnemyUnit(NO_TEAM))
			{
				if (!isHuman() || (pPlot->isVisible(getTeam(), false)))
				{
					if (pPlot->isVisibleEnemyUnit(this) != bAttack)
					{
						//FAssertMsg(isHuman() || (!bDeclareWar || (pPlot->isVisibleOtherUnit(getOwnerINLINE()) != bAttack)), "hopefully not an issue, but tracking how often this is the case when we dont want to really declare war");
						//if (!bDeclareWar || (pPlot->isVisibleOtherUnit(getOwnerINLINE()) != bAttack && !(bAttack && pPlot->getPlotCity() && !isNoCapture())))
						if (!bDeclareWar || (isNoCapture() && bAttack && pPlot->isCity()))
						{
							return false;
						}
					}
				}
			}

			if (bAttack)
			{
				CvUnit* pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
				if (NULL != pDefender)
				{
					if (!canAttack(*pDefender))
					{
						return false;
					}
				}
			}
		}
		else
		{
			if (bAttack)
			{
				return false;
			}

			if (!canCoexistWithEnemyUnit(NO_TEAM))
			{
				if (!isHuman() || pPlot->isVisible(getTeam(), false))
				{
					if (pPlot->isEnemyCity(*this))
					{
						return false;
					}

					if (pPlot->isVisibleEnemyUnit(this))
					{
						return false;
					}
				}
			}
		}

		if (isHuman())
		{
			ePlotTeam = pPlot->getRevealedTeam(getTeam(), false);
			bCanEnterArea = canEnterArea(ePlotTeam, pPlotArea);
		}

		if (!bCanEnterArea)
		{
			FAssert(ePlotTeam != NO_TEAM);

			if (!(GET_TEAM(getTeam()).canDeclareWar(ePlotTeam)))
			{
				return false;
			}

			if (isHuman())
			{
				if (!bDeclareWar)
				{
					return false;
				}
			}
			//TODO::add proper check for sneak attacks
			/**
			else
			{
				if (GET_TEAM(getTeam()).AI_isSneakAttackReady(ePlotTeam))
				{
					if (!(getGroup()->AI_isDeclareWar(pPlot)))
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			**/
			else
			{
				if(GET_TEAM(getTeam()).AI_getWarPlanTarget()!=ePlotTeam)
				{
					return false;
				}
				else if (!(getGroup()->AI_isDeclareWar(pPlot)))
				{
					return false;
				}
			}
		}
	}

//FfH: Added by Kael 09/02/2007
    if (pPlot->getFeatureType() != NO_FEATURE)
    {
        if (GC.getFeatureInfo((FeatureTypes)pPlot->getFeatureType()).getRequireResist() != NO_DAMAGE)
        {
            if (getDamageTypeResist((DamageTypes)GC.getFeatureInfo((FeatureTypes)pPlot->getFeatureType()).getRequireResist()) < GC.getDefineINT("FEATURE_REQUIRE_RESIST_AMOUNT"))
            {
                return false;
            }
        }
    }
    if (pPlot->isOwned())
    {
        if (pPlot->getTeam() != getTeam())
        {
            if (GET_PLAYER(pPlot->getOwnerINLINE()).getSanctuaryTimer() != 0)
            {
                return false;
            }
        }
    }
    if (getLevel() < pPlot->getMinLevel())
    {
        return false;
    }
    if (pPlot->isMoveDisabledHuman())
    {
        if (isHuman())
        {
            return false;
        }
    }
    if (pPlot->isMoveDisabledAI())
    {
        if (!isHuman())
        {
            return false;
        }
    }
//FfH: End Add

	if(isBarbarian())
	{
		if(isAnimal())
		{
			if(GC.getUnitInfo(getUnitType()).getMinWilderness()>pPlot->getWilderness())
			{
				return false;
			}
		}

		if(GC.getGameINLINE().getElapsedGameTurns() < 10)
		{
			if(pPlot->isOwned())
			{
				if(pPlot->getWorkingCity()!=NULL && pPlot->getWorkingCity()->isCapital())
				{
					if(GET_TEAM(getTeam()).isAtWar(pPlot->getTeam()))
					{
						return false;
					}
				}
			}
		}
	}

	if(pPlot->getNumUnits() >= GC.getGameINLINE().getUnitPerTileLimit() && !pPlot->isVisibleEnemyUnit(this))
	{
		if(isLimitedPerTile(getUnitType()))
		{
			int iCounter=0;

			CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
			while (pUnitNode != NULL)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pPlot->nextUnitNode(pUnitNode);

				if(pLoopUnit->getTeam()==getTeam())
				{
					if(isLimitedPerTile(pLoopUnit->getUnitType()))
					{
						iCounter++;
					}
				}
				if(iCounter>=GC.getDefineINT("UNIT_PER_TILE_LIMIT"))
				{
					return false;
				}
			}
		}
	}

	if (GC.getUSE_UNIT_CANNOT_MOVE_INTO_CALLBACK())
	{
		// Python Override
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());	// Player ID
		argsList.add(getID());	// Unit ID
		argsList.add(pPlot->getX());	// Plot X
		argsList.add(pPlot->getY());	// Plot Y
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "unitCannotMoveInto", argsList.makeFunctionArgs(), &lResult);

		if (lResult != 0)
		{
			return false;
		}
	}

	return true;
}


// bool CvUnit::canMoveOrAttackInto(const CvPlot* pPlot, bool bDeclareWar) const
bool CvUnit::canMoveOrAttackInto(const CvPlot* pPlot, bool bDeclareWar, bool bIgnoreLoad, bool bTestDanger) const
{
	return (canMoveInto(pPlot, false, bDeclareWar, bIgnoreLoad, bTestDanger) || canMoveInto(pPlot, true, bDeclareWar, bIgnoreLoad, bTestDanger));
}


bool CvUnit::canMoveThrough(const CvPlot* pPlot) const
{
	return canMoveInto(pPlot, false, false, true);
}


void CvUnit::attack(CvPlot* pPlot, bool bQuick)
{
	FAssert(canMoveInto(pPlot, true));
	FAssert(getCombatTimer() == 0);

	setAttackPlot(pPlot, false);

	updateCombat(bQuick);
}

void CvUnit::fightInterceptor(const CvPlot* pPlot, bool bQuick)
{
	FAssert(getCombatTimer() == 0);

	setAttackPlot(pPlot, true);

	updateAirCombat(bQuick);
}

void CvUnit::attackForDamage(CvUnit *pDefender, int attackerDamageChange, int defenderDamageChange)
{
	FAssert(getCombatTimer() == 0);
	FAssert(pDefender != NULL);
	FAssert(!isFighting());

	if(pDefender == NULL)
	{
		return;
	}

	setAttackPlot(pDefender->plot(), false);

	CvPlot* pPlot = getAttackPlot();
	if (pPlot == NULL)
	{
		return;
	}

	//rotate to face plot
	DirectionTypes newDirection = estimateDirection(this->plot(), pDefender->plot());
	if(newDirection != NO_DIRECTION)
	{
		setFacingDirection(newDirection);
	}

	//rotate enemy to face us
	newDirection = estimateDirection(pDefender->plot(), this->plot());
	if(newDirection != NO_DIRECTION)
	{
		pDefender->setFacingDirection(newDirection);
	}

	//check if quick combat
	bool bVisible = isCombatVisible(pDefender);

	//if not finished and not fighting yet, set up combat damage and mission
	if (!isFighting())
	{
		if (plot()->isFighting() || pPlot->isFighting())
		{
			return;
		}

		setCombatUnit(pDefender, true);
		pDefender->setCombatUnit(this, false);

		pDefender->getGroup()->clearMissionQueue();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		bool bFocused = (bVisible && isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus());
//		bool bFocused = (bVisible && isCombatFocus() && gDLL->getInterfaceIFace()->isCombatFocus() && plot()->isInViewport() && pDefender->isInViewport());
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

		if (bFocused)
		{
			DirectionTypes directionType = directionXY(plot(), pPlot);
			//								N			NE				E				SE					S				SW					W				NW
			NiPoint2 directions[8] = {NiPoint2(0, 1), NiPoint2(1, 1), NiPoint2(1, 0), NiPoint2(1, -1), NiPoint2(0, -1), NiPoint2(-1, -1), NiPoint2(-1, 0), NiPoint2(-1, 1)};
			NiPoint3 attackDirection = NiPoint3(directions[directionType].x, directions[directionType].y, 0);
			float plotSize = GC.getPLOT_SIZE();
			NiPoint3 lookAtPoint(plot()->getPoint().x + plotSize / 2 * attackDirection.x, plot()->getPoint().y + plotSize / 2 * attackDirection.y, (plot()->getPoint().z + pPlot->getPoint().z) / 2);
			attackDirection.Unitize();
			gDLL->getInterfaceIFace()->lookAt(lookAtPoint, (((getOwnerINLINE() != GC.getGameINLINE().getActivePlayer()) || gDLL->getGraphicOption(GRAPHICOPTION_NO_COMBAT_ZOOM)) ? CAMERALOOKAT_BATTLE : CAMERALOOKAT_BATTLE_ZOOM_IN), attackDirection);
		}
		else
		{
			PlayerTypes eAttacker = getVisualOwner(pDefender->getTeam());
			CvWString szMessage;
            if (BARBARIAN_PLAYER != eAttacker)
			{
				szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK", GET_PLAYER(getOwnerINLINE()).getNameKey());
			}
			else
			{
				szMessage = gDLL->getText("TXT_KEY_MISC_YOU_UNITS_UNDER_ATTACK_UNKNOWN");
			}

			gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true);
		}
	}

	FAssertMsg(plot()->isFighting(), "Current unit instance plot is not fighting as expected");
	FAssertMsg(pPlot->isFighting(), "pPlot is not fighting as expected");

	//setup battle object
	CvBattleDefinition kBattle;
	kBattle.setUnit(BATTLE_UNIT_ATTACKER, this);
	kBattle.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
	kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN, getDamage());
	kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN, pDefender->getDamage());

	changeDamage(attackerDamageChange, pDefender->getOwnerINLINE());
	pDefender->changeDamage(defenderDamageChange, getOwnerINLINE());

	if (bVisible)
	{
		kBattle.setDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_END, getDamage());
		kBattle.setDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_END, pDefender->getDamage());
		kBattle.setAdvanceSquare(canAdvance(pPlot, 1));

		kBattle.addDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_ATTACKER, BATTLE_TIME_BEGIN));
		kBattle.addDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_RANGED, kBattle.getDamage(BATTLE_UNIT_DEFENDER, BATTLE_TIME_BEGIN));

		int iTurns = planBattle( kBattle);
		kBattle.setMissionTime(iTurns * gDLL->getSecsPerTurn());
		setCombatTimer(iTurns);

		GC.getGameINLINE().incrementTurnTimer(getCombatTimer());

		if (pPlot->isActiveVisible(false))
		{
			ExecuteMove(0.5f, true);
			gDLL->getEntityIFace()->AddMission(&kBattle);
		}
	}
	else
	{
		setCombatTimer(1);
	}
}


void CvUnit::move(CvPlot* pPlot, bool bShow)
{
	FAssert(canMoveOrAttackInto(pPlot) || isMadeAttack());

	CvPlot* pOldPlot = plot();

	changeMoves(pPlot->movementCost(this, plot()));
	setXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true, bShow && pPlot->isVisibleToWatchingHuman(), bShow);
	//change feature
	FeatureTypes featureType = pPlot->getFeatureType();
	if(featureType != NO_FEATURE)
	{
		CvString featureString(GC.getFeatureInfo(featureType).getOnUnitChangeTo());
		if(!featureString.IsEmpty())
		{
			FeatureTypes newFeatureType = (FeatureTypes) GC.getInfoTypeForString(featureString);
			pPlot->setFeatureType(newFeatureType);
		}
	}

	if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
	{
		if (!(pPlot->isOwned()))
		{
			//spawn birds if trees present - JW
			if (featureType != NO_FEATURE)
			{
				if (GC.getASyncRand().get(100) < GC.getFeatureInfo(featureType).getEffectProbability())
				{
					EffectTypes eEffect = (EffectTypes)GC.getInfoTypeForString(GC.getFeatureInfo(featureType).getEffectType());
					gDLL->getEngineIFace()->TriggerEffect(eEffect, pPlot->getPoint(), (float)(GC.getASyncRand().get(360)));
					gDLL->getInterfaceIFace()->playGeneralSound("AS3D_UN_BIRDS_SCATTER", pPlot->getPoint());
				}
			}
		}
	}

//FfH: Modified by Kael 08/02/2008
//	CvEventReporter::getInstance().unitMove(pPlot, this, pOldPlot);
	if(GC.getUSE_ON_UNIT_MOVE_CALLBACK())
	{
		CvEventReporter::getInstance().unitMove(pPlot, this, pOldPlot);
	}
//FfH: End Modify

}

CvPlot* CvUnit::getNearestValidPlot() const
{
	CvCity* pNearestCity;
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot;
	int iValue;
	int iBestValue;
	int iI;

	FAssertMsg(!isAttacking(), "isAttacking did not return false as expected");
	FAssertMsg(!isFighting(), "isFighting did not return false as expected");

	pNearestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE());

	iBestValue = MAX_INT;
	pBestPlot = NULL;

	for (iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);

		if (pLoopPlot->isValidDomainForLocation(*this))
		{
			if (canMoveInto(pLoopPlot))
			{
				if (canEnterArea(pLoopPlot->getTeam(), pLoopPlot->area()) && !isEnemy(pLoopPlot->getTeam(), pLoopPlot))
				{
					FAssertMsg(!atPlot(pLoopPlot), "atPlot(pLoopPlot) did not return false as expected");

					if ((getDomainType() != DOMAIN_AIR) || pLoopPlot->isFriendlyCity(*this, true))
					{
						if (pLoopPlot->isRevealed(getTeam(), false))
						{
							iValue = (plotDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) * 2);

							if (pNearestCity != NULL)
							{
								iValue += plotDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pNearestCity->getX_INLINE(), pNearestCity->getY_INLINE());
							}

							if (getDomainType() == DOMAIN_SEA && !plot()->isWater())
							{
								if (!pLoopPlot->isWater() || !pLoopPlot->isAdjacentToArea(area()))
								{
									iValue *= 3;
								}
							}
							else
							{
								if (pLoopPlot->area() != area())
								{
									iValue *= 3;
								}
							}

							if (iValue < iBestValue)
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

	return pBestPlot;
}

// false if unit is killed
bool CvUnit::jumpToNearestValidPlot()
{
	CvPlot* pBestPlot = getNearestValidPlot();
	bool bValid = true;

	if (pBestPlot != NULL)
	{
		setXY(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
	}
	else
	{
		kill(false);
		bValid = false;
	}

	return bValid;
}


bool CvUnit::canAutomate(AutomateTypes eAutomate) const
{
	if (eAutomate == NO_AUTOMATE)
	{
		return false;
	}

	if (!isGroupHead())
	{
		return false;
	}

	switch (eAutomate)
	{
	case AUTOMATE_BUILD:
		if ((AI_getUnitAIType() != UNITAI_WORKER) && (AI_getUnitAIType() != UNITAI_WORKER_SEA))
		{
			return false;
		}
		break;

	case AUTOMATE_NETWORK:
		if ((AI_getUnitAIType() != UNITAI_WORKER) || !canBuildRoute())
		{
			return false;
		}
		break;

	case AUTOMATE_CITY:
		if (AI_getUnitAIType() != UNITAI_WORKER)
		{
			return false;
		}
		break;

	case AUTOMATE_EXPLORE:
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      06/02/09                                jdog5000      */
/*                                                                                              */
/* Player Interface                                                                             */
/************************************************************************************************/
/** BTS orig
		if ((!canFight() && (getDomainType() != DOMAIN_SEA)) || (getDomainType() == DOMAIN_AIR) || (getDomainType() == DOMAIN_IMMOBILE))
		{
			return false;
		}
**/
		// Enable exploration for air units
		if ((!canFight() && (getDomainType() != DOMAIN_SEA) && (getDomainType() != DOMAIN_AIR)) || (getDomainType() == DOMAIN_IMMOBILE))
		{
			return false;
		}

		if( getDomainType() == DOMAIN_AIR && !canRecon(NULL) )
		{
			return false;
		}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/
		break;

	case AUTOMATE_RELIGION:
		if (AI_getUnitAIType() != UNITAI_MISSIONARY)
		{
			return false;
		}
		break;
/*************************************************************************************************/
/**	ADDON (automatic terraforming) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
	//TODO:: delete old stuff
	case AUTOMATE_FREE_IMPROVEMENT:
		return false;
		break;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	default:
		FAssert(false);
		break;
	}

	return true;
}

/*************************************************************************************************/
/**	ADDON (New Functions) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
bool CvUnit::canSpellAutomate(int spell)
{
    if (!GET_PLAYER(this->getOwnerINLINE()).isHuman())
    {
        return false;
    }

    if (!GC.getSpellInfo((SpellTypes)spell).isAllowAuto())
    {
        return false;
    }

/*************************************************************************************************/
/**	ADDON (ObsoleteBySpell tag) Sephi                                          					**/
/*************************************************************************************************/
    for (int iSpell=0;iSpell<GC.getNumSpellInfos();iSpell++)
    {
        if(GC.getSpellInfo((SpellTypes)spell).isObsoletedBySpell(iSpell))
        {
            if(canSpellAutomate(iSpell))
            {
                return false;
            }
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

    int currentspell=getAutoSpellCast();
    if (currentspell!=NO_SPELL)
    {
        if (spell!=currentspell)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    bool bvalid=true;
	int iProm1=(PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq1();
	if (iProm1!=NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq1()))
		{
				bvalid=false;
		}
	}
	int iProm2=(PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq2();
	if (iProm2!=NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq2()))
		{
				bvalid=false;
		}
	}

	int iProm3=(PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq3();
	if (iProm3!=NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq3()))
		{
				bvalid=false;
		}
	}

	int iProm4=(PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq4();
	if (iProm4!=NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getPromotionPrereq4()))
		{
				bvalid=false;
		}
	}

    if ((UnitTypes)GC.getSpellInfo((SpellTypes)spell).getUnitPrereq()!=NO_UNIT)
    {
        if (getUnitType()!=(UnitTypes)GC.getSpellInfo((SpellTypes)spell).getUnitPrereq())
        {
            bvalid=false;
        }
    }

    if ((UnitClassTypes)GC.getSpellInfo((SpellTypes)spell).getUnitClassPrereq()!=NO_UNITCLASS)
    {
        if (getUnitClassType()!=(UnitClassTypes)GC.getSpellInfo((SpellTypes)spell).getUnitClassPrereq())
        {
            bvalid=false;
        }
    }

    if ((UnitCombatTypes)GC.getSpellInfo((SpellTypes)spell).getUnitCombatPrereq()!=NO_UNITCLASS)
    {
        if (getUnitCombatType()!=(UnitCombatTypes)GC.getSpellInfo((SpellTypes)spell).getUnitCombatPrereq())
        {
            bvalid=false;
        }
    }

    if ((CivilizationTypes)GC.getSpellInfo((SpellTypes)spell).getCivilizationPrereq()!=NO_CIVILIZATION)
    {
        if (getCivilizationType()!=(CivilizationTypes)GC.getSpellInfo((SpellTypes)spell).getCivilizationPrereq())
        {
            bvalid=false;
        }
    }

    if (GC.getSpellInfo((SpellTypes)spell).getBuildingClassOwnedPrereq()!=NO_BUILDINGCLASS)
    {
        if (GET_PLAYER(getOwnerINLINE()).getBuildingClassCount((BuildingClassTypes)GC.getSpellInfo((SpellTypes)spell).getBuildingClassOwnedPrereq())==0)
        {
            bvalid=false;
        }
    }

    if ((ReligionTypes)GC.getSpellInfo((SpellTypes)spell).getReligionPrereq()!=NO_RELIGION)
    {
        if (getReligion()!=(ReligionTypes)GC.getSpellInfo((SpellTypes)spell).getReligionPrereq())
        {
            bvalid=false;
        }
    }

    if ((ReligionTypes)GC.getSpellInfo((SpellTypes)spell).getStateReligionPrereq()!=NO_RELIGION)
    {
        if (getReligion()!=(ReligionTypes)GC.getSpellInfo((SpellTypes)spell).getStateReligionPrereq())
        {
            bvalid=false;
        }
    }

    return bvalid;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

void CvUnit::automate(AutomateTypes eAutomate)
{
	if (!canAutomate(eAutomate))
	{
		return;
	}

	getGroup()->setAutomateType(eAutomate);
}


bool CvUnit::canScrap() const
{
	if (plot()->isFighting())
	{
		return false;
	}

//FfH: Added by Kael 11/06/2007
    if (GET_PLAYER(getOwnerINLINE()).getTempPlayerTimer() > 0)
    {
        return false;
    }
    if (m_pUnitInfo->getEquipmentPromotion() != NO_PROMOTION)
    {
        return false;
    }
//FfH: End Add
	if(isAIControl())
	{
		return false;
	}

	return true;
}


void CvUnit::scrap()
{
	if (!canScrap())
	{
		return;
	}

//FfH: Added by Kael 11/04/2007
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
	    if (isHasPromotion((PromotionTypes)iI))
	    {
            if (GC.getPromotionInfo((PromotionTypes)iI).getBetrayalChance() != 0)
            {
                betray(BARBARIAN_PLAYER);
            }
        }
	}
//FfH: End Add

	kill(true);
}


bool CvUnit::canGift(bool bTestVisible, bool bTestTransport)
{

//FfH: Added by Kael 04/22/2008 (to disable gifting)
    return false;
//FfH: End Add

	CvPlot* pPlot = plot();
	CvUnit* pTransport = getTransportUnit();

	if (!(pPlot->isOwned()))
	{
		return false;
	}

	if (pPlot->getOwnerINLINE() == getOwnerINLINE())
	{
		return false;
	}

	if (pPlot->isVisibleEnemyUnit(this))
	{
		return false;
	}

	if (pPlot->isVisibleEnemyUnit(pPlot->getOwnerINLINE()))
	{
		return false;
	}

	if (!pPlot->isValidDomainForLocation(*this) && NULL == pTransport)
	{
		return false;
	}

	for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
	{
		if (m_pUnitInfo->getCorporationSpreads(iCorp) > 0)
		{
			return false;
		}
	}

	if (bTestTransport)
	{
		if (pTransport && pTransport->getTeam() != pPlot->getTeam())
		{
			return false;
		}
	}

	if (!bTestVisible)
	{
		if (GET_TEAM(pPlot->getTeam()).isUnitClassMaxedOut(getUnitClassType(), GET_TEAM(pPlot->getTeam()).getUnitClassMaking(getUnitClassType())))
		{
			return false;
		}

		if (GET_PLAYER(pPlot->getOwnerINLINE()).isUnitClassMaxedOut(getUnitClassType(), GET_PLAYER(pPlot->getOwnerINLINE()).getUnitClassMaking(getUnitClassType())))
		{
			return false;
		}

		if (!(GET_PLAYER(pPlot->getOwnerINLINE()).AI_acceptUnit(this)))
		{
			return false;
		}
	}

//FfH: Added by Kael 11/06/2007
    if (GET_PLAYER(getOwnerINLINE()).getTempPlayerTimer() > 0)
    {
        return false;
    }
//FfH: End Add

	return !atWar(pPlot->getTeam(), getTeam());
}


void CvUnit::gift(bool bTestTransport)
{
	CvUnit* pGiftUnit;
	CvWString szBuffer;
	PlayerTypes eOwner;

	if (!canGift(false, bTestTransport))
	{
		return;
	}

	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		aCargoUnits[i]->gift(false);
	}

	FAssertMsg(plot()->getOwnerINLINE() != NO_PLAYER, "plot()->getOwnerINLINE() is not expected to be equal with NO_PLAYER");
	pGiftUnit = GET_PLAYER(plot()->getOwnerINLINE()).initUnit(getUnitType(), getX_INLINE(), getY_INLINE(), AI_getUnitAIType());

	FAssertMsg(pGiftUnit != NULL, "GiftUnit is not assigned a valid value");

	eOwner = getOwnerINLINE();

	pGiftUnit->convert(this);

	GET_PLAYER(pGiftUnit->getOwnerINLINE()).AI_changePeacetimeGrantValue(eOwner, (pGiftUnit->getUnitInfo().getProductionCost() / 5));

	szBuffer = gDLL->getText("TXT_KEY_MISC_GIFTED_UNIT_TO_YOU", GET_PLAYER(eOwner).getNameKey(), pGiftUnit->getNameKey());
	gDLL->getInterfaceIFace()->addMessage(pGiftUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_UNITGIFTED", MESSAGE_TYPE_INFO, pGiftUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pGiftUnit->getX_INLINE(), pGiftUnit->getY_INLINE(), true, true);

	// Python Event
	CvEventReporter::getInstance().unitGifted(pGiftUnit, getOwnerINLINE(), plot());
}


bool CvUnit::canLoadUnit(const CvUnit* pUnit, const CvPlot* pPlot) const
{
	FAssert(pUnit != NULL);
	FAssert(pPlot != NULL);

	if (pUnit == this)
	{
		return false;
	}

	if (pUnit->getTeam() != getTeam())
	{
		return false;
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/30/09                     Mongoose & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
	// From Mongoose SDK
	if (isCargo())
	{
		return false;
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	if (getCargo() > 0)
	{
		return false;
	}

	if (pUnit->isCargo())
	{
		return false;
	}

	if (!(pUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType())))
	{
		return false;
	}

	if (!(pUnit->atPlot(pPlot)))
	{
		return false;
	}

//FfH Hidden Nationality: Modified by Kael 08/27/2007
//	if (!m_pUnitInfo->isHiddenNationality() && pUnit->getUnitInfo().isHiddenNationality())
	if (isHiddenNationality() != pUnit->isHiddenNationality())
//FfH: End Modify

	{
		return false;
	}

	if (NO_SPECIALUNIT != getSpecialUnitType())
	{
		if (GC.getSpecialUnitInfo(getSpecialUnitType()).isCityLoad())
		{
			if (!pPlot->isCity(true, getTeam()))
			{
				return false;
			}
		}
	}

//>>>>Advanced Rules: Added by Denev 2009/12/08
//*** Tweak for FfH rules (to prevent airship loads other airship).
	if (cargoSpace() > 0)
	{
		if (specialCargo() == pUnit->specialCargo() && domainCargo() == pUnit->domainCargo())
		{
			return false;
		}
	}
//<<<<Advanced Rules: End Add

	return true;
}


void CvUnit::loadUnit(CvUnit* pUnit)
{
	if (!canLoadUnit(pUnit, plot()))
	{
		return;
	}

	setTransportUnit(pUnit);
}

bool CvUnit::shouldLoadOnMove(const CvPlot* pPlot) const
{
	if (isCargo())
	{
		return false;
	}

	switch (getDomainType())
	{
	case DOMAIN_LAND:
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/30/09                     Mongoose & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
		if (pPlot->isWater())
*/
		// From Mongoose SDK
		if (pPlot->isWater() && !canMoveAllTerrain())
		{
			return true;
		}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

		break;
	case DOMAIN_AIR:
		if (!pPlot->isFriendlyCity(*this, true))
		{
			return true;
		}

		if (m_pUnitInfo->getAirUnitCap() > 0)
		{
			if (pPlot->airUnitSpaceAvailable(getTeam()) <= 0)
			{
				return true;
			}
		}
		break;
	default:
		break;
	}

	if (m_pUnitInfo->getTerrainImpassable(pPlot->getTerrainType()))
	{
		TechTypes eTech = (TechTypes)m_pUnitInfo->getTerrainPassableTech(pPlot->getTerrainType());
		if (NO_TECH == eTech || !GET_TEAM(getTeam()).isHasTech(eTech))
		{
			return true;
		}
	}

	return false;
}


bool CvUnit::canLoad(const CvPlot* pPlot) const
{
	PROFILE_FUNC();

	FAssert(pPlot != NULL);

	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (canLoadUnit(pLoopUnit, pPlot))
		{
			return true;
		}
	}

	return false;
}


void CvUnit::load()
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;
	int iPass;

	if (!canLoad(plot()))
	{
		return;
	}

	pPlot = plot();

	for (iPass = 0; iPass < 2; iPass++)
	{
		pUnitNode = pPlot->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (canLoadUnit(pLoopUnit, pPlot))
			{
				if ((iPass == 0) ? (pLoopUnit->getOwnerINLINE() == getOwnerINLINE()) : (pLoopUnit->getTeam() == getTeam()))
				{
					setTransportUnit(pLoopUnit);
					break;
				}
			}
		}

		if (isCargo())
		{
			break;
		}
	}
}


bool CvUnit::canUnload() const
{
	CvPlot& kPlot = *(plot());

	if (getTransportUnit() == NULL)
	{
		return false;
	}

	if (!kPlot.isValidDomainForLocation(*this))
	{
		return false;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		if (kPlot.isFriendlyCity(*this, true))
		{
			int iNumAirUnits = kPlot.countNumAirUnits(getTeam());
			CvCity* pCity = kPlot.getPlotCity();
			if (NULL != pCity)
			{
				if (iNumAirUnits >= pCity->getAirUnitCapacity(getTeam()))
				{
					return false;
				}
			}
			else
			{
				if (iNumAirUnits >= GC.getDefineINT("CITY_AIR_UNIT_CAPACITY"))
				{
					return false;
				}
			}
		}
	}

	return true;
}


void CvUnit::unload()
{
	if (!canUnload())
	{
		return;
	}

	setTransportUnit(NULL);
}


bool CvUnit::canUnloadAll() const
{
	if (getCargo() == 0)
	{
		return false;
	}

	return true;
}


void CvUnit::unloadAll()
{
	if (!canUnloadAll())
	{
		return;
	}

	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		CvUnit* pCargo = aCargoUnits[i];
		if (pCargo->canUnload())
		{
			pCargo->setTransportUnit(NULL);
		}
		else
		{
			FAssert(isHuman() || pCargo->getDomainType() == DOMAIN_AIR);
			pCargo->getGroup()->setActivityType(ACTIVITY_AWAKE);
		}
	}
}


bool CvUnit::canHold(const CvPlot* pPlot) const
{
	return true;
}


bool CvUnit::canSleep(const CvPlot* pPlot) const
{
	if (isFortifyable())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


bool CvUnit::canFortify(const CvPlot* pPlot) const
{
	if (!isFortifyable())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


bool CvUnit::canAirPatrol(const CvPlot* pPlot) const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (!canAirDefend(pPlot))
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


bool CvUnit::canSeaPatrol(const CvPlot* pPlot) const
{
	if (!pPlot->isWater())
	{
		return false;
	}

	if (getDomainType() != DOMAIN_SEA)
	{
		return false;
	}

	if (!canFight() || isOnlyDefensive())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


void CvUnit::airCircle(bool bStart)
{
	if (!GC.IsGraphicsInitialized())
	{
		return;
	}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	if (!isInViewport())
	{
		return;
	}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	if ((getDomainType() != DOMAIN_AIR) || (maxInterceptionProbability() == 0))
	{
		return;
	}

	//cancel previos missions
	gDLL->getEntityIFace()->RemoveUnitFromBattle( this );

	if (bStart)
	{
		CvAirMissionDefinition kDefinition;
		kDefinition.setPlot(plot());
		kDefinition.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefinition.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kDefinition.setMissionType(MISSION_AIRPATROL);
		kDefinition.setMissionTime(1.0f); // patrol is indefinite - time is ignored

		gDLL->getEntityIFace()->AddMission( &kDefinition );
	}
}


bool CvUnit::canHeal(const CvPlot* pPlot) const
{
	if (!isHurt())
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	if (healRate(pPlot) <= 0)
	{
		return false;
	}

	return true;
}


bool CvUnit::canSentry(const CvPlot* pPlot) const
{
	if (!canDefend(pPlot))
	{
		return false;
	}

	if (isWaiting())
	{
		return false;
	}

	return true;
}


int CvUnit::healRate(const CvPlot* pPlot) const
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	int iTotalHeal;
	int iHeal;
	int iBestHeal;
	int iI;

//FfH: Added by Kael 11/05/2008
    if (GC.getGameINLINE().isOption(GAMEOPTION_NO_HEALING_FOR_HUMANS))
    {
        if (isHuman())
        {
            if (isAlive())
            {
                return 0;
            }
        }
    }
//FfH: End Add

	pCity = pPlot->getPlotCity();

	iTotalHeal = 0;

	if (pPlot->isCity(true, getTeam()))
	{
		iTotalHeal += GC.getDefineINT("CITY_HEAL_RATE") + (GET_TEAM(getTeam()).isFriendlyTerritory(pPlot->getTeam()) ? getExtraFriendlyHeal() : getExtraNeutralHeal());
		if (pCity && !pCity->isOccupation())
		{
			iTotalHeal += pCity->getHealRate();
		}
	}
	else
	{
		if (!GET_TEAM(getTeam()).isFriendlyTerritory(pPlot->getTeam()))
		{
			if (isEnemy(pPlot->getTeam(), pPlot))
			{
				iTotalHeal += (GC.getDefineINT("ENEMY_HEAL_RATE") + getExtraEnemyHeal());

//FfH Mana Effects: Added by Kael 08/21/2007
				if (pPlot->getOwnerINLINE() != NO_PLAYER)
				{
                    iTotalHeal += GET_PLAYER(pPlot->getOwnerINLINE()).getHealChangeEnemy();
				}
//FfH: End Add

			}
			else
			{
				iTotalHeal += (GC.getDefineINT("NEUTRAL_HEAL_RATE") + getExtraNeutralHeal());
			}
		}
		else
		{
			iTotalHeal += (GC.getDefineINT("FRIENDLY_HEAL_RATE") + getExtraFriendlyHeal());

//FfH Mana Effects: Added by Kael 08/21/2007
            iTotalHeal += GET_PLAYER(getOwnerINLINE()).getHealChange();
//FfH: End Add

		}
	}

	// XXX optimize this (save it?)
	iBestHeal = 0;

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTeam() == getTeam()) // XXX what about alliances?
		{
			iHeal = pLoopUnit->getSameTileHeal();

			if (iHeal > iBestHeal)
			{
				iBestHeal = iHeal;
			}
		}
	}

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->area() == pPlot->area())
			{
				pUnitNode = pLoopPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

					if (pLoopUnit->getTeam() == getTeam()) // XXX what about alliances?
					{
						iHeal = pLoopUnit->getAdjacentTileHeal();

						if (iHeal > iBestHeal)
						{
							iBestHeal = iHeal;
						}
					}
				}
			}
		}
	}

	iTotalHeal += iBestHeal;
	// XXX

//FfH: Added by Kael 10/29/2007
    if (pPlot->getImprovementType() != NO_IMPROVEMENT)
    {
        iTotalHeal += GC.getImprovementInfo((ImprovementTypes)pPlot->getImprovementType()).getHealRateChange();
    }
    if (iTotalHeal < 0)
    {
        iTotalHeal = 0;
    }
//FfH: End Add

	return iTotalHeal;
}


int CvUnit::healTurns(const CvPlot* pPlot) const
{
	int iHeal;
	int iTurns;

	if (!isHurt())
	{
		return 0;
	}

	iHeal = healRate(pPlot);

	if (iHeal > 0)
	{
		iTurns = (getDamage() / iHeal);

		if ((getDamage() % iHeal) != 0)
		{
			iTurns++;
		}

		return iTurns;
	}
	else
	{
		return MAX_INT;
	}
}


void CvUnit::doHeal()
{
	changeDamage(-(healRate(plot())));
}


bool CvUnit::canAirlift(const CvPlot* pPlot) const
{
	CvCity* pCity;

	if (getDomainType() != DOMAIN_LAND)
	{
		return false;
	}

	if (hasMoved())
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getCurrAirlift() >= pCity->getMaxAirlift())
	{
		return false;
	}

	if (pCity->getTeam() != getTeam())
	{
		return false;
	}

	if(getGroup()->getNumUnits()>1)
	{
		return false;
	}

	if(isGarrision())
	{
		return false;
	}

	return true;
}


bool CvUnit::canAirliftAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvPlot* pTargetPlot;
	CvCity* pTargetCity;

	if (!canAirlift(pPlot))
	{
		return false;
	}

	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (!canMoveInto(pTargetPlot))
	{
		return false;
	}

	// Super Forts begin *airlift*
	if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && pTargetPlot->getTeam() != NO_TEAM)
	{
		if (pTargetPlot->getTeam() == getTeam() || GET_TEAM(pTargetPlot->getTeam()).isVassal(getTeam()))
		{
			if (pTargetPlot->getImprovementType() != NO_IMPROVEMENT)
			{
				if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).isActsAsCity())
				{
					return true;
				}
			}
		}
	}
	// Super Forts end

	pTargetCity = pTargetPlot->getPlotCity();

	if (pTargetCity == NULL)
	{
		return false;
	}

	if (pTargetCity->isAirliftTargeted())
	{
		return false;
	}

	if (pTargetCity->getTeam() != getTeam() && !GET_TEAM(pTargetCity->getTeam()).isVassal(getTeam()))
	{
		return false;
	}

	return true;
}


bool CvUnit::airlift(int iX, int iY)
{
	CvCity* pCity;
	CvCity* pTargetCity;
	CvPlot* pTargetPlot;

	if (!canAirliftAt(plot(), iX, iY))
	{
		return false;
	}

	pCity = plot()->getPlotCity();
	FAssert(pCity != NULL);
	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	FAssert(pTargetPlot != NULL);
	pTargetCity = pTargetPlot->getPlotCity();
	FAssert(pTargetCity != NULL);
	FAssert(pCity != pTargetCity);

	pCity->changeCurrAirlift(1);
	if (pTargetCity->getMaxAirlift() == 0)
	{
		pTargetCity->setAirliftTargeted(true);
	}

	finishMoves();

	setXY(pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE());

	return true;
}


bool CvUnit::isNukeVictim(const CvPlot* pPlot, TeamTypes eTeam) const
{
	CvPlot* pLoopPlot;
	int iDX, iDY;

	if (!(GET_TEAM(eTeam).isAlive()))
	{
		return false;
	}

	if (eTeam == getTeam())
	{
		return false;
	}

	for (iDX = -(nukeRange()); iDX <= nukeRange(); iDX++)
	{
		for (iDY = -(nukeRange()); iDY <= nukeRange(); iDY++)
		{
			pLoopPlot	= plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->getTeam() == eTeam)
				{
					return true;
				}

				if (pLoopPlot->plotCheck(PUF_isCombatTeam, eTeam, getTeam()) != NULL)
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvUnit::canNuke(const CvPlot* pPlot) const
{
	if (nukeRange() == -1)
	{
		return false;
	}

	return true;
}


bool CvUnit::canNukeAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvPlot* pTargetPlot;
	int iI;

	if (!canNuke(pPlot))
	{
		return false;
	}

	int iDistance = plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
	if (iDistance <= nukeRange())
	{
		return false;
	}

	if (airRange() > 0 && iDistance > airRange())
	{
		return false;
	}

	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (isNukeVictim(pTargetPlot, ((TeamTypes)iI)))
		{
			if (!isEnemy((TeamTypes)iI, pPlot))
			{
				return false;
			}
		}
	}

	return true;
}


bool CvUnit::nuke(int iX, int iY)
{
	CvPlot* pPlot;
	CvWString szBuffer;
	bool abTeamsAffected[MAX_TEAMS];
	TeamTypes eBestTeam;
	int iBestInterception;
	int iI, iJ, iK;

	if (!canNukeAt(plot(), iX, iY))
	{
		return false;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		abTeamsAffected[iI] = isNukeVictim(pPlot, ((TeamTypes)iI));
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (abTeamsAffected[iI])
		{
			if (!isEnemy((TeamTypes)iI))
			{
				GET_TEAM(getTeam()).declareWar(((TeamTypes)iI), false, WARPLAN_LIMITED);
			}
		}
	}

	iBestInterception = 0;
	eBestTeam = NO_TEAM;

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (abTeamsAffected[iI])
		{
			if (GET_TEAM((TeamTypes)iI).getNukeInterception() > iBestInterception)
			{
				iBestInterception = GET_TEAM((TeamTypes)iI).getNukeInterception();
				eBestTeam = ((TeamTypes)iI);
			}
		}
	}

	iBestInterception *= (100 - m_pUnitInfo->getEvasionProbability());
	iBestInterception /= 100;

	setReconPlot(pPlot);

	if (GC.getGameINLINE().getSorenRandNum(100, "Nuke") < iBestInterception)
	{
		for (iI = 0; iI < MAX_PLAYERS; iI++)
		{
			if (GET_PLAYER((PlayerTypes)iI).isAlive())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_NUKE_INTERCEPTED", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey(), GET_TEAM(eBestTeam).getName().GetCString());
				gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), (((PlayerTypes)iI) == getOwnerINLINE()), GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NUKE_INTERCEPTED", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			}
		}

		if (pPlot->isActiveVisible(false))
		{
			// Nuke entity mission
			CvMissionDefinition kDefiniton;
			kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_NUKE).getTime() * gDLL->getSecsPerTurn());
			kDefiniton.setMissionType(MISSION_NUKE);
			kDefiniton.setPlot(pPlot);
			kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
			kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, this);

			// Add the intercepted mission (defender is not NULL)
			gDLL->getEntityIFace()->AddMission(&kDefiniton);
		}

		kill(true);
		return true; // Intercepted!!! (XXX need special event for this...)
	}

	if (pPlot->isActiveVisible(false))
	{
		// Nuke entity mission
		CvMissionDefinition kDefiniton;
		kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_NUKE).getTime() * gDLL->getSecsPerTurn());
		kDefiniton.setMissionType(MISSION_NUKE);
		kDefiniton.setPlot(pPlot);
		kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, NULL);

		// Add the non-intercepted mission (defender is NULL)
		gDLL->getEntityIFace()->AddMission(&kDefiniton);
	}

	setMadeAttack(true);
	setAttackPlot(pPlot, false);

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (abTeamsAffected[iI])
		{
			GET_TEAM((TeamTypes)iI).changeWarWeariness(getTeam(), 100 * GC.getDefineINT("WW_HIT_BY_NUKE"));
			GET_TEAM(getTeam()).changeWarWeariness(((TeamTypes)iI), 100 * GC.getDefineINT("WW_ATTACKED_WITH_NUKE"));
			GET_TEAM(getTeam()).AI_changeWarSuccess(((TeamTypes)iI), GC.getDefineINT("WAR_SUCCESS_NUKE"));
		}
	}

	for (iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (iI != getTeam())
			{
				if (abTeamsAffected[iI])
				{
					for (iJ = 0; iJ < MAX_PLAYERS; iJ++)
					{
						if (GET_PLAYER((PlayerTypes)iJ).isAlive())
						{
							if (GET_PLAYER((PlayerTypes)iJ).getTeam() == ((TeamTypes)iI))
							{
								GET_PLAYER((PlayerTypes)iJ).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_NUKED_US, 1);
							}
						}
					}
				}
				else
				{
					for (iJ = 0; iJ < MAX_TEAMS; iJ++)
					{
						if (GET_TEAM((TeamTypes)iJ).isAlive())
						{
							if (abTeamsAffected[iJ])
							{
								if (GET_TEAM((TeamTypes)iI).isHasMet((TeamTypes)iJ))
								{
									if (GET_TEAM((TeamTypes)iI).AI_getAttitude((TeamTypes)iJ) >= ATTITUDE_CAUTIOUS)
									{
										for (iK = 0; iK < MAX_PLAYERS; iK++)
										{
											if (GET_PLAYER((PlayerTypes)iK).isAlive())
											{
												if (GET_PLAYER((PlayerTypes)iK).getTeam() == ((TeamTypes)iI))
												{
													GET_PLAYER((PlayerTypes)iK).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_NUKED_FRIEND, 1);
												}
											}
										}
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// XXX some AI should declare war here...

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_NUKE_LAUNCHED", GET_PLAYER(getOwnerINLINE()).getNameKey(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)iI), (((PlayerTypes)iI) == getOwnerINLINE()), GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NUKE_EXPLODES", MESSAGE_TYPE_MAJOR_EVENT, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
		}
	}

	if (isSuicide())
	{
		kill(true);
	}

	return true;
}


bool CvUnit::canRecon(const CvPlot* pPlot) const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (airRange() == 0)
	{
		return false;
	}

	if (m_pUnitInfo->isSuicide())
	{
		return false;
	}

	return true;
}



bool CvUnit::canReconAt(const CvPlot* pPlot, int iX, int iY) const
{
	if (!canRecon(pPlot))
	{
		return false;
	}

	int iDistance = plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY);
	if (iDistance > airRange() || 0 == iDistance)
	{
		return false;
	}

	return true;
}


bool CvUnit::recon(int iX, int iY)
{
	CvPlot* pPlot;

	if (!canReconAt(plot(), iX, iY))
	{
		return false;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	setReconPlot(pPlot);

	finishMoves();

	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_RECON);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_RECON).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}

	return true;
}


bool CvUnit::canParadrop(const CvPlot* pPlot) const
{
	if (getDropRange() <= 0)
	{
		return false;
	}

	if (hasMoved())
	{
		return false;
	}

	if (!pPlot->isFriendlyCity(*this, true))
	{
		return false;
	}

	return true;
}



bool CvUnit::canParadropAt(const CvPlot* pPlot, int iX, int iY) const
{
	if (!canParadrop(pPlot))
	{
		return false;
	}

	CvPlot* pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);
	if (NULL == pTargetPlot || pTargetPlot == pPlot)
	{
		return false;
	}

	if (!pTargetPlot->isVisible(getTeam(), false))
	{
		return false;
	}

	if (!canMoveInto(pTargetPlot, false, false, true))
	{
		return false;
	}

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iX, iY) > getDropRange())
	{
		return false;
	}

	if (!canCoexistWithEnemyUnit(NO_TEAM))
	{
		if (pTargetPlot->isEnemyCity(*this))
		{
			return false;
		}

		if (pTargetPlot->isVisibleEnemyUnit(this))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::paradrop(int iX, int iY)
{
	if (!canParadropAt(plot(), iX, iY))
	{
		return false;
	}

	CvPlot* pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	changeMoves(GC.getMOVE_DENOMINATOR() / 2);
	setMadeAttack(true);

	setXY(pPlot->getX_INLINE(), pPlot->getY_INLINE());

	//check if intercepted
	if(interceptTest(pPlot))
	{
		return true;
	}

	//play paradrop animation by itself
	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_PARADROP);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_PARADROP).getTime() * gDLL->getSecsPerTurn());
		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}

	return true;
}


bool CvUnit::canAirBomb(const CvPlot* pPlot) const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (airBombBaseRate() == 0)
	{
		return false;
	}

	if (isMadeAttack())
	{
		return false;
	}

	return true;
}


bool CvUnit::canAirBombAt(const CvPlot* pPlot, int iX, int iY) const
{
	CvCity* pCity;
	CvPlot* pTargetPlot;

	if (!canAirBomb(pPlot))
	{
		return false;
	}

	pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}

	if (pTargetPlot->isOwned())
	{
		if (!potentialWarAction(pTargetPlot))
		{
			return false;
		}
	}

	pCity = pTargetPlot->getPlotCity();

	if (pCity != NULL)
	{
		if (!(pCity->isBombardable(this)))
		{
			return false;
		}
	}
	else
	{
		if (pTargetPlot->getImprovementType() == NO_IMPROVEMENT)
		{
			return false;
		}

		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).isPermanent())
		{
			return false;
		}

		if (GC.getImprovementInfo(pTargetPlot->getImprovementType()).getAirBombDefense() == -1)
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::airBomb(int iX, int iY)
{
	CvCity* pCity;
	CvPlot* pPlot;
	CvWString szBuffer;

	if (!canAirBombAt(plot(), iX, iY))
	{
		return false;
	}

	pPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (!isEnemy(pPlot->getTeam()))
	{
		getGroup()->groupDeclareWar(pPlot, true);
	}

	if (!isEnemy(pPlot->getTeam()))
	{
		return false;
	}

	if (interceptTest(pPlot))
	{
		return true;
	}

	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		pCity->changeDefenseModifier(-airBombCurrRate());

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DEFENSES_REDUCED_TO", pCity->getNameKey(), pCity->getDefenseModifier(false), getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

		szBuffer = gDLL->getText("TXT_KEY_MISC_ENEMY_DEFENSES_REDUCED_TO", getNameKey(), pCity->getNameKey(), pCity->getDefenseModifier(false));
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());
	}
	else
	{
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getGameINLINE().getSorenRandNum(airBombCurrRate(), "Air Bomb - Offense") >=
					GC.getGameINLINE().getSorenRandNum(GC.getImprovementInfo(pPlot->getImprovementType()).getAirBombDefense(), "Air Bomb - Defense"))
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_DESTROYED_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

				if (pPlot->isOwned())
				{
					szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_IMP_WAS_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), getNameKey(), getVisualCivAdjective(pPlot->getTeam()));
					gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
				}

				pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));
			}
			else
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_UNIT_FAIL_DESTROY_IMP", getNameKey(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMB_FAILS", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			}
		}
	}

	setReconPlot(pPlot);

	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());

	if (pPlot->isActiveVisible(false))
	{
		CvAirMissionDefinition kAirMission;
		kAirMission.setMissionType(MISSION_AIRBOMB);
		kAirMission.setUnit(BATTLE_UNIT_ATTACKER, this);
		kAirMission.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		kAirMission.setDamage(BATTLE_UNIT_DEFENDER, 0);
		kAirMission.setDamage(BATTLE_UNIT_ATTACKER, 0);
		kAirMission.setPlot(pPlot);
		kAirMission.setMissionTime(GC.getMissionInfo((MissionTypes)MISSION_AIRBOMB).getTime() * gDLL->getSecsPerTurn());

		gDLL->getEntityIFace()->AddMission(&kAirMission);
	}

	if (isSuicide())
	{
		kill(true);
	}

	return true;
}


CvCity* CvUnit::bombardTarget(const CvPlot* pPlot) const
{
	int iBestValue = MAX_INT;
	CvCity* pBestCity = NULL;

	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		CvPlot* pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			CvCity* pLoopCity = pLoopPlot->getPlotCity();

			if (pLoopCity != NULL)
			{
				if (pLoopCity->isBombardable(this))
				{
					int iValue = pLoopCity->getDefenseDamage();

					// always prefer cities we are at war with
					if (isEnemy(pLoopCity->getTeam(), pPlot))
					{
						iValue *= 128;
					}

					if (iValue < iBestValue)
					{
						iBestValue = iValue;
						pBestCity = pLoopCity;
					}
				}
			}
		}
	}

	return pBestCity;
}

// Super Forts begin *bombard*
CvPlot* CvUnit::bombardImprovementTarget(const CvPlot* pPlot) const
{
	int iBestValue = MAX_INT;
	CvPlot* pBestPlot = NULL;

	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		CvPlot* pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			if (pLoopPlot->isBombardable(this))
			{
				int iValue = pLoopPlot->getDefenseDamage();

				// always prefer cities we are at war with
				if (isEnemy(pLoopPlot->getTeam(), pPlot))
				{
					iValue *= 128;
				}

				if (iValue < iBestValue)
				{
					iBestValue = iValue;
					pBestPlot = pLoopPlot;
				}
			}
		}
	}

	return pBestPlot;
}
// Super Forts end

bool CvUnit::canBombard(const CvPlot* pPlot)
{
	//can we Bombard at all?
	if(pPlot==NULL)
	{
		if (bombardRate() <= 0 || getUnitInfo().getBombardRange() <=0)
		{
			return false;
		}
		return true;
	}

	if (bombardRate() <= 0)
	{
		return false;
	}

	if (isMadeAttack())
	{
		return false;
	}

	if (isCargo())
	{
		return false;
	}

	if(getUnitInfo().getBombardRange()<stepDistance(getX_INLINE(),getY_INLINE(),pPlot->getX_INLINE(),pPlot->getY_INLINE()))
	{
		return false;
	}

	//check if we have a target
	if(!(pPlot->isCity() || pPlot->getNumUnits()>0))
	{
		return false;
	}

	if(bombard(pPlot,true)<=0)
	{
		return false;
	}

	return true;
}


int CvUnit::bombard(const CvPlot* pTarget, bool bCoundDamageOnly)
{
	int iDamageDone=0;

	if(pTarget==NULL)
	{
		return 0;
	}

	CvCity* pBombardCity = pTarget->getPlotCity();

	if((pBombardCity!=NULL) && (GET_TEAM(pBombardCity->getTeam()).isAtWar(getTeam())))
	{
		int iMaxDamage = (pBombardCity->getTotalDefense(false) * bombardRate()) / 100;
		iMaxDamage = std::min(iMaxDamage, pBombardCity->getTotalDefense(false));

		int iCurrentDamage = pBombardCity->getTotalDefense(false) - pBombardCity->getDefenseModifier(false);

		if(iMaxDamage>iCurrentDamage)
		{
			int iBombardModifier = 0;
//			if (!ignoreBuildingDefense())
//			{
//				iBombardModifier -= pBombardCity->getBuildingBombardDefense();
//			}

			int iBombardDamage=((iMaxDamage - iCurrentDamage)/3 * std::max(0, 100 + iBombardModifier)) / 100;
			iBombardDamage=std::max(1,iBombardDamage);

			if(bCoundDamageOnly)
			{
				iDamageDone += 50*iBombardDamage;
			}
			else
			{
				pBombardCity->changeDefenseModifier(iBombardDamage);

				CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_DEFENSES_IN_CITY_REDUCED_TO", pBombardCity->getNameKey(), pBombardCity->getDefenseModifier(false), GET_PLAYER(getOwnerINLINE()).getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pBombardCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pBombardCity->getX_INLINE(), pBombardCity->getY_INLINE(), true, true);

				szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_REDUCE_CITY_DEFENSES", getNameKey(), pBombardCity->getNameKey(), pBombardCity->getDefenseModifier(false));
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pBombardCity->getX_INLINE(), pBombardCity->getY_INLINE());
			}
		}
	}
	else if(pBombardCity==NULL && GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && pTarget->isBombardable(this))
	{
		//bombardImprovementTarget ? if (pLoopPlot->isBombardable(this))
		//pTarget->changeDefenseDamage(bombardRate());
		iDamageDone = bombardRate();
		CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_DEFENSES_IN_CITY_REDUCED_TO", GC.getImprovementInfo(pTarget->getImprovementType()).getText(),
				(GC.getImprovementInfo(pTarget->getImprovementType()).getDefenseModifier()-pTarget->getDefenseDamage()), GET_PLAYER(getOwnerINLINE()).getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pTarget->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARDED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pTarget->getX_INLINE(), pTarget->getY_INLINE(), true, true);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_REDUCE_CITY_DEFENSES", getNameKey(), GC.getImprovementInfo(pTarget->getImprovementType()).getText(),
			(GC.getImprovementInfo(pTarget->getImprovementType()).getDefenseModifier()-pTarget->getDefenseDamage()));
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BOMBARD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pTarget->getX_INLINE(), pTarget->getY_INLINE());

	}
	//apply Damage to units on plot
	int iMaxDamage=bombardRate()/10;

	if(pTarget->isCity())
	{
		iMaxDamage*=2;
	}

	int iDamagedoneByUnit=0;
	for (CLLNode<IDInfo>* pUnitNode = pTarget->headUnitNode(); pUnitNode != NULL; pUnitNode = pTarget->nextUnitNode(pUnitNode))
	{
		CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
		int iRangedDamage=std::max(0,iMaxDamage-pLoopUnit->getDamage()-pLoopUnit->getCollateralDamageProtection());
		if(bCoundDamageOnly)
		{
			if(pLoopUnit->isEnemy(getTeam()))
			{
				iDamageDone+=iRangedDamage;
			}
		}
		else
		{
			pLoopUnit->setDamage(pLoopUnit->getDamage()+iRangedDamage, getOwnerINLINE(), false);
			iDamagedoneByUnit+=iRangedDamage;

			CvWString szBuffer;
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pLoopUnit->getNameKey(), getNameKey(), -((std::min(iMaxDamage,pLoopUnit->getDamage())*100) / pLoopUnit->maxHitPoints()), GET_PLAYER(getVisualOwner(pLoopUnit->getTeam())).getCivilizationAdjectiveKey());
			//red icon over attacking unit
			gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COMBAT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), this->getX_INLINE(), this->getY_INLINE(), true, true);
			//white icon over defending unit
			gDLL->getInterfaceIFace()->addMessage(pLoopUnit->getOwnerINLINE(), false, 0, L"", "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, pLoopUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pLoopUnit->getNameKey(), -((std::min(iMaxDamage,pLoopUnit->getDamage())*100) / pLoopUnit->maxHitPoints()), GET_PLAYER(pLoopUnit->getVisualOwner(getTeam())).getCivilizationAdjectiveKey());

			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pLoopUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pTarget->getX_INLINE(), pTarget->getY_INLINE());
		}
/**
		if(bombardRate()*3<iDamagedoneByUnit)
		{
			break;
		}
		**/
	}

	if(bCoundDamageOnly)
	{
		return iDamageDone;
	}
	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());

	PlayerTypes iDefender=NO_PLAYER;
	if(pTarget->getOwnerINLINE()!=NO_PLAYER)
	{
		iDefender=pTarget->getOwnerINLINE();
	}
	else if(pTarget->getVisibleEnemyDefender(getOwnerINLINE())!=NULL)
	{
		iDefender=pTarget->getVisibleEnemyDefender(getOwnerINLINE())->getOwnerINLINE();
	}
	CvUnit* pDefender=pTarget->getVisibleEnemyDefender(getOwnerINLINE());

	if(!GC.getGameINLINE().isNetworkMultiPlayer() && pDefender!=NULL)
	{
		if (pTarget->isActiveVisible(false))
		{
			CvUnit *pDefender = pTarget->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);

			// Bombard entity mission
			CvMissionDefinition kDefiniton;
			kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_BOMBARD).getTime() * gDLL->getSecsPerTurn());
			kDefiniton.setMissionType(MISSION_BOMBARD);
			kDefiniton.setPlot(pTarget);
			kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
			kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
			gDLL->getEntityIFace()->AddMission(&kDefiniton);
		}
	}

	return iDamageDone;
}


bool CvUnit::canPillage(const CvPlot* pPlot) const
{
/** modified Sephi **/
//	if (!(m_pUnitInfo->isPillage()))
	if (m_pUnitInfo->isNoPillage())
	{
		return false;
	}
	if (getInvisibleType()!=NO_INVISIBLE)
	{
		return false;
	}

	//after this only checks for specific plot follow
	if(pPlot == NULL)
	{
		return true;
	}

	if(!pPlot->canBePillaged()) {
		return false;
	}

	if (!canAttack())
	{
		if(pPlot->getOwnerINLINE() != getOwnerINLINE())
		{
			return false;
		}
	}

	if (pPlot->isOwned())
	{
		if (!potentialWarAction(pPlot))
		{
			if ((pPlot->getImprovementType() == NO_IMPROVEMENT) || (pPlot->getOwnerINLINE() != getOwnerINLINE()))
			{
				return false;
			}
		}
	}

	if (!(pPlot->isValidDomainForAction(*this)))
	{
		return false;
	}

//FfH: Added by Kael 03/24/2009
    if (isBarbarian())
    {
        if (pPlot->getImprovementType() != NO_IMPROVEMENT)
        {
            if (GC.getImprovementInfo(pPlot->getImprovementType()).getSpawnUnitType() != NO_UNIT)
            {
                return false;
            }
        }

		if (!pPlot->isOwned())
			return false;
		//added Sephi
		if (pPlot->isGoody())
		{
			return false;
		}
    }
//FfH: End Add

	return true;
}


bool CvUnit::pillage()
{
	CvWString szBuffer;
	ImprovementTypes eTempImprovement = NO_IMPROVEMENT;
	RouteTypes eTempRoute = NO_ROUTE;

	CvPlot* pPlot = plot();

	if (!canPillage(pPlot))
	{
		return false;
	}

	if (pPlot->isOwned())
	{
		// we should not be calling this without declaring war first, so do not declare war here
		if (!isEnemy(pPlot->getTeam(), pPlot))
		{
			if ((pPlot->getImprovementType() == NO_IMPROVEMENT) || (pPlot->getOwnerINLINE() != getOwnerINLINE()))
			{
				return false;
			}
		}
	}

	if (pPlot->isWater())
	{
		CvUnit* pInterceptor = bestSeaPillageInterceptor(this, GC.getDefineINT("COMBAT_DIE_SIDES") / 2);
		if (NULL != pInterceptor)
		{
			setMadeAttack(false);

			int iWithdrawal = withdrawalProbability();
			changeExtraWithdrawal(-iWithdrawal); // no withdrawal since we are really the defender
			attack(pInterceptor->plot(), false);
			changeExtraWithdrawal(iWithdrawal);

			return false;
		}
	}

	if (pPlot->getImprovementType() != NO_IMPROVEMENT)
	{
		eTempImprovement = pPlot->getImprovementType();

		if (pPlot->getTeam() != getTeam() && pPlot->getTeam() != NO_TEAM && 
			GET_PLAYER(pPlot->getOwner()).isAlive())
		{
			int iPlunder = 0;
			YieldTypes eYield = NO_YIELD;

			for(int iI = 0; iI < GC.getNUM_YIELD_TYPES(); ++iI) {
				if(pPlot->getYield((YieldTypes)iI) > iPlunder) {
					if(pPlot->getWorkingCity() != NULL) {
						iPlunder = pPlot->getYield((YieldTypes)iI);
						eYield = (YieldTypes)iI;
					}
				}
			}

			if(eYield != NO_YIELD) {
				iPlunder = (int)pow((float)iPlunder, 1.7f);
				iPlunder *= 12;
				iPlunder += (iPlunder * GET_PLAYER(getOwnerINLINE()).getPillagingGold()) / 100;
				iPlunder += (iPlunder * getPillageChange()) / 100;
				iPlunder *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
				iPlunder /= 100;

				if (iPlunder > 0)
				{
					int iPlunderDuration = std::max(15, iPlunder / 20); 

					pPlot->setPlundered(iPlunderDuration);
					for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
					{
						CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), (DirectionTypes) iI);
						if (pAdjacentPlot != NULL)
						{
							pAdjacentPlot->setPlundered(iPlunderDuration);
						}
					}

					switch(eYield) {
						case YIELD_PRODUCTION:
							iPlunder /= 2;
							GET_PLAYER(getOwnerINLINE()).changeGold(iPlunder);							
							szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_PRODUCTION_FROM_IMP", iPlunder, GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
							if (pPlot->isOwned())
							{
								int iReduction = 0;
								BuildingTypes eBuilding = pPlot->getWorkingCity()->getProductionBuilding();
								UnitTypes eUnit = pPlot->getWorkingCity()->getProductionUnit();
								if(eBuilding != NO_BUILDING) {
									iReduction = pPlot->getWorkingCity()->getBuildingProduction(eBuilding) / 2;
									pPlot->getWorkingCity()->changeBuildingProduction(eBuilding, -iReduction);
								}
								else if(eUnit != NO_UNIT) {
									iReduction = pPlot->getWorkingCity()->getUnitProduction(eUnit) / 2;
									pPlot->getWorkingCity()->changeUnitProduction(eUnit, -iReduction);
								}
								if(iReduction > 0) {
									szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_PRODUCTION_FROM_IMP_ENEMY", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), getNameKey(), pPlot->getWorkingCity()->getNameKey(), iReduction);
									gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
								}
							}
							break;
						case YIELD_FOOD:
							iPlunder /= 2;
							GET_PLAYER(getOwnerINLINE()).addFoodToCities(iPlunder);
							szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_FOOD_FROM_IMP", iPlunder, GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
							break;
						case YIELD_COMMERCE:
							GET_PLAYER(getOwnerINLINE()).changeGold(iPlunder);							
							szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_GOLD_FROM_IMP", iPlunder, GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
							if (pPlot->isOwned())
							{
								int iPlundered = iPlunder / 5;
								iPlundered = std::min(iPlundered, GET_PLAYER(pPlot->getOwnerINLINE()).getGold());
								GET_PLAYER(pPlot->getOwnerINLINE()).changeGold(-iPlundered);
								szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_GOLD_FROM_IMP_ENEMY", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), iPlundered, getNameKey());
								gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
							}
							break;
						case YIELD_LUMBER:
						case YIELD_STONE:
						case YIELD_HERB:
						case YIELD_LEATHER:
						case YIELD_METAL:
							GET_PLAYER(getOwnerINLINE()).changeGlobalYield(eYield, iPlunder);
							szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_GY_FROM_IMP", iPlunder, GC.getYieldInfo(eYield).getDescription(), GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide());
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
							if (pPlot->isOwned())
							{
								int iPlundered = iPlunder / 5;
								iPlundered = std::min(iPlundered, GET_PLAYER(pPlot->getOwnerINLINE()).getGlobalYield(eYield));
								GET_PLAYER(pPlot->getOwnerINLINE()).changeGlobalYield(eYield, -iPlundered);
								szBuffer = gDLL->getText("TXT_KEY_MISC_PLUNDERED_GY_FROM_IMP_ENEMY", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), iPlundered, getNameKey(), GC.getYieldInfo(eYield).getDescription());
								gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
							}
							break;
						default:
							break;

					}
	/**
					if (pPlot->isOwned())
					{
						szBuffer = gDLL->getText("TXT_KEY_MISC_IMP_DESTROYED", GC.getImprovementInfo(pPlot->getImprovementType()).getTextKeyWide(), getNameKey(), getVisualCivAdjective(pPlot->getTeam()));
						gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
					}
	**/
				}
			}
		}

		if(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage() != NO_IMPROVEMENT) {
			pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));
		}
		else if(pPlot->getOwnerINLINE() == getOwnerINLINE()) {
			pPlot->setImprovementType(NO_IMPROVEMENT);
		}
	}
	else if (pPlot->isRoute())
	{
		eTempRoute = pPlot->getRouteType();
		pPlot->setRouteType(NO_ROUTE, true); // XXX downgrade rail???
	}

	changeMoves(GC.getMOVE_DENOMINATOR());

	if (pPlot->isActiveVisible(false))
	{
		// Pillage entity mission
		CvMissionDefinition kDefiniton;
		kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_PILLAGE).getTime() * gDLL->getSecsPerTurn());
		kDefiniton.setMissionType(MISSION_PILLAGE);
		kDefiniton.setPlot(pPlot);
		kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, NULL);
		gDLL->getEntityIFace()->AddMission(&kDefiniton);
	}

	if (eTempImprovement != NO_IMPROVEMENT || eTempRoute != NO_ROUTE)
	{
		CvEventReporter::getInstance().unitPillage(this, eTempImprovement, eTempRoute, getOwnerINLINE());
/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bImprovementPillaged                                                                    **/
/*************************************************************************************************/
		if(getTeam() != plot()->getTeam())
		{
			for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
			{
				if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
				{
					CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
					for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
					{
						if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
						{
							if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isImprovementPillaged())
							{
								GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
							}
						}
					}
				}
			}
		}
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/

	}

	if(pPlot->isOwned() && getTeam()!=pPlot->getTeam() && GET_TEAM(getTeam()).isAtWar(pPlot->getTeam()))
	{
		GET_TEAM(pPlot->getTeam()).AI_changeWarValueAccumulated(getTeam(), -20);
	}

	return true;
}


bool CvUnit::canPlunder(const CvPlot* pPlot, bool bTestVisible) const
{
	if (getDomainType() != DOMAIN_SEA)
	{
		return false;
	}

//	if (!(m_pUnitInfo->isPillage()))
	if (m_pUnitInfo->isNoPillage())
	{
		return false;
	}

	if (!pPlot->isWater())
	{
		return false;
	}

	if (pPlot->isFreshWater())
	{
		return false;
	}

	if (!pPlot->isValidDomainForAction(*this))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (pPlot->getTeam() == getTeam())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::plunder()
{
	CvPlot* pPlot = plot();

	if (!canPlunder(pPlot))
	{
		return false;
	}

	setBlockading(true);

	finishMoves();

	return true;
}


void CvUnit::updatePlunder(int iChange, bool bUpdatePlotGroups)
{
	int iBlockadeRange = GC.getDefineINT("SHIP_BLOCKADE_RANGE");

	bool bOldTradeNet;
	bool bChanged = false;

	for (int iTeam = 0; iTeam < MAX_TEAMS; ++iTeam)
	{
		if (isEnemy((TeamTypes)iTeam))
		{
			for (int i = -iBlockadeRange; i <= iBlockadeRange; ++i)
			{
				for (int j = -iBlockadeRange; j <= iBlockadeRange; ++j)
				{
					CvPlot* pLoopPlot = ::plotXY(getX_INLINE(), getY_INLINE(), i, j);

					if (NULL != pLoopPlot && pLoopPlot->isWater() && pLoopPlot->area() == area())
					{
						if (!bChanged)
						{
							bOldTradeNet = pLoopPlot->isTradeNetwork((TeamTypes)iTeam);
						}

						pLoopPlot->changeBlockadedCount((TeamTypes)iTeam, iChange);

						if (!bChanged)
						{
							bChanged = (bOldTradeNet != pLoopPlot->isTradeNetwork((TeamTypes)iTeam));
						}
					}
				}
			}
		}
	}

	if (bChanged)
	{
		gDLL->getInterfaceIFace()->setDirty(BlockadedPlots_DIRTY_BIT, true);

		if (bUpdatePlotGroups)
		{
			GC.getGameINLINE().updatePlotGroups();
		}
	}
}


int CvUnit::sabotageCost(const CvPlot* pPlot) const
{
	return GC.getDefineINT("BASE_SPY_SABOTAGE_COST");
}


// XXX compare with destroy prob...
int CvUnit::sabotageProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle) const
{
	CvPlot* pLoopPlot;
	int iDefenseCount;
	int iCounterSpyCount;
	int iProb;
	int iI;

	iProb = 0; // XXX

	if (pPlot->isOwned())
	{
		iDefenseCount = pPlot->plotCount(PUF_canDefend, -1, -1, NO_PLAYER, pPlot->getTeam());
		iCounterSpyCount = pPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());

		for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

			if (pLoopPlot != NULL)
			{
				iCounterSpyCount += pLoopPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());
			}
		}
	}
	else
	{
		iDefenseCount = 0;
		iCounterSpyCount = 0;
	}

	if (eProbStyle == PROBABILITY_HIGH)
	{
		iCounterSpyCount = 0;
	}

	iProb += (40 / (iDefenseCount + 1)); // XXX

	if (eProbStyle != PROBABILITY_LOW)
	{
		iProb += (50 / (iCounterSpyCount + 1)); // XXX
	}

	return iProb;
}


bool CvUnit::canSabotage(const CvPlot* pPlot, bool bTestVisible) const
{
	if (!(m_pUnitInfo->isSabotage()))
	{
		return false;
	}

	if (pPlot->getTeam() == getTeam())
	{
		return false;
	}

	if (pPlot->isCity())
	{
		return false;
	}

	if (pPlot->getImprovementType() == NO_IMPROVEMENT)
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < sabotageCost(pPlot))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::sabotage()
{
	CvCity* pNearestCity;
	CvPlot* pPlot;
	CvWString szBuffer;
	bool bCaught;

	if (!canSabotage(plot()))
	{
		return false;
	}

	pPlot = plot();

	bCaught = (GC.getGameINLINE().getSorenRandNum(100, "Spy: Sabotage") > sabotageProb(pPlot));

	GET_PLAYER(getOwnerINLINE()).changeGold(-(sabotageCost(pPlot)));

	if (!bCaught)
	{
		pPlot->setImprovementType((ImprovementTypes)(GC.getImprovementInfo(pPlot->getImprovementType()).getImprovementPillage()));

		finishMoves();

		pNearestCity = GC.getMapINLINE().findCity(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pPlot->getOwnerINLINE(), NO_TEAM, false);

		if (pNearestCity != NULL)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_SABOTAGED", getNameKey(), pNearestCity->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_SABOTAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			if (pPlot->isOwned())
			{
				szBuffer = gDLL->getText("TXT_KEY_MISC_SABOTAGE_NEAR", pNearestCity->getNameKey());
				gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_SABOTAGE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);
			}
		}

		if (pPlot->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SABOTAGE);
		}
	}
	else
	{
		if (pPlot->isOwned())
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_CAUGHT_AND_KILLED", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjective(), getNameKey());
			gDLL->getInterfaceIFace()->addMessage(pPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO);
		}

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_SPY_CAUGHT", getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SURRENDER);
		}

		if (pPlot->isOwned())
		{
			if (!isEnemy(pPlot->getTeam(), pPlot))
			{
				GET_PLAYER(pPlot->getOwnerINLINE()).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
			}
		}

		kill(true, pPlot->getOwnerINLINE());
	}

	return true;
}


int CvUnit::destroyCost(const CvPlot* pPlot) const
{
	CvCity* pCity;
	bool bLimited;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	bLimited = false;

	if (pCity->isProductionUnit())
	{
		bLimited = isLimitedUnitClass((UnitClassTypes)(GC.getUnitInfo(pCity->getProductionUnit()).getUnitClassType()));
	}
	else if (pCity->isProductionBuilding())
	{
		bLimited = isLimitedWonderClass((BuildingClassTypes)(GC.getBuildingInfo(pCity->getProductionBuilding()).getBuildingClassType()));
	}
	else if (pCity->isProductionProject())
	{
		bLimited = isLimitedProject(pCity->getProductionProject());
	}

	return (GC.getDefineINT("BASE_SPY_DESTROY_COST") + (pCity->getProduction() * ((bLimited) ? GC.getDefineINT("SPY_DESTROY_COST_MULTIPLIER_LIMITED") : GC.getDefineINT("SPY_DESTROY_COST_MULTIPLIER"))));
}


int CvUnit::destroyProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle) const
{
	CvCity* pCity;
	CvPlot* pLoopPlot;
	int iDefenseCount;
	int iCounterSpyCount;
	int iProb;
	int iI;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iProb = 0; // XXX

	iDefenseCount = pPlot->plotCount(PUF_canDefend, -1, -1, NO_PLAYER, pPlot->getTeam());

	iCounterSpyCount = pPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			iCounterSpyCount += pLoopPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());
		}
	}

	if (eProbStyle == PROBABILITY_HIGH)
	{
		iCounterSpyCount = 0;
	}

	iProb += (25 / (iDefenseCount + 1)); // XXX

	if (eProbStyle != PROBABILITY_LOW)
	{
		iProb += (50 / (iCounterSpyCount + 1)); // XXX
	}

	iProb += std::min(25, pCity->getProductionTurnsLeft()); // XXX

	return iProb;
}


bool CvUnit::canDestroy(const CvPlot* pPlot, bool bTestVisible) const
{
	CvCity* pCity;

	if (!(m_pUnitInfo->isDestroy()))
	{
		return false;
	}

	if (pPlot->getTeam() == getTeam())
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getProduction() == 0)
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < destroyCost(pPlot))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::destroy()
{
	CvCity* pCity;
	CvWString szBuffer;
	bool bCaught;

	if (!canDestroy(plot()))
	{
		return false;
	}

	bCaught = (GC.getGameINLINE().getSorenRandNum(100, "Spy: Destroy") > destroyProb(plot()));

	pCity = plot()->getPlotCity();
	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	GET_PLAYER(getOwnerINLINE()).changeGold(-(destroyCost(plot())));

	if (!bCaught)
	{
		pCity->setProduction(pCity->getProduction() / 2);

		finishMoves();

		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_DESTROYED_PRODUCTION", getNameKey(), pCity->getProductionNameKey(), pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DESTROY", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());

		szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_PRODUCTION_DESTROYED", pCity->getProductionNameKey(), pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DESTROY", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_DESTROY);
		}
	}
	else
	{
		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_CAUGHT_AND_KILLED", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjective(), getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_SPY_CAUGHT", getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SURRENDER);
		}

		if (!isEnemy(pCity->getTeam()))
		{
			GET_PLAYER(pCity->getOwnerINLINE()).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
		}

		kill(true, pCity->getOwnerINLINE());
	}

	return true;
}


int CvUnit::stealPlansCost(const CvPlot* pPlot) const
{
	CvCity* pCity;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	return (GC.getDefineINT("BASE_SPY_STEAL_PLANS_COST") + ((GET_TEAM(pCity->getTeam()).getTotalLand() + GET_TEAM(pCity->getTeam()).getTotalPopulation()) * GC.getDefineINT("SPY_STEAL_PLANS_COST_MULTIPLIER")));
}


// XXX compare with destroy prob...
int CvUnit::stealPlansProb(const CvPlot* pPlot, ProbabilityTypes eProbStyle) const
{
	CvCity* pCity;
	CvPlot* pLoopPlot;
	int iDefenseCount;
	int iCounterSpyCount;
	int iProb;
	int iI;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iProb = ((pCity->isGovernmentCenter()) ? 20 : 0); // XXX

	iDefenseCount = pPlot->plotCount(PUF_canDefend, -1, -1, NO_PLAYER, pPlot->getTeam());

	iCounterSpyCount = pPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());

	for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		pLoopPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			iCounterSpyCount += pLoopPlot->plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, pPlot->getTeam());
		}
	}

	if (eProbStyle == PROBABILITY_HIGH)
	{
		iCounterSpyCount = 0;
	}

	iProb += (20 / (iDefenseCount + 1)); // XXX

	if (eProbStyle != PROBABILITY_LOW)
	{
		iProb += (50 / (iCounterSpyCount + 1)); // XXX
	}

	return iProb;
}


bool CvUnit::canStealPlans(const CvPlot* pPlot, bool bTestVisible) const
{
	CvCity* pCity;

	if (!(m_pUnitInfo->isStealPlans()))
	{
		return false;
	}

	if (pPlot->getTeam() == getTeam())
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < stealPlansCost(pPlot))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::stealPlans()
{
	CvCity* pCity;
	CvWString szBuffer;
	bool bCaught;

	if (!canStealPlans(plot()))
	{
		return false;
	}

	bCaught = (GC.getGameINLINE().getSorenRandNum(100, "Spy: Steal Plans") > stealPlansProb(plot()));

	pCity = plot()->getPlotCity();
	FAssertMsg(pCity != NULL, "City is not assigned a valid value");

	GET_PLAYER(getOwnerINLINE()).changeGold(-(stealPlansCost(plot())));

	if (!bCaught)
	{
		GET_TEAM(getTeam()).changeStolenVisibilityTimer(pCity->getTeam(), 2);

		finishMoves();

		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_STOLE_PLANS", getNameKey(), pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_STEALPLANS", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pCity->getX_INLINE(), pCity->getY_INLINE());

		szBuffer = gDLL->getText("TXT_KEY_MISC_PLANS_STOLEN", pCity->getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_STEALPLANS", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE(), true, true);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_STEAL_PLANS);
		}
	}
	else
	{
		szBuffer = gDLL->getText("TXT_KEY_MISC_SPY_CAUGHT_AND_KILLED", GET_PLAYER(getOwnerINLINE()).getCivilizationAdjective(), getNameKey());
		gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_SPY_CAUGHT", getNameKey());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO);

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_SURRENDER);
		}

		if (!isEnemy(pCity->getTeam()))
		{
			GET_PLAYER(pCity->getOwnerINLINE()).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
		}

		kill(true, pCity->getOwnerINLINE());
	}

	return true;
}


bool CvUnit::canFound(const CvPlot* pPlot, bool bTestVisible) const
{
	if (!isFound())
	{
		return false;
	}

	if (!(GET_PLAYER(getOwnerINLINE()).canFound(pPlot->getX_INLINE(), pPlot->getY_INLINE(), bTestVisible)))
	{
		return false;
	}

//FfH: Added by Kael 11/10/2008
    if (pPlot->isFoundDisabled())
    {
        return false;
    }
    if (isHasCasted())
    {
        return false;
    }
    if (GC.getDefineINT("BONUS_MANA") != -1)
    {
        if (pPlot->getBonusType() == GC.getDefineINT("BONUS_MANA"))
        {
            return false;
        }
    }
	if (pPlot->getImprovementType() != NULL && pPlot->getImprovementType() != NO_IMPROVEMENT) // SayWhatEh Don't let the AI think he can settle on permanent improvements when he cant
	{
		if (GC.getImprovementInfo(pPlot->getImprovementType()).isPermanent())
		{
			//SpyFanatic: allow destroy of mana shrine, witch hut and ancient tower for human player
			if(!isHuman() || (pPlot->getImprovementType() != (ImprovementTypes)GC.getIMPROVEMENT_MANA_SHRINE() && pPlot->getImprovementType() != (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WITCH_HUT") && pPlot->getImprovementType() != (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_TOWER") && pPlot->getImprovementType() != (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_LOST_TEMPLE")))
			{
				return false;
			}
		}
	}
//FfH: End Add

	return true;
}


bool CvUnit::found()
{
	if (!canFound(plot()))
	{
		return false;
	}

	if (GC.getGameINLINE().getActivePlayer() == getOwnerINLINE())
	{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		gDLL->getInterfaceIFace()->lookAt(plot()->getPoint(), CAMERALOOKAT_NORMAL);
		//GC.getCurrentViewport().bringIntoView(getX_INLINE(), getY_INLINE());
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	}

	GET_PLAYER(getOwnerINLINE()).found(getX_INLINE(), getY_INLINE());

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_FOUND);
	}
/*************************************************************************************************/
/**	BETTER AI (New UNITAI) Sephi                                 					            **/
/*************************************************************************************************/
	if(!isHuman() && getAIGroup())
	{
		for (CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode(); pUnitNode != NULL; pUnitNode = plot()->nextUnitNode(pUnitNode))
		{
			CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
			if(pLoopUnit!=this && pLoopUnit->getAIGroup()==getAIGroup())
			{
				pLoopUnit->AIGroup_Reserve_join(getArea());
			}
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	kill(true);

	return true;
}


bool CvUnit::canSpread(const CvPlot* pPlot, ReligionTypes eReligion, bool bTestVisible) const
{
	CvCity* pCity;

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       08/19/09                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
/* orginal bts code
	if (GC.getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());
		argsList.add(getID());
		argsList.add((int) eReligion);
		argsList.add(pPlot->getX());
		argsList.add(pPlot->getY());
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSpreadReligion", argsList.makeFunctionArgs(), &lResult);
		if (lResult > 0)
		{
			return false;
		}
	}
*/
				// UP efficiency: Moved below faster calls
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

	if (eReligion == NO_RELIGION)
	{
		return false;
	}

	if (m_pUnitInfo->getReligionSpreads(eReligion) <= 0)
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->isHasReligion(eReligion))
	{
		return false;
	}

/*************************************************************************************************/
/**	ADDON (Block Religion for some Civs) Sephi                                                          	**/
/*************************************************************************************************/
	if(!pCity->canHaveReligion(eReligion))
	{
		return false;
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (!canEnterArea(pPlot->getTeam(), pPlot->area()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (pCity->getTeam() != getTeam())
		{
			if (GET_PLAYER(pCity->getOwnerINLINE()).isNoNonStateReligionSpread())
			{
				if (eReligion != GET_PLAYER(pCity->getOwnerINLINE()).getStateReligion())
				{
					return false;
				}
			}
		}
	}

/************************************************************************************************/
/* UNOFFICIAL_PATCH                       08/19/09                                jdog5000      */
/*                                                                                              */
/* Efficiency                                                                                   */
/************************************************************************************************/
	if (GC.getUSE_USE_CANNOT_SPREAD_RELIGION_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getOwnerINLINE());
		argsList.add(getID());
		argsList.add((int) eReligion);
		argsList.add(pPlot->getX());
		argsList.add(pPlot->getY());
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "cannotSpreadReligion", argsList.makeFunctionArgs(), &lResult);
		if (lResult > 0)
		{
			return false;
		}
	}
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/


	return true;
}


bool CvUnit::spread(ReligionTypes eReligion)
{
	CvCity* pCity;
	CvWString szBuffer;
	int iSpreadProb;

	if (!canSpread(plot(), eReligion))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		iSpreadProb = m_pUnitInfo->getReligionSpreads(eReligion);

		if (pCity->getTeam() != getTeam())
		{
			iSpreadProb /= 2;
		}

		bool bSuccess;

		iSpreadProb += (((GC.getNumReligionInfos() - pCity->getReligionCount()) * (100 - iSpreadProb)) / GC.getNumReligionInfos());

		if (GC.getGameINLINE().getSorenRandNum(100, "Unit Spread Religion") < iSpreadProb)
		{
			pCity->setHasReligion(eReligion, true, true, false);

			CvPlayer& kOwner = GET_PLAYER(getOwnerINLINE());
/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bSpreadReligion                                                                         **/
/*************************************************************************************************/
            UnitTypes eUnit=getUnitType();
            for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
            {
                if(kOwner.isAdventureEnabled(iAdvLoop) && !kOwner.isAdventureFinished(iAdvLoop))
                {
                    CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
                    for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
                    {
                        if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
                        {
                            if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isSpreadReligion())
                            {
                                if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getReligionType()==eReligion)
                                {
                                    kOwner.changeAdventureCounter(iAdvLoop,iJ,1);
                                }
                            }
                        }
                    }
                }
            }
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/

			bSuccess = true;
		}
		else
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_RELIGION_FAILED_TO_SPREAD", getNameKey(), GC.getReligionInfo(eReligion).getChar(), pCity->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NOSPREAD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE());
			bSuccess = false;
		}

		// Python Event
		CvEventReporter::getInstance().unitSpreadReligionAttempt(this, eReligion, bSuccess);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_SPREAD);
	}

	kill(true);

	return true;
}


bool CvUnit::canSpreadCorporation(const CvPlot* pPlot, CorporationTypes eCorporation, bool bTestVisible) const
{
	if (NO_CORPORATION == eCorporation)
	{
		return false;
	}

	if (!GET_PLAYER(getOwnerINLINE()).isActiveCorporation(eCorporation))
	{
		return false;
	}

	if (m_pUnitInfo->getCorporationSpreads(eCorporation) <= 0)
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();

	if (NULL == pCity)
	{
		return false;
	}

	if (pCity->isHasCorporation(eCorporation))
	{
		return false;
	}

	if (!canEnterArea(pPlot->getTeam(), pPlot->area()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (!GET_PLAYER(pCity->getOwnerINLINE()).isActiveCorporation(eCorporation))
		{
			return false;
		}

		for (int iCorporation = 0; iCorporation < GC.getNumCorporationInfos(); ++iCorporation)
		{
			if (pCity->isHeadquarters((CorporationTypes)iCorporation))
			{
				if (GC.getGameINLINE().isCompetingCorporation((CorporationTypes)iCorporation, eCorporation))
				{
					return false;
				}
			}
		}

		bool bValid = false;

//FfH: Added by Kael 10/21/2007 (So that corporations can be made to not require bonuses)
        if (GC.getCorporationInfo(eCorporation).getPrereqBonus(0) == NO_BONUS)
        {
            bValid = true;
        }
//FfH: End Add

		for (int i = 0; i < GC.getNUM_CORPORATION_PREREQ_BONUSES(); ++i)
		{
			BonusTypes eBonus = (BonusTypes)GC.getCorporationInfo(eCorporation).getPrereqBonus(i);
			if (NO_BONUS != eBonus)
			{
				if (pCity->hasBonus(eBonus))
				{
					bValid = true;
					break;
				}
			}
		}

		if (!bValid)
		{
			return false;
		}

		if (GET_PLAYER(getOwnerINLINE()).getGold() < spreadCorporationCost(eCorporation, pCity))
		{
			return false;
		}
	}

	return true;
}

int CvUnit::spreadCorporationCost(CorporationTypes eCorporation, CvCity* pCity) const
{
	int iCost = std::max(0, GC.getCorporationInfo(eCorporation).getSpreadCost() * (100 + GET_PLAYER(getOwnerINLINE()).calculateInflationRate()));
	iCost /= 100;

	if (NULL != pCity)
	{
		if (getTeam() != pCity->getTeam() && !GET_TEAM(pCity->getTeam()).isVassal(getTeam()))
		{
			iCost *= GC.getDefineINT("CORPORATION_FOREIGN_SPREAD_COST_PERCENT");
			iCost /= 100;
		}

		for (int iCorp = 0; iCorp < GC.getNumCorporationInfos(); ++iCorp)
		{
			if (iCorp != eCorporation)
			{
				if (pCity->isActiveCorporation((CorporationTypes)iCorp))
				{
					if (GC.getGameINLINE().isCompetingCorporation(eCorporation, (CorporationTypes)iCorp))
					{
						iCost *= 100 + GC.getCorporationInfo((CorporationTypes)iCorp).getSpreadFactor();
						iCost /= 100;
					}
				}
			}
		}
	}

	return iCost;
}

bool CvUnit::spreadCorporation(CorporationTypes eCorporation)
{
	int iSpreadProb;

	if (!canSpreadCorporation(plot(), eCorporation))
	{
		return false;
	}

	CvCity* pCity = plot()->getPlotCity();

	if (NULL != pCity)
	{
		GET_PLAYER(getOwnerINLINE()).changeGold(-spreadCorporationCost(eCorporation, pCity));

		iSpreadProb = m_pUnitInfo->getCorporationSpreads(eCorporation);

		if (pCity->getTeam() != getTeam())
		{
			iSpreadProb /= 2;
		}

		iSpreadProb += (((GC.getNumCorporationInfos() - pCity->getCorporationCount()) * (100 - iSpreadProb)) / GC.getNumCorporationInfos());

		if (GC.getGameINLINE().getSorenRandNum(100, "Unit Spread Corporation") < iSpreadProb)
		{
			pCity->setHasCorporation(eCorporation, true, true, false);
/*************************************************************************************************/
/**	ADDON (Houses of Erebus) Sephi			                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
            GET_PLAYER(getOwnerINLINE()).changeCorporationSupport(eCorporation,300);
            GET_PLAYER(pCity->getOwnerINLINE()).changeCorporationSupport(eCorporation,300);
            GET_PLAYER(pCity->getOwnerINLINE()).changeCorporationSupportMultiplier(eCorporation,4);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
		}
		else
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_CORPORATION_FAILED_TO_SPREAD", getNameKey(), GC.getCorporationInfo(eCorporation).getChar(), pCity->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_NOSPREAD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE());
		}
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_SPREAD_CORPORATION);
	}

	kill(true);

	return true;
}


bool CvUnit::canJoin(const CvPlot* pPlot, SpecialistTypes eSpecialist) const
{
	CvCity* pCity;

	if (eSpecialist == NO_SPECIALIST)
	{
		return false;
	}

	if (!(m_pUnitInfo->getGreatPeoples(eSpecialist)))
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (!(pCity->canJoin()))
	{
		return false;
	}

	if (pCity->getTeam() != getTeam())
	{
		return false;
	}

	if (isDelayedDeath())
	{
		return false;
	}

//FfH: Added by Kael 08/18/2008
    if (isHasCasted())
    {
        return false;
    }
//FfH: End Add

	return true;
}


bool CvUnit::join(SpecialistTypes eSpecialist)
{
	CvCity* pCity;

	if (!canJoin(plot(), eSpecialist))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->changeFreeSpecialistCount(eSpecialist, 1);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_JOIN);
	}

	kill(true);

	return true;
}


bool CvUnit::canConstruct(const CvPlot* pPlot, BuildingTypes eBuilding, bool bTestVisible) const
{
	CvCity* pCity;

	if (eBuilding == NO_BUILDING)
	{
		return false;
	}

//FfH: Added by Kael 08/18/2008
    if (isHasCasted())
    {
        return false;
    }
//FfH: End Add

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (getTeam() != pCity->getTeam())
	{
		return false;
	}

	if (pCity->getNumRealBuilding(eBuilding) > 0)
	{
		return false;
	}

	if (!(m_pUnitInfo->getForceBuildings(eBuilding)))
	{
		if (!(m_pUnitInfo->getBuildings(eBuilding)))
		{
			return false;
		}

		if (!(pCity->canConstruct(eBuilding, false, bTestVisible, true)))
		{
			return false;
		}
	}

	if (isDelayedDeath())
	{
		return false;
	}

	return true;
}


bool CvUnit::construct(BuildingTypes eBuilding)
{
	CvCity* pCity;

	if (!canConstruct(plot(), eBuilding))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->setNumRealBuilding(eBuilding, pCity->getNumRealBuilding(eBuilding) + 1);

		CvEventReporter::getInstance().buildingBuilt(pCity, eBuilding);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_CONSTRUCT);
	}

	kill(true);

	return true;
}


TechTypes CvUnit::getDiscoveryTech() const
{
	return ::getDiscoveryTech(getUnitType(), getOwnerINLINE());
}


int CvUnit::getDiscoverResearch(TechTypes eTech) const
{
	int iResearch;

	iResearch = (m_pUnitInfo->getBaseDiscover() + (m_pUnitInfo->getDiscoverMultiplier() * GET_TEAM(getTeam()).getTotalPopulation()));

	iResearch *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitDiscoverPercent();
	iResearch /= 100;

    if (eTech != NO_TECH)
    {
        iResearch = std::min(GET_TEAM(getTeam()).getResearchLeft(eTech), iResearch);
    }

//FfH: Added by Kael 08/18/2008
    if (isHasCasted())
    {
        return 0;
    }
//FfH: End Add

	return std::max(0, iResearch);
}


bool CvUnit::canDiscover(const CvPlot* pPlot) const
{
	TechTypes eTech;

	eTech = getDiscoveryTech();

	if (eTech == NO_TECH)
	{
		return false;
	}

	if (getDiscoverResearch(eTech) == 0)
	{
		return false;
	}

	if (isDelayedDeath())
	{
		return false;
	}

	return true;
}


bool CvUnit::discover()
{
	TechTypes eDiscoveryTech;

	if (!canDiscover(plot()))
	{
		return false;
	}

	eDiscoveryTech = getDiscoveryTech();
	FAssertMsg(eDiscoveryTech != NO_TECH, "DiscoveryTech is not assigned a valid value");

	GET_TEAM(getTeam()).changeResearchProgress(eDiscoveryTech, getDiscoverResearch(eDiscoveryTech), getOwnerINLINE());

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_DISCOVER);
	}

	kill(true);

	return true;
}


int CvUnit::getMaxHurryProduction(CvCity* pCity) const
{
	int iProduction;

	iProduction = (m_pUnitInfo->getBaseHurry() + (m_pUnitInfo->getHurryMultiplier() * pCity->getPopulation()));

	iProduction *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitHurryPercent();
	iProduction /= 100;

//FfH: Added by Kael 08/18/2008
    if (isHasCasted())
    {
        return 0;
    }
//FfH: End Add

	return std::max(0, iProduction);
}


int CvUnit::getHurryProduction(const CvPlot* pPlot) const
{
	CvCity* pCity;
	int iProduction;

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iProduction = getMaxHurryProduction(pCity);

	iProduction = std::min(pCity->productionLeft(), iProduction);

	return std::max(0, iProduction);
}


bool CvUnit::canHurry(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	CvCity* pCity;

	if (getHurryProduction(pPlot) == 0)
	{
		return false;
	}

	pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getProductionTurnsLeft() == 1)
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (!(pCity->isProductionBuilding()))
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::hurry()
{
	CvCity* pCity;

	if (!canHurry(plot()))
	{
		return false;
	}

	pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->changeProduction(getHurryProduction(plot()));
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_HURRY);
	}

	kill(true);

	return true;
}


int CvUnit::getTradeGold(const CvPlot* pPlot) const
{
	CvCity* pCapitalCity;
	CvCity* pCity;
	int iGold;

	pCity = pPlot->getPlotCity();
	pCapitalCity = GET_PLAYER(getOwnerINLINE()).getCapitalCity();

	if (pCity == NULL)
	{
		return 0;
	}

	iGold = (m_pUnitInfo->getBaseTrade() + (m_pUnitInfo->getTradeMultiplier() * ((pCapitalCity != NULL) ? pCity->calculateTradeProfit(pCapitalCity) : 0)));

	iGold *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitTradePercent();
	iGold /= 100;

//FfH: Added by Kael 08/18/2008
    if (isHasCasted())
    {
        return 0;
    }
//FfH: End Add

	return std::max(0, iGold);
}


bool CvUnit::canTrade(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (getTradeGold(pPlot) == 0)
	{
		return false;
	}

	if (!canEnterArea(pPlot->getTeam(), pPlot->area()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (pCity->getTeam() == getTeam())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::trade()
{
	if (!canTrade(plot()))
	{
		return false;
	}

	GET_PLAYER(getOwnerINLINE()).changeGold(getTradeGold(plot()));

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_TRADE);
	}

	kill(true);

	return true;
}


int CvUnit::getGreatWorkCulture(const CvPlot* pPlot) const
{
	int iCulture;

	iCulture = m_pUnitInfo->getGreatWorkCulture();

	iCulture *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent();
	iCulture /= 100;

	//cannot do GreatWork if Culture is too high
	if(pPlot->getPlotCity()!=NULL)
	{
		if(pPlot->getPlotCity()->getCulture(getOwnerINLINE())>iCulture)
		{
			return 0;
		}
	}
/** old FFH code
//FfH: Added by Kael 08/18/2008
    if (isHasCasted())
    {
        return 0;
    }
//FfH: End Add
**/
	return std::max(0, iCulture);
}


bool CvUnit::canGreatWork(const CvPlot* pPlot) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();

	if (pCity == NULL)
	{
		return false;
	}

	if (pCity->getOwnerINLINE() != getOwnerINLINE())
	{
		return false;
	}

	if (getGreatWorkCulture(pPlot) == 0)
	{
		return false;
	}

	return true;
}


bool CvUnit::greatWork()
{
	if (!canGreatWork(plot()))
	{
		return false;
	}

	CvCity* pCity = plot()->getPlotCity();

	if (pCity != NULL)
	{
		pCity->setCultureUpdateTimer(0);
		pCity->setOccupationTimer(0);

		int iCultureToAdd = 100 * getGreatWorkCulture(plot());
		int iNumTurnsApplied = (GC.getDefineINT("GREAT_WORKS_CULTURE_TURNS") * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent()) / 100;

		for (int i = 0; i < iNumTurnsApplied; ++i)
		{
			pCity->changeCultureTimes100(getOwnerINLINE(), iCultureToAdd / iNumTurnsApplied, true, true);
		}

		if (iNumTurnsApplied > 0)
		{
			pCity->changeCultureTimes100(getOwnerINLINE(), iCultureToAdd % iNumTurnsApplied, false, true);
		}
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_GREAT_WORK);
	}

	kill(true);

	return true;
}


int CvUnit::getEspionagePoints(const CvPlot* pPlot) const
{
	int iEspionagePoints;

	iEspionagePoints = m_pUnitInfo->getEspionagePoints();

	iEspionagePoints *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getUnitGreatWorkPercent();
	iEspionagePoints /= 100;

	return std::max(0, iEspionagePoints);
}

bool CvUnit::canInfiltrate(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		return false;
	}

	if (getEspionagePoints(NULL) == 0)
	{
		return false;
	}

	CvCity* pCity = pPlot->getPlotCity();
	if (pCity == NULL || pCity->isBarbarian())
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (NULL != pCity && pCity->getTeam() == getTeam())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::infiltrate()
{
	if (!canInfiltrate(plot()))
	{
		return false;
	}

	int iPoints = getEspionagePoints(NULL);
	GET_TEAM(getTeam()).changeEspionagePointsAgainstTeam(GET_PLAYER(plot()->getOwnerINLINE()).getTeam(), iPoints);
	GET_TEAM(getTeam()).changeEspionagePointsEver(iPoints);

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_INFILTRATE);
	}

	kill(true);

	return true;
}


bool CvUnit::canEspionage(const CvPlot* pPlot, bool bTestVisible) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (!isSpy())
	{
		return false;
	}

	if (GC.getGameINLINE().isOption(GAMEOPTION_NO_ESPIONAGE))
	{
		return false;
	}

	PlayerTypes ePlotOwner = pPlot->getOwnerINLINE();
	if (NO_PLAYER == ePlotOwner)
	{
		return false;
	}

	CvPlayer& kTarget = GET_PLAYER(ePlotOwner);

	if (kTarget.isBarbarian())
	{
		return false;
	}

	if (kTarget.getTeam() == getTeam())
	{
		return false;
	}

	if (GET_TEAM(getTeam()).isVassal(kTarget.getTeam()))
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (isMadeAttack())
		{
			return false;
		}

		if (hasMoved())
		{
			return false;
		}

		if (kTarget.getTeam() != getTeam() && !isInvisible(kTarget.getTeam(), false))
		{
			return false;
		}
	}

	return true;
}

bool CvUnit::espionage(EspionageMissionTypes eMission, int iData)
{
	if (!canEspionage(plot()))
	{
		return false;
	}

	PlayerTypes eTargetPlayer = plot()->getOwnerINLINE();

	if (NO_ESPIONAGEMISSION == eMission)
	{
		FAssert(GET_PLAYER(getOwnerINLINE()).isHuman());
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DOESPIONAGE);
		if (NULL != pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true);
		}
	}
	else if (GC.getEspionageMissionInfo(eMission).isTwoPhases() && -1 == iData)
	{
		FAssert(GET_PLAYER(getOwnerINLINE()).isHuman());
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DOESPIONAGE_TARGET);
		if (NULL != pInfo)
		{
			pInfo->setData1(eMission);
			gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true);
		}
	}
	else
	{
		if (testSpyIntercepted(eTargetPlayer, GC.getEspionageMissionInfo(eMission).getDifficultyMod()))
		{
			return false;
		}

		if (GET_PLAYER(getOwnerINLINE()).doEspionageMission(eMission, eTargetPlayer, plot(), iData, this))
		{
			if (plot()->isActiveVisible(false))
			{
				NotifyEntity(MISSION_ESPIONAGE);
			}

			if (!testSpyIntercepted(eTargetPlayer, GC.getDefineINT("ESPIONAGE_SPY_MISSION_ESCAPE_MOD")))
			{
				setFortifyTurns(0);
				setMadeAttack(true);
				finishMoves();

				CvCity* pCapital = GET_PLAYER(getOwnerINLINE()).getCapitalCity();
				if (NULL != pCapital)
				{
					setXY(pCapital->getX_INLINE(), pCapital->getY_INLINE(), false, false, false);

					CvWString szBuffer = gDLL->getText("TXT_KEY_ESPIONAGE_SPY_SUCCESS", getNameKey(), pCapital->getNameKey());
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_POSITIVE_DINK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pCapital->getX_INLINE(), pCapital->getY_INLINE(), true, true);
				}
			}

			return true;
		}
	}

	return false;
}

bool CvUnit::testSpyIntercepted(PlayerTypes eTargetPlayer, int iModifier)
{
	CvPlayer& kTargetPlayer = GET_PLAYER(eTargetPlayer);

	if (kTargetPlayer.isBarbarian())
	{
		return false;
	}

	if (GC.getGameINLINE().getSorenRandNum(10000, "Spy Interception") >= getSpyInterceptPercent(kTargetPlayer.getTeam()) * (100 + iModifier))
	{
		return false;
	}

	CvString szFormatNoReveal;
	CvString szFormatReveal;

	if (GET_TEAM(kTargetPlayer.getTeam()).getCounterespionageModAgainstTeam(getTeam()) > 0)
	{
		szFormatNoReveal = "TXT_KEY_SPY_INTERCEPTED_MISSION";
		szFormatReveal = "TXT_KEY_SPY_INTERCEPTED_MISSION_REVEAL";
	}
	else if (plot()->isEspionageCounterSpy(kTargetPlayer.getTeam()))
	{
		szFormatNoReveal = "TXT_KEY_SPY_INTERCEPTED_SPY";
		szFormatReveal = "TXT_KEY_SPY_INTERCEPTED_SPY_REVEAL";
	}
	else
	{
		szFormatNoReveal = "TXT_KEY_SPY_INTERCEPTED";
		szFormatReveal = "TXT_KEY_SPY_INTERCEPTED_REVEAL";
	}

	CvWString szCityName = kTargetPlayer.getCivilizationShortDescription();
	CvCity* pClosestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), eTargetPlayer, kTargetPlayer.getTeam(), true, false);
	if (pClosestCity != NULL)
	{
		szCityName = pClosestCity->getName();
	}

	CvWString szBuffer = gDLL->getText(szFormatReveal.GetCString(), GET_PLAYER(getOwnerINLINE()).getCivilizationAdjectiveKey(), getNameKey(), kTargetPlayer.getCivilizationAdjectiveKey(), szCityName.GetCString());
	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);

	if (GC.getGameINLINE().getSorenRandNum(100, "Spy Reveal identity") < GC.getDefineINT("ESPIONAGE_SPY_REVEAL_IDENTITY_PERCENT"))
	{
		if (!isEnemy(kTargetPlayer.getTeam()))
		{
			GET_PLAYER(eTargetPlayer).AI_changeMemoryCount(getOwnerINLINE(), MEMORY_SPY_CAUGHT, 1);
		}

		gDLL->getInterfaceIFace()->addMessage(eTargetPlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
	}
	else
	{
		szBuffer = gDLL->getText(szFormatNoReveal.GetCString(), getNameKey(), kTargetPlayer.getCivilizationAdjectiveKey(), szCityName.GetCString());
		gDLL->getInterfaceIFace()->addMessage(eTargetPlayer, true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_EXPOSE", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
	}

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_SURRENDER);
	}

	kill(true);

	return true;
}

int CvUnit::getSpyInterceptPercent(TeamTypes eTargetTeam) const
{
	FAssert(isSpy());
	FAssert(getTeam() != eTargetTeam);

	int iSuccess = 0;

	int iTargetPoints = GET_TEAM(eTargetTeam).getEspionagePointsEver();
	int iOurPoints = GET_TEAM(getTeam()).getEspionagePointsEver();
	iSuccess += (GC.getDefineINT("ESPIONAGE_INTERCEPT_SPENDING_MAX") * iTargetPoints) / std::max(1, iTargetPoints + iOurPoints);

	if (plot()->isEspionageCounterSpy(eTargetTeam))
	{
		iSuccess += GC.getDefineINT("ESPIONAGE_INTERCEPT_COUNTERSPY");
	}

	if (GET_TEAM(eTargetTeam).getCounterespionageModAgainstTeam(getTeam()) > 0)
	{
		iSuccess += GC.getDefineINT("ESPIONAGE_INTERCEPT_COUNTERESPIONAGE_MISSION");
	}

	if (0 == getFortifyTurns() || plot()->plotCount(PUF_isSpy, -1, -1, NO_PLAYER, getTeam()) > 1)
	{
		iSuccess += GC.getDefineINT("ESPIONAGE_INTERCEPT_RECENT_MISSION");
	}

	return std::min(100, std::max(0, iSuccess));
}

bool CvUnit::isIntruding() const
{
	TeamTypes eLocalTeam = plot()->getTeam();

	if (NO_TEAM == eLocalTeam || eLocalTeam == getTeam())
	{
		return false;
	}

	if (GET_TEAM(eLocalTeam).isVassal(getTeam()))
	{
		return false;
	}

	return true;
}

bool CvUnit::canGoldenAge(const CvPlot* pPlot, bool bTestVisible) const
{
	if (!isGoldenAge())
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).unitsRequiredForGoldenAge() > GET_PLAYER(getOwnerINLINE()).unitsGoldenAgeReady())
		{
			return false;
		}
	}

	return true;
}


bool CvUnit::goldenAge()
{
	if (!canGoldenAge(plot()))
	{
		return false;
	}

	GET_PLAYER(getOwnerINLINE()).killGoldenAgeUnits(this);

	GET_PLAYER(getOwnerINLINE()).changeGoldenAgeTurns(GET_PLAYER(getOwnerINLINE()).getGoldenAgeLength());
	GET_PLAYER(getOwnerINLINE()).changeNumUnitGoldenAges(1);

	if (plot()->isActiveVisible(false))
	{
		NotifyEntity(MISSION_GOLDEN_AGE);
	}

	kill(true);

	return true;
}


bool CvUnit::canBuild(CvPlot* pPlot, BuildTypes eBuild, bool bTestVisible) const
{
    FAssertMsg(eBuild < GC.getNumBuildInfos(), "Index out of bounds");

//FfH: Modified by Kael 08/31/2007 (so AI workers can upgrade mana nodes)
//	if (!(m_pUnitInfo->getBuilds(eBuild)))
//	{
//		return false;
//	}
//
//	if (!(GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false, bTestVisible)))
//	{
//		return false;
//	}
//
//	if (!pPlot->isValidDomainForAction(*this))
//	{
//		return false;
//	}
	if (!(GET_PLAYER(getOwnerINLINE()).canBuild(pPlot, eBuild, false, bTestVisible)))
	{
		return false;
	}
	if (!pPlot->isValidDomainForAction(*this))
	{
		return false;
	}
	/*if(isOOSLogging())
	{
		oosLog("AIWorker"
			,"Turn: %d,Player:%d,UnitID:%d,X:%d,Y:%d,canBuild:%S\n"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getOwner()
			,getID()
			,plot()->getX()
			,plot()->getY()
			,eBuild!=NO_IMPROVEMENT?GC.getBuildInfo(eBuild).getDescription():L"NO_IMPROVEMENT"
		);
	}*/
	if (!(m_pUnitInfo->getBuilds(eBuild)))
	{
		//Allow Adepts to build on MagicNodes

		if(GC.getBuildInfo(eBuild).getImprovement() == NO_IMPROVEMENT)
		{
			return false;
		}

		//No Adept?
//		if(!GC.getUnitInfo(getUnitType()).isCanUpgradeManaNodes()
		if((getUnitCombatType() != GC.getUNITCOMBAT_ADEPT())
			|| GC.getUnitInfo(getUnitType()).getWorkRate() == 0)
		{
			return false;
		}

		//No Mana Node?
		CvImprovementInfo &kImprovement = GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement());
		if(kImprovement.getBonusConvert() == NO_BONUS
			|| GC.getBonusInfo((BonusTypes)kImprovement.getBonusConvert()).getBonusClassType() != GC.getInfoTypeForString("BONUSCLASS_MANA"))
		{
			return false;
		}

		//No Bonus?
		if(pPlot->getBonusType() == NO_BONUS)
			return false;
		if((GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses((BonusTypes)kImprovement.getBonusConvert()) < 1)
			&& pPlot->getBonusType() != kImprovement.getBonusConvert())
			return false;
	}
//FfH: End Modify
/*************************************************************************************************/
/**	ADDON (stop automated workers from building forts) Sephi                                    **/
/*************************************************************************************************/
    if (isAutomated())
    {
        if (eBuild == GC.getDefineINT("BUILD_FORT"))
        {
			return false;
			/*
			//SpyFanatic: Allow AI to build fort at choke point
			if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
			{
				if(pPlot->getWorkingCity() != NULL)
				{
					return false; //Not inside City Plot
				}
				int iPlotCanalValue = pPlot->getCanalValue();
				int iPlotChokeValue = pPlot->getChokeValue();
				if(iPlotCanalValue == 0 && iPlotChokeValue == 0)
				{
					return false; //Not if does not act as canal or choke point
				}
				int iUniqueRange = GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement()).getUniqueRange();
				for (int iDX = -iUniqueRange; iDX <= iUniqueRange; iDX++)
				{
					for (int iDY = -iUniqueRange; iDY <= iUniqueRange; iDY++)
					{
						CvPlot *pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);
						if (pLoopPlot != NULL && pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
						{
							if (pPlot->getChokeValue() > iPlotChokeValue)
							{
								return false; //Not if there is a better Choke point nearby
							}
						}
					}
				}
			}
			else
			{
				return false;
			}
			*/
        }
    }

	if(pPlot->getImprovementType()!=NO_IMPROVEMENT)
	{
		if(isAutomated() && isHuman())
		{
			if(GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_SAFE_AUTOMATION))
			{
				return false;
			}
		}
	}

	if (isAutomated() || (!isHuman()))
	{
		if(pPlot->getImprovementType()!=NO_IMPROVEMENT && pPlot->getBonusType(getTeam())!=NO_BONUS)
		{
			if(GC.getBonusInfo(pPlot->getBonusType(getTeam())).getBonusClassType()==GC.getInfoTypeForString("BONUSCLASS_MANA"))
			{
				if(GC.getBuildInfo(eBuild).getRoute()==NO_ROUTE)
				{
					if(eBuild==NO_BUILD || GC.getBuildInfo(eBuild).getImprovement()==NO_IMPROVEMENT 
						|| GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement()).getBonusConvert()==NO_BONUS
						|| GC.getBonusInfo((BonusTypes)GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement()).getBonusConvert()).getBonusClassType()!=GC.getInfoTypeForString("BONUSCLASS_MANA"))
					{
						return false;
					}
				}
			}
		}
	}
/*************************************************************************************************/
/**	END                                                                  						**/
/*************************************************************************************************/


	return true;
}

// Returns true if build finished...
bool CvUnit::build(BuildTypes eBuild)
{
	bool bFinished;

	FAssertMsg(eBuild < GC.getNumBuildInfos(), "Invalid Build");

	if (!canBuild(plot(), eBuild))
	{
		return false;
	}

	// Note: notify entity must come before changeBuildProgress - because once the unit is done building,
	// that function will notify the entity to stop building.
	NotifyEntity((MissionTypes)GC.getBuildInfo(eBuild).getMissionType());

	GET_PLAYER(getOwnerINLINE()).changeGold(-(GET_PLAYER(getOwnerINLINE()).getBuildCost(plot(), eBuild)));

	//Improvement Yield Costs
	if(GC.getBuildInfo(eBuild).isApplyImprovementCost() 
		&& GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT) {

		GET_PLAYER(getOwnerINLINE()).changeGold(-(GET_PLAYER(getOwnerINLINE()).getImprovementGoldCost(plot(), eBuild)));
		for(int i = 0; i < NUM_YIELD_TYPES; ++i) {
			GET_PLAYER(getOwnerINLINE()).changeGlobalYield(i, -(GET_PLAYER(getOwnerINLINE()).getImprovementYieldCost(plot(), eBuild, (YieldTypes)i)));
		}
	}


	bFinished = plot()->changeBuildProgress(eBuild, workRate(false), getTeam());


	finishMoves(); // needs to be at bottom because movesLeft() can affect workRate()...

	if (bFinished)
	{
		/*if(isOOSLogging())
		{
			oosLog("AIWorker"
				,"Turn: %d,Player:%d,UnitID:%d,X:%d,Y:%d,Build:%S,Value:%d,Plot:%S\n"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,plot()->getX()
				,plot()->getY()
				,GC.getBuildInfo(eBuild).getImprovement()!=NO_IMPROVEMENT?GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement()).getDescription():L"NO_IMPROVEMENT"
				,plot()->getWorkingCity()!=NULL?static_cast<CvCityAI*>(plot()->getWorkingCity())->AI_PlotBuildValue(eBuild,plot()):-100
				,plot()->getBonusType()!=NO_BONUS?GC.getBonusInfo(plot()->getBonusType()).getDescription():L"NO_BONUS"
			);
		}*/
		// Super Forts begin *culture*
		if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
		{
			if(GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement()).getCulture() > 0 /*isActsAsCity()*/)
			{
				if(plot()->getOwner() == NO_PLAYER/* && getOwnerINLINE() < BARBARIAN_PLAYER*/)
				{
					plot()->setOwner(getOwnerINLINE(),true,true);
				}
			}
		}
		// Super Forts end

		if (GC.getBuildInfo(eBuild).isKill())
		{
			kill(true);
		}
	}

	// Python Event
	CvEventReporter::getInstance().unitBuildImprovement(this, eBuild, bFinished);

	return bFinished;
}


bool CvUnit::canPromote(PromotionTypes ePromotion, int iLeaderUnitId) const
{
	CvPromotionInfo& kPromotion=GC.getPromotionInfo(ePromotion);

	if (iLeaderUnitId >= 0)
	{
		if (iLeaderUnitId == getID())
		{
			return false;
		}

		// The command is always possible if it's coming from a Warlord unit that gives just experience points
		CvUnit* pWarlord = GET_PLAYER(getOwnerINLINE()).getUnit(iLeaderUnitId);
		if (pWarlord &&
			NO_UNIT != pWarlord->getUnitType() &&
			pWarlord->getUnitInfo().getLeaderExperience() > 0 &&
			NO_PROMOTION == pWarlord->getUnitInfo().getLeaderPromotion() &&
			canAcquirePromotionAny())
		{
			return true;
		}
	}

	if (ePromotion == NO_PROMOTION)
	{
		return false;
	}

/*************************************************************************************************/
/**	ADDON (Promotions can be unlocked by Gameoptions) Sephi                     					**/
/*************************************************************************************************/
	GameOptionTypes eGameOption = (GameOptionTypes)GC.getPromotionInfo(ePromotion).getRequiredGameOption();
	if (eGameOption!=NO_GAMEOPTION)
	{
		if (!GC.getGameINLINE().isOption(eGameOption))
		{
			return false;
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (Promotion dependancy on other Promotions) Sephi	                     				**/
/*************************************************************************************************/
    if (isDenyPromotion(ePromotion))
    {
        return false;
    }

    if (kPromotion.getNumPromotionMustHave() > 0)
	{
        for (int iJ = 0; iJ < kPromotion.getNumPromotionMustHave(); iJ++)
		{
            if(!isHasPromotion(kPromotion.getPromotionMustHave(iJ)))
            {
                return false;
            }
		}
	}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (!canAcquirePromotion(ePromotion))
	{
		return false;
	}

/*************************************************************************************************/
/**	ADDON (Gear Promotions ) Sephi							                     				**/
/*************************************************************************************************/
    if (kPromotion.isGear())
    {
        return true;
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

//FfH Units: Added by Kael 08/04/2007
    if (getFreePromotionPick() > 0  && getLevel()>3)
    {
        return true;
    }
//FfH: End Add

	if (GC.getPromotionInfo(ePromotion).isLeader())
	{
		if (iLeaderUnitId >= 0)
		{
			CvUnit* pWarlord = GET_PLAYER(getOwnerINLINE()).getUnit(iLeaderUnitId);
			if (pWarlord && NO_UNIT != pWarlord->getUnitType())
			{
				return (pWarlord->getUnitInfo().getLeaderPromotion() == ePromotion);
			}
		}
		return false;
	}
	else
	{
		if (!isPromotionReady())
		{
			return false;
		}
	}

	return true;
}

void CvUnit::promote(PromotionTypes ePromotion, int iLeaderUnitId)
{
	if (!canPromote(ePromotion, iLeaderUnitId))
	{
		return;
	}

	if (iLeaderUnitId >= 0)
	{
		CvUnit* pWarlord = GET_PLAYER(getOwnerINLINE()).getUnit(iLeaderUnitId);
		if (pWarlord)
		{
			pWarlord->giveExperience();
			if (!pWarlord->getNameNoDesc().empty())
			{
				setName(pWarlord->getNameKey());
			}

			//update graphics models
			m_eLeaderUnitType = pWarlord->getUnitType();
			reloadEntity();
		}
	}

/*************************************************************************************************/
/**	ADDON (Not all Promotions need XP) Sephi				                     				**/
/*************************************************************************************************/
	bool bNeedXP=true;

	if (getFreePromotionPick() != 0 && getLevel()>3)
		bNeedXP=false;
	if (GC.getPromotionInfo(ePromotion).isLeader())
		bNeedXP=false;
    if (GC.getPromotionInfo(ePromotion).isGear())
		bNeedXP=false;
	if (bNeedXP)
	{
		changeLevel(1);
		if(isBarbarian())
			changeDamage(-(getDamage() / 2));
	}
	else
	{
		if (!GC.getPromotionInfo(ePromotion).isLeader())
		{
			if (!GC.getPromotionInfo(ePromotion).isGear())
			{			
				changeFreePromotionPick(-1);
			}
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


	setHasPromotion(ePromotion, true);
/*************************************************************************************************/
/**	ADDON (Promotion GoldCost) Sephi						                     				**/
/*************************************************************************************************/
    if (GC.getPromotionInfo(ePromotion).getGoldCost() != 0)
    {
        GET_PLAYER(getOwnerINLINE()).changeGold(-GC.getPromotionInfo(ePromotion).getGoldCost());
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	testPromotionReady();

	if (IsSelected())
	{
		gDLL->getInterfaceIFace()->playGeneralSound(GC.getPromotionInfo(ePromotion).getSound());

		gDLL->getInterfaceIFace()->setDirty(UnitInfo_DIRTY_BIT, true);
	}
	else
	{
		setInfoBarDirty(true);
	}

	CvEventReporter::getInstance().unitPromoted(this, ePromotion);
}

bool CvUnit::lead(int iUnitId)
{
	if (!canLead(plot(), iUnitId))
	{
		return false;
	}

	PromotionTypes eLeaderPromotion = (PromotionTypes)m_pUnitInfo->getLeaderPromotion();

	if (-1 == iUnitId)
	{
		CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_LEADUNIT, eLeaderPromotion, getID());
		if (pInfo)
		{
			gDLL->getInterfaceIFace()->addPopup(pInfo, getOwnerINLINE(), true);
		}
		return false;
	}
	else
	{
		CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).getUnit(iUnitId);

		if (!pUnit || !pUnit->canPromote(eLeaderPromotion, getID()))
		{
			return false;
		}

		pUnit->joinGroup(NULL, true, true);

		pUnit->promote(eLeaderPromotion, getID());

		if (plot()->isActiveVisible(false))
		{
			NotifyEntity(MISSION_LEAD);
		}

		kill(true);

		return true;
	}
}


int CvUnit::canLead(const CvPlot* pPlot, int iUnitId) const
{
	PROFILE_FUNC();

	if (isDelayedDeath())
	{
		return 0;
	}

	if (NO_UNIT == getUnitType())
	{
		return 0;
	}
/*************************************************************************************************/
/**	ADDON(BATTLECAPTAINS) Sephi					                                **/
/*************************************************************************************************/
	if (GC.getUnitClassInfo(getUnitClassType()).isBattleCaptain())
	{
		return false;
	}

	CvUnitInfo& kUnitInfo = getUnitInfo();

	if((PromotionTypes)kUnitInfo.getLeaderPromotion()==NO_PROMOTION)
	{
		return 0;
	}
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/

	int iNumUnits = 0;

	if (-1 == iUnitId)
	{
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while(pUnitNode != NULL)
		{
			CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pUnit && pUnit != this && pUnit->getOwnerINLINE() == getOwnerINLINE() && pUnit->canPromote((PromotionTypes)kUnitInfo.getLeaderPromotion(), getID()))
			{
				++iNumUnits;
			}
		}
	}
	else
	{
		CvUnit* pUnit = GET_PLAYER(getOwnerINLINE()).getUnit(iUnitId);
		if (pUnit && pUnit != this && pUnit->canPromote((PromotionTypes)kUnitInfo.getLeaderPromotion(), getID()))
		{
			iNumUnits = 1;
		}
	}
	return iNumUnits;
}


int CvUnit::canGiveExperience(const CvPlot* pPlot) const
{
	int iNumUnits = 0;

	if (NO_UNIT != getUnitType() && m_pUnitInfo->getLeaderExperience() > 0)
	{
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while(pUnitNode != NULL)
		{
			CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);

			if (pUnit && pUnit != this && pUnit->getOwnerINLINE() == getOwnerINLINE() && pUnit->canAcquirePromotionAny())
			{
				++iNumUnits;
			}
		}
	}

	return iNumUnits;
}

bool CvUnit::giveExperience()
{
	CvPlot* pPlot = plot();

	if (pPlot)
	{
		int iNumUnits = canGiveExperience(pPlot);
		if (iNumUnits > 0)
		{
			int iTotalExperience = getStackExperienceToGive(iNumUnits);

			int iMinExperiencePerUnit = iTotalExperience / iNumUnits;
			int iRemainder = iTotalExperience % iNumUnits;

			CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
			int i = 0;
			while(pUnitNode != NULL)
			{
				CvUnit* pUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pPlot->nextUnitNode(pUnitNode);

				if (pUnit && pUnit != this && pUnit->getOwnerINLINE() == getOwnerINLINE() && pUnit->canAcquirePromotionAny())
				{
					pUnit->changeExperience(i < iRemainder ? iMinExperiencePerUnit+1 : iMinExperiencePerUnit);
					pUnit->testPromotionReady();
				}

				i++;
			}

			return true;
		}
	}

	return false;
}

int CvUnit::getStackExperienceToGive(int iNumUnits) const
{
	return (m_pUnitInfo->getLeaderExperience() * (100 + std::min(50, (iNumUnits - 1) * GC.getDefineINT("WARLORD_EXTRA_EXPERIENCE_PER_UNIT_PERCENT")))) / 100;
}

int CvUnit::upgradePrice(UnitTypes eUnit) const
{
	int iPrice;

/*************************************************************************************************/
/**	SPEEDTWEAK (Block Python) Sephi                                               	            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    if(GC.getDefineINT("USE_UPGRADEPRICEOVERRIDE_CALLBACK")==1)
    {
        CyArgsList argsList;
        argsList.add(getOwnerINLINE());
        argsList.add(getID());
        argsList.add((int) eUnit);
        long lResult=0;
        gDLL->getPythonIFace()->callFunction(PYGameModule, "getUpgradePriceOverride", argsList.makeFunctionArgs(), &lResult);
        if (lResult >= 0)
        {
            return lResult;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (isBarbarian())
	{
		return 0;
	}

	iPrice = GC.getDefineINT("BASE_UNIT_UPGRADE_COST");

	iPrice += (std::max(0, (GET_PLAYER(getOwnerINLINE()).getProductionNeeded(eUnit) - GET_PLAYER(getOwnerINLINE()).getProductionNeeded(getUnitType()))) * GC.getDefineINT("UNIT_UPGRADE_COST_PER_PRODUCTION"));

	if (!isHuman() && !isBarbarian())
	{
		iPrice *= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIUnitUpgradePercent();
		iPrice /= 100;

		iPrice *= std::max(0, ((GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIPerEraModifier() * GET_PLAYER(getOwnerINLINE()).getCurrentEra()) + 100));
		iPrice /= 100;
	}

	iPrice -= (iPrice * getUpgradeDiscount()) / 100;

//FfH Traits: Added by Kael 08/02/2007
    iPrice += (iPrice * GET_PLAYER(getOwnerINLINE()).getUpgradeCostModifier()) / 100;
//FfH: End Add

	return iPrice;
}


bool CvUnit::upgradeAvailable(UnitTypes eFromUnit, UnitClassTypes eToUnitClass, int iCount) const
{
/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
/** Orig Code
	UnitTypes eLoopUnit;
	int iI;
	int numUnitClassInfos = GC.getNumUnitClassInfos();

	if (iCount > numUnitClassInfos)
	{
		return false;
	}

	CvUnitInfo &fromUnitInfo = GC.getUnitInfo(eFromUnit);

	if (fromUnitInfo.getUpgradeUnitClass(eToUnitClass))
	{
		return true;
	}

	for (iI = 0; iI < numUnitClassInfos; iI++)
	{
		if (fromUnitInfo.getUpgradeUnitClass(iI))
		{
			eLoopUnit = ((UnitTypes)(GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(iI)));

			if (eLoopUnit != NO_UNIT)
			{
				if (upgradeAvailable(eLoopUnit, eToUnitClass, (iCount + 1)))
				{
					return true;
				}
			}
		}
	}
**/
	int numUnitClassInfos = GC.getNumUnitClassInfos();

	if (iCount > numUnitClassInfos)
	{
		return false;
	}

	if (GC.getUnitInfo(eFromUnit).isUpgradeUnitClassTypes(eToUnitClass))
	{
		return true;
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	return false;
}


bool CvUnit::canUpgrade(UnitTypes eUnit, bool bTestVisible) const
{
	if (eUnit == NO_UNIT)
	{
		return false;
	}

	if(!isReadyForUpgrade())
	{
		return false;
	}

	if (!bTestVisible)
	{
		if (GET_PLAYER(getOwnerINLINE()).getGold() < upgradePrice(eUnit))
		{
			return false;
		}
	}

//FfH Units: Added by Kael 05/24/2008
    CvUnitInfo& kUnitInfo = GC.getUnitInfo(eUnit);  // added Sephi faster?

    if (getLevel() < kUnitInfo.getMinLevel())
	{
        if (isHuman() || !GC.getGameINLINE().isOption(GAMEOPTION_AI_NO_MINIMUM_LEVEL))
        {
            return false;
        }
	}
    if (kUnitInfo.isDisableUpgradeTo())
    {
        return false;
    }
	if (GET_PLAYER(getOwnerINLINE()).isUnitClassMaxedOut((UnitClassTypes)(kUnitInfo.getUnitClassType())))
	{
		return false;
	}
	if (!isHuman()) //added so the AI wont spam UNTIAI_MISSIONARY priests by upgradign disciples
	{
        if (AI_getUnitAIType() == UNITAI_MISSIONARY && getLevel() < 2)
        {
            return false;
        }
	}
//FfH: End Add

/*************************************************************************************************/
/**	BETTER AI (Smarter Upgrading) Sephi                                      					**/
/*************************************************************************************************/

    if (!isHuman())
    {
		if(getAIGroup()!=NULL && getAIGroup()->getGroupType()==AIGROUP_CITY_DEFENSE)
		{
            if (kUnitInfo.isAIblockPermDefense())
            {
                return false;
            }

			if(kUnitInfo.isNoDefensiveBonus())
			{
				return false;
			}

			if(!kUnitInfo.isMilitaryHappiness())
			{
				int iCount=0;
				for (CLLNode<IDInfo>* pUnitNode = getAIGroup()->headUnitNode(); pUnitNode != NULL; pUnitNode = getAIGroup()->nextUnitNode(pUnitNode))
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

					if(GC.getUnitInfo(pLoopUnit->getUnitType()).isMilitaryHappiness())
					{
						iCount++;
					}
				}

				if(iCount<2)
				{
					return false;
				}
			}
        }

        if(eUnit==GC.getInfoTypeForString("UNIT_SCOUT"))
        {
            return false;
        }
    }

	//cannot upgrade if it would change the race
	/** disabled for now, too many exceptions
	for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
	{
		if(kUnitInfo.getFreePromotions(iI))
		{
			if(GC.getPromotionInfo((PromotionTypes)iI).isRace())
			{
				if(!isHasPromotion((PromotionTypes)iI))
				{
					return false;
				}
			}
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	if(isDelayedDeath())
	{
		return false;
	}

	if (hasUpgrade(eUnit))
	{
		return true;
	}

	return false;
}

bool CvUnit::isReadyForUpgrade() const
{
	if (!plot()->isCity() && !canMove())
	//if (!canMove())
	{
		return false;
	}

	if (plot()->getTeam() != getTeam())
	{
		return false;
	}

	return true;
}

// has upgrade is used to determine if an upgrade is possible,
// it specifically does not check whether the unit can move, whether the current plot is owned, enough gold
// those are checked in canUpgrade()
// does not search all cities, only checks the closest one
bool CvUnit::hasUpgrade(bool bSearch) const
{
	return (getUpgradeCity(bSearch) != NULL);
}

// has upgrade is used to determine if an upgrade is possible,
// it specifically does not check whether the unit can move, whether the current plot is owned, enough gold
// those are checked in canUpgrade()
// does not search all cities, only checks the closest one
bool CvUnit::hasUpgrade(UnitTypes eUnit, bool bSearch) const
{
	return (getUpgradeCity(eUnit, bSearch) != NULL);
}

// finds the 'best' city which has a valid upgrade for the unit,
// it specifically does not check whether the unit can move, or if the player has enough gold to upgrade
// those are checked in canUpgrade()
// if bSearch is true, it will check every city, if not, it will only check the closest valid city
// NULL result means the upgrade is not possible
CvCity* CvUnit::getUpgradeCity(bool bSearch) const
{
	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	UnitAITypes eUnitAI = AI_getUnitAIType();
	CvArea* pArea = area();

	int iCurrentValue = kPlayer.AI_unitValue(getUnitType(), eUnitAI, pArea);

	int iBestSearchValue = MAX_INT;
	CvCity* pBestUpgradeCity = NULL;

	for (int iI = 0; iI < GC.getNumUnitInfos(); iI++)
	{
		int iNewValue = kPlayer.AI_unitValue(((UnitTypes)iI), eUnitAI, pArea);
		if (iNewValue > iCurrentValue)
		{
			int iSearchValue;
			CvCity* pUpgradeCity = getUpgradeCity((UnitTypes)iI, bSearch, &iSearchValue);
			if (pUpgradeCity != NULL)
			{
				// if not searching or close enough, then this match will do
				if (!bSearch || iSearchValue < 16)
				{
					return pUpgradeCity;
				}

				if (iSearchValue < iBestSearchValue)
				{
					iBestSearchValue = iSearchValue;
					pBestUpgradeCity = pUpgradeCity;
				}
			}
		}
	}

	return pBestUpgradeCity;
}

// finds the 'best' city which has a valid upgrade for the unit, to eUnit type
// it specifically does not check whether the unit can move, or if the player has enough gold to upgrade
// those are checked in canUpgrade()
// if bSearch is true, it will check every city, if not, it will only check the closest valid city
// if iSearchValue non NULL, then on return it will be the city's proximity value, lower is better
// NULL result means the upgrade is not possible
CvCity* CvUnit::getUpgradeCity(UnitTypes eUnit, bool bSearch, int* iSearchValue) const
{
	if (eUnit == NO_UNIT)
	{
		return false;
	}

	CvPlayerAI& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvUnitInfo& kUnitInfo = GC.getUnitInfo(eUnit);

//FfH: Modified by Kael 05/09/2008
//	if (GC.getCivilizationInfo(kPlayer.getCivilizationType()).getCivilizationUnits(kUnitInfo.getUnitClassType()) != eUnit)
//	{
//		return false;
//	}
    if (m_pUnitInfo->getUpgradeCiv() == NO_CIVILIZATION)
    {
        if (!kPlayer.isAssimilation())
		{
            if (GC.getCivilizationInfo(kPlayer.getCivilizationType()).getCivilizationUnits(kUnitInfo.getUnitClassType()) != eUnit)
			{
                return false;
			}
		}
    }
    else
    {
        if (GC.getCivilizationInfo((CivilizationTypes)m_pUnitInfo->getUpgradeCiv()).getCivilizationUnits(kUnitInfo.getUnitClassType()) != eUnit)
        {
            return false;
        }
    }
//FfH: End Modify

	if (!upgradeAvailable(getUnitType(), ((UnitClassTypes)(kUnitInfo.getUnitClassType()))))
	{
		return false;
	}

	if (kUnitInfo.getCargoSpace() < getCargo())
	{
		return false;
	}

	CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (kUnitInfo.getSpecialCargo() != NO_SPECIALUNIT)
			{
				if (kUnitInfo.getSpecialCargo() != pLoopUnit->getSpecialUnitType())
				{
					return false;
				}
			}

			if (kUnitInfo.getDomainCargo() != NO_DOMAIN)
			{
				if (kUnitInfo.getDomainCargo() != pLoopUnit->getDomainType())
				{
					return false;
				}
			}
		}
	}

	// sea units must be built on the coast
	bool bCoastalOnly = (getDomainType() == DOMAIN_SEA);

	// results
	int iBestValue = MAX_INT;
	CvCity* pBestCity = NULL;

	// if search is true, check every city for our team
	if (bSearch)
	{
		// air units can travel any distance
		bool bIgnoreDistance = (getDomainType() == DOMAIN_AIR);

		TeamTypes eTeam = getTeam();
		int iArea = getArea();
		int iX = getX_INLINE(), iY = getY_INLINE();

		// check every player on our team's cities
		for (int iI = 0; iI < MAX_PLAYERS; iI++)
		{
			// is this player on our team?
			CvPlayerAI& kLoopPlayer = GET_PLAYER((PlayerTypes)iI);
			if (kLoopPlayer.isAlive() && kLoopPlayer.getTeam() == eTeam)
			{
				int iLoop;
				for (CvCity* pLoopCity = kLoopPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kLoopPlayer.nextCity(&iLoop))
				{
					// if coastal only, then make sure we are coast
					CvArea* pWaterArea = NULL;
					if (!bCoastalOnly || ((pWaterArea = pLoopCity->waterArea()) != NULL && !pWaterArea->isLake()))
					{
						// can this city tran this unit?

//FfH Units: Modified by Kael 05/24/2008
//						if (pLoopCity->canTrain(eUnit, false, false, true))
						if (pLoopCity->canUpgrade(eUnit, false, false, true))
//FfH: End Modify

						{
							// if we do not care about distance, then the first match will do
							if (bIgnoreDistance)
							{
								// if we do not care about distance, then return 1 for value
								if (iSearchValue != NULL)
								{
									*iSearchValue = 1;
								}

								return pLoopCity;
							}

							int iValue = plotDistance(iX, iY, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

							// if not same area, not as good (lower numbers are better)
							if (iArea != pLoopCity->getArea() && (!bCoastalOnly || iArea != pWaterArea->getID()))
							{
								iValue *= 16;
							}

							// if we cannot path there, not as good (lower numbers are better)
/*************************************************************************************************/
/**	SPEED TWEAK  Sephi                                                             				**/
/**	We only check for cities not that far away										            **/
/**	                                                                 							**/
/*************************************************************************************************/

/** Start Orig Code
							if (!generatePath(pLoopCity->plot(), 0, true))
							{
								iValue *= 16;
							}
/** End Orig Code **/
                            int XDist=pLoopCity->plot()->getX_INLINE() - plot()->getX_INLINE();
                            int YDist=pLoopCity->plot()->getY_INLINE() - plot()->getY_INLINE();
                            if (((XDist*XDist)+(YDist*YDist))<10)
                            {
                                if (!generatePath(pLoopCity->plot(), 0, true))
                                {
                                    iValue *= 16;
                                }
                            }
                            else
                            {
                                    iValue *= 16;
                            }
/*************************************************************************************************/
/**	END                                                                  						**/
/*************************************************************************************************/

							if (iValue < iBestValue)
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
	else
	{
		// find the closest city
		CvCity* pClosestCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), NO_PLAYER, getTeam(), true, bCoastalOnly);
		if (pClosestCity != NULL)
		{
			// if we can train, then return this city (otherwise it will return NULL)

//FfH Units: Modified by Kael 08/07/2007
//			if (pClosestCity->canTrain(eUnit, false, false, true))
			if (kPlayer.isAssimilation() && (m_pUnitInfo->getUpgradeCiv() == NO_CIVILIZATION))
			{
				if (GC.getCivilizationInfo(pClosestCity->getCivilizationType()).getCivilizationUnits(kUnitInfo.getUnitClassType()) != eUnit && GC.getCivilizationInfo(kPlayer.getCivilizationType()).getCivilizationUnits(kUnitInfo.getUnitClassType()) != eUnit)
				{
					return false;
				}
			}
			if (pClosestCity->canUpgrade(eUnit, false, false, true))
//FfH: End Add

			{
				// did not search, always return 1 for search value
				iBestValue = 1;

				pBestCity = pClosestCity;
			}
		}
	}

	// return the best value, if non-NULL
	if (iSearchValue != NULL)
	{
		*iSearchValue = iBestValue;
	}

	return pBestCity;
}

bool CvUnit::upgrade(UnitTypes eUnit)
{
	CvUnit* pUpgradeUnit;

	if (!canUpgrade(eUnit))
	{
		return false;
	}

	GET_PLAYER(getOwnerINLINE()).changeGold(-(upgradePrice(eUnit)));

//FfH: Modified by Kael 04/18/2009
//	pUpgradeUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, getX_INLINE(), getY_INLINE(), AI_getUnitAIType());
    UnitAITypes eUnitAI = AI_getUnitAIType();
    if (eUnitAI == UNITAI_MISSIONARY)
    {
        ReligionTypes eReligion = GET_PLAYER(getOwnerINLINE()).getStateReligion();
        if (eReligion == NO_RELIGION || GC.getUnitInfo(eUnit).getReligionSpreads(eReligion) == 0)
        {
            eUnitAI = UNITAI_RESERVE;
        }
    }
	pUpgradeUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, getX_INLINE(), getY_INLINE(), eUnitAI);
//FfH: End Modify

	FAssertMsg(pUpgradeUnit != NULL, "UpgradeUnit is not assigned a valid value");

	pUpgradeUnit->joinGroup(getGroup());
	pUpgradeUnit->setAIGroup(getAIGroup());
	pUpgradeUnit->convert(this);

	pUpgradeUnit->finishMoves();

	if (pUpgradeUnit->getLeaderUnitType() == NO_UNIT)
	{
		if (pUpgradeUnit->getExperience() > GC.getDefineINT("MAX_EXPERIENCE_AFTER_UPGRADE"))
		{
			pUpgradeUnit->setExperience(GC.getDefineINT("MAX_EXPERIENCE_AFTER_UPGRADE"));
		}
	}

	return true;
}


HandicapTypes CvUnit::getHandicapType() const
{
	return GET_PLAYER(getOwnerINLINE()).getHandicapType();
}


CivilizationTypes CvUnit::getCivilizationType() const
{
	return GET_PLAYER(getOwnerINLINE()).getCivilizationType();
}

const wchar* CvUnit::getVisualCivAdjective(TeamTypes eForTeam) const
{
	if (getVisualOwner(eForTeam) == getOwnerINLINE())
	{
		return GC.getCivilizationInfo(getCivilizationType()).getAdjectiveKey();
	}

	return L"";
}

SpecialUnitTypes CvUnit::getSpecialUnitType() const
{
	return ((SpecialUnitTypes)(m_pUnitInfo->getSpecialUnitType()));
}


UnitTypes CvUnit::getCaptureUnitType(CivilizationTypes eCivilization) const
{
	FAssert(eCivilization != NO_CIVILIZATION);
	return ((m_pUnitInfo->getUnitCaptureClassType() == NO_UNITCLASS) ? NO_UNIT : (UnitTypes)GC.getCivilizationInfo(eCivilization).getCivilizationUnits(m_pUnitInfo->getUnitCaptureClassType()));
}


UnitCombatTypes CvUnit::getUnitCombatType() const
{
	return ((UnitCombatTypes)(m_pUnitInfo->getUnitCombatType()));
}


DomainTypes CvUnit::getDomainType() const
{
	return ((DomainTypes)(m_pUnitInfo->getDomainType()));
}


InvisibleTypes CvUnit::getInvisibleType() const
{

//FfH: Modified by Kael 11/11/2008
//	return ((InvisibleTypes)(m_pUnitInfo->getInvisibleType()));
    if (m_iInvisibleType == NO_INVISIBLE)
    {
//>>>>Unofficial Bug Fix: Modified by Denev 2010/02/22
//*** CtD caused by Hidden Nationality unit (or co-existing with enemy unit with the same tile) is fixed completely.
//		if (plot()->isOwned())
		if (plot() != NULL && plot()->isOwned())
//<<<<Unofficial Bug Fix: End Modify
        {
            if (GET_PLAYER(plot()->getOwnerINLINE()).isHideUnits() && !isIgnoreHide())
            {
                if (plot()->getTeam() == getTeam())
                {
                    if (!plot()->isCity())
                    {
                        return ((InvisibleTypes)GC.getDefineINT("INVISIBLE_TYPE"));
                    }
                }
            }
        }
    }
    if (isInvisibleFromPromotion())
    {
        if (m_pUnitInfo->getEquipmentPromotion() != NO_PROMOTION)
        {
            return ((InvisibleTypes)2);
        }
        else
        {
            return ((InvisibleTypes)GC.getDefineINT("INVISIBLE_TYPE"));
        }
    }
	return ((InvisibleTypes)m_iInvisibleType);
//FfH: End Modify

}

int CvUnit::getNumSeeInvisibleTypes() const
{

//FfH: Added by Kael 12/07/2008
    if (isSeeInvisible())
    {
        return 1;
    }
//FfH: End Add

	return m_pUnitInfo->getNumSeeInvisibleTypes();
}

InvisibleTypes CvUnit::getSeeInvisibleType(int i) const
{

//FfH: Added by Kael 12/07/2008
    if (isSeeInvisible())
    {
        return ((InvisibleTypes)GC.getDefineINT("INVISIBLE_TYPE"));
    }
//FfH: End Add

	return (InvisibleTypes)(m_pUnitInfo->getSeeInvisibleType(i));
}


int CvUnit::flavorValue(FlavorTypes eFlavor) const
{
	return m_pUnitInfo->getFlavorValue(eFlavor);
}


bool CvUnit::isBarbarian() const
{
	return GET_PLAYER(getOwnerINLINE()).isBarbarian();
}


bool CvUnit::isHuman() const
{
	return GET_PLAYER(getOwnerINLINE()).isHuman();
}


int CvUnit::visibilityRange() const
{

//FfH: Modified by Kael 08/10/2007
//	return (GC.getDefineINT("UNIT_VISIBILITY_RANGE") + getExtraVisibilityRange());
    int iRange = GC.getDefineINT("UNIT_VISIBILITY_RANGE");
    iRange += getExtraVisibilityRange();
    if (plot()->getImprovementType() != NO_IMPROVEMENT)
    {
        iRange += GC.getImprovementInfo((ImprovementTypes)plot()->getImprovementType()).getVisibilityChange();
    }
	return iRange;
//FfH: End Modify

}


int CvUnit::baseMoves() const
{
	if(isGarrision())
	{
		return 0;
	}
	return (m_pUnitInfo->getMoves() + getExtraMoves() + GET_TEAM(getTeam()).getExtraMoves(getDomainType()));
}


int CvUnit::maxMoves() const
{
	return (baseMoves() * GC.getMOVE_DENOMINATOR());
}


int CvUnit::movesLeft() const
{
	return std::max(0, (maxMoves() - getMoves()));
}


bool CvUnit::canMove() const
{
	if (isDead())
	{
		return false;
	}

	if (getMoves() >= maxMoves())
	{
		return false;
	}

	if (getImmobileTimer() > 0)
	{
		return false;
	}

	if (isGarrision())
	{
		return false;
	}

	return true;
}


bool CvUnit::hasMoved()	const
{
	return (getMoves() > 0);
}


int CvUnit::airRange() const
{
	return (m_pUnitInfo->getAirRange() + getExtraAirRange());
}


int CvUnit::nukeRange() const
{
	return m_pUnitInfo->getNukeRange();
}


// XXX should this test for coal?
bool CvUnit::canBuildRoute() const
{
	int iI;

	for (iI = 0; iI < GC.getNumBuildInfos(); iI++)
	{
		if (GC.getBuildInfo((BuildTypes)iI).getRoute() != NO_ROUTE)
		{
			if (m_pUnitInfo->getBuilds(iI))
			{
				if (GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBuildInfo((BuildTypes)iI).getTechPrereq())))
				{
					return true;
				}
			}
		}
	}

	return false;
}

BuildTypes CvUnit::getBuildType() const
{
	BuildTypes eBuild;

	if (getGroup()->headMissionQueueNode() != NULL)
	{
		switch (getGroup()->headMissionQueueNode()->m_data.eMissionType)
		{
		case MISSION_MOVE_TO:
			break;

		case MISSION_ROUTE_TO:
			if (getGroup()->getBestBuildRoute(plot(), &eBuild) != NO_ROUTE)
			{
				return eBuild;
			}
			break;

		case MISSION_MOVE_TO_UNIT:
		case MISSION_SKIP:
		case MISSION_SLEEP:
		case MISSION_FORTIFY:
		case MISSION_PLUNDER:
		case MISSION_AIRPATROL:
		case MISSION_SEAPATROL:
		case MISSION_HEAL:
		case MISSION_SENTRY:
		case MISSION_AIRLIFT:
		case MISSION_NUKE:
		case MISSION_RECON:
		case MISSION_PARADROP:
		case MISSION_AIRBOMB:
		case MISSION_BOMBARD:
		case MISSION_RANGE_ATTACK:
		case MISSION_PILLAGE:
		case MISSION_SABOTAGE:
		case MISSION_DESTROY:
		case MISSION_STEAL_PLANS:
		case MISSION_FOUND:
		case MISSION_SPREAD:
		case MISSION_SPREAD_CORPORATION:
		case MISSION_JOIN:
		case MISSION_CONSTRUCT:
		case MISSION_DISCOVER:
		case MISSION_HURRY:
		case MISSION_TRADE:
		case MISSION_GREAT_WORK:
		case MISSION_INFILTRATE:
		case MISSION_GOLDEN_AGE:
		case MISSION_LEAD:
		case MISSION_ESPIONAGE:
		case MISSION_DIE_ANIMATION:
/*************************************************************************************************/
/**	ADDON(INTERFACEMODE_SPELL_OFFENSIVE) Sephi					                                **/
/*************************************************************************************************/
		case MISSION_SPELL_OFFENSIVE:
		case MISSION_TERRAFORM:
/*************************************************************************************************/
/**	END                                                                                         **/
/*************************************************************************************************/
			break;

		case MISSION_BUILD:
			return (BuildTypes)getGroup()->headMissionQueueNode()->m_data.iData1;
			break;

		default:
			FAssert(false);
			break;
		}
	}

	return NO_BUILD;
}


int CvUnit::workRate(bool bMax) const
{
	int iRate;

	if (!bMax)
	{
		if (!canMove())
		{
			return 0;
		}
	}

	iRate = m_pUnitInfo->getWorkRate();

//FfH: Added by Kael 08/13/2008
    iRate += getWorkRateModify();
//FfH: End Add

	iRate *= std::max(0, (GET_PLAYER(getOwnerINLINE()).getWorkerSpeedModifier() + 100));
	iRate /= 100;

	if (!isHuman() && !isBarbarian())
	{
		iRate *= std::max(0, (GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIWorkRateModifier() + 100));
		iRate /= 100;
	}

	return iRate;
}


bool CvUnit::isAnimal() const
{
	return m_pUnitInfo->isAnimal();
}


bool CvUnit::isNoBadGoodies() const
{
	return m_pUnitInfo->isNoBadGoodies();
}


bool CvUnit::isOnlyDefensive() const
{

//FfH Promotions: Added by Kael 08/14/2007
    if (m_iOnlyDefensive > 0)
    {
        return true;
    }
//FfH: End Add

	return m_pUnitInfo->isOnlyDefensive();
}


bool CvUnit::isNoCapture() const
{

//FfH: Added by Kael 10/25/2007
    if (isHiddenNationality())
    {
        return true;
    }
//FfH: End Add

	return m_pUnitInfo->isNoCapture();
}


bool CvUnit::isRivalTerritory() const
{
	return m_pUnitInfo->isRivalTerritory();
}


bool CvUnit::isMilitaryHappiness() const
{
	return m_pUnitInfo->isMilitaryHappiness();
}


bool CvUnit::isInvestigate() const
{
	return m_pUnitInfo->isInvestigate();
}


bool CvUnit::isCounterSpy() const
{
	return m_pUnitInfo->isCounterSpy();
}


bool CvUnit::isSpy() const
{
	return m_pUnitInfo->isSpy();
}


bool CvUnit::isFound() const
{
	return m_pUnitInfo->isFound();
}


bool CvUnit::isGoldenAge() const
{
	if (isDelayedDeath())
	{
		return false;
	}

	return m_pUnitInfo->isGoldenAge();
}

bool CvUnit::canCoexistWithEnemyUnit(TeamTypes eTeam) const
{
	if (NO_TEAM == eTeam)
	{
		if(alwaysInvisible())
		{
			return true;
		}

		return false;
	}

	if(isInvisible(eTeam, false))
	{
		return true;
	}

	return false;
}

bool CvUnit::isFighting() const
{
	return (getCombatUnit() != NULL);
}


bool CvUnit::isAttacking() const
{
	return (getAttackPlot() != NULL && !isDelayedDeath());
}


bool CvUnit::isDefending() const
{
	return (isFighting() && !isAttacking());
}


bool CvUnit::isCombat() const
{
	return (isFighting() || isAttacking());
}


int CvUnit::maxHitPoints() const
{
	return GC.getMAX_HIT_POINTS();
}


int CvUnit::currHitPoints()	const
{
	return (maxHitPoints() - getDamage());
}


bool CvUnit::isHurt() const
{
	return (getDamage() > 0);
}


bool CvUnit::isDead() const
{
	return (getDamage() >= maxHitPoints());
}


void CvUnit::setBaseCombatStr(int iCombat)
{
	m_iBaseCombat = iCombat;
}

int CvUnit::baseCombatStr() const
{
	if(GC.getUnitInfo(getUnitType()).getCombat() == 0)
		return 0;

	return std::max(0, m_iBaseCombat + m_iTotalDamageTypeCombat 
						+ m_iBaseCombatFromPromotion + m_iBaseCombatFromPotion);
}

//FfH Defense Str: Added by Kael 10/26/2007
void CvUnit::setBaseCombatStrDefense(int iCombat)
{
	m_iBaseCombatDefense = iCombat;
}

int CvUnit::baseCombatStrDefense() const
{
	if(GC.getUnitInfo(getUnitType()).getCombatDefense() == 0)
		return 0;

	return std::max(0, m_iBaseCombatDefense + m_iTotalDamageTypeCombat 
						+ m_iBaseCombatDefenseFromPromotion 
						+ m_iBaseCombatDefenseFromPotion);
}

//sometimes we need the "real" strength of a unit (e.g. when Nojah/courtesans kill an enemy)
int CvUnit::getRawBaseCombatStr() const
{
	return GC.getUnitInfo(getUnitType()).getCombat();
}

int CvUnit::getRawBaseCombatStrDefense() const
{
	return GC.getUnitInfo(getUnitType()).getCombatDefense();
}

//FfH: End Add

// maxCombatStr can be called in four different configurations
//		pPlot == NULL, pAttacker == NULL for combat when this is the attacker
//		pPlot valid, pAttacker valid for combat when this is the defender
//		pPlot valid, pAttacker == NULL (new case), when this is the defender, attacker unknown
//		pPlot valid, pAttacker == this (new case), when the defender is unknown, but we want to calc approx str
//			note, in this last case, it is expected pCombatDetails == NULL, it does not have to be, but some
//			values may be unexpectedly reversed in this case (iModifierTotal will be the negative sum)
int CvUnit::maxCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails) const
{
	int iCombat;

//FfH Damage Types: Added by Kael 09/02/2007
    const CvUnit* pDefender = NULL;
    if (pPlot == NULL)
    {
        if (pAttacker != NULL)
        {
            pDefender = pAttacker;
            pAttacker = NULL;
        }
    }
//FfH: End Add

	FAssertMsg((pPlot == NULL) || (pPlot->getTerrainType() != NO_TERRAIN), "(pPlot == NULL) || (pPlot->getTerrainType() is not expected to be equal with NO_TERRAIN)");

	// handle our new special case
	const	CvPlot*	pAttackedPlot = NULL;
	bool	bAttackingUnknownDefender = false;
	if (pAttacker == this)
	{
		bAttackingUnknownDefender = true;
		pAttackedPlot = pPlot;

		// reset these values, we will fiddle with them below
		pPlot = NULL;
		pAttacker = NULL;
	}
	// otherwise, attack plot is the plot of us (the defender)
	else if (pAttacker != NULL)
	{
		pAttackedPlot = plot();
	}

	if (pCombatDetails != NULL)
	{
		pCombatDetails->iExtraCombatPercent = 0;
		pCombatDetails->iAnimalCombatModifierTA = 0;
		pCombatDetails->iAIAnimalCombatModifierTA = 0;
		pCombatDetails->iAnimalCombatModifierAA = 0;
		pCombatDetails->iAIAnimalCombatModifierAA = 0;
		pCombatDetails->iBarbarianCombatModifierTB = 0;
		pCombatDetails->iAIBarbarianCombatModifierTB = 0;
		pCombatDetails->iBarbarianCombatModifierAB = 0;
		pCombatDetails->iAIBarbarianCombatModifierAB = 0;
		pCombatDetails->iPlotDefenseModifier = 0;
		pCombatDetails->iFortifyModifier = 0;
		pCombatDetails->iCityDefenseModifier = 0;
		pCombatDetails->iHillsAttackModifier = 0;
		pCombatDetails->iHillsDefenseModifier = 0;
		pCombatDetails->iFeatureAttackModifier = 0;
		pCombatDetails->iFeatureDefenseModifier = 0;
		pCombatDetails->iTerrainAttackModifier = 0;
		pCombatDetails->iTerrainDefenseModifier = 0;
		pCombatDetails->iCityAttackModifier = 0;
		pCombatDetails->iDomainDefenseModifier = 0;
		pCombatDetails->iCityBarbarianDefenseModifier = 0;
		pCombatDetails->iClassDefenseModifier = 0;
		pCombatDetails->iClassAttackModifier = 0;
		pCombatDetails->iCombatModifierA = 0;
		pCombatDetails->iCombatModifierT = 0;
		pCombatDetails->iDomainModifierA = 0;
		pCombatDetails->iDomainModifierT = 0;
		pCombatDetails->iAnimalCombatModifierA = 0;
		pCombatDetails->iAnimalCombatModifierT = 0;
		pCombatDetails->iRiverAttackModifier = 0;
		pCombatDetails->iAmphibAttackModifier = 0;
		pCombatDetails->iKamikazeModifier = 0;
		pCombatDetails->iModifierTotal = 0;
		pCombatDetails->iBaseCombatStr = 0;
		pCombatDetails->iCombat = 0;
		pCombatDetails->iMaxCombatStr = 0;
		pCombatDetails->iCurrHitPoints = 0;
		pCombatDetails->iMaxHitPoints = 0;
		pCombatDetails->iCurrCombatStr = 0;
		pCombatDetails->eOwner = getOwnerINLINE();
		pCombatDetails->eVisualOwner = getVisualOwner();
		pCombatDetails->sUnitName = getName().GetCString();
	}

//FfH Defense Str: Modified by Kael 08/18/2007
//	if (baseCombatStr() == 0)
//	{
//		return 0;
//	}
    int iStr;
    if ((pAttacker == NULL && pPlot == NULL) || pAttacker == this)
    {
        iStr = baseCombatStr();
    }
    else
    {
        iStr = baseCombatStrDefense();
    }

	if (iStr == 0)
	{
		return 0;
	}
//FfH: End Modify

	int iModifier = 0;
	int iExtraModifier;

	//added CombatPercentDefense, Sephi
	//iExtraModifier = getExtraCombatPercent();
	if((pPlot == NULL && pAttacker == NULL)
		|| pAttacker==this)
	{
		iExtraModifier = getExtraCombatPercent();
	}
	else
	{
		iExtraModifier = getCombatPercentDefense();
	}
	//end added
	iModifier += iExtraModifier;
	if (pCombatDetails != NULL)
	{
		pCombatDetails->iExtraCombatPercent = iExtraModifier;
	}

	// do modifiers for animals and barbarians (leaving these out for bAttackingUnknownDefender case)
	if (pAttacker != NULL)
	{
/*************************************************************************************************/
/**	ADDON (MultiBarb) Sephi    								                                	**/
/**	Only Civ Players have Handicap Bonus vs. Animals    										**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
		if (isAnimal())
/**								----  End Original Code  ----									**/
		if (getOwnerINLINE() == ANIMAL_PLAYER && (!pAttacker->isBarbarian()))
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
		{
			if (pAttacker->isHuman())
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierTA = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAIAnimalCombatModifierTA = iExtraModifier;
				}
			}
		}

/*************************************************************************************************/
/**	ADDON (MultiBarb) Sephi    								                                	**/
/**	Only Civ Players have Handicap Bonus vs. Animals    										**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
		if (pAttacker->isAnimal())
/**								----  End Original Code  ----									**/
		if (pAttacker->getOwnerINLINE() == ANIMAL_PLAYER && (!isBarbarian()))
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
		{
			if (isHuman())
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierAA = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIAnimalCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAIAnimalCombatModifierAA = iExtraModifier;
				}
			}
		}

		if (pAttacker->isBarbarian())
		{
			if (pAttacker->isHuman())
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iBarbarianCombatModifierTB = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAIBarbarianCombatModifierTB = iExtraModifier;
				}
			}
		}

		if (pAttacker->isBarbarian())
		{
			if (isHuman())
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iBarbarianCombatModifierAB = iExtraModifier;
				}
			}
			else
			{
				iExtraModifier = -GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIBarbarianCombatModifier();
				iModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{

//FfH: Modified by Kael 07/31/2008
//					pCombatDetails->iAIBarbarianCombatModifierTB = iExtraModifier;
					pCombatDetails->iAIBarbarianCombatModifierAB = iExtraModifier;
//FfH: End Modify

				}
			}
		}
	}

	// add defensive bonuses (leaving these out for bAttackingUnknownDefender case)
	if (pPlot != NULL)
	{
//>>>>Unofficial Bug Fix: Modified by Denev 2010/02/20
//*** Negative defense modifier from plot can apply to any units.
/*
		if (!noDefensiveBonus())
		{
			iExtraModifier = pPlot->defenseModifier(getTeam(), (pAttacker != NULL) ? pAttacker->ignoreBuildingDefense() : true);
*/
		iExtraModifier = pPlot->defenseModifier(getTeam(), (pAttacker != NULL) ? pAttacker->ignoreBuildingDefense() : true);

		if (pPlot->isDefensiveBonuses(this, iExtraModifier))
		{
//<<<<Unofficial Bug Fix: End Modify
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iPlotDefenseModifier = iExtraModifier;
			}
		}

		iExtraModifier = fortifyModifier();
		iModifier += iExtraModifier;
		if (pCombatDetails != NULL)
		{
			pCombatDetails->iFortifyModifier = iExtraModifier;
		}

		if (pPlot->isCity(true, getTeam()))
		{
			iExtraModifier = cityDefenseModifier();
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iCityDefenseModifier = iExtraModifier;
			}
		}

		if (pPlot->isHills())
		{
			iExtraModifier = hillsDefenseModifier();
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iHillsDefenseModifier = iExtraModifier;
			}
		}

		if (pPlot->getFeatureType() != NO_FEATURE)
		{
			iExtraModifier = featureDefenseModifier(pPlot->getFeatureType());
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iFeatureDefenseModifier = iExtraModifier;
			}
		}
		if (pPlot->getTerrainType() != NO_TERRAIN)
		{
			iExtraModifier = terrainDefenseModifier(pPlot->getTerrainType());
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iTerrainDefenseModifier = iExtraModifier;
			}
		}
	}

	// if we are attacking to an plot with an unknown defender, the calc the modifier in reverse
	if (bAttackingUnknownDefender)
	{
		pAttacker = this;
	}

	//added Sephi
	//EquipmentBonus
	iModifier += GET_PLAYER(getOwnerINLINE()).getCombatBonusFromEquipment(this);

	//end Added Sephi

	// calc attacker bonueses
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       09/20/09                                jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original code
	if (pAttacker != NULL)
*/
	if (pAttacker != NULL && pAttackedPlot != NULL)
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
	{
		int iTempModifier = 0;

		if (pAttackedPlot->isCity(true, getTeam()))
		{
			iExtraModifier = -pAttacker->cityAttackModifier();
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iCityAttackModifier = iExtraModifier;
			}

			if (pAttacker->isBarbarian())
			{
				iExtraModifier = GC.getDefineINT("CITY_BARBARIAN_DEFENSE_MODIFIER");
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iCityBarbarianDefenseModifier = iExtraModifier;
				}
			}
		}

		if (pAttackedPlot->isHills())
		{
			iExtraModifier = -pAttacker->hillsAttackModifier();
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iHillsAttackModifier = iExtraModifier;
			}
		}

		if (pAttackedPlot->getFeatureType() != NO_FEATURE)
		{
			iExtraModifier = -pAttacker->featureAttackModifier(pAttackedPlot->getFeatureType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iFeatureAttackModifier = iExtraModifier;
			}
		}
		if (pAttackedPlot->getTerrainType() != NO_TERRAIN)
		{
			iExtraModifier = -pAttacker->terrainAttackModifier(pAttackedPlot->getTerrainType());
			iModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iTerrainAttackModifier = iExtraModifier;
			}
		}

		// only compute comparisions if we are the defender with a known attacker
		if (!bAttackingUnknownDefender)
		{
			FAssertMsg(pAttacker != this, "pAttacker is not expected to be equal with this");

/*************************************************************************************************/
/**	ADDON (Combatauras) Sephi																	**/
/*************************************************************************************************/
			CvCombatAura* pAura=NULL;
			for(int i=0;i<4;i++)
			{
				if(i==0)
					pAura=getBlessing1();
				if(i==1)
					pAura=getBlessing2();
				if(i==2)
					pAura=getCurse1();
				if(i==3)
					pAura=getCurse2();

				if(pAura!=NULL)
				{
					if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()!=NO_PROMOTION && pAura->getAttackPromotionBonus()!=0)
					{
						if (pAttacker->isHasPromotion((PromotionTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()))
						{
							iModifier += pAura->getAttackPromotionBonus();
						}
					}

					if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType()!=NO_ALIGNMENT && pAura->getAlignmentAttackBonus()!=0)
					{
						if (GET_PLAYER(pAttacker->getOwner()).getAlignment()==GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType())
						{
							iModifier += pAura->getAlignmentAttackBonus();
						}
					}
				}
			}

			pAura=NULL;
			for(int i=0;i<4;i++)
			{
				if(i==0)
					pAura=pAttacker->getBlessing1();
				if(i==1)
					pAura=pAttacker->getBlessing2();
				if(i==2)
					pAura=pAttacker->getCurse1();
				if(i==3)
					pAura=pAttacker->getCurse2();

				if(pAura!=NULL)
				{
					//Bonus vs. Promotion
					if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()!=NO_PROMOTION && pAura->getAttackPromotionBonus()!=0)
					{
						if (isHasPromotion((PromotionTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPromotionType()))
						{							
							iModifier -= pAura->getAttackPromotionBonus();
						}
					}

					//Bonus vs. Alignment
					if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType()!=NO_ALIGNMENT && pAura->getAlignmentAttackBonus()!=0)
					{
						if (GET_PLAYER(getOwner()).getAlignment()==GC.getCombatAuraInfo(pAura->getCombatAuraType()).getAlignmentType())
						{
							iModifier -= pAura->getAlignmentAttackBonus();
						}
					}
				}
			}


			if(pAttacker->getBonusVSUnitsFullHealth()!=0)
			{
				if(getDamage()==0)			
				{
					iModifier -= pAttacker->getBonusVSUnitsFullHealth();
				}
			}

			if(getBonusVSUnitsFullHealth()!=0)
			{
				if(pAttacker->getDamage()==0)			
				{
					iModifier += getBonusVSUnitsFullHealth();
				}
			}

			iModifier -= pAttacker->getBonusNearUF();
			iModifier += getBonusNearUF();

			//Defensive/Ranged Strikes
			int iRangedSupportBonus = 0;
			pAttacker->getDefensiveStrikeUnit(iRangedSupportBonus, this);
			if(iRangedSupportBonus != 0)
			{
				iModifier -= iRangedSupportBonus;
			}
			getDefensiveStrikeUnit(iRangedSupportBonus, pAttacker);
			if(iRangedSupportBonus != 0)
			{
				iModifier += iRangedSupportBonus;
			}

			if(getWallOfFireBonus(pAttacker)!=0)
			{
				iModifier += getWallOfFireBonus(pAttacker);
			}

			for(int iI=0;iI<GC.getNumCivicOptionInfos();iI++)
			{
				if(GC.getCivicInfo((CivicTypes)GET_PLAYER(getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism()!=0)
				{
					if(plot()->getWorkingCity()!=NULL && plot()->getWorkingCity()->getOwnerINLINE()==getOwnerINLINE())
					{
						iModifier +=GC.getCivicInfo((CivicTypes)GET_PLAYER(getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism()*plot()->getWorkingCity()->getCultureLevel();
					}
				}

				if(GC.getCivicInfo((CivicTypes)GET_PLAYER(pAttacker->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism()!=0)
				{
					if(pAttacker->plot()->getWorkingCity()!=NULL && pAttacker->plot()->getWorkingCity()->getOwnerINLINE()==pAttacker->getOwnerINLINE())
					{
						iModifier -=GC.getCivicInfo((CivicTypes)GET_PLAYER(pAttacker->getOwnerINLINE()).getCivics((CivicOptionTypes)iI)).getNationalism()*pAttacker->plot()->getWorkingCity()->getCultureLevel();
					}
				}
			}

			if (calculateFlankingBonus(pAttacker,this)!=0)
			{
				iModifier -= calculateFlankingBonus(pAttacker,this);
			}

			if (calculateFlankingBonus(this,pAttacker)!=0)
			{
				iModifier += calculateFlankingBonus(this,pAttacker);
			}

			if(isGarrision())
			{
				iModifier += GC.getDefineINT("GARRISION_DEFENSE_BONUS");
			}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

//FfH Promotions: Added by Kael 08/13/2007
            for (int iJ=0;iJ<GC.getNumPromotionInfos();iJ++)
            {
                if ((isHasPromotion((PromotionTypes)iJ)) && (GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod()>0))
                {
                    if (pAttacker->isHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()))
                    {
                        iModifier += GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod();
                    }
                }
                if ((pAttacker->isHasPromotion((PromotionTypes)iJ)) && (GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod() > 0))
                {
                    if (isHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatType()))
                    {
                        iModifier -= GC.getPromotionInfo((PromotionTypes)iJ).getPromotionCombatMod();
                    }
                }
            }
            if (GC.getGameINLINE().getGlobalCounter() * getCombatPercentGlobalCounter() / 100 != 0)
            {
                iModifier += GC.getGameINLINE().getGlobalCounter() * getCombatPercentGlobalCounter() / 100;
            }
            if (GC.getGameINLINE().getGlobalCounter() * pAttacker->getCombatPercentGlobalCounter() / 100 != 0)
            {
                iModifier -= GC.getGameINLINE().getGlobalCounter() * pAttacker->getCombatPercentGlobalCounter() / 100;
            }
//FfH: End Add

			iExtraModifier = unitClassDefenseModifier(pAttacker->getUnitClassType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iClassDefenseModifier = iExtraModifier;
			}

			iExtraModifier = -pAttacker->unitClassAttackModifier(getUnitClassType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iClassAttackModifier = iExtraModifier;
			}

			if (pAttacker->getUnitCombatType() != NO_UNITCOMBAT)
			{
				iExtraModifier = unitCombatModifier(pAttacker->getUnitCombatType());
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iCombatModifierA = iExtraModifier;
				}
			}
			if (getUnitCombatType() != NO_UNITCOMBAT)
			{
				iExtraModifier = -pAttacker->unitCombatModifier(getUnitCombatType());
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iCombatModifierT = iExtraModifier;
				}
			}

			iExtraModifier = domainModifier(pAttacker->getDomainType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iDomainModifierA = iExtraModifier;
			}

			iExtraModifier = -pAttacker->domainModifier(getDomainType());
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iDomainModifierT = iExtraModifier;
			}

			if (pAttacker->isAnimal())
			{
				iExtraModifier = animalCombatModifier();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierA = iExtraModifier;
				}
			}

			if (isAnimal())
			{
				iExtraModifier = -pAttacker->animalCombatModifier();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAnimalCombatModifierT = iExtraModifier;
				}
			}
		}

		if (!(pAttacker->isRiver()))
		{
			if (pAttacker->plot()->isRiverCrossing(directionXY(pAttacker->plot(), pAttackedPlot)))
			{
				iExtraModifier = -GC.getRIVER_ATTACK_MODIFIER();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iRiverAttackModifier = iExtraModifier;
				}
			}
		}

		if (!(pAttacker->isAmphib()))
		{
			if (!(pAttackedPlot->isWater()) && pAttacker->plot()->isWater())
			{
				iExtraModifier = -GC.getAMPHIB_ATTACK_MODIFIER();
				iTempModifier += iExtraModifier;
				if (pCombatDetails != NULL)
				{
					pCombatDetails->iAmphibAttackModifier = iExtraModifier;
				}
			}
		}

		if (pAttacker->getKamikazePercent() != 0)
		{
			iExtraModifier = pAttacker->getKamikazePercent();
			iTempModifier += iExtraModifier;
			if (pCombatDetails != NULL)
			{
				pCombatDetails->iKamikazeModifier = iExtraModifier;
			}
		}

		// if we are attacking an unknown defender, then use the reverse of the modifier
		if (bAttackingUnknownDefender)
		{
			iModifier -= iTempModifier;
		}
		else
		{
			iModifier += iTempModifier;
		}
	}

//FfH Defense Str: Modified by Kael 08/18/2007
//	if (pCombatDetails != NULL)
//	{
//		pCombatDetails->iModifierTotal = iModifier;
//		pCombatDetails->iBaseCombatStr = baseCombatStr();
//	}
//
//	if (iModifier > 0)
//	{
//		iCombat = (baseCombatStr() * (iModifier + 100));
//	}
//	else
//	{
//		iCombat = ((baseCombatStr() * 10000) / (100 - iModifier));
//  }
    if (pCombatDetails != NULL)
    {
        pCombatDetails->iModifierTotal = iModifier;
        pCombatDetails->iBaseCombatStr = iStr;
    }

    iStr *= 100;
    if (pAttacker != NULL)
    {
        for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
        {
            if (getDamageTypeCombat((DamageTypes) iI) != 0)
            {
                if (pAttacker->getDamageTypeResist((DamageTypes) iI) != 0)
                {
                    iStr -= getDamageTypeCombat((DamageTypes) iI) * 100;
					iStr += std::max(0,getDamageTypeCombat((DamageTypes) iI) * 100 - 10*pAttacker->getDamageTypeResist((DamageTypes) iI));
                }
            }
        }
    }
    if (pDefender != NULL)
    {
        for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
        {
            if (getDamageTypeCombat((DamageTypes) iI) != 0)
            {
                if (pDefender->getDamageTypeResist((DamageTypes) iI) != 0)
                {
                    iStr -= getDamageTypeCombat((DamageTypes) iI) * 100;
					iStr += std::max(0,getDamageTypeCombat((DamageTypes)iI) * 100 - 10*pDefender->getDamageTypeResist((DamageTypes) iI));
                }
            }
        }
    }

    if (iModifier > 0)
    {
        iCombat = (iStr * (iModifier + 100)) / 100;
    }
    else
    {
        iCombat = ((iStr * 100) / (100 - iModifier));
    }
//FfH: End Modify

	if (pCombatDetails != NULL)
	{
		pCombatDetails->iCombat = iCombat;
		pCombatDetails->iMaxCombatStr = std::max(1, iCombat);
		pCombatDetails->iCurrHitPoints = currHitPoints();
		pCombatDetails->iMaxHitPoints = maxHitPoints();
		pCombatDetails->iCurrCombatStr = ((pCombatDetails->iMaxCombatStr * pCombatDetails->iCurrHitPoints) / pCombatDetails->iMaxHitPoints);
	}

	return std::max(1, iCombat);
}


int CvUnit::currCombatStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails) const
{
	return ((maxCombatStr(pPlot, pAttacker, pCombatDetails) * currHitPoints()) / maxHitPoints());
}


int CvUnit::currFirepower(const CvPlot* pPlot, const CvUnit* pAttacker) const
{
	return ((maxCombatStr(pPlot, pAttacker) + currCombatStr(pPlot, pAttacker) + 1) / 2);
}

// this nomalizes str by firepower, useful for quick odds calcs
// the effect is that a damaged unit will have an effective str lowered by firepower/maxFirepower
// doing the algebra, this means we mulitply by 1/2(1 + currHP)/maxHP = (maxHP + currHP) / (2 * maxHP)
int CvUnit::currEffectiveStr(const CvPlot* pPlot, const CvUnit* pAttacker, CombatDetails* pCombatDetails) const
{
	int currStr = currCombatStr(pPlot, pAttacker, pCombatDetails);

	currStr *= (maxHitPoints() + currHitPoints());
	currStr /= (2 * maxHitPoints());

	return currStr;
}

float CvUnit::maxCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const
{
	return (((float)(maxCombatStr(pPlot, pAttacker))) / 100.0f);
}


float CvUnit::currCombatStrFloat(const CvPlot* pPlot, const CvUnit* pAttacker) const
{
	return (((float)(currCombatStr(pPlot, pAttacker))) / 100.0f);
}


bool CvUnit::canFight() const
{

//FfH: Modified by Kael 10/31/2007
//	return (baseCombatStr() > 0);
    if (baseCombatStr() == 0 && baseCombatStrDefense() == 0)
    {
        return false;
    }
	return true;
//FfH: End Modify

}


bool CvUnit::canAttack() const
{
	if (!canFight())
	{
		return false;
	}

	if (isOnlyDefensive())
	{
		return false;
	}

//>>>>Advanced Rules: Added by Denev 2009/11/20
//*** Immobile unit can not attack.
	if (getImmobileTimer() > 0)
	{
		return false;
	}
//<<<<Advanced Rules: End Add

//FfH: Added by Kael 10/31/2007
    if (baseCombatStr() == 0)
    {
        return false;
    }
//FfH: End Add
	if(isGarrision())
	{
		return false;
	}

	return true;
}
bool CvUnit::canAttack(const CvUnit& defender) const
{
	if (!canAttack())
	{
		return false;
	}

	if (defender.getDamage() >= combatLimit())
	{
		return false;
	}

	// Artillery can't amphibious attack
	if (plot()->isWater() && !defender.plot()->isWater())
	{
//>>>>Advanced Rules: Modified by Denev 2009/11/07
//*** Illusions can amphibious attack from ships.
//		if (combatLimit() < 100)
		if (combatLimit() < 100 && collateralDamage() > 0)
//<<<<Advanced Rules: End Modify

		{
			return false;
		}
	}

	return true;
}

bool CvUnit::canDefend(const CvPlot* pPlot) const
{
	if (pPlot == NULL)
	{
		pPlot = plot();
	}

	if (!canFight())
	{
		return false;
	}

	if (!pPlot->isValidDomainForAction(*this))
	{
		if (GC.getDefineINT("LAND_UNITS_CAN_ATTACK_WATER_CITIES") == 0)
		{
			return false;
		}
	}

//FfH: Added by Kael 10/31/2007
    if (baseCombatStrDefense() == 0)
    {
        return false;
    }
//FfH: End Add

	return true;
}


bool CvUnit::canSiege(TeamTypes eTeam) const
{
	if (!canDefend())
	{
		return false;
	}

	if (!isEnemy(eTeam))
	{
		return false;
	}

	if (!isNeverInvisible())
	{
		return false;
	}

	return true;
}


int CvUnit::airBaseCombatStr() const
{
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**	SkyRocket							09/19/08									Xienwolf	**/
/**																								**/
/**						Includes Promotion Field with the UnitInfo								**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	return m_pUnitInfo->getAirCombat();
/**								----  End Original Code  ----									**/
	return m_pUnitInfo->getAirCombat() + getAirCombat();
/*************************************************************************************************/
/**	SkyRocket									END												**/
/*************************************************************************************************/
}

int CvUnit::airMaxCombatStr(const CvUnit* pOther) const
{
	int iModifier;
	int iCombat;

	if (airBaseCombatStr() == 0)
	{
		return 0;
	}

	iModifier = getExtraCombatPercent();

	if (getKamikazePercent() != 0)
	{
		iModifier += getKamikazePercent();
	}

	if (getExtraCombatPercent() != 0)
	{
		iModifier += getExtraCombatPercent();
	}

	if (NULL != pOther)
	{
		if (pOther->getUnitCombatType() != NO_UNITCOMBAT)
		{
			iModifier += unitCombatModifier(pOther->getUnitCombatType());
		}

		iModifier += domainModifier(pOther->getDomainType());

		if (pOther->isAnimal())
		{
			iModifier += animalCombatModifier();
		}
	}

	if (iModifier > 0)
	{
		iCombat = (airBaseCombatStr() * (iModifier + 100));
	}
	else
	{
		iCombat = ((airBaseCombatStr() * 10000) / (100 - iModifier));
	}

	return std::max(1, iCombat);
}


int CvUnit::airCurrCombatStr(const CvUnit* pOther) const
{
	return ((airMaxCombatStr(pOther) * currHitPoints()) / maxHitPoints());
}


float CvUnit::airMaxCombatStrFloat(const CvUnit* pOther) const
{
	return (((float)(airMaxCombatStr(pOther))) / 100.0f);
}


float CvUnit::airCurrCombatStrFloat(const CvUnit* pOther) const
{
	return (((float)(airCurrCombatStr(pOther))) / 100.0f);
}


int CvUnit::combatLimit() const
{

//FfH: Modified by Kael 04/26/2008
//	return m_pUnitInfo->getCombatLimit();
    return m_iCombatLimit;
//FfH: End Modify

}


int CvUnit::airCombatLimit() const
{
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**	SkyRocket							09/19/08									Xienwolf	**/
/**																								**/
/**						Includes Promotion Field with the UnitInfo								**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	return m_pUnitInfo->getAirCombatLimit();
/**								----  End Original Code  ----									**/
	return range(m_pUnitInfo->getAirCombatLimit() + getAirCombatLimitBoost(), 0, 100);
/*************************************************************************************************/
/**	SkyRocket									END												**/
/*************************************************************************************************/
}

bool CvUnit::canAirAttack() const
{
	return (airBaseCombatStr() > 0);
}


bool CvUnit::canAirDefend(const CvPlot* pPlot) const
{
	if (pPlot == NULL)
	{
		pPlot = plot();
	}

	if (maxInterceptionProbability() == 0)
	{
		return false;
	}

	if (getDomainType() != DOMAIN_AIR)
	{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       10/30/09                     Mongoose & jdog5000      */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* original bts code
		if (!pPlot->isValidDomainForLocation(*this))
*/
		// From Mongoose SDK
		// Land units which are cargo cannot intercept
		if (!pPlot->isValidDomainForLocation(*this) || isCargo())
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
		{
			return false;
		}
	}

	return true;
}


int CvUnit::airCombatDamage(const CvUnit* pDefender) const
{
	CvCity* pCity;
	CvPlot* pPlot;
	int iOurStrength;
	int iTheirStrength;
	int iStrengthFactor;
	int iDamage;

	pPlot = pDefender->plot();

	iOurStrength = airCurrCombatStr(pDefender);
	FAssertMsg(iOurStrength > 0, "Air combat strength is expected to be greater than zero");
	iTheirStrength = pDefender->maxCombatStr(pPlot, this);

	iStrengthFactor = ((iOurStrength + iTheirStrength + 1) / 2);

	iDamage = std::max(1, ((GC.getDefineINT("AIR_COMBAT_DAMAGE") * (iOurStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor)));

	pCity = pPlot->getPlotCity();

	if (pCity != NULL)
	{
		iDamage *= std::max(0, (pCity->getAirModifier() + 100));
		iDamage /= 100;
	}

	return iDamage;
}


int CvUnit::rangeCombatDamage(const CvUnit* pDefender) const
{
	CvPlot* pPlot;
	int iOurStrength;
	int iTheirStrength;
//	int iStrengthFactor;	modified Sephi
	int iDamage;

	pPlot = pDefender->plot();

	iOurStrength = airCurrCombatStr(pDefender);
	FAssertMsg(iOurStrength > 0, "Combat strength is expected to be greater than zero");
	iTheirStrength = pDefender->maxCombatStr(pPlot, this);

/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**	Vehem Tweak							02/05/09	 											**/
/**																								**/
/**		Ranged attack damage more dependant on unit strength and has random element				**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	iStrengthFactor = ((iOurStrength + iTheirStrength + 1) / 2);

	iDamage = std::max(1, ((GC.getDefineINT("RANGE_COMBAT_DAMAGE") * (iOurStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor)));
/**								----  End Original Code  ----									**/

	int rangeCombatDamage = GC.getDefineINT("RANGE_COMBAT_DAMAGE");
	int randomRangedCombatDamage = GC.getGameINLINE().getSorenRandNum(rangeCombatDamage, "RangedDamage1") + GC.getGameINLINE().getSorenRandNum(rangeCombatDamage, "RangedDamage2") + 1;
	iDamage = std::max(1, ( randomRangedCombatDamage * (iOurStrength) / (iTheirStrength) ));

/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/

	return iDamage;
}


CvUnit* CvUnit::bestInterceptor(const CvPlot* pPlot) const
{
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	iBestValue = 0;
	pBestUnit = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (isEnemy(GET_PLAYER((PlayerTypes)iI).getTeam()) && !isInvisible(GET_PLAYER((PlayerTypes)iI).getTeam(), false, false))
			{
				for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
				{
					if (pLoopUnit->canAirDefend())
					{
						if (!pLoopUnit->isMadeInterception())
						{
							if ((pLoopUnit->getDomainType() != DOMAIN_AIR) || !(pLoopUnit->hasMoved()))
							{
								if ((pLoopUnit->getDomainType() != DOMAIN_AIR) || (pLoopUnit->getGroup()->getActivityType() == ACTIVITY_INTERCEPT))
								{
									if (plotDistance(pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) <= pLoopUnit->airRange())
									{
										iValue = pLoopUnit->currInterceptionProbability();

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

	return pBestUnit;
}


CvUnit* CvUnit::bestSeaPillageInterceptor(CvUnit* pPillager, int iMinOdds) const
{
	CvUnit* pBestUnit = NULL;

	for (int iDX = -1; iDX <= 1; ++iDX)
	{
		for (int iDY = -1; iDY <= 1; ++iDY)
		{
			CvPlot* pLoopPlot = plotXY(pPillager->getX_INLINE(), pPillager->getY_INLINE(), iDX, iDY);

			if (NULL != pLoopPlot)
			{
				CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

				while (NULL != pUnitNode)
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

					if (NULL != pLoopUnit)
					{
						if (pLoopUnit->area() == pPillager->plot()->area())
						{
							if (!pLoopUnit->isInvisible(getTeam(), false))
							{
								if (isEnemy(pLoopUnit->getTeam()))
								{
									if (DOMAIN_SEA == pLoopUnit->getDomainType())
									{
										if (ACTIVITY_PATROL == pLoopUnit->getGroup()->getActivityType())
										{
											if (NULL == pBestUnit || pLoopUnit->isBetterDefenderThan(pBestUnit, this))
											{
												if (getCombatOdds(pPillager, pLoopUnit) < iMinOdds)
												{
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

	return pBestUnit;
}


bool CvUnit::isAutomated() const
{
	return getGroup()->isAutomated();
}


bool CvUnit::isWaiting() const
{
	return getGroup()->isWaiting();
}


bool CvUnit::isFortifyable() const
{
	if (!canFight() || noDefensiveBonus() || ((getDomainType() != DOMAIN_LAND) && (getDomainType() != DOMAIN_IMMOBILE)) || isAnimal())
	{
		return false;
	}

	return true;
}


int CvUnit::fortifyModifier() const
{
	if (!isFortifyable())
	{
		return 0;
	}

//FfH: Modified by Kael 10/26/2007
//	return (getFortifyTurns() * GC.getFORTIFY_MODIFIER_PER_TURN());
   	int i = getFortifyTurns() * GC.getFORTIFY_MODIFIER_PER_TURN();
    if (isDoubleFortifyBonus())
    {
        i *= 2;
    }
    return i;
//FfH: End Modify

}


int CvUnit::getExperienceNeeded(int iLevel) const
{
	//new Formula
	int iValue = MAX_INT;
	switch(iLevel)
	{	
		case 1:
			iValue = 10;
			break;
		case 2:
			iValue = 20;
			break;
		case 3:
			iValue = 30;
			break;
		case 4:
			iValue = 40;
			break;
		case 5:
			iValue = 55;
			break;
		case 6:
			iValue = 75;
			break;
		case 7:
			iValue = 100;
			break;
		case 8:
			iValue = 130;
			break;
		case 9:
			iValue = 165;
			break;
		case 10:
			iValue = 205;
			break;
		default:
			iValue = (iLevel - 6) * 50;
	}

	iValue *= 100 + GET_PLAYER(getOwnerINLINE()).getLevelExperienceModifier();
	iValue /= 100;

	return iValue;
}

int CvUnit::experienceNeeded() const
{
	return getExperienceNeeded(getLevel());

	// Use python to determine pillage amounts...
	int iExperienceNeeded;
	long lExperienceNeeded;

	lExperienceNeeded = 0;
	iExperienceNeeded = 0;

	CyArgsList argsList;
	argsList.add(getLevel());	// pass in the units level
	argsList.add(getOwnerINLINE());	// pass in the units

	gDLL->getPythonIFace()->callFunction(PYGameModule, "getExperienceNeeded", argsList.makeFunctionArgs(),&lExperienceNeeded);

	iExperienceNeeded = (int)lExperienceNeeded;

	return iExperienceNeeded;
}


int CvUnit::attackXPValue() const
{

//FfH: Modified by Kael 02/12/2009
//	return m_pUnitInfo->getXPValueAttack();
    int iXP = m_pUnitInfo->getXPValueAttack();
	if (isAnimal() || isBarbarian())
	{
	    iXP = iXP * GC.getDefineINT("BARBARIAN_EXPERIENCE_MODIFIER") / 100;
	}
	return iXP;
//FfH: End Modify

}


int CvUnit::defenseXPValue() const
{

//FfH: Modified by Kael 02/12/2009
//	return m_pUnitInfo->getXPValueDefense();
    int iXP = m_pUnitInfo->getXPValueDefense();
	if (isAnimal() || isBarbarian())
	{
	    iXP = iXP * GC.getDefineINT("BARBARIAN_EXPERIENCE_MODIFIER") / 100;
	}
	return iXP;
//FfH: End Modify

}


int CvUnit::maxXPValue() const
{
	int iMaxValue;

	iMaxValue = MAX_INT;

//FfH: Modified by Kael 02/12/2009
//	if (isAnimal())
//	{
//		iMaxValue = std::min(iMaxValue, GC.getDefineINT("ANIMAL_MAX_XP_VALUE"));
//	}
//	if (isBarbarian())
//	{
//		iMaxValue = std::min(iMaxValue, GC.getDefineINT("BARBARIAN_MAX_XP_VALUE"));
//	}
	if (!::isWorldUnitClass(getUnitClassType()))
	{
        if (isAnimal())
        {
            iMaxValue = std::min(iMaxValue, GC.getDefineINT("ANIMAL_MAX_XP_VALUE"));
        }
        if (isBarbarian())
        {
            iMaxValue = std::min(iMaxValue, GC.getDefineINT("BARBARIAN_MAX_XP_VALUE"));
        }
	}
//FfH: End Modify

	return iMaxValue;
}


int CvUnit::firstStrikes() const
{
	return std::max(0, (m_pUnitInfo->getFirstStrikes() + getExtraFirstStrikes()));
}


int CvUnit::chanceFirstStrikes() const
{
	return std::max(0, (m_pUnitInfo->getChanceFirstStrikes() + getExtraChanceFirstStrikes()));
}


int CvUnit::maxFirstStrikes() const
{
	return (firstStrikes() + chanceFirstStrikes());
}


bool CvUnit::isRanged() const
{
	int i;
	CvUnitInfo * pkUnitInfo = &getUnitInfo();
	for ( i = 0; i < pkUnitInfo->getGroupDefinitions(); i++ )
	{
		if ( !getArtInfo(i, GET_PLAYER(getOwnerINLINE()).getCurrentEra())->getActAsRanged() )
		{
			return false;
		}
	}
	return true;
}


bool CvUnit::alwaysInvisible() const
{
	return m_pUnitInfo->isInvisible();
}


bool CvUnit::immuneToFirstStrikes() const
{
	return (m_pUnitInfo->isFirstStrikeImmune() || (getImmuneToFirstStrikesCount() > 0));
}


bool CvUnit::noDefensiveBonus() const
{
	return m_pUnitInfo->isNoDefensiveBonus();
}


bool CvUnit::ignoreBuildingDefense() const
{

//FfH: Modifed by Kael 11/17/2007
//	return m_pUnitInfo->isIgnoreBuildingDefense();
    if (m_pUnitInfo->isIgnoreBuildingDefense())
    {
        return true;
    }
	return m_iIgnoreBuildingDefense == 0 ? false : true;
//FfH: End Modify

}


bool CvUnit::canMoveImpassable() const
{

//FfH Flying: Added by Kael 07/30/2007
    if (isFlying())
    {
        return true;
    }
//FfH: End Add

	return m_pUnitInfo->isCanMoveImpassable();
}

bool CvUnit::canMoveAllTerrain() const
{

//FfH Flying: Added by Kael 07/30/2007
    if (isFlying())
    {
        return true;
    }
    if (isWaterWalking() && getDomainType()==DOMAIN_LAND)
    {
        return true;
    }
//FfH: End Add

	return m_pUnitInfo->isCanMoveAllTerrain();
}

bool CvUnit::flatMovementCost() const
{

//FfH Flying: Added by Kael 07/30/2007
    if (isFlying())
    {
        return true;
    }
//FfH: End Add

	return m_pUnitInfo->isFlatMovementCost();
}


bool CvUnit::ignoreTerrainCost() const
{

//FfH Flying: Added by Kael 07/30/2007
    if (isFlying())
    {
        return true;
    }
//FfH: End Add

	return m_pUnitInfo->isIgnoreTerrainCost();
}


bool CvUnit::isNeverInvisible() const
{
	return (!alwaysInvisible() && (getInvisibleType() == NO_INVISIBLE));
}


bool CvUnit::isInvisible(TeamTypes eTeam, bool bDebug, bool bCheckCargo) const
{
	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return false;
	}

	if (getTeam() == eTeam)
	{
		return false;
	}

//>>>>Unofficial Bug Fix: Moved from below(*2) by Denev 2009/10/19
//*** Cargo checking should be placed before invisibile check, because cargo checking doesn't relate invisibility.
//*** This is not a visibility matter, but a sytem rule. Even Dies Diei can't reveal units which loaded on.
	if (alwaysInvisible())
	{
		return true;
	}

	if (bCheckCargo && isCargo())
	{
		return true;
	}
//<<<<Unofficial Bug Fix: End Move

//FfH: Added by Kael 04/11/2008
    if (plot() != NULL)
    {
        if (plot()->isCity())
        {
            if (getTeam() == plot()->getTeam())
            {
                return false;
            }
        }
        if (plot()->isOwned())
        {
            if (plot()->getTeam() != getTeam())
            {
                if (GET_PLAYER(plot()->getOwnerINLINE()).isSeeInvisible())
                {
                    return false;
                }
            }
            if (plot()->getTeam() == getTeam())
            {
                if (GET_PLAYER(plot()->getOwnerINLINE()).isHideUnits() && !isIgnoreHide())
                {
                    return true;
                }
            }
        }
    }
//FfH: End Add

//>>>>Unofficial Bug Fix: Moved to above(*2) by Denev 2009/10/19
/*
	if (alwaysInvisible())
	{
		return true;
	}

	if (bCheckCargo && isCargo())
	{
		return true;
	}
*/
//<<<<Unofficial Bug Fix: End Move

	if (getInvisibleType() == NO_INVISIBLE)
	{
		return false;
	}

//FfH: Added by Kael 01/16/2009
    if (plot() == NULL)
    {
        return false;
    }
//FfH: End Add

	return !(plot()->isInvisibleVisible(eTeam, getInvisibleType()));
}


bool CvUnit::isNukeImmune() const
{
	return m_pUnitInfo->isNukeImmune();
}


int CvUnit::maxInterceptionProbability() const
{
	return std::max(0, m_pUnitInfo->getInterceptionProbability() + getExtraIntercept());
}


int CvUnit::currInterceptionProbability() const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return maxInterceptionProbability();
	}
	else
	{
		return ((maxInterceptionProbability() * currHitPoints()) / maxHitPoints());
	}
}


int CvUnit::evasionProbability() const
{
	return std::max(0, m_pUnitInfo->getEvasionProbability() + getExtraEvasion());
}


int CvUnit::withdrawalProbability() const
{
	if (getDomainType() == DOMAIN_LAND && plot()->isWater())
	{
		return 0;
	}

//FfH: Added by Kael 04/06/2009
    if (getImmobileTimer() > 0)
    {
        return 0;
    }
//FfH: End Add
	
	return std::min(90,std::max(0, (m_pUnitInfo->getWithdrawalProbability() + getExtraWithdrawal())));
}


int CvUnit::collateralDamage() const
{
	return std::max(0, (m_pUnitInfo->getCollateralDamage()));
}


int CvUnit::collateralDamageLimit() const
{
	return std::max(0, m_pUnitInfo->getCollateralDamageLimit() * GC.getMAX_HIT_POINTS() / 100);
}


int CvUnit::collateralDamageMaxUnits() const
{
	return std::max(0, m_pUnitInfo->getCollateralDamageMaxUnits());
}


int CvUnit::cityAttackModifier() const
{
	return (m_pUnitInfo->getCityAttackModifier() + getExtraCityAttackPercent());
}


int CvUnit::cityDefenseModifier() const
{
	return (m_pUnitInfo->getCityDefenseModifier() + getExtraCityDefensePercent());
}


int CvUnit::animalCombatModifier() const
{
	return m_pUnitInfo->getAnimalCombatModifier();
}


int CvUnit::hillsAttackModifier() const
{
	return (m_pUnitInfo->getHillsAttackModifier() + getExtraHillsAttackPercent());
}


int CvUnit::hillsDefenseModifier() const
{
	return (m_pUnitInfo->getHillsDefenseModifier() + getExtraHillsDefensePercent());
}


int CvUnit::terrainAttackModifier(TerrainTypes eTerrain) const
{
	FAssertMsg(eTerrain >= 0, "eTerrain is expected to be non-negative (invalid Index)");
	FAssertMsg(eTerrain < GC.getNumTerrainInfos(), "eTerrain is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getTerrainAttackModifier(eTerrain) + getExtraTerrainAttackPercent(eTerrain));
}


int CvUnit::terrainDefenseModifier(TerrainTypes eTerrain) const
{
	FAssertMsg(eTerrain >= 0, "eTerrain is expected to be non-negative (invalid Index)");
	FAssertMsg(eTerrain < GC.getNumTerrainInfos(), "eTerrain is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getTerrainDefenseModifier(eTerrain) + getExtraTerrainDefensePercent(eTerrain));
}


int CvUnit::featureAttackModifier(FeatureTypes eFeature) const
{
	FAssertMsg(eFeature >= 0, "eFeature is expected to be non-negative (invalid Index)");
	FAssertMsg(eFeature < GC.getNumFeatureInfos(), "eFeature is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getFeatureAttackModifier(eFeature) + getExtraFeatureAttackPercent(eFeature));
}

int CvUnit::featureDefenseModifier(FeatureTypes eFeature) const
{
	FAssertMsg(eFeature >= 0, "eFeature is expected to be non-negative (invalid Index)");
	FAssertMsg(eFeature < GC.getNumFeatureInfos(), "eFeature is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getFeatureDefenseModifier(eFeature) + getExtraFeatureDefensePercent(eFeature));
}

int CvUnit::unitClassAttackModifier(UnitClassTypes eUnitClass) const
{
	FAssertMsg(eUnitClass >= 0, "eUnitClass is expected to be non-negative (invalid Index)");
	FAssertMsg(eUnitClass < GC.getNumUnitClassInfos(), "eUnitClass is expected to be within maximum bounds (invalid Index)");
	return m_pUnitInfo->getUnitClassAttackModifier(eUnitClass);
}


int CvUnit::unitClassDefenseModifier(UnitClassTypes eUnitClass) const
{
	FAssertMsg(eUnitClass >= 0, "eUnitClass is expected to be non-negative (invalid Index)");
	FAssertMsg(eUnitClass < GC.getNumUnitClassInfos(), "eUnitClass is expected to be within maximum bounds (invalid Index)");
	return m_pUnitInfo->getUnitClassDefenseModifier(eUnitClass);
}


int CvUnit::unitCombatModifier(UnitCombatTypes eUnitCombat) const
{
	FAssertMsg(eUnitCombat >= 0, "eUnitCombat is expected to be non-negative (invalid Index)");
	FAssertMsg(eUnitCombat < GC.getNumUnitCombatInfos(), "eUnitCombat is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getUnitCombatModifier(eUnitCombat) + getExtraUnitCombatModifier(eUnitCombat));
}


int CvUnit::domainModifier(DomainTypes eDomain) const
{
	FAssertMsg(eDomain >= 0, "eDomain is expected to be non-negative (invalid Index)");
	FAssertMsg(eDomain < NUM_DOMAIN_TYPES, "eDomain is expected to be within maximum bounds (invalid Index)");
	return (m_pUnitInfo->getDomainModifier(eDomain) + getExtraDomainModifier(eDomain));
}


int CvUnit::bombardRate() const
{
	return (m_pUnitInfo->getBombardRate() + getExtraBombardRate());
}


int CvUnit::airBombBaseRate() const
{
	return m_pUnitInfo->getBombRate();
}


int CvUnit::airBombCurrRate() const
{
	return ((airBombBaseRate() * currHitPoints()) / maxHitPoints());
}


SpecialUnitTypes CvUnit::specialCargo() const
{
	return ((SpecialUnitTypes)(m_pUnitInfo->getSpecialCargo()));
}


DomainTypes CvUnit::domainCargo() const
{
	return ((DomainTypes)(m_pUnitInfo->getDomainCargo()));
}


int CvUnit::cargoSpace() const
{
	return m_iCargoCapacity;
}

void CvUnit::changeCargoSpace(int iChange)
{
	if (iChange != 0)
	{
		m_iCargoCapacity += iChange;
		FAssert(m_iCargoCapacity >= 0);
		setInfoBarDirty(true);
	}
}

bool CvUnit::isFull() const
{
	return (getCargo() >= cargoSpace());
}


int CvUnit::cargoSpaceAvailable(SpecialUnitTypes eSpecialCargo, DomainTypes eDomainCargo) const
{
	if (specialCargo() != NO_SPECIALUNIT)
	{
		if (specialCargo() != eSpecialCargo)
		{
			return 0;
		}
	}

	if (domainCargo() != NO_DOMAIN)
	{
		if (domainCargo() != eDomainCargo)
		{
			return 0;
		}
	}

	return std::max(0, (cargoSpace() - getCargo()));
}


bool CvUnit::hasCargo() const
{
	return (getCargo() > 0);
}


bool CvUnit::canCargoAllMove() const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pPlot;

	pPlot = plot();

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (pLoopUnit->getDomainType() == DOMAIN_LAND)
			{
				if (!(pLoopUnit->canMove()))
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool CvUnit::canCargoEnterArea(TeamTypes eTeam, const CvArea* pArea, bool bIgnoreRightOfPassage) const
{
	CvPlot* pPlot = plot();

	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit->getTransportUnit() == this)
		{
			if (!pLoopUnit->canEnterArea(eTeam, pArea, bIgnoreRightOfPassage))
			{
				return false;
			}
		}
	}

	return true;
}

int CvUnit::getUnitAICargo(UnitAITypes eUnitAI) const
{
	int iCount = 0;

	std::vector<CvUnit*> aCargoUnits;
	getCargoUnits(aCargoUnits);
	for (uint i = 0; i < aCargoUnits.size(); ++i)
	{
		if (aCargoUnits[i]->AI_getUnitAIType() == eUnitAI)
		{
			++iCount;
		}
	}

	return iCount;
}


int CvUnit::getID() const
{
	return m_iID;
}


int CvUnit::getIndex() const
{
	return (getID() & FLTA_INDEX_MASK);
}


IDInfo CvUnit::getIDInfo() const
{
	IDInfo unit(getOwnerINLINE(), getID());
	return unit;
}


void CvUnit::setID(int iID)
{
	m_iID = iID;
}


int CvUnit::getGroupID() const
{
	return m_iGroupID;
}


bool CvUnit::isInGroup() const
{
	return(getGroupID() != FFreeList::INVALID_INDEX);
}


bool CvUnit::isGroupHead() const // XXX is this used???
{
	return (getGroup()->getHeadUnit() == this);
}


CvSelectionGroup* CvUnit::getGroup() const
{
	return GET_PLAYER(getOwnerINLINE()).getSelectionGroup(getGroupID());
}


bool CvUnit::canJoinGroup(const CvPlot* pPlot, CvSelectionGroup* pSelectionGroup) const
{
	CvUnit* pHeadUnit;

	// do not allow someone to join a group that is about to be split apart
	// this prevents a case of a never-ending turn
	if (pSelectionGroup->AI_isForceSeparate())
	{
		return false;
	}

	if (pSelectionGroup->getOwnerINLINE() == NO_PLAYER)
	{
		pHeadUnit = pSelectionGroup->getHeadUnit();

		if (pHeadUnit != NULL)
		{
			if (pHeadUnit->getOwnerINLINE() != getOwnerINLINE())
			{
				return false;
			}
		}
	}
	else
	{
		if (pSelectionGroup->getOwnerINLINE() != getOwnerINLINE())
		{
			return false;
		}
	}

	if (pSelectionGroup->getNumUnits() > 0)
	{
		if (!(pSelectionGroup->atPlot(pPlot)))
		{
			return false;
		}

		if (pSelectionGroup->getDomainType() != getDomainType())
		{
			return false;
		}

/*************************************************************************************************/
/**	Xienwolf Tweak							01/04/09											**/
/**	ADDON (Allow HN units to group) merged Sephi									            **/
/**							Allows HN units to group with each other							**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
//FfH: Added by Kael 11/14/2007
        if (isAIControl())
        {
            return false;
        }
        if (isHiddenNationality())
        {
            return false;
        }
        pHeadUnit = pSelectionGroup->getHeadUnit();
        if (pHeadUnit != NULL)
        {
            if (pHeadUnit->isHiddenNationality())
            {
                return false;
            }
            if (pHeadUnit->isAIControl())
            {
                return false;
            }
		}
//FfH: End Add
/**								----  End Original Code  ----									**/
        pHeadUnit = pSelectionGroup->getHeadUnit();
        if (pHeadUnit != NULL)
	    {
			if (pHeadUnit->isHiddenNationality() != isHiddenNationality())
			{
                return false;
			}
            if (pHeadUnit->isAIControl() != isAIControl())
			{
                return false;
			}
		}
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/

	}

	return true;
}


void CvUnit::joinGroup(CvSelectionGroup* pSelectionGroup, bool bRemoveSelected, bool bRejoin)
{
	/*TCHAR szSelectionList[2048];
	sprintf(szSelectionList,"");
	CLLNode<int>* pSelectionGroupNode = GET_PLAYER(getOwnerINLINE()).headGroupCycleNode();
	while (pSelectionGroupNode != NULL)
	{
		sprintf(szSelectionList,"%s,%d",szSelectionList,pSelectionGroupNode->m_data);
		pSelectionGroupNode = GET_PLAYER(getOwnerINLINE()).nextGroupCycleNode(pSelectionGroupNode);
	}
	//FAssert(getID() != 786448);
	oosLog("AISelectionGroup"
		,"Turn:%d,Player:%d,UnitID:%d,X:%d,Y:%d,GroupID:%d,SelectionGroupID:%d,X:%d,Y:%d,Count:%d,Flags:%d %d,joinGroup:%s\n"
		,GC.getGameINLINE().getElapsedGameTurns()
		,getOwner()
		,getID()
		,getX()
		,getY()
		,getGroupID()
		,pSelectionGroup != NULL ? pSelectionGroup->getID() : 0
		,pSelectionGroup != NULL ? pSelectionGroup->getX() : 0
		,pSelectionGroup != NULL ? pSelectionGroup->getY() : 0
		,pSelectionGroup != NULL ? pSelectionGroup->getNumUnits() : 0//,getNumSelectionGroups()
		,bRemoveSelected
		,bRejoin
		,szSelectionList
	);*/
	CvSelectionGroup* pOldSelectionGroup;
	CvSelectionGroup* pNewSelectionGroup;
	CvPlot* pPlot;

	pOldSelectionGroup = GET_PLAYER(getOwnerINLINE()).getSelectionGroup(getGroupID());

	if ((pSelectionGroup != pOldSelectionGroup) || (pOldSelectionGroup == NULL))
	{
		pPlot = plot();

		if (pSelectionGroup != NULL)
		{
			pNewSelectionGroup = pSelectionGroup;
		}
		else
		{
			if (bRejoin)
			{
				pNewSelectionGroup = GET_PLAYER(getOwnerINLINE()).addSelectionGroup();
				pNewSelectionGroup->init(pNewSelectionGroup->getID(), getOwnerINLINE());
			}
			else
			{
				pNewSelectionGroup = NULL;
			}
		}

		if ((pNewSelectionGroup == NULL) || canJoinGroup(pPlot, pNewSelectionGroup))
		{
			if (pOldSelectionGroup != NULL)
			{
				bool bWasHead = false;
				if (!isHuman())
				{
					if (pOldSelectionGroup->getNumUnits() > 1)
					{
						if (pOldSelectionGroup->getHeadUnit() == this)
						{
							bWasHead = true;
						}
					}
				}

				pOldSelectionGroup->removeUnit(this);

				// if we were the head, if the head unitAI changed, then force the group to separate (non-humans)
/*************************************************************************************************/
/**	BETTER AI (Stop some groups from forceseperating) Sephi                        	            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
/** orig code

				if (bWasHead)
				{
/**/
                bool bvalid=true;

                if (pSelectionGroup!=NULL && pSelectionGroup->getHeadUnit())
                {
                    if (AI_getGroupflag()==GROUPFLAG_CONQUEST && pSelectionGroup->getHeadUnit()->AI_getGroupflag()==GROUPFLAG_CONQUEST)
                    {
                        bvalid=false;
                    }
                }

                if (pSelectionGroup!=NULL && pSelectionGroup->getHeadUnit())
                {
                    if (AI_getGroupflag()==GROUPFLAG_DEFENSE_NEW && pSelectionGroup->getHeadUnit()->AI_getGroupflag()==GROUPFLAG_DEFENSE_NEW)
                    {
                        bvalid=false;
                    }
                }

                if (bWasHead && bvalid)
                {
/*************************************************************************************************/
/**	END                                                                  						**/
/*************************************************************************************************/
					FAssert(pOldSelectionGroup->getHeadUnit() != NULL);
					if (pOldSelectionGroup->getHeadUnit()->AI_getUnitAIType() != AI_getUnitAIType())
					{
						pOldSelectionGroup->AI_makeForceSeparate();
					}
				}
			}

			if ((pNewSelectionGroup != NULL) && pNewSelectionGroup->addUnit(this, false))
			{
				m_iGroupID = pNewSelectionGroup->getID();
			}
			else
			{
				m_iGroupID = FFreeList::INVALID_INDEX;
			}
			/*if(isOOSLogging())
			{ //PS: this is group, not AIGroup
				oosLog("AIGroup"
					,"Turn:%d,PlayerID:%d,GroupID:%d,UnitID:%d,joinGroup:%d,pOldSelectionGroup:%d\n"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwnerINLINE()
					,getGroup() != NULL ? getGroup()->getID() : 0
					,getID()
					,m_iGroupID
					,pOldSelectionGroup != NULL ? pOldSelectionGroup->getID() : 0
				);
			}*/
			if (getGroup() != NULL)
			{
				if (getGroup()->getNumUnits() > 1)
				{
					getGroup()->setActivityType(ACTIVITY_AWAKE);
				}
				else
				{
					GET_PLAYER(getOwnerINLINE()).updateGroupCycle(this);
				}
			}

			if (getTeam() == GC.getGameINLINE().getActiveTeam())
			{
				if (pPlot != NULL)
				{
					pPlot->setFlagDirty(true);
				}
			}

			if (pPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
			{
				gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
			}
		}

		if (bRemoveSelected)
		{
			if (IsSelected())
			{
				gDLL->getInterfaceIFace()->removeFromSelectionList(this);
			}
		}
	}
}


int CvUnit::getHotKeyNumber()
{
	return m_iHotKeyNumber;
}


void CvUnit::setHotKeyNumber(int iNewValue)
{
	CvUnit* pLoopUnit;
	int iLoop;

	FAssert(getOwnerINLINE() != NO_PLAYER);

	if (getHotKeyNumber() != iNewValue)
	{
		if (iNewValue != -1)
		{
			for(pLoopUnit = GET_PLAYER(getOwnerINLINE()).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER(getOwnerINLINE()).nextUnit(&iLoop))
			{
				if (pLoopUnit->getHotKeyNumber() == iNewValue)
				{
					pLoopUnit->setHotKeyNumber(-1);
				}
			}
		}

		m_iHotKeyNumber = iNewValue;

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}
	}
}


int CvUnit::getX() const
{
	return m_iX;
}


int CvUnit::getY() const
{
	return m_iY;
}


void CvUnit::setXY(int iX, int iY, bool bGroup, bool bUpdate, bool bShow, bool bCheckPlotVisible)
{
	CLLNode<IDInfo>* pUnitNode;
	CvCity* pOldCity;
	CvCity* pNewCity;
	CvCity* pWorkingCity;
	CvUnit* pTransportUnit;
	CvUnit* pLoopUnit;
	CvPlot* pOldPlot;
	CvPlot* pNewPlot;
	CvPlot* pLoopPlot;
	CLinkList<IDInfo> oldUnits;
	ActivityTypes eOldActivityType;
	int iI;

	// OOS!! Temporary for Out-of-Sync madness debugging...
	/*if (GC.getLogging())
	{
		if (gDLL->getChtLvl() > 0)
		{
			char szOut[1024];
			sprintf(szOut, "Player %d Unit %d (%S's %S) moving from %d:%d to %d:%d\n", getOwnerINLINE(), getID(), GET_PLAYER(getOwnerINLINE()).getNameKey(), getName().GetCString(), getX_INLINE(), getY_INLINE(), iX, iY);
			gDLL->messageControlLog(szOut);
		}
	}*/
	if(isOOSLogging())
	{
		CvPlot* pNewLogPlot = GC.getMapINLINE().plotINLINE(iX, iY);
		CvPlot* pOldLogPlot = plot();
		oosLog("AIsetXY"
			,"Turn:%d,PlayerID:%d,Unit:%d,Name:%S,Power:%d,GroupID:%d,AIGroupID:%d,AIGroupType:%S,AIUnitType:%S,fromArea:%d,X:%d,Y:%d,toArea:%d,X:%d,toY:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getOwnerINLINE()
			,getID()
			,getName().GetCString()
			,getPower()
			,getGroupID()
			,getAIGroup()!=NULL ? getAIGroup()->getID() : -1
			,getAIGroup()!=NULL ? GC.getAIGroupInfo(getAIGroup()->getGroupType()).getDescription() : L""
			,AI_getUnitAIType() != NO_UNITAI ? GC.getUnitAIInfo(AI_getUnitAIType()).getDescription() : L""
			,pOldLogPlot != NULL && pOldLogPlot->area() != NULL ? pOldLogPlot->area()->getID() : -MAX_INT
			,getX_INLINE()
			,getY_INLINE()
			,pNewLogPlot != NULL && pNewLogPlot->area() != NULL ? pNewLogPlot->area()->getID() : -MAX_INT
			,iX
			,iY
		);
	}

	FAssert(!at(iX, iY));
	FAssert(!isFighting());
	FAssert((iX == INVALID_PLOT_COORD) || (GC.getMapINLINE().plotINLINE(iX, iY)->getX_INLINE() == iX));
	FAssert((iY == INVALID_PLOT_COORD) || (GC.getMapINLINE().plotINLINE(iX, iY)->getY_INLINE() == iY));

	if (getGroup() != NULL)
	{
		eOldActivityType = getGroup()->getActivityType();
	}
	else
	{
		eOldActivityType = NO_ACTIVITY;
	}

	setBlockading(false);

	if (!bGroup || isCargo())
	{
		joinGroup(NULL, true);
		bShow = false;
	}

	pNewPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (pNewPlot != NULL)
	{
		pTransportUnit = getTransportUnit();

		if (pTransportUnit != NULL)
		{
			if (!(pTransportUnit->atPlot(pNewPlot)))
			{
				setTransportUnit(NULL);
			}
		}

		if (canFight())
		{
			oldUnits.clear();

			pUnitNode = pNewPlot->headUnitNode();

			while (pUnitNode != NULL)
			{
				oldUnits.insertAtEnd(pUnitNode->m_data);
				pUnitNode = pNewPlot->nextUnitNode(pUnitNode);
			}

			pUnitNode = oldUnits.head();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = oldUnits.next(pUnitNode);

				if (pLoopUnit != NULL)
				{
					if (isEnemy(pLoopUnit->getTeam(), pNewPlot) || pLoopUnit->isEnemy(getTeam()))
					{
						if (!pLoopUnit->canCoexistWithEnemyUnit(getTeam()))
						{
							if (NO_UNITCLASS == pLoopUnit->getUnitInfo().getUnitCaptureClassType() && pLoopUnit->canDefend(pNewPlot))
							{

//FfH: Modified by Kael 12/30/2008
//								pLoopUnit->jumpToNearestValidPlot(); // can kill unit

                                if (!isInvisible(pLoopUnit->getTeam(), false))
                                {
                                    pLoopUnit->jumpToNearestValidPlot(); // can kill unit
                                }
//FfH: End Modify

							}
							else
							{

//FfH Hidden Nationality: Modified by Kael 08/27/2007
//								if (!m_pUnitInfo->isHiddenNationality() && !pLoopUnit->getUnitInfo().isHiddenNationality())
								if (!isHiddenNationality() && !pLoopUnit->isHiddenNationality())
//FfH: End Modify

								{
									GET_TEAM(pLoopUnit->getTeam()).changeWarWeariness(getTeam(), *pNewPlot, GC.getDefineINT("WW_UNIT_CAPTURED"));
									GET_TEAM(getTeam()).changeWarWeariness(pLoopUnit->getTeam(), *pNewPlot, GC.getDefineINT("WW_CAPTURED_UNIT"));
									GET_TEAM(getTeam()).AI_changeWarSuccess(pLoopUnit->getTeam(), GC.getDefineINT("WAR_SUCCESS_UNIT_CAPTURING"));
								}

//FfH: Modified by Kael 12/30/2207
//								if (!isNoCapture())
//								{
//									pLoopUnit->setCapturingPlayer(getOwnerINLINE());
//								}
								if (!isNoCapture() || GC.getUnitInfo((UnitTypes)pLoopUnit->getUnitType()).getEquipmentPromotion() != NO_PROMOTION)
								{
								    if (!pLoopUnit->isHiddenNationality())
								    {
                                        pLoopUnit->setCapturingPlayer(getOwnerINLINE());
                                    }
								}
//FfH: End Modify

								pLoopUnit->kill(false, getOwnerINLINE());
							}
						}
					}
				}
			}
		}

		if (pNewPlot->isGoody(getTeam()))
		{
			GET_PLAYER(getOwnerINLINE()).doGoody(pNewPlot, this);
		}
	}

	//added Sephi
	if (isBarbarian() && pNewPlot != NULL &&
		(GC.getUnitClassInfo((UnitClassTypes)GC.getUnitInfo(getUnitType()).getUnitClassType()).getMaxGlobalInstances() == -1))
	{
		pNewCity = pNewPlot->getPlotCity();
		if (pNewCity != NULL) {
			if (isEnemy(pNewCity->getTeam()) && canFight()) {
				if(pNewCity->getPopulation() > 1) {
					pNewCity->changePopulation(-1);
					int iGold = GET_PLAYER(pNewCity->getOwnerINLINE()).getGold() / 5;					
					GET_PLAYER(pNewCity->getOwnerINLINE()).changeGold(-iGold);

					CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_BARBS_INVADE_CITY", pNewCity->getNameKey(), GAMETEXT.getLinkedText((UnitTypes)getUnitType(), true, false).c_str(), iGold);
					gDLL->getInterfaceIFace()->addMessage(pNewCity->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, 0, MESSAGE_TYPE_MAJOR_EVENT, GC.getUnitInfo((UnitTypes)getUnitType()).getButton(), NO_COLOR, getX_INLINE(), getY_INLINE());
					this->kill(true);
					return;
				}
				/**
				if(pNewCity->isCapital()) {
					this->kill(true);
					return;
				} **/
			}
		}
	}
	//end added

	pOldPlot = plot();

	if (pOldPlot != NULL)
	{
		pOldPlot->removeUnit(this, bUpdate && !hasCargo());

		pOldPlot->changeAdjacentSight(getTeam(), visibilityRange(), false, this, true);

		pOldPlot->area()->changeUnitsPerPlayer(getOwnerINLINE(), -1);
//		pOldPlot->area()->changePower(getOwnerINLINE(), -(m_pUnitInfo->getPowerValue())); new Power System
		pOldPlot->area()->changePower(getOwnerINLINE(), -getPower());

		if (AI_getUnitAIType() != NO_UNITAI)
		{
			pOldPlot->area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), -1);
		}

		if (isAnimal())
		{
			pOldPlot->area()->changeAnimalsPerPlayer(getOwnerINLINE(), -1);
		}

		if (pOldPlot->getTeam() != getTeam() && (pOldPlot->getTeam() == NO_TEAM || !GET_TEAM(pOldPlot->getTeam()).isVassal(getTeam())))
		{

//FfH: Modified by Kael 04/19/2009
//			GET_PLAYER(getOwnerINLINE()).changeNumOutsideUnits(-1);
            if (getDuration() == 0 && GC.getUnitInfo((UnitTypes)getUnitType()).isCitySupportNeeded())
            {
				/*if(isOOSLogging())
				{
					oosLog("OutSideUnit"
						,"Turn:%d,Player:%d,Unit:%d,Name:%S,X:%d,Y:%d,numOutsideUnits:%d,setXY:-1"
						,GC.getGameINLINE().getElapsedGameTurns()
						,getOwnerINLINE()
						,getID()
						,getName().GetCString()
						,pOldPlot->getX_INLINE()
						,pOldPlot->getY_INLINE()
						,GET_PLAYER(getOwnerINLINE()).getNumOutsideUnits()
					);
				}*/
                GET_PLAYER(getOwnerINLINE()).changeNumOutsideUnits(-1);
            }
//FfH: End Modify

		}

		setLastMoveTurn(GC.getGameINLINE().getTurnSlice());

		pOldCity = pOldPlot->getPlotCity();

		if (pOldCity != NULL)
		{
			if (isMilitaryHappiness())
			{
				pOldCity->changeMilitaryHappinessUnits(-1);
			}
		}

		pWorkingCity = pOldPlot->getWorkingCity();

		if (pWorkingCity != NULL)
		{
			if (canSiege(pWorkingCity->getTeam()))
			{
				pWorkingCity->AI_setAssignWorkDirty(true);
			}
		}

		if (pOldPlot->isWater())
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pLoopPlot = plotDirection(pOldPlot->getX_INLINE(), pOldPlot->getY_INLINE(), ((DirectionTypes)iI));

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isWater())
					{
						pWorkingCity = pLoopPlot->getWorkingCity();

						if (pWorkingCity != NULL)
						{
							if (canSiege(pWorkingCity->getTeam()))
							{
								pWorkingCity->AI_setAssignWorkDirty(true);
							}
						}
					}
				}
			}
		}

		if (pOldPlot->isActiveVisible(true))
		{
			pOldPlot->updateMinimapColor();
		}

		if (pOldPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->verifyPlotListColumn();

			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}
	}

	if (pNewPlot != NULL)
	{
		m_iX = pNewPlot->getX_INLINE();
		m_iY = pNewPlot->getY_INLINE();
	}
	else
	{
		m_iX = INVALID_PLOT_COORD;
		m_iY = INVALID_PLOT_COORD;
	}

	FAssertMsg(plot() == pNewPlot, "plot is expected to equal pNewPlot");

	if (pNewPlot != NULL)
	{
		pNewCity = pNewPlot->getPlotCity();

		if (pNewCity != NULL)
		{
			if (isEnemy(pNewCity->getTeam()) && !canCoexistWithEnemyUnit(pNewCity->getTeam()) && canFight())
			{
				GET_TEAM(getTeam()).changeWarWeariness(pNewCity->getTeam(), *pNewPlot, GC.getDefineINT("WW_CAPTURED_CITY"));
				GET_TEAM(getTeam()).AI_changeWarSuccess(pNewCity->getTeam(), GC.getDefineINT("WAR_SUCCESS_CITY_CAPTURING"));

				PlayerTypes eNewOwner = GET_PLAYER(getOwnerINLINE()).pickConqueredCityOwner(*pNewCity);

				if (NO_PLAYER != eNewOwner)
				{
					GET_PLAYER(eNewOwner).acquireCity(pNewCity, true, false, true); // will delete the pointer
					pNewCity = NULL;
				}
			}
		}

		// Super Forts begin *culture* *text*
		ImprovementTypes eImprovement = pNewPlot->getImprovementType();
		if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && eImprovement != NO_IMPROVEMENT)
		{
			if(GC.getImprovementInfo(eImprovement).getCulture() > 0 /*isActsAsCity()*/ && !isNoCapture())
			{
				if(pNewPlot->getOwner() != NO_PLAYER)
				{
					if(isEnemy(pNewPlot->getTeam()) && !canCoexistWithEnemyUnit(pNewPlot->getTeam()) && canFight())
					{
						CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_CAPTURED_BY", GC.getImprovementInfo(eImprovement).getText(), GET_PLAYER(getOwnerINLINE()).getCivilizationDescriptionKey());
						gDLL->getInterfaceIFace()->addMessage(pNewPlot->getOwner(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITYCAPTURED", MESSAGE_TYPE_MAJOR_EVENT, GC.getImprovementInfo(eImprovement).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pNewPlot->getX_INLINE(), pNewPlot->getY_INLINE(), true, true);
						pNewPlot->setOwner(getOwnerINLINE(),true,true);
					}
				}
				else
				{
					//if(getOwnerINLINE() < BARBARIAN_PLAYER)
					{
						pNewPlot->setOwner(getOwnerINLINE(),true,true);
					}
				}
			}
		}
		// Super Forts end

		//update facing direction
		if(pOldPlot != NULL)
		{
			DirectionTypes newDirection = estimateDirection(pOldPlot, pNewPlot);
			if(newDirection != NO_DIRECTION)
				m_eFacingDirection = newDirection;
		}

		//update cargo mission animations
		if (isCargo())
		{
			if (eOldActivityType != ACTIVITY_MISSION)
			{
				getGroup()->setActivityType(eOldActivityType);
			}
		}

		setFortifyTurns(0);

		pNewPlot->changeAdjacentSight(getTeam(), visibilityRange(), true, this, true); // needs to be here so that the square is considered visible when we move into it...
		pNewPlot->addUnit(this, bUpdate && !hasCargo());

		pNewPlot->area()->changeUnitsPerPlayer(getOwnerINLINE(), 1);
//		pNewPlot->area()->changePower(getOwnerINLINE(), m_pUnitInfo->getPowerValue()); new Power System
		pNewPlot->area()->changePower(getOwnerINLINE(), getPower());

		if (AI_getUnitAIType() != NO_UNITAI)
		{
			pNewPlot->area()->changeNumAIUnits(getOwnerINLINE(), AI_getUnitAIType(), 1);
		}

		if (isAnimal())
		{
			pNewPlot->area()->changeAnimalsPerPlayer(getOwnerINLINE(), 1);
		}

		if (pNewPlot->getTeam() != getTeam() && (pNewPlot->getTeam() == NO_TEAM || !GET_TEAM(pNewPlot->getTeam()).isVassal(getTeam())))
		{

//FfH: Modified by Kael 04/19/2009
//			GET_PLAYER(getOwnerINLINE()).changeNumOutsideUnits(1);
            if (getDuration() == 0 && GC.getUnitInfo((UnitTypes)getUnitType()).isCitySupportNeeded())
            {
				/*if(isOOSLogging())
				{
					oosLog("OutSideUnit"
						,"Turn:%d,Player:%d,Unit:%d,Name:%S,X:%d,Y:%d,numoutsideunits:%d,setXY:1"
						,GC.getGameINLINE().getElapsedGameTurns()
						,getOwnerINLINE()
						,getID()
						,getName().GetCString()
						,pNewPlot->getX_INLINE()
						,pNewPlot->getY_INLINE()
						,GET_PLAYER(getOwnerINLINE()).getNumOutsideUnits()
					);
				}*/
                GET_PLAYER(getOwnerINLINE()).changeNumOutsideUnits(1);
            }
//FfH: End Modify

		}

		if (shouldLoadOnMove(pNewPlot))
		{
			load();
		}

		for (iI = 0; iI < MAX_CIV_TEAMS; iI++)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (!isInvisible(((TeamTypes)iI), false))
				{
					if (pNewPlot->isVisible((TeamTypes)iI, false))
					{
						GET_TEAM((TeamTypes)iI).meet(getTeam(), true);
					}
				}
			}
		}

		pNewCity = pNewPlot->getPlotCity();

		if (pNewCity != NULL)
		{
			if (isMilitaryHappiness())
			{
				pNewCity->changeMilitaryHappinessUnits(1);
			}
		}

		pWorkingCity = pNewPlot->getWorkingCity();

		if (pWorkingCity != NULL)
		{
			if (canSiege(pWorkingCity->getTeam()))
			{
				pWorkingCity->verifyWorkingPlot(pWorkingCity->getCityPlotIndex(pNewPlot));
			}
		}

		if (pNewPlot->isWater())
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				pLoopPlot = plotDirection(pNewPlot->getX_INLINE(), pNewPlot->getY_INLINE(), ((DirectionTypes)iI));

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isWater())
					{
						pWorkingCity = pLoopPlot->getWorkingCity();

						if (pWorkingCity != NULL)
						{
							if (canSiege(pWorkingCity->getTeam()))
							{
								pWorkingCity->verifyWorkingPlot(pWorkingCity->getCityPlotIndex(pLoopPlot));
							}
						}
					}
				}
			}
		}

		if (pNewPlot->isActiveVisible(true))
		{
			pNewPlot->updateMinimapColor();
		}
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		if (GC.IsGraphicsInitialized())
//		if (GC.IsGraphicsInitialized() && isInViewport())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		{
			//override bShow if check plot visible
			if(bCheckPlotVisible && pNewPlot->isVisibleToWatchingHuman())
				bShow = true;

			if (bShow)
			{
				QueueMove(pNewPlot);
			}
			else
			{
				SetPosition(pNewPlot);
			}
		}

		if (pNewPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->verifyPlotListColumn();

			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}
	}

	if (pOldPlot != NULL)
	{
		if (hasCargo())
		{
			pUnitNode = pOldPlot->headUnitNode();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = pOldPlot->nextUnitNode(pUnitNode);

				if (pLoopUnit->getTransportUnit() == this)
				{
					pLoopUnit->setXY(iX, iY, bGroup, false);
				}
			}
		}
	}

	if (bUpdate && hasCargo())
	{
		if (pOldPlot != NULL)
		{
			pOldPlot->updateCenterUnit();
			pOldPlot->setFlagDirty(true);
		}

		if (pNewPlot != NULL)
		{
			pNewPlot->updateCenterUnit();
			pNewPlot->setFlagDirty(true);
		}
	}

//FfH Units: Added by Kael 11/10/2008
	if (pNewPlot != NULL)
	{
        int iImprovement = pNewPlot->getImprovementType();
        if (iImprovement != NO_IMPROVEMENT)
        {
            if (GC.getImprovementInfo((ImprovementTypes)iImprovement).getSpawnUnitType() != NO_UNIT)
            {
                if (!isHuman() || GC.getImprovementInfo((ImprovementTypes)iImprovement).isPermanent() == false)
                {
/*************************************************************************************************/
/**	ADDON (multibarb) merged Sephi		                                                    	**/
/**	adjusted Spawning from improvements for more barb teams 									**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
/**
                    if (atWar(getTeam(), GET_PLAYER(BARBARIAN_PLAYER).getTeam()))
**/
					int iCiv = GC.getImprovementInfo((ImprovementTypes)iImprovement).getSpawnUnitCiv();
					if(iCiv==GC.getDefineINT("ANIMAL_CIVILIZATION") && atWar(getTeam(), GET_PLAYER(ANIMAL_PLAYER).getTeam()))
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
					{
                        if (isHuman())
                        {
                            gDLL->getInterfaceIFace()->addMessage(getOwner(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), gDLL->getText("TXT_KEY_MESSAGE_LAIR_DESTROYED"), "AS2D_CITYRAZE", MESSAGE_TYPE_MAJOR_EVENT, GC.getImprovementInfo((ImprovementTypes)iImprovement).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pNewPlot->getX(), pNewPlot->getY(), true, true);
                        }
/*************************************************************************************************/
/**	ADDON (exploring Lairs) Sephi		                                                    	**/
/**	                                                         									**/
/** exploring animal lairs gives unique results triggered in python								**/
/*************************************************************************************************/
/** orig
                        pNewPlot->setImprovementType(NO_IMPROVEMENT);
**/
                        CyPlot* pyPlot1 = new CyPlot(plot());
                        CyUnit* pyUnit1 = new CyUnit(this);
                        CyArgsList argsList1;
                        argsList1.add(gDLL->getPythonIFace()->makePythonObject(pyPlot1));	// pass in plot class
                        argsList1.add(getOwnerINLINE());
                        argsList1.add(gDLL->getPythonIFace()->makePythonObject(pyUnit1));	// pass in plot class
                        long lResult=-1;
                        gDLL->getPythonIFace()->callFunction(PYGameModule, "AI_AnimalLairExplored", argsList1.makeFunctionArgs(), &lResult);
                        delete pyPlot1;	// python fxn must not hold on to this pointer
                        delete pyUnit1;	// python fxn must not hold on to this pointer
                        if (lResult==0)
                        {
                            pNewPlot->setImprovementType(NO_IMPROVEMENT);
                        }
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
                    }
                }
            }
        }
        iImprovement = pNewPlot->getImprovementType(); // rechecking because the previous function may have deleted the improvement
        if (iImprovement != NO_IMPROVEMENT)
        {
            if (!CvString(GC.getImprovementInfo((ImprovementTypes)iImprovement).getPythonOnMove()).empty())
            {
                if (pNewPlot->isPythonActive())
                {
                    CyUnit* pyUnit = new CyUnit(this);
                    CyPlot* pyPlot = new CyPlot(pNewPlot);
                    CyArgsList argsList;
                    argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                    argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
                    argsList.add(iImprovement);//the promotion #
                    gDLL->getPythonIFace()->callFunction(PYSpellModule, "onMove", argsList.makeFunctionArgs()); //, &lResult
                    delete pyUnit; // python fxn must not hold on to this pointer
                    delete pyPlot;	// python fxn must not hold on to this pointer
                }
            }
        }
        for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
        {
            pLoopPlot = plotDirection(pNewPlot->getX_INLINE(), pNewPlot->getY_INLINE(), ((DirectionTypes)iI));
            if (pLoopPlot != NULL)
            {
                if (pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
                {
                    if (!CvString(GC.getImprovementInfo((ImprovementTypes)pLoopPlot->getImprovementType()).getPythonAtRange()).empty())
                    {
                        if (pLoopPlot->isPythonActive())
                        {
                            CyUnit* pyUnit = new CyUnit(this);
                            CyPlot* pyPlot = new CyPlot(pLoopPlot);
                            CyArgsList argsList;
                            argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                            argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
                            argsList.add(pLoopPlot->getImprovementType());
                            gDLL->getPythonIFace()->callFunction(PYSpellModule, "atRange", argsList.makeFunctionArgs()); //, &lResult
                            delete pyUnit; // python fxn must not hold on to this pointer
                            delete pyPlot;	// python fxn must not hold on to this pointer
                        }
                    }
                }
            }
        }
        if (pNewPlot->getFeatureType() != NO_FEATURE)
        {
            if (!CvString(GC.getFeatureInfo((FeatureTypes)pNewPlot->getFeatureType()).getPythonOnMove()).empty())
            {
                CyUnit* pyUnit = new CyUnit(this);
                CyPlot* pyPlot = new CyPlot(pNewPlot);
                CyArgsList argsList;
                argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                argsList.add(gDLL->getPythonIFace()->makePythonObject(pyPlot));	// pass in plot class
                argsList.add(pNewPlot->getFeatureType());
                gDLL->getPythonIFace()->callFunction(PYSpellModule, "onMoveFeature", argsList.makeFunctionArgs()); //, &lResult
                delete pyUnit; // python fxn must not hold on to this pointer
                delete pyPlot;	// python fxn must not hold on to this pointer
            }
        }
        if (m_pUnitInfo->isAutoRaze())
        {
            if (pNewPlot->isOwned())
            {
                if (pNewPlot->getImprovementType() != NO_IMPROVEMENT)
                {
                    if (!GC.getImprovementInfo((ImprovementTypes)pNewPlot->getImprovementType()).isPermanent())
                    {
                        if (atWar(getTeam(), GET_PLAYER(pNewPlot->getOwner()).getTeam()))
                        {
                            pNewPlot->setImprovementType(NO_IMPROVEMENT);
                        }
                    }
                }
            }
            pNewPlot->setFeatureType(NO_FEATURE, -1);
        }
	}
//FfH: End Add

	FAssert(pOldPlot != pNewPlot);
	GET_PLAYER(getOwnerINLINE()).updateGroupCycle(this);

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	if ( pNewPlot != NULL &&
		 (((pOldPlot != NULL && pOldPlot->isActiveVisible(false)) != (pNewPlot != NULL && pNewPlot->isActiveVisible(false))) ||
		  ((pOldPlot != NULL && pOldPlot->isInViewport()) != pNewPlot->isInViewport())) )
	{
		reloadEntity();
	}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	setInfoBarDirty(true);

	if (IsSelected())
	{
		if (isFound())
		{
			gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);
			gDLL->getEngineIFace()->updateFoundingBorder();
		}

		gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
	}

	//update glow
	if (pNewPlot != NULL)
	{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		gDLL->getEntityIFace()->updateEnemyGlow(getUnitEntity());
/**
		if (isInViewport())
		{
			gDLL->getEntityIFace()->updateEnemyGlow(getUnitEntity());
		}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	}

	// report event to Python, along with some other key state

/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
	for (int iI = 0; iI < MAX_TEAMS; iI++)
	{
		if (GET_TEAM(getTeam()).isAtWar((TeamTypes)iI))
		{
			for (int iDX = -DANGER_RANGE; iDX <= DANGER_RANGE; iDX++)
			{
				for (int iDY = -DANGER_RANGE; iDY <= DANGER_RANGE; iDY++)
				{
					if (pOldPlot != NULL)
					{
                        int iIndex = GC.getMapINLINE().plotNumINLINE(pOldPlot->getX_INLINE() + iDX, pOldPlot->getY_INLINE() + iDY);
                        if (iIndex > -1 && iIndex < GC.getMapINLINE().numPlotsINLINE())
                        {
                            for (int iJ = 0; iJ < GET_TEAM((TeamTypes)iI).getPlayerMemberListSize(); iJ++)
                            {
                                GET_PLAYER(GET_TEAM((TeamTypes)iI).getPlayerMemberAt(iJ)).AI_invalidatePlotDangerCache(iIndex);
                            }
                        }
					}
					if (pNewPlot != NULL)
					{
                        int iIndex = GC.getMapINLINE().plotNumINLINE(pNewPlot->getX_INLINE() + iDX, pNewPlot->getY_INLINE() + iDY);
                        if (iIndex > -1 && iIndex < GC.getMapINLINE().numPlotsINLINE())
                        {
                            for (int iJ = 0; iJ < GET_TEAM((TeamTypes)iI).getPlayerMemberListSize(); iJ++)
                            {
                                GET_PLAYER(GET_TEAM((TeamTypes)iI).getPlayerMemberAt(iJ)).AI_invalidatePlotDangerCache(iIndex);
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

//FfH: Modified by Kael 10/15/2008
//	CvEventReporter::getInstance().unitSetXY(pNewPlot, this);
	if(GC.getUSE_ON_UNIT_MOVE_CALLBACK())
	{
		CvEventReporter::getInstance().unitSetXY(pNewPlot, this);
	}
//FfH: End Modify

}


bool CvUnit::at(int iX, int iY) const
{
	return((getX_INLINE() == iX) && (getY_INLINE() == iY));
}


bool CvUnit::atPlot(const CvPlot* pPlot) const
{
	return (plot() == pPlot);
}


CvPlot* CvUnit::plot() const
{
	return GC.getMapINLINE().plotSorenINLINE(getX_INLINE(), getY_INLINE());
}


int CvUnit::getArea() const
{
	return plot()->getArea();
}


CvArea* CvUnit::area() const
{
	return plot()->area();
}


bool CvUnit::onMap() const
{
	return (plot() != NULL);
}


int CvUnit::getLastMoveTurn() const
{
	return m_iLastMoveTurn;
}


void CvUnit::setLastMoveTurn(int iNewValue)
{
	m_iLastMoveTurn = iNewValue;
	FAssert(getLastMoveTurn() >= 0);
}


CvPlot* CvUnit::getReconPlot() const
{
	return GC.getMapINLINE().plotSorenINLINE(m_iReconX, m_iReconY);
}


void CvUnit::setReconPlot(CvPlot* pNewValue)
{
	CvPlot* pOldPlot;

	pOldPlot = getReconPlot();

	if (pOldPlot != pNewValue)
	{
		if (pOldPlot != NULL)
		{
			pOldPlot->changeAdjacentSight(getTeam(), GC.getDefineINT("RECON_VISIBILITY_RANGE"), false, this, true);
			pOldPlot->changeReconCount(-1); // changeAdjacentSight() tests for getReconCount()
		}

		if (pNewValue == NULL)
		{
			m_iReconX = INVALID_PLOT_COORD;
			m_iReconY = INVALID_PLOT_COORD;
		}
		else
		{
			m_iReconX = pNewValue->getX_INLINE();
			m_iReconY = pNewValue->getY_INLINE();

			pNewValue->changeReconCount(1); // changeAdjacentSight() tests for getReconCount()
			pNewValue->changeAdjacentSight(getTeam(), GC.getDefineINT("RECON_VISIBILITY_RANGE"), true, this, true);
		}
	}
}


int CvUnit::getGameTurnCreated() const
{
	return m_iGameTurnCreated;
}


void CvUnit::setGameTurnCreated(int iNewValue)
{
	m_iGameTurnCreated = iNewValue;
	FAssert(getGameTurnCreated() >= 0);
}


int CvUnit::getDamage() const
{
	return m_iDamage;
}


void CvUnit::setDamage(int iNewValue, PlayerTypes ePlayer, bool bNotifyEntity)
{
	int iOldValue;

	iOldValue = getDamage();

	m_iDamage = range(iNewValue, 0, maxHitPoints());

	FAssertMsg(currHitPoints() >= 0, "currHitPoints() is expected to be non-negative (invalid Index)");

	if (iOldValue != getDamage())
	{
		if (GC.getGameINLINE().isFinalInitialized() && bNotifyEntity)
		{
			NotifyEntity(MISSION_DAMAGE);
		}

		setInfoBarDirty(true);

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}

		if (plot() == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}
	}

	if (isDead())
	{
		kill(true, ePlayer);
	}
}


void CvUnit::changeDamage(int iChange, PlayerTypes ePlayer)
{
	setDamage((getDamage() + iChange), ePlayer);
}


int CvUnit::getMoves() const
{
	return m_iMoves;
}


void CvUnit::setMoves(int iNewValue)
{
	CvPlot* pPlot;

	if (getMoves() != iNewValue)
	{
		pPlot = plot();

		m_iMoves = iNewValue;

//>>>>BUGFfH: Deleted by Denev 2009/09/29
/*	Eliminate Hasting micromanagement	*/
//		FAssert(getMoves() >= 0);
//<<<<BUGFfH: End Delete


		if (getTeam() == GC.getGameINLINE().getActiveTeam())
		{
			if (pPlot != NULL)
			{
				pPlot->setFlagDirty(true);
			}
		}

		if (IsSelected())
		{
			gDLL->getFAStarIFace()->ForceReset(&GC.getInterfacePathFinder());

			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}

		if (pPlot == gDLL->getInterfaceIFace()->getSelectionPlot())
		{
			gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
		}

/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
       if (GC.getGameINLINE().isFinalInitialized())
        {
            for (int iI = 0; iI < MAX_TEAMS; iI++)
            {
                if (GET_TEAM(getTeam()).isAtWar((TeamTypes)iI))
                {
                    for (int iDX = -DANGER_RANGE; iDX <= DANGER_RANGE; iDX++)
                    {
                        for (int iDY = -DANGER_RANGE; iDY <= DANGER_RANGE; iDY++)
                        {
                            int iIndex = GC.getMapINLINE().plotNumINLINE(plot()->getX_INLINE() + iDX, plot()->getY_INLINE() + iDY);
                            if (iIndex > -1 && iIndex < GC.getMapINLINE().numPlotsINLINE())
                            {
                                for (int iJ = 0; iJ < GET_TEAM((TeamTypes)iI).getPlayerMemberListSize(); iJ++)
                                {
                                    GET_PLAYER(GET_TEAM((TeamTypes)iI).getPlayerMemberAt(iJ)).AI_invalidatePlotDangerCache(iIndex);
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
	}
}


void CvUnit::changeMoves(int iChange)
{
	setMoves(getMoves() + iChange);
}


void CvUnit::finishMoves()
{
	setMoves(maxMoves());
}


int CvUnit::getExperience() const
{
	return m_iExperience;
}

void CvUnit::setExperience(int iNewValue, int iMax)
{
	if ((getExperience() != iNewValue) && (getExperience() < ((iMax == -1) ? MAX_INT : iMax)))
	{
		m_iExperience = std::min(((iMax == -1) ? MAX_INT : iMax), iNewValue);
		FAssert(getExperience() >= -100);	//Events can take away xp.  Assert that the value isn't something crazy like -100 xp.

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}
	}
}

void CvUnit::changeExperience(int iChange, int iMax, bool bFromCombat, bool bInBorders, bool bUpdateGlobal)
{
	//no XP?
	if(isNoXP() || GC.getUnitInfo(getUnitType()).getUnitCombatType()==NO_UNITCOMBAT)
	{
		return;
	}
	int iUnitExperience = iChange;

	if (bFromCombat)
	{
		CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());

		int iCombatExperienceMod = kPlayer.getGreatGeneralRateModifier();

		if (bInBorders)
		{
			iCombatExperienceMod += kPlayer.getDomesticGreatGeneralRateModifier() + kPlayer.getExpInBorderModifier();
			iUnitExperience += (iChange * kPlayer.getExpInBorderModifier()) / 100;
		}

		if (bUpdateGlobal && kPlayer.getGreatGeneralRateModifier()>0)
		{
			kPlayer.changeCombatExperience((iChange * iCombatExperienceMod) / 100);
		}

		if (getExperiencePercent() != 0)
		{
			iUnitExperience *= std::max(0, 100 + getExperiencePercent());
			iUnitExperience /= 100;
		}

		if(getExperience()<30)
		{
            iUnitExperience *= 250;
            iUnitExperience /= 100;
            iUnitExperience += 3;
		}
//FfH: Added by Kael 05/17/2008
        if (GC.getGameINLINE().isOption(GAMEOPTION_SLOWER_XP))
        {
            iUnitExperience /= 2;
        }
//FfH: End Add
		if (isHasSecondChance())
		{
            iUnitExperience /= 2;
		}
/*************************************************************************************************/
/**	ADDON (unit XP tweak)  Sephi	                                                            **/
/**	make sure unit always gets atleast 1XP from Combat  										**/
/*************************************************************************************************/
		iUnitExperience = std::max(GC.getDefineINT("MIN_EXPERIENCE_PER_COMBAT"), iUnitExperience);
		if (isBlitz())
			iUnitExperience /= 3;

/*************************************************************************************************/
/**	ADDON(BATTLECAPTAINS) Sephi					                                **/
/*************************************************************************************************/
		CvUnit* pCaptain;
		for(int iI=0;iI<2;iI++)
		{
			pCaptain=NULL;
			if(iI==0 && getBlessing1()!=NULL)
				pCaptain=getBlessing1()->getUnit();
			if(iI==1 && getBlessing2()!=NULL)
				pCaptain=getBlessing2()->getUnit();

			if(pCaptain!=NULL && GC.getUnitClassInfo(pCaptain->getUnitClassType()).isBattleCaptain())
			{
				if(pCaptain!=this)
				{
					pCaptain->changeExperience(1);
				}
			}
		}
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/

	}

	setExperience((getExperience() + iUnitExperience), iMax);

	if(m_pUnitInfo->getUnitCombatType() != NO_UNITCOMBAT) {
		if(GC.getUnitCombatInfo((UnitCombatTypes)m_pUnitInfo->getUnitCombatType()).isNoPromotions()) {
			while(getExperience() >= experienceNeeded()) {
				changeLevel(1);
			}
		}
	}
}

int CvUnit::getLevel() const
{
	return m_iLevel;
}

void CvUnit::setLevel(int iNewValue)
{
	if (getLevel() != iNewValue)
	{
/*************************************************************************************************/
/**	ADDON (Combatauras) Sephi																	**/
/*************************************************************************************************/
		if(getCombatAura()!=NULL)
		{
			getCombatAura()->changePower(4 * (iNewValue-getLevel()));
		}

		//At Level 15, allow other Disciplines again
		if(iNewValue==15)
		{
			for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if(GC.getPromotionInfo((PromotionTypes)iJ).isClassPromotion())
				{
					if(isDenyPromotion((PromotionTypes)iJ))
					{
						changeDenyPromotion((PromotionTypes)iJ,-MAX_INT);
					}
				}
			}
		}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
		m_iLevel = iNewValue;
		FAssert(getLevel() >= 0);

		if (getLevel() > GET_PLAYER(getOwnerINLINE()).getHighestUnitLevel())
		{
			GET_PLAYER(getOwnerINLINE()).setHighestUnitLevel(getLevel());
		}

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bUnitLevel                                                                              **/
/*************************************************************************************************/
        UnitTypes eUnit=getUnitType();
        for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
        {
            if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
            {
                CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
                for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
                {
					if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
					{
					    if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getUnitLevel()>0)
					    {
                            if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getUnitLevel()==iNewValue && GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getUnitType()==eUnit)
                            {
                                GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
                            }
					    }
					}
                }
            }
        }
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/
		for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
		{
			if(isHasPromotion((PromotionTypes)iI))
			{
				if(GC.getPromotionInfo((PromotionTypes)iI).getMaxLevel()!=0)
				{
					if(GC.getPromotionInfo((PromotionTypes)iI).getMaxLevel()<=m_iLevel)
					{
						setHasPromotion((PromotionTypes)iI,false);
					}
				}
			}
		}
	}
}

void CvUnit::changeLevel(int iChange)
{
	setLevel(getLevel() + iChange);
}

int CvUnit::getCargo() const
{
	return m_iCargo;
}

void CvUnit::changeCargo(int iChange)
{
	m_iCargo += iChange;
	FAssert(getCargo() >= 0);
}

void CvUnit::getCargoUnits(std::vector<CvUnit*>& aUnits) const
{
	aUnits.clear();

	if (hasCargo())
	{
		CvPlot* pPlot = plot();
		CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
		while (pUnitNode != NULL)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pPlot->nextUnitNode(pUnitNode);
			if (pLoopUnit->getTransportUnit() == this)
			{
				aUnits.push_back(pLoopUnit);
			}
		}
	}

	FAssert(getCargo() == aUnits.size());
}

CvPlot* CvUnit::getAttackPlot() const
{
	return GC.getMapINLINE().plotSorenINLINE(m_iAttackPlotX, m_iAttackPlotY);
}


void CvUnit::setAttackPlot(const CvPlot* pNewValue, bool bAirCombat)
{
	if (getAttackPlot() != pNewValue)
	{
		if (pNewValue != NULL)
		{
			m_iAttackPlotX = pNewValue->getX_INLINE();
			m_iAttackPlotY = pNewValue->getY_INLINE();
		}
		else
		{
			m_iAttackPlotX = INVALID_PLOT_COORD;
			m_iAttackPlotY = INVALID_PLOT_COORD;
		}
	}

	m_bAirCombat = bAirCombat;
}

bool CvUnit::isAirCombat() const
{
	return m_bAirCombat;
}

int CvUnit::getCombatTimer() const
{
	return m_iCombatTimer;
}

void CvUnit::setCombatTimer(int iNewValue)
{
	m_iCombatTimer = iNewValue;
	FAssert(getCombatTimer() >= 0);
}

void CvUnit::changeCombatTimer(int iChange)
{
	setCombatTimer(getCombatTimer() + iChange);
}

int CvUnit::getCombatFirstStrikes() const
{
	return m_iCombatFirstStrikes;
}

void CvUnit::setCombatFirstStrikes(int iNewValue)
{
	m_iCombatFirstStrikes = iNewValue;
	FAssert(getCombatFirstStrikes() >= 0);
}

void CvUnit::changeCombatFirstStrikes(int iChange)
{
	setCombatFirstStrikes(getCombatFirstStrikes() + iChange);
}

int CvUnit::getFortifyTurns() const
{
	return m_iFortifyTurns;
}

void CvUnit::setFortifyTurns(int iNewValue)
{
	iNewValue = range(iNewValue, 0, GC.getDefineINT("MAX_FORTIFY_TURNS"));

	if (iNewValue != getFortifyTurns())
	{
		m_iFortifyTurns = iNewValue;
		setInfoBarDirty(true);
	}
}

void CvUnit::changeFortifyTurns(int iChange)
{
	setFortifyTurns(getFortifyTurns() + iChange);
}

int CvUnit::getBlitzCount() const
{
	return m_iBlitzCount;
}

bool CvUnit::isBlitz() const
{
	return (getBlitzCount() > 0);
}

void CvUnit::changeBlitzCount(int iChange)
{
	m_iBlitzCount += iChange;
	FAssert(getBlitzCount() >= 0);
}

int CvUnit::getAmphibCount() const
{
	return m_iAmphibCount;
}

bool CvUnit::isAmphib() const
{
	return (getAmphibCount() > 0);
}

void CvUnit::changeAmphibCount(int iChange)
{
	m_iAmphibCount += iChange;
	FAssert(getAmphibCount() >= 0);
}

int CvUnit::getRiverCount() const
{
	return m_iRiverCount;
}

bool CvUnit::isRiver() const
{
	return (getRiverCount() > 0);
}

void CvUnit::changeRiverCount(int iChange)
{
	m_iRiverCount += iChange;
	FAssert(getRiverCount() >= 0);
}

int CvUnit::getEnemyRouteCount() const
{
	return m_iEnemyRouteCount;
}

bool CvUnit::isEnemyRoute() const
{
	return (getEnemyRouteCount() > 0);
}

void CvUnit::changeEnemyRouteCount(int iChange)
{
	m_iEnemyRouteCount += iChange;
	FAssert(getEnemyRouteCount() >= 0);
}

int CvUnit::getAlwaysHealCount() const
{
	return m_iAlwaysHealCount;
}

bool CvUnit::isAlwaysHeal() const
{
	return (getAlwaysHealCount() > 0);
}

void CvUnit::changeAlwaysHealCount(int iChange)
{
	m_iAlwaysHealCount += iChange;
	FAssert(getAlwaysHealCount() >= 0);
}

int CvUnit::getHillsDoubleMoveCount() const
{
	return m_iHillsDoubleMoveCount;
}

bool CvUnit::isHillsDoubleMove() const
{
	return (getHillsDoubleMoveCount() > 0);
}

void CvUnit::changeHillsDoubleMoveCount(int iChange)
{
	m_iHillsDoubleMoveCount += iChange;
	FAssert(getHillsDoubleMoveCount() >= 0);
}

int CvUnit::getImmuneToFirstStrikesCount() const
{
	return m_iImmuneToFirstStrikesCount;
}

void CvUnit::changeImmuneToFirstStrikesCount(int iChange)
{
	m_iImmuneToFirstStrikesCount += iChange;
	FAssert(getImmuneToFirstStrikesCount() >= 0);
}


int CvUnit::getExtraVisibilityRange() const
{
	return m_iExtraVisibilityRange;
}

void CvUnit::changeExtraVisibilityRange(int iChange)
{
	if (iChange != 0)
	{
		plot()->changeAdjacentSight(getTeam(), visibilityRange(), false, this, true);

		m_iExtraVisibilityRange += iChange;
//		FAssert(getExtraVisibilityRange() >= 0);	some promotions can reduce visibility modified Sephi

		plot()->changeAdjacentSight(getTeam(), visibilityRange(), true, this, true);
	}
}

int CvUnit::getExtraMoves() const
{
	return m_iExtraMoves;
}


void CvUnit::changeExtraMoves(int iChange)
{
	m_iExtraMoves += iChange;
//	FAssert(getExtraMoves() >= 0); mutations can make this negative modified Sephi
}


int CvUnit::getExtraMoveDiscount() const
{
	return m_iExtraMoveDiscount;
}


void CvUnit::changeExtraMoveDiscount(int iChange)
{
	m_iExtraMoveDiscount += iChange;
//	FAssert(getExtraMoveDiscount() >= 0); //mutate can give negative value Sephi
}

int CvUnit::getExtraAirRange() const
{
	return m_iExtraAirRange;
}

void CvUnit::changeExtraAirRange(int iChange)
{
	m_iExtraAirRange += iChange;
}

int CvUnit::getExtraIntercept() const
{
	return m_iExtraIntercept;
}

void CvUnit::changeExtraIntercept(int iChange)
{
	m_iExtraIntercept += iChange;
}

int CvUnit::getExtraEvasion() const
{
	return m_iExtraEvasion;
}

void CvUnit::changeExtraEvasion(int iChange)
{
	m_iExtraEvasion += iChange;
}

int CvUnit::getExtraFirstStrikes() const
{
	return m_iExtraFirstStrikes;
}

void CvUnit::changeExtraFirstStrikes(int iChange)
{
	m_iExtraFirstStrikes += iChange;
//	FAssert(getExtraFirstStrikes() >= 0); Sephi
}

int CvUnit::getExtraChanceFirstStrikes() const
{
	return m_iExtraChanceFirstStrikes;
}

void CvUnit::changeExtraChanceFirstStrikes(int iChange)
{
	m_iExtraChanceFirstStrikes += iChange;
	FAssert(getExtraChanceFirstStrikes() >= 0);
}


int CvUnit::getExtraWithdrawal() const
{
	return m_iExtraWithdrawal;
}


void CvUnit::changeExtraWithdrawal(int iChange)
{
	m_iExtraWithdrawal += iChange;
//	FAssert(getExtraWithdrawal() >= 0);	//The heavy armor trait such as on scalemail has a negative withdrawal rate.
}

int CvUnit::getExtraCollateralDamage() const
{
	return m_iExtraCollateralDamage;
}

void CvUnit::changeExtraCollateralDamage(int iChange)
{
	m_iExtraCollateralDamage += iChange;
	FAssert(getExtraCollateralDamage() >= 0);
}

int CvUnit::getExtraBombardRate() const
{
	return m_iExtraBombardRate;
}

void CvUnit::changeExtraBombardRate(int iChange)
{
	m_iExtraBombardRate += iChange;
	FAssert(getExtraBombardRate() >= 0);
}

int CvUnit::getExtraEnemyHeal() const
{
	return m_iExtraEnemyHeal;
}

void CvUnit::changeExtraEnemyHeal(int iChange)
{
	m_iExtraEnemyHeal += iChange;
}

int CvUnit::getExtraNeutralHeal() const
{
	return m_iExtraNeutralHeal;
}

void CvUnit::changeExtraNeutralHeal(int iChange)
{
	m_iExtraNeutralHeal += iChange;
}

int CvUnit::getExtraFriendlyHeal() const
{
	return m_iExtraFriendlyHeal;
}


void CvUnit::changeExtraFriendlyHeal(int iChange)
{
	m_iExtraFriendlyHeal += iChange;
}

int CvUnit::getSameTileHeal() const
{
	return m_iSameTileHeal;
}

void CvUnit::changeSameTileHeal(int iChange)
{
	m_iSameTileHeal += iChange;
	FAssert(getSameTileHeal() >= 0);
}

int CvUnit::getAdjacentTileHeal() const
{
	return m_iAdjacentTileHeal;
}

void CvUnit::changeAdjacentTileHeal(int iChange)
{
	m_iAdjacentTileHeal += iChange;
	FAssert(getAdjacentTileHeal() >= 0);
}

int CvUnit::getExtraCombatPercent() const
{

//FfH: Modified by Kael 10/26/2007
//	return m_iExtraCombatPercent;
	int i = m_iExtraCombatPercent;
    if (plot()->getOwnerINLINE() == getOwnerINLINE())
    {
        i += getCombatPercentInBorders();
    }
	return i;
//FfH: End Modify

}

void CvUnit::changeExtraCombatPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraCombatPercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraCityAttackPercent() const
{
	return m_iExtraCityAttackPercent;
}

void CvUnit::changeExtraCityAttackPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraCityAttackPercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraCityDefensePercent() const
{
	return m_iExtraCityDefensePercent;
}

void CvUnit::changeExtraCityDefensePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraCityDefensePercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraHillsAttackPercent() const
{
	return m_iExtraHillsAttackPercent;
}

void CvUnit::changeExtraHillsAttackPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraHillsAttackPercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraHillsDefensePercent() const
{
	return m_iExtraHillsDefensePercent;
}

void CvUnit::changeExtraHillsDefensePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExtraHillsDefensePercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getRevoltProtection() const
{
	return m_iRevoltProtection;
}

void CvUnit::changeRevoltProtection(int iChange)
{
	if (iChange != 0)
	{
		m_iRevoltProtection += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getCollateralDamageProtection() const
{
	return m_iCollateralDamageProtection;
}

void CvUnit::changeCollateralDamageProtection(int iChange)
{
	if (iChange != 0)
	{
		m_iCollateralDamageProtection += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getPillageChange() const
{
	return m_iPillageChange;
}

void CvUnit::changePillageChange(int iChange)
{
	if (iChange != 0)
	{
		m_iPillageChange += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getUpgradeDiscount() const
{
	return m_iUpgradeDiscount;
}

void CvUnit::changeUpgradeDiscount(int iChange)
{
	if (iChange != 0)
	{
		m_iUpgradeDiscount += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExperiencePercent() const
{
	return m_iExperiencePercent;
}

void CvUnit::changeExperiencePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iExperiencePercent += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getKamikazePercent() const
{
	return m_iKamikazePercent;
}

void CvUnit::changeKamikazePercent(int iChange)
{
	if (iChange != 0)
	{
		m_iKamikazePercent += iChange;

		setInfoBarDirty(true);
	}
}

DirectionTypes CvUnit::getFacingDirection(bool checkLineOfSightProperty) const
{
	if (checkLineOfSightProperty)
	{
		if (m_pUnitInfo->isLineOfSight())
		{
			return m_eFacingDirection; //only look in facing direction
		}
		else
		{
			return NO_DIRECTION; //look in all directions
		}
	}
	else
	{
		return m_eFacingDirection;
	}
}

void CvUnit::setFacingDirection(DirectionTypes eFacingDirection)
{
	if (eFacingDirection != m_eFacingDirection)
	{
		if (m_pUnitInfo->isLineOfSight())
		{
			//remove old fog
			plot()->changeAdjacentSight(getTeam(), visibilityRange(), false, this, true);

			//change direction
			m_eFacingDirection = eFacingDirection;

			//clear new fog
			plot()->changeAdjacentSight(getTeam(), visibilityRange(), true, this, true);

			gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
		}
		else
		{
			m_eFacingDirection = eFacingDirection;
		}

		//update formation
		NotifyEntity(NO_MISSION);
	}
}

void CvUnit::rotateFacingDirectionClockwise()
{
	//change direction
	DirectionTypes eNewDirection = (DirectionTypes) ((m_eFacingDirection + 1) % NUM_DIRECTION_TYPES);
	setFacingDirection(eNewDirection);
}

void CvUnit::rotateFacingDirectionCounterClockwise()
{
	//change direction
	DirectionTypes eNewDirection = (DirectionTypes) ((m_eFacingDirection + NUM_DIRECTION_TYPES - 1) % NUM_DIRECTION_TYPES);
	setFacingDirection(eNewDirection);
}

int CvUnit::getImmobileTimer() const
{

//FfH: Added by Kael 09/15/2008
    if (isHeld())
    {
//>>>>Advanced Rules: Modified by Denev 2009/11/10
//		return 999;
		return MAX_INT;
//<<<<Advanced Rules: End Modify
    }
//FfH: End Add

	return m_iImmobileTimer;
}

void CvUnit::setImmobileTimer(int iNewValue)
{

//FfH: Modified by Kael 09/15/2008
//	if (iNewValue != getImmobileTimer())
//	{
//		m_iImmobileTimer = iNewValue;
//		setInfoBarDirty(true);
	if (iNewValue != getImmobileTimer() && !isHeld())
	{
		m_iImmobileTimer = iNewValue;
		setInfoBarDirty(true);
        if (getImmobileTimer() == 0)
        {
            if (getDelayedSpell() != NO_SPELL)
            {
                cast(getDelayedSpell());
            }
        }
//FfH: End Modify

	}
}

void CvUnit::changeImmobileTimer(int iChange)
{
	if (iChange != 0)
	{
		setImmobileTimer(std::max(0, getImmobileTimer() + iChange));
	}
}

bool CvUnit::isMadeAttack() const
{
	return m_bMadeAttack;
}


void CvUnit::setMadeAttack(bool bNewValue)
{
	m_bMadeAttack = bNewValue;
}


bool CvUnit::isMadeInterception() const
{
	return m_bMadeInterception;
}


void CvUnit::setMadeInterception(bool bNewValue)
{
	m_bMadeInterception = bNewValue;
}


bool CvUnit::isPromotionReady() const
{
	return m_bPromotionReady;
}


void CvUnit::setPromotionReady(bool bNewValue)
{
	if (isPromotionReady() != bNewValue)
	{
		m_bPromotionReady = bNewValue;
/*************************************************************************************************/
/**	ADDON (automatic terraforming) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
/** Orig
        if (m_bPromotionReady)
**/

		if ((m_bPromotionReady && AI_getUnitAIType()!=UNITAI_TERRAFORMER))
/*************************************************************************************************/
/**	END                                                                  						**/
/*************************************************************************************************/
		{
			getGroup()->setAutomateType(NO_AUTOMATE);
			getGroup()->clearMissionQueue();
			getGroup()->setActivityType(ACTIVITY_AWAKE);
		}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		gDLL->getEntityIFace()->showPromotionGlow(getUnitEntity(), bNewValue);
/**
		if (isInViewport())
		{
			gDLL->getEntityIFace()->showPromotionGlow(getUnitEntity(), bNewValue);
		}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		}
	}
}


void CvUnit::testPromotionReady()
{
	setPromotionReady((getExperience() >= experienceNeeded()) && canAcquirePromotionAny());
	//SpyFanatic: count also free promotion
	//setPromotionReady((getExperience() >= experienceNeeded() || (getFreePromotionPick() > 0 && getLevel()>3)) && canAcquirePromotionAny());
}


bool CvUnit::isDelayedDeath() const
{
	return m_bDeathDelay;
}


void CvUnit::startDelayedDeath()
{
	m_bDeathDelay = true;
}


// Returns true if killed...
bool CvUnit::doDelayedDeath()
{
	if (m_bDeathDelay && !isFighting())
	{
		/*oosLog(
			"AIsetXY"
			,"UnitID: %d,doDelayedDeath"
			,getID()

		);*/
		kill(false);
		return true;
	}

	return false;
}


bool CvUnit::isCombatFocus() const
{
	return m_bCombatFocus;
}


bool CvUnit::isInfoBarDirty() const
{
	return m_bInfoBarDirty;
}


void CvUnit::setInfoBarDirty(bool bNewValue)
{
	m_bInfoBarDirty = bNewValue;
}

bool CvUnit::isBlockading() const
{
	return m_bBlockading;
}

void CvUnit::setBlockading(bool bNewValue)
{
	if (bNewValue != isBlockading())
	{
		m_bBlockading = bNewValue;

		updatePlunder(isBlockading() ? 1 : -1, true);
	}
}

void CvUnit::collectBlockadeGold()
{
	if (plot()->getTeam() == getTeam())
	{
		return;
	}

	int iBlockadeRange = GC.getDefineINT("SHIP_BLOCKADE_RANGE");

	for (int i = -iBlockadeRange; i <= iBlockadeRange; ++i)
	{
		for (int j = -iBlockadeRange; j <= iBlockadeRange; ++j)
		{
			CvPlot* pLoopPlot = ::plotXY(getX_INLINE(), getY_INLINE(), i, j);

			if (NULL != pLoopPlot && pLoopPlot->isRevealed(getTeam(), false))
			{
				CvCity* pCity = pLoopPlot->getPlotCity();

				if (NULL != pCity && !pCity->isPlundered() && isEnemy(pCity->getTeam()) && !atWar(pCity->getTeam(), getTeam()))
				{
					if (pCity->area() == area() || pCity->plot()->isAdjacentToArea(area()))
					{
						int iGold = pCity->calculateTradeProfit(pCity) * pCity->getTradeRoutes();
						if (iGold > 0)
						{
							pCity->setPlundered(true);
							GET_PLAYER(getOwnerINLINE()).changeGold(iGold);
							GET_PLAYER(pCity->getOwnerINLINE()).changeGold(-iGold);

							CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_TRADE_ROUTE_PLUNDERED", getNameKey(), pCity->getNameKey(), iGold);
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BUILD_BANK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE());

							szBuffer = gDLL->getText("TXT_KEY_MISC_TRADE_ROUTE_PLUNDER", getNameKey(), pCity->getNameKey(), iGold);
							gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_BUILD_BANK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCity->getX_INLINE(), pCity->getY_INLINE());

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag isPlunderGold                                                                           **/
/*************************************************************************************************/
                            for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
                            {
                                if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
                                {
                                    CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
                                    for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
                                    {
                                        if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
                                        {
                                            if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isPlunderGold())
                                            {
                                                GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,iGold);
                                            }
                                        }
                                    }
                                }
                            }
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/

						}
					}
				}
			}
		}
	}
}


PlayerTypes CvUnit::getOwner() const
{
	return getOwnerINLINE();
}

PlayerTypes CvUnit::getVisualOwner(TeamTypes eForTeam) const
{
	if (NO_TEAM == eForTeam)
	{
		eForTeam = GC.getGameINLINE().getActiveTeam();
	}

/*************************************************************************************************/
/**	ADDON (MultiBarb) merged Sephi		12/23/08									Xienwolf	**/
/**	adjusted for more barb teams    															**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	if (getTeam() != eForTeam && eForTeam != BARBARIAN_TEAM)
	{

//FfH Hidden Nationality: Modified by Kael 08/27/2007
//		if (m_pUnitInfo->isHiddenNationality())
		if (isHiddenNationality())
//FfH: End Modify

		{
			if (!plot()->isCity(true, getTeam()))
			{
				return BARBARIAN_PLAYER;
			}
		}
	}
/**								----  End Original Code  ----									**/
	if (
        getTeam() != eForTeam && eForTeam != BARBARIAN_TEAM && eForTeam != ANIMAL_TEAM && eForTeam != WILDMANA_TEAM &&
        eForTeam!= PIRATES_TEAM && eForTeam != WHITEHAND_TEAM && eForTeam != DEVIL_TEAM && eForTeam != MERCENARIES_TEAM
        )
	{
		if (isHiddenNationality())
		{
			if (!plot()->isCity(true, getTeam()))
			{
			    if(getDomainType()==DOMAIN_SEA)
			    {
			        return PIRATES_PLAYER;
			    }
			    else if(isAnimal())
			    {
			        return ANIMAL_PLAYER;
			    }
			    else
			    {
                    return BARBARIAN_PLAYER;
			    }

			}
		}
	}
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/

	return getOwnerINLINE();
}


//>>>>Advanced Rules: Comment by Denev 2009/10/19
//This function lies that "always hostile" unit is a barbarians civilization, even if the unit is not a barbarians.
//Some leader has a barbarian trait, and it's not always true that barbarians is hostile.
//Therefore, this function is obsoleted and MUST NOT BE CALLED in FfH2.
//Sephi modify it so it returns Wildmana for barbarian Ally players
PlayerTypes CvUnit::getCombatOwner(TeamTypes eForTeam, const CvPlot* pPlot) const
{
	if (eForTeam != NO_TEAM && getTeam() != eForTeam && eForTeam != BARBARIAN_TEAM)
	{
		if (isAlwaysHostile(pPlot))
		{
		    //added Sephi
		    if(GET_TEAM(getTeam()).isBarbarianAlly())
		    {
		        return WILDMANA_PLAYER;
		    }
		    //added

			return BARBARIAN_PLAYER;
		}
	}

	return getOwnerINLINE();
}
//<<<<Advanced Rules: End Comment

TeamTypes CvUnit::getTeam() const
{
	return GET_PLAYER(getOwnerINLINE()).getTeam();
}


PlayerTypes CvUnit::getCapturingPlayer() const
{
	return m_eCapturingPlayer;
}


void CvUnit::setCapturingPlayer(PlayerTypes eNewValue)
{

//FfH: Modified by Kael 08/12/2007
//	m_eCapturingPlayer = eNewValue;
    if (!isImmuneToCapture())
    {
        m_eCapturingPlayer = eNewValue;
    }
//FfH: End Add

}
//FfH: Added by Kael
bool CvUnit::withdrawlToNearestValidPlot()
{
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot = NULL;
	int iValue;
	int iBestValue = MAX_INT;
	for (int iI = -1; iI <= 1; ++iI)
	{
		for (int iJ = -1; iJ <= 1; ++iJ)
		{
			pLoopPlot = ::plotXY(getX_INLINE(), getY_INLINE(), iI, iJ);
			if (NULL != pLoopPlot)
			{
                if (pLoopPlot->isValidDomainForLocation(*this))
                {
                    if (canMoveInto(pLoopPlot))
                    {
                        iValue = (plotDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) * 2);
                        if (getDomainType() == DOMAIN_SEA && !plot()->isWater())
                        {
                            if (!pLoopPlot->isWater() || !pLoopPlot->isAdjacentToArea(area()))
                            {
                                iValue *= 3;
                            }
                        }
                        else
                        {
                            if (pLoopPlot->area() != area())
                            {
                                iValue *= 3;
                            }
                        }
                        if (iValue < iBestValue)
                        {
                            iBestValue = iValue;
                            pBestPlot = pLoopPlot;
                        }
					}
				}
			}
		}
	}
	bool bValid = true;
	if (pBestPlot != NULL)
	{
		setXY(pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE());
	}
	else
	{
		kill(true);	//modified Sephi
		bValid = false;
	}
	return bValid;
}
//FfH: End Add

const UnitTypes CvUnit::getUnitType() const
{
	return m_eUnitType;
}

CvUnitInfo &CvUnit::getUnitInfo() const
{
	return *m_pUnitInfo;
}


UnitClassTypes CvUnit::getUnitClassType() const
{
	return (UnitClassTypes)m_pUnitInfo->getUnitClassType();
}

const UnitTypes CvUnit::getLeaderUnitType() const
{
	return m_eLeaderUnitType;
}

void CvUnit::setLeaderUnitType(UnitTypes leaderUnitType)
{
	if(m_eLeaderUnitType != leaderUnitType)
	{
		m_eLeaderUnitType = leaderUnitType;
		reloadEntity();
	}
}

CvUnit* CvUnit::getCombatUnit() const
{
	return getUnit(m_combatUnit);
}


void CvUnit::setCombatUnit(CvUnit* pCombatUnit, bool bAttacking)
{
	if (isCombatFocus())
	{
		gDLL->getInterfaceIFace()->setCombatFocus(false);
	}

	if (pCombatUnit != NULL)
	{
		if (bAttacking)
		{
			if (GC.getLogging())
			{
				if (gDLL->getChtLvl() > 0)
				{
					// Log info about this combat...
					char szOut[1024];
					sprintf( szOut, "*** KOMBAT!\n     ATTACKER: Player %d Unit %d (%S's %S), CombatStrength=%d\n     DEFENDER: Player %d Unit %d (%S's %S), CombatStrength=%d\n",
						getOwnerINLINE(), getID(), GET_PLAYER(getOwnerINLINE()).getName(), getName().GetCString(), currCombatStr(NULL, NULL),
						pCombatUnit->getOwnerINLINE(), pCombatUnit->getID(), GET_PLAYER(pCombatUnit->getOwnerINLINE()).getName(), pCombatUnit->getName().GetCString(), pCombatUnit->currCombatStr(pCombatUnit->plot(), this));
					gDLL->messageControlLog(szOut);
				}
			}

			if (getDomainType() == DOMAIN_LAND
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//				&& isInViewport()
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
				&& !m_pUnitInfo->isIgnoreBuildingDefense()
				&& pCombatUnit->plot()->getPlotCity()
				&& pCombatUnit->plot()->getPlotCity()->getBuildingDefense() > 0
				&& cityAttackModifier() >= GC.getDefineINT("MIN_CITY_ATTACK_MODIFIER_FOR_SIEGE_TOWER"))
			{
				CvDLLEntity::SetSiegeTower(true);
			}
		}

		FAssertMsg(getCombatUnit() == NULL, "Combat Unit is not expected to be assigned");
		FAssertMsg(!(plot()->isFighting()), "(plot()->isFighting()) did not return false as expected");
		m_bCombatFocus = (bAttacking && !(gDLL->getInterfaceIFace()->isFocusedWidget()) && ((getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) || ((pCombatUnit->getOwnerINLINE() == GC.getGameINLINE().getActivePlayer()) && !(GC.getGameINLINE().isMPOption(MPOPTION_SIMULTANEOUS_TURNS)))));
		m_combatUnit = pCombatUnit->getIDInfo();
		//IgnoreFirstStrikes tag added, Sephi
		//setCombatFirstStrikes((pCombatUnit->immuneToFirstStrikes()) ? 0 : (firstStrikes() + GC.getGameINLINE().getSorenRandNum(chanceFirstStrikes() + 1, "First Strike")));
		setCombatFirstStrikes((pCombatUnit->immuneToFirstStrikes()) ? 0 : (std::max(0,firstStrikes() + GC.getGameINLINE().getSorenRandNum(chanceFirstStrikes() + 1, "First Strike") - pCombatUnit->getIgnoreFirstStrikes())));
	}
	else
	{
		if(getCombatUnit() != NULL)
		{
			FAssertMsg(getCombatUnit() != NULL, "getCombatUnit() is not expected to be equal with NULL");
			FAssertMsg(plot()->isFighting(), "plot()->isFighting is expected to be true");
			m_bCombatFocus = false;
			m_combatUnit.reset();
			setCombatFirstStrikes(0);

			if (IsSelected())
			{
				gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
			}

			if (plot() == gDLL->getInterfaceIFace()->getSelectionPlot())
			{
				gDLL->getInterfaceIFace()->setDirty(PlotListButtons_DIRTY_BIT, true);
			}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
			CvDLLEntity::SetSiegeTower(false);
/**
			if (isInViewport())
			{
				CvDLLEntity::SetSiegeTower(false);
			}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		}
	}

	setCombatTimer(0);
	setInfoBarDirty(true);

	if (isCombatFocus())
	{
		gDLL->getInterfaceIFace()->setCombatFocus(true);
	}
}


CvUnit* CvUnit::getTransportUnit() const
{
	return getUnit(m_transportUnit);
}


bool CvUnit::isCargo() const
{
	return (getTransportUnit() != NULL);
}


void CvUnit::setTransportUnit(CvUnit* pTransportUnit)
{
	CvUnit* pOldTransportUnit;

	pOldTransportUnit = getTransportUnit();

	if (pOldTransportUnit != pTransportUnit)
	{
		if (pOldTransportUnit != NULL)
		{
			pOldTransportUnit->changeCargo(-1);
		}

		if (pTransportUnit != NULL)
		{
			FAssertMsg(pTransportUnit->cargoSpaceAvailable(getSpecialUnitType(), getDomainType()) > 0, "Cargo space is expected to be available");

			joinGroup(NULL, true); // Because what if a group of 3 tries to get in a transport which can hold 2...

			m_transportUnit = pTransportUnit->getIDInfo();

			if (getDomainType() != DOMAIN_AIR)
			{
				getGroup()->setActivityType(ACTIVITY_SLEEP);
			}

			if (GC.getGameINLINE().isFinalInitialized())
			{
				finishMoves();
			}

			pTransportUnit->changeCargo(1);
			pTransportUnit->getGroup()->setActivityType(ACTIVITY_AWAKE);
		}
		else
		{
			m_transportUnit.reset();

			getGroup()->setActivityType(ACTIVITY_AWAKE);
		}

#ifdef _DEBUG
		std::vector<CvUnit*> aCargoUnits;
		if (pOldTransportUnit != NULL)
		{
			pOldTransportUnit->getCargoUnits(aCargoUnits);
		}
		if (pTransportUnit != NULL)
		{
			pTransportUnit->getCargoUnits(aCargoUnits);
		}
#endif

	}
}


int CvUnit::getExtraDomainModifier(DomainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiExtraDomainModifier[eIndex];
}


void CvUnit::changeExtraDomainModifier(DomainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_DOMAIN_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	m_aiExtraDomainModifier[eIndex] = (m_aiExtraDomainModifier[eIndex] + iChange);
}


const CvWString CvUnit::getName(uint uiForm) const
{
	CvWString szBuffer;

	if (m_szName.empty())
	{
		return m_pUnitInfo->getDescription(uiForm);
	}

	szBuffer.Format(L"%s (%s)", m_szName.GetCString(), m_pUnitInfo->getDescription(uiForm));

	return szBuffer;
}


const wchar* CvUnit::getNameKey() const
{
	if (m_szName.empty())
	{
		return m_pUnitInfo->getTextKeyWide();
	}
	else
	{
		return m_szName.GetCString();
	}
}


const CvWString& CvUnit::getNameNoDesc() const
{
	return m_szName;
}


void CvUnit::setName(CvWString szNewValue)
{
	gDLL->stripSpecialCharacters(szNewValue);

	m_szName = szNewValue;

	if (IsSelected())
	{
		gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
	}
}


std::string CvUnit::getScriptData() const
{
	return m_szScriptData;
}


void CvUnit::setScriptData(std::string szNewValue)
{
	m_szScriptData = szNewValue;
}


int CvUnit::getTerrainDoubleMoveCount(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiTerrainDoubleMoveCount[eIndex];
}


bool CvUnit::isTerrainDoubleMove(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return (getTerrainDoubleMoveCount(eIndex) > 0);
}


void CvUnit::changeTerrainDoubleMoveCount(TerrainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiTerrainDoubleMoveCount[eIndex] = (m_paiTerrainDoubleMoveCount[eIndex] + iChange);
	FAssert(getTerrainDoubleMoveCount(eIndex) >= 0);
}


int CvUnit::getFeatureDoubleMoveCount(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiFeatureDoubleMoveCount[eIndex];
}


bool CvUnit::isFeatureDoubleMove(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return (getFeatureDoubleMoveCount(eIndex) > 0);
}


void CvUnit::changeFeatureDoubleMoveCount(FeatureTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiFeatureDoubleMoveCount[eIndex] = (m_paiFeatureDoubleMoveCount[eIndex] + iChange);
	FAssert(getFeatureDoubleMoveCount(eIndex) >= 0);
}


int CvUnit::getExtraTerrainAttackPercent(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiExtraTerrainAttackPercent[eIndex];
}


void CvUnit::changeExtraTerrainAttackPercent(TerrainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraTerrainAttackPercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraTerrainDefensePercent(TerrainTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiExtraTerrainDefensePercent[eIndex];
}


void CvUnit::changeExtraTerrainDefensePercent(TerrainTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumTerrainInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraTerrainDefensePercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraFeatureAttackPercent(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiExtraFeatureAttackPercent[eIndex];
}


void CvUnit::changeExtraFeatureAttackPercent(FeatureTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraFeatureAttackPercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraFeatureDefensePercent(FeatureTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiExtraFeatureDefensePercent[eIndex];
}


void CvUnit::changeExtraFeatureDefensePercent(FeatureTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumFeatureInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		m_paiExtraFeatureDefensePercent[eIndex] += iChange;

		setInfoBarDirty(true);
	}
}

int CvUnit::getExtraUnitCombatModifier(UnitCombatTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitCombatInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiExtraUnitCombatModifier[eIndex];
}


void CvUnit::changeExtraUnitCombatModifier(UnitCombatTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumUnitCombatInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiExtraUnitCombatModifier[eIndex] = (m_paiExtraUnitCombatModifier[eIndex] + iChange);
}


bool CvUnit::canAcquirePromotion(PromotionTypes ePromotion, bool bMinLevel) const
{
	FAssertMsg(ePromotion >= 0, "ePromotion is expected to be non-negative (invalid Index)");
	FAssertMsg(ePromotion < GC.getNumPromotionInfos(), "ePromotion is expected to be within maximum bounds (invalid Index)");

	CvPromotionInfo& kPromotion=GC.getPromotionInfo(ePromotion);

	if (isHasPromotion(ePromotion))
	{
		return false;
	}

	if (kPromotion.getPrereqPromotion() != NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)(kPromotion.getPrereqPromotion())))
		{
			return false;
		}
	}

//FfH: Modified by Kael 07/30/2007
//	if (GC.getPromotionInfo(ePromotion).getPrereqOrPromotion1() != NO_PROMOTION)
//	{
//		if (!isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPrereqOrPromotion1())))
//		{
//			if ((GC.getPromotionInfo(ePromotion).getPrereqOrPromotion2() == NO_PROMOTION) || !isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPrereqOrPromotion2())))
//			{
//				return false;
//			}
//		}
//	}
	if (kPromotion.getMinLevel() == -1)
	{
	    return false;
	}

	if(kPromotion.getMaxLevel()!=0)
	{
		if(getLevel()>=kPromotion.getMaxLevel()
			|| getExperience() >= getExperienceNeeded(kPromotion.getMaxLevel()))
		{
			return false;
		}
	}

	if(bMinLevel)
	{
		if (GC.getPromotionInfo(ePromotion).isRace())
		{
			return false;
		}

		if (GC.getPromotionInfo(ePromotion).getMinLevel() > getLevel())
		{
			return false;
		}
	}

	if (GC.getPromotionInfo(ePromotion).isEquipment())
	{
	    return false;
	}
	if (GC.getPromotionInfo(ePromotion).getPromotionPrereqAnd() != NO_PROMOTION)
	{
		if (!isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPromotionPrereqAnd())))
		{
			return false;
		}
	}
	if (GC.getPromotionInfo(ePromotion).getPrereqOrPromotion1() != NO_PROMOTION)
	{
	    bool bValid = false;
		if (isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPrereqOrPromotion1())))
		{
		    bValid = true;
		}
        if (GC.getPromotionInfo(ePromotion).getPrereqOrPromotion2() != NO_PROMOTION)
        {
            if (isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPrereqOrPromotion2())))
            {
                bValid = true;
            }
        }
        if (GC.getPromotionInfo(ePromotion).getPromotionPrereqOr3() != NO_PROMOTION)
        {
            if (isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPromotionPrereqOr3())))
            {
                bValid = true;
            }
        }
        if (GC.getPromotionInfo(ePromotion).getPromotionPrereqOr4() != NO_PROMOTION)
        {
            if (isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPromotionPrereqOr4())))
            {
                bValid = true;
            }
        }
        if (GC.getPromotionInfo(ePromotion).getPromotionPrereqOr5() != NO_PROMOTION)
        {
            if (isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPromotionPrereqOr5())))
            {
                bValid = true;
            }
        }
        if (GC.getPromotionInfo(ePromotion).getPromotionPrereqOr6() != NO_PROMOTION)
        {
            if (isHasPromotion((PromotionTypes)(GC.getPromotionInfo(ePromotion).getPromotionPrereqOr6())))
            {
                bValid = true;
            }
        }
        if (!bValid)
        {
            return false;
        }
	}
	if (GC.getPromotionInfo(ePromotion).getBonusPrereq() != NO_BONUS)
	{
	    if (!GET_PLAYER(getOwnerINLINE()).hasBonus((BonusTypes)GC.getPromotionInfo(ePromotion).getBonusPrereq()))
	    {
	        return false;
	    }
	}
    for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
    {
        if (isHasPromotion((PromotionTypes)iI))
        {
            if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionImmune1() == ePromotion)
            {
                return false;
            }
            if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionImmune2() == ePromotion)
            {
                return false;
            }
            if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionImmune3() == ePromotion)
            {
                return false;
            }
        }
    }
	if (GC.getPromotionInfo(ePromotion).isPrereqAlive())
	{
	    if (!isAlive())
	    {
	        return false;
	    }
	}
//FfH: End Add

/*************************************************************************************************/
/**	ADDON (Gear Promotions ) Sephi							                     				**/
/*************************************************************************************************/
	if (GC.getPromotionInfo(ePromotion).getEquipmentCategory()!=NO_EQUIPMENTCATEGORY)
	{
		return false;
	}

	if (GC.getPromotionInfo(ePromotion).isGear() && !plot()->isCity())
	{
		return false;
	}

    if (GC.getPromotionInfo(ePromotion).getGoldCost() > 0 && GET_PLAYER(getOwnerINLINE()).getGold() < GC.getPromotionInfo(ePromotion).getGoldCost())
	{
        return false;
	}

/** new Promotion tags **/
	/**
	if(GC.getPromotionInfo(ePromotion).isClassPromotion() && getLevel() < 2)
	{
		return false;
	}
**/
	bool bValid;
	int iNumPrereqs;

	if (plot() != NULL)
	{
		CvCity* pCity = plot()->getPlotCity();
		if (pCity != NULL)
		{
			iNumPrereqs = GC.getPromotionInfo(ePromotion).getNumPrereqBuildingORs();
			if (iNumPrereqs > 0)
			{
				bValid = false;
				for (int iI = 0; iI < iNumPrereqs; iI++)
				{
					if (pCity->getNumBuilding((BuildingTypes)GC.getPromotionInfo(ePromotion).getPrereqBuildingOR(iI)) > 0)
					{
						bValid = true;
					}
				}
				if (!bValid)
				{
					return false;
				}
			}

			iNumPrereqs = GC.getPromotionInfo(ePromotion).getNumPrereqBuildingANDs();
			if (iNumPrereqs > 0)
			{
				bValid = true;
				for (int iI = 0; iI < iNumPrereqs; iI++)
				{
					if (pCity->getNumBuilding((BuildingTypes)GC.getPromotionInfo(ePromotion).getPrereqBuildingAND(iI)) < 1)
					{
						bValid = false;
					}
				}
				if (!bValid)
				{
					return false;
				}
			}
		}
	}
    iNumPrereqs = GC.getPromotionInfo(ePromotion).getNumPrereqCivilizations();
    if (iNumPrereqs > 0)
    {
		bValid = false;
        for (int iI = 0; iI < iNumPrereqs; iI++)
		{
            if (GET_PLAYER(getOwner()).getCivilizationType() == (CivilizationTypes)GC.getPromotionInfo(ePromotion).getPrereqCivilization(iI))
			{
                bValid = true;
			}
		}
        if (!bValid)
		{
            return false;
		}
	}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (GC.getPromotionInfo(ePromotion).getTechPrereq() != NO_TECH)
	{
		if (!(GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getPromotionInfo(ePromotion).getTechPrereq()))))
		{
			return false;
		}
	}

	if (GC.getPromotionInfo(ePromotion).getStateReligionPrereq() != NO_RELIGION)
	{
		if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != GC.getPromotionInfo(ePromotion).getStateReligionPrereq())
		{
			return false;
		}
	}

/*************************************************************************************************/
/**	ADDON (Combatauras) Sephi																	**/
/*************************************************************************************************/

	if(GC.getPromotionInfo(ePromotion).getCombatAuraEffect()!=0)
	{
		if(getCombatAura()==NULL)
		{
			return false;
		}
	}

	if(GC.getPromotionInfo(ePromotion).getCombatAuraRange()!=0)
	{
		if(getCombatAura()==NULL)
		{
			return false;
		}
	}

	if(GC.getPromotionInfo(ePromotion).getCombatAuraTargets()!=0)
	{
		if(getCombatAura()==NULL)
		{
			return false;
		}
		if(getCombatAura()->getTargets()==-1)
		{
			return false;
		}
	}

/**
	PromotionTypes eHero=GC.getInfoTypeForString("PROMOTION_HERO");
	if(eHero!=NO_PROMOTION && isHasPromotion(eHero))
	{
		if(!GC.getPromotionInfo(ePromotion).isHeroPromotion())
		{
			return false;
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (!isPromotionValid(ePromotion))
	{
		return false;
	}

	return true;
}

bool CvUnit::isPromotionValid(PromotionTypes ePromotion) const
{
	if (!::isPromotionValid(ePromotion, getUnitType(), true))
	{
		return false;
	}

	CvPromotionInfo& promotionInfo = GC.getPromotionInfo(ePromotion);

//FfH: Modified by Kael 10/28/2008
//	if (promotionInfo.getWithdrawalChange() + m_pUnitInfo->getWithdrawalProbability() + getExtraWithdrawal() > GC.getDefineINT("MAX_WITHDRAWAL_PROBABILITY"))
//	{
//		return false;
//	}
    if (promotionInfo.getWithdrawalChange() > 0)
    {
        if (promotionInfo.getWithdrawalChange() + m_pUnitInfo->getWithdrawalProbability() + getExtraWithdrawal() > GC.getDefineINT("MAX_WITHDRAWAL_PROBABILITY"))
        {
            return false;
        }
    }
//FfH: End Modify

	if (promotionInfo.getInterceptChange() + maxInterceptionProbability() > GC.getDefineINT("MAX_INTERCEPTION_PROBABILITY"))
	{
		return false;
	}

	if (promotionInfo.getEvasionChange() + evasionProbability() > GC.getDefineINT("MAX_EVASION_PROBABILITY"))
	{
		return false;
	}

	return true;
}


bool CvUnit::canAcquirePromotionAny() const
{
	int iI;

	if(m_pUnitInfo->getUnitCombatType() != NO_UNITCOMBAT) {
		if(GC.getUnitCombatInfo((UnitCombatTypes)m_pUnitInfo->getUnitCombatType()).isNoPromotions()) {
			return false;
		}
	}

	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (canAcquirePromotion((PromotionTypes)iI))
		{
			return true;
		}
	}

	return false;
}


bool CvUnit::isHasPromotion(PromotionTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumPromotionInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_pabHasPromotion[eIndex];
}


void CvUnit::setHasPromotion(PromotionTypes eIndex, bool bNewValue)
{
	int iChange;
	int iI;
	CvPromotionInfo &kPromotion = GC.getPromotionInfo(eIndex);

//FfH: Added by Kael 07/28/2008
    if (bNewValue)
    {
/*************************************************************************************************/
/**	ADDON (Promotions can be unlocked by Gameoptions) Sephi                     				**/
/*************************************************************************************************/
        GameOptionTypes eGameOption = (GameOptionTypes)kPromotion.getRequiredGameOption();
        if (eGameOption!=NO_GAMEOPTION)
        {
            if (!GC.getGameINLINE().isOption(eGameOption))
            {
                return;
            }
        }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Promotion dependancy on other Promotions) Sephi	                     				**/
/*************************************************************************************************/

        if (isDenyPromotion(eIndex))
        {
            return;
        }

        if (kPromotion.getNumPromotionMustHave() > 0)
		{
            for (int iJ = 0; iJ < kPromotion.getNumPromotionMustHave(); iJ++)
			{
                if(!isHasPromotion(kPromotion.getPromotionMustHave(iJ)))
                {
                    return;
                }
			}
		}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

        for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
        {
            if (isHasPromotion((PromotionTypes)iI))
            {
                if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionImmune1() == eIndex)
                {
                    return;
                }
                if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionImmune2() == eIndex)
                {
                    return;
                }
                if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionImmune3() == eIndex)
                {
                    return;
                }
            }
        }
    }
//FfH: End Add

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bUnitPromoted                                                                           **/
/*************************************************************************************************/
    if (bNewValue)
    {
        for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
        {
            if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
            {
                CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
                for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
                {
                    if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
                    {
                        if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isUnitPromoted()
                            && GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getPromotionType()==eIndex)
                        {
                            GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
                        }
                    }
                }
            }
        }
    }
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/


	if (isHasPromotion(eIndex) != bNewValue)
	{
		m_pabHasPromotion[eIndex] = bNewValue;

		iChange = ((isHasPromotion(eIndex)) ? 1 : -1);

		changeBlitzCount((kPromotion.isBlitz()) ? iChange : 0);
		changeAmphibCount((kPromotion.isAmphib()) ? iChange : 0);
		changeRiverCount((kPromotion.isRiver()) ? iChange : 0);
		changeEnemyRouteCount((kPromotion.isEnemyRoute()) ? iChange : 0);
		changeAlwaysHealCount((kPromotion.isAlwaysHeal()) ? iChange : 0);
		changeHillsDoubleMoveCount((kPromotion.isHillsDoubleMove()) ? iChange : 0);
		changeImmuneToFirstStrikesCount((kPromotion.isImmuneToFirstStrikes()) ? iChange : 0);

		changeExtraVisibilityRange(kPromotion.getVisibilityChange() * iChange);
		changeExtraMoves(kPromotion.getMovesChange() * iChange);
		changeExtraMoveDiscount(kPromotion.getMoveDiscountChange() * iChange);
		changeExtraAirRange(kPromotion.getAirRangeChange() * iChange);
		changeExtraIntercept(kPromotion.getInterceptChange() * iChange);
		changeExtraEvasion(kPromotion.getEvasionChange() * iChange);
		changeExtraFirstStrikes(kPromotion.getFirstStrikesChange() * iChange);
		changeExtraChanceFirstStrikes(kPromotion.getChanceFirstStrikesChange() * iChange);
		changeExtraWithdrawal(kPromotion.getWithdrawalChange() * iChange);
		changeExtraCollateralDamage(kPromotion.getCollateralDamageChange() * iChange);
		changeExtraBombardRate(kPromotion.getBombardRateChange() * iChange);
		changeExtraEnemyHeal(kPromotion.getEnemyHealChange() * iChange);
		changeExtraNeutralHeal(kPromotion.getNeutralHealChange() * iChange);
		changeExtraFriendlyHeal(kPromotion.getFriendlyHealChange() * iChange);
		changeSameTileHeal(kPromotion.getSameTileHealChange() * iChange);
		changeAdjacentTileHeal(kPromotion.getAdjacentTileHealChange() * iChange);
		changeExtraCombatPercent(kPromotion.getCombatPercent() * iChange);
		changeExtraCityAttackPercent(kPromotion.getCityAttackPercent() * iChange);
		changeExtraCityDefensePercent(kPromotion.getCityDefensePercent() * iChange);
		changeExtraHillsAttackPercent(kPromotion.getHillsAttackPercent() * iChange);
		changeExtraHillsDefensePercent(kPromotion.getHillsDefensePercent() * iChange);
		changeRevoltProtection(kPromotion.getRevoltProtection() * iChange);
		changeCollateralDamageProtection(kPromotion.getCollateralDamageProtection() * iChange);
		changePillageChange(kPromotion.getPillageChange() * iChange);
		changeUpgradeDiscount(kPromotion.getUpgradeDiscount() * iChange);
		changeExperiencePercent(kPromotion.getExperiencePercent() * iChange);
		changeKamikazePercent((kPromotion.getKamikazePercent()) * iChange);
		changeCargoSpace(kPromotion.getCargoChange() * iChange);
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/*************************************************************************************************/
        changeAirCombat(kPromotion.getAirCombat() * iChange);
        changeAirCombatLimitBoost(kPromotion.getAirCombatLimitBoost() * iChange);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//>>>>Refined Defensive Strikes: Added by Denev 2009/11/10
		changeDefensiveStrikes(kPromotion.isDefensiveStrikes() ? iChange : 0);
//<<<<Refined Defensive Strikes: End Add
		changeBonusVSUnitsFullHealth(kPromotion.getBonusVSUnitsFullHealth() * iChange);
		changeSpellTargetIncrease(kPromotion.getSpellTargetIncrease() * iChange);
		setFlamingArrows((kPromotion.isFlamingArrows()) ? (iChange > 0) : isFlamingArrows());
		setEquipmentYieldType((kPromotion.getEquipmentYieldType()!=NO_YIELD) ? kPromotion.getEquipmentYieldType() : getEquipmentYieldType());
/*************************************************************************************************/
/**	ADDON (Houses of Erebus) Sephi			                                 					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
		if (bNewValue)
		{
			for (int i=0;i<GC.getNumCorporationInfos();i++)
			{
                if (kPromotion.getCorporationSupport((CorporationTypes)i)!=0)
                {
                    GET_PLAYER(getOwnerINLINE()).changeCorporationSupport((CorporationTypes)i,kPromotion.getCorporationSupport((CorporationTypes)i));
                }
			}
		}

		if (!bNewValue)
		{
			for (int i=0;i<GC.getNumCorporationInfos();i++)
			{
                if (kPromotion.getCorporationSupport((CorporationTypes)i)!=0)
                {
                    GET_PLAYER(getOwnerINLINE()).changeCorporationSupport((CorporationTypes)i,-kPromotion.getCorporationSupport((CorporationTypes)i));
                }
			}
		}

		if (bNewValue)
		{
			for (int i=0;i<GC.getNumCorporationInfos();i++)
			{
				if (kPromotion.getCorporationSupportMultiplier((CorporationTypes)i)!=0)
				{
					GET_PLAYER(getOwnerINLINE()).changeCorporationSupportMultiplier((CorporationTypes)i,kPromotion.getCorporationSupportMultiplier((CorporationTypes)i));
				}
			}
		}

		if (!bNewValue)
		{
			for (int i=0;i<GC.getNumCorporationInfos();i++)
			{
				if (kPromotion.getCorporationSupportMultiplier((CorporationTypes)i)!=0)
				{
					GET_PLAYER(getOwnerINLINE()).changeCorporationSupportMultiplier((CorporationTypes)i,-kPromotion.getCorporationSupportMultiplier((CorporationTypes)i));
				}
			}
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//FfH: Added by Kael 07/30/2007
        if (kPromotion.isAIControl() && bNewValue)
        {
            joinGroup(NULL);
        }
        changeAIControl((GC.getPromotionInfo(eIndex).isAIControl()) ? iChange : 0);
		changeAlive((kPromotion.isNotAlive()) ? iChange : 0);
		changeBaseCombatFromPromotion(kPromotion.getExtraCombatStr() * iChange);
		changeBaseCombatDefenseFromPromotion(kPromotion.getExtraCombatDefense() * iChange);
		changeBetterDefenderThanPercent(kPromotion.getBetterDefenderThanPercent() * iChange);
		changeBoarding((kPromotion.isBoarding()) ? iChange : 0);
		changeCaptureAnimalChance(kPromotion.getCaptureAnimalChance() * iChange);
		changeCaptureBeastChance(kPromotion.getCaptureBeastChance() * iChange);
		changeCombatHealPercent(kPromotion.getCombatHealPercent() * iChange);
		changeCombatManaGained(kPromotion.getCombatManaGained() * iChange);
		changeCombatPercentInBorders(kPromotion.getCombatPercentInBorders() * iChange);
		changeCombatPercentDefense(kPromotion.getCombatPercentDefense() * iChange);
		changeCombatPercentGlobalCounter(kPromotion.getCombatPercentGlobalCounter() * iChange);
		changeDefensiveStrikeChance(kPromotion.getDefensiveStrikeChance() * iChange);
		changeDefensiveStrikeDamage(kPromotion.getDefensiveStrikeDamage() * iChange);
		changeDoubleFortifyBonus((kPromotion.isDoubleFortifyBonus()) ? iChange : 0);
		changeFear((kPromotion.getFear() * iChange));
		changeFlying((kPromotion.isFlying()) ? iChange : 0);
		changeGoldFromCombat(kPromotion.getGoldFromCombat() * iChange);
		changeHeld((kPromotion.isHeld()) ? iChange : 0);
		changeHiddenNationality((kPromotion.isHiddenNationality()) ? iChange : 0);
		changeIgnoreBuildingDefense((kPromotion.isIgnoreBuildingDefense()) ? iChange : 0);
		changeImmortal((kPromotion.isImmortal()) ? iChange : 0);
		changeImmuneToCapture((kPromotion.isImmuneToCapture()) ? iChange : 0);
		changeImmuneToDefensiveStrike((kPromotion.isImmuneToDefensiveStrike()) ? iChange : 0);
		changeImmuneToFear((kPromotion.isImmuneToFear()) ? iChange : 0);
		changeImmuneToMagic((kPromotion.isImmuneToMagic()) ? iChange : 0);
		changeInvisibleFromPromotion((kPromotion.isInvisible()) ? iChange : 0);
		changeOnlyDefensive((kPromotion.isOnlyDefensive()) ? iChange : 0);
		changeResist(kPromotion.getResistMagic() * iChange);
		changeResistModify(kPromotion.getCasterResistModify() * iChange);
		changeSeeInvisible((kPromotion.isSeeInvisible()) ? iChange : 0);
		changeSpellCasterXP(kPromotion.getSpellCasterXP() * iChange);
		changeSpellDamageModify(kPromotion.getSpellDamageModify() * iChange);
		changeTargetWeakestUnit((kPromotion.isTargetWeakestUnit()) ? iChange : 0);
		changeTargetWeakestUnitCounter((kPromotion.isTargetWeakestUnitCounter()) ? iChange : 0);
		changeTwincast((kPromotion.isTwincast()) ? iChange : 0);
		changeWaterWalking((kPromotion.isWaterWalking()) ? iChange : 0);
		changeWorkRateModify(kPromotion.getWorkRateModify() * iChange);
        GC.getGameINLINE().changeGlobalCounter(kPromotion.getModifyGlobalCounter() * iChange);
        if (kPromotion.getCombatLimit() != 0)
        {
            calcCombatLimit();
        }
        if (eIndex == GC.getDefineINT("MUTATED_PROMOTION") && bNewValue)
        {
            mutate();
        }
        if (kPromotion.isRace())
        {
            if (bNewValue)
            {
                setRace(eIndex);
            }
            else
            {
                setRace(NO_PROMOTION);
            }
        }
		for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
		{
			changeDamageTypeCombat(((DamageTypes)iI), (kPromotion.getDamageTypeCombat(iI) * iChange));
		}
		for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			changeBonusAffinity(((BonusTypes)iI), (kPromotion.getBonusAffinity(iI) * iChange));
		}
		for (iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
		{
			changeDamageTypeResist(((DamageTypes)iI), (kPromotion.getDamageTypeResist(iI) * iChange));
		}
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (New Promotions) Sephi	                     				                        **/
/*************************************************************************************************/
		changeCannotLeaveCulture((kPromotion.isCannotLeaveCulture()) ? iChange : 0);
		changeNoXP((kPromotion.isNoXP()) ? iChange : 0);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Combatauras) Sephi																	**/
/*************************************************************************************************/
		if(kPromotion.getCombatAuraType()!=NO_COMBATAURA)
		{
			if(getCombatAuraType()==NO_COMBATAURA)
			{
				setCombatAuraType(kPromotion.getCombatAuraType());
			}
		}

		if(getCombatAura()!=NULL)
		{
			getCombatAura()->updatePromotionAdded(eIndex, iChange);
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Promotion dependancy on other Promotions) Sephi	                     				**/
/*************************************************************************************************/
        if (kPromotion.getNumPromotionExcludes() > 0)
		{
            for (int iJ = 0; iJ < kPromotion.getNumPromotionExcludes(); iJ++)
			{
                changeDenyPromotion(kPromotion.getPromotionExcludes(iJ), iChange);
			}
		}

		//if Class Promotion is picked, deny other Class Promotions
		if (kPromotion.isClassPromotion())
		{
            for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if(iJ!=eIndex && GC.getPromotionInfo((PromotionTypes)iJ).isClassPromotion())
				{
					changeDenyPromotion((PromotionTypes)iJ, iChange);
				}
			}
		}
		if (kPromotion.isDiscipline())
		{
            for (int iJ = 0; iJ < GC.getNumPromotionInfos(); iJ++)
			{
				if(iJ!=eIndex && GC.getPromotionInfo((PromotionTypes)iJ).isDiscipline())
				{
					changeDenyPromotion((PromotionTypes)iJ, iChange);
				}
			}
		}

        if (bNewValue)
        {
			for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
			{
                if (isHasPromotion((PromotionTypes)iI))
				{
                    if (GC.getPromotionInfo((PromotionTypes)iI).getNumPromotionReplacedBy() > 0)
					{
                        for (int iJ = 0; iJ < GC.getPromotionInfo((PromotionTypes)iI).getNumPromotionReplacedBy(); iJ++)
						{
                            if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionReplacedBy(iJ) == eIndex)
							{
                                setHasPromotion((PromotionTypes)iI, false);
							}
						}
					}
				}
			}
		}

		if(eIndex==GC.getInfoTypeForString("PROMOTION_PIECES_OF_BARNAXUS"))
		{
			GC.getGameINLINE().changePiecesOfBarnaxusCounter(iChange);
		}

		if(kPromotion.getEquipmentCategory()!=NO_EQUIPMENTCATEGORY)
		{
			EquipmentCategoryTypes eCategory=(EquipmentCategoryTypes)kPromotion.getEquipmentCategory();
			DurabilityTypes eDurability=(DurabilityTypes)kPromotion.getDurabilityType();
			if(bNewValue)
			{
				setEquipment((EquipmentCategoryTypes)kPromotion.getEquipmentCategory(),eIndex);
			}
			else
			{
				setEquipment((EquipmentCategoryTypes)kPromotion.getEquipmentCategory(),NO_PROMOTION);
			}
			for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
			{
				if(kPromotion.isEquipmentPromotion(iI))
				{
					setHasPromotion((PromotionTypes)iI,bNewValue);
				}
			}
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

		for (iI = 0; iI < GC.getNumTerrainInfos(); iI++)
		{
			changeExtraTerrainAttackPercent(((TerrainTypes)iI), (kPromotion.getTerrainAttackPercent(iI) * iChange));
			changeExtraTerrainDefensePercent(((TerrainTypes)iI), (kPromotion.getTerrainDefensePercent(iI) * iChange));
			changeTerrainDoubleMoveCount(((TerrainTypes)iI), ((kPromotion.getTerrainDoubleMove(iI)) ? iChange : 0));
		}

		for (iI = 0; iI < GC.getNumFeatureInfos(); iI++)
		{
			changeExtraFeatureAttackPercent(((FeatureTypes)iI), (kPromotion.getFeatureAttackPercent(iI) * iChange));
			changeExtraFeatureDefensePercent(((FeatureTypes)iI), (kPromotion.getFeatureDefensePercent(iI) * iChange));
			changeFeatureDoubleMoveCount(((FeatureTypes)iI), ((kPromotion.getFeatureDoubleMove(iI)) ? iChange : 0));
		}

		for (iI = 0; iI < GC.getNumUnitCombatInfos(); iI++)
		{
			changeExtraUnitCombatModifier(((UnitCombatTypes)iI), (kPromotion.getUnitCombatModifierPercent(iI) * iChange));
		}

		for (iI = 0; iI < GC.getNumDurabilityInfos(); iI++)
		{
			changeEquipmentDurabilityValidityCounter(((DurabilityTypes)iI), (kPromotion.isMakeEquipmentDurabilityValid(iI)) ? iChange : 0);
		}

		for (iI = 0; iI < NUM_DOMAIN_TYPES; iI++)
		{
			changeExtraDomainModifier(((DomainTypes)iI), (kPromotion.getDomainModifierPercent(iI) * iChange));
		}

		if (IsSelected())
		{
			gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(InfoPane_DIRTY_BIT, true);
		}

		//update graphics
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		gDLL->getEntityIFace()->updatePromotionLayers(getUnitEntity());
/**
		if (isInViewport())
		{
			gDLL->getEntityIFace()->updatePromotionLayers(getUnitEntity());
		}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

//FfH: Added by Kael 07/04/2009
        if (kPromotion.getUnitArtStyleType() != NO_UNIT_ARTSTYLE)
        {
//>>>>Advanced Rules: Modified by Denev 2009/10/25
//*** Prevent to destroy artstyle
/*
			if (iChange > 0)
			{
				setUnitArtStyleType(kPromotion.getUnitArtStyleType());
			}
			else
			{
				if (kPromotion.getUnitArtStyleType() == getUnitArtStyleType())
				{
					setUnitArtStyleType(NO_UNIT_ARTSTYLE);
				}
			}
*/
			CvWStringBuffer szArtDefineTag;
			UnitArtStyleTypes eUnitArtStyleType = NO_UNIT_ARTSTYLE;

			if (NO_PROMOTION != getRace())
			{
				eUnitArtStyleType = (UnitArtStyleTypes)GC.getPromotionInfo((PromotionTypes)getRace()).getUnitArtStyleType();
				if (NO_UNIT_ARTSTYLE != eUnitArtStyleType)
				{
					const int iID = GC.getInfoTypeForString(m_pUnitInfo->getType());
					szArtDefineTag.assign(GC.getUnitArtStyleTypeInfo(eUnitArtStyleType).getEarlyArtDefineTag(0, iID));
				}
			}
			if (szArtDefineTag.isEmpty())
			{
				for (int iPromotion = 0; iPromotion < GC.getNumPromotionInfos(); iPromotion++)
				{
					if (isHasPromotion((PromotionTypes)iPromotion))
					{
						eUnitArtStyleType = (UnitArtStyleTypes)GC.getPromotionInfo((PromotionTypes)iPromotion).getUnitArtStyleType();
						if (NO_UNIT_ARTSTYLE != eUnitArtStyleType)
						{
							const int iID = GC.getInfoTypeForString(m_pUnitInfo->getType());
							szArtDefineTag.assign(GC.getUnitArtStyleTypeInfo(eUnitArtStyleType).getEarlyArtDefineTag(0, iID));
							if (!szArtDefineTag.isEmpty())
							{
								break;
							}
						}
					}
				}
			}

			setUnitArtStyleType(eUnitArtStyleType);
//<<<<Advanced Rules: End Modify
            reloadEntity();
        }
        if (kPromotion.getGroupSize() != 0)
        {
            if (bNewValue)
            {
                setGroupSize(kPromotion.getGroupSize());
            }
            else
            {
                //modified Sephi (to allow remove of Pegasus Promotion)
                //setGroupSize(0);
                setGroupSize(GC.getUnitInfo(getUnitType()).getGroupSize());
            }
            reloadEntity();
        }
//FfH: End Add

// Better Unit Power (Added by Skyre)
		updatePower();
// End Better Unit Power
	}
}


int CvUnit::getSubUnitCount() const
{
	return m_pUnitInfo->getGroupSize();
}


int CvUnit::getSubUnitsAlive() const
{
	return getSubUnitsAlive( getDamage());
}


int CvUnit::getSubUnitsAlive(int iDamage) const
{
	if (iDamage >= maxHitPoints())
	{
		return 0;
	}
	else
	{

//FfH: Modified by Kael 10/26/2007
//		return std::max(1, (((m_pUnitInfo->getGroupSize() * (maxHitPoints() - iDamage)) + (maxHitPoints() / ((m_pUnitInfo->getGroupSize() * 2) + 1))) / maxHitPoints()));
		return std::max(1, (((getGroupSize() * (maxHitPoints() - iDamage)) + (maxHitPoints() / ((getGroupSize() * 2) + 1))) / maxHitPoints()));
//FfH: End Modify
	}
}
// returns true if unit can initiate a war action with plot (possibly by declaring war)
bool CvUnit::potentialWarAction(const CvPlot* pPlot) const
{
	TeamTypes ePlotTeam = pPlot->getTeam();
	TeamTypes eUnitTeam = getTeam();

	if (ePlotTeam == NO_TEAM)
	{
		return false;
	}

	if (isEnemy(ePlotTeam, pPlot))
	{
		return true;
	}

	if (getGroup()->AI_isDeclareWar(pPlot) && GET_TEAM(eUnitTeam).AI_getWarPlan(ePlotTeam) != NO_WARPLAN)
	{
		return true;
	}

	return false;
}

//FfH Spell System: Added by Kael 07/23/2007
bool CvUnit::canCast(int spell, bool bTestVisible, CvPlot* targetPlot)
{
    SpellTypes eSpell = (SpellTypes)spell;
	CvPlot* pPlot = NULL;
	if (targetPlot != NULL)
		pPlot = targetPlot;
	else 
		pPlot = plot();

    CvUnit* pLoopUnit;
    CLLNode<IDInfo>* pUnitNode;
    bool bValid = false;

	CvSpellInfo &kSpell = GC.getSpellInfo(eSpell);

	if (getImmobileTimer() > 0)
	{
		return false;
	}

	if(!kSpell.isThiefMission() && isSpy()) {
		return false;
	}

    if (!isHuman())
    {
        if (!kSpell.isAllowAI())
        {
            return false;
        }
    }
    if (kSpell.getPromotionPrereq1() != NO_PROMOTION)
    {
        if (!isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq1()))
        {
            return false;
        }
    }
    if (kSpell.getPromotionPrereq2() != NO_PROMOTION)
    {
        if (!isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq2()))
        {
            return false;
        }
    }
    if (kSpell.getPromotionPrereq3() != NO_PROMOTION)
    {
        if (!isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq3()))
        {
            return false;
        }
    }
    if (kSpell.getPromotionPrereq4() != NO_PROMOTION)
    {
        if (!isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq4()))
        {
            return false;
        }
    }

    if (kSpell.getUnitClassPrereq() != NO_UNITCLASS)
    {
        if (getUnitClassType() != (UnitClassTypes)kSpell.getUnitClassPrereq())
        {
            return false;
        }
    }
    if (kSpell.getUnitPrereq() != NO_UNIT)
    {
        if (getUnitType() != (UnitTypes)kSpell.getUnitPrereq())
        {
            return false;
        }
    }
    if (kSpell.getUnitCombatPrereq() != NO_UNITCOMBAT)
    {
        if (getUnitCombatType() != (UnitCombatTypes)kSpell.getUnitCombatPrereq())
        {
            return false;
        }
    }
    if (kSpell.getBuildingPrereq() != NO_BUILDING)
    {
        if (!pPlot->isCity())
        {
            return false;
        }
        if (pPlot->getPlotCity()->getNumBuilding((BuildingTypes)kSpell.getBuildingPrereq()) == 0)
        {
            return false;
        }
    }
    if (kSpell.getBuildingClassOwnedPrereq() != NO_BUILDINGCLASS)
    {
        if (GET_PLAYER(getOwnerINLINE()).getBuildingClassCount((BuildingClassTypes)kSpell.getBuildingClassOwnedPrereq())  == 0)
        {
            return false;
        }
    }
    if (kSpell.getCivilizationPrereq() != NO_CIVILIZATION)
    {
        if (getCivilizationType() != (CivilizationTypes)kSpell.getCivilizationPrereq())
        {
            return false;
        }
    }
    if (kSpell.getCorporationPrereq() != NO_CORPORATION)
    {
        if (!pPlot->isCity())
        {
            return false;
        }
        if (!pPlot->getPlotCity()->isHasCorporation((CorporationTypes)kSpell.getCorporationPrereq()))
        {
            return false;
        }
    }

	if(kSpell.isExploreDungeon())
	{
		if(pPlot->getDungeonType() == NO_DUNGEON)
		{
			return false;
		}
	}

    if (kSpell.getImprovementPrereq() != NO_IMPROVEMENT)
    {
        if (pPlot->getImprovementType() != kSpell.getImprovementPrereq())
        {
            return false;
        }
    }
    if (kSpell.getReligionPrereq() != NO_RELIGION)
    {
        if (getReligion() != (ReligionTypes)kSpell.getReligionPrereq())
        {
            return false;
        }
    }
    if (kSpell.getStateReligionPrereq() != NO_RELIGION)
    {
        if (GET_PLAYER(getOwnerINLINE()).getStateReligion() != (ReligionTypes)kSpell.getStateReligionPrereq())
        {
            return false;
        }
    }
    if (kSpell.getTechPrereq() != NO_TECH)
    {
        if (!GET_TEAM(getTeam()).isHasTech((TechTypes)kSpell.getTechPrereq()))
        {
            return false;
        }
    }
    if (kSpell.getConvertUnitType() != NO_UNIT)
    {
        if (getUnitType() == (UnitTypes)kSpell.getConvertUnitType())
        {
            return false;
        }
    }
    if (kSpell.isGlobal())
    {
        if (GET_PLAYER(getOwnerINLINE()).isFeatAccomplished(FEAT_GLOBAL_SPELL))
        {
            return false;
        }
    }
    if (kSpell.isPrereqSlaveTrade())
    {
        if (!GET_PLAYER(getOwnerINLINE()).isSlaveTrade())
        {
            return false;
        }
    }
    if (GC.getUnitInfo((UnitTypes)getUnitType()).getEquipmentPromotion() != NO_PROMOTION)
    {
        if (kSpell.getUnitClassPrereq() != getUnitClassType())
        {
            return false;
        }
    }
    if (kSpell.getCasterMinLevel() != 0)
    {
        if (getLevel() < GC.getSpellInfo(eSpell).getCasterMinLevel())
        {
            return false;
        }
    }
    if (kSpell.isCasterMustBeAlive())
    {
        if (!isAlive())
        {
            return false;
        }
    }
    if (kSpell.isCasterNoDuration())
    {
        if (getDuration() != 0)
        {
            return false;
        }
    }
    if (kSpell.getPromotionInStackPrereq() != NO_PROMOTION)
    {
        if (isHasPromotion((PromotionTypes)kSpell.getPromotionInStackPrereq()))
        {
            return false;
        }
        bValid = false;
        pUnitNode = pPlot->headUnitNode();
        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = pPlot->nextUnitNode(pUnitNode);
            if (pLoopUnit->isHasPromotion((PromotionTypes)GC.getSpellInfo(eSpell).getPromotionInStackPrereq()))
            {
                if (getOwner() == pLoopUnit->getOwner())
                {
                    bValid = true;
                }
            }
        }
        if (bValid == false)
        {
            return false;
        }
    }
    if (GC.getSpellInfo(eSpell).getUnitInStackPrereq() != NO_UNIT)
    {
        if (getUnitType() == GC.getSpellInfo(eSpell).getUnitInStackPrereq())
        {
            return false;
        }
        bValid = false;
        pUnitNode = pPlot->headUnitNode();
        while (pUnitNode != NULL)
        {
            pLoopUnit = ::getUnit(pUnitNode->m_data);
            pUnitNode = pPlot->nextUnitNode(pUnitNode);
            if (pLoopUnit->getUnitType() == (UnitTypes)GC.getSpellInfo(eSpell).getUnitInStackPrereq())
            {
				if(!pLoopUnit->isDelayedDeath())
				{
					if (getOwner() == pLoopUnit->getOwner())
					{
						bValid = true;
					}
				}
            }
        }
        if (bValid == false)
        {
            return false;
        }
    }

/*************************************************************************************************/
/**	ADDON (New tags) Sephi                                                    					**/
/*************************************************************************************************/
    if (kSpell.getPrereqAlignment()!=NO_ALIGNMENT)
    {
        if(kSpell.getPrereqAlignment()!=GET_PLAYER(getOwnerINLINE()).getAlignment())
        {
            return false;
        }
    }
/*************************************************************************************************/
/**	ADDON (ObsoleteBySpell tag) Sephi                                          					**/
/*************************************************************************************************/
	if(kSpell.isSpellObsoleted(this,eSpell))
	{
		return false;
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


	if (bTestVisible)
	{
		if (kSpell.isDisplayWhenDisabled())
		{
			return true;
		}
	}

	if (!kSpell.isIgnoreHasCasted())
	{
		if (isHasCasted())
		{
			return false;
		}
	}

	//this isn't checked if the spell is also doing damage
	if(!kSpell.isTargetedHostile())
	{
		if(kSpell.getTerrainOrPrereq1()!=NO_TERRAIN)
		{
			if(pPlot->getTerrainType()!=kSpell.getTerrainOrPrereq1())
			{
				if(kSpell.getTerrainOrPrereq2()==NO_TERRAIN || kSpell.getTerrainOrPrereq2()!= pPlot->getTerrainType())
				{
					return false;
				}
			}
		}

		if(kSpell.getFeatureOrPrereq1()!=NO_TERRAIN)
		{
			if(pPlot->getFeatureType()!=kSpell.getFeatureOrPrereq1())
			{
				if(kSpell.getFeatureOrPrereq2()==NO_TERRAIN || kSpell.getFeatureOrPrereq2()!= pPlot->getFeatureType())
				{
					return false;
				}
			}
		}
	}

	if (kSpell.isAdjacentToWaterOnly())
	{
		if (!pPlot->isAdjacentToWater())
		{
			return false;
		}
	}

	if (kSpell.isInBordersOnly())
	{
		if (pPlot->getOwner() != getOwner())
		{
			return false;
		}
	}
	if (kSpell.isInCityOnly())
	{
		if (!pPlot->isCity())
		{
			return false;
		}
	}
	if (kSpell.getChangePopulation() != 0)
	{
		if (!pPlot->isCity())
		{
			return false;
		}
		if (pPlot->getPlotCity()->getPopulation() < (-1 * kSpell.getChangePopulation()))
		{
			return false;
		}
	}

    int iCost = kSpell.getCost();
    if (iCost != 0)
    {
        if (kSpell.getConvertUnitType() != NO_UNIT)
        {
            iCost += (iCost * GET_PLAYER(getOwnerINLINE()).getUpgradeCostModifier()) / 100;
        }

/*************************************************************************************************/
/**	BUGFIX (Spell Costs scale with Gamespeed) Sephi                                	            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
        iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
		iCost /=100;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

        if (GET_PLAYER(getOwnerINLINE()).getGold() < iCost)
        {
            return false;
        }
    }
    if (kSpell.isRemoveHasCasted())
    {
        if (!isHasCasted())
        {
            return false;
        }
        if (getDuration() > 0)
        {
            return false;
        }
    }

//>>>>Unofficial Bug Fix: Added by Denev 2009/12/03
//*** Team Unit or National Unit is limited.
	if (kSpell.getConvertUnitType() != NO_UNIT)
	{
		const UnitClassTypes eUnitClass = (UnitClassTypes)GC.getUnitInfo((UnitTypes)kSpell.getConvertUnitType()).getUnitClassType();
		const int iTeamLimit	= GC.getUnitClassInfo(eUnitClass).getMaxTeamInstances();
		const int iPlayerLimit	= GC.getUnitClassInfo(eUnitClass).getMaxPlayerInstances();
		if (iTeamLimit != -1)
		{
			if (iTeamLimit <= GET_TEAM(getTeam()).getUnitClassCount(eUnitClass))
			{
				return false;
			}
		}
		if (iPlayerLimit != -1)
		{
			if (iPlayerLimit <= GET_PLAYER(getOwnerINLINE()).getUnitClassCount(eUnitClass))
			{
				return false;
			}
		}
	}
//<<<<Unofficial Bug Fix: End Add
    if (kSpell.getCreateUnitType() != NO_UNIT)
    {
        if (!canCreateUnit(spell))
        {
            return false;
        }
    }

/*************************************************************************************************/
/**	ADDON (New tag Definitions) Sephi                                          					**/
/*************************************************************************************************/
	if (kSpell.getManaCost() > 0)
	{
		if (GET_PLAYER(getOwnerINLINE()).getMana()<GET_PLAYER(getOwnerINLINE()).getSpellManaCost(eSpell, this))
		{
			return false;
		}
	}

	if (kSpell.getFaithCost() > 0)
	{
		if (GET_PLAYER(getOwnerINLINE()).getFaith()<GET_PLAYER(getOwnerINLINE()).getSpellFaithCost(eSpell, this))
		{
			return false;
		}
	}

	bool bDoesDamage=false;
	if(kSpell.isTargetedHostile())
	{
		bValid=false;
		int iRange=getSpellCastingRange();
		int iDX,iDY;
		for (iDX = -(iRange); iDX <= iRange; iDX++)
		{
			for (iDY = -(iRange); iDY <= iRange; iDY++)
			{
				CvPlot* pTargetPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

				if (pTargetPlot != NULL && canSpellTargetPlot(pTargetPlot))
				{
					if(castDamage(spell,pTargetPlot,true)>0)
					{
						bValid=true;
						bDoesDamage=true;
						break;
					}
				}
			}
		}
		if(!bValid)
		{
			return false;
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	if (GC.getSpellInfo(eSpell).getCreateBuildingType() != NO_BUILDING)
	{
		if (!pPlot->isCity())
		{
			return false;
		}
		if (pPlot->getPlotCity()->getNumBuilding((BuildingTypes)GC.getSpellInfo((SpellTypes)spell).getCreateBuildingType()) > 0)
		{
			return false;
		}
	}

	if (GC.getSpellInfo(eSpell).getCreateBuildingType1() != NO_BUILDING)
	{
		if (!pPlot->isCity())
		{
			return false;
		}
		if (pPlot->getPlotCity()->getNumBuilding((BuildingTypes)GC.getSpellInfo((SpellTypes)spell).getCreateBuildingType1()) > 0)
		{
			return false;
		}
	}

	if (GC.getSpellInfo(eSpell).getCreateBuildingType2() != NO_BUILDING)
	{
		if (!pPlot->isCity())
		{
			return false;
		}
		if (pPlot->getPlotCity()->getNumBuilding((BuildingTypes)GC.getSpellInfo((SpellTypes)spell).getCreateBuildingType2()) > 0)
		{
			return false;
		}
	}

	if (GC.getSpellInfo(eSpell).getRemoveBuildingType() != NO_BUILDING)
	{
		if (!pPlot->isCity())
		{
			return false;
		}
		if (pPlot->getPlotCity()->getNumBuilding((BuildingTypes)GC.getSpellInfo((SpellTypes)spell).getRemoveBuildingType()) == 0)
		{
			return false;
		}
	}

/*************************************************************************************************/
/**	BETTER AI (AI doesn't trigger war with Spells) Sephi                       					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    if (!isHuman())
    {
        int iRange = GC.getSpellInfo(eSpell).getRange();
        if (GC.getSpellInfo(eSpell).isCausesWar() && iRange>0)
        {
            CvPlot* pLoopPlot;
            int iDX, iDY;

            for (iDX = -(iRange); iDX <= iRange; iDX++)
            {
                for (iDY = -(iRange); iDY <= iRange; iDY++)
                {
                    pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

                    if (pLoopPlot!=NULL && pLoopPlot!=pPlot)
                    {

                        CLLNode<IDInfo>* pUnitNode;
                        CvUnit* pLoopUnit;

                        pUnitNode = pLoopPlot->headUnitNode();

                        while (pUnitNode != NULL)
                        {
                            pLoopUnit = ::getUnit(pUnitNode->m_data);
                            pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);

                            if (pLoopUnit->getTeam()==pLoopPlot->getTeam())
                            {
                                if (!GET_TEAM(pLoopUnit->getTeam()).isVassal(getTeam()) && !GET_TEAM(getTeam()).isVassal(pLoopUnit->getTeam()))
                                {
                                    if (pLoopUnit->getTeam() != getTeam())
                                    {
                                        if (!GET_TEAM(getTeam()).isPermanentWarPeace(pLoopUnit->getTeam()))
                                        {
                                            return false;
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

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	if (!CvString(GC.getSpellInfo(eSpell).getPyRequirement()).empty())
    {
        CyUnit* pyUnit = new CyUnit(this);
        CyArgsList argsList;
        argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
        argsList.add(spell);//the spell #
        long lResult=0;
        gDLL->getPythonIFace()->callFunction(PYSpellModule, "canCast", argsList.makeFunctionArgs(), &lResult);
        delete pyUnit; // python fxn must not hold on to this pointer
        if (lResult == 0)
        {
            return false;
        }
        return true;
    }

    if (GC.getSpellInfo(eSpell).isRemoveHasCasted())
    {
        if (isHasCasted())
        {
            return true;
        }
    }

	if (GC.getSpellInfo(eSpell).getAddPromotionType1() != NO_PROMOTION)
	{
		if (canAddPromotion(spell))
		{
			return true;
		}
	}
	if (GC.getSpellInfo(eSpell).getRemovePromotionType1() != NO_PROMOTION)
	{
		if (canRemovePromotion(eSpell))
		{
			return true;
		}
	}

	if(kSpell.getHeal()>0)
	{
		if(canCastHeal(eSpell))
		{
			return true;
		}
	}

    if (GC.getSpellInfo(eSpell).getConvertUnitType() != NO_UNIT)
    {
        return true;
    }
    if (GC.getSpellInfo(eSpell).getCreateFeatureType() != NO_FEATURE)
    {
        if (canCreateFeature(spell))
        {
            return true;
        }
    }
    if (GC.getSpellInfo(eSpell).getCreateImprovementType() != NO_IMPROVEMENT)
    {
        if (canCreateImprovement(spell))
        {
            return true;
        }
    }
    if (GC.getSpellInfo(eSpell).getSpreadReligion() != NO_RELIGION)
    {
        if (canSpreadReligion(spell))
        {
            return true;
        }
    }
    if (GC.getSpellInfo(eSpell).getCreateBuildingType() != NO_BUILDING)
    {
        return true;
    }
    if (GC.getSpellInfo(eSpell).getCreateBuildingType1() != NO_BUILDING)
    {
        return true;
    }
    if (GC.getSpellInfo(eSpell).getCreateBuildingType2() != NO_BUILDING)
    {
        return true;
    }

    if (GC.getSpellInfo(eSpell).getRemoveBuildingType() != NO_BUILDING)
    {
        return true;
    }

    if (GC.getSpellInfo(eSpell).getCreateUnitType() != NO_UNIT)
    {
        return true;
    }
    if (bDoesDamage)
    {
        return true;
    }

	if (GC.getSpellInfo(eSpell).isPush())
    {
        if (canPush(spell))
        {
            return true;
        }
    }
    if (GC.getSpellInfo(eSpell).getChangePopulation() > 0)
    {
        return true;
    }
	if (!CvString(GC.getSpellInfo(eSpell).getPyResult()).empty())
    {
        return true;
    }

    return false;
}

bool CvUnit::canCreateUnit(int spell) const
{
	CvSpellInfo& kSpell = GC.getSpellInfo((SpellTypes)spell);
    if (getDuration() > 0) // to prevent summons summoning spinlocks
    {
        if (kSpell.getCreateUnitType() == getUnitType())
        {
            return false;
        }
    }
    if (plot()->isVisibleEnemyUnit(getOwnerINLINE())) // keeps invisible units from CtDing summoning on top of enemies
    {
        return false;
    }
    if (kSpell.isPermanentUnitCreate())
    {
        int iCount = 0;
        int iLoop = 0;
        CvUnit* pLoopUnit;
        CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
        for (pLoopUnit = kPlayer.firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = kPlayer.nextUnit(&iLoop))
        {
			bool bValid = false;
			if(kSpell.getReligionPrereq() != NO_RELIGION &&
				pLoopUnit->getReligion() == kSpell.getReligionPrereq()) {
					bValid = true;
			}
			else if(kSpell.getPromotionPrereq1() != NO_PROMOTION &&
				pLoopUnit->isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq1())) {
					bValid = true;
			}
			else if(kSpell.getPromotionPrereq2() != NO_PROMOTION &&
				pLoopUnit->isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq2())) {
					bValid = true;
			}
			else if(kSpell.getPromotionPrereq3() != NO_PROMOTION &&
				pLoopUnit->isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq3())) {
					bValid = true;
			}
			else if(kSpell.getPromotionPrereq4() != NO_PROMOTION &&
				pLoopUnit->isHasPromotion((PromotionTypes)kSpell.getPromotionPrereq4())) {
					bValid = true;
			}
			if(bValid) {
                iCount += 1;
            }
        }
        if (iCount <= kPlayer.getUnitClassCount((UnitClassTypes)GC.getUnitInfo((UnitTypes)GC.getSpellInfo((SpellTypes)spell).getCreateUnitType()).getUnitClassType()))
        {
            return false;
        }
    }
    return true;
}

bool CvUnit::canAddPromotiontoUnit(int iPromotion) const
{
    if(iPromotion==NO_PROMOTION)
    {
        return false;
    }
    if(isDenyPromotion((PromotionTypes)iPromotion))
    {
        return false;
    }

    if(isHasPromotion((PromotionTypes)iPromotion))
    {
        return false;
    }

	//Special Rules for Equipment
	if(GC.getPromotionInfo((PromotionTypes)iPromotion).getEquipmentCategory()!=NO_EQUIPMENTCATEGORY)
	{
		if(!GET_PLAYER(getOwnerINLINE()).canEquipUnit(this, (PromotionTypes)iPromotion, false, true))
		{
			return false;
		}
	}

    return true;
}


bool CvUnit::canAddPromotion(int spell)
{
	if(GC.getSpellInfo((SpellTypes)spell).isTargetedHostile())
	{
		return false;
	}

   	PromotionTypes ePromotion1 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getAddPromotionType1();
   	PromotionTypes ePromotion2 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getAddPromotionType2();
   	PromotionTypes ePromotion3 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getAddPromotionType3();
   	PromotionTypes ePromotion4 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getAddPromotionType4();
   	PromotionTypes ePromotion5 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getAddPromotionType5();
    if (GC.getSpellInfo((SpellTypes)spell).isBuffCasterOnly())
    {
        if (canAddPromotiontoUnit(ePromotion1))
        {
            return true;
        }

        if (canAddPromotiontoUnit(ePromotion2))
        {
            return true;
        }

        if (canAddPromotiontoUnit(ePromotion3))
        {
            return true;
        }

        if (canAddPromotiontoUnit(ePromotion4))
        {
            return true;
        }

        if (canAddPromotiontoUnit(ePromotion5))
        {
            return true;
        }

        return false;
    }
	CvUnit* pLoopUnit;
    CvPlot* pLoopPlot;
    int iRange = GC.getSpellInfo((SpellTypes)spell).getRange();
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            if (NULL != pLoopPlot)
            {
                CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                    if (!pLoopUnit->isImmuneToSpell(this, spell))
                    {
                        if (ePromotion1 != NO_PROMOTION)
                        {
                            if (pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT)
                            {
                                if (GC.getPromotionInfo(ePromotion1).getUnitCombat(pLoopUnit->getUnitCombatType()))
                                {
                                    if (pLoopUnit->canAddPromotiontoUnit(ePromotion1))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        if (ePromotion2 != NO_PROMOTION)
                        {
                            if (pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT)
                            {
                                if (GC.getPromotionInfo(ePromotion2).getUnitCombat(pLoopUnit->getUnitCombatType()))
                                {
                                    if (pLoopUnit->canAddPromotiontoUnit(ePromotion2))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        if (ePromotion3 != NO_PROMOTION)
                        {
                            if (pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT)
                            {
                                if (GC.getPromotionInfo(ePromotion3).getUnitCombat(pLoopUnit->getUnitCombatType()))
                                {
                                    if (pLoopUnit->canAddPromotiontoUnit(ePromotion3))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        if (ePromotion4 != NO_PROMOTION)
                        {
                            if (pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT)
                            {
                                if (GC.getPromotionInfo(ePromotion4).getUnitCombat(pLoopUnit->getUnitCombatType()))
                                {
                                    if (pLoopUnit->canAddPromotiontoUnit(ePromotion4))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                        if (ePromotion5 != NO_PROMOTION)
                        {
                            if (pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT)
                            {
                                if (GC.getPromotionInfo(ePromotion5).getUnitCombat(pLoopUnit->getUnitCombatType()))
                                {
                                    if (pLoopUnit->canAddPromotiontoUnit(ePromotion5))
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
	return false;
}

bool CvUnit::canCreateFeature(int spell) const
{
    if (plot()->isCity())
    {
        return false;
    }
    if (!plot()->canHaveFeature((FeatureTypes)GC.getSpellInfo((SpellTypes)spell).getCreateFeatureType()))
    {
        return false;
    }
    if (plot()->getFeatureType() != NO_FEATURE)
    {
        return false;
    }
    if (plot()->getImprovementType() != NO_IMPROVEMENT)
    {
        if (!GC.getCivilizationInfo(getCivilizationType()).isMaintainFeatures(GC.getSpellInfo((SpellTypes)spell).getCreateFeatureType()))
        {
            return false;
        }
    }
    return true;
}

bool CvUnit::canCreateImprovement(int spell) const
{
    if (plot()->isCity())
    {
        return false;
    }
    if (!plot()->canHaveImprovement((ImprovementTypes)GC.getSpellInfo((SpellTypes)spell).getCreateImprovementType()))
    {
        return false;
    }
    if (plot()->getImprovementType() != NO_IMPROVEMENT)
    {
        return false;
    }
    return true;
}

bool CvUnit::canDispel(int spell)
{
    int iRange = GC.getSpellInfo((SpellTypes)spell).getRange();
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            if (NULL != pLoopPlot)
            {
                pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                    if (!pLoopUnit->isImmuneToSpell(this, spell))
                    {
                        for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
                        {
                            if (pLoopUnit->isHasPromotion((PromotionTypes)iI))
                            {
                                if (GC.getPromotionInfo((PromotionTypes)iI).isDispellable())
                                {
                                    if ((GC.getPromotionInfo((PromotionTypes)iI).getAIWeight() < 0 && pLoopUnit->getTeam() == getTeam())
                                    || (GC.getPromotionInfo((PromotionTypes)iI).getAIWeight() > 0 && pLoopUnit->isEnemy(getTeam())))
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
    return false;
}

bool CvUnit::canImmobile(int spell)
{
    int iRange = GC.getSpellInfo((SpellTypes)spell).getRange();
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            if (NULL != pLoopPlot)
            {
                pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                    if (!pLoopUnit->isImmuneToSpell(this, spell))
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool CvUnit::canPush(int spell)
{
    int iRange = GC.getSpellInfo((SpellTypes)spell).getRange();
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            if (NULL != pLoopPlot)
            {
                if (!pLoopPlot->isCity())
                {
                    pUnitNode = pLoopPlot->headUnitNode();
                    while (pUnitNode != NULL)
                    {
                        pLoopUnit = ::getUnit(pUnitNode->m_data);
                        pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                        if (!pLoopUnit->isImmuneToSpell(this, spell))
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

bool CvUnit::canCastHeal(int spell)
{
	CvUnit* pLoopUnit;

    CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
    while (pUnitNode != NULL)
    {
        pLoopUnit = ::getUnit(pUnitNode->m_data);
        pUnitNode = plot()->nextUnitNode(pUnitNode);
        if (!pLoopUnit->isImmuneToSpell(this, spell))
        {
            if (pLoopUnit->isAlive() && pLoopUnit->getDamage()>0)
            {
	            return true;
            }
        }
    }
	return false;
}

bool CvUnit::canRemovePromotion(int spell)
{
   	PromotionTypes ePromotion1 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getRemovePromotionType1();
   	PromotionTypes ePromotion2 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getRemovePromotionType2();
   	PromotionTypes ePromotion3 = (PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getRemovePromotionType3();
    if (plot()->isVisibleEnemyUnit(getOwnerINLINE()))
    {
        if (ePromotion1 == (PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION"))
        {
            return false;
        }
        if (ePromotion2 == (PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION"))
        {
            return false;
        }
        if (ePromotion3 == (PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION"))
        {
            return false;
        }
    }
    if (GC.getSpellInfo((SpellTypes)spell).isBuffCasterOnly())
    {
        if (ePromotion1 != NO_PROMOTION)
        {
            if (isHasPromotion(ePromotion1))
            {
                return true;
            }
        }
        if (ePromotion2 != NO_PROMOTION)
        {
            if (isHasPromotion(ePromotion2))
            {
                return true;
            }
        }
        if (ePromotion3 != NO_PROMOTION)
        {
            if (isHasPromotion(ePromotion3))
            {
                return true;
            }
        }
        return false;
    }
	CvUnit* pLoopUnit;
    CvPlot* pLoopPlot;
    int iRange = GC.getSpellInfo((SpellTypes)spell).getRange();
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            if (NULL != pLoopPlot)
            {
                CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                    if (!pLoopUnit->isImmuneToSpell(this, spell))
                    {
                        if (ePromotion1 != NO_PROMOTION)
                        {
                            if (pLoopUnit->isHasPromotion(ePromotion1))
                            {
                                return true;
                            }
                        }
                        if (ePromotion2 != NO_PROMOTION)
                        {
                            if (pLoopUnit->isHasPromotion(ePromotion2))
                            {
                                return true;
                            }
                        }
                        if (ePromotion3 != NO_PROMOTION)
                        {
                            if (pLoopUnit->isHasPromotion(ePromotion3))
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

bool CvUnit::canSpreadReligion(int spell) const
{
   	ReligionTypes eReligion = (ReligionTypes)GC.getSpellInfo((SpellTypes)spell).getSpreadReligion();
	if (eReligion == NO_RELIGION)
	{
		return false;
	}
	CvCity* pCity = plot()->getPlotCity();
	if (pCity == NULL)
	{
		return false;
	}
	if (pCity->isHasReligion(eReligion))
	{
		return false;
	}

	if (!pCity->canHaveReligion(eReligion))
	{
		return false;
	}

	return true;
}

void CvUnit::cast(int spell, CvPlot* pTargetPlot)
{
	if(spell==NO_SPELL)
	{
		return;
	}

	//TEMP PARALLEL TEST DO LAME STUFF HERE
/**
	//serial
	for(int iI=0;iI<1000;iI++)
	{
		perFormanceTest();
	}
	//parallel
	float* a;
	tbb::parallel_for(tbb::blocked_range<size_t>(0,1000,10000),ApplyFoo(a));
	//TEMP DEBUG DO LAME STUFF HERE
**/

//>>>>BUGFfH: Moved from below(*1) by Denev 2009/09/20
// Shows effect icon before spell effect moves caster's position.

	CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)spell);
	if(pTargetPlot==NULL)
	{
		pTargetPlot=plot();
	}

	if (pTargetPlot->isVisibleToWatchingHuman())
	{
		if(!kSpell.isNoDisplay())
		{
			if (kSpell.getEffect() != -1)
			{
				if(pTargetPlot==plot())
				{
					gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), (float)(GC.getASyncRand().get(360)));
				}
				else
				{
					if(pTargetPlot->getX()<plot()->getX())
					{
						if(plot()->getY()-pTargetPlot->getY()>(plot()->getX()-pTargetPlot->getX())/2)
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 5.50f);
						}
						else if(pTargetPlot->getY()-plot()->getY()>(plot()->getX()-pTargetPlot->getX())/2)
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 0.79f);
						}
						else
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 0.0f);
						}
					}
					else if(pTargetPlot->getX()>plot()->getX())
					{
						if(plot()->getY()-pTargetPlot->getY()>(pTargetPlot->getX()-plot()->getX())/2)
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 3.92f);
						}
						else if(pTargetPlot->getY()-plot()->getY()>(pTargetPlot->getX()-plot()->getX())/2)
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 2.36f);
						}
						else
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 3.14f);
						}
					}
					else
					{
						if(plot()->getY()-pTargetPlot->getY()>0)
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 4.71f);
						}
						else
						{
							gDLL->getEngineIFace()->TriggerEffect(kSpell.getEffect(), pTargetPlot->getPoint(), 1.57f);
						}
					}
				}
			}

			if (kSpell.getSound() != NULL)
			{
				gDLL->getInterfaceIFace()->playGeneralSound(kSpell.getSound(), pTargetPlot->getPoint());
			}
			gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), true, GC.getEVENT_MESSAGE_TIME(), kSpell.getDescription(), "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, kSpell.getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
		}
	}
//<<<<BUGFfH: End Move

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bCastAnySpell                                                                           **/
/** tag bSpell                                                                           **/
/*************************************************************************************************/
	for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
	{
		if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
		{
			CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
			for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
			{
				if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
				{
					if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isSpell() && GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getSpellType()==spell)
					{
						GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
					}

					if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isCastAnySpell())
					{
						if(!kSpell.isAbility() && kSpell.getDamage()>0)
						{
							GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
						}
					}
				}
			}
		}
	}
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/

    if (kSpell.isHasCasted())
    {
        setHasCasted(true);
    }
    for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
    {
        if (isHasPromotion((PromotionTypes)iI))
        {
            if (GC.getPromotionInfo((PromotionTypes)iI).isRemovedByCasting())
            {
                setHasPromotion((PromotionTypes)iI, false);
            }
        }
    }
    if (kSpell.isGlobal())
    {
        GET_PLAYER(getOwnerINLINE()).setFeatAccomplished(FEAT_GLOBAL_SPELL, true);
		for (int iPlayer = 0; iPlayer < MAX_CIV_PLAYERS; ++iPlayer)
		{
		    if (GET_PLAYER((PlayerTypes)iPlayer).isAlive())
		    {
                gDLL->getInterfaceIFace()->addMessage((PlayerTypes)iPlayer, false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_GLOBAL_SPELL", GC.getSpellInfo((SpellTypes)spell).getDescription()), "AS2D_CIVIC_ADOPT", MESSAGE_TYPE_MINOR_EVENT);
		    }
		}
    }
    int iMiscastChance = kSpell.getMiscastChance() + m_pUnitInfo->getMiscastChance();
    if (iMiscastChance > 0)
    {
        if (GC.getGameINLINE().getSorenRandNum(100, "Miscast") < iMiscastChance)
        {
            if (!CvString(kSpell.getPyMiscast()).empty())
            {
                CyUnit* pyUnit = new CyUnit(this);
                CyArgsList argsList;
                argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
                argsList.add(spell);//the spell #
                gDLL->getPythonIFace()->callFunction(PYSpellModule, "miscast", argsList.makeFunctionArgs()); //, &lResult
                delete pyUnit; // python fxn must not hold on to this pointer
            }
            gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_MISCAST"), "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, "art/interface/buttons/spells/miscast.dds", (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
            gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
            return;
        }
    }
    if (kSpell.getDelay() > 0)
    {
        if (getDelayedSpell() == NO_SPELL)
        {
/*************************************************************************************************/
/**	BUGFIX (SpellDelay scales with Gamespeed) Sephi                                	            **/
/*************************************************************************************************/
//          changeImmobileTimer(GC.getSpellInfo((SpellTypes)spell).getDelay());
            int iDelayTurns=kSpell.getDelay();
            iDelayTurns*=GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
            iDelayTurns/=100;
            iDelayTurns=std::max(iDelayTurns, 2);
            changeImmobileTimer(iDelayTurns);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
            setDelayedSpell(spell);
            gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
            gDLL->getInterfaceIFace()->changeCycleSelectionCounter((GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_MOVES)) ? 1 : 2);
            return;
        }
        setDelayedSpell(NO_SPELL);
    }

	if(kSpell.isThiefMission() && isSpy()) {
		int iCoolDown = std::max(5, kSpell.getThiefDifficulty() / 3);
		if(!tryThiefMission((SpellTypes)spell)) {
			//Thief was killed
			GET_PLAYER(getOwnerINLINE()).displayText(gDLL->getText("TXT_KEY_THIEF_CAUGHT"), getX_INLINE(), getY_INLINE());
			if(plot()->isCity()) {
				GET_PLAYER(plot()->getOwnerINLINE()).displayText(gDLL->getText("TXT_KEY_KILLED_THIEF"), getX_INLINE(), getY_INLINE());
			}
			kill(true);
			return;
		}

		if(tryThiefArrested((SpellTypes)spell)) {
			//Thief was Arrested
			GET_PLAYER(getOwnerINLINE()).displayText(gDLL->getText("TXT_KEY_THIEF_ARRESTED"), getX_INLINE(), getY_INLINE());

			if(plot()->isCity()) {
				GET_PLAYER(plot()->getOwnerINLINE()).displayText(gDLL->getText("TXT_KEY_ARRESTED_THIEF"), getX_INLINE(), getY_INLINE());
			}

			if(GET_PLAYER(getOwnerINLINE()).getCapitalCity() != NULL) {
				setXY(GET_PLAYER(getOwnerINLINE()).getCapitalCity()->getX_INLINE(), 
					  GET_PLAYER(getOwnerINLINE()).getCapitalCity()->getY_INLINE());
				changeImmobileTimer(iCoolDown);

			}
			else {
				kill(true);
			}
			return;
		}
		else {
			changeExperience(kSpell.getThiefExperienceChange());
		}
		changeImmobileTimer(iCoolDown);
	}

    if (kSpell.getCreateUnitType() != -1)
    {
        int iUnitNum = kSpell.getCreateUnitNum();
        if (isTwincast())
        {
            iUnitNum *= 2;
        }
        for (int i=0; i < iUnitNum; ++i)
        {
            castCreateUnit(spell);
        }
    }
    if (kSpell.getAddPromotionType1() != -1)
    {
        castAddPromotion(spell,pTargetPlot);
    }
    if (kSpell.getRemovePromotionType1() != -1)
    {
        castRemovePromotion(spell,pTargetPlot);
    }

	if(kSpell.getHeal()>0)
	{
		castHeal(spell, pTargetPlot);
	}

    if (kSpell.getCreateBuildingType() != NO_BUILDING)
    {
        if (pTargetPlot->isCity() && pTargetPlot->getPlotCity()->getNumBuilding((BuildingTypes)kSpell.getCreateBuildingType()) < 1)
        {
            pTargetPlot->getPlotCity()->setNumRealBuilding((BuildingTypes)kSpell.getCreateBuildingType(), true);
        }
    }

    if (kSpell.getCreateBuildingType1() != NO_BUILDING)
    {
        if (pTargetPlot->isCity() && pTargetPlot->getPlotCity()->getNumBuilding((BuildingTypes)kSpell.getCreateBuildingType1()) < 1)
        {
            pTargetPlot->getPlotCity()->setNumRealBuilding((BuildingTypes)kSpell.getCreateBuildingType1(), true);
        }
    }
    if (kSpell.getCreateBuildingType2() != NO_BUILDING)
    {
        if (pTargetPlot->isCity() && pTargetPlot->getPlotCity()->getNumBuilding((BuildingTypes)kSpell.getCreateBuildingType2()) < 1)
        {
            pTargetPlot->getPlotCity()->setNumRealBuilding((BuildingTypes)kSpell.getCreateBuildingType2(), true);
        }
    }

    if (kSpell.getRemoveBuildingType() != NO_BUILDING)
    {
        if (pTargetPlot->isCity() && pTargetPlot->getPlotCity()->getNumBuilding((BuildingTypes)kSpell.getRemoveBuildingType()) > 0)
        {
            pTargetPlot->getPlotCity()->setNumRealBuilding((BuildingTypes)kSpell.getRemoveBuildingType(), 0);
        }
    }

    if (kSpell.getCreateFeatureType() != NO_FEATURE)
    {
        if (canCreateFeature(spell))
        {
            pTargetPlot->setFeatureType((FeatureTypes)kSpell.getCreateFeatureType(), -1);
        }
    }
    if (kSpell.getCreateImprovementType() != NO_IMPROVEMENT)
    {
        if (canCreateImprovement(spell))
        {
            pTargetPlot->setImprovementType((ImprovementTypes)kSpell.getCreateImprovementType());
        }
    }
    if (kSpell.getSpreadReligion() != NO_RELIGION)
    {
        if (canSpreadReligion(spell))
        {
            pTargetPlot->getPlotCity()->setHasReligion((ReligionTypes)kSpell.getSpreadReligion(), true, true, true);

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bSpreadReligion                                                                         **/
/*************************************************************************************************/
            UnitTypes eUnit=getUnitType();
            for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
            {
                if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
                {
                    CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
                    for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
                    {
                        if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
                        {
                            if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isSpreadReligion())
                            {
                                if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getReligionType()==(ReligionTypes)kSpell.getSpreadReligion())
                                {
                                    GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
                                }
                            }
                        }
                    }
                }
            }
/*************************************************************************************************/
/**	END                     	                                        						**/
/*************************************************************************************************/
        }
    }
	if(!(kSpell.isTargetedHostile() && pTargetPlot==NULL))
	{
		castDamage(spell,pTargetPlot);
		//TEMP SOLUTION TO ALLOW ARCANE UNITS XP
		if(pTargetPlot!=plot()) {
			int iXPCap = kSpell.getManaCost() * 2;
			int iChange = std::min(5, iXPCap - getExperience());
			iChange = std::max(2, iChange);
			changeExperience(iChange);
		}
		CvPlot* pAdjacentPlot;
		if(kSpell.getTriggerSecondaryPlotChance()>0)
		{
			for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pAdjacentPlot = plotDirection(pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE(), ((DirectionTypes)iI));

				if(pAdjacentPlot!=NULL && castDamage(spell, pTargetPlot, true)>0 && kSpell.getTriggerSecondaryPlotChance()>GC.getGameINLINE().getSorenRandNum(100,"Trigger Secondary SpellDamage"))
				{
					castDamageSecondaryPlot(spell, pAdjacentPlot, true);
				}
			}
		}
	}
    if (kSpell.isPush())
    {
        castPush(spell);
    }
    if (kSpell.isRemoveHasCasted())
    {
        if (getDuration() == 0)
        {
            setHasCasted(false);
        }
    }
    int iCost = kSpell.getCost();
    if (iCost != 0)
    {
        if (kSpell.getConvertUnitType() != NO_UNIT)
        {
            iCost += (iCost * GET_PLAYER(getOwnerINLINE()).getUpgradeCostModifier()) / 100;
        }
/*************************************************************************************************/
/**	BUGFIX (Spell Costs scale with Gamespeed) Sephi                                	            **/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
        iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
		iCost /=100;
		iCost =std::max(1,iCost);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
        GET_PLAYER(getOwnerINLINE()).changeGold(-1 * iCost);
    }

/*************************************************************************************************/
/**	ADDON (New Mana) Sephi                                          					**/
/*************************************************************************************************/
    GET_PLAYER(getOwnerINLINE()).changeFaith(-GET_PLAYER((PlayerTypes)getOwnerINLINE()).getSpellFaithCost((SpellTypes)spell,this));
    GET_PLAYER(getOwnerINLINE()).changeMana(-GET_PLAYER((PlayerTypes)getOwnerINLINE()).getSpellManaCost((SpellTypes)spell,this));
	PromotionTypes eWillpower=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_WILLPOWER");
	if(eWillpower!=NO_PROMOTION && kSpell.getManaCost()>0)
	{
		setHasPromotion(eWillpower,false);
	}


	if (kSpell.isBombard())
	{
		if(pTargetPlot->isCity())
		{
			pTargetPlot->getPlotCity()->changeDefenseDamage(pTargetPlot->getPlotCity()->getDefenseModifier(false)/7);
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

    if (kSpell.getChangePopulation() != 0)
    {
        pTargetPlot->getPlotCity()->changePopulation(GC.getSpellInfo((SpellTypes)spell).getChangePopulation());
    }
/*************************************************************************************************/
/**	ADDON (Houses of Erebus) Sephi			                                 					**/
/*************************************************************************************************/
	for (int i=0;i<GC.getNumCorporationInfos();i++)
	{
        if (kSpell.getCorporationSupport((CorporationTypes)i)!=0)
        {
            GET_PLAYER(getOwnerINLINE()).changeCorporationSupport((CorporationTypes)i,GC.getSpellInfo((SpellTypes)spell).getCorporationSupport((CorporationTypes)i));
        }
	}

	for (int i=0;i<GC.getNumCorporationInfos();i++)
	{
        if (kSpell.getCorporationSupportMultiplier((CorporationTypes)i)!=0)
        {
            GET_PLAYER(getOwnerINLINE()).changeCorporationSupportMultiplier((CorporationTypes)i,GC.getSpellInfo((SpellTypes)spell).getCorporationSupportMultiplier((CorporationTypes)i));
        }
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	if (!CvString(kSpell.getPyResult()).empty())
    {
	//DEBUG
	if(isOOSLogging())	
	{
	oosLog(
	"AIUnitCast"
	,"Turn: %d,PlayerID: %d,UnitID: %d,cast: %s\n"
	,GC.getGameINLINE().getElapsedGameTurns()
	,getOwner()
	,getID()
	,CvString(kSpell.getPyResult()).GetCString()
	);
	}
	//DEBUG
        CyUnit* pyUnit = new CyUnit(this);
        CyArgsList argsList;
        argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
        argsList.add(spell);//the spell #
        gDLL->getPythonIFace()->callFunction(PYSpellModule, "cast", argsList.makeFunctionArgs()); //, &lResult
        delete pyUnit; // python fxn must not hold on to this pointer
    }

//BUGFFH: Added by Denev 2009/09/20
	if (!CvString(kSpell.getPyResultSecondary()).empty())
	{
	//DEBUG
	if(isOOSLogging())	
	{
	oosLog(
	"AIUnitCast"
	,"Turn: %d,PlayerID: %d,UnitID: %d,castSecondary: %s\n"
	,GC.getGameINLINE().getElapsedGameTurns()
	,getOwner()
	,getID()
	,CvString(kSpell.getPyResultSecondary()).GetCString()
	);
	}
	//DEBUG
		CyUnit* pyUnit = new CyUnit(this);
		CyArgsList argsList;
		argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
		argsList.add(spell);//the spell #
		gDLL->getPythonIFace()->callFunction(PYSpellModule, "castSecondary", argsList.makeFunctionArgs()); //, &lResult
		delete pyUnit; // python fxn must not hold on to this pointer
	}
//BUGFFH: End Add
//>>>>BUGFfH: Moved to above(*1) by Denev 2009/09/20
/*
	if (plot()->isVisibleToWatchingHuman())
	{
		if (GC.getSpellInfo((SpellTypes)spell).getEffect() != -1)
		{
			gDLL->getEngineIFace()->TriggerEffect(GC.getSpellInfo((SpellTypes)spell).getEffect(), plot()->getPoint(), (float)(GC.getASyncRand().get(360)));
		}
		if (GC.getSpellInfo((SpellTypes)spell).getSound() != NULL)
		{
			gDLL->getInterfaceIFace()->playGeneralSound(GC.getSpellInfo((SpellTypes)spell).getSound(), plot()->getPoint());
		}
		gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), true, GC.getEVENT_MESSAGE_TIME(), GC.getSpellInfo((SpellTypes)spell).getDescription(), "AS2D_WONDER_UNIT_BUILD", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
	}
*/
//<<<<BUGFfH: End Move
    if (kSpell.getConvertUnitType() != NO_UNIT)
    {
        castConvertUnit(spell);
    }

    gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
    if (kSpell.isSacrificeCaster())
    {
        kill(true);
    }

	if(spell==GC.getInfoTypeForString("SPELL_PICK_EQUIPMENT"))
	{
		//important not to cycle
		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		//gDLL->getInterfaceIFace()->changeCycleSelectionCounter((GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_MOVES)) ? 1 : 2);
		return;
	}
//>>>>Spell Interrupt Unit Cycling: Added by Denev 2009/10/17
/*	Casting spell triggers unit cycling	*/
	if (!isDelayedDeath() && IsSelected())
	{
		if(getGroup())
		{
			if (!getGroup()->readyToSelect(true) && !getGroup()->isBusy())
			{
		//		gDLL->getInterfaceIFace()->setDirty(SelectionButtons_DIRTY_BIT, true);
		//		gDLL->getInterfaceIFace()->changeCycleSelectionCounter((GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_MOVES)) ? 1 : 2);
				
				GC.getGameINLINE().updateSelectionList();
			}
		}
	}
//<<<<Spell Interrupt Unit Cycling: End Add

}

void CvUnit::castAddPromotion(int spell, CvPlot* pTargetplot)
{
	if(GC.getSpellInfo((SpellTypes)spell).isTargetedHostile())
	{
		return;
	}
	CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)spell);
	PromotionTypes ePromotion=NO_PROMOTION;
	for(int iI=0;iI<5;iI++)
	{
		if(iI==0)
		   	ePromotion = (PromotionTypes)kSpell.getAddPromotionType1();
		if(iI==1)
		   	ePromotion = (PromotionTypes)kSpell.getAddPromotionType2();
		if(iI==2)
		   	ePromotion = (PromotionTypes)kSpell.getAddPromotionType3();
		if(iI==3)
		   	ePromotion = (PromotionTypes)kSpell.getAddPromotionType4();
		if(iI==4)
		   	ePromotion = (PromotionTypes)kSpell.getAddPromotionType5();

		if(ePromotion!=NO_PROMOTION)
		{
			if (GC.getSpellInfo((SpellTypes)spell).isBuffCasterOnly())
			{
				if (canAddPromotiontoUnit(ePromotion))
				{
					if(GC.getPromotionInfo(ePromotion).getEquipmentCategory()==NO_EQUIPMENTCATEGORY)
					{
						setHasPromotion(ePromotion, true);
					}
					else
					{
						equip(ePromotion,true);
					}
				}
			}

		    else if(pTargetplot!=NULL)
			{
				bool bResistable = kSpell.isResistable();
				CvUnit* pLoopUnit;
				CLLNode<IDInfo>* pUnitNode;
				pUnitNode = pTargetplot->headUnitNode();
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pTargetplot->nextUnitNode(pUnitNode);
					if (!pLoopUnit->isImmuneToSpell(this, spell))
					{
						if (pLoopUnit->getTeam()==getTeam() || !bResistable || !pLoopUnit->isResisted(this, spell))
						{
							if (pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT)
							{
								if (GC.getPromotionInfo(ePromotion).getUnitCombat(pLoopUnit->getUnitCombatType()))
								{
									if (pLoopUnit->canAddPromotiontoUnit(ePromotion))
									{
										if(GC.getPromotionInfo(ePromotion).getEquipmentCategory()==NO_EQUIPMENTCATEGORY)
										{
											pLoopUnit->setHasPromotion(ePromotion, true);
										}
										else
										{
											pLoopUnit->equip(ePromotion,true);
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

void CvUnit::applyDamageCast(CvUnit* pCaster,int spell, int iDamage)
{
	CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)spell);
    int iDmg = iDamage;
    int iDmgLimit = kSpell.getDamageLimit();
    int iDmgType = kSpell.getDamageType();
    bool bResistable = kSpell.isResistable();

	if(iDmgType==NO_DAMAGE)
	{
		iDmgType=DAMAGE_PHYSICAL;
	}

	doDamage(iDmg, iDmgLimit, pCaster, iDmgType, true,false,true);

	//resist Spell?
	if (bResistable && isResisted(pCaster,spell))
	{
		return;
	}

	if(kSpell.getDoT()!=NO_PROMOTION)
	{
		setHasPromotion((PromotionTypes)kSpell.getDoT(),true);
	}

	if (kSpell.getImmobileTurns() > 0)
	{
		changeImmobileTimer(kSpell.getImmobileTurns());
		gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_IMMOBILE"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
		if(pCaster!=NULL)
		{
			gDLL->getInterfaceIFace()->addMessage((PlayerTypes)pCaster->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_IMMOBILE"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
		}
	}
	if(kSpell.getForcedTeleport() > 0 && !plot()->isCity())
	{
		int iSearchRange=std::min(5,kSpell.getForcedTeleport());
		int iBestValue=-MAX_INT;
		int iValue;
		CvPlot* pBestPlot=NULL;
		for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
		{
			for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
			{
				CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
				if(pLoopPlot!=NULL && pLoopPlot->getArea()==plot()->getArea() && canMoveInto(pLoopPlot))
				{
					iValue=GC.getGameINLINE().getSorenRandNum(100,"Trigger Forced Movement");
					if(iValue>iBestValue)
					{
						iBestValue=iValue;
						pBestPlot=pLoopPlot;
					}
				}
			}
		}
		if(pBestPlot!=NULL)
		{
			setXY(pBestPlot->getX_INLINE(),pBestPlot->getY_INLINE());
		}
	}

	if(kSpell.isDispel())
	{
		if(pCaster==NULL || pCaster->getTeam()!=getTeam())
		{
			for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
			{
				CvPromotionInfo& kPromotion=GC.getPromotionInfo((PromotionTypes)iI);
				if(kPromotion.isDispellable())
				{
					setHasPromotion((PromotionTypes)iI,false);
				}
			}

			if(getBlessing1()!=NULL)
			{
				getBlessing1()->Remove(this);
			}

			if(getBlessing2()!=NULL)
			{
				getBlessing2()->Remove(this);
			}
		}
	}

   	PromotionTypes ePromotion1 = (PromotionTypes)kSpell.getAddPromotionType1();
   	PromotionTypes ePromotion2 = (PromotionTypes)kSpell.getAddPromotionType2();
   	PromotionTypes ePromotion3 = (PromotionTypes)kSpell.getAddPromotionType3();
   	PromotionTypes ePromotion4 = (PromotionTypes)kSpell.getAddPromotionType4();
   	PromotionTypes ePromotion5 = (PromotionTypes)kSpell.getAddPromotionType5();
	if(ePromotion1!=NO_PROMOTION)
		setHasPromotion(ePromotion1,true);
	if(ePromotion2!=NO_PROMOTION)
		setHasPromotion(ePromotion1,true);
	if(ePromotion3!=NO_PROMOTION)
		setHasPromotion(ePromotion1,true);
	if(ePromotion4!=NO_PROMOTION)
		setHasPromotion(ePromotion1,true);
	if(ePromotion5!=NO_PROMOTION)
		setHasPromotion(ePromotion1,true);

	if (kSpell.isDomination() || 
		(GC.getUnitInfo(getUnitType()).getUnitCombatType()!=NO_UNITCOMBAT && kSpell.getUnitCombatCapture()==GC.getUnitInfo(getUnitType()).getUnitCombatType()))
	{
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(),true,GC.getEVENT_MESSAGE_TIME(),gDLL->getText("TXT_KEY_MESSAGE_DOMINATION"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(),(ColorTypes)GC.getInfoTypeForString("COLOR_RED"),getX(),getY(),true,true);
		gDLL->getInterfaceIFace()->addMessage(pCaster->getOwnerINLINE(),true,GC.getEVENT_MESSAGE_TIME(),gDLL->getText("TXT_KEY_MESSAGE_DOMINATION_ENEMY"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"),pCaster->getX(),pCaster->getY(),true,true);
		CvUnit* newUnit = GET_PLAYER((PlayerTypes)pCaster->getOwnerINLINE()).initUnit(getUnitType(), pCaster->getX(), pCaster->getY(), NO_UNITAI, DIRECTION_SOUTH);
		newUnit->convert(this);
		newUnit->changeImmobileTimer(1);
	}
}

int CvUnit::castDamage_addSpecialValue(int spell, CvUnit* pTarget, int iModifier)
{
	CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)spell);
	PromotionTypes iDoTPromotion=(PromotionTypes)kSpell.getDoT();

	int iValue=0;

	if (kSpell.getImmobileTurns() > 0 )
	{
		iValue+=250*kSpell.getImmobileTurns()*iModifier;
	}

	if (kSpell.getForcedTeleport() > 0 )
	{
		iValue+=250*kSpell.getForcedTeleport()*iModifier;
	}

	if(kSpell.isDomination() || kSpell.getUnitCombatCapture()==pTarget->getUnitCombatType())
	{
		iValue+=std::max(0,400*iModifier);
	}

   	PromotionTypes ePromotion1 = (PromotionTypes)kSpell.getAddPromotionType1();
   	PromotionTypes ePromotion2 = (PromotionTypes)kSpell.getAddPromotionType2();
   	PromotionTypes ePromotion3 = (PromotionTypes)kSpell.getAddPromotionType3();
   	PromotionTypes ePromotion4 = (PromotionTypes)kSpell.getAddPromotionType4();
   	PromotionTypes ePromotion5 = (PromotionTypes)kSpell.getAddPromotionType5();
	if(ePromotion1!=NO_PROMOTION && !pTarget->isHasPromotion(ePromotion1))
		iValue+=100;
	if(ePromotion2!=NO_PROMOTION && !pTarget->isHasPromotion(ePromotion2))
		iValue+=100;
	if(ePromotion3!=NO_PROMOTION && !pTarget->isHasPromotion(ePromotion3))
		iValue+=100;
	if(ePromotion4!=NO_PROMOTION && !pTarget->isHasPromotion(ePromotion4))
		iValue+=100;
	if(ePromotion5!=NO_PROMOTION && !pTarget->isHasPromotion(ePromotion5))
		iValue+=100;

	if (iDoTPromotion!=NO_PROMOTION && (!pTarget->isHasPromotion(iDoTPromotion)))
	{
		iValue+=200*iModifier;
	}

	if(kSpell.isDispel())
	{
		if(getTeam()!=pTarget->getTeam())
		{
			for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
			{
				CvPromotionInfo& kPromotion=GC.getPromotionInfo((PromotionTypes)iI);
				if(kPromotion.isDispellable())
				{
					iValue+=100;
				}
			}

			if(getBlessing1()!=NULL)
			{
				iValue+=100;
			}

			if(getBlessing2()!=NULL)
			{
				iValue+=100;
			}
		}
	}

	return iValue;
}

int CvUnit::castDamage(int spell, CvPlot* pTargetplot, bool bCountDamage)
{
	CvSpellInfo &kSpell = GC.getSpellInfo((SpellTypes)spell);
    bool bResistable = kSpell.isResistable();
    int iDmg = kSpell.getDamage();
	int iCritChance = getSpellCritChance(kSpell);
	PromotionTypes iDoTPromotion = (PromotionTypes)kSpell.getDoT();
    int iDmgLimit = kSpell.getDamageLimit();
    int iDmgType = kSpell.getDamageType();
    int iRange = kSpell.getRange();
	int iNumTargets = getSpellTargetNumber(kSpell);

	int iDamageCount = 0;
	int iMod = 1;

	bool* bUnitHit = NULL;

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	if (NULL != pTargetplot)
    {
        if (pTargetplot->getX() != plot()->getX() || pTargetplot->getY() != plot()->getY())
        {
			if(iNumTargets == -1)
			{
				pUnitNode = pTargetplot->headUnitNode();
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pTargetplot->nextUnitNode(pUnitNode);
					if (pLoopUnit != NULL && !pLoopUnit->isImmuneToSpell(this, spell))
					{
						if(kSpell.isCausesWar() || GET_TEAM(getTeam()).isAtWar(pLoopUnit->getTeam()))
						{
							if(pLoopUnit->getTeam() == getTeam())
							{
								iMod =- 2;
							}
							else
							{
								iMod = 1;
							}

							int iDamageModifier = 1;
							if(!bCountDamage)
							{
//							if(!GC.getGameINLINE().isNetworkMultiPlayer())
//							{
								if(iCritChance>GC.getGameINLINE().getSorenRandNum(100,"Spell Crit Chance"))
								{
									iDamageModifier = 3;
									if(!bCountDamage)
									{
	                                    gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), gDLL->getText("TXT_KEY_MESSAGE_CRITICAL_HIT"), "", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
									}
								}
							}
							int iBonusDamage = GET_PLAYER(getOwnerINLINE()).getBonusSpellDamage(spell);
							int iDamage = (iDmg + iBonusDamage) * iDamageModifier;

							iDamageCount += iMod * pLoopUnit->doDamage(iDamage,iDmgLimit,this,iDmgType,false,true,true);
							iDamageCount += castDamage_addSpecialValue(spell,pLoopUnit,iMod);

							if(!bCountDamage)
							{
								pLoopUnit->applyDamageCast(this, spell, iDamage);
							}
						}
					}
				}
			}
			else
			{
				int iUnitsOnPlot=pTargetplot->getNumUnits();
				int iValue;
				int iBestValue=0;
				int iBestUnitCounter=-1;
				CvUnit* pBestUnit=NULL;
				int iNumUnitsHit=0;

				bUnitHit = new bool[iUnitsOnPlot];

				for(int i=0;i<iUnitsOnPlot;i++)
				{
					bUnitHit[i]=false;
				}

				for(int iI=0;iI<std::min(iUnitsOnPlot,iNumTargets);iI++)
				{
					pUnitNode = pTargetplot->headUnitNode();
					int iCounter = -1;
					iBestValue=0;
					pBestUnit=NULL;
					while (pUnitNode != NULL)
					{
						iCounter++; // Start at 0
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = pTargetplot->nextUnitNode(pUnitNode);
						if (!pLoopUnit->isImmuneToSpell(this, spell))
						{
							if(kSpell.isCausesWar() || GET_TEAM(getTeam()).isAtWar(pLoopUnit->getTeam()))
							{
								if(!bUnitHit[iCounter])
								{
									iValue = getSpellDefenderValue(pLoopUnit, pTargetplot, iDmgType);
									iValue+=castDamage_addSpecialValue(spell,pLoopUnit);

									if(iValue>iBestValue)
									{
										iBestValue=iValue;
										pBestUnit=pLoopUnit;
										iBestUnitCounter=iCounter;
									}
								}
							}
						}
					}

					if(pBestUnit!=NULL)
					{
						if(pBestUnit->getTeam()==getTeam())
						{
							iMod=-2;
						}
						else
						{
							iMod=1;
						}

						int iDamageModifier=1;
						if(!bCountDamage)
						{
//						if(!GC.getGameINLINE().isNetworkMultiPlayer())
//						{
							if(iCritChance>GC.getGameINLINE().getSorenRandNum(100,"Spell Crit Chance"))
							{
								iDamageModifier=3;
								if(!bCountDamage)
								{
	                               gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), gDLL->getText("TXT_KEY_MESSAGE_CRITICAL_HIT"), "", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
								}
							}
						}
						int iBonusDamage=GET_PLAYER(getOwnerINLINE()).getBonusSpellDamage(spell);
						int iDamage=(iDmg+iBonusDamage)*iDamageModifier;

						for (int iModCounter = 0; iModCounter < iMod; iModCounter++) {
							iDamageCount += pBestUnit->doDamage(iDamage, iDmgLimit, this, iDmgType, false, true, true);
						}
						iDamageCount+=castDamage_addSpecialValue(spell,pLoopUnit,iMod);

						if(!bCountDamage)
						{
							pBestUnit->applyDamageCast(this, spell,iDamage);
						}
						bUnitHit[iBestUnitCounter]=true;
					}
				}
				SAFE_DELETE_ARRAY(bUnitHit);
			}
			//make sure Secondary Effects also Count as Damage
			if(kSpell.isFlameFeatures() && pTargetplot->getFeatureType()!=NO_FEATURE && pTargetplot->getImprovementType()==NO_IMPROVEMENT)
			{
				if(pTargetplot->getFeatureType()==GC.getInfoTypeForString("FEATURE_JUNGLE") || pTargetplot->getFeatureType()==GC.getInfoTypeForString("FEATURE_FOREST"))
				{
					if(!kSpell.isTargetedHostile() || pTargetplot->getTeam()!=getTeam())
					{
						ImprovementTypes iSmoke=(ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_SMOKE");
						if(pTargetplot->getImprovementType()!=iSmoke)
						{
							iDamageCount+=100;
							if(!bCountDamage)
							{
								pTargetplot->setImprovementType(iSmoke);
							}
						}
					}
				}
			}

			if(kSpell.isBombard() && pTargetplot->isCity() && pTargetplot->getPlotCity()->getDefenseModifier(false)>0)
			{
				if(GET_TEAM(getTeam()).isAtWar(pTargetplot->getTeam()))
				{
					iDamageCount+=100;
				}
			}
        }
    }

	if(bUnitHit!=NULL)
	{
		SAFE_DELETE_ARRAY(bUnitHit);
	}

	return iDamageCount;
}

int CvUnit::getSpellTargetNumber(CvSpellInfo &kSpell) const {
	int iNumTargets = kSpell.getNumTargets();
	if (iNumTargets != -1)
	{
		iNumTargets *= 100 + getSpellTargetIncrease();
		iNumTargets /= 100;
	}

	return iNumTargets;
}

int CvUnit::getSpellCritChance(CvSpellInfo &kSpell) const {
	return ((getUnitInfo().getTier() - 1) * 5) * ((kSpell.isHighCrit()) ? 2 : 1);
}

void CvUnit::castDamageSecondaryPlot(int spell, CvPlot* pTargetPlot, bool secondaryPlot)
{
	castDamage(spell, pTargetPlot);

	if (pTargetPlot->isVisibleToWatchingHuman())
	{
		if (GC.getSpellInfo((SpellTypes)spell).getEffect() != -1)
		{
			gDLL->getEngineIFace()->TriggerEffect(GC.getSpellInfo((SpellTypes)spell).getEffect(), pTargetPlot->getPoint(), (float)(GC.getASyncRand().get(360)));
		}
		if (GC.getSpellInfo((SpellTypes)spell).getSound() != NULL)
		{
			gDLL->getInterfaceIFace()->playGeneralSound(GC.getSpellInfo((SpellTypes)spell).getSound(), pTargetPlot->getPoint());
		}
	}

	//I hate fun; more like in a test this caused the spell to chain to something like 8 additional plots, plus it chained back onto plots more than once, so its way more powerful than I would like and very hard for an ai to eventually calculate.
/*	CvPlot* pAdjacentPlot;
	CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)spell);
	if(!secondaryPlot && kSpell.getTriggerSecondaryPlotChance()>0)
	{
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			pAdjacentPlot = plotDirection(pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE(), ((DirectionTypes)iI));

			if(pAdjacentPlot!=NULL && castDamage(spell, pTargetPlot, true)>0 && kSpell.getTriggerSecondaryPlotChance()>GC.getGameINLINE().getSorenRandNum(100,"Trigger Secondary SpellDamage"))
			{
				castDamageSecondaryPlot(spell, pAdjacentPlot, true);
			}
		}
	}*/
}

void CvUnit::castDispel(int spell)
{
    bool bResistable = GC.getSpellInfo((SpellTypes)spell).isResistable();
    int iRange = GC.getSpellInfo((SpellTypes)spell).getRange();
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            if (NULL != pLoopPlot)
            {
                pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                    if (!pLoopUnit->isImmuneToSpell(this, spell))
                    {
                        if (pLoopUnit->isEnemy(getTeam()))
                        {
                            if (bResistable)
                            {
                                if (pLoopUnit->isResisted(this, spell))
                                {
                                    continue;
                                }
                            }
                            for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
                            {
                                if (GC.getPromotionInfo((PromotionTypes)iI).isDispellable() && GC.getPromotionInfo((PromotionTypes)iI).getAIWeight() > 0)
                                {
                                    pLoopUnit->setHasPromotion((PromotionTypes)iI, false);
                                }
                            }
                        }
                        else
                        {
                            for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
                            {
                                if (GC.getPromotionInfo((PromotionTypes)iI).isDispellable() && GC.getPromotionInfo((PromotionTypes)iI).getAIWeight() < 0)
                                {
                                    pLoopUnit->setHasPromotion((PromotionTypes)iI, false);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void CvUnit::castPush(int spell)
{
    bool bResistable = GC.getSpellInfo((SpellTypes)spell).isResistable();
    int iRange = GC.getSpellInfo((SpellTypes)spell).getRange();
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	CvPlot* pPushPlot;
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            int iPushY = plot()->getY_INLINE() + (i*2);
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            pPushPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i*2, j*2);
            if (!pLoopPlot->isCity())
            {
                if (NULL != pLoopPlot)
                {
                    if (NULL != pPushPlot)
                    {
                        if (pLoopPlot->getX() != plot()->getX() || pLoopPlot->getY() != plot()->getY())
                        {
                            pUnitNode = pLoopPlot->headUnitNode();
                            while (pUnitNode != NULL)
                            {
                                pLoopUnit = ::getUnit(pUnitNode->m_data);
                                pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                                if (pLoopUnit->canMoveInto(pPushPlot, false, false, false))
                                {
                                    if (!pLoopUnit->isImmuneToSpell(this, spell))
                                    {
                                        if (bResistable)
                                        {
                                            if (!pLoopUnit->isResisted(this, spell))
                                            {
                                                pLoopUnit->setXY(pPushPlot->getX(),pPushPlot->getY(),false,true,true);
                                                gDLL->getInterfaceIFace()->addMessage((PlayerTypes)pLoopUnit->getOwner(), false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_PUSH"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
                                                gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_PUSH"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
                                            }
                                        }
                                        else
                                        {
                                            pLoopUnit->setXY(pPushPlot->getX(),pPushPlot->getY(),false,true,true);
                                            gDLL->getInterfaceIFace()->addMessage((PlayerTypes)pLoopUnit->getOwner(), false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_PUSH"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
                                            gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), false, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_PUSH"), "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MAJOR_EVENT, GC.getSpellInfo((SpellTypes)spell).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
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

void CvUnit::castHeal(int spell, CvPlot* pTargetPlot)
{
	CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)spell);
	int iAmountToHeal=kSpell.getHeal();
	if(kSpell.getTechPrereq()!=NO_TECH && GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus()!=NO_BONUS)
	{
		iAmountToHeal*=100+10*GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses((BonusTypes)GC.getTechInfo((TechTypes)kSpell.getTechPrereq()).getPrereqBonus());
		iAmountToHeal/=100;
	}

	if(pTargetPlot!=NULL)
	{
		CvUnit* pLoopUnit;
		CLLNode<IDInfo>* pUnitNode;
		pUnitNode = pTargetPlot->headUnitNode();
		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pTargetPlot->nextUnitNode(pUnitNode);
			if (!pLoopUnit->isImmuneToSpell(this, spell))
			{
				if(pLoopUnit->isAlive() && pLoopUnit->getDamage()>0)
				{
					int iHeal=std::min(pLoopUnit->getDamage(),iAmountToHeal);
					pLoopUnit->changeDamage(-iHeal,getOwnerINLINE());
					if(iAmountToHeal<=0)
						break;
				}
			}
		}
	}
}

void CvUnit::castRemovePromotion(int spell, CvPlot* pTargetPlot)
{
	CvSpellInfo &kSpell=GC.getSpellInfo((SpellTypes)spell);
	PromotionTypes ePromotion=NO_PROMOTION;
	for(int iI=0;iI<3;iI++)
	{
		if(iI==0)
		   	ePromotion = (PromotionTypes)kSpell.getRemovePromotionType1();
		if(iI==1)
		   	ePromotion = (PromotionTypes)kSpell.getRemovePromotionType2();
		if(iI==2)
		   	ePromotion = (PromotionTypes)kSpell.getRemovePromotionType3();

		if(ePromotion!=NO_PROMOTION)
		{
			if (GC.getSpellInfo((SpellTypes)spell).isBuffCasterOnly())
			{
				setHasPromotion(ePromotion, false);
			}

		    else if(pTargetPlot!=NULL)
			{
				bool bResistable = kSpell.isResistable();
				CvUnit* pLoopUnit;
				CLLNode<IDInfo>* pUnitNode;
				pUnitNode = pTargetPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = pTargetPlot->nextUnitNode(pUnitNode);
					if (!pLoopUnit->isImmuneToSpell(this, spell))
					{
						if (pLoopUnit->getTeam()==getTeam() || !bResistable || !pLoopUnit->isResisted(this, spell))
						{
							pLoopUnit->setHasPromotion(ePromotion, false);
						}
					}
                }
            }
        }
    }
}

void CvUnit::castConvertUnit(int spell)
{
	CvUnit* pUnit;
   	pUnit = GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)GC.getSpellInfo((SpellTypes)spell).getConvertUnitType(), getX_INLINE(), getY_INLINE(), AI_getUnitAIType());
   	pUnit->convert(this);
    pUnit->changeImmobileTimer(1);
/*************************************************************************************************/
/**	BETTER AI (Doviello Worker needs decent UNITAI after upgrade) Sephi            	            **/
/*************************************************************************************************/
    if(pUnit->AI_getUnitAIType()==UNITAI_WORKER)
    {
        int iWorkerClass = GC.getUNITCLASS_WORKER();

        if (iWorkerClass != NO_UNITCLASS && pUnit->getUnitClassType()!=iWorkerClass)
        {
            pUnit->AI_setUnitAIType(UNITAI_ATTACK_CITY);
            if (!isHuman())
            {
                pUnit->AI_setGroupflag(GROUPFLAG_CONQUEST);
            }
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
}

void CvUnit::castCreateUnit(int spell)
{
    int iI;
	CvUnit* pUnit;
   	pUnit = GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)GC.getSpellInfo((SpellTypes)spell).getCreateUnitType(), getX_INLINE(), getY_INLINE(), UNITAI_ATTACK);
    pUnit->setSummoner(getID());

/*************************************************************************************************/
/**	BETTER AI (Flag for Permanent Summons) Sephi                                  	            **/
/*************************************************************************************************/
   	if (GC.getSpellInfo((SpellTypes)spell).isPermanentUnitCreate())
    {
        pUnit->setPermanentSummon(true);
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

   	if (GC.getSpellInfo((SpellTypes)spell).isPermanentUnitCreate())
   	{
        pUnit->changeImmobileTimer(2);
   	}
   	else
   	{
        pUnit->changeDuration(2);
        if (plot()->getTeam() != getTeam() && pUnit->getUnitInfo().isCitySupportNeeded())
        {
			/*if(isOOSLogging())
			{
				oosLog("OutSideUnit"
					,"Turn:%d,Player:%d,Unit:%d,Name:%S,X:%d,Y:%d,numOutsideUnits:%d,castCreateUnit:-1"
					,GC.getGameINLINE().getElapsedGameTurns()
					,getOwnerINLINE()
					,getID()
					,getName().GetCString()
					,plot()->getX_INLINE()
					,plot()->getY_INLINE()
					,GET_PLAYER(getOwnerINLINE()).getNumOutsideUnits()
				);
			}*/
            GET_PLAYER(getOwnerINLINE()).changeNumOutsideUnits(-1);
        }
   	}
	for (iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
	    if (isHasPromotion((PromotionTypes)iI))
	    {
            if (GC.getSpellInfo((SpellTypes)spell).isCopyCastersPromotions())
            {
                if (!GC.getPromotionInfo((PromotionTypes)iI).isEquipment() && !GC.getPromotionInfo((PromotionTypes)iI).isRace() && iI != GC.getDefineINT("GREAT_COMMANDER_PROMOTION"))
                {
                    pUnit->setHasPromotion((PromotionTypes)iI, true);
                }
            }
//>>>>BUGFfH: Modified by Denev 2009/10/08
/*	if summoning action is not a spell (e.g. Hire Goblin), summoner's promotion doesn't affect to summoned creature.	*/
//			else
			else if (!GC.getSpellInfo((SpellTypes)spell).isAbility())
//<<<<BUGFfH: End Modify
            {
                if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionSummonPerk() != NO_PROMOTION)
                {
                    pUnit->setHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionSummonPerk(), true);
                }
            }
	    }
	}
   	if (GC.getSpellInfo((SpellTypes)spell).getCreateUnitPromotion() != NO_PROMOTION)
   	{
        pUnit->setHasPromotion((PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getCreateUnitPromotion(), true);
   	}
   	pUnit->doTurn();
   	if (!isHuman())
   	{
/*************************************************************************************************/
/**	BETTER AI (Better use of Summons) Sephi                                       	            **/
/*************************************************************************************************/
        //let Tiger Construct Cages
        if(isAnimal())
        {
            if(!GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(plot(), 2))
            {
                AI_setGroupflag(GROUPFLAG_ANIMAL_CONSTRUCTBUILDING);
            }
        }

        if (pUnit->getDuration()>0)
        {
            if (pUnit->getMoves()>0)
            {
                pUnit->joinGroup(getGroup(),false,true);
            }
        }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
   	}
}

bool CvUnit::isHasCasted() const
{
	return m_bHasCasted;
}

void CvUnit::setHasCasted(bool bNewValue)
{
	m_bHasCasted = bNewValue;
}

bool CvUnit::isIgnoreHide() const
{
	return m_bIgnoreHide;
}

void CvUnit::setIgnoreHide(bool bNewValue)
{
	m_bIgnoreHide = bNewValue;
}

bool CvUnit::isImmuneToSpell(CvUnit* pCaster, int spell) const
{
    if (isImmuneToMagic())
    {
        if (!GC.getSpellInfo((SpellTypes)spell).isAbility())
        {
            return true;
        }
    }
   	if (GC.getSpellInfo((SpellTypes)spell).isImmuneTeam())
   	{
   	    if (getTeam() == pCaster->getTeam())
   	    {
   	        return true;
   	    }
   	}
   	if (GC.getSpellInfo((SpellTypes)spell).isImmuneNeutral())
   	{
   	    if (getTeam() != pCaster->getTeam())
   	    {
   	        if (!isEnemy(pCaster->getTeam()))
            {
                return true;
            }
        }
   	}
   	if (GC.getSpellInfo((SpellTypes)spell).isImmuneEnemy())
   	{
        if (isEnemy(pCaster->getTeam()))
        {
            return true;
        }
   	}
   	if (GC.getSpellInfo((SpellTypes)spell).isImmuneFlying())
   	{
   	    if (isFlying())
   	    {
   	        return true;
   	    }
   	}
   	if (GC.getSpellInfo((SpellTypes)spell).isImmuneNotAlive())
   	{
   	    if (!isAlive())
   	    {
   	        return true;
   	    }
   	}

	if (GC.getSpellInfo((SpellTypes)spell).getDamageSpellPrereqPromotion()!=NO_PROMOTION)
	{
		if(!isHasPromotion((PromotionTypes)GC.getSpellInfo((SpellTypes)spell).getDamageSpellPrereqPromotion()))
		{
			return true;
		}
	}

	if (GC.getSpellInfo((SpellTypes)spell).getDamageSpellPrereqUnitCombat()!=NO_PROMOTION)
	{
		if(getUnitCombatType()!=GC.getSpellInfo((SpellTypes)spell).getDamageSpellPrereqUnitCombat())
		{
			return true;
		}
	}

	if(!(canAttack() || canDefend()))
	{
		return true;
	}

	if (isInvisible(pCaster->getTeam(), false))
	{
		return true;
	}

	if(isDelayedDeath())
	{
		return true;
	}

    return false;
}

int CvUnit::getDelayedSpell() const
{
	return m_iDelayedSpell;
}

void CvUnit::setDelayedSpell(int iNewValue)
{
	m_iDelayedSpell = iNewValue;
}

int CvUnit::getDuration() const
{
	return m_iDuration;
}

void CvUnit::setDuration(int iNewValue)
{
	m_iDuration = iNewValue;
}

void CvUnit::changeDuration(int iChange)
{
	setDuration(getDuration() + iChange);
}

bool CvUnit::isFleeWithdrawl() const
{
	return m_bFleeWithdrawl;
}

void CvUnit::setFleeWithdrawl(bool bNewValue)
{
    m_bFleeWithdrawl = bNewValue;
}

bool CvUnit::isAlive() const
{
	return m_iAlive == 0 ? true : false;
}

void CvUnit::changeAlive(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iAlive += iNewValue;
    }
}

bool CvUnit::isAIControl() const
{
	return m_iAIControl == 0 ? false : true;
}

void CvUnit::changeAIControl(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iAIControl += iNewValue;
    }
}

bool CvUnit::isBoarding() const
{
	return m_iBoarding == 0 ? false : true;
}

void CvUnit::changeBoarding(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iBoarding += iNewValue;
    }
}

int CvUnit::getDefensiveStrikeChance() const
{
	return m_iDefensiveStrikeChance;
}

void CvUnit::changeDefensiveStrikeChance(int iChange)
{
	if (iChange != 0)
	{
		m_iDefensiveStrikeChance += iChange;
	}
}

int CvUnit::getDefensiveStrikeDamage() const
{
	return m_iDefensiveStrikeDamage;
}

void CvUnit::changeDefensiveStrikeDamage(int iChange)
{
	if (iChange != 0)
	{
		m_iDefensiveStrikeDamage += iChange;
	}
}

bool CvUnit::isDoubleFortifyBonus() const
{
	return m_iDoubleFortifyBonus == 0 ? false : true;
}

void CvUnit::changeDoubleFortifyBonus(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iDoubleFortifyBonus += iNewValue;
    }
}

int CvUnit::getFear() const
{
	return m_iFear;
}

void CvUnit::changeFear(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iFear += iNewValue;
    }
}

bool CvUnit::isFlying() const
{
	return m_iFlying == 0 ? false : true;
}

void CvUnit::changeFlying(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iFlying += iNewValue;
    }
}

bool CvUnit::isHeld() const
{
	return m_iHeld == 0 ? false : true;
}

void CvUnit::changeHeld(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iHeld += iNewValue;
    }
}

bool CvUnit::isHiddenNationality() const
{
    if (isCargo())
    {
        if (!getTransportUnit()->isHiddenNationality())
        {
            return false;
        }
    }
	return m_iHiddenNationality == 0 ? false : true;
}

void CvUnit::changeHiddenNationality(int iNewValue)
{
    if (iNewValue != 0)
    {
        if (m_iHiddenNationality + iNewValue == 0)
        {
            updatePlunder(-1, false);
            m_iHiddenNationality += iNewValue;
            joinGroup(NULL, true);
            updatePlunder(1, false);
        }
        else
        {
            m_iHiddenNationality += iNewValue;
        }
    }
}

void CvUnit::changeIgnoreBuildingDefense(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iIgnoreBuildingDefense += iNewValue;
    }
}

bool CvUnit::isImmortal() const
{
	return m_iImmortal == 0 ? false : true;
}

void CvUnit::changeImmortal(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iImmortal += iNewValue;
        if (m_iImmortal < 0)
        {
            m_iImmortal = 0;
        }
    }
}

bool CvUnit::isImmuneToCapture() const
{
	if(getOwnerINLINE()==GC.getWILDMANA_PLAYER())
	{
		return true;
	}
	return m_iImmuneToCapture == 0 ? false : true;
}

void CvUnit::changeImmuneToCapture(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iImmuneToCapture += iNewValue;
    }
}

bool CvUnit::isImmuneToRangeStrike() const
{
	return m_iImmuneToRangeStrike > 0;
}

void CvUnit::changeImmuneToRangeStrike(int iChange)
{
    if (iChange != 0)
    {
        m_iImmuneToCapture += iChange;
    }
}

bool CvUnit::isImmuneToDefensiveStrike() const
{
	return m_iImmuneToDefensiveStrike == 0 ? false : true;
}

void CvUnit::changeImmuneToDefensiveStrike(int iChange)
{
    if (iChange != 0)
    {
        m_iImmuneToDefensiveStrike += iChange;
    }
}

bool CvUnit::isImmuneToFear() const
{
    if(!isAlive())
    {
        return true;
    }
	return m_iImmuneToFear == 0 ? false : true;
}

void CvUnit::changeImmuneToFear(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iImmuneToFear += iNewValue;
    }
}

bool CvUnit::isImmuneToMagic() const
{
	return m_iImmuneToMagic == 0 ? false : true;
}

void CvUnit::changeImmuneToMagic(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iImmuneToMagic += iNewValue;
    }
}

bool CvUnit::isInvisibleFromPromotion() const
{
	return m_iInvisible == 0 ? false : true;
}

void CvUnit::changeInvisibleFromPromotion(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iInvisible += iNewValue;
        if (!isInvisibleFromPromotion() && plot()->isVisibleEnemyUnit(this))
        {
            if (getDomainType() != DOMAIN_IMMOBILE)
            {
                withdrawlToNearestValidPlot();
            }
        }
    }
}

bool CvUnit::isSeeInvisible() const
{
	return m_iSeeInvisible == 0 ? false : true;
}

void CvUnit::changeSeeInvisible(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iSeeInvisible += iNewValue;
    }
}

void CvUnit::changeOnlyDefensive(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iOnlyDefensive += iNewValue;
    }
}

bool CvUnit::isTargetWeakestUnit() const
{
	return m_iTargetWeakestUnit == 0 ? false : true;
}

void CvUnit::changeTargetWeakestUnit(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iTargetWeakestUnit += iNewValue;
    }
}

bool CvUnit::isTargetWeakestUnitCounter() const
{
	return m_iTargetWeakestUnitCounter == 0 ? false : true;
}

void CvUnit::changeTargetWeakestUnitCounter(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iTargetWeakestUnitCounter += iNewValue;
    }
}

bool CvUnit::isTwincast() const
{
	return m_iTwincast == 0 ? false : true;
}

void CvUnit::changeTwincast(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iTwincast += iNewValue;
    }
}

bool CvUnit::isWaterWalking() const
{
	return m_iWaterWalking == 0 ? false : true;
}

void CvUnit::changeWaterWalking(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iWaterWalking += iNewValue;
    }
}

int CvUnit::getBetterDefenderThanPercent() const
{
	return m_iBetterDefenderThanPercent;
}

void CvUnit::changeBetterDefenderThanPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iBetterDefenderThanPercent = (m_iBetterDefenderThanPercent + iChange);
	}
}

int CvUnit::getCombatHealPercent() const
{
	return m_iCombatHealPercent;
}

void CvUnit::changeCombatHealPercent(int iChange)
{
	if (iChange != 0)
	{
		m_iCombatHealPercent = (m_iCombatHealPercent + iChange);
	}
}

int CvUnit::getCombatManaGained() const
{
	return m_iCombatManaGained;
}

void CvUnit::changeCombatManaGained(int iChange)
{
	if (iChange != 0)
	{
		m_iCombatManaGained = (m_iCombatManaGained + iChange);
	}
}

void CvUnit::calcCombatLimit()
{
    int iBestValue = m_pUnitInfo->getCombatLimit();
    int iValue = 0;
   	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (isHasPromotion((PromotionTypes)iI))
		{
		    iValue = GC.getPromotionInfo((PromotionTypes)iI).getCombatLimit();
            if (iValue != 0)
            {
                if (iValue < iBestValue)
                {
                    iBestValue = iValue;
                }
            }
		}
	}
	m_iCombatLimit = iBestValue;
}

int CvUnit::getCombatPercentDefense() const
{
	return m_iCombatPercentDefense;
}

void CvUnit::changeCombatPercentDefense(int iChange)
{
	if (iChange != 0)
	{
		m_iCombatPercentDefense = (m_iCombatPercentDefense + iChange);
		setInfoBarDirty(true);
	}
}

int CvUnit::getCombatPercentInBorders() const
{
	return m_iCombatPercentInBorders;
}

void CvUnit::changeCombatPercentInBorders(int iChange)
{
	if (iChange != 0)
	{
		m_iCombatPercentInBorders = (m_iCombatPercentInBorders + iChange);
		setInfoBarDirty(true);
	}
}

int CvUnit::getCombatPercentGlobalCounter() const
{
	return m_iCombatPercentGlobalCounter;
}

void CvUnit::changeCombatPercentGlobalCounter(int iChange)
{
	if (iChange != 0)
	{
		m_iCombatPercentGlobalCounter = (m_iCombatPercentGlobalCounter + iChange);
		setInfoBarDirty(true);
	}
}

int CvUnit::getFreePromotionPick() const
{
	return m_iFreePromotionPick;
}

void CvUnit::changeFreePromotionPick(int iChange)
{
    if (iChange != 0)
    {
        m_iFreePromotionPick = getFreePromotionPick() + iChange;
    }
}

int CvUnit::getGoldFromCombat() const
{
	return m_iGoldFromCombat;
}

void CvUnit::changeGoldFromCombat(int iChange)
{
    if (iChange != 0)
    {
        m_iGoldFromCombat = getGoldFromCombat() + iChange;
    }
}

void CvUnit::setGroupSize(int iNewValue)
{
    m_iGroupSize = iNewValue;
}

void CvUnit::mutate()
{
    int iMutationChance = GC.getDefineINT("MUTATION_CHANCE");
   	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (GC.getPromotionInfo((PromotionTypes)iI).isMutation())
		{
    		if (GC.getGameINLINE().getSorenRandNum(100, "Mutation") <= iMutationChance)
		    {
                setHasPromotion(((PromotionTypes)iI), true);
		    }
		}
	}
}

int CvUnit::getWithdrawlProbDefensive() const
{
    if (getImmobileTimer() > 0)
    {
        return 0;
    }

	return std::max(0, (m_pUnitInfo->getWithdrawlProbDefensive() + getExtraWithdrawal()));
}

void CvUnit::setInvisibleType(int iNewValue)
{
    m_iInvisibleType = iNewValue;
}

int CvUnit::getRace() const
{
	return m_iRace;
}

void CvUnit::setRace(int iNewValue)
{
    if (m_iRace != NO_PROMOTION)
    {
        setHasPromotion((PromotionTypes)m_iRace, false);
    }
	m_iRace = iNewValue;
}

int CvUnit::getReligion() const
{
	return m_iReligion;
}

void CvUnit::setReligion(int iReligion)
{
	m_iReligion = iReligion;
}

int CvUnit::getResist() const
{
	return m_iResist;
}

void CvUnit::setResist(int iNewValue)
{
	m_iResist = iNewValue;
}

void CvUnit::changeResist(int iChange)
{
	setResist(getResist() + iChange);
}

int CvUnit::getResistModify() const
{
	return m_iResistModify;
}

void CvUnit::setResistModify(int iNewValue)
{
	m_iResistModify = iNewValue;
}

void CvUnit::changeResistModify(int iChange)
{
	setResistModify(getResistModify() + iChange);
}

int CvUnit::getScenarioCounter() const
{
	return m_iScenarioCounter;
}

void CvUnit::setScenarioCounter(int iNewValue)
{
	m_iScenarioCounter = iNewValue;
}

int CvUnit::getSpellCasterXP() const
{
	return m_iSpellCasterXP;
}

void CvUnit::changeSpellCasterXP(int iChange)
{
    if (iChange != 0)
    {
        m_iSpellCasterXP += iChange;
    }
}

int CvUnit::getSpellDamageModify() const
{
	return m_iSpellDamageModify;
}

void CvUnit::changeSpellDamageModify(int iChange)
{
    if (iChange != 0)
    {
        m_iSpellDamageModify += iChange;
    }
}

int CvUnit::getSummoner() const
{
	return m_iSummoner;
}

void CvUnit::setSummoner(int iNewValue)
{
    m_iSummoner = iNewValue;
}

int CvUnit::getWorkRateModify() const
{
	return m_iWorkRateModify;
}

void CvUnit::changeWorkRateModify(int iChange)
{
    if (iChange != 0)
    {
        m_iWorkRateModify += iChange;
    }
}

bool CvUnit::isResisted(CvUnit* pCaster, int iSpell) const
{
    if (GC.getGameINLINE().getSorenRandNum(100, "is Resisted") <= getResistChance(pCaster, iSpell))
    {
        gDLL->getInterfaceIFace()->addMessage(getOwner(), true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_RESISTED"), "", MESSAGE_TYPE_MAJOR_EVENT, "art/interface/buttons/promotions/magicresistance.dds", (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
        gDLL->getInterfaceIFace()->addMessage(pCaster->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), gDLL->getText("TXT_KEY_MESSAGE_SPELL_RESISTED"), "", MESSAGE_TYPE_MAJOR_EVENT, "art/interface/buttons/promotions/magicresistance.dds", (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
        gDLL->getInterfaceIFace()->playGeneralSound("AS3D_RESIST", plot()->getPoint());
        return true;
    }
    return false;
}

int CvUnit::getResistChance(CvUnit* pCaster, int iSpell) const
{
    if (isImmuneToSpell(pCaster, iSpell))
    {
        return 100;
    }
    int iResist = GC.getDefineINT("SPELL_RESIST_CHANCE_BASE");
	iResist += getLevel() * 5;
    iResist += getResist();
    iResist += pCaster->getResistModify();
    iResist += GC.getSpellInfo((SpellTypes)iSpell).getResistModify();
    iResist += GET_PLAYER(getOwnerINLINE()).getResistModify();
    iResist += GET_PLAYER(pCaster->getOwnerINLINE()).getResistEnemyModify();
    if (plot()->isCity())
    {
        iResist += 10;
        iResist += plot()->getPlotCity()->getResistMagic();
    }
	if (iResist >= GC.getDefineINT("SPELL_RESIST_CHANCE_MAX"))
	{
		iResist = GC.getDefineINT("SPELL_RESIST_CHANCE_MAX");
	}
	if (iResist <= GC.getDefineINT("SPELL_RESIST_CHANCE_MIN"))
	{
		iResist = GC.getDefineINT("SPELL_RESIST_CHANCE_MIN");
	}
	return iResist;
}

void CvUnit::changeBaseCombatStr(int iChange)
{
	m_iBaseCombat += iChange;
}

void CvUnit::changeBaseCombatStrDefense(int iChange)
{
	m_iBaseCombatDefense += iChange;
}

void CvUnit::changeBaseCombatFromPromotion(int iChange)
{
	m_iBaseCombatFromPromotion += iChange;
}

void CvUnit::changeBaseCombatDefenseFromPromotion(int iChange)
{
	m_iBaseCombatDefenseFromPromotion += iChange;
}

void CvUnit::changeBaseCombatFromPotion(int iChange)
{
	m_iBaseCombatFromPotion += iChange;
}

void CvUnit::changeBaseCombatDefenseFromPotion(int iChange)
{
	m_iBaseCombatDefenseFromPotion += iChange;
}

int CvUnit::getUnitArtStyleType() const
{
	return m_iUnitArtStyleType;
}

void CvUnit::setUnitArtStyleType(int iNewValue)
{
	m_iUnitArtStyleType = iNewValue;
}

int CvUnit::chooseSpell()
{
    int iBestSpell = -1;
    int iRange;
    int iTempValue;
    int iValue;
    int iBestSpellValue = 0;
    CvPlot* pLoopPlot;
    CvUnit* pLoopUnit;
    CLLNode<IDInfo>* pUnitNode;

    for (int iSpell = 0; iSpell < GC.getNumSpellInfos(); iSpell++)
    {
        iValue = 0;
        if (canCast(iSpell, false))
        {
            iRange = GC.getSpellInfo((SpellTypes)iSpell).getRange();
            if (GC.getSpellInfo((SpellTypes)iSpell).getCreateUnitType() != NO_UNIT)
            {
                int iMoveRange = GC.getUnitInfo((UnitTypes)GC.getSpellInfo((SpellTypes)iSpell).getCreateUnitType()).getMoves() + getExtraSpellMove();
                bool bEnemy = false;
                for (int i = -iMoveRange; i <= iMoveRange; ++i)
                {
                    for (int j = -iMoveRange; j <= iMoveRange; ++j)
                    {
                        pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
                        if (NULL != pLoopPlot)
                        {
                            if (pLoopPlot->isVisibleEnemyUnit(this))
                            {
                                bEnemy = true;
                            }
                        }
                    }
                }
                if (bEnemy)
                {
                    iTempValue = GC.getUnitInfo((UnitTypes)GC.getSpellInfo((SpellTypes)iSpell).getCreateUnitType()).getCombat();
                    for (int iI = 0; iI < GC.getNumDamageTypeInfos(); iI++)
                    {
                        iTempValue += GC.getUnitInfo((UnitTypes)GC.getSpellInfo((SpellTypes)iSpell).getCreateUnitType()).getDamageTypeCombat(iI);
                    }
                    iTempValue *= 100;
                    iTempValue *= GC.getSpellInfo((SpellTypes)iSpell).getCreateUnitNum();
                    iValue += iTempValue;
                }
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getDamage() != 0)
            {
                int iDmg = GC.getSpellInfo((SpellTypes)iSpell).getDamage();
                int iDmgLimit = GC.getSpellInfo((SpellTypes)iSpell).getDamageLimit();
                for (int i = -iRange; i <= iRange; ++i)
                {
                    for (int j = -iRange; j <= iRange; ++j)
                    {
                        pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
                        if (NULL != pLoopPlot)
                        {
                            if (pLoopPlot->getX() != plot()->getX() || pLoopPlot->getY() != plot()->getY())
                            {
                                pUnitNode = pLoopPlot->headUnitNode();
                                while (pUnitNode != NULL)
                                {
                                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                                    pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                                    if (!pLoopUnit->isImmuneToSpell(this, iSpell))
                                    {
                                        if (pLoopUnit->isEnemy(getTeam()))
                                        {
                                            if (pLoopUnit->getDamage() < iDmgLimit)
                                            {
                                                iValue += iDmg * 10;
                                            }
                                        }
                                        if (pLoopUnit->getTeam() == getTeam())
                                        {
                                            iValue -= iDmg * 20;
                                        }
                                        if (pLoopUnit->getTeam() != getTeam() && pLoopUnit->isEnemy(getTeam()) == false)
                                        {
                                            iValue -= 1000;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType1() != NO_PROMOTION)
            {
                iValue += AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType1());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType2() != NO_PROMOTION)
            {
                iValue += AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType2());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType3() != NO_PROMOTION)
            {
                iValue += AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType3());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType4() != NO_PROMOTION)
            {
                iValue += AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType4());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType5() != NO_PROMOTION)
            {
                iValue += AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getAddPromotionType5());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getRemovePromotionType1() != NO_PROMOTION)
            {
                iValue -= AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getRemovePromotionType1());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getRemovePromotionType2() != NO_PROMOTION)
            {
                iValue -= AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getRemovePromotionType2());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getRemovePromotionType3() != NO_PROMOTION)
            {
                iValue -= AI_promotionValue((PromotionTypes)GC.getSpellInfo((SpellTypes)iSpell).getRemovePromotionType3());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getConvertUnitType() != NO_UNIT)
            {
                iValue += GET_PLAYER(getOwnerINLINE()).AI_unitValue((UnitTypes)GC.getSpellInfo((SpellTypes)iSpell).getConvertUnitType(), UNITAI_ATTACK, area());
                iValue -= GET_PLAYER(getOwnerINLINE()).AI_unitValue((UnitTypes)getUnitType(), UNITAI_ATTACK, area());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getCreateBuildingType() != NO_BUILDING)
            {
                iValue += plot()->getPlotCity()->AI_buildingValue((BuildingTypes)GC.getSpellInfo((SpellTypes)iSpell).getCreateBuildingType());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getCreateBuildingType1() != NO_BUILDING)
            {
                iValue += plot()->getPlotCity()->AI_buildingValue((BuildingTypes)GC.getSpellInfo((SpellTypes)iSpell).getCreateBuildingType1());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getCreateBuildingType2() != NO_BUILDING)
            {
                iValue += plot()->getPlotCity()->AI_buildingValue((BuildingTypes)GC.getSpellInfo((SpellTypes)iSpell).getCreateBuildingType2());
            }

            if (GC.getSpellInfo((SpellTypes)iSpell).getCreateFeatureType() != NO_FEATURE)
            {
                iValue += 10;
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getCreateImprovementType() != NO_IMPROVEMENT)
            {
                iValue += 10;
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).isDispel())
            {
                iValue += 25 * (iRange + 1) * (iRange + 1);
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).isPush())
            {
                iValue += 20 * (iRange + 1) * (iRange + 1);
                if (plot()->isCity())
                {
                    iValue *= 3;
                }
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getChangePopulation() != 0)
            {
                iValue += 50 * GC.getSpellInfo((SpellTypes)iSpell).getChangePopulation();
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getCost() != 0)
            {
                iValue -= 4 * GC.getSpellInfo((SpellTypes)iSpell).getCost();
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).getImmobileTurns() != 0)
            {
                iValue += 20 * GC.getSpellInfo((SpellTypes)iSpell).getImmobileTurns() * (iRange + 1) * (iRange + 1);
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).isSacrificeCaster())
            {
                iValue -= getLevel() * GET_PLAYER(getOwnerINLINE()).AI_unitValue((UnitTypes)getUnitType(), UNITAI_ATTACK, area());
            }
            if (GC.getSpellInfo((SpellTypes)iSpell).isResistable())
            {
                iValue /= 2;
            }
            iValue += GC.getSpellInfo((SpellTypes)iSpell).getAIWeight();
            if (iValue > iBestSpellValue)
            {
                iBestSpellValue = iValue;
                iBestSpell = iSpell;
            }
        }
    }
    return iBestSpell;
}

int CvUnit::getExtraSpellMove() const
{
    int iCount = 0;
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
	    if (isHasPromotion((PromotionTypes)iI))
	    {
	        if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionSummonPerk() != NO_PROMOTION)
	        {
	            iCount += GC.getPromotionInfo((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionSummonPerk()).getMovesChange();
	        }
	    }
	}
	return iCount;
}

/*int CvUnit::getMagicResistance() const
{
	if(isImmuneToMagic())
		return 1000;
	return (getUnitInfo().getTier()-1) * 10;
}*/

int CvUnit::doDamage(int iDmg, int iDmgLimit, CvUnit* pAttacker, int iDmgType, bool bStartWar, bool bCountOnly, bool bSpell)
{
    CvWString szMessage;
    int iResist;

	iResist = getUnitSpellResistance();

	int iDamageMod = 100;
	bool bWeakness = false;
	bool bResistance = false;
	bool bImmunity = false;
	
    if (iDmgType != -1)
    {
		int iResis = getDamageTypeResist((DamageTypes)iDmgType);
		if(iResis != 0) {
			if(iResis > 999 || isImmuneToMagic())
				bImmunity = true;
		/*	else if(iResis > 0)
				bResistance = true;
			else
				bWeakness = true;*/
		}
		iDamageMod -= iResis;
    }
    if (pAttacker != NULL && iDmgType != DAMAGE_PHYSICAL)
    {
        iDamageMod += pAttacker->getSpellDamageModify();
    }
	iDamageMod -= iResist;

	if (bImmunity)
		iDmg = 0;
/*	else if (bWeakness)
		iDmg = (iDmg * 300) / 2;
	else if (bResistance)
		iDmg = (iDmg * 100) / 2;
	else*/
	iDmg = iDmg * 100;

    iDmg = iDmg + (iDamageMod * 10) - 1000;

	if (bSpell) {
		iDmg = getSpellDamageGroupAdjustment(iDmg);
	}

	if (iDmg + (getDamage() * 100) > iDmgLimit * 100)
		iDmg = std::max(0, (iDmgLimit - getDamage()) * 100);

	int iDmgMod = iDmg % 100;
	iDmg /= 100;

	if(bCountOnly  || iDmg <= 0)
		return iDmg;

	if (GC.getGameINLINE().getSorenRandNum(100, "Extra damage percent") <= iDmgMod && (iDmg + 1 + (getDamage()) <= iDmgLimit))
		iDmg++;	//Collect the spare scraps of damage that arise from using integers.

    if (iDmg + getDamage() >= GC.getMAX_HIT_POINTS())
    {
        szMessage = gDLL->getText("TXT_KEY_MESSAGE_KILLED_BY", m_pUnitInfo->getDescription(), GC.getDamageTypeInfo((DamageTypes)iDmgType).getDescription());
    }
    else
    {
        szMessage = gDLL->getText("TXT_KEY_MESSAGE_DAMAGED_BY", m_pUnitInfo->getDescription(), iDmg, GC.getDamageTypeInfo((DamageTypes)iDmgType).getDescription());
    }
    gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)getOwner()), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szMessage, "", MESSAGE_TYPE_MAJOR_EVENT, GC.getDamageTypeInfo((DamageTypes)iDmgType).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
    if (pAttacker != NULL)
    {
		if(pAttacker->getTeam() != getTeam())
		{
			gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)pAttacker->getOwner()), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szMessage, "", MESSAGE_TYPE_MAJOR_EVENT, GC.getDamageTypeInfo((DamageTypes)iDmgType).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
		}
        changeDamage(iDmg, pAttacker->getOwner());
        if (getDamage() >= GC.getMAX_HIT_POINTS())
        {
            kill(true,pAttacker->getOwner());
        }
        if (bStartWar)
        {
            if (!(pAttacker->isHiddenNationality()) && !(isHiddenNationality()))
            {
                if (getTeam()==plot()->getTeam())
                {
                    if (!GET_TEAM(getTeam()).isVassal(pAttacker->getTeam()) && !GET_TEAM(pAttacker->getTeam()).isVassal(getTeam()))
                    {
                        if (getTeam() != pAttacker->getTeam())
                        {
                            if (!GET_TEAM(pAttacker->getTeam()).isPermanentWarPeace(getTeam()))
                            {
                                GET_TEAM(pAttacker->getTeam()).declareWar(getTeam(), false, WARPLAN_TOTAL);
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        changeDamage(iDmg, NO_PLAYER);
        if (getDamage() >= GC.getMAX_HIT_POINTS())
        {
            kill(true,NO_PLAYER);
        }
    }

	return 0;
}

/*
*	The damage represented here is the damage to a unit with a group size of 3.  A fireball spell doesn't get more intense when cast at an individual for example.
*	Therefore, the total amount of damage that fireball does is greater if there are more individuals.
*	Spell damage is modified by the number of remaining models from a base of 3.  A 1 model unit will take 1/3 of the damage of a 3 group unit and a 5 unit model will take 66% more damage.
*	A partially damaged unit will absorb the remaining members into existing groups so a spell cast on the unit will deal less damage as the unit gets progressively more injured.  
*   However, for the casting if the unit would drop in models, the damage will not overflow onto other members of the group.
*   What about magic missiles?  For the purposes of this logic, in Erebus magic missiles sometimes miss.  A lightning strike spell isn't just 1, so it too misses sometimes.
*/
int CvUnit::getSpellDamageGroupAdjustment(int iDmg) const {
	int currentGroupMemberCount = (int)(getGroupSize() + (((int)(getDamage() / (100.0 / getGroupSize()) * 100) % 100) > 0 ? 1 : 0) - (getDamage() / (100.0 / getGroupSize())));
	double damagePerMember = iDmg / 3;

	if (getDamage() > 0 && getGroupSize() > 1 && damagePerMember > (((100 / getGroupSize()) - (getDamage() % (100 / getGroupSize()))) * 10))
		return (int)(damagePerMember * (currentGroupMemberCount - 1)) + std::max(100, ((100 / getGroupSize()) - (getDamage() % (100 / getGroupSize()))) * 10);
	else 
		return std::max(100, (int) (damagePerMember * currentGroupMemberCount));
}

int CvUnit::getCityResistance() const {

	if (plot()->getPlotCity() != NULL)
	{
		return (plot()->getPlotCity()->getDefenseModifier(false) / 4);
	}	

	return 0;
}

int CvUnit::getUnitSpellResistance() const {
	
	int iResist = (getUnitInfo().getTier() - 1) * 10;
	iResist += getLevel();
	iResist += getCityResistance();	
	return iResist;
}

//>>>>Refined Defensive Strikes: Modified by Denev 2009/11/07
/*
void CvUnit::doDefensiveStrike(CvUnit* pAttacker)
{
	CvUnit* pBestUnit;
	int iBestDamage = 0;
	CvPlot* pPlot = plot();
	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);
		if (pLoopUnit->getDefensiveStrikeChance() > 0)
		{
			if (atWar(pLoopUnit->getTeam(), pAttacker->getTeam()))
			{
				if (pLoopUnit->isBlitz() || !pLoopUnit->isMadeAttack())
				{
					if (GC.getGameINLINE().getSorenRandNum(100, "Defensive Strike") < pLoopUnit->getDefensiveStrikeChance())
					{
						if (pLoopUnit->getDefensiveStrikeDamage() > iBestDamage)
						{
							iBestDamage = pLoopUnit->getDefensiveStrikeDamage();
							pBestUnit = pLoopUnit;
						}
					}
				}
			}
		}
	}
	if (iBestDamage > 0)
	{
		if (!pBestUnit->isBlitz())
		{
			pBestUnit->setMadeAttack(true);
		}
		int iDmg = 0;
		iDmg += GC.getGameINLINE().getSorenRandNum(pBestUnit->getDefensiveStrikeDamage(), "Defensive Strike 1");
		iDmg += GC.getGameINLINE().getSorenRandNum(pBestUnit->getDefensiveStrikeDamage(), "Defensive Strike 2");
		iDmg -= pAttacker->baseCombatStrDefense() * 2;
		iDmg -= pAttacker->getLevel();
		if (iDmg + pAttacker->getDamage() > 95)
		{
			iDmg = 95 - pAttacker->getDamage();
		}
		if (iDmg > 0)
		{
			pAttacker->changeDamage(iDmg, pBestUnit->getOwner());
			CvWString szMessage = gDLL->getText("TXT_KEY_MESSAGE_DEFENSIVE_STRIKE_BY", GC.getUnitInfo((UnitTypes)pAttacker->getUnitType()).getDescription(), iDmg, GC.getUnitInfo((UnitTypes)pBestUnit->getUnitType()).getDescription());
			gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)pAttacker->getOwner()), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szMessage, "", MESSAGE_TYPE_MAJOR_EVENT, GC.getUnitInfo((UnitTypes)pBestUnit->getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pAttacker->getX(), pAttacker->getY(), true, true);
			szMessage = gDLL->getText("TXT_KEY_MESSAGE_DEFENSIVE_STRIKE", GC.getUnitInfo((UnitTypes)pBestUnit->getUnitType()).getDescription(), GC.getUnitInfo((UnitTypes)pAttacker->getUnitType()).getDescription(), iDmg);
			gDLL->getInterfaceIFace()->addMessage(((PlayerTypes)pBestUnit->getOwner()), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szMessage, "", MESSAGE_TYPE_MAJOR_EVENT, GC.getUnitInfo((UnitTypes)pBestUnit->getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pAttacker->getX(), pAttacker->getY(), true, true);
			pBestUnit->changeExperience(1, 100, true, false, false);
		}
	}
}
*/
void CvUnit::incurDefensiveStrike(const CvUnit* pDefender)
{
	CvUnit* pBestUnit = NULL;
	int iBestAverageDamage = 0;
	int iBestMaxHitDamage = 0;
	int iBestDefensiveStrikesLeft = 0;
	CvPlot* pPlot = getAttackPlot();
	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopStriker = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (!pLoopStriker->isDefensiveStrikes())
		{
			continue;
		}

		if (!isEnemyBetweenUnits(pLoopStriker))
		{
			continue;
		}

		// Defending unit itself can not do defensive strike.
		if (pLoopStriker == pDefender)
		{
			continue;
		}

		if (!pLoopStriker->canDefensiveStrike(this))
		{
			continue;
		}

		// Fix the number of times of defensive strikes per turn, if it is not fixed yet.
		if (pLoopStriker->getCombatDefensiveStrikes() == -1)
		{
			pLoopStriker->setCombatDefensiveStrikes(pLoopStriker->firstStrikes() + GC.getGameINLINE().getSorenRandNum(pLoopStriker->chanceFirstStrikes() + 1, "Defensive Strike"));
		}
		const int iDefensiveStrikesLeft = pLoopStriker->movesLeft() / pLoopStriker->getMovementCostPerDefensiveStrike();

		if (iDefensiveStrikesLeft == 0)
		{
			continue;
		}

		CombatDetails cdStrikerDetails;	//dummy
		CombatDetails cdOurDetails;		//dummy
		const int iStrikerStrength = (pLoopStriker->currCombatStr(NULL, this, &cdStrikerDetails) * GC.getDefineINT("DEFENSIVE_STRIKE_STRENGTH_MODIFIER")) / 100;
		const int iStrikerFirepower = (pLoopStriker->currFirepower(NULL, this) * GC.getDefineINT("DEFENSIVE_STRIKE_STRENGTH_MODIFIER")) / 100;
		int iOurStrength;
		int iStrikerHitDamage;			//dummy
		int iMaxHitDamage;
		int iOurOdds;

		pLoopStriker->getDefenderCombatValues(*this, plot(), iStrikerStrength, iStrikerFirepower, iOurOdds, iOurStrength, iStrikerHitDamage, iMaxHitDamage, &cdOurDetails);

		// If average damage is not enough, skip this striker.
		const int iAverageDamage = (iMaxHitDamage * iDefensiveStrikesLeft * 2520) / (iDefensiveStrikesLeft + 1);	// '2520' is lowest common multiple of 1-10
		if (iAverageDamage <= iBestAverageDamage)
		{
			continue;
		}

		// hit roll
		const int iHitRate = (100 + pLoopStriker->getDefensiveStrikeChance()) * GC.getDefineINT("COMBAT_DIE_SIDES") / 100 - iOurOdds;
		for (int iLoop = 0; iLoop < iDefensiveStrikesLeft; iLoop++)
		{
			if (GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("COMBAT_DIE_SIDES"), "Defensive Strike Hit Roll") < iHitRate)
			{
				iBestAverageDamage = iAverageDamage;
				iBestMaxHitDamage = iMaxHitDamage + pLoopStriker->getDefensiveStrikeDamage();
				iBestDefensiveStrikesLeft = iDefensiveStrikesLeft;
				pBestUnit = pLoopStriker;
				break;
			}
		}
	}
	if (pBestUnit != NULL)
	{
		// damage roll
		int iDamage = 0;
		for (int iLoop = 0; iLoop < iBestDefensiveStrikesLeft; iLoop++)
		{
			int iDamageTemp = GC.getGameINLINE().getSorenRandNum(iBestMaxHitDamage, "Defensive Strike Damage Roll") + 1;
			if (iDamageTemp > iDamage)
			{
				iDamage = iDamageTemp;
			}
		}

		int iActualDamage = std::max(1, iDamage);
		if (iActualDamage + getDamage() > maxHitPoints() - 1)
		{
			iActualDamage = maxHitPoints() - getDamage() - 1;
		}

		if (iActualDamage > 0)
		{
			changeDamage(iActualDamage, pBestUnit->getOwnerINLINE());
			pBestUnit->setMadeAttack(true);
			pBestUnit->changeMoves(pBestUnit->getMovementCostPerDefensiveStrike());
			pBestUnit->changeExperience(1, GC.getDefineINT("FREE_XP_MAX"), true, false, false);

			CvWString szMessage;
			szMessage = gDLL->getText("TXT_KEY_MESSAGE_DEFENSIVE_STRIKE_BY", getNameKey(), iActualDamage, pBestUnit->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "", MESSAGE_TYPE_INFO, pBestUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);

			szMessage = gDLL->getText("TXT_KEY_MESSAGE_DEFENSIVE_STRIKE", pBestUnit->getNameKey(), getNameKey(), iActualDamage);
			gDLL->getInterfaceIFace()->addMessage(pBestUnit->getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szMessage, "", MESSAGE_TYPE_INFO, pBestUnit->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
		}
	}
}

bool CvUnit::canDefensiveStrike(const CvUnit* pTarget) const
{
	if (!canAttack(*pTarget))
	{
		return false;
	}

	// Blitz allows multiple strikes or moving strike
	if (!isBlitz())
	{
		if (isMadeAttack() || hasMoved())
		{
			return false;
		}
	}

	return true;
}

int CvUnit::getMovementCostPerDefensiveStrike() const
{
	FAssertMsg(getCombatDefensiveStrikes() != -1, "getMovementCostPerDefensiveStrike() needs fixed combat defensive strikes.");

	int iMovementCostPerStrike;

	if (getCombatDefensiveStrikes() == 0)
	{
		return MAX_INT;
	}

	// Round down
	iMovementCostPerStrike = maxMoves() / getCombatDefensiveStrikes();
//	iMovementCostPerStrike += maxMoves() % getCombatDefensiveStrikes() > 0 ? 1 : 0;

	return std::max(1, iMovementCostPerStrike);
}
//<<<<Refined Defensive Strikes: End Modify

int CvUnit::getSpellDefenderValue(CvUnit* pLoopUnit, CvPlot * pTargetplot, int iDmgType) const {
	int iValue = 0;

	if (pLoopUnit == pTargetplot->getBestDefender(getOwnerINLINE()))
	{
		iValue = MAX_INT;
	}
	else if (pLoopUnit->getTeam() == getTeam())
	{
		iValue = 1;
	}
	else if (!GET_TEAM(getTeam()).isAtWar(pLoopUnit->getTeam()))
	{
		iValue = 2;
	}
	else
	{
		iValue = (pLoopUnit->baseCombatStr() + pLoopUnit->baseCombatStrDefense()) * 100 / (10 + pLoopUnit->getDamage());
		if (iDmgType != -1)
		{
			iValue *= 100;
			iValue /= std::max(1, 100 + pLoopUnit->getDamageTypeResist((DamageTypes)iDmgType));
		}
		iValue = std::max(iValue, 3);
	}

	return iValue;
}

bool CvUnit::isImmortalRebirthDestinationExisting()
{
	return getImmortalRebirthDestination() != NULL;
}
CvCity* CvUnit::getImmortalRebirthDestination()
{
	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
    if (kPlayer.getCapitalCity() != NULL)
    {
		int iSpawnX = kPlayer.getCapitalCity()->getX();
		int iSpawnY = kPlayer.getCapitalCity()->getY();
		//SpyFanatic: if the unit die due to capital city being conquered...
		if(at(iSpawnX, iSpawnY))
		{
			CvPlot* pPlot;
			pPlot = plot();
			if(pPlot != NULL)
			{
				CvUnit* pLoopUnit;
				CLLNode<IDInfo>* pUnitNode;
				CvCity* pBestCity = NULL;
				bool bCapitalWillBeRazed = true;

				pUnitNode = pPlot->headUnitNode();
				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					if(pLoopUnit->getTeam() != getTeam() && atWar(pLoopUnit->getTeam(),getTeam()))
					{
						bCapitalWillBeRazed = true;
						//Not sure if actually the enemy unit has already 'moved' inside the city
						break;
					}
					else if(pLoopUnit->getTeam() == getTeam() && pLoopUnit->getID() != getID() && pLoopUnit->canDefend())
					{
						bCapitalWillBeRazed = false;
						//If there is at least one other unit who can defend, then you can do rebirth here
					}
					pUnitNode = pPlot->nextUnitNode(pUnitNode);
				}
				if(bCapitalWillBeRazed)
				{
					//capital has been conquered... immmortal rebirth cannot happen in this plot! Normally new capital will be find using CvPlayer::findNewCapital
					pBestCity = kPlayer.getNewCityCapital();
					if(pBestCity != NULL)
					{
						//rebirth in what will be the new capital
						return pBestCity;
					}
					else
					{
						//there will be no new capital as current one will get razed
						return NULL;
					}
				}
			}
		}
		else
		{
			//rebirth in capital
			return kPlayer.getCapitalCity();
		}
	}
	//No city, no rebirth
	return NULL;
}
void CvUnit::doEscape()
{
	CvCity* pCapitalCity = getImmortalRebirthDestination();
	if(pCapitalCity != NULL)
	{
		/*if(isOOSLogging())
		{
			oosLog("AIsetXY"
				,"Turn:%d,PlayerID:%d,Unit:%d,Name:%S,GroupID:%d,AIGroupID:%d,AIGroupType:%S,fromX:%d,fromY:%d,toX:%d,toY:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwnerINLINE()
				,getID()
				,getName().GetCString()
				,getGroupID()
				,getAIGroup()!=NULL ? getAIGroup()->getID() : -1
				,getAIGroup()!=NULL ? GC.getAIGroupInfo(getAIGroup()->getGroupType()).getDescription() : L""
				,getX_INLINE()
				,getY_INLINE()
				,pCapitalCity->getX()
				,pCapitalCity->getY()
			);
		}*/
		setXY(pCapitalCity->getX(), pCapitalCity->getY(), false, true, true);
	}
}

void CvUnit::doImmortalRebirth()
{
    joinGroup(NULL);
    setDamage(75, NO_PLAYER);
    bool bFromProm = false;
   	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
        if (isHasPromotion((PromotionTypes)iI))
        {
            if (GC.getPromotionInfo((PromotionTypes)iI).isImmortal())
            {
                setHasPromotion(((PromotionTypes)iI), false);
                bFromProm = true;
                break;
            }
		}
	}
	if (!bFromProm)
	{
        changeImmortal(-1);
	}
    doEscape();

	//SpyFanatic: manage combat auras
	/*if(isOOSLogging())
	{
		oosLog(
			"CombatAura"
			,"Turn:%d,Player:%d,UnitID:%d,getBlessing1:%d,getBlessing2:%d,getCurse1:%d,getCurse2:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getOwnerINLINE()
			,getID()
			,(getBlessing1()!=NULL)
			,(getBlessing2()!=NULL)
			,(getCurse1()!=NULL)
			,(getCurse2()!=NULL)
		);
	}
	if(getBlessing1()!=NULL)
		getBlessing1()->Remove(this);
	if(getBlessing2()!=NULL)
		getBlessing2()->Remove(this);
	if(getCurse1()!=NULL)
		getCurse1()->Remove(this);
	if(getCurse2()!=NULL)
		getCurse2()->Remove(this);
	setCombatAuraType(NO_COMBATAURA);
*/
/*************************************************************************************************/
/**	BETTER AI (Immortal Units heal) Sephi                                      					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    if (!isHuman())
    {
        getGroup()->pushMission(MISSION_HEAL);
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

}

void CvUnit::combatWon(CvUnit* pLoser, bool bAttacking)
{
    PromotionTypes ePromotion;
    bool bConvert = false;
    int iUnit = NO_UNIT;
    CLLNode<IDInfo>* pUnitNode;
    CvUnit* pLoopUnit;
    CvPlot* pPlot;
    CvUnit* pUnit;
	int iPromotionEnslavementChance = 0;

    for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
    {
        if (isHasPromotion((PromotionTypes)iI))
        {
            if (GC.getPromotionInfo((PromotionTypes)iI).getFreeXPFromCombat() != 0)
            {
                changeExperience(GC.getPromotionInfo((PromotionTypes)iI).getFreeXPFromCombat(), -1, false, false, false);
            }
            if (GC.getPromotionInfo((PromotionTypes)iI).getModifyGlobalCounterOnCombat() != 0)
            {
                if (pLoser->isAlive())
                {
                    GC.getGameINLINE().changeGlobalCounter(GC.getPromotionInfo((PromotionTypes)iI).getModifyGlobalCounterOnCombat());
                }
		    }
            if (GC.getPromotionInfo((PromotionTypes)iI).isRemovedByCombat())
            {
                setHasPromotion(((PromotionTypes)iI), false);
		    }
			if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionFromCombat()!=NO_PROMOTION)
			{
                if (GC.getGameINLINE().getSorenRandNum(100, "Combat Apply") <= GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatApplyChance())
                {
					setHasPromotion((PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionFromCombat(),true);
				}
			}

            if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatApply() != NO_PROMOTION)
            {
                ePromotion = (PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatApply();
                pPlot = pLoser->plot();
                pUnitNode = pPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pPlot->nextUnitNode(pUnitNode);
                    if (pLoopUnit->isHasPromotion(ePromotion) == false)
                    {
                        if (pLoopUnit->isAlive() || !GC.getPromotionInfo(ePromotion).isPrereqAlive())
                        {
                            if (isEnemy(pLoopUnit->getTeam()))
                            {
								if (GC.getPromotionInfo(ePromotion).isAnyUnitCombat() || 
									(pLoopUnit->getUnitCombatType()!=NO_UNITCOMBAT && GC.getPromotionInfo(ePromotion).getUnitCombat(pLoopUnit->getUnitCombatType())))
                                {
                                    if (GC.getGameINLINE().getSorenRandNum(100, "Combat Apply") <= GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatApplyChance())
                                    {
                                        pLoopUnit->setHasPromotion(ePromotion, true);
                                        gDLL->getInterfaceIFace()->addMessage((PlayerTypes)pLoopUnit->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), GC.getPromotionInfo(ePromotion).getDescription(), "", MESSAGE_TYPE_INFO, GC.getPromotionInfo(ePromotion).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
                                        gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), true, GC.getEVENT_MESSAGE_TIME(), GC.getPromotionInfo(ePromotion).getDescription(), "", MESSAGE_TYPE_INFO, GC.getPromotionInfo(ePromotion).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (GC.getPromotionInfo((PromotionTypes)iI).getCombatCapturePercent() != 0)
            {
                if (iUnit == NO_UNIT && pLoser->isAlive())
                {
                    if (GC.getGameINLINE().getSorenRandNum(100, "Combat Capture") <= GC.getPromotionInfo((PromotionTypes)iI).getCombatCapturePercent())
                    {
                        iUnit = pLoser->getUnitType();
                        bConvert = true;
                    }
                }
            }
            if (GC.getPromotionInfo((PromotionTypes)iI).getCaptureUnitCombat() != NO_UNITCOMBAT)
            {
                if (iUnit == NO_UNIT && pLoser->getUnitCombatType() == GC.getPromotionInfo((PromotionTypes)iI).getCaptureUnitCombat())
                {
                    iUnit = pLoser->getUnitType();
                    bConvert = true;
                }
            }
			if (GC.getPromotionInfo((PromotionTypes)iI).getEnslavementChance() > 0) {
				iPromotionEnslavementChance += GC.getPromotionInfo((PromotionTypes)iI).getEnslavementChance();
			}
		}
        if (pLoser->isHasPromotion((PromotionTypes)iI))
        {
            if (GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatApply() != NO_PROMOTION)
            {
                ePromotion = (PromotionTypes)GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatApply();
                if (isHasPromotion(ePromotion) == false)
                {
                    if (isAlive() || !GC.getPromotionInfo(ePromotion).isPrereqAlive())
                    {
                        if (pLoser->isEnemy(getTeam()))
                        {
                            if (GC.getGameINLINE().getSorenRandNum(100, "Combat Apply") <= GC.getPromotionInfo((PromotionTypes)iI).getPromotionCombatApplyChance())
                            {
                                setHasPromotion(ePromotion, true);
                                gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), true, GC.getEVENT_MESSAGE_TIME(), GC.getPromotionInfo(ePromotion).getDescription(), "", MESSAGE_TYPE_INFO, GC.getPromotionInfo(ePromotion).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
                                gDLL->getInterfaceIFace()->addMessage((PlayerTypes)pLoser->getOwner(), true, GC.getEVENT_MESSAGE_TIME(), GC.getPromotionInfo(ePromotion).getDescription(), "", MESSAGE_TYPE_INFO, GC.getPromotionInfo(ePromotion).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
                            }
                        }
                    }
                }
		    }
        }
	}

	if(pLoser->getUnitCombatType() == GC.getInfoTypeForString("UNITCOMBAT_ANIMAL"))
	{
        if (GC.getGameINLINE().getSorenRandNum(100, "Combat Capture") < getCaptureAnimalChanceTotal())
        {
            iUnit = pLoser->getUnitType();
            bConvert = true;
        }

		else if(pLoser->isBarbarian()) {
			doAnimalTurnedToFood(pLoser->getUnitType());
		}
	}

	if(pLoser->getUnitCombatType()==GC.getInfoTypeForString("UNITCOMBAT_BEAST"))
	{
        if (GC.getGameINLINE().getSorenRandNum(100, "Combat Capture") < getCaptureBeastChanceTotal())
        {
            iUnit = pLoser->getUnitType();
            bConvert = true;
        }
	}

    if (GET_PLAYER(getOwnerINLINE()).getFreeXPFromCombat() != 0)
    {
        changeExperience(GET_PLAYER(getOwnerINLINE()).getFreeXPFromCombat(), -1, false, false, false);
    }
    if (getCombatHealPercent() != 0 && bAttacking)
    {
        if (pLoser->isAlive())
        {
            int i = getCombatHealPercent();
            if (i > getDamage())
            {
                i = getDamage();
            }
            if (i != 0)
            {
                changeDamage(-1 * i, NO_PLAYER);
            }
        }
    }

    if (getCombatManaGained()!=0)
    {
		GET_PLAYER(getOwnerINLINE()).changeMana(getCombatManaGained());
    }

    if (m_pUnitInfo->isExplodeInCombat() && m_pUnitInfo->isSuicide())
    {
        if (bAttacking)
        {
            pPlot = pLoser->plot();
        }
        else
        {
            pPlot = plot();
        }
        if (plot()->isVisibleToWatchingHuman())
        {
            gDLL->getEngineIFace()->TriggerEffect((EffectTypes)GC.getInfoTypeForString("EFFECT_ARTILLERY_SHELL_EXPLODE"), pPlot->getPoint(), (float)(GC.getASyncRand().get(360)));
            gDLL->getInterfaceIFace()->playGeneralSound("AS3D_UN_GRENADE_EXPLODE", pPlot->getPoint());
        }
    }
    if (GC.getUnitInfo(pLoser->getUnitType()).isExplodeInCombat())
    {
        if (plot()->isVisibleToWatchingHuman())
        {
            gDLL->getEngineIFace()->TriggerEffect((EffectTypes)GC.getInfoTypeForString("EFFECT_ARTILLERY_SHELL_EXPLODE"), plot()->getPoint(), (float)(GC.getASyncRand().get(360)));
            gDLL->getInterfaceIFace()->playGeneralSound("AS3D_UN_GRENADE_EXPLODE", plot()->getPoint());
        }
    }
    if ((iPromotionEnslavementChance + m_pUnitInfo->getEnslavementChance() + GET_PLAYER(getOwnerINLINE()).getEnslavementChance()) > 0)
    {
        if (getDuration() == 0 && pLoser->isAlive() && !pLoser->isAnimal() && iUnit == NO_UNIT)
        {
            if (GC.getGameINLINE().getSorenRandNum(100, "Enslavement") <= (iPromotionEnslavementChance + m_pUnitInfo->getEnslavementChance() + GET_PLAYER(getOwnerINLINE()).getEnslavementChance()))
            {
                iUnit = GC.getDefineINT("SLAVE_UNIT");
            }
        }
    }
    if (m_pUnitInfo->getPromotionFromCombat() != NO_PROMOTION)
    {
        if (pLoser->isAlive())
        {
            setHasPromotion((PromotionTypes)m_pUnitInfo->getPromotionFromCombat(), true);
        }
    }
    if (getGoldFromCombat() != 0)
    {
        if (!pLoser->isAnimal())
        {
            GET_PLAYER(getOwnerINLINE()).changeGold(getGoldFromCombat());
            CvWString szBuffer = gDLL->getText("TXT_KEY_MESSAGE_GOLD_FROM_COMBAT", getGoldFromCombat()).GetCString();
            gDLL->getInterfaceIFace()->addMessage((PlayerTypes)getOwner(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_GOODY_GOLD", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
        }
    }
    if (getDuration() > 0)
    {
        changeDuration(m_pUnitInfo->getDurationFromCombat());
    }
    if (pLoser->getDamageTypeCombat(DAMAGE_POISON) > 0 && GC.getDefineINT("POISONED_PROMOTION") != -1)
    {
        if (isAlive() && getDamage() > 0)
        {
            if (GC.getGameINLINE().getSorenRandNum(100, "Poisoned") >= getDamageTypeResist(DAMAGE_POISON))
            {
                setHasPromotion((PromotionTypes)GC.getDefineINT("POISONED_PROMOTION"), true);
            }
        }
    }
	if(GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits() && m_pUnitInfo->getUnitCreateFromCombat() != NO_UNIT && 
		((GC.getUnitInfo((UnitTypes) m_pUnitInfo->getUnitCreateFromCombat()).isCitySupportNeeded() && GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits()) 
			|| !GC.getUnitInfo((UnitTypes) m_pUnitInfo->getUnitCreateFromCombat()).isCitySupportNeeded()))
	{
		if (!pLoser->isImmuneToCapture() && pLoser->isAlive() && GC.getUnitInfo((UnitTypes)pLoser->getUnitType()).getEquipmentPromotion() == NO_PROMOTION)
		{
			if (GC.getGameINLINE().getSorenRandNum(100, "Create Unit from Combat") <= m_pUnitInfo->getUnitCreateFromCombatChance())
			{
				pUnit = GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)m_pUnitInfo->getUnitCreateFromCombat(), plot()->getX_INLINE(), plot()->getY_INLINE());
				pUnit->setDuration(getDuration());
				if (isHiddenNationality())
				{
					pUnit->setHasPromotion((PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION"), true);
				}
				iUnit = NO_UNIT;
			}
		}
	}

	for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
	{
		if(isHasPromotion((PromotionTypes)iI) && GC.getPromotionInfo((PromotionTypes)iI).getUnitClassCreateFromCombat()!=NO_UNITCLASS)
		{
			UnitTypes eUnit=(UnitTypes)GC.getCivilizationInfo(getCivilizationType()).getCivilizationUnits(GC.getPromotionInfo((PromotionTypes)iI).getUnitClassCreateFromCombat());
			if(eUnit!=NO_UNIT && ((GC.getUnitInfo(eUnit).isCitySupportNeeded() && GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits()) || !GC.getUnitInfo(eUnit).isCitySupportNeeded()))
			{
				if (GC.getGameINLINE().getSorenRandNum(100, "Create Unit from Combat") <= GC.getPromotionInfo((PromotionTypes)iI).getUnitClassCreateFromCombatChance())
				{
					pUnit = GET_PLAYER(getOwnerINLINE()).initUnit(eUnit, plot()->getX_INLINE(), plot()->getY_INLINE());
					pUnit->setDuration(getDuration());
					if (isHiddenNationality())
					{
						pUnit->setHasPromotion((PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION"), true);
					}
					iUnit = NO_UNIT;
				}
			}
		}
	}

    if (iUnit != NO_UNIT)
    {
        if ((!pLoser->isImmuneToCapture() && !isNoCapture() && !pLoser->isImmortal())
          || GC.getUnitInfo((UnitTypes)pLoser->getUnitType()).getEquipmentPromotion() != NO_PROMOTION)
        {
			if(GC.getUnitInfo((UnitTypes)pLoser->getUnitType()).getCaptureDifficulty()<GC.getGameINLINE().getSorenRandNum(100,"Capture Difficulty"))
			{
				pUnit = GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)iUnit, plot()->getX_INLINE(), plot()->getY_INLINE());
				if (getDuration() != 0)
				{
					pUnit->setDuration(getDuration());
				}
				if (iUnit == GC.getDefineINT("SLAVE_UNIT"))
				{
					if (pLoser->getRace() != NO_PROMOTION)
					{
						pUnit->setHasPromotion((PromotionTypes)pLoser->getRace(), true);
					}
				}
				if (bConvert)
				{
					pLoser->setDamage(75, NO_PLAYER, false);
					pUnit->convert(pLoser);
				}

	/*************************************************************************************************/
	/**	ADDON (Adventures) Sephi                                                      				**/
	/** tag bLanunCaptureShip                                                                             **/
	/*************************************************************************************************/
				if (bConvert && GC.getUnitInfo((UnitTypes)iUnit).getUnitCombatType()==GC.getInfoTypeForString("UNITCOMBAT_NAVAL"))
				{
					for(int iAdvLoop=0;iAdvLoop<GC.getNumAdventureInfos();iAdvLoop++)
					{
						if(GET_PLAYER(getOwnerINLINE()).isAdventureEnabled(iAdvLoop) && !GET_PLAYER(getOwnerINLINE()).isAdventureFinished(iAdvLoop))
						{
							CvAdventureInfo &kAdv = GC.getAdventureInfo((AdventureTypes)iAdvLoop);
							for(int iJ=0;iJ<GC.getMAX_ADVENTURESTEPS();iJ++)
							{
								if(kAdv.getAdventureStep(iJ)!=NO_ADVENTURESTEP)
								{
									if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isLanunCaptureShip())
									{
										GET_PLAYER(getOwnerINLINE()).changeAdventureCounter(iAdvLoop,iJ,1);
									}
								}
							}
						}
					}
				}
	/*************************************************************************************************/
	/**	END                     	                                        						**/
	/*************************************************************************************************/
			}
        }
    }
	if (!CvString(GC.getUnitInfo(getUnitType()).getPyPostCombatWon()).empty())
    {
        CyUnit* pyCaster = new CyUnit(this);
        CyUnit* pyOpponent = new CyUnit(pLoser);
        CyArgsList argsList;
        argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCaster));	// pass in unit class
        argsList.add(gDLL->getPythonIFace()->makePythonObject(pyOpponent));	// pass in unit class
        gDLL->getPythonIFace()->callFunction(PYSpellModule, "postCombatWon", argsList.makeFunctionArgs()); //, &lResult
        delete pyCaster; // python fxn must not hold on to this pointer
        delete pyOpponent; // python fxn must not hold on to this pointer
    }
	if (!CvString(GC.getUnitInfo(pLoser->getUnitType()).getPyPostCombatLost()).empty())
    {
        CyUnit* pyCaster = new CyUnit(pLoser);
        CyUnit* pyOpponent = new CyUnit(this);
        CyArgsList argsList;
        argsList.add(gDLL->getPythonIFace()->makePythonObject(pyCaster));	// pass in unit class
        argsList.add(gDLL->getPythonIFace()->makePythonObject(pyOpponent));	// pass in unit class
        gDLL->getPythonIFace()->callFunction(PYSpellModule, "postCombatLost", argsList.makeFunctionArgs()); //, &lResult
        delete pyCaster; // python fxn must not hold on to this pointer
        delete pyOpponent; // python fxn must not hold on to this pointer
    }
    if (m_pUnitInfo->getUnitConvertFromCombat() != NO_UNIT)
    {
        if (GC.getGameINLINE().getSorenRandNum(100, "Convert Unit from Combat") <= m_pUnitInfo->getUnitConvertFromCombatChance())
        {
            pUnit = GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)m_pUnitInfo->getUnitConvertFromCombat(), getX_INLINE(), getY_INLINE(), AI_getUnitAIType());
            pUnit->convert(this);
        }
    }
}

void CvUnit::changeBonusAffinity(BonusTypes eIndex, int iChange)
{
    if (iChange != 0)
    {
        m_paiBonusAffinity[eIndex] += iChange;
    }
    updateBonusAffinity(eIndex);
}

int CvUnit::getBonusAffinity(BonusTypes eIndex) const
{
	return m_paiBonusAffinity[eIndex];
}

void CvUnit::updateBonusAffinity(BonusTypes eIndex)
{
    int iNew = GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses(eIndex) * getBonusAffinity(eIndex);
    int iOld = m_paiBonusAffinityAmount[eIndex];

	//iNew = std::min(iNew, 2 * GC.getUnitInfo(getUnitType()).getTier()); //added Sephi

    if (GC.getBonusInfo(eIndex).getDamageType() != NO_DAMAGE) {
        m_paiDamageTypeCombat[GC.getBonusInfo(eIndex).getDamageType()] += iNew - iOld;
    }

    m_iTotalDamageTypeCombat += iNew - iOld;

	m_paiBonusAffinityAmount[eIndex] = iNew;
}

void CvUnit::changeDamageTypeCombat(DamageTypes eIndex, int iChange)
{
    if (iChange != 0)
    {
        m_paiDamageTypeCombat[eIndex] = (m_paiDamageTypeCombat[eIndex] + iChange);
        m_iTotalDamageTypeCombat = (m_iTotalDamageTypeCombat + iChange);
    }
}

int CvUnit::getDamageTypeCombat(DamageTypes eIndex) const
{
	return m_paiDamageTypeCombat[eIndex];
}

int CvUnit::getTotalDamageTypeCombat() const
{
    return m_iTotalDamageTypeCombat;
}

int CvUnit::getDamageTypeResist(DamageTypes eIndex) const
{
    int i = m_paiDamageTypeResist[eIndex];
    if (i <= -100)
    {
        return -100;
    }
    if (i >= 100)
    {
        return 100;
    }
	return i;
}

void CvUnit::changeDamageTypeResist(DamageTypes eIndex, int iChange)
{
    if (iChange != 0)
    {
        m_paiDamageTypeResist[eIndex] = (m_paiDamageTypeResist[eIndex] + iChange);
    }
}

bool CvUnit::isEquipmentDurabilityValid(DurabilityTypes eIndex) const
{
	return m_paiEquipmentDurabilityValidityCounter[eIndex]>0;
}

int CvUnit::getEquipmentDurabilityValidityCounter(DurabilityTypes eIndex) const
{
    return m_paiEquipmentDurabilityValidityCounter[eIndex];
}

void CvUnit::changeEquipmentDurabilityValidityCounter(DurabilityTypes eIndex, int iChange)
{
    if (iChange != 0)
    {
        m_paiEquipmentDurabilityValidityCounter[eIndex] += iChange;
    }
}

int CvUnit::countUnitsWithinRange(int iRange, bool bEnemy, bool bNeutral, bool bTeam)
{
    CLLNode<IDInfo>* pUnitNode;
    CvUnit* pLoopUnit;
    CvPlot* pLoopPlot;
    int iCount = 0;
    for (int i = -iRange; i <= iRange; ++i)
    {
        for (int j = -iRange; j <= iRange; ++j)
        {
            pLoopPlot = ::plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), i, j);
            if (NULL != pLoopPlot)
            {
                pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                    if (bTeam && pLoopUnit->getTeam() == getTeam())
                    {
                        iCount += 1;
                    }
                    if (bEnemy && atWar(pLoopUnit->getTeam(), getTeam()))
                    {
                        iCount += 1;
                    }
                    if (bNeutral && pLoopUnit->getTeam() != getTeam() && !atWar(pLoopUnit->getTeam(), getTeam()))
                    {
                        iCount += 1;
                    }
                }
            }
        }
    }
    return iCount;
}

CvPlot* CvUnit::getOpenPlot() const
{
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot = NULL;
	int iValue;
	int iBestValue = MAX_INT;
	bool bEquipment = false;
	if (m_pUnitInfo->getEquipmentPromotion() != NO_PROMOTION)
	{
	    bEquipment = true;
	}
	for (int iI = 0; iI < GC.getMapINLINE().numPlotsINLINE(); iI++)
	{
		pLoopPlot = GC.getMapINLINE().plotByIndexINLINE(iI);
		if (pLoopPlot->isValidDomainForLocation(*this))
		{
			if (canMoveInto(pLoopPlot) || bEquipment)
			{
				if (pLoopPlot->getNumUnits() == 0)
				{
                    iValue = plotDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()) * 2;
                    if (pLoopPlot->area() != area())
                    {
                        iValue *= 3;
                    }
                    if (iValue < iBestValue)
                    {
                        iBestValue = iValue;
                        pBestPlot = pLoopPlot;
					}
				}
			}
		}
	}
	return pBestPlot;
}

void CvUnit::betray(PlayerTypes ePlayer)
{
   	if (getOwnerINLINE() == ePlayer)
	{
		return;
	}
    CvPlot* pNewPlot = getOpenPlot();
    if (pNewPlot != NULL)
    {
        CvUnit* pUnit = GET_PLAYER(ePlayer).initUnit((UnitTypes)getUnitType(), pNewPlot->getX(), pNewPlot->getY(), AI_getUnitAIType());
        pUnit->convert(this);
        if (pUnit->getDuration() > 0)
        {
            pUnit->setDuration(0);
        }
    }
	else
	{
		kill(false);
	}
}
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (New Functions) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
int CvUnit::getAutoSpellCast() const
{
    return m_iAutoSpellCast;
}

void CvUnit::setAutoSpellCast(int newvalue)
{
    m_iAutoSpellCast=newvalue;
    if (newvalue==NO_SPELL)
        setHasPromotion((PromotionTypes)GC.getDefineINT("PROMOTION_AUTOCASTING"),false);
    else
    {
        setHasPromotion((PromotionTypes)GC.getDefineINT("PROMOTION_AUTOCASTING"),true);
        if (canCast(newvalue,false))
        {
            cast(newvalue);
        }
    }
}

int CvUnit::getExtraLives() const
{
	//TEMP DISABLE  -- tag doesn't work, unit is just killed
	return 0;
	//TEMP DISABLE
	return m_iExtraLives;
}

int CvUnit::getBonusNearUF() const
{
	if(m_pUnitInfo->getBonusNearUF() == 0)
		return 0;

	if(plot()->isNearUF(3))
		return m_pUnitInfo->getBonusNearUF();

	return 0;
}

void CvUnit::changeExtraLives(int iChange)
{
	m_iExtraLives+=iChange;
}

int CvUnit::getStrengthPotionsUsed() const
{
	return m_iStrengthPotionsUsed;
}

void CvUnit::changeStrengthPotionsUsed(int iChange)
{
	m_iStrengthPotionsUsed += iChange;
}

int CvUnit::getMissionSpell() const
{
    return m_iMissionSpell;
}

void CvUnit::setMissionSpell(int newvalue)
{
    m_iMissionSpell=newvalue;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

void CvUnit::read(FDataStreamBase* pStream)
{
	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iID);
	pStream->Read(&m_iGroupID);
	pStream->Read(&m_iHotKeyNumber);
	pStream->Read(&m_iX);
	pStream->Read(&m_iY);
	pStream->Read(&m_iLastMoveTurn);
	pStream->Read(&m_iReconX);
	pStream->Read(&m_iReconY);
	pStream->Read(&m_iGameTurnCreated);
	pStream->Read(&m_iDamage);
	pStream->Read(&m_iMoves);
	pStream->Read(&m_iExperience);
	pStream->Read(&m_iLevel);
	pStream->Read(&m_iCargo);
	pStream->Read(&m_iCargoCapacity);
	pStream->Read(&m_iAttackPlotX);
	pStream->Read(&m_iAttackPlotY);
	pStream->Read(&m_iCombatTimer);
	pStream->Read(&m_iCombatFirstStrikes);
	if (uiFlag < 2)
	{
		int iCombatDamage;
		pStream->Read(&iCombatDamage);
	}
	pStream->Read(&m_iFortifyTurns);
	pStream->Read(&m_iBlitzCount);
	pStream->Read(&m_iAmphibCount);
	pStream->Read(&m_iRiverCount);
	pStream->Read(&m_iEnemyRouteCount);
	pStream->Read(&m_iAlwaysHealCount);
	pStream->Read(&m_iHillsDoubleMoveCount);
	pStream->Read(&m_iImmuneToFirstStrikesCount);
	pStream->Read(&m_iExtraVisibilityRange);
	pStream->Read(&m_iExtraMoves);
	pStream->Read(&m_iExtraMoveDiscount);
	pStream->Read(&m_iExtraAirRange);
	pStream->Read(&m_iExtraIntercept);
	pStream->Read(&m_iExtraEvasion);
	pStream->Read(&m_iExtraFirstStrikes);
	pStream->Read(&m_iExtraChanceFirstStrikes);
	pStream->Read(&m_iExtraWithdrawal);
	pStream->Read(&m_iExtraCollateralDamage);
	pStream->Read(&m_iExtraBombardRate);
	pStream->Read(&m_iExtraEnemyHeal);
	pStream->Read(&m_iExtraNeutralHeal);
	pStream->Read(&m_iExtraFriendlyHeal);
	pStream->Read(&m_iSameTileHeal);
	pStream->Read(&m_iAdjacentTileHeal);
	pStream->Read(&m_iExtraCombatPercent);
	pStream->Read(&m_iExtraCityAttackPercent);
	pStream->Read(&m_iExtraCityDefensePercent);
	pStream->Read(&m_iExtraHillsAttackPercent);
	pStream->Read(&m_iExtraHillsDefensePercent);
	pStream->Read(&m_iRevoltProtection);
	pStream->Read(&m_iCollateralDamageProtection);
	pStream->Read(&m_iPillageChange);
	pStream->Read(&m_iUpgradeDiscount);
	pStream->Read(&m_iExperiencePercent);
	pStream->Read(&m_iKamikazePercent);
	pStream->Read(&m_iBaseCombat);
	pStream->Read(&m_iBaseCombatFromPromotion);
	pStream->Read(&m_iBaseCombatFromPotion);
	pStream->Read(&m_iBaseCombatDefense);
	pStream->Read(&m_iBaseCombatDefenseFromPromotion);
	pStream->Read(&m_iBaseCombatDefenseFromPotion);
	pStream->Read((int*)&m_eFacingDirection);
	pStream->Read(&m_iImmobileTimer);

	pStream->Read(&m_bMadeAttack);
	pStream->Read(&m_bMadeInterception);
	pStream->Read(&m_bPromotionReady);
	pStream->Read(&m_bDeathDelay);
	pStream->Read(&m_bCombatFocus);
	// m_bInfoBarDirty not saved...
	pStream->Read(&m_bBlockading);
	if (uiFlag > 0)
	{
		pStream->Read(&m_bAirCombat);
	}

//FfH Spell System: Added by Kael 07/23/2007
	pStream->Read(&m_bFleeWithdrawl);
	pStream->Read(&m_bHasCasted);
	pStream->Read(&m_bIgnoreHide);
	pStream->Read(&m_iAlive);
	pStream->Read(&m_iAIControl);
	pStream->Read(&m_iBoarding);
	pStream->Read(&m_iDefensiveStrikeChance);
	pStream->Read(&m_iDefensiveStrikeDamage);
	pStream->Read(&m_iDoubleFortifyBonus);
	pStream->Read(&m_iFear);
	pStream->Read(&m_iFlying);
	pStream->Read(&m_iHeld);
	pStream->Read(&m_iHiddenNationality);
	pStream->Read(&m_iIgnoreBuildingDefense);
	pStream->Read(&m_iImmortal);
	pStream->Read(&m_iImmuneToCapture);
	pStream->Read(&m_iImmuneToRangeStrike);
	pStream->Read(&m_iImmuneToDefensiveStrike);
	pStream->Read(&m_iImmuneToFear);
	pStream->Read(&m_iImmuneToMagic);
	pStream->Read(&m_iInvisible);
	pStream->Read(&m_iSeeInvisible);
	pStream->Read(&m_iOnlyDefensive);
	pStream->Read(&m_iTargetWeakestUnit);
	pStream->Read(&m_iTargetWeakestUnitCounter);
	pStream->Read(&m_iTwincast);
	pStream->Read(&m_iWaterWalking);
	pStream->Read(&m_iBetterDefenderThanPercent);
	pStream->Read(&m_iCombatHealPercent);
	pStream->Read(&m_iCombatManaGained);
	pStream->Read(&m_iCombatLimit);
	pStream->Read(&m_iCombatPercentInBorders);
	pStream->Read(&m_iCombatPercentDefense);
	pStream->Read(&m_iCombatPercentGlobalCounter);
	pStream->Read(&m_iDelayedSpell);
	pStream->Read(&m_iDuration);
	pStream->Read(&m_iFreePromotionPick);
	pStream->Read(&m_iGoldFromCombat);
	pStream->Read(&m_iGroupSize);
	pStream->Read(&m_iInvisibleType);
	pStream->Read(&m_iRace);
	pStream->Read(&m_iReligion);
	pStream->Read(&m_iResist);
	pStream->Read(&m_iResistModify);
	pStream->Read(&m_iScenarioCounter);
	pStream->Read(&m_iSpellCasterXP);
	pStream->Read(&m_iSpellDamageModify);
	pStream->Read(&m_iSummoner);
	pStream->Read(&m_iTotalDamageTypeCombat);
	pStream->Read(&m_iUnitArtStyleType);
	pStream->Read(&m_iWorkRateModify);
	pStream->Read(GC.getNumBonusInfos(), m_paiBonusAffinity);
	pStream->Read(GC.getNumBonusInfos(), m_paiBonusAffinityAmount);
	pStream->Read(GC.getNumDamageTypeInfos(), m_paiDamageTypeCombat);
	pStream->Read(GC.getNumDamageTypeInfos(), m_paiDamageTypeResist);
	pStream->Read(GC.getNumDamageTypeInfos(), m_paiEquipmentDurabilityValidityCounter);
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (New Functions Definition) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    pStream->Read(&m_iAutoSpellCast);
    pStream->Read(&m_iMissionSpell);
    pStream->Read(&m_iCannotLeaveCulture);
    pStream->Read(&m_iExtraLives);
    pStream->Read(&m_iStrengthPotionsUsed);
    pStream->Read(&m_iDefensiveStrikesLeft);
    pStream->Read(&m_iNoXP);
	pStream->Read(&m_iCombatAuraID);
	pStream->Read(&m_iCombatAuraType);
	pStream->Read(&m_iBlessing1);
	pStream->Read(&m_iBlessing2);
	pStream->Read(&m_iCurse1);
	pStream->Read(&m_iCurse2);
	pStream->Read(&m_iCurse1Owner);
	pStream->Read(&m_iCurse2Owner);
	pStream->Read(&m_iBlessing1Owner);
	pStream->Read(&m_iBlessing2Owner);
	pStream->Read(&m_iFearEffect);
	pStream->Read(&m_iBonusVSUnitsFullHealth);
	pStream->Read(&m_iSpellTargetIncrease);
	pStream->Read(&m_iBonusSummonStrength);
	pStream->Read(&m_iIgnoreFirstStrikes);
	pStream->Read(&m_iCaptureAnimalChance);
	pStream->Read(&m_iCaptureBeastChance);
    pStream->Read(&m_bFlamingArrows);
    pStream->Read(&m_iEquipmentYieldType);
    pStream->Read(&m_bGarrision);
	pStream->Read(&m_iPower);
	pStream->Read(&m_iAIGroupID);
	pStream->Read(GC.getNumPromotionInfos(), m_piDenyPromotion);
	pStream->Read(GC.getNumEquipmentCategoryInfos(), m_piDurability);
	pStream->Read(GC.getNumEquipmentCategoryInfos(), m_piEquipment);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/*************************************************************************************************/
	pStream->Read(&m_iAirCombat);
	pStream->Read(&m_iAirCombatLimitBoost);
    pStream->Read(&m_bAutomatedRangeStrike);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//>>>>Refined Defensive Strikes: Added by Denev 2009/11/10
	pStream->Read(&m_iCombatDefensiveStrikes);
	pStream->Read(&m_iDefensiveStrikes);
//<<<<Refined Defensive Strikes: End Add
	pStream->Read((int*)&m_eOwner);
	pStream->Read((int*)&m_eCapturingPlayer);
	pStream->Read((int*)&m_eUnitType);
	FAssert(NO_UNIT != m_eUnitType);
	m_pUnitInfo = (NO_UNIT != m_eUnitType) ? &GC.getUnitInfo(m_eUnitType) : NULL;
	pStream->Read((int*)&m_eLeaderUnitType);

	pStream->Read((int*)&m_combatUnit.eOwner);
	pStream->Read(&m_combatUnit.iID);
	pStream->Read((int*)&m_transportUnit.eOwner);
	pStream->Read(&m_transportUnit.iID);

	pStream->Read(NUM_DOMAIN_TYPES, m_aiExtraDomainModifier);

	pStream->ReadString(m_szName);
	pStream->ReadString(m_szScriptData);

	pStream->Read(GC.getNumPromotionInfos(), m_pabHasPromotion);

	pStream->Read(GC.getNumTerrainInfos(), m_paiTerrainDoubleMoveCount);
	pStream->Read(GC.getNumFeatureInfos(), m_paiFeatureDoubleMoveCount);
	pStream->Read(GC.getNumTerrainInfos(), m_paiExtraTerrainAttackPercent);
	pStream->Read(GC.getNumTerrainInfos(), m_paiExtraTerrainDefensePercent);
	pStream->Read(GC.getNumFeatureInfos(), m_paiExtraFeatureAttackPercent);
	pStream->Read(GC.getNumFeatureInfos(), m_paiExtraFeatureDefensePercent);
	pStream->Read(GC.getNumUnitCombatInfos(), m_paiExtraUnitCombatModifier);
}


void CvUnit::write(FDataStreamBase* pStream)
{
	uint uiFlag=2;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iID);
	pStream->Write(m_iGroupID);
	pStream->Write(m_iHotKeyNumber);
	pStream->Write(m_iX);
	pStream->Write(m_iY);
	pStream->Write(m_iLastMoveTurn);
	pStream->Write(m_iReconX);
	pStream->Write(m_iReconY);
	pStream->Write(m_iGameTurnCreated);
	pStream->Write(m_iDamage);
	pStream->Write(m_iMoves);
	pStream->Write(m_iExperience);
	pStream->Write(m_iLevel);
	pStream->Write(m_iCargo);
	pStream->Write(m_iCargoCapacity);
	pStream->Write(m_iAttackPlotX);
	pStream->Write(m_iAttackPlotY);
	pStream->Write(m_iCombatTimer);
	pStream->Write(m_iCombatFirstStrikes);
	pStream->Write(m_iFortifyTurns);
	pStream->Write(m_iBlitzCount);
	pStream->Write(m_iAmphibCount);
	pStream->Write(m_iRiverCount);
	pStream->Write(m_iEnemyRouteCount);
	pStream->Write(m_iAlwaysHealCount);
	pStream->Write(m_iHillsDoubleMoveCount);
	pStream->Write(m_iImmuneToFirstStrikesCount);
	pStream->Write(m_iExtraVisibilityRange);
	pStream->Write(m_iExtraMoves);
	pStream->Write(m_iExtraMoveDiscount);
	pStream->Write(m_iExtraAirRange);
	pStream->Write(m_iExtraIntercept);
	pStream->Write(m_iExtraEvasion);
	pStream->Write(m_iExtraFirstStrikes);
	pStream->Write(m_iExtraChanceFirstStrikes);
	pStream->Write(m_iExtraWithdrawal);
	pStream->Write(m_iExtraCollateralDamage);
	pStream->Write(m_iExtraBombardRate);
	pStream->Write(m_iExtraEnemyHeal);
	pStream->Write(m_iExtraNeutralHeal);
	pStream->Write(m_iExtraFriendlyHeal);
	pStream->Write(m_iSameTileHeal);
	pStream->Write(m_iAdjacentTileHeal);
	pStream->Write(m_iExtraCombatPercent);
	pStream->Write(m_iExtraCityAttackPercent);
	pStream->Write(m_iExtraCityDefensePercent);
	pStream->Write(m_iExtraHillsAttackPercent);
	pStream->Write(m_iExtraHillsDefensePercent);
	pStream->Write(m_iRevoltProtection);
	pStream->Write(m_iCollateralDamageProtection);
	pStream->Write(m_iPillageChange);
	pStream->Write(m_iUpgradeDiscount);
	pStream->Write(m_iExperiencePercent);
	pStream->Write(m_iKamikazePercent);
	pStream->Write(m_iBaseCombat);
	pStream->Write(m_iBaseCombatFromPromotion);
	pStream->Write(m_iBaseCombatFromPotion);
	pStream->Write(m_iBaseCombatDefense);
	pStream->Write(m_iBaseCombatDefenseFromPromotion);
	pStream->Write(m_iBaseCombatDefenseFromPotion);

	pStream->Write(m_eFacingDirection);
	pStream->Write(m_iImmobileTimer);

	pStream->Write(m_bMadeAttack);
	pStream->Write(m_bMadeInterception);
	pStream->Write(m_bPromotionReady);
	pStream->Write(m_bDeathDelay);
	pStream->Write(m_bCombatFocus);
	// m_bInfoBarDirty not saved...
	pStream->Write(m_bBlockading);
	pStream->Write(m_bAirCombat);

//FfH Spell System: Added by Kael 07/23/2007
	pStream->Write(m_bFleeWithdrawl);
	pStream->Write(m_bHasCasted);
	pStream->Write(m_bIgnoreHide);
	pStream->Write(m_iAlive);
	pStream->Write(m_iAIControl);
	pStream->Write(m_iBoarding);
	pStream->Write(m_iDefensiveStrikeChance);
	pStream->Write(m_iDefensiveStrikeDamage);
	pStream->Write(m_iDoubleFortifyBonus);
	pStream->Write(m_iFear);
	pStream->Write(m_iFlying);
	pStream->Write(m_iHeld);
	pStream->Write(m_iHiddenNationality);
	pStream->Write(m_iIgnoreBuildingDefense);
	pStream->Write(m_iImmortal);
	pStream->Write(m_iImmuneToCapture);
	pStream->Write(m_iImmuneToRangeStrike);
	pStream->Write(m_iImmuneToDefensiveStrike);
	pStream->Write(m_iImmuneToFear);
	pStream->Write(m_iImmuneToMagic);
	pStream->Write(m_iInvisible);
	pStream->Write(m_iSeeInvisible);
	pStream->Write(m_iOnlyDefensive);
	pStream->Write(m_iTargetWeakestUnit);
	pStream->Write(m_iTargetWeakestUnitCounter);
	pStream->Write(m_iTwincast);
	pStream->Write(m_iWaterWalking);
	pStream->Write(m_iBetterDefenderThanPercent);
	pStream->Write(m_iCombatHealPercent);
	pStream->Write(m_iCombatManaGained);
	pStream->Write(m_iCombatLimit);
	pStream->Write(m_iCombatPercentInBorders);
	pStream->Write(m_iCombatPercentDefense);
	pStream->Write(m_iCombatPercentGlobalCounter);
	pStream->Write(m_iDelayedSpell);
	pStream->Write(m_iDuration);
	pStream->Write(m_iFreePromotionPick);
	pStream->Write(m_iGoldFromCombat);
	pStream->Write(m_iGroupSize);
	pStream->Write(m_iInvisibleType);
	pStream->Write(m_iRace);
	pStream->Write(m_iReligion);
	pStream->Write(m_iResist);
	pStream->Write(m_iResistModify);
	pStream->Write(m_iScenarioCounter);
	pStream->Write(m_iSpellCasterXP);
	pStream->Write(m_iSpellDamageModify);
	pStream->Write(m_iSummoner);
	pStream->Write(m_iTotalDamageTypeCombat);
	pStream->Write(m_iUnitArtStyleType);
	pStream->Write(m_iWorkRateModify);
	pStream->Write(GC.getNumBonusInfos(), m_paiBonusAffinity);
	pStream->Write(GC.getNumBonusInfos(), m_paiBonusAffinityAmount);
	pStream->Write(GC.getNumDamageTypeInfos(), m_paiDamageTypeCombat);
	pStream->Write(GC.getNumDamageTypeInfos(), m_paiDamageTypeResist);
	pStream->Write(GC.getNumDamageTypeInfos(), m_paiEquipmentDurabilityValidityCounter);
//FfH: End Add
/*************************************************************************************************/
/**	ADDON (New Functions Definition) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/
    pStream->Write(m_iAutoSpellCast);
    pStream->Write(m_iMissionSpell);
    pStream->Write(m_iCannotLeaveCulture);
    pStream->Write(m_iExtraLives);
    pStream->Write(m_iStrengthPotionsUsed);
    pStream->Write(m_iDefensiveStrikesLeft);
    pStream->Write(m_iNoXP);
	pStream->Write(m_iCombatAuraID);
	pStream->Write(m_iCombatAuraType);
	pStream->Write(m_iBlessing1);
	pStream->Write(m_iBlessing2);
	pStream->Write(m_iCurse1);
	pStream->Write(m_iCurse2);
	pStream->Write(m_iCurse1Owner);
	pStream->Write(m_iCurse2Owner);
	pStream->Write(m_iBlessing1Owner);
	pStream->Write(m_iBlessing2Owner);
	pStream->Write(m_iFearEffect);
	pStream->Write(m_iBonusVSUnitsFullHealth);
	pStream->Write(m_iSpellTargetIncrease);
	pStream->Write(m_iBonusSummonStrength);
	pStream->Write(m_iIgnoreFirstStrikes);
	pStream->Write(m_iCaptureAnimalChance);
	pStream->Write(m_iCaptureBeastChance);
    pStream->Write(m_bFlamingArrows);
    pStream->Write(m_iEquipmentYieldType);
    pStream->Write(m_bGarrision);
	pStream->Write(m_iPower);
	pStream->Write(m_iAIGroupID);
	pStream->Write(GC.getNumPromotionInfos(), m_piDenyPromotion);
	pStream->Write(GC.getNumEquipmentCategoryInfos(), m_piDurability);
	pStream->Write(GC.getNumEquipmentCategoryInfos(), m_piEquipment);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/*************************************************************************************************/
	pStream->Write(m_iAirCombat);
	pStream->Write(m_iAirCombatLimitBoost);
    pStream->Write(m_bAutomatedRangeStrike);
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//>>>>Refined Defensive Strikes: Added by Denev 2009/11/10
	pStream->Write(m_iCombatDefensiveStrikes);
	pStream->Write(m_iDefensiveStrikes);
//<<<<Refined Defensive Strikes: End Add
	pStream->Write(m_eOwner);
	pStream->Write(m_eCapturingPlayer);
	pStream->Write(m_eUnitType);
	pStream->Write(m_eLeaderUnitType);

	pStream->Write(m_combatUnit.eOwner);
	pStream->Write(m_combatUnit.iID);
	pStream->Write(m_transportUnit.eOwner);
	pStream->Write(m_transportUnit.iID);

	pStream->Write(NUM_DOMAIN_TYPES, m_aiExtraDomainModifier);

	pStream->WriteString(m_szName);
	pStream->WriteString(m_szScriptData);

	pStream->Write(GC.getNumPromotionInfos(), m_pabHasPromotion);

	pStream->Write(GC.getNumTerrainInfos(), m_paiTerrainDoubleMoveCount);
	pStream->Write(GC.getNumFeatureInfos(), m_paiFeatureDoubleMoveCount);
	pStream->Write(GC.getNumTerrainInfos(), m_paiExtraTerrainAttackPercent);
	pStream->Write(GC.getNumTerrainInfos(), m_paiExtraTerrainDefensePercent);
	pStream->Write(GC.getNumFeatureInfos(), m_paiExtraFeatureAttackPercent);
	pStream->Write(GC.getNumFeatureInfos(), m_paiExtraFeatureDefensePercent);
	pStream->Write(GC.getNumUnitCombatInfos(), m_paiExtraUnitCombatModifier);
}

// Protected Functions...

bool CvUnit::canAdvance(const CvPlot* pPlot, int iThreshold) const
{
	FAssert(canFight());
//	FAssert(!(isAnimal() && pPlot->isCity())); //modified animals Sephi
	FAssert(getDomainType() != DOMAIN_AIR);
	FAssert(getDomainType() != DOMAIN_IMMOBILE);

	if (pPlot->getNumVisibleEnemyDefenders(this) > iThreshold)
	{
		return false;
	}

	if (isNoCapture())
	{
		if (pPlot->isEnemyCity(*this))
		{
			return false;
		}
	}

	return true;
}


void CvUnit::collateralCombat(const CvPlot* pPlot, CvUnit* pSkipUnit)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;
	CvWString szBuffer;
	int iTheirStrength;
	int iStrengthFactor;
	int iCollateralDamage;
	int iUnitDamage;
	int iDamageCount;
	int iPossibleTargets;
	int iCount;
	int iValue;
	int iBestValue;
	std::map<CvUnit*, int> mapUnitDamage;
	std::map<CvUnit*, int>::iterator it;

	int iCollateralStrength = (getDomainType() == DOMAIN_AIR ? airBaseCombatStr() : baseCombatStr()) * (collateralDamage() + getExtraCollateralDamage()) / 100;
	if (iCollateralStrength == 0)
	{
		return;
	}

	iPossibleTargets = std::max(3, collateralDamageMaxUnits());
	iPossibleTargets = std::min((pPlot->getNumVisibleEnemyDefenders(this) - 1), iPossibleTargets);

	pUnitNode = pPlot->headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit != pSkipUnit)
		{
			if (isEnemy(pLoopUnit->getTeam(), pPlot))
			{
				if (!(pLoopUnit->isInvisible(getTeam(), false)))
				{
					if (pLoopUnit->canDefend())
					{
						iValue = (1 + GC.getGameINLINE().getSorenRandNum(10000, "Collateral Damage"));

						iValue *= pLoopUnit->currHitPoints();

						mapUnitDamage[pLoopUnit] = iValue;
					}
				}
			}
		}
	}

	CvCity* pCity = NULL;
	if (getDomainType() == DOMAIN_AIR)
	{
		pCity = pPlot->getPlotCity();
	}

	iDamageCount = 0;
	iCount = 0;

	while (iCount < iPossibleTargets)
	{
		iBestValue = 0;
		pBestUnit = NULL;

		for (it = mapUnitDamage.begin(); it != mapUnitDamage.end(); it++)
		{
			if (it->second > iBestValue)
			{
				iBestValue = it->second;
				pBestUnit = it->first;
			}
		}

		if (pBestUnit != NULL)
		{
			mapUnitDamage.erase(pBestUnit);

			if (NO_UNITCOMBAT == getUnitCombatType() || !pBestUnit->getUnitInfo().getUnitCombatCollateralImmune(getUnitCombatType()))
			{
				iTheirStrength = pBestUnit->baseCombatStr();

				iStrengthFactor = ((iCollateralStrength + iTheirStrength + 1) / 2);

				iCollateralDamage = (GC.getDefineINT("COLLATERAL_COMBAT_DAMAGE") * (iCollateralStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor);

				iCollateralDamage *= 100;

				iCollateralDamage *= std::max(0, 100 - pBestUnit->getCollateralDamageProtection());
				iCollateralDamage /= 100;

				if (pCity != NULL)
				{
					iCollateralDamage *= 100 + pCity->getAirModifier();
					iCollateralDamage /= 100;
				}

				iCollateralDamage /= 100;

				iCollateralDamage = std::max(0, iCollateralDamage);

				int iCollateralDamageLimit = std::max(collateralDamageLimit(),10);

				int iMaxDamage = std::min(iCollateralDamageLimit, (iCollateralDamageLimit * (iCollateralStrength + iStrengthFactor)) / (iTheirStrength + iStrengthFactor));
				iUnitDamage = std::max(pBestUnit->getDamage(), std::min(pBestUnit->getDamage() + iCollateralDamage, iMaxDamage));

				if (pBestUnit->getDamage() != iUnitDamage)
				{
					pBestUnit->setDamage(iUnitDamage, getOwnerINLINE());
					iDamageCount++;
				}
			}

			iCount++;
		}
		else
		{
			break;
		}
	}

	if (iDamageCount > 0)
	{
		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_SUFFER_COL_DMG", iDamageCount);
		gDLL->getInterfaceIFace()->addMessage(pSkipUnit->getOwnerINLINE(), (pSkipUnit->getDomainType() != DOMAIN_AIR), GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COLLATERAL", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pSkipUnit->getX_INLINE(), pSkipUnit->getY_INLINE(), true, true);

		szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_INFLICT_COL_DMG", getNameKey(), iDamageCount);
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COLLATERAL", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pSkipUnit->getX_INLINE(), pSkipUnit->getY_INLINE());
	}
}


void CvUnit::flankingStrikeCombat(const CvPlot* pPlot, int iAttackerStrength, int iAttackerFirepower, int iDefenderOdds, int iDefenderDamage, CvUnit* pSkipUnit)
{
	if (pPlot->isCity(true, pSkipUnit->getTeam()))
	{
		return;
	}

	CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

	std::vector< std::pair<CvUnit*, int> > listFlankedUnits;
	while (NULL != pUnitNode)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = pPlot->nextUnitNode(pUnitNode);

		if (pLoopUnit != pSkipUnit)
		{
			if (!pLoopUnit->isDead() && isEnemy(pLoopUnit->getTeam(), pPlot))
			{
				if (!(pLoopUnit->isInvisible(getTeam(), false)))
				{
					if (pLoopUnit->canDefend())
					{
						int iFlankingStrength = m_pUnitInfo->getFlankingStrikeUnitClass(pLoopUnit->getUnitClassType());

						if (iFlankingStrength > 0)
						{
							int iFlankedDefenderStrength;
							int iFlankedDefenderOdds;
							int iAttackerDamage;
							int iFlankedDefenderDamage;

							getDefenderCombatValues(*pLoopUnit, pPlot, iAttackerStrength, iAttackerFirepower, iFlankedDefenderOdds, iFlankedDefenderStrength, iAttackerDamage, iFlankedDefenderDamage);

							if (GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("COMBAT_DIE_SIDES"), "Flanking Combat") >= iDefenderOdds)
							{
								int iCollateralDamage = (iFlankingStrength * iDefenderDamage) / 100;
								int iUnitDamage = std::max(pLoopUnit->getDamage(), std::min(pLoopUnit->getDamage() + iCollateralDamage, collateralDamageLimit()));

								if (pLoopUnit->getDamage() != iUnitDamage)
								{
									listFlankedUnits.push_back(std::make_pair(pLoopUnit, iUnitDamage));
								}
							}
						}
					}
				}
			}
		}
	}

	int iNumUnitsHit = std::min((int)listFlankedUnits.size(), collateralDamageMaxUnits());

	for (int i = 0; i < iNumUnitsHit; ++i)
	{
		int iIndexHit = GC.getGameINLINE().getSorenRandNum(listFlankedUnits.size(), "Pick Flanked Unit");
		CvUnit* pUnit = listFlankedUnits[iIndexHit].first;
		int iDamage = listFlankedUnits[iIndexHit].second;
		pUnit->setDamage(iDamage, getOwnerINLINE());
		if (pUnit->isDead())
		{
			CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_KILLED_UNIT_BY_FLANKING", getNameKey(), pUnit->getNameKey(), pUnit->getVisualCivAdjective(getTeam()));
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNIT_DIED_BY_FLANKING", pUnit->getNameKey(), getNameKey(), getVisualCivAdjective(pUnit->getTeam()));
			gDLL->getInterfaceIFace()->addMessage(pUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

			pUnit->kill(false);
		}

		listFlankedUnits.erase(std::remove(listFlankedUnits.begin(), listFlankedUnits.end(), listFlankedUnits[iIndexHit]));
	}

	if (iNumUnitsHit > 0)
	{
		CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_DAMAGED_UNITS_BY_FLANKING", getNameKey(), iNumUnitsHit);
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitVictoryScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

		if (NULL != pSkipUnit)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNITS_DAMAGED_BY_FLANKING", getNameKey(), iNumUnitsHit);
			gDLL->getInterfaceIFace()->addMessage(pSkipUnit->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, GC.getEraInfo(GC.getGameINLINE().getCurrentEra()).getAudioUnitDefeatScript(), MESSAGE_TYPE_INFO, NULL, (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE());
		}
	}
}


// Returns true if we were intercepted...
bool CvUnit::interceptTest(const CvPlot* pPlot)
{
	if (GC.getGameINLINE().getSorenRandNum(100, "Evasion Rand") >= evasionProbability())
	{
		CvUnit* pInterceptor = bestInterceptor(pPlot);

		if (pInterceptor != NULL)
		{
			if (GC.getGameINLINE().getSorenRandNum(100, "Intercept Rand (Air)") < pInterceptor->currInterceptionProbability())
			{
				fightInterceptor(pPlot, false);

				return true;
			}
		}
	}

	return false;
}


CvUnit* CvUnit::airStrikeTarget(const CvPlot* pPlot) const
{
	CvUnit* pDefender;

/*************************************************************************************************/
/**	Xienwolf Tweak							04/15/09											**/
/**																								**/
/**				Prevent Ranged attacks against units you won't actually harm					**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	pDefender = pPlot->getBestDefender(NO_PLAYER, getOwnerINLINE(), this, true);
/**								----  End Original Code  ----									**/
	pDefender = pPlot->getBestRangedDefender(NO_PLAYER, getOwnerINLINE(), this, true);
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/

	if (pDefender != NULL)
	{
		if (!pDefender->isDead())
		{
			if (pDefender->canDefend())
			{
				return pDefender;
			}
		}
	}

	return NULL;
}


bool CvUnit::canAirStrike(const CvPlot* pPlot) const
{
	if (getDomainType() != DOMAIN_AIR)
	{
		return false;
	}

	if (!canAirAttack())
	{
		return false;
	}

	if (pPlot == plot())
	{
		return false;
	}

	if (!pPlot->isVisible(getTeam(), false))
	{
		return false;
	}

	if (plotDistance(getX_INLINE(), getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) > airRange())
	{
		return false;
	}

	if (airStrikeTarget(pPlot) == NULL)
	{
		return false;
	}

	return true;
}


bool CvUnit::airStrike(CvPlot* pPlot)
{
	if (!canAirStrike(pPlot))
	{
		return false;
	}

	if (interceptTest(pPlot))
	{
		return false;
	}

	CvUnit* pDefender = airStrikeTarget(pPlot);

	FAssert(pDefender != NULL);
	FAssert(pDefender->canDefend());

	setReconPlot(pPlot);

	setMadeAttack(true);
	changeMoves(GC.getMOVE_DENOMINATOR());

	int iDamage = airCombatDamage(pDefender);

	int iUnitDamage = std::max(pDefender->getDamage(), std::min((pDefender->getDamage() + iDamage), airCombatLimit()));

	CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pDefender->getNameKey(), getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACK", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pPlot->getX_INLINE(), pPlot->getY_INLINE(), true, true);

	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pDefender->getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_AIR_ATTACKED", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

	collateralCombat(pPlot, pDefender);

	pDefender->setDamage(iUnitDamage, getOwnerINLINE());

	return true;
}

bool CvUnit::canRangeStrike(bool bTest)
{
	if (getDomainType() == DOMAIN_AIR)
	{
		return false;
	}

	if (airRange() <= 0 && getUnitInfo().getBombardRange()<=0)
	{
		return false;
	}

	if (airBaseCombatStr() <= 0 && getUnitInfo().getBombardRate()<=0)
	{
		return false;
	}

    if (isBarbarian() && plot()->isOwned() && !GET_PLAYER(plot()->getOwner()).isHuman())
    {
        return false;
    }

	if(isGarrision())
	{
		return false;
	}

	if (!bTest)
	{
		if (!canFight())
		{
			return false;
		}

		if (isMadeAttack())
		{
			return false;
		}

        if (!canMove() && getMoves() > 0)
        {
            return false;
        }

	}

	return true;
}

/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**	Xienwolf Tweak							04/15/09											**/
/**																								**/
/**				Allows AI and Automated units to check potential attackers						**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
bool CvUnit::canRangeStrikeAt(const CvPlot* pPlot, int iX, int iY) const
{
	if (!canRangeStrike())
/**								----  End Original Code  ----									**/
bool CvUnit::canRangeStrikeAt(const CvPlot* pPlot, int iX, int iY, bool bTest)
{
	if (!canRangeStrike(bTest))
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/
	{
		return false;
	}

	CvPlot* pTargetPlot = GC.getMapINLINE().plotINLINE(iX, iY);

	if (NULL == pTargetPlot)
	{
		return false;
	}

	if (!pPlot->isVisible(getTeam(), false))
	{
		return false;
	}

	//can we do a Siege Attack?
	if(canBombard(pTargetPlot))
	{
		return true;
	}

	if(pTargetPlot->isCity())
	{
		return false;
	}

	if (plotDistance(pPlot->getX_INLINE(), pPlot->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > airRange())
	{
		return false;
	}

	CvUnit* pDefender = airStrikeTarget(pTargetPlot);
	if (NULL == pDefender)
	{
		return false;
	}

	if (!pPlot->canSeePlot(pTargetPlot, getTeam(), airRange(), getFacingDirection(true)))
	{
		return false;
	}

	return true;
}


bool CvUnit::rangeStrike(int iX, int iY)
{
	CvUnit* pDefender;
	CvWString szBuffer;
	int iUnitDamage;
	int iDamage;

	CvPlot* pPlot = GC.getMapINLINE().plot(iX, iY);
	if (NULL == pPlot)
	{
		return false;
	}

	if(GET_PLAYER(getOwnerINLINE()).AI_isRangedAttackMode())
	{
		if(!isAutomatedRangeStrike())
		{
			return false;
		}
	}

	if (!canRangeStrikeAt(pPlot, iX, iY))
	{
		return false;
	}

	if(canBombard())
	{
		bombard(pPlot);
		return true;
	}
	pDefender = airStrikeTarget(pPlot);

	FAssert(pDefender != NULL);
	FAssert(pDefender->canDefend());

	if (GC.getDefineINT("RANGED_ATTACKS_USE_MOVES") == 0)
	{
		setMadeAttack(true);
	}
/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**																								**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	changeMoves(GC.getMOVE_DENOMINATOR());
/**								----  End Original Code  ----									**/
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/


	iDamage = rangeCombatDamage(pDefender);

/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	iUnitDamage = std::max(pDefender->getDamage(), std::min((pDefender->getDamage() + iDamage), airCombatLimit()));
/**								----  End Original Code  ----									**/
	int iInitialUnitDamage = pDefender->getDamage();
	iUnitDamage = std::max(iInitialUnitDamage, std::min((iInitialUnitDamage + iDamage), airCombatLimit()));

	int iChance = GC.getGameINLINE().getSorenRandNum(100,"XP from Ranged Combat");

	iChance *=100;
	iChance /=std::max(1,(100-getExperience()));

	if(iChance<(iUnitDamage-iInitialUnitDamage))
	{
		changeExperience(1, -1, false, false, false);
	}

    pDefender->AI_setWasAttacked(2);
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**	Xienwolf Tweak							04/09/09											**/
/**																								**/
/**						Displays affiliation of opponent in ranged damage attacks				**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pDefender->getNameKey(), getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
	//red icon over attacking unit
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COMBAT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), this->getX_INLINE(), this->getY_INLINE(), true, true);
	//white icon over defending unit
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, 0, L"", "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pDefender->getX_INLINE(), pDefender->getY_INLINE(), true, true);

	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pDefender->getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()));
/**								----  End Original Code  ----									**/
	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ARE_ATTACKED_BY_AIR", pDefender->getNameKey(), getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()), GET_PLAYER(getVisualOwner(pDefender->getTeam())).getCivilizationAdjectiveKey());
	//red icon over attacking unit
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COMBAT", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), this->getX_INLINE(), this->getY_INLINE(), true, true);
	//white icon over defending unit
	gDLL->getInterfaceIFace()->addMessage(pDefender->getOwnerINLINE(), false, 0, L"", "AS2D_COMBAT", MESSAGE_TYPE_DISPLAY_ONLY, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), pDefender->getX_INLINE(), pDefender->getY_INLINE(), true, true);

	szBuffer = gDLL->getText("TXT_KEY_MISC_YOU_ATTACK_BY_AIR", getNameKey(), pDefender->getNameKey(), -(((iUnitDamage - pDefender->getDamage()) * 100) / pDefender->maxHitPoints()), GET_PLAYER(pDefender->getVisualOwner(getTeam())).getCivilizationAdjectiveKey());
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/

	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_COMBAT", MESSAGE_TYPE_INFO, pDefender->getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pPlot->getX_INLINE(), pPlot->getY_INLINE());

	collateralCombat(pPlot, pDefender);

	//set damage but don't update entity damage visibility
	pDefender->setDamage(iUnitDamage, getOwnerINLINE(), false);
	if(isFlamingArrows())
	{
		pDefender->doDamage(10,100,this,DAMAGE_FIRE,false);
	}
/**	Disabled because of OOS
	if (pPlot->isActiveVisible(false))
	{
		// Range strike entity mission
		CvMissionDefinition kDefiniton;
		kDefiniton.setMissionTime(GC.getMissionInfo(MISSION_RANGE_ATTACK).getTime() * gDLL->getSecsPerTurn());
		kDefiniton.setMissionType(MISSION_RANGE_ATTACK);
		kDefiniton.setPlot(pDefender->plot());
		kDefiniton.setUnit(BATTLE_UNIT_ATTACKER, this);
		kDefiniton.setUnit(BATTLE_UNIT_DEFENDER, pDefender);
		gDLL->getEntityIFace()->AddMission(&kDefiniton);

		//delay death
		pDefender->getGroup()->setMissionTimer(GC.getMissionInfo(MISSION_RANGE_ATTACK).getTime());
	}
**/
	return true;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::planBattle
//! \brief      Determines in general how a battle will progress.
//!
//!				Note that the outcome of the battle is not determined here. This function plans
//!				how many sub-units die and in which 'rounds' of battle.
//! \param      kBattleDefinition The battle definition, which receives the battle plan.
//! \retval     The number of game turns that the battle should be given.
//------------------------------------------------------------------------------------------------
int CvUnit::planBattle( CvBattleDefinition & kBattleDefinition ) const
{
#define BATTLE_TURNS_SETUP 4
#define BATTLE_TURNS_ENDING 4
#define BATTLE_TURNS_MELEE 6
#define BATTLE_TURNS_RANGED 6
#define BATTLE_TURN_RECHECK 4

	int								aiUnitsBegin[BATTLE_UNIT_COUNT];
	int								aiUnitsEnd[BATTLE_UNIT_COUNT];
	int								aiToKillMelee[BATTLE_UNIT_COUNT];
	int								aiToKillRanged[BATTLE_UNIT_COUNT];
	CvBattleRoundVector::iterator	iIterator;
	int								i, j;
	bool							bIsLoser;
	int								iRoundIndex;
	int								iTotalRounds = 0;
	int								iRoundCheck = BATTLE_TURN_RECHECK;

	// Initial conditions
	kBattleDefinition.setNumRangedRounds(0);
	kBattleDefinition.setNumMeleeRounds(0);

	int iFirstStrikesDelta = kBattleDefinition.getFirstStrikes(BATTLE_UNIT_ATTACKER) - kBattleDefinition.getFirstStrikes(BATTLE_UNIT_DEFENDER);
	if (iFirstStrikesDelta > 0) // Attacker first strikes
	{
		int iKills = computeUnitsToDie( kBattleDefinition, true, BATTLE_UNIT_DEFENDER );
		kBattleDefinition.setNumRangedRounds(std::max(iFirstStrikesDelta, iKills / iFirstStrikesDelta));
	}
	else if (iFirstStrikesDelta < 0) // Defender first strikes
	{
		int iKills = computeUnitsToDie( kBattleDefinition, true, BATTLE_UNIT_ATTACKER );
		iFirstStrikesDelta = -iFirstStrikesDelta;
		kBattleDefinition.setNumRangedRounds(std::max(iFirstStrikesDelta, iKills / iFirstStrikesDelta));
	}
	increaseBattleRounds( kBattleDefinition);


	// Keep randomizing until we get something valid
	do
	{
		iRoundCheck++;
		if ( iRoundCheck >= BATTLE_TURN_RECHECK )
		{
			increaseBattleRounds( kBattleDefinition);
			iTotalRounds = kBattleDefinition.getNumRangedRounds() + kBattleDefinition.getNumMeleeRounds();
			iRoundCheck = 0;
		}

		// Make sure to clear the battle plan, we may have to do this again if we can't find a plan that works.
		kBattleDefinition.clearBattleRounds();

		// Create the round list
		CvBattleRound kRound;
		kBattleDefinition.setBattleRound(iTotalRounds, kRound);

		// For the attacker and defender
		for ( i = 0; i < BATTLE_UNIT_COUNT; i++ )
		{
			// Gather some initial information
			BattleUnitTypes unitType = (BattleUnitTypes) i;
			aiUnitsBegin[unitType] = kBattleDefinition.getUnit(unitType)->getSubUnitsAlive(kBattleDefinition.getDamage(unitType, BATTLE_TIME_BEGIN));
			aiToKillRanged[unitType] = computeUnitsToDie( kBattleDefinition, true, unitType);
			aiToKillMelee[unitType] = computeUnitsToDie( kBattleDefinition, false, unitType);
			aiUnitsEnd[unitType] = aiUnitsBegin[unitType] - aiToKillMelee[unitType] - aiToKillRanged[unitType];

			// Make sure that if they aren't dead at the end, they have at least one unit left
			if ( aiUnitsEnd[unitType] == 0 && !kBattleDefinition.getUnit(unitType)->isDead() )
			{
				aiUnitsEnd[unitType]++;
				if ( aiToKillMelee[unitType] > 0 )
				{
					aiToKillMelee[unitType]--;
				}
				else
				{
					aiToKillRanged[unitType]--;
				}
			}

			// If one unit is the loser, make sure that at least one of their units dies in the last round
			if ( aiUnitsEnd[unitType] == 0 )
			{
				kBattleDefinition.getBattleRound(iTotalRounds - 1).addNumKilled(unitType, 1);
				if ( aiToKillMelee[unitType] > 0)
				{
					aiToKillMelee[unitType]--;
				}
				else
				{
					aiToKillRanged[unitType]--;
				}
			}

			// Randomize in which round each death occurs
			bIsLoser = aiUnitsEnd[unitType] == 0;

			// Randomize the ranged deaths
			for ( j = 0; j < aiToKillRanged[unitType]; j++ )
			{
				iRoundIndex = GC.getGameINLINE().getSorenRandNum( range( kBattleDefinition.getNumRangedRounds(), 0, kBattleDefinition.getNumRangedRounds()), "Ranged combat death");
				kBattleDefinition.getBattleRound(iRoundIndex).addNumKilled(unitType, 1);
			}

			// Randomize the melee deaths
			for ( j = 0; j < aiToKillMelee[unitType]; j++ )
			{
				iRoundIndex = GC.getGameINLINE().getSorenRandNum( range( kBattleDefinition.getNumMeleeRounds() - (bIsLoser ? 1 : 2 ), 0, kBattleDefinition.getNumMeleeRounds()), "Melee combat death");
				kBattleDefinition.getBattleRound(kBattleDefinition.getNumRangedRounds() + iRoundIndex).addNumKilled(unitType, 1);
			}

			// Compute alive sums
			int iNumberKilled = 0;
			for(int j=0;j<kBattleDefinition.getNumBattleRounds();j++)
			{
				CvBattleRound &round = kBattleDefinition.getBattleRound(j);
				round.setRangedRound(j < kBattleDefinition.getNumRangedRounds());
				iNumberKilled += round.getNumKilled(unitType);
				round.setNumAlive(unitType, aiUnitsBegin[unitType] - iNumberKilled);
			}
		}

		// Now compute wave sizes
		for(int i=0;i<kBattleDefinition.getNumBattleRounds();i++)
		{
			CvBattleRound &round = kBattleDefinition.getBattleRound(i);
			round.setWaveSize(computeWaveSize(round.isRangedRound(), round.getNumAlive(BATTLE_UNIT_ATTACKER) + round.getNumKilled(BATTLE_UNIT_ATTACKER), round.getNumAlive(BATTLE_UNIT_DEFENDER) + round.getNumKilled(BATTLE_UNIT_DEFENDER)));
		}

		if ( iTotalRounds > 400 )
		{
			kBattleDefinition.setNumMeleeRounds(1);
			kBattleDefinition.setNumRangedRounds(0);
			break;
		}
	}
	while ( !verifyRoundsValid( kBattleDefinition ));

	//add a little extra time for leader to surrender
	bool attackerLeader = false;
	bool defenderLeader = false;
	bool attackerDie = false;
	bool defenderDie = false;
	int lastRound = kBattleDefinition.getNumBattleRounds() - 1;
	if(kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->getLeaderUnitType() != NO_UNIT)
		attackerLeader = true;
	if(kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->getLeaderUnitType() != NO_UNIT)
		defenderLeader = true;
	if(kBattleDefinition.getBattleRound(lastRound).getNumAlive(BATTLE_UNIT_ATTACKER) == 0)
		attackerDie = true;
	if(kBattleDefinition.getBattleRound(lastRound).getNumAlive(BATTLE_UNIT_DEFENDER) == 0)
		defenderDie = true;

	int extraTime = 0;
	if((attackerLeader && attackerDie) || (defenderLeader && defenderDie))
		extraTime = BATTLE_TURNS_MELEE;
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
	if(gDLL->getEntityIFace()->GetSiegeTower(kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->getUnitEntity()) || gDLL->getEntityIFace()->GetSiegeTower(kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->getUnitEntity()))
/**
	if ( (kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->isInViewport() && gDLL->getEntityIFace()->GetSiegeTower(kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->getUnitEntity())) ||
		 (kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->isInViewport() && gDLL->getEntityIFace()->GetSiegeTower(kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->getUnitEntity())) )
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		extraTime = BATTLE_TURNS_MELEE;

	return BATTLE_TURNS_SETUP + BATTLE_TURNS_ENDING + kBattleDefinition.getNumMeleeRounds() * BATTLE_TURNS_MELEE + kBattleDefinition.getNumRangedRounds() * BATTLE_TURNS_MELEE + extraTime;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:	CvBattleManager::computeDeadUnits
//! \brief		Computes the number of units dead, for either the ranged or melee portion of combat.
//! \param		kDefinition The battle definition.
//! \param		bRanged true if computing the number of units that die during the ranged portion of combat,
//!					false if computing the number of units that die during the melee portion of combat.
//! \param		iUnit The index of the unit to compute (BATTLE_UNIT_ATTACKER or BATTLE_UNIT_DEFENDER).
//! \retval		The number of units that should die for the given unit in the given portion of combat
//------------------------------------------------------------------------------------------------
int CvUnit::computeUnitsToDie( const CvBattleDefinition & kDefinition, bool bRanged, BattleUnitTypes iUnit ) const
{
	FAssertMsg( iUnit == BATTLE_UNIT_ATTACKER || iUnit == BATTLE_UNIT_DEFENDER, "Invalid unit index");

	BattleTimeTypes iBeginIndex = bRanged ? BATTLE_TIME_BEGIN : BATTLE_TIME_RANGED;
	BattleTimeTypes iEndIndex = bRanged ? BATTLE_TIME_RANGED : BATTLE_TIME_END;
	return kDefinition.getUnit(iUnit)->getSubUnitsAlive(kDefinition.getDamage(iUnit, iBeginIndex)) -
		kDefinition.getUnit(iUnit)->getSubUnitsAlive( kDefinition.getDamage(iUnit, iEndIndex));
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::verifyRoundsValid
//! \brief      Verifies that all rounds in the battle plan are valid
//! \param      vctBattlePlan The battle plan
//! \retval     true if the battle plan (seems) valid, false otherwise
//------------------------------------------------------------------------------------------------
bool CvUnit::verifyRoundsValid( const CvBattleDefinition & battleDefinition ) const
{
	for(int i=0;i<battleDefinition.getNumBattleRounds();i++)
	{
		if(!battleDefinition.getBattleRound(i).isValid())
			return false;
	}
	return true;
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::increaseBattleRounds
//! \brief      Increases the number of rounds in the battle.
//! \param      kBattleDefinition The definition of the battle
//------------------------------------------------------------------------------------------------
void CvUnit::increaseBattleRounds( CvBattleDefinition & kBattleDefinition ) const
{
	if ( kBattleDefinition.getUnit(BATTLE_UNIT_ATTACKER)->isRanged() && kBattleDefinition.getUnit(BATTLE_UNIT_DEFENDER)->isRanged())
	{
		kBattleDefinition.addNumRangedRounds(1);
	}
	else
	{
		kBattleDefinition.addNumMeleeRounds(1);
	}
}

//------------------------------------------------------------------------------------------------
// FUNCTION:    CvUnit::computeWaveSize
//! \brief      Computes the wave size for the round.
//! \param      bRangedRound true if the round is a ranged round
//! \param		iAttackerMax The maximum number of attackers that can participate in a wave (alive)
//! \param		iDefenderMax The maximum number of Defenders that can participate in a wave (alive)
//! \retval     The desired wave size for the given parameters
//------------------------------------------------------------------------------------------------
int CvUnit::computeWaveSize( bool bRangedRound, int iAttackerMax, int iDefenderMax ) const
{
	FAssertMsg( getCombatUnit() != NULL, "You must be fighting somebody!" );
	int aiDesiredSize[BATTLE_UNIT_COUNT];
	if ( bRangedRound )
	{
		aiDesiredSize[BATTLE_UNIT_ATTACKER] = getUnitInfo().getRangedWaveSize();
		aiDesiredSize[BATTLE_UNIT_DEFENDER] = getCombatUnit()->getUnitInfo().getRangedWaveSize();
	}
	else
	{
		aiDesiredSize[BATTLE_UNIT_ATTACKER] = getUnitInfo().getMeleeWaveSize();
		aiDesiredSize[BATTLE_UNIT_DEFENDER] = getCombatUnit()->getUnitInfo().getMeleeWaveSize();
	}

	aiDesiredSize[BATTLE_UNIT_DEFENDER] = aiDesiredSize[BATTLE_UNIT_DEFENDER] <= 0 ? iDefenderMax : aiDesiredSize[BATTLE_UNIT_DEFENDER];
	aiDesiredSize[BATTLE_UNIT_ATTACKER] = aiDesiredSize[BATTLE_UNIT_ATTACKER] <= 0 ? iDefenderMax : aiDesiredSize[BATTLE_UNIT_ATTACKER];
	return std::min( std::min( aiDesiredSize[BATTLE_UNIT_ATTACKER], iAttackerMax ), std::min( aiDesiredSize[BATTLE_UNIT_DEFENDER],
		iDefenderMax) );
}

bool CvUnit::isTargetOf(const CvUnit& attacker) const
{
	CvUnitInfo& attackerInfo = attacker.getUnitInfo();
	CvUnitInfo& ourInfo = getUnitInfo();

	if (!plot()->isCity(true, getTeam()))
	{
		if (NO_UNITCLASS != getUnitClassType() && attackerInfo.getTargetUnitClass(getUnitClassType()))
		{
			return true;
		}

		if (NO_UNITCOMBAT != getUnitCombatType() && attackerInfo.getTargetUnitCombat(getUnitCombatType()))
		{
			return true;
		}

		for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
		{
			if(attacker.isHasPromotion((PromotionTypes)iI))
			{
				if(GC.getPromotionInfo((PromotionTypes)iI).getUnitCombatTarget()!=NO_UNITCOMBAT && GC.getPromotionInfo((PromotionTypes)iI).getUnitCombatTarget()==getUnitCombatType())
				{
					return true;
				}
			}
		}
	}

	if (NO_UNITCLASS != attackerInfo.getUnitClassType() && ourInfo.getDefenderUnitClass(attackerInfo.getUnitClassType()))
	{
		return true;
	}

	if (NO_UNITCOMBAT != attackerInfo.getUnitCombatType() && ourInfo.getDefenderUnitCombat(attackerInfo.getUnitCombatType()))
	{
		return true;
	}

	return false;
}

bool CvUnit::isEnemy(TeamTypes eTeam, const CvPlot* pPlot) const
{
	if (NULL == pPlot)
	{
		pPlot = plot();
	}

//FfH: Added by Kael 10/26/2007 (to prevent spinlocks when always hostile units attack barbarian allied teams)
    if (isAlwaysHostile(pPlot))
    {
        if (getTeam() != eTeam)
        {
            return true;
        }
    }
//FfH: End Add

	return (atWar(GET_PLAYER(getCombatOwner(eTeam, pPlot)).getTeam(), eTeam));
}

bool CvUnit::isPotentialEnemy(TeamTypes eTeam, const CvPlot* pPlot) const
{
	if (NULL == pPlot)
	{
		pPlot = plot();
	}

	return (::isPotentialEnemy(GET_PLAYER(getCombatOwner(eTeam, pPlot)).getTeam(), eTeam));
}

bool CvUnit::isSuicide() const
{
	return (m_pUnitInfo->isSuicide() || getKamikazePercent() != 0);
}

int CvUnit::getDropRange() const
{
	return (m_pUnitInfo->getDropRange());
}

void CvUnit::getDefenderCombatValues(CvUnit& kDefender, const CvPlot* pPlot, int iOurStrength, int iOurFirepower, int& iTheirOdds, int& iTheirStrength, int& iOurDamage, int& iTheirDamage, CombatDetails* pTheirDetails) const
{
	iTheirStrength = kDefender.currCombatStr(pPlot, this, pTheirDetails);
	int iTheirFirepower = kDefender.currFirepower(pPlot, this);

	FAssert((iOurStrength + iTheirStrength) > 0);
	FAssert((iOurFirepower + iTheirFirepower) > 0);

	iTheirOdds = ((GC.getDefineINT("COMBAT_DIE_SIDES") * iTheirStrength) / (iOurStrength + iTheirStrength));

	if (kDefender.isBarbarian())
	{
		if (GET_PLAYER(getOwnerINLINE()).getWinsVsBarbs() < GC.getHandicapInfo(GET_PLAYER(getOwnerINLINE()).getHandicapType()).getFreeWinsVsBarbs())
		{
			iTheirOdds = std::min((10 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iTheirOdds);
		}
	}
	if (isBarbarian())
	{
		if (GET_PLAYER(kDefender.getOwnerINLINE()).getWinsVsBarbs() < GC.getHandicapInfo(GET_PLAYER(kDefender.getOwnerINLINE()).getHandicapType()).getFreeWinsVsBarbs())
		{
			iTheirOdds =  std::max((90 * GC.getDefineINT("COMBAT_DIE_SIDES")) / 100, iTheirOdds);
		}
	}

	//added Sephi (new Combat Odds)
	if (!kDefender.isHuman() && !kDefender.isBarbarian() && isBarbarian())
	{
	    iTheirOdds = std::max(1000-10*GC.getHandicapInfo(GET_PLAYER((PlayerTypes)0).getHandicapType()).getAICombatOddsCapped(),iTheirOdds);
	}

	if (!isHuman() && !isBarbarian() && kDefender.isBarbarian())
	{
	    iTheirOdds = std::min(10*GC.getHandicapInfo(GET_PLAYER((PlayerTypes)0).getHandicapType()).getAICombatOddsCapped(),iTheirOdds);
	}
    //end new combat odds

	int iStrengthFactor = ((iOurFirepower + iTheirFirepower + 1) / 2);

	iOurDamage = std::max(1, ((GC.getDefineINT("COMBAT_DAMAGE") * (iTheirFirepower + iStrengthFactor)) / (iOurFirepower + iStrengthFactor)));
	iTheirDamage = std::max(1, ((GC.getDefineINT("COMBAT_DAMAGE") * (iOurFirepower + iStrengthFactor)) / (iTheirFirepower + iStrengthFactor)));
}

int CvUnit::getTriggerValue(EventTriggerTypes eTrigger, const CvPlot* pPlot, bool bCheckPlot) const
{
	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eTrigger);
	if (kTrigger.getNumUnits() <= 0)
	{
		return MIN_INT;
	}

	if (isDead())
	{
		return MIN_INT;
	}

	if (!CvString(kTrigger.getPythonCanDoUnit()).empty())
	{
		long lResult;

		CyArgsList argsList;
		argsList.add(eTrigger);
		argsList.add(getOwnerINLINE());
		argsList.add(getID());

		gDLL->getPythonIFace()->callFunction(PYRandomEventModule, kTrigger.getPythonCanDoUnit(), argsList.makeFunctionArgs(), &lResult);

		if (0 == lResult)
		{
			return MIN_INT;
		}
	}

	if (kTrigger.getNumUnitsRequired() > 0)
	{
		bool bFoundValid = false;
		for (int i = 0; i < kTrigger.getNumUnitsRequired(); ++i)
		{
			if (getUnitClassType() == kTrigger.getUnitRequired(i))
			{
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			return MIN_INT;
		}
	}

	if (bCheckPlot)
	{
		if (kTrigger.isUnitsOnPlot())
		{
			if (!plot()->canTrigger(eTrigger, getOwnerINLINE()))
			{
				return MIN_INT;
			}
		}
	}

	int iValue = 0;

	if (0 == getDamage() && kTrigger.getUnitDamagedWeight() > 0)
	{
		return MIN_INT;
	}

	iValue += getDamage() * kTrigger.getUnitDamagedWeight();

	iValue += getExperience() * kTrigger.getUnitExperienceWeight();

	if (NULL != pPlot)
	{
		iValue += plotDistance(getX_INLINE(), getY_INLINE(), pPlot->getX_INLINE(), pPlot->getY_INLINE()) * kTrigger.getUnitDistanceWeight();
	}

	return iValue;
}

bool CvUnit::canApplyEvent(EventTypes eEvent) const
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (0 != kEvent.getUnitExperience())
	{
		if (!canAcquirePromotionAny())
		{
			return false;
		}
	}

	if (NO_PROMOTION != kEvent.getUnitPromotion())
	{

//FfH: Modified by Kael 10/29/2007
//		if (!canAcquirePromotion((PromotionTypes)kEvent.getUnitPromotion()))
//		{
//			return false;
//		}
        if (isHasPromotion((PromotionTypes)kEvent.getUnitPromotion()))
        {
            return false;
        }
//FfH: End Modify

	}

	if (kEvent.getUnitImmobileTurns() > 0)
	{
		if (!canAttack())
		{
			return false;
		}
	}

	return true;
}

void CvUnit::applyEvent(EventTypes eEvent)
{
	if (!canApplyEvent(eEvent))
	{
		return;
	}

	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (0 != kEvent.getUnitExperience())
	{
		setDamage(0);
		changeExperience(kEvent.getUnitExperience());
	}

	if (NO_PROMOTION != kEvent.getUnitPromotion())
	{

//FfH: Modified by Kael 02/02/2009 (if we spawn a new unit the promotion goes to the spawned unit, not to the eventtrigger target)
//		setHasPromotion((PromotionTypes)kEvent.getUnitPromotion(), true);
        if (kEvent.getUnitClass() == NO_UNITCLASS || kEvent.getNumUnits() == 0)
        {
            setHasPromotion((PromotionTypes)kEvent.getUnitPromotion(), true);
        }
//FfH: End Modify

	}

	if (kEvent.getUnitImmobileTurns() > 0)
	{
		changeImmobileTimer(kEvent.getUnitImmobileTurns());
		CvWString szText = gDLL->getText("TXT_KEY_EVENT_UNIT_IMMOBILE", getNameKey(), kEvent.getUnitImmobileTurns());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szText, "AS2D_UNITGIFTED", MESSAGE_TYPE_INFO, getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_UNIT_TEXT"), getX_INLINE(), getY_INLINE(), true, true);
	}

	CvWString szNameKey(kEvent.getUnitNameKey());

	if (!szNameKey.empty())
	{
		setName(gDLL->getText(kEvent.getUnitNameKey()));
	}

	if (kEvent.isDisbandUnit())
	{
		kill(false);
	}
}

const CvArtInfoUnit* CvUnit::getArtInfo(int i, EraTypes eEra) const
{

//FfH: Added by Kael 10/26/2007
    if (getUnitArtStyleType() != NO_UNIT_ARTSTYLE)
    {
        return m_pUnitInfo->getArtInfo(i, eEra, (UnitArtStyleTypes) getUnitArtStyleType());
    }
//FfH: End Add

	return m_pUnitInfo->getArtInfo(i, eEra, (UnitArtStyleTypes) GC.getCivilizationInfo(getCivilizationType()).getUnitArtStyleType());
}

const TCHAR* CvUnit::getButton() const
{
	const CvArtInfoUnit* pArtInfo = getArtInfo(0, GET_PLAYER(getOwnerINLINE()).getCurrentEra());

	if (NULL != pArtInfo)
	{
		return pArtInfo->getButton();
	}

	return m_pUnitInfo->getButton();
}

//FfH: Modified by Kael 06/17/2009
//int CvUnit::getGroupSize() const
//{
//	return m_pUnitInfo->getGroupSize();
//}
int CvUnit::getGroupSize() const
{
    if (GC.getGameINLINE().isOption(GAMEOPTION_ADVENTURE_MODE))
    {
        return 1;
    }
	return m_iGroupSize;
}
//FfH: End Modify

int CvUnit::getGroupDefinitions() const
{
	return m_pUnitInfo->getGroupDefinitions();
}

int CvUnit::getUnitGroupRequired(int i) const
{
	return m_pUnitInfo->getUnitGroupRequired(i);
}

bool CvUnit::isRenderAlways() const
{
	return m_pUnitInfo->isRenderAlways();
}

float CvUnit::getAnimationMaxSpeed() const
{
	return m_pUnitInfo->getUnitMaxSpeed();
}

float CvUnit::getAnimationPadTime() const
{
	return m_pUnitInfo->getUnitPadTime();
}

const char* CvUnit::getFormationType() const
{
	return m_pUnitInfo->getFormationType();
}

bool CvUnit::isMechUnit() const
{
	return m_pUnitInfo->isMechUnit();
}

bool CvUnit::isRenderBelowWater() const
{
	return m_pUnitInfo->isRenderBelowWater();
}

int CvUnit::getRenderPriority(UnitSubEntityTypes eUnitSubEntity, int iMeshGroupType, int UNIT_MAX_SUB_TYPES) const
{
	if (eUnitSubEntity == UNIT_SUB_ENTITY_SIEGE_TOWER)
	{
		return (getOwner() * (GC.getNumUnitInfos() + 2) * UNIT_MAX_SUB_TYPES) + iMeshGroupType;
	}
	else
	{
		return (getOwner() * (GC.getNumUnitInfos() + 2) * UNIT_MAX_SUB_TYPES) + m_eUnitType * UNIT_MAX_SUB_TYPES + iMeshGroupType;
	}
}

bool CvUnit::isAlwaysHostile(const CvPlot* pPlot) const
{

//FfH: Added by Kael 09/15/2007
    if (isHiddenNationality())
    {
        return true;
    }
//FfH: End Add

	if (!m_pUnitInfo->isAlwaysHostile())
	{
		return false;
	}

	if (NULL != pPlot && pPlot->isCity(true, getTeam()))
	{
		return false;
	}

	return true;
}

bool CvUnit::verifyStackValid()
{
	if (!alwaysInvisible())
	{
		if (plot()->isVisibleEnemyUnit(this))
		{
			return jumpToNearestValidPlot();
		}
	}

	return true;
}


// Private Functions...

//check if quick combat
bool CvUnit::isCombatVisible(const CvUnit* pDefender) const
{
	bool bVisible = false;

	if (!m_pUnitInfo->isQuickCombat())
	{
		if (NULL == pDefender || !pDefender->getUnitInfo().isQuickCombat())
		{
			if (isHuman())
			{
				if (!GET_PLAYER(getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_ATTACK))
				{
					bVisible = true;
				}
			}
			else if (NULL != pDefender && pDefender->isHuman())
			{
				if (!GET_PLAYER(pDefender->getOwnerINLINE()).isOption(PLAYEROPTION_QUICK_DEFENSE))
				{
					bVisible = true;
				}
			}
		}
	}

	return bVisible;
}

// used by the executable for the red glow and plot indicators
bool CvUnit::shouldShowEnemyGlow(TeamTypes eForTeam) const
{
	if (isDelayedDeath())
	{
		return false;
	}

	if (getDomainType() == DOMAIN_AIR)
	{
		return false;
	}

	if (!canFight())
	{
		return false;
	}

	CvPlot* pPlot = plot();
	if (pPlot == NULL)
	{
		return false;
	}

	TeamTypes ePlotTeam = pPlot->getTeam();
	if (ePlotTeam != eForTeam)
	{
		return false;
	}

	if (!isEnemy(ePlotTeam))
	{
		return false;
	}

	return true;
}

bool CvUnit::shouldShowFoundBorders() const
{
	return isFound();
}


void CvUnit::cheat(bool bCtrl, bool bAlt, bool bShift)
{
	if (gDLL->getChtLvl() > 0)
	{
		if (bCtrl)
		{
			setPromotionReady(true);
		}
	}
}

float CvUnit::getHealthBarModifier() const
{
	return (GC.getDefineFLOAT("HEALTH_BAR_WIDTH") / (GC.getGameINLINE().getBestLandUnitCombat() * 2));
}

void CvUnit::getLayerAnimationPaths(std::vector<AnimationPathTypes>& aAnimationPaths) const
{
	for (int i=0; i < GC.getNumPromotionInfos(); ++i)
	{
		PromotionTypes ePromotion = (PromotionTypes) i;
		if (isHasPromotion(ePromotion))
		{
			AnimationPathTypes eAnimationPath = (AnimationPathTypes) GC.getPromotionInfo(ePromotion).getLayerAnimationPath();
			if(eAnimationPath != ANIMATIONPATH_NONE)
			{
				aAnimationPaths.push_back(eAnimationPath);
			}
		}
	}
}

int CvUnit::getSelectionSoundScript() const
{
	int iScriptId = getArtInfo(0, GET_PLAYER(getOwnerINLINE()).getCurrentEra())->getSelectionSoundScriptId();
	if (iScriptId == -1)
	{
		iScriptId = GC.getCivilizationInfo(getCivilizationType()).getSelectionSoundScriptId();
	}
	return iScriptId;
}

/*************************************************************************************************/
/**	ADDON (New Functions Definition) Sephi                                     					**/
/**																								**/
/**						                                            							**/
/*************************************************************************************************/

//Unlike CvUnit::canJoinGroup this Function doesn't check for atPlot(), so we can use it to plan AI moves ahead of time
bool CvUnit::AI_canJoinGroup(CvSelectionGroup* pSelectionGroup) const
{
	CvUnit* pHeadUnit;

	if (pSelectionGroup->getOwnerINLINE() == NO_PLAYER)
	{
		pHeadUnit = pSelectionGroup->getHeadUnit();

		if (pHeadUnit != NULL)
		{
			if (pHeadUnit->getOwnerINLINE() != getOwnerINLINE())
			{
				return false;
			}
		}
	}
	else
	{
		if (pSelectionGroup->getOwnerINLINE() != getOwnerINLINE())
		{
			return false;
		}
	}

	if (pSelectionGroup->getNumUnits() > 0)
	{
		if (pSelectionGroup->getDomainType() != getDomainType())
		{
			return false;
		}

        pHeadUnit = pSelectionGroup->getHeadUnit();
        if (pHeadUnit != NULL)
	    {
			if (pHeadUnit->isHiddenNationality() != isHiddenNationality())
			{
                return false;
			}
            if (pHeadUnit->isAIControl() != isAIControl())
			{
                return false;
			}
		}
	}

	return true;
}

bool CvUnit::isDenyPromotion(PromotionTypes eIndex) const
{
	return (m_piDenyPromotion[eIndex] > 0);
}
void CvUnit::changeDenyPromotion(PromotionTypes eIndex, int iChange)
{
	if (iChange != 0)
	{
		m_piDenyPromotion[eIndex] = std::max(0, m_piDenyPromotion[eIndex] + iChange);
	}
}

int CvUnit::getDurability(EquipmentCategoryTypes eCategory) const
{
	FAssertMsg(eCategory>=0,"getDurability called with wrong Parameter");
	FAssertMsg(eCategory<GC.getNumEquipmentCategoryInfos(),"getDurability called with wrong Parameter");
	return m_piDurability[eCategory];
}

void CvUnit::changeDurability(EquipmentCategoryTypes eCategory,int iChange)
{
	FAssertMsg(eCategory>=0,"changeDurability called with wrong Parameter");
	FAssertMsg(eCategory<GC.getNumEquipmentCategoryInfos(),"changeDurability called with wrong Parameter");
	m_piDurability[eCategory]+=iChange;
}

PromotionTypes CvUnit::getEquipment(EquipmentCategoryTypes eCategory) const
{
	FAssertMsg(eCategory>=0,"getEquipment called with wrong Parameter");
	FAssertMsg(eCategory<GC.getNumEquipmentCategoryInfos(),"getEquipment called with wrong Parameter");
	return (PromotionTypes)m_piEquipment[eCategory];
}

void CvUnit::setEquipment(EquipmentCategoryTypes eCategory,int iNewValue)
{
	FAssertMsg(eCategory>=0,"changeEquipment called with wrong Parameter");
	FAssertMsg(eCategory<GC.getNumEquipmentCategoryInfos(),"changeEquipment called with wrong Parameter");
	m_piEquipment[eCategory]=iNewValue;
}

bool CvUnit::isCannotLeaveCulture() const
{
	return m_iCannotLeaveCulture == 0 ? false : true;
}

void CvUnit::changeCannotLeaveCulture(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iCannotLeaveCulture += iNewValue;
        if (m_iCannotLeaveCulture < 0)
        {
            m_iCannotLeaveCulture = 0;
        }
    }
}

bool CvUnit::isNoXP() const
{
	return m_iNoXP > 0;
}

void CvUnit::changeNoXP(int iChange)
{
    if (iChange != 0)
    {
        m_iNoXP += iChange;
        if (m_iNoXP>0)
        {
			setExperience(0);
        }
    }
}

bool CvUnit::isFlamingArrows() const
{
	return m_bFlamingArrows;
}

void CvUnit::setFlamingArrows(bool bNewValue)
{
	m_bFlamingArrows = bNewValue;
}

int CvUnit::getGlobalYieldNeededForEquipmentBonus() const
{
	int iValue = 0;
	iValue += (int)sqrt((double)GC.getUnitInfo((UnitTypes)getUnitType()).getCombat());
	iValue += (int)sqrt((double)GC.getUnitInfo((UnitTypes)getUnitType()).getCombatDefense());
	iValue *= 6;
	return iValue;
}

int CvUnit::getEquipmentYieldType() const
{
	return m_iEquipmentYieldType;
}

void CvUnit::setEquipmentYieldType(int iYieldType)
{
	if(iYieldType != m_iEquipmentYieldType)
	{
		GET_PLAYER(getOwnerINLINE()).changeGlobalYieldNeededForEquipmentBonus(m_iEquipmentYieldType, -getGlobalYieldNeededForEquipmentBonus());
		m_iEquipmentYieldType = iYieldType;
		GET_PLAYER(getOwnerINLINE()).changeGlobalYieldNeededForEquipmentBonus(m_iEquipmentYieldType, getGlobalYieldNeededForEquipmentBonus());
	}
}

bool CvUnit::isGarrision() const
{
	return m_bGarrision;
}

void CvUnit::setGarrision(bool bNewValue)
{
	if(m_bGarrision != bNewValue)
	{
		GET_PLAYER(getOwnerINLINE()).changeNumGarrisionUnits((bNewValue) ? 1 : -1);
		GET_PLAYER(getOwnerINLINE()).changePower(getPower()*(bNewValue) ? 1 : -1);
		m_bGarrision = bNewValue;

		if(bNewValue)
		{
			getGroup()->pushMission(MISSION_FORTIFY);
			setAutomatedRangeStrike(false);
//			resolveEquipment();		//Garrisoned units should be able to have equipment.
		}
		else
		{
			getGroup()->setActivityType(ACTIVITY_AWAKE);
		}
		//modifies Support
		doCitySupport(bNewValue);
	}
}

bool CvUnit::canGarrision() const
{
	if(isGarrision())
	{
		return false;
	}

	if(!canDefend())
	{
		return false;
	}

	if(GC.getCivilizationInfo(getCivilizationType()).getDefaultUnit() != NO_UNIT)
		return false;

	if(plot()->getOwnerINLINE()!=getOwnerINLINE())
	{
		return false;
	}

	if(!plot()->isCity())
	{
		//SpyFanatic: allow units to garrison into forts
		if(!GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) || plot()->getImprovementType() == NO_IMPROVEMENT || GC.getImprovementInfo(plot()->getImprovementType()).getCulture() <= 0)
		{
			return false;
		}
		//return false;
	}

    CvUnit* pLoopUnit;
    CLLNode<IDInfo>* pUnitNode;
    pUnitNode = plot()->headUnitNode();
	int iCounter=0;
	int iMaxGarrison = GC.getDefineINT("NUM_GARRISION_UNITS_PER_CITY");
    while (pUnitNode != NULL)
    {
        pLoopUnit = ::getUnit(pUnitNode->m_data);
        pUnitNode = plot()->nextUnitNode(pUnitNode);
		if(pLoopUnit->getOwnerINLINE() == getOwnerINLINE())
		{
			if(pLoopUnit->isGarrision())
			{
				iCounter++;
				if(iCounter >= iMaxGarrison)
				{
					return false;
				}
			}
		}
	}

	return true;

}

int CvUnit::getFearEffect() const
{
	return m_iFearEffect;
}

void CvUnit::changeFearEffect(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iFearEffect += iNewValue;
    }
}

int CvUnit::getBonusVSUnitsFullHealth() const
{
	return m_iBonusVSUnitsFullHealth;
}

void CvUnit::changeBonusVSUnitsFullHealth(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iBonusVSUnitsFullHealth += iNewValue;
    }
}

int CvUnit::getSpellTargetIncrease() const
{
	return m_iSpellTargetIncrease;
}

void CvUnit::changeSpellTargetIncrease(int iNewValue)
{
    if (iNewValue != 0)
    {
        m_iSpellTargetIncrease += iNewValue;
    }
}

int CvUnit::getBonusSummonStrength() const
{
	return m_iBonusSummonStrength;
}

void CvUnit::changeBonusSummonStrength(int iChange)
{
	m_iBonusSummonStrength += iChange;
}

void CvUnit::updateBonusSummonStrength()
{
	//disabled
	return;

	if(!isSummon())
	{
		return;
	}

	ManaschoolTypes iSpellSchool=NO_MANASCHOOL;
	for(int iI=0;iI<GC.getNumProjectInfos();iI++)
	{
		if(GC.getProjectInfo((ProjectTypes)iI).getSummonUnitType()==getUnitType())
		{
			if(GC.getProjectInfo((ProjectTypes)iI).getTechPrereq()!=NO_TECH)
			{
				iSpellSchool=(ManaschoolTypes)GC.getTechInfo((TechTypes)GC.getProjectInfo((ProjectTypes)iI).getTechPrereq()).getSpellSchool();
			}
		}
	}

	if(iSpellSchool!=NO_MANASCHOOL)
	{
		BonusTypes iBonus=(BonusTypes)GC.getManaschoolInfo(iSpellSchool).getBonusSummonStrength();

		if(iBonus!=NO_BONUS)
		{
			int iNew=GET_PLAYER((PlayerTypes)getOwnerINLINE()).getNumAvailableBonuses(iBonus);
			iNew=std::min(iNew,2*GC.getUnitInfo(getUnitType()).getTier());
			int iOld=getBonusSummonStrength();

		    if (GC.getBonusInfo(iBonus).getDamageType() == NO_DAMAGE)
			{
				m_iBaseCombat += iNew - iOld;
				m_iBaseCombatDefense += iNew - iOld;
			}
			else
			{
				m_paiDamageTypeCombat[GC.getBonusInfo(iBonus).getDamageType()] += iNew - iOld;
				m_iTotalDamageTypeCombat += iNew - iOld;
			}
			changeBonusSummonStrength(iNew - iOld);
		}
	}
}

int CvUnit::getIgnoreFirstStrikes() const
{
	return m_iIgnoreFirstStrikes;
}

void CvUnit::changeIgnoreFirstStrikes(int iChange)
{
	m_iIgnoreFirstStrikes+=iChange;
}

int CvUnit::getCaptureAnimalChance() const
{
	return m_iCaptureAnimalChance;
}

int CvUnit::getCaptureAnimalChanceTotal() const
{ 
	if(getUnitCombatType() != GC.getInfoTypeForString("UNITCOMBAT_RECON"))
		return 0;
	TechTypes eHuntingGuild = (TechTypes)GC.getInfoTypeForString("TECH_HUNTERS_GUILD");
	if(eHuntingGuild != NO_TECH && GET_TEAM(getTeam()).isHasTech(eHuntingGuild))
		return 100;
	return 0;

	int iTotal=m_iCaptureAnimalChance;
	iTotal+=getUnitInfo().getTier()*20;
	return iTotal;
}

void CvUnit::changeCaptureAnimalChance(int iChange)
{
	m_iCaptureAnimalChance+=iChange;
}

int CvUnit::getCaptureBeastChance() const
{
	return m_iCaptureBeastChance;
}

int CvUnit::getCaptureBeastChanceTotal() const
{ 
	if(getUnitCombatType() != GC.getInfoTypeForString("UNITCOMBAT_RECON"))
		return 0;

	TechTypes eHuntingGuild = (TechTypes)GC.getInfoTypeForString("TECH_HUNTERS_GUILD");
	if(eHuntingGuild != NO_TECH && !GET_TEAM(getTeam()).isHasTech(eHuntingGuild))
		return 0;


	int iTotal=m_iCaptureBeastChance;
	iTotal+=(getUnitInfo().getTier()-2)*10;
	return iTotal;
}

void CvUnit::changeCaptureBeastChance(int iChange)
{
	m_iCaptureBeastChance+=iChange;
}

bool CvUnit::isEquipment()
{
	return (m_pUnitInfo->getEquipmentPromotion()!=NO_PROMOTION);
}

CvPlot* CvUnit::adjacentHigherDefensePlot(bool bNoEnemy, bool bRoute)
{
	CvPlot* pBestPlot=NULL;
	int iValue;
	int iBestValue=0;

	CvPlot* pMissionPlot=NULL;
	if(getAIGroup()!=NULL && getAIGroup()->getMissionTargetUnit()!=NULL)
	{
		pMissionPlot=getAIGroup()->getMissionTargetUnit()->plot();
	}

	int iCurrentValue=0;
	iCurrentValue+=getExtraTerrainDefensePercent(plot()->getTerrainType());
	iCurrentValue+=(plot()->getFeatureType()!=NO_FEATURE) ? getExtraFeatureDefensePercent(plot()->getFeatureType()) : 0;
	iCurrentValue+=(plot()->getImprovementType()!=NO_IMPROVEMENT) ? GC.getImprovementInfo(plot()->getImprovementType()).getDefenseModifier() : 0;

	for(int iPass=0; iPass<2; iPass++)
	{
		if(iPass>0 || pMissionPlot!=NULL)
		{
			for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));
				if(pAdjacentPlot!=NULL)
				{
					if(!bNoEnemy || !pAdjacentPlot->isVisibleEnemyDefender(this))
					{
						if(!bRoute || pAdjacentPlot->isRoute())
						{
							if(pMissionPlot==NULL || stepDistance(pAdjacentPlot->getX_INLINE(),pAdjacentPlot->getY_INLINE(),pMissionPlot->getX_INLINE(),pMissionPlot->getY_INLINE())>1)
							{
								iValue=(iPass==0) ? 100 : 0;
								iValue+=this->getExtraTerrainDefensePercent(pAdjacentPlot->getTerrainType());
								iValue+=(pAdjacentPlot->getFeatureType()!=NO_FEATURE) ? getExtraFeatureDefensePercent(pAdjacentPlot->getFeatureType()) : 0;
								iValue+=(pAdjacentPlot->getImprovementType()!=NO_IMPROVEMENT) ? GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getDefenseModifier() : 0;
								iValue+=(pAdjacentPlot->isCity()) ? 200 : 0;

								if(iValue>iBestValue)
								{
									iBestValue=iValue;
									pBestPlot=pAdjacentPlot;
								}
							}
						}
					}
				}
			}
		}
		if(iPass==0 && pBestPlot!=NULL)
		{
			return pBestPlot;
		}
	}
	return pBestPlot;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/*************************************************************************************************/

int CvUnit::getAirCombat() const
{
	return m_iAirCombat;
}
void CvUnit::setAirCombat(int iNewValue)
{
	m_iAirCombat = iNewValue;
}

void CvUnit::changeAirCombat(int iChange)
{
	setAirCombat(getAirCombat() + iChange);
}
int CvUnit::getAirCombatLimitBoost() const
{
	return m_iAirCombatLimitBoost;
}
void CvUnit::setAirCombatLimitBoost(int iNewValue)
{
	m_iAirCombatLimitBoost = iNewValue;
}
void CvUnit::changeAirCombatLimitBoost(int iChange)
{
	setAirCombatLimitBoost(getAirCombatLimitBoost() + iChange);
}

void CvUnit::setAutomatedRangeStrike(bool bNewValue)
{
    PromotionTypes iAutoProm=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_AUTORANGESTRIKE");

    if (iAutoProm!=NO_PROMOTION)
    {
        if (bNewValue==false)
            setHasPromotion(iAutoProm,false);
        else
            setHasPromotion(iAutoProm,true);
    }

    m_bAutomatedRangeStrike=bNewValue;
}

bool CvUnit::isAutomatedRangeStrike()
{
    return m_bAutomatedRangeStrike;
}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
//>>>>Refined Defensive Strikes: Added by Denev 2009/11/10
int CvUnit::getCombatDefensiveStrikes() const
{
	return m_iCombatDefensiveStrikes;
}

void CvUnit::setCombatDefensiveStrikes(int iNewValue)
{
	m_iCombatDefensiveStrikes = iNewValue;
}

bool CvUnit::isDefensiveStrikes() const
{
	//return m_iDefensiveStrikes == 0 ? false : true;
	if(getUnitCombatType() == NO_UNITCOMBAT)
		return false;
	else 
		return GC.getUnitCombatInfo(getUnitCombatType()).isArcherSupport();
}

void CvUnit::changeDefensiveStrikes(int iNewValue)
{
	if (iNewValue != 0)
	{
		m_iDefensiveStrikes += iNewValue;
	}
}

bool CvUnit::isEnemyBetweenUnits(const CvUnit* pUnit, const CvPlot* pOurPlot) const
{
	if (pUnit == NULL)
	{
		return false;
	}

	if (getTeam() == pUnit->getTeam())
	{
		return false;
	}

	if (isAlwaysHostile(pOurPlot) || pUnit->isAlwaysHostile(pUnit->plot()))
	{
		return true;
	}

	return atWar(getTeam(), pUnit->getTeam());
}

//<<<<Refined Defensive Strikes: End Add

/*************************************************************************************************/
/**	New Gameoption (InfluenceDrivenWar) merged Sephi                                         	**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/

//returns true if Unit can Influence Culture via battle
bool CvUnit::CanInfluenceWar()
{
	if (getDuration()>0)
	{
		return false;
	}
	if (isHiddenNationality())
	{
		return false;
	}
	if (isBarbarian())
	{
		return false;
	}
	if (isMechUnit() && (DOMAIN_SEA != getDomainType()))
	{
		return false;
	}

	return true;
}

//returns true if Unit Death can Influence Culture via battle
bool CvUnit::CanDeathInfluenceWar()
{
	if (getDuration()>0)
	{
		return false;
	}
	if (isHiddenNationality())
	{
		return false;
	}
	if (isBarbarian())
	{
		return false;
	}
	if (isAlive())
	{
		return true;
	}
	//not all non living units should influence Culture
	else
	{
		//Ships should influence Culture
		if ((isMechUnit() && (DOMAIN_SEA == getDomainType())))
		{
			return true;
		}
		//so should Golems
		PromotionTypes eGolem=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_GOLEM");
		if (eGolem!=NO_PROMOTION && isHasPromotion(eGolem))
		{
			return true;
		}
	}
	return true;
}

// unit influences combat area after victory
// returns influence % in defended plot
float CvUnit::doVictoryInfluence(CvUnit* pLoserUnit, bool bAttacking, bool bWithdrawal)
{
	if (GC.getDefineINT("IDW_NO_BARBARIAN_INFLUENCE"))
	{
		if (isBarbarian() || pLoserUnit->isBarbarian())
		{
			return 0.0f;
		}
	}
	if (GC.getDefineINT("IDW_NO_NAVAL_INFLUENCE"))
	{
		if (DOMAIN_SEA == getDomainType())
		{
			return 0.0f;
		}
	}

	CvPlot* pWinnerPlot = plot();
	CvPlot* pLoserPlot = pLoserUnit->plot();
	CvPlot* pDefenderPlot = NULL;
	if (!bAttacking)
	{
		pDefenderPlot = pWinnerPlot;
	}
	else
	{
		pDefenderPlot = pLoserPlot;
	}
	int iWinnerCultureBefore = pDefenderPlot->getCulture(getOwnerINLINE()); //used later for influence %

	float fWinnerPlotMultiplier = 1.0f; // by default: same influence in WinnerPlot and LoserPlot
	if (GC.getDefineFLOAT("IDW_WINNER_PLOT_MULTIPLIER"))
		fWinnerPlotMultiplier = GC.getDefineFLOAT("IDW_WINNER_PLOT_MULTIPLIER");

	float fLoserPlotMultiplier = 1.0f; // by default: same influence in WinnerPlot and LoserPlot
	if (GC.getDefineFLOAT("IDW_LOSER_PLOT_MULTIPLIER"))
		fLoserPlotMultiplier = GC.getDefineFLOAT("IDW_LOSER_PLOT_MULTIPLIER");

	float bWithdrawalMultiplier = 0.5f;
	if (bWithdrawal)
	{
		fWinnerPlotMultiplier *= bWithdrawalMultiplier;
		fLoserPlotMultiplier *= bWithdrawalMultiplier;
	}

	if (pLoserPlot->isEnemyCity(*this)) // city combat
	{
		if (pLoserPlot->getNumVisibleEnemyDefenders(this) > 1)
		{
			// if there are still some city defenders ->
			// we use same influence rules as for field combat
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier);
		}
		else // last defender is dead
		{
			float fNoCityDefenderMultiplier = 2.5; // default: 250%
			if (GC.getDefineFLOAT("IDW_NO_CITY_DEFENDER_MULTIPLIER"))
				fNoCityDefenderMultiplier = GC.getDefineFLOAT("IDW_NO_CITY_DEFENDER_MULTIPLIER");

			// last city defender is dead -> influence is increased
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier * fNoCityDefenderMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier * fNoCityDefenderMultiplier);
/**
			if (GC.getDefineINT("IDW_EMERGENCY_DRAFT_ENABLED"))
			{
				int iDefenderCulture = pLoserPlot->getCulture(pLoserUnit->getOwnerINLINE());
				int iAttackerCulture = pLoserPlot->getCulture(getOwnerINLINE());

				if (iDefenderCulture >= iAttackerCulture)
				{
					// if defender culture in city's central tile is still higher then atacker culture
					// -> city is not captured yet but emergency militia is drafted

					 pLoserPlot->getPlotCity()->emergencyConscript();

					// calculate city resistence % (to be displayed in game log)
					float fResistence = ((iDefenderCulture-iAttackerCulture)*100.0f)/(2*pDefenderPlot->countTotalCulture());

					CvWString szBuffer;
					szBuffer.Format(L"City militia has emerged! Resistance: %.1f%%", fResistence);
					gDLL->getInterfaceIFace()->addMessage(pLoserUnit->getOwnerINLINE(), false, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), pLoserPlot->getX_INLINE(), pLoserPlot->getY_INLINE(), true, true);
					gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pLoserPlot->getX_INLINE(), pLoserPlot->getY_INLINE());

				}
			}
**/
		}
	}
	else // field combat
	{
		if (!pLoserUnit->canDefend())
		{
			// no influence from worker capture
			return 0.0f;
		}

		if (pLoserPlot->getImprovementType() != NO_IMPROVEMENT
			&& GC.getImprovementInfo(pLoserPlot->getImprovementType()).getDefenseModifier() > 0
			&& pLoserPlot->getNumVisibleEnemyDefenders(this) > 1)
		{
			// fort captured
			float fFortCaptureMultiplier = 2.0f; // default: 200%
			if (GC.getDefineFLOAT("IDW_FORT_CAPTURE_MULTIPLIER"))
				fFortCaptureMultiplier = GC.getDefineFLOAT("IDW_FORT_CAPTURE_MULTIPLIER");

			// influence is increased
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier * fFortCaptureMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier * fFortCaptureMultiplier);

		}
		else
		{
			influencePlots(pLoserPlot, pLoserUnit->getOwnerINLINE(), fLoserPlotMultiplier);
			influencePlots(pWinnerPlot, pLoserUnit->getOwnerINLINE(), fWinnerPlotMultiplier);
		}
	}

	// calculate influence % in defended plot (to be displayed in game log)

	int iWinnerCultureAfter = pDefenderPlot->getCulture(getOwnerINLINE());
	int iTotalCulture = pDefenderPlot->countTotalCulture();
	float fInfluenceRatio = 0.0f;
	if (iTotalCulture > 0)
	{
		fInfluenceRatio = ((iWinnerCultureAfter-iWinnerCultureBefore)*100.0f)/iTotalCulture;
	}
    return fInfluenceRatio;
}

// unit influences given plot and surounding area i.e. transfers culture from target civ to unit's owner
void CvUnit::influencePlots(CvPlot* pCentralPlot, PlayerTypes eTargetPlayer, float fLocationMultiplier)
{
	float fBaseCombatInfluence = 4.0f;
	if (GC.getDefineFLOAT("IDW_BASE_COMBAT_INFLUENCE"))
        fBaseCombatInfluence = GC.getDefineFLOAT("IDW_BASE_COMBAT_INFLUENCE");

	// calculate base multiplier used for all plots
	float fGameSpeedMultiplier = (float) GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier = sqrt(fGameSpeedMultiplier);

	float fExperienceFactor = 0.01f;  // default: each point of experience increases influence by 1%
	if (GC.getDefineFLOAT("IDW_EXPERIENCE_FACTOR"))
        fExperienceFactor = GC.getDefineFLOAT("IDW_EXPERIENCE_FACTOR");
	float fExperienceMultiplier = 1.0f + (getExperience() * 0.005f);

	float fWarlordMultiplier = 1.0;
	if (NO_UNIT != getLeaderUnitType()) // warlord is here
	{
		fWarlordMultiplier = 1.5; // default: +50%
		if (GC.getDefineFLOAT("IDW_WARLORD_MULTIPLIER"))
			fWarlordMultiplier = GC.getDefineFLOAT("IDW_WARLORD_MULTIPLIER");
	}

	//Some Units have reduced Influence
	if (!isAlive())
	{
		//Ships should influence Culture
		if (DOMAIN_SEA != getDomainType())
		{
		    fWarlordMultiplier = 0.3f;
		}
	}
/*************************************************************************************************/
/**	ADDON (Promotion IDWPercent) Sephi						                     				**/
/*************************************************************************************************/
    for (int iI=0;iI<GC.getNumPromotionInfos();iI++)
    {
        if (isHasPromotion((PromotionTypes)iI) && GC.getPromotionInfo((PromotionTypes)iI).getIDWPercent() != 0)
        {
            fWarlordMultiplier *= 100+GC.getPromotionInfo((PromotionTypes)iI).getIDWPercent();
            fWarlordMultiplier /= 100;
        }
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


	float fBaseMultiplier = fBaseCombatInfluence * fGameSpeedMultiplier * fLocationMultiplier * fExperienceMultiplier * fWarlordMultiplier;
	if (fBaseMultiplier <= 0.0f)
		return;

	// get influence radius
	int iInfluenceRadius = 2; // default: like 2square city workable radius
	if (GC.getDefineINT("IDW_INFLUENCE_RADIUS"))
		iInfluenceRadius = GC.getDefineINT("IDW_INFLUENCE_RADIUS");
	if (iInfluenceRadius < 0)
		return;

	float fPlotDistanceFactor = 0.2f; // default: influence decreases by 20% with plot distance
	if (GC.getDefineFLOAT("IDW_PLOT_DISTANCE_FACTOR"))
        fPlotDistanceFactor = GC.getDefineFLOAT("IDW_PLOT_DISTANCE_FACTOR");

//	CvWString szBuffer;
//	szBuffer.Format(L"Factors: %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.3f, %d", fBaseCombatInfluence, fLocationMultiplier, fGameSpeedMultiplier, fPlotDistanceFactor, fExperienceMultiplier, fWarlordMultiplier, fBaseMultiplier, iInfluenceRadius);
//	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), pCentralPlot->getX_INLINE(), pCentralPlot->getY_INLINE());

	for (int iDX = -iInfluenceRadius; iDX <= iInfluenceRadius; iDX++)
	{
		for (int iDY = -iInfluenceRadius; iDY <= iInfluenceRadius; iDY++)
		{
			int iDistance = plotDistance(0, 0, iDX, iDY);

			if (iDistance <= iInfluenceRadius)
			{
				CvPlot* pLoopPlot = plotXY(pCentralPlot->getX_INLINE(), pCentralPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					//modified Sephi
					//need to do the culture transfer for every place who has some culture on the plot
					for (int i=0;i<MAX_CIV_PLAYERS;i++)
					{
						//Check only for Players not in our team
						if(GET_PLAYER((PlayerTypes)i).getTeam()!=getTeam())
						{
							// calculate distance multiplier for current plot
							float fDistanceMultiplier = 0.5f+0.5f*fPlotDistanceFactor-fPlotDistanceFactor*iDistance;
							if (fDistanceMultiplier <= 0.0f)
								continue;
							int iTargetCulture = pLoopPlot->getCulture((PlayerTypes)i);
							if (iTargetCulture <= 0)
								continue;
							if (iTargetCulture>pLoopPlot->getCulture(eTargetPlayer))
								continue;
							int iCultureTransfer = int (fBaseMultiplier * fDistanceMultiplier * sqrt((float) iTargetCulture));
							if (iTargetCulture < iCultureTransfer)
							{
								// cannot transfer more culture than remaining target culure
								iCultureTransfer = iTargetCulture;
							}
							if (iCultureTransfer == 0 && iTargetCulture > 0)
							{
								// always at least 1 point of culture must be transfered
								// othervise we may have the problems with capturing of very low culture cities.
								iCultureTransfer = 1;
							}

							if (iCultureTransfer > 0)
							{
								// target player's culture in plot is lowered
								pLoopPlot->changeCulture((PlayerTypes)i, -iCultureTransfer, false);
								// owners's culture in plot is raised
								pLoopPlot->changeCulture(getOwnerINLINE(), iCultureTransfer, true);
							}
						}
					}
				}
			}
		}
	}
}


// unit influences current tile via pillaging
// returns influence % in current plot
float CvUnit::doPillageInfluence()
{
    // only units which can influence war in battle can do so by pillaging
    if (!CanInfluenceWar())
    {
        return 0.0f;
    }

	if (isBarbarian() && GC.getDefineINT("IDW_NO_BARBARIAN_INFLUENCE"))
	{
		return 0.0f;
	}
	if ((DOMAIN_SEA == getDomainType()) && GC.getDefineINT("IDW_NO_NAVAL_INFLUENCE"))
	{
		return 0.0f;
	}

	CvPlot* pPlot = plot();
	if (pPlot == NULL)
	{
		//should not happen
		return 0.0f;
	}

	int iOurCultureBefore = pPlot->getCulture(getOwnerINLINE()); //used later for influence %

	float fBasePillageInfluence = 2.0f;
	if (GC.getDefineFLOAT("IDW_BASE_PILLAGE_INFLUENCE"))
        fBasePillageInfluence = GC.getDefineFLOAT("IDW_BASE_PILLAGE_INFLUENCE");

	float fGameSpeedMultiplier = (float) GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getConstructPercent();
	fGameSpeedMultiplier /= 100;
	fGameSpeedMultiplier = sqrt(fGameSpeedMultiplier);

	PlayerTypes eTargetPlayer = pPlot->getOwner();
	int iTargetCulture = pPlot->getCulture(eTargetPlayer);
	if (iTargetCulture <= 0)
	{
		//should not happen
		return 0.0f;
	}
	int iCultureTransfer = int (fBasePillageInfluence * fGameSpeedMultiplier * sqrt((float) iTargetCulture));
	if (iTargetCulture < iCultureTransfer)
	{
		// cannot transfer more culture than remaining target culure
		iCultureTransfer = iTargetCulture;
	}

	// target player's culture in plot is lowered
	pPlot->changeCulture(eTargetPlayer, -iCultureTransfer, false);
	// owners's culture in plot is raised
	pPlot->changeCulture(getOwnerINLINE(), iCultureTransfer, true);

	// calculate influence % in pillaged plot (to be displayed in game log)
    int iOurCultureAfter = pPlot->getCulture(getOwnerINLINE());
	float fInfluenceRatio = ((iOurCultureAfter-iOurCultureBefore)*100.0f)/pPlot->countTotalCulture();

//	CvWString szBuffer;
//	szBuffer.Format(L"Factors: %.1f, %.1f, %d, Result: %.3f, ", fGameSpeedMultiplier, fBasePillageInfluence, iTargetCulture, fInfluenceRatio);
//	gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getDefineINT("EVENT_MESSAGE_TIME"), szBuffer, "AS2D_UNIT_BUILD_UNIT", MESSAGE_TYPE_INFO, GC.getUnitInfo(getUnitType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), plot()->getX_INLINE(), plot()->getY_INLINE());

	return fInfluenceRatio;
}

/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

// Better Unit Power (Added by Skyre)
int CvUnit::getPower() const
{
	return m_iPower;
}

void CvUnit::updatePower()
{
	// Ignore non-combat units
	if (!canFight())
	{
		return;
	}

	long double iPower = getUnitInfo().getPowerValue();

//	iPower = std::max(iPower, getUnitInfo().getCombat() + getUnitInfo().getCombatDefense());
	iPower = std::max(getUnitInfo().getCombat(), getUnitInfo().getCombatDefense()) * 2;

	int iModifier = 100;

	// Calculate bonuses from affinities
	for (int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		int iAffinity = getBonusAffinity((BonusTypes)iI);

		if (iAffinity != 0)
		{
			// Counts double (attack and defense)
			iPower += iAffinity * 2 *
				GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses((BonusTypes)iI);
		}
	}

	// Calculate bonuses from promotions
	for (int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if (isHasPromotion((PromotionTypes)iI))
		{
			const CvPromotionInfo& kPromotion =
			   	GC.getPromotionInfo((PromotionTypes)iI);

			// Add configured XML value to power modifier
			iModifier += kPromotion.getPowerModifier();

			//Special case 1:
			//Any changes to attack, defense, moves or damage type combat are
			//added directly to the power rating

			iPower += kPromotion.getExtraCombatStr();
			iPower += kPromotion.getExtraCombatDefense();
			iPower += kPromotion.getMovesChange();
			iPower += kPromotion.getMoveDiscountChange();

			for (int iJ = 0; iJ < GC.getNumDamageTypeInfos(); iJ++)
			{
				// Counts double (attack and defense)
				iPower += kPromotion.getDamageTypeCombat(iJ) * 2;
			}

            //Special case 2:
			//This ability relies on current Armageddon Counter value

			int iCounterModifier = kPromotion.getCombatPercentGlobalCounter();

			if (iCounterModifier != 0)
			{
				iModifier += GC.getGameINLINE().getGlobalCounter() *
					iCounterModifier / 100;
			}
		}
	}

	// Number can get big here (see declaration of iPower above)
	iPower *= iModifier;
	//long double iPowerPrePowl = iPower;
	//iPower = powl(iPower,1.4); //SpyFanatic: Different results between wine and windows
	iPower = iPower * iPower;

	//long double iPowerPostPowl = iPower;
	//iPower /= 40; //SpyFanatic: Different results between wine and windows
	iPower = ((int)iPower)/10000;
	//long double iPowerPostPowl40 = iPower;


	int iMod=1+getExtraLives();
	if(isHasSecondChance())
	{
		iMod++;
	}

	iPower*=iMod;
	//long double iPowerPostPowlMod = iPower;

	// Calculated power value cannot be below zero
//	iPower = std::max(iPower, (long long)0);
    iPower = std::max(iPower, (long double)0);

	// Ignore Garrisons
	if (!isGarrision())
	{
		iPower*=3;
	}

	//if (isGarrision())
	//{
	//	iPower/=3; //SpyFanatic: Different results between wine and windows
	//}
//TODO: check if > MAX_INT ???

	/*if(isOOSLogging())
	{
		if (m_iPower != iPower)
		{
			oosLog("AIPower"
				,"Turn:%d,Player:%d,Unit:%d,m_iPower:%d,iPower[size %d]:%Lf,iPowerPrePowl:%Lf,iPowerPostPowl:%Lf,iPowerPostPowl40:%Lf,iPowerPostPowlMod:%Lf,iModifier:%d,isGarrision:%d,iMod:%d,floor_iPower:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwner()
				,getID()
				,m_iPower
				,sizeof(double)
				,iPower
				,iPowerPrePowl
				,iPowerPostPowl
				,iPowerPostPowl40
				,iPowerPostPowlMod
				,iModifier
				,isGarrision()
				,iMod
				,(int)std::floor(iPower)
			);
		}
	}*/
	// Update player and area power ratings to reflect new value
	if (m_iPower != iPower)
	{
		GET_PLAYER(getOwnerINLINE()).changePower(-m_iPower);
		area()->changePower(getOwnerINLINE(), -m_iPower);

		//m_iPower = (int)iPower;
		m_iPower = (int)std::floor(iPower);

		GET_PLAYER(getOwnerINLINE()).changePower(m_iPower);
		area()->changePower(getOwnerINLINE(), m_iPower);
	}
}
// End Better Unit Power

/*************************************************************************************************/
/**	ADDON (New Functions) Sephi																	**/
/*************************************************************************************************/
CvAIGroup* CvUnit::getAIGroup() const
{
	if(m_iAIGroupID==FFreeList::INVALID_INDEX)
	{
		return NULL;
	}
	return GET_PLAYER(getOwnerINLINE()).getAIGroup(m_iAIGroupID);
}

void CvUnit::setAIGroup(CvAIGroup* pNewGroup)
{
	CvAIGroup* pOldGroup=getAIGroup();

	if(pOldGroup!=pNewGroup)
	{
		if(pOldGroup!=NULL)
		{
			pOldGroup->removeUnit(this);
		}
		if(pNewGroup==NULL)
		{
			m_iAIGroupID=FFreeList::INVALID_INDEX;
		}
		else
		{
			m_iAIGroupID=pNewGroup->getID();
			pNewGroup->addUnit(this);
		}
		/*if(isOOSLogging() && m_iAIGroupID != -1)
		{
			oosLog("AIGroup"
				,"Turn:%d,PlayerID:%d,GroupID:%d,setAIGroup,UnitID:%d"
				,GC.getGameINLINE().getElapsedGameTurns()
				,getOwnerINLINE()
				,m_iAIGroupID
				,getID()
			);
		}*/
		if(!isDelayedDeath() && getGroup() && getGroup()->getNumUnits()>1)
		{
			joinGroup(NULL);
		}
	}
}

bool CvUnit::isSummon() const
{
	int iSummon=GC.getInfoTypeForString("PROMOTION_SUMMON");

	if (iSummon==-1 || !isHasPromotion((PromotionTypes)iSummon))
	{
		return false;
	}
	return true;
}

int CvUnit::getSpellCastingRange() const
{
	int iRange=GC.getUnitInfo(getUnitType()).getArcaneRange();
	iRange=std::max(1,iRange);

	return iRange;
}

bool CvUnit::canCastAt(SpellTypes iSpell, CvPlot* pTargetPlot)
{
	SpellTypes eSpell=(SpellTypes)iSpell;
	CvSpellInfo &kSpell=GC.getSpellInfo(iSpell);

	if(pTargetPlot==NULL)
	{
		pTargetPlot=plot();
	}

	if (plotDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pTargetPlot->getX_INLINE(), pTargetPlot->getY_INLINE()) > getSpellCastingRange())
	{
		return false;
	}

	if(kSpell.isTargetedHostile())
	{
		if(!pTargetPlot->isVisibleEnemyUnit(getOwnerINLINE()))
		{
			return false;
		}

		if(castDamage(eSpell,pTargetPlot,true)<=0)
		{
			return false;
		}
	}

	if(kSpell.getTerrainOrPrereq1()!=NO_TERRAIN)
	{
		if(pTargetPlot->getTerrainType()!=kSpell.getTerrainOrPrereq1())
		{
			if(kSpell.getTerrainOrPrereq2()==NO_TERRAIN || kSpell.getTerrainOrPrereq2()!=pTargetPlot->getTerrainType())
			{
				return false;
			}
		}
	}

	if(kSpell.getFeatureOrPrereq1()!=NO_TERRAIN)
	{
		if(pTargetPlot->getFeatureType()!=kSpell.getFeatureOrPrereq1())
		{
			if(kSpell.getFeatureOrPrereq2()==NO_TERRAIN || kSpell.getFeatureOrPrereq2()!=pTargetPlot->getFeatureType())
			{
				return false;
			}
		}
	}

	return canCast(iSpell,false);
}

bool CvUnit::canSpellTargetPlot(CvPlot* pTarget)
{
	if(pTarget==NULL)
		return false;

	if (!pTarget->isVisible(getTeam(), false))
	{
		return false;
	}

	int iRange=getSpellCastingRange();

	if (!plot()->canSeePlot(pTarget, getTeam(), iRange, getFacingDirection(true)))
	{
		return false;
	}

	if (plotDistance(getX_INLINE(), getY_INLINE(), pTarget->getX_INLINE(), pTarget->getY_INLINE()) > iRange)
	{
		return false;
	}

	return true;
}

/**	ADDON (Combatauras) Sephi																	**/
CvCombatAura* CvUnit::getCombatAura() const
{
	if(m_iCombatAuraID==FFreeList::INVALID_INDEX)
	{
		return NULL;
	}
	else
	{
		return GET_PLAYER(getOwnerINLINE()).getCombatAura(m_iCombatAuraID);
	}
}

void CvUnit::setCombatAura(CvCombatAura* pNewCombatAura)
{
	if (pNewCombatAura==NULL)
		m_iCombatAuraID=FFreeList::INVALID_INDEX;
	else
		m_iCombatAuraID=pNewCombatAura->getID();
}

int CvUnit::getCombatAuraType() const
{
	return m_iCombatAuraType;
}

void CvUnit::setCombatAuraType(int iNewValue)
{
	if(iNewValue!=m_iCombatAuraType)
	{
		m_iCombatAuraType=iNewValue;
		if(iNewValue==NO_COMBATAURA)
		{
			if(getCombatAura())
			{
				/*if(isOOSLogging())
				{
					oosLog(
						"CombatAura"
						,"Turn:%d,Player:%d,UnitID:%d,Kill:%d"
						,GC.getGameINLINE().getElapsedGameTurns()
						,getOwner()
						,getID()
						,getCombatAuraType()
					);
				}*/
				getCombatAura()->kill();
			}
		}
		else
		{
			setCombatAura(GET_PLAYER(getOwnerINLINE()).initCombatAura(getID(), m_iCombatAuraType));
		}
	}
}

CvCombatAura* CvUnit::getBlessing1() const
{
	if(m_iBlessing1 == FFreeList::INVALID_INDEX || m_iBlessing1Owner == NO_PLAYER)
	{
		return NULL;
	}
	else
	{
		return GET_PLAYER((PlayerTypes)m_iBlessing1Owner).getCombatAura(m_iBlessing1);
	}

}

void CvUnit::setBlessing1(CvCombatAura* pNewCombatAura)
{
	if (pNewCombatAura == NULL)
	{
		m_iBlessing1 = FFreeList::INVALID_INDEX;
		m_iBlessing1Owner = NO_PLAYER;
	}
	else
	{
		m_iBlessing1=pNewCombatAura->getID();
		m_iBlessing1Owner=pNewCombatAura->getOwnerINLINE();
	}
}

CvCombatAura* CvUnit::getBlessing2() const
{
	if(m_iBlessing2==FFreeList::INVALID_INDEX || m_iBlessing2Owner==NO_PLAYER)
	{
		return NULL;
	}
	else
	{
		return GET_PLAYER((PlayerTypes)m_iBlessing2Owner).getCombatAura(m_iBlessing2);
	}
}

void CvUnit::setBlessing2(CvCombatAura* pNewCombatAura)
{
	if (pNewCombatAura==NULL)
	{
		m_iBlessing2=FFreeList::INVALID_INDEX;
		m_iBlessing2Owner=NO_PLAYER;
	}
	else
	{
		m_iBlessing2=pNewCombatAura->getID();
		m_iBlessing2Owner=pNewCombatAura->getOwnerINLINE();
	}
}

CvCombatAura* CvUnit::getCurse1() const
{
	if(m_iCurse1==FFreeList::INVALID_INDEX || m_iCurse1Owner==NO_PLAYER)
	{
		return NULL;
	}
	else
	{
		return GET_PLAYER((PlayerTypes)m_iCurse1Owner).getCombatAura(m_iCurse1);
	}
}

void CvUnit::setCurse1(CvCombatAura* pNewCombatAura)
{
	if (pNewCombatAura==NULL)
	{
		m_iCurse1=FFreeList::INVALID_INDEX;
		m_iCurse1Owner=NO_PLAYER;
	}
	else
	{
		m_iCurse1=pNewCombatAura->getID();
		m_iCurse1Owner=pNewCombatAura->getOwnerINLINE();
	}
}

CvCombatAura* CvUnit::getCurse2() const
{
	if(m_iCurse2==FFreeList::INVALID_INDEX || m_iCurse2Owner==NO_PLAYER)
	{
		return NULL;
	}
	else
	{
		return GET_PLAYER((PlayerTypes)m_iCurse2Owner).getCombatAura(m_iCurse2);
	}
}

void CvUnit::setCurse2(CvCombatAura* pNewCombatAura)
{
	if (pNewCombatAura==NULL)
	{
		m_iCurse2=FFreeList::INVALID_INDEX;
		m_iCurse2Owner=NO_PLAYER;
	}
	else
	{
		m_iCurse2=pNewCombatAura->getID();
		m_iCurse2Owner=pNewCombatAura->getOwnerINLINE();
	}
}

bool CvUnit::canApplyCombatAura(CvCombatAura* pAura, bool bHigherTier)
{
	if(pAura==NULL)
	{
		return false;
	}

	if(pAura->isBlessing())
	{
		if(pAura->getUnit()->getTeam()!=getTeam())
		{
			return false;
		}
	}
	else
	{
		if(!GET_TEAM(pAura->getUnit()->getTeam()).isAtWar(getTeam()))
		{
			return false;
		}
	}

	if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPrereqPromotion()!=NO_PROMOTION)
	{
		if(!isHasPromotion((PromotionTypes)GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPrereqPromotion()))
		{
			return false;
		}
	}

	if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPrereqUnitClass()!=NO_UNITCLASS)
	{
		if(getUnitClassType()!=GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPrereqUnitClass())
		{
			return false;
		}
	}

	if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPrereqUnitCombat()!=NO_UNITCOMBAT)
	{
		if(getUnitCombatType()!=GC.getCombatAuraInfo(pAura->getCombatAuraType()).getPrereqUnitCombat())
		{
			return false;
		}
	}

	if(GC.getCombatAuraInfo(pAura->getCombatAuraType()).isPrereqAlive())
	{
		if(!isAlive())
		{
			return false;
		}
	}

	if(bHigherTier)
	{
		return true;
	}
	CvCombatAura* loopAura=NULL;
	for(int iI=0;iI<4;iI++)
	{
		if(iI==0)
		{
			loopAura=getBlessing1();
		}
		if(iI==1)
		{
			loopAura=getBlessing2();
		}
		if(iI==2)
		{
			loopAura=getCurse1();
		}
		if(iI==3)
		{
			loopAura=getCurse2();
		}

		if(loopAura!=NULL)
		{
			if(GC.getCombatAuraInfo(loopAura->getCombatAuraType()).getCombatAuraClassType()==GC.getCombatAuraInfo(pAura->getCombatAuraType()).getCombatAuraClassType())
			{
				return false;
			}
		}
	}

	return true;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

bool CvUnit::isInRange(CvPlot* pTarget,int iRange) const
{
	if(iRange<stepDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pTarget->getX_INLINE(), pTarget->getY_INLINE()))
	{
		return false;
	}

	return true;
}

void CvUnit::equip(PromotionTypes Equipment, bool bTreasure)
{
	if(!bTreasure 
		&& !GET_PLAYER(getOwnerINLINE()).canEquipUnit(this,Equipment))
	{
		return;
	}

	CvPromotionInfo& kPromotion=GC.getPromotionInfo(Equipment);

	if(!bTreasure)
	{
/*
		int iMod = (isHuman()) ? 100 : -100+GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAITrainPercent()*2;

		iMod -=GET_PLAYER(getOwnerINLINE()).getReducedEquipmentCost();
		iMod =std::max(iMod,20);
*/
		int iMod = GET_PLAYER(getOwnerINLINE()).getEquipmentCostModifier(Equipment);

		int iYieldCost = kPromotion.getYieldCost(GC.getUnitInfo(getUnitType()).getTier());
		//iYieldCost = (iYieldCost * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		iYieldCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();
		iYieldCost /= 100;
		if(kPromotion.getYieldCostType()!=NO_YIELD && iYieldCost>0)
		{
			if(Equipment == GC.getInfoTypeForString("PROMOTION_POTION_STRONG"))
				iYieldCost *= (getStrengthPotionsUsed() + 1);

			GET_PLAYER(getOwnerINLINE()).changeGlobalYield(kPromotion.getYieldCostType(),-(iYieldCost*iMod)/100);
		}

		//SpyFanatic: reduce cost also for gold, mana, faith
		int iGold =kPromotion.getGoldCost();
		iGold = (iGold * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		int iMana = kPromotion.getManaCost();
		iMana = (iMana * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;		
		int iFaith = kPromotion.getFaithCost();
		iFaith = (iFaith * GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent()) / 100;
		GET_PLAYER(getOwnerINLINE()).changeGold((-iGold*iMod)/100);
		GET_PLAYER(getOwnerINLINE()).changeMana((-iMana*iMod)/100);
		GET_PLAYER(getOwnerINLINE()).changeFaith((-iFaith*iMod)/100);
	}

	EquipmentCategoryTypes eCategory=(EquipmentCategoryTypes)kPromotion.getEquipmentCategory();
	for(int iI = 0; iI < GC.getNumPromotionInfos(); iI++)
	{
		if(isHasPromotion((PromotionTypes)iI))
		{
			if(GC.getPromotionInfo((PromotionTypes)iI).getEquipmentCategory()==eCategory)
			{
				if(iI != Equipment)
				{
					setHasPromotion((PromotionTypes)iI,false);
				}
			}
		}
	}
	setHasPromotion(Equipment,true);

	if(!bTreasure)
	{
		if(isHuman() && getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			gDLL->getInterfaceIFace()->selectUnit(this,false);
		}
	}
}

void CvUnit::exploreLair()
{
	if(plot()->getDungeonType()==NO_DUNGEON)
	{
		return;
	}

	CvDungeonInfo& kDungeon=GC.getDungeonInfo((DungeonTypes)plot()->getDungeonType());

	using namespace std;

	vector<int> PossibleResults;
	int iTotalWeight = 0;

	for(int iI =0 ;iI < GC.getNumLairResultInfos(); iI++)
	{
		if(kDungeon.isLairResult(iI))
		{
			bool bValid=true;
			if (!CvString(GC.getLairResultInfo((LairResultTypes)iI).getPyRequirement()).empty())
			{	
				CyUnit* pyUnit = new CyUnit(this);
				CyArgsList argsList;
				argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
				argsList.add(iI);
				argsList.add(GC.getLairResultInfo((LairResultTypes)iI).getValue());
				long lResult=0;
				gDLL->getPythonIFace()->callFunction(PYDungeonModule, "reqdungeon", argsList.makeFunctionArgs(), &lResult);
				delete pyUnit; // python fxn must not hold on to this pointer
				if (lResult == 0)
				{
					bValid=false;
				}
			}

			if(bValid)
			{
				PossibleResults.push_back(iI);
				iTotalWeight+=GC.getLairResultInfo((LairResultTypes)iI).getWeight();
			}
		}
	}
	
	int iPick=GC.getGameINLINE().getSorenRandNum(iTotalWeight,"Random Lairresult Pick");
	int iResult=NO_LAIRRESULT;
	int iCounter=0;
	int iPossibleResults=PossibleResults.size();
	for(int iI=0;iI<iPossibleResults;iI++)
	{
		iCounter+=GC.getLairResultInfo((LairResultTypes)PossibleResults[iI]).getWeight();
		if(iCounter>=iPick)
		{
			iResult=PossibleResults[iI];
			break;
		}
	}

	if(iResult != NO_LAIRRESULT)
	{
		CvLairResultInfo& kLairResult = GC.getLairResultInfo((LairResultTypes)iResult);

		int iValue=kLairResult.getValue();
		iValue*=100+(GC.getGameINLINE().getElapsedGameTurns()*kLairResult.getGameTurnMod())/GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getBarbPercent();
		iValue/=100;
		if (!CvString(kLairResult.getPyResult()).empty())
		{	
			CvWString szBuffer=gDLL->getText(kLairResult.getTextKeyWide(),iValue);
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(),true,GC.getEVENT_MESSAGE_TIME(),szBuffer,0,MESSAGE_TYPE_MINOR_EVENT,GC.getImprovementInfo((ImprovementTypes)plot()->getImprovementType()).getButton(),(ColorTypes)8,getX_INLINE(),getY_INLINE(),true,true);
			CyUnit* pyUnit = new CyUnit(this);
			CyArgsList argsList;
			argsList.add(gDLL->getPythonIFace()->makePythonObject(pyUnit));	// pass in unit class
			argsList.add(iResult);
			argsList.add(iValue);
			gDLL->getPythonIFace()->callFunction(PYDungeonModule, "dungeon", argsList.makeFunctionArgs()); //, &lResult
			delete pyUnit; // python fxn must not hold on to this pointer
		}

		if(!GC.getImprovementInfo((ImprovementTypes)plot()->getImprovementType()).isUnique())
		{
			plot()->setImprovementType(NO_IMPROVEMENT);
			plot()->destroyDungeon();
		}
	}
}

void CvUnit::doCitySupport(bool bRemove)
{
	if(GC.getUnitInfo(getUnitType()).isCitySupportNeeded())
	{
		GET_PLAYER(getOwnerINLINE()).changeUnitSupportUsed((bRemove) ? -1 : 1);
	}
}

bool CvUnit::canMobilize() const
{
	if(GC.getCivilizationInfo(getCivilizationType()).getDefaultUnit() != NO_UNIT)
		return false;

	if(GC.getUnitInfo(getUnitType()).isCitySupportNeeded())
	{
		if(!GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits())
		{
			return false;
		}
	}
	return (isGarrision() && GET_PLAYER(getOwnerINLINE()).getGold() >= 300);
}

bool CvUnit::isWarwizard() const
{
	return GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(getUnitType(),UNITCATEGORY_WARWIZARD);
}

bool CvUnit::isWarwizardinPosition(CvPlot* pTarget) const
{
	if(pTarget==NULL)
		return true;

	return (stepDistance(pTarget->getX_INLINE(),pTarget->getY_INLINE(),getX_INLINE(),getY_INLINE())<=getArcaneRange());
}
int CvUnit::getArcaneRange() const
{
	return GC.getUnitInfo(getUnitType()).getArcaneRange();
}

bool CvUnit::isHasSecondChance() const
{
	PromotionTypes eSecondChance=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_SECOND_CHANCE");

	if(eSecondChance==NO_PROMOTION || !isHasPromotion(eSecondChance))
	{
		return false;
	}

	return true;
}

//converts Equipment into Yields when unit dies
void CvUnit::resolveEquipment()
{
	PromotionTypes eEquipment;
	int iTier=GC.getUnitInfo(getUnitType()).getTier();

	for(int iI=0;iI<GC.getNumEquipmentCategoryInfos();iI++)
	{	
		eEquipment=getEquipment((EquipmentCategoryTypes)iI);
		if(eEquipment!=NO_PROMOTION)
		{
			int iCost = GC.getPromotionInfo(eEquipment).getYieldCost(iTier);
			iCost *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent() / 100;
			int eYield = GC.getPromotionInfo(eEquipment).getYieldCostType();

			if(eYield != NO_YIELD)
			{
				GET_PLAYER(getOwnerINLINE()).changeGlobalYield(eYield, iCost / 2);
			}
			setHasPromotion(eEquipment,false);
			setEquipment((EquipmentCategoryTypes)iI, NO_PROMOTION);
		}
	}
}

bool CvUnit::canAcquireDiscipline() const
{
	for(int iI=0;iI<GC.getNumPromotionInfos();iI++)
	{
		if(GC.getPromotionInfo((PromotionTypes)iI).isDiscipline())
		{
			if(canAcquirePromotion((PromotionTypes)iI))
			{
				return true;
			}
		}
	}
	return false;
}

void CvUnit::displayMessagePromotionGainedInBattle(PromotionTypes ePromotion) const
{
	CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_YOUR_UNIT_PROMOTED_IN_BATTLE", getNameKey(), GC.getPromotionInfo(ePromotion).getText());
	gDLL->getInterfaceIFace()->addMessage(
		getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer,
		GC.getPromotionInfo(ePromotion).getSound(), MESSAGE_TYPE_INFO, NULL,
		(ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), plot()->getX_INLINE(), plot()->getY_INLINE());
}

//A glorious Victory might be triggered after Battle
void CvUnit::doGloriousVictory(int iCombatOdds, CvUnit* pLoser)
{
	//TODO: check if a glorious Victory is triggered
	if(getExperience() > 20)
	{
		return;
	}

	int iMaximumOdds=GC.getDefineINT("COMBAT_DIE_SIDES")/4;
	if(pLoser!=NULL && pLoser->isHasSecondChance())
	{
		iMaximumOdds*=5;
		iMaximumOdds/=2;
	}
	//Odds to High
	if(iCombatOdds>iMaximumOdds)
	{
		return;
	}

	//Already have to many legends?
	CvPlayer& kPlayer =GET_PLAYER(getOwnerINLINE());
	int iLoop;
	int iCount=0;
	for(CvUnit* pLoopUnit=kPlayer.firstUnit(&iLoop);pLoopUnit!=NULL;pLoopUnit=kPlayer.nextUnit(&iLoop))
	{
		if(pLoopUnit->getExperience()>=50)
		{
			iCount++;
		}
	}
	if(kPlayer.getNumUnits()<iCount*10)
	{
		return;
	}

	//Experience
	changeExperience(50-getExperience());

	CvWString szBuffer = gDLL->getText("TXT_KEY_MISC_GLORIOUS_VICTORY", getNameKey());
	gDLL->getInterfaceIFace()->addMessage(
		getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer,
		"AS2D_WELOVEKING", MESSAGE_TYPE_INFO, NULL,
		(ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), plot()->getX_INLINE(), plot()->getY_INLINE());

	//Bonus Equipment
	for(int iI=0;iI<GC.getNumEquipmentCategoryInfos();iI++)
	{
		if(getEquipment((EquipmentCategoryTypes)iI)==NO_PROMOTION)
		{
			std::vector<int> aiPossibleEquipment;
			for(int iJ=0;iJ<GC.getNumPromotionInfos();iJ++)
			{
				if(kPlayer.canEquipUnit(this,(PromotionTypes)iJ,false,true))
				{
					aiPossibleEquipment.push_back(iJ);
				}
			}
			int iNumPicks=aiPossibleEquipment.size();
			if(iNumPicks>0)
			{
				int iPick=GC.getGameINLINE().getSorenRandNum(iNumPicks,"Glorious Victory");
				equip((PromotionTypes)aiPossibleEquipment[iPick],true);
				//TODO:add some code to display it
			}
		}
	}

	int iCulture = 100;	//Throw in a global culture bonus to a glorious victory
	iCulture *= GC.getWorldInfo(GC.getMapINLINE().getWorldSize()).getResearchPercent();
	iCulture /= 100;

	iCulture *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();
	iCulture /= 100;

	iCulture *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getResearchPercent();
	iCulture /= 100;

	iCulture *= std::max(0, ((GC.getDefineINT("TECH_COST_EXTRA_TEAM_MEMBER_MODIFIER") * (GET_TEAM(getTeam()).getNumMembers() - 1)) + 100));
	iCulture /= 100;
	kPlayer.changeGlobalCulture(iCulture);
}

// return how much Support Bonus a Unit gets from an Archer Unit on the Plot
CvUnit* CvUnit::getDefensiveStrikeUnit(int &iBonus, const CvUnit* pTarget) const
{
	CvUnit* pBestUnit=NULL;
	int iBestValue=0;
	int iValue;

	//Loop through all units on same plot
    CLLNode<IDInfo>* pUnitNode;
    CvUnit* pLoopUnit;
    pUnitNode = plot()->headUnitNode();
    while (pUnitNode != NULL)
    {
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = plot()->nextUnitNode(pUnitNode);
		if(pLoopUnit != this) 		{
			if(pLoopUnit->getOwnerINLINE() == getOwnerINLINE()) {
				if(pLoopUnit->isDefensiveStrikes()) {
					iValue = pLoopUnit->getDefensiveStrikeBonus(pTarget);
					if(iValue > iBestValue) {
						iBestValue = iValue;
						pBestUnit = pLoopUnit;
					}
				}
			}
		}
	}

	iBonus = iBestValue;

	return pBestUnit;
}

//returns Bonus a Unit Offers as Ranged Support against pTarget
int CvUnit::getDefensiveStrikeBonus(const CvUnit* pTarget) const
{
	if(pTarget==NULL)
	{
		return 0;
	}

	if(pTarget->isImmuneToDefensiveStrike() || pTarget->baseCombatStrDefense() == 0)
	{
		return 0;
	}

	if(!hasDefensiveStrikesLeft())
	{
		return 0;
	}

	int iBonus = 10 + getAirCombatLimitBoost();
	iBonus *= baseCombatStr();
	iBonus /= pTarget->baseCombatStrDefense();

	if(pTarget->plot()->getFeatureType() != NO_FEATURE || pTarget->plot()->isCity())
		iBonus /= 2;

	return range(iBonus, 10, 30);
}

bool CvUnit::hasDefensiveStrikesLeft() const
{
	return m_iDefensiveStrikesLeft > 0;
}

void CvUnit::changeDefensiveStrikesLeft(int iChange)
{
	m_iDefensiveStrikesLeft += iChange;
}

void CvUnit::setDefensiveStrikesLeft(int iNewValue)
{
	m_iDefensiveStrikesLeft = iNewValue;
}

void CvUnit::doAnimalTurnedToFood(UnitTypes eAnimal)
{
	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
	CvWString szBuffer;

	if(kPlayer.isIgnoreFood() || kPlayer.getNumCities() == 0)
		return;

	int iFood = (GC.getUnitInfo(eAnimal).getCombat() * 5);	// 100 + GC.getGame().getElapsedGameTurns();
	int iFoodTaken = kPlayer.addFoodToCities(iFood);

	if(iFoodTaken > 0) {
		szBuffer = gDLL->getText("TXT_KEY_MISC_ANIMAL_TURNED_TO_FOOD", iFoodTaken, GAMETEXT.getLinkedText((UnitTypes)eAnimal, true, false).c_str());
		gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), true, GC.getEVENT_MESSAGE_TIME(), szBuffer, 0, MESSAGE_TYPE_MAJOR_EVENT, GC.getUnitInfo(eAnimal).getButton(), NO_COLOR, getX_INLINE(), getY_INLINE());
	}
}

int CvUnit::getThiefMissionSuccessChance(SpellTypes eSpell) const
{
	int iChance = 50 + getLevel() * 5 - GC.getSpellInfo(eSpell).getThiefDifficulty();

	if(plot()->isCity()) {
		iChance += plot()->getPlotCity()->getCrime();
		iChance += plot()->getPlotCity()->getPopulation() * 3;

		if(plot()->getPlotCity()->isCapital())
			iChance -= 50;
	}

	iChance = range(iChance, 0, 100);

	return iChance;
}

int CvUnit::getThiefMissionArrestedChance(SpellTypes eSpell) const
{
	int iChance = 50 - getLevel() * 5 + GC.getSpellInfo(eSpell).getThiefDifficulty();

	iChance += plot()->getNumUnits() * 5;

	if(plot()->isCity()) {
		iChance -= plot()->getPlotCity()->getCrime();
		iChance -= plot()->getPlotCity()->getPopulation() * 3;
	}

	iChance = range(iChance, 0, 100);

	return iChance;
}

bool CvUnit::tryThiefMission(SpellTypes eSpell)
{
	return getThiefMissionSuccessChance(eSpell) > GC.getGame().getSorenRandNum(100, "Try Thief Mission");
}

bool CvUnit::tryThiefArrested(SpellTypes eSpell)
{
	return getThiefMissionArrestedChance(eSpell) > GC.getGame().getSorenRandNum(100, "Try Thief Mission");
}

void CvUnit::updateBaseCombatStrength()
{
	if(GC.getUnitInfo(getUnitType()).isScaleWithTech()) {
		m_iBaseCombat = GC.getUnitInfo(getUnitType()).getCombat()
						+ GET_PLAYER(getOwnerINLINE()).getIncreaseStrengthOfRegularUnits();

		m_iBaseCombatDefense = GC.getUnitInfo(getUnitType()).getCombatDefense()
						+ GET_PLAYER(getOwnerINLINE()).getIncreaseStrengthOfRegularUnits();
	}
}

/*************************************************************************************************/
/**	ADDON (Firewall) Sephi                														**/
/*************************************************************************************************/
int CvUnit::getWallOfFireBonus(const CvUnit* pAttacker) const
{
	if(!plot()->isCity() || pAttacker==NULL)
	{
		return 0;
	}

    int iBuilding=GC.getInfoTypeForString("BUILDING_WALL_OF_FIRE");
    if (iBuilding==NO_BUILDING || plot()->getPlotCity()->getNumBuilding((BuildingTypes)iBuilding)==0)
    {
		return 0;
	}

	DamageTypes eFire=(DamageTypes)GC.getInfoTypeForString("DAMAGE_FIRE");
    if(eFire!=NO_DAMAGE)
    {
		return std::max(0,20-pAttacker->getDamageTypeResist(eFire));
    }

	return 0;
}
/*************************************************************************************************/
/**	ADDON   								END													**/
/*************************************************************************************************/
int CvUnit::getTier() const {

	return getUnitInfo().getTier();
}
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
int CvUnit::getViewportX() const
{
	CvViewport&	pCurrentViewPort = GC.getCurrentViewport();

	return pCurrentViewPort.getViewportXFromMapX(m_iX);
}


int CvUnit::getViewportY() const
{
	CvViewport&	pCurrentViewPort = GC.getCurrentViewport();

	return pCurrentViewPort.getViewportYFromMapY(m_iY);
}

bool CvUnit::isInViewport(void) const
{
	return GC.getCurrentViewport().isInViewport(m_iX, m_iY);
}
**/
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/



/**
void CvUnit::createUnitAfterBattle(CvUnit* pLoser, UnitTypes eNewUnit) const
{
	if(!GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits())
	{
		return;
	}

	if (pLoser->isImmuneToCapture() || GC.getUnitInfo((UnitTypes)pLoser->getUnitType()).getEquipmentPromotion() == NO_PROMOTION)
	{
		return;
	}

	pUnit = GET_PLAYER(getOwnerINLINE()).initUnit((UnitTypes)eNewUnit, plot()->getX_INLINE(), plot()->getY_INLINE());

	if (isHiddenNationality())
	{
		pUnit->setHasPromotion((PromotionTypes)GC.getDefineINT("HIDDEN_NATIONALITY_PROMOTION"), true);
	}
}
**/
