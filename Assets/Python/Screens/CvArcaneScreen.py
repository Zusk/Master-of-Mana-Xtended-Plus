## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums
import CvScreensInterface

PIXEL_INCREMENT = 7
BOX_INCREMENT_WIDTH = 27 # Used to be 33 #Should be a multiple of 3...
BOX_INCREMENT_HEIGHT = 9 #Should be a multiple of 3...
BOX_INCREMENT_Y_SPACING = 6 #Should be a multiple of 3...
BOX_INCREMENT_X_SPACING = 9 #Should be a multiple of 3...

TEXTURE_SIZE = 24
X_START = 6
X_INCREMENT = 27
Y_ROW = 32

CIV_HAS_TECH = 0
CIV_IS_RESEARCHING = 1
CIV_HAS_BONUS = 2
CIV_TECH_AVAILABLE = 3
CIV_NO_TECH = 4

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

# BUG - Options - start
import BugUtil
import BugCore
import BugOptions
CityScreenOpt = BugCore.game.CityScreen
# BUG - Options - end

import CvForeignAdvisor

class CvArcaneScreen:
	"Tech Chooser Screen"

	def __init__(self):

		self.nWidgetCount = 0
		self.iCivSelected = 0
		self.aiCurrentState = []

		# Advanced Start
		self.m_iSelectedTech = -1
		self.m_bSelectedTechDirty = false
		self.m_bTechRecordsDirty = false

		self.iScreen = -1

		self.SCREEN_DICT = {
			"LIFE": 0,
			"NATURE": 1,
			"MIND": 2,
			"CHAOS": 3,
			"DEATH": 4,
			"ELEMENTS": 5,
			"ARCANA": 6,
			"ENCHANTA": 7,
			}

		self.REV_SCREEN_DICT = {}

		for key, value in self.SCREEN_DICT.items():
			self.REV_SCREEN_DICT[value] = key

		self.TXT_KEY_DICT = {
			"LIFE": "Life",
			"NATURE": "Nature",
			"MIND": "Mind",
			"CHAOS": "Chaos",
			"DEATH": "Death",
			"ELEMENTS": "Fire",
			"ARCANA": "Air",
			"ENCHANTA": "Water",
			}

		self.ORDER_LIST = ["LIFE", \
											 "NATURE", \
											 "MIND", \
											 "CHAOS", \
											 "DEATH", \
											 "ELEMENTS", \
											 "ARCANA", \
											 "ENCHANTA"]

		self.iDefaultScreen = self.SCREEN_DICT["LIFE"]

#added Sephi

	def setValues(self):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		self.X_SCREEN = screen.getXResolution()/2
		self.Y_SCREEN = screen.getYResolution()/2
		self.W_SCREEN = screen.getXResolution()
		self.H_SCREEN = screen.getYResolution()

		self.X_LINK = 0+screen.centerX(0)
		self.Y_LINK = self.H_SCREEN - 80

#added Sephi

	def hideScreen (self):
		# Get the screen
		screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )

		# Hide the screen
		screen.hideScreen()

	def killScreen(self):
		if (self.iScreen >= 0):
			screen = self.getScreen()
			screen.hideScreen()
			self.iScreen = -1
		return

	def getScreen(self):
		return CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN)

	# Screen construction function
	def interfaceScreen(self, iScreen):

		if ( CyGame().isPitbossHost() ):
			return

		self.setValues()	#added Sephi

		if (iScreen < 0):
			if (self.iScreen < 0):
				iScreen = self.iDefaultScreen
			else:
				iScreen = self.iScreen

		if (self.iScreen != iScreen):
			self.killScreen()
			self.iScreen = iScreen

		# Create a new screen, called ArcaneScreen, using the file CvArcaneScreen.py for input
		screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )
		screen.setRenderInterfaceOnly(True)
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)

		screen.hide("AddTechButton")
		screen.hide("ASPointsLabel")
		screen.hide("SelectedTechLabel")

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		if self.iScreen==0:
			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/GlobalEnchant.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		if self.iScreen==1:
			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/terraforming.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		if self.iScreen==2:
			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/lightmagic.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/DarkMagic.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		if self.iScreen==3 or self.iScreen==4:
			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/Summoning.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )

#		if self.iScreen==0 or self.iScreen==4:
#			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/DarkMagic.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		if self.iScreen==5 or self.iScreen==6 or self.iScreen==7:
			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/ElemMagic.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#		if self.iScreen==3:
#			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/Summoning.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#		if self.iScreen==4:
#			screen.addDrawControl( "BackgroundFlavor", "Art/Interface/terraforming.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		if ( CyGame().isDebugMode() ):
			screen.addDropDownBoxGFC( "CivDropDown", 22, 12, 192, WidgetTypes.WIDGET_GENERAL, -1, -1, FontTypes.SMALL_FONT )
			screen.setActivation( "CivDropDown", ActivationTypes.ACTIVATE_MIMICPARENTFOCUS )
			for j in range(gc.getMAX_PLAYERS()):
				if (gc.getPlayer(j).isAlive()):
					screen.addPullDownString( "CivDropDown", gc.getPlayer(j).getName(), j, j, False )
		else:
			screen.hide( "CivDropDown" )

		self.nWidgetCount = 0
		self.iCivSelected = gc.getGame().getActivePlayer()
		self.aiCurrentState = []
		screen.setPersistent( True )

		# Advanced Start
		if (gc.getPlayer(self.iCivSelected).getAdvancedStartPoints() >= 0):

			self.m_bSelectedTechDirty = true

			self.X_ADD_TECH_BUTTON = 10
			self.Y_ADD_TECH_BUTTON = 731
			self.W_ADD_TECH_BUTTON = 150
			self.H_ADD_TECH_BUTTON = 30
			self.X_ADVANCED_START_TEXT = self.X_ADD_TECH_BUTTON + self.W_ADD_TECH_BUTTON + 20

			szText = localText.getText("TXT_KEY_WB_AS_ADD_TECH", ())
			screen.setButtonGFC( "AddTechButton", szText, "", self.X_ADD_TECH_BUTTON, self.Y_ADD_TECH_BUTTON, self.W_ADD_TECH_BUTTON, self.H_ADD_TECH_BUTTON, WidgetTypes.WIDGET_GENERAL, -1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
			screen.hide("AddTechButton")

		# Here we set the background widget and exit button, and we show the screen
		screen.showWindowBackground( False )


#Added Sephi
#		screen.setDimensions(screen.centerX(0), screen.centerY(0), 1024, 768)
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		screen.setDimensions(0, 0, self.W_SCREEN, self.H_SCREEN)

		screen.addPanel("BottomLine", "", "", True, True, -100, screen.getYResolution()-120, screen.getXResolution()+200, 200, PanelStyles.PANEL_STYLE_MAIN)

		screen.setText( "TechChooserExit", "Background", u"<font=4>" + CyTranslator().getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + "</font>", CvUtil.FONT_RIGHT_JUSTIFY, self.W_SCREEN-30, self.H_SCREEN-40, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )
		screen.setActivation( "TechChooserExit", ActivationTypes.ACTIVATE_MIMICPARENTFOCUS )

		# Header...
		szText = u"<font=4>"
		szText = szText + localText.getText("TXT_KEY_ARCANE_SCREEN_TITLE", ()).upper()
		szText = szText + u"</font>"
		screen.setLabel( "TechTitleHeader", "Background", szText, CvUtil.FONT_CENTER_JUSTIFY, self.W_SCREEN/2-20, 8, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# Make the scrollable area for the city list...
		screen.addScrollPanel( "TechList", u"", 0, 64, self.W_SCREEN, self.H_SCREEN-180, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.setActivation( "TechList", ActivationTypes.ACTIVATE_NORMAL )
		screen.hide( "TechList" )
		self.drawContents()



	def drawContents(self):

		if (self.iScreen < 0):
			return

		screen = self.getScreen()

		# Link to other Spellresearch screens
		entryPerRow = (len (self.SCREEN_DICT) ) / 2
		self.X_EXIT = self.W_SCREEN - 10
		self.DX_LINK = (self.X_EXIT - self.X_LINK) / (entryPerRow + 1)
		xLink = self.DX_LINK / 2;
		yLink = self.Y_LINK

		for i in range (len (self.ORDER_LIST)):
			szTextId = self.getNextWidgetName()
			szScreen = self.ORDER_LIST[i]
			if (self.iScreen != self.SCREEN_DICT[szScreen]):
				screen.setText (szTextId, "", u"<font=4>" + localText.getText (self.TXT_KEY_DICT[szScreen], ()).upper() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, yLink, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_SPELL_RESEARCH, self.SCREEN_DICT[szScreen], -1)
			else:
				screen.setText (szTextId, "", u"<font=4>" + localText.getColorText (self.TXT_KEY_DICT[szScreen], (), gc.getInfoTypeForString ("COLOR_YELLOW")).upper() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, yLink, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_SPELL_RESEARCH, -1, -1)
			xLink += self.DX_LINK
			if(i == entryPerRow):
				yLink += 40
				xLink = self.DX_LINK / 2;

		self.placeTechs(self.iScreen)
		self.drawArrows(self.iScreen)

		screen.moveToFront( "CivDropDown" )

		screen.moveToFront( "AddTechButton" )


	def placeTechs (self, iFlag):

		iMaxX = 0
		iMaxY = 0

		# If we are the Pitboss, we don't want to put up an interface at all
		if ( CyGame().isPitbossHost() ):
			return

		# Get the screen
		screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )

		aiCurrentState = []

		# Go through all the techs
		for i in range(gc.getNumTechInfos()):
			if gc.getTechInfo(i).getSpellSchool() != iFlag:
				self.aiCurrentState.append(-1)
				continue

			# Create and place a tech in its proper location
			iX = 30 + ( (gc.getTechInfo(i).getGridX() - 1) * ( ( BOX_INCREMENT_X_SPACING + BOX_INCREMENT_WIDTH ) * PIXEL_INCREMENT ) )
			iY = ( gc.getTechInfo(i).getGridY() - 1 ) * ( BOX_INCREMENT_Y_SPACING * PIXEL_INCREMENT ) + 5
			szTechRecord = "TechRecord" + str(i)

			if ( iMaxX < iX + self.getXStart() ):
				iMaxX = iX + self.getXStart()
			if ( iMaxY < iY + ( BOX_INCREMENT_HEIGHT * PIXEL_INCREMENT ) ):
				iMaxY = iY + ( BOX_INCREMENT_HEIGHT * PIXEL_INCREMENT )

			screen.attachPanelAt( "TechList", szTechRecord, u"", u"", True, False, PanelStyles.PANEL_STYLE_TECH, iX - 6, iY - 6, self.getXStart() + 6, 12 + ( BOX_INCREMENT_HEIGHT * PIXEL_INCREMENT ), WidgetTypes.WIDGET_TECH_TREE, i, -1 )
			screen.setActivation( szTechRecord, ActivationTypes.ACTIVATE_MIMICPARENTFOCUS)
			screen.hide( szTechRecord )

			#reset so that it offsets from the tech record's panel
			iX = 6
			iY = 6

			bHasTech = gc.getTeam(gc.getPlayer(self.iCivSelected).getTeam()).isHasTech(i)
			bIsFocus = false
			if gc.getPlayer(self.iCivSelected).getSpellResearchFocus() == i:
				bIsFocus = true
			bCanResearch = gc.getPlayer(self.iCivSelected).canSpellResearchNext(i)
			bCanSpellResearchEver = true
			iBonus = gc.getTechInfo(i).getPrereqBonus()
			iAlignmentPrereq=gc.getTechInfo(i).getPreferredAlignment()
			iAlignmentBlocked=gc.getTechInfo(i).getBlockedbyAlignment()
			iAlignmentPlayer=gc.getPlayer(self.iCivSelected).getAlignment()
			if iBonus != -1:
				if gc.getPlayer(self.iCivSelected).getNumAvailableBonuses(iBonus) < 1:
					bCanSpellResearchEver = false
			if iAlignmentPrereq != -1:
				if iAlignmentPrereq != iAlignmentPlayer:
					bCanSpellResearchEver = false
			if iAlignmentBlocked != -1:
				if iAlignmentBlocked == iAlignmentPlayer:
					bCanSpellResearchEver = false

			if (bHasTech):
				screen.setPanelColor(szTechRecord, 85, 150, 87)
				self.aiCurrentState.append(CIV_HAS_TECH)
			elif (bIsFocus):
				screen.setPanelColor(szTechRecord, 104, 158, 165)
				self.aiCurrentState.append(CIV_IS_RESEARCHING)
			elif (bCanResearch):
				screen.setPanelColor(szTechRecord, 100, 104, 160)
				self.aiCurrentState.append(CIV_TECH_AVAILABLE)
			elif (bCanSpellResearchEver):
				screen.setPanelColor(szTechRecord, 206, 65, 69)
				self.aiCurrentState.append(CIV_HAS_BONUS)
			else:
				screen.setPanelColor(szTechRecord, 0, 0, 0)
				self.aiCurrentState.append(CIV_NO_TECH)

			szTechID = "TechID" + str(i)
			szTechString = "<font=1>"
			if ( gc.getPlayer(self.iCivSelected).isResearchingTech(i) ):
				szTechString = szTechString + str(gc.getPlayer(self.iCivSelected).getQueuePosition(i)) + ". "
			szTechString += gc.getTechInfo(i).getDescription()
			szTechString = szTechString + "</font>"
			screen.setTextAt( szTechID, szTechRecord, szTechString, CvUtil.FONT_LEFT_JUSTIFY, iX + 6 + X_INCREMENT, iY + 6, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_TECH_TREE, i, -1 )
			screen.setActivation( szTechID, ActivationTypes.ACTIVATE_MIMICPARENTFOCUS )

			szTechButtonID = "TechButtonID" + str(i)
			szButtonString = gc.getTechInfo(i).getButton()
			if not gc.getTechInfo(i).getPrereqBonus()==-1:
				szButtonString = gc.getBonusInfo(gc.getTechInfo(i).getPrereqBonus()).getButton()
			screen.addDDSGFCAt( szTechButtonID, szTechRecord, szButtonString, iX + 6, iY + 6, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_TECH_TREE, i, -1, False )

			fX = X_START

			j = 0
			k = 0

			# Unlockable units...
			for j in range( gc.getNumUnitClassInfos() ):
				eLoopUnit = gc.getCivilizationInfo(gc.getGame().getActiveCivilizationType()).getCivilizationUnits(j)
				if (eLoopUnit != -1):
					if ((gc.getUnitInfo(eLoopUnit).getPrereqCiv() == -1) or (gc.getUnitInfo(eLoopUnit).getPrereqCiv()==gc.getGame().getActiveCivilizationType())):	#modified Sephi
						if (gc.getUnitInfo(eLoopUnit).getPrereqAndTech() == i):
							szUnitButton = "Unit" + str(j)
							screen.addDDSGFCAt( szUnitButton, szTechRecord, gc.getPlayer(gc.getGame().getActivePlayer()).getUnitButton(eLoopUnit), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_UNIT, eLoopUnit, 1, True )
							fX += X_INCREMENT

			j = 0
			k = 0

			# Unlockable Spells...
			for j in range( gc.getNumSpellInfos() ):
				if gc.getSpellInfo(j).getTechPrereq()==i:
					if (gc.getSpellInfo(j).getCivilizationPrereq()==-1) or (gc.getSpellInfo(j).getCivilizationPrereq()==gc.getGame().getActiveCivilizationType()):
						szSpellButton = "Spell" + str( ( i * 1000 ) + j )
						screen.addDDSGFCAt( szSpellButton, szTechRecord, gc.getSpellInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_SPELL, j, 1, True )
						fX += X_INCREMENT

			j = 0
			k = 0
			# Unlockable Promotions...
			for j in range( gc.getNumPromotionInfos() ):
				if gc.getPromotionInfo(j).getTechPrereq()==i:
					szPromotionButton = "Promotion" + str(j)
					screen.addDDSGFCAt( szPromotionButton, szTechRecord, gc.getPromotionInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, j, 1, True )
					fX += X_INCREMENT

			j = 0
			k = 0

			iArcanePromotion=gc.getTechInfo(i).getArcaneAuraPromotion()
			if iArcanePromotion!=-1:
				szPromotionButton = "ArcaneAuraPromotion" + str(i)
				screen.addDDSGFCAt( szPromotionButton, szTechRecord, gc.getPromotionInfo(iArcanePromotion).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, iArcanePromotion, 1, True )
				fX += X_INCREMENT

			# Unlockable Buildings...
			for j in range(gc.getNumBuildingClassInfos()):
				bTechFound = 0
				eLoopBuilding = gc.getCivilizationInfo(gc.getGame().getActiveCivilizationType()).getCivilizationBuildings(j)

				if (eLoopBuilding != -1):
					if ((gc.getBuildingInfo(eLoopBuilding).getPrereqCiv() == -1) or (gc.getBuildingInfo(eLoopBuilding).getPrereqCiv()==gc.getGame().getActiveCivilizationType())):	#modified Sephi
						if (gc.getBuildingInfo(eLoopBuilding).getPrereqAndTech() == i):
							szBuildingButton = "Building" + str(j)
							screen.addDDSGFCAt( szBuildingButton, szTechRecord, gc.getBuildingInfo(eLoopBuilding).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BUILDING, eLoopBuilding, 1, True )
							fX += X_INCREMENT

			j = 0
			k = 0

			# Obsolete Buildings...
			for j in range(gc.getNumBuildingClassInfos()):
				eLoopBuilding = gc.getCivilizationInfo(gc.getPlayer(self.iCivSelected).getCivilizationType()).getCivilizationBuildings(j)

				if (eLoopBuilding != -1):
					if (gc.getBuildingInfo(eLoopBuilding).getObsoleteTech() == i):
						# Add obsolete picture here...
						szObsoleteButton = "Obsolete" + str(j)
						szObsoleteX = "ObsoleteX" + str(j)
						screen.addDDSGFCAt( szObsoleteButton, szTechRecord, gc.getBuildingInfo(eLoopBuilding).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_OBSOLETE, eLoopBuilding, -1, False )
						screen.addDDSGFCAt( szObsoleteX, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_RED_X").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_OBSOLETE, eLoopBuilding, -1, False )
						fX += X_INCREMENT

			j = 0
			k = 0

			# Obsolete Bonuses...
			for j in range(gc.getNumBonusInfos()):
				if (gc.getBonusInfo(j).getTechObsolete() == i):
					# Add obsolete picture here...
					szObsoleteButton = "ObsoleteBonus" + str(j)
					szObsoleteX = "ObsoleteXBonus" + str(j)
					screen.addDDSGFCAt( szObsoleteButton, szTechRecord, gc.getBonusInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_OBSOLETE_BONUS, j, -1, False )
					screen.addDDSGFCAt( szObsoleteX, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_RED_X").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_OBSOLETE_BONUS, j, -1, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Obsolete Monastaries...
			for j in range (gc.getNumSpecialBuildingInfos()):
				if (gc.getSpecialBuildingInfo(j).getObsoleteTech() == i):
						# Add obsolete picture here...
						szObsoleteSpecialButton = "ObsoleteSpecial" + str(j)
						szObsoleteSpecialX = "ObsoleteSpecialX" + str(j)
						screen.addDDSGFCAt( szObsoleteSpecialButton, szTechRecord, gc.getSpecialBuildingInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_OBSOLETE_SPECIAL, j, -1, False )
						screen.addDDSGFCAt( szObsoleteSpecialX, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_RED_X").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_OBSOLETE_SPECIAL, j, -1, False )
						fX += X_INCREMENT

			j = 0
			k = 0

			# Route movement change
			for j in range(gc.getNumRouteInfos()):
				if ( gc.getRouteInfo(j).getTechMovementChange(i) != 0 ):
					szMoveButton = "Move" + str(j)
					screen.addDDSGFCAt( szMoveButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_MOVE_BONUS").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_MOVE_BONUS, i, -1, False )
					fX += X_INCREMENT

	#FfH: Modified by Kael 10/01/2008
	#			j = 0
	#			k = 0
	#
	#			# Promotion Info
	#			for j in range( gc.getNumPromotionInfos() ):
	#				if ( gc.getPromotionInfo(j).getTechPrereq() == i ):
	#					szPromotionButton = "Promotion" + str(j)
	#					screen.addDDSGFCAt( szPromotionButton, szTechRecord, gc.getPromotionInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROMOTION, j, -1, False )
	#					fX += X_INCREMENT
	#
	#End Modify

			j = 0
			k = 0

			# Free unit
			if ( gc.getTechInfo(i).getFirstFreeUnitClass() != UnitClassTypes.NO_UNITCLASS ):
				szFreeUnitButton = "FreeUnit" + str(i)

				eLoopUnit = gc.getCivilizationInfo(gc.getGame().getActiveCivilizationType()).getCivilizationUnits(gc.getTechInfo(i).getFirstFreeUnitClass())
				if (eLoopUnit != -1):
					screen.addDDSGFCAt( szFreeUnitButton, szTechRecord, gc.getPlayer(gc.getGame().getActivePlayer()).getUnitButton(eLoopUnit), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_FREE_UNIT, eLoopUnit, i, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Feature production modifier
			if ( gc.getTechInfo(i).getFeatureProductionModifier() != 0 ):
				szFeatureProductionButton = "FeatureProduction" + str(i)
				screen.addDDSGFCAt( szFeatureProductionButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_FEATURE_PRODUCTION").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_FEATURE_PRODUCTION, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Worker speed
			if ( gc.getTechInfo(i).getWorkerSpeedModifier() != 0 ):
				szWorkerModifierButton = "Worker" + str(i)
				screen.addDDSGFCAt( szWorkerModifierButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_WORKER_SPEED").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_WORKER_RATE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Trade Routes per City change
			if ( gc.getTechInfo(i).getTradeRoutes() != 0 ):
				szTradeRouteButton = "TradeRoutes" + str(i)
				screen.addDDSGFCAt( szTradeRouteButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_TRADE_ROUTES").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_TRADE_ROUTES, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Health Rate bonus from this tech...
			if ( gc.getTechInfo(i).getHealth() != 0 ):
				szHealthRateButton = "HealthRate" + str(i)
				screen.addDDSGFCAt( szHealthRateButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_HEALTH").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_HEALTH_RATE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Happiness Rate bonus from this tech...
			if ( gc.getTechInfo(i).getHappiness() != 0 ):
				szHappinessRateButton = "HappinessRate" + str(i)
				screen.addDDSGFCAt( szHappinessRateButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_HAPPINESS").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_HAPPINESS_RATE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Free Techs
			if ( gc.getTechInfo(i).getFirstFreeTechs() > 0 ):
				szFreeTechButton = "FreeTech" + str(i)
				screen.addDDSGFCAt( szFreeTechButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_FREETECH").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_FREE_TECH, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Line of Sight bonus...
			if ( gc.getTechInfo(i).isExtraWaterSeeFrom() ):
				szLOSButton = "LOS" + str(i)
				screen.addDDSGFCAt( szLOSButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_LOS").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_LOS_BONUS, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Map Center Bonus...
			if ( gc.getTechInfo(i).isMapCentering() ):
				szMapCenterButton = "MapCenter" + str(i)
				screen.addDDSGFCAt( szMapCenterButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_MAPCENTER").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_MAP_CENTER, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Map Reveal...
			if ( gc.getTechInfo(i).isMapVisible() ):
				szMapRevealButton = "MapReveal" + str(i)
				screen.addDDSGFCAt( szMapRevealButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_MAPREVEAL").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_MAP_REVEAL, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Map Trading
			if ( gc.getTechInfo(i).isMapTrading() == True ):
				szMapTradeButton = "MapTrade" + str(i)
				screen.addDDSGFCAt( szMapTradeButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_MAPTRADING").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_MAP_TRADE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Tech Trading
			if ( gc.getTechInfo(i).isTechTrading() ):
				szTechTradeButton = "TechTrade" + str(i)
				screen.addDDSGFCAt( szTechTradeButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_TECHTRADING").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_TECH_TRADE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Gold Trading
			if ( gc.getTechInfo(i).isGoldTrading() ):
				szGoldTradeButton = "GoldTrade" + str(i)
				screen.addDDSGFCAt( szGoldTradeButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_GOLDTRADING").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_GOLD_TRADE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Open Borders
			if ( gc.getTechInfo(i).isOpenBordersTrading() ):
				szOpenBordersButton = "OpenBorders" + str(i)
				screen.addDDSGFCAt( szOpenBordersButton , szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_OPENBORDERS").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_OPEN_BORDERS, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Defensive Pact
			if ( gc.getTechInfo(i).isDefensivePactTrading() ):
				szDefensivePactButton = "DefensivePact" + str(i)
				screen.addDDSGFCAt( szDefensivePactButton , szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_DEFENSIVEPACT").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_DEFENSIVE_PACT, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Permanent Alliance
			if ( gc.getTechInfo(i).isPermanentAllianceTrading() ):
				szPermanentAllianceButton = "PermanentAlliance" + str(i)
				screen.addDDSGFCAt( szPermanentAllianceButton , szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_PERMALLIANCE").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_PERMANENT_ALLIANCE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Vassal States
			if ( gc.getTechInfo(i).isVassalStateTrading() ):
				szVassalStateButton = "VassalState" + str(i)
				screen.addDDSGFCAt( szVassalStateButton , szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_VASSAL").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_VASSAL_STATE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Bridge Building
			if ( gc.getTechInfo(i).isBridgeBuilding() ):
				szBuildBridgeButton = "BuildBridge" + str(i)
				screen.addDDSGFCAt( szBuildBridgeButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_BRIDGEBUILDING").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_BUILD_BRIDGE, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Irrigation unlocked...
			if ( gc.getTechInfo(i).isIrrigation() ):
				szIrrigationButton = "Irrigation" + str(i)
				screen.addDDSGFCAt( szIrrigationButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_IRRIGATION").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_IRRIGATION, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Ignore Irrigation unlocked...
			if ( gc.getTechInfo(i).isIgnoreIrrigation() ):
				szIgnoreIrrigationButton = "IgnoreIrrigation" + str(i)
				screen.addDDSGFCAt( szIgnoreIrrigationButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_NOIRRIGATION").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_IGNORE_IRRIGATION, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Coastal Work unlocked...
			if ( gc.getTechInfo(i).isWaterWork() ):
				szWaterWorkButton = "WaterWork" + str(i)
				screen.addDDSGFCAt( szWaterWorkButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_WATERWORK").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_WATER_WORK, i, -1, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Improvements
			for j in range(gc.getNumBuildInfos()):
				bTechFound = 0;

				if (gc.getBuildInfo(j).getTechPrereq() == -1):
					bTechFound = 0
					for k in range(gc.getNumFeatureInfos()):
						if (gc.getBuildInfo(j).getFeatureTech(k) == i):
							bTechFound = 1
				else:
					if (gc.getBuildInfo(j).getTechPrereq() == i):
						bTechFound = 1

				if (bTechFound == 1):
					szImprovementButton = "Improvement" + str( ( i * 1000 ) + j )
					screen.addDDSGFCAt( szImprovementButton, szTechRecord, gc.getBuildInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_IMPROVEMENT, i, j, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Domain Extra Moves
			for j in range( DomainTypes.NUM_DOMAIN_TYPES ):
				if (gc.getTechInfo(i).getDomainExtraMoves(j) != 0):
					szDomainExtraMovesButton = "DomainExtraMoves" + str( ( i * 1000 ) + j )
					screen.addDDSGFCAt( szDomainExtraMovesButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_WATERMOVES").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_DOMAIN_EXTRA_MOVES, i, j, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Adjustments
			for j in range( CommerceTypes.NUM_COMMERCE_TYPES ):
				if (gc.getTechInfo(i).isCommerceFlexible(j) and not (gc.getTeam(gc.getPlayer(self.iCivSelected).getTeam()).isCommerceFlexible(j))):
					szAdjustButton = "AdjustButton" + str( ( i * 1000 ) + j )
					if ( j == CommerceTypes.COMMERCE_CULTURE ):
						szFileName = ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_CULTURE").getPath()
					elif ( j == CommerceTypes.COMMERCE_ESPIONAGE ):
						szFileName = ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_ESPIONAGE").getPath()
					else:
						szFileName = ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_QUESTIONMARK").getPath()
					screen.addDDSGFCAt( szAdjustButton, szTechRecord, szFileName, iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_ADJUST, i, j, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Terrain opens up as a trade route
			for j in range( gc.getNumTerrainInfos() ):
				if (gc.getTechInfo(i).isTerrainTrade(j) and not (gc.getTeam(gc.getPlayer(self.iCivSelected).getTeam()).isTerrainTrade(j))):
					szTerrainTradeButton = "TerrainTradeButton" + str( ( i * 1000 ) + j )
					screen.addDDSGFCAt( szTerrainTradeButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_WATERTRADE").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_TERRAIN_TRADE, i, j, False )
					fX += X_INCREMENT

			j = gc.getNumTerrainInfos()
			if (gc.getTechInfo(i).isRiverTrade() and not (gc.getTeam(gc.getPlayer(self.iCivSelected).getTeam()).isRiverTrade())):
				szTerrainTradeButton = "TerrainTradeButton" + str( ( i * 1000 ) + j )
				screen.addDDSGFCAt( szTerrainTradeButton, szTechRecord, ArtFileMgr.getInterfaceArtInfo("INTERFACE_TECH_RIVERTRADE").getPath(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_TERRAIN_TRADE, i, j, False )
				fX += X_INCREMENT

			j = 0
			k = 0

			# Special buildings like monestaries...
			for j in range( gc.getNumSpecialBuildingInfos() ):
				if (gc.getSpecialBuildingInfo(j).getTechPrereq() == i):
					szSpecialBuilding = "SpecialBuildingButton" + str( ( i * 1000 ) + j )
					screen.addDDSGFCAt( szSpecialBuilding, szTechRecord, gc.getSpecialBuildingInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_SPECIAL_BUILDING, i, j, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Yield change
			for j in range( gc.getNumImprovementInfos() ):
				bFound = False

	#FfH: Modified by Kael 01/05/2009
	#				for k in range( YieldTypes.NUM_YIELD_TYPES ):
	#					if (gc.getImprovementInfo(j).getTechYieldChanges(i, k)):
	#						if ( bFound == False ):
	#							szYieldChange = "YieldChangeButton" + str( ( i * 1000 ) + j )
	#							screen.addDDSGFCAt( szYieldChange, szTechRecord, gc.getImprovementInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_YIELD_CHANGE, i, j, False )
	#							fX += X_INCREMENT
	#							bFound = True
				if not gc.getImprovementInfo(j).isUnique():
					for k in range( YieldTypes.NUM_YIELD_TYPES ):
						if (gc.getImprovementInfo(j).getTechYieldChanges(i, k)):
							if ( bFound == False ):
								szYieldChange = "YieldChangeButton" + str( ( i * 1000 ) + j )
								screen.addDDSGFCAt( szYieldChange, szTechRecord, gc.getImprovementInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_YIELD_CHANGE, i, j, False )
								fX += X_INCREMENT
								bFound = True
	#FfH: End Modify

			j = 0
			k = 0

			# Bonuses revealed
			for j in range( gc.getNumBonusInfos() ):
				if (gc.getBonusInfo(j).getTechReveal() == i):
					szBonusReveal = "BonusRevealButton" + str( ( i * 1000 ) + j )
					screen.addDDSGFCAt( szBonusReveal, szTechRecord, gc.getBonusInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_BONUS_REVEAL, i, j, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Civic options
			for j in range( gc.getNumCivicInfos() ):
				if (gc.getCivicInfo(j).getTechPrereq() == i):

	#FfH: Modified by Kael 07/03/2009
	#					szCivicReveal = "CivicRevealButton" + str( ( i * 1000 ) + j )
	#					screen.addDDSGFCAt( szCivicReveal, szTechRecord, gc.getCivicInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_CIVIC_REVEAL, i, j, False )
	#					fX += X_INCREMENT
					if (gc.getCivicInfo(j).getPrereqCivilization() == -1 or gc.getCivicInfo(j).getPrereqCivilization() == gc.getGame().getActiveCivilizationType()):
						szCivicReveal = "CivicRevealButton" + str( ( i * 1000 ) + j )
						screen.addDDSGFCAt( szCivicReveal, szTechRecord, gc.getCivicInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_CIVIC_REVEAL, i, j, False )
						fX += X_INCREMENT
	#FfH: End Modify

			j = 0
			k = 0

			# Projects possible
			for j in range( gc.getNumProjectInfos() ):
				if (gc.getProjectInfo(j).getTechPrereq() == i):

					if (gc.getProjectInfo(j).getPrereqCivilization() == -1 or gc.getProjectInfo(j).getPrereqCivilization() == gc.getGame().getActiveCivilizationType()):
						szProjectInfo = "ProjectInfoButton" + str( ( i * 1000 ) + j )
						screen.addDDSGFCAt( szProjectInfo, szTechRecord, gc.getProjectInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_PEDIA_JUMP_TO_PROJECT, j, 1, False )
						fX += X_INCREMENT

			j = 0
			k = 0

			# Processes possible
			for j in range( gc.getNumProcessInfos() ):
				if (gc.getProcessInfo(j).getTechPrereq() == i):
					szProcessInfo = "ProcessInfoButton" + str( ( i * 1000 ) + j )
					screen.addDDSGFCAt( szProcessInfo, szTechRecord, gc.getProcessInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_PROCESS_INFO, i, j, False )
					fX += X_INCREMENT

			j = 0
			k = 0

			# Religions unlocked
			for j in range( gc.getNumReligionInfos() ):
				if ( gc.getReligionInfo(j).getTechPrereq() == i ):
					szFoundReligion = "FoundReligionButton" + str( ( i * 1000 ) + j )
					if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_PICK_RELIGION):
						szButton = ArtFileMgr.getInterfaceArtInfo("INTERFACE_POPUPBUTTON_RELIGION").getPath()
					else:
						szButton = gc.getReligionInfo(j).getButton()
					screen.addDDSGFCAt( szFoundReligion, szTechRecord, szButton, iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_FOUND_RELIGION, i, j, False )
					fX += X_INCREMENT


			for j in range( gc.getNumCorporationInfos() ):
				if ( gc.getCorporationInfo(j).getTechPrereq() == i ):
					szFoundCorporation = "FoundCorporationButton" + str( ( i * 1000 ) + j )
					screen.addDDSGFCAt( szFoundCorporation, szTechRecord, gc.getCorporationInfo(j).getButton(), iX + fX, iY + Y_ROW, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_FOUND_CORPORATION, i, j, False )
					fX += X_INCREMENT

			screen.show( szTechRecord )

		screen.setViewMin( "TechList", iMaxX + 20, iMaxY + 20 )
		screen.show( "TechList" )
		screen.setFocus( "TechList" )


	# Will update the tech records based on color, researching, researched, queued, etc.
	def updateTechRecords (self, bForce):

		# If we are the Pitboss, we don't want to put up an interface at all
		if ( CyGame().isPitbossHost() ):
			return

		# Get the screen
		screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )

		abChanged = []
		bAnyChanged = 0

		# Go through all the techs
		for i in range(gc.getNumTechInfos()):

			abChanged.append(0)

			if(self.aiCurrentState[i] == -1):
				abChanged[i] = 0
				continue

			bHasTech = gc.getTeam(gc.getPlayer(self.iCivSelected).getTeam()).isHasTech(i)
			bIsFocus = false
			if gc.getPlayer(self.iCivSelected).getSpellResearchFocus()==i:
				bIsFocus = true
			bCanResearch = gc.getPlayer(self.iCivSelected).canSpellResearchNext(i)

			bCanSpellResearchEver = true
			iBonus = gc.getTechInfo(i).getPrereqBonus()
			iAlignmentPrereq = gc.getTechInfo(i).getPreferredAlignment()
			iAlignmentBlocked = gc.getTechInfo(i).getBlockedbyAlignment()
			iAlignmentPlayer = gc.getPlayer(self.iCivSelected).getAlignment()
			if iBonus!=-1:
				if gc.getPlayer(self.iCivSelected).getNumAvailableBonuses(iBonus)<1:
					bCanSpellResearchEver = false
			if iAlignmentPrereq!=-1:
				if iAlignmentPrereq!=iAlignmentPlayer:
					bCanSpellResearchEver = false
			if iAlignmentBlocked!=-1:
				if iAlignmentBlocked==iAlignmentPlayer:
					bCanSpellResearchEver = false

			if (bHasTech):
				if ( self.aiCurrentState[i] != CIV_HAS_TECH ):
					self.aiCurrentState[i] = CIV_HAS_TECH
					abChanged[i] = 1
					bAnyChanged = 1
			elif (bIsFocus):
				if ( self.aiCurrentState[i] != CIV_IS_RESEARCHING ):
					self.aiCurrentState[i] = CIV_IS_RESEARCHING
					abChanged[i] = 1
					bAnyChanged = 1
			elif (bCanResearch):
				if ( self.aiCurrentState[i] != CIV_TECH_AVAILABLE ):
					self.aiCurrentState[i] = CIV_TECH_AVAILABLE
					abChanged[i] = 1
					bAnyChanged = 1
			elif (bCanSpellResearchEver):
				if ( self.aiCurrentState[i] != CIV_HAS_BONUS ):
					self.aiCurrentState[i] = CIV_HAS_BONUS
					abChanged[i] = 1
					bAnyChanged = 1
			else:
				if ( self.aiCurrentState[i] != CIV_NO_TECH ):
					self.aiCurrentState[i] = CIV_NO_TECH
					abChanged[i] = 1
					bAnyChanged = 1

		for i in range(gc.getNumTechInfos()):

			if (abChanged[i] or bForce or (bAnyChanged and gc.getPlayer(self.iCivSelected).getSpellResearchFocus() == i)):
				# Create and place a tech in its proper location
				szTechRecord = "TechRecord" + str(i)
				szTechID = "TechID" + str(i)
				szTechString = "<font=1>"

				if ( gc.getPlayer(self.iCivSelected).isResearchingTech(i) ):
					szTechString = szTechString + unicode(gc.getPlayer(self.iCivSelected).getQueuePosition(i)) + ". "

				iX = 30 + ( (gc.getTechInfo(i).getGridX() - 1) * ( ( BOX_INCREMENT_X_SPACING + BOX_INCREMENT_WIDTH ) * PIXEL_INCREMENT ) )
				iY = ( gc.getTechInfo(i).getGridY() - 1 ) * ( BOX_INCREMENT_Y_SPACING * PIXEL_INCREMENT ) + 5

				szTechString += gc.getTechInfo(i).getDescription()
				if ( gc.getPlayer(self.iCivSelected).isResearchingTech(i) ):
					szTechString += " ("
					szTechString += str(gc.getPlayer(self.iCivSelected).getResearchTurnsLeft(i, ( gc.getPlayer(self.iCivSelected).getCurrentResearch() == i )))
					szTechString += ")"
				szTechString = szTechString + "</font>"
				screen.setTextAt( szTechID, "TechList", szTechString, CvUtil.FONT_LEFT_JUSTIFY, iX + 6 + X_INCREMENT, iY + 6, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_TECH_TREE, i, -1 )
				screen.setActivation( szTechID, ActivationTypes.ACTIVATE_MIMICPARENTFOCUS )

				bHasTech = gc.getTeam(gc.getPlayer(self.iCivSelected).getTeam()).isHasTech(i)
				bIsFocus = false
				if gc.getPlayer(self.iCivSelected).getSpellResearchFocus() == i:
					bIsFocus = true
				bCanResearch = gc.getPlayer(self.iCivSelected).canSpellResearchNext(i)
				bCanSpellResearchEver = true
				iBonus = gc.getTechInfo(i).getPrereqBonus()
				iAlignmentPrereq=gc.getTechInfo(i).getPreferredAlignment()
				iAlignmentBlocked=gc.getTechInfo(i).getBlockedbyAlignment()
				iAlignmentPlayer=gc.getPlayer(self.iCivSelected).getAlignment()
				if iBonus != -1:
					if gc.getPlayer(self.iCivSelected).getNumAvailableBonuses(iBonus) < 1:
						bCanSpellResearchEver = false
				if iAlignmentPrereq != -1:
					if iAlignmentPrereq != iAlignmentPlayer:
						bCanSpellResearchEver = false
				if iAlignmentBlocked != -1:
					if iAlignmentBlocked == iAlignmentPlayer:
						bCanSpellResearchEver = false


				if (bHasTech):
					screen.setPanelColor(szTechRecord, 85, 150, 87)
				elif (bIsFocus):
					screen.setPanelColor(szTechRecord, 104, 158, 165)
				elif (bCanResearch):
					screen.setPanelColor(szTechRecord, 100, 104, 160)
				elif (bCanSpellResearchEver):
					screen.setPanelColor(szTechRecord, 206, 65, 69)
				else:
					screen.setPanelColor(szTechRecord, 0, 0, 0)

	# Will draw the arrows
	def drawArrows (self, iFlag):

		screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )

		iLoop = 0
		self.nWidgetCount = 0

		ARROW_X = ArtFileMgr.getInterfaceArtInfo("ARROW_X").getPath()
		ARROW_Y = ArtFileMgr.getInterfaceArtInfo("ARROW_Y").getPath()
		ARROW_MXMY = ArtFileMgr.getInterfaceArtInfo("ARROW_MXMY").getPath()
		ARROW_XY = ArtFileMgr.getInterfaceArtInfo("ARROW_XY").getPath()
		ARROW_MXY = ArtFileMgr.getInterfaceArtInfo("ARROW_MXY").getPath()
		ARROW_XMY = ArtFileMgr.getInterfaceArtInfo("ARROW_XMY").getPath()
		ARROW_HEAD = ArtFileMgr.getInterfaceArtInfo("ARROW_HEAD").getPath()

		for i in range(gc.getNumTechInfos()):
			if gc.getTechInfo(i).getSpellSchool()==iFlag:
				bFirst = 1

				fX = (BOX_INCREMENT_WIDTH * PIXEL_INCREMENT) - 8

				for j in range( gc.getNUM_AND_TECH_PREREQS() ):

					eTech = gc.getTechInfo(i).getPrereqAndTechs(j)

					if ( eTech > -1 and gc.getTechInfo(eTech).getSpellSchool()==iFlag):

						fX = fX - X_INCREMENT

						iX = 30 + ( (gc.getTechInfo(i).getGridX() - 1) * ( ( BOX_INCREMENT_X_SPACING + BOX_INCREMENT_WIDTH ) * PIXEL_INCREMENT ) )
						iY = ( gc.getTechInfo(i).getGridY() - 1 ) * ( BOX_INCREMENT_Y_SPACING * PIXEL_INCREMENT ) + 5

						szTechPrereqID = "TechPrereqID" + str((i * 1000) + j)
						screen.addDDSGFCAt( szTechPrereqID, "TechList", gc.getTechInfo(eTech).getButton(), iX + fX, iY + 6, TEXTURE_SIZE, TEXTURE_SIZE, WidgetTypes.WIDGET_HELP_TECH_PREPREQ, eTech, -1, False )

#						szTechPrereqBorderID = "TechPrereqBorderID" + str((i * 1000) + j)
#						screen.addDDSGFCAt( szTechPrereqBorderID, "TechList", ArtFileMgr.getInterfaceArtInfo("TECH_TREE_BUTTON_BORDER").getPath(), iX + fX + 4, iY + 22, 32, 32, WidgetTypes.WIDGET_HELP_TECH_PREPREQ, eTech, -1, False )

						iX = 24 + ( (gc.getTechInfo(eTech).getGridX() - 1) * ( ( BOX_INCREMENT_X_SPACING + BOX_INCREMENT_WIDTH ) * PIXEL_INCREMENT ) )
						iY = ( gc.getTechInfo(eTech).getGridY() - 1 ) * ( BOX_INCREMENT_Y_SPACING * PIXEL_INCREMENT ) + 5

						# j is the pre-req, i is the tech...
						xDiff = gc.getTechInfo(i).getGridX() - gc.getTechInfo(eTech).getGridX()
						yDiff = gc.getTechInfo(i).getGridY() - gc.getTechInfo(eTech).getGridY()

						if (yDiff == 0):
							screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(3), self.getWidth(xDiff), 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
						elif (yDiff < 0):
							if ( yDiff == -6 ):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(1), self.getWidth(xDiff) / 2, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1) + 8 - self.getHeight(yDiff, 0), 8, self.getHeight(yDiff, 0) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1) - self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1) - self.getHeight(yDiff, 0), ( self.getWidth(xDiff) / 2 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(1) - self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							elif ( yDiff == -2 and xDiff == 2 ):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(2), self.getWidth(xDiff) * 5 / 6, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XY, iX + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2) + 8 - self.getHeight(yDiff, 3), 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XMY, iX + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), ( self.getWidth(xDiff) / 6 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							else:
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(2), self.getWidth(xDiff) / 2, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2) + 8 - self.getHeight(yDiff, 3), 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), ( self.getWidth(xDiff) / 2 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
						elif (yDiff > 0):
							if ( yDiff == 2 and xDiff == 2):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(4), self.getWidth(xDiff) / 6, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4) + 8, 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXY, iX + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), ( self.getWidth(xDiff) * 5 / 6 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							elif ( yDiff == 4 and xDiff == 1):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(5), self.getWidth(xDiff) / 3, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5) + 8, 8, self.getHeight(yDiff, 0) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXY, iX + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5) + self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5) + self.getHeight(yDiff, 0), ( self.getWidth(xDiff) * 2 / 3 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(5) + self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							else:
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(4), self.getWidth(xDiff) / 2, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4) + 8, 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), ( self.getWidth(xDiff) / 2 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )

				j = 0

				for j in range( gc.getNUM_OR_TECH_PREREQS() ):

					eTech = gc.getTechInfo(i).getPrereqOrTechs(j)

					if ( eTech > -1 ):

						iX = 24 + ( (gc.getTechInfo(eTech).getGridX() - 1) * ( ( BOX_INCREMENT_X_SPACING + BOX_INCREMENT_WIDTH ) * PIXEL_INCREMENT ) )
						iY = ( gc.getTechInfo(eTech).getGridY() - 1 ) * ( BOX_INCREMENT_Y_SPACING * PIXEL_INCREMENT ) + 5

						# j is the pre-req, i is the tech...
						xDiff = gc.getTechInfo(i).getGridX() - gc.getTechInfo(eTech).getGridX()
						yDiff = gc.getTechInfo(i).getGridY() - gc.getTechInfo(eTech).getGridY()

						if (yDiff == 0):
							screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(3), self.getWidth(xDiff), 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
						elif (yDiff < 0):
							if ( yDiff == -6 ):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(1), self.getWidth(xDiff) / 2, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1) + 8 - self.getHeight(yDiff, 0), 8, self.getHeight(yDiff, 0) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1) - self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(1) - self.getHeight(yDiff, 0), ( self.getWidth(xDiff) / 2 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(1) - self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							elif ( yDiff == -2 and xDiff == 2 ):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(2), self.getWidth(xDiff) * 5 / 6, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XY, iX + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2) + 8 - self.getHeight(yDiff, 3), 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XMY, iX + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) * 5 / 6 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), ( self.getWidth(xDiff) / 6 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							else:
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(2), self.getWidth(xDiff) / 2, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2) + 8 - self.getHeight(yDiff, 3), 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_XMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(2) - self.getHeight(yDiff, 3), ( self.getWidth(xDiff) / 2 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(2) - self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
						elif (yDiff > 0):
							if ( yDiff == 2 and xDiff == 2):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(4), self.getWidth(xDiff) / 6, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4) + 8, 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXY, iX + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 6 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), ( self.getWidth(xDiff) * 5 / 6 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							elif ( yDiff == 4 and xDiff == 1):
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(5), self.getWidth(xDiff) / 3, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5) + 8, 8, self.getHeight(yDiff, 0) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXY, iX + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5) + self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 3 ), iY + self.getYStart(5) + self.getHeight(yDiff, 0), ( self.getWidth(xDiff) * 2 / 3 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(5) + self.getHeight(yDiff, 0), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
							else:
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + self.getXStart(), iY + self.getYStart(4), self.getWidth(xDiff) / 2, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXMY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_Y, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4) + 8, 8, self.getHeight(yDiff, 3) - 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_MXY, iX + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_X, iX + 8 + self.getXStart() + ( self.getWidth(xDiff) / 2 ), iY + self.getYStart(4) + self.getHeight(yDiff, 3), ( self.getWidth(xDiff) / 2 ) - 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )
								screen.addDDSGFCAt( self.getNextWidgetName(), "TechList", ARROW_HEAD, iX + self.getXStart() + self.getWidth(xDiff), iY + self.getYStart(4) + self.getHeight(yDiff, 3), 8, 8, WidgetTypes.WIDGET_GENERAL, -1, -1, False )

		return

	def TechRecord (self, inputClass):
		return 0

	# Clicked the parent?
	def ParentClick (self, inputClass):
		return 0

	def CivDropDown( self, inputClass ):

		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED ):
			screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )
			iIndex = screen.getSelectedPullDownID("CivDropDown")
			self.iCivSelected = screen.getPullDownData("CivDropDown", iIndex)
			self.updateTechRecords(false)

	# Will handle the input for this screen...
	def handleInput (self, inputClass):

		# Get the screen
		screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )

		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
			if (inputClass.getButtonType() == WidgetTypes.WIDGET_TECH_TREE):
				self.m_bTechRecordsDirty = true
		# Advanced Start Stuff

		pPlayer = gc.getPlayer(self.iCivSelected)
		if (pPlayer.getAdvancedStartPoints() >= 0):

			# Add tech button
			if (inputClass.getFunctionName() == "AddTechButton"):
				if (pPlayer.getAdvancedStartTechCost(self.m_iSelectedTech, true) != -1):
					CyMessageControl().sendAdvancedStartAction(AdvancedStartActionTypes.ADVANCEDSTARTACTION_TECH, self.iCivSelected, -1, -1, self.m_iSelectedTech, true)	#Action, Player, X, Y, Data, bAdd
					self.m_bTechRecordsDirty = true
					self.m_bSelectedTechDirty = true

			# Tech clicked on
			elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
				if (inputClass.getButtonType() == WidgetTypes.WIDGET_TECH_TREE):
					self.m_iSelectedTech = inputClass.getData1()
					self.updateSelectedTech()

		' Calls function mapped in ArcaneScreenInputMap'
		# only get from the map if it has the key
		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_LISTBOX_ITEM_SELECTED ):
			self.CivDropDown( inputClass )
			return 1
		return 0

	def getNextWidgetName(self):
		szName = "TechArrow" + str(self.nWidgetCount)
		self.nWidgetCount += 1
		return szName

	def getXStart(self):
		return ( BOX_INCREMENT_WIDTH * PIXEL_INCREMENT )

	def getXSpacing(self):
		return ( BOX_INCREMENT_X_SPACING * PIXEL_INCREMENT )

	def getYStart(self, iY):
		return int((((BOX_INCREMENT_HEIGHT * PIXEL_INCREMENT ) / 6.0) * iY) - PIXEL_INCREMENT )

	def getWidth(self, xDiff):
		return ( ( xDiff * self.getXSpacing() ) + ( ( xDiff - 1 ) * self.getXStart() ) )

	def getHeight(self, yDiff, nFactor):
		return ( ( nFactor + ( ( abs( yDiff ) - 1 ) * 6 ) ) * PIXEL_INCREMENT )

	def update(self, fDelta):

		self.updateTechRecords(false)

		if (CyInterface().isDirty(InterfaceDirtyBits.Advanced_Start_DIRTY_BIT)):
			CyInterface().setDirty(InterfaceDirtyBits.Advanced_Start_DIRTY_BIT, false)

			if (self.m_bSelectedTechDirty):
				self.m_bSelectedTechDirty = false
				self.updateSelectedTech()

			if (self.m_bTechRecordsDirty):
				self.m_bTechRecordsDirty = false
				self.updateTechRecords(true)

			if (gc.getPlayer(self.iCivSelected).getAdvancedStartPoints() < 0):
				# hide the screen
				screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )
				screen.hide("AddTechButton")
				screen.hide("ASPointsLabel")
				screen.hide("SelectedTechLabel")

		return

	def updateSelectedTech(self):
		pPlayer = gc.getPlayer(CyGame().getActivePlayer())

		# Get the screen
		screen = CyGInterfaceScreen( "ArcaneScreen", CvScreenEnums.ARCANE_SCREEN )

		szName = ""
		iCost = 0

		if (self.m_iSelectedTech != -1):
			szName = gc.getTechInfo(self.m_iSelectedTech).getDescription()
			iCost = gc.getPlayer(CyGame().getActivePlayer()).getAdvancedStartTechCost(self.m_iSelectedTech, true)

		if iCost > 0:
			szText = u"<font=4>" + localText.getText("TXT_KEY_WB_AS_SELECTED_TECH_COST", (iCost, pPlayer.getAdvancedStartPoints())) + u"</font>"
			screen.setLabel( "ASPointsLabel", "Background", szText, CvUtil.FONT_LEFT_JUSTIFY, self.X_ADVANCED_START_TEXT, self.Y_ADD_TECH_BUTTON + 3, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		else:
			screen.hide("ASPointsLabel")

		szText = u"<font=4>"
		szText += localText.getText("TXT_KEY_WB_AS_SELECTED_TECH", (szName,))
		szText += u"</font>"
#		screen.setLabel( "SelectedTechLabel", "Background", szText, CvUtil.FONT_LEFT_JUSTIFY, self.X_ADVANCED_START_TEXT + 250, self.Y_ADD_TECH_BUTTON + 3, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

		# Want to add
		if (pPlayer.getAdvancedStartTechCost(self.m_iSelectedTech, true) != -1):
			screen.show("AddTechButton")
		else:
			screen.hide("AddTechButton")

	def onClose(self):
		pPlayer = gc.getPlayer(self.iCivSelected)
		if (pPlayer.getAdvancedStartPoints() >= 0):
			CyInterface().setDirty(InterfaceDirtyBits.Advanced_Start_DIRTY_BIT, true)
		return 0

class ArcaneScreenMaps:

	ArcaneScreenInputMap = {
		'TechRecord'			: CvArcaneScreen().TechRecord,
		'TechID'				: CvArcaneScreen().ParentClick,
		'TechPane'				: CvArcaneScreen().ParentClick,
		'TechButtonID'			: CvArcaneScreen().ParentClick,
		'TechButtonBorder'		: CvArcaneScreen().ParentClick,
		'Unit'					: CvArcaneScreen().ParentClick,
		'Building'				: CvArcaneScreen().ParentClick,
		'Obsolete'				: CvArcaneScreen().ParentClick,
		'ObsoleteX'				: CvArcaneScreen().ParentClick,
		'Move'					: CvArcaneScreen().ParentClick,
		'FreeUnit'				: CvArcaneScreen().ParentClick,
		'FeatureProduction'			: CvArcaneScreen().ParentClick,
		'Worker'				: CvArcaneScreen().ParentClick,
		'TradeRoutes'			: CvArcaneScreen().ParentClick,
		'HealthRate'			: CvArcaneScreen().ParentClick,
		'HappinessRate'			: CvArcaneScreen().ParentClick,
		'FreeTech'				: CvArcaneScreen().ParentClick,
		'LOS'					: CvArcaneScreen().ParentClick,
		'MapCenter'				: CvArcaneScreen().ParentClick,
		'MapReveal'				: CvArcaneScreen().ParentClick,
		'MapTrade'				: CvArcaneScreen().ParentClick,
		'TechTrade'				: CvArcaneScreen().ParentClick,
		'OpenBorders'		: CvArcaneScreen().ParentClick,
		'BuildBridge'			: CvArcaneScreen().ParentClick,
		'Irrigation'			: CvArcaneScreen().ParentClick,
		'Improvement'			: CvArcaneScreen().ParentClick,
		'DomainExtraMoves'			: CvArcaneScreen().ParentClick,
		'AdjustButton'			: CvArcaneScreen().ParentClick,
		'TerrainTradeButton'	: CvArcaneScreen().ParentClick,
		'SpecialBuildingButton'	: CvArcaneScreen().ParentClick,
		'YieldChangeButton'		: CvArcaneScreen().ParentClick,
		'BonusRevealButton'		: CvArcaneScreen().ParentClick,
		'CivicRevealButton'		: CvArcaneScreen().ParentClick,
		'ProjectInfoButton'		: CvArcaneScreen().ParentClick,
		'ProcessInfoButton'		: CvArcaneScreen().ParentClick,
		'FoundReligionButton'	: CvArcaneScreen().ParentClick,
		'CivDropDown'			: CvArcaneScreen().CivDropDown,
		}

