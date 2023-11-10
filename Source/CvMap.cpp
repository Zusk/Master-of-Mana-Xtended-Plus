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
					if(!pAdjacentPlot->isNonPlanar()) {
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
					&& (pLoopPlot->getX_INLINE() >= ((iPlanes / 2) * getGridWidth()) / getNumPlanes())){
					if((pLoopPlot->getY_INLINE() < getGridHeight() / 2 && iPlanes % 2 == 0) 
						|| (pLoopPlot->getY_INLINE() >= getGridHeight() / 2 && iPlanes % 2 == 1)) { 

						iOffset = 0;
						iBorderDistance = (pLoopPlot->getX_INLINE() * getNumPlanes()) % getGridWidth();
						if(iBorderDistance < 15) {
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
