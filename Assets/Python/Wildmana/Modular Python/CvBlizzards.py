import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#Blizzards Modcomp by TC01
#	doBlizzardTurn is called every game turn and moves, kills, or spawns blizzards randomly
#	moveBlizzard moves a blizzard to a new plot depending on direction
#	canBlizzard checks if a plot can have a blizzard move onto it or if a blizzard can be spawned there
#	doBlizzard applies the effects of a blizzard

#	All of these functions can be modified to add your own effects. I suggest not modifying doBlizzardTurn or moveBlizzard, but instead modifying the constants defined at the top of that 
#function. canBlizzard can be altered to change whether a blizzard can move onto or spawn on a plot. doBlizzard can be modified to cause different features or terrains to appear in different
#conditions.

#	 You can use this modcomp with any mod. Follow the instructions in Merging Guide.txt or in the forum thread to merge it. Be aware that it will be included in the next version of my Frozen
#civilization, so you will not have to merge it there.

def spellCallBlizzard(caster):
	iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
	iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
	iX = caster.getX()
	iY = caster.getY()
	pBlizPlot = -1
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if pPlot.getFeatureType() == iBlizzard:
				pBlizPlot = pPlot
	if pBlizPlot != -1:
		pBlizPlot.setFeatureType(-1, -1)
	pPlot = caster.plot()
	pPlot.setFeatureType(iBlizzard, 0)
	Blizzards.doBlizzard(pPlot)
	
def DoTheBlizzard(argsList):

	iIllians = gc.getInfoTypeForString('CIVILIZATION_ILLIANS')
	iFrozen = gc.getInfoTypeForString('CIVILIZATION_FROZEN')

	iAreFrozenPeopleHere=0
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)	
		if pPlayer.getCivilizationType() == iIllians or pPlayer.getCivilizationType() == iFrozen:
			iAreFrozenPeopleHere += 1
			
	if iAreFrozenPeopleHere == 0:
		return

	Blizzards().doBlizzardTurn()

class Blizzards:

	def doBlizzardTurn(self):
		#Constants:
		self.iBlizzardDriftChanceEast = 35		#Chance a blizzard drifts east
		self.iBlizzardDriftChanceWest = 35		#Chance a blizzard drifts west
		self.iBlizzardDriftChanceNorth = 35		#Chance a blizzard drifts north
		self.iBlizzardDriftChanceSouth = 35		#Chance a blizzard drifts south
		self.iBlizzardKillChance = 25			#Chance a blizzard expires
		self.iBlizzardChance = 5				#Chance a blizzard spawns
		self.iBlizzardChancePlus = 5			#Chance a blizzard spawns on a snow plot a blizzard passes by
		
		iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
		iIllians = gc.getInfoTypeForString('CIVILIZATION_ILLIANS')
		iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
		
		for i in range(CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getFeatureType() == iBlizzard:
				iBlizzardDirectionRand = CyGame().getSorenRandNum(100, "Blizzard")
				iBlizzardKillRand = CyGame().getSorenRandNum(100, "Kill Blizzard")

				#Moves a blizzard
				if iBlizzardDirectionRand <= 25:
					self.moveBlizzard(pPlot, 0)
				if 26 <= iBlizzardDirectionRand <= 50:
					self.moveBlizzard(pPlot, 1)
				if 51 <= iBlizzardDirectionRand <= 75:
					self.moveBlizzard(pPlot, 2)
				if 76 <= iBlizzardDirectionRand:
					self.moveBlizzard(pPlot, 3)
				
				#Kills a blizzard
				if iBlizzardKillRand <= self.iBlizzardKillChance:
					pPlot.setFeatureType(-1,-1)

			#Creates a blizzard
			if pPlot.getTerrainType() == iSnow:
				if pPlot.getFeatureType() == -1:
					if pPlot.getOwner() != -1:
						if (gc.getPlayer(pPlot.getOwner()).getCivilizationType() == iIllians):
							if self.canBlizzard(pPlot):
								if pPlot.getScriptData() == "Blizzard":
									if CyGame().getSorenRandNum(100, "Blizzard") < self.iBlizzardChancePlus:
										pPlot.setFeatureType(iBlizzard,0)
										pPlot.setScriptData("")
										self.doBlizzard(pPlot)
								else:
									if CyGame().getSorenRandNum(100, "Blizzard") < self.iBlizzardChance:
										pPlot.setFeatureType(iBlizzard,0)
										self.doBlizzard(pPlot)

	def moveBlizzard(self, pPlot, iDirection):
		iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
		iRnd = CyGame().getSorenRandNum(100, "Blizzards")
		if iDirection == 0:
			if iRnd <= self.iBlizzardDriftChanceEast:
				newPlot = CyMap().plot(pPlot.getX() + 1, pPlot.getY() + 1)
			if (iRnd > self.iBlizzardDriftChanceEast and iRnd < (100 - self.iBlizzardDriftChanceEast)):
				newPlot = CyMap().plot(pPlot.getX() + 1, pPlot.getY())
			if iRnd >= 100 - self.iBlizzardDriftChanceEast:
				newPlot = CyMap().plot(pPlot.getX() + 1, pPlot.getY() - 1)
		if iDirection == 1:
			if iRnd <= self.iBlizzardDriftChanceWest:
				newPlot = CyMap().plot(pPlot.getX() - 1, pPlot.getY() + 1)
			if (iRnd > self.iBlizzardDriftChanceWest and iRnd < (100 - self.iBlizzardDriftChanceWest)):
				newPlot = CyMap().plot(pPlot.getX() - 1, pPlot.getY())
			if iRnd >= 100 - self.iBlizzardDriftChanceWest:
				newPlot = CyMap().plot(pPlot.getX() - 1, pPlot.getY() - 1)
		if iDirection == 2:
			if iRnd <= self.iBlizzardDriftChanceNorth:
				newPlot = CyMap().plot(pPlot.getX() - 1, pPlot.getY() + 1)
			if (iRnd > self.iBlizzardDriftChanceNorth and iRnd < (100 - self.iBlizzardDriftChanceNorth)):
				newPlot = CyMap().plot(pPlot.getX(), pPlot.getY() + 1)
			if iRnd >= 100 - self.iBlizzardDriftChanceNorth:
				newPlot = CyMap().plot(pPlot.getX() + 1, pPlot.getY() + 1)
		if iDirection == 3:
			if iRnd <= self.iBlizzardDriftChanceSouth:
				newPlot = CyMap().plot(pPlot.getX() - 1, pPlot.getY() - 1)
			if (iRnd > self.iBlizzardDriftChanceSouth and iRnd < (100 - self.iBlizzardDriftChanceSouth)):
				newPlot = CyMap().plot(pPlot.getX(), pPlot.getY() - 1)
			if iRnd >= 100 - self.iBlizzardDriftChanceSouth:
				newPlot = CyMap().plot(pPlot.getX() + 1, pPlot.getY() - 1)
		
		if self.canBlizzard(newPlot) and newPlot.isNone() == False:
			newPlot.setFeatureType(iBlizzard,0)
			self.doBlizzard(newPlot)
			pPlot.setFeatureType(-1,-1)

	def canBlizzard(self, pPlot):
		if pPlot.isPeak():
			return False
		if pPlot.getFeatureType() != -1:
			return False
		if pPlot.getOwner()!=-1:
			pPlayer = gc.getPlayer(pPlot.getOwner())			
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
				return True
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_FROZEN'):
				return True				
		return False			

	def doBlizzard(self, pPlot):
		iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
		iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
		iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
		iFloodPlains = gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')
		iForest = gc.getInfoTypeForString('FEATURE_FOREST')
		iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
		iScrub = gc.getInfoTypeForString('FEATURE_SCRUB')
		iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
		bValid = False
		iX = pPlot.getX()
		iY = pPlot.getY()
		for iiX in range(iX-1, iX+2, 1):
			for iiY in range(iY-1, iY+2, 1):
				targetPlot = CyMap().plot(iiX,iiY)
				if targetPlot.isNone() == False:
					if not targetPlot.isWater():
					
						#Snow gets an increased chance to become a blizzard
						if targetPlot.getTerrainType() == iSnow:
							targetPlot.setScriptData("Blizzard")
						
						#Tundra becomes permanent snow terrain automatically
						if targetPlot.getTerrainType() == iTundra:
							targetPlot.setTerrainType(iSnow,true,true)
							
						#Plots will randomly be set to either permanent or temporary snow terrain
						if (targetPlot.getTerrainType() != iSnow and targetPlot.getTerrainType != iTundra):
							iPermanentRand = CyGame().getSorenRandNum(100, "Temp Terrain")
							if iPermanentRand <= 50:
								targetPlot.setTerrainType(iSnow,true,true)
							else:
								targetPlot.setTempTerrainType(iSnow, CyGame().getSorenRandNum(5, "Bob") + 10)
					
						#Specified features (and smoke) are destroyed/terraformd into something else
						if targetPlot.getImprovementType() == iSmoke:
							targetPlot.setImprovementType(-1)
						if targetPlot.getFeatureType() == iForest:
							targetPlot.setFeatureType(iForest, 2)
						if targetPlot.getFeatureType() == iJungle:
							targetPlot.setFeatureType(iForest, 2)
						if targetPlot.getFeatureType() == iFlames:
							targetPlot.setFeatureType(-1, -1)
						if targetPlot.getFeatureType() == iFloodPlains:
							targetPlot.setFeatureType(-1, -1)
						if targetPlot.getFeatureType() == iScrub:
							targetPlot.setFeatureType(-1, -1)
												
#END Blizzards Modcomp by TC01


		