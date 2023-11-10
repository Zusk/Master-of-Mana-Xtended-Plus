## MoreCiv4lerts
## From HOF MOD V1.61.001
## Based upon Gillmer J. Derge's Civ4lerts.py

from CvPythonExtensions import *
import CvUtil
import PyHelpers
import BugCore

gc = CyGlobalContext()
localText = CyTranslator()
PyGame = PyHelpers.PyGame()
PyPlayer = PyHelpers.PyPlayer
PyCity = PyHelpers.PyCity
PyInfo = PyHelpers.PyInfo

PEACE_TREATY_LENGTH = gc.getDefineINT("PEACE_TREATY_LENGTH")

class MoreCiv4lerts:

	def __init__(self, eventManager):
		## Init event handlers
		MoreCiv4lertsEvent(eventManager)

class AbstractMoreCiv4lertsEvent(object):
	
	def __init__(self, eventManager, *args, **kwargs):
			super( AbstractMoreCiv4lertsEvent, self).__init__(*args, **kwargs)

	def _addMessageNoIcon(self, iPlayer, message, iColor=-1):
			#Displays an on-screen message with no popup icon.
			self._addMessage(iPlayer, message, None, -1, -1, False, False, iColor)

	def _addMessageAtCity(self, iPlayer, message, icon, city, iColor=-1):
			#Displays an on-screen message with a popup icon that zooms to the given city.
			self._addMessage(iPlayer, message, icon, city.getX(), city.getY(), True, True, iColor)

	def _addMessageAtPlot(self, iPlayer, message, icon, plot, iColor=-1):
			#Displays an on-screen message with a popup icon that zooms to the given plot.
			self._addMessage(iPlayer, message, icon, plot.getX(), plot.getY(), True, True, iColor)

	def _addMessage(self, iPlayer, szString, szIcon, iFlashX, iFlashY, bOffArrow, bOnArrow, iColor):
			#Displays an on-screen message.
			eventMessageTimeLong = gc.getDefineINT("EVENT_MESSAGE_TIME_LONG")
			CyInterface().addMessage(iPlayer, True, eventMessageTimeLong,
									 szString, None, 0, szIcon, ColorTypes(iColor),
									 iFlashX, iFlashY, bOffArrow, bOnArrow)

class MoreCiv4lertsEvent( AbstractMoreCiv4lertsEvent):

	def __init__(self, eventManager, *args, **kwargs):
		super(MoreCiv4lertsEvent, self).__init__(eventManager, *args, **kwargs)

		eventManager.addEventHandler("BeginActivePlayerTurn", self.onBeginActivePlayerTurn)
		eventManager.addEventHandler("cityAcquired", self.OnCityAcquired)
		eventManager.addEventHandler("cityBuilt", self.OnCityBuilt)
		eventManager.addEventHandler("cityRazed", self.OnCityRazed)
		eventManager.addEventHandler("cityLost", self.OnCityLost)

		self.eventMgr = eventManager
		self.CurrAvailTechTrades = {}
		self.PrevAvailTechTrades = {}
		self.PrevAvailOpenBordersTrades = set()
		self.PrevAvailDefensivePactTrades = set()
		self.PrevAvailPermanentAllianceTrades = set()
		self.PrevAvailVassalTrades = set()
		self.PrevAvailSurrenderTrades = set()
		self.PrevAvailPeaceTrades = set()
		self.lastDomLimitMsgTurn = 0
		self.lastPopCount = 0
		self.lastLandCount = 0
		
		self.options = BugCore.game.MoreCiv4lerts

	def getCheckForDomPopVictory(self):
		return self.options.isShowDomPopAlert()

	def getCheckForDomLandVictory(self):
		return self.options.isShowDomLandAlert()

	def getPopThreshold(self):
		return self.options.getDomPopThreshold()

	def getLandThreshold(self):
		return self.options.getDomLandThreshold()

	def getCheckForCityBorderExpansion(self):
		return self.options.isShowCityPendingExpandBorderAlert()

	def getCheckForNewTrades(self):
		return self.options.isShowTechTradeAlert()

	def getCheckForOpenBorders(self):
		return self.options.isShowOpenBordersTradeAlert()

	def getCheckForDefensivePact(self):
		return self.options.isShowDefensivePactTradeAlert()

	def getCheckForPermanentAlliance(self):
		return self.options.isShowPermanentAllianceTradeAlert()
	
	def getCheckForVassal(self):
		return self.options.isShowVassalTradeAlert()
	
	def getCheckForSurrender(self):
		return self.options.isShowSurrenderTradeAlert()
	
	def getCheckForPeace(self):
		return self.options.isShowPeaceTradeAlert()

	def getCheckForDomVictory(self):
		return self.getCheckForDomPopVictory() or self.getCheckForDomLandVictory()
	
	def getCheckForForeignCities(self):
		return self.options.isShowCityFoundedAlert()

	def getDoChecks(self):
		return self.getCheckForDomVictory() or self.getCheckForCityBorderExpansion() or self.getCheckForNewTrades()

	def onBeginActivePlayerTurn(self, argsList):
		"Called when the active player can start making their moves."
		iGameTurn = argsList[0]
		if (not self.getDoChecks()): return
		iPlayer = gc.getGame().getActivePlayer()
		self.CheckForAlerts(iPlayer, PyPlayer(iPlayer).getTeam(), True)

	def OnCityAcquired(self, argsList):
		owner,playerType,city,bConquest,bTrade = argsList
		iPlayer = city.getOwner()
		if (not self.getCheckForDomVictory()): return
		if (iPlayer == gc.getGame().getActivePlayer()):
			self.CheckForAlerts(iPlayer, PyPlayer(iPlayer).getTeam(), False)

	def OnCityBuilt(self, argsList):
		city = argsList[0]
		iPlayer = city.getOwner()
		iActivePlayer = gc.getGame().getActivePlayer()
		if (self.getCheckForDomVictory()):
			if (iPlayer == iActivePlayer):
				self.CheckForAlerts(iPlayer, PyPlayer(iPlayer).getTeam(), False)
		if (self.getCheckForForeignCities()):
			if (iPlayer != iActivePlayer):
				player = gc.getPlayer(iPlayer)
				team = gc.getTeam(player.getTeam())
				if (not player.isBarbarian() and not player.isMinorCiv() and player.isAlive() and team.isHasMet(gc.getPlayer(iActivePlayer).getTeam()) and not team.isAVassal()):
					#iColor = gc.getPlayerColorInfo(player.getPlayerColor()).getColorTypePrimary()
					iColor = gc.getInfoTypeForString("COLOR_MAGENTA")
					if (city.isRevealed(gc.getActivePlayer().getTeam(), False)):
						message = localText.getText("TXT_KEY_MORECIV4LERTS_CITY_FOUNDED", (player.getName(), city.getName()))
						self._addMessageAtCity(iActivePlayer, message, "Art/Interface/Buttons/Actions/foundcity.dds", city, iColor)
					else:
						message = localText.getText("TXT_KEY_MORECIV4LERTS_CITY_FOUNDED_UNSEEN", (player.getName(), city.getName()))
						self._addMessageNoIcon(iActivePlayer, message, iColor)

	def OnCityRazed(self, argsList):
		city, iPlayer = argsList
		if (not self.getCheckForDomVictory()): return
		if (iPlayer == gc.getGame().getActivePlayer()):
			self.CheckForAlerts(iPlayer, PyPlayer(iPlayer).getTeam(), False)

	def OnCityLost(self, argsList):
		city = argsList[0]
		iPlayer = city.getOwner()
		if (not self.getCheckForDomVictory()): return
		if (iPlayer == gc.getGame().getActivePlayer()):
			self.CheckForAlerts(iPlayer, PyPlayer(iPlayer).getTeam(), False)

	def CheckForAlerts(self, iActivePlayer, activeTeam, BeginTurn):
	##Added "else: pass" code to diagnose strange results - might be related to indent issues
		ourPop = 0
		ourLand = 0
		totalPop = 0
		totalLand = 0
		LimitPop =0
		LimitLand = 0
		DomVictory = 3
		popGrowthCount = 0
		currentTurn = gc.getGame().getGameTurn()
		activePlayer = gc.getPlayer(iActivePlayer)

		if (self.getCheckForDomPopVictory() or (BeginTurn and self.getCheckForCityBorderExpansion())):
			# Check for cultural expansion and population growth
			teamPlayerList = []
			teamPlayerList = PyGame.getCivTeamList(PyGame.getActiveTeam())
			teamPlayerList.append(PyPlayer(iActivePlayer))
			for loopPlayer in range(len(teamPlayerList)):
				lCity = []
				# EF: This looks very wrong. Above the list of players will not be 0, 1, ...
				#     but here it uses loopPlayer which is 0, 1, ...
				lCity = PyPlayer(loopPlayer).getCityList()
				for loopCity in range(len(lCity)):
					city = gc.getPlayer(loopPlayer).getCity(loopCity)
					if (city.getFoodTurnsLeft() == 1 and not city.isFoodProduction()) and not city.AI_isEmphasize(5):
						popGrowthCount = popGrowthCount + 1
					if (BeginTurn and self.getCheckForCityBorderExpansion()):
						if (city.getCultureLevel() != gc.getNumCultureLevelInfos() - 1):
							if ((city.getCulture(loopPlayer) + city.getCommerceRate(CommerceTypes.COMMERCE_CULTURE)) >= city.getCultureThreshold()):
								if(city.getCultureLevel() % 2 == 1): #added Sephi
									message = localText.getText("TXT_KEY_MORECIV4LERTS_CITY_TO_EXPAND",(city.getName(),))
									icon = "Art/Interface/Buttons/General/Warning_popup.dds"
									self._addMessageAtCity(loopPlayer, message, icon, city)
							else: pass
						else: pass #expand check
					else: pass #message check
				else: pass #end city loop
			else: pass #end activePlayer loop
		else: pass # end expansion check / pop count

		# Check Domination Limit
		if (self.getCheckForDomVictory() and gc.getGame().isVictoryValid(DomVictory)):
			
			# Population Limit
			if (self.getCheckForDomPopVictory()):
				VictoryPopPercent = 0.0
				VictoryPopPercent = gc.getGame().getAdjustedPopulationPercent(DomVictory) * 1.0
				totalPop = gc.getGame().getTotalPopulation()
				LimitPop = int((totalPop * VictoryPopPercent) / 100.0)
				ourPop = activeTeam.getTotalPopulation()
				if (totalPop > 0):
					popPercent = (ourPop * 100.0) / totalPop
					NextpopPercent = ((ourPop + popGrowthCount) * 100.0) / totalPop
				else:
					popPercent = 0.0
					NextpopPercent = 0.0

				if (totalPop > 1 and (currentTurn <> self.lastDomLimitMsgTurn or (ourPop + popGrowthCount) <> self.lastPopCount)):
					self.lastPopCount = ourPop + popGrowthCount
					if (popPercent >= VictoryPopPercent):
						message = localText.getText("TXT_KEY_MORECIV4LERTS_POP_EXCEEDS_LIMIT",
						   		(ourPop, (u"%.2f%%" % popPercent), LimitPop, (u"%.2f%%" % VictoryPopPercent)))
						self._addMessageNoIcon(iActivePlayer, message)

					elif (popGrowthCount > 0 and NextpopPercent >= VictoryPopPercent):
						message = localText.getText("TXT_KEY_MORECIV4LERTS_POP_GROWTH_EXCEEDS_LIMIT",
						   		(ourPop, popGrowthCount, (u"%.2f%%" % NextpopPercent), LimitPop, (u"%.2f%%" % VictoryPopPercent)))
						self._addMessageNoIcon(iActivePlayer, message)

					elif (popGrowthCount > 0 and (VictoryPopPercent - NextpopPercent < self.getPopThreshold())):
						message = localText.getText("TXT_KEY_MORECIV4LERTS_POP_GROWTH_CLOSE_TO_LIMIT",
						   		(ourPop, popGrowthCount, (u"%.2f%%" % NextpopPercent), LimitPop, (u"%.2f%%" % VictoryPopPercent)))
						self._addMessageNoIcon(iActivePlayer, message)

## .005 			elif (VictoryPopPercent - popPercent < self.getPopThreshold()):
					elif (popGrowthCount > 0 and (VictoryPopPercent - popPercent < self.getPopThreshold())):
						message = localText.getText("TXT_KEY_MORECIV4LERTS_POP_CLOSE_TO_LIMIT",
						   		(ourPop, (u"%.2f%%" % popPercent), LimitPop, (u"%.2f%%" % VictoryPopPercent)))
						self._addMessageNoIcon(iActivePlayer, message)
					else: pass #end elif
				else: pass #end totalPop if
			else: pass #end pop limit if

			# Land Limit
			if (self.getCheckForDomLandVictory()):
				VictoryLandPercent = 0.0
				VictoryLandPercent = gc.getGame().getAdjustedLandPercent(DomVictory) * 1.0
				totalLand = gc.getMap().getLandPlots()
				LimitLand = int((totalLand * VictoryLandPercent) / 100.0)
				ourLand = activeTeam.getTotalLand()
				if (totalLand > 0):
					landPercent = (ourLand * 100.0) / totalLand
				else:
					landPercent = 0.0
				if (currentTurn <> self.lastDomLimitMsgTurn or ourLand <> self.lastLandCount):
					self.lastLandCount = ourLand
					if (landPercent > VictoryLandPercent):
						message = localText.getText("TXT_KEY_MORECIV4LERTS_LAND_EXCEEDS_LIMIT",
								(ourLand, (u"%.2f%%" % landPercent), LimitLand, (u"%.2f%%" % VictoryLandPercent)))
						self._addMessageNoIcon(iActivePlayer, message)
					elif (VictoryLandPercent - landPercent < self.getLandThreshold()):
						message = localText.getText("TXT_KEY_MORECIV4LERTS_LAND_CLOSE_TO_LIMIT",
								(ourLand, (u"%.2f%%" % landPercent), LimitLand, (u"%.2f%%" % VictoryLandPercent)))
						self._addMessageNoIcon(iActivePlayer, message)
					else: pass #end elif
				else: pass #end currentTurn if
			else: pass #end land limit if
		else: pass #end dom limt if
	
		#save turn num
		if (self.getCheckForDomVictory()):
		    self.lastDomLimitMsgTurn = currentTurn

		# new trades
		if (BeginTurn and self.getCheckForNewTrades()):
			researchTechs = set()
			for iTech in range(gc.getNumTechInfos()):
				if (activePlayer.canResearch(iTech, True)):
					researchTechs.add(iTech)
			techsByPlayer = self.getTechForTrade(activePlayer, activeTeam)
			for iLoopPlayer, currentTechs in techsByPlayer.iteritems():

				#Did he have trades avail last turn
				if (self.PrevAvailTechTrades.has_key(iLoopPlayer)):
					previousTechs = self.PrevAvailTechTrades[iLoopPlayer]
				else:
					previousTechs = set()
					
				#Determine new techs
				newTechs = currentTechs.difference(previousTechs).intersection(researchTechs)
				if (newTechs):
					szNewTechs = self.buildTechString(newTechs)
					message = localText.getText("TXT_KEY_MORECIV4LERTS_NEW_TECH_AVAIL",	
												(gc.getPlayer(iLoopPlayer).getName(), szNewTechs))
					self._addMessageNoIcon(iActivePlayer, message)
				
				#Determine removed techs
				removedTechs = previousTechs.difference(currentTechs).intersection(researchTechs)
				if (removedTechs):
					szRemovedTechs = self.buildTechString(removedTechs)
					message = localText.getText("TXT_KEY_MORECIV4LERTS_TECH_NOT_AVAIL",	
												(gc.getPlayer(iLoopPlayer).getName(), szRemovedTechs))
					self._addMessageNoIcon(iActivePlayer, message)
				
			else: pass #end activePlayer loop

			#save curr trades for next time
			self.PrevAvailTechTrades = techsByPlayer

		else: pass #end new trades if
		
		if (BeginTurn and self.getCheckForOpenBorders()):
			currentTrades = self.getOpenBordersTrades(activePlayer, activeTeam)
			newTrades = currentTrades.difference(self.PrevAvailOpenBordersTrades)
			self.PrevAvailOpenBordersTrades = currentTrades
			if (newTrades):
				players = self.buildPlayerString(newTrades)
				message = localText.getText("TXT_KEY_MORECIV4LERTS_OPEN_BORDERS", (players,))
				self._addMessageNoIcon(iActivePlayer, message)
		
		if (BeginTurn and self.getCheckForDefensivePact()):
			currentTrades = self.getDefensivePactTrades(activePlayer, activeTeam)
			newTrades = currentTrades.difference(self.PrevAvailDefensivePactTrades)
			self.PrevAvailDefensivePactTrades = currentTrades
			if (newTrades):
				players = self.buildPlayerString(newTrades)
				message = localText.getText("TXT_KEY_MORECIV4LERTS_DEFENSIVE_PACT", (players,))
				self._addMessageNoIcon(iActivePlayer, message)
		
		if (BeginTurn and self.getCheckForPermanentAlliance()):
			currentTrades = self.getPermanentAllianceTrades(activePlayer, activeTeam)
			newTrades = currentTrades.difference(self.PrevAvailPermanentAllianceTrades)
			self.PrevAvailPermanentAllianceTrades = currentTrades
			if (newTrades):
				players = self.buildPlayerString(newTrades)
				message = localText.getText("TXT_KEY_MORECIV4LERTS_PERMANENT_ALLIANCE", (players,))
				self._addMessageNoIcon(iActivePlayer, message)
		
		if (BeginTurn and self.getCheckForVassal()):
			currentTrades = self.getVassalTrades(activePlayer, activeTeam)
			newTrades = currentTrades.difference(self.PrevAvailVassalTrades)
			self.PrevAvailVassalTrades = currentTrades
			if (newTrades):
				players = self.buildPlayerString(newTrades)
				message = localText.getText("TXT_KEY_MORECIV4LERTS_VASSAL", (players,))
				self._addMessageNoIcon(iActivePlayer, message)
		
		if (BeginTurn and self.getCheckForSurrender()):
			currentTrades = self.getSurrenderTrades(activePlayer, activeTeam)
			newTrades = currentTrades.difference(self.PrevAvailSurrenderTrades)
			self.PrevAvailSurrenderTrades = currentTrades
			if (newTrades):
				players = self.buildPlayerString(newTrades)
				message = localText.getText("TXT_KEY_MORECIV4LERTS_SURRENDER", (players,))
				self._addMessageNoIcon(iActivePlayer, message)

# Disabled until it works well				
#		if (BeginTurn and self.getCheckForPeace()):
#			currentTrades = self.getPeaceTrades(activePlayer, activeTeam)
#			newTrades = currentTrades.difference(self.PrevAvailPeaceTrades)
#			self.PrevAvailPeaceTrades = currentTrades
#			if (newTrades):
#				players = self.buildPlayerString(newTrades)
#				message = localText.getText("TXT_KEY_MORECIV4LERTS_PEACE_TREATY", (players,))
#				self._addMessageNoIcon(iActivePlayer, message)

	def getTechForTrade(self, player, team):
		iTeamID = team.getID()
		iPlayerID = player.getID()
		tradeData = TradeData()
		tradeData.ItemType = TradeableItems.TRADE_TECHNOLOGIES
		techsByPlayer = {}

		for iLoopPlayer in range(gc.getMAX_PLAYERS()):
			loopPlayer = gc.getPlayer(iLoopPlayer)
			loopTeam = gc.getTeam(loopPlayer.getTeam())

			if (loopPlayer.isBarbarian()): continue
			if (loopPlayer.isMinorCiv()): continue
			if (not loopPlayer.isAlive()): continue
			if (iLoopPlayer != iPlayerID and loopTeam.isHasMet(iTeamID)):
				if loopPlayer.isHuman() or loopPlayer.AI_isWillingToTalk(iPlayerID):						
			
					if (team.isTechTrading() or loopTeam.isTechTrading()):
						techsToTrade = set()
						for iLoopTech in range(gc.getNumTechInfos()):
							tradeData.iData = iLoopTech
							if (loopPlayer.canTradeItem(iPlayerID, tradeData, False)):
								if (loopPlayer.getTradeDenial(iPlayerID, tradeData) == DenialTypes.NO_DENIAL): # will trade
									techsToTrade.add(iLoopTech)
						techsByPlayer[iLoopPlayer] = techsToTrade
		return techsByPlayer

	def getOpenBordersTrades(self, activePlayer, activeTeam):
		iActivePlayerID = activePlayer.getID()
		iActiveTeamID = activeTeam.getID()
		tradeData = TradeData()
		tradeData.ItemType = TradeableItems.TRADE_OPEN_BORDERS
		currentTrades = set()
		
		for iLoopPlayerID in range(gc.getMAX_PLAYERS()):
			loopPlayer = gc.getPlayer(iLoopPlayerID)
			iLoopTeamID = loopPlayer.getTeam()
			loopTeam = gc.getTeam(iLoopTeamID)
			if (loopPlayer.isBarbarian() or loopPlayer.isMinorCiv() or not loopPlayer.isAlive()):
				continue
			if (iLoopPlayerID != iActivePlayerID and loopTeam.isHasMet(iActiveTeamID)):
				if (activeTeam.isOpenBorders(iLoopTeamID) or loopTeam.isOpenBorders(iActiveTeamID)):
					continue
				if (activeTeam.isOpenBordersTrading() or loopTeam.isOpenBordersTrading()):
					if not loopPlayer.isHuman():
						if loopPlayer.AI_isWillingToTalk(iActivePlayerID):						
				
							#tradeData.iData = None
							if (loopPlayer.canTradeItem(iActivePlayerID, tradeData, False)):
								if (loopPlayer.getTradeDenial(iActivePlayerID, tradeData) == DenialTypes.NO_DENIAL): # will trade
									currentTrades.add(iLoopPlayerID)
		return currentTrades

	def getDefensivePactTrades(self, activePlayer, activeTeam):
		iActivePlayerID = activePlayer.getID()
		iActiveTeamID = activeTeam.getID()
		tradeData = TradeData()
		tradeData.ItemType = TradeableItems.TRADE_DEFENSIVE_PACT
		currentTrades = set()
		
		for iLoopPlayerID in range(gc.getMAX_PLAYERS()):
			loopPlayer = gc.getPlayer(iLoopPlayerID)
			iLoopTeamID = loopPlayer.getTeam()
			loopTeam = gc.getTeam(iLoopTeamID)
			if (loopPlayer.isBarbarian() or loopPlayer.isMinorCiv() or not loopPlayer.isAlive()):
				continue
			if (iLoopPlayerID != iActivePlayerID and loopTeam.isHasMet(iActiveTeamID)):
				if (activeTeam.isDefensivePact(iLoopTeamID) or loopTeam.isDefensivePact(iActiveTeamID)):
					continue
				if (activeTeam.isDefensivePactTrading() or loopTeam.isDefensivePactTrading()):
					if not loopPlayer.isHuman():
						if loopPlayer.AI_isWillingToTalk(iActivePlayerID):						
				
							#tradeData.iData = None
							if (loopPlayer.canTradeItem(iActivePlayerID, tradeData, False)):
								if (loopPlayer.getTradeDenial(iActivePlayerID, tradeData) == DenialTypes.NO_DENIAL): # will trade
									currentTrades.add(iLoopPlayerID)
		return currentTrades

	def getPermanentAllianceTrades(self, activePlayer, activeTeam):
		iActivePlayerID = activePlayer.getID()
		iActiveTeamID = activeTeam.getID()
		tradeData = TradeData()
		tradeData.ItemType = TradeableItems.TRADE_PERMANENT_ALLIANCE
		currentTrades = set()
		
		for iLoopPlayerID in range(gc.getMAX_PLAYERS()):
			loopPlayer = gc.getPlayer(iLoopPlayerID)
			iLoopTeamID = loopPlayer.getTeam()
			loopTeam = gc.getTeam(iLoopTeamID)
			if (loopPlayer.isBarbarian() or loopPlayer.isMinorCiv() or not loopPlayer.isAlive()):
				continue
			if (iLoopPlayerID != iActivePlayerID and loopTeam.isHasMet(iActiveTeamID)):
				if not loopPlayer.isHuman():
					if loopPlayer.AI_isWillingToTalk(iActivePlayerID):						
			
						# Once teams sign a PA, they become a single team
						#if (activeTeam.isDefensivePact(iLoopTeamID) or loopTeam.isDefensivePact(iActiveTeamID)):
						#	continue				
						if (activeTeam.isPermanentAllianceTrading() or loopTeam.isPermanentAllianceTrading()):
							#tradeData.iData = None
							if (loopPlayer.canTradeItem(iActivePlayerID, tradeData, False)):
								if (loopPlayer.getTradeDenial(iActivePlayerID, tradeData) == DenialTypes.NO_DENIAL): # will trade
									currentTrades.add(iLoopPlayerID)
		return currentTrades

	def getVassalTrades(self, activePlayer, activeTeam):
		iActivePlayerID = activePlayer.getID()
		iActiveTeamID = activeTeam.getID()
		tradeData = TradeData()
		tradeData.ItemType = TradeableItems.TRADE_VASSAL
		currentTrades = set()
		
		for iLoopPlayerID in range(gc.getMAX_PLAYERS()):
			loopPlayer = gc.getPlayer(iLoopPlayerID)
			iLoopTeamID = loopPlayer.getTeam()
			loopTeam = gc.getTeam(iLoopTeamID)
			if (loopPlayer.isBarbarian() or loopPlayer.isMinorCiv() or not loopPlayer.isAlive()):
				continue
			if (iLoopPlayerID != iActivePlayerID and loopTeam.isHasMet(iActiveTeamID)):
				if not loopPlayer.isHuman():
					if loopPlayer.AI_isWillingToTalk(iActivePlayerID):						
						if (loopPlayer.canTradeItem(iActivePlayerID, tradeData, False)):
							if (loopPlayer.getTradeDenial(iActivePlayerID, tradeData) == DenialTypes.NO_DENIAL): # will trade
								currentTrades.add(iLoopPlayerID)
		return currentTrades

	def getSurrenderTrades(self, activePlayer, activeTeam):
		iActivePlayerID = activePlayer.getID()
		iActiveTeamID = activeTeam.getID()
		tradeData = TradeData()
		tradeData.ItemType = TradeableItems.TRADE_SURRENDER
		currentTrades = set()
		
		for iLoopPlayerID in range(gc.getMAX_PLAYERS()):
			loopPlayer = gc.getPlayer(iLoopPlayerID)
			iLoopTeamID = loopPlayer.getTeam()
			loopTeam = gc.getTeam(iLoopTeamID)
			if (loopPlayer.isBarbarian() or loopPlayer.isMinorCiv() or not loopPlayer.isAlive()):
				continue
			if (iLoopPlayerID != iActivePlayerID and loopTeam.isHasMet(iActiveTeamID)):
				if not loopPlayer.isHuman():
					if loopPlayer.AI_isWillingToTalk(iActivePlayerID):			
						if (loopPlayer.canTradeItem(iActivePlayerID, tradeData, False)):
							if (loopPlayer.getTradeDenial(iActivePlayerID, tradeData) == DenialTypes.NO_DENIAL): # will trade
								currentTrades.add(iLoopPlayerID)
		return currentTrades

	def getPeaceTrades(self, activePlayer, activeTeam):
		iActivePlayerID = activePlayer.getID()
		iActiveTeamID = activeTeam.getID()
		tradeData = TradeData()
		tradeData.ItemType = TradeableItems.TRADE_PEACE_TREATY
		tradeData.iData = PEACE_TREATY_LENGTH
		currentTrades = set()
		
		for iLoopPlayerID in range(gc.getMAX_PLAYERS()):
			loopPlayer = gc.getPlayer(iLoopPlayerID)
			iLoopTeamID = loopPlayer.getTeam()
			loopTeam = gc.getTeam(iLoopTeamID)
			if (loopPlayer.isBarbarian() or loopPlayer.isMinorCiv() or not loopPlayer.isAlive()):
				continue
			if (iLoopPlayerID != iActivePlayerID and loopTeam.isHasMet(iActiveTeamID)):
				if not loopPlayer.isHuman():
					if loopPlayer.AI_isWillingToTalk(iActivePlayerID):
						if (loopTeam.isAtWar(iActiveTeamID)):
							if (loopPlayer.canTradeItem(iActivePlayerID, tradeData, False)):
								if (loopPlayer.getTradeDenial(iActivePlayerID, tradeData) == DenialTypes.NO_DENIAL): # will trade
									currentTrades.add(iLoopPlayerID)
		return currentTrades
	
	def buildTechString(self, techs):
		szTechs = u""
		for iTech in techs:
			tech = gc.getTechInfo(iTech)
			if (szTechs):
				szTechs += u", "
			szTechs += tech.getDescription()
		return szTechs

	def buildPlayerString(self, players):
		szPlayers = u""
		for iPlayer in players:
			if (szPlayers):
				szPlayers += u", "
			szPlayers += PyPlayer(iPlayer).getName()
		return szPlayers
