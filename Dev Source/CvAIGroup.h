#pragma once

#ifndef CIV4_AIGROUP_H
#define CIV4_AIGROUP_H

#include "CvDLLEntity.h"
#include "LinkedList.h"

#include "AI_Defines.h"

class CvAIGroup
{
public:
	// stuff necessary to communicate with rest of DLL
	CvAIGroup();
	virtual ~CvAIGroup();

	void init(int iID, int iOwner, int iGroupType);
	void uninit();
	void reset(int iID = 0, int iOwner = NO_PLAYER, int iGroupType = NO_AIGROUP, bool bConstructorCall = false);
	void kill();

	void read(FDataStreamBase* pStream);
	void write(FDataStreamBase* pStream);

	int getID() const;																			// Exposed to Python
	int getIndex() const;
	void setID(int iID);

	DllExport PlayerTypes getOwner() const;	
#ifdef _USRDLL
	inline PlayerTypes getOwnerINLINE() const
	{
		return m_eOwner;
	}

#endif

	AIGroupTypes getGroupType();
	TeamTypes getTeam() const;
	void addUnit(CvUnit* pUnit);
	void removeUnit(CvUnit* pUnit);
	CLLNode<IDInfo>* deleteUnitNode(CLLNode<IDInfo>* pNode);
	CLLNode<IDInfo>* nextUnitNode(CLLNode<IDInfo>* pNode) const;
	CLLNode<IDInfo>* prevUnitNode(CLLNode<IDInfo>* pNode) const;
	CLLNode<IDInfo>* headUnitNode() const;
	CLLNode<IDInfo>* tailUnitNode() const;
	CvUnit* getHeadUnit() const;
	int getNumUnits() const;
	int getNumUnitCategoryUnits(int iUnitCategory) const;
	
	bool isDeathDelayed() const;
	void setDeathDelayed(bool bNewValue);
	bool isSeeInvisible() const;
	void changeSeeInvisible(int iChange);

	CvCity*	getMissionCity() const;
	void setMissionCity(CvCity* pNewCity);
	int getMissionArea();
	void setMissionArea(int iNewArea);
	int getMissionBuild();
	void setMissionBuild(int iNewBuild);
	int getMissionStatus();
	void setMissionStatus(int iNewStatus);
	CvPlot* getMissionPlot() const;
	void setMissionPlot(CvPlot* pNewPlot);
	CvPlot* getMissionTarget() const;
	void setMissionTarget(CvPlot* pNewPlot);
	CvAIGroup* getReserveGroup();
	void setReserveGroup(CvAIGroup* pNewGroup);
	CvAIGroup* getSiegeGroup();
	void setSiegeGroup(CvAIGroup* pNewGroup);
	CvAIGroup* getHelpGroup1();
	void setHelpGroup1(CvAIGroup* pNewGroup);
	CvAIGroup* getHelpGroup2();
	void setHelpGroup2(CvAIGroup* pNewGroup);
	CvAIGroup* getHelpGroup3();
	void setHelpGroup3(CvAIGroup* pNewGroup);
	CvUnit* getMissionTargetUnit();
	void setMissionTargetUnit(CvUnit* pTargetUnit);
	void update();
	void update_Reserve();
	void update_Reserve_ReinforceCities();
	void update_Reserve_ShuffleCityDefense();
	void update_Reserve_ReinforceCounters();
	void update_Reserve_ReinforceDestroyLair();
	void update_Reserve_ReinforceSiege();
	void update_Reserve_ReinforceCityInvasion();
	void update_Reserve_ReinforceManaNode();
	void update_Reserve_ReinforceExplore();
	void update_Reserve_ReinforceSettlers();
	void update_Reserve_ExploreDungeon();
	void update_Reserve_PickupEquipment();
	void update_Reserve_RetreatToCity();
	void update_Reserve_Upgrade();
	void update_City_Defense();
	void update_City_Defense_Replace();
	void update_Settle();
	void update_Worker();
	void update_Explore();
	CLLNode<IDInfo>* setAIGroupInLoop(CvAIGroup* pNewGroup, CLLNode<IDInfo>* pUnitNode, CvUnit* pLoopUnit);
	void update_Counter();
	bool update_Counter_isTargetStillValid();
	void update_Destroy_Lair();
	bool update_City_Invasion_isTargetStillValid();
	void update_City_Invasion_InitMissionPlot();
	bool update_City_Invasion_enoughUnits();
	bool update_City_Invasion_doesEnemyOutNumberUs(double dMod = 1.2) const;
	bool update_City_Invasion_doWeHaveHeroToTrain() const;
	bool update_City_Invasion_canWePillageStuff() const;
	bool update_City_Invasion_canBuildMoreUnits() const;
	void update_City_Invasion_moveToSecurePlot();
	void update_City_Invasion_TrainHero();
	CvPlot* update_City_Invasion_TrainHero_GetTargetPlot();
	void update_City_Invasion_PillageStuff();
	void update_City_Invasion_LookForOtherTarget();
	void update_City_Invasion_AttackCity();
	void update_City_Invasion();
	void update_Siege();
	void update_Mana_Node();
	void update_Naval_Reserve();
	void update_Reserve_Reinforce_NavalInvasion();	
	void update_Reserve_Reinforce_NavalSettle();
	void update_Reserve_Reinforce_NavalCounter();
	void update_Reserve_Reinforce_NavalExplorer();
	void update_Naval_Counter();
	CvAIGroup* getNavalSettleGroupNeedSettler(int iArea);
	bool canFillNavalSettleWithReserveUnits(CvAIGroup* pReserve);
	void update_Naval_Settle();
	void update_Naval_Invasion();
	void update_Naval_Explorer();
	bool isValidPlotToPlunder(CvPlot* pPlot) const;
	void update_barb_plunder();
	void update_barb_new_lair();
	void update_barb_goblins();
	void update_barb_orcs();
	void update_barb_skeletons();

	int UnitsNeeded_City_Invasion(int iRatio);
	int UnitsNeeded_NavalTransportUnits();

	int UnitsNeeded(int iUnitCategory = NO_UNITCATEGORY);
	bool isAllUnitsForNavalGroup();
	void loadIntoShip(CvUnit* pUnit);
	bool canFillWithReserveUnits(CvAIGroup* pReserve);
	int UnitPowerNeeded();
	int calculateEnemyStrength(CvPlot* pPlot,int iRange, bool bOwnerTeamOnly = false, bool bAttackOnly = false, bool bDefensiveBonuses = false) const;
	bool isAllUnitsWithinRange(CvPlot* pPlot, int iRange);
	int getUnitsTurnToPlot(CvPlot* pPlot);
	int getGroupPowerWithinRange(CvPlot* pPlot, int iMaxTurns, bool bCountWoundedAsFullHealth = true, bool bDefensiveBonuses = false) const;
	bool isPossibleAttacker(CvUnit* pUnit, CvPlot* pPlot, int iMaxTurns) const;
	int getNumPossibleAttackers(CvPlot* pPlot, int iMaxTurns) const;
	int UnitsNeeded_Counter();
	int UnitsNeeded_Lair_destory();
	int getNumWarwizardsNeeded();
	void RemoveWeakestUnit();
	CvUnit* getWeakestUnit();
	CvUnit* getWeakestUnit_CityDefense();
	CvUnit* getWeakestUnit_Settle();
	CvUnit* getCloseUnit(CvPlot* pTarget, CvAIGroup* pNewGroup, bool bSameArea=true);
	bool isUnitAllowed(CvUnit* pUnit, int iGroupType);
	bool isFull(CvUnit* pUnit);
	void setInitialUnitAI(CvUnit* pUnit);
	void compareAttackStrength(CvPlot* pTarget,int* iEnemyStrength = NULL,int* iOurStrength = NULL, bool bAttackNow = true);
	void compareInvasionPower(CvPlot* pTarget, int* piEnemyStrength, int* piOurStrength);
	void launchAttack(CvPlot* pTarget);
	void prepareAttack(CvPlot* pTarget, int iReinforceAmount, int iTurnsTillAttack);
	void attackAnyUnit(int iOddsThreshold);
	void HeroattackAnyUnit();
	void Move(CvPlot* pTarget);
	void Fortify();
	bool atPlot(CvPlot* pPlot) const;
	void pillage(int iBonusThreshold = 0);
	int pillagePlot(CvPlot* pTarget);
	void pillageAdjacentPlots(bool bIgnoreDanger = false);
	void healUnits(int iThreshold, CvPlot* pTarget = NULL);
	void checkForPossibleUpgrade();
	void AIGroup_Pillage(CvCity* pTarget,int iRange, bool bIgnoreDanger);
	void AIGroup_Siege(bool bCity, int iThreshold);
	void MoveSiegeToTarget(CvPlot* pTarget);
	void TargetSpell(CvPlot* pTarget, int iThreshold);
	void castAnyTarget(int iThreshold, bool bOtherArea = false);
	bool canMoveIntoPlot(const CvPlot* pTarget, bool bWater) const;

protected:

	// stuff necessary to communicate with rest of DLL
	int m_iID;
	PlayerTypes m_eOwner;
	// end

	AIGroupTypes m_eGroupType;
	bool m_bDeathDelayed;
	int m_iSeeInvisible;
	int m_iBarbPlunder;
	int m_iMissionCity;
	int m_iMissionArea;
	int m_iMissionBuild;
	int m_iMissionStatus;
	int m_iReserveGroup;
	int m_iSiegeGroup;
	int m_iHelpGroup1;
	int m_iHelpGroup2;
	int m_iHelpGroup3;
	int m_iMissionPlotX, m_iMissionPlotY;
	int m_iMissionTargetX, m_iMissionTargetY;
	int m_iMissionTargetUnit, m_iMissionTargetUnitOwner;
	CLinkList<IDInfo> m_units;
};

#endif
