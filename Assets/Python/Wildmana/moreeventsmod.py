import CvUtil
from CvPythonExtensions import *
import CustomFunctions
import PyHelpers

cf = CustomFunctions.CustomFunctions()
gc = CyGlobalContext()
localText = CyTranslator()
PyPlayer = PyHelpers.PyPlayer

#More Events Mod Begins#

def onMoveBrokenSepulcher(pCaster, pPlot):
	if CyGame().getScenarioCounter()==6:
		if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GELA')):
			pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GELA'), False)
			pPlayer = gc.getPlayer(pCaster.getOwner())
			for pyCity in PyPlayer(pCaster.getOwner()).getCityList() :
				pCity = pyCity.GetCy()
				if CyGame().getSorenRandNum(100, "Mane") <= 60:
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				if pCity.getPopulation() > 2:
					pCity.changePopulation(-2)
			CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_GELA_BROKEN", ()),'',1,'Art/Interface/Buttons/Improvements/Broken Sepulcher.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)


def onMoveMaelstrom(pCaster, pPlot):
	if CyGame().getScenarioCounter()==3:
		if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GELA')):
			pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GELA'), False)
			pPlayer = gc.getPlayer(pCaster.getOwner())
			iProm = gc.getInfoTypeForString('PROMOTION_WATER_WALKING')
			if  pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS'):
				newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_STYGIAN_GUARD'), pPlot.getX()+1, pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit1.setHasPromotion(iProm, True)
				newUnit2 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_STYGIAN_GUARD'), pPlot.getX()+1, pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit2.setHasPromotion(iProm, True)
				newUnit3 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_STYGIAN_GUARD'), pPlot.getX()+1, pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit3.setHasPromotion(iProm, True)
				newUnit4 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_STYGIAN_GUARD'), pPlot.getX()+1, pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit4.setHasPromotion(iProm, True)
				newUnit5 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DISCIPLE_OCTOPUS_OVERLORDS'), pPlot.getX()+1, pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit5.setHasPromotion(iProm, True)
				newUnit5.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HERO'), True)
				CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MAELSTROM_GELA_1",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),pCaster.getX(),pCaster.getY(),True,True)
				pCaster.kill(True, PlayerTypes.NO_PLAYER)
			else:
				iStygianChance = 300
				bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
				for i in range (CyMap().numPlots()):
					pPlot = CyMap().plotByIndex(i)
					if  pPlot.isWater():
						if pPlot.getNumUnits() == 0:
							if CyGame().getSorenRandNum(10000, "Stygian") <= iStygianChance:
								newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_STYGIAN_GUARD'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								newUnit.setUnitAIType(gc.getInfoTypeForString('UNITAI_ANIMAL'))
								newUnit.setHasPromotion(iProm, True)
				for i in range (CyMap().numPlots()):
					pPlot = CyMap().plotByIndex(i)
					if  pPlot.isWater():
						if pPlot.getNumUnits() == 0:
							if CyGame().getSorenRandNum(10000, "SeaSerpent") <= iStygianChance:
								newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_SEA_SERPENT'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								newUnit.setUnitAIType(gc.getInfoTypeForString('UNITAI_ANIMAL'))
								newUnit.setHasPromotion(iProm, True)
				CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MAELSTROM_GELA_2",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),pCaster.getX(),pCaster.getY(),True,True)
				pCaster.kill(True, PlayerTypes.NO_PLAYER)
		else:
			if CyGame().getSorenRandNum(100, "Maelstrom") <= 25:
				CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MAELSTROM_KILL",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
				pCaster.kill(True, PlayerTypes.NO_PLAYER)
			else:
				iOcean = gc.getInfoTypeForString('TERRAIN_OCEAN')
				iBestValue = 0
				pBestPlot = -1
				for i in range (CyMap().numPlots()):
					iValue = 0
					pTargetPlot = CyMap().plotByIndex(i)
					if pTargetPlot.getTerrainType() == iOcean:
						iValue = CyGame().getSorenRandNum(1000, "Maelstrom")
						if pTargetPlot.isOwned() == false:
							iValue += 1000
						if iValue > iBestValue:
							iBestValue = iValue
							pBestPlot = pTargetPlot
				if pBestPlot != -1:
					pCaster.setXY(pBestPlot.getX(), pBestPlot.getY(), false, true, true)
					pCaster.setDamage(25, PlayerTypes.NO_PLAYER)
					CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MAELSTROM_MOVE",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),pCaster.getX(),pCaster.getY(),True,True)

	else:
		if CyGame().getSorenRandNum(100, "Maelstrom") <= 25:
			CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MAELSTROM_KILL",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			pCaster.kill(True, PlayerTypes.NO_PLAYER)
		else:
			iOcean = gc.getInfoTypeForString('TERRAIN_OCEAN')
			iBestValue = 0
			pBestPlot = -1
			for i in range (CyMap().numPlots()):
				iValue = 0
				pTargetPlot = CyMap().plotByIndex(i)
				if pTargetPlot.getTerrainType() == iOcean:
					iValue = CyGame().getSorenRandNum(1000, "Maelstrom")
					if pTargetPlot.isOwned() == false:
						iValue += 1000
					if iValue > iBestValue:
						iBestValue = iValue
						pBestPlot = pTargetPlot
			if pBestPlot != -1:
				pCaster.setXY(pBestPlot.getX(), pBestPlot.getY(), false, true, true)
				pCaster.setDamage(25, PlayerTypes.NO_PLAYER)
				CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MAELSTROM_MOVE",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),pCaster.getX(),pCaster.getY(),True,True)




def onMoveMirrorOfHeaven(pCaster, pPlot):
	if CyGame().getScenarioCounter()==7 :
		if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GELA')):
			pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GELA'), False)
			CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_GELA_MIRROR",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Mirror Of Heaven.dds',ColorTypes(7),pCaster.getX(),pCaster.getY(),True,True)
			pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SOL'), True)
			pPlayer = gc.getPlayer(pCaster.getOwner())
			iX = pPlot.getX()
			iY = pPlot.getY()
			iChampion = gc.getInfoTypeForString('UNIT_CHAMPION')
			iDemon = gc.getInfoTypeForString('PROMOTION_DEMON')
			iHellfire = gc.getInfoTypeForString('IMPROVEMENT_HELLFIRE')
			bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
			for iPlayer2 in range(gc.getMAX_PLAYERS()):
				pPlayer2 = gc.getPlayer(iPlayer2)
				if (pPlayer2.isAlive()):
					if pPlayer2.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
						bPlayer = pPlayer2
						enemyTeam = bPlayer.getTeam()
						eTeam = gc.getTeam(pPlayer.getTeam())
						eTeam.declareWar(enemyTeam, true, WarPlanTypes.WARPLAN_TOTAL)
			for iiX in range(iX-2, iX+3, 1):
				for iiY in range(iY-2, iY+3, 1):
					pPlot2 = CyMap().plot(iiX,iiY)
					if not pPlot2.isWater():
						if pPlot2.getNumUnits() == 0:
							if not pPlot2.isCity():
								if pPlot2.isFlatlands():
									if CyGame().getSorenRandNum(500, "Hellfire") <= 400:
										iImprovement = pPlot2.getImprovementType()
										bValid = True
										if iImprovement != -1 :
											if gc.getImprovementInfo(iImprovement).isPermanent():
												bValid = False
										if bValid :
											pPlot2.setImprovementType(iHellfire)
											newUnit = bPlayer.initUnit(iChampion, pPlot2.getX(), pPlot2.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
											newUnit.setHasPromotion(iDemon, True)


def ApplyGela2(argsList):
	gc.getGame().changeScenarioCounter(2)

def ApplyGela3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	gc.getGame().changeScenarioCounter(3)

def ApplyGela4(argsList):
	gc.getGame().changeScenarioCounter(4)

def ApplyGela5(argsList):
	gc.getGame().changeScenarioCounter(5)

def ApplyGela6(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	gc.getGame().changeScenarioCounter(6)

def ApplyGela7(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	gc.getGame().changeScenarioCounter(7)

def CanDoGela4(argsList):
	iPlayer2 = cf.getCivilization(gc.getInfoTypeForString('CIVILIZATION_MERCURIANS'))
	if 	iPlayer2 != -1:
		return True
	return False

def CanDoGela2(argsList):
	iImp = gc.getInfoTypeForString('IMPROVEMENT_POOL_OF_TEARS')
	iCount = 0
	for i in range(CyMap().getNumAreas()):
		iCount += CyMap().getArea(i).getNumImprovements(iImp)
	if iCount == 0:
		return False
	return True

def CanDoGela3(argsList):
	iImp = gc.getInfoTypeForString('IMPROVEMENT_MAELSTROM')
	iCount = 0
	for i in range(CyMap().getNumAreas()):
		iCount += CyMap().getArea(i).getNumImprovements(iImp)
	if iCount == 0:
		return False
	return True

def CanDoGela5(argsList):
	iImp = gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC')
	iCount = 0
	for i in range(CyMap().getNumAreas()):
		iCount += CyMap().getArea(i).getNumImprovements(iImp)
	if iCount == 0:
		return False
	return True

def CanDoGela6(argsList):
	iImp = gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER')
	iCount = 0
	for i in range(CyMap().getNumAreas()):
		iCount += CyMap().getArea(i).getNumImprovements(iImp)
	if iCount == 0:
		return False
	return True

def CanDoGela7(argsList):
	iImp = gc.getInfoTypeForString('IMPROVEMENT_MIRROR_OF_HEAVEN')
	iCount = 0
	for i in range(CyMap().getNumAreas()):
		iCount += CyMap().getArea(i).getNumImprovements(iImp)
	if iCount == 0:
		return False
	return True

def helpStrangeAdept(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_STRANGE_ADEPT_HELP", ())
	return szHelp

def CanDoHellRefugees(argsList):
	for iPlayer2 in range(gc.getMAX_PLAYERS()):
		pPlayer2 = gc.getPlayer(iPlayer2)
		if (pPlayer2.isAlive()):
			if pPlayer2.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
				return true
	return false

def	doHellRefugees3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_CHAMPION'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_BLESSED'), True)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON_SLAYING'), True)
	newUnit2 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_CHAMPION'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_BLESSED'), True)
	newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON_SLAYING'), True)

def CanDoHellRefugees4(argsList):
	if CyGame().getGlobalCounter() >35 :
		return true
	return false

def doHellRefugees4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_HIGH_PRIEST_OF_THE_ORDER'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def doHellRefugees5(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)

	eTeam = gc.getTeam(pPlayer.getTeam())
	for iPlayer2 in range(gc.getMAX_PLAYERS()):
		pPlayer2 = gc.getPlayer(iPlayer2)
		if (pPlayer2.isAlive() and pPlayer2 != pPlayer  and iPlayer2 != gc.getBARBARIAN_PLAYER()):
			iReligion = pPlayer2.getStateReligion()
			if iReligion == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
				i2Team = gc.getPlayer(iPlayer2).getTeam()
				if eTeam.isAtWar(i2Team):
					eTeam.makePeace(i2Team)

def CanDoScholars4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_BARBARIAN')):
		return true
	return false

def DoScholars4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SWORDSMAN'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit2 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SWORDSMAN'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def CanDoScholars2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_LIBRARY')) == 0:
		return False
	return True

def CanDoTrappedFrostlings3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_FREAK_SHOW')) == 0:
		return False
	return True

def DoTrappedFrostlings2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	iUnit = gc.getInfoTypeForString('UNIT_FROSTLING')
	pBestPlot = -1
	iBestPlot = -1
	iX = pCity.getX()
	iY = pCity.getY()
	for iiX in range(iX-3, iX+3, 1):
		for iiY in range(iY-3, iY+3, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if not pPlot2.isWater():
				if pPlot2.getNumUnits() == 0:
					if not pPlot2.isCity():
						iPlot = CyGame().getSorenRandNum(500, "Frostlings")
						if iPlot > iBestPlot:
							iBestPlot = iPlot
							pBestPlot = pPlot2
	if iBestPlot != -1:
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(iUnit, pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)

def DoTrappedFrostlings5(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	iUnit = gc.getInfoTypeForString('UNIT_FROSTLING')
	newUnit = pPlayer.initUnit(iUnit, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), True)

def canTriggerPacifistDemonstration(argsList):
	kTriggeredData = argsList[0]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	destPlayer = gc.getPlayer(kTriggeredData.eOtherPlayer)
	if not gc.getTeam(pPlayer.getTeam()).canChangeWarPeace(destPlayer.getTeam()):
		return false
	if gc.getTeam(pPlayer.getTeam()).AI_getAtWarCounter(destPlayer.getTeam()) < 10:
		return false
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
		return False
	return true

def DoPacifistDemonstration2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	destPlayer = gc.getPlayer(kTriggeredData.eOtherPlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	eTeam = gc.getTeam(pPlayer.getTeam())
	eTeam.changeWarWeariness(pPlayer.getTeam(),25)


def DoPacifistDemonstration5(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	destPlayer = gc.getPlayer(kTriggeredData.eOtherPlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	eTeam = gc.getTeam(pPlayer.getTeam())
	eTeam.changeWarWeariness(pPlayer.getTeam(),-10)
	for i in range(pCity.plot().getNumUnits()):
		pUnit = pCity.plot().getUnit(i)
		if pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE')):
			pUnit.changeExperience(3, -1, False, False, False)

def CanDoPacifistDemonstration4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_BARBARIAN')):
		return true
	return false

def DoPacifistDemonstration4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_AXEMAN'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)

def DoPacifistDemonstration3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	for i in range(pCity.plot().getNumUnits()):
		pUnit = pCity.plot().getUnit(i)
		pUnit.changeExperience(3, -1, False, False, False)

def CanTriggerTrappedFrostlings (argsList):
	kTriggeredData = argsList[0]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	iX = pCity.getX()
	iY = pCity.getY()
	iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
	iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
	for iiX in range(iX-3, iX+3, 1):
		for iiY in range(iY-3, iY+3, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			iTerrain = pPlot2.getTerrainType()
			if iTerrain == iTundra:
				return true
			if iTerrain == iSnow:
				return true
	return false

def CanTriggerDemonSign (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_MERCURIANS")):
		return false
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_INFERNALS")):
		return false
	return true

def	doDemonSign2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_PROPHET'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def	doDemonSign3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_HIGH_PRIEST_OF_THE_VEIL'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PROPHECY_MARK'), True)

def	doDemonSign5(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ADVENTURER'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PROPHECY_MARK'), True)

def	doDemonSign6(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LUONNOTAR'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def CanDoAshCough2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_SHEAIM")):
		return true
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_KURIOTATES")):
		return true
	return false

def CanDoAshCough4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_MAGE_GUILD')) == 0:
		return False
	return True

def doAshCough4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if CyGame().getSorenRandNum(100, "Cough")< 50 :
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ADEPT'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ADEPT_DRAGON_MAGE'), True)
	else:
		pCity.changeHurryAngerTimer(10)

def CanDoDeadAngel2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
		return False
	return true

def doDeadAngel4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pPlot.changePlotCounter(100)

def doDeadAngel5 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ANGEL'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_UNDEAD'), True)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PLAGUE_CARRIER'), True)

def doDevastatingPlague1 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)

	iPop = pCity.getPopulation()
	iPop = int(iPop / 2)
	if iPop == 0:
		iPop = 1
	pCity.setPopulation(iPop)
	for i in range((pCity.plot()).getNumUnits()):
		pUnit = (pCity.plot()).getUnit(i)
		pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PLAGUED'), True)

def doDevastatingPlague4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)

	iPop = pCity.getPopulation()
	iPop = int(iPop / 2)
	if iPop == 0:
		iPop = 1
	pCity.setPopulation(iPop)
	iPop = int(iPop / 2)
	if iPop == 0:
		iPop = 1
	for i in range(0,iPop,1):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DISEASED_CORPSE'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def doMassiveSuicide5 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit2 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def CanDoNecroCannibalism2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):
		return False
	return true

def doNecroCannibalism2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)

	iPop = pCity.getPopulation()
	iPop = int(iPop / 2)
	if iPop == 0:
		iPop = 1
	pCity.setPopulation(iPop)
	for i in range(0,iPop,1):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SWORDSMAN'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CANNIBALIZE'), True)

def	doNecroCannibalism4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DISEASED_CORPSE'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DISEASED_CORPSE'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def canTriggerHellPortalCity(argsList):
	eTrigger = argsList[0]
	ePlayer = argsList[1]
	iCity = argsList[2]
	pPlayer = gc.getPlayer(ePlayer)
	pCity = pPlayer.getCity(iCity)
	if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS')) == 0:
			return True
		return False
	return False

def doHellPortal(argsList):
	kTriggeredData = argsList[0]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS'), 1)

def doGhostShip (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	irand = CyGame().getSorenRandNum(120, "GhostShip")
	iX = pCity.getX()
	iY = pCity.getY()
	bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	if irand<20 :
		for iiX in range(iX-2, iX+2, 1):
			for iiY in range(iY-2, iY+2, 1):
				pPlot2 = CyMap().plot(iiX,iiY)
				if pPlot2.isWater():
					if pPlot2.getNumUnits() == 0:
						newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_DROWN'), iiX, iiY, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		CyInterface().addMessage(kTriggeredData.ePlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_GHOST_SHIP_1_1",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),iX,iY,True,True)
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GALLEON'), iX, iY, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	if irand>=20:
		if irand<40:
			pCity.changeEspionageHealthCounter(5)
			CyInterface().addMessage(kTriggeredData.ePlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_GHOST_SHIP_1_2",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),iX,iY,True,True)
			newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GALLEON'), iX, iY, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		if irand>=40:
			if irand<60:
				newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LUNATIC'), iX, iY, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GALLEON'), iX, iY, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				CyInterface().addMessage(kTriggeredData.ePlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_GHOST_SHIP_1_3",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),iX,iY,True,True)

			if irand>=60:
				if irand<80:
					pPlayer.changeGold(50)
					CyInterface().addMessage(kTriggeredData.ePlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_GHOST_SHIP_1_4",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),iX,iY,True,True)
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GALLEON'), iX, iY, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				if irand>=80:
					if irand<100:
						pCity.changeHurryAngerTimer(5)
						CyInterface().addMessage(kTriggeredData.ePlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_GHOST_SHIP_1_5",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),iX,iY,True,True)

					if irand>=100:
						iPlayer = kTriggeredData.ePlayer
						cf.placeTreasure(iPlayer, gc.getInfoTypeForString('EQUIPMENT_TREASURE'))
						CyInterface().addMessage(kTriggeredData.ePlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_GHOST_SHIP_1_6",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Maelstrom.dds',ColorTypes(7),iX,iY,True,True)
						newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GALLEON'), iX, iY, UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)


def doOrphanedGoblin1 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pUnit = pPlayer.getUnit(kTriggeredData.iUnitId)
	pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORC_SLAYING'), True)
	pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)

def doOrphanedGoblin2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pUnit = pPlayer.getUnit(kTriggeredData.iUnitId)
	if not isWorldUnitClass(pUnit.getUnitClassType()):
		if pUnit.isAlive():
			if not pUnit.isAnimal():
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GOBLIN'), True)

def doOrphanedGoblin3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pUnit = pPlayer.getUnit(kTriggeredData.iUnitId)
	iBestValue = 0
	pBestPlot = -1
	bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	iX = pUnit.getX()
	iY = pUnit.getY()
	for iiX in range(iX-2, iX+2, 1):
		for iiY in range(iY-2, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			iValue = 0
			if not pPlot.isWater():
				if not pPlot.isPeak():
					if pPlot.getNumUnits() == 0:
						iValue = CyGame().getSorenRandNum(1000, "Goblin1")
						if iValue > iBestValue:
							iBestValue = iValue
							pBestPlot = pPlot
	if (pBestPlot!=-1):
		if CyGame().getSorenRandNum(1000, "Goblin1")<500:
			newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_GOBLIN'), pBestPlot.getX(),pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)

		if CyGame().getSorenRandNum(1000, "Goblin1")>500:
			newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GOBLIN'), pBestPlot.getX(),pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)

def doOrphanedGoblin4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pUnit = pPlayer.getUnit(kTriggeredData.iUnitId)
	pUnit.changeExperience(1* -1, -1, False, False, False)
	pCity = pPlayer.getCapitalCity()
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GOBLIN'), pCity.getX(),pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def doThatKindOfDay1 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	iX = pCity.getX()
	iY = pCity.getY()
	pCity.setPopulation(1)

	for iiX in range(iX-2, iX+2, 1):
		for iiY in range(iY-2, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			for i in range(pPlot2.getNumUnits()):
				pUnit2 = pPlot2.getUnit(i)
				if pUnit2.getOwner()== gc.getBARBARIAN_PLAYER():
					if not isWorldUnitClass(pUnit2.getUnitClassType()):
						pUnit2.kill(True, PlayerTypes.NO_PLAYER)

def doThatKindOfDay2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	iX = pCity.getX()
	iY = pCity.getY()
	pCity.setProduction(0)

	for iiX in range(iX-2, iX+2, 1):
		for iiY in range(iY-2, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			for i in range(pPlot2.getNumUnits()):
				pUnit2 = pPlot2.getUnit(i)
				if pUnit2.getOwner()== gc.getBARBARIAN_PLAYER() :
					if not isWorldUnitClass(pUnit2.getUnitClassType()):
						pUnit2.kill(True, PlayerTypes.NO_PLAYER)



def canDoThatKindOfDay3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if pPlayer.isHuman() == False:
		return False
	if CyGame().getRankPlayer(CyGame().countCivPlayersAlive()-1) == kTriggeredData.ePlayer:
		return False

	if CyGame().getWBMapScript():
		return False
	return True

def doThatKindOfDay3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iNewPlayer = CyGame().getRankPlayer(CyGame().countCivPlayersAlive()-1)
	iOldPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlayer2 = gc.getPlayer(CyGame().getRankPlayer(CyGame().countCivPlayersAlive()-1 ))
	pCity = pPlayer.getCity(kTriggeredData.iCityId)

	CyGame().reassignPlayerAdvanced(iOldPlayer, iNewPlayer, -1)

def doThatKindOfDay4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	iX = pCity.getX()
	iY = pCity.getY()

	for iiX in range(iX-2, iX+2, 1):
		for iiY in range(iY-2, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			for i in range(pPlot2.getNumUnits()):
				pUnit2 = pPlot2.getUnit(i)
				if pUnit2.getOwner()== gc.getBARBARIAN_PLAYER() :
					if not isWorldUnitClass(pUnit2.getUnitClassType()):
						pUnit2.kill(True, PlayerTypes.NO_PLAYER)


def doThatKindOfDay5 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	iX = pCity.getX()
	iY = pCity.getY()
	pCity.changePopulation(1)
	for iiX in range(iX-2, iX+2, 1):
		for iiY in range(iY-2, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			for i in range(pPlot2.getNumUnits()):
				pUnit2 = pPlot2.getUnit(i)
				if pUnit2.getOwner()== gc.getBARBARIAN_PLAYER() :
					if not isWorldUnitClass(pUnit2.getUnitClassType()):
						pUnit2.kill(True, PlayerTypes.NO_PLAYER)

def CanTriggerThatKindOfDay(argsList):
	kTriggeredData = argsList[0]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)

	if (pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_CLAN_OF_EMBERS')):
		if (gc.getGame().getGameTurnYear())<100:
			if pPlayer.getNumCities()==1 :
				pCity = pPlayer.getCapitalCity()
				iX = pCity.getX()
				iY = pCity.getY()

				pPlot=CyMap().plot(iX,iY)
				if pPlot.getNumUnits() == 0:
					for iiX in range(iX-2, iX+2, 1):
						for iiY in range(iY-2, iY+2, 1):
							pPlot2 = CyMap().plot(iiX,iiY)
							for i in range(pPlot2.getNumUnits()):
								pUnit2 = pPlot2.getUnit(i)
								if pUnit2.getOwner()== gc.getBARBARIAN_PLAYER():
									if not isWorldUnitClass(pUnit2.getUnitClassType()):
										return true

	return false



def canDoThatKindOfDay4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_AGGRESSIVE')) == True:
		return True
	return False

def canDoThatKindOfDay5(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_KURIOTATES")):
		return True
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_ELOHIM")):
		return True
	return False

def CanDoPrincessRule4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALACE_MERCURIANS')) == 0:

		return False
	else:
		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALACE_INFERNAL')) == 0:
			return False
	return True

def CanDoImmigrants1 (argsList):
	if CyGame().isOption(GameOptionTypes.GAMEOPTION_ONE_CITY_CHALLENGE):
		return False
	return True

def CanDoCorruptJudge4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_MEMBERSHIP')) != gc.getInfoTypeForString('CIVIC_UNDERCOUNCIL'):
		return False
	return True

def CanDoWaywardElves2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_LJOSALFAR")):
		return True
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_SVARTALFAR")):
		return True
	return False

def CanDoWaywardElves4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_LABOR')) != gc.getInfoTypeForString('CIVIC_SLAVERY'):
		return False
	return True

def doWayWardElves4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SLAVE'), pPlot.getX(),pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit2 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SLAVE'), pPlot.getX(),pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit1.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ELF'), True)
	newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ELF'), True)

def doWayWardElves5 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ARCHERY_RANGE'), 1)
	newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pCity.getX(),pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit1.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ELF'), True)
	newUnit1.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEXTEROUS'), True)
	newUnit1.setHasPromotion(gc.getInfoTypeForString('PROMOTION_COMMANDO'), True)

def doBoardGame4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DISCIPLE_THE_ORDER'), pCity.getX(),pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit1.setHasPromotion(gc.getInfoTypeForString('PROMOTION_INQUISITOR'), True)

def doWayWardElves1 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_HAMLET'))

def doTraveller1 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()
	implist = []
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_MAELSTROM') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_MIRROR_OF_HEAVEN') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_YGGDRASIL') :
			implist = implist + [pPlot]
	pPlot= implist[CyGame().getSorenRandNum(len(implist), "Pick Plot")]
	pPlot.setRevealed(iTeam, True, False, TeamTypes.NO_TEAM)

def doTraveller2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()
	implist = []
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
#		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_AIFON_ISLE') :
#			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_DRAGON_BONES') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_REMNANTS_OF_PATRIA') :
			implist = implist + [pPlot]
	pPlot= implist[CyGame().getSorenRandNum(len(implist), "Pick Plot")]
	pPlot.setRevealed(iTeam, True, False, TeamTypes.NO_TEAM)

def doTraveller3 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()
	implist = []
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_LETUM_FRIGUS') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_SEVEN_PINES') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_TOMB_OF_SUCELLUS') :
			implist = implist + [pPlot]
	pPlot= implist[CyGame().getSorenRandNum(len(implist), "Pick Plot")]
	pPlot.setRevealed(iTeam, True, False, TeamTypes.NO_TEAM)


def doTraveller4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()
	implist = []
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		# if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_ODIOS_PRISON') :
			# implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_POOL_OF_TEARS') :
			implist = implist + [pPlot]
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_RING_OF_CARCER') :
			implist = implist + [pPlot]
	pPlot= implist[CyGame().getSorenRandNum(len(implist), "Pick Plot")]
	pPlot.setRevealed(iTeam, True, False, TeamTypes.NO_TEAM)

def doTraveller5 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()
	maxpop = -1
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.isAlive() and pPlayer.getID() != kTriggeredData.ePlayer:
			for pyCity in PyPlayer(iPlayer).getCityList() :
				pCity = pyCity.GetCy()
				if pCity.getPopulation() > maxpop:
					maxpop = pCity.getPopulation()
					pPlot = pCity.plot()
	pPlot.setRevealed(iTeam, True, False, TeamTypes.NO_TEAM)

def CanDoTraveller1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()

	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_MAELSTROM') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_MIRROR_OF_HEAVEN') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_YGGDRASIL') :
			return true
	return false

def CanDoTraveller2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()

	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
#		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_AIFON_ISLE') :
#			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_DRAGON_BONES') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_REMNANTS_OF_PATRIA') :
			return true
	return false

def CanDoTraveller3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()

	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_LETUM_FRIGUS') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_SEVEN_PINES') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_TOMB_OF_SUCELLUS') :
			return true
	return false

def CanDoTraveller4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	iTeam = pPlayer.getTeam()

	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		# if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_ODIOS_PRISON') :
			# return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_POOL_OF_TEARS') :
			return true
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_RING_OF_CARCER') :
			return true
	return false

def CanTriggerUnfortunateAssassinCity(argsList):
	eTrigger = argsList[0]
	ePlayer = argsList[1]
	iCity = argsList[2]
	pPlayer = gc.getPlayer(ePlayer)
	pCity = pPlayer.getCity(iCity)
	if pCity.isCapital():
		return true
	return false

def doUnfortunateAssassin3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pdestPlayer = pPlayer
	minattitude=0
	for iLoopPlayer in range(gc.getMAX_PLAYERS()):
		pLoopPlayer = gc.getPlayer(iLoopPlayer)
		if pLoopPlayer.isAlive():
			if pLoopPlayer != pPlayer :
				test= CyGame().getSorenRandNum(100, "Pick CIV")
				if test > minattitude :
					pdestPlayer=pLoopPlayer
					minattitude = test
	if CyGame().getSorenRandNum(100, "Pick Plot")<50 :
		pdestPlayer.AI_changeAttitudeExtra(kTriggeredData.ePlayer,-1)
	else:
		pdestPlayer.AI_changeAttitudeExtra(kTriggeredData.ePlayer,1)

def doUnfortunateAssassin5(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit1 = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ASSASSIN'), pCity.getX(),pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def CanDoOvercrowdedDungeon5 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):
		return False
	return true

def doOvercrowdedDungeon5 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	if CyGame().getSorenRandNum(100, "Riot")<25 :
		pCity.changeOccupationTimer(5)

def doClairone1 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.eOtherPlayer)
	pPlayer.changeExtraHappiness(-1)


def doAncientBurial2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if CyGame().getSorenRandNum(100, "Skeleton")<20 :
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_SKELETON'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setName("Ancient Warrior")
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEROIC_DEFENSE'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)

def doAncientBurial3 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pPlot.setPlotType(PlotTypes.PLOT_LAND, True, True)
	if CyGame().getSorenRandNum(100, "Skeleton")<90 :
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_SKELETON'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setName("Ancient Warrior")
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEROIC_DEFENSE'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)

def doAncientBurial4 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if CyGame().getSorenRandNum(100, "Skeleton")<40 :
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_SKELETON'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setName("Ancient Warrior")
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEROIC_DEFENSE'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)
	else:
		pPlayer.changeGold(90)

def doMadGolemicist2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pUnit = pPlayer.getUnit(kTriggeredData.iUnitId)
	pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SHEUT_STONE'), True)
	bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	pPlot = pUnit.plot()
	pNewPlot = cf.findClearPlot(-1, pPlot)
	if pNewPlot != -1:
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_WOOD_GOLEM'), pNewPlot.getX(), pNewPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit2 = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_WOOD_GOLEM'), pNewPlot.getX(), pNewPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def doMadGolemicist3 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pUnit = pPlayer.getUnit(kTriggeredData.iUnitId)
	pPlot = pUnit.plot()
	if pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_BARNAXUS'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_MAD_GOLEMICIST_3_BARNAXUS",()),'',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		pUnit.kill(True, PlayerTypes.NO_PLAYER)
	if pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_MAD_GOLEMICIST_3_GOLEM",()),'',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEROIC_DEFENSE'), True)
		pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEROIC_STRENGTH'), True)
	else:
		if CyGame().getSorenRandNum(100, "Golem")<50 :
			newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_FLESH_GOLEM'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_MAD_GOLEMICIST_3_HUMAN_1",()),'',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		else:
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_EVENT_MAD_GOLEMICIST_3_HUMAN_2",()),'',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM'), True)
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EMPOWER1'), True)
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EMPOWER2'), True)
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EMPOWER3'), True)
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EMPOWER4'), True)
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EMPOWER5'), True)

def doMonkPilgrimage2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pUnit = pPlayer.getUnit(kTriggeredData.iUnitId)
	pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FIRST_PILGRIMAGE'), True)

def CanDoMonkPilgrimage2 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.isHuman():
		return true
	return false

def doElderDeath3 (argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SKELETON'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def canTriggerSkilledJeweler(argsList):
	eTrigger = argsList[0]
	ePlayer = argsList[1]
	iCity = argsList[2]
	pPlayer = gc.getPlayer(ePlayer)
	pCity = pPlayer.getCity(iCity)
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_JEWELER')) > 0:
		return False
	if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_KURIOTATES'):
		return False
	return True

######## MORE EVENTS MOD EXPANDED STARTS ########

def canTriggerInfernalFilter(argsList):
	kTriggeredData = argsList[0]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
		return False
	return True

				# More Events Mod Ends #

def canTriggerAshenVeilFilter(argsList):
	kTriggeredData = argsList[0]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
		return False
	return True

def CanDoOil3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_KHAZAD")):
		return True
	if (pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_LUCHUIRP")):
		return True
	return False

def helpPoisonedWater3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_POISONED_WATER_HELP_3", ())
	return szHelp

def helpPoisonedWater4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_POISONED_WATER_HELP_4", ())
	return szHelp


def canTriggerDemonicTower(argsList):
	kTriggeredData = argsList[0]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.isNone():
		return False
	if pPlot.getNumUnits() > 0:
		return False
	if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
		return False
	return True


def doDemonicTower1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.getNumUnits() == 0:
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANTICORE'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
      	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)
      	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PLAGUED'), True)


def helpDemonicTower1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_DEMONIC_TOWER_HELP_1", ())
	return szHelp


def canTriggerRalphAndSam(argsList):
	kTriggeredData = argsList[0]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.isNone():
		return False
	if pPlot.getNumUnits() > 0:
		return False
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
		return False
	return True

def doRalphAndSam1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.getNumUnits() == 0:
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_WOLF_PACK'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
      	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)

def helpRalphAndSam1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_RALPH_AND_SAM_HELP_1", ())
	return szHelp


def doCentaurTribe1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.getNumUnits() == 0:
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_CENTAUR_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
      	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)


def helpCentaurTribe1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_CENTAUR_TRIBE_HELP_1", ())
	return szHelp


def doCentaurTribe2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.getNumUnits() == 0:
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_CENTAUR'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def doSeaSerpent1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.getNumUnits() == 0:
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_SEA_SERPENT'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
      	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)


def helpSeaSerpent1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_SEASERPENT_HELP_1", ())
	return szHelp

def doMonkeySee1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.getNumUnits() == 0:
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_GORILLA'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)


def helpMonkeySee1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_MONKEY_SEE_HELP_1", ())
	return szHelp

def canTriggerLanunPirates(argsList):

	kTriggeredData = argsList[0]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
	map = gc.getMap()

#   If Barbarians are disabled in this game, this event will not occur.
	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_BARBARIANS):
		return False

#   If Lanun already exist in this game, this event will not occur.
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_LANUN'):
			return false
	return true


#   At least one civ on the board must know Optics
	bFoundValid = false
	iTech = CvUtil.findInfoTypeNum(gc.getTechInfo, gc.getNumTechInfos(), 'TECH_OPTICS')
	for iPlayer in range(gc.getMAX_CIV_PLAYERS()):
		loopPlayer = gc.getPlayer(iPlayer)
		if loopPlayer.isAlive():
			if gc.getTeam(loopPlayer.getTeam()).isHasTech(iTech):
				bFoundValid = true
				break

	if not bFoundValid:
		return False


#	Find an eligible plot
	map = gc.getMap()
	for i in range(map.numPlots()):
		plot = map.plotByIndex(i)
		if (plot.getOwner() == -1 and plot.isWater() and not plot.isImpassable() and not plot.getNumUnits() > 0 and not plot.isLake() and plot.isAdjacentPlayer(kTriggeredData.ePlayer, true)):
			return True

	return False

def getHelpLanunPirates1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]

	szHelp = localText.getText("TXT_KEY_EVENT_LANUN_PIRATES_HELP_1", ())

	return szHelp

def applyLanunPirates1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)

	listPlots = []
	map = gc.getMap()
	for i in range(map.numPlots()):
		plot = map.plotByIndex(i)
		if (plot.getOwner() == -1 and plot.isWater() and not plot.isImpassable() and not plot.getNumUnits() > 0 and not plot.isLake() and plot.isAdjacentPlayer(kTriggeredData.ePlayer, true)):
			listPlots.append(i)

	if 0 == len(listPlots):
		return

	plot = map.plotByIndex(listPlots[gc.getGame().getSorenRandNum(len(listPlots), "Lanun Pirates event location")])

	if map.getWorldSize() == CvUtil.findInfoTypeNum(gc.getWorldInfo, gc.getNumWorldInfos(), 'WORLDSIZE_DUEL'):
		iNumUnit1  = 2
	elif map.getWorldSize() == CvUtil.findInfoTypeNum(gc.getWorldInfo, gc.getNumWorldInfos(), 'WORLDSIZE_TINY'):
		iNumUnit1  = 2
	elif map.getWorldSize() == CvUtil.findInfoTypeNum(gc.getWorldInfo, gc.getNumWorldInfos(), 'WORLDSIZE_SMALL'):
		iNumUnit1  = 3
	elif map.getWorldSize() == CvUtil.findInfoTypeNum(gc.getWorldInfo, gc.getNumWorldInfos(), 'WORLDSIZE_STANDARD'):
		iNumUnit1  = 4
	elif map.getWorldSize() == CvUtil.findInfoTypeNum(gc.getWorldInfo, gc.getNumWorldInfos(), 'WORLDSIZE_LARGE'):
		iNumUnit1  = 5
	else:
		iNumUnit1  = 6

	iUnitType1 = CvUtil.findInfoTypeNum(gc.getUnitInfo, gc.getNumUnitInfos(), 'UNIT_PRIVATEER')


	barbPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	for i in range(iNumUnit1):
		barbPlayer.initUnit(iUnitType1, plot.getX(), plot.getY(), UnitAITypes.UNITAI_ATTACK_SEA, DirectionTypes.DIRECTION_SOUTH)

	(loopUnit, iter) = barbPlayer.firstUnit(false)
	while (loopUnit):
		if loopUnit.getUnitType() == iUnitType1:
			loopUnit.setName("Lanun Raider")
		(loopUnit, iter) = barbPlayer.nextUnit(iter, false)


def canTriggerKuriotatesWorkboat(argsList):
	kTriggeredData = argsList[0]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_KURIOTATES'):
		return False
	return True


def canTriggerCityKuriotatesWorkboat(argsList):
	eTrigger = argsList[0]
	ePlayer = argsList[1]
	iCity = argsList[2]
	pPlayer = gc.getPlayer(ePlayer)
	pCity = pPlayer.getCity(iCity)
	if not pCity.isSettlement():
		return False
	return True

def doHauntedCastle4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	if pPlot.getNumUnits() == 0:
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_SPECTRE'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setName("Vengeful Dead")
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEROIC_DEFENSE'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)

def helpHauntedCastle4(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_HAUNTED_CASTLE_HELP_4", ())
	return szHelp

def doPerfectStorm3(argsList):
	#kTriggeredData = argsList[0]
	iEvent = argsList[0]
	kTriggeredData = argsList[1]

	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	pPlayer.changeMana(100)

def helpPerfectStorm3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_PERFECT_STORM_HELP_3", ())
	return szHelp

######## MORE EVENTS MOD EXPANDED ENDS ########
