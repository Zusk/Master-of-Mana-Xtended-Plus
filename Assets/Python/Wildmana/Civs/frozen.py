import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

#	Added in Frozen: TC01
#	Custom spell functions added in Frozen module. They do not need specific statements since they are only called from the XML. They are:
#		reqWintering checks if the Wintering worldspell can be casted
#		spellWintering does the effects of the Wintering worldspell
#		spellSnowfallPassive causes passive spread ice terrain as an Ascended moves about the map
#		reqFreezeForest checks if the Freeze Forest spell can be used
#		spellFreezeForest does the effects of the Freeze Forest spell
#		effectNested is the effect of Young being hatched from Nested units
#		effectYoung is the effect of a Young dying

def reqWintering(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if not pPlayer.isHuman():
			if caster.getOwner() == pPlot.getOwner():
				if pPlot.getTerrainTypeCount(gc.getInfoTypeForString('TERRAIN_SNOW')) < 20:
					return False
		return True

def spellWintering(caster):
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SNOW'):
			if pPlot.getOwner() == caster.getOwner():
				pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_WINTER'), 0)
			elif CyGame().getSorenRandNum(2, "Wintering: create blizzard on snow"):
				pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_BLIZZARD'), 0)
		elif pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_TUNDRA'):
			if pPlot.getOwner() == caster.getOwner():
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_SNOW'),True,True)
				pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_WINTER'), 0)
			else:
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_SNOW'),True,True)
				if CyGame().getSorenRandNum(2, "Wintering: create blizzard on snow"):
					pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_BLIZZARD'), 0)
		elif pPlot.getOwner() == caster.getOwner():
			if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_DESERT'):
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_TUNDRA'),True,True)
			else:
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_SNOW'),True,True)
				pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_WINTER'), 0)
		for e in range(pPlot.getNumUnits()):
			pUnit = pPlot.getUnit(e)
			if not (pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WINTERBORN')) or pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WINTERED'))):
				if not pUnit.getOwner() == gc.getInfoTypeForString('CIVILIZATION_FROZEN'):
					if not pUnit.getDomainType() == gc.getInfoTypeForString('DOMAIN_WATER'):
						if pUnit.isAlive():
							pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WINTERED'), True)

def spellSnowfallPassive(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlot = CyMap().plot(iX, iY)
	iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
	iFloodPlains = gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')
	iForest = gc.getInfoTypeForString('FEATURE_FOREST')
	iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
	iScrub = gc.getInfoTypeForString('FEATURE_SCRUB')
	iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
	iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
	iIce = gc.getInfoTypeForString('FEATURE_ICE')
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pLoopPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				iRnd = CyGame().getSorenRandNum(12, "Snowfall") + 6
				if not pLoopPlot.isWater():
					if pLoopPlot.getTerrainType() != iSnow:
						pLoopPlot.setTempTerrainType(iSnow, iRnd)
						if pLoopPlot.getImprovementType() == iSmoke:
							pLoopPlot.setImprovementType(-1)
						iFeature = pLoopPlot.getFeatureType()
						if iFeature == iForest:
							pLoopPlot.setFeatureType(iForest, 2)
						if iFeature == iJungle:
							pLoopPlot.setFeatureType(iForest, 2)
						if iFeature == iFlames:
							pLoopPlot.setFeatureType(-1, -1)
						if iFeature == iFloodPlains:
							pLoopPlot.setFeatureType(-1, -1)
						if iFeature == iScrub:
							pLoopPlot.setFeatureType(-1, -1)
				if pPlot.isWater():
					if pPlot.getFeatureType() != iIce:
						pPlot.setFeatureType(iIce, 0)

	return false

def reqFreezeForest(caster):
	pPlot = caster.plot()
	iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
	iForest = gc.getInfoTypeForString('FEATURE_FOREST')
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlot.getFeatureType() == -1:
		if pPlot.getTerrainType() == iSnow:
			return true
	if pPlot.getFeatureType() == iForest:
		if not pPlot.getFeatureType() == iSnow:
			return true
	if not pPlayer.isHuman():
		if caster.getOwner() == pPlot.getOwner():
			return true
	return false

def spellFreezeForest(caster):
	pPlot = caster.plot()
	iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
	iForest = gc.getInfoTypeForString('FEATURE_FOREST')
	if pPlot.getFeatureType() == -1:
		if pPlot.getTerrainType() == iSnow:
			pPlot.setFeatureType(iForest, 2)
	if pPlot.getFeatureType() == iForest:
		if not pPlot.getFeatureType() == iSnow:
			pPlot.setTerrainType(iSnow, true, true)
			pPlot.setFeatureType(iForest, 2)

#End of Frozen
