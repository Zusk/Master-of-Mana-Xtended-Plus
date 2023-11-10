//
//	FILE:	 CvMap.cpp
//	AUTHOR:  Soren Johnson
//	PURPOSE: Game map class
//-----------------------------------------------------------------------------
//	Copyright (c) 2004 Firaxis Games, Inc. All rights reserved.
//-----------------------------------------------------------------------------
//


#include "CvGameCoreDLL.h"
#include "CvMap.h"
#include "CvCity.h"
#include "CvPlotGroup.h"
#include "CvGlobals.h"
#include "CvGameAI.h"
#include "CvPlayerAI.h"
#include "CvRandom.h"
#include "CvGameCoreUtils.h"
#include "CvFractal.h"
#include "CvPlot.h"
#include "CvGameCoreUtils.h"
#include "CvMap.h"
#include "CvMapGenerator.h"
#include "FAStarNode.h"
#include "CvInitCore.h"
#include "CvInfos.h"
#include "FProfiler.h"
#include "CyArgsList.h"

#include "CvDLLEngineIFaceBase.h"
#include "CvDLLIniParserIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvDLLFAStarIFaceBase.h"
#include "CvDLLPythonIFaceBase.h"

// Public Functions...

CvMap::CvMap()
{
	CvMapInitData defaultMapData;

	m_paiNumBonus = NULL;
	m_paiNumBonusOnLand = NULL;

	m_pMapPlots = NULL;

	reset(&defaultMapData);
}


CvMap::~CvMap()
{
	uninit();
}

// FUNCTION: init()
// Initializes the map.
// Parameters:
//	pInitInfo					- Optional init structure (used for WB load)
// Returns:
//	nothing.
void CvMap::init(CvMapInitData* pInitInfo/*=NULL*/)
{
	int iX, iY;

	PROFILE("CvMap::init");
	gDLL->logMemState( CvString::format("CvMap::init begin - world size=%s, climate=%s, sealevel=%s, num custom options=%6",
		GC.getWorldInfo(GC.getInitCore().getWorldSize()).getDescription(),
		GC.getClimateInfo(GC.getInitCore().getClimate()).getDescription(),
		GC.getSeaLevelInfo(GC.getInitCore().getSeaLevel()).getDescription(),
		GC.getInitCore().getNumCustomMapOptions()).c_str() );

	gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "beforeInit");

	//--------------------------------
	// Init saved data
	reset(pInitInfo);

	//--------------------------------
	// Init containers
	m_areas.init();
	m_dungeons.init();

	//--------------------------------
	// Init non-saved data
	setup();

	//--------------------------------
	// Init other game data
	gDLL->logMemState("CvMap before init plots");
	m_pMapPlots = new CvPlot[numPlotsINLINE()];
	for (iX = 0; iX < getGridWidthINLINE(); iX++)
	{
		gDLL->callUpdater();
		for (iY = 0; iY < getGridHeightINLINE(); iY++)
		{
			plotSorenINLINE(iX, iY)->init(iX, iY);
		}
	}
	//calculatePlanes();
	calculateAreas();
	gDLL->logMemState("CvMap after init plots");

	//Pathfinding
	//calculateRegions();
}


void CvMap::uninit()
{
	SAFE_DELETE_ARRAY(m_paiNumBonus);
	SAFE_DELETE_ARRAY(m_paiNumBonusOnLand);

	SAFE_DELETE_ARRAY(m_pMapPlots);

	m_areas.uninit();
	m_dungeons.uninit();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	for(int iI = 0; iI < (int)m_viewports.size(); iI++)
	{
		delete m_viewports[iI];
	}

	m_viewports.clear();
	m_iCurrentViewportIndex = -1;
**/
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
}

// FUNCTION: reset()
// Initializes data members that are serialized.
void CvMap::reset(CvMapInitData* pInitInfo)
{
	int iI;

	//--------------------------------
	// Uninit class
	uninit();

	//
	// set grid size
	// initially set in terrain cell units
	//
	m_iGridWidth = (GC.getInitCore().getWorldSize() != NO_WORLDSIZE) ? GC.getWorldInfo(GC.getInitCore().getWorldSize()).getGridWidth (): 0;	//todotw:tcells wide
	m_iGridHeight = (GC.getInitCore().getWorldSize() != NO_WORLDSIZE) ? GC.getWorldInfo(GC.getInitCore().getWorldSize()).getGridHeight (): 0;
	m_iNumPlanes = GC.getNumPlaneInfos();

	m_bPlanes = true;
	if(!pInitInfo)
	{
		long bPlanes = true;
		bool okX = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "isPlaneMapscript", NULL, &bPlanes);
		bool bOverride = !gDLL->getPythonIFace()->pythonUsingDefaultImpl();
		if(bOverride)
			m_bPlanes = bPlanes;
	}
	m_bPortals = true;
	if(!pInitInfo)
	{
		long bPortals = true;
		bool okX = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "isPortalMapscript", NULL, &bPortals);
		bool bOverride = !gDLL->getPythonIFace()->pythonUsingDefaultImpl();
		if(bOverride)
			m_bPortals = bPortals;
	}

	if(GC.getDefineINT("DISABLE_MULTIPLE_PLANES") == 1)
	{
		m_iNumPlanes = 1;
	}
	// allow grid size override
	if (pInitInfo)
	{
		m_iGridWidth	= pInitInfo->m_iGridW;
		m_iGridHeight	= pInitInfo->m_iGridH;
	}
	else
	{
		/** Disable override for planes -- Sephi
		enabled again for erebuscontinents. need to modfiy other mapscripts though
		**/
		// check map script for grid size override
		if (GC.getInitCore().getWorldSize() != NO_WORLDSIZE)
		{
			std::vector<int> out;
			CyArgsList argsList;
			argsList.add(GC.getInitCore().getWorldSize());
			bool ok = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "getGridSize", argsList.makeFunctionArgs(), &out);

			if (ok && !gDLL->getPythonIFace()->pythonUsingDefaultImpl() && out.size() == 2)
			{
				m_iGridWidth = out[0];
				m_iGridHeight = out[1];
				FAssertMsg(m_iGridWidth > 0 && m_iGridHeight > 0, "the width and height returned by python getGridSize() must be positive");
			}
		}

		// convert to plot dimensions
		if (GC.getNumLandscapeInfos() > 0)
		{
			m_iGridWidth *= GC.getLandscapeInfo(GC.getActiveLandscapeID()).getPlotsPerCellX();
			m_iGridHeight *= GC.getLandscapeInfo(GC.getActiveLandscapeID()).getPlotsPerCellY();
		}


	}

	m_iLandPlots = 0;
	m_iOwnedPlots = 0;

	if (pInitInfo)
	{
		m_iTopLatitude = pInitInfo->m_iTopLatitude;
		m_iBottomLatitude = pInitInfo->m_iBottomLatitude;
	}
	else
	{
		// Check map script for latitude override (map script beats ini file)

		long resultTop = -1, resultBottom = -1;
		bool okX = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "getTopLatitude", NULL, &resultTop);
		bool overrideX = !gDLL->getPythonIFace()->pythonUsingDefaultImpl();
		bool okY = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "getBottomLatitude", NULL, &resultBottom);
		bool overrideY = !gDLL->getPythonIFace()->pythonUsingDefaultImpl();

		if (okX && okY && overrideX && overrideY && resultTop != -1 && resultBottom != -1)
		{
			m_iTopLatitude = resultTop;
			m_iBottomLatitude = resultBottom;
		}
	}

	m_iTopLatitude = std::min(m_iTopLatitude, 90);
	m_iTopLatitude = std::max(m_iTopLatitude, -90);
	m_iBottomLatitude = std::min(m_iBottomLatitude, 90);
	m_iBottomLatitude = std::max(m_iBottomLatitude, -90);

	m_iNextRiverID = 0;

	//
	// set wrapping
	//
	m_bWrapX = true;
	m_bWrapY = false;
	if (pInitInfo)
	{
		m_bWrapX = pInitInfo->m_bWrapX;
		m_bWrapY = pInitInfo->m_bWrapY;
	}
	else
	{
		// Check map script for wrap override (map script beats ini file)

		long resultX = -1, resultY = -1;
		bool okX = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "getWrapX", NULL, &resultX);
		bool overrideX = !gDLL->getPythonIFace()->pythonUsingDefaultImpl();
		bool okY = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "getWrapY", NULL, &resultY);
		bool overrideY = !gDLL->getPythonIFace()->pythonUsingDefaultImpl();

		if (okX && okY && overrideX && overrideY && resultX != -1 && resultY != -1)
		{
			m_bWrapX = (resultX != 0);
			m_bWrapY = (resultY != 0);
		}
	}

	if (GC.getNumBonusInfos())
	{
		FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated in CvMap::reset");
		FAssertMsg(m_paiNumBonus==NULL, "mem leak m_paiNumBonus");
		m_paiNumBonus = new int[GC.getNumBonusInfos()];
		FAssertMsg(m_paiNumBonusOnLand==NULL, "mem leak m_paiNumBonusOnLand");
		m_paiNumBonusOnLand = new int[GC.getNumBonusInfos()];
		for (iI = 0; iI < GC.getNumBonusInfos(); iI++)
		{
			m_paiNumBonus[iI] = 0;
			m_paiNumBonusOnLand[iI] = 0;
		}
	}

	m_areas.removeAll();
	m_dungeons.removeAll();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
	//	Create a viewport of the requisite external size without initial positioning (this can be repositioned
	//	at any time before it is graphically initialised, of after with a map switch)
	setCurrentViewport(addViewport(-1,
								   -1,
								   false));
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
}


// FUNCTION: setup()
// Initializes all data that is not serialized but needs to be initialized after loading.
void CvMap::setup()
{
	PROFILE("CvMap::setup");

	gDLL->getFAStarIFace()->Initialize(&GC.getPathFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), pathDestValid, pathHeuristic, pathCost, pathValid, pathAdd, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getInterfacePathFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), pathDestValid, pathHeuristic, pathCost, pathValid, pathAdd, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getStepFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), stepDestValid, stepHeuristic, stepCost, stepValid, stepAdd, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getRouteFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, routeValid, NULL, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getBorderFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, borderValid, NULL, NULL, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getAreaFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, areaValid, NULL, joinArea, NULL);
	gDLL->getFAStarIFace()->Initialize(&GC.getPlotGroupFinder(), getGridWidthINLINE(), getGridHeightINLINE(), isWrapXINLINE(), isWrapYINLINE(), NULL, NULL, NULL, plotGroupValid, NULL, countPlotGroup, NULL);
}


//////////////////////////////////////
// graphical only setup
//////////////////////////////////////
void CvMap::setupGraphical()
{
	if (!GC.IsGraphicsInitialized())
		return;

	if (m_pMapPlots != NULL)
	{
		int iI;
		for (iI = 0; iI < numPlotsINLINE(); iI++)
		{
			gDLL->callUpdater();	// allow windows msgs to update
			plotByIndexINLINE(iI)->setupGraphical();
		}
	}
/*************************************************************************************************/
/**	ADDON (MultiBarb)  merged Sephi  		12/23/08								Xienwolf	**/
/** Not sure if this is needed																	**/
/**				Workaround required until source of this call for Barbarians is located			**/
/*************************************************************************************************/

	for (int iPlayer = MAX_CIV_PLAYERS; iPlayer < MAX_PLAYERS; ++iPlayer)
	{
        CvUnit* pLoopUnit;
        int iLoop;
        for(pLoopUnit = GET_PLAYER((PlayerTypes)iPlayer).firstUnit(&iLoop, true); pLoopUnit != NULL; pLoopUnit = GET_PLAYER((PlayerTypes)iPlayer).nextUnit(&iLoop, true))
        {
            pLoopUnit->reloadEntity();
        }
	}

/*************************************************************************************************/
/**	MultiBarb								END													**/
/*************************************************************************************************/

}


void CvMap::erasePlots()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->erase();
	}
}


void CvMap::setRevealedPlots(TeamTypes eTeam, bool bNewValue, bool bTerrainOnly)
{
	PROFILE_FUNC();

	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->setRevealed(eTeam, bNewValue, bTerrainOnly, NO_TEAM, false);
	}

	GC.getGameINLINE().updatePlotGroups();
}


void CvMap::setAllPlotTypes(PlotTypes ePlotType)
{
	//float startTime = (float) timeGetTime();

	for(int i=0;i<numPlotsINLINE();i++)
	{
		plotByIndexINLINE(i)->setPlotType(ePlotType, false, false);
	}

	recalculateAreas();

	//rebuild landscape
	gDLL->getEngineIFace()->RebuildAllPlots();

	//mark minimap as dirty
	gDLL->getEngineIFace()->SetDirty(MinimapTexture_DIRTY_BIT, true);
	gDLL->getEngineIFace()->SetDirty(GlobeTexture_DIRTY_BIT, true);

	//float endTime = (float) timeGetTime();
	//OutputDebugString(CvString::format("[Jason] setAllPlotTypes: %f\n", endTime - startTime).c_str());
}


// XXX generalize these funcs? (macro?)
void CvMap::doTurn()
{
/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Start("Map::doTurn");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/

	PROFILE("CvMap::doTurn()")

	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->doTurn();
	}

	UpdateLandMarks();

/*************************************************************************************************/
/**	TIME MEASURE																	Sephi		**/
/*************************************************************************************************/
	GC.getTimeMeasure().Stop("Map::doTurn");
/*************************************************************************************************/
/**	TIME MEASURE							END													**/
/*************************************************************************************************/
}


void CvMap::updateFlagSymbols()
{
	PROFILE_FUNC();

	CvPlot* pLoopPlot;
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);

		if (pLoopPlot->isFlagDirty())
		{
			pLoopPlot->updateFlagSymbol();
			pLoopPlot->setFlagDirty(false);
		}
	}
}


void CvMap::updateFog()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateFog();
	}
}


void CvMap::updateVisibility()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateVisibility();
	}
}


void CvMap::updateSymbolVisibility()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateSymbolVisibility();
	}
}


void CvMap::updateSymbols()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateSymbols();
	}
}


void CvMap::updateMinimapColor()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateMinimapColor();
	}
}


void CvMap::updateSight(bool bIncrement)
{
	for (int iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateSight(bIncrement, false);
	}

	GC.getGameINLINE().updatePlotGroups();
}


void CvMap::updateIrrigated()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateIrrigated();
	}
}


void CvMap::updateCenterUnit()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateCenterUnit();
	}
}


void CvMap::updateWorkingCity()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateWorkingCity();
	}
}


void CvMap::updateMinOriginalStartDist(CvArea* pArea)
{
	PROFILE_FUNC();

	CvPlot* pStartingPlot;
	CvPlot* pLoopPlot;
	int iDist;
	int iI, iJ;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);

		if (pLoopPlot->area() == pArea)
		{
			pLoopPlot->setMinOriginalStartDist(-1);
		}
	}

	for (iI = 0; iI < MAX_CIV_PLAYERS; iI++)
	{
		pStartingPlot = GET_PLAYER((PlayerTypes)iI).getStartingPlot();

		if (pStartingPlot != NULL)
		{
			if (pStartingPlot->area() == pArea)
			{
				for (iJ = 0; iJ < numPlotsINLINE(); iJ++)
				{
					pLoopPlot = plotByIndexINLINE(iJ);

					if (pLoopPlot->area() == pArea)
					{

						//iDist = GC.getMapINLINE().calculatePathDistance(pStartingPlot, pLoopPlot);
						iDist = stepDistance(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());

						if (iDist != -1)
						{
						    //int iCrowDistance = plotDistance(pStartingPlot->getX_INLINE(), pStartingPlot->getY_INLINE(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE());
						    //iDist = std::min(iDist,  iCrowDistance * 2);
							if ((pLoopPlot->getMinOriginalStartDist() == -1) || (iDist < pLoopPlot->getMinOriginalStartDist()))
							{
								pLoopPlot->setMinOriginalStartDist(iDist);
							}
						}
					}
				}
			}
		}
	}
}


void CvMap::updateYield()
{
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->updateYield();
	}
}


void CvMap::verifyUnitValidPlot()
{
	for (int iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->verifyUnitValidPlot();
	}
}


void CvMap::combinePlotGroups(PlayerTypes ePlayer, CvPlotGroup* pPlotGroup1, CvPlotGroup* pPlotGroup2)
{
	CLLNode<XYCoords>* pPlotNode;
	CvPlotGroup* pNewPlotGroup;
	CvPlotGroup* pOldPlotGroup;
	CvPlot* pPlot;

	FAssertMsg(pPlotGroup1 != NULL, "pPlotGroup is not assigned to a valid value");
	FAssertMsg(pPlotGroup2 != NULL, "pPlotGroup is not assigned to a valid value");

	if (pPlotGroup1 == pPlotGroup2)
	{
		return;
	}

	if (pPlotGroup1->getLengthPlots() > pPlotGroup2->getLengthPlots())
	{
		pNewPlotGroup = pPlotGroup1;
		pOldPlotGroup = pPlotGroup2;
	}
	else
	{
		pNewPlotGroup = pPlotGroup2;
		pOldPlotGroup = pPlotGroup1;
	}

	pPlotNode = pOldPlotGroup->headPlotsNode();
	while (pPlotNode != NULL)
	{
		pPlot = plotSorenINLINE(pPlotNode->m_data.iX, pPlotNode->m_data.iY);
		pNewPlotGroup->addPlot(pPlot);
		pPlotNode = pOldPlotGroup->deletePlotsNode(pPlotNode);
	}
}

CvPlot* CvMap::syncRandPlot(int iFlags, int iArea, int iMinUnitDistance, int iTimeout)
{
	CvPlot* pPlot;
	CvPlot* pTestPlot;
	CvPlot* pLoopPlot;
	bool bValid;
	int iCount;
	int iDX, iDY;

	pPlot = NULL;

	iCount = 0;

	while (iCount < iTimeout)
	{
		pTestPlot = plotSorenINLINE(GC.getGameINLINE().getSorenRandNum(getGridWidthINLINE(), "Rand Plot Width"), GC.getGameINLINE().getSorenRandNum(getGridHeightINLINE(), "Rand Plot Height"));

		FAssertMsg(pTestPlot != NULL, "TestPlot is not assigned a valid value");

		if ((iArea == -1) || (pTestPlot->getArea() == iArea))
		{
			bValid = true;

			if (bValid)
			{
				if (iMinUnitDistance != -1)
				{
					for (iDX = -(iMinUnitDistance); iDX <= iMinUnitDistance; iDX++)
					{
						for (iDY = -(iMinUnitDistance); iDY <= iMinUnitDistance; iDY++)
						{
							pLoopPlot	= plotXY(pTestPlot->getX_INLINE(), pTestPlot->getY_INLINE(), iDX, iDY);

							if (pLoopPlot != NULL)
							{
								if (pLoopPlot->isUnit())
								{
									bValid = false;
								}
							}
						}
					}
				}
			}
			if (bValid)
			{
				if (iFlags & RANDPLOT_LAND)
				{
					if (pTestPlot->isWater())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_UNOWNED)
				{
					if (pTestPlot->isOwned())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_ADJACENT_UNOWNED)
				{
					if (pTestPlot->isAdjacentOwned())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_ADJACENT_LAND)
				{
					if (!(pTestPlot->isAdjacentToLand()))
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_PASSIBLE)
				{
					if (pTestPlot->isImpassable())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_NOT_VISIBLE_TO_CIV)
				{
					if (pTestPlot->isVisibleToCivTeam())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				if (iFlags & RANDPLOT_NOT_CITY)
				{
					if (pTestPlot->isCity())
					{
						bValid = false;
					}
				}
			}

			if (bValid)
			{
				pPlot = pTestPlot;
				break;
			}
		}

		iCount++;
	}

	return pPlot;
}


CvCity* CvMap::findCity(int iX, int iY, PlayerTypes eOwner, TeamTypes eTeam, bool bSameArea, bool bCoastalOnly, TeamTypes eTeamAtWarWith, DirectionTypes eDirection, CvCity* pSkipCity)
{
	PROFILE_FUNC();

	CvCity* pLoopCity;
	CvCity* pBestCity;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	// XXX look for barbarian cities???

	iBestValue = MAX_INT;
	pBestCity = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((eOwner == NO_PLAYER) || (iI == eOwner))
			{
				if ((eTeam == NO_TEAM) || (GET_PLAYER((PlayerTypes)iI).getTeam() == eTeam))
				{
					for (pLoopCity = GET_PLAYER((PlayerTypes)iI).firstCity(&iLoop); pLoopCity != NULL; pLoopCity = GET_PLAYER((PlayerTypes)iI).nextCity(&iLoop))
					{
						if (!bSameArea || (pLoopCity->area() == plotINLINE(iX, iY)->area()) || (bCoastalOnly && (pLoopCity->waterArea() == plotINLINE(iX, iY)->area())))
						{
							if (!bCoastalOnly || pLoopCity->isCoastal(GC.getMIN_WATER_SIZE_FOR_OCEAN()))
							{
								if ((eTeamAtWarWith == NO_TEAM) || atWar(GET_PLAYER((PlayerTypes)iI).getTeam(), eTeamAtWarWith))
								{
									if ((eDirection == NO_DIRECTION) || (estimateDirection(dxWrap(pLoopCity->getX_INLINE() - iX), dyWrap(pLoopCity->getY_INLINE() - iY)) == eDirection))
									{
										if ((pSkipCity == NULL) || (pLoopCity != pSkipCity))
										{
											iValue = plotDistance(iX, iY, pLoopCity->getX_INLINE(), pLoopCity->getY_INLINE());

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
				}
			}
		}
	}

	return pBestCity;
}


CvSelectionGroup* CvMap::findSelectionGroup(int iX, int iY, PlayerTypes eOwner, bool bReadyToSelect, bool bWorkers)
{
	CvSelectionGroup* pLoopSelectionGroup;
	CvSelectionGroup* pBestSelectionGroup;
	int iValue;
	int iBestValue;
	int iLoop;
	int iI;

	// XXX look for barbarian cities???

	iBestValue = MAX_INT;
	pBestSelectionGroup = NULL;

	for (iI = 0; iI < MAX_PLAYERS; iI++)
	{
		if (GET_PLAYER((PlayerTypes)iI).isAlive())
		{
			if ((eOwner == NO_PLAYER) || (iI == eOwner))
			{
				for(pLoopSelectionGroup = GET_PLAYER((PlayerTypes)iI).firstSelectionGroup(&iLoop); pLoopSelectionGroup != NULL; pLoopSelectionGroup = GET_PLAYER((PlayerTypes)iI).nextSelectionGroup(&iLoop))
				{
					if (!bReadyToSelect || pLoopSelectionGroup->readyToSelect())
					{
						if (!bWorkers || pLoopSelectionGroup->hasWorker())
						{
							iValue = plotDistance(iX, iY, pLoopSelectionGroup->getX(), pLoopSelectionGroup->getY());

							if (iValue < iBestValue)
							{
								iBestValue = iValue;
								pBestSelectionGroup = pLoopSelectionGroup;
							}
						}
					}
				}
			}
		}
	}

	return pBestSelectionGroup;
}


CvArea* CvMap::findBiggestArea(bool bWater)
{
	CvArea* pLoopArea;
	CvArea* pBestArea;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestArea = NULL;

	for(pLoopArea = firstArea(&iLoop); pLoopArea != NULL; pLoopArea = nextArea(&iLoop))
	{
		if (pLoopArea->isWater() == bWater)
		{
			iValue = pLoopArea->getNumTiles();

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				pBestArea = pLoopArea;
			}
		}
	}

	return pBestArea;
}


int CvMap::getMapFractalFlags()
{
	int wrapX = 0;
	if (isWrapXINLINE())
	{
		wrapX = (int)CvFractal::FRAC_WRAP_X;
	}

	int wrapY = 0;
	if (isWrapYINLINE())
	{
		wrapY = (int)CvFractal::FRAC_WRAP_Y;
	}

	return (wrapX | wrapY);
}


//"Check plots for wetlands or seaWater.  Returns true if found"
bool CvMap::findWater(CvPlot* pPlot, int iRange, bool bFreshWater)
{
	PROFILE("CvMap::findWater()");

	CvPlot* pLoopPlot;
	int iDX, iDY;

	for (iDX = -(iRange); iDX <= iRange; iDX++)
	{
		for (iDY = -(iRange); iDY <= iRange; iDY++)
		{
			pLoopPlot	= plotXY(pPlot->getX_INLINE(), pPlot->getY_INLINE(), iDX, iDY);

			if (pLoopPlot != NULL)
			{
				if (bFreshWater)
				{
					if (pLoopPlot->isFreshWater())
					{
						return true;
					}
				}
				else
				{
					if (pLoopPlot->isWater())
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}


bool CvMap::isPlot(int iX, int iY) const
{
	return isPlotINLINE(iX, iY);
}


int CvMap::numPlots() const
{
	return numPlotsINLINE();
}


int CvMap::plotNum(int iX, int iY) const
{
	return plotNumINLINE(iX, iY);
}


int CvMap::plotX(int iIndex) const
{
	return (iIndex % getGridWidthINLINE());
}


int CvMap::plotY(int iIndex) const
{
	return (iIndex / getGridWidthINLINE());
}


int CvMap::pointXToPlotX(float fX)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return (int)(((fX + (fWidth/2.0f)) / fWidth) * getGridWidthINLINE());
}


float CvMap::plotXToPointX(int iX)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return ((iX * fWidth) / ((float)getGridWidthINLINE())) - (fWidth / 2.0f) + (GC.getPLOT_SIZE() / 2.0f);
}


int CvMap::pointYToPlotY(float fY)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return (int)(((fY + (fHeight/2.0f)) / fHeight) * getGridHeightINLINE());
}


float CvMap::plotYToPointY(int iY)
{
	float fWidth, fHeight;
	gDLL->getEngineIFace()->GetLandscapeGameDimensions(fWidth, fHeight);
	return ((iY * fHeight) / ((float)getGridHeightINLINE())) - (fHeight / 2.0f) + (GC.getPLOT_SIZE() / 2.0f);
}


float CvMap::getWidthCoords()
{
	return (GC.getPLOT_SIZE() * ((float)getGridWidthINLINE()));
}


float CvMap::getHeightCoords()
{
	return (GC.getPLOT_SIZE() * ((float)getGridHeightINLINE()));
}


int CvMap::maxPlotDistance()
{
	return std::max(1, plotDistance(0, 0, ((isWrapXINLINE()) ? (getGridWidthINLINE() / 2) : (getGridWidthINLINE() - 1)), ((isWrapYINLINE()) ? (getGridHeightINLINE() / 2) : (getGridHeightINLINE() - 1))));
}


int CvMap::maxStepDistance()
{
	return std::max(1, stepDistance(0, 0, ((isWrapXINLINE()) ? (getGridWidthINLINE() / 2) : (getGridWidthINLINE() - 1)), ((isWrapYINLINE()) ? (getGridHeightINLINE() / 2) : (getGridHeightINLINE() - 1))));
}


int CvMap::getGridWidth() const
{
	return getGridWidthINLINE();
}


int CvMap::getGridHeight() const
{
	return getGridHeightINLINE();
}


int CvMap::getLandPlots()
{
	return m_iLandPlots;
}


void CvMap::changeLandPlots(int iChange)
{
	m_iLandPlots = (m_iLandPlots + iChange);
	FAssert(getLandPlots() >= 0);
}

int CvMap::getNumPlanes()
{
	return m_iNumPlanes;
}

int CvMap::getOwnedPlots()
{
	return m_iOwnedPlots;
}


void CvMap::changeOwnedPlots(int iChange)
{
	m_iOwnedPlots = (m_iOwnedPlots + iChange);
	FAssert(getOwnedPlots() >= 0);
}


int CvMap::getTopLatitude()
{
	return m_iTopLatitude;
}


int CvMap::getBottomLatitude()
{
	return m_iBottomLatitude;
}


int CvMap::getNextRiverID()
{
	return m_iNextRiverID;
}


void CvMap::incrementNextRiverID()
{
	m_iNextRiverID++;
}


bool CvMap::isWrapX()
{
	return isWrapXINLINE();
}


bool CvMap::isWrapY()
{
	return isWrapYINLINE();
}

bool CvMap::isWrap()
{
	return isWrapINLINE();
}

WorldSizeTypes CvMap::getWorldSize()
{
	return GC.getInitCore().getWorldSize();
}


ClimateTypes CvMap::getClimate()
{
	return GC.getInitCore().getClimate();
}


SeaLevelTypes CvMap::getSeaLevel()
{
	return GC.getInitCore().getSeaLevel();
}



int CvMap::getNumCustomMapOptions()
{
	return GC.getInitCore().getNumCustomMapOptions();
}


CustomMapOptionTypes CvMap::getCustomMapOption(int iOption)
{
	return GC.getInitCore().getCustomMapOption(iOption);
}


int CvMap::getNumBonuses(BonusTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiNumBonus[eIndex];
}


void CvMap::changeNumBonuses(BonusTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiNumBonus[eIndex] = (m_paiNumBonus[eIndex] + iChange);
	FAssert(getNumBonuses(eIndex) >= 0);
}


int CvMap::getNumBonusesOnLand(BonusTypes eIndex)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	return m_paiNumBonusOnLand[eIndex];
}


void CvMap::changeNumBonusesOnLand(BonusTypes eIndex, int iChange)
{
	FAssertMsg(eIndex >= 0, "eIndex is expected to be non-negative (invalid Index)");
	FAssertMsg(eIndex < GC.getNumBonusInfos(), "eIndex is expected to be within maximum bounds (invalid Index)");
	m_paiNumBonusOnLand[eIndex] = (m_paiNumBonusOnLand[eIndex] + iChange);
	FAssert(getNumBonusesOnLand(eIndex) >= 0);
}


CvPlot* CvMap::plotByIndex(int iIndex) const
{
	return plotByIndexINLINE(iIndex);
}


CvPlot* CvMap::plot(int iX, int iY) const
{
	return plotINLINE(iX, iY);
}


CvPlot* CvMap::pointToPlot(float fX, float fY)
{
	return plotINLINE(pointXToPlotX(fX), pointYToPlotY(fY));
}


int CvMap::getIndexAfterLastArea()
{
	return m_areas.getIndexAfterLast();
}


int CvMap::getNumAreas()
{
	return m_areas.getCount();
}


int CvMap::getNumLandAreas()
{
	CvArea* pLoopArea;
	int iNumLandAreas;
	int iLoop;

	iNumLandAreas = 0;

	for(pLoopArea = GC.getMap().firstArea(&iLoop); pLoopArea != NULL; pLoopArea = GC.getMap().nextArea(&iLoop))
	{
		if (!(pLoopArea->isWater()))
		{
			iNumLandAreas++;
		}
	}

	return iNumLandAreas;
}


CvArea* CvMap::getArea(int iID)
{
	return m_areas.getAt(iID);
}


CvArea* CvMap::addArea()
{
	return m_areas.add();
}


void CvMap::deleteArea(int iID)
{
	m_areas.removeAt(iID);
}


CvArea* CvMap::firstArea(int *pIterIdx, bool bRev)
{
	return !bRev ? m_areas.beginIter(pIterIdx) : m_areas.endIter(pIterIdx);
}


CvArea* CvMap::nextArea(int *pIterIdx, bool bRev)
{
	return !bRev ? m_areas.nextIter(pIterIdx) : m_areas.prevIter(pIterIdx);
}


void CvMap::recalculateAreas()
{
	PROFILE("CvMap::recalculateAreas");

	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		plotByIndexINLINE(iI)->setArea(FFreeList::INVALID_INDEX);
	}

	m_areas.removeAll();

	calculateAreas();
}


void CvMap::resetPathDistance()
{
	gDLL->getFAStarIFace()->ForceReset(&GC.getStepFinder());
}


int CvMap::calculatePathDistance(CvPlot *pSource, CvPlot *pDest)
{
	FAStarNode* pNode;

	if (pSource == NULL || pDest == NULL)
	{
		return -1;
	}

	if (gDLL->getFAStarIFace()->GeneratePath(&GC.getStepFinder(), pSource->getX_INLINE(), pSource->getY_INLINE(), pDest->getX_INLINE(), pDest->getY_INLINE(), false, 0, true))
	{
		pNode = gDLL->getFAStarIFace()->GetLastNode(&GC.getStepFinder());

		if (pNode != NULL)
		{
			return pNode->m_iData1;
		}
	}

	return -1; // no passable path exists
}

//
// read object from a stream
// used during load
//
void CvMap::read(FDataStreamBase* pStream)
{
	CvMapInitData defaultMapData;

	// Init data before load
	reset(&defaultMapData);

	uint uiFlag=0;
	pStream->Read(&uiFlag);	// flags for expansion

	pStream->Read(&m_iGridWidth);
	pStream->Read(&m_iGridHeight);
	pStream->Read(&m_iLandPlots);
	pStream->Read(&m_iNumPlanes);
	pStream->Read(&m_iOwnedPlots);
	pStream->Read(&m_iTopLatitude);
	pStream->Read(&m_iBottomLatitude);
	pStream->Read(&m_iNextRiverID);

	pStream->Read(&m_bWrapX);
	pStream->Read(&m_bWrapY);
	pStream->Read(&m_bPlanes);
	pStream->Read(&m_bPortals);
	

	FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated");
	pStream->Read(GC.getNumBonusInfos(), m_paiNumBonus);
	pStream->Read(GC.getNumBonusInfos(), m_paiNumBonusOnLand);

	if (numPlotsINLINE() > 0)
	{
		m_pMapPlots = new CvPlot[numPlotsINLINE()];
		int iI;
		for (iI = 0; iI < numPlotsINLINE(); iI++)
		{
			m_pMapPlots[iI].read(pStream);
		}
	}

	// call the read of the free list CvArea class allocations
	ReadStreamableFFreeListTrashArray(m_areas, pStream);
	ReadStreamableFFreeListTrashArray(m_dungeons, pStream);
	setup();

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
//	getCurrentViewport()->resizeForMap();
/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
}

// save object to a stream
// used during save
//
void CvMap::write(FDataStreamBase* pStream)
{
	uint uiFlag=0;
	pStream->Write(uiFlag);		// flag for expansion

	pStream->Write(m_iGridWidth);
	pStream->Write(m_iGridHeight);
	pStream->Write(m_iLandPlots);
	pStream->Write(m_iNumPlanes);
	pStream->Write(m_iOwnedPlots);
	pStream->Write(m_iTopLatitude);
	pStream->Write(m_iBottomLatitude);
	pStream->Write(m_iNextRiverID);

	pStream->Write(m_bWrapX);
	pStream->Write(m_bWrapY);
	pStream->Write(m_bPlanes);
	pStream->Write(m_bPortals);

	FAssertMsg((0 < GC.getNumBonusInfos()), "GC.getNumBonusInfos() is not greater than zero but an array is being allocated");
	pStream->Write(GC.getNumBonusInfos(), m_paiNumBonus);
	pStream->Write(GC.getNumBonusInfos(), m_paiNumBonusOnLand);

	int iI;
	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		m_pMapPlots[iI].write(pStream);
	}



	// call the read of the free list CvArea class allocations
	WriteStreamableFFreeListTrashArray(m_areas, pStream);

	WriteStreamableFFreeListTrashArray(m_dungeons, pStream);
}


//
// used for loading WB maps
//
void CvMap::rebuild(int iGridW, int iGridH, int iTopLatitude, int iBottomLatitude, bool bWrapX, bool bWrapY, WorldSizeTypes eWorldSize, ClimateTypes eClimate, SeaLevelTypes eSeaLevel, int iNumCustomMapOptions, CustomMapOptionTypes * aeCustomMapOptions)
{
	CvMapInitData initData(iGridW, iGridH, iTopLatitude, iBottomLatitude, bWrapX, bWrapY);

	// Set init core data
	GC.getInitCore().setWorldSize(eWorldSize);
	GC.getInitCore().setClimate(eClimate);
	GC.getInitCore().setSeaLevel(eSeaLevel);
	GC.getInitCore().setCustomMapOptions(iNumCustomMapOptions, aeCustomMapOptions);

	// Init map
	init(&initData);
}


//////////////////////////////////////////////////////////////////////////
// Protected Functions...
//////////////////////////////////////////////////////////////////////////

void CvMap::calculateAreas()
{
	PROFILE("CvMap::calculateAreas");
	CvPlot* pLoopPlot;
	CvArea* pArea;
	int iArea;
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);
		gDLL->callUpdater();
		FAssertMsg(pLoopPlot != NULL, "LoopPlot is not assigned a valid value");

		if (pLoopPlot->getArea() == FFreeList::INVALID_INDEX)
		{
			pArea = addArea();
			pArea->init(pArea->getID(), pLoopPlot->isWater());

			iArea = pArea->getID();

			pLoopPlot->setArea(iArea);

			gDLL->getFAStarIFace()->GeneratePath(&GC.getAreaFinder(), pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), -1, -1, pLoopPlot->isWater(), iArea);
		}
	}
}

// Private Functions...

bool CvMap::isPlanes() { return m_bPlanes; }
bool CvMap::isPortals() { return m_bPortals; }

void CvMap::calculateWilderness()
{
	CvPlot* pLoopPlot;
	int iValue=MAX_INT;
	int iI;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);
		pLoopPlot->setWilderness(-1);

		for(int iJ = 0; iJ < GC.getMAX_CIV_PLAYERS(); iJ++)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iJ);
			if(kPlayer.isEverAlive() && kPlayer.getStartingPlot() == pLoopPlot)
			{
				pLoopPlot->setWilderness(0);
			}
		}
	}

	for(int iPass = 1; iPass < 31; iPass++)
	{
		for(int iJ = 0;iJ < GC.getMAX_CIV_PLAYERS(); iJ++)
		{
			CvPlayer& kPlayer = GET_PLAYER((PlayerTypes)iJ);
			if(kPlayer.isEverAlive())
			{
				CvPlot* pStartPlot=kPlayer.getStartingPlot();
				//found a Starting Plot
				for (iI = 0; iI < numPlotsINLINE(); iI++)
				{
					pLoopPlot = plotByIndexINLINE(iI);

					//need a new value?
					if(pLoopPlot->getWilderness()==-1 && stepDistance(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),pStartPlot->getX_INLINE(),pStartPlot->getY_INLINE())==iPass)
					{		
/**
deactivated random increase  
						int iValue=0;
						int iCount=0;
						for (int iK = 0; iK < NUM_DIRECTION_TYPES; iK++)
						{
							CvPlot* pAdjacentPlot = plotDirection(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), (DirectionTypes) iK);
							if(pAdjacentPlot!=NULL && pAdjacentPlot->getWilderness()!=-1)
							{
								if(stepDistance(pAdjacentPlot->getX_INLINE(),pAdjacentPlot->getY_INLINE(),pStartPlot->getX_INLINE(),pStartPlot->getY_INLINE()==iPass-1))
								{
									iValue+=pAdjacentPlot->getWilderness();
									iCount++;
								}
							}
						}
					
						if(iCount>0)
						{
							int iNewValue=iValue/iCount;
							if(iPass>7)
							{
								iNewValue+=GC.getGameINLINE().getMapRandNum(5, "Wilderness");
							}
							else if(iPass%3!=0)
							{							
								iNewValue++;
							}
							iNewValue=std::max(1,iNewValue);
							pLoopPlot->setWilderness(iNewValue);
						}
**/
						pLoopPlot->setWilderness((3 * stepDistance(pLoopPlot->getX_INLINE(), pLoopPlot->getY_INLINE(), pStartPlot->getX_INLINE(), pStartPlot->getY_INLINE())) / 2);
					}
				}
			}
		}
	}

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);
		if(pLoopPlot->getWilderness()==-1)
			pLoopPlot->setWilderness(MAX_INT);
	}

}

void CvMap::calculatePlanes_OnlyOne()
{
	CvPlot* pLoopPlot;

	for (int iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndex(iI);
		pLoopPlot->setPlane((PlaneTypes)0);
	}
}

void CvMap::calculatePlanes_Continents()
{
	int eNextPlane = 0;
	for(int i = 0; i < numPlots(); ++i) {
		CvPlot* pLoopOrigin = plotByIndex(i);
		if(!pLoopOrigin->isWater()) {
			if(pLoopOrigin->getPlane() == -1) {
				int iLoopArea = pLoopOrigin->getArea();
				for(int j = 0; j < numPlots(); ++j) {
					CvPlot* pLoopPlot = plotByIndex(j);
					if(pLoopPlot->getPlane() == -1 && (!pLoopPlot->isWater())
						&& pLoopPlot->getArea() == iLoopArea) {
							pLoopPlot->setPlane((PlaneTypes)eNextPlane);
					}
				}
				++eNextPlane;
				if(eNextPlane >= GC.getNumPlaneInfos())
					eNextPlane = 0;
			}
		}
	}

	for(int i = 0; i < numPlots(); ++i) {
		CvPlot* pLoopOrigin = plotByIndex(i);
		if(pLoopOrigin->isWater() && pLoopOrigin->getTerrainType() == GC.getInfoTypeForString("TERRAIN_OCEAN")) {
			bool bAdjacentOnlyWater = true;

			for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++) {
				CvPlot* pAdjacentPlot = plotDirection(pLoopOrigin->getX_INLINE(), pLoopOrigin->getY_INLINE(), (DirectionTypes) iI);
				if (pAdjacentPlot != NULL) {
					if(pAdjacentPlot->getPlane() != NO_PLANE) {
						if(!pAdjacentPlot->isWater() || pAdjacentPlot->getTerrainType() != GC.getInfoTypeForString("TERRAIN_OCEAN") ) {
							bAdjacentOnlyWater = false;
						}
					}
				}
			}

			if(bAdjacentOnlyWater) {
				pLoopOrigin->setNonPlanar(true);
			}
		}
		if(pLoopOrigin->isWater()) {
			bool bAdjacentHell = false;

			for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++) {
				CvPlot* pAdjacentPlot = plotDirection(pLoopOrigin->getX_INLINE(), pLoopOrigin->getY_INLINE(), (DirectionTypes) iI);
				if (pAdjacentPlot != NULL) {
					if(pAdjacentPlot->getPlane() != NO_PLANE &&
						GC.getPlaneInfo(pAdjacentPlot->getPlane()).isReplaceWaterWithPeaks()) {
						bAdjacentHell = true;
					}
				}
			}

			if(bAdjacentHell) {
				pLoopOrigin->setPlotType(PLOT_PEAK, false, true);
				TerrainTypes eHellTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_FIELDS_OF_PERDITION");
				if(eHellTerrain != NO_TERRAIN)
					pLoopOrigin->setTerrainType(eHellTerrain);
				pLoopOrigin->setNonPlanar(true);
			}
		}
		if(pLoopOrigin->isWater()) {
			bool bAdjacentPlane = false;

			for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++) {
				CvPlot* pAdjacentPlot = plotDirection(pLoopOrigin->getX_INLINE(), pLoopOrigin->getY_INLINE(), (DirectionTypes) iI);
				if (pAdjacentPlot != NULL) {
					if(pAdjacentPlot->getPlane() != NO_PLANE) {
						bAdjacentPlane = true;
					}
				}
			}

			if(!bAdjacentPlane) {
				pLoopOrigin->setNonPlanar(true);
			}
		}
	}
}

void CvMap::calculatePlanes_Default()
{
	int iI, iOffset, iBorderDistance;
	CvPlot* pLoopPlot;

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);

		iOffset = 0;
		iBorderDistance = (pLoopPlot->getX_INLINE() * getNumPlanes()) % getGridWidth();
		if(iBorderDistance < 15) {
			if(GC.getGameINLINE().getSorenRandNum(iBorderDistance / 3 + 1, "calculate Planes") == 0) {
				iOffset = 1;
			}
		}
		pLoopPlot->setPlane((PlaneTypes)(pLoopPlot->getX_INLINE()*getNumPlanes()/getGridWidth() - iOffset));
	}

	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);
		if(pLoopPlot->getPlane() == -1) {
			pLoopPlot->setPlane((PlaneTypes)(getNumPlanes() - 1));
		}
	}
}

void CvMap::calculatePlanes_Areas()
{
	int iI, iOffset, iBorderDistance;
	int iPlanes;
	int* iNumEachPlane = new int[GC.getNumPlaneInfos()];
	CvPlot* pLoopPlot;

	for(iI = 0; iI < GC.getNumPlaneInfos(); ++iI) {
		iNumEachPlane[iI] = 0;
	}

	for(iPlanes = 0; iPlanes < GC.getNumPlaneInfos() * 2; ++iPlanes) {
		std::vector<int> possiblePlanes;
		int iNextPlane = NO_PLANE;
		for(iI = 0; iI < GC.getNumPlaneInfos(); ++iI) {
			if(iNumEachPlane[iI] < 2) {
				possiblePlanes.push_back(iI);
			}
		}

		iNextPlane = possiblePlanes[GC.getGameINLINE().getSorenRandNum(possiblePlanes.size(), "pick plane")];

		if(iNextPlane != NO_PLANE) {
			++iNumEachPlane[iNextPlane];
			for (iI = 0; iI < numPlotsINLINE(); iI++)
			{
				pLoopPlot = plotByIndexINLINE(iI);
				if((pLoopPlot->getX_INLINE() < ((1 + iPlanes / 2) * getGridWidth()) / getNumPlanes())
					&& (pLoopPlot->getX_INLINE() >= ((iPlanes / 2) * getGridWidth()) / getNumPlanes()))
				{
					if((pLoopPlot->getY_INLINE() < getGridHeight() / 2 && iPlanes % 2 == 0) 
						|| (pLoopPlot->getY_INLINE() >= getGridHeight() / 2 && iPlanes % 2 == 1)) 
					{ 

						iOffset = 0;
						iBorderDistance = (pLoopPlot->getX_INLINE() * getNumPlanes()) % getGridWidth();
						if(iBorderDistance < 15) 
						{
							if(GC.getGameINLINE().getSorenRandNum(iBorderDistance / 3 + 1, "calculate Planes") == 0) {
								iOffset = 1;
							}
						}
						pLoopPlot->setPlane((PlaneTypes)(iNextPlane - iOffset));
					}
				}
			}
		}
	}

	//should be adjusted to create Plane of nearby plots
	for (iI = 0; iI < numPlotsINLINE(); iI++)
	{
		pLoopPlot = plotByIndexINLINE(iI);
		if(pLoopPlot->getPlane() == -1) {
			pLoopPlot->setPlane((PlaneTypes)(getNumPlanes() - 1));
		}
	}
}

enum TerrainRequirements
{
	NONE = -1,
	TERRAIN_GRASS = 0,
	TERRAIN_GRASS_FERTILE = 1,
	TERRAIN_PLAINS = 2,
	TERRAIN_DESERT = 3,
	TERRAIN_MARSH = 4,
	TERRAIN_SWAMP = 5,
	TERRAIN_NEVER_NEVER = 6,
	TERRAIN_TUNDRA = 7,
	TERRAIN_SNOW = 8,
	TERRAIN_BURNING_SANDS = 9,
	TERRAIN_BROKEN_LANDS = 10,
	TERRAIN_FIELDS_OF_PERDITION = 11,
	TERRAIN_SHALLOWS = 12,
	TERRAIN_WASTELAND = 13,
	TERRAIN_MUSHGROUND = 14,
	TERRAIN_DARK_MUSHGROUND = 15,
	TERRAIN_COAST = 16,
	TERRAIN_OCEAN = 17,
	TERRAIN_PEAK = 18,
	TERRAIN_HILL = 19,
	TERRAIN_OCEAN_DEEP = 20,
	USE_PLANE = 21,
};


void ClimateGroup::adjustRimPlots()
{

	std::set<CvPlot*>::const_iterator it;
	for (it = plots.begin(); it != plots.end(); it++)
	{

		CvPlot* startPlot = *it;

		if (startPlot->isWater())
			continue;

		if (startPlot->isPeak())
			continue;

		int iSearchRange = 1;
		int iDX, iDY;

		// Check neighbouring plots to see what terrain they have

		std::map<TerrainTypes, int> adjecentTypes;
		bool adjecentToOtherArea = false;
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				CvPlot* pLoopPlot = plotXY(startPlot->getX_INLINE(), startPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot == NULL)
					continue;

				const bool is_in = plots.find(pLoopPlot) != plots.end();
				if (is_in) // part of the group
					continue;

				if (pLoopPlot->isWater())
					continue;

				if (startPlot->getArea() != pLoopPlot->getArea())
					adjecentToOtherArea = true;

				TerrainTypes terrain = pLoopPlot->getTerrainType();

				adjecentTypes[terrain]++;
			}
		}

		std::map<TerrainTypes, int>::iterator it;
		TerrainTypes mostCommon = NO_TERRAIN;
		int mostCommonCount = 0;
		for (it = adjecentTypes.begin(); it != adjecentTypes.end(); it++)
		{
			if (it->second > mostCommonCount)
			{
				mostCommon = it->first;
				mostCommonCount = it->second;
			}
		}

		if (mostCommon == NO_TERRAIN)
			continue;

		TerrainTypes softener = getTerrainInBetween(startPlot->getTerrainType(), mostCommon);
		if (softener != startPlot->getTerrainType())
		{
			if (softener != NO_TERRAIN)
				startPlot->setTerrainType(softener, true, true);

			// We might need to change plot type afterwards...
			if (softener == (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PEAK"))
				startPlot->setPlotType(PLOT_PEAK);
			else if (softener == (TerrainTypes)GC.getInfoTypeForString("TERRAIN_HILL"))
				startPlot->setPlotType(PLOT_HILLS);
			else if (softener == (TerrainTypes)GC.getInfoTypeForString("TERRAIN_OCEAN"))
				startPlot->setPlotType(PLOT_OCEAN);
			else if (softener == (TerrainTypes)GC.getInfoTypeForString("TERRAIN_COAST"))
				startPlot->setPlotType(PLOT_OCEAN);
			else if (!adjecentToOtherArea)
			{
				// Give a random amount of mountains between the areas
				if (GC.getGameINLINE().getSorenRandNum(100, "wanna mountain?") < 30)
				{
					startPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_PEAK"), true, true);
					startPlot->setPlotType(PLOT_PEAK);
				}
			}
		}
	}
}


std::vector<ClimateGroup> ClimateGroup::getNeighbouringClimateGroups(std::vector<ClimateGroup> groups)
{
	std::vector<ClimateGroup> result;
	std::vector<int> resultInt;
	std::set<CvPlot*>::const_iterator it;
	for (it = plots.begin(); it != plots.end(); it++)
	{

		CvPlot* startPlot = *it;

		if (startPlot->isWater())
			continue;

		if (startPlot->isPeak())
			continue;

		int iSearchRange = 1;
		int iDX, iDY;

		bool adjecentToOtherArea = false;
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				CvPlot* pLoopPlot = plotXY(startPlot->getX_INLINE(), startPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot == NULL)
					continue;

				const bool is_in = plots.find(pLoopPlot) != plots.end();
				if (is_in) // part of the group
					continue;

				if (pLoopPlot->isWater())
					continue;

				if (startPlot->getArea() != pLoopPlot->getArea())
					adjecentToOtherArea = true;

				for (size_t i = 0; i < groups.size(); i++)
				{
					if (std::find(resultInt.begin(), resultInt.end(), (int)i) != resultInt.end())
						continue;

					if (groups[i].plots.size() == plots.size() && groups[i].terrain == terrain)
						continue;

					if (groups[i].plots.find(pLoopPlot) != groups[i].plots.end())
					{
						result.push_back(groups[i]);
						resultInt.push_back(i);
					}
				}
			}
		}
	}
	return result;
}

TerrainTypes ClimateGroup::getTerrainInBetween(TerrainTypes first, TerrainTypes second)
{
	if (first == second)
		return first; // No change

	if (first == GC.getInfoTypeForString("TERRAIN_DESERT"))
	{
		if (second == GC.getInfoTypeForString("TERRAIN_SNOW"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PEAK");

		return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");

	}

	if (first == GC.getInfoTypeForString("TERRAIN_SNOW"))
	{
		if (second == GC.getInfoTypeForString("TERRAIN_DESERT"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PEAK");

		return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA");
	}

	if (first == GC.getInfoTypeForString("TERRAIN_MARSH"))
	{
		if (second == GC.getInfoTypeForString("TERRAIN_DESERT"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
		if (second == GC.getInfoTypeForString("TERRAIN_SNOW"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA");
		if (second == GC.getInfoTypeForString("TERRAIN_PLAINS"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	}

	if (first == GC.getInfoTypeForString("TERRAIN_SWAMP"))
	{
		if (second == GC.getInfoTypeForString("TERRAIN_DESERT"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
		if (second == GC.getInfoTypeForString("TERRAIN_SNOW"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA");
		if (second == GC.getInfoTypeForString("TERRAIN_PLAINS"))
			return (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	}

	return NO_TERRAIN;
}


std::set<CvPlot*> ClimateGroup::rimPlots() const
{
	std::set<CvPlot*> result;

	std::set<CvPlot*>::const_iterator it;
	for (it = plots.begin(); it != plots.end(); it++)
	{
		CvPlot* startPlot = *it;

		int iSearchRange = 1;
		int iDX, iDY;
		bool addedPlot = false;
		// Check neighbouring plots if they can be added
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				CvPlot* pLoopPlot = plotXY(startPlot->getX_INLINE(), startPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot == NULL)
					continue;

				const bool is_in = plots.find(pLoopPlot) != plots.end();
				if (is_in) // part of the group
					continue;

				result.insert(startPlot);
				addedPlot = true;
			}
			if (addedPlot)
				break;
		}
	}
	return result;
}

int StartRequirements::EvaluatePlot(CvPlot* plot, bool cityCenter) const
{
	int value = 100;
	CvString name = GC.getCivilizationInfo(GC.getInitCore().getCiv((PlayerTypes)playerNumber)).getType();

	if ((name.CompareNoCase("CIVILIZATION_AMURITES") == 0 || name.CompareNoCase("CIVILIZATION_MAZATL") == 0) && cityCenter && plot->isHills())
		value -= 50000;

	
	if (reqIsland)
	{
		if (cityCenter && GC.getMapINLINE().getArea(plot->getArea())->getNumTiles() < 10) // Just makeshift detection for island
		{
			value += (500 + GC.getMapINLINE().getArea(plot->getArea())->getNumTiles() * 50);
		}
		if (!cityCenter && plot->isWater())
		{
			value += GC.getMapINLINE().getArea(plot->getArea())->getNumTiles() * 1; // Favour the biggest ocean
		}
	}
	if (reqForest)
	{
		FeatureTypes eForest = (FeatureTypes)GC.getInfoTypeForString("FEATURE_FOREST");
		FeatureTypes eForestAncient = (FeatureTypes)GC.getInfoTypeForString("FEATURE_FOREST_ANCIENT");

		if (plot->getFeatureType() != eForest && plot->getFeatureType() != eForestAncient)
			value -= 25;
	}
	if (reqJungle)
	{
		FeatureTypes eJungle = (FeatureTypes)GC.getInfoTypeForString("FEATURE_JUNGLE");
		if (plot->getFeatureType() != eJungle)
			value -= 25;
	}
	if (reqHills)
	{
		if (!plot->isHills())
			value -= 25;
	}
	if (!cityCenter && reqMountains)
	{
		if (!plot->isPeak())
			value -= 10;
	}
	if (reqTerrain != NONE)
	{
		if (reqTerrain != plot->getTerrainType())
		{
			if (!reqIsland)
				value -= 200;
			else
			{
				// Since island's second best option is a coastal area....

				if (!cityCenter && plot->isWater())
					value += 25;

				if (plot->getFeatureType() == (FeatureTypes)GC.getInfoTypeForString("FEATURE_KELP"))
					value += 25;
			}
		}
	}



	if (plot->getBonusType(NO_TEAM) != NO_BONUS && 
		GET_PLAYER((PlayerTypes)playerNumber).isIgnoreFood() &&
		name.CompareNoCase("CIVILIZATION_ARISTRAKH") != 0 &&
		GC.getBonusInfo(plot->getBonusType()).getCityYieldModifierType() == YIELD_FOOD)
		value -= 100;
	else if (!cityCenter && plot->getBonusType(NO_TEAM) != NO_BONUS)
		value += 50;
	if (cityCenter && plot->getBonusType(NO_TEAM) != NO_BONUS)
		value -= 10000; // don't want cities on resources preferably

	if (plot->getFeatureType() != NO_FEATURE)
	{
		if (reqTerrain == GC.getInfoTypeForString("TERRAIN_DESERT"))
		{
			FeatureTypes eFlood = (FeatureTypes)GC.getInfoTypeForString("FEATURE_FLOOD_PLAINS");
			FeatureTypes eOasis = (FeatureTypes)GC.getInfoTypeForString("FEATURE_OASIS");
			if (!cityCenter && plot->getFeatureType() == eOasis)
				value += 75;
			if (plot->getFeatureType() == eFlood)
				value += 50;
		}
	}

	if (plot->getImprovementType() != NO_IMPROVEMENT)
	{
		CvImprovementInfo impInfo = GC.getImprovementInfo(plot->getImprovementType());
		if (!cityCenter)
		{
			if (impInfo.isUnique())
				value += 100;
			else if (impInfo.isPermanent())
				value += 25;
		}
		else // Not wanna have city centre on stuff that cannot be removed
		{
			if (impInfo.isUnique())
				value -= 500000;
			else if (impInfo.isPermanent())
				value -= 500000;
		}
	}

	if (plot->isFreshWater())
	{
		if (cityCenter)
			value += 50;
		else
			value += 20;
	}

	return value;
}



StartRequirements CvMap::getWantedStartForPlayer(int player)
{
	CvCivilizationInfo& info = GC.getCivilizationInfo(GC.getInitCore().getCiv((PlayerTypes)player));

	StartRequirements wanted;
	wanted.szName = info.getType();
	wanted.playerNumber = player;
	wanted.reqForest = false;
	wanted.reqIsland = false;
	wanted.reqJungle = false;
	wanted.reqHills = false;
	wanted.reqMountains = false;
	wanted.plane = (PlaneTypes)info.getNativePlane();

	if (wanted.szName.CompareNoCase("CIVILIZATION_BANNOR") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"); // because of favorite improvement is cottage
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_MALAKIM") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT");
		// Oasis or floodplain also?
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_ELOHIM") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
		// Close to unique feature?
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_MERCURIANS") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_LUCHUIRP") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
		wanted.reqHills = true;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_KURIOTATES") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_LJOSALFAR") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
		wanted.reqForest = true;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_KHAZAD") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
		wanted.reqHills = true;
		wanted.reqMountains = true;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_SIDAR") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_LANUN") == 0)
	{
		wanted.reqIsland = true;
		// Island start is a must I think, will code to make an island for them if there is none
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_GRIGORI") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_HIPPUS") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
		// Wanna have horses besides in the capital?
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_AMURITES") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH");
		// Shrine or tower or mana?
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_DOVIELLO") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA");
		wanted.reqForest = true; // Tundra without forest is to hard to live in basically
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_BALSERAPHS") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_CLAN_OF_EMBERS") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
		//wanted.reqJungle = true;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_SVARTALFAR") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA");
		wanted.reqForest = true;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_CALABIM") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_SHEAIM") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
		// Shrine or tower or mana?
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_INFERNAL") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_FIELDS_OF_PERDITION");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_ILLIANS") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_ARISTRAKH") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_AUSTRIN") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_DURAL") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_FAERIES") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
		wanted.reqForest = true;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_JOTNAR") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_MAZATL") == 0)
	{
		wanted.reqJungle = true;
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SWAMP");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_SCIONS") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_FROZEN") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW");
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_BARBARIAN") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_ANIMAL") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_WILDMANA") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_PIRATES") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
		wanted.reqIsland = true;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_WHITEHAND") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_DEVIL") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_MERCENARIES") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}
	else if (wanted.szName.CompareNoCase("CIVILIZATION_RANDOM") == 0)
	{
		wanted.reqTerrain = (TerrainTypes)NONE;
	}

	return  wanted;
}
 
std::vector<StartRequirements> CvMap::getPlayersAndStartsWanted()
{
	std::vector<StartRequirements> playersAndStartsWanted;

	// The amount of climate areas will be based on the players, just more to get more variation
	for (size_t iPlayer = 0; iPlayer < MAX_PLAYERS; iPlayer++)
	{
		if ((GC.getInitCore().getCiv((PlayerTypes)iPlayer) != NO_CIVILIZATION) && (GC.getInitCore().getLeader((PlayerTypes)iPlayer) != NO_LEADER))
		{
			StartRequirements wanted = getWantedStartForPlayer(iPlayer);
			playersAndStartsWanted.push_back(wanted);
		}
	}
	return playersAndStartsWanted;
}

bool CvMap::useClimateBasedStartPos()
{
	long result;
	bool calledMap = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "useClimateBasedStartPos", NULL, &result);

	if (!calledMap) // Map script did not have the function
		return false;

	return result > 0;
}

void CvMap::add_terrain_as_wanted(std::map<TerrainTypes, int>& wantedTerrains, std::vector<TerrainTypes>& randomPickList, int& wantedCount, TerrainTypes req_terrain)
{
	if (req_terrain != (TerrainTypes)NONE)
	{
		randomPickList.push_back(req_terrain);
		wantedTerrains[req_terrain]++;
		wantedCount++;
	}
}

void CvMap::changeTerrainBasedOnPlayers()
{

	GC.getMapINLINE().calculateAreas();

	std::set<CvPlot*> islandPlots;
	//	// Try to put some islands in the water

	for (size_t iPlot = 0; (int)iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		CvPlot* startPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);

		if (!startPlot->isWater())
			continue;

		int iSearchRange = 1;
		int iDX, iDY;
		bool validForIsland = true;
		// Check neighbouring plots if they can be added
		for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
		{
			for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
			{
				CvPlot* pLoopPlot = plotXY(startPlot->getX_INLINE(), startPlot->getY_INLINE(), iDX, iDY);

				if (pLoopPlot == NULL)
					continue;

				if (!pLoopPlot->isWater())
				{
					const bool is_Done = islandPlots.find(pLoopPlot) != islandPlots.end();
					if (is_Done) // an island already made is still ok to make an island next to.
						continue;
						
					validForIsland = false;
					break;
				}
			}
			if (!validForIsland)
				break;
		}
		if (!validForIsland)
			continue;

		if (GC.getGameINLINE().getSorenRandNum(100, "Bob") < 20)
		{
			int terrainType = GC.getGameINLINE().getSorenRandNum(100, "Bob");
			if (terrainType < 40)
				startPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS"));
			else if (terrainType < 80)
				startPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS"));
			else
				startPlot->setTerrainType((TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT"));

			int terrainType2 = GC.getGameINLINE().getSorenRandNum(100, "Bob");
			if (terrainType2 < 30)
				startPlot->setPlotType(PLOT_HILLS);
			else
				startPlot->setPlotType(PLOT_LAND);

			islandPlots.insert(startPlot);
		}
	}
	islandPlots.clear();

	GC.getMapINLINE().calculateAreas();

	int areas = GC.getMapINLINE().getNumAreas();
	std::vector<CvArea*> landAreas;
	std::vector<CvArea*> waterAreas;
	int totalLandPlots = 0;

	for (size_t iArea = 0; (int)iArea < areas; iArea++)
	{
		CvArea* area = GC.getMapINLINE().getArea(iArea);
		int size = area->getNumTiles();
		if (area->isWater())
		{
			waterAreas.push_back(area);
		}
		else
		{
			landAreas.push_back(area);
			totalLandPlots += size;
		}
	}

	if (landAreas.size() == 1)
	{
		if (landAreas[0]->getNumTiles() < 10)
		{
			char	*szMessage = new char[512];
			char	szMessage1[128];

			sprintf(szMessage1, "Climate areas failed because map areas was corrupt, even after a calculate areas!");

			szMessage = strcat(szMessage, szMessage1);

			gDLL->MessageBox(szMessage, "Climate mapgeneration feedback");
		}
	}

	std::vector<StartRequirements> playersAndStartsWanted = getPlayersAndStartsWanted();
	std::map<TerrainTypes, int> wantedTerrains;
	std::vector<TerrainTypes> randomPickList;
	int wantedCount = 0;
	for (size_t iStarts = 0; iStarts < playersAndStartsWanted.size(); iStarts++)
	{
		add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, playersAndStartsWanted[iStarts].reqTerrain);
	}

	//add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_BURNING_SANDS"));
	//add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_FIELDS_OF_PERDITION"));
	int areasBesidesPlayers = 0;
	switch (GC.getMapINLINE().getWorldSize())
	{
		case NO_WORLDSIZE:			areasBesidesPlayers = 0; break;
		case WORLDSIZE_DUEL:		areasBesidesPlayers = 1; break;
		case WORLDSIZE_TINY:		areasBesidesPlayers = 2; break;
		case WORLDSIZE_SMALL:		areasBesidesPlayers = 4; break;
		case WORLDSIZE_STANDARD:	areasBesidesPlayers = 8; break;
		case WORLDSIZE_LARGE:		areasBesidesPlayers = 12; break;
		case WORLDSIZE_HUGE:		areasBesidesPlayers = 16; break;
		case NUM_WORLDSIZE_TYPES:	areasBesidesPlayers = 0; break;
		default: ;
	}


	for (int i = 0; i < areasBesidesPlayers; ++i)
	{
		int iC = GC.getGameINLINE().getSorenRandNum(6, "Terrain generation");
		switch (iC)
		{
			case 0: add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_MUSHGROUND")); break;
			case 1: add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DARK_MUSHGROUND"));break;
			case 2: add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW"));break;
			case 3: add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT"));break;
			case 4: add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA"));break;
			case 5: add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH"));break;
			case 6: add_terrain_as_wanted(wantedTerrains, randomPickList, wantedCount, (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SWAMP"));break;

			default:;
		}
	}

	//enum TerrainRequirements
	//{
	//	NONE = -1,
	//	TERRAIN_GRASS = 0,
	//	TERRAIN_GRASS_FERTILE = 1,
	//	TERRAIN_PLAINS = 2,
	//	TERRAIN_DESERT = 3,
	//	TERRAIN_MARSH = 4,
	//	TERRAIN_SWAMP = 5,
	//	TERRAIN_NEVER_NEVER = 6,
	//	TERRAIN_TUNDRA = 7,
	//	TERRAIN_SNOW = 8,
	//	TERRAIN_BURNING_SANDS = 9,
	//	TERRAIN_BROKEN_LANDS = 10,
	//	TERRAIN_FIELDS_OF_PERDITION = 11,
	//	TERRAIN_SHALLOWS = 12,
	//	TERRAIN_WASTELAND = 13,
	//	TERRAIN_MUSHGROUND = 14,
	//	TERRAIN_DARK_MUSHGROUND = 15,
	//	TERRAIN_COAST = 16,
	//	TERRAIN_OCEAN = 17,
	//	TERRAIN_PEAK = 18,
	//	TERRAIN_HILL = 19,
	//	TERRAIN_OCEAN_DEEP = 20,
	//	USE_PLANE = 21,
	//};


	int sizePerStart = totalLandPlots / wantedCount;
	int climageGroupCounter = 0;
	std::set<CvPlot*> donePlots;
	std::vector<ClimateGroup> climateGroups;
	for (size_t iArea = 0; iArea < landAreas.size(); iArea++)
	{
		CvArea* a = landAreas[iArea];
		int size = a->getNumTiles();
		if (size < sizePerStart / 2)
			continue; // Too small area to be nice starting site

		for (size_t iPlot = 0; (int)iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
		{
			CvPlot* startPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);

			if (startPlot->getArea() != a->getID())
				continue;


			const bool is_Done = donePlots.find(startPlot) != donePlots.end();
			if (is_Done) // already set terrain too in another group
				continue;

			ClimateGroup climateGroup;

			climateGroup.plots.insert(startPlot);
			// gather a group of plots, in the current area that is of start size
			CvPlot* currentPlot = startPlot;
			int growTries = 0;
			bool abort = false;
			while ((int)climateGroup.plots.size() < sizePerStart && growTries < 1000) // Right now this is brute force, stupid and random just to get going basically
			{
				int iSearchRange = 1;
				int iDX, iDY;
				bool added = false;
				// Check neighbouring plots if they can be added
				for (iDX = -(iSearchRange); iDX <= iSearchRange; iDX++)
				{
					for (iDY = -(iSearchRange); iDY <= iSearchRange; iDY++)
					{
						CvPlot* pLoopPlot = plotXY(currentPlot->getX_INLINE(), currentPlot->getY_INLINE(), iDX, iDY);

						if (pLoopPlot == NULL)
							continue;

						if (pLoopPlot->getArea() != a->getID())
							continue;

						const bool is_in = climateGroup.plots.find(pLoopPlot) != climateGroup.plots.end();
						if (is_in) // already added to this climate group
							continue;

						const bool is_Done2 = donePlots.find(pLoopPlot) != donePlots.end();
						if (is_Done2) // already set terrain too in another group
							continue;

						climateGroup.plots.insert(pLoopPlot);
						added = true;
					}
				}

				if (!added && climateGroup.plots.size() == 1) // we did not find anything to grow from, move on
				{
					abort = true;
					break;
				}

				climateGroup.plots.begin();
				// Move to a random one and continue to get a random shape
				int iC = GC.getGameINLINE().getSorenRandNum((int)climateGroup.plots.size() - 1, "Terrain generation") + 1;
				std::set<CvPlot*>::iterator start = climateGroup.plots.begin();
				for (size_t i = 0; (int)i < iC; i++)
				{
					start._Inc();
				}
				growTries++;
				currentPlot = *start;

			}

			if (abort)
				continue;

			// Grab the terrain from the closest area instead of making small zones
			std::vector<ClimateGroup> neighbours = climateGroup.getNeighbouringClimateGroups(climateGroups);
			ClimateGroup biggestNeighbour = climateGroup;
			for (size_t j = 0; j < neighbours.size(); j++)
			{
				if (neighbours[j].terrain == NO_TERRAIN)
					continue;

				std::vector<TerrainTypes>::iterator pick = std::find(randomPickList.begin(), randomPickList.end(), neighbours[j].terrain);
				if (pick == randomPickList.end())
					continue;

				if (biggestNeighbour.plots.size() < neighbours[j].plots.size())
					biggestNeighbour = neighbours[j];
			}
			TerrainTypes neighbourHasThis = biggestNeighbour.terrain;
			int iT = -1;
			if (neighbourHasThis != NO_TERRAIN)
			{
				std::vector<TerrainTypes>::iterator pick = std::find(randomPickList.begin(), randomPickList.end(), neighbourHasThis);
				if (pick != randomPickList.end())
				{
					if (GC.getGameINLINE().getSorenRandNum(100, "Use neighbour terrrain type") < 100)
					{
						climateGroup.terrain = neighbourHasThis;
						iT = std::distance(randomPickList.begin(), pick);
					}
				}
			}
			
			if (iT == -1)
			{
				iT = GC.getGameINLINE().getSorenRandNum((int)randomPickList.size(), "Terrain generation");
				climateGroup.terrain = randomPickList[iT];
			}

			// loop through all in the climate group and change climate and also add them to the done collection
			std::set<CvPlot*>::iterator it;
			for (it = climateGroup.plots.begin(); it != climateGroup.plots.end(); it++)
				donePlots.insert(*it);

			climateGroups.push_back(climateGroup);
			climageGroupCounter++;
			// if this was a major area made and assigned, then remove it's pick from the random list

			if (growTries < 1000)
			{
				randomPickList.erase(randomPickList.begin() + iT);
				climateGroup.IsMajorArea = true;
			}
			else
			{
				climateGroup.IsMajorArea = false;
			}
		}
	}

	for (size_t i = 0; i < climateGroups.size(); i++)
	{
		if (climateGroups[i].plots.size() < (size_t)(sizePerStart / 4))
		{
			// Grab the terrain from the closest area instead of making small zones
			std::vector<ClimateGroup> neighbours = climateGroups[i].getNeighbouringClimateGroups(climateGroups);
			ClimateGroup biggestNeighbour = climateGroups[i];
			for (size_t j = 0; j < neighbours.size(); j++)
			{
				if (neighbours[j].terrain == NO_TERRAIN)
					continue;
				if (biggestNeighbour.plots.size() < neighbours[j].plots.size())
					biggestNeighbour = neighbours[j];
			}
			climateGroups[i].terrain = biggestNeighbour.terrain;
			continue;

		}
	}

	// loop through all in the climate group and change climate and also add them to the done collection
	for (size_t i = 0; i < climateGroups.size(); i++)
	{
		std::set<CvPlot*>::iterator it;
		for (it = climateGroups[i].plots.begin(); it != climateGroups[i].plots.end(); it++)
		{
			(*it)->setTerrainType(climateGroups[i].terrain);
			donePlots.insert(*it);
		}
	}

	// Ok, we got the climate groups and changed the terrain to match. Some groups might want softer transitions, glacier to desert might seem wierd for example without something in between.
	for (size_t i = 0; i < climateGroups.size(); i++)
	{
		climateGroups[i].adjustRimPlots();

	}

	
	addTerrainSpecials();
	addJungleAndOasis();
	addMana();

	// Reporting result code...
	
	double factor = (double)donePlots.size() / (double)totalLandPlots;
	//int groups = (int)climateGroups.size();
	int numberReachedRightSize = 0;
	for (size_t i = 0; i < climateGroups.size(); i++)
	{
		if ((int)climateGroups[i].plots.size() >= sizePerStart)
			numberReachedRightSize++;
	}

///*
	char	*szMessage = new char[512];
	char	szMessage1[128];
	char	szMessage2[128];
	char	szMessage3[128];
	//char	szMessage4[128];
	sprintf(szMessage1, "Climate generation/transformation of map is complete. \n Number of climate groups was: %d \n", climageGroupCounter);
	sprintf(szMessage2, "Coverage of groups vs total land areas was: %f \n", factor*100);
	sprintf(szMessage3, "Number of groups that reached wanted size was: %d \n", numberReachedRightSize);
	//sprintf(szMessage4, "Coverage of groups vs total land areas was: %d \n", factor);
	szMessage = strcat(szMessage1, szMessage2);
	szMessage = strcat(szMessage, szMessage3);
	
	gDLL->MessageBox(szMessage, "Climate mapgeneration feedback");
	//*/

	for (size_t i = 0; i < climateGroups.size(); i++)
	{
		climateGroups[i].plots.clear();

	}

	climateGroups.clear();
	playersAndStartsWanted.clear();
	landAreas.clear();
	waterAreas.clear();
	wantedTerrains.clear();
	randomPickList.clear();
	donePlots.clear();
}

void CvMap::addMana()
{

	BonusTypes iRawMana = (BonusTypes)GC.getInfoTypeForString("BONUS_MANA");
	TerrainTypes iDesert = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT");
	TerrainTypes iMarsh = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH");
	TerrainTypes iSwamp = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SWAMP");
	TerrainTypes iGrass = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	TerrainTypes iGrassFertile = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS_FERTILE");
	TerrainTypes iPlains = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	TerrainTypes iTundra = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA");
	TerrainTypes iSnow = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW");

	const char *manas[] = { "BONUS_MANA_ICE", "BONUS_MANA_AIR", "BONUS_MANA_CHAOS", "BONUS_MANA_DEATH", "BONUS_MANA_LIFE", "BONUS_MANA_MIND", "BONUS_MANA_NATURE", "BONUS_MANA_FIRE", "BONUS_MANA" };

	for (size_t iPlot = 0; (int)iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		TerrainTypes terrain = pPlot->getTerrainType();

		BonusTypes iBonus = pPlot->getBonusType();

		if (pPlot->isWater() || pPlot->getImprovementType() != NO_IMPROVEMENT || iBonus != iRawMana)
			continue;

		//int iManaRnd = GC.getGameINLINE().getSorenRandNum(100, "Mana Creation");
		const char* mana_str = manas[GC.getGameINLINE().getSorenRandNum(9, "Pick Mana")];
		BonusTypes iRandomMana = (BonusTypes)GC.getInfoTypeForString(mana_str);

		pPlot->setBonusType(iRandomMana);

	}
}

void CvMap::addTerrainSpecials()
{
	TerrainTypes iGrass = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS");
	TerrainTypes iGrassFertile = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_GRASS_FERTILE");
	TerrainTypes iPlains = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_PLAINS");
	TerrainTypes iTundra = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_TUNDRA");
	TerrainTypes iCoast = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_COAST");
	TerrainTypes iOcean = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_OCEAN");
	FeatureTypes iOasis = (FeatureTypes)GC.getInfoTypeForString("FEATURE_OASIS");
	TerrainTypes iMarsh = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH");
	TerrainTypes iSwamp = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SWAMP");
	TerrainTypes iDesert = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT");
	TerrainTypes iSnow = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SNOW");
	FeatureTypes iIce = (FeatureTypes)GC.getInfoTypeForString("FEATURE_ICE");

	for (size_t iPlot = 0; (int)iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		TerrainTypes terrain = pPlot->getTerrainType();

		// Ice just annoys me... kill it with fire!
		if (pPlot->isWater() && pPlot->getFeatureType() == iIce)
			pPlot->setFeatureType(NO_FEATURE);

		if (pPlot->isHills() || pPlot->isPeak() || pPlot->isWater())
			continue;

		if (terrain == iGrass && pPlot->getBonusType() != NO_BONUS && GC.getBonusInfo(pPlot->getBonusType()).getCityYieldModifierType() == YIELD_FOOD)
			pPlot->setTerrainType(iGrassFertile, true, true);

	}
}

void CvMap::addJungleAndOasis()
{
	FeatureTypes iJungle = (FeatureTypes)GC.getInfoTypeForString("FEATURE_JUNGLE");
	FeatureTypes iOasis = (FeatureTypes)GC.getInfoTypeForString("FEATURE_OASIS");
	TerrainTypes iMarsh = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_MARSH");
	TerrainTypes iSwamp = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_SWAMP");
	TerrainTypes iDesert = (TerrainTypes)GC.getInfoTypeForString("TERRAIN_DESERT");

	for (size_t iPlot = 0; (int)iPlot < GC.getMapINLINE().numPlotsINLINE(); iPlot++)
	{
		CvPlot* pPlot = GC.getMapINLINE().plotByIndexINLINE(iPlot);
		TerrainTypes terrain = pPlot->getTerrainType();
		if (pPlot->getFeatureType() != NO_FEATURE)
			continue;
		if (pPlot->getImprovementType() != NO_IMPROVEMENT)
			continue;

		if (terrain == iSwamp)
		{ 
			if (GC.getGameINLINE().getSorenRandNum(100, "Bob") < 90)
			{
				pPlot->setFeatureType(iJungle, 2);
			}
		}
		if (terrain == iMarsh)
		{
			if (GC.getGameINLINE().getSorenRandNum(100, "Bob") < 70)
			{
				pPlot->setFeatureType(iJungle, 2);
			}
		}
		if (terrain == iDesert)
		{
			if (!pPlot->isHills() && !pPlot->isPeak() && pPlot->getBonusType() == NO_BONUS && !pPlot->isFreshWater())
			{
				int iChance = 2;
				bool bValid = true;

				for (int iI = 0; iI < NUM_DIRECTION_TYPES; iI++)
				{
					CvPlot* pAdjPlot = plotDirection(pPlot->getX_INLINE(), pPlot->getY_INLINE(), ((DirectionTypes)iI));

					if (pAdjPlot == NULL)
						continue;

					if (pAdjPlot->getFeatureType() == iOasis)
					{
						bValid = false;
						break;
					}
					if (pAdjPlot->getTerrainType() == iDesert)
						iChance += 5;
				}

				if (bValid && GC.getGameINLINE().getSorenRandNum(100, "wanna have oasis and cheese?") < iChance)
				{
					pPlot->setFeatureType(iOasis, 2);
				}

			}
		}
	}
}

void CvMap::calculatePlanes()
{
	int iValue = MAX_INT;

	//Pick a Random Method to generate Planes 
	int iPick = GC.getGameINLINE().getSorenRandNum(2, "Generate Planes");

	//DEBUG
	iPick = 3;
	//DEBUG

	long result = 0;
	bool okX = gDLL->getPythonIFace()->callFunction(gDLL->getPythonIFace()->getMapScriptModule(), "getPlaneCreationMethod", NULL, &result);
	bool bOverride = !gDLL->getPythonIFace()->pythonUsingDefaultImpl();

	if(bOverride)
		iPick = result;

	switch(iPick) {
		case 3:
			calculatePlanes_OnlyOne();
			break;
		case 2:
			calculatePlanes_Continents();
			break;
		case 1:
			calculatePlanes_Areas();
			break;
		// generate n Planes next to each other that reach from top to bottom of map
		case 0:
		default:
			calculatePlanes_Default();
			break;
	}
}

void CvMap::placePortals()
{
	int iPortalsToCreate = GC.getMapINLINE().getWorldSize() * 2;
	int iPortalsCreated = 0;

	for(int i = 0; i < numPlots(); ++i) {
		CvPlot* pLoopPlot = plotByIndex(i);
		if(pLoopPlot->area()->getNumPortals() == 0 && pLoopPlot->area()->getNumTiles() > 5) {
			placePortalOnArea(pLoopPlot->getArea());
			++iPortalsCreated;
		}
	}

	int iValue;
	while(iPortalsCreated < iPortalsToCreate) {
		int iBestArea = -1;
		int iBestValue = 0;

		for(int i = 0; i < numPlots(); ++i) {
			CvPlot* pLoopPlot = plotByIndex(i);
			iValue = pLoopPlot->area()->getNumTiles() / (pLoopPlot->area()->getNumPortals() + 1);
			if(iValue > iBestValue) {
				iBestValue = iValue;
				iBestArea = pLoopPlot->getArea();
			}
		}

		if(iBestArea != -1) {
			placePortalOnArea(iBestArea);
			++iPortalsCreated;
		}
		else {
			break;
		}
	}

	int iBigArea = findLargestArea();
	int iNextBigArea = findNextLargestArea(iBigArea);
	//Is it Possible to access every Area?
	int iHack = 0;
	while(true) {
		++iHack;
		if(iHack > 100)
			break;

		if(iBigArea != -1 && iNextBigArea != -1) {
			if(!getArea(iBigArea)->canAreaBeReachedThroughPortal(iNextBigArea)) {
				placePortalOnArea(iBigArea, iNextBigArea); 
			}
			iBigArea = iNextBigArea;
			iNextBigArea = findNextLargestArea(iBigArea);
		}

	}
}

int CvMap::findLargestArea()
{
	CvArea* pLoopArea;
	CvArea* pBestArea;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestArea = NULL;

	for(pLoopArea = firstArea(&iLoop); pLoopArea != NULL; pLoopArea = nextArea(&iLoop))
	{
		if (!pLoopArea->isWater())
		{
			iValue = pLoopArea->getNumTiles();

			if (iValue > iBestValue)
			{
				iBestValue = iValue;
				pBestArea = pLoopArea;
			}
		}
	}

	if(pBestArea == NULL)
		return -1;

	return pBestArea->getID();
}

//find the largest Area that is still smaller than iArea
int CvMap::findNextLargestArea(int iArea)
{
	CvArea* pLoopArea;
	CvArea* pBestArea;
	int iValue;
	int iBestValue;
	int iLoop;

	iBestValue = 0;
	pBestArea = NULL;

	if(iArea == -1) {
		return -1;
	}
	int iMax = getArea(iArea)->getNumTiles();

	for(pLoopArea = firstArea(&iLoop); pLoopArea != NULL; pLoopArea = nextArea(&iLoop))
	{
		if (!pLoopArea->isWater())
		{
			iValue = pLoopArea->getNumTiles();

			if (iValue > iBestValue && iValue <= iMax && 
				(iValue < iMax || pLoopArea->getID() > iArea))
			{
				iBestValue = iValue;
				pBestArea = pLoopArea;
			}
		}
	}

	if(pBestArea == NULL)
		return -1;

	return pBestArea->getID();
}

void CvMap::placePortalOnArea(int iArea, int iTargetArea)
{
	int iBestValue = 0;
	int iValue;
	CvPlot* pBestTarget = NULL;

	for(int i = 0; i < numPlots(); ++i) {
		CvPlot* pLoopPlot = plotByIndex(i);
		if(pLoopPlot->getArea() == iArea) {
			if(pLoopPlot->canCreatePortal()) {
				iValue = GC.getGame().getMapRandNum(10000, "Place Portal");
				if(iValue > iBestValue) {
					iBestValue = iValue;
					pBestTarget = pLoopPlot;
				}
			}
		}
	}

	if(pBestTarget != NULL) {
		pBestTarget->createPortal(iTargetArea);
	}
}

void CvMap::placeEpicLairs()
{
	CvPlot* pLoopPlot;
	int iI;
	//Wildmana Guardians
	BonusTypes eMana = (BonusTypes)GC.getInfoTypeForString("BONUS_MANA");
	for(int iJ = 0; iJ<GC.getNumUnitInfos(); iJ++)
	{
		if(GC.getUnitInfo((UnitTypes)iJ).isWildmanaGuardian())
		{
			UnitTypes eGuardian = (UnitTypes)iJ;
			CvUnitInfo& kUnit = GC.getUnitInfo(eGuardian);
			BonusTypes eManaConvert = (BonusTypes)GC.getUnitInfo((UnitTypes)iJ).getPrereqAndBonus();
			CvPlot* pBestPlot = NULL;
			int iValue, iBestValue = 0;
			ImprovementTypes eImprovement = NO_IMPROVEMENT;

			if(eGuardian == NO_UNIT || eManaConvert == NO_BONUS)
				continue;

			bool bPlaceOnMana = true;
			for(iI = 0; iI < GC.getNumImprovementInfos(); iI++)
			{
				if(GC.getImprovementInfo((ImprovementTypes)iI).getSpawnUnitType() == iJ)
				{
					bPlaceOnMana = false;
					eImprovement = (ImprovementTypes)iI;
				}
			}

			if(bPlaceOnMana && eMana == NO_BONUS)
				continue;

			for (iI = 0; iI < numPlotsINLINE(); iI++)
			{
				pLoopPlot = plotByIndexINLINE(iI);
				if(!pLoopPlot->isWater() && !pLoopPlot->isPeak())
				{
					if(!bPlaceOnMana || 
						(pLoopPlot->getBonusType()==eMana && pLoopPlot->getWilderness()>=kUnit.getMinWilderness() && pLoopPlot->getWilderness()<=kUnit.getMinWilderness()))
					{
						if(bPlaceOnMana || pLoopPlot->getImprovementType()==eImprovement)
						{
							iValue=pLoopPlot->getWilderness();
							if(pLoopPlot->area()->getNumTiles()>5)
								iValue+=1000;
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
				CvUnit* pNewUnit = GET_PLAYER((PlayerTypes)GC.getWILDMANA_PLAYER()).initUnit(eGuardian,pBestPlot->getX_INLINE(),pBestPlot->getY_INLINE(),UNITAI_MANADEFENDER);
				pBestPlot->setBonusType(eManaConvert);

				//Immunity to Wildmana
				for(int iDifX = -5; iDifX <= 5; iDifX++)
				{
					for(int iDifY = -5; iDifY <= 5; iDifY++)
					{
						pLoopPlot = plotXY(pBestPlot->getX_INLINE(),pBestPlot->getY_INLINE(),iDifX,iDifY);
						if(pLoopPlot!=NULL)
						{
							if(GC.getGameINLINE().getMapRandNum(40, "Plot Immunity to Wildmana")<iDifX*iDifX+iDifY*iDifY+10)
							{
								pLoopPlot->setWildmanaImmune(true);
							}
						}
					}
				}
			}
		}
	}
}

void CvMap::placeCityStates()
{
	for(int iI=0;iI<GC.getNumCityStateInfos();iI++)
	{
		//Does City State already exist? (regenerate Map)
		bool bValid=true;
		for(int iJ=0;iJ<GC.getMAX_CIV_PLAYERS();iJ++)
		{
			if(GET_PLAYER((PlayerTypes)iJ).getCityStateType()==iI)
			{
				bValid=false;
				break;
			}
		}

		if(!bValid)
			continue;

		placeCityState((CityStateTypes)iI);

	}

    CyArgsList argsList;
    long lResult=-1;
	argsList.add(0);
    gDLL->getPythonIFace()->callFunction(PYGameModule, "CreateCityStates", argsList.makeFunctionArgs(), &lResult);
}

//return the ID of the CityState created
int CvMap::placeCityState(CityStateTypes eState)
{
	CvPlot* pLoopPlot;
	CvPlot* pBestPlot=NULL;
	int iValue, iBestValue;
	int iSearchRange=2;
	int iBestLeader=NO_LEADER;

	CvCityStateInfo &kState=GC.getCityStateInfo(eState);
	iBestValue=0;
	pBestPlot=NULL;
	iBestLeader=NO_LEADER;

	for (int iJ = 0; iJ < numPlotsINLINE(); iJ++)
	{
		pLoopPlot = plotByIndexINLINE(iJ);
		if(pLoopPlot->isValidCityStateLocation())
		{
			iValue=100;
			if(pLoopPlot->isCoastalLand() && kState.isCoastal())
			{
				iValue+=1000;
			}

			for (int iX=-iSearchRange;iX<=iSearchRange;iX++)
			{
				for (int iY=-iSearchRange;iY<=iSearchRange;iY++)
				{
					CvPlot* pAdjacentPlot=plotXY(pLoopPlot->getX_INLINE(),pLoopPlot->getY_INLINE(),iX,iY);
					if(pAdjacentPlot!=NULL)
					{
						if(kState.getNativeFeature()!=NO_FEATURE)
						{
							if(pAdjacentPlot->getFeatureType()==kState.getNativeFeature())
							{
								iValue+=100;
							}
						}

						if(kState.getNativeImprovement()!=NO_IMPROVEMENT)
						{
							if(pAdjacentPlot->getImprovementType()==kState.getNativeImprovement())
							{
								iValue+=1000;
							}
						}

						if(kState.getNativeTerrain()!=NO_TERRAIN)
						{
							if(pAdjacentPlot->getTerrainType()==kState.getNativeTerrain())
							{
								iValue+=100;
							}
						}

					}
					else
					{
						iValue-=1000;
					}
				}
			}

			if(iValue>iBestValue)
			{
				iBestValue=iValue;
				pBestPlot=pLoopPlot;
			}

		}
	}

	iBestValue=0;
	for(int iJ=0;iJ<GC.getNumLeaderHeadInfos();iJ++)
	{
		if(GC.getCivilizationInfo((CivilizationTypes)kState.getCivilizationType()).isLeaders(iJ))
		{
			iValue=10000;
			for(int iK=0;iK<GC.getMAX_CIV_PLAYERS();iK++)
			{
				if(GET_PLAYER((PlayerTypes)iK).isEverAlive())
				{
					if(GET_PLAYER((PlayerTypes)iK).getLeaderType()==iJ)
					{
						iValue-=9000;
						break;
					}
				}
			}
		}
		if(iValue>iBestValue)
		{
			iBestValue=iValue;
			iBestLeader=iJ;
		}
	}

	if(kState.getLeaderType()!=NO_LEADER)
	{
		iBestLeader=kState.getLeaderType();
	}

	PlayerTypes iPlayer=(PlayerTypes)GC.getGameINLINE().getOpenPlayer();

	if(pBestPlot!=NULL && iBestLeader!=NO_LEADER && iPlayer!=NO_PLAYER)
	{
		GC.getGame().addPlayer(iPlayer,(LeaderHeadTypes)iBestLeader,(CivilizationTypes)kState.getCivilizationType());
		CvPlayer& kPlayer=GET_PLAYER(iPlayer);
		kPlayer.setCityStateType(eState);
		CvCity* pCapital=kPlayer.initCity(pBestPlot->getX_INLINE(),pBestPlot->getY_INLINE(),true,true);
		kPlayer.setCapitalCity(pCapital);
		if(kState.getReligionType()!=NO_RELIGION)
		{
			pCapital->setHasReligion((ReligionTypes)kState.getReligionType(),true,false,false);
		}
		for(int iJ=GC.getMAX_CIV_TEAMS();iJ<GC.getMAX_TEAMS();iJ++)
		{
			if(GET_TEAM((TeamTypes)iJ).isBarbarian())
			{
				GET_TEAM((TeamTypes)iJ).declareWar(kPlayer.getTeam(),false,NO_WARPLAN);
			}
		}
	}

	return (int)iPlayer;
}

CvDungeon* CvMap::getDungeon(int iID)
{
	return m_dungeons.getAt(iID);
}

CvDungeon* CvMap::addDungeon()
{
	return m_dungeons.add();
}
void CvMap::deleteDungeon(int iID)
{
	m_dungeons.removeAt(iID);
}

CvDungeon* CvMap::firstDungeon(int *pIterIdx, bool bRev)
{
	return !bRev ? m_dungeons.beginIter(pIterIdx) : m_dungeons.endIter(pIterIdx);
}

CvDungeon* CvMap::nextDungeon(int *pIterIdx, bool bRev)
{
	return !bRev ? m_dungeons.nextIter(pIterIdx) : m_dungeons.prevIter(pIterIdx);
}

void CvMap::UpdateLandMarks()
{
//	gDLL->getEngineIFace()->clearSigns();

	for(int i = 0; i < numPlots(); ++i) {
		CvPlot* pLoopPlot = plotByIndex(i);

		if(pLoopPlot->isPortal()) {
			CvWString szText;
			szText.append(L"Portal");
			gDLL->getEngineIFace()->addLandmark(pLoopPlot, szText);
		}
		else if(pLoopPlot->getDungeon() != NULL) {
			pLoopPlot->getDungeon()->updateLandmark();
		}
	}
}

/*************************************************************************************************/
/**	Viewport C2C				                                   								**/
/*************************************************************************************************/
/**
int	CvMap::getNumViewports() const
{
	return m_viewports.size();
}

CvViewport* CvMap::getViewport(int iIndex) const
{
	FAssert(iIndex >= 0 && iIndex < (int)m_viewports.size());

	return m_viewports[iIndex];
}

int CvMap::addViewport(int iXOffset, int iYOffset, bool bIsFullMapContext)	//	Returns new viewport index
{
	CvViewport*	viewport = new CvViewport(this, bIsFullMapContext);

	if ( iXOffset >= 0 && iYOffset >= 0 )
	{
		viewport->setMapOffset(iXOffset, iYOffset);
	}
	m_viewports.push_back(viewport);

	return (int)m_viewports.size()-1;
}

void CvMap::deleteViewport(int iIndex)
{
	FAssert(iIndex >= 0 && iIndex < (int)m_viewports.size());

	if (m_iCurrentViewportIndex == iIndex)
	{
		m_iCurrentViewportIndex = -1;
	}

	delete m_viewports[iIndex];

	for(unsigned int iI = iIndex; iI < m_viewports.size()-1; iI++)
	{
		m_viewports[iI] = m_viewports[iI+1];
	}

	m_viewports.pop_back();
}

void CvMap::setCurrentViewport(int iIndex)
{
	FAssert(iIndex >= 0 && iIndex < (int)m_viewports.size());

	m_iCurrentViewportIndex = iIndex;
}

CvViewport* CvMap::getCurrentViewport() const
{
	FAssert( m_iCurrentViewportIndex == -1 || m_iCurrentViewportIndex < (int)m_viewports.size() );

	return (m_iCurrentViewportIndex == -1 ? NULL : m_viewports[m_iCurrentViewportIndex]);
}

/*************************************************************************************************/
/**	END Viewport C2C			                                        						**/
/*************************************************************************************************/
