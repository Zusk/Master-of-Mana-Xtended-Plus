from CvPythonExtensions import *
import CvUtil
import Popup as PyPopup
import CvScreensInterface
import sys
import PyHelpers
import CustomFunctions

cf = CustomFunctions.CustomFunctions()

PyInfo = PyHelpers.PyInfo
PyPlayer = PyHelpers.PyPlayer
gc = CyGlobalContext()

def globalenchantment(argsList):
	iPlayer, eProject = argsList
	GE = gc.getProjectInfo(eProject)
	eval(GE.getPyResult())
	
def addWhiteHandUnit(iUnit):
	pBestPlot = -1
	iBestPlot = -1
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		iPlot = -1
		if pPlot.isWater() == False:
			if pPlot.getNumUnits() == 0:
				if pPlot.isCity() == False:
					if pPlot.isImpassable() == False:
						iPlot = CyGame().getSorenRandNum(500, "Add Unit")
						iPlot = iPlot + (pPlot.area().getNumTiles() * 10)
						if pPlot.isOwned():
							iPlot = iPlot / 2
						if iPlot > iBestPlot:
							iBestPlot = iPlot
							pBestPlot = pPlot
	if iBestPlot != -1:
		bPlayer = gc.getPlayer(gc.getWHITEHAND_PLAYER())
		newUnit = bPlayer.initUnit(iUnit, pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)	
		
def samhain(iPlayer):
	for pyCity in PyPlayer(iPlayer).getCityList():
		pCity = pyCity.GetCy()
		pCity.changeHappinessTimer(20)
	iCount = CyGame().countCivPlayersAlive() + int(CyGame().getHandicapType()) - 5
	for i in range(iCount):
	
		addWhiteHandUnit(gc.getInfoTypeForString('UNIT_FROSTLING'))
		addWhiteHandUnit(gc.getInfoTypeForString('UNIT_FROSTLING'))
		addWhiteHandUnit(gc.getInfoTypeForString('UNIT_FROSTLING_ARCHER'))
		addWhiteHandUnit(gc.getInfoTypeForString('UNIT_FROSTLING_WOLF_RIDER'))
	addWhiteHandUnit(gc.getInfoTypeForString('UNIT_MOKKA'))

def whitehand(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	pCity = pPlayer.getCapitalCity()
	iPriest = gc.getInfoTypeForString('UNIT_PRIEST_OF_WINTER')
	iHighPriest = gc.getInfoTypeForString('UNIT_HIGH_PRIEST_OF_WINTER')
	
	bDum=true
	bRiu=true
	bAna=true
	py = PyPlayer(iPlayer)
	for pUnit in py.getUnitList():
		if (pUnit.getUnitType()==iPriest or pUnit.getUnitType()==iHighPriest):
			if pUnit.getNameKey()=="Dumannios":
				bDum=false
			if pUnit.getNameKey()=="Riuros":
				bRiu=false
			if pUnit.getNameKey()=="Anagantios":
				bAna=false

	if bDum:
		newUnit1 = pPlayer.initUnit(iPriest, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit1.setName("Dumannios")
		
	if bRiu:		
		newUnit2 = pPlayer.initUnit(iPriest, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit2.setName("Riuros")
	
	if bAna:	
		newUnit3 = pPlayer.initUnit(iPriest, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit3.setName("Anagantios")
		
def deepening(iPlayer):	
	iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
	iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
	iMarsh = gc.getInfoTypeForString('TERRAIN_MARSH')
	iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
	iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
	iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
	iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
	iTimer = 40 + (CyGame().getGameSpeedType() * 20)
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		bValid = False
		if pPlot.isWater() == False:
			if CyGame().getSorenRandNum(100, "The Deepening") < 75:
				iTerrain = pPlot.getTerrainType()
				chance = CyGame().getSorenRandNum(100, "Bob")
				if iTerrain == iSnow:
					bValid = True
				if iTerrain == iTundra:
					pPlot.setTempTerrainType(iSnow, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
					bValid = True
				if iTerrain == iGrass or iTerrain == iMarsh:
					if chance < 40:
						pPlot.setTempTerrainType(iSnow, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
					else:
						pPlot.setTempTerrainType(iTundra, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
					bValid = True
				if iTerrain == iPlains:
					if chance < 60:
						pPlot.setTempTerrainType(iSnow, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
					else:
						pPlot.setTempTerrainType(iTundra, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
					bValid = True
				if iTerrain == iDesert:
					if chance < 10:
						pPlot.setTempTerrainType(iSnow, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
					elif chance < 30:
						pPlot.setTempTerrainType(iTundra, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
					else:
						pPlot.setTempTerrainType(iPlains, CyGame().getSorenRandNum(iTimer, "Bob") + 10)
				if bValid:
					if CyGame().getSorenRandNum(750, "The Deepening") < 10:
						pPlot.setFeatureType(iBlizzard,-1)
						
def stirfromslumber(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	pCity = pPlayer.getCapitalCity()
	iDragon = gc.getInfoTypeForString('UNIT_DRIFA')

	bDrifa=true

	py = PyPlayer(iPlayer)
	for pUnit in py.getUnitList():
		if pUnit.getUnitType()==iDragon:
				bDrifa=false

	if bDrifa:
		newUnit1 = pPlayer.initUnit(iDragon, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def thedraw(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	
	pPlayer.changeNoDiplomacyWithEnemies(1)
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iLoopTeam in range(gc.getMAX_TEAMS()):
		if iLoopTeam != iTeam:
			if iLoopTeam != gc.getPlayer(gc.getBARBARIAN_PLAYER()).getTeam():
				eLoopTeam = gc.getTeam(iLoopTeam)
				if eLoopTeam.isAlive():
					if not eLoopTeam.isAVassal():
						eTeam.declareWar(iLoopTeam, false, WarPlanTypes.WARPLAN_LIMITED)
	py = PyPlayer(iPlayer)
	for pUnit in py.getUnitList():
		iDmg = pUnit.getDamage() * 2
		if iDmg > 99:
			iDmg = 99
		if iDmg < 50:
			iDmg = 50
		pUnit.setDamage(iDmg, iPlayer)
	for pyCity in PyPlayer(iPlayer).getCityList():
		pLoopCity = pyCity.GetCy()
		iPop = int(pLoopCity.getPopulation() / 2)
		if iPop < 1:
			iPop = 1
		pLoopCity.setPopulation(iPop)

def AscensionFinal(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	pCity = pPlayer.getCapitalCity()
	
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


def GrigoriAdventure(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	
	iMapPlots=CyMap().numPlots()
	listPlots = []
	
	for i in range(iMapPlots):
		bvalid=false
		pPlot = CyMap().plotByIndex(i)			
		if not pPlot.isWater():
			if not pPlot.isPeak():
				if pPlot.getOwner()==iPlayer:
					if pPlot.getImprovementType()==-1:
						bValid=true
						for iiX in range(pPlot.getX()-2, pPlot.getY()+3, 1):
							for iiY in range(pPlot.getY()-2, pPlot.getY()+3, 1):
								pPlot2 = CyMap().plot(iiX,iiY)
								if pPlot2.getImprovementType()==gc.getInfoTypeForString('IMPROVEMENT_DUNGEON'):
									bValid=false
								
						if bValid:
							listPlots.append(i)

	if(len(listPlots)>0):	
		iRnd=CyGame().getSorenRandNum(len(listPlots), "New Dungeon")
		pPlot = CyMap().plotByIndex(listPlots[iRnd])			
		pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_DUNGEON'))
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DUNGEON_DISCOVERED",()),'AS2D_DISCOVERBONUS',3,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,3,4',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)

def GiftofKilmorph(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_METAL,300)

def HallowingElohim(iPlayer):
	CyGame().changeGlobalCounter(-25)

def ElegySheaim(iPlayer):
	CyGame().changeGlobalCounter(15)

def DemonPortal(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = pPlayer.getCapitalCity().plot()
	
	iUnitType = gc.getInfoTypeForString('UNIT_CHAOS_MARAUDER')
	
	if CyGame().getGlobalCounter()>50:
		iUnitType = gc.getInfoTypeForString('UNIT_SUCCUBUS')	
	if CyGame().getGlobalCounter()>75:
		iUnitType = gc.getInfoTypeForString('UNIT_MANTICORE')	
		
	if iUnitType!=-1:
		newUnit = pPlayer.initUnit(iUnitType, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)	
		
def SummonMeshabberDis(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	pCity = pPlayer.getCapitalCity()
	iDemon = gc.getInfoTypeForString('UNIT_MESHABBER')

	bMeshabber=true

	py = PyPlayer(iPlayer)
	for pUnit in py.getUnitList():
		if pUnit.getUnitType()==iDemon:
				bMeshabber=false

	if bMeshabber:
		newUnit1 = pPlayer.initUnit(iDemon, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		
def SummonCthulthu(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)
	pCity = pPlayer.getCapitalCity()
	iDemon = gc.getInfoTypeForString('UNIT_CTHULTHU')

	bCthulthu=true

	py = PyPlayer(iPlayer)
	for pUnit in py.getUnitList():
		if pUnit.getUnitType()==iDemon:
				bCthulthu=false

	if bCthulthu:
		newUnit1 = pPlayer.initUnit(iDemon, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
