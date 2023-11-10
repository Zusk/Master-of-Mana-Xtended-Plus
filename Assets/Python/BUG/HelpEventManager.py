## HelpEventManager
##
## Opens the BUG help file when Shift-F1 is hit..
##
## Copyright (c) 2008 The BUG Mod.
##
## Author: EmperorFool

from CvPythonExtensions import *
import BugHelp

class HelpEventManager:

	def __init__(self, eventManager):
		eventManager.addEventHandler("kbdEvent", self.onKbdEvent)
		self.eventMgr = eventManager

	def onKbdEvent(self, argsList):
		eventType, key, mx, my, px, py = argsList
		if eventType == self.eventMgr.EventKeyDown:
			theKey = int(key)
			# Check if Alt + Ctrl + F1 were hit
			if (theKey == int(InputTypes.KB_F1) and self.eventMgr.bAlt and self.eventMgr.bCtrl):
				BugHelp.launch()
				return 1
		return 0
