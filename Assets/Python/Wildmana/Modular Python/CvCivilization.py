import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#Amurites

def AmuriteLeaderStartResearch(argsList):
	'Called at the start of the game'
	for iPlayer in range(gc.getMAX_CIV_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		pTeam = gc.getTeam(pPlayer.getTeam())

		if not pPlayer.isAlive():
			continue
		bValid = false
		
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_LAW_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_LIFE')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_EARTH_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_NATURE')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_AIR_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_AIR')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FIRE_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_FIRE')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)
			
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_WATER_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_ICE')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)
			
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_MIND_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_MIND')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_DEATH_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_DEATH')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SHADOW_MAGIC_MASTERY')):
			iBonus = gc.getInfoTypeForString('BONUS_MANA_CHAOS')
			pPlayer.setManaBonus2(iBonus)
			pPlayer.setManaBonus3(iBonus)
			pPlayer.setManaBonus4(iBonus)

#BALSERAPH

def doCityBuilt(argsList):
	city = argsList[0]
	iPlayer = city.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

#	if pPlayer.getLeaderType()==gc.getInfoTypeForString('LEADER_NOJAH'):
#		if pPlayer.getNumCities()==1:
#			if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_NOJAH'))==0:
#				newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_NOJAH'), pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
#				newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ILLUSION'),false)

	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SPRAWLING')):
		city.changeCulture(iPlayer, 250, true)

#BANNOR

def BannorFollowerType(iPlayer):
	pPlayer = gc.getPlayer(iPlayer)

	if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_MYSTICISM')):
		return gc.getInfoTypeForString('UNIT_DEMAGOG')
	if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_PRIESTHOOD')):
		return gc.getInfoTypeForString('UNIT_DEMON_HUNTER')
	if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_RIGHTEOUSNESS')):
		return gc.getInfoTypeForString('UNIT_HUMAN_HEAVY_SWORDSMAN')

	return gc.getInfoTypeForString('UNIT_FANATIC')

def BannorGainFollowers(argsList):
	'City Acquired'
	iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
	pPlayer = gc.getPlayer(iNewOwner)

	if not bConquest:
		return
	if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_BANNOR'):
		return

	iFollower = BannorFollowerType(iNewOwner)
	if iFollower != -1:
		newUnit = pPlayer.initUnit(iFollower, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		CyInterface().addMessage(iNewOwner,true,25,"A soldier joins the cause of the Bannor!",'',0,'',ColorTypes(11), pCity.getX(), pCity.getY(), True,True)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_GREAT_LEADER_BANNOR')):
			newUnit = pPlayer.initUnit(iFollower, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			CyInterface().addMessage(iNewOwner,true,25,"Because of your magnificent Leadership another soldier joins the cause of the Bannor!",'',0,'',ColorTypes(11), pCity.getX(), pCity.getY(), True,True)
						


def onAristrakhProjectBuilt(argsList):
	'Project Completed'
	pCity, iProjectType = argsList
	iPlayer = pCity.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = pCity.plot()

#	if iProjectType == gc.getInfoTypeForString('PROJECT_'):
#		pPlayer.setCivilizationType(gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'))
