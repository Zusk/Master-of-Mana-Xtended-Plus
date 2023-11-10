import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#PIRATES

def DoSpawnPirates(argsList):
	'Called at the beginning of the end of each turn'
	iGameTurn = argsList[0]

	iPlayer = gc.getPIRATES_PLAYER()
	pPlayer = gc.getPlayer(iPlayer)
	py = PyPlayer(iPlayer)

#Spawn Critters in Deep Oceans (1 for 100 mapplots should be enough)

	if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_SEA_SERPENT'))*100<CyMap().numPlots():
		iMapPlots=CyMap().numPlots()
		for i in range(0,20,1):
			bvalid=false
			iRnd=CyGame().getSorenRandNum(iMapPlots, "Arrrgh")
			pPlot = CyMap().plotByIndex(iRnd)
			if pPlot.isWater():
				if pPlot.getNumUnits()==0:
					if pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_OCEAN_DEEP'):
						bvalid=true

			if bvalid:
				newUnit= pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SEA_SERPENT'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				break


	if CyGame().countKnownTechNumTeams(gc.getInfoTypeForString('TECH_OPTICS'))==0:
		return

	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_PIRATES):
		return

	iPirateShip1 = gc.getInfoTypeForString('UNIT_PIRATESHIP1')
	iPirate = gc.getInfoTypeForString('UNIT_BOARDING_PARTY')

	iPiratesNeeded = CyGame().getNumCities()/10
	if iPiratesNeeded>5:
		iPiratesNeeded=5+CyGame().getNumCities()/20

	if pPlayer.getNumUnits()<iPiratesNeeded:
		iMapPlots=CyMap().numPlots()
		for i in range(0,20,1):
			bvalid=false
			iRnd=CyGame().getSorenRandNum(iMapPlots, "Arrrgh")
			pPlot = CyMap().plotByIndex(iRnd)
			if pPlot.isWater() and pPlot.getFeatureType()==-1 and pPlot.getImprovementType()==-1:
				if pPlot.getNumUnits()==0:
					if not pPlot.isVisibleToCivTeam():
						bvalid=true

			if pPlayer.getNumUnits()<iPiratesNeeded:
				if bvalid:
					newUnit= pPlayer.initUnit(iPirateShip1, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			else:
				break

	iPlunderTime=10+CyGame().getNumCities()-pPlayer.getNumUnits()
	if CyGame().countKnownTechNumTeams(gc.getInfoTypeForString('TECH_TRADE'))==0:
		iPlunderTime*=2

	iPlunderTime = 1


	#Naval Invasion
	if CyGame().getSorenRandNum(100, "Plunder")<iPlunderTime:
		#choose Target City
		pTargetCity=-1
		iHack=0
		pTarget=-1
		while iHack<100:
			iTarget=CyGame().getSorenRandNum(gc.getMAX_CIV_PLAYERS(), "pickPlayer")
			pTarget = gc.getPlayer(iTarget)
			if pTarget.isHuman:
				py = PyPlayer(iTarget)
				for pyCity in PyPlayer(iTarget).getCityList():
					pCity = pyCity.GetCy()
					if pCity.isCoastal(20):
						pTargetCity=pCity
						iHack=200
			iHack+=1

		#Choose Plot to Spawn Pirates
		if pTargetCity!=-1:

			iMapPlots=CyMap().numPlots()
			iHack=0
			bvalid=false
			pPlot=-1
			while iHack<100:
				iRnd=CyGame().getSorenRandNum(iMapPlots, "Pirates")
				pPlot = CyMap().plotByIndex(iRnd)
				if pPlot.isWater():
					if pPlot.getNumUnits()==0:
						if not pPlot.isVisibleToCivTeam():
							if pTargetCity.plot().isAdjacentToArea(pPlot.area()):
								bvalid=true
								iHack=100
				iHack+=1

			if bvalid:
				if CyGame().getSorenRandNum(30, "Pirates")==0:
					iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_PIRATES_INVASION')
					pTarget.trigger(iEvent)

				newUnit= pPlayer.initUnit(iPirateShip1, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				Cargo= pPlayer.initUnit(iPirate, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				Cargo.getGroup().setTransportUnit(newUnit)
				Cargo= pPlayer.initUnit(iPirate, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				Cargo.getGroup().setTransportUnit(newUnit)
				Cargo= pPlayer.initUnit(iPirate, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				Cargo.getGroup().setTransportUnit(newUnit)
				Cargo= pPlayer.initUnit(iPirate, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				Cargo.getGroup().setTransportUnit(newUnit)

#END PIRATES


