import CvUtil
from CvPythonExtensions import *

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

def onMoveRemnantsOfPatria(pCaster, pPlot):
	pPlayer = gc.getPlayer(pCaster.getOwner())
	if pPlayer.isFeatAccomplished(FeatTypes.FEAT_VISIT_REMNANTS_OF_PATRIA) == false:
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_ELOHIM"):
			iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_REMNANTS_OF_PATRIA_ELOHIM')
			triggerData = pPlayer.initTriggeredData(iEvent, true, -1, pCaster.getX(), pCaster.getY(), -1, -1, -1, -1, -1, -1)
			pPlayer.setFeatAccomplished(FeatTypes.FEAT_VISIT_REMNANTS_OF_PATRIA, True)
		elif pPlayer.getCivilizationType() == gc.getInfoTypeForString("CIVILIZATION_SCIONS"):
			iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_REMNANTS_OF_PATRIA_SCIONS')
			triggerData = pPlayer.initTriggeredData(iEvent, true, -1, pCaster.getX(), pCaster.getY(), -1, -1, -1, -1, -1, -1)
			pPlayer.setFeatAccomplished(FeatTypes.FEAT_VISIT_REMNANTS_OF_PATRIA, True)

# Scions start
def spellArawnsCall(caster):
	iSpell = gc.getInfoTypeForString('SPELL_ARAWN_BECKONS')
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iCalled = gc.getInfoTypeForString('PROMOTION_CALLED')
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.isHasPromotion(iUndead) == true:
					iResist = pUnit.getResistChance(caster, iSpell)
					if CyGame().getSorenRandNum(100, "Arawan Beckons resist roll") > iResist:
						pUnit.setHasPromotion(iCalled, True)

def reqArawnsCall(caster):
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iScion = gc.getInfoTypeForString('CIVILIZATION_SCIONS')
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = pPlayer.getTeam()
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.isHasPromotion(iUndead) == true:
					if pUnit.getCivilizationType() != iScion:
						if pPlayer.isHuman():
							return True
						p2Player = gc.getPlayer(pUnit.getOwner())
						e2Team = gc.getTeam(p2Player.getTeam())
						if e2Team.isAtWar(eTeam):
							return True
	return False

# Spell for Awakened and Reborn
def reqAddPop(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	if (pCity.happyLevel() - pCity.unhappyLevel(0)) < 1:
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pCity.getOwner() != caster.getOwner():
		return False
	if pPlayer.isHuman() == false:
		if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_AWAKENED'))+pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_REBORN'))==1:
			if pPlayer.getNumCities()<5 and pCity.getPopulation()/2>pPlayer.getNumCities():
				return False
	return True

# Used when some Pelemoc spells start a war.
def spellCB(caster):
	pPlot = caster.plot()
	cf.startWar(caster.getOwner(), pPlot.getOwner(), WarPlanTypes.WARPLAN_LIMITED)

# Many of Pelemoc's spells require that he be in another civ's city.
def reqStrife(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	if pCity.getOwner() == caster.getOwner():
		return False
	return True

# req for Succor
def reqSuccor(caster):
	iUH = 0
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	iUH = pCity.goodHealth() - pCity.badHealth(False)
	if pCity.getOwner() == caster.getOwner():
		return False
	if iUH > -2:
		return False
	return True

# Pelemoc's Subversion spell.
def spellSubversion(caster,loc):
	pPlot = caster.plot()
	if CyGame().getSorenRandNum(4, "subversion war odds") < 1:
		cf.startWar(caster.getOwner(), pPlot.getOwner(), WarPlanTypes.WARPLAN_LIMITED)

	player = caster.getOwner()
	pPlayer = gc.getPlayer(player)
	pCity = pPlayer.getCapitalCity()

	iReborn = gc.getInfoTypeForString('UNIT_REBORN')
	spawnUnit = pPlayer.initUnit(iReborn, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

# Pelemoc's Bring Out Your I'm Not Dead Yet spell.
def spellSuccor(caster,loc):
	pPlot = caster.plot()

	player = caster.getOwner()
	pPlayer = gc.getPlayer(player)
	pCity = pPlayer.getCapitalCity()

	iReborn = gc.getInfoTypeForString('UNIT_REBORN')
	spawnUnit = pPlayer.initUnit(iReborn, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

# Many spells also require that a particular building not already be present.  This one's for Emperor's Claim.
def reqEclaim(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	if pCity.getOwner() == caster.getOwner():
		return False
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_EMPERORS_CLAIM')) > 0:
		return False
	return True

# Req. for Rousing Oration.
def reqOrat(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	if pCity.getOwner() == caster.getOwner():
		return False
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_ROUSING_ORATION')) > 0:
		return False
	return True

# Req for Poison Words
def reqPwords(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	if pCity.getOwner() == caster.getOwner():
		return False
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_POISON_WORDS')) > 0:
		return False
	return True

# Req for Corrupt Guild
def reqPac(caster):
	pCity = caster.plot().getPlotCity()
	pPlayer = gc.getPlayer(caster.getOwner())
	if pCity.getOwner() == caster.getOwner():
		return False
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_CORRUPT_GUILD')) > 0:
		return False
	return True

# Req for Melante's Estate.
def reqMelante(caster):
	pCity = caster.plot().getPlotCity()
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALACE_SCIONS')) > 0:
		return False
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_MELANTE')) > 0:
		return False
	return True


# req for Emperor's Mark.
def reqEmark(caster):
	pCity = caster.plot().getPlotCity()
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_EMPERORS_MARK')) > 0:
		return False
	return True

# Redactor's Haunted Land creating spell.   - Doesn't change mountains, floodplinas.  Doesn't always work on forests, jungles.
def spellHL(caster):
	pPlot = caster.plot()
	if pPlot.isOwned():
		cf.startWar(caster.getOwner(), pPlot.getOwner(), WarPlanTypes.WARPLAN_LIMITED)

	iX = caster.getX()
	iY = caster.getY()
#	pPlot = CyMap().plot(iX,iY)
	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if pPlot.isWater() == False:
				if pPlot.isCity() == False:
					if pPlot.isPeak() == False:
						if (pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_VOLCANO')) == False:
							if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')) == False:
								if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT') and (CyGame().getSorenRandNum(3, "HL destroy A_Forest Chance") > 0)) == False:
									if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') and (CyGame().getSorenRandNum(3, "HL destroy Forest Chance") == 0)) == False:
										if (pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE') and (CyGame().getSorenRandNum(3, "HL destroy jungle Chance") == 0)) == False:
											pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_HAUNTED_LANDS'),0)
#								if pPlot.isOwned():
#									startWar(caster.getOwner(), pPlot.getOwner())


# Ghostwalker's HL spell.
def spellHL2(caster):
	pPlot = caster.plot()
	pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_HAUNTED_LANDS'),0)

	if pPlot.isOwned():
		cf.startWar(caster.getOwner(), pPlot.getOwner(), WarPlanTypes.WARPLAN_LIMITED)

# Keeps HL spell from being cast on water or in a HL tile.
def reqHL(caster):
	iHaunted= gc.getInfoTypeForString('FEATURE_HAUNTED_LANDS')
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	if (pPlot.getFeatureType() == iHaunted):
		return False
	if pPlot.isWater():
		return False
#	iNewForest= gc.getInfoTypeForString('FEATURE_FOREST_NEW')
#	iForest= gc.getInfoTypeForString('FEATURE_FOREST')
#	iAncientForest= gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')
#	iJungle= gc.getInfoTypeForString('FEATURE_JUNGLE')
#	if pPlot.getTerrainType() != gc.getInfoTypeForString('TERRAIN_GRASS') and pPlot.getTerrainType() != gc.getInfoTypeForString('TERRAIN_PLAINS') and pPlot.getTerrainType() != gc.getInfoTypeForString('TERRAIN_TUNDRA'):
#		return False
#	if (pPlot.getFeatureType() == iNewForest or pPlot.getFeatureType() == iForest or pPlot.getFeatureType() == iAncientForest or pPlot.getFeatureType() == iJungle):
#		return False
#	if (pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_SVARTALFAR') and pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_LJOSALFAR')):
#		if pPlot.isCity():
#			return False
		if pPlot.getImprovementType() != -1:
			return False
	if pPlot.isCity():
		return False
	return True

# Req for Give Gift spell.
def reqGiveGift(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iGhastlyDragoon = gc.getInfoTypeForString('PROMOTION_GHASTLY_DRAGOON')
	iBroken = gc.getInfoTypeForString('PROMOTION_BROKEN')
	iNaval = gc.getInfoTypeForString('UNITCOMBAT_NAVAL')
	iMounted = gc.getInfoTypeForString('UNITCOMBAT_MOUNTED')
	iSiege = gc.getInfoTypeForString('UNITCOMBAT_SIEGE')
	iGolem = gc.getInfoTypeForString('PROMOTION_GOLEM')
	iScion = gc.getInfoTypeForString('CIVILIZATION_SCIONS')
	iRedLady = gc.getInfoTypeForString('UNIT_KORRINA_RED_LADY')
	iBlackLady = gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY')
	iWhiteLady = gc.getInfoTypeForString('UNIT_KORRINA_WHITE_LADY')
	iKorrina = gc.getInfoTypeForString('UNIT_KORRINA_PROTECTOR')
	iHauntKorrina = gc.getInfoTypeForString('UNIT_HAUNT_KORRINA')
	lKorrinas = [iRedLady, iBlackLady, iWhiteLady, iKorrina, iHauntKorrina]
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() or pUnit.getUnitCombatType() == iSiege or pUnit.getRace() == iGolem or pUnit.getUnitCombatType() == iNaval):
			if not pUnit.isHasPromotion(iUndead):
				if pUnit.getCivilizationType() == iScion:
					if pUnit.getUnitType() not in lKorrinas:
						return True
		if pUnit.getUnitCombatType() == iMounted and not pUnit.isHasPromotion(iBroken):
			return True
	return False

# Redactor spell that makes some non-undead units Undead and grants 'Ghastly Dragoon' to mounted units
def spellGiveGift(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
	iGhastlyDragoon = gc.getInfoTypeForString('PROMOTION_GHASTLY_DRAGOON')
	iBroken = gc.getInfoTypeForString('PROMOTION_BROKEN')
	iNaval = gc.getInfoTypeForString('UNITCOMBAT_NAVAL')
	iMounted = gc.getInfoTypeForString('UNITCOMBAT_MOUNTED')
	iSiege = gc.getInfoTypeForString('UNITCOMBAT_SIEGE')
	iGolem = gc.getInfoTypeForString('PROMOTION_GOLEM')
	iScion = gc.getInfoTypeForString('CIVILIZATION_SCIONS')
	iRedLady = gc.getInfoTypeForString('UNIT_KORRINA_RED_LADY')
	iBlackLady = gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY')
	iWhiteLady = gc.getInfoTypeForString('UNIT_KORRINA_WHITE_LADY')
	iKorrina = gc.getInfoTypeForString('UNIT_KORRINA_PROTECTOR')
	iHauntKorrina = gc.getInfoTypeForString('UNIT_HAUNT_KORRINA')
	lKorrinas = [iRedLady, iBlackLady, iWhiteLady, iKorrina, iHauntKorrina]
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if (pUnit.isAlive() or pUnit.getUnitCombatType() == iSiege or pUnit.getRace() == iGolem or pUnit.getUnitCombatType() == iNaval):
			if pUnit.getCivilizationType() == iScion:
				if pUnit.getUnitType() not in lKorrinas:
					pUnit.setHasPromotion(iUndead, True)
		if pUnit.getUnitCombatType() == iMounted and not pUnit.isHasPromotion(iBroken):
			pUnit.setHasPromotion(iGhastlyDragoon, True)

# Ghostwalkers can only become Haunts on a HL tile.
def reqHaunt(caster):
	pPlot = caster.plot()
	if pPlot.getFeatureType() != gc.getInfoTypeForString('FEATURE_HAUNTED_LANDS'):
		return False
	return True

# Changes Ghostwalker to a Haunt.
def spellHaunt(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_HAUNT'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	for i in range(gc.getNumPromotionInfos()):
		if gc.getPromotionInfo(i).isEquipment() == False:
			newUnit.setHasPromotion(i, caster.isHasPromotion(i))
	newUnit.setDamage(caster.getDamage(), caster.getOwner())
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HAUNT'), True)
	newUnit.setLevel(caster.getLevel())
	newUnit.setExperience(caster.getExperience(), -1)# Changes Ghostwalker to a Haunt.

def reqHauntKorrina(caster):
	if caster.isMadeAttack():
		return False
	return True

def spellHauntKorrina(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_HAUNT_KORRINA'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	for i in range(gc.getNumPromotionInfos()):
		if gc.getPromotionInfo(i).isEquipment() == False:
			newUnit.setHasPromotion(i, caster.isHasPromotion(i))
	newUnit.setDamage(caster.getDamage(), caster.getOwner())
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HAUNT'), True)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_INVISIBLE'), True)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STEALTH'), False)
	newUnit.setLevel(caster.getLevel())
	newUnit.setExperience(caster.getExperience(), -1)
	newUnit.setHasCasted(True)

def spellKorrinaHaunt(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY'), caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	for i in range(gc.getNumPromotionInfos()):
		if gc.getPromotionInfo(i).isEquipment() == False:
			newUnit.setHasPromotion(i, caster.isHasPromotion(i))
	newUnit.setDamage(caster.getDamage(), caster.getOwner())
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HAUNT'), False)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_INVISIBLE'), False)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STEALTH'), True)
	newUnit.setLevel(caster.getLevel())
	newUnit.setExperience(caster.getExperience(), -1)
	newUnit.setHasCasted(True)
	newUnit.setMadeAttack(True)

# MoP don't always use suicide attacks, and never aginast workers and hawks.
def mopsuicide(pCaster, pOpponent):
	if (pOpponent.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_HAWK')):
		return
	if (pOpponent.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_WORKER')):
		return
	if CyGame().getSorenRandNum(3, "MoP suicide attack Check") > 1:
		pCaster.kill(True, 0)

# Req for Feeding Creepers.
def reqPlant(caster):
	return False
	pPlot = caster.plot()
	iCreeper = gc.getInfoTypeForString('UNIT_CREEPER')
	iPPromo = gc.getInfoTypeForString('PROMOTION_FED_CREEPER')
	iPSent = gc.getInfoTypeForString('PROMOTION_SENTRY2')
	if caster.getExperience() < 4:
			return False
	for i in range(pPlot.getNumUnits()):
		pUnit = pPlot.getUnit(i)
		if pUnit.getUnitType() == iCreeper:
			if pUnit.isHasPromotion(iPSent):
				return True

#Alcinus Keep

def reqBuildKeep(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()

	if pPlot.getPlotCity().isNone==False:
		if pPlot.getPlotCity().isCapital():
			return False

	if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_KEEP_MAIN'))>0:
		return False

	return True

def reqSpiralGate(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())

	if pPlayer.isHuman() == False:
		iCastingChance = -1
		if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ARCANE_LORE')) == True:
			iCastingChance += 1
		if eTeam.isHasTech(gc.getInfoTypeForString('TECH_RELIGIOUS_LAW')) == True:
			iCastingChance += 2
		if eTeam.isHasTech(gc.getInfoTypeForString('TECH_MITHRIL_WORKING')) == True:
			iCastingChance += 2
		if eTeam.isHasTech(gc.getInfoTypeForString('TECH_GUILDS')) == True:
			iCastingChance += 2
		if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ENGINEERING')) == True:
			iCastingChance += 2
		if CyGame().getSorenRandNum(199, "AI chance for Spiral Gate") < iCastingChance:
			return True

	return True

# The Breach (Themoch spell)
def spellSpiralGate(argslist):
	if isinstance( argslist, tuple ):
		makeHelpText = True
		eSpell, caster = argslist
	else:
		caster = argslist
		makeHelpText=False
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	eTeam = gc.getTeam(pPlayer.getTeam())

	bonusCategory = dict()
	bonusCategory['false laroth'] = dict()
	bonusCategory['archer'] = dict()
	bonusCategory['adept'] = dict()
	bonusCategory['melee'] = dict()
	bonusCategory['recon'] = dict()
	bonusCategory['mounted'] = dict()
	bonusCategory['supplies'] = dict()
	bonusCategory['population'] = dict()
	unitUpgrades = dict()

    # A list of units to be given
    # Each unit is described with a tuple
    # ( unitString, bonusCategoryName, promotionList, name)
	giveUnits = []
	giveUnits.append( ( 'UNIT_NECROMANCER', 'false laroth', ['PROMOTION_HIDDEN', 'PROMOTION_IMPLACABLE', 'PROMOTION_NECROMANCER'], CyTranslator().getText('TXT_KEY_UNIT_FALSE_LAROTH',() ) ) )
	bonusCategory['false laroth']['xp'] = 10
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ARCHERY') ):
		giveUnits.append( ( 'UNIT_ARCHER', 'archer', ['PROMOTION_IMPLACABLE', ],  'Anyon' ) )
		giveUnits.append( ( 'UNIT_ARCHER', 'archer', ['PROMOTION_IMPLACABLE', ],  'Lann' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_BRONZE_WORKING') ):
		giveUnits.append( ( 'UNIT_HONORED_BAND', 'melee', ['PROMOTION_IMPLACABLE', ],  'Medb' ) )
		giveUnits.append( ( 'UNIT_HONORED_BAND', 'melee', ['PROMOTION_IMPLACABLE', ],  'Aife' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_IRON_WORKING') ):
		giveUnits.append( ( 'UNIT_PRINCIPES', 'melee', ['PROMOTION_IMPLACABLE', ],  'Scathach' ) )
		giveUnits.append( ( 'UNIT_PRINCIPES', 'melee', ['PROMOTION_IMPLACABLE', ],  'Voadica' ) )
		giveUnits.append( ( 'UNIT_HONORED_BAND', 'melee', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_TRADE')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_STIRRUPS')):
		giveUnits.append( ( 'UNIT_HORSE_ARCHER', 'mounted', [],  '' ) )
		giveUnits.append( ( 'UNIT_HORSE_ARCHER', 'mounted', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_WARHORSES')):
		giveUnits.append( ( 'UNIT_HORSE_ARCHER', 'mounted', [],  '' ) )
		giveUnits.append( ( 'UNIT_HORSE_ARCHER', 'mounted', [],  '' ) )
		giveUnits.append( ( 'UNIT_HORSE_ARCHER', 'mounted', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ANIMAL_HANDLING')):
		giveUnits.append( ( 'UNIT_CETRATUS', 'recon', ['PROMOTION_IMPLACABLE', ],  '' ) )
		giveUnits.append( ( 'UNIT_CETRATUS', 'recon', ['PROMOTION_IMPLACABLE', ],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_FERAL_BOND')):
		giveUnits.append( ( 'UNIT_GHOSTWALKER', 'recon', ['PROMOTION_IMPLACABLE', ],  '' ) )
		giveUnits.append( ( 'UNIT_GHOSTWALKER', 'recon', ['PROMOTION_IMPLACABLE', ],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_FERAL_BOND')):
		giveUnits.append( ( 'UNIT_GHOSTWALKER', 'recon', ['PROMOTION_IMPLACABLE', ],  'Gurith' ) )
		giveUnits.append( ( 'UNIT_GHOSTWALKER', 'recon', ['PROMOTION_IMPLACABLE', ],  'Fathen' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_SMELTING')):
		giveUnits.append( ( 'UNIT_HONORED_BAND', 'melee', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_SANITATION')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_MEDICINE')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_PHALANX')):
		giveUnits.append( ( 'UNIT_PHALANX', 'melee', [ ],  '' ) )
		giveUnits.append( ( 'UNIT_PHALANX', 'melee', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_TRADE')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_BOWYERS')):
		giveUnits.append( ( 'UNIT_LONGBOWMAN', 'archer', [],  '' ) )
		giveUnits.append( ( 'UNIT_LONGBOWMAN', 'archer', [],  '' ) )
		giveUnits.append( ( 'UNIT_LONGBOWMAN', 'archer', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_PRECISION')):
		giveUnits.append( ( 'UNIT_LONGBOWMAN', 'archer', [],  '' ) )
		giveUnits.append( ( 'UNIT_LONGBOWMAN', 'archer', [],  '' ) )
		giveUnits.append( ( 'UNIT_MARKSMAN', 'archer', ['PROMOTION_IMPLACABLE', ],  'Tadc' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_TAXATION')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_GUILDS')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_MERCANTILISM')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_MILITARY_STRATEGY')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		bonusCategory['melee']['xp'] = 6
		bonusCategory['archer']['xp'] = 6
		bonusCategory['recon']['xp'] = 6
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ENGINEERING')):
		giveUnits.append( ( 'UNIT_CATAPULT', 'supplies', ['PROMOTION_ACCURACY', ],  '' ) )
		giveUnits.append( ( 'UNIT_CATAPULT', 'supplies', ['PROMOTION_ACCURACY', ],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_BLASTING_POWDER')):
		giveUnits.append( ( 'UNIT_BONE_HORDE', 'archer', [],  '' ) )
		giveUnits.append( ( 'UNIT_BONE_HORDE', 'archer', [],  '' ) )
		giveUnits.append( ( 'UNIT_BONE_HORDE', 'archer', [],  '' ) )
		giveUnits.append( ( 'UNIT_BONE_HORDE', 'archer', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_SORCERY')):
		unitUpgrades['UNIT_ADEPT'] = 'UNIT_NECROMANCER'
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ARCANE_LORE')):
		giveUnits.append( ( 'UNIT_NECROMANCER', 'adept', ['PROMOTION_NECROMANCER', ], '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_STRENGTH_OF_WILL')):
		giveUnits.append( ( 'UNIT_NECROMANCER', 'adept', ['PROMOTION_NECROMANCER', ], '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_PASS_THROUGH_THE_ETHER')):
		giveUnits.append( ( 'UNIT_NECROMANCER', 'adept', ['PROMOTION_NECROMANCER', ], '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_MACHINERY')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_CATAPULT', 'supplies', [],  '' ) )
		giveUnits.append( ( 'UNIT_CATAPULT', 'supplies', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_OMNISCIENCE')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_NECROMANCER', 'adept', ['PROMOTION_NECROMANCER', ], '' ) )
		giveUnits.append( ( 'UNIT_NECROMANCER', 'adept', ['PROMOTION_NECROMANCER', 'PROMOTION_IMPLACABLE', ], 'Tetricues' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_DIVINATION')):
		bonusCategory['adept']['xp'] = bonusCategory['adept'].get('xp', 0 ) +2
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ALTERATION')):
		giveUnits.append( ( 'UNIT_GHOSTWALKER', 'recon', ['PROMOTION_ILLUSIONIST', 'PROMOTION_IMPLACABLE', ],  'Otal the Faded' ) )
		bonusCategory['adept']['xp'] = bonusCategory['adept'].get('xp', 0 ) +2
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_NECROMANCY')):
		bonusCategory['adept']['xp'] = bonusCategory['adept'].get('xp', 0 ) +2
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ELEMENTALISM')):
		bonusCategory['adept']['xp'] = bonusCategory['adept'].get('xp', 0 ) +2
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_RELIGIOUS_LAW')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_THEOLOGY')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_COMMUNE_WITH_NATURE')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_FANATICISM')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_REVENANT', 'melee', ['PROMOTION_INQUISITOR', ],  'Rasaec the Hollow' ) ) # Revenants are disciples, but very melee like disciples
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_MALEVOLENT_DESIGNS')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_RIGHTEOUSNESS')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_DIVINE_ESSENCE')):
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_AWAKENED', 'population', [],  '' ) )
		giveUnits.append( ( 'UNIT_ABOMINATION', 'melee', [],  '' ) )

	if not makeHelpText:
		pCapital = pPlayer.getCapitalCity()
		capitalX = pCapital.getX()
		capitalY = pCapital.getY()
		bestPlotGoodness = -1
		iBestPlot = None
		for i in xrange (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			plotGoodness = -1
			if ( pPlot.getOwner() != iPlayer
				and not pPlot.isWater()
				and pPlot.getNumUnits() == 0
				and not pPlot.isCity()
				and not pPlot.isImpassable() ):
					plotGoodness = CyGame().getSorenRandNum(500, "Place Spiral Gate")
					# I'm placing a limit on the added goodness.
					# This makes final goodness a bit less map dependant and less predictable.
					# The limits will need finetuning.
					# With a limit of 250, all continents with at least 175 tiles are equally good.
					plotGoodness += min( pPlot.area().getNumTiles() * 2, 100 )
					# With a limit of 500, all continents with at least 100 unclaimed tiles are equally good.
					plotGoodness += min( pPlot.area().getNumUnownedTiles() * 5, 400 )
					# Distance to capital is squared, since area is squared
					# With a limit of 450, all tiles further away than 450**.5 or 21 squares are equally good
					plotGoodness += min( ( (pPlot.getX()-capitalX)**2 + (pPlot.getY()-capitalY)**2 ), 750)
					if not pPlot.isOwned():
						plotGoodness += 300
			if plotGoodness > bestPlotGoodness:
				bestPlotGoodness = plotGoodness
				pBestPlot = pPlot
		if pBestPlot is None:
			# It's a world spell, it shouldn't fail just because. Probably won't anyway
			# but just to be sure.
			pBestPlot = pCapital.plot()

		pBestPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_HAUNTED_LANDS'),0)

		commonInitUnitArgs = ( pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH )
		for unitString, bonusCategoryName, promotionList, name in giveUnits:
			while unitString in unitUpgrades:
				unitString = unitUpgrades[unitString]
			newUnit = pPlayer.initUnit(gc.getInfoTypeForString(unitString), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
#			newUnit = pPlayer.initUnit( gc.getInfoTypeForString(unitString), *commonInitUnitArgs )
			for promotionString in promotionList:
				newUnit.setHasPromotion( gc.getInfoTypeForString( promotionString ), True)
			if 'promotions' in bonusCategory[bonusCategoryName]:
				for promotionString in bonusCategory[bonusCategoryName]['promotions']:
					newUnit.setHasPromotion( gc.getInfoTypeForString( promotionString ), True)
			if 'weapons' in bonusCategory[bonusCategoryName]:
				newUnit.setHasPromotion( gc.getInfoTypeForString( bonusCategory[bonusCategoryName]['weapons'] ), True)
			if 'xp' in bonusCategory[bonusCategoryName]:
				newUnit.setExperience(bonusCategory[bonusCategoryName]['xp'], -1)
			if name:
				newUnit.setName(name)

	else:
		counts = dict( (key,0) for key in [ 'false laroth', 'archer', 'adept', 'melee', 'recon', 'mounted', 'supplies', 'population' ])
		for unitString, bonusCategoryName, promotionList, name in giveUnits:
			counts[unitString] = counts.get( unitString, 0) +1
			counts[bonusCategoryName] += 1
		counts['military'] = sum( c for key, c in counts.iteritems() if key in set( [ 'false laroth', 'archer', 'adept', 'melee', 'recon', 'mounted', ] ) )
		templateHelpString = 'Treachery in the Land of the Dead.  A false Laroth steals away additional subjects for the Scions.  Currently casting this spell will give %(military)i military units, %(supplies)i supplies, and %(population)i Awakened.  The units will appear in a random, probably distant, location.'
		return templateHelpString % counts

# BlackArchon: obsolete
# "Feed" spell - Ghostwalkers spend xps to allow Creepers to create HL - The Creeper spell requires the Sentry2 promo.
#def spellPlant(caster):
#	pPlot = caster.plot()
#	caster.changeExperience(-3, -1, false, false, false)
#	iCreeper = gc.getInfoTypeForString('UNIT_CREEPER')
#	iPPromo = gc.getInfoTypeForString('PROMOTION_FED_CREEPER')
#	iPSent = gc.getInfoTypeForString('PROMOTION_SENTRY2')
#	for i in range(pPlot.getNumUnits()):
#		pUnit = pPlot.getUnit(i)
#		if pUnit.getUnitType() == iCreeper:
#			if pUnit.isHasPromotion(iPSent):
#				pUnit.setHasPromotion(iPPromo, True)
#				pUnit.setHasPromotion(iPSent, False)
#				return

# BlackArchon: obsolete
# Req for Corrupt Quartermaster
#def reqOTT(caster):
#	pPlot = caster.plot()
#	iPPromo = gc.getInfoTypeForString('PROMOTION_CORRUPT_QUARTERMASTER')
#	for i in range(pPlot.getNumUnits()):
#		pUnit = pPlot.getUnit(i)
#		if (caster.getOwner() != pUnit.getOwner()):
#			if pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CORRUPT_QUARTERMASTER')) == False:
#				return True
#	return False

# BlackArchon: obsolete
# Corrupt Quartermaster
#def spellOTT(caster):
#	pPlot = caster.plot()
#	iPPromo = gc.getInfoTypeForString('PROMOTION_CORRUPT_QUARTERMASTER')
#	iAnimal = gc.getInfoTypeForString('UNITCOMBAT_ANIMAL')
#	iBeast = gc.getInfoTypeForString('UNITCOMBAT_BEAST')
#	for i in range(pPlot.getNumUnits()):
#		pUnit = pPlot.getUnit(i)
#		if pUnit.getUnitCombatType() != iAnimal:
#			if pUnit.getUnitCombatType() != iBeast:
#				if (caster.getOwner() != pUnit.getOwner()):
#			if pUnit.isHasPromotion(iPPromo):
#					if pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CORRUPT_QUARTERMASTER')) == False:
#						pUnit.setHasPromotion(iPPromo, True)
#						return

# BlackArchon: obsolete
# Pelemoc's Horsemen.  Gives Pelemoc and effect promo and carries slight chance for war.
#def effectSuperSchemer(caster):
#	pCity = caster.plot().getPlotCity()
#	iUH = CyGame().getSorenRandNum(15, "Pelemocs Horsemen anger amount")
#	if CyGame().getSorenRandNum(10, "AirEllySpawn") < 5:
#		pCity.changeHappinessTimer(-3)
#		pCity.changeExtraHappiness(-1)
#	pCity.changeHurryAngerTimer(iUH)
#	caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SUPER_SCHEMER'), True)
#	if CyGame().getSorenRandNum(150, "Pelemocs Horesman war odds") == 1:
#		cf.startWar(caster.getOwner(), pPlot.getOwner(), WarPlanTypes.WARPLAN_LIMITED)

# BlackArchon: obsolete
# Req for Pelemoc's Horsemen
#def reqSuperSchemer(caster):
#	pCity = caster.plot().getPlotCity()
#	pPlayer = gc.getPlayer(caster.getOwner())
#	if pCity.getOwner() == caster.getOwner():
#		return False
#	return True

# Arawn's Dust
def spellSpoil(caster):
	pPlot = caster.plot()
	pPlot.setImprovementType(-1)

# BlackArchon: currently not used, but maybe suited for Emperor's Dagger?
#def spellSeekAlcinus(caster):
#	iAlcinus1 = gc.getInfoTypeForString('UNIT_ALCINUS')
#	iAlcinus2 = gc.getInfoTypeForString('UNIT_ALCINUS_UPGRADED')
#	iAlcinus3 = gc.getInfoTypeForString('UNIT_ALCINUS_ARCHMAGE')
#	for iPlayer in range(gc.getMAX_PLAYERS()):
#		pPlayer = gc.getPlayer(iPlayer)
#		py = PyPlayer(iPlayer)
#		if (pPlayer.isAlive()):
#			for pUnit in py.getUnitList():
#				if pUnit.getUnitType() == iAlcinus1 or pUnit.getUnitType() == iAlcinus2 or pUnit.getUnitType() == iAlcinus3:
#					pPlot = pUnit.plot()
#					pPlayer = gc.getPlayer(caster.getOwner())
#					iTeam = pPlayer.getTeam()
#					pPlot.setRevealed(iTeam, True, False, TeamTypes.NO_TEAM)
#					CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("We have found the traitor!", ()),'',1,'Art/Interface/Buttons/Units/Scions/MIT.dds',ColorTypes(8),pUnit.getX(),pUnit.getY(),True,True)

def reqManifestHornedDread(caster):

	iGhostwalker = gc.getInfoTypeForString('UNIT_GHOSTWALKER')
	iHornedDreadPromo = gc.getInfoTypeForString('PROMOTION_HORNED_DREAD')

	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isFeatAccomplished(FeatTypes.FEAT_MANIFEST_HORNED_DREAD) == false:
		return False
	if pPlayer.isFeatAccomplished(FeatTypes.FEAT_MANIFEST_FIRST_HORNED_DREAD) == false:
		return False

	if caster.isHasPromotion(iHornedDreadPromo):
		return False

	return True

def spellManifestHornedDread(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	py = PyPlayer(caster.getOwner())
	pPlayer.setFeatAccomplished(FeatTypes.FEAT_MANIFEST_HORNED_DREAD, false)

	iGhostwalker = gc.getInfoTypeForString('UNIT_GHOSTWALKER')
	iHornedDreadPromo = gc.getInfoTypeForString('PROMOTION_HORNED_DREAD')

	for pUnit in py.getUnitList():
		if pUnit.getUnitType() == iGhostwalker:
			if pUnit.isHasPromotion(iHornedDreadPromo):
				pUnit.setHasPromotion(iHornedDreadPromo, false)

	caster.setHasPromotion(iHornedDreadPromo, true)

def reqManifestFirstHornedDread(caster):
	pPlayer = gc.getPlayer(caster.getOwner())

	if pPlayer.isFeatAccomplished(FeatTypes.FEAT_MANIFEST_FIRST_HORNED_DREAD) == true:
		return False

	return True

def spellManifestFirstHornedDread(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlayer.setFeatAccomplished(FeatTypes.FEAT_MANIFEST_FIRST_HORNED_DREAD, true)

	iGhostwalker = gc.getInfoTypeForString('UNIT_GHOSTWALKER')
	iHornedDreadPromo = gc.getInfoTypeForString('PROMOTION_HORNED_DREAD')

	caster.setHasPromotion(iHornedDreadPromo, true)

# Scions end

def PostCombatBlackLadyLost(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pCaster.getOwner())

	pCapital = pPlayer.getCapitalCity()
	capitalX = pCapital.getX()
	capitalY = pCapital.getY()
	bestPlotGoodness = -1
	bBestPlot = None

	if pCaster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SECOND_CHANCE')):
		return

	py = PyPlayer(pCaster.getOwner())
	iBlackLady = gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY')
	iBLCount = 0
	for pUnit in py.getUnitList():
		if pUnit.getUnitType() == iBlackLady:
			iBLCount += 1
		if iBLCount > 1:
			pUnit.kill(True, 0)

	for i in xrange (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		plotGoodness = -1
#		if ( pPlot.getOwner() != iPlayer
		if ( not pPlot.isWater()
#			and not pPlot.isWater()
			and pPlot.getNumUnits() == 0
			and not pPlot.isCity()
			and not pPlot.isImpassable() ):
				plotGoodness = CyGame().getSorenRandNum(500, "Place Korrina")
				# I'm placing a limit on the added goodness.
				# This makes final goodness a bit less map dependant and less predictable.
				# The limits will need finetuning.
				# With a limit of 250, all continents with at least 175 tiles are equally good.
				plotGoodness += min( pPlot.area().getNumTiles() * 2, 100 )
				# With a limit of 500, all continents with at least 100 unclaimed tiles are equally good.
				plotGoodness += min( pPlot.area().getNumUnownedTiles() * 5, 400 )
				# Distance to capital is squared, since area is squared
				# With a limit of 450, all tiles further away than 450**.5 or 21 squares are equally good
#				plotGoodness += min( ( (pPlot.getX()-capitalX)**2 + (pPlot.getY()-capitalY)**2 ), 750)
				if not pPlot.isOwned():
					plotGoodness += 0
				if pPlot.getOwner() == pPlayer:
					plotGoodness += 1050

		if plotGoodness > bestPlotGoodness:
			bestPlotGoodness = plotGoodness
			pBestPlot = pPlot
	if pBestPlot is None:
		# It's a world spell, it shouldn't fail just because. Probably won't anyway
		# but just to be sure.
		pBestPlot = pCapital.plot()

#		pBestPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_HAUNTED_LANDS'),0)

	pCaster.setDamage(95, -1)

#	pCaster.setXY(pBestPlot.getX(), pBestPlot.getY(), false, true, true)
#	pCaster.setMoves(2)
#	pCaster.setMadeAttack(False)
#	CyInterface().selectUnit(pCaster, True, True, False)
#	pCaster.setHasCasted(True)
#	pCaster.setMadeAttack(True)
#	pCaster.finishMoves()
#	pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MORTIFICATION'), True)

	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setDamage(95, -1)
	for i in range(gc.getNumPromotionInfos()):
		newUnit.setHasPromotion(i, pCaster.isHasPromotion(i))
	newUnit.setDamage(95, -1)
	newUnit.setLevel(pCaster.getLevel())
	pCaster.kill(True, 0)
	newUnit.setHasCasted(True)
	newUnit.setMadeAttack(True)
	newUnit.finishMoves()
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MORTIFICATION'), True)
	CyInterface().selectUnit(newUnit, True, True, False)

#	newUnit.convert(pCaster)
#	pCaster.kill(True, 0)
#	newUnit.setDamage(95, -1)
#	newUnit.setHasCasted(True)
#	newUnit.setMadeAttack(True)
#	newUnit.finishMoves()
#	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MORTIFICATION'), True)
#	CyInterface().selectUnit(newUnit, True, True, False)

#	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
#	pPlot = pCaster.plot()
#	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)


def PostCombatBlackLadyWon(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pCaster.getOwner())

	pCapital = pPlayer.getCapitalCity()
	capitalX = pCapital.getX()
	capitalY = pCapital.getY()
	casterX = pCaster.getX()
	casterY = pCaster.getY()
	bestPlotGoodness = -1
	pBestPlot = None

	py = PyPlayer(pCaster.getOwner())
	iBlackLady = gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY')
	iBLCount = 0
	for pUnit in py.getUnitList():
		if pUnit.getUnitType() == iBlackLady:
			iBLCount += 1
		if iBLCount > 1:
			pUnit.kill(True, 0)

	for i in xrange (CyMap().numPlots()):
		pPlot = CyMap().plotByIndex(i)
		plotGoodness = -1
		if ( not pPlot.isWater()
			and pPlot.getNumUnits() == 0
			and not pPlot == pCaster.plot
			and not pPlot.isCity()
			and not pPlot.isImpassable() ):
				plotGoodness = CyGame().getSorenRandNum(500, "Place Korrina")
				iPX = abs(pPlot.getX()-casterX)
				iPY = abs(pPlot.getY()-casterY)
				if CyGame().getSorenRandNum(100, "where does she go") <= 10:
					plotGoodness -= max( ( (iPX + iPY) * 10), 400)
				else:
					plotGoodness -= (iPX + iPY) * 10
#				if not pPlot.isOwned():
#					plotGoodness += 0
#				if pPlot.getOwner() == pPlayer:
#					plotGoodness += 250

		if plotGoodness > bestPlotGoodness:
			bestPlotGoodness = plotGoodness
			pBestPlot = pPlot
	if pBestPlot is None:
		pBestPlot = pCapital.plot()

	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_KORRINA_BLACK_LADY'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.convert(pCaster)
	pCaster.kill(True, 0)
	iUnitMove = newUnit.baseMoves()
	newUnit.setMoves(iUnitMove)
#	pCaster.setXY(pBestPlot.getX(), pBestPlot.getY(), false, true, true)
	newUnit.setMadeAttack(False)
#	newUnit.setHasCasted(True)
	CyInterface().selectUnit(newUnit, True, True, False)

	return

#	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
#	pPlot = pCaster.plot()
#	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)


def effectGyrasBlessing(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())

	if(CyGame().getGameTurn() - pCaster.getGameTurnCreated()) == 10:
		pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MORTIFICATION'), False)

	if CyGame().getSorenRandNum(100, "Disfavor")<20:
		pPlayer.changeMana(-50)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("We have failed the Black Lady and a great power disapproves.  The Emperor has warded us from wrath, but at cost.  -50 Mana.", ()),'',1,'Art/Interface/Buttons/Promotions/Channeling1.dds',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
# Scions end

def postCombatScionPriest(pCaster, pOpponent):
	pPlayer = gc.getPlayer(pCaster.getOwner())

	if (pCaster.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_DOOMGIVER')):
		pPlot = pOpponent.plot()
		if pPlot.isCity():
			pCity = pPlot.getPlotCity()
			CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_POPULATION_REDUCED", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,1,4',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
			CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DOOMGIVER_CITY_ATTACK", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,1,4',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
			iDoomgiverHurryAnger = (5 + CyGame().getSorenRandNum(10, "Doomgiver attack anger amount"))
			pCity.changeHurryAngerTimer(iDoomgiverHurryAnger)
			if pCity.getPopulation() > 1:
				if CyGame().getSorenRandNum(5, "Doomgiver chance to reduce pop") == 1:
					pCity.changePopulation(-1)

	pPlot = pCaster.plot()
	if pOpponent.isAnimal():
		return
	if not pOpponent.isAlive():
		return
	iCasterLevel = pCaster.getLevel()
	iBarbMod = 0
	if pOpponent.isBarbarian():
		iBarbMod = 5
	iPriestMod = 0
	iDivine = gc.getInfoTypeForString('PROMOTION_DIVINE')
	if pCaster.isHasPromotion(iDivine) == true:
		iPriestMod = 3
	iCommand1 = gc.getInfoTypeForString('PROMOTION_COMMAND1')
	iCommand2 = gc.getInfoTypeForString('PROMOTION_COMMAND2')
	iCommand3 = gc.getInfoTypeForString('PROMOTION_COMMAND3')
	iCommandMod = 0
	if pCaster.isHasPromotion(iCommand1) == true:
		iCommandMod = 3
	if pCaster.isHasPromotion(iCommand2) == true:
		iCommandMod = 6
	if pCaster.isHasPromotion(iCommand3) == true:
		iCommandMod = 9
	iTurnRoll = CyGame().getSorenRandNum(100, "Scion priest Turn roll")
	if iTurnRoll < (7 + iPriestMod + iCommandMod - iBarbMod + (iCasterLevel * 2)):
#		pPlayer.changeFaith(100)
		iUnitRoll = CyGame().getSorenRandNum(100, "Scion priest Unit roll")
		if iUnitRoll < (30 + (8 * iBarbMod) - (100 * iCommandMod)):
			iArcher = gc.getInfoTypeForString('UNITCOMBAT_ARCHER')
			iRecon = gc.getInfoTypeForString('UNITCOMBAT_RECON')
			if pOpponent.getUnitCombatType() == iArcher:
				newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.setExperience((pOpponent.getExperience() / 3), -1)
				newUnit.setDamage(50, -1)
				newUnit.finishMoves()
				CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_UNIT_SPAWNED_PRIEST", ()),'',1,'Art/Interface/Buttons/Units/reborn.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
				return
			elif pOpponent.getUnitCombatType() == iRecon:
				newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_VELITES'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
				newUnit.setExperience((pOpponent.getExperience() / 3), -1)
				newUnit.setDamage(50, -1)
				newUnit.finishMoves()
				CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_UNIT_SPAWNED_PRIEST", ()),'',1,'Art/Interface/Buttons/Units/reborn.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
				return
			#iTurnedUndeadArt = gc.getInfoTypeForString('PROMOTION_TURNED_UNDEAD_ART')
			iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
			newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_CENTENI'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			#newUnit.setHasPromotion(iTurnedUndeadArt, True)
			newUnit.setHasPromotion(iUndead, True)
			newUnit.setExperience((pOpponent.getExperience() / 3), -1)
			newUnit.setDamage(50, -1)
			newUnit.finishMoves()
			CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_UNIT_SPAWNED_PRIEST", ()),'',1,'Art/Interface/Buttons/Units/reborn.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return
		else:
			newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_REBORN'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			newUnit.finishMoves()
			CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_REBORN_SPAWNED_PRIEST", ()),'',1,'Art/Interface/Buttons/Units/reborn.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return

def reqDrawStrength(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_RISEN_EMPEROR'):
		return True
	return False

def reqDarkC(caster):
	pCity = caster.plot().getPlotCity()
	if pCity.getTeam() != caster.getTeam():
		return False
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_PRIEST')) == 0:
		return False
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ARTIST')) == 0:
		return False
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_MERCHANT')) == 0:
		return False
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ENGINEER')) == 0:
		return False
	if pCity.getFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_SCIENTIST')) == 0:
		return False
	return True

def spellDarkC(caster):
	pCity = caster.plot().getPlotCity()
	pCity.changeFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_PRIEST'), -1)
	pCity.changeFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ARTIST'), -1)
	pCity.changeFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_MERCHANT'), -1)
	pCity.changeFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_ENGINEER'), -1)
	pCity.changeFreeSpecialistCount(gc.getInfoTypeForString('SPECIALIST_GREAT_SCIENTIST'), -1)

def reqMelanteKidnap(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
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

	if pPlayer.isHuman() == False:
		if pPlayer.getGold() < 2500:
			return False

	return True

def spellMelanteKidnap(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
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
	iGold = pPlayer.getGold()
	iChance = iGold * 0.02
	iBribe = iGold / 5
	if iBribe > 2000:
		iBribe = 2000
	pPlayer.changeGold(-iBribe)
	if CyGame().getSorenRandNum(100, "Melante Bribe") <= iChance:
		newUnit = pPlayer.initUnit(iUnit, caster.getX(), caster.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		pCity.changeFreeSpecialistCount(iSpec, -1)
	else:
		if CyGame().getSorenRandNum(100, "Melante Bribe") <= 30:
			caster.setXY(pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), false, true, true)
		else:
			caster.kill(True, 0)
		cf.startWar(caster.getOwner(), pCity.getOwner(), WarPlanTypes.WARPLAN_LIMITED)

def spellEmperorsfist(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	pCity.changeHurryAngerTimer(20)
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_EMPERORS_FIST')) > 0:
		pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_EMPERORS_HAND'), 0)

def reqEmperorshand(caster):
	pPlot = caster.plot()
	pCity = pPlot.getPlotCity()
	if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_EMPERORS_FIST')) > 0:
		return False
	return True

def spellBuildKeep(caster):
	pPlayer = gc.getPlayer(caster.getOwner())
	pPlot = caster.plot()
	pCity = caster.plot().getPlotCity()

	iChance = CyGame().getSorenRandNum(9, "Keep building roll")
	if iChance < 5:
		iBonus = gc.getInfoTypeForString('BONUS_MANA_DEATH')
	elif iChance == 5:
		iBonus = gc.getInfoTypeForString('BONUS_MANA_MIND')
	elif iChance == 6:
		iBonus = gc.getInfoTypeForString('BONUS_MANA_AIR')
	elif iChance == 7:
		iBonus = gc.getInfoTypeForString('BONUS_MANA_FIRE')
	else:
		iBonus = gc.getInfoTypeForString('BONUS_MANA_CHAOS')

	iChance = CyGame().getSorenRandNum(17, "Keep building roll")
	if iChance < 10:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_DEATH')
	elif iChance == 10:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_AIR')
	elif iChance == 11:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_FIRE')
	elif iChance == 12:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_CHAOS')
	elif iChance == 13:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_MIND')
	elif iChance == 14:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_ICE')
	elif iChance == 15:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_LIFE')
	else:
		iBonus2 = gc.getInfoTypeForString('BONUS_MANA_NATURE')

	pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_KEEP_MAIN'), 1)
	if iBonus != iBonus2:
		pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_KEEP_MAIN'),iBonus,1)
		pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_KEEP_MAIN'),iBonus2,1)
	else:
		pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_KEEP_MAIN'),iBonus,2)

# Scions end

# Req for Arawn's Dust
def reqSpoil(caster):

#	return True
	pPlot = caster.plot()
	iFarm = gc.getInfoTypeForString('IMPROVEMENT_FARM')
	iPasture = gc.getInfoTypeForString('IMPROVEMENT_PASTURE')
	iPlantation = gc.getInfoTypeForString('IMPROVEMENT_PLANTATION')
	iCamp = gc.getInfoTypeForString('IMPROVEMENT_CAMP')
	if  pPlot.getOwner() == caster.getOwner():
		return False
	if pPlot.getImprovementType() == iFarm:
		return True
	if pPlot.getImprovementType() == iCamp:
		return True
	if pPlot.getImprovementType() == iPasture:
		return True
	if pPlot.getImprovementType() == iPlantation:
		return True


def effectGyrasBlessing(pCaster):
	pPlayer = gc.getPlayer(pCaster.getOwner())

	if(CyGame().getGameTurn() - pCaster.getGameTurnCreated()) == 10:
		pCaster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MORTIFICATION'), False)

	if CyGame().getSorenRandNum(100, "Disfavor")<20:
		pPlayer.changeMana(-50)
		CyInterface().addMessage(pCaster.getOwner(),True,25,CyTranslator().getText("We have failed the Black Lady and a great power disapproves.  The Emperor has warded us from wrath, but at cost.  -50 Mana.", ()),'',1,'Art/Interface/Buttons/Promotions/Channeling1.dds',ColorTypes(8),pCaster.getX(),pCaster.getY(),True,True)
# Scions end
