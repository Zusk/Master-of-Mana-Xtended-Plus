import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#STORMS

def DoStorms(argsList):
	'Called at the beginning of the end of each turn'
	iGameTurn = argsList[0]
	iDeepOcean = gc.getInfoTypeForString("TERRAIN_OCEAN_DEEP")
	iOcean = gc.getInfoTypeForString("TERRAIN_OCEAN")
	iWaterMana = gc.getInfoTypeForString("BONUS_MANA_NATURE")

	for i in range(CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.isWater():
			if pPlot.getTerrainType()==iDeepOcean and pPlot.getNumUnits()>0:
				for j in range(pPlot.getNumUnits()):
					pUnit = pPlot.getUnit(j)
					if not pUnit.isBarbarian():
						iNumWaterMana = gc.getPlayer(pUnit.getOwner()).getNumAvailableBonuses(iWaterMana)
						iDamage=25-5*iNumWaterMana
						if iDamage>0:
							pUnit.doDamageNoCaster(iDamage, 30, gc.getInfoTypeForString('DAMAGE_COLD'), false)
							CyInterface().addMessage(pUnit.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DAMAGED_BY_STORM", ()),'',1,', ,Art/Interface/Buttons/Misc_Atlas1.dds,8,6',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)

#STORMS


