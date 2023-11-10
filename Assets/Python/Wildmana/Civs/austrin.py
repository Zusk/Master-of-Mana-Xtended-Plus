import CvUtil
from CvPythonExtensions import *

import CvEventManager
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def reqTempest(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		iTeam = gc.getPlayer(caster.getOwner()).getTeam()
		eTeam = gc.getTeam(iTeam)
		if eTeam.getAtWarCount(True) < 2:
			return False
	return True
	
def spellTempest(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iAirElementalChance = 9
	iAirElly = gc.getInfoTypeForString('UNIT_AIR_ELEMENTAL')
	iTeam = pPlayer.getTeam()	

	
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		iOwner = pPlot.getOwner()
		
		if iOwner == iPlayer:
			if pPlot.getNumUnits() == 0:
				if CyGame().getSorenRandNum(100, "AirEllySpawn") < iAirElementalChance:
					newUnit = pPlayer.initUnit(iAirElly, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
					newUnit.setDuration(CyGame().getSorenRandNum(5, "AirEllySpawn") + 1) 
			else:
				for i in range(pPlot.getNumUnits()):
					pUnit = pPlot.getUnit(i)
					iTeam2 = gc.getTeam(gc.getPlayer(pUnit.getOwner()).getTeam())
					if iTeam2.isAtWar(iTeam):
						pUnit.setImmobileTimer(CyGame().getSorenRandNum(5, "AirEllySpawn") + 1)

def reqPegasus(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	
	if pPlayer.isBarbarian():
		return false

	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_PEGASUS_RIDER')):
		return False

		
	return True
						
def spellMountPegasus(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	iPegasus = gc.getInfoTypeForString('UNIT_PEGASUS')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (caster.getOwner() == pUnit.getOwner() and pUnit.getUnitType() == iPegasus):
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM'), True)
			pUnit.kill(False,0)
			return true
	
	return false

def reqAustrinRemoteFound(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())

	if not pPlayer.isFeatAccomplished(FeatTypes.FEAT_EXPEDITION_READY):
		return false
		
	if pPlot.isOwned() and pPlot.getOwner() != caster.getOwner():
		return false

	iX = pPlot.getX()
	iY = pPlot.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlotLoop = CyMap().plot(iiX,iiY)
			if pPlotLoop!=pPlot and pPlotLoop.isCity():
				return false

		
	if pPlot.isCityRadius():
		return false
		
	if pPlot.isWater():
		return false

	if not pPlayer.isHuman():
		if pPlot.getFoundValue() < (pPlot.area().getBestFoundValue() * 2) / 3:
			return false

	return true

def spellAustrinRemoteFound(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())

	pCity = pPlayer.initCity(pPlot.getX(),pPlot.getY())
	CvEventManager.CvEventManager().onCityBuilt([pCity])
	pPlayer.setFeatAccomplished(FeatTypes.FEAT_EXPEDITION_READY, false)
	
def reqStartSwapCities(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()

	if not pPlayer.isHuman():
		return false
	
	if not pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_NOMADS')):
		return false
	
	if pPlayer.getNumCities()<2:
		return false
	
	if not pPlot.isCity():
		return false

	if not pPlot.getOwner()==caster.getOwner():
		return false
		
	if pPlot.getPlotCity().getPopulation()<5:
		return false
		
	iSwap = gc.getInfoTypeForString('PROMOTION_PREPARE_TO_SWAP_CITY')
		
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.isHasPromotion(iSwap):
			return false
			
	py = PyPlayer(caster.getOwner())			
	count=0
	for pUnit in py.getUnitList():
		if pUnit.isHasPromotion(iSwap):
			count+=1
		if count>1:
			return false
			
	return true

def spellStartSwapCities(caster):	

	py = PyPlayer(caster.getOwner())			
	count=0
	pUnit1=-1
	pUnit2=-1

	iPrep = gc.getInfoTypeForString('PROMOTION_PREPARE_TO_SWAP_CITY')
	caster.setHasPromotion(iPrep, true)
	
	for pUnit in py.getUnitList():
		if pUnit.isHasPromotion(iPrep):
			count+=1
			if count==1:
				pUnit1=pUnit
			if count==2:
				pUnit2=pUnit
	
	if not pUnit2==-1:
		pUnit1.setHasPromotion(iPrep,false)
		pUnit2.setHasPromotion(iPrep,false)
		pCity1=pUnit1.plot().getPlotCity()
		pCity2=pUnit2.plot().getPlotCity()
		
		pCity1.swapAustrinCities(pUnit2.plot())
		
# added by BlackArchon
# We need this check to be sure that Sorah is casting this only once
def reqJoining(caster):
	if PyPlayer(caster.getOwner()).getNumCities() < 1:
		return true
	return false
# end added by BlackArchon

def spellJoining(caster):

	pPlot = caster.plot()
	
	if pPlot.isCity():
		pCity = pPlot.getPlotCity()
		ePlayer = caster.getOwner()
		pPlayer = gc.getPlayer(caster.getOwner())
		eFromPlayer = pPlot.getOwner()
		pPlayer.acquireCity(pCity,false,false)
		pCity = pPlot.getPlotCity()	#pointer must be set again after acquireCity
		CyInterface().setDirty(InterfaceDirtyBits.CityInfo_DIRTY_BIT, True)
		pCity.changeCulture(caster.getOwner(), 300, True)
		
		#transfer culture
		isearch=1
		for iiX in range(pCity.getX()-isearch, pCity.getX()+isearch+1, 1):
			for iiY in range(pCity.getY()-isearch, pCity.getY()+isearch+1, 1):
				pPlot = CyMap().plot(iiX,iiY)
				if not pPlot.isNone():						
					iCultureTransfer=-pPlot.getCulture(eFromPlayer)
					pPlot.changeCulture(eFromPlayer, iCultureTransfer, false);
					pPlot.changeCulture(ePlayer, iCultureTransfer, true);
		