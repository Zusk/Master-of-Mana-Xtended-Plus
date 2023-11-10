import CvUtil
from CvPythonExtensions import *

import CvEventManager
import PyHelpers
import CustomFunctions

PyInfo = PyHelpers.PyInfo
PyPlayer = PyHelpers.PyPlayer
gc = CyGlobalContext()
cf = CustomFunctions.CustomFunctions()

def reqExploreLairEpic(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	# Elohim blocked
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ELOHIM'):
		return False
	if pPlayer.isHuman() == False:
		if CyGame().getGameTurn()<100:
			return False
		if pCaster.plot().getImprovementType()!=-1:
			if gc.getImprovementInfo(pCaster.plot().getImprovementType()).isUnique():
				if CyGame().getGameTurn()<200:
					return False
				if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_ELOHIM'):
					return False
	if pCaster.plot().getOwner()!=-1:
		if pCaster.getOwner()!=pCaster.plot().getOwner():
			return False
	if pCaster.isOnlyDefensive():
		return False
	if pCaster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_SIEGE'):
		return False
	if pCaster.isBarbarian():
		return False
	if pCaster.getSpecialUnitType() == gc.getInfoTypeForString('SPECIALUNIT_SPELL'):
		return False
	if pCaster.getSpecialUnitType() == gc.getInfoTypeForString('SPECIALUNIT_BIRD'):
		return False
	pPlot = pCaster.plot()
#added Sephi
	if pPlot.getImprovementType() == -1:
		return False
	if gc.getImprovementInfo(pPlot.getImprovementType()).getSpawnUnitCiv()!=gc.getInfoTypeForString('CIVILIZATION_BARBARIAN'):
		return True
	if not pPlot.isWater():
		pPlayer = gc.getPlayer(pCaster.getOwner())
		iTeam = pPlayer.getTeam()
		eTeam = gc.getTeam(iTeam)
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		if not eTeam.isAtWar(bPlayer.getTeam()):
			return False
	return True

def spellExploreLairEpic(pCaster):
	pPlot = pCaster.plot()
	iRnd = CyGame().getSorenRandNum(100, "Explore Lair") + pCaster.getLevel()
	iDestroyLair = 0
	if iRnd < 54:
		iDestroyLair = cf.exploreLairBigBad(pCaster)
	if iRnd >= 54:
		iDestroyLair = cf.exploreLairBigGood(pCaster)
	if iDestroyLair > CyGame().getSorenRandNum(100, "Explore Lair"):
#		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_LAIR_DESTROYED", ()),'AS2D_POSITIVE_DINK',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		iImprovement = pPlot.getImprovementType()
		iExplored = doGetExploredImprovement(iImprovement)
		pPlot.setImprovementType(iExplored)
		#  regenerate signs?
	pCaster.finishMoves()
	pCaster.changeExperience(1, -1, false, false, false)

def reqSanctifyUF(pCaster):
	pPlot = pCaster.plot()
	iImprovement = pPlot.getImprovementType()
	if iImprovement == -1:
		return False
	if not gc.getImprovementInfo(iImprovement).isUnique():
		return False
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_BRADELINES_WELL') or iImprovement == gc.getInfoTypeForString('IMPROVEMENT_BRADELINES_WELL_EXPLORED'):
		return True
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER') or iImprovement == gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER_EXPLORED'):
		return True
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC') or iImprovement == gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC_EXPLORED'):
		return True
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_TOWER_OF_EYES') or iImprovement == gc.getInfoTypeForString('IMPROVEMENT_TOWER_OF_EYES_EXPLORED'):
		return True
	return False

def spellSanctifyUF(pCaster):
	pPlot = pCaster.plot()
	iUF = pPlot.getImprovementType()
	pPlayer = gc.getPlayer(pCaster.getOwner())
	bSealed = false
	if iUF == gc.getInfoTypeForString('IMPROVEMENT_AIFON_ISLE') or iUF == gc.getInfoTypeForString('IMPROVEMENT_AIFON_ISLE_EXPLORED'):
		pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_AIFON_ISLE_SEALED'))
		bSealed = true
	elif iUF == gc.getInfoTypeForString('IMPROVEMENT_BRADELINES_WELL') or iUF == gc.getInfoTypeForString('IMPROVEMENT_BRADELINES_WELL_EXPLORED'):
		pPlot.setBonusType(-1)
		# Bradeline's Well can't be sealed
		iPlayer = pCaster.getOwner()
		CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_BRADELINES_WELL_CANT_BE_SEALED",()),'',1,gc.getUnitInfo(pCaster.getUnitType()).getButton(),ColorTypes(7),pCaster.getX(),pCaster.getY(),True,True)
	elif iUF == gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER') or iUF == gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER_EXPLORED'):
		pPlot.setBonusType(-1)
		pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER_SEALED'))
		bSealed = true
	elif iUF == gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC') or iUF == gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC_EXPLORED'):
		pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC_SEALED'))
		bSealed = true
	elif iUF == gc.getInfoTypeForString('IMPROVEMENT_TOWER_OF_EYES') or iUF == gc.getInfoTypeForString('IMPROVEMENT_TOWER_OF_EYES_EXPLORED'):
		pPlot.setBonusType(-1)
		pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_TOWER_OF_EYES_SEALED'))
		bSealed = true
	#  regenerate signs?
#	pCaster.changeExperience(pCaster.getExperience() + 100, -1, False, False, False)
	pCaster.changeExperience(100, -1, False, False, False)
	if bSealed:
		pPlayer.setPurityCounterCache1(pPlayer.getPurityCounterCache1()+7)

def doGetExploredImprovement(iImprovement):
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_BRADELINES_WELL'):
		return gc.getInfoTypeForString('IMPROVEMENT_BRADELINES_WELL_EXPLORED')
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER'):
		return gc.getInfoTypeForString('IMPROVEMENT_BROKEN_SEPULCHER_EXPLORED')
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC'):
		return gc.getInfoTypeForString('IMPROVEMENT_PYRE_OF_THE_SERAPHIC_EXPLORED')
	if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_TOWER_OF_EYES'):
		return gc.getInfoTypeForString('IMPROVEMENT_TOWER_OF_EYES_EXPLORED')
	return -1



