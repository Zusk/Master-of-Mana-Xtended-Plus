import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#Jotnar
def onJotnarCityBuilt(argsList):
	'City Built'
	city = argsList[0]
	pPlot = city.plot()
	pPlayer = gc.getPlayer(city.getOwner())
		
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_JOTNAR'):
		if not city.isCapital():
			if pPlayer.getNumCities() > 1:
				city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_JOT_STAEDDING'), 1)
		
def onJotnarCityAcquired(argsList):
	'City Acquired'
	iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
	pPlayer = gc.getPlayer(iNewOwner)

	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_JOTNAR'):
		if pPlayer.getNumCities() > 1:
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_JOT_STAEDDING'), 1)
#			if pCity.getPopulation() > 8:
#				pCity.setPopulation(8)

def onJotnarDoTurn(argsList):
	iGameTurn  = argsList[0]
	iPlayer = argsList[1]
	pPlayer = gc.getPlayer(iPlayer)
		
	if not (pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_JOTNAR')):
		return

	py = PyPlayer(iPlayer)
	iGiantKin = gc.getInfoTypeForString('PROMOTION_JOT_GIANTKIN')
	iOld1 = gc.getInfoTypeForString('PROMOTION_JOT_SEASONED_GIANT')
	iOld2 = gc.getInfoTypeForString('PROMOTION_JOT_RENOWNED_GIANT')
	iOld3 = gc.getInfoTypeForString('PROMOTION_JOT_PROMINENT_GIANT')
	iOld4 = gc.getInfoTypeForString('PROMOTION_JOT_LEGENDARY_GIANT')
	
	iMinTurns1 = 35 * gc.getGameSpeedInfo(CyGame().getGameSpeedType()).getTrainPercent()
	iMinTurns1 /=100

	iMinTurns2 = 70 * gc.getGameSpeedInfo(CyGame().getGameSpeedType()).getTrainPercent()
	iMinTurns2 /=100

	iMinTurns3 = 140 * gc.getGameSpeedInfo(CyGame().getGameSpeedType()).getTrainPercent()
	iMinTurns3 /=100

	iMinTurns4 = 250 * gc.getGameSpeedInfo(CyGame().getGameSpeedType()).getTrainPercent()
	iMinTurns4 /=100
	
	for pUnit in py.getUnitList():
		if pUnit.isHasPromotion(iGiantKin):
			if(iGameTurn - pUnit.getGameTurnCreated())>iMinTurns1:
				if not pUnit.isHasPromotion(iOld1):
					pUnit.setHasPromotion(iOld1,true)
				if(iGameTurn - pUnit.getGameTurnCreated())>iMinTurns2:
					if not pUnit.isHasPromotion(iOld2):
						pUnit.setHasPromotion(iOld2,true)
					if(iGameTurn - pUnit.getGameTurnCreated())>iMinTurns3:
						if not pUnit.isHasPromotion(iOld3):
							pUnit.setHasPromotion(iOld3,true)
						if(iGameTurn - pUnit.getGameTurnCreated())>iMinTurns4:
							if not pUnit.isHasPromotion(iOld4):
								pUnit.setHasPromotion(iOld4,true)
			
def onJotnarCityDoTurn(argsList):
	'City Production'
	pCity = argsList[0]
	iPlayer = argsList[1]
	pPlot = pCity.plot()
	iPlayer = pCity.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	if gc.getPlayer(iPlayer).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_JOTNAR'):
#		if not pCity.isCapital():
#			if pCity.getPopulation() > 8:
#				iPop = 8
#				pCity.setPopulation(iPop)
#				pCity.changeCulture(iPlayer, 10, True)

		iGiantKinPromotion= gc.getInfoTypeForString('PROMOTION_JOT_GIANTKIN')
		iCurrentNumberOfGiants = 0

		for pUnit in PyPlayer(iPlayer).getUnitList():
			if pUnit.isHasPromotion(iGiantKinPromotion):
				iCurrentNumberOfGiants += 1

		iMaxGiants = (3 * pPlayer.countNumBuildings(gc.getInfoTypeForString('BUILDING_JOT_STAEDDING'))) + pPlayer.getUnitSupportLimitTotal() / 5

		iGiantSpawnChance = 10 * (iMaxGiants - iCurrentNumberOfGiants)

		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
			iGiantSpawnChance *= 1.5
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
			iGiantSpawnChance /= 1.5
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
			iGiantSpawnChance /= 3

		if CyGame().getSorenRandNum(1000, "Giants born") < iGiantSpawnChance:
			iUnit = gc.getInfoTypeForString('UNIT_JOT_ADULT')
			newUnit = pPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_GIANT_BORN",()),'AS2D_DISCOVERBONUS',1,gc.getUnitInfo(newUnit.getUnitType()).getButton(),ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
			pCity.applyBuildEffects(newUnit)


		