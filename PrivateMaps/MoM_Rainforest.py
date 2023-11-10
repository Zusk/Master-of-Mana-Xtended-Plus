#
#	FILE:	 Rainforest.py
#	AUTHOR:  Bob Thomas (Sirian)
#	PURPOSE: Regional map script - Tropical rainforest
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
class RainforestFractalWorld(CvMapGeneratorUtil.FractalWorld):
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
class RainforestTerrainGenerator(CvMapGeneratorUtil.TerrainGenerator):
	def __init__(self, fracXExp=-1, fracYExp=-1):
		self.gc = CyGlobalContext()
		self.map = CyMap()

		self.iWidth = self.map.getGridWidth()
		self.iHeight = self.map.getGridHeight()

		self.mapRand = self.gc.getGame().getMapRand()

		self.iFlags = 0  # Disallow FRAC_POLAR flag, to prevent "zero row" problems.

		self.desert=CyFractal()
		self.plains=CyFractal()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.initFractals()
		
	def initFractals(self):
		self.desert.fracInit(self.iWidth, self.iHeight, 1, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.plains.fracInit(self.iWidth, self.iHeight, 4, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		self.iDesert = self.desert.getHeightFromPercent(86)
		self.iDesertEdge = self.desert.getHeightFromPercent(82)
		self.iPlains = self.plains.getHeightFromPercent(95)

		self.terrainDesert = self.gc.getInfoTypeForString("TERRAIN_DESERT")
		self.terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
		self.terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")

	def generateTerrainAtPlot(self,iX,iY):
		if (self.map.plot(iX, iY).isWater()):
			return self.map.plot(iX, iY).getTerrainType()
		else:
			desertVal = self.desert.getHeight(iX, iY)
			plainsVal = self.plains.getHeight(iX, iY)
			if desertVal >= self.iDesert:
				terrainVal = self.terrainDesert
			elif desertVal >= self.iDesertEdge:
				terrainVal = self.terrainPlains
			elif plainsVal >= self.iPlains:
				terrainVal = self.terrainPlains
			else:
				terrainVal = self.terrainGrass

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal

class RainforestFeatureGenerator(CvMapGeneratorUtil.FeatureGenerator):
	def __init__(self, iJunglePercent=85, iForestPercent=15,
	             forest_grain=6, fracXExp=-1, fracYExp=-1):
		self.gc = CyGlobalContext()
		self.map = CyMap()
		self.mapRand = self.gc.getGame().getMapRand()
		self.forests = CyFractal()
		
		self.iFlags = 0 

		self.iGridW = self.map.getGridWidth()
		self.iGridH = self.map.getGridHeight()
		
		self.iJunglePercent = iJunglePercent
		self.iForestPercent = iForestPercent
		
		self.forest_grain = forest_grain + self.gc.getWorldInfo(self.map.getWorldSize()).getFeatureGrainChange()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.__initFractals()
		self.__initFeatureTypes()
	
	def __initFractals(self):
		self.forests.fracInit(self.iGridW, self.iGridH, self.forest_grain, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		
		self.iJungleLevel = self.forests.getHeightFromPercent(self.iJunglePercent)
		self.iForestLevel = self.forests.getHeightFromPercent(self.iForestPercent)

	def __initFeatureTypes(self):
		self.featureJungle = self.gc.getInfoTypeForString("FEATURE_JUNGLE")
		self.featureForest = self.gc.getInfoTypeForString("FEATURE_FOREST")
		self.featureOasis = self.gc.getInfoTypeForString("FEATURE_OASIS")
		self.featureFlood = self.gc.getInfoTypeForString("FEATURE_FLOOD_PLAINS")

	def addFeaturesAtPlot(self, iX, iY):
		pPlot = self.map.sPlot(iX, iY)
		
		if pPlot.isPeak() or pPlot.isWater(): pass
		
		else:
			if pPlot.isRiverSide() and pPlot.isFlatlands():
				self.addFloodAtPlot(pPlot, iX, iY)

			if (pPlot.getFeatureType() == FeatureTypes.NO_FEATURE):
				self.addOasisAtPlot(pPlot, iX, iY)

			if (pPlot.getFeatureType() == FeatureTypes.NO_FEATURE):
				self.addForestsAtPlot(pPlot, iX, iY)

			if (pPlot.getFeatureType() == FeatureTypes.NO_FEATURE):
				self.addJunglesAtPlot(pPlot, iX, iY)
			
	def addFloodAtPlot(self, pPlot, iX, iY):
		if pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_DESERT"):
			pPlot.setFeatureType(self.featureFlood, -1)

	def addOasisAtPlot(self, pPlot, iX, iY):
		if not pPlot.isFreshWater():
			if pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_DESERT"):
				if self.mapRand.get(25, "Add Feature PYTHON") == 23:
					pPlot.setFeatureType(self.featureOasis, -1)
	
	def addJunglesAtPlot(self, pPlot, iX, iY):
		# Warning: this version of JunglesAtPlot is using the forest fractal!
		if pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_GRASS"):
			if (self.forests.getHeight(iX, iY) <= self.iJungleLevel):
				pPlot.setFeatureType(self.featureJungle, -1)

	def addForestsAtPlot(self, pPlot, iX, iY):
		if pPlot.getTerrainType() != self.gc.getInfoTypeForString("TERRAIN_DESERT"):
			if self.forests.getHeight(iX, iY) <= self.iForestLevel:
				pPlot.setFeatureType(self.featureForest, -1)


class MoM_Rainforest:
	def getDescription(self):
		return "TXT_KEY_MAP_SCRIPT_RAINFOREST_DESCR"
	
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
		return 15
		
	def getBottomLatitude(self):
		return -15

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
		NiTextOut("Setting Plot Types (Python Rainforest) ...")
		global fractal_world
		fractal_world = RainforestFractalWorld()
		fractal_world.initFractal(continent_grain=3, rift_grain = -1, has_center_rift = False, polar = False)
		plot_types = fractal_world.generatePlotTypes(water_percent = 5)
		return plot_types

	def generateTerrainTypes(self):
		NiTextOut("Generating Terrain (Python Rainforest) ...")
		terraingen = RainforestTerrainGenerator()
		terrainTypes = terraingen.generateTerrain()
		return terrainTypes
	
	def addFeatures(self):
		NiTextOut("Adding Features (Python Rainforest) ...")
		featuregen = RainforestFeatureGenerator()
		featuregen.addFeatures()
		return 0
	
	# Sirian's "Sahara Regional Bonus Placement" system.
	def addBonusType(self, argsList):

		# Init all bonuses. This is your master key.
		resourcesToEliminate = ('BONUS_WHALE', 'BONUS_CLAM', 'BONUS_FISH', 'BONUS_CRAB')

		jungleFood = ('BONUS_BANANA', 'BONUS_PIG', 'BONUS_RICE')
		banana = ('BONUS_BANANA')
		pig = ('BONUS_PIG')
		rice = ('BONUS_RICE')

		resourcesToForce = ('BONUS_FUR', 'BONUS_SILVER', 'BONUS_DEER')
		forcePlacementOnFlats = ('BONUS_FUR', 'BONUS_DEER')
		forcePlacementOnHills = ('BONUS_SILVER')

		print('*******')
		[iBonusType] = argsList
		gc = CyGlobalContext()
		map = CyMap()
		dice = gc.getGame().getMapRand()
		type_string = gc.getBonusInfo(iBonusType).getType()

		if (type_string in resourcesToEliminate):
			print('-NONE-', type_string, '-NONE-')
			return None # These bonus types will not appear, at all.
		elif not ((type_string in resourcesToForce) or (type_string in jungleFood)):
			CyPythonMgr().allowDefaultImpl() # Let C handle this bonus in the default way.
		else: # Current bonus type is custom-handled. Assignments to follow.
			print('+++', type_string, '+++')
			iW = map.getGridWidth()
			iH = map.getGridHeight()

			# init forced-eligibility flags
			if (type_string not in resourcesToForce): unforced = True
			else: unforced = False
			forceFlats = False
			forceHills = False
			if (type_string in forcePlacementOnFlats): forceFlats = True
			if (type_string in forcePlacementOnHills): forceHills = True

			# Generate jungle food
			# Note: any fractal assignment of bonuses, like this one, must come before determining the count for forced bonuses.
			if (type_string in jungleFood):
				print('---', type_string, '---')
				global food
				NiTextOut("Placing Jungle Food (Jungle Food - Python Rainforest) ...")
				iRiceBottom = food.getHeightFromPercent(24)
				iRiceTop = food.getHeightFromPercent(27)
				iPigBottom = food.getHeightFromPercent(49)
				iPigTop = food.getHeightFromPercent(52)
				iBananaBottom = food.getHeightFromPercent(73)
				iBananaTop = food.getHeightFromPercent(76)

				for y in range(iH):
					for x in range(iW):
						# Fractalized placement
						pPlot = map.plot(x,y)
						if pPlot.isWater() or pPlot.isPeak() or pPlot.getFeatureType() != gc.getInfoTypeForString("FEATURE_JUNGLE"): continue
						if pPlot.getBonusType(-1) == -1:
							foodVal = food.getHeight(x,y)
							if (type_string in banana):
								if (foodVal >= iBananaBottom and foodVal <= iBananaTop):
									map.plot(x,y).setBonusType(iBonusType)
							if (type_string in pig):
								if (foodVal >= iPigBottom and foodVal <= iPigTop):
									map.plot(x,y).setBonusType(iBonusType)
							if (type_string in rice):
								if (foodVal >= iRiceBottom and foodVal <= iRiceTop):
									map.plot(x,y).setBonusType(iBonusType)

				return None

			# determine number of bonuses to place (defined as count)
			# size modifier is a fixed component based on world size
			sizekey = map.getWorldSize()
			sizevalues = {
				WorldSizeTypes.WORLDSIZE_DUEL:		1,
				WorldSizeTypes.WORLDSIZE_TINY:		1,
				WorldSizeTypes.WORLDSIZE_SMALL:		1,
				WorldSizeTypes.WORLDSIZE_STANDARD:	2,
				WorldSizeTypes.WORLDSIZE_LARGE:		2,
				WorldSizeTypes.WORLDSIZE_HUGE:		3
				}
			sizemodifier = sizevalues[sizekey]
			# playermodifier involves two layers of randomnity.
			players = gc.getGame().countCivPlayersEverAlive()
			plrcomponent1 = int(players / 3.0) # Bonus Method Fixed Component
			plrcomponent2 = dice.get(players, "Bonus Method Abundant Component - Rainforest PYTHON") + 1
			plrcomponent3 = dice.get(int(players / 1.6), "Bonus Method Medium Component - Rainforest PYTHON") - 1
			plrmethods = [plrcomponent1, plrcomponent2, plrcomponent3]

			playermodifier = plrmethods[dice.get(3, "Forced Bonus NoRarity - Rainforest PYTHON")]
			count = sizemodifier + playermodifier
			if count <= 0:
				return None # This bonus drew a short straw. None will be placed!

			# Set plot eligibility for current bonus.
			# Begin by initiating the list, into which eligible plots will be recorded.
			eligible = []
			# Loop through all plots on the map, adding eligible plots to the list.
			for x in range(iW):
				for y in range(iH):
					# First check the plot for an existing bonus.
					pPlot = map.plot(x,y)
					if pPlot.getBonusType(-1) != -1: continue # to next plot.
					if pPlot.getFeatureType() == gc.getInfoTypeForString("FEATURE_OASIS"): continue # Soren wants no bonuses in oasis plots. So mote it be.
					# Check plot type and features for eligibility.
					if forceHills and pPlot.isHills(): pass
					elif (forceFlats and pPlot.isFlatlands()) and (pPlot.getFeatureType() == gc.getInfoTypeForString("FEATURE_FOREST")): pass
					else: continue # to next plot.
					# Finally we have run all the checks.
					# 1. The plot has no bonus.
					# 2. The plot has an eligible terrain and feature type.
					# Now we append this plot to the eligible list.
					eligible.append([x,y])
										
			# Now we assign the bonuses to eligible plots chosen completely at random.
			while count > 0:
				if eligible == []: break # No eligible plots left!
				index = dice.get(len(eligible), "Bonus Placement - Rainforest PYTHON")
				[x,y] = eligible[index]
				map.plot(x,y).setBonusType(iBonusType)
				del eligible[index] # Remove this plot from the eligible list.
				count = count - 1  # Reduce number of bonuses left to place.
			# This bonus type is done.

			return None
			

mapscript = MoM_Rainforest()
			
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
	
def addBonusType(argsList):	
	return mapscript.addBonusType(argsList)