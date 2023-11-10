import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def reqFoundWyvern(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iWyvernGuard = gc.getInfoTypeForString('UNITCLASS_DRAGON_GUARDIAN')
	if pPlayer.getUnitClassCount(iWyvernGuard) >= 12:
		return True
	return False

def spellFoundWyvern(caster):
	pPlot = caster.plot()
	iHero = gc.getInfoTypeForString('UNIT_COATLANN')
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlayer.initUnit(iHero, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def reqClearSwamp(caster):
	pPlot = caster.plot()

	if caster.isHuman():
		return true

	iTargetJungle=0

	iLizJungle = gc.getInfoTypeForString('FEATURE_LIZ_JUNGLE')
	iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
	iSwamp = gc.getInfoTypeForString('IMPROVEMENT_SWAMP')

	for i in range (DirectionTypes.NUM_DIRECTION_TYPES):
		pLoopPlot=pPlot
		if i==0:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTH)
		if i==1:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHEAST)
		if i==2:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_EAST)
		if i==3:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHEAST)
		if i==4:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTH)
		if i==5:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHWEST)
		if i==6:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_WEST)
		if i==7:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHWEST)

		if not pLoopPlot.isNone():
			if pLoopPlot!=pPlot:
				if pLoopPlot.isWater() or pLoopPlot.isPeak():
					iTargetJungle+=50
				if pLoopPlot.getFeatureType()==iJungle or pLoopPlot.getFeatureType()==iLizJungle:
					iTargetJungle+=50
				if pLoopPlot.getImprovementType()==iSwamp:
					iTargetJungle+=100

	if iTargetJungle>400:
		return true

	return false

def spellClearSwamp(caster):
	pPlot = caster.plot()
	if not pPlot.isNone():
		pPlot.setImprovementType(-1)

def spellShapeJungle(caster):
	pPlot = caster.plot()
	if not pPlot.isNone():
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_SWAMP'), True, True)
		pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_LIZ_JUNGLE'), 0)

def reqCreateSwamp(caster):
	pPlot = caster.plot()

	iSwamp = gc.getInfoTypeForString('IMPROVEMENT_SWAMP')
	if iSwamp!=-1 and pPlot.getImprovementType() == iSwamp:
		return False

	if (pPlot.isCity  () == True) and (pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SWAMP')):
		return false

	if caster.isHuman():
		return true

	iTargetJungle=0

	iLizJungle = gc.getInfoTypeForString('FEATURE_LIZ_JUNGLE')
	iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
	iSwamp = gc.getInfoTypeForString('IMPROVEMENT_SWAMP')

	for i in range (DirectionTypes.NUM_DIRECTION_TYPES):
		pLoopPlot=pPlot
		if i==0:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTH)
		if i==1:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHEAST)
		if i==2:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_EAST)
		if i==3:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHEAST)
		if i==4:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTH)
		if i==5:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_SOUTHWEST)
		if i==6:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_WEST)
		if i==7:
			pLoopPlot=plotDirection(pPlot.getX(),pPlot.getY(),DirectionTypes.DIRECTION_NORTHWEST)

		if not pLoopPlot.isNone():
			if pLoopPlot!=pPlot:
				if pLoopPlot.isWater() or pLoopPlot.isPeak():
					iTargetJungle+=50
				if pLoopPlot.getFeatureType()==iJungle or pLoopPlot.getFeatureType()==iLizJungle:
					iTargetJungle+=50
				if pLoopPlot.getImprovementType()==iSwamp:
					iTargetJungle+=100

	if iTargetJungle<400:
		return true

	return false

def spellCreateSwamp(caster):
	pPlot = caster.plot()
	if not pPlot.isNone():
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_SWAMP'), True, True)
