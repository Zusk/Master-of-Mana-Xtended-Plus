## BugGeneralOptionsTab
##
## Tab for the BUG General Options (Main and City Screens).
##
## Copyright (c) 2007-2008 The BUG Mod.
##
## Author: EmperorFool

import BugOptionsTab
import CvModName

class BugGeneralOptionsTab(BugOptionsTab.BugOptionsTab):
	"BUG General Options Screen Tab"

	def __init__(self, screen):
		BugOptionsTab.BugOptionsTab.__init__(self, "General", "General")

	def create(self, screen):
		tab = self.createTab(screen)
		panel = self.createMainPanel(screen)
		column = self.addOneColumnLayout(screen, panel)

		left, right = self.addTwoColumnLayout(screen, column, "Top", True)

		#self.addLabel(screen, left, "MainInterface", "MAIN INTERFACE")
		self.addCheckbox(screen, left, "MainInterface__GPBar")
		self.addTextDropdown(screen, left, left, "MainInterface__GPBar_Types", True)

		self.addSpacer(screen, left, "General1")

		self.addCheckbox(screen, left, "MainInterface__FieldOfView")
		self.addCheckbox(screen, left, "MainInterface__FieldOfView_Remember")

		self.addSpacer(screen, left, "General2")

		#self.addLabel(screen, left, "StrategyOverlay", "STRATEGY LAYER")
		self.addCheckbox(screen, left, "StrategyOverlay__Enabled")

		#self.addSpacer(screen, left, "StrategyOverlay_DotMap")
		self.addCheckbox(screen, left, "StrategyOverlay__ShowDotMap")
		self.addCheckbox(screen, left, "StrategyOverlay__DotMapDrawDots")
		leftL, leftR = self.addTwoColumnLayout(screen, left, "DotMapBrightness")
		#self.addTextEdit(screen, leftL, leftR, "StrategyOverlay__DotMapDotIcon")
		self.addSlider(screen, leftL, leftR, "StrategyOverlay__DotMapBrightness", False, False, False, "up", 0, 100)
		self.addSlider(screen, leftL, leftR, "StrategyOverlay__DotMapHighlightBrightness", False, False, False, "up", 0, 100)

		self.addLabel(screen, right, "TechWindow", "Tech Splash Screen:")
		self.addTextDropdown(screen, right, right, "TechWindow__ViewType", True)
		self.addCheckbox(screen, right, "TechWindow__CivilopediaText")

		self.addSpacer(screen, right, "General3")

		self.addLabel(screen, right, "Misc", "Misc:")
		self.addCheckbox(screen, right, "MainInterface__GoldRateWarning")
		self.addCheckbox(screen, right, "MainInterface__MinMax_Commerce")
		self.addCheckbox(screen, right, "MainInterface__ProgressBarsTickMarks")
		self.addCheckbox(screen, right, "MainInterface__CityArrows")
		self.addCheckbox(screen, right, "MainInterface__UnitMovementPointsFraction")
		self.addCheckbox(screen, right, "MainInterface__StackMovementPoints")
		self.addCheckbox(screen, right, "MainInterface__Manabar")
