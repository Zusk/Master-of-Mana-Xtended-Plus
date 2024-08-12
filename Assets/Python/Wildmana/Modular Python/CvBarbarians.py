import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#BARBARIANS

def DoBarbSpawn(argsList):
	'Unit Completed'
	unit = argsList[0]
	player = PyPlayer(unit.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())

	iBarbPlayer = gc.getBARBARIAN_PLAYER()

	if unit.getOwner() != iBarbPlayer:
		return

	if unit.plot().isCity():
		return

	iUnit = unit.getUnitType()

	if iUnit == gc.getInfoTypeForString('UNIT_WARRIOR'):
		unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CITY_RAIDER1'), True)

def BarbarianDefeated(argsList):
	'Combat Result'
	pWinner,pLoser = argsList
	pPlayerW = gc.getPlayer(pWinner.getOwner())
	pPlayerL = gc.getPlayer(pLoser.getOwner())

	# deactivate for now / Equipmentsystem has changed
	return

	if not pLoser.isBarbarian():
		return

	if not pWinner.isBarbarian():
		if pLoser.isHasPromotion(gc.getInfoTypeForString('PROMOTION_TREASURE_GEAR')):
			ListPossibleGear =[]
			for i in range(gc.getNumPromotionInfos()):
				if pPlayerW.canEquipUnit(pWinner,i,false,true,false):
					ListPossibleGear.append(i)

			iPick=CyGame().getSorenRandNum(len(ListPossibleGear), "Pick loot from named Barbs")
			iEquipment=ListPossibleGear[iPick]

			pWinner.setHasPromotion(iEquipment,true)
			CyInterface().addMessage(pWinner.getOwner(),true,25,"Your Unit looted %s by Defeating %s"%(gc.getPromotionInfo(iEquipment).getDescription(),pLoser.getName()),'',0,'',ColorTypes(11), pWinner.getX(), pWinner.getY(), True,True)

		eUnit=pLoser.getUnitType()
		eAdventure=-1
		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_JAECR'):
			eAdventure=gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_RING_OF_REGENERATION')
		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_GROUR'):
			eAdventure=gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_VAMPIRIC_SWORD')
		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_JOAL'):
			eAdventure=gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_RING_OF_VALOR')
		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_THIUFEU'):
			eAdventure=gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_CLOAK_OF_NATURE')
		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_FLAUREA'):
			eAdventure=gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_CLOAK_OF_SHADOW')
		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_VEUXA'):
			eAdventure=gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_FLAMING_WEAPONS')
		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_WOCI'):
			eAdventure=gc.getInfoTypeForString('ADVENTURE_EQUIPMENT_FROST_WEAPONS')

		if not eAdventure==-1:
			pPlayerW.setAdventureFinished(eAdventure,true)
			CyInterface().addMessage(pPlayerW.getID(),True,25,GC.getAdventureInfo(eAdventure).getDiscription(),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Spells/Explore Lair.dds',ColorTypes(7),pWinner.getX(),pWinner.getY(),True,True)

		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_TAIX'):
			if not pPlayerW.getCapitalCity().isNone():
				pPlayerW.getCapitalCity().changeBuildingCommerceChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), CommerceTypes.COMMERCE_MANA, 10)

		if eUnit==gc.getInfoTypeForString('UNIT_GUARDIAN_XAIG'):
			if CyGame().getSorenRandNum(100, "Guardian XAIG defeated")<35:
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FLYING'),true)
			if CyGame().getSorenRandNum(100, "Guardian XAIG defeated")<35:
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEAVY'),true)
			if CyGame().getSorenRandNum(100, "Guardian XAIG defeated")<35:
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_LIGHT'),true)
			if CyGame().getSorenRandNum(100, "Guardian XAIG defeated")<35:
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'),true)
			if CyGame().getSorenRandNum(100, "Guardian XAIG defeated")<35:
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISON_RESISTANCE'),true)
