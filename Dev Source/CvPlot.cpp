// plot.cpp

#include "CvGameCoreDLL.h"
#include "CvPlot.h"
#include "CvCity.h"
#include "CvUnit.h"
#include "CvGlobals.h"
#include "CvArea.h"
#include "CvGameAI.h"
#include "CvDLLInterfaceIFaceBase.h"
#include "CvDLLSymbolIFaceBase.h"
#include "CvDLLEntityIFaceBase.h"
#include "CvDLLPlotBuilderIFaceBase.h"
#include "CvDLLEngineIFaceBase.h"
#include "CvDLLFlagEntityIFaceBase.h"
#include "CvMap.h"
#include "CvPlayerAI.h"
#include "CvTeamAI.h"
#include "CvGameCoreUtils.h"
#include "CvRandom.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "CvArtFileMgr.h"
#include "CyArgsList.h"
#include "CvDLLPythonIFaceBase.h"
#include "CvEventReporter.h"
#include "CvGameTextMgr.h"
#include "CvMapGenerator.h"

// lfgr UI 11/2020: Allow cycling through units in plot help
#include "PlotHelpCycling.h"

#define STANDARD_MINIMAP_ALPHA		(0.6f)

/*************************************************************************************************/
/** Skyre Mod                                                                                   **/
/** BETTER AI (Lanun Pirate Coves) merged Sephi                                                 **/
/**						                                            							**/
/*************************************************************************************************/
bool CvPlot::c_bPirateCoveTypesLoaded = false;
std::set<ImprovementTypes> CvPlot::c_aePirateCoveTypes;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/


// Public Functions...

CvPlot::CvPlot()
{
	m_aiYield = new short[NUM_YIELD_TYPES];

	m_aiCulture = NULL;
	m_aiCultureRangeForts = NULL; // Super Forts *culture*
	m_aiFoundValue = NULL;
	m_aiPlayerCityRadiusCount = NULL;
	m_aiPlotGroup = NULL;
	m_aiVisibilityCount = NULL;
	m_aiStolenVisibilityCount = NULL;
	m_aiBlockadedCount = NULL;
	m_aiRevealedOwner = NULL;
	m_abRiverCrossing = NULL;
	m_abRevealed = NULL;
	m_aeRevealedImprovementType = NULL;
	m_aeRevealedRouteType = NULL;
	m_paiBuildProgress = NULL;
	m_apaiCultureRangeCities = NULL;
	m_apaiInvisibleVisibilityCount = NULL;

	m_pFeatureSymbol = NULL;
	m_pPlotBuilder = NULL;
	m_pRouteSymbol = NULL;
	m_pRiverSymbol = NULL;
	m_pFlagSymbol = NULL;
	m_pFlagSymbolOffset = NULL;
	m_pCenterUnit = NULL;

	m_szScriptData = NULL;
/*************************************************************************************************/
/** Skyre Mod                                                                                   **/
/** BETTER AI (Lanun Pirate Coves) merged Sephi                                                 **/
/**						                                            							**/
/*************************************************************************************************/
    if (!c_bPirateCoveTypesLoaded)
    {
        ImprovementTypes ePirateCove = (ImprovementTypes)GC.getDefineINT("PIRATE_COVE_IMPROVEMENT");

        while (ePirateCove != NO_IMPROVEMENT)
        {
            c_aePirateCoveTypes.insert(ePirateCove);
            ePirateCove = (ImprovementTypes)GC.getImprovementInfo(ePirateCove).getImprovementUpgrade();
        }

        c_bPirateCoveTypesLoaded = true;
    }
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
	m_apaiPlayerDangerCache = NULL;
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	

	reset(0, 0, true);
}


CvPlot::~CvPlot()
{
	uninit();

	SAFE_DELETE_ARRAY(m_aiYield);
}

void CvPlot::init(int iX, int iY)
{
	//--------------------------------
	// Init saved data
	reset(iX, iY);

	//--------------------------------
	// Init non-saved data

	//--------------------------------
	// Init other game data
}


void CvPlot::uninit()
{
	SAFE_DELETE_ARRAY(m_szScriptData);

	gDLL->getFeatureIFace()->destroy(m_pFeatureSymbol);
	if(m_pPlotBuilder)
	{
		gDLL->getPlotBuilderIFace()->destroy(m_pPlotBuilder);
	}
	gDLL->getRouteIFace()->destroy(m_pRouteSymbol);
	gDLL->getRiverIFace()->destroy(m_pRiverSymbol);
	gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbol);
	gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbolOffset);
	m_pCenterUnit = NULL;

	deleteAllSymbols();

	SAFE_DELETE_ARRAY(m_aiCulture);
	SAFE_DELETE_ARRAY(m_aiCultureRangeForts); // Super Forts *culture*
	SAFE_DELETE_ARRAY(m_aiFoundValue);
	SAFE_DELETE_ARRAY(m_aiPlayerCityRadiusCount);
	SAFE_DELETE_ARRAY(m_aiPlotGroup);

	SAFE_DELETE_ARRAY(m_aiVisibilityCount);
	SAFE_DELETE_ARRAY(m_aiStolenVisibilityCount);
	SAFE_DELETE_ARRAY(m_aiBlockadedCount);
	SAFE_DELETE_ARRAY(m_aiRevealedOwner);

	SAFE_DELETE_ARRAY(m_abRiverCrossing);
	SAFE_DELETE_ARRAY(m_abRevealed);

	SAFE_DELETE_ARRAY(m_aeRevealedImprovementType);
	SAFE_DELETE_ARRAY(m_aeRevealedRouteType);

	SAFE_DELETE_ARRAY(m_paiBuildProgress);

	if (NULL != m_apaiCultureRangeCities)
	{
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			SAFE_DELETE_ARRAY(m_apaiCultureRangeCities[iI]);
		}
		SAFE_DELETE_ARRAY(m_apaiCultureRangeCities);
	}

	if (NULL != m_apaiInvisibleVisibilityCount)
	{
		for (int iI = 0; iI < MAX_TEAMS; ++iI)
		{
			SAFE_DELETE_ARRAY(m_apaiInvisibleVisibilityCount[iI]);
		}
		SAFE_DELETE_ARRAY(m_apaiInvisibleVisibilityCount);
	}

/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
	if (NULL != m_apaiPlayerDangerCache)
	{
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			SAFE_DELETE_ARRAY(m_apaiPlayerDangerCache[iI]);
		}
		SAFE_DELETE_ARRAY(m_apaiPlayerDangerCache);
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	m_units.clear();
}

// FUNCTION: reset()
// Initializes data members that are serialized.
void CvPlot::reset(int iX, int iY, bool bConstructorCall)
{
	int iI;

	//--------------------------------
	// Uninit class
	uninit();

	m_iX = iX;
	m_iY = iY;
	m_iArea = FFreeList::INVALID_INDEX;
	m_pPlotArea = NULL;
	m_iFeatureVariety = 0;
	m_iOwnershipDuration = 0;
	m_iImprovementDuration = 0;
	m_iUpgradeProgress = 0;
	m_iForceUnownedTimer = 0;
	m_iCityRadiusCount = 0;
	m_iRiverID = -1;
	m_iMinOriginalStartDist = -1;
	m_iReconCount = 0;
	m_iRiverCrossingCount = 0;

	// Super Forts begin *canal* *choke*
	m_iCanalValue = 0;
	m_iChokeValue = 0;
	// Super Forts end
	// Super Forts begin *bombard*
	m_iDefenseDamage = 0;
	m_bBombarded = false;
	// Super Forts end

	m_bStartingPlot = false;
	m_bHills = false;
	m_bNOfRiver = false;
	m_bWOfRiver = false;
	m_bIrrigated = false;
	m_bPotentialCityWork = false;
	m_bShowCitySymbols = false;
	m_bFlagDirty = false;
	m_bPlotLayoutDirty = false;
	m_bLayoutStateWorked = false;

	m_eOwner = NO_PLAYER;
	m_ePlotType = PLOT_OCEAN;
	m_eTerrainType = NO_TERRAIN;
	m_eFeatureType = NO_FEATURE;
	m_eBonusType = NO_BONUS;
	m_eImprovementType = NO_IMPROVEMENT;
	m_eRouteType = NO_ROUTE;
	m_eRiverNSDirection = NO_CARDINALDIRECTION;
	m_eRiverWEDirection = NO_CARDINALDIRECTION;

//FfH: Added by Kael 10/13/2007
    m_bMoveDisabledAI = false;
    m_bMoveDisabledHuman = false;
    m_bBuildDisabled = false;
    m_bFoundDisabled = false;
    m_bPythonActive = true;
	m_eRealTerrainType = NO_TERRAIN;
    m_iMinLevel = 0;
    m_iPlotCounter = 0;
    m_iPortalExitX = 0;
    m_iPortalExitY = 0;
    m_iTempTerrainTimer = 0;
//FfH: End Add

	m_iDungeonType = NO_DUNGEON;
	m_iPortalExit = FFreeList::INVALID_INDEX;
	m_iWilderness = 0;
	//figure out witch Plane this Plot belongs to
	m_iPlane = -1;
	m_iPlundered = 0;
	m_bWildmanaImmune = false;
	m_bNonPlanar = false;
	m_iAnimalDensity = 0;
    m_iAnimalDensityPlusLairs = 0;
    m_iBarBarianDensity = 0;
    m_iBarBarianDensityPlusLairs = 0;

	m_iDungeon = FFreeList::INVALID_INDEX;

	m_bGraphicsPageIndex = true;

	//pathfinding
	//m_iRegion = -1;
	//m_iSector = -1;

	m_plotCity.reset();
	m_workingCity.reset();
	m_workingCityOverride.reset();

	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		m_aiYield[iI] = 0;
	}
}


//////////////////////////////////////
// graphical only setup
//////////////////////////////////////
void CvPlot::setupGraphical()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/				
	{
		return;
	}

	updateSymbols();
	updateFeatureSymbol();
	updateRiverSymbol();
	updateMinimapColor();

	updateVisibility();
}

void CvPlot::updateGraphicEra()
{
	if(m_pRouteSymbol != NULL)
		gDLL->getRouteIFace()->updateGraphicEra(m_pRouteSymbol);

	if(m_pFlagSymbol != NULL)
		gDLL->getFlagEntityIFace()->updateGraphicEra(m_pFlagSymbol);
}

void CvPlot::erase()
{
	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvUnit* pLoopUnit;
	CLinkList<IDInfo> oldUnits;

	// kill units
	oldUnits.clear();

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		oldUnits.insertAtEnd(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);
	}

	pUnitNode = oldUnits.head();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = oldUnits.next(pUnitNode);

		if (pLoopUnit != NULL)
		{
			pLoopUnit->kill(false);
		}
	}

	// kill cities
	pCity = getPlotCity();
	if (pCity != NULL)
	{
		pCity->kill(false);
	}

	setBonusType(NO_BONUS);
	setImprovementType(NO_IMPROVEMENT);
	setRouteType(NO_ROUTE, false);
	setFeatureType(NO_FEATURE);
	destroyDungeon();
	destroyPortal();

	// disable rivers
	setNOfRiver(false, NO_CARDINALDIRECTION);
	setWOfRiver(false, NO_CARDINALDIRECTION);
	setRiverID(-1);
}


float CvPlot::getPointX() const
{
	/* Fourth Yield (Asaf) - Start */
	float fRes = GC.getMapINLINE().plotXToPointX(getX_INLINE());
		
	static float s_fLeftConst = -22.0f;
	static float s_fRightConst = 0.0f;

	static float s_fLeftFactor = -10.0f;
	static float s_fRightFactor = 35.0f;

	float fConst = 0.0f;
	if (m_eCurSymbolRenderOffsetMode == en_RenderOffsetLeft)
		fConst = s_fLeftConst + s_fLeftFactor * getSymbolSize();
	else if (m_eCurSymbolRenderOffsetMode == en_RenderOffsetRight)
		fConst = s_fRightConst + s_fRightFactor * getSymbolSize();

	return fRes + fConst;
	//return GC.getMapINLINE().plotXToPointX(getX_INLINE());
	/* Fourth Yield (Asaf) - End */


/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	CvViewport& pCurrentViewport = GC.getCurrentViewport();

	return pCurrentViewport.plotXToPointX(pCurrentViewport.getViewportXFromMapX(getX_INLINE()));
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
}


float CvPlot::getPointY() const
{
	/* Fourth Yield (Asaf) - Start */
	float fRes = GC.getMapINLINE().plotYToPointY(getY_INLINE());
	
	static float s_fRightHillsConst = -8.0f;

	float fConst = 0.0f;
	if (m_eCurSymbolRenderOffsetMode == en_RenderOffsetRight)
	{
		if (isHills())
			fConst = s_fRightHillsConst;
	}

	return fRes + fConst;

	//return GC.getMapINLINE().plotYToPointY(getY_INLINE());
	/* Fourth Yield (Asaf) - End */
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	CvViewport& pCurrentViewport = GC.getCurrentViewport();

	return pCurrentViewport.plotYToPointY(pCurrentViewport.getViewportYFromMapY(getY_INLINE()));
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
}


NiPoint3 CvPlot::getPoint() const
{
	NiPoint3 pt3Point;

	pt3Point.x = getPointX();
	pt3Point.y = getPointY();
	pt3Point.z = 0.0f;

	pt3Point.z = gDLL->getEngineIFace()->GetHeightmapZ(pt3Point);

	return pt3Point;
}


float CvPlot::getSymbolSize() const
{
	/* Fourth Yield (Asaf) - Start */
	float fFactor = 1.0f;
	if (m_eCurSymbolRenderOffsetMode != en_RenderOffsetNormal)
		fFactor = 0.9f;

	if (isVisibleWorked())
	{
		if (isShowCitySymbols())
		{
			return 1.6f * fFactor;
		}
		else
		{
			return 1.2f * fFactor;
		}
	}
	else
	{
		if (isShowCitySymbols())
		{
			return 1.2f * fFactor;
		}
		else
		{
			return 0.8f;
		}
	}
/**
	if (isVisibleWorked())
	{
		if (isShowCitySymbols())
		{
			return 1.6f;
		}
		else
		{
			return 1.2f;
		}
	}
	else
	{
		if (isShowCitySymbols())
		{
			return 1.2f;
		}
		else
		{
			return 0.8f;
		}
	}
**/
/* Fourth Yield (Asaf) - End */
}


float CvPlot::getSymbolOffsetX(int iOffset) const
{
	return ((40.0f + (((float)iOffset) * 28.0f * getSymbolSize())) - (GC.getPLOT_SIZE() / 2.0f));
}


float CvPlot::getSymbolOffsetY(int iOffset) const
{
	return (-(GC.getPLOT_SIZE() / 2.0f) + 50.0f);
}


TeamTypes CvPlot::getTeam() const
{
	if (isOwned())
	{
		return GET_PLAYER(getOwnerINLINE()).getTeam();
	}
	else
	{
		return NO_TEAM;
	}
}


void CvPlot::doTurn()
{
	PROFILE_FUNC();

	if (getForceUnownedTimer() > 0)
	{
		changeForceUnownedTimer(-1);
	}

	if (isOwned())
	{
		changeOwnershipDuration(1);
	}

	setPlundered(std::max(0, getPlundered() - 1));
	//Lairs - added Sephi
	calculateAnimalDensity();	// necessary so that not too many animals spawn
	doLairTurn();

	if (getImprovementType() != NO_IMPROVEMENT)
	{
		changeImprovementDuration(1);

//FfH Improvements: Added by Kael 08/07/2007
		ImprovementTypes eImprovementUpdrade = (ImprovementTypes)GC.getImprovementInfo(getImprovementType()).getImprovementUpgrade();
		if (eImprovementUpdrade != NO_IMPROVEMENT)
		{
            if (!isBeingWorked())
            {
                if (GC.getImprovementInfo(eImprovementUpdrade).isOutsideBorders())
                {
                    doImprovementUpgrade();
                }
            }
		}

		//check if Terrain is still valid
		/**
		if(getImprovementType()!=NO_IMPROVEMENT && getBonusType(NO_TEAM)==NO_BONUS)
		{
			CvImprovementInfo &kImprovement=GC.getImprovementInfo(getImprovementType());
			if(!kImprovement.isPermanent() && !kImprovement.isUnique())
			{
				if(!isHills() || !kImprovement.isHillsMakesValid())
				{
					if(!isRiverSide() || !kImprovement.isRiverSideMakesValid())
					{
						if(!isFreshWater() || !kImprovement.isFreshWaterMakesValid())
						{
							if(getFeatureType()!=GC.getInfoTypeForString("FEATURE_FLOOD_PLAINS") || !kImprovement.isFloodPlainsMakesValid())
							{
								if(!kImprovement.getTerrainMakesValid(getTerrainType())
									&& !kImprovement.getFeatureMakesValid(getFeatureType()))
								{
									setImprovementType(NO_IMPROVEMENT);
								}
							}
						}
					}
				}
			}
		}
		**/
	}


	if (getImprovementType() != NO_IMPROVEMENT)
	{
        if (GC.getImprovementInfo(getImprovementType()).getFeatureUpgrade() != NO_FEATURE)
        {
            if (GC.getGameINLINE().getSorenRandNum(100, "Feature Upgrade") < GC.getDefineINT("FEATURE_UPGRADE_CHANCE"))
            {
                setFeatureType((FeatureTypes)GC.getImprovementInfo(getImprovementType()).getFeatureUpgrade());
                setImprovementType(NO_IMPROVEMENT);
            }
        }
//FfH: End Add
	}

	if (getImprovementType() != NO_IMPROVEMENT)
	{
		int iDestroyChance = GC.getImprovementInfo(getImprovementType()).getDestroyAdjacentImprovementChance();
		if(iDestroyChance > 0) {
			for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
			{
				CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
				if(pAdjacentPlot != NULL && pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					if(!GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).isPermanent()) {
						if(GC.getGame().getSorenRandNum(100, "Volcano effect") < iDestroyChance) {
							if(pAdjacentPlot->getOwnerINLINE() != NO_PLAYER) {
								CvWString szBuffer = gDLL->getText("TXT_KEY_IMPROVEMENT_DESTROYED_IMPROVEMENT",  GAMETEXT.getLinkedText((ImprovementTypes)getImprovementType()).c_str(), GAMETEXT.getLinkedText((ImprovementTypes)pAdjacentPlot->getImprovementType()).c_str());
								gDLL->getInterfaceIFace()->addMessage(pAdjacentPlot->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_PILLAGED", MESSAGE_TYPE_INFO);
							}
							pAdjacentPlot->setImprovementType(NO_IMPROVEMENT);
						}
					}
				}
			}
		}
	}

	// Super Forts begin *bombard*
	if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
	{
		if (!isBombarded() && getDefenseDamage() > 0)
		{
			changeDefenseDamage(-(GC.getDefineINT("CITY_DEFENSE_DAMAGE_HEAL_RATE")));
		}
		setBombarded(false);
	}
	// Super Forts end

	doFeature();

	doCulture();

	verifyUnitValidPlot();

//FfH: Added by Kael 11/02/2007
    if (getTempTerrainTimer() > 0)
    {
        changeTempTerrainTimer(-1);
        if (getTempTerrainTimer() == 0)
        {
			if(getRealTerrainType()!=NO_TERRAIN)	// added Sephi
			{
				setTerrainType(getRealTerrainType(),true,true);
				setRealTerrainType(NO_TERRAIN);
			}
        }
    }
//FfH: End Add

	/*
	if (!isOwned())
	{
		doImprovementUpgrade();
	}
	*/

	// XXX
#ifdef _DEBUG
	{
		CLLNode<IDInfo>* pUnitNode;
		CvUnit* pLoopUnit;

		pUnitNode = headUnitNode();

		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			FAssertMsg(pLoopUnit->atPlot(this), "pLoopUnit is expected to be at the current plot instance");
		}
	}
#endif
	// XXX
}


void CvPlot::doImprovement()
{
	PROFILE_FUNC();

	CvCity* pCity;
	CvWString szBuffer;
	int iI;
	FAssert(isOwned());
/*
	bool bIncreasedSpawning = GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN);
	if(!bIncreasedSpawning)
	{
		for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
		{
			int iChance = getBonusSpawnChance((BonusTypes)iI, getOwnerINLINE());
			if(iChance > 0) {

				if (GC.getGameINLINE().getSorenRandNum(10000, "Bonus Discovery") < iChance) {

					setBonusType((BonusTypes)iI);

					pCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE(), NO_TEAM, false);

					if(isOOSLogging())
					{
						int iTotalPlotRandNumPerTurn = 0;
						for(int i = 0; i < GC.getMapINLINE().numPlots(); +//0) {
							CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(i);
							if(pLoopPlot->isOwned() && !pLoopPlot->isWater() && pLoopPlot != this && pLoopPlot->getImprovementType() == NO_IMPROVEMENT && pLoopPlot->getBonusType() == NO_BONUS)
							{
								iTotalPlotRandNumPerTurn+//0;
							}
						}
						CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());

						CvImprovementInfo& kImprovement = GC.getImprovementInfo(getImprovementType());
						ProjectTypes eFertileLands = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_FERTILE_LANDS");
						ProjectTypes eArid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_ARID");
						ProjectTypes eHumid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_HUMID");
						ProjectTypes eGrow = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_EVERGROW");
						bool adjacentBonusFound = false;
						int iModifier = 0;
						if (isOwned())
						{
							iModifier +//0= kPlayer.getDiscoverRandModifier();
							for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK+//0)
							{
								CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
								if (pAdjacentPlot != NULL) {
									if (pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
										if (pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT) {
											iModifier +//0= (GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify());
										}
										if (pAdjacentPlot->getBonusType(kPlayer.getTeam()) == (BonusTypes)iI) {
											adjacentBonusFound = true;
										}
									}
								}
							}
						}

						oosLog("ResourceSpawn"
							,"Turn:%d,MapSize:%d,TotalPlot:%d,OwnedPlot:%d,LandPlot:%d,Chance:%d,X:%d,Y:%d,Owner:%d,Bonus:%S,TotalRand:%d,Base:%d,eFertileLands:%d,eArid:%d,eHumid:%d,eGrow:%d,MerchantDistrict:%d,Adjacent:%d,Modifier:%d+//0-n"
							,GC.getGameINLINE().getElapsedGameTurns()
							,GC.getMapINLINE().getWorldSize()
							,GC.getMapINLINE().getGridWidth() * GC.getMapINLINE().getGridHeight()
							,GC.getMapINLINE().getOwnedPlots()
							,GC.getMapINLINE().getLandPlots()
							,iChance
							,getX()
							,getY()
							,getOwner()
							,GC.getBonusInfo((BonusTypes)iI).getDescription()
							,iTotalPlotRandNumPerTurn

							,!isWater() ? kImprovement.getImprovementBonusDiscoverRand((BonusTypes)iI) : -1
							,eFertileLands != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eFertileLands) > 0 ? 1 : 0
							,eArid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eArid) > 0 ? 1 : 0
							,eHumid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eHumid) > 0 ? 1 : 0
							,eGrow != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eGrow) > 0 ? 1 : 0
							,kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT())
							,adjacentBonusFound
							,iModifier
						);
					}

					if (pCity != NULL) {
						szBuffer = gDLL->getText("TXT_KEY_MISC_DISCOVERED_NEW_RESOURCE", GC.getBonusInfo((BonusTypes) iI).getTextKeyWide(), pCity->getNameKey());
						gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MINOR_EVENT, GC.getBonusInfo((BonusTypes) iI).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
					}

					break;
				}
			}
		}
		return;
	}
*/
	std::vector<int> foundBonuses;
	bool resourceFound = false;

	//SpyFanatic: shift array each turn so that first element is always different, giving some different chances of bonus to spawn. Testing with all increase the rand number and increase the spawning rate
	std::vector<int> listBonus;
	/*
	int shifts = GC.getGameINLINE().getElapsedGameTurns() % GC.getNumBonusInfos();
	for (iI = shifts; iI < GC.getNumBonusInfos(); ++iI)
	{
		listBonus.push_back(iI);
	}
	for (iI = 0; iI < shifts; ++iI)
	{
		listBonus.push_back(iI);
	}
	*/
	for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
	{
		BonusTypes eBonus = listBonus.size() != 0 ? (BonusTypes)listBonus.at(iI) : (BonusTypes)iI;
		int iChance = 0;
		
		if (isWater())
			iChance = getPossibleWaterBonusDiscoverChance(eBonus, getOwnerINLINE());
		else
			iChance = getLandBonusDiscoverChance(eBonus, getOwnerINLINE());

		if(iChance > 0)
		{
			//if (GC.getGameINLINE().getSorenRandNum(100000, "Bonus Discovery") < iChance)
			if (GC.getGameINLINE().getSorenRandNum(10000, "Bonus Discovery") < iChance)
			{
				foundBonuses.push_back(eBonus);
				resourceFound = true;
				//DEBUG
				if(isOOSLogging())
				{
					int iTotalPlotRandNumPerTurn = 0;
					for(int i = 0; i < GC.getMapINLINE().numPlots(); i++) {
						CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(i);
						if(pLoopPlot->isOwned() && !pLoopPlot->isWater() && pLoopPlot != this && pLoopPlot->getImprovementType() == NO_IMPROVEMENT && pLoopPlot->getBonusType() == NO_BONUS)
						{
							iTotalPlotRandNumPerTurn++;
						}
					}
					CvPlayer& kPlayer = GET_PLAYER(getOwnerINLINE());
					if(!isWater())
					{
						CvImprovementInfo& kImprovement = GC.getImprovementInfo(getImprovementType());
						ProjectTypes eFertileLands = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_FERTILE_LANDS");
						ProjectTypes eArid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_ARID");
						ProjectTypes eHumid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_HUMID");
						ProjectTypes eGrow = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_EVERGROW");
						bool adjacentBonusFound = false;
						int iModifier = 0;
						if (isOwned())
						{
							iModifier += kPlayer.getDiscoverRandModifier();
							for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
							{
								CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
								if (pAdjacentPlot != NULL) {
									if (pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
										if (pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT) {
											iModifier += (GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify());
										}
										if (pAdjacentPlot->getBonusType(kPlayer.getTeam()) == eBonus) {
											adjacentBonusFound = true;
										}
									}
								}
							}
						}
						oosLog("ResourceSpawnLand"
							,"Turn:%d,MapSize:%d,TotalPlot:%d,OwnedPlot:%d,LandPlot:%d,Chance:%d,X:%d,Y:%d,Owner:%d,Bonus:%S,TotalRand:%d,Base:%d,eFertileLands:%d,eArid:%d,eHumid:%d,eGrow:%d,MerchantDistrict:%d,Adjacent:%d,Modifier:%d"
							,GC.getGameINLINE().getElapsedGameTurns()
							,GC.getMapINLINE().getWorldSize()
							,GC.getMapINLINE().getGridWidth() * GC.getMapINLINE().getGridHeight()
							,GC.getMapINLINE().getOwnedPlots()
							,GC.getMapINLINE().getLandPlots()
							,iChance
							,getX()
							,getY()
							,getOwner()
							,GC.getBonusInfo(eBonus).getDescription()
							,iTotalPlotRandNumPerTurn

							,kImprovement.getImprovementBonusDiscoverRand(eBonus)
							,eFertileLands != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eFertileLands) > 0 ? 1 : 0
							,eArid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eArid) > 0 ? 1 : 0
							,eHumid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eHumid) > 0 ? 1 : 0
							,eGrow != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eGrow) > 0 ? 1 : 0
							,kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT())
							,adjacentBonusFound
							,iModifier
						);
					}
					else
					{
						ProjectTypes eRichSeas = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_RICHES_OF_THE_SEA");
						bool riverFound = false;
						int iNearbyModifier = 0;
						for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
						{
							CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
							if (pAdjacentPlot != NULL && pAdjacentPlot->isWater()) {
								if (pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
									if (((pAdjacentPlot->getBonusType(kPlayer.getTeam()) == eBonus && eBonus != GC.getInfoTypeForString("BONUS_WHALE"))
										|| (pAdjacentPlot->getBonusType(kPlayer.getTeam()) == GC.getInfoTypeForString("BONUS_FISH") && eBonus == GC.getInfoTypeForString("BONUS_WHALE")))) {
										iNearbyModifier += GC.getBonusInfo(eBonus).getDiscoverRandModifier();
									}
								}
							}
							else if (pAdjacentPlot != NULL && pAdjacentPlot->isRiver()) {
								if (pAdjacentPlot->getRiverNSDirection() == CARDINALDIRECTION_NORTH) {
									if ((DirectionTypes)iK == DIRECTION_SOUTH || (DirectionTypes)iK == DIRECTION_SOUTHWEST) {
										riverFound = true;
									}
								}
								else if (pAdjacentPlot->getRiverNSDirection() == CARDINALDIRECTION_SOUTH) {
									if ((DirectionTypes)iK == DIRECTION_NORTH || (DirectionTypes)iK == DIRECTION_NORTHWEST) {
										riverFound = true;
									}
								}
								else if (pAdjacentPlot->getRiverWEDirection() == CARDINALDIRECTION_EAST) {
									if ((DirectionTypes)iK == DIRECTION_WEST || (DirectionTypes)iK == DIRECTION_NORTHWEST) {
										riverFound = true;
									}
								}
								else if (pAdjacentPlot->getRiverWEDirection() == CARDINALDIRECTION_WEST) {
									if ((DirectionTypes)iK == DIRECTION_EAST || (DirectionTypes)iK == DIRECTION_NORTHEAST) {
										riverFound = true;
									}
								}
							}
						}
						oosLog("ResourceSpawnWater"
							,"Turn:%d,MapSize:%d,TotalPlot:%d,OwnedPlot:%d,LandPlot:%d,Chance:%d,X:%d,Y:%d,Owner:%d,Bonus:%S,TotalRand:%d,Base:%d,eRichSeas:%d,Improvement:%d,Seafaring:%d,Kelp:%d,ReligionOO:%d,HeronThrone:%d,Harbour:%d,RiverFound:%d,NearbyModifier:%d"
							,GC.getGameINLINE().getElapsedGameTurns()
							,GC.getMapINLINE().getWorldSize()
							,GC.getMapINLINE().getGridWidth() * GC.getMapINLINE().getGridHeight()
							,GC.getMapINLINE().getOwnedPlots()
							,GC.getMapINLINE().getLandPlots()
							,iChance
							,getX()
							,getY()
							,getOwner()
							,GC.getBonusInfo((BonusTypes)iI).getDescription()
							,iTotalPlotRandNumPerTurn

							,GC.getBonusInfo(eBonus).getDiscoverRandModifier()
							,eRichSeas != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eRichSeas) > 0 ? 1 : 0
							,getImprovementType() != NO_IMPROVEMENT ? 1 : 0
							,kPlayer.isHasTech(GC.getInfoTypeForString("TECH_SEAFARING")) ? 1 : 0
							,getFeatureType() == GC.getInfoTypeForString("FEATURE_KELP") ? 1 : 0
							,kPlayer.getStateReligion() == RELIGION_OCTOPUS_OVERLORDS ? 1 : 0
							,kPlayer.getBuildingClassCount((BuildingClassTypes) GC.getInfoTypeForString("BUILDINGCLASS_HERON_THRONE", false)) > 0 ? 1 : 0
							,(getWorkingCity() != 0 && (getWorkingCity()->getNumActiveBuilding((BuildingTypes)GC.getInfoTypeForString("BUILDING_HARBOR", false)) > 0 || getWorkingCity()->getNumActiveBuilding((BuildingTypes) GC.getInfoTypeForString("BUILDING_HARBOR_LANUN", false)) > 0)) ? 1 : 0
							,riverFound
							,iNearbyModifier
						);
					}
				}
				//DEBUG
				//if( !GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN))
				if(listBonus.size() != 0)
				{
					break; //Stop at first resource found
				}
			}
		}
	}

	if (resourceFound)
	{
		int bonus = foundBonuses.at(foundBonuses.size() == 1 ? 0 : GC.getGameINLINE().getSorenRandNum(foundBonuses.size(), "Bonus Discovery Index"));
		setBonusType((BonusTypes)bonus);

		pCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE(), NO_TEAM, false);

		if (pCity != NULL) {
			szBuffer = gDLL->getText("TXT_KEY_MISC_DISCOVERED_NEW_RESOURCE", GC.getBonusInfo((BonusTypes)bonus).getTextKeyWide(), pCity->getNameKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_DISCOVERBONUS", MESSAGE_TYPE_MINOR_EVENT, GC.getBonusInfo((BonusTypes)bonus).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
		}
	}
}

void CvPlot::doImprovementUpgrade()
{
	if (getImprovementType() != NO_IMPROVEMENT)
	{
		ImprovementTypes eImprovementUpdrade = (ImprovementTypes)GC.getImprovementInfo(getImprovementType()).getImprovementUpgrade();
		if (eImprovementUpdrade != NO_IMPROVEMENT)
		{
			// Super Forts begin *upgrade* - added if-else statement
			if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(getImprovementType()).getCulture() > 0)
			{
					bool bDefenderFound = false;
					CLinkList<IDInfo> oldUnits;
					CLLNode<IDInfo>* pUnitNode = headUnitNode();
					CvUnit* pLoopUnit;

					while (pUnitNode != NULL)
					{
						oldUnits.insertAtEnd(pUnitNode->m_data);
						pUnitNode = nextUnitNode(pUnitNode);
					}

					pUnitNode = oldUnits.head();

					while (pUnitNode != NULL)
					{
						pLoopUnit = ::getUnit(pUnitNode->m_data);
						pUnitNode = nextUnitNode(pUnitNode);

						if(pLoopUnit->getFortifyTurns() > 0)
						{
							//SpyFanatic: must be also != NO_UNITCOMBAT
							if (pLoopUnit->getTeam() == getTeam() && pLoopUnit->canDefend() && pLoopUnit->getUnitCombatType() != NO_UNITCOMBAT)
							{
								bDefenderFound = true;
								break;
							}
						}
					}

					if(bDefenderFound)
					{
						//changeUpgradeProgress(calculateImprovementUpgradeRate(getImprovementType(), getOwnerINLINE()));

						changeUpgradeProgress(GET_PLAYER(getOwnerINLINE()).getImprovementUpgradeRate());

						if (getUpgradeProgress() >= GC.getGameINLINE().getImprovementUpgradeTime(getImprovementType()))
						{
							setImprovementType(eImprovementUpdrade);
						}
					}
			}
			else
			{
			if (isBeingWorked() || GC.getImprovementInfo(eImprovementUpdrade).isOutsideBorders())
			{

//FfH: Modified by Kael 05/12/2008
//				changeUpgradeProgress(GET_PLAYER(getOwnerINLINE()).getImprovementUpgradeRate());
                if (isOwned())
                {
                    if (GC.getImprovementInfo(eImprovementUpdrade).getPrereqCivilization() == NO_CIVILIZATION ||
                      GC.getImprovementInfo(eImprovementUpdrade).getPrereqCivilization() == GET_PLAYER(getOwnerINLINE()).getCivilizationType())
                    {
                        changeUpgradeProgress(calculateImprovementUpgradeRate(getImprovementType(), getOwnerINLINE()));
                    }
                    if (GC.getImprovementInfo(getImprovementType()).getPrereqCivilization() != NO_CIVILIZATION &&
                      GC.getImprovementInfo(getImprovementType()).getPrereqCivilization() != GET_PLAYER(getOwnerINLINE()).getCivilizationType())
                    {
                        setImprovementType((ImprovementTypes)GC.getImprovementInfo(getImprovementType()).getImprovementPillage());
                    }
                }
                else
                {
                    if (GC.getImprovementInfo(eImprovementUpdrade).getPrereqCivilization() == NO_CIVILIZATION)
                    {
                        changeUpgradeProgress(1);
                    }
                }
//FfH: End Modify

/*************************************************************************************************/
/**	BUGFIX (some FFH code above can change the Plot Improvement ot NO_IMPROVEMENT) Sephi		**/
/**																			                    **/
/**	                                                                 							**/
/*************************************************************************************************/
/**
				if (getUpgradeProgress() >= GC.getGameINLINE().getImprovementUpgradeTime(getImprovementType()))
**/
				if (getImprovementType()!=NO_IMPROVEMENT && getUpgradeProgress() >= GC.getGameINLINE().getImprovementUpgradeTime(getImprovementType()))
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
				{
					setImprovementType(eImprovementUpdrade);
				}
			}
			} // Super Forts end
		}
	}
}

void CvPlot::updateCulture(bool bBumpUnits, bool bUpdatePlotGroups)
{
	// Super Forts begin *culture*
	bool bNotCityPlot = !isCity();
	if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && bNotCityPlot) //If its a city no need to check
	{
//oosLog("CultureFort","Plot %d %d noplayer:%d noimprovement:%d improvementculture:%d bNotCityPlot:%d",getX_INLINE(),getY_INLINE(),getOwnerINLINE() == NO_PLAYER,NO_IMPROVEMENT == getImprovementType(),NO_IMPROVEMENT != getImprovementType() ?GC.getImprovementInfo(getImprovementType()).getCulture() : -1,bNotCityPlot);
//return (getPlotCity() != NULL);
		if(getOwnerINLINE() == NO_PLAYER || NO_IMPROVEMENT == getImprovementType() || GC.getImprovementInfo(getImprovementType()).getCulture() <= 0)
		{
			bNotCityPlot = true;
		}
		else
		{
			bNotCityPlot = false;
		}
	}
	if(bNotCityPlot)
//if (!isCity(true) || (getOwnerINLINE() == NO_PLAYER))
	//if (!isCity() || (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && (!isCity(true) || getOwnerINLINE() == NO_PLAYER)))
//	if (!isCity() || (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && (!isCity(true) || getOwnerINLINE() == NO_PLAYER)))
	// if (!isCity()) Original Code
	{
		setOwner(calculateCulturalOwner(), bBumpUnits, bUpdatePlotGroups);
	}
	// Super Forts end
}


void CvPlot::updateFog()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	FAssert(GC.getGameINLINE().getActiveTeam() != NO_TEAM);

	if (isRevealed(GC.getGameINLINE().getActiveTeam(), false))
	{
		if (gDLL->getInterfaceIFace()->isBareMapMode())
		{
			gDLL->getEngineIFace()->LightenVisibility(getFOWIndex());
		}
		else
		{
			int cityScreenFogEnabled = GC.getDefineINT("CITY_SCREEN_FOG_ENABLED");
			if (cityScreenFogEnabled && gDLL->getInterfaceIFace()->isCityScreenUp() && (gDLL->getInterfaceIFace()->getHeadSelectedCity() != getWorkingCity()))
			{
				gDLL->getEngineIFace()->DarkenVisibility(getFOWIndex());
			}
			else if (isActiveVisible(false))
			{
				gDLL->getEngineIFace()->LightenVisibility(getFOWIndex());
			}
			else
			{
				gDLL->getEngineIFace()->DarkenVisibility(getFOWIndex());
			}
		}
	}
	else
	{
		gDLL->getEngineIFace()->BlackenVisibility(getFOWIndex());
	}
}


void CvPlot::updateVisibility()
{
	PROFILE("CvPlot::updateVisibility");

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	setLayoutDirty(true);

	updateSymbolVisibility();
	updateFeatureSymbolVisibility();
	updateRouteSymbol();

	CvCity* pCity = getPlotCity();
	if (pCity != NULL)
	{
		pCity->updateVisibility();
	}
}


void CvPlot::updateSymbolDisplay()
{
	PROFILE_FUNC();

	CvSymbol* pLoopSymbol;
	int iLoop;

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	for (iLoop = 0; iLoop < getNumSymbols(); iLoop++)
	{
		pLoopSymbol = getSymbol(iLoop);

		/* Fourth Yield (Asaf) - Start */
		m_eCurSymbolRenderOffsetMode = m_symbolsOffsetModes[iLoop];
		/* Fourth Yield (Asaf) - End */

		if (pLoopSymbol != NULL)
		{
			if (isShowCitySymbols())
			{
				gDLL->getSymbolIFace()->setAlpha(pLoopSymbol, (isVisibleWorked()) ? 1.0f : 0.8f);
			}
			else
			{
				gDLL->getSymbolIFace()->setAlpha(pLoopSymbol, (isVisibleWorked()) ? 0.8f : 0.6f);
			}
			gDLL->getSymbolIFace()->setScale(pLoopSymbol, getSymbolSize());
			gDLL->getSymbolIFace()->updatePosition(pLoopSymbol);
		}
	}

	/* Fourth Yield (Asaf) - Start */
	m_eCurSymbolRenderOffsetMode = en_RenderOffsetNormal;
	/* Fourth Yield (Asaf) - End */
}


void CvPlot::updateSymbolVisibility()
{
	PROFILE_FUNC();

	CvSymbol* pLoopSymbol;
	int iLoop;

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	for (iLoop = 0; iLoop < getNumSymbols(); iLoop++)
	{
		pLoopSymbol = getSymbol(iLoop);

		if (pLoopSymbol != NULL)
		{
			if (isRevealed(GC.getGameINLINE().getActiveTeam(), true) &&
				  (isShowCitySymbols() ||
				   (gDLL->getInterfaceIFace()->isShowYields() && !(gDLL->getInterfaceIFace()->isCityScreenUp()))))
			{
				gDLL->getSymbolIFace()->Hide(pLoopSymbol, false);
			}
			else
			{
				gDLL->getSymbolIFace()->Hide(pLoopSymbol, true);
			}
		}
	}
}


void CvPlot::updateSymbols()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	deleteAllSymbols();

	/* Fourth Yield (Asaf) - Start */
	int nNumRenderedYields = 0;
	int yieldAmounts[NUM_YIELD_TYPES];
	for (int iYieldType = 0; iYieldType < NUM_YIELD_TYPES; iYieldType++)
	{
		int iYield = calculateYield(((YieldTypes)iYieldType), true);
		yieldAmounts[iYieldType] = iYield;

		if (iYield > 0)
			++ nNumRenderedYields;
	}

	if (nNumRenderedYields <= 3)
	{
		// Render as normal
		m_eCurSymbolRenderOffsetMode = en_RenderOffsetNormal;
		updateSymbolsForRange(0, NUM_YIELD_TYPES, yieldAmounts);
	}
	else
	{
		// Split!
		m_eCurSymbolRenderOffsetMode = en_RenderOffsetLeft;
		updateSymbolsForRange(0, 3, yieldAmounts);
		m_eCurSymbolRenderOffsetMode = en_RenderOffsetRight;
		updateSymbolsForRange(3, NUM_YIELD_TYPES, yieldAmounts);
		m_eCurSymbolRenderOffsetMode = en_RenderOffsetNormal;
	}
	/* Fourth Yield (Asaf) - End */


	/* Fourth Yield (Asaf) - Start: Commented out this section */
	/////////////////////////////////////////
	/**
	int yieldAmounts[NUM_YIELD_TYPES];
	int maxYield = 0;
	for (int iYieldType = 0; iYieldType < NUM_YIELD_TYPES; iYieldType++)
	{
		int iYield = calculateYield(((YieldTypes)iYieldType), true);
		yieldAmounts[iYieldType] = iYield;
		if(iYield>maxYield)
		{
			maxYield = iYield;
		}
	}

	if(maxYield>0)
	{
		int maxYieldStack = GC.getDefineINT("MAX_YIELD_STACK");
		int layers = maxYield /maxYieldStack + 1;

		CvSymbol *pSymbol= NULL;
		for(int i=0;i<layers;i++)
		{
			pSymbol = addSymbol();
			for (int iYieldType = 0; iYieldType < NUM_YIELD_TYPES; iYieldType++)
			{
				int iYield = yieldAmounts[iYieldType] - (maxYieldStack * i);
				LIMIT_RANGE(0,iYield, maxYieldStack);
				if(yieldAmounts[iYieldType])
				{
					gDLL->getSymbolIFace()->setTypeYield(pSymbol,iYieldType,iYield);
				}
			}
		}
		for(int i=0;i<getNumSymbols();i++)
		{
			SymbolTypes eSymbol  = (SymbolTypes)0;
			pSymbol = getSymbol(i);
			gDLL->getSymbolIFace()->init(pSymbol, gDLL->getSymbolIFace()->getID(pSymbol), i, eSymbol, this);
		}
	}
	**/
	/////////////////////////////////////////
	/* Fourth Yield (Asaf) - End */


	updateSymbolDisplay();
	updateSymbolVisibility();
}


void CvPlot::updateMinimapColor()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	if (!shouldHaveGraphics())
	{
		return;
	}

	gDLL->getInterfaceIFace()->setMinimapColor(MINIMAPMODE_TERRITORY, getViewportX(),getViewportY(), plotMinimapColor(), STANDARD_MINIMAP_ALPHA);
	return;
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	if (!GC.IsGraphicsInitialized())
	{
		return;
	}

	gDLL->getInterfaceIFace()->setMinimapColor(MINIMAPMODE_TERRITORY, getX_INLINE(), getY_INLINE(), plotMinimapColor(), STANDARD_MINIMAP_ALPHA);
}


void CvPlot::updateCenterUnit()
{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	if (!isActiveVisible(true))
	{
		setCenterUnit(NULL);
		return;
	}

	setCenterUnit(getSelectedUnit());

	// lfgr UI 11/2020: No cycling when a unit on this plot is selected (since the selected unit is alawys shown; can use plot list buttons instead)
	bool bUsedSelectedUnit = ( getCenterUnit() != NULL );

	if (getCenterUnit() == NULL)
	{
		setCenterUnit(getBestDefender(GC.getGameINLINE().getActivePlayer(), NO_PLAYER, NULL, false, false, true));
	}

	if (getCenterUnit() == NULL)
	{
		setCenterUnit(getBestDefender(GC.getGameINLINE().getActivePlayer()));
	}

	if (getCenterUnit() == NULL)
	{
		setCenterUnit(getBestDefender(NO_PLAYER, GC.getGameINLINE().getActivePlayer(), gDLL->getInterfaceIFace()->getHeadSelectedUnit(), true));
	}

	if (getCenterUnit() == NULL)
	{
		setCenterUnit(getBestDefender(NO_PLAYER, GC.getGameINLINE().getActivePlayer(), gDLL->getInterfaceIFace()->getHeadSelectedUnit()));
	}

	if (getCenterUnit() == NULL)
	{
		setCenterUnit(getBestDefender(NO_PLAYER, GC.getGameINLINE().getActivePlayer()));
	}
	// lfgr UI 11/2020: Consider cycling
	if( getCenterUnit() != NULL && !bUsedSelectedUnit && PlotHelpCyclingManager::getInstance().getCycleIdx() != 0 )
	{
		std::vector<CvUnit *> plotUnits;
		GC.getGameINLINE().getPlotUnits( this, plotUnits );
		int iCenterUnitIdx = -1;
		for( int i = 0; i < (int) plotUnits.size(); i++ )
		{
			if( plotUnits.at( i ) == getCenterUnit() )
			{
				iCenterUnitIdx = i;
				break;
			}
		}
		FAssertMsg( iCenterUnitIdx != -1, "Center unit not found in unit list." )

		int iNewIdx = iCenterUnitIdx + PlotHelpCyclingManager::getInstance().getCycleIdx();
		iNewIdx %= plotUnits.size();
		if( iNewIdx < 0 )
		{
			iNewIdx += plotUnits.size();
		}
		setCenterUnit( plotUnits.at( iNewIdx ) );
	}
}


void CvPlot::verifyUnitValidPlot()
{
	PROFILE_FUNC();

	std::vector<CvUnit*> aUnits;
	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);
		if (NULL != pLoopUnit)
		{
			aUnits.push_back(pLoopUnit);
		}
	}

	std::vector<CvUnit*>::iterator it = aUnits.begin();
	while (it != aUnits.end())
	{
		CvUnit* pLoopUnit = *it;
		bool bErased = false;

		if (pLoopUnit != NULL)
		{
			if (pLoopUnit->atPlot(this))
			{
				if (!(pLoopUnit->isCargo()))
				{
					if (!(pLoopUnit->isCombat()))
					{
						if (!isValidDomainForLocation(*pLoopUnit) || !(pLoopUnit->canEnterArea(getTeam(), area())))
						{
							if (!pLoopUnit->jumpToNearestValidPlot())
							{
								bErased = true;
							}
						}
					}
				}
			}
		}

		if (bErased)
		{
			it = aUnits.erase(it);
		}
		else
		{
			++it;
		}
	}

	if (isOwned())
	{
		it = aUnits.begin();
		while (it != aUnits.end())
		{
			CvUnit* pLoopUnit = *it;
			bool bErased = false;

			if (pLoopUnit != NULL)
			{
				if (pLoopUnit->atPlot(this))
				{
					if (!(pLoopUnit->isCombat()))
					{
						if (pLoopUnit->getTeam() != getTeam() && (getTeam() == NO_TEAM || !GET_TEAM(getTeam()).isVassal(pLoopUnit->getTeam())))
						{
							if (isVisibleEnemyUnit(pLoopUnit))
							{
								if (!(pLoopUnit->isInvisible(getTeam(), false)))
								{
									if (!pLoopUnit->jumpToNearestValidPlot())
									{
										bErased = true;
									}
								}
							}
						}
					}
				}
			}

			if (bErased)
			{
				it = aUnits.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}


void CvPlot::nukeExplosion(int iRange, CvUnit* pNukeUnit)
{
	CLLNode<IDInfo>* pUnitNode;
	CvCity* pLoopCity;
	CvUnit* pLoopUnit;
	CvPlot* pLoopPlot;
	CLinkList<IDInfo> oldUnits;
	CvWString szBuffer;
	int iNukeDamage;
	int iNukedPopulation;
	int iDX, iDY;
	int iI;

	GC.getGameINLINE().changeNukesExploded(1);

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				// if we remove roads, don't remove them on the city... XXX

				pLoopCity = pLoopPlot->getPlotCity();

				if (pLoopCity == NULL)
				{
					if (!(pLoopPlot->isWater()) && !(pLoopPlot->isImpassable()))
					{
						if (NO_FEATURE == pLoopPlot->getFeatureType() || !GC.getFeatureInfo(pLoopPlot->getFeatureType()).isNukeImmune())
						{
							if (GC.getGameINLINE().getSorenRandNum(100, "Nuke Fallout") < GC.getDefineINT("NUKE_FALLOUT_PROB"))
							{
								pLoopPlot->setImprovementType(NO_IMPROVEMENT);
								pLoopPlot->setFeatureType((FeatureTypes)(GC.getDefineINT("NUKE_FEATURE")));
							}
						}
					}
				}

				oldUnits.clear();

				pUnitNode = pLoopPlot->headUnitNode();

				while (pUnitNode != NULL)
				{
					oldUnits.insertAtEnd(pUnitNode->m_data);
					pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
				}

				pUnitNode = oldUnits.head();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = oldUnits.next(pUnitNode);

					if (pLoopUnit != NULL)
					{
						if (pLoopUnit != pNukeUnit)
						{
							if (!pLoopUnit->isNukeImmune() && !pLoopUnit->isDelayedDeath())
							{
								iNukeDamage = (GC.getDefineINT("NUKE_UNIT_DAMAGE_BASE") + GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("NUKE_UNIT_DAMAGE_RAND_1"), "Nuke Damage 1") + GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("NUKE_UNIT_DAMAGE_RAND_2"), "Nuke Damage 2"));

								if (pLoopCity != NULL)
								{
									iNukeDamage *= std::max(0, (pLoopCity->getNukeModifier() + 100));
									iNukeDamage /= 100;
								}

								if (pLoopUnit->canFight() || pLoopUnit->airBaseCombatStr() > 0)
								{
									pLoopUnit->changeDamage(iNukeDamage, ((pNukeUnit != NULL) ? pNukeUnit->getOwnerINLINE() : NO_PLAYER));
								}
								else if (iNukeDamage >= GC.getDefineINT("NUKE_NON_COMBAT_DEATH_THRESHOLD"))
								{
									pLoopUnit->kill(false, ((pNukeUnit != NULL) ? pNukeUnit->getOwnerINLINE() : NO_PLAYER));
								}
							}
						}
					}
				}

				if (pLoopCity != NULL)
				{
					for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
					{
						if (pLoopCity->getNumRealBuilding((BuildingTypes)iI) > 0)
						{
							if (!(GC.getBuildingInfo((BuildingTypes) iI).isNukeImmune()))
							{
								if (GC.getGameINLINE().getSorenRandNum(100, "Building Nuked") < GC.getDefineINT("NUKE_BUILDING_DESTRUCTION_PROB"))
								{
									pLoopCity->setNumRealBuilding(((BuildingTypes)iI), pLoopCity->getNumRealBuilding((BuildingTypes)iI) - 1);
								}
							}
						}
					}

					iNukedPopulation = ((pLoopCity->getPopulation() * (GC.getDefineINT("NUKE_POPULATION_DEATH_BASE") + GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("NUKE_POPULATION_DEATH_RAND_1"), "Population Nuked 1") + GC.getGameINLINE().getSorenRandNum(GC.getDefineINT("NUKE_POPULATION_DEATH_RAND_2"), "Population Nuked 2"))) / 100);

					iNukedPopulation *= std::max(0, (pLoopCity->getNukeModifier() + 100));
					iNukedPopulation /= 100;

					pLoopCity->changePopulation(-(std::min((pLoopCity->getPopulation() - 1), iNukedPopulation)));
				}
			}
		}
	}

	CvEventReporter::getInstance().nukeExplosion(this, pNukeUnit);
}


bool CvPlot::isConnectedTo(const CvCity* pCity) const
{
	if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
	{
	if(isOwned())
	{
		return ((getPlotGroup(getOwnerINLINE()) == pCity->plotGroup(getOwnerINLINE())) || (getPlotGroup(pCity->getOwnerINLINE()) == pCity->plotGroup(pCity->getOwnerINLINE())));
	}
	else
	{
		return false;
	}
	}
	else
	{
	FAssert(isOwned());
	return ((getPlotGroup(getOwnerINLINE()) == pCity->plotGroup(getOwnerINLINE())) || (getPlotGroup(pCity->getOwnerINLINE()) == pCity->plotGroup(pCity->getOwnerINLINE())));
	}
	// Super Forts end
}


bool CvPlot::isConnectedToCapital(PlayerTypes ePlayer) const
{
	CvCity* pCapitalCity;

	if (ePlayer == NO_PLAYER)
	{
		ePlayer = getOwnerINLINE();
	}

	if (ePlayer != NO_PLAYER)
	{
		pCapitalCity = GET_PLAYER(ePlayer).getCapitalCity();

		if (pCapitalCity != NULL)
		{
			return isConnectedTo(pCapitalCity);
		}
	}

	return false;
}


int CvPlot::getPlotGroupConnectedBonus(PlayerTypes ePlayer, BonusTypes eBonus) const
{
	CvPlotGroup* pPlotGroup;

	FAssertMsg(ePlayer != NO_PLAYER, "Player is not assigned a valid value");
	FAssertMsg(eBonus != NO_BONUS, "Bonus is not assigned a valid value");

	pPlotGroup = getPlotGroup(ePlayer);

	if (pPlotGroup != NULL)
	{
		return pPlotGroup->getNumBonuses(eBonus);
	}
	else
	{
		return 0;
	}
}


bool CvPlot::isPlotGroupConnectedBonus(PlayerTypes ePlayer, BonusTypes eBonus) const
{
	return (getPlotGroupConnectedBonus(ePlayer, eBonus) > 0);
}


bool CvPlot::isAdjacentPlotGroupConnectedBonus(PlayerTypes ePlayer, BonusTypes eBonus) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isPlotGroupConnectedBonus(ePlayer, eBonus))
			{
				return true;
			}
		}
	}

	return false;
}


void CvPlot::updatePlotGroupBonus(bool bAdd)
{
	PROFILE_FUNC();

	CvCity* pPlotCity;
	CvPlotGroup* pPlotGroup;
	BonusTypes eNonObsoleteBonus;
	int iI;

	if (!isOwned())
	{
		return;
	}

	pPlotGroup = getPlotGroup(getOwnerINLINE());

	if (pPlotGroup != NULL)
	{
		pPlotCity = getPlotCity();

		if (pPlotCity != NULL)
		{
			for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
			{
				if (!GET_TEAM(getTeam()).isBonusObsolete((BonusTypes)iI))
				{
					pPlotGroup->changeNumBonuses(((BonusTypes)iI), (pPlotCity->getFreeBonus((BonusTypes)iI) * ((bAdd) ? 1 : -1)));
				}
			}

			if (pPlotCity->isCapital())
			{
				for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
				{
					pPlotGroup->changeNumBonuses(((BonusTypes)iI), (GET_PLAYER(getOwnerINLINE()).getBonusExport((BonusTypes)iI) * ((bAdd) ? -1 : 1)));
					pPlotGroup->changeNumBonuses(((BonusTypes)iI), (GET_PLAYER(getOwnerINLINE()).getBonusImport((BonusTypes)iI) * ((bAdd) ? 1 : -1)));
				}
			}
		}

		eNonObsoleteBonus = getNonObsoleteBonusType(getTeam());

		if (eNonObsoleteBonus != NO_BONUS)
		{
			if (GET_TEAM(getTeam()).isHasTech((TechTypes)(GC.getBonusInfo(eNonObsoleteBonus).getTechCityTrade())))
			{
				if (isCity(true, getTeam()) ||
					//SpyFanatic: add also forts to connect to bonus
					(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && getImprovementType() != NO_IMPROVEMENT && GC.getImprovementInfo(getImprovementType()).getCulture() > 0) ||
					((getImprovementType() != NO_IMPROVEMENT) && GC.getImprovementInfo(getImprovementType()).isImprovementBonusTrade(eNonObsoleteBonus)))
				{
					if ((pPlotGroup != NULL) && isBonusNetwork(getTeam()))
					{
						pPlotGroup->changeNumBonuses(eNonObsoleteBonus, ((bAdd) ? 1 : -1));
					}
				}
			}
		}
	}
}


bool CvPlot::isAdjacentToArea(int iAreaID) const
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->getArea() == iAreaID)
			{
				return true;
			}
		}
	}

	return false;
}

bool CvPlot::isAdjacentToArea(const CvArea* pArea) const
{
	return isAdjacentToArea(pArea->getID());
}


bool CvPlot::shareAdjacentArea(const CvPlot* pPlot) const
{
	PROFILE_FUNC();

	int iCurrArea;
	int iLastArea;
	CvPlot* pAdjacentPlot;
	int iI;

	iLastArea = FFreeList::INVALID_INDEX;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			iCurrArea = pAdjacentPlot->getArea();

			if (iCurrArea != iLastArea)
			{
				if (pPlot->isAdjacentToArea(iCurrArea))
				{
					return true;
				}

				iLastArea = iCurrArea;
			}
		}
	}

	return false;
}


bool CvPlot::isAdjacentToLand() const
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (!(pAdjacentPlot->isWater()))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isCoastalLand(int iMinWaterSize) const
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;
	int iI;

	if (isWater())
	{
		return false;
	}

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isWater())
			{
				if (pAdjacentPlot->area()->getNumTiles() >= iMinWaterSize)
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvPlot::isVisibleWorked() const
{
	if (isBeingWorked())
	{
		if ((getTeam() == GC.getGameINLINE().getActiveTeam()) || GC.getGameINLINE().isDebugMode())
		{
			return true;
		}
	}

	return false;
}


bool CvPlot::isWithinTeamCityRadius(TeamTypes eTeam, PlayerTypes eIgnorePlayer) const
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
			{
				if ((eIgnorePlayer == NO_PLAYER) || (((PlayerTypes)iI) != eIgnorePlayer))
				{
					if (isPlayerCityRadius((PlayerTypes)iI))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


bool CvPlot::isLake() const
{
	CvArea* pArea = area();
	if (pArea != NULL)
	{
		return pArea->isLake();
	}

	return false;
}


// XXX if this changes need to call updateIrrigated() and pCity->updateFreshWaterHealth()
// XXX precalculate this???
bool CvPlot::isFreshWater() const
{
	CvPlot* pLoopPlot;
	int iDX, iDY;

	if (isWater())
	{
		return false;
	}

	if (isImpassable())
	{
		return false;
	}

	if (isRiver())
	{
		return true;
	}

	for (iDX = -1; iDX <= 1; iDX++)
	{
		for (iDY = -1; iDY <= 1; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->isLake())
				{
					return true;
				}

				if (pLoopPlot->getFeatureType() != NO_FEATURE)
				{
					if (GC.getFeatureInfo(pLoopPlot->getFeatureType()).isAddsFreshWater())
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


bool CvPlot::isPotentialIrrigation() const
{
	if ((isCity() && !isHills()) || ((getImprovementType() != NO_IMPROVEMENT) && (GC.getImprovementInfo(getImprovementType()).isCarriesIrrigation())))
	{
		if ((getTeam() != NO_TEAM) && GET_TEAM(getTeam()).isIrrigation())
		{
			return true;
		}
	}

	return false;
}


bool CvPlot::canHavePotentialIrrigation() const
{
	int iI;

	if (isCity() && !isHills())
	{
		return true;
	}

	for (iI = 0; iI < GC.getNumImprovementInfos(); ++iI)
	{
		if (GC.getImprovementInfo((ImprovementTypes)iI).isCarriesIrrigation())
		{
			if (canHaveImprovement(((ImprovementTypes)iI), NO_TEAM, true))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isIrrigationAvailable(bool bIgnoreSelf) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	if (!bIgnoreSelf && isIrrigated())
	{
		return true;
	}

	if (isFreshWater())
	{
		return true;
	}

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isIrrigated())
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isRiverMask() const
{
	CvPlot* pPlot;

	if (isNOfRiver())
	{
		return true;
	}

	if (isWOfRiver())
	{
		return true;
	}

	pPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_EAST);
	if ((pPlot != NULL) && pPlot->isNOfRiver())
	{
		return true;
	}

	pPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_SOUTH);
	if ((pPlot != NULL) && pPlot->isWOfRiver())
	{
		return true;
	}

	return false;
}


bool CvPlot::isRiverCrossingFlowClockwise(DirectionTypes eDirection) const
{
	CvPlot *pPlot;
	switch(eDirection)
	{
	case DIRECTION_NORTH:
		pPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_NORTH);
		if (pPlot != NULL)
		{
			return (pPlot->getRiverWEDirection() == CARDINALDIRECTION_EAST);
		}
		break;
	case DIRECTION_EAST:
		return (getRiverNSDirection() == CARDINALDIRECTION_SOUTH);
		break;
	case DIRECTION_SOUTH:
		return (getRiverWEDirection() == CARDINALDIRECTION_WEST);
		break;
	case DIRECTION_WEST:
		pPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_WEST);
		if(pPlot != NULL)
		{
			return (pPlot->getRiverNSDirection() == CARDINALDIRECTION_NORTH);
		}
		break;
	default:
		FAssert(false);
		break;
	}

	return false;
}


bool CvPlot::isRiverSide() const
{
	CvPlot* pLoopPlot;
	int iI;

	for (iI = 0; iI < NUM_CARDINALDIRECTION_TYPES; ++iI)
	{
		pLoopPlot = plotCardinalDirection(getX_INLINE(), getY_INLINE(), ((CardinalDirectionTypes)iI));

		if (pLoopPlot != NULL)
		{
			if (isRiverCrossing(directionXY(this, pLoopPlot)))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isRiver() const
{
	return (getRiverCrossingCount() > 0);
}


bool CvPlot::isRiverConnection(DirectionTypes eDirection) const
{
	if (eDirection == NO_DIRECTION)
	{
		return false;
	}

	switch (eDirection)
	{
	case DIRECTION_NORTH:
		return (isRiverCrossing(DIRECTION_EAST) || isRiverCrossing(DIRECTION_WEST));
		break;

	case DIRECTION_NORTHEAST:
		return (isRiverCrossing(DIRECTION_NORTH) || isRiverCrossing(DIRECTION_EAST));
		break;

	case DIRECTION_EAST:
		return (isRiverCrossing(DIRECTION_NORTH) || isRiverCrossing(DIRECTION_SOUTH));
		break;

	case DIRECTION_SOUTHEAST:
		return (isRiverCrossing(DIRECTION_SOUTH) || isRiverCrossing(DIRECTION_EAST));
		break;

	case DIRECTION_SOUTH:
		return (isRiverCrossing(DIRECTION_EAST) || isRiverCrossing(DIRECTION_WEST));
		break;

	case DIRECTION_SOUTHWEST:
		return (isRiverCrossing(DIRECTION_SOUTH) || isRiverCrossing(DIRECTION_WEST));
		break;

	case DIRECTION_WEST:
		return (isRiverCrossing(DIRECTION_NORTH) || isRiverCrossing(DIRECTION_SOUTH));
		break;

	case DIRECTION_NORTHWEST:
		return (isRiverCrossing(DIRECTION_NORTH) || isRiverCrossing(DIRECTION_WEST));
		break;

	default:
		FAssert(false);
		break;
	}

	return false;
}


CvPlot* CvPlot::getNearestLandPlotInternal(int iDistance) const
{
	if (iDistance > GC.getMapINLINE().getGridHeightINLINE() && iDistance > GC.getMapINLINE().getGridWidthINLINE())
	{
		return NULL;
	}

	for (int iDX = -iDistance; iDX <= iDistance; iDX++)
	{
		for (int iDY = -iDistance; iDY <= iDistance; iDY++)
		{
			if (abs(iDX) + abs(iDY) == iDistance)
			{
				CvPlot* pPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
				if (pPlot != NULL)
				{
					if (!pPlot->isWater())
					{
						return pPlot;
					}
				}
			}
		}
	}
	return getNearestLandPlotInternal(iDistance + 1);
}


int CvPlot::getNearestLandArea() const
{
	CvPlot* pPlot = getNearestLandPlot();
	return pPlot ? pPlot->getArea() : -1;
}


CvPlot* CvPlot::getNearestLandPlot() const
{
	return getNearestLandPlotInternal(0);
}


int CvPlot::seeFromLevel(TeamTypes eTeam) const
{
	int iLevel;

	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	iLevel = GC.getTerrainInfo(getTerrainType()).getSeeFromLevel();

	// Super Forts begin *vision*
	if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && getImprovementType() != NO_IMPROVEMENT)
	{
		iLevel += GC.getImprovementInfo(getImprovementType()).getSeeFrom();
	}
	// Super Forts end

	if (isPeak())
	{
		iLevel += GC.getPEAK_SEE_FROM_CHANGE();
	}

	if (isHills())
	{
		iLevel += GC.getHILLS_SEE_FROM_CHANGE();
	}

	if (isWater())
	{
		iLevel += GC.getSEAWATER_SEE_FROM_CHANGE();

		if (GET_TEAM(eTeam).isExtraWaterSeeFrom())
		{
			iLevel++;
		}
	}

	return iLevel;
}


int CvPlot::seeThroughLevel() const
{
	int iLevel;

	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	iLevel = GC.getTerrainInfo(getTerrainType()).getSeeThroughLevel();

	if (getFeatureType() != NO_FEATURE)
	{
		iLevel += GC.getFeatureInfo(getFeatureType()).getSeeThroughChange();
	}

	if (isPeak())
	{
		iLevel += GC.getPEAK_SEE_THROUGH_CHANGE();
	}

	if (isHills())
	{
		iLevel += GC.getHILLS_SEE_THROUGH_CHANGE();
	}

	if (isWater())
	{
		iLevel += GC.getSEAWATER_SEE_FROM_CHANGE();
	}

	return iLevel;
}



void CvPlot::changeAdjacentSight(TeamTypes eTeam, int iRange, bool bIncrement, CvUnit* pUnit, bool bUpdatePlotGroups)
{
	bool bAerial = (pUnit != NULL && pUnit->getDomainType() == DOMAIN_AIR);

	DirectionTypes eFacingDirection = NO_DIRECTION;
	if (!bAerial && NULL != pUnit)
	{
		eFacingDirection = pUnit->getFacingDirection(true);
	}

	//fill invisible types
	std::vector<InvisibleTypes> aSeeInvisibleTypes;
	if (NULL != pUnit)
	{
		for(int i=0;i<pUnit->getNumSeeInvisibleTypes();i++)
		{
			aSeeInvisibleTypes.push_back(pUnit->getSeeInvisibleType(i));
		}
	}

	if(aSeeInvisibleTypes.size() == 0)
	{
		aSeeInvisibleTypes.push_back(NO_INVISIBLE);
	}

	//check one extra outer ring
	if (!bAerial)
	{
		iRange++;
	}

	for(int i=0;i<(int)aSeeInvisibleTypes.size();i++)
	{
		for (int dx = -iRange; dx <= iRange; dx++)
		{
			for (int dy = -iRange; dy <= iRange; dy++)
			{
				//check if in facing direction
				if (bAerial || shouldProcessDisplacementPlot(dx, dy, iRange - 1, eFacingDirection))
				{
					bool outerRing = false;
					if ((abs(dx) == iRange) || (abs(dy) == iRange))
					{
						outerRing = true;
					}

					//check if anything blocking the plot
					if (bAerial || canSeeDisplacementPlot(eTeam, dx, dy, dx, dy, true, outerRing))
					{
						CvPlot* pPlot = plotXY(getX_INLINE(), getY_INLINE(), dx, dy);
						if (NULL != pPlot)
						{
							pPlot->changeVisibilityCount(eTeam, ((bIncrement) ? 1 : -1), aSeeInvisibleTypes[i], bUpdatePlotGroups);
						}
					}
				}

				if (eFacingDirection != NO_DIRECTION)
				{
					if((abs(dx) <= 1) && (abs(dy) <= 1)) //always reveal adjacent plots when using line of sight
					{
						CvPlot* pPlot = plotXY(getX_INLINE(), getY_INLINE(), dx, dy);
						if (NULL != pPlot)
						{
							pPlot->changeVisibilityCount(eTeam, 1, aSeeInvisibleTypes[i], bUpdatePlotGroups);
							pPlot->changeVisibilityCount(eTeam, -1, aSeeInvisibleTypes[i], bUpdatePlotGroups);
						}
					}
				}
			}
		}
	}
}

bool CvPlot::canSeePlot(CvPlot *pPlot, TeamTypes eTeam, int iRange, DirectionTypes eFacingDirection) const
{
	iRange++;

	if (pPlot == NULL)
	{
		return false;
	}

	//find displacement
	int dx = pPlot->getX() - getX();
	int dy = pPlot->getY() - getY();
	dx = dxWrap(dx); //world wrap
	dy = dyWrap(dy);

	//check if in facing direction
	if (shouldProcessDisplacementPlot(dx, dy, iRange - 1, eFacingDirection))
	{
		bool outerRing = false;
		if ((abs(dx) == iRange) || (abs(dy) == iRange))
		{
			outerRing = true;
		}

		//check if anything blocking the plot
		if (canSeeDisplacementPlot(eTeam, dx, dy, dx, dy, true, outerRing))
		{
			return true;
		}
	}

	return false;
}

bool CvPlot::canSeeDisplacementPlot(TeamTypes eTeam, int dx, int dy, int originalDX, int originalDY, bool firstPlot, bool outerRing) const
{
	CvPlot *pPlot = plotXY(getX_INLINE(), getY_INLINE(), dx, dy);
	if (pPlot != NULL)
	{
		//base case is current plot
		if((dx == 0) && (dy == 0))
		{
			return true;
		}

		//find closest of three points (1, 2, 3) to original line from Start (S) to End (E)
		//The diagonal is computed first as that guarantees a change in position
		// -------------
		// |   | 2 | S |
		// -------------
		// | E | 1 | 3 |
		// -------------

		int displacements[3][2] = {{dx - getSign(dx), dy - getSign(dy)}, {dx - getSign(dx), dy}, {dx, dy - getSign(dy)}};
		int allClosest[3];
		int closest = -1;
		for (int i=0;i<3;i++)
		{
			//int tempClosest = abs(displacements[i][0] * originalDX - displacements[i][1] * originalDY); //more accurate, but less structured on a grid
			allClosest[i] = abs(displacements[i][0] * dy - displacements[i][1] * dx); //cross product
			if((closest == -1) || (allClosest[i] < closest))
			{
				closest = allClosest[i];
			}
		}

		//iterate through all minimum plots to see if any of them are passable
		for(int i=0;i<3;i++)
		{
			int nextDX = displacements[i][0];
			int nextDY = displacements[i][1];
			if((nextDX != dx) || (nextDY != dy)) //make sure we change plots
			{
				if(allClosest[i] == closest)
				{
					if(canSeeDisplacementPlot(eTeam, nextDX, nextDY, originalDX, originalDY, false, false))
					{
						int fromLevel = seeFromLevel(eTeam);
						int throughLevel = pPlot->seeThroughLevel();
						if(outerRing) //check strictly higher level
						{
							CvPlot *passThroughPlot = plotXY(getX_INLINE(), getY_INLINE(), nextDX, nextDY);
							int passThroughLevel = passThroughPlot->seeThroughLevel();
							if (fromLevel >= passThroughLevel)
							{
								if((fromLevel > passThroughLevel) || (pPlot->seeFromLevel(eTeam) > fromLevel)) //either we can see through to it or it is high enough to see from far
								{
									return true;
								}
							}
						}
						else
						{
							if(fromLevel >= throughLevel) //we can clearly see this level
							{
								return true;
							}
							else if(firstPlot) //we can also see it if it is the first plot that is too tall
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

bool CvPlot::shouldProcessDisplacementPlot(int dx, int dy, int range, DirectionTypes eFacingDirection) const
{
	if(eFacingDirection == NO_DIRECTION)
	{
		return true;
	}
	else if((dx == 0) && (dy == 0)) //always process this plot
	{
		return true;
	}
	else
	{
		//							N		NE		E		SE			S		SW		W			NW
		int displacements[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};

		int directionX = displacements[eFacingDirection][0];
		int directionY = displacements[eFacingDirection][1];

		//compute angle off of direction
		int crossProduct = directionX * dy - directionY * dx; //cross product
		int dotProduct = directionX * dx + directionY * dy; //dot product

		float theta = atan2((float) crossProduct, (float) dotProduct);
		float spread = 60 * (float) M_PI / 180;
		if((abs(dx) <= 1) && (abs(dy) <= 1)) //close plots use wider spread
		{
			spread = 90 * (float) M_PI / 180;
		}

		if((theta >= -spread / 2) && (theta <= spread / 2))
		{
			return true;
		}
		else
		{
			return false;
		}

		/*
		DirectionTypes leftDirection = GC.getTurnLeftDirection(eFacingDirection);
		DirectionTypes rightDirection = GC.getTurnRightDirection(eFacingDirection);

		//test which sides of the line equation (cross product)
		int leftSide = displacements[leftDirection][0] * dy - displacements[leftDirection][1] * dx;
		int rightSide = displacements[rightDirection][0] * dy - displacements[rightDirection][1] * dx;
		if((leftSide <= 0) && (rightSide >= 0))
			return true;
		else
			return false;
		*/
	}
}

void CvPlot::updateSight(bool bIncrement, bool bUpdatePlotGroups)
{
	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
//	CvCity* pHolyCity;
	CvUnit* pLoopUnit;
	int iLoop;
	int iI;

	pCity = getPlotCity();

	if (pCity != NULL)
	{
		// Religion - Disabled with new Espionage System
/*		for (iI = 0; iI < GC.getNumReligionInfos(); ++iI)
		{
			if (pCity->isHasReligion((ReligionTypes)iI))
			{
				pHolyCity = GC.getGameINLINE().getHolyCity((ReligionTypes)iI);

				if (pHolyCity != NULL)
				{
					if (GET_PLAYER(pHolyCity->getOwnerINLINE()).getStateReligion() == iI)
					{
						changeAdjacentSight(pHolyCity->getTeam(), GC.getDefineINT("PLOT_VISIBILITY_RANGE"), bIncrement, NO_INVISIBLE);
					}
				}
			}
		}*/

//FfH: Added by Kael 11/03/2007
		for (iI = 0; iI < GC.getNumReligionInfos(); ++iI)
		{
			if (pCity->isHasReligion((ReligionTypes)iI))
			{
			    if (GC.getReligionInfo((ReligionTypes)iI).isUpdateSight())
			    {
                    CvCity* pHolyCity = GC.getGameINLINE().getHolyCity((ReligionTypes)iI);
                    if (pHolyCity != NULL)
                    {
	                    if (GET_PLAYER(pHolyCity->getOwnerINLINE()).getStateReligion() == iI)
                        {
                            changeAdjacentSight(pHolyCity->getTeam(), GC.getDefineINT("PLOT_VISIBILITY_RANGE"), bIncrement, NULL, bUpdatePlotGroups);
                        }
                    }
				}
			}
		}
//FfH: End Add

		// Vassal
		for (iI = 0; iI < MAX_TEAMS; ++iI)
		{
			if (GET_TEAM(getTeam()).isVassal((TeamTypes)iI))
			{
				changeAdjacentSight((TeamTypes)iI, GC.getDefineINT("PLOT_VISIBILITY_RANGE"), bIncrement, NULL, bUpdatePlotGroups);
			}
		}

		// EspionageEffect
		for (iI = 0; iI < MAX_CIV_TEAMS; ++iI)
		{
			if (pCity->getEspionageVisibility((TeamTypes)iI))
			{
				// Passive Effect: enough EPs gives you visibility into someone's cities
				changeAdjacentSight((TeamTypes)iI, GC.getDefineINT("PLOT_VISIBILITY_RANGE"), bIncrement, NULL, bUpdatePlotGroups);
			}
		}
	}

	// Owned
	if (isOwned())
	{
		changeAdjacentSight(getTeam(), GC.getDefineINT("PLOT_VISIBILITY_RANGE"), bIncrement, NULL, bUpdatePlotGroups);
	}

	pUnitNode = headUnitNode();

	// Unit
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);


		changeAdjacentSight(pLoopUnit->getTeam(), pLoopUnit->visibilityRange(), bIncrement, pLoopUnit, bUpdatePlotGroups);
	}

	if (getReconCount() > 0)
	{
		int iRange = GC.getDefineINT("RECON_VISIBILITY_RANGE");
		for (iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			for(pLoopUnit = GET_PLAYER((PlayerTypes)iI).firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iI).nextUnit(&iLoop))
			{
				if (pLoopUnit->getReconPlot() == this)
				{
					changeAdjacentSight(pLoopUnit->getTeam(), iRange, bIncrement, pLoopUnit, bUpdatePlotGroups);
				}
			}
		}
	}
}


void CvPlot::updateSeeFromSight(bool bIncrement, bool bUpdatePlotGroups)
{
	CvPlot* pLoopPlot;
	int iDX, iDY;

	int iRange = GC.getDefineINT("UNIT_VISIBILITY_RANGE") + 1;
	for (int iPromotion = 0; iPromotion < GC.getNumPromotionInfos(); ++iPromotion)
	{
		iRange += GC.getPromotionInfo((PromotionTypes)iPromotion).getVisibilityChange();
	}

	iRange = std::max(GC.getDefineINT("RECON_VISIBILITY_RANGE") + 1, iRange);

	for (iDX = -iRange; iDX <= iRange; iDX++)
	{
		for (iDY = -iRange; iDY <= iRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				pLoopPlot->updateSight(bIncrement, bUpdatePlotGroups);
			}
		}
	}
}


bool CvPlot::canHaveBonus(BonusTypes eBonus, bool bIgnoreLatitude) const
{
	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	if (eBonus == NO_BONUS)
	{
		return true;
	}

	if (getBonusType() != NO_BONUS)
	{
		return false;
	}

	if (isPeak())
	{
		return false;
	}

	if (getFeatureType() != NO_FEATURE)
	{
		if (!(GC.getBonusInfo(eBonus).isFeature(getFeatureType())))
		{
			return false;
		}

		if (!(GC.getBonusInfo(eBonus).isFeatureTerrain(getTerrainType())))
		{
			return false;
		}
	}
	else
	{
		if (!(GC.getBonusInfo(eBonus).isTerrain(getTerrainType())))
		{
			return false;
		}
	}

	if (isHills())
	{
		if (!(GC.getBonusInfo(eBonus).isHills()))
		{
			return false;
		}
	}
	else if (isFlatlands())
	{
		if (!(GC.getBonusInfo(eBonus).isFlatlands()))
		{
			return false;
		}
	}

	if (GC.getBonusInfo(eBonus).isNoRiverSide())
	{
		if (isRiverSide())
		{
			return false;
		}
	}

	if (GC.getBonusInfo(eBonus).getMinAreaSize() != -1)
	{
		if (area()->getNumTiles() < GC.getBonusInfo(eBonus).getMinAreaSize())
		{
			return false;
		}
	}

	if (!bIgnoreLatitude)
	{
		if (getLatitude() > GC.getBonusInfo(eBonus).getMaxLatitude())
		{
			return false;
		}

		if (getLatitude() < GC.getBonusInfo(eBonus).getMinLatitude())
		{
			return false;
		}
	}

	if (!isPotentialCityWork())
	{
		return false;
	}

//FfH: Added by Kael 12/18/2008
    if (isCity())
    {
        if (GC.getDefineINT("BONUS_MANA") != -1)
        {
            if (eBonus == GC.getDefineINT("BONUS_MANA"))
            {
                return false;
            }
        }
    }
//FfH: End Add

	if(GC.getBonusInfo(eBonus).getMinWilderness()>getWilderness())
	{
		return false;
	}

	if(GC.getBonusInfo(eBonus).getMaxWilderness()<getWilderness())
	{
		return false;
	}

	if (!GC.getMapINLINE().useClimateBasedStartPos() && GC.getMap().isPlanes()) {
		if(getPlane() != NO_PLANE)
		{
			if(!GC.getPlaneInfo(getPlane()).isBonusSpawn(eBonus))
			{
				return false;
			}
		}
	}

	if(getImprovementType() != NO_IMPROVEMENT && 
		GC.getImprovementInfo(getImprovementType()).isPermanent()) {
		if(GC.getImprovementInfo(getImprovementType()).getBonusConvert() == NO_BONUS ||
			eBonus != GC.getImprovementInfo(getImprovementType()).getBonusConvert()) {
			return false;
		}
	}

	if(isStartingPlot()) {
	    if (GC.getBonusInfo(eBonus).getBonusClassType()==GC.getDefineINT("BONUSCLASS_MANA"))
	    {
	        return false;
	    }

        if (GC.getBonusInfo(eBonus).getBonusClassType()==GC.getDefineINT("BONUSCLASS_MANA_RAW"))
	    {
	        return false;
	    }
	}

	return true;
}

bool CvPlot::canHaveImprovement(ImprovementTypes eImprovement, TeamTypes eTeam, bool bPotential, bool bIgnoreFeature) const
{
	CvPlot* pLoopPlot;
	bool bValid;
	int iI;

	FAssertMsg(eImprovement != NO_IMPROVEMENT, "Improvement is not assigned a valid value");
	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	bValid = false;

	if (isCity())
	{
		return false;
	}

//FfH: Modified by Kael 09/10/2008
//	if (isImpassable())
//	{
//		return false;
//	}
	if (isImpassable())
	{
		if (isPeak())
		{
			if (!GC.getImprovementInfo(eImprovement).isRequiresPeak())
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
    if (GC.getImprovementInfo(eImprovement).isRequiresPeak())
    {
		if (!isPeak())
		{
            return false;
		}
    }
//FfH: End Modify
	if (!GC.getMapINLINE().useClimateBasedStartPos() && GC.getMap().isPlanes()) {
		if (GC.getImprovementInfo(eImprovement).getNativePlane() != NO_PLANE) {
			if(getPlane() != GC.getImprovementInfo(eImprovement).getNativePlane()) {
				return false;
			}
		}
	}

	if (GC.getImprovementInfo(eImprovement).getMinWilderness() > getWilderness()) {
		return false;
	}
	if (GC.getImprovementInfo(eImprovement).getMaxWilderness() < getWilderness()) {
		return false;
	}
	
	if (GC.getImprovementInfo(eImprovement).isWater() != isWater())
	{
		return false;
	}

	if (getFeatureType() != NO_FEATURE)
	{
		if (GC.getFeatureInfo(getFeatureType()).isNoImprovement())
		{
			return false;
		}
	}

	if ((getBonusType(eTeam) != NO_BONUS) && GC.getImprovementInfo(eImprovement).isImprovementBonusMakesValid(getBonusType(eTeam)))
	{
		return true;
	}

	if (GC.getImprovementInfo(eImprovement).isNoFreshWater() && isFreshWater())
	{
		return false;
	}

	if (GC.getImprovementInfo(eImprovement).isRequiresFlatlands() && !isFlatlands())
	{
		return false;
	}

	if (GC.getImprovementInfo(eImprovement).isRequiresFeature() && (getFeatureType() == NO_FEATURE))
	{
		return false;
	}

	if (GC.getImprovementInfo(eImprovement).getTerrainMakesValid(getTerrainType()))
	{
		bValid = true;
	}

	if (GC.getImprovementInfo(eImprovement).isHillsMakesValid() && isHills())
	{
		bValid = true;
	}

	if (GC.getImprovementInfo(eImprovement).isFreshWaterMakesValid() && isFreshWater())
	{
		bValid = true;
	}

	if (GC.getImprovementInfo(eImprovement).isRiverSideMakesValid() && isRiverSide())
	{
		bValid = true;
	}

	if (GC.getImprovementInfo(eImprovement).isFloodPlainsMakesValid() && getFeatureType() == GC.getInfoTypeForString("FEATURE_FLOOD_PLAINS"))
	{
		bValid = true;
	}

	if ((getFeatureType() != NO_FEATURE) && GC.getImprovementInfo(eImprovement).getFeatureMakesValid(getFeatureType()))
	{
		bValid = true;
	}

	if (!bValid)
	{
		return false;
	}

	if (GC.getImprovementInfo(eImprovement).isRequiresRiverSide())
	{
		bValid = false;

		for (iI = 0; iI < NUM_CARDINALDIRECTION_TYPES; ++iI)
		{
			pLoopPlot = plotCardinalDirection(getX_INLINE(), getY_INLINE(), ((CardinalDirectionTypes)iI));

			if (pLoopPlot != NULL)
			{
				if (isRiverCrossing(directionXY(this, pLoopPlot)))
				{
					if (pLoopPlot->getImprovementType() != eImprovement)
					{
						bValid = true;
						break;
					}
				}
			}
		}

		if (!bValid)
		{
			return false;
		}
	}

	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		if (calculateNatureYield(((YieldTypes)iI), eTeam, bIgnoreFeature) < GC.getImprovementInfo(eImprovement).getPrereqNatureYield(iI))
		{
			return false;
		}
	}

	if ((getTeam() == NO_TEAM) || !(GET_TEAM(getTeam()).isIgnoreIrrigation()))
	{
		if (!bPotential && GC.getImprovementInfo(eImprovement).isRequiresIrrigation() && !isIrrigationAvailable())
		{
			return false;
		}
	}

	return true;
}

bool CvPlot::canHaveRiver() const
{
	if(isWater())
	{
		return false;
	}

	if(isRiver())
	{
		return false;
	}

	if (GC.getMapINLINE().useClimateBasedStartPos() || !GC.getMap().isPlanes()) {
		return true;
	}

	//only Planes logic after this
	if(getPlane()!=NO_PLANE && GC.getPlaneInfo(getPlane()).isNoRiver())
	{
		return false;
	}


	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if(pAdjacentPlot->getPlane() != getPlane())
			{
				return false;
			}
		}
	}

	return true;
}


bool CvPlot::canBuild(BuildTypes eBuild, PlayerTypes ePlayer, bool bTestVisible) const
{

//FfH: Added by Kael 11/10/2008
    if (isBuildDisabled())
    {
        return false;
    }
//FfH: End Add


	ImprovementTypes eImprovement;
	ImprovementTypes eFinalImprovementType;
	RouteTypes eRoute;
	bool bValid;

	/** TEMPORARY DISABLED **
	if(GC.getUSE_CAN_BUILD_CALLBACK())
	{
		CyArgsList argsList;
		argsList.add(getX_INLINE());
		argsList.add(getY_INLINE());
		argsList.add((int)eBuild);
		argsList.add((int)ePlayer);
		long lResult=0;
		gDLL->getPythonIFace()->callFunction(PYGameModule, "canBuild", argsList.makeFunctionArgs(), &lResult);
		if (lResult >= 1)
		{
			return true;
		}
		else if (lResult == 0)
		{
			return false;
		}
	}
	**/
	if (eBuild == NO_BUILD)
	{
		return false;
	}

	bValid = false;

	eImprovement = ((ImprovementTypes)(GC.getBuildInfo(eBuild).getImprovement()));

	if (eImprovement != NO_IMPROVEMENT)
	{
		//SpyFanatic: if the build will remove the feature check without considering the feature (e.g. farm on a pasture/jungle plot near a river)
		bool bIgnoreFeature = getFeatureType() != NO_FEATURE && GC.getBuildInfo(eBuild).isFeatureRemove(getFeatureType());
		if (!canHaveImprovement(eImprovement, GET_PLAYER(ePlayer).getTeam(), bTestVisible, bIgnoreFeature))
		{
			return false;
		}

		// Super Forts begin *build*
		if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(eImprovement).getUniqueRange() > 0)
		{
			int iUniqueRange = GC.getImprovementInfo(eImprovement).getUniqueRange();
			for (int iDX = -iUniqueRange; iDX <= iUniqueRange; iDX++) 
			{
				for (int iDY = -iUniqueRange; iDY <= iUniqueRange; iDY++)
				{
					CvPlot *pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
					if (pLoopPlot != NULL && pLoopPlot->getImprovementType() != NO_IMPROVEMENT)
					{
						if (finalImprovementUpgrade(pLoopPlot->getImprovementType()) == finalImprovementUpgrade(eImprovement))
						{
							return false;
						}
					}
				}
			}
		}
		// Super Forts end

		if (getImprovementType() != NO_IMPROVEMENT)
		{
			if (GC.getImprovementInfo(getImprovementType()).isPermanent())
			{
				//SpyFanatic: allow destroy of mana shrine, witch hut and ancient tower for human player
				if(!GET_PLAYER(ePlayer).isHuman() || (getImprovementType() != (ImprovementTypes)GC.getIMPROVEMENT_MANA_SHRINE() && getImprovementType() != (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WITCH_HUT") && getImprovementType() != (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_TOWER") && getImprovementType() != (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_LOST_TEMPLE")))
				{
					return false;
				}
			}

			// Super Forts begin *AI_worker* - prevent forts from being built over when outside culture range
			if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(getImprovementType()).getCulture() > 0 /*isActsAsCity()*/)
			{
				if (!isWithinCultureRange(ePlayer) && !(getCultureRangeForts(ePlayer) > 1))
				{
					return false;
				}
			}
			// Super Forts end

			if (getImprovementType() == eImprovement)
			{
				return false;
			}

			eFinalImprovementType = finalImprovementUpgrade(getImprovementType());

			if (eFinalImprovementType != NO_IMPROVEMENT)
			{
				if (eFinalImprovementType == finalImprovementUpgrade(eImprovement))
				{
					return false;
				}
			}
		}

		if (!bTestVisible)
		{
			if (GET_PLAYER(ePlayer).getTeam() != getTeam())
			{
				//outside borders can't be built in other's culture
				if (GC.getImprovementInfo(eImprovement).isOutsideBorders())
				{
					if (getTeam() != NO_TEAM)
					{
						return false;
					}
				}
				else //only buildable in own culture
				{
					return false;
				}
			}
			// Super Forts begin *AI_worker* - prevent workers from two different players from building a fort in the same plot
			if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(eImprovement).getCulture() > 0 /*isActsAsCity()*/)
			{
				CLinkList<IDInfo> oldUnits;
				CLLNode<IDInfo>* pUnitNode = headUnitNode();
				CvUnit* pLoopUnit;

				while (pUnitNode != NULL)
				{
					oldUnits.insertAtEnd(pUnitNode->m_data);
					pUnitNode = nextUnitNode(pUnitNode);
				}

				pUnitNode = oldUnits.head();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = nextUnitNode(pUnitNode);
					if(pLoopUnit->getOwner() != ePlayer)	
					{
						if(pLoopUnit->getBuildType() != NO_BUILD)
						{
							ImprovementTypes eImprovementBuild = (ImprovementTypes)(GC.getBuildInfo(pLoopUnit->getBuildType()).getImprovement());
							if(eImprovementBuild != NO_IMPROVEMENT)
							{
								if(GC.getImprovementInfo(eImprovementBuild).isActsAsCity())
								{
									return false;
								}
							}
						}
					}
				}
			}
			// Super Forts end
		}

		bValid = true;
	}

	eRoute = ((RouteTypes)(GC.getBuildInfo(eBuild).getRoute()));

	if (eRoute != NO_ROUTE)
	{
		if (getRouteType() != NO_ROUTE)
		{
			if (GC.getRouteInfo(getRouteType()).getValue() >= GC.getRouteInfo(eRoute).getValue())
			{
				return false;
			}
		}

		// Malakim do not allow roads on terrain you can already trade on
		if(GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isTerrainTrade(getTerrainType()))
			return false;

		if (!bTestVisible)
		{
			if (GC.getRouteInfo(eRoute).getPrereqBonus() != NO_BONUS)
			{
				if (!isAdjacentPlotGroupConnectedBonus(ePlayer, ((BonusTypes)(GC.getRouteInfo(eRoute).getPrereqBonus()))))
				{
					return false;
				}
			}

			bool bFoundValid = true;
			for (int i = 0; i < GC.getNUM_ROUTE_PREREQ_OR_BONUSES(); ++i)
			{
				if (NO_BONUS != GC.getRouteInfo(eRoute).getPrereqOrBonus(i))
				{
					bFoundValid = false;

					if (isAdjacentPlotGroupConnectedBonus(ePlayer, ((BonusTypes)(GC.getRouteInfo(eRoute).getPrereqOrBonus(i)))))
					{
						bFoundValid = true;
						break;
					}
				}
			}

			if (!bFoundValid)
			{
				return false;
			}
		}

		bValid = true;
	}

	if (getFeatureType() != NO_FEATURE)
	{
		if (GC.getBuildInfo(eBuild).isFeatureRemove(getFeatureType()))
		{
			if (isOwned() && (GET_PLAYER(ePlayer).getTeam() != getTeam()) && !atWar(GET_PLAYER(ePlayer).getTeam(), getTeam())

//FfH: Added by Kael 04/24/2008
              && !GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).isMaintainFeatures(getFeatureType())
//FfH: End Add

			)
			{
				return false;
			}

			bValid = true;
		}
	}

	if (GC.getBuildInfo(eBuild).getTerrainPre() != NO_TERRAIN &&
		GC.getBuildInfo(eBuild).getTerrainPost() != NO_TERRAIN) {
		if (getTerrainType() == GC.getBuildInfo(eBuild).getTerrainPre()) {
			bValid = true;
		}
	}

	return bValid;
}


int CvPlot::getBuildTime(BuildTypes eBuild) const
{
	int iTime;

	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	iTime = GC.getBuildInfo(eBuild).getTime();

	if (getFeatureType() != NO_FEATURE)
	{
		iTime += GC.getBuildInfo(eBuild).getFeatureTime(getFeatureType());
	}

	iTime *= std::max(0, (GC.getTerrainInfo(getTerrainType()).getBuildModifier() + 100));
	iTime /= 100;

	iTime *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getBuildPercent();
	iTime /= 100;

	iTime *= GC.getEraInfo(GC.getGameINLINE().getStartEra()).getBuildPercent();
	iTime /= 100;

	return iTime;
}


int CvPlot::getBuildTurnsLeft(BuildTypes eBuild, int iNowExtra, int iThenExtra) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iNowBuildRate;
	int iThenBuildRate;
	int iBuildLeft;
	int iTurnsLeft;

	iNowBuildRate = iNowExtra;
	iThenBuildRate = iThenExtra;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->getBuildType() == eBuild)
		{
			if (pLoopUnit->canMove())
			{
				iNowBuildRate += pLoopUnit->workRate(false);
			}
			iThenBuildRate += pLoopUnit->workRate(true);
		}
	}

	if (iThenBuildRate == 0)
	{
		//this means it will take forever under current circumstances
		return MAX_INT;
	}

	iBuildLeft = getBuildTime(eBuild);

	iBuildLeft -= getBuildProgress(eBuild);
	iBuildLeft -= iNowBuildRate;

	iBuildLeft = std::max(0, iBuildLeft);

	iTurnsLeft = (iBuildLeft / iThenBuildRate);

	if ((iTurnsLeft * iThenBuildRate) < iBuildLeft)
	{
		iTurnsLeft++;
	}

	iTurnsLeft++;
	return std::max(1, iTurnsLeft);
}


int CvPlot::getFeatureProduction(BuildTypes eBuild, TeamTypes eTeam, CvCity** ppCity) const
{
	int iProduction;

	if (getFeatureType() == NO_FEATURE)
	{
		return 0;
	}

	*ppCity = getWorkingCity();

	if (*ppCity == NULL)
	{
		*ppCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), NO_PLAYER, eTeam, false);
	}

	if (*ppCity == NULL)
	{
		return 0;
	}

	iProduction = (GC.getBuildInfo(eBuild).getFeatureProduction(getFeatureType()) - (std::max(0, (plotDistance(getX_INLINE(), getY_INLINE(), (*ppCity)->getX_INLINE(), (*ppCity)->getY_INLINE()) - 2)) * 5));

	iProduction *= std::max(0, (GET_PLAYER((*ppCity)->getOwnerINLINE()).getFeatureProductionModifier() + 100));
	iProduction /= 100;

	iProduction *= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getFeatureProductionPercent();
	iProduction /= 100;

	if (getTeam() != eTeam)
	{
		iProduction *= GC.getDefineINT("DIFFERENT_TEAM_FEATURE_PRODUCTION_PERCENT");
		iProduction /= 100;
	}

	return std::max(0, iProduction);
}


CvUnit* CvPlot::getBestDefender(PlayerTypes eOwner, PlayerTypes eAttackingPlayer, const CvUnit* pAttacker, bool bTestAtWar, bool bTestPotentialEnemy, bool bTestCanMove) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;

	pBestUnit = NULL;

	//FfH: Added by Kael 10/28/2008
	bool bNoGuard = true;
	pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);
		if (pLoopUnit->isTargetWeakestUnitCounter())
		{
			bNoGuard = false;
		}
	}
	//FfH: End Add

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((eOwner == NO_PLAYER) || (pLoopUnit->getOwnerINLINE() == eOwner))
		{
			if ((eAttackingPlayer == NO_PLAYER) || !(pLoopUnit->isInvisible(GET_PLAYER(eAttackingPlayer).getTeam(), false)))
			{
				if (!bTestAtWar || eAttackingPlayer == NO_PLAYER || pLoopUnit->isEnemy(GET_PLAYER(eAttackingPlayer).getTeam(), this) || (NULL != pAttacker && pAttacker->isEnemy(GET_PLAYER(pLoopUnit->getOwnerINLINE()).getTeam(), this)))
				{
					if (!bTestPotentialEnemy || (eAttackingPlayer == NO_PLAYER) || pLoopUnit->isPotentialEnemy(GET_PLAYER(eAttackingPlayer).getTeam(), this) || (NULL != pAttacker && pAttacker->isPotentialEnemy(GET_PLAYER(pLoopUnit->getOwnerINLINE()).getTeam(), this)))
					{
						if (!bTestCanMove || (pLoopUnit->canMove() && !(pLoopUnit->isCargo())))
						{
							if ((pAttacker == NULL) || (pAttacker->getDomainType() != DOMAIN_AIR) || (pLoopUnit->getDamage() < pAttacker->airCombatLimit()))
							{

								//FfH: Modified by Kael 10/28/2008
								//                              if (pLoopUnit->isBetterDefenderThan(pBestUnit, pAttacker))
								//                              {
								//                                  pBestUnit = pLoopUnit;
								//                              }
								if (pAttacker != NULL && pAttacker->isTargetWeakestUnit() && pAttacker->getDomainType() == pLoopUnit->getDomainType() && pLoopUnit->baseCombatStrDefense() != 0 && bNoGuard)
								{
									if (pBestUnit != NULL && (pAttacker->getDomainType() != pBestUnit->getDomainType() || pBestUnit->baseCombatStrDefense() == 0))
									{
										pBestUnit = NULL;
									}
									if (pBestUnit == NULL || pBestUnit->isBetterDefenderThan(pLoopUnit, pAttacker))
									{
										pBestUnit = pLoopUnit;
									}
								}
								else
								{
									if (pLoopUnit->isBetterDefenderThan(pBestUnit, pAttacker) && (pAttacker == NULL || (pAttacker->getOwner() != pLoopUnit->getOwner())))
									{
										pBestUnit = pLoopUnit;
									}
								}
								//FfH: End Modify

							}
						}
					}
				}
			}
		}
	}

	return pBestUnit;
}

// returns a sum of the strength (adjusted by firepower) of all the units on a plot
int CvPlot::AI_sumStrength(PlayerTypes eOwner, PlayerTypes eAttackingPlayer, DomainTypes eDomainType, bool bDefensiveBonuses, bool bTestAtWar, bool bTestPotentialEnemy) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int	strSum = 0;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((eOwner == NO_PLAYER) || (pLoopUnit->getOwnerINLINE() == eOwner))
		{
			if ((eAttackingPlayer == NO_PLAYER) || !(pLoopUnit->isInvisible(GET_PLAYER(eAttackingPlayer).getTeam(), false)))
			{
				if (!bTestAtWar || (eAttackingPlayer == NO_PLAYER) || atWar(GET_PLAYER(eAttackingPlayer).getTeam(), pLoopUnit->getTeam()))
				{
					if (!bTestPotentialEnemy || (eAttackingPlayer == NO_PLAYER) || pLoopUnit->isPotentialEnemy(GET_PLAYER(eAttackingPlayer).getTeam(), this))
					{
						// we may want to be more sophisticated about domains
						// somewhere we need to check to see if this is a city, if so, only land units can defend here, etc
						if (eDomainType == NO_DOMAIN || (pLoopUnit->getDomainType() == eDomainType))
						{
							const CvPlot* pPlot = NULL;

							if (bDefensiveBonuses)
								pPlot = this;

							strSum += pLoopUnit->currEffectiveStr(pPlot, NULL);
						}
					}
				}
			}
		}
	}

	return strSum;
}


CvUnit* CvPlot::getSelectedUnit() const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->IsSelected())
		{
			return pLoopUnit;
		}
	}

	return NULL;
}


int CvPlot::getUnitPower(PlayerTypes eOwner) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iCount;

	iCount = 0;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((eOwner == NO_PLAYER) || (pLoopUnit->getOwnerINLINE() == eOwner))
		{
			iCount += pLoopUnit->getUnitInfo().getPowerValue();
		}
	}

	return iCount;
}

// Super Forts begin *bombard*
bool CvPlot::isBombardable(const CvUnit* pUnit) const
{
	ImprovementTypes eImprovement;

	if (NULL != pUnit && !pUnit->isEnemy(getTeam()))
	{
		return false;
	}

	eImprovement = getImprovementType();
	if(eImprovement == NO_IMPROVEMENT)
	{
		return false;
	}
	else
	{
		if(GC.getImprovementInfo(eImprovement).isBombardable())
		{
			return (getDefenseDamage() < GC.getImprovementInfo(eImprovement).getDefenseModifier());
		}
	}
	return false;
}

bool CvPlot::isBombarded() const
{
	return m_bBombarded;
}

void CvPlot::setBombarded(bool bNewValue)
{
	m_bBombarded = bNewValue;
}

int CvPlot::getDefenseDamage() const																
{
	return m_iDefenseDamage;
}

void CvPlot::changeDefenseDamage(int iChange)
{
	if ((iChange != 0) && (getImprovementType() != NO_IMPROVEMENT))
	{
		m_iDefenseDamage = range((m_iDefenseDamage + iChange), 0, GC.getImprovementInfo(getImprovementType()).getDefenseModifier());

		if (iChange > 0)
		{
			setBombarded(true);
		}
	}
}
// Super Forts end

// Super Forts begin *culture*
int CvPlot::getCultureRangeForts(PlayerTypes ePlayer) const
{
	if (NULL == m_aiCultureRangeForts)
	{
		return 0;
	}

	return m_aiCultureRangeForts[ePlayer];
}

void CvPlot::setCultureRangeForts(PlayerTypes ePlayer, int iNewValue)
{
	if (getCultureRangeForts(ePlayer) != iNewValue /*&& ePlayer < BARBARIAN_PLAYER*/)
	{
		if(NULL == m_aiCultureRangeForts)
		{
			m_aiCultureRangeForts = new short[MAX_PLAYERS];
			for (int iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				m_aiCultureRangeForts[iI] = 0;
			}
		}

		m_aiCultureRangeForts[ePlayer] = iNewValue;
		
		if(getCulture(ePlayer) == 0)
		{
			changeCulture(ePlayer, 1, false);
		}
	}
}

void CvPlot::changeCultureRangeForts(PlayerTypes ePlayer, int iChange)
{
	if (0 != iChange)
	{
		setCultureRangeForts(ePlayer, (getCultureRangeForts(ePlayer) + iChange));
	}
}

bool CvPlot::isWithinFortCultureRange(PlayerTypes ePlayer) const
{
	return (getCultureRangeForts(ePlayer) > 0);
}

void CvPlot::changeCultureRangeFortsWithinRange(PlayerTypes ePlayer, int iChange, int iRange, bool bUpdate)
{
	CvPlot* pLoopPlot;
	int iDX, iDY;
	int iCultureDistance;

	if ((0 != iChange) && (iRange >= 0) /*&& ePlayer < BARBARIAN_PLAYER*/)
	{
		for (iDX = -iRange; iDX <= iRange; iDX++)
		{
			for (iDY = -iRange; iDY <= iRange; iDY++)
			{
				iCultureDistance = plotDistance(0, 0, iDX, iDY);

				if(iCultureDistance <= iRange)
				{
					pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

					if (pLoopPlot != NULL)
					{
						pLoopPlot->changeCultureRangeForts(ePlayer, iChange);
//oosLog("CultureFort","Plot %d %d update:%d change:%d range:%d x:%d y:%d",getX_INLINE(),getY_INLINE(),bUpdate,iChange,iRange,pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE());
						if(bUpdate)
						{
							pLoopPlot->updateCulture(true,true);
						}
					}
				}
			}
		}
	}
}

void CvPlot::doImprovementCulture()
{
	CvPlot* pLoopPlot;
	int iDX, iDY;
	int iCultureDistance, iCulture, iCultureRange;
	ImprovementTypes eImprovement;
	PlayerTypes ePlayer;

	eImprovement = getImprovementType();
	if (eImprovement != NO_IMPROVEMENT)
	{
		ePlayer = getOwnerINLINE();
		if(ePlayer != NO_PLAYER /* && ePlayer < BARBARIAN_PLAYER*/)
		{
			iCulture = GC.getImprovementInfo(eImprovement).getCulture();
			if(iCulture > 0)
			{
				iCultureRange = GC.getImprovementInfo(eImprovement).getCultureRange();
				
				if(iCultureRange > 0)
				{
					for (iDX = -iCultureRange; iDX <= iCultureRange; iDX++)
					{
						for (iDY = -iCultureRange; iDY <= iCultureRange; iDY++)
						{
							iCultureDistance = plotDistance(0, 0, iDX, iDY);

							if(iCultureDistance <= iCultureRange)
							{
								pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

								if (pLoopPlot != NULL)
								{
									int iChange = ((iCultureRange - ((iCultureDistance == 0) ? 1 : iCultureDistance))*iCulture) + iCulture;
									pLoopPlot->changeCulture(ePlayer,iChange,false);
								}
							}
						}
					}
				}
				else
				{
					changeCulture(ePlayer,iCulture,false);
				}
			}
		}
	}
}
// Super Forts end

// Super Forts begin *canal* *choke*
int CvPlot::countRegionPlots(const CvPlot* pInvalidPlot) const
{
	int iCount = 0;
	int iInvalidPlot = (pInvalidPlot == NULL) ? 0 : GC.getMapINLINE().plotNum(pInvalidPlot->getX_INLINE(), pInvalidPlot->getY_INLINE()) + 1;
	FAStar* pRegionFinder = gDLL->getFAStarIFace()->create();
	gDLL->getFAStarIFace()->Initialize(pRegionFinder, GC.getMapINLINE().getGridWidthINLINE(), GC.getMapINLINE().getGridHeightINLINE(), GC.getMapINLINE().isWrapXINLINE(), GC.getMapINLINE().isWrapYINLINE(), 
		NULL, NULL, NULL, stepValid, NULL, countPlotGroup, NULL);
	gDLL->getFAStarIFace()->SetData(pRegionFinder, &iCount);
	// Note to self: for GeneratePath() should bReuse be true or false?
	gDLL->getFAStarIFace()->GeneratePath(pRegionFinder, getX_INLINE(), getY_INLINE(), -1, -1, false, iInvalidPlot, false);
	gDLL->getFAStarIFace()->destroy(pRegionFinder);
	return iCount;
}

int CvPlot::countAdjacentPassableSections(bool bWater) const
{
	CvPlot* pAdjacentPlot;
	int iPassableSections = 0;
	bool bInPassableSection = false;

	// Are we looking for water passages or land passages?
	if(bWater)
	{
		bool bPlotIsWater = isWater();
		// This loop is for water
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));
			if(pAdjacentPlot != NULL)
			{
				if(pAdjacentPlot->isWater())
				{
					// Don't count diagonal hops across land isthmus
					if (bPlotIsWater && !isCardinalDirection((DirectionTypes)iI))
					{
						if (!(GC.getMapINLINE().plotINLINE(getX_INLINE(), pAdjacentPlot->getY_INLINE())->isWater()) && !(GC.getMapINLINE().plotINLINE(pAdjacentPlot->getX_INLINE(), getY_INLINE())->isWater()))
						{
							continue;
						}
					}
					if(pAdjacentPlot->isImpassable())
					{
						if(isCardinalDirection((DirectionTypes)iI))
						{
							bInPassableSection = false;
						}
					}
					else if(!bInPassableSection)
					{
						bInPassableSection = true;
						++iPassableSections;
					}
				}
				else
				{
					bInPassableSection = false;
				}
			}
		}
	}
	else
	{
		// This loop is for land
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));
			if(pAdjacentPlot != NULL)
			{
				if(pAdjacentPlot->isWater() || pAdjacentPlot->isImpassable())
				{	
					if(isCardinalDirection((DirectionTypes)iI))
					{
						bInPassableSection = false;
					}
				}
				else if(!bInPassableSection)
				{
					bInPassableSection = true;
					++iPassableSections;
				}
			}
		}
	}
	// Corner Case Correction
	pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_NORTH);
	if(pAdjacentPlot != NULL && (bWater == pAdjacentPlot->isWater()) && !pAdjacentPlot->isImpassable())
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_NORTHWEST);
		if(pAdjacentPlot != NULL && (bWater == pAdjacentPlot->isWater()))
		{
			if(pAdjacentPlot->isImpassable())
			{
				pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_WEST);
				if(pAdjacentPlot != NULL && (bWater == pAdjacentPlot->isWater()) && !pAdjacentPlot->isImpassable())
				{
					--iPassableSections;
				}
			}
			else
			{
				--iPassableSections;
			}
		}
		else if(!bWater)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_WEST);
			if(pAdjacentPlot != NULL && !pAdjacentPlot->isWater() && !pAdjacentPlot->isImpassable())
			{
				--iPassableSections;
			}
		}
	}
	return iPassableSections;
}

int CvPlot::countImpassableCardinalDirections() const
{
	CvPlot* pAdjacentPlot;
	int iCount = 0;
	for(int iI = 0; iI < NUM_CARDINALDIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotCardinalDirection(getX_INLINE(), getY_INLINE(), ((CardinalDirectionTypes)iI));
		if(pAdjacentPlot != NULL)
		{
			if(pAdjacentPlot->isImpassable() || (area() != pAdjacentPlot->area()))
			{
				++iCount;
			}
		}
	}
	return iCount;
}
// Super Forts end

// Super Forts begin *canal*
int CvPlot::getCanalValue() const																
{
	return m_iCanalValue;
}

void CvPlot::setCanalValue(int iNewValue)
{
	m_iCanalValue = iNewValue;
}

void CvPlot::calculateCanalValue()
{
	bool bInWaterSection;
	CvPlot *pAdjacentPlot, *apPlotsToCheck[4];
	int iWaterSections, iPlotsFound, iMaxDistance;
	int iCanalValue = 0;

	if(isCoastalLand() && !isImpassable())
	{
		iWaterSections = countAdjacentPassableSections(true);
		if(iWaterSections > 1)
		{
			iMaxDistance = 0;
			iPlotsFound = 0;
			bInWaterSection = false;
			// Find appropriate plots to be used for path distance calculations
			for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));
				if(pAdjacentPlot != NULL)
				{
					if(pAdjacentPlot->isWater())
					{
						if(pAdjacentPlot->isImpassable())
						{
							if(isCardinalDirection((DirectionTypes)iI))
							{
								bInWaterSection = false;
							}
						}
						else if(!bInWaterSection)
						{
							bInWaterSection = true;
							apPlotsToCheck[iPlotsFound] = pAdjacentPlot;
							if((++iPlotsFound) == iWaterSections)
								break;
						}
					}
					else
					{
						bInWaterSection = false;
					}
				}
			}
			// Find the max path distance out of all possible pairs of plots
			for (int iI = 0; iI < (iPlotsFound - 1); ++iI)
			{
				for (int iJ = iI + 1; iJ < iPlotsFound; ++iJ)
				{
					if(!apPlotsToCheck[iI]->isLake() || !apPlotsToCheck[iJ]->isLake())
					{
						int iDistance = GC.getMapINLINE().calculatePathDistance(apPlotsToCheck[iI], apPlotsToCheck[iJ]);
						if(iDistance == -1)
						{
						
							// If no path was found then value is based off the number of plots in the region minus a minimum area
							iDistance = std::min(apPlotsToCheck[iI]->countRegionPlots(), apPlotsToCheck[iJ]->countRegionPlots()) - 7;
							iDistance *= 4;
						}
						else
						{
							// Path already would have required 2 steps, and I don't care that much about saving just 1 or 2 moves
							iDistance -= 4;
						}
						if(iDistance > iMaxDistance)
						{
							iMaxDistance = iDistance;
						}
					}
				}
			}
			iCanalValue = iMaxDistance * (iPlotsFound - 1);
		}
	}
	setCanalValue(iCanalValue);
}
// Super Forts end

// Super Forts begin *choke*
int CvPlot::getChokeValue() const
{
	return m_iChokeValue;
}

void CvPlot::setChokeValue(int iNewValue)
{
	m_iChokeValue = iNewValue;
}

void CvPlot::calculateChokeValue()
{
	bool bInPassableSection;
	CvPlot *pAdjacentPlot, *apPlotsToCheck[4];
	int iPassableSections, iPlotsFound, iMaxDistance;
	int iChokeValue = 0;
	bool bWater = isWater();

	if(!isImpassable() && countImpassableCardinalDirections() > 1)
	{
		iPassableSections = countAdjacentPassableSections(bWater);
		if(iPassableSections > 1)
		{
			iMaxDistance = 0;
			iPlotsFound = 0;
			bInPassableSection = false;
			// Find appropriate plots to be used for path distance calculations
			for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));
				if(pAdjacentPlot != NULL)
				{
					if(pAdjacentPlot->isWater() == bWater)
					{	
						// Don't count diagonal hops across land isthmus
						if (bWater && !isCardinalDirection((DirectionTypes)iI))
						{
							if (!(GC.getMapINLINE().plotINLINE(getX_INLINE(), pAdjacentPlot->getY_INLINE())->isWater()) && !(GC.getMapINLINE().plotINLINE(pAdjacentPlot->getX_INLINE(), getY_INLINE())->isWater()))
							{
								continue;
							}
						}
						if(pAdjacentPlot->isImpassable())
						{
							if(isCardinalDirection((DirectionTypes)iI))
							{
								bInPassableSection = false;
							}
						}
						else if(!bInPassableSection)
						{
							bInPassableSection = true;
							apPlotsToCheck[iPlotsFound] = pAdjacentPlot;
							if((++iPlotsFound) == iPassableSections)
								break;
						}
					}
					else if(bWater || isCardinalDirection((DirectionTypes)iI))
					{
						bInPassableSection = false;
					}
				}
			}
			// Find the max path distance out of all possible pairs of plots
			for (int iI = 0; iI < (iPlotsFound - 1); ++iI)
			{
				for (int iJ = iI + 1; iJ < iPlotsFound; ++iJ)
				{
					int iDistance = GC.getMapINLINE().calculatePathDistance(apPlotsToCheck[iI], apPlotsToCheck[iJ], this);
					if(iDistance == -1)
					{
						// If no path was found then value is based off the number of plots in the region minus a minimum area
						iDistance = std::min(apPlotsToCheck[iI]->countRegionPlots(this), apPlotsToCheck[iJ]->countRegionPlots(this)) - 4;
						iDistance *= 4;
					}
					else
					{
						// Path already would have required 2 steps, but we forced the enemy to go another way so there is some value
						iDistance -= 1;
					}
					if(iDistance > iMaxDistance)
					{
						iMaxDistance = iDistance;
					}
				}
			}
			iChokeValue = iMaxDistance * (iPlotsFound - 1);
		}
	}
	setChokeValue(iChokeValue);
}
// Super Forts end

int CvPlot::defenseModifier(TeamTypes eDefender, bool bIgnoreBuilding, bool bHelp) const
{
	CvCity* pCity;
	ImprovementTypes eImprovement;
	int iModifier;

	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

/*************************************************************************************************/
/**	ADDON (make Defensemod of Terrain and Feature stack) Sephi		                        	**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	iModifier = ((getFeatureType() == NO_FEATURE) ? GC.getTerrainInfo(getTerrainType()).getDefenseModifier() : GC.getFeatureInfo(getFeatureType()).getDefenseModifier());
/**								----  End Original Code  ----									**/
	iModifier = GC.getTerrainInfo(getTerrainType()).getDefenseModifier();
	if(getFeatureType() != NO_FEATURE && (!isCity()))
	{
        iModifier+=GC.getFeatureInfo(getFeatureType()).getDefenseModifier();
	}
/*************************************************************************************************/
/**	ADDON   								END													**/
/*************************************************************************************************/

	if (isHills())
	{
		iModifier += GC.getHILLS_EXTRA_DEFENSE();
	}

	if (bHelp)
	{
		eImprovement = getRevealedImprovementType(GC.getGameINLINE().getActiveTeam(), false);
	}
	else
	{
		eImprovement = getImprovementType();
	}

	if (eImprovement != NO_IMPROVEMENT)
	{
		if (eDefender != NO_TEAM && (getTeam() == NO_TEAM || GET_TEAM(eDefender).isFriendlyTerritory(getTeam())))
		{
			// Super Forts begin *bombard*
			iModifier += GC.getImprovementInfo(eImprovement).getDefenseModifier() -  (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) ? getDefenseDamage() : 0);
			// iModifier += GC.getImprovementInfo(eImprovement).getDefenseModifier(); - Original code
			// Super Forts end
		}
	}

	if (!bHelp)
	{
		pCity = getPlotCity();

		if (pCity != NULL)
		{
			iModifier += pCity->getDefenseModifier(bIgnoreBuilding);
		}

//FfH: Added by Kael
        else
        {
            if (eDefender != NO_TEAM && (getTeam() == NO_TEAM || GET_TEAM(eDefender).isFriendlyTerritory(getTeam())))
            {
                iModifier += getRangeDefense(eDefender, 3, false, true);
            }
        }
//FfH: End Add

	}

	return iModifier;
}


int CvPlot::movementCost(const CvUnit* pUnit, const CvPlot* pFromPlot) const
{
	int iRegularCost;
	int iRouteCost;
	int iRouteFlatCost;

	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	if (pUnit->flatMovementCost() || (pUnit->getDomainType() == DOMAIN_AIR))
	{
		return GC.getMOVE_DENOMINATOR();
	}

	if (pUnit->isHuman())
	{
		if (!isRevealed(pUnit->getTeam(), false))
		{
			return pUnit->maxMoves();
		}
	}

	if (!pFromPlot->isValidDomainForLocation(*pUnit))
	{
		return pUnit->maxMoves();
	}

	if (!isValidDomainForAction(*pUnit))
	{
		return GC.getMOVE_DENOMINATOR();
	}

	FAssert(pUnit->getDomainType() != DOMAIN_IMMOBILE);

	if (pUnit->ignoreTerrainCost())
	{
		iRegularCost = 1;
	}
	else
	{
		iRegularCost = ((getFeatureType() == NO_FEATURE) ? GC.getTerrainInfo(getTerrainType()).getMovementCost() : GC.getFeatureInfo(getFeatureType()).getMovementCost());

		if (isHills())
		{
			iRegularCost += GC.getHILLS_EXTRA_MOVEMENT();
		}

		if (iRegularCost > 0)
		{
			iRegularCost = std::max(1, (iRegularCost - pUnit->getExtraMoveDiscount()));
		}
	}
//BUGFfH: Modified by Denev 2009/09/29
/*	Eliminate Hasting micromanagement	*/
/*
	bool bHasTerrainCost = (iRegularCost > 1);

	iRegularCost = std::min(iRegularCost, pUnit->baseMoves());

	iRegularCost *= GC.getMOVE_DENOMINATOR();

	if (bHasTerrainCost)
	{
		if (((getFeatureType() == NO_FEATURE) ? pUnit->isTerrainDoubleMove(getTerrainType()) : pUnit->isFeatureDoubleMove(getFeatureType())) ||
			(isHills() && pUnit->isHillsDoubleMove()))
		{
			iRegularCost /= 2;
		}
	}
*/
	if ((getFeatureType() != NO_FEATURE && pUnit->isFeatureDoubleMove(getFeatureType()) ||
		(pUnit->isTerrainDoubleMove(getTerrainType())) ||
		(isHills() && pUnit->isHillsDoubleMove())))
	{
		iRegularCost = std::min(iRegularCost, pUnit->baseMoves());
		iRegularCost *= GC.getMOVE_DENOMINATOR();
		iRegularCost /= 2;
	}
	else
	{
		iRegularCost *= GC.getMOVE_DENOMINATOR();
	}
//BUGFfH: End Modify

	if (pFromPlot->isValidRoute(pUnit) && isValidRoute(pUnit) && ((GET_TEAM(pUnit->getTeam()).isBridgeBuilding() || !(pFromPlot->isRiverCrossing(directionXY(pFromPlot, this))))))
	{
		iRouteCost = std::max((GC.getRouteInfo(pFromPlot->getRouteType()).getMovementCost() + GET_TEAM(pUnit->getTeam()).getRouteChange(pFromPlot->getRouteType())),
			               (GC.getRouteInfo(getRouteType()).getMovementCost() + GET_TEAM(pUnit->getTeam()).getRouteChange(getRouteType())));
		iRouteFlatCost = std::max((GC.getRouteInfo(pFromPlot->getRouteType()).getFlatMovementCost() * pUnit->baseMoves()),
			                   (GC.getRouteInfo(getRouteType()).getFlatMovementCost() * pUnit->baseMoves()));
	}
	else
	{
		iRouteCost = MAX_INT;
		iRouteFlatCost = MAX_INT;
	}

	return std::max(1, std::min(iRegularCost, std::min(iRouteCost, iRouteFlatCost)));
}

int CvPlot::getExtraMovePathCost() const
{
	return GC.getGameINLINE().getPlotExtraCost(getX_INLINE(), getY_INLINE());
}


void CvPlot::changeExtraMovePathCost(int iChange)
{
	GC.getGameINLINE().changePlotExtraCost(getX_INLINE(), getY_INLINE(), iChange);
}

bool CvPlot::isAdjacentOwned() const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isOwned())
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isAdjacentPlayer(PlayerTypes ePlayer, bool bLandOnly) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->getOwnerINLINE() == ePlayer)
			{
				if (!bLandOnly || !(pAdjacentPlot->isWater()))
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvPlot::isAdjacentTeam(TeamTypes eTeam, bool bLandOnly) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->getTeam() == eTeam)
			{
				if (!bLandOnly || !(pAdjacentPlot->isWater()))
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvPlot::isWithinCultureRange(PlayerTypes ePlayer) const
{
	int iI;

	for (iI = 0; iI < GC.getNumCultureLevelInfos(); ++iI)
	{
		if (isCultureRangeCity(ePlayer, iI))
		{
			return true;
		}
	}

	return false;
}


int CvPlot::getNumCultureRangeCities(PlayerTypes ePlayer) const
{
	int iCount;
	int iI;

	iCount = 0;

	for (iI = 0; iI < GC.getNumCultureLevelInfos(); ++iI)
	{
		iCount += getCultureRangeCities(ePlayer, iI);
	}

	return iCount;
}


PlayerTypes CvPlot::calculateCulturalOwner() const
{
	PROFILE("CvPlot::calculateCulturalOwner()")

	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pLoopPlot;
	PlayerTypes eBestPlayer;
	bool bValid;
	int iCulture;
	int iBestCulture;
	int iPriority;
	int iBestPriority;
	int iI;

	if (isForceUnowned())
	{
		return NO_PLAYER;
	}

	iBestCulture = 0;
	eBestPlayer = NO_PLAYER;

	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iCulture = getCulture((PlayerTypes)iI);

			if (iCulture > 0)
			{
				// Super Forts begin *culture* - modified if statement
				if (isWithinCultureRange((PlayerTypes)iI) || (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && /*((PlayerTypes)iI) < BARBARIAN_PLAYER &&*/ isWithinFortCultureRange((PlayerTypes)iI)))
				//if (isWithinCultureRange((PlayerTypes)iI)) - Original Code
				// Super Forts end
				{
					if ((iCulture > iBestCulture) || ((iCulture == iBestCulture) && (getOwnerINLINE() == iI)))
					{
						iBestCulture = iCulture;
						eBestPlayer = ((PlayerTypes)iI);
					}
				}
			}
		}
	}

	if (!isCity())
	{
		if (eBestPlayer != NO_PLAYER)
		{
			iBestPriority = MAX_INT;
			pBestCity = NULL;

			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					pLoopCity = pLoopPlot->getPlotCity();

					if (pLoopCity != NULL)
					{
						if (pLoopCity->getTeam() == GET_PLAYER(eBestPlayer).getTeam() || GET_TEAM(GET_PLAYER(eBestPlayer).getTeam()).isVassal(pLoopCity->getTeam()))
						{
							if (getCulture(pLoopCity->getOwnerINLINE()) > 0)
							{
								if (isWithinCultureRange(pLoopCity->getOwnerINLINE()))
								{
									iPriority = GC.getCityPlotPriority()[iI];

									if (pLoopCity->getTeam() == GET_PLAYER(eBestPlayer).getTeam())
									{
										iPriority += 5; // priority ranges from 0 to 4 -> give priority to Masters of a Vassal
									}

									if ((iPriority < iBestPriority) || ((iPriority == iBestPriority) && (pLoopCity->getOwnerINLINE() == eBestPlayer)))
									{
										iBestPriority = iPriority;
										pBestCity = pLoopCity;
									}
								}
							}
						}
					}
				}
			}

			if (pBestCity != NULL)
			{
				eBestPlayer = pBestCity->getOwnerINLINE();
			}
		}
	}

	if (eBestPlayer == NO_PLAYER)
	{
		bValid = true;

		for (iI = 0; iI < NUM_CARDINALDIRECTION_TYPES; ++iI)
		{
			pLoopPlot = plotCardinalDirection(getX_INLINE(), getY_INLINE(), ((CardinalDirectionTypes)iI));

			if (pLoopPlot != NULL)
			{
				if (pLoopPlot->isOwned())
				{
					if (eBestPlayer == NO_PLAYER)
					{
						eBestPlayer = pLoopPlot->getOwnerINLINE();
					}
					else if (eBestPlayer != pLoopPlot->getOwnerINLINE())
					{
						bValid = false;
						break;
					}
				}
				else
				{
					bValid = false;
					break;
				}
			}
		}

		if (!bValid
			// Super Forts begin *culture*
			|| (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && !GET_PLAYER(eBestPlayer).isAlive()))
			// Super Forts end
		{
			eBestPlayer = NO_PLAYER;
		}
	}

	return eBestPlayer;
}


void CvPlot::plotAction(PlotUnitFunc func, int iData1, int iData2, PlayerTypes eOwner, TeamTypes eTeam)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((eOwner == NO_PLAYER) || (pLoopUnit->getOwnerINLINE() == eOwner))
		{
			if ((eTeam == NO_TEAM) || (pLoopUnit->getTeam() == eTeam))
			{
				func(pLoopUnit, iData1, iData2);
			}
		}
	}
}


int CvPlot::plotCount(ConstPlotUnitFunc funcA, int iData1A, int iData2A, PlayerTypes eOwner, TeamTypes eTeam, ConstPlotUnitFunc funcB, int iData1B, int iData2B) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	int iCount;

	iCount = 0;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((eOwner == NO_PLAYER) || (pLoopUnit->getOwnerINLINE() == eOwner))
		{
			if ((eTeam == NO_TEAM) || (pLoopUnit->getTeam() == eTeam))
			{
				if ((funcA == NULL) || funcA(pLoopUnit, iData1A, iData2A))
				{
					if ((funcB == NULL) || funcB(pLoopUnit, iData1B, iData2B))
					{
						iCount++;
					}
				}
			}
		}
	}

	return iCount;
}

CvUnit* CvPlot::plotCheck(ConstPlotUnitFunc funcA, int iData1A, int iData2A, PlayerTypes eOwner, TeamTypes eTeam, ConstPlotUnitFunc funcB, int iData1B, int iData2B) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((eOwner == NO_PLAYER) || (pLoopUnit->getOwnerINLINE() == eOwner))
		{
			if ((eTeam == NO_TEAM) || (pLoopUnit->getTeam() == eTeam))
			{
				if (funcA(pLoopUnit, iData1A, iData2A))
				{
					if ((funcB == NULL) || funcB(pLoopUnit, iData1B, iData2B))
					{
						return pLoopUnit;
					}
				}
			}
		}
	}
	return NULL;
}


bool CvPlot::isOwned() const
{
	return (getOwnerINLINE() != NO_PLAYER);
}


bool CvPlot::isBarbarian() const
{
/*************************************************************************************************/
/**	ADDON (multibarb) merged Sephi		12/23/08									Xienwolf	**/
/**	adjusted for more barb teams                    											**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	return (getOwnerINLINE() == BARBARIAN_PLAYER);
/**								----  End Original Code  ----									**/
	return (getOwnerINLINE() >= BARBARIAN_PLAYER);
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
}


bool CvPlot::isRevealedBarbarian() const
{
/*************************************************************************************************/
/**	ADDON (MultiBarb) merged Sephi		12/23/08									Xienwolf	**/
/**	adjusted for more barb teams                    											**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
	return (getRevealedOwner(GC.getGameINLINE().getActiveTeam(), true) == BARBARIAN_PLAYER);
/**								----  End Original Code  ----									**/
    return (getRevealedOwner(GC.getGameINLINE().getActiveTeam(), true) >= BARBARIAN_PLAYER);
/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/
}


bool CvPlot::isVisible(TeamTypes eTeam, bool bDebug) const
{
	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return true;
	}
	else
	{
		if(isNonPlanar()) {
			return false;
		}

		if (eTeam == NO_TEAM)
		{
			return false;
		}

		return ((getVisibilityCount(eTeam) > 0) || (getStolenVisibilityCount(eTeam) > 0));
	}
}


bool CvPlot::isActiveVisible(bool bDebug) const
{
	return isVisible(GC.getGameINLINE().getActiveTeam(), bDebug);
}


bool CvPlot::isVisibleToCivTeam() const
{
	int iI;

	for (iI = 0; iI < MAX_CIV_TEAMS; ++iI)
	{
		if (GET_TEAM((TeamTypes)iI).isAlive())
		{
			if (isVisible(((TeamTypes)iI), false))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isVisibleToWatchingHuman() const
{
	int iI;

	for (iI = 0; iI < MAX_CIV_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).isHuman())
			{
				if (isVisible(GET_PLAYER((PlayerTypes)iI).getTeam(), false))
				{
					return true;
				}
			}
		}
	}

	return false;
}


bool CvPlot::isAdjacentVisible(TeamTypes eTeam, bool bDebug) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isVisible(eTeam, bDebug))
			{
				return true;
			}
		}
	}

	return false;
}

bool CvPlot::isAdjacentNonvisible(TeamTypes eTeam) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isVisible(eTeam, false))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isGoody(TeamTypes eTeam) const
{
	if ((eTeam != NO_TEAM) && GET_TEAM(eTeam).isBarbarian())
	{
		return false;
	}

	return ((getImprovementType() == NO_IMPROVEMENT) ? false : GC.getImprovementInfo(getImprovementType()).isGoody());
}


bool CvPlot::isRevealedGoody(TeamTypes eTeam) const
{
	if (eTeam == NO_TEAM)
	{
		return isGoody();
	}

	if (GET_TEAM(eTeam).isBarbarian())
	{
		return false;
	}

//FfH: Added by Kael 08/27/2007

    if (isUnit())
    {
		CLLNode<IDInfo>* pUnitNode = headUnitNode();
		CvUnit* pLoopUnit;
		while (pUnitNode != NULL)
		{
			pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);
			if (GC.getUnitInfo((UnitTypes)pLoopUnit->getUnitType()).getEquipmentPromotion() != NO_PROMOTION && atWar(eTeam, pLoopUnit->getTeam()))
			{
			    return true;
			}
		}
	}
//FfH: End Add

	return ((getRevealedImprovementType(eTeam, false) == NO_IMPROVEMENT) ? false : GC.getImprovementInfo(getRevealedImprovementType(eTeam, false)).isGoody());
}


void CvPlot::removeGoody()
{
	setImprovementType(NO_IMPROVEMENT);
}


bool CvPlot::isCity(bool bCheckImprovement, TeamTypes eForTeam) const
{
	if (bCheckImprovement && NO_IMPROVEMENT != getImprovementType())
	{
		if (GC.getImprovementInfo(getImprovementType()).isActsAsCity())
		{
			if (NO_TEAM == eForTeam || (NO_TEAM == getTeam() && GC.getImprovementInfo(getImprovementType()).isOutsideBorders()) || GET_TEAM(eForTeam).isFriendlyTerritory(getTeam()))
			{
				return true;
			}
		}
		//SpyFanatic: super forts are friendly cities
		if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(getImprovementType()).getCulture() > 0)
		{
			if (NO_TEAM == eForTeam || (NO_TEAM == getTeam() && GC.getImprovementInfo(getImprovementType()).isOutsideBorders()) || GET_TEAM(eForTeam).isFriendlyTerritory(getTeam()))
			{
				return true;
			}
		}
	}

	return (getPlotCity() != NULL);
}


bool CvPlot::isFriendlyCity(const CvUnit& kUnit, bool bCheckImprovement) const
{
	if (!isCity(bCheckImprovement, kUnit.getTeam()))
	{
		return false;
	}

	if (isVisibleEnemyUnit(&kUnit))
	{
		return false;
	}

	TeamTypes ePlotTeam = getTeam();

	if (NO_TEAM != ePlotTeam)
	{
		if (kUnit.isEnemy(ePlotTeam))
		{
			return false;
		}

		TeamTypes eTeam = GET_PLAYER(kUnit.getCombatOwner(ePlotTeam, this)).getTeam();

		if (eTeam == ePlotTeam)
		{
			return true;
		}

		if (GET_TEAM(eTeam).isOpenBorders(ePlotTeam))
		{
			return true;
		}

		if (GET_TEAM(ePlotTeam).isVassal(eTeam))
		{
			return true;
		}
	}

	return false;
}


bool CvPlot::isEnemyCity(const CvUnit& kUnit) const
{
	// Super Forts begin *culture*
	if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
	{
		TeamTypes ePlotTeam = getTeam();
		if (isCity(true) && (ePlotTeam != NO_TEAM))
		{
			return kUnit.isEnemy(ePlotTeam, this);
		}
		return false;
	}
	CvCity* pCity = getPlotCity();

	if (pCity != NULL)
	{
		return kUnit.isEnemy(pCity->getTeam(), this);
	}

	return false;
}


bool CvPlot::isOccupation() const
{
	CvCity* pCity;

	pCity = getPlotCity();

	if (pCity != NULL)
	{
		return pCity->isOccupation();
	}

	return false;
}


bool CvPlot::isBeingWorked() const
{
	CvCity* pWorkingCity;

	pWorkingCity = getWorkingCity();

	if (pWorkingCity != NULL)
	{
		return pWorkingCity->isWorkingPlot(this);
	}

	return false;
}


bool CvPlot::isUnit() const
{
	return (getNumUnits() > 0);
}


bool CvPlot::isInvestigate(TeamTypes eTeam) const
{
	return (plotCheck(PUF_isInvestigate, -1, -1, NO_PLAYER, eTeam) != NULL);
}


bool CvPlot::isVisibleEnemyDefender(const CvUnit* pUnit) const
{
	return (plotCheck(PUF_canDefendEnemy, pUnit->getOwnerINLINE(), pUnit->isAlwaysHostile(this), NO_PLAYER, NO_TEAM, PUF_isVisible, pUnit->getOwnerINLINE()) != NULL);
}


CvUnit *CvPlot::getVisibleEnemyDefender(PlayerTypes ePlayer) const
{
	return plotCheck(PUF_canDefendEnemy, ePlayer, false, NO_PLAYER, NO_TEAM, PUF_isVisible, ePlayer);
}


int CvPlot::getNumDefenders(PlayerTypes ePlayer) const
{
	return plotCount(PUF_canDefend, -1, -1, ePlayer);
}


int CvPlot::getNumVisibleEnemyDefenders(const CvUnit* pUnit) const
{
	return plotCount(PUF_canDefendEnemy, pUnit->getOwnerINLINE(), pUnit->isAlwaysHostile(this), NO_PLAYER, NO_TEAM, PUF_isVisible, pUnit->getOwnerINLINE());
}


int CvPlot::getNumVisiblePotentialEnemyDefenders(const CvUnit* pUnit) const
{
	return plotCount(PUF_canDefendPotentialEnemy, pUnit->getOwnerINLINE(), pUnit->isAlwaysHostile(this), NO_PLAYER, NO_TEAM, PUF_isVisible, pUnit->getOwnerINLINE());
}


bool CvPlot::isVisibleEnemyUnit(PlayerTypes ePlayer) const
{
	return (plotCheck(PUF_isEnemy, ePlayer, false, NO_PLAYER, NO_TEAM, PUF_isVisible, ePlayer) != NULL);
}

bool CvPlot::isVisibleEnemyUnitNoBarbarian(PlayerTypes ePlayer) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (pLoopUnit->getOwnerINLINE() < BARBARIAN_PLAYER)
		{
			if (pLoopUnit->getTeam() < BARBARIAN_TEAM)
			{
				if (PUF_isEnemy(pLoopUnit, ePlayer, false))
				{
					if (PUF_isVisible(pLoopUnit, ePlayer, -1))
					{
						return pLoopUnit;
					}
				}
			}
		}
	}
	return NULL;
}

int CvPlot::getNumVisibleUnits(PlayerTypes ePlayer) const
{
	return plotCount(PUF_isVisibleDebug, ePlayer);
}


bool CvPlot::isVisibleEnemyUnit(const CvUnit* pUnit) const
{
	return (plotCheck(PUF_isEnemy, pUnit->getOwnerINLINE(), pUnit->isAlwaysHostile(this), NO_PLAYER, NO_TEAM, PUF_isVisible, pUnit->getOwnerINLINE()) != NULL);
}

bool CvPlot::isVisibleOtherUnit(PlayerTypes ePlayer) const
{
	return (plotCheck(PUF_isOtherTeam, ePlayer, -1, NO_PLAYER, NO_TEAM, PUF_isVisible, ePlayer) != NULL);
}


bool CvPlot::isFighting() const
{
	return (plotCheck(PUF_isFighting) != NULL);
}


bool CvPlot::canHaveFeature(FeatureTypes eFeature) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	if (eFeature == NO_FEATURE)
	{
		return true;
	}

	if (getFeatureType() != NO_FEATURE)
	{
		return false;
	}

	if (isPeak())
	{
		return false;
	}

	if (isCity())
	{
		return false;
	}

	if (!(GC.getFeatureInfo(eFeature).isTerrain(getTerrainType())))
	{
		return false;
	}

	if (GC.getFeatureInfo(eFeature).isNoCoast() && isCoastalLand())
	{
		return false;
	}

	if (GC.getFeatureInfo(eFeature).isNoRiver() && isRiver())
	{
		return false;
	}

	if (GC.getFeatureInfo(eFeature).isRequiresFlatlands() && isHills())
	{
		return false;
	}

	if (GC.getFeatureInfo(eFeature).isNoAdjacent())
	{
		for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if (pAdjacentPlot->getFeatureType() == eFeature)
				{
					return false;
				}
			}
		}
	}

	if (GC.getFeatureInfo(eFeature).isRequiresRiver() && !isRiver())
	{
		return false;
	}

	if(isStartingPlot() && GC.getFeatureInfo(eFeature).isNoCity()) {
		return false;
	}

	//added Sephi - Planes
	/**
	if(!GC.getGameINLINE().isOption(GAMEOPTION_NO_PLANES)) {
		if(getPlane() != NO_PLANE) {
			if(GC.getFeatureInfo(eFeature).getFeatureClass() != NO_FEATURECLASS) {
				if(GC.getPlaneInfo(getPlane()).getNativeFeatureType(GC.getFeatureInfo(eFeature).getFeatureClass()) != eFeature) {
					return false;
				}
			}
		}
	}
	**/

	return true;
}


bool CvPlot::isRoute() const
{
	return (getRouteType() != NO_ROUTE);
}


bool CvPlot::isValidRoute(const CvUnit* pUnit) const
{
	if (isRoute())
	{
		if (!pUnit->isEnemy(getTeam(), this) || pUnit->isEnemyRoute())
		{
			return true;
		}
	}

	return false;
}


bool CvPlot::isTradeNetworkImpassable(TeamTypes eTeam) const
{
	return (isImpassable() && !isRiverNetwork(eTeam));
}

bool CvPlot::isRiverNetwork(TeamTypes eTeam) const
{
	if (!isRiver())
	{
		return false;
	}

	if (GET_TEAM(eTeam).isRiverTrade())
	{
		return true;
	}

	if (getTeam() == eTeam)
	{
		return true;
	}

	return false;
}

bool CvPlot::isNetworkTerrain(TeamTypes eTeam) const
{
	FAssertMsg(eTeam != NO_TEAM, "eTeam is not assigned a valid value");
	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	if (GET_TEAM(eTeam).isTerrainTrade(getTerrainType()))
	{
		return true;
	}

	// CivilizationInfo :: isAlwaysConnectResources tag
	if (isOwned() && getTeam() == eTeam 
		&& GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).isAlwaysConnectResources())
	{
		return true;
	}

	if (isWater())
	{
		if (getTeam() == eTeam)
		{
			return true;
		}
	}

	return false;
}


bool CvPlot::isBonusNetwork(TeamTypes eTeam) const
{
	if (isRoute())
	{
		return true;
	}

	if (isRiverNetwork(eTeam))
	{
		return true;
	}

	if (isNetworkTerrain(eTeam))
	{
		return true;
	}

	return false;
}


bool CvPlot::isTradeNetwork(TeamTypes eTeam) const
{
	FAssertMsg(eTeam != NO_TEAM, "eTeam is not assigned a valid value");

	if (atWar(eTeam, getTeam()))
	{
		return false;
	}

	if (getBlockadedCount(eTeam) > 0)
	{
		return false;
	}

	if (isTradeNetworkImpassable(eTeam))
	{
		return false;
	}

	if (!isOwned())
	{
		if (!isRevealed(eTeam, false))
		{
			return false;
		}
	}

	return isBonusNetwork(eTeam);
}


bool CvPlot::isTradeNetworkConnected(const CvPlot* pPlot, TeamTypes eTeam) const
{
	FAssertMsg(eTeam != NO_TEAM, "eTeam is not assigned a valid value");

	if (atWar(eTeam, getTeam()) || atWar(eTeam, pPlot->getTeam()))
	{
		return false;
	}

	if (isTradeNetworkImpassable(eTeam) || pPlot->isTradeNetworkImpassable(eTeam))
	{
		return false;
	}

	if (!isOwned())
	{
		if (!isRevealed(eTeam, false) || !(pPlot->isRevealed(eTeam, false)))
		{
			return false;
		}
	}

	if (isRoute())
	{
		if (pPlot->isRoute())
		{
			return true;
		}
	}

	if (isCity(true, eTeam))
	{
		if (pPlot->isNetworkTerrain(eTeam))
		{
			return true;
		}
	}

	if (isNetworkTerrain(eTeam))
	{
		if (pPlot->isCity(true, eTeam))
		{
			return true;
		}

		if (pPlot->isNetworkTerrain(eTeam))
		{
			return true;
		}

		if (pPlot->isRiverNetwork(eTeam))
		{
			if (pPlot->isRiverConnection(directionXY(pPlot, this)))
			{
				return true;
			}
		}
	}

	if (isRiverNetwork(eTeam))
	{
		if (pPlot->isNetworkTerrain(eTeam))
		{
			if (isRiverConnection(directionXY(this, pPlot)))
			{
				return true;
			}
		}

		if (isRiverConnection(directionXY(this, pPlot)) || pPlot->isRiverConnection(directionXY(pPlot, this)))
		{
			if (pPlot->isRiverNetwork(eTeam))
			{
				return true;
			}
		}
	}

	return false;
}


bool CvPlot::isValidDomainForLocation(const CvUnit& unit) const
{
	if (isValidDomainForAction(unit))
	{
		return true;
	}

	return isCity(true, unit.getTeam());
}


bool CvPlot::isValidDomainForAction(const CvUnit& unit) const
{
	switch (unit.getDomainType())
	{
	case DOMAIN_SEA:
		return (isWater() || unit.canMoveAllTerrain());
		break;

	case DOMAIN_AIR:
		return false;
		break;

	case DOMAIN_LAND:
	case DOMAIN_IMMOBILE:
		return (!isWater() || unit.canMoveAllTerrain());
		break;

	default:
		FAssert(false);
		break;
	}

	return false;
}


bool CvPlot::isImpassable() const
{
	if (isPeak())
	{
		return true;
	}

	if (getTerrainType() == NO_TERRAIN)
	{
		return false;
	}

	return ((getFeatureType() == NO_FEATURE) ? GC.getTerrainInfo(getTerrainType()).isImpassable() : GC.getFeatureInfo(getFeatureType()).isImpassable());
}


int CvPlot::getX() const
{
	return m_iX;
}


int CvPlot::getY() const
{
	return m_iY;
}


bool CvPlot::at(int iX, int iY) const
{
	return ((getX_INLINE() == iX) && (getY_INLINE() == iY));
}


int CvPlot::getLatitude() const
{
/************************************************************************************************/
/* UNOFFICIAL_PATCH                       07/12/09                       Temudjin & jdog5000    */
/*                                                                                              */
/* Bugfix                                                                                       */
/************************************************************************************************/
/* orginal bts code
	int iLatitude;

	if (GC.getMapINLINE().isWrapXINLINE() || !(GC.getMapINLINE().isWrapYINLINE()))
	{
		iLatitude = ((getY_INLINE() * 100) / GC.getMapINLINE().getGridHeightINLINE());
	}
	else
	{
		iLatitude = ((getX_INLINE() * 100) / GC.getMapINLINE().getGridWidthINLINE());
	}

	iLatitude = ((iLatitude * (GC.getMapINLINE().getTopLatitude() - GC.getMapINLINE().getBottomLatitude())) / 100);

	return abs(iLatitude + GC.getMapINLINE().getBottomLatitude());
*/
	int iLatitude;
	double fLatitude;

	if (GC.getMapINLINE().isWrapXINLINE() || !(GC.getMapINLINE().isWrapYINLINE()))
	{
		fLatitude = ((getY_INLINE() * 1.0) / (GC.getMapINLINE().getGridHeightINLINE()-1));
	}
	else
	{
		fLatitude = ((getX_INLINE() * 1.0) / (GC.getMapINLINE().getGridWidthINLINE()-1));
	}

	fLatitude = fLatitude * (GC.getMapINLINE().getTopLatitude() - GC.getMapINLINE().getBottomLatitude());

	iLatitude =(int)(fLatitude + 0.5);
	return abs( (iLatitude + GC.getMapINLINE().getBottomLatitude()));
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
}


int CvPlot::getFOWIndex() const
{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	CvViewport& pCurrentViewport = GC.getCurrentViewport();

	return ((((pCurrentViewport.getGridHeight() - 1) - pCurrentViewport.getViewportYFromMapY(getY_INLINE())) * pCurrentViewport.getGridWidth() * LANDSCAPE_FOW_RESOLUTION * LANDSCAPE_FOW_RESOLUTION) + (pCurrentViewport.getViewportXFromMapX(getX_INLINE()) * LANDSCAPE_FOW_RESOLUTION));
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	return ((((GC.getMapINLINE().getGridHeight() - 1) - getY_INLINE()) * GC.getMapINLINE().getGridWidth() * LANDSCAPE_FOW_RESOLUTION * LANDSCAPE_FOW_RESOLUTION) + (getX_INLINE() * LANDSCAPE_FOW_RESOLUTION));
}


CvArea* CvPlot::area() const
{
	if(m_pPlotArea == NULL)
	{
		m_pPlotArea = GC.getMapINLINE().getArea(getArea());
	}

	return m_pPlotArea;
}


CvArea* CvPlot::waterArea() const
{
	CvArea* pBestArea;
	CvPlot* pAdjacentPlot;
	int iValue;
	int iBestValue;
	int iI;

	if (isWater())
	{
		return area();
	}

	iBestValue = 0;
	pBestArea = NULL;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isWater())
			{
				iValue = pAdjacentPlot->area()->getNumTiles();

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestArea = pAdjacentPlot->area();
				}
			}
		}
	}

	return pBestArea;
}

CvArea* CvPlot::secondWaterArea() const
{

	CvArea* pWaterArea = waterArea();
	CvArea* pBestArea;
	CvPlot* pAdjacentPlot;
	int iValue;
	int iBestValue;
	int iI;

	FAssert(!isWater());

	iBestValue = 0;
	pBestArea = NULL;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isWater() && (pAdjacentPlot->getArea() != pWaterArea->getID()))
			{
				iValue = pAdjacentPlot->area()->getNumTiles();

				if (iValue > iBestValue)
				{
					iBestValue = iValue;
					pBestArea = pAdjacentPlot->area();
				}
			}
		}
	}

	return pBestArea;

}


int CvPlot::getArea() const
{
	return m_iArea;
}


void CvPlot::setArea(int iNewValue)
{
	bool bOldLake;

	if (getArea() != iNewValue)
	{
		bOldLake = isLake();
/*
		oosLog("AreaPower"
			,"Turn: %d,Player:%d,X:%d,Y:%d,FromArea:%d,ToAfter:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getOwnerINLINE()
			,getX_INLINE()
			,getY_INLINE()
			,m_iArea
			,iNewValue
		);
*/
		if (area() != NULL)
		{
			processArea(area(), -1);
		}

		m_iArea = iNewValue;
		m_pPlotArea = NULL;

		if (area() != NULL)
		{
			processArea(area(), 1);

			updateIrrigated();
			updateYield();
		}
	}
}


int CvPlot::getFeatureVariety() const
{
	FAssert((getFeatureType() == NO_FEATURE) || (m_iFeatureVariety < GC.getFeatureInfo(getFeatureType()).getArtInfo()->getNumVarieties()));
	FAssert(m_iFeatureVariety >= 0);
	return m_iFeatureVariety;
}


int CvPlot::getOwnershipDuration() const
{
	return m_iOwnershipDuration;
}


bool CvPlot::isOwnershipScore() const
{
	return (getOwnershipDuration() >= GC.getDefineINT("OWNERSHIP_SCORE_DURATION_THRESHOLD"));
}


void CvPlot::setOwnershipDuration(int iNewValue)
{
	bool bOldOwnershipScore;

	if (getOwnershipDuration() != iNewValue)
	{
		bOldOwnershipScore = isOwnershipScore();

		m_iOwnershipDuration = iNewValue;
		FAssert(getOwnershipDuration() >= 0);

		if (bOldOwnershipScore != isOwnershipScore())
		{
			if (isOwned())
			{
				if (!isWater())
				{
					GET_PLAYER(getOwnerINLINE()).changeTotalLandScored((isOwnershipScore()) ? 1 : -1);
				}
			}
		}
	}
}


void CvPlot::changeOwnershipDuration(int iChange)
{
	setOwnershipDuration(getOwnershipDuration() + iChange);
}


int CvPlot::getImprovementDuration() const
{
	return m_iImprovementDuration;
}


void CvPlot::setImprovementDuration(int iNewValue)
{
	m_iImprovementDuration = iNewValue;
	FAssert(getImprovementDuration() >= 0);
}


void CvPlot::changeImprovementDuration(int iChange)
{
	setImprovementDuration(getImprovementDuration() + iChange);
}


int CvPlot::getUpgradeProgress() const
{
	return m_iUpgradeProgress;
}


int CvPlot::getUpgradeTimeLeft(ImprovementTypes eImprovement, PlayerTypes ePlayer) const
{
	int iUpgradeLeft;
	int iUpgradeRate;
	int iTurnsLeft;

	iUpgradeLeft = (GC.getGameINLINE().getImprovementUpgradeTime(eImprovement) - ((getImprovementType() == eImprovement) ? getUpgradeProgress() : 0));

	if (ePlayer == NO_PLAYER)
	{
		return iUpgradeLeft;
	}
	//SpyFanatic: what about improvement that does not have an upgrade or the upgrade is not for your civilization
	if(GC.getImprovementInfo(eImprovement).getImprovementUpgrade() == NO_IMPROVEMENT)
	{
		return 0;
	}
	ImprovementTypes eImprovementUpgrade = (ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade();
	if(GC.getImprovementInfo(eImprovementUpgrade).getPrereqCivilization() != NO_CIVILIZATION && GC.getImprovementInfo(eImprovementUpgrade).getPrereqCivilization() != GET_PLAYER(ePlayer).getCivilizationType())
	{
		return 0;
	}
	//SpyFanatic end

	iUpgradeRate = calculateImprovementUpgradeRate(eImprovement, ePlayer);
		
	if (iUpgradeRate == 0)
	{
		return iUpgradeLeft;
	}

	iTurnsLeft = (iUpgradeLeft / iUpgradeRate);

	if ((iTurnsLeft * iUpgradeRate) < iUpgradeLeft)
	{
		iTurnsLeft++;
	}

	return std::max(1, iTurnsLeft);
}

int CvPlot::calculateImprovementUpgradeRate(ImprovementTypes eImprovement, PlayerTypes ePlayer) const
{
	int iUpgradeRate;
	bool iImprovement = false;
	bool iConnection = false;
	bool iBonus = false;

	if (ePlayer == NO_PLAYER)
		return 1;

	iUpgradeRate = 1;
	for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
	{
		CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
		if (pAdjacentPlot != NULL && !pAdjacentPlot->isWater()) {
			if (pAdjacentPlot->getTeam() == GET_PLAYER(ePlayer).getTeam()) {
				bool bonusConnectingImprovement = false;
				for (int i = 0; i < GC.getNumBonusInfos(); i++)
				{
					if (GC.getImprovementInfo(eImprovement).getImprovementBonusDiscoverRand(i) > 0) {
						bonusConnectingImprovement = true;
						iImprovement = false;	//Don't want camps and the like to improve faster because there is another improvement nearby.
						if (isTradeNetworkConnected(this, GET_PLAYER(ePlayer).getTeam()) && getRouteType() != NO_ROUTE) 
							iConnection = true;	//Connecting the improvement to the trade network and building a road will help it improve faster.
						if (getBonusType() != NO_BONUS && getBonusType() == i)
							iBonus = true;		//only improve faster if built on a resource that the improvement can connect.
					}
				}

				if (pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT && GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify() == 0 &&\
					GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).isPermanent() == false && !bonusConnectingImprovement) {
					iImprovement = true;
					if (isTradeNetworkConnected(this, GET_PLAYER(ePlayer).getTeam()) && getRouteType() != NO_ROUTE && pAdjacentPlot->getRouteType() != NO_ROUTE) {
						iConnection = true;
						if (pAdjacentPlot->getBonusType() != NO_BONUS) {
							iBonus = true;
							break;
						}
					}
				}
			}
		}
	}

	//An improvement that can discover resources increases the growth of cottages by 1
	if (iImprovement &&
		GC.getGameINLINE().getGameSpeedType() != (GameSpeedTypes)GC.getInfoTypeForString("GAMESPEED_QUICK") &&
		GC.getGameINLINE().getGameSpeedType() != (GameSpeedTypes)GC.getInfoTypeForString("GAMESPEED_NORMAL"))
		iUpgradeRate++;

	// Having a road connection to a town increases town growth by 1.  Malakim automatically connect to roads with the right terrain type so the growth bonus is automatic.
	if (iConnection || (GET_TEAM(GET_PLAYER(ePlayer).getTeam()).isTerrainTrade(getTerrainType())))
		iUpgradeRate++;

		//A bonus increases opportunities in towns and grants +1 to growth.
	if (iBonus &&
		GC.getGameINLINE().getGameSpeedType() != (GameSpeedTypes)GC.getInfoTypeForString("GAMESPEED_QUICK") && 
		GC.getGameINLINE().getGameSpeedType() != (GameSpeedTypes)GC.getInfoTypeForString("GAMESPEED_NORMAL"))
		iUpgradeRate++;

	iUpgradeRate *= GET_PLAYER(ePlayer).getImprovementUpgradeRate();
	
	return iUpgradeRate;
}

void CvPlot::setUpgradeProgress(int iNewValue)
{
	m_iUpgradeProgress = iNewValue;
	FAssert(getUpgradeProgress() >= 0);
}


void CvPlot::changeUpgradeProgress(int iChange)
{
	setUpgradeProgress(getUpgradeProgress() + iChange);
}


int CvPlot::getForceUnownedTimer() const
{
	return m_iForceUnownedTimer;
}


bool CvPlot::isForceUnowned() const
{
	return (getForceUnownedTimer() > 0);
}


void CvPlot::setForceUnownedTimer(int iNewValue)
{
	m_iForceUnownedTimer = iNewValue;
	FAssert(getForceUnownedTimer() >= 0);
}


void CvPlot::changeForceUnownedTimer(int iChange)
{
	setForceUnownedTimer(getForceUnownedTimer() + iChange);
}


int CvPlot::getCityRadiusCount() const
{
	return m_iCityRadiusCount;
}


int CvPlot::isCityRadius() const
{
	return (getCityRadiusCount() > 0);
}


void CvPlot::changeCityRadiusCount(int iChange)
{
	m_iCityRadiusCount = (m_iCityRadiusCount + iChange);
	FAssert(getCityRadiusCount() >= 0);
}


bool CvPlot::isStartingPlot() const
{
	return m_bStartingPlot;
}


void CvPlot::setStartingPlot(bool bNewValue)
{
	m_bStartingPlot = bNewValue;
}


bool CvPlot::isNOfRiver() const
{
	return m_bNOfRiver;
}


void CvPlot::setNOfRiver(bool bNewValue, CardinalDirectionTypes eRiverDir)
{
	CvPlot* pAdjacentPlot;
	int iI;

	if ((isNOfRiver() != bNewValue) || (eRiverDir != m_eRiverWEDirection))
	{
		if (isNOfRiver() != bNewValue)
		{
			updatePlotGroupBonus(false);
			m_bNOfRiver = bNewValue;
			updatePlotGroupBonus(true);

			updateRiverCrossing();
			updateYield();

			for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

				if (pAdjacentPlot != NULL)
				{
					pAdjacentPlot->updateRiverCrossing();
					pAdjacentPlot->updateYield();
				}
			}

			if (area() != NULL)
			{
				area()->changeNumRiverEdges((isNOfRiver()) ? 1 : -1);
			}
		}

		FAssertMsg(eRiverDir == CARDINALDIRECTION_WEST || eRiverDir == CARDINALDIRECTION_EAST || eRiverDir == NO_CARDINALDIRECTION, "invalid parameter");
		m_eRiverWEDirection = eRiverDir;

		updateRiverSymbol(true, true);
	}
}


bool CvPlot::isWOfRiver() const
{
	return m_bWOfRiver;
}


void CvPlot::setWOfRiver(bool bNewValue, CardinalDirectionTypes eRiverDir)
{
	CvPlot* pAdjacentPlot;
	int iI;

	if ((isWOfRiver() != bNewValue) || (eRiverDir != m_eRiverNSDirection))
	{
		if (isWOfRiver() != bNewValue)
		{
			updatePlotGroupBonus(false);
			m_bWOfRiver = bNewValue;
			updatePlotGroupBonus(true);

			updateRiverCrossing();
			updateYield();

			for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

				if (pAdjacentPlot != NULL)
				{
					pAdjacentPlot->updateRiverCrossing();
					pAdjacentPlot->updateYield();
				}
			}

			if (area())
			{
				area()->changeNumRiverEdges((isWOfRiver()) ? 1 : -1);
			}
		}

		FAssertMsg(eRiverDir == CARDINALDIRECTION_NORTH || eRiverDir == CARDINALDIRECTION_SOUTH || eRiverDir == NO_CARDINALDIRECTION, "invalid parameter");
		m_eRiverNSDirection = eRiverDir;

		updateRiverSymbol(true, true);
	}
}


CardinalDirectionTypes CvPlot::getRiverNSDirection() const
{
	return (CardinalDirectionTypes)m_eRiverNSDirection;
}


CardinalDirectionTypes CvPlot::getRiverWEDirection() const
{
	return (CardinalDirectionTypes)m_eRiverWEDirection;
}


// This function finds an *inland* corner of this plot at which to place a river.
// It then returns the plot with that corner at its SE.

CvPlot* CvPlot::getInlandCorner() const
{
	CvPlot* pRiverPlot = NULL; // will be a plot through whose SE corner we want the river to run
	int aiShuffle[4];

	shuffleArray(aiShuffle, 4, GC.getGameINLINE().getMapRand());

	for (int iI = 0; iI < 4; ++iI)
	{
		switch (aiShuffle[iI])
		{
		case 0:
			pRiverPlot = GC.getMapINLINE().plotSorenINLINE(getX_INLINE(), getY_INLINE()); break;
		case 1:
			pRiverPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_NORTH); break;
		case 2:
			pRiverPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_NORTHWEST); break;
		case 3:
			pRiverPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_WEST); break;
		}
		if (pRiverPlot != NULL && !pRiverPlot->hasCoastAtSECorner())
		{
			break;
		}
		else
		{
			pRiverPlot = NULL;
		}
	}

	return pRiverPlot;
}


bool CvPlot::hasCoastAtSECorner() const
{
	CvPlot* pAdjacentPlot;

	if (isWater())
	{
		return true;
	}

	pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_EAST);
	if (pAdjacentPlot != NULL && pAdjacentPlot->isWater())
	{
		return true;
	}

	pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_SOUTHEAST);
	if (pAdjacentPlot != NULL && pAdjacentPlot->isWater())
	{
		return true;
	}

	pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_SOUTH);
	if (pAdjacentPlot != NULL && pAdjacentPlot->isWater())
	{
		return true;
	}

	return false;
}


bool CvPlot::isIrrigated() const
{
	return m_bIrrigated;
}


void CvPlot::setIrrigated(bool bNewValue)
{
	CvPlot* pLoopPlot;
	int iDX, iDY;

	if (isIrrigated() != bNewValue)
	{
		m_bIrrigated = bNewValue;

		for (iDX = -1; iDX <= 1; iDX++)
		{
			for (iDY = -1; iDY <= 1; iDY++)
			{
				pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

				if (pLoopPlot != NULL)
				{
					pLoopPlot->updateYield();
					pLoopPlot->setLayoutDirty(true);
				}
			}
		}
	}
}


void CvPlot::updateIrrigated()
{
	PROFILE("CvPlot::updateIrrigated()");

	CvPlot* pLoopPlot;
	FAStar* pIrrigatedFinder;
	bool bFoundFreshWater;
	bool bIrrigated;
	int iI;

	if (area() == NULL)
	{
		return;
	}

	if (!(GC.getGameINLINE().isFinalInitialized()))
	{
		return;
	}

	pIrrigatedFinder = gDLL->getFAStarIFace()->create();

	if (isIrrigated())
	{
		if (!isPotentialIrrigation())
		{
			setIrrigated(false);

			for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

				if (pLoopPlot != NULL)
				{
					bFoundFreshWater = false;
					gDLL->getFAStarIFace()->Initialize(pIrrigatedFinder, GC.getMapINLINE().getGridWidthINLINE(), GC.getMapINLINE().getGridHeightINLINE(), GC.getMapINLINE().isWrapXINLINE(), GC.getMapINLINE().isWrapYINLINE(), NULL, NULL, NULL, potentialIrrigation, NULL, checkFreshWater, &bFoundFreshWater);
					gDLL->getFAStarIFace()->GeneratePath(pIrrigatedFinder, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), -1, -1);

					if (!bFoundFreshWater)
					{
						bIrrigated = false;
						gDLL->getFAStarIFace()->Initialize(pIrrigatedFinder, GC.getMapINLINE().getGridWidthINLINE(), GC.getMapINLINE().getGridHeightINLINE(), GC.getMapINLINE().isWrapXINLINE(), GC.getMapINLINE().isWrapYINLINE(), NULL, NULL, NULL, potentialIrrigation, NULL, changeIrrigated, &bIrrigated);
						gDLL->getFAStarIFace()->GeneratePath(pIrrigatedFinder, pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), -1, -1);
					}
				}
			}
		}
	}
	else
	{
		if (isPotentialIrrigation() && isIrrigationAvailable(true))
		{
			bIrrigated = true;
			gDLL->getFAStarIFace()->Initialize(pIrrigatedFinder, GC.getMapINLINE().getGridWidthINLINE(), GC.getMapINLINE().getGridHeightINLINE(), GC.getMapINLINE().isWrapXINLINE(), GC.getMapINLINE().isWrapYINLINE(), NULL, NULL, NULL, potentialIrrigation, NULL, changeIrrigated, &bIrrigated);
			gDLL->getFAStarIFace()->GeneratePath(pIrrigatedFinder, getX_INLINE(), getY_INLINE(), -1, -1);
		}
	}

	gDLL->getFAStarIFace()->destroy(pIrrigatedFinder);
}


bool CvPlot::isPotentialCityWork() const
{
	return m_bPotentialCityWork;
}


bool CvPlot::isPotentialCityWorkForArea(CvArea* pArea) const
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iI;

	for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
	{
		pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if (!(pLoopPlot->isWater()) || GC.getDefineINT("WATER_POTENTIAL_CITY_WORK_FOR_AREA"))
			{
				if (pLoopPlot->area() == pArea)
				{
					return true;
				}
			}
		}
	}

	return false;
}


void CvPlot::updatePotentialCityWork()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	bool bValid;
	int iI;

	bValid = false;

//FfH: Modified by Kael 07/12/208
//  for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
	for (iI = 0; iI < 21; ++iI)
//FfH: End Modify

	{
		pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			if (!(pLoopPlot->isWater()))
			{
				bValid = true;
				break;
			}
		}
	}

	if (isPotentialCityWork() != bValid)
	{
		m_bPotentialCityWork = bValid;

		updateYield();
	}
}


bool CvPlot::isShowCitySymbols() const
{
	return m_bShowCitySymbols;
}


void CvPlot::updateShowCitySymbols()
{
	CvCity* pLoopCity;
	CvPlot* pLoopPlot;
	bool bNewShowCitySymbols;
	int iI;

	bNewShowCitySymbols = false;

	for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
	{
		pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

		if (pLoopPlot != NULL)
		{
			pLoopCity = pLoopPlot->getPlotCity();

			if (pLoopCity != NULL)
			{
				if (pLoopCity->isCitySelected() && gDLL->getInterfaceIFace()->isCityScreenUp())
				{
					if (pLoopCity->canWork(this))
					{
						bNewShowCitySymbols = true;
						break;
					}
				}
			}
		}
	}

	if (isShowCitySymbols() != bNewShowCitySymbols)
	{
		m_bShowCitySymbols = bNewShowCitySymbols;

		updateSymbolDisplay();
		updateSymbolVisibility();
	}
}


bool CvPlot::isFlagDirty() const
{
	return m_bFlagDirty;
}


void CvPlot::setFlagDirty(bool bNewValue)
{
	m_bFlagDirty = bNewValue;
}


PlayerTypes CvPlot::getOwner() const
{
	return getOwnerINLINE();
}


void CvPlot::setOwner(PlayerTypes eNewValue, bool bCheckUnits, bool bUpdatePlotGroup)
{
	PROFILE_FUNC();

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pOldCity;
	CvCity* pNewCity;
	CvUnit* pLoopUnit;
	CvWString szBuffer;
	UnitTypes eBestUnit;
	int iFreeUnits;
	int iI;

	if (getOwnerINLINE() != eNewValue)
	{
		GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_PLOT_OWNER_CHANGE, eNewValue, (char*)NULL, getX_INLINE(), getY_INLINE());

		pOldCity = getPlotCity();
/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
		if (GC.getGameINLINE().isFinalInitialized() && getTeam() != NO_TEAM)
		{
            for (iI = 0; iI < MAX_TEAMS; iI++)
            {
                if (GET_TEAM(getTeam()).isAtWar((TeamTypes)iI))
                {
                    for (int iDX = -DANGER_RANGE; iDX <= DANGER_RANGE; iDX++)
                    {
                        for (int iDY = -DANGER_RANGE; iDY <= DANGER_RANGE; iDY++)
                        {
                            int iIndex = GC.getMapINLINE().plotNumINLINE(getX_INLINE() + iDX, getY_INLINE() + iDY);
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
		if (pOldCity != NULL)
		{
			szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_REVOLTED_JOINED", pOldCity->getNameKey(), GET_PLAYER(eNewValue).getCivilizationDescriptionKey());
			gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTUREFLIP", MESSAGE_TYPE_MAJOR_EVENT,  ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
			gDLL->getInterfaceIFace()->addMessage(eNewValue, false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTUREFLIP", MESSAGE_TYPE_MAJOR_EVENT,  ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);

			szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_REVOLTS_JOINS", pOldCity->getNameKey(), GET_PLAYER(eNewValue).getCivilizationDescriptionKey());
			GC.getGameINLINE().addReplayMessage(REPLAY_MESSAGE_MAJOR_EVENT, getOwnerINLINE(), szBuffer, getX_INLINE(), getY_INLINE(), (ColorTypes)GC.getInfoTypeForString("COLOR_ALT_HIGHLIGHT_TEXT"));

			FAssertMsg(pOldCity->getOwnerINLINE() != eNewValue, "pOldCity->getOwnerINLINE() is not expected to be equal with eNewValue");
			GET_PLAYER(eNewValue).acquireCity(pOldCity, false, false, bUpdatePlotGroup); // will delete the pointer
			pOldCity = NULL;
			pNewCity = getPlotCity();
			FAssertMsg(pNewCity != NULL, "NewCity is not assigned a valid value");

			if (pNewCity != NULL)
			{
				CLinkList<IDInfo> oldUnits;

				pUnitNode = headUnitNode();

				while (pUnitNode != NULL)
				{
					oldUnits.insertAtEnd(pUnitNode->m_data);
					pUnitNode = nextUnitNode(pUnitNode);
				}

				pUnitNode = oldUnits.head();

				while (pUnitNode != NULL)
				{
					pLoopUnit = ::getUnit(pUnitNode->m_data);
					pUnitNode = oldUnits.next(pUnitNode);

					if (pLoopUnit)
					{
						if (pLoopUnit->isEnemy(GET_PLAYER(eNewValue).getTeam(), this))
						{
							FAssert(pLoopUnit->getTeam() != GET_PLAYER(eNewValue).getTeam());
							pLoopUnit->kill(false, eNewValue);
						}
					}
				}

//FfH: Modified by Kael 01/23/2009
//				eBestUnit = pNewCity->AI_bestUnitAI(UNITAI_CITY_DEFENSE);
//				if (eBestUnit == NO_UNIT)
//				{
//					eBestUnit = pNewCity->AI_bestUnitAI(UNITAI_ATTACK);
//				}
				eBestUnit = pNewCity->getConscriptUnit();
//FfH: End Modify

				if (eBestUnit != NO_UNIT)
				{
					iFreeUnits = (GC.getDefineINT("BASE_REVOLT_FREE_UNITS") + ((pNewCity->getHighestPopulation() * GC.getDefineINT("REVOLT_FREE_UNITS_PERCENT")) / 100));

					for (iI = 0; iI < iFreeUnits; ++iI)
					{
						GET_PLAYER(eNewValue).initUnit(eBestUnit, getX_INLINE(), getY_INLINE(), UNITAI_CITY_DEFENSE);
					}
				}
			}
		}
		else
		{
			setOwnershipDuration(0);

			if (isOwned())
			{
				//chance to Destroy Improvements when taking over Lands
				if(getImprovementType() != NO_IMPROVEMENT
					&& GC.getImprovementInfo(getImprovementType()).getGoldCost() > 0
					&& !GC.getImprovementInfo(getImprovementType()).isUnique()) {
					if(eNewValue != NO_PLAYER && !GET_PLAYER(eNewValue).isBarbarian()
						&& !isBarbarian())
					{				
						if(GC.getGame().getSorenRandNum(100, "Chance to scorch earth imps") < 30) {
							ImprovementTypes iSmoke = (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_SMOKE");					
							setImprovementType(iSmoke);
						}
					}
				}

				changeAdjacentSight(getTeam(), GC.getDefineINT("PLOT_VISIBILITY_RANGE"), false, NULL, bUpdatePlotGroup);

				if (area())
				{
					area()->changeNumOwnedTiles(-1);
				}
				GC.getMapINLINE().changeOwnedPlots(-1);

				if (!isWater())
				{
					GET_PLAYER(getOwnerINLINE()).changeTotalLand(-1);
					GET_TEAM(getTeam()).changeTotalLand(-1);

					if (isOwnershipScore())
					{
						GET_PLAYER(getOwnerINLINE()).changeTotalLandScored(-1);
					}
				}

				if (getImprovementType() != NO_IMPROVEMENT)
				{
					GET_PLAYER(getOwnerINLINE()).changeImprovementCount(getImprovementType(), -1);
					// Super Forts begin *culture*
//oosLog("CultureFort","Plot %d %d Improvement:%S culture:%d setOwner",getX_INLINE(),getY_INLINE(),GC.getImprovementInfo(getImprovementType()).getDescription(),GC.getImprovementInfo(getImprovementType()).getCulture());
					if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(getImprovementType()).getCulture() > 0 /*.isActsAsCity() && getOwnerINLINE() < BARBARIAN_PLAYER*/)
					{
						changeCultureRangeFortsWithinRange(getOwnerINLINE(), -1, GC.getImprovementInfo(getImprovementType()).getCultureRange(), false);
					}
					// Super Forts end
				}

				updatePlotGroupBonus(false);
			}

			pUnitNode = headUnitNode();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);

				if (pLoopUnit->getTeam() != getTeam() && (getTeam() == NO_TEAM || !GET_TEAM(getTeam()).isVassal(pLoopUnit->getTeam())))
				{

//FfH: Modified by Kael 04/19/2009
//					GET_PLAYER(pLoopUnit->getOwnerINLINE()).changeNumOutsideUnits(-1);
                    if (pLoopUnit->getDuration() == 0 && pLoopUnit->getUnitInfo().isCitySupportNeeded())
                    {
						/*if(isOOSLogging())
						{
							oosLog("OutSideUnit"
								,"Turn:%d,Player:%d,Unit:%d,Name:%S,X:%d,Y:%d,numoutsideunits:%d,setOwner:-1"
								,GC.getGameINLINE().getElapsedGameTurns()
								,pLoopUnit->getOwnerINLINE()
								,pLoopUnit->getID()
								,pLoopUnit->getName().GetCString()
								,getX_INLINE()
								,getY_INLINE()
								,GET_PLAYER(pLoopUnit->getOwnerINLINE()).getNumOutsideUnits()
							);
						}*/
                        GET_PLAYER(pLoopUnit->getOwnerINLINE()).changeNumOutsideUnits(-1);
                    }
//FfH: End Modify

				}

				if (pLoopUnit->isBlockading())
				{
					pLoopUnit->setBlockading(false);
					pLoopUnit->getGroup()->clearMissionQueue();
					pLoopUnit->getGroup()->setActivityType(ACTIVITY_AWAKE);
				}
			}

			m_eOwner = eNewValue;

			setWorkingCityOverride(NULL);
			updateWorkingCity();

			if (isOwned())
			{
				changeAdjacentSight(getTeam(), GC.getDefineINT("PLOT_VISIBILITY_RANGE"), true, NULL, bUpdatePlotGroup);

				if (area())
				{
					area()->changeNumOwnedTiles(1);
				}
				GC.getMapINLINE().changeOwnedPlots(1);

				if (!isWater())
				{
					GET_PLAYER(getOwnerINLINE()).changeTotalLand(1);
					GET_TEAM(getTeam()).changeTotalLand(1);

					if (isOwnershipScore())
					{
						GET_PLAYER(getOwnerINLINE()).changeTotalLandScored(1);
					}
				}

				if (getImprovementType() != NO_IMPROVEMENT)
				{
					GET_PLAYER(getOwnerINLINE()).changeImprovementCount(getImprovementType(), 1);
			// Super Forts begin *culture*

			if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(getImprovementType()).getCulture() > 0/*.isActsAsCity()*/)
			{
//oosLog("CultureFort","Plot %d %d Improvement:%S culture:%d setOwner",getX_INLINE(),getY_INLINE(),GC.getImprovementInfo(getImprovementType()).getDescription(),GC.getImprovementInfo(getImprovementType()).getCulture());
				changeCultureRangeFortsWithinRange(getOwnerINLINE(), 1, GC.getImprovementInfo(getImprovementType()).getCultureRange(), true);
			}
			// Super Forts end
				}

				updatePlotGroupBonus(true);
			}

			pUnitNode = headUnitNode();

			while (pUnitNode != NULL)
			{
				pLoopUnit = ::getUnit(pUnitNode->m_data);
				pUnitNode = nextUnitNode(pUnitNode);

				if (pLoopUnit->getTeam() != getTeam() && (getTeam() == NO_TEAM || !GET_TEAM(getTeam()).isVassal(pLoopUnit->getTeam())))
				{

//FfH: Modified by Kael 04/19/2009
//					GET_PLAYER(pLoopUnit->getOwnerINLINE()).changeNumOutsideUnits(1);
                    if (pLoopUnit->getDuration() == 0 && pLoopUnit->getUnitInfo().isCitySupportNeeded())
                    {
						/*if(isOOSLogging())
						{
							oosLog("OutSideUnit"
								,"Turn:%d,Player:%d,Unit:%d,Name:%S,X:%d,Y:%d,numoutsideunits:%d,setOwner:1"
								,GC.getGameINLINE().getElapsedGameTurns()
								,pLoopUnit->getOwnerINLINE()
								,pLoopUnit->getID()
								,pLoopUnit->getName().GetCString()
								,getX_INLINE()
								,getY_INLINE()
								,GET_PLAYER(pLoopUnit->getOwnerINLINE()).getNumOutsideUnits()
							);
						}*/
                        GET_PLAYER(pLoopUnit->getOwnerINLINE()).changeNumOutsideUnits(1);
                    }
//FfH: End Modify

				}
			}

			for (iI = 0; iI < MAX_TEAMS; ++iI)
			{
				if (GET_TEAM((TeamTypes)iI).isAlive())
				{
					updateRevealedOwner((TeamTypes)iI);
				}
			}

			updateIrrigated();
			updateYield();

			if (bUpdatePlotGroup)
			{
				updatePlotGroup();
			}

			if (bCheckUnits)
			{
				verifyUnitValidPlot();
			}

			if (isOwned())
			{
				if(((GET_PLAYER(getOwnerINLINE()).getStateReligion() == RELIGION_FELLOWSHIP_OF_LEAVES) && ((FeatureTypes)GC.getInfoTypeForString("FEATURE_FOREST_ANCIENT") == getFeatureType())
					|| ((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_LOST_TEMPLE") == getImprovementType())))
				{
					GET_PLAYER(getOwnerINLINE()).updateFaithIncome();
				}

				if ((getImprovementType() == (ImprovementTypes)GC.getIMPROVEMENT_MANA_SHRINE())
					|| (getBonusType(getTeam()) != NO_BONUS))
				{
					GET_PLAYER(getOwnerINLINE()).updateManaIncome();
				}
			}	

			if (isOwned())
			{
				if (isGoody())
				{
					GET_PLAYER(getOwnerINLINE()).doGoody(this, NULL);
				}

				for (iI = 0; iI < MAX_CIV_TEAMS; ++iI)
				{
					if (GET_TEAM((TeamTypes)iI).isAlive())
					{
						if (isVisible((TeamTypes)iI, false))
						{
							GET_TEAM((TeamTypes)iI).meet(getTeam(), true);
						}
					}
				}
			}

			if (GC.getGameINLINE().isDebugMode())
			{
				updateMinimapColor();

				gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);

				gDLL->getEngineIFace()->SetDirty(CultureBorders_DIRTY_BIT, true);
			}
		}

/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
		if (GC.getGameINLINE().isFinalInitialized() && getTeam() != NO_TEAM)
		{
            for (iI = 0; iI < MAX_TEAMS; iI++)
            {
                if (GET_TEAM(getTeam()).isAtWar((TeamTypes)iI))
                {
                    for (int iDX = -DANGER_RANGE; iDX <= DANGER_RANGE; iDX++)
                    {
                        for (int iDY = -DANGER_RANGE; iDY <= DANGER_RANGE; iDY++)
                        {
                            int iIndex = GC.getMapINLINE().plotNumINLINE(getX_INLINE() + iDX, getY_INLINE() + iDY);
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
		updateSymbols();
	}
}


PlotTypes CvPlot::getPlotType() const
{
	return (PlotTypes)m_ePlotType;
}


bool CvPlot::isWater() const
{
	return (getPlotType() == PLOT_OCEAN);
}


bool CvPlot::isFlatlands() const
{
	return (getPlotType() == PLOT_LAND);
}


bool CvPlot::isHills() const
{
	return (getPlotType() == PLOT_HILLS);
}


bool CvPlot::isPeak() const
{
	return (getPlotType() == PLOT_PEAK);
}


void CvPlot::setPlotType(PlotTypes eNewValue, bool bRecalculate, bool bRebuildGraphics)
{
	CvArea* pNewArea;
	CvArea* pCurrArea;
	CvArea* pLastArea;
	CvPlot* pLoopPlot;
	bool bWasWater;
	bool bRecalculateAreas;
	int iAreaCount;
	int iI;

	if (getPlotType() != eNewValue)
	{
		if ((getPlotType() == PLOT_OCEAN) || (eNewValue == PLOT_OCEAN))
		{
			erase();
		}

		bWasWater = isWater();

		updateSeeFromSight(false, true);

		m_ePlotType = eNewValue;

		updateYield();
		updatePlotGroup();

		updateSeeFromSight(true, true);

		if ((getTerrainType() == NO_TERRAIN) || (GC.getTerrainInfo(getTerrainType()).isWater() != isWater()))
		{
			if (isWater())
			{
				if (isAdjacentToLand())
				{
					setTerrainType(((TerrainTypes)(GC.getDefineINT("SHALLOW_WATER_TERRAIN"))), bRecalculate, bRebuildGraphics);
				}
				else
				{
					setTerrainType(((TerrainTypes)(GC.getDefineINT("DEEP_WATER_TERRAIN"))), bRecalculate, bRebuildGraphics);
				}
			}
			else
			{
				TerrainTypes eNewTerrain = (TerrainTypes)GC.getDefineINT("LAND_TERRAIN");
				if(getPlane() != NO_PLANE) {
					eNewTerrain = (TerrainTypes)GC.getPlaneInfo(getPlane()).getNativeTerrainType(GC.getTerrainInfo(eNewTerrain).getTerrainClassType());
				}
				setTerrainType(eNewTerrain, bRecalculate, bRebuildGraphics);
			}
		}

		GC.getMapINLINE().resetPathDistance();

		if (bWasWater != isWater())
		{
			if (bRecalculate)
			{
				for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
				{
					pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

					if (pLoopPlot != NULL)
					{
						if (pLoopPlot->isWater())
						{
							if (pLoopPlot->isAdjacentToLand())
							{
								pLoopPlot->setTerrainType(((TerrainTypes)(GC.getDefineINT("SHALLOW_WATER_TERRAIN"))), bRecalculate, bRebuildGraphics);
							}
							else
							{
								pLoopPlot->setTerrainType(((TerrainTypes)(GC.getDefineINT("DEEP_WATER_TERRAIN"))), bRecalculate, bRebuildGraphics);
							}
						}
					}
				}
			}

			for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

				if (pLoopPlot != NULL)
				{
					pLoopPlot->updateYield();
					pLoopPlot->updatePlotGroup();
				}
			}

			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					pLoopPlot->updatePotentialCityWork();
				}
			}

			GC.getMapINLINE().changeLandPlots((isWater()) ? -1 : 1);

			if (getBonusType() != NO_BONUS)
			{
				GC.getMapINLINE().changeNumBonusesOnLand(getBonusType(), ((isWater()) ? -1 : 1));
			}

			if (isOwned())
			{
				GET_PLAYER(getOwnerINLINE()).changeTotalLand((isWater()) ? -1 : 1);
				GET_TEAM(getTeam()).changeTotalLand((isWater()) ? -1 : 1);
			}

			if (bRecalculate)
			{
				pNewArea = NULL;
				bRecalculateAreas = false;

				// XXX might want to change this if we allow diagonal water movement...
				if (isWater())
				{
					for (iI = 0; iI < NUM_CARDINALDIRECTION_TYPES; ++iI)
					{
						pLoopPlot = plotCardinalDirection(getX_INLINE(), getY_INLINE(), ((CardinalDirectionTypes)iI));

						if (pLoopPlot != NULL)
						{
							if (pLoopPlot->area()->isWater())
							{
								if (pNewArea == NULL)
								{
									pNewArea = pLoopPlot->area();
								}
								else if (pNewArea != pLoopPlot->area())
								{
									bRecalculateAreas = true;
									break;
								}
							}
						}
					}
				}
				else
				{
					for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
					{
						pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

						if (pLoopPlot != NULL)
						{
							if (!(pLoopPlot->area()->isWater()))
							{
								if (pNewArea == NULL)
								{
									pNewArea = pLoopPlot->area();
								}
								else if (pNewArea != pLoopPlot->area())
								{
									bRecalculateAreas = true;
									break;
								}
							}
						}
					}
				}

				if (!bRecalculateAreas)
				{
					pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)(NUM_DIRECTION_TYPES - 1)));

					if (pLoopPlot != NULL)
					{
						pLastArea = pLoopPlot->area();
					}
					else
					{
						pLastArea = NULL;
					}

					iAreaCount = 0;

					for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
					{
						pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

						if (pLoopPlot != NULL)
						{
							pCurrArea = pLoopPlot->area();
						}
						else
						{
							pCurrArea = NULL;
						}

						if (pCurrArea != pLastArea)
						{
							iAreaCount++;
						}

						pLastArea = pCurrArea;
					}

					if (iAreaCount > 2)
					{
						bRecalculateAreas = true;
					}
				}

				if (bRecalculateAreas)
				{
					GC.getMapINLINE().recalculateAreas();
				}
				else
				{
					setArea(FFreeList::INVALID_INDEX);

					if ((area() != NULL) && (area()->getNumTiles() == 1))
					{
						GC.getMapINLINE().deleteArea(getArea());
					}

					if (pNewArea == NULL)
					{
						pNewArea = GC.getMapINLINE().addArea();
						pNewArea->init(pNewArea->getID(), isWater());
					}

					setArea(pNewArea->getID());
				}
			}
		}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//		if (bRebuildGraphics && GC.IsGraphicsInitialized())
		if (bRebuildGraphics && shouldHaveGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		{
			//Update terrain graphical
			gDLL->getEngineIFace()->RebuildPlot(getX_INLINE(), getY_INLINE(), true, true);
			//gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true); //minimap does a partial update
			//gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);

			updateFeatureSymbol();
			setLayoutDirty(true);
			updateRouteSymbol(false, true);
			updateRiverSymbol(false, true);
		}
	}
}


TerrainTypes CvPlot::getTerrainType() const
{
	return (TerrainTypes)m_eTerrainType;
}

//
//	Rewritten For Planes
//

void CvPlot::setTerrainType(TerrainTypes eNewValue, bool bRecalculate, bool bRebuildGraphics)
{
	bool bUpdateSight;

	TerrainTypes eNewTerrain=eNewValue;
	/** Code Forced Terrain to Match Plane
	PlaneTypes ePlane=(PlaneTypes)getPlane();
	if(ePlane==NO_PLANE)
	{
		ePlane=(PlaneTypes)0;
	}

	if(eNewTerrain!=NO_TERRAIN)
	{
		if(GC.getTerrainInfo(eNewTerrain).getTerrainClassType()!=NO_TERRAINCLASS)
		{
			eNewTerrain=(TerrainTypes)GC.getPlaneInfo(ePlane).getNativeTerrainType(GC.getTerrainInfo(eNewTerrain).getTerrainClassType());
			if(eNewTerrain==NO_TERRAIN)
				eNewTerrain=eNewValue;
		}
	}
	**/
	if (getTerrainType() != eNewTerrain)
	{
		if ((getTerrainType() != NO_TERRAIN) &&
			  (eNewTerrain != NO_TERRAIN) &&
			  ((GC.getTerrainInfo(getTerrainType()).getSeeFromLevel() != GC.getTerrainInfo(eNewTerrain).getSeeFromLevel()) ||
				 (GC.getTerrainInfo(getTerrainType()).getSeeThroughLevel() != GC.getTerrainInfo(eNewTerrain).getSeeThroughLevel())))
		{
			bUpdateSight = true;
		}
		else
		{
			bUpdateSight = false;
		}

		if (bUpdateSight)
		{
			updateSeeFromSight(false, true);
		}

		m_eTerrainType = eNewTerrain;

		updateYield();
		updatePlotGroup();

		if (bUpdateSight)
		{
			updateSeeFromSight(true, true);
		}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//		if (bRebuildGraphics && GC.IsGraphicsInitialized())
		if (bRebuildGraphics && shouldHaveGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		{
			//Update terrain graphics
			gDLL->getEngineIFace()->RebuildPlot(getX_INLINE(), getY_INLINE(),false,true);
			//gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true); //minimap does a partial update
			//gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
		}

		if (GC.getTerrainInfo(getTerrainType()).isWater() != isWater())
		{
			setPlotType(((GC.getTerrainInfo(getTerrainType()).isWater()) ? PLOT_OCEAN : PLOT_LAND), bRecalculate, bRebuildGraphics);
		}
	}
}

FeatureTypes CvPlot::getFeatureType() const
{
	return (FeatureTypes)m_eFeatureType;
}


void CvPlot::setFeatureType(FeatureTypes eNewValue, int iVariety)
{
	CvCity* pLoopCity;
	CvPlot* pLoopPlot;
	FeatureTypes eOldFeature;
	bool bUpdateSight;
	int iI;

	eOldFeature = getFeatureType();

/*************************************************************************************************/
/**	ADDON (Adventures) Sephi                                                      				**/
/** tag bSpreadFeature                                                                              **/
/*************************************************************************************************/
    if(eOldFeature!=eNewValue && isOwned())
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
                        if(GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).isSpreadFeature()
                            && GC.getAdventureStepInfo((AdventureStepTypes)kAdv.getAdventureStep(iJ)).getFeatureType()==eNewValue)
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

	if (eNewValue != NO_FEATURE)
	{
		if (iVariety == -1)
		{
			iVariety = ((GC.getFeatureInfo(eNewValue).getArtInfo()->getNumVarieties() * ((getLatitude() * 9) / 8)) / 90);
		}

		iVariety = range(iVariety, 0, (GC.getFeatureInfo(eNewValue).getArtInfo()->getNumVarieties() - 1));
	}
	else
	{
		iVariety = 0;
	}

	if ((eOldFeature != eNewValue) || (m_iFeatureVariety != iVariety))
	{
		if ((eOldFeature == NO_FEATURE) ||
			  (eNewValue == NO_FEATURE) ||
			  (GC.getFeatureInfo(eOldFeature).getSeeThroughChange() != GC.getFeatureInfo(eNewValue).getSeeThroughChange()))
		{
			bUpdateSight = true;
		}
		else
		{
			bUpdateSight = false;
		}

		if (bUpdateSight)
		{
			updateSeeFromSight(false, true);
		}

		m_eFeatureType = eNewValue;
		m_iFeatureVariety = iVariety;

		updateYield();

		if (bUpdateSight)
		{
			updateSeeFromSight(true, true);
		}

		updateFeatureSymbol();

		if (((eOldFeature != NO_FEATURE) && (GC.getFeatureInfo(eOldFeature).getArtInfo()->isRiverArt())) ||
			  ((getFeatureType() != NO_FEATURE) && (GC.getFeatureInfo(getFeatureType()).getArtInfo()->isRiverArt())))
		{
			updateRiverSymbolArt(true);
		}

		for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				pLoopCity = pLoopPlot->getPlotCity();

				if (pLoopCity != NULL)
				{
					pLoopCity->updateFeatureHealth();
					pLoopCity->updateFeatureHappiness();
				}
			}
		}

		if (getFeatureType() == NO_FEATURE)
		{
			if (getImprovementType() != NO_IMPROVEMENT)
			{
				if (GC.getImprovementInfo(getImprovementType()).isRequiresFeature())
				{
					setImprovementType(NO_IMPROVEMENT);
				}
			}
		}
/*************************************************************************************************/
/**	ADDON (New Improvement System) Sephi                                 						**/
/*************************************************************************************************/
		bool bMustUpdateHealth=false;
		if((eOldFeature!=NO_FEATURE && GC.getFeatureInfo(eOldFeature).isAddsFreshWater())
			|| (eNewValue!=NO_FEATURE && GC.getFeatureInfo(eNewValue).isAddsFreshWater()))
		{
			bMustUpdateHealth=true;
			updateIrrigated();
			if(getPlotCity()!=NULL)
			{
				getPlotCity()->updateFreshWaterHealth();
			}
		}

		for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				pAdjacentPlot->updateYield();

				if(bMustUpdateHealth)
				{
					pAdjacentPlot->updateIrrigated();
					if(pAdjacentPlot->getPlotCity()!=NULL)
					{
						pAdjacentPlot->getPlotCity()->updateFreshWaterHealth();
					}
				}
			}
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	}
}

void CvPlot::setFeatureDummyVisibility(const char *dummyTag, bool show)
{
	FAssertMsg(m_pFeatureSymbol != NULL, "[Jason] No feature symbol.");
	if(m_pFeatureSymbol != NULL)
	{
		gDLL->getFeatureIFace()->setDummyVisibility(m_pFeatureSymbol, dummyTag, show);
	}
}

void CvPlot::addFeatureDummyModel(const char *dummyTag, const char *modelTag)
{
	FAssertMsg(m_pFeatureSymbol != NULL, "[Jason] No feature symbol.");
	if(m_pFeatureSymbol != NULL)
	{
		gDLL->getFeatureIFace()->addDummyModel(m_pFeatureSymbol, dummyTag, modelTag);
	}
}

void CvPlot::setFeatureDummyTexture(const char *dummyTag, const char *textureTag)
{
	FAssertMsg(m_pFeatureSymbol != NULL, "[Jason] No feature symbol.");
	if(m_pFeatureSymbol != NULL)
	{
		gDLL->getFeatureIFace()->setDummyTexture(m_pFeatureSymbol, dummyTag, textureTag);
	}
}

CvString CvPlot::pickFeatureDummyTag(int mouseX, int mouseY)
{
	FAssertMsg(m_pFeatureSymbol != NULL, "[Jason] No feature symbol.");
	if(m_pFeatureSymbol != NULL)
	{
		return gDLL->getFeatureIFace()->pickDummyTag(m_pFeatureSymbol, mouseX, mouseY);
	}

	return NULL;
}

void CvPlot::resetFeatureModel()
{
	FAssertMsg(m_pFeatureSymbol != NULL, "[Jason] No feature symbol.");
	if(m_pFeatureSymbol != NULL)
	{
		gDLL->getFeatureIFace()->resetModel(m_pFeatureSymbol);
	}
}

BonusTypes CvPlot::getBonusType(TeamTypes eTeam) const
{
	if (eTeam != NO_TEAM)
	{
		if (m_eBonusType != NO_BONUS)
		{
			if (!GET_TEAM(eTeam).isHasTech((TechTypes)(GC.getBonusInfo((BonusTypes)m_eBonusType).getTechReveal())) && !GET_TEAM(eTeam).isForceRevealedBonus((BonusTypes)m_eBonusType))
			{
				return NO_BONUS;
			}
		}
	}

	return (BonusTypes)m_eBonusType;
}


BonusTypes CvPlot::getNonObsoleteBonusType(TeamTypes eTeam) const
{
	FAssert(eTeam != NO_TEAM);

	BonusTypes eBonus = getBonusType(eTeam);
	if (eBonus != NO_BONUS)
	{
		if (GET_TEAM(eTeam).isBonusObsolete(eBonus))
		{
			return NO_BONUS;
		}
	}

	return eBonus;
}


void CvPlot::setBonusType(BonusTypes eNewValue)
{
	if (getBonusType() != eNewValue)
	{
		if (getBonusType() != NO_BONUS)
		{
			if (area())
			{
				area()->changeNumBonuses(getBonusType(), -1);
			}
			GC.getMapINLINE().changeNumBonuses(getBonusType(), -1);

			if (!isWater())
			{
				GC.getMapINLINE().changeNumBonusesOnLand(getBonusType(), -1);
			}
		}

		updatePlotGroupBonus(false);
		m_eBonusType = eNewValue;
		updatePlotGroupBonus(true);

		if (getBonusType() != NO_BONUS)
		{
			if (area())
			{
				area()->changeNumBonuses(getBonusType(), 1);
			}
			GC.getMapINLINE().changeNumBonuses(getBonusType(), 1);

			if (!isWater())
			{
				GC.getMapINLINE().changeNumBonusesOnLand(getBonusType(), 1);
			}
		}

		updateYield();

		setLayoutDirty(true);

		gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);
	}
}


ImprovementTypes CvPlot::getImprovementType() const
{
	return (ImprovementTypes)m_eImprovementType;
}


void CvPlot::setImprovementType(ImprovementTypes eNewValue, bool bWorker)
{
	int iI;
	ImprovementTypes eOldImprovement = getImprovementType();

	if (getImprovementType() != eNewValue)
	{
		if (getImprovementType() != NO_IMPROVEMENT)
		{
			if (area())
			{
				area()->changeNumImprovements(getImprovementType(), -1);
			}
			if (isOwned())
			{
				GET_PLAYER(getOwnerINLINE()).changeImprovementCount(getImprovementType(), -1);
				// Super Forts begin *culture*
//oosLog("CultureFort","Plot %d %d Improvement:%S culture:%d setImprovementType",getX_INLINE(),getY_INLINE(),GC.getImprovementInfo(getImprovementType()).getDescription(),GC.getImprovementInfo(getImprovementType()).getCulture());
				if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(getImprovementType()).getCulture() > 0 /*.isActsAsCity()*/)
				{
					changeCultureRangeFortsWithinRange(getOwnerINLINE(), -1, GC.getImprovementInfo(getImprovementType()).getCultureRange(), true);
				}
				// Super Forts end
			}
		}
		// Super Forts begin *vision*
		if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
		{
			updateSight(false, true);
		}
		// Super Forts end

		updatePlotGroupBonus(false);
		m_eImprovementType = eNewValue;
		updatePlotGroupBonus(true);

		if (getImprovementType() == NO_IMPROVEMENT)
		{
			setImprovementDuration(0);
		}

		setUpgradeProgress(0);

		for (iI = 0; iI < MAX_TEAMS; ++iI)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (isVisible((TeamTypes)iI, false))
				{
					setRevealedImprovementType((TeamTypes)iI, getImprovementType());
				}
			}
		}

		if (getImprovementType() != NO_IMPROVEMENT)
		{
			if (area())
			{
				area()->changeNumImprovements(getImprovementType(), 1);
			}

			if (isOwned())
			{
				GET_PLAYER(getOwnerINLINE()).changeImprovementCount(getImprovementType(), 1);
				// Super Forts begin *culture*
//oosLog("CultureFort","Plot %d %d Improvement:%S culture:%d setImprovementType",getX_INLINE(),getY_INLINE(),GC.getImprovementInfo(getImprovementType()).getDescription(),GC.getImprovementInfo(getImprovementType()).getCulture());
				if (GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) && GC.getImprovementInfo(getImprovementType()).getCulture() > 0 /*.isActsAsCity()*/)
				{
					changeCultureRangeFortsWithinRange(getOwnerINLINE(), 1, GC.getImprovementInfo(getImprovementType()).getCultureRange(), true);
				}
				// Super Forts end
			}
		}
		// Super Forts begin *vision*
		if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS))
		{
			updateSight(true, true);
		}
		// Super Forts end

		updateIrrigated();
		updateYield();

		for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
		{
			CvPlot* pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				CvCity* pLoopCity = pLoopPlot->getPlotCity();

				if (pLoopCity != NULL)
				{
					pLoopCity->updateFeatureHappiness();
					pLoopCity->updateImprovementHealth(); //added Sephi needed for new tag
				}
			}
		}

		// Building or removing a fort will now force a plotgroup update to verify resource connections.
		if ( (NO_IMPROVEMENT != getImprovementType() && GC.getImprovementInfo(getImprovementType()).isActsAsCity()) !=
			 (NO_IMPROVEMENT != eOldImprovement && GC.getImprovementInfo(eOldImprovement).isActsAsCity()) )
		{
			updatePlotGroup();
		}

		if (NO_IMPROVEMENT != eOldImprovement && GC.getImprovementInfo(eOldImprovement).isActsAsCity())
		{
			verifyUnitValidPlot();
		}

		if (GC.getGameINLINE().isDebugMode())
		{
			setLayoutDirty(true);
		}

//added Sephi
/*		if(getWorkingCity()!=NULL)
		{
			for(int iI=YIELD_COMMERCE+1;iI<GC.getNUM_YIELD_TYPES();iI++)
			{
				if(getImprovementType() == NO_IMPROVEMENT || GC.getImprovementInfo(getImprovementType()).getYieldChange(iI)>0)
				{
					getWorkingCity()->updateYield();
				}
			}
			
			//update BestplotBuild
			//getWorkingCity()->AI_bestPlotBuild(this);
		}*/
//end Add

		if (getImprovementType() != NO_IMPROVEMENT)
		{

//FfH Improvements: Added by Kael 08/07/2007
            if (GC.getImprovementInfo(getImprovementType()).getBonusConvert() != NO_BONUS)
            {
                setBonusType((BonusTypes)GC.getImprovementInfo(getImprovementType()).getBonusConvert());
            }
//FfH: End Add

			if (GC.getImprovementInfo(getImprovementType()).getSpawnUnitCiv() == GC.getDefineINT("BARBARIAN_CIVILIZATION"))
			{
				if(GC.getImprovementInfo(getImprovementType()).getDungeonType() != NO_DUNGEON)
				{
					setDungeonType(GC.getImprovementInfo(getImprovementType()).getDungeonType());
				}
				else
				{
					setDungeonType(GC.getInfoTypeForString("DUNGEON_REGULAR"));
				}
			}

			CvEventReporter::getInstance().improvementBuilt(getImprovementType(), getX_INLINE(), getY_INLINE(), bWorker);
		}

		if (getImprovementType() == NO_IMPROVEMENT)
		{
			CvEventReporter::getInstance().improvementDestroyed(eOldImprovement, getOwnerINLINE(), getX_INLINE(), getY_INLINE());

			setDungeonType(NO_DUNGEON);
		}

		CvCity* pWorkingCity = getWorkingCity();
		if (NULL != pWorkingCity)
		{
			if ((NO_IMPROVEMENT != eNewValue && pWorkingCity->getImprovementFreeSpecialists(eNewValue) > 0)	||
				(NO_IMPROVEMENT != eOldImprovement && pWorkingCity->getImprovementFreeSpecialists(eOldImprovement) > 0))
			{

				pWorkingCity->AI_setAssignWorkDirty(true);

			}
		}

		gDLL->getInterfaceIFace()->setDirty(CitizenButtons_DIRTY_BIT, true);

/*************************************************************************************************/
/**	ADDON (New Improvement System) Sephi                                 						**/
/*************************************************************************************************/
		//force Recalculation of adjacent plots
		if(eNewValue!=NO_IMPROVEMENT && GC.getImprovementInfo(eNewValue).getImprovementClassType()==0)
		{
			for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
			{
				CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

				if (pAdjacentPlot != NULL)
				{
					if(pAdjacentPlot->getTeam()==getTeam())
					{
						pAdjacentPlot->updateYield();
					}
				}
			}
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
	}
}


RouteTypes CvPlot::getRouteType() const
{
	return (RouteTypes)m_eRouteType;
}


void CvPlot::setRouteType(RouteTypes eNewValue, bool bUpdatePlotGroups)
{
	bool bOldRoute;
	int iI;

	if (getRouteType() != eNewValue)
	{
		bOldRoute = isRoute(); // XXX is this right???

		updatePlotGroupBonus(false);
		m_eRouteType = eNewValue;
		updatePlotGroupBonus(true);

		for (iI = 0; iI < MAX_TEAMS; ++iI)
		{
			if (GET_TEAM((TeamTypes)iI).isAlive())
			{
				if (isVisible((TeamTypes)iI, false))
				{
					setRevealedRouteType((TeamTypes)iI, getRouteType());
				}
			}
		}

		updateYield();

		if (bUpdatePlotGroups)
		{
			if (bOldRoute != isRoute())
			{
				updatePlotGroup();
			}
		}

		if (GC.getGameINLINE().isDebugMode())
		{
			updateRouteSymbol(true, true);
		}
/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
		if (GC.getGameINLINE().isFinalInitialized())
		{
			for (int iDX = -DANGER_RANGE; iDX <= DANGER_RANGE; iDX++)
			{
				for (int iDY = -DANGER_RANGE; iDY <= DANGER_RANGE; iDY++)
				{
					int iIndex = GC.getMapINLINE().plotNumINLINE(getX_INLINE() + iDX, getY_INLINE() + iDY);
					if (iIndex > -1 && iIndex < GC.getMapINLINE().numPlotsINLINE())
					{
						CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(iIndex);
						FAssert (pLoopPlot != NULL);

						int iDistance = stepDistance(getX_INLINE(), getY_INLINE(), getX_INLINE() + iDX, getY_INLINE() + iDY);

						FAssert (iDistance <= DANGER_RANGE);
						for (iI = 0; iI < MAX_PLAYERS; iI++)
						{
							for (int iJ = 0; iJ < iDistance; iJ++)
							{
								pLoopPlot->invalidatePlayerDangerCache((PlayerTypes)iI, iJ);
							}
						}
					}
				}
			}
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
		if (getRouteType() != NO_ROUTE)
		{
			CvEventReporter::getInstance().routeBuilt(getRouteType(), getX_INLINE(), getY_INLINE());
		}
	}
}


void CvPlot::updateCityRoute(bool bUpdatePlotGroup)
{
	RouteTypes eCityRoute;

	if (isCity())
	{
		FAssertMsg(isOwned(), "isOwned is expected to be true");

		eCityRoute = GET_PLAYER(getOwnerINLINE()).getBestRoute();

		if (eCityRoute == NO_ROUTE)
		{
			eCityRoute = ((RouteTypes)(GC.getDefineINT("INITIAL_CITY_ROUTE_TYPE")));
		}

		setRouteType(eCityRoute, bUpdatePlotGroup);
	}
}


CvCity* CvPlot::getPlotCity() const
{
	return getCity(m_plotCity);
}


void CvPlot::setPlotCity(CvCity* pNewValue)
{
	CvPlotGroup* pPlotGroup;
	CvPlot* pLoopPlot;
	int iI;

	if (getPlotCity() != pNewValue)
	{
		if (isCity())
		{
			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					/*
					if(isOOSLogging())
					{
						oosLog("AICityRadius","Turn:%d,Player:%d,CityID:%d,X:%d,Y:%d,iChange:%d,m_iCityRadiusCount:%d"
								,GC.getGameINLINE().getElapsedGameTurns()
								,getPlotCity() != NULL ? getPlotCity()->getOwner() : -1
								,getPlotCity() != NULL ? getPlotCity()->getID() : 0
								,pLoopPlot->getX()
								,pLoopPlot->getY()
								,-1
								,pLoopPlot->getCityRadiusCount()
							);
					}
					*/
					//SpyFanatic: fixed the below issue in CvCity setPlotRadius
					pLoopPlot->changeCityRadiusCount(-1);		//When a Kuriotate second city is founded (at least via AS), the starting culture causes it to be founded with 2 rings, but the m_iCityRadiusCount is only set to 1 so that if/when it is conquered, this gives an error because it sets the value to -1
					pLoopPlot->changePlayerCityRadiusCount(getPlotCity()->getOwnerINLINE(), -1);
				}
			}
		}

		updatePlotGroupBonus(false);
		if (isCity())
		{
			pPlotGroup = getPlotGroup(getOwnerINLINE());

			if (pPlotGroup != NULL)
			{
				FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvPlot::setPlotCity");
				for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
				{
					getPlotCity()->changeNumBonuses(((BonusTypes)iI), -(pPlotGroup->getNumBonuses((BonusTypes)iI)));
				}
			}
		}
		if (pNewValue != NULL)
		{
			m_plotCity = pNewValue->getIDInfo();
		}
		else
		{
			m_plotCity.reset();
		}
		if (isCity())
		{
			pPlotGroup = getPlotGroup(getOwnerINLINE());

			if (pPlotGroup != NULL)
			{
				FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvPlot::setPlotCity");
				for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
				{
					getPlotCity()->changeNumBonuses(((BonusTypes)iI), pPlotGroup->getNumBonuses((BonusTypes)iI));
				}
			}
		}
		updatePlotGroupBonus(true);

		if (isCity())
		{
			for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
			{
				pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

				if (pLoopPlot != NULL)
				{
					/*
					if(isOOSLogging())
					{
						oosLog("AICityRadius","Turn:%d,Player:%d,CityID:%d,X:%d,Y:%d,iChange:%d,m_iCityRadiusCount:%d"
								,GC.getGameINLINE().getElapsedGameTurns()
								,getPlotCity() != NULL ? getPlotCity()->getOwner() : -1
								,getPlotCity() != NULL ? getPlotCity()->getID() : 0
								,pLoopPlot->getX()
								,pLoopPlot->getY()
								,1
								,pLoopPlot->getCityRadiusCount()
							);
					}
					*/

					pLoopPlot->changeCityRadiusCount(1);
					pLoopPlot->changePlayerCityRadiusCount(getPlotCity()->getOwnerINLINE(), 1);
				}
			}
		}

		updateIrrigated();
		updateYield();

		updateMinimapColor();
	}
}


CvCity* CvPlot::getWorkingCity() const
{
	return getCity(m_workingCity);
}


void CvPlot::updateWorkingCity()
{
	CvCity* pOldWorkingCity;
	CvCity* pLoopCity;
	CvCity* pBestCity;
	CvPlot* pLoopPlot;
	int iBestPlot;
	int iI;

	pBestCity = getPlotCity();

	if (pBestCity == NULL)
	{
		pBestCity = getWorkingCityOverride();
		FAssertMsg((pBestCity == NULL) || (pBestCity->getOwnerINLINE() == getOwnerINLINE()), "pBest city is expected to either be NULL or the current plot instance's");
	}

	if ((pBestCity == NULL) && isOwned())
	{
		iBestPlot = 0;

		for (iI = 0; iI < NUM_CITY_PLOTS; ++iI)
		{
			pLoopPlot = plotCity(getX_INLINE(), getY_INLINE(), iI);

			if (pLoopPlot != NULL)
			{
				pLoopCity = pLoopPlot->getPlotCity();

				if (pLoopCity != NULL)
				{
/*************************************************************************************************/
/**	BUGFIX (modified CityRadius) Sephi                                      					**/
/**																								**/
/**	makes sure an AI city only tries to work plots it can actually use 							**/
/*************************************************************************************************/
					if (iI<pLoopCity->getNumCityPlots())
					{
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
						if (pLoopCity->getOwnerINLINE() == getOwnerINLINE())
						{
							// XXX use getGameTurnAcquired() instead???
							if ((pBestCity == NULL) ||
								  (GC.getCityPlotPriority()[iI] < GC.getCityPlotPriority()[iBestPlot]) ||
								  ((GC.getCityPlotPriority()[iI] == GC.getCityPlotPriority()[iBestPlot]) &&
								   ((pLoopCity->getGameTurnFounded() < pBestCity->getGameTurnFounded()) ||
									((pLoopCity->getGameTurnFounded() == pBestCity->getGameTurnFounded()) &&
									 (pLoopCity->getID() < pBestCity->getID())))))
							{
								iBestPlot = iI;
								pBestCity = pLoopCity;
							}
						}
/*************************************************************************************************/
/**	BUGFIX (modified CityRadius) Sephi                                      					**/
/**																								**/
/**	makes sure an AI city only tries to work plots it can actually use 							**/
/*************************************************************************************************/
					}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/
				}
			}
		}
	}

	pOldWorkingCity = getWorkingCity();

	if (pOldWorkingCity != pBestCity)
	{
		if (pOldWorkingCity != NULL)
		{
			//Sephi, added additional check
			// pOldWorkingCity->setWorkingPlot(this, false);
			if(pOldWorkingCity->getCityPlotIndex(this) != -1)
			{
				pOldWorkingCity->setWorkingPlot(this, false);
			}
		}

		if (pBestCity != NULL)
		{
			FAssertMsg(isOwned(), "isOwned is expected to be true");
			FAssertMsg(!isBeingWorked(), "isBeingWorked did not return false as expected");
			m_workingCity = pBestCity->getIDInfo();
		}
		else
		{
			m_workingCity.reset();
		}

		if (pOldWorkingCity != NULL)
		{
			pOldWorkingCity->AI_setAssignWorkDirty(true);
			pOldWorkingCity->updateFeatureHappiness();
		}
		if (getWorkingCity() != NULL)
		{
			getWorkingCity()->AI_setAssignWorkDirty(true);
			getWorkingCity()->updateFeatureHappiness();
		}

		updateYield();
/*************************************************************************************************/
/**	ADDON (New Improvement System) Sephi                                 						**/
/*************************************************************************************************/
		//force Recalculation of adjacent plots
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if(pAdjacentPlot->getTeam()==getTeam())
				{
					pAdjacentPlot->updateYield();
				}
			}
		}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

		updateFog();
		updateShowCitySymbols();

		if (getOwnerINLINE() == GC.getGameINLINE().getActivePlayer())
		{
			if (gDLL->getGraphicOption(GRAPHICOPTION_CITY_RADIUS))
			{
				if (gDLL->getInterfaceIFace()->canSelectionListFound())
				{
					gDLL->getInterfaceIFace()->setDirty(ColoredPlots_DIRTY_BIT, true);
				}
			}
		}
	}
}


CvCity* CvPlot::getWorkingCityOverride() const
{
	return getCity(m_workingCityOverride);
}


void CvPlot::setWorkingCityOverride( const CvCity* pNewValue)
{
	if (getWorkingCityOverride() != pNewValue)
	{
		if (pNewValue != NULL)
		{
			FAssertMsg(pNewValue->getOwnerINLINE() == getOwnerINLINE(), "Argument city pNewValue's owner is expected to be the same as the current instance");
			m_workingCityOverride = pNewValue->getIDInfo();
		}
		else
		{
			m_workingCityOverride.reset();
		}

		updateWorkingCity();
	}
}


int CvPlot::getRiverID() const
{
	return m_iRiverID;
}


void CvPlot::setRiverID(int iNewValue)
{
	m_iRiverID = iNewValue;
}


int CvPlot::getMinOriginalStartDist() const
{
	return m_iMinOriginalStartDist;
}


void CvPlot::setMinOriginalStartDist(int iNewValue)
{
	m_iMinOriginalStartDist = iNewValue;
}


int CvPlot::getReconCount() const
{
	return m_iReconCount;
}


void CvPlot::changeReconCount(int iChange)
{
	m_iReconCount = (m_iReconCount + iChange);
	FAssert(getReconCount() >= 0);
}


int CvPlot::getRiverCrossingCount() const
{
	return m_iRiverCrossingCount;
}


void CvPlot::changeRiverCrossingCount(int iChange)
{
	m_iRiverCrossingCount = (m_iRiverCrossingCount + iChange);
	FAssert(getRiverCrossingCount() >= 0);
}


short* CvPlot::getYield()
{
	return m_aiYield;
}


int CvPlot::getYield(YieldTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_YIELD_TYPES, "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_aiYield[eIndex];
}


int CvPlot::calculateNatureYield(YieldTypes eYield, TeamTypes eTeam, bool bIgnoreFeature) const
{
	BonusTypes eBonus;
	int iYield;

	if (isImpassable())
	{
		return 0;
	}

	FAssertMsg(getTerrainType() != NO_TERRAIN, "TerrainType is not assigned a valid value");

	iYield = GC.getTerrainInfo(getTerrainType()).getYield(eYield);

	if (isHills())
	{
		iYield += GC.getYieldInfo(eYield).getHillsChange();
	}

	if (isPeak())
	{
		iYield += GC.getYieldInfo(eYield).getPeakChange();
	}

	if (isLake())
	{
		iYield += GC.getYieldInfo(eYield).getLakeChange();
	}

	if (eTeam != NO_TEAM)
	{
		eBonus = getBonusType(eTeam);

		if (eBonus != NO_BONUS)
		{
			iYield += GC.getBonusInfo(eBonus).getYieldChange(eYield);
		}
	}

	if (isRiver())
	{
		iYield += ((bIgnoreFeature || (getFeatureType() == NO_FEATURE)) ? GC.getTerrainInfo(getTerrainType()).getRiverYieldChange(eYield) : GC.getFeatureInfo(getFeatureType()).getRiverYieldChange(eYield));
	}

	if (isHills())
	{
		iYield += ((bIgnoreFeature || (getFeatureType() == NO_FEATURE)) ? GC.getTerrainInfo(getTerrainType()).getHillsYieldChange(eYield) : GC.getFeatureInfo(getFeatureType()).getHillsYieldChange(eYield));
	}

	if (!bIgnoreFeature)
	{
		if (getFeatureType() != NO_FEATURE)
		{
			iYield += GC.getFeatureInfo(getFeatureType()).getYieldChange(eYield);
		}
	}

//FfH: Added by Kael 09/25/2008
    if (isOwned())
    {
        if (GET_PLAYER(getOwnerINLINE()).getCivilizationType() == GC.getTerrainInfo((TerrainTypes)getTerrainType()).getCivilizationYieldType())
        {
            iYield += GC.getTerrainInfo((TerrainTypes)getTerrainType()).getCivilizationYieldChange(eYield);
        }
    }
//FfH: End Add

	return std::max(0, iYield);
}


int CvPlot::calculateBestNatureYield(YieldTypes eIndex, TeamTypes eTeam) const
{
	return std::max(calculateNatureYield(eIndex, eTeam, false), calculateNatureYield(eIndex, eTeam, true));
}


int CvPlot::calculateTotalBestNatureYield(TeamTypes eTeam) const
{
	return (calculateBestNatureYield(YIELD_FOOD, eTeam) + calculateBestNatureYield(YIELD_PRODUCTION, eTeam) + calculateBestNatureYield(YIELD_COMMERCE, eTeam));
}


/************************************************************************************************/
/* BETTER_BTS_AI_MOD                      10/06/09                                jdog5000      */
/*                                                                                              */
/* City AI                                                                                      */
/************************************************************************************************/
int CvPlot::calculateImprovementYieldChange(ImprovementTypes eImprovement, YieldTypes eYield, PlayerTypes ePlayer, bool bOptimal, bool bBestRoute) const
{
	PROFILE_FUNC();

	BonusTypes eBonus;
	int iBestYield;
	int iYield;
	int iI;

	iYield = GC.getImprovementInfo(eImprovement).getYieldChange(eYield);

	if (isRiverSide())
	{
		iYield += GC.getImprovementInfo(eImprovement).getRiverSideYieldChange(eYield);
	}

	if (isHills())
	{
		iYield += GC.getImprovementInfo(eImprovement).getHillsYieldChange(eYield);
	}

	if ((bOptimal) ? true : isIrrigationAvailable())
	{
		iYield += GC.getImprovementInfo(eImprovement).getIrrigatedYieldChange(eYield);
	}

	if (bOptimal)
	{
		iBestYield = 0;

		for (iI = 0; iI < GC.getNumRouteInfos(); ++iI)
		{
			iBestYield = std::max(iBestYield, GC.getImprovementInfo(eImprovement).getRouteYieldChanges(iI, eYield));
		}

		iYield += iBestYield;
	}
	else
	{
		RouteTypes eRoute = getRouteType();;

		if( bBestRoute && ePlayer != NO_PLAYER )
		{
			eRoute = GET_PLAYER(ePlayer).getBestRoute(GC.getMapINLINE().plotSorenINLINE(getX_INLINE(), getY_INLINE()));
		}

		if (eRoute != NO_ROUTE)
		{
			iYield += GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eRoute, eYield);
		}
	}

	if (bOptimal || ePlayer == NO_PLAYER)
	{
		for (iI = 0; iI < GC.getNumTechInfos(); ++iI)
		{
			iYield += GC.getImprovementInfo(eImprovement).getTechYieldChanges(iI, eYield);
		}

		for (iI = 0; iI < GC.getNumCivicInfos(); ++iI)
		{
			iYield += GC.getCivicInfo((CivicTypes) iI).getImprovementYieldChanges(eImprovement, eYield);
		}
	}
	else
	{
		iYield += GET_PLAYER(ePlayer).getImprovementYieldChange(eImprovement, eYield);
		iYield += GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getImprovementYieldChange(eImprovement, eYield);
	}

	if (ePlayer != NO_PLAYER)
	{
		eBonus = getBonusType(GET_PLAYER(ePlayer).getTeam());

		if (eBonus != NO_BONUS)
		{
			iYield += GC.getImprovementInfo(eImprovement).getImprovementBonusYield(eBonus, eYield);
		}
	}

	return iYield;
}
/**
int CvPlot::calculateImprovementYieldChange(ImprovementTypes eImprovement, YieldTypes eYield, PlayerTypes ePlayer, bool bOptimal) const
{
	PROFILE_FUNC();

	BonusTypes eBonus;
	int iBestYield;
	int iYield;
	int iI;

	iYield = GC.getImprovementInfo(eImprovement).getYieldChange(eYield);

	if (isRiverSide())
	{
		iYield += GC.getImprovementInfo(eImprovement).getRiverSideYieldChange(eYield);
	}

	if (isHills())
	{
		iYield += GC.getImprovementInfo(eImprovement).getHillsYieldChange(eYield);
	}

	if ((bOptimal) ? true : isIrrigationAvailable())
	{
		iYield += GC.getImprovementInfo(eImprovement).getIrrigatedYieldChange(eYield);
	}

	if (bOptimal)
	{
		iBestYield = 0;

		for (iI = 0; iI < GC.getNumRouteInfos(); ++iI)
		{
			iBestYield = std::max(iBestYield, GC.getImprovementInfo(eImprovement).getRouteYieldChanges(iI, eYield));
		}

		iYield += iBestYield;
	}
	else
	{
		if (getRouteType() != NO_ROUTE)
		{
			iYield += GC.getImprovementInfo(eImprovement).getRouteYieldChanges(getRouteType(), eYield);
		}
	}

	if (bOptimal || ePlayer == NO_PLAYER)
	{
		for (iI = 0; iI < GC.getNumTechInfos(); ++iI)
		{
			iYield += GC.getImprovementInfo(eImprovement).getTechYieldChanges(iI, eYield);
		}

		for (iI = 0; iI < GC.getNumCivicInfos(); ++iI)
		{
			iYield += GC.getCivicInfo((CivicTypes) iI).getImprovementYieldChanges(eImprovement, eYield);
		}
	}
	else
	{
		iYield += GET_PLAYER(ePlayer).getImprovementYieldChange(eImprovement, eYield);
		iYield += GET_TEAM(GET_PLAYER(ePlayer).getTeam()).getImprovementYieldChange(eImprovement, eYield);
	}

	if (ePlayer != NO_PLAYER)
	{
		eBonus = getBonusType(GET_PLAYER(ePlayer).getTeam());

		if (eBonus != NO_BONUS)
		{
			iYield += GC.getImprovementInfo(eImprovement).getImprovementBonusYield(eBonus, eYield);
		}
	}

	return iYield;
}
/************************************************************************************************/
/* BETTER_BTS_AI_MOD                       END                                                  */
/************************************************************************************************/




int CvPlot::calculateYield(YieldTypes eYield, bool bDisplay) const
{
	CvCity* pCity;
	CvCity* pWorkingCity;
	ImprovementTypes eImprovement;
	RouteTypes eRoute;
	PlayerTypes ePlayer;
	bool bCity;
	int iYield;

//FfH: Added by Kael 10/04/2008
    if (isOwned())
    {
        if (eYield == YIELD_FOOD)
        {
            if (GET_PLAYER(getOwnerINLINE()).isIgnoreFood())
            {
                return 0;
            }
        }
    }
//FfH: End Add

	if (bDisplay && GC.getGameINLINE().isDebugMode())
	{
		return getYield(eYield);
	}

	if (getTerrainType() == NO_TERRAIN)
	{
		return 0;
	}

	if (!isPotentialCityWork())
	{
		return 0;
	}

	bCity = false;

	if (bDisplay)
	{
		ePlayer = getRevealedOwner(GC.getGameINLINE().getActiveTeam(), false);
		eImprovement = getRevealedImprovementType(GC.getGameINLINE().getActiveTeam(), false);
		eRoute = getRevealedRouteType(GC.getGameINLINE().getActiveTeam(), false);

		if (ePlayer == NO_PLAYER)
		{
			ePlayer = GC.getGameINLINE().getActivePlayer();
		}
	}
	else
	{
		ePlayer = getOwnerINLINE();
		eImprovement = getImprovementType();
		eRoute = getRouteType();
	}

	iYield = calculateNatureYield(eYield, ((ePlayer != NO_PLAYER) ? GET_PLAYER(ePlayer).getTeam() : NO_TEAM));

	if (eImprovement != NO_IMPROVEMENT)
	{
		iYield += calculateImprovementYieldChange(eImprovement, eYield, ePlayer);
	}

	if (eRoute != NO_ROUTE)
	{
		iYield += GC.getRouteInfo(eRoute).getYieldChange(eYield);
	}

	if (ePlayer != NO_PLAYER)
	{
		pCity = getPlotCity();

		if (pCity != NULL)
		{
			if (!bDisplay || pCity->isRevealed(GC.getGameINLINE().getActiveTeam(), false))
			{
				iYield += GC.getYieldInfo(eYield).getCityChange();
				if (GC.getYieldInfo(eYield).getPopulationChangeDivisor() != 0)
				{
					iYield += ((pCity->getPopulation() + GC.getYieldInfo(eYield).getPopulationChangeOffset()) / GC.getYieldInfo(eYield).getPopulationChangeDivisor());
				}
				bCity = true;
			}
		}

		if (isWater())
		{
			if (!isImpassable())
			{
				iYield += GET_PLAYER(ePlayer).getSeaPlotYield(eYield);

				pWorkingCity = getWorkingCity();

				if (pWorkingCity != NULL)
				{
					if (!bDisplay || pWorkingCity->isRevealed(GC.getGameINLINE().getActiveTeam(), false))
					{
						iYield += pWorkingCity->getSeaPlotYield(eYield);
					}
				}
			}
		}

		if (isRiver())
		{
			if (!isImpassable())
			{
				pWorkingCity = getWorkingCity();

				if (NULL != pWorkingCity)
				{
					if (!bDisplay || pWorkingCity->isRevealed(GC.getGameINLINE().getActiveTeam(), false))
					{
						iYield += pWorkingCity->getRiverPlotYield(eYield);
					}
				}
			}
		}
	}

	if (bCity)
	{
		iYield = std::max(iYield, GC.getYieldInfo(eYield).getMinCity());
	}

	iYield += GC.getGameINLINE().getPlotExtraYield(m_iX, m_iY, eYield);

	if (ePlayer != NO_PLAYER)
	{
		if (GET_PLAYER(ePlayer).getExtraYieldThreshold(eYield) > 0)
		{
			if (iYield >= GET_PLAYER(ePlayer).getExtraYieldThreshold(eYield))
			{
				iYield += GC.getDefineINT("EXTRA_YIELD");
			}
		}

		if (GET_PLAYER(ePlayer).isGoldenAge())
		{
			if (iYield >= GC.getYieldInfo(eYield).getGoldenAgeYieldThreshold())
			{
				iYield += GC.getYieldInfo(eYield).getGoldenAgeYield();
			}
		}
	}

/*************************************************************************************************/
/**	ADDON (New Improvement System) Sephi                                 						**/
/*************************************************************************************************/
	if (eImprovement != NO_IMPROVEMENT)
	{
		for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if(pAdjacentPlot->getTeam() == getTeam() && pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT)
				{
					if(isExtraYieldfromAdjacentImprovementValid(pAdjacentPlot, pAdjacentPlot->getImprovementType())) {
						iYield += calculateExtraYieldfromAdjacentPlot(pAdjacentPlot, eYield, getImprovementType(), pAdjacentPlot->getImprovementType());
					}
				}
			}
		}
	}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

	return std::max(0, iYield);
}


bool CvPlot::hasYield() const
{
	int iI;

	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		if (getYield((YieldTypes)iI) > 0)
		{
			return true;
		}
	}

	return false;
}


void CvPlot::updateYield()
{
	CvCity* pWorkingCity;
	bool bChange;
	int iNewYield;
	int iOldYield;
	int iI;

	if (area() == NULL)
	{
		return;
	}

	bChange = false;

	for (iI = 0; iI < NUM_YIELD_TYPES; ++iI)
	{
		iNewYield = calculateYield((YieldTypes)iI);

		if (getYield((YieldTypes)iI) != iNewYield)
		{
			iOldYield = getYield((YieldTypes)iI);

			m_aiYield[iI] = iNewYield;
			FAssert(getYield((YieldTypes)iI) >= 0);

			pWorkingCity = getWorkingCity();

			if (pWorkingCity != NULL)
			{
				if (isBeingWorked())
				{
					pWorkingCity->changeBaseYieldRate(((YieldTypes)iI), (getYield((YieldTypes)iI) - iOldYield), true);
				}

				pWorkingCity->AI_setAssignWorkDirty(true);
			}

			bChange = true;
		}
	}

	if (bChange)
	{
		updateSymbols();
	}
}


int CvPlot::getCulture(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "iIndex is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_aiCulture)
	{
		return 0;
	}

	return m_aiCulture[eIndex];
}


int CvPlot::countTotalCulture() const
{
	int iTotalCulture;
	int iI;

	iTotalCulture = 0;

	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			iTotalCulture += getCulture((PlayerTypes)iI);
		}
	}

	return iTotalCulture;
}


TeamTypes CvPlot::findHighestCultureTeam() const
{
	PlayerTypes eBestPlayer = findHighestCulturePlayer();

	if (NO_PLAYER == eBestPlayer)
	{
		return NO_TEAM;
	}

	return GET_PLAYER(eBestPlayer).getTeam();
}


PlayerTypes CvPlot::findHighestCulturePlayer() const
{
	PlayerTypes eBestPlayer = NO_PLAYER;
	int iBestValue = 0;

	for (int iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			int iValue = getCulture((PlayerTypes)iI);

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				eBestPlayer = (PlayerTypes)iI;
			}
		}
	}

	return eBestPlayer;
}


int CvPlot::calculateCulturePercent(PlayerTypes eIndex) const
{
	int iTotalCulture;

	iTotalCulture = countTotalCulture();

	if (iTotalCulture > 0)
	{
		return ((getCulture(eIndex) * 100) / iTotalCulture);
	}

	return 0;
}


int CvPlot::calculateTeamCulturePercent(TeamTypes eIndex) const
{
	int iTeamCulturePercent;
	int iI;

	iTeamCulturePercent = 0;

	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if (GET_PLAYER((PlayerTypes)iI).getTeam() == eIndex)
			{
				iTeamCulturePercent += calculateCulturePercent((PlayerTypes)iI);
			}
		}
	}

	return iTeamCulturePercent;
}


void CvPlot::setCulture(PlayerTypes eIndex, int iNewValue, bool bUpdate, bool bUpdatePlotGroups)
{
	PROFILE_FUNC();

	CvCity* pCity;

	FAssertMsg(eIndex >= 0, "iIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "iIndex is expected to be within maximum bounds (invalid Index)");

	if (getCulture(eIndex) != iNewValue)
	{
		if(NULL == m_aiCulture)
		{
			m_aiCulture = new int[MAX_PLAYERS];
			for (int iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				m_aiCulture[iI] = 0;
			}
		}

		m_aiCulture[eIndex] = iNewValue;
		FAssert(getCulture(eIndex) >= 0);

		if (bUpdate)
		{
			updateCulture(true, bUpdatePlotGroups);
		}

		pCity = getPlotCity();

		if (pCity != NULL)
		{
			pCity->AI_setAssignWorkDirty(true);
		}
	}

}


void CvPlot::changeCulture(PlayerTypes eIndex, int iChange, bool bUpdate)
{
	if(isNonPlanar())
		return;

	if (0 != iChange)
	{
		setCulture(eIndex, (getCulture(eIndex) + iChange), bUpdate, true);
	}
}


int CvPlot::getFoundValue(PlayerTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_aiFoundValue)
	{
		return 0;
	}

	if (m_aiFoundValue[eIndex] == -1)
	{
		long lResult=-1;
		if(GC.getUSE_GET_CITY_FOUND_VALUE_CALLBACK())
		{
			CyArgsList argsList;
			argsList.add((int)eIndex);
			argsList.add(getX());
			argsList.add(getY());
			gDLL->getPythonIFace()->callFunction(PYGameModule, "getCityFoundValue", argsList.makeFunctionArgs(), &lResult);
		}

		if (lResult == -1)
		{
			m_aiFoundValue[eIndex] = GET_PLAYER(eIndex).AI_foundValue(getX_INLINE(), getY_INLINE(), -1, true);
		}

		if (m_aiFoundValue[eIndex] > area()->getBestFoundValue(eIndex))
		{
			area()->setBestFoundValue(eIndex, m_aiFoundValue[eIndex]);
		}
	}

	return m_aiFoundValue[eIndex];
}


bool CvPlot::isBestAdjacentFound(PlayerTypes eIndex)
{
	CvPlot* pAdjacentPlot;
	int iI;

	int iPlotValue = GET_PLAYER(eIndex).AI_foundValue(getX_INLINE(), getY_INLINE());

	if (iPlotValue == 0)
	{
		return false;
	}

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if ((pAdjacentPlot != NULL) && pAdjacentPlot->isRevealed(GET_PLAYER(eIndex).getTeam(), false))
		{
			//if (pAdjacentPlot->getFoundValue(eIndex) >= getFoundValue(eIndex))
			if (GET_PLAYER(eIndex).AI_foundValue(pAdjacentPlot->getX_INLINE(), pAdjacentPlot->getY_INLINE()) > iPlotValue)
			{
				return false;
			}
		}
	}

	return true;
}


void CvPlot::setFoundValue(PlayerTypes eIndex, int iNewValue)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");
	FAssert(iNewValue >= -1);

	if (NULL == m_aiFoundValue && 0 != iNewValue)
	{
		m_aiFoundValue = new short[MAX_PLAYERS];
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			m_aiFoundValue[iI] = 0;
		}
	}

	if (NULL != m_aiFoundValue)
	{
		m_aiFoundValue[eIndex] = iNewValue;
	}
}


int CvPlot::getPlayerCityRadiusCount(PlayerTypes eIndex) const
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_aiPlayerCityRadiusCount)
	{
		return 0;
	}

	return m_aiPlayerCityRadiusCount[eIndex];
}


bool CvPlot::isPlayerCityRadius(PlayerTypes eIndex) const
{
	return (getPlayerCityRadiusCount(eIndex) > 0);
}


void CvPlot::changePlayerCityRadiusCount(PlayerTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < MAX_PLAYERS, "eIndex is expected to be within maximum bounds (invalid Index)");

	if (0 != iChange)
	{
		if (NULL == m_aiPlayerCityRadiusCount)
		{
			m_aiPlayerCityRadiusCount = new char[MAX_PLAYERS];
			for (int iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				m_aiPlayerCityRadiusCount[iI] = 0;
			}
			if (iChange < 0)
				return;
		}

		m_aiPlayerCityRadiusCount[eIndex] += iChange;
		FAssert(getPlayerCityRadiusCount(eIndex) >= 0);
	}
}


CvPlotGroup* CvPlot::getPlotGroup(PlayerTypes ePlayer) const
{
	FAssertMsg(ePlayer >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(ePlayer < MAX_PLAYERS, "ePlayer is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_aiPlotGroup)
	{
		return GET_PLAYER(ePlayer).getPlotGroup(FFreeList::INVALID_INDEX);
	}

	return GET_PLAYER(ePlayer).getPlotGroup(m_aiPlotGroup[ePlayer]);
}


CvPlotGroup* CvPlot::getOwnerPlotGroup() const
{
	if (getOwnerINLINE() == NO_PLAYER)
	{
		return NULL;
	}

	return getPlotGroup(getOwnerINLINE());
}


void CvPlot::setPlotGroup(PlayerTypes ePlayer, CvPlotGroup* pNewValue)
{
	int iI;

	CvPlotGroup* pOldPlotGroup = getPlotGroup(ePlayer);

	if (pOldPlotGroup != pNewValue)
	{
		if (NULL ==  m_aiPlotGroup)
		{
			m_aiPlotGroup = new int[MAX_PLAYERS];
			for (int iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				m_aiPlotGroup[iI] = FFreeList::INVALID_INDEX;
			}
		}

		CvCity* pCity = getPlotCity();

		if (ePlayer == getOwnerINLINE())
		{
			updatePlotGroupBonus(false);
		}

		if (pOldPlotGroup != NULL)
		{
			if (pCity != NULL)
			{
				if (pCity->getOwnerINLINE() == ePlayer)
				{
					FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvPlot::setPlotGroup");
					for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
					{
						/*if(isOOSLogging() && (pOldPlotGroup->getNumBonuses((BonusTypes)iI)) != 0)
						{
							oosLog("AIBonuses","Turn:%d,Player:%d,CityID:%d,Bonus:%S,pOldPlotGroup:%d,GoodHealth:%d,BadHealth:%d"
								,GC.getGameINLINE().getElapsedGameTurns()
								,pCity->getOwnerINLINE()
								,pCity->getID()
								,GC.getBonusInfo((BonusTypes)iI).getDescription()
								,-(pOldPlotGroup->getNumBonuses((BonusTypes)iI))
								,pCity->getBonusGoodHealth()
								,pCity->getBonusBadHealth()
							);
						}*/

						pCity->changeNumBonuses(((BonusTypes)iI), -(pOldPlotGroup->getNumBonuses((BonusTypes)iI)));
					}
				}
			}
		}

		if (pNewValue == NULL)
		{
			m_aiPlotGroup[ePlayer] = FFreeList::INVALID_INDEX;
		}
		else
		{
			m_aiPlotGroup[ePlayer] = pNewValue->getID();
		}

		if (getPlotGroup(ePlayer) != NULL)
		{
			if (pCity != NULL)
			{
				if (pCity->getOwnerINLINE() == ePlayer)
				{
					FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvPlot::setPlotGroup");
					for (iI = 0; iI < GC.getNumBonusInfos(); ++iI)
					{
						/*if(isOOSLogging() && getPlotGroup(ePlayer)->getNumBonuses((BonusTypes)iI) != 0)
						{
							oosLog("AIBonuses","Turn:%d,Player:%d,CityID:%d,Bonus:%S,pNewValue:%d,GoodHealth:%d,BadHealth:%d"
								,GC.getGameINLINE().getElapsedGameTurns()
								,pCity->getOwnerINLINE()
								,pCity->getID()
								,GC.getBonusInfo((BonusTypes)iI).getDescription()
								,getPlotGroup(ePlayer)->getNumBonuses((BonusTypes)iI)
								,pCity->getBonusGoodHealth()
								,pCity->getBonusBadHealth()
							);
						}*/

						pCity->changeNumBonuses(((BonusTypes)iI), getPlotGroup(ePlayer)->getNumBonuses((BonusTypes)iI));
					}
				}
			}
		}
		if (ePlayer == getOwnerINLINE())
		{
			updatePlotGroupBonus(true);
		}
	}
}


void CvPlot::updatePlotGroup()
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			updatePlotGroup((PlayerTypes)iI);
		}
	}
}


void CvPlot::updatePlotGroup(PlayerTypes ePlayer, bool bRecalculate)
{
	PROFILE("CvPlot::updatePlotGroup(Player)");

	CvPlotGroup* pPlotGroup;
	CvPlotGroup* pAdjacentPlotGroup;
	CvPlot* pAdjacentPlot;
	bool bConnected;
	bool bEmpty;
	int iI;

	if (!(GC.getGameINLINE().isFinalInitialized()))
	{
		return;
	}

	pPlotGroup = getPlotGroup(ePlayer);

	if (pPlotGroup != NULL)
	{
		if (bRecalculate)
		{
			bConnected = false;

			if (isTradeNetwork(GET_PLAYER(ePlayer).getTeam()))
			{
				bConnected = true;

				for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
				{
					pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

					if (pAdjacentPlot != NULL)
					{
						if (pAdjacentPlot->getPlotGroup(ePlayer) == pPlotGroup)
						{
							if (!isTradeNetworkConnected(pAdjacentPlot, GET_PLAYER(ePlayer).getTeam()))
							{
								bConnected = false;
								break;
							}
						}
					}
				}
			}

			if (!bConnected)
			{
				bEmpty = (pPlotGroup->getLengthPlots() == 1);
				FAssertMsg(pPlotGroup->getLengthPlots() > 0, "pPlotGroup should have more than 0 plots");

				pPlotGroup->removePlot(this);

				if (!bEmpty)
				{
					pPlotGroup->recalculatePlots();
				}
			}
		}

		pPlotGroup = getPlotGroup(ePlayer);
	}

	if (isTradeNetwork(GET_PLAYER(ePlayer).getTeam()))
	{
		for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				pAdjacentPlotGroup = pAdjacentPlot->getPlotGroup(ePlayer);

				if ((pAdjacentPlotGroup != NULL) && (pAdjacentPlotGroup != pPlotGroup))
				{
					if (isTradeNetworkConnected(pAdjacentPlot, GET_PLAYER(ePlayer).getTeam()))
					{
						if (pPlotGroup == NULL)
						{
							pAdjacentPlotGroup->addPlot(this);
							pPlotGroup = pAdjacentPlotGroup;
							FAssertMsg(getPlotGroup(ePlayer) == pPlotGroup, "ePlayer's plot group is expected to equal pPlotGroup");
						}
						else
						{
							FAssertMsg(getPlotGroup(ePlayer) == pPlotGroup, "ePlayer's plot group is expected to equal pPlotGroup");
							GC.getMapINLINE().combinePlotGroups(ePlayer, pPlotGroup, pAdjacentPlotGroup);
							pPlotGroup = getPlotGroup(ePlayer);
							FAssertMsg(pPlotGroup != NULL, "PlotGroup is not assigned a valid value");
						}
					}
				}
			}
		}

		if (pPlotGroup == NULL)
		{
			GET_PLAYER(ePlayer).initPlotGroup(this);
		}
	}
}


int CvPlot::getVisibilityCount(TeamTypes eTeam) const
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_aiVisibilityCount)
	{
		return 0;
	}

	return m_aiVisibilityCount[eTeam];
}


void CvPlot::changeVisibilityCount(TeamTypes eTeam, int iChange, InvisibleTypes eSeeInvisible, bool bUpdatePlotGroups)
{
	CvCity* pCity;
	CvPlot* pAdjacentPlot;
	bool bOldVisible;
	int iI;

	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		if (NULL == m_aiVisibilityCount)
		{
			m_aiVisibilityCount = new short[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_aiVisibilityCount[iI] = 0;
			}
		}

//FfH: Added by Kael 08/23/2008 (to fix an issue where visibility can get into the negatives)
        if (m_aiVisibilityCount[eTeam] < 0)
        {
            m_aiVisibilityCount[eTeam] = 0;
        }
//FfH: End Add

		bOldVisible = isVisible(eTeam, false);

		m_aiVisibilityCount[eTeam] += iChange;
//		FAssert(getVisibilityCount(eTeam) >= 0);  Some things can reduce Visibility modified Sephi

		if (eSeeInvisible != NO_INVISIBLE)
		{
			changeInvisibleVisibilityCount(eTeam, eSeeInvisible, iChange);
		}

		if (bOldVisible != isVisible(eTeam, false))
		{
			if (isVisible(eTeam, false))
			{
				setRevealed(eTeam, true, false, NO_TEAM, bUpdatePlotGroups);

				for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
				{
					pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

					if (pAdjacentPlot != NULL)
					{
						pAdjacentPlot->updateRevealedOwner(eTeam);
					}
				}

				if (getTeam() != NO_TEAM)
				{
					GET_TEAM(getTeam()).meet(eTeam, true);
				}
			}

			pCity = getPlotCity();

			if (pCity != NULL)
			{
				pCity->setInfoDirty(true);
			}

			for (iI = 0; iI < MAX_TEAMS; ++iI)
			{
				if (GET_TEAM((TeamTypes)iI).isAlive())
				{
					if (GET_TEAM((TeamTypes)iI).isStolenVisibility(eTeam))
					{
						changeStolenVisibilityCount(((TeamTypes)iI), ((isVisible(eTeam, false)) ? 1 : -1));
					}
				}
			}

			if (eTeam == GC.getGameINLINE().getActiveTeam())
			{
				updateFog();
				updateMinimapColor();
				updateCenterUnit();
			}
		}
	}
}


int CvPlot::getStolenVisibilityCount(TeamTypes eTeam) const
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_aiStolenVisibilityCount)
	{
		return 0;
	}

	return m_aiStolenVisibilityCount[eTeam];
}


void CvPlot::changeStolenVisibilityCount(TeamTypes eTeam, int iChange)
{
	CvCity* pCity;
	bool bOldVisible;

	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		if (NULL == m_aiStolenVisibilityCount)
		{
			m_aiStolenVisibilityCount = new short[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_aiStolenVisibilityCount[iI] = 0;
			}
		}

		bOldVisible = isVisible(eTeam, false);

		m_aiStolenVisibilityCount[eTeam] += iChange;
		FAssert(getStolenVisibilityCount(eTeam) >= 0);

		if (bOldVisible != isVisible(eTeam, false))
		{
			if (isVisible(eTeam, false))
			{
				setRevealed(eTeam, true, false, NO_TEAM, true);
			}

			pCity = getPlotCity();

			if (pCity != NULL)
			{
				pCity->setInfoDirty(true);
			}

			if (eTeam == GC.getGameINLINE().getActiveTeam())
			{
				updateFog();
				updateMinimapColor();
				updateCenterUnit();
			}
		}
	}
}


int CvPlot::getBlockadedCount(TeamTypes eTeam) const
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_aiBlockadedCount)
	{
		return 0;
	}

	return m_aiBlockadedCount[eTeam];
}

void CvPlot::changeBlockadedCount(TeamTypes eTeam, int iChange)
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		if (NULL == m_aiBlockadedCount)
		{
			m_aiBlockadedCount = new short[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_aiBlockadedCount[iI] = 0;
			}
		}

		m_aiBlockadedCount[eTeam] += iChange;
		FAssert(getBlockadedCount(eTeam) >= 0);
		FAssert(getBlockadedCount(eTeam) == 0 || isWater())

		CvCity* pWorkingCity = getWorkingCity();
		if (NULL != pWorkingCity)
		{
			pWorkingCity->AI_setAssignWorkDirty(true);
		}
	}
}

PlayerTypes CvPlot::getRevealedOwner(TeamTypes eTeam, bool bDebug) const
{
	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return getOwnerINLINE();
	}
	else
	{
		FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
		FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

		if (NULL == m_aiRevealedOwner)
		{
			return NO_PLAYER;
		}

		return (PlayerTypes)m_aiRevealedOwner[eTeam];
	}
}


TeamTypes CvPlot::getRevealedTeam(TeamTypes eTeam, bool bDebug) const
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	PlayerTypes eRevealedOwner = getRevealedOwner(eTeam, bDebug);

	if (eRevealedOwner != NO_PLAYER)
	{
		return GET_PLAYER(eRevealedOwner).getTeam();
	}
	else
	{
		return NO_TEAM;
	}
}


void CvPlot::setRevealedOwner(TeamTypes eTeam, PlayerTypes eNewValue)
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (getRevealedOwner(eTeam, false) != eNewValue)
	{
		if (NULL == m_aiRevealedOwner)
		{
			m_aiRevealedOwner = new char[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_aiRevealedOwner[iI] = -1;
			}
		}

		m_aiRevealedOwner[eTeam] = eNewValue;

		if (eTeam == GC.getGameINLINE().getActiveTeam())
		{
			updateMinimapColor();

			if (GC.IsGraphicsInitialized())
			{
				gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);

				gDLL->getEngineIFace()->SetDirty(CultureBorders_DIRTY_BIT, true);
			}
		}
	}

	FAssert((NULL == m_aiRevealedOwner) || (m_aiRevealedOwner[eTeam] == eNewValue));
}


void CvPlot::updateRevealedOwner(TeamTypes eTeam)
{
	CvPlot* pAdjacentPlot;
	bool bRevealed;
	int iI;

	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	bRevealed = false;

	if (!bRevealed)
	{
		if (isVisible(eTeam, false))
		{
			bRevealed = true;
		}
	}

	if (!bRevealed)
	{
		for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				if (pAdjacentPlot->isVisible(eTeam, false))
				{
					bRevealed = true;
					break;
				}
			}
		}
	}

	if (bRevealed)
	{
		setRevealedOwner(eTeam, getOwnerINLINE());
	}
}


bool CvPlot::isRiverCrossing(DirectionTypes eIndex) const
{
	FAssertMsg(eIndex < NUM_DIRECTION_TYPES, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (eIndex == NO_DIRECTION)
	{
		return false;
	}

	if (NULL == m_abRiverCrossing)
	{
		return false;
	}

	return m_abRiverCrossing[eIndex];
}


void CvPlot::updateRiverCrossing(DirectionTypes eIndex)
{
	CvPlot* pNorthEastPlot;
	CvPlot* pSouthEastPlot;
	CvPlot* pSouthWestPlot;
	CvPlot* pNorthWestPlot;
	CvPlot* pCornerPlot;
	CvPlot* pPlot;
	bool bValid;

	FAssertMsg(eIndex >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < NUM_DIRECTION_TYPES, "eTeam is expected to be within maximum bounds (invalid Index)");

	pCornerPlot = NULL;
	bValid = false;
	pPlot = plotDirection(getX_INLINE(), getY_INLINE(), eIndex);

	if ((NULL == pPlot || !pPlot->isWater()) && !isWater())
	{
		switch (eIndex)
		{
		case DIRECTION_NORTH:
			if (pPlot != NULL)
			{
				bValid = pPlot->isNOfRiver();
			}
			break;

		case DIRECTION_NORTHEAST:
			pCornerPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_NORTH);
			break;

		case DIRECTION_EAST:
			bValid = isWOfRiver();
			break;

		case DIRECTION_SOUTHEAST:
			pCornerPlot = this;
			break;

		case DIRECTION_SOUTH:
			bValid = isNOfRiver();
			break;

		case DIRECTION_SOUTHWEST:
			pCornerPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_WEST);
			break;

		case DIRECTION_WEST:
			if (pPlot != NULL)
			{
				bValid = pPlot->isWOfRiver();
			}
			break;

		case DIRECTION_NORTHWEST:
			pCornerPlot = plotDirection(getX_INLINE(), getY_INLINE(), DIRECTION_NORTHWEST);
			break;

		default:
			FAssert(false);
			break;
		}

		if (pCornerPlot != NULL)
		{
			pNorthEastPlot = plotDirection(pCornerPlot->getX_INLINE(), pCornerPlot->getY_INLINE(), DIRECTION_EAST);
			pSouthEastPlot = plotDirection(pCornerPlot->getX_INLINE(), pCornerPlot->getY_INLINE(), DIRECTION_SOUTHEAST);
			pSouthWestPlot = plotDirection(pCornerPlot->getX_INLINE(), pCornerPlot->getY_INLINE(), DIRECTION_SOUTH);
			pNorthWestPlot = pCornerPlot;

			if (pSouthWestPlot && pNorthWestPlot && pSouthEastPlot && pNorthEastPlot)
			{
				if (pSouthWestPlot->isWOfRiver() && pNorthWestPlot->isWOfRiver())
				{
					bValid = true;
				}
				else if (pNorthEastPlot->isNOfRiver() && pNorthWestPlot->isNOfRiver())
				{
					bValid = true;
				}
				else if ((eIndex == DIRECTION_NORTHEAST) || (eIndex == DIRECTION_SOUTHWEST))
				{
					if (pNorthEastPlot->isNOfRiver() && (pNorthWestPlot->isWOfRiver() || pNorthWestPlot->isWater()))
					{
						bValid = true;
					}
					else if ((pNorthEastPlot->isNOfRiver() || pSouthEastPlot->isWater()) && pNorthWestPlot->isWOfRiver())
					{
						bValid = true;
					}
					else if (pSouthWestPlot->isWOfRiver() && (pNorthWestPlot->isNOfRiver() || pNorthWestPlot->isWater()))
					{
						bValid = true;
					}
					else if ((pSouthWestPlot->isWOfRiver() || pSouthEastPlot->isWater()) && pNorthWestPlot->isNOfRiver())
					{
						bValid = true;
					}
				}
				else
				{
					FAssert((eIndex == DIRECTION_SOUTHEAST) || (eIndex == DIRECTION_NORTHWEST));

					if (pNorthWestPlot->isNOfRiver() && (pNorthWestPlot->isWOfRiver() || pNorthEastPlot->isWater()))
					{
						bValid = true;
					}
					else if ((pNorthWestPlot->isNOfRiver() || pSouthWestPlot->isWater()) && pNorthWestPlot->isWOfRiver())
					{
						bValid = true;
					}
					else if (pNorthEastPlot->isNOfRiver() && (pSouthWestPlot->isWOfRiver() || pSouthWestPlot->isWater()))
					{
						bValid = true;
					}
					else if ((pNorthEastPlot->isNOfRiver() || pNorthEastPlot->isWater()) && pSouthWestPlot->isWOfRiver())
					{
						bValid = true;
					}
				}
			}

		}
	}

	if (isRiverCrossing(eIndex) != bValid)
	{
		if (NULL == m_abRiverCrossing)
		{
			m_abRiverCrossing = new bool[NUM_DIRECTION_TYPES];
			for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				m_abRiverCrossing[iI] = false;
			}
		}

		m_abRiverCrossing[eIndex] = bValid;

		changeRiverCrossingCount((isRiverCrossing(eIndex)) ? 1 : -1);
	}
}


void CvPlot::updateRiverCrossing()
{
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		updateRiverCrossing((DirectionTypes)iI);
	}
}


bool CvPlot::isRevealed(TeamTypes eTeam, bool bDebug) const
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return true;
	}

	if(isNonPlanar()) {
		return false;
	}

	if (NULL == m_abRevealed)
	{
		return false;
	}

	return m_abRevealed[eTeam];
}


void CvPlot::setRevealed(TeamTypes eTeam, bool bNewValue, bool bTerrainOnly, TeamTypes eFromTeam, bool bUpdatePlotGroup)
{
	CvCity* pCity;
	int iI;

	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	pCity = getPlotCity();

	/*if(isOOSLogging())
	{
		oosLog("AIPlotVisibility"
			,"Turn:%d,X:%d,Y:%d,Team:%d,NewVal:%d"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getX()
			,getY()
			,eTeam
			,bNewValue
		);
	}*/

	if (isRevealed(eTeam, false) != bNewValue)
	{
		if (NULL == m_abRevealed)
		{
			m_abRevealed = new bool[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_abRevealed[iI] = false;
			}
		}

		m_abRevealed[eTeam] = bNewValue;

		if (area())
		{
			area()->changeNumRevealedTiles(eTeam, ((isRevealed(eTeam, false)) ? 1 : -1));
		}

		if (bUpdatePlotGroup)
		{
			for (iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				if (GET_PLAYER((PlayerTypes)iI).isAlive())
				{
					if (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam)
					{
						updatePlotGroup((PlayerTypes)iI);
					}
				}
			}
		}

		if (eTeam == GC.getGameINLINE().getActiveTeam())
		{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
			if (shouldHaveGraphics())
			{
				CvPlot* pAdjacentPlot;
				int iI;

				for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
				{
					pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

					if (pAdjacentPlot != NULL)
					{
						//	Unrevealed plots adjacent to newly revealed ones, which are ocean plots
						//	also need redrawing to prevent artifacting
						if (pAdjacentPlot->isWater() && !pAdjacentPlot->isRevealed(eTeam, false) && pAdjacentPlot->isInViewport())
						{
							gDLL->getEngineIFace()->RebuildPlot(pAdjacentPlot->getViewportX(), pAdjacentPlot->getViewportY(),true,true);
							pAdjacentPlot->setLayoutDirty(true);
						}
					}
				}

				if (isWater())
				{
					GC.getGame().noteGraphicRebuildNeeded();
				}

				//Update terrain graphics
				gDLL->getEngineIFace()->RebuildPlot(getViewportX(), getViewportY(),true,true);
				//gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true); //minimap does a partial update
				//gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
				updateFeatureSymbol();
				updateRiverSymbol(false, true);
			}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

			updateSymbols();
			updateFog();
			updateVisibility();

			gDLL->getInterfaceIFace()->setDirty(MinimapSection_DIRTY_BIT, true);
			gDLL->getInterfaceIFace()->setDirty(GlobeLayer_DIRTY_BIT, true);
		}

		if (isRevealed(eTeam, false))
		{
			// ONEVENT - PlotRevealed

//FfH: Modified by Kael 08/02/2008
//			CvEventReporter::getInstance().plotRevealed(this, eTeam);
            if(GC.getUSE_PLOT_REVEALED_CALLBACK())
            {
				CvEventReporter::getInstance().plotRevealed(this, eTeam);
            }
//FfH: End Modify

		}
	}

	if (!bTerrainOnly)
	{
		if (isRevealed(eTeam, false))
		{
			if (eFromTeam == NO_TEAM)
			{
				setRevealedOwner(eTeam, getOwnerINLINE());
				setRevealedImprovementType(eTeam, getImprovementType());
				setRevealedRouteType(eTeam, getRouteType());

				if (pCity != NULL)
				{
					pCity->setRevealed(eTeam, true);
				}
			}
			else
			{
				if (getRevealedOwner(eFromTeam, false) == getOwnerINLINE())
				{
					setRevealedOwner(eTeam, getRevealedOwner(eFromTeam, false));
				}

				if (getRevealedImprovementType(eFromTeam, false) == getImprovementType())
				{
					setRevealedImprovementType(eTeam, getRevealedImprovementType(eFromTeam, false));
				}

				if (getRevealedRouteType(eFromTeam, false) == getRouteType())
				{
					setRevealedRouteType(eTeam, getRevealedRouteType(eFromTeam, false));
				}

				if (pCity != NULL)
				{
					if (pCity->isRevealed(eFromTeam, false))
					{
						pCity->setRevealed(eTeam, true);
					}
				}
			}
		}
		else
		{
			setRevealedOwner(eTeam, NO_PLAYER);
			setRevealedImprovementType(eTeam, NO_IMPROVEMENT);
			setRevealedRouteType(eTeam, NO_ROUTE);

			if (pCity != NULL)
			{
				pCity->setRevealed(eTeam, false);
			}
		}
	}
}

bool CvPlot::isAdjacentRevealed(TeamTypes eTeam) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isRevealed(eTeam, false))
			{
				return true;
			}
		}
	}

	return false;
}

bool CvPlot::isAdjacentNonrevealed(TeamTypes eTeam) const
{
	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (!pAdjacentPlot->isRevealed(eTeam, false))
			{
				return true;
			}
		}
	}

	return false;
}


ImprovementTypes CvPlot::getRevealedImprovementType(TeamTypes eTeam, bool bDebug) const
{
	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return getImprovementType();
	}
	else
	{
		FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
		FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

		if (NULL == m_aeRevealedImprovementType)
		{
			return NO_IMPROVEMENT;
		}

		return (ImprovementTypes)m_aeRevealedImprovementType[eTeam];
	}
}


void CvPlot::setRevealedImprovementType(TeamTypes eTeam, ImprovementTypes eNewValue)
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (getRevealedImprovementType(eTeam, false) != eNewValue)
	{
		if (NULL == m_aeRevealedImprovementType)
		{
			m_aeRevealedImprovementType = new short[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_aeRevealedImprovementType[iI] = NO_IMPROVEMENT;
			}
		}

		m_aeRevealedImprovementType[eTeam] = eNewValue;

		if (eTeam == GC.getGameINLINE().getActiveTeam())
		{
			updateSymbols();
			setLayoutDirty(true);
			//gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);
		}
	}
}


RouteTypes CvPlot::getRevealedRouteType(TeamTypes eTeam, bool bDebug) const
{
	if (bDebug && GC.getGameINLINE().isDebugMode())
	{
		return getRouteType();
	}
	else
	{
		FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
		FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

		if (NULL == m_aeRevealedRouteType)
		{
			return NO_ROUTE;
		}

		return (RouteTypes)m_aeRevealedRouteType[eTeam];
	}
}


void CvPlot::setRevealedRouteType(TeamTypes eTeam, RouteTypes eNewValue)
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");

	if (getRevealedRouteType(eTeam, false) != eNewValue)
	{
		if (NULL == m_aeRevealedRouteType)
		{
			m_aeRevealedRouteType = new short[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_aeRevealedRouteType[iI] = NO_ROUTE;
			}
		}

		m_aeRevealedRouteType[eTeam] = eNewValue;

		if (eTeam == GC.getGameINLINE().getActiveTeam())
		{
			updateSymbols();
			updateRouteSymbol(true, true);
		}
	}
}


int CvPlot::getBuildProgress(BuildTypes eBuild) const
{
	if (NULL == m_paiBuildProgress)
	{
		return 0;
	}

	return m_paiBuildProgress[eBuild];
}


// Returns true if build finished...
bool CvPlot::changeBuildProgress(BuildTypes eBuild, int iChange, TeamTypes eTeam)
{
	CvCity* pCity;
	CvWString szBuffer;
	int iProduction;
	bool bFinished;

	bFinished = false;

	if (iChange != 0)
	{
		if (NULL == m_paiBuildProgress)
		{
			m_paiBuildProgress = new short[GC.getNumBuildInfos()];
			for (int iI = 0; iI < GC.getNumBuildInfos(); ++iI)
			{
				m_paiBuildProgress[iI] = 0;
			}
		}

		m_paiBuildProgress[eBuild] += iChange;
		FAssert(getBuildProgress(eBuild) >= 0);

		if (getBuildProgress(eBuild) >= getBuildTime(eBuild))
		{
			m_paiBuildProgress[eBuild] = 0;

			if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
			{
				setImprovementType((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement(), true);
			}

			if (GC.getBuildInfo(eBuild).getRoute() != NO_ROUTE)
			{
				setRouteType((RouteTypes)GC.getBuildInfo(eBuild).getRoute(), true);
			}

			if (getFeatureType() != NO_FEATURE)
			{
				if (GC.getBuildInfo(eBuild).isFeatureRemove(getFeatureType()))
				{

//FfH: Added by Kael 11/11/2007
                    bool bValid = true;
                    if (isOwned())
                    {
                        if (GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).isMaintainFeatures(getFeatureType()))
                        {
                            if (GC.getBuildInfo(eBuild).getImprovement() != NO_IMPROVEMENT)
                            {
                                bValid = false;
                            }
                        }
                    }
                    if (bValid)
                    {
//FfH: End Add

                    FAssertMsg(eTeam != NO_TEAM, "eTeam should be valid");

					iProduction = getFeatureProduction(eBuild, eTeam, &pCity);

					if (iProduction > 0)
					{
/**
						pCity->changeFeatureProduction(iProduction);

						szBuffer = gDLL->getText("TXT_KEY_MISC_CLEARING_FEATURE_BONUS", GC.getFeatureInfo(getFeatureType()).getTextKeyWide(), iProduction, pCity->getNameKey());
						gDLL->getInterfaceIFace()->addMessage(pCity->getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer,  ARTFILEMGR.getInterfaceArtInfo("WORLDBUILDER_CITY_EDIT")->getPath(), MESSAGE_TYPE_INFO, GC.getFeatureInfo(getFeatureType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
**/
						GET_PLAYER(pCity->getOwnerINLINE()).changeGlobalYield(YIELD_LUMBER, iProduction);
					}

					// Python Event
					CvEventReporter::getInstance().plotFeatureRemoved(this, getFeatureType(), pCity);

					setFeatureType(NO_FEATURE);

//FfH: Added by Kael 11/11/2007
                    }
//FfH: End Add

				}
			}

			if(GC.getBuildInfo(eBuild).getTerrainPre() != NO_TERRAIN &&
				GC.getBuildInfo(eBuild).getTerrainPost() != NO_TERRAIN) {
				setTerrainType((TerrainTypes)GC.getBuildInfo(eBuild).getTerrainPost());
			}

			bFinished = true;
		}
	}

	/*if(isOOSLogging())
	{
		oosLog("AIWorker"
			,"Turn: %d,X:%d,Y:%d,bFinished:%d,changeBuildProgress:%S\n"
			,GC.getGameINLINE().getElapsedGameTurns()
			,getX()
			,getY()
			,bFinished
			,eBuild!=NO_IMPROVEMENT?GC.getBuildInfo(eBuild).getDescription():L"NO_IMPROVEMENT"
		);
	}*/
	//added Sephi
	//Update BestBuild now that Improvement is Build
	if(bFinished)
	{
		if(getWorkingCity()!=NULL)
		{
			CvCityAI* pCity=static_cast<CvCityAI*>(getWorkingCity());
			pCity->AI_updateBestBuild();
			pCity->getAIGroup_Worker()->setMissionBuild(NO_BUILD);
			pCity->getAIGroup_Worker()->setMissionPlot(NULL);
		}
	}
	//end added Sephi

	return bFinished;
}


void CvPlot::updateFeatureSymbolVisibility()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	if (m_pFeatureSymbol != NULL)
	{
		bool bVisible = isRevealed(GC.getGameINLINE().getActiveTeam(), true);
		if(getFeatureType() != NO_FEATURE)
		{
			if(GC.getFeatureInfo(getFeatureType()).isVisibleAlways())
				bVisible = true;
		}

		bool wasVisible = !gDLL->getFeatureIFace()->IsHidden(m_pFeatureSymbol);
		if(wasVisible != bVisible)
		{
			gDLL->getFeatureIFace()->Hide(m_pFeatureSymbol, !bVisible);
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
			gDLL->getEngineIFace()->MarkPlotTextureAsDirty(m_iX, m_iY);
//			gDLL->getEngineIFace()->MarkPlotTextureAsDirty(getViewportX(),getViewportY());
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		}
	}
}


void CvPlot::updateFeatureSymbol(bool bForce)
{
	PROFILE_FUNC();

	FeatureTypes eFeature;

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	eFeature = getFeatureType();
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
	gDLL->getEngineIFace()->RebuildTileArt(m_iX,m_iY);
//	gDLL->getEngineIFace()->RebuildTileArt(getViewportX(),getViewportY());
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

	if ((eFeature == NO_FEATURE) ||
		  (GC.getFeatureInfo(eFeature).getArtInfo()->isRiverArt()) ||
		  (GC.getFeatureInfo(eFeature).getArtInfo()->getTileArtType() != TILE_ART_TYPE_NONE)
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//			||(!isRevealed(GC.getGame().getActiveTeam(), true))
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/		  
		  )
	{
		gDLL->getFeatureIFace()->destroy(m_pFeatureSymbol);
		return;
	}

	if (bForce || (m_pFeatureSymbol == NULL) || (gDLL->getFeatureIFace()->getFeature(m_pFeatureSymbol) != eFeature))
	{
		gDLL->getFeatureIFace()->destroy(m_pFeatureSymbol);
		m_pFeatureSymbol = gDLL->getFeatureIFace()->createFeature();

		FAssertMsg(m_pFeatureSymbol != NULL, "m_pFeatureSymbol is not expected to be equal with NULL");

		gDLL->getFeatureIFace()->init(m_pFeatureSymbol, 0, 0, eFeature, this);

		updateFeatureSymbolVisibility();
	}
	else
	{
		gDLL->getEntityIFace()->updatePosition((CvEntity*)m_pFeatureSymbol); //update position and contours
	}
}


CvRoute* CvPlot::getRouteSymbol() const
{
	return m_pRouteSymbol;
}


// XXX route symbols don't really exist anymore...
void CvPlot::updateRouteSymbol(bool bForce, bool bAdjacent)
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;
	RouteTypes eRoute;
	int iI;

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	if (bAdjacent)
	{
		for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

			if (pAdjacentPlot != NULL)
			{
				pAdjacentPlot->updateRouteSymbol(bForce, false);
				//pAdjacentPlot->setLayoutDirty(true);
			}
		}
	}

	eRoute = getRevealedRouteType(GC.getGameINLINE().getActiveTeam(), true);

	if (eRoute == NO_ROUTE)
	{
		gDLL->getRouteIFace()->destroy(m_pRouteSymbol);
		return;
	}

	if (bForce || (m_pRouteSymbol == NULL) || (gDLL->getRouteIFace()->getRoute(m_pRouteSymbol) != eRoute))
	{
		gDLL->getRouteIFace()->destroy(m_pRouteSymbol);
		m_pRouteSymbol = gDLL->getRouteIFace()->createRoute();
		FAssertMsg(m_pRouteSymbol != NULL, "m_pRouteSymbol is not expected to be equal with NULL");

		gDLL->getRouteIFace()->init(m_pRouteSymbol, 0, 0, eRoute, this);
		setLayoutDirty(true);
	}
	else
	{
		gDLL->getEntityIFace()->updatePosition((CvEntity *)m_pRouteSymbol); //update position and contours
	}
}


CvRiver* CvPlot::getRiverSymbol() const
{
	return m_pRiverSymbol;
}


CvFeature* CvPlot::getFeatureSymbol() const
{
	return m_pFeatureSymbol;
}


void CvPlot::updateRiverSymbol(bool bForce, bool bAdjacent)
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	if (bAdjacent)
	{
		for(int i=0;i<NUM_DIRECTION_TYPES;i++)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)i));
			if (pAdjacentPlot != NULL)
			{
				pAdjacentPlot->updateRiverSymbol(bForce, false);
				//pAdjacentPlot->setLayoutDirty(true);
			}
		}
	}

	if (!isRiverMask()
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//			||((!isRevealed(GC.getGame().getActiveTeam(), true) && GC.getCurrentViewport().getSpoofHiddenGraphics() == VIEWPORT_SPOOF_ALL_UNREVEALED)
//				|| !isAdjacentRevealed(GC.getGame().getActiveTeam()))
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/		  		
		)
	{
		gDLL->getRiverIFace()->destroy(m_pRiverSymbol);
		return;
	}

	if (bForce || (m_pRiverSymbol == NULL))
	{
		//create river
		gDLL->getRiverIFace()->destroy(m_pRiverSymbol);
		m_pRiverSymbol = gDLL->getRiverIFace()->createRiver();
		FAssertMsg(m_pRiverSymbol != NULL, "m_pRiverSymbol is not expected to be equal with NULL");
		gDLL->getRiverIFace()->init(m_pRiverSymbol, 0, 0, 0, this);

		//force tree cuts for adjacent plots
		DirectionTypes affectedDirections[] = {NO_DIRECTION, DIRECTION_EAST, DIRECTION_SOUTHEAST, DIRECTION_SOUTH};
		for(int i=0;i<4;i++)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), affectedDirections[i]);
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/

			if (pAdjacentPlot != NULL)
			{
				gDLL->getEngineIFace()->ForceTreeOffsets(pAdjacentPlot->getX(), pAdjacentPlot->getY());
			}
/**
			if (pAdjacentPlot != NULL && pAdjacentPlot->isInViewport())
			{
				gDLL->getEngineIFace()->ForceTreeOffsets(getViewportX(),getViewportY());
			}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
		}

		//cut out canyons
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
		gDLL->getEngineIFace()->RebuildRiverPlotTile(getX_INLINE(), getY_INLINE(), true, false);
//		gDLL->getEngineIFace()->RebuildRiverPlotTile(getViewportX(),getViewportY(), true, false);
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/

		//recontour adjacent rivers
		for(int i=0;i<NUM_DIRECTION_TYPES;i++)
		{
			pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)i));
			if((pAdjacentPlot != NULL) && (pAdjacentPlot->m_pRiverSymbol != NULL))
			{
				gDLL->getEntityIFace()->updatePosition((CvEntity *)pAdjacentPlot->m_pRiverSymbol); //update position and contours
			}
		}

		// update the symbol
		setLayoutDirty(true);
	}

	//recontour rivers
	gDLL->getEntityIFace()->updatePosition((CvEntity *)m_pRiverSymbol); //update position and contours
}


void CvPlot::updateRiverSymbolArt(bool bAdjacent)
{
	//this is used to update floodplain features
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
	gDLL->getEntityIFace()->setupFloodPlains(m_pRiverSymbol);
/**
	if (isRevealed(GC.getGame().getActiveTeam(), true) )
	{
		gDLL->getEntityIFace()->setupFloodPlains(m_pRiverSymbol);
	}
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/


	if(bAdjacent)
	{
		for(int i=0;i<NUM_DIRECTION_TYPES;i++)
		{
			CvPlot *pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), (DirectionTypes) i);
			if((pAdjacentPlot != NULL) && (pAdjacentPlot->m_pRiverSymbol != NULL)
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//				&& (pAdjacentPlot->isRevealed(GC.getGame().getActiveTeam(), true))
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/				
				)
			{
				gDLL->getEntityIFace()->setupFloodPlains(pAdjacentPlot->m_pRiverSymbol);
			}
		}
	}
}


CvFlagEntity* CvPlot::getFlagSymbol() const
{
	return m_pFlagSymbol;
}

CvFlagEntity* CvPlot::getFlagSymbolOffset() const
{
	return m_pFlagSymbolOffset;
}

void CvPlot::updateFlagSymbol()
{
	PROFILE_FUNC();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		return;
	}

	PlayerTypes ePlayer = NO_PLAYER;
	PlayerTypes ePlayerOffset = NO_PLAYER;

	CvUnit* pCenterUnit = getCenterUnit();

	//get the plot's unit's flag
	if (pCenterUnit != NULL)
	{
		ePlayer = pCenterUnit->getVisualOwner();
	}

	//get moving unit's flag
	if (gDLL->getInterfaceIFace()->getSingleMoveGotoPlot() == this)
	{
		if(ePlayer == NO_PLAYER)
		{
			ePlayer = GC.getGameINLINE().getActivePlayer();
		}
		else
		{
			ePlayerOffset = GC.getGameINLINE().getActivePlayer();
		}
	}

	//don't put two of the same flags
	if(ePlayerOffset == ePlayer)
	{
		ePlayerOffset = NO_PLAYER;
	}

	//destroy old flags
	if (ePlayer == NO_PLAYER)
	{
		gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbol);
	}
	if (ePlayerOffset == NO_PLAYER)
	{
		gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbolOffset);
	}

	//create and/or update unit's flag
	if (ePlayer != NO_PLAYER)
	{
		if ((m_pFlagSymbol == NULL) || (gDLL->getFlagEntityIFace()->getPlayer(m_pFlagSymbol) != ePlayer))
		{
			if (m_pFlagSymbol != NULL)
			{
				gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbol);
			}
			m_pFlagSymbol = gDLL->getFlagEntityIFace()->create(ePlayer);
			if (m_pFlagSymbol != NULL)
			{
				gDLL->getFlagEntityIFace()->setPlot(m_pFlagSymbol, this, false);
			}
		}

		if (m_pFlagSymbol != NULL)
		{
			gDLL->getFlagEntityIFace()->updateUnitInfo(m_pFlagSymbol, this, false);
		}
	}

	//create and/or update offset flag
	if (ePlayerOffset != NO_PLAYER)
	{
		if ((m_pFlagSymbolOffset == NULL) || (gDLL->getFlagEntityIFace()->getPlayer(m_pFlagSymbolOffset) != ePlayerOffset))
		{
			if (m_pFlagSymbolOffset != NULL)
			{
				gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbolOffset);
			}
			m_pFlagSymbolOffset = gDLL->getFlagEntityIFace()->create(ePlayerOffset);
			if (m_pFlagSymbolOffset != NULL)
			{
				gDLL->getFlagEntityIFace()->setPlot(m_pFlagSymbolOffset, this, true);
			}
		}

		if (m_pFlagSymbolOffset != NULL)
		{
			gDLL->getFlagEntityIFace()->updateUnitInfo(m_pFlagSymbolOffset, this, true);
		}
	}
}


CvUnit* CvPlot::getCenterUnit() const
{
	return m_pCenterUnit;
}


CvUnit* CvPlot::getDebugCenterUnit() const
{
	CvUnit* pCenterUnit;

	pCenterUnit = getCenterUnit();

	if (pCenterUnit == NULL)
	{
		if (GC.getGameINLINE().isDebugMode())
		{
			CLLNode<IDInfo>* pUnitNode = headUnitNode();
			if(pUnitNode == NULL)
				pCenterUnit = NULL;
			else
				pCenterUnit = ::getUnit(pUnitNode->m_data);
		}
	}

	return pCenterUnit;
}


void CvPlot::setCenterUnit(CvUnit* pNewValue)
{
	CvUnit* pOldValue;

	pOldValue = getCenterUnit();

	if (pOldValue != pNewValue)
	{
		m_pCenterUnit = pNewValue;
		updateMinimapColor();

		setFlagDirty(true);

		if (getCenterUnit() != NULL)
		{
			getCenterUnit()->setInfoBarDirty(true);
		}
	}
}


int CvPlot::getCultureRangeCities(PlayerTypes eOwnerIndex, int iRangeIndex) const
{
	FAssert(eOwnerIndex >= 0);
	FAssert(eOwnerIndex < MAX_PLAYERS);
	FAssert(iRangeIndex >= 0);
	FAssert(iRangeIndex < GC.getNumCultureLevelInfos());

	if (NULL == m_apaiCultureRangeCities)
	{
		return 0;
	}
	else if (NULL == m_apaiCultureRangeCities[eOwnerIndex])
	{
		return 0;
	}

	return m_apaiCultureRangeCities[eOwnerIndex][iRangeIndex];
}


bool CvPlot::isCultureRangeCity(PlayerTypes eOwnerIndex, int iRangeIndex) const
{
	return (getCultureRangeCities(eOwnerIndex, iRangeIndex) > 0);
}


void CvPlot::changeCultureRangeCities(PlayerTypes eOwnerIndex, int iRangeIndex, int iChange, bool bUpdatePlotGroups)
{
	bool bOldCultureRangeCities;

	FAssert(eOwnerIndex >= 0);
	FAssert(eOwnerIndex < MAX_PLAYERS);
	FAssert(iRangeIndex >= 0);
	FAssert(iRangeIndex < GC.getNumCultureLevelInfos());

	if (0 != iChange)
	{
		bOldCultureRangeCities = isCultureRangeCity(eOwnerIndex, iRangeIndex);

		if (NULL == m_apaiCultureRangeCities)
		{
			m_apaiCultureRangeCities = new char*[MAX_PLAYERS];
			for (int iI = 0; iI < MAX_PLAYERS; ++iI)
			{
				m_apaiCultureRangeCities[iI] = NULL;
			}
		}

		if (NULL == m_apaiCultureRangeCities[eOwnerIndex])
		{
			m_apaiCultureRangeCities[eOwnerIndex] = new char[GC.getNumCultureLevelInfos()];
			for (int iI = 0; iI < GC.getNumCultureLevelInfos(); ++iI)
			{
				m_apaiCultureRangeCities[eOwnerIndex][iI] = 0;
			}
		}

		m_apaiCultureRangeCities[eOwnerIndex][iRangeIndex] += iChange;

		if (bOldCultureRangeCities != isCultureRangeCity(eOwnerIndex, iRangeIndex))
		{
			updateCulture(true, bUpdatePlotGroups);
		}
	}
}


int CvPlot::getInvisibleVisibilityCount(TeamTypes eTeam, InvisibleTypes eInvisible) const
{
	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eInvisible >= 0, "eInvisible is expected to be non-negative (invalid Index)");
	FAssertMsg(eInvisible < GC.getNumInvisibleInfos(), "eInvisible is expected to be within maximum bounds (invalid Index)");

	if (NULL == m_apaiInvisibleVisibilityCount)
	{
		return 0;
	}
	else if (NULL == m_apaiInvisibleVisibilityCount[eTeam])
	{
		return 0;
	}

	return m_apaiInvisibleVisibilityCount[eTeam][eInvisible];
}


bool CvPlot::isInvisibleVisible(TeamTypes eTeam, InvisibleTypes eInvisible)	const
{
	return (getInvisibleVisibilityCount(eTeam, eInvisible) > 0);
}


void CvPlot::changeInvisibleVisibilityCount(TeamTypes eTeam, InvisibleTypes eInvisible, int iChange)
{
	bool bOldInvisibleVisible;

	FAssertMsg(eTeam >= 0, "eTeam is expected to be non-negative (invalid Index)");
	FAssertMsg(eTeam < MAX_TEAMS, "eTeam is expected to be within maximum bounds (invalid Index)");
	FAssertMsg(eInvisible >= 0, "eInvisible is expected to be non-negative (invalid Index)");
	FAssertMsg(eInvisible < GC.getNumInvisibleInfos(), "eInvisible is expected to be within maximum bounds (invalid Index)");

	if (iChange != 0)
	{
		bOldInvisibleVisible = isInvisibleVisible(eTeam, eInvisible);

		if (NULL == m_apaiInvisibleVisibilityCount)
		{
			m_apaiInvisibleVisibilityCount = new short*[MAX_TEAMS];
			for (int iI = 0; iI < MAX_TEAMS; ++iI)
			{
				m_apaiInvisibleVisibilityCount[iI] = NULL;
			}
		}

		if (NULL == m_apaiInvisibleVisibilityCount[eTeam])
		{
			m_apaiInvisibleVisibilityCount[eTeam] = new short[GC.getNumInvisibleInfos()];
			for (int iI = 0; iI < GC.getNumInvisibleInfos(); ++iI)
			{
				m_apaiInvisibleVisibilityCount[eTeam][iI] = 0;
			}
		}

		m_apaiInvisibleVisibilityCount[eTeam][eInvisible] += iChange;

		if (bOldInvisibleVisible != isInvisibleVisible(eTeam, eInvisible))
		{
			if (eTeam == GC.getGameINLINE().getActiveTeam())
			{
				updateCenterUnit();
			}
		}
	}
}


int CvPlot::getNumUnits() const
{
	return m_units.getLength();
}


CvUnit* CvPlot::getUnitByIndex(int iIndex) const
{
	CLLNode<IDInfo>* pUnitNode;

	pUnitNode = m_units.nodeNum(iIndex);

	if (pUnitNode != NULL)
	{
		return ::getUnit(pUnitNode->m_data);
	}
	else
	{
		return NULL;
	}
}


void CvPlot::addUnit(CvUnit* pUnit, bool bUpdate)
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	FAssertMsg(pUnit->at(getX_INLINE(), getY_INLINE()), "pUnit is expected to be at getX_INLINE and getY_INLINE");

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);

		if (!isBeforeUnitCycle(pLoopUnit, pUnit))
		{
			break;
		}

		pUnitNode = nextUnitNode(pUnitNode);
	}

	if (pUnitNode != NULL)
	{
		m_units.insertBefore(pUnit->getIDInfo(), pUnitNode);
	}
	else
	{
		m_units.insertAtEnd(pUnit->getIDInfo());
	}

	if (bUpdate)
	{
		updateCenterUnit();

		setFlagDirty(true);
	}
}


void CvPlot::removeUnit(CvUnit* pUnit, bool bUpdate)
{
	CLLNode<IDInfo>* pUnitNode;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		if (::getUnit(pUnitNode->m_data) == pUnit)
		{
			FAssertMsg(::getUnit(pUnitNode->m_data)->at(getX_INLINE(), getY_INLINE()), "The current unit instance is expected to be at getX_INLINE and getY_INLINE");
			m_units.deleteNode(pUnitNode);
			break;
		}
		else
		{
			pUnitNode = nextUnitNode(pUnitNode);
		}
	}

	if (bUpdate)
	{
		updateCenterUnit();

		setFlagDirty(true);
	}
}


CLLNode<IDInfo>* CvPlot::nextUnitNode(CLLNode<IDInfo>* pNode) const
{
	return m_units.next(pNode);
}


CLLNode<IDInfo>* CvPlot::prevUnitNode(CLLNode<IDInfo>* pNode) const
{
	return m_units.prev(pNode);
}


CLLNode<IDInfo>* CvPlot::headUnitNode() const
{
	return m_units.head();
}


CLLNode<IDInfo>* CvPlot::tailUnitNode() const
{
	return m_units.tail();
}


int CvPlot::getNumSymbols() const
{
	return m_symbols.size();
}


CvSymbol* CvPlot::getSymbol(int iID) const
{
	return m_symbols[iID];
}


CvSymbol* CvPlot::addSymbol()
{
	CvSymbol* pSym=gDLL->getSymbolIFace()->createSymbol();
	m_symbols.push_back(pSym);
	/* Fourth Yield (Asaf) - Start */
	m_symbolsOffsetModes.push_back(m_eCurSymbolRenderOffsetMode);
	/* Fourth Yield (Asaf) - End */
	return pSym;
}


void CvPlot::deleteSymbol(int iID)
{
	m_symbols.erase(m_symbols.begin()+iID);
	/* Fourth Yield (Asaf) - Start */
	m_symbolsOffsetModes.erase(m_symbolsOffsetModes.begin()+iID);
	/* Fourth Yield (Asaf) - End */
}


void CvPlot::deleteAllSymbols()
{
	int i;
	for(i=0;i<getNumSymbols();i++)
	{
		gDLL->getSymbolIFace()->destroy(m_symbols[i]);
	}
	m_symbols.clear();
}

CvString CvPlot::getScriptData() const
{
	return m_szScriptData;
}

void CvPlot::setScriptData(const char* szNewValue)
{
	SAFE_DELETE_ARRAY(m_szScriptData);
	m_szScriptData = _strdup(szNewValue);
}

// Protected Functions...

void CvPlot::doFeature()
{
	PROFILE("CvPlot::doFeature()")

	CvCity* pCity;
	CvPlot* pLoopPlot;
	CvWString szBuffer;
	int iProbability;
	int iI, iJ;

	if (getFeatureType() != NO_FEATURE)
	{

//FfH: Added by Kael 03/20/2008
        if (GC.getFeatureInfo(getFeatureType()).getFeatureUpgrade() != NO_FEATURE)
        {
            if (GC.getFeatureInfo((FeatureTypes)GC.getFeatureInfo(getFeatureType()).getFeatureUpgrade()).getPrereqStateReligion() == NO_RELIGION
              || isOwned() && GC.getFeatureInfo((FeatureTypes)GC.getFeatureInfo(getFeatureType()).getFeatureUpgrade()).getPrereqStateReligion() == GET_PLAYER(getOwnerINLINE()).getStateReligion())
            {
/************************************************************************************************/
/* UNOFFICIAL_PATCH Merged Sephi          10/22/09                                jdog5000      */
/*                                                                                              */
/* Gamespeed scaling                                                                            */
/************************************************************************************************/
/* original bts code
                if (GC.getGameINLINE().getSorenRandNum(100, "Feature Upgrade") < GC.getDefineINT("FEATURE_UPGRADE_CHANCE"))
*/
				int iOdds = (100*GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getFeatureProductionPercent());
				iOdds/=100;
                if (GC.getGameINLINE().getSorenRandNum(iOdds, "Feature Upgrade") < GC.getDefineINT("FEATURE_UPGRADE_CHANCE"))
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
                {
                    setFeatureType((FeatureTypes)GC.getFeatureInfo(getFeatureType()).getFeatureUpgrade());
                }
            }
        }
        if (GC.getDefineINT("FLAMES_FEATURE") != -1  && GC.getDefineINT("FLAMES_SPREAD_EFFECT") != -1)
        {
            if (getFeatureType() == GC.getDefineINT("FLAMES_FEATURE"))
            {
                if (GC.getGameINLINE().getSorenRandNum(100, "Flames Spread") < GC.getDefineINT("FLAMES_SPREAD_CHANCE"))
                {
                    CvPlot* pAdjacentPlot;
                    for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
                    {
                        pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));
                        if (pAdjacentPlot != NULL)
                        {
                            if (pAdjacentPlot->getFeatureType() != NO_FEATURE)
                            {
                                if (GC.getFeatureInfo((FeatureTypes)pAdjacentPlot->getFeatureType()).isFlammable())
                                {
                                    if (pAdjacentPlot->getImprovementType() == NO_IMPROVEMENT)
                                    {
                                        pAdjacentPlot->setImprovementType((ImprovementTypes)GC.getDefineINT("FLAMES_SPREAD_EFFECT"));
                                    }
                                }
                            }
                        }
                    }
                }
                if (!GC.getFeatureInfo((FeatureTypes)GC.getDefineINT("FLAMES_FEATURE")).isTerrain(getTerrainType()))
                {
                    if (GC.getGameINLINE().getSorenRandNum(100, "Flames Spread") < GC.getDefineINT("FLAMES_EXPIRE_CHANCE"))
                    {
                        setFeatureType(NO_FEATURE);
                        if (GC.getDefineINT("FLAMES_EXPIRE_EFFECT") != -1)
                        {
                            if (canHaveFeature((FeatureTypes)GC.getDefineINT("FLAMES_EXPIRE_EFFECT")))
                            {
                                setFeatureType((FeatureTypes)GC.getDefineINT("FLAMES_EXPIRE_EFFECT"), -1);
                            }
                        }
                        if (getFeatureType() == NO_FEATURE)
                        {
                            return;
                        }
                    }
                }
            }
        }
//FfH: End Add

		iProbability = GC.getFeatureInfo(getFeatureType()).getDisappearanceProbability();
		if (iProbability > 0)
		{
/************************************************************************************************/
/* UNOFFICIAL_PATCH merged Sephi          10/22/09                                jdog5000      */
/*                                                                                              */
/* Gamespeed scaling                                                                            */
/************************************************************************************************/
/* original bts code
			if (GC.getGameINLINE().getSorenRandNum(10000, "Feature Disappearance") < iProbability)
*/
			int iOdds = (10000*GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getFeatureProductionPercent());
			iOdds/=100;

			if (GC.getGameINLINE().getSorenRandNum(iOdds, "Feature Disappearance") < iProbability)
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/
			{
				setFeatureType(NO_FEATURE);
			}
		}
	}
	else
	{
		if (!isUnit())
		{
			if (getImprovementType() == NO_IMPROVEMENT)
			{
				for (iI = 0; iI < GC.getNumFeatureInfos(); ++iI)
				{
					if (canHaveFeature((FeatureTypes)iI))
					{
						if ((getBonusType() == NO_BONUS) || (GC.getBonusInfo(getBonusType()).isFeature(iI)))
						{
							iProbability = 0;

							for (iJ = 0; iJ < NUM_CARDINALDIRECTION_TYPES; iJ++)
							{
								pLoopPlot = plotCardinalDirection(getX_INLINE(), getY_INLINE(), ((CardinalDirectionTypes)iJ));

								if (pLoopPlot != NULL)
								{
									if (pLoopPlot->getFeatureType() == ((FeatureTypes)iI))
									{
										if (pLoopPlot->getImprovementType() == NO_IMPROVEMENT)
										{
											iProbability += GC.getFeatureInfo((FeatureTypes)iI).getGrowthProbability();
										}
										else
										{
											iProbability += GC.getImprovementInfo(pLoopPlot->getImprovementType()).getFeatureGrowthProbability();
										}
									}
								}
							}

							iProbability *= std::max(0, (GC.getFEATURE_GROWTH_MODIFIER() + 100));
							iProbability /= 100;

							if (isRoute())
							{
								iProbability *= std::max(0, (GC.getROUTE_FEATURE_GROWTH_MODIFIER() + 100));
								iProbability /= 100;
							}

							if (iProbability > 0)
							{
/************************************************************************************************/
/* UNOFFICIAL_PATCH merged Sephi          10/22/09                                jdog5000      */
/*                                                                                              */
/* Gamespeed scaling                                                                            */
/************************************************************************************************/
/* original bts code
								if (GC.getGameINLINE().getSorenRandNum(10000, "Feature Growth") < iProbability)
*/
								int iOdds = (10000*GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getFeatureProductionPercent());
								iOdds/=100;

								if( GC.getGameINLINE().getSorenRandNum(iOdds, "Feature Growth") < iProbability )
/************************************************************************************************/
/* UNOFFICIAL_PATCH                        END                                                  */
/************************************************************************************************/

								{
									setFeatureType((FeatureTypes)iI);

									pCity = GC.getMapINLINE().findCity(getX_INLINE(), getY_INLINE(), getOwnerINLINE(), NO_TEAM, false);

									if (pCity != NULL)
									{
										// Tell the owner of this city.
										szBuffer = gDLL->getText("TXT_KEY_MISC_FEATURE_GROWN_NEAR_CITY", GC.getFeatureInfo((FeatureTypes) iI).getTextKeyWide(), pCity->getNameKey());
										gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_FEATUREGROWTH", MESSAGE_TYPE_INFO, GC.getFeatureInfo((FeatureTypes) iI).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE"), getX_INLINE(), getY_INLINE(), true, true);
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


void CvPlot::doCulture()
{
	PROFILE("CvPlot::doCulture()")

	CLLNode<IDInfo>* pUnitNode;
	CvCity* pCity;
	CvUnit* pLoopUnit;
	CvWString szBuffer;
	PlayerTypes eCulturalOwner;
	int iGarrison;
	int iCityStrength;

	// Super Forts begin *culture*
	if(GC.getGameINLINE().isOption(GAMEOPTION_SUPER_FORTS) /*&& getOwnerINLINE() < BARBARIAN_PLAYER*/)
	{
		doImprovementCulture();

		ImprovementTypes eImprovement = getImprovementType();
		if(eImprovement != NO_IMPROVEMENT)
		{
			// Check for a fort culture flip
			if(GC.getImprovementInfo(eImprovement).getCulture() > 0 /*.isActsAsCity()*/ && (getOwnershipDuration() > GC.getDefineINT("SUPER_FORTS_DURATION_BEFORE_REVOLT")))
			{
				eCulturalOwner = calculateCulturalOwner();
				if(eCulturalOwner != NO_PLAYER)
				{
					if(GET_PLAYER(eCulturalOwner).getTeam() != getTeam())
					{
						bool bDefenderFound = false;
						CLinkList<IDInfo> oldUnits;
						pUnitNode = headUnitNode();

						while (pUnitNode != NULL)
						{
							oldUnits.insertAtEnd(pUnitNode->m_data);
							pUnitNode = nextUnitNode(pUnitNode);
						}

						pUnitNode = oldUnits.head();

						while (pUnitNode != NULL)
						{
							pLoopUnit = ::getUnit(pUnitNode->m_data);
							pUnitNode = nextUnitNode(pUnitNode);
							if(pLoopUnit->canDefend(this))
							{
								if(pLoopUnit->getOwner() == getOwnerINLINE())
								{
									bDefenderFound = true;
									break;
								}
							}
						}
						if(!bDefenderFound)
						{
							szBuffer = gDLL->getText("TXT_KEY_MISC_CITY_REVOLTED_JOINED", GC.getImprovementInfo(getImprovementType()).getText(), GET_PLAYER(eCulturalOwner).getCivilizationDescriptionKey());
							gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTUREFLIP", MESSAGE_TYPE_INFO, GC.getImprovementInfo(getImprovementType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
							gDLL->getInterfaceIFace()->addMessage(eCulturalOwner, false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CULTUREFLIP", MESSAGE_TYPE_INFO, GC.getImprovementInfo(getImprovementType()).getButton(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
							setOwner(eCulturalOwner,true,true);
						}
					}
				}
			}
		}
	}
	// Super Forts end

	pCity = getPlotCity();

	if (pCity != NULL)
	{
		eCulturalOwner = calculateCulturalOwner();

		if (eCulturalOwner != NO_PLAYER)
		{
			if (GET_PLAYER(eCulturalOwner).getTeam() != getTeam())
			{
				if (!(pCity->isOccupation()))
				{
					if (GC.getGameINLINE().getSorenRandNum(100, "Revolt #1") < pCity->getRevoltTestProbability())
					{
						iCityStrength = pCity->cultureStrength(eCulturalOwner);
						iGarrison = pCity->cultureGarrison(eCulturalOwner);
/*************************************************************************************************/
/**	ADDON (Barbarian Cityflip) Sephi                       							12/22/08	**/
/**																								**/
/**						Allows CultureGarrison for Barbs                                        **/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
						if ((GC.getGameINLINE().getSorenRandNum(iCityStrength, "Revolt #2") > iGarrison) || pCity->isBarbarian())
/**								----  End Original Code  ----									**/
						if ((GC.getGameINLINE().getSorenRandNum(iCityStrength, "Revolt #2") > iGarrison))
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/
						{
							CLinkList<IDInfo> oldUnits;

							pUnitNode = headUnitNode();

							while (pUnitNode != NULL)
							{
								oldUnits.insertAtEnd(pUnitNode->m_data);
								pUnitNode = nextUnitNode(pUnitNode);
							}

							pUnitNode = oldUnits.head();

							while (pUnitNode != NULL)
							{
								pLoopUnit = ::getUnit(pUnitNode->m_data);
								pUnitNode = nextUnitNode(pUnitNode);

								if (pLoopUnit)
								{
									if (pLoopUnit->isBarbarian())
									{
/*************************************************************************************************/
/**	ADDON (Barbarian Cityflip) merged Sephi Xienwolf Tweak							12/22/08	**/
/**																								**/
/**						Prevents killing of Barbarian Heroes and Equipment						**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
										pLoopUnit->kill(false, eCulturalOwner);
/**								----  End Original Code  ----									**/
										pLoopUnit->jumpToNearestValidPlot();
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/
									}
									else if (pLoopUnit->canDefend())
									{
										pLoopUnit->changeDamage((pLoopUnit->currHitPoints() / 2), eCulturalOwner);
									}
								}

							}

							if (pCity->isBarbarian() || ((!(pCity->isEverOwned(eCulturalOwner))) && (pCity->getNumRevolts(eCulturalOwner) >= GC.getDefineINT("NUM_WARNING_REVOLTS"))))
							{
								if (GC.getGameINLINE().isOption(GAMEOPTION_ONE_CITY_CHALLENGE) && GET_PLAYER(eCulturalOwner).isHuman())
								{
									pCity->kill(true);
								}
								else
								{
									setOwner(eCulturalOwner, true, true); // will delete pCity
								}
								pCity = NULL;
							}
							else
							{
								pCity->changeNumRevolts(eCulturalOwner, 1);
								pCity->changeOccupationTimer(GC.getDefineINT("BASE_REVOLT_OCCUPATION_TURNS") + ((iCityStrength * GC.getDefineINT("REVOLT_OCCUPATION_TURNS_PERCENT")) / 100));

								// XXX announce for all seen cities?
								szBuffer = gDLL->getText("TXT_KEY_MISC_REVOLT_IN_CITY", GET_PLAYER(eCulturalOwner).getCivilizationAdjective(), pCity->getNameKey());
								gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(), false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITY_REVOLT", MESSAGE_TYPE_MINOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_RESISTANCE")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_RED"), getX_INLINE(), getY_INLINE(), true, true);
								gDLL->getInterfaceIFace()->addMessage(eCulturalOwner, false, GC.getEVENT_MESSAGE_TIME(), szBuffer, "AS2D_CITY_REVOLT", MESSAGE_TYPE_MINOR_EVENT, ARTFILEMGR.getInterfaceArtInfo("INTERFACE_RESISTANCE")->getPath(), (ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"), getX_INLINE(), getY_INLINE(), true, true);
							}
						}
					}
				}
			}
		}
	}

	updateCulture(true, true);
}


void CvPlot::processArea(CvArea* pArea, int iChange)
{
	CvCity* pCity;
	int iI, iJ;

	// XXX am not updating getBestFoundValue() or getAreaAIType()...

	pArea->changeNumTiles(iChange);

	if (isOwned())
	{
		pArea->changeNumOwnedTiles(iChange);
	}

	if (isNOfRiver())
	{
		pArea->changeNumRiverEdges(iChange);
	}
	if (isWOfRiver())
	{
		pArea->changeNumRiverEdges(iChange);
	}

	if (getBonusType() != NO_BONUS)
	{
		pArea->changeNumBonuses(getBonusType(), iChange);
	}

	if (getImprovementType() != NO_IMPROVEMENT)
	{
		pArea->changeNumImprovements(getImprovementType(), iChange);
	}

	for (iI = 0; iI < MAX_PLAYERS; ++iI)
	{
		if (GET_PLAYER((PlayerTypes)iI).getStartingPlot() == this)
		{
			pArea->changeNumStartingPlots(iChange);
		}

		pArea->changePower(((PlayerTypes)iI), (getUnitPower((PlayerTypes)iI) * iChange));

		pArea->changeUnitsPerPlayer(((PlayerTypes)iI), (plotCount(PUF_isPlayer, iI) * iChange));
		pArea->changeAnimalsPerPlayer(((PlayerTypes)iI), (plotCount(PUF_isAnimal, -1, -1, ((PlayerTypes)iI)) * iChange));

		for (iJ = 0; iJ < NUM_UNITAI_TYPES; iJ++)
		{
			pArea->changeNumAIUnits(((PlayerTypes)iI), ((UnitAITypes)iJ), (plotCount(PUF_isUnitAIType, iJ, -1, ((PlayerTypes)iI)) * iChange));
		}
	}

	for (iI = 0; iI < MAX_TEAMS; ++iI)
	{
		if (isRevealed(((TeamTypes)iI), false))
		{
			pArea->changeNumRevealedTiles(((TeamTypes)iI), iChange);
		}
	}

	pCity = getPlotCity();

	if (pCity != NULL)
	{
		// XXX make sure all of this (esp. the changePower()) syncs up...
		pArea->changePower(pCity->getOwnerINLINE(), (getPopulationPower(pCity->getPopulation()) * iChange));

		pArea->changeCitiesPerPlayer(pCity->getOwnerINLINE(), iChange);
		pArea->changePopulationPerPlayer(pCity->getOwnerINLINE(), (pCity->getPopulation() * iChange));

		for (iI = 0; iI < GC.getNumBuildingInfos(); ++iI)
		{
			if (pCity->getNumActiveBuilding((BuildingTypes)iI) > 0)
			{
				pArea->changePower(pCity->getOwnerINLINE(), (GC.getBuildingInfo((BuildingTypes)iI).getPowerValue() * iChange * pCity->getNumActiveBuilding((BuildingTypes)iI)));

				if (GC.getBuildingInfo((BuildingTypes) iI).getAreaHealth() > 0)
				{
					pArea->changeBuildingGoodHealth(pCity->getOwnerINLINE(), (GC.getBuildingInfo((BuildingTypes)iI).getAreaHealth() * iChange * pCity->getNumActiveBuilding((BuildingTypes)iI)));
				}
				else
				{
					pArea->changeBuildingBadHealth(pCity->getOwnerINLINE(), (GC.getBuildingInfo((BuildingTypes)iI).getAreaHealth() * iChange * pCity->getNumActiveBuilding((BuildingTypes)iI)));
				}
				pArea->changeBuildingHappiness(pCity->getOwnerINLINE(), (GC.getBuildingInfo((BuildingTypes)iI).getAreaHappiness() * iChange * pCity->getNumActiveBuilding((BuildingTypes)iI)));
				pArea->changeFreeSpecialist(pCity->getOwnerINLINE(), (GC.getBuildingInfo((BuildingTypes)iI).getAreaFreeSpecialist() * iChange * pCity->getNumActiveBuilding((BuildingTypes)iI)));

				pArea->changeCleanPowerCount(pCity->getTeam(), ((GC.getBuildingInfo((BuildingTypes)iI).isAreaCleanPower()) ? iChange * pCity->getNumActiveBuilding((BuildingTypes)iI) : 0));

				pArea->changeBorderObstacleCount(pCity->getTeam(), ((GC.getBuildingInfo((BuildingTypes)iI).isAreaBorderObstacle()) ? iChange * pCity->getNumActiveBuilding((BuildingTypes)iI) : 0));

				for (iJ = 0; iJ < NUM_YIELD_TYPES; iJ++)
				{
					pArea->changeYieldRateModifier(pCity->getOwnerINLINE(), ((YieldTypes)iJ), (GC.getBuildingInfo((BuildingTypes)iI).getAreaYieldModifier(iJ) * iChange * pCity->getNumActiveBuilding((BuildingTypes)iI)));
				}
			}
		}

		for (iI = 0; iI < NUM_UNITAI_TYPES; ++iI)
		{
			pArea->changeNumTrainAIUnits(pCity->getOwnerINLINE(), ((UnitAITypes)iI), (pCity->getNumTrainUnitAI((UnitAITypes)iI) * iChange));
		}

		for (iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			if (pArea->getTargetCity((PlayerTypes)iI) == pCity)
			{
				pArea->setTargetCity(((PlayerTypes)iI), NULL);
			}
		}
	}
}


ColorTypes CvPlot::plotMinimapColor()
{
	CvUnit* pCenterUnit;

	if (GC.getGameINLINE().getActivePlayer() != NO_PLAYER)
	{
		CvCity* pCity;

		pCity = getPlotCity();

		if ((pCity != NULL) && pCity->isRevealed(GC.getGameINLINE().getActiveTeam(), true))
		{
			return (ColorTypes)GC.getInfoTypeForString("COLOR_WHITE");
		}

		if (isActiveVisible(true))
		{
			pCenterUnit = getDebugCenterUnit();

			if (pCenterUnit != NULL)
			{
				return ((ColorTypes)(GC.getPlayerColorInfo(GET_PLAYER(pCenterUnit->getVisualOwner()).getPlayerColor()).getColorTypePrimary()));
			}
		}

		if ((getRevealedOwner(GC.getGameINLINE().getActiveTeam(), true) != NO_PLAYER) && !isRevealedBarbarian())
		{
			return ((ColorTypes)(GC.getPlayerColorInfo(GET_PLAYER(getRevealedOwner(GC.getGameINLINE().getActiveTeam(), true)).getPlayerColor()).getColorTypePrimary()));
		}
	}

	return (ColorTypes)GC.getInfoTypeForString("COLOR_CLEAR");
}

//
// read object from a stream
// used during load
//
void CvPlot::read(FDataStreamBase* pStream)
{
	int iI;
	bool bVal;
	char cCount;
	int iCount;

	// Init saved data
	reset();

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iX);
	pStream->Read(&m_iY);
	pStream->Read(&m_iArea);
	// m_pPlotArea not saved
	pStream->Read(&m_iFeatureVariety);
	pStream->Read(&m_iOwnershipDuration);
	pStream->Read(&m_iImprovementDuration);
	pStream->Read(&m_iUpgradeProgress);
	pStream->Read(&m_iForceUnownedTimer);
	pStream->Read(&m_iCityRadiusCount);
	pStream->Read(&m_iRiverID);
	pStream->Read(&m_iMinOriginalStartDist);
	pStream->Read(&m_iReconCount);
	pStream->Read(&m_iRiverCrossingCount);

	// Super Forts begin *canal* *choke*
	pStream->Read(&m_iCanalValue);
	pStream->Read(&m_iChokeValue);
	// Super Forts end
	// Super Forts begin *bombard*
	pStream->Read(&m_iDefenseDamage);
	pStream->Read(&m_bBombarded);
	// Super Forts end
	
	pStream->Read(&bVal);
	m_bStartingPlot = bVal;
	pStream->Read(&bVal);
	m_bHills = bVal;
	pStream->Read(&bVal);
	m_bNOfRiver = bVal;
	pStream->Read(&bVal);
	m_bWOfRiver = bVal;
	pStream->Read(&bVal);
	m_bIrrigated = bVal;
	pStream->Read(&bVal);
	m_bPotentialCityWork = bVal;
	// m_bShowCitySymbols not saved
	// m_bFlagDirty not saved
	// m_bPlotLayoutDirty not saved
	// m_bLayoutStateWorked not saved

	pStream->Read(&m_eOwner);
	pStream->Read(&m_ePlotType);
	pStream->Read(&m_eTerrainType);
	pStream->Read(&m_eFeatureType);
	pStream->Read(&m_eBonusType);
	pStream->Read(&m_eImprovementType);
	pStream->Read(&m_eRouteType);
	pStream->Read(&m_eRiverNSDirection);
	pStream->Read(&m_eRiverWEDirection);

	pStream->Read((int*)&m_plotCity.eOwner);
	pStream->Read(&m_plotCity.iID);
	pStream->Read((int*)&m_workingCity.eOwner);
	pStream->Read(&m_workingCity.iID);
	pStream->Read((int*)&m_workingCityOverride.eOwner);
	pStream->Read(&m_workingCityOverride.iID);

	pStream->Read(NUM_YIELD_TYPES, m_aiYield);

//FfH: Added by Kael 08/15/2007
	pStream->Read(&m_bMoveDisabledAI);
	pStream->Read(&m_bMoveDisabledHuman);
	pStream->Read(&m_bBuildDisabled);
	pStream->Read(&m_bFoundDisabled);
	pStream->Read(&m_bPythonActive);
	pStream->Read(&m_eRealTerrainType);
	pStream->Read(&m_iMinLevel);
	pStream->Read(&m_iPlotCounter);
	pStream->Read(&m_iPortalExitX);
	pStream->Read(&m_iPortalExitY);
	pStream->Read(&m_iTempTerrainTimer);
//FfH: End Add

	pStream->Read(&m_iDungeonType);
	pStream->Read(&m_iWilderness);
	pStream->Read(&m_iPlane);
	pStream->Read(&m_iPlundered);
	pStream->Read(&m_bWildmanaImmune);
	pStream->Read(&m_bNonPlanar);
	pStream->Read(&m_iAnimalDensity);
	pStream->Read(&m_iAnimalDensityPlusLairs);
	pStream->Read(&m_iBarBarianDensity);
	pStream->Read(&m_iBarBarianDensityPlusLairs);
	pStream->Read(&m_iDungeon);
	pStream->Read(&m_iPortalExit);

	//pathfinding
	//pStream->Read(&m_iRegion);
	//pStream->Read(&m_iSector);

	SAFE_DELETE_ARRAY(m_aiCulture);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiCulture = new int[cCount];
		pStream->Read(cCount, m_aiCulture);
	}

	// Super Forts begin *culture*
	SAFE_DELETE_ARRAY(m_aiCultureRangeForts);
	pStream->Read(&cCount);
	if(cCount > 0)
	{
		m_aiCultureRangeForts = new short[cCount];
		pStream->Read(cCount, m_aiCultureRangeForts);
	}
	// Super Forts end

	SAFE_DELETE_ARRAY(m_aiFoundValue);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiFoundValue = new short[cCount];
		pStream->Read(cCount, m_aiFoundValue);
	}

	SAFE_DELETE_ARRAY(m_aiPlayerCityRadiusCount);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiPlayerCityRadiusCount = new char[cCount];
		pStream->Read(cCount, m_aiPlayerCityRadiusCount);
	}

	SAFE_DELETE_ARRAY(m_aiPlotGroup);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiPlotGroup = new int[cCount];
		pStream->Read(cCount, m_aiPlotGroup);
	}

	SAFE_DELETE_ARRAY(m_aiVisibilityCount);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiVisibilityCount = new short[cCount];
		pStream->Read(cCount, m_aiVisibilityCount);
	}

	SAFE_DELETE_ARRAY(m_aiStolenVisibilityCount);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiStolenVisibilityCount = new short[cCount];
		pStream->Read(cCount, m_aiStolenVisibilityCount);
	}

	SAFE_DELETE_ARRAY(m_aiBlockadedCount);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiBlockadedCount = new short[cCount];
		pStream->Read(cCount, m_aiBlockadedCount);
	}

	SAFE_DELETE_ARRAY(m_aiRevealedOwner);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aiRevealedOwner = new char[cCount];
		pStream->Read(cCount, m_aiRevealedOwner);
	}

	SAFE_DELETE_ARRAY(m_abRiverCrossing);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_abRiverCrossing = new bool[cCount];
		pStream->Read(cCount, m_abRiverCrossing);
	}

	SAFE_DELETE_ARRAY(m_abRevealed);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_abRevealed = new bool[cCount];
		pStream->Read(cCount, m_abRevealed);
	}

	SAFE_DELETE_ARRAY(m_aeRevealedImprovementType);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aeRevealedImprovementType = new short[cCount];
		pStream->Read(cCount, m_aeRevealedImprovementType);
	}

	SAFE_DELETE_ARRAY(m_aeRevealedRouteType);
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_aeRevealedRouteType = new short[cCount];
		pStream->Read(cCount, m_aeRevealedRouteType);
	}

	m_szScriptData = pStream->ReadString();

	SAFE_DELETE_ARRAY(m_paiBuildProgress);
	pStream->Read(&iCount);
	if (iCount > 0)
	{
		m_paiBuildProgress = new short[iCount];
		pStream->Read(iCount, m_paiBuildProgress);
	}

	if (NULL != m_apaiCultureRangeCities)
	{
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			SAFE_DELETE_ARRAY(m_apaiCultureRangeCities[iI]);
		}
		SAFE_DELETE_ARRAY(m_apaiCultureRangeCities);
	}
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_apaiCultureRangeCities = new char*[cCount];
		for (iI = 0; iI < cCount; ++iI)
		{
			pStream->Read(&iCount);
			if (iCount > 0)
			{
				m_apaiCultureRangeCities[iI] = new char[iCount];
				pStream->Read(iCount, m_apaiCultureRangeCities[iI]);
			}
			else
			{
				m_apaiCultureRangeCities[iI] = NULL;
			}
		}
	}

	if (NULL != m_apaiInvisibleVisibilityCount)
	{
		for (int iI = 0; iI < MAX_TEAMS; ++iI)
		{
			SAFE_DELETE_ARRAY(m_apaiInvisibleVisibilityCount[iI]);
		}
		SAFE_DELETE_ARRAY(m_apaiInvisibleVisibilityCount);
	}
	pStream->Read(&cCount);
	if (cCount > 0)
	{
		m_apaiInvisibleVisibilityCount = new short*[cCount];
		for (iI = 0; iI < cCount; ++iI)
		{
			pStream->Read(&iCount);
			if (iCount > 0)
			{
				m_apaiInvisibleVisibilityCount[iI] = new short[iCount];
				pStream->Read(iCount, m_apaiInvisibleVisibilityCount[iI]);
			}
			else
			{
				m_apaiInvisibleVisibilityCount[iI] = NULL;
			}
		}
	}

	m_units.Read(pStream);
}

//
// write object to a stream
// used during save
//
void CvPlot::write(FDataStreamBase* pStream)
{
	uint iI;

	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iX);
	pStream->Write(m_iY);
	pStream->Write(m_iArea);
	// m_pPlotArea not saved
	pStream->Write(m_iFeatureVariety);
	pStream->Write(m_iOwnershipDuration);
	pStream->Write(m_iImprovementDuration);
	pStream->Write(m_iUpgradeProgress);
	pStream->Write(m_iForceUnownedTimer);
	pStream->Write(m_iCityRadiusCount);
	pStream->Write(m_iRiverID);
	pStream->Write(m_iMinOriginalStartDist);
	pStream->Write(m_iReconCount);
	pStream->Write(m_iRiverCrossingCount);

	// Super Forts begin *canal* *choke*
	pStream->Write(m_iCanalValue);
	pStream->Write(m_iChokeValue);
	// Super Forts end
	// Super Forts begin *bombard*
	pStream->Write(m_iDefenseDamage);
	pStream->Write(m_bBombarded);
	// Super Forts end

	pStream->Write(m_bStartingPlot);
	pStream->Write(m_bHills);
	pStream->Write(m_bNOfRiver);
	pStream->Write(m_bWOfRiver);
	pStream->Write(m_bIrrigated);
	pStream->Write(m_bPotentialCityWork);
	// m_bShowCitySymbols not saved
	// m_bFlagDirty not saved
	// m_bPlotLayoutDirty not saved
	// m_bLayoutStateWorked not saved

	pStream->Write(m_eOwner);
	pStream->Write(m_ePlotType);
	pStream->Write(m_eTerrainType);
	pStream->Write(m_eFeatureType);
	pStream->Write(m_eBonusType);
	pStream->Write(m_eImprovementType);
	pStream->Write(m_eRouteType);
	pStream->Write(m_eRiverNSDirection);
	pStream->Write(m_eRiverWEDirection);

	pStream->Write(m_plotCity.eOwner);
	pStream->Write(m_plotCity.iID);
	pStream->Write(m_workingCity.eOwner);
	pStream->Write(m_workingCity.iID);
	pStream->Write(m_workingCityOverride.eOwner);
	pStream->Write(m_workingCityOverride.iID);

	pStream->Write(NUM_YIELD_TYPES, m_aiYield);

//FfH: Added by Kael 08/15/2007
	pStream->Write(m_bMoveDisabledAI);
	pStream->Write(m_bMoveDisabledHuman);
	pStream->Write(m_bBuildDisabled);
	pStream->Write(m_bFoundDisabled);
	pStream->Write(m_bPythonActive);
	pStream->Write(m_eRealTerrainType);
	pStream->Write(m_iMinLevel);
	pStream->Write(m_iPlotCounter);
	pStream->Write(m_iPortalExitX);
	pStream->Write(m_iPortalExitY);
	pStream->Write(m_iTempTerrainTimer);
//FfH: End Add

	pStream->Write(m_iDungeonType);
	pStream->Write(m_iWilderness);
	pStream->Write(m_iPlane);
	pStream->Write(m_iPlundered);
	pStream->Write(m_bWildmanaImmune);
	pStream->Write(m_bNonPlanar);
	pStream->Write(m_iAnimalDensity);
	pStream->Write(m_iAnimalDensityPlusLairs);
	pStream->Write(m_iBarBarianDensity);
	pStream->Write(m_iBarBarianDensityPlusLairs);
	pStream->Write(m_iDungeon);
	pStream->Write(m_iPortalExit);	

	//pathfinding
	//pStream->Write(m_iRegion);
	//pStream->Write(m_iSector);

	if (NULL == m_aiCulture)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_PLAYERS);
		pStream->Write(MAX_PLAYERS, m_aiCulture);
	}

	// Super Forts begin *culture*
	if (NULL == m_aiCultureRangeForts)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_PLAYERS);
		pStream->Write(MAX_PLAYERS, m_aiCultureRangeForts);
	}
	// Super Forts end

	if (NULL == m_aiFoundValue)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_PLAYERS);
		pStream->Write(MAX_PLAYERS, m_aiFoundValue);
	}

	if (NULL == m_aiPlayerCityRadiusCount)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_PLAYERS);
		pStream->Write(MAX_PLAYERS, m_aiPlayerCityRadiusCount);
	}

	if (NULL == m_aiPlotGroup)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_PLAYERS);
		pStream->Write(MAX_PLAYERS, m_aiPlotGroup);
	}

	if (NULL == m_aiVisibilityCount)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		pStream->Write(MAX_TEAMS, m_aiVisibilityCount);
	}

	if (NULL == m_aiStolenVisibilityCount)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		pStream->Write(MAX_TEAMS, m_aiStolenVisibilityCount);
	}

	if (NULL == m_aiBlockadedCount)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		pStream->Write(MAX_TEAMS, m_aiBlockadedCount);
	}

	if (NULL == m_aiRevealedOwner)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		pStream->Write(MAX_TEAMS, m_aiRevealedOwner);
	}

	if (NULL == m_abRiverCrossing)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)NUM_DIRECTION_TYPES);
		pStream->Write(NUM_DIRECTION_TYPES, m_abRiverCrossing);
	}

	if (NULL == m_abRevealed)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		pStream->Write(MAX_TEAMS, m_abRevealed);
	}

	if (NULL == m_aeRevealedImprovementType)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		pStream->Write(MAX_TEAMS, m_aeRevealedImprovementType);
	}

	if (NULL == m_aeRevealedRouteType)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		pStream->Write(MAX_TEAMS, m_aeRevealedRouteType);
	}

	pStream->WriteString(m_szScriptData);

	if (NULL == m_paiBuildProgress)
	{
		pStream->Write((int)0);
	}
	else
	{
		pStream->Write((int)GC.getNumBuildInfos());
		pStream->Write(GC.getNumBuildInfos(), m_paiBuildProgress);
	}

	if (NULL == m_apaiCultureRangeCities)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_PLAYERS);
		for (iI=0; iI < MAX_PLAYERS; ++iI)
		{
			if (NULL == m_apaiCultureRangeCities[iI])
			{
				pStream->Write((int)0);
			}
			else
			{
				pStream->Write((int)GC.getNumCultureLevelInfos());
				pStream->Write(GC.getNumCultureLevelInfos(), m_apaiCultureRangeCities[iI]);
			}
		}
	}

	if (NULL == m_apaiInvisibleVisibilityCount)
	{
		pStream->Write((char)0);
	}
	else
	{
		pStream->Write((char)MAX_TEAMS);
		for (iI=0; iI < MAX_TEAMS; ++iI)
		{
			if (NULL == m_apaiInvisibleVisibilityCount[iI])
			{
				pStream->Write((int)0);
			}
			else
			{
				pStream->Write((int)GC.getNumInvisibleInfos());
				pStream->Write(GC.getNumInvisibleInfos(), m_apaiInvisibleVisibilityCount[iI]);
			}
		}
	}

	m_units.Write(pStream);
}

void CvPlot::setLayoutDirty(bool bDirty)
{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (!GC.IsGraphicsInitialized())
	if (!shouldHaveFullGraphics())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/				
	{
		return;
	}

	if (isLayoutDirty() != bDirty)
	{
		m_bPlotLayoutDirty = bDirty;

		if (isLayoutDirty() && (m_pPlotBuilder == NULL))
		{
			if (!updatePlotBuilder())
			{
				m_bPlotLayoutDirty = false;
			}
		}
	}
}

bool CvPlot::updatePlotBuilder()
{
/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	if (GC.IsGraphicsInitialized() && shouldUsePlotBuilder())
	if (shouldHaveFullGraphics() && shouldUsePlotBuilder())
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
	{
		if (m_pPlotBuilder == NULL) // we need a plot builder... but it doesn't exist
		{
			m_pPlotBuilder = gDLL->getPlotBuilderIFace()->create();
			gDLL->getPlotBuilderIFace()->init(m_pPlotBuilder, this);
		}

		return true;
	}

	return false;
}

bool CvPlot::isLayoutDirty() const
{
	return m_bPlotLayoutDirty;
}

bool CvPlot::isLayoutStateDifferent() const
{
	bool bSame = true;
	// is worked
	bSame &= m_bLayoutStateWorked == isBeingWorked();

	// done
	return !bSame;
}

void CvPlot::setLayoutStateToCurrent()
{
	m_bLayoutStateWorked = isBeingWorked();
}

//------------------------------------------------------------------------------------------------

void CvPlot::getVisibleImprovementState(ImprovementTypes& eType, bool& bWorked)
{
	eType = NO_IMPROVEMENT;
	bWorked = false;

	if (GC.getGameINLINE().getActiveTeam() == NO_TEAM)
	{
		return;
	}

	eType = getRevealedImprovementType(GC.getGameINLINE().getActiveTeam(), true);

	if (eType == NO_IMPROVEMENT)
	{
		if (isActiveVisible(true))
		{
			if (isBeingWorked() && !isCity())
			{
				if (isWater())
				{
					eType = ((ImprovementTypes)(GC.getDefineINT("WATER_IMPROVEMENT")));
				}
				else
				{
					eType = ((ImprovementTypes)(GC.getDefineINT("LAND_IMPROVEMENT")));
				}
			}
		}
	}

	// worked state
	if (isActiveVisible(false) && isBeingWorked())
	{
		bWorked = true;
	}
}

void CvPlot::getVisibleBonusState(BonusTypes& eType, bool& bImproved, bool& bWorked)
{
	eType = NO_BONUS;
	bImproved = false;
	bWorked = false;

	if (GC.getGameINLINE().getActiveTeam() == NO_TEAM)
	{
		return;
	}

	if (GC.getGameINLINE().isDebugMode())
	{
		eType = getBonusType();
	}
	else if (isRevealed(GC.getGameINLINE().getActiveTeam(), false))
	{
		eType = getBonusType(GC.getGameINLINE().getActiveTeam());
	}

	// improved and worked states ...
	if (eType != NO_BONUS)
	{
		ImprovementTypes eRevealedImprovement = getRevealedImprovementType(GC.getGameINLINE().getActiveTeam(), true);

		if ((eRevealedImprovement != NO_IMPROVEMENT) && GC.getImprovementInfo(eRevealedImprovement).isImprovementBonusTrade(eType))
		{
			bImproved = true;
			bWorked = isBeingWorked();
		}
	}
}

bool CvPlot::shouldUsePlotBuilder()
{
	bool bBonusImproved; bool bBonusWorked; bool bImprovementWorked;
	BonusTypes eBonusType;
	ImprovementTypes eImprovementType;
	getVisibleBonusState(eBonusType, bBonusImproved, bBonusWorked);
	getVisibleImprovementState(eImprovementType, bImprovementWorked);
	if(eBonusType != NO_BONUS || eImprovementType != NO_IMPROVEMENT)
	{
		return true;
	}
	return false;
}


int CvPlot::calculateMaxYield(YieldTypes eYield) const
{
	if (getTerrainType() == NO_TERRAIN)
	{
		return 0;
	}

	int iMaxYield = calculateNatureYield(eYield, NO_TEAM);

	int iImprovementYield = 0;
	for (int iImprovement = 0; iImprovement < GC.getNumImprovementInfos(); iImprovement++)
	{
		iImprovementYield = std::max(calculateImprovementYieldChange((ImprovementTypes)iImprovement, eYield, NO_PLAYER, true), iImprovementYield);
	}
	iMaxYield += iImprovementYield;

	int iRouteYield = 0;
	for (int iRoute = 0; iRoute < GC.getNumRouteInfos(); iRoute++)
	{
		iRouteYield = std::max(GC.getRouteInfo((RouteTypes)iRoute).getYieldChange(eYield), iRouteYield);
	}
	iMaxYield += iRouteYield;

	if (isWater() && !isImpassable())
	{
		int iBuildingYield = 0;
		for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); iBuilding++)
		{
			CvBuildingInfo& building = GC.getBuildingInfo((BuildingTypes)iBuilding);
			iBuildingYield = std::max(building.getSeaPlotYieldChange(eYield) + building.getGlobalSeaPlotYieldChange(eYield), iBuildingYield);
		}
		iMaxYield += iBuildingYield;
	}

	if (isRiver())
	{
		int iBuildingYield = 0;
		for (int iBuilding = 0; iBuilding < GC.getNumBuildingInfos(); iBuilding++)
		{
			CvBuildingInfo& building = GC.getBuildingInfo((BuildingTypes)iBuilding);
			iBuildingYield = std::max(building.getRiverPlotYieldChange(eYield), iBuildingYield);
		}
		iMaxYield += iBuildingYield;
	}

	int iExtraYieldThreshold = 0;
	for (int iTrait = 0; iTrait < GC.getNumTraitInfos(); iTrait++)
	{
		CvTraitInfo& trait = GC.getTraitInfo((TraitTypes)iTrait);
		iExtraYieldThreshold  = std::max(trait.getExtraYieldThreshold(eYield), iExtraYieldThreshold);
	}
	if (iExtraYieldThreshold > 0 && iMaxYield > iExtraYieldThreshold)
	{
		iMaxYield += GC.getDefineINT("EXTRA_YIELD");
	}

	return iMaxYield;
}

int CvPlot::getYieldWithBuild(BuildTypes eBuild, YieldTypes eYield, bool bWithUpgrade) const
{
	int iYield = 0;

	bool bIgnoreFeature = false;
	if (getFeatureType() != NO_FEATURE)
	{
		if (GC.getBuildInfo(eBuild).isFeatureRemove(getFeatureType()))
		{
			bIgnoreFeature = true;
		}

//FfH: Added by Kael 04/24/2008
        if (isOwned())
        {
            if (GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).isMaintainFeatures(getFeatureType()))
            {
                bIgnoreFeature = false;
            }
        }
//FfH: End Add

	}

	iYield += calculateNatureYield(eYield, getTeam(), bIgnoreFeature);

	ImprovementTypes eImprovement = (ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement();

	if (eImprovement != NO_IMPROVEMENT)
	{
		if (bWithUpgrade)
		{
			//in the case that improvements upgrade, use 2 upgrade levels higher for the
			//yield calculations.
			ImprovementTypes eUpgradeImprovement = (ImprovementTypes)GC.getImprovementInfo(eImprovement).getImprovementUpgrade();
			if (eUpgradeImprovement != NO_IMPROVEMENT)
			{
				//unless it's commerce on a low food tile, in which case only use 1 level higher
				if ((eYield != YIELD_COMMERCE) || (getYield(YIELD_FOOD) >= GC.getFOOD_CONSUMPTION_PER_POPULATION()))
				{
					ImprovementTypes eUpgradeImprovement2 = (ImprovementTypes)GC.getImprovementInfo(eUpgradeImprovement).getImprovementUpgrade();
					if (eUpgradeImprovement2 != NO_IMPROVEMENT)
					{
						eUpgradeImprovement = eUpgradeImprovement2;
					}
				}
			}

			if ((eUpgradeImprovement != NO_IMPROVEMENT) && (eUpgradeImprovement != eImprovement))
			{
				eImprovement = eUpgradeImprovement;
			}
		}

		iYield += calculateImprovementYieldChange(eImprovement, eYield, getOwnerINLINE(), false);
	}

	RouteTypes eRoute = (RouteTypes)GC.getBuildInfo(eBuild).getRoute();
	if (eRoute != NO_ROUTE)
	{
		eImprovement = getImprovementType();
		if (eImprovement != NO_IMPROVEMENT)
		{
			for (int iI = 0; iI < NUM_YIELD_TYPES; iI++)
			{
				iYield += GC.getImprovementInfo(eImprovement).getRouteYieldChanges(eRoute, iI);
				if (getRouteType() != NO_ROUTE)
				{
					iYield -= GC.getImprovementInfo(eImprovement).getRouteYieldChanges(getRouteType(), iI);
				}
			}
		}
	}


	return iYield;
}

bool CvPlot::canTrigger(EventTriggerTypes eTrigger, PlayerTypes ePlayer) const
{
	FAssert(::isPlotEventTrigger(eTrigger));

	CvEventTriggerInfo& kTrigger = GC.getEventTriggerInfo(eTrigger);

	if (kTrigger.isOwnPlot() && getOwnerINLINE() != ePlayer)
	{
		return false;
	}

	if (kTrigger.getPlotType() != NO_PLOT)
	{
		if (getPlotType() != kTrigger.getPlotType())
		{
			return false;
		}
	}

	if (kTrigger.getNumFeaturesRequired() > 0)
	{
		bool bFoundValid = false;

		for (int i = 0; i < kTrigger.getNumFeaturesRequired(); ++i)
		{
			if (kTrigger.getFeatureRequired(i) == getFeatureType())
			{
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			return false;
		}
	}

	if (kTrigger.getNumTerrainsRequired() > 0)
	{
		bool bFoundValid = false;

		for (int i = 0; i < kTrigger.getNumTerrainsRequired(); ++i)
		{
			if (kTrigger.getTerrainRequired(i) == getTerrainType())
			{
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			return false;
		}
	}

	if (kTrigger.getNumImprovementsRequired() > 0)
	{
		bool bFoundValid = false;

		for (int i = 0; i < kTrigger.getNumImprovementsRequired(); ++i)
		{
			if (kTrigger.getImprovementRequired(i) == getImprovementType())
			{
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			return false;
		}
	}

	if (kTrigger.getNumBonusesRequired() > 0)
	{
		bool bFoundValid = false;

		for (int i = 0; i < kTrigger.getNumBonusesRequired(); ++i)
		{
			if (kTrigger.getBonusRequired(i) == getBonusType(kTrigger.isOwnPlot() ? GET_PLAYER(ePlayer).getTeam() : NO_TEAM))
			{
				bFoundValid = true;
				break;
			}
		}

		if (!bFoundValid)
		{
			return false;
		}
	}

	if (kTrigger.getNumRoutesRequired() > 0)
	{
		bool bFoundValid = false;

		if (NULL == getPlotCity())
		{
		for (int i = 0; i < kTrigger.getNumRoutesRequired(); ++i)
		{
			if (kTrigger.getRouteRequired(i) == getRouteType())
			{
				bFoundValid = true;
				break;
			}
		}

		}

		if (!bFoundValid)
		{
			return false;
		}
	}

	if (kTrigger.isUnitsOnPlot())
	{
		bool bFoundValid = false;

		CLLNode<IDInfo>* pUnitNode = headUnitNode();

		while (NULL != pUnitNode)
		{
			CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
			pUnitNode = nextUnitNode(pUnitNode);

			if (pLoopUnit->getOwnerINLINE() == ePlayer)
			{
				if (-1 != pLoopUnit->getTriggerValue(eTrigger, this, false))
				{
					bFoundValid = true;
					break;
				}
			}
		}

		if (!bFoundValid)
		{
			return false;
		}
	}


	if (kTrigger.isPrereqEventCity() && kTrigger.getNumPrereqEvents() > 0)
	{
		bool bFoundValid = true;

		for (int iI = 0; iI < kTrigger.getNumPrereqEvents(); ++iI)
		{
			const EventTriggeredData* pTriggeredData = GET_PLAYER(ePlayer).getEventOccured((EventTypes)kTrigger.getPrereqEvent(iI));
			if (NULL == pTriggeredData || pTriggeredData->m_iPlotX != getX_INLINE() || pTriggeredData->m_iPlotY != getY_INLINE())
			{
				bFoundValid = false;
				break;
			}
		}

		if (!bFoundValid)
		{
			return false;
		}
	}


	return true;
}

bool CvPlot::canApplyEvent(EventTypes eEvent) const
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (kEvent.getFeatureChange() > 0)
	{
		if (NO_FEATURE != kEvent.getFeature())
		{
			if (NO_IMPROVEMENT != getImprovementType() || !canHaveFeature((FeatureTypes)kEvent.getFeature()))
			{
				return false;
			}
		}
	}
	else if (kEvent.getFeatureChange() < 0)
	{
		if (NO_FEATURE == getFeatureType())
		{
			return false;
		}
	}

	if (kEvent.getImprovementChange() > 0)
	{
		if (NO_IMPROVEMENT != kEvent.getImprovement())
		{
			if (!canHaveImprovement((ImprovementTypes)kEvent.getImprovement(), getTeam()))
			{
				return false;
			}
		}
	}
	else if (kEvent.getImprovementChange() < 0)
	{
		if (NO_IMPROVEMENT == getImprovementType())
		{
			return false;
		}
	}

//FfH: Modified by Kael 03/19/2008
//	if (kEvent.getBonusChange() > 0)
//	{
//		if (NO_BONUS != kEvent.getBonus())
//		{
//			if (!canHaveBonus((BonusTypes)kEvent.getBonus(), false))
//			{
//				return false;
//			}
//		}
//	}
//    else if (kEvent.getBonusChange() < 0)
//	{
//		if (NO_BONUS == getBonusType())
//		{
//			return false;
//		}
//	}
//FfH: End Modify

	if (kEvent.getRouteChange() < 0)
	{
		if (NO_ROUTE == getRouteType())
		{
			return false;
		}

		if (isCity())
		{
			return false;
		}
	}

	return true;
}

void CvPlot::applyEvent(EventTypes eEvent)
{
	CvEventInfo& kEvent = GC.getEventInfo(eEvent);

	if (kEvent.getFeatureChange() > 0)
	{
		if (NO_FEATURE != kEvent.getFeature())
		{
			setFeatureType((FeatureTypes)kEvent.getFeature());
		}
	}
	else if (kEvent.getFeatureChange() < 0)
	{
		setFeatureType(NO_FEATURE);
	}

	if (kEvent.getImprovementChange() > 0)
	{
		if (NO_IMPROVEMENT != kEvent.getImprovement())
		{
			setImprovementType((ImprovementTypes)kEvent.getImprovement());
		}
	}
	else if (kEvent.getImprovementChange() < 0)
	{
		setImprovementType(NO_IMPROVEMENT);
	}

	if (kEvent.getBonusChange() > 0)
	{
		if (NO_BONUS != kEvent.getBonus())
		{
			setBonusType((BonusTypes)kEvent.getBonus());
		}
	}
	else if (kEvent.getBonusChange() < 0)
	{
		setBonusType(NO_BONUS);
	}

	if (kEvent.getRouteChange() > 0)
	{
		if (NO_ROUTE != kEvent.getRoute())
		{
			setRouteType((RouteTypes)kEvent.getRoute(), true);
		}
	}
	else if (kEvent.getRouteChange() < 0)
	{
		setRouteType(NO_ROUTE, true);
	}

	for (int i = 0; i < NUM_YIELD_TYPES; ++i)
	{
		int iChange = kEvent.getPlotExtraYield(i);
		if (0 != iChange)
		{
			GC.getGameINLINE().setPlotExtraYield(m_iX, m_iY, (YieldTypes)i, iChange);
		}
	}
}

bool CvPlot::canTrain(UnitTypes eUnit, bool bContinue, bool bTestVisible, bool bIgnoreResources) const
{
	CvCity* pCity = getPlotCity();

	if (GC.getUnitInfo(eUnit).isPrereqReligion())
	{
		if (NULL == pCity || pCity->getReligionCount() > 0)
		{
			return false;
		}
	}

	if (GC.getUnitInfo(eUnit).getPrereqReligion() != NO_RELIGION)
	{
		if (NULL == pCity || !pCity->isHasReligion((ReligionTypes)(GC.getUnitInfo(eUnit).getPrereqReligion())))
		{
			return false;
		}
	}

	if (GC.getUnitInfo(eUnit).getPrereqCorporation() != NO_CORPORATION)
	{
		if (NULL == pCity || !pCity->isActiveCorporation((CorporationTypes)(GC.getUnitInfo(eUnit).getPrereqCorporation())))
		{
			return false;
		}
	}

	if (!bIgnoreResources && GC.getUnitInfo(eUnit).isPrereqBonuses())
	{
		if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_SEA)
		{
			bool bValid = false;

			for (int iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
			{
				CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

				if (pLoopPlot != NULL)
				{
					if (pLoopPlot->isWater())
					{
						if (pLoopPlot->area()->getNumTotalBonuses() > 0)
						{
							bValid = true;
							break;
						}
					}
				}
			}

			if (!bValid)
			{
				return false;
			}
		}
		else
		{
			if (area()->getNumTotalBonuses() > 0)
			{
				return false;
			}
		}
	}

	if (isCity())
	{
		if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_SEA)
		{
			if (!isWater() && !isCoastalLand(GC.getUnitInfo(eUnit).getMinAreaSize()))
			{
				return false;
			}
		}
		else
		{
			if (area()->getNumTiles() < GC.getUnitInfo(eUnit).getMinAreaSize())
			{
				return false;
			}
		}
	}
	else
	{
		if (area()->getNumTiles() < GC.getUnitInfo(eUnit).getMinAreaSize())
		{
			return false;
		}

		if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_SEA)
		{
			if (!isWater())
			{
				return false;
			}
		}
		else if (GC.getUnitInfo(eUnit).getDomainType() == DOMAIN_LAND)
		{
			if (isWater())
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	if (!bTestVisible)
	{
		if (GC.getUnitInfo(eUnit).getHolyCity() != NO_RELIGION)
		{
			if (NULL == pCity || !pCity->isHolyCity(((ReligionTypes)(GC.getUnitInfo(eUnit).getHolyCity()))))
			{
				return false;
			}
		}

//FfH Units: Modified by Kael 12/08/2007
//		if (GC.getUnitInfo(eUnit).getPrereqBuilding() != NO_BUILDING)
//		{
//			if (NULL == pCity)
//			{
//				return false;
//			}
//
//			if (pCity->getNumBuilding((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding())) == 0)
//			{
//				SpecialBuildingTypes eSpecialBuilding = ((SpecialBuildingTypes)(GC.getBuildingInfo((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding())).getSpecialBuildingType()));
//
//				if ((eSpecialBuilding == NO_SPECIALBUILDING) || !(GET_PLAYER(getOwnerINLINE()).isSpecialBuildingNotRequired(eSpecialBuilding)))
//				{
//					return false;
//				}
//			}
//		}
        bool bValid = true;
        if (isOwned())
        {
            if (!GET_PLAYER(getOwnerINLINE()).isHuman())
            {
/*************************************************************************************************/
/**	Xienwolf Tweak							02/01/09											**/
/**																								**/
/**			Ensures that Barbarian Cities do not devote themselves to building Empires			**/
/*************************************************************************************************/
/**								---- Start Original Code ----									**
                if (GC.getGameINLINE().isOption(GAMEOPTION_AI_NO_BUILDING_PREREQS))
/**								----  End Original Code  ----									**/
                if (GC.getGameINLINE().isOption(GAMEOPTION_AI_NO_BUILDING_PREREQS) || GET_PLAYER(getOwnerINLINE()).isBarbarian())
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/
                {
                    bValid = false;
                }
            }
        }
			// for now allow AI to ignore Construction prereqs
        if (bValid && GET_PLAYER(getOwnerINLINE()).isHuman())
        {
            if (GC.getUnitInfo(eUnit).getPrereqBuilding() != NO_BUILDING)
            {
                if (NULL == pCity)
                {
                    return false;
                }
                if (pCity->getNumBuilding((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding())) == 0)
                {
                    SpecialBuildingTypes eSpecialBuilding = ((SpecialBuildingTypes)(GC.getBuildingInfo((BuildingTypes)(GC.getUnitInfo(eUnit).getPrereqBuilding())).getSpecialBuildingType()));
                    if ((eSpecialBuilding == NO_SPECIALBUILDING) || !(GET_PLAYER(getOwnerINLINE()).isSpecialBuildingNotRequired(eSpecialBuilding)))
                    {
                        return false;
                    }
                }
            }
            if (GC.getUnitInfo(eUnit).getPrereqBuildingClass() != NO_BUILDINGCLASS)
            {
                if (NULL == pCity)
                {
                    return false;
                }
                if (!pCity->isHasBuildingClass(GC.getUnitInfo(eUnit).getPrereqBuildingClass()))
                {
                    return false;
                }
            }
		}
//FfH: End Modify

		if (GC.getUnitInfo(eUnit).getPrereqAndBonus() != NO_BONUS)
		{
			if (NULL == pCity)
			{
				if (!isPlotGroupConnectedBonus(getOwnerINLINE(), (BonusTypes)GC.getUnitInfo(eUnit).getPrereqAndBonus()))
				{
					return false;
				}
			}
			else
			{
				if (!pCity->hasBonus((BonusTypes)GC.getUnitInfo(eUnit).getPrereqAndBonus()))
				{
					return false;
				}
			}
		}

		bool bRequiresBonus = false;
		bool bNeedsBonus = true;

		for (int iI = 0; iI < GC.getNUM_UNIT_PREREQ_OR_BONUSES(); ++iI)
		{
			if (GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI) != NO_BONUS)
			{
				bRequiresBonus = true;

				if (NULL == pCity)
				{
					if (isPlotGroupConnectedBonus(getOwnerINLINE(), (BonusTypes)GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI)))
					{
						bNeedsBonus = false;
						break;
					}
				}
				else
				{
					if (pCity->hasBonus((BonusTypes)GC.getUnitInfo(eUnit).getPrereqOrBonuses(iI)))
					{
						bNeedsBonus = false;
						break;
					}
				}
			}
		}

		if (bRequiresBonus && bNeedsBonus)
		{
			return false;
		}
	}

	return true;
}

int CvPlot::countFriendlyCulture(TeamTypes eTeam) const
{
	int iTotalCulture = 0;

	for (int iPlayer = 0; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
		CvPlayer& kLoopPlayer = GET_PLAYER((PlayerTypes)iPlayer);
		if (kLoopPlayer.isAlive())
		{
			CvTeam& kLoopTeam = GET_TEAM(kLoopPlayer.getTeam());
			if (kLoopPlayer.getTeam() == eTeam || kLoopTeam.isVassal(eTeam) || kLoopTeam.isOpenBorders(eTeam))
			{
				iTotalCulture += getCulture((PlayerTypes)iPlayer);
			}
		}
	}

	return iTotalCulture;
}

int CvPlot::countNumAirUnits(TeamTypes eTeam) const
{
	int iCount = 0;

	CLLNode<IDInfo>* pUnitNode = headUnitNode();
	while (pUnitNode != NULL)
	{
		CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if (DOMAIN_AIR == pLoopUnit->getDomainType() && !pLoopUnit->isCargo() && pLoopUnit->getTeam() == eTeam)
		{
			iCount += GC.getUnitInfo(pLoopUnit->getUnitType()).getAirUnitCap();
		}
	}

	return iCount;
}

int CvPlot::airUnitSpaceAvailable(TeamTypes eTeam) const
{
	int iMaxUnits = 0;

	CvCity* pCity = getPlotCity();
	if (NULL != pCity)
	{
		iMaxUnits = pCity->getAirUnitCapacity(getTeam());
	}
	else
	{
		iMaxUnits = GC.getDefineINT("CITY_AIR_UNIT_CAPACITY");
	}

	return (iMaxUnits - countNumAirUnits(eTeam));
}


bool CvPlot::isEspionageCounterSpy(TeamTypes eTeam) const
{
	CvCity* pCity = getPlotCity();

	if (NULL != pCity && pCity->getTeam() == eTeam)
	{
		if (pCity->getEspionageDefenseModifier() > 0)
		{
			return true;
		}
	}

	if (plotCount(PUF_isCounterSpy, -1, -1, NO_PLAYER, eTeam) > 0)
	{
		return true;
	}

	return false;
}

int CvPlot::getAreaIdForGreatWall() const
{
	return getArea();
}

int CvPlot::getSoundScriptId() const
{
	int iScriptId = -1;
	if (isActiveVisible(true))
	{
		if (getImprovementType() != NO_IMPROVEMENT)
		{
			iScriptId = GC.getImprovementInfo(getImprovementType()).getWorldSoundscapeScriptId();
		}
		else if (getFeatureType() != NO_FEATURE)
		{
			iScriptId = GC.getFeatureInfo(getFeatureType()).getWorldSoundscapeScriptId();
		}
		else if (getTerrainType() != NO_TERRAIN)
		{
			iScriptId = GC.getTerrainInfo(getTerrainType()).getWorldSoundscapeScriptId();
		}
	}
	return iScriptId;
}

int CvPlot::get3DAudioScriptFootstepIndex(int iFootstepTag) const
{
	if (getFeatureType() != NO_FEATURE)
	{
		return GC.getFeatureInfo(getFeatureType()).get3DAudioScriptFootstepIndex(iFootstepTag);
	}

	if (getTerrainType() != NO_TERRAIN)
	{
		return GC.getTerrainInfo(getTerrainType()).get3DAudioScriptFootstepIndex(iFootstepTag);
	}

	return -1;
}

float CvPlot::getAqueductSourceWeight() const
{
	float fWeight = 0.0f;
	if (isLake() || isPeak() || (getFeatureType() != NO_FEATURE && GC.getFeatureInfo(getFeatureType()).isAddsFreshWater()))
	{
		fWeight = 1.0f;
	}
	else if (isHills())
	{
		fWeight = 0.67f;
	}

	return fWeight;
}

bool CvPlot::shouldDisplayBridge(CvPlot* pToPlot, PlayerTypes ePlayer) const
{
	TeamTypes eObservingTeam = GET_PLAYER(ePlayer).getTeam();
	TeamTypes eOurTeam = getRevealedTeam(eObservingTeam, true);
	TeamTypes eOtherTeam = NO_TEAM;
	if (pToPlot != NULL)
	{
		eOtherTeam = pToPlot->getRevealedTeam(eObservingTeam, true);
	}

	if (eOurTeam == eObservingTeam || eOtherTeam == eObservingTeam || (eOurTeam == NO_TEAM && eOtherTeam == NO_TEAM))
	{
		return GET_TEAM(eObservingTeam).isBridgeBuilding();
	}

	if (eOurTeam == NO_TEAM)
	{
		return GET_TEAM(eOtherTeam).isBridgeBuilding();
	}

	if (eOtherTeam == NO_TEAM)
	{
		return GET_TEAM(eOurTeam).isBridgeBuilding();
	}

	return (GET_TEAM(eOurTeam).isBridgeBuilding() && GET_TEAM(eOtherTeam).isBridgeBuilding());
}

bool CvPlot::checkLateEra() const
{
	PlayerTypes ePlayer = getOwnerINLINE();
	if (ePlayer == NO_PLAYER)
	{
		//find largest culture in this plot
		ePlayer = GC.getGameINLINE().getActivePlayer();
		int maxCulture = getCulture(ePlayer);
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			int newCulture = getCulture((PlayerTypes) i);
			if (newCulture > maxCulture)
			{
				maxCulture = newCulture;
				ePlayer = (PlayerTypes) i;
			}
		}
	}

	return (GET_PLAYER(ePlayer).getCurrentEra() > GC.getNumEraInfos() / 2);
}

//FfH: Added by Kael 08/15/2007
bool CvPlot::isMoveDisabledAI() const
{
    return m_bMoveDisabledAI;
}

void CvPlot::setMoveDisabledAI(bool bNewValue)
{
    m_bMoveDisabledAI = bNewValue;
}

bool CvPlot::isMoveDisabledHuman() const
{
    return m_bMoveDisabledHuman;
}

void CvPlot::setMoveDisabledHuman(bool bNewValue)
{
    m_bMoveDisabledHuman = bNewValue;
}

bool CvPlot::isBuildDisabled() const
{
    return m_bBuildDisabled;
}

void CvPlot::setBuildDisabled(bool bNewValue)
{
    m_bBuildDisabled = bNewValue;
}

bool CvPlot::isFoundDisabled() const
{
    return m_bFoundDisabled;
}

void CvPlot::setFoundDisabled(bool bNewValue)
{
    m_bFoundDisabled = bNewValue;
}

int CvPlot::getMinLevel() const
{
    return m_iMinLevel;
}

void CvPlot::setMinLevel(int iNewValue)
{
    m_iMinLevel = iNewValue;
}

int CvPlot::getNumAnimalUnits() const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
    pUnitNode = headUnitNode();
    int iCount = 0;
    while (pUnitNode != NULL)
    {
        pLoopUnit = ::getUnit(pUnitNode->m_data);
        pUnitNode = nextUnitNode(pUnitNode);
        if (pLoopUnit->isAnimal())
        {
            iCount += 1;
        }
    }
    return iCount;
}

int CvPlot::getPlotCounter() const
{
    return m_iPlotCounter;
}

void CvPlot::changePlotCounter(int iChange)
{
	if (iChange != 0)
	{
		const int iOldCounter = getPlotCounter();
		m_iPlotCounter = range(iOldCounter + iChange, 0, 100);
/** Disabled Sephi
		const int iPlotCounterDown	= GC.getTerrainInfo(getTerrainType()).getPlotCounterDown();
		const int iPlotCounterUp	= GC.getTerrainInfo(getTerrainType()).getPlotCounterUp();
		bool bChange = false;
		if (getPlotCounter() < iPlotCounterDown && iPlotCounterDown <= iOldCounter)
		{
			setTerrainType((TerrainTypes)GC.getTerrainInfo(getTerrainType()).getTerrainDown(), true, true);
			bChange = true;
		}
		if (iOldCounter <= iPlotCounterUp && iPlotCounterUp < getPlotCounter())
		{
			setTerrainType((TerrainTypes)GC.getTerrainInfo(getTerrainType()).getTerrainUp(), true, true);
			bChange = true;
		}

		if (bChange)
		{
			if (getFeatureType() != NO_FEATURE)
			{
				if (!GC.getFeatureInfo(getFeatureType()).isTerrain(getTerrainType()))
				{
					setFeatureType(NO_FEATURE);
				}
			}
		}
**/
	}
}

bool CvPlot::isPythonActive() const
{
    return m_bPythonActive;
}

void CvPlot::setPythonActive(bool bNewValue)
{
    m_bPythonActive = bNewValue;
}

bool CvPlot::isAdjacentToWater() const
{
	PROFILE_FUNC();

	CvPlot* pAdjacentPlot;
	int iI;

	for (iI = 0; iI < NUM_DIRECTION_TYPES; ++iI)
	{
		pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if (pAdjacentPlot != NULL)
		{
			if (pAdjacentPlot->isWater())
			{
				return true;
			}
		}
	}

	return false;
}

bool CvPlot::isBuilding(BuildTypes eBuild, TeamTypes eTeam, int iRange, bool bExcludeCenter) const
{
	CvPlot* pLoopPlot;
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
    for (int iDX = -iRange; iDX <= iRange; iDX++)
    {
        for (int iDY = -iRange; iDY <= iRange; iDY++)
        {
            pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
            if (pLoopPlot != NULL)
            {
                if (pLoopPlot->isOwned())
                {
                    if (pLoopPlot->getTeam() == eTeam)
                    {
                        if (iDX != 0 || iDY != 0 || !bExcludeCenter)
                        {
                            pUnitNode = pLoopPlot->headUnitNode();
                            while (pUnitNode != NULL)
                            {
                                pLoopUnit = ::getUnit(pUnitNode->m_data);
                                pUnitNode = pLoopPlot->nextUnitNode(pUnitNode);
                                if (pLoopUnit->getBuildType() == eBuild)
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
    return false;
}

int CvPlot::getRangeDefense(TeamTypes eDefender, int iRange, bool bFinal, bool bExcludeCenter) const
{
    int iModifier = 0;
    int iBestModifier = 0;
	CvPlot* pLoopPlot;
	ImprovementTypes eImprovement;
    for (int iDX = -iRange; iDX <= iRange; iDX++)
    {
        for (int iDY = -iRange; iDY <= iRange; iDY++)
        {
            pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
            if (pLoopPlot != NULL)
            {
                if (pLoopPlot->isOwned())
                {
                    eImprovement = pLoopPlot->getImprovementType();
                    iModifier = 0;
                    if (eImprovement != NO_IMPROVEMENT)
                    {
                        if (bFinal)
                        {
                            if (finalImprovementUpgrade(eImprovement) != NO_IMPROVEMENT)
                            {
                                eImprovement = finalImprovementUpgrade(eImprovement);
                            }
                        }
                        if (pLoopPlot->getTeam() == eDefender)
                        {
                            if (pLoopPlot->plotCheck(PUF_isEnemy, pLoopPlot->getOwner(), false, NO_PLAYER, NO_TEAM, NULL) == NULL)
                            {
                                if (iDX == 0 && iDY == 0)
                                {
                                    if (!bExcludeCenter)
                                    {
                                        iModifier = GC.getImprovementInfo(eImprovement).getDefenseModifier();
                                    }
                                }
                                else
                                {
                                    if (abs(iDX) <= GC.getImprovementInfo(eImprovement).getRange() && abs(iDY) <= GC.getImprovementInfo(eImprovement).getRange())
                                    {
                                        iModifier = GC.getImprovementInfo(eImprovement).getRangeDefenseModifier();
                                    }
                                }
                                if (iModifier > iBestModifier)
                                {
                                    iBestModifier = iModifier;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return iBestModifier;
}

TerrainTypes CvPlot::getRealTerrainType() const
{
	return (TerrainTypes)m_eRealTerrainType;
}

void CvPlot::setRealTerrainType(TerrainTypes eNewValue)
{
    m_eRealTerrainType = eNewValue;
}

void CvPlot::setTempTerrainType(TerrainTypes eNewValue, int iTimer)
{
    if (getTerrainType() != eNewValue)
    {
        if (getRealTerrainType() == NO_TERRAIN) //Dont overwrite the real terrain if we double temp
        {
            setRealTerrainType(getTerrainType());
        }

		setTerrainType(eNewValue, true, true);
		changeTempTerrainTimer(iTimer);
    }
}

int CvPlot::getTempTerrainTimer() const
{
    return m_iTempTerrainTimer;
}

void CvPlot::changeTempTerrainTimer(int iChange)
{
    if (iChange != 0)
    {
        m_iTempTerrainTimer += iChange;
    }
}

int CvPlot::getPortalExitX() const
{
    return m_iPortalExitX;
}

void CvPlot::setPortalExitX(int iNewValue)
{
    m_iPortalExitX = iNewValue;
}

int CvPlot::getPortalExitY() const
{
    return m_iPortalExitY;
}

void CvPlot::setPortalExitY(int iNewValue)
{
    m_iPortalExitY = iNewValue;
}
//FfH: End Add

/*************************************************************************************************/
/** Skyre Mod                                                                                   **/
/** BETTER AI (Lanun Pirate Coves) merged Sephi                                                 **/
/**						                                            							**/
/*************************************************************************************************/

bool CvPlot::isPirateCove() const
{
    return c_aePirateCoveTypes.count(getImprovementType()) != 0;
}

bool CvPlot::isPirateCoveValid(PlayerTypes ePlayer) const
{
    if (!isWater())
    {
        return false;
    }

    if (!isAdjacentToLand())
    {
        return false;
    }

    if (isCity())
    {
        return false;
    }

    if (getOwnerINLINE() != ePlayer)
    {
        return false;
    }

    if (getImprovementType() != NO_IMPROVEMENT)
    {
        return false;
    }

    if (getBonusType() != NO_BONUS)
    {
        return false;
    }

    for (int iX = -2; iX <= 2; iX++)
    {
        for (int iY = -2; iY <= 2; iY++)
        {
            CvPlot* pPlot = plotXY(getX_INLINE(), getY_INLINE(), iX, iY);

            if (pPlot != NULL && pPlot->isPirateCove())
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

/*************************************************************************************************/
/**	BETTER AI (New Functions Definition) Sephi                                 					**/
/*************************************************************************************************/
//Returns how many more of a given Type Defenders can replace others in close cities
int CvPlot::AI_neededBetterDefender(int type)
{
    if (getOwnerINLINE()==NO_PLAYER)
    {
        return 0;
    }
    CivilizationTypes Civili;
    if (isCity())
    {
        Civili=getPlotCity()->getCivilizationType();
    }
    else
    {
        Civili=GET_PLAYER(getOwnerINLINE()).getCivilizationType();
    }
    //make Sure we check a valid Unitclass
    switch (type)
    {
        case 1:
            if(GC.getCivilizationInfo(Civili).getBestDefender1()==NO_UNITCLASS)
            {
                return 0;
            }
            break;
        case 2:
            if(GC.getCivilizationInfo(Civili).getBestDefender2()==NO_UNITCLASS)
            {
                return 0;
            }
            break;
        case 3:
            if(GC.getCivilizationInfo(Civili).getBestDefender3()==NO_UNITCLASS)
            {
                return 0;
            }
            break;
        default:
            return 0;
    }

    int unitsneeded=0;
    int iSearchRange=8;
    int iDX,iDY;
    CvPlot* pLoopPlot;

	for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
	{
		for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
		{
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
			    if(pLoopPlot->isCity() && pLoopPlot->getOwnerINLINE()==getOwnerINLINE() && (pLoopPlot->getArea()==getArea()))
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
                            if(pLoopUnit->AI_getGroupflag()==GROUPFLAG_PERMDEFENSE && pLoopUnit->AI_getUnitAIType()==UNITAI_CITY_DEFENSE)
                            {
                                bool bvalid;
                                if (type==1)
                                {
                                    bvalid=true;
                                    if(pLoopUnit->getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender1())
                                        bvalid=false;
                                    if(pLoopUnit->getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender2())
                                        bvalid=false;
                                    if(pLoopUnit->getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender3())
                                        bvalid=false;
                                    if (bvalid)
                                        unitsneeded++;
                                }
                                if (type==2)
                                {
                                    bvalid=true;
                                    if(pLoopUnit->getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender2())
                                        bvalid=false;
                                    if(pLoopUnit->getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender3())
                                        bvalid=false;
                                    if (bvalid)
                                        unitsneeded++;
                                }
                                if (type==3)
                                {
                                    bvalid=true;
                                    if(pLoopUnit->getUnitClassType()==GC.getCivilizationInfo(Civili).getBestDefender3())
                                        bvalid=false;
                                    if (bvalid)
                                        unitsneeded++;
                                }
                            }
                        }
                    }
                }
            }
        }
	}
    return unitsneeded;
}

bool CvPlot::isAIGroupClose(PlayerTypes ePlayer, int iRange, int iAIGroupType, int iFlag)
{
	CvPlayerAI &kPlayer=GET_PLAYER(ePlayer);
	int iLoop;

	for(CvAIGroup* pAIGroup = kPlayer.firstAIGroup(&iLoop); pAIGroup != NULL; pAIGroup = kPlayer.nextAIGroup(&iLoop))
	{
		if(pAIGroup->getGroupType()==iAIGroupType)
		{
			if(iFlag!=1 || (pAIGroup->getMissionPlot()!=NULL && stepDistance(getX_INLINE(),getY_INLINE(),pAIGroup->getMissionPlot()->getX_INLINE(),pAIGroup->getMissionPlot()->getY_INLINE())<=iRange))
			{
				return true;
			}
		}
	}
	return false;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (multibarb)  Sephi	    	                                                    	**/
/**	AnimalDensity and BarBarianDensity                       									**/
/**							Adds extra Barbarian Civilizations									**/
/*************************************************************************************************/
int CvPlot::getAnimalDensity(bool bPlusLairs)
{
    if (bPlusLairs)
    {
        return m_iAnimalDensityPlusLairs;
    }
    return m_iAnimalDensity;
}
void CvPlot::calculateAnimalDensity()
{
    CvPlot* pLoopPlot;
    int iRange=3;
    int iCount = 0;
    int iCountPlusLairs = 0;
    int iDX, iDY;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot)
			{
                CLLNode<IDInfo>* pUnitNode;
                CvUnit* pLoopUnit;
                pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = nextUnitNode(pUnitNode);

                    if(pLoopUnit->getOwnerINLINE()==ANIMAL_PLAYER)
                    {
                        iCountPlusLairs++;
                        iCount ++;
                    }
                }
			}
        }
    }
    m_iAnimalDensity=iCount;
    m_iAnimalDensityPlusLairs=iCountPlusLairs;
}

int CvPlot::getBarBarianDensity(bool bPlusLairs)
{
    if (bPlusLairs)
    {
        return m_iBarBarianDensityPlusLairs;
    }
    return m_iBarBarianDensity;
}
void CvPlot::calculateBarBarianDensity()
{
    CvPlot* pLoopPlot;
    int iRange=3;
    int iCount = 0;
    int iCountPlusLairs = 0;
    int iDX, iDY;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot)
			{
                CLLNode<IDInfo>* pUnitNode;
                CvUnit* pLoopUnit;
                pUnitNode = pLoopPlot->headUnitNode();
                while (pUnitNode != NULL)
                {
                    pLoopUnit = ::getUnit(pUnitNode->m_data);
                    pUnitNode = nextUnitNode(pUnitNode);

                    if(pLoopUnit->getOwnerINLINE()==BARBARIAN_PLAYER)
                    {
                        iCountPlusLairs++;
                        if(pLoopUnit->getOriginPlot()==NULL)
                        {
                            iCount ++;
                        }
                    }
                }
			}
        }
    }
    m_iBarBarianDensity=iCount;
    m_iBarBarianDensityPlusLairs=iCountPlusLairs;
}

void CvPlot::doDevilGateSpawn()
{
	changePlotCounter(100);
	for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
	{
		CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
		if(pAdjacentPlot!=NULL)
		{
			pAdjacentPlot->changePlotCounter(100);
		}
	}

	if(GC.getGameINLINE().getSorenRandNum(100,"DevilGate AC increase")<20)
	{
		GC.getGameINLINE().changeGlobalCounter(1);
	}

	int iCountDevils=0;
	CvPlayer &kPlayer=GET_PLAYER((PlayerTypes)GC.getDEVIL_PLAYER());
	int iLoop;

	for(CvUnit* pLoopUnit = kPlayer.firstUnit(&iLoop); pLoopUnit != NULL; pLoopUnit = kPlayer.nextUnit(&iLoop))
	{
		if(pLoopUnit->getOriginPlot()==this)
		{
			iCountDevils++;
			if(iCountDevils>20)
			{
				return;
			}
		}
	}

	UnitTypes iSpawnUnit=NO_UNIT;
	int iValue=GC.getGameINLINE().getSorenRandNum(100,"Devilgate unit pick");

	int iDevilCaster = GC.getInfoTypeForString("UNIT_DEVIL_CASTER");
	int iDisciple = GC.getInfoTypeForString("UNIT_DEVIL_DISCIPLE");
	int iMelee = GC.getInfoTypeForString("UNIT_DEVIL_MELEE");

	int iDevilCasterSpawnChance = 2 + GC.getGameINLINE().getGlobalCounter()/800;
	int iDiscipleSpawnChance = iDevilCasterSpawnChance + 10 + GC.getGameINLINE().getGlobalCounter()/1000;
	int iMeleeCasterSpawnChance = iDiscipleSpawnChance + 5 + GC.getGameINLINE().getGlobalCounter()/400;

	if(iValue<iDevilCasterSpawnChance)
	{
		iSpawnUnit=(UnitTypes)iDevilCaster;
	}
	else if(iValue<iDiscipleSpawnChance)
	{
		iSpawnUnit=(UnitTypes)iDisciple;
	}
	else if(iValue<iMeleeCasterSpawnChance)
	{
		iSpawnUnit=(UnitTypes)iMelee;
	}

	if(iSpawnUnit!=NO_UNIT)
	{
		CvUnit* pNewUnit=kPlayer.initUnit(iSpawnUnit,getX_INLINE(),getY_INLINE());
		pNewUnit->setOriginPlot(this);
	}
}

//returns Special Yields of Plot times 100
int CvPlot::getCityYield(YieldTypes eYield)
{
	if(getImprovementType() != NO_IMPROVEMENT || isCity())
	{
		return 0;
	}
/**
	int iForest = GC.getInfoTypeForString("FEATURE_FOREST");
	int iForestAncient = GC.getInfoTypeForString("FEATURE_FOREST_ANCIENT");
	int iTundra = GC.getInfoTypeForString("TERRAIN_TUNDRA");
	int iPlains = GC.getInfoTypeForString("TERRAIN_PLAINS");
**/

	switch(eYield)
	{
		case YIELD_LUMBER:
			if(getFeatureType() == GC.getDefineINT("FEATURE_FOREST") || getFeatureType() == GC.getDefineINT("FEATURE_FOREST_ANCIENT"))
			{
				return 100;
			}
			break;
		case YIELD_LEATHER:
			if(getFeatureType() == GC.getDefineINT("FEATURE_FOREST") || getTerrainType() == GC.getDefineINT("TERRAIN_TUNDRA"))
			{
				return 51;
			}
			break;
		case YIELD_STONE:
			if(getFeatureType() == NO_FEATURE && 
				(getTerrainType() == GC.getDefineINT("TERRAIN_PLAINS") 
				|| getTerrainType() == GC.getDefineINT("TERRAIN_DESERT")))
			{
				return 100;
			}
			break;
		case YIELD_METAL:
			if(isHills())
			{
				return 100;
			}
			break;
		default:
			break;
	}
	return 0;
}

bool CvPlot::isValidforBarbSpecialAction()
{
    if(!isOwned())
    {
        return true;
    }

    if (GET_TEAM(getTeam()).isAtWar((TeamTypes)GC.getBARBARIAN_TEAM()))
    {
        return true;
    }

    return false;
}

int CvPlot::getDungeonType() const
{
	return m_iDungeonType;
}

void CvPlot::setDungeonType(int iNewValue)
{
	if(iNewValue!=m_iDungeonType)
	{
		m_iDungeonType=iNewValue;
		if(iNewValue!=NO_DUNGEON && GC.getDungeonInfo((DungeonTypes)getDungeonType()).getImprovementType()!=NO_IMPROVEMENT)
		{
			setImprovementType((ImprovementTypes)GC.getDungeonInfo((DungeonTypes)getDungeonType()).getImprovementType());
		}
	}
}

int CvPlot::getWilderness() const
{
	return m_iWilderness;
}

void CvPlot::setWilderness(int iNewValue)
{
	m_iWilderness = iNewValue;
}

PlaneTypes CvPlot::getPlane() const
{
	return (PlaneTypes)m_iPlane;
}

void CvPlot::setPlane(PlaneTypes iNewValue)
{
	m_iPlane = iNewValue;
}

bool CvPlot::isPlundered() const
{
	return m_iPlundered > 0;
}

void CvPlot::setPlundered(int iNewValue)
{
	m_iPlundered = iNewValue;
}

int CvPlot::getPlundered() const
{
	return m_iPlundered;
}

bool CvPlot::isWildmanaImmune() const
{
	return m_bWildmanaImmune;
}

void CvPlot::setWildmanaImmune(bool bNewValue)
{
	m_bWildmanaImmune=bNewValue;
}

bool CvPlot::isNonPlanar() const
{
	return m_bNonPlanar;
}

void CvPlot::setNonPlanar(bool bNewValue)
{
	if(bNewValue != m_bNonPlanar) {

		if(bNewValue) {
			if(!isPeak())
				if(GC.getInfoTypeForString("FEATURE_NON_PLANAR") != NO_FEATURE)
					setFeatureType((FeatureTypes)GC.getInfoTypeForString("FEATURE_NON_PLANAR"));
			//setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_HILL"));
		}

		m_bNonPlanar = bNewValue;
	}
}

bool CvPlot::doTerraformRitual(ProjectTypes eRitual, PlayerTypes eCaster, int& iValue, bool bCountOnly)
{
	CvProjectInfo &kProject=GC.getProjectInfo(eRitual);
	bool bValid=false;
	int iBaseYieldValue=50;

	FeatureTypes eFlood=(FeatureTypes)GC.getInfoTypeForString("FEATURE_FLOOD_PLAINS");
	FeatureTypes eOasis=(FeatureTypes)GC.getInfoTypeForString("FEATURE_OASIS");
	FeatureTypes eHaunted=(FeatureTypes)GC.getFEATURE_HAUNTED_LANDS();
	FeatureTypes eAncientForest=(FeatureTypes)GC.getInfoTypeForString("FEATURE_FOREST_ANCIENT");

	int iMod=0;
	CvCity* pCity=getWorkingCity();
	if(pCity!=NULL && pCity->getTeam()==getTeam())
	{
		iMod=2;
		if(isBeingWorked())
			iMod=4;
	}

	if(kProject.isHostileTerraform() && isOwned() && GET_PLAYER(getOwnerINLINE()).getResistHostileTerraforming()>0)
	{
		iMod/=2;
	}

	if(kProject.isCreateHills() && !isHills() && !isWater() && !isPeak())
	{
		bValid=true;
		if(!bCountOnly)
		{
			setPlotType(PLOT_HILLS);
		}

		if(pCity!=NULL && pCity->getProduction()<pCity->getPopulation())
		{
			iValue+=iBaseYieldValue*iMod;
		}
	}

	if(kProject.isCreateRiver()  && !isPeak())
	{
		if(!bCountOnly)
		{
			if(CvMapGenerator::GetInstance().addRiver(this))
			{
				bValid=true;
			}
		}
		iValue+=10;
	}

	if(kProject.isCreateLand() && isWater() && getNumUnits()==0)
	{
		bValid=true;
		if(!bCountOnly)
		{
			setPlotType(PLOT_LAND);
		}

		iValue+=2*iBaseYieldValue*iMod;
	}

	if(kProject.isDrownLand() && getPlotType()==PLOT_LAND && getNumUnits()==0 && !isCity())
	{
		bValid=true;
		if(!bCountOnly)
			setPlotType(PLOT_OCEAN);
	}

	if(eFlood!=NO_FEATURE)
	{
		if(kProject.isCreateFloodPlains() && isRiverSide() && getFeatureType()==NO_FEATURE && getTerrainType()==GC.getInfoTypeForString("TERRAIN_DESERT")  && !isPeak())
		{
			bValid=true;
			if(!bCountOnly)
				setFeatureType(eFlood);

			int iTotalYieldChange=0;
			for(int iI=0;iI<GC.getNUM_YIELD_TYPES();iI++)
			{
				iTotalYieldChange+=GC.getFeatureInfo(eFlood).getYieldChange(iI);
			}
			
			iValue+=iTotalYieldChange*iBaseYieldValue*iMod;
		}
	}

	if(kProject.isCreateRandom()) {
		bValid = true;
		if(!bCountOnly) {
			std::vector<int> possibleTerrains;
			std::vector<int> possibleFeatures;
			for(int i = 0; i < GC.getNumTerrainInfos(); ++i) {
				if(GC.getTerrainInfo((TerrainTypes)i).isRandomTerraform()) {
					possibleTerrains.push_back(i);
				}
			}
			possibleFeatures.push_back(NO_FEATURE);
			for(int i = 0; i < GC.getNumFeatureInfos(); ++i) {
				if(GC.getFeatureInfo((FeatureTypes)i).isRandomTerraform()) {
					possibleFeatures.push_back(i);
				}
			}

			TerrainTypes eNewTerrain = (TerrainTypes)possibleTerrains[GC.getGame().getSorenRandNum(possibleTerrains.size(), "pick random Terrain")];
			FeatureTypes eNewFeature = (FeatureTypes)possibleFeatures[GC.getGame().getSorenRandNum(possibleFeatures.size(), "pick random Terrain")];

			setTerrainType(eNewTerrain);
			setFeatureType(eNewFeature);
		}
	}

	if(kProject.isCreateHellTerrain() && getPlotCounter()==0)
	{
		bValid=true;
		if(!bCountOnly)
			changePlotCounter(100);

		if(isOwned())
		{
			if(GET_PLAYER(getOwnerINLINE()).getAlignment()!=GC.getInfoTypeForString("ALIGNMENT_EVIL"))
			{
				iValue+=-2*iBaseYieldValue*iMod;
			}
		}
	}

	if(kProject.isRemoveHellTerrain() && getPlotCounter()>0)
	{
		bValid=true;
		if(!bCountOnly)
			changePlotCounter(-getPlotCounter());

		if(isOwned())
		{
			if(GET_PLAYER(eCaster).isHuman() || GET_PLAYER(getOwnerINLINE()).getAlignment()!=GC.getInfoTypeForString("ALIGNMENT_EVIL"))
			{
				iValue+=2*iBaseYieldValue*iMod;
			}
		}
	}

	if(kProject.isRemovePeak() && isPeak())
	{
		if(!bCountOnly)
			setPlotType(PLOT_HILLS);
		bValid=true;

		int iTotalYieldChange=0;
		for(int iI=0;iI<GC.getNUM_YIELD_TYPES();iI++)
		{
			iTotalYieldChange+=GC.getTerrainInfo(getTerrainType()).getYield(iI);
		}
		
		iValue+=iTotalYieldChange*iBaseYieldValue*iMod;
	}

	if(kProject.isCreatePeak() && !isPeak() && !isCity() && !isWater())
	{
		if(!bCountOnly) {
			setPlotType(PLOT_PEAK);
			setFeatureType(NO_FEATURE);
			setImprovementType(NO_IMPROVEMENT);
		}
		bValid = true;

		int iTotalYieldChange = 0;

		//TODO 
		//AI adjust iValue
	}

	if(!isPeak()) {
		if(!((getFeatureType()==eFlood || getFeatureType()==eOasis) && getTerrainType()==GC.getInfoTypeForString("TERRAIN_DESERT")))
		{
			if(getOwnerINLINE()==NO_PLAYER || !GC.getCivilizationInfo((CivilizationTypes)GET_PLAYER(getOwnerINLINE()).getCivilizationType()).isNativeTerrains(getTerrainType()))
			{
				for(int iJ=0;iJ<5;iJ++)
				{
					TerrainTypes TerrainPre=NO_TERRAIN;
					TerrainTypes TerrainPost=NO_TERRAIN;

					if(iJ==0)
					{
						TerrainPre=(TerrainTypes)kProject.getTerrainType1();
						TerrainPost=(TerrainTypes)kProject.getTerrainToType1();
					}

					if(iJ==1)
					{
						TerrainPre=(TerrainTypes)kProject.getTerrainType2();
						TerrainPost=(TerrainTypes)kProject.getTerrainToType2();
					}

					if(iJ==2)
					{
						TerrainPre=(TerrainTypes)kProject.getTerrainType3();
						TerrainPost=(TerrainTypes)kProject.getTerrainToType3();
					}

					if(iJ==3)
					{
						TerrainPre=(TerrainTypes)kProject.getTerrainType4();
						TerrainPost=(TerrainTypes)kProject.getTerrainToType4();
					}

					if(iJ==4)
					{
						TerrainPre=(TerrainTypes)kProject.getTerrainType5();
						TerrainPost=(TerrainTypes)kProject.getTerrainToType5();
					}

					if(TerrainPre == getTerrainType() && TerrainPost != NO_TERRAIN)
					{
						if(!bCountOnly) {
							if(kProject.isHostileTerraform()) {
								int iLength = GC.getGameINLINE().getSorenRandNum(50, "hostile terraform");
								setTempTerrainType(TerrainPost, iLength);
							}
							else {
								setTerrainType(TerrainPost);
							}
						}
						bValid = true;

						int iTotalYieldChange = 0;
						for(int iI = 0; iI < GC.getNUM_YIELD_TYPES(); iI++)
						{
							iTotalYieldChange += GC.getTerrainInfo(TerrainPost).getYield(iI) - GC.getTerrainInfo(TerrainPre).getYield(iI);
						}
						
						iValue += iTotalYieldChange * iBaseYieldValue * iMod;
					}
				}
			}
		}
	}

	if(kProject.getFeatureTypePre() != NO_FEATURE || kProject.getFeatureTypePost() != NO_FEATURE)
	{
		if(kProject.getFeatureTypePre() == getFeatureType()  && !isPeak())
		{
			ImprovementTypes eImprovement = getImprovementType();
			if(eImprovement == NO_IMPROVEMENT 
				|| (isWater() && kProject.isWater())//allow Kelp
				|| (isOwned() && kProject.getFeatureTypePost() != NO_FEATURE && GC.getCivilizationInfo(GET_PLAYER(getOwnerINLINE()).getCivilizationType()).isMaintainFeatures(kProject.getFeatureTypePost())))
			{
				if(!isCity())
				{
					if(!bCountOnly)
					{
						setFeatureType((FeatureTypes)kProject.getFeatureTypePost());
					}
					bValid = true;

					int iTotalYieldChange = 0;
					for(int iI = 0; iI < GC.getNUM_YIELD_TYPES(); iI++)
					{
						if(kProject.getFeatureTypePre() != NO_FEATURE)
						{
							iTotalYieldChange -= GC.getFeatureInfo((FeatureTypes)kProject.getFeatureTypePre()).getYieldChange(iI);
						}
						if(kProject.getFeatureTypePost()!=NO_FEATURE)
						{
							iTotalYieldChange+=GC.getFeatureInfo((FeatureTypes)kProject.getFeatureTypePost()).getYieldChange(iI);
						}
					}
					
					iValue+=iTotalYieldChange*iBaseYieldValue*iMod;

					if(kProject.getFeatureTypePre()==eHaunted && kProject.getFeatureTypePost()!=eHaunted)
					{
						iValue+=200;
					}
					if(kProject.getFeatureTypePost()==eAncientForest)
					{
						iValue+=300;
					}
				}
			}
		}
	}

	if(kProject.getFeatureType2Pre()!=NO_FEATURE || kProject.getFeatureType2Post()!=NO_FEATURE)
	{
		if(kProject.getFeatureType2Pre()==getFeatureType()  && !isPeak())
		{
			if(!bCountOnly)
				setFeatureType((FeatureTypes)kProject.getFeatureType2Post());
			bValid=true;

			int iTotalYieldChange=0;
			for(int iI=0;iI<GC.getNUM_YIELD_TYPES();iI++)
			{
				if(kProject.getFeatureType2Pre()!=NO_FEATURE)
				{
					iTotalYieldChange-=GC.getFeatureInfo((FeatureTypes)kProject.getFeatureType2Pre()).getYieldChange(iI);
				}
				if(kProject.getFeatureType2Post()!=NO_FEATURE)
				{
					iTotalYieldChange+=GC.getFeatureInfo((FeatureTypes)kProject.getFeatureType2Post()).getYieldChange(iI);
				}
			}
			
			iValue+=iTotalYieldChange*iBaseYieldValue*iMod;

			if(kProject.getFeatureTypePre()==eHaunted && kProject.getFeatureTypePost()!=eHaunted)
			{
				iValue+=100;
			}
		}
	}

	if(kProject.getImprovementTypePre()!=NO_IMPROVEMENT || kProject.getImprovementTypePost()!=NO_IMPROVEMENT)
	{
		if(kProject.getImprovementTypePre()==NO_IMPROVEMENT || kProject.getImprovementTypePre()==getImprovementType())
		{
			if(!bCountOnly)
				setImprovementType((ImprovementTypes)kProject.getImprovementTypePost());
			bValid=true;

			int iTotalYieldChange=0;
			for(int iI=0;iI<GC.getNUM_YIELD_TYPES();iI++)
			{
				if(kProject.getImprovementTypePre()!=NO_IMPROVEMENT)
				{
					iTotalYieldChange-=GC.getImprovementInfo((ImprovementTypes)kProject.getImprovementTypePre()).getYieldChange(iI);
					iValue-=GC.getImprovementInfo((ImprovementTypes)kProject.getImprovementTypePre()).getDefenseModifier();
				}
				if(kProject.getImprovementTypePost()!=NO_IMPROVEMENT)
				{
					iTotalYieldChange+=GC.getImprovementInfo((ImprovementTypes)kProject.getImprovementTypePost()).getYieldChange(iI);
					iValue+=GC.getImprovementInfo((ImprovementTypes)kProject.getImprovementTypePost()).getDefenseModifier();
				}
			}
			
			iValue+=iTotalYieldChange*iBaseYieldValue*iMod;
		}
	}

	if(kProject.getImprovementType2Pre()!=NO_IMPROVEMENT || kProject.getImprovementType2Post()!=NO_IMPROVEMENT)
	{
		if(kProject.getImprovementType2Pre()==NO_IMPROVEMENT || kProject.getImprovementType2Pre()==getImprovementType())
		{
			if(!bCountOnly)
				setImprovementType((ImprovementTypes)kProject.getImprovementType2Post());
			bValid=true;

			int iTotalYieldChange=0;
			for(int iI=0;iI<GC.getNUM_YIELD_TYPES();iI++)
			{
				if(kProject.getImprovementType2Pre()!=NO_IMPROVEMENT)
				{
					iTotalYieldChange-=GC.getImprovementInfo((ImprovementTypes)kProject.getImprovementType2Pre()).getYieldChange(iI);
				}
				if(kProject.getImprovementType2Post()!=NO_IMPROVEMENT)
				{
					iTotalYieldChange+=GC.getImprovementInfo((ImprovementTypes)kProject.getImprovementType2Post()).getYieldChange(iI);
				}
			}
			
			iValue+=iTotalYieldChange*iBaseYieldValue*iMod;
		}
	}

	if(bValid && !bCountOnly)
	{
		if(GC.getGameINLINE().getActivePlayer()==getOwnerINLINE())
		{
			if(kProject.isHostileTerraform())
			{
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(),true,GC.getEVENT_MESSAGE_TIME(),kProject.getDescription(),0,MESSAGE_TYPE_MINOR_EVENT,kProject.getButton(),(ColorTypes)GC.getInfoTypeForString("COLOR_RED"),getX_INLINE(),getY_INLINE(),true,true);

				GET_TEAM(getTeam()).AI_changeWarValueAccumulated(GET_PLAYER(eCaster).getTeam(),-100);
				GET_TEAM(GET_PLAYER(eCaster).getTeam()).AI_changeWarValueAccumulated(getTeam(),100);
			}
			else
			{
				gDLL->getInterfaceIFace()->addMessage(getOwnerINLINE(),true,GC.getEVENT_MESSAGE_TIME(),kProject.getDescription(),0,MESSAGE_TYPE_MINOR_EVENT,kProject.getButton(),(ColorTypes)GC.getInfoTypeForString("COLOR_GREEN"),getX_INLINE(),getY_INLINE(),true,true);
			}
			
			if(kProject.getEffect()!=NO_EFFECT)
			{
				gDLL->getEngineIFace()->TriggerEffect(kProject.getEffect(), getPoint());
			}
		}
	}

	return bValid;
}

/*************************************************************************************************/
/**	ADDON (New Improvement System) Sephi                                 						**/
/*************************************************************************************************/
CvPlot* CvPlot::getcloseSecurePlot(TeamTypes eTeam) const
{
	CvPlot* pBestPlot=NULL;
	int iValue;
	int iBestValue=0;

	for(int iPass=0;iPass<2;iPass++)
	{
		if (pBestPlot!=NULL)
			break;
		for(int iDifX=-10;iDifX<10;iDifX++)
		{
			for(int iDifY=-10;iDifY<10;iDifY++)
			{
				CvPlot* pLoopPlot=GC.getMapINLINE().plot(getX_INLINE()+iDifX,getY_INLINE()+iDifY);
				iValue=10000-iDifX*iDifX-iDifY*iDifY;
				if(pLoopPlot!=NULL && pLoopPlot->getArea()==getArea())
				{
					if(iPass>0
						||(pLoopPlot->getTeam()==eTeam && pLoopPlot->isCity()))
					{
						if(iPass>1 || pLoopPlot->getTeam()==eTeam)
						{
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
	}

	return pBestPlot;
}

//function deactivated
bool CvPlot::canHaveImprovementClassType(ImprovementTypes eImprovement, int iClass) const
{
	return true;

	int iImprovementClass=GC.getImprovementInfo(eImprovement).getImprovementClassType();
	if(iClass!=-1)
	{
		iImprovementClass=iClass;
	}

	if(iImprovementClass==-1)
	{
		return true;
	}

	if(getBonusType(getTeam())!=NO_BONUS)
	{
		if(GC.getImprovementInfo(eImprovement).isImprovementBonusTrade(getBonusType(getTeam())))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if(iImprovementClass==0)
	{
		int iSearchRange=1;

		for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
		{
			for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
			{
				CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
				if(pLoopPlot!=NULL && pLoopPlot!=this)
				{
					if(pLoopPlot->getImprovementType()!=NO_IMPROVEMENT && GC.getImprovementInfo((ImprovementTypes)pLoopPlot->getImprovementType()).getImprovementClassType()==0)
					{
						return false;
					}
					if(pLoopPlot->getNumUnits()>0)
					{
						for (CLLNode<IDInfo>* pUnitNode = pLoopPlot->headUnitNode(); pUnitNode != NULL; pUnitNode = pLoopPlot->nextUnitNode(pUnitNode))
						{
							CvUnit* pLoopUnit = ::getUnit(pUnitNode->m_data);
							if(GC.getUnitInfo(pLoopUnit->getUnitType()).getWorkRate()>0)
							{
								if(pLoopUnit->getGroup()->headMissionQueueNode()!=NULL && pLoopUnit->getGroup()->headMissionQueueNode()->m_data.eMissionType==MISSION_BUILD)
								{
									BuildTypes eBuild=(BuildTypes)pLoopUnit->getGroup()->headMissionQueueNode()->m_data.iData1;
									if(eBuild!=NO_BUILD && GC.getBuildInfo(eBuild).getImprovement()!=NO_IMPROVEMENT)
									{
										if(GC.getImprovementInfo((ImprovementTypes)GC.getBuildInfo(eBuild).getImprovement()).getImprovementClassType()==0)
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

	return true;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/

/*************************************************************************************************/
/**	ADDON (Ranged Combat) Sephi								                     				**/
/**	Xienwolf Tweak							04/15/09											**/
/**																								**/
/**				Prevent Ranged attacks against units you won't actually harm					**/
/*************************************************************************************************/
CvUnit* CvPlot::getBestRangedDefender(PlayerTypes eOwner, PlayerTypes eAttackingPlayer, const CvUnit* pAttacker, bool bTestAtWar, bool bTestPotentialEnemy, bool bTestCanMove) const
{
	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;
	CvUnit* pBestUnit;

	pBestUnit = NULL;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if ((eOwner == NO_PLAYER) || (pLoopUnit->getOwnerINLINE() == eOwner))
		{
			if ((eAttackingPlayer == NO_PLAYER) || !(pLoopUnit->isInvisible(GET_PLAYER(eAttackingPlayer).getTeam(), false)))
			{
				if (!bTestAtWar || eAttackingPlayer == NO_PLAYER || pLoopUnit->isEnemy(GET_PLAYER(eAttackingPlayer).getTeam(), this) || (NULL != pAttacker && pAttacker->isEnemy(GET_PLAYER(pLoopUnit->getOwnerINLINE()).getTeam(), this)))
				{
					if (!bTestPotentialEnemy || (eAttackingPlayer == NO_PLAYER) ||  pLoopUnit->isPotentialEnemy(GET_PLAYER(eAttackingPlayer).getTeam(), this) || (NULL != pAttacker && pAttacker->isPotentialEnemy(GET_PLAYER(pLoopUnit->getOwnerINLINE()).getTeam(), this)))
					{
						if (!bTestCanMove || (pLoopUnit->canMove() && !(pLoopUnit->isCargo())))
						{
							if(!pLoopUnit->isImmuneToRangeStrike())
							{
								if ((pAttacker == NULL) || (pLoopUnit->getDamage() < pAttacker->airCombatLimit()))
								{
									if (pAttacker != NULL && pAttacker->isTargetWeakestUnit() && (pLoopUnit->isAlive() || pLoopUnit->isTargetWeakestUnitCounter()))
									{
										if(pBestUnit == NULL || (pLoopUnit->getDomainType() == pAttacker->getDomainType() && (pLoopUnit->baseCombatStr() != 0 || ::isLimitedUnitClass((UnitClassTypes)(pLoopUnit->getUnitClassType())))))
										{
											if (pBestUnit == NULL || (pBestUnit->isBetterDefenderThan(pLoopUnit, pAttacker) && !pBestUnit->isTargetWeakestUnitCounter()) || (pLoopUnit->isTargetWeakestUnitCounter() && (pLoopUnit->isBetterDefenderThan(pBestUnit, pAttacker) || !pBestUnit->isTargetWeakestUnitCounter())))
											{
												pBestUnit = pLoopUnit;
											}
										}
									}
									else
									{
										if (pLoopUnit->isBetterDefenderThan(pBestUnit, pAttacker) && (pAttacker == NULL || (pAttacker->getOwner() != pLoopUnit->getOwner())))
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

	return pBestUnit;
}
/*************************************************************************************************/
/**	Tweak									END													**/
/*************************************************************************************************/


/*************************************************************************************************/
/**	SPEEDTWEAK (CAR MOD) merged Sephi                                         					**/
/**																								**/
/**	                                                                 							**/
/*************************************************************************************************/
int CvPlot::getPlayerDangerCache(PlayerTypes ePlayer, int iRange)
{
	if (NULL == m_apaiPlayerDangerCache)
	{
		m_apaiPlayerDangerCache = new short*[MAX_PLAYERS];
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			m_apaiPlayerDangerCache[iI] = NULL;
		}
	}

	if (NULL == m_apaiPlayerDangerCache[ePlayer])
	{
		m_apaiPlayerDangerCache[ePlayer] = new short[DANGER_RANGE + 1];
		for (int iI = 0; iI < DANGER_RANGE + 1; ++iI)
		{
			m_apaiPlayerDangerCache[ePlayer][iI] = MAX_SHORT;
		}
	}

	return m_apaiPlayerDangerCache[ePlayer][iRange];
}

void CvPlot::setPlayerDangerCache(PlayerTypes ePlayer, int iRange, int iNewValue)
{
	if (NULL == m_apaiPlayerDangerCache)
	{
		m_apaiPlayerDangerCache = new short*[MAX_PLAYERS];
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			m_apaiPlayerDangerCache[iI] = NULL;
		}
	}

	if (NULL == m_apaiPlayerDangerCache[ePlayer])
	{
		m_apaiPlayerDangerCache[ePlayer] = new short[DANGER_RANGE + 1];
		for (int iI = 0; iI < DANGER_RANGE + 1; ++iI)
		{
			m_apaiPlayerDangerCache[ePlayer][iI] = MAX_SHORT;
		}
	}

	m_apaiPlayerDangerCache[ePlayer][iRange] = iNewValue;
}

void CvPlot::invalidatePlayerDangerCache(PlayerTypes ePlayer, int iRange)
{
	if (NULL == m_apaiPlayerDangerCache)
	{
		m_apaiPlayerDangerCache = new short*[MAX_PLAYERS];
		for (int iI = 0; iI < MAX_PLAYERS; ++iI)
		{
			m_apaiPlayerDangerCache[iI] = NULL;
		}
	}

	if (NULL == m_apaiPlayerDangerCache[ePlayer])
	{
		m_apaiPlayerDangerCache[ePlayer] = new short[DANGER_RANGE + 1];
		for (int iI = 0; iI < DANGER_RANGE + 1; ++iI)
		{
			m_apaiPlayerDangerCache[ePlayer][iI] = MAX_SHORT;
		}
	}

	m_apaiPlayerDangerCache[ePlayer][iRange] = MAX_SHORT;
}
/*************************************************************************************************/
/**	END	                                        												**/
/*************************************************************************************************/

void CvPlot::SpawnBarbarianUnit(UnitTypes eUnit, PlayerTypes ePlayer, bool bLeashed, UnitAITypes eUnitAI, bool bLoyalty)
{
	CvPlot* pTarget=NULL;

	int iSearchRange=2;
	int iValue;
	int iBestValue=0;

	if(!isLimitedPerTile(eUnit))
	{
		pTarget=this;
	}
	else
	{
		for (int iDX = -iSearchRange; iDX <= iSearchRange; ++iDX)
		{
			for (int iDY = -iSearchRange; iDY <= iSearchRange; ++iDY)
			{
				CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
				if(pLoopPlot!=NULL && pLoopPlot->getArea()==getArea())
				{
					if(pLoopPlot->isWater()==isWater())
					{
						if(!pLoopPlot->isPeak())
						{
							if(!pLoopPlot->isVisibleEnemyUnit(ePlayer))
							{
								if(pLoopPlot->getNumUnits()<GC.getGameINLINE().getUnitPerTileLimit())
								{
									iValue=50-stepDistance(getX_INLINE(),getY_INLINE(),pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE());
									if(iBestValue<iValue)
									{
										iBestValue=iValue;
										pTarget=pLoopPlot;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if(pTarget!=NULL)
	{
		CvUnit* pUnit;
		pUnit = GET_PLAYER(ePlayer).initUnit(eUnit,pTarget->getX_INLINE(),pTarget->getY_INLINE(), eUnitAI);
		if(bLeashed)
		{
			pUnit->setOriginPlot(this);
		}
		if(bLoyalty)
		{
			PromotionTypes eLoyalty=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_LOYALTY");
			if(eLoyalty!=NO_PROMOTION)
			{
				pUnit->setHasPromotion(eLoyalty, true);
			}
		}
	}
}

bool CvPlot::isLair() const
{
	if(getImprovementType()==NO_IMPROVEMENT)
	{
		return false;
	}

	if(GC.getImprovementInfo(getImprovementType()).getSpawnUnitCiv()!=GC.getDefineINT("BARBARIAN_CIVILIZATION"))
	{
		return false;
	}

	return true;
}

bool CvPlot::isWildmanaLair() const
{
	if(getNumVisibleUnits((PlayerTypes)GC.getWILDMANA_PLAYER())==0)
	{
		return false;
	}

	PromotionTypes iGuardian=(PromotionTypes)GC.getInfoTypeForString("PROMOTION_MANA_DEFENDER");

	CLLNode<IDInfo>* pUnitNode;
	CvUnit* pLoopUnit;

	pUnitNode = headUnitNode();

	while (pUnitNode != NULL)
	{
		pLoopUnit = ::getUnit(pUnitNode->m_data);
		pUnitNode = nextUnitNode(pUnitNode);

		if(pLoopUnit->isHasPromotion(iGuardian))
		{
			return true;
		}
	}

	return false;
}

bool CvPlot::isImprovementToExpensive(PlayerTypes eOwner, ImprovementTypes eNewImprovement) const
{
	CvPlayer& kPlayer=GET_PLAYER(eOwner);
	CvCity* pWorkingCity=getWorkingCity();
	
	if(pWorkingCity==NULL || getOwnerINLINE()==NO_PLAYER)
	{
		return false;
	}

	if(getBonusType(getTeam())!=NO_BONUS)
	{
		if(GC.getImprovementInfo(eNewImprovement).isImprovementBonusTrade(getBonusType(getTeam())))
		{
			return false;
		}
	}

	if(getImprovementType()==NO_IMPROVEMENT || GC.getImprovementInfo(getImprovementType()).getImprovementClassType()!=0)
	{
		int iFavoredImp=GC.getCivilizationInfo(kPlayer.getCivilizationType()).getImpInfrastructureHalfCost();
		if(iFavoredImp!=eNewImprovement || pWorkingCity->getNumFavoredImprovements()>=GC.getDefineINT("FREE_FAVORED_IMPROVEMENTS_PER_CITY"))
		{
			int iCost=pWorkingCity->calculateImprovementInfrastructureCost()/100;
			if(iCost>0)
			{
				return true;
			}
		}
	}

	return false;
}

bool CvPlot::isValidCityStateLocation() const
{
	if(getWilderness()<10)
	{
		return false;
	}

	if(getWilderness()>25)
	{
		return false;
	}

	if(isCity())
	{
		return false;
	}

	if(isPeak())
	{
		return false;
	}

	if(isWater())
	{
		return false;
	}

	if(GC.getTerrainInfo(getTerrainType()).getYield(YIELD_FOOD)+GC.getTerrainInfo(getTerrainType()).getYield(YIELD_PRODUCTION)<2)
	{
		return false;
	}

	int iSearchRange=5;

	for (int iX=-iSearchRange;iX<=iSearchRange;iX++)
	{
		for (int iY=-iSearchRange;iY<=iSearchRange;iY++)
		{
			CvPlot* pAdjacentPlot=plotXY(getX_INLINE(),getY_INLINE(),iX,iY);
			if(pAdjacentPlot!=NULL)
			{
				if(pAdjacentPlot->isCity())
				{
					return false;
				}
			}
		}
	}

	return true;
}

bool CvPlot::isAdjacentToPeak() const
{
	for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
	{
		CvPlot* pLoopPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iI));

		if(pLoopPlot!=NULL)
		{
			if(pLoopPlot->isPeak())
			{
				return true;
			}
		}
	}

	return false;
}

bool CvPlot::isImprovementOrUpgrade(ImprovementTypes eImprovement) const
{
	ImprovementTypes eUpgrade=eImprovement;
	while(eUpgrade!=NO_IMPROVEMENT)
	{
		if(eUpgrade==getImprovementType())
		{
			return true;
		}
		eUpgrade=(ImprovementTypes)GC.getImprovementInfo(eUpgrade).getImprovementUpgrade();
	}
	return false;
}

int CvPlot::StartingPlotValue(CivilizationTypes eCiv, int iPass) const
{
	if(isWater() || isPeak() || isStartingPlot()) {
		return 0;
	}

	int iValue = 0;
	int iRandom = 100;
	bool bOtherPlaneNear = false;
	bool bOtherStartingPlotVeryClose = false;
	bool bOtherStartingPlotTooClose = false;
	int iRange = 14;
	int iCountStartingPlots = 0;

	for(int iDX = -iRange; iDX <= iRange; iDX++) { 
		for( int iDY = -iRange; iDY <= iRange; iDY++ ) {
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);
			if(pLoopPlot != NULL && pLoopPlot != this) {
				if((abs(iDX) <= (iRange / 3)) && (abs(iDY) <= (iRange / 3))) {
					if(pLoopPlot->isStartingPlot()) {
						bOtherStartingPlotTooClose = true;
					}
				}
				if((abs(iDX) <= (iRange / 2)) && (abs(iDY) <= (iRange / 2))) {
					if(pLoopPlot->isStartingPlot()) {
						bOtherStartingPlotVeryClose = true;
					}
				}
				else if ((abs(iDX) <= iRange) && (abs(iDY) <= iRange)){
					if(pLoopPlot->isStartingPlot()) {
						++iCountStartingPlots;
					}
				}
				if(GC.getCivilizationInfo(eCiv).isNativeTerrains(pLoopPlot->getTerrainType())) {
					iValue += 30;
				}
			}
			else if(pLoopPlot == NULL && 
				(abs(iDX) <= iRange / 2) && (abs(iDY) <= iRange / 2)) {
				bOtherPlaneNear = true;
			}
		}
	}

	//added
	if(bOtherStartingPlotTooClose)
		return 0;

	if(!bOtherStartingPlotVeryClose) {
		if(iCountStartingPlots > 0) {
			iValue += iRandom;
//			if(iCountStartingPlots < 4) {
			if(iCountStartingPlots < 2) {
				iValue += iRandom;
			}
		}
	}

	if(iPass < 5) {
		if(bOtherStartingPlotVeryClose || bOtherPlaneNear)
			return 0;
	}

	if(iPass < 2) {
		if(area()->getNumTiles() < 30) {
			return 0;	
		}
		if(iPass == 0 && iCountStartingPlots > 0) {
		//if(iPass == 0 && iCountStartingPlots == 0) {
			return 0;	
		}
	}

	if(iPass == 2 || iPass == 3) {
		if(area()->getNumTiles() < 20) {
			return 0;	
		}
		if(iPass == 2 && iCountStartingPlots > 0) {
			return 0;	
		}
	}

	if(iPass == 4) {
		if(area()->getNumTiles() < 10) {
			if(iCountStartingPlots > 0)
				return 0;	
		}
	}

	if(iPass == 5 && bOtherStartingPlotVeryClose) {
		return 0;
	}

	return iValue + iRandom + GC.getGameINLINE().getSorenRandNum(iRandom, "Randomize Starting Location");
}

bool CvPlot::canBePillaged() const 
{
	if (isCity())
	{
		return false;
	}

	if (isPlundered())
	{
		return false;
	}

	if (getImprovementType() == NO_IMPROVEMENT)
	{
		if (!(isRoute()))
		{
			return false;
		}
	}
	else
	{
		if (GC.getImprovementInfo(getImprovementType()).isPermanent())
		{
			return false;
		}

		if(getDungeon() != NULL) {
			return false;
		}
	}

	return true;
}

void CvPlot::createDungeon(DungeonTypes eDungeon)
{
	if(eDungeon == NO_DUNGEON || getDungeon() != NULL)
		return;

	CvDungeon* pDungeon;

	pDungeon = GC.getMapINLINE().addDungeon();

	pDungeon->init(pDungeon->getID(), (int)eDungeon, GC.getMapINLINE().plotNum(getX_INLINE(), getY_INLINE()));

	m_iDungeon = pDungeon->getID();
}

void CvPlot::destroyDungeon()
{
	if(getDungeon() == NULL)
		return;

	getDungeon()->kill();
}

CvDungeon* CvPlot::getDungeon() const
{
	if(m_iDungeon == FFreeList::INVALID_INDEX)
	{
		return NULL;
	}
	else
	{
		return GC.getMapINLINE().getDungeon(m_iDungeon);
	}
}

void CvPlot::setPortalExit(int iPlotIndex) 
{
	m_iPortalExit = iPlotIndex;
}

CvPlot* CvPlot::getPortalExit() const
{
	if(m_iPortalExit == FFreeList::INVALID_INDEX)
	{
		return NULL;
	}
	else
	{
		return GC.getMapINLINE().plotByIndex(m_iPortalExit);
	}
}

bool CvPlot::isPortal() const
{
	return getPortalExit() != NULL;
}

bool CvPlot::canCreatePortal() const
{
	if(isNonPlanar())
		return false;

	if(isPortal() ||  isWater() || isCity() 
		|| (getImprovementType() != NO_IMPROVEMENT) 
		|| isPeak() || isStartingPlot()){
		return false;
	}

	if(isInRangePortal(5)) {
		return false;
	}

	if(area()->getNumTiles() < 5)
		return false;

	return true;
}

bool CvPlot::isInRangePortal(int iRange) const
{
    CvPlot* pLoopPlot;
    int iDX, iDY;

	for (iDX = -(iRange); iDX <= iRange; iDX++) {
		for (iDY = -(iRange); iDY <= iRange; iDY++) {
			pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL) {
				if(pLoopPlot->getArea() == getArea()) {
					if(pLoopPlot->isPortal()) {
						return true;					
					}
				}
			}
		}
	}
	return false;
}

int CvPlot::getID() const
{
	return GC.getMapINLINE().plotNum(getX_INLINE(), getY_INLINE());
}

void CvPlot::createPortal(int iTargetArea)
{
	ImprovementTypes ePortalImprovement = (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_DEVIL_PORTAL");

	if(ePortalImprovement == NO_IMPROVEMENT)
		return;

	CvPlot* bTarget = NULL;
	int iValue, iBestValue = 0;
	//search for possible Exit

	for(int iPass = 0; iPass < 4; ++iPass) {
		for(int i = 0; i < GC.getMapINLINE().numPlots(); ++i) {
			CvPlot* pLoopPlot = GC.getMapINLINE().plotByIndex(i);
			if(pLoopPlot != this) {
				if(iTargetArea == -1 || pLoopPlot->getArea() == iTargetArea) {
					if(pLoopPlot->getArea() != getArea() || iPass > 2) {
						if(!pLoopPlot->area()->canAreaBeReachedThroughPortal(getArea()) || iPass > 1) {
							if(pLoopPlot->area()->getNumPortals() == 0 || iPass > 0) {
								if(pLoopPlot->canCreatePortal()) {
									iValue = pLoopPlot->area()->getNumTiles() / (1 + pLoopPlot->area()->getNumPortals());
									if (iValue > iBestValue) {
										 bTarget = pLoopPlot;
										 iBestValue = iValue;
									}
								}
							}
						}
					}
				}
			}
		}
		if(bTarget != NULL)
			break;
	}

	if(bTarget == NULL)
		return;

	setImprovementType(ePortalImprovement);
	setPortalExit(bTarget->getID());
	area()->changeNumPortals(1);
	doLairDefenderSpawn();
	bTarget->setImprovementType(ePortalImprovement);
	bTarget->setPortalExit(getID());
	bTarget->area()->changeNumPortals(1);
	bTarget->doLairDefenderSpawn();
	TCHAR szOut[1024];
	TCHAR szFile[1024];
	sprintf(szFile, "Portals.log");

	sprintf(szOut, "Portal X, Y: %d, %d\n", getPortalExit()->getX_INLINE(), getPortalExit()->getY_INLINE());
	gDLL->logMsg(szFile,szOut, false, false);
	sprintf(szOut, "Portal Target X, Y: %d, %d\n", bTarget->getPortalExit()->getX_INLINE(), bTarget->getPortalExit()->getY_INLINE());
	gDLL->logMsg(szFile,szOut, false, false);
}

void CvPlot::destroyPortal()
{
	if(isPortal()) {
		setImprovementType(NO_IMPROVEMENT);
		setPortalExit(-1);
		area()->changeNumPortals(-1);
	}
}

int CvPlot::getLandBonusDiscoverChance(BonusTypes eBonus, PlayerTypes ePlayer) const {
	if (getBonusType() != NO_BONUS)
		return 0;

	if ((getImprovementType() == NO_IMPROVEMENT || eBonus == NO_BONUS))
		return 0;

	return getPossibleLandBonusDiscoverChance(eBonus, ePlayer, GC.getImprovementInfo(getImprovementType()));
}

int CvPlot::getPossibleLandBonusDiscoverChance(BonusTypes eBonus, PlayerTypes ePlayer, CvImprovementInfo& kImprovement) const
{
	if (!isOwned())
		return 0;

	if (kImprovement.getPrereqTerrainSpreadResources() != NO_TERRAIN &&
		kImprovement.getPrereqTerrainSpreadResources() != getTerrainType())
		return 0;

	if (getWilderness() < GC.getBonusInfo(eBonus).getMinWilderness()
		|| getWilderness() > GC.getBonusInfo(eBonus).getMaxWilderness())
		return 0;

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	TechTypes eTechReveal = (TechTypes)GC.getBonusInfo(eBonus).getTechReveal();

	if (eTechReveal != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechReveal)))
		return 0;

	TechTypes eTechDiscover = (TechTypes)GC.getBonusInfo(eBonus).getTechDiscover();
	if (eTechDiscover != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechDiscover)))
		return 0;

	int iBase = kImprovement.getImprovementBonusDiscoverRand(eBonus);

	if (iBase == 0)
		return 0;

	//GLOBAL ENCHANTMENTS
	ProjectTypes eFertileLands = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_FERTILE_LANDS");
	if (eFertileLands != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eFertileLands) > 0)
	{
		if (GC.getProjectInfo(eFertileLands).getManaBonusLinked() != NO_BONUS) {
			//OLD: iBase += 3 +//0 GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses(GC.getProjectInfo(eFertileLands).getManaBonusLinked());
			iBase *= 2;
			iBase *= 100 + 20 * GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses(GC.getProjectInfo(eFertileLands).getManaBonusLinked());
			iBase /= 100;
		}
	}

	//if(GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN))
	{
		int iMod = 100;
		ProjectTypes eArid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_ARID");
		if (eArid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eArid) > 0)
		{	//We already know the improvement can discover the resource, but we want to make sure it can discover a mining resource before applying the bonus.
			if (GC.getImprovementInfo((ImprovementTypes) GC.getInfoTypeForString("IMPROVEMENT_MINE")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
				GC.getImprovementInfo((ImprovementTypes) GC.getInfoTypeForString("IMPROVEMENT_QUARRY_SCIONS")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WINERY")).getImprovementBonusDiscoverRand(eBonus) > 0) {	//IRL wineries thrive with long warm summers
				iMod += 40;
			}
		}

		ProjectTypes eHumid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_HUMID");
		if (eHumid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eHumid) > 0)
		{	//All food and forest bonus discover chances are increased
			if (kImprovement.getYieldChange(YIELD_FOOD) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_FOREST_LODGE")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_CAMP")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WINERY")).getImprovementBonusDiscoverRand(eBonus) > 0) { //IRL wineries thrive with rainy winters
				iMod += 40;
			}
		}

		ProjectTypes eGrow = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_EVERGROW");
		if (eGrow != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eGrow) > 0)
		{	//All food and forest bonus discover chances are increased
			if (kImprovement.getYieldChange(YIELD_FOOD) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_FOREST_LODGE")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_CAMP")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WINERY")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
				GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_PLANTATION")).getImprovementBonusDiscoverRand(eBonus) > 0) {
				iMod += 50;
			}
		}

		if (eArid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eArid) > 0 && eHumid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eHumid) > 0)
		{
			if (GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_PLANTATION")).getImprovementBonusDiscoverRand(eBonus) > 0) {
				iMod += 20;	//As of this writing there are 8 bonuses for plantations vs. only 3 for farms, and each bonus rolls once per turn so balance it here.
			}
		}
		iBase *= iMod;
		iBase /= 100;
	}

	//Player
	if (isOwned())
	{
		iBase += kPlayer.getDiscoverRandModifier();
	}

	//Adjacent cottages and trade post
	bool adjacentBonusFound = false;
	int iModifier = 0;	//Fertile lands shouldn't increase the chance of cottages to find resources
	if (isOwned())
	{
		for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
			if (pAdjacentPlot != NULL) {
				if (pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
					if (pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT) {
						int iTempModifier = (GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify());
						/*
						//TODO: set in XML
                        if(
							pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_COTTAGE")
							|| pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_FROZEN_COTTAGE")
							|| pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_INFERNAL_COTTAGE")
						){
							iTempModifier=1;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_HAMLET​")){
							iTempModifier=2;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_VILLAGE")){
							iTempModifier=3;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_TOWN")){
							iTempModifier=4;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_ENCLAVE")){
							iTempModifier=5;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_TRADE1")){
							iTempModifier=1;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_TRADE2")){
							iTempModifier=2;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_TRADE3")){
							iTempModifier=3;
						}else if(pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_CARAVAN_CENTER")
							|| pAdjacentPlot->getImprovementType() == (ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_CARAVANSARY")){
							iTempModifier=4;
						}*/
						iModifier += iTempModifier;
						/*
						if(GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN))
						{
							if(iTempModifier > iModifier)
							{
								iModifier = iTempModifier;
							}
						}
						else
						{
							iModifier +//0= iTempModifier;
						}
						*/
					}
					if (pAdjacentPlot->getBonusType(kPlayer.getTeam()) == eBonus) {
						adjacentBonusFound = true;
					}
				}
			}
		}
	}
	iBase += GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN) ? iModifier : 0;

	//Merchant district
	if (GC.getBUILDINGCLASS_MERCHANT_DISTRICT() != NO_BUILDINGCLASS)
	{
		if(GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN))
		{
			iBase *= 100 + 20 * kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT());
			iBase /= 100;
		}
		else
		{
			iModifier *= 100 + 20 * kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT());
			iModifier /= 100;
		}
	}

	//Adjacent resource
	if (adjacentBonusFound)
	{
		if(GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN))
		{
			iBase *= 2;
		}
		else
		{
			iBase += iModifier;
		}
	}

	iBase = range(iBase, 0, 100);

	//Scale by speed
	iBase *= 100;
	iBase /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();

	//AI
	/*
	if (!kPlayer.isHuman()) {
		iBase *= 100;
		iBase /= GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIGrowthPercent();
		//iBase = iBase / (GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIGrowthPercent() / 100.0);		//TODO: AI needs lots of help to develop.  Likes to spam one improvement type rather than buff with complementary cottages.
	}
	*/
	return iBase;




/*
	//TODO: This doesn't show the correct values in debug mode.
	if (!isOwned())
		return 0;

	if (kImprovement.getPrereqTerrainSpreadResources() != NO_TERRAIN &&
		kImprovement.getPrereqTerrainSpreadResources() != getTerrainType())
		return 0;

	if (getWilderness() < GC.getBonusInfo(eBonus).getMinWilderness()
		|| getWilderness() > GC.getBonusInfo(eBonus).getMaxWilderness())
		return 0;

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	TechTypes eTechReveal = (TechTypes)GC.getBonusInfo(eBonus).getTechReveal();

	if (eTechReveal != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechReveal)))
		return 0;

	TechTypes eTechDiscover = (TechTypes)GC.getBonusInfo(eBonus).getTechDiscover();
	if (eTechDiscover != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechDiscover)))
		return 0;

	double iBase = kImprovement.getImprovementBonusDiscoverRand(eBonus);

	if (iBase == 0)
		return 0;

	ProjectTypes eFertileLands = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_FERTILE_LANDS");
	if (eFertileLands != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eFertileLands) > 0)
	{
		if (GC.getProjectInfo(eFertileLands).getManaBonusLinked() != NO_BONUS) {
			iBase = (iBase * 2);
			if (GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses(GC.getProjectInfo(eFertileLands).getManaBonusLinked()) > 0)
				iBase = iBase + (iBase * (.2 * GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses(GC.getProjectInfo(eFertileLands).getManaBonusLinked())));
		}
	}

	int iMod = 100;
	ProjectTypes eArid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_ARID");
	if (eArid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eArid) > 0)
	{	//We already know the improvement can discover the resource, but we want to make sure it can discover a mining resource before applying the bonus.
		if (GC.getImprovementInfo((ImprovementTypes) GC.getInfoTypeForString("IMPROVEMENT_MINE")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
			GC.getImprovementInfo((ImprovementTypes) GC.getInfoTypeForString("IMPROVEMENT_QUARRY_SCIONS")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
			GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WINERY")).getImprovementBonusDiscoverRand(eBonus) > 0) {	//IRL wineries thrive with long warm summers
			iMod += 40;
		}
	}

	ProjectTypes eHumid = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_CLIMATE_HUMID");
	if (eHumid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eHumid) > 0)
	{	//All food and forest bonus discover chances are increased
		if (kImprovement.getYieldChange(YIELD_FOOD) > 0 || GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_FOREST_LODGE")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
			GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_CAMP")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
			GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WINERY")).getImprovementBonusDiscoverRand(eBonus) > 0) { //IRL wineries thrive with rainy winters
			iMod += 40;
		}
	}

	ProjectTypes eGrow = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_EVERGROW");
	if (eGrow != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eGrow) > 0)
	{	//All food and forest bonus discover chances are increased
		if (kImprovement.getYieldChange(YIELD_FOOD) > 0 || GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_FOREST_LODGE")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
			GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_CAMP")).getImprovementBonusDiscoverRand(eBonus) > 0 ||
			GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_WINERY")).getImprovementBonusDiscoverRand(eBonus) > 0 || 
			GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_PLANTATION")).getImprovementBonusDiscoverRand(eBonus) > 0) { 
			iMod += 50;
		}
	}

	if (eArid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eArid) > 0 && eHumid != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eHumid) > 0)
	{	
		if (GC.getImprovementInfo((ImprovementTypes)GC.getInfoTypeForString("IMPROVEMENT_PLANTATION")).getImprovementBonusDiscoverRand(eBonus) > 0) {
			iMod += 20;	//As of this writing there are 8 bonuses for plantations vs. only 3 for farms, and each bonus rolls once per turn so balance it here.
		}
	}

	bool adjacentBonusFound = false;
	double iModifier = 0;	//Fertile lands shouldn't increase the chance of cottages to find resources
	if (isOwned())
	{
		iModifier += kPlayer.getDiscoverRandModifier();

		for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
			if (pAdjacentPlot != NULL) {
				if (pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
					if (pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT) {
						//iModifier += (GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify());
						int iTempModifier = (GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify());
						if(iTempModifier!=0){
							if(iTempModifier==3){iModifier+=0;}
							else if(iTempModifier==6){iModifier+=1;}
							else if(iTempModifier==9){iModifier+=2;}
							else if(iTempModifier==10){iModifier+=1;}
							else if(iTempModifier==15){iModifier+=3;}
							else if(iTempModifier==20){iModifier+=2;}
							else if(iTempModifier==21){iModifier+=5;}
							else if(iTempModifier==30){iModifier+=3;}
						}
					}
					if (pAdjacentPlot->getBonusType(kPlayer.getTeam()) == eBonus) {
						adjacentBonusFound = true;
					}
				}
			}
		}
	}
	iBase = (iBase * iMod) / 100;
	iBase += iModifier;

	if (GC.getBUILDINGCLASS_MERCHANT_DISTRICT() != NO_BUILDINGCLASS && kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT()) > 0)
	{
		iBase *= 1 + (0.20 * kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT()));
	}

	if (adjacentBonusFound)
		iBase *= 2;

	//iBase *= 1000;
	iBase *= 500; //SpyFanatic: scale down probability of a factor 2
	iBase = range((int) iBase, 0, 10000000);


//	iBase = range(iBase, 0, 100);
//	iBase *= 100;

	iBase /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();

	if (!kPlayer.isHuman()) {
		iBase = iBase / (GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIGrowthPercent() / 100.0);		//TODO: AI needs lots of help to develop.  Likes to spam one improvement type rather than buff with complementary cottages.
	}

	oosLog("AIPlotDiscovery"
		,"Turn: %d,Owner:%d,X:%d,Y:%d,Bonus:%S,iBase:%d,iModifier:%d,iMod:%d,MerchantDistrict:%d,adjacentBonusFound:%d,getDiscoverRandModifier:%d,getImprovementBonusDiscoverRand:%d"
		,GC.getGameINLINE().getElapsedGameTurns()
		,getOwner()
		,getX()
		,getY()
		,GC.getBonusInfo(eBonus).getDescription()
		,(int) iBase
		,(int) iModifier
		,iMod
		,kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT())
		,adjacentBonusFound
		,kPlayer.getDiscoverRandModifier()
		,kImprovement.getImprovementBonusDiscoverRand(eBonus)
	);

	return (int) iBase;
	*/
}

int CvPlot::getPossibleWaterBonusDiscoverChance(BonusTypes eBonus, PlayerTypes ePlayer) const {

	if (!GC.getGameINLINE().isOption(GAMEOPTION_INCREASE_RESOURCE_SPAWN))
		return 0;

	if (!isOwned() || getBonusType() != NO_BONUS)
		return 0;

	if (!isWater())
		return 0;

	if (getWilderness() < GC.getBonusInfo(eBonus).getMinWilderness()
		|| getWilderness() > GC.getBonusInfo(eBonus).getMaxWilderness())
		return 0;

	if (!GC.getBonusInfo(eBonus).isTerrain(getTerrainType()))
		return 0;

	if (getImprovementType() != NO_IMPROVEMENT && GC.getImprovementInfo(getImprovementType()).isUnique()) //SpyFanatic: do now spawn water resources on improvement (maelstrom)
		return 0;

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	TechTypes eTechReveal = (TechTypes)GC.getBonusInfo(eBonus).getTechReveal();

	if (eTechReveal != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechReveal)))
		return 0;

	TechTypes eTechDiscover = (TechTypes)GC.getBonusInfo(eBonus).getTechDiscover();
	if (eTechDiscover != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechDiscover)))
		return 0;

	int iBase = GC.getBonusInfo(eBonus).getDiscoverRandModifier();

	if (iBase <= 0)
		return 0;

	//River
	bool riverFound = false;
	for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
	{
		CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
		if (pAdjacentPlot != NULL && pAdjacentPlot->isWater()) {
			if (pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
				if (((pAdjacentPlot->getBonusType(kPlayer.getTeam()) == eBonus && eBonus != GC.getInfoTypeForString("BONUS_WHALE"))
					|| (pAdjacentPlot->getBonusType(kPlayer.getTeam()) == GC.getInfoTypeForString("BONUS_FISH") && eBonus == GC.getInfoTypeForString("BONUS_WHALE")))) {
					if(pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT){ //SpyFanatic: only if adjacentPlot has an improvement
						iBase += GC.getBonusInfo(eBonus).getDiscoverRandModifier();
					}
				}
			}
		}
		else if (pAdjacentPlot != NULL && pAdjacentPlot->isRiver()) {
			if (pAdjacentPlot->getRiverNSDirection() == CARDINALDIRECTION_NORTH) {
				if ((DirectionTypes)iK == DIRECTION_SOUTH || (DirectionTypes)iK == DIRECTION_SOUTHWEST) {
					riverFound = true;
				}
			}
			else if (pAdjacentPlot->getRiverNSDirection() == CARDINALDIRECTION_SOUTH) {
				if ((DirectionTypes)iK == DIRECTION_NORTH || (DirectionTypes)iK == DIRECTION_NORTHWEST) {
					riverFound = true;
				}
			}
			else if (pAdjacentPlot->getRiverWEDirection() == CARDINALDIRECTION_EAST) {
				if ((DirectionTypes)iK == DIRECTION_WEST || (DirectionTypes)iK == DIRECTION_NORTHWEST) {
					riverFound = true;
				}
			}
			else if (pAdjacentPlot->getRiverWEDirection() == CARDINALDIRECTION_WEST) {
				if ((DirectionTypes)iK == DIRECTION_EAST || (DirectionTypes)iK == DIRECTION_NORTHEAST) {
					riverFound = true;
				}
			}
		}
	}

	//Global Enchantments
	ProjectTypes eRichSeas = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_RICHES_OF_THE_SEA");
	if (eRichSeas != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eRichSeas) > 0)
	{
		if (GC.getProjectInfo(eRichSeas).getManaBonusLinked() != NO_BONUS && kPlayer.getNumAvailableBonuses(GC.getProjectInfo(eRichSeas).getManaBonusLinked()) > 0) {
			iBase *= 100 + 20 * kPlayer.getNumAvailableBonuses(GC.getProjectInfo(eRichSeas).getManaBonusLinked());
			iBase /= 100;
		}
	}

	if ((getImprovementType() != NO_IMPROVEMENT)) {	//Improvements should be an additive multiplier not affected by riches of the sea.
		CvImprovementInfo& kImprovement = GC.getImprovementInfo(getImprovementType());
		iBase += kImprovement.getImprovementBonusDiscoverRand(eBonus);
	}

	int iModifier = 100;
	if (kPlayer.isHasTech(GC.getInfoTypeForString("TECH_SEAFARING")))
		iModifier += 100;

	if(getFeatureType() == GC.getInfoTypeForString("FEATURE_KELP"))
		iModifier += 100;

	if ( kPlayer.getStateReligion() == RELIGION_OCTOPUS_OVERLORDS)
		iModifier += 50;

	if (getWorkingCity() != 0 && (getWorkingCity()->getNumActiveBuilding((BuildingTypes)GC.getInfoTypeForString("BUILDING_HARBOR", false)) > 0 || getWorkingCity()->getNumActiveBuilding((BuildingTypes) GC.getInfoTypeForString("BUILDING_HARBOR_LANUN", false)) > 0))
		iModifier += 50;

	if (kPlayer.getBuildingClassCount((BuildingClassTypes) GC.getInfoTypeForString("BUILDINGCLASS_HERON_THRONE", false)) > 0)
		iModifier += 20;

	if (riverFound)
		iModifier += 50;

	iBase *= iModifier;
	iBase /= 100;


	iBase = range(iBase, 0, 100);

	iBase *= 100;
	iBase /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();

	/*if (!kPlayer.isHuman()) {
		iBase = iBase / (GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIGrowthPercent() / 100.0);	//TODO: AI needs lots of help to develop.  Likes to spam one improvement type rather than buff with complementary cottages.
	}*/

	return iBase;

/*
	if (!isOwned() || getBonusType() != NO_BONUS)
		return 0;

	if (!isWater()) 
		return 0;

	if (getWilderness() < GC.getBonusInfo(eBonus).getMinWilderness()
		|| getWilderness() > GC.getBonusInfo(eBonus).getMaxWilderness())
		return 0;

	if (!GC.getBonusInfo(eBonus).isTerrain(getTerrainType()))
		return 0;

	if (getImprovementType() != NO_IMPROVEMENT && GC.getImprovementInfo(getImprovementType()).isUnique()) //SpyFanatic: do now spawn water resources on improvement (maelstrom)
		return 0;

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	TechTypes eTechReveal = (TechTypes)GC.getBonusInfo(eBonus).getTechReveal();

	if (eTechReveal != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechReveal)))
		return 0;

	TechTypes eTechDiscover = (TechTypes)GC.getBonusInfo(eBonus).getTechDiscover();
	if (eTechDiscover != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechDiscover)))
		return 0;

	double iBase = GC.getBonusInfo(eBonus).getDiscoverRandModifier();

	if (iBase <= 0)
		return 0;

	bool riverFound = false;
	for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
	{
		CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
		if (pAdjacentPlot != NULL && pAdjacentPlot->isWater()) {
			if (pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
				if (((pAdjacentPlot->getBonusType(kPlayer.getTeam()) == eBonus && eBonus != GC.getInfoTypeForString("BONUS_WHALE"))
					|| (pAdjacentPlot->getBonusType(kPlayer.getTeam()) == GC.getInfoTypeForString("BONUS_FISH") && eBonus == GC.getInfoTypeForString("BONUS_WHALE")))) {
					iBase += GC.getBonusInfo(eBonus).getDiscoverRandModifier();
				}
			}
		}
		else if (pAdjacentPlot != NULL && pAdjacentPlot->isRiver()) {
			if (pAdjacentPlot->getRiverNSDirection() == CARDINALDIRECTION_NORTH) {
				if ((DirectionTypes)iK == DIRECTION_SOUTH || (DirectionTypes)iK == DIRECTION_SOUTHWEST) {
					riverFound = true;
				}
			}
			else if (pAdjacentPlot->getRiverNSDirection() == CARDINALDIRECTION_SOUTH) {
				if ((DirectionTypes)iK == DIRECTION_NORTH || (DirectionTypes)iK == DIRECTION_NORTHWEST) {
					riverFound = true;
				}
			}
			else if (pAdjacentPlot->getRiverWEDirection() == CARDINALDIRECTION_EAST) {
				if ((DirectionTypes)iK == DIRECTION_WEST || (DirectionTypes)iK == DIRECTION_NORTHWEST) {
					riverFound = true;
				}
			}
			else if (pAdjacentPlot->getRiverWEDirection() == CARDINALDIRECTION_WEST) {
				if ((DirectionTypes)iK == DIRECTION_EAST || (DirectionTypes)iK == DIRECTION_NORTHEAST) {
					riverFound = true;
				}
			}				
		}
	}

	ProjectTypes eRichSeas = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_RICHES_OF_THE_SEA");
	if (eRichSeas != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eRichSeas) > 0)
	{
		if (GC.getProjectInfo(eRichSeas).getManaBonusLinked() != NO_BONUS && kPlayer.getNumAvailableBonuses(GC.getProjectInfo(eRichSeas).getManaBonusLinked()) > 0) {
			iBase = iBase + (iBase * (.2 * kPlayer.getNumAvailableBonuses(GC.getProjectInfo(eRichSeas).getManaBonusLinked())));
		}
	}

	if ((getImprovementType() != NO_IMPROVEMENT)) {	//Improvements should be an additive multiplier not affected by riches of the sea.
		CvImprovementInfo& kImprovement = GC.getImprovementInfo(getImprovementType());
		iBase += kImprovement.getImprovementBonusDiscoverRand(eBonus);
	}

	int iModifier = 100;
	if (kPlayer.isHasTech(GC.getInfoTypeForString("TECH_SEAFARING")))
		iModifier += 100;

	if(getFeatureType() == GC.getInfoTypeForString("FEATURE_KELP"))
		iModifier += 100;

	if ( kPlayer.getStateReligion() == RELIGION_OCTOPUS_OVERLORDS)
		iModifier += 50;

	if (getWorkingCity() != 0 && (getWorkingCity()->getNumActiveBuilding((BuildingTypes)GC.getInfoTypeForString("BUILDING_HARBOR", false)) > 0 || getWorkingCity()->getNumActiveBuilding((BuildingTypes) GC.getInfoTypeForString("BUILDING_HARBOR_LANUN", false)) > 0))
		iModifier += 50;

	if (kPlayer.getBuildingClassCount((BuildingClassTypes) GC.getInfoTypeForString("BUILDINGCLASS_HERON_THRONE", false)) > 0)
		iModifier += 20;

	if (riverFound)
		iModifier += 50;
	
	iBase = (iBase * iModifier) / 100;

	iBase /= 3;

	//iBase *= 1000;
	iBase *= 500; //SpyFanatic: scale down probability of a factor 2
	iBase = range((int) iBase, 0, 10000000);

	iBase /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();

	if (!kPlayer.isHuman()) {
		iBase = iBase / (GC.getHandicapInfo(GC.getGameINLINE().getHandicapType()).getAIGrowthPercent() / 100.0);	//TODO: AI needs lots of help to develop.  Likes to spam one improvement type rather than buff with complementary cottages.
	}

	return (int) iBase;
	*/
}

int CvPlot::getBonusSpawnChance(BonusTypes eBonus, PlayerTypes ePlayer) const
{
	if(!isOwned() || getBonusType() != NO_BONUS)
		return 0;

	if(getImprovementType() == NO_IMPROVEMENT || eBonus == NO_BONUS)
		return 0;

	CvImprovementInfo& kImprovement = GC.getImprovementInfo(getImprovementType());
	bool bNearSameBonus = false;
	bool bOnlySpreadNearSameBonus = false;
	int iModifier = 20;

	if(kImprovement.getPrereqTerrainSpreadResources() != NO_TERRAIN &&
		kImprovement.getPrereqTerrainSpreadResources() != getTerrainType())
		bOnlySpreadNearSameBonus = true;

	if(getWilderness() < GC.getBonusInfo(eBonus).getMinWilderness()
		|| getWilderness() > GC.getBonusInfo(eBonus).getMaxWilderness())
		return 0;

	CvPlayer& kPlayer = GET_PLAYER(ePlayer);

	TechTypes eTechReveal = (TechTypes)GC.getBonusInfo(eBonus).getTechReveal();

	if(eTechReveal != NO_TECH && (!GET_TEAM(kPlayer.getTeam()).isHasTech(eTechReveal)))
		return 0;

	int iBase = kImprovement.getImprovementBonusDiscoverRand(eBonus);

	if(iBase == 0)
		return 0;

	ProjectTypes eFertileLands = (ProjectTypes)GC.getInfoTypeForString("PROJECT_GLOBAL_FERTILE_LANDS");
	if(eFertileLands != NO_PROJECT && GET_TEAM(getTeam()).getProjectCount(eFertileLands) > 0)
	{
		if(GC.getProjectInfo(eFertileLands).getManaBonusLinked() != NO_BONUS) {
			iBase += 3 + GET_PLAYER(getOwnerINLINE()).getNumAvailableBonuses(GC.getProjectInfo(eFertileLands).getManaBonusLinked());
		}
	}

	if (isOwned())
	{
		iModifier += kPlayer.getDiscoverRandModifier();

		for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
		{
			CvPlot* pAdjacentPlot = plotDirection(getX_INLINE(), getY_INLINE(), ((DirectionTypes)iK));
			if(pAdjacentPlot != NULL) {
				if(pAdjacentPlot->getTeam() == kPlayer.getTeam()) {
					if(pAdjacentPlot->getImprovementType() != NO_IMPROVEMENT) {
						//iModifier += GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify();
						int iTempModifier = (GC.getImprovementInfo(pAdjacentPlot->getImprovementType()).getAdjacentFindResourceModify());
						if(iTempModifier!=0){
							if(iTempModifier==3){iModifier+=0;}
							else if(iTempModifier==6){iModifier+=1;}
							else if(iTempModifier==9){iModifier+=2;}
							else if(iTempModifier==10){iModifier+=1;}
							else if(iTempModifier==15){iModifier+=3;}
							else if(iTempModifier==20){iModifier+=2;}
							else if(iTempModifier==21){iModifier+=5;}
							else if(iTempModifier==30){iModifier+=3;}
						}
					}
					if(pAdjacentPlot->getBonusType(kPlayer.getTeam()) == eBonus) {
						bNearSameBonus = true;
					}
				}
			}
		}
	}

	if(bOnlySpreadNearSameBonus && !bNearSameBonus)
		return 0;

	if(GC.getBUILDINGCLASS_MERCHANT_DISTRICT() != NO_BUILDINGCLASS)
	{
		iModifier *= 100 + 20 * kPlayer.getBuildingClassCount((BuildingClassTypes)GC.getBUILDINGCLASS_MERCHANT_DISTRICT());
		iModifier /= 100;
	}

	if(bNearSameBonus)
		iBase += iModifier;


	iBase = range(iBase, 0, 100);

	iBase *= 100;
	iBase /= GC.getGameSpeedInfo(GC.getGameINLINE().getGameSpeedType()).getResearchPercent();

	return iBase;
}


bool CvPlot::isNearUF(int iRange) const
{
	for(int iX = -iRange; iX <= iRange; ++iX) {
		for(int iY = -iRange; iY <= iRange; ++iY) {
			CvPlot* pLoopPlot = plotXY(getX_INLINE(), getY_INLINE(), 
										iX, iY);
			if(pLoopPlot != NULL) {
				if(pLoopPlot->getImprovementType() != NO_IMPROVEMENT 
					&& GC.getImprovementInfo(pLoopPlot->getImprovementType()).isUnique()) {
					return true;
				}
			}
		}
	}

	return false;
}

bool CvPlot::isDefensiveBonuses(const CvUnit* pUnit, int iModifier) const
{
	if(pUnit == NULL)
		return false;

	if(iModifier < 0)
		return true;

	if (pUnit->getUnitCombatType() == NO_UNITCOMBAT)
		return false;

	if(pUnit->noDefensiveBonus())
		return false;

	if(isCity()) {
		return GC.getUnitCombatInfo(pUnit->getUnitCombatType()).isCityDefensiveBonuses();
	} else {
		return GC.getUnitCombatInfo(pUnit->getUnitCombatType()).isDefensiveBonuses();
	}
}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/

/* Fourth Yield (Asaf) - Start */
struct SomeStruct1
{
	char data[208];
};

struct SomeStruct2
{
	char data[28];
};

struct Matrix3x3
{
	float m[3][3];
};

struct ActualYieldInfo
{
	int m_iType;
	int m_iAmount;
};

struct CvMySymbol
{
	UINT_PTR m_pVTBL;
	int m_data1[2];
	SomeStruct1* m_pSomeStruct1;
	int m_data2[20];
	SomeStruct2* m_pSomeStruct2;
	int m_data3[9];
	Matrix3x3 m_arrYieldMatrices[3];
	int m_data4[5];
	ActualYieldInfo* m_arrYieldsStart;
	ActualYieldInfo* m_arrYieldsEnd;
	ActualYieldInfo* m_arrYieldsEndAgain;
};

static const float s_fSymbolYieldTextureHeight = 1.0f/64.0f;
static const float s_fTopEmptyYield = 1 - s_fSymbolYieldTextureHeight;

void CvPlot::updateSymbolsForRange(int iYieldStart, int iYieldEnd, int* arrYieldAmounts)
{
	int maxYieldStack = GC.getDefineINT("MAX_YIELD_STACK");
	int iYieldType;

	int nFirstSymbol = getNumSymbols();

	int iMaxYield = 0;
	for (iYieldType = iYieldStart; iYieldType < iYieldEnd; iYieldType++)
	{
		if(arrYieldAmounts[iYieldType] > iMaxYield)
		{
			iMaxYield = arrYieldAmounts[iYieldType];
		}
	}

	if (iMaxYield == 0)
		// No symbols to add for this
		return;

	int layers = iMaxYield / maxYieldStack + 1;

	CvSymbol *pSymbol= NULL;
	for(int i=0;i<layers;i++)
	{
		pSymbol = addSymbol();
		for (iYieldType = iYieldStart; iYieldType < iYieldEnd; iYieldType++)
		{
			int iYield = arrYieldAmounts[iYieldType] - (maxYieldStack * i);
			LIMIT_RANGE(0,iYield, maxYieldStack);
			if(arrYieldAmounts[iYieldType])
			{
				gDLL->getSymbolIFace()->setTypeYield(pSymbol,iYieldType,iYield);
			}
		}
	}

	CvMySymbol* pMySymbol = NULL;

	for(int i=nFirstSymbol; i<getNumSymbols(); i++)
	{
		SymbolTypes eSymbol  = (SymbolTypes)0;
		pSymbol = getSymbol(i);
		int iLocalIndex = i - nFirstSymbol;
		gDLL->getSymbolIFace()->init(pSymbol, gDLL->getSymbolIFace()->getID(pSymbol), iLocalIndex, eSymbol, this);

		if (iLocalIndex > 0)
		{
			// Fix empty padding
			int iOverrideIndex = 0;
			for (iYieldType = iYieldStart; iYieldType < iYieldEnd; ++iYieldType)
			{
				if (arrYieldAmounts[iYieldType])
				{
					if (arrYieldAmounts[iYieldType] - (maxYieldStack * iLocalIndex) <= 0)
					{
						pMySymbol = (CvMySymbol*)pSymbol;
						if (iOverrideIndex < 3)
							pMySymbol->m_arrYieldMatrices[iOverrideIndex].m[2][1] = s_fTopEmptyYield;
					}
					++iOverrideIndex;
				}
			}
		}
	}
}
/* Fourth Yield (Asaf) - End */

bool CvPlot::shouldHaveGraphics(void) const
{
//	return GC.IsGraphicsInitialized() && isInViewport(); // && isRevealed(GC.getGame().getActiveTeam(), false);
	return GC.IsGraphicsInitialized(); // && isRevealed(GC.getGame().getActiveTeam(), false);
}

void CvPlot::setShouldHaveFullGraphics(bool bShouldHaveFullGraphics)
{
	if(bShouldHaveFullGraphics != m_bGraphicsPageIndex) {
		m_bGraphicsPageIndex = bShouldHaveFullGraphics;

		setLayoutDirty(true);
		if ( getPlotCity() != NULL )
		{
			getPlotCity()->setLayoutDirty(true);
		}

		if ( bShouldHaveFullGraphics  )
		{
			//gDLL->getEngineIFace()->RebuildPlot(getViewportX(), getViewportY(),false,true);
			updateFeatureSymbol();
			updateRiverSymbol();
			updateRouteSymbol();
		}
		else
		{
			destroyGraphics();
		}

		updateCenterUnit();
	}
}

bool CvPlot::shouldHaveFullGraphics(void) const
{
	return m_bGraphicsPageIndex	&& shouldHaveGraphics();
}

void CvPlot::destroyGraphics()
{
	gDLL->getFeatureIFace()->destroy(m_pFeatureSymbol);

	if(m_pPlotBuilder)
	{
		gDLL->getPlotBuilderIFace()->destroy(m_pPlotBuilder);
		m_pPlotBuilder = NULL;
	}
	gDLL->getRouteIFace()->destroy(m_pRouteSymbol);
	gDLL->getRiverIFace()->destroy(m_pRiverSymbol);
	gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbol);
	gDLL->getFlagEntityIFace()->destroy(m_pFlagSymbolOffset);
	m_pCenterUnit = NULL;
	m_pFeatureSymbol = NULL;
	m_pRouteSymbol = NULL;
	m_pRiverSymbol = NULL;
	m_pFlagSymbol = NULL;
	m_pFlagSymbolOffset = NULL;
	m_bPlotLayoutDirty = false;
	m_bLayoutStateWorked = false;
	m_bFlagDirty = false;
	deleteAllSymbols();
} 
/**
int CvPlot::getViewportX() const
{
	CvViewport&	pCurrentViewPort = GC.getCurrentViewport();

	return pCurrentViewPort.getViewportXFromMapX(m_iX);
}


int CvPlot::getViewportY() const
{
	CvViewport&	pCurrentViewPort = GC.getCurrentViewport();

	return pCurrentViewPort.getViewportYFromMapY(m_iY);
}

bool CvPlot::isInViewport(int comfortBorderSize) const
{
	CvViewport&	pCurrentViewPort = GC.getCurrentViewport();

	return pCurrentViewPort.isInViewport(m_iX, m_iY, comfortBorderSize);
}
**/
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
