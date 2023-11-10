import CvUtil
from CvPythonExtensions import *
import CustomFunctions
import PyHelpers

cf = CustomFunctions.CustomFunctions()
gc = CyGlobalContext()
localText = CyTranslator()
PyPlayer = PyHelpers.PyPlayer

#Unblemished Events Mod Begins#	
		
def helpUnblemishedRioting3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_UNBLEMISHED_RIOTING3_HELP", ())
	return szHelp
	
def helpUnblemishedRioting2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_UNBLEMISHED_RIOTING2_HELP", ())
	return szHelp
	
def helpUnblemishedMoot3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_UNBLEMISHED_MOOT3_HELP", ())
	return szHelp
	
def helpUnblemishedmoot2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_UNBLEMISHED_MOOT2_HELP", ())
	return szHelp
	
def helpUnblemishedComplacency2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_UNBLEMISHED_COMPLACENCY2_HELP", ())
	return szHelp
	
def helpUnblemishedComplacency3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	szHelp = localText.getText("TXT_KEY_EVENT_UNBLEMISHED_COMPLACENCY3_HELP", ())
	return szHelp
	
def applyUnblemishedMoot3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()

	iAccord = pPlayer.getPurityCounter()
	iAccord -= 30
	pPlayer.setPurityCounter(iAccord)
	pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNBLEMISHED_MOOT'), 0)
	CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_WTF",()),'AS2D_SPELL_FIRE_ELEMENTAL',1,'Art/Interface/Buttons/Fire.dds',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
	
def candoUnblemishedMoot3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()

	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNBLEMISHED_MOOT')) == 0:
		return False
	
def candounblemishedmoot2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	
	if pPlayer.getGlobalYield(YieldTypes.YIELD_METAL)>=250:
		if pPlayer.getGlobalYield(YieldTypes.YIELD_LEATHER)>=250:
			return True
			
	return False
	
def applyUnblemishedmoot2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()
	
	pPlayer.changeGlobalYield(YieldTypes.YIELD_METAL,-250)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_LEATHER,-250)
	pCity.changeHappinessTimer(10)
	
	
def	applyUnblemishedMoot1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	
	pPlayer.changeGold(-175)

def candoUnblemishedMoot1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	
	iGold = pPlayer.getGold()
	if pPlayer.getGold() < 175:
		return False
		
	return True
	
def applyUnblemishedComplacency3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)

	iAccord = pPlayer.getPurityCounter()
	iAccord -= 15
	pPlayer.setPurityCounter(iAccord)
	
def candounblemishedcomplacency2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	
	if pPlayer.getGlobalYield(YieldTypes.YIELD_LUMBER)>=250:
		if pPlayer.getGlobalYield(YieldTypes.YIELD_STONE)>=500:
			return True
			
	return False
	
def applyUnblemishedComplacency2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()
	
	pPlayer.changeGlobalYield(YieldTypes.YIELD_LUMBER,-250)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_STONE,-500)
	pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNBLEMISHED_MOOT'), 1)
	
def	applyUnblemishedComplacency1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	
	pPlayer.changeGold(-100)

def candoUnblemishedComplacency1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	
	iGold = pPlayer.getGold()
	if pPlayer.getGold() < 100:
		return False
		
	return True

def candoUnblemishedRioting1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
	
	iGold = pPlayer.getGold()
	if pPlayer.getGold() < 300:
		return False
		
	return True
	
def	applyUnblemishedRioting1(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
#	pPlayer = gc.getPlayer(iPlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()
	
	pPlayer.changeGold(-300)
	pCity.setOccupationTimer(0)
	
def candoUnblemishedRioting2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
	pPlayer = gc.getPlayer(iPlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()
#	iCity = argsList[2]
#	pCity = pPlayer.getCity(iCity)
	
	if pCity.getPopulation() < 2:
		return False
		
	return True

def applyUnblemishedRioting2(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]

#	iPlayer = kTriggeredData.ePlayer
#	pPlayer = gc.getPlayer(iPlayer)

#	iCity = argsList[2]
#	pCity = pPlayer.getCity(iCity)
	
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()
	
	pCity.changePopulation(-1)
	if CyGame().getSorenRandNum(99, "vim rioting event") < 25:
#		listBuildings = []
#		for iBuilding in range(gc.getNumBuildingInfos()):
#			if (pCity.getNumRealBuilding(iBuilding) > 0 and gc.getBuildingInfo(iBuilding).getProductionCost() > 0 and not isLimitedWonderClass(gc.getBuildingInfo(iBuilding).getBuildingClassType())):
#				listBuildings.append(iBuilding)
#
#			if len(listBuildings) > 0:
#				iBuilding = listBuildings[gc.getGame().getSorenRandNum(len(listBuildings), "Looters event building destroyed")]
#				szBuffer = localText.getText("TXT_KEY_EVENT_CITY_IMPROVEMENT_DESTROYED", (gc.getBuildingInfo(iBuilding).getTextKey(), ))
#				pCity.setNumRealBuilding(iBuilding, 0)
		iCount = 0
		iDemon = gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS')
		for iBuilding in range(gc.getNumBuildingInfos()):
			if iBuilding != iDemon:
				if (pCity.getNumRealBuilding(iBuilding) > 0 and gc.getBuildingInfo(iBuilding).getProductionCost() > 0 and not isLimitedWonderClass(gc.getBuildingInfo(iBuilding).getBuildingClassType())):
					if gc.getBuildingInfo(iBuilding).getConquestProbability() != 100:
						if CyGame().getSorenRandNum(99, "City Fire") <= 9:
							pCity.setNumRealBuilding(iBuilding, 0)
							CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE",(gc.getBuildingInfo(iBuilding).getDescription(), )),'',1,gc.getBuildingInfo(iBuilding).getButton(),ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
							iCount += 1
		if iCount == 0:
			CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE_NO_DAMAGE",()),'AS2D_SPELL_FIRE_ELEMENTAL',1,'Art/Interface/Buttons/Fire.dds',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
	
	pCity.setOccupationTimer(0)
	
def	applyUnblemishedRioting3(argsList):
	iEvent = argsList[0]
	kTriggeredData = argsList[1]
	iPlayer = kTriggeredData.ePlayer
#	pPlayer = gc.getPlayer(iPlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	pPlayer = gc.getPlayer(kTriggeredData.ePlayer)
#	pCity = pPlayer.getCity(kTriggeredData.iCityId)
	
	
	pPlot = gc.getMap().plot(kTriggeredData.iPlotX, kTriggeredData.iPlotY)
	pCity = pPlot.getPlotCity()
	
	iAccord = pPlayer.getPurityCounter()
	iAccord += 5
	pPlayer.setPurityCounter(iAccord)
	
	iRnd = CyGame().getSorenRandNum(3, "unblemished rioting 3")
	if iRnd >= 2:
		iRevoltTurns = 4
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
			iRevoltTurns = 3
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
			iRevoltTurns = 6
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
			iRevoltTurns = 12
		pCity.changeOccupationTimer(iRevoltTurns)
		
	if iRnd == 1:
#		iTimer = pCity.getOccuptionTimer()
#		iTimer += iTimer	
		pCity.changeOccupationTimer(-1)
		
	if iRnd == 0:
		pCity.setOccupationTimer(0)
		
		
	
######## Unblemished EVENTS MOD EXPANDED ENDS ########