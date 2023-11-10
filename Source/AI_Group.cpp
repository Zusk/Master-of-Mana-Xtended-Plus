// CombatAura.cpp

#include "CvGameCoreDLL.h"
#include "CvAIGroup.h"
#include "Hilfsfunktionen.h"

// Public Functions...


CvAIGroup::CvAIGroup()
{
	reset(0,NO_PLAYER,NO_AIGROUP,true);
}


CvAIGroup::~CvAIGroup()
{
	uninit();
}


void CvAIGroup::init(int iID, int iOwner, int iGroupType)
{
	/** DEBUG **/
	if(iGroupType!=NO_AIGROUP)
	{
		TCHAR szOut[1024];
		sprintf(szOut, "%S %d::init \n",GC.getAIGroupInfo((AIGroupTypes)iGroupType).getDescription(), iID);
		TCHAR szFile[1024];
		sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
		gDLL->logMsg(szFile,szOut, false, false);
	}
	/** DEBUG **/

/**
	FAssert(NO_COMBATAURA != iCombatAuraType);

	//--------------------------------
	// Init saved data
**/
	reset(iID,iOwner, iGroupType,false);
}

void CvAIGroup::uninit()
{
	m_units.clear();
}



// FUNCTION: reset()
// Initializes data members that are serialized.
void CvAIGroup::reset(int iID, int iOwner, int iGroupType, bool bConstructorCall)
{
	//--------------------------------
	// Uninit class
	uninit();

	m_iID = iID;
	m_eOwner = (PlayerTypes)iOwner;

	m_eGroupType = (AIGroupTypes)iGroupType;
	m_bDeathDelayed = false;
	m_iSeeInvisible = 0;
	m_iBarbPlunder = 0;
	m_iMissionPlotX, m_iMissionPlotY = -1;
	m_iMissionTargetX, m_iMissionTargetY = -1;
	m_iMissionCity = FFreeList::INVALID_INDEX;
	m_iMissionArea = FFreeList::INVALID_INDEX;
	m_iMissionBuild = NO_BUILD;
	m_iMissionStatus = NO_AIGROUP_STATUS;
	m_iReserveGroup = FFreeList::INVALID_INDEX;
	m_iSiegeGroup = FFreeList::INVALID_INDEX;
	m_iHelpGroup1 = FFreeList::INVALID_INDEX;
	m_iHelpGroup2 = FFreeList::INVALID_INDEX;
	m_iHelpGroup3 = FFreeList::INVALID_INDEX;
	m_iMissionTargetUnit = FFreeList::INVALID_INDEX;
	m_iMissionTargetUnitOwner = NO_PLAYER;

}

void CvAIGroup::kill()
{
	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "%S %d::kill \n",GC.getAIGroupInfo(getGroupType()).getDescription(), getID());
	TCHAR szFile[1024];
	sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
	gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	std::vector<int> aiUnitIDs;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit!=NULL && !pLoopUnit->isDelayedDeath())
		{
			aiUnitIDs.push_back(pLoopUnit->getID());
		}
	}

	for(std::vector<int>::iterator it=aiUnitIDs.begin() ; it < aiUnitIDs.end(); it++ )
	{
		GET_PLAYER(getOwnerINLINE()).getUnit(*it)->setAIGroup(NULL);
	}

	GET_PLAYER(getOwnerINLINE()).deleteAIGroup(getID());
}

int CvAIGroup::getID() const
{
	return m_iID;
}


int CvAIGroup::getIndex() const
{
	return (getID() & FLTA_INDEX_MASK);
}

void CvAIGroup::setID(int iID)
{
	m_iID = iID;
}

void CvAIGroup::read(FDataStreamBase* pStream)
{
	// Init data before load
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iID);
	pStream->Read((int*)&m_eOwner);	

	pStream->Read((int*)&m_eGroupType);
	pStream->Read(&m_iMissionCity);
	pStream->Read(&m_iMissionArea);
	pStream->Read(&m_iMissionBuild);
	pStream->Read(&m_iMissionStatus);
	pStream->Read(&m_iReserveGroup);
	pStream->Read(&m_iSiegeGroup);
	pStream->Read(&m_iHelpGroup1);
	pStream->Read(&m_iHelpGroup2);
	pStream->Read(&m_iHelpGroup3);
	pStream->Read(&m_iSeeInvisible);	
	pStream->Read(&m_iBarbPlunder);	
	pStream->Read(&m_bDeathDelayed);
	pStream->Read(&m_iMissionPlotX);
	pStream->Read(&m_iMissionPlotY);
	pStream->Read(&m_iMissionTargetX);
	pStream->Read(&m_iMissionTargetY);
	pStream->Read(&m_iMissionTargetUnit);
	pStream->Read(&m_iMissionTargetUnitOwner);

	m_units.Read(pStream);
}


void CvAIGroup::write(FDataStreamBase* pStream)
{
	uint uiFlag=2;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iID);
	pStream->Write(m_eOwner);

	pStream->Write(m_eGroupType);
	pStream->Write(m_iMissionCity);
	pStream->Write(m_iMissionArea);
	pStream->Write(m_iMissionBuild);
	pStream->Write(m_iMissionStatus);
	pStream->Write(m_iReserveGroup);
	pStream->Write(m_iSiegeGroup);
	pStream->Write(m_iHelpGroup1);
	pStream->Write(m_iHelpGroup2);
	pStream->Write(m_iHelpGroup3);
	pStream->Write(m_iSeeInvisible);	
	pStream->Write(m_iBarbPlunder);	
	pStream->Write(m_bDeathDelayed);
	pStream->Write(m_iMissionPlotX);
	pStream->Write(m_iMissionPlotY);
	pStream->Write(m_iMissionTargetX);
	pStream->Write(m_iMissionTargetY);
	pStream->Write(m_iMissionTargetUnit);
	pStream->Write(m_iMissionTargetUnitOwner);

	m_units.Write(pStream);
}

//
//
//
//
//

TeamTypes CvAIGroup::getTeam() const
{
	return GET_PLAYER(getOwnerINLINE()).getTeam();
}

void CvAIGroup::addUnit(CvUnit* pUnit)
{
	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "%S %d::add -- %S\n",GC.getAIGroupInfo(getGroupType()).getDescription(), getID(), GC.getUnitInfo(pUnit->getUnitType()).getDescription());
	TCHAR szFile[1024];
	sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
	gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	m_units.insertAtEnd(pUnit->getIDInfo());
	setInitialUnitAI(pUnit);
	changeSeeInvisible((pUnit->isSeeInvisible()) ? 1 : 0);
}

void CvAIGroup::removeUnit(CvUnit* pUnit)
{
	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "%S %d::remove -- %S\n",GC.getAIGroupInfo(getGroupType()).getDescription(), getID(), GC.getUnitInfo(pUnit->getUnitType()).getDescription());
	TCHAR szFile[1024];
	sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
	gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	changeSeeInvisible((pUnit->isSeeInvisible()) ? -1 : 0);

	CLLNode<IDInfo>* pUnitNode;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		if (::getUnit(pUnitNode->m_data) == pUnit)
		{
			deleteUnitNode(pUnitNode);
			break;
		}
		else
		{
			pUnitNode = nextUnitNode(pUnitNode);
		}
	}
}

CLLNode<IDInfo>* CvAIGroup::deleteUnitNode(CLLNode<IDInfo>* pNode)
{
	CLLNode<IDInfo>* pNextUnitNode;

	pNextUnitNode = m_units.deleteNode(pNode);

	return pNextUnitNode;
}

CLLNode<IDInfo>* CvAIGroup::nextUnitNode(CLLNode<IDInfo>* pNode) const
{
	return m_units.next(pNode);
}


int CvAIGroup::getNumUnits() const
{
	return m_units.getLength();
}

CLLNode<IDInfo>* CvAIGroup::headUnitNode() const
{
	return m_units.head();
}

CvUnit* CvAIGroup::getHeadUnit() const
{
	CLLNode<IDInfo>* pUnitNode = headUnitNode();

	if (pUnitNode != NULL)
	{
		return ::getUnit(pUnitNode->m_data);
	}
	else
	{
		return NULL;
	}
}

AIGroupTypes CvAIGroup::getGroupType()
{
	return m_eGroupType;
}

CvCity*	CvAIGroup::getMissionCity() const
{
	if(m_iMissionCity==FFreeList::INVALID_INDEX)
		return NULL;

	return GET_PLAYER(getOwnerINLINE()).getCity(m_iMissionCity);
}

void CvAIGroup::setMissionCity(CvCity* pNewCity)
{
	if(pNewCity==NULL)
	{
		m_iMissionCity=FFreeList::INVALID_INDEX;
	}
	else
	{
		m_iMissionCity=pNewCity->getID();
	}
}

void CvAIGroup::setMissionArea(int iNewArea)
{
	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "%S %d::setMissionArea -- %d\n",GC.getAIGroupInfo(getGroupType()).getDescription(), getID(),iNewArea);
	TCHAR szFile[1024];
	sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
	gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	m_iMissionArea=iNewArea;
}

int CvAIGroup::getMissionArea()
{
	return m_iMissionArea;
}

void CvAIGroup::setMissionBuild(int iNewBuild)
{
	/** DEBUG **/
	/**
	TCHAR szOut[1024];
	sprintf(szOut, "%S %d::setMissionBuild -- %d\n",GC.getAIGroupInfo(getGroupType()).getDescription(), getID(),iNewBuild);
	TCHAR szFile[1024];
	sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
	gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	m_iMissionBuild=iNewBuild;
}

int CvAIGroup::getMissionBuild()
{
	return m_iMissionBuild;
}

void CvAIGroup::setMissionStatus(int iNewStatus)
{
	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "%S %d::setMissionStatus -- %d\n",GC.getAIGroupInfo(getGroupType()).getDescription(), getID(),iNewStatus);
	TCHAR szFile[1024];
	sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
	gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	m_iMissionStatus=iNewStatus;
}

int CvAIGroup::getMissionStatus()
{
	return m_iMissionStatus;
}

CvAIGroup* CvAIGroup::getReserveGroup()
{
	if(m_iReserveGroup==FFreeList::INVALID_INDEX)
		return NULL;

	return GET_PLAYER(getOwnerINLINE()).getAIGroup(m_iReserveGroup);
}

void CvAIGroup::setReserveGroup(CvAIGroup* pNewGroup)
{
	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "CvAIGroup %d,%d::setReserveGroup -- \n",getOwnerINLINE(),getID());
		TCHAR szFile[1024];
		sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
		gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	if(pNewGroup==NULL)
	{
		m_iReserveGroup=FFreeList::INVALID_INDEX;
	}
	else
	{
		m_iReserveGroup=pNewGroup->getID();
	}
}

CvAIGroup* CvAIGroup::getSiegeGroup()
{
	if(m_iSiegeGroup==FFreeList::INVALID_INDEX)
		return NULL;

	return GET_PLAYER(getOwnerINLINE()).getAIGroup(m_iSiegeGroup);
}

void CvAIGroup::setSiegeGroup(CvAIGroup* pNewGroup)
{
	if(pNewGroup==NULL)
	{
		m_iSiegeGroup=FFreeList::INVALID_INDEX;
	}
	else
	{
		m_iSiegeGroup=pNewGroup->getID();
	}
}

CvAIGroup* CvAIGroup::getHelpGroup1()
{
	if(m_iHelpGroup1==FFreeList::INVALID_INDEX)
		return NULL;

	return GET_PLAYER(getOwnerINLINE()).getAIGroup(m_iHelpGroup1);
}

void CvAIGroup::setHelpGroup1(CvAIGroup* pNewGroup)
{
	if(pNewGroup==NULL)
	{
		m_iHelpGroup1=FFreeList::INVALID_INDEX;
	}
	else
	{
		m_iHelpGroup1=pNewGroup->getID();
	}
}

CvAIGroup* CvAIGroup::getHelpGroup2()
{
	if(m_iHelpGroup2==FFreeList::INVALID_INDEX)
		return NULL;

	return GET_PLAYER(getOwnerINLINE()).getAIGroup(m_iHelpGroup2);
}

void CvAIGroup::setHelpGroup2(CvAIGroup* pNewGroup)
{

	if(pNewGroup==NULL)
	{
		m_iHelpGroup2=FFreeList::INVALID_INDEX;
	}
	else
	{
		m_iHelpGroup2=pNewGroup->getID();
	}
}

CvAIGroup* CvAIGroup::getHelpGroup3()
{
	if(m_iHelpGroup3==FFreeList::INVALID_INDEX)
		return NULL;

	return GET_PLAYER(getOwnerINLINE()).getAIGroup(m_iHelpGroup3);
}

void CvAIGroup::setHelpGroup3(CvAIGroup* pNewGroup)
{

	if(pNewGroup==NULL)
	{
		m_iHelpGroup3=FFreeList::INVALID_INDEX;
	}
	else
	{
		m_iHelpGroup3=pNewGroup->getID();
	}
}

CvUnit* CvAIGroup::getMissionTargetUnit()
{
	if(m_iMissionTargetUnit==FFreeList::INVALID_INDEX || m_iMissionTargetUnitOwner==NO_PLAYER || !GET_PLAYER((PlayerTypes)m_iMissionTargetUnitOwner).isAlive())
	{
		return NULL;
	}
	else
	{
		return GET_PLAYER((PlayerTypes)m_iMissionTargetUnitOwner).getUnit(m_iMissionTargetUnit);
	}
}
void CvAIGroup::setMissionTargetUnit(CvUnit* pTargetUnit)
{
	if (pTargetUnit==NULL)
	{
		m_iMissionTargetUnit=FFreeList::INVALID_INDEX;
		m_iMissionTargetUnitOwner=NO_PLAYER;
	}
	else
	{
		m_iMissionTargetUnit=pTargetUnit->getID();
		m_iMissionTargetUnitOwner=pTargetUnit->getOwnerINLINE();
	}
}

CvPlot* CvAIGroup::getMissionPlot() const
{
	if(m_iMissionPlotX==-1 && m_iMissionPlotY==-1)
		return NULL;

	return GC.getMapINLINE().plot(m_iMissionPlotX,m_iMissionPlotY);
}

void CvAIGroup::setMissionPlot(CvPlot* pNewPlot)
{
	if(pNewPlot==NULL)
	{
		m_iMissionPlotX=-1;
		m_iMissionPlotY=-1;
	}
	else
	{
		m_iMissionPlotX=pNewPlot->getX_INLINE();
		m_iMissionPlotY=pNewPlot->getY_INLINE();
	}
}

CvPlot* CvAIGroup::getMissionTarget() const
{
	if(m_iMissionTargetX==-1 && m_iMissionTargetY==-1)
		return NULL;

	return GC.getMapINLINE().plot(m_iMissionTargetX,m_iMissionTargetY);
}

void CvAIGroup::setMissionTarget(CvPlot* pNewPlot)
{
	if(pNewPlot==NULL)
	{
		m_iMissionTargetX=-1;
		m_iMissionTargetY=-1;
	}
	else
	{
		m_iMissionTargetX=pNewPlot->getX_INLINE();
		m_iMissionTargetY=pNewPlot->getY_INLINE();
	}
}

bool CvAIGroup::isDeathDelayed() const
{
	return m_bDeathDelayed;
}
	
void CvAIGroup::setDeathDelayed(bool bNewValue)
{
	m_bDeathDelayed=bNewValue;
}

bool CvAIGroup::isSeeInvisible() const
{
	return (m_iSeeInvisible>0);
}

void CvAIGroup::changeSeeInvisible(int iChange)
{
	m_iSeeInvisible+=iChange;
}


CvUnit* CvAIGroup::getCloseUnit(CvPlot* pTarget, CvAIGroup* pNewGroup, bool bSameArea)
{
	int iBestValue=-MAX_INT;
	int iValue;
	int iPathTurns;
	int difX,difY;
	CvUnit* pBestUnit=NULL;

	if(pTarget == NULL || pNewGroup == NULL)
		return NULL;

	for(int iPass=0; iPass<2; iPass++)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			if(!bSameArea || pLoopUnit->getArea()==pTarget->getArea())
			{
				if(isUnitAllowed(pLoopUnit,pNewGroup->getGroupType()) && !pNewGroup->isFull(pLoopUnit))
				{
					difX=pTarget->getX_INLINE()-pLoopUnit->getX_INLINE();
					difY=pTarget->getY_INLINE()-pLoopUnit->getY_INLINE();
					if(iPass>0 || difX*difX+difY*difY<200)
					{
						iPathTurns=50;
						if(pLoopUnit->plot()==pTarget || (pTarget->getTeam()!=getTeam() && pLoopUnit->baseCombatStr()==0) ||
						(pTarget->isOwned() && pTarget->getTeam()!=getTeam() && GET_TEAM(getTeam()).canDeclareWar(pTarget->getTeam()))
							|| pLoopUnit->generatePath(pTarget,0,false,&iPathTurns))
						{

							if(pTarget->isVisibleEnemyUnit(getOwnerINLINE()))
							{
								iValue=pLoopUnit->AI_attackOdds(pTarget,false);
							}
							else
							{
								iValue=100;
							}

							//Bonus for Units that can see invisible
							if(getMissionTargetUnit()!=NULL && getMissionTargetUnit()->isInvisible(getTeam(),false))
							{
								if(!isSeeInvisible() && pLoopUnit->isSeeInvisible())
								{
									iValue=1000;
								}
							}

							//Siege Units
							if(pNewGroup!=NULL && pNewGroup->getNumUnitCategoryUnits(UNITCATEGORY_SIEGE)<3)
							{
								if(GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_SIEGE))
								{
									iValue=2000;
								}
							}

							//Wizards
							if(pNewGroup!=NULL && pNewGroup->getNumWarwizardsNeeded()>0)
							{
								if(GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
								{
									iValue=2000;
								}
							}

							//Naval Units
							if(pNewGroup!=NULL && (pNewGroup->getGroupType()==AIGROUP_NAVAL_SETTLE
													||  pNewGroup->getGroupType()==AIGROUP_NAVAL_INVASION))
							{
								/**
								if(pNewGroup->UnitsNeeded(UNITCATEGORY_SETTLE)>0)
								{
									iValue+=10000 * (GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_SETTLE) ? 1 : -1);
								}
								if(pNewGroup->UnitsNeeded(UNITCATEGORY_COMBAT)>0)
								{
									iValue+=5000 * (GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_COMBAT) ? 1 : -1);
								}
								if(pNewGroup->UnitsNeeded(UNITCATEGORY_COMBAT)>0)
								{
									iValue+=5000 * (GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_COMBAT) ? 1 : -1);
								}
								**/
								if(pNewGroup->UnitsNeeded_NavalTransportUnits()>0)
								{
									int iFreeCargo=pLoopUnit->getUnitInfo().getCargoSpace()-pLoopUnit->getCargo();
									if(iFreeCargo>0)
									{
										iValue+=10000*iFreeCargo;
									}
								}
							}

							if(pNewGroup!=NULL && pNewGroup->getGroupType()==AIGROUP_MANA_NODE)
							{
								if(GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
								{
									iValue=2000;
								}				
								else
								{
									iValue=-MAX_INT;
								}
							}

							iValue/=iPathTurns+1;
							if(pLoopUnit->getDamage()>0 && pLoopUnit->getGroup()->getActivityType()==ACTIVITY_HEAL)
							{
								iValue/=1000;
							}

							if(iValue>iBestValue)
							{
								iBestValue=iValue;
								pBestUnit=pLoopUnit;
							}
						}
					}
				}
			}
		}
		if(pBestUnit!=NULL)
		{
			return pBestUnit;
		}
	}
	return pBestUnit;
}

bool CvAIGroup::isFull(CvUnit* pUnit)
{
	CvPlayer& kPlayer=GET_PLAYER(getOwnerINLINE());

	switch (getGroupType())
	{
		case AIGROUP_NAVAL_SETTLE:
		case AIGROUP_NAVAL_INVASION:
			if(UnitsNeeded(UNITCATEGORY_SETTLE)<=0)
			{
				if(kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_SETTLE))
				{
					return true;
				}
			}
			if(UnitsNeeded(UNITCATEGORY_COMBAT)<=0)
			{
				if(kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_COMBAT))
				{
					return true;
				}
			}
			if(UnitsNeeded(UNITCATEGORY_TRANSPORT_NAVAL)<=0)
			{
				if(UnitsNeeded(UNITCATEGORY_COMBAT_NAVAL)<=0 || !kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_COMBAT_NAVAL))
				{
					if(kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_TRANSPORT_NAVAL))
					{
						return true;
					}
				}
			}
			if(UnitsNeeded(UNITCATEGORY_COMBAT_NAVAL)<=0)
			{
				if(UnitsNeeded(UNITCATEGORY_TRANSPORT_NAVAL)<=0 || !kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_TRANSPORT_NAVAL))
				{
					if(kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_COMBAT_NAVAL))
					{
						return true;
					}
				}
			}
			break;

		default:
			break;
	}

	return false;
}

bool CvAIGroup::isUnitAllowed(CvUnit* pUnit, int iGroupType)
{
	CvUnitInfo& kUnitInfo = GC.getUnitInfo(pUnit->getUnitType());
	CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());

	switch(iGroupType)
	{
		case AIGROUP_NAVAL_RESERVE:
		case AIGROUP_NAVAL_COUNTER:
		case AIGROUP_NAVAL_EXPLORER:
			if (pUnit->getDomainType() != DOMAIN_SEA)
				return false;

		//can have both land and sea units
		case AIGROUP_NAVAL_SETTLE:
		case AIGROUP_NAVAL_INVASION:
			break;

		default:
			if (pUnit->getDomainType() != DOMAIN_LAND)
				return false;
	}

	switch (iGroupType)
	{
		case AIGROUP_CITY_INVASION:
		case AIGROUP_SIEGE:
		case AIGROUP_CITY_INVASION_PILLAGE:
		case AIGROUP_COUNTER:
		case AIGROUP_DESTROY_LAIR:
		case AIGROUP_EXPLORE:
		case AIGROUP_CITY_DEFENSE:
		case AIGROUP_SETTLE:

			if (pUnit->isHiddenNationality())
			{
				return false;
			}

			switch (pUnit->AI_getUnitAIType())
			{
				case UNITAI_SETTLE:
				case UNITAI_WORKER:
				case UNITAI_TERRAFORMER:
				case UNITAI_MANA_UPGRADE:
				case UNITAI_MISSIONARY:
				case UNITAI_ANIMAL:
				case UNITAI_PROPHET:
				case UNITAI_ARTIST:
				case UNITAI_SCIENTIST:
				case UNITAI_GENERAL:
				case UNITAI_MERCHANT:
				case UNITAI_ENGINEER:
				case UNITAI_FEASTING:
				case UNITAI_SETTLER_SEA:
				case UNITAI_WORKER_SEA:
				case UNITAI_ATTACK_SEA:
				case UNITAI_RESERVE_SEA:
				case UNITAI_ESCORT_SEA:
				case UNITAI_ASSAULT_SEA:
				case UNITAI_MISSIONARY_SEA:
				case UNITAI_SPY_SEA:
				case UNITAI_PIRATE_SEA:
					return false;
					break;
				default:
					break;
			}

			break;

		case AIGROUP_WORKER:
			if(pUnit->AI_getUnitAIType()!=UNITAI_WORKER)
			{
				return false;
			}
			break;
		case AIGROUP_MANA_NODE:
			if(pUnit->getUnitCombatType() != GC.getUNITCOMBAT_ADEPT())
				return false;
			break;
		case AIGROUP_BARB_PLUNDER:
		case AIGROUP_BARB_NEW_LAIR:
		case AIGROUP_BARB_GOBLINS:
		case AIGROUP_BARB_SKELETONS:
		case AIGROUP_BARB_ORCS:
		case AIGROUP_BARB_MERCENARIES:
		case AIGROUP_BARB_HILLGIANTS:
		case AIGROUP_BARB_WEREWOLFS:
			if(pUnit->isBarbarian())
			{
				return true;
			}
			else
			{
				return false;
			}
			break;
		default:
			break;
	}

	//Some Groups shouldn't have Tier4 Units
	switch (iGroupType)
	{
		case AIGROUP_EXPLORE:
		case AIGROUP_CITY_DEFENSE:
		case AIGROUP_CITY_INVASION_PILLAGE:
		case AIGROUP_SETTLE:
		case AIGROUP_NAVAL_SETTLE:
			if (pUnit->getDuration()>0)
			{
				return false;
			}
			if(pUnit->isSummon())
			{
				return false;
			}
			if(kUnitInfo.isImmortal()) {
				return false;
			}
			if(pUnit->AI_getUnitAIType()==UNITAI_HERO)
			{
				return false;
			}

			if(iGroupType!=AIGROUP_SETTLE &&( kUnitInfo.getManaUpkeep()>0 || kUnitInfo.getFaithUpkeep()>0))
			{
				return false;
			}

			if(kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_SIEGE)
				|| kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
			{
				return false;
			}

			break;
		
		case AIGROUP_DESTROY_LAIR:
			if(kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_SIEGE)
				|| kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
			{
				return false;
			}
			break;
		default:
			break;
	}

	if(iGroupType==AIGROUP_CITY_DEFENSE)
	{
		if (kUnitInfo.isAIblockPermDefense())
		{
			return false;
		}

		//block scouts because of early exploration
		if(pUnit->AI_getUnitAIType()==UNITAI_EXPLORE && kUnitInfo.getTier()==1)
		{
			return false;
		}

		if(!kUnitInfo.isMilitaryHappiness())
		{
			int iCount=0;
			for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
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

	//only allow Siege units in Siege Groups
	if(iGroupType==AIGROUP_SIEGE)
	{
		if(!kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_SIEGE))
		{
			return false;
		}
	}

	if(kPlayer.AI_isUnitCategory(pUnit->getUnitType(),UNITCATEGORY_SIEGE))
	{
		if(iGroupType==AIGROUP_CITY_INVASION || iGroupType==AIGROUP_CITY_INVASION_PILLAGE)
		{
			return false;
		}
	}

    return true;
}

void CvAIGroup::setInitialUnitAI(CvUnit* pUnit)
{
	if(pUnit==NULL)
	{
		return;
	}

	pUnit->AI_setGroupflag(GROUPFLAG_NONE);

	//remove invalid PromotionSpecialization
	if(pUnit->AI_getPromotionSpecialization()!=NO_AIPROMOTIONSPECIALIZATION &&
		GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)pUnit->AI_getPromotionSpecialization()).getAIGroup()!=getGroupType())
	{
		pUnit->AI_setPromotionSpecialization(NO_AIPROMOTIONSPECIALIZATION);
	}

	if(pUnit->AI_getPromotionSpecialization()==NO_AIPROMOTIONSPECIALIZATION)
	{
		for(int iI=0;iI<GC.getNumAIPromotionSpecializationInfos();iI++)
		{
			if(GC.getAIPromotionSpecializationInfo((AIPromotionSpecializationTypes)iI).getAIGroup()==getGroupType())
			{
				pUnit->AI_setPromotionSpecialization((AIPromotionSpecializationTypes)iI);
				pUnit->AI_promote();
				break;
			}
		}
	}
}

void CvAIGroup::compareAttackStrength(CvPlot* pTarget, int* piEnemyStrength, int* piOurStrength, bool bAttackNow)
{
	*piEnemyStrength=GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(pTarget,0,true,false);
	*piOurStrength=0;

	int iPathTurns;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canMove() && pLoopUnit->canAttack())
		{
			if(stepDistance(pLoopUnit->getX_INLINE(),pLoopUnit->getY_INLINE(),pTarget->getX_INLINE(),pTarget->getY_INLINE())<10)
			{
				if(pLoopUnit->generatePath(pTarget,0,false,&iPathTurns))
				{
					if(!bAttackNow || iPathTurns<=1)
					{
						*piOurStrength+=pLoopUnit->currEffectiveStr(NULL, NULL);
					}
				}
			}
		}
	}
}

void CvAIGroup::compareInvasionPower(CvPlot* pTarget, int* piEnemyStrength, int* piOurStrength)
{
	*piEnemyStrength=0;
	*piOurStrength=0;

	*piOurStrength=getGroupPowerWithinRange(pTarget,1,false);

	*piEnemyStrength=calculateEnemyStrength(pTarget,0,true,false,true);
}

void CvAIGroup::prepareAttack(CvPlot* pTarget, int iReinforceAmount, int iTurnsTillAttack)
{
	//Heal Units first
	healUnits(20);

	int iReinforceCounter=0;
	int iPathTurns;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canMove() && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
		{
			if(pLoopUnit->generatePath(pTarget,0,true,&iPathTurns))
			{
				if(iPathTurns>iTurnsTillAttack)
				{
					iReinforceCounter+=pLoopUnit->maxCombatStr(NULL,NULL);
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot() ->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
				}

				if(iPathTurns<=iTurnsTillAttack)
				{
					if(iTurnsTillAttack<=1)
					{
						//ToDo: Write so that only Route is used which can be used by Player
						if(pLoopUnit->plot()->isRoute() && !pLoopUnit->plot()->isCity())
						{
							CvPlot* pBetterPlot=pLoopUnit->adjacentHigherDefensePlot(false,true);
							if(pBetterPlot!=NULL)
							{
								pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pBetterPlot->getX_INLINE(), pBetterPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
							}
						}
					}
					//check if we might retreat to more safety
					else if(iPathTurns<iTurnsTillAttack)
					{
						if(GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pLoopUnit->plot()))
						{
							CvPlot* pBestPlot=NULL;
							int iValue;
							int iBestValue=MAX_INT;
							for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
							{
								CvPlot* pLoopPlot = plotDirection(pLoopUnit->plot()->getX_INLINE(), pLoopUnit->plot()->getY_INLINE(), ((DirectionTypes)iI));
								if(pLoopPlot!=NULL && !pLoopPlot->isVisibleEnemyDefender(pLoopUnit) && pLoopUnit->canMoveInto(pLoopPlot))
								{
									if(!GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pLoopPlot))
									{
										iValue=stepDistance(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),pTarget->getX_INLINE(),pTarget->getY_INLINE());
										if(iValue<iBestValue)
										{
											iBestValue=iValue;
											pBestPlot=pLoopPlot;
										}
									}
								}
							}
							if(pBestPlot!=NULL)
							{
								pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
							}
						}

						//check if adjacent plots can be pillaged without danger
						pillageAdjacentPlots(false);
					}
				}

				if(iReinforceCounter>iReinforceAmount)
				{
					return;
				}
			}
		}
	}

	//move Warwizards closer to the target
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canMove() && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
		{
			if(pLoopUnit->isWarwizard() &&!pLoopUnit->isWarwizardinPosition(pTarget))
			{
				int iValue=0;
				int iBestValue=0;
				CvPlot* pBestPlot=NULL;
				int iRange=pLoopUnit->getArcaneRange();
				for(int iX=-iRange;iX<=iRange;iX++)
				{
					for(int iY=-iRange;iY<=iRange;iY++)
					{
						CvPlot* pLoopPlot=plotXY(pTarget->getX_INLINE(),pTarget->getY_INLINE(),iX,iY);
						if(pLoopPlot!=NULL && !pLoopPlot->isVisibleEnemyUnit(pLoopUnit))
						{
							if(pLoopPlot->getArea()==pTarget->getArea())
							{
								iValue=100*stepDistance(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),pLoopUnit->getX_INLINE(),pLoopUnit->getY_INLINE());
								iValue+=50*pLoopPlot->getNumUnits();
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
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
				}
			}
		}
	}
}

void CvAIGroup::launchAttack(CvPlot* pTarget)
{
	//range Attack First

	//Cast Spells
	TargetSpell(pTarget, 0);

	//Charge!
	CLinkList<IDInfo> tempUnitCycle;
	CLinkList<IDInfo> finalAttackCycle;
	int iPathTurns;

	tempUnitCycle.clear();
	finalAttackCycle.clear();

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canMove() && pLoopUnit->canAttack())
		{
			if(pLoopUnit->generatePath(pTarget, 0, false, &iPathTurns))
			{
				if(iPathTurns <= 1)
				{
					tempUnitCycle.insertAtEnd(pUnitNode->m_data);
				}
			}
		}
	}

	while(tempUnitCycle.getLength()>0)
	{
		int iBestValue = -MAX_INT;
		CLLNode<IDInfo>* pBestNode;
		for (CLLNode<IDInfo>* pUnitNode = tempUnitCycle.head(); pUnitNode != NULL; pUnitNode = tempUnitCycle.next(pUnitNode))
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			int iValue = pLoopUnit->AI_attackOdds(pTarget,true);
			if (iValue >= 99)
			{
				iValue = MAX_INT;
			}
			else if(pLoopUnit->isHasSecondChance())
			{
				iValue = MAX_INT/2;
			}
			else
			{
				iValue = MAX_INT/2-std::max(10,((100-iValue)*pLoopUnit->getPower()*(10+pLoopUnit->getLevel()*3)));
			}
			if(iValue >= iBestValue)
			{
				iBestValue = iValue;
				pBestNode = pUnitNode;
			}
		}

		finalAttackCycle.insertAtEnd(pBestNode->m_data);
		tempUnitCycle.deleteNode(pBestNode);
	}

	int iCount=0;
	for (CLLNode<IDInfo>* pUnitNode = finalAttackCycle.head(); pUnitNode != NULL; pUnitNode = finalAttackCycle.next(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		iCount++;
		if(pTarget->isVisibleEnemyDefender(pLoopUnit) || pTarget->getTeam()!=getTeam())
		{
			pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pTarget->getX_INLINE(), pTarget->getY_INLINE(), MOVE_DIRECT_ATTACK);
		}
		else 
		{
			if((pTarget->isCity() && pTarget->getTeam()==getTeam() && pTarget->getNumDefenders(getOwnerINLINE())<3)
				|| iCount<3)
			{
				pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pTarget->getX_INLINE(), pTarget->getY_INLINE(), MOVE_DIRECT_ATTACK);
			}
		}
	}
}

void CvAIGroup::pillage(int iBonusThreshold)
{
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
		if(pLoopUnit->canMove() && pLoopUnit->canAttack())
		{
			pLoopUnit->AI_pillageRange(pLoopUnit->baseMoves(),iBonusThreshold);
		}
	}
}

int CvAIGroup::pillagePlot(CvPlot* pTarget)
{
	if(pTarget == NULL)
		return 0;

	int iCounter = 0;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode)) {
		CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));

		if((pLoopUnit->plot() == pTarget) && pLoopUnit->canMove()
			&& pLoopUnit->canPillage(pTarget)) {
			pLoopUnit->getGroup()->pushMission(MISSION_PILLAGE);
			++iCounter;
		}
	}

	return iCounter;
}

void CvAIGroup::pillageAdjacentPlots(bool bIgnoreDanger)
{
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
		if(pLoopUnit->canMove() && pLoopUnit->canAttack())
		{
			CvPlot* pPlot=pLoopUnit->plot();
			if(pPlot->isOwned() && (!pPlot->isBarbarian()) && pLoopUnit->isEnemy(pPlot->getTeam()))
			{
				if(pLoopUnit->canPillage(pPlot))
				{
					pLoopUnit->pillage();
					continue;
				}
			}

			for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				CvPlot* pAdjacentPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), (DirectionTypes) iI);
				if (pAdjacentPlot != NULL)
				{
					if(pAdjacentPlot->isOwned() && (!pAdjacentPlot->isBarbarian()) && pLoopUnit->isEnemy(pAdjacentPlot->getTeam()))
					{
						if(!pAdjacentPlot->isVisibleEnemyDefender(pLoopUnit))
						{
							if(bIgnoreDanger || (!GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pAdjacentPlot)))
							{
								if(pLoopUnit->canPillage(pAdjacentPlot))
								{
									pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
									pLoopUnit->pillage();
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

int CvAIGroup::UnitsNeeded(int iUnitCategory)
{
	int iUnitsAllowed=-1;
	switch(getGroupType())
	{
		case AIGROUP_CITY_DEFENSE:
			iUnitsAllowed=getMissionCity()->AI_neededPermDefense();
			break;
		case AIGROUP_SETTLE:
			if(getHeadUnit() == NULL)
			{
				iUnitsAllowed = 0;
			}
			else if(getMissionPlot() == NULL
				|| GET_PLAYER(getOwnerINLINE()).isLackMoneyForExpansion())
			{
				iUnitsAllowed = 2;
			}
			else
			{
				iUnitsAllowed=getHeadUnit()->AI_getSettlerPatrolUnitsNeeded();
			}
			break;
		case AIGROUP_CITY_INVASION:
			switch(iUnitCategory)
			{
				case UNITCATEGORY_WARWIZARD:
					if(GET_PLAYER(getOwnerINLINE()).AI_getMagicStrategy()==STRATEGY_MAGIC_EVOCATION)
					{
						return 3-getNumUnitCategoryUnits(UNITCATEGORY_WARWIZARD);
					}
					return 0;
				default:
					iUnitsAllowed=getNumUnits()+UnitsNeeded_City_Invasion(150);
			}
			break;
		case AIGROUP_CITY_INVASION_PILLAGE:
			iUnitsAllowed=1;
			break;
		case AIGROUP_SIEGE:
			iUnitsAllowed=6;
			break;
		case AIGROUP_COUNTER:
		case AIGROUP_NAVAL_COUNTER:
			iUnitsAllowed=getNumUnits()+UnitsNeeded_Counter();
			break;
		case AIGROUP_DESTROY_LAIR:
			iUnitsAllowed=getNumUnits()+UnitsNeeded_Lair_destory();
			break;
		case AIGROUP_WORKER:
			if(getMissionPlot()==NULL)
				iUnitsAllowed=-1;
			else
				iUnitsAllowed=getNumUnits()+1;
			break;
		case AIGROUP_MANA_NODE:
		case AIGROUP_NAVAL_EXPLORER:
			iUnitsAllowed=1;
			break;
		case AIGROUP_NAVAL_SETTLE:
			if(getMissionStatus()==STATUS_NAVAL_IN_TRANSIT)
			{
				iUnitsAllowed=getNumUnits();
			}
			else
			{
				iUnitsAllowed=getNumUnits();
				switch(iUnitCategory)
				{
					case UNITCATEGORY_SETTLE:
						if(getNumUnitCategoryUnits(UNITCATEGORY_SETTLE)==0)
						{
							iUnitsAllowed++;
						}
						break;
					case UNITCATEGORY_COMBAT:
						iUnitsAllowed+=std::max(0,3-getNumUnitCategoryUnits(UNITCATEGORY_COMBAT));
						break;
					case UNITCATEGORY_TRANSPORT_NAVAL:
						iUnitsAllowed+=UnitsNeeded_NavalTransportUnits();				
						break;
					default:
						break;
				}
//				iUnitsAllowed+=std::max(0,2-getNumNavalCombatUnits());
			}
			break;
		case AIGROUP_NAVAL_INVASION:
			//TODO: add logic for reinforcements
			if(getMissionStatus()==STATUS_NAVAL_IN_TRANSIT)
			{
				iUnitsAllowed=getNumUnits();
			}
			else
			{
				iUnitsAllowed=getNumUnits();
				switch(iUnitCategory)
				{
					case UNITCATEGORY_COMBAT:
						iUnitsAllowed+=UnitsNeeded_City_Invasion(150);
						break;
					case UNITCATEGORY_TRANSPORT_NAVAL:
						iUnitsAllowed+=UnitsNeeded_NavalTransportUnits();				
						break;
					case UNITCATEGORY_COMBAT_NAVAL:
						iUnitsAllowed+=std::max(0,3-getNumUnitCategoryUnits(UNITCATEGORY_COMBAT_NAVAL));				
						break;
					default:
						break;
				}
//				iUnitsAllowed+=std::max(0,2-getNumNavalCombatUnits());
			}
			break;				

		case AIGROUP_RESERVE:
		default:
			break;
	}

	if(iUnitsAllowed==-1)
	{
		return 0;
	}

	return iUnitsAllowed-getNumUnits();
}

int CvAIGroup::UnitsNeeded_Counter()
{
	if(getMissionTargetUnit()==NULL)
	{
		return 0;
	}

	int iEnemyStrength=0;
	int iOurStrength=0;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canAttack())
		{
			iOurStrength+=pLoopUnit->maxCombatStr(NULL, NULL);
		}
	}

	for(CLLNode<IDInfo>* pUnitNode=getMissionTargetUnit()->plot()->headUnitNode(); pUnitNode != NULL; pUnitNode = getMissionTargetUnit()->plot()->nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->isEnemy(getTeam()))
		{
			iEnemyStrength+=pLoopUnit->maxCombatStr(pLoopUnit->plot(), NULL);
		}
	}

	iEnemyStrength=(int)(iEnemyStrength*1.2);

	if(iEnemyStrength<iOurStrength)
	{
		return 0;
	}

	if (iOurStrength==0)
	{
		return 3;
	}
	return std::max(1,(iEnemyStrength-iOurStrength)/(iOurStrength/std::max(1,getNumUnits())));
}

int CvAIGroup::UnitsNeeded_Lair_destory()
{
	if(getMissionPlot()==NULL)
	{
		return 0;
	}

	int iEnemyStrength=0;
	int iOurStrength=0;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canAttack())
		{
			iOurStrength+=pLoopUnit->maxCombatStr(NULL, NULL);
		}
	}

	for(CLLNode<IDInfo>* pUnitNode=getMissionPlot()->headUnitNode(); pUnitNode != NULL; pUnitNode = getMissionPlot()->nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->isEnemy(getTeam()))
		{
			iEnemyStrength+=pLoopUnit->maxCombatStr(pLoopUnit->plot(), NULL);
		}
	}

	iEnemyStrength=(int)(iEnemyStrength*1.4);

	return (iEnemyStrength*12>iOurStrength*10) ? 3 : 0;
}

int CvAIGroup::UnitsNeeded_City_Invasion(int iRatio)
{
	if(getMissionTarget()==NULL)
	{
		return 0;
	}

	int iPossibleAttackers=getNumPossibleAttackers(NULL,MAX_INT);

	if(iPossibleAttackers>30)
	{
		return 0;
	}

	//use atleast 5 units
	if(iPossibleAttackers<5)
	{
		return 5;
	}

	int iEnemyStrength=calculateEnemyStrength(getMissionTarget(), 2,true,false,true);
	iEnemyStrength*=iRatio;
	iEnemyStrength/=100;

	int iOurStrength=getGroupPowerWithinRange(getMissionTarget(),MAX_INT);

	if(iOurStrength==0)
	{
		return 5;
	}

	if(iEnemyStrength<iOurStrength)
	{
		return 0;
	}

	return std::max(1,(iEnemyStrength-iOurStrength)/(iOurStrength/std::max(1,iPossibleAttackers)));
}

int CvAIGroup::UnitsNeeded_NavalTransportUnits()
{
	int iUnitsToTransport=0;
	int iCargoSpaceAvailable=0;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
		if(!pLoopUnit->isCargo())
		{
			if(pLoopUnit->getDomainType()==DOMAIN_LAND)
			{
				iUnitsToTransport++;
			}
			if(pLoopUnit->getDomainType()==DOMAIN_SEA)
			{
				iCargoSpaceAvailable+=pLoopUnit->getUnitInfo().getCargoSpace()-pLoopUnit->getCargo();
			}
		}
	}

	return std::max(0,(iUnitsToTransport-iCargoSpaceAvailable+2)/3);
}

int CvAIGroup::getNumWarwizardsNeeded()
{
	switch(getGroupType())
	{
		case AIGROUP_CITY_INVASION:
		case AIGROUP_COUNTER:
		case AIGROUP_DESTROY_LAIR:
			return 3-getNumUnitCategoryUnits(UNITCATEGORY_WARWIZARD);
		default:
			break;
	}
	return 0;
}

void CvAIGroup::RemoveWeakestUnit()
{
	getWeakestUnit()->setAIGroup(NULL);
}

//which Unit do we need the least
CvUnit* CvAIGroup::getWeakestUnit()
{
	switch(getGroupType())
	{
		case AIGROUP_CITY_DEFENSE:
			return getWeakestUnit_CityDefense();			
		case AIGROUP_SETTLE:
			return getWeakestUnit_Settle();			
		default:
			return getHeadUnit();
	}
}

CvUnit* CvAIGroup::getWeakestUnit_CityDefense()
{
	int iBestValue=MAX_INT;
	int iValue;
	CvUnit* pBestUnit=NULL;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		iValue=pLoopUnit->currCombatStr(getMissionCity()->plot(), NULL);
		iValue*=100+GC.getUnitInfo(pLoopUnit->getUnitType()).getCityDefenseModifier();
		iValue/=100;
		if(iValue<iBestValue)
		{
			iBestValue=iValue;
			pBestUnit=pLoopUnit;
		}
	}
	return pBestUnit;
}

CvUnit* CvAIGroup::getWeakestUnit_Settle()
{
	int iBestValue=MAX_INT;
	int iValue;
	CvUnit* pBestUnit=NULL;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		iValue=pLoopUnit->currCombatStr(pLoopUnit->plot(), NULL);
		iValue*=pLoopUnit->baseMoves();
		if(pLoopUnit->AI_getUnitAIType()==UNITAI_SETTLE)
		{
			iValue=MAX_INT-1;
		}
		if(iValue<iBestValue)
		{
			iBestValue=iValue;
			pBestUnit=pLoopUnit;
		}
	}
	return pBestUnit;
}

//send units that have more damage than iThreshold and that aren't adjacent to pTarget to a secure plot where they can heal in a good amount of turns
void CvAIGroup::healUnits(int iThreshold,CvPlot* pTarget)
{
	FeatureTypes eHauntedLands = (FeatureTypes)GC.getFEATURE_HAUNTED_LANDS();

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->getDamage()>iThreshold && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL
			&& (pTarget==NULL || !pLoopUnit->isInRange(pTarget,1)))
		{
			CvPlot* pBestPlot=pLoopUnit->plot();
			CvPlot* pBestMove=pLoopUnit->plot();

			if(pLoopUnit->healTurns(pBestPlot)>2 || 
				(pBestPlot->getFeatureType() != NO_FEATURE && pBestPlot->getFeatureType() == eHauntedLands))
			{
				int iSearchRange=5;
				int iDX,iDY;
				int iPathTurns;
				int iBestValue=pLoopUnit->healTurns(pBestPlot);

				for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
				{
					for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
					{
						CvPlot* pLoopPlot	= plotXY(pLoopUnit->plot()->getX_INLINE(), pLoopUnit->plot()->getY_INLINE(), iDX, iDY);
						if(pLoopPlot!=NULL && pLoopPlot->getArea()==pLoopUnit->plot()->getArea() && pLoopPlot->getFeatureType()!=eHauntedLands)
						{
							if((pLoopPlot->isCity() && pLoopPlot->getTeam()==getTeam())
								|| (!pLoopPlot->isVisibleEnemyDefender(pLoopUnit) && (pLoopPlot->getNumUnits()>0 
								|| GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopPlot)==0)))
							{
								if(pLoopUnit->generatePath(pLoopPlot,0,true,&iPathTurns))
								{
									int iValue=pLoopUnit->healTurns(pLoopPlot)+iPathTurns;

									iValue+=GET_PLAYER(getOwnerINLINE()).AI_getPlotDanger(pLoopPlot);
									if(pLoopPlot->isOwned() && pLoopPlot->getTeam()!=pLoopUnit->getTeam())
									{
										iValue*=2;
									}

									if(iValue<iBestValue)
									{
										iBestValue=iValue;
										pBestPlot=pLoopPlot;
										pBestMove=pLoopUnit->getPathEndTurnPlot();
									}
								}
							}
						}
					}
				}
			}


			if(!pLoopUnit->atPlot(pBestPlot))
			{
				pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pBestMove->getX_INLINE(), pBestMove->getY_INLINE(),MOVE_SAFE_TERRITORY);
			}
			if(pLoopUnit->atPlot(pBestPlot))
			{
				pLoopUnit->getGroup()->pushMission(MISSION_HEAL);
			}
		}
	}
}

void CvAIGroup::checkForPossibleUpgrade()
{
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pLoopUnit->AI_upgrade();
	}
}

int CvAIGroup::getNumUnitCategoryUnits(int UnitCategory) const
{
	//count UnitCategory Units;
	int iCountUnits=0;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		switch(UnitCategory)
		{
			case NO_UNITCATEGORY:
				break;
			default:
				if(GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),(UnitCategoryTypes)UnitCategory))
				{
					iCountUnits++;
				}
				break;
		}
	}

	return iCountUnits;
}

bool CvAIGroup::atPlot(CvPlot* pTarget) const
{
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

		if(pLoopUnit->plot() != pTarget)
			return false;
	}

	return true;
}


void CvAIGroup::Move(CvPlot* pTarget)
{
	int iPathTurns;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);

		if(pLoopUnit->generatePath(pTarget,0,true,&iPathTurns))
		{
			pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
		}
	}
}

void CvAIGroup::AIGroup_Pillage(CvCity* pTarget,int iRange, bool bIgnoreDanger)
{
	FAssertMsg(pTarget!=NULL,"AIGroup_Pillage target is NULL");
	if(pTarget==NULL)
	{
		return;
	}

	int iValue;
	int iBestValue=0;
	int iPathTurns;
	int MAX_PILLAGE_TARGETS=pTarget->getNumCityPlots();
	CvPlot* pBestPlot=NULL;
	CvUnit* pBestUnit=NULL;
	int* iPillageValue= new int[MAX_PILLAGE_TARGETS];
	bool* bPlotTaken=new bool[MAX_PILLAGE_TARGETS];

	for (int iI = 0; iI < MAX_PILLAGE_TARGETS; iI++)
	{
		iPillageValue[iI]=0;
		bPlotTaken[iI]=false;
		CvPlot* pLoopPlot = pTarget->getCityIndexPlot(iI);
		if (pLoopPlot != NULL)
		{
			if(pLoopPlot->getWorkingCity()==pTarget)
			{
				if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT 
					&& GC.getImprovementInfo(pLoopPlot->getImprovementType()).getImprovementClassType()==0
					&& !GC.getImprovementInfo(pLoopPlot->getImprovementType()).isPermanent())
				{
					if(bIgnoreDanger || GET_PLAYER((PlayerTypes)getOwnerINLINE()).AI_getPlotDanger(pLoopPlot,1,false)==0)
					{
						iValue=100;
						if(pLoopPlot->getBonusType(getTeam())!=NO_BONUS)
						{
							iValue+=1000;
						}
						if(GET_PLAYER((PlayerTypes)getOwnerINLINE()).AI_getPlotDanger(pLoopPlot,1,false)<4)
						{
							iValue+=3000;
						}
						iValue/=(pLoopPlot->getNumUnits()+1);

						iPillageValue[iI]=iValue;
					}
				}
			}
		}
	}

	//do the obvious, check if unit is already on plot that can be pillaged
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->plot()->getWorkingCity()==pTarget)
		{
			if(pLoopUnit->canPillage(pLoopUnit->plot()) && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
			{
				if(pLoopUnit->canMove())
					pLoopUnit->getGroup()->pushMission(MISSION_PILLAGE);
			}
		}

		for(int iI=1;iI<iRange;iI++)
		{
			for(int iX=-iI;iX<iI;iX++)
			{
				for(int iY=-iI;iY<iI;iY++)
				{
					CvPlot* pLoopPlot=GC.getMapINLINE().plot(pLoopUnit->getX_INLINE()+iX,pLoopUnit->getY_INLINE()+iY);
					if(pLoopPlot!=NULL && pLoopPlot->getWorkingCity()==pTarget)
					{
						int iIndex=pTarget->getCityPlotIndex(pLoopPlot);
						if(bPlotTaken[iIndex]==false && iPillageValue[iIndex]>0)
						{
							if(pLoopUnit->generatePath(pLoopPlot,0,true,&iPathTurns))
							{
								if(!pLoopUnit->getPathEndTurnPlot()->isVisibleEnemyDefender(pLoopUnit))
								{
									bPlotTaken[iIndex]=true;
									pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
									
									if(pLoopUnit->atPlot(pLoopPlot) && pLoopUnit->canMove())
									{
										if(pLoopUnit->canPillage(pLoopUnit->plot()))
										{
											pLoopUnit->getGroup()->pushMission(MISSION_PILLAGE);
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

	delete(iPillageValue);
}

void CvAIGroup::update()
{
	/** DEBUG **/
	TCHAR szOut[1024];
	sprintf(szOut, "%S %d::update -- Units: %d MissionStatus: %d\n",GC.getAIGroupInfo(getGroupType()).getDescription(), getID(),getNumUnits(),getMissionStatus());
	TCHAR szFile[1024];
	sprintf(szFile, "AI%d Group.log",getOwnerINLINE());
	gDLL->logMsg(szFile,szOut, false, false);

	/** DEBUG **/

	switch(getGroupType())
	{
		case AIGROUP_RESERVE:
			update_Reserve();
			break;
		case AIGROUP_CITY_DEFENSE:
			update_City_Defense();
			break;
		case AIGROUP_CITY_DEFENSE_REPLACE:
			update_City_Defense_Replace();
			break;
		case AIGROUP_SETTLE:
			update_Settle();
			break;
		case AIGROUP_WORKER:
			update_Worker();
			break;
		case AIGROUP_EXPLORE:
			update_Explore();
			break;
		case AIGROUP_COUNTER:
			update_Counter();
			break;
		case AIGROUP_DESTROY_LAIR:
			update_Destroy_Lair();
			break;
		case AIGROUP_CITY_INVASION:
			update_City_Invasion();
			break;
		case AIGROUP_MANA_NODE:
			update_Mana_Node();
			break;
		case AIGROUP_NAVAL_RESERVE:
			update_Naval_Reserve();
			break;
		case AIGROUP_NAVAL_COUNTER:
			update_Naval_Counter();
			break;
		case AIGROUP_NAVAL_SETTLE:
			update_Naval_Settle();
			break;
		case AIGROUP_NAVAL_INVASION:
			update_Naval_Invasion();
			break;
		case AIGROUP_NAVAL_EXPLORER:
			update_Naval_Explorer();
			break;
		case AIGROUP_BARB_PLUNDER:
			update_barb_plunder();
			break;
		case AIGROUP_BARB_NEW_LAIR:
			update_barb_new_lair();
			break;
		case AIGROUP_BARB_GOBLINS:
			update_barb_goblins();
			break;
		case AIGROUP_BARB_ORCS:
			update_barb_orcs();
			break;
		case AIGROUP_BARB_SKELETONS:
			update_barb_skeletons();
			break;
		default:
			break;
	}

	if(isDeathDelayed())
	{
		kill();
		return;
	}

	switch(getGroupType())
	{
		case AIGROUP_CITY_DEFENSE:
		case AIGROUP_SETTLE:
			if(UnitsNeeded()<0)
			{
				while(UnitsNeeded()<0)
				{
					RemoveWeakestUnit();
				}
			}
			break;
		case AIGROUP_RESERVE:
		case AIGROUP_EXPLORE:
		case AIGROUP_COUNTER:
		case AIGROUP_CITY_INVASION:
		case AIGROUP_DESTROY_LAIR:
		default:
			break;
	}
}

void CvAIGroup::update_Reserve()
{
	healUnits(30);

	update_Reserve_ReinforceCities();

	healUnits(10);

	update_Reserve_ReinforceDestroyLair();
	update_Reserve_ReinforceCounters();

	healUnits(0);

	update_Reserve_ShuffleCityDefense();

	update_Reserve_ReinforceSiege();

	update_Reserve_ReinforceCityInvasion();

	update_Reserve_Reinforce_NavalInvasion();

	update_Reserve_ReinforceManaNode();

	update_Reserve_ReinforceSettlers();

	update_Reserve_Reinforce_NavalSettle();

	update_Reserve_PickupEquipment();

	update_Reserve_ExploreDungeon();

	update_Reserve_ReinforceExplore();

	update_Reserve_Upgrade();

	update_Reserve_RetreatToCity();
}

void CvAIGroup::update_Reserve_ReinforceCities()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

    for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
    {
		if(pLoopCity->getArea()==getMissionArea())
		{
			if(pLoopCity->getAIGroup_Defense()->UnitsNeeded()>0)
			{
				while(pLoopCity->getAIGroup_Defense()->UnitsNeeded()>0)
				{
					CvUnit* pUnit=getCloseUnit(pLoopCity->plot(),pLoopCity->getAIGroup_Defense(),true);
					if(pUnit==NULL)
						break;
					pUnit->setAIGroup(pLoopCity->getAIGroup_Defense());
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_ShuffleCityDefense()
{
	//TODO::rewrite this by also allowing the AI to delete/remove Garrision units
	return;
	//END TODO

	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CvCityAI* pCapital=static_cast<CvCityAI*>(GET_PLAYER(getOwnerINLINE()).getCapitalCity());
	int iCounter=0;
	int iLoop;
	int INDEX=getNumUnits();

	//make a Sorted List of Units Defense Value
	if(INDEX==0 || pCapital==NULL)
	{
		//nothing to sort
		return;
	}

	int* iListDefenseValues=new int[INDEX];
	int* iListIDs=new int[INDEX];

	int iI=0;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(isUnitAllowed(pLoopUnit,AIGROUP_CITY_DEFENSE))
		{
			iListDefenseValues[iI]=pCapital->AI_ValueCityDefender(pLoopUnit->getUnitType(),pCapital->getAIGroup_Defense());
		}
		else
		{
			iListDefenseValues[iI]=-MAX_INT;
		}
		iListIDs[iI]=pLoopUnit->getID();
		iI++;
	}

	sortList(iListDefenseValues,iListIDs,INDEX);

    for (CvCityAI* pLoopCity = static_cast<CvCityAI*>(kPlayer.firstCity(&iLoop)); pLoopCity != NULL; pLoopCity = static_cast<CvCityAI*>(kPlayer.nextCity(&iLoop)))
    {
		if(pLoopCity->getArea()==getMissionArea())
		{
			CLLNode<IDInfo>* pNextUnitNode;

			for (CLLNode<IDInfo>* pUnitNode = pLoopCity->getAIGroup_Defense()->headUnitNode(); pUnitNode != NULL; pUnitNode = pNextUnitNode)
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				pNextUnitNode = pLoopCity->getAIGroup_Defense()->nextUnitNode(pUnitNode);

				if(pLoopCity->getAIGroup_Defense_Replace()==NULL)
				{
					for(int iI=0;iI<INDEX;iI++)
					{
						int iValue=pLoopCity->AI_ValueCityDefender(pLoopUnit->getUnitType(),pLoopCity->getAIGroup_Defense());
						if(iListDefenseValues[iCounter]>iValue)
						{
							CvAIGroup* pNewGroup=kPlayer.initAIGroup(AIGROUP_CITY_DEFENSE_REPLACE);
							pLoopCity->setAIGroup_Defense_Replace(pNewGroup);
							pNewGroup->addUnit(kPlayer.getUnit(iListIDs[iCounter]));
							pNewGroup->setMissionCity(pLoopCity);
							pNewGroup->setMissionPlot(pLoopCity->plot());
							iCounter++;
							break;
						}
					}
				}
			}
		}
		if(iCounter>=INDEX)
		{
			break;
		}
	}

	delete(iListDefenseValues);
	delete(iListIDs);
}

void CvAIGroup::update_Reserve_ReinforceCounters()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_COUNTER && pAIGroup->getMissionArea()==getMissionArea())
		{
			if(pAIGroup->UnitsNeeded()>0)
			{
				if(pAIGroup->canFillWithReserveUnits(this))
				{
					while(pAIGroup->UnitsNeeded()>0)
					{
						CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionTargetUnit()->plot(),pAIGroup, true);
						if(pUnit==NULL)
							break;
						pUnit->setAIGroup(pAIGroup);
					}
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_ReinforceDestroyLair()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_DESTROY_LAIR && pAIGroup->getMissionArea()==getMissionArea())
		{
			if(pAIGroup->UnitsNeeded()>0)
			{
				if(pAIGroup->canFillWithReserveUnits(this))
				{
					while(pAIGroup->UnitsNeeded()>0)
					{
						CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionPlot(),pAIGroup, true);
						if(pUnit==NULL)
							break;
						pUnit->setAIGroup(pAIGroup);
					}
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_ReinforceCityInvasion()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_CITY_INVASION && pAIGroup->getMissionArea()==getMissionArea() && pAIGroup->getMissionTarget()!=NULL)
		{
			//make sure the AI will prefere to use single units to fill up Groups already in action 
			if(pAIGroup->getMissionStatus()>NO_AIGROUP_STATUS)
			{
				if(pAIGroup->UnitsNeeded()>0)
				{
					while(pAIGroup->UnitsNeeded()>0)
					{
						CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionTarget(),pAIGroup, true);
						if(pUnit==NULL)
							break;
						pUnit->setAIGroup(pAIGroup);
					}
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_ReinforceManaNode()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;
	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_MANA_NODE && pAIGroup->getMissionArea()==getMissionArea())
		{
			if(pAIGroup->UnitsNeeded()>0)
			{
				while(pAIGroup->UnitsNeeded()>0)
				{
					CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionPlot(),pAIGroup, true);
					if(pUnit==NULL)
						break;
					pUnit->setAIGroup(pAIGroup);
				}
			}
		}
	}
}
	
void CvAIGroup::update_Reserve_ReinforceSiege()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_SIEGE && pAIGroup->getMissionArea()==getMissionArea())
		{
			if(pAIGroup->UnitsNeeded()>0)
			{
				while(pAIGroup->UnitsNeeded()>0)
				{
					CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionPlot(),pAIGroup, true);
					if(pUnit==NULL)
						break;
					pUnit->setAIGroup(pAIGroup);
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_ReinforceSettlers()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	CLLNode<IDInfo>* pNextUnitNode;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = pNextUnitNode)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pNextUnitNode = nextUnitNode(pUnitNode);
		if (pLoopUnit->AI_getUnitAIType()==UNITAI_SETTLE)
		{
			CvAIGroup* pNewGroup=kPlayer.initAIGroup(AIGROUP_SETTLE);
			pLoopUnit->setAIGroup(pNewGroup);
			pNewGroup->setMissionArea(getMissionArea());
			pNewGroup->setMissionPlot(pLoopUnit->plot());
		}
	}

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType() == AIGROUP_SETTLE && pAIGroup->getMissionArea()==getMissionArea()
			&& 	pAIGroup->getMissionPlot() != NULL)
		{
			while(pAIGroup->UnitsNeeded()>0)
			{
				CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionPlot(),pAIGroup,true);
				if(pUnit==NULL)
					break;
				pUnit->setAIGroup(pAIGroup);
			}
		}
	}
}

void CvAIGroup::update_Reserve_ReinforceExplore()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	CLLNode<IDInfo>* pNextUnitNode;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = pNextUnitNode)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pNextUnitNode = nextUnitNode(pUnitNode);
		if (pLoopUnit->AI_getUnitAIType()==UNITAI_EXPLORE)
		{
			int iLoop;
			for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
			{
				if(pAIGroup->getGroupType()==AIGROUP_EXPLORE)
				{
					pLoopUnit->setAIGroup(pAIGroup);
					break;
				}
			}
			if(pLoopUnit->getAIGroup()->getGroupType()!=AIGROUP_EXPLORE)
			{
				CvAIGroup* pNewGroup=kPlayer.initAIGroup(AIGROUP_EXPLORE);
				pLoopUnit->setAIGroup(pNewGroup);
			}
		}
	}
}

void CvAIGroup::update_Reserve_PickupEquipment()
{
	CvUnit* pEquipment=NULL;
	CvPlot* pLoopPlot;

	for(int iI=0;iI<GC.getMapINLINE().numPlots();iI++)
	{
		pLoopPlot=GC.getMapINLINE().plotByIndex(iI);
		if(pLoopPlot->getArea()==getMissionArea())
		{
			if(pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && pLoopPlot->getNumUnits()>0)
			{
				for (CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode(); pUnitNode != NULL; pUnitNode = pLoopPlot->nextUnitNode(pUnitNode))
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					if(pLoopUnit->isEquipment())
					{
						if(pLoopUnit->getOwnerINLINE()==getOwnerINLINE() || GET_TEAM(pLoopUnit->getTeam()).isAtWar(getTeam()))
						{
							pEquipment=pLoopUnit;
						}
					}
				}
			}
		}
	}

	if(pEquipment!=NULL)
	{
		int iBestValue=10;
		int iValue;
		int iPathTurns;
		int difX,difY;
		CvUnit* pBestUnit=NULL;
		CvPlot* pMove=NULL;

		for(int iPass=0; iPass<2; iPass++)
		{
			for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				
				difX=pEquipment->getX_INLINE()-pLoopUnit->getX_INLINE();
				difY=pEquipment->getY_INLINE()-pLoopUnit->getY_INLINE();
				if(iPass>0 || difX*difX+difY*difY<50)
				{
					if(pLoopUnit->generatePath(pEquipment->plot(),0,true,&iPathTurns))
					{
						iValue=iPathTurns;
						if(iValue<iBestValue)
						{
							iBestValue=iValue;
							pBestUnit = pLoopUnit;
							pMove = pLoopUnit->getPathEndTurnPlot();
						}
					}
				}
			}
		}
		if(pBestUnit!=NULL)
		{
			if(!pBestUnit->atPlot(pEquipment->plot()))
			{
				pBestUnit->getGroup()->pushMission(MISSION_MOVE_TO, pMove->getX_INLINE(), pMove->getY_INLINE(),MOVE_DIRECT_ATTACK);
			}

			if(pBestUnit->atPlot(pEquipment->plot()))
			{
				//maybe rewrite how Equipment is picked up by Spells
				int iSpell=pBestUnit->chooseSpell();
				pBestUnit->cast(iSpell,pBestUnit->plot());
			}
		}
	}
}

void CvAIGroup::update_Reserve_ExploreDungeon()
{
	CvPlot* pLoopPlot;

	for(int iI=0;iI<GC.getMapINLINE().numPlots();iI++)
	{
		pLoopPlot=GC.getMapINLINE().plotByIndex(iI);
		if(pLoopPlot->getArea()==getMissionArea())
		{
			if(pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && !pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE()))
			{
				ImprovementTypes eGraveYard=(ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_GRAVEYARD");
				ImprovementTypes eDungeon=(ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_DUNGEON");
				if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT &&
					(GC.getImprovementInfo(pLoopPlot->getImprovementType()).getSpawnUnitCiv()!=NO_CIVILIZATION || pLoopPlot->getImprovementType()==eGraveYard || pLoopPlot->getImprovementType()==eDungeon))
				{
					int iBestValue=10;
					int iValue;
					int iPathTurns;
					int difX,difY;
					CvUnit* pBestUnit=NULL;
					CvPlot* pMove=NULL;

					for(int iPass=0; iPass<2; iPass++)
					{
						for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
						{
							CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
							
							difX=pLoopPlot->getX_INLINE()-pLoopUnit->getX_INLINE();
							difY=pLoopPlot->getY_INLINE()-pLoopUnit->getY_INLINE();
							if(iPass>0 || difX*difX+difY*difY<50)
							{
								if(pLoopUnit->canMove())
								{
									if(pLoopUnit->generatePath(pLoopPlot,0,true,&iPathTurns))
									{
										iValue=iPathTurns;
										if(iValue<iBestValue)
										{
											iBestValue=iValue;
											pBestUnit = pLoopUnit;
											pMove = pLoopUnit->getPathEndTurnPlot();
										}
									}
								}
							}
						}
					}
					if(pBestUnit!=NULL)
					{
						if(!pBestUnit->atPlot(pLoopPlot))
						{
							pBestUnit->getGroup()->pushMission(MISSION_MOVE_TO, pMove->getX_INLINE(), pMove->getY_INLINE(),MOVE_DIRECT_ATTACK);
						}

						if(pBestUnit->atPlot(pLoopPlot))
						{
							if(pBestUnit->canPillage(pLoopPlot))
							{
								pBestUnit->pillage();
							}
							else
							{
								int iSpell=pBestUnit->chooseSpell();
								pBestUnit->cast(iSpell,pBestUnit->plot());
							}
						}
					}
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_Upgrade()
{
	CvArea* pArea=GC.getMapINLINE().getArea(getMissionArea());
	if(pArea!=NULL && pArea->getCitiesPerPlayer(getOwnerINLINE())>0)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
			pLoopUnit->AI_upgrade();
		}
	}
}

void CvAIGroup::update_Reserve_RetreatToCity()
{
	CvArea* pArea=GC.getMapINLINE().getArea(getMissionArea());
	if(pArea!=NULL && pArea->getCitiesPerPlayer(getOwnerINLINE())>0)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
			if(!pLoopUnit->plot()->isCity())
			{
				if(pLoopUnit->getGroup()->getActivityType()==ACTIVITY_AWAKE)
				{
					pLoopUnit->AI_retreatToCity();
				}
			}
		}
	}
}

void CvAIGroup::update_City_Defense()
{
	CvPlot* pHome=getMissionCity()->plot();
	if(pHome==NULL || pHome->getOwnerINLINE()!=getOwnerINLINE())
	{
		setDeathDelayed(true);
		return;
	}
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->plot()!=pHome)
		{
			pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pHome->getX_INLINE(), pHome->getY_INLINE(),MOVE_DIRECT_ATTACK);
		}
		else
		{
			if(pLoopUnit->canGarrision())
			{
				pLoopUnit->setGarrision(true);
			}
			pLoopUnit->AI_upgrade();			
		}
	}
}

void CvAIGroup::update_City_Defense_Replace()
{
	CvCityAI* pHome=static_cast<CvCityAI*>(getMissionCity());
	CvPlot* pPlot=getMissionPlot();
	CvUnit* pUnit = ::getUnit(headUnitNode()->m_data);

	if(pHome==NULL || pUnit==NULL)
	{
		setDeathDelayed(true);
		return;
	}

	if(pUnit->plot()!=pPlot)
	{
		if(pUnit->generatePath(pPlot,0,true))
		{
			pUnit->getGroup()->pushMission(MISSION_MOVE_TO, pHome->plot()->getX_INLINE(), pHome->plot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
		}
	}

	if(pUnit->plot()==getMissionPlot())
	{
		//join the City Defense
		if(pHome->getAIGroup_Defense()->getNumUnits()>0 && pHome->getAIGroup_Defense()->UnitsNeeded()<=0)
		{
			pHome->getAIGroup_Defense()->RemoveWeakestUnit();			
		}

		pUnit->setAIGroup(pHome->getAIGroup_Defense());
		setDeathDelayed(true);
		return;
	}
}

void CvAIGroup::update_Worker()
{
	if(getMissionPlot() != NULL)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
			if (pLoopUnit->AI_getUnitAIType()==UNITAI_WORKER && pLoopUnit->canMove())
			{
				if (GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pLoopUnit->plot(), 3))
				{
					if(!pLoopUnit->plot()->isCity())
					{
						if (pLoopUnit->AI_retreatToCity())
						{
							continue;
						}

						if (pLoopUnit->AI_safety())
						{
							continue;
						}
					}
				}
				if(getMissionPlot()!=NULL)
				{
					if(!pLoopUnit->atPlot(getMissionPlot()))
					{
						pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), 0, false, false, MISSIONAI_BUILD, getMissionPlot());
					}
					if(pLoopUnit->atPlot(getMissionPlot()))
					{
						pLoopUnit->getGroup()->pushMission(MISSION_BUILD, getMissionBuild(), -1, MOVE_SAFE_TERRITORY, (pLoopUnit->getGroup()->getLengthMissionQueue() > 0), false, MISSIONAI_BUILD, getMissionPlot());
					}
				}
			}
		}
	}
	else
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));

			if (GET_PLAYER(getOwnerINLINE()).AI_isPlotThreatened(pLoopUnit->plot(), 3))
			{
				if(!pLoopUnit->plot()->isCity())
				{
					if (pLoopUnit->AI_retreatToCity())
					{
						continue;
					}

					if (pLoopUnit->AI_safety())
					{
						continue;
					}
				}
			}

			pLoopUnit->setAIGroup(NULL);
		}
	}
}

void CvAIGroup::update_Explore()
{
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
		if (pLoopUnit->AI_getUnitAIType()==UNITAI_EXPLORE && pLoopUnit->canMove())
		{
			pLoopUnit->AI_exploreMove();
		}
		else
		{
			pLoopUnit->setAIGroup(NULL);
		}
	}
}

void CvAIGroup::update_Counter()
{
	//Target still valid?
	if(!update_Counter_isTargetStillValid())
	{
		return;
	}

	//Powerful Enough to Launch attack?
	//ToDo add in Calculation for Damage Spells pre Combat
	int iEnemyStrength, iOurStrength;
	compareAttackStrength(getMissionPlot(),&iEnemyStrength,&iOurStrength);

	if(iOurStrength>iEnemyStrength*1.2)
	{
		launchAttack(getMissionPlot());
	}
	else
	{
		prepareAttack(getMissionPlot(),MAX_INT,1);
	}

	checkForPossibleUpgrade();
}

void CvAIGroup::update_Destroy_Lair()
{
	//Target still valid?
	if(getMissionPlot()==NULL || getMissionPlot()->getImprovementType()==NO_IMPROVEMENT || GC.getImprovementInfo(getMissionPlot()->getImprovementType()).getSpawnUnitCiv()!=GC.getDefineINT("BARBARIAN_CIVILIZATION") || GET_TEAM(getTeam()).isBarbarianAlly()
		||(getMissionPlot()->isOwned() && getMissionPlot()->getOwnerINLINE()!=getOwnerINLINE()))
	{
		setDeathDelayed(true);
		return;
	}

	//Powerful Enough to Launch attack?
	//ToDo add in Calculation for Damage Spells pre Combat
	int iEnemyStrength, iOurStrength;
	compareAttackStrength(getMissionPlot(),&iEnemyStrength,&iOurStrength);

	if(iOurStrength>iEnemyStrength*1.3 || isAllUnitsWithinRange(getMissionPlot(),1))
	{
		launchAttack(getMissionPlot());
	}
	else if(UnitPowerNeeded()<=0)
	{
		prepareAttack(getMissionPlot(),MAX_INT,1);
	}

	int iExploreSpell=GC.getInfoTypeForString("SPELL_EXPLORE_DUNGEON");

	if(iExploreSpell!=NO_SPELL)
	{
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
			if (pLoopUnit->plot()==getMissionPlot() )
			{
				if(pLoopUnit->canCast(iExploreSpell,false))
				{
					pLoopUnit->cast(iExploreSpell);
					break;
				}
			}
		}
	}

	if(UnitPowerNeeded()>0)
	{
		checkForPossibleUpgrade();
	}
}

bool CvAIGroup::update_Counter_isTargetStillValid()
{
	CvPlot* pLoopPlot;
	int iDX,iDY;
	int iSearchRange=5;
	int iValue;
	int iBestValue=0;
	CvPlot* pBestPlot;

	//Target Moved to Different Area
	if(getMissionTargetUnit()!=NULL && getMissionPlot()!=NULL)
	{
		if(getMissionTargetUnit()->plot()->getArea()!=getMissionPlot()->getArea())
		{
			setMissionTargetUnit(NULL);
		}

		else if(getMissionTargetUnit()->plot()->isCity())
		{
			setMissionTargetUnit(NULL);
		}
	}

	if(getMissionTargetUnit()==NULL)
	{
		if(getMissionPlot()==NULL || getMissionPlot()->getTeam()!=getTeam())
		{
			setDeathDelayed(true);
			return false;
		}

		iSearchRange=5;
		iBestValue=0;
		pBestPlot=NULL;
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				pLoopPlot = plotXY(getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->getArea()==getMissionPlot()->getArea())
					{
						//maybe find a better way to block Acheron
						if(!pLoopPlot->isCity())
						{
							if(pLoopPlot->getNumUnits()>0)
							{
								iValue=GET_PLAYER(getOwnerINLINE()).AI_getEnemyPlotStrength(pLoopPlot,0,true,false);
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
		if(pBestPlot!=NULL)
		{
			iBestValue=0;
			CvUnit* pBestUnit=NULL;
			for (CLLNode<IDInfo>* pUnitNode = pBestPlot->headUnitNode(); pUnitNode != NULL; pUnitNode = pBestPlot->nextUnitNode(pUnitNode))
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				iValue=pLoopUnit->getPower();
				iValue*=pLoopUnit->getGroup()->getNumUnits();
				if(iValue>iBestValue)
				{
					iBestValue=iValue;
					pBestUnit=pLoopUnit;
				}
			}
			if(pBestUnit!=NULL)
			{
				setMissionTargetUnit(pBestUnit);
				setMissionPlot(pBestUnit->plot());
			}
		}
		//there is nothing more to do for us
		if(getMissionTargetUnit()==NULL)
		{
			setDeathDelayed(true);
			return false;
		}
	}
	else
	{
		setMissionPlot(getMissionTargetUnit()->plot());
		bool bValid=false;
		if(getMissionPlot()->getTeam()==getTeam())
			bValid=true;
		else if(!getMissionTargetUnit()->isBarbarian())
		{
			iSearchRange=3;			
			for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
			{
				for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
				{
					pLoopPlot = plotXY(getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), iDX, iDY);

					if (pLoopPlot != NULL)
					{
						if(pLoopPlot->getArea()==getMissionPlot()->getArea())
						{
							if(pLoopPlot->getTeam()==getTeam())
							{
								bValid=true;
								break;
							}
						}
					}
				}
			}
		}
		if(!bValid)
		{
			setDeathDelayed(true);
			return false;
		}
	}
	return true;
}
/** old function
void CvAIGroup::update_City_Invasion()
{
	if(!update_City_Invasion_isTargetStillValid())
	{
		return;
	}

	TCHAR szOut[1024];
	sprintf(szOut, "CvAIInvasion %d,%d::update -- %S\n",getOwnerINLINE(),getID(),GC.getAIGroupInfo(getGroupType()).getDescription());
	gDLL->logMsg("AIInvasion.log",szOut, false, false);
	sprintf(szOut, "TargetCity -- %S\n",getMissionPlot()->getPlotCity()->getNameKey());
	gDLL->logMsg("AIInvasion.log",szOut, false, false);
	sprintf(szOut, "Power Ratio -- %d\n",GET_TEAM(getTeam()).getPower(true)*100/(1+GET_TEAM(getMissionPlot()->getTeam()).getPower(true)));
	gDLL->logMsg("AIInvasion.log",szOut, false, false);
	sprintf(szOut, "Already at War? -- %d\n",GET_TEAM(getTeam()).isAtWar(getMissionPlot()->getTeam()));
	gDLL->logMsg("AIInvasion.log",szOut, false, false);
	sprintf(szOut, "Current MissionStatus -- %d\n",getMissionStatus());
	gDLL->logMsg("AIInvasion.log",szOut, false, false);
	sprintf(szOut, "Units, Units needed -- %d,%d\n",getNumUnits(),UnitsNeeded());
	gDLL->logMsg("AIInvasion.log",szOut, false, false);	

	if(GET_PLAYER(getOwnerINLINE()).getCivilizationType()==GC.getInfoTypeForString("CIVILIZATION_BALSERAPHS"))
	{
		int iYeahWhatever=0;
	}

	if(getMissionStatus()==NO_AIGROUP_STATUS)
	{
		setMissionStatus(STATUS_CITY_INVASION_PREPARATION);
	}

	//TODO: add that we do not target cities already targeted
	if(getMissionStatus()==STATUS_CITY_INVASION_PREPARATION)
	{
		if(update_City_Invasion_sendPillageUnits())
		{
			setMissionStatus(STATUS_CITY_INVASION_PILLAGE_SEND);
		}
	}

	if(getMissionStatus()>=STATUS_CITY_INVASION_PILLAGE_SEND)
	{
		if(UnitsNeeded()<=0)
		{
			CvPlot* pTarget=getMissionPlot()->getcloseSecurePlot(getTeam());

			int iCountUnitsReadyToGo=0;
			for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				if(pTarget==NULL || pLoopUnit->isInRange(pTarget,3))
				{
					iCountUnitsReadyToGo++;
				}
			}

			if(iCountUnitsReadyToGo*4>=getNumUnits()*3)
			{
				setMissionStatus(STATUS_CITY_INVASION_SIEGE);

				//go into Final Attack mode if we cannot bombard City anymore
				//TODO: add bombardcheck
				if(UnitsNeeded()<=0)
				{
					setMissionStatus(STATUS_CITY_INVASION_FINAL);
				}
			}
		}
		//cannot train more units
		if(!GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits(0))
		{
			setMissionStatus(STATUS_CITY_INVASION_FINAL);
		}
	}

	update_City_Invasion_SiegeGroup();

	if(getMissionStatus()<=STATUS_CITY_INVASION_PREPARATION)
	{
		healUnits(10);
		update_City_Invasion_moveToSecurePlot();
		checkForPossibleUpgrade();
		return;
	}

	if(getMissionStatus()>=STATUS_CITY_INVASION_PILLAGE_SEND)
	{
		//declare war if necessary
		if(!GET_TEAM(getTeam()).isAtWar(getMissionPlot()->getTeam()))
		{
			if(GET_TEAM(getTeam()).canDeclareWar(getMissionPlot()->getTeam()))
			{
				GET_TEAM(getTeam()).declareWar(getMissionPlot()->getTeam(), true, WARPLAN_LIMITED);
			}
		}
	}

	//Powerful Enough to Launch Final attack?
	//ToDo add in Calculation for Damage Spells pre Combat
	int iEnemyStrength, iOurStrength;
	compareInvasionPower(getMissionPlot(),&iEnemyStrength,&iOurStrength);

	if(iOurStrength>iEnemyStrength*1.2)
	{
		launchAttack(getMissionPlot());
	}

	//remove units from group that are more needed in City Defense
	update_City_Invasion_ReinforceCityDefenses();



	castAnyTarget(0);
	attackAnyUnit(80);

	//send Pillage Units to Scout cities
	if(getMissionStatus()>=STATUS_CITY_INVASION_PILLAGE_SEND)
	{
		update_City_Invasion_PillageGroups();
	}

	//we do not have enough units to take the City anyway, so look around
	if(UnitsNeeded()>0)
	{
		update_City_Invasion_CheckForEasierTarget();
	}

	if(getMissionStatus()==STATUS_CITY_INVASION_PILLAGE_SEND)
	{
		update_City_Invasion_moveToSecurePlot();
	}

	if(getMissionStatus()==STATUS_CITY_INVASION_SIEGE)
	{
		healUnits(30);

		pillage();

		update_City_Invasion_SiegeCity();		
	}
	if(getMissionStatus()==STATUS_CITY_INVASION_FINAL)
	{
		if(iOurStrength>iEnemyStrength*1.2)
		{
			launchAttack(getMissionPlot());
		}
		else if(UnitsNeeded()<=0 || GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits(5))
		{
			compareAttackStrength(getMissionPlot(),&iEnemyStrength,&iOurStrength,false);
			int iRange=4;
			
			iOurStrength=getUnitPowerWithinRange(getMissionPlot(),4);
			if(iOurStrength>iEnemyStrength*1.2)
			{
				if(getUnitPowerWithinRange(getMissionPlot(),2)<=iEnemyStrength*1.2)
				{
					iRange=2;
				}
				else
				{
					iRange=1;
				}
			}
			prepareAttack(getMissionPlot(),MAX_INT, iRange);
		}
	}

	//what happens we are struck because we cannot train more units? Charge!
	if(UnitsNeeded()>0 && getMissionStatus()==STATUS_CITY_INVASION_FINAL && !GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits(5))
	{
		attackAnyUnit(120);
		//try to get Enough Units within Attack Range
		if(getUnitPowerWithinRange(getMissionPlot(),1)*100<=getUnitPowerWithinRange(getMissionPlot(),10)*80)
		{
			int iRange=4;
			
			if(getUnitPowerWithinRange(getMissionPlot(),1)*100<=getUnitPowerWithinRange(getMissionPlot(),4)*80)
			{
				iRange=1;
			}
			prepareAttack(getMissionPlot(),MAX_INT, iRange);
		}
		//Attack!
		else
		{
			launchAttack(getMissionPlot());
		}
	}
}

//if Enough Units are in the Group it will add units to the pillage units and return true
bool CvAIGroup::update_City_Invasion_sendPillageUnits()
{
	int iNeededUnitsForPillage=2;
	
	if(getHelpGroup1()==NULL)
	{
		CvAIGroup* pNewGroup=GET_PLAYER(getOwnerINLINE()).initAIGroup(AIGROUP_CITY_INVASION_PILLAGE);
		pNewGroup->setMissionPlot(getMissionPlot());
		setHelpGroup1(pNewGroup);
	}

	int iCounter=0;

	int iSearchRange=7;
	int iDX,iDY;
	CvPlot* pLoopPlot;

	if(getHelpGroup3()==NULL)
	{
		//look if we can find more target cities
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				pLoopPlot = plotXY(getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					if(pLoopPlot->isCity() && pLoopPlot->getTeam()==getMissionPlot()->getTeam() && pLoopPlot->getArea()==getMissionPlot()->getArea())
					{
						if(!pLoopPlot->isBarbarian() && GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam()))
						{
							if((getHelpGroup1()==NULL || getHelpGroup1()->getMissionPlot()!=pLoopPlot)
								&&(getHelpGroup2()==NULL || getHelpGroup2()->getMissionPlot()!=pLoopPlot))
							{
								if(iCounter==0)
								{
									if(getHelpGroup2()==NULL)
									{
										CvAIGroup* pNewGroup=GET_PLAYER(getOwnerINLINE()).initAIGroup(AIGROUP_CITY_INVASION_PILLAGE);
										pNewGroup->setMissionCity(pLoopPlot->getPlotCity());
										pNewGroup->setMissionPlot(pLoopPlot);
										setHelpGroup2(pNewGroup);
									}
									iCounter++;
								}
								else if(iCounter==1)
								{
									if(getHelpGroup3()==NULL)
									{
										CvAIGroup* pNewGroup=GET_PLAYER(getOwnerINLINE()).initAIGroup(AIGROUP_CITY_INVASION_PILLAGE);
										pNewGroup->setMissionCity(pLoopPlot->getPlotCity());
										pNewGroup->setMissionPlot(pLoopPlot);
										setHelpGroup3(pNewGroup);
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

	if(getHelpGroup2()!=NULL)
		iNeededUnitsForPillage+=2;
	if(getHelpGroup3()!=NULL)
		iNeededUnitsForPillage+=2;

	//go into STATUS_CITY_INVASION_PILLAGE_SEND modus?
	if(getNumUnits()<iNeededUnitsForPillage)
		return false;

	
	iCounter=0;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canMove() && pLoopUnit->canAttack())
		{
			if(isUnitAllowed(pLoopUnit,AIGROUP_CITY_INVASION_PILLAGE))
			{
				iCounter++;
			}
		}
	}

	if(iCounter<iNeededUnitsForPillage)
		return false;

	CvAIGroup* pLoopGroup=NULL;
	for(int iI=0;iI<3;iI++)
	{
		if(iI==0)
			pLoopGroup=getHelpGroup1();
		if(iI==1)
			pLoopGroup=getHelpGroup2();
		if(iI==2)
			pLoopGroup=getHelpGroup3();

		iCounter=0;
		while(pLoopGroup!=NULL && pLoopGroup->UnitsNeeded()>0)
		{
			for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
			{
				CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
				if(pLoopUnit->canMove() && pLoopUnit->canAttack())
				{
					if(isUnitAllowed(pLoopUnit,AIGROUP_CITY_INVASION_PILLAGE))
					{
						pLoopUnit->setAIGroup(pLoopGroup);
					}
				}
			}

			iCounter++;
			if(iCounter>100)
			{
				break;
			}
		}
	}

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canMove() && pLoopUnit->canAttack())
		{
			if(isUnitAllowed(pLoopUnit,AIGROUP_CITY_INVASION_PILLAGE))
			{
				if(getHelpGroup1()!=NULL && getHelpGroup1()->UnitsNeeded()>0)
				{
					pLoopUnit->setAIGroup(getHelpGroup1());
				}
				if(getHelpGroup2()!=NULL && getHelpGroup2()->UnitsNeeded()>0)
				{
					pLoopUnit->setAIGroup(getHelpGroup2());
				}
				if(getHelpGroup3()!=NULL && getHelpGroup3()->UnitsNeeded()>0)
				{
					pLoopUnit->setAIGroup(getHelpGroup3());
				}
			}
		}
	}
	return true;
}

void CvAIGroup::update_City_Invasion_PillageGroups()
{
	TCHAR szOut[1024];

	if(getHelpGroup1()!=NULL)
	{
		sprintf(szOut, "Update HelpGroup1 \n");
		gDLL->logMsg("AIInvasion.log",szOut, false, false);	

		getHelpGroup1()->update_City_Invasion_Scout_Enemy_City();
	}

	if(getHelpGroup2()!=NULL)
	{
		sprintf(szOut, "Update HelpGroup2 \n");
		gDLL->logMsg("AIInvasion.log",szOut, false, false);	

		getHelpGroup2()->update_City_Invasion_Scout_Enemy_City();
	}

	if(getHelpGroup3()!=NULL)
	{
		sprintf(szOut, "Update HelpGroup3 \n");
		gDLL->logMsg("AIInvasion.log",szOut, false, false);	

		getHelpGroup3()->update_City_Invasion_Scout_Enemy_City();
	}
}

void CvAIGroup::update_City_Invasion_Scout_Enemy_City()
{
	if(!update_City_Invasion_isTargetStillValid())
	{
		return;
	}

	
	TCHAR szOut[1024];
	sprintf(szOut, "CvAIInvasion %d,%d::update_City_Invasion_Scout_Enemy_City -- %S\n",getOwnerINLINE(),getID(),GC.getAIGroupInfo(getGroupType()).getDescription());
	gDLL->logMsg("AIInvasion.log",szOut, false, false);
	sprintf(szOut, "TargetCity -- %S\n",getMissionPlot()->getPlotCity()->getNameKey());
	gDLL->logMsg("AIInvasion.log",szOut, false, false);
	sprintf(szOut, "Units -- %d\n",getNumUnits());
	gDLL->logMsg("AIInvasion.log",szOut, false, false);	
	

	AIGroup_Pillage(getMissionPlot()->getPlotCity(),3,false);

	prepareAttack(getMissionPlot(),MAX_INT,2);
}

void CvAIGroup::update_City_Invasion_SiegeGroup()
{
	if(getSiegeGroup()==NULL)
	{
		CvAIGroup* pNewGroup=GET_PLAYER(getOwnerINLINE()).initAIGroup(AIGROUP_SIEGE);
		pNewGroup->setSiegeGroup(this);
		setSiegeGroup(pNewGroup);
	}

	getSiegeGroup()->update_Siege();
}


void CvAIGroup::update_City_Invasion_SiegeCity()		
{
	if(getMissionPlot()==NULL)
		return;

	CvCity* pTarget=getMissionPlot()->getPlotCity();

	if(pTarget==NULL)
		return;

	//Heal Units First
	healUnits(20);

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		int iPathTurns;
		if(pLoopUnit->canMove() && pLoopUnit->getGroup()->getActivityType()!=ACTIVITY_HEAL)
		{
			if(pLoopUnit->plot()->getWorkingCity()!=pTarget)
			{
				if(pLoopUnit->generatePath(getMissionPlot(),0,true,&iPathTurns))
				{
					if(iPathTurns>1)
					{
						if(pLoopUnit->canAttack())
						{
							pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot() ->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
							continue;
						}
						else if(GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
						{
							pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot() ->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_IGNORE_DANGER);
						}
					}
				}
			}
			if(pLoopUnit->canPillage(pLoopUnit->plot()))
			{
				pLoopUnit->getGroup()->pushMission(MISSION_PILLAGE);
				continue;
			}
		}
	}

	attackAnyUnit(80);

	AIGroup_Pillage(pTarget,3,true);
}

//decide to attack another city
//returns true if we target another City
bool CvAIGroup::update_City_Invasion_CheckForEasierTarget()
{
	if(getMissionPlot()==NULL)
	{
		return false;
	}

	CvPlot* pBestNewTarget=NULL;
	int iValue;
	int iBestValue=MAX_INT;

    int iSearchRange=10;
    int iDX,iDY;
    CvPlot* pLoopPlot;

	bool bVisible=false;
	//If we have no chance to take the city by sending our whole army at it, move to a different city
	if(GET_PLAYER(getOwnerINLINE()).canSupportMoreUnits(UnitsNeeded()))
		bVisible=true;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
			    if(pLoopPlot->isCity() && pLoopPlot->getArea()==getMissionPlot()->getArea())
			    {
					if(bVisible || pLoopPlot->isVisible(getTeam(),false))
					{
						if(!pLoopPlot->isBarbarian() && GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam()))
						{
							int iEnemyStrength=0;
							int iOurStrength=0;

							for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
							{
								CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
								if(pLoopUnit->canAttack())
								{
									iOurStrength+=pLoopUnit->maxCombatStr(NULL, NULL);
								}
							}

							for(CLLNode<IDInfo>* pUnitNode=pLoopPlot->headUnitNode(); pUnitNode != NULL; pUnitNode = pLoopPlot->nextUnitNode(pUnitNode))
							{
								CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
								if(pLoopUnit->getTeam()==pLoopPlot->getTeam())
								{
									iEnemyStrength+=pLoopUnit->maxCombatStr(pLoopUnit->plot(), NULL);
								}
							}


							if(iOurStrength>iEnemyStrength*3)
							{
								iValue=iEnemyStrength;
								if(iValue<iBestValue)
								{
									iBestValue=iValue;
									pBestNewTarget=pLoopPlot;
								}
							}
						}
					}
				}
			}
		}
	}

	if(pBestNewTarget!=NULL)
	{
		setMissionPlot(pBestNewTarget);
		setMissionCity(pBestNewTarget->getPlotCity());
		return true;
	}
	return false;
}

void CvAIGroup::update_City_Invasion_ReinforceCityDefenses()
{
	CLLNode<IDInfo>* pNextUnitNode;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = pNextUnitNode)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pNextUnitNode = nextUnitNode(pUnitNode);
		if(pLoopUnit->plot()->isCity() && pLoopUnit->plot()->getOwnerINLINE()==getOwnerINLINE())
		{
			if(pLoopUnit->plot()->getPlotCity()->getAIGroup_Defense()->UnitsNeeded()>0)
			{
				if(isUnitAllowed(pLoopUnit,AIGROUP_CITY_DEFENSE))
				{
					pLoopUnit->setAIGroup(pLoopUnit->plot()->getPlotCity()->getAIGroup_Defense());
				}
			}
		}
	}
}	
**/
/**
void CvAIGroup::update_Siege()
{
	if(getSiegeGroup()==NULL)
	{
		setDeathDelayed(true);
		return;
	}

	setMissionPlot(getSiegeGroup()->getMissionPlot());
	setMissionArea(getSiegeGroup()->getMissionArea());

	TCHAR szOut[1024];
	sprintf(szOut, "CvAISiege %d,%d::update -- %S\n",getOwnerINLINE(),getID(),GC.getAIGroupInfo(getGroupType()).getDescription());
	gDLL->logMsg("AISiege.log",szOut, false, false);
	if(getMissionPlot()->getPlotCity()!=NULL)
	{
		sprintf(szOut, "TargetCity -- %S\n",getMissionPlot()->getPlotCity()->getNameKey());
		gDLL->logMsg("AISiege.log",szOut, false, false);
	}
	sprintf(szOut, "Current MissionStatus -- %d\n",getSiegeGroup()->getMissionStatus());
	gDLL->logMsg("AISiege.log",szOut, false, false);
	sprintf(szOut, "Units, Siege Units, Units needed -- %d,%d,%d\n",getNumUnits(),getNumUnitCategoryUnits(UNITCATEGORY_SIEGE),UnitsNeeded());
	gDLL->logMsg("AISiege.log",szOut, false, false);	

	if(getSiegeGroup()->getMissionStatus()<=STATUS_CITY_INVASION_PREPARATION)
	{
		AIGroup_Siege(false,100);
		if(getSiegeGroup()->getMissionPlot()!=NULL)
		{
			MoveSiegeToTarget(getMissionPlot()->getcloseSecurePlot(getTeam()));
		}
		AIGroup_Siege(true,0);
		AIGroup_Siege(false,1);

		return;
	}

	if(getSiegeGroup()->getMissionStatus()>=STATUS_CITY_INVASION_SIEGE)
	{
		AIGroup_Siege(false,100);
		MoveSiegeToTarget(getSiegeGroup()->getMissionPlot());
		AIGroup_Siege(false,1);
		return;
	}
}

void CvAIGroup::AIGroup_Siege(bool bCity, int iThreshold)
{
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(GET_PLAYER(getOwnerINLINE()).AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_SIEGE))
		{
			int iSearchRange;
			iSearchRange=std::max(GC.getUnitInfo(pLoopUnit->getUnitType()).getBombardRange(),GC.getUnitInfo(pLoopUnit->getUnitType()).getArcaneRange());
			if(iSearchRange<=0)
			{
				continue;
			}

			//Search Target Within Range
			int iValue;
			int iBestValue=iThreshold;
			CvPlot* pBestPlot=NULL;

			for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
			{
				for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
				{
					CvPlot* pLoopPlot = plotXY(pLoopUnit->getX_INLINE(), pLoopUnit->getY_INLINE(), iDX, iDY);

					if(pLoopPlot)
					{
						if(!bCity || pLoopPlot->isCity())
						{
							if(pLoopUnit->canBombard(pLoopPlot))
							{
								iValue=pLoopUnit->bombard(pLoopPlot,true);
								if(iValue>=iBestValue)
								{
									iBestValue=iValue;
									pBestPlot=pLoopPlot;
								}
							}
						}
					}
				}
			}


			//found plot, now bombard
			if(pBestPlot!=NULL)
			{
				pLoopUnit->bombard(pBestPlot);
			}
		}
	}
}

void CvAIGroup::MoveSiegeToTarget(CvPlot* pTarget)
{
	if(pTarget==NULL)
	{
		return;
	}

	int iPathTurns;
	int iBestValue;

	CvPlot* pBestPlot=NULL;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		int iSiegeRange=std::max(GC.getUnitInfo(pLoopUnit->getUnitType()).getBombardRange(),GC.getUnitInfo(pLoopUnit->getUnitType()).getArcaneRange());

		//if target friendly City 
		if(pTarget->getTeam()==getTeam())
		{
			if(pLoopUnit->generatePath(pTarget,0,true))
			{
				pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pLoopUnit->getPathEndTurnPlot()->getX_INLINE(), pLoopUnit->getPathEndTurnPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
			}
		}
		//if we are out of Range to bombard, move closer
		else if(stepDistance(pTarget->getX_INLINE(),pTarget->getY_INLINE(),pLoopUnit->getX_INLINE(),pLoopUnit->getY_INLINE())>iSiegeRange)
		{
			for(int iPass=0;iPass<2;iPass++)
			{
				pBestPlot=NULL;
				iBestValue=MAX_INT;
				for(int iX=-iSiegeRange;iX<iSiegeRange;iX++)
				{
					for(int iY=-iSiegeRange;iY<iSiegeRange;iY++)
					{
						CvPlot* pLoopPlot=GC.getMapINLINE().plot(pTarget->getX_INLINE()+iX,pTarget->getY_INLINE()+iY);
						if(pLoopPlot!=NULL)
						{
							if(iPass==1 || (pLoopPlot->getNumUnits()>0 && pLoopPlot->getBestDefender(getOwnerINLINE())!=NULL))
							{
								if(!pLoopPlot->isVisibleEnemyDefender(pLoopUnit))
								{
									if(pLoopUnit->generatePath(pLoopPlot,0,true,&iPathTurns))
									{
										if(iPathTurns<iBestValue)
										{
											iBestValue=iPathTurns;
											pBestPlot=pLoopPlot;
										}
									}
								}
							}
						}
					}
				}
				if(pBestPlot!=NULL)
				{
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
					break;
				}
			}
		}
	}
}
**/

void CvAIGroup::TargetSpell(CvPlot* pTarget, int iThreshold)
{
	if(pTarget==NULL)
	{
		return;
	}

	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
		{
			if(pLoopUnit->getUnitInfo().getArcaneRange()>=stepDistance(pLoopUnit->getX_INLINE(),pLoopUnit->getY_INLINE(),pTarget->getX_INLINE(),pTarget->getY_INLINE()))
			{
				for(int iI=0;iI<GC.getNumSpellInfos();iI++)
				{
					if(GC.getSpellInfo((SpellTypes)iI).isTargetedHostile())
					{
						if(pLoopUnit->canCastAt((SpellTypes)iI,pTarget))
						{
							if(pLoopUnit->castDamage((SpellTypes)iI,pTarget,true)/std::max(1,GC.getSpellInfo((SpellTypes)iI).getManaCost())>=iThreshold)
							{
								pLoopUnit->cast((SpellTypes)iI,pTarget);
							}
						}
					}
				}
			}
		}
	}
}

void CvAIGroup::update_Mana_Node()
{
	CvPlayerAI &kPlayer = GET_PLAYER(getOwnerINLINE());
	if(getMissionPlot() == NULL || getMissionPlot()->getBonusType(getTeam()) == NO_BONUS)
	{
		setDeathDelayed(true);
		return;
	}

	int iBestValue=0;
	int iValue;
	int iBestBonus = NO_BONUS;

	for(int iI = 0; iI < GC.getNumBonusInfos(); iI++)
	{
		iValue = kPlayer.AI_ValueManaNode(iI);
		if(iValue > iBestValue)
		{
			iBestValue = iValue;
			iBestBonus = iI;
		}
	}

	if(iBestBonus == NO_BONUS)
	{
		setDeathDelayed(true);
		return;
	}

	BuildTypes eBestBuild = NO_BUILD;
	for(int iI = 0; iI < GC.getNumBuildInfos(); iI++) {
		if(GC.getBuildInfo((BuildTypes)iI).getImprovement() != NO_IMPROVEMENT) {
			if(GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo((BuildTypes)iI).getImprovement()).getBonusConvert() == iBestBonus)  {
				eBestBuild = (BuildTypes)iI;
			}
		}
	}

	if(eBestBuild == NO_BUILD || GC.getBuildInfo(eBestBuild).getImprovement() == getMissionPlot()->getImprovementType())
	{
		setDeathDelayed(true);
		return;
	}

	// 1. Move to the plot
	if(!atPlot(getMissionPlot())) 
		Move(getMissionPlot());



	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
		{
			if(pLoopUnit->atPlot(getMissionPlot()))
			{
				if(pLoopUnit->canBuild(getMissionPlot(), eBestBuild))
					pLoopUnit->getGroup()->pushMission(MISSION_BUILD, eBestBuild);
				else {
					if(pLoopUnit->canCast(GC.getInfoTypeForString("SPELL_DISPEL_MAGIC"), false)) {
						pLoopUnit->cast(GC.getInfoTypeForString("SPELL_DISPEL_MAGIC"));
					}
				}
			}
		}
	}
}

void CvAIGroup::castAnyTarget(int iThreshold, bool bOtherArea)
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iDX,iDY;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_WARWIZARD))
		{
			int iSearchRange=pLoopUnit->getUnitInfo().getArcaneRange();

			for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
			{
				for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
				{
					CvPlot* pLoopPlot	= plotXY(pLoopUnit->plot()->getX_INLINE(), pLoopUnit->plot()->getY_INLINE(), iDX, iDY);
					if(pLoopPlot!=NULL)
					{
						if(bOtherArea || pLoopPlot->getArea()==pLoopUnit->getArea())
						{
							if(pLoopPlot->getNumUnits()>0)
							{
								for(int iI=0;iI<GC.getNumSpellInfos();iI++)
								{
									if(GC.getSpellInfo((SpellTypes)iI).isTargetedHostile())
									{
										if(pLoopUnit->canCastAt((SpellTypes)iI,pLoopPlot))
										{
											if(pLoopUnit->castDamage((SpellTypes)iI,pLoopPlot,true)/std::max(1,GC.getSpellInfo((SpellTypes)iI).getManaCost())>=iThreshold)
											{
												pLoopUnit->cast((SpellTypes)iI,pLoopPlot);
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

void CvAIGroup::attackAnyUnit(int iOddsThreshold)
{
	int iDX, iDY;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canAttack() && pLoopUnit->canMove())
		{
			int iSearchRange = pLoopUnit->baseMoves();

			for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
			{
				for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
				{
					if(pLoopUnit != NULL) {	//pLoopUnit might have been killed in Launchattack
						CvPlot* pLoopPlot = plotXY(pLoopUnit->plot()->getX_INLINE(), pLoopUnit->plot()->getY_INLINE(), iDX, iDY);
						if(pLoopPlot != NULL)
						{
							if(pLoopPlot->getArea() == pLoopUnit->getArea())
							{
								if(pLoopPlot->getNumUnits() > 0 && pLoopPlot->isOwned() || 
									(pLoopPlot->isCity() && pLoopPlot->getTeam() != getTeam() && GET_TEAM(getTeam()).isAtWar(pLoopPlot->getTeam())))
								{
									if(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE()))
									{
										if(pLoopPlot->getTeam() == getTeam()|| pLoopPlot->getVisibleEnemyDefender(getOwnerINLINE()) == NULL || pLoopPlot->getVisibleEnemyDefender(getOwnerINLINE())->getTeam() == pLoopPlot->getTeam())
										{
											int iEnemyStrength;
											int iOurStrength;
											compareAttackStrength(pLoopPlot, &iEnemyStrength, &iOurStrength);
											if(iOurStrength * iOddsThreshold > iEnemyStrength * 100)
											{
												launchAttack(pLoopPlot);
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

	HeroattackAnyUnit();
}

void CvAIGroup::HeroattackAnyUnit()
{
	int iDX, iDY;
	int iPathTurns;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(pLoopUnit->canAttack() && pLoopUnit->canMove())
		{
			int iSearchRange=std::max(1, pLoopUnit->baseMoves() / 2);
			int iValue=0;
			int iBestValue=0;
			CvPlot* pBestPlot=NULL;

			for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
			{
				for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
				{
					CvPlot* pLoopPlot	= plotXY(pLoopUnit->plot()->getX_INLINE(), pLoopUnit->plot()->getY_INLINE(), iDX, iDY);
					if(pLoopPlot!=NULL)
					{
						if(pLoopPlot->getArea()==pLoopUnit->getArea())
						{
							if(pLoopPlot->getNumUnits()>0 && pLoopPlot->isOwned())
							{
								if(pLoopPlot->isVisibleEnemyUnit(getOwnerINLINE()))
								{
									if(pLoopPlot->getTeam()==getTeam() && pLoopPlot->getVisibleEnemyDefender(getOwnerINLINE())!=NULL)
									{
										if(pLoopUnit->generatePath(pLoopPlot,0,true,&iPathTurns) && (iPathTurns<=1))
										{
											iValue=getCombatOdds(pLoopUnit,pLoopPlot->getVisibleEnemyDefender(getOwnerINLINE()));
											if(iValue >= 95 || pLoopUnit->isHasSecondChance()) {
												iValue*=pLoopUnit->getPower();
												if(iValue>iBestValue)
												{
													iValue = iBestValue;
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

			if(pBestPlot != NULL)
			{
				pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO, pBestPlot->getX_INLINE(), pBestPlot->getY_INLINE(), MOVE_DIRECT_ATTACK);
			}
		}
	}
}

void CvAIGroup::update_Naval_Reserve()
{
	healUnits(30);

	update_Reserve_Reinforce_NavalInvasion();
	
	update_Reserve_Reinforce_NavalSettle();

	update_Reserve_Reinforce_NavalCounter();

	update_Reserve_Reinforce_NavalExplorer();

	//TODO add function that forces Ships to return to safe plots
	checkForPossibleUpgrade();
}

//add Ships to Naval Invasion Group
void CvAIGroup::update_Reserve_Reinforce_NavalInvasion()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_INVASION && pAIGroup->getMissionCity()!=NULL)
		{
			if(pAIGroup->getMissionArea()==getMissionArea() || pAIGroup->getMissionCity()->getArea()==getMissionArea())
			{
				while(!pAIGroup->isAllUnitsForNavalGroup())
				{
					CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionCity()->plot(),pAIGroup, false);
					if(pUnit==NULL)
						break;
					pUnit->setAIGroup(pAIGroup);
				}
			}
		}
	}
}

//add Ships to NavalGroup
void CvAIGroup::update_Reserve_Reinforce_NavalSettle()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_SETTLE && pAIGroup->getMissionCity()!=NULL)
		{
			if(pAIGroup->getMissionArea()==getMissionArea() || pAIGroup->getMissionCity()->getArea()==getMissionArea())
			{
				while(!pAIGroup->isAllUnitsForNavalGroup())
				{
					CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionCity()->plot(),pAIGroup, false);
					if(pUnit==NULL)
						break;
					pUnit->setAIGroup(pAIGroup);
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_Reinforce_NavalExplorer()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_EXPLORER && pAIGroup->getMissionArea()==getMissionArea())
		{
			if(pAIGroup->UnitsNeeded()>0 && pAIGroup->getMissionPlot()!=NULL)
			{
				while(pAIGroup->UnitsNeeded()>0)
				{
					CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionPlot(),pAIGroup, false);
					if(pUnit==NULL)
						break;
					pUnit->setAIGroup(pAIGroup);
				}
			}
		}
	}
}

void CvAIGroup::update_Reserve_Reinforce_NavalCounter()
{
	CvPlayerAI &kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_COUNTER && pAIGroup->getMissionArea()==getMissionArea())
		{
			if(pAIGroup->UnitsNeeded()>0)
			{
				while(pAIGroup->UnitsNeeded()>0)
				{
					CvUnit* pUnit=getCloseUnit(pAIGroup->getMissionTargetUnit()->plot(),pAIGroup, false);
					if(pUnit==NULL)
						break;
					pUnit->setAIGroup(pAIGroup);
				}
			}
		}
	}
}

void CvAIGroup::update_Naval_Counter()
{
	//Target still valid?
	if(!update_Counter_isTargetStillValid())
	{
		return;
	}

	//Powerful Enough to Launch attack?
	//ToDo add in Calculation for Damage Spells pre Combat
	int iEnemyStrength, iOurStrength;
	compareAttackStrength(getMissionPlot(),&iEnemyStrength,&iOurStrength);

	if(iOurStrength>iEnemyStrength*1.2)
	{
		launchAttack(getMissionPlot());
	}
	else
	{
		prepareAttack(getMissionPlot(),MAX_INT,1);
	}

	checkForPossibleUpgrade();
}

bool CvAIGroup::isAllUnitsForNavalGroup()
{
	if(UnitsNeeded(UNITCATEGORY_SETTLE)>0)
	{
		return false;
	}
	if(UnitsNeeded(UNITCATEGORY_COMBAT)>0)
	{
		return false;
	}
	if(UnitsNeeded(UNITCATEGORY_TRANSPORT_NAVAL)>0)
	{
		return false;
	}
	if(UnitsNeeded(UNITCATEGORY_COMBAT_NAVAL)>0)
	{
		return false;
	}
	return true;
}

void CvAIGroup::loadIntoShip(CvUnit* pUnit)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iPass;

	for (iPass = 0; iPass < 2; iPass++)
	{
		pUnitNode = pUnit->plot()->headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = pUnit->plot()->nextUnitNode(pUnitNode);

			if(pLoopUnit->getAIGroup()==this)
			{
				if (pUnit->canLoadUnit(pLoopUnit, pUnit->plot()))
				{
					if ((iPass == 0) ? (pLoopUnit->getOwnerINLINE() == getOwnerINLINE()) : (pLoopUnit->getTeam() == getTeam()))
					{
						pUnit->setTransportUnit(pLoopUnit);
						break;
					}
				}
			}
		}

		if (pUnit->isCargo())
		{
			break;
		}
	}
}

//returns Naval Group that needs a Settler, iArea is the Area of the Settler on land
CvAIGroup* CvAIGroup::getNavalSettleGroupNeedSettler(int iArea)
{
	CvPlayerAI& kPlayer=GET_PLAYER(getOwnerINLINE());
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==AIGROUP_NAVAL_SETTLE && pAIGroup->getMissionCity()!=NULL && pAIGroup->getMissionCity()->getArea()==iArea)
		{
			if(pAIGroup->UnitsNeeded(UNITCATEGORY_SETTLE)>0)
			{
				return pAIGroup;
			}
		}
	}

	return NULL;
}

void CvAIGroup::update_Naval_Settle()
{
	CvPlayerAI& kPlayer=GET_PLAYER(getOwnerINLINE());

	//are we already there?
	if(getMissionPlot()!=NULL)
	{
		CvUnit* pSettler=NULL;
		for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
		{
			CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
			if(kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_SETTLE))
			{
				pSettler=pLoopUnit;
			}
		}
		if(pSettler!=NULL && pSettler->atPlot(getMissionPlot()))
		{
			pSettler->getGroup()->pushMission(MISSION_FOUND, -1, -1, 0, false, false, MISSIONAI_FOUND, getMissionPlot());
			if(getMissionPlot()->isCity() && getMissionPlot()->getOwnerINLINE()==getOwnerINLINE())
			{
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
					if(pLoopUnit->atPlot(getMissionPlot()))
					{
						if(pLoopUnit->getOwnerINLINE()==getOwnerINLINE() && GC.getUnitInfo(pLoopUnit->getUnitType()).getCombatDefense()>0)
						{
							if(pLoopUnit->plot()->getPlotCity()->getAIGroup_Defense()->UnitsNeeded()>0)
							{
								pLoopUnit->setAIGroup(pLoopUnit->plot()->getPlotCity()->getAIGroup_Defense());
							}
						}
					}
				}
			}
			setMissionPlot(NULL);
		}
	}

	//TODO: also check if Naval Mission must be aborted
	if(getMissionArea()==FFreeList::INVALID_INDEX || getMissionPlot()==NULL 
		|| (kPlayer.AI_foundValue(getMissionPlot()->getX_INLINE(),getMissionPlot()->getY_INLINE())==0))
	{
		setDeathDelayed(true);
		return;
	}

	if(getMissionStatus()==NO_AIGROUP_STATUS)
	{
		if(getMissionCity()==NULL)
		{
			int iValue;
			int iLoop;
			int iBestValue=0;
			CvCity* pBestCity=NULL;
			for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				if(pLoopCity->plot()->isAdjacentToArea(getMissionArea()))
				{
					iValue=pLoopCity->getPopulation()*100;
					int DifX=abs(getMissionPlot()->getX_INLINE()-pLoopCity->getX_INLINE());
					int DifY=abs(getMissionPlot()->getY_INLINE()-pLoopCity->getY_INLINE());
					iValue /=(DifX+10);
					iValue /=(DifY+10);

					if(iValue>iBestValue)
					{
						iBestValue=iValue;
						pBestCity=pLoopCity;
					}
				}
			}
			if(pBestCity!=NULL)
			{
				setMissionCity(pBestCity);
			}
		}

		if(getMissionCity()!=NULL)
		{
			if(isAllUnitsWithinRange(getMissionCity()->plot(),0) && isAllUnitsForNavalGroup())
			{
				//load Units in Transports
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
					
					if(kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_SETTLE)
						|| kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_COMBAT))
					{
						loadIntoShip(pLoopUnit);
					}

				}

				setMissionStatus(STATUS_NAVAL_IN_TRANSIT);
			}
			else
			{
				//send Units to MissionCity
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));

					if(!pLoopUnit->atPlot(getMissionCity()->plot()))
					{
						pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO,getMissionCity()->plot()->getX_INLINE(),getMissionCity()->plot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
					}
				}
			}
		}
	}
	if(getMissionStatus()==STATUS_NAVAL_IN_TRANSIT)
	{
		//move to MissionPlot
		CvPlot* pAssaultPlot=NULL;
		for (int iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), ((DirectionTypes)iJ));
			if(pAdjacentPlot!=NULL)
			{
				if(pAdjacentPlot->getArea()==getMissionArea())
				{
					pAssaultPlot=pAdjacentPlot;
					break;
				}
			}
		}

		if(pAssaultPlot!=NULL)
		{
			if(isAllUnitsWithinRange(pAssaultPlot,0))
			{
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
					
					pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO,getMissionPlot()->getX_INLINE(),getMissionPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);

					if(pLoopUnit->atPlot(getMissionPlot()))
					{
						if(pLoopUnit->canFound(getMissionPlot()))
						{
							pLoopUnit->found();
						}
					}
				}
			}
			else
			{
				//Move Units to Assault Plot
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));

					if(!pLoopUnit->atPlot(pAssaultPlot))
					{
						if(!pLoopUnit->isCargo())
						{
							pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO,pAssaultPlot->getX_INLINE(),pAssaultPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
						}
					}
				}
			}
		}
	}
}

void CvAIGroup::update_Naval_Invasion()
{
	CvPlayerAI& kPlayer=GET_PLAYER(getOwnerINLINE());

	//did we already took the city?

	//TODO: also check if Naval Mission must be aborted
	if(getMissionArea()==FFreeList::INVALID_INDEX || getMissionPlot()==NULL 
		|| getMissionPlot()->getTeam() == getTeam() || !getMissionPlot()->isCity())
	{
		setDeathDelayed(true);
		return;
	}

	if(getMissionStatus()==NO_AIGROUP_STATUS)
	{
		//pick City where we want to load units into ships (rally point)
		if(getMissionCity()==NULL)
		{
			int iValue;
			int iLoop;
			int iBestValue=0;
			CvCity* pBestCity=NULL;
			for (CvCity* pLoopCity = kPlayer.firstCity(&iLoop); pLoopCity != NULL; pLoopCity = kPlayer.nextCity(&iLoop))
			{
				if(pLoopCity->plot()->isAdjacentToArea(getMissionArea()))
				{
					iValue=pLoopCity->getPopulation()*100;
					int DifX=abs(getMissionPlot()->getX_INLINE()-pLoopCity->getX_INLINE());
					int DifY=abs(getMissionPlot()->getY_INLINE()-pLoopCity->getY_INLINE());
					iValue /=(DifX+10);
					iValue /=(DifY+10);

					if(iValue>iBestValue)
					{
						iBestValue=iValue;
						pBestCity=pLoopCity;
					}
				}
			}
			if(pBestCity!=NULL)
			{
				setMissionCity(pBestCity);

				TCHAR szOut[1024];
				sprintf(szOut, "%d,%d :: setMissionCity to Coords -- %d, %d\n",getOwnerINLINE(),getID(),pBestCity->getX_INLINE(),pBestCity->getY_INLINE());
				gDLL->logMsg("NavalInvasionAIGroup.log",szOut, false, false);

			}
		}

		if(getMissionCity()!=NULL)
		{

			if(isAllUnitsWithinRange(getMissionCity()->plot(),0) && isAllUnitsForNavalGroup())
			{
				//load Units in Transports
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
					
					if(kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_SETTLE)
						|| kPlayer.AI_isUnitCategory(pLoopUnit->getUnitType(),UNITCATEGORY_COMBAT))
					{
						loadIntoShip(pLoopUnit);
					}

				}

				setMissionStatus(STATUS_NAVAL_IN_TRANSIT);

				TCHAR szOut[1024];
				sprintf(szOut, "%d,%d :: units loaded into transport -- \n",getOwnerINLINE(),getID());
				gDLL->logMsg("NavalInvasionAIGroup.log",szOut, false, false);

			}
			else
			{
				int iNumUnitsAtRallyPoint=0;
				//send Units to MissionCity
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));

					if(!pLoopUnit->atPlot(getMissionCity()->plot()))
					{
						pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO,getMissionCity()->plot()->getX_INLINE(),getMissionCity()->plot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
					}
					else
					{
						iNumUnitsAtRallyPoint++;
					}
				}

				TCHAR szOut[1024];
				sprintf(szOut, "%d,%d :: wait at Rally Point with num units -- %d\n",getOwnerINLINE(),getID(),iNumUnitsAtRallyPoint);
				gDLL->logMsg("NavalInvasionAIGroup.log",szOut, false, false);

			}
		}
	}
	if(getMissionStatus()==STATUS_NAVAL_IN_TRANSIT)
	{
		//move to MissionPlot
		CvPlot* pAssaultPlot=NULL;
		for (int iJ = 0; iJ < NUM_DIRECTION_TYPES; iJ++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getMissionPlot()->getX_INLINE(), getMissionPlot()->getY_INLINE(), ((DirectionTypes)iJ));
			if(pAdjacentPlot!=NULL)
			{
				if(pAdjacentPlot->getArea()==getMissionArea())
				{
					pAssaultPlot=pAdjacentPlot;
					break;
				}
			}
		}

		if(pAssaultPlot!=NULL)
		{
			if(isAllUnitsWithinRange(pAssaultPlot,0))
			{
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
					
					if(pLoopUnit->canAttack())
					{
						pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO,getMissionPlot()->getX_INLINE(),getMissionPlot()->getY_INLINE(),MOVE_DIRECT_ATTACK);
					}

				}

				TCHAR szOut[1024];
				sprintf(szOut, "%d,%d :: Aussault done -- \n",getOwnerINLINE(),getID());
				gDLL->logMsg("NavalInvasionAIGroup.log",szOut, false, false);

			}
			else
			{
				//Move Units to Assault Plot
				for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
				{
					CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));

					if(!pLoopUnit->atPlot(pAssaultPlot))
					{
						if(!pLoopUnit->isCargo())
						{
							pLoopUnit->getGroup()->pushMission(MISSION_MOVE_TO,pAssaultPlot->getX_INLINE(),pAssaultPlot->getY_INLINE(),MOVE_DIRECT_ATTACK);
						}
					}
				}

				TCHAR szOut[1024];
				sprintf(szOut, "%d,%d :: move units to Assault plot -- %d, %d\n",getOwnerINLINE(),getID(),pAssaultPlot->getX_INLINE(), pAssaultPlot->getY_INLINE());
				gDLL->logMsg("NavalInvasionAIGroup.log",szOut, false, false);

			}
		}
	}
}

void CvAIGroup::update_Naval_Explorer()
{
	CvPlot* pLoopPlot;

	bool bValid=false;
	for(int iI=0;iI<GC.getMapINLINE().numPlots();iI++)
	{
		pLoopPlot=GC.getMapINLINE().plotByIndex(iI);
		if(pLoopPlot->getArea()==getMissionArea())
		{
			if(!pLoopPlot->isRevealed(getTeam(),false))
			{
				bValid=true;
				break;
			}
		}
	}

	if(!bValid)
	{
		setDeathDelayed(true);
		return;
	}

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnitAI* pLoopUnit = static_cast<CvUnitAI*>(::getUnit(pUnitNode->m_data));
		if(pLoopUnit->canMove())
			pLoopUnit->AI_exploreSeaMove();
	}
}

//checks if all Units are there to fill a Naval Settle Group
bool CvAIGroup::canFillNavalSettleWithReserveUnits(CvAIGroup* pReserve)
{
	if(UnitsNeeded(UNITCATEGORY_SETTLE)>0)
	{
		if(getMissionCity()!=NULL)
		{
			CvAIGroup* pGroup=getMissionCity()->area()->getReserveGroup(getOwnerINLINE());
			if(pGroup!=NULL)
			{
				if(pGroup->getNumUnitCategoryUnits(UNITCATEGORY_SETTLE)>=UnitsNeeded(UNITCATEGORY_SETTLE))
				{
					if(pGroup->getNumUnitCategoryUnits(UNITCATEGORY_COMBAT)>=UnitsNeeded(UNITCATEGORY_COMBAT))
					{
						return true;
					}
				}
			}
		}
	}
	return false;
}
//return true if there are enough Units in the Reserve to Fill the Current Unit
bool CvAIGroup::canFillWithReserveUnits(CvAIGroup* pReserve)
{
	switch(getGroupType())
	{
		case AIGROUP_NAVAL_SETTLE:
			return canFillNavalSettleWithReserveUnits(pReserve);
		default:
			break;
	}

	int iNeededPower=UnitPowerNeeded();
	int iTotalPowerReserve=0;
	for (CLLNode<IDInfo>* pUnitNode = pReserve->headUnitNode(); pUnitNode != NULL; pUnitNode = pReserve->nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		iTotalPowerReserve+=pLoopUnit->maxCombatStr(NULL,NULL);
	}

	return iTotalPowerReserve>=iNeededPower;
}

int CvAIGroup::UnitPowerNeeded()
{
	int UnitPowerNeeded=0;
	switch(getGroupType())
	{
		case AIGROUP_CITY_INVASION:
			UnitPowerNeeded=calculateEnemyStrength(getMissionPlot(), 2,true,false,true);
			break;
		case AIGROUP_COUNTER:
		case AIGROUP_NAVAL_COUNTER:
		case AIGROUP_DESTROY_LAIR:
			UnitPowerNeeded=calculateEnemyStrength(getMissionPlot(), 0,false,false,true);
			break;
		default:
			break;
	}

	UnitPowerNeeded-=getGroupPowerWithinRange(NULL,MAX_INT);

	UnitPowerNeeded*=7;
	UnitPowerNeeded/=4;

	return std::max(0,UnitPowerNeeded);
}

/**
calculates the Strength of Enemies
pPlot, iRange : Only Enemies in range of pPlot are counted (maybe change it to iMaxTurns?
bOwnerTeamOnly : Only Enemies that belong to owner of pPlot are counted
bAttackOnly : Only Enemies that can attack are counted
bDefensiveBonuses : Enemies are counted with their defensive Bonuses (terrain/Cities)
**/
int CvAIGroup::calculateEnemyStrength(CvPlot* pPlot,int iRange, bool bOwnerTeamOnly, bool bAttackOnly, bool bDefensiveBonuses) const
{
	if(pPlot==NULL)
	{
		return 0;
	}
	int iEnemyStrength = 0;
	int iSearchRange = iRange;
	int iCityDefenseModifier=100;
	iCityDefenseModifier+=(pPlot->isCity()) ? pPlot->getPlotCity()->getDefenseModifier(false) : 0;

	for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
	{
		for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
		{
			CvPlot* pLoopPlot = plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				for(CLLNode<IDInfo>* pUnitNode=pLoopPlot->headUnitNode(); pUnitNode != NULL; pUnitNode = pLoopPlot->nextUnitNode(pUnitNode))
				{
					CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
					if(!bOwnerTeamOnly || pLoopUnit->getTeam()==pLoopPlot->getTeam())
					{
						if(!bAttackOnly || pLoopUnit->canAttack())
						{
							iEnemyStrength+=pLoopUnit->maxCombatStr((bDefensiveBonuses ? pLoopUnit->plot() : NULL), NULL);
/**
							iTemp=pLoopUnit->getPower();
							if(pPlot->isCity())
							{
								iTemp*=100+pLoopUnit->cityDefenseModifier();
								iTemp/=100;
							}
							iTemp*=iCityDefenseModifier;
							iTemp/=100;

							iEnemyStrength+=iTemp;
**/
						}
					}
				}
			}
		}
	}

	return iEnemyStrength;
}

bool CvAIGroup::isAllUnitsWithinRange(CvPlot* pPlot, int iRange)
{
	int iPathTurns;

	if(pPlot==NULL)
		return true;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(stepDistance(pLoopUnit->getX_INLINE(),pLoopUnit->getY_INLINE(),pPlot->getX_INLINE(),pPlot->getY_INLINE())>iRange)
		{
			if(pLoopUnit->generatePath(pPlot,0,false,&iPathTurns))
			{
				if(iPathTurns>iRange)
				{
					return false;
				}
			}
		}
	}
	return true;
}

int CvAIGroup::getNumPossibleAttackers(CvPlot* pPlot, int iMaxTurns) const
{
	int iCount=0;
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(isPossibleAttacker(pLoopUnit, pPlot, iMaxTurns))
		{
			iCount++;
		}
	}

	return iCount;
}

bool CvAIGroup::isPossibleAttacker(CvUnit* pUnit, CvPlot* pPlot, int iMaxTurns) const
{
	if(pUnit->getAIGroup()!=this)
	{
		return false;
	}

	//do not count Heroes, we might want to preserve them!
	if(pUnit->AI_getUnitAIType()==UNITAI_HERO)
	{
		return false;
	}

	int iPathTurns;
	if(pUnit->canAttack())
	{
		if(iMaxTurns==MAX_INT || pPlot==NULL || (pUnit->canMove() && pUnit->generatePath(pPlot,0,false,&iPathTurns)))
		{
			if(iMaxTurns==MAX_INT || iPathTurns<=iMaxTurns)
			{
				return true;
			}
		}
	}

	return false;
}

//set iMaxTurns==MAX_INT if we do not want to do any pathfinding
//set bWounded to false to count wounded Units as if they were at full health
int CvAIGroup::getGroupPowerWithinRange(CvPlot* pPlot, int iMaxTurns, bool bCountWoundedAsFullHealth, bool bDefensiveBonuses) const
{
	int iPower=0;

	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		if(isPossibleAttacker(pLoopUnit,pPlot, iMaxTurns))
		{
			if(bCountWoundedAsFullHealth)
			{
				iPower+=pLoopUnit->maxCombatStr((bDefensiveBonuses ? pLoopUnit->plot() : NULL), NULL);
			}
			else
			{
				iPower+=pLoopUnit->currEffectiveStr((bDefensiveBonuses ? pLoopUnit->plot() : NULL), NULL);
			}
		}
	}
	return iPower;
}

bool CvAIGroup::canMoveIntoPlot(const CvPlot* pTarget, bool bWater) const
{
	if(pTarget->isPeak() || pTarget->isNonPlanar())
	{
		return false;
	}
	if(pTarget->isWater() && !bWater)
	{
		return false;
	}
	return true;
}

void CvAIGroup::Fortify()
{
	for (CLLNode<IDInfo>* pUnitNode = headUnitNode(); pUnitNode != NULL; pUnitNode = nextUnitNode(pUnitNode))
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pLoopUnit->getGroup()->pushMission(MISSION_FORTIFY);
	}
}
