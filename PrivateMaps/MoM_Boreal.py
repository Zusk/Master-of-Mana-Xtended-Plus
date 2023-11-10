#
#	FILE:	 Boreal.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Regional map script - Boreal forest region / tundra.
#-----------------------------------------------------------------------------
#	Copyright (c) 2007 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
import random
import sys
from math import sqrt
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator

# Subclass
class BorealFractalWorld(CvMapGeneratorUtil.FractalWorld):
	def generatePlotTypes(self, water_percent=78, shift_plot_types=True, grain_amount=3):
		# Check for changes to User Input variances.
		self.checkForOverrideDefaultUserInputVariances()
		
		self.hillsFrac.fracInit(self.iNumPlotsX, self.iNumPlotsY, 2, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.peaksFrac.fracInit(self.iNumPlotsX, self.iNumPlotsY, 5, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		water_percent += self.seaLevelChange
		water_percent = min(water_percent, self.seaLevelMax)
		water_percent = max(water_percent, self.seaLevelMin)

		iWaterThreshold = self.continentsFrac.getHeightFromPercent(water_percent)
		iHillsBottom1 = self.hillsFrac.getHeightFromPercent(82)
		iHillsBottom2 = self.peaksFrac.getHeightFromPercent(90)
		iPeakThreshold = self.hillsFrac.getHeightFromPercent(90)
		iSecondPeakThreshold = self.peaksFrac.getHeightFromPercent(97)

		for x in range(self.iNumPlotsX):
			for y in range(self.iNumPlotsY):
				i = y*self.iNumPlotsX + x
				val = self.continentsFrac.getHeight(x,y)
				if val <= iWaterThreshold:
					self.plotTypes[i] = PlotTypes.PLOT_OCEAN
				else:
					hillVal = self.hillsFrac.getHeight(x,y)
					peakVal = self.peaksFrac.getHeight(x,y)
					if hillVal >= iHillsBottom1:
						if (hillVal >= iPeakThreshold):
							self.plotTypes[i] = PlotTypes.PLOT_PEAK
						else:
							self.plotTypes[i] = PlotTypes.PLOT_HILLS
					elif peakVal >= iHillsBottom2:
						if peakVal >= iSecondPeakThreshold:
							self.plotTypes[i] = PlotTypes.PLOT_PEAK
						else:
							self.plotTypes[i] = PlotTypes.PLOT_HILLS
					else:
						self.plotTypes[i] = PlotTypes.PLOT_LAND

		if shift_plot_types:
			self.shiftPlotTypes()

		return self.plotTypes

# subclass TerrainGenerator to redefine everything. This is a regional map.
class BorealTerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def __init__(self, fracXExp=-1, fracYExp=-1):
		self.gc = CyGlobalContext()
		self.map = CyMap()

		self.iWidth = self.map.getGridWidth()
		self.iHeight = self.map.getGridHeight()

		self.mapRand = self.gc.getGame().getMapRand()

		self.iFlags = 0  # Disallow FRAC_POLAR flag, to prevent "zero row" problems.

		self.ice=CyFractal()
		self.plains=CyFractal()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.initFractals()
		
	def initFractals(self):
		self.ice.fracInit(self.iWidth, self.iHeight, 1, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.plains.fracInit(self.iWidth, self.iHeight, 4, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		self.iIce = self.ice.getHeightFromPercent(86)
		self.iIceEdge = self.plains.getHeightFromPercent(94)
		self.iPlains = self.plains.getHeightFromPercent(21)

		self.terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
		self.terrainTundra = self.gc.getInfoTypeForString("TERRAIN_TUNDRA")
		self.terrainIce = self.gc.getInfoTypeForString("TERRAIN_SNOW")

	def generateTerrainAtPlot(self,iX,iY):
		if (self.map.plot(iX, iY).isWater()):
			return self.map.plot(iX, iY).getTerrainType()
		else:
			iceVal = self.ice.getHeight(iX, iY)
			plainsVal = self.plains.getHeight(iX, iY)
			if iceVal >= self.iIce:
				terrainVal = self.terrainIce
			elif plainsVal >= self.iIceEdge:
				terrainVal = self.terrainIce
			elif plainsVal <= self.iPlains:
				terrainVal = self.terrainPlains
			else:
				terrainVal = self.terrainTundra

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal

class BorealFeatureGenerator(CvMapGeneratorUtil.FeatureGenerator):
	def __init__(self, forest_grain=5, fracXExp=-1, fracYExp=-1):
		self.gc = CyGlobalContext()
		self.map = CyMap()
		self.mapRand = self.gc.getGame().getMapRand()
		self.forests = CyFractal()
		
		self.iFlags = 0 

		self.iGridW = self.map.getGridWidth()
		self.iGridH = self.map.getGridHeight()
		
		self.forest_grain = forest_grain + self.gc.getWorldInfo(self.map.getWorldSize()).getFeatureGrainChange()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.__initFractals()
		self.__initFeatureTypes()
	
	def __initFractals(self):
		self.forests.fracInit(self.iGridW, self.iGridH, self.forest_grain, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		
		self.iForestLevel1 = self.forests.getHeightFromPercent(90)
		self.iForestLevel2 = self.forests.getHeightFromPercent(15)

	def __initFeatureTypes(self):
		self.featureForest = self.gc.getInfoTypeForString("FEATURE_FOREST")

	def addFeaturesAtPlot(self, iX, iY):
		pPlot = self.map.sPlot(iX, iY)
		
		if pPlot.isPeak() or pPlot.isWater(): pass
		
		else:
			if pPlot.isRiverSide() and pPlot.isFlatlands():
				if pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_SNOW"):
					print('Changing River Ice Plot to Tundra')
					terrainTundra = self.gc.getInfoTypeForString("TERRAIN_TUNDRA")
					pPlot.setTerrainType(terrainTundra, true, true)
				elif pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_TUNDRA"):
					print('Changing River Tundra Plot to Plains')
					terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
					pPlot.setTerrainType(terrainPlains, true, true)
				elif pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_PLAINS"):
					print('Changing River Plains Plot to Grass')
					terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
					pPlot.setTerrainType(terrainGrass, true, true)
			self.addForestsAtPlot(pPlot, iX, iY)

	def addForestsAtPlot(self, pPlot, iX, iY):
		if pPlot.getTerrainType() != self.gc.getInfoTypeForString("TERRAIN_SNOW"):
			if self.forests.getHeight(iX, iY) <= self.iForestLevel1 and self.forests.getHeight(iX, iY) >= self.iForestLevel2:
				if self.gc.getFeatureInfo(self.featureForest).isTerrain(pPlot.getTerrainType()):
					pPlot.setFeatureType(self.featureForest, 2)

class MoM_Boreal:
					
	def getDescription(self):
		return "TXT_KEY_MAP_SCRIPT_BOREAL_DESCR"

	def isClimateMap(self):
		return 0

	def isSeaLevelMap(self):
		return 0

	def getNumCustomMapOptions(self):
		return 1

	def getNumHiddenCustomMapOptions(self):
		return 1

	def getCustomMapOptionName(self, argsList):
		translated_text = unicode(CyTranslator().getText("TXT_KEY_MAP_WORLD_WRAP", ()))
		return translated_text
		
	def getNumCustomMapOptionValues(self, argsList):
		return 3
		
	def getCustomMapOptionDescAt(self, argsList):
		iSelection = argsList[1]
		selection_names = ["TXT_KEY_MAP_WRAP_FLAT",
						   "TXT_KEY_MAP_WRAP_CYLINDER",
						   "TXT_KEY_MAP_WRAP_TOROID"]
		translated_text = unicode(CyTranslator().getText(selection_names[iSelection], ()))
		return translated_text
		
	def getCustomMapOptionDefault(self, argsList):
		return 0

	def isRandomCustomMapOption(self, argsList):
		return false

	def getWrapX(self):
		map = CyMap()
		return (map.getCustomMapOption(0) == 1 or map.getCustomMapOption(0) == 2)
		
	def getWrapY(self):
		map = CyMap()
		return (map.getCustomMapOption(0) == 2)

	def getTopLatitude(self):
		return 90
		
	def getBottomLatitude(self):
		return 70

	def beforeGeneration(self):
		gc = CyGlobalContext()
		map = CyMap()
		dice = gc.getGame().getMapRand()
		iW = map.getGridWidth()
		iH = map.getGridHeight()
		global food
		food = CyFractal()
		food.fracInit(iW, iH, 7, dice, 0, -1, -1)
			

	def generatePlotTypes(self):
		NiTextOut("Setting Plot Types (Python Boreal) ...")
		global fractal_world
		fractal_world = BorealFractalWorld()
		fractal_world.initFractal(continent_grain=3, rift_grain = -1, has_center_rift = False, polar = False)
		plot_types = fractal_world.generatePlotTypes(water_percent = 12)
		return plot_types


	def generateTerrainTypes(self):
		NiTextOut("Generating Terrain (Python Boreal) ...")
		terraingen = BorealTerrainGenerator()
		terrainTypes = terraingen.generateTerrain()
		return terrainTypes

	def addFeatures(self):
		NiTextOut("Adding Features (Python Boreal) ...")
		featuregen = BorealFeatureGenerator()
		featuregen.addFeatures()
		return 0


mapscript = MoM_Boreal()
			
def getDescription():
	return mapscript.getDescription()

#MapOptions
def isClimateMap():
	return mapscript.isClimateMap()

def isSeaLevelMap():
	return mapscript.isSeaLevelMap()

def getNumCustomMapOptions():
	return mapscript.getNumCustomMapOptions()

def getNumHiddenCustomMapOptions():
	return mapscript.getNumHiddenCustomMapOptions()

def getCustomMapOptionName(argsList):
	return mapscript.getCustomMapOptionName(argsList)
	
def getNumCustomMapOptionValues(argsList):
	return mapscript.getNumCustomMapOptionValues(argsList)
	
def getCustomMapOptionDescAt(argsList):
	return mapscript.getCustomMapOptionDescAt(argsList)
	
def getCustomMapOptionDefault(argsList):
	return mapscript.getCustomMapOptionDefault(argsList)

def isRandomCustomMapOption(argsList):
	return mapscript.isRandomCustomMapOption(argsList)

#general functions
def beforeGeneration():
	return mapscript.beforeGeneration()

def getWrapX():
	return mapscript.getWrapX()
	
def getWrapY():
	return mapscript.getWrapY()
	
def getTopLatitude():
	return mapscript.getTopLatitude()
	
def getBottomLatitude():
	return mapscript.getBottomLatitude()

def generatePlotTypes():
	return mapscript.generatePlotTypes()
	
def generateTerrainTypes():
	return mapscript.generateTerrainTypes()
	
def addFeatures():
	return mapscript.addFeatures()