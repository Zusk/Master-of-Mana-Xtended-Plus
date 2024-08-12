import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def doCivicBanditLords(ePlayer,eCivic):
	pPlayer = gc.getPlayer(ePlayer)

	eBandit = gc.getInfoTypeForString('UNIT_BANDIT_LORD')
	eBanditClass = gc.getInfoTypeForString('UNITCLASS_BANDIT_LORD')

	if eBandit != -1 and eBanditClass != -1:
		iBanditsNeeded = pPlayer.getNumCities()-pPlayer.getUnitClassCount(eBanditClass)
		if iBanditsNeeded>0:
			iRnd = CyGame().getSorenRandNum(100, "Bandit Lords")
			if iRnd<iBanditsNeeded*3:

				iMapPlots=CyMap().numPlots()
				iPossiblePlots=0
				for i in range(0,iMapPlots,1):
					pPlot = CyMap().plotByIndex(i)
					if not pPlot.isNone():
						if not pPlot.isWater():
							if not pPlot.isPeak():
								if pPlot.getOwner()==ePlayer:
									if pPlot.getNumUnits()==0:
										iPossiblePlots=iPossiblePlots+1

				iRnd = CyGame().getSorenRandNum(iPossiblePlots, "Pick Bandit Lords Spawn Plot")
				iCountPlots=-1
				for i in range(0,iMapPlots,1):
					pPlot = CyMap().plotByIndex(i)
					if not pPlot.isNone():
						if not pPlot.isWater():
							if not pPlot.isPeak():
								if pPlot.getOwner()==ePlayer:
									if pPlot.getNumUnits()==0:
										iCountPlots=iCountPlots+1
										if iCountPlots==iRnd:
											newUnit = pPlayer.initUnit(eBandit, pPlot.getX(), pPlot.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
											newUnit.changeAIControl(1)
											return

# obsolete - done in the DLL
# def doCivicPreservation(ePlayer,eTrait):
	# pPlayer = gc.getPlayer(ePlayer)

	# iMapPlots=CyMap().numPlots()
	# iPossiblePlots=0
	# for i in range(0,iMapPlots,1):
		# pPlot = CyMap().plotByIndex(i)
		# if not pPlot.isNone():
			# if not pPlot.isWater():
				# if not pPlot.isPeak():
					# if pPlot.getOwner()==ePlayer:
						# if pPlot.getNumUnits()==0:
							# iPossiblePlots=iPossiblePlots+1

	# iRnd = CyGame().getSorenRandNum(iPossiblePlots, "Pick Bandit Lords Spawn Plot")
	# iCountPlots=-1
	# for i in range(0,iMapPlots,1):
		# pPlot = CyMap().plotByIndex(i)
		# if not pPlot.isNone():
			# if not pPlot.isWater():
				# if not pPlot.isPeak():
					# if pPlot.getOwner()==ePlayer:
						# if pPlot.getNumUnits()==0:
							# iCountPlots=iCountPlots+1
							# if iCountPlots==iRnd:
								# if pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_DESERT'):
									# if pPlot.getFeatureType()!=gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS'):
										# pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),true,true)
								# if pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_SNOW'):
									# pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_TUNDRA'),true,true)
								# return

def doTraitArcaneMastery(ePlayer,eTrait):

	pPlayer = gc.getPlayer(ePlayer)
	eTeam = gc.getTeam(pPlayer.getTeam())

	bGovannon = pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_GOVANNON'))

	if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_CHAMPION')) > 0 or bGovannon > 0:
		for i in range(pPlayer.getNumUnits()):
			pUnit = pPlayer.getUnit(i)
			if pUnit.getUnitClassType()==gc.getInfoTypeForString('UNITCLASS_CHAMPION'):
				if pUnit.getLevel()>2:
					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GUARDSMAN'), True)
					if pUnit.getLevel()>5:
						pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING2'), True)
			if bGovannon > 0:
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_KYLORIN_LEGACY'),true)


def doTraitUrbane(ePlayer,eTrait):
	pPlayer = gc.getPlayer(ePlayer)

	if pPlayer.getNumCities() > 0:
		for pyCity in PyPlayer(ePlayer).getCityList():
			iBuilding1 = gc.getInfoTypeForString('BUILDING_GRANDEUR1')
			iBuilding2 = gc.getInfoTypeForString('BUILDING_GRANDEUR2')
			iBuilding3 = gc.getInfoTypeForString('BUILDING_GRANDEUR3')
			iBuilding4 = gc.getInfoTypeForString('BUILDING_GRANDEUR4')
			pCity = pyCity.GetCy()
			iCycle = 1
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
				iCycle = .75
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
				iCycle = 1.5
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
				iCycle = 3
			iCulture = pCity.getCulture(ePlayer)/iCycle
			if (iCulture >= 149 and iCulture <= 499):
				iNewBuilding = iBuilding1
			if (iCulture >= 500 and iCulture <= 1999):
				iNewBuilding = iBuilding2
			if (iCulture >= 2000 and iCulture <= 4499):
				iNewBuilding = iBuilding3
			if (iCulture >= 4500):
				iNewBuilding = iBuilding4
			pCity.setNumRealBuilding(iBuilding1, 0)
			pCity.setNumRealBuilding(iBuilding2, 0)
			pCity.setNumRealBuilding(iBuilding3, 0)
			pCity.setNumRealBuilding(iBuilding4, 0)
			if (iCulture>=149):
				pCity.setNumRealBuilding(iNewBuilding, 1)

### Buildings


### Buildings

#Balseraph

def postCombatShapechange(pCaster,pOpponent):

	attackDiff = pOpponent.getRawBaseCombatStr() - pCaster.getRawBaseCombatStr()
	defenseDiff = pOpponent.getRawBaseCombatStrDefense() - pCaster.getRawBaseCombatStrDefense()

#	if attackDiff > 0:
#		pCaster.changeBaseCombatStr(attackDiff)
#		pCaster.setUnitArtStyleType(pOpponent.getUnitArtStyleType())
#	if defenseDiff > 0:
#		pCaster.changeBaseCombatStrDefense(defenseDiff)
#		pCaster.setUnitArtStyleType(pOpponent.getUnitArtStyleType())

#Doviello

def reqDovielloChallenge(caster):
	pPlot = caster.plot()
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ANIMAL') or caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_NAVAL'):
		return False
	if not caster.isAlive():
		return False
	if caster.isOnlyDefensive():
		return False
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getOwner()==caster.getOwner():
			if pUnit.getID() != caster.getID():
				if pUnit.getLevel()>(caster.getLevel()-2) and pUnit.getLevel()<(caster.getLevel()+2):
					if not pUnit.isOnlyDefensive():
						return True

	return False

def spellDovielloChallenge(caster):
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		iRnd = CyGame().getSorenRandNum(100, "Doviello Duel")
		if pUnit.getOwner()==caster.getOwner():
			if pUnit.getID() != caster.getID():
				if pUnit.getLevel()>(caster.getLevel()-2) and pUnit.getLevel()<(caster.getLevel()+2):
					if not pUnit.isOnlyDefensive():
						iChallengerOdds = getCombatOdds(pUnit, caster)
						iChallengerRnd = (iRnd + (iChallengerOdds * 2)) / 3
						iDefenderOdds = getCombatOdds(caster, pUnit)
						iDefenderRnd = (iRnd + (iDefenderOdds * 2)) / 3
						if iDefenderOdds > iChallengerOdds:
							iDefenseRnd = CyGame().getSorenRandNum(100, "Doviello Duel Defender")
							iDefenseTieRnd = CyGame().getSorenRandNum(100, "Doviello Duel Defender Tie")
							if iDefenseTieRnd < 10:
								caster.changeExperience(pUnit.getExperience() / 10, -1, False, False, False)
								caster.setDamage(75, caster.getOwner())
								pUnit.changeExperience(caster.getExperience() / 10, -1, False, False, False)
								pUnit.setDamage(75, pUnit.getOwner())
								CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_DRAW", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
							elif (iDefenseTieRnd <= 12 and iDefenseTieRnd >= 10):
								iVictorRnd = CyGame().getSorenRandNum(100, "Doviello Duel")
								if iVictorRnd < 50:
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_DEATH_DEFENDER", ()),'',1,'',ColorTypes(7),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
								else:
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_DEATH_CHALLENGER", ()),'',1,'',ColorTypes(7),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
							elif iDefenseTieRnd > 12:
								if iDefenseRnd > iDefenderOdds:
									pUnit.changeExperience((caster.getExperience() / 2) + 2, -1, False, False, False)
									pUnit.setDamage(25, pUnit.getOwner())
									pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHALLENGER'), False)
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_LOSS", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
								if iDefenseRnd < iDefenderOdds:
									caster.changeExperience((pUnit.getExperience() / 2) + 2, -1, False, False, False)
									caster.setDamage(25, caster.getOwner())
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_WIN", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
						if iChallengerOdds > iDefenderOdds:
							iChallengeRnd = CyGame().getSorenRandNum(100, "Doviello Duel Challenger")
							iChallengerTieRnd = CyGame().getSorenRandNum(100, "Doviello Duel Challenger Tie")
							if iChallengerTieRnd < 10:
								caster.changeExperience(pUnit.getExperience() / 10, -1, False, False, False)
								caster.setDamage(75, caster.getOwner())
								pUnit.changeExperience(caster.getExperience() / 10, -1, False, False, False)
								pUnit.setDamage(75, pUnit.getOwner())
								CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_DRAW", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
							elif (iChallengerTieRnd <= 12 and iChallengerTieRnd >= 10):
								iVictorRnd = CyGame().getSorenRandNum(100, "Doviello Duel")
								if iVictorRnd < 50:
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_DEATH_DEFENDER", ()),'',1,'',ColorTypes(7),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
								else:
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_DEATH_CHALLENGER", ()),'',1,'',ColorTypes(7),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
							elif iChallengerTieRnd > 12:
								if iChallengeRnd < iChallengerOdds:
									pUnit.changeExperience((caster.getExperience() / 2) + 2, -1, False, False, False)
									pUnit.setDamage(25, pUnit.getOwner())
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_LOSS", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
								if iChallengeRnd > iChallengerOdds:
									caster.changeExperience((pUnit.getExperience() / 2) + 2, -1, False, False, False)
									caster.setDamage(25, caster.getOwner())
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_WIN", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
						if iChallengerOdds == iDefenderOdds:
							iEqualRnd = CyGame().getSorenRandNum(100, "Doviello Duel Equals")
							iEqualTieRnd = CyGame().getSorenRandNum(100, "Doviello Duel Equals Tie")
							if iEqualTieRnd < 10:
								caster.changeExperience(pUnit.getExperience() / 10, -1, False, False, False)
								caster.setDamage(75, caster.getOwner())
								pUnit.changeExperience(caster.getExperience() / 10, -1, False, False, False)
								pUnit.setDamage(75, pUnit.getOwner())
								CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_DRAW", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
							elif (iEqualTieRnd <= 12 and iEqualTieRnd >= 10):
								iVictorRnd = CyGame().getSorenRandNum(100, "Doviello Duel")
								if iVictorRnd < 50:
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_DEATH_DEFENDER", ()),'',1,'',ColorTypes(7),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
								else:
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_DEATH_CHALLENGER", ()),'',1,'',ColorTypes(7),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
							elif iEqualTieRnd > 12:
								if iEqualRnd < 50:
									pUnit.changeExperience((caster.getExperience() / 2) + 2, -1, False, False, False)
									pUnit.setDamage(25, pUnit.getOwner())
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_LOSS", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
									caster.kill(True, PlayerTypes.NO_PLAYER)
								if iEqualRnd >= 50:
									caster.changeExperience((pUnit.getExperience() / 2) + 2, -1, False, False, False)
									caster.setDamage(25, caster.getOwner())
									CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOVIELLO_DUEL_WIN", ()),'',1,'',ColorTypes(8),caster.getX(),caster.getY(),True,True)
									pUnit.kill(True, PlayerTypes.NO_PLAYER)
						gc.getPlayer(caster.getOwner()).changeMana(75+CyGame().getSorenRandNum(75, "Doviello Mana from Duel"))
						break

def reqEnlightenCity(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	for iTarget in range(gc.getNumReligionInfos()):
		if (pCity.isHasReligion(iTarget) and pCity.isHolyCityByType(iTarget) == False):
			return True
	if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_HAND')) > 0:
		return True
	return False

def spellEnlightenCity(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	iRnd = CyGame().getSorenRandNum(4, "Bob")
	for iTarget in range(gc.getNumReligionInfos()):
		if (pCity.isHasReligion(iTarget) and not pCity.isHolyCityByType(iTarget)):
			pCity.setHasReligion(iTarget, False, True, True)
			iRnd = iRnd + 1
			for i in range(gc.getNumBuildingInfos()):
				if gc.getBuildingInfo(i).getPrereqReligion() == iTarget:
					pCity.setNumRealBuilding(i, 0)
	if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_HAND')):
		pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_HAND'), 0)
		iRnd = iRnd + 1
	if iRnd >= 1:
		pCity.changeHurryAngerTimer(iRnd)
	if CyGame().getSorenRandNum(100, "Museum New Adventurer") <= pCity.getPopulation():
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ADVENTURER'), pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def reqAsceticism(caster):
	if caster.plot().getImprovementType()!=-1:
		return false
	if caster.plot().getNumUnits()>1:
		return false
	return true

def spellAsceticism(caster):
	iXPGained = CyGame().getSorenRandNum(10, "Asceticism")
	if caster.getUnitType()==gc.getInfoTypeForString('UNIT_CHALID'):
		iXPGained = iXPGained * 2
	caster.setExperience(caster.getExperience() + iXPGained, -1)

def reqPathToAsceticism(caster):
	if caster.plot().getImprovementType()!=-1:
		return false
	if caster.plot().getNumUnits()>3:
		return false
	return true

def spellPathToAsceticism(caster):
	iXPGained = (10 * CyGame().getSorenRandNum(10, "Asceticism"))
	caster.setExperience(caster.getExperience() + iXPGained, -1)

def reqInfiltrateCityGuard(caster):
	pCity = caster.plot().getPlotCity()

	if pCity.getCrime() > 29:
		return false
	
	if caster.plot().getTeam() == caster.getTeam():
		return false

	return true

def spellInfiltrateCityGuard(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())

	iCrimeChange = CyGame().getSorenRandNum(20, "InfiltrateCityGuardEffect")
	pCity.changeCrime(iCrimeChange)
	CyInterface().addMessage(caster.getOwner(),true,25,"Your Enforcer managed to infiltrate the CityGuard! Crime in the city increased by %s%%." %(iCrimeChange),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)

	return

def reqPoisonWater(caster):
	pCity = caster.plot().getPlotCity()

	if pCity.getEspionageHealthCounter() > 0:
		return false

	if caster.plot().getTeam()==caster.getTeam():
		return false

	return true

def spellPoisonWater(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())

	iMissionStrength = 4 + caster.getLevel()

	iUnhealth = 4 + CyGame().getSorenRandNum(iMissionStrength, "PoisonWaterEffect")
	pCity.changeEspionageHealthCounter(iUnhealth)
	CyInterface().addMessage(caster.getOwner(),true,25,"Your Enforcer managed to poison the water! Unhealth in the City increased by %d" %(iUnhealth),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)

	return

def goldToSteal(caster):
	return 10 + caster.getLevel() * caster.getLevel()
	
def reqStealGold(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())
	if pPlayer is None:
		return false

	if pPlayer.getGold() < goldToSteal(caster):
		return false

	if caster.plot().getTeam() == caster.getTeam():
		return false

	return true

def spellStealGold(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())

	iMissionStrength = goldToSteal(caster)

	pPlayer.changeGold(-iMissionStrength)
	gc.getPlayer(caster.getOwner()).changeGold(iMissionStrength)
		
	CyInterface().addMessage(caster.getOwner(),true,25,"Your Thief managed to steal %d Gold from the local citizens." %(iMissionStrength),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)
	CyInterface().addMessage(pCity.getOwner(),true,25,"An enemy Thief stole %d Gold." %(iMissionStrength),'',0,'',ColorTypes(11), pCity.getX(), pCity.getY(), True,True)	

	return
	
	
def reqAssasinateGovernour(caster):
	pCity = caster.plot().getPlotCity()

	if pCity.getOccupationTimer() > 0:
		return false

	if caster.plot().getTeam() == caster.getTeam():
		return false

	return true

def spellAssasinateGovernour(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())

	iMissionStrength = 3 + caster.getLevel()

	iRevoltTurns = CyGame().getSorenRandNum(iMissionStrength, "AssasinateGovernourEffect")
	pCity.changeOccupationTimer(iRevoltTurns)
	pCity.changeCultureUpdateTimer(iRevoltTurns)
	CyInterface().addMessage(caster.getOwner(),true,25,"Your Enforcer managed to assasinate the Governour! City is in revolt for %d turns" %(iRevoltTurns),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)

	return

def reqStealMaps(caster):
	pCity = caster.plot().getPlotCity()

	if caster.plot().getTeam() == caster.getTeam():
		return false

	return true

def spellStealMaps(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())

	eTeam = caster.getTeam()
	eTargetTeam = pCity.getTeam()

	iCapitalOffset = 0
	if pCity.isCapital():
		iCapitalOffset = 10

	iMissionStrength = 3 + caster.getLevel() + iCapitalOffset / 3

	iX = caster.getX()
	iY = caster.getY()

	for iiX in range(iX-iMissionStrength, iX+iMissionStrength, 1):
		for iiY in range(iY-iMissionStrength, iY+iMissionStrength, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				if pPlot.isRevealed(eTargetTeam, false):
					pPlot.setRevealed(eTeam, true, false, -1)

	CyInterface().addMessage(caster.getOwner(),true,25,"Your Enforcer managed to steal valuable Maps of the surrounding Area from a Distance of %d!" %(iMissionStrength),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)

	return

def canStealTech(Tech, pPlayer, pTargetTeam):
	if pPlayer.canResearch(Tech, true):
		if pTargetTeam.hasMasteredTech(Tech):
			if not gc.getTeam(pPlayer.getTeam()).isHasTech(Tech):
				return true
	return false

def reqStealKnowledge(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())

	if caster.plot().getTeam() == caster.getTeam():
		return false

	pTargetTeam = gc.getTeam(pCity.getTeam())
	pTeam = gc.getTeam(caster.getTeam())

	for i in range(gc.getNumTechInfos()):
		if canStealTech(i, pPlayer, pTargetTeam):
			return true

	return false

def spellStealKnowledge(caster):
	pCity = caster.plot().getPlotCity()
	pTargetTeam = gc.getTeam(pCity.getTeam())
	pPlayer = gc.getPlayer(caster.getOwner())

	TechsToSteal = []

	for i in range(gc.getNumTechInfos()):
		if canStealTech(i, pPlayer, pTargetTeam):
			TechsToSteal.append(i)

	iPickTech = gc.getGame().getSorenRandNum(len(TechsToSteal), "Pick Tech to steal")
	iTechStolen = TechsToSteal[iPickTech]
	gc.getTeam(pPlayer.getTeam()).setHasTech(iTechStolen, true, -1, false, true)

	CyInterface().addMessage(caster.getOwner(),true,25,"Your Enforcer managed to steal Knowledge of %s" %(gc.getTechInfo(iTechStolen).getDescription()),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)
	return

def reqCorruption(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())

	if caster.plot().getTeam() == caster.getTeam():
		return false

	StateBelief = pPlayer.getStateReligion()
	if StateBelief == -1:
		if caster.getOwner() != pCity.getOwner():
			return False
	for iTarget in range(gc.getNumReligionInfos()):
		if (StateBelief != iTarget and pCity.isHasReligion(iTarget) and pCity.isHolyCityByType(iTarget) == False):
			return True
	return False

def spellCorruption(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())
	StateBelief = gc.getPlayer(caster.getOwner()).getStateReligion()

	for iTarget in range(gc.getNumReligionInfos()):
		if (not StateBelief == iTarget and pCity.isHasReligion(iTarget) and not pCity.isHolyCityByType(iTarget)):
			pCity.setHasReligion(iTarget, False, True, True)
			CyInterface().addMessage(caster.getOwner(),true,25,"Your Enforcer managed to assasinate priests of Religion %s" %(gc.getReligionInfo(iTarget).getDescription()),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)

	return

def reqInciteRevolt(caster):
	pCity = caster.plot().getPlotCity()

	if caster.plot().getTeam() == caster.getTeam():
		return false

	if pCity.isCapital():
		return false

	return true

def spellInciteRevolt(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())

	CyInterface().addMessage(caster.getOwner(),true,25,"Your Enforcer managed to incite a revolt! The City is now in our control" %(),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True,True)
	gc.getPlayer(caster.getOwner()).acquireCity(pCity,false,false)

	return

def reqSabotageRitual(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())
	
	if caster.plot().getTeam() == caster.getTeam():
		return false

	if pPlayer.getCurrentMagicRitual() == -1:
		return false
		
	iBuildingClass = gc.getInfoTypeForString("BUILDINGCLASS_MAGE_GUILD")
	iBuilding = gc.getCivilizationInfo(pCity.getCivilizationType()).getCivilizationBuildings(iBuildingClass)
	if iBuilding == -1:
		return false

	if pCity.getNumRealBuilding(iBuilding) == 0:
		return false

	return true

def spellSabotageRitual(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(pCity.getOwner())

	sRitualName = gc.getProjectInfo(pPlayer.getCurrentMagicRitual()).getDescription()
	CyInterface().addMessage(caster.getOwner(), true, 25, "Your Enforcer managed to sabotage the Ritual %s!" %(sRitualName),'',0,'',ColorTypes(11), caster.getX(), caster.getY(), True, True)

	pPlayer.setCurrentMagicRitual(-1)
	
	return
	
def postCombatExplode(pCaster, pOpponent):

	fireSpreadToTile(pCaster.getX(), pCaster.getY())
	fireSpreadToTile(pOpponent.getX(), pOpponent.getY())

	pOpponent.doDamage(20, 100, pCaster, gc.getInfoTypeForString('DAMAGE_FIRE'), false)
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SHEUT_STONE')):
		pOpponent.doDamage(20, 100, pCaster, gc.getInfoTypeForString('DAMAGE_UNHOLY'), false)

def fireSpreadToTile(iX, iY):
	pPlot = CyMap().plot(iX,iY)

	iFeature = pPlot.getFeatureType()

	if iFeature == -1:
		return

	if (iFeature == gc.getInfoTypeForString('FEATURE_FOREST') or iFeature == gc.getInfoTypeForString('FEATURE_JUNGLE') or iFeature == gc.getInfoTypeForString('FEATURE_FOREST_NEW')):
		if CyGame().getSorenRandNum(100, "Flames Spread") <= gc.getDefineINT('FLAMES_SPREAD_CHANCE'):
			bValid = True
			iImprovement = pPlot.getImprovementType()
			if iImprovement != -1 :
				if gc.getImprovementInfo(iImprovement).isPermanent() :
					bValid = False
			if bValid:
				pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_SMOKE'))

def reqRevealDungeons(caster):

	iRuins = gc.getInfoTypeForString('IMPROVEMENT_RUINS')
	iDungeon = gc.getInfoTypeForString('IMPROVEMENT_DUNGEON')
	iBarrow = gc.getInfoTypeForString('IMPROVEMENT_BARROW')

	iX = caster.getX()
	iY = caster.getY()

	for iiX in range(iX-30, iX+30, 1):
		for iiY in range(iY-30, iY+30, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				iImp = pPlot.getImprovementType()
				if iImp == iRuins or iImp == iDungeon or iImp == iBarrow:
					if not pPlot.isVisible(caster.getTeam(),false):
						return true
	return false

def SpellRevealDungeons(caster):

	iRuins = gc.getInfoTypeForString('IMPROVEMENT_RUINS')
	iDungeon = gc.getInfoTypeForString('IMPROVEMENT_DUNGEON')
	iBarrow = gc.getInfoTypeForString('IMPROVEMENT_BARROW')

	iX = caster.getX()
	iY = caster.getY()

	for iiX in range(iX-30, iX+30, 1):
		for iiY in range(iY-30, iY+30, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				iImp = pPlot.getImprovementType()
				if iImp == iRuins or iImp == iDungeon or iImp == iBarrow:
					if not pPlot.isVisible(caster.getTeam(),false):
						pPlot.changeVisibilityCount(caster.getTeam(),1,-1)

def reqEsusHeal(caster):
	pPlot = caster.plot()
	iDiseased = gc.getInfoTypeForString('PROMOTION_DISEASED')
	iPoisoned = gc.getInfoTypeForString('PROMOTION_POISONED')
	if (caster.isAlive() and caster.getDamage() > 0):
		return True
	if caster.isHasPromotion(iDiseased):
		return True
	if caster.isHasPromotion(iPoisoned):
		return True
	return False

def spellEsusHeal(caster,amount):
	pPlot = caster.plot()
	if caster.isAlive():
		caster.changeDamage(-amount,0) #player doesn't matter - it won't kill
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DISEASED'), false)
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), false)

def reqGreatWorks(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.getNumCities < 1:
		return False
	if pPlayer.isHuman() == False:
		prodNeededTotal = 0
		for iCity in range(pPlayer.getNumCities()):
			pCity = pPlayer.getCity(iCity)
			prodNeededTotal += pCity.getProductionNeeded()
		if prodNeededTotal > 1000 :
			return True
		else:
			return False
	return True


def spellGreatWorks(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	Production=100

	Production*=gc.getGameSpeedInfo(CyGame().getGameSpeedType()).getConstructPercent()
	Production/=100

	for iCity in range(pPlayer.getNumCities()):
		pCity = pPlayer.getCity(iCity)
		iProdNeeded = pCity.getProductionNeeded()
		if iProdNeeded > Production :
			pCity.changeProduction(Production)
		else :
			pCity.changeProduction(iProdNeeded)

def reqEnlistAsMercenary(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	if gc.getUnitInfo(caster.getUnitType()).getTier()==1 or gc.getUnitInfo(caster.getUnitType()).getTier()==4:
		return false
	if caster.baseCombatStr()<4:
		return false
	if caster.isAnimal():
		return false
	iUnitClass=gc.getUnitInfo(caster.getUnitType()).getUnitClassType()

	if(gc.getUnitClassInfo(iUnitClass).getMaxGlobalInstances()!=-1):
		return false

	if(gc.getUnitClassInfo(iUnitClass).getMaxTeamInstances()!=-1):
		return false

	if(gc.getUnitClassInfo(iUnitClass).getMaxPlayerInstances()!=-1):
		return false

	return True

def spellEnlistAsHeadHunter(caster):
	mPlayer = gc.getPlayer(gc.getMERCENARIES_PLAYER())
	newUnit = mPlayer.initUnit(caster.getUnitType(), caster.getX(), caster.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
	newUnit.convert(caster)
	newUnit.AI_setGroupflag(110)

def spellEnlistAsRaider(caster):
	mPlayer = gc.getPlayer(gc.getMERCENARIES_PLAYER())
	newUnit = mPlayer.initUnit(caster.getUnitType(), caster.getX(), caster.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
	newUnit.convert(caster)
	newUnit.AI_setGroupflag(111)

def spellEnlistAsPillager(caster):
	mPlayer = gc.getPlayer(gc.getMERCENARIES_PLAYER())
	newUnit = mPlayer.initUnit(caster.getUnitType(), caster.getX(), caster.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
	newUnit.convert(caster)
	newUnit.AI_setGroupflag(112)

def reqLaunchPickEquipmentPopup(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	for i in range(gc.getNumPromotionInfos()):
		if pPlayer.canEquipUnit(caster,i,false,false,false):
			return true

	return false

def spellLaunchPickEquipmentPopup(caster):
	pPlayer = gc.getPlayer(caster.getOwner())

	if caster.getOwner()==CyGame().getActivePlayer():
		if caster.isHeadSelectedUnit():
			pPlayer.launchPickEquipmentPopup()

def reqDebug(caster):
	if CyGame().isDebugMode():
		return true

	return false
#	return true

def spellDebug(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pTeam = gc.getTeam(caster.getTeam())
	pPlayer.changeGlobalYield(YieldTypes.YIELD_LUMBER,100000)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_LEATHER,100000)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_STONE,100000)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_METAL,100000)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_HERB,100000)

	pPlayer.changeGold(100000)
	pPlayer.changeMana(100000)
	pPlayer.changeFaith(100000)

	for i in range(gc.getNumTechInfos()):
		pTeam.setHasTech(i,true,caster.getOwner(),false,false)

def reqTradeMission(caster):
	#only in foreign cities
	if caster.plot().getOwner()==caster.getOwner():
		return false

	return true

def spellTradeMission(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pTargetCity=caster.plot().getPlotCity()
	iGold = CyGame().getSorenRandNum(100, "Some gold from Trade")

	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
		iGold=(iGold / 3) * 2
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
		iGold=(iGold / 2) * 3
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
		iGold=(iGold) * 3

	pPlayer.changeGold(iGold)

	ListBonus = []
	for iLoopBonus in range (0,gc.getNumBonusInfos(),1):
		if pTargetCity.hasBonus(iLoopBonus):
			ListBonus.append(iLoopBonus)

	iRnd = CyGame().getSorenRandNum(len(ListBonus), "Some gold from Trade")

	iBonus = ListBonus[iRnd]
	pCity = pPlayer.getCapitalCity()
	if pCity == -1:
		return

	pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), iBonus, 1)
	CyInterface().addMessage(iPlayer,true,25,"We now have access to Resource %s" %(gc.getBonusInfo(iBonus).getDescription()),'',0,'',ColorTypes(11), pCity.getX(), pCity.getY(), True,True)

def reqDiscoverFarmResource(caster):
	pPlot = caster.plot()
	#is there a Bonus Resource already?
	if pPlot.getBonusType(-1)!=-1:
		return false

	#Permanent Improvement?
	if pPlot.getImprovementType()!=-1:
		if gc.getImprovementInfo(pPlot.getImprovementType()).isPermanent():
			return false

	if pPlot.isWater() or pPlot.isPeak():
		return false

	return true

def spellDiscoverFarmResource(caster):
	pPlot = caster.plot()

	iRnd = CyGame().getSorenRandNum(3,"Pick Bonus")

	if iRnd==0:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_CORN"))
	if iRnd==1:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_WHEAT"))
	if iRnd==2:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_RICE"))

def reqDiscoverPastureResource(caster):
	pPlot = caster.plot()
	#is there a Bonus Resource already?
	if pPlot.getBonusType(-1)!=-1:
		return false

	#Permanent Improvement?
	if pPlot.getImprovementType()!=-1:
		if gc.getImprovementInfo(pPlot.getImprovementType()).isPermanent():
			return false

	if pPlot.isWater() or pPlot.isPeak():
		return false

	return true

def spellDiscoverPastureResource(caster):
	pPlot = caster.plot()

	iRnd = CyGame().getSorenRandNum(4,"Pick Bonus")

	if iRnd==0:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_COW"))
	if iRnd==1:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_SHEEP"))
	if iRnd==2:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_PIG"))
	if iRnd==3:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_BISON"))

def reqDiscoverMiningResource(caster):
	pPlot = caster.plot()

	if not pPlot.isHills():
		return false

	#is there a Bonus Resource already?
	if pPlot.getBonusType(-1)!=-1:
		return false

	#Permanent Improvement?
	if pPlot.getImprovementType()!=-1:
		if gc.getImprovementInfo(pPlot.getImprovementType()).isPermanent():
			return false

	if pPlot.isWater() or pPlot.isPeak():
		return false

	return true

def spellDiscoverMiningResource(caster):
	pPlot = caster.plot()

	iRnd = CyGame().getSorenRandNum(8,"Pick Bonus")

	if iRnd==0:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_GOLD"))
	if iRnd==1:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_SILVER"))
	if iRnd==2:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_COPPER"))
	if iRnd==3:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_IRON"))
	if iRnd==4:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_GEMS"))
	if iRnd==5:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_AMBER"))
	if iRnd==6:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_MITHRIL"))
	if iRnd==7:
		pPlot.setBonusType(gc.getInfoTypeForString("BONUS_GUNPOWDER"))

def reqDiscoverFishResource(caster):
	pPlot = caster.plot()
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	if not pPlot.isCity():
		return false

	iHarbor=gc.getCivilizationInfo(pPlayer.getCivilizationType()).getCivilizationBuildings(gc.getInfoTypeForString("BUILDINGCLASS_HARBOR"))

	if iHarbor==-1 or pPlot.getPlotCity().getNumRealBuilding(iHarbor)==0:
		return false

	bValid=false
	iX = pPlot.getX()
	iY = pPlot.getY()

	for i in range(pPlot.getPlotCity().getNumCityPlots()):
		pPlot2 =pPlot.getPlotCity().getCityIndexPlot(i)
		if pPlot2.isWater() and pPlot2.getBonusType(-1)==-1:
			bValid=true

	if not bValid:
		return false

	return true

def spellDiscoverFishResource(caster):
	pPlot = caster.plot()

	iX = pPlot.getX()
	iY = pPlot.getY()
	for i in range(pPlot.getPlotCity().getNumCityPlots()):
		pPlot2 =pPlot.getPlotCity().getCityIndexPlot(i)
		if pPlot2.isWater() and pPlot2.getBonusType(-1)==-1:

			iRnd = CyGame().getSorenRandNum(3,"Pick Bonus")

			if iRnd==0:
				pPlot2.setBonusType(gc.getInfoTypeForString("BONUS_CLAM"))
			if iRnd==1:
				pPlot2.setBonusType(gc.getInfoTypeForString("BONUS_FISH"))
			if iRnd==2:
				pPlot2.setBonusType(gc.getInfoTypeForString("BONUS_CRAB"))

			return

def reqDiscoverManaNode(caster):
	pPlot = caster.plot()
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	if pPlot.isCity():
		return false

	if gc.getBonusInfo(pPlot.getBonusType(-1)) == "BONUS_MANA":
		return false

	iX = pPlot.getX()
	iY = pPlot.getY()

	iMana=gc.getInfoTypeForString("BONUSCLASS_MANA")
	iRawMana=gc.getInfoTypeForString("BONUSCLASS_RAWMANA")
	for iiX in range(iX-2, iX+2, 1):
		for iiY in range(iY-2, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if pPlot2.getBonusType(-1)!=-1:
				if gc.getBonusInfo(pPlot2.getBonusType(-1)).getBonusClassType()==iMana or gc.getBonusInfo(pPlot2.getBonusType(-1)).getBonusClassType()==iRawMana:
					return false

	return true

def spellDiscoverManaNode(caster):
	pPlot = caster.plot()

	pPlot.setBonusType(gc.getInfoTypeForString("BONUS_MANA"))


def reqDiscoverSpellResearch(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	for iTech in range(gc.getNumTechInfos()):
		if gc.getTechInfo(iTech).isSpellResearch():
			if pPlayer.canSpellResearchNext(iTech):
				if gc.getTechInfo(iTech).getResearchCost()<2000:
					return true

	return false

def spellDiscoverSpellResearch(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	TechList =[]
	for iTech in range(gc.getNumTechInfos()):
		if gc.getTechInfo(iTech).isSpellResearch():
			if pPlayer.canSpellResearchNext(iTech):
				TechList.append(iTech)

	iRnd = CyGame().getSorenRandNum(len(TechList), "Discover Spellresearch")

	gc.getTeam(pPlayer.getTeam()).setHasTech(TechList[iRnd],true,iPlayer,false,true)

def getInventEquipment(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pTeam = gc.getTeam(pPlayer.getTeam())

	if pTeam.isHasTech(gc.getInfoTypeForString('TECH_ARCHERY')):
		if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_COMPOSITE_LONGBOW')):
			return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_COMPOSITE_LONGBOW')
	if pTeam.isHasTech(gc.getInfoTypeForString('TECH_BRONZE_WORKING')):
		if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WARAXE')):
			return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WARAXE')
	if pTeam.isHasTech(gc.getInfoTypeForString('TECH_HORSEBACK_RIDING')):
		if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_MASTERWORK_LANCE')):
			return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_MASTERWORK_LANCE')
	if pTeam.isHasTech(gc.getInfoTypeForString('TECH_HUNTING')):
		if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_LONGSPEAR')):
			return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_LONGSPEAR')

	if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WYRMSCALE')):
		return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WYRMSCALE')
	if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_FORGEMAIL')):
		return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_FORGEMAIL')
	if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_ELDERSCALE')):
		return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_ELDERSCALE')
	if not pPlayer.isAdventureFinished(gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WARPLATE')):
		return gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WARPLATE')

def reqDiscoverEquipment(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	#List of possible Equipment
	if getInventEquipment(caster)!=-1:
		return true
	return false

def spellDiscoverEquipment(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	iAdventure = getInventEquipment(caster)
	pPlayer.setAdventureFinished(iAdventure,true)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_COMPOSITE_LONGBOW'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_COMPOSITE_LONGBOWS",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WARAXE'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_WARAXE",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_LONGSPEAR'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_LONGSPEAR",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_MASTERWORK_LANCE'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_MASTERWORK_LANCE",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WYRMSCALE'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_WYRMSCALE",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_FORGEMAIL'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_FORGEMAIL",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_ELDERSCALE'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_ELDERSCALE",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
	if iAdventure==gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_WARPLATE'):
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_WARPLATE",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)


def	spellIncreaseGlobalCulture(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	#xtended
	if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_ADVENTURER_TALE')):
		pPlayer.changeGlobalCulture(1500)
	else:
		pPlayer.changeGlobalCulture(1000)	

def reqDivineIntervention(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	return pPlayer.getStateReligion()!=-1

def	spellDivineIntervention(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	pPlayer.changeFaith(1000)

def reqIncreaseInfluenceOnHolyCity(caster):
	iReligion=gc.getPlayer(caster.getOwner()).getStateReligion()
	if(iReligion==-1):
		return false
	pCity=caster.plot().getPlotCity()
	if(pCity.isHolyCityByType(iReligion)):
		return true
	return false

def	spellIncreaseInfluenceOnHolyCity(caster):
	iPlayer=caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	pPlayer.changeFaith(2000)

