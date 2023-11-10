import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#GRIGORI
def doTraitDiverse(argsList):
	'Unit Completed'
	city = argsList[0]
	unit = argsList[1]
	player = PyPlayer(city.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())

	if not pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_DIVERSE')):
		return
		
	if unit.getRace() == -1:
		iChance = 40 
		if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_MUSEUM')) > 0:
			iChance = 70 
		if CyGame().getSorenRandNum(100, "Bob") <= iChance: 
			race = CyGame().getSorenRandNum(3, "Bob")
			if race == 0 and unit.isAlive():
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORC'), True)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_SCOUT'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GOBLIN'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_HUNTER'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LIZARDMAN'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_HORSEMAN'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WOLF_RIDER'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(False, PlayerTypes.NO_PLAYER)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_ADEPT'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SHAMAN'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORC'), True)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_ASSASSIN'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LIZARDMAN_ASSASSIN'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_RANGER'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LIZARDMAN_RANGER'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_BEASTMASTER'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LIZARDMAN_BEASTMASTER'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_PHALANX'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_STONESKIN_OGRE'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_IMMORTAL'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_OGRE_WARCHIEF'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
			elif race == 1 and unit.isAlive():
				race = CyGame().getSorenRandNum(2, "Bob")
				if race == 0:
					unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ELF'), True)
					if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_AXEMAN'):
						newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SWORDSMAN'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ELF'), True)
						city.applyBuildEffects(newUnit)
						unit.kill(True,0)
					if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_HORSE_ARCHER'):
						newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_FYRDWELL'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						city.applyBuildEffects(newUnit)
						unit.kill(True,0)
				elif race == 1:
					unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DARK_ELF'), True)
					if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_AXEMAN'):
						newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SWORDSMAN'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DARK_ELF'), True)
						city.applyBuildEffects(newUnit)
						unit.kill(True,0)
					if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_HORSE_ARCHER'):
						newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_NYXKIN'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						city.applyBuildEffects(newUnit)
						unit.kill(True,0)
			elif race == 2:
				if (unit.isAlive()):
					unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DWARF'), True)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_CANNON'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DWARVEN_CANNON'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_ARCHER'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DWARVEN_SLINGER'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_KNIGHT'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_HORNGUARD'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_HORSEMAN'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_BOAR_RIDER'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_SHADOW'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DWARVEN_SHADOW'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)
				if unit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_BEASTMASTER'):
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MYCONID'), unit.getX(), unit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					city.applyBuildEffects(newUnit)
					unit.kill(True,0)

def GrigoriHero(argsList):
	'Combat Result'
	pWinner,pLoser = argsList
	pPlayerW = gc.getPlayer(pWinner.getOwner())
	pPlayerL = gc.getPlayer(pLoser.getOwner())

	if not pPlayerW.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_GRIGORI'):
		return
	if not pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HERO')):
		return
	if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_LEGENDARY_STATUS')):
		return

	if isWorldUnitClass(pLoser.getUnitClassType()):
		pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_LEGENDARY_STATUS'),true)
		newUnit = pPlayerW.initUnit(gc.getInfoTypeForString('UNIT_ADVENTURER'), pPlayerW.getCapitalCity().getX(), pPlayerW.getCapitalCity().getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

	if pWinner.getLevel()>14:
		pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_LEGENDARY_STATUS'),true)
		if CyGame().getSorenRandNum(100, "New Adventurer") <= 30:
			newUnit = pPlayerW.initUnit(gc.getInfoTypeForString('UNIT_ADVENTURER'), pPlayerW.getCapitalCity().getX(), pPlayerW.getCapitalCity().getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)


