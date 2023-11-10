import CvUtil
from CvPythonExtensions import *

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def reqAddInfusionToGolem(pCaster):
	pPlot=pCaster.plot()

	if pPlot.getNumUnits()<2:
		return false
		
	for i in range(pPlot.getNumUnits()):
		if pPlot.getUnit(i).getOwner() == pCaster.getOwner():
			if pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
				if not pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMONIC_INFUSION')):
					return true

	return false	

def spellAddInfusionToGolem(pCaster):
	pPlot=pCaster.plot()
	
	for i in range(pPlot.getNumUnits()):
		if pPlot.getUnit(i).getOwner() == pCaster.getOwner():
			if pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
				if not pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMONIC_INFUSION')):
					pPlot.getUnit(i).setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMONIC_INFUSION'),true)
					return	
					
def reqAddMasterCraftedGolemComponents(pCaster):
	pPlot=pCaster.plot()

	if pPlot.getNumUnits()<2:
		return false
		
	for i in range(pPlot.getNumUnits()):
		if pPlot.getUnit(i).getOwner() == pCaster.getOwner():
			if pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
				if not pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_MASTER_CRAFTED_GOLEM_COMPONENTS')):
					return true

	return false	

def spellAddMasterCraftedGolemComponents(pCaster):
	pPlot=pCaster.plot()
	
	for i in range(pPlot.getNumUnits()):
		if pPlot.getUnit(i).getOwner() == pCaster.getOwner():
			if pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
				if not pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_MASTER_CRAFTED_GOLEM_COMPONENTS')):
					pPlot.getUnit(i).setHasPromotion(gc.getInfoTypeForString('PROMOTION_MASTER_CRAFTED_GOLEM_COMPONENTS'),true)
					return

def reqAddInfusionToGolem(pCaster):
	pPlot=pCaster.plot()

	if pPlot.getNumUnits()<2:
		return false
		
	for i in range(pPlot.getNumUnits()):
		if pPlot.getUnit(i).getOwner() == pCaster.getOwner():
			if pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
				if not pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENCHANTED_OIL')):
					return true

	return false	

def spellAddInfusionToGolem(pCaster):
	pPlot=pCaster.plot()
	
	for i in range(pPlot.getNumUnits()):
		if pPlot.getUnit(i).getOwner() == pCaster.getOwner():
			if pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
				if not pPlot.getUnit(i).isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENCHANTED_OIL')):
					pPlot.getUnit(i).setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENCHANTED_OIL'),true)
					return											