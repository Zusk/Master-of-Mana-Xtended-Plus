## BugUtil
##
## Collection of general-purpose utility functions.
##
## Text Formatting
##
##   - getPlainText(key, default), getText(key, values, default)
##     Retrieves a translated <TEXT> message from CyTranslator. If the key doesn't
##     exist, the default (if given) is returned. If no default is given, an
##     error message is returned.
##
##   - colorText(text, color)
##     Accepts both a string (e.g. "COLOR_RED") or integer color.
##
##   - formatFloat(number, decimals)
##     Returns number formatted as a string with <decimals> number of digits
##     after the decimal point (default 0).
##
## Logging
##
##   See "BUG Core.xml" for options. They take effect during initialization.
##   Until then, the defaults in this module are used.
##
##   - alert(message, args...)
##     Displays a formatted message on-screen. Use this sparingly as it's lame.
##
##   - debug(message, args...) 
##     info(message, args...) 
##     warn(message, args...)
##     error(message, args...)
##     Logs a message with optional arguments (percent substitutions) to the
##     screen and/or the debug file depending on the current level cutoffs.
##     e.g. BugUtil.warn("TradeUtil - unknown item type %d", trade.ItemType)
##
##   - trace(message, detail?, stack?)
##     Logs an exception message with optional detail and stacktrace (default on)
##
## Event Tracing (handleInput)
##
##   - debugInput(inputClass, flags?)
##     Logs a DEBUG message detailing the input event.
##     If flags is given and True, calls debugInputFlags() as well.
##
##   - debugInputFlags(inputClass)
##     Logs a DEBUG message detailing the input event's flags, if any.
##
## Timing Code Execution
##
##   - Timer class
##     Times single and multiple blocks of code and provides methods for logging
##     the individual and total times at DEBUG level.
##
## Binding and Calling Functions Dynamically
##
##   - getFunction(module, functionOrClass, bind?, args..., kwargs...)
##     Returns a Function object that can be used to dynamically call a function
##     or class constructor at a later time with the arguments provided when
##     the Function was created.
##
##   - callFunction(module, functionOrClass, args..., kwargs...)
##     Returns the result of calling the function bound using getFunction().
##
## Exception Classes
##
##   - BugError, ConfigError
##     Raise these in BUG Core code rather than generic exceptions.
##
## Civ4 Helpers (move these)
##
##   - isNoEspionage()
##     Returns True if running at least 3.17 and the No Espionage option is set
##     for the game in progress.
##
## Copyright (c) 2008 The BUG Mod.
##
## Author: EmperorFool

from CvPythonExtensions import *
import sys
import time
import traceback
import types
import ColorUtil

gc = CyGlobalContext()
localText = CyTranslator()
interface = CyInterface()

## Getting translated text from CIV4GameText XML files and general formatting

def getPlainText(key, default=None):
	"""
	Looks up a translated message in XML without any replacement parameters.
	If the key isn't found, the default is returned.
	"""
	return getText(key, (), default)

def getText(key, values=(), default=None):
	"""
	Looks up a translated message in XML with a tuple of replacement parameters.
	It is safe to pass in a single value instead of tuple/list.
	If the key isn't found, the default is returned.
	"""
	if values is None:
		values = ()
	elif not isinstance(values, tuple) and not isinstance(values, list):
		values = (values,)
	text = localText.getText(key, values)
	if (text and text != key):
		return text
	else:
		if default is not None:
			return default
		else:
			debug("BugUtil.getText - XML key %s not found", key)
			return "XML key %s not found" % key

def colorText(text, color):
	"""Applies the color (string or int) to text and returns the resulting string."""
	if text is not None and color is not None:
		if isinstance(color, types.StringTypes):
			color = ColorUtil.keyToType(color)
		if color >= 0:
			return localText.changeTextColor(text, color)
	return text

def formatFloat(number, decimals=0):
	"""
	Formats value as a floating point number with decimals digits in the mantissa
	and returns the resulting string.
	"""
	if decimals <= 0:
		return "%f" % number
	else:
		return ("%." + str(decimals) + "f") % number


## Logging to the Screen and Debug File

DEBUG = 0
INFO = 1
WARN = 2
ERROR = 3

LEVEL_PREFIXES = (
	"DEBUG: ",
	"INFO : ",
	"WARN : ",
	"ERROR: ",
)

screenLogLevel = ERROR
fileLogLevel = DEBUG
minimumLogLevel = min(screenLogLevel, fileLogLevel)

logTime = True

def alert(message, *args):
	logToScreen(message % args)

def trace(message, detail=True, stack=True):
	# TODO: register with ConfigTracker
	logToFile("TRACE: " + message)
	if detail:
		logToFile("  " + str(sys.exc_info()[1]))
	if stack:
		traceback.print_exc()

def debug(message, *args):
	"""Logs a message at DEBUG level."""
	log(DEBUG, message, args)

def info(message, *args):
	"""Logs a message at INFO level."""
	log(INFO, message, args)

def warn(message, *args):
	"""Logs a message at WARN level."""
	log(WARN, message, args)

def error(message, *args):
	"""Logs a message at ERROR level."""
	log(ERROR, message, args)

def log(level, message, args=()):
	"""
	Logs a message on-screen and/or to a file depending on the current levels.
	
	The message is sent to each if the level is at least that of the destination.
	The level of the message is prepended to the message, and if logTime is True, 
	the current time in HH:MM:SS format is prepended as well.
	"""
	if level >= minimumLogLevel:
		if args:
			message = message % args
		if level >= screenLogLevel:
			logToScreen(message)
		if level >= fileLogLevel:
			logToFile(LEVEL_PREFIXES[level] + message)

def logToScreen(message):
	interface.addImmediateMessage(message, "")

def logToFile(message):
	if logTime:
		message = time.asctime()[11:20] + message
	sys.stdout.write(message + "\n")

def readLoggingOptions(option=None, value=None):
	"""
	Pulls the logging options from the BUG Core options and stores into local copies.
	Done this way to avoid hitting the options in tight loops that use logging.
	"""
	import BugCore
	CoreOpt = BugCore.game.Core
	global screenLogLevel, fileLogLevel, minimumLogLevel, logTime
	screenLogLevel = CoreOpt.getScreenLogLevel()
	fileLogLevel = CoreOpt.getFileLogLevel()
	minimumLogLevel = min(screenLogLevel, fileLogLevel)
	logTime = CoreOpt.isLogTime()


## Event Tracking and Output

INPUT_CODES = {
	NotifyCode.NOTIFY_MOUSEMOVE             : "Mouse Move", 
	NotifyCode.NOTIFY_MOUSEWHEELDOWN        : "Mouse Wheel Down", 
	NotifyCode.NOTIFY_MOUSEWHEELUP          : "Mouse Wheel Up", 
	NotifyCode.NOTIFY_CURSOR_MOVE_ON        : "Mouse Enter", 
 	NotifyCode.NOTIFY_CURSOR_MOVE_OFF       : "Mouse Leave", 
    NotifyCode.NOTIFY_CLICKED               : "Click",
    NotifyCode.NOTIFY_DBL_CLICKED           : "Double Click",
    
	NotifyCode.NOTIFY_CHARACTER             : "Character", 
    
    NotifyCode.NOTIFY_TABLE_HEADER_SELECTED : "Table Header Select",
    NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED : "List Select",
    NotifyCode.NOTIFY_SCROLL_DOWN           : "Scroll Down",
    NotifyCode.NOTIFY_SCROLL_UP             : "Scroll Up",
    
    NotifyCode.NOTIFY_NEW_HORIZONTAL_STOP   : "New Horizontal Stop",
    NotifyCode.NOTIFY_NEW_VERTICAL_STOP     : "New Vertical Stop",
    NotifyCode.NOTIFY_SLIDER_NEWSTOP        : "Slider New Stop",
    
    NotifyCode.NOTIFY_FOCUS                 : "Focus",
    NotifyCode.NOTIFY_UNFOCUS               : "Unfocus",
    
    NotifyCode.NOTIFY_LINKEXECUTE           : "Link Execute",
    NotifyCode.NOTIFY_FLYOUT_ITEM_SELECTED  : "Flyout Item Selected",
    NotifyCode.NOTIFY_MOVIE_DONE            : "Movie Done",
}

def debugInput(inputClass, flags=False):
	"""
	Prints a debug message detailing the given input event.
	
	Add this to the handleInput function to see all events as they occur.
	Pass True for flags to output the mouse flags, if any.
	
	def handleInput(self, inputClass):
		BugUtil.debugInput(inputClass)
	"""
	if inputClass.getNotifyCode() in INPUT_CODES:
		debug("%s - %s #%d, data %d, widget %d %d %d",
			INPUT_CODES[inputClass.getNotifyCode()], 
			inputClass.getFunctionName(),
			inputClass.getID(), 
			inputClass.getData(),
			inputClass.getButtonType(),
			inputClass.getData1(),
			inputClass.getData2()
		)
	if flags:
		debugInputFlags(inputClass)

MOUSE_FLAGS = {
	MouseFlags.MOUSE_CLICKED:			"Click",
	MouseFlags.MOUSE_CONTROL:			"Control",
	MouseFlags.MOUSE_DBLCLICKED:		"DblClick",
	MouseFlags.MOUSE_EVENT:				"Event",
	MouseFlags.MOUSE_LBUTTON:			"L-Click",
	MouseFlags.MOUSE_LBUTTONDBLCLK:		"L-DblClick",
	MouseFlags.MOUSE_LBUTTONDOWN:		"L-Down",
	MouseFlags.MOUSE_LBUTTONUP:			"L-Up",
	MouseFlags.MOUSE_MBUTTON:			"M-Click",
	MouseFlags.MOUSE_MBUTTONDBLCLK:		"M-DblClick",
	MouseFlags.MOUSE_MBUTTONDOWN:		"M-Down",
	MouseFlags.MOUSE_MBUTTONUP:			"M-Up",
	MouseFlags.MOUSE_MOUSEMOVE:			"Move",
	MouseFlags.MOUSE_MOUSEWHEELDOWN: 	"W-Down",
	MouseFlags.MOUSE_MOUSEWHEELUP:		"W-Up",
	MouseFlags.MOUSE_RBUTTON:			"R-Click",
	MouseFlags.MOUSE_RBUTTONDBLCLK:		"R-DblClick",
	MouseFlags.MOUSE_RBUTTONDOWN:		"R-Down",
	MouseFlags.MOUSE_RBUTTONUP:			"R-Up",
	MouseFlags.MOUSE_RELEASED:			"Release",
	MouseFlags.MOUSE_SHIFT:				"Shift",
	MouseFlags.MOUSE_STATE:				"State",
	MouseFlags.MOUSE_XBUTTON1:			"X1-Click",
	MouseFlags.MOUSE_XBUTTON2:			"X2-Click",
	MouseFlags.MOUSE_XBUTTONDBLCLK:		"X-DblClick",
	MouseFlags.MOUSE_XBUTTONDOWN:		"X-Down",
	MouseFlags.MOUSE_XBUTTONUP:			"X-Up",
}

def debugInputFlags(inputClass):
	"""
	Prints a debug message detailing the given input event's mouse flags, if any.
	
	def handleInput(self, inputClass):
		BugUtil.debugInputFlags(inputClass)
	"""
	flags = inputClass.getFlags()
	if flags:
		flagList = []
		for flag, text in MOUSE_FLAGS.iteritems():
			if flags & flag:
				flagList.append(text)
		if flagList:
			debug("Flags - %s", ", ".join(flagList))


## Timing Code Execution

class Timer:
	"""
	Stopwatch for timing code execution and logging the results.
	
	timer = BugUtil.Timer('function')
	... code to time ...
	timer.log()
	
	In a loop, log() will display each iteration's time. Since Timers are started
	when created, call reset() before entering the loop or pass in False.
	Use logTotal() at the end if you want to see the sum of all iterations.
	
	timer = BugUtil.Timer('draw loop', False)
	for/while ...
		timer.start()
		... code to time ...
		timer.log()
	timer.logTotal()
	
	A single Timer can be reused for timing loops without creating a new Timer
	for each iteration by calling restart().
	"""
	def __init__(self, item, start=True):
		"""Starts the timer."""
		self._item = item
		self.reset()
		if start:
			self.start()
	
	def reset(self):
		"""Resets all times to zero and stops the timer."""
		self._initial = None
		self._start = None
		self._time = 0
		self._total = 0
		return self
	
	def start(self):
		"""Starts the timer or starts it again if it is already running."""
		self._start = time.clock()
		if self._initial is None:
			self._initial = self._start
		return self
	
	def restart(self):
		"""Resets all times to zero and starts the timer."""
		return self.reset().start()
	
	def stop(self):
		"""
		Stops the timer if it is running and returns the elapsed time since start,
		otherwise returns 0.
		"""
		if self.running():
			self._final = time.clock()
			self._time = self._final - self._start
			self._total += self._time
			self._start = None
			return self._time
		return 0
	
	def running(self):
		"""Returns True if the timer is running."""
		return self._start is not None
	
	def time(self):
		"""Returns the most recent timing or 0 if none has completed."""
		return self._time
	
	def total(self):
		"""Returns the sum of all the individual timings."""
		return self._total
	
	def span(self):
		"""Returns the span of time from the first start() to the last stop()."""
		if self._initial is None:
			warn("called span() on a Timer that has not been started")
			return 0
		elif self._final is None:
			return time.clock() - self._initial
		else:
			return self._final - self._initial
	
	def log(self, extra=None):
		"""
		Stops the timer and logs the time of the current timing.
		
		This is the same as calling logTotal() or logSpan() for the first time.
		"""
		self.stop()
		return self._log(self.time(), extra)
	
	def logTotal(self, extra="total"):
		"""
		Stops the timer and logs the sum of all timing steps.
		
		This is the same as calling log() or logSpan() for the first time.
		"""
		self.stop()
		return self._log(self.total(), extra)
	
	def logSpan(self, extra=None):
		"""
		Stops the timer and logs the span of time covering all timings.
		
		This is the same as calling log() or logTotal() for the first time.
		"""
		self.stop()
		return self._log(self.span(), extra)
	
	def _log(self, runtime, extra):
		"""Logs the passed in runtime value."""
		if extra is None:
			debug("Timer - %s took %d ms" % (self._item, 1000 * runtime))
		else:
			debug("Timer - %s [%s] took %d ms" % (self._item, str(extra), 1000 * runtime))
		return self


## Binding and Calling Functions Dynamically
##
## (looking up module and function/class by name rather than directly in Python
## and passing in arguments set up at time of creation or when called)

class Function:
	
	def __init__(self, module, functionOrClass, *args, **kwargs):
		self.module = module
		self.functionOrClass = functionOrClass
		self.function = None
		self.setArguments(*args, **kwargs)
	
	def bind(self):
		try:
			if self.function is None:
				debug("BUG: binding %s.%s" % (self.module, self.functionOrClass))
				self.function = getattr(__import__(self.module), self.functionOrClass)
		except ImportError:
			raise ConfigError("No such module '%s'" % self.module)
		except AttributeError:
			raise ConfigError("Module '%s' must define function or class '%s'" % (self.module, self.functionOrClass))
	
	def setArguments(self, *args, **kwargs):
		self.args = args
		self.kwargs = kwargs
	
	def call(self, *args, **kwargs):
		self.bind()
		if args or kwargs:
			self.setArguments(*args, **kwargs)
		debug("BUG: calling %r" % self)
		return self.function(*self.args, **self.kwargs)
	
	def __call__(self, *args, **kwargs):
		return self.call(*args, **kwargs)
	
	def __repr__(self):
		if self.args or self.kwargs:
			return "<func %s.%s (%r, %r)>" % \
		   	   	   (self.module, self.functionOrClass, self.args, self.kwargs)
		else:
			return "<func %s.%s>" % \
		   	   	   (self.module, self.functionOrClass)

def getFunction(module, functionOrClass, bind=False, *args, **kwargs):
	func = Function(module, functionOrClass, *args, **kwargs)
	if bind:
		func.bind()
	return func

def callFunction(module, functionOrClass, *args, **kwargs):
	func = getFunction(module, functionOrClass, True)
	return func(*args, **kwargs)


## Exception Classes

class BugError(Exception):
	"""Generic BUG-related error."""
	def __init__(self, message):
		Exception.__init__(self, message)

class ConfigError(BugError):
	"""
	Error related to configuration problems.
	
	These are caught and reported during BUG initialization, and initizliation
	is allowed to continue so most problems can be reported at once. This may
	result in false-positive errors being reported.
	"""
	def __init__(self, message):
		BugError.__init__(self, message)


## Civ4 Helpers

def isNoEspionage():
	"""Returns True if using at least 3.17 and the 'No Espionage' option is enabled."""
	try:
		return gc.getGame().isOption(GameOptionTypes.GAMEOPTION_NO_ESPIONAGE)
	except:
		return False
