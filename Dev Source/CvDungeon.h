#pragma once

#ifndef CIV4_DUNGEON_H
#define CIV4_DUNGEON_H

#include "CvDLLEntity.h"
#include "LinkedList.h"
//#include <vector>

class CvDungeon
{
public:
	// stuff necessary to communicate with rest of DLL
	CvDungeon();
	virtual ~CvDungeon();

	void init(int iID, int iDungeonType, int iPlotIndex);
	void uninit();
	void reset(int iID = 0, int iDungeonType = NO_DUNGEON, int iPlotIndex = -1, bool bConstructorCall = false);
	void kill();

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	int getID() const;																			// Exposed to Python
	int getIndex() const;
//	IDInfo getIDInfo() const;
	void setID(int iID);

	DungeonTypes getDungeonType() const;

	// end

	CvPlot* plot() const;

	int getX() const;
	int getY() const;

	int getScoutRange() const;

	void spawnGuards();
	void spawnGuard(); 
	void doTurn();
	int calculateDungeonSpawnDelay();
	int countDungeonGuards(CvPlot* pPlot);
	int calculateDungeonGuardsNeeded();
	void processDungeonGuardSpawning(CvPlot* pPlot);
	void processDungeonWealthAndPopulation();
    //const std::vector<CvUnit*>& getUnits() const;
	void doLogging();

	void updateLandmark();

	void triggerDeals();
	int getChanceToTriggerDealsTimes100(PlayerTypes ePlayer) const;
	void ResolveDeal(PlayerTypes ePlayer, DungeonEventTypes eEvent);
	bool isDealAvailable(PlayerTypes ePlayer, DungeonEventTypes eEvent) const;
	void Deal_DonateUnits(PlayerTypes ePlayer, DungeonEventTypes eEvent);
	void Deal_TriggerRaidGroup(PlayerTypes ePlayer, DungeonEventTypes eEvent);
	void sellSlaves(PlayerTypes ePlayer, int iSlaves) const;

	int Deal_calculateGoldCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const;
	int Deal_calculateGoldGained(PlayerTypes ePlayer, DungeonEventTypes eEvent) const;
	int Deal_calculateMetalCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const;
	int Deal_calculateLeatherCost(PlayerTypes ePlayer, DungeonEventTypes eEvent) const;

	int getPower() const;
	int getPowerTimes1000() const;
	void changePowerTimes1000(int iChange);
	void UpdateDungeonBarbarianStrength(CvUnit* pUnit);

	int getPopulation() const;
	void changePopulation(int iChange);
	ReligionTypes getReligionType() const;
	void setReligionType(ReligionTypes eNewReligion);
	bool isWealthy() const;
	void setWealthy(bool bNewValue);

	void IncreaseAttitude(PlayerTypes ePlayer);
	void DecreaseAttitude(PlayerTypes ePlayer);
	AttitudeTypes getAttitude(PlayerTypes ePlayer) const;


protected:

	// stuff necessary to communicate with rest of DLL
	int m_iID;
	DungeonTypes m_eDungeonType;
	// end

	int m_iPlotIndex;
	UnitTypes m_eGuardUnit;
	PlayerTypes m_eOwner;
	int m_iLastTurnTriggeredDeal;
	int m_iFirstTurnToTrigger;
	int m_iPower;
	int m_iScoutRange;
	int m_iPopulation;
	//std::vector<CvUnit*> m_units; //List of units associated with the dungeon.
    int m_iTurnsSinceLastSpawn; //This variable is used to track how many turns it has been since we have spawned a guard. Implementing this is mostly to fix Dungeons spawning guards instantly if they are under their cap.
	ReligionTypes m_eReligion;

	bool m_bWealthy;

	int* m_paiPlayerAttitude;
};

#endif
