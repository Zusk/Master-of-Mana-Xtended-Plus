## BugOptionsScreen
##
## Displays the BUG Options Screen and its tabs.
##
## For input handlers see CvOptionsScreenCallbackInterface in Python/EntryPoints.
##
## Copyright (c) 2007-2008 The BUG Mod.
##
## Author: EmperorFool

from CvPythonExtensions import *
import BugOptions
import BugUtil
import CvScreensInterface

import BugErrorOptionsTab

class BugOptionsScreen:
	"BUG Mod Options Screen"
	
	def __init__(self):
		self.iScreenHeight = 50
		self.options = BugOptions.getOptions()
		self.tabs = []
		self.reopen = False

	def addTab(self, tab):
		self.tabs.append(tab)
		tab.setOptions(self.options)

	def getTabControl(self):
		return self.pTabControl

	def refreshScreen(self):
		return 1		

	def interfaceScreen(self):
		"Initial creation of the screen"
		title = BugUtil.getPlainText("TXT_KEY_BUG_OPT_TITLE", "BUG Mod Options")
		self.pTabControl = CyGTabCtrl(title, False, False)
		self.pTabControl.setModal(1)
		self.pTabControl.setSize(900, 705)
		self.pTabControl.setControlsExpanding(False)
		self.pTabControl.setColumnLength(self.iScreenHeight)
		
		if self.options.isLoaded():
			self.createTabs()
		else:
			BugErrorOptionsTab.BugErrorOptionsTab(self).create(self.pTabControl)

	def createTabs(self):
		for i, tab in enumerate(self.tabs):
			if not self.reopen or i % 2:
				tab.create(self.pTabControl)

	def clearAllTranslations(self):
		"Clear the translations of all tabs in response to the user choosing a language"
		for tab in self.tabs:
			tab.clearTranslation()
	
	def close(self):
		# TODO: check for error
		self.options.write()
		self.pTabControl.destroy()
		self.pTabControl = None
		if self.reopen:
			self.reopen = False
			self.interfaceScreen()


def clearAllTranslations(argsList=None):
	CvScreensInterface.getBugOptionsScreen().clearAllTranslations()
