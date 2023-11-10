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
#class CvPediaUnit:
class CvPediaUnit( CvPediaScreen.CvPediaScreen ):
##--------	BUGFfH: End Modify
	"Civilopedia Screen for Units"

	def __init__(self, main):
		self.iUnit = -1
		self.top = main

##--------	BUGFfH: Modified by Denev 2009/09/18
		"""
		self.X_UNIT_PANE = 20
		self.Y_UNIT_PANE = 70
		self.W_UNIT_PANE = 433
		self.H_UNIT_PANE = 210

		self.X_UNIT_ANIMATION = 475
		self.Y_UNIT_ANIMATION = 78
		self.W_UNIT_ANIMATION = 303
		self.H_UNIT_ANIMATION = 200
		self.X_ROTATION_UNIT_ANIMATION = -20
		self.Z_ROTATION_UNIT_ANIMATION = 30
		self.SCALE_ANIMATION = 1.0

		self.X_ICON = 48
		self.Y_ICON = 105
		self.W_ICON = 150
		self.H_ICON = 150
		self.ICON_SIZE = 64

		self.BUTTON_SIZE = 64
		self.PROMOTION_ICON_SIZE = 32

		self.X_STATS_PANE = 210
		self.Y_STATS_PANE = 145
		self.W_STATS_PANE = 250
		self.H_STATS_PANE = 200

		self.X_SPECIAL_PANE = 20
		self.Y_SPECIAL_PANE = 420
		self.W_SPECIAL_PANE = 433
		self.H_SPECIAL_PANE = 144

		self.X_PREREQ_PANE = 20
		self.Y_PREREQ_PANE = 292
		self.W_PREREQ_PANE = 433
		self.H_PREREQ_PANE = 124

		self.X_UPGRADES_TO_PANE = 475
		self.Y_UPGRADES_TO_PANE = 292
		self.W_UPGRADES_TO_PANE = 303
		self.H_UPGRADES_TO_PANE = 124

		self.X_PROMO_PANE = 20
		self.Y_PROMO_PANE = 574
		self.W_PROMO_PANE = 433
		self.H_PROMO_PANE = 124

		self.X_HISTORY_PANE = 475
		self.Y_HISTORY_PANE = 420
		self.W_HISTORY_PANE = 303
		self.H_HISTORY_PANE = 278
		"""
		X_MERGIN = self.top.X_MERGIN
		Y_MERGIN = self.top.Y_MERGIN

		self.X_UNIT_PANE = self.top.X_PEDIA_PAGE
		self.Y_UNIT_PANE = self.top.Y_PEDIA_PAGE
		self.W_UNIT_PANE = 436
		self.H_UNIT_PANE = 210

		self.X_UNIT_ANIMATION = self.X_UNIT_PANE + self.W_UNIT_PANE + X_MERGIN
		self.Y_UNIT_ANIMATION = self.Y_UNIT_PANE + 7
		self.W_UNIT_ANIMATION = self.top.R_PEDIA_PAGE - self.X_UNIT_ANIMATION
		self.H_UNIT_ANIMATION = self.H_UNIT_PANE - 7
		self.X_ROTATION_UNIT_ANIMATION = -20
		self.Z_ROTATION_UNIT_ANIMATION = 30
		self.SCALE_ANIMATION = 1.0

		self.W_ICON = 150
		self.H_ICON = 150
		self.X_ICON = self.X_UNIT_PANE + (self.H_UNIT_PANE - self.H_ICON) / 2
		self.Y_ICON = self.Y_UNIT_PANE + (self.H_UNIT_PANE - self.H_ICON) / 2
		self.ICON_SIZE = 64

		self.BUTTON_SIZE = 64
		self.PROMOTION_ICON_SIZE = 32
		self.PROMOTION_ICON_SIZE_SMALL = 24

		self.X_STATS_PANE = self.X_UNIT_PANE + 190
		self.Y_STATS_PANE = self.Y_UNIT_PANE + 75
		self.W_STATS_PANE = 250
		self.H_STATS_PANE = 200

		self.X_PREREQ_PANE = self.X_UNIT_PANE
		self.Y_PREREQ_PANE = self.Y_UNIT_PANE + self.H_UNIT_PANE + Y_MERGIN
		self.W_PREREQ_PANE = self.W_UNIT_PANE
		self.H_PREREQ_PANE = 110

		self.X_UPGRADES_TO_PANE = self.X_PREREQ_PANE + self.W_PREREQ_PANE + X_MERGIN
		self.Y_UPGRADES_TO_PANE = self.Y_PREREQ_PANE
		self.W_UPGRADES_TO_PANE = self.top.R_PEDIA_PAGE - self.X_UPGRADES_TO_PANE
		self.H_UPGRADES_TO_PANE = self.H_PREREQ_PANE

		self.W_PROMO_PANE = self.W_UNIT_PANE
		self.H_PROMO_PANE = 110
		self.X_PROMO_PANE = self.X_UNIT_PANE
		self.Y_PROMO_PANE = self.top.B_PEDIA_PAGE - self.H_PROMO_PANE

		self.X_SPECIAL_PANE = self.X_PREREQ_PANE
		self.Y_SPECIAL_PANE = self.Y_PREREQ_PANE + self.H_PREREQ_PANE + Y_MERGIN
		self.W_SPECIAL_PANE = self.W_PREREQ_PANE
		self.H_SPECIAL_PANE = self.Y_PROMO_PANE - self.Y_SPECIAL_PANE - Y_MERGIN

		self.X_HISTORY_PANE = self.X_UPGRADES_TO_PANE
		self.Y_HISTORY_PANE = self.Y_UPGRADES_TO_PANE + self.H_UPGRADES_TO_PANE + Y_MERGIN
		self.W_HISTORY_PANE = self.W_UPGRADES_TO_PANE
		self.H_HISTORY_PANE = self.top.B_PEDIA_PAGE - self.Y_HISTORY_PANE
##--------	BUGFfH: End Modify

	# Screen construction function
	def interfaceScreen(self, iUnit):

		self.iUnit = iUnit

		self.top.deleteAllWidgets()

		screen = self.top.getScreen()

		bNotActive = (not screen.isActive())
		if bNotActive:
			self.top.setPediaCommonWidgets()

		# Header...
		szHeader = u"<font=4b>" + gc.getUnitInfo(self.iUnit).getDescription().upper() + u"</font>"
		screen.setLabel(self.top.getNextWidgetName(), "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, CivilopediaPageTypes.CIVILOPEDIA_PAGE_UNIT, iUnit)

		# Top
##--------	BUGFfH: Modified by Denev 2009/09/19
#		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, CivilopediaPageTypes.CIVILOPEDIA_PAGE_UNIT, -1)
		link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_UNIT
		if self.getUnitType(iUnit):
			link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_HERO
		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, link, -1)
##--------	BUGFfH: End Modify

		if self.top.iLastScreen	!= CvScreenEnums.PEDIA_UNIT or bNotActive:
			self.placeLinks(true)
			self.top.iLastScreen = CvScreenEnums.PEDIA_UNIT
		else:
			self.placeLinks(false)

		# Icon
		screen.addPanel( self.top.getNextWidgetName(), "", "", False, False,
			self.X_UNIT_PANE, self.Y_UNIT_PANE, self.W_UNIT_PANE, self.H_UNIT_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		screen.addPanel(self.top.getNextWidgetName(), "", "", false, false,
			self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
##--------	BUGFfH: Modified by Denev 2009/09/19
		#Unit Icon ArtStyle depends on active civilization.
		"""
		szButton = gc.getUnitInfo(self.iUnit).getButton()
		if self.top.iActivePlayer != -1:
			szButton = gc.getPlayer(self.top.iActivePlayer).getUnitButton(self.iUnit)
		"""
		szButton = gc.getUnitInfo(self.iUnit).getUnitButtonWithCivArtStyle(self.top.iActiveCiv)
##--------	BUGFfH: End Modify
		screen.addDDSGFC(self.top.getNextWidgetName(), szButton,
			self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1 )

#FfH: Added by Kael 08/10/2007
		if isWorldUnitClass(gc.getUnitInfo(iUnit).getUnitClassType()):
			szImage = str(gc.getUnitInfo(iUnit).getImage())
##--------	BUGFfH: Modified by Denev 2009/09/19
			"""
			screen.addDDSGFC(self.top.getNextWidgetName(), szImage,
				self.X_UPGRADES_TO_PANE, self.Y_UPGRADES_TO_PANE, self.W_UPGRADES_TO_PANE, self.H_UPGRADES_TO_PANE, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			"""
			iHeight = self.W_UPGRADES_TO_PANE / 3
			iPositionY = self.Y_UPGRADES_TO_PANE + self.H_UPGRADES_TO_PANE - iHeight
			screen.addDDSGFC(self.top.getNextWidgetName(), szImage, self.X_UPGRADES_TO_PANE, iPositionY, self.W_UPGRADES_TO_PANE, iHeight, WidgetTypes.WIDGET_GENERAL, -1, -1 )
##--------	BUGFfH: End Modify
#FfH: End Add

		# Unit animation
		screen.addUnitGraphicGFC(self.top.getNextWidgetName(), self.iUnit, self.X_UNIT_ANIMATION, self.Y_UNIT_ANIMATION, self.W_UNIT_ANIMATION, self.H_UNIT_ANIMATION, WidgetTypes.WIDGET_GENERAL, -1, -1, self.X_ROTATION_UNIT_ANIMATION, self.Z_ROTATION_UNIT_ANIMATION, self.SCALE_ANIMATION, True)

		self.placeStats()

		self.placeUpgradesTo()

		self.placeRequires()

		self.placeSpecial()

		self.placePromotions()

		self.placeHistory()

	# Place strength/movement
	def placeStats(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()

		# Unit combat group
		iCombatType = gc.getUnitInfo(self.iUnit).getUnitCombatType()
		if (iCombatType != -1):
			screen.setImageButton(self.top.getNextWidgetName(), gc.getUnitCombatInfo(iCombatType).getButton(), self.X_STATS_PANE, self.Y_STATS_PANE - 40, 32, 32, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT, iCombatType, 0)
			screen.setText(self.top.getNextWidgetName(), "", u"<font=3>" + gc.getUnitCombatInfo(iCombatType).getDescription() + u"</font>", CvUtil.FONT_LEFT_JUSTIFY, self.X_STATS_PANE + 37, self.Y_STATS_PANE - 35, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT, iCombatType, 0)

		screen.addListBoxGFC(panelName, "", self.X_STATS_PANE, self.Y_STATS_PANE, self.W_STATS_PANE, self.H_STATS_PANE, TableStyles.TABLE_STYLE_EMPTY)
		screen.enableSelect(panelName, False)

		if (gc.getUnitInfo(self.iUnit).getAirCombat() > 0 and gc.getUnitInfo(self.iUnit).getCombat() == 0):
			iStrength = gc.getUnitInfo(self.iUnit).getAirCombat()
		else:
			iStrength = gc.getUnitInfo(self.iUnit).getCombat()

		szName = self.top.getNextWidgetName()

#FfH Defense Str: Modified by Kael 08/18/2007
#		szStrength = localText.getText("TXT_KEY_PEDIA_STRENGTH", ( iStrength, ) )
		if iStrength == gc.getUnitInfo(self.iUnit).getCombatDefense():
			szStrength = localText.getText("TXT_KEY_PEDIA_STRENGTH", ( iStrength, ) )
		else:
			szStrength = localText.getText("TXT_KEY_PEDIA_STRENGTH_DEFENSE", ( iStrength, gc.getUnitInfo(self.iUnit).getCombatDefense()) )
#FfH: End Modify

		screen.appendListBoxStringNoUpdate(panelName, u"<font=4>" + szStrength.upper() + u"%c" % CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		szName = self.top.getNextWidgetName()
		szMovement = localText.getText("TXT_KEY_PEDIA_MOVEMENT", ( gc.getUnitInfo(self.iUnit).getMoves(), ) )
		screen.appendListBoxStringNoUpdate(panelName, u"<font=4>" + szMovement.upper() + u"%c" % CyGame().getSymbolID(FontSymbols.MOVES_CHAR) + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		if (gc.getUnitInfo(self.iUnit).getProductionCost() >= 0 and not gc.getUnitInfo(self.iUnit).isFound()):
			szName = self.top.getNextWidgetName()
			if self.top.iActivePlayer == -1:
				szCost = localText.getText("TXT_KEY_PEDIA_COST", ((gc.getUnitInfo(self.iUnit).getProductionCost() * gc.getDefineINT("UNIT_PRODUCTION_PERCENT"))/100,))
			else:
				szCost = localText.getText("TXT_KEY_PEDIA_COST", ( gc.getActivePlayer().getUnitProductionNeeded(self.iUnit), ) )
			screen.appendListBoxStringNoUpdate(panelName, u"<font=4>" + szCost.upper() + u"%c" % gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		if (gc.getUnitInfo(self.iUnit).getAirRange() > 0):
			szName = self.top.getNextWidgetName()
			szRange = localText.getText("TXT_KEY_PEDIA_RANGE", ( gc.getUnitInfo(self.iUnit).getAirRange(), ) )
			screen.appendListBoxStringNoUpdate(panelName, u"<font=4>" + szRange.upper() + u"</font>", WidgetTypes.WIDGET_GENERAL, 0, 0, CvUtil.FONT_LEFT_JUSTIFY)

		screen.updateListBox(panelName)

	# Place prereqs (techs, resources)
	def placeRequires(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_REQUIRES", ()), "", false, true, self.X_PREREQ_PANE, self.Y_PREREQ_PANE, self.W_PREREQ_PANE, self.H_PREREQ_PANE, PanelStyles.PANEL_STYLE_BLUE50 )

		screen.attachLabel(panelName, "", "  ")

		# add tech buttons
		iPrereq = gc.getUnitInfo(self.iUnit).getPrereqAndTech()
		if (iPrereq >= 0):
			screen.attachImageButton( panelName, "", gc.getTechInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iPrereq, 1, False )

		for j in range(gc.getDefineINT("NUM_UNIT_AND_TECH_PREREQS")):
			iPrereq = gc.getUnitInfo(self.iUnit).getPrereqAndTechs(j)
			if (iPrereq >= 0):
				screen.attachImageButton( panelName, "", gc.getTechInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iPrereq, -1, False )

		# add resource buttons
		bFirst = True
		iPrereq = gc.getUnitInfo(self.iUnit).getPrereqAndBonus()
		if (iPrereq >= 0):
			bFirst = False
			screen.attachImageButton( panelName, "", gc.getBonusInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iPrereq, -1, False )

##--------	BUGFfH: Modified by Denev 2009/09/26
		"Even if AND resource doesn't exist, bracket off OR resources"
		"""
		# count the number of OR resources
		nOr = 0
		for j in range(gc.getNUM_UNIT_PREREQ_OR_BONUSES()):
			if (gc.getUnitInfo(self.iUnit).getPrereqOrBonuses(j) > -1):
				nOr += 1

		szLeftDelimeter = ""
		szRightDelimeter = ""
		#  Display a bracket if we have more than one OR resource and an AND resource
		if (not bFirst):
			if (nOr > 1):
				szLeftDelimeter = localText.getText("TXT_KEY_AND", ()) + "( "
				szRightDelimeter = " ) "
			elif (nOr > 0):
				szLeftDelimeter = localText.getText("TXT_KEY_AND", ())

		if len(szLeftDelimeter) > 0:
			screen.attachLabel(panelName, "", szLeftDelimeter)

		bFirst = True
		for j in range(gc.getNUM_UNIT_PREREQ_OR_BONUSES()):
			eBonus = gc.getUnitInfo(self.iUnit).getPrereqOrBonuses(j)
			if (eBonus > -1):
				if (not bFirst):
					screen.attachLabel(panelName, "", localText.getText("TXT_KEY_OR", ()))
				else:
					bFirst = False
				screen.attachImageButton( panelName, "", gc.getBonusInfo(eBonus).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, eBonus, -1, False )

		if len(szRightDelimeter) > 0:
			screen.attachLabel(panelName, "", szRightDelimeter)
		"""
		"list up all OR resources"
		liBonusPrereq = []
		for iIndex in range(gc.getNUM_UNIT_PREREQ_OR_BONUSES()):
			iBonusPrereq = gc.getUnitInfo(self.iUnit).getPrereqOrBonuses(iIndex)
			if (iBonusPrereq != BonusTypes.NO_BONUS):
				liBonusPrereq.append(iBonusPrereq)

		"place OR resources"
		if not bFirst:
			if len(liBonusPrereq) > 0:
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_AND", ()))
		if len(liBonusPrereq) > 1:
			screen.attachLabel(panelName, "", "(")

		bFirst = true
		for iBonusPrereq in liBonusPrereq:
			if not bFirst:
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_OR", ()))
			screen.attachImageButton( panelName, "", gc.getBonusInfo(iBonusPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iBonusPrereq, -1, False )
			bFirst = false

		if len(liBonusPrereq) > 1:
			screen.attachLabel(panelName, "", ")")
##--------	BUGFfH: End Modify

##--------	BUGFfH: Moved to below(*1) by Denev 2009/09/25
		"""
		# add religion buttons
		iPrereq = gc.getUnitInfo(self.iUnit).getPrereqReligion()
		if (iPrereq >= 0):
			screen.attachImageButton( panelName, "", gc.getReligionInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_RELIGION, iPrereq, -1, False )
		"""
##--------	BUGFfH: End Move

		# add building buttons
		iPrereq = gc.getUnitInfo(self.iUnit).getPrereqBuilding()
		if (iPrereq >= 0):
			screen.attachImageButton( panelName, "", gc.getBuildingInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iPrereq, -1, False )

#FfH: Added by Kael 08/10/2007
		iPrereq = gc.getUnitInfo(self.iUnit).getPrereqBuildingClass()
		if (iPrereq >= 0):
##--------	BUGFfH: Modified by Denev 2009/09/25
#			iPrereq = gc.getBuildingClassInfo(iPrereq).getDefaultBuildingIndex()
			liAvailableBuilding = self.getListTrainableBuilding(iPrereq, self.iUnit)
			if len(liAvailableBuilding) == 1:
				iPrereq = liAvailableBuilding[0]
			elif self.top.iActiveCiv != CivilizationTypes.NO_CIVILIZATION\
			 and gc.getCivilizationInfo(self.top.iActiveCiv).getCivilizationBuildings(iPrereq) in liAvailableBuilding:
				iPrereq = gc.getCivilizationInfo(self.top.iActiveCiv).getCivilizationBuildings(iPrereq)
			else:
				iPrereq = gc.getBuildingClassInfo(iPrereq).getDefaultBuildingIndex()
##--------	BUGFfH: End Modify
			screen.attachImageButton( panelName, "", gc.getBuildingInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iPrereq, -1, False )

		iPrereq = gc.getUnitInfo(self.iUnit).getPrereqCivic()
		if (iPrereq >= 0):
			screen.attachImageButton( panelName, "", gc.getCivicInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, iPrereq, -1, False )
#FfH: End Add

##--------	BUGFfH: Moved from above(*1) by Denev 2009/09/25
		# add religion buttons
		iPrereq = gc.getUnitInfo(self.iUnit).getStateReligion()
		if (iPrereq != ReligionTypes.NO_RELIGION):
			screen.attachLabel(panelName, "", u"  <font=3>(" + localText.getText("TXT_KEY_PEDIA_REQ_STATE_RELIGION", ()) + u"</font>")
			screen.attachImageButton( panelName, "", gc.getReligionInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_RELIGION, iPrereq, -1, False )
			screen.attachLabel(panelName, "", u"<font=3>)</font>")
		else:
			iPrereq = gc.getUnitInfo(self.iUnit).getPrereqReligion()
			if (iPrereq != ReligionTypes.NO_RELIGION):
				screen.attachLabel(panelName, "", u"  <font=3>(" + localText.getText("TXT_KEY_PEDIA_REQ_CITY_HAS", ()) + u"</font>")
				screen.attachImageButton( panelName, "", gc.getReligionInfo(iPrereq).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_RELIGION, iPrereq, -1, False )
				screen.attachLabel(panelName, "", u"<font=3>)</font>")
##--------	BUGFfH: End Move

##--------	BUGFfH: Added by Denev 2009/09/28
		# add spell buttons
		if gc.getUnitInfo(self.iUnit).getProductionCost() < 0:
			liSummining = []
			liReincarnation = []
			for iSpell in range(gc.getNumSpellInfos()):
				if self.iUnit == gc.getSpellInfo(iSpell).getCreateUnitType():
					liSummining.append(iSpell)
				if self.iUnit == gc.getSpellInfo(iSpell).getConvertUnitType():
					liReincarnation.append(iSpell)
			for iSummining in liSummining:
				screen.attachImageButton( panelName, "", gc.getSpellInfo(iSummining).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, iSummining, -1, False )
			if len(liSummining) == 0:
				for iReincarnation in liReincarnation:
					screen.attachImageButton( panelName, "", gc.getSpellInfo(iReincarnation).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, iReincarnation, -1, False )
##--------	BUGFfH: End Add


	# Place upgrades
	def placeUpgradesTo(self):

#FfH: Added by Kael 08/10/2007
		if isWorldUnitClass(gc.getUnitInfo(self.iUnit).getUnitClassType()) == False:
#FfH: End Add

			screen = self.top.getScreen()

			panelName = self.top.getNextWidgetName()
			screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_UPGRADES_TO", ()), "", false, true, self.X_UPGRADES_TO_PANE, self.Y_UPGRADES_TO_PANE, self.W_UPGRADES_TO_PANE, self.H_UPGRADES_TO_PANE, PanelStyles.PANEL_STYLE_BLUE50)

			screen.attachLabel(panelName, "", "  ")

##--------	BUGFfH: Modified by Denev 2009/09/19
			"""
			for k in range(gc.getNumUnitClassInfos()):
				if self.top.iActivePlayer == -1:
					eLoopUnit = gc.getUnitClassInfo(k).getDefaultUnitIndex()
				else:
					eLoopUnit = gc.getCivilizationInfo(gc.getGame().getActiveCivilizationType()).getCivilizationUnits(k)

#FfH: Modified by Kael 02/15/2009
#				if (eLoopUnit >= 0 and gc.getUnitInfo(self.iUnit).getUpgradeUnitClass(k)):
				if (eLoopUnit >= 0 and gc.getUnitInfo(self.iUnit).getUpgradeUnitClass(k) and gc.getUnitInfo(eLoopUnit).isDisableUpgradeTo() == False):
#FfH: End Modify

					szButton = gc.getUnitInfo(eLoopUnit).getButton()
					if self.top.iActivePlayer != -1:
						szButton = gc.getPlayer(self.top.iActivePlayer).getUnitButton(eLoopUnit)
					screen.attachImageButton( panelName, "", szButton, GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, eLoopUnit, 1, False )
			"""
			#get available civilizations which can use the selected unit.
			liAvailableCiv = self.getListUnitCivilization(self.iUnit)

			#if active player's civilization can use the selected unit, adopt it only.
			if self.top.iActiveCiv in liAvailableCiv:
				liAvailableCiv = [self.top.iActiveCiv]

			ltUpgradeUnit = []
			for iUpgradeUnitClass in range(gc.getNumUnitClassInfos()):
				ltUpgradeUnitTemp = []

				#get UnitType (and icon) on each avilable civilization
				for iAvailableCiv in liAvailableCiv:
					iUpgradeUnit = gc.getCivilizationInfo(iAvailableCiv).getCivilizationUnits(iUpgradeUnitClass)

					#does loop UnitClass match to the unit as upgrade target?
					if iUpgradeUnit != UnitTypes.NO_UNIT\
					and gc.getUnitInfo(self.iUnit).getUpgradeUnitClass(iUpgradeUnitClass)\
					and not gc.getUnitInfo(iUpgradeUnit).isDisableUpgradeTo():
						szUnitIcon = gc.getUnitInfo(iUpgradeUnit).getUnitButtonWithCivArtStyle(iAvailableCiv)
						ltUpgradeUnitTemp.append( (iUpgradeUnit, szUnitIcon) )

				#remove repeated items
				ltUpgradeUnitTemp = sorted(set(ltUpgradeUnitTemp))

				#if UnitType and icon can be selected uniquely, adopt it.
				if len(ltUpgradeUnitTemp) == 1:
					ltUpgradeUnit.append(ltUpgradeUnitTemp[0])
				#if more than one kind of unit icon exists, adopt default.
				elif len(ltUpgradeUnitTemp) > 1:
					iUpgradeUnit = gc.getUnitClassInfo(iUpgradeUnitClass).getDefaultUnitIndex()
					szUnitIcon = gc.getUnitInfo(iUpgradeUnit).getButton()
					ltUpgradeUnit.append( (iUpgradeUnit, szUnitIcon) )

			#place unit icons.
			for tUpgradeUnit in ltUpgradeUnit:
				screen.attachImageButton(panelName, "", tUpgradeUnit[1], GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, tUpgradeUnit[0], 1, False)
##--------	BUGFfH: End Modify

	# Place Special abilities
	def placeSpecial(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ()), "", true, false,
								self.X_SPECIAL_PANE, self.Y_SPECIAL_PANE, self.W_SPECIAL_PANE, self.H_SPECIAL_PANE, PanelStyles.PANEL_STYLE_BLUE50 )

		listName = self.top.getNextWidgetName()

##--------	BUGFfH: Modified by Denev 2009/09/18
		"""
		szSpecialText = CyGameTextMgr().getUnitHelp( self.iUnit, True, False, False, None )[1:]
		screen.addMultilineText(listName, szSpecialText, self.X_SPECIAL_PANE+5, self.Y_SPECIAL_PANE+30, self.W_SPECIAL_PANE-10, self.H_SPECIAL_PANE-35, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		"""
		szSpecialText = CyGameTextMgr().getUnitHelp( self.iUnit, True, False, False, None )
		szSpecialText = szSpecialText.strip("\n")	#Trim empty line
		screen.addMultilineText(listName, szSpecialText, self.X_SPECIAL_PANE+5, self.Y_SPECIAL_PANE+30, self.W_SPECIAL_PANE-5, self.H_SPECIAL_PANE-32, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Modify

	def placeHistory(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_CIVILOPEDIA_HISTORY", ()), "", True, True,
						self.X_HISTORY_PANE, self.Y_HISTORY_PANE,
						self.W_HISTORY_PANE, self.H_HISTORY_PANE,
						PanelStyles.PANEL_STYLE_BLUE50 )

		textName = self.top.getNextWidgetName()
		szText = u""
		if len(gc.getUnitInfo(self.iUnit).getStrategy()) > 0:
			szText += localText.getText("TXT_KEY_CIVILOPEDIA_STRATEGY", ())
			szText += gc.getUnitInfo(self.iUnit).getStrategy()
			szText += u"\n\n"
		szText += localText.getText("TXT_KEY_CIVILOPEDIA_BACKGROUND", ())
		szText += gc.getUnitInfo(self.iUnit).getCivilopedia()
##--------	BUGFfH: Modified by Denev 2009/09/18
		"""
		screen.addMultilineText( textName, szText, self.X_HISTORY_PANE + 15, self.Y_HISTORY_PANE + 40,
			self.W_HISTORY_PANE - (15 * 2), self.H_HISTORY_PANE - (15 * 2) - 25, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		"""
		screen.addMultilineText( textName, szText, self.X_HISTORY_PANE + 5, self.Y_HISTORY_PANE + 30,
			self.W_HISTORY_PANE - 5, self.H_HISTORY_PANE - 32, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Modify

	def placeLinks(self, bRedraw):

		screen = self.top.getScreen()

		if bRedraw:
			screen.clearListBoxGFC(self.top.LIST_ID)

##--------	BUGFfH: Modified by Denev 2009/09/19
		"""
		listSorted = self.getUnitSortedList(self.getUnitType(self.iUnit))
		# sort Units alphabetically
#		unitsList=[(0,0)]*gc.getNumUnitInfos()
#		for j in range(gc.getNumUnitInfos()):
#			unitsList[j] = (gc.getUnitInfo(j).getDescription(), j)
#		unitsList.sort()

		i = 0
		iSelected = 0
		for iI in range(len(listSorted)):
			if (not gc.getUnitInfo(listSorted[iI][1]).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxString(self.top.LIST_ID, listSorted[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, listSorted[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY)
				if listSorted[iI][1] == self.iUnit:
					iSelected = i
				i += 1
#		for iI in range(gc.getNumUnitInfos()):
#			if (not gc.getUnitInfo(unitsList[iI][1]).isGraphicalOnly()):
#				if (not gc.getDefineINT("CIVILOPEDIA_SHOW_ACTIVE_CIVS_ONLY") or not gc.getGame().isFinalInitialized() or gc.getGame().isUnitEverActive(unitsList[iI][1])):
#					if bRedraw:
#						screen.appendListBoxStringNoUpdate( self.top.LIST_ID, unitsList[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, unitsList[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY )
#					if unitsList[iI][1] == self.iUnit:
#						iSelected = i
#					i += 1

#		if bRedraw:
#			screen.updateListBox(self.top.LIST_ID)
		"""
		listSorted = self.getSortedList( gc.getNumUnitInfos(), gc.getUnitInfo, self.getUnitType(self.iUnit), self.getUnitType )

		iSelected = 0
		for iIndex, (szDescription, iUnit) in enumerate(listSorted):
			if (not gc.getUnitInfo(iUnit).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxString(self.top.LIST_ID, szDescription, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUnit, 0, CvUtil.FONT_LEFT_JUSTIFY)
				if iUnit == self.iUnit:
					iSelected = iIndex
##--------	BUGFfH: End Modify

		screen.setSelectedListBoxStringGFC(self.top.LIST_ID, iSelected)

	def getUnitType(self, iUnit):
		if (isWorldUnitClass(gc.getUnitInfo(iUnit).getUnitClassType())):
			return true

		# regular Unit #
		return false;

##--------	BUGFfH: Deleted by Denev 2009/09/19
		"""
	def getUnitSortedList(self, bHero):
		listUnits = []
		iCount = 0
		for iUnit in range(gc.getNumUnitInfos()):
			if (self.getUnitType(iUnit) == bHero and not gc.getUnitInfo(iUnit).isGraphicalOnly()):
				listUnits.append(iUnit)
				iCount += 1

		listSorted = [(0,0)] * iCount
		iI = 0
		for iUnit in listUnits:
			listSorted[iI] = (gc.getUnitInfo(iUnit).getDescription(), iUnit)
			iI += 1
		listSorted.sort()
		return listSorted
		"""
##--------	BUGFfH: End Delete
##FfH: End Add##

	def placePromotions(self):
		screen = self.top.getScreen()

		# add pane and text
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_CATEGORY_PROMOTION", ()), "", true, true, self.X_PROMO_PANE, self.Y_PROMO_PANE, self.W_PROMO_PANE, self.H_PROMO_PANE, PanelStyles.PANEL_STYLE_BLUE50 )

		# add promotion buttons
		rowListName = self.top.getNextWidgetName()

#		screen.addMultiListControlGFC(rowListName, "", self.X_PROMO_PANE+15, self.Y_PROMO_PANE+40, self.W_PROMO_PANE-20, self.H_PROMO_PANE-40, 1, self.PROMOTION_ICON_SIZE, self.PROMOTION_ICON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
		iChanneling1 = gc.getInfoTypeForString('PROMOTION_CHANNELING1')
		iChanneling2 = gc.getInfoTypeForString('PROMOTION_CHANNELING2')
		iChanneling3 = gc.getInfoTypeForString('PROMOTION_CHANNELING3')
##--------	BUGFfH: Deleted by Denev 2009/09/18
#		screen.addMultiListControlGFC(rowListName, "", self.X_PROMO_PANE+5, self.Y_PROMO_PANE+30, self.W_PROMO_PANE-5, self.H_PROMO_PANE-30, 1, self.PROMOTION_ICON_SIZE, self.PROMOTION_ICON_SIZE, TableStyles.TABLE_STYLE_STANDARD)
##--------	BUGFfH: End Delete

##--------	BUGFfH: Added by Denev 2009/09/19
		liValidPromotion = []
##--------	BUGFfH: End Add
		for k in range(gc.getNumPromotionInfos()):
			if (isPromotionValid(k, self.iUnit, false) and not gc.getPromotionInfo(k).isGraphicalOnly()):

#				screen.appendMultiListButton( rowListName, gc.getPromotionInfo(k).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, k, -1, false )
				if (gc.getPromotionInfo(k).getPromotionPrereqAnd() != iChanneling1 or gc.getUnitInfo(self.iUnit).getFreePromotions(iChanneling1)):
					if (gc.getPromotionInfo(k).getPromotionPrereqAnd() != iChanneling2 or gc.getUnitInfo(self.iUnit).getFreePromotions(iChanneling2)):
						if (gc.getPromotionInfo(k).getPromotionPrereqAnd() != iChanneling3 or gc.getUnitInfo(self.iUnit).getFreePromotions(iChanneling3)):
##--------	BUGFfH: Modified by Denev 2009/09/19
#							screen.appendMultiListButton( rowListName, gc.getPromotionInfo(k).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, k, -1, false )
							liValidPromotion.append(k)
##--------	BUGFfH: End Modify

##--------	BUGFfH: Added by Denev 2009/09/25
		iPromotionIconSize = self.PROMOTION_ICON_SIZE
		if len(liValidPromotion) > 24:
			iPromotionIconSize = self.PROMOTION_ICON_SIZE_SMALL
		screen.addMultiListControlGFC(rowListName, "", self.X_PROMO_PANE+4, self.Y_PROMO_PANE+30, self.W_PROMO_PANE-4, self.H_PROMO_PANE-30, 1, iPromotionIconSize, iPromotionIconSize, TableStyles.TABLE_STYLE_STANDARD)
		for iValidPromotion in liValidPromotion:
			screen.appendMultiListButton( rowListName, gc.getPromotionInfo(iValidPromotion).getButton(), 0, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, iValidPromotion, -1, false )
##--------	BUGFfH: End Add


	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		return 0


