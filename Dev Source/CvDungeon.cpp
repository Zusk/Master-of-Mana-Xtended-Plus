// Dungeon.cpp

#include "CvGameCoreDLL.h"
#include "CvDungeon.h"
#include "CvPopupInfo.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"

// Public Functions...


CvDungeon::CvDungeon()
{
    m_paiPlayerAttitude = new int[MAX_PLAYERS];
    m_iTurnsSinceLastSpawn = 0;

	reset(0, true);
}


CvDungeon::~CvDungeon()
{
	uninit();

    SAFE_DELETE_ARRAY(m_paiPlayerAttitude);
}


void CvDungeon::init(int iID, int iDungeonType, int iPlotIndex)
{
	//--------------------------------
	// Init saved data

	reset(iID, iDungeonType, iPlotIndex, true);
}

void CvDungeon::uninit()
{
}

// FUNCTION: reset()
// Initializes data members that are serialized.
void CvDungeon::reset(int iID, int iDungeonType, int iPlotIndex, bool bConstructorCall)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_iID = iID;
	m_eDungeonType = (DungeonTypes)iDungeonType;
	m_iPlotIndex = iPlotIndex;

	m_eGuardUnit = NO_UNIT;
	m_eOwner = NO_PLAYER;
	m_iFirstTurnToTrigger = GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getBarbPercent() / 2;
	m_iLastTurnTriggeredDeal = m_iFirstTurnToTrigger;
	m_iPower = 0;
	m_iScoutRange = 5;
	m_iPopulation = 0;
	m_eReligion = NO_RELIGION;
	m_bWealthy = false;

    for (int iI = 0; iI < MAX_PLAYERS; iI++)
    {
        m_paiPlayerAttitude[iI] = ATTITUDE_ANNOYED;
    }

	if (!bConstructorCall)
	{
	}
}

void CvDungeon::kill()
{
	GC.getMap().deleteDungeon(getID());
}

int CvDungeon::getID() const
{
	return m_iID;
}


int CvDungeon::getIndex() const
{
	return (getID() & FLTA_INDEX_MASK);
}

void CvDungeon::setID(int iID)
{
	m_iID = iID;
}

DungeonTypes CvDungeon::getDungeonType() const
{
	return m_eDungeonType;
}

int CvDungeon::getPower() const
{
	return getPowerTimes1000() / 1000;
}

int CvDungeon::getPowerTimes1000() const
{
	return m_iPower;
}

int CvDungeon::getScoutRange() const
{
	return m_iScoutRange;
}

int CvDungeon::getPopulation() const
{
	return m_iPopulation;
}

void CvDungeon::changePopulation(int iChange)
{
	m_iPopulation = std::max(0, m_iPopulation + iChange);
}

ReligionTypes CvDungeon::getReligionType() const
{
	return m_eReligion;
}

void CvDungeon::setReligionType(ReligionTypes eNewReligion)
{
	m_eReligion = eNewReligion;
}

bool CvDungeon::isWealthy() const
{
	return m_bWealthy;
}

void CvDungeon::setWealthy(bool bNewValue)
{
	m_bWealthy = bNewValue;
}

/*
const std::vector<CvUnit*>& CvDungeon::getUnits() const
{
    return m_units;
}
*/

void CvDungeon::changePowerTimes1000(int iChange)
{
	if(iChange != 0) {
		int m_iOld = m_iPower;
		m_iPower += iChange * 100 / std::max(1, GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getBarbPercent());

		m_iPower = range(m_iPower, 0, 10000);

		int iUnitPowerChange = (m_iPower / 1000) - (m_iOld / 1000);
		//Update Power of Units on Plot
		if(iUnitPowerChange != 0) {
			if(isOOSLogging())
			{
				oosLog("Dungeon"
					,"Turn:%d,Dungeon:%S,ID:%d,X:%d,Y:%d,Attitude:%d,OldPower:%d,NewPower:%d"
					,GC.getGameINLINE().getElapsedGameTurns()
					,GC.getImprovementInfo(plot()->getImprovementType()).getDescription()
					,getID()
					,getX()
					,getY()
					,getAttitude((PlayerTypes)0)
					,m_iOld / 1000
					,m_iPower / 1000
				);
			}
			CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
			while (pUnitNode != NULL)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = plot()->nextUnitNode(pUnitNode);
				UpdateDungeonBarbarianStrength(pLoopUnit);
			}
		}
	}
}

void CvDungeon::UpdateDungeonBarbarianStrength(CvUnit* pUnit) {
    int dungeonPower = getPower(); // Get the dungeon's power
    bool isRagingBarbarians = GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS);

    // Adjust dungeon power based on the 'Raging Barbarians' game option
    if (!isRagingBarbarians) {
        dungeonPower /= 2; // Half the power if 'Raging Barbarians' is not active
    }

    // Get the unit's current base combat strengths
    int baseCombatStr = pUnit->baseCombatStr();
    int baseCombatStrDefense = pUnit->baseCombatStrDefense();

    // Initialize power increase variables
    int powerIncrease = 0;
    int defensePowerIncrease = 0;

    // Determine power increases based on unit's current strengths and conditions
    if (baseCombatStr <= 23) {
        if (baseCombatStr > 19 && !isRagingBarbarians) {
            // No power increase unless Raging Barbarians is on
        } else if (baseCombatStr > 17) {
            // Increase power 5% of the time
            if (GC.getGame().getSorenRandNum(100, "Dungeon Unit: Increase Combat Strength 1%") < 5) {
                powerIncrease = dungeonPower;
            }
        } else if (baseCombatStr > 14) {
            // Increase power 10% of the time
            if (GC.getGame().getSorenRandNum(100, "Dungeon Unit: Increase Combat Strength 10%") < 10) {
                powerIncrease = dungeonPower;
            }
        } else {
            // Always increase power if base strength is 14 or less
            powerIncrease = dungeonPower;
        }

        // Apply the power increases to the unit's combat strengths, clamping to a max of 25
        int newCombatStr = std::min(baseCombatStr + powerIncrease, 25);
        int newCombatStrDefense = std::min(baseCombatStrDefense + powerIncrease, 25);

        pUnit->setBaseCombatStr(newCombatStr);
        pUnit->setBaseCombatStrDefense(newCombatStrDefense);
    }
}


void CvDungeon::read(FDataStreamBase* pStream)
{

	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iID);
	pStream->Read((int*)&m_eDungeonType);
	pStream->Read(&m_iPlotIndex);

	pStream->Read((int*)&m_eGuardUnit);
	pStream->Read((int*)&m_eOwner);
	pStream->Read(&m_iLastTurnTriggeredDeal);
	pStream->Read(&m_iFirstTurnToTrigger);
	pStream->Read(&m_iPower);
	pStream->Read(&m_iScoutRange);
	pStream->Read(&m_iPopulation);	
	pStream->Read((int*)&m_eReligion);
	pStream->Read(&m_bWealthy);	
	pStream->Read(MAX_PLAYERS, m_paiPlayerAttitude);
    pStream->Read(&m_iTurnsSinceLastSpawn);
}

void CvDungeon::write(FDataStreamBase* pStream)
{

	uint uiFlag=2;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iID);
	pStream->Write(m_eDungeonType);
	pStream->Write(m_iPlotIndex);
	pStream->Write(m_eGuardUnit);

	pStream->Write(m_eOwner);
	pStream->Write(m_iLastTurnTriggeredDeal);
	pStream->Write(m_iFirstTurnToTrigger);
	pStream->Write(m_iPower);
	pStream->Write(m_iScoutRange);
	pStream->Write(m_iPopulation);
	pStream->Write(m_eReligion);
	pStream->Write(m_bWealthy);
    pStream->Write(m_iTurnsSinceLastSpawn);

	pStream->Write(MAX_PLAYERS, m_paiPlayerAttitude);
}

CvPlot* CvDungeon::plot() const
{
	return GC.getMapINLINE().plotByIndex(m_iPlotIndex);
}

void CvDungeon::spawnGuards()
{
	ImprovementTypes eImprovement = plot()->getImprovementType();
	if(eImprovement == NO_IMPROVEMENT)
		return;

	m_eGuardUnit = (UnitTypes)GC.getImprovementInfo(eImprovement).getSpawnUnitType();
    int iCiv = GC.getImprovementInfo(eImprovement).getSpawnUnitCiv();

	if(m_eGuardUnit != NO_UNIT) {
		int iNumGuards = GC.getGame().getSorenRandNum(3, "Random number of guards") + 2;

		if (iCiv == GC.getDefineINT("BARBARIAN_CIVILIZATION")) {
			m_eOwner = BARBARIAN_PLAYER;
			for (int i = 0; i < iNumGuards; ++i) {
				spawnGuard();
			}
		}

		if (iCiv == GC.getDefineINT("ANIMAL_CIVILIZATION")) {
			m_eOwner = ANIMAL_PLAYER;
			for (int i = 0; i < iNumGuards; ++i) {
				spawnGuard();
			}
		}
	}
}

void CvDungeon::spawnGuard() 
{
	if(m_eOwner == BARBARIAN_PLAYER)
		plot()->SpawnBarbarianUnit(m_eGuardUnit, m_eOwner);

    // Check if the dungeon owner is the animal player.
    if (m_eOwner == ANIMAL_PLAYER) {
		plot()->SpawnBarbarianUnit(m_eGuardUnit, m_eOwner, true, UNITAI_ANIMALDEFENDER);
		/*Need to add caps to animals per dungeon
        // Count the current guards in the dungeon.
        int iGuardCount = countDungeonGuards(plot());

        // If no guards are present, spawn the guard as a defender of the den.
        if (iGuardCount < (calculateDungeonGuardsNeeded() - 1)) {
            plot()->SpawnBarbarianUnit(m_eGuardUnit, m_eOwner, true, NO_UNITAI);
        } else {
            // If there are already guards, spawn the guard with animal AI.
            plot()->SpawnBarbarianUnit(m_eGuardUnit, m_eOwner, true, UNITAI_ANIMAL);
        }
	*/
	}
}

AttitudeTypes CvDungeon::getAttitude(PlayerTypes ePlayer) const
{
	return (AttitudeTypes)m_paiPlayerAttitude[ePlayer];
}

void CvDungeon::IncreaseAttitude(PlayerTypes ePlayer)
{
	if(m_paiPlayerAttitude[ePlayer] < ATTITUDE_FRIENDLY)
		++m_paiPlayerAttitude[ePlayer];
}
void CvDungeon::DecreaseAttitude(PlayerTypes ePlayer)
{
	if(m_paiPlayerAttitude[ePlayer] > ATTITUDE_FURIOUS)
		--m_paiPlayerAttitude[ePlayer];	
}

void CvDungeon::doTurn()
{
	//SpyFanatic: patch taken from Zusk, to remove Dungeon destroyed but not removed
	CvPlot* pPlot = plot();
	if(pPlot == NULL
		|| pPlot->getImprovementType() == NO_IMPROVEMENT
		|| GC.getImprovementInfo(pPlot->getImprovementType()).getSpawnUnitCiv() == NO_CIVILIZATION
		|| (pPlot->getPlotCity() != NULL && pPlot->getPlotCity()->getOwnerINLINE() < BARBARIAN_PLAYER)
	)
	{
		if(isOOSLogging())
		{
			oosLog("Dungeon"
				,"Turn:%d,Dungeon:%S,SpawnTeam:%d,ID:%d,X:%d,Y:%d,Owner:%d,isCity:%d,Kill"
				,GC.getGameINLINE().getElapsedGameTurns()
				,plot()->getImprovementType() != NO_IMPROVEMENT ? GC.getImprovementInfo(plot()->getImprovementType()).getDescription() : L"NO_IMPROVEMENT"
				,plot()->getImprovementType() != NO_IMPROVEMENT ? GC.getImprovementInfo(pPlot->getImprovementType()).getSpawnUnitCiv() : -1
				,getID()
				,getX()
				,getY()
				,m_eOwner
				,plot()->getPlotCity()!=NULL
			);
		}
		kill(); // Destroy the object which is probably a leftover and can cause CTD
		return;
	}
	/*
	ImprovementTypes iGraveYard = (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_GRAVEYARD");
	ImprovementTypes iDungeon = (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_DUNGEON");
	ImprovementTypes iBarrow = (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_BARROW");
	CvPlot* pPlot = plot();
	if (pPlot == NULL || (pPlot->getImprovementType() != iGraveYard && pPlot->getImprovementType() != iDungeon && pPlot->getImprovementType() != iBarrow))
	{
		kill(); // Destroy the object if the plot is null or the improvement is not a graveyard, dungeon, or barrow
		return;
	}
	if (pPlot != NULL && (pPlot->getPlotCity() != NULL && pPlot->getPlotCity()->getOwnerINLINE() < BARBARIAN_PLAYER))
	{
		kill(); // Destroy the object if the plot is null or the improvement is not a graveyard, dungeon, or barrow
		return;
	}
	*/
	//Rest of code normal after this
	doLogging();
	triggerDeals();

    // Get the type of improvement on the plot.
    ImprovementTypes eImprovementType = pPlot->getImprovementType();
    // Default to no civilization type for spawning.
    CivilizationTypes eSpawnCiv = NO_CIVILIZATION;

    // Check if the plot has a valid improvement.
    if (eImprovementType != NO_IMPROVEMENT) {
        // Retrieve the civilization type that is supposed to spawn from the improvement.
        eSpawnCiv = static_cast<CivilizationTypes>(GC.getImprovementInfo(eImprovementType).getSpawnUnitCiv());
    } else {
        // If there is no valid improvement, remove the dungeon.
        kill();
        return;
    }

    // If the improvement has a civilization type for spawning, execute turn processes.
    if (eSpawnCiv != NO_CIVILIZATION) {
        // Perform logging activities for the dungeon.
        doLogging();
        //Only barbarians care about deals. Animals and necromancers cba.
		if(eSpawnCiv == GC.getDefineINT("BARBARIAN_CIVILIZATION")){
			// Trigger any deals associated with the dungeon.
			triggerDeals();
		}
        // Increase the dungeon's power slightly every turn.
        changePowerTimes1000(10);
        // Process changes to wealth and population.
        processDungeonWealthAndPopulation();
        // Handle the spawning of guards based on various conditions.
        processDungeonGuardSpawning(pPlot);
    } else {
        // If the improvement type does not spawn any units, remove the dungeon.
        kill();
    }
}

void CvDungeon::processDungeonWealthAndPopulation() {
    // If the dungeon is wealthy, there's a chance to increase its population.
    if (isWealthy()) {
        if (GC.getGame().getSorenRandNum(100, "Trigger PopulationIncrease") == 0) {
            changePopulation(1);
        }
    }

    // There's a chance for a wealthy dungeon to lose its wealth over time.
    if (GC.getGame().getSorenRandNum(100, "Trigger DungeonWealthDecay") == 0) {
        setWealthy(false);
    }
}

/*
bool areAdjacentAnimalsPresent(CvPlot* pLoopPlot) {
    // Loop through all direction types
    for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI) {
        // Get the adjacent plot in the current direction
        CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX(), pLoopPlot->getY(), (DirectionTypes)iI);
        
        // Check if the adjacent plot exists
        if (pAdjacentPlot != NULL) {
            // Check for the presence of any units
            for (int i = 0; i < pAdjacentPlot->getNumUnits(); ++i) {
                // Get the unit in the adjacent plot
    			// Retrieve the first unit in the plot's unit list.
   				 CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode();

    			// Iterate over all units in the plot.
    			if (pUnitNode != NULL) {
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					if(pLoopUnit != NULL){
						if (pLoopUnit->getOwner() == GC.getANIMAL_PLAYER()) {
							// Return true as soon as an animal is found
							return true;
						}
					}
				}
			}
		}
    }
    
    // Return false if no animals are found in any adjacent plots
    return false;
}
*/

void CvDungeon::processDungeonGuardSpawning(CvPlot* pPlot) {
    // Do not spawn guards if there's an enemy defender visible.
    if (pPlot->getVisibleEnemyDefender(m_eOwner) != NULL) {
        return;
    }

    // Do not spawn guards if the dungeon is located in a city plot.
    if (pPlot->getPlotCity() != NULL) {
        return;
    }

    // Calculate the number of guards needed based on current conditions.
    int iGuardsNeeded = calculateDungeonGuardsNeeded();
    // Count the current number of guards at the plot.
    int iCountGuards = countDungeonGuards(pPlot);

    // Determine how many turns to wait before spawning a new guard.
    int spawnDelay = calculateDungeonSpawnDelay();

    // If the required number of guards is present, reset the spawn counter.
    if (iCountGuards >= iGuardsNeeded) {
        m_iTurnsSinceLastSpawn = 0;
    } else {
        // If not enough guards, increment the counter.
        m_iTurnsSinceLastSpawn++;
    }

    // Spawn a guard if the conditions are met.
    if (iCountGuards < iGuardsNeeded && m_iTurnsSinceLastSpawn >= spawnDelay) {
		spawnGuard();
        // Reset the spawn counter after spawning a guard.
        m_iTurnsSinceLastSpawn = 0;
    }
}

int CvDungeon::calculateDungeonGuardsNeeded() {
    // Start with a base number of guards.
    int iGuardsNeeded = 3;
    // Check if the 'Raging Barbarians' game option is enabled.
    bool isRagingBarbarians = GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS);
    // Adjust the dungeon's power based on the game difficulty and settings.
    int iDungeonPower = std::max(1, getPower()) / (isRagingBarbarians ? 2 : 4);
    // Calculate additional guards needed based on dungeon power and game settings.
    iGuardsNeeded += isRagingBarbarians ? std::min(5, iDungeonPower) : std::min(2, iDungeonPower);

    // Check if the dungeon's civilization is the animal civilization and halve the guards needed if so.
	ImprovementTypes eImprovement = plot()->getImprovementType();
    int iCiv = GC.getImprovementInfo(eImprovement).getSpawnUnitCiv();
    if (iCiv == GC.getDefineINT("ANIMAL_CIVILIZATION")) {
        iGuardsNeeded = std::max(2, iGuardsNeeded / 2);
    }

    return iGuardsNeeded;
}

int CvDungeon::countDungeonGuards(CvPlot* pPlot) {
    // Initialize the guard counter.
    int iCountGuards = 0;
    // Retrieve the first unit in the plot's unit list.
    CLLNode<IDInfo>* pUnitNode = pPlot->headUnitNode();

    // Iterate over all units in the plot.
    while (pUnitNode != NULL) {
        CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
        pUnitNode = pPlot->nextUnitNode(pUnitNode);

        // Count units that belong to the dungeon's owner and are not in any AI group.
        if (pLoopUnit->getOwnerINLINE() == m_eOwner && pLoopUnit->getAIGroup() == NULL) {
            ++iCountGuards;
        }
    }
    // Return the total count of guards.
    return iCountGuards;
}

int CvDungeon::calculateDungeonSpawnDelay() {
	int iSpawnDelay;
    // Check if 'Raging Barbarians' is active to adjust spawn delay.
    bool isRagingBarbarians = GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS);
	iSpawnDelay = isRagingBarbarians ? 2 : 3; 
    // Set the spawn delay based on the game option.

	//For animals
	ImprovementTypes eImprovement = plot()->getImprovementType();
    int iCiv = GC.getImprovementInfo(eImprovement).getSpawnUnitCiv();
    if (iCiv == GC.getDefineINT("ANIMAL_CIVILIZATION")) {
		iSpawnDelay = isRagingBarbarians ? 4 : 6;
	}
	//For non animals
	else{
		iSpawnDelay = isRagingBarbarians ? 2 : 3;
	}
	//Return the value
    return iSpawnDelay;
}

void CvDungeon::doLogging()
{
	if(isOOSLogging())
	{
		oosLog("Dungeon"
			,"Turn:%d,Dungeon:%S,ID:%d,X:%d,Y:%d,Attitude:%d,Last Turn Triggered Deal:%d,Chance to Trigger:%d,Guard Unit:%S,Owner:%d,Power:%d,isCity:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,GC.getImprovementInfo(plot()->getImprovementType()).getDescription()
			,getID()
			,getX()
			,getY()
			,getAttitude((PlayerTypes)0)
			,m_iLastTurnTriggeredDeal
			,getChanceToTriggerDealsTimes100((PlayerTypes)0) / 100
			,(m_eGuardUnit != NO_UNIT?GC.getUnitInfo(m_eGuardUnit).getDescription():L"None")
			,m_eOwner
			,getPower()
			,plot()->getPlotCity()!=NULL
		);
	}
}

//returns Chance to Trigger Deals with Barbs Times 100
int CvDungeon::getChanceToTriggerDealsTimes100(PlayerTypes ePlayer) const
{
	int iBase = 100 + (GC.getGame().getElapsedGameTurns() - m_iLastTurnTriggeredDeal) * 25;

//	if(ePlayer != NO_PLAYER)
//		iBase /= std::max<int>(1, getAttitude(ePlayer)); //value between 1 and 4

	return iBase;
}

void CvDungeon::triggerDeals()
{
	int iLoop;
	int iWeight;
	int iBestWeight = 0;
	PlayerTypes bTarget = NO_PLAYER;
	int iCurrentTurn = GC.getGame().getElapsedGameTurns();
	int iRangeToCities = getScoutRange();

	//does plot Already have a SpawnGroup?
	CvPlayer& kOwner = GET_PLAYER((PlayerTypes)m_eOwner);

	for(CvAIGroup* pAIGroup = kOwner.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kOwner.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getHeadUnit() != NULL)
		{
			if(pAIGroup->getHeadUnit()->getOriginPlot() == plot())
			{
				//there is already a Spawngroup from this Dungeon
				return;
			}
		}
	}

	for(int i = 0; i < MAX_CIV_PLAYERS; ++i) {
		CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)i);
		iWeight = 0;

		//skip if early and no Second City yet
		//if(kPlayer.getNumCities() < 2 && iCurrentTurn < 50) {
		//	continue;
		//}

		if(kPlayer.isHuman()) {
			for(CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop)) {
				if(plotDistance(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), plot()->getX_INLINE(), plot()->getY_INLINE()) < iRangeToCities) {
					iWeight += pLoopCity->getPopulation() / (std::max<int>(1, getAttitude((PlayerTypes)i)));
				}
			}

			if(iWeight > iBestWeight) {
				iBestWeight = iWeight;
				bTarget = (PlayerTypes)i;
			}
		}
	}

	if(GC.getGame().getSorenRandNum(10000, "Trigger Dungeon Deal") < getChanceToTriggerDealsTimes100(bTarget)) {
		m_iLastTurnTriggeredDeal = iCurrentTurn;
	}
	else {
		return;
	}

	if(bTarget != NO_PLAYER) {
		if (GET_PLAYER(bTarget).isHuman())
		{
			CvPopupInfo* pInfo = new CvPopupInfo(BUTTONPOPUP_DUNGEON_EVENT);
			if (NULL != pInfo)
			{
				pInfo->setData1(getID());
				pInfo->setData2(GET_PLAYER(bTarget).getCapitalCity()->getID());
				gDLL->getInterfaceIFace()->addPopup(pInfo, bTarget);
			}
			return;
		}
	}
}

int CvDungeon::getX() const { return GC.getMapINLINE().plotByIndex(m_iPlotIndex)->getX_INLINE(); } 
int CvDungeon::getY() const { return GC.getMapINLINE().plotByIndex(m_iPlotIndex)->getY_INLINE(); }

bool CvDungeon::isDealAvailable(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	CvDungeonEventInfo& kEvent = GC.getDungeonEventInfo(eEvent);
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	if(!kEvent.isPrereqAttitude(getAttitude(ePlayer))) {
		return false;
	}

	if(kEvent.getPopulationChange() < 0) {
		if(getPopulation() < DUNGEON_POPULATION_HIGH) {
			return false;
		}
	}

	if(kEvent.getGold() > 0) {
		if(kPlayer.getGold() < Deal_calculateGoldCost(ePlayer, eEvent)) {
			return false;
		}
	}

	if(kEvent.getMetal() > 0) {
		if(kPlayer.getGlobalYield(YIELD_METAL) < Deal_calculateMetalCost(ePlayer, eEvent)) {
			return false;
		} 
	}

	if(kEvent.getLeather() > 0) {
		if(kPlayer.getGlobalYield(YIELD_LEATHER) < Deal_calculateLeatherCost(ePlayer, eEvent)) {
			return false;
		} 
	}

	if(kEvent.isDonateUnits()) {
		if(this->m_eGuardUnit == NO_UNIT)
			return false;
	}

	if(!kEvent.isPositive()) {
		if(!GET_TEAM(kPlayer.getTeam()).isAtWar(BARBARIAN_TEAM)) {
			return false;
		}
	}

	return true;
}

void CvDungeon::ResolveDeal(PlayerTypes ePlayer, DungeonEventTypes eEvent)
{
	CvDungeonEventInfo& kEvent = GC.getDungeonEventInfo(eEvent);
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	int iGold, iMetal, iLeather;

	if(kEvent.isPositive()) {
		IncreaseAttitude(ePlayer);
		changePowerTimes1000(200); // 0.2 in Unitstrength increase
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_ATTITUDE_INCREASE", GC.getAttitudeInfo(getAttitude(ePlayer)).getDescription()));
	}
	else {
		DecreaseAttitude(ePlayer);
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_ATTITUDE_DECREASE", GC.getAttitudeInfo(getAttitude(ePlayer)).getDescription()));
	}

	if(kEvent.getPowerChange() != 0) {
		changePowerTimes1000(kEvent.getPowerChange());
	}

	if(kEvent.getPopulationChange() != 0) {
		changePopulation(kEvent.getPopulationChange());
	}

	if(kEvent.getGold() > 0) {
		iGold = Deal_calculateGoldCost(ePlayer, eEvent);
		kPlayer.changeGold(-iGold);
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_GOLD_COST", iGold));
		setWealthy(true);		
	}
	if(kEvent.getGold() < 0) {
		iGold = Deal_calculateGoldGained(ePlayer, eEvent);
		kPlayer.changeGold(iGold);
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_GOLD_GAINED", iGold));
	}

	if(kEvent.getMetal() > 0) {
		iMetal = Deal_calculateMetalCost(ePlayer, eEvent);
		kPlayer.changeGlobalYield(YIELD_METAL, -iMetal);
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_METAL_COST", iMetal));
	}

	if(kEvent.getLeather() > 0) {
		iLeather = Deal_calculateLeatherCost(ePlayer, eEvent);
		kPlayer.changeGlobalYield(YIELD_LEATHER, -iLeather);
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_LEATHER_COST", iLeather));
	}

	if(kEvent.getLeather() < 0) {
		iLeather = -kEvent.getLeather();
		kPlayer.changeGlobalYield(YIELD_LEATHER, iLeather);
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_LEATHER_GAINED", iLeather));
	}

	if(kEvent.getPopulation() > 0) {
		sellSlaves(ePlayer, kEvent.getPopulation());
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_REDUCED_POPULATION"));
		//TODO: display Text
	}

	if(kEvent.isDonateUnits()) {
		Deal_DonateUnits(ePlayer, eEvent);
	}

	if(!kEvent.isPositive()) {
		Deal_TriggerRaidGroup(ePlayer, eEvent);
	}
}

void CvDungeon::Deal_DonateUnits(PlayerTypes ePlayer, DungeonEventTypes eEvent)
{
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	int iLoop, iValue;
	int iBestValue = MAX_INT;
	CvPlot* pTarget = NULL;

	if(m_eGuardUnit == NO_UNIT)
		return;

	for(CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop)) {
		iValue = plotDistance(pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE(), plot()->getX_INLINE(), plot()->getY_INLINE());
		if(iValue < iBestValue) {
			iBestValue = iValue;
			pTarget = pLoopCity->plot();
		}
	}

	if(pTarget != NULL) {
		for(int iI = 0; iI < 2; iI++)
		{
			CvUnit* pNewUnit = kPlayer.initUnit(m_eGuardUnit, pTarget->getX_INLINE(), pTarget->getY_INLINE());
			UpdateDungeonBarbarianStrength(pNewUnit);
		}
	}
}

void CvDungeon::Deal_TriggerRaidGroup(PlayerTypes ePlayer, DungeonEventTypes eEvent)
{
	CvDungeonEventInfo& kEvent = GC.getDungeonEventInfo(eEvent);
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	CvPlayer& kBarb = GET_PLAYER(m_eOwner);

	if(kEvent.isThreaten()) {
		//Chance to give in and Spawn nothing
		int iChance = 5;
		for(int iI = 0; iI < MAX_CIV_PLAYERS; ++iI) {
			if(kPlayer.getID() != iI) {
				if(kPlayer.getPower() > GET_PLAYER((PlayerTypes)iI).getPower()) {
					iChance += 5;
				}
			}
		}
		iChance = std::min(50, iChance);

		if(GC.getGameINLINE().getSorenRandNum(100, "DungeonEvent Threaten") < iChance) {
			//spawn no Barbs
			return;
		}
	}

	int iSize = 1; //Raids are slightly smaller by default.

	switch(getPopulation()) {
		case DUNGEON_POPULATION_LOW:
			break;
		case DUNGEON_POPULATION_AVERAGE:
			iSize += 1;
			break;
		case DUNGEON_POPULATION_HIGH:
			iSize += 2;
			break;
		case DUNGEON_POPULATION_VERY_HIGH:
		default:
			iSize += 3;
			break;
	}

	int iHeroChance = 5;
	int iGiveInChance = 5;
	if(kEvent.isTaunt()) {
		iSize += 2;
		iHeroChance *= 10;
	}
	if(kEvent.isThreaten()) {
		iSize += 1;	
	}

	CvPlot* pTarget = NULL;
	int iSearchRange = getScoutRange();
	int iValue;
	int iBestValue = 0;
	for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
	{
		for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
		{
			CvPlot* pLoopPlot = plotXY(plot()->getX_INLINE(), plot()->getY_INLINE(), iDX, iDY);
			if(pLoopPlot != NULL)
			{
				if(pLoopPlot->getArea() == plot()->getArea())
				{
					if(!pLoopPlot->isBarbarian() && pLoopPlot->isCity() && pLoopPlot->getOwnerINLINE() == ePlayer)
					{
						if(pLoopPlot->isOwned() && GET_TEAM(pLoopPlot->getTeam()).isAtWar((TeamTypes)GC.getBARBARIAN_TEAM()))
						{
							iValue = 100 / std::max(1, plotDistance(plot()->getX_INLINE(), plot()->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE()));
							if(iValue > iBestValue) {
								pTarget = pLoopPlot;
								iBestValue = iValue;
							}
						}
					}
				}
			}
		}
	}

	if(pTarget == NULL)
		return;

	UnitTypes eUnit1 = m_eGuardUnit;
	if(eUnit1 == NO_UNIT)
		eUnit1 = (UnitTypes)GC.getInfoTypeForString("UNIT_AXEMAN");

	//TODO: read from XML
	AIGroupTypes eGroupType = AIGROUP_BARB_PLUNDER;

	int iNumUnit1 = iSize;

	if(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		iNumUnit1 += 2;
	}

	CvAIGroup* pSpawnGroup;
	if(eUnit1 != NO_UNIT)
	{
		pSpawnGroup = kBarb.initAIGroup(eGroupType);
		pSpawnGroup->setMissionTarget(pTarget);
		for(int iI = 0; iI < iNumUnit1; iI++)
		{
			CvUnit* pNewUnit = kBarb.initUnit(eUnit1, plot()->getX_INLINE(), plot()->getY_INLINE());
			pNewUnit->setOriginPlot(plot());
			pNewUnit->setAIGroup(pSpawnGroup);
			//Assigns the unit promotions based on the dungeons power.
			UpdateDungeonBarbarianStrength(pNewUnit);
		}
		
		changePopulation(-(1 + iNumUnit1 / 5));
	}
}

int CvDungeon::Deal_calculateGoldCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	int iGold = GC.getDungeonEventInfo(eEvent).getGold() * (100 + 30 * (3 - getAttitude(ePlayer)));

	if(GC.getDungeonEventInfo(eEvent).isDonateUnits()) {
		iGold += 10 * getPowerTimes1000();
		iGold += 10000 * std::max(0, GC.getUnitInfo(m_eGuardUnit).getCombat() - 10);
	}

	return iGold / 100;
}

int CvDungeon::Deal_calculateGoldGained(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	int iGold = -GC.getDungeonEventInfo(eEvent).getGold() * (100 + 50 * getAttitude(ePlayer));

	iGold *= (isWealthy()) ? 2 : 1;

	return iGold / 100;
}

int CvDungeon::Deal_calculateMetalCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	return GC.getDungeonEventInfo(eEvent).getMetal();
}

int CvDungeon::Deal_calculateLeatherCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	return GC.getDungeonEventInfo(eEvent).getLeather();
}

//This used to reduce EVERY cities population by 10%, which essentially was crippling if you were playing wide, with how food growth is harder in MoM. This had to be nerfed to be about equivelant to the gold option.
void CvDungeon::sellSlaves(PlayerTypes ePlayer, int iSlaves) const
{
    CvPlayer& kPlayer = GET_PLAYER(ePlayer);
    int iLoop;
    int iPopChange;
    CvCity* pTargetCity = NULL;
    int iBestValue = MAX_INT; // Initialize with maximum possible int value to find the minimum later

    // Find the city with the smallest population greater than 1 to be the target
    for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop)) {
        if (pLoopCity->getPopulation() > 1 && pLoopCity->getPopulation() < iBestValue) {
            iBestValue = pLoopCity->getPopulation();
            pTargetCity = pLoopCity;
        }
    }

    // If a target city is found, reduce its population
    if (pTargetCity != NULL) {
        iPopChange = std::max(1, iSlaves); // Use the number of slaves sold to determine the population change
        pTargetCity->changePopulation(-iPopChange);
    }
}

void CvDungeon::updateLandmark()
{
	//DEBUG
	return;

	PlayerTypes ePlayer = GC.getGame().getActivePlayer();
	CvWString szText;
	if(ePlayer != NO_PLAYER) {
		szText.append(GC.getAttitudeInfo(getAttitude(ePlayer)).getDescription());
		gDLL->getEngineIFace()->addLandmark(plot(), szText);
	}
}
