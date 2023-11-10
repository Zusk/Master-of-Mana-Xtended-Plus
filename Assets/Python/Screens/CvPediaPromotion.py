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
#class CvPediaPromotion:
class CvPediaPromotion( CvPediaScreen.CvPediaScreen ):
##--------	BUGFfH: End Modify
	"Civilopedia Screen for Promotions"

	def __init__(self, main):
		self.iPromotion = -1
		self.top = main

##--------	BUGFfH: Modified by Denev 2009/09/22
		"""
		self.BUTTON_SIZE = 46

		self.X_UNIT_PANE = 50
		self.Y_UNIT_PANE = 80
		self.W_UNIT_PANE = 250
		self.H_UNIT_PANE = 210

		self.X_ICON = 98
		self.Y_ICON = 110
		self.W_ICON = 150
		self.H_ICON = 150
		self.ICON_SIZE = 64

		self.X_PREREQ_PANE = 330
		self.Y_PREREQ_PANE = 60
		self.W_PREREQ_PANE = 420
		self.H_PREREQ_PANE = 110

		self.X_LEADS_TO_PANE = 330
		self.Y_LEADS_TO_PANE = 180
		self.W_LEADS_TO_PANE = self.W_PREREQ_PANE
		self.H_LEADS_TO_PANE = 110

		self.X_SPECIAL_PANE = 330
		self.Y_SPECIAL_PANE = 294
		self.W_SPECIAL_PANE = self.W_PREREQ_PANE
		self.H_SPECIAL_PANE = 380 - 4 -  self.H_LEADS_TO_PANE

		self.X_SPELL_PANE = self.X_PREREQ_PANE
		self.Y_SPELL_PANE = self.Y_SPECIAL_PANE + self.H_SPECIAL_PANE + 4
		self.W_SPELL_PANE = self.W_PREREQ_PANE
		self.H_SPELL_PANE = self.H_PREREQ_PANE

		self.X_UNIT_GROUP_PANE = 50
		self.Y_UNIT_GROUP_PANE = 294
		self.W_UNIT_GROUP_PANE = 250
		self.H_UNIT_GROUP_PANE = 380
		self.DY_UNIT_GROUP_PANE = 25
#		self.ITEMS_MARGIN = 18
#		self.ITEMS_SEPARATION = 2
		"""
		X_MERGIN = self.top.X_MERGIN
		Y_MERGIN = self.top.Y_MERGIN

		self.X_MAIN_PANE = self.top.X_PEDIA_PAGE
		self.Y_MAIN_PANE = self.top.Y_PEDIA_PAGE + 10
		self.W_MAIN_PANE = 250
		self.H_MAIN_PANE = 210

		self.W_ICON = 150
		self.H_ICON = 150
		self.X_ICON = self.X_MAIN_PANE + (self.W_MAIN_PANE - self.W_ICON) / 2
		self.Y_ICON = self.Y_MAIN_PANE + (self.H_MAIN_PANE - self.H_ICON) / 2
		self.ICON_SIZE = 64

		self.X_UNIT_ANIMATION = self.X_MAIN_PANE + self.W_MAIN_PANE + X_MERGIN
		self.W_UNIT_ANIMATION = self.top.R_PEDIA_PAGE - self.X_UNIT_ANIMATION
		self.Y_UNIT_ANIMATION = self.Y_MAIN_PANE + 7
		self.H_UNIT_ANIMATION = self.H_MAIN_PANE - 7
		self.X_ROTATION_UNIT_ANIMATION = -20
		self.Z_ROTATION_UNIT_ANIMATION = 30
		self.SCALE_ANIMATION = 0.7

		self.H_LEADS_TO = 110
		self.X_LEADS_TO = self.X_MAIN_PANE + self.W_MAIN_PANE + X_MERGIN
		self.Y_LEADS_TO = self.Y_MAIN_PANE + self.H_MAIN_PANE - self.H_LEADS_TO
		self.W_LEADS_TO = self.top.R_PEDIA_PAGE - self.X_LEADS_TO

		self.W_REQS_PLAYER = 160
		self.H_REQS_PLAYER = 110
		self.X_REQS_PLAYER = self.top.R_PEDIA_PAGE - self.W_REQS_PLAYER
		self.Y_REQS_PLAYER = self.Y_LEADS_TO - self.H_REQS_PLAYER - Y_MERGIN

		self.X_REQUIRES = self.X_LEADS_TO
		self.Y_REQUIRES = self.Y_REQS_PLAYER
		self.W_REQUIRES = self.X_REQS_PLAYER - self.X_LEADS_TO - X_MERGIN
		self.H_REQUIRES = self.H_REQS_PLAYER

		self.X_UNIT_COMBAT = self.X_MAIN_PANE
		self.Y_UNIT_COMBAT = self.Y_MAIN_PANE + self.H_MAIN_PANE + Y_MERGIN
		self.W_UNIT_COMBAT = self.W_MAIN_PANE
		self.H_UNIT_COMBAT = self.top.B_PEDIA_PAGE - self.Y_UNIT_COMBAT
		self.UNIT_COMBAT_ICON_SIZE = 24

		self.H_SPELL = 110
		self.X_SPELL = self.X_LEADS_TO
		self.Y_SPELL = self.top.B_PEDIA_PAGE - self.H_SPELL
		self.W_SPELL = self.W_LEADS_TO

		self.X_SPECIAL = self.X_SPELL
		self.Y_SPECIAL = self.Y_UNIT_COMBAT
		self.W_SPECIAL = self.W_SPELL
		self.H_SPECIAL = self.Y_SPELL - self.Y_SPECIAL - Y_MERGIN

		self.TYPE_REGULAR,\
		self.TYPE_EFFECT,\
		self.TYPE_EQUIPMENT,\
		self.TYPE_RACE = range(4)
##--------	BUGFfH: End Modify

	# Screen construction function
	def interfaceScreen(self, iPromotion):

		self.iPromotion = iPromotion

		self.top.deleteAllWidgets()

		screen = self.top.getScreen()

		bNotActive = (not screen.isActive())
		if bNotActive:
			self.top.setPediaCommonWidgets()

		# Header...
		szHeader = u"<font=4b>" + gc.getPromotionInfo(self.iPromotion).getDescription().upper() + u"</font>"
		szHeaderId = self.top.getNextWidgetName()
		screen.setLabel(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)

		# Top
##--------	BUGFfH: Modified by Denev 2009/09/19
#		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, CivilopediaPageTypes.CIVILOPEDIA_PAGE_PROMOTION, -1)
		if self.getPromotionType(iPromotion) == self.TYPE_RACE:
			link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_RACE
		elif self.getPromotionType(iPromotion) == self.TYPE_EQUIPMENT:
			link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_EQUIPMENT
		elif self.getPromotionType(iPromotion) == self.TYPE_EFFECT:
			link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_EFFECT
		elif self.getPromotionType(iPromotion) == self.TYPE_REGULAR:
			link = CivilopediaPageTypes.CIVILOPEDIA_PAGE_PROMOTION
		screen.setText(self.top.getNextWidgetName(), "Background", self.top.MENU_TEXT, CvUtil.FONT_LEFT_JUSTIFY, self.top.X_MENU, self.top.Y_MENU, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_PEDIA_MAIN, link, -1)
##--------	BUGFfH: End Modify

		if self.top.iLastScreen	!= CvScreenEnums.PEDIA_PROMOTION or bNotActive:
			self.placeLinks(true)
			self.top.iLastScreen = CvScreenEnums.PEDIA_PROMOTION
		else:
			self.placeLinks(false)

		# Icon
		screen.addPanel( self.top.getNextWidgetName(), "", "", False, False,
			self.X_MAIN_PANE, self.Y_MAIN_PANE, self.W_MAIN_PANE, self.H_MAIN_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		screen.addPanel(self.top.getNextWidgetName(), "", "", false, false,
			self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getPromotionInfo(self.iPromotion).getButton(),
			self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getPromotionInfo(self.iPromotion).getButton(), self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, WidgetTypes.WIDGET_GENERAL, self.iPromotion, -1 )

##--------	BUGFfH: Modified by Denev 2009/09/22
		"""
		# Place Required promotions
		self.placePrereqs()

		# Place Allowing promotions
		self.placeLeadsTo()

		# Place the Special abilities block
		self.placeSpecial()

		self.placeSpells()

		self.placeUnitGroups()
		"""
		self.placeSpecial()
		self.placeSpells()
		if self.getPromotionType(iPromotion) == self.TYPE_EQUIPMENT:
			for iUnit in range(gc.getNumUnitInfos()):
				self.iUnit = UnitTypes.NO_UNIT
				if self.iPromotion == gc.getUnitInfo(iUnit).getEquipmentPromotion():
					self.iUnit = iUnit
					break
			if self.iUnit != UnitTypes.NO_UNIT:
				screen.addUnitGraphicGFC(self.top.getNextWidgetName(), self.iUnit, self.X_UNIT_ANIMATION, self.Y_UNIT_ANIMATION, self.W_UNIT_ANIMATION, self.H_UNIT_ANIMATION, WidgetTypes.WIDGET_GENERAL, -1, -1, self.X_ROTATION_UNIT_ANIMATION, self.Z_ROTATION_UNIT_ANIMATION, self.SCALE_ANIMATION, True)
			self.placeHistory()
		else:
			self.placePrereqs()
			self.placePrereqsPlayer()
			self.placeLeadsTo()
			self.placeUnitGroups()
##--------	BUGFfH: End Add


	# Place Leads To...
	def placeLeadsTo(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_LEADS_TO", ()), "", false, true,
				 self.X_LEADS_TO, self.Y_LEADS_TO, self.W_LEADS_TO, self.H_LEADS_TO, PanelStyles.PANEL_STYLE_BLUE50 )

		screen.attachLabel(panelName, "", "  ")

		for j in range(gc.getNumPromotionInfos()):
			iPrereq = gc.getPromotionInfo(j).getPrereqPromotion()
			if (gc.getPromotionInfo(j).getPrereqPromotion() == self.iPromotion or gc.getPromotionInfo(j).getPrereqOrPromotion1() == self.iPromotion or gc.getPromotionInfo(j).getPrereqOrPromotion2() == self.iPromotion or gc.getPromotionInfo(j).getPromotionPrereqOr3() == self.iPromotion or gc.getPromotionInfo(j).getPromotionPrereqOr4() == self.iPromotion or gc.getPromotionInfo(j).getPromotionPrereqOr5() == self.iPromotion or gc.getPromotionInfo(j).getPromotionPrereqOr6() == self.iPromotion or gc.getPromotionInfo(j).getPromotionPrereqAnd() == self.iPromotion):
				screen.attachImageButton( panelName, "", gc.getPromotionInfo(j).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, j, 1, False )

	# Place prereqs...
	def placePrereqs(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_REQUIRES", ()), "", false, true,
				 self.X_REQUIRES, self.Y_REQUIRES, self.W_REQUIRES, self.H_REQUIRES, PanelStyles.PANEL_STYLE_BLUE50 )

		screen.attachLabel(panelName, "", "  ")

##--------	BUGFfH: Added by Denev 2009/09/22
		"""
		ePromo = gc.getPromotionInfo(self.iPromotion).getPrereqPromotion()
		if (ePromo > -1):
			screen.attachImageButton( panelName, "", gc.getPromotionInfo(ePromo).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, ePromo, 1, False )

		ePromoOr1 = gc.getPromotionInfo(self.iPromotion).getPrereqOrPromotion1()
		ePromoOr2 = gc.getPromotionInfo(self.iPromotion).getPrereqOrPromotion2()

#FfH: Added by Kael 09/03/2007
		ePromoAnd = gc.getPromotionInfo(self.iPromotion).getPromotionPrereqAnd()
		ePromoOr3 = gc.getPromotionInfo(self.iPromotion).getPromotionPrereqOr3()
		ePromoOr4 = gc.getPromotionInfo(self.iPromotion).getPromotionPrereqOr4()

		if (ePromoAnd > -1):
			screen.attachLabel(panelName, "", localText.getText("TXT_KEY_AND", ()))
			screen.attachImageButton( panelName, "", gc.getPromotionInfo(ePromoAnd).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, ePromoAnd, 1, False )
#FfH: End Add

		if (ePromoOr1 > -1):
			if (ePromo > -1):
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_AND", ()))

#FfH: Added by Kael 09/03/2007
#			if (ePromoAnd > -1):
#				screen.attachImageButton( panelName, "", gc.getPromotionInfo(ePromoAnd).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, ePromoAnd, 1, False )
#				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_AND", ()))
#FfH: End Add

				if (ePromoOr2 > -1):
					screen.attachLabel(panelName, "", "(")

			screen.attachImageButton( panelName, "", gc.getPromotionInfo(ePromoOr1).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, ePromoOr1, 1, False )

			if (ePromoOr2 > -1):
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_OR", ()))
				screen.attachImageButton( panelName, "", gc.getPromotionInfo(ePromoOr2).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, ePromoOr2, 1, False )

#FfH: Modified by Kael 09/03/2007
#				if (ePromo > -1):
#					screen.attachLabel(panelName, "", ")")
			if (ePromoOr3 > -1):
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_OR", ()))
				screen.attachImageButton( panelName, "", gc.getPromotionInfo(ePromoOr3).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, ePromoOr3, 1, False )

			if (ePromoOr4 > -1):
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_OR", ()))
				screen.attachImageButton( panelName, "", gc.getPromotionInfo(ePromoOr4).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, ePromoOr4, 1, False )

			if (ePromoOr2 > -1):
				screen.attachLabel(panelName, "", ")")
#FfH: End Modify
		"""
		liPromoAnd = []
		liPromoAnd.append(gc.getPromotionInfo(self.iPromotion).getPrereqPromotion())
		liPromoAnd.append(gc.getPromotionInfo(self.iPromotion).getPromotionPrereqAnd())
		liPromoAnd = sorted(set(liPromoAnd), key=liPromoAnd.index)	#remove repeated items
		if liPromoAnd.count(PromotionTypes.NO_PROMOTION):
			liPromoAnd.remove(PromotionTypes.NO_PROMOTION)

		liPromoOr = []
		liPromoOr.append(gc.getPromotionInfo(self.iPromotion).getPrereqOrPromotion1())
		liPromoOr.append(gc.getPromotionInfo(self.iPromotion).getPrereqOrPromotion2())
		liPromoOr.append(gc.getPromotionInfo(self.iPromotion).getPromotionPrereqOr3())
		liPromoOr.append(gc.getPromotionInfo(self.iPromotion).getPromotionPrereqOr4())
		liPromoOr = sorted(set(liPromoOr), key=liPromoOr.index)	#remove repeated items
		if liPromoOr.count(PromotionTypes.NO_PROMOTION):
			liPromoOr.remove(PromotionTypes.NO_PROMOTION)

		bFirst = true
		for iPromotion in liPromoAnd:
			if not bFirst:
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_AND", ()))
			screen.attachImageButton(panelName, "", gc.getPromotionInfo(iPromotion).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, iPromotion, 1, False )
			bFirst = false

		if len(liPromoOr) and not bFirst:
			screen.attachLabel(panelName, "", localText.getText("TXT_KEY_AND", ()))

		if len(liPromoOr) > 1:
			screen.attachLabel(panelName, "", "(")

		bOrFirst = true
		for iPromotion in liPromoOr:
			if not bOrFirst:
				screen.attachLabel(panelName, "", localText.getText("TXT_KEY_OR", ()))
			screen.attachImageButton(panelName, "", gc.getPromotionInfo(iPromotion).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, iPromotion, 1, False )
			bFirst = false
			bOrFirst = false

		if len(liPromoOr) > 1:
			screen.attachLabel(panelName, "", ")")
##--------	BUGFfH: End Modify


##--------	BUGFfH: Added by Denev 2009/09/22
	def placePrereqsPlayer(self):
		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_PLAYER_PREREQ", ()), "", false, true,
				 self.X_REQS_PLAYER, self.Y_REQS_PLAYER, self.W_REQS_PLAYER, self.H_REQS_PLAYER, PanelStyles.PANEL_STYLE_BLUE50 )

		screen.attachLabel(panelName, "", "  ")
##--------	BUGFfH: End Add
		eTech = gc.getPromotionInfo(self.iPromotion).getTechPrereq()
		if (eTech > -1):
			screen.attachImageButton( panelName, "", gc.getTechInfo(eTech).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, eTech, 1, False )

		eReligion = gc.getPromotionInfo(self.iPromotion).getStateReligionPrereq()
		if (eReligion > -1):
			screen.attachImageButton( panelName, "", gc.getReligionInfo(eReligion).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_RELIGION, eReligion, 1, False )

##--------	BUGFfH: Added by Denev 2009/09/22
		iBonus = gc.getPromotionInfo(self.iPromotion).getBonusPrereq()
		if iBonus != BonusTypes.NO_BONUS:
			screen.attachImageButton( panelName, "", gc.getBonusInfo(iBonus).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iBonus, 1, False )
##--------	BUGFfH: End Add

	def placeSpecial(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_SPECIAL_ABILITIES", ()), "", true, false,
				 self.X_SPECIAL, self.Y_SPECIAL, self.W_SPECIAL, self.H_SPECIAL, PanelStyles.PANEL_STYLE_BLUE50 )

		listName = self.top.getNextWidgetName()

##--------	BUGFfH: Modified by Denev 2009/09/21
		"""
		szSpecialText = CyGameTextMgr().getPromotionHelp(self.iPromotion, True)[1:]
		screen.addMultilineText(listName, szSpecialText, self.X_SPECIAL_PANE+5, self.Y_SPECIAL_PANE+30, self.W_SPECIAL_PANE-10, self.H_SPECIAL_PANE-35, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		"""
		szSpecialText = CyGameTextMgr().getPromotionHelp(self.iPromotion, True)
		szSpecialText = szSpecialText.strip("\n")	#trim empty lines
		screen.addMultilineText(listName, szSpecialText, self.X_SPECIAL + 5, self.Y_SPECIAL + 30, self.W_SPECIAL - 5, self.H_SPECIAL - 32, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Modify

	def placeUnitGroups(self):
		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_PROMOTION_UNITS", ()), "", true, true,
			self.X_UNIT_COMBAT, self.Y_UNIT_COMBAT, self.W_UNIT_COMBAT, self.H_UNIT_COMBAT, PanelStyles.PANEL_STYLE_BLUE50 )

		szTable = self.top.getNextWidgetName()
##--------	BUGFfH: Modified by Denev 2009/09/22
		"""
		screen.addTableControlGFC(szTable, 1,
			self.X_UNIT_COMBAT + 10, self.Y_UNIT_COMBAT + 40, self.W_UNIT_COMBAT - 20, self.H_UNIT_COMBAT - 50, False, False, 24, 24, TableStyles.TABLE_STYLE_EMPTY)
		"""
		screen.addTableControlGFC(szTable, 1,
			self.X_UNIT_COMBAT + 10, self.Y_UNIT_COMBAT + 40, self.W_UNIT_COMBAT - 20, self.H_UNIT_COMBAT - 50, False, False, self.UNIT_COMBAT_ICON_SIZE, self.UNIT_COMBAT_ICON_SIZE, TableStyles.TABLE_STYLE_EMPTY)
##--------	BUGFfH: End Modify

		i = 0
		for iI in range(gc.getNumUnitCombatInfos()):
			if (0 != gc.getPromotionInfo(self.iPromotion).getUnitCombat(iI)):
				iRow = screen.appendTableRow(szTable)
##--------	BUGFfH: Modified by Denev 2009/09/22
#				screen.setTableText(szTable, 0, i, u"<font=2>" + gc.getUnitCombatInfo(iI).getDescription() + u"</font>", gc.getUnitCombatInfo(iI).getButton(), WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT, iI, -1, CvUtil.FONT_LEFT_JUSTIFY)
				screen.setTableText(szTable, 0, i, u"<font=3>" + gc.getUnitCombatInfo(iI).getDescription() + u"</font>", gc.getUnitCombatInfo(iI).getButton(), WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT_COMBAT, iI, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Modify
				i += 1


##--------	BUGFfH: Added by Denev 2009/09/22
	def placeHistory(self):
		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_CIVILOPEDIA_HISTORY", ()), "", true, true, self.X_UNIT_COMBAT, self.Y_UNIT_COMBAT, self.W_UNIT_COMBAT, self.H_UNIT_COMBAT, PanelStyles.PANEL_STYLE_BLUE50 )

#		szText = gc.getPromotionInfo(self.iPromotion).getCivilopedia()
#		screen.attachMultilineText( panelName, "", ()), szText, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
##--------	BUGFfH: End Add


	# Place Leads To...
	def placeSpells(self):

		screen = self.top.getScreen()

		panelName = self.top.getNextWidgetName()
		screen.addPanel( panelName, localText.getText("TXT_KEY_PEDIA_SPELLS (or LEADS_TO)", ()), "", false, true,
				 self.X_SPELL, self.Y_SPELL, self.W_SPELL, self.H_SPELL, PanelStyles.PANEL_STYLE_BLUE50 )

		screen.attachLabel(panelName, "", "  ")

		for j in range(gc.getNumSpellInfos()):
			iPrereq = gc.getSpellInfo(j).getPromotionPrereq1()
			if (iPrereq == self.iPromotion):
				screen.attachImageButton( panelName, "", gc.getSpellInfo(j).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, j, 1, False )
			iPrereq = gc.getSpellInfo(j).getPromotionPrereq2()
			if (iPrereq == self.iPromotion):
				screen.attachImageButton( panelName, "", gc.getSpellInfo(j).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, j, 1, False )

	def placeLinks(self, bRedraw):

		screen = self.top.getScreen()

		if bRedraw:
			screen.clearListBoxGFC(self.top.LIST_ID)

		# sort techs alphabetically
##--------	BUGFfH: Modified by Denev 2009/09/19
		"""
#FfH: Modified by Kael 08/27/2008
#		listSorted=[(0,0)]*gc.getNumPromotionInfos()
#		for j in range(gc.getNumPromotionInfos()):
#			listSorted[j] = (gc.getPromotionInfo(j).getDescription(), j)
#		listSorted.sort()
#
#		i = 0
#		iSelected = 0
#		for iI in range(gc.getNumPromotionInfos()):
		listSorted = self.getPromotionSortedList(self.getPromotionType(self.iPromotion))

		i = 0
		iSelected = 0
		for iI in range(len(listSorted)):
#FfH: End Modify

			if (not gc.getPromotionInfo(listSorted[iI][1]).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxStringNoUpdate( self.top.LIST_ID, listSorted[iI][0], WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, listSorted[iI][1], 0, CvUtil.FONT_LEFT_JUSTIFY )
				if listSorted[iI][1] == self.iPromotion:
					iSelected = i
				i += 1

		if bRedraw:
			screen.updateListBox(self.top.LIST_ID)
		"""
		listSorted = self.getSortedList( gc.getNumPromotionInfos(), gc.getPromotionInfo, self.getPromotionType(self.iPromotion), self.getPromotionType )

		iSelected = 0
		for iIndex, (szDescription, iPromotion) in enumerate(listSorted):
			if (not gc.getPromotionInfo(iPromotion).isGraphicalOnly()):
				if bRedraw:
					screen.appendListBoxString(self.top.LIST_ID, szDescription, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, iPromotion, 0, CvUtil.FONT_LEFT_JUSTIFY)
				if iPromotion == self.iPromotion:
					iSelected = iIndex
##--------	BUGFfH: End Modify

		screen.setSelectedListBoxStringGFC(self.top.LIST_ID, iSelected)

#FfH: Added by Kael 08/27/2008
	def getPromotionType(self, iPromotion):
		if (gc.getPromotionInfo(iPromotion).isRace()):
			return self.TYPE_RACE
		elif (gc.getPromotionInfo(iPromotion).isEquipment()):
			return self.TYPE_EQUIPMENT
		elif (gc.getPromotionInfo(iPromotion).getMinLevel() < 0):
			return self.TYPE_EFFECT
		else:
			return self.TYPE_REGULAR

##--------	BUGFfH: Deleted by Denev 2009/09/19
		"""
	def getPromotionSortedList(self, Type):
		listPromotions = []
		iCount = 0
		for iPromotion in range(gc.getNumPromotionInfos()):
			if (self.getPromotionType(iPromotion) == Type and not gc.getPromotionInfo(iPromotion).isGraphicalOnly()):
				listPromotions.append(iPromotion)
				iCount += 1

		listSorted = [(0,0)] * iCount
		iI = 0
		for iPromotion in listPromotions:
			listSorted[iI] = (gc.getPromotionInfo(iPromotion).getDescription(), iPromotion)
			iI += 1
		listSorted.sort()
		return listSorted
		"""
##--------	BUGFfH: End Delete
#FfH: End Add

	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		return 0

