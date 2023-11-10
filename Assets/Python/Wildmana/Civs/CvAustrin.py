import CvUtil
from CvPythonExtensions import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

gc = CyGlobalContext()

import CustomFunctions
cf = CustomFunctions.CustomFunctions()

#AUSTRIN
def onAustrinCityAcquired(argsList):
	'City Acquired'
	iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
	pPlayer = gc.getPlayer(iNewOwner)

	if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_AUSTRIN'):
		pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_AUSTRIN_SETTLEMENT'), 1)
	else:
		pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_AUSTRIN_SETTLEMENT'), 0)			
		
def onAustrinBuildingBuilt(argsList):
	'Building Completed'
	pCity, iBuildingType = argsList
	player = pCity.getOwner()
	pPlayer = gc.getPlayer(player)
	pPlot = pCity.plot()
	game = gc.getGame()
	iBuildingClass = gc.getBuildingInfo(iBuildingType).getBuildingClassType()
		
		
	if iBuildingType == gc.getInfoTypeForString('BUILDING_JOINING'):
#			CyInterface().addImmediateMessage('The Joining Built!', "AS2D_NEW_ERA")
		iNewPlayer = cf.getOpenPlayer()
		iTeam = pPlayer.getTeam()
		pPlot2 = cf.findClearPlot(-1, pCity.plot())
		if (iNewPlayer != -1 and pPlot2 != -1):
			for i in range(pPlot.getNumUnits(), -1, -1):
				pUnit = pPlot.getUnit(i)
				pUnit.setXY(pPlot2.getX(), pPlot2.getY(), true, true, true)
			#End mercurian gate code.  ENTER THE BARB CIV CODE!	
#				CyInterface().addImmediateMessage('Starting new civ code...', "AS2D_NEW_ERA")
			newLeaderIdx=1
			newCivIdx=1
			iPirate = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_PIRATES')
			iAnimal = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ANIMAL')
			iOrc = CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_BARBARIAN')
			iRandom=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_RANDOM')
			iInfernals=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_INFERNAL')
			iMercurians=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_MERCURIANS')
			iFrozen=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_FROZEN')
			#added by LordShaggynator
			iWhiteHand=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_WHITEHAND')
			iDevils=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_DEVIL')
			iJotnar=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_JOTNAR')
			iAristrakh=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_ARISTRAKH')
			iMercenaries=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_MERCENARIES')
			iWildmana=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_WILDMANA')
			#iUnblemished=CvUtil.findInfoTypeNum(gc.getCivilizationInfo,gc.getNumCivilizationInfos(),'CIVILIZATION_UNBLEMISHED')
			#/LordShaggynator
			unwantedCivsList = [iPirate, iAnimal, iOrc, iRandom, iInfernals, iMercurians, iFrozen, iWhiteHand, iDevils, iJotnar, iAristrakh, iMercenaries, iWildmana]
			availableCivs = list()
			taken = True
			for civType in range(0,gc.getNumCivilizationInfos()) :
				#if( not ((civType == iAnimal) or (civType == iOrc) or (civType == iPirate) or (civType==iRandom) or (civType==iMercurians) or (civType==iInfernals) or (civType==iFrozen) or (civType==iWhiteHand) or (civType==iDevils) or (civType==iJotnar) or (civType==iAristrakh) or (civType==iMercenaries) or (civType==iWildmana))):
				# BlackArchon: This is far more easier to read.
				if civType not in unwantedCivsList:
					taken = False
#						CyInterface().addImmediateMessage('The program gets the loop, yay.', "AS2D_NEW_ERA")
					for i in range(0,gc.getMAX_CIV_PLAYERS()) :
						if( civType == gc.getPlayer(i).getCivilizationType() ) :
							if( gc.getPlayer(i).isAlive() or gc.getPlayer(i).isFoundedFirstCity() or gc.getPlayer(i).getCitiesLost() > 0 ) :
								taken = True
								break
					if( not taken ) :
						availableCivs.append(civType)
			if( len(availableCivs) < 1 ) :
				if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - ERROR: Unexpected lack of unused civ types")
			newCivIdx = availableCivs[game.getSorenRandNum(len(availableCivs),'New civ: pick unused civ type')]
			leaderList = list()
			for leaderType in range(0,gc.getNumLeaderHeadInfos()) :
				if( gc.getCivilizationInfo(newCivIdx).isLeaders(leaderType) ) :
					leaderList.append(leaderType)
			if( len(leaderList) < 1 ) :
				if( self.LOG_DEBUG ) : CvUtil.pyPrint("  BC - ERROR: Unexpected lack of possible leaders")
			newLeaderIdx = leaderList[game.getSorenRandNum(len(leaderList),'New Civ: pick leader')]		
#				CyInterface().addImmediateMessage('End Barb Civ Code', "AS2D_NEW_ERA")
			
			CyGame().addPlayerAdvanced(iNewPlayer, iTeam, newLeaderIdx, newCivIdx)
			gc.getPlayer(iNewPlayer).initUnit(gc.getInfoTypeForString('UNIT_ARCHER'), pPlot.getX(), pPlot.getY(), UnitAITypes.UNITAI_CITY_DEFENSE, DirectionTypes.DIRECTION_NORTH)
			gc.getPlayer(iNewPlayer).initUnit(gc.getInfoTypeForString('UNIT_SORAH'), pPlot.getX(), pPlot.getY(), UnitAITypes.UNITAI_CITY_DEFENSE, DirectionTypes.DIRECTION_NORTH)						
			
			#added by BlackArchon: to get a question if the human player wants to change his civ to the new ally
			if pPlayer.isHuman():
					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON)
					#popupInfo.setText(CyTranslator().getText("TXT_KEY_POPUP_CONTROL_MERCURIANS",()))
					popupInfo.setText("Do you want to change your civilization to your new ally?")
					popupInfo.setData1(player)
					popupInfo.setData2(iNewPlayer)
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_YES", ()), "")
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_NO", ()), "")
					popupInfo.setOnClickedPythonCallback("reassignPlayer")
					popupInfo.addPopup(player)
			#/BlackArchon
						

def onAustrinProjectBuilt(argsList):
	'Project Completed'
	pCity, iProjectType = argsList
	game = gc.getGame()
	iPlayer = pCity.getOwner()
	pPlayer = gc.getPlayer(iPlayer)
	
	if iProjectType == gc.getInfoTypeForString('PROJECT_PREPARE_EXPEDITION'):
		pPlayer.setFeatAccomplished(FeatTypes.FEAT_EXPEDITION_READY, True)


		