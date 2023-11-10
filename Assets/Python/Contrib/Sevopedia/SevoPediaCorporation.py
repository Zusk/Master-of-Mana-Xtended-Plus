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

class SevoPediaCorporation:

	def __init__(self, main):
		self.iCorporation = -1
		self.top = main

		self.X_MAIN_PANE = self.top.X_PEDIA_PAGE
		self.Y_MAIN_PANE = self.top.Y_PEDIA_PAGE
		self.W_MAIN_PANE = 200

		self.X_REQUIRES = self.X_MAIN_PANE + self.W_MAIN_PANE + 10
		self.Y_REQUIRES = self.Y_MAIN_PANE
#		self.W_REQUIRES = self.top.R_PEDIA_PAGE - self.X_REQUIRES
		self.W_REQUIRES = 80
		self.H_REQUIRES = 110

		self.X_OFFERS = self.X_MAIN_PANE + self.W_MAIN_PANE + 20 + self.W_REQUIRES
		self.Y_OFFERS = self.Y_MAIN_PANE
		self.W_OFFERS = self.top.R_PEDIA_PAGE - self.X_OFFERS
		self.H_OFFERS = 110
		
		self.X_SPECIAL = self.X_MAIN_PANE + self.W_MAIN_PANE + 10
		self.Y_SPECIAL = self.Y_REQUIRES + self.H_REQUIRES + 10
		self.W_SPECIAL = self.top.R_PEDIA_PAGE - self.X_SPECIAL
		self.H_SPECIAL = 110

		self.X_ITEMLIST1 = self.X_MAIN_PANE 
		self.Y_ITEMLIST1 = self.Y_SPECIAL + self.H_SPECIAL + 10
		self.W_ITEMLIST1  = self.top.R_PEDIA_PAGE - self.X_ITEMLIST1
		self.H_ITEMLIST1 = 110
		
		self.X_ITEMLIST2 = self.X_MAIN_PANE 
		self.Y_ITEMLIST2 = self.Y_ITEMLIST1 + self.H_ITEMLIST1 + 10
		self.W_ITEMLIST2  = self.top.R_PEDIA_PAGE - self.X_ITEMLIST2
		self.H_ITEMLIST2 = 110		
		
		self.H_MAIN_PANE = self.Y_SPECIAL + self.H_SPECIAL - self.Y_MAIN_PANE

		self.W_ICON = 150
		self.H_ICON = 150
		self.X_ICON = self.X_MAIN_PANE + (self.W_MAIN_PANE - self.W_ICON) / 2
		self.Y_ICON = self.Y_MAIN_PANE + (self.H_MAIN_PANE - self.H_ICON) / 2
		self.ICON_SIZE = 64

		self.X_TEXT = self.X_MAIN_PANE
		self.Y_TEXT = self.Y_SPECIAL + self.H_SPECIAL + self.H_ITEMLIST1 + self.H_ITEMLIST2 +20
		self.W_TEXT = self.top.R_PEDIA_PAGE - self.X_TEXT
		self.H_TEXT = self.top.B_PEDIA_PAGE - self.Y_TEXT



	def interfaceScreen(self, iCorporation):
		self.iCorporation = iCorporation
		screen = self.top.getScreen()

#BUGFfH: Added by Denev 2009/08/16
		# Header...
		szHeader = u"<font=4b>" + gc.getCorporationInfo(self.iCorporation).getDescription() + u"</font>"
		szHeaderId = "PediaMainHeader"
		screen.setText(szHeaderId, "Background", szHeader, CvUtil.FONT_CENTER_JUSTIFY, self.top.X_SCREEN, self.top.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
#BUGFfH: End Add

		screen.addPanel(self.top.getNextWidgetName(), "", "", False, False, self.X_MAIN_PANE, self.Y_MAIN_PANE, self.W_MAIN_PANE, self.H_MAIN_PANE, PanelStyles.PANEL_STYLE_BLUE50)
		screen.addPanel(self.top.getNextWidgetName(), "", "", False, False, self.X_ICON, self.Y_ICON, self.W_ICON, self.H_ICON, PanelStyles.PANEL_STYLE_MAIN)
		screen.addDDSGFC(self.top.getNextWidgetName(), gc.getCorporationInfo(self.iCorporation).getButton(), self.X_ICON + self.W_ICON/2 - self.ICON_SIZE/2, self.Y_ICON + self.H_ICON/2 - self.ICON_SIZE/2, self.ICON_SIZE, self.ICON_SIZE, WidgetTypes.WIDGET_GENERAL, -1, -1)

		self.placeSpecial()
		self.placeRequires()
		self.placeText()
		
		self.placeOffers()
		self.placeItemsPositiveLinked()
		self.placeItemsNegativeLinked()
		
	def placeRequires(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_REQUIRES", ()), "", False, True, self.X_REQUIRES, self.Y_REQUIRES, self.W_REQUIRES, self.H_REQUIRES, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")
		iTech = gc.getCorporationInfo(self.iCorporation).getTechPrereq()
		if (iTech > -1):
			screen.attachImageButton(panelName, "", gc.getTechInfo(iTech).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iTech, 1, False)
		for iBuilding in range(gc.getNumBuildingInfos()):
			if (gc.getBuildingInfo(iBuilding).getFoundsCorporation() == self.iCorporation):
				screen.attachImageButton(panelName, "", gc.getBuildingInfo(iBuilding).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iBuilding, 1, False)
		for iUnit in range(gc.getNumUnitInfos()):
			bRequired = False
			for iBuilding in range(gc.getNumBuildingInfos()):
				if (gc.getBuildingInfo(iBuilding).getFoundsCorporation() == self.iCorporation):
					if gc.getUnitInfo(iUnit).getBuildings(iBuilding) or gc.getUnitInfo(iUnit).getForceBuildings(iBuilding):
						bRequired = True
						break
			if bRequired:
				screen.attachImageButton(panelName, "", gc.getUnitInfo(iUnit).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUnit, 1, False)

	def placeOffers(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_HOUSES_OF_EREBUS_ENABLED", ()), "", False, True, self.X_OFFERS, self.Y_OFFERS, self.W_OFFERS, self.H_OFFERS, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")
		for iBuilding in range(gc.getNumBuildingInfos()):
			if (gc.getBuildingInfo(iBuilding).getPrereqCorporation() == self.iCorporation):
				screen.attachImageButton(panelName, "", gc.getBuildingInfo(iBuilding).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iBuilding, 1, False)
				
		for iUnit in range(gc.getNumUnitInfos()):
			if (gc.getUnitInfo(iUnit).getPrereqCorporation() == self.iCorporation):
				screen.attachImageButton(panelName, "", gc.getUnitInfo(iUnit).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUnit, 1, False)
				
				for iBuild in range(gc.getNumBuildInfos()):
					if gc.getUnitInfo(iUnit).getBuilds(iBuild):
						iImprovement=gc.getBuildInfo(iBuild).getImprovement()
						screen.attachImageButton(panelName, "", gc.getImprovementInfo(iImprovement).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_IMPROVEMENT, iImprovement, 1, False)						

	def placeSpecial(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_EFFECTS", ()), "", True, False, self.X_SPECIAL, self.Y_SPECIAL, self.W_SPECIAL, self.H_SPECIAL, PanelStyles.PANEL_STYLE_BLUE50)
		listName = self.top.getNextWidgetName()
		szSpecialText = CyGameTextMgr().parseCorporationInfo(self.iCorporation, True)[1:]
		screen.addMultilineText(listName, szSpecialText, self.X_SPECIAL+5, self.Y_SPECIAL+30, self.W_SPECIAL-10, self.H_SPECIAL-35, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

	def placeItemsPositiveLinked(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_HOUSES_OF_EREBUS_POSITIVE_ITEMS_LINKED", ()), "", False, True, self.X_ITEMLIST1, self.Y_ITEMLIST1, self.W_ITEMLIST1, self.H_ITEMLIST1, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")
		for iTech in range(gc.getNumTechInfos()):
			if (gc.getTechInfo(iTech).getCorporationSupport(self.iCorporation)>0 or gc.getTechInfo(iTech).getCorporationSupportMultiplier(self.iCorporation)>0):
				screen.attachImageButton(panelName, "", gc.getTechInfo(iTech).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iTech, 1, False)

		for iCivic in range(gc.getNumCivicInfos()):
			if (gc.getCivicInfo(iCivic).getCorporationSupport(self.iCorporation)>0 or  gc.getCivicInfo(iCivic).getCorporationSupportMultiplier(self.iCorporation)>0):
				screen.attachImageButton(panelName, "", gc.getCivicInfo(iCivic).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, iCivic, 1, False)

		for iBuilding in range(gc.getNumBuildingInfos()):
			if (gc.getBuildingInfo(iBuilding).getCorporationSupport(self.iCorporation)>0 or gc.getBuildingInfo(iBuilding).getCorporationSupportMultiplier(self.iCorporation)>0):
				screen.attachImageButton(panelName, "", gc.getBuildingInfo(iBuilding).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iBuilding, 1, False)
				
		for iPromotion in range(gc.getNumPromotionInfos()):
			if (gc.getPromotionInfo(iPromotion).getCorporationSupport(self.iCorporation)>0 or gc.getPromotionInfo(iPromotion).getCorporationSupportMultiplier(self.iCorporation)>0):
				screen.attachImageButton(panelName, "", gc.getPromotionInfo(iPromotion).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, iPromotion, 1, False)

		for iSpell in range(gc.getNumSpellInfos()):
			if (gc.getSpellInfo(iSpell).getCorporationSupport(self.iCorporation)>0 or gc.getSpellInfo(iSpell).getCorporationSupportMultiplier(self.iCorporation)>0):
				screen.attachImageButton(panelName, "", gc.getSpellInfo(iSpell).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, iSpell, 1, False)
				
		for iUnit in range(gc.getNumUnitInfos()):
			if (gc.getUnitInfo(iUnit).getCorporationSupport(self.iCorporation)>0 or gc.getUnitInfo(iUnit).getCorporationSupportMultiplier(self.iCorporation)>0):
				screen.attachImageButton(panelName, "", gc.getUnitInfo(iUnit).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUnit, 1, False)
				
#		for iTrait in range(gc.getNumTraitInfos()):
#			if ((not gc.getTraitInfo(iTrait).getCorporationSupport(self.iCorporation)==0) or  (not gc.getTraitInfo(iTrait).getCorporationSupportMultiplier(self.iCorporation)==0)):
#				screen.attachImageButton(panelName, "", gc.getTraitInfo(iTrait).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TRAIT, iTrait, 1, False)

	def placeItemsNegativeLinked(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, localText.getText("TXT_KEY_PEDIA_HOUSES_OF_EREBUS_NEGATIVE_ITEMS_LINKED", ()), "", False, True, self.X_ITEMLIST2, self.Y_ITEMLIST2, self.W_ITEMLIST2, self.H_ITEMLIST2, PanelStyles.PANEL_STYLE_BLUE50)
		screen.attachLabel(panelName, "", "  ")
		for iTech in range(gc.getNumTechInfos()):
			if (gc.getTechInfo(iTech).getCorporationSupport(self.iCorporation)<0 or gc.getTechInfo(iTech).getCorporationSupportMultiplier(self.iCorporation)<0):
				screen.attachImageButton(panelName, "", gc.getTechInfo(iTech).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TECH, iTech, 1, False)

		for iCivic in range(gc.getNumCivicInfos()):
			if (gc.getCivicInfo(iCivic).getCorporationSupport(self.iCorporation)<0 or gc.getCivicInfo(iCivic).getCorporationSupportMultiplier(self.iCorporation)<0):
				screen.attachImageButton(panelName, "", gc.getCivicInfo(iCivic).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_CIVIC, iCivic, 1, False)

		for iBuilding in range(gc.getNumBuildingInfos()):
			if (gc.getBuildingInfo(iBuilding).getCorporationSupport(self.iCorporation)<0 or gc.getBuildingInfo(iBuilding).getCorporationSupportMultiplier(self.iCorporation)<0):
				screen.attachImageButton(panelName, "", gc.getBuildingInfo(iBuilding).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, iBuilding, 1, False)
				
		for iPromotion in range(gc.getNumPromotionInfos()):
			if (gc.getPromotionInfo(iPromotion).getCorporationSupport(self.iCorporation)<0 or gc.getPromotionInfo(iPromotion).getCorporationSupportMultiplier(self.iCorporation)<0):
				screen.attachImageButton(panelName, "", gc.getPromotionInfo(iPromotion).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, iPromotion, 1, False)

		for iSpell in range(gc.getNumSpellInfos()):
			if (gc.getSpellInfo(iSpell).getCorporationSupport(self.iCorporation)<0 or gc.getSpellInfo(iSpell).getCorporationSupportMultiplier(self.iCorporation)<0):
				screen.attachImageButton(panelName, "", gc.getSpellInfo(iSpell).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, iSpell, 1, False)
				
		for iUnit in range(gc.getNumUnitInfos()):
			if (gc.getUnitInfo(iUnit).getCorporationSupport(self.iCorporation)<0 or gc.getUnitInfo(iUnit).getCorporationSupportMultiplier(self.iCorporation)<0):
				screen.attachImageButton(panelName, "", gc.getUnitInfo(iUnit).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, iUnit, 1, False)
				
#		for iTrait in range(gc.getNumTraitInfos()):
#			if ((not gc.getTraitInfo(iTrait).getCorporationSupport(self.iCorporation)==0) or  (not gc.getTraitInfo(iTrait).getCorporationSupportMultiplier(self.iCorporation)==0)):
#				screen.attachImageButton(panelName, "", gc.getTraitInfo(iTrait).getButton(), GenericButtonSizes.BUTTON_SIZE_CUSTOM, WidgetTypes.WIDGET_PEDIA_JUMP_TO_TRAIT, iTrait, 1, False)
				
	def placeText(self):
		screen = self.top.getScreen()
		panelName = self.top.getNextWidgetName()
		screen.addPanel(panelName, "", "", True, True, self.X_TEXT, self.Y_TEXT, self.W_TEXT, self.H_TEXT, PanelStyles.PANEL_STYLE_BLUE50)
		szText = gc.getCorporationInfo(self.iCorporation).getCivilopedia()
		screen.attachMultilineText(panelName, "Text", szText, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

	def handleInput (self, inputClass):
		return 0
