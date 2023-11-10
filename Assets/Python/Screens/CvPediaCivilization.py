## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
##--------	BUGFfH: Added by Denev 2009/09/22
import CvPediaScreen		# base class
##--------	BUGFfH: End Add
import CvScreenEnums

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

##--------	BUGFfH: Modified by Denev 2009/09/21
#class CvPediaCivilization:
class CvPediaCivilization( CvPediaScreen.CvPediaScreen ):
##--------	BUGFfH: End Modify
	"Civilopedia Screen for Civilizations"

	def __init__(self, main):
		self.iCivilization = -1
		self.top = main

##--------	BUGFfH: Modified by Denev 2009/09/22
		"""
		self.X_MAIN_PANE = 50
		self.Y_MAIN_PANE = 90
		self.W_MAIN_PANE = 250
		self.H_MAIN_PANE = 210

		self.X_ICON = 98
		self.Y_ICON = 125
		self.W_ICON = 150
		self.H_ICON = 150
		self.ICON_SIZE = 64

		self.X_TECH = 330
		self.Y_TECH = 70
		self.W_TECH = 200
		self.H_TECH = 110

		self.X_BUILDING = 555
		self.Y_BUILDING = 190
		self.W_BUILDING = 200
		self.H_BUILDING = 110

		self.X_UNIT = 330
		self.Y_UNIT = 190
		self.W_UNIT = 200
		self.H_UNIT = 110

		self.X_LEADER = 555
		self.Y_LEADER = 70
		self.W_LEADER = 200
		self.H_LEADER = 110

		self.X_TEXT = self.X_MAIN_PANE
		self.Y_TEXT = self.Y_MAIN_PANE + self.H_MAIN_PANE + 20
		self.W_TEXT = 705
		self.H_TEXT = 350
		"""
		X_MERGIN = self.top.X_MERGIN
		Y_MERGIN = self.top.Y_MERGIN

		self.X_MAIN_PANE = self.top.X_PEDIA_PAGE
		self.Y_MAIN_PANE = self.top.Y_PEDIA_PAGE
		self.W_MAIN_PANE = 210
		self.H_MAIN_PANE = 210

		self.W_ICON = 150
		self.H_ICON = 150
		self.X_ICON = self.X_MAIN_PANE + (self.W_MAIN_PANE - self.W_ICON) / 2
		self.Y_ICON = self.Y_MAIN_PANE + (self.H_MAIN_PANE - self.H_ICON) / 2
		self.ICON_SIZE = 64

		self.H_LEADER = 110
		self.X_LEADER = self.X_MAIN_PANE + self.W_MAIN_PANE + X_MERGIN
		self.Y_LEADER = self.Y_MAIN_PANE + self.H_MAIN_PANE - self.H_LEADER

		NUM_COL = 3
		self.W_SPELL = ( self.top.R_PEDIA_PAGE - self.X_LEADER - X_MERGIN * (NUM_COL - 1) ) // NUM_COL
		self.H_SPELL = self.H_LEADER
		self.X_SPELL = self.top.R_PEDIA_PAGE - self.W_SPELL
		self.Y_SPELL = self.Y_LEADER

		self.W_LEADER = self.X_SPELL - self.X_LEADER - X_MERGIN

		self.H_TECH = 110
		self.X_TECH = self.X_LEADER
		self.Y_TECH = self.Y_LEADER - self.H_TECH - Y_MERGIN
		self.W_TECH = self.W_SPELL

		self.X_TRAIT = self.X_TECH + self.W_TECH + X_MERGIN
		self.Y_TRAIT = self.Y_TECH
		self.W_TRAIT = self.W_TECH
		self.H_TRAIT = self.H_TECH

		self.X_HERO = self.X_TRAIT + self.W_TRAIT + X_MERGIN
		self.Y_HERO = self.Y_TRAIT
		self.W_HERO = self.top.R_PEDIA_PAGE - self.X_HERO
		self.H_HERO = self.H_TRAIT

		self.X_BUILDING = self.X_MAIN_PANE
		self.Y_BUILDING = self.Y_MAIN_PANE + self.H_MAIN_PANE + Y_MERGIN
		self.W_BUILDING = ( self.top.R_PEDIA_PAGE - self.X_BUILDING - X_MERGIN * (NUM_COL - 1) ) // NUM_COL
		self.H_BUILDING = 110

		self.X_UNIT = self.X_BUILDING + self.W_BUILDING + X_MERGIN
		self.Y_UNIT = self.Y_BUILDING
		self.W_UNIT = self.top.R_PEDIA_PAGE - self.X_UNIT
		self.H_UNIT = self.H_BUILDING

		self.X_EX_BUILDING = self.X_BUILDING
		self.Y_EX_BUILDING = self.Y_BUILDING + self.H_BUILDING + Y_MERGIN
		self.W_EX_BUILDING = self.W_BUILDING
		self.H_EX_BUILDING = 110

		self.X_EX_UNIT = self.X_UNIT
		self.Y_EX_UNIT = self.Y_EX_BUILDING
		self.W_EX_UNIT = self.W_UNIT
		self.H_EX_UNIT = self.H_EX_BUILDING

		self.X_TEXT = self.X_MAIN_PANE
		self.Y_TEXT = self.Y_EX_BUILDING + self.H_EX_BUILDING + Y_MERGIN
		self.W_TEXT = self.top.W_PEDIA_PAGE
		self.H_TEXT = self.top.B_PEDIA_PAGE - self.Y_TEXT

		self.Y_WIDE_TEXT = self.Y_EX_BUILDING
		self.H_WIDE_TEXT = self.top.B_PEDIA_PAGE - self.Y_WIDE_TEXT
##--------	BUGFfH: End Modify

	# Screen construction function
	def interfaceScreen(self, iCivilization):

		self.iCivilization = iCivilization

		self.top.deleteAllWidgets()

		screen = self.top.getScreen()

		bNotActive = (not screen.isActive())
		if bNotActive:
			self.top.setPediaCommonWidgets()

		# Header...
		szHeader = u"<font=4b>" + gc.getCivilizationInfo(self.iCivilization).getDescription().upper() + u"</font>"
		szHeaderId = self.top.getNextWidgetName()
		screen.setLabel(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		# Top
		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, CivilopediaPageTypes.CIVILOPEDIA_PAGE_CIV, -1)

		if self.top.iLastScreen	!= CvScreenEnums.PEDIA_CIVILIZATION or bNotActive:
			self.placeLinks(true)
			self.top.iLastScreen = CvScreenEnums.PEDIA_CIVILIZATION
		else:
			self.placeLinks(false)

		# Icon
		screen.addPanel( self.top.getNextWidgetName(), "", "", False, False,
			self.X_MAIN_PANE, self.Y_MAIN_PANE, self.W_MAIN_PANE, self.H_MAIN_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		screen.addPanel(self.top.getNextWidgetName(), "", "", false, false,
			self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
		screen.addDDSGFC(self.top.getNextWidgetName(), ArtFileMgr.getCivilizationArtInfo(gc.getCivilizationInfo(self.iCivilization).getArtDefineTag()).getButton(),
			self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		self.placeTech()
		self.placeBuilding()
		self.placeUnit()
		self.placeLeader()

#BUGFfH: Added by Denev 2009/08/22
		self.bWideText = true

		self.placeTrait()
		self.placeHero()
		self.placeWorldSpell()

		szPanelName1 = self.placeBlockedBuilding()
		szPanelName2 = self.placeBlockedUnit()

		if self.bWideText:
			screen.hide(szPanelName1)
			screen.hide(szPanelName2)
#BUGFfH: End Add

		self.placeText()

		return

	def placeTech(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_FREE_TECHS", ()), "", false, true,
				 self.X_TECH, self.Y_TECH, self.W_TECH, self.H_TECH, PanelStyles.PANEL_STYLE_BLUE50 )
		screen.attachLabel(panelName, "", "  ")

		for iTech in range(gc.getNumTechInfos()):
			if (gc.getCivilizationInfo(self.iCivilization).isCivilizationFreeTechs(iTech)):
				screen.attachImageButton( panelName, "", gc.getTechInfo(iTech).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iTech, 1, False )

	def placeBuilding(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_UNIQUE_BUILDINGS", ()), "", false, true,
				self.X_BUILDING, self.Y_BUILDING, self.W_BUILDING, self.H_BUILDING, PanelStyles.PANEL_STYLE_BLUE50 )
		screen.attachLabel(panelName, "", "  ")

		for iBuildingClass in range(gc.getNumBuildingClassInfos()):
			iUniqueBuilding = gc.getCivilizationInfo(self.iCivilization).getCivilizationBuildings(iBuildingClass)
			iDefaultBuilding = gc.getBuildingClassInfo(iBuildingClass).getDefaultBuildingIndex()
			if (iDefaultBuilding > -1 and iUniqueBuilding > -1 and iDefaultBuilding != iUniqueBuilding):
				screen.attachImageButton( panelName, "", gc.getBuildingInfo(iUniqueBuilding).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iUniqueBuilding, 1, False )

##--------	BUGFfH: Added by Denev 2009/09/22
			if iUniqueBuilding == BuildingTypes.NO_BUILDING:
				continue

			iUniqueCiv = gc.getBuildingInfo(iUniqueBuilding).getPrereqCiv()
			"""
			#if the building is not available to all other civs, it is regarded as unique building.
			if (iUniqueCiv == CivilizationTypes.NO_CIVILIZATION):
				iCount = 0
				for iCiv in range(gc.getNumCivilizationInfos()):
					iUniqueBuilding = gc.getCivilizationInfo(iCiv).getCivilizationBuildings(iBuildingClass)
					if iUniqueBuilding != BuildingTypes.NO_BUILDING:
						iUniqueCivilization = iCiv
						++iCount
				if iCount != 1:
					iUniqueCiv = CivilizationTypes.NO_CIVILIZATION
			"""
			if iUniqueCiv == self.iCivilization:
				szButton = gc.getBuildingInfo(iUniqueBuilding).getButton()
				screen.attachImageButton(panelName, "", szButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iUniqueBuilding, 1, False)
##--------	BUGFfH: End Add

	def placeUnit(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_FREE_UNITS", ()), "", false, true,
				 self.X_UNIT, self.Y_UNIT, self.W_UNIT, self.H_UNIT, PanelStyles.PANEL_STYLE_BLUE50 )
		screen.attachLabel(panelName, "", "  ")

		for iUnitClass in range(gc.getNumUnitClassInfos()):
			iUniqueUnit = gc.getCivilizationInfo(self.iCivilization).getCivilizationUnits(iUnitClass)
			iDefaultUnit = gc.getUnitClassInfo(iUnitClass).getDefaultUnitIndex()
			if (iDefaultUnit > -1 and iUniqueUnit > -1 and iDefaultUnit != iUniqueUnit):
##--------	BUGFfH: Modified by Denev 2009/09/22
				"""
				szButton = gc.getUnitInfo(iUniqueUnit).getButton()
				if self.top.iActivePlayer != -1:
					szButton = gc.getPlayer(self.top.iActivePlayer).getUnitButton(iUniqueUnit)
				screen.attachImageButton( panelName, "", szButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUniqueUnit, 1, False )
				"""
				szUnitButton = gc.getUnitInfo(iUniqueUnit).getUnitButtonWithCivArtStyle(self.iCivilization)
				screen.attachImageButton(panelName, "", szUnitButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUniqueUnit, 1, False)
##--------	BUGFfH: End Modify

##--------	BUGFfH: Added by Denev 2009/09/22
			if iUniqueUnit == UnitTypes.NO_UNIT:
				continue

			iUniqueCiv = gc.getUnitInfo(iUniqueUnit).getPrereqCiv()
			"""
			#if the unit is not available to all other civs, it is regarded as unique unit.
			if (iUniqueCiv == CivilizationTypes.NO_CIVILIZATION):
				iCount = 0
				for iCiv in range(gc.getNumCivilizationInfos()):
					iUniqueUnit = gc.getCivilizationInfo(iCiv).getCivilizationUnits(iUnitClass)
					if iUniqueUnit != UnitTypes.NO_UNIT:
						iUniqueCivilization = iCiv
						++iCount
				if iCount != 1:
					iUniqueCiv = CivilizationTypes.NO_CIVILIZATION
			"""
			if iUniqueCiv == self.iCivilization:
				szButton = gc.getUnitInfo(iUniqueUnit).getUnitButtonWithCivArtStyle(iUniqueCiv)
				screen.attachImageButton(panelName, "", szButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUniqueUnit, 1, False)
##--------	BUGFfH: End Add

	def placeLeader(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_CONCEPT_LEADERS", ()), "", false, true,
				 self.X_LEADER, self.Y_LEADER, self.W_LEADER, self.H_LEADER, PanelStyles.PANEL_STYLE_BLUE50 )
		screen.attachLabel(panelName, "", "  ")

		for iLeader in range(gc.getNumLeaderHeadInfos()):
			civ = gc.getCivilizationInfo(self.iCivilization)
			if civ.isLeaders(iLeader):
				screen.attachImageButton( panelName, "", gc.getLeaderHeadInfo(iLeader).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_LEADER, iLeader, self.iCivilization, False )

	def placeText(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
##--------	BUGFfH: Modified by Denev 2009/09/22
		"""
		screen.addPanel( panelName, "", "", true, true,
				 self.X_TEXT, self.Y_TEXT, self.W_TEXT, self.H_TEXT, PanelStyles.PANEL_STYLE_BLUE50 )
		"""
		iY, iH = self.Y_TEXT, self.H_TEXT
		if self.bWideText:
			iY, iH = self.Y_WIDE_TEXT, self.H_WIDE_TEXT
		screen.addPanel( panelName, "", "", true, true, self.X_TEXT, iY, self.W_TEXT, iH, PanelStyles.PANEL_STYLE_BLUE50 )
##--------	BUGFfH: End Modify

		szText = gc.getCivilizationInfo(self.iCivilization).getCivilopedia()
		screen.attachMultilineText( panelName, "Text", szText, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

##--------	BUGFfH: Added by Denev 2009/09/22
	def placeTrait(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()

		#civilization trait
		pCivilization = gc.getCivilizationInfo(self.iCivilization)
		iCivTrait = pCivilization.getCivTrait()

		#terrain yield modifier
		ltTerrainModifier = []
		for iTerrain in range(gc.getNumTerrainInfos()):
			if self.iCivilization == gc.getTerrainInfo(iTerrain).getCivilizationYieldType():
				szYield = u""
				bFirst = true
				for iYield in range(YieldTypes.NUM_YIELD_TYPES):
					iYieldChange = gc.getTerrainInfo(iTerrain).getCivilizationYieldChange(iYield)
					if (iYieldChange != 0):
						if not bFirst:
							szYield += ", "
						bFirst = false

						szSign = "+"
						if szYield < 0:
							szSign = ""
						szYield += u"<font=3>%s%i%c</font>" % (szSign, iYieldChange, gc.getYieldInfo(iYield).getChar())
				ltTerrainModifier.append( (iTerrain, szYield) )

		#place panel with variable title
		szPanelTitle = localText.getText("TXT_KEY_CIV_TRAIT", ())
		if iCivTrait == TraitTypes.NO_TRAIT:
			if len(ltTerrainModifier) > 0:
				szPanelTitle = localText.getText("TXT_KEY_MISC_CIV_TERRAIN", ())
		screen.addPanel(panelName, szPanelTitle, "", False, True, self.X_TRAIT, self.Y_TRAIT, self.W_TRAIT, self.H_TRAIT, PanelStyles.PANEL_STYLE_BLUE50)

		#place each buttons
		if pCivilization.getCivTrait() != TraitTypes.NO_TRAIT:
			screen.attachImageButton(panelName, "", gc.getTraitInfo(iCivTrait).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TRAIT, iCivTrait, 1, False)
		for iTerrain, szYield in ltTerrainModifier:
			childPanelName = self.top.getNextWidgetName()
			screen.attachLabel(panelName, "", "  ")
			screen.attachPanel(panelName, childPanelName, "", "", False, False, PanelStyles.PANEL_STYLE_EMPTY)
			screen.attachImageButton(childPanelName, "", gc.getTerrainInfo(iTerrain).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TERRAIN, iTerrain, 1, False)
			screen.attachLabel(childPanelName, "", szYield)


	def placeHero(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_CATEGORY_HERO", ()), "", False, True, self.X_HERO, self.Y_HERO, self.W_HERO, self.H_HERO, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")

		iHero = gc.getCivilizationInfo(self.iCivilization).getHero()
		if iHero != UnitTypes.NO_UNIT:
			heroInfo = gc.getUnitInfo(iHero)
			screen.attachImageButton(panelName, "", heroInfo.getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iHero, 1, False)


	def placeWorldSpell(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_CONCEPT_WORLD_SPELLS", ()), "", False, True, self.X_SPELL, self.Y_SPELL, self.W_SPELL, self.H_SPELL, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")

		for iSpell in range(gc.getNumSpellInfos()):
			pSpell = gc.getSpellInfo(iSpell)
			if pSpell.isGlobal():
				if pSpell.getCivilizationPrereq() == self.iCivilization:
					screen.attachImageButton(panelName, "", pSpell.getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, iSpell, 1, False)
					break


	def placeBlockedBuilding(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_MISC_CIV_BLOCKED_BUILDINGS", ()), "", False, True, self.X_EX_BUILDING, self.Y_EX_BUILDING, self.W_EX_BUILDING, self.H_EX_BUILDING, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")

		for iBuildingClass in range(gc.getNumBuildingClassInfos()):
			iUniqueBuilding = gc.getCivilizationInfo(self.iCivilization).getCivilizationBuildings(iBuildingClass)
			iDefaultBuilding = gc.getBuildingClassInfo(iBuildingClass).getDefaultBuildingIndex()
			if iDefaultBuilding != BuildingTypes.NO_BUILDING:
				if iUniqueBuilding == BuildingTypes.NO_BUILDING:
					if gc.getBuildingInfo(iDefaultBuilding).getPrereqCiv() == CivilizationTypes.NO_CIVILIZATION:
						szButton = gc.getBuildingInfo(iDefaultBuilding).getButton()
						screen.attachImageButton(panelName, "", szButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iDefaultBuilding, 1, False)

					self.bWideText = false

		return panelName


	def placeBlockedUnit(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_MISC_CIV_BLOCKED_UNITS", ()), "", False, True, self.X_EX_UNIT, self.Y_EX_UNIT, self.W_EX_UNIT, self.H_EX_UNIT, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")

		for iUnitClass in range(gc.getNumUnitClassInfos()):
			iUniqueUnit = gc.getCivilizationInfo(self.iCivilization).getCivilizationUnits(iUnitClass)
			iDefaultUnit = gc.getUnitClassInfo(iUnitClass).getDefaultUnitIndex()
			if iDefaultUnit != UnitTypes.NO_UNIT:
				if iUniqueUnit == UnitTypes.NO_UNIT:
					if gc.getUnitInfo(iDefaultUnit).getPrereqCiv() == CivilizationTypes.NO_CIVILIZATION:
						if not isWorldUnitClass(iUnitClass):
							szButton = gc.getUnitInfo(iDefaultUnit).getButton()
							screen.attachImageButton(panelName, "", szButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iDefaultUnit, 1, False)

							self.bWideText = false

		return panelName
##--------	BUGFfH: End Add

	def placeLinks(self, bRedraw):

		screen = self.top.getScreen()

		if bRedraw:
			screen.clearListBoxGFC(self.top.LIST_ID)

		# sort Improvements alphabetically
##--------	BUGFfH: Modified by Denev 2009/09/22
		"""
		listSorted=[(0,0)]*gc.getNumCivilizationInfos()
		for j in range(gc.getNumCivilizationInfos()):
			listSorted[j] = (gc.getCivilizationInfo(j).getDescription(), j)
		listSorted.sort()

		iSelected = 0
		i = 0
		for iI in range(gc.getNumCivilizationInfos()):

#FfH: Modified by Kael 12/23/2007
#			if (gc.getCivilizationInfo(listSorted[iI][1]).isPlayable() and not gc.getCivilizationInfo(listSorted[iI][1]).isGraphicalOnly()):
			if not gc.getCivilizationInfo(listSorted[iI][1]).isGraphicalOnly():
#FfH: End Modify

				if (not gc.getDefineINT("CIVILOPEDIA_SHOW_ACTIVE_CIVS_ONLY") or not gc.getGame().isFinalInitialized() or gc.getGame().isCivEverActive(listSorted[iI][1])):
					if bRedraw:
						screen.appendListBoxStringNoUpdate(self.top.LIST_ID, listSorted[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIV, listSorted[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY )
					if listSorted[iI][1] == self.iCivilization:
						iSelected = i
					i += 1

		if bRedraw:
			screen.updateListBox(self.top.LIST_ID)
		"""
		listSorted = self.getSortedList( gc.getNumCivilizationInfos(), gc.getCivilizationInfo )

		iSelected = 0
		for iIndex, (szDescription, iCivilization) in enumerate(listSorted):
			if (not gc.getCivilizationInfo(iCivilization).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxString(self.top.LIST_ID, szDescription, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIV, iCivilization, 0, CvUtil.FONT_LEFT_JUSTIFY)
				if iCivilization == self.iCivilization:
					iSelected = iIndex
##--------	BUGFfH: End Modify

		screen.setSelectedListBoxStringGFC(self.top.LIST_ID, iSelected)


	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		return 0


