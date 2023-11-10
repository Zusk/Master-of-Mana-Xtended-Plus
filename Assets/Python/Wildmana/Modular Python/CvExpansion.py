import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()



def WildmanaGuardianDefeated(argsList):
	'Combat Result'
	pWinner,pLoser = argsList
	pPlayerW = gc.getPlayer(pWinner.getOwner())
	pPlayerL = gc.getPlayer(pLoser.getOwner())

	eTeam = gc.getTeam(pWinner.getTeam())
	eTech = -1

	#Wildmana Guardian Defeated
	if pLoser.getOwner()!=gc.getWILDMANA_PLAYER():
		if pLoser.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MANA_DEFENDER')):
			CyInterface().addMessage(pWinner.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_WILDMANA_GUARDIAN_DEFEATED",()),'',1,gc.getUnitInfo(pLoser.getUnitType()).getButton(),ColorTypes(8),pLoser.getX(),pLoser.getY(),True,True)
			pPlayerW.changeArcane(2000)

	#Necromancy Effect
	if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CURSED_BRINGER')):
		if pLoser.isAlive():
#			if not pLoser.isImmuneToCapture():
#Xtended
			if CyGame().getSorenRandNum(100, "Necromancy") > 50:
				pPlayerW.initUnit(gc.getInfoTypeForString('UNIT_SKELETON_DUMMY'),pWinner.getX(),pWinner.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
#TODO: ability to convert stronger undead as better Spellresearch is unlocked

	#Learning by Doing
	iOrder = gc.getInfoTypeForString('RELIGION_THE_ORDER')
	iAV = gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL')
	if (pPlayerW.getStateReligion()==iOrder and pPlayerL.getStateReligion()==iAV) or (pPlayerL.getStateReligion()==iOrder and pPlayerW.getStateReligion()==iAV):
		iFanatic=gc.getInfoTypeForString('PROMOTION_FANATIC')
		if not pWinner.isHasPromotion(iFanatic):
			if CyGame().getSorenRandNum(100, "Fanatic born in Battle")<5:
				pWinner.setHasPromotion(iFanatic,true)
				pWinner.displayMessagePromotionGainedInBattle(iFanatic)

	iBard = gc.getInfoTypeForString('PROMOTION_CLASS_BARD')
	if pWinner.isHasPromotion(iBard):
		pPlayerW.changeGlobalCulture(20 + (pWinner.getLevel()*5))

#	iGladiator = gc.getInfoTypeForString('PROMOTION_GLADIATOR_HERO')
#	if pWinner.isHasPromotion(iGladiator):
#		pPlayerW.changeGlobalCulture(50 + (pWinner.getLevel()*5))
	
#	iAssimilate = gc.getInfoTypeForString('PROMOTION_ASSIMILATE_MIND')
#	if pWinner.isHasPromotion(iAssimilate):
		#researcg can be -1 so overflow
#		eTeam.changeResearchProgress(pPlayerW.getCurrentResearch(),(8*pWinner.getLevel()),pPlayerW.getID())


#TRAITS

def TraitEffectonUnitCreated(argsList):

	'Unit Completed'
	unit = argsList[0]
	player = PyPlayer(unit.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())

	if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SUMMONER')):
		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SUMMON')):
#			unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), True)
			unit.changeExperience(30, -1, False, False, False)


def TraitEffectAfterCombat(argsList):
	'Combat Result'
	pWinner,pLoser = argsList
	pPlayerW = gc.getPlayer(pWinner.getOwner())
	pPlayerL = gc.getPlayer(pLoser.getOwner())

	#Redeeming Trait
	if pPlayerW.hasTrait(gc.getInfoTypeForString('TRAIT_REDEEMING')):
		if pPlayerL.getAlignment()==gc.getInfoTypeForString('ALIGNMENT_EVIL') or pPlayerL.isBarbarian():
			if CyGame().getSorenRandNum(25, "trigger Golden Age") <= (0 + pLoser.getLevel()):
				if not pPlayerW.isGoldenAge():
					pPlayerW.changeGoldenAgeTurns(3)

#END TRAITS


#BEGIN Buildings and Wonders

def BuildingCreated(argsList):
	'Building Completed'
	pCity, iBuildingType = argsList
	player = pCity.getOwner()
	pPlayer = gc.getPlayer(player)
	pPlot = pCity.plot()
	game = gc.getGame()
	iBuildingClass = gc.getBuildingInfo(iBuildingType).getBuildingClassType()

	if (iBuildingType==gc.getInfoTypeForString('BUILDING_TREE_OF_SUCELLUS')):

		iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
		iForest = gc.getInfoTypeForString('FEATURE_FOREST')
		iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
		iScrub = gc.getInfoTypeForString('FEATURE_SCRUB')
		iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
		iAncient = gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')

		iX = pCity.getX()
		iY = pCity.getY()
		for iiX in range(iX-3, iX+4, 1):
			for iiY in range(iY-3, iY+4, 1):
				pLoopPlot = CyMap().plot(iiX,iiY)
				if not pLoopPlot.isNone():
					if not pLoopPlot.isWater():
						iFeature = pLoopPlot.getFeatureType()
						if iFeature == iForest:
							pLoopPlot.setFeatureType(iAncient, 0)
						if iFeature == iJungle:
							pLoopPlot.setFeatureType(iAncient, 0)
						if iFeature == iFlames:
							pLoopPlot.setFeatureType(-1, -1)
						iImprovement = pLoopPlot.getImprovementType()
						if iImprovement == iSmoke:
							pLoopPlot.setImprovementType(-1)

	elif (iBuildingType==gc.getInfoTypeForString('BUILDING_ENGINEERING_CORPS')):
		pPlayer.setHasTrait(gc.getInfoTypeForString('TRAIT_ENGINEER'),true)

# Water Well
	elif iBuildingType == gc.getInfoTypeForString( 'BUILDING_WATER_WELL' ):
		pPlot.setFeatureType(gc.getInfoTypeForString( "FEATURE_UNDERGROUND_WATER" ), 1)
# End Water Well

	elif iBuildingType==gc.getInfoTypeForString('BUILDING_COURTHOUSE'):
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_AGNOSTIC')):
			if CyGame().getSorenRandNum(100, "pick Alignment") <= 20:
				iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_PICK_ALIGNMENT')
				pPlayer.trigger(iEvent)

#END Buildings and Wonders
