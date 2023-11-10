import CvUtil
from CvPythonExtensions import *

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

import PyHelpers
PyPlayer = PyHelpers.PyPlayer

#import CvUnblemishedEvents
#import unblemishedeventmod

gc = CyGlobalContext()

def doTraitUnblemished(ePlayer,eTrait):

	pPlayer = gc.getPlayer(ePlayer)
	eTeam = gc.getTeam(pPlayer.getTeam())
	py = PyPlayer(ePlayer)

	iAccord = pPlayer.getPurityCounter()

# War weariness mod
#	wwmod = pPlayer.getWarWearinessPercentAnger()
#	iAccord -= wwmod / 20

# Research mod
	sciPerc = pPlayer.getCommercePercent( CommerceTypes.COMMERCE_RESEARCH )
#	sciPercMod = (100 - sciPerc) / 10
#	iAccord -= int(sciPercMod * 0.5)
	if sciPerc <= 70:
		iAccord -= 1
	if sciPerc <= 40:
		iAccord -= 1

# Per city mod, Revolt mod	- unchecked
	iPlayer = CyGame().getActivePlayer()
#	iAccord += 1

	for pyCity in PyPlayer(iPlayer).getCityList():
		pCity = pyCity.GetCy()
#		iAccord -= 1
		if pPlayer.getAnarchyTurns() < 1:
			if pCity.getOccupationTimer()>1:
				if pCity.getCulture(pCity.getOwner()) > 0:
					iRevoltTurns = 4
					if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
						iRevoltTurns = 3
					if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
						iRevoltTurns = 6
					if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
						iRevoltTurns = 12
					if pCity.getOccupationTimer()<=iRevoltTurns:
						iPop = pCity.getPopulation()
						if CyGame().getSorenRandNum(20, "vim rioting event") < iPop:
							iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_UNBLEMISHED_RIOTING')
							triggerData = pPlayer.initTriggeredData(iEvent, true, -1, pCity.getX(), pCity.getY(), pCity.getOwner(), pCity.getID(), -1, -1, -1, -1)

#			iAccord += 0
#			if CyGame().getSorenRandNum(100, "accord spawning") < iAccord:
#				iGold = pPlayer.getGold()
#				iGoldMod = iGold / 10
#				if CyGame().getSorenRandNum(iGoldMod, "accord spawning") > 10:
#					pPlayer.changeGold(-50)
#					pCity.changeOccupationTimer(-1)
#					iAccord += 5
#					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_GOLD", ()),'',1,'Modules/NormalModules/Unblemished/exemplar.dds',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)

# Accord spawning - part of per city mod function
			if pCity.getOccupationTimer()==0:
				iPop = pCity.getPopulation()
				iGold = pPlayer.getGold()
				iAccordFactor = iAccord * 1.5
				iCompFactor = (iPop * 8) + (iGold / 50) + (iAccordFactor - 100)
				iCompFactor = iCompFactor * .1
#			iCompFactor = ((iPop - 1) * 50000) + (iGold / 25) + (iAccord - 100)
#				iCompFactor = 1000000
				if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNBLEMISHED_MOOT')) == 0:
					if CyGame().getSorenRandNum(400, "complacency") < iCompFactor:
						iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_UNBLEMISHED_COMPLACENCY')
						triggerData = pPlayer.initTriggeredData(iEvent, true, -1, pCity.getX(), pCity.getY(), pCity.getOwner(), pCity.getID(), -1, -1, -1, -1)
				if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNBLEMISHED_MOOT')) > 0:
					if CyGame().getSorenRandNum(1200, "complacency with moot") < iCompFactor:
						iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_UNBLEMISHED_MOOT')
						triggerData = pPlayer.initTriggeredData(iEvent, true, -1, pCity.getX(), pCity.getY(), pCity.getOwner(), pCity.getID(), -1, -1, -1, -1)

# Armageddon mod
	ArmMod = CyGame().getGlobalCounter()
	if ArmMod > 40:
		iAccord += 1
	if ArmMod > 80:
		iAccord += 1

# Total Happiness mod
#	if pPlayer.getNumCities() > 0:
#		iHappylevel = 0
#		for pyCity in PyPlayer(ePlayer).getCityList():
#			pCity = pyCity.GetCy()
#			iHappylevel += (pCity.happyLevel() - pCity.unhappyLevel(0))
#		iTotalCities = pPlayer.getNumCities()
#		iHappyMod = iHappylevel / iTotalCities
#		iAccord += iHappyMod

#Civics
#/Civics

# Rank mod
#	fTurnRatio = float(gc.getGame().getGameTurn()) / float(gc.getGame().getEstimateEndTurn())
	iTurnRatio = gc.getGame().getGameTurn() / gc.getGame().getEstimateEndTurn()
	if iTurnRatio > 0.3:
		iPlayerRank = CyGame().getPlayerRank(ePlayer)
		iNumPlayers = CyGame().countCivPlayersAlive()
		iTopRanks = iNumPlayers / 3
		if iTopRanks > iPlayerRank:
#	iRankMod = iPlayerRank - iNumPlayers + 1
			iAccord -= int(iTopRanks - iPlayerRank)
			CyInterface().addImmediateMessage( "High rank: Vim lost.","" )

#No less than 0
#	if iAccord < 0:
#		iAccord = 0

	pCity = pPlayer.getCapitalCity()

#	if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_DRAGON_FIRE"))>0:
#		iBonus = gc.getInfoTypeForString('BONUS_MANA_CREATION')
#		pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'),iBonus,1)
#	if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_DRAGON_FIRE"))<1:
#		iBonus = gc.getInfoTypeForString('BONUS_MANA_CREATION')
#		pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'),iBonus,0)
	if iAccord > 100:
		iTeam = pPlayer.getTeam()
		eTeam = gc.getTeam(iTeam)
		iExcessVim = iAccord - 100
		iExcessVimFactor = (iExcessVim * iExcessVim) / 100
#		if iExcessVimFactor < 51:
#			iExcessVimFactor = 50

		if CyGame().getSorenRandNum(1800, "high accord spawning") < iExcessVimFactor:

#		iExcessVimFactor = iExcessVim * 1.25
#		if CyGame().getSorenRandNum(10000, "high accord spawning") < iExcessVimFactor:

#			iTotalLand = int(CyMap().getLandPlots())
		#			message = "%0.2f" %(iTotalLand)
		#			CyInterface().addImmediateMessage(message,"")



			if eTeam.getAtWarCount(True) == 0:
				pPlot = pCity.plot()
				iWhichOne = CyGame().getSorenRandNum(3, "which one peace")
				if iWhichOne > 1:
					iAccord -= int(iAccord * 0.7)
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_COMMANDER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_COMMANDER", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas2.dds,3,3',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
					cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_COMMANDER",()), ', ,Art/Interface/Buttons/Units/Units_Atlas2.dds,3,3')

				if iWhichOne == 1:
					iAccord -= int(iAccord * 0.7)
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_MERCHANT'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_MERCHANT", ()),'',1,',Art/Interface/Buttons/Units/GreatTycoon.dds,Art/Interface/Buttons/Unit_Resource_Atlas.dds,3,4',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
					cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_MERCHANT",()), ',Art/Interface/Buttons/Units/GreatTycoon.dds,Art/Interface/Buttons/Unit_Resource_Atlas.dds,3,4')

				if iWhichOne == 0:
					iAccord -= int(iAccord * 0.7)
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_PROPHET'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_PROPHET_PEACE", ()),'',1,', ,Art/Interface/Buttons/Misc_Atlas1.dds,6,8',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
					cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_PROPHET_PEACE",()), ', ,Art/Interface/Buttons/Misc_Atlas1.dds,6,8')

			if eTeam.getAtWarCount(True) >= 1:
				iWhichOne = CyGame().getSorenRandNum(100, "which one war")
#				iWhichOne = 75
				if iWhichOne >= 66:
					pPlot = pCity.plot()
					iAccord -= int(iAccord * 0.7)
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ARTIST'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_ARTIST", ()),'',1,',Art/Interface/Buttons/Units/GreatArtist.dds,Art/Interface/Buttons/Unit_Resource_Atlas.dds,2,4',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
					cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_ARTIST",()), ',Art/Interface/Buttons/Units/GreatArtist.dds,Art/Interface/Buttons/Unit_Resource_Atlas.dds,2,4')
				if iWhichOne <= 65 and iWhichOne >= 35:
					pPlot = pCity.plot()
					iAccord -= int(iAccord * 0.7)
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_PROPHET'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_PROPHET", ()),'',1,', ,Art/Interface/Buttons/Misc_Atlas1.dds,6,8',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
					cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_PROPHET",()), ', ,Art/Interface/Buttons/Misc_Atlas1.dds,6,8')
				if iWhichOne <= 34:
					pPlot = pCity.plot()
					iAccord -= int(iAccord * 0.7)
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ENGINEER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
#					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_ENGINEER", ()),'',1,',Art/Interface/Buttons/Units/GreatEngineer.dds',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
					cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_ENGINEER",()), ',Art/Interface/Buttons/Units/GreatEngineer.dds')

	pPlayer.setPurityCounter(iAccord)

	if iAccord < 0:
		iAnarchyFactor = 100 + iAccord
		if CyGame().getSorenRandNum(100, "high accord spawning") > iAnarchyFactor:
			iAnarchyTurns = 7
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
				iAnarchyTurns = 5
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
				iAnarchyTurns = 9
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
				iAnarchyTurns = 12
			pPlayer.changeAnarchyTurns(iAnarchyTurns)
			cf.addPopup(CyTranslator().getText("TXT_KEY_ACCORD_ANARCHY",()), ',Art/Interface/Buttons/Civics/Liberty.dds')
			pPlayer.setPurityCounter(90)

	if CyGame().getGameTurn() == CyGame().getStartTurn():
		pPlayer.setPurityCounter(100)

	iRevoltTurns = 4
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
		iRevoltTurns = 3
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
		iRevoltTurns = 6
	if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
		iRevoltTurns = 12
	iPlayer = CyGame().getActivePlayer()
	iArmRevoltMod = (CyGame().getGlobalCounter() * .1)
#	if iArmRevoltMod < 0:
#		iArmRevoltMod = 0
	for pyCity in PyPlayer(iPlayer).getCityList():
		pCity = pyCity.GetCy()
		if pCity.getOccupationTimer()==0:
#			iAnger = pCity.getHurryAngerTimer()
#			iRevoltChance += (iAnger / 5)

			iRevoltChance = ((100 - iAccord) * 0.2)
			iUH = 0
			iUH = pCity.badHealth(False) - pCity.goodHealth()
			if iUH > 2:
				iRevoltChance += 1
			if iUH > 4:
				iRevoltChance += 1
			if iUH > 6:
				iRevoltChance += 1
			if iUH > 8:
				iRevoltChance += 2
			iUnhappy = (pCity.happyLevel() - pCity.unhappyLevel(0))
			if iUnhappy <0:
				iRevoltChance -= iUnhappy
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PILLAR_OF_COURAGE')) > 0:
				iArmRevoltMod *= 0.5
			iRevoltChance += iArmRevoltMod
			iwwmod = pPlayer.getWarWearinessPercentAnger()
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_BUILDING_DUNGEON')) > 0:
				iwwmod *= 0.5
			iRevoltChance += iwwmod / 20
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_UNBLEMISHED_MOOT')) > 0:
				iRevoltChance *= 0.8
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_HOPE')) > 0:
				iRevoltChance *= 0.8
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_COURTHOUSE')) > 0:
				iRevoltChance *= 0.9
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALACE_UNBLEMISHED')) > 0:
				iRevoltChance -= 10
#				message = "%0.2f" %(iRevoltChance)
#				CyInterface().addImmediateMessage(message,"")
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_WINTER_PALACE')) > 0:
				iRevoltChance *= 1.20
			if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_FORBIDDEN_PALACE')) > 0:
				iRevoltChance *= 1.20

#			iRevoltBaseCity = CvUnblemishedEvents.UnblemishedFunctions().getRevoltBaseCity()


			if iRevoltChance < 1:
				iRevoltChance = 0
			if CyGame().getGameTurn() != CyGame().getStartTurn():
				if CyGame().getSorenRandNum(100, "Accord city revolt") < iRevoltChance:
					pCity.changeOccupationTimer(iRevoltTurns)
					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ACCORD_CITY_RIOT", ()),'',1,', ,Art/Interface/Buttons/Spells/Spells_Atlas1.dds,4,3',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)


#   check for preserve_weapons promo
	iPreserveWeapons = gc.getInfoTypeForString('PROMOTION_PRESERVE_WEAPONS')
	iUnblemished = gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED')
	iGlassWeapons = gc.getInfoTypeForString('PROMOTION_GLASS_WEAPONS')
	iCrystalWeapons = gc.getInfoTypeForString('PROMOTION_CRYSTAL_WEAPONS')
	iPreservedGlassWeapons = gc.getInfoTypeForString('PROMOTION_PRESERVED_GLASS_WEAPONS')
	iPreservedCrystalWeapons = gc.getInfoTypeForString('PROMOTION_PRESERVED_CRYSTAL_WEAPONS')
	iDrake = gc.getInfoTypeForString('UNIT_DRAKE')
	iGreatDrake = gc.getInfoTypeForString('UNIT_GREAT_DRAKE')
	iCrystalDragon = gc.getInfoTypeForString('UNIT_CRYSTAL_DRAGON')
	iDragonBrother = gc.getInfoTypeForString('UNIT_DRAGON_BROTHER')
	iBeast = gc.getInfoTypeForString('UNITCOMBAT_BEAST')
	iAnimal = gc.getInfoTypeForString('UNITCOMBAT_ANIMAL')
	iSummon = gc.getInfoTypeForString('PROMOTION_SUMMON')
	iMartyr = gc.getInfoTypeForString('PROMOTION_MARTYR')
	iMark = gc.getInfoTypeForString('PROMOTION_MARK_OF_REDEMPTION')
	iInitiate = gc.getInfoTypeForString('PROMOTION_INITIATE_OF_NAN')
	iDedicated = gc.getInfoTypeForString('PROMOTION_DEDICATED_TO_SUCELLUS')

	#remove once BC issue fixed.
#	iBattleCaptain = gc.getInfoTypeForString('UNIT_BATTLECAPTAIN')
#	iParagon = gc.getInfoTypeForString('UNIT_PARAGON')
#	iParagonPromo = gc.getInfoTypeForString('PROMOTION_PARAGON')
#	iBCPromo = gc.getInfoTypeForString('PROMOTION_BATTLECAPTAIN')
	#Also remove artstlyle, add civinfos UU reference
	iInitCount = 0
	for pUnit in py.getUnitList():
		#start remove once BC issue fixed
#		if pUnit.getUnitType() == iBattleCaptain:
#			if not pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_COMBAT1')):
#				pUnit.setName("Paragon")
#				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_COMBAT1'), True)
#				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EMPOWERED_COMBAT_AURA1'), True)
		#end remove once BC issue fixed


		if pUnit.isHasPromotion(iInitiate):
			iInitCount += 1

		if pUnit.isHasPromotion(iDedicated):
			iAccord = pPlayer.getPurityCounter()
			if iAccord < 100:
				iAccord += 3
				pPlayer.setPurityCounter(iAccord)
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEDICATED_TO_SUCELLUS'), False)
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEDICATED_TO_SUCELLUS2'), True)
				CyInterface().addImmediateMessage( "Dedicated unit contribues 3 Vim.","" )


		if pUnit.isHasPromotion(iMartyr):
			iLevel = pUnit.getLevel()
			iAccord = pPlayer.getPurityCounter()
			iAccord += (iLevel * 5)
			pPlayer.setPurityCounter(iAccord)
			pUnit.kill(true,-1)

		if pUnit.isHasPromotion(iMark):
			iLevel = pUnit.getLevel()
			iAccord = pPlayer.getPurityCounter()
			iAccord -= 1
			pPlayer.setPurityCounter(iAccord)

		if pUnit.getUnitType() == iGreatDrake or pUnit.getUnitType() == iDrake or pUnit.getUnitType() == iDragonBrother or pUnit.getUnitType() == iCrystalDragon:
			if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_DRAGON_HEART"))<1:
				if pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG')):
					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), False)
			if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_DRAGON_HEART"))>0:
				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), True)


		if pUnit.getUnitCombatType() != iAnimal:
			if pUnit.getUnitCombatType() != iBeast:
				if not pUnit.isHasPromotion(iSummon):

					if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_TEARS_OF_NANTOSUELTA1"))<1:
						if pUnit.isHasPromotion(iPreservedGlassWeapons):
							pUnit.setHasPromotion(iPreservedGlassWeapons,false)

					if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_TEARS_OF_NANTOSUELTA2"))<1:
						if pUnit.isHasPromotion(iPreservedCrystalWeapons):
							pUnit.setHasPromotion(iPreservedCrystalWeapons,false)

					if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_TEARS_OF_NANTOSUELTA1"))>0:
						pPlot = pUnit.plot()
						if ( pPlot.getOwner()==pUnit.getOwner()
							or pUnit.isHasPromotion(iPreserveWeapons) ):
								pUnit.setHasPromotion(iPreservedGlassWeapons,true)
						else:
							pUnit.setHasPromotion(iPreservedGlassWeapons,false)

					if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_TEARS_OF_NANTOSUELTA2"))>0:
						if pUnit.isHasPromotion(iPreservedGlassWeapons):
							pUnit.setHasPromotion(iPreservedGlassWeapons,false)

						pPlot = pUnit.plot()
						if ( pPlot.getOwner()==pUnit.getOwner()
							or pUnit.isHasPromotion(iPreserveWeapons) ):
								pUnit.setHasPromotion(iPreservedCrystalWeapons,true)
						else:
							pUnit.setHasPromotion(iPreservedCrystalWeapons,false)

#		if pUnit.isHasPromotion(iNewDragon):
		if pUnit.getUnitType() == iGreatDrake or pUnit.getUnitType() == iDrake or pUnit.getUnitType() == iDragonBrother or pUnit.getUnitType() == iCrystalDragon:
			if(CyGame().getGameTurn() - pUnit.getGameTurnCreated()) == 1:
				iDestructionMod = 40
				if pUnit.getUnitType() == iGreatDrake:
					iDestructionMod = 66
				if pUnit.getUnitType() == iCrystalDragon:
					iDestructionMod = 100
				if CyGame().getSorenRandNum(100, "Summon dragon destruction") <= iDestructionMod:
					listCities = []
					for pyCity in PyPlayer(ePlayer).getCityList():
						pCity = pyCity.GetCy()
						listCities.append(pCity)
						iHit = CyGame().getSorenRandNum(len(listCities), "Who gets Hit")
					pCity = listCities[iHit]
					iPop = pCity.getPopulation()
					if pUnit.getUnitType() == iDrake:
						iPop = int(iPop - 2)
					if pUnit.getUnitType() == iGreatDrake:
						iPop = int(iPop - 3)
					if pUnit.getUnitType() == iCrystalDragon:
						iPop = int(iPop / 3)
					if iPop <= 0:
						iPop = 1
					pCity.setPopulation(iPop)
					CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_BAD_DRAGON", ()),'',1,'Art/Interface/Buttons/Apocalypse.dds',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)

					iCount = 0
					iDemon = gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS')
					for iBuilding in range(gc.getNumBuildingInfos()):
						if iBuilding != iDemon:
							if (pCity.getNumRealBuilding(iBuilding) > 0 and gc.getBuildingInfo(iBuilding).getProductionCost() > 0 and not isLimitedWonderClass(gc.getBuildingInfo(iBuilding).getBuildingClassType())):
								if gc.getBuildingInfo(iBuilding).getConquestProbability() != 100:
#									if iDestructionMod > 90:
#										iDestructionMod = 75
									if CyGame().getSorenRandNum(100, "City Fire") <= (iDestructionMod / 2):
										pCity.setNumRealBuilding(iBuilding, 0)
										CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE",(gc.getBuildingInfo(iBuilding).getDescription(), )),'',1,gc.getBuildingInfo(iBuilding).getButton(),ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
										iCount += 1
					if iCount == 0:
						CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE_NO_DAMAGE",()),'AS2D_SPELL_FIRE_ELEMENTAL',1,'Art/Interface/Buttons/Fire.dds',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)

#	if iInitCount >= 8:
#		pCity = pPlayer.getCapitalCity()
#		iBonus = gc.getInfoTypeForString('BONUS_MANA_ENCHANTMENT')
#		if pPlayer.getCapitalCity().getBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), gc.getInfoTypeForString('BONUS_MANA_ENCHANTMENT'))<1:
#			pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'),iBonus,1)
#			CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_PLUS_ENCHANTMENT",()),'AS2D_SPELL_FIRE_ELEMENTAL',1,', ,Art/Interface/Buttons/Misc_Atlas1.dds,4,2',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
#			cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_PLUS_ENCHANTMENT",()), ', ,Art/Interface/Buttons/Misc_Atlas1.dds,4,2')
#	if iInitCount < 8:
#		pCity = pPlayer.getCapitalCity()
#		iBonus = gc.getInfoTypeForString('BONUS_MANA_ENCHANTMENT')
#		if pPlayer.getCapitalCity().getBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'), gc.getInfoTypeForString('BONUS_MANA_ENCHANTMENT'))>=1:
#			pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'),iBonus,0)
#			CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MINUS_ENCHANTMENT",()),'AS2D_SPELL_FIRE_ELEMENTAL',1,', ,Art/Interface/Buttons/Misc_Atlas1.dds,4,2',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
#			cf.addPopup(CyTranslator().getText("TXT_KEY_MESSAGE_MINUS_ENCHANTMENT",()), ', ,Art/Interface/Buttons/Misc_Atlas1.dds,4,2')



#This is what results from fuzzy thinking
#			if not pUnit.isHasPromotion(iPreserveWeapons):
#				pUnit.setHasPromotion(iPreservedGlassWeapons,false)
#				pUnit.setHasPromotion(iPreservedCrystalWeapons,false)
#			if gc.getPlayer(pPlot.getOwner()).getCivilizationType() != iUnblemished:
#				if pUnit.isHasPromotion(iGlassWeapons):
#					pUnit.setHasPromotion(iGlassWeapons,false)
#				if not pPlot.isOwned():
#					if pUnit.isHasPromotion(iGlassWeapons):
#						pUnit.setHasPromotion(iPreservedGlassWeapons,true)
#					if pUnit.isHasPromotion(iCrystalWeapons):
#						pUnit.setHasPromotion(iPreservedCrystalWeapons,true)
#			if pPlot.isOwned():
#				if gc.getPlayer(pPlot.getOwner()).getCivilizationType() != iUnblemished:
#					if pUnit.isHasPromotion(iGlassWeapons):
#						pUnit.setHasPromotion(iPreservedGlassWeapons,true)
#					if pUnit.isHasPromotion(iCrystalWeapons):
#						pUnit.setHasPromotion(iPreservedCrystalWeapons,true)
#				if gc.getPlayer(pPlot.getOwner()).getCivilizationType() == iUnblemished:
#					pUnit.setHasPromotion(iPreservedGlassWeapons,false)
#					pUnit.setHasPromotion(iPreservedCrystalWeapons,false)
#					pUnit.setHasPromotion(iGlassWeapons,true)

#	if gc.getTeam(ePlayer).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_TEARS_OF_NANTOSUELTA2"))>0:
#		for pUnit in py.getUnitList():
#			pPlot = pUnit.plot()
#			if not pUnit.isHasPromotion(iPreserveWeapons):
#				pUnit.setHasPromotion(iPreservedGlassWeapons,false)
#				pUnit.setHasPromotion(iPreservedCrystalWeapons,false)
#			if not gc.getPlayer(pPlot.getOwner()).getCivilizationType() == iUnblemished:
#				if pUnit.isHasPromotion(iCrystalWeapons):
#					pUnit.setHasPromotion(iCrystalWeapons,false)
#			if pUnit.isHasPromotion(iPreserveWeapons):
#				if not pPlot.isOwned():
#					if pUnit.isHasPromotion(iGlassWeapons):
#					if pUnit.isHasPromotion(iCrystalWeapons):
#						pUnit.setHasPromotion(iPreservedCrystalWeapons,true)
#			if pPlot.isOwned():
#				if gc.getPlayer(pPlot.getOwner()).getCivilizationType() != iUnblemished:
#					if pUnit.isHasPromotion(iGlassWeapons):
#						pUnit.setHasPromotion(iPreservedGlassWeapons,true)
#					if pUnit.isHasPromotion(iCrystalWeapons):
#						pUnit.setHasPromotion(iPreservedCrystalWeapons,true)
#				if gc.getPlayer(pPlot.getOwner()).getCivilizationType() == iUnblemished:
#					pUnit.setHasPromotion(iPreservedGlassWeapons,false)
#					pUnit.setHasPromotion(iPreservedCrystalWeapons,false)
#					pUnit.setHasPromotion(iGlassWeapons,false)
#					pUnit.setHasPromotion(iCrystalWeapons,true)


#	Drakes have a 100% expire promo - check for it.  Random chance to destroy buildings, improvements, or reduce pop.
#	for pUnit in py.getUnitList():
#				if pUnit.getUnitType() == iCrystalDragon:
#					for pyCity in PyPlayer(ePlayer).getCityList():
#						for pyCity in PyPlayer(ePlayer).getCityList():
#							pCity = pyCity.GetCy()
#							listCities.append(pCity)
#							iHit = CyGame().getSorenRandNum(len(listCities), "Who gets Hit")
#						pCity = listCities[iHit]
#						iPop = pCity.getPopulation()
#						iPop = int(iPop / 3)
#						if iPop == 0:
#							iPop = 1
#						pCity.setPopulation(iPop)
#						CyInterface().addMessage(ePlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_BAD_DRAGON", ()),'',1,'Art/Interface/Buttons/Apocalypse.dds',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
#
#						iCount = 0
#						iDemon = gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS')
#						for iBuilding in range(gc.getNumBuildingInfos()):
#							if iBuilding != iDemon:
#								if pCity.getNumRealBuilding(iBuilding) > 0:
#									if gc.getBuildingInfo(iBuilding).getConquestProbability() != 100:
#										if CyGame().getSorenRandNum(100, "City Fire") <= (iDestructionMod / 2):
#											pCity.setNumRealBuilding(iBuilding, 0)
#											CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE",(gc.getBuildingInfo(iBuilding).getDescription(), )),'',1,gc.getBuildingInfo(iBuilding).getButton(),ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
#											iCount += 1
#						if iCount == 0:
#							CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_CITY_FIRE_NO_DAMAGE",()),'AS2D_SPELL_FIRE_ELEMENTAL',1,'Art/Interface/Buttons/Fire.dds',ColorTypes(8),pCity.getX(),pCity.getY(),True,True)


def spellPrimevalFury(pCaster):

#	pPlayer = gc.getPlayer(ePlayer)
	pPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
	pPlot = pCaster.plot()

	iSpawnMod = (pCaster.getLevel() * 5)


	newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LION'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)

	if CyGame().getSorenRandNum(100, "More animals check") < (50 + iSpawnMod):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LION'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)

	if CyGame().getSorenRandNum(100, "More animals check") < (50 + iSpawnMod):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LION'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)

	if CyGame().getSorenRandNum(100, "More animals check") < (50 + iSpawnMod):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LION_PRIDE'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)

	if CyGame().getSorenRandNum(100, "More animals check") < (25 + iSpawnMod):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_TREANT'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)

	if CyGame().getSorenRandNum(100, "More animals check") < (0 + iSpawnMod):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_TREANT'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)

	if CyGame().getSorenRandNum(100, "More animals check") < (0 + iSpawnMod):
		newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_GREAT_DRAKE'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)

	pCaster.kill(True, 0)

def doTraitFireMastery(ePlayer,eTrait):

	pPlayer = gc.getPlayer(ePlayer)
	eTeam = gc.getTeam(pPlayer.getTeam())

	eTech =gc.getInfoTypeForString('SPELLRESEARCH_FIREBALL')
	eTeam.setHasTech(eTech,true,-1,false,true)
	eTech =gc.getInfoTypeForString('SPELLRESEARCH_ELEMENTAL_ASSAULT1')
	eTeam.setHasTech(eTech,true,-1,false,true)
	eTech =gc.getInfoTypeForString('SPELLRESEARCH_FIRESHIELD')
	eTeam.setHasTech(eTech,true,-1,false,true)

def doTraitMagnificent(ePlayer,eTrait):
	pPlayer = gc.getPlayer(ePlayer)

	if pPlayer.getNumCities() > 0:
		for pyCity in PyPlayer(ePlayer).getCityList():
			iBuilding1 = gc.getInfoTypeForString('BUILDING_MAG1')
			iBuilding2 = gc.getInfoTypeForString('BUILDING_MAG2')
			iBuilding3 = gc.getInfoTypeForString('BUILDING_MAG3')
			iBuilding4 = gc.getInfoTypeForString('BUILDING_MAG4')
			pCity = pyCity.GetCy()
			iCycle = 1
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
				iCycle = .75
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
				iCycle = 1.5
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
				iCycle = 3
			iCulture = pCity.getCulture(ePlayer)/iCycle
			if (iCulture >= 149 and iCulture <= 599):
				iNewBuilding = iBuilding1
			if (iCulture >= 600 and iCulture <= 2199):
				iNewBuilding = iBuilding2
			if (iCulture >= 2200 and iCulture <= 4799):
				iNewBuilding = iBuilding3
			if (iCulture >= 4800):
				iNewBuilding = iBuilding4
			pCity.setNumRealBuilding(iBuilding1, 0)
			pCity.setNumRealBuilding(iBuilding2, 0)
			pCity.setNumRealBuilding(iBuilding3, 0)
			pCity.setNumRealBuilding(iBuilding4, 0)
			if (iCulture>=149):
				pCity.setNumRealBuilding(iNewBuilding, 1)

def reqRitesofSucellus(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	eTeam = gc.getTeam(pPlayer.getTeam())
#	py = PyPlayer(ePlayer)
	if eTeam.isHasTech(gc.getInfoTypeForString('TECH_FERAL_BOND')) == False:
		return False
	if pPlayer.isHuman() == False:
		iCount = 0
		for pUnit in py.getUnitList():
			if pUnit.getLevel() >= 8:
				iCount +=1
		if iCount < 3:
			return False
	return True

def spellRitesofSucellus(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	pCity = pPlayer.getCapitalCity()
	py = PyPlayer(caster.getOwner())
	iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_SUCELLUS')
	newUnit = pPlayer.initUnit(iUnit, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	for pUnit in py.getUnitList():
		if pUnit.getLevel() >= 8:
			pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MARK_OF_REDEMPTION'), True)
			newUnit = pPlayer.initUnit(iUnit, pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
	iAccord = pPlayer.getPurityCounter()
	iAccord += 50
	pPlayer.setPurityCounter(iAccord)

def reqGlory(caster):
#	iPlayer = caster.getOwner()
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(caster.getOwner())

	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GLORY')):
		return False

	iAccord = pPlayer.getPurityCounter()

	if iAccord < 15:
		return False

	iTeam = pPlayer.getTeam()
	eTeam = gc.getTeam(iTeam)
	if eTeam.getAtWarCount(True) == 0:
		return False

#	if pPlayer.isHuman() == False:
#		bValid=false
#		iX = caster.getX()
#		iY = caster.getY()
#		for iiX in range(iX-1, iX+2, 1):
#			for iiY in range(iY-1, iY+2, 1):
#				pPlot = CyMap().plot(iiX,iiY)
#				if pPlot.isVisibleEnemyUnit(iPlayer):
#					bValid=true
#
#	return bValid

	return True

def spellGlory(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	iAccord = pPlayer.getPurityCounter()
	iAccord -= 15
	pPlayer.setPurityCounter(iAccord)

def reqAccordtoFaith(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	iAccord = pPlayer.getPurityCounter()
	iFaith = pPlayer.getFaith()

	if iFaith < 500:
		return False

	if pPlayer.isHuman() == False:
		if iAccord > 99:
			return False
		if iFaith < 1000:
			return False

	return True

def spellAccordtoFaith(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	iAccord = pPlayer.getPurityCounter()
	iAccord += 10
	pPlayer.setPurityCounter(iAccord)
	pPlayer.changeFaith(-500)

def reqMartyr(caster):
	iPlayer = caster.getOwner()
	pPlayer = gc.getPlayer(iPlayer)

	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MARK_OF_REDEMPTION')):
		return False

	if pPlayer.isHuman() == False:
		iAccord = pPlayer.getPurityCounter()
		if iAccord > 60:
			return False

	return True

def reqRedemption(caster):
#	iPlayer = caster.getOwner()
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(caster.getOwner())

	iAccord = pPlayer.getPurityCounter()

	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MARTYR')):
		return False
	if caster.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MARK_OF_REDEMPTION')):
		return False

	if iAccord < 10:
		return False

	if pPlayer.isHuman() == False:
		if iAccord < 125:
			return False
		if caster.getLevel() < 8:
			return False

	return True

def spellRedemption(caster):
	iPlayer = caster.getOwner()
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(caster.getOwner())

	iAccord = pPlayer.getPurityCounter()
	iAccord -= 10
	pPlayer.setPurityCounter(iAccord)

def reqStrengthofPurpose(caster):
	iPlayer = caster.getOwner()
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(caster.getOwner())
	if caster.getDamage() == 0:
		return False
	iAccord = pPlayer.getPurityCounter()
	if iAccord < 10:
		return False
	pPlayer = gc.getPlayer(caster.getOwner())
	if pPlayer.isHuman() == False:
		if caster.getDamage() < 75:
			return False
		if iAccord < 100:
			return False
	return True

def spellStrengthofPurpose(caster):
	iPlayer = caster.getOwner()
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(caster.getOwner())
	caster.setDamage(0, caster.getOwner())
	iAccord = pPlayer.getPurityCounter()
	iAccord -= 10
	pPlayer.setPurityCounter(iAccord)
	caster.finishMoves()
	caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FATIGUED'), True)
	caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_SLOW'), false)
	if CyGame().getSorenRandNum(100, "vim rioting event") < 10:
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DISEASED'), false)
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PLAGUED'), false)
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), false)
		caster.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WITHERED'), false)

def reqCrystalDragon(caster):
#	iPlayer = caster.getOwner()
#	pPlayer = gc.getPlayer(iPlayer)
	pPlayer = gc.getPlayer(caster.getOwner())
	eTeam = gc.getTeam(pPlayer.getTeam())
#	py = PyPlayer(ePlayer)
	py = PyPlayer(caster.getOwner())
	iCrystalDragon = gc.getInfoTypeForString('UNIT_CRYSTAL_DRAGON')
#	pTeam = gc.getTeam(pPlayer.getTeam())

	if gc.getTeam(pPlayer.getTeam()).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_DRAGON_FIRE"))<1:
		return False
	if gc.getTeam(pPlayer.getTeam()).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_DRAGON_HEART"))<1:
		return False
	if gc.getTeam(pPlayer.getTeam()).getProjectCount(gc.getInfoTypeForString("PROJECT_GLOBAL_DRAGON_FORGE"))<1:
		return False

	for pUnit in py.getUnitList():
		#start remove once BC issue fixed
#		if pUnit.getUnitType() == iBattleCaptain:
#			if not pUnit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_COMBAT1')):
#				pUnit.setName("Paragon")
#				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_COMBAT1'), True)
#				pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_EMPOWERED_COMBAT_AURA1'), True)
		#end remove once BC issue fixed

		if pUnit.getUnitType() == iCrystalDragon:
			return False

	return True

def reqDestroyBarbBase(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())

	iAccord = pPlayer.getPurityCounter()
	if iAccord < 10:
		return False

	cityNearby = False
	specialImprovementNearby = False
	barbarianUnitNearby = False

	iX = caster.getX()
	iY = caster.getY()

	for iiX in range(iX-3, iX+4, 1):
		for iiY in range(iY-3, iY+4, 1):
			pLoopPlot = CyMap().plot(iiX,iiY)
			if not pLoopPlot.isNone():
				if pLoopPlot.isCity() and pLoopPlot.getOwner()==caster.getOwner():
					pCity = pLoopPlot.getPlotCity()
					if pCity.getPopulation() > 2:
						cityNearby = True

	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			if pPlot.isNone() == False:
				improvementType = pPlot.getImprovementType()
				if improvementType.getImprovementInfo(improvementType).getSpawnUnitCiv() != CivilizationTypes.NO_CIVILIZATION:
					specialImprovementNearby = True
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				if pUnit.getOwner() == gc.getBARBARIAN_PLAYER():
					barbarianUnitNearby = True

	if cityNearby and specialImprovementNearby and barbarianUnitNearby:
		return True

	return False

## need to check for barb fortress?

def spellDestroyBarbBase(caster):
	iX = caster.getX()
	iY = caster.getY()
	pPlayer = gc.getPlayer(caster.getOwner())

	iAccord = pPlayer.getPurityCounter()
	iAccord -= 10
	pPlayer.setPurityCounter(iAccord)

	for iiX in range(iX-1, iX+2, 1):
		for iiY in range(iY-1, iY+2, 1):
			pPlot = CyMap().plot(iiX,iiY)
			for i in range(pPlot.getNumUnits()):
				pUnit = pPlot.getUnit(i)
				improvementType = pPlot.getImprovementType()
#				if pUnit.getRace() == gc.getInfoTypeForString('PROMOTION_UNDEAD'):
				if pUnit.getOwner() == gc.getBARBARIAN_PLAYER():
					pUnit.doDamage(50, 100, caster, gc.getInfoTypeForString('DAMAGE_PHYSICAL'), true)
			if improvementType.getImprovementInfo(improvementType).getSpawnUnitCiv() != CivilizationTypes.NO_CIVILIZATION:
				pPlot.setImprovementType(-1)


	iX = caster.getX()
	iY = caster.getY()

	for iiX in range(iX-3, iX+4, 1):
		for iiY in range(iY-3, iY+4, 1):
			pLoopPlot = CyMap().plot(iiX,iiY)
			if not pLoopPlot.isNone():
				if pLoopPlot.isCity() and pLoopPlot.getOwner()==caster.getOwner():
					pCity = pLoopPlot.getPlotCity()
					if pCity.getPopulation() > 2:
						pCity.changePopulation(-1)
						CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MILITIA_ASSAULT", ()),'',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,1,4',ColorTypes(7),pCity.getX(),pCity.getY(),True,True)
						return

## Need to destroy barb fortress.
## Need to reduce pop of nearest city by 1-3.
