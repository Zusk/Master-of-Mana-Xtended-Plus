# Sid Meier's Civilization 4
# Copyright Firaxis Games 2006
#
# CvEventManager
# This class is passed an argsList from CvAppInterface.onEvent
# The argsList can contain anything from mouse location to key info
# The EVENTLIST that are being notified can be found

from CvPythonExtensions import *
import CvUtil
import CvScreensInterface
import CvDebugTools
import CvWBPopups
import PyHelpers
import Popup as PyPopup
import CvCameraControls
#import CvTopCivs
import sys
import CvWorldBuilderScreen
import CvAdvisorUtils
import CvTechChooser

import CvIntroMovieScreen
import CustomFunctions

#FfH: Card Game: begin
import CvSomniumInterface
import CvCorporationScreen
#FfH: Card Game: end

#FfH: Added by Kael 10/15/2008 for OOS Logging
import OOSLogger
#FfH: End Add

# globals
cf = CustomFunctions.CustomFunctions()
gc = CyGlobalContext()
localText = CyTranslator()
PyPlayer = PyHelpers.PyPlayer
PyInfo = PyHelpers.PyInfo

#FfH: Card Game: begin
cs = CvCorporationScreen.cs
#FfH: Card Game: end


# globals
###################################################
class CvEventManager:
	def __init__(self):
		#################### ON EVENT MAP ######################
		#print "EVENTMANAGER INIT"

		self.bCtrl = False
		self.bShift = False
		self.bAlt = False
		self.bAllowCheats = False

		# OnEvent Enums
		self.EventLButtonDown=1
		self.EventLcButtonDblClick=2
		self.EventRButtonDown=3
		self.EventBack=4
		self.EventForward=5
		self.EventKeyDown=6
		self.EventKeyUp=7

		self.__LOG_MOVEMENT = 0
		self.__LOG_BUILDING = 0
		self.__LOG_COMBAT = 0
		self.__LOG_CONTACT = 0
		self.__LOG_IMPROVEMENT =0
		self.__LOG_CITYLOST = 0
		self.__LOG_CITYBUILDING = 0
		self.__LOG_TECH = 0
		self.__LOG_UNITBUILD = 0
		self.__LOG_UNITKILLED = 1
		self.__LOG_UNITLOST = 0
		self.__LOG_UNITPROMOTED = 0
		self.__LOG_UNITSELECTED = 0
		self.__LOG_UNITPILLAGE = 0
		self.__LOG_GOODYRECEIVED = 0
		self.__LOG_GREATPERSON = 0
		self.__LOG_RELIGION = 0
		self.__LOG_RELIGIONSPREAD = 0
		self.__LOG_GOLDENAGE = 0
		self.__LOG_ENDGOLDENAGE = 0
		self.__LOG_WARPEACE = 0
		self.__LOG_PUSH_MISSION = 0

		## EVENTLIST
		self.EventHandlerMap = {
			'mouseEvent'			: self.onMouseEvent,
			'kbdEvent' 				: self.onKbdEvent,
			'ModNetMessage'					: self.onModNetMessage,
			'Init'					: self.onInit,
			'Update'				: self.onUpdate,
			'UnInit'				: self.onUnInit,
			'OnSave'				: self.onSaveGame,
			'OnPreSave'				: self.onPreSave,
			'OnLoad'				: self.onLoadGame,
			'GameStart'				: self.onGameStart,
			'GameEnd'				: self.onGameEnd,
			'plotRevealed' 			: self.onPlotRevealed,
			'plotFeatureRemoved' 	: self.onPlotFeatureRemoved,
			'plotPicked'			: self.onPlotPicked,
			'nukeExplosion'			: self.onNukeExplosion,
			'gotoPlotSet'			: self.onGotoPlotSet,
			'BeginGameTurn'			: self.onBeginGameTurn,
			'EndGameTurn'			: self.onEndGameTurn,
			'BeginPlayerTurn'		: self.onBeginPlayerTurn,
			'EndPlayerTurn'			: self.onEndPlayerTurn,
			'endTurnReady'			: self.onEndTurnReady,
			'combatResult' 			: self.onCombatResult,
		  'combatLogCalc'	 		: self.onCombatLogCalc,
		  'combatLogHit'				: self.onCombatLogHit,
			'improvementBuilt' 		: self.onImprovementBuilt,
			'improvementDestroyed' 		: self.onImprovementDestroyed,
			'routeBuilt' 		: self.onRouteBuilt,
			'firstContact' 			: self.onFirstContact,
			'cityBuilt' 			: self.onCityBuilt,
			'cityRazed'				: self.onCityRazed,
			'cityAcquired' 			: self.onCityAcquired,
			'cityAcquiredAndKept' 	: self.onCityAcquiredAndKept,
			'cityLost'				: self.onCityLost,
			'cultureExpansion' 		: self.onCultureExpansion,
			'cityGrowth' 			: self.onCityGrowth,
			'cityDoTurn' 			: self.onCityDoTurn,
			'cityBuildingUnit'	: self.onCityBuildingUnit,
			'cityBuildingBuilding'	: self.onCityBuildingBuilding,
			'cityRename'				: self.onCityRename,
			'cityHurry'				: self.onCityHurry,
			'selectionGroupPushMission'		: self.onSelectionGroupPushMission,
			'unitMove' 				: self.onUnitMove,
			'unitSetXY' 			: self.onUnitSetXY,
			'unitCreated' 			: self.onUnitCreated,
			'unitBuilt' 			: self.onUnitBuilt,
			'unitKilled'			: self.onUnitKilled,
			'unitLost'				: self.onUnitLost,
			'unitPromoted'			: self.onUnitPromoted,
			'unitSelected'			: self.onUnitSelected,
			'UnitRename'				: self.onUnitRename,
			'unitPillage'				: self.onUnitPillage,
			'unitSpreadReligionAttempt'	: self.onUnitSpreadReligionAttempt,
			'unitGifted'				: self.onUnitGifted,
			'unitBuildImprovement'				: self.onUnitBuildImprovement,
			'goodyReceived'        	: self.onGoodyReceived,
			'greatPersonBorn'      	: self.onGreatPersonBorn,
			'buildingBuilt' 		: self.onBuildingBuilt,
			'projectBuilt' 			: self.onProjectBuilt,
			'techAcquired'			: self.onTechAcquired,
			'techSelected'			: self.onTechSelected,
			'religionFounded'		: self.onReligionFounded,
			'religionSpread'		: self.onReligionSpread,
			'religionRemove'		: self.onReligionRemove,
			'corporationFounded'	: self.onCorporationFounded,
			'corporationSpread'		: self.onCorporationSpread,
			'corporationRemove'		: self.onCorporationRemove,
			'goldenAge'				: self.onGoldenAge,
			'endGoldenAge'			: self.onEndGoldenAge,
			'chat' 					: self.onChat,
			'victory'				: self.onVictory,
			'vassalState'			: self.onVassalState,
			'changeWar'				: self.onChangeWar,
			'setPlayerAlive'		: self.onSetPlayerAlive,
			'playerChangeStateReligion'		: self.onPlayerChangeStateReligion,
			'playerGoldTrade'		: self.onPlayerGoldTrade,
			'windowActivation'		: self.onWindowActivation,
			'gameUpdate'			: self.onGameUpdate,		# sample generic event
		}

		################## Events List ###############################
		#
		# Dictionary of Events, indexed by EventID (also used at popup context id)
		#   entries have name, beginFunction, applyFunction [, randomization weight...]
		#
		# Normal events first, random events after
		#
		################## Events List ###############################
		self.Events={
			CvUtil.EventEditCityName : ('EditCityName', self.__eventEditCityNameApply, self.__eventEditCityNameBegin),
			CvUtil.EventEditCity : ('EditCity', self.__eventEditCityApply, self.__eventEditCityBegin),
			CvUtil.EventPlaceObject : ('PlaceObject', self.__eventPlaceObjectApply, self.__eventPlaceObjectBegin),
			CvUtil.EventAwardTechsAndGold: ('AwardTechsAndGold', self.__eventAwardTechsAndGoldApply, self.__eventAwardTechsAndGoldBegin),
			CvUtil.EventEditUnitName : ('EditUnitName', self.__eventEditUnitNameApply, self.__eventEditUnitNameBegin),
			CvUtil.EventWBAllPlotsPopup : ('WBAllPlotsPopup', self.__eventWBAllPlotsPopupApply, self.__eventWBAllPlotsPopupBegin),
			CvUtil.EventWBLandmarkPopup : ('WBLandmarkPopup', self.__eventWBLandmarkPopupApply, self.__eventWBLandmarkPopupBegin),
			CvUtil.EventWBScriptPopup : ('WBScriptPopup', self.__eventWBScriptPopupApply, self.__eventWBScriptPopupBegin),
			CvUtil.EventWBStartYearPopup : ('WBStartYearPopup', self.__eventWBStartYearPopupApply, self.__eventWBStartYearPopupBegin),
			CvUtil.EventShowWonder: ('ShowWonder', self.__eventShowWonderApply, self.__eventShowWonderBegin),
		}
## FfH Card Game: begin
		self.Events[CvUtil.EventSelectSolmniumPlayer] = ('selectSolmniumPlayer', self.__EventSelectSolmniumPlayerApply, self.__EventSelectSolmniumPlayerBegin)
		self.Events[CvUtil.EventSolmniumAcceptGame] = ('solmniumAcceptGame', self.__EventSolmniumAcceptGameApply, self.__EventSolmniumAcceptGameBegin)
		self.Events[CvUtil.EventSolmniumConcedeGame] = ('solmniumConcedeGame', self.__EventSolmniumConcedeGameApply, self.__EventSolmniumConcedeGameBegin)
## FfH Card Game: end

#################### EVENT STARTERS ######################
	def handleEvent(self, argsList):
		'EventMgr entry point'
		# extract the last 6 args in the list, the first arg has already been consumed
		self.origArgsList = argsList	# point to original
		tag = argsList[0]				# event type string
		idx = len(argsList)-6
		bDummy = false
		self.bDbg, bDummy, self.bAlt, self.bCtrl, self.bShift, self.bAllowCheats = argsList[idx:]
		ret = 0
		if self.EventHandlerMap.has_key(tag):
			fxn = self.EventHandlerMap[tag]
			ret = fxn(argsList[1:idx])
		return ret

#################### EVENT APPLY ######################
	def beginEvent( self, context, argsList=-1 ):
		'Begin Event'
		entry = self.Events[context]
		return entry[2]( argsList )

	def applyEvent( self, argsList ):
		'Apply the effects of an event '
		context, playerID, netUserData, popupReturn = argsList

		if context == CvUtil.PopupTypeEffectViewer:
			return CvDebugTools.g_CvDebugTools.applyEffectViewer( playerID, netUserData, popupReturn )

		entry = self.Events[context]

		if ( context not in CvUtil.SilentEvents ):
			self.reportEvent(entry, context, (playerID, netUserData, popupReturn) )
		return entry[1]( playerID, netUserData, popupReturn )   # the apply function

	def reportEvent(self, entry, context, argsList):
		'Report an Event to Events.log '
		if (gc.getGame().getActivePlayer() != -1):
			message = "DEBUG Event: %s (%s)" %(entry[0], gc.getActivePlayer().getName())
			CyInterface().addImmediateMessage(message,"")
			CvUtil.pyPrint(message)
		return 0

#################### ON EVENTS ######################
	def onKbdEvent(self, argsList):
		'keypress handler - return 1 if the event was consumed'

		eventType,key,mx,my,px,py = argsList
		game = gc.getGame()

		if (self.bAllowCheats):
			# notify debug tools of input to allow it to override the control
			argsList = (eventType,key,self.bCtrl,self.bShift,self.bAlt,mx,my,px,py,gc.getGame().isNetworkMultiPlayer())
			if ( CvDebugTools.g_CvDebugTools.notifyInput(argsList) ):
				return 0

		if ( eventType == self.EventKeyDown ):
			theKey=int(key)

#FfH: Added by Kael 07/05/2008
			if (theKey == int(InputTypes.KB_LEFT)):
				if self.bCtrl:
						CyCamera().SetBaseTurn(CyCamera().GetBaseTurn() - 45.0)
						return 1
				elif self.bShift:
						CyCamera().SetBaseTurn(CyCamera().GetBaseTurn() - 10.0)
						return 1

			if (theKey == int(InputTypes.KB_RIGHT)):
					if self.bCtrl:
							CyCamera().SetBaseTurn(CyCamera().GetBaseTurn() + 45.0)
							return 1
					elif self.bShift:
							CyCamera().SetBaseTurn(CyCamera().GetBaseTurn() + 10.0)
							return 1
#FfH: End Add

			CvCameraControls.g_CameraControls.handleInput( theKey )

# Ronnar: EventTriggerMenu START
# Shift+Ctrl+E in cheat mode
			if( theKey == int(InputTypes.KB_E) and self.bShift and self.bCtrl):
				iPlayer = gc.getGame().getActivePlayer()
				popupInfo = CyPopupInfo()
				popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON)
				popupInfo.setText(CyTranslator().getText("TXT_KEY_POPUP_SELECT_EVENT",()))
				popupInfo.setData1(iPlayer)
				popupInfo.setOnClickedPythonCallback("selectOneEvent")
				for i in range(gc.getNumEventTriggerInfos()):
					trigger = gc.getEventTriggerInfo(i)
					popupInfo.addPythonButton(str(trigger.getType()), "")
				popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_SELECT_NEVER_MIND", ()), "")
                
				popupInfo.addPopup(iPlayer)
# Ronnar: EventTriggerMenu END 
			
			if (self.bAllowCheats):
				# Shift - T (Debug - No MP)
				if (theKey == int(InputTypes.KB_T)):
					if ( self.bShift ):
						self.beginEvent(CvUtil.EventAwardTechsAndGold)
						#self.beginEvent(CvUtil.EventCameraControlPopup)
						return 1

				elif (theKey == int(InputTypes.KB_W)):
					if ( self.bShift and self.bCtrl):
						self.beginEvent(CvUtil.EventShowWonder)
						return 1

				# Shift - ] (Debug - currently mouse-overd unit, health += 10
				elif (theKey == int(InputTypes.KB_LBRACKET) and self.bShift ):
					unit = CyMap().plot(px, py).getUnit(0)
					if ( not unit.isNone() ):
						d = min( unit.maxHitPoints()-1, unit.getDamage() + 10 )
						unit.setDamage( d, PlayerTypes.NO_PLAYER )

				# Shift - [ (Debug - currently mouse-overd unit, health -= 10
				elif (theKey == int(InputTypes.KB_RBRACKET) and self.bShift ):
					unit = CyMap().plot(px, py).getUnit(0)
					if ( not unit.isNone() ):
						d = max( 0, unit.getDamage() - 10 )
						unit.setDamage( d, PlayerTypes.NO_PLAYER )

				elif (theKey == int(InputTypes.KB_F1)):
					if ( self.bShift ):
						CvScreensInterface.replayScreen.showScreen(False)
						return 1
					# don't return 1 unless you want the input consumed

				elif (theKey == int(InputTypes.KB_F2)):
					if ( self.bShift ):
						import CvDebugInfoScreen
						CvScreensInterface.showDebugInfoScreen()
						return 1

				elif (theKey == int(InputTypes.KB_F3)):
					if ( self.bShift ):
						CvScreensInterface.showDanQuayleScreen(())
						return 1

				elif (theKey == int(InputTypes.KB_F4)):
					if ( self.bShift ):
						CvScreensInterface.showUnVictoryScreen(())
						return 1

		return 0

	def onModNetMessage(self, argsList):
		'Called whenever CyMessageControl().sendModNetMessage() is called - this is all for you modders!'

		iData1, iData2, iData3, iData4, iData5 = argsList

#FfH Card Game: begin
#		print("Modder's net message!")
#		CvUtil.pyPrint( 'onModNetMessage' )
		if iData1 == CvUtil.Somnium : # iData1 == 0 : Solmnium message, iData2 = function, iData3 to iData5 = parameters
			if iData2 == 0 :
				if (iData3 == gc.getGame().getActivePlayer()):
					self.__EventSelectSolmniumPlayerBegin()
			elif iData2 == 1 :
				if (iData4 == gc.getGame().getActivePlayer()):
					self.__EventSolmniumConcedeGameBegin((iData3, iData4))
			else :
				cs.applyAction(iData2, iData3, iData4, iData5)
# FfH Card Game: end
		if iData1 == CvUtil.TerraformPlan:
			gc.getPlayer(iData2).setTerraformPlan(iData3)

# Promotion Remove OOS Fix: begin
		if iData1 == CvUtil.UnitPromotion: # iData1 == 2 : Promotion, iData2 = playerId, iData3 = unitId, iData4 = promotion, iData5 = true/false
			if iData5 == 0:
				gc.getPlayer(iData2).getUnit(iData3).setHasPromotion(iData4,False)
			else :
				gc.getPlayer(iData2).getUnit(iData3).setHasPromotion(iData4,True)
#Stunned OOS Fix: begin
		if iData1 == CvUtil.UnitStun: # iData1 == 3 : UnitStun, iData2 = playerId, iData3 = unitId, iData4 = finishMoves, iData5 = setHasCasted
			if iData4 == 1:
				gc.getPlayer(iData2).getUnit(iData3).finishMoves()
			if iData5 == 1:
				gc.getPlayer(iData2).getUnit(iData3).setHasCasted(True)


# Promotion Remove OOS Fix: end

	def onInit(self, argsList):
		'Called when Civ starts up'
		CvUtil.pyPrint( 'OnInit' )

	def onUpdate(self, argsList):
		'Called every frame'
		fDeltaTime = argsList[0]

		# allow camera to be updated
		CvCameraControls.g_CameraControls.onUpdate( fDeltaTime )

	def onWindowActivation(self, argsList):
		'Called when the game window activates or deactivates'
		bActive = argsList[0]

	def onUnInit(self, argsList):
		'Called when Civ shuts down'
		CvUtil.pyPrint('OnUnInit')

	def onPreSave(self, argsList):
		"called before a game is actually saved"
		CvUtil.pyPrint('OnPreSave')

	def onSaveGame(self, argsList):
		"return the string to be saved - Must be a string"
		return ""

	def onLoadGame(self, argsList):
		CvAdvisorUtils.resetNoLiberateCities()
		return 0

	def onGameStart(self, argsList):
		'Called at the start of the game'

#   	introMovie = CvIntroMovieScreen.CvIntroMovieScreen()
#		introMovie.interfaceScreen()

		cf.CreateTerrainOnGameStart()
		cf.CreateFeatureOnGameStart()
		iUnit = gc.getInfoTypeForString('UNIT_ZARCAZ')
		cf.addUnit(iUnit)

		iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
		iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
		iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
		iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
		iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')

		if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_THAW):
			for i in range (CyMap().numPlots()):
				pPlot = CyMap().plotByIndex(i)
				if pPlot.getFeatureType() == -1:
					if pPlot.getImprovementType() == -1:
						if pPlot.isWater() == False:
							iTerrain = pPlot.getTerrainType()
							if iTerrain == iTundra:
								pPlot.setTempTerrainType(iSnow, CyGame().getSorenRandNum(90, "Bob") + 10)
							if iTerrain == iGrass:
								pPlot.setTempTerrainType(iTundra, CyGame().getSorenRandNum(90, "Bob") + 10)
							if iTerrain == iPlains:
								pPlot.setTempTerrainType(iTundra, CyGame().getSorenRandNum(90, "Bob") + 10)
							if iTerrain == iDesert:
								pPlot.setTempTerrainType(iPlains, CyGame().getSorenRandNum(90, "Bob") + 10)

		for iPlayer in range(gc.getMAX_PLAYERS()):
			player = gc.getPlayer(iPlayer)
			if (player.isAlive() and player.isHuman()):
				#Elohim
				if player.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ELOHIM'):
					cf.showUniqueImprovements(iPlayer)		

#Init Diplomacy

		for iPlayer in range(gc.getMAX_PLAYERS()):
			pPlayer = gc.getPlayer(iPlayer)
			if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_LANUN'):
				eTeam = gc.getTeam(gc.getPIRATES_TEAM())
				eTeam.makePeace(pPlayer.getTeam())
				gc.getTeam(pPlayer.getTeam()).makePeace(gc.getPIRATES_TEAM())
			if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
				eTeam = gc.getTeam(gc.getWHITEHAND_TEAM())
				eTeam.makePeace(pPlayer.getTeam())
				gc.getTeam(pPlayer.getTeam()).makePeace(gc.getWHITEHAND_TEAM())

#xtended
			if not pPlayer.isHuman():
				if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_GRIGORI'):
					eTeam = gc.getTeam(pPlayer.getTeam())
					eTeam.setHasTech(gc.getInfoTypeForString('TECH_HEALERS_GUILD'),true,-1,false,false)
				if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
					eTeam = gc.getTeam(pPlayer.getTeam())
					eTeam.setHasTech(gc.getInfoTypeForString('TECH_MYSTICISM'),true,-1,false,false)
				if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
					eTeam = gc.getTeam(pPlayer.getTeam())
					eTeam.setHasTech(gc.getInfoTypeForString('TECH_HUNTERS_GUILD'),true,-1,false,false)
					eTeam.setHasTech(gc.getInfoTypeForString('TECH_ESUS_GUILD'),true,-1,false,false)
				if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_ASSASSIN')):
					eTeam = gc.getTeam(pPlayer.getTeam())
					eTeam.setHasTech(gc.getInfoTypeForString('TECH_ESUS_GUILD'),true,-1,false,false)

#				if pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_LANUN') or pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_SIDAR') or pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_SVARTALFAR'):
#					eTeam = gc.getTeam(pPlayer.getTeam())
#					eTeam.setHasTech(gc.getInfoTypeForString('TECH_ESUS_GUILD'),true,-1,false,false)
#				if (pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_DOVIELLO') or pPlayer.getCivilizationType()==gc.getInfoTypeForString('CIVILIZATION_CLAN_OF_EMBERS')):
#					eTeam = gc.getTeam(pPlayer.getTeam())
#					eTeam.setHasTech(gc.getInfoTypeForString('TECH_SLAVERY'),true,-1,false,false)

			#Xtended Starting Settler bonus
#			py = PyPlayer(iPlayer)
#			for pUnit in py.getUnitList():
#				if pUnit.getUnitClassType() == gc.getInfoTypeForString('UNITCLASS_SETTLER'):
#					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STARTING_SETTLER'), True)
#					if not pPlayer.isHuman():
#						pPlot = pUnit.plot()
#						pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_GRASS'), True, True)
#						pPlot.setFeatureType(-1, -1)
#						pPlot.setImprovementType(-1)
				
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SLAVEMASTER')):
				eTeam = gc.getTeam(pPlayer.getTeam())
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_SLAVERY'),true,-1,false,false)
				
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_MERCIFUL')):
				eTeam = gc.getTeam(pPlayer.getTeam())
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_HEALERS_GUILD'),true,-1,false,false)
				
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_TRADER')):
				eTeam = gc.getTeam(pPlayer.getTeam())
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_CARTOGRAPHY'),true,-1,false,false)

#attune mana Bonuses for Players
		for iPlayer in range(gc.getMAX_PLAYERS()):
			pPlayer = gc.getPlayer(iPlayer)

			if pPlayer.isAlive():
				gc.getTeam(pPlayer.getTeam()).setHasTech(gc.getInfoTypeForString('TECH_EXPLORATION'), true, -1, false, false)
			#disable for now
			continue
			AlignmentList = []
			if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):
				AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_LIFE'))
			if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
				AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_DEATH'))
				AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_CHAOS'))
			AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_AIR'))
			AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_FIRE'))
			AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_ICE'))
			AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_MIND'))
			AlignmentList.append(gc.getInfoTypeForString('BONUS_MANA_NATURE'))

			for i in range(3):
				iPick = CyGame().getSorenRandNum(len(AlignmentList), "Mana Creation")
				if i==0:
					pPlayer.setManaBonus2(AlignmentList[iPick])
				if i==1:
					pPlayer.setManaBonus3(AlignmentList[iPick])
				if i==2:
					pPlayer.setManaBonus4(AlignmentList[iPick])
				AlignmentList.remove(AlignmentList[iPick])
#set civics for barbs

		bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
		bPlayer.setCivics(gc.getInfoTypeForString('CIVICOPTION_CULTURAL_VALUES'),gc.getInfoTypeForString('CIVIC_SACRIFICE_THE_WEAK'))
		bPlayer.setCivics(gc.getInfoTypeForString('CIVICOPTION_LABOR'),gc.getInfoTypeForString('CIVIC_SLAVERY'))

		bPlayer = gc.getPlayer(gc.getWILDMANA_PLAYER())
		iRawMana = gc.getInfoTypeForString('BONUS_MANA')
		iDesert = gc.getInfoTypeForString('TERRAIN_DESERT')
		iGrass = gc.getInfoTypeForString('TERRAIN_GRASS')
		iPlains = gc.getInfoTypeForString('TERRAIN_PLAINS')
		iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
		iTundra = gc.getInfoTypeForString('TERRAIN_TUNDRA')
		iMarsh = gc.getInfoTypeForString('TERRAIN_MARSH')
		lList = ['BONUS_MANA_ICE', 'BONUS_MANA_AIR', 'BONUS_MANA_CHAOS', 'BONUS_MANA_DEATH', 'BONUS_MANA_LIFE', 'BONUS_MANA_MIND', 'BONUS_MANA_NATURE', 'BONUS_MANA_FIRE', 'BONUS_MANA']

		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getImprovementType() == -1:
				if pPlot.isWater() == False:
					iBonus = pPlot.getBonusType(-1)
					if iBonus == iRawMana:
						iManaRnd = CyGame().getSorenRandNum(100, "Mana Creation")
						sMana = lList[CyGame().getSorenRandNum(len(lList), "Pick Mana")-1]
						iBonus = gc.getInfoTypeForString(sMana)
						iPlane = pPlot.getPlane()
						if iPlane == -1 or gc.getPlaneInfo(iPlane).isBonusSpawn(iBonus):
							pPlot.setBonusType(iBonus)



#set initial global yields
		for iPlayer in range(gc.getMAX_PLAYERS()):
			pPlayer = gc.getPlayer(iPlayer)
			iValue = 200
			iValue*=gc.getGameSpeedInfo(CyGame().getGameSpeedType()).getConstructPercent()
			iValue/=100
#			if not pPlayer.isHuman():
#				if CyGame().getHandicapType() > 2:
#					iValue += 100*CyGame().getHandicapType()
#			else:
#				if CyGame().getHandicapType() > 2:
#					iValue -= 130 * (CyGame().getHandicapType() - 2)
			pPlayer.changeGold(iValue)
			if pPlayer.getGlobalYield(YieldTypes.YIELD_METAL) == 0:
				pPlayer.changeGlobalYield(YieldTypes.YIELD_METAL,iValue)
			if pPlayer.getGlobalYield(YieldTypes.YIELD_LUMBER) == 0:
				pPlayer.changeGlobalYield(YieldTypes.YIELD_LUMBER,iValue)
			if pPlayer.getGlobalYield(YieldTypes.YIELD_HERB) == 0:
				pPlayer.changeGlobalYield(YieldTypes.YIELD_HERB,iValue)
			if pPlayer.getGlobalYield(YieldTypes.YIELD_LEATHER) == 0:
				pPlayer.changeGlobalYield(YieldTypes.YIELD_LEATHER,iValue)
			if pPlayer.getGlobalYield(YieldTypes.YIELD_STONE) == 0:
				pPlayer.changeGlobalYield(YieldTypes.YIELD_STONE,iValue)
				
		#make Sure Unique Improvements have their Bonus, added Sephi
		for i in range (CyMap().numPlots()):
			pPlot = CyMap().plotByIndex(i)
			if pPlot.getImprovementType()!=-1:
				if gc.getImprovementInfo(pPlot.getImprovementType()).isUnique():
					if gc.getImprovementInfo(pPlot.getImprovementType()).getBonusConvert()!=-1:
						pPlot.setBonusType(gc.getImprovementInfo(pPlot.getImprovementType()).getBonusConvert())

		if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_CHALLENGE_HOLY_CITY) and (not gc.getGame().isNetworkMultiPlayer()):
			pPlayer =gc.getPlayer(0)
			iCiv= pPlayer.getCivilizationType()
			iKhazad=gc.getInfoTypeForString('CIVILIZATION_KHAZAD')
			iBannor=gc.getInfoTypeForString('CIVILIZATION_BANNOR')
			iMalakim=gc.getInfoTypeForString('CIVILIZATION_MALAKIM')
			iLanun=gc.getInfoTypeForString('CIVILIZATION_LANUN')
			iLjosalfar=gc.getInfoTypeForString('CIVILIZATION_LJOSALFAR')
			iSheaim=gc.getInfoTypeForString('CIVILIZATION_SHEAIM')
			iRandom=-1
			iNewID=-1

			iRandom=CyGame().getSorenRandNum(6, "Holy City pick")
			if iCiv==iKhazad:
				iRandom=0
			if iCiv==iBannor:
				iRandom=1
			if iCiv==iMalakim:
				iRandom=2
			if iCiv==iLanun:
				iRandom=3
			if iCiv==iLjosalfar:
				iRandom=4
			if iCiv==iSheaim:
				iRandom=5

			if not iRandom==-1:
				iNewID == CyGame().placeCityState(iRandom)
				CyGame().reassignPlayerAdvanced(0, iNewID, -1)

		if (gc.getGame().getGameTurnYear() == gc.getDefineINT("START_YEAR") and not gc.getGame().isOption(GameOptionTypes.GAMEOPTION_ADVANCED_START)):
			if not CyGame().getWBMapScript():
				for iPlayer in range(gc.getMAX_PLAYERS()):
					player = gc.getPlayer(iPlayer)
					if (player.isAlive() and player.isHuman()):
						popupInfo = CyPopupInfo()
						popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
						popupInfo.setText(u"showDawnOfMan")
						popupInfo.addPopup(iPlayer)
		else:
			CyInterface().setSoundSelectionReady(true)

		if gc.getGame().isPbem():
			for iPlayer in range(gc.getMAX_PLAYERS()):
				player = gc.getPlayer(iPlayer)
				if (player.isAlive() and player.isHuman()):
					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_DETAILS)
					popupInfo.setOption1(true)
					popupInfo.addPopup(iPlayer)

		CvAdvisorUtils.resetNoLiberateCities()

		#debug tool to see if Bonuses are placed correctly
		if 2 < 1:
			for i in range(gc.getNumBonusInfos()):
				iCount = 0
				for j in range(CyMap().numPlots()):
					if CyMap().plotByIndex(j).getBonusType(-1) == i:
						iCount += 1
				message = "Number of %s : %d" %(gc.getBonusInfo(i).getDescription(), iCount)
				CyInterface().addMessage(0,False,25,message,'',3,'',ColorTypes(7),0,0,True,True)

		#debug tool to see if Lairs are placed correctly
		if 2 < 1:
			for i in range(gc.getNumImprovementInfos()):
				if(gc.getImprovementInfo(i).getSpawnUnitType() != -1):
					iCount = 0
					for j in range(CyMap().numPlots()):
						if CyMap().plotByIndex(j).getImprovementType() == i:
							iCount += 1
					message = "Number of %s : %d" %(gc.getImprovementInfo(i).getDescription(), iCount)
					CyInterface().addMessage(0,False,25,message,'',3,'',ColorTypes(7),0,0,True,True)

		#debug tool to see max wilderness
		if 2 < 1:
			iCount = 0
			for j in range(CyMap().numPlots()):
				if CyMap().plotByIndex(j).getWilderness() > iCount:
					iCount = CyMap().plotByIndex(j).getWilderness()
			message = "Max Wilderness : %d" %(iCount)
			CyInterface().addMessage(0,False,25,message,'',3,'',ColorTypes(7),0,0,True,True)

		#safety check for XML
		for i in range(gc.getNumBuildingInfos()):
			if(gc.getBuildingInfo(i).getPrereqBuildingClass() != -1):
				for j in range(gc.getNumBuildingInfos()):
					if (i != j):
						if(gc.getBuildingInfo(j).getBuildingClassType() == gc.getBuildingInfo(i).getPrereqBuildingClass()):
							if(gc.getBuildingInfo(i).getBuildingClassType() == gc.getBuildingInfo(j).getPrereqBuildingClass()):
								message = "ERROR: %s requires (%s)" %(gc.getBuildingInfo(i).getDescription(), gc.getBuildingInfo(j).getDescription())
								CyInterface().addImmediateMessage(message,"")
								message = "and %s requires (%s)" %(gc.getBuildingInfo(j).getDescription(), gc.getBuildingInfo(i).getDescription())
								CyInterface().addImmediateMessage(message,"")

		for i in range(gc.getNumTechInfos()):
			for j in range(gc.getNumTechInfos()):
				if (i != j):
					if cf.techneeded(i, j):
						if cf.techneeded(j, i):
							message = "ERROR: %s requires (%s)" %(gc.getTechInfo(i).getDescription(), gc.getTechInfo(j).getDescription())
							CyInterface().addImmediateMessage(message,"")
							message = "and %s requires (%s)" %(gc.getTechInfo(j).getDescription(), gc.getTechInfo(i).getDescription())
							CyInterface().addImmediateMessage(message,"")

	def onGameEnd(self, argsList):
		'Called at the End of the game'
		print("Game is ending")
		return

	def onBeginGameTurn(self, argsList):
		'Called at the beginning of the end of each turn'
		iGameTurn = argsList[0]

		iOrthusTurn = 55
		if CyGame().getStartEra()>0:
			iOrthusTurn +=25
		if not CyGame().isUnitClassMaxedOut(gc.getInfoTypeForString('UNITCLASS_ORTHUS'), 0):
			#if not CyGame().isOption(gc.getInfoTypeForString('GAMEOPTION_NO_ORTHUS')):
				bOrthus = False
				if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
					if iGameTurn >= iOrthusTurn / 3 * 2:
						bOrthus = True
				if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_NORMAL'):
					if iGameTurn >= iOrthusTurn:
						bOrthus = True
				if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
					if iGameTurn >= iOrthusTurn * 3 / 2:
						bOrthus = True
				if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
					if iGameTurn >= iOrthusTurn * 3:
						bOrthus = True
				if bOrthus:
					bvalid=true
					for i in range (CyMap().numPlots()):
						pPlot = CyMap().plotByIndex(i)
						iPlot = -1
						if pPlot.getImprovementType()==gc.getInfoTypeForString('IMPROVEMENT_GOBLIN_FORT'):
							bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
							if not pPlot.isVisibleOtherUnit(gc.getBARBARIAN_PLAYER()):
								bPlayer.initUnit(gc.getInfoTypeForString('UNIT_ORTHUS'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
								bvalid=false
								break

					if bvalid:
						iUnit = gc.getInfoTypeForString('UNIT_ORTHUS')
						cf.addUnit(iUnit)
						cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_ORTHUS_CREATION",()), str(gc.getUnitInfo(iUnit).getImage()))

#		if not CyGame().isOption(gc.getInfoTypeForString('GAMEOPTION_NO_PLOT_COUNTER')):
#			cf.doHellTerrain()

#		CvTopCivs.CvTopCivs().turnChecker(iGameTurn)

	def onEndGameTurn(self, argsList):
		'Called at the end of the end of each turn'
		iGameTurn = argsList[0]

	def onBeginPlayerTurn(self, argsList):
		'Called at the beginning of a players turn'
		iGameTurn, iPlayer = argsList
		pPlayer = gc.getPlayer(iPlayer)
		player = PyPlayer(iPlayer)

		cf.doTurnCheckAlignment(iPlayer)
		
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
			for pyCity in PyPlayer(iPlayer).getCityList():
				pCity = pyCity.GetCy()
				pCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ORDER'), False, True, True)
				pCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'), True, True, True)


		if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'):
			cf.doTurnCondantis(iPlayer)
			if pPlayer.getCurrentEra() != gc.getInfoTypeForString('ERA_VEIL'):
				pPlayer.setCurrentEra(gc.getInfoTypeForString('ERA_VEIL'))

		if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_ORDER'):
			if pPlayer.getCurrentEra() != gc.getInfoTypeForString('ERA_ORDE'):
				pPlayer.setCurrentEra(gc.getInfoTypeForString('ERA_ORDE'))

		if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_FELLOWSHIP_OF_LEAVES'):
			if pPlayer.getCurrentEra() != gc.getInfoTypeForString('ERA_LEAF'):
				pPlayer.setCurrentEra(gc.getInfoTypeForString('ERA_LEAF'))

		if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH'):
			if pPlayer.getCurrentEra() != gc.getInfoTypeForString('ERA_RUNE'):
				pPlayer.setCurrentEra(gc.getInfoTypeForString('ERA_RUNE'))

		if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS'):
			cf.doTurnCondantis(iPlayer)
			if pPlayer.getCurrentEra() != gc.getInfoTypeForString('ERA_OCTO'):
				pPlayer.setCurrentEra(gc.getInfoTypeForString('ERA_OCTO'))

		if pPlayer.getStateReligion() == gc.getInfoTypeForString('RELIGION_THE_EMPYREAN'):
			if pPlayer.getCurrentEra() != gc.getInfoTypeForString('ERA_EMPY'):
				pPlayer.setCurrentEra(gc.getInfoTypeForString('ERA_EMPY'))

		if not pPlayer.isHuman():
			if not CyGame().getWBMapScript():
				cf.warScript(iPlayer)

		if pPlayer.getCivics(gc.getInfoTypeForString('CIVICOPTION_FAERIE_COURT')) == gc.getInfoTypeForString('CIVIC_CRUSADE'):
			cf.doCrusade(iPlayer)

		cf.doTurnKhazad(iPlayer)

		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_LUCHUIRP'):
			cf.doTurnLuchuirp(iPlayer)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_INSANE')):
			if CyGame().getSorenRandNum(1000, "Insane") < 20:
				iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_TRAIT_INSANE')
				triggerData = pPlayer.initTriggeredData(iEvent, true, -1, -1, -1, iPlayer, -1, -1, -1, -1, -1)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_ADAPTIVE')):
			iCycle = 100
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
				iCycle = 75
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
				iCycle = 150
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
				iCycle = 300
			for i in range(10):
				if (i * iCycle) - 5 == iGameTurn:
					iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_TRAIT_ADAPTIVE')
					triggerData = pPlayer.initTriggeredData(iEvent, true, -1, -1, -1, iPlayer, -1, -1, -1, -1, -1)

		if pPlayer.isHuman():
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_BARBARIAN')):
				eTeam = gc.getTeam(gc.getPlayer(gc.getBARBARIAN_PLAYER()).getTeam())
				iTeam = pPlayer.getTeam()
				if eTeam.isAtWar(iTeam) == False:
					if 2 * CyGame().getPlayerScore(iPlayer) >= 3 * CyGame().getPlayerScore(CyGame().getRankPlayer(1)):
						#Xtended to give some breather to non-Ancient start.
						if CyGame().getStartEra() == gc.getInfoTypeForString('ERA_ANCIENT'): 
							if iGameTurn >= 20:
								eTeam.declareWar(iTeam, false, WarPlanTypes.WARPLAN_TOTAL)
								if iPlayer == CyGame().getActivePlayer():
									cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_BARBARIAN_DECLARE_WAR",()), 'art/interface/popups/Barbarian.dds')
						else:
							if iGameTurn >= 50:
								eTeam.declareWar(iTeam, false, WarPlanTypes.WARPLAN_TOTAL)
								if iPlayer == CyGame().getActivePlayer():
									cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_BARBARIAN_DECLARE_WAR",()), 'art/interface/popups/Barbarian.dds')
		
		if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_AMURITES'):
			if pPlayer.getDisableSpellcasting() > 0:
				cf.doArcaneLacuna(iPlayer)

	#disabled in dll
	def onEndPlayerTurn(self, argsList):
		'Called at the end of a players turn'
		iGameTurn, iPlayer = argsList

		if (gc.getGame().getElapsedGameTurns() == 1):
			if (gc.getPlayer(iPlayer).isHuman()):
				if (gc.getPlayer(iPlayer).canRevolution(0)):
					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_CHANGECIVIC)
					popupInfo.addPopup(iPlayer)

		CvAdvisorUtils.resetAdvisorNags()
		CvAdvisorUtils.endTurnFeats(iPlayer)

	def onEndTurnReady(self, argsList):
		iGameTurn = argsList[0]

	def onFirstContact(self, argsList):
		'Contact'
		iTeamX,iHasMetTeamY = argsList
		if (not self.__LOG_CONTACT):
			return
		CvUtil.pyPrint('Team %d has met Team %d' %(iTeamX, iHasMetTeamY))

	def onCombatResult(self, argsList):
		'Combat Result'
		pWinner,pLoser = argsList
		playerX = PyPlayer(pWinner.getOwner())
		unitX = PyInfo.UnitInfo(pWinner.getUnitType())
		playerY = PyPlayer(pLoser.getOwner())
		unitY = PyInfo.UnitInfo(pLoser.getUnitType())
		if (not self.__LOG_COMBAT):
			return
		if playerX and playerX and unitX and playerY:
			CvUtil.pyPrint('Player %d Civilization %s Unit %s has defeated Player %d Civilization %s Unit %s'
				%(playerX.getID(), playerX.getCivilizationName(), unitX.getDescription(),
				playerY.getID(), playerY.getCivilizationName(), unitY.getDescription()))
				
	def onCombatLogCalc(self, argsList):
		'Combat Result'
		genericArgs = argsList[0][0]
		cdAttacker = genericArgs[0]
		cdDefender = genericArgs[1]
		iCombatOdds = genericArgs[2]
		CvUtil.combatMessageBuilder(cdAttacker, cdDefender, iCombatOdds)

	def onCombatLogHit(self, argsList):
		'Combat Message'
		global gCombatMessages, gCombatLog
		genericArgs = argsList[0][0]
		cdAttacker = genericArgs[0]
		cdDefender = genericArgs[1]
		iIsAttacker = genericArgs[2]
		iDamage = genericArgs[3]

		if cdDefender.eOwner == cdDefender.eVisualOwner:
			szDefenderName = gc.getPlayer(cdDefender.eOwner).getNameKey()
		else:
			szDefenderName = localText.getText("TXT_KEY_TRAIT_PLAYER_UNKNOWN", ())
		if cdAttacker.eOwner == cdAttacker.eVisualOwner:
			szAttackerName = gc.getPlayer(cdAttacker.eOwner).getNameKey()
		else:
			szAttackerName = localText.getText("TXT_KEY_TRAIT_PLAYER_UNKNOWN", ())

		if (iIsAttacker == 0):
			combatMessage = localText.getText("TXT_KEY_COMBAT_MESSAGE_HIT", (szDefenderName, cdDefender.sUnitName, iDamage, cdDefender.iCurrHitPoints, cdDefender.iMaxHitPoints))
			CyInterface().addCombatMessage(cdAttacker.eOwner,combatMessage)
			CyInterface().addCombatMessage(cdDefender.eOwner,combatMessage)
			if (cdDefender.iCurrHitPoints <= 0):
				combatMessage = localText.getText("TXT_KEY_COMBAT_MESSAGE_DEFEATED", (szAttackerName, cdAttacker.sUnitName, szDefenderName, cdDefender.sUnitName))
				CyInterface().addCombatMessage(cdAttacker.eOwner,combatMessage)
				CyInterface().addCombatMessage(cdDefender.eOwner,combatMessage)
		elif (iIsAttacker == 1):
			combatMessage = localText.getText("TXT_KEY_COMBAT_MESSAGE_HIT", (szAttackerName, cdAttacker.sUnitName, iDamage, cdAttacker.iCurrHitPoints, cdAttacker.iMaxHitPoints))
			CyInterface().addCombatMessage(cdAttacker.eOwner,combatMessage)
			CyInterface().addCombatMessage(cdDefender.eOwner,combatMessage)
			if (cdAttacker.iCurrHitPoints <= 0):
				combatMessage = localText.getText("TXT_KEY_COMBAT_MESSAGE_DEFEATED", (szDefenderName, cdDefender.sUnitName, szAttackerName, cdAttacker.sUnitName))
				CyInterface().addCombatMessage(cdAttacker.eOwner,combatMessage)
				CyInterface().addCombatMessage(cdDefender.eOwner,combatMessage)

	def onImprovementBuilt(self, argsList):
		'Improvement Built'
		iImprovement, iX, iY, bWorker = argsList
		pPlot = CyMap().plot(iX, iY)

		if gc.getImprovementInfo(iImprovement).isUnique():
			CyEngine().addLandmark(pPlot, CvUtil.convertToStr(gc.getImprovementInfo(iImprovement).getDescription()))

			if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_RING_OF_CARCER'):
				pPlot.setMinLevel(10)
				bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())
				bPlayer.initUnit(gc.getInfoTypeForString('UNIT_BRIGIT_HELD'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_SEVEN_PINES'):
				pPlot.setTerrainType(gc.getInfoTypeForString('TERRAIN_GRASS'), True, True)
				pPlot.setFeatureType(gc.getInfoTypeForString('FEATURE_FOREST'), 1)
				CyEngine().addLandmark(pPlot, CvUtil.convertToStr(gc.getImprovementInfo(iImprovement).getDescription()))

		if gc.getInfoTypeForString('IMPROVEMENT_DEVIL_PORTAL') == iImprovement:
			CyEngine().addLandmark(pPlot, CvUtil.convertToStr('Dimensional Portal'))
						
		cf.onImprovementBuilt(argsList)

		if (not self.__LOG_IMPROVEMENT):
			return
		CvUtil.pyPrint('Improvement %s was built at %d, %d'
			%(PyInfo.ImprovementInfo(iImprovement).getDescription(), iX, iY))

	def onImprovementDestroyed(self, argsList):
		'Improvement Destroyed'
		iImprovement, iOwner, iX, iY = argsList

		if gc.getImprovementInfo(iImprovement).isUnique():
			pPlot = CyMap().plot(iX, iY)
			CyEngine().removeLandmark(pPlot)

			if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_RING_OF_CARCER'):
				pPlot.setMinLevel(-1)
			
		if iImprovement == gc.getInfoTypeForString('IMPROVEMENT_NECROTOTEM'):
			CyGame().changeGlobalCounter(-2)

		if (not self.__LOG_IMPROVEMENT):
			return
		CvUtil.pyPrint('Improvement %s was Destroyed at %d, %d'
			%(PyInfo.ImprovementInfo(iImprovement).getDescription(), iX, iY))

	def onRouteBuilt(self, argsList):
		'Route Built'
		iRoute, iX, iY = argsList
		if (not self.__LOG_IMPROVEMENT):
			return
		CvUtil.pyPrint('Route %s was built at %d, %d'
			%(gc.getRouteInfo(iRoute).getDescription(), iX, iY))

	def onPlotRevealed(self, argsList):
		'Plot Revealed'
		pPlot = argsList[0]
		iTeam = argsList[1]

	def onPlotFeatureRemoved(self, argsList):
		'Plot Revealed'
		pPlot = argsList[0]
		iFeatureType = argsList[1]
		pCity = argsList[2] # This can be null

	def onPlotPicked(self, argsList):
		'Plot Picked'
		pPlot = argsList[0]
		CvUtil.pyPrint('Plot was picked at %d, %d'
			%(pPlot.getX(), pPlot.getY()))

	def onNukeExplosion(self, argsList):
		'Nuke Explosion'
		pPlot, pNukeUnit = argsList
		CvUtil.pyPrint('Nuke detonated at %d, %d'
			%(pPlot.getX(), pPlot.getY()))

	def onGotoPlotSet(self, argsList):
		'Nuke Explosion'
		pPlot, iPlayer = argsList

	def onBuildingBuilt(self, argsList):
		'Building Completed'
		pCity, iBuildingType = argsList
		player = pCity.getOwner()
		pPlayer = gc.getPlayer(player)
		pPlot = pCity.plot()
		game = gc.getGame()
		iBuildingClass = gc.getBuildingInfo(iBuildingType).getBuildingClassType()

			
		if ((not gc.getGame().isNetworkMultiPlayer()) and (pCity.getOwner() == gc.getGame().getActivePlayer()) and isWorldWonderClass(iBuildingClass)):
			if gc.getBuildingInfo(iBuildingType).getMovie():
				# If this is a wonder...
				popupInfo = CyPopupInfo()
				popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
				popupInfo.setData1(iBuildingType)
				popupInfo.setData2(pCity.getID())
				popupInfo.setData3(0)
				popupInfo.setText(u"showWonderMovie")
				popupInfo.addPopup(pCity.getOwner())

		if pCity.isCapital():
			if gc.getBuildingInfo(iBuildingType).isDistrict():
				iCount = pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_GRAND_MENAGERIE'))
				iCount += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_BAZAAR_OF_MAMMON'))
				iCount += pPlayer.getBuildingClassCount(gc.getInfoTypeForString('BUILDINGCLASS_GREAT_LIBRARY'))
				if iCount == 0:
					if iBuildingType == gc.getInfoTypeForString('BUILDING_SAGE_DISTRICT'):
						pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_GREAT_LIBRARY'), 1)
					if iBuildingType == gc.getInfoTypeForString('BUILDING_MERCHANT_DISTRICT'):
						pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_BAZAAR_OF_MAMMON'), 1)
					if iBuildingType == gc.getInfoTypeForString('BUILDING_NOBLE_DISTRICT'):
						pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_GRAND_MENAGERIE'), 1)
					if iBuildingType == gc.getInfoTypeForString('BUILDING_AUSTRIN_SAGE_DISTRICT'):
						pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_GREAT_LIBRARY'), 1)
					if iBuildingType == gc.getInfoTypeForString('BUILDING_AUSTRIN_MERCHANT_DISTRICT'):
						pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_BAZAAR_OF_MAMMON'), 1)
					if iBuildingType == gc.getInfoTypeForString('BUILDING_AUSTRIN_NOBLE_DISTRICT'):
						pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_GRAND_MENAGERIE'), 1)

		if iBuildingType == gc.getInfoTypeForString('BUILDING_INFERNAL_GRIMOIRE'):
			if CyGame().getSorenRandNum(100, "Bob") <= 20:
				pPlot2 = cf.findClearPlot(-1, pPlot)
				if pPlot2 != -1:
					bPlayer = pPlayer
					#Infernal player will get the Balor
					if(pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_INFERNAL')):
						bPlayer = gc.getPlayer(gc.getBARBARIAN_PLAYER())

					newUnit = bPlayer.initUnit(gc.getInfoTypeForString('UNIT_BALOR'), pPlot2.getX(), pPlot2.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
					CyInterface().addMessage(pCity.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_INFERNAL_GRIMOIRE_BALOR",()),'AS2D_BALOR',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,6,11',ColorTypes(7),newUnit.getX(),newUnit.getY(),True,True)

					if pCity.getOwner() == CyGame().getActivePlayer():
						cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_INFERNAL_GRIMOIRE_BALOR",()), 'art/interface/popups/Balor.dds')

		if iBuildingType == gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_FINAL'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_EXALTED'), 0)
		if iBuildingType == gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_EXALTED'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_DIVINE'), 0)
		if iBuildingType == gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_DIVINE'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_CONSECRATED'), 0)
		if iBuildingType == gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_CONSECRATED'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_BLESSED'), 0)
		if iBuildingType == gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_BLESSED'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_ANOINTED'), 0)
		if iBuildingType == gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR_ANOINTED'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_ALTAR_OF_THE_LUONNOTAR'), 0)

		if iBuildingType == gc.getInfoTypeForString('BUILDING_MERCURIAN_GATE'):
			iMercurianPlayer = cf.getOpenPlayer()
			iTeam = pPlayer.getTeam()
			pPlot2 = cf.findClearPlot(-1, pCity.plot())
			if (iMercurianPlayer != -1 and pPlot2 != -1):
				for i in range(pPlot.getNumUnits(), -1, -1):
					pUnit = pPlot.getUnit(i)
					pUnit.setXY(pPlot2.getX(), pPlot2.getY(), true, true, true)
				CyGame().addPlayerAdvanced(iMercurianPlayer, iTeam, gc.getInfoTypeForString('LEADER_BASIUM'), gc.getInfoTypeForString('CIVILIZATION_MERCURIANS'))
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_BASIUM'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_SETTLER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_OPHANIM'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_VALKYRIE'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_SERAPH'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_HERALD'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_ANGEL'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_ANGEL'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_ANGEL'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_ANGEL'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_ANGEL'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_ANGEL'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_WORKER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
				gc.getPlayer(iMercurianPlayer).initUnit(gc.getInfoTypeForString('UNIT_WORKER'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)

				if pPlayer.isHuman():
					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON)
					popupInfo.setText(CyTranslator().getText("TXT_KEY_POPUP_CONTROL_MERCURIANS",()))
					popupInfo.setData1(player)
					popupInfo.setData2(iMercurianPlayer)
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_YES", ()), "")
					popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_NO", ()), "")
					popupInfo.setOnClickedPythonCallback("reassignPlayer")
					popupInfo.addPopup(player)

		if iBuildingType == gc.getInfoTypeForString('BUILDING_TOWER_OF_NECROMANCY'):
			iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
			iStrong = gc.getInfoTypeForString('PROMOTION_STRONG')
#scions start
			iBH = gc.getInfoTypeForString('UNIT_BONE_HORDE')
			iSkeleton = gc.getInfoTypeForString('UNIT_SKELETON')
			iDiseased = gc.getInfoTypeForString('UNIT_DISEASED_CORPSE')
			iScions = gc.getInfoTypeForString('CIVILIZATION_SCIONS')
#scions end
			apUnitList = PyPlayer(player).getUnitList()
			for pUnit in apUnitList:
#scions start - All Undead units from non-Scions civs get the benefits of the Tower of N.
				if pPlayer.getCivilizationType() != iScions:
#scions end
					if pUnit.isHasPromotion(iUndead):
						pUnit.setHasPromotion(iStrong, True)
#scions start - Only certain Scions units get the benefits.  (A balance measure)
				else:
					if pUnit.getUnitType() == iBH:
						pUnit.setHasPromotion(iStrong, True)
					if pUnit.getUnitType() == iDiseased:
						pUnit.setHasPromotion(iStrong, True)
					if pUnit.getUnitType() == iSkeleton:
						pUnit.setHasPromotion(iStrong, True)


		if iBuildingType == gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_HAND'):
			iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
			iFlames = gc.getInfoTypeForString('FEATURE_FLAMES')
			iFloodPlains = gc.getInfoTypeForString('FEATURE_FLOOD_PLAINS')
			iForest = gc.getInfoTypeForString('FEATURE_FOREST')
			iJungle = gc.getInfoTypeForString('FEATURE_JUNGLE')
			iScrub = gc.getInfoTypeForString('FEATURE_SCRUB')
			iSmoke = gc.getInfoTypeForString('IMPROVEMENT_SMOKE')
			iBlizzard = gc.getInfoTypeForString('FEATURE_BLIZZARD')
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
							if CyGame().getSorenRandNum(100, "Snowfall") < 2:
								pLoopPlot.setFeatureType(iBlizzard, -1)
			CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_SNOWFALL'),pPlot.getPoint())

		if iBuildingType == gc.getInfoTypeForString('BUILDING_SUMMON_CIRCLE'):
			for pyCity in PyPlayer(pCity.getOwner()).getCityList():
				pLoopCity = pyCity.GetCy()
				if pLoopCity.getID() != pCity.getID():
					pLoopCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_SUMMON_CIRCLE'), 0)
					
		if iBuildingType == gc.getInfoTypeForString('BUILDING_GRAND_MENAGERIE'):
			if pPlayer.isHuman():
				if not CyGame().getWBMapScript():
					t = "TROPHY_FEAT_GRAND_MENAGERIE"
					if not CyGame().isHasTrophy(t):
						CyGame().changeTrophyValue(t, 1)

#Xtended
		if iBuildingType == gc.getInfoTypeForString('BUILDING_LUXURY_DISTRICT'):
			eResourceClass=gc.getInfoTypeForString('BUILDINGCLASS_LUXURY_DISTRICT')
			pTeam=gc.getTeam(pCity.getTeam())
			BonusList=[]
			BonusList.append(gc.getInfoTypeForString('BONUS_ALE'))
			BonusList.append(gc.getInfoTypeForString('BONUS_WINE'))
			BonusList.append(gc.getInfoTypeForString('BONUS_FINE_CLOTHES'))
			BonusList.append(gc.getInfoTypeForString('BONUS_JEWELS'))
			BonusList.append(gc.getInfoTypeForString('BONUS_COTTON'))
			BonusList.append(gc.getInfoTypeForString('BONUS_DYE'))
			BonusList.append(gc.getInfoTypeForString('BONUS_FUR'))
			BonusList.append(gc.getInfoTypeForString('BONUS_SILK'))
			BonusList.append(gc.getInfoTypeForString('BONUS_AMBER'))
			BonusList.append(gc.getInfoTypeForString('BONUS_GEMS'))
			BonusList.append(gc.getInfoTypeForString('BONUS_GOLD'))
			BonusList.append(gc.getInfoTypeForString('BONUS_SILVER'))
			BonusList.append(gc.getInfoTypeForString('BONUS_PEARL'))
			BonusList.append(gc.getInfoTypeForString('BONUS_SUGAR'))
			BonusList.append(gc.getInfoTypeForString('BONUS_SALT'))
			BonusList.append(gc.getInfoTypeForString('BONUS_TEA'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MARBLE'))

			iRnd = CyGame().getSorenRandNum(len(BonusList), "Luxury District Creation")
			pCity.setBuildingBonusChange(eResourceClass,BonusList[iRnd],1)
#			iRnd2 = CyGame().getSorenRandNum(len(BonusList), "Luxury District Creation")
#			pCity.setBuildingBonusChange(eResourceClass,BonusList[iRnd2],1)
#			iRnd3 = CyGame().getSorenRandNum(len(BonusList), "Luxury District Creation")
#			pCity.setBuildingBonusChange(eResourceClass,BonusList[iRnd3],1)

				
		if iBuildingType == gc.getInfoTypeForString('BUILDING_HOUSE_CANNITH_ALCHEMY_LAB'):
			eResourceClass=gc.getInfoTypeForString('BUILDINGCLASS_HOUSE_CANNITH_ALCHEMY_LAB')
			pTeam=gc.getTeam(pCity.getTeam())
			BonusList=[]
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_AIR'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_FIRE'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_ICE'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_LIFE'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_NATURE'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_MIND'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_CHAOS'))
			BonusList.append(gc.getInfoTypeForString('BONUS_MANA_DEATH'))

			iRnd = CyGame().getSorenRandNum(len(BonusList), "Wizard Tower")
			pCity.setBuildingBonusChange(eResourceClass,BonusList[iRnd],1)

		if iBuildingType == gc.getInfoTypeForString('BUILDING_VACANT_MAUSOLEUM'):
			if(pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS')):
				for pyCity in PyPlayer(player).getCityList():
					pCity = pyCity.GetCy()
					pCity.changePopulation(1)

		CvAdvisorUtils.buildingBuiltFeats(pCity, iBuildingType)

		if (not self.__LOG_BUILDING):
			return
		CvUtil.pyPrint('%s was finished by Player %d Civilization %s'
			%(PyInfo.BuildingInfo(iBuildingType).getDescription(), pCity.getOwner(), gc.getPlayer(pCity.getOwner()).getCivilizationDescription(0)))

	def onProjectBuilt(self, argsList):
		'Project Completed'
		pCity, iProjectType = argsList
		game = gc.getGame()
		iPlayer = pCity.getOwner()
		pPlayer = gc.getPlayer(iPlayer)
		if ((not gc.getGame().isNetworkMultiPlayer()) and (pCity.getOwner() == gc.getGame().getActivePlayer())):
			popupInfo = CyPopupInfo()
			popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
			popupInfo.setData1(iProjectType)
			popupInfo.setData2(pCity.getID())
			popupInfo.setData3(2)
			popupInfo.setText(u"showWonderMovie")
			popupInfo.addPopup(iPlayer)

		if iProjectType == gc.getInfoTypeForString('PROJECT_BANE_DIVINE'):
			iCombatDisciple = gc.getInfoTypeForString('UNITCOMBAT_DISCIPLE')
			for iLoopPlayer in range(gc.getMAX_PLAYERS()):
				pLoopPlayer = gc.getPlayer(iLoopPlayer)
				if pLoopPlayer.isAlive() :
					apUnitList = PyPlayer(iLoopPlayer).getUnitList()
					for pUnit in apUnitList:
						if pUnit.getUnitCombatType() == iCombatDisciple:
							pUnit.kill(False, pCity.getOwner())

		if iProjectType == gc.getInfoTypeForString('PROJECT_GENESIS'):
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
				cf.snowgenesis(iPlayer)
			else:
				cf.genesis(iPlayer)


		if iProjectType == gc.getInfoTypeForString('PROJECT_GLORY_EVERLASTING'):
			iDemon = gc.getInfoTypeForString('PROMOTION_DEMON')
			iUndead = gc.getInfoTypeForString('PROMOTION_UNDEAD')
			for iLoopPlayer in range(gc.getMAX_PLAYERS()):
				pLoopPlayer = gc.getPlayer(iLoopPlayer)
				player = PyPlayer(iLoopPlayer)
				if pLoopPlayer.isAlive():
					apUnitList = player.getUnitList()
					for pUnit in apUnitList:
						if (pUnit.isHasPromotion(iDemon) or pUnit.isHasPromotion(iUndead)):
							pUnit.kill(False, iPlayer)

		if iProjectType == gc.getInfoTypeForString('PROJECT_RITES_OF_OGHMA'):
			i = 7
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_DUEL'):
				i = i - 3
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_TINY'):
				i = i - 2
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_SMALL'):
				i = i - 1
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_LARGE'):
				i = i + 1
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_HUGE'):
				i = i + 3
			cf.addBonus('BONUS_MANA',i,'Art/Interface/Buttons/WorldBuilder/mana_button.dds')

		if iProjectType == gc.getInfoTypeForString('PROJECT_TWILIGHT_GENESIS'):
			i = 7
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_DUEL'):
				i = i - 3
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_TINY'):
				i = i - 2
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_SMALL'):
				i = i - 1
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_LARGE'):
				i = i + 1
			if CyMap().getWorldSize() == gc.getInfoTypeForString('WORLDSIZE_HUGE'):
				i = i + 3
			cf.addBonus('BONUS_RAZORWEED',i,'Art/Interface/Buttons/TechTree/Poisons.dds')

		if iProjectType == gc.getInfoTypeForString('PROJECT_BLOOD_OF_THE_PHOENIX'):
			py = PyPlayer(iPlayer)
			apUnitList = py.getUnitList()
			for pUnit in apUnitList:
				if pUnit.isAlive():
					pUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_IMMORTAL'), True)

		if iProjectType == gc.getInfoTypeForString('PROJECT_BIRTHRIGHT_REGAINED'):
			pPlayer.setFeatAccomplished(FeatTypes.FEAT_GLOBAL_SPELL, False)

#		if iProjectType == gc.getInfoTypeForString('PROJECT_STIR_FROM_SLUMBER'):
#			pPlayer.initUnit(gc.getInfoTypeForString('UNIT_DRIFA'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

#		if iProjectType == gc.getInfoTypeForString('PROJECT_THE_DRAW'):
#			pPlayer.changeNoDiplomacyWithEnemies(1)
#			iTeam = pPlayer.getTeam()
#			eTeam = gc.getTeam(iTeam)
#			for iLoopTeam in range(gc.getMAX_TEAMS()):
#				if iLoopTeam != iTeam:
#					if iLoopTeam != gc.getPlayer(gc.getBARBARIAN_PLAYER()).getTeam():
#						eLoopTeam = gc.getTeam(iLoopTeam)
#						if eLoopTeam.isAlive():
#								eTeam.declareWar(iLoopTeam, false, WarPlanTypes.WARPLAN_LIMITED)
#			py = PyPlayer(iPlayer)
#			for pUnit in py.getUnitList():
#				iDmg = pUnit.getDamage() * 2
#				if iDmg > 99:
#					iDmg = 99
#				if iDmg < 50:
#					iDmg = 50
#				pUnit.setDamage(iDmg, iPlayer)
#			for pyCity in PyPlayer(iPlayer).getCityList():
#				pLoopCity = pyCity.GetCy()
#				iPop = int(pLoopCity.getPopulation() / 2)
#				if iPop < 1:
#					iPop = 1
#				pLoopCity.setPopulation(iPop)

#Ascension Ritual moved to Frozen / CvFFHPLUS.py

#Faeries
		if iProjectType == gc.getInfoTypeForString('PROJECT_RETURN_TO_GLORY'):
			pPlayer.initUnit(gc.getInfoTypeForString('UNIT_LARINCSIDHE'), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
		if iProjectType == gc.getInfoTypeForString('PROJECT_ETERNAL_RESTORATION'):
			pPlayer.setHasTrait(gc.getInfoTypeForString('TRAIT_FEY'),True)

#Faeries

		if iProjectType == gc.getInfoTypeForString('PROJECT_LOCAL_LEGEND_RISING'):
			iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_LOCAL_LEGEND')
			pPlayer.trigger(iEvent)


	def onSelectionGroupPushMission(self, argsList):
		'selection group mission'
		eOwner = argsList[0]
		eMission = argsList[1]
		iNumUnits = argsList[2]
		listUnitIds = argsList[3]

		if (not self.__LOG_PUSH_MISSION):
			return
		if pHeadUnit:
			CvUtil.pyPrint("Selection Group pushed mission %d" %(eMission))

	def onUnitMove(self, argsList):
		'unit move'
		pPlot,pUnit,pOldPlot = argsList
		player = PyPlayer(pUnit.getOwner())
		unitInfo = PyInfo.UnitInfo(pUnit.getUnitType())
		if (not self.__LOG_MOVEMENT):
			return
		if player and unitInfo:
			CvUtil.pyPrint('Player %d Civilization %s unit %s is moving to %d, %d'
				%(player.getID(), player.getCivilizationName(), unitInfo.getDescription(),
				pUnit.getX(), pUnit.getY()))

	def onUnitSetXY(self, argsList):
		'units xy coords set manually'
		pPlot,pUnit = argsList
		player = PyPlayer(pUnit.getOwner())
		unitInfo = PyInfo.UnitInfo(pUnit.getUnitType())
		if (not self.__LOG_MOVEMENT):
			return

	def onUnitCreated(self, argsList):
		'Unit Completed'
		unit = argsList[0]
		player = PyPlayer(unit.getOwner())
		pPlayer = gc.getPlayer(unit.getOwner())

		iRegular = gc.getInfoTypeForString('PROMOTION_COMBAT1')
		iAdept = gc.getInfoTypeForString('UNITCOMBAT_ADEPT')
		iGolem = gc.getInfoTypeForString('PROMOTION_GOLEM')
		iSummon = gc.getInfoTypeForString('PROMOTION_SUMMON')
		#temporary - every unit Starts with the Regular Promotion
		
		if unit.getLevel()==1:
			if not unit.getUnitCombatType()==iAdept:
				unit.setHasPromotion(iRegular, True)
		
		#Adventurer Origin
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_ADVENTURER'):
			iRace = CyGame().getSorenRandNum(15, "Adventurer")
			if iRace == 0:
				iSubRace = CyGame().getSorenRandNum(10, "Adventurer")
				if iSubRace < 8:
					unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORC'), True)
				else:
					unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_GOBLIN'), True)
			elif iRace == 1:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DWARF'), True)
			elif iRace == 2:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ELF'), True)
			elif iRace == 3:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DARK_ELF'), True)
			elif iRace == 4:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_AMURITES'), True)
			elif iRace == 5:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_BALSERAPHS'), True)
			elif iRace == 6:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_BANNOR'), True)
			elif iRace == 7:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_DOVIELLO'), True)
			elif iRace == 8:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_ELOHIM'), True)
			elif iRace == 9:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_MALAKIM'), True)
			elif iRace == 10:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_AUSTRIN'), True)
			elif iRace == 11:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_SIDAR'), True)
			else:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORIGIN_GRIGORI'), True)
				
		#Craftmaster Trait
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_CRAFTMASTER')):
			if unit.isHasPromotion(iGolem):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), True)
				# BlackArchon: Combat I doesn't grant +10 percent strength anymore, so we give them Combat II
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_AGGRESSIVE'), True)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SUNDERED')):
			if unit.isHasPromotion(iSummon):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STIGMATA'), True)

#Xtended - Illusion Trait
		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_ILLUSIONIST')):
			if unit.isHasPromotion(iSummon):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ILLUSION'), True)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ANGEL'), False)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), False)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DRAGON'), False)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ELEMENTAL'), False)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FROSTLING'), False)
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_UNDEAD'), False)


		
		
		if (not self.__LOG_UNITBUILD):
			return

	def onUnitBuilt(self, argsList):
		'Unit Completed'
		city = argsList[0]
		unit = argsList[1]
		player = PyPlayer(city.getOwner())
		pPlayer = gc.getPlayer(unit.getOwner())

		iFreeProm = unit.getFreePromotionPick()

#Sephi

		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_BEAST_OF_AGARES'):
			if city.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
				iPop = city.getPopulation() - 4
				if iPop <= 1:
					iPop = 1
				city.setPopulation(iPop)
				city.setOccupationTimer(4)

		if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_CHANCEL_OF_GUARDIANS')) > 0:
			if CyGame().getSorenRandNum(100, "Chancel of Guardians") < 20:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEFENSIVE'), True)

		if (city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_CAVE_OF_ANCESTORS')) > 0 and unit.getUnitCombatType() == (gc.getInfoTypeForString('UNITCOMBAT_ADEPT'))):
			i = 0
			for iBonus in range(gc.getNumBonusInfos()):
				if gc.getBonusInfo(iBonus).getBonusClassType() == gc.getInfoTypeForString('BONUSCLASS_MANA'):
					if city.hasBonus(iBonus):
						i = i + 1
			if i >= 1:
				unit.changeExperience(2*i, -1, False, False, False)

		if (city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_ESUS_TRAINING')) > 0 and unit.getUnitCombatType() == (gc.getInfoTypeForString('UNITCOMBAT_THIEF'))):
			i = city.getProductionExperience(unit.getUnitType())
			if i >= 1:
				unit.changeExperience(i, -1, False, False, False)

		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_GOLEM')):
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALLENS_ENGINE')) > 0:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_PERFECT_SIGHT'), True)
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_BLASTING_WORKSHOP')) > 0:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FIRE_EMULATION'), True)
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_ADULARIA_CHAMBER')) > 0:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FAST_SIEGE'), True)
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_GEOMANCERS_STATION')) > 0:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MASTER_CRAFTED_GOLEM_COMPONENTS'), True)
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_HYDROFACTORY')) > 0:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENCHANTED_OIL'), True)
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_INFERNAL_MACHINE')) > 0:
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMONIC_INFUSION'), True)

		if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_ASYLUM')) > 0:
			if unit.isAlive():
				if isWorldUnitClass(unit.getUnitClassType()) == False:
					if CyGame().getSorenRandNum(100, "Bob") <= 10:
						unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CRAZED'), True)
						unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ENRAGED'), True)
						unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STRONG'), True)

		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DWARF')):
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_BREWERY')) > 0:
				unit.changeExperience(10, -1, False, False, False)

		if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON')):
			if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_DEMONS_ALTAR')) > 0:
				unit.changeExperience(10, -1, False, False, False)

		if unit.getFreePromotionPick() < iFreeProm:
			unit.changeFreePromotionPick(iFreeProm - unit.getFreePromotionPick())

		if city.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_WARRENS')) > 0:
			if isWorldUnitClass(unit.getUnitClassType()) == False:
				if isNationalUnitClass(unit.getUnitClassType()) == False:
					if unit.getUnitCombatType() != gc.getInfoTypeForString('UNITCOMBAT_SIEGE'):
						if unit.getUnitCombatType() != gc.getInfoTypeForString('UNITCOMBAT_NAVAL'):
							newUnit = pPlayer.initUnit(unit.getUnitType(), city.getX(), city.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
							city.applyBuildEffects(newUnit)

		#Xtended - Malakim
		iOrder = gc.getInfoTypeForString('RELIGION_THE_ORDER')
		iEmp = gc.getInfoTypeForString('RELIGION_THE_EMPYREAN')
		iRunes = gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH')
		iFOL = gc.getInfoTypeForString('RELIGION_FELLOWSHIP_OF_LEAVES')
		iOO = gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS')
		iVeil = gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL')
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_DERVISH'):
			if pPlayer.getStateReligion() == iOrder and city.isHasReligion(iOrder):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FOLLOWER_OF_ORDER'), True)
			elif pPlayer.getStateReligion() == iEmp and city.isHasReligion(iEmp):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FOLLOWER_OF_EMPYREAN'), True)
			elif pPlayer.getStateReligion() == iRunes and city.isHasReligion(iRunes):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FOLLOWER_OF_KILMORPH'), True)
			elif pPlayer.getStateReligion() == iFOL and city.isHasReligion(iFOL):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FOLLOWER_OF_FOL'), True)
			elif pPlayer.getStateReligion() == iOO and city.isHasReligion(iOO):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FOLLOWER_OF_OO'), True)
			elif pPlayer.getStateReligion() == iVeil and city.isHasReligion(iVeil):
				unit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_FOLLOWER_OF_AV'), True)

		CvAdvisorUtils.unitBuiltFeats(city, unit)
#WU POPUP: add begin
		if not gc.getGame().isGameMultiPlayer():
			cf.processWU_POPUP(unit, player)
#WU POPUP: add end

		if (not self.__LOG_UNITBUILD):
			return
		CvUtil.pyPrint('%s was finished by Player %d Civilization %s'
			%(PyInfo.UnitInfo(unit.getUnitType()).getDescription(), player.getID(), player.getCivilizationName()))

	def onUnitKilled(self, argsList):
		'Unit Killed'
		unit, iAttacker = argsList
		iPlayer = unit.getOwner()
		player = PyPlayer(iPlayer)
		attacker = PyPlayer(iAttacker)
		pPlayer = gc.getPlayer(iPlayer)

		if (unit.isAlive() and unit.isImmortal() == False):
			iX = unit.getX()
			iY = unit.getY()
			pPlot = CyMap().plot(iX,iY)
			if pPlayer.getCivilizationType() != gc.getInfoTypeForString('CIVILIZATION_GRIGORI'):
				if (unit.getReligion() == gc.getInfoTypeForString('RELIGION_THE_EMPYREAN') or unit.getReligion() == gc.getInfoTypeForString('RELIGION_THE_ORDER') or unit.getReligion() == gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH') or (unit.getUnitCombatType() != gc.getInfoTypeForString('UNITCOMBAT_ANIMAL') and pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_MERCURIANS'))):
					cf.giftUnit(gc.getInfoTypeForString('UNIT_ANGEL'), gc.getInfoTypeForString('CIVILIZATION_MERCURIANS'), unit.getExperience(), unit.plot(), unit.getOwner())
				elif (unit.getReligion() == gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL') or unit.getReligion() == gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS')):
					cf.giftUnit(gc.getInfoTypeForString('UNIT_MANES'), gc.getInfoTypeForString('CIVILIZATION_INFERNAL'), 0, unit.plot(), unit.getOwner())
#					cf.giftUnit(gc.getInfoTypeForString('UNIT_MANES'), gc.getInfoTypeForString('CIVILIZATION_INFERNAL'), 0, unit.plot(), unit.getOwner())

#Xtended
			if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_MARK_OF_SERVITUDE')):
				if unit.isAlive():
					newUnit = pPlayer.initUnit(gc.getInfoTypeForString('UNIT_ZOMBIED'), pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
			
			if unit.isHasPromotion(gc.getInfoTypeForString('PROMOTION_SPIRIT_GUIDE')):
				if unit.getExperience() > 0:
					py = PyPlayer(iPlayer)
					lUnits = []
					for pLoopUnit in py.getUnitList():
						if pLoopUnit.isAlive():
							if not pLoopUnit.isOnlyDefensive():
								if not pLoopUnit.isDelayedDeath():
									lUnits.append(pLoopUnit)
					if len(lUnits) > 0:
						pUnit = lUnits[CyGame().getSorenRandNum(len(lUnits), "Spirit Guide")-1]
						iXP = unit.getExperience() / 2
						pUnit.changeExperience(iXP, -1, false, false, false)
						unit.changeExperience(iXP * -1, -1, false, false, false)
						CyInterface().addMessage(unit.getOwner(),True,25,CyTranslator().getText("TXT_KEY_MESSAGE_SPIRIT_GUIDE",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Promotions/SpiritGuide.dds',ColorTypes(7),pUnit.getX(),pUnit.getY(),True,True)

			# more events mod starts #
		if unit.getUnitType() == gc.getInfoTypeForString('UNIT_GOBLIN') or unit.getUnitType() == gc.getInfoTypeForString('UNIT_GOBLIN_SCORPION_CLAN'):
			if CyGame().getSorenRandNum(10000, "Goblin2")<10:
				if iPlayer == gc.getBARBARIAN_PLAYER() :
					iX = unit.getX()
					iY = unit.getY()
					int=1
					pAttacker = gc.getPlayer(iAttacker)
					for iiX in range(iX-2, iX+2, 1):
						for iiY in range(iY-2, iY+2, 1):
							pPlot2 = CyMap().plot(iiX,iiY)
							for i in range(pPlot2.getNumUnits()):
								pUnit2 = pPlot2.getUnit(i)
								if pUnit2.getOwner()== iAttacker:
									iWorker = gc.getInfoTypeForString('UNITCLASS_WORKER')
									iSettler = gc.getInfoTypeForString('UNITCLASS_SETTLER')
									if not pUnit2.getUnitClassType() == iWorker and not pUnit2.getUnitClassType() == iSettler and int==1 :
										iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_ORPHANED_GOBLIN')
										triggerData = pAttacker.initTriggeredData(iEvent, true, -1, pUnit2.getX(), pUnit2.getY(), pUnit2.getOwner(), -1, -1, -1, pUnit2.getID(), -1)
										int=0
					# more events mod ends #

		if (not self.__LOG_UNITKILLED):
			return
		CvUtil.pyPrint('Player %d Civilization %s Unit %s was killed by Player %d'
			%(player.getID(), player.getCivilizationName(), PyInfo.UnitInfo(unit.getUnitType()).getDescription(), attacker.getID()))

	def onUnitLost(self, argsList):
		'Unit Lost'
		unit = argsList[0]
		player = PyPlayer(unit.getOwner())
		if (not self.__LOG_UNITLOST):
			return
		CvUtil.pyPrint('%s was lost by Player %d Civilization %s'
			%(PyInfo.UnitInfo(unit.getUnitType()).getDescription(), player.getID(), player.getCivilizationName()))

	def onUnitPromoted(self, argsList):
		'Unit Promoted'
		pUnit, iPromotion = argsList
		player = PyPlayer(pUnit.getOwner())
		if (not self.__LOG_UNITPROMOTED):
			return
		CvUtil.pyPrint('Unit Promotion Event: %s - %s' %(player.getCivilizationName(), pUnit.getName(),))
							# More Events mod starts  #
		if player.isHuman():
			iGela=gc.getInfoTypeForString('PROMOTION_GELA')
			if iPromotion ==iGela :
				if (not player.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNALS')):
					iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_GELA')
					triggerData = player.initTriggeredData(iEvent, true, -1, -1, -1, pUnit.getOwner(), -1, -1, -1, -1, -1)
		# More Events mod ends #

	def onUnitSelected(self, argsList):
		'Unit Selected'
		unit = argsList[0]
		player = PyPlayer(unit.getOwner())
		if (not self.__LOG_UNITSELECTED):
			return
		CvUtil.pyPrint('%s was selected by Player %d Civilization %s'
			%(PyInfo.UnitInfo(unit.getUnitType()).getDescription(), player.getID(), player.getCivilizationName()))

	def onUnitRename(self, argsList):
		'Unit is renamed'
		pUnit = argsList[0]
		if (pUnit.getOwner() == gc.getGame().getActivePlayer()):
			self.__eventEditUnitNameBegin(pUnit)

	def onUnitPillage(self, argsList):
		'Unit pillages a plot'
		pUnit, iImprovement, iRoute, iOwner = argsList
		iPlotX = pUnit.getX()
		iPlotY = pUnit.getY()
		pPlot = CyMap().plot(iPlotX, iPlotY)
		pPlayer = gc.getPlayer(pUnit.getOwner())
		iRaider = gc.getInfoTypeForString('PROMOTION_RAIDER1')
		if pPlot.getTeam()!=-1:
			if pPlot.getTeam()!=pUnit.getTeam():
				if not pUnit.isHasPromotion(iRaider):
					if CyGame().getSorenRandNum(100, "Raider Bob") <= 10:
						pUnit.setHasPromotion(iRaider,true)
						pUnit.displayMessagePromotionGainedInBattle(iRaider)

		if (not self.__LOG_UNITPILLAGE):
			return
		CvUtil.pyPrint("Player %d's %s pillaged improvement %d and route %d at plot at (%d, %d)"
			%(iOwner, PyInfo.UnitInfo(pUnit.getUnitType()).getDescription(), iImprovement, iRoute, iPlotX, iPlotY))

	def onUnitSpreadReligionAttempt(self, argsList):
		'Unit tries to spread religion to a city'
		pUnit, iReligion, bSuccess = argsList

		iX = pUnit.getX()
		iY = pUnit.getY()
		pPlot = CyMap().plot(iX, iY)
		pCity = pPlot.getPlotCity()

	def onUnitGifted(self, argsList):
		'Unit is gifted from one player to another'
		pUnit, iGiftingPlayer, pPlotLocation = argsList

	def onUnitBuildImprovement(self, argsList):
		'Unit begins enacting a Build (building an Improvement or Route)'
		pUnit, iBuild, bFinished = argsList

	def onGoodyReceived(self, argsList):
		'Goody received'
		iPlayer, pPlot, pUnit, iGoodyType = argsList
		if (not self.__LOG_GOODYRECEIVED):
			return
		CvUtil.pyPrint('%s received a goody' %(gc.getPlayer(iPlayer).getCivilizationDescription(0)),)

	def onGreatPersonBorn(self, argsList):
		'Unit Promoted'
		pUnit, iPlayer, pCity = argsList
		player = PyPlayer(iPlayer)
		pPlayer = gc.getPlayer(iPlayer)

		if pUnit.isNone() or pCity.isNone():
			return

#GREAT PERSON MOD START
		if not gc.getGame().isGameMultiPlayer():
			if 50 > CyGame().getSorenRandNum(100, "GP Popup"):
				if gc.getPlayer(iPlayer).isHuman():
					if not (str(pUnit.getNameNoDesc()) == ""):
						Message = "TXT_KEY_QUOTE_%s" %(pUnit.getNameNoDesc(),)
						cf.addPopup(CyTranslator().getText(str(Message),()), 'Art/GreatPeople/Simple/'+str(gc.getUnitInfo(pUnit.getUnitType()).getType())+'/'+str(pUnit.getNameNoDesc())+'.dds')
					else:
						if pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_ADVENTURER') or pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_ARTIST') or pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_COMMANDER') or pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_ENGINEER') or pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_COMMANDER') or pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_MERCHANT') or pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_PROPHET') or pUnit.getUnitType() == gc.getInfoTypeForString('UNIT_SCIENTIST'):
							Message = "TXT_KEY_QUOTE_%s" %(gc.getUnitInfo(pUnit.getUnitType()).getType(),)
							cf.addPopup(CyTranslator().getText(str(Message),()), 'Art/GreatPeople/Simple/'+str(gc.getUnitInfo(pUnit.getUnitType()).getType())+'/'+str(gc.getUnitInfo(pUnit.getUnitType()).getType())+'.dds')
					return
#GREAT PERSON MOD END
		if (not self.__LOG_GREATPERSON):
			return
		CvUtil.pyPrint('A %s was born for %s in %s' %(pUnit.getName(), player.getCivilizationName(), pCity.getName()))

	def onTechAcquired(self, argsList):
		'Tech Acquired'
		iTechType, iTeam, iPlayer, bAnnounce = argsList
		# Note that iPlayer may be NULL (-1) and not a refer to a player object
		pPlayer = gc.getPlayer(iPlayer)
		pTeam = gc.getTeam(pPlayer.getTeam())
		# Show tech splash when applicable
		if (iPlayer > -1 and bAnnounce and not CyInterface().noTechSplash()):
			if (gc.getGame().isFinalInitialized() and not gc.getGame().GetWorldBuilderMode()):
				if ((not gc.getGame().isNetworkMultiPlayer()) and (iPlayer == gc.getGame().getActivePlayer())):
					popupInfo = CyPopupInfo()
					popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
					popupInfo.setData1(iTechType)
					popupInfo.setText(u"showTechSplash")
					popupInfo.addPopup(iPlayer)

		#Knowledge of the Ether
		iGameTurn = CyGame().getElapsedGameTurns()
		iGameSpeed = 3
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
			iGamespeed=2
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
			iGamespeed=6
		if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
			iGamespeed=9

		if iTechType == gc.getInfoTypeForString('TECH_KNOWLEDGE_OF_THE_ARCANE'):
			iArcane = 1500 - (iGameTurn * 30) / iGameSpeed
			if iArcane > 0:
				pPlayer.changeArcane(iArcane)
				if pPlayer.getNumCities() > 0:
					CyInterface().addMessage(iPlayer, True, 25, CyTranslator().getText("TXT_KEY_MESSAGE_RESEARCHED_KOTE_EARLY",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Techtree/elementalism.dds', ColorTypes(8), pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), True, True)

		if iTechType == gc.getInfoTypeForString('TECH_MYSTICISM'):
#			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FAITHFUL')) or pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FAITHFUL_ADAPTIVE')):
#				if not pPlayer.isAgnostic():
#					if pPlayer.getNumCities() > 0:
#						iUnit = gc.getInfoTypeForString('UNIT_PROPHET')
#						if iUnit != -1:
#							pPlayer.createGreatPeople(iUnit, false, false, pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY())
#							CyInterface().addMessage(iPlayer, True, 25, CyTranslator().getText("TXT_KEY_MESSAGE_RESEARCHED_MYSTICISM_EARLY",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Techtree/Divination.dds', ColorTypes(8), pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), True, True)

			#Xtended Dural
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_DURAL'):
				eTeam = gc.getTeam(pPlayer.getTeam())
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_ORDERS_FROM_HEAVEN'),true,-1,false,false)
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_HONOR'),true,-1,false,false)
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_WAY_OF_THE_EARTHMOTHER'),true,-1,false,false)
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_WAY_OF_THE_FORESTS'),true,-1,false,false)
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_MESSAGE_FROM_THE_DEEP'),true,-1,false,false)
				eTeam.setHasTech(gc.getInfoTypeForString('TECH_CORRUPTION_OF_SPIRIT'),true,-1,false,false)

							
		if (iPlayer != -1 and iPlayer != gc.getBARBARIAN_PLAYER()):
			pPlayer = gc.getPlayer(iPlayer)

#Sephi
#Go into Conquest Mode
			if iTechType == gc.getInfoTypeForString('TECH_WARRIOR_CASTE'):
				pPlot = pPlayer.getCapitalCity().plot()
				if pPlot:
					iUnitWarriorClass = gc.getInfoTypeForString('UNITCLASS_CIV_DEFAULT')
#					if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ARISTARKH'):
#						iUnitWarriorClass = gc.getInfoTypeForString('UNITCLASS_CIV_DEFAULT')
					iUnitWarrior = gc.getCivilizationInfo(pPlayer.getCivilizationType()).getCivilizationUnits(iUnitWarriorClass)
					if not iUnitWarrior == -1:
						for i in range(2):
							newUnit = pPlayer.initUnit(iUnitWarrior, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
							newUnit.changeExperience(20,-1,false,false,false)
						
			iReligion = -1
			if iTechType == gc.getInfoTypeForString('TECH_CORRUPTION_OF_SPIRIT'):
				iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_THE_ASHEN_VEIL')
				iReligion = gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL')
			if iTechType == gc.getInfoTypeForString('TECH_ORDERS_FROM_HEAVEN'):
				iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_THE_ORDER')
				iReligion = gc.getInfoTypeForString('RELIGION_THE_ORDER')
			if iTechType == gc.getInfoTypeForString('TECH_WAY_OF_THE_FORESTS'):
				iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_FELLOWSHIP_OF_LEAVES')
				iReligion = gc.getInfoTypeForString('RELIGION_FELLOWSHIP_OF_LEAVES')
			if iTechType == gc.getInfoTypeForString('TECH_WAY_OF_THE_EARTHMOTHER'):
				iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_RUNES_OF_KILMORPH')
				iReligion = gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH')
			if iTechType == gc.getInfoTypeForString('TECH_MESSAGE_FROM_THE_DEEP'):
				iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_OCTOPUS_OVERLORDS')
				iReligion = gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS')
			if iTechType == gc.getInfoTypeForString('TECH_HONOR'):
				iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_EMPYREAN')
				iReligion = gc.getInfoTypeForString('RELIGION_THE_EMPYREAN')
			if iReligion != -1:
#				if (iReligion==pPlayer.getFavoriteReligion()):
				pPlayer.getCapitalCity().setHasReligion(iReligion,True,True,True)

#					pPlayer.initUnit(iUnit, pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
#				cf.giftUnit(iUnit, pPlayer.getCivilizationType(), 0, -1, -1)

				#Xtended
				if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FAITHFUL')) or pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_FAITHFUL_ADAPTIVE')):
					if not pPlayer.isAgnostic():
						if pPlayer.getNumCities() > 0:
							iUnit = gc.getInfoTypeForString('UNIT_PROPHET')
							if iUnit != -1:
								pPlayer.createGreatPeople(iUnit, false, false, pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY())
								CyInterface().addMessage(iPlayer, True, 25, CyTranslator().getText("TXT_KEY_MESSAGE_RESEARCHED_MYSTICISM_EARLY",()),'AS2D_DISCOVERBONUS',1,'Art/Interface/Buttons/Techtree/Divination.dds', ColorTypes(8), pPlayer.getCapitalCity().getX(), pPlayer.getCapitalCity().getY(), True, True)


		if 1==1:
			if (iTechType == gc.getInfoTypeForString('TECH_INFERNAL_PACT') and iPlayer != -1):
				iCount = 0
				for iTeam in range(gc.getMAX_TEAMS()):
					eTeam = gc.getTeam(iTeam)
					if eTeam.isHasTech(gc.getInfoTypeForString('TECH_INFERNAL_PACT')):
						iCount = iCount + 1
				if iCount == 1:
					iInfernalPlayer = cf.getOpenPlayer()
					pBestPlot = -1
					iBestPlot = -1
					for i in range (CyMap().numPlots()):
						pPlot = CyMap().plotByIndex(i)
						iPlot = -1
						if pPlot.isWater() == False:
							if pPlot.getNumUnits() == 0:
								if pPlot.isCity() == False:
									if pPlot.isImpassable() == False:
										iPlot = CyGame().getSorenRandNum(500, "Place Hyborem")
										iPlot = iPlot + (pPlot.area().getNumTiles() * 2)
										iPlot = iPlot + (pPlot.area().getNumUnownedTiles() * 10)
										if pPlot.isOwned() == False:
											iPlot = iPlot + 500
										if pPlot.getOwner() == iPlayer:
											iPlot = iPlot + 200
						if iPlot > iBestPlot:
							iBestPlot = iPlot
							pBestPlot = pPlot
					if (iInfernalPlayer != -1 and pBestPlot != -1):
						CyGame().addPlayerAdvanced(iInfernalPlayer, -1, gc.getInfoTypeForString('LEADER_HYBOREM'), gc.getInfoTypeForString('CIVILIZATION_INFERNAL'))
						iFounderTeam = gc.getPlayer(iPlayer).getTeam()
						eFounderTeam = gc.getTeam(gc.getPlayer(iPlayer).getTeam())
						iInfernalTeam = gc.getPlayer(iInfernalPlayer).getTeam()
						eInfernalTeam = gc.getTeam(iInfernalTeam)
						for iTech in range(gc.getNumTechInfos()):
							if eFounderTeam.isHasTech(iTech):
								eInfernalTeam.setHasTech(iTech, true, iInfernalPlayer, true, false)
						eFounderTeam.signOpenBorders(iInfernalTeam)
						eInfernalTeam.signOpenBorders(iFounderTeam)
						pInfernalPlayer = gc.getPlayer(iInfernalPlayer)
						pInfernalPlayer.AI_changeAttitudeExtra(iPlayer,4)

						eInfernalTeam.makePeace(gc.getPlayer(gc.getBARBARIAN_PLAYER()).getTeam())
						eInfernalTeam.makePeace(gc.getPlayer(gc.getANIMAL_PLAYER()).getTeam())
						eInfernalTeam.declareWar(gc.getPlayer(gc.getWILDMANA_PLAYER()).getTeam(), false, WarPlanTypes.WARPLAN_LIMITED)
						eInfernalTeam.declareWar(gc.getPlayer(gc.getPIRATES_PLAYER()).getTeam(), false, WarPlanTypes.WARPLAN_LIMITED)

						for iTeam in range(gc.getMAX_CIV_TEAMS()):
							if iTeam != iInfernalTeam:
								eTeam = gc.getTeam(iTeam)
								if eTeam.isAlive():
									if eFounderTeam.isAtWar(iTeam):
										eInfernalTeam.declareWar(iTeam, false, WarPlanTypes.WARPLAN_LIMITED)
						newUnit1 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_HYBOREM'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit1.setHasPromotion(gc.getInfoTypeForString('PROMOTION_IMMORTAL'), true)
						newUnit1.setHasCasted(true)
						newUnit2 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_HORNED_DEMON'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
						newUnit3 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_HORNED_DEMON'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit3.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
						newUnit4 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_BALOR'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit4.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
						newUnit5 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_BALOR'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit5.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
						newUnit6 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_WORKER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit6.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), true)
						newUnit7 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_IMP'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit7.setHasPromotion(gc.getInfoTypeForString('PROMOTION_MOBILITY1'), true)
						newUnit8 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit9 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit10 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit11 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_SETTLER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit11.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), true)
						newUnit12 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_SETTLER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit12.setHasPromotion(gc.getInfoTypeForString('PROMOTION_DEMON'), true)
						newUnit13 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_RAM_DEMON'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit13.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CITY_GARRISON1'), true)
						newUnit14 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_RAM_DEMON'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit14.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CITY_GARRISON1'), true)
						newUnit15 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_RAM_DEMON'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit15.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CITY_GARRISON1'), true)
						newUnit16 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_RAM_DEMON'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit16.setHasPromotion(gc.getInfoTypeForString('PROMOTION_CITY_GARRISON1'), true)
						newUnit17 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_SOUL_HUNTER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit18 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_SOUL_HUNTER'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit19 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_BEAST_OF_AGARES'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit20 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit21 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						newUnit22 = pInfernalPlayer.initUnit(gc.getInfoTypeForString('UNIT_MANES'), pBestPlot.getX(), pBestPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
						if gc.getPlayer(iPlayer).isHuman():
							popupInfo = CyPopupInfo()
							popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON)
							popupInfo.setText(CyTranslator().getText("TXT_KEY_POPUP_CONTROL_INFERNAL",()))
							popupInfo.setData1(iPlayer)
							popupInfo.setData2(iInfernalPlayer)
							popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_YES", ()), "")
							popupInfo.addPythonButton(CyTranslator().getText("TXT_KEY_POPUP_NO", ()), "")
							popupInfo.setOnClickedPythonCallback("reassignPlayer")
							popupInfo.addPopup(iPlayer)



		if (not self.__LOG_TECH):
			return
		CvUtil.pyPrint('%s was finished by Team %d'
			%(PyInfo.TechnologyInfo(iTechType).getDescription(), iTeam))

	def onTechSelected(self, argsList):
		'Tech Selected'
		iTechType, iPlayer = argsList
		if (not self.__LOG_TECH):
			return
		CvUtil.pyPrint('%s was selected by Player %d' %(PyInfo.TechnologyInfo(iTechType).getDescription(), iPlayer))

	def onReligionFounded(self, argsList):
		'Religion Founded'
		iReligion, iFounder = argsList
		player = PyPlayer(iFounder)
		pPlayer = gc.getPlayer(iFounder)

		iCityId = gc.getGame().getHolyCity(iReligion).getID()
		if (gc.getGame().isFinalInitialized() and not gc.getGame().GetWorldBuilderMode()):
			if ((not gc.getGame().isNetworkMultiPlayer()) and (iFounder == gc.getGame().getActivePlayer())):
				popupInfo = CyPopupInfo()
				popupInfo.setButtonPopupType(ButtonPopupTypes.BUTTONPOPUP_PYTHON_SCREEN)
				popupInfo.setData1(iReligion)
				popupInfo.setData2(iCityId)
				if (iReligion == gc.getInfoTypeForString('RELIGION_THE_EMPYREAN')):
					popupInfo.setData3(3)
				else:
					popupInfo.setData3(1)
				popupInfo.setText(u"showWonderMovie")
				popupInfo.addPopup(iFounder)

		if (not self.__LOG_RELIGION):
			return
		CvUtil.pyPrint('Player %d Civilization %s has founded %s'
			%(iFounder, player.getCivilizationName(), gc.getReligionInfo(iReligion).getDescription()))

	def onReligionSpread(self, argsList):
		'Religion Has Spread to a City'
		iReligion, iOwner, pSpreadCity = argsList
		player = PyPlayer(iOwner)
		iOrder = gc.getInfoTypeForString('RELIGION_THE_ORDER')
		iEmp = gc.getInfoTypeForString('RELIGION_THE_EMPYREAN')
		iRunes = gc.getInfoTypeForString('RELIGION_RUNES_OF_KILMORPH')
		iFOL = gc.getInfoTypeForString('RELIGION_FELLOWSHIP_OF_LEAVES')
		iOverlords = gc.getInfoTypeForString('RELIGION_OCTOPUS_OVERLORDS')
		iVeil = gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL')
		pPlayer = gc.getPlayer(iOwner)

		if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_ZEALOUS')):
			if iReligion == iOrder and CyGame().getGameTurn() != CyGame().getStartTurn():
				if (pPlayer.getStateReligion() == iOrder and pSpreadCity.getOccupationTimer() <= 0):
					eTeam = gc.getTeam(pPlayer.getTeam())
					if eTeam.isHasTech(gc.getInfoTypeForString('TECH_ESSENCE_OF_LAW')):
						iUnit = gc.getInfoTypeForString('UNIT_CRUSADER')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_ORDER_SPAWN_CRUSADER",()),'AS2D_UNIT_BUILD_UNIT',1,', ,Art/Interface/Buttons/Units/Units_Atlas2.dds,5,4',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					else:
						iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_THE_ORDER')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_DISCIPLE",()),'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Units/UCDisciple.dds',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					newUnit = pPlayer.initUnit(iUnit, pSpreadCity.getX(), pSpreadCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			elif iReligion == iEmp and CyGame().getGameTurn() != CyGame().getStartTurn():
				if (pPlayer.getStateReligion() == iEmp and pSpreadCity.getOccupationTimer() <= 0):
					eTeam = gc.getTeam(pPlayer.getTeam())
					if eTeam.isHasTech(gc.getInfoTypeForString('TECH_VIRTUE_LUGUS')):
						iUnit = gc.getInfoTypeForString('UNIT_RADIANT_GUARD')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_RADIANT_GUARD",()),'AS2D_UNIT_BUILD_UNIT',1,', ,Art/Interface/Buttons/Units/Units_Atlas3.dds,5,16',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					else:
						iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_EMPYREAN')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_DISCIPLE",()),'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Units/UCDisciple.dds',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					newUnit = pPlayer.initUnit(iUnit, pSpreadCity.getX(), pSpreadCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			elif iReligion == iRunes and CyGame().getGameTurn() != CyGame().getStartTurn():
				if (pPlayer.getStateReligion() == iRunes and pSpreadCity.getOccupationTimer() <= 0):
					eTeam = gc.getTeam(pPlayer.getTeam())
					if eTeam.isHasTech(gc.getInfoTypeForString('TECH_DEDICATION')):
						iUnit = gc.getInfoTypeForString('UNIT_DWARVEN_SOLDIER_RUNES')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_KILMORPH",()),'AS2D_UNIT_BUILD_UNIT',1,', ,Art/Interface/Buttons/Units/Units_atlas4.dds,3,7',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					else:
						iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_RUNES_OF_KILMORPH')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_DISCIPLE",()),'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Units/UCDisciple.dds',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					newUnit = pPlayer.initUnit(iUnit, pSpreadCity.getX(), pSpreadCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			elif iReligion == iFOL and CyGame().getGameTurn() != CyGame().getStartTurn():
				if (pPlayer.getStateReligion() == iFOL and pSpreadCity.getOccupationTimer() <= 0):
					eTeam = gc.getTeam(pPlayer.getTeam())
					if eTeam.isHasTech(gc.getInfoTypeForString('TECH_SACRED_WOODS')):
						iUnit = gc.getInfoTypeForString('UNIT_FAWN')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_FAWN",()),'AS2D_UNIT_BUILD_UNIT',1,', ,Art/Interface/Buttons/Units/Units_Atlas2.dds,7,8',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					else:
						iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_FELLOWSHIP_OF_LEAVES')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_DISCIPLE",()),'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Units/UCDisciple.dds',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					newUnit = pPlayer.initUnit(iUnit, pSpreadCity.getX(), pSpreadCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			elif iReligion == iOverlords and CyGame().getGameTurn() != CyGame().getStartTurn():
				if (pPlayer.getStateReligion() == iOverlords and pSpreadCity.getOccupationTimer() <= 0):
					eTeam = gc.getTeam(pPlayer.getTeam())
					if eTeam.isHasTech(gc.getInfoTypeForString('TECH_NIGHTMARISH_EPIPHANY')):
						iUnit = gc.getInfoTypeForString('UNIT_PRIEST_OF_THE_OVERLORDS')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_CULTIST",()),'AS2D_UNIT_BUILD_UNIT',1,', ,Art/Interface/Buttons/Units/Units_Atlas3.dds,1,16',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					else:
						iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_OCTOPUS_OVERLORDS')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_DISCIPLE",()),'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Units/UCDisciple.dds',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					newUnit = pPlayer.initUnit(iUnit, pSpreadCity.getX(), pSpreadCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			elif iReligion == iVeil and CyGame().getGameTurn() != CyGame().getStartTurn():
				if (pPlayer.getStateReligion() == iVeil and pSpreadCity.getOccupationTimer() <= 0):
					eTeam = gc.getTeam(pPlayer.getTeam())
					if eTeam.isHasTech(gc.getInfoTypeForString('TECH_INFERNAL_PACT')):
						iUnit = gc.getInfoTypeForString('UNIT_AZER')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_AZER",()),'AS2D_UNIT_BUILD_UNIT',1,', ,Art/Interface/Buttons/Units/Units_Atlas1.dds,5,11',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					else:
						iUnit = gc.getInfoTypeForString('UNIT_DISCIPLE_THE_ASHEN_VEIL')
						CyInterface().addMessage(iOwner,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_MISSIONARY_SPAWN_DISCIPLE",()),'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Units/UCDisciple.dds',ColorTypes(8),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					newUnit = pPlayer.initUnit(iUnit, pSpreadCity.getX(), pSpreadCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)

		if pSpreadCity.isHasReligion(iVeil) and iReligion == iOrder:
			if not pSpreadCity.isHolyCity():
				result = CyGame().getSorenRandNum(100, "Order-Veil")
				if (result < 35):
					CyInterface().addMessage(iOwner,True,25,"A bloodbath ensues as the Order and Veil battle for control of the city! In the end, Ritualists lie dead in the streets. The Order is triumphant!",'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Religions/Order.dds',ColorTypes(14),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					pSpreadCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'), False, True, True)
					iPop = pSpreadCity.getPopulation()-1
					if (iPop > 0):
						pSpreadCity.setPopulation(iPop)
					CyGame().changeGlobalCounter(-1)
				else:
					CyInterface().addMessage(iOwner,True,25,"A bloodbath ensues as the Order and Veil battle for control of the city! In the end, the streets are coated in Crusader blood. The Veil is victorious!",'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Religions/Ashen.dds',ColorTypes(5),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					pSpreadCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ORDER'), False, True, True)
		elif pSpreadCity.isHasReligion(iOrder) and iReligion == iVeil:
			if not pSpreadCity.isHolyCity():
				result = CyGame().getSorenRandNum(100, "Veil-Order")
				if (result < 35):
					CyInterface().addMessage(iOwner,True,25,"A bloodbath ensues as the Order and Veil battle for control of the city! In the end, the streets are coated in Crusader blood. The Veil is victorious!",'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Religions/Ashen.dds',ColorTypes(5),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					pSpreadCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ORDER'), False, True, True)
					iPop = pSpreadCity.getPopulation()-1
					if (iPop > 0):
						pSpreadCity.setPopulation(iPop)
					CyGame().changeGlobalCounter(1)
				else :
					CyInterface().addMessage(iOwner,True,25,"A bloodbath ensues as the Order and Veil battle for control of the city! In the end, Ritualists lie dead in the streets. The Order is triumphant!",'AS2D_UNIT_BUILD_UNIT',1,'Art/Interface/Buttons/Religions/Order.dds',ColorTypes(14),pSpreadCity.getX(),pSpreadCity.getY(),True,True)
					pSpreadCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'), False, True, True)

		if (not self.__LOG_RELIGIONSPREAD):
			return
		CvUtil.pyPrint('%s has spread to Player %d Civilization %s city of %s'
			%(gc.getReligionInfo(iReligion).getDescription(), iOwner, player.getCivilizationName(), pSpreadCity.getName()))

	def onReligionRemove(self, argsList):
		'Religion Has been removed from a City'
		iReligion, iOwner, pRemoveCity = argsList
		player = PyPlayer(iOwner)
		if (not self.__LOG_RELIGIONSPREAD):
			return
		CvUtil.pyPrint('%s has been removed from Player %d Civilization %s city of %s'
			%(gc.getReligionInfo(iReligion).getDescription(), iOwner, player.getCivilizationName(), pRemoveCity.getName()))

	def onCorporationFounded(self, argsList):
		'Corporation Founded'
		iCorporation, iFounder = argsList
		player = PyPlayer(iFounder)
		if (not self.__LOG_RELIGION):
			return
		CvUtil.pyPrint('Player %d Civilization %s has founded %s'
			%(iFounder, player.getCivilizationName(), gc.getCorporationInfo(iCorporation).getDescription()))

	def onCorporationSpread(self, argsList):
		'Corporation Has Spread to a City'
		iCorporation, iOwner, pSpreadCity = argsList
		player = PyPlayer(iOwner)
		if (not self.__LOG_RELIGIONSPREAD):
			return
		CvUtil.pyPrint('%s has spread to Player %d Civilization %s city of %s'
			%(gc.getCorporationInfo(iCorporation).getDescription(), iOwner, player.getCivilizationName(), pSpreadCity.getName()))

	def onCorporationRemove(self, argsList):
		'Corporation Has been removed from a City'
		iCorporation, iOwner, pRemoveCity = argsList
		player = PyPlayer(iOwner)
		if (not self.__LOG_RELIGIONSPREAD):
			return
		CvUtil.pyPrint('%s has been removed from Player %d Civilization %s city of %s'
			%(gc.getReligionInfo(iReligion).getDescription(), iOwner, player.getCivilizationName(), pRemoveCity.getName()))

	def onGoldenAge(self, argsList):
		'Golden Age'
		iPlayer = argsList[0]
		player = PyPlayer(iPlayer)
		pPlayer = gc.getPlayer(iPlayer)

		if (not self.__LOG_GOLDENAGE):
			return
		CvUtil.pyPrint('Player %d Civilization %s has begun a golden age'
			%(iPlayer, player.getCivilizationName()))

	def onEndGoldenAge(self, argsList):
		'End Golden Age'
		iPlayer = argsList[0]
		player = PyPlayer(iPlayer)
		if (not self.__LOG_ENDGOLDENAGE):
			return
		CvUtil.pyPrint('Player %d Civilization %s golden age has ended'
			%(iPlayer, player.getCivilizationName()))

	def onChangeWar(self, argsList):
		'War Status Changes'
		bIsWar = argsList[0]
		iTeam = argsList[1]
		iRivalTeam = argsList[2]
		if (not self.__LOG_WARPEACE):
			return
		if (bIsWar):
			strStatus = "declared war"
		else:
			strStatus = "declared peace"
		CvUtil.pyPrint('Team %d has %s on Team %d'
			%(iTeam, strStatus, iRivalTeam))

	def onChat(self, argsList):
		'Chat Message Event'
		chatMessage = "%s" %(argsList[0],)

	def onSetPlayerAlive(self, argsList):
		'Set Player Alive Event'
		iPlayerID = argsList[0]
		bNewValue = argsList[1]
		CvUtil.pyPrint("Player %d's alive status set to: %d" %(iPlayerID, int(bNewValue)))
#AIAutoplay disable popup
		if (bNewValue == False and gc.getGame().getGameTurnYear() >= 5 and gc.getGame().getAIAutoPlay() == 0) :
			pPlayer = gc.getPlayer(iPlayerID)
			if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):
				CyGame().changeGlobalCounter(5)
			if pPlayer.getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
				CyGame().changeGlobalCounter(-5)
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_ALEXIS'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_CALABIM",()),'art/interface/popups/Alexis.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_AMELANCHIER'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_LJOSALFAR",()),'art/interface/popups/Amelanchier.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_ARENDEL'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_LJOSALFAR",()),'art/interface/popups/Arendel.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_ARTURUS'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_KHAZAD",()),'art/interface/popups/Arturus.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_AURIC'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_ILLIANS",()),'art/interface/popups/Auric.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_BASIUM'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_MERCURIANS",()),'art/interface/popups/Basium.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_BEERI'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_LUCHUIRP",()),'art/interface/popups/Beeri.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_CAPRIA'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_BANNOR",()),'art/interface/popups/Capria.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_CARDITH'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_KURIOTATES",()),'art/interface/popups/Cardith.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_CASSIEL'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_GRIGORI",()),'art/interface/popups/Cassiel.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_CHARADON'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_DOVIELLO",()),'art/interface/popups/Charadon.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_DAIN'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_AMURITES",()),'art/interface/popups/Dain.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_DECIUS'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_DECIUS",()),'art/interface/popups/Decius.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_EINION'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_ELOHIM",()),'art/interface/popups/Einion.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_ETHNE'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_ELOHIM",()),'art/interface/popups/Ethne.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_FAERYL'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_SVARTALFAR",()),'art/interface/popups/Faeryl.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_FALAMAR'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_LANUN",()),'art/interface/popups/Falamar.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_FLAUROS'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_CALABIM",()),'art/interface/popups/Flauros.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_GARRIM'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_LUCHUIRP",()),'art/interface/popups/Garrim.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_HANNAH'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_HANNAH",()),'art/interface/popups/Hannah.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_HYBOREM'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_INFERNAL",()),'art/interface/popups/Hyborem.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_JONAS'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_CLAN_OF_EMBERS",()),'art/interface/popups/Jonus.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_KANDROS'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_KHAZAD",()),'art/interface/popups/Kandros.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_KEELYN'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_KEELYN",()),'art/interface/popups/Keelyn.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_MAHALA'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_DOVIELLO",()),'art/interface/popups/Mahala.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_SANDALPHON'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_SIDAR",()),'art/interface/popups/Sandalphon.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_OS-GABELLA'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_SHEAIM",()),'art/interface/popups/Os-Gabella.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_PERPENTACH'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_BALSERAPHS",()),'art/interface/popups/Perpentach.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_RHOANNA'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_HIPPUS",()),'art/interface/popups/Rhoanna.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_SABATHIEL'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_BANNOR",()),'art/interface/popups/Sabathiel.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_SHEELBA'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_CLAN_OF_EMBERS",()),'art/interface/popups/Sheelba.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_TASUNKE'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_HIPPUS",()),'art/interface/popups/Tasunke.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_TEBRYN'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_SHEAIM",()),'art/interface/popups/Tebryn.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_THESSA'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_LJOSALFAR",()),'art/interface/popups/Thessa.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_VALLEDIA'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_AMURITES",()),'art/interface/popups/Valledia.dds')
			if pPlayer.getLeaderType() == gc.getInfoTypeForString('LEADER_VARN'):
				cf.addPopup(CyTranslator().getText("TXT_KEY_POPUP_DEFEATED_MALAKIM",()),'art/interface/popups/Varn.dds')

	def onPlayerChangeStateReligion(self, argsList):
		'Player changes his state religion'
		iPlayer, iNewReligion, iOldReligion = argsList

	def onPlayerGoldTrade(self, argsList):
		'Player Trades gold to another player'
		iFromPlayer, iToPlayer, iGoldAmount = argsList

	def onCityBuilt(self, argsList):
		'City Built'
		city = argsList[0]
		pPlot = city.plot()
		pPlayer = gc.getPlayer(city.getOwner())

#		if CyGame().getElapsedGameTurns() < 10:
#			if pPlayer.getNumCities() == 1 and pPlayer.isHuman():
#				iEvent = CvUtil.findInfoTypeNum(gc.getEventTriggerInfo, gc.getNumEventTriggerInfos(),'EVENTTRIGGER_ADVISOR_EARLY_TECH')
#				triggerData = pPlayer.initTriggeredData(iEvent, true, -1, -1, -1, -1, -1, -1, -1, -1, -1)

#xtended
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SHEAIM'):
			if not pPlayer.isHuman():
				city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_PLANAR_GATE'), 1)
				
			
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
			city.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'), True, True, True)
			city.setPopulation(3)
			city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_DEMONIC_CITIZENS'), 1)

#xtended
			iHell1 = gc.getInfoTypeForString('TERRAIN_BROKEN_LANDS')
			iHell2 = gc.getInfoTypeForString('TERRAIN_FIELDS_OF_PERDITION')
			iSnow = gc.getInfoTypeForString('TERRAIN_SNOW')
			iForest = gc.getInfoTypeForString('FEATURE_FOREST')
			iBurntForest = gc.getInfoTypeForString('FEATURE_FOREST_BURNT')
			iX = city.getX()
			iY = city.getY()
			for iiX in range(iX-2, iX+3, 1):
				for iiY in range(iY-2, iY+3, 1):
					pLoopPlot = CyMap().plot(iiX,iiY)
					if pLoopPlot.getTerrainType() == iSnow:
						if CyGame().getSorenRandNum(3, "Sing") == 0:
							pLoopPlot.setTerrainType(iHell1, True, True)
						else:
							pLoopPlot.setTerrainType(iHell2, True, True)
						if pLoopPlot.getFeatureType() == iForest:
							pLoopPlot.setFeatureType(iBurntForest, 0)

		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_BARBARIAN'):
			eTeam = gc.getTeam(gc.getPlayer(gc.getBARBARIAN_PLAYER()).getTeam())

			iUnit = gc.getInfoTypeForString('UNIT_ARCHER')
#			if (eTeam.isHasTech(gc.getInfoTypeForString('TECH_BOWYERS')) or CyGame().getStartEra() > gc.getInfoTypeForString('ERA_CLASSICAL')):
			if 1 < 2:
				iUnit = gc.getInfoTypeForString('UNIT_LONGBOWMAN')
			newUnit2 = pPlayer.initUnit(iUnit, city.getX(), city.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			newUnit3 = pPlayer.initUnit(iUnit, city.getX(), city.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_SOUTH)
			newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORC'), true)
			newUnit3.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ORC'), true)
			if ((not eTeam.isHasTech(gc.getInfoTypeForString('TECH_ARCHERY'))) or CyGame().getStartEra() == gc.getInfoTypeForString('ERA_ANCIENT')):
				newUnit2.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), true)
				newUnit3.setHasPromotion(gc.getInfoTypeForString('PROMOTION_WEAK'), true)

#AUSTRIN
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_AUSTRIN'):
			city.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_AUSTRIN_SETTLEMENT'), 1)
#AUSTRIN
		#AIAutoplay
		if (city.getOwner() == gc.getGame().getActivePlayer() and gc.getGame().getAIAutoPlay() == 0 ):
		#if (city.getOwner() == gc.getGame().getActivePlayer()):
			self.__eventEditCityNameBegin(city, False)
		CvUtil.pyPrint('City Built Event: %s' %(city.getName()))

	def onCityRazed(self, argsList):
		'City Razed'
		city, iPlayer = argsList
		iOwner = city.findHighestCulture()

		pPlayer = gc.getPlayer(city.getOwner())

		iAngel = gc.getInfoTypeForString('UNIT_ANGEL')
		iInfernal = gc.getInfoTypeForString('CIVILIZATION_INFERNAL')
		iManes = gc.getInfoTypeForString('UNIT_MANES')
		iMercurians = gc.getInfoTypeForString('CIVILIZATION_MERCURIANS')
		pPlayer = gc.getPlayer(iPlayer)
		if gc.getPlayer(city.getOriginalOwner()).getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
			if gc.getPlayer(city.getOriginalOwner()).getCivilizationType() != iInfernal:
				for i in range(city.getPopulation()):
					cf.giftUnit(iManes, iInfernal, 0, city.plot(), city.getOwner())

		if gc.getPlayer(city.getOriginalOwner()).getAlignment() == gc.getInfoTypeForString('ALIGNMENT_NEUTRAL'):
			for i in range((city.getPopulation() / 4) + 1):
				cf.giftUnit(iManes, iInfernal, 0, city.plot(), city.getOwner())
				cf.giftUnit(iManes, iInfernal, 0, city.plot(), city.getOwner())
				cf.giftUnit(iAngel, iMercurians, 0, city.plot(), city.getOwner())

		if gc.getPlayer(city.getOriginalOwner()).getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):
			for i in range((city.getPopulation() / 2) + 1):
				cf.giftUnit(iAngel, iMercurians, 0, city.plot(), city.getOwner())

		#Xtended Aristrakh consumes more soul when razing cities
		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
			iConqPop = city.getPopulation()
			iTemp = pPlayer.getPurityCounterCache1()
			iTraitMod=1
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_HERALD_OF_DEATH')):
				iTraitMod=2
				
			pPlayer.setPurityCounterCache1(iTemp + iConqPop*20*iTraitMod)


		pPlot = city.plot()
		iPop = city.getPopulation()
		iCalabim = gc.getInfoTypeForString('CIVILIZATION_CALABIM')
		if pPlayer.getCivilizationType() == iCalabim and iPop > 2:
			iVampire = gc.getInfoTypeForString('PROMOTION_VAMPIRE')
			for i in range(pPlot.getNumUnits()):
				if iPop < 3: break
				pUnit = pPlot.getUnit(i)
				if pUnit.isHasPromotion(iVampire):
					pUnit.changeExperience(iPop, -1, false, false, false)
					iPop = iPop - 1

		CvUtil.pyPrint("City Razed Event: %s" %(city.getName(),))

	def onCityAcquired(self, argsList):
		'City Acquired'
		iPreviousOwner,iNewOwner,pCity,bConquest,bTrade = argsList
		pPlayer = gc.getPlayer(iNewOwner)
		pPrevious = gc.getPlayer(iPreviousOwner)

		if pPrevious.getNumCities()==0 and not pPrevious.isBarbarian():
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALACE_RUINS'), 1)
			infoCiv = gc.getCivilizationInfo(pPrevious.getCivilizationType())
			iPalace = infoCiv.getCivilizationBuildings(gc.getInfoTypeForString('BUILDINGCLASS_PALACE'))
			iBonus = gc.getBuildingInfo(iPalace).getFreeBonus()
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_PALACE_RUINS'), 1)
			pCity.setBuildingBonusChange(gc.getInfoTypeForString('BUILDINGCLASS_PALACE_RUINS'),iBonus,1)

		if pPrevious.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
			pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_OBSIDIAN_GATE'), 0)
#move OOS Fix 1.2
#			pCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ORDER'), False, True, True)
#			pCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'), True, True, True)
		
		CvUtil.pyPrint('City Acquired Event: %s' %(pCity.getName()))

	def onCityAcquiredAndKept(self, argsList):
		'City Acquired and Kept'
		iOwner,pCity = argsList

		pPlayer = gc.getPlayer(iOwner)

#		if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):


		#Functions added here tend to cause OOS issues

		CvUtil.pyPrint('City Acquired and Kept Event: %s' %(pCity.getName()))

	def onCityLost(self, argsList):
		'City Lost'
		city = argsList[0]
		player = PyPlayer(city.getOwner())
		if (not self.__LOG_CITYLOST):
			return
		CvUtil.pyPrint('City %s was lost by Player %d Civilization %s'
			%(city.getName(), player.getID(), player.getCivilizationName()))

	def onCultureExpansion(self, argsList):
		'City Culture Expansion'
		pCity = argsList[0]
		iPlayer = argsList[1]
		CvUtil.pyPrint("City %s's culture has expanded" %(pCity.getName(),))

	def onCityGrowth(self, argsList):
		'City Population Growth'
		pCity = argsList[0]
		iPlayer = argsList[1]
		CvUtil.pyPrint("%s has grown" %(pCity.getName(),))

	def onCityDoTurn(self, argsList):
		'City Production'
		pCity = argsList[0]
		iPlayer = argsList[1]
		pPlot = pCity.plot()
		iPlayer = pCity.getOwner()
		pPlayer = gc.getPlayer(iPlayer)

		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_CITADEL_OF_LIGHT')) > 0:
			iX = pCity.getX()
			iY = pCity.getY()
			eTeam = gc.getTeam(pPlayer.getTeam())
			iBestValue = 0
			pBestPlot = -1
			for iiX in range(iX-2, iX+3, 1):
				for iiY in range(iY-2, iY+3, 1):
					pPlot2 = CyMap().plot(iiX,iiY)
					bEnemy = false
					bNeutral = false
					iValue = 0
					if pPlot2.isVisibleEnemyUnit(iPlayer):
						for i in range(pPlot2.getNumUnits()):
							pUnit = pPlot2.getUnit(i)
							if eTeam.isAtWar(pUnit.getTeam()):
								iValue += 3 * pUnit.baseCombatStr()
								iValue += 5 * pUnit.getLevel()
							else:
								bNeutral = true
						if (iValue > iBestValue and bNeutral == false):
							iBestValue = iValue
							pBestPlot = pPlot2
			if pBestPlot != -1:
				for i in range(pBestPlot.getNumUnits()):
					pUnit = pBestPlot.getUnit(i)
					pUnit.doDamageNoCaster(20, 60, gc.getInfoTypeForString('DAMAGE_FIRE'), False)
				if (pBestPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_FOREST') or pBestPlot.getFeatureType() == gc.getInfoTypeForString('FEATURE_JUNGLE')):
					bValid = True
					iImprovement = pPlot.getImprovementType()
					if iImprovement != -1 :
						if gc.getImprovementInfo(iImprovement).isPermanent():
							bValid = False
					if bValid:
						if CyGame().getSorenRandNum(100, "Flames Spread") <= gc.getDefineINT('FLAMES_SPREAD_CHANCE'):
							pBestPlot.setImprovementType(gc.getInfoTypeForString('IMPROVEMENT_SMOKE'))
				CyEngine().triggerEffect(gc.getInfoTypeForString('EFFECT_PILLAR_OF_FIRE'),pBestPlot.getPoint())

		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_HALL_OF_MIRRORS')) > 0:
			if CyGame().getSorenRandNum(100, "Hall of Mirrors") <= 100:
				pUnit = -1
				iX = pCity.getX()
				iY = pCity.getY()
				eTeam = gc.getTeam(pPlayer.getTeam())
				for iiX in range(iX-1, iX+2, 1):
					for iiY in range(iY-1, iY+2, 1):
						pPlot2 = CyMap().plot(iiX,iiY)
						if pPlot2.isVisibleEnemyUnit(iPlayer):
							for i in range(pPlot2.getNumUnits()):
								pUnit2 = pPlot2.getUnit(i)
								if eTeam.isAtWar(pUnit2.getTeam()):
									if gc.getUnitInfo(pUnit2.getUnitType()).getEquipmentPromotion()==-1:		#added Sephi
										pUnit = pUnit2
				if pUnit != -1:
					newUnit = pPlayer.initUnit(pUnit.getUnitType(), pCity.getX(), pCity.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
					newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_ILLUSION'), true)
					if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SUMMONER')):
						newUnit.setDuration(5)
					else:
						newUnit.setDuration(3)

		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_PLANAR_GATE')) > 0:
			iGamespeed=1
			iMax = 1+ CyGame().getGlobalCounter()/50
			iMult = 1+ CyGame().getGlobalCounter()/25
			iexp = CyGame().getGlobalCounter()
			iCeridwen = 1
			if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_CERIDWEN')):
				iCeridwen = 2
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_QUICK'):
				iGamespeed = 1.5
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_EPIC'):
				iGamespeed = 0.66
			if CyGame().getGameSpeedType() == gc.getInfoTypeForString('GAMESPEED_MARATHON'):
				iGamespeed = 0.33
			if CyGame().getSorenRandNum(10000, "Planar Gate") <= gc.getDefineINT('PLANAR_GATE_CHANCE') *iMult *iGamespeed *iCeridwen:
				listUnits = []
				iMax = iMax *iCeridwen *pPlayer.countNumBuildings(gc.getInfoTypeForString('BUILDING_PLANAR_GATE'))
				if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_PLANAR_GATE')) > 0:
					if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_MATAGOT')) < iMax:
						listUnits.append(gc.getInfoTypeForString('UNIT_MATAGOT'))
				if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_MAGE_GUILD')) > 0:
					if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_SUCCUBUS')) < iMax:
						listUnits.append(gc.getInfoTypeForString('UNIT_SUCCUBUS'))
				if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_TEMPLE_OF_THE_VEIL')) > 0:
					if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_TAR_DEMON')) < iMax:
						listUnits.append(gc.getInfoTypeForString('UNIT_TAR_DEMON'))
				if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_NOBLE_DISTRICT')) > 0:
					if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_MINOTAUR')) < iMax:
						listUnits.append(gc.getInfoTypeForString('UNIT_MINOTAUR'))
				if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_INFERNAL_EFFIGY')) > 0:
					if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_MANTICORE')) < iMax:
						listUnits.append(gc.getInfoTypeForString('UNIT_MANTICORE'))
				if pCity.getNumBuilding(gc.getInfoTypeForString('BUILDING_OBSIDIAN_GATE')) > 0:
					if pPlayer.getUnitClassCount(gc.getInfoTypeForString('UNITCLASS_COLUBRA')) < iMax:
						listUnits.append(gc.getInfoTypeForString('UNIT_COLUBRA'))
				if len(listUnits) > 0:
					iUnit = listUnits[CyGame().getSorenRandNum(len(listUnits), "Planar Gate")]
					newUnit = pPlayer.initUnit(iUnit, pPlot.getX(), pPlot.getY(), UnitAITypes.NO_UNITAI, DirectionTypes.DIRECTION_NORTH)
					newUnit.changeExperience(iexp, -1, False, False, False)
					CyInterface().addMessage(iPlayer,True,25,CyTranslator().getText("TXT_KEY_MESSAGE_PLANAR_GATE",()),'AS2D_DISCOVERBONUS',1,gc.getUnitInfo(newUnit.getUnitType()).getButton(),ColorTypes(8),pCity.getX(),pCity.getY(),True,True)
					if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SUNDERED')):
						newUnit.setHasPromotion(gc.getInfoTypeForString('PROMOTION_STIGMATA'), True)
					if pPlayer.hasTrait(gc.getInfoTypeForString('TRAIT_SUMMONER')):
						newUnit.changeExperience(30, -1, False, False, False)

					for iI in range(gc.getNumAdventureInfos()):
						if(pPlayer.isAdventureEnabled(iI) and not pPlayer.isAdventureFinished(iI)):
							for iJ in range(gc.getMAX_ADVENTURESTEPS()):
								if not gc.getAdventureInfo(iI).getAdventureStep(iJ)==-1:
									if(gc.getAdventureStepInfo(gc.getAdventureInfo(iI).getAdventureStep(iJ)).isSheaimSummonDemon()):
										pPlayer.changeAdventureCounter(iI,iJ,1)


		if gc.getPlayer(pCity.getOwner()).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
			if pCity.isHasReligion(gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL')) == False:
				pCity.setHasReligion(gc.getInfoTypeForString('RELIGION_THE_ASHEN_VEIL'), True, True, True)

		#Zusk: Re-add this line to return Shrine of Sirona functionality
		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_SHRINE_OF_SIRONA')) > 0:
			pPlayer.setFeatAccomplished(FeatTypes.FEAT_HEAL_UNIT_PER_TURN, True)

		if pCity.getNumRealBuilding(gc.getInfoTypeForString('BUILDING_LUXURY_DISTRICT')) > 0:
			eResourceClass=gc.getInfoTypeForString('BUILDINGCLASS_LUXURY_DISTRICT')
			eManaClass=gc.getInfoTypeForString('BONUSCLASS_MANA')
			pTeam=gc.getTeam(pCity.getTeam())
			for i in range(gc.getNumBonusInfos()):
				if gc.getBonusInfo(i).getBonusClassType() == gc.getInfoTypeForString('BONUSCLASS_MANA'):
					continue
				iCount=0
				for iI in range(pCity.getNumCityPlots()):
					pLoopPlot=pCity.getCityIndexPlot(iI)
					#LordShaggynator: fix for cities at map borders
					if(pLoopPlot.getOwner() == pPlayer.getID()):
					#endfix
						if pLoopPlot.getWorkingCity().getID()==pCity.getID() and pLoopPlot.isConnectedTo(pCity):
							if pLoopPlot.getBonusType(pCity.getTeam())==i:
								if pLoopPlot.getImprovementType() != -1:
									if gc.getImprovementInfo(pLoopPlot.getImprovementType()).isImprovementBonusMakesValid(i):
										if gc.getBonusInfo(i).getTechCityTrade()==-1 or pTeam.isHasTech(gc.getBonusInfo(i).getTechCityTrade()):
											iCount=iCount+1
					if iCount>0:
						pCity.setBuildingBonusChange(eResourceClass,i,iCount)

		if pCity.isCapital():
			ePalaceClass=gc.getInfoTypeForString('BUILDINGCLASS_PALACE')
			iManaBonus=pPlayer.getManaBonus2()
			if not iManaBonus==-1:
				pCity.setBuildingBonusChange(ePalaceClass,iManaBonus,1)
			iManaBonus=pPlayer.getManaBonus3()
			if not iManaBonus==-1:
				pCity.setBuildingBonusChange(ePalaceClass,iManaBonus,1)
			iManaBonus=pPlayer.getManaBonus4()
			if not iManaBonus==-1:
				pCity.setBuildingBonusChange(ePalaceClass,iManaBonus,1)
			if pPlayer.getNumCities()==1:
				pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_CITY_STATE_DISTRICT'),1)
			else:
				pCity.setNumRealBuilding(gc.getInfoTypeForString('BUILDING_CITY_STATE_DISTRICT'),0)
		CvAdvisorUtils.cityAdvise(pCity, iPlayer)

	def onCityBuildingUnit(self, argsList):
		'City begins building a unit'
		pCity = argsList[0]
		iUnitType = argsList[1]
		if (not self.__LOG_CITYBUILDING):
			return
		CvUtil.pyPrint("%s has begun building a %s" %(pCity.getName(),gc.getUnitInfo(iUnitType).getDescription()))

	def onCityBuildingBuilding(self, argsList):
		'City begins building a Building'
		pCity = argsList[0]
		iBuildingType = argsList[1]
		if (not self.__LOG_CITYBUILDING):
			return
		CvUtil.pyPrint("%s has begun building a %s" %(pCity.getName(),gc.getBuildingInfo(iBuildingType).getDescription()))

	def onCityRename(self, argsList):
		'City is renamed'
		pCity = argsList[0]
		if (pCity.getOwner() == gc.getGame().getActivePlayer()):
			self.__eventEditCityNameBegin(pCity, True)

	def onCityHurry(self, argsList):
		'City is renamed'
		pCity = argsList[0]
		iHurryType = argsList[1]

	def onVictory(self, argsList):
		'Victory'
		iTeam, iVictory = argsList
		if (iVictory >= 0 and iVictory < gc.getNumVictoryInfos()):
			for iPlayer in range(gc.getMAX_PLAYERS()):
				pPlayer = gc.getPlayer(iPlayer)
				if pPlayer.isAlive():
					if pPlayer.isHuman():
						if pPlayer.getTeam() == iTeam:
							iCiv = pPlayer.getCivilizationType()
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_AMURITES'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_AMURITES", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_BALSERAPHS'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_BALSERAPHS", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_BANNOR'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_BANNOR", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_CALABIM'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_CALABIM", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_CLAN_OF_EMBERS'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_CLAN_OF_EMBERS", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_DOVIELLO'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_DOVIELLO", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_ELOHIM'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_ELOHIM", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_GRIGORI'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_GRIGORI", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_HIPPUS'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_HIPPUS", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_ILLIANS'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_ILLIANS", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_INFERNAL'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_INFERNAL", 1)
							if iCiv	== gc.getInfoTypeForString('CIVILIZATION_KHAZAD'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_KHAZAD", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_KURIOTATES'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_KURIOTATES", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_LANUN'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_LANUN", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_LJOSALFAR'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_LJOSALFAR", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_LUCHUIRP'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_LUCHUIRP", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_MALAKIM'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_MALAKIM", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_MERCURIANS'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_MERCURIANS", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_SHEAIM'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_SHEAIM", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_SIDAR'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_SIDAR", 1)
							if iCiv == gc.getInfoTypeForString('CIVILIZATION_SVARTALFAR'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_SVARTALFAR", 1)

							if iVictory == gc.getInfoTypeForString('VICTORY_ALTAR_OF_THE_LUONNOTAR'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_ALTAR_OF_THE_LUONNOTAR", 1)
							if iVictory == gc.getInfoTypeForString('VICTORY_CONQUEST'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_CONQUEST", 1)
							if iVictory == gc.getInfoTypeForString('VICTORY_CULTURAL'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_CULTURAL", 1)
							if iVictory == gc.getInfoTypeForString('VICTORY_DOMINATION'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_DOMINATION", 1)
							if iVictory == gc.getInfoTypeForString('VICTORY_RELIGIOUS'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_RELIGIOUS", 1)
							if iVictory == gc.getInfoTypeForString('VICTORY_SCORE'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_SCORE", 1)
							if iVictory == gc.getInfoTypeForString('VICTORY_TIME'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_TIME", 1)
							if iVictory == gc.getInfoTypeForString('VICTORY_TOWER_OF_MASTERY'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_TOWER_OF_MASTERY", 1)
	#added Sephi
							if iVictory == gc.getInfoTypeForString('VICTORY_OMNIPOTENCE'):
								CyGame().changeTrophyValue("TROPHY_VICTORY_OMNIPOTENCE", 1)
	#added Sephi
							if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_CHALLENGE_CUT_LOSERS):
								CyGame().changeTrophyValue("TROPHY_VICTORY_FINAL_FIVE", 1)
							if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_CHALLENGE_HIGH_TO_LOW):
								CyGame().changeTrophyValue("TROPHY_VICTORY_HIGH_TO_LOW", 1)
							if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_CHALLENGE_INCREASING_DIFFICULTY):
								CyGame().changeTrophyValue("TROPHY_VICTORY_INCREASING_DIFFICULTY", 1)

			victoryInfo = gc.getVictoryInfo(int(iVictory))
			CvUtil.pyPrint("Victory!  Team %d achieves a %s victory"
				%(iTeam, victoryInfo.getDescription()))

	def onVassalState(self, argsList):
		'Vassal State'
		iMaster, iVassal, bVassal = argsList

		if (bVassal):
			CvUtil.pyPrint("Team %d becomes a Vassal State of Team %d"
				%(iVassal, iMaster))
		else:
			CvUtil.pyPrint("Team %d revolts and is no longer a Vassal State of Team %d"
				%(iVassal, iMaster))

	def onGameUpdate(self, argsList):
		'sample generic event, called on each game turn slice'
		genericArgs = argsList[0][0]	# tuple of tuple of my args
		turnSlice = genericArgs[0]

#FfH: 10/15/2008 Added by Kael for OOS logging.
		OOSLogger.doGameUpdate()
#FfH: End add

	def onMouseEvent(self, argsList):
		'mouse handler - returns 1 if the event was consumed'
		eventType,mx,my,px,py,interfaceConsumed,screens = argsList
		if ( px!=-1 and py!=-1 ):
			if ( eventType == self.EventLButtonDown ):
				if (self.bAllowCheats and self.bCtrl and self.bAlt and CyMap().plot(px,py).isCity() and not interfaceConsumed):
					# Launch Edit City Event
					self.beginEvent( CvUtil.EventEditCity, (px,py) )
					return 1

				elif (self.bAllowCheats and self.bCtrl and self.bShift and not interfaceConsumed):
					# Launch Place Object Event
					self.beginEvent( CvUtil.EventPlaceObject, (px, py) )
					return 1

		if ( eventType == self.EventBack ):
			return CvScreensInterface.handleBack(screens)
		elif ( eventType == self.EventForward ):
			return CvScreensInterface.handleForward(screens)

		return 0


#################### TRIGGERED EVENTS ##################

	def __eventEditCityNameBegin(self, city, bRename):
		popup = PyPopup.PyPopup(CvUtil.EventEditCityName, EventContextTypes.EVENTCONTEXT_ALL)
		popup.setUserData((city.getID(), bRename))
		popup.setHeaderString(localText.getText("TXT_KEY_NAME_CITY", ()))
		popup.setBodyString(localText.getText("TXT_KEY_SETTLE_NEW_CITY_NAME", ()))
		popup.createEditBox(city.getName())
		popup.setEditBoxMaxCharCount( 15 )
		popup.launch()

	def __eventEditCityNameApply(self, playerID, userData, popupReturn):
		'Edit City Name Event'
		iCityID = userData[0]
		bRename = userData[1]
		player = gc.getPlayer(playerID)
		city = player.getCity(iCityID)
		cityName = popupReturn.getEditBoxString(0)
		if (len(cityName) > 30):
			cityName = cityName[:30]
		city.setName(cityName, not bRename)

	def __eventEditCityBegin(self, argsList):
		'Edit City Event'
		px,py = argsList
		CvWBPopups.CvWBPopups().initEditCity(argsList)

	def __eventEditCityApply(self, playerID, userData, popupReturn):
		'Edit City Event Apply'
		if (getChtLvl() > 0):
			CvWBPopups.CvWBPopups().applyEditCity( (popupReturn, userData) )

	def __eventPlaceObjectBegin(self, argsList):
		'Place Object Event'
		CvDebugTools.CvDebugTools().initUnitPicker(argsList)

	def __eventPlaceObjectApply(self, playerID, userData, popupReturn):
		'Place Object Event Apply'
		if (getChtLvl() > 0):
			CvDebugTools.CvDebugTools().applyUnitPicker( (popupReturn, userData) )

	def __eventAwardTechsAndGoldBegin(self, argsList):
		'Award Techs & Gold Event'
		CvDebugTools.CvDebugTools().cheatTechs()

	def __eventAwardTechsAndGoldApply(self, playerID, netUserData, popupReturn):
		'Award Techs & Gold Event Apply'
		if (getChtLvl() > 0):
			CvDebugTools.CvDebugTools().applyTechCheat( (popupReturn) )

	def __eventShowWonderBegin(self, argsList):
		'Show Wonder Event'
		CvDebugTools.CvDebugTools().wonderMovie()

	def __eventShowWonderApply(self, playerID, netUserData, popupReturn):
		'Wonder Movie Apply'
		if (getChtLvl() > 0):
			CvDebugTools.CvDebugTools().applyWonderMovie( (popupReturn) )

	def __eventEditUnitNameBegin(self, argsList):
		pUnit = argsList
		popup = PyPopup.PyPopup(CvUtil.EventEditUnitName, EventContextTypes.EVENTCONTEXT_ALL)
		popup.setUserData((pUnit.getID(),))
		popup.setBodyString(localText.getText("TXT_KEY_RENAME_UNIT", ()))
		popup.createEditBox(pUnit.getNameNoDesc())
		popup.launch()

	def __eventEditUnitNameApply(self, playerID, userData, popupReturn):
		'Edit Unit Name Event'
		iUnitID = userData[0]
		unit = gc.getPlayer(playerID).getUnit(iUnitID)
		newName = popupReturn.getEditBoxString(0)
		if (len(newName) > 25):
			newName = newName[:25]
		unit.setName(newName)

	def __eventWBAllPlotsPopupBegin(self, argsList):
		CvScreensInterface.getWorldBuilderScreen().allPlotsCB()
		return
	def __eventWBAllPlotsPopupApply(self, playerID, userData, popupReturn):
		if (popupReturn.getButtonClicked() >= 0):
			CvScreensInterface.getWorldBuilderScreen().handleAllPlotsCB(popupReturn)
		return

	def __eventWBLandmarkPopupBegin(self, argsList):
		CvScreensInterface.getWorldBuilderScreen().setLandmarkCB("")
		#popup = PyPopup.PyPopup(CvUtil.EventWBLandmarkPopup, EventContextTypes.EVENTCONTEXT_ALL)
		#popup.createEditBox(localText.getText("TXT_KEY_WB_LANDMARK_START", ()))
		#popup.launch()
		return

	def __eventWBLandmarkPopupApply(self, playerID, userData, popupReturn):
		if (popupReturn.getEditBoxString(0)):
			szLandmark = popupReturn.getEditBoxString(0)
			if (len(szLandmark)):
				CvScreensInterface.getWorldBuilderScreen().setLandmarkCB(szLandmark)
		return

	def __eventWBScriptPopupBegin(self, argsList):
		popup = PyPopup.PyPopup(CvUtil.EventWBScriptPopup, EventContextTypes.EVENTCONTEXT_ALL)
		popup.setHeaderString(localText.getText("TXT_KEY_WB_SCRIPT", ()))
		popup.createEditBox(CvScreensInterface.getWorldBuilderScreen().getCurrentScript())
		popup.launch()
		return

	def __eventWBScriptPopupApply(self, playerID, userData, popupReturn):
		if (popupReturn.getEditBoxString(0)):
			szScriptName = popupReturn.getEditBoxString(0)
			CvScreensInterface.getWorldBuilderScreen().setScriptCB(szScriptName)
		return

	def __eventWBStartYearPopupBegin(self, argsList):
		popup = PyPopup.PyPopup(CvUtil.EventWBStartYearPopup, EventContextTypes.EVENTCONTEXT_ALL)
		popup.createSpinBox(0, "", gc.getGame().getStartYear(), 1, 5000, -5000)
		popup.launch()
		return

	def __eventWBStartYearPopupApply(self, playerID, userData, popupReturn):
		iStartYear = popupReturn.getSpinnerWidgetValue(int(0))
		CvScreensInterface.getWorldBuilderScreen().setStartYearCB(iStartYear)
		return

## FfH Card Game: begin
	def __EventSelectSolmniumPlayerBegin(self):
		iHUPlayer = gc.getGame().getActivePlayer()

		if iHUPlayer == -1 : return 0
		if not cs.canStartGame(iHUPlayer) : return 0

		popup = PyPopup.PyPopup(CvUtil.EventSelectSolmniumPlayer, EventContextTypes.EVENTCONTEXT_ALL)

		sResText = CyUserProfile().getResolutionString(CyUserProfile().getResolution())
		sX, sY = sResText.split("x")
		iXRes = int(sX)
		iYRes = int(sY)

		iW = 620
		iH = 650

		popup.setSize(iW, iH)
		popup.setPosition((iXRes - iW) / 2, 30)

		lStates = []

                for iPlayer in range(gc.getMAX_CIV_PLAYERS()) :
                        pPlayer = gc.getPlayer(iPlayer)

                        if pPlayer.isNone() : continue

                        if pPlayer.isHuman() :
                                lPlayerState = cs.getStartGameMPWith(iHUPlayer, iPlayer)
                                if lPlayerState[0][0] in ["No", "notMet"] : continue
                                lStates.append([iPlayer, lPlayerState])
                        else :
                                lPlayerState = cs.getStartGameAIWith(iHUPlayer, iPlayer)
                                if lPlayerState[0][0] in ["No", "notMet"] : continue
                                lStates.append([iPlayer, lPlayerState])

                lPlayerButtons = []

		popup.addDDS(CyArtFileMgr().getInterfaceArtInfo("SOMNIUM_POPUP_INTRO").getPath(), 0, 0, 512, 128)
		popup.addSeparator()
		#popup.setHeaderString(localText.getText("TXT_KEY_SOMNIUM_START", ()), CvUtil.FONT_CENTER_JUSTIFY)
		if len(lStates) == 0 :
                        popup.setBodyString(localText.getText("TXT_KEY_SOMNIUM_NOONE_MET", ()))
                else :
                        #popup.setBodyString(localText.getText("TXT_KEY_SOMNIUM_PLAY_WITH", ()))
                        popup.addSeparator()
                        popup.addSeparator()

                        sText = u""
                        for iPlayer, lPlayerState in lStates :
                                pPlayer = gc.getPlayer(iPlayer)
                                sPlayerName = pPlayer.getName()
                                iPositiveChange = gc.getLeaderHeadInfo(pPlayer.getLeaderType()).getMemoryAttitudePercent(MemoryTypes.MEMORY_SOMNIUM_POSITIVE) / 100
                                iNegativeChange = gc.getLeaderHeadInfo(pPlayer.getLeaderType()).getMemoryAttitudePercent(MemoryTypes.MEMORY_SOMNIUM_NEGATIVE) / 100
                                bShift = True

                                for item in lPlayerState :

                                        sTag = item[0]
                                        if (sTag == "atWar") :
                                                if len(sText) > 0 : sText += localText.getText("[NEWLINE]", ())
                                                sText += localText.getText("TXT_KEY_SOMNIUM_AT_WAR", (sPlayerName, ))

                                        elif (sTag == "InGame") :
                                                if len(sText) > 0 : sText += localText.getText("[NEWLINE]", ())
                                                sText += localText.getText("TXT_KEY_SOMNIUM_IN_GAME", (sPlayerName, ))

                                        elif (sTag == "relation") :
                                                delay = item[1]
                                                if (delay > 0) :
                                                        if len(sText) > 0 : sText += localText.getText("[NEWLINE]", ())
                                                        sText += localText.getText("TXT_KEY_SOMNIUM_GAME_DELAYED", (sPlayerName, delay))
                                                else :
                                                        if bShift :
                                                                bShift = False
                                                                popup.addSeparator()
                                                        popup.addButton(localText.getText("TXT_KEY_SOMNIUM_GAME_RELATION", (sPlayerName, iPositiveChange, iNegativeChange)))
                                                        lPlayerButtons.append((iPlayer, -1))

                                        elif (sTag == "gold") :
                                                for iGold in item[1] :
                                                        if bShift :
                                                                bShift = False
                                                                popup.addSeparator()
                                                        if iGold == 0 :
                                                                popup.addButton(localText.getText("TXT_KEY_SOMNIUM_GAME_FUN", (sPlayerName, )))
                                                                lPlayerButtons.append((iPlayer, iGold))
                                                        else :
                                                                popup.addButton(localText.getText("TXT_KEY_SOMNIUM_GAME_GOLD", (sPlayerName, iGold)))
                                                                lPlayerButtons.append((iPlayer, iGold))

                        if len(sText) > 0 :
                                popup.addSeparator()
                                popup.addSeparator()
                                popup.setBodyString(sText)

		popup.setUserData(tuple(lPlayerButtons))
		popup.launch()

	def __EventSelectSolmniumPlayerApply(self, playerID, userData, popupReturn):
                if userData :
                        idButtonCliked = popupReturn.getButtonClicked()
                        if idButtonCliked in range(len(userData)) :
                                iOpponent, iGold = userData[idButtonCliked]

                                pLeftPlayer = gc.getPlayer(playerID)
                                pRightPlayer = gc.getPlayer(iOpponent)

                                if not pRightPlayer.isHuman() :
                                        if (cs.canStartGame(playerID)) and (pLeftPlayer.isAlive()) and (pRightPlayer.isAlive()) :
                                                cs.startGame(playerID, iOpponent, iGold)
                                        else :
                                                CyInterface().addMessage(playerID, True, 25, CyTranslator().getText("TXT_KEY_SOMNIUM_CANT_START_GAME", (gc.getPlayer(iOpponent).getName(), )), '', 1, '', ColorTypes(7), -1, -1, False, False)
                                else :
                                        if (cs.canStartGame(playerID)) and (cs.canStartGame(iOpponent)) and (pLeftPlayer.isAlive()) and (pRightPlayer.isAlive()) :
                                                if (iOpponent == gc.getGame().getActivePlayer()):
                                                        self.__EventSolmniumAcceptGameBegin((playerID, iOpponent, iGold))
                                        else :
                                                CyInterface().addMessage(playerID, True, 25, CyTranslator().getText("TXT_KEY_SOMNIUM_CANT_START_GAME", (gc.getPlayer(iOpponent).getName(), )), '', 1, '', ColorTypes(7), -1, -1, False, False)

	def __EventSolmniumAcceptGameBegin(self, argslist):
		iPlayer, iOpponent, iGold = argslist
		if not gc.getPlayer(iOpponent).isAlive() : return 0

		popup = PyPopup.PyPopup(CvUtil.EventSolmniumAcceptGame, EventContextTypes.EVENTCONTEXT_ALL)

		popup.setUserData(argslist)

		popup.setHeaderString(localText.getText("TXT_KEY_SOMNIUM_START", ()))
		if iGold > 0 :
                        popup.setBodyString(localText.getText("TXT_KEY_SOMNIUM_ACCEPT_GAME", (gc.getPlayer(iPlayer).getName(), iGold)))
                else :
                        popup.setBodyString(localText.getText("TXT_KEY_SOMNIUM_ACCEPT_GAME_FUN", (gc.getPlayer(iPlayer).getName(), )))

                popup.addButton( localText.getText("AI_DIPLO_ACCEPT_1", ()) )
                popup.addButton( localText.getText("AI_DIPLO_NO_PEACE_3", ()) )

		popup.launch(False, PopupStates.POPUPSTATE_IMMEDIATE)

	def __EventSolmniumAcceptGameApply(self, playerID, userData, popupReturn):
                if userData :
                        iPlayer, iOpponent, iGold = userData
                        idButtonCliked = popupReturn.getButtonClicked()
                        if idButtonCliked == 0 :
                                if (cs.canStartGame(iPlayer)) and (cs.canStartGame(iOpponent)) and (gc.getPlayer(iPlayer).isAlive()) and (gc.getPlayer(iOpponent).isAlive()) :
                                        cs.startGame(iPlayer, iOpponent, iGold)
                                else :
                                        CyInterface().addMessage(iPlayer, True, 25, CyTranslator().getText("TXT_KEY_SOMNIUM_CANT_START_GAME", (gc.getPlayer(iOpponent).getName(), )), '', 1, '', ColorTypes(7), -1, -1, False, False)
                                        CyInterface().addMessage(iOpponent, True, 25, CyTranslator().getText("TXT_KEY_SOMNIUM_CANT_START_GAME", (gc.getPlayer(iPlayer).getName(), )), '', 1, '', ColorTypes(7), -1, -1, False, False)
                        else :
                                        CyInterface().addMessage(iPlayer, True, 25, CyTranslator().getText("TXT_KEY_SOMNIUM_REFUSE_GAME", (gc.getPlayer(iOpponent).getName(), iGold)), '', 1, '', ColorTypes(7), -1, -1, False, False)

	def __EventSolmniumConcedeGameBegin(self, argslist):
		popup = PyPopup.PyPopup(CvUtil.EventSolmniumConcedeGame, EventContextTypes.EVENTCONTEXT_ALL)

		popup.setUserData(argslist)

		popup.setHeaderString(localText.getText("TXT_KEY_SOMNIUM_START", ()))
		popup.setBodyString(localText.getText("TXT_KEY_SOMNIUM_CONCEDE_GAME", ()))

                popup.addButton( localText.getText("AI_DIPLO_ACCEPT_1", ()) )
                popup.addButton( localText.getText("AI_DIPLO_NO_PEACE_3", ()) )

		popup.launch(False, PopupStates.POPUPSTATE_IMMEDIATE)

	def __EventSolmniumConcedeGameApply(self, playerID, userData, popupReturn):
                if userData :
                        idButtonCliked = popupReturn.getButtonClicked()
                        if idButtonCliked == 0 :
                                cs.endGame(userData[0], userData[1])
## FfH Card Game: end
