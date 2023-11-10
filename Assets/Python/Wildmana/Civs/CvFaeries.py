import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#BEGIN Faeries
iCivicOptionFaerieCourt = gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')
iCivicSummerCourt = gc.getInfoTypeForString('CIVIC_SUMMER_COURT')
iCivicWinterCourt = gc.getInfoTypeForString('CIVIC_WINTER_COURT')
iCivicEternalCourt = gc.getInfoTypeForString('CIVIC_ETERNAL_COURT')
iFaeries = gc.getInfoTypeForString('CIVILIZATION_FAERIES')
iBarbarians = gc.getInfoTypeForString('CIVILIZATION_BARBARIAN')
iAnimals = gc.getInfoTypeForString('CIVILIZATION_ANIMAL')

iPromFaerie = gc.getInfoTypeForString('PROMOTION_FAERIE')
iEternalFaerie = gc.getInfoTypeForString('PROMOTION_ETERNAL_FAERIE')
iPromEternal = gc.getInfoTypeForString('PROMOTION_ETERNAL')
iWinterFaerie = gc.getInfoTypeForString('PROMOTION_WINTER_FAERIE')
iSummerFaerie = gc.getInfoTypeForString('PROMOTION_SUMMER_FAERIE')
iAssimilate = gc.getInfoTypeForString('PROMOTION_ASSIMILATE_MIND')
iPromWoodsMan1 = gc.getInfoTypeForString('PROMOTION_WOODSMAN1')

# faerie colours
iPromClear = gc.getInfoTypeForString('PROMOTION_CLEAR_FAERIE')
iPromTrueClear = gc.getInfoTypeForString('PROMOTION_TRUE_CLEAR_FAERIE')
iPromWhite = gc.getInfoTypeForString('PROMOTION_WHITE_FAERIE')
iPromTrueWhite = gc.getInfoTypeForString('PROMOTION_TRUE_WHITE_FAERIE')
iPromGreen = gc.getInfoTypeForString('PROMOTION_GREEN_FAERIE')
iPromTrueGreen = gc.getInfoTypeForString('PROMOTION_TRUE_GREEN_FAERIE')
iPromBlue = gc.getInfoTypeForString('PROMOTION_BLUE_FAERIE')
iPromTrueBlue = gc.getInfoTypeForString('PROMOTION_TRUE_BLUE_FAERIE')
iPromRed = gc.getInfoTypeForString('PROMOTION_RED_FAERIE')
iPromTrueRed = gc.getInfoTypeForString('PROMOTION_TRUE_RED_FAERIE')
iPromPurple = gc.getInfoTypeForString('PROMOTION_PURPLE_FAERIE')
iPromTruePurple = gc.getInfoTypeForString('PROMOTION_TRUE_PURPLE_FAERIE')

colorList = [iPromGreen, iPromWhite, iPromBlue, iPromRed, iPromPurple, iPromClear, iPromTrueGreen, iPromTrueWhite, iPromTrueBlue, iPromTrueRed, iPromTruePurple, iPromTrueClear]

nonFaerieUnitCombats = [-1]
nonFaerieUnitCombats.append(gc.getInfoTypeForString('UNITCOMBAT_SIEGE'))
nonFaerieUnitCombats.append(gc.getInfoTypeForString('UNITCOMBAT_NAVAL'))

iWorkerClass = gc.getInfoTypeForString('UNITCLASS_WORKER')
iSettlerClass = gc.getInfoTypeForString('UNITCLASS_SETTLER')
iDisciple = gc.getInfoTypeForString('UNITCOMBAT_DISCIPLE')
iArcane = gc.getInfoTypeForString('UNITCOMBAT_ADEPT')

# basic land types
iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
iMarsh = gc.getInfoTypeForString('TERRAIN_MARSH')
iOasis = gc.getInfoTypeForString('FEATURE_OASIS')
iVolcano = gc.getInfoTypeForString('IMPROVEMENT_VOLCANO')
iFlood = gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')
iForestAncient = gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')
iForest = gc.getInfoTypeForString('FEATURE_FOREST')
iNewForest = gc.getInfoTypeForString('FEATURE_FOREST_NEW')
# special land types - mostly Faerie only
iNever = gc.getInfoTypeForString('TERRAIN_NEVER_NEVER')
iFields = gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')
iSands = gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')
iBroken = gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS')
iEternalWoods = gc.getInfoTypeForString('FEATURE_ETERNAL_WOODS')
iMushGround = gc.getInfoTypeForString('TERRAIN_MUSHGROUND')
# resources
iEternalFruits = gc.getInfoTypeForString('BONUS_ETERNAL_FRUITS')
iUnicorn = gc.getInfoTypeForString('BONUS_UNICORN')
iCorn = gc.getInfoTypeForString('BONUS_CORN')
iHorse = gc.getInfoTypeForString('BONUS_HORSE')

# Units
iWood = gc.getInfoTypeForString('UNIT_WOOD_SPIRIT')
iDryad = gc.getInfoTypeForString('UNIT_DRYAD')
iSprite = gc.getInfoTypeForString('UNIT_SPRITE')
iSunFaerie = gc.getInfoTypeForString('UNIT_SUN_FAERIE')
iSummerLord = gc.getInfoTypeForString('UNIT_SUMMER_LORD')
iTroll = gc.getInfoTypeForString('UNIT_TROLL')
iFurie = gc.getInfoTypeForString('UNIT_FURIE')
iPixie = gc.getInfoTypeForString('UNIT_PIXIE')
iMoonFaerie = gc.getInfoTypeForString('UNIT_MOON_FAERIE')
iWinterLord = gc.getInfoTypeForString('UNIT_WINTER_LORD')
iEternalLord = gc.getInfoTypeForString('UNIT_FAE_LORD')
iWildHunter = gc.getInfoTypeForString('UNIT_WILD_HUNTER')
iErlking = gc.getInfoTypeForString('UNIT_ERLKING')

# Traits
iTraitSpiritual = gc.getInfoTypeForString('TRAIT_SPIRITUAL')
iTraitFallow = gc.getInfoTypeForString('TRAIT_FALLOW')

def onEndFaeriesTurn(argsList):
	'Called at the end of a players turn'
	iGameTurn, iPlayer = argsList
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.getCivilizationType() == iBarbarians or pPlayer.getCivilizationType() == iAnimals:
		return
	py = PyPlayer(iPlayer)

	for pUnit in py.getUnitList():
		# The following effects should also apply to faeries which other players may have (gained by domination, for example)
		FaeriesFunctions.unitGiveWoodsmanToHighLevelFaeries(FaeriesFunctions(), pUnit)
		FaeriesFunctions.unitSetCountdownIfNeeded(FaeriesFunctions(), pUnit)

		if pUnit.isHasPromotion(iEternalFaerie):
			if pUnit.plot().getFeatureType()==iEternalWoods:
				pUnit.setHasPromotion(iPromEternal,true)
			else:
				pUnit.setHasPromotion(iPromEternal,false)
		elif pUnit.isHasPromotion(iSummerFaerie) or pUnit.isHasPromotion(iWinterFaerie):
			pUnit.setHasPromotion(iPromEternal,false)

		#LordShaggynator/BlackArchon: only process units if the Aos Si player reaches this
		if pPlayer.getCivilizationType() == iFaeries:
			#LordShaggynator/BlackArchon: we do not want to process Settlers, Workers, siege units and naval units
			if not (pUnit.getUnitCombatType() in nonFaerieUnitCombats):
				if pUnit.isHasPromotion(iSummerFaerie) or pUnit.isHasPromotion(iWinterFaerie) or pUnit.isHasPromotion(iEternalFaerie):
					colors = FaeriesFunctions.getNumColors(FaeriesFunctions(), pUnit)
					#LordShaggynator/BlackArchon: grant one color to a unit if it hasn't already one
					if colors == 0:
						FaeriesFunctions.setColor(FaeriesFunctions(), pUnit)

	#civics
	if pPlayer.getCivilizationType() == iFaeries:
		if pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicSummerCourt:
			FaeriesFunctions.doSummer(FaeriesFunctions(),iPlayer)
		elif pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicWinterCourt:
			FaeriesFunctions.doWinter(FaeriesFunctions(),iPlayer)
		elif pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicEternalCourt:
			FaeriesFunctions.doEternal(FaeriesFunctions(),iPlayer)

def onBeginFaeriesTurn(argsList):
	bAreThereFaeriesHere = False
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.getCivilizationType()== iFaeries:
			bAreThereFaeriesHere = True
			break
	if bAreThereFaeriesHere:
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getTerrainType() == iNever and pPlot.isOwned() and gc.getPlayer(pPlot.getOwner()).getCivilizationType() != iFaeries:
				pPlot.setTerrainType(iGrass,True,True)
				iBonus=pPlot.getBonusType(pPlayer.getTeam())
				if iBonus == iEternalFruits:
					pPlot.setBonusType(iCorn)
				elif iBonus==iUnicorn:
					pPlot.setBonusType(iHorse)

def onFaerieCreated(argsList):
	'Unit Completed'
	unit = argsList[0]
	player = PyPlayer(unit.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())
	if pPlayer.getCivilizationType() == iFaeries:
		if unit.getUnitCombatType() in nonFaerieUnitCombats:
			return

		if unit.isHasPromotion(iPromFaerie):
			unit.setHasPromotion(iPromFaerie,false)
			if pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicSummerCourt:
				unit.setHasPromotion(iSummerFaerie,true)
			elif pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicWinterCourt:
				unit.setHasPromotion(iWinterFaerie,true)
			elif pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicEternalCourt:
				unit.setHasPromotion(iEternalFaerie,true)

def FaerieCombatResult(argsList):
	'Combat Result'
	pWinner,pLoser = argsList
	pPlayerW = gc.getPlayer(pWinner.getOwner())
	pPlayerL = gc.getPlayer(pLoser.getOwner())

	if pPlayerW.getCivilizationType()==iFaeries:
#		if pWinner.isHasPromotion(iAssimilate):
#			eFaerieTeam = gc.getTeam(pPlayerW.getTeam())
#			eFaerieTeam.changeResearch(pPlayerW.getCurrentResearch(),8,pPlayerW.getID())

		if pWinner.getUnitType()==iWildHunter:
			pUnit.setDuration(pUnit.getDuration()+2)

class FaeriesFunctions:
	def addColorPromotion(self, unit, listColorPromotions):
		iRnd2 = CyGame().getSorenRandNum(len(listColorPromotions), "Fairytale gone bad")
		unit.setHasPromotion(listColorPromotions[iRnd2], True)

	def unitSetCountdownIfNeeded(self, pUnit):
		didSetAnyCountdowns = false
		if pUnit.getUnitType()==iWildHunter:
			if pUnit.getDuration()==0:
				pUnit.setDuration(2)
				didSetAnyCountdowns = true
		if pUnit.getUnitType()==iErlking:
			if pUnit.getDuration()==0:
				pUnit.setDuration(20)
				didSetAnyCountdowns = true

	def unitGiveWoodsmanToHighLevelFaeries(self, pUnit):
		if pUnit.isHasPromotion(iPromWoodsMan1):
			return
		if (pUnit.isHasPromotion(iSummerFaerie) or pUnit.isHasPromotion(iWinterFaerie) or pUnit.isHasPromotion(iEternalFaerie)) and pUnit.getLevel()>3:
			pUnit.setHasPromotion(iPromWoodsMan1,true)

	#disabled because of exploits - Sephi
	def unitConvertLandItsStandingOn(self, pUnit):
		if (not pUnit.plot().isWater()):
			if pUnit.isHasPromotion(iPromWhite):
				iSnowChance=20
				if pUnit.plot().getOwner()==-1:
					iSnowChance+=-10
				if pUnit.plot().getOwner()==pUnit.getOwner():
					iSnowChance=-100
				if iSnowChance>CyGame().getSorenRandNum(100, "snow fall"):
					pUnit.plot().setTerrainType(iSnow,True,True)

			if pUnit.isHasPromotion(iPromGreen):
				if pUnit.plot().getFeatureType()==-1:
					if CyGame().getSorenRandNum(100, "Faerie plant trees") <= 75:
						pUnit.plot().setFeatureType(iForest, 1)

			if pUnit.isHasPromotion(iPromTrueWhite):
				pUnit.plot().setTerrainType(iNever,True,True)

			if pUnit.isHasPromotion(iPromTrueGreen):
				pUnit.plot().setFeatureType(iEternalWoods, 0)

	def convertFaerieLand( self, pPlot, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)

		if ( pPlot.getOwner() != iPlayer ) or pPlot.isWater() or pPlot.isPeak():
			return

		iPlotTerrainType = pPlot.getTerrainType()
		iPlotFeatureType = pPlot.getFeatureType()
		iBonus=pPlot.getBonusType(pPlayer.getTeam())

		# BlackArchon: Explanation for the Summer/Winter/Eternal land changes:
		# 1. Transform land to target type (grass, snow, never never)
		# 2. Transform bonus resources
		# 3.1 For Summer Court: Spread forest
		# 3.2 For Eternal Court: Upgrade or create forest
		if pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicSummerCourt:
			if iPlotTerrainType != iGrass:
				if CyGame().getSorenRandNum(100, "Faerie Terrain Change") < 30:
					if(iPlotTerrainType == iSnow):
						pPlot.setTerrainType(iTundra,True,True)
					elif(iPlotTerrainType == iMarsh):
						pPlot.setTerrainType(iGrass,True,True)
					elif(iPlotTerrainType == iDesert):
						if iPlotFeatureType != iOasis and iPlotFeatureType != iFlood:
							pPlot.setTerrainType(iPlains,True,True)
					elif(iPlotTerrainType == iPlains):
						pPlot.setTerrainType(iGrass,True,True)
					else:
						pPlot.setTerrainType(iPlains,True,True)

			# Convert Eternal Court specific bonus resources to their normal kinds
			if iBonus == iEternalFruits:
				pPlot.setBonusType(iCorn)
			elif iBonus==iUnicorn:
				pPlot.setBonusType(iHorse)


			iPlotTerrainType = pPlot.getTerrainType()
			if( (iPlotTerrainType == iGrass) and (iPlotFeatureType != iForestAncient) and (pPlot.getImprovementType() == -1) and (pPlot.isPeak() == False) and (pPlot.isCity() == False)):
				if iPlotFeatureType != iForest:
					iMod=0
					for i in range (DirectionTypes.NUM_DIRECTION_TYPES):
						pLoopPlot=pPlot
						if i==0:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTH)
						if i==1:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHEAST)
						if i==2:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_EAST)
						if i==3:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHEAST)
						if i==4:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTH)
						if i==5:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHWEST)
						if i==6:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_WEST)
						if i==7:
							pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHWEST)

						if not pLoopPlot.isNone():
							if pLoopPlot!=pPlot:
								if pLoopPlot.getFeatureType()==iForest or pLoopPlot.getFeatureType()==iForestAncient:
									iMod+=10

					if iMod > CyGame().getSorenRandNum(1000, "Forest Spread"):
						pPlot.setFeatureType(iForest, 1)

		elif pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicWinterCourt:
			if iPlotTerrainType != iSnow:
				if CyGame().getSorenRandNum(100, "Faerie Terrain Change") < 30:
					#if(iPlotTerrainType == iPlains):
					#	pPlot.setTerrainType(iTundra,True,True)
					if(iPlotTerrainType == iDesert):
						pPlot.setTerrainType(iPlains,True,True)
					#elif(iPlotTerrainType == iMarsh):
					#	pPlot.setTerrainType(iTundra,True,True)
					elif(iPlotTerrainType == iTundra):
						pPlot.setTerrainType(iSnow,True,True)
					elif(iPlotTerrainType == iDesert):
						pPlot.setTerrainType(iPlains,True,True)
					elif(iPlotTerrainType == iGrass):
						pPlot.setTerrainType(iPlains,True,True)
					else:
						pPlot.setTerrainType(iTundra,True,True)

			# Convert Eternal Court specific bonus resources to their normal kinds
			if iBonus == iEternalFruits:
				pPlot.setBonusType(iCorn)
			elif iBonus==iUnicorn:
				pPlot.setBonusType(iHorse)

		elif pPlayer.getCivics(iCivicOptionFaerieCourt) == iCivicEternalCourt:
			if(iPlotTerrainType == iNever):
				if iBonus == iCorn:
					pPlot.setBonusType(iEternalFruits)
				elif iBonus == iHorse:
					pPlot.setBonusType(iUnicorn)
				iForestUpgradeChance = CyGame().getSorenRandNum(100, "Faerie Forest Upgrade chance")
				if not pPlot.isCity() and iForestUpgradeChance < 30:
					if iPlotFeatureType == iNewForest:
						pPlot.setFeatureType(iForest, 0)
					elif iPlotFeatureType == iForest:
						pPlot.setFeatureType(iForestAncient, 0)
					elif iPlotFeatureType == iForestAncient:
						pPlot.setFeatureType(iEternalWoods, 0)
					elif iPlotFeatureType == iEternalWoods:
						pass
					else:
						pPlot.setFeatureType(iNewForest, 0)
			else:
				if CyGame().getSorenRandNum(100, "Faerie Terrain Change") < 30:
					pPlot.setTerrainType(iNever,True,True)

	def convertUnitForCourt(self, pUnit, courtType):
		if pUnit.getUnitCombatType() in nonFaerieUnitCombats:
			return

		iUnitRace = pUnit.getRace()
		if iUnitRace != iSummerFaerie and iUnitRace != iWinterFaerie and iUnitRace != iEternalFaerie:
			return

		if courtType == iCivicSummerCourt:
			pUnit.setHasPromotion(iWinterFaerie, False)
			pUnit.setHasPromotion(iEternalFaerie, False)
			pUnit.setHasPromotion(iSummerFaerie, True)
			self.convertToNormal(pUnit)
		elif courtType == iCivicWinterCourt:
			pUnit.setHasPromotion(iSummerFaerie, False)
			pUnit.setHasPromotion(iEternalFaerie, False)
			pUnit.setHasPromotion(iWinterFaerie, True)
			self.convertToNormal(pUnit)
		elif courtType == iCivicEternalCourt:
			pUnit.setHasPromotion(iWinterFaerie, False)
			pUnit.setHasPromotion(iSummerFaerie, False)
			pUnit.setHasPromotion(iEternalFaerie, True)

	def doSummer(self, iPlayer):
		py = PyPlayer(iPlayer)
		pPlayer = gc.getPlayer(iPlayer)
		pPlayer.setHasTrait(iTraitSpiritual,True)
		pPlayer.setHasTrait(iTraitFallow,False)

		for i in range (CyMap().numPlots()):
			 pPlot = CyMap().plotByIndex(i)
			 if pPlot.getOwner() == iPlayer:
				self.convertFaerieLand(pPlot,iPlayer)

		for pUnit in py.getUnitList():
			self.convertUnitForCourt(pUnit, iCivicSummerCourt)
			if pUnit.getUnitType() == iPixie:
				newUnit = pPlayer.initUnit(iSprite, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif pUnit.getUnitType() == iFurie:
				newUnit = pPlayer.initUnit(iDryad, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif pUnit.getUnitType() == iMoonFaerie:
				newUnit = pPlayer.initUnit(iSunFaerie, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif pUnit.getUnitType() == iTroll:
				newUnit = pPlayer.initUnit(iWood, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif (pUnit.getUnitType() == iWinterLord or pUnit.getUnitType() == iEternalLord):
				newUnit = pPlayer.initUnit(iSummerLord, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)

	def doWinter(self, iPlayer):
		py = PyPlayer(iPlayer)
		pPlayer = gc.getPlayer(iPlayer)
		pPlayer.setHasTrait(iTraitSpiritual,False)
		pPlayer.setHasTrait(iTraitFallow,True)

		for i in range (CyMap().numPlots()):
			 pPlot = CyMap().plotByIndex(i)
			 if pPlot.getOwner() == iPlayer:
				self.convertFaerieLand(pPlot,iPlayer)
				if pPlot.isVisibleEnemyUnit(iPlayer):
					for j in range(pPlot.getNumUnits()):
						pUnit = pPlot.getUnit(j)
						pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FROZEN'), True)
						pUnit.doDamageNoCaster(2, 20, gc.getInfoTypeForString('DAMAGE_COLD'), False)

		for pUnit in py.getUnitList():
			self.convertUnitForCourt(pUnit, iCivicWinterCourt)
			if pUnit.getUnitType() == iSprite:
				newUnit = pPlayer.initUnit(iPixie, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif pUnit.getUnitType() == iDryad:
				newUnit = pPlayer.initUnit(iFurie, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif pUnit.getUnitType() == iSunFaerie:
				newUnit = pPlayer.initUnit(iMoonFaerie, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif pUnit.getUnitType() == iWood:
				newUnit = pPlayer.initUnit(iTroll, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			elif (pUnit.getUnitType() == iSummerLord or pUnit.getUnitType() == iEternalLord):
				newUnit = pPlayer.initUnit(iWinterLord, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)

	def doEternal(self, iPlayer):
		py = PyPlayer(iPlayer)
		pPlayer = gc.getPlayer(iPlayer)
		pPlayer.setHasTrait(iTraitSpiritual,True)
		pPlayer.setHasTrait(iTraitFallow,False)

		for i in range (CyMap().numPlots()):
			 pPlot = CyMap().plotByIndex(i)
			 if pPlot.getOwner() == iPlayer:
				self.convertFaerieLand(pPlot,iPlayer)

		for pUnit in py.getUnitList():
			self.convertUnitForCourt(pUnit, iCivicEternalCourt)
			#LordShaggynator: Must process Lord conversion before true color change
			if pUnit.getUnitType() == iWinterLord or pUnit.getUnitType() == iSummerLord:
				newUnit = pPlayer.initUnit(iEternalLord, pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.convert(pUnit)
			if pUnit.isHasPromotion(iPromGreen):
				pUnit.setHasPromotion(iPromGreen, False)
				pUnit.setHasPromotion(iPromTrueGreen, True)
			elif pUnit.isHasPromotion(iPromWhite):
				pUnit.setHasPromotion(iPromWhite, False)
				pUnit.setHasPromotion(iPromTrueWhite, True)
			elif pUnit.isHasPromotion(iPromRed):
				pUnit.setHasPromotion(iPromRed, False)
				pUnit.setHasPromotion(iPromTrueRed, True)
			elif pUnit.isHasPromotion(iPromPurple):
				pUnit.setHasPromotion(iPromPurple, False)
				pUnit.setHasPromotion(iPromTruePurple, True)
			elif pUnit.isHasPromotion(iPromClear):
				pUnit.setHasPromotion(iPromClear, False)
				pUnit.setHasPromotion(iPromTrueClear, True)
			elif pUnit.isHasPromotion(iPromBlue):
				pUnit.setHasPromotion(iPromBlue, False)
				pUnit.setHasPromotion(iPromTrueBlue, True)

	def getNumColors(self, pUnit):
		i = 0
		for color in colorList:
			if(pUnit.isHasPromotion(color)):
				i += 1
		return i

	#LordShaggynator: resetting TrueColors (when converting Eternal Court units to Summer/Winter ones)
	def convertToNormal(self, pUnit):
		if pUnit.isHasPromotion(iPromTrueGreen):
			pUnit.setHasPromotion(iPromGreen, True)
			pUnit.setHasPromotion(iPromTrueGreen, False)
		elif pUnit.isHasPromotion(iPromTrueWhite):
			pUnit.setHasPromotion(iPromWhite, True)
			pUnit.setHasPromotion(iPromTrueWhite, False)
		elif pUnit.isHasPromotion(iPromTrueRed):
			pUnit.setHasPromotion(iPromRed, True)
			pUnit.setHasPromotion(iPromTrueRed, False)
		elif pUnit.isHasPromotion(iPromTruePurple):
			pUnit.setHasPromotion(iPromPurple, True)
			pUnit.setHasPromotion(iPromTruePurple, False)
		elif pUnit.isHasPromotion(iPromTrueClear):
			pUnit.setHasPromotion(iPromClear, True)
			pUnit.setHasPromotion(iPromTrueClear, False)
		elif pUnit.isHasPromotion(iPromTrueBlue):
			pUnit.setHasPromotion(iPromBlue, True)
			pUnit.setHasPromotion(iPromTrueBlue, False)

	# Grants a random color to a faerie
	def setColor(self, unit):
		listColorPromotions = []
		if unit.isHasPromotion(iSummerFaerie) or unit.isHasPromotion(iWinterFaerie):
			#Create List of Race Promotions
			listColorPromotions.append(iPromGreen)
			listColorPromotions.append(iPromRed)
			listColorPromotions.append(iPromPurple)
			listColorPromotions.append(iPromWhite)
			if unit.getUnitCombatType() == iDisciple or unit.getUnitCombatType() == iArcane:
				listColorPromotions.append(iPromClear)
			if unit.airBaseCombatStr()>0:
				listColorPromotions.append(iPromBlue)
			self.addColorPromotion(unit, listColorPromotions)
		elif unit.isHasPromotion(iEternalFaerie):
			#Create List of Race Promotions
			listColorPromotions.append(iPromTrueGreen)
			listColorPromotions.append(iPromTrueRed)
			listColorPromotions.append(iPromTruePurple)
			listColorPromotions.append(iPromTrueWhite)
			if unit.getUnitCombatType() == iDisciple or unit.getUnitCombatType() == iArcane:
				listColorPromotions.append(iPromTrueClear)
			if unit.airBaseCombatStr()>0:
				listColorPromotions.append(iPromTrueBlue)
			self.addColorPromotion(unit, listColorPromotions)

def DoGameStartFaeries(argsList):
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		eTerrain = pPlot.getTerrainType()
		if eTerrain == iNever:
			pPlot.setTerrainType(iGrass, False, True)
