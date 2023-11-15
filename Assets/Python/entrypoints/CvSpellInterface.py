#Spell system and FfH specific callout python functions
#All code by Kael, all bugs by woodelf

from CvPythonExtensions import *
import CvUtil
import Popup as PyPopup
import CvScreensInterface
import sys
import PyHelpers
import CustomFunctions

from moreeventsmod import *
#SCIONS
from scions import *
#SCIONS

PyInfo = PyHelpers.PyInfo
PyPlayer = PyHelpers.PyPlayer
gc = CyGlobalContext()
cf = CustomFunctions.CustomFunctions()

def cast(argsList):
	pCaster, eSpell = argsList
	spell = gc.getSpellInfo(eSpell)
	eval(spell.getPyResult())

def canCast(argsList):
	pCaster, eSpell = argsList
	spell = gc.getSpellInfo(eSpell)
	return eval(spell.getPyRequirement())

def canResearch(argsList):
	ePlayer, eTech = argsList
	tech = gc.getTechInfo(eTech)
	pPlayer = gc.getPlayer(ePlayer)
	return eval(tech.getPyRequirement())

def effect(argsList):
	pCaster, eProm = argsList
	prom = gc.getPromotionInfo(eProm)
	eval(prom.getPyPerTurn())

def trait(argsList):
	ePlayer, eTrait = argsList
	eval(gc.getTraitInfo(eTrait).getPyPerTurn())

def civic(argsList):
	ePlayer, eCivic = argsList
	eval(gc.getCivicInfo(eCivic).getPyPerTurn())

def atRange(argsList):
	pCaster, pPlot, eImp = argsList
	imp = gc.getImprovementInfo(eImp)
	eval(imp.getPythonAtRange())

def onMove(argsList):
	pCaster, pPlot, eImp = argsList
	imp = gc.getImprovementInfo(eImp)
	eval(imp.getPythonOnMove())

def onMoveFeature(argsList):
	pCaster, pPlot, eFeature = argsList
	feature = gc.getFeatureInfo(eFeature)
	eval(feature.getPythonOnMove())

def vote(argsList):
	eVote, int = argsList
	vote = gc.getVoteInfo(eVote)
	eval(vote.getPyResult())

def miscast(argsList):
	pCaster, eSpell = argsList
	spell = gc.getSpellInfo(eSpell)
	eval(spell.getPyMiscast())

def postCombatLost(argsList):
	pCaster, pOpponent = argsList
	unit = gc.getUnitInfo(pCaster.getUnitType())
	eval(unit.getPyPostCombatLost())

def postCombatWon(argsList):
	pCaster, pOpponent = argsList
	unit = gc.getUnitInfo(pCaster.getUnitType())
	eval(unit.getPyPostCombatWon())

def findClearPlot(pUnit, plot):
	BestPlot = -1
	iBestPlot = 0
	if pUnit == -1:
		iX = plot.getX()
		iY = plot.getY()
		for iiX in range(iX-1, iX+2, 1):
			for iiY in range(iY-1, iY+2, 1):
				iCurrentPlot = 0
				pPlot = CyMap().plot(iiX,iiY)
				if pPlot.getNumUnits() == 0:
					if (pPlot.isWater() == plot.isWater() and pPlot.isPeak() == False and pPlot.isCity() == False):
						iCurrentPlot = iCurrentPlot + 5
				if iCurrentPlot >= 1:
					iCurrentPlot = iCurrentPlot + CyGame().getSorenRandNum(5, "findClearPlot")
					if iCurrentPlot >= iBestPlot:
						BestPlot = pPlot
						iBestPlot = iCurrentPlot
		return BestPlot
	iX = pUnit.getX()
	iY = pUnit.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			iCurrentPlot = 0
			pPlot = CyMap().plot(iiX,iiY)
			if pPlot.getNumUnits() == 0:
				if pUnit.canMoveOrAttackInto(pPlot, False):
					iCurrentPlot = iCurrentPlot + 5
			for i in range(pPlot.getNumUnits()):
				if pPlot.getUnit(i).getOwner() == pUnit.getOwner():
					if pUnit.canMoveOrAttackInto(pPlot, False):
						iCurrentPlot = iCurrentPlot + 15
			if pPlot.isCity():
				if pPlot.getPlotCity().getOwner() == pUnit.getOwner():
					iCurrentPlot = iCurrentPlot + 50
			if (iX == iiX and iY == iiY):
				iCurrentPlot = 0
			if iCurrentPlot >= 1:
				iCurrentPlot = iCurrentPlot + CyGame().getSorenRandNum(5, "findClearPlot")
				if iCurrentPlot >= iBestPlot:
					BestPlot = pPlot
					iBestPlot = iCurrentPlot
	return BestPlot

def postCombatConsumePaladin(pCaster, pOpponent):
	if (pOpponent.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_PALADIN')):
		pCaster.setDamage(0, pCaster.getOwner())
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CONSUME_PALADIN", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,4,12',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)

def postCombatExplode(pCaster, pOpponent):
	iX = pCaster.getX()
	iY = pCaster.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			if not (iiX == iX and iiY == iY):
				pPlot = CyMap().plot(iiX,iiY)
				if pPlot.isNone() == False:
					if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_NEW')):
						if CyGame().getSorenRandNum(100, "Flames Spread") <= gc.getDefineINT('FLAMES_SPREAD_CHANCE'):
							bValid = True
							iImprovement = pPlot.getImprovementType()
							if iImprovement != -1 :
								if gc.getImprovementInfo(iImprovement).isPermanent() :
									bValid = False
							if bValid:
								pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_SMOKE'))
					for i in range(pPlot.getNumUnits()):
						pUnit = pPlot.getUnit(i)
						pUnit.doDamage(10, 100, pCaster, gc.getInfoTypeForString('DAMAGE_FIRE'), false)

def postCombatHeal50(pCaster, pOpponent):
	if pCaster.getDamage() > 0:
		pCaster.setDamage(pCaster.getDamage() / 2, pCaster.getOwner())

#added Fix from Denev
def postCombatIra(pCaster, pOpponent):
	if pOpponent.isAlive():
		if pCaster.baseCombatStr() < 32:
			pCaster.setBaseCombatStr(pCaster.baseCombatStr() - pCaster.getTotalDamageTypeCombat() + 1)
			pCaster.setBaseCombatStrDefense(pCaster.baseCombatStrDefense() - pCaster.getTotalDamageTypeCombat() + 1)

def postCombatMimic(pCaster, pOpponent):
	promBlacklist = []
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_CHANNELING3'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_DIVINE'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_GREAT_COMMANDER'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_COMBAT1'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_COMBAT2'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_COMBAT3'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_SUMMON'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_HERO'))
	promBlacklist.append(gc.getInfoTypeForString('PROMOTION_LOCAL_LEGEND'))
	iHN = gc.getInfoTypeForString('PROMOTION_HIDDEN_NATIONALITY')
	listProms = []
	iCount = 0
	for iProm in range(gc.getNumPromotionInfos()):
		if pCaster.isHasPromotion(iProm):
			iCount += 1
		else:
			if (pOpponent.isHasPromotion(iProm)):
				if gc.getPromotionInfo(iProm).isEquipment() == False and gc.getPromotionInfo(iProm).getEquipmentCategory()==-1:
					if gc.getPromotionInfo(iProm).isClassPromotion() == False and gc.getPromotionInfo(iProm).isDiscipline() == False:
						if not iProm in promBlacklist:
							if not gc.getPromotionInfo(iProm).isRace():
								if ((iProm != iHN) or (pCaster.getGroup().getNumUnits()==1)):
									listProms.append(iProm)

	pPlayer = gc.getPlayer(pCaster.getOwner())
	if len(listProms) > 0:
		iCount += 1
		iRnd = CyGame().getSorenRandNum(len(listProms), "Mimic")
		pCaster.setHasPromotion(listProms[iRnd], True)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_PROMOTION_STOLEN", ()),'',1,gc.getPromotionInfo(listProms[iRnd]).getButton(),ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
		for iI in range(gc.getNumAdventureInfos()):
			if(pPlayer.isAdventureEnabled(iI) and not pPlayer.isAdventureFinished(iI)):
				for iJ in range(gc.getMAX_ADVENTURESTEPS()):
					if not gc.getAdventureInfo(iI).getAdventureStep(iJ)==-1:
						if(gc.getAdventureStepInfo(gc.getAdventureInfo(iI).getAdventureStep(iJ)).isMimicPromotion()):
							pPlayer.changeAdventureCounter(iI,iJ,1)

	if iCount >= 20:
		if pPlayer.isHuman():
			t = "TROPHY_FEAT_MIMIC_20"
			if not CyGame().isHasTrophy(t):
				CyGame().changeTrophyValue(t, 1)

def postCombatAcheron(pCaster, pOpponent):
	pOpponent.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DRAGONS_HORDE'),true)
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_DEFEATED_ACHERON"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

def postCombatArs(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_DEFEATED_ARS"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

def postCombatAuricAscendedLost(pCaster, pOpponent):
	iPlayer = pCaster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	for iTrait in range(gc.getNumTraitInfos()):
		if pPlayer.hasTrait(iTrait):
			pPlayer.setHasTrait(iTrait,False)
	if pOpponent.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GODSLAYER')):
		pOppPlayer = gc.getPlayer(pOpponent.getOwner())
		if pOppPlayer.isHuman():
			t = "TROPHY_FEAT_GODSLAYER"
			if not CyGame().isHasTrophy(t):
				CyGame().changeTrophyValue(t, 1)

def postCombatAuricAscendedWon(pCaster, pOpponent):
	if pOpponent.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GODSLAYER')):
		iPlayer = pCaster.getOwner()
		pPlayer = gc.getPlayer(iPlayer)
		for iTrait in range(gc.getNumTraitInfos()):
			if pPlayer.hasTrait(iTrait):
				pPlayer.setHasTrait(iTrait,False)
		pCaster.kill(True, pOpponent.getOwner())
		pOppPlayer = gc.getPlayer(pOpponent.getOwner())
		if pOppPlayer.isHuman():
			t = "TROPHY_FEAT_GODSLAYER"
			if not CyGame().isHasTrophy(t):
				CyGame().changeTrophyValue(t, 1)

def postCombatBasium(pCaster, pOpponent):
	if not pCaster.isImmortal():
		iPlayer = pCaster.getOwner()
		pPlayer = gc.getPlayer(iPlayer)
		for iTrait in range(gc.getNumTraitInfos()):
			if pPlayer.hasTrait(iTrait):
				pPlayer.setHasTrait(iTrait,False)
		pOppPlayer = gc.getPlayer(pOpponent.getOwner())
		if pOppPlayer.isHuman():
			t = "TROPHY_DEFEATED_BASIUM"
			if not CyGame().isHasTrophy(t):
				CyGame().changeTrophyValue(t, 1)

def postCombatBrigitHeld(pCaster, pOpponent):
	pPlot = pCaster.plot()
	pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MAGIC_IMMUNE'),false)	#added Sephi
	if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_RING_OF_CARCER'):
		pPlot.setImprovementType(-1)
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_FEAT_RESCUE_BRIGIT"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

def postCombatBuboes(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_DEFEATED_BUBOES"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

def postCombatHyborem(pCaster, pOpponent):
	if not pCaster.isImmortal():
		iPlayer = pCaster.getOwner()
		pPlayer = gc.getPlayer(iPlayer)
		for iTrait in range(gc.getNumTraitInfos()):
			if iTrait != gc.getInfoTypeForString('TRAIT_FALLOW'):
				if pPlayer.hasTrait(iTrait):
					pPlayer.setHasTrait(iTrait,False)
		pOppPlayer = gc.getPlayer(pOpponent.getOwner())
		if pOppPlayer.isHuman():
			t = "TROPHY_DEFEATED_HYBOREM"
			if not CyGame().isHasTrophy(t):
				CyGame().changeTrophyValue(t, 1)

def postCombatLeviathan(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_DEFEATED_LEVIATHAN"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

def postCombatOrthus(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_DEFEATED_ORTHUS"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

def postCombatStephanos(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_DEFEATED_STEPHANOS"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

def postCombatTreant(pCaster, pOpponent):
	pPlot = pCaster.plot()
	if pPlot.getFeatureType() == -1:
		if pPlot.canHaveFeature(gc.getInfoTypeForString('FEATURE_FOREST_NEW')):
			pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_FOREST_NEW'), 0)

def postCombatYersinia(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pOpponent.getOwner())
	if pPlayer.isHuman():
		t = "TROPHY_DEFEATED_YERSINIA"
		if not CyGame().isHasTrophy(t):
			CyGame().changeTrophyValue(t, 1)

#added fix from Denev
def postCombatReduceCombat1(pCaster, pOpponent):
	if pOpponent.isAlive():
		if pCaster.baseCombatStr() > pCaster.getTotalDamageTypeCombat():
			pCaster.setBaseCombatStr(pCaster.baseCombatStr() - pCaster.getTotalDamageTypeCombat() - 1)
			pCaster.setBaseCombatStrDefense(pCaster.baseCombatStrDefense() - pCaster.getTotalDamageTypeCombat() - 1)
			CyInterface().addMessage(pCaster.getOwner(), True, gc.getEVENT_MESSAGE_TIME(), CyTranslator().getText('TXT_KEY_MESSAGE_STRENGTH_REDUCED', (pCaster.getNameKey(), )), '', InterfaceMessageTypes.MESSAGE_TYPE_INFO, pCaster.getButton(), gc.getInfoTypeForString('COLOR_RED'), pCaster.getX(), pCaster.getY(), True, True)

def postCombatReducePopulation(pCaster, pOpponent):
	pPlot = pOpponent.plot()
	if pPlot.isCity():
		pCity = pPlot.getPlotCity()
		if pCity.getPopulation() > 1:
			pCity.changePopulation(-1)
			CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_POPULATION_REDUCED", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,1,4',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)

def postCombatLostSailorsDirge(pCaster, pOpponent):
	iPlayer = pOpponent.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_SAILORS_DIRGE_DEFEATED')
	triggerData = pPlayer.initTriggeredData(iEvent, true, -1, pCaster.getX(), pCaster.getY(), iPlayer, -1, -1, -1, -1, -1)

def postCombatSplit(pCaster, pOpponent):
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK_SPLIT')) == False:
		pPlayer = gc.getPlayer(pCaster.getOwner())
		iUnit = pCaster.getUnitType()
		newUnit = pPlayer.initUnit(iUnit, pCaster.getX(), pCaster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit2 = pPlayer.initUnit(iUnit, pCaster.getX(), pCaster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), False)
		newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), False)
		newUnit.setDamage(20, -1)
		newUnit2.setDamage(20, -1)
		if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG')) == False:
			newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK_SPLIT'), True)
			newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK_SPLIT'), True)
		newUnit.setDuration(pCaster.getDuration())
		newUnit2.setDuration(pCaster.getDuration())
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_SPLIT", ()),'',1,gc.getUnitInfo(iUnit).getButton(),ColorTypes(7),pCaster.getX(),pCaster.getY(),True,True)

def postCombatWolfRider(pCaster, pOpponent):
	if (pOpponent.getUnitType() == gc.getInfoTypeForString('UNIT_WOLF') or pOpponent.getUnitType() == gc.getInfoTypeForString('UNIT_WOLF_PACK')):
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_WOLF_RIDER", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas2.dds,1,10',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
		pPlayer = gc.getPlayer(pCaster.getOwner())
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WOLF_RIDER'), pCaster.getX(), pCaster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.convert(pCaster)

def reqBloodSacrifice(caster):
	pPlot = caster.plot()
	pVictim = -1
	iBloodMage = gc.getInfoTypeForString('PROMOTION_ADEPT_BLOOD_MAGE')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (caster.getOwner() == pUnit.getOwner() and pUnit.isAlive()):
			if not pUnit.isHasPromotion(iBloodMage):
				if pVictim == -1 or pVictim.getLevel() < pUnit.getLevel():
					pVictim = pUnit
	if pVictim != -1:
		return True
	return False
	
def spellBloodSacrifice(caster):
	pPlot = caster.plot()
	pVictim = -1
	iBloodMage = gc.getInfoTypeForString('PROMOTION_ADEPT_BLOOD_MAGE')
	iEmpoweredBlood = gc.getInfoTypeForString('PROMOTION_EMPOWERED_BY_BLOOD')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (caster.getOwner() == pUnit.getOwner() and pUnit.isAlive()):
			if not pUnit.isHasPromotion(iBloodMage):
				if pVictim == -1 or pVictim.getLevel() < pUnit.getLevel():
					pVictim = pUnit
				
	if pVictim != -1:
		iLevel = pVictim.getLevel()
		pVictim.kill(True, PlayerTypes.NO_PLAYER)
		caster.setHasPromotion(iEmpoweredBlood, True)
		caster.changeExperience(2 * iLevel, -1, False, False, False)

def spellBloodHeal(caster):
	iEmpoweredBlood = gc.getInfoTypeForString('PROMOTION_EMPOWERED_BY_BLOOD')
	if caster.isHasPromotion(iEmpoweredBlood):
		caster.setHasPromotion(iEmpoweredBlood, False)
		
def reqBodySnatch(caster):  #Xtended Gibbon
	if caster.isImmortal():
		return False
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iX = caster.getX()
	iY = caster.getY()
	eTeam = gc.getTeam(pPlayer.getTeam())
	iBestValue = (5 * caster.baseCombatStr()) + (3 * caster.getLevel())
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			iValue = 0
			for i in range(pPlot2.getNumUnits()):
				pUnit2 = pPlot2.getUnit(i)
				if not isWorldUnitClass(pUnit2.getUnitClassType()):
					if gc.getUnitInfo(pUnit2.getUnitType()).getEquipmentPromotion()==-1:		
						iValue += (5 * pUnit2.baseCombatStr())
						iValue += (3 * pUnit2.getLevel())
					if (iValue > iBestValue):
						iBestValue = iValue
						return True
	return False
	
def spellBodySnatch(caster): #Xtended Gibbon
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iX = caster.getX()
	iY = caster.getY()
	eTeam = gc.getTeam(pPlayer.getTeam())
	iBestValue = 0
	pCopyUnit = -1
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			iValue = 0
			for i in range(pPlot2.getNumUnits()):
				pUnit2 = pPlot2.getUnit(i)
				if not isWorldUnitClass(pUnit2.getUnitClassType()):
					if gc.getUnitInfo(pUnit2.getUnitType()).getEquipmentPromotion()==-1:		
						iValue += (5 * pUnit2.baseCombatStr())
						iValue += (3 * pUnit2.getLevel())
					if (iValue > iBestValue):
						iBestValue = iValue
						pCopyUnit = pUnit2
	if pCopyUnit != -1:
		newUnit = pPlayer.initUnit(pCopyUnit.getUnitType(), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
		for iProm in range(gc.getNumPromotionInfos()):
			if pCopyUnit.isHasPromotion(iProm):
				if not gc.getPromotionInfo(iProm).isEquipment():
					newUnit.setHasPromotion(iProm, true)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GIBBON'), true)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HERO'), true)
		newUnit.setLevel(caster.getLevel())
		newUnit.setExperience(caster.getExperience(), -1)
		newUnit.setDamage(0, caster.getOwner())
		newUnit.setName("Gibbon Goetia")

		#remove all promotions then kill old Gibbon
		for iProm in range(gc.getNumPromotionInfos()):
			if caster.isHasPromotion(iProm):
				caster.setHasPromotion(iProm, false)
		caster.kill(True, PlayerTypes.NO_PLAYER)

def spellPotionRejuvenate(caster):
	iPotion = gc.getInfoTypeForString('PROMOTION_POTION_REJUVENATION')
	if caster.isHasPromotion(iPotion):
		caster.setHasPromotion(iPotion, False)

def reqAddToFleshGolem(caster):
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_IMMORTAL')):
		return false
	if caster.isImmuneToMagic():
		return False
	return True

def spellAddToFleshGolem(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	eWeapon = gc.getInfoTypeForString('EQUIPMENTCATEGORY_WEAPON')
	eArmor = gc.getInfoTypeForString('EQUIPMENTCATEGORY_ARMOR')
	iChanneling = gc.getInfoTypeForString('PROMOTION_CHANNELING')
	iChanneling2 = gc.getInfoTypeForString('PROMOTION_CHANNELING2')
	iChanneling3 = gc.getInfoTypeForString('PROMOTION_CHANNELING3')
	iDivine = gc.getInfoTypeForString('PROMOTION_DIVINE')
	iFleshGolem = gc.getInfoTypeForString('UNITCLASS_FLESH_GOLEM')
	pFleshGolem = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (caster.getOwner() == pUnit.getOwner() and pUnit.getUnitClassType() == iFleshGolem):
			pFleshGolem = pUnit
	if pFleshGolem != -1:
		if caster.baseCombatStr() > pFleshGolem.baseCombatStr():
			pFleshGolem.setBaseCombatStr(pFleshGolem.baseCombatStr() + 1)
		if caster.baseCombatStrDefense() > pFleshGolem.baseCombatStrDefense():
			pFleshGolem.setBaseCombatStrDefense(pFleshGolem.baseCombatStrDefense() + 1)
		for iCount in range(gc.getNumPromotionInfos()):
			if (caster.isHasPromotion(iCount)):
				if (iCount != iChanneling and iCount != iChanneling2 and iCount != iChanneling3 and iCount != iDivine):
					if not gc.getPromotionInfo(iCount).isRace():
						if gc.getPromotionInfo(iCount).getBonusPrereq() == -1:
							if gc.getPromotionInfo(iCount).getPromotionPrereqAnd() != iChanneling2:
								if gc.getPromotionInfo(iCount).getPromotionPrereqAnd() != iChanneling3:
									if gc.getPromotionInfo(iCount).isEquipment() == False:
										if gc.getPromotionInfo(iCount).getEquipmentCategory() != eWeapon:
											if gc.getPromotionInfo(iCount).getEquipmentCategory() != eArmor:
												pFleshGolem.setHasPromotion(iCount, true)
		if pFleshGolem.baseCombatStr() >= 15:
			if pPlayer.isHuman():
				t = "TROPHY_FEAT_FLESH_GOLEM_15"
				if not CyGame().isHasTrophy(t):
					CyGame().changeTrophyValue(t, 1)

def reqAddToFreakShowHuman(caster):
	if caster.getRace() != -1:
		return False
	return True

def reqAddToWolfPack(caster):
	pPlot = caster.plot()
	iWolfPack = gc.getInfoTypeForString('UNIT_WOLF_PACK')
	iEmpower5 = gc.getInfoTypeForString('PROMOTION_EMPOWER5')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getUnitType() == iWolfPack:
			if pUnit.getOwner() == caster.getOwner():
				if not pUnit.isHasPromotion(iEmpower5):
					return True
	return False

def spellAddToWolfPack(caster):
	pPlot = caster.plot()
	iWolfPack = gc.getInfoTypeForString('UNIT_WOLF_PACK')
	iEmpower1 = gc.getInfoTypeForString('PROMOTION_EMPOWER1')
	iEmpower2 = gc.getInfoTypeForString('PROMOTION_EMPOWER2')
	iEmpower3 = gc.getInfoTypeForString('PROMOTION_EMPOWER3')
	iEmpower4 = gc.getInfoTypeForString('PROMOTION_EMPOWER4')
	iEmpower5 = gc.getInfoTypeForString('PROMOTION_EMPOWER5')
	bValid = True
	for i in range(pPlot.getNumUnits()):
		if bValid:
			pUnit = pPlot.getUnit(i)
			if pUnit.getUnitType() == iWolfPack:
				if pUnit.getOwner() == caster.getOwner():
					iProm = -1
					if not pUnit.isHasPromotion(iEmpower5):
						iProm = iEmpower5
					if not pUnit.isHasPromotion(iEmpower4):
						iProm = iEmpower4
					if not pUnit.isHasPromotion(iEmpower3):
						iProm = iEmpower3
					if not pUnit.isHasPromotion(iEmpower2):
						iProm = iEmpower2
					if not pUnit.isHasPromotion(iEmpower1):
						iProm = iEmpower1
					if iProm != -1:
						pUnit.setHasPromotion(iProm, True)
						bValid = False

def spellAnimalSoulVault(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iAnimalLevel = caster.getLevel()
	iStr = caster.baseCombatStr()
	iSoul = iStr + (5 * iAnimalLevel)
	iVault = pPlayer.getPurityCounterCache1()
	pPlayer.setPurityCounterCache1(iVault + iSoul)

def reqArcaneLacuna(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())
	manaTypes = [ 'BONUS_MANA_AIR','BONUS_MANA_CHAOS','BONUS_MANA_DEATH','BONUS_MANA_FIRE','BONUS_MANA_LIFE','BONUS_MANA_MIND','BONUS_MANA_NATURE']
	iCount = 0
	for szBonus in manaTypes:
		iBonus = gc.getInfoTypeForString(szBonus)
		iCount += CyMap().getNumBonuses(iBonus)
	if iCount == 0:
		return False
	if pPlayer.isHuman() == False:
		if eTeam.isHasTech(gc.getInfoTypeForString('TECH_SORCERY')) == False:
			return False
		if eTeam.getAtWarCount(True)==0:
			return False

	return True

def spellArcaneLacuna(caster):
	manaTypes = [ 'BONUS_MANA_AIR','BONUS_MANA_CHAOS','BONUS_MANA_DEATH','BONUS_MANA_FIRE','BONUS_MANA_LIFE','BONUS_MANA_MIND','BONUS_MANA_NATURE']
	iAdept = gc.getInfoTypeForString('UNITCOMBAT_ADEPT')
	iCount = 0
	pPlayer = gc.getPlayer(caster.getOwner())
	for szBonus in manaTypes:
		iBonus = gc.getInfoTypeForString(szBonus)
		iCount += CyMap().getNumBonuses(iBonus)
	py = PyPlayer(caster.getOwner())
	for pUnit in py.getUnitList():
		if pUnit.getUnitCombatType() == iAdept:
			pUnit.changeExperience(iCount*2, -1, False, False, False)
	iDelay = 20
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
		iDelay = 10
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
		iDelay = 30
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
		iDelay = 60
	for iPlayer2 in range(gc.getMAX_PLAYERS()):
		pPlayer2 = gc.getPlayer(iPlayer2)
		if pPlayer2.isAlive():
			if pPlayer2.getTeam() != pPlayer.getTeam():
				pPlayer2.changeDisableSpellcasting(iDelay)

def reqSilencedArcaneLacuna(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	caster.setHasCasted(True)
	if pPlayer.getDisableSpellcasting() == 0:
		CyMessageControl().sendModNetMessage(CvUtil.UnitPromotion,caster.getOwner(),caster.getID(),gc.getInfoTypeForString('PROMOTION_ARCANE_LACUNA'),0)
		#caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ARCANE_LACUNA'), False)

	return

def reqArdor(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getGreatPeopleCreated() == 0:
		return False
	if pPlayer.isHuman() == False:
		if pPlayer.getGreatPeopleCreated() < 5:
			return False
	return True

def spellArdor(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.setGreatPeopleCreated(0)
	pPlayer.setGreatPeopleThresholdModifier(0)

def reqArenaBattle(caster):
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_MELEE'):
		return True
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_RECON'):
		return True
	if caster.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_SLAVE'):
		return True
	return False

def spellArenaBattle(caster):
	pCity = caster.plot().getPlotCity()
	pCity.changeHappinessTimer(3)
	if CyGame().getSorenRandNum(100, "Arena Battle") < 50:
		caster.changeExperience(CyGame().getSorenRandNum(10, "Arena Battle") + 2, -1, False, False, False)
		caster.setDamage(25, caster.getOwner())
		CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_WIN", ()),'',1,', ,Art/Interface/Buttons/Buildings/Buildings_Atlas1.dds,6,2',ColorTypes(8),caster.getX(),caster.getY(),True,True)
		if caster.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_SLAVE'):
			pPlayer = gc.getPlayer(caster.getOwner())
			newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WARRIOR'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			newUnit.convert(caster)
	else:
		CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ARENA_LOSE", ()),'',1,', ,Art/Interface/Buttons/Buildings/Buildings_Atlas1.dds,6,2',ColorTypes(7),caster.getX(),caster.getY(),True,True)
		caster.kill(True, PlayerTypes.NO_PLAYER)

def reqCallBlizzard(caster):
	iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
	pPlot = caster.plot()
	if pPlot.getFeatureType() == iBlizzard:
		return False
	if pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT'):
		return False
	iX = caster.getX()
	iY = caster.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if pPlot.getFeatureType() == iBlizzard:
				return True
	return False

def spellCallBlizzard(caster):
	iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
	iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
	iX = caster.getX()
	iY = caster.getY()
	pBlizPlot = -1
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if pPlot.getFeatureType() == iBlizzard:
				pBlizPlot = pPlot
	if pBlizPlot != -1:
		pBlizPlot.setFeatureType(-1, -1)
	pPlot = caster.plot()
	pPlot.setFeatureType(iBlizzard, 0)
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_GRASS'):
		pPlot.setTerrainType(iTundra,True,True)
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_PLAINS'):
		pPlot.setTerrainType(iTundra,True,True)
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_DESERT'):
		pPlot.setTerrainType(iTundra,True,True)

def reqCallForm(caster):
	if caster.getSummoner() == -1:
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	pUnit = pPlayer.getUnit(caster.getSummoner())
	pPlot = caster.plot()
	if not pUnit.canMoveInto(pPlot, False, False, False):
		return False
	return True

def spellCallForm(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pUnit = pPlayer.getUnit(caster.getSummoner())
	pPlot = caster.plot()
	pUnit.setXY(pPlot.getX(), pPlot.getY(), false, true, true)

def reqCallOfTheGrave(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iX = caster.getX()
	iY = caster.getY()
	eTeam = gc.getTeam(pPlayer.getTeam())
	iGraveyard = gc.getInfoTypeForString('IMPROVEMENT_GRAVEYARD')
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				p2Player = gc.getPlayer(pUnit.getOwner())
				e2Team = p2Player.getTeam()
				if eTeam.isAtWar(e2Team) == True:
					return True
				if pUnit.getRace() == iUndead:
					if pUnit.getDamage() != 0:
						return True
				if pPlot.getImprovementType() == iGraveyard:
					return True
	return False

def spellCallOfTheGrave(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iX = caster.getX()
	iY = caster.getY()
	eTeam = gc.getTeam(pPlayer.getTeam())
	iGraveyard = gc.getInfoTypeForString('IMPROVEMENT_GRAVEYARD')
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			bValid = False
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				p2Player = gc.getPlayer(pUnit.getOwner())
				e2Team = p2Player.getTeam()
				if eTeam.isAtWar(e2Team) == True:
					pUnit.doDamage(40, 100, caster, gc.getInfoTypeForString('DAMAGE_UNHOLY'), true)
					bValid = True
				else:
					if pUnit.getRace() == iUndead:
						if pUnit.getDamage() != 0:
							pUnit.setDamage(0, caster.getOwner())
				if pPlot.getImprovementType() == iGraveyard:
					pPlot.setImprovementType(-1)
					pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WRAITH'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WRAITH'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WRAITH'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					bValid = True
			if bValid:
				CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SACRIFICE'),pPlot.getPoint())

#Xtended
def effectCharmCorlindale(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SILENCED'),True)
					CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_CHARM_PERSON'),pPlot.getPoint())
	
def reqCommanderJoin(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GREAT_COMMANDER')):
		return False
	iCommander = gc.getInfoTypeForString('UNITCLASS_COMMANDER')
	pCommander = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (caster.getOwner() == pUnit.getOwner() and pUnit.getUnitClassType() == iCommander):
			pCommander = pUnit
	if pCommander == -1:
		return False
	if pCommander.isHasCasted():
		return False
	if pPlayer.isHuman() == False:
		if caster.baseCombatStr() <= 5:
			return False
	return True

def spellCommanderJoin(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	iCommander = gc.getInfoTypeForString('UNITCLASS_COMMANDER')
	pCommander = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (caster.getOwner() == pUnit.getOwner() and pUnit.getUnitClassType() == iCommander):
			pCommander = pUnit
	if pCommander != -1:
		pCommander.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM'), True)
		pCommander.kill(False, PlayerTypes.NO_PLAYER)

def spellCommanderJoinDecius(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	iDecius = gc.getInfoTypeForString('UNIT_DECIUS')
	pCommander = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (caster.getOwner() == pUnit.getOwner() and pUnit.getUnitType() == iDecius):
			pCommander = pUnit
	if pCommander != -1:
		caster.setScenarioCounter(iDecius)
		pCommander.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM'), True)
		pCommander.kill(False, PlayerTypes.NO_PLAYER)

def spellCommanderSplit(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iCommander = gc.getInfoTypeForString('UNIT_COMMANDER')
	if caster.getScenarioCounter() == gc.getInfoTypeForString('UNIT_DECIUS'):
		iCommander = gc.getInfoTypeForString('UNIT_DECIUS')
		caster.setScenarioCounter(-1)
	newUnit = pPlayer.initUnit(iCommander, caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def reqConvertCityBasium(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	if pCity.getOwner() == caster.getOwner():
		return False
	return True

def spellConvertCityBasium(caster):
	pCity = caster.plot().getPlotCity()
	iPlayer = caster.getOwner()
	iPalace = gc.getInfoTypeForString('BUILDING_PALACE_MERCURIANS')
	iValue = 5 * gc.getGame().getElapsedGameTurns()
	pPlayer = gc.getPlayer(iPlayer)
	pPlayer.acquireCity(pCity,false,false)
	CyInterface().setDirty(InterfaceDirtyBits.CityInfo_DIRTY_BIT, True)
	pCity = caster.plot().getPlotCity()
	pCity.changeCulture(iPlayer, 5000, True)
	pCity.setNumRealBuilding(iPalace, 1)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_LUMBER, iValue)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_LEATHER, iValue)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_METAL, iValue)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_HERB, iValue)
	pPlayer.changeGlobalYield(YieldTypes.YIELD_STONE, iValue)

def reqConvertCityRantine(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ONE_CITY_CHALLENGE):
		return False
	if pCity.getOwner() == caster.getOwner():
		return False
	if pCity.getOwner() != gc.getBARBARIAN_PLAYER():
		return False
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getOwner() == gc.getBARBARIAN_PLAYER():
			if pUnit.baseCombatStr() > caster.baseCombatStr():
				return False
	return True

def spellConvertCityRantine(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.acquireCity(pCity,false,false)

def reqCreateBatteringRam(caster):
	pPlot = caster.plot()
	if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_NEW') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE')):
		return True
	return False
	
def spellCreateBatteringRam(caster):
	pPlot = caster.plot()
	pPlot.setFeatureType(-1, -1)

def effectCrownOfBrillance(caster):
	caster.cast(gc.getInfoTypeForString('SPELL_CROWN_OF_BRILLANCE'))
	
def reqCrownOfBrillance(caster):
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CROWN_OF_BRILLANCE')):
		return False
	return True

def spellCrownOfBrillance(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	iPurge = 1
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					if (pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON')) or pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_UNDEAD'))):
						iPurge = 2
					iRnd = CyGame().getSorenRandNum(10, "Junil") + caster.getLevel()
					pUnit.doDamage(iRnd*iPurge, 100, caster, gc.getInfoTypeForString('DAMAGE_HOLY'), true)
					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_VERDICT_HERO_DEBUFF'),True)
	CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_DESTROY_UNDEAD'),caster.plot().getPoint())
		
def reqCrush(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			bEnemy = false
			bNeutral = false
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					bEnemy = true
				else:
					bNeutral = true
			if (bEnemy and bNeutral == false):
				return true
	return false

def spellCrush(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	iBestValue = 0
	pBestPlot = -1
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			bNeutral = false
			iValue = 0
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					iValue = iValue + 10
				else:
					bNeutral = true
			if (iValue > iBestValue and bNeutral == false):
				iBestValue = iValue
				pBestPlot = pPlot
	if pBestPlot != -1:
		for i in range(pBestPlot.getNumUnits()):
			pUnit = pBestPlot.getUnit(i)
			pUnit.doDamage(50, 75, caster, gc.getInfoTypeForString('DAMAGE_PHYSICAL'), true)
		CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_CRUSH'),pBestPlot.getPoint())

def reqDestroyUndead(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = pPlayer.getTeam()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.getRace() == gc.getInfoTypeForString('PROMOTION_UNDEAD'):
					if pPlayer.isHuman():
						return True
					p2Player = gc.getPlayer(pUnit.getOwner())
					e2Team = gc.getTeam(p2Player.getTeam())
					if e2Team.isAtWar(eTeam):
						return True
	return False

def spellDestroyUndead(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.getRace() == gc.getInfoTypeForString('PROMOTION_UNDEAD'):
					pUnit.doDamage(30, 100, caster, gc.getInfoTypeForString('DAMAGE_HOLY'), true)

def reqUnboundNode(caster):
	pPlot = caster.plot()
	if not pPlot.getBonusType(-1) == -1:
		if gc.getBonusInfo(pPlot.getBonusType(-1)).getBonusClassType() == gc.getInfoTypeForString('BONUSCLASS_MANA'):
			return True
	return False

def spellUnboundNode(caster):
	pPlot = caster.plot()
	if pPlot.getBonusType(-1) != -1:
		if gc.getBonusInfo(pPlot.getBonusType(-1)).getBonusClassType() == gc.getInfoTypeForString('BONUSCLASS_MANA'):
			if pPlot.getImprovementType() == -1:
				pPlot.setBonusType(gc.getInfoTypeForString('BONUS_MANA'))
			else:
				if gc.getImprovementInfo(pPlot.getImprovementType()).isPermanent() == False:
					pPlot.setBonusType(gc.getInfoTypeForString('BONUS_MANA'))
					pPlot.setImprovementType(-1)

def reqDispelMagic(caster):
	promDebuff = []
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_BILLOWING_GALES'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_BROKEN_MIND'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CHARMED_UNDEAD'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CHILLED_UNIT'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CONFUSED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CURSED_AURA'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CURSED_ANGEL'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DARKNESS'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DEMON_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DESPERATION'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DEVIL_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_FRENZY'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_HAND_OF_THE_REAPER'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_IMPAIRED_SIGHT'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_MESMERIZED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_SILENCED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_SLOW'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_SPELL_ON_FIRE'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_STUNNED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_UNDEAD_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_WHISPERING_PAIN'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_WINTERED'))

	promBuff = []
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ACCELERATED'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_AIR_SHIELD'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ARCANE_SHIELD'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ARCANE_WEAPON'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_BEAR'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_CHEETAH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_EAGLE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_SNAKE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_BLACK_PRAYER'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_BLESSED'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_BRIGHT_VISION'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_CHANNELING_PUPPET'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_CHARGED_WEAPON'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_COURAGE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_COURAGE_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_CROWN_OF_BRILLANCE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_DEATH_KISS'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_DEFILING'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_DRAW_STRENGTH_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_EARTH_LORE_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ELEMENTAL_WEAPONS'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ENCHANTMENT_UNHOLY_ADDICTION'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ESSENCE_OF_DEATH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FAIR_WINDS'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FIRE_FIST_GOLEM'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FOREST_GUIDE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FOL_SENTINEL'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_HASTED'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_HIDDEN_GOLEM'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_INFUSED_LIFE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_INVISIBILITY_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_MARK_OF_SERVITUDE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_SHIELD_OF_FAITH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_SPIRIT_GUIDE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_UNDEAD_STRONG_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_UNHOLY_STRENGTH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_WIND_SHIELD'))
	
	bCanDispel = False
	
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())

	iX = caster.getX()
	iY = caster.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for iUnit in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(iUnit)
				p2Player = gc.getPlayer(pUnit.getOwner())
				e2Team = p2Player.getTeam()
				
				if pUnit.getTeam() == caster.getTeam():
					for iProm in range(gc.getNumPromotionInfos()):
						if iProm in promDebuff:
							bCanDispel = True
				
				elif eTeam.isAtWar(e2Team) == True:
					for iProm in range(gc.getNumPromotionInfos()):
						if iProm in promBuff:
							bCanDispel = True
	if bCanDispel == True:
		return True
	
	return False
							
#	if caster.canDispel(gc.getInfoTypeForString('SPELL_DISPEL_MAGIC')):
#		return True

def spellDispelMagic(caster):
	promDebuff = []
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_BILLOWING_GALES'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_BROKEN_MIND'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CHARMED_UNDEAD'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CHILLED_UNIT'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CONFUSED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CURSED_AURA'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_CURSED_ANGEL'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DARKNESS'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DEMON_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DESPERATION'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_DEVIL_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_FRENZY'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_HAND_OF_THE_REAPER'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_IMPAIRED_SIGHT'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_MESMERIZED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_SILENCED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_SLOW'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_SPELL_ON_FIRE'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_STUNNED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_UNDEAD_CHARMED'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_WHISPERING_PAIN'))
	promDebuff.append(gc.getInfoTypeForString('PROMOTION_WINTERED'))

	promBuff = []
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ACCELERATED'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_AIR_SHIELD'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ARCANE_SHIELD'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ARCANE_WEAPON'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_BEAR'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_CHEETAH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_EAGLE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ASPECT_OF_SNAKE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_BLACK_PRAYER'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_BLESSED'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_BRIGHT_VISION'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_CHANNELING_PUPPET'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_CHARGED_WEAPON'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_COURAGE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_COURAGE_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_CROWN_OF_BRILLANCE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_DEATH_KISS'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_DEFILING'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_DRAW_STRENGTH_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_EARTH_LORE_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ELEMENTAL_WEAPONS'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ENCHANTMENT_UNHOLY_ADDICTION'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_ESSENCE_OF_DEATH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FAIR_WINDS'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FIRE_FIST_GOLEM'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FOREST_GUIDE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_FOL_SENTINEL'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_HASTED'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_HIDDEN_GOLEM'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_INFUSED_LIFE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_INVISIBILITY_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_MARK_OF_SERVITUDE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_SHIELD_OF_FAITH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_SPIRIT_GUIDE'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_UNDEAD_STRONG_AURA'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_UNHOLY_STRENGTH'))
	promBuff.append(gc.getInfoTypeForString('PROMOTION_WIND_SHIELD'))
	
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())

	iX = caster.getX()
	iY = caster.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for iUnit in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(iUnit)
				p2Player = gc.getPlayer(pUnit.getOwner())
				e2Team = p2Player.getTeam()
				
				if pUnit.getTeam() == caster.getTeam():
					for iProm in range(gc.getNumPromotionInfos()):
						if iProm in promDebuff:
							pUnit.setHasPromotion(iProm, False)
				
				elif eTeam.isAtWar(e2Team) == True:
					for iProm in range(gc.getNumPromotionInfos()):
						if iProm in promBuff:
							pUnit.setHasPromotion(iProm, False)
						
def reqDisrupt(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	if caster.getTeam() == pCity.getTeam():
		return False
	if pCity.getCultureLevel() > 1:
		return False
	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ONE_CITY_CHALLENGE):
		return False
	return True

def spellDisrupt(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iPlayer2 = pCity.getOwner()
	pPlayer2 = gc.getPlayer(iPlayer2)
	CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISRUPT_ENEMY",()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,4,3',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
	CyInterface().addMessage(iPlayer2,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISRUPT",()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,4,3',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
	pCity.changeHurryAngerTimer(2)

	pPlayer.acquireCity(pCity,false,false)
	pPlayer2.AI_changeMemoryCount(iPlayer, MemoryTypes.MEMORY_MAGIC_CHARM, -300)
	iTolerance = -1000 + gc.getGame().getHandicapType() * 100
	if iTolerance > -100:
		iTolerance = -100

	if pPlayer2.AI_getMemoryCount(iPlayer, MemoryTypes.MEMORY_MAGIC_CHARM) < iTolerance:
		if gc.getTeam(pPlayer2.getTeam()).canDeclareWar(pPlayer.getTeam()):
			gc.getTeam(pPlayer2.getTeam()).declareWar(pPlayer.getTeam(), false, -1)


def reqDivineRetribution(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		iTeam = gc.getPlayer(caster.getOwner()).getTeam()
		eTeam = gc.getTeam(iTeam)
		if eTeam.getAtWarCount(True) < 2:
			return False
	return True

def spellDivineRetribution(caster):
	iDemon = gc.getInfoTypeForString('PROMOTION_DEMON')
	iDevil = gc.getInfoTypeForString('PROMOTION_DEVIL')
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	for iPlayer in range(gc.getMAX_PLAYERS()):
		player = gc.getPlayer(iPlayer)
		if player.isAlive():
			py = PyPlayer(iPlayer)
			for pUnit in py.getUnitList():
				if (pUnit.getRace() == iDemon or pUnit.getRace() == iUndead or pUnit.getRace() == iDevil):
					pUnit.doDamage(50, 100, caster, gc.getInfoTypeForString('DAMAGE_HOLY'), false)

def reqDomination(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iResistMax = 95
	if pPlayer.isHuman() == false:
		iResistMax = 20
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.isAlive():
					if not pUnit.isDelayedDeath():
						if eTeam.isAtWar(pUnit.getTeam()):
							iResist = pUnit.getResistChance(caster, gc.getInfoTypeForString('SPELL_DOMINATION'))
							if iResist <= iResistMax:
								return true
	return false

def spellDomination(caster):
	iSpell = gc.getInfoTypeForString('SPELL_DOMINATION')
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iResistMax = 95
	iBestValue = 0
	pBestUnit = -1
	if pPlayer.isHuman() == false:
		iResistMax = 20
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				iValue = 0
				if pUnit.isAlive():
					if pUnit.isDelayedDeath() == False:
						if eTeam.isAtWar(pUnit.getTeam()):
							iResist = pUnit.getResistChance(caster, iSpell)
							if iResist <= iResistMax:
								iValue = pUnit.baseCombatStr() * 10
								iValue = iValue + (100 - iResist)
								if iValue > iBestValue:
									iBestValue = iValue
									pBestUnit = pUnit
	if pBestUnit != -1:
		pPlot = caster.plot()
		if pBestUnit.isResisted(caster, iSpell) == false:
			CyInterface().addMessage(pBestUnit.getOwner(),true,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOMINATION", ()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,6,3',ColorTypes(7),pBestUnit.getX(),pBestUnit.getY(),True,True)
			CyInterface().addMessage(caster.getOwner(),true,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOMINATION_ENEMY", ()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,6,3',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			newUnit = pPlayer.initUnit(pBestUnit.getUnitType(), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			newUnit.convert(pBestUnit)
			newUnit.changeImmobileTimer(1)
		else:
			CyInterface().addMessage(caster.getOwner(),true,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOMINATION_FAILED", ()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,6,3',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MIND3'), False)

def reqDrown(caster):
	if caster.getUnitCombatType() == -1:
		return False
	elif caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ANIMAL'):
		return False
	elif caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_SIEGE'):
		return False
		
	return True
	
			
def reqEarthquake(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		eTeam = pPlayer.getTeam()
		for iiX in range(iX-1, iX+2, 1):
			for iiY in range(iY-1, iY+2, 1):
				pPlot = CyMap().plot(iiX,iiY)
				p2Player = gc.getPlayer(pPlot.getOwner())
				if pPlot.isOwned():
					e2Team = gc.getTeam(p2Player.getTeam())
					if e2Team.isAtWar(eTeam) == False:
						return False
	return True

def spellEarthquake(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = pPlayer.getTeam()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				if (pPlot.isCity() or pPlot.getImprovementType() != -1):
					if pPlot.isOwned():
						cf.startWar(caster.getOwner(), pPlot.getOwner(), WarPlanTypes.WARPLAN_TOTAL)
				if pPlot.isCity():
					pCity = pPlot.getPlotCity()
					for i in range(gc.getNumBuildingInfos()):
						iRnd = CyGame().getSorenRandNum(100, "Bob")
						if (gc.getBuildingInfo(i).getConquestProbability() != 100 and iRnd <= 25):
							pCity.setNumRealBuilding(i, 0)
				for iUnit in range(pPlot.getNumUnits()):
					pUnit = pPlot.getUnit(iUnit)
					iLevel = pUnit.getLevel()
					iRnd = CyGame().getSorenRandNum(100/iLevel, "Bob")
					if pUnit.isFlying() == False:
						pUnit.setFortifyTurns(0)
						pUnit.doDamage(iRnd, 100, caster, gc.getInfoTypeForString('DAMAGE_PHYSICAL'), false)
				iRnd = CyGame().getSorenRandNum(100, "Bob")
				if iRnd <= 25:
					iImprovement = pPlot.getImprovementType()
					if iImprovement != -1:
						if gc.getImprovementInfo(iImprovement).isPermanent() == False:
							pPlot.setImprovementType(-1)

def spellEnterPortal(caster):
	pPlot = caster.plot()
	iX = pPlot.getPortalExitX()
	iY = pPlot.getPortalExitY()
	pExitPlot = CyMap().plot(iX,iY)
	if not pPlot.isNone():
		caster.setXY(iX, iY, false, true, true)

def spellEntertain(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	iPlayer = caster.getOwner()
	iPlayer2 = pCity.getOwner()
	if iPlayer != iPlayer2:
		pPlayer = gc.getPlayer(iPlayer)
		pPlayer2 = gc.getPlayer(iPlayer2)
		iGold = (pCity.getPopulation() / 2) + 1
		pPlayer.changeGold(iGold)
		szBuffer = CyTranslator().getText("TXT_KEY_MESSAGE_ENTERTAIN_GOOD", (iGold, ))
		CyInterface().addMessage(iPlayer,true,25,szBuffer,'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,1,4',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
		iGold = iGold * -1
		pPlayer2.changeGold(iGold)
		szBuffer = CyTranslator().getText("TXT_KEY_MESSAGE_ENTERTAIN_BAD", (iGold, ))
		CyInterface().addMessage(iPlayer2,true,25,szBuffer,'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,1,4',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
	pCity.changeHappinessTimer(2)

def reqEscape(caster):
	if caster.getOwner() == gc.getBARBARIAN_PLAYER():
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
#		if caster.getDamage() >= 50:
		return False
	return True

def reqExploreLair(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if caster.plot().getImprovementType()!=-1:
			if gc.getImprovementInfo(caster.plot().getImprovementType()).isUnique():
				if CyGame().getGameTurn()<200:
					return False
				if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_ELOHIM'):
					return False

	if caster.plot().getOwner() != -1:
		if caster.getOwner() != caster.plot().getOwner():
			return False
	if caster.isOnlyDefensive():
		return False
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_SIEGE'):
		return False
	if caster.isBarbarian():
		return False
#	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SUMMON')):
#		return False
	if caster.getSpecialUnitType() == gc.getInfoTypeForString('SPECIALUNIT_SPELL'):
		return False
	if caster.getSpecialUnitType() == gc.getInfoTypeForString('SPECIALUNIT_BIRD'):
		return False
	pPlot = caster.plot()
#added Sephi
	if pPlot.getImprovementType() != gc.getInfoTypeForString('IMPROVEMENT_DUNGEON') and pPlot.getImprovementType() != gc.getInfoTypeForString('IMPROVEMENT_SHIP_WRECK'):
		if pPlot.getDungeonType() == -1:
			return False
	if pPlot.getImprovementType() == -1:
		return False
#	if gc.getImprovementInfo(pPlot.getImprovementType()).getSpawnUnitCiv()!=gc.getInfoTypeForString('CIVILIZATION_BARBARIAN'):
#		return True
	if not pPlot.isWater():
		pPlayer = gc.getPlayer(caster.getOwner())
		iTeam = pPlayer.getTeam()
		eTeam = gc.getTeam(iTeam)
		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		if not eTeam.isAtWar(bPlayer.getTeam()) and caster.getUnitType() != gc.getInfoTypeForString('UNIT_RANTINE'):
			return False
	return True

def spellExploreLair(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())

	if pPlot.getImprovementType()!=gc.getInfoTypeForString('IMPROVEMENT_DUNGEON') and pPlot.getImprovementType()!=gc.getInfoTypeForString('IMPROVEMENT_SHIP_WRECK'):
		caster.exploreLair()
		
		#Austrin
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_WANDERER')):
			pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_FORTIFIED_DUNGEON'))

		
		return

	iRnd = CyGame().getSorenRandNum(100, "Explore Lair") + caster.getLevel()
	iDestroyLair = 0
	
	bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	eTeam = gc.getTeam(pPlayer.getTeam())

	# Rantine pillages for gold only
#	if eTeam.isAtWar(bPlayer.getTeam()) == False:
#		pPlayer.changeGold(50)
#		pPlot.setImprovementType(-1)
#		caster.finishMoves()
#		caster.changeExperience(10, -1, false, false, false)
#		return

	
	#Grigori
	if caster.getUnitType()== gc.getInfoTypeForString('UNIT_TREASURE_HUNTER'):
		iRnd +=14

	#Austrin
	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_WANDERER')):
		if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_RECON'):
			iRnd +=14

	if iRnd < 14:
		iDestroyLair = cf.exploreLairBigBad(caster)
	if iRnd >= 14 and iRnd < 44:
		iDestroyLair = cf.exploreLairBad(caster)
	if iRnd >= 44 and iRnd < 74:
		iDestroyLair = cf.exploreLairNeutral(caster)
	if iRnd >= 74 and iRnd < 94:
		iDestroyLair = cf.exploreLairGood(caster)
	if iRnd >= 94:
		iDestroyLair = cf.exploreLairBigGood(caster)
		
	if iDestroyLair > CyGame().getSorenRandNum(100, "Explore Lair"):
		CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_LAIR_DESTROYED", ()),'AS2D_POSITIVE_DINK',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		pPlot.setImprovementType(-1)
		
		#Austrin
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_WANDERER')):
			pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_FORTIFIED_DUNGEON'))

	caster.finishMoves()
	caster.changeExperience(5, -1, false, false, false)
	

def spellExploreLairEpic(caster):
	pPlot = caster.plot()
	iRnd = CyGame().getSorenRandNum(100, "Explore Lair") + caster.getLevel()
	iDestroyLair = 0
	
	#Xtended
	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_WANDERER')):
		if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_RECON'):
			iRnd +=44
		
	if iRnd < 54:
		iDestroyLair = cf.exploreLairBigBad(caster)
	if iRnd >= 54:
		iDestroyLair = cf.exploreLairBigGood(caster)
	if iDestroyLair > CyGame().getSorenRandNum(100, "Explore Lair"):
		CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_LAIR_DESTROYED", ()),'AS2D_POSITIVE_DINK',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		pPlot.setImprovementType(-1)
	caster.finishMoves()
	caster.changeExperience(10, -1, false, false, false)

def reqFeast(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	if pCity.getPopulation() < 3:
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == false:
		if pCity.getPopulation()<15:
			return False
		if caster.getUnitAIType()!=(gc.getInfoTypeForString('UNITAI_FEASTING')):
			if pCity.unhappyLevel(0) > pCity.happyLevel():
				return False
	return True

def spellFeast(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	caster.changeExperience(pCity.getPopulation()-2, -1, false, false, false)
	pCity.changeHurryAngerTimer(3)

def reqFeed(caster):
	if caster.getDamage() == 0:
		return false
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == false:
		if caster.getDamage() < 20:
			return false
	return true

def spellFeed(caster):
	pVictim = -1
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getOwner() == caster.getOwner():
##--------		Unofficial Bug Fix: Modified by Denev	--------##
#			if pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_BLOODPET'):
			if pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_BLOODPET') and not pUnit.isDelayedDeath():
##--------		Unofficial Bug Fix: End Modify			--------##
				if pVictim == -1 or pVictim.getLevel() > pUnit.getLevel():
					pVictim = pUnit
	if pVictim != -1:
##--------		Unofficial Bug Fix: Modified by Denev	--------##
#		pVictim.kill(True, 0)
		pVictim.kill(True, PlayerTypes.NO_PLAYER)
		caster.setDamage(caster.getDamage() - 20, caster.getOwner())
		caster.setMadeAttack(false)
##--------		Unofficial Bug Fix: End Modify			--------##

def reqForTheHorde(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	eTeam = gc.getTeam(pPlayer.getTeam())
	if eTeam.isAtWar(bPlayer.getTeam()):
		return False
	if bPlayer.getNumUnits() == 0:
		return False
	if pPlayer.isHuman() == False:
		if bPlayer.getNumUnits() < 60:
			return False
		if pPlayer.isConquestMode() == False:
			return False
	return True

def spellForTheHorde(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iHero = gc.getInfoTypeForString('PROMOTION_HERO')
	iOrc = gc.getInfoTypeForString('PROMOTION_ORC')
	iGoblin = gc.getInfoTypeForString('PROMOTION_GOBLIN')
	py = PyPlayer(gc.getBARBARIAN_PLAYER())

	iCount=0
	for pUnit in py.getUnitList():
		if (pUnit.getUnitAIType()!=gc.getInfoTypeForString('UNITAI_MANADEFENDER')):
			if ((pUnit.getRace() == iOrc or pUnit.getRace() == iGoblin) and pUnit.isHasPromotion(iHero) == False):
#Sephi
				iChance=50
				XDif=pUnit.getX()-pPlayer.getCapitalCity().getX()
				XDif*=XDif
				YDif=pUnit.getY()-pPlayer.getCapitalCity().getY()
				YDif*=YDif

				if XDif>1600:
					XDif=1600
				XDif/16
				if YDif>1600:
					YDif=1600
				YDif/16
				iChance*=(XDif*YDif)
				iChance/=10000

				if (CyGame().getSorenRandNum(100, "Bob") > iChance):

					if not pUnit.plot().isVisibleEnemyDefender(caster):
						pPlot = pUnit.plot()
						for i in range(pPlot.getNumUnits(), -1, -1):
							pNewPlot = -1
							pLoopUnit = pPlot.getUnit(i)
							if pLoopUnit.isHiddenNationality():
								pNewPlot = cf.findClearPlot(pLoopUnit, -1)
								if pNewPlot != -1:
									pLoopUnit.setXY(pNewPlot.getX(), pNewPlot.getY(), false, true, true)

						newUnit = pPlayer.initUnit(pUnit.getUnitType(), pUnit.getX(), pUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						newUnit.convert(pUnit)
						if (newUnit.area()==pPlayer.getCapitalCity().area()):
							newUnit.startConquestMode()
						else:
							newUnit.setUnitAIType(gc.getInfoTypeForString('UNITAI_EXPLORE'))
						iCount+=1
						if iCount>20:
							break

def reqForestFury(caster):
	pPlot = caster.plot()
	if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE')):
		return True
	return False
	
def reqFormWolfPack(caster):
	pPlot = caster.plot()
	iWolf = gc.getInfoTypeForString('UNIT_WOLF')
	iCount = 0

#Added Sephi
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if pPlayer.isConquestMode() == False:
			return False

	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getUnitType() == iWolf:
			if pUnit.getOwner() == caster.getOwner():
				iCount += 1
	if iCount < 2:
		return False
	return True

def spellFormWolfPack(caster):
	pPlot = caster.plot()
	iWolf = gc.getInfoTypeForString('UNIT_WOLF')
	pWolf2 = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getUnitType() == iWolf:
			if pUnit.getOwner() == caster.getOwner():
				if pUnit!=caster:
					pWolf2 = pUnit
	if pWolf2 != -1 and not pWolf2.isDelayedDeath():
		pPlayer = gc.getPlayer(caster.getOwner())
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WOLF_PACK'), caster.getX(), caster.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setExperience(caster.getExperience() + pWolf2.getExperience(), -1)
		caster.kill(True, PlayerTypes.NO_PLAYER)
		pWolf2.kill(True, PlayerTypes.NO_PLAYER)


def reqGiftsOfNantosuelta(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getNumCities() == 0:
		return False
	if pPlayer.isHuman() == False:
		if pPlayer.getNumCities() < 5:
			return False
	return True

def spellGiftsOfNantosuelta(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iGoldenHammer = gc.getInfoTypeForString('EQUIPMENT_GOLDEN_HAMMER')
	for pyCity in PyPlayer(iPlayer).getCityList() :
		pCity = pyCity.GetCy()
		newUnit = pPlayer.initUnit(iGoldenHammer, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def reqGiftVampirism(caster):
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() and pUnit.getOwner() == caster.getOwner()):
			if not pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE')):
				if pUnit.getLevel() >= 6:
					return True
				if (pUnit.getLevel() >= 4 and pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_MOROI')):
					return True
	return False

def spellGiftVampirism(caster):
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() and pUnit.getOwner() == caster.getOwner()):
			if not pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE')):
				if pUnit.getLevel() >= 6:
					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE'),True)
				if (pUnit.getLevel() >= 4 and pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_MOROI')):
					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE'),True)

def spellGiveHammerToCraftsman(caster):
	pCity = caster.plot().getPlotCity()
	pCity.changeFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_ENGINEER'), 1)

def reqHastursRazor(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = pPlayer.getTeam()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.getDamage() > 0:
					if pPlayer.isHuman():
						return True
					if pUnit.getOwner() == caster.getOwner():
						return True
	return False

def spellHastursRazor(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	listDamage = []
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				listDamage.append(pUnit.getDamage())
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				iRnd = listDamage[CyGame().getSorenRandNum(len(listDamage), "Hasturs Razor")]
				if iRnd != pUnit.getDamage():
					CyInterface().addMessage(pUnit.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_HASTURS_RAZOR",()),'AS2D_CHARM_PERSON',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,4,5',ColorTypes(7),pUnit.getX(),pUnit.getY(),True,True)
					if pUnit.getOwner() != caster.getOwner():
						CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_HASTURS_RAZOR",()),'AS2D_CHARM_PERSON',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,4,5',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)
					pUnit.setDamage(iRnd, caster.getOwner())

def reqHeal(caster):
	pPlot = caster.plot()
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iCrippled = gc.getInfoTypeForString('PROMOTION_CRIPPLED')
	iDiseased = gc.getInfoTypeForString('PROMOTION_DISEASED')
	iPlagued = gc.getInfoTypeForString('PROMOTION_PLAGUED')
	iPoisoned = gc.getInfoTypeForString('PROMOTION_POISONED')
	iWithered = gc.getInfoTypeForString('PROMOTION_WITHERED')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if not pUnit.isHasPromotion(iUndead):
			if (pUnit.getDamage() > 0):
				return True
			if pUnit.isHasPromotion(iCrippled):
				return True
			if pUnit.isHasPromotion(iDiseased):
				return True
			if pUnit.isHasPromotion(iPlagued):
				return True
			if pUnit.isHasPromotion(iPoisoned):
				return True
			if pUnit.isHasPromotion(iWithered):
				return True
	return False

def spellHeal(caster,amount):
	pPlot = caster.plot()
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iCrippled = gc.getInfoTypeForString('PROMOTION_CRIPPLED')
	iDiseased = gc.getInfoTypeForString('PROMOTION_DISEASED')
	iPlagued = gc.getInfoTypeForString('PROMOTION_PLAGUED')
	iPoisoned = gc.getInfoTypeForString('PROMOTION_POISONED')
	iWithered = gc.getInfoTypeForString('PROMOTION_WITHERED')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if not pUnit.isHasPromotion(iUndead):
			pUnit.changeDamage(-amount,0) #player doesn't matter - it won't kill
			pUnit.setHasPromotion(iCrippled,False)
			pUnit.setHasPromotion(iDiseased,False)
			pUnit.setHasPromotion(iPlagued,False)
			pUnit.setHasPromotion(iPoisoned,False)
			pUnit.setHasPromotion(iWithered,False)

def reqHealingSalve(caster):
	if not caster.isAlive():
		return False
	if caster.getDamage() == 0:
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if caster.getDamage() < 25:
			return False
	return True

def spellHealingSalve(caster, amount):
	if caster.isAlive():
		caster.changeDamage(-amount,0) #player doesn't matter - it won't kill
#		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DISEASED'), false)
#		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), false)

def reqHellfire(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
		return False
	pPlot = caster.plot()
	if pPlot.isCity():
		return False
	if pPlot.isWater():
		return False
	if pPlot.getImprovementType() != -1:
		return False
	iHellFire = gc.getInfoTypeForString('IMPROVEMENT_HELLFIRE')
	iX = pPlot.getX()
	iY = pPlot.getY()
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if pPlot2.getImprovementType() == iHellFire:
				return False
	if pPlayer.isHuman() == False:
		if pPlot.isOwned():
			if pPlot.getOwner() == caster.getOwner():
				return False
	return True

def reqHeraldsCall(caster):
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() and pUnit.getOwner() == caster.getOwner()):
			return true
	return False

def spellHeraldsCall(caster):
	iValor = gc.getInfoTypeForString('PROMOTION_VALOR')
	iBlessed = gc.getInfoTypeForString('PROMOTION_BLESSED')
	iCourage = gc.getInfoTypeForString('PROMOTION_COURAGE')
	iLoyalty = gc.getInfoTypeForString('PROMOTION_LOYALTY')
	iSpirit = gc.getInfoTypeForString('PROMOTION_SPIRIT_GUIDE')
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() and pUnit.getOwner() == caster.getOwner()):
			pUnit.setHasPromotion(iValor,True)
			pUnit.setHasPromotion(iBlessed, True)
			pUnit.setHasPromotion(iCourage, True)
			pUnit.setHasPromotion(iLoyalty, True)
			pUnit.setHasPromotion(iSpirit, True)
			pUnit.setDuration(1)

def reqHide(caster):
	if caster.isMadeAttack():
		return False
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HIDDEN')):
		return False
	return True

def reqHireScorpionClan(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	eTeam = gc.getTeam(pPlayer.getTeam())
	if not pPlayer.canSupportMoreUnits():
		return False
	if eTeam.isAtWar(bPlayer.getTeam()):
		return False
	return True

def spellHireArcher(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER_SCORPION_CLAN'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def spellHireChariot(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.initUnit(gc.getInfoTypeForString('UNIT_CHARIOT_SCORPION_CLAN'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def spellHireGoblin(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GOBLIN_SCORPION_CLAN'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def spellHireUnits(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pCity = caster.plot().getPlotCity()
	iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_MAGNADINE_HIRE_UNITS')
	triggerData = pPlayer.initTriggeredData(iEvent, true, pCity.getID(), pCity.getX(), pCity.getY(), iPlayer, -1, -1, -1, -1, -1)

def spellHireWolfRider(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WOLF_RIDER_SCORPION_CLAN'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def reqHyboremsWhisper(caster):
	if gc.getGame().isNetworkMultiPlayer():
		if not caster.plot().isCity():
			return False
	pCity = cf.getAshenVeilCity(3)
	if pCity == -1:
		return False
	return True

def spellHyboremsWhisper(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	if gc.getGame().isNetworkMultiPlayer():
		pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		return

	iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_HYBOREMS_WHISPER')
	triggerData = pPlayer.initTriggeredData(iEvent, true, -1, caster.getX(), caster.getY(), iPlayer, -1, -1, -1, -1, -1)

def reqImpersonateLeader(caster):
	pCity = caster.plot().getPlotCity()
	if pCity.getOwner() == caster.getOwner():
		return False
	if gc.getPlayer(pCity.getOwner()).isHuman():
		return False
	return True

def spellIncarnate(pCaster):
	iForest = gc.getInfoTypeForString('FEATURE_FOREST')
	iX = pCaster.getX()
	iY = pCaster.getY()
	pPlot = pCaster.plot()
	pPlot.setFeatureType(iForest,0)
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			if not (iiX == iX and iiY == iY):
				pPlot = CyMap().plot(iiX,iiY)
				if pPlot.isNone() == False:
					if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_NEW')):
						pPlot.setFeatureType(iForest,0)
					if pPlot.getFeatureType() == -1:
						if CyGame().getSorenRandNum(100, "Incarnate") <= 25:
							pPlot.setFeatureType(iForest,0)
						
def spellImpersonateLeader(caster):
	pCity = caster.plot().getPlotCity()
	iNewPlayer = pCity.getOwner()
	iOldPlayer = caster.getOwner()
	iTimer = 5 + CyGame().getSorenRandNum(10, "Impersonate Leader")
	CyGame().reassignPlayerAdvanced(iOldPlayer, iNewPlayer, iTimer)

def reqInquisition(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	StateBelief = pPlayer.getStateReligion()
	if StateBelief == -1:
		if caster.getOwner() != pCity.getOwner():
			return False
	if (StateBelief != gc.getPlayer(pCity.getOwner()).getStateReligion()):
		return False
	for iTarget in range(gc.getNumReligionInfos()):
		if (StateBelief != iTarget and pCity.isHasReligion(iTarget) and pCity.isHolyCityByType(iTarget) == False):
			return True
	return False

def spellInquisition(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	StateBelief = gc.getPlayer(pCity.getOwner()).getStateReligion()
	iRnd = CyGame().getSorenRandNum(4, "Bob")
	if StateBelief == gc.getInfoTypeForString('RELIGION_THE_ORDER'):
		iRnd = iRnd - 1
	for iTarget in range(gc.getNumReligionInfos()):
		if (not StateBelief == iTarget and pCity.isHasReligion(iTarget) and not pCity.isHolyCityByType(iTarget)):
			pCity.setHasReligion(iTarget, False, True, True)
			iRnd = iRnd + 1
			for i in range(gc.getNumBuildingInfos()):
				if gc.getBuildingInfo(i).getPrereqReligion() == iTarget:
					pCity.setNumRealBuilding(i, 0)
	if iRnd >= 1:
		pCity.changeHurryAngerTimer(iRnd)

def reqIntoTheMist(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if pPlayer.getNumUnits() < 40:
			return False
	return True

def spellIntoTheMist(caster):
	iInvisible = gc.getInfoTypeForString('PROMOTION_HIDDEN')
	pPlayer = gc.getPlayer(caster.getOwner())
	py = PyPlayer(caster.getOwner())
	for pUnit in py.getUnitList():
		pUnit.setHasPromotion(iInvisible, True)

#	iHiddenCity = gc.getInfoTypeForString('BUILDING_HIDDEN_CITY')
#	iMist = gc.getInfoTypeForString('BUILDING_MIST')
#	if iHiddenCity!=-1:
#		if pPlayer.getNumCities() > 0:
#			for pyCity in PyPlayer(caster.getOwner()).getCityList():
#				pCity = pyCity.GetCy()
#				pCity.setNumRealBuilding(iHiddenCity, 1)
#				pCity.setNumRealBuilding(iMist, 1)


def reqIraUnleashed(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_IRA')) >= 4:
		return False
	return True

def spellIraUnleashed(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iCount = 4 - pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_IRA'))
	for i in range(iCount):
		pPlayer.initUnit(gc.getInfoTypeForString('UNIT_IRA'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def reqKidnap(caster):
	pCity = caster.plot().getPlotCity()
	if pCity.getTeam() == caster.getTeam():
		return False
	i = 0
	i = i + pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_PRIEST'))
	i = i + pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ARTIST'))
	i = i + pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_MERCHANT'))
	i = i + pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ENGINEER'))
	i = i + pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_SCIENTIST'))
	if i == 0:
		return False
	return True

def spellKidnap(caster):
	pCity = caster.plot().getPlotCity()
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_PRIEST')) > 0:
		iUnit = gc.getInfoTypeForString('UNIT_PROPHET')
		iSpec = gc.getInfoTypeForString('SPECIALIST_GREAT_PRIEST')
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ARTIST')) > 0:
		iUnit = gc.getInfoTypeForString('UNIT_ARTIST')
		iSpec = gc.getInfoTypeForString('SPECIALIST_GREAT_ARTIST')
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_MERCHANT')) > 0:
		iUnit = gc.getInfoTypeForString('UNIT_MERCHANT')
		iSpec = gc.getInfoTypeForString('SPECIALIST_GREAT_MERCHANT')
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ENGINEER')) > 0:
		iUnit = gc.getInfoTypeForString('UNIT_ENGINEER')
		iSpec = gc.getInfoTypeForString('SPECIALIST_GREAT_ENGINEER')
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_SCIENTIST')) > 0:
		iUnit = gc.getInfoTypeForString('UNIT_SCIENTIST')
		iSpec = gc.getInfoTypeForString('SPECIALIST_GREAT_SCIENTIST')
	iChance = caster.baseCombatStr() * 8
	if iChance > 95:
		iChance = 95
	pPlayer = gc.getPlayer(caster.getOwner())
	if CyGame().getSorenRandNum(100, "Kidnap") <= iChance:
		newUnit = pPlayer.initUnit(iUnit, caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		pCity.changeFreeSpecialistCount(iSpec, -1)
	else:
		if CyGame().getSorenRandNum(100, "Kidnap") <= 50:
			caster.setXY(pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), false, true, true)
		else:
			caster.kill(True, 0)
		cf.startWar(caster.getOwner(), pCity.getOwner(), WarPlanTypes.WARPLAN_TOTAL)

def reqLegends(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getNumCities() == 0:
		return False
	if pPlayer.isHuman() == False:
		if pPlayer.getNumCities() < 3:
			return False
	return True

def spellLegends(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	for pyCity in PyPlayer(iPlayer).getCityList() :
		pCity = pyCity.GetCy()
		pCity.changeCulture(iPlayer, 300, True)

def reqLichdom(caster):
	if caster.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_FLESH_GOLEM'):
		return False
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_PUPPET')):
		return False
	if isWorldUnitClass(caster.getUnitClassType()):
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_LICH')) >= 4:
		return False
	return True

def reqMask(caster):
	if caster.isHiddenNationality():
		return False
	if caster.hasCargo():
		return False
	if caster.isCargo():
		return False
	pGroup = caster.getGroup()
	if pGroup.isNone() == False:
		if pGroup.getNumUnits() > 1:
			return False
	return True

def reqMezmerizeAnimal(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ANIMAL'):
					if eTeam.isAtWar(pUnit.getTeam()):
						return True
	return False

def spellMezmerizeAnimal(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ANIMAL'):
					if eTeam.isAtWar(pUnit.getTeam()):
						if pUnit.isDelayedDeath() == False:
							if pUnit.isResisted(caster, gc.getInfoTypeForString('SPELL_MEZMERIZE_ANIMAL')) == False:
								newUnit = pPlayer.initUnit(pUnit.getUnitType(), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								newUnit.convert(pUnit)

def reqMirror(caster):
	iPlayer = caster.getOwner()
	pPlot = caster.plot()
	if caster.isImmuneToMagic():
		return False
	if pPlot.isVisibleEnemyUnit(iPlayer):
		return False
	return True

def spellMirror(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	newUnit = pPlayer.initUnit(caster.getUnitType(), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	for i in range(gc.getNumPromotionInfos()):
		newUnit.setHasPromotion(i, caster.isHasPromotion(i))
		if gc.getPromotionInfo(i).isEquipment():
			newUnit.setHasPromotion(i, False)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ILLUSION'), True)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_IMMORTAL'), False)
	if newUnit.isImmortal():
		newUnit.changeImmortal(-1)
	newUnit.setDamage(caster.getDamage(), caster.getOwner())
	newUnit.setLevel(caster.getLevel())
	newUnit.setExperience(caster.getExperience(), -1)
	newUnit.setHasCasted(True)
	newUnit.setDuration(1)

def reqPeace(caster):
	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_CHANGING_WAR_PEACE):
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	if eTeam.getAtWarCount(True) > 2:
		return False
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_PEACEFUL_CORLINDALE')):
		return False
	return True

def spellPeace(caster):
	eTeam = gc.getTeam(gc.getPlayer(caster.getOwner()).getTeam())
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if (pPlayer.isAlive() and iPlayer != caster.getOwner() and iPlayer != gc.getBARBARIAN_PLAYER()):
			i2Team = gc.getPlayer(iPlayer).getTeam()
			if eTeam.isAtWar(i2Team):
				eTeam.makePeace(i2Team)
	CyGame().changeGlobalCounter(-1 * (CyGame().getGlobalCounter() / 2))

def reqPeaceSevenPines(caster):
	pPlot = caster.plot()
	if not pPlot.isPythonActive():
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	if eTeam.getAtWarCount(True) == 0:
		return False
#AI added Sephi
	if caster.isBarbarian():
		return False
	if not pPlayer.isHuman():
		for iTeam in range(gc.getMAX_CIV_TEAMS()):
			eTeam = gc.getTeam(iTeam)
			if iTeam!=caster.getTeam():
				if eTeam.isAtWar(caster.getTeam()):
					if gc.getTeam(caster.getTeam()).getPower(true)*80<eTeam.getPower(true)*100:
						return True
		return False
#AI added Sephi
	return True

def spellPeaceSevenPines(caster):
	eTeam = gc.getTeam(gc.getPlayer(caster.getOwner()).getTeam())
	for iPlayer in range(gc.getMAX_CIV_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if (pPlayer.isAlive() and iPlayer != caster.getOwner() and iPlayer != gc.getBARBARIAN_PLAYER()):
			i2Team = gc.getPlayer(iPlayer).getTeam()
			if eTeam.isAtWar(i2Team):
				eTeam.makePeace(i2Team)
	CyGame().changeGlobalCounter(-1 * (CyGame().getGlobalCounter() / 2))
	pPlot = caster.plot()
	pPlot.setPythonActive(False)

def reqPillarofFire(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			bEnemy = false
			bNeutral = false
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					bEnemy = true
				else:
					bNeutral = true
			if (bEnemy and bNeutral == false):
				return true
	return false

def spellPillarofFire(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	iBestValue = 0
	pBestPlot = -1
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			bNeutral = false
			iValue = 0
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					iValue += 5 * pUnit.baseCombatStr()
				else:
					bNeutral = true
			if (iValue > iBestValue and bNeutral == false):
				iBestValue = iValue
				pBestPlot = pPlot
	if pBestPlot != -1:
		for i in range(pBestPlot.getNumUnits()):
			pUnit = pBestPlot.getUnit(i)
			pUnit.doDamage(50, 75, caster, gc.getInfoTypeForString('DAMAGE_FIRE'), true)
		if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE')):
			bValid = True
			iImprovement = pPlot.getImprovementType()
			if iImprovement != -1 :
				if gc.getImprovementInfo(iImprovement).isPermanent():
					bValid = False
			if bValid:
				if CyGame().getSorenRandNum(100, "Flames Spread") <= gc.getDefineINT('FLAMES_SPREAD_CHANCE'):
					pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_SMOKE'))
		CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_PILLAR_OF_FIRE'),pBestPlot.getPoint())

def reqPirateCove(caster):
	pPlot = caster.plot()
	if pPlot.isWater() == False:
		return False
	if pPlot.isAdjacentToLand() == False:
		return False
	if pPlot.isCity():
		return False
	if pPlot.getOwner() != caster.getOwner():
		return False
	if pPlot.getImprovementType() != -1:
		return False
	if pPlot.getBonusType(caster.getTeam()) != -1:
		return False
	iPirateCove = gc.getInfoTypeForString('IMPROVEMENT_PIRATE_COVE')
	iPirateHarbor = gc.getInfoTypeForString('IMPROVEMENT_PIRATE_HARBOR')
	iPiratePort = gc.getInfoTypeForString('IMPROVEMENT_PIRATE_PORT')
	iX = caster.getX()
	iY = caster.getY()
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				iImprovement = pPlot.getImprovementType()
				if iImprovement == iPirateCove:
					return False
				if iImprovement == iPirateHarbor:
					return False
				if iImprovement == iPiratePort:
					return False

	return True

def spellPirateCove(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())

	if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_LANUN'):
		pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_PIRATE_COVE'))

def reqMarchOfTheTrees(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		iTeam = gc.getPlayer(caster.getOwner()).getTeam()
		eTeam = gc.getTeam(iTeam)
		if eTeam.getAtWarCount(True) < 2:
			return False
	return True

def spellMarchOfTheTrees(caster):
	iAncientForest = gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')
	iForest = gc.getInfoTypeForString('FEATURE_FOREST')
	iNewForest = gc.getInfoTypeForString('FEATURE_FOREST_NEW')
	iTreant = gc.getInfoTypeForString('UNIT_TREANT')
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.isOwned():
			if pPlot.getOwner() == iPlayer:
				if (pPlot.getFeatureType() == iForest or pPlot.getFeatureType() == iAncientForest):
					if not pPlot.isVisibleEnemyUnit(iPlayer):
						newUnit = pPlayer.initUnit(iTreant, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
						newUnit.setDuration(5)
						pPlot.setFeatureType(iNewForest,0)
#added Sephi
						newUnit.AI_setGroupflag(43)	# 43=GROUPFLAG_SUICIDE_SUMMON
						if newUnit.isHuman():
							newUnit.getGroup().setActivityType(ActivityTypes.ACTIVITY_SLEEP)

def reqMotherLode(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if pPlayer.getImprovementCount(gc.getInfoTypeForString('IMPROVEMENT_MINE')) < 10:
			return False
	return True

def spellMotherLode(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlayer.changeGold(pPlayer.getImprovementCount(gc.getInfoTypeForString('IMPROVEMENT_MINE')) * 25)
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.isOwned():
			if pPlot.getOwner() == iPlayer:
				if pPlot.isWater() == False:
					if pPlot.isPeak() == False:
						if pPlot.isHills() == False:
							if CyGame().getSorenRandNum(100, "Mother Lode") < 10:
								pPlot.setPlotType(PlotTypes.PLOT_HILLS, True, True)

def spellOpenChest(caster):
	pPlot = caster.plot()
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	bValid = True
	if bValid:
		if CyGame().getSorenRandNum(100, "Explore Lair") < 25:
			lTrapList = ['POISON', 'FIRE', 'SPORES']
			sTrap = lTrapList[CyGame().getSorenRandNum(len(lTrapList), "Pick Trap")-1]
			point = pPlot.getPoint()
			if sTrap == 'POISON':
				caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), True)
				caster.doDamageNoCaster(25, 90, gc.getInfoTypeForString('DAMAGE_POISON'), false)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_TRAP_POISON", ()),'AS2D_POSITIVE_DINK',1,'Art/Interface/Buttons/Promotions/Poisoned.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			if sTrap == 'FIRE':
				caster.doDamageNoCaster(50, 90, gc.getInfoTypeForString('DAMAGE_FIRE'), false)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_TRAP_FIRE", ()),'AS2D_POSITIVE_DINK',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,5,9',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
				CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_RING_OF_FLAMES'),point)
				CyAudioGame().Play3DSound("AS3D_SPELL_FIREBALL",point.x,point.y,point.z)
			if sTrap == 'SPORES':
				caster.changeImmobileTimer(3)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_TRAP_SPORES", ()),'AS2D_POSITIVE_DINK',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,2,11',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
				CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SPORES'),point)
				CyAudioGame().Play3DSound("AS3D_SPELL_CONTAGION",point.x,point.y,point.z)

		lList = ['EMPTY', 'HIGH_GOLD', 'ITEM_HEALING_SALVE', 'ITEM_JADE_TORC', 'ITEM_ROD_OF_WINDS', 'ITEM_TIMOR_MASK', 'TECH']
		sGoody = lList[CyGame().getSorenRandNum(len(lList), "Pick Goody")-1]
		if sGoody == 'EMPTY':
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_TREASURE_EMPTY",()),'AS2D_POSITIVE_DINK',1,'Art/Interface/Buttons/Equipment/Treasure.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		if sGoody == 'HIGH_GOLD':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_HIGH_GOLD'), caster)
		if sGoody == 'ITEM_HEALING_SALVE':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_HEALING_SALVE'), caster)
		if sGoody == 'ITEM_JADE_TORC':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_JADE_TORC'), caster)
		if sGoody == 'ITEM_POTION_OF_INVISIBILITY':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_POTION_OF_INVISIBILITY'), caster)
		if sGoody == 'ITEM_POTION_OF_RESTORATION':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_POTION_OF_RESTORATION'), caster)
		if sGoody == 'ITEM_ROD_OF_WINDS':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_ROD_OF_WINDS'), caster)
		if sGoody == 'ITEM_TIMOR_MASK':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_TIMOR_MASK'), caster)
		if sGoody == 'TECH':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_TECH'), caster)
		pTreasure = -1
		iTreasure = gc.getInfoTypeForString('EQUIPMENT_TREASURE')
		for i in range(pPlot.getNumUnits()):
			pUnit = pPlot.getUnit(i)
##--------		Unofficial Bug Fix: Modified by Denev	--------##
#			if pUnit.getUnitType() == iTreasure:
			if pUnit.getUnitType() == iTreasure and not pUnit.isDelayedDeath():
##--------		Unofficial Bug Fix: End Modify			--------##
				pTreasure = pUnit
##--------		Unofficial Bug Fix: Modified by Denev	--------##
#		pTreasure.kill(True, 0)
		pTreasure.kill(True, PlayerTypes.NO_PLAYER)
##--------		Unofficial Bug Fix: End Modify			--------##


def reqPromoteSettlement(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	if not pCity.isSettlement():
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getNumCities() - pPlayer.getNumSettlements() >= pPlayer.getMaxCities():
		return False
	return True

def spellPromoteSettlement(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pCity.setSettlement(False)
	pCity.setPlotRadius(3)
#	pPlayer = gc.getPlayer(caster.getOwner())
#	pCity.changeCulture(iPlayer, 500, true)

def reqRagingSeas(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		eTeam = gc.getTeam(pPlayer.getTeam())
		iArcane = gc.getInfoTypeForString('UNITCLASS_ARCANE_BARGE')
		iCaravel = gc.getInfoTypeForString('UNITCLASS_CARAVEL')
		iGalley = gc.getInfoTypeForString('UNITCLASS_GALLEY')
		iFrigate = gc.getInfoTypeForString('UNITCLASS_FRIGATE')
		iGalleon = gc.getInfoTypeForString('UNITCLASS_GALLEON')
		iManOWar = gc.getInfoTypeForString('UNITCLASS_MAN_O_WAR')
		iPrivateer = gc.getInfoTypeForString('UNITCLASS_PRIVATEER')
		iQueenOfTheLine = gc.getInfoTypeForString('UNITCLASS_QUEEN_OF_THE_LINE')
		iTrireme = gc.getInfoTypeForString('UNITCLASS_TRIREME')
		for iPlayer2 in range(gc.getMAX_PLAYERS()):
			pPlayer2 = gc.getPlayer(iPlayer2)
			if pPlayer2.isAlive():
				iTeam2 = gc.getPlayer(iPlayer2).getTeam()
				if eTeam.isAtWar(iTeam2):
					iCount = pPlayer2.getUnitClassCount(iArcane)
					iCount += pPlayer2.getUnitClassCount(iCaravel)
					iCount += pPlayer2.getUnitClassCount(iGalley)
					iCount += pPlayer2.getUnitClassCount(iFrigate) * 2
					iCount += pPlayer2.getUnitClassCount(iGalleon) * 2
					iCount += pPlayer2.getUnitClassCount(iManOWar) * 3
					iCount += pPlayer2.getUnitClassCount(iPrivateer)
					iCount += pPlayer2.getUnitClassCount(iQueenOfTheLine) * 3
					iCount += pPlayer2.getUnitClassCount(iTrireme)
					if iCount > 10:
						return True
		return False
	return True

def spellRagingSeas(caster):
	iCold = gc.getInfoTypeForString('DAMAGE_COLD')
	iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
	iLanun = gc.getInfoTypeForString('CIVILIZATION_LANUN')
	iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
	iSpring = gc.getInfoTypeForString('EFFECT_SPRING')
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.isAdjacentToWater():
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.getCivilizationType() != iLanun:
					pUnit.doDamageNoCaster(75, 100, iCold, false)
			if pPlot.getImprovementType() != -1:
				if pPlot.getFeatureType() == iFlames:
					pPlot.setFeatureType(-1, 0)
				if pPlot.getImprovementType() == iSmoke:
					pPlot.setImprovementType(-1)
				else:
					if pPlot.isOwned():
						if gc.getImprovementInfo(pPlot.getImprovementType()).isPermanent() == False:
							if gc.getPlayer(pPlot.getOwner()).getCivilizationType() != iLanun:
								if CyGame().getSorenRandNum(100, "Raging Seas") <= 25:
									pPlot.setImprovementType(-1)
			if pPlot.isVisibleToWatchingHuman():
				CyEngine().triggerEffect(iSpring,pPlot.getPoint())

def spellRaiseSkeleton(caster):
	pPlot = caster.plot()
	if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_GRAVEYARD'):
		pPlot.setImprovementType(-1)
		caster.cast(gc.getInfoTypeForString('SPELL_RAISE_SKELETON'))
		caster.cast(gc.getInfoTypeForString('SPELL_RAISE_SKELETON'))

def reqRally(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) != gc.getInfoTypeForString('CIVIC_CRUSADE'):
		return False
	if pPlayer.isHuman() == False:
		if pPlayer.getNumCities() < 5:
			return False
	return True

def spellRally(caster):
	iOwner = caster.getOwner()
	pPlayer = gc.getPlayer(iOwner)
	iDemagog = gc.getInfoTypeForString('UNIT_DEMAGOG')
	iTown = gc.getInfoTypeForString('IMPROVEMENT_TOWN')
	iEnclave = gc.getInfoTypeForString('IMPROVEMENT_ENCLAVE')
	iVillage = gc.getInfoTypeForString('IMPROVEMENT_VILLAGE')
	iCount = 0
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.getOwner() == iOwner:
			if pPlot.isCity():
				newUnit = pPlayer.initUnit(iDemagog, pPlot.getX(), pPlot.getY(),  UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.changeExperience(10, -1, false, false, false)
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_THE_ASHEN_VEIL'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_THE_ASHEN_VEIL'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_THE_ORDER'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_THE_ORDER'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_FELLOWSHIP_OF_LEAVES'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_FELLOWSHIP_OF_LEAVES'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_RUNES_OF_KILMORPH'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_RUNES_OF_KILMORPH'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_OCTOPUS_OVERLORDS'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_OCTOPUS_OVERLORDS'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_THE_EMPYREAN'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_THE_EMPYREAN'));
			if pPlot.getImprovementType() == iTown or pPlot.getImprovementType() ==  iEnclave :
				pCity = pPlot.getWorkingCity()
				newUnit = pPlayer.initUnit(iDemagog, pPlot.getX(), pPlot.getY(),  UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_THE_ASHEN_VEIL'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_THE_ASHEN_VEIL'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_THE_ORDER'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_THE_ORDER'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_FELLOWSHIP_OF_LEAVES'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_FELLOWSHIP_OF_LEAVES'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_RUNES_OF_KILMORPH'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_RUNES_OF_KILMORPH'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_OCTOPUS_OVERLORDS'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_OCTOPUS_OVERLORDS'));
				if pPlayer.getStateReligion() ==  gc.getInfoTypeForString ('RELIGION_THE_EMPYREAN'):
					newUnit.setReligion (gc.getInfoTypeForString ('RELIGION_THE_EMPYREAN'));
				if pPlot.getImprovementType() == iEnclave:
					pPlot.setImprovementType(iTown)
				else:
					pPlot.setImprovementType(iVillage)

def spellReadTheGrimoire(caster):
	iBonus=0
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot=caster.plot()
	point = pPlot.getPoint()
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING1')):
		iBonus=100
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING2')):
		iBonus=iBonus+100
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING3')):
		iBonus=iBonus+100
	if caster.getReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
		iBonus=iBonus+5
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEATH1')):
		iBonus=iBonus+5
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEATH2')):
		iBonus=iBonus+5
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEATH3')):
		iBonus=iBonus+5
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENTROPY1')):
		iBonus=iBonus+5
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENTROPY2')):
		iBonus=iBonus+5
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENTROPY3')):
		iBonus=iBonus+5
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HERO')):
		iBonus=iBonus+10
	iRnd = CyGame().getSorenRandNum(100, "Read the Grimoire")+iBonus
	caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PROPHECY_MARK'), True)
	if iRnd < 30:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)
	if iRnd >= 30 and iRnd < 40:
		caster.cast(gc.getInfoTypeForString('SPELL_BURNING_BLOOD'))
	if iRnd >= 40 and iRnd < 50:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
	if iRnd >= 50 and iRnd < 70:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), True)
	if iRnd >= 70 and iRnd < 80:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), True)
	if iRnd >= 80 and iRnd < 100:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING1'), True)
	if iRnd >= 100 and iRnd < 110:
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_PIT_BEAST'))
	if iRnd >= 110 and iRnd < 120:
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_SPECTRE'))
	if iRnd >= 120 and iRnd < 130:
		caster.cast(gc.getInfoTypeForString('SPELL_WITHER'))
	if iRnd >= 130 and iRnd < 140:
		caster.cast(gc.getInfoTypeForString('SPELL_MUTATION'))
	if iRnd >= 140 and iRnd < 145:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STIGMATA'), True)
	if iRnd >= 145 and iRnd < 150:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), True)
	if iRnd >= 150 and iRnd < 155:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
	if iRnd >= 155 and iRnd < 160:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)
	if iRnd >= 160 and iRnd < 190:
		caster.kill(True, PlayerTypes.NO_PLAYER)
	if iRnd >= 190 and iRnd < 200:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING1'), True)
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING2'), True)
	if iRnd >= 200 and iRnd < 210:
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_BALOR'))
	if iRnd >= 210 and iRnd < 220:
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_WRAITH'))
	if iRnd >= 220 and iRnd < 230:
		caster.cast(gc.getInfoTypeForString('SPELL_WITHER'))
		caster.cast(gc.getInfoTypeForString('SPELL_WITHER'))
	if iRnd >= 230 and iRnd < 240:
		caster.cast(gc.getInfoTypeForString('SPELL_WONDER'))
	if iRnd >= 240 and iRnd < 245:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STIGMATA'), True)
	if iRnd >= 245 and iRnd < 250:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), True)
	if iRnd >= 250 and iRnd < 255:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
	if iRnd >= 255 and iRnd < 260:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)
	if iRnd >= 260 and iRnd < 265:
		CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SPELL1'),point)
		CyAudioGame().Play3DSound("AS3D_SPELL_DEFILE",point.x,point.y,point.z)
		for iX in range(pPlot.getX()-1, pPlot.getX()+2, 1):
			for iY in range(pPlot.getY()-1, pPlot.getY()+2, 1):
				pLoopPlot = CyMap().plot(iX,iY)
				if pLoopPlot.isNone() == False:
					pLoopPlot.changePlotCounter(100)
	if iRnd >= 265 and iRnd < 270:
		caster.kill(True, PlayerTypes.NO_PLAYER)
	if iRnd >= 270 and iRnd < 275:
		caster.cast(gc.getInfoTypeForString('SPELL_BURNING_BLOOD'))
	if iRnd >= 275 and iRnd < 295:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POTENCY'), True)
	if iRnd >= 295 and iRnd < 300:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING1'), True)
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING2'), True)
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING3'), True)
	if iRnd >= 300 and iRnd < 310:
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_BALOR'))
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_BALOR'))
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_BALOR'))
	if iRnd >= 310 and iRnd < 320:
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_WRAITH'))
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_WRAITH'))
		caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_WRAITH'))
	if iRnd >= 320 and iRnd < 330:
		caster.cast(gc.getInfoTypeForString('SPELL_WITHER'))
		caster.cast(gc.getInfoTypeForString('SPELL_WITHER'))
		caster.cast(gc.getInfoTypeForString('SPELL_WITHER'))
	if iRnd >= 330 and iRnd < 340:
		caster.cast(gc.getInfoTypeForString('SPELL_WONDER'))
		caster.cast(gc.getInfoTypeForString('SPELL_WONDER'))
		caster.cast(gc.getInfoTypeForString('SPELL_WONDER'))
	if iRnd >= 340 and iRnd < 345:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), True)
	if iRnd >= 345 and iRnd < 350:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), True)
	if iRnd >= 350 and iRnd < 355:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
	if iRnd >= 355 and iRnd < 360:
		CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SPELL1'),point)
		CyAudioGame().Play3DSound("AS3D_SPELL_DEFILE",point.x,point.y,point.z)
		for iX in range(pPlot.getX()-1, pPlot.getX()+2, 1):
			for iY in range(pPlot.getY()-1, pPlot.getY()+2, 1):
				pLoopPlot = CyMap().plot(iX,iY)
				if pLoopPlot.isNone() == False:
					pLoopPlot.changePlotCounter(100)
	if iRnd >= 360 and iRnd < 363:
		caster.kill(True, PlayerTypes.NO_PLAYER)
	if iRnd >= 363 and iRnd < 365:
		caster.cast(gc.getInfoTypeForString('SPELL_BURNING_BLOOD'))
	if iRnd >= 365 and iRnd < 370:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_VILE_TOUCH'), True)
	if iRnd >= 370 and iRnd < 380:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POTENCY'), True)
	if iRnd >= 380 and iRnd < 385:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STIGMATA'), True)
	if iRnd >= 385 and iRnd < 400:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SUNDERED'), True)
	if iRnd >= 400 and iRnd < 405:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MAGIC_IMMUNE'), True)
	if iRnd >= 405 and iRnd < 410:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FEAR'), True)
	if iRnd >= 410 and iRnd < 420:
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WRAITH'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
		newUnit.finishMoves()
		newUnit.setHasCasted(True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_BURNING_BLOOD'), True)
	if iRnd >= 420 and iRnd < 430:
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_BALOR'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
		newUnit.finishMoves()
		newUnit.setHasCasted(True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_BURNING_BLOOD'), True)
	if iRnd >= 430 and iRnd < 440:
		caster.changeExperience(50, -1, False, False, False)
	if iRnd >= 440 and iRnd < 443:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TWINCAST'), True)
	if iRnd >= 443:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_IMMORTAL'), True)

def reqRebuildBarnaxus(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pCityPlayer = gc.getPlayer(pCity.getOwner())
	if caster.getUnitType() == gc.getInfoTypeForString('UNIT_BARNAXUS'):
		return False
	if pCityPlayer is None or pCityPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_LUCHUIRP'):
		return False
	return True

def spellRebuildBarnaxus(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pCityPlayer = gc.getPlayer(pCity.getOwner())
	newUnit = pCityPlayer.initUnit(gc.getInfoTypeForString('UNIT_BARNAXUS'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
	newUnit.setDamage(75, caster.getOwner())
	newUnit.finishMoves()
	pCity.applyBuildEffects(newUnit)
	if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_LUCHUIRP'):
		pCityPlayer.AI_changeAttitudeExtra(iPlayer,2)

def reqRecklessCharge(caster):
	iRecklessCharge = gc.getInfoTypeForString('PROMOTION_RECKLESS_CHARGE')
	iFatigued = gc.getInfoTypeForString('PROMOTION_FATIGUED')
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() and pUnit.getOwner() == caster.getOwner()):
			if not (pUnit.isHasPromotion(iFatigued) or pUnit.isHasPromotion(iRecklessCharge)):
				return true
	return False

def spellRecklessCharge(caster): 
	iRecklessCharge = gc.getInfoTypeForString('PROMOTION_RECKLESS_CHARGE')
	iFatigued = gc.getInfoTypeForString('PROMOTION_FATIGUED')
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() and pUnit.getOwner() == caster.getOwner()):
			if not pUnit.isHasPromotion(iFatigued):
				pUnit.setHasPromotion(iRecklessCharge,True)

def reqRecruitMercenary(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		pPlot = caster.plot()
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_KHAZAD'):
			return False
		iX = caster.getX()
		iY = caster.getY()
		eTeam = gc.getTeam(pPlayer.getTeam())
		for iiX in range(iX-1, iX+2, 1):
			for iiY in range(iY-1, iY+2, 1):
				pPlot = CyMap().plot(iiX,iiY)
				for i in range(pPlot.getNumUnits()):
					pUnit = pPlot.getUnit(i)
					p2Player = gc.getPlayer(pUnit.getOwner())
					e2Team = p2Player.getTeam()
					if eTeam.isAtWar(e2Team) == True:
						return True
		return False
	return True

def spellRecruitMercenary(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iUnit = gc.getInfoTypeForString('UNITCLASS_MERCENARY')
	infoCiv = gc.getCivilizationInfo(pPlayer.getCivilizationType())
	iUnit = infoCiv.getCivilizationUnits(iUnit)
	newUnit = pPlayer.initUnit(iUnit, caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
	newUnit.finishMoves()
	newUnit.setHasCasted(True)
	if caster.getUnitType() == gc.getInfoTypeForString('UNIT_MAGNADINE'):
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_LOYALTY'), True)

def spellReleaseFromCage(caster):
	pPlot = caster.plot()
	pPlot.setImprovementType(-1)

def reqReligiousFervor(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iReligion = pPlayer.getStateReligion()
	if iReligion == -1:
		return False
	if pPlayer.isHuman() == False:
		iCount = 0
		for pyCity in PyPlayer(iPlayer).getCityList() :
			pCity = pyCity.GetCy()
			if pCity.isHasReligion(iReligion):
				iCount += 1
		if iCount < 5:
			return False
	return True

def spellReligiousFervor(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	iReligion = pPlayer.getStateReligion()
	if iReligion == gc.getInfoTypeForString('RELIGION_FELLOWSHIP_OF_LEAVES'):
		iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_LEAVES')
	if iReligion == gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH'):
		iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_KILMORPH')
	if iReligion == gc.getInfoTypeForString('RELIGION_THE_EMPYREAN'):
		iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_THE_EMPYREAN')
	if iReligion == gc.getInfoTypeForString('RELIGION_THE_ORDER'):
		iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_THE_ORDER')
	if iReligion == gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS'):
		iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_THE_OVERLORDS')
	if iReligion == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
		iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_THE_VEIL')
	iCount = 0
	for pyCity in PyPlayer(iPlayer).getCityList() :
		pCity = pyCity.GetCy()
		if pCity.isHasReligion(iReligion):
			iCount += 1
	for pyCity in PyPlayer(iPlayer).getCityList() :
		pCity = pyCity.GetCy()
		newUnit = pPlayer.initUnit(iUnit, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.changeExperience(iCount*2, -1, False, False, False)
		newUnit.setReligion(iReligion)

def reqRepair(caster):
	pPlot = caster.plot()
	iGolem = gc.getInfoTypeForString('PROMOTION_GOLEM')
	iNaval = gc.getInfoTypeForString('UNITCOMBAT_NAVAL')
	iSiege = gc.getInfoTypeForString('UNITCOMBAT_SIEGE')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.getUnitCombatType() == iSiege or pUnit.getUnitCombatType() == iNaval or pUnit.isHasPromotion(iGolem)):
			if pUnit.getDamage() > 0:
				return True
	return False

def spellRepair(caster,amount):
	pPlot = caster.plot()
	iGolem = gc.getInfoTypeForString('PROMOTION_GOLEM')
	iNaval = gc.getInfoTypeForString('UNITCOMBAT_NAVAL')
	iSiege = gc.getInfoTypeForString('UNITCOMBAT_SIEGE')
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.getUnitCombatType() == iSiege or pUnit.getUnitCombatType() == iNaval or pUnit.isHasPromotion(iGolem)):
			pUnit.changeDamage(-amount,0)

def reqRessurection(caster):
	if reqRessurectionGraveyard(caster):
		return True
	pPlayer = gc.getPlayer(caster.getOwner())
	iHero = cf.getHero(pPlayer)
	if iHero == -1:
		return False
	if not CyGame().isUnitClassMaxedOut(iHero, 0):
		return False
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.getUnitClassCount(iHero) > 0:
			return False
	py = PyPlayer(caster.getOwner())
	iSpell = gc.getInfoTypeForString('SPELL_RESSURECTION')
	for pUnit in py.getUnitList():
		if pUnit.getDelayedSpell() == iSpell:
			return False

# Scions start - Hopefully prevents innapropriate Alexi ressurections.
	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
		if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_KORRINA_BLACK_LADY')) > 0:
			return False
		elif pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_KORRINA_RED_LADY')) > 0:
			return False
		elif pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_KORRINA')) > 0:
			return False
		elif pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_KORRINA_WHITE_LADY')) > 0:
			return False
		elif pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_RISEN_EMPEROR')) > 0:
			return False
# Scions end

	return True

def spellRessurection(caster):
	if reqRessurectionGraveyard(caster):  # stay away from graveyards if you need to resurrect your hero ;)
		spellRessurectionGraveyard(caster)
	else:
		pPlot = caster.plot()
		pPlayer = gc.getPlayer(caster.getOwner())
		iHero = cf.getHero(pPlayer)
		infoCiv = gc.getCivilizationInfo(pPlayer.getCivilizationType())
		iUnit = infoCiv.getCivilizationUnits(iHero)
		# Scions start - Hopefully prevents innapropriate Alexi ressurections.
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
			if CyGame().isUnitClassMaxedOut(gc.getInfoTypeForString('UNITCLASS_KORRINA_BLACK_LADY'), 0):
				iUnit=gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY')
			elif CyGame().isUnitClassMaxedOut(gc.getInfoTypeForString('UNITCLASS_KORRINA_RED_LADY'), 0):
				iUnit=gc.getInfoTypeForString('UNIT_KORRINA_RED_LADY')
			elif CyGame().isUnitClassMaxedOut(gc.getInfoTypeForString('UNITCLASS_KORRINA_WHITE_LADY'), 0):
				iUnit=gc.getInfoTypeForString('UNIT_KORRINA_WHITE_LADY')
		# Scions end
		iBarn = gc.getInfoTypeForString('EQUIPMENT_PIECES_OF_BARNAXUS')
		iBarnProm = gc.getInfoTypeForString('PROMOTION_PIECES_OF_BARNAXUS')
		if iUnit == gc.getInfoTypeForString('UNIT_BARNAXUS'):
			for iPlayer2 in range(gc.getMAX_PLAYERS()):
				pPlayer2 = gc.getPlayer(iPlayer2)
				if pPlayer2.isAlive():
					py = PyPlayer(iPlayer2)
					for pUnit in py.getUnitList():
						if pUnit.isHasPromotion(iBarnProm):
							pUnit.setHasPromotion(iBarnProm, False)
							CyInterface().addMessage(iPlayer2,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_PIECES_LOST", ()),'AS2D_CHARM_PERSON',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,8,11',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)
						if pUnit.getUnitType() == iBarn:
							CyInterface().addMessage(iPlayer2,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_PIECES_LOST", ()),'AS2D_CHARM_PERSON',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,8,11',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)
							pUnit.kill(True, PlayerTypes.NO_PLAYER)
		newUnit = pPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
		for iProm in range(gc.getNumPromotionInfos()):
			if newUnit.isHasPromotion(iProm):
				if gc.getPromotionInfo(iProm).isEquipment():
					newUnit.setHasPromotion(iProm, False)
		CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_HERO_RESSURECTED", ()),'AS2D_CHARM_PERSON',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,7,8',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)

def reqRessurectionGraveyard(caster):
	pPlot = caster.plot()
	return pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_GRAVEYARD')

def spellRessurectionGraveyard(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())
	infoCiv = gc.getCivilizationInfo(pPlayer.getCivilizationType())
	iUnit = infoCiv.getCivilizationUnits(gc.getInfoTypeForString('UNITCLASS_CHAMPION'))
	if iUnit == -1:
		iUnit = gc.getInfoTypeForString('UNIT_CHAMPION')
	newUnit = pPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.changeExperience(CyGame().getSorenRandNum(30, "Ressurection Graveyard"), -1, False, False, False)
	pPlot.setImprovementType(-1)

def reqReveal(caster):
	if caster.isIgnoreHide():
		return False
	pPlot = caster.plot()
	if pPlot.getOwner() != caster.getOwner():
		return False
	return True

def spellReveal(caster):
	caster.setIgnoreHide(True)

def reqRevelry(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isGoldenAge():
		return False
	if pPlayer.isHuman() == False:
		if pPlayer.getTotalPopulation() < 25:
			return False
	return True

def spellRevelry(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.changeGoldenAgeTurns(CyGame().goldenAgeLength() * 2)

def reqRevelation(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	iX = caster.getX()
	iY = caster.getY()
	iHidden = gc.getInfoTypeForString('PROMOTION_HIDDEN')
	iHiddenNationality = gc.getInfoTypeForString('PROMOTION_HIDDEN_NATIONALITY')
	iIllusion = gc.getInfoTypeForString('PROMOTION_ILLUSION')
	iInvisible = gc.getInfoTypeForString('PROMOTION_INVISIBLE')
	for iiX in range(iX-3, iX+4, 1):
		for iiY in range(iY-3, iY+4, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for iUnit in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(iUnit)
				if eTeam.isAtWar(pUnit.getTeam()):
#				if pUnit.getTeam() != iTeam:
					if pUnit.isHasPromotion(iHidden):
						return True
					if pUnit.isHasPromotion(iHiddenNationality):
						return True
					if pUnit.isHasPromotion(iInvisible):
						return True
					if pUnit.isHasPromotion(iIllusion):
						return True
	return False

def spellRevelation(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	iX = caster.getX()
	iY = caster.getY()
	iHidden = gc.getInfoTypeForString('PROMOTION_HIDDEN')
	iHiddenNationality = gc.getInfoTypeForString('PROMOTION_HIDDEN_NATIONALITY')
	iIllusion = gc.getInfoTypeForString('PROMOTION_ILLUSION')
	iInvisible = gc.getInfoTypeForString('PROMOTION_INVISIBLE')
	for iiX in range(iX-3, iX+4, 1):
		for iiY in range(iY-3, iY+4, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for iUnit in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(iUnit)
				if eTeam.isAtWar(pUnit.getTeam()):
#				if pUnit.getTeam() != iTeam:
					if pUnit.isHasPromotion(iHidden):
						pUnit.setHasPromotion(iHidden, False)
					if pUnit.isHasPromotion(iHiddenNationality):
						pUnit.setHasPromotion(iHiddenNationality, False)
					if pUnit.isHasPromotion(iInvisible):
						pUnit.setHasPromotion(iInvisible, False)
					if pUnit.isHasPromotion(iIllusion):
						iRnd = CyGame().getSorenRandNum(10, "Lugus") + caster.getLevel()
						pUnit.doDamage(iRnd, 100, caster, gc.getInfoTypeForString('DAMAGE_HOLY'), true)

def reqRingofFlames(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman():
		return True
	bvalid=false
	iX = caster.getX()
	iY = caster.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			if not (iiX == iX and iiY == iY):
				pPlot = CyMap().plot(iiX,iiY)
				if pPlot.isVisibleEnemyUnit(caster.getOwner()):
					bvalid=true
	if bvalid:
		return True
	return False

def spellRingofFlames(caster):
	iX = caster.getX()
	iY = caster.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			if not (iiX == iX and iiY == iY):
				pPlot = CyMap().plot(iiX,iiY)
				bValid = True
				if pPlot.getImprovementType() != -1:
					if gc.getImprovementInfo(pPlot.getImprovementType()).isPermanent():
						bValid = False
				if bValid:
					if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE') or pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_NEW')):
						if CyGame().getSorenRandNum(100, "Flames Spread") <= gc.getDefineINT('FLAMES_SPREAD_CHANCE'):
							pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_SMOKE'))

def reqRiverOfBlood(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getNumCities() == 0:
		return False
	if pPlayer.isHuman() == False:
		if pPlayer.getNumCities() < 5:
			return False
	return True

def spellRiverOfBlood(caster):
	iOwner = caster.getOwner()
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.isAlive():
			if iPlayer != iOwner:
				for pyCity in PyPlayer(iPlayer).getCityList() :
					pCity = pyCity.GetCy()
					if pCity.getPopulation() > 2:
						if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FALLOW')):
							pCity.changePopulation(-1)
						else:
							pCity.changePopulation(-2)
						CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_RIVER_OF_BLOOD", ()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,6,9',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
			if iPlayer == iOwner:
				for pyCity in PyPlayer(iPlayer).getCityList() :
					pCity = pyCity.GetCy()
					pCity.changePopulation(2)

def reqRoar(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())
	iX = caster.getX()
	iY = caster.getY()
	iTarget = -1
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				if not pPlot.isCity():
					for i in range(pPlot.getNumUnits()):
						pUnit = pPlot.getUnit(i)
						if not pUnit.isImmuneToFear():
							p2Player = gc.getPlayer(pUnit.getOwner())
							e2Team = p2Player.getTeam()
							if eTeam.isAtWar(e2Team) == True:
								return True
	return False

def spellRoar(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	eTeam = gc.getTeam(pPlayer.getTeam())
	pPlot = caster.plot()
	iX = caster.getX()
	iY = caster.getY()

	iMaxFearTargets = gc.getUnitInfo(caster.getUnitType()).getTier()
	iTargetsFeared = 0

	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pLoopPlot = CyMap().plot(iiX,iiY)
			if not pLoopPlot.isNone():
				if not pLoopPlot.isCity():
					for i in range(pLoopPlot.getNumUnits() -1, -1, -1):
						pUnit = pLoopPlot.getUnit(i)
						p2Player = gc.getPlayer(pUnit.getOwner())
						if p2Player:
							i2Team = p2Player.getTeam()
							if eTeam.isAtWar(i2Team):
								if cf.doFear(pUnit, pPlot, caster, True):
									CyInterface().addMessage(pUnit.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_FEAR", (gc.getUnitInfo(pUnit.getUnitType()).getDescription(), )),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,7,9',ColorTypes(7),pUnit.getX(),pUnit.getY(),True,True)
									CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_FEAR_ENEMY", (gc.getUnitInfo(pUnit.getUnitType()).getDescription(), )),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,7,9',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)
									iTargetsFeared+=1
									if iTargetsFeared==iMaxFearTargets:
										return

def reqRobGrave(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):
			return False
	if pPlayer.isBarbarian():	#added Sephi
		return False
	return True

def spellRobGrave(caster):
	CyGame().changeGlobalCounter(1)
	pPlot = caster.plot()
	pPlot.setImprovementType(-1)
	pPlayer = gc.getPlayer(caster.getOwner())
	lList = ['LOW_GOLD', 'HIGH_GOLD', 'SPAWN_SKELETONS', 'SPAWN_SPECTRE']
	if pPlayer.canReceiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_TECH'), caster):
		lList = lList + ['TECH']
	sGoody = lList[CyGame().getSorenRandNum(len(lList), "Pick Goody")-1]
	if sGoody == 'LOW_GOLD':
		pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_LOW_GOLD'), caster)
	if sGoody == 'HIGH_GOLD':
		pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_HIGH_GOLD'), caster)
	if sGoody == 'TECH':
		pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_TECH'), caster)
	if sGoody == 'SPAWN_SKELETONS':
		pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_SKELETONS'), caster)
	if sGoody == 'SPAWN_SPECTRE':
		pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_SPECTRE'), caster)

def spellSacrificeAltar(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	eTeam = gc.getTeam(pPlayer.getTeam())
	iTech = pPlayer.getCurrentResearch()
	iNum = 10 + (caster.getLevel() * caster.getLevel())
	eTeam.changeResearchProgress(iTech, iNum, caster.getOwner())

#Sephi
def reqSacrificePyre(caster):
	if caster.getUnitType==gc.getInfoTypeForString('UNIT_HOLY_AVENGER'):
		return False
	return True

def spellSacrificePyre(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	caster.cast(gc.getInfoTypeForString('SPELL_RING_OF_FLAMES'))
	if caster.isImmortal():
		caster.changeImmortal(-1)
	iCount = 1
	if isWorldUnitClass(caster.getUnitClassType()):
		iCount = 7
	for i in range(iCount):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_FIRE_ELEMENTAL'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

def reqSanctuary(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getNumCities() == 0:
		return False
	if not pPlayer.isHuman():
		iTeam = gc.getPlayer(caster.getOwner()).getTeam()
		eTeam = gc.getTeam(iTeam)
		if eTeam.getAtWarCount(True) < 2:
			return False
	return True

def spellSanctuary(caster):
	iPlayer = caster.getOwner()
	iTeam = caster.getTeam()
	pPlayer = gc.getPlayer(iPlayer)
	pPlayer.changeSanctuaryTimer(30)
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.isOwned():
			if pPlot.getOwner() == iPlayer:
				for i in range(pPlot.getNumUnits(), -1, -1):
					pUnit = pPlot.getUnit(i)
					if pUnit.getTeam() != iTeam:
						pUnit.jumpToNearestValidPlot()

def reqSandLion(caster):
	pPlot = caster.plot()
	if (pPlot.getTerrainType() != gc.getInfoTypeForString('TERRAIN_DESERT') and pPlot.getTerrainType() != gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')):
		return False
	return True

def reqScorch(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())

	iMazatl=gc.getInfoTypeForString('CIVILIZATION_MAZATL')

	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ADVANCED_TERRAFORMING):
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_SWAMP'):
			if caster.isHuman() or iMazatl==-1 or pPlayer.getCivilizationType()!=gc.getInfoTypeForString('CIVILIZATION_MAZATL'):
				return True
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_MARSH'):
			if caster.isHuman() or iMazatl==-1 or pPlayer.getCivilizationType()!=gc.getInfoTypeForString('CIVILIZATION_MAZATL'):
				return True
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_TUNDRA'):
			if pPlayer.isHuman() == False:
				if caster.getOwner() != pPlot.getOwner():
					return False
				if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_DOVIELLO'):
					return False
				if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
					return False
			return True
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_MARSH'):
			if pPlayer.isHuman() == False:
				if caster.getOwner() != pPlot.getOwner():
					return False
			return True
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_GRASS'):
			if pPlayer.isHuman() == False:
				if caster.getOwner() != pPlot.getOwner():
					return False
			return True

	if (pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_PLAINS') or pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')):
		if pPlayer.isHuman() == False:
			if caster.getOwner() != pPlot.getOwner():
				return False
		return True
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SNOW'):
		if pPlayer.isHuman() == False:
			if caster.getOwner() != pPlot.getOwner():
				return False
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_DOVIELLO'):
				return False
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
				return False
		return True
	return False

def spellScorch(caster):
	pPlot = caster.plot()
	bstepone =false

	iSwamp = gc.getInfoTypeForString('IMPROVEMENT_SWAMP')

	if iSwamp!=-1:
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_SWAMP'):
			pPlot.setImprovementType(-1)

	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ADVANCED_TERRAFORMING):
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_TUNDRA'):
			pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),True,True)
			bstepone=true
		elif pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_MARSH'):
			pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_GRASS'),True,True)
			bstepone=true
		elif pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_GRASS'):
			pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),True,True)
			bstepone=true

	if not bstepone:
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_PLAINS'):
			pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_DESERT'),True,True)
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION'):
			pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_BURNING_SANDS'),True,True)
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SNOW'):
			pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_TUNDRA'),True,True)
	if pPlot.isOwned():
		cf.startWar(caster.getOwner(), pPlot.getOwner(), WarPlanTypes.WARPLAN_TOTAL)

def spellSing(caster):
	pPlot = caster.plot()
	point = pPlot.getPoint()
	iRnd = CyGame().getSorenRandNum(100, "Sing")
	szText = "AS3D_SING1"
	if iRnd > 25:
		szText = "AS3D_SING2"
	if iRnd > 50:
		szText = "AS3D_SING3"
	if iRnd > 75:
		szText = "AS3D_SING4"
	CyAudioGame().Play3DSound(szText,point.x,point.y,point.z)

def reqSironasTouch(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isFeatAccomplished(FeatTypes.FEAT_HEAL_UNIT_PER_TURN) == false:
		return False
	if not caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED')):
		if caster.getDamage() == 0:
			return False
		if pPlayer.isHuman() == False:
			if caster.getDamage() < 15:
				return False
	return True

def spellSironasTouch(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.setFeatAccomplished(FeatTypes.FEAT_HEAL_UNIT_PER_TURN, false)
	caster.changeDamage(-15,0)
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED')) and caster.getDamage() == 0:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), false)

def spellSlaveTradeBuy(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_SLAVE'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	iPromotion = -1
	iRnd = CyGame().getSorenRandNum(100, "Slave Trade Buy")
	if (iRnd >= 60 and iRnd < 70):
		iPromotion = gc.getInfoTypeForString('PROMOTION_DWARF')
	if (iRnd >= 70 and iRnd < 80):
		iPromotion = gc.getInfoTypeForString('PROMOTION_ELF')
	if (iRnd >= 80):
		iPromotion = gc.getInfoTypeForString('PROMOTION_ORC')
	if iPromotion != -1:
		newUnit.setHasPromotion(iPromotion, True)

def spellSlaveTrade(caster, iGold):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlayer.changeGold(iGold)
		

def spellHireMercenary(caster,unit):
	iUnit = gc.getInfoTypeForString(unit)
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = caster.plot()
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString(unit), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)


def spellSnowfall(caster):
	iX = caster.getX()
	iY = caster.getY()
	iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
	iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
	iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
	iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				if not pPlot.isWater():
					if pPlot.getTerrainType() != iSnow:
						iRnd = CyGame().getSorenRandNum(6, "Snowfall") + 3
						pPlot.setTempTerrainType(iSnow, iRnd)
						if pPlot.getFeatureType() == -1:
							if CyGame().getSorenRandNum(100, "Snowfall") < 2:
								pPlot.setFeatureType(iBlizzard, -1)
						if pPlot.getFeatureType() == iFlames:
							pPlot.setFeatureType(-1, -1)
						if pPlot.getImprovementType() == iSmoke:
							pPlot.setImprovementType(-1)

def spellSnowfallGreator(caster):
	iX = caster.getX()
	iY = caster.getY()
	iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
	iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
	iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
	iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if not pPlot.isNone():
				if not pPlot.isWater():
					if pPlot.getTerrainType() != iSnow:
						iRnd = CyGame().getSorenRandNum(12, "Snowfall") + 6
						pPlot.setTempTerrainType(iSnow, iRnd)
						if pPlot.getFeatureType() == -1:
							if CyGame().getSorenRandNum(100, "Snowfall") < 10:
								pPlot.setFeatureType(iBlizzard, -1)
						if pPlot.getFeatureType() == iFlames:
							pPlot.setFeatureType(-1, -1)
						if pPlot.getImprovementType() == iSmoke:
							pPlot.setImprovementType(-1)

def reqSpreadTheCouncilOfEsus(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pCity = caster.plot().getPlotCity()
	return False

def reqSpring(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())
	bFlames = false
	iX = pPlot.getX()
	iY = pPlot.getY()

	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_MALAKIM'):
		return False

	if pPlayer.isHuman() == False:
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
			return False
		if caster.getOwner() != pPlot.getOwner():
			return False


	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if pPlot2.getFeatureType() == gc.getInfoTypeForString('FEATURE_FLAMES') or pPlot2.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_SMOKE'):
				bFlames = true

	if bFlames:
		return True

	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ADVANCED_TERRAFORMING):
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_DESERT'):
			if pPlot.getFeatureType() != gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS'):
				if pPlot.getFeatureType() != gc.getInfoTypeForString('FEATURE_OASIS'):
					return True

	if pPlot.getTerrainType() != gc.getInfoTypeForString('TERRAIN_DESERT'):
		return False
	if pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS'):
		return False

	return True

def spellSpring(caster):
	pPlot = caster.plot()
	pPlayer = gc.getPlayer(caster.getOwner())

	iTerrain =pPlot.getTerrainType()

	if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ADVANCED_TERRAFORMING):
		if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_DESERT') and pPlot.isRiver():
			if pPlot.getFeatureType() == -1:
				pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS'),0)

	if (iTerrain == gc.getInfoTypeForString('TERRAIN_DESERT') and pPlot.getFeatureType() != gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')):
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),True,True)
		if pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_SCRUB'):
			pPlot.setFeatureType(-1, -1)

	iX = pPlot.getX()
	iY = pPlot.getY()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot2 = CyMap().plot(iiX,iiY)
			if pPlot2.getFeatureType() == gc.getInfoTypeForString('FEATURE_FLAMES'):
				pPlot2.setFeatureType(-1, -1)
			if pPlot2.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_SMOKE'):
				pPlot2.setImprovementType(-1)

def reqSprint(caster):
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_FATIGUED')):
		return False
	return True

def reqStasis(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if pPlayer.getNumCities() < 5:
			return False
	return True

def spellStasis(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	iDelay = 20
	iTeam = pPlayer.getTeam()
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
		iDelay = 10
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
		iDelay = 30
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
		iDelay = 60
	for iPlayer2 in range(gc.getMAX_PLAYERS()):
		pPlayer2 = gc.getPlayer(iPlayer2)
		if pPlayer2.isAlive():
			if pPlayer2.getTeam() != iTeam:
				pPlayer2.changeDisableProduction(iDelay)
				pPlayer2.changeDisableResearch(iDelay)

def reqSteal(caster):
	iTeam = caster.getTeam()
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getTeam() != iTeam:
			for iProm in range(gc.getNumPromotionInfos()):
				if pUnit.isHasPromotion(iProm) and not pUnit.isBarbarian():
					if gc.getPromotionInfo(iProm).isEquipment():
						return True
	if pPlot.isCity():
		pCity = pPlot.getPlotCity()
		if pCity.getTeam() != iTeam:
			for iBuild in range(gc.getNumBuildingInfos()):
				if pCity.getNumRealBuilding(iBuild) > 0:
					if gc.getBuildingInfo(iBuild).isEquipment():
						return True
	return False

def spellSteal(caster):
	iTeam = caster.getTeam()
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getTeam() != iTeam  and not pUnit.isBarbarian():
			for iProm in range(gc.getNumPromotionInfos()):
				if pUnit.isHasPromotion(iProm):
					if gc.getPromotionInfo(iProm).isEquipment():
 						if CyGame().getSorenRandNum(100, "Steal") <= 33:
							cf.startWar(caster.getOwner(), pUnit.getOwner(), WarPlanTypes.WARPLAN_TOTAL)
						else:
							caster.setHasPromotion(iProm, True)
							pUnit.setHasPromotion(iProm, False)
							return
	if pPlot.isCity() :
		pCity = pPlot.getPlotCity()
		if pCity.getTeam() != iTeam  and not pCity.isBarbarian():
			for iBuild in range(gc.getNumBuildingInfos()):
				if pCity.getNumRealBuilding(iBuild) > 0:
					if gc.getBuildingInfo(iBuild).isEquipment():
 						if CyGame().getSorenRandNum(100, "Steal") <= 33:
							cf.startWar(caster.getOwner(), pUnit.getOwner(), WarPlanTypes.WARPLAN_TOTAL)
						else:
							for iUnit in range(gc.getNumUnitInfos()):
								if gc.getUnitInfo(iUnit).getBuildings(iBuild):
									pCity.setNumRealBuilding(iBuild, 0)
									caster.setHasPromotion(gc.getUnitInfo(iUnit).getEquipmentPromotion(), True)
									return

def reqTakeEquipmentBuilding(caster,unit):
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_NAVAL'):
		return False
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_SIEGE'):
		return False
	if caster.getSpecialUnitType() == gc.getInfoTypeForString('SPECIALUNIT_SPELL'):
		return False
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ILLUSION')):
		return False
	iUnit = gc.getInfoTypeForString(unit)
	iProm = gc.getUnitInfo(iUnit).getEquipmentPromotion()
	if caster.isHasPromotion(iProm):
		return False
	return True

def spellTakeEquipmentBuilding(caster,unit):
	iUnit = gc.getInfoTypeForString(unit)
	pPlot = caster.plot()
	for i in range(gc.getNumBuildingInfos()):
		if gc.getUnitInfo(iUnit).getBuildings(i):
			pPlot.getPlotCity().setNumRealBuilding(i, 0)

def reqTakeEquipmentPromotion(caster,unit):
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_NAVAL'):
		return False
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_SIEGE'):
		return False
	if caster.getSpecialUnitType() == gc.getInfoTypeForString('SPECIALUNIT_SPELL'):
		return False
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ILLUSION')):
		return False
	iUnit = gc.getInfoTypeForString(unit)
	iProm = gc.getUnitInfo(iUnit).getEquipmentPromotion()
	if caster.isHasPromotion(iProm):
		return False
	iPlayer = caster.getOwner()
	pPlot = caster.plot()
	pHolder = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.getOwner() == iPlayer and pUnit.isHasPromotion(iProm)):
			pHolder = pUnit
	if pHolder == -1:
		return False
	if pHolder.isHasCasted():
		return False
	if pHolder.getUnitType() == gc.getInfoTypeForString('UNIT_BARNAXUS'):
		if iProm == gc.getInfoTypeForString('PROMOTION_PIECES_OF_BARNAXUS'):
			return False
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.isHuman() == False:
		if caster.baseCombatStr() - 2 <= pHolder.baseCombatStr():
			return False
		if gc.getUnitInfo(pHolder.getUnitType()).getFreePromotions(iProm):
			return False
	return True

def spellTakeEquipmentPromotion(caster,unit):
	iUnit = gc.getInfoTypeForString(unit)
	iProm = gc.getUnitInfo(iUnit).getEquipmentPromotion()
	iPlayer = caster.getOwner()
	pPlot = caster.plot()
	pHolder = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.getOwner() == iPlayer and pUnit.isHasPromotion(iProm) and pUnit != caster):
			pHolder = pUnit
	if pHolder != -1:
		pHolder.setHasPromotion(iProm, False)
		caster.setHasPromotion(iProm, True)

def reqTakeEquipmentUnit(caster,unit):
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_NAVAL'):
		return False
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_SIEGE'):
		return False
	if caster.getSpecialUnitType() == gc.getInfoTypeForString('SPECIALUNIT_SPELL'):
		return False
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ILLUSION')):
		return False
	iUnit = gc.getInfoTypeForString(unit)
	iProm = gc.getUnitInfo(iUnit).getEquipmentPromotion()
	if caster.isHasPromotion(iProm):
		return False
	iPlayer = caster.getOwner()
	pPlot = caster.plot()
	pHolder = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.getOwner() == iPlayer and pUnit.getUnitType() == iUnit):
			if pUnit.isDelayedDeath() == False:
				pHolder = pUnit
	if pHolder == -1:
		return False
	return True

def spellTakeEquipmentUnit(caster,unit):
	iUnit = gc.getInfoTypeForString(unit)
	iProm = gc.getUnitInfo(iUnit).getEquipmentPromotion()
	iPlayer = caster.getOwner()
	pPlot = caster.plot()
	pHolder = -1
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.getOwner() == iPlayer and pUnit.getUnitType() == iUnit):
			if pUnit.isDelayedDeath() == False:
				pHolder = pUnit
	if pHolder != -1:
		pHolder.kill(True, PlayerTypes.NO_PLAYER)

def reqTaunt(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	bValid = False
	pPlot = caster.plot()

	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pLoopPlot = CyMap().plot(iiX,iiY)
			for i in range(pLoopPlot.getNumUnits()):
				pUnit = pLoopPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					if pUnit.isAlive():
						if pPlot.isNone() == false:
							if pUnit.canMoveInto(pPlot,True,False,True):	#modified Sephi
								if not pUnit.isOnlyDefensive():
									if pUnit.getImmobileTimer() == 0:
										return True
	return bValid

def spellTaunt(caster):
	iSpell = gc.getInfoTypeForString('SPELL_TAUNT')
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	bValid = False
#added for Lanun Equipment
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GEAR_PARROT')) and caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GEAR_RUM')):
		iSpell = gc.getInfoTypeForString('SPELL_GIVE_RUM_TO_PARROT')
	elif caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GEAR_PARROT')):
		iSpell = gc.getInfoTypeForString('SPELL_LET_THE_PARROT_TALK')
#end added

	iMaxTaunts = gc.getUnitInfo(caster.getUnitType()).getTier()
	iUnitsTaunted = 0

	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pLoopPlot = CyMap().plot(iiX,iiY)
			for i in range(pLoopPlot.getNumUnits()):
				pUnit = pLoopPlot.getUnit(i)
				if eTeam.isAtWar(pUnit.getTeam()):
					if pUnit.isAlive():
						if not pUnit.isOnlyDefensive():
							if pPlot.isNone() == false:
								if pUnit.canMoveInto(pPlot,True,False,True):	#modified Sephi
									if pUnit.getImmobileTimer() == 0:
										if (pUnit.getGroup().getNumUnits()==1) or (pUnit.getGroup().getHeadUnit()!=pUnit):
											if not pUnit.isResisted(caster, iSpell):
												pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'),true)
												pUnit.attack(pPlot, False)
												iUnitsTaunted+=1
												if iUnitsTaunted==iMaxTaunts:
													return

def spellTeleport(caster,loc):
	player = caster.getOwner()
	pPlayer = gc.getPlayer(player)
	pCity = pPlayer.getCapitalCity()
	caster.setXY(pCity.getX(), pCity.getY(), false, true, true)

def reqTeachSpellcasting(caster):
	iAnimal = gc.getInfoTypeForString('UNITCOMBAT_ANIMAL')
	iBird = gc.getInfoTypeForString('SPECIALUNIT_BIRD')
	lList = []
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_AIR1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_AIR1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_BODY1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_BODY1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CHAOS1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_CHAOS1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEATH1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_DEATH1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_EARTH1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_EARTH1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENCHANTMENT1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_ENCHANTMENT1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENTROPY1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_ENTROPY1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_FIRE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_FIRE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ICE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_ICE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_LAW1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_LAW1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_LIFE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_LIFE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_METAMAGIC1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_METAMAGIC1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MIND1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_MIND1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_NATURE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_NATURE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SHADOW1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_SHADOW1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SPIRIT1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_SPIRIT1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SUN1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_SUN1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WATER1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_WATER1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_FORCE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_FORCE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CREATION1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_CREATION1')]
	if len(lList) > 0:
		pPlot = caster.plot()
		iPlayer = caster.getOwner()
		for i in range(pPlot.getNumUnits()):
			pUnit = pPlot.getUnit(i)
			if pUnit.getOwner() == iPlayer:
				if pUnit.isAlive():
					if pUnit.getUnitCombatType() != iAnimal:
						if pUnit.getSpecialUnitType() != iBird:
							for iProm in range(len(lList)):
								if not pUnit.isHasPromotion(lList[iProm]):
									return True
	return False

def spellTeachSpellcasting(caster):
	iAnimal = gc.getInfoTypeForString('UNITCOMBAT_ANIMAL')
	iBird = gc.getInfoTypeForString('SPECIALUNIT_BIRD')
	iChanneling1 = gc.getInfoTypeForString('PROMOTION_CHANNELING1')
	lList = []
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_AIR1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_AIR1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_BODY1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_BODY1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CHAOS1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_CHAOS1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEATH1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_DEATH1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_EARTH1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_EARTH1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENCHANTMENT1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_ENCHANTMENT1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENTROPY1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_ENTROPY1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_FIRE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_FIRE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ICE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_ICE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_LAW1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_LAW1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_LIFE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_LIFE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_METAMAGIC1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_METAMAGIC1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MIND1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_MIND1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_NATURE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_NATURE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SHADOW1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_SHADOW1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SPIRIT1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_SPIRIT1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SUN1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_SUN1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WATER1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_WATER1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_FORCE1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_FORCE1')]
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CREATION1')):
		lList = lList + [gc.getInfoTypeForString('PROMOTION_CREATION1')]
	pPlot = caster.plot()
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getOwner() == iPlayer:
			if pUnit.isAlive():
				if pUnit.getUnitCombatType() != iAnimal:
					if pUnit.getSpecialUnitType() != iBird:
						for iProm in range(len(lList)):
							if not pUnit.isHasPromotion(lList[iProm]):
								pUnit.setHasPromotion(lList[iProm], True)

								if not gc.getUnitInfo(pUnit.getUnitType()).getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ADEPT'):
									for iI in range(gc.getNumAdventureInfos()):
										if(pPlayer.isAdventureEnabled(iI) and not pPlayer.isAdventureFinished(iI)):
											for iJ in range(gc.getMAX_ADVENTURESTEPS()):
												if not gc.getAdventureInfo(iI).getAdventureStep(iJ)==-1:
													if(gc.getAdventureStepInfo(gc.getAdventureInfo(iI).getAdventureStep(iJ)).isGovannonsLegacy()):
														pPlayer.changeAdventureCounter(iI,iJ,1)


def spellTreetopDefence(caster):
	pPlot = caster.plot()
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getTeam() == caster.getTeam():
			pUnit.setFortifyTurns(5)

def reqTrust(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isFeatAccomplished(FeatTypes.FEAT_TRUST):
		return False
	if pPlayer.isBarbarian():
		return False
	return True

def spellTrust(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pCity = pPlayer.getCapitalCity()
	pPlayer.setFeatAccomplished(FeatTypes.FEAT_TRUST, true)

def spellTsunami(caster):
	iX = caster.getX()
	iY = caster.getY()
	for iiX in range(iX-2, iX+3, 1):
		for iiY in range(iY-2, iY+3, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if pPlot.isAdjacentToWater():
				if (iX != iiX or iY != iiY):
					for i in range(pPlot.getNumUnits()):
						pUnit = pPlot.getUnit(i)
						pUnit.doDamage(30, 75, caster, gc.getInfoTypeForString('DAMAGE_COLD'), true)
					if pPlot.getImprovementType() != -1:
						if gc.getImprovementInfo(pPlot.getImprovementType()).isPermanent() == False:
							if CyGame().getSorenRandNum(100, "Tsunami") <= 25:
								pPlot.setImprovementType(-1)
					CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SPRING'),pPlot.getPoint())

def spellUnyieldingOrder(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pCity.setOccupationTimer(0)
	pCity.changeHurryAngerTimer(-9)

def reqUpgradeDovielloWarrior(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if not pPlayer.isHuman():
		if caster.plot().getOwner()==caster.getOwner():
			if pPlayer.getNumCities() > pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_WORKER')):
				return False
	return True

def spellUpgradeDovielloWarrior(caster):
	caster.startConquestMode()	#added Sephi
	caster.setUnitAIType(gc.getInfoTypeForString('UNITAI_CITY_ATTACK'))		#added Sephi

def useBomb(caster, promotion):
	iPromotion = gc.getInfoTypeForString(promotion)
	caster.setHasPromotion(iPromotion, False)
	
def spellAwakeColdIron(caster):
	pPlayer = gc.getPlayer(caster.getOwner())

	lucianColdIron = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LUCIAN1'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
	lucianColdIron.setLevel(caster.getLevel())
	lucianColdIron.setExperience(caster.getExperience(), -1)

	for iProm in range(gc.getNumPromotionInfos()):
		if caster.isHasPromotion(iProm):
			# remove Lucians promotions so items won't be duplicated
			caster.setHasPromotion(iProm, False)
			lucianColdIron.setHasPromotion(iProm, True)

def reqVeilOfNight(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if pPlayer.getNumUnits() < 25:
			return False
		iTeam = gc.getPlayer(caster.getOwner()).getTeam()
		eTeam = gc.getTeam(iTeam)
		if eTeam.getAtWarCount(True) > 0:
			return False
	return True

def spellVeilOfNight(caster):
	iHiddenNationality = gc.getInfoTypeForString('PROMOTION_HIDDEN_NATIONALITY')
	py = PyPlayer(caster.getOwner())
	for pUnit in py.getUnitList():
		if pUnit.baseCombatStr() > 0:
			pUnit.setHasPromotion(iHiddenNationality, True)

def reqVitalize(caster):
	pPlot = caster.plot()
	if pPlot.getOwner() != caster.getOwner():
		return False
	if pPlot.isWater():
		return false
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_GRASS'):
		return False
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_BURNING_SANDS'):
		return False
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS'):
		return False
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION'):
		return False
#	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_MARSH'):
#		return False
	return True

def spellVitalize(caster):
	pPlot = caster.plot()
	if(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_SNOW')):
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_TUNDRA'),True,True)
	elif(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_TUNDRA')):
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),True,True)
	elif(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_DESERT')):
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_PLAINS'),True,True)
		if pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_SCRUB'):
			pPlot.setFeatureType(-1, -1)
	elif(pPlot.getTerrainType()==gc.getInfoTypeForString('TERRAIN_PLAINS')):
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_GRASS'),True,True)
	if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_MARSH'):
		pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_GRASS'),True,True)

def reqWane(caster):
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ANIMAL'):
		return False
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_BEAST'):
		return False
	if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ADEPT'):
		return False
	if caster.isImmortal():
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_SHADE')) >= 4:
		return False
	if caster.getExperience() < 26:
		return False
	return True

def reqWarcry(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if pPlayer.getNumUnits() < 25:
			return False
		iTeam = gc.getPlayer(caster.getOwner()).getTeam()
		eTeam = gc.getTeam(iTeam)
		if eTeam.getAtWarCount(True) == 0:
			return False
	return True

def spellWarcry(caster):
	iWarcry = gc.getInfoTypeForString('PROMOTION_WARCRY')
	py = PyPlayer(caster.getOwner())
	for pUnit in py.getUnitList():
		if pUnit.getUnitCombatType() != -1:
			pUnit.setHasPromotion(iWarcry, True)

def reqWhiteout(caster):
	pPlot = caster.plot()
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HIDDEN')):
		return False
	if pPlot.getTerrainType() != gc.getInfoTypeForString('TERRAIN_SNOW'):
		return False
	return True

def reqWildHunt(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	if pPlayer.isHuman() == False:
		iTeam = gc.getPlayer(iPlayer).getTeam()
		eTeam = gc.getTeam(iTeam)
#added Sephi
		if pPlayer.isConquestMode() == False:
			return False

		if eTeam.getAtWarCount(True) == 0:
			return False
		if pPlayer.getNumUnits() < 20:
			return False
	return True

def spellWildHunt(caster):
	iWolf = gc.getInfoTypeForString('UNIT_WOLF')
	pPlayer = gc.getPlayer(caster.getOwner())
	py = PyPlayer(caster.getOwner())
	for pUnit in py.getUnitList():
		if pUnit.baseCombatStr() > 0:
			newUnit = pPlayer.initUnit(iWolf, pUnit.getX(), pUnit.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
			if pUnit.baseCombatStr() > 3:
				i = (pUnit.baseCombatStr() - 2) / 2
				newUnit.setBaseCombatStr(2 + i)
#added Sephi
			newUnit.startConquestMode()
			newUnit.setUnitAIType(UnitAITypes.UNITAI_ATTACK_CITY)
			newUnit.joinGroup(pUnit.getGroup())
		
def spellWonder(caster):
	iCount = CyGame().getSorenRandNum(3, "Wonder") + 3
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	bCity = False
	point = pPlot.getPoint()
	if pPlot.isCity():
		bCity = True
	for i in range (iCount):
		iRnd = CyGame().getSorenRandNum(66, "Wonder")
		iUnit = -1
		if iRnd == 0:
			caster.cast(gc.getInfoTypeForString('SPELL_BLAZE'))
		if iRnd == 1:
			caster.cast(gc.getInfoTypeForString('SPELL_BLESS'))
		if iRnd == 2:
			caster.cast(gc.getInfoTypeForString('SPELL_BLINDING_LIGHT'))
		if iRnd == 3:
			caster.cast(gc.getInfoTypeForString('SPELL_BLOOM'))
		if iRnd == 4:
			caster.cast(gc.getInfoTypeForString('SPELL_BLUR'))
		if iRnd == 5:
			caster.cast(gc.getInfoTypeForString('SPELL_CHARM_PERSON'))
		if iRnd == 6:
			caster.cast(gc.getInfoTypeForString('SPELL_CONTAGION'))
		if iRnd == 7:
			caster.cast(gc.getInfoTypeForString('SPELL_COURAGE'))
		if iRnd == 8:
			caster.cast(gc.getInfoTypeForString('SPELL_CRUSH'))
		if iRnd == 9:
			caster.cast(gc.getInfoTypeForString('SPELL_DESTROY_UNDEAD'))
		if iRnd == 10:
			caster.cast(gc.getInfoTypeForString('SPELL_DISPEL_MAGIC'))
		if iRnd == 11:
			caster.cast(gc.getInfoTypeForString('SPELL_EARTHQUAKE'))
		if iRnd == 12:
			caster.cast(gc.getInfoTypeForString('SPELL_ENCHANTED_BLADE'))
		if iRnd == 13:
			CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SPELL1'),point)
			CyAudioGame().Play3DSound("AS3D_SPELL_DEFILE",point.x,point.y,point.z)
			for iX in range(pPlot.getX()-1, pPlot.getX()+2, 1):
				for iY in range(pPlot.getY()-1, pPlot.getY()+2, 1):
					pLoopPlot = CyMap().plot(iX,iY)
					if pLoopPlot.isNone() == False:
						pLoopPlot.changePlotCounter(100)
		if iRnd == 14:
			caster.cast(gc.getInfoTypeForString('SPELL_ENTANGLE'))
		if iRnd == 15:
			if caster.getOwner() != gc.getBARBARIAN_PLAYER():
				caster.cast(gc.getInfoTypeForString('SPELL_ESCAPE'))
		if iRnd == 16:
			caster.cast(gc.getInfoTypeForString('SPELL_FIREBALL'))
		if iRnd == 17:
			caster.cast(gc.getInfoTypeForString('SPELL_FLAMING_ARROWS'))
		if iRnd == 18:
			caster.cast(gc.getInfoTypeForString('SPELL_FLOATING_EYE'))
		if iRnd == 19:
			caster.cast(gc.getInfoTypeForString('SPELL_HASTE'))
		if iRnd == 20:
			caster.cast(gc.getInfoTypeForString('SPELL_HASTURS_RAZOR'))
		if iRnd == 21:
			caster.cast(gc.getInfoTypeForString('SPELL_HEAL'))
		if iRnd == 22:
			caster.cast(gc.getInfoTypeForString('SPELL_HIDE'))
		if iRnd == 23:
			caster.cast(gc.getInfoTypeForString('SPELL_LOYALTY'))
		if iRnd == 24:
			caster.cast(gc.getInfoTypeForString('SPELL_MAELSTROM'))
		if iRnd == 25:
			caster.cast(gc.getInfoTypeForString('SPELL_MORALE'))
		if iRnd == 26:
			caster.cast(gc.getInfoTypeForString('SPELL_MUTATION'))
		if iRnd == 27:
			caster.cast(gc.getInfoTypeForString('SPELL_PILLAR_OF_FIRE'))
		if iRnd == 28:
			caster.cast(gc.getInfoTypeForString('SPELL_POISONED_BLADE'))
		if iRnd == 29:
			caster.cast(gc.getInfoTypeForString('SPELL_REVELATION'))
		if iRnd == 30:
			caster.cast(gc.getInfoTypeForString('SPELL_RING_OF_FLAMES'))
		if iRnd == 31:
			caster.cast(gc.getInfoTypeForString('SPELL_RUST'))
		if iRnd == 32:
			caster.cast(gc.getInfoTypeForString('SPELL_SANCTIFY'))
		if iRnd == 33:
			caster.cast(gc.getInfoTypeForString('SPELL_SCORCH'))
		if iRnd == 34:
			caster.cast(gc.getInfoTypeForString('SPELL_SHADOWWALK'))
		if iRnd == 35:
			caster.cast(gc.getInfoTypeForString('SPELL_SPORES'))
		if iRnd == 36:
			caster.cast(gc.getInfoTypeForString('SPELL_SPRING'))
		if iRnd == 37:
			caster.cast(gc.getInfoTypeForString('SPELL_STONESKIN'))
		if iRnd == 38:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_AIR_ELEMENTAL'))
		if iRnd == 39:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_AUREALIS'))
		if iRnd == 40:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_BALOR'))
		if iRnd == 41:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_DJINN'))
		if iRnd == 42:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_EARTH_ELEMENTAL'))
		if iRnd == 43:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_EINHERJAR'))
		if iRnd == 44:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_FIRE_ELEMENTAL'))
		if iRnd == 45:
			iUnit = gc.getInfoTypeForString('UNIT_KRAKEN')
		if iRnd == 46:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_MISTFORM'))
		if iRnd == 47:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_PIT_BEAST'))
		if iRnd == 48:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_SAND_LION'))
		if iRnd == 49:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_SPECTRE'))
		if iRnd == 50:
			iUnit = gc.getInfoTypeForString('UNIT_TIGER')
		if iRnd == 51:
			iUnit = gc.getInfoTypeForString('UNIT_TREANT')
		if iRnd == 52:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_WATER_ELEMENTAL'))
		if iRnd == 53:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_WRAITH'))
		if iRnd == 54:
			caster.cast(gc.getInfoTypeForString('SPELL_TSUNAMI'))
		if iRnd == 55:
			caster.cast(gc.getInfoTypeForString('SPELL_VALOR'))
		if iRnd == 56:
			caster.cast(gc.getInfoTypeForString('SPELL_VITALIZE'))
		if iRnd == 57:
			caster.cast(gc.getInfoTypeForString('SPELL_WITHER'))
		if iRnd == 58:
			if bCity == False:
				iImprovement = pPlot.getImprovementType()
				bValid = True
				if iImprovement != -1 :
					if gc.getImprovementInfo(iImprovement).isPermanent() :
						bValid = False
				if bValid :
					pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_PENGUINS'))
					CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_WONDER_PENGUINS", ()),'',1,'Art/Interface/Buttons/Improvements/Penguins.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		if iRnd == 59:
			if bCity == False:
				iImprovement = pPlot.getImprovementType()
				bValid = True
				if iImprovement != -1 :
					if gc.getImprovementInfo(iImprovement).isPermanent() :
						bValid = False
				if bValid :
					pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_MUSHROOMS'))
					CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_WONDER_MUSHROOMS", ()),'',1,'Art/Interface/Buttons/Improvements/Mushrooms.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		if iRnd == 60:
			for iProm in range(gc.getNumPromotionInfos()):
				if caster.isHasPromotion(iProm):
					if gc.getPromotionInfo(iProm).isRace():
						caster.setHasPromotion(iProm, False)
			caster.setUnitArtStyleType(gc.getInfoTypeForString('UNIT_ARTSTYLE_BABOON'))
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_WONDER_BABOON", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas3.dds,1,9',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			if pPlayer.isHuman():
				t = "TROPHY_FEAT_BABOON"
				if not CyGame().isHasTrophy(t):
					CyGame().changeTrophyValue(t, 1)
		if iRnd == 61:
			CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SPELL1'),point)
			CyAudioGame().Play3DSound("AS3D_SPELL_SANCTIFY",point.x,point.y,point.z)
			for iX in range(pPlot.getX()-2, pPlot.getX()+3, 1):
				for iY in range(pPlot.getY()-2, pPlot.getY()+3, 1):
					pLoopPlot = CyMap().plot(iX,iY)
					if pLoopPlot.isNone() == False:
						pLoopPlot.changePlotCounter(-100)
		if iRnd == 62:
			iUnit = gc.getInfoTypeForString('UNIT_SPIDERKIN')
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_WONDER_SPIDERKIN", ()),'',1,', ,Art/Interface/Buttons/Units/Units_atlas4.dds,7,7',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		if iUnit != -1:
			newUnit = pPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SUMMONER')):
				newUnit.setDuration(2)
			else:
				newUnit.setDuration(1)
		if iRnd == 63:
			caster.cast(gc.getInfoTypeForString('SPELL_SLOW'))
		if iRnd == 64:
			caster.cast(gc.getInfoTypeForString('SPELL_SUMMON_ICE_ELEMENTAL'))
		if iRnd == 65:
			caster.cast(gc.getInfoTypeForString('SPELL_SNOWFALL'))

def reqWorldbreak(caster):
	if CyGame().getGlobalCounter() == 0:
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if CyGame().getGlobalCounter() < 50:
			return False
	return True

def spellWorldbreak(caster):
	iCounter = CyGame().getGlobalCounter()
	iFire = gc.getInfoTypeForString('DAMAGE_FIRE')
	iForest = gc.getInfoTypeForString('FEATURE_FOREST')
	iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
	iPillar = gc.getInfoTypeForString('EFFECT_PILLAR_OF_FIRE')
	iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
	for i in range (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		if pPlot.isOwned():
			if pPlot.getOwner() == caster.getOwner():
				continue
		if pPlot.isCity():
			if CyGame().getSorenRandNum(100, "Worldbreak") <= (iCounter / 4):
				cf.doCityFire(pPlot.getPlotCity())
		for i in range(pPlot.getNumUnits()):
			pUnit = pPlot.getUnit(i)
			pUnit.doDamage(iCounter, 100, caster, iFire, false)
			CyEngine().triggerEffect(iPillar,pPlot.getPoint())
		if (pPlot.getFeatureType() == iForest or pPlot.getFeatureType() == iJungle):
			if pPlot.getImprovementType() == -1:
				if CyGame().getSorenRandNum(100, "Flames Spread") <= (iCounter / 4):
					pPlot.setImprovementType(iSmoke)

def atRangeGuardian(pCaster, pPlot):
	if pPlot.getNumUnits() == 0:
		if CyGame().getStartTurn() + 20 < CyGame().getGameTurn(): #fixes a problem if units spawn next to the gargoyle
			iPlayer = pCaster.getOwner()
			if iPlayer != gc.getBARBARIAN_PLAYER() and iPlayer != gc.getANIMAL_PLAYER() and iPlayer != gc.getWILDMANA_PLAYER():
				bPlayer = gc.getPlayer(gc.getANIMAL_PLAYER())
				iUnit = gc.getInfoTypeForString('UNIT_GARGOYLE')
				newUnit1 = bPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit1.setUnitAIType(gc.getInfoTypeForString('UNITAI_ANIMAL'))
#				newUnit1.setOriginPlot(newUnit1.plot())
				newUnit2 = bPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit2.setUnitAIType(gc.getInfoTypeForString('UNITAI_ANIMAL'))
#				newUnit2.setOriginPlot(newUnit2.plot())
				newUnit3 = bPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit3.setUnitAIType(gc.getInfoTypeForString('UNITAI_ANIMAL'))
#				newUnit3.setOriginPlot(newUnit3.plot())
				CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_GUARDIAN", ()),'',1,gc.getUnitInfo(iUnit).getButton(),ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
				pPlot.setPythonActive(False)

def onMoveAncientForest(pCaster, pPlot):
	if pPlot.isOwned():
		if pPlot.getNumUnits() == 1:
			if pCaster.isFlying() == False:
				iChance = gc.getDefineINT('TREANT_SPAWN_CHANCE')
				if pPlot.isBeingWorked():
					pCity = pPlot.getWorkingCity()
					if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_LEAVES')) > 0:
						iChance = iChance * 3
					if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_SONG_OF_AUTUMN')) > 0:
						iChance = iChance * 3
				if CyGame().getSorenRandNum(100, "Treant Spawn Chance") <= iChance:
					pPlayer = gc.getPlayer(pCaster.getOwner())
					p2Player = gc.getPlayer(pPlot.getOwner())
					eTeam = gc.getTeam(pPlayer.getTeam())
					i2Team = p2Player.getTeam()
					if (eTeam.isAtWar(i2Team) and p2Player.getStateReligion() == gc.getInfoTypeForString('RELIGION_FELLOWSHIP_OF_LEAVES')):
						for i in range(pPlot.getNumUnits()):
							p2Unit = pPlot.getUnit(i)
							p2Plot = cf.findClearPlot(p2Unit, -1)
							if p2Plot != -1:
								p2Unit.setXY(p2Plot.getX(),p2Plot.getY(), false, true, true)
								p2Unit.finishMoves()
						if pPlot.getNumUnits() == 0:
							newUnit = p2Player.initUnit(gc.getInfoTypeForString('UNIT_TREANT'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
							newUnit.setDuration(3)
							newUnit.AI_setGroupflag(43)	# 43=GROUPFLAG_SUICIDE_SUMMON
							CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_TREANT_ENEMY",()),'AS2D_FEATUREGROWTH',1,', ,Art/Interface/Buttons/Units/Units_atlas4.dds,7,9',ColorTypes(7),newUnit.getX(),newUnit.getY(),True,True)
							CyInterface().addMessage(pPlot.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_TREANT",()),'AS2D_FEATUREGROWTH',1,', ,Art/Interface/Buttons/Units/Units_atlas4.dds,7,9',ColorTypes(8),newUnit.getX(),newUnit.getY(),True,True)

#	Changed in Frozen: TC01
#	Frozen units should not be damaged by Blizzards. If the module is loaded, they will not be, otherwise, I use the original code. The line I changed is marked.
def onMoveBlizzard(pCaster, pPlot):
	iFrozen = gc.getInfoTypeForString('CIVILIZATION_FROZEN')
	iIllians = gc.getInfoTypeForString('CIVILIZATION_ILLIANS')
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WINTERBORN')) == False:
		if pCaster.getCivilizationType() != iFrozen:
			if pCaster.getCivilizationType() != iIllians:
				pCaster.doDamage(10, 50, pCaster, gc.getInfoTypeForString('DAMAGE_COLD'), false)
#End of Frozen


def onMoveLetumFrigus(pCaster, pPlot):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	if pPlayer.isHuman():
		iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_LETUM_FRIGUS')
		triggerData = pPlayer.initTriggeredData(iEvent, true, -1, pCaster.getX(), pCaster.getY(), pCaster.getOwner(), -1, -1, -1, -1, -1)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_LETUM_FRIGUS", ()),'',1,'Art/Interface/Buttons/Improvements/Letum Frigus.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
		pPlot.setPythonActive(False)

def onMovePoolOfTears(pCaster, pPlot):
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CRIPPLED')):
		pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRIPPLED'), false)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_POOL_OF_TEARS_DISEASED",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Pool of Tears.dds',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DISEASED')):
		pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DISEASED'), false)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_POOL_OF_TEARS_DISEASED",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Pool of Tears.dds',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_PLAGUED')):
		pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PLAGUED'), false)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_POOL_OF_TEARS_PLAGUED",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Pool of Tears.dds',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED')):
		pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), false)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_POOL_OF_TEARS_POISONED",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Pool of Tears.dds',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WITHERED')):
		pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WITHERED'), false)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_POOL_OF_TEARS_WITHERED",()),'AS2D_FEATUREGROWTH',1,'Art/Interface/Buttons/Improvements/Pool of Tears.dds',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
		
def onMoveWallofLight(pCaster, pPlot):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	p2Player = gc.getPlayer(pPlot.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())
	i2Team = p2Player.getTeam()
	if eTeam.isAtWar(i2Team):
		for i in range(pPlot.getNumUnits()):
			pCaster.doDamage(10, 50, pCaster, gc.getInfoTypeForString('DAMAGE_FIRE'), false)


def voteFundDissidents():
	iOvercouncil = gc.getInfoTypeForString('DIPLOVOTE_OVERCOUNCIL')
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.isAlive():
			if pPlayer.isFullMember(iOvercouncil):
				for pyCity in PyPlayer(iPlayer).getCityList() :
					if CyGame().getSorenRandNum(100, "Fund Dissidents") < 50:
						pCity = pyCity.GetCy()
						pCity.changeHurryAngerTimer(1 + CyGame().getSorenRandNum(3, "Fund Dissidents"))
						
def voteCutSevenPinesOC():
	iOvercouncil = gc.getInfoTypeForString('DIPLOVOTE_OVERCOUNCIL')
	iValue = 5 * gc.getGame().getElapsedGameTurns()
	
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.isAlive():
			if pPlayer.isFullMember(iOvercouncil):
				pPlayer.changeGlobalYield(YieldTypes.YIELD_LUMBER, iValue)

def voteCutSevenPines():
	iOvercouncil = gc.getInfoTypeForString('DIPLOVOTE_UNDERCOUNCIL')
	iValue = 5 * gc.getGame().getElapsedGameTurns()
	
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.isAlive():
			if pPlayer.isFullMember(iOvercouncil):
				pPlayer.changeGlobalYield(YieldTypes.YIELD_LUMBER, iValue)

def voteSlaveTrade():
	iOvercouncil = gc.getInfoTypeForString('DIPLOVOTE_UNDERCOUNCIL')
	iValue = 5 * gc.getGame().getElapsedGameTurns()
	
	for iPlayer in range(gc.getMAX_PLAYERS()):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.isAlive():
			if pPlayer.isFullMember(iOvercouncil):
				pPlayer.changeGlobalYield(YieldTypes.YIELD_STONE, iValue)

				
#WILDMANA

def reqBirth(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()

	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FALLOW')):
		return false
	if pCity.getPopulation() > 7:
		return false

	return true

def spellFestivalofSun(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.changeGoldenAgeTurns(CyGame().goldenAgeLength())
	
def reqFertility(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = caster.plot()

	pBonus = pPlot.getBonusType(-1)
	if pBonus == -1:
		return false

	iWheat = gc.getInfoTypeForString('BONUS_WHEAT')
	iRice = gc.getInfoTypeForString('BONUS_RICE')
	iCorn = gc.getInfoTypeForString('BONUS_CORN')

	iCow = gc.getInfoTypeForString('BONUS_COW')
	iSheep = gc.getInfoTypeForString('BONUS_SHEEP')
 	iPig = gc.getInfoTypeForString('BONUS_PIG')

	if pBonus == iWheat or pBonus == iRice or pBonus == iCorn or pBonus == iCow or pBonus == iSheep or pBonus == iPig:
		return true

	return false

def spellFertility(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pPlot = caster.plot()

	pBonus = pPlot.getBonusType(-1)

	iWheat = gc.getInfoTypeForString('BONUS_WHEAT')
	iRice = gc.getInfoTypeForString('BONUS_RICE')
	iCorn = gc.getInfoTypeForString('BONUS_CORN')

	iCow = gc.getInfoTypeForString('BONUS_COW')
	iSheep = gc.getInfoTypeForString('BONUS_SHEEP')
 	iPig = gc.getInfoTypeForString('BONUS_PIG')

 	if pBonus == iWheat:
 		pPlot.setBonusType(iRice)
 		return
 	if pBonus == iRice:
 		pPlot.setBonusType(iCorn)
 		return
  	if pBonus == iCorn:
  		pPlot.setBonusType(iWheat)
 		return

  	if pBonus == iCow:
 		pPlot.setBonusType(iSheep)
 		return
 	if pBonus == iSheep:
 		pPlot.setBonusType(iPig)
 		return
 	if pBonus == iPig:
 		pPlot.setBonusType(iCow)
 		return

def spellReadGearSpellbook(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())

	iChange = 10
	iChange *= 100 - pCaster.getExperience()
	iChange /= 100

	if iChange < 1:
		iChange = 1

	pCaster.changeExperience(iChange, 200000, false, false, false)

#Faeries
def effectDusted(caster):
	caster.doDamageNoCaster(15, 50, gc.getInfoTypeForString('DAMAGE_POISON'), false)

def spellPotionOfClarity(caster):
	caster.changeExperience(30, -1, false, false, false)

def	spellPotionStrong(caster):
	caster.changeBaseCombatFromPotion(1)
	caster.changeBaseCombatDefenseFromPotion(1)
	caster.changeStrengthPotionsUsed(1)

def effectBleeding(caster):
	if caster.getDamage() > 0:
		caster.doDamageNoCaster(10, 100, gc.getInfoTypeForString('DAMAGE_PHYSICAL'), false)

def effectBrokenMinded(caster):
	caster.doDamageNoCaster(10, 90, gc.getInfoTypeForString('DAMAGE_UNHOLY'), false)
	
def effectOnFire(caster):
	caster.doDamageNoCaster(10, 90, gc.getInfoTypeForString('DAMAGE_FIRE'), false)

def reqSilenced(caster):
	caster.setHasCasted(True)
	
def reqStunned(caster):
	#caster.finishMoves()
	#caster.setHasCasted(True)
	CyMessageControl().sendModNetMessage(CvUtil.UnitStun,caster.getOwner(),caster.getID(),1,1)
	
def reqWinterfeast(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) == gc.getInfoTypeForString('CIVIC_WINTER_COURT'):
		if (pCity.unhappyLevel(0)>0):
			if caster.getTeam() == pCity.getTeam():
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

def effectKillOgres(caster):
	caster.kill(True, PlayerTypes.NO_PLAYER)
