## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
##--------	BUGFfH: Added by Denev 2009/09/19
import CvPediaScreen		# base class
##--------	BUGFfH: End Add
import CvScreenEnums

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

##--------	BUGFfH: Modified by Denev 2009/09/19
#class CvPediaImprovement:
class CvPediaImprovement( CvPediaScreen.CvPediaScreen ):
##--------	BUGFfH: End Modify
	"Civilopedia Screen for tile Improvements"

	def __init__(self, main):
		self.iImprovement = -1
		self.top = main
		
##--------	BUGFfH: Modified by Denev 2009/09/18
		"""
		self.X_UPPER_PANE = 20
		self.Y_UPPER_PANE = 65
		self.W_UPPER_PANE = 433
		self.H_UPPER_PANE = 210
		
		self.X_IMPROVENEMT_ANIMATION = 475
		self.Y_IMPROVENEMT_ANIMATION = 73
		self.W_IMPROVENEMT_ANIMATION = 303
		self.H_IMPROVENEMT_ANIMATION = 200
		self.X_ROTATION_IMPROVENEMT_ANIMATION = -20
		self.Z_ROTATION_IMPROVENEMT_ANIMATION = 30
		self.SCALE_ANIMATION = 0.8
		
		self.X_ICON = 165
		self.Y_ICON = 100
		self.W_ICON = 150
		self.H_ICON = 150
		self.ICON_SIZE = 64
		
		self.X_IMPROVEMENTS_PANE = self.X_UPPER_PANE
		self.Y_IMPROVEMENTS_PANE = self.Y_UPPER_PANE + self.H_UPPER_PANE + 20
		self.W_IMPROVEMENTS_PANE = 500
		self.H_IMPROVEMENTS_PANE = 135
		
		self.X_BONUS_YIELDS_PANE = self.X_IMPROVEMENTS_PANE + self.W_IMPROVEMENTS_PANE + 25
		self.Y_BONUS_YIELDS_PANE = self.Y_UPPER_PANE + self.H_UPPER_PANE + 20
		self.W_BONUS_YIELDS_PANE = 230
		self.H_BONUS_YIELDS_PANE = 402
		
		self.X_REQUIRES = self.X_UPPER_PANE
		self.Y_REQUIRES = self.Y_IMPROVEMENTS_PANE + self.H_IMPROVEMENTS_PANE + 20
		self.W_REQUIRES = 500
		self.H_REQUIRES = 110
		
		self.X_EFFECTS = self.X_UPPER_PANE
		self.Y_EFFECTS = self.Y_REQUIRES + self.H_REQUIRES + 20
		self.W_EFFECTS = 500
		self.H_EFFECTS = 120
		"""
		X_MERGIN = self.top.X_MERGIN
		Y_MERGIN = self.top.Y_MERGIN

		self.X_UPPER_PANE = self.top.X_PEDIA_PAGE
		self.Y_UPPER_PANE = self.top.Y_PEDIA_PAGE
		self.W_UPPER_PANE = 436
		self.H_UPPER_PANE = 210
		
		self.W_ICON = 150
		self.H_ICON = 150
		self.X_ICON = self.X_UPPER_PANE + (self.H_UPPER_PANE - self.H_ICON) / 2
		self.Y_ICON = self.Y_UPPER_PANE + (self.H_UPPER_PANE - self.H_ICON) / 2
		self.ICON_SIZE = 64

		self.X_STATUS = self.X_ICON + self.W_ICON + 10
		self.Y_STATUS = self.Y_ICON
		self.W_STATUS = self.X_UPPER_PANE + self.W_UPPER_PANE - self.X_STATUS
		self.H_STATUS = self.Y_UPPER_PANE + self.H_UPPER_PANE - self.Y_STATUS
		
		self.X_IMPROVEMENT_ANIMATION = self.X_UPPER_PANE + self.W_UPPER_PANE + X_MERGIN
		self.Y_IMPROVEMENT_ANIMATION = self.Y_UPPER_PANE + 7
		self.W_IMPROVEMENT_ANIMATION = self.top.R_PEDIA_PAGE - self.X_IMPROVEMENT_ANIMATION
		self.H_IMPROVEMENT_ANIMATION = self.H_UPPER_PANE - 7
		self.X_ROTATION_IMPROVEMENT_ANIMATION = -20
		self.Z_ROTATION_IMPROVEMENT_ANIMATION = 30
		self.SCALE_ANIMATION = 0.8

		self.W_HISTORY = (self.top.R_PEDIA_PAGE - self.top.X_PEDIA_PAGE) // 2 - X_MERGIN // 2
		self.X_HISTORY = self.top.R_PEDIA_PAGE - self.W_HISTORY
		self.Y_HISTORY = self.Y_IMPROVEMENT_ANIMATION + self.H_IMPROVEMENT_ANIMATION + Y_MERGIN
		self.H_HISTORY = self.top.B_PEDIA_PAGE - self.Y_HISTORY
		
		self.W_BONUS_YIELDS = 230
		self.X_BONUS_YIELDS = self.top.R_PEDIA_PAGE - self.W_BONUS_YIELDS
		self.Y_BONUS_YIELDS = self.Y_UPPER_PANE + self.H_UPPER_PANE + Y_MERGIN
		self.H_BONUS_YIELDS = self.top.B_PEDIA_PAGE - self.Y_BONUS_YIELDS
		
		self.W_UPGRADES_TO = 160
		self.H_UPGRADES_TO = 110
		self.X_UPGRADES_TO = self.X_BONUS_YIELDS - self.W_UPGRADES_TO - X_MERGIN
		self.Y_UPGRADES_TO = self.Y_UPPER_PANE + self.H_UPPER_PANE + Y_MERGIN
		
		self.X_REQUIRES = self.X_UPPER_PANE
		self.Y_REQUIRES = self.Y_UPGRADES_TO
		self.W_REQUIRES = self.X_UPGRADES_TO - self.X_REQUIRES - X_MERGIN
		self.H_REQUIRES = self.H_UPGRADES_TO

		self.X_IMPROVEMENTS = self.X_UPPER_PANE

		self.X_EFFECTS = self.X_UPPER_PANE

	def adjustPanePosition(self, bGlobal):
		X_MERGIN = self.top.X_MERGIN
		Y_MERGIN = self.top.Y_MERGIN

		if not bGlobal:
			self.Y_IMPROVEMENTS = self.Y_REQUIRES + self.H_REQUIRES + Y_MERGIN
			self.W_IMPROVEMENTS = self.X_BONUS_YIELDS - self.X_UPPER_PANE - X_MERGIN
			self.H_IMPROVEMENTS = 125
			
			self.Y_EFFECTS = self.Y_IMPROVEMENTS + self.H_IMPROVEMENTS + Y_MERGIN
			self.W_EFFECTS = self.W_IMPROVEMENTS
			self.H_EFFECTS = self.top.B_PEDIA_PAGE - self.Y_EFFECTS
		else:
			self.Y_IMPROVEMENTS = self.Y_HISTORY
			self.W_IMPROVEMENTS = self.X_HISTORY - self.top.X_PEDIA_PAGE - X_MERGIN
			self.H_IMPROVEMENTS = (self.top.B_PEDIA_PAGE - self.Y_IMPROVEMENTS) // 3

			self.Y_EFFECTS = self.Y_IMPROVEMENTS + self.H_IMPROVEMENTS + Y_MERGIN
			self.W_EFFECTS = self.W_IMPROVEMENTS
			self.H_EFFECTS = self.top.B_PEDIA_PAGE - self.Y_EFFECTS
##--------	BUGFfH: End Modify
		
	# Screen construction function
	def interfaceScreen(self, iImprovement):
			
		self.iImprovement = iImprovement
	
		self.top.deleteAllWidgets()
							
		screen = self.top.getScreen()
		
		bNotActive = (not screen.isActive())
		if bNotActive:
			self.top.setPediaCommonWidgets()

		# Header...
		szHeader = u"<font=4b>" + gc.getImprovementInfo(self.iImprovement).getDescription().upper() + u"</font>"
		szHeaderId = self.top.getNextWidgetName()
		screen.setLabel(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
		
		# Top
##--------	BUGFfH: Modified by Denev 2009/09/19
#		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, CivilopediaPageTypes.CIVILOPEDIA_PAGE_IMPROVEMENT, -1)
		bGlobal = self.getImprovementType(iImprovement)

		link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_IMPROVEMENT
		if bGlobal:
			link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_UNIQUE_FEATURE
		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, link, -1)

		# adjust pane position whether improvement is global or not
		self.adjustPanePosition(bGlobal)
##--------	BUGFfH: End Modify

		if self.top.iLastScreen	!= CvScreenEnums.PEDIA_IMPROVEMENT or bNotActive:
			self.placeLinks(true)
			self.top.iLastScreen = CvScreenEnums.PEDIA_IMPROVEMENT
		else:
			self.placeLinks(false)
			
		# Icon
		screen.addPanel( self.top.getNextWidgetName(), "", "", False, False,
			self.X_UPPER_PANE, self.Y_UPPER_PANE, self.W_UPPER_PANE, self.H_UPPER_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		screen.addPanel(self.top.getNextWidgetName(), "", "", false, false,
			self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getImprovementInfo(self.iImprovement).getButton(),
			self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# Bonus animation
		screen.addImprovementGraphicGFC(self.top.getNextWidgetName(), self.iImprovement, self.X_IMPROVEMENT_ANIMATION, self.Y_IMPROVEMENT_ANIMATION, self.W_IMPROVEMENT_ANIMATION, self.H_IMPROVEMENT_ANIMATION, WidgetTypes.WIDGET_GENERAL, -1, -1, self.X_ROTATION_IMPROVEMENT_ANIMATION, self.Z_ROTATION_IMPROVEMENT_ANIMATION, self.SCALE_ANIMATION, True)
		
##--------	BUGFfH: Modofied by Denev 2009/08/20
		"""
		self.placeSpecial()
		
		self.placeBonusYield()
		
		self.placeYield()
		
		self.placeRequires()
		"""
		self.placeYield()
		self.placeSpecial(bGlobal)
		if not self.getImprovementType(self.iImprovement):
			self.placeStatus()
			self.placeRequires()
			self.placeUpgradesTo()
			self.placeBonusYield()
		else:
			self.placeHistory()
##--------	BUGFfH: End Add

##--------	BUGFfH: Added by Denev 2009/08/14
	def placeStatus(self):
		screen = self.top.getScreen()

		panelTitle = self.top.getNextWidgetName()
		screen.addListBoxGFC(panelTitle, "", self.X_STATUS, self.Y_STATUS + 10, self.W_STATUS, self.H_STATUS - 10, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(panelTitle, False)

		szCostTitle = localText.getText("TXT_KEY_PEDIA_IMPROVEMENT_COST_TITLE", ())
		screen.appendListBoxStringNoUpdate(panelTitle, u"<font=4>" + szCostTitle.upper() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		panelBody = self.top.getNextWidgetName()

		lBuildCostText = []
		for iLoopBuild in range(gc.getNumBuildInfos()):
			if gc.getBuildInfo(iLoopBuild).getImprovement() == self.iImprovement:
				buildInfo = gc.getBuildInfo(iLoopBuild)
				lWorkableUnitClassList = self.getWorkableUnitClassList(iLoopBuild)
				for iLoopUnitClass in lWorkableUnitClassList:
					iWorkableUnit = gc.getUnitClassInfo(iLoopUnitClass).getDefaultUnitIndex()
					pWorkableUnit = gc.getUnitInfo(iWorkableUnit)

					iWorkerTurns = buildInfo.getTime() // pWorkableUnit.getWorkRate()
					if buildInfo.getTime()%pWorkableUnit.getWorkRate():
						iWorkerTurns += 1
					iWorkerTurns = max(1, iWorkerTurns)

					szLinkedName = u"<link=%s>%s</link>" % (pWorkableUnit.getType(), pWorkableUnit.getDescription())

					if not buildInfo.isKill():
						szReqWorkerTurns = localText.getText("TXT_KEY_PEDIA_IMPROVEMENT_COST", (iWorkerTurns, szLinkedName))
					else:
						if iWorkerTurns == 1:
							szReqWorkerTurns = localText.getText("TXT_KEY_PEDIA_IMPROVEMENT_COST_KILL", (szLinkedName, ))
						else:
							szReqWorkerTurns = localText.getText("TXT_KEY_PEDIA_IMPROVEMENT_COST_KILL_MULTIPLE", (iWorkerTurns, szLinkedName))
					lBuildCostText.append(u"<font=4>" + szReqWorkerTurns + u"</font>")

				if lBuildCostText:
					textName = self.top.getNextWidgetName()
					szBuildCostText = "\n".join(lBuildCostText)
					screen.addMultilineText(textName, szBuildCostText, self.X_STATUS + 5, self.Y_STATUS + 36, self.W_STATUS - 5, self.H_STATUS - 38, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)	

					screen.updateListBox(panelTitle)
					screen.updateListBox(panelBody)
				break
##--------	BUGFfH: End Add

	def placeYield(self):
		
		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
##--------	BUGFfH: Modofied by Denev 2009/08/23
		"""
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_CATEGORY_IMPROVEMENT", ()), "", true, true,
				 self.X_IMPROVEMENTS, self.Y_IMPROVEMENTS, self.W_IMPROVEMENTS, self.H_IMPROVEMENTS, PanelStyles.PANEL_STYLE_BLUE50 )

		info = gc.getImprovementInfo(self.iImprovement)
		
		szYield = u""

#FfH: Added by Kael 09/27/2007
		szYield += CyGameTextMgr().getImprovementHelp(self.iImprovement, True) + u"\n"
#FfH: End Add

		for k in range(YieldTypes.NUM_YIELD_TYPES):
			iYieldChange = gc.getImprovementInfo(self.iImprovement).getYieldChange(k)
			if (iYieldChange != 0):
				if (iYieldChange > 0):
					sign = "+"
				else:
					sign = ""
					
				szYield += (u"%s: %s%i%c\n" % (gc.getYieldInfo(k).getDescription(), sign, iYieldChange, gc.getYieldInfo(k).getChar()))
			
		for k in range(YieldTypes.NUM_YIELD_TYPES):
			iYieldChange = gc.getImprovementInfo(self.iImprovement).getIrrigatedYieldChange(k)
			if (iYieldChange != 0):
				szYield += localText.getText("TXT_KEY_PEDIA_IRRIGATED_YIELD", (gc.getYieldInfo(k).getTextKey(), iYieldChange, gc.getYieldInfo(k).getChar())) + u"\n"
			
		for k in range(YieldTypes.NUM_YIELD_TYPES):
			iYieldChange = gc.getImprovementInfo(self.iImprovement).getHillsYieldChange(k)
			if (iYieldChange != 0):
				szYield += localText.getText("TXT_KEY_PEDIA_HILLS_YIELD", (gc.getYieldInfo(k).getTextKey(), iYieldChange, gc.getYieldInfo(k).getChar())) + u"\n"
			
		for k in range(YieldTypes.NUM_YIELD_TYPES):
			iYieldChange = gc.getImprovementInfo(self.iImprovement).getRiverSideYieldChange(k)
			if (iYieldChange != 0):
				szYield += localText.getText("TXT_KEY_PEDIA_RIVER_YIELD", (gc.getYieldInfo(k).getTextKey(), iYieldChange, gc.getYieldInfo(k).getChar())) + u"\n"

		for iTech in range(gc.getNumTechInfos()):
			for k in range(YieldTypes.NUM_YIELD_TYPES):
				iYieldChange = gc.getImprovementInfo(self.iImprovement).getTechYieldChanges(iTech, k)
				if (iYieldChange != 0):
					szYield += localText.getText("TXT_KEY_PEDIA_TECH_YIELD", (gc.getYieldInfo(k).getTextKey(), iYieldChange, gc.getYieldInfo(k).getChar(), gc.getTechInfo(iTech).getTextKey())) + u"\n"

		for iCivic in range(gc.getNumCivicInfos()):
			for k in range(YieldTypes.NUM_YIELD_TYPES):
				iYieldChange = gc.getCivicInfo(iCivic).getImprovementYieldChanges(self.iImprovement, k)
				if (iYieldChange != 0):
					szYield += localText.getText("TXT_KEY_PEDIA_TECH_YIELD", (gc.getYieldInfo(k).getTextKey(), iYieldChange, gc.getYieldInfo(k).getChar(), gc.getCivicInfo(iCivic).getTextKey())) + u"\n"

		for iRoute in range(gc.getNumRouteInfos()):
			for k in range(YieldTypes.NUM_YIELD_TYPES):
				iYieldChange = gc.getImprovementInfo(self.iImprovement).getRouteYieldChanges(iRoute, k)
				if (iYieldChange != 0):
					szYield += localText.getText("TXT_KEY_PEDIA_ROUTE_YIELD", (gc.getYieldInfo(k).getTextKey(), iYieldChange, gc.getYieldInfo(k).getChar(), gc.getRouteInfo(iRoute).getTextKey())) + u"\n"

		listName = self.top.getNextWidgetName()

#FfH: Modified by Kael 09/27/2007
#		screen.addMultilineText(listName, szYield, self.X_IMPROVEMENTS_PANE+5, self.Y_IMPROVEMENTS_PANE+30, self.W_IMPROVEMENTS_PANE-10, self.H_IMPROVEMENTS_PANE-35, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.addMultilineText(listName, szYield, self.X_IMPROVEMENTS_PANE+5, self.Y_IMPROVEMENTS_PANE+10, self.W_IMPROVEMENTS_PANE-10, self.H_IMPROVEMENTS_PANE-15, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
#FfH: End Modify
		"""
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_BASE_YIELDS", ()), "", true, true,
				 self.X_IMPROVEMENTS, self.Y_IMPROVEMENTS, self.W_IMPROVEMENTS, self.H_IMPROVEMENTS, PanelStyles.PANEL_STYLE_BLUE50 )

		szYield = CyGameTextMgr().getImprovementYieldHelp(self.iImprovement, true)
		szYield = szYield.strip("\n")	#trim empty lines
		textName = self.top.getNextWidgetName()
		screen.addMultilineText(textName, szYield, self.X_IMPROVEMENTS + 5, self.Y_IMPROVEMENTS + 30, self.W_IMPROVEMENTS - 5, self.H_IMPROVEMENTS - 32, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Modify
		
	def placeBonusYield(self):
		
		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_BONUS_YIELDS", ()), "", true, true,
				 self.X_BONUS_YIELDS, self.Y_BONUS_YIELDS, self.W_BONUS_YIELDS, self.H_BONUS_YIELDS, PanelStyles.PANEL_STYLE_BLUE50 )
		
		info = gc.getImprovementInfo(self.iImprovement)

		for j in range(gc.getNumBonusInfos()):
			bFirst = True
			szYield = u""
			bEffect = False
			for k in range(YieldTypes.NUM_YIELD_TYPES):
				iYieldChange = info.getImprovementBonusYield(j, k)
				if (iYieldChange != 0):
					bEffect = True
					if (bFirst):
						bFirst = False
					else:
						szYield += u", "
						
					if (iYieldChange > 0):
						sign = u"+"
					else:
						sign = u""
						
					szYield += (u"%s%i%c" % (sign, iYieldChange, gc.getYieldInfo(k).getChar()))
			if (bEffect):
				childPanelName = self.top.getNextWidgetName()
				screen.attachPanel(panelName, childPanelName, "", "", False, False, PanelStyles.PANEL_STYLE_EMPTY)

				screen.attachLabel(childPanelName, "", "  ")
				screen.attachImageButton( childPanelName, "", gc.getBonusInfo(j).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, j, 1, False )
				screen.attachLabel(childPanelName, "", u"<font=4>" + szYield + u"</font>")
	
	def placeRequires(self):
		
		screen = self.top.getScreen()
		
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_REQUIRES", ()), "", false, true,
				 self.X_REQUIRES, self.Y_REQUIRES, self.W_REQUIRES, self.H_REQUIRES, PanelStyles.PANEL_STYLE_BLUE50 )
		
		screen.attachLabel(panelName, "", "  ")
		
		for iBuild in range(gc.getNumBuildInfos()):
			if (gc.getBuildInfo(iBuild).getImprovement() == self.iImprovement):	 
				iTech = gc.getBuildInfo(iBuild).getTechPrereq()
				if (iTech > -1):
					screen.attachImageButton( panelName, "", gc.getTechInfo(iTech).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iTech, 1, False )

##--------	BUGFfH: Added by Denev 2009/09/21
		#Civilization Requirement
		iCivPrereq = gc.getImprovementInfo(self.iImprovement).getPrereqCivilization()
		if iCivPrereq != CivilizationTypes.NO_CIVILIZATION:
			screen.attachImageButton( panelName, "", gc.getCivilizationInfo(iCivPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIV, iCivPrereq, 1, False )
##--------	BUGFfH: End Add

##--------	BUGFfH: Added by Denev 2009/10/08
	def placeUpgradesTo(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_UPGRADES_TO", ()), "", False, True, self.X_UPGRADES_TO, self.Y_UPGRADES_TO, self.W_UPGRADES_TO, self.H_UPGRADES_TO, PanelStyles.PANEL_STYLE_BLUE50 )
		screen.attachLabel(panelName, "", "  ")

		iUpgradesTo = gc.getImprovementInfo(self.iImprovement).getImprovementUpgrade()
		if iUpgradesTo != ImprovementTypes.NO_IMPROVEMENT:
			szButton = gc.getImprovementInfo(iUpgradesTo).getButton()
			szUpgradeTurns = localText.getText("INTERFACE_CITY_TURNS", (CyGame().getImprovementUpgradeTime(self.iImprovement), ))

			childPanelName = self.top.getNextWidgetName()
			screen.attachPanel(panelName, childPanelName, "", "", False, False, PanelStyles.PANEL_STYLE_EMPTY)
			screen.attachImageButton(childPanelName, "", szButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_IMPROVEMENT, iUpgradesTo, 1, False )
			screen.attachLabel(childPanelName, "", szUpgradeTurns)

	def placeHistory(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_CIVILOPEDIA_HISTORY", ()), "", True, True, self.X_HISTORY, self.Y_HISTORY, self.W_HISTORY, self.H_HISTORY, PanelStyles.PANEL_STYLE_BLUE50)
		textName = self.top.getNextWidgetName()
		szText = u""
		if len(gc.getImprovementInfo(self.iImprovement).getStrategy()) > 0:
			szText += localText.getText("TXT_KEY_CIVILOPEDIA_STRATEGY", ())
			szText += gc.getImprovementInfo(self.iImprovement).getStrategy()
			szText += u"\n\n"
		szText += localText.getText("TXT_KEY_CIVILOPEDIA_BACKGROUND", ())
		szText += gc.getImprovementInfo(self.iImprovement).getCivilopedia()
		screen.addMultilineText(textName, szText, self.X_HISTORY + 5, self.Y_HISTORY + 30, self.W_HISTORY - 5, self.H_HISTORY - 32, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Add

	def placeSpecial(self, bGlobal):
		
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()

##--------	BUGFfH: Modofied by Denev 2009/08/20
		"""
#FfH: Modified by Kael 09/27/2007
#		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_EFFECTS", ()), "", true, false,
#				 self.X_EFFECTS, self.Y_EFFECTS, self.W_EFFECTS, self.H_EFFECTS, PanelStyles.PANEL_STYLE_BLUE50 )
#		listName = self.top.getNextWidgetName()
#		szSpecialText = CyGameTextMgr().getImprovementHelp(self.iImprovement, True)
#		screen.addMultilineText(listName, szSpecialText, self.X_EFFECTS+5, self.Y_EFFECTS+5, self.W_EFFECTS-10, self.H_EFFECTS-10, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.addPanel( panelName, localText.getText("TXT_KEY_CIVILOPEDIA_HISTORY", ()), "", true, false,
				 self.X_EFFECTS, self.Y_EFFECTS, self.W_EFFECTS, self.H_EFFECTS, PanelStyles.PANEL_STYLE_BLUE50 )
		if len(gc.getImprovementInfo(self.iImprovement).getCivilopedia()) > 0:
			listName = self.top.getNextWidgetName()
			szSpecialText = gc.getImprovementInfo(self.iImprovement).getCivilopedia()
			screen.addMultilineText(listName, szSpecialText, self.X_EFFECTS+5, self.Y_EFFECTS+25, self.W_EFFECTS-10, self.H_EFFECTS-30, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
#FfH: End Modify
		"""
		szPanelTitle = localText.getText("TXT_KEY_PEDIA_EFFECTS", ())
		if not bGlobal:
			szPanelTitle += localText.getText("TXT_KEY_OR", ()) + localText.getText("TXT_KEY_CIVILOPEDIA_HISTORY", ())
		screen.addPanel( panelName, szPanelTitle, "", true, false, self.X_EFFECTS, self.Y_EFFECTS, self.W_EFFECTS, self.H_EFFECTS, PanelStyles.PANEL_STYLE_BLUE50 )

		listName = self.top.getNextWidgetName()
		szSpecialText = CyGameTextMgr().getImprovementHelp(self.iImprovement, true) + u"\n\n"
		if not bGlobal:
			if len(gc.getImprovementInfo(self.iImprovement).getCivilopedia()) > 0:
				szSpecialText += localText.getText("TXT_KEY_CIVILOPEDIA_BACKGROUND", ())
				szSpecialText += gc.getImprovementInfo(self.iImprovement).getCivilopedia()
		szSpecialText = szSpecialText.strip("\n")	#trim empty lines
		screen.addMultilineText(listName, szSpecialText, self.X_EFFECTS + 5, self.Y_EFFECTS + 30, self.W_EFFECTS - 5, self.H_EFFECTS - 32, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Modify

	def placeLinks(self, bRedraw):

		screen = self.top.getScreen()

		if bRedraw:
			screen.clearListBoxGFC(self.top.LIST_ID)
		
		# sort Improvements alphabetically
##--------	BUGFfH: Modified by Denev 2009/09/19
		"""
		rowListName=[(0,0)]*gc.getNumImprovementInfos()
		for j in range(gc.getNumImprovementInfos()):
			rowListName[j] = (gc.getImprovementInfo(j).getDescription(), j)
		rowListName.sort()

		iSelected = 0
		i = 0

		for iI in range(gc.getNumImprovementInfos()):
			if (not gc.getImprovementInfo(rowListName[iI][1]).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxString(self.top.LIST_ID, rowListName[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_IMPROVEMENT, rowListName[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY)
				if rowListName[iI][1] == self.iImprovement:
					iSelected = i
				i += 1
		"""
		listSorted = self.getSortedList( gc.getNumImprovementInfos(), gc.getImprovementInfo, self.getImprovementType(self.iImprovement), self.getImprovementType )

		iSelected = 0
		for iIndex, (szDescription, iImprovement) in enumerate(listSorted):
			if (not gc.getImprovementInfo(iImprovement).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxString(self.top.LIST_ID, szDescription, WidgetTypes.WIDGET_PEDIA_JUMP_TO_IMPROVEMENT, iImprovement, 0, CvUtil.FONT_LEFT_JUSTIFY)
				if iImprovement == self.iImprovement:
					iSelected = iIndex
##--------	BUGFfH: End Modify
					
		screen.setSelectedListBoxStringGFC(self.top.LIST_ID, iSelected)

##--------	BUGFfH: Added by Denev 2009/09/19
	def getWorkableUnitClassList(self, iBuild):
		lWorkableUnitClassList = []
		for iLoopUnit in range(gc.getNumUnitInfos()):
			pUnit = gc.getUnitInfo(iLoopUnit)
			if pUnit.getBuilds(iBuild):
				iUnitClass = pUnit.getUnitClassType()
				if not isWorldUnitClass(iUnitClass):
					if not isTeamUnitClass(iUnitClass):
#						if not isNationalUnitClass(iUnitClass):
							if pUnit.getProductionCost() > 0:
								lWorkableUnitClassList.append(iUnitClass)
		lWorkableUnitClassList = sorted(set(lWorkableUnitClassList))
		return lWorkableUnitClassList

	def getImprovementType(self, iImprovement):
		if (gc.getImprovementInfo(iImprovement).isUnique()):
			return true

		# regular Improvement #    
		return false;
##--------	BUGFfH: End Add

##--------	BUGFfH: Deleted by Denev 2009/09/20
		"""
	def getImprovementSortedList(self, bUnique):
		listImprovements = []
		iCount = 0
		for iImprovement in range(gc.getNumImprovementInfos()):
			if (not gc.getImprovementInfo(iImprovement).isGraphicalOnly()):
				if bUnique:
					if (gc.getImprovementInfo(iImprovement).isUnique()):
						listImprovements.append(iImprovement)
						iCount += 1
				else:
					if (not gc.getImprovementInfo(iImprovement).isUnique()):
						listImprovements.append(iImprovement)
						iCount += 1
		
		listSorted = [(0,0)] * iCount
		iI = 0
		for iImprovement in listImprovements:
			listSorted[iI] = (gc.getImprovementInfo(iImprovement).getDescription(), iImprovement)
			iI += 1
		listSorted.sort()
		return listSorted
		"""
##--------	BUGFfH: End Delete

	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		return 0
