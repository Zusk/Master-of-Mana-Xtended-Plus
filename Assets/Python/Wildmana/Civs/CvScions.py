import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#SCIONS

def onScionsCaptureCity(argsList):

	'City Acquired'
	iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
	pPlayer = gc.getPlayer(iNewOwner)
	pPrevious = gc.getPlayer(iPreviousOwner)

	#Scions Start - extra reduction of Scions-acquired cities.  Use of a decimal seems to break it.
	if (gc.getPlayer(pCity.getOwner()).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS') and gc.getPlayer(pCity.getOwner()).getLeaderType() != gc.getInfoTypeForString('LEADER_KOUN')):
		iBCPop = pCity.getPopulation() * 10
#		if gc.getPlayer(pCity.getOwner()).getCivics(gc.getInfoTypeForString('CIVICOPTION_GOVERNMENT')) == gc.getInfoTypeForString('CIVIC_GOD_KING'):
#			iBCMod = iBCPop / 16
#		else:
		iBCMod = iBCPop / 12
		pCity.changePopulation(-iBCMod)

	if gc.getPlayer(pCity.getPreviousOwner()).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
		iBCPop = pCity.getPopulation() * 10
		iBCMod = iBCPop / 12
		pCity.changePopulation(-iBCMod)
	#Scions End

def onScionsRazeCity(argsList):
	'City Razed'
	city, iPlayer = argsList
	iOwner = city.findHighestCulture()

	pPlayer = gc.getPlayer(city.getOwner())
	# scions start - Gives Reborn when razing cities.  The function reducing the population of Scion conquests kicks in first.  Currently Reborn given = that population -1.  Requires Sorc. and Priestood.  It's been suggested that be changed to requiring the civic "Glory."
	iReborn = gc.getInfoTypeForString('UNIT_REBORN')
	eTeam = gc.getTeam(pPlayer.getTeam())
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
		if (eTeam.isHasTech(gc.getInfoTypeForString('TECH_SORCERY')) and eTeam.isHasTech(gc.getInfoTypeForString('TECH_PRIESTHOOD'))):
#			if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_GOVERNMENT')) != gc.getInfoTypeForString('CIVIC_GOD_KING'):
			if city.getPopulation() > 1:
				CyInterface().addMessage(city.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_REBORN_SPAWNED_RAZED", ()),'',1,'Art/Interface/Buttons/Units/reborn.dds',ColorTypes(8),city.getX(),city.getY(),True,True)
			for i in range((city.getPopulation()) - 1):
				spawnUnit = pPlayer.initUnit(iReborn, city.getX(), city.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
#			else:
#				if city.getPopulation() > 1:
#					CyInterface().addMessage(city.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_REBORN_SPAWNED_RAZED", ()),'',1,'Art/Interface/Buttons/Units/reborn.dds',ColorTypes(8),city.getX(),city.getY(),True,True)
#				for i in range((city.getPopulation()) / 2):
#					spawnUnit = pPlayer.initUnit(iReborn, city.getX(), city.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	# scions end

def ScionUnitCreated(argsList):
	'Unit Completed'

	unit = argsList[0]
	pPlayer = gc.getPlayer(unit.getOwner())

	if pPlayer.getCivilizationType()!=gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
		return

	iUnitType=unit.getUnitType()

	if iUnitType==gc.getInfoTypeForString('UNIT_ARCHER'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEADLESS'),true)
	elif iUnitType==gc.getInfoTypeForString('UNIT_CENTENI'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_UNDEAD'),false)
	elif iUnitType==gc.getInfoTypeForString('UNIT_KORRINA_PROTECTOR'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_UNDEAD'),false)
	elif iUnitType==gc.getInfoTypeForString('UNIT_LONGBOWMAN'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEADLESS'),true)
	elif iUnitType==gc.getInfoTypeForString('UNIT_CROSSBOWMAN'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEADLESS'),true)
	elif iUnitType==gc.getInfoTypeForString('UNIT_MARKSMAN'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEADLESS'),true)
	elif iUnitType==gc.getInfoTypeForString('UNIT_PRINCIPES'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEADLESS'),true)
	elif iUnitType==gc.getInfoTypeForString('UNIT_PHALANX'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEADLESS'),true)
	elif iUnitType==gc.getInfoTypeForString('UNIT_KORRINA_RED_LADY'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'),false)
	elif iUnitType==gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'),false)
	elif iUnitType==gc.getInfoTypeForString('UNIT_KORRINA_WHITE_LADY'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'),false)
	elif iUnitType == gc.getInfoTypeForString('UNIT_EMPERORS_DAGGER'):
		lPromoList = ['PROMOTION_MARCH_WINE_RECON', 'PROMOTION_NIGHT_GEAR_RECON', 'PROMOTION_WOODS_GEAR_RECON', 'PROMOTION_DESERT_GEAR_RECON', 'PROMOTION_SUPERIOR_KET_RECON', 'PROMOTION_HIGH_QUALITY_MITHRIL_MELEE', 'PROMOTION_DEADLY_TRAPS_RECON']
		sPromo = lPromoList[CyGame().getSorenRandNum(len(lPromoList), "Pick Promotion")]
		unit.setHasPromotion(gc.getInfoTypeForString(sPromo), True)
		lPromoList.remove(sPromo)
		sPromo = lPromoList[CyGame().getSorenRandNum(len(lPromoList), "Pick Promotion")]
		unit.setHasPromotion(gc.getInfoTypeForString(sPromo), True)
		lPromoList.remove(sPromo)
		sPromo = lPromoList[CyGame().getSorenRandNum(len(lPromoList), "Pick Promotion")]
		unit.setHasPromotion(gc.getInfoTypeForString(sPromo), True)
	elif iUnitType==gc.getInfoTypeForString('UNIT_MARTYR_OF_PATRIA'):
		if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_POISONS')):
			unit.setHasPromotion(gc.getInfoTypeForString('EQUIPMENT_POISONS'),true)

def onBeginGameTurnScions(argsList):
	'Called at the beginning of the end of each turn'
	iGameTurn = argsList[0]

	bAreThereScionsHere = False
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.getCivilizationType()== gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
			iAreThereScionsHere = True
			break

	#Haunted Lands Effect

	iHauntedLands = gc.getInfoTypeForString('FEATURE_HAUNTED_LANDS')
	iKorinnaRed = gc.getInfoTypeForString('UNIT_KORRINA_RED_LADY')
	iKorinnaBlack = gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY')
	iKorinnaWhite = gc.getInfoTypeForString('UNIT_KORRINA_WHITE_LADY')
	iChance = 15
	if CyGame().getSorenRandNum(100, "Chance for negative HL Effect") < iChance:
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getFeatureType()==iHauntedLands:
				for i in range(pPlot.getNumUnits()):
					currentUnit = pPlot.getUnit(i)
					if currentUnit.isAlive() and currentUnit.getUnitType()!=iKorinnaRed and currentUnit.getUnitType()!=iKorinnaBlack and currentUnit.getUnitType()!=iKorinnaWhite and currentUnit.getUnitCombatType()!=-1:
						if currentUnit.isHuman() or currentUnit.getGroup().getHeadUnit()!=currentUnit:	#AI Headunits are immune to HL effects
							if currentUnit.getExperience()>0:
								currentUnit.setExperience(currentUnit.getExperience()-1,99999)
							if currentUnit.getExperience()<5 and CyGame().getSorenRandNum(17, "Chance for HL effect Desperation") < 4:
								currentUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DESPERATION'), true)
							lPromoList = []
							lPromoList = lPromoList + ['PROMOTION_CONFUSED', 'PROMOTION_POISONED']
							sPromo = lPromoList[CyGame().getSorenRandNum(len(lPromoList), "Pick Promotion")]
							currentUnit.setHasPromotion(gc.getInfoTypeForString(sPromo), True)
							if currentUnit.getDamage() >= 20 and CyGame().getSorenRandNum(11, "Chance for HL effect Undead") < 4:
								currentUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_UNDEAD'), true)
								currentUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ESTRANGED'), true)
			elif bAreThereScionsHere:
				iHLChance = ScionFunctions.calculateHLChance(ScionFunctions(), pPlayer)
				ScionFunctions.createHauntedLands(ScionFunctions(), pPlot, iHLChance)
	#Haunted Lands Effect

# SCIONS - Awakened spawning

def onBeginScionsTurn(argsList):
	iGameTurn  = argsList[0]
	iPlayer = argsList[1]
	pPlayer = gc.getPlayer(iPlayer)

	if not (pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS')):
		return

	py = PyPlayer(iPlayer)
	pTomb = pPlayer.getCapitalCity()
	iAwakened = gc.getInfoTypeForString('UNIT_AWAKENED')
	iSpawnOdds = ScionFunctions.doChanceAwakenedSpawn(ScionFunctions(),iPlayer)

	if CyGame().getSorenRandNum(10000, "Spawn Roll") < iSpawnOdds:
		if pPlayer.isHuman():
			spawnUnit = pPlayer.initUnit(iAwakened, pTomb.getX(), pTomb.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		else:
			# ai uses its awakened only in the city in which they appear so let's spawn them at a random place
			numCities = py.getNumCities()
			iCity = CyGame().getSorenRandNum(numCities, "Spawn Roll")
			city = py.getCityList()[iCity]
			spawnUnit = pPlayer.initUnit(iAwakened, city.getX(), city.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

	#Units
	iGhastlyDragoon = gc.getInfoTypeForString('PROMOTION_GHASTLY_DRAGOON')
	iBroken = gc.getInfoTypeForString('PROMOTION_BROKEN')
	iMounted = gc.getInfoTypeForString('UNITCOMBAT_MOUNTED')
	iNatureMana = pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_MANA_NATURE'))
	iLifeMana = pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_MANA_LIFE'))
	iCreeperClass = gc.getInfoTypeForString('UNITCLASS_CREEPER')
	iBlackLadyClass = gc.getInfoTypeForString('UNITCLASS_KORRINA_BLACK_LADY')
	iGhostwalkerClass = gc.getInfoTypeForString('UNITCLASS_RANGER')
	iBlackLadyFactor = pPlayer.getUnitClassCount(iBlackLadyClass) * 2
	iNumCreepers = pPlayer.getUnitClassCount(iCreeperClass)
	iNumGhostwalkers = pPlayer.getUnitClassCount(iGhostwalkerClass)
	iCreeperSeed = 1 + iNatureMana + iLifeMana + iBlackLadyFactor

	if iNumCreepers >= iNumGhostwalkers:
		iCreeperSeed = 0
	iAlcinus = gc.getInfoTypeForString('UNIT_ALCINUS')
	iAlcinusUpgrade = gc.getInfoTypeForString('UNIT_ALCINUS_UPGRADED')
	iAlcinusArchmage = gc.getInfoTypeForString('UNIT_ALCINUS_ARCHMAGE')
	iHauntProm = gc.getInfoTypeForString('PROMOTION_HAUNT')
	for pUnit in py.getUnitList():
		if pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY'):
			if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_FERAL_BOND')):
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DARK_EMPATHY'),true)
		elif pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_MARTYR_OF_PATRIA'):
			if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_POISONS')):
				pUnit.setHasPromotion(gc.getInfoTypeForString('EQUIPMENT_POISONS'),true)
		elif pUnit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_MAGE'):
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_NECROMANCER'),true)
		#elif pUnit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_KORRINA_RED_LADY'):
		#	pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING1'),false)
		elif pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HEADLESS')):
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_UNDEAD'),true)
		elif pUnit.getUnitType() == iAlcinus or pUnit.getUnitType() == iAlcinusUpgrade or pUnit.getUnitType() == iAlcinusArchmage:
			if not pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED')):
				if CyGame().getSorenRandNum(100, "Madness")<3:
					if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_EMPERORS_DAGGER')):
						CyInterface().addMessage(pUnit.getOwner(),True,25,CyTranslator().getText("Alcinus was about to go mad, but after a friendly reminder from the Emperor's Dagger, he now feels a little bit more loyal.", ()),'',0,'Art/Interface/Buttons/Units/Empdagger.dds',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)
						pUnit.setDamage(30+CyGame().getSorenRandNum(40, "Alcinus damage"), pUnit.getOwner())
					else:
						pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'),True)
						CyInterface().addMessage(pUnit.getOwner(),True,25,CyTranslator().getText("Alcinus has again gone mad.", ()),'',0,'Art/Interface/Buttons/Promotions/Enraged.dds',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)

		if iCreeperSeed > 0:
			if pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_GHOSTWALKER'):
				if CyGame().getSorenRandNum(100, "Creeper check")<=iCreeperSeed:
					spawnUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_CREEPER'), pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					spawnUnit.AI_setGroupflag(45)	#45=GROUPFLAG_CREEPER

		if pUnit.isHasPromotion(iHauntProm) and iGhostwalkerClass==pUnit.getUnitClassType():
			if CyGame().getSorenRandNum(4, "Haunt to Ghostwalker Check") >= 1:  #TODO: add small betrayal chance for Haunted Ghostwalkers
				newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GHOSTWALKER'), pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.setDamage(pUnit.getDamage(), pUnit.getOwner())
				newUnit.convert(pUnit)
				newUnit.setHasPromotion(iHauntProm, False)
				continue
				
		# Ghastly Dragoon mechanic
		if pUnit.getUnitCombatType() == iMounted and pUnit.isHasPromotion(iBroken):
			if CyGame().getSorenRandNum(4, "Broken to Ghastly Dragoon Check") == 0:
				pUnit.setHasPromotion(iBroken, False)
				pUnit.setHasPromotion(iGhastlyDragoon, True)

	pTomb = pPlayer.getCapitalCity()
	iTombPop = pTomb.getPopulation()
	if pPlayer.getNumCities() == 1:
		if iTombPop == 1:
			pTomb.changePopulation(1)

	if pPlayer.getNumCities() > 0:
		for pyCity in PyPlayer(iPlayer).getCityList():
			pCity = pyCity.GetCy()
			iOBBuilding = gc.getInfoTypeForString('BUILDING_OBBUILDING')
			iOBBuildingClass = gc.getInfoTypeForString('BUILDINGCLASS_OBBUILDING')
			iReborn = gc.getInfoTypeForString('UNIT_REBORN')

			if pCity.getProductionUnit() != iReborn:
				pCity.setNumRealBuilding(iOBBuilding, 0)
			else:
				pCity.setNumRealBuilding(iOBBuilding, 1)
				teamPlayer = gc.getTeam(pPlayer.getTeam())
				iNumOpenBorders = (teamPlayer.getOpenBordersTradingCount()) - 1
				for iLoopCiv in range(gc.getMAX_CIV_PLAYERS()):
					if (teamPlayer.isOpenBorders(iLoopCiv)):
						iNumOpenBorders = iNumOpenBorders + 1

#				if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_GOVERNMENT')) == gc.getInfoTypeForString('CIVIC_GOD_KING'):
#					iNumOpenBorders = iNumOpenBorders + 1

				if iNumOpenBorders == 1:
					pCity.setBuildingYieldChange(iOBBuildingClass,YieldTypes.YIELD_PRODUCTION,3)
				elif iNumOpenBorders == 2:
					pCity.setBuildingYieldChange(iOBBuildingClass,YieldTypes.YIELD_PRODUCTION,5)
				elif iNumOpenBorders == 3:
					pCity.setBuildingYieldChange(iOBBuildingClass,YieldTypes.YIELD_PRODUCTION,8)
				elif iNumOpenBorders == 4:
					pCity.setBuildingYieldChange(iOBBuildingClass,YieldTypes.YIELD_PRODUCTION,12)
				elif iNumOpenBorders == 5:
					pCity.setBuildingYieldChange(iOBBuildingClass,YieldTypes.YIELD_PRODUCTION,16)
				elif iNumOpenBorders == 6:
					pCity.setBuildingYieldChange(iOBBuildingClass,YieldTypes.YIELD_PRODUCTION,20)
				elif iNumOpenBorders > 6:
					pCity.setBuildingYieldChange(iOBBuildingClass,YieldTypes.YIELD_PRODUCTION,25)

			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_NECROPOLIS')) > 0:
				iNumHealthBonus = 0
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_MANA_LIFE')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_TOAD')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_BANANA')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_WHEAT')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_CORN')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_RICE')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_CRAB')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_CLAM')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_FISH')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_DEER')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_PIG')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_COW')) > 0:
					iNumHealthBonus += 1
				if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_SHEEP')) > 0:
					iNumHealthBonus += 1
				iBonusBuilding = gc.getInfoTypeForString('BUILDING_NECRO_BONUS_BUILDING')
				iBonusBuildingClass = gc.getInfoTypeForString('BUILDINGCLASS_NECRO_BONUS_BUILDING')
				pCity.setNumRealBuilding(iBonusBuilding, 1)
				pCity.setBuildingYieldChange(iBonusBuildingClass,YieldTypes.YIELD_PRODUCTION,iNumHealthBonus)
				pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT1'), 0)
				pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT2'), 0)
			else:
				iUH = 0
				iUH = pCity.badHealth(False) - pCity.goodHealth()

				if (iUH >= 3 and iUH <= 5):
					pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT1'), 1)
					pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT2'), 0)
				elif iUH >= 6:
					pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT2'), 1)
					pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT1'), 0)
				else:
					pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT1'), 0)
					pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNHEALTHY_DISCONTENT2'), 0)

			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_RISEN_EMPEROR'):
				if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_GIFT')) == 0:
					if pCity.getOriginalOwner()==iPlayer:
						pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_EMPERORS_MARK'), 1)
				else:
					pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_EMPERORS_MARK'), 0)

			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_KEEP_MAIN')) == 1:
				plot=pCity.plot()
				for ii in range(plot.getNumUnits()):
					pUnit=plot.getUnit(ii)
					if pUnit.getUnitCombatType()==gc.getInfoTypeForString('UNITCOMBAT_ADEPT'):
						# there is a 3% chance for arcane units per turn to get estranged when they are in the city with the keep
						# only heroes and Alcinus are immune to this
						if not (pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HERO')) or pUnit.getUnitType() == iAlcinus or pUnit.getUnitType() == iAlcinusUpgrade or pUnit.getUnitType() == iAlcinusArchmage):
							if CyGame().getSorenRandNum(100, "Estranged-Keep_Main")<3:
								pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ESTRANGED'),true)
								CyInterface().addMessage(pUnit.getOwner(),True,25,CyTranslator().getText("A mage at the Keep feels a little bit estranged.", ()),'',0,'Art/Interface/Buttons/Promotions/restorelife.dds',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)
						# there is a 15% chance for arcane units per turn to get 1 XP when they are in the city with the keep
						if CyGame().getSorenRandNum(100, "XP-gaining-Keep_Main")<15:
							pUnit.setExperience(pUnit.getExperience()+1,-1)

	pPlayer.setFeatAccomplished(FeatTypes.FEAT_MANIFEST_HORNED_DREAD, True)

	iHauntClass = gc.getInfoTypeForString('UNITCLASS_HAUNT')
	iEligibleNum = (pPlayer.getUnitClassCount(iGhostwalkerClass) + pPlayer.getUnitClassCount(iHauntClass))

	if iEligibleNum < 1:
		pPlayer.setFeatAccomplished(FeatTypes.FEAT_MANIFEST_FIRST_HORNED_DREAD, False)

	if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_KNOWLEDGE_OF_THE_ETHER')) and pPlayer.getNumCities() > 0:
		iAlcinusClass = gc.getInfoTypeForString('UNITCLASS_ALCINUS')
		iAlcinusUpgradeClass = gc.getInfoTypeForString('UNITCLASS_ALCINUS_UPGRADED')
		iAlcinusArchmageClass = gc.getInfoTypeForString('UNITCLASS_ALCINUS_ARCHMAGE')
		if pPlayer.getUnitClassCount(iAlcinusClass) + pPlayer.getUnitClassCount(iAlcinusUpgradeClass) + pPlayer.getUnitClassCount(iAlcinusArchmageClass) == 0:
			iAlcinusOdds = 6
			estiEnd = CyGame().getEstimateEndTurn()
			if ( estiEnd >= 1500 ):
				iAlcinusOdds = 2
			elif ( estiEnd >= 750 ):
				iAlcinusOdds = 3
			elif ( estiEnd >= 500 ):
				iAlcinusOdds = 4
			elif ( estiEnd >= 330 ):
				iAlcinusOdds = 5
			if CyGame().getSorenRandNum(20, "Alcinus spawn check") < iAlcinusOdds:
				if gc.getPlayer(gc.getBARBARIAN_PLAYER()).getUnitClassCount(iAlcinusClass) + gc.getPlayer(gc.getBARBARIAN_PLAYER()).getUnitClassCount(iAlcinusUpgradeClass) + gc.getPlayer(gc.getBARBARIAN_PLAYER()).getUnitClassCount(iAlcinusArchmageClass) == 0:
						if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_STRENGTH_OF_WILL')):
							newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ALCINUS_ARCHMAGE'), pTomb.getX(), pTomb.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						elif pPlayer.isHasTech(gc.getInfoTypeForString('TECH_SORCERY')):
							newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ALCINUS_UPGRADED'), pTomb.getX(), pTomb.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						else:
							newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ALCINUS'), pTomb.getX(), pTomb.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ESTRANGED'), True)


class ScionFunctions:

	def doChanceAwakenedSpawn(self,iPlayer):
		if iPlayer == -1:
			pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
		else:
			pPlayer = gc.getPlayer(iPlayer)

		if pPlayer.getNumCities() > 0 and pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
			iSpawnChance = 0
			pTomb = pPlayer.getCapitalCity()
			iTombPop = pTomb.getPopulation()
			iTPop = pPlayer.getTotalPopulation()

		# Stops spawning once a certain population is reached.  That pop. depends on worldsize.  (I'd rather link it to turn # and gamespeed.)
		#			iTPopLmt = 50
		#			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_DUEL'):
		#				iTPopLmt = 20
		#			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_TINY'):
		#				iTPopLmt = 25
		#			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_SMALL'):
		#				iTPopLmt = 30
		#			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_STANDARD'):
		#				iTPopLmt = 50
		#			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_LARGE'):
		#				iTPopLmt = 70
		#			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_HUGE'):
		#				iTPopLmt = 100

			iTotalLand = int(CyMap().getLandPlots())
		#			message = "%0.2f" %(iTotalLand)
		#			CyInterface().addImmediateMessage(message,"")

			iTPopLmt = (iTotalLand / 40) + 6

		#			iTotalLand = int(CyMap().getLandPlots())
		#			message = "%0.2f" %(iTPopLmt)
		#			CyInterface().addImmediateMessage(message,"")

			iAwakened = gc.getInfoTypeForString('UNIT_AWAKENED')
			iAwakenedClass = gc.getInfoTypeForString('UNITCLASS_AWAKENED')
			iTemple = gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_GIFT')
			iHall = gc.getInfoTypeForString('BUILDING_HALL_OF_THE_COVENANT')
			iCen = gc.getInfoTypeForString('BUILDING_IMPERIAL_CENOTAPH')
			iShrine = gc.getInfoTypeForString('BUILDING_SHRINE_TO_KYLORIN')
			iCult = gc.getInfoTypeForString('BUILDING_KYLORIN_CULT')
			iNumTemples = pPlayer.countNumBuildings(iTemple)
			iNumHalls = pPlayer.countNumBuildings(iHall)
			iNumCen = pPlayer.countNumBuildings(iCen)
			iNumShrine = pPlayer.countNumBuildings(iShrine)
			iNumCult = pPlayer.countNumBuildings(iCult)

		# Different buildings give different modifiers.  Aristocracy gives a +66% mod.  (
			iShrineMod = iNumShrine * 0.5
			iCenMod = iNumCen
			iHallsMod = iNumHalls * 2
			iTemplesMod = iNumTemples
			iCultMod = iNumCult
			iCapMod = iTombPop * 0.25
			iTPopLmtMod = iTPopLmt * 0.025
			iCivicMod = 0.00
			iCivicMult = 1
			if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_GOVERNMENT')) == gc.getInfoTypeForString('CIVIC_ARISTOCRACY'):
				iCivicMod = 4
				iCivicMult = 1.25

		# Makes it very unlikely the Scions are screwed by missing "rolls" in the very early game.
			iPopLowMod = 1
			if iTPop <= 5:
				iPopLowMod = 1.5

			iLuxuryMod = 0
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_PEARL')) > 0:
				iLuxuryMod += 0.5
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_INCENSE')) > 0:
				iLuxuryMod += 0.5
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_FUR')) > 0:
				iLuxuryMod += 0.5
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_SILK')) > 0:
				iLuxuryMod += 1
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_GOLD')) > 0:
				iLuxuryMod += 0.5
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_GEMS')) > 0:
				iLuxuryMod += 1
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_IVORY')) > 0:
				iLuxuryMod += 0.5
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_DYE')) > 0:
				iLuxuryMod += 1
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_COTTON')) > 0:
				iLuxuryMod += 0.5
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_TEA')) > 0:
				iLuxuryMod += 0.5
			if iLuxuryMod >= 4.5:
				iLuxuryMod = iLuxuryMod * 2

		# Raise ASpnd to power (between 1 and 2) instead?
		# Modifies the rate based on gamespeed.
			iSpeedMod = 1
			estiEnd = CyGame().getEstimateEndTurn()
			if ( estiEnd >= 1500 ):
				iSpeedMod = 2.0
			elif ( estiEnd >= 750 ):
				iSpeedMod = 1.5
			elif ( estiEnd >= 500 ):
				iSpeedMod = 1.0
			elif ( estiEnd >= 330 ):
				iSpeedMod = 0.67

		#Decaymod
			iTurnMod = 1
			estiEnd = CyGame().getEstimateEndTurn()
			if ( estiEnd >= 1500 ):
				iTurnMod = 0.106
			elif ( estiEnd >= 750 ):
				iTurnMod = 0.14
			elif ( estiEnd >= 500 ):
				iTurnMod = 0.213
			elif ( estiEnd >= 330 ):
				iTurnMod = 0.425

			# BlackArchon: This was formerly the body mana bonus by the Flesh Studio.
			# Since this mana type no longer exists, I set it to 2 if the building is available.
			iCorpusB = gc.getInfoTypeForString('BUILDING_FLESH_STUDIO')
			iNumCorpusB = pPlayer.countNumBuildings(iCorpusB)
			if iNumCorpusB > 0:
				iFleshStudioMod = 2
			else:
				iFleshStudioMod = 0

			# Difficulty level modifier
			iHumanmod = 1
			iAImod = 1

			if not pPlayer.isHuman():
				iDifficulty = gc.getNumHandicapInfos() + 1 - int(gc.getGame().getHandicapType())
				iAImod = .3 + (iDifficulty * 0.08)

			# Per/resource Patrian Artifact modifier
			iPA = pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_PATRIAN_ARTIFACTS'))
			iPAMod = iPA * 0.75
			# Slight reduction in odds each turn.
			iGTurn = CyGame().getGameTurn()
			iDecayMod = 111 - (iGTurn * iTurnMod)
			if (iDecayMod < 1):
				iDecayMod = 1
			# default adjustments of gamespeed training are M =2, E = 1.5, N = 1, Q = .67
			iSpawnChance = round(((iCapMod + iFleshStudioMod + iTemplesMod + iCenMod + iHallsMod + iShrineMod + iCultMod + iPAMod + iLuxuryMod + iTPopLmtMod + iCivicMod + 3) * iCivicMult * iHumanmod / iSpeedMod * iDecayMod * 0.01 * iPopLowMod / iAImod),2)

			if iTPop >= iTPopLmt:
				iSpawnChance =round(iSpawnChance*iTPopLmt/iTPop)

			if pPlayer.getDisableProduction() > 0:
				iSpawnChance = 0

			if not pPlayer.isHuman():
				iSpawnChance = iSpawnChance * 70
				iSpawnChance = iSpawnChance / 100

			iSpawnChance = int(iSpawnChance * 100)

			return iSpawnChance


	def createHauntedLands(self, pPlot, iHLChance):
		if pPlot.isOwned() and not pPlot.isWater() and not pPlot.isPeak():
			pPlayer = gc.getPlayer(pPlot.getOwner())
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
				if CyGame().getSorenRandNum(1400, "Chance for HL in unwooded tile") < iHLChance :
					pPlot.setFeatureType(iHauntedLands, 0)


	def calculateHLChance(self, pPlayer):
		iNatureMana = pPlayer.getNumAvailableBonuses(iManaNature)
		iManaMod = 1 + (iNatureMana * 0.1)
		iGhostwalkerFactor = pPlayer.getUnitClassCount(iGhostwalkerClass)
		iHauntFactor = pPlayer.getUnitClassCount(iHauntClass) * 2
		iRedactorFactor = pPlayer.getUnitClassCount(iRedactorClass) * 4
		iBlackLadyFactor = pPlayer.getUnitClassCount(iBlackLadyClass) + 1
		iHLChance = (iGhostwalkerFactor + iHauntFactor + iRedactorFactor) * iBlackLadyFactor * iManaMod

		return iHLChance
