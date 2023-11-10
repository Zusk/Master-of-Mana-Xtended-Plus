import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#OnUnitCreated

def UnitCreated(argsList):
	'Unit Completed'
	unit = argsList[0]
	player = PyPlayer(unit.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())

	if gc.getTeam(unit.getOwner()).getProjectCount(gc.getInfoTypeForString("PROJECT_FORBIDDEN_EXPERIMENTS"))>0:
		iChaosMana = pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_MANA_CHAOS'))
		iChance = 50 + (10 * iChaosMana)

		if unit.isAlive():
			if CyGame().getSorenRandNum(100, "Forbidden Rituals") < iChance:
				unit.setHasPromotion(gc.getInfoTypeForString("PROMOTION_STRONG"),true)
				unit.setHasPromotion(gc.getInfoTypeForString("PROMOTION_MUTATED"),true)

def onPlayerTurn(argsList):
	'Called at the beginning of a players turn'
	iGameTurn  = argsList[0]
	iPlayer = argsList[1]
	pPlayer = gc.getPlayer(iPlayer)

	if gc.getTeam(iPlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_ARMAGEDDON"))>0:
#		if CyGame().getGameTurn() % 2==0:
		CyGame().changeGlobalCounter(1)

