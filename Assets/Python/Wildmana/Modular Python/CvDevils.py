import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

def SpawnDevilGates(argsList):
	'Tech Acquired'
	iTechType, iTeam, iPlayer, bAnnounce = argsList

	if iTechType==gc.getInfoTypeForString('TECH_INFERNAL_PACT'):
		iGatesNeeded=0
		if CyGame().getDevilGatesCreated()==0:
			iGatesNeeded+=CyMap().getWorldSize()
			if iGatesNeeded==0:
				iGatesNeeded=1

		for iGate in range(iGatesNeeded):
			SpawnDevilGate()

		if iGatesNeeded>0:
			for i in range(gc.getMAX_PLAYERS()):
				iEvent = gc.getInfoTypeForString("EVENTTRIGGER_DEVIL_WARNING")
				if gc.getPlayer(i).getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_SHEAIM'):
					iEvent = gc.getInfoTypeForString("EVENTTRIGGER_DEVIL_INFO_SHEAIM")
					gc.getTeam(gc.getDEVIL_TEAM()).makePeace(gc.getPlayer(i).getTeam())
				elif gc.getPlayer(i).getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_INFERNALS'):
					gc.getTeam(gc.getDEVIL_TEAM()).makePeace(gc.getPlayer(i).getTeam())
				gc.getPlayer(i).trigger(iEvent)

def SpawnDevilGate():
	iBestValue=0
	iValue=0
	pBestPlot=-1

	iDevilGate=gc.getInfoTypeForString('IMPROVEMENT_DEVIL_PORTAL')
	iDevils=gc.getInfoTypeForString('CIVILIZATION_DEVIL')
	iSheaim=gc.getInfoTypeForString('CIVILIZATION_SHEAIM')
	iInfernal=gc.getInfoTypeForString('CIVILIZATION_INFERNAL')

	pDevil = gc.getPlayer(gc.getDEVIL_PLAYER())
	if iDevils==-1:
		return

	for i in range (CyMap().numPlots()):
		iValue=20000
		pPlot = CyMap().plotByIndex(i)
		if pPlot.isWater() or pPlot.isPeak():
			continue
		if pPlot.getNumUnits()>0:
			continue
		if pPlot.isCity():
			continue
		if pPlot.getWilderness() < 10:
			continue
		if pPlot.getWilderness() > 20:
			iValue+=5000
		if not pPlot.getOwner()==-1:
			if gc.getPlayer(pPlot.getOwner()).getCivilizationType()==iSheaim or gc.getPlayer(pPlot.getOwner()).getCivilizationType()==iInfernal:
				continue
		if CyGame().getDevilGatesCreated()>0:
			for iiX in range(pPlot.getX()-20, pPlot.getX()+21, 1):
				for iiY in range(pPlot.getY()-20, pPlot.getY()+21, 1):
					pLoopPlot = CyMap().plot(iiX,iiY)
					if pLoopPlot.getImprovementType()==iDevilGate:
						iValue-=10000
						break

		if pPlot.area().getNumTiles()<20:
			iValue-=5000
		if pPlot.getOwner!=-1:
			iValue+=4000
		iValue+=CyGame().getSorenRandNum(3000,"Devil Gate")

		if iValue>iBestValue:
			iBestValue=iValue
			pBestPlot=pPlot

	if pBestPlot!=-1:
		pBestPlot.setImprovementType(iDevilGate)
		CyGame().changeDevilGatesCreated(1)
		pBestPlot.changePlotCounter(100)
		iUnitType = gc.getInfoTypeForString("UNIT_DEVIL_HEAVY_MELEE");
		newUnit=pDevil.initUnit(iUnitType,pBestPlot.getX(),pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
		newUnit.setOriginPlot(pBestPlot)
		newUnit=pDevil.initUnit(iUnitType,pBestPlot.getX(),pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
		newUnit.setOriginPlot(pBestPlot)

def reqCloseDevilGateGood(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pDevil = gc.getPlayer(gc.getDEVIL_PLAYER())

	if gc.getTeam(pPlayer.getTeam()).isHasMet(gc.getDEVIL_TEAM()):
		if not pPlayer.isBarbarian():
			return True

#	if pPlayer.getAlignment()==gc.getInfoTypeForString('ALIGNMENT_GOOD'):
#		return True
#	if pPlayer.getAlignment()==gc.getInfoTypeForString('ALIGNMENT_NEUTRAL'):
#		return True

	return False

def spellCloseDevilGateGood(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pPlot = pCaster.plot()

	pPlot.setImprovementType(-1)
#	CyEngine().removeLandmark(pPlot)
	pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_DEVIL_PORTAL_CLOSED'))
#	pPlayer.setAlignment(gc.getInfoTypeForString('ALIGNMENT_GOOD'))
	pPlayer.changeFaith(1000)
	pCaster.changeExperience(100, -1, false, false, false)

	CyGame().changeGlobalCounter(-20)

	for iiX in range(pPlot.getX()-20, pPlot.getX()+21, 1):
		for iiY in range(pPlot.getY()-20, pPlot.getY()+21, 1):
			pLoopPlot = CyMap().plot(iiX,iiY)
			pLoopPlot.changePlotCounter(-100)
			if(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS')):
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_GRASS'),True,True)
			elif(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')):
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),True,True)
			elif(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_SHALLOWS')):
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_TUNDRA'),True,True)
			elif(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')):
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),True,True)
			
def reqCloseDevilGateEvil(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())

	if gc.getTeam(pPlayer.getTeam()).isAtWar(gc.getDEVIL_TEAM()):
		return True
	if not pPlayer.isBarbarian():
		return True

	return False

def spellCloseDevilGateEvil(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pPlot = pCaster.plot()

	pPlayer.changeMana(1000)
	pPlot.setImprovementType(-1)
#	CyEngine().removeLandmark(pPlot)
	pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_DEVIL_PORTAL_CLOSED'))
	pCaster.changeExperience(100, -1, false, false, false)

def reqCloseDevilGateNeutral(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())

	if not pPlayer.isBarbarian():
		return True

	if gc.getTeam(pPlayer.getTeam()).isAtWar(gc.getDEVIL_TEAM()):
		return True

#	if pPlayer.getAlignment()==gc.getInfoTypeForString('ALIGNMENT_NEUTRAL'):
#		return True
#	if pPlayer.getAlignment()==gc.getInfoTypeForString('ALIGNMENT_EVIL'):
#		return True

	return 

def spellCloseDevilGateNeutral(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pPlot = pCaster.plot()

	pPlot.setImprovementType(-1)
#	CyEngine().removeLandmark(pPlot)
	pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_DEVIL_PORTAL_CLOSED'))
#	pPlayer.setAlignment(gc.getInfoTypeForString('ALIGNMENT_EVIL'))
	pPlayer.changeGoldenAgeTurns(CyGame().goldenAgeLength())
	pCaster.changeExperience(100, -1, false, false, false)
