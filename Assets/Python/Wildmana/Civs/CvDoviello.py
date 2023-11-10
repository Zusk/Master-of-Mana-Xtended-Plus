import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#DOVIELLO
def FeralScavengeWeapons(argsList):
	'Combat Result'
	pWinner,pLoser = argsList
	pPlayerW = gc.getPlayer(pWinner.getOwner())
	pPlayerL = gc.getPlayer(pLoser.getOwner())

	#Feral trait
	if pPlayerW.hasTrait(gc.getInfoTypeForString('TRAIT_FERAL')):

		#Special for the Baron
		if pPlayerW.getLeaderType()==gc.getInfoTypeForString('LEADER_DUIN') or pPlayerW.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_DUIN'))>0:
			if pWinner.isAlive():
				if CyGame().getSorenRandNum(20, "Moon") ==0:
					if not pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CANNIBALIZE')):
						pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CANNIBALIZE'),True)
						return

					if gc.getUnitInfo(pWinner.getUnitType()).getTier()==3:
						if not pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WEREWOLF')):
							if not isWorldUnitClass(gc.getUnitInfo(pWinner.getUnitType()).getUnitClassType()):
								iWerewolf= gc.getInfoTypeForString('UNIT_WEREWOLF')
								newUnit = pPlayerW.initUnit(iWerewolf, pWinner.getX(), pWinner.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								iAtt=pWinner.baseCombatStr()
								iDef=pWinner.baseCombatStrDefense()
								newUnit.convert(pWinner)
								pWinner.kill(true,pWinner.getOwner())
								newUnit.setBaseCombatStr(iAtt)
								newUnit.setBaseCombatStrDefense(iDef)
								newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'),false)
								return
		# BlackArchon: There is no durability for weapons or armor anymore, so we don't need that.
		#eWeapon = gc.getInfoTypeForString('EQUIPMENTCATEGORY_WEAPON')
		#eArmor = gc.getInfoTypeForString('EQUIPMENTCATEGORY_ARMOR')
		#if pLoser.getEquipment(eWeapon)!=-1:
		#	if pWinner.getEquipment(eWeapon)!=-1:
		#		pWinner.changeDurability(eWeapon,5)
		#if pLoser.getEquipment(eArmor)!=-1:
		#	if pWinner.getEquipment(eArmor)!=-1:
		#		pWinner.changeDurability(eArmor,5)
		# --End BlackArchon
		
		#Xtended
		eWeapon = gc.getInfoTypeForString('EQUIPMENTCATEGORY_WEAPON')
		eArmor = gc.getInfoTypeForString('EQUIPMENTCATEGORY_ARMOR')
		if pLoser.getEquipment(eWeapon)!=-1:
			if pWinner.getEquipment(eWeapon)==-1:
				if CyGame().getSorenRandNum(100, "Junil") + pWinner.getLevel() > 80:
					for iPromW in range(gc.getNumPromotionInfos()):
						if (pLoser.isHasPromotion(iPromW)):
							if gc.getPromotionInfo(iPromW).getEquipmentCategory()== eWeapon:
								pWinner.setHasPromotion(iPromW, true)
		if pLoser.getEquipment(eArmor)!=-1:
			if pWinner.getEquipment(eArmor)==-1:
				if CyGame().getSorenRandNum(100, "Junil") + pWinner.getLevel() > 80:
					for iPromA in range(gc.getNumPromotionInfos()):
						if (pLoser.isHasPromotion(iPromA)):
							if gc.getPromotionInfo(iPromA).getEquipmentCategory()== eArmor:
								pWinner.setHasPromotion(iPromA, true)

		if not pLoser.getUnitCombatType()==gc.getInfoTypeForString('UNITCOMBAT_ANIMAL'):
			return

		if not pWinner.isAlive():
			return

		iChance=5000+5000*(pPlayerW.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_PAGAN_TEMPLE'))/(pPlayerW.getNumCities()+1))
		if CyGame().getSorenRandNum(10000, "Gain Strength") < iChance:

			iUnit=pLoser.getUnitType()

			#Wolf
			if iUnit==gc.getInfoTypeForString('UNIT_WOLF') or iUnit==gc.getInfoTypeForString('UNIT_WOLF_PACK'):
				if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_WOLF')):
					if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DRILL1')):
						pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DRILL2'),true)
					pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DRILL1'),true)
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_WOLF'),true)
			#Bear
			if iUnit==gc.getInfoTypeForString('UNIT_BEAR'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_BEAR'),true)
			#Polar Bear
			if iUnit==gc.getInfoTypeForString('UNIT_POLAR_BEAR'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_POLAR_BEAR'),true)
			#Lion
			if iUnit==gc.getInfoTypeForString('UNIT_LION') or iUnit==gc.getInfoTypeForString('UNIT_LION_PRIDE'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_LION'),true)
			#Tiger
			if iUnit==gc.getInfoTypeForString('UNIT_TIGER') or iUnit==gc.getInfoTypeForString('UNIT_SABRETOOTH'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_TIGER'),true)
			#Griffon
			if iUnit==gc.getInfoTypeForString('UNIT_GRIFFON'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_GRIFFON'),true)
			#Spider
			if iUnit==gc.getInfoTypeForString('UNIT_GIANT_SPIDER'):
				if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_SPIDER')):
					if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN1')):
						pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN2'),true)
					pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN1'),true)
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_SPIDER'),true)
			#Gorilla
			if iUnit==gc.getInfoTypeForString('UNIT_GORILLA'):
				if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_GORILLA')):
					if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN1')):
						pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TAUNTING_PRESENCE'),true)
					pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN1'),true)
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_GORILLA'),true)
			#Elephant
			if iUnit==gc.getInfoTypeForString('UNIT_ELEPHANT'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_ELEPHANT'),true)
			#Mammoth
			if iUnit==gc.getInfoTypeForString('UNIT_MAMMOTH'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TOTEM_MAMMOTH'),true)
			#Desert Creatures
			if iUnit==gc.getInfoTypeForString('UNIT_SCORPION') or iUnit==gc.getInfoTypeForString('UNIT_LIZARD') or iUnit==gc.getInfoTypeForString('UNIT_HYENA'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_BONE_WEAPONS'),true)


			#Stag
			if iUnit==gc.getInfoTypeForString('UNIT_STAG'):
				if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN1')):
					pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN2'),true)
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WOODSMAN1'),true)
			#Allosaur
			if iUnit==gc.getInfoTypeForString('UNIT_ALLOSAUR'):
				if pWinner.isHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG')):
					pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HEROIC_ATTACK'),true)
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'),true)
			#Drakes
			if iUnit==gc.getInfoTypeForString('UNIT_BLACK_DRAKE'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENTROPY1'),true)
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING1'),true)
			if iUnit==gc.getInfoTypeForString('UNIT_RED_DRAKE'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FIRE2'),true)
			if iUnit==gc.getInfoTypeForString('UNIT_WHITE_DRAKE'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ICE1'),true)
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CHANNELING1'),true)
			if iUnit==gc.getInfoTypeForString('UNIT_BRASS_DRAKE'):
				pWinner.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EARTH2'),true)

			CyInterface().addMessage(pWinner.getOwner(),true,25,"One of your Men has proven to be a mighty warrior by defeating a strong animal!",'',0,'',ColorTypes(11), pWinner.getX(), pWinner.getY(), True,True)

			for iI in range(gc.getNumAdventureInfos()):
				if(pPlayerW.isAdventureEnabled(iI) and not pPlayerW.isAdventureFinished(iI)):
					for iJ in range(gc.getMAX_ADVENTURESTEPS()):
						if not gc.getAdventureInfo(iI).getAdventureStep(iJ)==-1:
							if(gc.getAdventureStepInfo(gc.getAdventureInfo(iI).getAdventureStep(iJ)).isTroophyFromAnimal()):
								pPlayerW.changeAdventureCounter(iI,iJ,1)


