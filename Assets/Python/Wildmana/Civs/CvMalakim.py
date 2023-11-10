import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#START Malakim

iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')					
iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')					
iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')					
iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')					
iVolcano = gc.getInfoTypeForString('IMPROVEMENT_VOLCANO')
iOasis = gc.getInfoTypeForString('FEATURE_OASIS')		
iScrubs = gc.getInfoTypeForString('FEATURE_SCRUB')
iForest = gc.getInfoTypeForString('FEATURE_FOREST')
iForestAncient = gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')	
iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')	
iFloodplains = gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')	
iHorse = gc.getInfoTypeForString('BONUS_HORSE')
iCamel = gc.getInfoTypeForString('BONUS_CAMEL')
iUnitCombatRecon = gc.getInfoTypeForString('UNITCOMBAT_RECON')
iUnitCombatMounted = gc.getInfoTypeForString('UNITCOMBAT_MOUNTED')
iPromRecon = gc.getInfoTypeForString('PROMOTION_SONS_OF_THE_DESERT_RECON')
iPromMounted = gc.getInfoTypeForString('PROMOTION_SONS_OF_THE_DESERT_MOUNTED')
iMalakim = gc.getInfoTypeForString('CIVILIZATION_MALAKIM')	
	
def doDesertEconomy(ePlayer,eTrait):
	pPlayer = gc.getPlayer(ePlayer)
	eTeam = gc.getTeam(pPlayer.getTeam())
	
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.getOwner() == ePlayer: 
			iFeature = pPlot.getFeatureType()
			iImprovement = pPlot.getImprovementType()			
			if not (pPlot.isWater() or pPlot.isPeak() or iImprovement == iVolcano):
				iBonus = pPlot.getBonusType(-1)
				iTerrain = pPlot.getTerrainType()			
				if pPlot.getRouteType() != -1:
					if CyGame().getSorenRandNum(100, "no roads in Desert") < 20:
						if pPlot.getTerrainType() == iDesert:						
							pPlot.setRouteType(-1)
				if iTerrain == iDesert:															
					if iFeature == iForest or iFeature == iJungle or iFeature == iForestAncient:
						pPlot.setFeatureType(iScrubs,1)										
				elif iTerrain != iDesert:  #disable for now, use different code below
					if iTerrain == iSnow:
						if CyGame().getSorenRandNum(100, "melt Snow") < 5:											
							pPlot.setTerrainType(iTundra,true,true)												
					elif iTerrain != iPlains:
						if CyGame().getSorenRandNum(100, "dry Terrain") < 5:											
							pPlot.setTerrainType(iPlains,true,true)																		
					elif CyGame().getSorenRandNum(100, "no roads in Desert") < 20:					
						pPlot.setTerrainType(iDesert,true,true)						
						if pPlot.isRiverSide() and pPlot.getBonusType(pPlayer.getTeam())==-1:
							if not pPlot.isHills():
								if CyGame().getSorenRandNum(100, "Discover Floodplains") < 50:							
									pPlot.setFeatureType(iFloodplains,1)
						elif pPlot.isCity() and (not pPlot.isHills()):
							pPlot.setFeatureType(iOasis,1)														
						elif CyGame().getSorenRandNum(100, "Discover Oasis") < 33:							
							if not pPlot.isHills():
								if iBonus == -1:
									pPlot.setFeatureType(iOasis,1)							
						elif CyGame().getSorenRandNum(100, "Discover Scrubs") < 20:							
							pPlot.setFeatureType(iScrubs,1)
						if iBonus == iHorse:
							pPlot.setBonusType(iCamel)
						elif iBonus == -1:
							if not pPlot.isHills:
								pPlot.setBonusType(iCamel)
				if iTerrain == iPlains and 1>2:
					if CyGame().getSorenRandNum(100, "no roads in Desert") < 5:					
						pPlot.setTerrainType(iDesert,true,true)						
						if pPlot.isRiverSide() and pPlot.getBonusType(pPlayer.getTeam()) == -1:
							if not pPlot.isHills():
								if CyGame().getSorenRandNum(100, "Discover Floodplains") < 50:							
									pPlot.setFeatureType(iFloodplains,1)
						elif pPlot.isCity() and (not pPlot.isHills()):
							pPlot.setFeatureType(iOasis, 1)														
						elif CyGame().getSorenRandNum(100, "Discover Oasis") < 33:							
							if not pPlot.isHills():
								if iBonus == -1:
									pPlot.setFeatureType(iOasis, 1)							
						elif CyGame().getSorenRandNum(100, "Discover Scrubs") < 20:							
							pPlot.setFeatureType(iScrubs,1)
						if iBonus == iHorse:
							pPlot.setBonusType(iCamel)
						elif iBonus == -1:
							if not pPlot.isHills:
								pPlot.setBonusType(iCamel)
				

	
def doMalakimTurn(argsList):
	'Called at the beginning of the end of each turn'
	iGameTurn = argsList[0]
	
	iMalakim = gc.getInfoTypeForString('CIVILIZATION_MALAKIM')	
	iAreDesertPeopleHere=0
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)	
		if pPlayer.getCivilizationType()==iMalakim:
			iAreDesertPeopleHere+=1
			#Horses from Palace
			if pPlayer.getNumAvailableBonuses(gc.getInfoTypeForString('BONUS_CAMEL'))>0:
				if pPlayer.getCapitalCity().getBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), gc.getInfoTypeForString('BONUS_HORSE')) == 0:
					pPlayer.getCapitalCity().setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), gc.getInfoTypeForString('BONUS_HORSE'), 1)												
			else:
				if pPlayer.getCapitalCity().getBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), gc.getInfoTypeForString('BONUS_HORSE')) > 0:			
					pPlayer.getCapitalCity().setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), gc.getInfoTypeForString('BONUS_HORSE'), -1)												
			
	if iAreDesertPeopleHere==0:
		return
	
						
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)	
		if pPlayer.getCivilizationType()==iMalakim:						
			py = PyPlayer(iPlayer)					
			for pUnit in py.getUnitList():
				if pUnit.getUnitCombatType()==iUnitCombatRecon:
					if pUnit.plot().getTerrainType()==iDesert:
						pUnit.setHasPromotion(iPromRecon,true)						
					else:
						pUnit.setHasPromotion(iPromRecon,false)						
				elif pUnit.getUnitCombatType()==iUnitCombatMounted:
					if pUnit.plot().getTerrainType()==iDesert:
						pUnit.setHasPromotion(iPromMounted,true)						
					else:
						pUnit.setHasPromotion(iPromMounted,false)						
						
#END Malakim


		