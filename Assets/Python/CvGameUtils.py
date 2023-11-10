## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
##
## Implementaion of miscellaneous game functions

import CvUtil
from CvPythonExtensions import *
import CvEventInterface
import CustomFunctions

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

# globals
cf = CustomFunctions.CustomFunctions()
gc = CyGlobalContext()



class CvGameUtils:
	"Miscellaneous game functions"
	def __init__(self):
		pass

	def isVictoryTest(self):
		if ( gc.getGame().getElapsedGameTurns() > 10 ):
			return True
		else:
			return False

	def isVictory(self, argsList):
		eVictory = argsList[0]
		return True

	def isPlayerResearch(self, argsList):
		ePlayer = argsList[0]
		return True

	#disabled
	def getExtraCost(self, argsList):
		ePlayer = argsList[0]
		return 0

	def createBarbarianCities(self):
		return False

	def createBarbarianUnits(self):
		return False

	def skipResearchPopup(self,argsList):
		ePlayer = argsList[0]
		return False

	def showTechChooserButton(self,argsList):
		ePlayer = argsList[0]
		return True

	def getFirstRecommendedTech(self,argsList):
		ePlayer = argsList[0]
		return TechTypes.NO_TECH

	def getSecondRecommendedTech(self,argsList):
		ePlayer = argsList[0]
		eFirstTech = argsList[1]
		return TechTypes.NO_TECH

	def canRazeCity(self,argsList):
		iRazingPlayer, pCity = argsList
		return True

	def canDeclareWar(self,argsList):
		iAttackingTeam, iDefendingTeam = argsList
		return True

	def skipProductionPopup(self,argsList):
		pCity = argsList[0]
		return False

	def showExamineCityButton(self,argsList):
		pCity = argsList[0]
		return True

	def getRecommendedUnit(self,argsList):
		pCity = argsList[0]
		return UnitTypes.NO_UNIT

	def getRecommendedBuilding(self,argsList):
		pCity = argsList[0]
		return BuildingTypes.NO_BUILDING

	def updateColoredPlots(self):
		return False

	def isActionRecommended(self,argsList):
		pUnit = argsList[0]
		iAction = argsList[1]
		return False

	def unitCannotMoveInto(self,argsList):
		ePlayer = argsList[0]
		iUnitId = argsList[1]
		iPlotX = argsList[2]
		iPlotY = argsList[3]
		return False

	def cannotHandleAction(self,argsList):
		pPlot = argsList[0]
		iAction = argsList[1]
		bTestVisible = argsList[2]
		return False

	def canBuild(self,argsList):
		iX, iY, iBuild, iPlayer = argsList
		pPlayer = gc.getPlayer(iPlayer)
		eTeam = gc.getTeam(pPlayer.getTeam())
		pPlot = CyMap().plot(iX, iY)
		iterrain=pPlot.getTerrainType()

		if pPlayer.isHuman() == False:
			if iBuild == gc.getInfoTypeForString('BUILD_FORT'):
				return 0

			if iBuild==gc.getInfoTypeForString('BUILD_ROAD'):
				if pPlot.isWater():
					return 0

			if iBuild==gc.getInfoTypeForString('BUILD_COTTAGE'):
				eAIEcon=pPlayer.AI_getEconomyType()
				if eAIEcon==AIEconomyTypes.AIECONOMY_TRADE:
					return 0

		if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_MALAKIM'):
			if iterrain==gc.getInfoTypeForString('TERRAIN_DESERT'):
				if iBuild == gc.getInfoTypeForString('BUILD_ROAD'):
					return 0

		return -1	# Returning -1 means ignore; 0 means Build cannot be performed; 1 or greater means it can

	def cannotFoundCity(self,argsList):
		iPlayer, iPlotX, iPlotY = argsList
		pPlot = CyMap().plot(iPlotX,iPlotY)
		return False

	def cannotSelectionListMove(self,argsList):
		pPlot = argsList[0]
		bAlt = argsList[1]
		bShift = argsList[2]
		bCtrl = argsList[3]
		return False

	def cannotSelectionListGameNetMessage(self,argsList):
		eMessage = argsList[0]
		iData2 = argsList[1]
		iData3 = argsList[2]
		iData4 = argsList[3]
		iFlags = argsList[4]
		bAlt = argsList[5]
		bShift = argsList[6]
		return False

	def cannotDoControl(self,argsList):
		eControl = argsList[0]
		return False

	def canResearch(self,argsList):
		ePlayer = argsList[0]
		eTech = argsList[1]
		bTrade = argsList[2]
		return False

	def cannotResearch(self,argsList):
		ePlayer = argsList[0]
		eTech = argsList[1]
		bTrade = argsList[2]
		pPlayer = gc.getPlayer(ePlayer)
		iCiv = pPlayer.getCivilizationType()
		eTeam = gc.getTeam(pPlayer.getTeam())
				
		return False

	def canDoCivic(self,argsList):
		ePlayer = argsList[0]
		eCivic = argsList[1]
		return False

	def cannotDoCivic(self,argsList):
		ePlayer = argsList[0]
		eCivic = argsList[1]
		pPlayer = gc.getPlayer(ePlayer)
		eTeam = gc.getTeam(pPlayer.getTeam())

		pPlayer = gc.getPlayer(ePlayer)

		if eCivic == gc.getInfoTypeForString('CIVIC_SLAVERY'):
			if gc.getPlayer(ePlayer).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
				if pPlayer.isHuman() == False:
					return True

#Faeries
		if gc.getPlayer(ePlayer).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_FAERIES'):
			if eCivic == gc.getInfoTypeForString('CIVIC_NONE'):
				return True

			if eCivic == gc.getInfoTypeForString('CIVIC_ETERNAL_COURT'):
#				if not pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FAE')):
#					return True
				if eTeam.getProjectCount(gc.getInfoTypeForString('PROJECT_ETERNAL_RESTORATION')) < 1:
					return True
#Faeries
#Jotnar
		if not gc.getInfoTypeForString('CIVILIZATION_JOTNAR')==-1:
			if eCivic == gc.getInfoTypeForString('CIVIC_NONE'):
				if gc.getPlayer(ePlayer).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_JOTNAR'):
					return True
#Jotnar

#Aristrakh
		if gc.getPlayer(ePlayer).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
			if eCivic == gc.getInfoTypeForString('CIVIC_NONE'):
				return True



		return False

	def canTrain(self,argsList):
		pCity = argsList[0]
		eUnit = argsList[1]
		bContinue = argsList[2]
		bTestVisible = argsList[3]
		bIgnoreCost = argsList[4]
		bIgnoreUpgrades = argsList[5]
		return False

	def cannotTrain(self,argsList):
		pCity = argsList[0]
		eUnit = argsList[1]
		bContinue = argsList[2]
		bTestVisible = argsList[3]
		bIgnoreCost = argsList[4]
		bIgnoreUpgrades = argsList[5]
		ePlayer = pCity.getOwner()
		pPlayer = gc.getPlayer(ePlayer)
		eUnitClass = gc.getUnitInfo(eUnit).getUnitClassType()
		eUnitCombat = gc.getUnitInfo(eUnit).getUnitCombatType()		
		eTeam = gc.getTeam(pPlayer.getTeam())

		if pPlayer.isHuman() == False:
			infoCiv = gc.getCivilizationInfo(pPlayer.getCivilizationType())
			if eUnitClass == gc.getInfoTypeForString('UNITCLASS_SCOUT'):
				if pPlayer.getUnitClassCountPlusMaking(gc.getInfoTypeForString('UNITCLASS_SCOUT')) >= 1:
					return True

#			if eUnit == gc.getInfoTypeForString('UNIT_WORKER'):
			if eUnitClass == gc.getInfoTypeForString('UNITCLASS_WORKER'):
				if 2 * pPlayer.getNumCities() <= pPlayer.getUnitClassCountPlusMaking(gc.getInfoTypeForString('UNITCLASS_WORKER')) :
					return True

#			if eUnitClass == gc.getInfoTypeForString('UNITCLASS_WARRIOR'):
#				civtype = pPlayer.getCivilizationType()
#				infoCiv = gc.getCivilizationInfo(civtype)
#				iUnit = infoCiv.getCivilizationUnits(gc.getInfoTypeForString('UNITCLASS_AXEMAN'))
#					if pCity.canTrain(iUnit,False,False):
#						return True
#				iUnit = infoCiv.getCivilizationUnits(gc.getInfoTypeForString('UNITCLASS_ARCHER'))
#					if pCity.canTrain(iUnit,False,False):
#						return True


			if eUnitClass == gc.getInfoTypeForString('UNITCLASS_ADEPT'):
				if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_ADEPT')) > 3+pPlayer.getNumCities()*2:
					if not (pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_ARCANE')) or pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_ARCANE_ADAPTIVE'))) :
						if not eTeam.isHasTech(gc.getInfoTypeForString('TECH_SORCERY')):
							return True
					elif not pPlayer.isConquestMode():
						return True
						
			if eUnit == gc.getInfoTypeForString('UNIT_AIRSHIP'):
				return True
#			if eUnit == gc.getInfoTypeForString('UNIT_LIGHTBRINGER'):
#				return True

			if eUnit == gc.getInfoTypeForString('UNIT_PRIVATEER'):
				iPrivateercounter = 0
				for iTeam2 in range(gc.getMAX_PLAYERS()):
					eTeam2 = gc.getTeam(iTeam2)
					if eTeam2.isAlive():
						if eTeam2.isHasTech(gc.getInfoTypeForString('TECH_OPTICS')) and eTeam2.isHasTech(gc.getInfoTypeForString('TECH_IRON_WORKING')):
							iPrivateercounter = iPrivateercounter +1
				if iPrivateercounter > 1:
					return True

			if eUnit == gc.getInfoTypeForString('UNIT_EXPLORER1'):
				if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_EXPLORER1')) > 2:
					return True
#xtended
			if eUnitClass == gc.getInfoTypeForString('UNITCLASS_CATAPULT'):
				if pPlayer.getUnitClassCountPlusMaking(gc.getInfoTypeForString('UNITCLASS_CATAPULT')) >= 4:
					return True

			if eUnitClass == gc.getInfoTypeForString('UNITCLASS_TREBUCHET'):
				if pPlayer.getUnitClassCountPlusMaking(gc.getInfoTypeForString('UNITCLASS_TREBUCHET')) >= 4:
					return True

			if eUnitClass == gc.getInfoTypeForString('UNITCLASS_CANNON'):
				if pPlayer.getUnitClassCountPlusMaking(gc.getInfoTypeForString('UNITCLASS_CANNON')) >= 8:
					return True

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SPRAWLING')):
			if not pPlayer.isHuman():
				if (pPlayer.getNumCities() + pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_SETTLER'))) >= pPlayer.getMaxCities():
					if eUnit == gc.getInfoTypeForString('UNIT_SETTLER'):
						return True

		if eUnit == gc.getInfoTypeForString('UNIT_BEAST_OF_AGARES'):
			if pCity.getPopulation() <= 5:
				return True

#faeries
		if not gc.getInfoTypeForString('CIVILIZATION_FAERIES')==-1:
			if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) == gc.getInfoTypeForString('CIVIC_SUMMER_COURT'):
				if eUnit == gc.getInfoTypeForString('UNIT_PIXIE'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_FURIE'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_TROLL'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_MOON_FAERIE'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_WINTER_LORD'):
					return True

			if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) == gc.getInfoTypeForString('CIVIC_WINTER_COURT'):
				if eUnit == gc.getInfoTypeForString('UNIT_SPRITE'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_DRYAD'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_WOOD_SPIRIT'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_SUN_FAERIE'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_SUMMER_LORD'):
					return True

			if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) == gc.getInfoTypeForString('CIVIC_ETERNAL_COURT'):
				if eUnit == gc.getInfoTypeForString('UNIT_SUMMER_LORD'):
					return True
				if eUnit == gc.getInfoTypeForString('UNIT_WINTER_LORD'):
					return True
#faeries

#Aristrakh
		if not gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH')==-1:
			iSoulVault = pPlayer.getPurityCounterCache1()
			if not bTestVisible:
				if iSoulVault < 15:
					if eUnit == gc.getInfoTypeForString('UNIT_UD_GENERAL'):
						return True
				if iSoulVault < 30:
					if eUnit == gc.getInfoTypeForString('UNIT_MOUNTED_VAMPIRE'):
						return True
					if eUnit == gc.getInfoTypeForString('UNIT_DRAGONRIDER'):
						return True
					if eUnit == gc.getInfoTypeForString('UNIT_VAMPIRE_COUNT'):
						return True
					if eUnit == gc.getInfoTypeForString('UNIT_BRUJAH_ARISTRAKH'):
						return True
					if eUnit == gc.getInfoTypeForString('UNIT_VAMPIRE_LORD_ARISTRAKH'):
						return True
					if eUnit == gc.getInfoTypeForString('UNIT_BLOOD_SEEKER'):
						return True
				if iSoulVault < 50:
					if eUnit == gc.getInfoTypeForString('UNIT_NIKEPHOROS'):
						return True
					if eUnit == gc.getInfoTypeForString('UNIT_NOFUOS'):
						return True
#Aristrakh

# Scions
		if eUnit == gc.getInfoTypeForString('UNIT_BONE_HORDE'):
			iCountBoneHorde = 0
			iCountNecromancer = 0
			for pUnit in PyPlayer(ePlayer).getUnitList():
				pUnitType = pUnit.getUnitType()
				if pUnitType == gc.getInfoTypeForString('UNIT_BONE_HORDE'):
					iCountBoneHorde += 1
				elif pUnitType == gc.getInfoTypeForString('UNIT_NECROMANCER'):
					iCountNecromancer += 1
			if iCountBoneHorde >= iCountNecromancer*3:
				return True

		if pPlayer.getLeaderType() != gc.getInfoTypeForString('LEADER_RISEN_EMPEROR'):
			if eUnit == gc.getInfoTypeForString('UNIT_LEGATE'):
				return True
			if eUnit == gc.getInfoTypeForString('UNIT_DOOMSAYER'):
				return True
			if eUnit == gc.getInfoTypeForString('UNIT_DOOMGIVER'):
				return True
				
		if pPlayer.getLeaderType() != gc.getInfoTypeForString('LEADER_XIVAN'):
			if eUnit == gc.getInfoTypeForString('UNIT_XIVAN'):
				return True

# Scions

#Condantis
		if not pPlayer.getBuildingClassCount(CvUtil.findInfoTypeNum(gc.getBuildingClassInfo, gc.getNumBuildingClassInfos(), 'BUILDINGCLASS_SHRINE_OF_DANALIN')) > 0:
			if eUnit == gc.getInfoTypeForString('UNIT_CHILD_OF_CONDANTIS'):
				return True

		return False

	def canConstruct(self,argsList):
		pCity = argsList[0]
		eBuilding = argsList[1]
		bContinue = argsList[2]
		bTestVisible = argsList[3]
		bIgnoreCost = argsList[4]
		return False

	def cannotConstruct(self,argsList):
		pCity = argsList[0]
		eBuilding = argsList[1]
		bContinue = argsList[2]
		bTestVisible = argsList[3]
		bIgnoreCost = argsList[4]
		pPlayer = gc.getPlayer(pCity.getOwner())
		iBuildingClass = gc.getBuildingInfo(eBuilding).getBuildingClassType()
		eTeam = gc.getTeam(pPlayer.getTeam())

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_AGNOSTIC')):
			if eBuilding == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_LEAVES'):
				return True
			if eBuilding == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_KILMORPH'):
				return True
			if eBuilding == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_EMPYREAN'):
				return True
			if eBuilding == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_OVERLORDS'):
				return True
			if eBuilding == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_VEIL'):
				return True
			if eBuilding == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_ORDER'):
				return True

		if eBuilding == gc.getInfoTypeForString('BUILDING_SHRINE_OF_THE_CHAMPION'):
			iHero = cf.getHero(pPlayer)
			if iHero == -1:
				return True
			if CyGame().isUnitClassMaxedOut(iHero, 0) == False:
				return True
			if pPlayer.getUnitClassCount(iHero) > 0:
				return True

		if eBuilding == gc.getInfoTypeForString('BUILDING_MERCURIAN_GATE'):
			if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
				return True
			if pCity.isCapital():
				return True
			if pPlayer.isHuman() == False:
				if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
					return True

		iAltar1 = gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR')
		iAltar2 = gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_ANOINTED')
		iAltar3 = gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_BLESSED')
		iAltar4 = gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_CONSECRATED')
		iAltar5 = gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_DIVINE')
		iAltar6 = gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_EXALTED')
		iAltar7 = gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_FINAL')
		if (eBuilding == iAltar1 or eBuilding == iAltar2 or eBuilding == iAltar3 or eBuilding == iAltar4 or eBuilding == iAltar5 or eBuilding == iAltar6 or eBuilding == iAltar7):
			if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
				return True
			if eBuilding == iAltar1:
				if (pPlayer.countNumBuildings(iAltar2) > 0 or pPlayer.countNumBuildings(iAltar3) > 0 or pPlayer.countNumBuildings(iAltar4) > 0 or pPlayer.countNumBuildings(iAltar5) > 0 or pPlayer.countNumBuildings(iAltar6) > 0 or pPlayer.countNumBuildings(iAltar7) > 0):
					return True
			if eBuilding == iAltar2:
				if (pPlayer.countNumBuildings(iAltar3) > 0 or pPlayer.countNumBuildings(iAltar4) > 0 or pPlayer.countNumBuildings(iAltar5) > 0 or pPlayer.countNumBuildings(iAltar6) > 0 or pPlayer.countNumBuildings(iAltar7) > 0):
					return True
			if eBuilding == iAltar3:
				if (pPlayer.countNumBuildings(iAltar4) > 0 or pPlayer.countNumBuildings(iAltar5) > 0 or pPlayer.countNumBuildings(iAltar6) > 0 or pPlayer.countNumBuildings(iAltar7) > 0):
					return True
			if eBuilding == iAltar4:
				if (pPlayer.countNumBuildings(iAltar5) > 0 or pPlayer.countNumBuildings(iAltar6) > 0 or pPlayer.countNumBuildings(iAltar7) > 0):
					return True
			if eBuilding == iAltar5:
				if (pPlayer.countNumBuildings(iAltar6) > 0 or pPlayer.countNumBuildings(iAltar7) > 0):
					return True
			if eBuilding == iAltar6:
				if pPlayer.countNumBuildings(iAltar7) > 0:
					return True

		if eBuilding == gc.getInfoTypeForString('BUILDING_PROPHECY_OF_RAGNAROK'):
			if pPlayer.getAlignment() != gc.getInfoTypeForString('ALIGNMENT_EVIL'):
				return True

		if pPlayer.isHuman() == False:
			iPalace = gc.getInfoTypeForString('BUILDINGCLASS_PALACE')
			if iBuildingClass == iPalace:
				if pPlayer.getBuildingClassCount(iPalace)>0:
					return True

		if eBuilding == gc.getInfoTypeForString('BUILDING_SMUGGLERS_PORT'):
			if pPlayer.isSmugglingRing() == False:
				return True

		if eBuilding == gc.getInfoTypeForString('BUILDING_SUMMON_CIRCLE'):
			if pCity.isCapital():
				return True
				
#Xtended
		if eBuilding == gc.getInfoTypeForString('BUILDING_VACANT_MAUSOLEUM'):
			if pPlayer.getLeaderType() != gc.getInfoTypeForString('LEADER_KORINNA'):
				return True

		if eBuilding == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_GIFT'):
			if pPlayer.getLeaderType() != gc.getInfoTypeForString('LEADER_RISEN_EMPEROR'):
				return True

		if eBuilding == gc.getInfoTypeForString('BUILDING_FOOD_CARAVAN'):
			if pPlayer.isHuman() == False:
				if pCity.isCapital():
					if pCity.getPopulation() < 8:
						return True
				else:
					if pCity.getPopulation() < 6:
						return True
				
#Aristrakh
		iDefCat = gc.getInfoTypeForString('BUILDING_DEFILING_CATALYST')
		iVoidCat = gc.getInfoTypeForString('BUILDING_VOID_CATALYST')
		iVampPal = gc.getInfoTypeForString('BUILDING_VAMPIRE_PALACE')
		iNecAri = gc.getInfoTypeForString('BUILDING_NECROPOLIS_ARISTRAKH')
		iAriPal = gc.getInfoTypeForString('BUILDING_PALACE_ARISTRAKH')
		iCatac = gc.getInfoTypeForString('BUILDING_CATACOMBS')
		iCabal = gc.getInfoTypeForString('BUILDING_CABAL_SHRINE')
		iCircDes = gc.getInfoTypeForString('BUILDING_CIRCLE_OF_DESPAIR')
		iCircBlo = gc.getInfoTypeForString('BUILDING_CIRCLE_OF_BLOOD')
		iGravePact = gc.getInfoTypeForString('BUILDING_GRAVE_PACT')

		#can only have either one: Circle of Blood or Circle of Despair
		if eBuilding == iCircDes or eBuilding == iCircBlo:
			if pPlayer.countNumBuildings(iCircDes) > 0 or pPlayer.countNumBuildings(iCircBlo) > 0:
				return True

		#Catalysts cannot be build in the same city as Palace, Circle of Blood or Despair
		if eBuilding == iDefCat or eBuilding == iVoidCat:
			if pCity.getNumRealBuilding(iCircDes) > 0 or pCity.getNumRealBuilding(iCircBlo) > 0 or pCity.getNumRealBuilding(iAriPal) > 0:
				return True

		#Palace/Capital can not be moved away from City with Circle of Despair/Blood or to a city with a Catalyst
		if eBuilding == iAriPal:
			if pCity.getNumRealBuilding(iDefCat) > 0 or pCity.getNumRealBuilding(iVoidCat) > 0 or pPlayer.countNumBuildings(iCircDes) > 0 or pPlayer.countNumBuildings(iCircBlo) > 0:
				return True

		#Xtended Catacomb can be built in any city
		#if eBuilding == iCatac or eBuilding == iNecAri or eBuilding == iVampPal:
		
		#Vampire Palace or Necropolis requires either circle of despair and circle of blood
		if eBuilding == iCabal or eBuilding == iNecAri or eBuilding == iVampPal:
			if pCity.getNumRealBuilding(iCircBlo) < 1 and pCity.getNumRealBuilding(iCircDes) < 1:
				return True

		#Grave Pact requires Necropolis or Vampire Palace
		if eBuilding == iGravePact:
			if pCity.getNumRealBuilding(iNecAri) < 1 and pCity.getNumRealBuilding(iVampPal) < 1:
				return True


#Aristrakh

		return False

	def canCreate(self,argsList):
		pCity = argsList[0]
		eProject = argsList[1]
		bContinue = argsList[2]
		bTestVisible = argsList[3]
		return False

	def cannotCreate(self,argsList):
		pCity = argsList[0]
		eProject = argsList[1]
		bContinue = argsList[2]
		bTestVisible = argsList[3]
		pPlayer = gc.getPlayer(pCity.getOwner())
		eTeam = gc.getTeam(pPlayer.getTeam())

		if gc.getProjectInfo(eProject).getLastTurnPossibleBuild() > 0:
			return False #Handled inside DLL

		if eProject == gc.getInfoTypeForString('PROJECT_BIRTHRIGHT_REGAINED'):
			if not pPlayer.isFeatAccomplished(FeatTypes.FEAT_GLOBAL_SPELL):
				return True

		if eProject == gc.getInfoTypeForString('PROJECT_THE_WHITE_HAND'):
			pPlayer = gc.getPlayer(iPlayer)
			pCity = pPlayer.getCapitalCity()
			iPriest = gc.getInfoTypeForString('UNIT_PRIEST_OF_WINTER')
			iHighPriest = gc.getInfoTypeForString('UNIT_HIGH_PRIEST_OF_WINTER')
			iWhiteHand = 0
			py = PyPlayer(iPlayer)
			for pUnit in py.getUnitList():
				if (pUnit.getUnitType()==iPriest or pUnit.getUnitType()==iHighPriest):
					iWhiteHand += 1
#					if pUnit.getNameKey()=="Dumannios":
#						iWhiteHand += 1
#					if pUnit.getNameKey()=="Riuros":
#						iWhiteHand += 1
#					if pUnit.getNameKey()=="Anagantios":
#						iWhiteHand += 1
			if iWhiteHand >= 3:
				return True
				
		if eProject == gc.getInfoTypeForString('PROJECT_THE_DEEPENING'):
			pPlayer = gc.getPlayer(iPlayer)
			pCity = pPlayer.getCapitalCity()
			iPriest = gc.getInfoTypeForString('UNIT_PRIEST_OF_WINTER')
			iHighPriest = gc.getInfoTypeForString('UNIT_HIGH_PRIEST_OF_WINTER')
			iDeepening = 0
			py = PyPlayer(iPlayer)
			for pUnit in py.getUnitList():
				if (pUnit.getUnitType()==iPriest or pUnit.getUnitType()==iHighPriest):
					if pUnit.getNameKey()=="Dumannios":
						iDeepening += 1
					if pUnit.getNameKey()=="Riuros":
						iDeepening += 1
					if pUnit.getNameKey()=="Anagantios":
						iDeepening += 1
			if iDeepening < 3:
				return True
				
				
		if eProject == gc.getInfoTypeForString('PROJECT_STIR_FROM_SLUMBER'):
			if pPlayer.getPlayersKilled() == 0:
				return True

		if eProject == gc.getInfoTypeForString('PROJECT_ASCENSION'):
			if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_ILLIANS') and pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_FROZEN'):
				return True

		if eProject == gc.getInfoTypeForString('PROJECT_SAMHAIN'):
			if pPlayer.isHuman() == False:
				if pPlayer.getNumCities() <= 3:
					return True

		if eProject == gc.getInfoTypeForString('PROJECT_LIBERATION'):
			if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_ILLIANS') and pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_FROZEN'):
				return True
#SpyFanatic: allow Illians to create Frozen
#			if pPlayer.isHuman() == False:
#				return True

		#xtended - moved to CvGlobalEnchantmentInterface.py
#		if eProject == gc.getInfoTypeForString('PROJECT_SUMMON_CTHULTHU'):
#			iCountOverlords = 0
#			for pUnit in PyPlayer(ePlayer).getUnitList():
#				pUnitType = pUnit.getUnitType()
#				if pUnitType == gc.getInfoTypeForString('UNIT_CTHULTHU'):
#					iCountOverlords += 1
#			if iCountOverlords > 0:
#				return True
		
		if pPlayer.isHuman() == False:
			if pPlayer.getUnitClassCountPlusMaking(gc.getInfoTypeForString('UNITCLASS_WARRIOR')) <= 2:
				if not eTeam.isHasTech(gc.getInfoTypeForString('TECH_BRONZE_WORKING')):
					if not eTeam.isHasTech(gc.getInfoTypeForString('TECH_ARCHERY')):
						return True

		if pPlayer.isHuman() == False:
			if pCity.getBaseYieldRate(1) <=10 :
				return True

#Faeries
		if not gc.getInfoTypeForString('CIVILIZATION_FAERIES')==-1:
			if eProject == gc.getInfoTypeForString('PROJECT_SUMMER_RESTORATION'):
				if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) != gc.getInfoTypeForString('CIVIC_SUMMER_COURT'): #and pCity.getPopulation() < 15:
					return True

			if eProject == gc.getInfoTypeForString('PROJECT_WINTER_RESTORATION'):
				if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) != gc.getInfoTypeForString('CIVIC_WINTER_COURT'): #and pPlayer.getPlayersKilled() == 0:
					return True

			if eProject == gc.getInfoTypeForString('PROJECT_RETURN_TO_GLORY'):
				if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_FAE_LORD')) < 4:
					return True

			if eProject == gc.getInfoTypeForString('PROJECT_ETERNAL_RESTORATION'):
				if not pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_FAERIES'):
					return True
				if not CyGame().getProjectCreatedCount(gc.getInfoTypeForString('PROJECT_SUMMER_RESTORATION'))>0:
					return True
				if not CyGame().getProjectCreatedCount(gc.getInfoTypeForString('PROJECT_WINTER_RESTORATION'))>0:
					return True
#Faeries

#Austrin - Nomad's Preparing Expedition
		if eProject == gc.getInfoTypeForString('PROJECT_PREPARE_EXPEDITION'):
			if not pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_NOMADS')):
				return True
			if pPlayer.isFeatAccomplished(FeatTypes.FEAT_EXPEDITION_READY):
				return True

		return False

	def canMaintain(self,argsList):
		pCity = argsList[0]
		eProcess = argsList[1]
		bContinue = argsList[2]
		return False

	def cannotMaintain(self,argsList):
		pCity = argsList[0]
		eProcess = argsList[1]
		bContinue = argsList[2]
		return False
		
	def AI_chooseTech(self,argsList):
		ePlayer = argsList[0]
		bFree = argsList[1]
		pPlayer = gc.getPlayer(ePlayer)
		pCity = pPlayer.getCapitalCity()
		iCiv=pPlayer.getCivilizationType()
		iFavRel=pPlayer.getFavoriteReligion()
		eTeam = gc.getTeam(pPlayer.getTeam())
		iTech = -1

		return TechTypes.NO_TECH

	def AI_chooseProduction(self,argsList):
		pCity = argsList[0]
		ePlayer = pCity.getOwner()
		pPlayer = gc.getPlayer(ePlayer)
		player = PyPlayer(ePlayer)
		civtype = pPlayer.getCivilizationType()
		infoCiv = gc.getCivilizationInfo(civtype)
		eTeam = gc.getTeam(pPlayer.getTeam())
		pPlot = pCity.plot()

#Barbarian Cities have their own Function
		if pCity.isBarbarian():
			if (cf.BarbCityProduction(pCity)==0):
				return False
			else:
				return True

		return False

	def AI_unitUpdate(self,argsList):
		pUnit = argsList[0]
		pPlot = pUnit.plot()

		return False

	def AI_doWar(self,argsList):
		eTeam = argsList[0]
		return False

	def AI_doDiplo(self,argsList):
		ePlayer = argsList[0]
		return False

	def calculateScore(self,argsList):
		ePlayer = argsList[0]
		bFinal = argsList[1]
		bVictory = argsList[2]

		iPopulationScore = CvUtil.getScoreComponent(gc.getPlayer(ePlayer).getPopScore(), gc.getGame().getInitPopulation(), gc.getGame().getMaxPopulation(), gc.getDefineINT("SCORE_POPULATION_FACTOR"), True, bFinal, bVictory)
		iLandScore = CvUtil.getScoreComponent(gc.getPlayer(ePlayer).getLandScore(), gc.getGame().getInitLand(), gc.getGame().getMaxLand(), gc.getDefineINT("SCORE_LAND_FACTOR"), True, bFinal, bVictory)
		iTechScore = CvUtil.getScoreComponent(gc.getPlayer(ePlayer).getTechScore(), gc.getGame().getInitTech(), gc.getGame().getMaxTech(), gc.getDefineINT("SCORE_TECH_FACTOR"), True, bFinal, bVictory)
		iWondersScore = CvUtil.getScoreComponent(gc.getPlayer(ePlayer).getWondersScore(), gc.getGame().getInitWonders(), gc.getGame().getMaxWonders(), gc.getDefineINT("SCORE_WONDER_FACTOR"), False, bFinal, bVictory)
		return int(iPopulationScore + iLandScore + iWondersScore + iTechScore)

	def doHolyCity(self):
		return True

	def doHolyCityTech(self,argsList):
		eTeam = argsList[0]
		ePlayer = argsList[1]
		eTech = argsList[2]
		bFirst = argsList[3]
		return False

	def doGold(self,argsList):
		ePlayer = argsList[0]
		return False

	def doResearch(self,argsList):
		ePlayer = argsList[0]
		return False

	def doGoody(self,argsList):
		ePlayer = argsList[0]
		pPlot = argsList[1]
		pUnit = argsList[2]
		return False

	def doGrowth(self,argsList):
		pCity = argsList[0]
		return False

	def doProduction(self,argsList):
		pCity = argsList[0]
		return False

	def doCulture(self,argsList):
		pCity = argsList[0]
		return False

	def doPlotCulture(self,argsList):
		pCity = argsList[0]
		bUpdate = argsList[1]
		ePlayer = argsList[2]
		iCultureRate = argsList[3]
		return False

	def doReligion(self,argsList):
		pCity = argsList[0]
		return False

	def cannotSpreadReligion(self,argsList):
		iOwner, iUnitID, iReligion, iX, iY = argsList[0]
		return False

	def doGreatPeople(self,argsList):
		pCity = argsList[0]
		return False

	def doMeltdown(self,argsList):
		pCity = argsList[0]
		return False

	def doReviveActivePlayer(self,argsList):
		"allows you to perform an action after an AIAutoPlay"
		iPlayer = argsList[0]
		return False

	def doPillageGold(self, argsList):
		"controls the gold result of pillaging"
		pPlot = argsList[0]
		pUnit = argsList[1]

		iPillageGold = 0
		iPillageGold = CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 1")
		iPillageGold += CyGame().getSorenRandNum(gc.getImprovementInfo(pPlot.getImprovementType()).getPillageGold(), "Pillage Gold 2")

		iPillageGold += (pUnit.getPillageChange() * iPillageGold) / 100

		return iPillageGold

	def doCityCaptureGold(self, argsList):
		"controls the gold result of capturing a city"

		pOldCity = argsList[0]

		iCaptureGold = 0

		iCaptureGold += gc.getDefineINT("BASE_CAPTURE_GOLD")
		iCaptureGold += (pOldCity.getPopulation() * gc.getDefineINT("CAPTURE_GOLD_PER_POPULATION"))
		iCaptureGold += CyGame().getSorenRandNum(gc.getDefineINT("CAPTURE_GOLD_RAND1"), "Capture Gold 1")
		iCaptureGold += CyGame().getSorenRandNum(gc.getDefineINT("CAPTURE_GOLD_RAND2"), "Capture Gold 2")

		if (gc.getDefineINT("CAPTURE_GOLD_MAX_TURNS") > 0):
			iCaptureGold *= cyIntRange((CyGame().getGameTurn() - pOldCity.getGameTurnAcquired()), 0, gc.getDefineINT("CAPTURE_GOLD_MAX_TURNS"))
			iCaptureGold /= gc.getDefineINT("CAPTURE_GOLD_MAX_TURNS")

		if pOldCity.isBarbarian():
			iCaptureGold += 1000

		return iCaptureGold

	def citiesDestroyFeatures(self,argsList):
		iX, iY= argsList
		return True

	def canFoundCitiesOnWater(self,argsList):
		iX, iY= argsList
		return False

	def doCombat(self,argsList):
		pSelectionGroup, pDestPlot = argsList
		return False

	def getConscriptUnitType(self, argsList):
		iPlayer = argsList[0]
		iConscriptUnitType = -1 #return this with the value of the UNIT TYPE you want to be conscripted, -1 uses default system

		return iConscriptUnitType

	def getCityFoundValue(self, argsList):
		iPlayer, iPlotX, iPlotY = argsList
		iFoundValue = -1 # Any value besides -1 will be used

		return iFoundValue

	def canPickPlot(self, argsList):
		pPlot = argsList[0]
		return true

	def getUnitCostMod(self, argsList):
		iPlayer, iUnit = argsList
		iCostMod = -1 # Any value > 0 will be used

		return iCostMod

	def getBuildingCostMod(self, argsList):
		iPlayer, iCityID, iBuilding = argsList
		pPlayer = gc.getPlayer(iPlayer)
		pCity = pPlayer.getCity(iCityID)

		iCostMod = -1 # Any value > 0 will be used

		if iBuilding == gc.getInfoTypeForString('BUILDING_GAMBLING_HOUSE'):
			if pPlayer.isGamblingRing():
				iCostMod = gc.getBuildingInfo(iBuilding).getProductionCost() / 4

		return iCostMod

	def canUpgradeAnywhere(self, argsList):
		pUnit = argsList

		bCanUpgradeAnywhere = 0

		return bCanUpgradeAnywhere

	def getWidgetHelp(self, argsList):
		eWidgetType, iData1, iData2, bOption = argsList

		return u""

	def getUpgradePriceOverride(self, argsList):
		iPlayer, iUnitID, iUnitTypeUpgrade = argsList

		return -1	# Any value 0 or above will be used

	def getExperienceNeeded(self, argsList):
		# use this function to set how much experience a unit needs
		iLevel, iOwner = argsList

		iExperienceNeeded = 0

		# regular epic game experience
		iExperienceNeeded = iLevel * iLevel + 1

		#modified Exp gain Sephi
		if iLevel>3:
			iExperienceNeeded +=(iLevel-3) * (iLevel-3)
		#modified Exp gain

		iModifier = gc.getPlayer(iOwner).getLevelExperienceModifier()
		if (0 != iModifier):
			iExperienceNeeded += (iExperienceNeeded * iModifier + 99) / 100   # ROUND UP

		return iExperienceNeeded

#returns the current flag for Tower Victory
	def AI_TowerMastery(self, argsList):
		ePlayer = argsList[0]
		flag = argsList[1]

		pPlayer = gc.getPlayer(ePlayer)
		eTeam = gc.getTeam(pPlayer.getTeam())

		return 0
