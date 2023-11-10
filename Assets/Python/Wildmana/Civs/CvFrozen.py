import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#FROZEN

def onProjectFrozen(argsList):
		'Project Completed'
		pCity, iProjectType = argsList
		iPlayer = pCity.getOwner()
		pPlayer = gc.getPlayer(iPlayer)
		pPlot = pCity.plot()
		iFrozen = gc.getInfoTypeForString('CIVILIZATION_FROZEN')		#Changed in Frozen: TC01

		if iProjectType == gc.getInfoTypeForString('PROJECT_ASCENSION'):
#	Changed in Frozen: TC01
#	If the module is not loaded, things happen as they normally do. But if the module is loaded, then it will check to see who built it and award the appropriate Ascended unit.
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
				pPlayer.initUnit(gc.getInfoTypeForString('UNIT_AURIC_ASCENDED'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			elif pPlayer.getCivilizationType() == iFrozen:
				pPlayer.initUnit(gc.getInfoTypeForString('UNIT_TARANIS_ASCENDED'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				for iPlot in range(CyMap().numPlots()):
					pTaranisPlot = CyMap().plotByIndex(iPlot)
					for iUnit in range(pTaranisPlot.getNumUnits()):
						pTaranis = pTaranisPlot.getUnit(iUnit)
						if pTaranis.getUnitType() == gc.getInfoTypeForString('UNIT_TARANIS'):
							pTaranis.kill(false, -1)
#End of Frozen
			if pPlayer.isHuman():
				t = "TROPHY_FEAT_ASCENSION"
				if not CyGame().isHasTrophy(t):
					CyGame().changeTrophyValue(t, 1)
			if not CyGame().getWBMapScript():
				iBestPlayer = -1
				iBestValue = 0
				for iLoopPlayer in range(gc.getMAX_PLAYERS()):
					pLoopPlayer = gc.getPlayer(iLoopPlayer)
					if pLoopPlayer.isAlive():
						if not pLoopPlayer.isBarbarian():
							if pLoopPlayer.getTeam() != pPlayer.getTeam():
								iValue = CyGame().getSorenRandNum(500, "Ascension")
								if pLoopPlayer.isHuman():
									iValue += 2000
								iValue += (20 - CyGame().getPlayerRank(iLoopPlayer)) * 50
								if iValue > iBestValue:
									iBestValue = iValue
									iBestPlayer = iLoopPlayer
				if iBestPlayer != -1:
					pBestPlayer = gc.getPlayer(iBestPlayer)
					pBestCity = pBestPlayer.getCapitalCity()
					if pBestPlayer.isHuman():
						iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_GODSLAYER')
						triggerData = gc.getPlayer(iBestPlayer).initTriggeredData(iEvent, true, -1, pBestCity.getX(), pBestCity.getY(), iBestPlayer, -1, -1, -1, -1, -1)
					else:
						containerUnit = -1
						pPlot = pBestCity.plot()
						for i in range(pPlot.getNumUnits()):
							if pPlot.getUnit(i).getUnitType() == gc.getInfoTypeForString('EQUIPMENT_CONTAINER'):
								containerUnit = pPlot.getUnit(i)
						if containerUnit == -1:
							containerUnit = gc.getPlayer(gc.getBARBARIAN_PLAYER()).initUnit(gc.getInfoTypeForString('EQUIPMENT_CONTAINER'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						containerUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GODSLAYER'), True)

#	Added in Frozen: TC01
#	Will spawn the Frozen when completed. No if statement is needed, since this is only triggered when a project in the module is built.
		if iProjectType == gc.getInfoTypeForString('PROJECT_LIBERATION'):
			iFrozenPlayer = cf.getOpenPlayer()
			pBestPlot = -1
			iBestPlot = -1
			for i in range (CyMap().numPlots()):
				pPlot = CyMap().plotByIndex(i)
				iPlot = -1
				if pPlot.isWater() == False:
					if pPlot.getNumUnits() == 0:
						if pPlot.isCity() == False:
							if pPlot.isImpassable() == False:
								iPlot = CyGame().getSorenRandNum(500, "Place Taranis")
								iPlot = iPlot + (pPlot.area().getNumTiles() * 2)
								iPlot = iPlot + (pPlot.area().getNumUnownedTiles() * 10)
								if pPlot.isOwned() == False:
									iPlot = iPlot + 500
								if pPlot.getOwner() == iPlayer:
									iPlot = iPlot + 200
				if iPlot > iBestPlot:
					iBestPlot = iPlot
					pBestPlot = pPlot
			if (iFrozenPlayer != -1 and pBestPlot != -1):
				CyGame().addPlayerAdvanced(iFrozenPlayer, -1, gc.getInfoTypeForString('LEADER_TARANIS'), gc.getInfoTypeForString('CIVILIZATION_FROZEN'))
				iFounderTeam = gc.getPlayer(iPlayer).getTeam()
				eFounderTeam = gc.getTeam(gc.getPlayer(iPlayer).getTeam())
				iFrozenTeam = gc.getPlayer(iFrozenPlayer).getTeam()
				eFrozenTeam = gc.getTeam(iFrozenTeam)
				for iTech in range(gc.getNumTechInfos()):
					if eFounderTeam.isHasTech(iTech):
						eFrozenTeam.setHasTech(iTech, true, iFrozenPlayer, true, false)
				eFounderTeam.signOpenBorders(iFrozenTeam)
				eFrozenTeam.signOpenBorders(iFounderTeam)
				pFrozenPlayer = gc.getPlayer(iFrozenPlayer)
				pFrozenPlayer.AI_changeAttitudeExtra(iPlayer,4)

				gc.getTeam(gc.getPlayer(gc.getBARBARIAN_PLAYER()).getTeam()).declareWar(iFrozenTeam, false, WarPlanTypes.WARPLAN_TOTAL)
				gc.getTeam(gc.getPlayer(gc.getANIMAL_PLAYER()).getTeam()).declareWar(iFrozenTeam, false, WarPlanTypes.WARPLAN_TOTAL)
				gc.getTeam(gc.getPlayer(gc.getWILDMANA_PLAYER()).getTeam()).declareWar(iFrozenTeam, false, WarPlanTypes.WARPLAN_TOTAL)
				gc.getTeam(gc.getPlayer(gc.getPIRATES_PLAYER()).getTeam()).declareWar(iFrozenTeam, false, WarPlanTypes.WARPLAN_TOTAL)

				eFrozenTeam.setHasTech(gc.getInfoTypeForString('TECH_WINTER_TECH'), true, iFrozenPlayer, true, false)
				newUnit1 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_ICE_GOLEM'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit1.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
				newUnit2 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_ICE_GOLEM'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
				newUnit3 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_CHAMPION'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit3.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
				newUnit4 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_CHAMPION'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit4.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
				newUnit5 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_WORKER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit5.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FROSTLING'), true)
				newUnit6 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_ADEPT'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit6.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
				newUnit7 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_FROZEN_SOUL'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit8 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_FROZEN_SOUL'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit9 = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_FROZEN_SOUL'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				newUnit = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_SETTLER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
#					newUnit10.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), true)
				newUnit = pFrozenPlayer.initUnit(gc.getInfoTypeForString('UNIT_SETTLER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
#					newUnit11.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), true)
				if gc.getPlayer(iPlayer).isHuman():
					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON)
					popupInfo.setText(CyTranslator().getText("TXT_KEY_POPUP_CONTROL_FROZEN",()))
					popupInfo.setData1(iPlayer)
					popupInfo.setData2(iFrozenPlayer)
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_YES", ()), "")
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_NO", ()), "")
					popupInfo.setOnClickedPythonCallback("reassignPlayer")
					popupInfo.addPopup(iPlayer)
#End of Frozen

def onWinteredKilled(argsList):
	'Unit Killed'
	unit, iAttacker = argsList
	iPlayer = unit.getOwner()
	player = PyPlayer(iPlayer)
	attacker = PyPlayer(iAttacker)
	pPlayer = gc.getPlayer(iPlayer)
	iWintered = gc.getInfoTypeForString('PROMOTION_WINTERED')		#Added in Frozen: TC01

#	Added in Frozen: TC01
#	If units with the Winterborn or Wintered promotion die with the module loaded, the Frozen get Frozen Souls.

#	CyInterface().addImmediateMessage('Unit has been Killed', "AS2D_NEW_ERA")

	if (unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WINTERBORN')) or unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WINTERED'))):
		if not unit.getOwner() == gc.getInfoTypeForString('CIVILIZATION_FROZEN'):
			cf.giftUnit(gc.getInfoTypeForString('UNIT_FROZEN_SOUL'), gc.getInfoTypeForString('CIVILIZATION_FROZEN'), 0, unit.plot(), unit.getOwner())
			cf.giftUnit(gc.getInfoTypeForString('UNIT_FROZEN_SOUL'), gc.getInfoTypeForString('CIVILIZATION_FROZEN'), 0, unit.plot(), unit.getOwner())
#End of Frozen

def onFrozenCityBuilt(argsList):
	'City Built'
	city = argsList[0]
	pPlot = city.plot()
	pPlayer = gc.getPlayer(city.getOwner())
	iFrozen = gc.getInfoTypeForString('CIVILIZATION_FROZEN')		#Added in Frozen: TC01

#	Added in Frozen: TC01
#	Builds buildings in Frozen cities if the Frozen module is loaded. Triggers Temple of the Hand custom function.
	if pPlayer.getCivilizationType() == iFrozen:
		city.setPopulation(2)
		city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ELDER_COUNCIL'), 1)
		city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_HAND'), 1)
		city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_OBSIDIAN_GATE'), 1)
		city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_FORGE'), 1)
		city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_MAGE_GUILD'), 1)
		city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_FROZEN_SOULS'), 1)

		cf.doTempleWhiteHand(city)
#End of Frozen

def onFrozenCityAcquired(argsList):
		'City Acquired'
		iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
		pPlayer = gc.getPlayer(iNewOwner)
		iFrozen = gc.getInfoTypeForString('CIVILIZATION_FROZEN')		#Added in Frozen: TC01

		if (gc.getPlayer(iPreviousOwner).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL') or gc.getPlayer(iPreviousOwner).getCivilizationType() == iFrozen):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_OBSIDIAN_GATE'), 0)
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_FROZEN_SOULS'), 0)
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_FROZEN'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS'), 0)
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ELDER_COUNCIL'), 1)
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_HAND'), 1)
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_OBSIDIAN_GATE'), 1)
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_FORGE'), 1)
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_MAGE_GUILD'), 1)
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_FROZEN_SOULS'), 1)
			cf.doTempleWhiteHand(pCity)

def onFrozenCityDoTurn(argsList):
		'City Production'
		pCity = argsList[0]
		iPlayer = argsList[1]
		pPlot = pCity.plot()
		iPlayer = pCity.getOwner()
		pPlayer = gc.getPlayer(iPlayer)

#	Added in Frozen: TC01
#	If the module is loaded, has a chance of spawning Tar Demons in Frozen cities. Chance varies based on how many have already been spawned. To change chances, adjust the modifiers labeled
#with comments below. It also removes all religions from Frozen cities every turn.
		iFrozen = gc.getInfoTypeForString('CIVILIZATION_FROZEN')
		pPlayer = gc.getPlayer(pCity.getOwner())
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_FROZEN'):
			for iTarget in range(gc.getNumReligionInfos()):
				if not pCity.isHolyCityByType(iTarget):
					for i in range(gc.getNumBuildingInfos()):
						if gc.getBuildingInfo(i).getPrereqReligion() == iTarget:
							pCity.setNumRealBuilding(i, 0)

#End of Frozen


