import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def spellDeathMarch(caster):
	py = PyPlayer(caster.getOwner())			
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iArcher = gc.getInfoTypeForString('UNITCOMBAT_ARCHER')
	iMelee = gc.getInfoTypeForString('UNITCOMBAT_MELEE')
	iRecon = gc.getInfoTypeForString('UNITCOMBAT_RECON')
	iMounted = gc.getInfoTypeForString('UNITCOMBAT_MOUNTED')
	iDeathMarch = gc.getInfoTypeForString('PROMOTION_DEATH_MARCH')
	
	for pUnit in py.getUnitList():
		if pUnit.isHasPromotion(iUndead):			
#			iUnitCombat=gc.getUnitInfo(pUnit.getUnitType()).getUnitCombatType()
#			if iUnitCombat==iArcher or iUnitCombat==iMelee or iUnitCombat==iRecon or iUnitCombat==iMounted:
			pUnit.setHasPromotion(iDeathMarch, True)							

def reqHealUD(caster):
	pPlot = caster.plot()
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isHasPromotion(iUndead)):
			if pUnit.getDamage() > 0:
				return True
	return False

def spellHealUD(caster,amount):
	pPlot = caster.plot()
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isHasPromotion(iUndead)):
			pUnit.changeDamage(-amount,0)
			