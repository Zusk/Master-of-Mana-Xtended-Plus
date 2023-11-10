## BugConfig
##
## Parses a BUG Config XML file into a collection of configuration objects
## including Mods, IniFiles, Options, Screens, Tabs and Events.
##
## TODO:
##  - Remove IniFile
##  X Move Mod to BugOptions/BugGame
##  - Add Mod.module; used as default wherever a module is required
##		- <init>
##		- <event>/<events>
##  - Same for screens and tabs
##  X Make sure inits run after options
##  ? Run events after options
##  - Change separator from "__" to "." (fix XML and tabs)
##
##  - Build an internal representation (partially done already) of the XML files
##    and then process everything in the correct order
##
## Copyright (c) 2008 The BUG Mod.
##
## Author: EmperorFool

# block error alert about xmllib deprecation
try:
	import sys
	stderr = sys.stderr
	sys.stderr = sys.stdout
	import xmllib
finally:
	sys.stderr = stderr

import BugCore
import BugInit
import BugOptions
import BugPath
import BugUtil
import FontUtil
import InputUtil
import CvEventInterface
import CvScreensInterface

MOD_OPTION_SEP = "__"

TRUE_STRINGS = ('true', 't', 'yes', 'y', 'on', '1')

TYPE_REPLACE = { "bool": "boolean",
			 	 "bit": "boolean",
			 	 "str": "string",
				 "integer": "int",
				 "long": "int",
				 "number": "int",
				 "real": "float",
				 "double": "float",
				 "decimal": "float",
				 "vector": "tuple",
				 "map": "dict",
				 "keys": "key" }
TYPE_DEFAULT = { "boolean": False,
			 	 "string": "",
				 "int": 0,
				 "float": 0.0,
				 "key": "",
				 "tuple": () }
TYPE_DEFAULT_FUNC = { "list": list,
					  "set": set,
					  "dict": dict }
TYPE_EVAL = { "boolean": lambda x: x.lower() in TRUE_STRINGS,
			  "string": lambda x: x,
			  "int": lambda x: int(x),
			  "float": lambda x: float(x),
			  "tuple": lambda x: eval("(%s,)" % x),
			  "list": lambda x: eval("[%s]" % x),
			  "set": lambda x: eval("set([%s])" % x),
			  "dict": lambda x: eval("{%s}" % x),
			  "key": lambda x: InputUtil.stringToKeystrokes(x) }

g_builder = None

def makeOptionId(modId, optionId):
	"""Concatenates the mod and option ID with a separator of the option ID
	doesn't already have one.
	
	Returns a fully qualified option ID.
	"""
	if optionId is not None and modId is not None:
		if optionId.find(MOD_OPTION_SEP) == -1:
			return modId + MOD_OPTION_SEP + optionId
	return optionId

class GameBuilder:
	"Builds each of the objects read by the XML parser."
	
	def __init__(self, game=None, attrs=None):
		if game is None:
			game = BugCore.game
		self.game = game
		self.eventManager = CvEventInterface.getEventManager()
		self.options = BugOptions.getOptions()
		self.optionsScreen = CvScreensInterface.getBugOptionsScreen()
		
		self.attrs = attrs
		self.mod = None
		self.iniFile = None
		self.section = None
		self.option = None
		self.screen = None
		self.symbol = None

	def createBuilder(self, module, clazz=None, attrs=None):
		if not clazz:
			clazz = module
		return BugUtil.callFunction(module, clazz, self.game, attrs)
	
	def loadMod(self, name):
		BugInit.loadMod(name)
	
	def createMod(self, id, name, author=None, url=None, version=None, build=None, releaseDate=None, attrs=None):
		#self.mod = Mod(id, name, author, url, version, build, releaseDate, attrs)
		self.mod = self.game._getMod(id)
		return self.mod
	
	
	def createIniFile(self, id, name, attrs=None):
		self.iniFile = BugOptions.IniFile(id, self.mod, name)
		self.iniFile.attrs = attrs
		self.options.addFile(self.iniFile)
		return self.iniFile
	
	def createSection(self, id, attrs=None):
		self.section = id
	
	
	def addOption(self, option, getter=None, setter=None, attrs=None):
		self.options.addOption(option)
		self.mod._addOption(option)
		option.attrs = attrs
		option.createAccessorPair(getter, setter)
	
	def createLinkedOption(self, id, linkId, getter=None, setter=None, attrs=None):
		id = makeOptionId(self.mod._id, id)
		linkId = makeOptionId(self.mod._id, linkId)
		option = self.options.getOption(linkId)
		if option is not None:
			link = option.createLinkedOption(self.mod, id)
			self.addOption(link, getter, setter, attrs)
			return link
		else:
			BugUtil.error("BugConfig - link option %s not found", linkId)
			return None
	
	def createOption(self, id, type, key=None, default=None, andId=None, 
					 title=None, tooltip=None, dirtyBit=None, getter=None, setter=None, 
					 attrs=None):
		if type == "color":
			return self.createOptionList(id, type, key, default, andId, type, None, None, title, tooltip, dirtyBit, getter, setter, attrs)
		id = makeOptionId(self.mod._id, id)
		andId = makeOptionId(self.mod._id, andId)
		if key is None:
			self.option = BugOptions.UnsavedOption(self.mod, id, type, 
												   default, andId, title, tooltip, dirtyBit)
		else:
			self.option = BugOptions.IniOption(self.mod, id, self.iniFile, self.section, key, 
											   type, default, andId, title, tooltip, dirtyBit)
		self.addOption(self.option, getter, setter, attrs)
		return self.option
	
	def createOptionList(self, id, type, key=None, default=None, andId=None, 
						 listType=None, values=None, format=None, 
						 title=None, tooltip=None, dirtyBit=None, getter=None, setter=None, 
						 attrs=None):
		id = makeOptionId(self.mod._id, id)
		andId = makeOptionId(self.mod._id, andId)
		if key is None:
			self.option = BugOptions.UnsavedListOption(self.mod, id, type, 
													   default, andId, listType, values, format, 
													   title, tooltip, dirtyBit)
		else:
			self.option = BugOptions.IniListOption(self.mod, id, self.iniFile, self.section, key, 
												   type, default, andId, listType, values, format, 
												   title, tooltip, dirtyBit)
		self.addOption(self.option, getter, setter, attrs)
		return self.option
	
	def createChangeDirtyBit(self, dirtyBit):
		self.option.addDirtyBit(dirtyBit)
	
	def createChangeFunction(self, module, function):
		self.option.addDirtyFunction(BugUtil.getFunction(module, function))
	
	def createChoice(self, id, getter=None, setter=None):
		self.option.addValue(id, getter, setter)
	
	def createAccessor(self, id, getter=None, setter=None, attrs=None):
		self.mod._createParameterizedAccessorPair(id, getter, setter)
	
	
	def createScreen(self, id, attrs=None):
		self.screen = OptionsScreen(id, attrs)
		self.game._addScreen(self.screen)
		return self.screen
	
	def createTab(self, screenId, id, module, clazz=None, attrs=None):
		# TODO: add args
		if screenId:
			screen = self.game._getScreen(screenId)
		else:
			screen = self.screen
		if not screen:
			raise BugUtil.ConfigError("<tab> must be in <screen> or have screenId")
		if not clazz:
			clazz = module
		tab = BugUtil.callFunction(module, clazz, self.optionsScreen)
		tab.attrs = attrs
		screen.addTab(tab)
		self.optionsScreen.addTab(tab)
		return tab
	
	
	def createInit(self, module, function="init", immediate=False, args=(), kwargs={}, attrs=None):
		func = BugUtil.getFunction(module, function, *args, **kwargs)
		if immediate:
			func()
		else:
			BugInit.addInit(module, func)
	
	def createEvent(self, eventType, module, function, args=(), kwargs={}, attrs=None):
		self.eventManager.addEventHandler(eventType, BugUtil.getFunction(module, function, *args, **kwargs))
	
	def createEvents(self, module, function=None, args=(), kwargs={}, attrs=None):
		if not function:
			function = module
		return BugUtil.callFunction(module, function, self.eventManager, *args, **kwargs)
	
	def createShortcut(self, key, module, function, args=(), kwargs={}, attrs=None):
		self.eventManager.addShortcutHandler(key, BugUtil.getFunction(module, function, *args, **kwargs))
	
	def createArgument(self, name, type=None, value=None, attrs=None):
		if type:
			type = type.lower()
			if type in TYPE_REPLACE:
				type = TYPE_REPLACE[type]
			if value is None:
				if type in TYPE_DEFAULT:
					return TYPE_DEFAULT[type]
				elif type in TYPE_DEFAULT_FUNC:
					return TYPE_DEFAULT_FUNC[type]()
				else:
					raise BugUtil.ConfigError("type %s requires a value" % type)
			else:
				value = value.replace("\n", " ")
				if type in TYPE_EVAL:
					return TYPE_EVAL[type](value)
		return eval(value)

	def createFontSymbol(self, key, fromSymbolOrKey=None, offset=None, name=None, attrs=None):
		if not fromSymbolOrKey:
			if not self.symbol:
				raise BugUtil.ConfigError("symbol %s requires an offset symbol" % key)
			fromSymbolOrKey = self.symbol
		if offset is None:
			offset = 1
		self.symbol = FontUtil.addOffsetSymbol(key, fromSymbolOrKey, offset, name)

def setGameBuilder(builder=None):
	global g_builder
	if builder is None:
		builder = GameBuilder(BugCore.game)
	g_builder = builder


class Mod:
	
	def __init__(self, id, name, author=None, url=None, 
				 version=None, build=None, releaseDate=None, attrs=None):
		self.id = id
		self.name = name
		self.author = author
		self.url = url
		self.version = version
		self.build = build
		self.releaseDate = releaseDate
		self.attrs = attrs


class OptionsScreen:
	
	def __init__(self, id, attrs=None):
		self.id = id
		self.attrs = attrs
		
		self.tabs = []
	
	def addTab(self, tab):
		self.tabs.append(tab)


ID = "id"

BUILDER = "builder"

LOAD = "load"

MOD = "mod"
NAME = "name"
AUTHOR = "author"
VERSION = "version"
BUILD = "build"
DATE = "date"
URL = "ürl"

OPTIONS = "options"
FILE = "file"

SECTION = "section"

ACCESSOR = "accessor"
GETTER = "get"
SETTER = "set"

OPTION = "option"
LINK = "link"
KEY = "key"
TYPE = "type"
DEFAULT = "default"
AND = "and"
TITLE = "label"
TOOLTIP = "help"
DIRTYBIT = "dirtyBit"

LIST = "list"
LISTTYPE = "listType"
VALUES = "values"
FORMAT = "format"

CHOICE = "choice"
CHANGE = "change"

SCREEN = "screen"
TAB = "tab"

INIT = "init"
IMMEDIATE = "immediate"
EVENT = "event"
EVENTS = "events"
SHORTCUT = "shortcut"

ARG = "arg"
VALUE = "value"

SYMBOL = "symbol"
FROM = "from"
OFFSET = "offset"

MODULE = "module"
CLASS = "class"
FUNCTION = "function"

class XmlParser(xmllib.XMLParser):

	def __init__(self):
		xmllib.XMLParser.__init__(self)
		self.game = BugCore.game
		self.mod = None
		self.module = None
		self.iniFile = None
		self.option = None
		self.saving = False
		self.savedText = None
		self.arg_container_attrs = None
		self.arg_attrs = None
		self.args = None
		self.kwargs = None
		setGameBuilder(g_builder)

	def parse(self, path):
		try:
			file = open(path)
			try:
				while True:
					s = file.read(512)
					if not s:
						break
					self.feed(s)
			finally:
				file.close()
		except IOError:
			BugUtil.trace("BugConfig - IOError parsing %s" % path)
		except BugUtil.ConfigError:
			BugUtil.trace("BugConfig - failure parsing %s at line %d" % (path, self.lineno))
		except:
			BugUtil.trace("BugConfig - failure parsing %s at line %d" % (path, self.lineno))
			raise
		self.close()
	
	def syntax_error(self, message):
		raise BugUtil.ConfigError("error parsing XML: %s" % message)
	
	def unknown_starttag(self, tag, attrs):
		BugUtil.warn("BugConfig - unknown XML start tag %s with %d attributes", tag, len(attrs))
	
	def unknown_endtag(self, tag):
		BugUtil.warn("BugConfig - unknown XML end tag %s", tag)
	
	def handle_data(self, data):
		if self.saving:
			self.savedText += data
	
	
	def start_builder(self, attrs):
		module = self.getModuleAttribute(attrs, BUILDER)
		clazz = self.getAttribute(attrs, CLASS, module)
		builder = g_builder.createBuilder(module, clazz, attrs)
		if builder:
			setGameBuilder(builder)
	
	def end_builder(self):
		pass
	
	
	def start_load(self, attrs):
		name = self.getRequiredAttribute(attrs, MOD, LOAD)
		g_builder.loadMod(name)
	
	def end_load(self):
		pass
	
	
	def start_mod(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, MOD)
		name = self.getAttribute(attrs, NAME)
		author = self.getAttribute(attrs, AUTHOR)
		version = self.getAttribute(attrs, VERSION)
		build = self.getAttribute(attrs, BUILD)
		date = self.getAttribute(attrs, DATE)
		url = self.getAttribute(attrs, URL)
		self.module = self.getAttribute(attrs, MODULE)
		self.mod = g_builder.createMod(id, name, author, url, 
									   version, build, date, attrs)
	
	def end_mod(self):
		self.mod._initDone()
		self.game._addMod(self.mod)
	
	
	def start_options(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, OPTIONS)
		file = self.getRequiredAttribute(attrs, FILE, OPTIONS)
		self.iniFile = g_builder.createIniFile(id, file, attrs)
	
	def end_options(self):
		if self.iniFile:
			self.iniFile.read()
	
	def start_section(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, SECTION)
		section = g_builder.createSection(id)
	
	def end_section(self):
		pass
	
	
	def start_option(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, OPTION)
		link = self.getAttribute(attrs, LINK)
		if link:
			getter = self.getAttribute(attrs, GETTER)
			setter = self.getAttribute(attrs, SETTER)
			option = g_builder.createLinkedOption(id, link, getter, setter, attrs)
		else:
			type = self.getRequiredAttribute(attrs, TYPE, OPTION)
			key = self.getAttribute(attrs, KEY)
			default = self.getAttribute(attrs, DEFAULT)
			andId = self.getAttribute(attrs, AND)
			dirtyBit = self.getAttribute(attrs, DIRTYBIT)
			title = self.getAttribute(attrs, TITLE)
			tooltip = self.getAttribute(attrs, TOOLTIP)
			getter = self.getAttribute(attrs, GETTER)
			setter = self.getAttribute(attrs, SETTER)
			option = g_builder.createOption(id, type, key, default, andId, title, tooltip, dirtyBit, getter, setter, attrs)
	
	def end_option(self):
		pass
	
	def start_change(self, attrs):
		dirtyBit = self.getAttribute(attrs, DIRTYBIT)
		if dirtyBit:
			g_builder.createChangeDirtyBit(dirtyBit)
		else:
			module = self.getModuleAttribute(attrs, CHANGE)
			function = self.getRequiredAttribute(attrs, FUNCTION, CHANGE)
			g_builder.createChangeFunction(module, function)
	
	def end_change(self):
		pass
	
	def start_list(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, LIST)
		type = self.getAttribute(attrs, TYPE)
		key = self.getAttribute(attrs, KEY)
		if type is None:
			listType = self.getRequiredAttribute(attrs, LISTTYPE, LIST)
		else:
			listType = self.getAttribute(attrs, LISTTYPE)
		default = self.getAttribute(attrs, DEFAULT)
		andId = self.getAttribute(attrs, AND)
		values = self.getAttribute(attrs, VALUES)
		format = self.getAttribute(attrs, FORMAT)
		dirtyBit = self.getAttribute(attrs, DIRTYBIT)
		title = self.getAttribute(attrs, TITLE)
		tooltip = self.getAttribute(attrs, TOOLTIP)
		getter = self.getAttribute(attrs, GETTER)
		setter = self.getAttribute(attrs, SETTER)
		self.option = g_builder.createOptionList(id, type, key, default, andId, listType, values, format, title, tooltip, dirtyBit, getter, setter, attrs)
	
	def end_list(self):
		self.option.createComparers()
	
	def start_choice(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, CHOICE)
		getter = self.getAttribute(attrs, GETTER)
		setter = self.getAttribute(attrs, SETTER)
		g_builder.createChoice(id, getter, setter)
	
	def end_choice(self):
		pass
	
	def start_accessor(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, ACCESSOR)
		getter = self.getRequiredAttribute(attrs, GETTER, ACCESSOR)
		setter = self.getAttribute(attrs, SETTER)
		g_builder.createAccessor(id, getter, setter, attrs)
	
	def end_accessor(self):
		pass
	
	
	def start_screen(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, SCREEN)
		screen = g_builder.createScreen(id, attrs)
	
	def end_screen(self):
		pass

	def start_tab(self, attrs):
		screenId = self.getAttribute(attrs, SCREEN)
		module = self.getModuleAttribute(attrs, TAB)
		clazz = self.getAttribute(attrs, CLASS, module)
		id = self.getAttribute(attrs, ID, module)
		tab = g_builder.createTab(screenId, id, module, clazz, attrs)
	
	def end_tab(self):
		pass
	
	
	def start_init(self, attrs):
		self.startArgs(attrs)
	
	def end_init(self):
		attrs, args, kwargs = self.endArgs()
		module = self.getModuleAttribute(attrs, INIT)
		function = self.getAttribute(attrs, FUNCTION, INIT)
		immediate = self.getAttribute(attrs, IMMEDIATE, INIT)
		if immediate:
			immediate = immediate.lower() in TRUE_STRINGS
		else:
			immediate = False
		g_builder.createInit(module, function, immediate, args, kwargs, attrs)
	
	def start_event(self, attrs):
		self.startArgs(attrs)
	
	def end_event(self):
		attrs, args, kwargs = self.endArgs()
		eventType = self.getRequiredAttribute(attrs, TYPE, EVENT)
		module = self.getModuleAttribute(attrs, EVENT)
		function = self.getRequiredAttribute(attrs, FUNCTION, EVENT)
		g_builder.createEvent(eventType, module, function, args, kwargs, attrs)
	
	def start_events(self, attrs):
		self.startArgs(attrs)
	
	def end_events(self):
		attrs, args, kwargs = self.endArgs()
		module = self.getModuleAttribute(attrs, EVENTS)
		function = self.getAttribute(attrs, FUNCTION, module)
		clazz = self.getAttribute(attrs, CLASS, function)
		g_builder.createEvents(module, clazz, args, kwargs, attrs)
	
	def start_shortcut(self, attrs):
		self.startArgs(attrs)
	
	def end_shortcut(self):
		attrs, args, kwargs = self.endArgs()
		key = self.getRequiredAttribute(attrs, KEY, SHORTCUT)
		module = self.getModuleAttribute(attrs, SHORTCUT)
		function = self.getRequiredAttribute(attrs, FUNCTION, SHORTCUT)
		g_builder.createShortcut(key, module, function, args, kwargs, attrs)
	
	
	def start_arg(self, attrs):
		self.arg_attrs = attrs
		value = self.getAttribute(attrs, VALUE)
		if value is None:
			self.startText()
	
	def end_arg(self):
		name = self.getAttribute(self.arg_attrs, NAME)
		type = self.getAttribute(self.arg_attrs, TYPE)
		value = self.getAttribute(self.arg_attrs, VALUE)
		if value is None:
			value = self.endText()
		arg = g_builder.createArgument(name, type, value)
		BugUtil.debug("BugConfig - %s argument %s = %r", type, name, arg)
		if arg is not None:
			if name:
				self.kwargs[name] = arg
			else:
				self.args.append(arg)
	
	def start_symbol(self, attrs):
		id = self.getRequiredAttribute(attrs, ID, SYMBOL)
		fromSymbolOrKey = self.getAttribute(attrs, FROM)
		offset = self.getAttribute(attrs, OFFSET)
		if offset:
			offset = int(offset)
		name = self.getAttribute(attrs, NAME)
		symbol = g_builder.createFontSymbol(id, fromSymbolOrKey, offset, name, attrs)
	
	def end_symbol(self):
		pass
	
	
	def getAttribute(self, attrs, key, default=None):
		if key in attrs:
			return attrs[key]
		else:
			return default
	
	def getRequiredAttribute(self, attrs, key, tag, default=None):
		if key in attrs:
			return attrs[key]
		elif default:
			return default
		else:
			raise BugUtil.ConfigError("<%s>.%s is required" % (tag, key))
	
	def getModuleAttribute(self, attrs, tag):
		return self.getRequiredAttribute(attrs, MODULE, tag, self.module)
	
	def startText(self):
		if self.saving:
			BugUtil.error("BugConfig - already saving text in XML parser")
		self.saving = True
		self.savedText = ""
	
	def endText(self):
		if not self.saving:
			BugUtil.error("BugConfig - not saving text in XML parser")
			return ""
		self.saving = False
		return self.savedText
	
	def startArgs(self, attrs):
		self.arg_container_attrs = attrs
		self.args = []
		self.kwargs = {}
	
	def endArgs(self):
		return self.arg_container_attrs, self.args, self.kwargs


"""
Regexps for options Python -> XML

<option>
self.addOption\(Option\("[A-Za-z0-9]+_([^"]+)",[ \t\r\n]*"([^"]+)", "([^"]+)", ([^,]+),[ \t\r\n]*"([^"]+)",[ \t\r\n]*"([^"]+)",[ \t\r\n]*InterfaceDirtyBits.([^\n\r]+)_DIRTY_BIT\)\)
<option id="$1" key="$3" \R\t\t\t\t\ttype="boolean" default="$4" \R\t\t\t\t\tget="" set="" dirtyBit="$7" \R\t\t\t\t\tlabel="$5" \R\t\t\t\t\thelp="$6"/>
<option id="$1" key="$3" \R\t\t\t\t\ttype="boolean" default="$4" \R\t\t\t\t\tget="" set="" dirtyBit="$7"/>

<option> no dirty bit
self.addOption\(Option\("[A-Za-z0-9]+_([^"]+)",[ \t\r\n]*"([^"]+)", "([^"]+)", ([^,]+),[ \t\r\n]*"([^"]+)",[ \t\r\n]*"([^"]+)"\)\)
<option id="$1" key="$3" \R\t\t\t\t\ttype="boolean" default="$4" \R\t\t\t\t\tget="" set="" \R\t\t\t\t\tlabel="$5" \R\t\t\t\t\thelp="$6"/>
<option id="$1" key="$3" \R\t\t\t\t\ttype="boolean" default="$4" \R\t\t\t\t\tget=""/>

<list>
self.addOption\(OptionList\("[A-Za-z0-9]+_([^"]+)",[ \t\r\n]*"([^"]+)", "([^"]+)", ([^,]+),[ \t\r\n]*"([^"]+)",[ \t\r\n]*"([^"]+)",[ \t\r\n]*\[([^]]+)\],[ \t\r\n]*"([^"]+)",[ \t\r\n]*InterfaceDirtyBits.([^\n\r]+)_DIRTY_BIT\)\)
<list   id="$1" key="$3" \R\t\t\t\t\ttype="int" default="$4" listType="int" \R\t\t\t\t\tvalues="$7" format="$8" \R\t\t\t\t\tget="" set="" dirtyBit="$9" \R\t\t\t\t\tlabel="$5" \R\t\t\t\t\thelp="$6"/>

<list> no dirty bit
self.addOption\(OptionList\("[A-Za-z0-9]+_([^"]+)",[ \t\r\n]*"([^"]+)", "([^"]+)", ([^,]+),[ \t\r\n]*"([^"]+)",[ \t\r\n]*"([^"]+)",[ \t\r\n]*\[([^]]+)\],[ \t\r\n]*"([^"]+)"\)\)
<list   id="$1" key="$3" \R\t\t\t\t\ttype="int" default="$4" listType="int" \R\t\t\t\t\tvalues="$7" format="$8" \R\t\t\t\t\tget="" set="" \R\t\t\t\t\tlabel="$5" \R\t\t\t\t\thelp="$6"/>

<list> no format
self.addOption\(OptionList\("[A-Za-z0-9]+_([^"]+)",[ \t\r\n]*"([^"]+)", "([^"]+)", ([^,]+),[ \t\r\n]*"([^"]+)",[ \t\r\n]*"([^"]+)",[ \t\r\n]*\[([^]]+)\], None,[ \t\r\n]*InterfaceDirtyBits.([^\n\r]+)_DIRTY_BIT\)\)
<list   id="$1" key="$3" \R\t\t\t\t\ttype="int" default="$4" listType="string" \R\t\t\t\t\tvalues="$7" \R\t\t\t\t\tget="" set="" dirtyBit="$8" \R\t\t\t\t\tlabel="$5" \R\t\t\t\t\thelp="$6"/>
<list   id="$1" key="$3" \R\t\t\t\t\ttype="int" default="$4" listType="string" \R\t\t\t\t\tvalues="$7" \R\t\t\t\t\tget="" set="" dirtyBit="$8"/>

<list> no format or dirty bit
self.addOption\(OptionList\("[A-Za-z0-9]+_([^"]+)",[ \t\r\n]*"([^"]+)", "([^"]+)", ([^,]+),[ \t\r\n]*"([^"]+)",[ \t\r\n]*"([^"]+)",[ \t\r\n]*\[([^]]+)\](, None)?\)\)
<list   id="$1" key="$3" \R\t\t\t\t\ttype="int" default="$4" listType="string" \R\t\t\t\t\tvalues="$7" \R\t\t\t\t\tget="" set="" \R\t\t\t\t\tlabel="$5" \R\t\t\t\t\thelp="$6"/>

accessors
get="(is|get)([^"]+)"
get="$1$2" set="set$2"
"""
