import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def reqBrilliance(caster):
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iVampire = gc.getInfoTypeForString('PROMOTION_VAMPIRE')
	iWerewolf = gc.getInfoTypeForString('PROMOTION_WEREWOLF')
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = pPlayer.getTeam()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if (pUnit.isHasPromotion(iUndead) or pUnit.isHasPromotion(iVampire) or pUnit.isHasPromotion(iWerewolf)):
					if pPlayer.isHuman():	
						return True
					p2Player = gc.getPlayer(pUnit.getOwner())
					e2Team = gc.getTeam(p2Player.getTeam())
					if e2Team.isAtWar(eTeam):
						return True
	return False

def spellBrilliance(caster):
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iVampire = gc.getInfoTypeForString('PROMOTION_VAMPIRE')
	iWerewolf = gc.getInfoTypeForString('PROMOTION_WEREWOLF')
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				iRnd = CyGame().getSorenRandNum(10, "Brilliance") + caster.getLevel()
				if (pUnit.isHasPromotion(iUndead) or pUnit.isHasPromotion(iVampire) or pUnit.isHasPromotion(iWerewolf)):
					pUnit.doDamage(iRnd, 60, caster, gc.getInfoTypeForString('DAMAGE_FIRE'), true)