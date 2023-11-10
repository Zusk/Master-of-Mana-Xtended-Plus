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

#include "CyPlot.h"

//updates BarbarianPower and BarbarianPoints every turn
void CvPlayer::updateBarbarianPower()
{
	int iChange = getNumCities() * 2;

	for(int iI = 0; iI < GC.getMapINLINE().numPlots(); ++iI) {
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(iI);

		if(pLoopPlot->getImprovementType() != NO_IMPROVEMENT) {
			CvImprovementInfo &kImprovement = GC.getImprovementInfo(pLoopPlot->getImprovementType());
			if(kImprovement.getSpawnUnitCiv() == getCivilizationType()) {
				++iChange;
			}
		}
	}

	//modify iChange by Worldsize
	iChange *= 10000;
	iChange /= GC.getMapINLINE().numPlots();

	if(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)) {
		iChange *= 3;
		iChange /= 2;
	}

	changeBarbarianPower(iChange);
	changeBarbarianPoints(getBarbarianPower());
}

void CvPlayer::barb_spawn_group_new_lair()
{
	int iIndex;
	UnitTypes GuardianUnitType = (UnitTypes)GC.getInfoTypeForString("UNIT_ARCHER");

	if(getBarbarianPower() > 2000)
		GuardianUnitType = (UnitTypes)GC.getInfoTypeForString("UNIT_OGRE");

	CvCity* pBarbCity = firstCity(&iIndex);

	if(pBarbCity != NULL
		&& GuardianUnitType != NO_UNIT) {

		CvAIGroup* pSpawnGroup;
		pSpawnGroup = initAIGroup(AIGROUP_BARB_NEW_LAIR);
		pSpawnGroup->setMissionCity(pBarbCity);

		for(int iI = 0; iI < 2; iI++)
		{
			CvUnit* pNewUnit = initUnit(GuardianUnitType, pBarbCity->getX_INLINE(), pBarbCity->getY_INLINE());
			pNewUnit->setOriginPlot(pBarbCity->plot());
			pNewUnit->setAIGroup(pSpawnGroup);
		}		
	}
}

void CvPlayer::barb_spawn_plunder_group_in_city()
{
	int iIndex;
	UnitTypes MeleeUnitType = (UnitTypes)GC.getInfoTypeForString("UNIT_WARRIOR");

	if(getBarbarianPower()>500)
		MeleeUnitType = (UnitTypes)GC.getInfoTypeForString("UNIT_AXEMAN");
	if(getBarbarianPower()>2000) {
		MeleeUnitType = (UnitTypes)GC.getInfoTypeForString("UNIT_OGRE");
	}

	CvCity* pBarbCity = firstCity(&iIndex);

	if(pBarbCity != NULL
		&& MeleeUnitType != NO_UNIT) {

		CvAIGroup* pSpawnGroup;
		pSpawnGroup = initAIGroup(AIGROUP_BARB_PLUNDER);
		pSpawnGroup->setMissionCity(pBarbCity);

		for(int iI = 0; iI < 4; iI++)
		{
			CvUnit* pNewUnit = initUnit(MeleeUnitType, pBarbCity->getX_INLINE(), pBarbCity->getY_INLINE());
			pNewUnit->setOriginPlot(pBarbCity->plot());
			pNewUnit->setAIGroup(pSpawnGroup);
		}		
	}
}

// Barbarianfactions use their hoarded Points
void CvPlayer::spendBarbarianPoints()
{
	if(getBarbarianPoints() < 1500)
		return;

	if(getBarbarianPower()<500) {
		while(getBarbarianPoints() > 1500) {
			// Barbarians may get a plunder group or a create new lair group

			int iAction = GC.getGameINLINE().getSorenRandNum(2, "spendBarbarianPoints");

			switch(iAction)
			{
				case 0:
					barb_spawn_plunder_group_in_city();
					break;
				case 1:
					barb_spawn_group_new_lair();
					break;
/**
				case 2:
					barb_spawn_group_on_random_lair();
					break;
**/
				default:
					FAssertMsg(false, "spendBarbarianPoints()");
				break;
			}

			//TODO
			changeBarbarianPoints(-1500);
		}
	}
}

void CvPlayer::createInitialLairs()
{
	/**
	TODO Create Lairs close to Barbarian Cities
	**/
	return;


	int iNumInitialLairs = GC.getGame().getSorenRandNum(4, "initial Barbarian Lairs") + 1;

	iNumInitialLairs *= 100 + 20 * GC.getMapINLINE().getWorldSize();
	iNumInitialLairs /= 100;

	std::vector<int> possibleTiles;

	for(int i = 0; i < GC.getMapINLINE().numPlots(); ++i) {
		CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(i);
		if(pLoopPlot->isValidLairLocation()) {
			//add check for distance to Barb Cities
			possibleTiles.push_back(i);
		}
	}

	if(possibleTiles.size() > 0) {
		for(int i = 0; i < iNumInitialLairs; ++i) {
			int iLair = GC.getGame().getSorenRandNum(possibleTiles.size(), "Create Initial Lairs");
			GC.getMapINLINE().plotByIndex(possibleTiles[iLair])->CreateLair(getCivilizationType());
		}
	}
}

void CvPlayer::doTurn_BarbPlayer()
{
	if(GC.getGame().getElapsedGameTurns() == 0) {
		init_BarbPlayer();
	}

	updateBarbarianPower();
	spendBarbarianPoints();
}

void CvPlayer::init_BarbPlayer()
{
	int Index;
	UnitTypes eUnit;

	switch(getID()) {
		case BARBARIAN_PLAYER: 
			GC.getGameINLINE().createBarbarianCities();
			//Time to Create Orthus
			eUnit = (UnitTypes)GC.getInfoTypeForString("UNIT_ORTHUS");
			if(eUnit != NO_UNIT && getNumCities() > 0) {
				FAssert(getNumCities() > 0);
				CvCity* pCity = firstCity(&Index);
				initUnit(eUnit, pCity->getX(), pCity->getY());
			}			
			break;
		default:
			break;
	}

	createInitialLairs();
}

void CvPlot::doLairTurn()
{
/*************************************************************************************************/
/**	ADDON (multibarb) merged Sephi		                                                    	**/
/**	adjusted Spawning from improvements for more barb teams 									**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
	/**
	int iDevilGate=GC.getInfoTypeForString("IMPROVEMENT_DEVIL_PORTAL");
	if(iDevilGate!=NO_IMPROVEMENT && getImprovementType() == iDevilGate)
	{
		doDevilGateSpawn();
	}
	**/

	if(getDungeon() != NULL) {
		getDungeon()->doTurn();
	}

	//doLairDefenderSpawn();
	//doLairSpawnGroup();
}

void CvPlot::doLairDefenderSpawn()
{
	PlayerTypes eOwner = NO_PLAYER;
	if (getImprovementType() != NO_IMPROVEMENT) {

        int iUnit = GC.getImprovementInfo(getImprovementType()).getSpawnUnitType();
        int iCiv = GC.getImprovementInfo(getImprovementType()).getSpawnUnitCiv();

        if (iUnit != NO_UNIT) {

            if (iCiv == GC.getDefineINT("BARBARIAN_CIVILIZATION"))
				eOwner = BARBARIAN_PLAYER;

			if(eOwner != NO_PLAYER)	{
				int iBarbsNeeded = 2;
				int iBarbsAround = getNumUnits();

                if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)) {
                    iBarbsNeeded *= 3;
                    iBarbsNeeded /= 2;
                }

				if(iBarbsAround < iBarbsNeeded) {
					int iChance = 10;

                    if (GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)) {
                        iChance *= 3;
                        iChance /= 2;
                    }

					iChance *= 100;
					iChance /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getTrainPercent();

					if (GC.getGameINLINE().getSorenRandNum(10000, "Spawn Unit") < iChance) {
						if (!isVisibleOtherUnit(BARBARIAN_PLAYER)) {
							SpawnBarbarianUnit((UnitTypes)iUnit, BARBARIAN_PLAYER, true, UNITAI_ATTACK);
						}
					}
				} // if(iBarbsAround < iBarbsNeeded)
            }  // if (iCiv == ...
		}  // iUnit != -1 
	}  // Improvement != -1
}

void CvPlot::doLairSpawnGroup()
{
	if(getImprovementType() == NO_IMPROVEMENT || GC.getImprovementInfo(getImprovementType()).getSpawnGroup() == NO_AIGROUP)
	{
		return;
	}

	if(GC.getGameINLINE().getElapsedGameTurns() < 30) {
		return;
	}

	int iSpawnChance = GC.getDefineINT("SPAWNGROUP_CHANCE") * ((GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)) ? 2 : 1);

	if(GC.getGameINLINE().getSorenRandNum(1000, "create Spawngroup") > iSpawnChance)
	{
		return;
	}

	//does plot Already have a SpawnGroup?
	CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)GC.getBARBARIAN_PLAYER());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getHeadUnit() != NULL)
		{
			if(pAIGroup->getHeadUnit()->getOriginPlot() == this)
			{
				//there is already a Spawngroup from this improvement
				return;
			}
		}
	}

	CvPlot* pTarget = NULL;
	int iSearchRange = 5;
	for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
	{
		for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
		{
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
			if(pLoopPlot!=NULL)
			{
				if(pLoopPlot->getArea() == getArea())
				{
					//TODO: Allow Barbarians to target AI as well
					if(!pLoopPlot->isBarbarian() && pLoopPlot->isCity() && GET_PLAYER(pLoopPlot->getOwnerINLINE()).isHuman())
					{
						if(pLoopPlot->isOwned() && GET_TEAM(pLoopPlot->getTeam()).isAtWar((TeamTypes)GC.getBARBARIAN_TEAM()))
						{
							pTarget = pLoopPlot;
							break;
						}
					}
				}
			}
		}
	}

	if(pTarget == NULL)
		return;

	UnitTypes eUnit1 = NO_UNIT;
	int iNumUnit1 = 0;
	UnitTypes eUnit2 = NO_UNIT;
	int iNumUnit2 = 0;
	switch(GC.getImprovementInfo(getImprovementType()).getSpawnGroup())
	{
		case AIGROUP_BARB_GOBLINS:
			eUnit1 = (UnitTypes)GC.getInfoTypeForString("UNIT_ARCHER_SCORPION_CLAN");
			iNumUnit1 = 1;
			eUnit2 = (UnitTypes)GC.getInfoTypeForString("UNIT_GOBLIN");
			iNumUnit2 = 1;
			break;
		case AIGROUP_BARB_ORCS:
			eUnit1 = (UnitTypes)GC.getInfoTypeForString("UNIT_AXEMAN");
			iNumUnit1 = 1;
			eUnit2 = (UnitTypes)GC.getInfoTypeForString("UNIT_WOLF_RIDER");
			iNumUnit2 = 1;
			break;
		case AIGROUP_BARB_SKELETONS:
			eUnit1 = (UnitTypes)GC.getInfoTypeForString("UNIT_SKELETON");
			iNumUnit1 = 2;
			eUnit2 = (UnitTypes)GC.getInfoTypeForString("UNIT_NECROMANCER");
			iNumUnit2 = 1;
			break;
		default:
			break;
	}

	if(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		iNumUnit1 *= 2;
		iNumUnit2 *= 2;
	}

	CvAIGroup* pSpawnGroup;
	if(eUnit1 != NO_UNIT)
	{
		pSpawnGroup = kPlayer.initAIGroup(GC.getImprovementInfo(getImprovementType()).getSpawnGroup());
		pSpawnGroup->setMissionPlot(pTarget);
		for(int iI=0;iI<iNumUnit1;iI++)
		{
			CvUnit* pNewUnit = kPlayer.initUnit(eUnit1,getX_INLINE(),getY_INLINE());
			pNewUnit->setOriginPlot(this);
			pNewUnit->setAIGroup(pSpawnGroup);
		}

		if(eUnit2 != NO_UNIT)
		{
			for(int iI = 0; iI < iNumUnit2; iI++)
			{
				CvUnit* pNewUnit = kPlayer.initUnit(eUnit2,getX_INLINE(),getY_INLINE());
				pNewUnit->setOriginPlot(this);
				pNewUnit->setAIGroup(pSpawnGroup);
			}
		}
	}
}

void CvAIGroup::update_barb_new_lair()
{
	CvPlot* pMission = getMissionPlot();
	CvPlot* pLoopPlot;
	int iDX, iDY;

	if(pMission != NULL)
		if(!pMission->isValidLairLocation())
			pMission = NULL;

	if(pMission == NULL && getMissionCity() != NULL) {
		//pick new Mission Plot
		for(int iPass = 1; iPass < 4; ++iPass) {
			int iSearchRange = iPass * 10;
			int iBestValue = MAX_INT;
			CvPlot* pBestPlot = NULL;

			for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)	{
				for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++) {
					pLoopPlot = plotXY(getMissionCity()->getX_INLINE(), getMissionCity()->getY_INLINE(), iDX, iDY);

					if(pLoopPlot != NULL && pLoopPlot->isValidLairLocation()) {
						if(std::abs(iDX) + std::abs(iDY) < iBestValue) {
							pBestPlot = pLoopPlot;
							iBestValue = std::abs(iDX) + std::abs(iDY);
						}
					}
				}
			}

			if(pBestPlot != NULL) {
				pMission = pBestPlot;
				setMissionPlot(pMission);
				break;
			}
		} // for iPass
	}

	if(pMission == NULL) {
		setDeathDelayed(true);
		return;
	}

	bool bSuccess = false;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode)) {
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->plot() == pMission) {
			pMission->CreateLair(GET_PLAYER(getOwnerINLINE()).getCivilizationType());
			bSuccess = true;
		}
	}

	if(bSuccess) {
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode)) {
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pLoopUnit->kill(true);
		}
		setDeathDelayed(true);
		return;
	}

	Move(pMission);
}

bool CvAIGroup::isValidPlotToPlunder(CvPlot* pPlot) const
{
	if(pPlot==NULL)
		return false;

	if(pPlot->isCity())
		return false;

	if(pPlot->getImprovementType() == NO_IMPROVEMENT 
		|| GC.getImprovementInfo(pPlot->getImprovementType()).isPermanent())
		return false;

	if(!pPlot->isOwned() || GET_TEAM(getTeam()).isAtWar(pPlot->getTeam()))
		return false;

	return true;
}

void CvAIGroup::update_barb_plunder()
{
	//is group still alive?
	CvCity* pCity = getMissionCity();
	if(getNumUnits() == 0 || pCity == NULL) {
		setDeathDelayed(true);
		return;
	}

	CvPlot* pMission = getMissionPlot();
	CvPlot* pLoopPlot;
	int iDX, iDY;

	//TODO
	//see if there is a vulnerable City nearby that we can easily attack

	if(m_iBarbPlunder < 1000) {

		if(!isValidPlotToPlunder(pMission))
			pMission = NULL;
		
		if(pMission == NULL) {
			//pick new Mission Plot
			for(int iPass = 1; iPass < 4; ++iPass) {
				int iSearchRange = iPass * 10;
				int iBestValue = MAX_INT;
				CvPlot* pBestPlot = NULL;

				for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)	{
					for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++) {
						pLoopPlot	= plotXY(getMissionCity()->getX_INLINE(), getMissionCity()->getY_INLINE(), iDX, iDY);

						if(isValidPlotToPlunder(pLoopPlot)) {
							if(std::abs(iDX) + std::abs(iDY) < iBestValue) {
								pBestPlot = pLoopPlot;
								iBestValue = std::abs(iDX) + std::abs(iDY);
							}
						}
					}
				}

				if(pBestPlot != NULL) {
					pMission = pBestPlot;
					setMissionPlot(pMission);
					break;
				}
			} // for iPass
		}

		//couldn't find new Mission Plot
		if(pMission == NULL) {
			setDeathDelayed(true);
			return;
		}

		//move units to Mission Plot and pillage it (yeah)
		//increase Plundervalue when we pillage
		Move(pMission);
		int iCounter = pillagePlot(pMission);
		if(iCounter > 0) {
			m_iBarbPlunder += 200 * iCounter;
		}
	}
	else {
		Move(pCity->plot());
		if(atPlot(pCity->plot())) {
			GET_PLAYER(getOwnerINLINE()).changeBarbarianPower(m_iBarbPlunder / 10);
			m_iBarbPlunder = 0;
		}
	}
}

void CvAIGroup::update_barb_goblins()
{
	CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)GC.getBARBARIAN_PLAYER());
	bool bRetreat=false;
	int iValue,iBestValue=0;
	int iPathTurns;
	CvPlot* pBestPlot=NULL;

	if(getNumUnits() == 0)
	{
		setDeathDelayed(true);
		return;
	}

	//attack adjacent enemies only at very good odds
	attackAnyUnit(25);

	//retreat if in Danger
	if(getHeadUnit() != NULL && kPlayer.AI_getPlotDanger(getHeadUnit()->plot(),1) > 2)
	{
		bRetreat = true;
	}

	if(getMissionPlot() == NULL || (!getMissionPlot()->isCity()))
	{
		bRetreat = true;
	}

	if(bRetreat)
	{
		if(getHeadUnit()->generatePath(getHeadUnit()->getOriginPlot(),0,true))
		{
			Move(getHeadUnit()->getPathEndTurnPlot());
			return;
		}
		return;
	}

	CvCity* pTargetCity = getMissionPlot()->getPlotCity();

	for (int iI = 0; iI < pTargetCity->getNumCityPlots(); iI++)
	{
		CvPlot* pLoopPlot = pTargetCity->getCityIndexPlot(iI);
		if (pLoopPlot != NULL)
		{
			if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT 
				&& GC.getImprovementInfo(pLoopPlot->getImprovementType()).getImprovementClassType()==0
				&& !GC.getImprovementInfo(pLoopPlot->getImprovementType()).isPermanent())
			{
				int iDanger=kPlayer.AI_getPlotDanger(pLoopPlot,1);
				if(iDanger<3)
				{
					if(getHeadUnit()->generatePath(pLoopPlot,0,false,&iPathTurns))
					{
						iValue=100/(iPathTurns+2);
						if(iValue>iBestValue)
						{
							iBestValue=iValue;
							pBestPlot=pLoopPlot;
						}
					}
				}
			}
		}
	}

	if(pBestPlot!=NULL && !pBestPlot->isVisibleEnemyUnit((PlayerTypes)GC.getBARBARIAN_PLAYER()))
	{
		Move(pBestPlot);
		pillage();
	}
	return;
}

void CvAIGroup::update_barb_orcs()
{
	CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)GC.getBARBARIAN_PLAYER());
	CvPlot* pBestPlot=NULL;

	if(getNumUnits()==0)
	{
		setDeathDelayed(true);
		return;
	}

	//attack adjacent enemies even at bad odds
	attackAnyUnit(200);

	if(getMissionPlot()==NULL || (!getMissionPlot()->isCity()))
	{
		if(getHeadUnit()->generatePath(getHeadUnit()->getOriginPlot(),0,true))
		{
			Move(getHeadUnit()->getPathEndTurnPlot());
			return;
		}
		return;
	}

	CvCity* pTargetCity=getMissionPlot()->getPlotCity();

	//move towards target
	if(getHeadUnit()->generatePath(getMissionPlot(),0,true))
	{
		Move(getHeadUnit()->getPathEndTurnPlot());
	}

	pillageAdjacentPlots();
}

void CvAIGroup::update_barb_skeletons()
{
	CvPlayer& kPlayer=GET_PLAYER((PlayerTypes)GC.getBARBARIAN_PLAYER());
	int iValue,iBestValue=0;
	int iPathTurns;
	CvPlot* pBestPlot=NULL;

	if(getNumUnits()==0)
	{
		setDeathDelayed(true);
		return;
	}

	//attack adjacent enemies even at bad odds
	attackAnyUnit(25);

	if(getMissionPlot()==NULL || (!getMissionPlot()->isCity()))
	{
		if(getHeadUnit()->generatePath(getHeadUnit()->getOriginPlot(),0,true))
		{
			Move(getHeadUnit()->getPathEndTurnPlot());
			return;
		}
		return;
	}

	CvCity* pTargetCity=getMissionPlot()->getPlotCity();
    ImprovementTypes iGraveyard=(ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_GRAVEYARD");
	if(iGraveyard!=NO_IMPROVEMENT)
	{
		if(getHeadUnit()->plot()->getImprovementType()==iGraveyard)
		{
			Move(getHeadUnit()->plot());			
			return;
		}

		for (int iI = 0; iI < pTargetCity->getNumCityPlots(); iI++)
		{
			CvPlot* pLoopPlot = pTargetCity->getCityIndexPlot(iI);
			if (pLoopPlot != NULL)
			{
				if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT 
					&& GC.getImprovementInfo(pLoopPlot->getImprovementType()).getImprovementClassType()==0
					&& !GC.getImprovementInfo(pLoopPlot->getImprovementType()).isPermanent())
				{
					if(getHeadUnit()->generatePath(pLoopPlot,0,false,&iPathTurns))
					{
						int iDanger=kPlayer.AI_getPlotDanger(pLoopPlot,2);
						iValue=1000/(iPathTurns+iDanger+2);
						if(iValue>iBestValue)
						{
							iBestValue=iValue;
							pBestPlot=pLoopPlot;
						}
					}
				}
			}
		}

		if(pBestPlot!=NULL)
		{
			Move(pBestPlot);
			if(getHeadUnit()->atPlot(pBestPlot))
			{
				if(getHeadUnit()->getFortifyTurns() > 2) {
					pBestPlot->setImprovementType(iGraveyard);
				}

				Fortify();
			}
		}
	}
	return;
}

void CvPlot::CreateLair(CivilizationTypes eCiv)
{
	if (!isValidLairLocation())	{
		return;
	}

	int iValue;
	int iBestValue = 0;
	ImprovementTypes eBestImprovement = NO_IMPROVEMENT;

    for (int iJ = 0; iJ < GC.getNumImprovementInfos(); iJ++) {
		if (canHaveImprovement((ImprovementTypes)iJ, NO_TEAM) && !GC.getImprovementInfo((ImprovementTypes)iJ).isUnique()) {
			int iCiv = GC.getImprovementInfo((ImprovementTypes)iJ).getSpawnUnitCiv();
			if (iCiv != NO_CIVILIZATION) {
				if(iCiv == eCiv || eCiv == NO_CIVILIZATION) {
					if (iCiv != GC.getDefineINT("BARBARIAN_CIVILIZATION") || !isVisibleEnemyUnit(BARBARIAN_PLAYER))
					{
						iValue = GC.getGameINLINE().getSorenRandNum(100, "Improvement Choice");
						if (iValue > iBestValue)
						{
							iBestValue = iValue;
							eBestImprovement = (ImprovementTypes)iJ;
						}
					}
				}
			}  //iUnit != NO_UNIT
		}
	} // for ImprovementTypes

	if (eBestImprovement != NO_IMPROVEMENT) {
        int iUnit = GC.getImprovementInfo(eBestImprovement).getSpawnUnitType();
        int iCiv = GC.getImprovementInfo(eBestImprovement).getSpawnUnitCiv();
		DungeonTypes eDungeon = (DungeonTypes)GC.getImprovementInfo(eBestImprovement).getDungeonType();

		setImprovementType(eBestImprovement);
		if(eDungeon != NO_DUNGEON) {
			createDungeon(eDungeon);
			getDungeon()->spawnGuards();
		}	
	}
}

//returns % of how many Barbarians should spawn on a plot
int CvPlot::getBarBarianActivity()
{
    if (isOwned())
    {
        return 0;
    }

    CvPlot* pLoopPlot;
    int iRange=2;
    int iDX, iDY;
    int iCultureInfluence=0;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot)
			{
				if (pLoopPlot->isOwned() && pLoopPlot->getOwnerINLINE()<MAX_CIV_PLAYERS)
				{
                    int iDistance = stepDistance(getX_INLINE(), getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());

                    if (iDistance==1)
                    {
                        iCultureInfluence+=25;
                    }

                    if (iDistance==2)
                    {
                        iCultureInfluence+=10;
                    }

				}
			}
		}
	}

    return std::max(0,100-iCultureInfluence);
}

/**
 * @brief Returns if a Lair may spawn on a Plot
 */
bool CvPlot::isValidLairLocation()
{
	if(isCity())
		return false;

	if(isPeak() || isWater())
		return false;

	if(GC.getGame().getElapsedGameTurns() < 10) {

		if (isOwned()) {
			return false;
		}

		if (!GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)) {
			if(isVisibleToCivTeam()) {
				return false;
			}
		}

		if (area()->getNumTiles() < 5) {
			return false;
		}

		int iRange = 4;

		for (int iDX = -(iRange); iDX <= iRange; iDX++) {
			for (int iDY = -(iRange); iDY <= iRange; iDY++) {
				CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL) {
					if(pLoopPlot->isStartingPlot())
						return false;
				}
			}
		}
	}

	if (getImprovementType() != NO_IMPROVEMENT) {
		return false;
	}

//	if(isInRangeLair((GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS)) ? 2 : 3))
	if(isInRangeLair(2)) 
	{
		return false;
	}
	
	for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++) {
		CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
		
		if((pAdjacentPlot != NULL) 
			&& (pAdjacentPlot->isWildmanaLair() || pAdjacentPlot->isCity())) {
			return false;
		}
	}

	return true;
}

/**
* @brief Returns if a Barbarian Lair of eOwnerCiv is in given range
*
* @param  iRange  Maximum Search Range
* @param  eOwnerCiv ID of Barbarian Civ
*/
bool CvPlot::isInRangeLair(int iRange,CivilizationTypes eOwnerCiv)
{
    CvPlot* pLoopPlot;
    int iDX, iDY;

	for (iDX = -(iRange); iDX <= iRange; iDX++) {
		for (iDY = -(iRange); iDY <= iRange; iDY++) {
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot!=NULL) {
				if (pLoopPlot->getImprovementType()!=NO_IMPROVEMENT) {
					if(eOwnerCiv==NO_CIVILIZATION || GC.getImprovementInfo(pLoopPlot->getImprovementType()).getSpawnUnitCiv()==eOwnerCiv) {
						if(GC.getImprovementInfo(pLoopPlot->getImprovementType()).getSpawnUnitType()!=NO_UNIT)
							return true;					
					}
				}
			}
		}
	}
	return false;
}
