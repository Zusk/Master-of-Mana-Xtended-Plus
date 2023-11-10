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
	m_iScoutRange = 6;

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

void CvDungeon::changePowerTimes1000(int iChange)
{
	if(iChange != 0) {
		int m_iOld = m_iPower;
		m_iPower += iChange * 100 / std::max(1, GC.getGameSpeedInfo(GC.getGame().getGameSpeedType()).getBarbPercent());

		int iUnitPowerChange = (m_iPower / 1000) - (m_iOld / 1000);
		//Update Power of Units on Plot
		if(iUnitPowerChange != 0) {
			CLLNode<IDInfo>* pUnitNode = plot()->headUnitNode();
			while (pUnitNode != NULL)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = plot()->nextUnitNode(pUnitNode);
				
				pLoopUnit->setBaseCombatStr(pLoopUnit->baseCombatStr() + getPower());
				pLoopUnit->setBaseCombatStrDefense(pLoopUnit->baseCombatStrDefense() + getPower());
			}
		}
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
	pStream->Read(MAX_PLAYERS, m_paiPlayerAttitude);

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
		if (iCiv == GC.getDefineINT("BARBARIAN_CIVILIZATION")) {
			m_eOwner = BARBARIAN_PLAYER;
			spawnGuard();
			spawnGuard();
		}

		if (iCiv == GC.getDefineINT("ANIMAL_CIVILIZATION")) {
			m_eOwner = ANIMAL_PLAYER;
			spawnGuard();
			spawnGuard();
		}
	}
}

void CvDungeon::spawnGuard() 
{
	if(m_eOwner == BARBARIAN_PLAYER)
		plot()->SpawnBarbarianUnit(m_eGuardUnit, m_eOwner);

	if(m_eOwner == ANIMAL_PLAYER)
		plot()->SpawnBarbarianUnit(m_eGuardUnit, m_eOwner, true, UNITAI_ANIMAL);
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
	doLogging();
	triggerDeals();

	changePowerTimes1000(10);
}

void CvDungeon::doLogging()
{
	TCHAR szOut[1024];
	TCHAR szFile[1024];
	sprintf(szFile, "Dungeon_%d.log",getID());

	sprintf(szOut, "Attitude: %d\n", getAttitude((PlayerTypes)0));
	gDLL->logMsg(szFile,szOut, false, false);
	sprintf(szOut, "Last Turn Triggered Deal: %d\n", m_iLastTurnTriggeredDeal);
	gDLL->logMsg(szFile,szOut, false, false);
	sprintf(szOut, "Chance to Trigger: %d\n", getChanceToTriggerDealsTimes100((PlayerTypes)0) / 100);
	gDLL->logMsg(szFile,szOut, false, false);
	if(m_eGuardUnit != NO_UNIT) {
		sprintf(szOut, "Guard Unit: %S\n", GC.getUnitInfo(m_eGuardUnit).getDescription());
		gDLL->logMsg(szFile,szOut, false, false);
	}
	sprintf(szOut, "Owner: %d\n", m_eOwner);
	gDLL->logMsg(szFile,szOut, false, false);
	sprintf(szOut, "Power: %d\n", getPower());
	gDLL->logMsg(szFile,szOut, false, false);
}

//returns Chance to Trigger Deals with Barbs Times 100
int CvDungeon::getChanceToTriggerDealsTimes100(PlayerTypes ePlayer) const
{
	int iBase = 100 + (GC.getGame().getElapsedGameTurns() - m_iLastTurnTriggeredDeal) * 50;

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
		if(kPlayer.getNumCities() < 2 && iCurrentTurn < 50) {
			continue;
		}

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
	int iGold, iMetal;

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

	if(kEvent.getGold() > 0) {
		iGold = Deal_calculateGoldCost(ePlayer, eEvent);
		kPlayer.changeGold(-iGold);
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_GOLD_COST", iGold));
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

	if(kEvent.getPopulation() > 0) {
		sellSlaves(ePlayer, kEvent.getPopulation());
		kPlayer.displayText(gDLL->getText("TXT_KEY_DUNGEONEVENT_REDUCED_POPULATION"));
		//TODO: display Text
	}

	if(!kEvent.isPositive()) {
		Deal_TriggerRaidGroup(ePlayer, eEvent);
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

	int iStrength = 100;
	int iHeroChance = 5;
	int iGiveInChance = 5;
	if(kEvent.isTaunt()) {
		iStrength *= 2;
		iHeroChance *= 10;
	}
	if(kEvent.isThreaten()) {
		iStrength *= 3;
		iStrength /= 2;
		
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
	AIGroupTypes eGroupType = AIGROUP_BARB_ORCS;

	int iNumUnit1 = 2;

	if(GC.getGameINLINE().isOption(GAMEOPTION_RAGING_BARBARIANS))
	{
		iNumUnit1 *= 2;
	}

	CvAIGroup* pSpawnGroup;
	if(eUnit1 != NO_UNIT)
	{
		pSpawnGroup = kBarb.initAIGroup(eGroupType);
		pSpawnGroup->setMissionPlot(pTarget);
		for(int iI = 0; iI < iNumUnit1; iI++)
		{
			CvUnit* pNewUnit = kBarb.initUnit(eUnit1, plot()->getX_INLINE(), plot()->getY_INLINE());
			pNewUnit->setOriginPlot(plot());
			pNewUnit->setAIGroup(pSpawnGroup);
			pNewUnit->setBaseCombatStr(pNewUnit->baseCombatStr() + getPower());
			pNewUnit->setBaseCombatStrDefense(pNewUnit->baseCombatStrDefense() + getPower());
		}
	}
}

int CvDungeon::Deal_calculateGoldCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	int iGold = GC.getDungeonEventInfo(eEvent).getGold() * (100 + 30 * (3 - getAttitude(ePlayer)));

	return iGold / 100;
}

int CvDungeon::Deal_calculateGoldGained(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	int iGold = -GC.getDungeonEventInfo(eEvent).getGold() * (100 + 50 * getAttitude(ePlayer));

	return iGold / 100;
}

int CvDungeon::Deal_calculateMetalCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const
{
	int iMetal = GC.getDungeonEventInfo(eEvent).getMetal() * (100 + 30 * (3 - getAttitude(ePlayer)));

	return iMetal / 100;
}

void CvDungeon::sellSlaves(PlayerTypes ePlayer, int iSlaves) const
{
	CvPlayer& kPlayer = GET_PLAYER(ePlayer);
	int iLoop, iPopChange;

	for(CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop)) {
		if(pLoopCity->getPopulation() > 1) {
			iPopChange = std::max(1, pLoopCity->getPopulation() / 10);
			pLoopCity->changePopulation(-iPopChange);
		}
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
