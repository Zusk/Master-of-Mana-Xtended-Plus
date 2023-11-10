## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005

from CvPythonExtensions import *
import CvUtil
import Popup as PyPopup
import PyHelpers
import CvScreenEnums
import CvCameraControls

# globals
gc = CyGlobalContext()
PyPlayer = PyHelpers.PyPlayer

class CustomFunctions:

	def addBonus(self, iBonus, iNum, sIcon):
		listPlots = []
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if (pPlot.canHaveBonus(gc.getInfoTypeForString(iBonus),True) and pPlot.getBonusType(-1) == -1 and pPlot.isCity() == False):
				listPlots.append(i)
		if len(listPlots) > 0:
			for i in range (iNum):
				iRnd = CyGame().getSorenRandNum(len(listPlots), "Add Bonus")
				pPlot = CyMap().plotByIndex(listPlots[iRnd])
				pPlot.setBonusType(gc.getInfoTypeForString(iBonus))
				if sIcon != -1:
					iActivePlayer = CyGame().getActivePlayer()
					CyInterface().addMessage(iActivePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_RESOURCE_DISCOVERED",()),'AS2D_DISCOVERBONUS',1,sIcon,ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)

	def addPopup(self, szText, sDDS):
		szTitle = CyGameTextMgr().getTimeStr(CyGame().getGameTurn(), false)
		popup = PyPopup.PyPopup(-1)
		popup.addDDS(sDDS, 0, 0, 128, 384)
		popup.addSeparator()
		popup.setHeaderString(szTitle)
		popup.setBodyString(szText)
		popup.launch(true, PopupStates.POPUPSTATE_IMMEDIATE)

	def addPlayerPopup(self, szText, iPlayer):
		popupInfo = CyPopupInfo()
		popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON)
		popupInfo.setText(szText)
		popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_CLOSE", ()), "")
		popupInfo.addPopup(iPlayer)

	def addUnit(self, iUnit):
		pBestPlot = -1
		iBestPlot = -1
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			iPlot = -1
			if pPlot.isWater() == False:
				if pPlot.getNumUnits() == 0:
					if pPlot.isCity() == False:
						if pPlot.isImpassable() == False:
							iPlot = CyGame().getSorenRandNum(500, "Add Unit")
							iPlot = iPlot + (pPlot.area().getNumTiles() * 10)
							if pPlot.isBarbarian():
								iPlot = iPlot + 200
							if pPlot.isOwned():
								iPlot = iPlot / 2
							if iPlot > iBestPlot:
								iBestPlot = iPlot
								pBestPlot = pPlot
		if iBestPlot != -1:
			bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
			if iUnit==gc.getInfoTypeForString('UNIT_GURID') or iUnit==gc.getInfoTypeForString('UNIT_MARGALARD'):
				bPlayer = gc.getPlayer(gc.getANIMAL_PLAYER())
			
			#Xtended
			elif iUnit==gc.getInfoTypeForString('UNIT_ARS') or iUnit==gc.getInfoTypeForString('UNIT_BUBOES') or iUnit==gc.getInfoTypeForString('UNIT_YERSINIA') or iUnit==gc.getInfoTypeForString('UNIT_STEPHANOS'):
				bPlayer = gc.getPlayer(gc.getDEVIL_PLAYER())
			newUnit = bPlayer.initUnit(iUnit, pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)

	def addUnitFixed(self, caster, iUnit):
		pPlot = caster.plot()
		pNewPlot = self.findClearPlot(-1, pPlot)
		if pNewPlot != -1:
			pPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
			newUnit = pPlayer.initUnit(iUnit, pNewPlot.getX(), pNewPlot.getY(), UnitAITypes.UNITAI_ATTACK, DirectionTypes.DIRECTION_SOUTH)
			return newUnit
		return -1

	def doCrusade(self, iPlayer):
		iCrusadeChance = gc.getDefineINT('CRUSADE_SPAWN_CHANCE')
		iDemagog = gc.getInfoTypeForString('UNIT_DEMAGOG')
		iEnclave = gc.getInfoTypeForString('IMPROVEMENT_ENCLAVE')
		iTown = gc.getInfoTypeForString('IMPROVEMENT_TOWN')
		iVillage = gc.getInfoTypeForString('IMPROVEMENT_VILLAGE')
		building = gc.getInfoTypeForString('BUILDING_CHAPEL')
		pPlayer = gc.getPlayer(iPlayer)
		iPlatemail = gc.getInfoTypeForString('EQUIPMENT_ARMOR_PLATEMAIL')
		iMace = gc.getInfoTypeForString('EQUIPMENT_WEAPON_MACE')

		for pyCity in PyPlayer(iPlayer).getCityList():
			pCity = pyCity.GetCy()
			#break if we cannot support more units
			if not pPlayer.canSupportMoreUnits():
				return
			#Count Town Improvements around City
			iCount=0
			for i in range (pCity.getNumCityPlots()):
				pPlot = pCity.getCityIndexPlot(i)
				if (not pPlot.isNone()) and pCity.canWork(pPlot):
					if pPlot.getImprovementType() == iTown or pPlot.getImprovementType() == iEnclave :
						iCount+=1

			if iCount>0:
				pPlot = pCity.plot()
				if CyGame().getSorenRandNum(100, "Crusade") < iCount:
					for i in range(1+CyGame().getSorenRandNum(iCount, "Crusade")):
						if pCity.getNumRealBuilding(building) > 0:
							if CyGame().getSorenRandNum(100, "Religion") < 20:
								if pPlayer.getStateReligion() ==  gc.getInfoTypeForString('RELIGION_THE_ORDER') and pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_ORDER')) > 0:
									newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_CRUSADER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								elif pPlayer.getStateReligion() ==  gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH') and pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_KILMORPH')) > 0:
									newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_PARAMANDER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								elif pPlayer.getStateReligion() ==  gc.getInfoTypeForString('RELIGION_THE_EMPYREAN') and pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_EMPYREAN')) > 0:
									newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_RATHA'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								else:
									newUnit = pPlayer.initUnit(iDemagog, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
							else:
								newUnit = pPlayer.initUnit(iDemagog, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
							if CyGame().getSorenRandNum(100, "Religion") < 40 :
								newUnit.setReligion(pPlayer.getStateReligion())
						else:
							newUnit = pPlayer.initUnit(iDemagog, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

						newUnit.setHasPromotion(iMace,true)
						newUnit.setHasPromotion(iPlatemail,true)

	def doFear(self, pVictim, pPlot, pCaster, bResistable):
		if pVictim.isImmuneToFear():
			return False
		if bResistable:
			if CyGame().getSorenRandNum(100, "Crusade") < pVictim.getResistChance(pCaster, gc.getInfoTypeForString('SPELL_ROAR')):
				return False
		iX = pVictim.getX()
		iY = pVictim.getY()
		pBestPlot = -1
		iBestPlot = 0
		for iiX in range(iX-1, iX+2, 1):
			for iiY in range(iY-1, iY+2, 1):
				pLoopPlot = CyMap().plot(iiX,iiY)
				if not pLoopPlot.isNone():
					if not pLoopPlot.isVisibleEnemyUnit(pVictim.getOwner()):
						if pVictim.canMoveOrAttackInto(pLoopPlot, False):
							if (abs(pLoopPlot.getX() - pPlot.getX())>1) or (abs(pLoopPlot.getY() - pPlot.getY())>1):
								iRnd = CyGame().getSorenRandNum(500, "Fear")
								if iRnd > iBestPlot:
									iBestPlot = iRnd
									pBestPlot = pLoopPlot
		if pBestPlot != -1:
			pVictim.setXY(pBestPlot.getX(), pBestPlot.getY(), false, true, true)
			return True
		return False

	def exploreLairBigBad(self, caster):
		iPlayer = caster.getOwner()
		pPlot = caster.plot()
		pPlayer = gc.getPlayer(caster.getOwner())

		lList = ['UNIT_AZER']
		lPromoList = ['PROMOTION_MUTATED', 'PROMOTION_CANNIBALIZE', 'PROMOTION_MOBILITY1', 'PROMOTION_STRONG', 'PROMOTION_BLITZ', 'PROMOTION_COMMAND1', 'PROMOTION_HEROIC_STRENGTH', 'PROMOTION_HEROIC_DEFENSE', 'PROMOTION_MAGIC_IMMUNE', 'PROMOTION_VALOR', 'PROMOTION_VILE_TOUCH']
		lHenchmanList = ['UNIT_AZER', 'UNIT_GRIFFON']
		if self.grace() == False:
			lList = lList + ['UNIT_AIR_ELEMENTAL']
		if not pPlot.isWater():
			lList = lList + ['UNIT_ASSASSIN', 'UNIT_OGRE', 'UNIT_GIANT_SPIDER', 'UNIT_HILL_GIANT', 'UNIT_SPECTRE', 'UNIT_SCORPION']
			lHenchmanList = lHenchmanList + ['UNIT_AXEMAN', 'UNIT_WOLF', 'UNIT_CHAOS_MARAUDER', 'UNIT_WOLF_RIDER', 'UNIT_MISTFORM', 'UNIT_LION', 'UNIT_TIGER', 'UNIT_BABY_SPIDER', 'UNIT_FAWN', 'UNIT_SCORPION']
			if self.grace() == False:
				lList = lList + ['UNIT_EARTH_ELEMENTAL', 'UNIT_FIRE_ELEMENTAL', 'UNIT_GARGOYLE', 'UNIT_VAMPIRE', 'UNIT_MYCONID', 'UNIT_EIDOLON', 'UNIT_LICH', 'UNIT_OGRE_WARCHIEF', 'UNIT_SATYR', 'UNIT_WEREWOLF']
				lPromoList = lPromoList + ['PROMOTION_HERO', 'PROMOTION_MARKSMAN']
				lHenchmanList = lHenchmanList + ['UNIT_OGRE']
				if pPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST'):
					lList = lList + ['UNIT_TREANT']
			if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SNOW'):
				lHenchmanList = lHenchmanList + ['UNIT_FROSTLING_ARCHER', 'UNIT_FROSTLING_WOLF_RIDER', 'UNIT_POLAR_BEAR']
			if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_BARROW'):
				lHenchmanList = lHenchmanList + ['UNIT_SKELETON', 'UNIT_PYRE_ZOMBIE']
				if self.grace() == False:
					lList = lList + ['UNIT_WRAITH']
			elif pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_RUINS'):
				lHenchmanList = lHenchmanList + ['UNIT_LIZARDMAN', 'UNIT_GORILLA']
				if self.grace() == False:
					lList = lList + ['UNIT_MANTICORE']
			if CyGame().getGlobalCounter() > 40:
				lList = lList + ['UNIT_PIT_BEAST', 'UNIT_DEATH_KNIGHT', 'UNIT_BALOR']
				lPromoList = lPromoList + ['PROMOTION_FEAR']
				lHenchmanList = lHenchmanList + ['UNIT_IMP', 'UNIT_HELLHOUND']
		if pPlot.isWater():
			lList = lList + ['UNIT_SEA_SERPENT', 'UNIT_STYGIAN_GUARD', 'UNIT_PIRATE']
			lHenchmanList = lHenchmanList + ['UNIT_DROWN']
			if self.grace() == False:
				lList = lList + ['UNIT_WATER_ELEMENTAL', 'UNIT_KRAKEN']

		sMonster = lList[CyGame().getSorenRandNum(len(lList), "Pick Monster")]
		sHenchman = lHenchmanList[CyGame().getSorenRandNum(len(lHenchmanList), "Pick Henchman")]
		sPromo = lPromoList[CyGame().getSorenRandNum(len(lPromoList), "Pick Promotion")]
		iUnit = gc.getInfoTypeForString(sMonster)
		iHenchman = gc.getInfoTypeForString(sHenchman)
		newUnit = self.addUnitFixed(caster,iUnit)
		if newUnit != -1:
			newUnit.setHasPromotion(gc.getInfoTypeForString(sPromo), True)
			newUnit.setName(self.MarnokNameGenerator(newUnit))
			newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_TREASURE_GEAR'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BIGBAD",()),'',1,gc.getUnitInfo(iUnit).getButton(),ColorTypes(7),newUnit.getX(),newUnit.getY(),True,True)
			bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
			for i in range (CyGame().getSorenRandNum(5, "Pick Henchmen")):
				bPlayer.initUnit(iHenchman, newUnit.getX(), newUnit.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		return 0

	def exploreLairBad(self, caster):
		iPlayer = caster.getOwner()
		pPlot = caster.plot()
		pPlayer = gc.getPlayer(caster.getOwner())

		lList = ['COLLAPSE']
		if caster.getLevel() == 1:
			lList = lList + ['DEATH']
#Modified Sephi
#Higher Level Units are better at exploring lairs
#		if caster.isAlive():
#			lList = lList + ['CRAZED', 'DEMONIC_POSSESSION', 'DISEASED', 'ENRAGED', 'PLAGUED', 'POISONED', 'WITHERED']

		if caster.getLevel() < 4 and caster.isAlive():
			lList = lList + ['CRAZED', 'DEMONIC_POSSESSION', 'DISEASED', 'ENRAGED', 'PLAGUED', 'POISONED', 'WITHERED']

		if caster.getLevel() < 6 and caster.isAlive():
			lList = lList + ['CRAZED', 'DEMONIC_POSSESSION', 'PLAGUED', 'WITHERED']

		# if caster.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_MELEE'):
			# lList = lList + ['RUSTED']
		if pPlot.isWater():
			lList = lList + ['SPAWN_DROWN', 'SPAWN_SEA_SERPENT']
		else:
			lList = lList + ['SPAWN_SPIDER', 'SPAWN_SPECTRE']
		if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_GOBLIN_FORT'):
			lList = lList + ['SPAWN_SCORPION_BAD', 'SPAWN_SCORPION_BAD', 'SPAWN_SCORPION_BAD']

		sGoody = lList[CyGame().getSorenRandNum(len(lList), "Pick Goody")]
		if sGoody == 'DEATH':
			caster.kill(True,0)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_DEATH", ()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,3,4',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 0
		elif sGoody == 'COLLAPSE':
			caster.doDamageNoCaster(50, 90, gc.getInfoTypeForString('DAMAGE_PHYSICAL'), false)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_COLLAPSE", ()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,3,4',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'CRAZED':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_CRAZED", ()),'',1,'Art/Interface/Buttons/Promotions/Crazed.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		elif sGoody == 'DEMONIC_POSSESSION':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), True)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_POSSESSED", ()),'',1,'Art/Interface/Buttons/Units/UCDemon.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		elif sGoody == 'DISEASED':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DISEASED'), True)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_DISEASED", ()),'',1,'Art/Interface/Buttons/Promotions/Diseased.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		elif sGoody == 'ENRAGED':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_ENRAGED", ()),'',1,'Art/Interface/Buttons/Promotions/Enraged.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		elif sGoody == 'PLAGUED':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PLAGUED'), True)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_PLAGUED", ()),'',1,'Art/Interface/Buttons/Promotions/Plagued.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		elif sGoody == 'POISONED':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), True)
			caster.doDamageNoCaster(25, 90, gc.getInfoTypeForString('DAMAGE_POISON'), false)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_POISONED", ()),'',1,'Art/Interface/Buttons/Promotions/Poisoned.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		elif sGoody == 'WITHERED':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WITHERED'), True)
			CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_WITHERED", ()),'',1,'Art/Interface/Buttons/Promotions/Withered.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		#if sGoody == 'RUSTED':
		#	caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_RUSTED'), True)
		#	CyInterface().addMessage(caster.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_RUSTED", ()),'',1,'Art/Interface/Buttons/Promotions/Rusted.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
		#	return 80
		elif sGoody == 'SPAWN_DROWN':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_DROWN'), caster)
			return 50
		elif sGoody == 'SPAWN_SCORPION_BAD':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_SCORPION_BAD'), caster)
			return 50
		elif sGoody == 'SPAWN_SEA_SERPENT':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_SEA_SERPENT'), caster)
			return 50
		elif sGoody == 'SPAWN_SPECTRE':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_SPECTRE'), caster)
			return 50
		elif sGoody == 'SPAWN_SPIDER':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_SPIDER'), caster)
			return 50
		return 100

	def exploreLairNeutral(self, caster):
		iPlayer = caster.getOwner()
		pPlot = caster.plot()
		pPlayer = gc.getPlayer(caster.getOwner())
		lList = ['NOTHING']
		if not pPlot.isWater():
			lList = lList + ['SPAWN_SKELETON', 'SPAWN_LIZARDMAN', 'SPAWN_SPIDER', 'PORTAL', 'DEPTHS', 'DWARF_VS_LIZARDMEN', 'CAGE']
			if pPlot.getTerrainType() == gc.getInfoTypeForString('TERRAIN_SNOW'):
				lList = lList + ['SPAWN_FROSTLING']
			if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_BARROW'):
				lList = lList + ['SPAWN_SKELETON', 'SPAWN_SKELETON']
			if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_RUINS'):
				lList = lList + ['SPAWN_LIZARDMAN', 'SPAWN_LIZARDMAN']
			if pPlot.getImprovementType() == gc.getInfoTypeForString('IMPROVEMENT_GOBLIN_FORT'):
				lList = lList + ['SPAWN_SCORPION', 'SPAWN_SCORPION', 'SPAWN_SCORPION']
		else:
			lList = lList + ['SPAWN_DROWN']
		if caster.isAlive():
			if not caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MUTATED')):
				lList = lList + ['MUTATED']

		sGoody = lList[CyGame().getSorenRandNum(len(lList), "Pick Goody")]
		if sGoody == 'CAGE':
			pPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_CAGE'))
			for i in range(pPlot.getNumUnits(), -1, -1):
				pUnit = pPlot.getUnit(i)
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HELD'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_CAGE",()),'',1,'Art/Interface/Buttons/Improvements/Cage.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
			return 0
		elif sGoody == 'DEPTHS':
			iUnitID = self.getUnitPlayerID(caster)
			if iUnitID != -1:
				iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_EXPLORE_LAIR_DEPTHS')
				triggerData = pPlayer.initTriggeredData(iEvent, true, -1, caster.getX(), caster.getY(), caster.getOwner(), -1, -1, -1, iUnitID, -1)
			return 100
		elif sGoody == 'DWARF_VS_LIZARDMEN':
			iUnitID = self.getUnitPlayerID(caster)
			if iUnitID != -1:
				iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_EXPLORE_LAIR_DWARF_VS_LIZARDMEN')
				triggerData = pPlayer.initTriggeredData(iEvent, true, -1, caster.getX(), caster.getY(), caster.getOwner(), -1, -1, -1, iUnitID, -1)
			return 100
		elif sGoody == 'MUTATED':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MUTATED'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_MUTATED",()),'',1,'Art/Interface/Buttons/Promotions/Mutated.dds',ColorTypes(7),caster.getX(),caster.getY(),True,True)
			return 50
		elif sGoody == 'NOTHING':
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_NOTHING",()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,3,4',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'PORTAL':
			iUnitID = self.getUnitPlayerID(caster)
			if iUnitID != -1:
				iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_EXPLORE_LAIR_PORTAL')
				triggerData = pPlayer.initTriggeredData(iEvent, true, -1, caster.getX(), caster.getY(), caster.getOwner(), -1, -1, -1, iUnitID, -1)
			return 0
		elif sGoody == 'SPAWN_DROWN':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_DROWN'), caster)
			return 50
		elif sGoody == 'SPAWN_FROSTLING':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_FROSTLING'), caster)
			return 50
		elif sGoody == 'SPAWN_LIZARDMAN':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_LIZARDMAN'), caster)
			return 50
		elif sGoody == 'SPAWN_SCORPION':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_SCORPION'), caster)
			return 50
		elif sGoody == 'SPAWN_SKELETON':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_SKELETON'), caster)
			return 50
		elif sGoody == 'SPAWN_SPIDER':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_SPAWN_SPIDER'), caster)
			return 50
		return 100

	def exploreLairGood(self, caster):
		iPlayer = caster.getOwner()
		pPlot = caster.plot()
		pPlayer = gc.getPlayer(caster.getOwner())
		lList = ['HIGH_GOLD', 'TREASURE', 'EXPERIENCE']
		if caster.isAlive():
			if not caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SPIRIT_GUIDE')):
				lList = lList + ['SPIRIT_GUIDE']
		if not pPlot.isWater():
			lList = lList + ['ITEM_HEALING_SALVE', 'ITEM_POTION_OF_INVISIBILITY', 'ITEM_POTION_OF_CLARITY', 'ITEM_RING_OF_REGENERATION', 'ITEM_RING_OF_VALOR']
			if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_MYSTICISM')):
				lList = lList + ['PRISONER_DISCIPLE_ASHEN', 'PRISONER_DISCIPLE_EMPYREAN', 'PRISONER_DISCIPLE_LEAVES', 'PRISONER_DISCIPLE_OVERLORDS', 'PRISONER_DISCIPLE_RUNES', 'PRISONER_DISCIPLE_ORDER']

		sGoody = lList[CyGame().getSorenRandNum(len(lList), "Pick Goody")]
		if sGoody == 'HIGH_GOLD':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_HIGH_GOLD'), caster)
			return 90
		elif sGoody == 'TREASURE':
			self.placeTreasure(iPlayer, gc.getInfoTypeForString('EQUIPMENT_TREASURE'))
			return 80
		elif sGoody == 'EXPERIENCE':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_EXPERIENCE'), caster)
			return 100
		elif sGoody == 'SPIRIT_GUIDE':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SPIRIT_GUIDE'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_SPIRIT_GUIDE",()),'AS2D_POSITIVE_DINK',1,'Art/Interface/Buttons/Promotions/SpiritGuide.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 80
		elif sGoody == 'ITEM_HEALING_SALVE':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_HEALING_SALVE'), caster)
			return 100
		elif sGoody == 'ITEM_POTION_OF_INVISIBILITY':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_POTION_OF_INVISIBILITY'), caster)
			return 100
		elif sGoody == 'ITEM_POTION_OF_CLARITY':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POTION_OF_CLARITY'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_POTION",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons1/potion_clarity.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'ITEM_RING_OF_REGENERATION':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_RING_OF_REGENERATION'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_LOST_TRINKET",()),'',1,'Art/Interface/Buttons/Equipment/ringofregen.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'ITEM_RING_OF_VALOR':
			caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_RING_OF_VALOR'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_LOST_TRINKET",()),'',1,'Art/Interface/Buttons/Equipment/ringofvalor.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'PRISONER_DISCIPLE_ASHEN':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_DISCIPLE_ASHEN'), caster)
			return 100
		elif sGoody == 'PRISONER_DISCIPLE_EMPYREAN':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_DISCIPLE_EMPYREAN'), caster)
			return 100
		elif sGoody == 'PRISONER_DISCIPLE_LEAVES':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_DISCIPLE_LEAVES'), caster)
			return 100
		elif sGoody == 'PRISONER_DISCIPLE_OVERLORDS':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_DISCIPLE_OVERLORDS'), caster)
			return 100
		elif sGoody == 'PRISONER_DISCIPLE_RUNES':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_DISCIPLE_RUNES'), caster)
			return 100
		elif sGoody == 'PRISONER_DISCIPLE_ORDER':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_DISCIPLE_ORDER'), caster)
			return 100
		return 100

	def exploreLairBigGood(self, caster):
		iPlayer = caster.getOwner()
		pPlot = caster.plot()
		pPlayer = gc.getPlayer(caster.getOwner())

		lList = ['TREASURE_VAULT', 'GOLDEN_AGE']
		if pPlayer.canReceiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_TECH'), caster):
			lList = lList + ['TECH']
		if not pPlot.isWater():
			lList = lList + ['ITEM_ALSOPHOCUS_TOME', 'ITEM_MARTYR_CROWN', 'ITEM_PRESERVER', 'ITEM_SEVEN_LEAGUE', 'ITEM_TIMOR_MASK', 'ITEM_BLOOD_DRAGON', 'ITEM_HIGH_PRIEST_REGALIA', 'ITEM_ARMOR_BRIMSTONE', 'ITEM_ARMOR_KINGS', 'PRISONER_ADVENTURER', 'PRISONER_ARTIST', 'PRISONER_COMMANDER', 'PRISONER_ENGINEER', 'PRISONER_MAGE', 'PRISONER_MERCHANT', 'PRISONER_PROPHET', 'PRISONER_SCIENTIST']
			if pPlot.getBonusType(-1) == -1:
				lList = lList + ['BONUS_MANA']
				if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_MINING_GUILD')):
					lList = lList + ['BONUS_COPPER', 'BONUS_AMBER', 'BONUS_GEMS', 'BONUS_GOLD', 'BONUS_SILVER']
				if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_MINT')):
					lList = lList + ['BONUS_IRON']
				if pPlayer.isHasTech(gc.getInfoTypeForString('TECH_MITHRIL_WORKING')):
					lList = lList + ['BONUS_MITHRIL']
		else:
			lList = lList + ['PRISONER_SEA_SERPENT']
			if pPlot.getBonusType(-1) == -1:
				lList = lList + ['BONUS_CLAM', 'BONUS_CRAB', 'BONUS_FISH']
#		if self.grace() == False:
#			lList = lList + ['PRISONER_ANGEL', 'PRISONER_MONK', 'PRISONER_ASSASSIN', 'PRISONER_CHAMPION', 'PRISONER_MAGE']
		sGoody = lList[CyGame().getSorenRandNum(len(lList), "Pick Goody")]

		if sGoody == 'TREASURE_VAULT':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_TREASURE_VAULT'), caster)
			return 100
		elif sGoody == 'BONUS_CLAM':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_CLAM'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_CLAM",()),'',1,'Art/Interface/Buttons/WorldBuilder/Clam.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_COPPER':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_COPPER'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_COPPER",()),'',1,'Art/Interface/Buttons/WorldBuilder/Copper.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_CRAB':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_CRAB'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_CRAB",()),'',1,'Art/Interface/Buttons/WorldBuilder/Crab.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_FISH':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_FISH'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_FISH",()),'',1,'Art/Interface/Buttons/WorldBuilder/Fish.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_AMBER':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_AMBER'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_AMBER",()),'',1,'Art/Interface/Buttons/WorldBuilder/Gold.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_SILVER':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_SILVER'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_SILVER",()),'',1,'Art/Interface/Buttons/WorldBuilder/Gold.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_GOLD':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_GOLD'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_GOLD",()),'',1,'Art/Interface/Buttons/WorldBuilder/Gold.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_GEMS':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_GEMS'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_GEMS",()),'',1,'Art/Interface/Buttons/WorldBuilder/Gems.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_IRON':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_IRON'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_IRON",()),'',1,'Art/Interface/Buttons/WorldBuilder/Iron.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'BONUS_MANA':
			pPlot.setBonusType(gc.getInfoTypeForString('BONUS_MANA'))
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_BONUS_MANA",()),'',1,'Art/Interface/Buttons/WorldBuilder/mana_button.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'GOLDEN_AGE':
			pPlayer.changeGoldenAgeTurns(CyGame().goldenAgeLength())
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_GOLDEN_AGE",()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,3,4',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'ITEM_TIMOR_MASK':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_TIMOR_MASK'), caster)
			return 100
		elif sGoody == 'ITEM_ALSOPHOCUS_TOME':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_ALSOPHOCUS_TOME'), caster)
			return 100
		elif sGoody == 'ITEM_MARTYR_CROWN':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_MARTYR_CROWN'), caster)
			return 100
		elif sGoody == 'ITEM_PRESERVER':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_PRESERVER'), caster)
			return 100
		elif sGoody == 'ITEM_SEVEN_LEAGUE':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_ITEM_SEVEN_LEAGUE'), caster)
			return 100
		elif sGoody == 'ITEM_BLOOD_DRAGON':
			iPotion = CyGame().getSorenRandNum(100, "alchemy")
			if iPotion > 90:
				caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POTION_OF_DRAGON'), True)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_POTION_OF_DRAGON",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons1/potion_dragon.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			elif iPotion > 70:
				caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POTION_REJUVENATION'), True)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_POTION",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons1/potion_rejuvenation.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			elif iPotion > 50:
				caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POTION_STRONG'), True)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_POTION",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons1/potion_strength.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			else:
				caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POTION_HASTE'), True)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_POTION",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons1/potion_haste.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'ITEM_HIGH_PRIEST_REGALIA':
			caster.setHasPromotion(gc.getInfoTypeForString('EQUIPMENT_ROBE_HIGH_PRIEST'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_LOST_ARMORS",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons2/high_priest.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'ITEM_ARMOR_BRIMSTONE':
			caster.setHasPromotion(gc.getInfoTypeForString('EQUIPMENT_ARMOR_BRIMSTONE'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_LOST_ARMORS",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons2/breastplate_brimstone.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'ITEM_ARMOR_KINGS':
			caster.setHasPromotion(gc.getInfoTypeForString('EQUIPMENT_ARMOR_KINGS'), True)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_LOST_ARMORS",()),'',1,'Modules/NormalModules/Xtended/XWands/Buttons2/armor_of_kings.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
			return 100
		elif sGoody == 'PRISONER_ADVENTURER':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_ADVENTURER'), caster)
			return 100
		elif sGoody == 'PRISONER_ANGEL':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_ANGEL'), caster)
			return 100
		elif sGoody == 'PRISONER_ARTIST':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_ARTIST'), caster)
			return 100
		elif sGoody == 'PRISONER_ASSASSIN':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_ASSASSIN'), caster)
			return 100
#		elif sGoody == 'PRISONER_CHAMPION':
#			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_CHAMPION'), caster)
			return 100
		elif sGoody == 'PRISONER_COMMANDER':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_COMMANDER'), caster)
			return 100
		elif sGoody == 'PRISONER_ENGINEER':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_ENGINEER'), caster)
			return 100
		elif sGoody == 'PRISONER_MAGE':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_MAGE'), caster)
			return 100
		elif sGoody == 'PRISONER_MERCHANT':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_MERCHANT'), caster)
			return 100
#		elif sGoody == 'PRISONER_MONK':
#			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_PROPHET'), caster)
			return 100
		elif sGoody == 'PRISONER_PROPHET':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_PROPHET'), caster)
			return 100
		elif sGoody == 'PRISONER_SEA_SERPENT':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_SEA_SERPENT'), caster)
			return 100
		elif sGoody == 'PRISONER_SCIENTIST':
			pPlayer.receiveGoody(pPlot,gc.getInfoTypeForString('GOODY_EXPLORE_LAIR_PRISONER_SCIENTIST'), caster)
			return 100
		elif sGoody == 'TECH':
			pPlayer.receiveGoody(pPlot, gc.getInfoTypeForString('GOODY_GRAVE_TECH'), caster)
			return 100
		return 100

	def formEmpire(self, iCiv, iLeader, iTeam, pCity, iAlignment, pFromPlayer):
		iPlayer = self.getOpenPlayer()
		pPlot = pCity.plot()
		pPlot2 = self.findClearPlot(-1, pCity.plot())
		if (iPlayer != -1 and pPlot2 != -1):
			for i in range(pPlot.getNumUnits(), -1, -1):
				pUnit = pPlot.getUnit(i)
				pUnit.setXY(pPlot2.getX(), pPlot2.getY(), true, true, true)
			CyGame().addPlayerAdvanced(iPlayer, iTeam, iLeader, iCiv)
			pPlayer = gc.getPlayer(iPlayer)
			pPlayer.acquireCity(pCity, False, False)
			pCity = pPlot.getPlotCity()
			pCity.changeCulture(iPlayer, 500, True)
			pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			pPlayer.initUnit(gc.getInfoTypeForString('UNIT_WORKER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			if iAlignment != -1:
				pPlayer.setAlignment(iAlignment)
			if pFromPlayer != -1:
				eFromTeam = gc.getTeam(pFromPlayer.getTeam())
				eTeam = gc.getTeam(pPlayer.getTeam())
				for iTech in range(gc.getNumTechInfos()):
					if eFromTeam.isHasTech(iTech):
						eTeam.setHasTech(iTech, true, iPlayer, true, false)

	def grace(self):
                iGrace = 20 * (int(CyGame().getGameSpeedType()) + 1)
		iDiff = gc.getNumHandicapInfos() + 1 - int(gc.getGame().getHandicapType())
		iGrace = iGrace * iDiff
		iGrace = CyGame().getSorenRandNum(iGrace, "grace") + iGrace
		if iGrace > CyGame().getGameTurn():
			return True
		return False

	def doCityFire(self, pCity):
		iCount = 0
		iDemon = gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS')
		for iBuilding in range(gc.getNumBuildingInfos()):
			if iBuilding != iDemon:
				if pCity.getNumRealBuilding(iBuilding) > 0:
					if gc.getBuildingInfo(iBuilding).getConquestProbability() != 100:
						if CyGame().getSorenRandNum(100, "City Fire") <= 10:
							pCity.setNumRealBuilding(iBuilding, 0)
							CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE",(gc.getBuildingInfo(iBuilding).getDescription(), )),'',1,gc.getBuildingInfo(iBuilding).getButton(),ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
							iCount += 1
		if iCount == 0:
			CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE_NO_DAMAGE",()),'AS2D_SPELL_FIRE_ELEMENTAL',1,'Art/Interface/Buttons/Fire.dds',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)

	#xtended
	def doHellTerrain(self):
		iAshenVeil = gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL')
		iBurningSands = gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')
		iBanana = gc.getInfoTypeForString('BONUS_BANANA')
		iBison = gc.getInfoTypeForString('BONUS_BISON')
		iCotton = gc.getInfoTypeForString('BONUS_COTTON')
		iCorn = gc.getInfoTypeForString('BONUS_CORN')
		iCow = gc.getInfoTypeForString('BONUS_COW')
		iDeer = gc.getInfoTypeForString('BONUS_DEER')
		iDye = gc.getInfoTypeForString('BONUS_DYE')
		iEvil = gc.getInfoTypeForString('ALIGNMENT_EVIL')
		iFur = gc.getInfoTypeForString('BONUS_FUR')
		iFarm = gc.getInfoTypeForString('IMPROVEMENT_FARM')
		iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
		iFlamesSpreadChance = gc.getDefineINT('FLAMES_SPREAD_CHANCE')
		iGulagarm = gc.getInfoTypeForString('BONUS_GULAGARM')
		iHorse = gc.getInfoTypeForString('BONUS_HORSE')
		iInfernal = gc.getInfoTypeForString('CIVILIZATION_INFERNAL')
		iMarble = gc.getInfoTypeForString('BONUS_MARBLE')
		iNeutral = gc.getInfoTypeForString('ALIGNMENT_NEUTRAL')
		iNightmare = gc.getInfoTypeForString('BONUS_NIGHTMARE')
		iPig = gc.getInfoTypeForString('BONUS_PIG')
		iRazorweed = gc.getInfoTypeForString('BONUS_RAZORWEED')
		iRice = gc.getInfoTypeForString('BONUS_RICE')
		iSheep = gc.getInfoTypeForString('BONUS_SHEEP')
		iSheutStone = gc.getInfoTypeForString('BONUS_SHEUT_STONE')
		iSilk = gc.getInfoTypeForString('BONUS_SILK')
		iSnakePillar = gc.getInfoTypeForString('IMPROVEMENT_SNAKE_PILLAR')
		iSugar = gc.getInfoTypeForString('BONUS_SUGAR')
		iStone = gc.getInfoTypeForString('BONUS_STONE')
		iSalt = gc.getInfoTypeForString('BONUS_SALT')
		iTea = gc.getInfoTypeForString('BONUS_TEA')
		iToad = gc.getInfoTypeForString('BONUS_TOAD')
		iWheat = gc.getInfoTypeForString('BONUS_WHEAT')
		iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
#		iGrassFertile = gc.getInfoTypeForString('TERRAIN_GRASS_FERTILE')
		iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
		iMarsh = gc.getInfoTypeForString('TERRAIN_MARSH')
		iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
		iShallows = gc.getInfoTypeForString('TERRAIN_SHALLOWS')
		iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
		iBrokenLands = gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS')
		iBurningSands = gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')
		iFieldsOfPerdition = gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')
		iShallows = gc.getInfoTypeForString('TERRAIN_SHALLOWS')
		iMushroom = gc.getInfoTypeForString('TERRAIN_MUSHGROUND')
		iDarkMushroom = gc.getInfoTypeForString('TERRAIN_DARK_MUSHGROUND')
		
		print "QWERTZ"
		
		iCount = CyGame().getGlobalCounter()
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			iFeature = pPlot.getFeatureType()
			iTerrain = pPlot.getTerrainType()
			iBonus = pPlot.getBonusType(-1)
			iImprovement = pPlot.getImprovementType()
			iHellSpread = 0
			iRnd = CyGame().getSorenRandNum(99, "Sing")
			bUntouched = True
			if pPlot.isOwned():
				pPlayer = gc.getPlayer(pPlot.getOwner())
				iAlignment = pPlayer.getAlignment()
				if pPlayer.getCivilizationType() == iInfernal:
					iHellSpread = 100
					bUntouched = False
				elif bUntouched and pPlayer.getStateReligion() == iAshenVeil: 
					iHellSpread = iCount / 10
					bUntouched = False
				elif bUntouched and ((iCount >= 50 and iAlignment == iEvil) or (iCount >= 75 and iAlignment == iNeutral)):
					iX = pPlot.getX()
					iY = pPlot.getY()
					for iiX in range(iX-1, iX+2, 1):
						for iiY in range(iY-1, iY+2, 1):
							pAdjacentPlot = CyMap().plot(iiX,iiY)
							if pAdjacentPlot.isNone() == False:
								iTerrain2 = pAdjacentPlot.getTerrainType()
								if (iTerrain2 == iBrokenLands or iTerrain2 == iBurningSands or iTerrain2 == iFieldsOfPerdition or iTerrain2 == iShallows):
									iHellSpread = iCount / 10
					bUntouched = False
			elif (bUntouched and pPlot.isOwned() == false and iCount > 50):
				iX = pPlot.getX()
				iY = pPlot.getY()
				for iiX in range(iX-1, iX+2, 1):
					for iiY in range(iY-1, iY+2, 1):
						pAdjacentPlot = CyMap().plot(iiX,iiY)
						if pAdjacentPlot.isNone() == False:
							iTerrain2 = pAdjacentPlot.getTerrainType()
							if (iTerrain2 == iBrokenLands or iTerrain2 == iBurningSands or iTerrain2 == iFieldsOfPerdition or iTerrain2 == iShallows):
								iHellSpread = iCount / 10
				bUntouched = False
			if iRnd < iHellSpread:
				if iTerrain == iGrass:
					pPlot.setTerrainType(iBrokenLands, True, True)
				elif iTerrain == iDesert or iTerrain == iDarkMushroom:
					pPlot.setTerrainType(iBurningSands, True, True)
				elif iTerrain == iPlains:
					pPlot.setTerrainType(iFieldsOfPerdition, True, True)
				elif iTerrain == iMarsh:
					pPlot.setTerrainType(iShallows, True, True)
				elif iTerrain == iTundra or iTerrain == iMushroom:
					pPlot.setTerrainType(iFieldsOfPerdition, True, True)
				if (iBonus == iSheep or iBonus == iPig or iBonus == iDeer or iBonus == iFur):
					pPlot.setBonusType(iToad)
				elif (iBonus == iHorse or iBonus == iCow or iBonus == iBison):
					pPlot.setBonusType(iNightmare)
				elif (iBonus == iCotton or iBonus == iDye or iBonus == iSilk or iBonus == iTea):
					pPlot.setBonusType(iRazorweed)
				elif (iBonus == iBanana or iBonus == iSugar):
					pPlot.setBonusType(iGulagarm)
				elif (iBonus == iMarble or iBonus == iStone or iBonus == iSalt):
					pPlot.setBonusType(iSheutStone)
				elif (iBonus == iCorn or iBonus == iRice or iBonus == iWheat):
					pPlot.setBonusType(-1)
					pPlot.setImprovementType(iSnakePillar)
			if iTerrain == iBurningSands:
				if pPlot.isCity() == False:
					if pPlot.isPeak() == False:
						if CyGame().getSorenRandNum(100, "Flames") <= iFlamesSpreadChance:
							pPlot.setFeatureType(iFlames, 0)
							
	def doTurnKhazad(self, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)
		if pPlayer.getNumCities() > 0:
			iVault1 = gc.getInfoTypeForString('BUILDING_DWARVEN_VAULT_EMPTY')
			iVault2 = gc.getInfoTypeForString('BUILDING_DWARVEN_VAULT_LOW')
			iVault3 = gc.getInfoTypeForString('BUILDING_DWARVEN_VAULT')
			iVault4 = gc.getInfoTypeForString('BUILDING_DWARVEN_VAULT_STOCKED')
			iVault5 = gc.getInfoTypeForString('BUILDING_DWARVEN_VAULT_ABUNDANT')
			iVault6 = gc.getInfoTypeForString('BUILDING_DWARVEN_VAULT_FULL')
			iVault7 = gc.getInfoTypeForString('BUILDING_DWARVEN_VAULT_OVERFLOWING')
			iGold = pPlayer.getGold() / pPlayer.getNumCities()
			if iGold <= 49:
				iNewVault = iVault1
			if (iGold >= 50 and iGold <= 99):
				iNewVault = iVault2
			if (iGold >= 100 and iGold <= 199):
				iNewVault = iVault3
			if (iGold >= 200 and iGold <= 499):
				iNewVault = iVault4
			if (iGold >= 500 and iGold <= 749):
				iNewVault = iVault5
			if (iGold >= 750 and iGold <= 999):
				iNewVault = iVault6
			if iGold >= 1000:
				iNewVault = iVault7
			for pyCity in PyPlayer(iPlayer).getCityList():
				pCity = pyCity.GetCy()
				pCity.setNumRealBuilding(iVault1, 0)
				pCity.setNumRealBuilding(iVault2, 0)
				pCity.setNumRealBuilding(iVault3, 0)
				pCity.setNumRealBuilding(iVault4, 0)
				pCity.setNumRealBuilding(iVault5, 0)
				pCity.setNumRealBuilding(iVault6, 0)
				pCity.setNumRealBuilding(iVault7, 0)
				if pCity.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_KHAZAD'):
					pCity.setNumRealBuilding(iNewVault, 1)

	def doTurnLuchuirp(self, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)
		iBarnaxus = gc.getInfoTypeForString('UNITCLASS_BARNAXUS')
		if pPlayer.getUnitClassCount(iBarnaxus) > 0:
			py = PyPlayer(iPlayer)
			pBarnaxus = -1
			bEmp1 = False
			bEmp2 = False
			bEmp3 = False
			bEmp4 = False
			bEmp5 = False
			iCombat1 = gc.getInfoTypeForString('PROMOTION_COMBAT1')
			iCombat2 = gc.getInfoTypeForString('PROMOTION_COMBAT2')
			iCombat3 = gc.getInfoTypeForString('PROMOTION_COMBAT3')
			iCombat4 = gc.getInfoTypeForString('PROMOTION_COMBAT4')
			iCombat5 = gc.getInfoTypeForString('PROMOTION_COMBAT5')
			iEmpower1 = gc.getInfoTypeForString('PROMOTION_EMPOWER1')
			iEmpower2 = gc.getInfoTypeForString('PROMOTION_EMPOWER2')
			iEmpower3 = gc.getInfoTypeForString('PROMOTION_EMPOWER3')
			iEmpower4 = gc.getInfoTypeForString('PROMOTION_EMPOWER4')
			iEmpower5 = gc.getInfoTypeForString('PROMOTION_EMPOWER5')
			iGolem = gc.getInfoTypeForString('PROMOTION_GOLEM')

			lGolems = []
			for pUnit in py.getUnitList():
				if pUnit.getUnitClassType() == iBarnaxus :
					pBarnaxus = pUnit
				elif pUnit.isHasPromotion(iGolem) :
					lGolems.append(pUnit)
			if pBarnaxus != -1 :
				if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_CRAFTMASTER')):
					pBarnaxus.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), True)
					# BlackArchon: Giving Barnaxus Combat II might get him stuck in the promotion tree, so he should get something like Strong
					#pBarnaxus.setHasPromotion(gc.getInfoTypeForString('PROMOTION_COMBAT2'), True)
					pBarnaxus.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), True)
				bEmp1 = bool(pBarnaxus.isHasPromotion(iCombat1))
				bEmp2 = bool(pBarnaxus.isHasPromotion(iCombat2))
				bEmp3 = bool(pBarnaxus.isHasPromotion(iCombat3))
				bEmp4 = bool(pBarnaxus.isHasPromotion(iCombat4))
				bEmp5 = bool(pBarnaxus.isHasPromotion(iCombat5))
			for pUnit in lGolems :
				pUnit.setHasPromotion(iEmpower1, False)
				pUnit.setHasPromotion(iEmpower2, False)
				pUnit.setHasPromotion(iEmpower3, False)
				pUnit.setHasPromotion(iEmpower4, False)
				pUnit.setHasPromotion(iEmpower5, False)
				if bEmp1:
					pUnit.setHasPromotion(iEmpower1, True)
				if bEmp2:
					pUnit.setHasPromotion(iEmpower1, True)
					pUnit.setHasPromotion(iEmpower2, True)
				if bEmp3:
					pUnit.setHasPromotion(iEmpower1, True)
					pUnit.setHasPromotion(iEmpower2, True)
					pUnit.setHasPromotion(iEmpower3, True)
				if bEmp4:
					pUnit.setHasPromotion(iEmpower4, True)
				if bEmp5:
					pUnit.setHasPromotion(iEmpower5, True)

#			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_CRAFTMASTER')):
#				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), True)
#				# BlackArchon: Combat I doesn't grant +10 percent strength anymore, so we give them Combat II
#				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_AGGRESSIVE'), True)

		elif pPlayer.getUnitClassCount(gc.getInfoTypeForString('EQUIPMENTCLASS_PIECES_OF_BARNAXUS')) == 1:
			if pPlayer.getNumUnits()==1:
				py = PyPlayer(iPlayer)
				for pUnit in py.getUnitList():
					pUnit.kill(true,-1)

		if CyGame().getUnitClassCreatedCount(iBarnaxus)>0:
			if pPlayer.getUnitClassCount(iBarnaxus) == 0:
				if CyGame().getPiecesOfBarnaxusCounter()==0:
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('EQUIPMENT_PIECES_OF_BARNAXUS'), pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

	def doTurnCondantis(self, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)
		iCondantis = gc.getInfoTypeForString('PROMOTION_FOLLOWER_OF_DANALIN')
		for pUnit in PyPlayer(iPlayer).getUnitList():
			if pUnit.isHasPromotion(iCondantis):
				pUnit.kill(True, PlayerTypes.NO_PLAYER)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CONDANTIS_LEFT",()),'AS3D_SPELL_DISPEL_MAGIC',1, pUnit.getButton(),ColorTypes(7),pUnit.getX(),pUnit.getY(),True,True)

	def doArcaneLacuna(self, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)
		for pUnit in PyPlayer(iPlayer).getUnitList():
			if pUnit.getUnitCombatType() != gc.getInfoTypeForString('UNITCOMBAT_ANIMAL') or pUnit.getUnitCombatType() != gc.getInfoTypeForString('UNITCOMBAT_BEAST'):
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ARCANE_LACUNA'), True)
				
	def doTurnCheckAlignment(self, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)
		iPaladin = gc.getInfoTypeForString('UNITCLASS_PALADIN')
		iDruid = gc.getInfoTypeForString('UNITCLASS_DRUID')
		iIceDruid = gc.getInfoTypeForString('UNIT_ICE_DRUID')
		iEidolon = gc.getInfoTypeForString('UNITCLASS_EIDOLON')	
		if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):		
			for pUnit in PyPlayer(iPlayer).getUnitList():
			 if pUnit.getUnitClassType() == iEidolon or pUnit.getUnitClassType() == iDruid:	
				pUnit.kill(True, PlayerTypes.NO_PLAYER)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISAPPOINTED_BY_ALIGNMENT",()),'AS3D_SPELL_DISPEL_MAGIC',1, pUnit.getButton(),ColorTypes(7),pUnit.getX(),pUnit.getY(),True,True)
		if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_NEUTRAL'):		
			for pUnit in PyPlayer(iPlayer).getUnitList():
			 if pUnit.getUnitClassType() == iEidolon or pUnit.getUnitClassType() == iPaladin or pUnit.getUnitType() == iIceDruid:	
				pUnit.kill(True, PlayerTypes.NO_PLAYER)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISAPPOINTED_BY_ALIGNMENT",()),'AS3D_SPELL_DISPEL_MAGIC',1, pUnit.getButton(),ColorTypes(7),pUnit.getX(),pUnit.getY(),True,True)
		if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):		
			for pUnit in PyPlayer(iPlayer).getUnitList():
			 if (pUnit.getUnitClassType() == iDruid and pUnit.getUnitType() != iIceDruid) or pUnit.getUnitClassType() == iPaladin:	
				pUnit.kill(True, PlayerTypes.NO_PLAYER)
				CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_DISAPPOINTED_BY_ALIGNMENT",()),'AS3D_SPELL_DISPEL_MAGIC',1, pUnit.getButton(),ColorTypes(7),pUnit.getX(),pUnit.getY(),True,True)
			 
	def findClearPlot(self, pUnit, plot):
		BestPlot = -1
		iBestPlot = 0
		if pUnit == -1:
			iX = plot.getX()
			iY = plot.getY()
			for iiX in range(iX-1, iX+2, 1):
				for iiY in range(iY-1, iY+2, 1):
					iCurrentPlot = 0
					pPlot = CyMap().plot(iiX,iiY)
					if pPlot.isNone() == False:
						if pPlot.getNumUnits() == 0:
							if (pPlot.isWater() == plot.isWater() and pPlot.isPeak() == False and pPlot.isCity() == False):
								iCurrentPlot = iCurrentPlot + 5
						if iCurrentPlot >= 1:
							iCurrentPlot = iCurrentPlot + CyGame().getSorenRandNum(5, "FindClearPlot")
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
				if pPlot.isNone() == False:
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
						iCurrentPlot = iCurrentPlot + CyGame().getSorenRandNum(5, "FindClearPlot")
						if iCurrentPlot >= iBestPlot:
							BestPlot = pPlot
							iBestPlot = iCurrentPlot
		return BestPlot

	def genesis(self, iPlayer):
		iBrokenLands = gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS')
		iBurningSands = gc.getInfoTypeForString('TERRAIN_BURNING_SANDS')
		iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
		iFields = gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')
		iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
		iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
		iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
		iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
		iForestAncient = gc.getInfoTypeForString('FEATURE_FOREST_ANCIENT')
		iForest = gc.getInfoTypeForString('FEATURE_FOREST')
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getOwner() == iPlayer:
				iTerrain = pPlot.getTerrainType()
				if iTerrain == iSnow:
					pPlot.setTerrainType(iTundra,True,True)
				if iTerrain == iTundra:
					pPlot.setTerrainType(iPlains,True,True)
				if (iTerrain == iDesert or iTerrain == iBurningSands):
					pPlot.setTerrainType(iPlains,True,True)
				if (iTerrain == iPlains or iTerrain == iFields or iTerrain == iBrokenLands):
					pPlot.setTerrainType(iGrass,True,True)
				if (iTerrain == iGrass and pPlot.getImprovementType() == -1 and pPlot.getFeatureType() != iForestAncient and pPlot.isPeak() == False and pPlot.isCity() == False):
					pPlot.setFeatureType(iForest, 0)
				iTemp = pPlot.getFeatureType()
				pPlot.changePlotCounter(-100)
				if iTemp!=-1:
					pPlot.setFeatureType(iTemp, 0)

	def snowgenesis(self, iPlayer):
		iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
		iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
		iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
		iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
		iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getOwner() == iPlayer:
				pPlot.changePlotCounter(0)
				if(pPlot.getTerrainType() == iGrass):
					pPlot.setTerrainType(iSnow,True,True)
				elif(pPlot.getTerrainType() == iPlains):
					pPlot.setTerrainType(iSnow,True,True)
				elif(pPlot.getTerrainType() == iDesert):
					pPlot.setTerrainType(iTundra,True,True)

	def getAshenVeilCity(self, iNum):
		iInfernal = gc.getInfoTypeForString('CIVILIZATION_INFERNAL')
		iVeil = gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL')
		iBestValue1 = 0
		iBestValue2 = 0
		iBestValue3 = 0
		pBestCity1 = -1
		pBestCity2 = -1
		pBestCity3 = -1
		for iPlayer in range(gc.getMAX_PLAYERS()):
			pPlayer = gc.getPlayer(iPlayer)
			if (pPlayer.isAlive() and pPlayer.getCivilizationType() != iInfernal):
				for pyCity in PyPlayer(iPlayer).getCityList():
					pCity = pyCity.GetCy()
					if (pCity.isHasReligion(iVeil) and pCity.isCapital() == False):
						bValid = True
						iValue = pCity.getPopulation() * 100
						iValue += pCity.getCulture(iPlayer) / 3
						iValue += pCity.getNumBuildings() * 10
						iValue += pCity.getNumWorldWonders() * 100
						iValue += pCity.countNumImprovedPlots()
						if iValue > iBestValue1:
							iBestValue3 = iBestValue2
							pBestCity3 = pBestCity2
							iBestValue2 = iBestValue1
							pBestCity2 = pBestCity1
							iBestValue1 = iValue
							pBestCity1 = pCity
							bValid = False
						if (bValid and iValue > iBestValue2):
							iBestValue3 = iBestValue2
							pBestCity3 = pBestCity2
							iBestValue2 = iValue
							pBestCity2 = pCity
							bValid = False
						if (bValid and iValue > iBestValue3):
							iBestValue3 = iValue
							pBestCity3 = pCity
		if iNum == 1:
			return pBestCity1
		if iNum == 2:
			return pBestCity2
		if iNum == 3:
			return pBestCity3
		return -1

	def getCivilization(self, iCiv):
		i = -1
		for iPlayer in range(gc.getMAX_PLAYERS()):
			pPlayer = gc.getPlayer(iPlayer)
			if pPlayer.getCivilizationType() == iCiv:
				i = iPlayer
		return i

	def getHero(self, pPlayer):
		iHero = -1
		if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_KORINNA'):
			iHero = gc.getInfoTypeForString('UNITCLASS_RISEN_EMPEROR')
		elif pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_XIVAN'):
			iHero = gc.getInfoTypeForString('UNITCLASS_XIVAN')
		elif pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS'):
			iHero = gc.getInfoTypeForString('UNITCLASS_KORRINA')
			if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_KORRINA_BLACK_LADY')) > 0:
				iHero = gc.getInfoTypeForString('UNITCLASS_KORRINA_BLACK_LADY')
			elif pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_KORRINA_RED_LADY')) > 0:
				iHero = gc.getInfoTypeForString('UNITCLASS_KORRINA_RED_LADY')
			elif pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_KORRINA_WHITE_LADY')) > 0:
				iHero = gc.getInfoTypeForString('UNITCLASS_KORRINA_WHITE_LADY')
		else:
			iHeroUnit = gc.getCivilizationInfo(pPlayer.getCivilizationType()).getHero()
			if iHeroUnit != -1:
				iHero = gc.getUnitInfo(iHeroUnit).getUnitClassType()

		return iHero

	def getLeader(self, iLeader):
		i = -1
		for iPlayer in range(gc.getMAX_PLAYERS()):
			pPlayer = gc.getPlayer(iPlayer)
			if pPlayer.getLeaderType() == iLeader:
				i = iPlayer
		return i

	def getOpenPlayer(self):
		i = -1
		for iPlayer in range(gc.getMAX_PLAYERS()):
			pPlayer = gc.getPlayer(iPlayer)
			if (pPlayer.isEverAlive() == False and i == -1):
				i = iPlayer
		return i

	def getUnholyVersion(self, pUnit):
		iUnit = -1
		if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ADEPT'):
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 2:
				iUnit = gc.getInfoTypeForString('UNIT_IMP')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 3:
				iUnit = gc.getInfoTypeForString('UNIT_MAGE')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 4:
				iUnit = gc.getInfoTypeForString('UNIT_LICH')
		if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ANIMAL') or pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_BEAST'):
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 1:
				iUnit = gc.getInfoTypeForString('UNIT_SCOUT')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 2:
				iUnit = gc.getInfoTypeForString('UNIT_HELLHOUND')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 3:
				iUnit = gc.getInfoTypeForString('UNIT_ASSASSIN')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 4:
				iUnit = gc.getInfoTypeForString('UNIT_BEAST_OF_AGARES')
		if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ARCHER'):
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 2:
				iUnit = gc.getInfoTypeForString('UNIT_ARCHER')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 3:
				iUnit = gc.getInfoTypeForString('UNIT_LONGBOWMAN')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 4:
				iUnit = gc.getInfoTypeForString('UNIT_CROSSBOWMAN')
		if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_DISCIPLE'):
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 2:
				iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_THE_ASHEN_VEIL')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 3:
				iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_THE_VEIL')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 4:
				iUnit = gc.getInfoTypeForString('UNIT_EIDOLON')
		if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_MELEE'):
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 1:
				iUnit = gc.getInfoTypeForString('UNIT_SKELETON')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 2:
				iUnit = gc.getInfoTypeForString('UNIT_DISEASED_CORPSE')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 3:
				iUnit = gc.getInfoTypeForString('UNIT_CHAMPION')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 4:
				iUnit = gc.getInfoTypeForString('UNIT_BALOR')
		if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_MOUNTED'):
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 2:
				iUnit = gc.getInfoTypeForString('UNIT_HORSEMAN')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 3:
				iUnit = gc.getInfoTypeForString('UNIT_CHARIOT')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 4:
				iUnit = gc.getInfoTypeForString('UNIT_DEATH_KNIGHT')
		if pUnit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_RECON'):
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 1:
				iUnit = gc.getInfoTypeForString('UNIT_SCOUT')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 2:
				iUnit = gc.getInfoTypeForString('UNIT_HELLHOUND')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 3:
				iUnit = gc.getInfoTypeForString('UNIT_ASSASSIN')
			if gc.getUnitInfo(pUnit.getUnitType()).getTier() == 4:
				iUnit = gc.getInfoTypeForString('UNIT_BEASTMASTER')
		return iUnit

	def getUnitPlayerID(self, pUnit):
		pPlayer = gc.getPlayer(pUnit.getOwner())
		iID = pUnit.getID()
		iUnitID = -1
		for iUnit in range(pPlayer.getNumUnits()):
			pLoopUnit = pPlayer.getUnit(iUnit)
			if pLoopUnit.getID() == iID:
				iUnitID = iUnit
		return iUnitID

#	Changed in Frozen: TC01
#	This function is altered in the Frozen civ module by TC01. I have seperated it into two parts, one called if the module is loaded, one if the module is not. I have also highlighted
#specific code changes I made to the first half of the function with additional code statements.
	def giftUnit(self, iUnit, iCivilization, iXP, pFromPlot, iFromPlayer):
		iAngel = gc.getInfoTypeForString('UNIT_ANGEL')
		iManes = gc.getInfoTypeForString('UNIT_MANES')
		if (iUnit == iAngel or iUnit == iManes):
			iChance = 100 - (CyGame().countCivPlayersAlive() * 3)
			iChance = iChance + iXP/100
			if iChance < 5:
				iChance = 5
			if iChance > 95:
				iChance = 95
			if CyGame().getSorenRandNum(100, "Gift Unit") > iChance:
				iUnit = -1

		if iUnit != -1:
			bValid = False
			for iPlayer in range (gc.getMAX_CIV_PLAYERS()):
				pPlayer = gc.getPlayer(iPlayer)
				py = PyPlayer(iPlayer)


				if pPlayer.getCivilizationType()==iCivilization:
#Changed in Frozen: TC01
					iFrozenSouls = gc.getInfoTypeForString('UNIT_FROZEN_SOUL')
					if iUnit == iFrozenSouls:
						iChance=100*pPlayer.getAveragePopulation()
						iChance/=15
						if iChance>98:
							iChance=98
						if CyGame().getSorenRandNum(100, "Gift Frozen Soul") < iChance:
							break
#End of Frozen
					if pPlayer.getNumCities() > 0:
						iRnd = CyGame().getSorenRandNum(py.getNumCities(), "Gift Unit")
						pCity = py.getCityList()[iRnd]
						pPlot = pCity.plot()
						newUnit = pPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit.changeExperience(iXP, -1, false, false, false)
						if (pFromPlot != -1 and gc.getPlayer(iFromPlayer).isHuman()):
							bValid = True
						if pPlayer.isHuman():
							if iUnit == iManes:
								CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ADD_MANES",()),'AS2D_UNIT_FALLS',1,'Art/Interface/Buttons/Promotions/Demon.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
							if iUnit == iAngel:
								CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ADD_ANGEL",()),'AS2D_UNIT_FALLS',1,'Art/Interface/Buttons/Promotions/Angel.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
#Changed in Frozen: TC01
							if iUnit == iFrozenSouls:
								CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ADD_FROZEN_SOULS",()),'AS2D_UNIT_FALLS',1,'Art/Interface/Buttons/Promotions/Demon.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)
						if (pPlayer.isHuman() == False and (iUnit == iManes or iUnit == iFrozenSouls) and pCity != -1):
#End of Frozen
							if CyGame().getSorenRandNum(100, "Manes") < (100 - (pCity.getPopulation() * 5)):
								pCity.changePopulation(1)
								newUnit.kill(True, PlayerTypes.NO_PLAYER)
			if bValid:
				if iUnit == iManes:
					CyInterface().addMessage(iFromPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_UNIT_FALLS",()),'AS2D_UNIT_FALLS',1,'Art/Interface/Buttons/Promotions/Demon.dds',ColorTypes(7),pFromPlot.getX(),pFromPlot.getY(),True,True)
				if iUnit == iAngel:
					CyInterface().addMessage(iFromPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_UNIT_RISES",()),'AS2D_UNIT_FALLS',1,'Art/Interface/Buttons/Promotions/Angel.dds',ColorTypes(7),pFromPlot.getX(),pFromPlot.getY(),True,True)
#Added in Frozen: TC01
				if iUnit == iFrozenSouls:
					CyInterface().addMessage(iFromPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_UNIT_FREEZES",()),'AS2D_UNIT_FALLS',1,'Art/Interface/Buttons/Promotions/Demon.dds',ColorTypes(7),pFromPlot.getX(),pFromPlot.getY(),True,True)
#End of Frozen

	def placeTreasure(self, iPlayer, iUnit):
		pPlayer = gc.getPlayer(iPlayer)
		pBestPlot = -1
		iBestPlot = -1
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			iPlot = -1
			if not pPlot.isWater():
				if pPlot.getNumUnits() == 0:
					if not pPlot.isCity():
						if not pPlot.isImpassable():
							iPlot = CyGame().getSorenRandNum(1000, "Add Unit")
							if pPlot.area().getNumTiles() < 8:
								iPlot += 1000
							if not pPlot.isOwned():
								iPlot += 1000
							if iPlot > iBestPlot:
								iBestPlot = iPlot
								pBestPlot = pPlot
		if iBestPlot != -1:
			newUnit = pPlayer.initUnit(iUnit, pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_EXPLORE_LAIR_TREASURE",()),'',1,'Art/Interface/Buttons/Equipment/Treasure.dds',ColorTypes(8),newUnit.getX(),newUnit.getY(),True,True)
			CyCamera().JustLookAtPlot(pBestPlot)

	def showUniqueImprovements(self, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)
		iTeam = pPlayer.getTeam()
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getImprovementType() != -1:
				if gc.getImprovementInfo(pPlot.getImprovementType()).isUnique():
					pPlot.setRevealed(iTeam, True, False, TeamTypes.NO_TEAM)

	def startWar(self, iPlayer, i2Player, iWarPlan):
		iTeam = gc.getPlayer(iPlayer).getTeam()
		i2Team = gc.getPlayer(i2Player).getTeam()
		eTeam = gc.getTeam(iTeam)
		e2Team = gc.getTeam(i2Team)
		if eTeam.isAlive():
			if e2Team.isAlive():
				if not eTeam.isAtWar(i2Team):
					if iTeam != i2Team:
						if eTeam.isHasMet(i2Team):
							if not eTeam.isPermanentWarPeace(i2Team):
								eTeam.declareWar(i2Team, false, iWarPlan)

	def warScript(self, iPlayer):
		pPlayer = gc.getPlayer(iPlayer)
		iEnemy = -1
		for iPlayer2 in range(gc.getMAX_PLAYERS()):
			pPlayer2 = gc.getPlayer(iPlayer2)
			if pPlayer2.isAlive():
				iTeam = gc.getPlayer(iPlayer).getTeam()
				iTeam2 = gc.getPlayer(iPlayer2).getTeam()
				eTeam = gc.getTeam(iTeam)
				if eTeam.isAVassal() == False:
					if eTeam.isAtWar(iTeam2):
						if CyGame().getSorenRandNum(100, "War Script") < 5:
							self.dogpile(iPlayer, iPlayer2)
							
					if self.warScriptAllow(iPlayer, iPlayer2):
						if pPlayer2.getBuildingClassMaking(gc.getInfoTypeForString('BUILDINGCLASS_TOWER_OF_MASTERY')) > 0:
							if eTeam.getAtWarCount(True) == 0:
								self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
								
						if (pPlayer2.getNumBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_DIVINE')) > 0 or pPlayer2.getNumBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_EXALTED')) > 0):
							if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
								if eTeam.getAtWarCount(True) == 0:
									self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
									
						if (pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_ESSENCE_OF_LAW')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_VIRTUE_LUGUS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DEDICATION')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_SACRED_WOODS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_NIGHTMARISH_EPIPHANY')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_INFERNAL_PACT'))):
							if (pPlayer.getStateReligion() != pPlayer2.getStateReligion()):
								if pPlayer.getAlignment() != pPlayer2.getAlignment():
									iRand = CyGame().getSorenRandNum(100, "Holy War")
									if iRand > 50:
										if eTeam.getAtWarCount(True) == 0:
											self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
						if (pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_JUDGEMENT_OF_JUNIL')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DEVOTION_TO_LUGUS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_TRADITION_OF_THE_GODDESS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_HIDDEN_PATHS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_COMPLACENCY_OO')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_INFERNAL_POSSESION'))):
							if (pPlayer.getStateReligion() != pPlayer2.getStateReligion()):
								iRand = CyGame().getSorenRandNum(100, "Holy War")
								if pPlayer.getAlignment() != pPlayer2.getAlignment():
									iRand = iRand + 25
								if iRand > 50:
									if eTeam.getAtWarCount(True) == 0:
										self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
						if (pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DIVINE_JUNIL')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DIVINE_LUGUS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DIVINE_KILMORPH')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DIVINE_CERNUNNOS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DIVINE_OVERLORDS')) or pPlayer2.isHasTech(gc.getInfoTypeForString('TECH_DIVINE_AGARES'))):
							if (pPlayer.getStateReligion() != pPlayer2.getStateReligion()):
								if eTeam.getAtWarCount(True) == 0:
									self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
									
						if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_MERCURIANS'):
							if pPlayer2.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
								self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
								
						if CyGame().getGlobalCounter() > 20:
							if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SVARTALFAR'):
								if (pPlayer2.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_LJOSALFAR') and CyGame().getPlayerRank(iPlayer) > CyGame().getPlayerRank(iPlayer2)):
									self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
							if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_LJOSALFAR'):
								if (pPlayer2.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SVARTALFAR') and CyGame().getPlayerRank(iPlayer) > CyGame().getPlayerRank(iPlayer2)):
									self.startWar(iPlayer, iPlayer2, WarPlanTypes.WARPLAN_TOTAL)
						if (CyGame().getGlobalCounter() > 40 or pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL') or pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_DOVIELLO')):
							if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
								if (eTeam.getAtWarCount(True) == 0 and CyGame().getPlayerRank(iPlayer2) > CyGame().getPlayerRank(iPlayer)):
									if (iEnemy == -1 or CyGame().getPlayerRank(iPlayer2) > CyGame().getPlayerRank(iEnemy)):
										iEnemy = iPlayer2
		if iEnemy != -1:
			if CyGame().getPlayerRank(iPlayer) > CyGame().getPlayerRank(iEnemy):
				self.startWar(iPlayer, iEnemy, WarPlanTypes.WARPLAN_TOTAL)

	def warScriptAllow(self, iPlayer, iPlayer2):
		pPlayer = gc.getPlayer(iPlayer)
		pPlayer2 = gc.getPlayer(iPlayer2)
		iTeam = gc.getPlayer(iPlayer).getTeam()
		iTeam2 = gc.getPlayer(iPlayer2).getTeam()
		eTeam = gc.getTeam(iTeam)
		if iPlayer == gc.getBARBARIAN_PLAYER():
			return False
		if eTeam.isHasMet(iTeam2) == False:
			return False
		if eTeam.AI_getAtPeaceCounter(iTeam2) < 20:
			return False
#		if pPlayer.AI_getAttitude(iPlayer2) <= gc.getLeaderHeadInfo(pPlayer.getLeaderType()).getDeclareWarRefuseAttitudeThreshold():
#			return False
		if eTeam.isAtWar(iTeam2):
			return False
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
			if pPlayer2.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
				return False
		return True

	def dogpile(self, iPlayer, iVictim):
		pPlayer = gc.getPlayer(iPlayer)
		for iPlayer2 in range(gc.getMAX_PLAYERS()):
			pPlayer2 = gc.getPlayer(iPlayer2)
			iChance = -1
			if pPlayer2.isAlive():
				if (self.dogPileAllow(iPlayer, iPlayer2) and self.warScriptAllow(iPlayer2, iVictim)):
					iChance = pPlayer2.AI_getAttitude(iPlayer) * 5
					if iChance > 0:
						iChance = iChance - (pPlayer2.AI_getAttitude(iVictim) * 5) - 10
						if CyGame().isOption(gc.getInfoTypeForString('GAMEOPTION_AGGRESSIVE_AI')) == False:
							iChance = iChance - 10
						if iChance > 0:
							iChance = iChance + (CyGame().getGlobalCounter() / 3)
							if pPlayer2.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_BALSERAPHS'):
								iChance = CyGame().getSorenRandNum(50, "Dogpile")
							if CyGame().getSorenRandNum(100, "Dogpile") < iChance:
								self.startWar(iPlayer2, iVictim, WarPlanTypes.WARPLAN_DOGPILE)

	def dogPileAllow(self, iPlayer, iPlayer2):
		pPlayer = gc.getPlayer(iPlayer)
		pPlayer2 = gc.getPlayer(iPlayer2)
		iTeam = gc.getPlayer(iPlayer).getTeam()
		iTeam2 = gc.getPlayer(iPlayer2).getTeam()
		if iPlayer == iPlayer2:
			return False
		if iTeam == iTeam2:
			return False
		if pPlayer2.isHuman():
			return False
		if pPlayer2.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ELOHIM'):
			return False
		if gc.getTeam(iTeam2).isAVassal():
			return False
		return True

	def warn(self, iPlayer, szText, pPlot):
		pPlayer = gc.getPlayer(iPlayer)
		for iPlayer2 in range(gc.getMAX_PLAYERS()):
			pPlayer2 = gc.getPlayer(iPlayer2)
			if (pPlayer2.isAlive() and iPlayer != iPlayer2):
				if pPlayer2.isHuman():
					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON)
					popupInfo.setText(szText)
					popupInfo.setOnClickedPythonCallback("selectWarn")
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_MAIN_MENU_OK",()), "")
					popupInfo.addPopup(iPlayer2)
				if pPlot != -1:
					CyInterface().addMessage(iPlayer2,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ALTAR_OF_THE_LUONNOTAR",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Buildings/AltaroftheLuonnotar.dds',ColorTypes(7),pPlot.getX(),pPlot.getY(),True,True)

	def MarnokNameGenerator(self, unit):
		pPlayer = gc.getPlayer(unit.getOwner())
		pCiv = pPlayer.getCivilizationType()
		pReligion = pPlayer.getStateReligion()
		pAlign = pPlayer.getAlignment()

		lPre=["ta","go","da","bar","arc","ken","an","ad","mi","kon","kar","mar","wal","he", "ha", "re", "ar", "bal", "bel", "bo", "bri", "car","dag","dan","ma","ja","co","be","ga","qui","sa"]
		lMid=["ad","z","the","and","tha","ent","ion","tio","for","tis","oft","che","gan","an","en","wen","on","d","n","g","t","ow","dal"]
		lEnd=["ar","sta","na","is","el","es","ie","us","un","th", "er","on","an","re","in","ed","nd","at","en","le","man","ck","ton","nok","git","us","or","a","da","u","cha","ir"]

		lEpithet=["red","blue","black","grey","white","strong","brave","old","young","great","slayer","hunter","seeker"]
		lNoun=["spirit","soul","boon","born","staff","rod","shield","autumn","winter","spring","summer","wit","horn","tusk","glory","claw","tooth","head","heart", "blood","breath", "blade", "hand", "lover","bringer","maker","taker","river","stream","moon","star","face","foot","half","one","hundred","thousand"]
		lSchema=["CPME","CPMESCPME","CPESCPE","CPE","CPMME","CPMDCME","CPMAME","KCPMESCUM","CPMME[ the ]CX", "CPMESCXN", "CPMME[ of ]CPMME", "CNNSCXN"]

		if pAlign == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
			lNoun = lNoun + ["fear","terror","reign","brood","snare","war","strife","pain","hate","evil","hell","misery","murder","anger","fury","rage","spawn","sly","blood","bone","scythe","slave","bound","ooze","scum"]
			lEpithet=["dark","black","white","cruel","foul"]

		if pReligion == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
			lEpithet = lEpithet + ["fallen","diseased","infernal","profane","corrupt"]
			lSchema = lSchema + ["CPME[ the ]CX"]
		if pReligion == gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS'):
			lPre = lPre + ["cth","cht","shu","az","ts","dag","hy","gla","gh","rh","x","ll"]
			lMid = lMid + ["ul","tha","on","ug","st","oi"]
			lEnd = lEnd + ["hu","on", "ha","ua","oa","uth","oth","ath","thua", "thoa","ur","ll","og","hua"]
			lEpithet = lEpithet + ["nameless","webbed","deep","watery"]
			lNoun = lNoun + ["tentacle","wind","wave","sea","ocean","dark","crab","abyss","island"]
			lSchema = lSchema + ["CPMME","CPDMME","CPAMAME","CPMAME","CPAMAMEDCPAMAE"]
		if pReligion == gc.getInfoTypeForString('RELIGION_THE_ORDER'):
			lPre = lPre + ["ph","v","j"]
			lMid = lMid + ["an","al","un"]
			lEnd = lEnd + ["uel","in","il"]
			lEpithet = lEpithet + ["confessor","crusader", "faithful","obedient","good"]
			lNoun = lNoun + ["order", "faith", "heaven","law"]
			lSchema = lSchema + ["CPESCPME","CPMESCPE","CPMESCPME", "CPESCPE"]
		if pReligion == gc.getInfoTypeForString('RELIGION_FELLOWSHIP_OF_LEAVES'):
			lPre = lPre + ["ki","ky","yv"]
			lMid = lMid + ["th","ri"]
			lEnd = lEnd + ["ra","el","ain"]
			lEpithet = lEpithet + ["green"]
			lNoun = lNoun + ["tree","bush","wood","berry","elm","willow","oak","leaf","flower","blossom"]
			lSchema = lSchema + ["CPESCN","CPMESCNN","CPMESCXN"]
		if pReligion == gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH'):
			lPre = lPre + ["bam","ar","khel","ki"]
			lMid = lMid + ["th","b","en"]
			lEnd = lEnd + ["ur","dain","ain","don"]
			lEpithet = lEpithet + ["deep","guard","miner"]
			lNoun = lNoun + ["rune","flint","slate","stone","rock","iron","copper","mithril","thane","umber"]
			lSchema = lSchema + ["CPME","CPMME"]
		if pReligion == gc.getInfoTypeForString('RELIGION_THE_EMPYREAN'):
			lEpithet = lEpithet + ["radiant","holy"]
			lNoun = lNoun + ["honor"]

		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED')) == True:
			# I have left this as a copy of the Barbarian, see how it goes, this might do the trick. I plan to use it when there is a chance a unit will go Barbarian anyway.
			lPre = lPre + ["gru","bra","no","os","dir","ka","z"]
			lMid = lMid + ["g","ck","gg","sh","b","bh","aa"]
			lEnd = lEnd + ["al","e","ek","esh","ol","olg","alg"]
			lNoun = lNoun + ["death", "hate", "rage", "mad","insane","berserk"]
			lEpithet = lEpithet + ["smasher", "breaker", "mangle","monger"]

		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED')) == True:
			# might want to tone this down, because I plan to use it as possession/driven to madness, less than madcap zaniness.
			lPre = lPre + ["mad","pim","zi","zo","fli","mum","dum","odd","slur"]
			lMid = lMid + ["bl","pl","gg","ug","bl","b","zz","abb","odd"]
			lEnd = lEnd + ["ad","ap","izzle","onk","ing","er","po","eep","oggle","y"]

		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_VAMPIRE')) == True:
			lPre = lPre + ["dra","al","nos","vam","vla","tep","bat","bar","cor","lil","ray","zar","stra","le"]
			lMid = lMid + ["cul","u","car","fer","pir","or","na","ov","sta"]
			lEnd = lEnd + ["a","d","u","e","es","y","bas","vin","ith","ne","ak","ich","hd","t"]

		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON')) == True:
			lPre = lPre + ["aa","ab","adr","ah","al","de","ba","cro","da","be","eu","el","ha","ib","me","she","sth","z"]
			lMid = lMid + ["rax","lia","ri","al","as","b","bh","aa","al","ze","phi","sto","phe","cc","ee"]
			lEnd = lEnd + ["tor","tan","ept","lu","res","ah","mon","gon","bul","gul","lis","les","uz"]
			lSchema = ["CPMMME","CPMACME", "CPKMAUAPUE", "CPMMME[ the ]CNX"]

		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_HILL_GIANT'):
			lPre = lPre + ["gor","gra","gar","gi","gol"]
			lMid = lMid + ["gan","li","ri","go"]
			lEnd = lEnd + ["tus","tan","ath","tha"]
			lSchema = lSchema +["CXNSCNN","CPESCNE", "CPMME[ the ]CX"]
			lEpithet = lEpithet + ["large","huge","collossal","brutal","basher","smasher","crasher","crusher"]
			lNoun = lNoun + ["fist","tor","hill","brute","stomp"]

		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_LIZARDMAN'):
			lPre = lPre + ["ss","s","th","sth","hss"]
			lEnd = lEnd + ["ess","iss","ath","tha"]
			lEpithet = lEpithet + ["cold"]
			lNoun = lNoun + ["hiss","tongue","slither","scale","tail","ruin"]
			lSchema = lSchema + ["CPAECPAE","CPAKECPAU","CPAMMAE"]
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_FIRE_ELEMENTAL') or unit.getUnitType() == gc.getInfoTypeForString('UNIT_AZER'):
			lPre = lPre + ["ss","cra","th","sth","hss","roa"]
			lMid = lMid + ["ss","ck","rr","oa","iss","tt"]
			lEnd = lEnd + ["le","iss","st","r","er"]
			lNoun = lNoun + ["hot", "burn","scald","roast","flame","scorch","char","sear","singe","fire","spit"]
			lSchema = ["CNN","CNX","CPME","CPME[ the ]CNX","CPMME","CNNSCPME"]
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_WATER_ELEMENTAL'):
			lPre = lPre + ["who","spl","dr","sl","spr","sw","b"]
			lMid = lMid + ["o","a","i","ub","ib"]
			lEnd = lEnd + ["sh","p","ter","ble"]
			lNoun = lNoun + ["wave","lap","sea","lake","water","tide","surf","spray","wet","damp","soak","gurgle","bubble"]
			lSchema = ["CNN","CNX","CPME","CPME[ the ]CNX","CPMME","CNNSCPME"]
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_AIR_ELEMENTAL'):
			lPre = lPre + ["ff","ph","th","ff","ph","th"]
			lMid = lMid + ["oo","aa","ee","ah","oh"]
			lEnd = lEnd + ["ff","ph","th","ff","ph","th"]
			lNoun = lNoun + ["wind","air","zephyr","breeze","gust","blast","blow"]
			lSchema = ["CNN","CNX","CPME","CPME[ the ]CNX","CPMME","CNNSCPME"]
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_EARTH_ELEMENTAL'):
			lPre = lPre + ["gra","gro","kro","ff","ph","th"]
			lMid = lMid + ["o","a","u"]
			lEnd = lEnd + ["ck","g","k"]
			lNoun = lNoun + ["rock","stone","boulder","slate","granite","rumble","quake"]
			lSchema = ["CNN","CNX","CPME","CPME[ the ]CNX","CPMME","CNNSCPME"]

		# SEA BASED
		# Check for ships - special schemas
		if unit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_NAVAL'):
			lEnd = lEnd + ["ton","town","port"]
			lNoun = lNoun + ["lady","jolly","keel","bow","stern", "mast","sail","deck","hull","reef","wave"]
			lEpithet = lEpithet + ["sea", "red", "blue","grand","barnacle","gull"]
			lSchema = ["[The ]CNN", "[The ]CXN", "[The ]CNX","[The ]CNSCN", "[The ]CNSCX","CPME['s ]CN","[The ]CPME", "[The ]CNX","CNX","CN['s ]CN"]

		# # #
		# Pick a Schema
		sSchema = lSchema[CyGame().getSorenRandNum(len(lSchema), "Name Gen")-1]
		sFull = ""
		sKeep = ""
		iUpper = 0
		iKeep = 0
		iSkip = 0

		# Run through each character in schema to generate name
		for iCount in range (0,len(sSchema)):
			sAdd=""
			iDone = 0
			sAction = sSchema[iCount]
			if iSkip == 1:
				if sAction == "]":
					iSkip = 0
				else:
					sAdd = sAction
					iDone = 1
			else:					# MAIN SECTION
				if sAction == "P": 	# Pre 	: beginnings of names
					sAdd = lPre[CyGame().getSorenRandNum(len(lPre), "Name Gen")-1]
					iDone = 1
				if sAction == "M":	# Mid 	: middle syllables
					sAdd = lMid[CyGame().getSorenRandNum(len(lMid), "Name Gen")-1]
					iDone = 1
				if sAction == "E":	# End	: end of names
					sAdd = lEnd[CyGame().getSorenRandNum(len(lEnd), "Name Gen")-1]
					iDone = 1
				if sAction == "X":	# Epithet	: epithet word part
					#epithets ("e" was taken!)
					sAdd = lEpithet[CyGame().getSorenRandNum(len(lEpithet), "Name Gen")-1]
					iDone = 1
				if sAction == "N":	# Noun	: noun word part
					#noun
					sAdd = lNoun[CyGame().getSorenRandNum(len(lNoun), "Name Gen")-1]
					iDone = 1
				if sAction == "S":	# Space	: a space character. (Introduced before [ ] was possible )
					sAdd =  " "
					iDone = 1
				if sAction == "D":	# Dash	: a - character. Thought to be common and useful enough to warrant inclusion : Introduced before [-] was possible
					sAdd =  "-"
					iDone = 1
				if sAction == "A":	# '		: a ' character - as for -, introduced early
					sAdd = "'"
					iDone = 1
				if sAction == "C":	# Caps	: capitalizes first letter of next phrase generated. No effect on non-letters.
					iUpper = 1
				if sAction == "K":	# Keep	: stores the next phrase generated for re-use with U
					iKeep = 1
				if sAction == "U":	# Use	: re-uses a stored phrase.
					sAdd = sKeep
					iDone = 1
				if sAction == "[":	# Print	: anything between [] is added to the final phrase "as is". Useful for [ the ] and [ of ] among others.
					iSkip = 1
			# capitalizes phrase once.
			if iUpper == 1 and iDone == 1:
				sAdd = sAdd.capitalize()
				iUpper = 0
			# stores the next phrase generated.
			if iKeep == 1 and iDone == 1:
				sKeep = sAdd
				iKeep = 0
			# only adds the phrase if a new bit was actally created.
			if iDone == 1:
				sFull = sFull + sAdd

		# trim name length
		if len(sFull) > 25:
			sFull = sFull[:25]
		#CyInterface().addMessage(caster.getOwner(),True,25,"NAME : "+sFull,'AS2D_POSITIVE_DINK',1,'Art/Interface/Buttons/Spells/Rob Grave.dds',ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)

		return sFull

	def BarbCityProduction(self, pCity):

		ePlayer = pCity.getOwner()
		pPlayer = gc.getPlayer(ePlayer)
		player = PyPlayer(ePlayer)
		civtype = pPlayer.getCivilizationType()
		infoCiv = gc.getCivilizationInfo(civtype)
		eTeam = gc.getTeam(pPlayer.getTeam())
		pPlot = pCity.plot()

		if pPlayer.isHuman():
			return False

		sProd = ['UNITCLASS_WARRIOR']
		sProd = sProd + ['UNITCLASS_ADEPT','UNITCLASS_AXEMAN','UNITCLASS_HUNTER','UNITCLASS_ARCHER','UNITCLASS_HORSEMAN']
		sProd = sProd + ['UNITCLASS_CHAMPION','UNITCLASS_CHARIOT','UNITCLASS_RANGER','UNITCLASS_LONGBOWMAN','UNITCLASS_ASSASSIN']
		sProd = sProd + ['UNITCLASS_ACHERON','UNITCLASS_TUMTUM']

		iValue=[0]
		for i in range (len(sProd)):
			iValue+=[0]

		iValue[sProd.index('UNITCLASS_AXEMAN')]=19500
		iValue[sProd.index('UNITCLASS_ARCHER')]=19000+650/(1+pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_ARCHER')))
		iValue[sProd.index('UNITCLASS_HUNTER')]=19400+200/(1+pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_HUNTER')))
		iValue[sProd.index('UNITCLASS_HORSEMAN')]=19400+200/(1+pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_HORSEMAN')))
		iValue[sProd.index('UNITCLASS_WARRIOR')]=19002

		if pPlayer.getUnitClassCountPlusMaking(gc.getInfoTypeForString('UNITCLASS_TUMTUM'))==0:
			if gc.getGame().getUnitClassCreatedCount(gc.getInfoTypeForString('UNITCLASS_TUMTUM'))==0:
				iValue[sProd.index('UNITCLASS_TUMTUM')]=100000

#Value higher Tier units more

		if (iValue[sProd.index('UNITCLASS_WARRIOR')]>iValue[sProd.index('UNITCLASS_AXEMAN')]):
			iValue[sProd.index('UNITCLASS_AXEMAN')]=iValue[sProd.index('UNITCLASS_WARRIOR')]+1000
		if (iValue[sProd.index('UNITCLASS_WARRIOR')]>iValue[sProd.index('UNITCLASS_ARCHER')]):
			iValue[sProd.index('UNITCLASS_ARCHER')]=iValue[sProd.index('UNITCLASS_WARRIOR')]+1000
		if (iValue[sProd.index('UNITCLASS_WARRIOR')]>iValue[sProd.index('UNITCLASS_HUNTER')]):
			iValue[sProd.index('UNITCLASS_HUNTER')]=iValue[sProd.index('UNITCLASS_WARRIOR')]+1000

		if (iValue[sProd.index('UNITCLASS_AXEMAN')]>0):
			iValue[sProd.index('UNITCLASS_CHAMPION')]+=iValue[sProd.index('UNITCLASS_AXEMAN')]+1000

		if iValue[sProd.index('UNITCLASS_ARCHER')]>0:
			iValue[sProd.index('UNITCLASS_LONGBOWMAN')]+=iValue[sProd.index('UNITCLASS_ARCHER')]+1000

		if iValue[sProd.index('UNITCLASS_HUNTER')]>0:
			iValue[sProd.index('UNITCLASS_RANGER')]+=iValue[sProd.index('UNITCLASS_HUNTER')]+1000
			iValue[sProd.index('UNITCLASS_ASSASSIN')]+=iValue[sProd.index('UNITCLASS_HUNTER')]+1000

		if iValue[sProd.index('UNITCLASS_HORSEMAN')]>0:
			iValue[sProd.index('UNITCLASS_CHARIOT')]+=iValue[sProd.index('UNITCLASS_HORSEMAN')]+1000


		infoCiv = gc.getCivilizationInfo(civtype)
		iBestBuilding=-1
		iBestBuildingValue=0
		for i in range(len(sProd)):
			if iValue[i]>iBestBuildingValue:
				iUnit = infoCiv.getCivilizationUnits(gc.getInfoTypeForString(sProd[i]))
				if iUnit != -1:
					if pCity.canTrain(iUnit,True,False):
						iBestBuildingValue=iValue[i]
						iBestBuilding=i

		if iUnit==gc.getInfoTypeForString('UNIT_ACHERON'):
##--------	BUGFfH: Modified by Denev 2009/10/12
#			CyInterface().addMessage(0,true,25,"Travellers from far away speak of a mighty Dragon, who wants to make %s his lair." %(pCity.getName()),'',0,'',ColorTypes(11), pPlot.getX(), pPlot.getY(), True,True)
			CyInterface().addMessage(0, true, 25, CyTranslator().getText("TXT_KEY_MESSAGE_START_PRODUCTION_ACHERON", (pCity.getName(), )), '', 0, '', ColorTypes(11), pPlot.getX(), pPlot.getY(), True, True)
##--------	BUGFfH: End Modify

		if iBestBuilding != -1:
			iUnit = infoCiv.getCivilizationUnits(gc.getInfoTypeForString(sProd[iBestBuilding]))
			if iUnit == gc.getInfoTypeForString('UNIT_ACHERON'):  # Check if the unit is Acheron
				# If it's Acheron, use the special AI type for Acheron
				pCity.pushOrder(OrderTypes.ORDER_TRAIN, iUnit, gc.getInfoTypeForString('UNITAI_ACHERON'), False, False, False, False)
			elif (gc.getUnitInfo(iUnit).getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_ARCHER')):
				# If the unit is an archer, set it to city defense
				pCity.pushOrder(OrderTypes.ORDER_TRAIN, iUnit, gc.getInfoTypeForString('UNITAI_CITY_DEFENSE'), False, False, False, False)
			else:
				# Otherwise, set the unit to attack city
				pCity.pushOrder(OrderTypes.ORDER_TRAIN, iUnit, gc.getInfoTypeForString('UNITAI_ATTACK_CITY'), False, False, False, False)
			return 1

		return False


#	Added in Frozen: TC01
#	This function is called when a city is captured by the Frozen and a Temple of the Hand is built via python. This applies the effects.

	def doTempleWhiteHand(self, pCity):
		iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
		iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
		iFloodPlains = gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')
		iForest = gc.getInfoTypeForString('FEATURE_FOREST')
		iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
		iScrub = gc.getInfoTypeForString('FEATURE_SCRUB')
		iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
		iX = pCity.getX()
		iY = pCity.getY()
		for iiX in range(iX-2, iX+3, 1):
			for iiY in range(iY-2, iY+3, 1):
				pLoopPlot = CyMap().plot(iiX,iiY)
				if not pLoopPlot.isNone():
					if not pLoopPlot.isWater():
						pLoopPlot.setTerrainType(iSnow, True, True)
						if pLoopPlot.getImprovementType() == iSmoke:
							pLoopPlot.setImprovementType(-1)
						iFeature = pLoopPlot.getFeatureType()
						if iFeature == iForest:
							pLoopPlot.setFeatureType(iForest, 2)
						if iFeature == iJungle:
							pLoopPlot.setFeatureType(iForest, 2)
						if iFeature == iFlames:
							pLoopPlot.setFeatureType(-1, -1)
						if iFeature == iFloodPlains:
							pLoopPlot.setFeatureType(-1, -1)
						if iFeature == iScrub:
							pLoopPlot.setFeatureType(-1, -1)
#End of Frozen

	def processWU_POPUP(self, unit, player):
		#SpyFanatic: Do not display in case of AIAutoPlay
		if CyGame().getAIAutoPlay() > 0:
			return

		if isWorldUnitClass(unit.getUnitClassType()) == True:
			if player.isBarbarian() == False:
				activePlayer = gc.getPlayer(gc.getGame().getActivePlayer())

				sPlayerName = player.getName()
				sUnitName = gc.getUnitInfo(unit.getUnitType()).getDescription()

				if gc.getTeam(unit.getTeam()).isHasMet(activePlayer.getTeam()):
					sPopupText = CyTranslator().getText('TXT_KEY_MISC_SOMEONE_CREATED_UNITX',(sPlayerName, sUnitName))
				else:
					sPopupText = CyTranslator().getText('TXT_KEY_MISC_UNKNOWN_CREATED_UNITX',(sUnitName, ))
				self.addPopup(sPopupText, str(gc.getUnitInfo(unit.getUnitType()).getImage()))

	def onImprovementBuilt(self, argsList):
		'Improvement Built'
		iImprovement, iX, iY, bWorker = argsList
		pPlot = CyMap().plot(iX, iY)
		ePlayer = pPlot.getOwner()
		eTeam = pPlot.getTeam()

		if bWorker == false or ePlayer == -1 or iImprovement == -1:
			return
		pPlayer = gc.getPlayer(ePlayer)

#		if pPlot.getOwner() != -1:
#			gc.getPlayer(pPlot.getOwner()).changeGold(100)

		if CyGame().getSorenRandNum(2, "Improvement Event") == 0:
			#Location Specific Events
			pPlayer.changeGold(0)
		else:
			#General Events
			iRand = CyGame().getSorenRandNum(100, "Improvement Event")
			if iRand < 3:
				iGold = 340
				pPlayer.changeGold(iGold)
				message = CyTranslator().getText('TXT_KEY_MISC_IMPROVEMENT_EVENTS_ARTIFACT',(iGold, ))
				CyInterface().addMessage(ePlayer, True, 25, message,'',1, gc.getImprovementInfo(iImprovement).getButton(),ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
				return
			if iRand < 6:
				message = CyTranslator().getText('TXT_KEY_MISC_IMPROVEMENT_EVENTS_OLD_MAP',())
				CyInterface().addMessage(ePlayer, True, 25, message,'',1, gc.getImprovementInfo(iImprovement).getButton(),ColorTypes(8),pPlot.getX(),pPlot.getY(),True,True)
				iPlot = CyGame().getSorenRandNum(CyMap().numPlots(), "Improvement Event")
				pPlot = CyMap().plotByIndex(iPlot)
				iRange = 7
				iX = pPlot.getX()
				iY = pPlot.getY()

				for iiX in range(iX - 3, iX + 4, 1):
					for iiY in range(iY - 3, iY + 4, 1):
						pLoopPlot = CyMap().plot(iiX, iiY)
						if not pLoopPlot.isNone():
							if CyGame().getSorenRandNum(2, "Improvement Event") == 0:
								pLoopPlot.setRevealed(eTeam, true, true, -1)
				return

	def techneeded(self, i, j):
		for m in range(4):
			if gc.getTechInfo(i).getPrereqOrTechs(m) == j:
				return true
			if gc.getTechInfo(i).getPrereqAndTechs(m) == j:
				return true

		return false

	def CreateTerrainOnGameStart(self):
		iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
#		iFertile = gc.getInfoTypeForString('TERRAIN_GRASS_FERTILE')
		iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
		iMarsh = gc.getInfoTypeForString('TERRAIN_MARSH')
		iSwamp = gc.getInfoTypeForString('TERRAIN_SWAMP')
		iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')

		#fertile Grasslands on Food Bonuses
#		for i in range (CyMap().numPlots()):
#			pPlot = CyMap().plotByIndex(i)
#			if not pPlot.isWater() and (not pPlot.isHills()) and (not pPlot.isPeak()):
#				if pPlot.getTerrainType() == iGrass:
#					if pPlot.getBonusType(-1) != -1:
#						if gc.getBonusInfo(pPlot.getBonusType(-1)).getCityYieldModifierType() == YieldTypes.YIELD_FOOD:
#							pPlot.setTerrainType(iFertile, True, True)

		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
#			Adjplot = pPlot
			iTerrain = pPlot.getTerrainType()
			if pPlot.getImprovementType() == -1:
				if not pPlot.isWater() and (not pPlot.isHills()) and (not pPlot.isPeak()):
					if pPlot.getFeatureType() == -1 and pPlot.getBonusType(-1) == -1:
						#create Marshlands
						if iTerrain == iGrass:
							iChance = 20
							bValid = true
							for eDirection in range(DirectionTypes.NUM_DIRECTION_TYPES):
								Adjplot = plotDirection(pPlot.getX(), pPlot.getY(), DirectionTypes(eDirection))
								if Adjplot.getTerrainType() == iTundra:
									bValid = false
								if Adjplot.getTerrainType() == iMarsh:
									iChance = 30
							if bValid:
								if CyGame().getSorenRandNum(100, "Bob") < iChance:
									pPlot.setTerrainType(iMarsh, True, True)
						#create Fertile Grasslands
#						if iTerrain == iGrass:
#							iChance = 10
#							bValid = true
#							for eDirection in range(DirectionTypes.NUM_DIRECTION_TYPES):
#								Adjplot = plotDirection(pPlot.getX(), pPlot.getY(), DirectionTypes(eDirection))
#								if not Adjplot.getTerrainType() == iGrass:
#									if not Adjplot.getTerrainType() == iFertile:
#										bValid = false
#								if Adjplot.getTerrainType() == iFertile:
#									iChance = 30

#							if bValid:
#								if CyGame().getSorenRandNum(100, "Bob") < iChance:
#									pPlot.setTerrainType(iFertile, True, True)

					#create Plains
					if iTerrain == iGrass:
						iChance = 5
						bValid = true
						for eDirection in range(DirectionTypes.NUM_DIRECTION_TYPES):
							Adjplot = plotDirection(pPlot.getX(), pPlot.getY(), DirectionTypes(eDirection))
							if Adjplot.getTerrainType() == iPlains:
								iChance = 30

						if bValid:
							if CyGame().getSorenRandNum(100, "Bob") < iChance:
								pPlot.setTerrainType(iPlains, True, True)


			if	pPlot.isHills() and iTerrain == iSwamp:
				pPlot.setTerrainType(iGrass, True, True)

	def CreateFeatureOnGameStart(self):
		iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
		iOasis = gc.getInfoTypeForString('FEATURE_OASIS')
		iMarsh = gc.getInfoTypeForString('TERRAIN_MARSH')
		iSwamp = gc.getInfoTypeForString('TERRAIN_SWAMP')
		iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')

		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			iTerrain = pPlot.getTerrainType()
			if pPlot.getFeatureType() == -1:

				#Place Jungles on Swamps
				if pPlot.getTerrainType() == iSwamp:
					pPlot.setFeatureType(iJungle, 2)
				if pPlot.getTerrainType() == iMarsh:
					if CyGame().getSorenRandNum(100, "Bob") < 70:
						pPlot.setFeatureType(iJungle, 2)

				#Place Oasis on Deserts
				if pPlot.getTerrainType() == iDesert:
					if (pPlot.isHills() == false) and (pPlot.isPeak == false):
						if pPlot.getBonusType(-1) == -1:
							iChance = 20
							bValid = true
							for eDirection in range(DirectionTypes.NUM_DIRECTION_TYPES):
								Adjplot = plotDirection(pPlot.getX(), pPlot.getY(), DirectionTypes(eDirection))
								if Adjplot.getFeatureType() == iOasis:
									bValid = false
								if Adjplot.getTerrainType() == iDesert:
									iChance += 20
							if bValid:
								if CyGame().getSorenRandNum(100, "Bob") < iChance:
									pPlot.setFeatureType(iOasis, 2)
