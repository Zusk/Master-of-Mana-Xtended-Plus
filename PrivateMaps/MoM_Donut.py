#
#	FILE:	 Donut.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Global map script - Circular continent with center region.
#-----------------------------------------------------------------------------
#	Copyright (c) 2007 Firaxis Games, Inc. All rights reserved.
#-----------------------------------------------------------------------------
#

from CvPythonExtensions import *
import CvUtil
import CvMapGeneratorUtil
from math import sqrt
from CvMapGeneratorUtil import FractalWorld
from CvMapGeneratorUtil import TerrainGenerator
from CvMapGeneratorUtil import FeatureGenerator
from CvMapGeneratorUtil import BonusBalancer

balancer = BonusBalancer()

class DonutFeatureGenerator(CvMapGeneratorUtil.FeatureGenerator):
	def addIceAtPlot(self, pPlot, iX, iY, lat):
		# We don' need no steeking ice. M'kay? Alrighty then.
		ice = 0
		
	def addJunglesAtPlot(self, pPlot, iX, iY, lat):
		if (CyMap().getCustomMapOption(1) == 1): pass #No Jungles option
		else: #Normal Jungles
			if pPlot.canHaveFeature(self.featureJungle):
				iJungleHeight = self.jungles.getHeight(iX, iY)
				if self.iJungleTop >= iJungleHeight >= self.iJungleBottom + (self.iJungleTop - self.iJungleBottom)*self.gc.getClimateInfo(self.map.getClimate()).getJungleLatitude()*lat:
					pPlot.setFeatureType(self.featureJungle, -1)


# subclass TerrainGenerator to create a lush grassland utopia.
class DonutTerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def __init__(self, fracXExp=-1, fracYExp=-1, grain_amount=5):
		self.gc = CyGlobalContext()
		self.map = CyMap()

		self.grain_amount = grain_amount + self.gc.getWorldInfo(self.map.getWorldSize()).getTerrainGrainChange()

		self.iWidth = self.map.getGridWidth()
		self.iHeight = self.map.getGridHeight()

		self.mapRand = self.gc.getGame().getMapRand()

		self.iFlags = 0  # Disallow FRAC_POLAR flag, to prevent "zero row" problems.

		self.terrain=CyFractal()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.initFractals()

		self.iCenterX = int(self.map.getGridWidth() / 2)
		self.iCenterY = int(self.map.getGridHeight() / 2)
		self.iRadius = min((self.iCenterX - 4), (self.iCenterY - 4))
		self.iHoleRadius = int(self.iRadius / 2)
		self.userInputCenter = self.map.getCustomMapOption(0)
		
	def initFractals(self):
		self.terrain.fracInit(self.iWidth, self.iHeight, self.grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.iGrassBottom = self.terrain.getHeightFromPercent(12)

		self.terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
		self.terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
		self.terrainDesert = self.gc.getInfoTypeForString("TERRAIN_DESERT")

	def getLatitudeAtPlot(self, iX, iY):
		return None

	def generateTerrain(self):		
		terrainData = [0]*(self.iWidth*self.iHeight)
		for x in range(self.iWidth):
			for y in range(self.iHeight):
				iI = y*self.iWidth + x
				terrain = self.generateTerrainAtPlot(x, y)
				terrainData[iI] = terrain
		return terrainData

	def generateTerrainAtPlot(self,iX,iY):
		if (self.map.plot(iX, iY).isWater()):
			return self.map.plot(iX, iY).getTerrainType()

		if iX == self.iCenterX and iY == self.iCenterY:
			fDistance = 0
		else:
			fDistance = sqrt(((iX - self.iCenterX) ** 2) + ((iY - self.iCenterY) ** 2))
		if fDistance < self.iHoleRadius and self.userInputCenter != 4: # Plot is in hole of donut.
			if self.userInputCenter == 2:
				terrainVal = self.terrainDesert
			elif self.userInputCenter == 3:
				return self.map.plot(iX, iY).getTerrainType()
			else:
				terrainVal = self.terrainGrass
		else:
			val = self.terrain.getHeight(iX, iY)
			if val >= self.iGrassBottom:
				terrainVal = self.terrainGrass
			else:
				terrainVal = self.terrainPlains

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal
		
class DonutFractalWorld(CvMapGeneratorUtil.FractalWorld):
	def generatePlotTypes(self, water_percent=78, shift_plot_types=True, grain_amount=3):
		self.hillsFrac.fracInit(self.iNumPlotsX, self.iNumPlotsY, grain_amount, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.peaksFrac.fracInit(self.iNumPlotsX, self.iNumPlotsY, grain_amount+1, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		iHillsBottom1 = self.hillsFrac.getHeightFromPercent(max((self.hillGroupOneBase - self.hillGroupOneRange), 0))
		iHillsTop1 = self.hillsFrac.getHeightFromPercent(min((self.hillGroupOneBase + self.hillGroupOneRange), 100))
		iHillsBottom2 = self.hillsFrac.getHeightFromPercent(max((self.hillGroupTwoBase - self.hillGroupTwoRange), 0))
		iHillsTop2 = self.hillsFrac.getHeightFromPercent(min((self.hillGroupTwoBase + self.hillGroupTwoRange), 100))
		iPeakThreshold = self.peaksFrac.getHeightFromPercent(self.peakPercent)
		
		iCenterX = int(self.iNumPlotsX / 2)
		iCenterY = int(self.iNumPlotsY / 2)
		iRadius = min((iCenterX - 4), (iCenterY - 4))
		iHoleRadius = int(iRadius / 2)
		userInputCenter = self.map.getCustomMapOption(0)

		for x in range(self.iNumPlotsX):
			for y in range(self.iNumPlotsY):
				i = y*self.iNumPlotsX + x
				if x == iCenterX and y == iCenterY:
					fDistance = 0
				else:
					fDistance = sqrt(((((x - iCenterX) * self.iNumPlotsY) / self.iNumPlotsX) ** 2) + ((y - iCenterY) ** 2))					
				if fDistance > iRadius:
					self.plotTypes[i] = PlotTypes.PLOT_OCEAN
				elif fDistance < iHoleRadius and userInputCenter != 4: # Plot is in hole of donut.
					if userInputCenter == 0:
						self.plotTypes[i] = PlotTypes.PLOT_HILLS
					elif userInputCenter == 2:
						self.plotTypes[i] = PlotTypes.PLOT_LAND
					elif userInputCenter == 3:
						self.plotTypes[i] = PlotTypes.PLOT_OCEAN
					else:
						self.plotTypes[i] = PlotTypes.PLOT_PEAK
				else:
					hillVal = self.hillsFrac.getHeight(x,y)
					if ((hillVal >= iHillsBottom1 and hillVal <= iHillsTop1) or (hillVal >= iHillsBottom2 and hillVal <= iHillsTop2)):
						peakVal = self.peaksFrac.getHeight(x,y)
						if (peakVal <= iPeakThreshold):
							self.plotTypes[i] = PlotTypes.PLOT_PEAK
						else:
							self.plotTypes[i] = PlotTypes.PLOT_HILLS
					else:
						self.plotTypes[i] = PlotTypes.PLOT_LAND

		if shift_plot_types:
			self.shiftPlotTypes()

		return self.plotTypes
		

class MoM_Donut:
		
	def getDescription(self):
		return "TXT_KEY_MAP_SCRIPT_DONUT_DESCR"

	def getNumCustomMapOptions(self):
		return 4

	def getNumHiddenCustomMapOptions(self):
		return 2

	def getCustomMapOptionName(self, argsList):
		[iOption] = argsList
		option_names = {
			0:	"TXT_KEY_MAP_SCRIPT_CENTER_REGION",
			1:	"TXT_KEY_FEATURE_JUNGLE",
			2:	"TXT_KEY_MAP_WORLD_WRAP",
			3:  "TXT_KEY_CONCEPT_RESOURCES"
			}
		translated_text = unicode(CyTranslator().getText(option_names[iOption], ()))
		return translated_text
		
	def getNumCustomMapOptionValues(self, argsList):
		[iOption] = argsList
		option_values = {
			0:	5,
			1:	2,
			2:	3,
			3:  2
			}
		return option_values[iOption]
		
	def getCustomMapOptionDescAt(self, argsList):
		[iOption, iSelection] = argsList
		selection_names = {
			0:	{
				0: "TXT_KEY_MAP_SCRIPT_HILLS",
				1: "TXT_KEY_MAP_SCRIPT_PEAKS",
				2: "TXT_KEY_MAP_SCRIPT_DESERT",
				3: "TXT_KEY_MAP_SCRIPT_OCEAN",
				4: "TXT_KEY_WORLD_STANDARD"
				},
			1:	{
				0: "TXT_KEY_FEATURE_JUNGLE",
				1: "TXT_KEY_MAP_SCRIPT_NO_JUNGLES"
				},
			2:	{
				0: "TXT_KEY_MAP_WRAP_FLAT",
				1: "TXT_KEY_MAP_WRAP_CYLINDER",
				2: "TXT_KEY_MAP_WRAP_TOROID"
				},
			3:	{
				0: "TXT_KEY_WORLD_STANDARD",
				1: "TXT_KEY_MAP_BALANCED"
				}
			}
		translated_text = unicode(CyTranslator().getText(selection_names[iOption][iSelection], ()))
		return translated_text
		
	def getCustomMapOptionDefault(self, argsList):
		[iOption] = argsList
		option_defaults = {
			0:	1,
			1:	0,
			2:	0,
			3:  0
			}
		return option_defaults[iOption]

	def isRandomCustomMapOption(self, argsList):
		[iOption] = argsList
		option_random = {
			0:	false,
			1:	false,
			2:	false,
			3:  false
			}
		return option_random[iOption]

	def getWrapX(self):
		map = CyMap()
		return (map.getCustomMapOption(2) == 1 or map.getCustomMapOption(2) == 2)
		
	def getWrapY(self):
		map = CyMap()
		return (map.getCustomMapOption(2) == 2)
		
	def normalizeAddExtras(self):
		if (CyMap().getCustomMapOption(3) == 1):
			balancer.normalizeAddExtras()
		CyPythonMgr().allowDefaultImpl()	# do the rest of the usual normalizeStartingPlots stuff, don't overrride

	def addBonusType(self, argsList):
		[iBonusType] = argsList
		gc = CyGlobalContext()
		type_string = gc.getBonusInfo(iBonusType).getType()

		if (CyMap().getCustomMapOption(3) == 1):
			if (type_string in balancer.resourcesToBalance) or (type_string in balancer.resourcesToEliminate):
				return None # don't place any of this bonus randomly
			
		CyPythonMgr().allowDefaultImpl() # pretend we didn't implement this method, and let C handle this bonus in the default way

	def isClimateMap(self):
		return 0

	def isSeaLevelMap(self):
		return 0

	def generatePlotTypes(self):
		NiTextOut("Setting Plot Types (Python Donut) ...")
		fractal_world = DonutFractalWorld()
		return fractal_world.generatePlotTypes()

	def generateTerrainTypes(self):
		NiTextOut("Generating Terrain (Python Donut) ...")
		terraingen = DonutTerrainGenerator()
		terrainTypes = terraingen.generateTerrain()
		return terrainTypes

	def addFeatures(self):
		NiTextOut("Adding Features (Python Donut) ...")
		featuregen = DonutFeatureGenerator()
		featuregen.addFeatures()
		return 0
		
mapscript = MoM_Donut()
			
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
def getWrapX():
	return mapscript.getWrapX()
	
def getWrapY():
	return mapscript.getWrapY()
	
def generatePlotTypes():
	return mapscript.generatePlotTypes()
	
def generateTerrainTypes():
	return mapscript.generateTerrainTypes()
	
def addFeatures():
	return mapscript.addFeatures()
	
def addBonusType(argsList):	
	return mapscript.addBonusType(argsList)