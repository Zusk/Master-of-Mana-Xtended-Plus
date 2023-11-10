import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def reqWinterfeast(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pPlayer = caster.getOwner()
	if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) == gc.getInfoTypeForString('CIVIC_WINTER_COURT'):
		if (pCity.unhappyLevel(0)>0):
			if caster.getTeam() == pCity.getTeam():
#				if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SNOW'):
				return True
	return False

def spellWinterfeast(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pCity.changePopulation(1)
	
def reqCreateCottage(caster):
	pPlot = caster.plot()

	iCottage = gc.getInfoTypeForString('IMPROVEMENT_COTTAGE')
	if iCottage!=-1 and pPlot.getImprovementType() == iCottage:
		return False

	if (pPlot.isCity  () == True):
		return False

	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SNOW'):
		return True
	return False
	
def spellCreateCottage(caster):
	pPlot = caster.plot()
	iCottage = gc.getInfoTypeForString('IMPROVEMENT_COTTAGE')
	if pPlot.getImprovementType() != iCottage:
		pPlot.setImprovementType(iCottage)
