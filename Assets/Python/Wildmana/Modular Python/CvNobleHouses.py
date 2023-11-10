import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#HOUSES OF EREBUS

def	GuildHouseBuild(argsList):
	'Building Completed'
	pCity, iBuildingType = argsList
	player = pCity.getOwner()
	pPlayer = gc.getPlayer(player)
	pPlot = pCity.plot()
	game = gc.getGame()
	iBuildingClass = gc.getBuildingInfo(iBuildingType).getBuildingClassType()

	if iBuildingClass == gc.getInfoTypeForString('BUILDINGCLASS_HOUSE_PHIARLAN_MASQUERADE'):
		pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARTIST'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

