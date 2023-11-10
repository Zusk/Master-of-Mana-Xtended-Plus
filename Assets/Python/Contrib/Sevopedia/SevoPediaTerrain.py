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
import ScreenInput
import SevoScreenEnums
import string

gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

class SevoPediaTerrain:

	def __init__(self, main):
		self.iTerrain = -1
		self.top = main

##--------	BUGFfH: Added by Denev 2009/10/07
		X_MERGIN = self.top.X_MERGIN
		Y_MERGIN = self.top.Y_MERGIN

		self.W_EARTHLY = 300
		self.H_EARTHLY = 150
		self.X_EARTHLY = self.top.R_PEDIA_PAGE - self.W_EARTHLY
		self.Y_EARTHLY = self.top.Y_PEDIA_PAGE

		self.X_HELL = self.X_EARTHLY
		self.Y_HELL = self.Y_EARTHLY + self.H_EARTHLY + Y_MERGIN
		self.W_HELL = self.W_EARTHLY
		self.H_HELL = 150
		
##--------	BUGFfH: End Add

		self.X_ICON_PANE = self.top.X_PEDIA_PAGE
		self.Y_ICON_PANE = self.top.Y_PEDIA_PAGE
##--------	BUGFfH: Modified by Denev 2009/10/07
		"""
		self.W_ICON_PANE = self.top.W_PEDIA_PAGE
		self.H_ICON_PANE = 210
		"""
		self.W_ICON_PANE = self.top.W_PEDIA_PAGE - self.W_EARTHLY - X_MERGIN
		self.H_ICON_PANE = self.Y_HELL + self.H_HELL - self.Y_ICON_PANE - 150
##--------	BUGFfH: End Modify

		self.W_ICON = 110
		self.H_ICON = 110
		self.X_ICON = self.X_ICON_PANE + 20# + (self.W_ICON_PANE - self.W_ICON) / 2
		self.Y_ICON = self.Y_ICON_PANE + 20# + (self.H_ICON_PANE - self.H_ICON) / 2
		self.ICON_SIZE = 64

		self.W_BONUS = self.W_ICON_PANE
		self.H_BONUS = 110
		self.X_BONUS = self.X_ICON_PANE
		self.Y_BONUS = self.Y_HELL + 40		
		
##--------	BUGFfH: Modified by Denev 2009/10/07
		"""
		self.X_STATS_PANE = self.X_ICON_PANE + 210
		self.W_STATS_PANE = self.top.R_PEDIA_PAGE - self.X_STATS_PANE
		self.Y_STATS_PANE = 130
		self.H_STATS_PANE = 110
		"""
		self.X_STATS_PANE = self.X_ICON + self.W_ICON + 10
		self.Y_STATS_PANE = self.Y_ICON
		self.W_STATS_PANE = self.X_ICON_PANE + self.W_ICON_PANE - self.X_STATS_PANE
		self.H_STATS_PANE = self.Y_ICON_PANE + self.H_ICON_PANE - self.Y_STATS_PANE
##--------	BUGFfH: End Modify

		self.X_SPECIAL_PANE = self.X_ICON_PANE
##--------	BUGFfH: Modified by Denev 2009/10/07
#		self.W_SPECIAL_PANE = self.W_ICON_PANE
		self.W_SPECIAL_PANE = self.top.W_PEDIA_PAGE
##--------	BUGFfH: End Modify
		self.Y_SPECIAL_PANE = self.Y_HELL + self.H_HELL + 10
		self.H_SPECIAL_PANE = self.top.B_PEDIA_PAGE - self.Y_SPECIAL_PANE



	def interfaceScreen(self, iTerrain):
		self.iTerrain = iTerrain
		screen = self.top.getScreen()

#BUGFfH: Added by Denev 2009/08/16
		# Header...
		szHeader = u"<font=4b>" + gc.getTerrainInfo(self.iTerrain).getDescription() + u"</font>"
		szHeaderId = "PediaMainHeader"
		screen.setText(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
#BUGFfH: End Add

		screen.addPanel(self.top.getNextWidgetName(), "", "", False, False, self.X_ICON_PANE, self.Y_ICON_PANE, self.W_ICON_PANE, self.H_ICON_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		screen.addPanel(self.top.getNextWidgetName(), "", "", False, False, self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getTerrainInfo(self.iTerrain).getButton(), self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1)

		self.placeStats()
		self.placeSpecial()
##--------	BUGFfH: Added by Denev 2009/10/07
		self.placeEarthly()
		self.placeHell()
		self.placeBonus()



	def placeEarthly(self):
		iTerrain = self.iTerrain		
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_TERRAIN_EARTHLY", ()), "", False, True, self.X_EARTHLY, self.Y_EARTHLY, self.W_EARTHLY, self.H_EARTHLY, PanelStyles.PANEL_STYLE_BLUE50)

		for iImprovement in range(gc.getNumImprovementInfos()):
			if (gc.getImprovementInfo(iImprovement).getTerrainMakesValid(iTerrain)):
				if not (gc.getImprovementInfo(iImprovement).isPermanent()):
					screen.attachLabel(panelName, "", "  ")
					screen.attachImageButton(panelName, "", gc.getImprovementInfo(iImprovement).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_IMPROVEMENT, iImprovement, 1, False)

	def placeHell(self):
		iTerrain = self.iTerrain	
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_TERRAIN_FEATURE", ()), "", False, True, self.X_HELL, self.Y_HELL, self.W_HELL, self.H_HELL, PanelStyles.PANEL_STYLE_BLUE50)

		for iFeature in range(gc.getNumFeatureInfos()):
			if (gc.getFeatureInfo(iFeature).isTerrain(iTerrain)):	
				screen.attachLabel(panelName, "", "  ")
				screen.attachImageButton(panelName, "", gc.getFeatureInfo(iFeature).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_FEATURE, iFeature, 1, False)

	def placeBonus(self):
		iTerrain = self.iTerrain	
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_TERRAIN_BONUS", ()), "", False, True, self.X_BONUS, self.Y_BONUS, self.W_BONUS, self.H_BONUS, PanelStyles.PANEL_STYLE_BLUE50)

		for iBonus in range(gc.getNumBonusInfos()):
			if (gc.getBonusInfo(iBonus).isTerrain(iTerrain)) or (gc.getBonusInfo(iBonus).isFeatureTerrain(iTerrain)):	
				screen.attachLabel(panelName, "", "  ")
				screen.attachImageButton(panelName, "", gc.getBonusInfo(iBonus).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iBonus, 1, False)
				
	def placeStats(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addListBoxGFC(panelName, "", self.X_STATS_PANE, self.Y_STATS_PANE, self.W_STATS_PANE, self.H_STATS_PANE, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(panelName, False)
		for k in range(YieldTypes.NUM_YIELD_TYPES):
			iYield = gc.getTerrainInfo(self.iTerrain).getYield(k)
			if (iYield != 0):
				szYield = (u"%s: %i" % (gc.getYieldInfo(k).getDescription().upper(), iYield))
				screen.appendListBoxString(panelName, u"<font=3>" + szYield + (u"%c" % gc.getYieldInfo(k).getChar()) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)



	def placeSpecial(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ()), "", True, False, self.X_SPECIAL_PANE, self.Y_SPECIAL_PANE, self.W_SPECIAL_PANE, self.H_SPECIAL_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		listName = self.top.getNextWidgetName()
		screen.attachListBoxGFC(panelName, listName, "", TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(listName, False)
		szSpecialText = CyGameTextMgr().getTerrainHelp(self.iTerrain, True)
		splitText = string.split(szSpecialText, "\n")
		for special in splitText:
			if len(special) != 0:
				screen.appendListBoxString(listName, special, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)



	def handleInput (self, inputClass):
		return 0
