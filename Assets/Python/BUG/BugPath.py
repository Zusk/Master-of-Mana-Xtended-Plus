## BugPath
##
## Helps locate INI and other setup files for the BUG Mod.
## Based on CvPath by Dr. Elmer Jiggles.
##
## Copyright (c) 2008 The BUG Mod.
##
## Author: EmperorFool

# This module locates INI and other configuration files and directories.
# It exposes several useful variables:
#
# modName
#   Pulled from CvModName
#
# modFolder
#   Contains the name of the loaded mod, or None if no mod is loaded.
#
# userDir
#   Typically the "My Documents\My Games" folder on Windows and the "Documents"
#   folder on MacOS X. Both are in the user's private documents area.
#
# rootDir
#   Full path to directory containing the CivilizationIV.ini file.
#   Can be overridden with the CvAltRoot module.
#
# appDir
#   Full path to directory containing the Civ4 executable (CIV4BeyondSword.exe on Windows)
#
# appName
#   The final component in appDir, typically "Beyond the Sword"
#
# iniFileSearchPaths
#   A set of existing directories in which this module searches for configuration
#   files and directories.
#
#   The preference order is
#
#     1. <user-dir>\<mod-name>
#     2. <user-dir>\<app-name>\Mods\<mod-name>
#     3. <user-dir>\<app-name>\<mod-name>
#     3. <user-dir>\<app-name>
#     4. <root-dir>\<app-name>\Mods\<mod-name>
#     5. <root-dir>\<app-name>\<mod-name>
#     5. <root-dir>\<app-name>
#
# mainModIniDir
#   If the main INI file is loaded, this holds the directory where it was found.
#   It will be one of the directories in iniFileSearchPaths. It can be used as a
#   preference for where to store new files.
#
# All directory variables exposed are guaranteed to be existing directories when
# they were found. Otherwise they will be the Python None value.

import os
import os.path
import sys
import BugConfigTracker
import BugUtil

try:
	import CvModName
	modName = CvModName.modName
except:
	modName = None

def getModFolder():
	initModFolder()
	return modFolder

modDir = None
modFolder = None
modFolderInitDone = False
def initModFolder():
	global modDir
	global modFolder
	global modFolderInitDone
	if modFolderInitDone:
		return
	BugUtil.debug("BugPath - initializing modFolder")
	try:
		from CvPythonExtensions import CyReplayInfo
		replay = CyReplayInfo()
		replay.createInfo(0)
		modDir = replay.getModName()
		#BugUtil.debug("BugPath - modDir is %s", modDir)
		if modDir:
			modDir = os.path.normpath(modDir)
			modFolder = os.path.basename(modDir)
	except:
		BugUtil.warn("BugPath - replay not ready")
		try:
			import CvModFolder
			modFolder = CvModFolder.modFolder
		except:
			modFolder = None
	if modFolder:
		BugUtil.debug("BugPath - mod is %s", modFolder)
	else:
		BugUtil.debug("BugPath - no mod found")
	modFolderInitDone = True

rootDir = None
appDir = None
appName = "Beyond the Sword"
userDir = None
systemFoldersInitDone = False
def initSystemFolders():
	global systemFoldersInitDone
	if systemFoldersInitDone:
		return
	global rootDir, appDir, appName, userDir
	try:
		import CvAltRoot
		if os.path.isdir(CvAltRoot.rootDir):
			if os.path.isfile(os.path.join(CvAltRoot.rootDir, "CivilizationIV.ini")):
				rootDir = CvAltRoot.rootDir
				BugUtil.debug("BugPath - overriding rootDir")
			else:
				BugUtil.error("Cannot find CivilizationIV.ini in directory from CvAltRoot.py")
		else:
			BugUtil.error("Directory from CvAltRoot.py is not valid")
	except:
		pass
	
	if rootDir:
		userDir = os.path.dirname(rootDir)
	else:
		# Determine the base user directory (e.g. "C:\...\My Documents\My Games").
		userDir = None
		if (sys.platform == 'darwin'):
			"Mac OS X"
			userDir = os.path.join(os.environ['HOME'], "Documents")
		else:
			import _winreg
			"Windows"
			def __getRegValue(root, subkey, name):
				key = _winreg.OpenKey(root, subkey)
				try:
					value = _winreg.QueryValueEx(key, name)
					return value[0]
				finally:
					key.Close()
			
			myDocuments = None
			try:
				myDocuments = __getRegValue(_winreg.HKEY_CURRENT_USER, 
						r"Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders",
						"Personal")
			except:
				try:
					# Vista
					myDocuments = __getRegValue(_winreg.HKEY_CURRENT_USER, 
							r"Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders",
							"Personal")
				except:
					pass
			
			if myDocuments is None:
				BugUtil.error("Cannot find My Documents folder registry key")
				userDir = "\\"
			else:
				#BugUtil.debug("BugPath - My Documents dir is '%s'", myDocuments)
				userDir = os.path.join(myDocuments, "My Games")
	#BugUtil.debug("BugPath - user dir is '%s'", userDir)
	
	# Determine the root directory that holds the executable and mods
	# as well as the directory inside "My Games" that holds CustomAssets and mods.
	if (sys.executable):
		#BugUtil.debug("BugPath - exe is '%s'", sys.executable)
		appDir = os.path.dirname(sys.executable)
		appName = os.path.basename(appDir)
		#BugUtil.debug("BugPath - app dir is '%s'", appDir)
		#BugUtil.debug("BugPath - app name is '%s'", appName)
	else:
		BugUtil.warn("BugPath - no executable")
	
	if rootDir is None:
		rootDir = os.path.join(userDir, appName)
	#BugUtil.debug("BugPath - rootDir is '%s'", rootDir)
	systemFoldersInitDone = True

# Create an ordered list of paths which are searched for INI files.
iniFileSearchPaths = []
def addIniFileSearchPath(path):
	"""Adds the given path to the search list if it is a valid directory."""
	if (os.path.isdir(path)):
		#BugUtil.info("BugPath - adding search path '%s'", path)
		iniFileSearchPaths.append(path)

searchPathsInitDone = False
def initSearchPaths():
	global searchPathsInitDone
	if searchPathsInitDone:
		return
	BugUtil.debug("BugPath - initializing search paths")
	initSystemFolders()
	initModFolder()
	# INI file search paths
	if (userDir):
		if (modName):
			addIniFileSearchPath(os.path.join(userDir, modName))
			addIniFileSearchPath(os.path.join(rootDir, modName))
			if (modDir):
				addIniFileSearchPath(os.path.join(modDir, modName))
		if (modDir):
			addIniFileSearchPath(os.path.join(modDir))
		addIniFileSearchPath(os.path.join(rootDir))
	if (appDir):
		if (modName):
			addIniFileSearchPath(os.path.join(appDir, modName))
		addIniFileSearchPath(os.path.join(appDir))
	searchPathsInitDone = True
	if (iniFileSearchPaths):
		BugConfigTracker.add("Config_Search_Paths", iniFileSearchPaths)
	else:
		pass
	# asset search paths (art files, XML, config, etc)
	if (userDir):
		noCustomAssets = False
		if (modDir and modFolder):
# don't check game option because BUG is initialized only once
#			if CyGame().isOption(GameOptionTypes.GAMEOPTION_LOCK_MODS):
#				noCustomAssets = True
#				BugUtil.debug("BugPath - Lock Modified Assets is set")
#			else:
				try:
					from configobj import ConfigObj
					config = ConfigObj(os.path.join(modDir, modFolder + ".ini"), encoding='utf_8')
					noCustomAssets = config["CONFIG"].as_bool("NoCustomAssets")
					BugUtil.debug("BugPath - NoCustomAssets: %s", noCustomAssets)
				except:
					BugUtil.warn("BugPath - failed to parse mod INI for NoCustomAssets")
		if (not noCustomAssets):
			addAssetFileSearchPath(os.path.join(rootDir, "CustomAssets"))
		if (modDir):
			addAssetFileSearchPath(os.path.join(modDir, "Assets"))
	if (appDir):
		addAssetFileSearchPath(os.path.join(appDir, "Assets"))
	if (assetFileSearchPaths):
		BugConfigTracker.add("Asset_Search_Paths", assetFileSearchPaths)
	else:
		pass

def findIniFile(name, subdir=None):
	"Locates the named configuration file using the search paths above."
	initSearchPaths()
	for dir in iniFileSearchPaths:
		if (subdir):
			path = os.path.join(dir, subdir, name)
		else:
			path = os.path.join(dir, name)
		if (os.path.isfile(path)):
			return path
	return None

def createIniFile(name, subdir=None):
	"Returns the path for the default location of the named INI file so it can be created."
	initSearchPaths()
	if iniFileSearchPaths:
		dir = iniFileSearchPaths[0]
		if (subdir):
			return os.path.join(dir, subdir, name)
		else:
			return os.path.join(dir, name)
	if (subdir):
		return os.path.join(subdir, name)
	else:
		return name

mainModIniDir = None
def findMainModIniFile():
	"Locates the main INI file for the mod."
	global mainModIniDir
	if (modName):
		file = findIniFile(modName + ".ini")
		if (file):
			mainModIniDir = os.path.dirname(file)
		return file
	else:
		return None

def findDir(name):
	"Locates the named directory using the INI file search paths above."
	initSearchPaths()
	for dir in iniFileSearchPaths:
		path = os.path.join(dir, name)
		if (os.path.isdir(path)):
			return path
	return None

def makeDir(name):
	"Creates a new directory where the INI file was found or the first directory in the search path."
	initSearchPaths()
	if (mainModIniDir):
		path = os.path.join(mainModIniDir, name)
	elif (iniFileSearchPaths):
		path = os.path.join(iniFileSearchPaths[0], name)
	else:
		path = name
	if (not os.path.isdir(path)):
		os.makedirs(path)
	return path

def findOrMakeDir(name):
	"Locates or creates the specified directory."
	dir = findDir(name)
	if (not dir):
		dir = makeDir(name)
	return dir


# Create an ordered list of paths which are searched for asset files.
assetFileSearchPaths = []
def addAssetFileSearchPath(path):
	"Adds the given path to the search list if it is a directory."
	if (os.path.isdir(path)):
		assetFileSearchPaths.append(path)

def findAssetFile(name, subdir=None):
	"Locates the named asset file using the search paths above."
	initSearchPaths()
	for dir in assetFileSearchPaths:
		if (subdir):
			path = os.path.join(dir, subdir, name)
		else:
			path = os.path.join(dir, name)
		if (os.path.isfile(path)):
			return path
	if (subdir):
		BugUtil.warn("BugPath - cannot find asset file %s in %s", name, subdir)
	else:
		BugUtil.warn("BugPath - cannot find asset file %s", name)
	return None
