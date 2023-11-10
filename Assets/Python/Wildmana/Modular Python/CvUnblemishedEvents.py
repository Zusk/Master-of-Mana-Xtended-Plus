import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()


#def ChangeWarAccordEvent(argsList):
#	'War Status Changes'
#	bIsWar = argsList[0]
#	iTeam = argsList[1]
#	eTeam = gc.getTeam(iTeam)
##	pPlayer = gc.getPlayer(iPlayer)
#		
#	if eTeam.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
#		iAccord = eTeam.getPurityCounter()
#		iAccord += 10
#		eTeam.setPurityCounter(iAccord)

def GoldenAgeAccordEvent(argsList):
	'Golden Age'
	iPlayer = argsList[0]
	player = PyPlayer(iPlayer)
	pPlayer = gc.getPlayer(iPlayer)
		
	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
		iAccord = pPlayer.getPurityCounter()
		iAccord += 25
		pPlayer.setPurityCounter(iAccord)
		CyInterface().addImmediateMessage( "Golden Age +25 Vim","" )
	
		
def TechAcquiredAccordEvent(argsList):
	'Tech Acquired'
	iTechType, iTeam, iPlayer, bAnnounce = argsList
	# Note that iPlayer may be NULL (-1) and not a refer to a player object
	pPlayer = gc.getPlayer(iPlayer)
	pTeam = gc.getTeam(pPlayer.getTeam())	

	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
		iAccord = pPlayer.getPurityCounter()
		iAccord += 2
		pPlayer.setPurityCounter(iAccord)
		CyInterface().addImmediateMessage( "Tech Acquired +2 Vim","" )

def UnitPromotedAccordEvent(argsList):
	'Unit Promoted'
	pUnit, iPromotion = argsList
	player = PyPlayer(pUnit.getOwner())
#	pPlayer = gc.getPlayer(player)
#	player = PyPlayer(unit.getOwner())
	pPlayer = gc.getPlayer(pUnit.getOwner())
	
#	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
#		iDedication=gc.getInfoTypeForString('PROMOTION_DEDICATED_TO_SUCELLUS')
#		if iPromotion==iDedication:
#			iAccord = pPlayer.getPurityCounter()
#			iAccord += 5
#			pPlayer.setPurityCounter(iAccord)
#			CyInterface().addImmediateMessage( "Dedicated to Sucellus +5 Vim","" )
		
def BuildingBuiltAccordEvent(argsList):
	'Building Completed'
	pCity, iBuildingType = argsList
	player = pCity.getOwner()
	pPlayer = gc.getPlayer(player)
	pPlot = pCity.plot()
	game = gc.getGame()
	iBuildingClass = gc.getBuildingInfo(iBuildingType).getBuildingClassType()
		
	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
		if isWorldWonderClass(iBuildingClass):
			iAccord = pPlayer.getPurityCounter()
			iAccord += 15
			pPlayer.setPurityCounter(iAccord)
			CyInterface().addImmediateMessage( "Wonder Built +15 Vim","" )
		
		
def UnitKilledAccordEvent(argsList):
	'Unit Killed'
	unit, iAttacker = argsList
	iPlayer = unit.getOwner()
	player = PyPlayer(iPlayer)
	attacker = PyPlayer(iAttacker)
	pPlayer = gc.getPlayer(iPlayer)
	
	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
		iMartyr = gc.getInfoTypeForString('PROMOTION_MARTYR')
		iMark = gc.getInfoTypeForString('PROMOTION_MARK_OF_REDEMPTION')
		iDedicated = gc.getInfoTypeForString('PROMOTION_DEDICATED_TO_SUCELLUS')
		
		if unit.isHasPromotion(iDedicated):
			iAccord = pPlayer.getPurityCounter()
			iAccord += 3
			pPlayer.setPurityCounter(iAccord)
			CyInterface().addImmediateMessage( "Unit Dedicated to Sucellus killed:  +3 Vim.","" )
		
		if unit.isHasPromotion(iMark):
			iAccord = pPlayer.getPurityCounter()
			iAccord -= 16
			pPlayer.setPurityCounter(iAccord)
			pTomb = pPlayer.getCapitalCity()
			iUnitType = unit.getUnitType()
			newUnit = pPlayer.initUnit(iUnitType, pTomb.getX(), pTomb.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			for i in range(gc.getNumPromotionInfos()):
				if gc.getPromotionInfo(i).isEquipment() == False:
					newUnit.setHasPromotion(i, unit.isHasPromotion(i))
			newUnit.setDamage(95, -1)
			newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MARK_OF_REDEMPTION'), False)
			newUnit.setLevel(unit.getLevel())
			newUnit.setExperience(unit.getExperience(), -1)
			CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_UNIT_REDEEMED", ()),'',1,', ,Art/Interface/Buttons/Misc_Atlas1.dds,6,8',ColorTypes(7),pTomb.getX(),pTomb.getY(),True,True)
			if CyGame().getGlobalCounter() > 1:
				CyGame().changeGlobalCounter(-1)
		
		if unit.isHasPromotion(iMartyr):
			iLevel = unit.getLevel()
			iAccord = pPlayer.getPurityCounter()
			iAccord += (iLevel * 5)
			pPlayer.setPurityCounter(iAccord)
			CyInterface().addImmediateMessage( "Unit Martyred:  Vim increased.","" )
		else:
			iAccord = pPlayer.getPurityCounter()
			iAccord += 1
			pPlayer.setPurityCounter(iAccord)
	
	
def GPAccordEvent(argsList):
	'Unit Completed'
	unit = argsList[0]
	player = PyPlayer(unit.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())
	
	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_PROPHET'):
			iAccord = pPlayer.getPurityCounter()
			iAccord = int(iAccord * 0.6)
			pPlayer.setPurityCounter(iAccord)
			pPlayer.changeFaith(iAccord * 12)
			CyInterface().addImmediateMessage( "Great Prophet:  Vim to Faith","" )
		
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_ARTIST'):
			iAccord = pPlayer.getPurityCounter()
#			CyInterface().addMessage(0,true,25,"This is Player is (%i)" %(iAccord),'',0,'',ColorTypes(11), 0, 0, True,True)	
			iAccord = int(iAccord * 1.5) + 10
			pPlayer.setPurityCounter(iAccord)
			CyInterface().addImmediateMessage( "Great Bard:  Vim Bonus","" )
			
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_SCIENTIST'):
			iAccord = pPlayer.getPurityCounter()
			iAccord = int(iAccord * 0.6)
			pPlayer.setPurityCounter(iAccord)
			eTeam = gc.getTeam(pPlayer.getTeam())
			iTech = pPlayer.getCurrentResearch()
			iNum = (iAccord * 10)
			eTeam.changeResearchProgress(iTech, iNum, unit.getOwner())
			CyInterface().addImmediateMessage( "Great Sage:  Vim to Research","" )
			
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_MERCHANT'):
			iAccord = pPlayer.getPurityCounter()
			iAccordChange = int(iAccord * 0.4)
			iAccord = iAccord - iAccordChange
			pPlayer.setPurityCounter(iAccord)
			iGold = (iAccordChange * 10)
			pPlayer.changeGold(iGold)
			CyInterface().addImmediateMessage( "Great Merchant:  Vim to Gold","" )
			
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_ENGINEER'):
			iAccord = pPlayer.getPurityCounter()
			iAccord = int(iAccord * 0.6)
			pPlayer.setPurityCounter(iAccord)
			iResources = (iAccord * 5)
			pPlayer.changeGlobalYield(YieldTypes.YIELD_METAL,iResources)
			pPlayer.changeGlobalYield(YieldTypes.YIELD_LUMBER,iResources)
			pPlayer.changeGlobalYield(YieldTypes.YIELD_HERB,iResources)
			pPlayer.changeGlobalYield(YieldTypes.YIELD_LEATHER,iResources)
			pPlayer.changeGlobalYield(YieldTypes.YIELD_STONE,iResources)
			CyInterface().addImmediateMessage( "Great Engineer:  Vim to Global Resources","" )
			
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_SORCERER'):
			iAccord = pPlayer.getPurityCounter()
			iAccord = int(iAccord * 0.6)
			pPlayer.setPurityCounter(iAccord)
			pPlayer.changeMana(iAccord * 5)
			
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_COMMANDER'):
			py = PyPlayer(unit.getOwner())
			iAccord = pPlayer.getPurityCounter()
			iAccordChange = int(iAccord * 0.4)
			if iAccordChange > 60:
				iAccordChange = 60
			pPlayer.setPurityCounter(iAccord - iAccordChange)
			iAccordMod = int(iAccordChange * 0.1)
			CyInterface().addImmediateMessage( "Great Commander:  Vim to Paragon xp","" )
#			if iAccordMod > 5:
#				iAccordMod = 5:
			iParagon = gc.getInfoTypeForString('UNIT_PARAGON')
			iScearn = gc.getInfoTypeForString('UNIT_SCEARN_COLDINE')
			for pUnit in py.getUnitList():
				if (pUnit.getUnitType() == iParagon or pUnit.getUnitType() == iScearn):
					pUnit.changeExperience(iAccordMod, -1, false, false, false)
					
def CityLostAccordEvent(argsList):
	'City Lost'
	city = argsList[0]
	player = PyPlayer(city.getOwner())
	pPlayer = gc.getPlayer(city.getOwner())
#	iActivePlayer = CyGame().getActivePlayer()
	
	if city.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
#		if pPlayer != iActivePlayer:
		iAccord = pPlayer.getPurityCounter()
		iAccord += 10 
		pPlayer.setPurityCounter(iAccord)
		CyInterface().addImmediateMessage( "City Lost +10 Vim","" )
	
def CityGrowthAccordEvent(argsList):
	'City Population Growth'
	pCity = argsList[0]
	iPlayer = argsList[1]
	pPlayer = gc.getPlayer(iPlayer)
	
	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
		iAccord = pPlayer.getPurityCounter()
		iAccord += 3
		pPlayer.setPurityCounter(iAccord)
		
def CityAcquiredAndKeptAccordEvent(argsList):
		'City Acquired and Kept'
		iOwner,pCity = argsList
		
		pPlayer = gc.getPlayer(iOwner)
		
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
			iAccord = pPlayer.getPurityCounter()
			iAccord += 30
			pPlayer.setPurityCounter(iAccord)
			CyInterface().addImmediateMessage( "City Taken +10 Vim","" )

def CityAcquiredAccordEvent(argsList):
		'City Acquired'
		iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
		pPlayer = gc.getPlayer(iNewOwner)
		pPrevious = gc.getPlayer(iPreviousOwner)
		
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
			iAccord = pPlayer.getPurityCounter()
			iAccord -= 20
			pPlayer.setPurityCounter(iAccord)

class UnblemishedFunctions:

#UTEST
	def Vimpopup(self):
		CyInterface().addImmediateMessage( "Civic works +10 Vim","" )
		pPlayer = gc.getActivePlayer()
		if pPlayer.getGold() > 199:
			iRoll1 = CyGame().getSorenRandNum(100, "roll 1")
			iRoll2 = CyGame().getSorenRandNum(100, "roll 2")
			iGold = iRoll1 + iRoll2 + 200
			iAccord = pPlayer.getPurityCounter()
			pPlayer.changeGold(-iGold)
			iAccord += 10
			pPlayer.setPurityCounter(iAccord)
	
	def getRevoltBase(self):
		pPlayer = gc.getActivePlayer()
		iRevoltBase = 0
		iAccord = pPlayer.getPurityCounter()
		iRevoltChance = ((100 - iAccord) * 0.2)
		iArmRevoltMod = (CyGame().getGlobalCounter() * .1)
#		if iArmRevoltMod < 0:
#			iArmRevoltMod = 0
		iRevoltChance += iArmRevoltMod
		iwwmod = pPlayer.getWarWearinessPercentAnger()
		iRevoltChance += iwwmod / 20
		iRevoltBase = iRevoltChance
		
		return iRevoltBase
	
	def getRevoltBaseCity(self):
		pPlayer = gc.getActivePlayer()
		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		pCity = CyInterface().getHeadSelectedCity()
		iAccord = pPlayer.getPurityCounter()
		iRevoltChance = 0
		iRevoltChance = ((100 - iAccord) * 0.2)
		iUH = 0
		iUH = pCity.badHealth(False) - pCity.goodHealth()
		if iUH > 2:
			iRevoltChance += 1
		if iUH > 4:
			iRevoltChance += 1
		if iUH > 6:
			iRevoltChance += 1
		if iUH > 8:
			iRevoltChance += 2
		iUnhappy = (pCity.happyLevel() - pCity.unhappyLevel(0))
		if iUnhappy <0:
			iRevoltChance -= iUnhappy
		iArmRevoltMod = (CyGame().getGlobalCounter() * .1)
#		if iArmRevoltMod < 0:
#			iArmRevoltMod = 0
		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PILLAR_OF_COURAGE')) > 0:
			iArmRevoltMod *= 0.5
		iRevoltChance += iArmRevoltMod
		iwwmod = pPlayer.getWarWearinessPercentAnger()
		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_BUILDING_DUNGEON')) > 0:
			iwwmod *= 0.5
		iRevoltChance += iwwmod / 20
		if CyGame().getGameTurn() != CyGame().getStartTurn():
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNBLEMISHED_MOOT')) > 0:
				iRevoltChance *= 0.8
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_HOPE')) > 0:
				iRevoltChance *= 0.8
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALACE_UNBLEMISHED')) > 0:
				iRevoltChance -= 10
#				message = "%0.2f" %(iRevoltChance)
#				CyInterface().addImmediateMessage(message,"")
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_WINTER_PALACE')) > 0:
				iRevoltChance *= 1.25
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_FORBIDDEN_PALACE')) > 0:
				iRevoltChance *= 1.25
		iRevoltBaseCity = iRevoltChance
		
		return iRevoltBaseCity