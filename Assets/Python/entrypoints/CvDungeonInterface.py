from CvPythonExtensions import *
import CvUtil
import Popup as PyPopup
import CvScreensInterface
import sys
import PyHelpers

PyInfo = PyHelpers.PyInfo
PyPlayer = PyHelpers.PyPlayer
gc = CyGlobalContext()

def dungeon(argsList):
	pAdventurer, eLairResult, iValue = argsList
	LairResult = gc.getLairResultInfo(eLairResult)
	eval(LairResult.getPyResult())

def reqdungeon(argsList):
	pAdventurer, eLairResult, iValue = argsList
	LairResult = gc.getLairResultInfo(eLairResult)
	return eval(LairResult.getPyRequirement())
	
def getNearestCity(pAdventurer):	

	iX = pAdventurer.getX()
	iY = pAdventurer.getY()

	for iPass in range(6):
		for iiX in range(iX-iPass, iX+iPass+1, 1):
			for iiY in range(iY-iPass, iY+iPass+1, 1):
				pLoopPlot = CyMap().plot(iiX,iiY)
				if not pLoopPlot.isNone():
					if pLoopPlot.isCity() and pLoopPlot.getOwner()==pAdventurer.getOwner():
						return pLoopPlot.getPlotCity()
	
	return -1
	
def reqNearCity(pAdventurer, iValue):

	if getNearestCity(pAdventurer)==-1:
		return false
		
	return true
	
def LairResultProduction(pAdventurer, iValue):			

	pCity = getNearestCity(pAdventurer)
	pCity.changeProduction(iValue)

def LairResultCulture(pAdventurer, iValue):			

	pCity = getNearestCity(pAdventurer)
	pCity.changeCulture(pAdventurer.getOwner(),iValue,true)
	
def reqLairResultResearch(pAdventurer, iValue):			

	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	return pPlayer.getCurrentResearch()!=-1
	
def LairResultResearch(pAdventurer, iValue):			

	pTeam=gc.getTeam(pAdventurer.getTeam())
	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	pTeam.changeResearchProgress(pPlayer.getCurrentResearch(),iValue,pAdventurer.getOwner())

def LairResultSpellResearch(pAdventurer, iValue):			

	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	if gc.getGame().getSorenRandNum(100,'LairResult SpellResearch')>20:	
		pPlayer.changeArcane(iValue)
	else:
		listSpellResearch = []	
		pTeam=gc.getTeam(pAdventurer.getTeam())
		for i in range(gc.getNumTechInfos()):
			if gc.getTechInfo(i).isSpellResearch():
				if not pTeam.isHasTech(i):
					if not pPlayer.isCanEverSpellResearch(i):
						listSpellResearch.append(i)
						
		if(len(listSpellResearch)>0):
			iPick=gc.getGame().getSorenRandNum(len(listSpellResearch),'LairResult SpellResearch')
			pTeam.setHasTech(listSpellResearch[iPick],true,-1,false,false)
	
def LairResultGreatPerson(pAdventurer, iValue):			

	pCity = getNearestCity(pAdventurer)
	
	iPick = gc.getGame().getSorenRandNum(gc.getNumSpecialistInfos(),'LairResult GreatPerson')	
	pCity.changeFreeSpecialistCount(iPick,1)

def LairResultGoldenage(pAdventurer, iValue):			

	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	pPlayer.changeGoldenAgeTurns(iValue)
	
def CanAddRandomBuilding(pCity,iBuildingType):	
	if pCity.canConstruct(iBuildingType, False, False, False):
		if not gc.getBuildingInfo(iBuildingType).isDistrict():	
			iClass=gc.getBuildingInfo(iBuildingType).getBuildingClassType()	
			if not (isWorldWonderClass(iClass)):
				if not (isTeamWonderClass(iClass)):
					if not (isNationalWonderClass(iClass)):
						return true
	return false
	
def reqLairResultBuilding(pAdventurer, iValue):			

	pCity = getNearestCity(pAdventurer)
	listBuildings = []	

	if pCity==-1:
		return false
		
	for i in range(gc.getNumBuildingInfos()):
		if CanAddRandomBuilding(pCity,i):
			listBuildings.append(i)
			
	return len(listBuildings)>0
	
def LairResultBuilding(pAdventurer, iValue):			

	pCity = getNearestCity(pAdventurer)
	listBuildings = []	

	for i in range(gc.getNumBuildingInfos()):
		if CanAddRandomBuilding(pCity,i):
			listBuildings.append(i)
			
	iPick = gc.getGame().getSorenRandNum(len(listBuildings),'LairResult Building')				
	pCity.setNumRealBuilding(listBuildings[iPick],1)

def reqLairResultAggressive(pAdventurer, iValue):			

	return (not pAdventurer.isHasPromotion(gc.getInfoTypeForString('PROMOTION_AGGRESSIVE')))
	
def LairResultAggressive(pAdventurer, iValue):			

	pAdventurer.setHasPromotion(gc.getInfoTypeForString('PROMOTION_AGGRESSIVE'),true)

def reqLairResultWorker(pAdventurer, iValue):			

	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	iUnitClassWorker=gc.getInfoTypeForString('UNITCLASS_WORKER')
	iUnitClassSlave=gc.getInfoTypeForString('UNITCLASS_SLAVE')	
	if pPlayer.getUnitClassCount(iUnitClassWorker)>5:
		return false
	iCiv=pPlayer.getCivilizationType()
	iSlave = gc.getCivilizationInfo(iCiv).getCivilizationUnits(iUnitClassSlave)
	if iSlave==-1:
		return false
	
def LairResultWorker(pAdventurer, iValue):			

	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	iImprovement=pAdventurer.plot().getImprovementType()
	iUnitClassWorker=gc.getInfoTypeForString('UNITCLASS_WORKER')
	iUnitClassSlave=gc.getInfoTypeForString('UNITCLASS_SLAVE')		
	iCiv=pPlayer.getCivilizationType()	
	iSlave = gc.getCivilizationInfo(iCiv).getCivilizationUnits(iUnitClassSlave)
	if iImprovement==gc.getInfoTypeForString('IMPROVEMENT_GOBLIN_FORT'):
		iSlave=gc.getInfoTypeForString('UNIT_WORKER_CLAN')
	pPlayer.initUnit(iSlave,pAdventurer.getX(),pAdventurer.getY(), UnitAITypes.UNITAI_WORKER, DirectionTypes.DIRECTION_SOUTH)
	
def LairResultStone(pAdventurer, iValue):			

	gc.getPlayer(pAdventurer.getOwner()).changeGlobalYield(YieldTypes.YIELD_STONE,iValue)
	
def LairResultLumber(pAdventurer, iValue):			

	gc.getPlayer(pAdventurer.getOwner()).changeGlobalYield(YieldTypes.YIELD_LUMBER,iValue)

def LairResultLeather(pAdventurer, iValue):			

	gc.getPlayer(pAdventurer.getOwner()).changeGlobalYield(YieldTypes.YIELD_LEATHER,iValue)

def LairResultHerb(pAdventurer, iValue):			

	gc.getPlayer(pAdventurer.getOwner()).changeGlobalYield(YieldTypes.YIELD_HERB,iValue)

def LairResultMetal(pAdventurer, iValue):			

	gc.getPlayer(pAdventurer.getOwner()).changeGlobalYield(YieldTypes.YIELD_METAL,iValue)	
	
	
def getEquipment(pAdventurer):

	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	iImprovement=pAdventurer.plot().getImprovementType()

	for i in range(gc.getNumPromotionInfos()):
		if pPlayer.canEquipUnit(pAdventurer,i,true,true):
			return i
			
	return -1
	
	
def reqLairResultEquipment(pAdventurer, iValue):
	
	return not (getEquipment(pAdventurer)==-1)
	
def LairResultEquipment(pAdventurer, iValue):

	pPlayer=gc.getPlayer(pAdventurer.getOwner())
	
	EquipmentList = []

	for i in range(gc.getNumPromotionInfos()):
		if pPlayer.canEquipUnit(pAdventurer,i,true,true):
			EquipmentList.append(i)
	
	iPick = gc.getGame().getSorenRandNum(len(EquipmentList),"pick dungeonresult Equipment")
	iEquipment = EquipmentList[iPick]
	pAdventurer.equip(iEquipment,true)
	CyInterface().addMessage(pAdventurer.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_NEW_EQUIPMENT",(gc.getUnitInfo(pAdventurer.getUnitType()).getDescription(),gc.getPromotionInfo(iEquipment).getDescription())),'AS2D_DISCOVERBONUS',1,gc.getPromotionInfo(iEquipment).getButton(),ColorTypes(8),pAdventurer.getX(),pAdventurer.getY(),True,True)		
	return
			

def reqLairResultGrigoriGreaterMuseum(pAdventurer, iValue):

	pPlayer = gc.getPlayer(pAdventurer.getOwner())
	
	if pPlayer.getCivilizationType!=gc.getInfoTypeForString('CIVILIZATION_GRIGORI'):
		return false

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_MUSEUM_GREATER'))>0:
		return false
		
	pCity = getNearestCity(pAdventurer)
	
	if pCity ==0:
		return false
		
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_MUSEUM'))==0:
		return false
		
	return true
			
def LairResultGrigoriGreaterMuseum(pAdventurer, iValue):

	pPlayer = gc.getPlayer(pAdventurer.getOwner())			
	pCity = getNearestCity(pAdventurer)

	pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_MUSEUM_GREATER'),1)
	
	CyInterface().addMessage(pAdventurer.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISCOVERED_ANCIENT_TREASURIES",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),pAdventurer.getX(),pAdventurer.getY(),True,True)	
	
	iAdventurer = gc.getInfoTypeForString('UNIT_ADVENTURER')
	pPlayer.initUnit(iAdventurer,pCity.getX(),pCity.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)		
	pPlayer.initUnit(iAdventurer,pCity.getX(),pCity.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)	