## BugEventManager
##
## Extended version of CvCustomEventManager by Gillmer J. Derge.
##
## Changes:
##
## You no longer need to modify this module in order to add your custom events.
## Instead, add <event> and <events> tags to your mod's config XML or call
## addEventHandler() from your mod's Python, preferably an <init> function.
##
## * New public methods
##
##   - hasEvent(eventType)
##       Returns True if eventType is defined
##
##   - addEvent(eventType)
##       Adds a new event type with no default handler; you can also add a final True
##       parameter to your call to addEventHandler() or use <event> and <events>
##       in your mod's config XML to add your events and handlers.
##
##   - fireEvent(eventType, args...)
##       Fires an event from Python, building an argList from the arguments passed in
##
##   - removePopupHandler(eventType)
##       Removes the handlers for a popup event (int)
##
##   - addShortcutHandler(keys, function)
##       Adds a handler for the given keyboard shortcut(s)
##
## * New events
##
##   - BeginActivePlayerTurn
##       Signifies the moment the active player can begin making their moves
##       Fired from CvMainInterface.updateScreen()
##
##   - LanguageChanged 
##       Fired from CvOptionsScreenCallbackInterface.handleLanguagesDropdownBoxInput()
##
##   - ResolutionChanged 
##       Fired from CvOptionsScreenCallbackInterface.handleResolutionDropdownInput()
##
##   - PreGameStart
##       Fired from CvAppInterface.preGameStart()
##
##   - PythonReloaded
##       Fired after Python modules have been reloaded while game is running
##
## * Fixed events
##
##   - endTurnReady
##       Signifies the moment the "End Turn" text is displayed on the screen
##       Fired from CvMainInterface.updateScreen()
##
##   - gameUpdate
##       Fired from CvMainInterface.updateScreen() every 250 milliseconds
##
## * Events and their arguments are optionally logged.
## * Added configure() to set the options.
##
## * Calls BugInit.init() once before "OnLoad" or "PreGameStart" events
##   are handled because CyGlobalContext is not ready during "Init" event.
##   Both must do it because "OnLoad" happens before "PreGameStart", but
##   the latter happens before "GameStart" (as expected).
##
## Copyright (c) 2008 The BUG Mod.
##
## Author: EmperorFool, Gillmer J. Derge

from CvPythonExtensions import *
import CvEventManager
import BugUtil
import InputUtil
import types
# --------- Revolution mod (1/2) -------------
import RevolutionInit

DEFAULT_LOGGING = False
DEFAULT_NOLOG_EVENTS = set((
	"gameUpdate",
))

gc = CyGlobalContext()
g_eventManager = None

class BugEventManager(CvEventManager.CvEventManager):

	"""
	Extends the standard event manager by adding support for multiple
	handlers for each event.
	
	Instead of modifying this file as you would have done with CvCustomEventManager,
	use the <event> and <events> tags in your mod's initialization XML file.
	
	Methods exist for both adding and removing event handlers.  A set method 
	also exists to override the default handlers.  Clients should not depend 
	on event handlers being called in a particular order, though they are
	called in the order in which they are added.
	
	Note that the naming conventions for the event type strings vary from event
	to event.  Some use initial capitalization, some do not; some eliminate the
	"on..." prefix used in the event handler function name, some do not.  Look
	at the unmodified CvEventManager.py source code to determine the correct
	name for a particular event.
	
	Take care with event handlers that also extend CvEventManager.  Since
	this event manager handles invocation of the base class handler function,
	additional handlers should not also call the base class function themselves.
	
	It's best *not* to extend CvEventManager or CvCustomEventManager. In fact,
	you are free to use module methods outside classes if you wish. 
	
	"""

	def __init__(self, logging=None, noLogEvents=None):
		CvEventManager.CvEventManager.__init__(self)
		
		global g_eventManager
		if g_eventManager is not None:
			raise ConfigError("BugEventManager already created")
		g_eventManager = self
		
		if logging is None:
			self.setLogging(DEFAULT_LOGGING)
		else:
			self.setLogging(logging)
		if noLogEvents is None:
			self.setNoLogEvents(DEFAULT_NOLOG_EVENTS)
		else:
			self.setNoLogEvents(noLogEvents)
		
		self.bDbg = False
		self.bMultiPlayer = False
		self.bAllowCheats = False
		
		# used to register shortcut handlers
		self.shortcuts = {}
		
		# used for BeginActivePlayerTurn
		self.iActiveTurn = -1
		self.bEndTurnFired = False
		
		# map the initial EventHandlerMap values into the new data structure
		for eventType, eventHandler in self.EventHandlerMap.iteritems():
			self.setEventHandler(eventType, eventHandler)
		
		# add new core events
		self.addEvent("PreGameStart")
		self.addEvent("BeginActivePlayerTurn")
		self.addEvent("LanguageChanged")
		self.addEvent("ResolutionChanged")
		self.addEvent("PythonReloaded")
		
		self.addEventHandler("kbdEvent", self.onKbdEvent)
		self.addEventHandler("OnLoad", self.resetActiveTurn)
		self.addEventHandler("GameStart", self.resetActiveTurn)

		# --------- Revolution mod (2/2) -------------
        	revConfigFile = "AIAutoPlay.ini"
        	RevolutionInit.RevolutionInit( self, revConfigFile )
	
	def setLogging(self, logging):
		if logging is not None:
			self.logging = bool(logging)
	
	def setNoLogEvents(self, noLogEvents):
		if noLogEvents is not None:
			try:
				x = "gameUpdate" in noLogEvents
			except:
				raise ConfigError("noLogEvents must be tuple, list or set")
			else:
				self.noLogEvents = noLogEvents

	def hasEvent(self, eventType):
		"""Returns True if the given event type is defined."""
		return eventType in self.EventHandlerMap

	def _checkEvent(self, eventType):
		"""Raises ConfigError if the eventType is undefined."""
		if not self.hasEvent(eventType):
			raise BugUtil.ConfigError("Event '%s' is undefined" % eventType)

	def addEvent(self, eventType):
		"""Creates a new event type without any handlers.
		
		Prints a warning if eventType is already defined.
		"""
		if self.hasEvent(eventType):
			BugUtil.warn("BugEventManager - event '%s' already defined", eventType)
		else:
			BugUtil.debug("BugEventManager - adding event '%s'", eventType)
			self.EventHandlerMap[eventType] = []

	def addEventHandler(self, eventType, eventHandler=None):
		"""Adds a handler for the given event type, adding the event if necessary.
		
		If eventHandler is None, the event is added if necessary without a handler.
		
		A list of supported event types can be found in the initialization 
		of EventHandlerMap in the CvEventManager class. A debug message is
		printed if the event type doesn't exist.

		"""
		if not self.hasEvent(eventType):
			self.addEvent(eventType)
		if eventHandler:
			self.EventHandlerMap[eventType].append(eventHandler)

	def removeEventHandler(self, eventType, eventHandler):
		"""Removes a handler for the given event type.
		
		A list of supported event types can be found in the initialization 
		of EventHandlerMap in the CvEventManager class.  It is an error if 
		the given handler is not found in the list of installed handlers.
		
		Throws ConfigError if the eventType is undefined.

		"""
		self._checkEvent(eventType)
		self.EventHandlerMap[eventType].remove(eventHandler)
	
	def setEventHandler(self, eventType, eventHandler):
		"""Removes all previously installed event handlers for the given 
		event type and installs a new handler, adding the event if necessary.
		
		A list of supported event types can be found in the initialization 
		of EventHandlerMap in the CvEventManager class.  This method is 
		primarily useful for overriding, rather than extending, the default 
		event handler functionality.

		"""
		if not self.hasEvent(eventType):
			self.addEvent(eventType)
		if eventHandler is not None:
			self.EventHandlerMap[eventType] = [eventHandler]
		else:
			self.EventHandlerMap[eventType] = []
	
	def setPopupHandler(self, eventType, popupHandler):
		"""Removes all previously installed popup handlers for the given 
		event type and installs a new handler.
		
		The eventType should be an integer.  It must be unique with respect
		to the integers assigned to built in events.  The popupHandler should
		be a list made up of (name, beginFunction, applyFunction).  The name
		is used in debugging output.  The begin and apply functions are invoked
		by beginEvent and applyEvent, respectively, to manage a popup dialog
		in response to the event.

		"""
		BugUtil.debug("BugEventManager - setting popup handler for event %d", eventType)
		self.Events[eventType] = popupHandler
	
	def removePopupHandler(self, eventType):
		"""Removes all previously installed popup handlers for the given 
		event type.
		
		The eventType should be an integer. It is an error to fire this
		eventType after removing its handlers.

		"""
		if eventType in self.Events:
			BugUtil.debug("BugEventManager - removing popup handler for event %d", eventType)
			del self.Events[eventType]
		else:
			BugUtil.warn("BugEventManager - event %d has no popup handler", eventType)
	
	def addShortcutHandler(self, keys, handler):
		"""Adds the given handler to be called when any of the keyboard shortcut(s) is hit.
		
		The keys argument may be a single Keystroke, a collection of one or more Keystrokes, or
		a string which will be converted to such.
		If any keystrokes have existing handlers, new ones are ignored and a warning is displayed.
		
		"""
		if isinstance(keys, InputUtil.Keystroke):
			keys = (keys,)
		elif isinstance(keys, types.StringTypes):
			keys = InputUtil.stringToKeystrokes(keys)
		for key in keys:
			if key in self.shortcuts:
				BugUtil.error("shortcut %s already assigned", key)
			else:
				BugUtil.debug("BugEventManager - setting shortcut handler for %s", key)
				self.shortcuts[key] = handler
	
	
	def fireEvent(self, eventType, *args):
		"""Fires the given event passing in all args as a list."""
		self._dispatchEvent(eventType, args)

	def handleEvent(self, argsList):
		"""Handles events by calling all installed handlers."""
		self.bDbg, self.bMultiPlayer, self.bAlt, self.bCtrl, self.bShift, self.bAllowCheats = argsList[-6:]
		self._dispatchEvent(argsList[0], argsList[1:-6])
	
	def _dispatchEvent(self, eventType, argsList):
		if self.logging:
			self._logEvent(eventType, argsList)
		return EVENT_FUNCTION_MAP.get(eventType, BugEventManager._handleDefaultEvent)(self, eventType, argsList)

	def _logEvent(self, eventType, argsList):
		if self.logging and eventType not in self.noLogEvents:
			if argsList:
				BugUtil.debug("BugEventManager - event %s: %r", eventType, argsList)
			else:
				BugUtil.debug("BugEventManager - event %s", eventType)

	def _handleDefaultEvent(self, eventType, argsList):
		if self.EventHandlerMap.has_key(eventType):
			for eventHandler in self.EventHandlerMap[eventType]:
				eventHandler(argsList)

	def _handleConsumableEvent(self, eventType, argsList):
		"""Handles events that can be consumed by the handlers, such as
		keyboard or mouse events.
		
		If a handler returns non-zero, processing is terminated, and no 
		subsequent handlers are invoked.

		"""
		if self.EventHandlerMap.has_key(eventType):
			for eventHandler in self.EventHandlerMap[eventType]:
				result = eventHandler(argsList)
				if (result > 0):
					return result
		return 0

	# TODO: this probably needs to be more complex
	def _handleOnSaveEvent(self, eventType, argsList):
		"""Handles OnSave events by concatenating the results obtained
		from each handler to form an overall consolidated save string.
		"""
		result = ""
		if self.EventHandlerMap.has_key(eventType):
			for eventHandler in self.EventHandlerMap[eventType]:
				result = result + eventHandler(argsList)
		return result

	def _handleInitBugEvent(self, eventType, argsList):
		"""Initializes BUG before handling event normally."""
		initBug()
		self._handleDefaultEvent(eventType, argsList)
	
	
	def resetActiveTurn(self, argsList):
		self.iActiveTurn = -1
		self.bEndTurnFired = False
	
	def updateActiveTurn(self):
		"""Called from CvMainInterface.updateScreen() every 250 milliseconds."""
		iGameTurn = gc.getGame().getGameTurn()
		if self.iActiveTurn != iGameTurn:
			self.iActiveTurn = iGameTurn
			self.bEndTurnFired = False
			self.fireEvent("BeginActivePlayerTurn", gc.getGame().getActivePlayer(), iGameTurn)
	
	def updateEndTurn(self):
		"""Called from CvMainInterface.updateScreen() when end turn button is shown."""
		if not self.bEndTurnFired:
			self.bEndTurnFired = True
			self.fireEvent("endTurnReady", self.iActiveTurn)


	def onKbdEvent(self, argsList):
		"""Handles onKbdEvent by firing the keystroke's handler if it has one registered.
		"""
		eventType, key, mx, my, px, py = argsList
		if eventType == self.EventKeyDown:
			if not InputUtil.isModifier(key):
				stroke = InputUtil.Keystroke(key, self.bAlt, self.bCtrl, self.bShift)
				if stroke in self.shortcuts:
					BugUtil.debug("BugEventManager - calling handler for shortcut %s", stroke)
					self.shortcuts[stroke](argsList)
					return 1
		return 0
	
	def onPreGameStart(self, argsList):
		"""Fired from CvAppInterface.preGameStart()."""
		pass
	
	def onBeginActivePlayerTurn(self, argsList):
		"""Called when the active player can start their turn."""
		ePlayer, iGameTurn = argsList
	
	def onLanguageChanged(self, argsList):
		"""Called when the user changes their language selection."""
		iLanguage = argsList[0]

	def onResolutionChanged(self, argsList):
		"""Called when the user changes their graphics resolution."""
		iResolution = argsList[0]

EVENT_FUNCTION_MAP = {
	"kbdEvent": BugEventManager._handleConsumableEvent,
	"mouseEvent": BugEventManager._handleConsumableEvent,
	"OnSave": BugEventManager._handleOnSaveEvent,
	"OnLoad": BugEventManager._handleInitBugEvent,
	"PreGameStart": BugEventManager._handleInitBugEvent,
	#"GameStart": BugEventManager._handleInitBugEvent,
	#"windowActivation": BugEventManager._handleInitBugEvent,
}


def configure(logging=None, noLogEvents=None):
	"""Sets the global event manager's logging options."""
	if g_eventManager is not None:
		g_eventManager.setLogging(logging)
		g_eventManager.setNoLogEvents(noLogEvents)
	else:
		BugUtil.error("BugEventManager - BugEventManager not setup before configure()")

g_initDone = False
def initBug():
	"""Called once after Civ has initialized its data structures."""
	global g_initDone
	if not g_initDone:
		import BugInit
		if BugInit.init():
			g_initDone = True
