## Copyright (c) 2005-2006, Gillmer J. Derge.

## This file is part of Civilization IV Alerts mod.
##
## Civilization IV Alerts mod is free software; you can redistribute
## it and/or modify it under the terms of the GNU General Public
## License as published by the Free Software Foundation; either
## version 2 of the License, or (at your option) any later version.
##
## Civilization IV Alerts mod is distributed in the hope that it will
## be useful, but WITHOUT ANY WARRANTY; without even the implied
## warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Civilization IV Alerts mod; if not, write to the Free
## Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
## 02110-1301 USA

__version__ = "$Revision: 1.2 $"
# $Source: /usr/local/cvsroot/Civ4lerts/src/main/python/Civ4lerts.py,v $

## Civ4lerts
## This class extends the built in event manager and overrides various
## event handlers to display alerts about important game situations.
##
## [*] = Already implemented in the Civ4lerts mod
## [o] = Partially implemented in the Civ4lerts mod
## [x] = Already implemented in CivIV
## [?] = Not sure if this applies in CivIV
## 
## Golden Age turns left
## At Year 1000 B.C. (QSC Save Submission)
## Within 10 tiles of domination limit
## There is new technology for sale
## There is a new luxury resource for sale
## There is a new strategic resource for sale
## There is a new bonus resource for sale
## We can sell a technology
## We can sell a luxury resource
## We can sell a strategic resource
## We can sell a bonus resource
## [*] Rival has lots of cash
## [*] Rival has lots of cash per turn
## [x] Rival has changed civics
## Rival has entered a new Era
## Trade deal expires next turn
## [o] Enemy at war is willing to negotiate
## [x] There are foreign units in our territory
## City is about to riot or rioting
## [*] City has grown or shrunk
## City has shrunk
## [*] City is unhealthy
## [*] City is angry
## City specialists reassigned
## [*] City is about to grow
## City is about to starve
## [*] City is about to grow into unhealthiness
## [*] City is about to grow into anger
## City is in resistance
## [?] City is wasting food
## City is working unimproved tiles
## Disconnected resources in our territory
## City is about to produce a great person
## 
## Other:
## City is under cultural pressure


from CvPythonExtensions import *
import BugCore
import BugUtil


# Must set alerts to "not immediate" to have icons show up
# Need a healthy person icon
HEALTHY_ICON = "Art/Interface/Buttons/General/unhealthy_person.dds"
UNHEALTHY_ICON = "Art/Interface/Buttons/General/unhealthy_person.dds"

HAPPY_ICON = "Art/Interface/Buttons/General/happy_person.dds"
UNHAPPY_ICON = "Art/Interface/mainscreen/cityscreen/angry_citizen.dds"

### Globals

gc = CyGlobalContext()
localText = CyTranslator()

Civ4lertsOpt = BugCore.game.Civ4lerts


class Civ4lerts:

	def __init__(self, eventManager):
		cityEvent = BeginActivePlayerTurnCityAlertManager(eventManager)
		cityEvent.add(CityOccupation(eventManager))
		cityEvent.add(CityGrowth(eventManager))
		cityEvent.add(CityHealthiness(eventManager))
		cityEvent.add(CityHappiness(eventManager))
		cityEvent.add(CanHurryPopulation(eventManager))
		cityEvent.add(CanHurryGold(eventManager))
		cityEvent.add(CanHurryStone(eventManager))		
		
		cityEvent = EndTurnReadyCityAlertManager(eventManager)
		cityEvent.add(CityPendingGrowth(eventManager))
		
		GoldTrade(eventManager)
		GoldPerTurnTrade(eventManager)

### Displaying alerts on-screen

def addMessageNoIcon(iPlayer, message):
	"Displays an on-screen message with no popup icon."
	addMessage(iPlayer, message, None)

def addMessageAtCity(iPlayer, message, icon, city):
	"Displays an on-screen message with a popup icon that zooms to the given city."
	addMessage(iPlayer, message, icon, city.getX(), city.getY(), True, True)

def addMessageAtPlot(iPlayer, message, icon, plot):
	"Displays an on-screen message with a popup icon that zooms to the given plot."
	addMessage(iPlayer, message, icon, plot.getX(), plot.getY(), True, True)

def addMessage(iPlayer, szString, szIcon, iFlashX=-1, iFlashY=-1, bOffArrow=False, bOnArrow=False):
	"Displays an on-screen message."
	"""
	Make these alerts optionally show a delayable popup with various options.
	a) show: 
	
	Happy: Zoom to City, Turn OFF avoid growth, Whip (maybe?), Ignore
	Unhappy:  Zoom to City, Turn on Avoid Growth, Suggest cheapest military unit (with right civic), Open Resources screen in FA, Ignore. (for future = suggest building)
	
	Healthy: Zoom to City, Turn OFF avoid growth, Ignore
	Unhealthy:  Zoom to City, Turn on Avoid Growth, Whip population, Open Resources screen in FA, Ignore. (for future = suggest building)
	
	Growth: Zoom to City, Turn on avoid Growth, Whip, Ignore
	Starvation: Zoom to City, Turn on avoid Growth, Ignore
	
	Culture:  Zoom to City, Ignore
	"""
	eventMessageTimeLong = gc.getDefineINT("EVENT_MESSAGE_TIME_LONG")
	CyInterface().addMessage(iPlayer, True, eventMessageTimeLong,
							 szString, None, 0, szIcon, ColorTypes(-1),
							 iFlashX, iFlashY, bOffArrow, bOnArrow)


### Abstract and Core Classes

class AbstractStatefulAlert:
	"""
	Provides a base class and several convenience functions for 
	implementing an alert that retains state information between turns.
	"""
	def __init__(self, eventManager):
		eventManager.addEventHandler("GameStart", self.onGameStart)
		eventManager.addEventHandler("OnLoad", self.onLoadGame)

	def onGameStart(self, argsList):
		self._init()
		self._reset()

	def onLoadGame(self, argsList):
		self._init()
		self._reset()
		return 0

	def _init(self):
		"Initializes globals that could not be done in __init__."
		pass

	def _reset(self):
		"Resets the state for this alert."
		pass

class AbstractCityAlertManager(AbstractStatefulAlert):
	"""
	Triggered when cities are acquired or lost, this event manager passes 
	each off to a set of alert checkers.
	
	All of the alerts are reset when the game is loaded or started.
	"""
	def __init__(self, eventManager):
		AbstractStatefulAlert.__init__(self, eventManager)
		eventManager.addEventHandler("cityAcquiredAndKept", self.onCityAcquiredAndKept)
		eventManager.addEventHandler("cityLost", self.onCityLost)
		self.alerts = []

	def add(self, alert):
		self.alerts.append(alert)
		alert.init()
	
	def onCityAcquiredAndKept(self, argsList):
		iPlayer, city = argsList
		if (iPlayer == gc.getGame().getActivePlayer()):
			self._resetCity(city)
	
	def onCityLost(self, argsList):
		'City Lost'
		city = argsList[0]
		iPlayer = gc.getGame().getActivePlayer()
		if (iPlayer == city.getOwner()):
			self._discardCity(city)

	def _init(self):
		"Initializes each alert."
		for alert in self.alerts:
			alert.init()

	def _reset(self):
		"Resets each alert."
		for alert in self.alerts:
			alert.reset()

	def _resetCity(self, city):
		"tells each alert to check the state of the given city -- no alerts are displayed."
		for alert in self.alerts:
			alert.resetCity(city)

	def _discardCity(self, city):
		"tells each alert to discard the state of the given city."
		for alert in self.alerts:
			alert.discardCity(city)

class BeginActivePlayerTurnCityAlertManager(AbstractCityAlertManager):
	"""
	Extends AbstractCityAlertManager to loop over all of the active player's
	cities at the start of their turn.
	"""
	def __init__(self, eventManager):
		AbstractCityAlertManager.__init__(self, eventManager)
		eventManager.addEventHandler("BeginActivePlayerTurn", self.onBeginActivePlayerTurn)
	
	def onBeginActivePlayerTurn(self, argsList):
		"Loops over active player's cities, telling each to perform its check."
		iTurn = argsList[0]
		iPlayer = gc.getGame().getActivePlayer()
		player = gc.getActivePlayer()
		for iCity in range(player.getNumCities()):
			city = player.getCity(iCity)
			if (city and not city.isNone()):
				for alert in self.alerts:
					iCityID = city.getID()
					alert.checkCity(iTurn, iCityID, city, iPlayer, player)

class EndTurnReadyCityAlertManager(AbstractCityAlertManager):
	"""
	Extends AbstractCityAlertManager to loop over all of the active player's
	cities at the end of their turn (the moment the End Turn button turns red).
	"""
	def __init__(self, eventManager):
		AbstractCityAlertManager.__init__(self, eventManager)
		eventManager.addEventHandler("endTurnReady", self.onEndTurnReady)
	
	def onEndTurnReady(self, argsList):
		"Loops over active player's cities, telling each to perform its check."
		iTurn = argsList[0]
		iPlayer = gc.getGame().getActivePlayer()
		player = gc.getActivePlayer()
		for iCity in range(player.getNumCities()):
			city = player.getCity(iCity)
			if (city and not city.isNone()):
				for alert in self.alerts:
					iCityID = city.getID()
					alert.checkCity(iTurn, iCityID, city, iPlayer, player)


class AbstractCityAlert:
	"""
	Tracks cities from turn-to-turn and checks each at the end of every game turn
	to see if the alert should be displayed.
	"""
	def __init__(self, eventManager):
		"Performs static initialization that doesn't require game data."
		pass
	
	def checkCity(self, iTurn, iCityID, city, iPlayer, player):
		"Checks the city, updates its tracked state and possibly displays an alert."
		pass
	
	def init(self):
		"Initializes globals that could not be done in __init__ and resets the data."
		self._beforeReset()
	
	def reset(self):
		"Clears state kept for each city."
		self._beforeReset()
		player = gc.getActivePlayer()
		for iCity in range(player.getNumCities()):
			city = player.getCity(iCity)
			if (city and not city.isNone()):
				self.resetCity(city)
	
	def _beforeReset(self):
		"Performs clearing of state before looping over cities."
		pass
	
	def resetCity(self, city):
		"Checks the city and updates its tracked state."
		pass
	
	def discardCity(self, city):
		"Discards the tracked state of the city."
		pass

class AbstractCityTestAlert(AbstractCityAlert):
	"""
	Extends the basic city alert by applying a boolean test to each city, tracking the results,
	and displaying an alert whenever a city switches or will switch state on the following turn.
	
	State: set of city IDs that pass the test.
	"""
	def __init__(self, eventManager):
		AbstractCityAlert.__init__(self, eventManager)

	def checkCity(self, iTurn, iCityID, city, iPlayer, player):
		message = None
		passes = self._passesTest(city)
		passed = iCityID in self.cities
		if (passes != passed):
			# City switched this turn, save new state and display an alert
			if (passes):
				self.cities.add(iCityID)
				if (self._isShowAlert(passes)):
					message, icon = self._getAlertMessageIcon(city, passes)
			else:
				self.cities.discard(iCityID)
				if (self._isShowAlert(passes)):
					message, icon = self._getAlertMessageIcon(city, passes)
		elif (self._isShowPendingAlert(passes)):
			# See if city will switch next turn
			willPass = self._willPassTest(city)
			if (passed != willPass):
				message, icon = self._getPendingAlertMessageIcon(city, willPass)
		if (message):
			addMessageAtCity(iPlayer, message, icon, city)
	
	def _passedTest(self, iCityID):
		"Returns true if the city passed the test last turn."
		return iCityID in self.cities

	def _passesTest(self, city):
		"Returns true if the city passes the test."
		return False

	def _willPassTest(self, city):
		"Returns true if the city will pass the test next turn based on current conditions."
		return False

	def _beforeReset(self):
		self.cities = set()
	
	def resetCity(self, city):
		if (self._passesTest(city)):
			self.cities.add(city.getID())
	
	def discardCity(self, city):
		self.cities.discard(city.getID())
	
	def _isShowAlert(self, passes):
		"Returns true if the alert is enabled."
		return False
	
	def _getAlertMessageIcon(self, city, passes):
		"Returns a tuple of the message and icon to use for the alert."
		return (None, None)
	
	def _isShowPendingAlert(self, passes):
		"Returns true if the alert is enabled."
		return False

	def _getPendingAlertMessageIcon(self, city, passes):
		"Returns a tuple of the message and icon to use for the pending alert."
		return (None, None)


### Population

class CityPendingGrowth(AbstractCityAlert):
	"""
	Displays an alert when a city's population will change next turn.
	State: None.
	"""
	def __init__(self, eventManager):
		AbstractCityAlert.__init__(self, eventManager)
	
	def checkCity(self, iTurn, iCityID, city, iPlayer, player):
		if (Civ4lertsOpt.isShowCityPendingGrowthAlert()):
			iFoodRate = city.foodDifference(True)
			if (iFoodRate > 0 and city.getFoodTurnsLeft() == 1 
			and not city.isFoodProduction() and not city.AI_isEmphasize(5) and not city.AI_stopGrowth()):
				message = localText.getText(
						"TXT_KEY_CIV4LERTS_ON_CITY_PENDING_GROWTH",
						(city.getName(), city.getPopulation() + 1))
				icon = "Art/Interface/Symbols/Food/food05.dds"
				addMessageAtCity(iPlayer, message, icon, city)
			elif (iFoodRate < 0 and city.getFood() // -iFoodRate == 0):
				if not player.hasTrait(gc.getInfoTypeForString('TRAIT_FALLOW')):
					message = localText.getText(
							"TXT_KEY_CIV4LERTS_ON_CITY_PENDING_SHRINKAGE",
							(city.getName(), city.getPopulation() - 1))
					icon = "Art/Interface/Symbols/Food/food05.dds"
					addMessageAtCity(iPlayer, message, icon, city)

class CityGrowth(AbstractCityAlert):
	"""
	Displays an alert when a city's population changes.
	State: map of populations by city ID.
	"""
	def __init__(self, eventManager):
		AbstractCityAlert.__init__(self, eventManager)
	
	def checkCity(self, iTurn, iCityID, city, iPlayer, player):
		if (iCityID not in self.populations):
			self.resetCity(city)
		else:
			iPop = city.getPopulation()
			iOldPop = self.populations[iCityID]
			iWhipCounter = city.getHurryAngerTimer()
			iOldWhipCounter = self.CityWhipCounter[iCityID]
			iConscriptCounter = city.getConscriptAngerTimer()
			iOldConscriptCounter = self.CityConscriptCounter[iCityID]
			
			bWhipOrDraft = False
			if (iWhipCounter > iOldWhipCounter
			or  iConscriptCounter > iOldConscriptCounter):
				bWhipOrDraft = True
			
			if (Civ4lertsOpt.isShowCityGrowthAlert()):
				if (iPop > iOldPop):
					message = localText.getText(
							"TXT_KEY_CIV4LERTS_ON_CITY_GROWTH",
							(city.getName(), iPop))
					icon = "Art/Interface/Symbols/Food/food05.dds"
					addMessageAtCity(iPlayer, message, icon, city)
				elif iPop < iOldPop and not bWhipOrDraft:
					message = localText.getText(
							"TXT_KEY_CIV4LERTS_ON_CITY_SHRINKAGE",
							(city.getName(), iPop))
					icon = "Art/Interface/Symbols/Food/food05.dds"
					addMessageAtCity(iPlayer, message, icon, city)

			self.populations[iCityID] = iPop
			self.CityWhipCounter[iCityID] = iWhipCounter
			self.CityConscriptCounter[iCityID] = iConscriptCounter

	def _beforeReset(self):
		self.populations = dict()
		self.CityWhipCounter = dict()
		self.CityConscriptCounter = dict()
	
	def resetCity(self, city):
		self.populations[city.getID()] = city.getPopulation()
		self.CityWhipCounter[city.getID()] = city.getHurryAngerTimer()
		self.CityConscriptCounter[city.getID()] = city.getConscriptAngerTimer()
	
	def discardCity(self, city):
		cityID = city.getID()
		if (cityID in self.populations):
			del self.populations[cityID]
			del self.CityWhipCounter[cityID]
			del self.CityConscriptCounter[cityID]

### Happiness and Healthiness

class CityHappiness(AbstractCityTestAlert):
	"""
	Displays an event when a city goes from happy to angry or vice versa.
	
	Test: True if the city is unhappy.
	"""
	def __init__(self, eventManager):
		AbstractCityTestAlert.__init__(self, eventManager)
	
	def init(self):
		AbstractCityAlert.init(self)
		self.kiTempHappy = gc.getDefineINT("TEMP_HAPPY")
	
	def _passesTest(self, city):
		return city.angryPopulation(0) > 0

	def _willPassTest(self, city):
		if (city.getFoodTurnsLeft() == 1 and not city.isFoodProduction() and not city.AI_isEmphasize(5) and not city.AI_stopGrowth()):
			iExtra = 1
		elif (city.getFoodTurnsLeft() == -1):
			iExtra = -1
		else:
			iExtra = 0
		iHappy = city.happyLevel()
		iUnhappy = city.unhappyLevel(iExtra)
		if (iUnhappy > 0 and city.getHurryAngerTimer() > 0 
		and city.getHurryAngerTimer() % city.flatHurryAngerLength() == 0):
			iUnhappy -= 1
		if (iUnhappy > 0 and city.getConscriptAngerTimer()
		and city.getConscriptAngerTimer() % city.flatConscriptAngerLength() == 0):
			iUnhappy -= 1
		if (iUnhappy > 0 and city.getDefyResolutionAngerTimer() > 0
		and city.getDefyResolutionAngerTimer() % city.flatDefyResolutionAngerLength() == 0):
			iUnhappy -= 1
		if (iUnhappy > 0 and city.getEspionageHappinessCounter() > 0):
			iUnhappy -= 1
		if (iHappy > 0 and city.getHappinessTimer() == 1):
			iHappy -= self.kiTempHappy
		if (iHappy < 0):
			iHappy = 0
		if (iUnhappy < 0):
			iUnhappy = 0
		return iHappy < iUnhappy
	
	def _isShowAlert(self, passes):
		return Civ4lertsOpt.isShowCityHappinessAlert()
	
	def _getAlertMessageIcon(self, city, passes):
		if (passes):
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_UNHAPPY", (city.getName(), )),
					UNHAPPY_ICON)
		else:
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_HAPPY", (city.getName(), )),
					HAPPY_ICON)
	
	def _isShowPendingAlert(self, passes):
		return Civ4lertsOpt.isShowCityPendingHappinessAlert()

	def _getPendingAlertMessageIcon(self, city, passes):
		if (passes):
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_PENDING_UNHAPPY", (city.getName(), )),
					UNHAPPY_ICON)
		else:
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_PENDING_HAPPY", (city.getName(), )),
					HAPPY_ICON)

class CityHealthiness(AbstractCityTestAlert):
	"""
	Displays an event when a city goes from healthy to sick or vice versa.
	
	Test: True if the city is unhealthy.
	"""
	def __init__(self, eventManager):
		AbstractCityTestAlert.__init__(self, eventManager)
	
	def _passesTest(self, city):
		return city.healthRate(False, 0) < 0

	def _willPassTest(self, city):
		if (city.getFoodTurnsLeft() == 1 and not city.isFoodProduction() and not city.AI_isEmphasize(5) and not city.AI_stopGrowth()):
			iExtra = 1
		elif (city.getFoodTurnsLeft() == -1):
			iExtra = -1
		else:
			iExtra = 0
		# badHealth() doesn't take iExtra!
		iHealthRate = city.healthRate(False, iExtra)
		if (city.getEspionageHealthCounter() > 0):
			iHealthRate += 1
		return iHealthRate < 0
	
	def _isShowAlert(self, passes):
		return Civ4lertsOpt.isShowCityHealthinessAlert()
	
	def _getAlertMessageIcon(self, city, passes):
		if (passes):
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_UNHEALTHY", (city.getName(), )),
					UNHEALTHY_ICON)
		else:
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_HEALTHY", (city.getName(), )),
					HEALTHY_ICON)
	
	def _isShowPendingAlert(self, passes):
		return Civ4lertsOpt.isShowCityPendingHealthinessAlert()

	def _getPendingAlertMessageIcon(self, city, passes):
		if (passes):
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_PENDING_UNHEALTHY", (city.getName(), )),
					UNHEALTHY_ICON)
		else:
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_PENDING_HEALTHY", (city.getName(), )),
					HEALTHY_ICON)


### Anarchy

class CityOccupation(AbstractCityTestAlert):
	"""
	Displays an alert when a city switches to/from occupation.
	
	Test: True if the city is under occupation.
	"""
	def __init__(self, eventManager):
		AbstractCityTestAlert.__init__(self, eventManager)
	
	def _passesTest(self, city):
		return city.isOccupation()

	def _willPassTest(self, city):
		return city.isOccupation() and city.getOccupationTimer() > 1
	
	def _isShowAlert(self, passes):
		return Civ4lertsOpt.isShowCityOccupationAlert()
	
	def _getAlertMessageIcon(self, city, passes):
		if (passes):
			BugUtil.debug("%s passed occupation test, ignoring", city.getName())
			return (None, None)
		else:
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_PACIFIED", (city.getName(), )),
					HAPPY_ICON)
	
	def _isShowPendingAlert(self, passes):
		return Civ4lertsOpt.isShowCityPendingOccupationAlert()

	def _getPendingAlertMessageIcon(self, city, passes):
		if (passes):
			BugUtil.warn("%s passed pending occupation test, ignoring", city.getName())
			return (None, None)
		else:
			return (localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_PENDING_PACIFIED", (city.getName(), )),
					HAPPY_ICON)


### Hurrying Production

class AbstractCanHurry(AbstractCityTestAlert):
	"""
	Displays an alert when a city can hurry the current production item.
	
	Test: True if the city can hurry.
	"""
	def __init__(self, eventManager):
		AbstractCityTestAlert.__init__(self, eventManager)
		eventManager.addEventHandler("cityBuildingUnit", self.onCityBuildingUnit)
		eventManager.addEventHandler("cityBuildingBuilding", self.onCityBuildingBuilding)
	
	def init(self, szHurryType):
		AbstractCityAlert.init(self)
		self.keHurryType = gc.getInfoTypeForString(szHurryType)

	def onCityBuildingUnit(self, argsList):
		city, iUnit = argsList
		self._onItemStarted(city)

	def onCityBuildingBuilding(self, argsList):
		city, iBuilding = argsList
		self._onItemStarted(city)

	def _onItemStarted(self, city):
		if (city.getOwner() == gc.getGame().getActivePlayer()):
			self.discardCity(city)
	
	def _passesTest(self, city):
#		return city.canHurry(self.keHurryType, False)
		return city.canHurry(self.keHurryType, False) and city.getProductionTurnsLeft()>1
	
	def _getAlertMessageIcon(self, city, passes):
		if (passes):
			info = None
			if (city.isProductionBuilding()):
				iType = city.getProductionBuilding()
				if (iType >= 0):
					info = gc.getBuildingInfo(iType)
			elif (city.isProductionUnit()):
				iType = city.getProductionUnit()
				if (iType >= 0):
					info = gc.getUnitInfo(iType)
			elif (city.isProductionProject()):
				# Can't hurry projects, but just in case
				iType = city.getProductionProject()
				if (iType >= 0):
					info = gc.getProjectInfo(iType)
			if (info):
				return (self._getAlertMessage(city, info), info.getButton())
		return (None, None)

class CanHurryPopulation(AbstractCanHurry):
#   Displays an alert when a city can hurry using population.

	def __init__(self, eventManager): 
		AbstractCanHurry.__init__(self, eventManager)
	
	def init(self):
		AbstractCanHurry.init(self, "HURRY_POPULATION")

	def _isShowAlert(self, passes):
		return passes and Civ4lertsOpt.isShowCityCanHurryPopAlert()
	
	def _getAlertMessage(self, city, info):
		iPop = city.hurryPopulation(self.keHurryType)
		iOverflow = city.hurryProduction(self.keHurryType) - city.productionLeft()
		if Civ4lertsOpt.isWhipAssistOverflowCountCurrentProduction():
			iOverflow = iOverflow + city.getCurrentProductionDifference(True, False)
		iAnger = city.getHurryAngerTimer() + city.flatHurryAngerLength()
		iMaxOverflow = min(city.getProductionNeeded(), iOverflow)
		iOverflowGold = max(0, iOverflow - iMaxOverflow) * gc.getDefineINT("MAXED_UNIT_GOLD_PERCENT") / 100
		iOverflow =  100 * iMaxOverflow / city.getBaseYieldRateModifier(gc.getInfoTypeForString("YIELD_PRODUCTION"), city.getProductionModifier())
		if (iOverflowGold > 0):
			return localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_CAN_HURRY_POP_PLUS_GOLD", (city.getName(), info.getDescription(), iPop, iOverflow, iAnger, iOverflowGold))

		else:
			return localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_CAN_HURRY_POP", (city.getName(), info.getDescription(), iPop, iOverflow, iAnger))

class CanHurryGold(AbstractCanHurry):
#   Displays an alert when a city can hurry using gold.

	def __init__(self, eventManager): 
		AbstractCanHurry.__init__(self, eventManager)

	def init(self):
		AbstractCanHurry.init(self, "HURRY_GOLD")

	def _isShowAlert(self, passes):
		return passes and Civ4lertsOpt.isShowCityCanHurryGoldAlert()
	
	def _getAlertMessage(self, city, info):
		iGold = city.hurryGold(self.keHurryType)
		return localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_CAN_HURRY_GOLD", 
								 (city.getName(), info.getDescription(), iGold))


class CanHurryStone(AbstractCanHurry):
#   Displays an alert when a city can hurry using gold.

	def __init__(self, eventManager): 
		AbstractCanHurry.__init__(self, eventManager)

	def init(self):
		AbstractCanHurry.init(self, "HURRY_STONE")

	def _isShowAlert(self, passes):
		return passes and Civ4lertsOpt.isShowCityCanHurryGoldAlert()
	
	def _getAlertMessage(self, city, info):
		iGold = city.hurryStone(self.keHurryType)
		return localText.getText("TXT_KEY_CIV4LERTS_ON_CITY_CAN_HURRY_STONE", 
								 (city.getName(), info.getDescription(), iGold))
								 
### Trading Gold

class GoldTrade(AbstractStatefulAlert):
#   Displays an alert when a civilization has a significant increase
#	in gold available for trade since the last alert.

	def __init__(self, eventManager):
		AbstractStatefulAlert.__init__(self, eventManager)
		self._reset()
		eventManager.addEventHandler("BeginActivePlayerTurn", self.onBeginActivePlayerTurn)

	def onBeginActivePlayerTurn(self, argsList):
		if (not Civ4lertsOpt.isShowGoldTradeAlert()):
			return
		
		turn = argsList[0]
		player = gc.getGame().getActivePlayer()
		team = gc.getTeam(gc.getPlayer(player).getTeam())
		for rival in range(gc.getMAX_PLAYERS()):
			if (rival == player): continue
			rivalPlayer = gc.getPlayer(rival)
			rivalTeam = gc.getTeam(rivalPlayer.getTeam())
			# TODO: does this need to check for war or trade denial?
			if (team.isHasMet(rivalPlayer.getTeam())
			and (team.isGoldTrading() or rivalTeam.isGoldTrading())):
				oldMaxGoldTrade = self._getMaxGoldTrade(player, rival)
				newMaxGoldTrade = rivalPlayer.AI_maxGoldTrade(player)
				deltaMaxGoldTrade = newMaxGoldTrade - oldMaxGoldTrade
				if (deltaMaxGoldTrade >= Civ4lertsOpt.getGoldTradeThreshold()):
					message = localText.getText(
							"TXT_KEY_CIV4LERTS_ON_GOLD_TRADE",
							(gc.getPlayer(rival).getName(),
							 newMaxGoldTrade))
					addMessageNoIcon(player, message)
					self._setMaxGoldTrade(player, rival, newMaxGoldTrade)
				else:
					maxGoldTrade = min(oldMaxGoldTrade, newMaxGoldTrade)
					self._setMaxGoldTrade(player, rival, maxGoldTrade)

	def _reset(self):
		self.maxGoldTrade = {}
		for player in range(gc.getMAX_PLAYERS()):
			self.maxGoldTrade[player] = {}
			for rival in range(gc.getMAX_PLAYERS()):
				self._setMaxGoldTrade(player, rival, 0)

	def _getMaxGoldTrade(self, player, rival):
		return self.maxGoldTrade[player][rival]
	
	def _setMaxGoldTrade(self, player, rival, value):
		self.maxGoldTrade[player][rival] = value

class GoldPerTurnTrade(AbstractStatefulAlert):
#   Displays an alert when a civilization has a significant increase
#   in gold per turn available for trade since the last alert.

	def __init__(self, eventManager):
		AbstractStatefulAlert.__init__(self, eventManager)
		eventManager.addEventHandler("BeginActivePlayerTurn", self.onBeginActivePlayerTurn)

	def onBeginActivePlayerTurn(self, argsList):
		if (not Civ4lertsOpt.isShowGoldPerTurnTradeAlert()):
			return
		
		turn = argsList[0]
		player = gc.getGame().getActivePlayer()
		team = gc.getTeam(gc.getPlayer(player).getTeam())
		for rival in range(gc.getMAX_PLAYERS()):
			if (rival == player): continue
			rivalPlayer = gc.getPlayer(rival)
			rivalTeam = gc.getTeam(rivalPlayer.getTeam())
			# TODO: does this need to check for war or trade denial?
			if (team.isHasMet(rivalPlayer.getTeam())
				and (team.isGoldTrading() or rivalTeam.isGoldTrading())):
				oldMaxGoldPerTurnTrade = self._getMaxGoldPerTurnTrade(player, rival)
				newMaxGoldPerTurnTrade = rivalPlayer.AI_maxGoldPerTurnTrade(player)
				deltaMaxGoldPerTurnTrade = newMaxGoldPerTurnTrade - oldMaxGoldPerTurnTrade
				if (deltaMaxGoldPerTurnTrade >= Civ4lertsOpt.getGoldPerTurnTradeThreshold()):
					message = localText.getText(
							"TXT_KEY_CIV4LERTS_ON_GOLD_PER_TURN_TRADE",
							(gc.getPlayer(rival).getName(),
							 newMaxGoldPerTurnTrade))
					addMessageNoIcon(player, message)
					self._setMaxGoldPerTurnTrade(player, rival, newMaxGoldPerTurnTrade)
				else:
					maxGoldPerTurnTrade = min(oldMaxGoldPerTurnTrade, newMaxGoldPerTurnTrade)
					self._setMaxGoldPerTurnTrade(player, rival, maxGoldPerTurnTrade)

	def _reset(self):
		self.maxGoldPerTurnTrade = {}
		for player in range(gc.getMAX_PLAYERS()):
			self.maxGoldPerTurnTrade[player] = {}
			for rival in range(gc.getMAX_PLAYERS()):
				self._setMaxGoldPerTurnTrade(player, rival, 0)

	def _getMaxGoldPerTurnTrade(self, player, rival):
		return self.maxGoldPerTurnTrade[player][rival]
	
	def _setMaxGoldPerTurnTrade(self, player, rival, value):
		self.maxGoldPerTurnTrade[player][rival] = value
