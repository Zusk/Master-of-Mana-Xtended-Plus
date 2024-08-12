import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#ARISTRAKH


def SoulVaultinit(argsList):
	'Called at the start of the game'
	for iPlayer in range(gc.getMAX_CIV_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)

		if not pPlayer.isAlive():
			continue

		if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_VAMPIRE_LORD')):
				pPlayer.setPurityCounterCache1(150)
			else:
				pPlayer.setPurityCounterCache1(100)



def doTurnAristrakh(argsList):
	'Run once a Turn'
	iGameTurn  = argsList[0]
	iPlayer = argsList[1]
	pPlayer = gc.getPlayer(iPlayer)
	py = PyPlayer(iPlayer)
	
	#Aristrakh Civilization
	iAristrakh = gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH')

	if not (pPlayer.getCivilizationType() == iAristrakh):
		return

		
	if pPlayer.getNumCities() < 1:
		return
	
	
	# part 0: declarations
	
	
	#Effects
	iPillar = gc.getInfoTypeForString('EFFECT_WRAITH_SUMMON')
	#Terrain and Features
	iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
	iWasteLand = gc.getInfoTypeForString('TERRAIN_WASTELAND')
	iSandStorm = gc.getInfoTypeForString('FEATURE_SANDSTORM')
	#Buildings
	iDefCat = gc.getInfoTypeForString('BUILDING_DEFILING_CATALYST')
	iVoidCat = gc.getInfoTypeForString('BUILDING_VOID_CATALYST')
	iNexPower = gc.getInfoTypeForString('BUILDING_NEXUS_POWER')
	iNexWealth = gc.getInfoTypeForString('BUILDING_NEXUS_WEALTH')
	iNexKnow = gc.getInfoTypeForString('BUILDING_NEXUS_KNOWLEDGE')
	iPalAri = gc.getInfoTypeForString('BUILDING_PALACE_ARISTRAKH')
	iCircDes = gc.getInfoTypeForString('BUILDING_CIRCLE_OF_DESPAIR')
	iCircBlo = gc.getInfoTypeForString('BUILDING_CIRCLE_OF_BLOOD')
	iTWell = gc.getInfoTypeForString('BUILDING_TAINTED_WELL')
	iUGlyph = gc.getInfoTypeForString('BUILDING_UNHOLY_GLYPHS')
	iSPrison = gc.getInfoTypeForString('BUILDING_SOUL_PRISON')
	iSFunnel = gc.getInfoTypeForString('BUILDING_SOUL_FUNNEL')
	iCatac = gc.getInfoTypeForString('BUILDING_CATACOMBS')
	iCShrine = gc.getInfoTypeForString('BUILDING_CABAL_SHRINE')
	iCGPact = gc.getInfoTypeForString('BUILDING_GRAVE_PACT')
	iWitherH = gc.getInfoTypeForString('BUILDING_WITHERING_HEX')
	#removed improvements
	iCamp = gc.getInfoTypeForString('IMPROVEMENT_CAMP')
	iCottage =  gc.getInfoTypeForString('IMPROVEMENT_COTTAGE')
	iTown =  gc.getInfoTypeForString('IMPROVEMENT_TOWN')
	iWaterMill =  gc.getInfoTypeForString('IMPROVEMENT_WATERMILL')
	iWell =  gc.getInfoTypeForString('IMPROVEMENT_GROUNDWATER_WELL')
	iLumberMill =  gc.getInfoTypeForString('IMPROVEMENT_LUMBERMILL')
	iEnclave =  gc.getInfoTypeForString('IMPROVEMENT_ENCLAVE')
	iTrade1 =  gc.getInfoTypeForString('IMPROVEMENT_TRADE1')
	iTrade2 =  gc.getInfoTypeForString('IMPROVEMENT_TRADE2')
	iTrade3 =  gc.getInfoTypeForString('IMPROVEMENT_TRADE3')
	iFarm =  gc.getInfoTypeForString('IMPROVEMENT_FARM')
	iFarmM =  gc.getInfoTypeForString('IMPROVEMENT_FARM_MALAKIM')
	iHamlet =  gc.getInfoTypeForString('IMPROVEMENT_HAMLET')
	iPasture =  gc.getInfoTypeForString('IMPROVEMENT_PASTURE')
	iPlantation =  gc.getInfoTypeForString('IMPROVEMENT_PLANTATION')
	iVillage =  gc.getInfoTypeForString('IMPROVEMENT_VILLAGE')
	iWindmill =  gc.getInfoTypeForString('IMPROVEMENT_WINDMILL')
	iWinery =  gc.getInfoTypeForString('IMPROVEMENT_WINERY')
	iWorkshop =  gc.getInfoTypeForString('IMPROVEMENT_WORKSHOP')
	iAWorkshop = gc.getInfoTypeForString('IMPROVEMENT_ARTISAN_WORKSHOP')
	iCarav1 = gc.getInfoTypeForString('IMPROVEMENT_CARAVANSARY')
	iCarav2 = gc.getInfoTypeForString('IMPROVEMENT_CARAVAN_CENTER')
	iFLodge = gc.getInfoTypeForString('IMPROVEMENT_FOREST_LODGE')
	iGer = gc.getInfoTypeForString('IMPROVEMENT_GER')
	iHome = gc.getInfoTypeForString('IMPROVEMENT_HOMESTEAD')
	iLWindmill = gc.getInfoTypeForString('IMPROVEMENT_LARGE_WINDMILL')
	iYaranga = gc.getInfoTypeForString('IMPROVEMENT_YARANGA')
	
	unwantedImprovementsList = [iCamp, iCottage, iTown, iWaterMill, iWell, iLumberMill, iEnclave, iTrade1, iTrade2, iTrade3, iFarm, iFarmM, iHamlet, iPasture, iPlantation, iVillage, iWindmill, iWinery, iWorkshop, iAWorkshop, iCarav1, iCarav2, iFLodge, iGer, iHome, iLWindmill, iYaranga]
	#Defiler, catacombs & crypt
	iDR1 = gc.getInfoTypeForString('IMPROVEMENT_DEFILER1')
	iDR2 = gc.getInfoTypeForString('IMPROVEMENT_DEFILER2')
	iDR3 = gc.getInfoTypeForString('IMPROVEMENT_DEFILER3')
	inumDR1 = 0
	inumDR2 = 0
	iDR3 = gc.getInfoTypeForString('IMPROVEMENT_DEFILER3')
	iDR4 = gc.getInfoTypeForString('IMPROVEMENT_CATACOMBS')
	iDR5 = gc.getInfoTypeForString('IMPROVEMENT_CRYPT')
	inumDR1 = 0
	inumDR2 = 0
	inumDR3 = 0
	inumDR4 = 0
	inumDR5 = 0
	#Mana
	iMana = gc.getInfoTypeForString('BONUS_MANA')
	iManaLife = gc.getInfoTypeForString('BONUS_MANA_LIFE')
	iManaNature = gc.getInfoTypeForString('BONUS_MANA_NATURE')		
	GoodMana = [iManaLife, iManaNature]
	

	# part 1: soul income and terraforing  
	
	
	#Death Mana income
	iDeathMana = pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_MANA_DEATH'))
	itemp = pPlayer.getPurityCounterCache1()
	pPlayer.setPurityCounterCache1(itemp + iDeathMana)
	
	#Ai help
	if pPlayer.isHuman() == False:
		iSouls = pPlayer.getPurityCounterCache1()
		inumC = pPlayer.getNumCities()
		if iSouls*3 < inumC:
			pPlayer.setPurityCounterCache1(iSouls + 3*inumC)
		
	#number of Catalysts
	inumVoidC = pPlayer.countNumBuildings(iVoidCat)
	inumDefC = pPlayer.countNumBuildings(iDefCat)
	#number of soul generating buildings
	inumSPrison = pPlayer.countNumBuildings(iSPrison)
	inumSFunnel = pPlayer.countNumBuildings(iSFunnel)
	#Soul Income from Void Catalysts and other buildings
	if inumVoidC > 0:
		itemp = pPlayer.getPurityCounterCache1()
		pPlayer.setPurityCounterCache1(itemp + 4*inumVoidC + inumSFunnel + inumSPrison)
		
	#passive terraforming
	iOnceRes = 1 + inumVoidC + inumDefC

	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		iTerrain = pPlot.getTerrainType()
		if pPlot.isOwned(): 
			pPlayer2 = gc.getPlayer(pPlot.getOwner())
			#reverses wasteland spread in other players borders
			if not pPlayer2.getCivilizationType() == iAristrakh:
				if iTerrain == iWasteLand:
					if CyGame().getSorenRandNum(100, "heal Terrain") < 10:
						pPlot.setTerrainType(iDesert,true,true)
			if pPlayer2.getCivilizationType() == iAristrakh:
				iBonus = pPlot.getBonusType(-1)
				iImprovement = pPlot.getImprovementType()
				iFeature = pPlot.getFeatureType()
				if iBonus != -1:
					#converts nature and life mana to normal mana
					if iBonus in GoodMana:
						pPlot.setBonusType(iMana)
					#remove resources from defiler improvements and consumes terrain
					if iImprovement == iDR1:
						pPlot.setBonusType(-1)
						AristrakhFunctions.doConsumeTerrain(AristrakhFunctions(),pPlayer,pPlot)
				#remove unwanted improvements and counts Defilers
				if iImprovement != -1:
					if iImprovement in unwantedImprovementsList:
						pPlot.setImprovementType(-1)
					elif iImprovement == iDR1:
						inumDR1 = inumDR1 + 1
					elif iImprovement == iDR2:
						inumDR2 = inumDR2 + 1
					elif iImprovement == iDR3:
						inumDR3 = inumDR3 + 1
					elif iImprovement == iDR4:
						inumDR3 = inumDR4 + 1
					elif iImprovement == iDR5:
						inumDR3 = inumDR5 + 1
				#spread sandstorms
				if iTerrain == iWasteLand:
					if iFeature == -1 and pPlot.isPeak() == False and pPlot.isCity() == False and iImprovement == -1:
						if CyGame().getSorenRandNum(1000, "spread Sandstorm") < 10:
							pPlot.setFeatureType(iSandStorm, 0)
					elif iFeature == iSandStorm:
						if CyGame().getSorenRandNum(1000, "remove Sandstorm") < 100:
							pPlot.setFeatureType(-1, 0)					
				#defiles non hell terrain
				elif iOnceRes > 0:
					#no defiling of empty water tiles
					if not (iFeature == -1 and iBonus == -1 and pPlot.isWater()):
						#has to be chance based else terraforming is row wise
						if CyGame().getSorenRandNum(100, "consume Terrain") < (10 + 4*inumVoidC + 2*inumDefC):
							AristrakhFunctions.doConsumeTerrain(AristrakhFunctions(),pPlayer,pPlot)
							iOnceRes = iOnceRes - 1
						
	#Income from Defilers
	itemp = pPlayer.getPurityCounterCache1()
	pPlayer.setPurityCounterCache1(itemp + inumDR1 + inumDR2 + inumDR3 + inumDR4 + inumDR5)
	
		
	#part 2:  soul spending in cities

	
	#reserve income for capital (4 is default palace income)
	iReserve = 4
	if pPlayer.countNumBuildings(iCircBlo) > 0:
		iReserve = 6
	if pPlayer.countNumBuildings(iCircDes) > 0:	
		iReserve = 8
	if 	pPlayer.countNumBuildings(iCatac) > 0:
		iReserve = iReserve + 4
		if pPlayer.countNumBuildings(iCShrine) > 0:
			iReserve = iReserve + 5
			if pPlayer.countNumBuildings(iCGPact) > 0:
				iReserve = iReserve + 7
	
	
	
	for pyCity in PyPlayer(iPlayer).getCityList():
		pCity = pyCity.GetCy()
		iSouls = pPlayer.getPurityCounterCache1()
		iBuildIncome = 1
		iSoulIn = 0
		pPlot = pCity.plot()
		if pCity.getNumRealBuilding(iPalAri) > 0:
			iBuildIncome = 4
			iReserve = 0
		if pCity.getNumRealBuilding(iCircBlo) > 0:
			iBuildIncome = 6
		if pCity.getNumRealBuilding(iCircDes) > 0:
			iBuildIncome = 8
		if pCity.getNumRealBuilding(iDefCat) > 0:
			iBuildIncome = 2
		if pCity.getNumRealBuilding(iNexPower) > 0:
			iBuildIncome = 3
		if pCity.getNumRealBuilding(iNexWealth) > 0:
			iBuildIncome = 3
		if pCity.getNumRealBuilding(iNexKnow) > 0:
			iBuildIncome = 3
		if pCity.getNumRealBuilding(iTWell) > 0:
			iBuildIncome = iBuildIncome + 1
		if pCity.getNumRealBuilding(iUGlyph) > 0:
			iBuildIncome = iBuildIncome + 1
		if pCity.getNumRealBuilding(iCatac) > 0:
			iBuildIncome = iBuildIncome + 4
		if pCity.getNumRealBuilding(iCShrine) > 0:
			iBuildIncome = iBuildIncome + 5
		if pCity.getNumRealBuilding(iCGPact) > 0:
			iBuildIncome = iBuildIncome + 7
		#must be last building check
		if pCity.getNumRealBuilding(iVoidCat) > 0:
			iBuildIncome = 0
		#Convertes Souls from Vault to City
		if iSouls - iReserve > 0:
			if iSouls < iBuildIncome:
				iSoulIn = iSouls
			else:
				iSoulIn = iBuildIncome
			#city only grows if happy
			iHappy = pCity.happyLevel()
			iUnhappy = pCity.unhappyLevel(0)
			#stop adding souls when just short of next pop level
			if iHappy - iUnhappy >= 0:
				iCurrentFood = pCity.getFood()
				iMaxFood = pCity.growthThreshold()
				iOverflow = iMaxFood - iCurrentFood - iSoulIn
				#added souls will not go to next poplevel
				if iHappy == iUnhappy and iOverflow < 1:
					iSoulIn = iSoulIn + iOverflow - 1
				#adds souls to city
				pCity.changeFood(iSoulIn)
				#substracts consumed souls from soul vault
				itemp = pPlayer.getPurityCounterCache1()
				pPlayer.setPurityCounterCache1(itemp - iSoulIn)
				#checks if city growths
				if pCity.getFood() >= pCity.growthThreshold():
					pCity.changeFood(- (pCity.growthThreshold()))
					pCity.changePopulation(1)
					CyEngine().triggerEffect(iPillar,pPlot.getPoint())
		#Bonus Effects from Consuming Souls
		iBonus = 0
		iBonusBase = 0
		#Defiling Catalyst adds 1 commerce per soul
		if iSoulIn > 0:
			iBonusBase = iSoulIn
		if pCity.getNumRealBuilding(iDefCat) > 0:					
			pCity.setBuildingYieldChange(gc.getBuildingInfo(iDefCat).getBuildingClassType(), YieldTypes.YIELD_COMMERCE, iBonusBase)
		#Nexus of Power adds 5 spell research per soul
		if pCity.getNumRealBuilding(iNexPower) > 0:
			iBonus = iBonusBase*5					
			pCity.setBuildingCommerceChange(gc.getBuildingInfo(iNexPower).getBuildingClassType(), CommerceTypes.COMMERCE_ARCANE, iBonus)
		#Nexus of Wealth adds 4 gold per soul
		if pCity.getNumRealBuilding(iNexWealth) > 0:
			iBonus = iBonusBase*4					
			pCity.setBuildingCommerceChange(gc.getBuildingInfo(iNexWealth).getBuildingClassType(), CommerceTypes.COMMERCE_GOLD, iBonus)
		#Nexus of Knowledge adds 5 research per soul
		if pCity.getNumRealBuilding(iNexKnow) > 0:
			iBonus = iBonusBase*5
			pCity.setBuildingCommerceChange(gc.getBuildingInfo(iNexKnow).getBuildingClassType(), CommerceTypes.COMMERCE_RESEARCH, iBonus)
		#Circle of Blood adds 0.5 production and 1 commerce per soul
		if pCity.getNumRealBuilding(iCircBlo) > 0:
			iBonus = iBonusBase//2
			pCity.setBuildingYieldChange(gc.getBuildingInfo(iCircBlo).getBuildingClassType(), YieldTypes.YIELD_PRODUCTION, iBonus)
			pCity.setBuildingYieldChange(gc.getBuildingInfo(iCircBlo).getBuildingClassType(), YieldTypes.YIELD_COMMERCE, iBonusBase)
		#Circle of Despair adds 1 culture and 1 commerce per soul
		if pCity.getNumRealBuilding(iCircDes) > 0:
			pCity.setBuildingCommerceChange(gc.getBuildingInfo(iCircDes).getBuildingClassType(), CommerceTypes.COMMERCE_CULTURE, iBonusBase)
			pCity.setBuildingYieldChange(gc.getBuildingInfo(iCircDes).getBuildingClassType(), YieldTypes.YIELD_COMMERCE, iBonusBase)
		#Withering Hex adds 1 mana per soul	
		if pCity.getNumRealBuilding(iWitherH) > 0:
			pCity.setBuildingCommerceChange(gc.getBuildingInfo(iWitherH).getBuildingClassType(), CommerceTypes.COMMERCE_MANA, iBonusBase)
		
		
def onAriBuildingBuilt(argsList):
	'Building Completed'
	pCity, iBuildingType = argsList
	player = pCity.getOwner()
	pPlayer = gc.getPlayer(player)
	pPlot = pCity.plot()
	game = gc.getGame()
	
	if pPlayer.getCivilizationType()!=gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
		return
		
		
	if iBuildingType == gc.getInfoTypeForString('BUILDING_VOID_CATALYST'):
		iSoulsCity = pCity.getFood()
		if iSoulsCity > 0:
			#sets food to base value, i.e. 0 
			pCity.changeFood(-1*(iSoulsCity))
			#adds souls back to the vault
			itemp = pPlayer.getPurityCounterCache1()
			pPlayer.setPurityCounterCache1(itemp + iSoulsCity)

						
def onAristrakhProjectBuilt(argsList):
	'Project Completed'
	pCity, iProjectType = argsList
	game = gc.getGame()
	iPlayer = pCity.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlotC = pCity.plot()
	
	if iProjectType == gc.getInfoTypeForString('PROJECT_DEATHS_EMBRACE'):
		pPlayer.setCivilizationType(gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'))
		pPlayer.setHasTrait(gc.getInfoTypeForString('TRAIT_FALLOW'),true)
		pPlayer.setHasTrait(gc.getInfoTypeForString('TRAIT_AGNOSTIC'),true)
		pPlayer.setHasTrait(gc.getInfoTypeForString('TRAIT_WASTELAND_ECON'),true)
		pPlayer.setLastStateReligion(gc.getInfoTypeForString('NO_RELIGION'))

		for pUnit in PyPlayer(iPlayer).getUnitList():
			if pPlayer.getNumUnits()>2:
				if pUnit.isAlive() and pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE'))==false:
					pUnit.kill(false,-1)

		for pyCity in PyPlayer(iPlayer).getCityList():
			pCity = pyCity.GetCy()	
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNDEAD_CITY'),1)
		

def BattleFrenzy(argsList):
	'Combat Result'
	pWinner,pLoser = argsList
	
	pPlayer = gc.getPlayer(pWinner.getOwner())
	
	if pPlayer.getCivilizationType()!=gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
		return
	if not pLoser.isAlive():
		return
	
	#combat growth bonus
	iTemp = pPlayer.getPurityCounterCache1()
	iStrL = pLoser.baseCombatStr()
	iLvlL = pLoser.getLevel()
	iCombatIncome = iLvlL
	iStrengthBonus = iStrL//2
	iVampBonus = 0
	iTraitBonus = 0
	
	#bonus income for najesh	
	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_HERALD_OF_DEATH')):
		iTraitBonus = iStrL//4

	if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SOULDRINKER')):
		#bonus income for vampires
		iVampBonus = iStrL//4
		#murderous frenzy mechanic
		if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY1')):
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY1'),false)
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY2'),true)
		elif pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY2')):
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY2'),false)
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY3'),true)
		elif pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY3')):
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY3'),false)
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY4'),true)
		elif pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY4')):
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY4'),false)
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY5'),true)
		elif pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY5')):
			pass
		else:
			pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MURDEROUS_FRENZY1'),true)

	#overall income
	pPlayer.setPurityCounterCache1(iTemp + iCombatIncome + iTraitBonus + iVampBonus + iStrengthBonus)
		
def onAristrakhCityAcquired(argsList):
	'City Acquired'
	iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
	pPlayer = gc.getPlayer(iNewOwner)
	pPlot = pCity.plot()
	
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
		iTraitMod=1
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_HERALD_OF_DEATH')):
			iTraitMod=2
		iConqPop = pCity.getPopulation()
		pCity.setPopulation(1)
		iTemp = pPlayer.getPurityCounterCache1()
		pPlayer.setPurityCounterCache1(iTemp + iConqPop*10*iTraitMod)
		AristrakhFunctions.doConsumeTerrain(AristrakhFunctions(),pPlayer,pPlot)
		
def onAristrakhCityBuilt(argsList):
	'City Built'
	city = argsList[0]
	pPlot = city.plot()
	pPlayer = gc.getPlayer(city.getOwner())
	
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
		AristrakhFunctions.doConsumeTerrain(AristrakhFunctions(),pPlayer,pPlot)
		
def UnitSoulCost(argsList):
	'Unit Completed'
	unit = argsList[0]
	player = PyPlayer(unit.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())

	if pPlayer.getCivilizationType()!=gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
		return
		
	#vampire lord trait	
	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_VAMPIRE_LORD')):
		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE')):
			unit.setHasPromotion(gc.getInfoTypeForString("PROMOTION_UNDEAD_COMMAND"),true)
				
	iUnitType=unit.getUnitType()
	#certain unit costs souls
	iUnitCost = 0
	if iUnitType==gc.getInfoTypeForString('UNIT_UD_GENERAL'):
		iUnitCost = 15
	elif unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE')):
		iUnitCost = 30
		if iUnitType==gc.getInfoTypeForString('UNIT_NIKEPHOROS'):
			iUnitCost = 50
	elif iUnitType==gc.getInfoTypeForString('UNIT_NOFUOS'):
		iUnitCost = 50
	#substract cost from soul vault
	if iUnitCost > 0:
		iTemp = pPlayer.getPurityCounterCache1()
		pPlayer.setPurityCounterCache1(iTemp - iUnitCost)


		
class AristrakhFunctions:

	def doConsumeTerrain(self, pPlayer, pPlot):
		iPlayer = pPlayer.getID()
		iTerrain = pPlot.getTerrainType()
		iFeature = pPlot.getFeatureType()
		iBonus = pPlot.getBonusType(-1)
		iImprovement = pPlot.getImprovementType()
		#Hell Terrain
		iBrokenLands = gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS')
		iShallows = gc.getInfoTypeForString('TERRAIN_SHALLOWS')
		iPerdition = gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')
		iBruningSands = gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')
		HellTerrain = [iBrokenLands, iShallows, iPerdition, iBruningSands]
		#terrain									
		iWasteLand = gc.getInfoTypeForString('TERRAIN_WASTELAND')
		#soul income
		iTerrainConv = 0
		iFeatureBonus = 0
		#terraform effect
		iTFEffect = gc.getInfoTypeForString('EFFECT_CONTAGION')
		#features
		iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
		iLizJungle = gc.getInfoTypeForString('FEATURE_LIZ_JUNGLE')
		iForest = gc.getInfoTypeForString('FEATURE_FOREST')
		iBurntForest = gc.getInfoTypeForString('FEATURE_FOREST_BURNT')
		iNewForest = gc.getInfoTypeForString('FEATURE_FOREST_NEW')
		iEterW = gc.getInfoTypeForString('FEATURE_ETERNAL_WOODS')
		iAncFor = gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')
		Forests = [iJungle, iLizJungle, iForest, iNewForest, iEterW, iAncFor]
		
		#terrain conversion
		if not pPlot.isWater():
			#no hell terrain conversion
			if iTerrain in HellTerrain:
				return
			else:
				pPlot.setTerrainType(iWasteLand,true,true)
				CyEngine().triggerEffect(iTFEffect,pPlot.getPoint())
				iTerrainConv = 8
				#every tile defiled has diplomatic penalty
				for i in range(gc.getMAX_CIV_PLAYERS()):
					if gc.getPlayer(i).getTeam()!=pPlayer.getTeam():
						gc.getPlayer(i).AI_changeMemoryCount(iPlayer, MemoryTypes.MEMORY_MAGIC_CHARM, -10)
		#removes water resources and improvements
		elif iBonus != -1:
			pPlot.setBonusType(-1)
			if iImprovement != -1:
				pPlot.setImprovementType(-1)
			iTerrainConv = 30
			CyEngine().triggerEffect(iTFEffect,pPlot.getPoint())
			for i in range(gc.getMAX_CIV_PLAYERS()):
				if gc.getPlayer(i).getTeam()!=pPlayer.getTeam():
					gc.getPlayer(i).AI_changeMemoryCount(iPlayer, MemoryTypes.MEMORY_MAGIC_CHARM, -30)
		#feature conversion
		if iFeature != -1:
			#replaces forests with burnt forest
			if pPlot.isCity() == False and iFeature in Forests:
				pPlot.setFeatureType(iBurntForest, 0)
			#removes feature gives bonus souls
			else:
				iFeatureBonus = 2
				pPlot.setFeatureType(-1,1)
				
		#overall soul income
		itemp = pPlayer.getPurityCounterCache1()
		pPlayer.setPurityCounterCache1(itemp + iFeatureBonus + iTerrainConv)
