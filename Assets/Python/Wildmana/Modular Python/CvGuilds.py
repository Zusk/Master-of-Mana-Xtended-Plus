import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

def reqSmelting(pPlayer):

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_GUILDHALL_ARTISAN')) == 0:
		return false
		
	return true

def reqShaftMine(pPlayer):

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_FORGE')) == 0:
		return false
	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_MINING_GUILD')) == 0:
		return false
		
	return true

def reqMint(pPlayer):

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_SHAFT_MINE')) < 3:
		return false
		
	return true

def reqMithrilWorking(pPlayer):

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_JEWELER')) < 3:
		return false
		
	return true
	
def reqHuntingLodge(pPlayer):
	iCountBuilding = 0
	
	iCountBuilding += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_WOLF_PEN'))
	iCountBuilding += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_DANCING_BEAR'))
	iCountBuilding += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_GORILLA_CAGE'))
	iCountBuilding += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_LION_CAGE'))
	iCountBuilding += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_STAG_CAMP'))
	iCountBuilding += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_SPIDER_PEN'))	
	iCountBuilding += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_TIGER_CAGE'))		
	
	return iCountBuilding > 5
	
def reqTracking(pPlayer):
	apUnitList = PyPlayer(pPlayer.getID()).getUnitList()
	eRecon = gc.getInfoTypeForString('UNITCOMBAT_RECON')
	for pUnit in apUnitList:
		if pUnit.getLevel() > 7:
			if pUnit.getUnitCombatType() == eRecon:
				return true
				
	return false
	
def reqCalendar(pPlayer):

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_BANK')) < 2:
		return false
		
	return true

def reqTradersHall(pPlayer):	

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_RESOURCE_DISTRICT')) < 1:
		return false
	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_FOREIGN_TRADE_DISTRICT')) < 1:
		return false
	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_LUXURY_DISTRICT')) < 1:
		return false

	return true
	
def reqWeaving(pPlayer):	

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_SILK_TRADER')) < 1:
		return false
	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_WEAVERS_HUT')) < 3:
		return false

	return true
	
def reqCaravansary(pPlayer):

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_GUILD_HALLS')) < 6:
		return false

	return true
	
def reqHerbalist(pPlayer):	

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_HERBALIST')) < 6:
		return false

	return true

def reqBenevolence(pPlayer):

	if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_CULTURAL_VALUES')) == gc.getInfoTypeForString('CIVIC_BENEVOLENCE'):
		return true
		
	return false