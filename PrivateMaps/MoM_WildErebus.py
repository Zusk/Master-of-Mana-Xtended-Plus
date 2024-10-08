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

#overrides of CvMapgeneratorUtils class

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
		iHillsBottom2 = self.peaksFrac.getHeightFromPercent(85)
		iPeakThreshold = self.hillsFrac.getHeightFromPercent(92)
		iSecondPeakThreshold = self.peaksFrac.getHeightFromPercent(94)

		hill1 = self.hillsFrac.getHeightFromPercent(53)
		hill2 = self.hillsFrac.getHeightFromPercent(82)
		peak1 = self.hillsFrac.getHeightFromPercent(60)
		peak2 = self.hillsFrac.getHeightFromPercent(75)
		for x in range(self.iNumPlotsX):
			for y in range(self.iNumPlotsY):
				i = y*self.iNumPlotsX + x
				val = self.continentsFrac.getHeight(x,y)
				if val <= iWaterThreshold:
					self.plotTypes[i] = PlotTypes.PLOT_OCEAN
				else:
					hillVal = self.hillsFrac.getHeight(x,y)
					peakVal = self.peaksFrac.getHeight(x,y)
					if hillVal >= hill1 and hillVal <= hill2:
						if (hillVal >= peak1 and hillVal <= peak2):
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

		self.heat=CyFractal()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.initFractals()
		
	def initFractals(self):
		self.heat.fracInit(self.iWidth, self.iHeight, 2, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)

		self.iDesert = self.heat.getHeightFromPercent(80)
		self.iPlains = self.heat.getHeightFromPercent(48)
		self.iGrass = self.heat.getHeightFromPercent(20)
		self.iTundra = self.heat.getHeightFromPercent(8)

		self.terrainDesert = self.gc.getInfoTypeForString("TERRAIN_DESERT")
		self.terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
		self.terrainPlains = self.gc.getInfoTypeForString("TERRAIN_PLAINS")
		self.terrainTundra = self.gc.getInfoTypeForString("TERRAIN_TUNDRA")
		self.terrainIce = self.gc.getInfoTypeForString("TERRAIN_SNOW")

	def generateTerrainAtPlot(self,iX,iY):
#		terrainVal = self.terrainPlains
#		return terrainVal
		if (self.map.plot(iX, iY).isWater()):
			return self.map.plot(iX, iY).getTerrainType()
		else:
			heatVal = self.heat.getHeight(iX, iY)
			if heatVal >= self.iDesert:
				terrainVal = self.terrainDesert
			elif heatVal >= self.iPlains:
				terrainVal = self.terrainPlains
			elif heatVal >= self.iGrass:
				terrainVal = self.terrainGrass
			elif heatVal >= self.iTundra:
				terrainVal = self.terrainTundra
			else:
				terrainVal = self.terrainIce

		if (terrainVal == TerrainTypes.NO_TERRAIN):
			return self.map.plot(iX, iY).getTerrainType()

		return terrainVal
		
class BorealFeatureGenerator(CvMapGeneratorUtil.FeatureGenerator):
	def __init__(self, forest_grain=5, fracXExp=-1, fracYExp=-1):
		self.gc = CyGlobalContext()
		self.map = CyMap()
		self.mapRand = self.gc.getGame().getMapRand()
		self.forests = CyFractal()
		self.oasis = CyFractal()
		
		self.iFlags = 0 

		self.iGridW = self.map.getGridWidth()
		self.iGridH = self.map.getGridHeight()
		
		self.forest_grain = 3 # forest_grain + self.gc.getWorldInfo(self.map.getWorldSize()).getFeatureGrainChange()

		self.fracXExp = fracXExp
		self.fracYExp = fracYExp

		self.__initFractals()
		self.__initFeatureTypes()
	
	def __initFractals(self):
		self.forests.fracInit(self.iGridW, self.iGridH, self.forest_grain, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		self.oasis.fracInit(self.iGridW, self.iGridH, 8, self.mapRand, self.iFlags, self.fracXExp, self.fracYExp)
		
		self.iForestLevel = self.forests.getHeightFromPercent(80)
		self.iForestLevelPlains = self.forests.getHeightFromPercent(30)
		self.iJungleLevel = self.forests.getHeightFromPercent(60)
		self.iOasisLevel = self.oasis.getHeightFromPercent(25)

	def __initFeatureTypes(self):
		self.featureForest = self.gc.getInfoTypeForString("FEATURE_FOREST")
		self.featureJungle = self.gc.getInfoTypeForString("FEATURE_JUNGLE")
		self.featureOasis = self.gc.getInfoTypeForString("FEATURE_OASIS")
		self.featureFlood = self.gc.getInfoTypeForString("FEATURE_FLOOD_PLAINS")

	def addFeaturesAtPlot(self, iX, iY):
		pPlot = self.map.sPlot(iX, iY)
		dice = self.gc.getGame().getMapRand()
		
		
		if pPlot.isPeak(): pass
		
		else:
			diceSize = 30
			diceRoll = dice.get(2 * diceSize, "Dice roll - CA test PYTHON")
			# Make cold river tiles more fertile
			if pPlot.isRiver():
				if pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_SNOW"):
					print('Changing River Ice Plot to Tundra')
					terrainTundra = self.gc.getInfoTypeForString("TERRAIN_TUNDRA")
					pPlot.setTerrainType(terrainTundra, true, true)
				elif pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_TUNDRA"):
					print('Changing River Plains Plot to Grass')
					terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
					pPlot.setTerrainType(terrainGrass, true, true)
			
			# Add forests and jungles
			if pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_DESERT"):
				if self.forests.getHeight(iX, iY) >= self.iJungleLevel - diceSize + diceRoll and dice.get(100, "Dice Roll") > 8:
					terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
					pPlot.setTerrainType(terrainGrass, true, true)
					pPlot.setFeatureType(self.featureJungle, 1)
			elif pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_PLAINS"):
				if self.forests.getHeight(iX, iY) >= self.iJungleLevel - diceSize + diceRoll and dice.get(100, "Dice Roll") > 8:
					if self.forests.getHeight(iX, iY) > self.iForestLevel - diceSize + diceRoll:
						terrainGrass = self.gc.getInfoTypeForString("TERRAIN_GRASS")
						pPlot.setTerrainType(terrainGrass, true, true)
						pPlot.setFeatureType(self.featureJungle, 1)
					else:
						pPlot.setFeatureType(self.featureForest, 1)
				elif self.forests.getHeight(iX, iY) <= self.iForestLevelPlains - diceSize + diceRoll and dice.get(100, "Dice Roll") > 8:
					pPlot.setFeatureType(self.featureForest, 1)
			elif pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_GRASS"):
				if self.forests.getHeight(iX, iY) <= self.iForestLevel - diceSize + diceRoll and dice.get(100, "Dice Roll") > 8:
					pPlot.setFeatureType(self.featureForest, 1)
			elif pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_TUNDRA"):
				if self.forests.getHeight(iX, iY) <= self.iForestLevel - diceSize + diceRoll and dice.get(100, "Dice Roll") > 8:
					pPlot.setFeatureType(self.featureForest, 2)
			
			# Add flood plains and oasis to desert
			if pPlot.getTerrainType() == self.gc.getInfoTypeForString("TERRAIN_DESERT"):
				if pPlot.isFlatlands():
					if pPlot.isRiver():
						pPlot.setFeatureType(self.featureFlood, 1)
					else:
						if self.oasis.getHeight(iX, iY) <= self.iOasisLevel and pPlot.canHaveFeature(self.featureOasis):
							pPlot.setFeatureType(self.featureOasis, 1)

			if pPlot.getTerrainType()== self.gc.getInfoTypeForString("TERRAIN_COAST"):
				if diceRoll<10:
#			if pPlot.isWater():
					pPlot.setFeatureType(self.gc.getInfoTypeForString("FEATURE_KELP"), 1)

class WildErebus:
	def getDescription(self):
		return "TXT_KEY_MAP_SCRIPT_BOREAL_DESCR"	
		
#MapOptions		
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
		
#general functions		
	def getWrapX(self):
		map = CyMap()
		return (map.getCustomMapOption(0) == 1 or map.getCustomMapOption(0) == 2)
		
	def getWrapY(self):
		map = CyMap()
		return (map.getCustomMapOption(0) == 2)

	def getTopLatitude(self):
		return 95
		
	def getBottomLatitude(self):
		return 5
	
	def generatePlotTypes(self):
		NiTextOut("Setting Plot Types (Python Boreal) ...")
		global fractal_world
		fractal_world = BorealFractalWorld()
		fractal_world.initFractal(continent_grain=3, rift_grain = -1, has_center_rift = False, polar = False)
		plot_types = fractal_world.generatePlotTypes(water_percent = 21)
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
		CyMapGenerator().addKelp()
		return 0

	def addBonusType(self, argsList):
		print('*******')
		[iBonusType] = argsList
		gc = CyGlobalContext()
		map = CyMap()
		dice = gc.getGame().getMapRand()
		type_string = gc.getBonusInfo(iBonusType).getType()
		iW = map.getGridWidth()
		iH = map.getGridHeight()
		nrPlayers = gc.getGame().countCivPlayersEverAlive()

		# Sirian's "Sahara Regional Bonus Placement" system.

		bonusRare = ('BONUS_GOLD', 'BONUS_DYE', 'BONUS_RICE', 'BONUS_INCENSE', 'BONUS_SILK', 
									'BONUS_CORN', 'BONUS_COW', 'BONUS_IVORY', 'BONUS_GEMS', 'BONUS_SHEEP', 'BONUS_SUGAR', 'BONUS_WINE')
		bonusCommon = ('BONUS_OIL')
		bonusFractal = ('BONUS_DEER', 'BONUS_FUR', 'BONUS_STONE', 'BONUS_BANANA', 'BONUS_MARBLE')
		needsChopping = ('BONUS_CORN', 'BONUS_SHEEP', 'BONUS_WINE', 'BONUS_WHEAT', 'BONUS_IVORY', 'BONUS_COW')
		metal = ('BONUS_IRON', 'BONUS_COPPER', 'BONUS_GOLD', 'BONUS_SILVER', 'BONUS_ALUMINUM')
		deer = ('BONUS_DEER')
		fur = ('BONUS_FUR')
		banana = ('BONUS_BANANA')
		stone = ('BONUS_STONE')
		marble = ('BONUS_MARBLE')
			
		# Generate fractal featured resources	
		if (type_string in bonusFractal):
			print("Fractal resource implementation:", type_string)
			furDist = CyFractal()
			furDist.fracInit(iW, iH, 7, gc.getGame().getMapRand(), 0, -1, -1)
			furLevelBottom = furDist.getHeightFromPercent(80)
			furLevelTop = furDist.getHeightFromPercent(90)
			deerDist = CyFractal()
			deerDist.fracInit(iW, iH, 7, gc.getGame().getMapRand(), 0, -1, -1)
			deerLevelBottom = deerDist.getHeightFromPercent(75)
			deerLevelTop = deerDist.getHeightFromPercent(90)
			bananaDist = CyFractal()
			bananaDist.fracInit(iW, iH, 7, gc.getGame().getMapRand(), 0, -1, -1)
			bananaLevelBottom = bananaDist.getHeightFromPercent(75)
			bananaLevelTop = bananaDist.getHeightFromPercent(90)
			stoneDist = CyFractal()
			stoneDist.fracInit(iW, iH, 7, gc.getGame().getMapRand(), 0, -1, -1)
			stoneLevelBottom = stoneDist.getHeightFromPercent(80)
			stoneLevelTop = stoneDist.getHeightFromPercent(90)
			marbleDist = CyFractal()
			marbleDist.fracInit(iW, iH, 7, gc.getGame().getMapRand(), 0, -1, -1)
			marbleLevelBottom = marbleDist.getHeightFromPercent(80)
			marbleLevelTop = marbleDist.getHeightFromPercent(90)
			for y in range(iH):
				for x in range(iW):
					# Fractalized placement
					pPlot = map.plot(x,y)
					if pPlot.getBonusType(-1) != -1 or pPlot.isPeak():
						continue
					if (type_string in deer):
						if pPlot.isFlatlands() and pPlot.getFeatureType() == gc.getInfoTypeForString("FEATURE_FOREST"):
							distVal = deerDist.getHeight(x,y)
							if (distVal >= deerLevelBottom and distVal < deerLevelTop):
								map.plot(x,y).setBonusType(iBonusType)
					if (type_string in fur):
						if pPlot.getTerrainType() != gc.getInfoTypeForString("TERRAIN_PLAINS") and pPlot.getFeatureType() == gc.getInfoTypeForString("FEATURE_FOREST"):
							distVal = furDist.getHeight(x,y)
							if (distVal >= furLevelBottom and distVal < furLevelTop):
								map.plot(x,y).setBonusType(iBonusType)
					if (type_string in banana):
						if pPlot.isFlatlands() and pPlot.getFeatureType() == gc.getInfoTypeForString("FEATURE_JUNGLE"):
							distVal = bananaDist.getHeight(x,y)
							if (distVal >= bananaLevelBottom and distVal < bananaLevelTop):
								map.plot(x,y).setBonusType(iBonusType)
					if (type_string in stone):
						if pPlot.getTerrainType() == gc.getInfoTypeForString("TERRAIN_DESERT") and pPlot.getFeatureType() == -1:
							distVal = stoneDist.getHeight(x,y)
							if (distVal >= stoneLevelBottom and distVal < stoneLevelTop):
								map.plot(x,y).setBonusType(iBonusType)
					if (type_string in marble):
						if pPlot.getTerrainType() == gc.getInfoTypeForString("TERRAIN_TUNDRA") or pPlot.getTerrainType() == gc.getInfoTypeForString("TERRAIN_SNOW"):
							distVal = marbleDist.getHeight(x,y)
							if (distVal >= marbleLevelBottom and distVal < marbleLevelTop):
								pPlot.setFeatureType(-1,1)
								map.plot(x,y).setBonusType(iBonusType)

		# Generate special featured resources	
		if (type_string in bonusCommon or type_string in bonusRare):
			print("Special resource implementation:", type_string)

			# Default limits
			maxCount = nrPlayers + map.getWorldSize()
			maxNrClusters = 1
			if map.getWorldSize() >= 2:
				maxNrClusters += 1
			if map.getWorldSize() >= 4:
				maxNrClusters += 1
			maxClusterCount = 3
			clusterProb = 20
			plotProb = 70

			# Resource specific limits
			if (type_string in metal):
				clusterProb = 40
			if (type_string in bonusRare):
				print("Rare resource")
				maxCount = (nrPlayers + 1) / 2
				maxNrClusters = 1
				clusterProb = 60
				maxClusterCount = 6
			if (type_string in metal):
				maxNrClusters += 1
				maxCount += 1

			# Place resource
			count = 0
			nrClusters = 0
			for count in range(maxCount):
				index = dice.get(iW * iH, "Dice roll - CA test PYTHON")
				done = 0
				loops = 0
				while done == 0 and loops < 1000:
					if map.plotByIndex(index).getBonusType(-1) == -1:
						if (type_string in metal):
							if map.plotByIndex(index).isHills():
								map.plotByIndex(index).setBonusType(iBonusType)
								done = 1
						else:
							if map.plotByIndex(index).canHaveBonus(iBonusType, true):
								map.plotByIndex(index).setBonusType(iBonusType)
								done = 1
					if done == 0:
						index = dice.get(iW * iH, "Dice roll - CA test PYTHON")
						loops = loops + 1
				if loops >= 1000:
					print("Could not find spot for resource")
					return None
				xPos = map.plotByIndex(index).getX()
				yPos = map.plotByIndex(index).getY()
				print("Special resource at:", xPos, yPos)
				xPos = min(iW - 3, xPos - 1)
				yPos = min(iH - 3, yPos - 1)
				xPos = max(xPos, 0)
				yPos = max(yPos, 0)
				
				# Fill in possible cluster
				clusterCount = 1 # middle plot is already assigned
				diceRoll = dice.get(100, "Dice roll")
				if diceRoll <= clusterProb and nrClusters < maxNrClusters:
					print("Clustering special resource")
					nrClusters = nrClusters + 1
					for y in range(yPos, yPos + 3):
						for x in range(xPos, xPos + 3):
							pPlot = map.plot(x,y)
							diceRoll = dice.get(100, "Dice roll - CA test PYTHON")
							if pPlot.getBonusType(-1) == -1:
								if (type_string in metal):
									if pPlot.isHills():
										if diceRoll <= plotProb and clusterCount < maxClusterCount:
											pPlot.setBonusType(iBonusType)
											clusterCount = clusterCount + 1
								elif (type_string in needsChopping):
									changedForest = 0
									resourceSet = 0
									if pPlot.getFeatureType() == gc.getInfoTypeForString("FEATURE_FOREST"):
										pPlot.setFeatureType(-1,1)
										print("Chopping forest at", x, y)
										changedForest = 1
									if pPlot.getFeatureType() == gc.getInfoTypeForString("FEATURE_JUNGLE"):
										pPlot.setFeatureType(-1,1)
										print("Chopping jungle at", x, y)
										changedForest = 2
									if pPlot.canHaveBonus(iBonusType, true):
										if diceRoll <= plotProb and clusterCount < maxClusterCount:
											pPlot.setBonusType(iBonusType)
											clusterCount = clusterCount + 1
											resourceSet = 1
									if changedForest == 1 and resourceSet == 0: # Put forest back
										variety = 1
										if pPlot.getTerrainType() == gc.getInfoTypeForString("TERRAIN_TUNDRA"):
											variety = 2
										pPlot.setFeatureType(gc.getInfoTypeForString("FEATURE_FOREST"), variety)
									if changedForest == 2 and resourceSet == 0: # Put jungle back
										pPlot.setFeatureType(gc.getInfoTypeForString("FEATURE_JUNGLE"), 1)
											
								else:
									if pPlot.canHaveBonus(iBonusType, true):
										if diceRoll <= plotProb and clusterCount < maxClusterCount:
											pPlot.setBonusType(iBonusType)
											clusterCount = clusterCount + 1
				

		# Generate standard resources
		if (not type_string in bonusRare and not type_string in bonusFractal and not type_string in bonusCommon):
			print("Standard resource implementation:", type_string)
			CyPythonMgr().allowDefaultImpl() # Let C handle this bonus in the default way.

		return None
					
def getDescription():
	return WildErebus().getDescription()

#MapOptions
def isClimateMap():
	return WildErebus().isClimateMap()

def isSeaLevelMap():
	return WildErebus().isSeaLevelMap()

def getNumCustomMapOptions():
	return WildErebus().getNumCustomMapOptions()

def getNumHiddenCustomMapOptions():
	return WildErebus().getNumHiddenCustomMapOptions()

def getCustomMapOptionName(argsList):
	return WildErebus().getCustomMapOptionName(argsList)
	
def getNumCustomMapOptionValues(argsList):
	return WildErebus().getNumCustomMapOptionValues(argsList)
	
def getCustomMapOptionDescAt(argsList):
	return WildErebus().getCustomMapOptionDescAt(argsList)
	
def getCustomMapOptionDefault(argsList):
	return WildErebus().getCustomMapOptionDefault(argsList)

def isRandomCustomMapOption(argsList):
	return WildErebus().isRandomCustomMapOption(argsList)

#general functions
def getWrapX():
	return WildErebus().getWrapX()
	
def getWrapY():
	return WildErebus().getWrapY()
	
def getTopLatitude():
	return WildErebus().getTopLatitude()
	
def getBottomLatitude():
	return WildErebus().getBottomLatitude()

def generatePlotTypes():
	return WildErebus().generatePlotTypes()
	
def generateTerrainTypes():
	return WildErebus().generateTerrainTypes()
	
def addFeatures():
	return WildErebus().addFeatures()
	
def addBonusType(argsList):	
	return WildErebus().addBonusType(argsList)	
