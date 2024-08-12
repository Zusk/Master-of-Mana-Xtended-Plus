import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

def	spellCallToArms(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pPlot = pCaster.plot()
	pCity = pPlot.getPlotCity()
	eConscript = pCity.getConscriptUnit()
	if (eConscript != -1):
		pPlayer.initUnit(eConscript, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

	return

def reqLightOfLugus(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pPlot = pCaster.plot()
	pOtherPlayer = gc.getPlayer(pPlot.getOwner())
	pCity = pPlot.getPlotCity()

	if pPlot.getTeam() == pCaster.getTeam():
		return false

#	if not pCity.isCapital():
#		return false
	if pOtherPlayer is None:
		return false

	if pOtherPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
		return false

	return true

def spellLightOfLugus(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pPlot = pCaster.plot()
	pOtherPlayer = gc.getPlayer(pPlot.getOwner())
	pCity = pPlot.getPlotCity()

	pOtherPlayer.AI_changeAttitudeExtra(pCaster.getOwner(), 2)
#	pOtherPlayer.AI_changeMemoryCount(pCaster.getOwner(), MemoryTypes.MEMORY_EVENT_GOOD_TO_US, 2)

def reqVisionary(pCaster):
	ePlayer = pCaster.getOwner()
	pPlayer = gc.getPlayer(ePlayer)
	pPlot = pCaster.plot()

	if pPlot.getOwner() != ePlayer:
		return false

	if pPlot.getImprovementType() == -1:
		return false

	#SpyFanatic: only if upgrade take more than 1 turn
	if pPlot.getUpgradeTimeLeft(pPlot.getImprovementType(), ePlayer) <= 1:
		return false

	return true

def spellVisionary(pCaster):
	ePlayer = pCaster.getOwner()
	pPlayer = gc.getPlayer(ePlayer)
	pPlot = pCaster.plot()

	pPlot.changeUpgradeProgress(20)
	return

def	spellLugusGuidance(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	pPlot = pCaster.plot()
	eGreatperson = -1
	iRand = CyGame().getSorenRandNum(5, "Lugus Guidance")
	if iRand == 0:
		eGreatperson = gc.getInfoTypeForString('UNIT_ARTIST')
	elif iRand == 1:
		eGreatperson = gc.getInfoTypeForString('UNIT_MERCHANT')
	elif iRand == 2:
		eGreatperson = gc.getInfoTypeForString('UNIT_SCIENTIST')
	elif iRand == 3:
		eGreatperson = gc.getInfoTypeForString('UNIT_ENGINEER')
	else:
		eGreatperson = gc.getInfoTypeForString('UNIT_PROPHET')

	if (eGreatperson != -1):
		pPlayer.initUnit(eGreatperson, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

	return

def	spellMiracleKilmorphProduction(pCaster):
	pCity = pCaster.plot().getPlotCity()

	pCity.changeProduction(60)

#SpyFanatic: spawn in caster plot rather then in random one,like fellowship of leaves Call to Nature
def reqSpellBountyOfKilmorph(caster):
	pPlot = caster.plot()
	if pPlot.isHills():
		if pPlot.getOwner()==caster.getOwner():
			if pPlot.getBonusType(-1)==-1:
				return True
	return False

def spellBountyOfKilmorph(caster):
	pPlayer = gc.getPlayer(caster.getOwner())

	pPlot = caster.plot()
	if pPlot.isHills():
		if pPlot.getOwner()==caster.getOwner():
			if pPlot.getBonusType(-1)==-1:
				lBonusList = []
				lBonusList = lBonusList + ['BONUS_IRON']
				lBonusList = lBonusList + ['BONUS_COPPER']
				lBonusList = lBonusList + ['BONUS_MITHRIL']
				lBonusList = lBonusList + ['BONUS_GEMS']
				lBonusList = lBonusList + ['BONUS_SILVER']
				lBonusList = lBonusList + ['BONUS_AMBER']

				sBonus = lBonusList[CyGame().getSorenRandNum(len(lBonusList), "Pick Bonus")]
				pPlot.setBonusType(gc.getInfoTypeForString(sBonus))

	#iMapPlots=CyMap().numPlots()

	#listPlots = []

	#for i in range(iMapPlots):
	#	pPlot = CyMap().plotByIndex(i)
	#	if pPlot.isHills():
	#		if pPlot.getOwner()==caster.getOwner():
	#			if pPlot.getBonusType(-1)==-1:
	#				listPlots.append(i)

	#if len(listPlots)>0:
	#	iRnd=CyGame().getSorenRandNum(len(listPlots), "Arrrgh")
	#	pPlot = CyMap().plotByIndex(listPlots[iRnd])
	#	lBonusList = []
	#	lBonusList = lBonusList + ['BONUS_IRON']
	#	lBonusList = lBonusList + ['BONUS_COPPER']
	#	lBonusList = lBonusList + ['BONUS_MITHRIL']
	#	lBonusList = lBonusList + ['BONUS_GEMS']
	#	lBonusList = lBonusList + ['BONUS_SILVER']
	#	lBonusList = lBonusList + ['BONUS_AMBER']

	#	sBonus = lBonusList[CyGame().getSorenRandNum(len(lBonusList), "Pick Bonus")]
	#	pPlot.setBonusType(gc.getInfoTypeForString(sBonus))
	#	return
