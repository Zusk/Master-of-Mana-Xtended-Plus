import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#START ESUS

def onEsusUnitBuilt(argsList):
	'Unit Completed'
	city = argsList[0]
	unit = argsList[1]
	player = PyPlayer(city.getOwner())
	pPlayer = gc.getPlayer(unit.getOwner())

#Esus Mechanics
	iEsus = gc.getInfoTypeForString('RELIGION_COUNCIL_OF_ESUS')

	if pPlayer.isFollowEsus():
		if unit.getUnitCombatType() == gc.getInfoTypeForString('UNITCOMBAT_RECON'):
			if gc.getUnitInfo(unit.getUnitType()).getReligionType()==-1:
				unit.setReligion(iEsus)

	if unit.isAlive() and city.isHasReligion(iEsus):
		if pPlayer.isFollowEsus():
			if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HERO')):
				unit.setReligion(iEsus)
			if unit.getUnitType() == gc.getInfoTypeForString('UNIT_DRUID'):
				unit.setReligion(iEsus)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HIDDEN'), True)
			EsusFunctions.doGoodCrime(EsusFunctions(),city, unit)
		else:
			EsusFunctions.doBadCrime(EsusFunctions(),city, unit)

class EsusFunctions:

	def doBadCrime(self, city, unit):
		num = city.getCrime()/10
		if city.isHasReligion(gc.getInfoTypeForString('RELIGION_THE_EMPYREAN')):
			num = 0
		if not gc.getPlayer(city.getOwner()).hasTrait(gc.getInfoTypeForString('TRAIT_SINISTER')) and not unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_HERO')) and unit.isAlive():
			result = CyGame().getSorenRandNum(5, "Esus")
			if result < num:
				CyInterface().addMessage(unit.getOwner(),True,25,CyTranslator().getText("An assassin has given our unit a crippling poison!", ()),'',1,'Art/Interface/Buttons/Promotions/Weak.dds',ColorTypes(7),unit.getX(),unit.getY(),True,True)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), True)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), True)
				unit.doDamageNoCaster(50, 90, gc.getInfoTypeForString('DAMAGE_POISON'), false)
			elif result < num + 2:
				CyInterface().addMessage(unit.getOwner(),True,25,CyTranslator().getText("An assassin has poisoned our unit!", ()),'',1,'Art/Interface/Buttons/Poison.dds',ColorTypes(7),unit.getX(),unit.getY(),True,True)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_POISONED'), True)
				unit.doDamageNoCaster(25, 90, gc.getInfoTypeForString('DAMAGE_POISON'), false)
			elif result < num + 4 and unit.getUnitCombatType() != gc.getInfoTypeForString('UNITCOMBAT_DISCIPLE'):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_HIDDEN_NATIONALITY'), True)
				unit.setReligion(gc.getInfoTypeForString('RELIGION_COUNCIL_OF_ESUS'))

	def doGoodCrime(self, city, unit):
		crime = city.getCrime()
		pop = city.getPopulation()
		owner = city.getOwner()
		pPlayer = gc.getPlayer(owner)
		iEsus = gc.getInfoTypeForString('RELIGION_COUNCIL_OF_ESUS')
		minorNum = CyGame().getSorenRandNum((crime/10), "Esus")
		maxNum = pop/5
		if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_CULT_OF_ESUS_HALL')) > 0:
			maxNum = maxNum + 1
		if minorNum > 0:
			if minorNum > 3:
				unit.setReligion(iEsus)
			iBlade = gc.getInfoTypeForString('EQUIPMENT_POISONS')
			iFlanking1 = gc.getInfoTypeForString('PROMOTION_FLANKING1')
			iMarksman = gc.getInfoTypeForString('PROMOTION_MARKSMAN')
			iBounty = gc.getInfoTypeForString('PROMOTION_BOUNTY_HUNTER')
			iHidden = gc.getInfoTypeForString('PROMOTION_HIDDEN')
			iNationality = gc.getInfoTypeForString('PROMOTION_HIDDEN_NATIONALITY')
			iMobility = gc.getInfoTypeForString('PROMOTION_MOBILITY1')
			iPoisonResist = gc.getInfoTypeForString('PROMOTION_POISON_RESISTANCE')
			if pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_CULT_OF_ESUS_HALL')) > 0:
				iFlanking2 = gc.getInfoTypeForString('PROMOTION_FLANKING2')
				iSinister = gc.getInfoTypeForString('PROMOTION_SINISTER')
				iFear = gc.getInfoTypeForString('PROMOTION_FEAR')
				iAeron = gc.getInfoTypeForString('PROMOTION_AERONS_CHOSEN')
				result = CyGame().getSorenRandNum(10, "Esus")
				if result==0:
					unit.setHasPromotion(iFlanking1, True)
					unit.setHasPromotion(iFlanking2, True)
				if result==1:
					if unit.isHasPromotion(iSinister):
						unit.setBaseCombatStr(unit.baseCombatStr() + 1)
					else:
						unit.setHasPromotion(iSinister, True)
				if result==2:
					unit.setHasPromotion(iAeron, True)
				if result==3:
					unit.setHasPromotion(iFear, True)
				maxNum = maxNum - 1
			for i in range (minorNum):
				if maxNum <= 0:
					break
				result = CyGame().getSorenRandNum(13, "Esus")
				if result==0:
					unit.equip(iBlade, True)
				elif result==1:
					unit.setHasPromotion(iFlanking1, True)
				elif result==2:
					unit.setHasPromotion(iMarksman, True)
				elif result==3:
					unit.setHasPromotion(iBounty, True)
				elif result==4:
					unit.setHasPromotion(iHidden, True)
				elif result==5:
					unit.setHasPromotion(iNationality, True)
				elif result==6:
					unit.setHasPromotion(iMobility, True)
				elif result==7:
					unit.setHasPromotion(iPoisonResist, True)
				if result < 8:
					maxNum = maxNum - 1


#END ESUS


