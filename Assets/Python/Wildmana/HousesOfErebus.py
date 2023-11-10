import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()
			
def reqOpenMiningCompany(caster):

	if not caster.plot().isHills():
		return false

	if caster.plot().getImprovementType() != -1 and gc.getImprovementInfo(caster.plot().getImprovementType()).isUnique():
		return false
		
	iX = caster.plot().getX()
	iY = caster.plot().getY()
	iCountHills=0
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if not (pPlot2.isNone() or pPlot2.isImpassable()):
				if pPlot2.isHills() and pPlot2.getBonusType(-1)!=-1:
					return false
				if pPlot2.isHills():
					iCountHills+=1
				
	if iCountHills<4:
		return false
		
	return true
	
def spellOpenMiningCompany(caster):

	if CyGame().getSorenRandNum(100, "Mining failed")>50:
		return
	
	lBonusList = []
	lBonusList = lBonusList + ['BONUS_IRON']
	lBonusList = lBonusList + ['BONUS_COPPER']
	lBonusList = lBonusList + ['BONUS_MITHRIL']
	lBonusList = lBonusList + ['BONUS_GEMS']	
	lBonusList = lBonusList + ['BONUS_SILVER']		
	lBonusList = lBonusList + ['BONUS_AMBER']			
	
	sBonus = lBonusList[CyGame().getSorenRandNum(len(lBonusList), "Pick Bonus")-1]
	caster.plot().setBonusType(gc.getInfoTypeForString(sBonus))	
	
def reqOpenFishingCompany(caster):

	if not caster.plot().isCoastalLand():
		return false
	
	iX = caster.plot().getX()
	iY = caster.plot().getY()
	iCountWater=0
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if not (pPlot2.isNone() or pPlot2.isImpassable()):
				if pPlot2.isWater() and pPlot2.getBonusType(-1)!=-1:
					return false
				if pPlot2.isWater():
					iCountWater+=1
				
	if iCountWater<4:
		return false
		
	return true
	
def spellOpenFishingCompany(caster):

	if CyGame().getSorenRandNum(100, "Fishing failed")>50:
		return
	
	lBonusList = []
	lBonusList = lBonusList + ['BONUS_FISH']
	lBonusList = lBonusList + ['BONUS_CRAB']
	lBonusList = lBonusList + ['BONUS_WHALE']
	lBonusList = lBonusList + ['BONUS_CLAM']	
	
	sBonus = lBonusList[CyGame().getSorenRandNum(len(lBonusList), "Pick Bonus")-1]

	iX = caster.plot().getX()
	iY = caster.plot().getY()

	iCountWater=0
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if not (pPlot2.isNone() or pPlot2.isImpassable()):
				if pPlot2.isWater():
					iCountWater+=1

	iRand=CyGame().getSorenRandNum(iCountWater, "How Much is the Fish")
	iCountWater=0
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if not (pPlot2.isNone() or pPlot2.isImpassable()):
				if pPlot2.isWater():
					if iCountWater==iRand:
						pPlot2.setBonusType(gc.getInfoTypeForString(sBonus))
						return
					iCountWater+=1
	
		