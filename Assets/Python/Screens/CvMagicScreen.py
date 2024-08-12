## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005

# Thanks to Requies and Elhoim from CivFanatics for this interface mod

from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums
import math
import CvForeignAdvisor
import DomPyHelpers

# BUG - Options - start
import BugUtil
import BugCore
import BugOptions
CityScreenOpt = BugCore.game.CityScreen
# BUG - Options - end

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

TEXTURE_SIZE = 24

PyPlayer = DomPyHelpers.DomPyPlayer
PyCity = DomPyHelpers.DomPyCity

# Debugging help
def ExoticForPrint (stuff):
	stuff = "ExoForAdv: " + stuff
	CvUtil.pyPrint (stuff)

# this class is shared by both the resource and technology foreign advisors
class CvMagicScreen:
	"Exotic Foreign Advisor Screen"

	def __init__(self):

		self.iScreen = -1
		
#		help (CyPlayer)
#		help (CyGInterfaceScreen)
		self.GLANCE_HEADER = "ForeignAdvisorGlanceHeader"
		self.GLANCE_BUTTON = "ForeignAdvisorPlusMinus"
		self.X_LINK = 0
		self.Y_LINK = 726

		self.BACKGROUND_ID = "MagicScreen Background"
		
		self.SUMMON_PANEL_ID = "SummonsPanel"
		self.SUMMON_LIST_ID = "SummonsList"

		self.MAP_MARGIN = 20

		self.TERRAFORM_PANEL_ID = "TerraformPanel"
		self.TERRAFORM_LIST_ID = "TerraformList"

		self.GE_NEW_PANEL_ID= "GE_NEWPanel"
		self.GE_NEW_LIST_ID= "GE_NEWList"		
		self.GE_CANCEL_PANEL_ID= "GE_CANCELPanel"
		self.GE_CANCEL_LIST_ID= "GE_CANCELList"
		self.GE_HOSTILE_PANEL_ID= "GE_HOSTILEPanel"
		self.GE_HOSTILE_LIST_ID= "GE_HOSTILEList"
		
		self.SCREEN_DICT = {
			"SUMMONS": 0,		
			"TERRAFORM": 1,
			"GLOBAL_ENCHANTMENTS": 2,
			"MIRACLES": 3,
			}

		self.REV_SCREEN_DICT = {}

		for key, value in self.SCREEN_DICT.items():
			self.REV_SCREEN_DICT[value] = key

		self.DRAW_DICT = {
			"SUMMONS": self.drawSummons,
			"TERRAFORM": self.drawTerraform,
			"GLOBAL_ENCHANTMENTS": self.drawGlobalenchantments,
			"MIRACLES": self.drawMiracles,			
			}

		self.TXT_KEY_DICT = {
			"SUMMONS": "TXT_KEY_MAGIC_SCREEN_SUMMONS",
			"TERRAFORM": "TXT_KEY_MAGIC_SCREEN_TERRAFORM",
			"GLOBAL_ENCHANTMENTS": "TXT_KEY_MAGIC_SCREEN_GLOBAL_ENCHANTMENTS",
			"MIRACLES": "TXT_KEY_MAGIC_SCREEN_MIRACLES",								
			}

		self.ORDER_LIST = ["SUMMONS", \
											 "TERRAFORM", \
											 "GLOBAL_ENCHANTMENTS", \
											 "MIRACLES"]

		self.iDefaultScreen = self.SCREEN_DICT["SUMMONS"]
	
#added Sephi	
				
	def setValues(self):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )		
		
		self.X_LINK = 0+screen.centerX(0)
		self.Y_LINK = screen.getYResolution()-40
		
		self.W_SCREEN = screen.getXResolution()
		self.H_SCREEN = screen.getYResolution()

		self.Z_BACKGROUND = -2.1
		self.Z_CONTROLS = self.Z_BACKGROUND - 0.2
		
		self.X_OVERVIEW_SPELLRESEARCH = screen.getXResolution()-400
		self.X_OVERVIEW_SPELLRESEARCH_TEXT = self.X_OVERVIEW_SPELLRESEARCH + 30
		self.Y_OVERVIEW_SPELLRESEARCH = 500
		self.W_OVERVIEW_SPELLRESEARCH = 400
		self.H_OVERVIEW_SPELLRESEARCH = 300
		
		self.X_OVERVIEW_MANAEFFECTS = screen.getXResolution()-400
		self.X_OVERVIEW_MANAEFFECTS_TEXT = self.X_OVERVIEW_MANAEFFECTS + 30
		self.Y_OVERVIEW_MANAEFFECTS = 0
		self.W_OVERVIEW_MANAEFFECTS = 400
		self.H_OVERVIEW_MANAEFFECTS = 500
		
		self.X_SUMMON = 100
		self.Y_SUMMON = 100
		self.W_SUMMON = screen.getXResolution()/3
		self.H_SUMMON = screen.getYResolution()/2

		self.X_CANCEL = 100+screen.getXResolution()/2
		self.Y_CANCEL = 100
		self.W_CANCEL = screen.getXResolution()/3
		self.H_CANCEL = (screen.getYResolution()/2)-100

		self.X_HOSTILE = 100+screen.getXResolution()/2
		self.Y_HOSTILE = 50+screen.getYResolution()/2
		self.W_HOSTILE = screen.getXResolution()/3
		self.H_HOSTILE = screen.getYResolution()/3
		
#added Sephi									
	
	def hideScreen (self):
	
		# Get the screen
		screen = CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN )

		# Hide the screen
		screen.hideScreen()
		
	def killScreen(self):
		if (self.iScreen >= 0):
			screen = self.getScreen()
			screen.hideScreen()
			self.iScreen = -1
		return
	
	def getScreen(self):
		return CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN)
	
	def interfaceScreen (self, iScreen):

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
		screen = CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN )
		screen.setRenderInterfaceOnly(True)
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, False)
			
		
		# Set the background and exit button, and show the screen
#Added Sephi		
#		screen.setDimensions(screen.centerX(0), screen.centerY(0), self.W_SCREEN, self.H_SCREEN)		
		screen.setDimensions(0, 0, self.W_SCREEN, self.H_SCREEN)

#end added
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()		
		
		if self.iScreen==0:				
			screen.addDrawControl( self.BACKGROUND_ID, "Art/Interface/Summoning.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		if self.iScreen==1:				
			screen.addDrawControl( self.BACKGROUND_ID, "Art/Interface/terraforming.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		if self.iScreen==2:						
			screen.addDrawControl( self.BACKGROUND_ID, "Art/Interface/GlobalEnchant.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		if self.iScreen==3:				
			screen.addDrawControl( self.BACKGROUND_ID, "Art/Interface/LightMagic.dds", 0, 0, xResolution, yResolution, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			
		# Set the background and exit button, and show the screen
		
		screen.addPanel("BottomLine", "", "", True, True, -100, screen.getYResolution()-60, screen.getXResolution()+200, 100, PanelStyles.PANEL_STYLE_MAIN)				
		
		screen.setText( "MagicScreenExit", "Background", u"<font=4>" + CyTranslator().getText("TXT_KEY_PEDIA_SCREEN_EXIT", ()).upper() + "</font>", CvUtil.FONT_RIGHT_JUSTIFY, self.W_SCREEN-30, self.H_SCREEN-40, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_CLOSE_SCREEN, -1, -1 )
		screen.setActivation( "MagicScreenExit", ActivationTypes.ACTIVATE_MIMICPARENTFOCUS )

		screen.showWindowBackground(False)
		
		self.nWidgetCount = 0
		self.nLineCount = 0
		
		CyInterface().setDirty(InterfaceDirtyBits.Foreign_Screen_DIRTY_BIT, False)
		
		# Draw leader heads
		self.drawContents(True)		
				
	# Drawing Leaderheads
	def drawContents(self, bInitial):
	
		if (self.iScreen < 0):
			return
							
		screen = CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN )

		# Header...
#		self.SCREEN_TITLE = u"<font=4b>" + localText.getText("TXT_KEY_FOREIGN_ADVISOR_TITLE", ()).upper() + u"</font>"		
#		screen.setLabel(self.getNextWidgetName(), "", self.SCREEN_TITLE, CvUtil.FONT_CENTER_JUSTIFY, self.W_SCREEN / 2, self.Y_TITLE, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
	
		if (self.REV_SCREEN_DICT.has_key(self.iScreen)):
			self.DRAW_DICT[self.REV_SCREEN_DICT[self.iScreen]] (bInitial)
		else:
			return

		# Link to other Foreign advisor screens
		self.X_EXIT = self.W_SCREEN - 10		
		self.DX_LINK = (self.X_EXIT - self.X_LINK) / (len (self.SCREEN_DICT) + 1)		
		xLink = self.DX_LINK / 2;

		for i in range (len (self.ORDER_LIST)):
			szTextId = self.getNextWidgetName()
			szScreen = self.ORDER_LIST[i]
			if (self.iScreen != self.SCREEN_DICT[szScreen]):
				screen.setText (szTextId, "", u"<font=4>" + localText.getText (self.TXT_KEY_DICT[szScreen], ()).upper() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_MAGIC_SCREEN, self.SCREEN_DICT[szScreen], -1)
			else:
				screen.setText (szTextId, "", u"<font=4>" + localText.getColorText (self.TXT_KEY_DICT[szScreen], (), gc.getInfoTypeForString ("COLOR_YELLOW")).upper() + u"</font>", CvUtil.FONT_CENTER_JUSTIFY, xLink, self.Y_LINK, 0, FontTypes.TITLE_FONT, WidgetTypes.WIDGET_MAGIC_SCREEN, -1, -1)
			xLink += self.DX_LINK
																
		
	def drawSummons (self, bInitial):
#		ExoticForPrint ("Entered drawGlance")

		screen = CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN )
		
		screen.addPanel(self.SUMMON_PANEL_ID, "", "", True, True, self.X_SUMMON, self.Y_SUMMON, self.W_SUMMON, self.H_SUMMON, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
	
		screen.addListBoxGFC(self.SUMMON_LIST_ID, "", self.X_SUMMON+self.MAP_MARGIN, self.Y_SUMMON+self.MAP_MARGIN+15, self.W_SUMMON-2*self.MAP_MARGIN, self.H_SUMMON-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.SUMMON_LIST_ID, False)
		screen.setStyle(self.SUMMON_LIST_ID, "Table_StandardCiv_Style")

		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
		iMana = pPlayer.getMana()
		iCapitalID = -1 		
		if not pPlayer.getCapitalCity().isNone():
			iCapitalID = pPlayer.getCapitalCity().getID()
			
		eTeam = gc.getTeam(pPlayer.getTeam())
		for i in range(gc.getNumProjectInfos()):
			if gc.getProjectInfo(i).getTechPrereq==-1 or eTeam.isHasTech(gc.getProjectInfo(i).getTechPrereq()):
				eUnit = gc.getProjectInfo(i).getSummonUnitType()	
				if eUnit!=-1:
					szDescription = gc.getProjectInfo(i).getDescription()
					szDescription = szDescription + "("+str(pPlayer.getMagicRitualTurnsNeeded(i,false))+" Turns)"
					if (not gc.getPlayer(gc.getGame().getActivePlayer()).canCreateSummon(i)) or pPlayer.getCurrentMagicRitual()==i or pPlayer.getDisableSpellcasting() > 0:
						szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_RED"))
					else:
						szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_GREEN"))				
					screen.appendListBoxString(self.SUMMON_LIST_ID, szDescription, WidgetTypes.WIDGET_DO_SUMMON, eUnit, iCapitalID, CvUtil.FONT_LEFT_JUSTIFY)

				eUnit = gc.getProjectInfo(i).getDevilSummon()					
				if eUnit!=-1:
					if gc.getProjectInfo(i).getPrereqCivilization()==gc.getPlayer(gc.getGame().getActivePlayer()).getCivilizationType():
						szDescription = gc.getProjectInfo(i).getDescription()
						szDescription = szDescription + "("+str(pPlayer.getMagicRitualTurnsNeeded(i,false))+" Turns)"						
						if (not gc.getPlayer(gc.getGame().getActivePlayer()).canDoGlobalEnchantment(i)) or pPlayer.getCurrentMagicRitual()==i or pPlayer.getDisableSpellcasting() > 0:
							szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_RED"))
						else:
							szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_GREEN"))				
						screen.appendListBoxString(self.SUMMON_LIST_ID, szDescription, WidgetTypes.WIDGET_DO_GLOBAL_ENCHANTMENT, i, iCapitalID, CvUtil.FONT_LEFT_JUSTIFY)

					
	
	def drawTerraform(self, bInitial):
		screen = CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN )
	
		screen.addPanel(self.TERRAFORM_PANEL_ID, "", "", True, True, self.X_SUMMON, self.Y_SUMMON, self.W_SUMMON, self.H_SUMMON, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
	
#		szText = localText.getText("TXT_KEY_PEDIA_ALL_UNITS", ()).upper()

		screen.addListBoxGFC(self.TERRAFORM_LIST_ID, "", self.X_SUMMON+self.MAP_MARGIN, self.Y_SUMMON+self.MAP_MARGIN+15, self.W_SUMMON-2*self.MAP_MARGIN, self.H_SUMMON-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.TERRAFORM_LIST_ID, False)
		screen.setStyle(self.TERRAFORM_LIST_ID, "Table_StandardCiv_Style")
#		screen.appendListBoxString(self.SUMMON_LIST_ID, szText, WidgetTypes.WIDGET_GENERAL, 1, -1, CvUtil.FONT_LEFT_JUSTIFY)

		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
		iMana = pPlayer.getMana()
		iCapitalID = -1 		
		if not pPlayer.getCapitalCity().isNone():
			iCapitalID = pPlayer.getCapitalCity().getID()
			
		eTeam = gc.getTeam(pPlayer.getTeam())
		for iPass in range(2):
			szText=""
			if iPass==0:
				szText = "Enhancing Terraforming"
			if iPass==1:
				szText = "Hostile Terraforming"				
			screen.appendListBoxString(self.TERRAFORM_LIST_ID, szText, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)					
			for i in range(gc.getNumProjectInfos()):
				if (gc.getProjectInfo(i).isHostileTerraform() and iPass==1) or ((not gc.getProjectInfo(i).isHostileTerraform()) and iPass==0):					
					if gc.getProjectInfo(i).getTechPrereq==-1 or eTeam.isHasTech(gc.getProjectInfo(i).getTechPrereq()):
						if gc.getProjectInfo(i).getNumTerrainChanges()>0 and gc.getProjectInfo(i).getManaCost()!=-1:	#need to add: only show summons that are unlocked by Spellresearch
							szDescription = gc.getProjectInfo(i).getDescription()
							szDescription = szDescription + "("+str(pPlayer.getMagicRitualTurnsNeeded(i,false))+" Turns)"							
							if (not gc.getPlayer(gc.getGame().getActivePlayer()).canDoTerraformRitual(i)) or pPlayer.getCurrentMagicRitual()==i or pPlayer.getDisableSpellcasting() > 0:
								szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_RED"))
							else:
								szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_GREEN"))				
							screen.appendListBoxString(self.TERRAFORM_LIST_ID, szDescription, WidgetTypes.WIDGET_DO_TERRAFORM, i, iCapitalID, CvUtil.FONT_LEFT_JUSTIFY)
		
		screen.setActivation("szDescription", ActivationTypes.ACTIVATE_MIMICPARENTFOCUS )					
		
	def drawGlobalenchantments(self, bInitial):
		screen = CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN )
		
		screen.addPanel(self.GE_NEW_PANEL_ID, "", "", True, True, self.X_SUMMON, self.Y_SUMMON, self.W_SUMMON, self.H_SUMMON, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
		screen.addPanel(self.GE_CANCEL_PANEL_ID, "", "", True, True, self.X_CANCEL, self.Y_CANCEL, self.W_CANCEL, self.H_CANCEL, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
		screen.addPanel(self.GE_HOSTILE_PANEL_ID, "", "", True, True, self.X_HOSTILE, self.Y_HOSTILE, self.W_HOSTILE, self.H_HOSTILE, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
		
		screen.addListBoxGFC(self.GE_NEW_LIST_ID, "", self.X_SUMMON+self.MAP_MARGIN, self.Y_SUMMON+self.MAP_MARGIN+15, self.W_SUMMON-2*self.MAP_MARGIN, self.H_SUMMON-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.GE_NEW_LIST_ID, False)
		screen.setStyle(self.GE_NEW_LIST_ID, "Table_StandardCiv_Style")

		screen.addListBoxGFC(self.GE_CANCEL_LIST_ID, "", self.X_CANCEL+self.MAP_MARGIN, self.Y_CANCEL+self.MAP_MARGIN+15, self.W_CANCEL-2*self.MAP_MARGIN, self.H_CANCEL-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.GE_CANCEL_LIST_ID, False)
		screen.setStyle(self.GE_CANCEL_LIST_ID, "Table_StandardCiv_Style")

		screen.addListBoxGFC(self.GE_HOSTILE_LIST_ID, "", self.X_HOSTILE+self.MAP_MARGIN, self.Y_HOSTILE+self.MAP_MARGIN+15, self.W_HOSTILE-2*self.MAP_MARGIN, self.H_HOSTILE-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.GE_HOSTILE_LIST_ID, False)
		screen.setStyle(self.GE_HOSTILE_LIST_ID, "Table_StandardCiv_Style")
		
		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())		
		iMana = pPlayer.getMana()
		pTeam = gc.getTeam(pPlayer.getTeam())
		iCapitalID = -1 		
		if not pPlayer.getCapitalCity().isNone():
			iCapitalID = pPlayer.getCapitalCity().getID()
			
		eTeam = gc.getTeam(gc.getPlayer(gc.getGame().getActivePlayer()).getTeam())
		for iPass in range(2):
			szText=""
			if iPass==0:
				szText = "Empowering Enchantments"
			if iPass==1:
				szText = "Hostile Enchantments"	
			bFirst=True				
			for i in range(gc.getNumProjectInfos()):
				if (gc.getProjectInfo(i).isHostile() and iPass==1) or ((not gc.getProjectInfo(i).isHostile()) and iPass==0):
					if gc.getProjectInfo(i).getTechPrereq()==-1 or eTeam.isHasTech(gc.getProjectInfo(i).getTechPrereq()):
						if gc.getProjectInfo(i).getPrereqCivilization()==-1 or gc.getProjectInfo(i).getPrereqCivilization()==gc.getPlayer(gc.getGame().getActivePlayer()).getCivilizationType():
							if gc.getProjectInfo(i).isGlobalEnchantment() and pTeam.getProjectCount(i)==0:
								if gc.getProjectInfo(i).getFaithCost()==0 and gc.getProjectInfo(i).getFaithUpkeep()==0:
									if bFirst:
										bFirst=false
										screen.appendListBoxString(self.GE_NEW_LIST_ID, szText, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
										
									szDescription = "      " + gc.getProjectInfo(i).getDescription()
									szDescription = szDescription + "("+str(pPlayer.getMagicRitualTurnsNeeded(i,false))+" Turns)"								
									if (not gc.getPlayer(gc.getGame().getActivePlayer()).canDoGlobalEnchantment(i)) or pPlayer.getCurrentMagicRitual()==i or pPlayer.getDisableSpellcasting() > 0:
										szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_RED"))												
									else:
										szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_GREEN"))								
									screen.appendListBoxString(self.GE_NEW_LIST_ID, szDescription, WidgetTypes.WIDGET_DO_GLOBAL_ENCHANTMENT, i, iCapitalID, CvUtil.FONT_LEFT_JUSTIFY)		

		for i in range(gc.getNumProjectInfos()):
			if gc.getProjectInfo(i).isGlobalEnchantment() and pTeam.getProjectCount(i)==1:
				if gc.getProjectInfo(i).getFaithCost()==0 and gc.getProjectInfo(i).getFaithUpkeep()==0:			
					szDescription = gc.getProjectInfo(i).getDescription()
					szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_GREEN"))				
					screen.appendListBoxString(self.GE_CANCEL_LIST_ID, szDescription, WidgetTypes.WIDGET_REMOVE_GLOBAL_ENCHANTMENT, i, iCapitalID, CvUtil.FONT_LEFT_JUSTIFY)		

		for i in range(gc.getNumProjectInfos()):
			for j in range(gc.getMAX_CIV_TEAMS()):
				if pTeam.isAtWar(j):
					if gc.getProjectInfo(i).isGlobalEnchantment() and gc.getProjectInfo(i).isHostile() and gc.getTeam(j).getProjectCount(i)==1:
						if gc.getProjectInfo(i).getFaithCost()==0 and gc.getProjectInfo(i).getFaithUpkeep()==0 or pPlayer.getDisableSpellcasting() > 0:
							szDescription = gc.getProjectInfo(i).getDescription()
							szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_RED"))				
							screen.appendListBoxString(self.GE_HOSTILE_LIST_ID, szDescription, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)		

	def drawMiracles(self, bInitial):
		screen = CyGInterfaceScreen( "MagicScreen", CvScreenEnums.MAGIC_SCREEN )
		
		screen.addPanel(self.GE_NEW_PANEL_ID, "", "", True, True, self.X_SUMMON, self.Y_SUMMON, self.W_SUMMON, self.H_SUMMON, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
		screen.addPanel(self.GE_CANCEL_PANEL_ID, "", "", True, True, self.X_CANCEL, self.Y_CANCEL, self.W_CANCEL, self.H_CANCEL, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
		screen.addPanel(self.GE_HOSTILE_PANEL_ID, "", "", True, True, self.X_HOSTILE, self.Y_HOSTILE, self.W_HOSTILE, self.H_HOSTILE, PanelStyles.PANEL_STYLE_MAIN_BLACK50)		
		
		screen.addListBoxGFC(self.GE_NEW_LIST_ID, "", self.X_SUMMON+self.MAP_MARGIN, self.Y_SUMMON+self.MAP_MARGIN+15, self.W_SUMMON-2*self.MAP_MARGIN, self.H_SUMMON-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.GE_NEW_LIST_ID, False)
		screen.setStyle(self.GE_NEW_LIST_ID, "Table_StandardCiv_Style")

		screen.addListBoxGFC(self.GE_CANCEL_LIST_ID, "", self.X_CANCEL+self.MAP_MARGIN, self.Y_CANCEL+self.MAP_MARGIN+15, self.W_CANCEL-2*self.MAP_MARGIN, self.H_CANCEL-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.GE_CANCEL_LIST_ID, False)
		screen.setStyle(self.GE_CANCEL_LIST_ID, "Table_StandardCiv_Style")

		screen.addListBoxGFC(self.GE_HOSTILE_LIST_ID, "", self.X_HOSTILE+self.MAP_MARGIN, self.Y_HOSTILE+self.MAP_MARGIN+15, self.W_HOSTILE-2*self.MAP_MARGIN, self.H_HOSTILE-2*self.MAP_MARGIN-15, TableStyles.TABLE_STYLE_STANDARD)
		screen.enableSelect(self.GE_HOSTILE_LIST_ID, False)
		screen.setStyle(self.GE_HOSTILE_LIST_ID, "Table_StandardCiv_Style")
		
		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())		
		iMana = pPlayer.getMana()
		pTeam = gc.getTeam(pPlayer.getTeam())
		iCapitalID = -1 		
		if not pPlayer.getCapitalCity().isNone():
			iCapitalID = pPlayer.getCapitalCity().getID()
			
		eTeam = gc.getTeam(gc.getPlayer(gc.getGame().getActivePlayer()).getTeam())
		for iPass in range(2):
			szText=""
			if iPass==0:
				szText = "Empowering Miracles"
			if iPass==1:
				szText = "Hostile Miracles"
			bFirst=True
			for i in range(gc.getNumProjectInfos()):
				if (gc.getProjectInfo(i).isHostile() and iPass==1) or ((not gc.getProjectInfo(i).isHostile()) and iPass==0):
					if gc.getProjectInfo(i).getTechPrereq()==-1 or eTeam.isHasTech(gc.getProjectInfo(i).getTechPrereq()):
						if gc.getProjectInfo(i).getPrereqCivilization()==-1 or gc.getProjectInfo(i).getPrereqCivilization()==gc.getPlayer(gc.getGame().getActivePlayer()).getCivilizationType():
							if gc.getProjectInfo(i).isGlobalEnchantment() and pTeam.getProjectCount(i)==0:
								if not (gc.getProjectInfo(i).getFaithCost()==0 and gc.getProjectInfo(i).getFaithUpkeep()==0):							
									if bFirst:
										bFirst=false
										screen.appendListBoxString(self.GE_NEW_LIST_ID, szText, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
									szDescription = "      " + gc.getProjectInfo(i).getDescription()
									szDescription = szDescription + "("+str(pPlayer.getMagicRitualTurnsNeeded(i,false))+" Turns)"
									#SpyFanatic: dont show miracle if victory option not enabled
									if (not gc.getPlayer(gc.getGame().getActivePlayer()).canDoGlobalEnchantment(i)) or pPlayer.getCurrentMagicRitual()==i or pPlayer.getDisableSpellcasting() > 0 or (gc.getProjectInfo(i).getVictoryPrereq() >= 0 and not CyGame().isVictoryValid(gc.getProjectInfo(i).getVictoryPrereq())):
										szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_RED"))
									else:
										szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_GREEN"))
									screen.appendListBoxString(self.GE_NEW_LIST_ID, szDescription, WidgetTypes.WIDGET_DO_GLOBAL_ENCHANTMENT, i, iCapitalID, CvUtil.FONT_LEFT_JUSTIFY)

		for i in range(gc.getNumProjectInfos()):
			if gc.getProjectInfo(i).isGlobalEnchantment() and pTeam.getProjectCount(i)==1:
				if not (gc.getProjectInfo(i).getFaithCost()==0 and gc.getProjectInfo(i).getFaithUpkeep()==0):							
					szDescription = gc.getProjectInfo(i).getDescription()
					szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_GREEN"))				
					screen.appendListBoxString(self.GE_CANCEL_LIST_ID, szDescription, WidgetTypes.WIDGET_REMOVE_GLOBAL_ENCHANTMENT, i, iCapitalID, CvUtil.FONT_LEFT_JUSTIFY)		

		for i in range(gc.getNumProjectInfos()):
			for j in range(gc.getMAX_CIV_TEAMS()):
				if pTeam.isAtWar(j):
					if gc.getProjectInfo(i).isGlobalEnchantment() and gc.getProjectInfo(i).isHostile() and gc.getTeam(j).getProjectCount(i)==1:
						if not (gc.getProjectInfo(i).getFaithCost()==0 and gc.getProjectInfo(i).getFaithUpkeep()==0) or pPlayer.getDisableSpellcasting() > 0:
							szDescription = gc.getProjectInfo(i).getDescription()
							szDescription = localText.changeTextColor(szDescription, gc.getInfoTypeForString("COLOR_RED"))				
							screen.appendListBoxString(self.GE_HOSTILE_LIST_ID, szDescription, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)		
						
	def update(self, fDelta):
		
		return
				
	# Handles the input for this screen...
	def handleInput (self, inputClass):
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
			if (inputClass.getButtonType() == WidgetTypes.WIDGET_LEADERHEAD):
				if (inputClass.getFlags() & MouseFlags.MOUSE_LBUTTONUP):
					self.iSelectedLeader = inputClass.getData1()
					self.drawContents(False)
				elif (inputClass.getFlags() & MouseFlags.MOUSE_RBUTTONUP):
					if inputClass.getData1() != self.iActiveLeader:
						self.getScreen().hideScreen()
			elif (inputClass.getFunctionName() == self.GLANCE_BUTTON):
				self.handlePlusMinusToggle()
############################################
### BEGIN CHANGES ENHANCED INTERFACE MOD ###
############################################
#			elif (inputClass.getButtonType() == WidgetTypes.WIDGET_GENERAL):
#				if (inputClass.getData1() == self.SCROLL_TABLE_UP):
#					self.scrollTradeTableUp()
#				elif (inputClass.getData1() == self.SCROLL_TABLE_DOWN):
#					self.scrollTradeTableDown()
#			elif (inputClass.getButtonType() == WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS):
##				ExoticForPrint ("FOOOOOO!!!!")
#				pass
##########################################
### END CHANGES ENHANCED INTERFACE MOD ###
##########################################
			 
		elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CHARACTER):
			if (inputClass.getData() == int(InputTypes.KB_LSHIFT) or inputClass.getData() == int(InputTypes.KB_RSHIFT)):
				self.iShiftKeyDown = inputClass.getID() 

		return 0

	def getNextWidgetName(self):
		szName = "TechArrow" + str(self.nWidgetCount)
		self.nWidgetCount += 1
		return szName
		
