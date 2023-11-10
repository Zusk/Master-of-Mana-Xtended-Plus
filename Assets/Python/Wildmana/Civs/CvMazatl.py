import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

iLizCiv = gc.getInfoTypeForString('CIVILIZATION_MAZATL')
iVolcano = gc.getInfoTypeForString('IMPROVEMENT_VOLCANO')
iLizJungle = gc.getInfoTypeForString('FEATURE_LIZ_JUNGLE')
iAncientForest = gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')
iForest = gc.getInfoTypeForString('FEATURE_FOREST')
iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
iGrassland = gc.getInfoTypeForString('TERRAIN_GRASS')
iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
iWetland = gc.getInfoTypeForString('TERRAIN_SWAMP')
iWetlandChance = 5
iPlainsChance = 2
iJungleChance = 10
iSwamp = gc.getInfoTypeForString('IMPROVEMENT_SWAMP')
iSwampChance = 25
iRoad = gc.getInfoTypeForString('ROUTE_ROAD')
iTrail = gc.getInfoTypeForString('ROUTE_TRAIL')

# Mazatl
# LordShaggynator: Cleaned up the code and did some small additional optimizations
def doMazatlTurn(argsList):
	'Called at the beginning of the end of each turn'
	iGameTurn = argsList[0]

	# Set reference to mazatl player
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.getCivilizationType()==iLizCiv:
			break;
	else:
		return

	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)

		if not (pPlot.isWater() or pPlot.isPeak() or pPlot.getImprovementType() == iVolcano):

			#### Lizard Terrain Section

			iBonus = pPlot.getBonusType(-1)
			iFeature = pPlot.getFeatureType()
			iImprovement = pPlot.getImprovementType()
			iTerrain = pPlot.getTerrainType()

			if pPlot.isOwned() and gc.getPlayer(pPlot.getOwner()).getCivilizationType() == iLizCiv:
				if pPlot.getRouteType() == iRoad:
					if CyGame().getSorenRandNum(100, "Trail") < 20 :
						pPlot.setRouteType(iTrail)
				if iFeature == iJungle:
					pPlot.setFeatureType(iLizJungle, 0)

				iMod=100
				for i in range (DirectionTypes.NUM_DIRECTION_TYPES):
					pLoopPlot=pPlot
					if i==0:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTH)
					if i==1:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHEAST)
					if i==2:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_EAST)
					if i==3:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHEAST)
					if i==4:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTH)
					if i==5:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHWEST)
					if i==6:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_WEST)
					if i==7:
						pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHWEST)

					if not pLoopPlot.isNone():
						if pLoopPlot!=pPlot:
							if pLoopPlot.getTerrainType()==iWetland:
								iMod+=20
							elif pLoopPlot.getTerrainType()==iGrassland:
								iMod+=10

							if pLoopPlot.getFeatureType()==iJungle or pLoopPlot.getFeatureType()==iLizJungle:
								iMod+=20
							if pLoopPlot.getImprovementType()==iSwamp:
								iMod+=100

				if pPlot.isHills() == False:
					if iTerrain == iGrassland:
						iChance = (iWetlandChance*iMod)/100
						if pPlot.isRiverSide() == True:
							iChance = (iWetlandChance*5*iMod)/100
						if CyGame().getSorenRandNum(1000, "Wetlands") < iChance *5:
							pPlot.setTerrainType(iWetland, True, True)
						continue

					if iTerrain == iPlains:
						iChance = (iPlainsChance*iMod)/100
						if CyGame().getSorenRandNum(1000, "Plains") < iChance *5:
							pPlot.setTerrainType(iGrassland, True, True)
						continue

				if iTerrain == iWetland :
					if iImprovement == -1 and iBonus == -1 and pPlot.isCity  () == False and pPlot.getNumUnits()==0:
						iChance = (iSwampChance*iMod)/100
						if pPlayer.isHuman() == False:
							iChance = iChance * 2
						if CyGame().getSorenRandNum(1000, "Swamp") < iChance :
							pPlot.setImprovementType(iSwamp)

					if iFeature == iForest or iFeature == iAncientForest:
						iChance = (iJungleChance * 5 *iMod)/100
						if CyGame().getSorenRandNum(1000, "Jungle") < iChance :
							pPlot.setFeatureType(iJungle, 0)

					if iFeature == -1:
						iChance = (iJungleChance *iMod)/100
						if CyGame().getSorenRandNum(1000, "Jungle") < iChance :
							pPlot.setFeatureType(iJungle, 0)

			else:
				if pPlot.getFeatureType() == iLizJungle:
					pPlot.setFeatureType(iJungle, 0)
				if pPlot.isOwned() and pPlot.getRouteType() == iTrail:
					if CyGame().getSorenRandNum(100, "Trail") < 10 :
						pPlot.setRouteType(iRoad)

			if (iImprovement == iSwamp) and (pPlot.getTerrainType() != iWetland) :
				pPlot.setImprovementType(-1)

def MazatlUnitCreated(argsList):
	'Unit Completed'

	unit = argsList[0]
	pPlayer = gc.getPlayer(unit.getOwner())

	iMazatl = gc.getInfoTypeForString('CIVILIZATION_MAZATL')

	if iMazatl==-1 or pPlayer.getCivilizationType()!=iMazatl:
		return

	iUnitType=unit.getUnitType()

	if iUnitType==gc.getInfoTypeForString('UNIT_LIZARD_WORKER_MAZATL'):
		if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_SANITATION')):
			unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEEP_SHAPER'),true)
