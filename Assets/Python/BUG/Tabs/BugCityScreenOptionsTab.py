## BugCityScreenOptionsTab
##
## Tab for the BUG City Screen Options (City Screen).
##
## Copyright (c) 2007-2008 The BUG Mod.
##
## Author: EmperorFool

import BugOptionsTab
import CvModName

class BugCityScreenOptionsTab(BugOptionsTab.BugOptionsTab):
	"BUG City Screen Options Screen Tab"
	
	def __init__(self, screen):
		BugOptionsTab.BugOptionsTab.__init__(self, "CityScreen", "City Screen")

	def create(self, screen):
		tab = self.createTab(screen)
		panel = self.createMainPanel(screen)
		column = self.addOneColumnLayout(screen, panel)
		
		left, right = self.addTwoColumnLayout(screen, column, "Top", True)
		
		#self.addLabel(screen, left, "CityScreen", "CITY SCREEN")
		self.addCheckbox(screen, left, "CityScreen__WhipAssist")
		self.addCheckbox(screen, left, "CityScreen__WhipAssistOverflowCountCurrentProduction")
		self.addCheckbox(screen, left, "CityScreen__Anger_Counter")
		self.addCheckbox(screen, left, "CityScreen__Disable_Flavor")		
		
		self.addSpacer(screen, left, "CityScreen1")
		
		self.addCheckbox(screen, left, "CityScreen__RawYields")
		self.addTextDropdown(screen, left, left, "CityScreen__RawYields_View", True)
		
		#self.addSpacer(screen, right, "CityScreen2")
		self.addLabel(screen, right, "Misc", "Misc:")
		self.addCheckbox(screen, right, "CityScreen__CultureTurns")
		self.addCheckbox(screen, right, "CityScreen__GreatPersonTurns")
		self.addCheckbox(screen, right, "CityScreen__GreatPersonInfo")
		self.addCheckbox(screen, right, "CityScreen__FoodAssist")
		self.addCheckbox(screen, right, "MainInterface__ProgressBarsTickMarks")
		self.addTextDropdown(screen, right, right, "CityScreen__Specialists", True)
