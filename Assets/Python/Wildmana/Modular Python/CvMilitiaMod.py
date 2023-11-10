import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#START MILITIA PROMOTIONS CODE

def MilitiaMod(argsList):
	'Unit Completed'
	city = argsList[0]
	unit = argsList[1]
	player = PyPlayer(city.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())

#If the unit being built is alive and is not a settler, worker or hero, then begin the function
#Every tile around the city is checked and added to a count based on terrain/plot type
#Based on these results, there is a chance the unit will be given a free Winterborn, Sand Devil, Cliff Walker or Tree Warden promotion

	if (unit.isAlive() and isWorldUnitClass(unit.getUnitClassType()) == False):
		if (unit.getUnitType() != gc.getInfoTypeForString('UNIT_SETTLER') and unit.getUnitType() != gc.getInfoTypeForString('UNIT_WORKER')):
			iX = city.getX()
			iY = city.getY()
			iNumCold = 0
			iNumHot = 0
			iNumHell = 0
			iNumHill = 0
			iNumTree = 0
			iNumCoast = 0
			iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
			iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
			iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
			iBroken = gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS')
			iSands = gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')
			iShallows = gc.getInfoTypeForString('TERRAIN_SHALLOWS')
			iPerd = gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')
			iHill = gc.getInfoTypeForString('TERRAIN_HILL')
			iPeak = gc.getInfoTypeForString('TERRAIN_PEAK')

			for iiX in range(iX-2, iX+3, 1):
				for iiY in range(iY-2, iY+3, 1):
					pPlot = CyMap().plot(iiX,iiY)
					iTerrain = pPlot.getTerrainType()
					iPlot = pPlot.getPlotType()
					if iTerrain == gc.getInfoTypeForString('TERRAIN_COAST'):
						iNumCoast = iNumCoast +1
					if iTerrain == iDesert:
						iNumHot = iNumHot +1
					if (iPlot == 1 or iPlot == 0):
						iNumHill = iNumHill +1
					if (city.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_ILLIANS') and city.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_DOVIELLO')):
						if (iTerrain == iTundra or iTerrain == iSnow):
							iNumCold = iNumCold +1
					if city.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
						if (iTerrain == iSands or iTerrain == iBroken or iTerrain == iShallows or iTerrain == iPerd):
							iNumHell = iNumHell +1
					if ((pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE')) or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')):
						iNumTree = iNumTree + 1

			iRnd = CyGame().getSorenRandNum(100, "")
			ibasechance = 40
			iRnd = (iRnd*100)/ibasechance
			if (city.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SVARTALFAR') or city.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_LJOSALFAR')):
					iNumTree = iNumTree / 2
			if iRnd < (iNumTree * 1.25):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GREEN_WARDEN'), True)
			iRnd = CyGame().getSorenRandNum(100, "")
#			if iRnd < (iNumCold * 1.25):
#				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WINTERBORN'), True)
			iRnd = CyGame().getSorenRandNum(100, "")
			if iRnd < (iNumHot * 1.5):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SAND_DEVIL'), True)
			iRnd = CyGame().getSorenRandNum(100, "")
			if iRnd < iNumHell * 2:
				if not pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
					unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON_SLAYING'), True)
			iRnd = CyGame().getSorenRandNum(100, "")
			if iRnd < (iNumHill * 1.5):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CLIFF_WALKER'), True)
			iRnd = CyGame().getSorenRandNum(100, "")
			if iRnd < (iNumCoast * 1.5):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_AMPHIBIOUS'), True)
	
#END MILITIA PROMOTIONS CODE


