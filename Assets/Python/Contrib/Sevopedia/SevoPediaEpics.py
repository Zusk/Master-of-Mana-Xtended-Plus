# Sid Meier's Civilization 4
# Copyright Firaxis Games 2005

#
# Sevopedia 2.3
#   sevotastic.blogspot.com
#   sevotastic@yahoo.com
#
# additional work by Gaurav, Progor, Ket, Vovan, Fitchn, LunarMongoose
# see ReadMe for details
#

from CvPythonExtensions import *
import CvUtil
#import BugUtil
import ScreenInput
import SevoScreenEnums
##--------	BUGFfH: Added by Denev 2009/08/14
#import SevoPediaMain
##--------	BUGFfH: End Add

gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class SevoPediaEpics:

	def __init__(self, main):
		self.iCivilization = -1
		self.top = main

		self.X_TEXT = self.top.X_PEDIA_PAGE
		self.Y_TEXT = self.top.Y_PEDIA_PAGE + (116 - 100) / 2
		self.W_TEXT = self.top.R_PEDIA_PAGE - self.X_TEXT
		self.H_TEXT = self.top.B_PEDIA_PAGE - self.Y_TEXT

	def interfaceScreen(self, iCivilization):
		self.iCivilization = iCivilization
		screen = self.top.getScreen()

		szTable = self.top.getNextWidgetName()
		screen.addTableControlGFC(szTable, 6, self.X_TEXT, self.Y_TEXT, self.W_TEXT, self.H_TEXT, False, False, 32,32, TableStyles.TABLE_STYLE_STANDARD)
		screen.setTableColumnHeader(szTable, 0, "", 250)
		screen.setTableColumnHeader(szTable, 1, "", 330)

		for i in range(gc.getNumAdventureInfos()):
			if gc.getAdventureInfo(i).isPrereqCiv(self.iCivilization):
				iRow = screen.appendTableRow(szTable)
				screen.setTableText(szTable, 0, iRow, gc.getAdventureInfo(i).getDescription().upper(), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
				for j in range(gc.getMAX_ADVENTURESTEPS()):
					if gc.getAdventureInfo(i).getAdventureStep(j) >= 0 and gc.getAdventureInfo(i).getAdventureStep(j) < gc.getNumAdventureStepInfos():
						iRow = screen.appendTableRow(szTable)
						screen.setTableText(szTable, 0, iRow, gc.getAdventureStepInfo(gc.getAdventureInfo(i).getAdventureStep(j)).getDescription(), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						textkey =str(gc.getAdventureStepInfo(gc.getAdventureInfo(i).getAdventureStep(j)).getTextKey())
						screen.setTableText(szTable, 1, iRow, localText.getText(textkey, (0,gc.getAdventureInfo(i).getAdventureCounter(j))), "", WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
						

	def handleInput (self, inputClass):
		print "Input"
		return 0
