## Sid Meier's Civilization 4
## Copyright Firaxis Games 2005
from CvPythonExtensions import *
import CvUtil
import ScreenInput
import CvScreenEnums
import CvEventInterface
import time
import sys
import math

#Counter display
import CvScions
#import CvUnblemishedEvents
import CvAristrakh
#Counter display

# BUG - Options - start
import BugCore
import BugOptions
ClockOpt = BugCore.game.NJAGC
ScoreOpt = BugCore.game.Scores
MainOpt = BugCore.game.MainInterface
CityScreenOpt = BugCore.game.CityScreen
# BUG - Options - end

# BUG - PLE - start
import MonkeyTools as mt
import string
from AStarTools import *
import PyHelpers
PyPlayer = PyHelpers.PyPlayer

PleOpt = BugCore.game.PLE
# BUG - PLE - end

# BUG - Align Icons - start
import Scoreboard
import PlayerUtil
# BUG - Align Icons - end

# globals
gc = CyGlobalContext()
ArtFileMgr = CyArtFileMgr()
localText = CyTranslator()

##################################################################################################
# Winamp Start
##################################################################################################

CUP = CyUserProfile()
CUP.setMasterNoSound(false)
WinAMP = gc.getWinamp()
g_WinAMP = true
g_szWinampText = ""

##################################################################################################
# Winamp End
##################################################################################################

bshowManaBar = 0
iShowItem = 0

#Added for HUD Modification by seZereth
#####################################   CITY SECTION ##############################################
#Top Bar, stretching from Treasury to Clock
HUD_City_Top_Panel_Height = 41

#Behind Mini-Map & Advisor Buttons
HUD_City_Bottom_Right_Width = 243
HUD_City_Bottom_Right_Height = 186

#Behind Build Order Queue
HUD_City_Bottom_Left_Width = 243
HUD_City_Bottom_Left_Height = 186

#Behind Build Options, Width is automatically defined to stretch between the above 2 Panels
HUD_City_Bottom_Center_Height = 149

#Width for Background of 3 Sections, City Status (Science/Culture/Gold Sliders, Maintenance Costs - Fixed Height), Trade Routes Panel (fixed Height) and Buildings Panel (Stretches to connect Trade Routes Panel to the Bottom Left Panel)  --  Background will extend slightly into the Minimap Panel (City Bottom Right)
HUD_City_Left_Panel_Width = 258

#Label for Buildings Section
HUD_City_Buildings_Label_Height = 30
HUD_City_Buildings_Label_Width = 238

#Label for Trade Routes Section
HUD_City_TradeRoute_Label_Height = 30
HUD_City_TradeRoute_Label_Width = 238

#Panel for City Status Section
HUD_City_Status_Panel_Height = 105
HUD_City_Status_Panel_Width = 238

#Width for Background of 3 Sections, Religions/Corporations Panel (Fixed Height), Resources Panel (Stretches to Connect Religions/Corporations Panel to Specialists Panel) and Specialists Panel (Fixed Height)  --  Background will extend slightly into the Minimap Panel (City Bottom Right)
HUD_City_Right_Panel_Width = 258

#Behind the Food/Production Progress Bars.    Defined as a Width Exclusion since it is a centered item, so this number is how far it is from each edge of the screen.
HUD_City_Growth_Panel_Width_Exclusion = 260
HUD_City_Growth_Panel_Height = 60

#Background for above 2 sections is automatically defined: Stretches to connect City Left Panel and City Right Panel and Stretches from the Top bar to the Bottom Center bar  -- City_Top_Center_Background_Panel.tga

#Extra Panel to place anywhere you desire
HUD_City_Extra_1_Location_X = 10
HUD_City_Extra_1_Location_Y = 10
HUD_City_Extra_1_Height = 10
HUD_City_Extra_1_Width = 10

#Behind the City Name, Arrows to Cycle through Multiple Cities, and Defense Percentage.    Defined as a Width Exclusion since it is a centered item, so this number is how far it is from each edge of the screen.
HUD_City_Name_Panel_Width_Exclusion = 260
HUD_City_Name_Panel_Height = 38

#####################################   MAIN SECTION ##############################################


#Behind Mini-Map
#HUD_Main_Bottom_Right_Width = 243
#HUD_Main_Bottom_Right_Height = 186
HUD_Main_Bottom_Right_Width = 330
HUD_Main_Bottom_Right_Height = 186


#Main Panel Behind Unit Stats
#HUD_Main_Bottom_Left_Width = 243 modified for BUG
HUD_Main_Bottom_Left_Width = 300
HUD_Main_Bottom_Left_Height = 186



#Behind Action Buttons (Stretches to connect Bottom Right to Bottom Left)
HUD_Main_Bottom_Center_Height = 149

#Behind Treasury and Log Button
HUD_Main_Top_Left_Width = 286
#HUD_Main_Top_Left_Height = 60
HUD_Main_Top_Left_Height = 100

#Behind  GameClock and Advisor Buttons
HUD_Main_Top_Right_Width = 286
HUD_Main_Top_Right_Height = 60

#Behind Tech Progress Bar (Stretches to Connect Top Right and Top Left)
HUD_Main_Top_Center_Height = 60

#Extra Panel to place anywhere you desire
HUD_Main_Extra_1_Location_X = 10
HUD_Main_Extra_1_Location_Y = 10
HUD_Main_Extra_1_Height = 10
HUD_Main_Extra_1_Width = 10

#FfH: Added by Kael 10/29/2007
manaTypes1 = [ 'BONUS_MANA_AIR','BONUS_MANA_CHAOS','BONUS_MANA_DEATH','BONUS_MANA_FIRE']
manaTypes2 = [ 'BONUS_MANA_LIFE','BONUS_MANA_MIND','BONUS_MANA_NATURE','BONUS_MANA_ICE' ]
#FfH: End Add

# BUG - 3.17 No Espionage - start
import BugUtil
# BUG - 3.17 No Espionage - end

# BUG - Great General Bar - start
import GGUtil
# BUG - Great General Bar - end

# BUG - Great Person Bar - start
import GPUtil
GP_BAR_WIDTH = 320
# BUG - Great Person Bar - end

# BUG - Progress Bar - Tick Marks - start
import ProgressBarUtil
# BUG - Progress Bar - Tick Marks - end

g_NumEmphasizeInfos = 0
g_NumCityTabTypes = 0
g_NumHurryInfos = 0
g_NumUnitClassInfos = 0
g_NumBuildingClassInfos = 0
g_NumProjectInfos = 0
g_NumProcessInfos = 0
g_NumActionInfos = 0
g_eEndTurnButtonState = -1

# BUG - city specialist - start
g_iSuperSpecialistCount = 0
g_iCitySpecialistCount = 0
g_iAngryCitizensCount = 0
SUPER_SPECIALIST_STACK_WIDTH = 15
SPECIALIST_ROW_HEIGHT = 34
SPECIALIST_ROWS = 3
MAX_SPECIALIST_BUTTON_SPACING = 30
SPECIALIST_AREA_MARGIN = 45
# BUG - city specialist - end

MAX_SELECTED_TEXT = 5
MAX_DISPLAYABLE_BUILDINGS = 15
MAX_DISPLAYABLE_TRADE_ROUTES = 4
MAX_BONUS_ROWS = 10
MAX_CITIZEN_BUTTONS = 8

SELECTION_BUTTON_COLUMNS = 8
SELECTION_BUTTON_ROWS = 2
NUM_SELECTION_BUTTONS = SELECTION_BUTTON_ROWS * SELECTION_BUTTON_COLUMNS

g_iNumBuildingWidgets = MAX_DISPLAYABLE_BUILDINGS
g_iNumTradeRouteWidgets = MAX_DISPLAYABLE_TRADE_ROUTES

# END OF TURN BUTTON POSITIONS
######################
iEndOfTurnButtonSize = 64

#FfH: Modified by Kael 07/18/2008
#iEndOfTurnPosX = 296 # distance from right
iEndOfTurnPosX = 296 # distance from right
#FfH: End Modify

iEndOfTurnPosY = 147 # distance from bottom

# MINIMAP BUTTON POSITIONS
######################
iMinimapButtonsExtent = 228
iMinimapButtonsX = 227
iMinimapButtonsY_Regular = 160
iMinimapButtonsY_Minimal = 32
iMinimapButtonWidth = 24
iMinimapButtonHeight = 24

# Globe button
iGlobeButtonX = 48
iGlobeButtonY_Regular = 168
iGlobeButtonY_Minimal = 40
iGlobeToggleWidth = 48
iGlobeToggleHeight = 48

# GLOBE LAYER OPTION POSITIONING
######################
iGlobeLayerOptionsX  = 235
iGlobeLayerOptionsY_Regular  = 170# distance from bottom edge
iGlobeLayerOptionsY_Minimal  = 38 # distance from bottom edge
iGlobeLayerOptionsWidth = 400
iGlobeLayerOptionHeight = 24

# STACK BAR
#####################
iStackBarHeight = 27


# MULTI LIST
#####################

#FfH: Modified by Kael 07/17/2008
iMultiListXL = 318
iMultiListXR = 332
#iMultiListXL = 250
#iMultiListXR = 236
#FfH: End Modify

# TOP CENTER TITLE
#####################
iCityCenterRow1X = 398
iCityCenterRow1Y = 78
iCityCenterRow2X = 398
iCityCenterRow2Y = 104

iCityCenterRow1Xa = 347
iCityCenterRow2Xa = 482


g_iNumTradeRoutes = 0
g_iNumBuildings = 0
g_iNumLeftBonus = 0
g_iNumCenterBonus = 0
g_iNumRightBonus = 0

g_szTimeText = ""

# BUG - NJAGC - start
g_bShowTimeTextAlt = False
g_iTimeTextCounter = -1
# BUG - NJAGC - end

# BUG - Raw Yields - start
import RawYields
g_bRawShowing = False
g_bYieldView, g_iYieldType = RawYields.getViewAndType(0)
g_iYieldTiles = RawYields.WORKED_TILES
RAW_YIELD_HELP = ( "TXT_KEY_RAW_YIELD_VIEW_TRADE",
				   "TXT_KEY_RAW_YIELD_VIEW_FOOD",
				   "TXT_KEY_RAW_YIELD_VIEW_PRODUCTION",
				   "TXT_KEY_RAW_YIELD_VIEW_COMMERCE",
				   "TXT_KEY_RAW_YIELD_TILES_WORKED",
				   "TXT_KEY_RAW_YIELD_TILES_CITY",
				   "TXT_KEY_RAW_YIELD_TILES_OWNED",
				   "TXT_KEY_RAW_YIELD_TILES_ALL" )
# BUG - Raw Yields - end

# BUG - field of view slider - start
DEFAULT_FIELD_OF_VIEW = 42
# BUG - field of view slider - end

HELP_TEXT_MINIMUM_WIDTH = 300

g_pSelectedUnit = 0

#FfH: Added by Kael 07/17/2008
iHelpX = 120
#FfH: End Add

class CvMainInterface:
	"Main Interface Screen"

	def __init__(self):

# BUG - PLE - start
		self.PLOT_LIST_BUTTON_NAME 		= "MyPlotListButton"
		self.PLOT_LIST_MINUS_NAME 		= "MyPlotListMinus"
		self.PLOT_LIST_PLUS_NAME 		= "MyPlotListPlus"
		self.PLOT_LIST_UP_NAME 			= "MyPlotListUp"
		self.PLOT_LIST_DOWN_NAME 		= "MyPlotListDown"
		self.PLOT_LIST_PROMO_NAME 		= "MyPlotListPromo"
		self.PLOT_LIST_UPGRADE_NAME 	= "MyPlotListUpgrade"

		self.PLE_VIEW_MODE 		 	   	= "PLE_VIEW_MODE1"
		self.PLE_MODE_STANDARD 			= "PLE_MODE_STANDARD1"
		self.PLE_MODE_MULTILINE 		= "PLE_MODE_MULTILINE1"
		self.PLE_MODE_STACK_VERT 		= "PLE_MODE_STACK_VERT1"
		self.PLE_MODE_STACK_HORIZ 		= "PLE_MODE_STACK_HORIZ1"
		self.PLE_VIEW_MODES = ( self.PLE_MODE_STANDARD,
							    self.PLE_MODE_MULTILINE,
							    self.PLE_MODE_STACK_VERT,
							    self.PLE_MODE_STACK_HORIZ )
		self.PLE_VIEW_MODE_CYCLE = { self.PLE_MODE_STANDARD : self.PLE_MODE_MULTILINE,
									 self.PLE_MODE_MULTILINE : self.PLE_MODE_STACK_VERT,
									 self.PLE_MODE_STACK_VERT : self.PLE_MODE_STACK_HORIZ,
									 self.PLE_MODE_STACK_HORIZ : self.PLE_MODE_STANDARD }
		self.PLE_VIEW_MODE_ART = { self.PLE_MODE_STANDARD : "PLE_MODE_STANDARD",
									 self.PLE_MODE_MULTILINE : "PLE_MODE_MULTILINE",
									 self.PLE_MODE_STACK_VERT : "PLE_MODE_STACK_VERT",
									 self.PLE_MODE_STACK_HORIZ : "PLE_MODE_STACK_HORIZ" }

		self.PLE_RESET_FILTERS			= "PLE_RESET_FILTERS1"
		self.PLE_FILTER_NOTWOUND		= "PLE_FILTER_NOTWOUND1"
		self.PLE_FILTER_WOUND			= "PLE_FILTER_WOUND1"
		self.PLE_FILTER_LAND			= "PLE_FILTER_LAND1"
		self.PLE_FILTER_SEA				= "PLE_FILTER_SEA1"
		self.PLE_FILTER_AIR				= "PLE_FILTER_AIR1"
		self.PLE_FILTER_MIL				= "PLE_FILTER_MIL1"
		self.PLE_FILTER_DOM				= "PLE_FILTER_DOM1"
		self.PLE_FILTER_OWN				= "PLE_FILTER_OWN1"
		self.PLE_FILTER_FOREIGN			= "PLE_FILTER_FOREIGN1"
		self.PLE_FILTER_CANMOVE			= "PLE_FILTER_CANMOVE1"
		self.PLE_FILTER_CANTMOVE		= "PLE_FILTER_CANTMOVE1"

		self.PLE_PROMO_BUTTONS_UNITINFO = "PLE_PROMO_BUTTONS_UNITINFO"

		self.PLE_GRP_UNITTYPE			= "PLE_GRP_UNITTYPE1"
		self.PLE_GRP_GROUPS				= "PLE_GRP_GROUPS1"
		self.PLE_GRP_PROMO				= "PLE_GRP_PROMO1"
		self.PLE_GRP_UPGRADE			= "PLE_GRP_UPGRADE1"
		self.PLE_GROUP_MODES = ( self.PLE_GRP_UNITTYPE,
								 self.PLE_GRP_GROUPS,
								 self.PLE_GRP_PROMO,
								 self.PLE_GRP_UPGRADE )

		self.UNIT_INFO_PANE				= "PLE_UNIT_INFO_PANE_ID"
		self.UNIT_INFO_TEXT				= "PLE_UNIT_INFO_TEXT_ID"
		self.UNIT_INFO_TEXT_SHADOW		= "PLE_UNIT_INFO_TEXT_SHADOW_ID"

		# filter constants
		self.nPLEFilterModeCanMove		= 0x0001
		self.nPLEFilterModeCantMove		= 0x0002
		self.nPLEFilterModeWound		= 0x0004
		self.nPLEFilterModeNotWound		= 0x0008
		self.nPLEFilterModeAir 			= 0x0010
		self.nPLEFilterModeSea 			= 0x0020
		self.nPLEFilterModeLand			= 0x0040
		self.nPLEFilterModeDom			= 0x0080
		self.nPLEFilterModeMil			= 0x0100
		self.nPLEFilterModeOwn			= 0x0200
		self.nPLEFilterModeForeign		= 0x0400

		self.nPLEFilterGroupHealth	    = self.nPLEFilterModeWound | self.nPLEFilterModeNotWound
		self.nPLEFilterGroupDomain	   	= self.nPLEFilterModeAir | self.nPLEFilterModeSea | self.nPLEFilterModeLand
		self.nPLEFilterGroupType        = self.nPLEFilterModeDom | self.nPLEFilterModeMil
		self.nPLEFilterGroupOwner       = self.nPLEFilterModeOwn | self.nPLEFilterModeForeign
		self.nPLEFilterGroupMove  	    = self.nPLEFilterModeCanMove | self.nPLEFilterModeCantMove

		self.nPLEAllFilters = self.nPLEFilterGroupHealth | self.nPLEFilterGroupDomain | self.nPLEFilterGroupType | self.nPLEFilterGroupOwner | self.nPLEFilterGroupMove

		# set all filters to active -> all units
		self.nPLEFilter 				= self.nPLEAllFilters

		self.MainInterfaceInputMap = {

		}

		self.iColOffset 			= 0
		self.iRowOffset 			= 0
		self.iVisibleUnits 			= 0
		self.pActPlot 				= 0
		self.pOldPlot 				= self.pActPlot
		self.sPLEMode 				= self.PLE_MODE_MULTILINE
		self.iMaxPlotListIcons 		= 0
		self.nPLEGrpMode 			= self.PLE_GRP_UNITTYPE
		self.nPLELastGrpMode  	    = self.nPLEGrpMode
		self.pActPlotListUnit		= 0
		self.iActPlotListGroup		= 0
		self.pLastPlotListUnit		= 0
		self.iLastPlotListGroup		= 0

		self.IDX_PLAYER				= 0
		self.IDX_DOMAIN 			= 1
		self.IDX_GROUPID			= 2
		self.IDX_COMBAT				= 3
		self.IDX_UNITTYPE			= 4
		self.IDX_LEVEL				= 5
		self.IDX_XP					= 6
		self.IDX_TRANSPORTID		= 7
		self.IDX_CARGOID			= 8
		self.IDX_UNITID				= 9
		self.IDX_UNIT				= 10

		self.lPLEUnitList 			= []
		self.lPLEUnitListTempOK		= []
		self.lPLEUnitListTempNOK	= []
		self.dPLEUnitInfo 			= {}

		self.iLoopCnt 				= 0
		self.bPLEHide 				= False
		self.bUpdatePLEUnitList 	= True

		self.dUnitPromoList			= {}
		self.dUnitUpgradeList		= {}

		self.bInit					= False

		self.ASMA 					= AStarMoveArea()

		self.tLastMousePos			= (0,0)
		self.bInfoPaneActive		= False
		self.bUnitPromoButtonsActive = False
		self.iMousePosTol			= 30
		self.iInfoPaneCnt			= 0
		self.iLastInfoPaneCnt		= -1

		self.xResolution = 0
		self.yResolution = 0
# BUG - PLE - end

# BUG - field of view slider - start
		self.szSliderTextId = "FieldOfViewSliderText"
		self.sFieldOfView_Text = ""
		self.szSliderId = "FieldOfViewSlider"
		self.iField_View_Prev = -1
# BUG - field of view slider - end

#adding missing stuff
		self.CFG_INFOPANE_DX = 0
		self.CFG_INFOPANE_PIX_PER_LINE_1 = 0
		self.CFG_INFOPANE_PIX_PER_LINE_2 = 0
		self.CFG_INFOPANE_Y = 0
		self.CFG_INFOPANE_BUTTON_SIZE = 0
		self.CFG_INFOPANE_BUTTON_PER_LINE = 0
		self.CFG_INFOPANE_Y2 = 0
		self.pBarResearchBar_n = ProgressBarUtil.ProgressBar("ResearchBar-Canvas", 0, 2, 487, 5, 0, 0, True)
		self.pBarResearchBar_w = ProgressBarUtil.ProgressBar("ResearchBar-Canvas", 0, 2, 487, 5, 0, 0, True)
		self.pBarPopulationBar = ProgressBarUtil.ProgressBar("ResearchBar-Canvas", 0, 2, 487, 5, 0, 0, True)
		self.pBarProductionBar = ProgressBarUtil.ProgressBar("ResearchBar-Canvas", 0, 2, 487, 5, 0, 0, True)
		self.pBarProductionBar_Whip = ProgressBarUtil.ProgressBar("ResearchBar-Canvas", 0, 2, 487, 5, 0, 0, True)
		self.iX_FoVSlider = 0
		self.iY_FoVSlider = 0

############## base functions to calculate/transform the number of objects dependent on the screen resolution ######################

	def getMaxCol(self):
		return ((self.xResolution - (iMultiListXL+iMultiListXR) - 68) / 34)

	def getMaxRow(self):
		return ((self.yResolution - 160) / PleOpt.getVerticalSpacing())

	def getRow(self, i):
		return i / self.getMaxCol()

	def getCol(self, i):
		return i % self.getMaxCol()

	def getX(self, nCol):
		return 315 + (nCol * PleOpt.getHoriztonalSpacing())

	def getY(self, nRow):
		return self.yResolution - 169 - (nRow * PleOpt.getVerticalSpacing())

	def getI(self, nRow, nCol):
		return ( nRow * self.getMaxCol() ) + ( nCol % self.getMaxCol() )

############## functions for visual objects (show and hide) ######################

################ functions for normal/shift/ctrl/alt unit selection within the plot list itself #################

	def displayUnitPlotList_Weapon( self, screen, pLoopUnit, szString, iCount, x, y ):
		# this if statement and everything inside, handles the display of the Weapon buttons in the upper left corner of each unit icon.
		xSize = 16
		ySize = 16
		xOffset = 7
		yOffset = -3

		if pLoopUnit.getEquipment(gc.getInfoTypeForString('EQUIPMENTCATEGORY_WEAPON'))==-1:
			szStringIcon = szString+"Weapon"
			screen.hide( szStringIcon )
			return

		szFileNameState = ArtFileMgr.getInterfaceArtInfo("OVERLAY_WEAPON").getPath()

		# display the colored spot icon
		szStringIcon = szString+"Weapon"
		screen.addDDSGFC( szStringIcon, szFileNameState, x-3+xOffset, y-7+yOffset, xSize, ySize, WidgetTypes.WIDGET_GENERAL, iCount, -1 )
		screen.show( szStringIcon )

		return 0

	def displayUnitPlotList_Armor( self, screen, pLoopUnit, szString, iCount, x, y ):
		# this if statement and everything inside, handles the display of the Weapon buttons in the upper left corner of each unit icon.
		xSize = 16
		ySize = 16
		xOffset = 14
		yOffset = -3

		if pLoopUnit.getEquipment(gc.getInfoTypeForString('EQUIPMENTCATEGORY_ARMOR'))==-1:
			szStringIcon = szString+"Armor"
			screen.hide( szStringIcon )
			return

		szFileNameState = ArtFileMgr.getInterfaceArtInfo("OVERLAY_ARMOR").getPath()

		# display the colored spot icon
		szStringIcon = szString+"Armor"
		screen.addDDSGFC( szStringIcon, szFileNameState, x-3+xOffset, y-7+yOffset, xSize, ySize, WidgetTypes.WIDGET_GENERAL, iCount, -1 )
		screen.show( szStringIcon )

		return 0


	# replaces the buggy civ 4 version.
	def getInterfacePlotUnit(self, i):
		return self.lPLEUnitList[i][self.IDX_UNIT]


#########################################################################################
#########################################################################################
#########################################################################################
#########################################################################################

	def numPlotListButtons(self):
		return self.m_iNumPlotListButtons

	def initState (self, screen=None):
		"""
		Initialize screen instance (self.foo) and global variables.

		This function is called before drawing the screen (from interfaceScreen() below)
		and anytime the Python modules are reloaded (from CvEventInterface).

		THIS FUNCTION MUST NOT ALTER THE SCREEN -- screen.foo()
		"""
		if screen is None:
			screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		self.xResolution = screen.getXResolution()
		self.yResolution = screen.getYResolution()

# BUG - Raw Yields - begin
		global g_bYieldView
		global g_iYieldType
		g_bYieldView, g_iYieldType = RawYields.getViewAndType(CityScreenOpt.getRawYieldsDefaultView())
# BUG - Raw Yields - end

# BUG - PLE - begin
		self.pOldPlot = 0
		self.m_iNumPlotListButtons = (self.xResolution - (iMultiListXL + iMultiListXR) - 68) / 34

		self.CFG_INFOPANE_PIX_PER_LINE_1 			= 24
		self.CFG_INFOPANE_PIX_PER_LINE_2 			= 19
		self.CFG_INFOPANE_DX 					    = 290

		self.CFG_INFOPANE_Y		 			= self.yResolution - PleOpt.getInfoPaneY()
		self.CFG_INFOPANE_BUTTON_SIZE		= self.CFG_INFOPANE_PIX_PER_LINE_1 - 2
		self.CFG_INFOPANE_BUTTON_PER_LINE	= self.CFG_INFOPANE_DX / self.CFG_INFOPANE_BUTTON_SIZE
		self.CFG_INFOPANE_Y2				= self.CFG_INFOPANE_Y + 105

		self.sPLEMode = self.PLE_VIEW_MODES[PleOpt.getDefaultViewMode()]
		self.nPLEGrpMode = self.PLE_GROUP_MODES[PleOpt.getDefaultGroupMode()]
# BUG - PLE - end

		# Global variables being set here
		global g_NumEmphasizeInfos
		global g_NumCityTabTypes
		global g_NumHurryInfos
		global g_NumUnitClassInfos
		global g_NumBuildingClassInfos
		global g_NumProjectInfos
		global g_NumProcessInfos
		global g_NumActionInfos

		global MAX_SELECTED_TEXT
		global MAX_DISPLAYABLE_BUILDINGS
		global MAX_DISPLAYABLE_TRADE_ROUTES
		global MAX_BONUS_ROWS
		global MAX_CITIZEN_BUTTONS

		global HUD_City_Bottom_Right_Width
		global HUD_City_Bottom_Right_Height
		global HUD_City_Bottom_Left_Width
		global HUD_City_Bottom_Left_Height
		global HUD_City_Bottom_Center_Height
		global HUD_City_Left_Panel_Width
		global HUD_City_Buildings_Label_Height
		global HUD_City_Buildings_Label_Width
		global HUD_City_TradeRoute_Label_Height
		global HUD_City_TradeRoute_Label_Width
		global HUD_City_Status_Panel_Height
		global HUD_City_Status_Panel_Width
		global HUD_City_Right_Panel_Width
		global HUD_City_Growth_Panel_Width_Exclusion
		global HUD_City_Growth_Panel_Height
		global HUD_City_Name_Panel_Width_Exclusion
		global HUD_City_Name_Panel_Height
		global HUD_City_Top_Center_Background_Panel_Height
		global HUD_City_Top_Panel_Height
		global HUD_City_Extra_1_Location_X
		global HUD_City_Extra_1_Location_Y
		global HUD_City_Extra_1_Height
		global HUD_City_Extra_1_Width
		global HUD_Main_Bottom_Right_Width
		global HUD_Main_Bottom_Right_Height
		global HUD_Main_Bottom_Left_Width
		global HUD_Main_Bottom_Left_Height
		global HUD_Main_Bottom_Center_Height
		global HUD_Main_Top_Left_Width
		global HUD_Main_Top_Left_Height
		global HUD_Main_Top_Right_Width
		global HUD_Main_Top_Right_Height
		global HUD_Main_Top_Center_Height
		global HUD_Main_Extra_1_Location_X
		global HUD_Main_Extra_1_Location_Y
		global HUD_Main_Extra_1_Height
		global HUD_Main_Extra_1_Width

		if ( CyGame().isPitbossHost() ):
			return

		# This is the main interface screen, create it as such
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		screen.setForcedRedraw(True)

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		self.m_iNumPlotListButtons = (xResolution - (iMultiListXL+iMultiListXR) - 68) / 34

		screen.setDimensions(0, 0, xResolution, yResolution)

		# Set up our global variables...
		g_NumEmphasizeInfos = gc.getNumEmphasizeInfos()
		g_NumCityTabTypes = CityTabTypes.NUM_CITYTAB_TYPES
		g_NumHurryInfos = gc.getNumHurryInfos()
		g_NumUnitClassInfos = gc.getNumUnitClassInfos()
		g_NumBuildingClassInfos = gc.getNumBuildingClassInfos()
		g_NumProjectInfos = gc.getNumProjectInfos()
		g_NumProcessInfos = gc.getNumProcessInfos()
		g_NumActionInfos = gc.getNumActionInfos()

# BUG - field of view slider - start
		iBtnY = 27
		self.iX_FoVSlider = self.xResolution - 120
		self.iY_FoVSlider = iBtnY + 30
		self.sFieldOfView_Text = localText.getText("TXT_KEY_BUG_OPT_MAININTERFACE__FIELDOFVIEW_TEXT", ())
		if MainOpt.isRememberFieldOfView():
			self.iField_View = int(MainOpt.getFieldOfView())
		else:
			self.iField_View = DEFAULT_FIELD_OF_VIEW
# BUG - field of view slider - end

# BUG - PLE - begin
		self.iMaxPlotListIcons = self.getMaxCol() * 20
# BUG - PLE - end

# BUG - Progress Bar - Tick Marks - start
		xCoord = 268 + (self.xResolution - 1024) / 2
		self.pBarResearchBar_n = ProgressBarUtil.ProgressBar("ResearchBar-Canvas", xCoord, 2, 487, iStackBarHeight, gc.getInfoTypeForString("COLOR_RESEARCH_RATE"), ProgressBarUtil.TICK_MARKS, True)
		self.pBarResearchBar_n.addBarItem("ResearchBar")
		self.pBarResearchBar_n.addBarItem("ResearchText")
# BUG - Progress Bar - Tick Marks - end

# BUG - Progress Bar - Tick Marks - start
		xCoord = 268 + (self.xResolution - 1440) / 2
		xCoord += 6 + 84
		self.pBarResearchBar_w = ProgressBarUtil.ProgressBar("ResearchBar-w-Canvas", xCoord, 2, 487, iStackBarHeight, gc.getInfoTypeForString("COLOR_RESEARCH_RATE"), ProgressBarUtil.TICK_MARKS, True)
		self.pBarResearchBar_w.addBarItem("ResearchBar-w")
		self.pBarResearchBar_w.addBarItem("ResearchText")
# BUG - Progress Bar - Tick Marks - end

# BUG - Progress Bar - Tick Marks - start
		self.pBarPopulationBar = ProgressBarUtil.ProgressBar("PopulationBar-Canvas", iCityCenterRow1X, iCityCenterRow1Y-4, self.xResolution - (iCityCenterRow1X*2), iStackBarHeight, gc.getYieldInfo(YieldTypes.YIELD_FOOD).getColorType(), ProgressBarUtil.SOLID_MARKS, True)
		self.pBarPopulationBar.addBarItem("PopulationBar")
		self.pBarPopulationBar.addBarItem("PopulationText")
		self.pBarProductionBar = ProgressBarUtil.ProgressBar("ProductionBar-Canvas", iCityCenterRow2X, iCityCenterRow2Y-4, self.xResolution - (iCityCenterRow2X*2), iStackBarHeight, gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getColorType(), ProgressBarUtil.TICK_MARKS, True)
		self.pBarProductionBar.addBarItem("ProductionBar")
		self.pBarProductionBar.addBarItem("ProductionText")
		self.pBarProductionBar_Whip = ProgressBarUtil.ProgressBar("ProductionBar-Whip-Canvas", iCityCenterRow2X, iCityCenterRow2Y-4, self.xResolution - (iCityCenterRow2X*2), iStackBarHeight, gc.getInfoTypeForString("COLOR_YELLOW"), ProgressBarUtil.CENTER_MARKS, False)
		self.pBarProductionBar_Whip.addBarItem("ProductionBar")
		self.pBarProductionBar_Whip.addBarItem("ProductionText")
# BUG - Progress Bar - Tick Marks - end

	def interfaceScreen (self):
		"""
		Draw all of the screen elements.

		This function is called once after starting or loading a game.

		THIS FUNCTION MUST NOT CREATE ANY INSTANCE OR GLOBAL VARIABLES.
		It may alter existing ones created in __init__() or initState(), however.
		"""
		if ( CyGame().isPitbossHost() ):
			return

		# This is the main interface screen, create it as such
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		self.initState(screen)
		screen.setForcedRedraw(True)
		screen.setDimensions(0, 0, self.xResolution, self.yResolution)

		# to avoid changing all the code below
		xResolution = self.xResolution
		yResolution = self.yResolution

##################################################################################################
# Winamp Start
##################################################################################################
		if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_WINAMP_GUI):
			self.WinAMPCounter = 0
			self.WinAMPMuteCounter = 0
			iscreenmiddle = xResolution / 2
			screen.setImageButton("WinAMPButton1", "Art/Interface/Buttons/WinAMP/WinAMP.dds", xResolution - 30, 64, 24, 24, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			screen.setImageButton("WinAMPPrevButton1", "Art/Interface/Buttons/WinAMP/Prev.dds", iscreenmiddle - 130, 55, 24, 24, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_WINAMP_PREV).getActionInfoIndex(), -1 )
			screen.setImageButton("WinAMPPlayButton1", "Art/Interface/Buttons/WinAMP/Play.dds", iscreenmiddle - 105, 55, 24, 24, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_WINAMP_PLAY).getActionInfoIndex(), -1 )
			screen.setImageButton("WinAMPPauseButton1", "Art/Interface/Buttons/WinAMP/Pause.dds", iscreenmiddle - 80, 55, 24, 24, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_WINAMP_PAUSE).getActionInfoIndex(), -1 )
			screen.setImageButton("WinAMPStopButton1", "Art/Interface/Buttons/WinAMP/Stop.dds", iscreenmiddle - 55, 55, 24, 24, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_WINAMP_STOP).getActionInfoIndex(), -1 )
			screen.setImageButton("WinAMPNextButton1", "Art/Interface/Buttons/WinAMP/Next.dds", iscreenmiddle - 30, 55, 24, 24, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_WINAMP_NEXT).getActionInfoIndex(), -1 )
			screen.addSlider("WinAMPVolumeSlider", iscreenmiddle - 4, 55, 105, 25, WinAMP.GetVolume(), 0, 255, WidgetTypes.WIDGET_GENERAL, -1, -1, False) # 130
			screen.setImageButton("WinAMPMuteButton1", "Art/Interface/Buttons/WinAMP/Mute1.dds", iscreenmiddle + 110, 60, 19, 19, WidgetTypes.WIDGET_GENERAL, gc.getControlInfo(ControlTypes.CONTROL_WINAMP_NEXT).getActionInfoIndex(), -1 )
#			screen.hide("WinAMPPlayButton1")
#			screen.hide("WinAMPPauseButton1")
#			screen.hide("WinAMPStopButton1")
#			screen.hide("WinAMPPrevButton1")
#			screen.hide("WinAMPNextButton1")
#			screen.hide("WinAMPVolumeSlider")
#			screen.hide("WinAMPMuteButton1")
##################################################################################################
# Winamp End
##################################################################################################

# Customized City Background start
		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
		iCiv=pPlayer.getCivilizationType()
		szArt=str(gc.getCivilizationInfo(iCiv).getFlavorCityScreenImage())
		szTop=szArt+"_top.dds"
		szLeft=szArt+"_left.dds"
		szRight=szArt+"_right.dds"
		szBottom=szArt+"_bottom.dds"

		screen.addPanel( "InterfaceCityBackgroundTop", u"", u"", True, False, 0, 0, xResolution , yResolution/6 , PanelStyles.PANEL_STYLE_STANDARD)
		screen.addDrawControl( "InterfaceCityBackgroundTop", szTop, 0, 0, xResolution , yResolution/6 , WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceCityBackgroundTop" )

		screen.addPanel( "InterfaceCityBackgroundLeft", u"", u"", True, False, 0, yResolution/6, xResolution / 4, (yResolution*4)/6 , PanelStyles.PANEL_STYLE_STANDARD)
		screen.addDrawControl( "InterfaceCityBackgroundLeft", szLeft,  0, yResolution/6, xResolution / 4, (yResolution*4)/6 , WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceCityBackgroundLeft" )

		screen.addPanel( "InterfaceCityBackgroundRight", u"", u"", True, False, (xResolution*3)/4, yResolution/6, xResolution/4 , (yResolution*4)/6 , PanelStyles.PANEL_STYLE_STANDARD)
		screen.addDrawControl( "InterfaceCityBackgroundRight", szRight, (xResolution*3)/4, yResolution/6, xResolution/4 , (yResolution*4)/6  , WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceCityBackgroundRight" )

		screen.addPanel( "InterfaceCityBackgroundBottom", u"", u"", True, False, 0, (yResolution*5)/6, xResolution, yResolution/6 , PanelStyles.PANEL_STYLE_STANDARD)
		screen.addDrawControl( "InterfaceCityBackgroundBottom", szBottom, 0, (yResolution*5)/6, xResolution + 4, yResolution/6 , WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceCityBackgroundBottom" )
# Flavor City Background end

		screen.addDDSGFC( "InterfaceCityBackgroundTopCenter", 'Art/Interface/Screens/Default/Main_Top_Center.tga', xResolution - HUD_Main_Top_Left_Width, 0, xResolution - HUD_Main_Top_Left_Width - HUD_Main_Top_Right_Width, HUD_Main_Top_Center_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceCityBackgroundTopCenter" )

		screen.addDDSGFC( "InterfaceCityBackgroundTopLeft", 'Art/Interface/Screens/Default/Main_Top_Left.tga', 0, 0, HUD_Main_Top_Left_Width-20, HUD_Main_Top_Left_Height+55, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceCityBackgroundTopLeft" )

		screen.addDDSGFC( "InterfaceCityBackgroundTopRight", 'Art/Interface/Screens/Default/Main_Top_Right.tga', xResolution - HUD_Main_Top_Left_Width+30, 0, HUD_Main_Top_Right_Width-30, HUD_Main_Top_Right_Height+57, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceCityBackgroundTopRight" )

# Customized City Background End

		# Help Text Area
		screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, 7, yResolution - 172, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, HELP_TEXT_MINIMUM_WIDTH )

		# Center Left
		screen.addPanel( "InterfaceCenterLeftBackgroundWidget", u"", u"", True, False, 0, 0, 258, yResolution-149, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "InterfaceCenterLeftBackgroundWidget", "Panel_City_Left_Style" )
		screen.hide( "InterfaceCenterLeftBackgroundWidget" )

		# Top Left
		screen.addPanel( "InterfaceTopLeftBackgroundWidget", u"", u"", True, False, 258, 0, xResolution - 516, yResolution-149, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "InterfaceTopLeftBackgroundWidget", "Panel_City_Top_Style" )
		screen.hide( "InterfaceTopLeftBackgroundWidget" )

		# Center Right
		screen.addPanel( "InterfaceCenterRightBackgroundWidget", u"", u"", True, False, xResolution - 258, 0, 258, yResolution-149, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "InterfaceCenterRightBackgroundWidget", "Panel_City_Right_Style" )
		screen.hide( "InterfaceCenterRightBackgroundWidget" )

		screen.addPanel( "CityScreenAdjustPanel", u"", u"", True, False, 10, 44, 238, 105, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "CityScreenAdjustPanel", "Panel_City_Info_Style" )
		screen.hide( "CityScreenAdjustPanel" )

		screen.addPanel( "TopCityPanelLeft", u"", u"", True, False, 260, 70, xResolution/2-260, 60, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "TopCityPanelLeft", "Panel_City_TanTL_Style" )
		screen.hide( "TopCityPanelLeft" )

		screen.addPanel( "TopCityPanelRight", u"", u"", True, False, xResolution/2, 70, xResolution/2-260, 60, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "TopCityPanelRight", "Panel_City_TanTR_Style" )
		screen.hide( "TopCityPanelRight" )

		# Top Bar

		# SF CHANGE
		screen.addPanel( "CityScreenTopWidget", u"", u"", True, False, 0, -2, xResolution, 41, PanelStyles.PANEL_STYLE_STANDARD )

		screen.setStyle( "CityScreenTopWidget", "Panel_TopBar_Style" )
		screen.hide( "CityScreenTopWidget" )

		# Top Center Title
		screen.addPanel( "CityNameBackground", u"", u"", True, False, 260, 31, xResolution - (260*2), 38, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "CityNameBackground", "Panel_City_Title_Style" )
		screen.hide( "CityNameBackground" )

		# Left Background Widget
# modified for FFH
		screen.addDDSGFC( "InterfaceLeftBackgroundWidget", 'Art/Interface/Screens/Default/Main_Bottom_Left.tga', 0, yResolution - HUD_Main_Bottom_Left_Height, HUD_Main_Bottom_Left_Width, HUD_Main_Bottom_Left_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceLeftBackgroundWidget" )

		screen.addDDSGFC( "InterfaceCenterBackgroundWidget", 'Art/Interface/Screens/Default/Main_Bottom_Center.tga', HUD_Main_Bottom_Left_Width, yResolution - HUD_Main_Bottom_Center_Height, xResolution - HUD_Main_Bottom_Left_Width - HUD_Main_Bottom_Right_Width, HUD_Main_Bottom_Center_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setHitTest( "InterfaceCenterBackgroundWidget", HitTestTypes.HITTEST_NOHIT )
		screen.hide( "InterfaceCenterBackgroundWidget" )

		screen.addDDSGFC( "InterfaceRightBackgroundWidget", 'Art/Interface/Screens/Default/Main_Bottom_Right.tga', xResolution - HUD_Main_Bottom_Right_Width, yResolution - HUD_Main_Bottom_Right_Height, HUD_Main_Bottom_Right_Width, HUD_Main_Bottom_Right_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceRightBackgroundWidget" )

		screen.addDDSGFC( "InterfaceTopCenter", 'Art/Interface/Screens/Default/Main_Top_Center.tga', xResolution - HUD_Main_Top_Left_Width, 0, xResolution - HUD_Main_Top_Left_Width - HUD_Main_Top_Right_Width, HUD_Main_Top_Center_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceTopCenter" )

		screen.addDDSGFC( "InterfaceTopLeft", 'Art/Interface/Screens/Default/Main_Top_Left.tga', 0, 0, HUD_Main_Top_Left_Width, HUD_Main_Top_Left_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceTopLeft" )

		screen.addDDSGFC( "InterfaceTopRight", 'Art/Interface/Screens/Default/Main_Top_Right.tga', xResolution - HUD_Main_Top_Left_Width, 0, HUD_Main_Top_Right_Width, HUD_Main_Top_Right_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "InterfaceTopRight" )


		# Center Background Widget
#		screen.addPanel( "InterfaceCenterBackgroundWidget", u"", u"", True, False, 296, yResolution - 133, xResolution - (296*2), 133, PanelStyles.PANEL_STYLE_STANDARD)
#		screen.setStyle( "InterfaceCenterBackgroundWidget", "Panel_Game_HudBC_Style" )
#		screen.hide( "InterfaceCenterBackgroundWidget" )

		# Left Background Widget
#		screen.addPanel( "InterfaceLeftBackgroundWidget", u"", u"", True, False, 0, yResolution - 168, 304, 168, PanelStyles.PANEL_STYLE_STANDARD)
#		screen.setStyle( "InterfaceLeftBackgroundWidget", "Panel_Game_HudBL_Style" )
#		screen.hide( "InterfaceLeftBackgroundWidget" )

		# Right Background Widget
#		screen.addPanel( "InterfaceRightBackgroundWidget", u"", u"", True, False, xResolution - 304, yResolution - 168, 304, 168, PanelStyles.PANEL_STYLE_STANDARD)
#		screen.setStyle( "InterfaceRightBackgroundWidget", "Panel_Game_HudBR_Style" )
#		screen.hide( "InterfaceRightBackgroundWidget" )

		# Top Center Background

		# SF CHANGE
#		screen.addPanel( "InterfaceTopCenter", u"", u"", True, False, 257, -2, xResolution-(257*2), 48, PanelStyles.PANEL_STYLE_STANDARD)
#		screen.setStyle( "InterfaceTopCenter", "Panel_Game_HudTC_Style" )
#		screen.hide( "InterfaceTopCenter" )

		# Top Left Background
#		screen.addPanel( "InterfaceTopLeft", u"", u"", True, False, 0, -2, 267, 60, PanelStyles.PANEL_STYLE_STANDARD)
#		screen.setStyle( "InterfaceTopLeft", "Panel_Game_HudTL_Style" )
#		screen.hide( "InterfaceTopLeft" )

		# Top Right Background
#		screen.addPanel( "InterfaceTopRight", u"", u"", True, False, xResolution - 267, -2, 267, 60, PanelStyles.PANEL_STYLE_STANDARD)
#		screen.setStyle( "InterfaceTopRight", "Panel_Game_HudTR_Style" )
#		screen.hide( "InterfaceTopRight" )

		screen.addDDSGFC( "MainExtra1", 'Art/Interface/Screens/Default/Main_Extra_1.tga', HUD_Main_Extra_1_Location_X, HUD_Main_Extra_1_Location_Y, HUD_Main_Extra_1_Width, HUD_Main_Extra_1_Height, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setHitTest( "MainExtra1", HitTestTypes.HITTEST_NOHIT )
		screen.hide( "MainExtra1" )

#		screen.setImageButton("RawManaButton1", "Art/Interface/Screens/RawManaButton.dds", 80, 88, 20, 20, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#		screen.hide( "RawManaButton1" )
#		screen.addPanel( "ManaToggleHelpTextPanel", u"", u"", True, True, 100, 88, 170, 30, PanelStyles.PANEL_STYLE_HUD_HELP )
#		screen.hide( "ManaToggleHelpTextPanel" )
#		szText = "<font=2>" + localText.getText("[COLOR_HIGHLIGHT_TEXT]Toggle Manabar Display[COLOR_REVERT]", ()) + "</font=2>"
#		screen.addMultilineText( "ManaToggleHelpText", szText, 102, 93, 167, 27, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
#		screen.hide( "ManaToggleHelpText" )
#SCIONS
		screen.addPanel( "SpawnOddsHelpTextPanel", u"", u"", True, True, 100, 88, 170, 30, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "SpawnOddsHelpTextPanel" )
		szText = "<font=2>" + localText.getText("[COLOR_HIGHLIGHT_TEXT]Awakened Spawn Chance[COLOR_REVERT]", ()) + "</font=2>"
		screen.addMultilineText( "SpawnOddsHelpText", szText, 102, 93, 167, 27, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.hide( "SpawnOddsHelpText" )

		screen.addPanel( "PurityHelpTextPanel", u"", u"", True, True, 100, 88, 170, 30, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "PurityHelpTextPanel" )
		szText = "<font=2>" + localText.getText("[COLOR_HIGHLIGHT_TEXT]Purity Counter[COLOR_REVERT]", ()) + "</font=2>"
		screen.addMultilineText( "PurityHelpText", szText, 102, 93, 167, 27, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.hide( "PurityHelpText" )

		screen.addPanel( "SidarCounterHelpTextPanel", u"", u"", True, True, 100, 88, 170, 30, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "SidarCounterHelpTextPanel" )
		szText = "<font=2>" + localText.getText("[COLOR_HIGHLIGHT_TEXT]If you have enough Units with High Experience, you can cast the Waning Global Enchantment![COLOR_REVERT]", ()) + "</font=2>"
		screen.addMultilineText( "SidarCounterHelpText", szText, 102, 93, 167, 27, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.hide( "SidarCounterHelpText" )

		screen.addPanel( "SpiritVaultHelpTextPanel", u"", u"", True, True, 100, 88, 170, 30, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "SpiritVaultHelpTextPanel" )
		szText = "<font=2>" + localText.getText("[COLOR_HIGHLIGHT_TEXT]Spirit Vault[COLOR_REVERT]", ()) + "</font=2>"
		screen.addMultilineText( "SpiritVaultHelpText", szText, 102, 93, 167, 27, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)
		screen.hide( "SpiritVaultHelpText" )

#/SCIONS

		iBtnWidth	= 28
		iBtnAdvance = 25
		iBtnY = 7
		iBtnX = (xResolution - 1024) / 2
		if iBtnX>50:
			iBtnX=iBtnX-50

		# Turn log Button

		iBtnX = xResolution - 277
		iBtnY = 3

		screen.setImageButton( "TurnLogButton", "", iBtnX, iBtnY - 2, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_TURN_LOG).getActionInfoIndex(), -1 )
		screen.setStyle( "TurnLogButton", "Button_HUDLog_Style" )
		screen.hide( "TurnLogButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "GuildButton", "", iBtnX, iBtnY - 2, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_GUILD_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "GuildButton", "Button_HUDAdvisorGuild_Style" )
		screen.hide( "GuildButton" )
#		iBtnX += iBtnAdvance
#		screen.setImageButton( "TrophyButton", "", iBtnX, iBtnY - 2, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_TROPHY).getActionInfoIndex(), -1 )
#		screen.setStyle( "TrophyButton", "Button_HUDTrophy_Style" )
#		screen.hide( "TrophyButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "MagicButton", "", iBtnX, iBtnY - 2, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_MAGIC_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "MagicButton", "Button_HUDMagic_Style" )
		screen.hide( "MagicButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "ArcaneButton", "", iBtnX, iBtnY - 2, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_ARCANE_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "ArcaneButton", "Button_HUDArcane_Style" )
		screen.hide( "ArcaneButton" )

		iBtnX = xResolution - 277
		iBtnY = 27

		# Advisor Buttons...
		screen.setImageButton( "DomesticAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_DOMESTIC_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "DomesticAdvisorButton", "Button_HUDAdvisorDomestic_Style" )
		screen.hide( "DomesticAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "FinanceAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_FINANCIAL_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "FinanceAdvisorButton", "Button_HUDAdvisorFinance_Style" )
		screen.hide( "FinanceAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "CivicsAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_CIVICS_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "CivicsAdvisorButton", "Button_HUDAdvisorCivics_Style" )
		screen.hide( "CivicsAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "ForeignAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_FOREIGN_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "ForeignAdvisorButton", "Button_HUDAdvisorForeign_Style" )
		screen.hide( "ForeignAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "MilitaryAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_MILITARY_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "MilitaryAdvisorButton", "Button_HUDAdvisorMilitary_Style" )
		screen.hide( "MilitaryAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "TechAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_TECH_CHOOSER).getActionInfoIndex(), -1 )
		screen.setStyle( "TechAdvisorButton", "Button_HUDAdvisorTechnology_Style" )
		screen.hide( "TechAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "ReligiousAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_RELIGION_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "ReligiousAdvisorButton", "Button_HUDAdvisorReligious_Style" )
		screen.hide( "ReligiousAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "CorporationAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_CORPORATION_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "CorporationAdvisorButton", "Button_HUDAdvisorCorporation_Style" )
		screen.hide( "CorporationAdvisorButton" )
#Houses of Erebus added
#		iBtnX += iBtnAdvance
#		screen.setImageButton( "GuildButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_GUILD_SCREEN).getActionInfoIndex(), -1 )
#		screen.setStyle( "GuildButton", "Button_HUDAdvisorGuild_Style" )
#		screen.hide( "GuildButton" )
#Houses of Erebus added

		iBtnX += iBtnAdvance
		screen.setImageButton( "VictoryAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_VICTORY_SCREEN).getActionInfoIndex(), -1 )
		screen.setStyle( "VictoryAdvisorButton", "Button_HUDAdvisorVictory_Style" )
		screen.hide( "VictoryAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "InfoAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_INFO).getActionInfoIndex(), -1 )
		screen.setStyle( "InfoAdvisorButton", "Button_HUDAdvisorRecord_Style" )
		screen.hide( "InfoAdvisorButton" )

		iBtnX += iBtnAdvance
		screen.setImageButton( "TrophyButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_TROPHY).getActionInfoIndex(), -1 )
		screen.setStyle( "TrophyButton", "Button_HUDTrophy_Style" )
		screen.hide( "TrophyButton" )
		
# BUG - 3.17 No Espionage - start
		if not BugUtil.isNoEspionage():
			iBtnX += iBtnAdvance
			screen.setImageButton( "EspionageAdvisorButton", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_ESPIONAGE_SCREEN).getActionInfoIndex(), -1 )
			screen.setStyle( "EspionageAdvisorButton", "Button_HUDAdvisorEspionage_Style" )
			screen.hide( "EspionageAdvisorButton" )
# BUG - 3.17 No Espionage - end

# BUG - field of view slider - start
		self.setFieldofView_Text(screen)
		iW = 100
		iH = 15
		screen.addSlider(self.szSliderId, self.iX_FoVSlider + 5, self.iY_FoVSlider, iW, iH, self.iField_View - 1, 0, 100 - 1, WidgetTypes.WIDGET_GENERAL, -1, -1, False);
		screen.hide(self.szSliderTextId)
		screen.hide(self.szSliderId)
# BUG - field of view slider - end

		# City Tabs
		iBtnX = xResolution - 324
		iBtnY = yResolution - 94
		iBtnWidth = 24
		iBtnAdvance = 24

		screen.setButtonGFC( "CityTab0", "", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_CITY_TAB, 0, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
		screen.setStyle( "CityTab0", "Button_HUDJumpUnit_Style" )
		screen.hide( "CityTab0" )

		iBtnY += iBtnAdvance
		screen.setButtonGFC( "CityTab1", "", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_CITY_TAB, 1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
		screen.setStyle( "CityTab1", "Button_HUDJumpBuilding_Style" )
		screen.hide( "CityTab1" )

		iBtnY += iBtnAdvance
		screen.setButtonGFC( "CityTab2", "", "", iBtnX, iBtnY, iBtnWidth, iBtnWidth, WidgetTypes.WIDGET_CITY_TAB, 2, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
		screen.setStyle( "CityTab2", "Button_HUDJumpWonder_Style" )
		screen.hide( "CityTab2" )

		# Minimap initialization
		screen.setMainInterface(True)

#FfH: Modified by Kael 07/18/2008
		screen.addPanel( "MiniMapPanel", u"", u"", True, False, xResolution - 214, yResolution - 151, 208, 151, PanelStyles.PANEL_STYLE_STANDARD )
#		screen.addPanel( "MiniMapPanel", u"", u"", True, False, xResolution - 236, yResolution - 151, 240, 155, PanelStyles.PANEL_STYLE_STANDARD )
#FfH: End Modify

		screen.setStyle( "MiniMapPanel", "Panel_Game_HudMap_Style" )
		screen.hide( "MiniMapPanel" )

#FfH: Modified by Kael 07/18/2008
		screen.initMinimap( xResolution - 210, xResolution - 9, yResolution - 131, yResolution - 9, -0.1 )
#		screen.initMinimap( xResolution - 232, xResolution, yResolution - 131, yResolution, -0.1 )
#FfH: End Modify

		gc.getMap().updateMinimapColor()

		self.createMinimapButtons()

		# Help button (always visible)
		screen.setImageButton( "InterfaceHelpButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_CIVILOPEDIA_ICON").getPath(), xResolution - 28, 2, 24, 24, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_CIVILOPEDIA).getActionInfoIndex(), -1 )
		screen.hide( "InterfaceHelpButton" )

		screen.setImageButton( "MainMenuButton", ArtFileMgr.getInterfaceArtInfo("INTERFACE_GENERAL_MENU_ICON").getPath(), xResolution - 54, 2, 24, 24, WidgetTypes.WIDGET_MENU_ICON, -1, -1 )
		screen.hide( "MainMenuButton" )

		# Globeview buttons
		self.createGlobeviewButtons( )

		screen.addMultiListControlGFC( "BottomButtonContainer", u"", iMultiListXL, yResolution - 113, xResolution - (iMultiListXL+iMultiListXR), 100, 4, 48, 48, TableStyles.TABLE_STYLE_STANDARD )
		screen.hide( "BottomButtonContainer" )

		# *********************************************************************************
		# PLOT LIST BUTTONS
		# *********************************************************************************

# BUG - PLE - begin
		for j in range(gc.getMAX_PLOT_LIST_ROWS()):
			yRow = (j - gc.getMAX_PLOT_LIST_ROWS() + 1) * 34

#FfH: Modified by Kael 07/18/2008
			yPixel = yResolution - 169 + yRow - 3
			xPixel = 315 - 3
#			yPixel = yResolution - 174 + yRow - 3
#			xPixel = 249 - 3
#FfH: End Modify

			xWidth = self.numPlotListButtons() * 34 + 3
			yHeight = 32 + 3

			szStringPanel = "PlotListPanel" + str(j)
			screen.addPanel(szStringPanel, u"", u"", True, False, xPixel, yPixel, xWidth, yHeight, PanelStyles.PANEL_STYLE_EMPTY)

			for i in range(self.numPlotListButtons()):
				k = j*self.numPlotListButtons()+i

				xOffset = i * 34

				szString = "PlotListButton" + str(k)

# BUG - plot list - start
				szFileNamePromo = ArtFileMgr.getInterfaceArtInfo("OVERLAY_PROMOTION_FRAME").getPath()
				szStringPromoFrame  = szString + "PromoFrame"
				screen.addDDSGFCAt( szStringPromoFrame , szStringPanel, szFileNamePromo, xOffset +  2,  2, 32, 32, WidgetTypes.WIDGET_PLOT_LIST, k, -1, False )
				screen.hide( szStringPromoFrame  )
# BUG - plot list - end

				screen.addCheckBoxGFCAt(szStringPanel, szString, ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_GOVERNOR").getPath(), ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), xOffset + 3, 3, 32, 32, WidgetTypes.WIDGET_PLOT_LIST, k, -1, ButtonStyles.BUTTON_STYLE_LABEL, True )
				screen.hide( szString )

				szStringHealth = szString + "Health"
#FfH: Modified
				"""
				screen.addStackedBarGFCAt( szStringHealth, szStringPanel, xOffset + 3, 26, 32, 11, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, k, -1 )
				"""
				screen.addStackedBarGFCAt( szStringHealth, szStringPanel, xOffset + 3, 22, 32, 22, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, k, -1 )
#FfH: End Modify
				screen.hide( szStringHealth )
#FfH: Added
				screen.setHitTest( szStringHealth, HitTestTypes.HITTEST_NOHIT )
#FfH: End Add

				szStringIcon = szString + "Icon"
				szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_MOVE").getPath()
				screen.addDDSGFCAt( szStringIcon, szStringPanel, szFileName, xOffset, 0, 12, 12, WidgetTypes.WIDGET_PLOT_LIST, k, -1, False )
				screen.hide( szStringIcon )

		iHealthyColor = PleOpt.getHealthyColor()
		iWoundedColor = PleOpt.getWoundedColor()
		iMovementColor = PleOpt.getFullMovementColor()
		iHasMovedColor = PleOpt.getHasMovedColor()
		iNoMovementColor = PleOpt.getNoMovementColor()

		szFileNamePromo = ArtFileMgr.getInterfaceArtInfo("OVERLAY_PROMOTION_FRAME").getPath()
		szFileNameGovernor = ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_GOVERNOR").getPath()
		szFileNameHilite = ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath()
		for i in range( self.iMaxPlotListIcons ):
			# create button name
			szString = self.PLOT_LIST_BUTTON_NAME + str(i)

			x = self.getX( self.getCol( i ) )
			y = self.getY( self.getRow( i ) )

##--------		Adjust Unit Button Display Format	--------##
##--------		BUGFfH: Moved to below(*1) by Denev	--------##
			"""
			# place/init the promotion frame. Important to have it at first place within the for loop.
			szStringPromoFrame = szString + "PromoFrame"
			screen.addDDSGFC( szStringPromoFrame, szFileNamePromo, x, y, 32, 32, WidgetTypes.WIDGET_GENERAL, i, -1 )
			screen.hide( szStringPromoFrame )

			# place the plot list unit button
			screen.addCheckBoxGFC( szString, szFileNameGovernor, szFileNameHilite, x, y, 32, 32, WidgetTypes.WIDGET_GENERAL, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
			screen.hide( szString )
			"""
##--------		BUGFfH: End Move					--------##

			# place/init the health bar. Important to have it at last place within the for loop.
			szStringHealthBar = szString + "HealthBar"
##--------		BUGFfH: Modified by Denev	--------##
			"""
#			screen.addStackedBarGFC( szStringHealthBar, x+7, y-7, 25, 14, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			screen.addStackedBarGFC( szStringHealthBar, x+5, y-9, 29, 11, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, i, -1 )
			"""
			screen.addStackedBarGFC( szStringHealthBar, x-3, y+26, 37, 18, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, i, -1 )
##--------		BUGFfH: End Modify			--------##
			screen.setStackedBarColors( szStringHealthBar, InfoBarTypes.INFOBAR_STORED, iHealthyColor )
			screen.setStackedBarColors( szStringHealthBar, InfoBarTypes.INFOBAR_RATE, iWoundedColor )
			screen.hide( szStringHealthBar )

			# place/init the movement bar. Important to have it at last place within the for loop.
			szStringMoveBar = szString + "MoveBar"
##--------		BUGFfH: Modified by Denev	--------##
#			screen.addStackedBarGFC( szStringMoveBar, x+5, y-5, 29, 11, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, i, -1 )
			screen.addStackedBarGFC( szStringMoveBar, x+3, y-10, 31, 18, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, i, -1 )
##--------		BUGFfH: End Modify			--------##
			screen.setStackedBarColors( szStringMoveBar, InfoBarTypes.INFOBAR_STORED, iMovementColor )
			screen.setStackedBarColors( szStringMoveBar, InfoBarTypes.INFOBAR_RATE, iHasMovedColor )
			screen.setStackedBarColors( szStringMoveBar, InfoBarTypes.INFOBAR_EMPTY, iNoMovementColor )
			screen.hide( szStringMoveBar )

##--------		BUGFfH: Moved from above(*1) by Denev	--------##
			# place/init the promotion frame. Important to have it at first place within the for loop.
			szStringPromoFrame = szString + "PromoFrame"
			screen.addDDSGFC( szStringPromoFrame, szFileNamePromo, x, y, 32, 32, WidgetTypes.WIDGET_GENERAL, i, -1 )
			screen.hide( szStringPromoFrame )

			# place the plot list unit button
			screen.addCheckBoxGFC( szString, szFileNameGovernor, szFileNameHilite, x, y, 32, 32, WidgetTypes.WIDGET_GENERAL, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
			screen.hide( szString )
##--------		BUGFfH: End Move			--------##


		# End Turn Text
		screen.setLabel( "EndTurnText", "Background", u"", CvUtil.FONT_CENTER_JUSTIFY, 0, yResolution - 188, -0.1, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setHitTest( "EndTurnText", HitTestTypes.HITTEST_NOHIT )

		# Three states for end turn button...
		screen.addDDSGFC( "ACIcon", 'Art/Interface/Screens/armageddon.dds', xResolution - (iEndOfTurnButtonSize/2) - iEndOfTurnPosX, yResolution - (iEndOfTurnButtonSize/2) - iEndOfTurnPosY, iEndOfTurnButtonSize, iEndOfTurnButtonSize, WidgetTypes.WIDGET_END_TURN, -1, -1 )
		screen.hide( "ACIcon" )
		screen.setImageButton( "EndTurnButton", "", xResolution - (iEndOfTurnButtonSize/2) - iEndOfTurnPosX, yResolution - (iEndOfTurnButtonSize/2) - iEndOfTurnPosY, iEndOfTurnButtonSize, iEndOfTurnButtonSize, WidgetTypes.WIDGET_END_TURN, -1, -1 )
		screen.setStyle( "EndTurnButton", "Button_HUDEndTurn_Style" )
		screen.setEndTurnState( "EndTurnButton", "Red" )
		screen.hide( "EndTurnButton" )

		# *********************************************************************************
		# RESEARCH BUTTONS
		# *********************************************************************************

		i = 0
		for i in range( gc.getNumTechInfos() ):
			szName = "ResearchButton" + str(i)
			screen.setImageButton( szName, gc.getTechInfo(i).getButton(), 0, 0, 32, 32, WidgetTypes.WIDGET_RESEARCH, i, -1 )
			screen.hide( szName )

		i = 0
		for i in range(gc.getNumReligionInfos()):
			szName = "ReligionButton" + str(i)
			if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_PICK_RELIGION):
				szButton = gc.getReligionInfo(i).getGenericTechButton()
			else:
				szButton = gc.getReligionInfo(i).getTechButton()
			screen.setImageButton( szName, szButton, 0, 0, 32, 32, WidgetTypes.WIDGET_RESEARCH, gc.getReligionInfo(i).getTechPrereq(), -1 )
			screen.hide( szName )

		# *********************************************************************************
		# CITIZEN BUTTONS
		# *********************************************************************************

		szHideCitizenList = []

		# Angry Citizens
		i = 0
		for i in range(MAX_CITIZEN_BUTTONS):
			szName = "AngryCitizen" + str(i)
			screen.setImageButton( szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_ANGRYCITIZEN_TEXTURE").getPath(), xResolution - 74 - (26 * i), yResolution - 238, 24, 24, WidgetTypes.WIDGET_ANGRY_CITIZEN, -1, -1 )
			screen.hide( szName )

		iCount = 0

		# Increase Specialists...
		i = 0
		for i in range( gc.getNumSpecialistInfos() ):
			if (gc.getSpecialistInfo(i).isVisible()):
				szName = "IncreaseSpecialist" + str(i)
				screen.setButtonGFC( szName, u"", "", xResolution - 46, (yResolution - 270 - (26 * iCount)), 20, 20, WidgetTypes.WIDGET_CHANGE_SPECIALIST, i, 1, ButtonStyles.BUTTON_STYLE_CITY_PLUS )
				screen.hide( szName )

				iCount = iCount + 1

		iCount = 0

		# Decrease specialists
		i = 0
		for i in range( gc.getNumSpecialistInfos() ):
			if (gc.getSpecialistInfo(i).isVisible()):
				szName = "DecreaseSpecialist" + str(i)
				screen.setButtonGFC( szName, u"", "", xResolution - 24, (yResolution - 270 - (26 * iCount)), 20, 20, WidgetTypes.WIDGET_CHANGE_SPECIALIST, i, -1, ButtonStyles.BUTTON_STYLE_CITY_MINUS )
				screen.hide( szName )

				iCount = iCount + 1

		iCount = 0

		# Citizen Buttons
		i = 0
		for i in range( gc.getNumSpecialistInfos() ):

			if (gc.getSpecialistInfo(i).isVisible()):

				szName = "CitizenDisabledButton" + str(i)
				screen.setImageButton( szName, gc.getSpecialistInfo(i).getTexture(), xResolution - 74, (yResolution - 272 - (26 * i)), 24, 24, WidgetTypes.WIDGET_DISABLED_CITIZEN, i, -1 )
				screen.enable( szName, False )
				screen.hide( szName )

				for j in range(MAX_CITIZEN_BUTTONS):
					szName = "CitizenButton" + str((i * 100) + j)
					screen.addCheckBoxGFC( szName, gc.getSpecialistInfo(i).getTexture(), "", xResolution - 74 - (26 * j), (yResolution - 272 - (26 * i)), 24, 24, WidgetTypes.WIDGET_CITIZEN, i, j, ButtonStyles.BUTTON_STYLE_LABEL )
					screen.hide( szName )

# BUG - city specialist - start
		screen.addPanel( "SpecialistBackground", u"", u"", True, False, xResolution - 243, yResolution - 449, 230, 30, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "SpecialistBackground", "Panel_City_Header_Style" )
		screen.hide( "SpecialistBackground" )
		screen.setLabel( "SpecialistLabel", "Background", localText.getText("TXT_KEY_CONCEPT_SPECIALISTS", ()), CvUtil.FONT_CENTER_JUSTIFY, xResolution - 128, yResolution - 441, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "SpecialistLabel" )
# BUG - city specialist - end

		# **********************************************************
		# GAME DATA STRINGS
		# **********************************************************

		szGameDataList = []

		xCoord = 268 + (xResolution - 1024) / 2
#		screen.addStackedBarGFC( "ResearchBar", xCoord, 2, 487, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_RESEARCH, -1, -1 )
		screen.addStackedBarGFC( "ResearchBar", xCoord, 2, 243, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_RESEARCH, -1, -1 )
		screen.setStackedBarColors( "ResearchBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RESEARCH_STORED") )
		screen.setStackedBarColors( "ResearchBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_RESEARCH_RATE") )
		screen.setStackedBarColors( "ResearchBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "ResearchBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "ResearchBar" )

#SPELL RESEARCH
#		xCoord = 268 + (xResolution - 1024) / 2
#		screen.addStackedBarGFC( "SpellResearchBar", xCoord, 52, 487, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_ARCANE_SCREEN).getActionInfoIndex(), -1 )
		xCoord = 268 + 244+(xResolution - 1024) / 2
		screen.addStackedBarGFC( "SpellResearchBar", xCoord, 2, 487-244, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_ARCANE_SCREEN).getActionInfoIndex(), -1 )
		screen.setStackedBarColors( "SpellResearchBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RESEARCH_STORED") )
		screen.setStackedBarColors( "SpellResearchBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_RESEARCH_RATE") )
		screen.setStackedBarColors( "SpellResearchBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "SpellResearchBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "SpellResearchBar" )
#SPELL RESEARCH
		xCoord = 268 + (xResolution - 1024) / 2
# BUG - Great General Bar - start
		screen.addStackedBarGFC( "GreatGeneralBar", xCoord, 27, 100, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_GREAT_GENERAL, -1, -1 )
		screen.setStackedBarColors( "GreatGeneralBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_NEGATIVE_RATE") ) #gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors( "GreatGeneralBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "GreatGeneralBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "GreatGeneralBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "GreatGeneralBar" )
# BUG - Great General Bar - end

# BUG - Great Person Bar - start
		screen.addStackedBarGFC( "GreatPersonBar", xCoord, 27, 243, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setStackedBarColors( "GreatPersonBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors( "GreatPersonBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
		screen.setStackedBarColors( "GreatPersonBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "GreatPersonBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "GreatPersonBar" )
# BUG - Great Person Bar - end

# Sephi - give Spell Research Bar a try
		screen.addStackedBarGFC( "MagicRitualBar", xCoord+243, 27, 244, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setStackedBarColors( "MagicRitualBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors( "MagicRitualBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
		screen.setStackedBarColors( "MagicRitualBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "MagicRitualBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "MagicRitualBar" )

# Sephi - give Spell Research Bar a try

# BUG - Bars on single line for higher resolution screens - start
		xCoord = 268 + (xResolution - 1440) / 2
		screen.addStackedBarGFC( "GreatGeneralBar-w", xCoord, 2, 84, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_GREAT_GENERAL, -1, -1 )
		screen.setStackedBarColors( "GreatGeneralBar-w", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_NEGATIVE_RATE") ) #gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors( "GreatGeneralBar-w", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "GreatGeneralBar-w", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "GreatGeneralBar-w", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "GreatGeneralBar-w" )

		xCoord += 6 + 84
#		screen.addStackedBarGFC( "ResearchBar-w", xCoord, 2, 487, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_RESEARCH, -1, -1 )
		screen.addStackedBarGFC( "ResearchBar-w", xCoord, 2, 243, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_RESEARCH, -1, -1 )
		screen.setStackedBarColors( "ResearchBar-w", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RESEARCH_STORED") )
		screen.setStackedBarColors( "ResearchBar-w", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_RESEARCH_RATE") )
		screen.setStackedBarColors( "ResearchBar-w", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "ResearchBar-w", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "ResearchBar-w" )
#SPELL RESEARCH
#		screen.addStackedBarGFC( "SpellResearchBar-w", xCoord, 52, 487, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_ARCANE_SCREEN).getActionInfoIndex(), -1 )
		screen.addStackedBarGFC( "SpellResearchBar-w", xCoord+244, 2, 243, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_ARCANE_SCREEN).getActionInfoIndex(), -1 )
		screen.setStackedBarColors( "SpellResearchBar-w", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RESEARCH_STORED") )
		screen.setStackedBarColors( "SpellResearchBar-w", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_RESEARCH_RATE") )
		screen.setStackedBarColors( "SpellResearchBar-w", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "SpellResearchBar-w", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "SpellResearchBar-w" )
#SPELL RESEARCH

		xCoord += 6 + 487
		screen.addStackedBarGFC( "GreatPersonBar-w", xCoord, 2, 320, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setStackedBarColors( "GreatPersonBar-w", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors( "GreatPersonBar-w", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
		screen.setStackedBarColors( "GreatPersonBar-w", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "GreatPersonBar-w", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "GreatPersonBar-w" )
# BUG - Bars on single line for higher resolution screens - end

# Sephi - give Spell Research Bar a try

		screen.addStackedBarGFC( "MagicRitualBar-w", xCoord, 2+30, 320, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.setStackedBarColors( "MagicRitualBar-w", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors( "MagicRitualBar-w", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
		screen.setStackedBarColors( "MagicRitualBar-w", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "MagicRitualBar-w", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "MagicRitualBar-w" )

# Sephi - give Spell Research Bar a try

		# *********************************************************************************
		# SELECTION DATA BUTTONS/STRINGS
		# *********************************************************************************

		szHideSelectionDataList = []

		screen.addStackedBarGFC( "PopulationBar", iCityCenterRow1X, iCityCenterRow1Y-4, xResolution - (iCityCenterRow1X*2), iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_POPULATION, -1, -1 )
		screen.setStackedBarColors( "PopulationBar", InfoBarTypes.INFOBAR_STORED, gc.getYieldInfo(YieldTypes.YIELD_FOOD).getColorType() )
		screen.setStackedBarColorsAlpha( "PopulationBar", InfoBarTypes.INFOBAR_RATE, gc.getYieldInfo(YieldTypes.YIELD_FOOD).getColorType(), 0.8 )
		screen.setStackedBarColors( "PopulationBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_NEGATIVE_RATE") )
		screen.setStackedBarColors( "PopulationBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "PopulationBar" )

		screen.addStackedBarGFC( "ProductionBar", iCityCenterRow2X, iCityCenterRow2Y-4, xResolution - (iCityCenterRow2X*2), iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_PRODUCTION, -1, -1 )
		screen.setStackedBarColors( "ProductionBar", InfoBarTypes.INFOBAR_STORED, gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getColorType() )
		screen.setStackedBarColorsAlpha( "ProductionBar", InfoBarTypes.INFOBAR_RATE, gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getColorType(), 0.8 )
		screen.setStackedBarColors( "ProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getYieldInfo(YieldTypes.YIELD_FOOD).getColorType() )
		screen.setStackedBarColors( "ProductionBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "ProductionBar" )

		screen.addStackedBarGFC( "GreatPeopleBar", xResolution - 246, yResolution - 188, 240, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_GREAT_PEOPLE, -1, -1 )
		screen.setStackedBarColors( "GreatPeopleBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_STORED") )
		screen.setStackedBarColors( "GreatPeopleBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_GREAT_PEOPLE_RATE") )
		screen.setStackedBarColors( "GreatPeopleBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "GreatPeopleBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "GreatPeopleBar" )

		screen.addStackedBarGFC( "CultureBar", 6, yResolution - 188, 240, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_CULTURE, -1, -1 )
		screen.setStackedBarColors( "CultureBar", InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_CULTURE_STORED") )
		screen.setStackedBarColors( "CultureBar", InfoBarTypes.INFOBAR_RATE, gc.getInfoTypeForString("COLOR_CULTURE_RATE") )
		screen.setStackedBarColors( "CultureBar", InfoBarTypes.INFOBAR_RATE_EXTRA, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.setStackedBarColors( "CultureBar", InfoBarTypes.INFOBAR_EMPTY, gc.getInfoTypeForString("COLOR_EMPTY") )
		screen.hide( "CultureBar" )

		# Holy City Overlay
		for i in range( gc.getNumReligionInfos() ):
			xCoord = xResolution - 242 + (i * 34)
			yCoord = 42
			szName = "ReligionHolyCityDDS" + str(i)
			screen.addDDSGFC( szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_HOLYCITY_OVERLAY").getPath(), xCoord, yCoord, 24, 24, WidgetTypes.WIDGET_HELP_RELIGION_CITY, i, -1 )
			screen.hide( szName )

		for i in range( gc.getNumCorporationInfos() ):
			xCoord = xResolution - 242 + (i * 34)
			yCoord = 66
			szName = "CorporationHeadquarterDDS" + str(i)
			screen.addDDSGFC( szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_HOLYCITY_OVERLAY").getPath(), xCoord, yCoord, 24, 24, WidgetTypes.WIDGET_HELP_CORPORATION_CITY, i, -1 )
			screen.hide( szName )

		screen.addStackedBarGFC( "NationalityBar", 6, yResolution - 214, 240, iStackBarHeight, InfoBarTypes.NUM_INFOBAR_TYPES, WidgetTypes.WIDGET_HELP_NATIONALITY, -1, -1 )
		screen.hide( "NationalityBar" )

		screen.setButtonGFC( "CityScrollMinus", u"", "", 274, 32, 32, 32, WidgetTypes.WIDGET_CITY_SCROLL, -1, -1, ButtonStyles.BUTTON_STYLE_ARROW_LEFT )
		screen.hide( "CityScrollMinus" )

		screen.setButtonGFC( "CityScrollPlus", u"", "", 288, 32, 32, 32, WidgetTypes.WIDGET_CITY_SCROLL, 1, -1, ButtonStyles.BUTTON_STYLE_ARROW_RIGHT )
		screen.hide( "CityScrollPlus" )

# BUG - City Arrows - start
		screen.setButtonGFC( "MainCityScrollMinus", u"", "", xResolution - 275, yResolution - 165, 32, 32, WidgetTypes.WIDGET_CITY_SCROLL, -1, -1, ButtonStyles.BUTTON_STYLE_ARROW_LEFT )
		screen.hide( "MainCityScrollMinus" )

		screen.setButtonGFC( "MainCityScrollPlus", u"", "", xResolution - 255, yResolution - 165, 32, 32, WidgetTypes.WIDGET_CITY_SCROLL, 1, -1, ButtonStyles.BUTTON_STYLE_ARROW_RIGHT )
		screen.hide( "MainCityScrollPlus" )
# BUG - City Arrows - end

# BUG - PLE - begin
#		screen.setButtonGFC( "PlotListMinus", u"", "", 315 + ( xResolution - (iMultiListXL+iMultiListXR) - 68 ), yResolution - 171, 32, 32, WidgetTypes.WIDGET_PLOT_LIST_SHIFT, -1, -1, ButtonStyles.BUTTON_STYLE_ARROW_LEFT )
#		screen.hide( "PlotListMinus" )
#
#		screen.setButtonGFC( "PlotListPlus", u"", "", 298 + ( xResolution - (iMultiListXL+iMultiListXR) - 34 ), yResolution - 171, 32, 32, WidgetTypes.WIDGET_PLOT_LIST_SHIFT, 1, -1, ButtonStyles.BUTTON_STYLE_ARROW_RIGHT )
#		screen.hide( "PlotListPlus" )

		screen.setButtonGFC( self.PLOT_LIST_MINUS_NAME, u"", "", 315 + ( xResolution - (iMultiListXL+iMultiListXR) - 68 ), yResolution - 171, 32, 32, WidgetTypes.WIDGET_GENERAL, -1, -1, ButtonStyles.BUTTON_STYLE_ARROW_LEFT )
		screen.hide( self.PLOT_LIST_MINUS_NAME )
		screen.setButtonGFC( self.PLOT_LIST_PLUS_NAME, u"", "", 298 + ( xResolution - (iMultiListXL+iMultiListXR) - 34 ), yResolution - 171, 32, 32, WidgetTypes.WIDGET_GENERAL, 1, -1, ButtonStyles.BUTTON_STYLE_ARROW_RIGHT )
		screen.hide( self.PLOT_LIST_PLUS_NAME )

		screen.setImageButton( self.PLOT_LIST_UP_NAME, ArtFileMgr.getInterfaceArtInfo("PLE_ARROW_UP").getPath(), 315 + ( xResolution - (iMultiListXL+iMultiListXR) - 68 ) + 5, yResolution - 171 + 5, 20, 20, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( self.PLOT_LIST_UP_NAME )
		screen.setImageButton( self.PLOT_LIST_DOWN_NAME, ArtFileMgr.getInterfaceArtInfo("PLE_ARROW_DOWN").getPath(), 298 + ( xResolution - (iMultiListXL+iMultiListXR) - 34 ) + 5, yResolution - 171 + 5, 20, 20, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( self.PLOT_LIST_DOWN_NAME )
# BUG - PLE - end

		screen.addPanel( "TradeRouteListBackground", u"", u"", True, False, 10, 157, 238, 30, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "TradeRouteListBackground", "Panel_City_Header_Style" )
		screen.hide( "TradeRouteListBackground" )

		screen.setLabel( "TradeRouteListLabel", "Background", localText.getText("TXT_KEY_HEADING_TRADEROUTE_LIST", ()), CvUtil.FONT_CENTER_JUSTIFY, 129, 165, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "TradeRouteListLabel" )

# BUG - Raw Yields - start
		nX = 10 + 24
		nY = 157 + 5
		nSize = 24
		nDist = 24
		nGap = 10
		szHighlightButton = ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_HIGHLIGHT").getPath()

		# Trade
		screen.addCheckBoxGFC( "RawYieldsTrade0", ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_TRADE").getPath(), szHighlightButton, nX, nY, nSize, nSize, WidgetTypes.WIDGET_GENERAL, 0, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.hide("RawYieldsTrade0")

		# Yields
		nX += nDist + nGap
		screen.addCheckBoxGFC( "RawYieldsFood1", ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_FOOD").getPath(), szHighlightButton, nX, nY, nSize, nSize, WidgetTypes.WIDGET_GENERAL, 1, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.hide("RawYieldsFood1")
		nX += nDist
		screen.addCheckBoxGFC( "RawYieldsProduction2", ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_PRODUCTION").getPath(), szHighlightButton, nX, nY, nSize, nSize, WidgetTypes.WIDGET_GENERAL, 2, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.hide("RawYieldsProduction2")
		nX += nDist
		screen.addCheckBoxGFC( "RawYieldsCommerce3", ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_COMMERCE").getPath(), szHighlightButton, nX, nY, nSize, nSize, WidgetTypes.WIDGET_GENERAL, 3, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.hide("RawYieldsCommerce3")

		# Tile Selection
		nX += nDist + nGap
		screen.addCheckBoxGFC( "RawYieldsWorkedTiles4", ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_WORKED_TILES").getPath(), szHighlightButton, nX, nY, nSize, nSize, WidgetTypes.WIDGET_GENERAL, 4, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.hide("RawYieldsWorkedTiles4")
		nX += nDist
		screen.addCheckBoxGFC( "RawYieldsCityTiles5", ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_CITY_TILES").getPath(), szHighlightButton, nX, nY, nSize, nSize, WidgetTypes.WIDGET_GENERAL, 5, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.hide("RawYieldsCityTiles5")
		nX += nDist
		screen.addCheckBoxGFC( "RawYieldsOwnedTiles6", ArtFileMgr.getInterfaceArtInfo("RAW_YIELDS_OWNED_TILES").getPath(), szHighlightButton, nX, nY, nSize, nSize, WidgetTypes.WIDGET_GENERAL, 6, -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.hide("RawYieldsOwnedTiles6")
# BUG - Raw Yields - end

		screen.addPanel( "BuildingListBackground", u"", u"", True, False, 10, 287, 238, 30, PanelStyles.PANEL_STYLE_STANDARD )
		screen.setStyle( "BuildingListBackground", "Panel_City_Header_Style" )
		screen.hide( "BuildingListBackground" )

		screen.setLabel( "BuildingListLabel", "Background", localText.getText("TXT_KEY_CONCEPT_BUILDINGS", ()), CvUtil.FONT_CENTER_JUSTIFY, 129, 295, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
		screen.hide( "BuildingListLabel" )

		# *********************************************************************************
		# UNIT INFO ELEMENTS
		# *********************************************************************************

		i = 0
		for i in range(gc.getNumPromotionInfos()):
			szName = "PromotionButton" + str(i)
			screen.addDDSGFC( szName, gc.getPromotionInfo(i).getButton(), 180, yResolution - 18, 24, 24, WidgetTypes.WIDGET_ACTION, gc.getPromotionInfo(i).getActionInfoIndex(), -1 )
			screen.hide( szName )

# BUG - PLE - begin
			szName = self.PLE_PROMO_BUTTONS_UNITINFO + str(i)
			screen.addDDSGFC( szName, gc.getPromotionInfo(i).getButton(), \
								180, yResolution - 18, \
								self.CFG_INFOPANE_BUTTON_SIZE, self.CFG_INFOPANE_BUTTON_SIZE, \
								WidgetTypes.WIDGET_ACTION, gc.getPromotionInfo(i).getActionInfoIndex(), -1 )
			screen.hide( szName )
# BUG - PLE - end

		# *********************************************************************************
		# SCORES
		# *********************************************************************************

		screen.addPanel( "ScoreBackground", u"", u"", True, False, 0, 0, 0, 0, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "ScoreBackground" )

		screen.addPanel( "CultureBackground", u"", u"", True, False, 0, 0, 0, 0, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "CultureBackground" )

		screen.addPanel( "MagicBackground", u"", u"", True, False, 0, 0, 0, 0, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "MagicBackground" )

#FfH: Added by Kael 10/29/2007
		screen.addPanel( "ManaBackground", u"", u"", True, False, 0, 0, 0, 0, PanelStyles.PANEL_STYLE_HUD_HELP )
		screen.hide( "ManaBackground" )
#FfH: End Add

		for i in range( gc.getMAX_PLAYERS() ):
			szName = "ScoreText" + str(i)
			screen.setText( szName, "Background", u"", CvUtil.FONT_RIGHT_JUSTIFY, 996, 622, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_CONTACT_CIV, i, -1 )
			screen.hide( szName )

		# This should be a forced redraw screen
		screen.setForcedRedraw( True )

		# This should show the screen immidiately and pass input to the game
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, True)

		szHideList = []

		szHideList.append( "CreateGroup" )
		szHideList.append( "DeleteGroup" )

		# City Tabs
		for i in range( g_NumCityTabTypes ):
			szButtonID = "CityTab" + str(i)
			szHideList.append( szButtonID )

		for i in range( g_NumHurryInfos ):
			szButtonID = "Hurry" + str(i)
			szHideList.append( szButtonID )

		szHideList.append( "Hurry0" )
		szHideList.append( "Hurry1" )
		szHideList.append( "Hurry2" )

		screen.registerHideList( szHideList, len(szHideList), 0 )

		return 0

	# Will update the screen (every 250 MS)
	def updateScreen(self):

		global g_szTimeText
		global g_iTimeTextCounter

##################################################################################################
# Winamp Start
##################################################################################################
		global g_szWinampText
##################################################################################################
# Winamp End
##################################################################################################


# BUG - Options - start
		BugOptions.write()
# BUG - Options - end

# BUG - Event Manager - start
		CvEventInterface.getEventManager().updateActiveTurn()
		CvEventInterface.getEventManager().fireEvent("gameUpdate", ((-1,),))
# BUG - Event Manager - end

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		self.m_iNumPlotListButtons = (xResolution - (iMultiListXL+iMultiListXR) - 68) / 34

		# This should recreate the minimap on load games and returns if already exists -JW
		screen.initMinimap( xResolution - 210, xResolution - 9, yResolution - 131, yResolution - 9, -0.1 )

		messageControl = CyMessageControl()

		bShow = False

		# Hide all interface widgets
		#screen.hide( "EndTurnText" )

		if ( CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY ):
			if (gc.getGame().isPaused()):
				# Pause overrides other messages
				acOutput = localText.getText("SYSTEM_GAME_PAUSED", (gc.getPlayer(gc.getGame().getPausePlayer()).getNameKey(), ))
				#screen.modifyLabel( "EndTurnText", acOutput, CvUtil.FONT_CENTER_JUSTIFY )
				screen.setEndTurnState( "EndTurnText", acOutput )
				bShow = True
			elif (messageControl.GetFirstBadConnection() != -1):
				# Waiting on a bad connection to resolve
				if (messageControl.GetConnState(messageControl.GetFirstBadConnection()) == 1):
					if (gc.getGame().isMPOption(MultiplayerOptionTypes.MPOPTION_ANONYMOUS)):
						acOutput = localText.getText("SYSTEM_WAITING_FOR_PLAYER", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), 0))
					else:
						acOutput = localText.getText("SYSTEM_WAITING_FOR_PLAYER", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), (messageControl.GetFirstBadConnection() + 1)))
					#screen.modifyLabel( "EndTurnText", acOutput, CvUtil.FONT_CENTER_JUSTIFY )
					screen.setEndTurnState( "EndTurnText", acOutput )
					bShow = True
				elif (messageControl.GetConnState(messageControl.GetFirstBadConnection()) == 2):
					if (gc.getGame().isMPOption(MultiplayerOptionTypes.MPOPTION_ANONYMOUS)):
						acOutput = localText.getText("SYSTEM_PLAYER_JOINING", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), 0))
					else:
						acOutput = localText.getText("SYSTEM_PLAYER_JOINING", (gc.getPlayer(messageControl.GetFirstBadConnection()).getNameKey(), (messageControl.GetFirstBadConnection() + 1)))
					#screen.modifyLabel( "EndTurnText", acOutput, CvUtil.FONT_CENTER_JUSTIFY )
					screen.setEndTurnState( "EndTurnText", acOutput )
					bShow = True
			else:
				# Flash select messages if no popups are present
				if ( CyInterface().shouldDisplayReturn() ):
					acOutput = localText.getText("SYSTEM_RETURN", ())
					#screen.modifyLabel( "EndTurnText", acOutput, CvUtil.FONT_CENTER_JUSTIFY )
					screen.setEndTurnState( "EndTurnText", acOutput )
					bShow = True
				elif ( CyInterface().shouldDisplayWaitingOthers() ):
					acOutput = localText.getText("SYSTEM_WAITING", ())
					#screen.modifyLabel( "EndTurnText", acOutput, CvUtil.FONT_CENTER_JUSTIFY )
					screen.setEndTurnState( "EndTurnText", acOutput )
					bShow = True
				elif ( CyInterface().shouldDisplayEndTurn() ):
# BUG - Event Manager - start
					CvEventInterface.getEventManager().updateEndTurn()
# BUG - Event Manager - end
					acOutput = localText.getText("SYSTEM_END_TURN", ())
					#screen.modifyLabel( "EndTurnText", acOutput, CvUtil.FONT_CENTER_JUSTIFY )
					screen.setEndTurnState( "EndTurnText", acOutput )
					bShow = True
				elif ( CyInterface().shouldDisplayWaitingYou() ):
					acOutput = localText.getText("SYSTEM_WAITING_FOR_YOU", ())
					#screen.modifyLabel( "EndTurnText", acOutput, CvUtil.FONT_CENTER_JUSTIFY )
					screen.setEndTurnState( "EndTurnText", acOutput )
					bShow = True

		if ( bShow ):
			screen.showEndTurn( "EndTurnText" )
			if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW or CyInterface().isCityScreenUp() ):
				screen.moveItem( "EndTurnText", 0, yResolution - 194, -0.1 )
			else:
				screen.moveItem( "EndTurnText", 0, yResolution - 86, -0.1 )
		else:
			screen.hideEndTurn( "EndTurnText" )

		screen.hide( "ACText" )
		if (not CyInterface().isCityScreenUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):
			iPlayer = gc.getGame().getActivePlayer()
			if iPlayer != -1:
				pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
				ACstr = u"<font=2i><color=%d,%d,%d,%d>%s</color></font>" %(pPlayer.getPlayerTextColorR(),pPlayer.getPlayerTextColorG(),pPlayer.getPlayerTextColorB(),pPlayer.getPlayerTextColorA(),str(CyGame().getGlobalCounter()) + str(" "))
				screen.setText( "ACText", "Background", ACstr, CvUtil.FONT_CENTER_JUSTIFY, xResolution - 296, yResolution - 157, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.setHitTest( "ACText", HitTestTypes.HITTEST_NOHIT )

		self.updateEndTurnButton()

#Civ Counters


		screen.hide( "SRText" )
		screen.hide( "Awakenedchance" )
		screen.hide( "PurText" )
		screen.hide( "ShadeText" )
		screen.hide( "SidarCounter" )
		screen.hide( "AccordText" )
		screen.hide( "Accordvalue" )
		screen.hide( "AccordButton1" )
		screen.hide( "RevoltText" )
		screen.hide( "SpiritVault" )
		screen.hide( "SpiritVaultText" )
		if (not CyInterface().isCityScreenUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):
			pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
			#Awakened display
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SCIONS') and pPlayer.getNumCities() > 0:

				#figuring spawn chance
				SF=CvScions.ScionFunctions()
				fSpawnOdds = SF.doChanceAwakenedSpawn(-1)
				fFirstPart = fSpawnOdds / 100
				fSecondPart = fSpawnOdds - (fFirstPart * 100)
				#/figuring spawn chance
				if fFirstPart + fSecondPart > 0:

					SRstr = u"<font=2i>%s</font>" %(str("Awakened Chance: ") + str(fFirstPart) + str(".") + str(fSecondPart) + str("% "))
					screen.setImageButton("Awakenedchance", ", ,Art/Interface/Buttons/Units/Units_Atlas1.dds,1,10", 20, 200, 16, 16, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.setText( "SRText", "Background", SRstr, CvUtil.FONT_LEFT_JUSTIFY, 35, 200, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.setHitTest( "SRText", HitTestTypes.HITTEST_NOHIT )
				else:
					screen.hide( "Awakenedchance" )
					screen.hide( "SRText" )
			#/Awakened display


			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_SIDAR'):
				iCounter = pPlayer.CalculateCivCounter(0,0)
				if iCounter>0:
					SRstr = u"<font=2i>%s%i%s%i</font>" %(str("Exp for Shades: "),(iCounter),str("/"),pPlayer.CalculateCivCounter(1,0))
					screen.setImageButton("SidarCounter", ", ,Art/Interface/Buttons/Units/Units_atlas4.dds,8,5", 20, 200, 16, 16, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.setText( "ShadeText", "Background", SRstr, CvUtil.FONT_LEFT_JUSTIFY, 35, 200, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				else:
					screen.hide( "SidarCounter" )
					screen.hide( "ShadeText" )


#Aristrakh
			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):

				iGlobP = pPlayer.getPurityCounterCache1()
				if pPlayer.getPurityCounterCache1() > -1:
					SRstr = u"<font=2i>%s%i%s</font>" %(str("Soul Vault: "),(iGlobP),str(" "))
					screen.setImageButton("SpiritVault", ", ,Art/Interface/Buttons/Misc_Atlas1.dds,4,16", 20, 200, 32, 32, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.setText( "SpiritVaultText", "Background", SRstr, CvUtil.FONT_LEFT_JUSTIFY, 50, 205, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				else:
					screen.hide( "SpiritVault" )
					screen.hide( "SpiritVaultText" )



#Unblemished
#			if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
#
#				iAccord = pPlayer.getPurityCounter()
#				iRevoltBase = CvUnblemishedEvents.UnblemishedFunctions().getRevoltBase()
#/figuring spawn chance
#				if iAccord > -1000:
#
#					SRstr = u"<font=2i>%s%i%s</font>" %(str("Unblemished Vim: "),(iAccord),str(" "))
#					SRstrR = u"<font=2i>%s%i%s</font>" %(str(" / "),(iRevoltBase),str("% "))
#					SRstrR = u"<font=2i>%s%i%s</font>" %(str(" / "),(iRevoltBase),str("% "))
#					screen.setImageButton("AccordButton1", "Art/Interface/Buttons/Spells/infusehealth.dds", 20, 200, 20, 20, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.setText( "AccordText", "Background", SRstr, CvUtil.FONT_LEFT_JUSTIFY, 36, 200, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.setText( "RevoltText", "Background", SRstrR, CvUtil.FONT_LEFT_JUSTIFY, 45, 220, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.setText( "RevoltText", "Background", SRstrR, CvUtil.FONT_LEFT_JUSTIFY, 162, 200, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.setHitTest( "AccordText", HitTestTypes.HITTEST_NOHIT )
#
#				else:
#					screen.hide( "Accordvalue" )
#					screen.hide( "AccordButton1" )
#					screen.hide( "RevoltText" )
#					screen.hide( "AccordText" )
#/Unblemished


# Civ Counters

# BUG - NJAGC - start
		global g_bShowTimeTextAlt
		if (CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START):
			if (ClockOpt.isEnabled()):
#				if (ClockOpt.isShowEra()):
#					screen.show( "EraText" )
#				else:
#					screen.hide( "EraText" )

				if (ClockOpt.isAlternateTimeText()):
					#global g_iTimeTextCounter (already done above)
					if (CyUserProfile().wasClockJustTurnedOn() or g_iTimeTextCounter <= 0):
						# reset timer, display primary
						g_bShowTimeTextAlt = False
						g_iTimeTextCounter = ClockOpt.getAlternatePeriod() * 1000
						CyUserProfile().setClockJustTurnedOn(False)
					else:
						# countdown timer
						g_iTimeTextCounter -= 250
						if (g_iTimeTextCounter <= 0):
							# timer elapsed, toggle between primary and alternate
							g_iTimeTextCounter = ClockOpt.getAlternatePeriod() * 1000
							g_bShowTimeTextAlt = not g_bShowTimeTextAlt
				else:
					g_bShowTimeTextAlt = False

				self.updateTimeText()
				screen.setLabel( "TimeText", "Background", g_szTimeText, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 56, 6, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.show( "TimeText" )
			else:
#				screen.hide( "EraText" )
				self.updateTimeText()
				screen.setLabel( "TimeText", "Background", g_szTimeText, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 56, 6, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.show( "TimeText" )
		else:
			screen.hide( "TimeText" )
#			screen.hide( "EraText" )
# BUG - NJAGC - end

##################################################################################################
# Winamp Start
##################################################################################################
		if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_WINAMP_GUI):
			if ((WinAMP.GetTitleName() != "") and (g_WinAMP)):
				self.updateWinampText()
				screen.setLabel("WinampText", "Background", g_szWinampText, CvUtil.FONT_CENTER_JUSTIFY, xResolution / 2, 35, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				if ((self.WinAMPCounter == 1) and not (CyInterface().isCityScreenUp())):
					screen.show("WinampText")
				else:
					screen.hide("WinampText")
##################################################################################################
# Winamp End
##################################################################################################


		return 0

	# Will redraw the interface
	def redraw( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# field of view
		self.setFieldofView(screen, CyInterface().isCityScreenUp())

		# Check Dirty Bits, see what we need to redraw...
		if (CyInterface().isDirty(InterfaceDirtyBits.PercentButtons_DIRTY_BIT) == True):
			# Percent Buttons
			self.updatePercentButtons()
			CyInterface().setDirty(InterfaceDirtyBits.PercentButtons_DIRTY_BIT, False)
		if (CyInterface().isDirty(InterfaceDirtyBits.Flag_DIRTY_BIT) == True):
			# Percent Buttons
			self.updateFlag()
			CyInterface().setDirty(InterfaceDirtyBits.Flag_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.MiscButtons_DIRTY_BIT) == True ):
			# Miscellaneous buttons (civics screen, etc)
			self.updateMiscButtons()
			CyInterface().setDirty(InterfaceDirtyBits.MiscButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.InfoPane_DIRTY_BIT) == True ):
			# Info Pane Dirty Bit
			# This must come before updatePlotListButtons so that the entity widget appears in front of the stats
			self.updateInfoPaneStrings()
			CyInterface().setDirty(InterfaceDirtyBits.InfoPane_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.PlotListButtons_DIRTY_BIT) == True ):
			# Plot List Buttons Dirty
			self.updatePlotListButtons()
			CyInterface().setDirty(InterfaceDirtyBits.PlotListButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.SelectionButtons_DIRTY_BIT) == True ):
			# Selection Buttons Dirty
			self.updateSelectionButtons()
			CyInterface().setDirty(InterfaceDirtyBits.SelectionButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.ResearchButtons_DIRTY_BIT) == True ):
			# Research Buttons Dirty
			self.updateResearchButtons()
			CyInterface().setDirty(InterfaceDirtyBits.ResearchButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.CitizenButtons_DIRTY_BIT) == True ):
			# Citizen Buttons Dirty

# BUG - city specialist - start
			self.updateCitizenButtons_hide()
			if (CityScreenOpt.isCitySpecialist_Stacker()):
				self.updateCitizenButtons_Stacker()
			elif (CityScreenOpt.isCitySpecialist_Chevron()):
				self.updateCitizenButtons_Chevron()
			else:
				self.updateCitizenButtons()
# BUG - city specialist - end

			CyInterface().setDirty(InterfaceDirtyBits.CitizenButtons_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.GameData_DIRTY_BIT) == True ):
			# Game Data Strings Dirty
			self.updateGameDataStrings()
			CyInterface().setDirty(InterfaceDirtyBits.GameData_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.Help_DIRTY_BIT) == True ):
			# Help Dirty bit
			self.updateHelpStrings()
			CyInterface().setDirty(InterfaceDirtyBits.Help_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.CityScreen_DIRTY_BIT) == True ):
			# Selection Data Dirty Bit
			self.updateCityScreen()
			CyInterface().setDirty(InterfaceDirtyBits.Domestic_Advisor_DIRTY_BIT, True)
			CyInterface().setDirty(InterfaceDirtyBits.CityScreen_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.Score_DIRTY_BIT) == True or CyInterface().checkFlashUpdate() ):
			# Scores!
			self.updateScoreStrings()

			self.updateCultureStrings()
			self.updateMagicStrings()

#FfH: Added by Kael 04/30/2007
			self.updateManaStrings()
#FfH: End Add

			CyInterface().setDirty(InterfaceDirtyBits.Score_DIRTY_BIT, False)
		if ( CyInterface().isDirty(InterfaceDirtyBits.GlobeInfo_DIRTY_BIT) == True ):
			# Globeview and Globelayer buttons
			CyInterface().setDirty(InterfaceDirtyBits.GlobeInfo_DIRTY_BIT, False)
			self.updateGlobeviewButtons()

		return 0

	# Will update the percent buttons
	def updatePercentButtons( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		for iI in range( CommerceTypes.NUM_COMMERCE_TYPES ):
			szString = "IncreasePercent" + str(iI)
			screen.hide( szString )
			szString = "DecreasePercent" + str(iI)
			screen.hide( szString )
# BUG - Min/Max Sliders - start
			szString = "MaxPercent" + str(iI)
			screen.hide( szString )
			szString = "MinPercent" + str(iI)
			screen.hide( szString )
# BUG - Min/Max Sliders - start

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		if ( not CyInterface().isCityScreenUp() or ( pHeadSelectedCity.getOwner() == gc.getGame().getActivePlayer() ) or gc.getGame().isDebugMode() ):
			iCount = 0

			if ( CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START):
				for iI in range( CommerceTypes.NUM_COMMERCE_TYPES ):
					if (not CyInterface().isCityScreenUp()) or pHeadSelectedCity.getCommercePercentTypeForced()==-1 or pHeadSelectedCity.getCommercePercentTypeForced()==iI:
						# Intentional offset...
						eCommerce = (iI + 1) % CommerceTypes.NUM_COMMERCE_TYPES

						if eCommerce==CommerceTypes.COMMERCE_MANA:
							continue

						if eCommerce==CommerceTypes.COMMERCE_ARCANE and not CyInterface().isCityScreenUp() and not gc.getActivePlayer().isCommerceFlexible(eCommerce):
							#SpyFanatic: make an empty row for spellresearch in main screen
							iCount = iCount + 1
							continue

						iShift = 60
						if (CyInterface().isCityScreenUp()):
							iShift = -30

						bShow=false
						if (gc.getActivePlayer().isCommerceFlexible(eCommerce)):
							bShow=true
						if (CyInterface().isCityScreenUp() and eCommerce == CommerceTypes.COMMERCE_GOLD):
							bShow=true
						if (CyInterface().isCityScreenUp() and pHeadSelectedCity.getCommercePercentTypeForced()!=-1):
							bShow=false
						if (not bShow) and CyInterface().isCityScreenUp():
							if pHeadSelectedCity.getCommerceRate(eCommerce)!=0:
								iCount=iCount+1

						if bShow:
	# BUG - Min/Max Sliders - start
							bEnable = gc.getActivePlayer().isCommerceFlexible(eCommerce)
							if MainOpt.isShowMinMaxCommerceButtons() and not CyInterface().isCityScreenUp():
								iMinMaxAdjustX = 20
								szString = "MaxPercent" + str(eCommerce)
								screen.setButtonGFC( szString, u"", "", 70, 50 + (19 * iCount)+iShift, 20, 20, WidgetTypes.WIDGET_CHANGE_PERCENT, eCommerce, 100, ButtonStyles.BUTTON_STYLE_CITY_PLUS )
								screen.show( szString )
								screen.enable( szString, bEnable )
								szString = "MinPercent" + str(eCommerce)
								screen.setButtonGFC( szString, u"", "", 130, 50 + (19 * iCount)+iShift, 20, 20, WidgetTypes.WIDGET_CHANGE_PERCENT, eCommerce, -100, ButtonStyles.BUTTON_STYLE_CITY_MINUS )
								screen.show( szString )
								screen.enable( szString, bEnable )
							else:
								iMinMaxAdjustX = 0

							szString = "IncreasePercent" + str(eCommerce)
							screen.setButtonGFC( szString, u"", "", 70 + iMinMaxAdjustX, 50 + (19 * iCount)+iShift, 20, 20, WidgetTypes.WIDGET_CHANGE_PERCENT, eCommerce, gc.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"), ButtonStyles.BUTTON_STYLE_CITY_PLUS )
							screen.show( szString )
							screen.enable( szString, bEnable )
							szString = "DecreasePercent" + str(eCommerce)
							screen.setButtonGFC( szString, u"", "", 90 + iMinMaxAdjustX, 50 + (19 * iCount)+iShift, 20, 20, WidgetTypes.WIDGET_CHANGE_PERCENT, eCommerce, -gc.getDefineINT("COMMERCE_PERCENT_CHANGE_INCREMENTS"), ButtonStyles.BUTTON_STYLE_CITY_MINUS )
							screen.show( szString )
							screen.enable( szString, bEnable )

							iCount = iCount + 1
							# moved enabling above
# BUG - Min/Max Sliders - end

		return 0

	# Will update the end Turn Button
	def updateEndTurnButton( self ):

		global g_eEndTurnButtonState

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		if ( CyInterface().shouldDisplayEndTurnButton() and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):

			eState = CyInterface().getEndTurnState()

			bShow = False

			if ( eState == EndTurnButtonStates.END_TURN_OVER_HIGHLIGHT ):
				screen.setEndTurnState( "EndTurnButton", u"Red" )
				bShow = True
			elif ( eState == EndTurnButtonStates.END_TURN_OVER_DARK ):
				screen.setEndTurnState( "EndTurnButton", u"Red" )
				bShow = True
			elif ( eState == EndTurnButtonStates.END_TURN_GO ):
				screen.setEndTurnState( "EndTurnButton", u"Green" )
				bShow = True

			if ( bShow ):
				screen.showEndTurn( "EndTurnButton" )
			else:
				screen.hideEndTurn( "EndTurnButton" )

			if ( g_eEndTurnButtonState == eState ):
				return

			g_eEndTurnButtonState = eState

		else:
			screen.hideEndTurn( "EndTurnButton" )

		return 0

	# Update the miscellaneous buttons
	def updateMiscButtons( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		xResolution = screen.getXResolution()

# BUG - Great Person Bar - start
		self.updateGreatPersonBar(screen)
# BUG - Great Person Bar - end

		self.updateCultureStrings()
		self.updateMagicStrings()

		self.updateManaStrings()
		if ( CyInterface().shouldDisplayFlag() and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
			screen.show( "CivilizationFlag" )
			screen.show( "InterfaceHelpButton" )
			screen.show( "MainMenuButton" )
		else:
			screen.hide( "CivilizationFlag" )
			screen.hide( "InterfaceHelpButton" )
			screen.hide( "MainMenuButton" )

		if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE_ALL or CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_MINIMAP_ONLY ):
			screen.hide( "RawManaButton1" )
			screen.hide( "InterfaceLeftBackgroundWidget" )
			screen.hide( "InterfaceTopBackgroundWidget" )
			screen.hide( "InterfaceCenterBackgroundWidget" )
			screen.hide( "ACIcon" )
			screen.hide( "InterfaceRightBackgroundWidget" )
			screen.hide( "MiniMapPanel" )
			screen.hide( "InterfaceTopLeft" )
			screen.hide( "MainExtra1" )
			screen.hide( "InterfaceTopCenter" )
			screen.hide( "InterfaceTopRight" )
			screen.hide( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.hide( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.hide( "GuildButton" )
			screen.hide( "ArcaneButton" )
			screen.hide( "MagicButton" )
#Houses of Erebus
			screen.hide( "EspionageAdvisorButton" )
			screen.hide( "DomesticAdvisorButton" )
			screen.hide( "ForeignAdvisorButton" )
			screen.hide( "TechAdvisorButton" )
			screen.hide( "CivicsAdvisorButton" )
			screen.hide( "ReligiousAdvisorButton" )
			screen.hide( "CorporationAdvisorButton" )
			screen.hide( "FinanceAdvisorButton" )
			screen.hide( "MilitaryAdvisorButton" )
			screen.hide( "VictoryAdvisorButton" )
			screen.hide( "InfoAdvisorButton" )
# BUG - NJAGC - start
#			screen.hide( "EraText" )
# BUG - NJAGC - end
# BUG - City Arrows - start
			screen.hide( "MainCityScrollMinus" )
			screen.hide( "MainCityScrollPlus" )
# BUG - City Arrows - end

# BUG - field of view slider - start
			screen.hide(self.szSliderTextId)
			screen.hide(self.szSliderId)
# BUG - field of view slider - end

		elif ( CyInterface().isCityScreenUp() ):
			screen.hide( "RawManaButton1" )
			screen.show( "InterfaceLeftBackgroundWidget" )
			screen.show( "InterfaceTopBackgroundWidget" )
			screen.show( "InterfaceCenterBackgroundWidget" )
			screen.show( "InterfaceRightBackgroundWidget" )
			screen.show( "MiniMapPanel" )

			screen.hide( "InterfaceTopLeft" )
			screen.hide( "InterfaceTopCenter" )
			screen.hide( "InterfaceTopRight" )

			screen.hide( "ACIcon" )
			screen.hide( "MainExtra1" )
			screen.hide( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.hide( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.hide( "GuildButton" )
			screen.hide( "ArcaneButton" )
			screen.hide( "MagicButton" )
#Houses of Erebus

			screen.hide( "EspionageAdvisorButton" )
			screen.hide( "DomesticAdvisorButton" )
			screen.hide( "ForeignAdvisorButton" )
			screen.hide( "TechAdvisorButton" )
			screen.hide( "CivicsAdvisorButton" )
			screen.hide( "ReligiousAdvisorButton" )
			screen.hide( "CorporationAdvisorButton" )
			screen.hide( "FinanceAdvisorButton" )
			screen.hide( "MilitaryAdvisorButton" )
			screen.hide( "VictoryAdvisorButton" )
			screen.hide( "InfoAdvisorButton" )
# BUG - City Arrows - start
			screen.hide( "MainCityScrollMinus" )
			screen.hide( "MainCityScrollPlus" )
# BUG - City Arrows - end
# BUG - field of view slider - start
			screen.hide(self.szSliderTextId)
			screen.hide(self.szSliderId)
# BUG - field of view slider - end

		elif ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE):
			screen.hide( "RawManaButton1" )
			screen.hide( "InterfaceLeftBackgroundWidget" )
			screen.show( "InterfaceTopBackgroundWidget" )
			screen.hide( "InterfaceCenterBackgroundWidget" )
			screen.hide( "ACIcon" )
			screen.hide( "InterfaceRightBackgroundWidget" )
			screen.hide( "MiniMapPanel" )
			screen.show( "InterfaceTopLeft" )
			screen.show( "MainExtra1" )
			screen.show( "InterfaceTopCenter" )
			screen.show( "InterfaceTopRight" )
			screen.show( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.show( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.show( "GuildButton" )
			screen.show( "ArcaneButton" )
			screen.show( "MagicButton" )
#Houses of Erebus

			screen.show( "EspionageAdvisorButton" )
			screen.show( "DomesticAdvisorButton" )
			screen.show( "ForeignAdvisorButton" )
			screen.show( "TechAdvisorButton" )
			screen.show( "CivicsAdvisorButton" )
			screen.show( "ReligiousAdvisorButton" )
			screen.show( "CorporationAdvisorButton" )
			screen.show( "FinanceAdvisorButton" )
			screen.show( "MilitaryAdvisorButton" )
			screen.show( "VictoryAdvisorButton" )
			screen.show( "InfoAdvisorButton" )
# BUG - City Arrows - start
			screen.hide( "MainCityScrollMinus" )
			screen.hide( "MainCityScrollPlus" )
# BUG - City Arrows - end
# BUG - field of view slider - start
			screen.hide(self.szSliderTextId)
			screen.hide(self.szSliderId)
# BUG - field of view slider - end

			screen.moveToFront( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.moveToFront( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.moveToFront( "GuildButton" )
			screen.moveToFront( "ArcaneButton" )
			screen.moveToFront( "MagicButton" )
#Houses of Erebus

			screen.moveToFront( "EspionageAdvisorButton" )
			screen.moveToFront( "DomesticAdvisorButton" )
			screen.moveToFront( "ForeignAdvisorButton" )
			screen.moveToFront( "TechAdvisorButton" )
			screen.moveToFront( "CivicsAdvisorButton" )
			screen.moveToFront( "ReligiousAdvisorButton" )
			screen.moveToFront( "CorporationAdvisorButton" )
			screen.moveToFront( "FinanceAdvisorButton" )
			screen.moveToFront( "MilitaryAdvisorButton" )
			screen.moveToFront( "VictoryAdvisorButton" )
			screen.moveToFront( "InfoAdvisorButton" )

		elif (CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_ADVANCED_START):
			screen.hide( "RawManaButton1" )
			screen.hide( "InterfaceLeftBackgroundWidget" )
			screen.hide( "InterfaceCenterBackgroundWidget" )
			screen.hide( "ACIcon" )
			screen.hide( "InterfaceRightBackgroundWidget" )
			screen.show( "MiniMapPanel" )
			screen.hide( "InterfaceTopLeft" )
			screen.hide( "MainExtra1" )
			screen.hide( "InterfaceTopCenter" )
			screen.hide( "InterfaceTopRight" )
			screen.hide( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.hide( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.hide( "GuildButton" )
			screen.hide( "ArcaneButton" )
			screen.hide( "MagicButton" )
#Houses of Erebus

			screen.hide( "EspionageAdvisorButton" )
			screen.hide( "DomesticAdvisorButton" )
			screen.hide( "ForeignAdvisorButton" )
			screen.hide( "TechAdvisorButton" )
			screen.hide( "CivicsAdvisorButton" )
			screen.hide( "ReligiousAdvisorButton" )
			screen.hide( "CorporationAdvisorButton" )
			screen.hide( "FinanceAdvisorButton" )
			screen.hide( "MilitaryAdvisorButton" )
			screen.hide( "VictoryAdvisorButton" )
			screen.hide( "InfoAdvisorButton" )
# BUG - City Arrows - start
			screen.hide( "MainCityScrollMinus" )
			screen.hide( "MainCityScrollPlus" )
# BUG - City Arrows - end

		elif ( CyEngine().isGlobeviewUp() ):
			screen.hide( "RawManaButton1" )
			screen.hide( "InterfaceLeftBackgroundWidget" )
			screen.hide( "InterfaceTopBackgroundWidget" )
			screen.hide( "InterfaceCenterBackgroundWidget" )
			screen.show( "ACIcon" )
			screen.show( "InterfaceRightBackgroundWidget" )
			screen.show( "MiniMapPanel" )
			screen.show( "InterfaceTopLeft" )
			screen.show( "MainExtra1" )
			screen.show( "InterfaceTopCenter" )
			screen.show( "InterfaceTopRight" )
			screen.show( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.show( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.show( "GuildButton" )
			screen.show( "ArcaneButton" )
			screen.show( "MagicButton" )
#Houses of Erebus

			screen.show( "EspionageAdvisorButton" )
			screen.show( "DomesticAdvisorButton" )
			screen.show( "ForeignAdvisorButton" )
			screen.show( "TechAdvisorButton" )
			screen.show( "CivicsAdvisorButton" )
			screen.show( "ReligiousAdvisorButton" )
			screen.show( "CorporationAdvisorButton" )
			screen.show( "FinanceAdvisorButton" )
			screen.show( "MilitaryAdvisorButton" )
			screen.show( "VictoryAdvisorButton" )
			screen.show( "InfoAdvisorButton" )
# BUG - City Arrows - start
			screen.hide( "MainCityScrollMinus" )
			screen.hide( "MainCityScrollPlus" )
# BUG - City Arrows - end
# BUG - field of view slider - start
			screen.hide(self.szSliderTextId)
			screen.hide(self.szSliderId)
# BUG - field of view slider - end

			screen.moveToFront( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.moveToFront( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.moveToFront( "GuildButton" )
			screen.moveToFront( "ArcaneButton" )
			screen.moveToFront( "MagicButton" )
#Houses of Erebus

			screen.moveToFront( "EspionageAdvisorButton" )
			screen.moveToFront( "DomesticAdvisorButton" )
			screen.moveToFront( "ForeignAdvisorButton" )
			screen.moveToFront( "TechAdvisorButton" )
			screen.moveToFront( "CivicsAdvisorButton" )
			screen.moveToFront( "ReligiousAdvisorButton" )
			screen.moveToFront( "CorporationAdvisorButton" )
			screen.moveToFront( "FinanceAdvisorButton" )
			screen.moveToFront( "MilitaryAdvisorButton" )
			screen.moveToFront( "VictoryAdvisorButton" )
			screen.moveToFront( "InfoAdvisorButton" )

		else:
			screen.show( "RawManaButton1" )
			screen.show( "InterfaceLeftBackgroundWidget" )
			screen.show( "InterfaceTopBackgroundWidget" )
			screen.show( "InterfaceCenterBackgroundWidget" )
			screen.show( "ACIcon" )
			screen.show( "InterfaceRightBackgroundWidget" )
			screen.show( "MiniMapPanel" )
			screen.show( "InterfaceTopLeft" )
			screen.show( "MainExtra1" )
			screen.show( "InterfaceTopCenter" )
			screen.show( "InterfaceTopRight" )
			screen.show( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.show( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.show( "GuildButton" )
			screen.show( "ArcaneButton" )
			screen.show( "MagicButton" )
#Houses of Erebus

			screen.show( "EspionageAdvisorButton" )
			screen.show( "DomesticAdvisorButton" )
			screen.show( "ForeignAdvisorButton" )
			screen.show( "TechAdvisorButton" )
			screen.show( "CivicsAdvisorButton" )
			screen.show( "ReligiousAdvisorButton" )
			screen.show( "CorporationAdvisorButton" )
			screen.show( "FinanceAdvisorButton" )
			screen.show( "MilitaryAdvisorButton" )
			screen.show( "VictoryAdvisorButton" )
			screen.show( "InfoAdvisorButton" )
# BUG - City Arrows - start
			if (MainOpt.isShowCityCycleArrows()):
				screen.show( "MainCityScrollMinus" )
				screen.show( "MainCityScrollPlus" )
			else:
				screen.hide( "MainCityScrollMinus" )
				screen.hide( "MainCityScrollPlus" )
# BUG - City Arrows - end
# BUG - field of view slider - start
			if (MainOpt.isShowFieldOfView()):
				screen.show(self.szSliderTextId)
				screen.show(self.szSliderId)
			else:
				screen.hide(self.szSliderTextId)
				screen.hide(self.szSliderId)
# BUG - field of view slider - end

			screen.moveToFront( "TurnLogButton" )

#FfH: Added by Kael 09/24/2008
			screen.moveToFront( "TrophyButton" )
#FfH: End Add
#Houses of Erebus
			screen.moveToFront( "GuildButton" )
			screen.moveToFront( "ArcaneButton" )
			screen.moveToFront( "MagicButton" )
#Houses of Erebus

			screen.moveToFront( "EspionageAdvisorButton" )
			screen.moveToFront( "DomesticAdvisorButton" )
			screen.moveToFront( "ForeignAdvisorButton" )
			screen.moveToFront( "TechAdvisorButton" )
			screen.moveToFront( "CivicsAdvisorButton" )
			screen.moveToFront( "ReligiousAdvisorButton" )
			screen.moveToFront( "CorporationAdvisorButton" )
			screen.moveToFront( "FinanceAdvisorButton" )
			screen.moveToFront( "MilitaryAdvisorButton" )
			screen.moveToFront( "VictoryAdvisorButton" )
			screen.moveToFront( "InfoAdvisorButton" )

		screen.updateMinimapVisibility()

		return 0

	# Update plot List Buttons
	def updatePlotListButtons( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		bHandled = False
		if ( CyInterface().shouldDisplayUnitModel() and CyEngine().isGlobeviewUp() == false and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL ):
			if ( CyInterface().isCitySelection() ):

				iOrders = CyInterface().getNumOrdersQueued()

				for i in range( iOrders ):
					if ( bHandled == False ):
						eOrderNodeType = CyInterface().getOrderNodeType(i)
						if (eOrderNodeType  == OrderTypes.ORDER_TRAIN ):
							screen.addUnitGraphicGFC( "InterfaceUnitModel", CyInterface().getOrderNodeData1(i), 115, yResolution - 138, 123, 132, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1,  -20, 30, 1, False )
							bHandled = True
						elif ( eOrderNodeType == OrderTypes.ORDER_CONSTRUCT ):
							screen.addBuildingGraphicGFC( "InterfaceUnitModel", CyInterface().getOrderNodeData1(i), 115, yResolution - 138, 123, 132, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1,  -20, 30, 0.8, False )
							bHandled = True
						elif ( eOrderNodeType == OrderTypes.ORDER_CREATE ):
							if(gc.getProjectInfo(CyInterface().getOrderNodeData1(i)).isSpaceship()):
								modelType = 0
								screen.addSpaceShipWidgetGFC("InterfaceUnitModel", 115, yResolution - 138, 123, 132, CyInterface().getOrderNodeData1(i), modelType, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1)
							else:
								screen.hide( "InterfaceUnitModel" )
							bHandled = True
						elif ( eOrderNodeType == OrderTypes.ORDER_MAINTAIN ):
							screen.hide( "InterfaceUnitModel" )
							bHandled = True

				if ( not bHandled ):
					screen.hide( "InterfaceUnitModel" )
					bHandled = True

				screen.moveToFront("SelectedCityText")

			elif ( CyInterface().getHeadSelectedUnit() ):
				screen.addSpecificUnitGraphicGFC( "InterfaceUnitModel", CyInterface().getHeadSelectedUnit(), -20, yResolution - 350, 160, 198, WidgetTypes.WIDGET_UNIT_MODEL, CyInterface().getHeadSelectedUnit().getUnitType(), -1,  -20, 30, 1, False )
				screen.moveToFront("SelectedUnitText")
			else:
				screen.hide( "InterfaceUnitModel" )
		else:
			screen.hide( "InterfaceUnitModel" )

		pPlot = CyInterface().getSelectionPlot()

		for i in range(gc.getNumPromotionInfos()):
			szName = "PromotionButton" + str(i)
			screen.moveToFront( szName )

		screen.hide( "PlotListMinus" )
		screen.hide( "PlotListPlus" )

		for j in range(gc.getMAX_PLOT_LIST_ROWS()):
			#szStringPanel = "PlotListPanel" + str(j)
			#screen.hide(szStringPanel)

			for i in range(self.numPlotListButtons()):
				szString = "PlotListButton" + str(j*self.numPlotListButtons()+i)
				screen.hide( szString )

				szStringHealth = szString + "Health"
				screen.hide( szStringHealth )

				szStringIcon = szString + "Icon"
				screen.hide( szStringIcon )

		if ( pPlot and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyEngine().isGlobeviewUp() == False):

			iVisibleUnits = CyInterface().getNumVisibleUnits()
			iCount = -(CyInterface().getPlotListColumn())


			bLeftArrow = False
			bRightArrow = False

			if (CyInterface().isCityScreenUp()):
				iMaxRows = 1
				iSkipped = (gc.getMAX_PLOT_LIST_ROWS() - 1) * self.numPlotListButtons()
				iCount += iSkipped
			else:
				iMaxRows = gc.getMAX_PLOT_LIST_ROWS()
				iCount += CyInterface().getPlotListOffset()
				iSkipped = 0

			CyInterface().cacheInterfacePlotUnits(pPlot)
			for i in range(CyInterface().getNumCachedInterfacePlotUnits()):
				pLoopUnit = CyInterface().getCachedInterfacePlotUnit(i)
				if (pLoopUnit):

					if ((iCount == 0) and (CyInterface().getPlotListColumn() > 0)):
						bLeftArrow = True
					elif ((iCount == (gc.getMAX_PLOT_LIST_ROWS() * self.numPlotListButtons() - 1)) and ((iVisibleUnits - iCount - CyInterface().getPlotListColumn() + iSkipped) > 1)):
						bRightArrow = True

					if ((iCount >= 0) and (iCount <  self.numPlotListButtons() * gc.getMAX_PLOT_LIST_ROWS())):
						if ((pLoopUnit.getTeam() != gc.getGame().getActiveTeam()) or pLoopUnit.isWaiting()):
							szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_FORTIFY").getPath()

						elif (pLoopUnit.canMove()):
							if (pLoopUnit.hasMoved()):
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_HASMOVED").getPath()
							else:
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_MOVE").getPath()
						else:
							szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_NOMOVE").getPath()

						szString = "PlotListButton" + str(iCount)
						screen.changeImageButton( szString, pLoopUnit.getButton() )
						if ( pLoopUnit.getOwner() == gc.getGame().getActivePlayer() ):
							bEnable = True
						else:
							bEnable = False
						screen.enable(szString, bEnable)

						if (pLoopUnit.IsSelected()):
							screen.setState(szString, True)
						else:
							screen.setState(szString, False)
						screen.show( szString )

						# place the health bar
						if (pLoopUnit.isFighting()):
							bShowHealth = False
						elif (pLoopUnit.getDomainType() == DomainTypes.DOMAIN_AIR):
							bShowHealth = pLoopUnit.canAirAttack()
						else:
							bShowHealth = pLoopUnit.canFight()

						if bShowHealth:
							szStringHealth = szString + "Health"
							screen.setBarPercentage( szStringHealth, InfoBarTypes.INFOBAR_STORED, float( pLoopUnit.currHitPoints() ) / float( pLoopUnit.maxHitPoints() ) )
							if (pLoopUnit.getDamage() >= ((pLoopUnit.maxHitPoints() * 2) / 3)):
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RED"))
							elif (pLoopUnit.getDamage() >= (pLoopUnit.maxHitPoints() / 3)):
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_YELLOW"))
							else:
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREEN"))
							screen.show( szStringHealth )

						# Adds the overlay first
						szStringIcon = szString + "Icon"
						screen.changeDDSGFC( szStringIcon, szFileName )
						screen.show( szStringIcon )

					iCount = iCount + 1

			if (iVisibleUnits > self.numPlotListButtons() * iMaxRows):
				screen.enable("PlotListMinus", bLeftArrow)
				screen.show( "PlotListMinus" )

				screen.enable("PlotListPlus", bRightArrow)
				screen.show( "PlotListPlus" )

		return 0

	def updatePlotListButtons_Hide( self, screen ):
		# hide all buttons
		self.hidePlotListButtonPLEObjects(screen)
		self.hideUnitInfoPane()

		self.hidePlotListButtonNonPLEObjects(screen)

	def hidePlotListButtonNonPLEObjects(self, screen):
		for j in range(gc.getMAX_PLOT_LIST_ROWS()):
			for i in range(self.numPlotListButtons()):
				szString = "PlotListButton" + str(j*self.numPlotListButtons()+i)
				screen.hide( szString )

				szStringHealth = szString + "Health"
				screen.hide( szStringHealth )

				szStringIcon = szString + "Icon"
				screen.hide( szStringIcon )

				szStringPromoFrame = szString + "PromoFrame"
				screen.hide( szStringPromoFrame )

				szStringActionIcon = szString + "ActionIcon"
				screen.hide( szStringActionIcon )

				szStringUpgrade = szString + "Upgrade"
				screen.hide( szStringUpgrade )

				szStringUpgrade = szString + "GreatGeneral"
				screen.hide( szStringUpgrade )

				szStringIcon = szString + "Weapon"
				screen.hide( szStringIcon )

				szStringIcon = szString + "Armor"
				screen.hide( szStringIcon )

		return 0

	def updatePlotListButtons_Common( self, screen ):

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		# Capture these for looping over the plot's units
		self.bShowWoundedIndicator = PleOpt.isShowWoundedIndicator()
		self.bShowGreatGeneralIndicator = PleOpt.isShowGreatGeneralIndicator()
		self.bShowPromotionIndicator = PleOpt.isShowPromotionIndicator()
		self.bShowUpgradeIndicator = PleOpt.isShowUpgradeIndicator()
		self.bShowMissionInfo = PleOpt.isShowMissionInfo()

		self.bShowHealthBar = PleOpt.isShowHealthBar()
		self.bHideHealthBarWhileFighting = PleOpt.isHideHealthFighting()
		self.bShowMoveBar = PleOpt.isShowMoveBar()

		bHandled = False
		if ( CyInterface().shouldDisplayUnitModel() and not CyEngine().isGlobeviewUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL ):
			if ( CyInterface().isCitySelection() ):

				iOrders = CyInterface().getNumOrdersQueued()

				for i in range( iOrders ):
					if ( bHandled == False ):
						eOrderNodeType = CyInterface().getOrderNodeType(i)
						if (eOrderNodeType  == OrderTypes.ORDER_TRAIN ):

#FfH: Modified by Kael 07/18/2008
#							screen.addUnitGraphicGFC( "InterfaceUnitModel", CyInterface().getOrderNodeData1(i), 175, yResolution - 138, 123, 132, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1,  -20, 30, 1, False )
							screen.addUnitGraphicGFC( "InterfaceUnitModel", CyInterface().getOrderNodeData1(i), 115, yResolution - 138, 123, 132, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1,  -20, 30, 1, False )
#FfH: End Modify

							bHandled = True
						elif ( eOrderNodeType == OrderTypes.ORDER_CONSTRUCT ):

#FfH: Modified by Kael 07/18/2008
#							screen.addBuildingGraphicGFC( "InterfaceUnitModel", CyInterface().getOrderNodeData1(i), 175, yResolution - 138, 123, 132, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1,  -20, 30, 0.8, False )
							screen.addBuildingGraphicGFC( "InterfaceUnitModel", CyInterface().getOrderNodeData1(i), 115, yResolution - 138, 123, 132, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1,  -20, 30, 0.8, False )
#FfH: End Modify

							bHandled = True
						elif ( eOrderNodeType == OrderTypes.ORDER_CREATE ):
							if(gc.getProjectInfo(CyInterface().getOrderNodeData1(i)).isSpaceship()):
								modelType = 0

#FfH: Modified by Kael 07/18/2008
#								screen.addSpaceShipWidgetGFC("InterfaceUnitModel", 175, yResolution - 138, 123, 132, CyInterface().getOrderNodeData1(i), modelType, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1)
								screen.addSpaceShipWidgetGFC("InterfaceUnitModel", 115, yResolution - 138, 123, 132, CyInterface().getOrderNodeData1(i), modelType, WidgetTypes.WIDGET_HELP_SELECTED, 0, -1)
#FfH: End Modify

							else:
								screen.hide( "InterfaceUnitModel" )
							bHandled = True
						elif ( eOrderNodeType == OrderTypes.ORDER_MAINTAIN ):
							screen.hide( "InterfaceUnitModel" )
							bHandled = True

				if ( not bHandled ):
					screen.hide( "InterfaceUnitModel" )
					bHandled = True

				screen.moveToFront("SelectedCityText")

			elif ( CyInterface().getHeadSelectedUnit() ):

#FfH: Modified by Kael 07/17/2008
#				screen.addSpecificUnitGraphicGFC( "InterfaceUnitModel", CyInterface().getHeadSelectedUnit(), 175, yResolution - 138, 123, 132, WidgetTypes.WIDGET_UNIT_MODEL, CyInterface().getHeadSelectedUnit().getUnitType(), -1,  -20, 30, 1, False )
				screen.addSpecificUnitGraphicGFC( "InterfaceUnitModel", CyInterface().getHeadSelectedUnit(), -20, yResolution - 350, 160, 198, WidgetTypes.WIDGET_UNIT_MODEL, CyInterface().getHeadSelectedUnit().getUnitType(), -1,  -20, 30, 1, False )
#FfH: End Modify

				screen.moveToFront("SelectedUnitText")
			else:
				screen.hide( "InterfaceUnitModel" )
		else:
			screen.hide( "InterfaceUnitModel" )

	def updatePlotListButtons_Orig( self, screen ):

# need to put in something similar to 	def displayUnitPlotListObjects( self, screen, pLoopUnit, nRow, nCol ):

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		pPlot = CyInterface().getSelectionPlot()

		for i in range(gc.getNumPromotionInfos()):
			szName = "PromotionButton" + str(i)
			screen.moveToFront( szName )

		screen.hide( "PlotListMinus" )
		screen.hide( "PlotListPlus" )

		for j in range(gc.getMAX_PLOT_LIST_ROWS()):
			#szStringPanel = "PlotListPanel" + str(j)
			#screen.hide(szStringPanel)

			for i in range(self.numPlotListButtons()):
				szString = "PlotListButton" + str(j*self.numPlotListButtons()+i)
				screen.hide( szString )

				szStringHealth = szString + "Health"
				screen.hide( szStringHealth )

				szStringIcon = szString + "Icon"
				screen.hide( szStringIcon )

				szStringIcon = szString + "Armor"
				screen.hide( szStringIcon )

				szStringIcon = szString + "Weapon"
				screen.hide( szStringIcon )

		if ( pPlot and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyEngine().isGlobeviewUp() == False):

			iVisibleUnits = CyInterface().getNumVisibleUnits()
			iCount = -(CyInterface().getPlotListColumn())

			bLeftArrow = False
			bRightArrow = False

			if (CyInterface().isCityScreenUp()):
				iMaxRows = 1
				iSkipped = (gc.getMAX_PLOT_LIST_ROWS() - 1) * self.numPlotListButtons()
				iCount += iSkipped
			else:
				iMaxRows = gc.getMAX_PLOT_LIST_ROWS()
				iCount += CyInterface().getPlotListOffset()
				iSkipped = 0

			CyInterface().cacheInterfacePlotUnits(pPlot)
			for i in range(CyInterface().getNumCachedInterfacePlotUnits()):
				pLoopUnit = CyInterface().getCachedInterfacePlotUnit(i)
				if (pLoopUnit):

					if ((iCount == 0) and (CyInterface().getPlotListColumn() > 0)):
						bLeftArrow = True
					elif ((iCount == (gc.getMAX_PLOT_LIST_ROWS() * self.numPlotListButtons() - 1)) and ((iVisibleUnits - iCount - CyInterface().getPlotListColumn() + iSkipped) > 1)):
						bRightArrow = True

					if ((iCount >= 0) and (iCount <  self.numPlotListButtons() * gc.getMAX_PLOT_LIST_ROWS())):
						if ((pLoopUnit.getTeam() != gc.getGame().getActiveTeam()) or pLoopUnit.isWaiting()):
							szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_FORTIFY").getPath()

						elif (pLoopUnit.canMove()):
							if (pLoopUnit.hasMoved()):
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_HASMOVED").getPath()
							else:
								szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_MOVE").getPath()
						else:
							szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_NOMOVE").getPath()

						szString = "PlotListButton" + str(iCount)
						screen.changeImageButton( szString, gc.getUnitInfo(pLoopUnit.getUnitType()).getButton() )

						if ( pLoopUnit.getOwner() == gc.getGame().getActivePlayer() ):
							bEnable = True
						else:
							bEnable = False
						screen.enable(szString, bEnable)

						if (pLoopUnit.IsSelected()):
							screen.setState(szString, True)
						else:
							screen.setState(szString, False)
						screen.show( szString )

						# place the health bar
						if (pLoopUnit.isFighting()):
							bShowHealth = False
						elif (pLoopUnit.getDomainType() == DomainTypes.DOMAIN_AIR):
							bShowHealth = pLoopUnit.canAirAttack()
						else:
							bShowHealth = pLoopUnit.canFight()

						if bShowHealth:
							szStringHealth = szString + "Health"
							screen.setBarPercentage( szStringHealth, InfoBarTypes.INFOBAR_STORED, float( pLoopUnit.currHitPoints() ) / float( pLoopUnit.maxHitPoints() ) )
							if (pLoopUnit.getDamage() >= ((pLoopUnit.maxHitPoints() * 2) / 3)):
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_RED"))
							elif (pLoopUnit.getDamage() >= (pLoopUnit.maxHitPoints() / 3)):
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_YELLOW"))
							else:
								screen.setStackedBarColors(szStringHealth, InfoBarTypes.INFOBAR_STORED, gc.getInfoTypeForString("COLOR_GREEN"))
							screen.show( szStringHealth )

						# Adds the overlay first
						szStringIcon = szString + "Icon"
						screen.changeDDSGFC( szStringIcon, szFileName )
						screen.show( szStringIcon )

						if bEnable:
							x = 315 + ((iCount % self.numPlotListButtons()) * 34)
							y = yResolution - 169 + (iCount / self.numPlotListButtons() - gc.getMAX_PLOT_LIST_ROWS() + 1) * 34

							self.displayUnitPlotList_Dot( screen, pLoopUnit, szString, iCount, x, y + 4 )
							self.displayUnitPlotList_Promo( screen, pLoopUnit, szString )
							self.displayUnitPlotList_Upgrade( screen, pLoopUnit, szString, iCount, x, y )
							self.displayUnitPlotList_Mission( screen, pLoopUnit, szString, iCount, x, y - 22, 12)
							self.displayUnitPlotList_Weapon( screen, pLoopUnit, szString, iCount, x, y + 4 )
							self.displayUnitPlotList_Armor( screen, pLoopUnit, szString, iCount, x, y + 4 )

					iCount = iCount + 1

			if (iVisibleUnits > self.numPlotListButtons() * iMaxRows):
				screen.enable("PlotListMinus", bLeftArrow)
				screen.show( "PlotListMinus" )

				screen.enable("PlotListPlus", bRightArrow)
				screen.show( "PlotListPlus" )

		return 0


	# This will update the flag widget for SP hotseat and dbeugging
	def updateFlag( self ):

		if ( CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START ):
			screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
			xResolution = screen.getXResolution()
			yResolution = screen.getYResolution()

#FfH: Modified by Kael 07/17/2008
			screen.addFlagWidgetGFC( "CivilizationFlag", xResolution - 288, yResolution - 138, 68, 250, gc.getGame().getActivePlayer(), WidgetTypes.WIDGET_FLAG, gc.getGame().getActivePlayer(), -1)
#			screen.addFlagWidgetGFC( "CivilizationFlag", 0, -20, 68, 250, gc.getGame().getActivePlayer(), WidgetTypes.WIDGET_FLAG, gc.getGame().getActivePlayer(), -1)
#FfH: End Modify

	# Will hide and show the selection buttons and their associated buttons
	def updateSelectionButtons( self ):

		global SELECTION_BUTTON_COLUMNS
		global MAX_SELECTION_BUTTONS
		global g_pSelectedUnit

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		pHeadSelectedUnit = CyInterface().getHeadSelectedUnit()

		global g_NumEmphasizeInfos
		global g_NumCityTabTypes
		global g_NumHurryInfos
		global g_NumUnitClassInfos
		global g_NumBuildingClassInfos
		global g_NumProjectInfos
		global g_NumProcessInfos
		global g_NumActionInfos

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

#FfH: Modified by Kael 07/18/2008
#		screen.addMultiListControlGFC( "BottomButtonContainer", u"", iMultiListXL, yResolution - 113, xResolution - (iMultiListXL+iMultiListXR), 100, 4, 48, 48, TableStyles.TABLE_STYLE_STANDARD )
#		screen.clearMultiList( "BottomButtonContainer" )
#		screen.hide( "BottomButtonContainer" )
		if (not CyEngine().isGlobeviewUp() and pHeadSelectedCity):
			screen.addMultiListControlGFC( "BottomButtonContainer", u"", iMultiListXL, yResolution - 113, xResolution - (iMultiListXL+iMultiListXR) - 90, 100, 4, 48, 48, TableStyles.TABLE_STYLE_STANDARD )
		else:
			screen.addMultiListControlGFC( "BottomButtonContainer", u"", iMultiListXL, yResolution - 113, xResolution - (iMultiListXL+iMultiListXR), 100, 4, 48, 48, TableStyles.TABLE_STYLE_STANDARD )
		screen.clearMultiList( "BottomButtonContainer" )
		screen.hide( "BottomButtonContainer" )
#FfH: End Modify

		# All of the hides...
		self.setMinimapButtonVisibility(False)

		screen.hideList( 0 )

		for i in range (g_NumEmphasizeInfos):
			szButtonID = "Emphasize" + str(i)
			screen.hide( szButtonID )

		# Hurry button show...
		for i in range( g_NumHurryInfos ):
			szButtonID = "Hurry" + str(i)
			screen.hide( szButtonID )

		# Conscript Button Show
		screen.hide( "Conscript" )
		#screen.hide( "Liberate" )
		screen.hide( "AutomateProduction" )
		screen.hide( "AutomateCitizens" )

		if (not CyEngine().isGlobeviewUp() and pHeadSelectedCity):

			self.setMinimapButtonVisibility(True)

			if ((pHeadSelectedCity.getOwner() == gc.getGame().getActivePlayer()) or gc.getGame().isDebugMode()):

				iBtnX = xResolution - 284
				iBtnY = yResolution - 177
				iBtnW = 64
				iBtnH = 30

				# Liberate button
				#szText = "<font=1>" + localText.getText("TXT_KEY_LIBERATE_CITY", ()) + "</font>"
				#screen.setButtonGFC( "Liberate", szText, "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_LIBERATE_CITY, -1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
				#screen.setStyle( "Liberate", "Button_CityT1_Style" )
				#screen.hide( "Liberate" )

#FfH: Modified by Kael 07/18/2008
#				iBtnSX = xResolution - 284
				iBtnSX = xResolution - 300
#FfH: End Modify

				iBtnX = iBtnSX
				iBtnY = yResolution - 140
				iBtnW = 64
				iBtnH = 30

				# Conscript button
				szText = "<font=1>" + localText.getText("TXT_KEY_DRAFT", ()) + "</font>"
				screen.setButtonGFC( "Conscript", szText, "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_CONSCRIPT, -1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
				screen.setStyle( "Conscript", "Button_CityT1_Style" )
				screen.hide( "Conscript" )

				iBtnY += iBtnH
				iBtnW = 32
				iBtnH = 28

				# Hurry Buttons
				screen.setButtonGFC( "Hurry0", "", "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_HURRY, 0, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
				screen.setStyle( "Hurry0", "Button_CityC1_Style" )
				screen.hide( "Hurry0" )

				iBtnX += iBtnW

				screen.setButtonGFC( "Hurry1", "", "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_HURRY, 1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
				screen.setStyle( "Hurry1", "Button_CityC2_Style" )
				screen.hide( "Hurry1" )

				screen.setButtonGFC( "Hurry2", "", "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_HURRY, 2, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
				screen.setStyle( "Hurry2", "Button_CityC2_Style" )
				screen.hide( "Hurry2" )

				iBtnX = iBtnSX
				iBtnY += iBtnH

				# Automate Production Button
				screen.addCheckBoxGFC( "AutomateProduction", "", "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_AUTOMATE_PRODUCTION, -1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
				screen.setStyle( "AutomateProduction", "Button_CityC3_Style" )

				iBtnX += iBtnW

				# Automate Citizens Button
				screen.addCheckBoxGFC( "AutomateCitizens", "", "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_AUTOMATE_CITIZENS, -1, -1, ButtonStyles.BUTTON_STYLE_STANDARD )
				screen.setStyle( "AutomateCitizens", "Button_CityC4_Style" )

				iBtnY += iBtnH
				iBtnX = iBtnSX

				iBtnW	= 22
				iBtnWa	= 20
				iBtnH	= 24
				iBtnHa	= 27

				# Set Emphasize buttons
				i = 0
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", iBtnX, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i+1) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )

				i+=1
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", iBtnX+iBtnW, iBtnY, iBtnWa, iBtnH, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i+1) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )

				i+=1
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", iBtnX+iBtnW+iBtnWa, iBtnY, iBtnW, iBtnH, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i+1) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )

				iBtnY += iBtnH

				i+=1
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", iBtnX, iBtnY, iBtnW, iBtnHa, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i+1) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )

				i+=1
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", iBtnX+iBtnW, iBtnY, iBtnWa, iBtnHa, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i+1) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )

				i+=1
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", iBtnX+iBtnW+iBtnWa, iBtnY, iBtnW, iBtnHa, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i+1) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )

# FlavourMod: Added by Jean Elcard 09/03/2009 (growth control buttons)
				i+=1
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", xResolution - HUD_City_Growth_Panel_Width_Exclusion - iBtnWa, iCityCenterRow2Y, iBtnWa, iBtnHa, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )

				i+=1
				szButtonID = "Emphasize" + str(i)
				screen.addCheckBoxGFC( szButtonID, "", "", xResolution - HUD_City_Growth_Panel_Width_Exclusion - iBtnWa, iCityCenterRow1Y, iBtnWa, iBtnHa, WidgetTypes.WIDGET_EMPHASIZE, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
				szStyle = "Button_CityB" + str(i-1) + "_Style"
				screen.setStyle( szButtonID, szStyle )
				screen.hide( szButtonID )
# FlavourMod: End Add

				g_pSelectedUnit = 0
				screen.setState( "AutomateCitizens", pHeadSelectedCity.isCitizensAutomated() )
				screen.setState( "AutomateProduction", pHeadSelectedCity.isProductionAutomated() )

# FlavourMod: Added by Jean Elcard 09/03/2009 (growth control buttons)
				iNumCustomEmphasizeInfos = 2
# FlavourMod: End Add

				for i in range (g_NumEmphasizeInfos):
# FlavourMod: Added by Jean Elcard 09/03/2009 (growth control buttons)
# -> Don't show the buttons, if complete city screen isn't up.
					if (i < g_NumEmphasizeInfos - iNumCustomEmphasizeInfos) or CyInterface().isCityScreenUp():
# FlavourMod: End Add (block below indented)
						szButtonID = "Emphasize" + str(i)
						screen.show( szButtonID )
						if ( pHeadSelectedCity.AI_isEmphasize(i) ):
							screen.setState( szButtonID, True )
						else:
							screen.setState( szButtonID, False )

				# City Tabs
				for i in range( g_NumCityTabTypes ):
					szButtonID = "CityTab" + str(i)
					screen.show( szButtonID )

				# Hurry button show...
				for i in range( g_NumHurryInfos ):
					szButtonID = "Hurry" + str(i)
					if pHeadSelectedCity.getProductionUnit()!=-1 or i!=1:
						if pHeadSelectedCity.getProductionUnit()==-1 or i!=2:
							screen.show( szButtonID )
							screen.enable( szButtonID, pHeadSelectedCity.canHurry(i, False) )

				# Conscript Button Show
				screen.show( "Conscript" )
				if (pHeadSelectedCity.canConscript()):
					screen.enable( "Conscript", True )
				else:
					screen.enable( "Conscript", False )

				# Liberate Button Show
				#screen.show( "Liberate" )
				#if (-1 != pHeadSelectedCity.getLiberationPlayer()):
				#	screen.enable( "Liberate", True )
				#else:
				#	screen.enable( "Liberate", False )

				iCount = 0
				iRow = 0
				bFound = False

				# Units to construct
				for i in range ( g_NumUnitClassInfos ):
					eLoopUnit = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getCivilizationUnits(i)

#FfH: Added by Kael 10/05/2007
					if eLoopUnit != -1:
#FfH: End Add

						if (pHeadSelectedCity.canTrain(eLoopUnit, False, True)):
							szButton = gc.getPlayer(pHeadSelectedCity.getOwner()).getUnitButton(eLoopUnit)

#FfH: Added by Kael 02/06/2009
#							iProm = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getDefaultRace()
#							if iProm != -1:
#								szButton = gc.getUnitInfo(eLoopUnit).getUnitStyleButton(iProm)
#FfH: End Add

							screen.appendMultiListButton( "BottomButtonContainer", szButton, iRow, WidgetTypes.WIDGET_TRAIN, i, -1, False )
							screen.show( "BottomButtonContainer" )

							if ( not pHeadSelectedCity.canTrain(eLoopUnit, False, False) ):
								screen.disableMultiListButton( "BottomButtonContainer", iRow, iCount, szButton)

							iCount = iCount + 1
							bFound = True

				iCount = 0
				if (bFound):
					iRow = iRow + 1
				bFound = False

				# Buildings to construct
				for i in range ( g_NumBuildingClassInfos ):
					if (not isLimitedWonderClass(i)):
						eLoopBuilding = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getCivilizationBuildings(i)

						if (pHeadSelectedCity.canConstruct(eLoopBuilding, False, True, False, False)):
							screen.appendMultiListButton( "BottomButtonContainer", gc.getBuildingInfo(eLoopBuilding).getButton(), iRow, WidgetTypes.WIDGET_CONSTRUCT, i, -1, False )
							screen.show( "BottomButtonContainer" )

							if ( not pHeadSelectedCity.canConstruct(eLoopBuilding, False, False, False, False) ):
								screen.disableMultiListButton( "BottomButtonContainer", iRow, iCount, gc.getBuildingInfo(eLoopBuilding).getButton() )

							iCount = iCount + 1
							bFound = True

				iCount = 0
				if (bFound):
					iRow = iRow + 1
				bFound = False

				# Wonders to construct
				i = 0
				for i in range( g_NumBuildingClassInfos ):
					if (isLimitedWonderClass(i)):
						eLoopBuilding = gc.getCivilizationInfo(pHeadSelectedCity.getCivilizationType()).getCivilizationBuildings(i)

						if (pHeadSelectedCity.canConstruct(eLoopBuilding, False, True, False, False)):
							screen.appendMultiListButton( "BottomButtonContainer", gc.getBuildingInfo(eLoopBuilding).getButton(), iRow, WidgetTypes.WIDGET_CONSTRUCT, i, -1, False )
							screen.show( "BottomButtonContainer" )

							if ( not pHeadSelectedCity.canConstruct(eLoopBuilding, False, False, False, False) ):
								screen.disableMultiListButton( "BottomButtonContainer", iRow, iCount, gc.getBuildingInfo(eLoopBuilding).getButton() )

							iCount = iCount + 1
							bFound = True

				iCount = 0
				if (bFound):
					iRow = iRow + 1
				bFound = False

				# Projects
				i = 0
				for i in range( g_NumProjectInfos ):
					if (pHeadSelectedCity.canCreate(i, False, True)):
						screen.appendMultiListButton( "BottomButtonContainer", gc.getProjectInfo(i).getButton(), iRow, WidgetTypes.WIDGET_CREATE, i, -1, False )
						screen.show( "BottomButtonContainer" )

						if ( not pHeadSelectedCity.canCreate(i, False, False) ):
							screen.disableMultiListButton( "BottomButtonContainer", iRow, iCount, gc.getProjectInfo(i).getButton() )

						iCount = iCount + 1
						bFound = True

				# Processes
				i = 0
				for i in range( g_NumProcessInfos ):
					if (pHeadSelectedCity.canMaintain(i, False)):
						screen.appendMultiListButton( "BottomButtonContainer", gc.getProcessInfo(i).getButton(), iRow, WidgetTypes.WIDGET_MAINTAIN, i, -1, False )
						screen.show( "BottomButtonContainer" )

						iCount = iCount + 1
						bFound = True

				screen.selectMultiList( "BottomButtonContainer", CyInterface().getCityTabSelectionRow() )

		elif (not CyEngine().isGlobeviewUp() and pHeadSelectedUnit and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY):

			self.setMinimapButtonVisibility(True)

			if (CyInterface().getInterfaceMode() == InterfaceModeTypes.INTERFACEMODE_SELECTION):

				if ( pHeadSelectedUnit.getOwner() == gc.getGame().getActivePlayer() and g_pSelectedUnit != pHeadSelectedUnit ):

					g_pSelectedUnit = pHeadSelectedUnit

					iCount = 0

					actions = CyInterface().getActionsToShow()
					for i in actions:

#FfH: Modified by Kael 02/07/2009
#						screen.appendMultiListButton( "BottomButtonContainer", gc.getActionInfo(i).getButton(), 0, WidgetTypes.WIDGET_ACTION, i, -1, False )

						szButton = gc.getActionInfo(i).getButton()
						if gc.getActionInfo(i).getCommandType() == CommandTypes.COMMAND_UPGRADE:
							szButton = gc.getPlayer(pHeadSelectedUnit.getOwner()).getUnitButton(gc.getActionInfo(i).getCommandData())	#Added Sephi
#							iProm = gc.getCivilizationInfo(gc.getPlayer(pHeadSelectedUnit.getOwner()).getCivilizationType()).getDefaultRace()
#							if iProm != -1:
#								szButton = gc.getUnitInfo(gc.getActionInfo(i).getCommandData()).getUnitStyleButton(iProm)
						screen.appendMultiListButton( "BottomButtonContainer", szButton, 0, WidgetTypes.WIDGET_ACTION, i, -1, False )
#FfH: End Modify

						screen.show( "BottomButtonContainer" )

						if ( not CyInterface().canHandleAction(i, False) ):
							screen.disableMultiListButton( "BottomButtonContainer", 0, iCount, gc.getActionInfo(i).getButton() )

						if ( pHeadSelectedUnit.isActionRecommended(i) ):#or gc.getActionInfo(i).getCommandType() == CommandTypes.COMMAND_PROMOTION ):
							screen.enableMultiListPulse( "BottomButtonContainer", True, 0, iCount )
						else:
							screen.enableMultiListPulse( "BottomButtonContainer", False, 0, iCount )

						iCount = iCount + 1

					if (CyInterface().canCreateGroup()):
						screen.appendMultiListButton( "BottomButtonContainer", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_CREATEGROUP").getPath(), 0, WidgetTypes.WIDGET_CREATE_GROUP, -1, -1, False )
						screen.show( "BottomButtonContainer" )

						iCount = iCount + 1

					if (CyInterface().canDeleteGroup()):
						screen.appendMultiListButton( "BottomButtonContainer", ArtFileMgr.getInterfaceArtInfo("INTERFACE_BUTTONS_SPLITGROUP").getPath(), 0, WidgetTypes.WIDGET_DELETE_GROUP, -1, -1, False )
						screen.show( "BottomButtonContainer" )

						iCount = iCount + 1

		elif (CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY):

			self.setMinimapButtonVisibility(True)

		return 0

	# Will update the research buttons
	def updateResearchButtons( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		for i in range( gc.getNumTechInfos() ):
			szName = "ResearchButton" + str(i)
			screen.hide( szName )

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		#screen.hide( "InterfaceOrnamentLeftLow" )
		#screen.hide( "InterfaceOrnamentRightLow" )

		for i in range(gc.getNumReligionInfos()):
			szName = "ReligionButton" + str(i)
			screen.hide( szName )

		i = 0
		if ( CyInterface().shouldShowResearchButtons() and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
			iCount = 0

			for i in range( gc.getNumTechInfos() ):
				if (gc.getActivePlayer().canResearch(i, False)):

#FfH: Modified by Karl 08/24/2007
#					if (iCount < 20):
					if (iCount < 30):
#FfH: End Modify

						szName = "ResearchButton" + str(i)

						bDone = False
						for j in range( gc.getNumReligionInfos() ):
							if ( not bDone ):
								if (gc.getReligionInfo(j).getTechPrereq() == i):
									if not (gc.getGame().isReligionSlotTaken(j)):
										szName = "ReligionButton" + str(j)
										bDone = True

						screen.show( szName )
						self.setResearchButtonPosition(szName, iCount)

					iCount = iCount + 1

		return 0

# BUG - city specialist - start
	def updateCitizenButtons_hide( self ):

		global MAX_CITIZEN_BUTTONS

		bHandled = False

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		for i in range( MAX_CITIZEN_BUTTONS ):
			szName = "FreeSpecialist" + str(i)
			screen.hide( szName )
			szName = "AngryCitizen" + str(i)
			screen.hide( szName )
			szName = "AngryCitizenChevron" + str(i)
			screen.hide( szName )

		for i in range( gc.getNumSpecialistInfos() ):
			szName = "IncreaseSpecialist" + str(i)
			screen.hide( szName )
			szName = "DecreaseSpecialist" + str(i)
			screen.hide( szName )
			szName = "CitizenDisabledButton" + str(i)
			screen.hide( szName )
			for j in range(MAX_CITIZEN_BUTTONS):
				szName = "CitizenButton" + str((i * 100) + j)
				screen.hide( szName )
				szName = "CitizenButtonHighlight" + str((i * 100) + j)
				screen.hide( szName )
				szName = "CitizenChevron" + str((i * 100) + j)
				screen.hide( szName )

				szName = "IncresseCitizenButton" + str((i * 100) + j)
				screen.hide( szName )
				szName = "IncresseCitizenBanner" + str((i * 100) + j)
				screen.hide( szName )
				szName = "DecresseCitizenButton" + str((i * 100) + j)
				screen.hide( szName )
				szName = "CitizenButtonHighlight" + str((i * 100) + j)
				screen.hide( szName )

		global g_iSuperSpecialistCount
		global g_iCitySpecialistCount
		global g_iAngryCitizensCount

		screen.hide( "SpecialistBackground" )
		screen.hide( "SpecialistLabel" )

		for i in range( g_iSuperSpecialistCount ):
			szName = "FreeSpecialist" + str(i)
			screen.hide( szName )
		for i in range( g_iAngryCitizensCount ):
			szName = "AngryCitizen" + str(i)
			screen.hide( szName )

		for i in range( gc.getNumSpecialistInfos() ):
			for k in range( g_iCitySpecialistCount ):
				szName = "IncresseCitizenBanner" + str((i * 100) + k)
				screen.hide( szName )
				szName = "IncresseCitizenButton" + str((i * 100) + k)
				screen.hide( szName )
				szName = "DecresseCitizenButton" + str((i * 100) + k)
				screen.hide( szName )

		return 0
# BUG - city specialist - end

	# Will update the citizen buttons
	def updateCitizenButtons( self ):

		if not CyInterface().isCityScreenUp(): return 0

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		if not (pHeadSelectedCity and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW): return 0

		global MAX_CITIZEN_BUTTONS

		bHandled = False
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		if ( pHeadSelectedCity.angryPopulation(0) < MAX_CITIZEN_BUTTONS ):
			iCount = pHeadSelectedCity.angryPopulation(0)
		else:
			iCount = MAX_CITIZEN_BUTTONS

		for i in range(iCount):
			bHandled = True
			szName = "AngryCitizen" + str(i)
			screen.show( szName )

		iFreeSpecialistCount = 0
		for i in range(gc.getNumSpecialistInfos()):
			iFreeSpecialistCount += pHeadSelectedCity.getFreeSpecialistCount(i)

		iCount = 0

		bHandled = False

		if (iFreeSpecialistCount > MAX_CITIZEN_BUTTONS):
			for i in range(gc.getNumSpecialistInfos()):
				if (pHeadSelectedCity.getFreeSpecialistCount(i) > 0):
					if (iCount < MAX_CITIZEN_BUTTONS):
						szName = "FreeSpecialist" + str(iCount)
						screen.setImageButton( szName, gc.getSpecialistInfo(i).getTexture(), (xResolution - 74  - (26 * iCount)), yResolution - 214, 24, 24, WidgetTypes.WIDGET_FREE_CITIZEN, i, 1 )
						screen.show( szName )
						bHandled = true
					iCount += 1

		else:
			for i in range(gc.getNumSpecialistInfos()):
				for j in range( pHeadSelectedCity.getFreeSpecialistCount(i) ):
					if (iCount < MAX_CITIZEN_BUTTONS):
						szName = "FreeSpecialist" + str(iCount)
						screen.setImageButton( szName, gc.getSpecialistInfo(i).getTexture(), (xResolution - 74  - (26 * iCount)), yResolution - 214, 24, 24, WidgetTypes.WIDGET_FREE_CITIZEN, i, -1 )
						screen.show( szName )
						bHandled = true

					iCount = iCount + 1

		for i in range( gc.getNumSpecialistInfos() ):

			bHandled = False

			if (pHeadSelectedCity.getOwner() == gc.getGame().getActivePlayer() or gc.getGame().isDebugMode()):

				if (pHeadSelectedCity.isCitizensAutomated()):
					iSpecialistCount = max(pHeadSelectedCity.getSpecialistCount(i), pHeadSelectedCity.getForceSpecialistCount(i))
				else:
					iSpecialistCount = pHeadSelectedCity.getSpecialistCount(i)

				if (pHeadSelectedCity.isSpecialistValid(i, 1) and (pHeadSelectedCity.isCitizensAutomated() or iSpecialistCount < (pHeadSelectedCity.getPopulation() + pHeadSelectedCity.totalFreeSpecialists()))):
					szName = "IncreaseSpecialist" + str(i)
					screen.show( szName )
					szName = "CitizenDisabledButton" + str(i)
					screen.show( szName )

				if iSpecialistCount > 0:
					szName = "CitizenDisabledButton" + str(i)
					screen.hide( szName )
					szName = "DecreaseSpecialist" + str(i)
					screen.show( szName )

			if (pHeadSelectedCity.getSpecialistCount(i) < MAX_CITIZEN_BUTTONS):
				iCount = pHeadSelectedCity.getSpecialistCount(i)
			else:
				iCount = MAX_CITIZEN_BUTTONS

			j = 0
			for j in range( iCount ):
				bHandled = True
				szName = "CitizenButton" + str((i * 100) + j)
				screen.addCheckBoxGFC( szName, gc.getSpecialistInfo(i).getTexture(), "", xResolution - 74 - (26 * j), (yResolution - 272 - (26 * i)), 24, 24, WidgetTypes.WIDGET_CITIZEN, i, j, ButtonStyles.BUTTON_STYLE_LABEL )
				screen.show( szName )
				szName = "CitizenButtonHighlight" + str((i * 100) + j)
				screen.addDDSGFC( szName, ArtFileMgr.getInterfaceArtInfo("BUTTON_HILITE_SQUARE").getPath(), xResolution - 74 - (26 * j), (yResolution - 272 - (26 * i)), 24, 24, WidgetTypes.WIDGET_CITIZEN, i, j )
				if ( pHeadSelectedCity.getForceSpecialistCount(i) > j ):
					screen.show( szName )
				else:
					screen.hide( szName )

			if ( not bHandled ):
				szName = "CitizenDisabledButton" + str(i)
				screen.show( szName )

		return 0

# BUG - city specialist - start
	def updateCitizenButtons_Stacker( self ):

		if not CyInterface().isCityScreenUp(): return 0

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		if not (pHeadSelectedCity and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW): return 0

		global g_iSuperSpecialistCount
		global g_iCitySpecialistCount
		global g_iAngryCitizensCount

		bHandled = False

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		currentAngryCitizenCount = pHeadSelectedCity.angryPopulation(0)

		if(currentAngryCitizenCount > 0):
			stackWidth = 220 / currentAngryCitizenCount
			if (stackWidth > MAX_SPECIALIST_BUTTON_SPACING):
				stackWidth = MAX_SPECIALIST_BUTTON_SPACING

		for i in range(currentAngryCitizenCount):
			bHandled = True
			szName = "AngryCitizen" + str(i)
			screen.setImageButton( szName, ArtFileMgr.getInterfaceArtInfo("INTERFACE_ANGRYCITIZEN_TEXTURE").getPath(), xResolution - SPECIALIST_AREA_MARGIN - (stackWidth * i), yResolution - (282- SPECIALIST_ROW_HEIGHT), 30, 30, WidgetTypes.WIDGET_ANGRY_CITIZEN, -1, -1 )
			screen.show( szName )

		# update the max ever citizen counts
		if g_iAngryCitizensCount < currentAngryCitizenCount:
			g_iAngryCitizensCount = currentAngryCitizenCount

		iCount = 0
		bHandled = False
		currentSuperSpecialistCount = 0

		for i in range(gc.getNumSpecialistInfos()):
			if(pHeadSelectedCity.getFreeSpecialistCount(i) > 0):
				currentSuperSpecialistCount = currentSuperSpecialistCount + pHeadSelectedCity.getFreeSpecialistCount(i)

		if(currentSuperSpecialistCount > 0):
			stackWidth = 220 / currentSuperSpecialistCount
			if (stackWidth > MAX_SPECIALIST_BUTTON_SPACING):
				stackWidth = MAX_SPECIALIST_BUTTON_SPACING

		for i in range(gc.getNumSpecialistInfos()):
			for j in range( pHeadSelectedCity.getFreeSpecialistCount(i) ):

				szName = "FreeSpecialist" + str(iCount)
				screen.setImageButton( szName, gc.getSpecialistInfo(i).getTexture(), (xResolution - SPECIALIST_AREA_MARGIN  - (stackWidth * iCount)), yResolution - (282 - SPECIALIST_ROW_HEIGHT * 2), 30, 30, WidgetTypes.WIDGET_FREE_CITIZEN, i, 1 )
				screen.show( szName )
				bHandled = true

				iCount = iCount + 1

		# update the max ever citizen counts
		if g_iSuperSpecialistCount < iCount:
			g_iSuperSpecialistCount = iCount

		iXShiftVal = 0
		iYShiftVal = 0
		iSpecialistCount = 0

		for i in range( gc.getNumSpecialistInfos() ):

			bHandled = False
			if( iSpecialistCount > SPECIALIST_ROWS ):
				iXShiftVal = 115
				iYShiftVal = (iSpecialistCount % SPECIALIST_ROWS) + 1
			else:
				iYShiftVal = iSpecialistCount

			if (gc.getSpecialistInfo(i).isVisible()):
				iSpecialistCount = iSpecialistCount + 1

			if (gc.getPlayer(pHeadSelectedCity.getOwner()).isSpecialistValid(i) or i == 0):
				iCount = (pHeadSelectedCity.getPopulation() - pHeadSelectedCity.angryPopulation(0)) +  pHeadSelectedCity.totalFreeSpecialists()
			else:
				iCount = pHeadSelectedCity.getMaxSpecialistCount(i)

			# update the max ever citizen counts
			if g_iCitySpecialistCount < iCount:
				g_iCitySpecialistCount = iCount

			RowLength = 110
			if (i == 0):
			#if (i == gc.getInfoTypeForString(gc.getDefineSTRING("DEFAULT_SPECIALIST"))):
				RowLength *= 2

			HorizontalSpacing = MAX_SPECIALIST_BUTTON_SPACING
			if (iCount > 0):
				HorizontalSpacing = RowLength / iCount
			if (HorizontalSpacing > MAX_SPECIALIST_BUTTON_SPACING):
				HorizontalSpacing = MAX_SPECIALIST_BUTTON_SPACING

			for k in range (iCount):
				if (k  >= pHeadSelectedCity.getSpecialistCount(i)):
					szName = "IncresseCitizenBanner" + str((i * 100) + k)
					screen.addCheckBoxGFC( szName, gc.getSpecialistInfo(i).getTexture(), "", xResolution - (SPECIALIST_AREA_MARGIN + iXShiftVal) - (HorizontalSpacing * k), (yResolution - 282 - (SPECIALIST_ROW_HEIGHT * iYShiftVal)), 30, 30, WidgetTypes.WIDGET_CHANGE_SPECIALIST, i, 1, ButtonStyles.BUTTON_STYLE_LABEL )
					screen.enable( szName, False )
					screen.show( szName )

					szName = "IncresseCitizenButton" + str((i * 100) + k)
					screen.addCheckBoxGFC( szName, "", "", xResolution - (SPECIALIST_AREA_MARGIN + iXShiftVal) - (HorizontalSpacing * k), (yResolution - 282 - (SPECIALIST_ROW_HEIGHT * iYShiftVal)), 30, 30, WidgetTypes.WIDGET_CHANGE_SPECIALIST, i, 1, ButtonStyles.BUTTON_STYLE_LABEL )
					screen.show( szName )

				else:
					szName = "DecresseCitizenButton" + str((i * 100) + k)
					screen.addCheckBoxGFC( szName, gc.getSpecialistInfo(i).getTexture(), "", xResolution - (SPECIALIST_AREA_MARGIN + iXShiftVal) - (HorizontalSpacing * k), (yResolution - 282 - (SPECIALIST_ROW_HEIGHT * iYShiftVal)), 30, 30, WidgetTypes.WIDGET_CHANGE_SPECIALIST, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
					screen.show( szName )

		screen.show( "SpecialistBackground" )
		screen.show( "SpecialistLabel" )

		return 0
# BUG - city specialist - end

# BUG - city specialist - start
	def updateCitizenButtons_Chevron( self ):

		if not CyInterface().isCityScreenUp(): return 0

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		if not (pHeadSelectedCity and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW): return 0

		global MAX_CITIZEN_BUTTONS

		bHandled = False

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()


		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		iCount = pHeadSelectedCity.angryPopulation(0)

		j = 0
		while (iCount > 0):
			bHandled = True
			szName = "AngryCitizen" + str(j)
			screen.show( szName )

			xCoord = xResolution - 74 - (26 * j)
			yCoord = yResolution - 238

			szName = "AngryCitizenChevron" + str(j)
			if iCount >= 20:
				szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_CHEVRON20").getPath()
				iCount -= 20
			elif iCount >= 10:
				szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_CHEVRON10").getPath()
				iCount -= 10
			elif iCount >= 5:
				szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_CHEVRON5").getPath()
				iCount -= 5
			else:
				szFileName = ""
				iCount -= 1

			if (szFileName != ""):
				screen.addDDSGFC( szName , szFileName, xCoord, yCoord, 10, 10, WidgetTypes.WIDGET_CITIZEN, j, False )
				screen.show( szName )

			j += 1

		iFreeSpecialistCount = 0
		for i in range(gc.getNumSpecialistInfos()):
			iFreeSpecialistCount += pHeadSelectedCity.getFreeSpecialistCount(i)

		iCount = 0

		bHandled = False

		if (iFreeSpecialistCount > MAX_CITIZEN_BUTTONS):
			for i in range(gc.getNumSpecialistInfos()):
				if (pHeadSelectedCity.getFreeSpecialistCount(i) > 0):
					if (iCount < MAX_CITIZEN_BUTTONS):
						szName = "FreeSpecialist" + str(iCount)
						screen.setImageButton( szName, gc.getSpecialistInfo(i).getTexture(), (xResolution - 74  - (26 * iCount)), yResolution - 214, 24, 24, WidgetTypes.WIDGET_FREE_CITIZEN, i, 1 )
						screen.show( szName )
						bHandled = True
					iCount += 1

		else:
			for i in range(gc.getNumSpecialistInfos()):
				for j in range( pHeadSelectedCity.getFreeSpecialistCount(i) ):
					if (iCount < MAX_CITIZEN_BUTTONS):
						szName = "FreeSpecialist" + str(iCount)
						screen.setImageButton( szName, gc.getSpecialistInfo(i).getTexture(), (xResolution - 74  - (26 * iCount)), yResolution - 214, 24, 24, WidgetTypes.WIDGET_FREE_CITIZEN, i, -1 )
						screen.show( szName )
						bHandled = True

					iCount = iCount + 1

		for i in range(gc.getNumSpecialistInfos()):
			bHandled = False

			if (pHeadSelectedCity.getOwner() == gc.getGame().getActivePlayer() or gc.getGame().isDebugMode()):

				if (pHeadSelectedCity.isCitizensAutomated()):
					iSpecialistCount = max(pHeadSelectedCity.getSpecialistCount(i), pHeadSelectedCity.getForceSpecialistCount(i))
				else:
					iSpecialistCount = pHeadSelectedCity.getSpecialistCount(i)

				if (pHeadSelectedCity.isSpecialistValid(i, 1) and (pHeadSelectedCity.isCitizensAutomated() or iSpecialistCount < (pHeadSelectedCity.getPopulation() + pHeadSelectedCity.totalFreeSpecialists()))):
					CvUtil.pyPrint('updateCitizenButtons_Chevron increase %d specialist=%s'%(i,gc.getSpecialistInfo(i).getDescription()))
					szName = "IncreaseSpecialist" + str(i)
					screen.show( szName )
					szName = "CitizenDisabledButton" + str(i)
					screen.show( szName )

				if iSpecialistCount > 0:
					CvUtil.pyPrint('updateCitizenButtons_Chevron decrease %d specialist=%s'%(i,gc.getSpecialistInfo(i).getDescription()))
					szName = "CitizenDisabledButton" + str(i)
					screen.hide( szName )
					szName = "DecreaseSpecialist" + str(i)
					screen.show( szName )

			iCount = pHeadSelectedCity.getSpecialistCount(i)

			j = 0
			while (iCount > 0):
				bHandled = True

				xCoord = xResolution - 74 - (26 * j)
				yCoord = yResolution - 272 - (26 * i)

				szName = "CitizenButton" + str((i * 100) + j)
				screen.addCheckBoxGFC( szName, gc.getSpecialistInfo(i).getTexture(), "", xCoord, yCoord, 24, 24, WidgetTypes.WIDGET_CITIZEN, i, j, ButtonStyles.BUTTON_STYLE_LABEL )
				screen.show( szName )

				szName = "CitizenChevron" + str((i * 100) + j)
				if iCount >= 20:
					szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_CHEVRON20").getPath()
					iCount -= 20
				elif iCount >= 10:
					szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_CHEVRON10").getPath()
					iCount -= 10
				elif iCount >= 5:
					szFileName = ArtFileMgr.getInterfaceArtInfo("OVERLAY_CHEVRON5").getPath()
					iCount -= 5
				else:
					szFileName = ""
					iCount -= 1

				if (szFileName != ""):
					screen.addDDSGFC( szName , szFileName, xCoord, yCoord, 10, 10, WidgetTypes.WIDGET_CITIZEN, i, False )
					screen.show( szName )

				j += 1

			if ( not bHandled ):
				szName = "CitizenDisabledButton" + str(i)
				screen.show( szName )

		return 0
# BUG - city specialist - end

	# Will update the game data strings
	def updateGameDataStrings( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		screen.hide( "ResearchText" )
		screen.hide( "GoldText" )
		screen.hide( "NewCommerceText" )

		screen.hide( "FaithText" )
		screen.hide( "LumberText" )
		screen.hide( "LeatherText" )
		screen.hide( "MetalText" )
		screen.hide( "HerbText" )
		screen.hide( "StoneText" )
		screen.hide( "UnitLimitText" )
		screen.hide( "GlobalCultureText" )

		screen.hide( "TimeText" )

##################################################################################################
# Winamp Start
##################################################################################################
		screen.hide("WinampText")
##################################################################################################
# Winamp End
##################################################################################################

		screen.hide( "ResearchBar" )
		screen.hide( "SpellResearchBar" )
		screen.hide( "SpellResearchText" )

# BUG - NJAGC - start
#		screen.hide( "EraText" )
# BUG - NJAGC - end

# BUG - Great Person Bar - start
		screen.hide( "GreatPersonBar" )
		screen.hide( "GreatPersonBarText" )
# BUG - Great Person Bar - end

# BUG - Bars on single line for higher resolution screens - start
		screen.hide( "GreatGeneralBar-w" )
		screen.hide( "ResearchBar-w" )
		screen.hide( "SpellResearchBar-w" )
		screen.hide( "GreatPersonBar-w" )
# BUG - Bars on single line for higher resolution screens - end

# Sephi - give Spell Research Bar a try
		screen.hide( "MagicRitualBar")
		screen.hide( "MagicRitualBarText")
		screen.hide( "MagicRitualBar-w")
# Sephi - give Spell Research Bar a try

# BUG - Progress Bar - Tick Marks - start
		self.pBarResearchBar_n.hide(screen)
		self.pBarResearchBar_w.hide(screen)
# BUG - Progress Bar - Tick Marks - end

		bShift = CyInterface().shiftKey()

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		if (pHeadSelectedCity):
			ePlayer = pHeadSelectedCity.getOwner()
		else:
			ePlayer = gc.getGame().getActivePlayer()

		if ( ePlayer < 0 or ePlayer >= gc.getMAX_PLAYERS() ):
			return 0

		for iI in range(CommerceTypes.NUM_COMMERCE_TYPES):
			szString = "PercentText" + str(iI)
			screen.hide(szString)
			szString = "RateText" + str(iI)
			screen.hide(szString)

		if ( CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY  and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START):

			# Percent of commerce
			if (gc.getPlayer(ePlayer).isAlive()):
				iCount = 0
				for iI in range( CommerceTypes.NUM_COMMERCE_TYPES ):
					eCommerce = (iI + 1) % CommerceTypes.NUM_COMMERCE_TYPES
					if eCommerce==CommerceTypes.COMMERCE_MANA:
						continue

					bShow=false
					if not CyInterface().isCityScreenUp():
						if gc.getPlayer(ePlayer).isCommerceFlexible(eCommerce):
							bShow=true
						if  eCommerce == CommerceTypes.COMMERCE_ARCANE or eCommerce == CommerceTypes.COMMERCE_GOLD:
							bShow=true
					if CyInterface().isCityScreenUp():
						if pHeadSelectedCity.getCommercePercentTypeForced()==-1:
							if gc.getPlayer(ePlayer).isCommerceFlexible(eCommerce) or eCommerce == CommerceTypes.COMMERCE_GOLD:
								bShow=true
						else:
							if pHeadSelectedCity.getCommerceRate(eCommerce)!=0 or eCommerce == CommerceTypes.COMMERCE_GOLD:
								bShow=true

					if (not bShow) and CyInterface().isCityScreenUp():
						if pHeadSelectedCity.getCommerceRate(eCommerce)!=0:
							iCount=iCount+1

					if bShow:
						iShift = 60
						if (CyInterface().isCityScreenUp()):
							iShift = -30
						iCommercePercent=gc.getPlayer(ePlayer).getCommercePercent(eCommerce)
						if CyInterface().isCityScreenUp() and (not pHeadSelectedCity.getCommercePercentTypeForced()==-1):
							if pHeadSelectedCity.getCommercePercentTypeForced()==eCommerce:
								iCommercePercent=100
							else:
								iCommercePercent=0
						szOutText = u"<font=2>%c:%d%%</font>" %(gc.getCommerceInfo(eCommerce).getChar(), iCommercePercent)
						szString = "PercentText" + str(iI)
						screen.setLabel( szString, "Background", szOutText, CvUtil.FONT_LEFT_JUSTIFY, 14, 50 + (iCount * 19) + iShift, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
						screen.show( szString )

						if not CyInterface().isCityScreenUp():
							szOutText = u"<font=2>" + localText.getText("TXT_KEY_MISC_POS_GOLD_PER_TURN", (gc.getPlayer(ePlayer).getCommerceRate(CommerceTypes(eCommerce)), )) + u"</font>"
							szString = "RateText" + str(iI)
# BUG - Min/Max Sliders - start
							if MainOpt.isShowMinMaxCommerceButtons():
								iMinMaxAdjustX = 40
							else:
								iMinMaxAdjustX = 0
							screen.setLabel( szString, "Background", szOutText, CvUtil.FONT_LEFT_JUSTIFY, 112 + iMinMaxAdjustX, 50 + (iCount * 19) + iShift, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
# BUG - Min/Max Sliders - end
							screen.show( szString )

						iCount = iCount + 1

			self.updateTimeText()
			screen.setLabel( "TimeText", "Background", g_szTimeText, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 56, 6, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
			screen.show( "TimeText" )

			if (gc.getPlayer(ePlayer).isAlive()):

				szText = CyGameTextMgr().getGoldStr(ePlayer)

#FfH: Added by Kael 12/08/2007
#				if (gc.getPlayer(ePlayer).getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_KHAZAD') and gc.getPlayer(ePlayer).getNumCities() > 0):
#					iGold = gc.getPlayer(ePlayer).getGold() / gc.getPlayer(ePlayer).getNumCities()
#					if iGold <= 49:
#						szText = szText + " " + localText.getText("TXT_KEY_MISC_DWARVEN_VAULT_EMPTY", ())
#					if (iGold >= 50 and iGold <= 99):
#						szText = szText + " " + localText.getText("TXT_KEY_MISC_DWARVEN_VAULT_LOW", ())
#					if (iGold >= 150 and iGold <= 199):
#						szText = szText + " " + localText.getText("TXT_KEY_MISC_DWARVEN_VAULT_STOCKED", ())
#					if (iGold >= 200 and iGold <= 299):
#						szText = szText + " " + localText.getText("TXT_KEY_MISC_DWARVEN_VAULT_ABUNDANT", ())
#					if (iGold >= 300 and iGold <= 499):
#						szText = szText + " " + localText.getText("TXT_KEY_MISC_DWARVEN_VAULT_FULL", ())
#					if iGold >= 500:
#						szText = szText + " " + localText.getText("TXT_KEY_MISC_DWARVEN_VAULT_OVERFLOWING", ())
#FfH: End Add

#added Sephi
				szText1 = CyGameTextMgr().getManaStr(ePlayer)
				szText2 = CyGameTextMgr().getFaithStr(ePlayer)
				szText3 = CyGameTextMgr().getGlobalYieldStr(ePlayer,YieldTypes.YIELD_LUMBER)
				szText4 = CyGameTextMgr().getGlobalYieldStr(ePlayer,YieldTypes.YIELD_LEATHER)
				szText5 = CyGameTextMgr().getGlobalYieldStr(ePlayer,YieldTypes.YIELD_METAL)
				szText6 = CyGameTextMgr().getGlobalYieldStr(ePlayer,YieldTypes.YIELD_HERB)
				szText7 = CyGameTextMgr().getGlobalYieldStr(ePlayer,YieldTypes.YIELD_STONE)
				szText8 = CyGameTextMgr().getUnitSupportStr(ePlayer)
				szText9 = CyGameTextMgr().getGlobalCultureStr(ePlayer)
				iShift = 0
				if (CyInterface().isCityScreenUp()):
					iShift = 0

				iTier1=0
				iTier2=75
				iTier3=150

				if not CyInterface().isCityScreenUp():
#					if (((gc.getPlayer(ePlayer).calculateGoldRate() != 0) and not (gc.getPlayer(ePlayer).isAnarchy())) or (gc.getPlayer(ePlayer).getGold() != 0)):
					screen.setLabel( "GoldText", "Background", szText, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift, 6, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_GOLD, -1, -1 )
					screen.show( "GoldText" )

					screen.setLabel( "NewCommerceText", "Background", szText1, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier1, 30, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_MANA, -1, -1 )
					screen.show( "NewCommerceText" )

					screen.setLabel( "FaithText", "Background", szText2, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier3, 30, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_FAITH, -1, -1 )
					screen.show( "FaithText" )

					screen.setLabel( "LumberText", "Background", szText3, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier1, 54, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_LUMBER, -1, -1 )
					screen.show( "LumberText" )

					screen.setLabel( "LeatherText", "Background", szText4, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier2, 54, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_LEATHER, -1, -1 )
					screen.show( "LeatherText" )

					screen.setLabel( "HerbText", "Background", szText6, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier3, 54, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_HERB, -1, -1 )
					screen.show( "HerbText" )

					screen.setLabel( "StoneText", "Background", szText7, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier1, 78, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_STONE, -1, -1 )
					screen.show( "StoneText" )

					screen.setLabel( "MetalText", "Background", szText5, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier2, 78, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_METAL, -1, -1 )
					screen.show( "MetalText" )

					screen.setLabel( "UnitLimitText", "Background", szText8, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier3, 6, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_UNITLIMIT, -1, -1 )
					screen.show( "UnitLimitText" )

					screen.setLabel( "GlobalCultureText", "Background", szText9, CvUtil.FONT_LEFT_JUSTIFY, 12 + iShift+iTier3, 78, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_GLOBAL_CULTURE, -1, -1 )
					screen.show( "GlobalCultureText" )

##################################################################################################
# Winamp Start
##################################################################################################
				if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_WINAMP_GUI):
					if ((WinAMP.GetTitleName() != "") and (g_WinAMP)):
						self.updateWinampText()
						screen.setLabel("WinampText", "Background", g_szWinampText, CvUtil.FONT_CENTER_JUSTIFY, xResolution / 2, 35, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
						if ((self.WinAMPCounter == 1) and not (CyInterface().isCityScreenUp())):
							screen.show("WinampText")
						else:
							screen.hide("WinampText")
##################################################################################################
# Winamp End
##################################################################################################

				if (gc.getPlayer(ePlayer).isAnarchy()):

# BUG - Bars on single line for higher resolution screens - start
					if (xResolution >= 1440	and MainOpt.isShowGPProgressBar()):
						xCoord = 268 + (xResolution - 1440) / 2 + 84 + 6 + 487 / 2
					else:
						xCoord = screen.centerX(512)

					yCoord = 3
					szText = localText.getText("INTERFACE_ANARCHY", (gc.getPlayer(ePlayer).getAnarchyTurns(), ))
					screen.setText( "ResearchText", "Background", szText, CvUtil.FONT_CENTER_JUSTIFY, xCoord, yCoord, -0.4, FontTypes.GAME_FONT, WidgetTypes.WIDGET_RESEARCH, -1, -1 )
# BUG - Bars on single line for higher resolution screens - end

					if ( gc.getPlayer(ePlayer).getCurrentResearch() != -1 ):
						screen.show( "ResearchText" )
					else:
						screen.hide( "ResearchText" )

				elif (gc.getPlayer(ePlayer).getCurrentResearch() != -1):

					szText = CyGameTextMgr().getResearchStr(ePlayer)

# BUG - Bars on single line for higher resolution screens - start
					if (xResolution >= 1440	and MainOpt.isShowGPProgressBar()):
						szResearchBar = "ResearchBar-w"
#						xCoord = 268 + (xResolution - 1440) / 2 + 84 + 6 + 487 / 2
						xCoord = 268 + (xResolution - 1440) / 2 + 200
					else:
						szResearchBar = "ResearchBar"
#						xCoord = screen.centerX(512)
						xCoord = screen.centerX(380)
					yCoord = 3
					screen.setText( "ResearchText", "Background", szText, CvUtil.FONT_CENTER_JUSTIFY, xCoord, yCoord, -0.4, FontTypes.GAME_FONT, WidgetTypes.WIDGET_RESEARCH, -1, -1 )
					screen.show( "ResearchText" )
# BUG - Bars on single line for higher resolution screens - end

					researchProgress = gc.getTeam(gc.getPlayer(ePlayer).getTeam()).getResearchProgress(gc.getPlayer(ePlayer).getCurrentResearch())
					overflowResearch = (gc.getPlayer(ePlayer).getOverflowResearch() * gc.getPlayer(ePlayer).calculateResearchModifier(gc.getPlayer(ePlayer).getCurrentResearch()))/100
					researchCost = gc.getTeam(gc.getPlayer(ePlayer).getTeam()).getResearchCost(gc.getPlayer(ePlayer).getCurrentResearch())
					researchRate = gc.getPlayer(ePlayer).calculateResearchRate(-1)

					iFirst = float(researchProgress + overflowResearch) / float(researchCost)
					screen.setBarPercentage( szResearchBar, InfoBarTypes.INFOBAR_STORED, iFirst )
					if ( iFirst == 1 ):
						screen.setBarPercentage( szResearchBar, InfoBarTypes.INFOBAR_RATE, ( float(researchRate) / float(researchCost) ) )
					else:
						screen.setBarPercentage( szResearchBar, InfoBarTypes.INFOBAR_RATE, ( ( float(researchRate) / float(researchCost) ) ) / ( 1 - iFirst ) )

					screen.show( szResearchBar )

# BUG - Progress Bar - Tick Marks - start
#					if MainOpt.isShowpBarTickMarks():
					if 2<1:
						if szResearchBar == "ResearchBar":
							self.pBarResearchBar_n.drawTickMarks(screen, researchProgress + overflowResearch, researchCost, researchRate, researchRate, False)
						else:
							self.pBarResearchBar_w.drawTickMarks(screen, researchProgress + overflowResearch, researchCost, researchRate, researchRate, False)
# BUG - Progress Bar - Tick Marks - end

#SPELL RESEARCH
#				if (gc.getPlayer(ePlayer).getSpellResearchFocus() != -1):
				if gc.getPlayer(ePlayer).getCurrentResearch()!=-1 and not gc.getPlayer(ePlayer).isAnarchy():
# BUG - Bars on single line for higher resolution screens - start
					if (xResolution >= 1440	and MainOpt.isShowGPProgressBar()):
						szResearchBar = "SpellResearchBar-w"
#						xCoord = 268 + (xResolution - 1440) / 2 + 84 + 6 + 487 / 2
						xCoord = 268 + (xResolution - 1440) / 2 + 487 / 2 + 250
					else:
						szResearchBar = "SpellResearchBar"
#						xCoord = screen.centerX(512)
						xCoord = screen.centerX(650)

					yCoord = 3
					szText = "Choose SpellResearch"
					if gc.getPlayer(ePlayer).getSpellResearchFocus()!=-1:
						szText = gc.getTechInfo(gc.getPlayer(ePlayer).getSpellResearchFocus()).getDescription()
						iTurnsNeeded = gc.getPlayer(ePlayer).getArcaneNeeded(gc.getPlayer(ePlayer).getSpellResearchFocus())-gc.getPlayer(ePlayer).getArcane()
						if gc.getPlayer(ePlayer).getArcaneIncome()>0:
							iTurnsNeeded = (iTurnsNeeded * 100) / gc.getPlayer(ePlayer).getArcaneIncomeTimes100()
						iTurnsNeeded = iTurnsNeeded +1
						szText = szText + " (" + str(iTurnsNeeded) + ")"
					screen.setText( "SpellResearchText", "Background", szText, CvUtil.FONT_CENTER_JUSTIFY, xCoord, yCoord, -0.4, FontTypes.GAME_FONT, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_ARCANE_SCREEN).getActionInfoIndex(), -1 )
					screen.show( "SpellResearchText" )
# BUG - Bars on single line for higher resolution screens - end

					researchProgress = gc.getPlayer(ePlayer).getArcane()
					overflowResearch = 0
					researchCost = 1000
					if gc.getPlayer(ePlayer).getSpellResearchFocus()!=-1:
						researchCost = gc.getTeam(gc.getPlayer(ePlayer).getTeam()).getResearchCost(gc.getPlayer(ePlayer).getSpellResearchFocus())
					researchRate = gc.getPlayer(ePlayer).getArcaneIncome()

					iFirst = float(researchProgress + overflowResearch) / float(researchCost)
					screen.setBarPercentage( szResearchBar, InfoBarTypes.INFOBAR_STORED, iFirst )
					if ( iFirst == 1 ):
						screen.setBarPercentage( szResearchBar, InfoBarTypes.INFOBAR_RATE, ( float(researchRate) / float(researchCost) ) )
					else:
						screen.setBarPercentage( szResearchBar, InfoBarTypes.INFOBAR_RATE, ( ( float(researchRate) / float(researchCost) ) ) / ( 1 - iFirst ) )

					screen.show( szResearchBar )

#SPELL RESEARCH

# BUG - Great Person Bar - start
				self.updateGreatPersonBar(screen)
# BUG - Great Person Bar - end

		return 0

# BUG - Great Person Bar - start
	def updateGreatPersonBar(self, screen):
		if (not CyInterface().isCityScreenUp() and MainOpt.isShowGPProgressBar()):
			szText = u""
# BUG - Bars on single line for higher resolution screens - start
			xResolution = screen.getXResolution()
			pPlayer = gc.getPlayer(CyGame().getActivePlayer())

			if (xResolution >= 1440):
				szGreatPersonBar = "GreatPersonBar-w"
				xCoord = 268 + (xResolution - 1440) / 2 + 84 + 6 + 487 + 6 + 320 / 2
				yCoord = 5
			else:
				szGreatPersonBar = "GreatPersonBar"
				xCoord = 268 + (xResolution - 1024) / 2 + 243 / 2
				yCoord = 30
# BUG - Bars on single line for higher resolution screens - end

			# find city with next great person
			bestValue = sys.maxint
			bestCity = None
			for city in PlayerUtil.playerCities(pPlayer):
				if (city.getNextGreatPersonInfo(True) != -1):
					currentValue = city.getNextGreatPersonInfo(False)
					if (currentValue < bestValue):
						bestValue = currentValue
						bestCity = city

			if(bestCity):
				iCityID = bestCity.getID()
			else:
				iCityID = -1

			if (bestCity):

				szText = u"%s: %d%c" % (gc.getUnitInfo(bestCity.getNextGreatPersonInfo(true)).getDescription(),bestCity.getNextGreatPersonInfo(false) + 1,CyGame().getSymbolID(FontSymbols.GREAT_PEOPLE_CHAR))
				szText = u"<font=2>%s</font>" % (szText)

				fThreshold = bestCity.getGreatPeopleThreshold() + float(gc.getPlayer( bestCity.getOwner() ).greatPeopleThreshold(False))
				nextGreatPerson = bestCity.getNextGreatPersonInfo(True)
				fRate = bestCity.getNextGreatPersonInfo(False)
				fFirst = float(bestCity.getGreatPeopleUnitProgress(nextGreatPerson)) / fThreshold

				screen.setBarPercentage( szGreatPersonBar, InfoBarTypes.INFOBAR_STORED, fFirst )
				if ( fFirst == 1 ):
					screen.setBarPercentage( szGreatPersonBar, InfoBarTypes.INFOBAR_RATE, 1 )
				else:
					screen.setBarPercentage( szGreatPersonBar, InfoBarTypes.INFOBAR_RATE, 1 / float(fRate + 1) )
			else:
				screen.setBarPercentage( szGreatPersonBar, InfoBarTypes.INFOBAR_STORED, 0 )
				screen.setBarPercentage( szGreatPersonBar, InfoBarTypes.INFOBAR_RATE, 0 )
				screen.setHitTest( "GreatPersonBarText", HitTestTypes.HITTEST_NOHIT )

			screen.setText( "GreatPersonBarText", "Background", szText, CvUtil.FONT_CENTER_JUSTIFY, xCoord, yCoord, -0.4, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, iCityID, -1 )
			screen.show( "GreatPersonBarText" )
			screen.show( szGreatPersonBar )
# BUG - Great Person Bar - end

# Sephi - give Spell Research Bar a try

		if (not CyInterface().isCityScreenUp() and MainOpt.isShowGPProgressBar()):
			pPlayer = gc.getPlayer(CyGame().getActivePlayer())
			szText = "Magic Ritual"
			if pPlayer.getCurrentMagicRitual()!=-1:
				szText = gc.getProjectInfo(pPlayer.getCurrentMagicRitual()).getDescription()
				iTurnsNeeded = max(0, pPlayer.getCurrentMagicRitualManaNeeded()-pPlayer.getMana())
				if pPlayer.getManaIncome() > 0:
					iTurnsNeeded = math.ceil(iTurnsNeeded / float(pPlayer.getManaIncome()))
				iTurnsNeeded = int(iTurnsNeeded + 1)
				szText = szText + " (" + str(iTurnsNeeded) + ")"
			szText = u"<font=2>%s</font>" % (szText)

# BUG - Bars on single line for higher resolution screens - start

			if (xResolution >= 1440):
				szMagicRitualBar = "MagicRitualBar-w"
				xCoord = 268 + (xResolution - 1440) / 2 + 84 + 6 + 487 + 6 + 320 / 2
				yCoord = 5+30
			else:
				szMagicRitualBar = "MagicRitualBar"
				xCoord = 268 + (xResolution - 1024) / 2 + 244 + 243 / 2
				yCoord = 30

			screen.setText( "MagicRitualBarText", "Background", szText, CvUtil.FONT_CENTER_JUSTIFY, xCoord, yCoord, -0.4, FontTypes.GAME_FONT, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_MAGIC_SCREEN).getActionInfoIndex(), -1 )
			screen.show( "MagicRitualBarText" )

# BUG - Bars on single line for higher resolution screens - end

			fThreshold = 1000
			if pPlayer.getCurrentMagicRitual()!=-1:
				fThreshold = float(pPlayer.getSpecificMagicRitualCost(pPlayer.getCurrentMagicRitual()))

			iProgress = pPlayer.getManaIncome()

			fRate = float(iProgress)
			fFirst = 0
			if pPlayer.getCurrentMagicRitual()!=-1:
				fFirst = float((fThreshold - (pPlayer.getCurrentMagicRitualManaNeeded() - pPlayer.getMana())) / fThreshold)

			screen.setBarPercentage( szMagicRitualBar, InfoBarTypes.INFOBAR_STORED, fFirst)
			if ( fFirst == 1 ):
				screen.setBarPercentage( szMagicRitualBar, InfoBarTypes.INFOBAR_RATE, fRate / fThreshold )
			else:
				screen.setBarPercentage( szMagicRitualBar, InfoBarTypes.INFOBAR_RATE, fRate / fThreshold / ( 1 - fFirst ) )

			screen.show( szMagicRitualBar )
# Sephi - give Spell Research Bar a try

	def updateTimeText( self ):

		global g_szTimeText

		ePlayer = gc.getGame().getActivePlayer()

# BUG - NJAGC - start
		if (ClockOpt.isEnabled()):
			"""
			Format: Time - GameTurn/Total Percent - GA (TurnsLeft) Date

			Ex: 10:37 - 220/660 33% - GA (3) 1925
			"""
			if (g_bShowTimeTextAlt):
				bShowTime = ClockOpt.isShowAltTime()
				bShowGameTurn = ClockOpt.isShowAltGameTurn()
				bShowTotalTurns = ClockOpt.isShowAltTotalTurns()
				bShowPercentComplete = ClockOpt.isShowAltPercentComplete()
				bShowDateGA = ClockOpt.isShowAltDateGA()
			else:
				bShowTime = ClockOpt.isShowTime()
				bShowGameTurn = ClockOpt.isShowGameTurn()
				bShowTotalTurns = ClockOpt.isShowTotalTurns()
				bShowPercentComplete = ClockOpt.isShowPercentComplete()
				bShowDateGA = ClockOpt.isShowDateGA()

			if (not gc.getGame().getMaxTurns() > 0):
				bShowTotalTurns = False
				bShowPercentComplete = False

			bFirst = True
			g_szTimeText = ""

			if (bShowTime):
				bFirst = False
				g_szTimeText += getClockText()

			if (bShowGameTurn):
				if (bFirst):
					bFirst = False
				else:
					g_szTimeText += u" - "
				g_szTimeText += u"%d" %( gc.getGame().getElapsedGameTurns() )
				if (bShowTotalTurns):
					g_szTimeText += u"/%d" %( gc.getGame().getMaxTurns() )

			if (bShowPercentComplete):
				if (bFirst):
					bFirst = False
				else:
					if (not bShowGameTurn):
						g_szTimeText += u" - "
					else:
						g_szTimeText += u" "
				g_szTimeText += u"%2.2f%%" %( 100 *(float(gc.getGame().getElapsedGameTurns()) / float(gc.getGame().getMaxTurns())) )

			if (bShowDateGA):
				if (bFirst):
					bFirst = False
				else:
					g_szTimeText += u" - "
				szDateGA = unicode(CyGameTextMgr().getInterfaceTimeStr(ePlayer))
				if(ClockOpt.isUseEraColor()):
					iEraColor = ClockOpt.getEraColor(gc.getEraInfo(gc.getPlayer(ePlayer).getCurrentEra()).getType())
					if (iEraColor >= 0):
						szDateGA = localText.changeTextColor(szDateGA, iEraColor)
				g_szTimeText += szDateGA
		else:
			"""
			Original Clock
			Format: Time - 'Turn' GameTurn - GA (TurnsLeft) Date

			Ex: 10:37 - Turn 220 - GA (3) 1925
			"""
			g_szTimeText = localText.getText("TXT_KEY_TIME_TURN", (CyGame().getGameTurn(), )) + u" - " + unicode(CyGameTextMgr().getInterfaceTimeStr(ePlayer))
			if (CyUserProfile().isClockOn()):
				g_szTimeText = getClockText() + u" - " + g_szTimeText
# BUG - NJAGC - end

##################################################################################################
# Winamp Start
##################################################################################################
	def updateWinampText(self):
		global g_szWinampText
		iWinAMPProgress = WinAMP.GetTitleProgress()/1000
		if (iWinAMPProgress < 0):
			iWinAMPProgress = 0
		iWinAMPLength = WinAMP.GetTitleLength()
		iMinutes = iWinAMPLength/60
		iSeconds = iWinAMPLength-(iMinutes*60)

		if (iSeconds < 10):
			szSeconds = "0" + str(iSeconds)
		else:
			szSeconds = str(iSeconds)

		iMinutesProgress = iWinAMPProgress/60
		iSecondsProgress = iWinAMPProgress-(iMinutesProgress*60)

		if (iSecondsProgress < 10):
			szSecondsProgress = "0" + str(iSecondsProgress)
		else:
			szSecondsProgress = str(iSecondsProgress)

		sWinAMPHelpText = unicode(WinAMP.GetTitleName())
		sWinAMPText = sWinAMPHelpText.replace("&", "+")
		g_szWinampText = sWinAMPText + " (" + str(iMinutesProgress) + ":" + szSecondsProgress + " / " + str(iMinutes) + ":" + szSeconds + ")"
##################################################################################################
# Winamp End
##################################################################################################


	# Will update the selection Data Strings
	def updateCityScreen( self ):

		global MAX_DISPLAYABLE_BUILDINGS
		global MAX_DISPLAYABLE_TRADE_ROUTES
		global MAX_BONUS_ROWS

		global g_iNumTradeRoutes
		global g_iNumBuildings
		global g_iNumLeftBonus
		global g_iNumCenterBonus
		global g_iNumRightBonus

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()

		# Find out our resolution
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		bShift = CyInterface().shiftKey()

		screen.hide( "PopulationBar" )
		screen.hide( "ProductionBar" )
		screen.hide( "GreatPeopleBar" )
		screen.hide( "CultureBar" )
		screen.hide( "MaintenanceText" )
		screen.hide( "MaintenanceAmountText" )
		screen.hide( "SpecializationText" )
		screen.hide( "InfrastructureText" )

# BUG - Progress Bar - Tick Marks - start
		self.pBarPopulationBar.hide(screen)
		self.pBarProductionBar.hide(screen)
		self.pBarProductionBar_Whip.hide(screen)
# BUG - Progress Bar - Tick Marks - end

# BUG - Raw Commerce - start
		screen.hide("RawYieldsTrade0")
		screen.hide("RawYieldsFood1")
		screen.hide("RawYieldsProduction2")
		screen.hide("RawYieldsCommerce3")
		screen.hide("RawYieldsWorkedTiles4")
		screen.hide("RawYieldsCityTiles5")
		screen.hide("RawYieldsOwnedTiles6")
# BUG - Raw Commerce - end

		screen.hide( "NationalityText" )
		screen.hide( "NationalityBar" )
# UNB start
		screen.hide( "RevoltTextCity" )
# UNB end
		screen.hide( "DefenseText" )
		screen.hide( "CityScrollMinus" )
		screen.hide( "CityScrollPlus" )
		screen.hide( "CityNameText" )
		screen.hide( "PopulationText" )
		screen.hide( "PopulationInputText" )
		screen.hide( "HealthText" )
		screen.hide( "ProductionText" )
		screen.hide( "ProductionInputText" )
		screen.hide( "HappinessText" )
		screen.hide( "CultureText" )
		screen.hide( "GreatPeopleText" )

		for i in range( gc.getNumReligionInfos() ):
			szName = "ReligionHolyCityDDS" + str(i)
			screen.hide( szName )
			szName = "ReligionDDS" + str(i)
			screen.hide( szName )

		for i in range( gc.getNumCorporationInfos() ):
			szName = "CorporationHeadquarterDDS" + str(i)
			screen.hide( szName )
			szName = "CorporationDDS" + str(i)
			screen.hide( szName )

		for i in range(CommerceTypes.NUM_COMMERCE_TYPES):
			szName = "CityPercentText" + str(i)
			screen.hide( szName )

#FfH: Added by Kael 07/18/2007
#		screen.setPanelSize( "InterfaceCenterBackgroundWidget", 296, yResolution - 133, xResolution - (296*2), 133)
#		screen.setPanelSize( "InterfaceLeftBackgroundWidget", 0, yResolution - 168, 304, 168)
#		screen.setPanelSize( "InterfaceRightBackgroundWidget", xResolution - 304, yResolution - 168, 304, 168)
#		screen.setPanelSize( "MiniMapPanel", xResolution - 214, yResolution - 151, 208, 151)
		iMultiListXR = 332
#FfH: End Add

		# changed 94 to 114
		#SpyFanatic: changed 520 to 620 and 536 to 636 else Yeoman is not selectable, and adjusted width
		#screen.addPanel( "BonusPane0", u"", u"", True, False, xResolution - 244, 114, 57, yResolution - 520, PanelStyles.PANEL_STYLE_CITY_COLUMNL )
		screen.hide( "BonusPane0" )
		screen.addPanel( "BonusPane0", u"", u"", True, False, xResolution - 244, 114, 65, yResolution - 560, PanelStyles.PANEL_STYLE_CITY_COLUMNL )
		screen.hide( "BonusPane0" )
		#screen.addScrollPanel( "BonusBack0", u"", xResolution - 242, 114, 157, yResolution - 536, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.addScrollPanel( "BonusBack0", u"", xResolution - 244, 114, 65, yResolution - 566, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.hide( "BonusBack0" )

		#screen.addPanel( "BonusPane1", u"", u"", True, False, xResolution - 187, 114, 68, yResolution - 520, PanelStyles.PANEL_STYLE_CITY_COLUMNC )
		screen.addPanel( "BonusPane1", u"", u"", True, False, xResolution - 191, 114, 94, yResolution - 560, PanelStyles.PANEL_STYLE_CITY_COLUMNC )
		screen.hide( "BonusPane1" )
		#screen.addScrollPanel( "BonusBack1", u"", xResolution - 191, 114, 184, yResolution - 536, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.addScrollPanel( "BonusBack1", u"", xResolution - 191, 114, 94, yResolution - 566, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.hide( "BonusBack1" )

		#screen.addPanel( "BonusPane2", u"", u"", True, False, xResolution - 119, 114, 107, yResolution - 520, PanelStyles.PANEL_STYLE_CITY_COLUMNR )
		screen.addPanel( "BonusPane2", u"", u"", True, False, xResolution - 109, 114, 100, yResolution - 560, PanelStyles.PANEL_STYLE_CITY_COLUMNR )
		screen.hide( "BonusPane2" )
		#screen.addScrollPanel( "BonusBack2", u"", xResolution - 125, 114, 205, yResolution - 536, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.addScrollPanel( "BonusBack2", u"", xResolution - 109, 114, 100, yResolution - 566, PanelStyles.PANEL_STYLE_EXTERNAL )
		screen.hide( "BonusBack2" )

		screen.hide( "TradeRouteTable" )
		screen.hide( "BuildingListTable" )

		screen.hide( "BuildingListBackground" )
		screen.hide( "TradeRouteListBackground" )
		screen.hide( "BuildingListLabel" )
		screen.hide( "TradeRouteListLabel" )

		i = 0
		for i in range( g_iNumLeftBonus ):
			szName = "LeftBonusItem" + str(i)
			screen.hide( szName )

		i = 0
		for i in range( g_iNumCenterBonus ):
			szName = "CenterBonusItemLeft" + str(i)
			screen.hide( szName )
			szName = "CenterBonusItemRight" + str(i)
			screen.hide( szName )

		i = 0
		for i in range( g_iNumRightBonus ):
			szName = "RightBonusItemLeft" + str(i)
			screen.hide( szName )
			szName = "RightBonusItemRight" + str(i)
			screen.hide( szName )

		i = 0
		for i in range( 3 ):
			szName = "BonusPane" + str(i)
			screen.hide( szName )
			szName = "BonusBack" + str(i)
			screen.hide( szName )

		i = 0
		if ( CyInterface().isCityScreenUp() ):
			if ( pHeadSelectedCity ):

# Customized City Background start
				if CityScreenOpt.isDisableFlavor():
					screen.show( "InterfaceCenterLeftBackgroundWidget" )
					screen.show( "InterfaceCenterRightBackgroundWidget" )
					screen.show( "InterfaceCityCenterBackgroundWidget" )

					screen.show( "TopCityPanelLeft" )
					screen.show( "TopCityPanelRight" )

					screen.show( "InterfaceTopLeftBackgroundWidget" )
					screen.show( "InterfaceTopRightBackgroundWidget" )
					screen.show( "CityScreenTopWidget" )
					screen.show( "CityNameBackground" )

					screen.show( "CityExtra1" )
					screen.show( "InterfaceCityLeftBackgroundWidget" )
					screen.show( "InterfaceCityRightBackgroundWidget" )

					screen.show( "CityScreenAdjustPanel" )

				else:
					screen.show( "InterfaceCityBackgroundTopLeft" )
					screen.show( "InterfaceCityBackgroundTopRight" )
					screen.show( "InterfaceCityBackgroundTopCenter" )

					screen.show( "TopCityPanelLeft" )
					screen.show( "TopCityPanelRight" )

					screen.show( "CityNameBackground" )

#					screen.show( "CityExtra1" )
#					screen.show( "InterfaceCityLeftBackgroundWidget" )
#					screen.show( "InterfaceCityRightBackgroundWidget" )

					screen.show("InterfaceCityBackgroundTop")
					screen.show("InterfaceCityBackgroundLeft")
					screen.show("InterfaceCityBackgroundRight")
					screen.show("InterfaceCityBackgroundBottom")

#					screen.hide( "InterfaceCenterBackgroundWidget" )
# Customized City Background End


				if ( pHeadSelectedCity.getTeam() == gc.getGame().getActiveTeam() ):
					if ( gc.getActivePlayer().getNumCities() > 1 ):
						screen.show( "CityScrollMinus" )
						screen.show( "CityScrollPlus" )

				# Help Text Area
				screen.setHelpTextArea( 390, FontTypes.SMALL_FONT, 0, 0, -2.2, True, ArtFileMgr.getInterfaceArtInfo("POPUPS_BACKGROUND_TRANSPARENT").getPath(), True, True, CvUtil.FONT_LEFT_JUSTIFY, 0 )

				iFoodDifference = pHeadSelectedCity.foodDifference(True)
				iProductionDiffNoFood = pHeadSelectedCity.getCurrentProductionDifference(True, True)
				iProductionDiffJustFood = (pHeadSelectedCity.getCurrentProductionDifference(False, True) - iProductionDiffNoFood)

				szBuffer = u"<font=4>"

				if (pHeadSelectedCity.isCapital()):
					szBuffer += u"%c" %(CyGame().getSymbolID(FontSymbols.STAR_CHAR))
				elif (pHeadSelectedCity.isGovernmentCenter()):
					szBuffer += u"%c" %(CyGame().getSymbolID(FontSymbols.SILVER_STAR_CHAR))

				if (pHeadSelectedCity.isPower()):
					szBuffer += u"%c" %(CyGame().getSymbolID(FontSymbols.POWER_CHAR))

				szBuffer += u"%s: %d" %(pHeadSelectedCity.getName(), pHeadSelectedCity.getPopulation())

				if (pHeadSelectedCity.isOccupation()):
					szBuffer += u" (%c:%d)" %(CyGame().getSymbolID(FontSymbols.OCCUPATION_CHAR), pHeadSelectedCity.getOccupationTimer())

				if (gc.getPlayer(pHeadSelectedCity.getOwner()).getPopulationLimit()>-1):
					szBuffer += u" (Population Limit: %d)"%(gc.getPlayer(pHeadSelectedCity.getOwner()).getPopulationLimit()+pHeadSelectedCity.getPopulationLimit())

				szBuffer += u"</font>"

				screen.setText( "CityNameText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, screen.centerX(512), 32, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_CITY_NAME, -1, -1 )
				screen.setStyle( "CityNameText", "Button_Stone_Style" )
				screen.show( "CityNameText" )

				if ( (iFoodDifference != 0) or not (pHeadSelectedCity.isFoodProduction() ) ):
# FlavourMod: Changed by Jean Elcard 09/03/2009 (growth control buttons)
					'''
					if (iFoodDifference > 0):
					'''
					if pHeadSelectedCity.AI_stopGrowth():
						szBuffer = localText.getText("INTERFACE_CITY_STAGNANT_GROWTH_CONTROL", ())
					elif (iFoodDifference > 0):
# FlavourMod: End Change
						szBuffer = localText.getText("INTERFACE_CITY_GROWING", (pHeadSelectedCity.getFoodTurnsLeft(), ))
					elif (iFoodDifference < 0):
						if (CityScreenOpt.isShowFoodAssist()):
							iTurnsToStarve = pHeadSelectedCity.getFood() / -iFoodDifference + 1
							if iTurnsToStarve > 1:
								szBuffer = localText.getText("INTERFACE_CITY_SHRINKING", (iTurnsToStarve, ))
							else:
								szBuffer = localText.getText("INTERFACE_CITY_STARVING", ())
						else:
							szBuffer = localText.getText("INTERFACE_CITY_STARVING", ())
					else:
						szBuffer = localText.getText("INTERFACE_CITY_STAGNANT", ())

					screen.setLabel( "PopulationText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, screen.centerX(512), iCityCenterRow1Y, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.setHitTest( "PopulationText", HitTestTypes.HITTEST_NOHIT )
					screen.show( "PopulationText" )

				if (not pHeadSelectedCity.isDisorder() and not pHeadSelectedCity.isFoodProduction()):

					if (CityScreenOpt.isShowFoodAssist()):
						iFoodYield = pHeadSelectedCity.getYieldRate(YieldTypes.YIELD_FOOD)
						iFoodEaten = pHeadSelectedCity.foodConsumption(False, 0)
						if iFoodYield == iFoodEaten:
							szBuffer = localText.getText("INTERFACE_CITY_FOOD_STAGNATE", (iFoodYield, iFoodEaten))
						elif iFoodYield > iFoodEaten:
							szBuffer = localText.getText("INTERFACE_CITY_FOOD_GROW", (iFoodYield, iFoodEaten, iFoodYield - iFoodEaten))
						else:
							szBuffer = localText.getText("INTERFACE_CITY_FOOD_SHRINK", (iFoodYield, iFoodEaten, iFoodYield - iFoodEaten))
					else:
						szBuffer = u"%d%c - %d%c" %(pHeadSelectedCity.getYieldRate(YieldTypes.YIELD_FOOD), gc.getYieldInfo(YieldTypes.YIELD_FOOD).getChar(), pHeadSelectedCity.foodConsumption(False, 0), CyGame().getSymbolID(FontSymbols.EATEN_FOOD_CHAR))
					screen.setLabel( "PopulationInputText", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, iCityCenterRow1X - 6, iCityCenterRow1Y, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_FOOD_MOD_HELP, -1, -1 )
					screen.show( "PopulationInputText" )

				else:

					szBuffer = u"%d%c" %(iFoodDifference, gc.getYieldInfo(YieldTypes.YIELD_FOOD).getChar())
					screen.setLabel( "PopulationInputText", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, iCityCenterRow1X - 6, iCityCenterRow1Y, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_FOOD_MOD_HELP, -1, -1 )
					screen.show( "PopulationInputText" )

				if ((pHeadSelectedCity.badHealth(False) > 0) or (pHeadSelectedCity.goodHealth() >= 0)):
					if (pHeadSelectedCity.healthRate(False, 0) < 0):
						szBuffer = localText.getText("INTERFACE_CITY_HEALTH_BAD", (pHeadSelectedCity.goodHealth(), pHeadSelectedCity.badHealth(False), 2*pHeadSelectedCity.healthRate(False, 0)))
					elif (pHeadSelectedCity.badHealth(False) > 0):
						szBuffer = localText.getText("INTERFACE_CITY_HEALTH_GOOD", (pHeadSelectedCity.goodHealth(), pHeadSelectedCity.badHealth(False)))
					else:
						szBuffer = localText.getText("INTERFACE_CITY_HEALTH_GOOD_NO_BAD", (pHeadSelectedCity.goodHealth(), ))

					screen.setLabel( "HealthText", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, xResolution - iCityCenterRow1X + 6, iCityCenterRow1Y, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_HEALTH, -1, -1 )
					screen.show( "HealthText" )

				if (iFoodDifference < 0):

					if ( pHeadSelectedCity.getFood() + iFoodDifference > 0 ):
						iDeltaFood = pHeadSelectedCity.getFood() + iFoodDifference
					else:
						iDeltaFood = 0
					if ( -iFoodDifference < pHeadSelectedCity.getFood() ):
						iExtraFood = -iFoodDifference
					else:
						iExtraFood = pHeadSelectedCity.getFood()
					iFirst = float(iDeltaFood) / float(pHeadSelectedCity.growthThreshold())
					screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_STORED, iFirst )
					screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_RATE, 0.0 )
					if ( iFirst == 1 ):
						screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_RATE_EXTRA, ( float(iExtraFood) / float(pHeadSelectedCity.growthThreshold()) ) )
					else:
						screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_RATE_EXTRA, ( ( float(iExtraFood) / float(pHeadSelectedCity.growthThreshold()) ) ) / ( 1 - iFirst ) )

				else:

					iFirst = float(pHeadSelectedCity.getFood()) / float(pHeadSelectedCity.growthThreshold())
					screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_STORED, iFirst )
					if ( iFirst == 1 ):
						screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_RATE, ( float(iFoodDifference) / float(pHeadSelectedCity.growthThreshold()) ) )
					else:
						screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_RATE, ( ( float(iFoodDifference) / float(pHeadSelectedCity.growthThreshold()) ) ) / ( 1 - iFirst ) )
					screen.setBarPercentage( "PopulationBar", InfoBarTypes.INFOBAR_RATE_EXTRA, 0.0 )

				screen.show( "PopulationBar" )

# BUG - Progress Bar - Tick Marks - start
				if MainOpt.isShowpBarTickMarks():
					self.pBarPopulationBar.drawTickMarks(screen, pHeadSelectedCity.getFood(), pHeadSelectedCity.growthThreshold(), iFoodDifference, iFoodDifference,False)
# BUG - Progress Bar - Tick Marks - end

				if (pHeadSelectedCity.getOrderQueueLength() > 0):
					if (pHeadSelectedCity.isProductionProcess()):
						szBuffer = pHeadSelectedCity.getProductionName()
# BUG - Whip Assist - start
					else:
						HURRY_WHIP = gc.getInfoTypeForString("HURRY_POPULATION")
						HURRY_BUY = gc.getInfoTypeForString("HURRY_GOLD")
						if (CityScreenOpt.isShowWhipAssist() and pHeadSelectedCity.canHurry(HURRY_WHIP, False)):
							iHurryPop = pHeadSelectedCity.hurryPopulation(HURRY_WHIP)
							iHurryOverflow = pHeadSelectedCity.hurryProduction(HURRY_WHIP) - pHeadSelectedCity.productionLeft()
							if CityScreenOpt.isWhipAssistOverflowCountCurrentProduction():
								iHurryOverflow = iHurryOverflow + pHeadSelectedCity.getCurrentProductionDifference(True, False)
							iMaxOverflow = min(pHeadSelectedCity.getProductionNeeded(), iHurryOverflow)
							iOverflowGold = max(0, iHurryOverflow - iMaxOverflow) * gc.getDefineINT("MAXED_UNIT_GOLD_PERCENT") / 100
							iHurryOverflow =  100 * iMaxOverflow / pHeadSelectedCity.getBaseYieldRateModifier(gc.getInfoTypeForString("YIELD_PRODUCTION"), pHeadSelectedCity.getProductionModifier())
							if (iOverflowGold > 0):
								szBuffer = localText.getText("INTERFACE_CITY_PRODUCTION_WHIP_PLUS_GOLD", (pHeadSelectedCity.getProductionNameKey(), pHeadSelectedCity.getProductionTurnsLeft(), iHurryPop, iHurryOverflow, iOverflowGold))
							else:
								szBuffer = localText.getText("INTERFACE_CITY_PRODUCTION_WHIP", (pHeadSelectedCity.getProductionNameKey(), pHeadSelectedCity.getProductionTurnsLeft(), iHurryPop, iHurryOverflow))
						elif (CityScreenOpt.isShowWhipAssist() and pHeadSelectedCity.canHurry(HURRY_BUY, False)):
							iHurryCost = pHeadSelectedCity.hurryGold(HURRY_BUY)
							szBuffer = localText.getText("INTERFACE_CITY_PRODUCTION_BUY", (pHeadSelectedCity.getProductionNameKey(), pHeadSelectedCity.getProductionTurnsLeft(), iHurryCost))
						else:
							szBuffer = localText.getText("INTERFACE_CITY_PRODUCTION", (pHeadSelectedCity.getProductionNameKey(), pHeadSelectedCity.getProductionTurnsLeft()))
# BUG - Whip Assist - end

					screen.setLabel( "ProductionText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, screen.centerX(512), iCityCenterRow2Y, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.setHitTest( "ProductionText", HitTestTypes.HITTEST_NOHIT )
					screen.show( "ProductionText" )

				if (pHeadSelectedCity.isProductionProcess()):
					szBuffer = u"%d%c" %(pHeadSelectedCity.getYieldRate(YieldTypes.YIELD_PRODUCTION), gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar())
				elif (pHeadSelectedCity.isFoodProduction() and (iProductionDiffJustFood > 0)):
					szBuffer = u"%d%c + %d%c" %(iProductionDiffJustFood, gc.getYieldInfo(YieldTypes.YIELD_FOOD).getChar(), iProductionDiffNoFood, gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar())
				else:
					szBuffer = u"%d%c" %(iProductionDiffNoFood, gc.getYieldInfo(YieldTypes.YIELD_PRODUCTION).getChar())

				screen.setLabel( "ProductionInputText", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, iCityCenterRow1X - 6, iCityCenterRow2Y, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_PRODUCTION_MOD_HELP, -1, -1 )
				screen.show( "ProductionInputText" )

# BUG - Anger Display - start
				bShowAngerCounter = False
# BUG - Anger Display - end
				if ((pHeadSelectedCity.happyLevel() >= 0) or (pHeadSelectedCity.unhappyLevel(0) > 0)):
					if (pHeadSelectedCity.isDisorder()):
						szBuffer = u"%d%c" %(pHeadSelectedCity.angryPopulation(0), CyGame().getSymbolID(FontSymbols.ANGRY_POP_CHAR))
					elif (pHeadSelectedCity.angryPopulation(0) > 0):
						szBuffer = localText.getText("INTERFACE_CITY_UNHAPPY", (pHeadSelectedCity.happyLevel(), pHeadSelectedCity.unhappyLevel(0), pHeadSelectedCity.angryPopulation(0)))
# BUG - Anger Display - start
						bShowAngerCounter = True
# BUG - Anger Display - end
					elif (pHeadSelectedCity.unhappyLevel(0) > 0):
						szBuffer = localText.getText("INTERFACE_CITY_HAPPY", (pHeadSelectedCity.happyLevel(), pHeadSelectedCity.unhappyLevel(0)))
# BUG - Anger Display - start
						bShowAngerCounter = True
# BUG - Anger Display - end
					else:
						szBuffer = localText.getText("INTERFACE_CITY_HAPPY_NO_UNHAPPY", (pHeadSelectedCity.happyLevel(), ))

# BUG - Anger Display - start
					if (CityScreenOpt.isShowAngerCounter()
					and bShowAngerCounter):
						iAngerTimer = pHeadSelectedCity.getHurryAngerTimer()
						if iAngerTimer < pHeadSelectedCity.getConscriptAngerTimer():
							iAngerTimer = pHeadSelectedCity.getConscriptAngerTimer()

						if iAngerTimer != 0:
							szAnger = u"(%i)" %(iAngerTimer)
						else:
							szAnger = ""
#						szAnger = pHeadSelectedCity.flatHurryAngerLength()
						szBuffer = u"%s %s" %(szBuffer, szAnger)
# BUG - Anger Display - end

					screen.setLabel( "HappinessText", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, xResolution - iCityCenterRow1X + 6, iCityCenterRow2Y, -0.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_HELP_HAPPINESS, -1, -1 )
					screen.show( "HappinessText" )

				if (not(pHeadSelectedCity.isProductionProcess())):

					iFirst = ((float(pHeadSelectedCity.getProduction())) / (float(pHeadSelectedCity.getProductionNeeded())))
					screen.setBarPercentage( "ProductionBar", InfoBarTypes.INFOBAR_STORED, iFirst )
					if ( iFirst == 1 ):
						iSecond = ( ((float(iProductionDiffNoFood)) / (float(pHeadSelectedCity.getProductionNeeded()))) )
					else:
						iSecond = ( ((float(iProductionDiffNoFood)) / (float(pHeadSelectedCity.getProductionNeeded()))) ) / ( 1 - iFirst )
					screen.setBarPercentage( "ProductionBar", InfoBarTypes.INFOBAR_RATE, iSecond )
					if ( iFirst + iSecond == 1 ):
						screen.setBarPercentage( "ProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, ( ((float(iProductionDiffJustFood)) / (float(pHeadSelectedCity.getProductionNeeded()))) ) )
					else:
						screen.setBarPercentage( "ProductionBar", InfoBarTypes.INFOBAR_RATE_EXTRA, ( ( ((float(iProductionDiffJustFood)) / (float(pHeadSelectedCity.getProductionNeeded()))) ) ) / ( 1 - ( iFirst + iSecond ) ) )

					screen.show( "ProductionBar" )

# BUG - Progress Bar - Tick Marks - start
#					BugUtil.debug("tick mark test: getName %s", pHeadSelectedCity.getName())
#					BugUtil.debug("tick mark test: getProduction %i", pHeadSelectedCity.getProduction())
#					BugUtil.debug("tick mark test: getProductionModifier %i", pHeadSelectedCity.getProductionModifier())
#					BugUtil.debug("tick mark test: getProductionNeeded %i", pHeadSelectedCity.getProductionNeeded())
#					BugUtil.debug("tick mark test: getProductionTurnsLeft %i", pHeadSelectedCity.getProductionTurnsLeft())
#					BugUtil.debug("tick mark test: getCurrentProductionDifference TT %i", pHeadSelectedCity.getCurrentProductionDifference(True, True))
#					BugUtil.debug("tick mark test: getCurrentProductionDifference FT %i", pHeadSelectedCity.getCurrentProductionDifference(False, True))
#					BugUtil.debug("tick mark test: getCurrentProductionDifference TF %i", pHeadSelectedCity.getCurrentProductionDifference(True, False))
#					BugUtil.debug("tick mark test: getCurrentProductionDifference FF %i", pHeadSelectedCity.getCurrentProductionDifference(False, False))
					if MainOpt.isShowpBarTickMarks():
						if (pHeadSelectedCity.isProductionProcess()):
							iFirst = 0
							iRate = 0
						elif (pHeadSelectedCity.isFoodProduction() and (iProductionDiffJustFood > 0)):
							iFirst = pHeadSelectedCity.getCurrentProductionDifference(True, True)  # ignore food, overflow
							iRate = pHeadSelectedCity.getCurrentProductionDifference(False, False)
						else:
							iFirst = pHeadSelectedCity.getCurrentProductionDifference(True, True)
							iRate = pHeadSelectedCity.getCurrentProductionDifference(True, False)
						self.pBarProductionBar.drawTickMarks(screen, pHeadSelectedCity.getProduction(), pHeadSelectedCity.getProductionNeeded(), iFirst, iRate, False)

						HURRY_WHIP = gc.getInfoTypeForString("HURRY_POPULATION")
						if pHeadSelectedCity.canHurry(HURRY_WHIP, False):
							iRate = pHeadSelectedCity.hurryProduction(HURRY_WHIP) / pHeadSelectedCity.hurryPopulation(HURRY_WHIP)
							self.pBarProductionBar_Whip.drawTickMarks(screen, pHeadSelectedCity.getProduction(), pHeadSelectedCity.getProductionNeeded(), iFirst, iRate, True)
# BUG - Progress Bar - Tick Marks - end

				iCount = 0

				for iI in range( CommerceTypes.NUM_COMMERCE_TYPES ):
					eCommerce = (iI + 1) % CommerceTypes.NUM_COMMERCE_TYPES
					if eCommerce==CommerceTypes.COMMERCE_MANA:
						continue
					iShift = -30

					bShow=false
					if CyInterface().isCityScreenUp():
						if pHeadSelectedCity.getCommerceRate(eCommerce)!=0 or (gc.getPlayer(pHeadSelectedCity.getOwner()).isCommerceFlexible(eCommerce) and pHeadSelectedCity.getCommercePercentTypeForced()==-1):
							bShow=true
					if bShow:
#					if ((gc.getPlayer(pHeadSelectedCity.getOwner()).isCommerceFlexible(eCommerce) or eCommerce == CommerceTypes.COMMERCE_GOLD) and (() or (CyInterface().isCityScreenUp() and (pHeadSelectedCity.getCommercePercentTypeForced()==-1)))) or (CyInterface().isCityScreenUp() and pHeadSelectedCity.getCommercePercentTypeForced()==eCommerce):

						szBuffer = u"%d.%02d %c" %(pHeadSelectedCity.getCommerceRate(eCommerce), pHeadSelectedCity.getCommerceRateTimes100(eCommerce)%100, gc.getCommerceInfo(eCommerce).getChar())

						iHappiness = pHeadSelectedCity.getCommerceHappinessByType(eCommerce)

						if (iHappiness != 0):
							if ( iHappiness > 0 ):
								szTempBuffer = u", %d%c" %(iHappiness, CyGame().getSymbolID(FontSymbols.HAPPY_CHAR))
							else:
								szTempBuffer = u", %d%c" %(-iHappiness, CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR))
							szBuffer = szBuffer + szTempBuffer

						szName = "CityPercentText" + str(iCount)
						screen.setLabel( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, 220, 45 + (19 * iCount) + 4+iShift, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_COMMERCE_MOD_HELP, eCommerce, -1 )
						screen.show( szName )
						iCount = iCount + 1

				iCount = 0

				screen.addTableControlGFC( "TradeRouteTable", 3, 10, 187, 238, 98, False, False, 32, 32, TableStyles.TABLE_STYLE_STANDARD )
				screen.setStyle( "TradeRouteTable", "Table_City_Style" )
				screen.addTableControlGFC( "BuildingListTable", 3, 10, 317, 238, yResolution - 541, False, False, 32, 32, TableStyles.TABLE_STYLE_STANDARD )
				screen.setStyle( "BuildingListTable", "Table_City_Style" )

# BUG - Raw Yields - start
				bShowRawYields = g_bYieldView and CityScreenOpt.isShowRawYields()
				if (bShowRawYields):
					screen.addTableControlGFC( "TradeRouteTable", 4, 10, 187, 238, 98, False, False, 32, 32, TableStyles.TABLE_STYLE_STANDARD )
					screen.setStyle( "TradeRouteTable", "Table_City_Style" )
					screen.setTableColumnHeader( "TradeRouteTable", 0, u"", 111 )
					screen.setTableColumnHeader( "TradeRouteTable", 1, u"", 60 )
					screen.setTableColumnHeader( "TradeRouteTable", 2, u"", 55 )
					screen.setTableColumnHeader( "TradeRouteTable", 3, u"", 10 )
					screen.setTableColumnRightJustify( "TradeRouteTable", 1 )
					screen.setTableColumnRightJustify( "TradeRouteTable", 2 )
				else:
					screen.addTableControlGFC( "TradeRouteTable", 3, 10, 187, 238, 98, False, False, 32, 32, TableStyles.TABLE_STYLE_STANDARD )
					screen.setStyle( "TradeRouteTable", "Table_City_Style" )
					screen.setTableColumnHeader( "TradeRouteTable", 0, u"", 78 )
					screen.setTableColumnHeader( "TradeRouteTable", 1, u"", 148 )
					screen.setTableColumnHeader( "TradeRouteTable", 2, u"", 10 )
					screen.setTableColumnRightJustify( "TradeRouteTable", 1 )
# BUG - Raw Yields - end

				screen.setTableColumnHeader( "BuildingListTable", 0, u"", 108 )
				screen.setTableColumnHeader( "BuildingListTable", 1, u"", 118 )
				screen.setTableColumnHeader( "BuildingListTable", 2, u"", 10 )
				screen.setTableColumnRightJustify( "BuildingListTable", 1 )

				screen.show( "BuildingListBackground" )
				screen.show( "TradeRouteListBackground" )
				screen.show( "BuildingListLabel" )

# BUG - Raw Yields - start
				if (CityScreenOpt.isShowRawYields()):
					screen.setState("RawYieldsTrade0", not g_bYieldView)
					screen.show("RawYieldsTrade0")

					screen.setState("RawYieldsFood1", g_bYieldView and g_iYieldType == YieldTypes.YIELD_FOOD)
					screen.show("RawYieldsFood1")
					screen.setState("RawYieldsProduction2", g_bYieldView and g_iYieldType == YieldTypes.YIELD_PRODUCTION)
					screen.show("RawYieldsProduction2")
					screen.setState("RawYieldsCommerce3", g_bYieldView and g_iYieldType == YieldTypes.YIELD_COMMERCE)
					screen.show("RawYieldsCommerce3")

					screen.setState("RawYieldsWorkedTiles4", g_iYieldTiles == RawYields.WORKED_TILES)
					screen.show("RawYieldsWorkedTiles4")
					screen.setState("RawYieldsCityTiles5", g_iYieldTiles == RawYields.CITY_TILES)
					screen.show("RawYieldsCityTiles5")
					screen.setState("RawYieldsOwnedTiles6", g_iYieldTiles == RawYields.OWNED_TILES)
					screen.show("RawYieldsOwnedTiles6")
				else:
					screen.show( "TradeRouteListLabel" )
# BUG - Raw Yields - end

				for i in range( 3 ):
					szName = "BonusPane" + str(i)
					screen.show( szName )
					szName = "BonusBack" + str(i)
					screen.show( szName )

				i = 0
				iNumBuildings = 0
# BUG - Raw Yields - start
				self.yields = RawYields.Tracker()
# BUG - Raw Yields - end
				for i in range( gc.getNumBuildingInfos() ):
					if (pHeadSelectedCity.getNumBuilding(i) > 0):

						for k in range(pHeadSelectedCity.getNumBuilding(i)):

							szLeftBuffer = gc.getBuildingInfo(i).getDescription()
							szRightBuffer = u""
							bFirst = True

							if (pHeadSelectedCity.getNumActiveBuilding(i) > 0):
								iHealth = pHeadSelectedCity.getBuildingHealth(i)

								if (iHealth != 0):
									if ( bFirst == False ):
										szRightBuffer = szRightBuffer + ", "
									else:
										bFirst = False

									if ( iHealth > 0 ):
										szTempBuffer = u"+%d%c" %( iHealth, CyGame().getSymbolID(FontSymbols.HEALTHY_CHAR) )
										szRightBuffer = szRightBuffer + szTempBuffer
									else:
										szTempBuffer = u"+%d%c" %( -(iHealth), CyGame().getSymbolID(FontSymbols.UNHEALTHY_CHAR) )
										szRightBuffer = szRightBuffer + szTempBuffer

								iHappiness = pHeadSelectedCity.getBuildingHappiness(i)

								if (iHappiness != 0):
									if ( bFirst == False ):
										szRightBuffer = szRightBuffer + ", "
									else:
										bFirst = False

									if ( iHappiness > 0 ):
										szTempBuffer = u"+%d%c" %(iHappiness, CyGame().getSymbolID(FontSymbols.HAPPY_CHAR) )
										szRightBuffer = szRightBuffer + szTempBuffer
									else:
										szTempBuffer = u"+%d%c" %( -(iHappiness), CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR) )
										szRightBuffer = szRightBuffer + szTempBuffer

								for j in range( YieldTypes.NUM_YIELD_TYPES):
									iYield = gc.getBuildingInfo(i).getYieldChange(j) + pHeadSelectedCity.getNumBuilding(i) * pHeadSelectedCity.getBuildingYieldChange(gc.getBuildingInfo(i).getBuildingClassType(), j)

									if (iYield != 0):
										if ( bFirst == False ):
											szRightBuffer = szRightBuffer + ", "
										else:
											bFirst = False

										if ( iYield > 0 ):
											szTempBuffer = u"%s%d%c" %( "+", iYield, gc.getYieldInfo(j).getChar() )
											szRightBuffer = szRightBuffer + szTempBuffer
										else:
											szTempBuffer = u"%s%d%c" %( "", iYield, gc.getYieldInfo(j).getChar() )
											szRightBuffer = szRightBuffer + szTempBuffer

# BUG - Raw Yields - start
										self.yields.addBuilding(j, iYield)
# BUG - Raw Yields - end

							for j in range(CommerceTypes.NUM_COMMERCE_TYPES):
								iCommerce = pHeadSelectedCity.getBuildingCommerceByBuilding(j, i) / pHeadSelectedCity.getNumBuilding(i)

								if (iCommerce != 0):
									if ( bFirst == False ):
										szRightBuffer = szRightBuffer + ", "
									else:
										bFirst = False

									if ( iCommerce > 0 ):
										szTempBuffer = u"%s%d%c" %( "+", iCommerce, gc.getCommerceInfo(j).getChar() )
										szRightBuffer = szRightBuffer + szTempBuffer
									else:
										szTempBuffer = u"%s%d%c" %( "", iCommerce, gc.getCommerceInfo(j).getChar() )
										szRightBuffer = szRightBuffer + szTempBuffer

							szBuffer = szLeftBuffer + "  " + szRightBuffer

							screen.appendTableRow( "BuildingListTable" )
							screen.setTableText( "BuildingListTable", 0, iNumBuildings, "<font=1>" + szLeftBuffer + "</font>", "", WidgetTypes.WIDGET_HELP_BUILDING, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
							screen.setTableText( "BuildingListTable", 1, iNumBuildings, "<font=1>" + szRightBuffer + "</font>", "", WidgetTypes.WIDGET_HELP_BUILDING, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )

							iNumBuildings = iNumBuildings + 1

				if ( iNumBuildings > g_iNumBuildings ):
					g_iNumBuildings = iNumBuildings

				iNumTradeRoutes = 0

				for i in range(gc.getDefineINT("MAX_TRADE_ROUTES")):
					pLoopCity = pHeadSelectedCity.getTradeCity(i)

					if (pLoopCity and pLoopCity.getOwner() >= 0):
						player = gc.getPlayer(pLoopCity.getOwner())
						szLeftBuffer = u"<color=%d,%d,%d,%d>%s</color>" %(player.getPlayerTextColorR(), player.getPlayerTextColorG(), player.getPlayerTextColorB(), player.getPlayerTextColorA(), pLoopCity.getName() )
						szRightBuffer = u""

						for j in range( YieldTypes.NUM_YIELD_TYPES ):
							iTradeProfit = pHeadSelectedCity.calculateTradeYield(j, pHeadSelectedCity.calculateTradeProfitTimes100(pLoopCity)) #modified Sephi Better Traderoutes

							if (iTradeProfit != 0):
								if ( iTradeProfit > 0 ):
									szTempBuffer = u"%s%.2f%c" %( "+", float(iTradeProfit)/100, gc.getYieldInfo(j).getChar() )
									szRightBuffer = szRightBuffer + szTempBuffer
								else:
									szTempBuffer = u"%s%.2f%c" %( "", float(iTradeProfit)/100, gc.getYieldInfo(j).getChar() )
									szRightBuffer = szRightBuffer + szTempBuffer
# BUG - Raw Yields - start
								if (j == YieldTypes.YIELD_COMMERCE):
									if pHeadSelectedCity.getTeam() == pLoopCity.getTeam():
										self.yields.addDomesticTrade(iTradeProfit/100)
									elif pHeadSelectedCity.area().getID() == pLoopCity.area().getID():
										self.yields.addForeignTrade(iTradeProfit/100)
									else:
										self.yields.addForeignOverseasTrade(iTradeProfit/100)

						if (not bShowRawYields):
							screen.appendTableRow( "TradeRouteTable" )
							screen.setTableText( "TradeRouteTable", 0, iNumTradeRoutes, "<font=1>" + szLeftBuffer + "</font>", "", WidgetTypes.WIDGET_HELP_TRADE_ROUTE_CITY, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
							screen.setTableText( "TradeRouteTable", 1, iNumTradeRoutes, "<font=1>" + szRightBuffer + "</font>", "", WidgetTypes.WIDGET_HELP_TRADE_ROUTE_CITY, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )
# BUG - Raw Yields - end

						iNumTradeRoutes = iNumTradeRoutes + 1

				if ( iNumTradeRoutes > g_iNumTradeRoutes ):
					g_iNumTradeRoutes = iNumTradeRoutes

				i = 0
				iLeftCount = 0
				iCenterCount = 0
				iRightCount = 0

				for i in range( gc.getNumBonusInfos() ):
					bHandled = False
					if ( pHeadSelectedCity.hasBonus(i) ):

						iHealth = pHeadSelectedCity.getBonusHealth(i)
						iHappiness = pHeadSelectedCity.getBonusHappiness(i)
##--------	Advanced Rules: Added by Denev 2009/12/30	--------##
##	Cumulative resources show total of healthiness or happiness
#						if gc.getBonusInfo(i).isModifierPerBonus():
#							iNumBonuses = pHeadSelectedCity.getNumBonuses(i)
#							iHealth		*= iNumBonuses
#							iHappiness	*= iNumBonuses
##--------	Advanced Rules: End Add						--------##

						szBuffer = u""
						szLeadBuffer = u""

						szTempBuffer = u"<font=1>%c" %( gc.getBonusInfo(i).getChar() )
						szLeadBuffer = szLeadBuffer + szTempBuffer

						if (pHeadSelectedCity.getNumBonuses(i) > 1):
							szTempBuffer = u"(%d)" %( pHeadSelectedCity.getNumBonuses(i) )
							szLeadBuffer = szLeadBuffer + szTempBuffer

						szLeadBuffer = szLeadBuffer + "</font>"

						if (iHappiness != 0):
							if ( iHappiness > 0 ):
								szTempBuffer = u"<font=1>+%d%c</font>" %(iHappiness, CyGame().getSymbolID(FontSymbols.HAPPY_CHAR) )
							else:
								szTempBuffer = u"<font=1>+%d%c</font>" %( -iHappiness, CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR) )

							#SpyFanatic: show health bonus also in column 1
							#if ( iHealth > 0 ):
							#	szTempBuffer += u"<font=1>,+%d%c</font>" %( iHealth, CyGame().getSymbolID( FontSymbols.HEALTHY_CHAR ) )

							szName = "RightBonusItemLeft" + str(iRightCount)
							screen.setLabelAt( szName, "BonusBack2", szLeadBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, (iRightCount * 20) + 4, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, i, -1 )
							szName = "RightBonusItemRight" + str(iRightCount)
							#screen.setLabelAt( szName, "BonusBack2", szTempBuffer, CvUtil.FONT_RIGHT_JUSTIFY, 102, (iRightCount * 20) + 4, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, i, -1 )
							screen.setLabelAt( szName, "BonusBack2", szTempBuffer, CvUtil.FONT_RIGHT_JUSTIFY, 62, (iRightCount * 20) + 4, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, i, -1 )

							iRightCount = iRightCount + 1

							#bHandled = True
							if ( iHealth == 0 ):
								bHandled = True

						if (iHealth != 0 and bHandled == False):
							if ( iHealth > 0 ):
								szTempBuffer = u"<font=1>+%d%c</font>" %( iHealth, CyGame().getSymbolID( FontSymbols.HEALTHY_CHAR ) )
							else:
								szTempBuffer = u"<font=1>+%d%c</font>" %( -iHealth, CyGame().getSymbolID(FontSymbols.UNHEALTHY_CHAR) )

							szName = "CenterBonusItemLeft" + str(iCenterCount)
							screen.setLabelAt( szName, "BonusBack1", szLeadBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, (iCenterCount * 20) + 4, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, i, -1 )
							szName = "CenterBonusItemRight" + str(iCenterCount)
							screen.setLabelAt( szName, "BonusBack1", szTempBuffer, CvUtil.FONT_RIGHT_JUSTIFY, 62, (iCenterCount * 20) + 4, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, i, -1 )

							iCenterCount = iCenterCount + 1

							bHandled = True

						szBuffer = u""
						if ( not bHandled ):

							szName = "LeftBonusItem" + str(iLeftCount)
							screen.setLabelAt( szName, "BonusBack0", szLeadBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, (iLeftCount * 20) + 4, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, i, -1 )

							iLeftCount = iLeftCount + 1

							bHandled = True

				g_iNumLeftBonus = iLeftCount
				g_iNumCenterBonus = iCenterCount
				g_iNumRightBonus = iRightCount

				iMaintenance = pHeadSelectedCity.getMaintenanceTimes100()

				szBuffer = localText.getText("INTERFACE_CITY_MAINTENANCE", ())

#				screen.setLabel( "MaintenanceText", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 15, 126, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_MAINTENANCE, -1, -1 )
				screen.setLabel( "MaintenanceText", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 15, 111, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_MAINTENANCE, -1, -1 )
				screen.show( "MaintenanceText" )

				szBuffer = u"-%d.%02d %c" %(iMaintenance/100, iMaintenance%100, gc.getCommerceInfo(CommerceTypes.COMMERCE_GOLD).getChar())
#				screen.setLabel( "MaintenanceAmountText", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, 220, 125, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_MAINTENANCE, -1, -1 )
				screen.setLabel( "MaintenanceAmountText", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, 220, 110, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_MAINTENANCE, -1, -1 )
				screen.show( "MaintenanceAmountText" )

#added Sephi

#				szBuffer = localText.getText("TXT_KEY_INTERFACE_CITY_SPECIALIZATION", ())
#				screen.setLabel( "SpecializationText", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 15, 1, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_CITY_SPECIALIZATION, -1, -1 )
#				screen.show( "SpecializationText" )

#				szBuffer = localText.getText("TXT_KEY_INTERFACE_CITY_INFRASTRUCTURE", ())
#				screen.setLabel( "InfrastructureText", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 15, 130, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )

#				szBuffer = "InfrastructureButton"
#				screen.setButtonGFC( szBuffer, u"", "", 200, 130, 20, 20, WidgetTypes.WIDGET_HELP_INFRASTRUCTURE, -1, -1, ButtonStyles.BUTTON_STYLE_CITY_PLUS )
#				if gc.getPlayer(gc.getGame().getActivePlayer()).canIncreaseCityInfrastructureLevel(pHeadSelectedCity):
#					screen.show( "InfrastructureText" )
#					screen.show( szBuffer )
#				else:
#					screen.hide( "InfrastructureText" )
#					screen.hide( szBuffer )


# BUG - Raw Yields - start
				if (bShowRawYields):
					self.yields.processCity(pHeadSelectedCity)
					self.yields.fillTable(screen, "TradeRouteTable", g_iYieldType, g_iYieldTiles)
# BUG - Raw Yields - end

				szBuffer = u""

				for i in range(gc.getNumReligionInfos()):
					xCoord = xResolution - 242 + (i * 34)
					yCoord = 42

					bEnable = True

					if (pHeadSelectedCity.isHasReligion(i)):

#FfH: Added by Kael 11/03/2007
						if (gc.getPlayer(gc.getGame().getActivePlayer()).canSeeReligion(i)):
#FfH: End Add

							if (pHeadSelectedCity.isHolyCityByType(i)):
								szTempBuffer = u"%c" %(gc.getReligionInfo(i).getHolyCityChar())
								szName = "ReligionHolyCityDDS" + str(i)
								screen.show( szName )
							else:
								szTempBuffer = u"%c" %(gc.getReligionInfo(i).getChar())
							szBuffer = szBuffer + szTempBuffer

						j = 0
						for j in range(CommerceTypes.NUM_COMMERCE_TYPES):
							iCommerce = pHeadSelectedCity.getReligionCommerceByReligion(j, i)

							if (iCommerce != 0):
								if ( iCommerce > 0 ):
									szTempBuffer = u",%s%d%c" %("+", iCommerce, gc.getCommerceInfo(j).getChar() )
									szBuffer = szBuffer + szTempBuffer
								else:
									szTempBuffer = u",%s%d%c" %( "", iCommerce, gc.getCommerceInfo(j).getChar() )
									szBuffer = szBuffer + szTempBuffer

						iHappiness = pHeadSelectedCity.getReligionHappiness(i)

						if (iHappiness != 0):
							if ( iHappiness > 0 ):
								szTempBuffer = u",+%d%c" %(iHappiness, CyGame().getSymbolID(FontSymbols.HAPPY_CHAR) )
								szBuffer = szBuffer + szTempBuffer
							else:
								szTempBuffer = u",+%d%c" %(-(iHappiness), CyGame().getSymbolID(FontSymbols.UNHAPPY_CHAR) )
								szBuffer = szBuffer + szTempBuffer

						szBuffer = szBuffer + " "

						szButton = gc.getReligionInfo(i).getButton()

					else:

						bEnable = False
						szButton = gc.getReligionInfo(i).getButton()

					szName = "ReligionDDS" + str(i)
					screen.setImageButton( szName, szButton, xCoord, yCoord, 24, 24, WidgetTypes.WIDGET_HELP_RELIGION_CITY, i, -1 )
					screen.enable( szName, bEnable )
					screen.show( szName )

				for i in range(gc.getNumCorporationInfos()):
					xCoord = xResolution - 242 + (i * 34)
					yCoord = 66

					#added Sephi
					if i>6:	#line 2
						xCoord -=7*34
						yCoord += 20

					bEnable = True

					if (pHeadSelectedCity.isHasCorporation(i)):
						if (pHeadSelectedCity.isHeadquartersByType(i)):
							szTempBuffer = u"%c" %(gc.getCorporationInfo(i).getHeadquarterChar())
							szName = "CorporationHeadquarterDDS" + str(i)
							screen.show( szName )
						else:
							szTempBuffer = u"%c" %(gc.getCorporationInfo(i).getChar())
						szBuffer = szBuffer + szTempBuffer

						for j in range(YieldTypes.NUM_YIELD_TYPES):
							iYield = pHeadSelectedCity.getCorporationYieldByCorporation(j, i)

							if (iYield != 0):
								if ( iYield > 0 ):
									szTempBuffer = u",%s%d%c" %("+", iYield, gc.getYieldInfo(j).getChar() )
									szBuffer = szBuffer + szTempBuffer
								else:
									szTempBuffer = u",%s%d%c" %( "", iYield, gc.getYieldInfo(j).getChar() )
									szBuffer = szBuffer + szTempBuffer

						for j in range(CommerceTypes.NUM_COMMERCE_TYPES):
							iCommerce = pHeadSelectedCity.getCorporationCommerceByCorporation(j, i)

							if (iCommerce != 0):
								if ( iCommerce > 0 ):
									szTempBuffer = u",%s%d%c" %("+", iCommerce, gc.getCommerceInfo(j).getChar() )
									szBuffer = szBuffer + szTempBuffer
								else:
									szTempBuffer = u",%s%d%c" %( "", iCommerce, gc.getCommerceInfo(j).getChar() )
									szBuffer = szBuffer + szTempBuffer

						szBuffer += " "

						szButton = gc.getCorporationInfo(i).getButton()

					else:

						bEnable = False
						szButton = gc.getCorporationInfo(i).getButton()

					szName = "CorporationDDS" + str(i)
					screen.setImageButton( szName, szButton, xCoord, yCoord, 24, 24, WidgetTypes.WIDGET_HELP_CORPORATION_CITY, i, -1 )
					screen.enable( szName, bEnable )
					screen.show( szName )

				szBuffer = u"%d%% %s" %(pHeadSelectedCity.plot().calculateCulturePercent(pHeadSelectedCity.getOwner()), gc.getPlayer(pHeadSelectedCity.getOwner()).getCivilizationAdjective(0) )
				screen.setLabel( "NationalityText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, 125, yResolution - 210, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.setHitTest( "NationalityText", HitTestTypes.HITTEST_NOHIT )
				screen.show( "NationalityText" )
				iRemainder = 0
				iWhichBar = 0
				for h in range( gc.getMAX_PLAYERS() ):
					if ( gc.getPlayer(h).isAlive() ):
						fPercent = pHeadSelectedCity.plot().calculateCulturePercent(h)
						if ( fPercent != 0 ):
							fPercent = fPercent / 100.0
							screen.setStackedBarColorsRGB( "NationalityBar", iWhichBar, gc.getPlayer(h).getPlayerTextColorR(), gc.getPlayer(h).getPlayerTextColorG(), gc.getPlayer(h).getPlayerTextColorB(), gc.getPlayer(h).getPlayerTextColorA() )
							if ( iRemainder == 1 ):
								screen.setBarPercentage( "NationalityBar", iWhichBar, fPercent )
							else:
								screen.setBarPercentage( "NationalityBar", iWhichBar, fPercent / ( 1 - iRemainder ) )
							iRemainder += fPercent
							iWhichBar += 1
				screen.show( "NationalityBar" )

#Unblemished
#				screen.hide( "RevoltText" )
#				if (not CyInterface().isCityScreenUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_ADVANCED_START and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):
#				pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
#				if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_UNBLEMISHED'):
#
#					iAccord = pPlayer.getPurityCounter()
#					iRevoltBaseCity = CvUnblemishedEvents.UnblemishedFunctions().getRevoltBaseCity()
#
#					if iAccord > -1000:
#						SRstrR = u"<font=2i>%s%i%s</font>" %(str(" Vim Revolt "),(iRevoltBaseCity),str("% "))
#						screen.setText( "RevoltTextCity", "Background", SRstrR, CvUtil.FONT_LEFT_JUSTIFY, 115, 87, 0.5, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					else:
#						screen.hide( "RevoltTextCity" )
#/Unblemished

				iDefenseModifier = pHeadSelectedCity.getDefenseModifier(False)

				if (iDefenseModifier != 0):
					szBuffer = localText.getText("TXT_KEY_MAIN_CITY_DEFENSE", (CyGame().getSymbolID(FontSymbols.DEFENSE_CHAR), iDefenseModifier))

					if (pHeadSelectedCity.getDefenseDamage() > 0):
						szTempBuffer = u" (%d%%)" %( ( ( gc.getMAX_CITY_DEFENSE_DAMAGE() - pHeadSelectedCity.getDefenseDamage() ) * 100 ) / gc.getMAX_CITY_DEFENSE_DAMAGE() )
						szBuffer = szBuffer + szTempBuffer
					szNewBuffer = "<font=4>"
					szNewBuffer = szNewBuffer + szBuffer
					szNewBuffer = szNewBuffer + "</font>"
					screen.setLabel( "DefenseText", "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 270, 40, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_HELP_DEFENSE, -1, -1 )
					screen.show( "DefenseText" )

				if ( pHeadSelectedCity.getCultureLevel != CultureLevelTypes.NO_CULTURELEVEL ):
					iRate = pHeadSelectedCity.getCommerceRateTimes100(CommerceTypes.COMMERCE_CULTURE)
					if (iRate%100 == 0):
						szBuffer = localText.getText("INTERFACE_CITY_COMMERCE_RATE", (gc.getCommerceInfo(CommerceTypes.COMMERCE_CULTURE).getChar(), gc.getCultureLevelInfo(pHeadSelectedCity.getCultureLevel()).getTextKey(), iRate/100))
					else:
						szRate = u"+%d.%02d" % (iRate/100, iRate%100)
						szBuffer = localText.getText("INTERFACE_CITY_COMMERCE_RATE_FLOAT", (gc.getCommerceInfo(CommerceTypes.COMMERCE_CULTURE).getChar(), gc.getCultureLevelInfo(pHeadSelectedCity.getCultureLevel()).getTextKey(), szRate))

# BUG - Culture Turns - start
					if CityScreenOpt.isShowCultureTurns() and iRate > 0:
						iCultureTimes100 = pHeadSelectedCity.getCultureTimes100(pHeadSelectedCity.getOwner())
						iCultureLeftTimes100 = 100 * pHeadSelectedCity.getCultureThreshold() - iCultureTimes100
						szBuffer += u" " + localText.getText("INTERFACE_CITY_TURNS", (((iCultureLeftTimes100 + iRate - 1) / iRate),))
# BUG - Culture Turns - end

					screen.setLabel( "CultureText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, 125, yResolution - 184, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.setHitTest( "CultureText", HitTestTypes.HITTEST_NOHIT )
					screen.show( "CultureText" )

				if (pHeadSelectedCity.getNextGreatPersonInfo(True) != -1):
# BUG - Great Person Turns - start
						szBuffer = u"%s: %d%c" % (gc.getUnitInfo(pHeadSelectedCity.getNextGreatPersonInfo(true)).getDescription(),pHeadSelectedCity.getNextGreatPersonInfo(false) + 1,CyGame().getSymbolID(FontSymbols.GREAT_PEOPLE_CHAR))
						szBuffer = u"<font=2>%s</font>" % (szBuffer)
# BUG - Great Person Turns - end

						screen.setLabel( "GreatPeopleText", "Background", szBuffer, CvUtil.FONT_CENTER_JUSTIFY, xResolution - 126, yResolution - 182, -1.3, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
						screen.setHitTest( "GreatPeopleText", HitTestTypes.HITTEST_NOHIT )
						screen.show( "GreatPeopleText" )

						fThreshold = pHeadSelectedCity.getGreatPeopleThreshold() + float(gc.getPlayer( pHeadSelectedCity.getOwner() ).greatPeopleThreshold(False))
						nextGreatPerson = pHeadSelectedCity.getNextGreatPersonInfo(True)
						fRate = pHeadSelectedCity.getNextGreatPersonInfo(False)
						fFirst = float(pHeadSelectedCity.getGreatPeopleUnitProgress(nextGreatPerson)) / fThreshold

						screen.setBarPercentage( "GreatPeopleBar", InfoBarTypes.INFOBAR_STORED, fFirst )
						if ( fFirst == 1 ):
							screen.setBarPercentage( "GreatPeopleBar", InfoBarTypes.INFOBAR_RATE, 1 )
						else:
							screen.setBarPercentage( "GreatPeopleBar", InfoBarTypes.INFOBAR_RATE, 1 / float(fRate + 1) )

						screen.show( "GreatPeopleBar" )

				iFirst = float(pHeadSelectedCity.getCultureTimes100(pHeadSelectedCity.getOwner())) / float(100 * pHeadSelectedCity.getCultureThreshold())
				screen.setBarPercentage( "CultureBar", InfoBarTypes.INFOBAR_STORED, iFirst )
				if ( iFirst == 1 ):
					screen.setBarPercentage( "CultureBar", InfoBarTypes.INFOBAR_RATE, ( float(pHeadSelectedCity.getCommerceRate(CommerceTypes.COMMERCE_CULTURE)) / float(pHeadSelectedCity.getCultureThreshold()) ) )
				else:
					screen.setBarPercentage( "CultureBar", InfoBarTypes.INFOBAR_RATE, ( ( float(pHeadSelectedCity.getCommerceRate(CommerceTypes.COMMERCE_CULTURE)) / float(pHeadSelectedCity.getCultureThreshold()) ) ) / ( 1 - iFirst ) )
				screen.show( "CultureBar" )

		else:

			# Help Text Area
			if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):

#FfH: Modified by Kael 07/17/2008
#				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, 7, yResolution - 172, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, iHelpX, yResolution - 172, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )

#FfH: End Modify

			else:

#FfH: Modified by Kael 07/17/2008
#				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, 7, yResolution - 50, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, iHelpX, yResolution - 50, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
#FfH: End Modify

# Customized City Background start
			if CityScreenOpt.isDisableFlavor():
				screen.hide( "InterfaceCenterLeftBackgroundWidget" )
				screen.hide( "InterfaceCenterRightBackgroundWidget" )
				screen.hide( "InterfaceCityCenterBackgroundWidget" )

				screen.hide( "TopCityPanelLeft" )
				screen.hide( "TopCityPanelRight" )

				screen.hide( "InterfaceTopLeftBackgroundWidget" )
				screen.hide( "InterfaceTopRightBackgroundWidget" )
				screen.hide( "CityScreenTopWidget" )
				screen.hide( "CityNameBackground" )

				screen.hide( "CityScreenAdjustPanel" )

				screen.hide( "CityExtra1" )
				screen.hide( "InterfaceCityLeftBackgroundWidget" )
				screen.hide( "InterfaceCityRightBackgroundWidget" )
			else:

				screen.hide( "InterfaceCityBackgroundTopLeft" )
				screen.hide( "InterfaceCityBackgroundTopRight" )
				screen.hide( "InterfaceCityBackgroundTopCenter" )

				screen.hide( "TopCityPanelLeft" )
				screen.hide( "TopCityPanelRight" )

				screen.hide( "CityNameBackground" )

				screen.hide( "CityExtra1" )
				screen.hide( "InterfaceCityLeftBackgroundWidget" )
				screen.hide( "InterfaceCityRightBackgroundWidget" )

				screen.hide("InterfaceCityBackgroundTop")
				screen.hide("InterfaceCityBackgroundLeft")
				screen.hide("InterfaceCityBackgroundRight")
				screen.hide("InterfaceCityBackgroundBottom")


# Customized City Background End
			if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
				self.setMinimapButtonVisibility(True)

		return 0

	# Will update the info pane strings
	def updateInfoPaneStrings( self ):

		iRow = 0

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		pHeadSelectedCity = CyInterface().getHeadSelectedCity()
		pHeadSelectedUnit = CyInterface().getHeadSelectedUnit()

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		bShift = CyInterface().shiftKey()

#FfH: Modified by Kael 07/01/2007
#		screen.addPanel( "SelectedUnitPanel", u"", u"", True, False, 8, yResolution - 140, 280, 130, PanelStyles.PANEL_STYLE_STANDARD )
		screen.addPanel( "SelectedUnitPanel", u"", u"", True, False, 8, yResolution - 140, 140, 130, PanelStyles.PANEL_STYLE_STANDARD )
#FfH: End Modify

		screen.setStyle( "SelectedUnitPanel", "Panel_Game_HudStat_Style" )
		screen.hide( "SelectedUnitPanel" )

#FfH: Modified by Kael 07/01/2007
#		screen.addTableControlGFC( "SelectedUnitText", 3, 10, yResolution - 109, 183, 102, False, False, 32, 32, TableStyles.TABLE_STYLE_STANDARD )
		screen.addTableControlGFC( "SelectedUnitText", 3, 10, yResolution - 109, 250, 102, False, False, 32, 32, TableStyles.TABLE_STYLE_STANDARD )
#FfH: End Modify


		screen.setStyle( "SelectedUnitText", "Table_EmptyScroll_Style" )
		screen.hide( "SelectedUnitText" )
		screen.hide( "SelectedUnitLabel" )

		screen.addTableControlGFC( "SelectedCityText", 3, 10, yResolution - 139, 183, 128, False, False, 32, 32, TableStyles.TABLE_STYLE_STANDARD )
		screen.setStyle( "SelectedCityText", "Table_EmptyScroll_Style" )
		screen.hide( "SelectedCityText" )

		for i in range(gc.getNumPromotionInfos()):
			szName = "PromotionButton" + str(i)
			screen.hide( szName )

		if CyEngine().isGlobeviewUp():
			return

		if (pHeadSelectedCity):

			iOrders = CyInterface().getNumOrdersQueued()

			screen.setTableColumnHeader( "SelectedCityText", 0, u"", 121 )
			screen.setTableColumnHeader( "SelectedCityText", 1, u"", 54 )
			screen.setTableColumnHeader( "SelectedCityText", 2, u"", 10 )
			screen.setTableColumnRightJustify( "SelectedCityText", 1 )

			for i in range( iOrders ):

				szLeftBuffer = u""
				szRightBuffer = u""

				if ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_TRAIN ):
					szLeftBuffer = gc.getUnitInfo(CyInterface().getOrderNodeData1(i)).getDescription()
					szRightBuffer = "(" + str(pHeadSelectedCity.getUnitProductionTurnsLeft(CyInterface().getOrderNodeData1(i), i)) + ")"

					if (CyInterface().getOrderNodeSave(i)):
						szLeftBuffer = u"*" + szLeftBuffer

				elif ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_CONSTRUCT ):
					szLeftBuffer = gc.getBuildingInfo(CyInterface().getOrderNodeData1(i)).getDescription()
					szRightBuffer = "(" + str(pHeadSelectedCity.getBuildingProductionTurnsLeft(CyInterface().getOrderNodeData1(i), i)) + ")"

				elif ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_CREATE ):
					szLeftBuffer = gc.getProjectInfo(CyInterface().getOrderNodeData1(i)).getDescription()
					szRightBuffer = "(" + str(pHeadSelectedCity.getProjectProductionTurnsLeft(CyInterface().getOrderNodeData1(i), i)) + ")"

				elif ( CyInterface().getOrderNodeType(i) == OrderTypes.ORDER_MAINTAIN ):
					szLeftBuffer = gc.getProcessInfo(CyInterface().getOrderNodeData1(i)).getDescription()

				screen.appendTableRow( "SelectedCityText" )
				screen.setTableText( "SelectedCityText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
				screen.setTableText( "SelectedCityText", 1, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )
				screen.show( "SelectedCityText" )
				screen.show( "SelectedUnitPanel" )
				iRow += 1

		elif (pHeadSelectedUnit and CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW):

#FfH: Modified by Kael 07/17/2008
#			screen.setTableColumnHeader( "SelectedUnitText", 0, u"", 100 )
#			screen.setTableColumnHeader( "SelectedUnitText", 1, u"", 75 )
#			screen.setTableColumnHeader( "SelectedUnitText", 2, u"", 10 )
			screen.setTableColumnHeader( "SelectedUnitText", 0, u"", 80 )
			screen.setTableColumnHeader( "SelectedUnitText", 1, u"", 55 )
			screen.setTableColumnHeader( "SelectedUnitText", 2, u"", 100 )
#FfH: End Modify

			screen.setTableColumnRightJustify( "SelectedUnitText", 1 )

			if (CyInterface().mirrorsSelectionGroup()):
				pSelectedGroup = pHeadSelectedUnit.getGroup()
			else:
				pSelectedGroup = 0

			if (CyInterface().getLengthSelectionList() > 1):

# BUG - Stack Movement Display - start
				szBuffer = localText.getText("TXT_KEY_UNIT_STACK", (CyInterface().getLengthSelectionList(), ))
				if MainOpt.isShowStackMovementPoints():
					iMinMoves = 100000
					iMaxMoves = 0
					for i in range(CyInterface().getLengthSelectionList()):
						pUnit = CyInterface().getSelectionUnit(i)
						if (pUnit is not None):
							iLoopMoves = pUnit.movesLeft()
							if (iLoopMoves > iMaxMoves):
								iMaxMoves = iLoopMoves
							if (iLoopMoves < iMinMoves):
								iMinMoves = iLoopMoves
					if (iMinMoves == iMaxMoves):
						fMinMoves = float(iMinMoves) / gc.getMOVE_DENOMINATOR()
						szBuffer += u" %.1f%c" % (fMinMoves, CyGame().getSymbolID(FontSymbols.MOVES_CHAR))
					else:
						fMinMoves = float(iMinMoves) / gc.getMOVE_DENOMINATOR()
						fMaxMoves = float(iMaxMoves) / gc.getMOVE_DENOMINATOR()
						szBuffer += u" %.1f - %.1f%c" % (fMinMoves, fMaxMoves, CyGame().getSymbolID(FontSymbols.MOVES_CHAR))

				screen.setText( "SelectedUnitLabel", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 18, yResolution - 137, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_UNIT_NAME, -1, -1 )
# BUG - Stack Movement Display - end

				if ((pSelectedGroup == 0) or (pSelectedGroup.getLengthMissionQueue() <= 1)):
					if (pHeadSelectedUnit):
						for i in range(gc.getNumUnitInfos()):
							iCount = CyInterface().countEntities(i)

							if (iCount > 0):
								szRightBuffer = u""

								szLeftBuffer = gc.getUnitInfo(i).getDescription()

								if (iCount > 1):
									szRightBuffer = u"(" + str(iCount) + u")"

								szBuffer = szLeftBuffer + u"  " + szRightBuffer
								screen.appendTableRow( "SelectedUnitText" )
								screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
								screen.setTableText( "SelectedUnitText", 1, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )
								screen.show( "SelectedUnitText" )
								screen.show( "SelectedUnitPanel" )
								iRow += 1
			else:

				iNumRace = 0
				for i in range(gc.getNumPromotionInfos()):
					if pHeadSelectedUnit.isHasPromotion(i):
						if gc.getPromotionInfo(i).isRace():
							szName = "PromotionButton" + str(i)
							screen.moveItem( szName, 18 + iNumRace * 24, yResolution - 137, -0.3 )
							screen.moveToFront( szName )
							screen.show( szName )
							iNumRace += 1

				if (pHeadSelectedUnit.getHotKeyNumber() == -1):
					szBuffer = localText.getText("INTERFACE_PANE_UNIT_NAME", (pHeadSelectedUnit.getName(), ))
				else:
					szBuffer = localText.getText("INTERFACE_PANE_UNIT_NAME_HOT_KEY", (pHeadSelectedUnit.getHotKeyNumber(), pHeadSelectedUnit.getName()))
				if (len(szBuffer) > 60):
					szBuffer = "<font=2>" + szBuffer + "</font>"
				screen.setText( "SelectedUnitLabel", "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 18 + 24 * iNumRace, yResolution - 137, -0.1, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_UNIT_NAME, -1, -1 )

				if ((pSelectedGroup == 0) or (pSelectedGroup.getLengthMissionQueue() <= 1)):
					screen.show( "SelectedUnitText" )
					screen.show( "SelectedUnitPanel" )

					szBuffer = u""

					szLeftBuffer = u""
					szRightBuffer = u""

					if (pHeadSelectedUnit.getDomainType() == DomainTypes.DOMAIN_AIR):
						if (pHeadSelectedUnit.airBaseCombatStr() > 0):
							szLeftBuffer = localText.getText("INTERFACE_PANE_AIR_STRENGTH", ())
							if (pHeadSelectedUnit.isFighting()):
								szRightBuffer = u"?/%d%c" %(pHeadSelectedUnit.airBaseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
							elif (pHeadSelectedUnit.isHurt()):
								szRightBuffer = u"%.1f/%d%c" %(((float(pHeadSelectedUnit.airBaseCombatStr() * pHeadSelectedUnit.currHitPoints())) / (float(pHeadSelectedUnit.maxHitPoints()))), pHeadSelectedUnit.airBaseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
							else:
								szRightBuffer = u"%d%c" %(pHeadSelectedUnit.airBaseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
					else:
						if (pHeadSelectedUnit.canFight()):
							szLeftBuffer = localText.getText("INTERFACE_PANE_STRENGTH", ())
##BUGFfH: Added by Denev 2009/09/08
							#wounded color
							iDmgLv = max([0, min([3, 4 * pHeadSelectedUnit.getDamage() // pHeadSelectedUnit.maxHitPoints()])])
							iDmgColor = gc.getInfoTypeForString(
							[
								"COLOR_WHITE",
								"COLOR_WHITE",
								"COLOR_PLAYER_LIGHT_ORANGE_TEXT",
								"COLOR_WARNING_TEXT"
							][iDmgLv])
##BUGFfH: End Add
							if (pHeadSelectedUnit.isFighting()):
##BUGFfH: Modified by Denev 2009/09/08
##								szRightBuffer = u"?/%d%c" %(pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
								if (pHeadSelectedUnit.baseCombatStr() == pHeadSelectedUnit.baseCombatStrDefense()):
									szRightBuffer = u"?(%d)%c" % (pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
								else:
									szRightBuffer = u"?/?(%d/%d)%c" % (pHeadSelectedUnit.baseCombatStr(), pHeadSelectedUnit.baseCombatStrDefense(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
##BUGFfH: End Modify

#FfH: Modified by Kael 08/18/2007
#							elif (pHeadSelectedUnit.isHurt()):
#								szRightBuffer = u"%.1f/%d%c" %(((float(pHeadSelectedUnit.baseCombatStr() * pHeadSelectedUnit.currHitPoints())) / (float(pHeadSelectedUnit.maxHitPoints()))), pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
#							else:
#								szRightBuffer = u"%d%c" %(pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
##BUGFfH: Modified by Denev 2009/09/08
##							elif (pHeadSelectedUnit.isHurt()):
##								if pHeadSelectedUnit.baseCombatStr() == pHeadSelectedUnit.baseCombatStrDefense():
##									szRightBuffer = u"%.1f/%d%c" %(((float(pHeadSelectedUnit.baseCombatStr() * pHeadSelectedUnit.currHitPoints())) / (float(pHeadSelectedUnit.maxHitPoints()))), pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
##								else:
##									szRightBuffer = u"%.1f/%.lf%c" %(((float(pHeadSelectedUnit.baseCombatStr() * pHeadSelectedUnit.currHitPoints())) / (float(pHeadSelectedUnit.maxHitPoints()))), ((float(pHeadSelectedUnit.baseCombatStrDefense() * pHeadSelectedUnit.currHitPoints())) / (float(pHeadSelectedUnit.maxHitPoints()))), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
##							else:
##								if pHeadSelectedUnit.baseCombatStr() == pHeadSelectedUnit.baseCombatStrDefense():
##									szRightBuffer = u"%d%c" %(pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
##								else:
##									szRightBuffer = u"%d/%d%c" %(pHeadSelectedUnit.baseCombatStr(), pHeadSelectedUnit.baseCombatStrDefense(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
							elif (pHeadSelectedUnit.baseCombatStr() == pHeadSelectedUnit.baseCombatStrDefense()):
								if (pHeadSelectedUnit.isHurt()):
									fAttStr = float(pHeadSelectedUnit.baseCombatStr() * pHeadSelectedUnit.currHitPoints()) / pHeadSelectedUnit.maxHitPoints()
									szRightBuffer = localText.changeTextColor(u"%.1f" % fAttStr, iDmgColor) + u"(%d)%c" % (pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
								else:
									szRightBuffer = u"%d%c" % (pHeadSelectedUnit.baseCombatStr(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
							else:
								if (pHeadSelectedUnit.isHurt()):
									fAttStr = float(pHeadSelectedUnit.baseCombatStr() * pHeadSelectedUnit.currHitPoints()) / pHeadSelectedUnit.maxHitPoints()
									fDefStr = float(pHeadSelectedUnit.baseCombatStrDefense() * pHeadSelectedUnit.currHitPoints()) / pHeadSelectedUnit.maxHitPoints()
#									szRightBuffer = localText.changeTextColor(u"%.1f" % fAttStr, iDmgColor) + u"/"\
#												  + localText.changeTextColor(u"%.1f" % fDefStr, iDmgColor)\
#												  + u"(%d/%d)" % (pHeadSelectedUnit.baseCombatStr(), pHeadSelectedUnit.baseCombatStrDefense())\
#												  + u"%c" % CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR)
									szRightBuffer = localText.changeTextColor(u"%.1f" % fAttStr, iDmgColor) + u"/"\
												  + localText.changeTextColor(u"%.1f" % fDefStr, iDmgColor)\
												  + u"%c" % CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR)
								else:
									szRightBuffer = u"%d/%d%c" % (pHeadSelectedUnit.baseCombatStr(), pHeadSelectedUnit.baseCombatStrDefense(), CyGame().getSymbolID(FontSymbols.STRENGTH_CHAR))
#FfH: End Modify

					szBuffer = szLeftBuffer + szRightBuffer
					if ( szRightBuffer ):
						screen.appendTableRow( "SelectedUnitText" )
#						screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.setTableText( "SelectedUnitText", 0, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.show( "SelectedUnitText" )
						screen.show( "SelectedUnitPanel" )
#						iRow += 1

					szLeftBuffer = u""
					szRightBuffer = u""

# BUG - Unit Movement Fraction - start
					szLeftBuffer = localText.getText("INTERFACE_PANE_MOVEMENT", ())
					if MainOpt.isShowUnitMovementPointsFraction():
						szRightBuffer = u"%d%c" %(pHeadSelectedUnit.baseMoves(), CyGame().getSymbolID(FontSymbols.MOVES_CHAR))
						if (pHeadSelectedUnit.movesLeft() == 0):
							szRightBuffer = u"0/" + szRightBuffer
						elif (pHeadSelectedUnit.movesLeft() == pHeadSelectedUnit.baseMoves() * gc.getMOVE_DENOMINATOR()):
							pass
						else:
							fCurrMoves = float(pHeadSelectedUnit.movesLeft()) / gc.getMOVE_DENOMINATOR()
							szRightBuffer = (u"%.1f/" % fCurrMoves) + szRightBuffer
					else:
						if ( (pHeadSelectedUnit.movesLeft() % gc.getMOVE_DENOMINATOR()) > 0 ):
							iDenom = 1
						else:
							iDenom = 0
						iCurrMoves = ((pHeadSelectedUnit.movesLeft() / gc.getMOVE_DENOMINATOR()) + iDenom )
						if (pHeadSelectedUnit.baseMoves() == iCurrMoves):
							szRightBuffer = u"%d%c" %(pHeadSelectedUnit.baseMoves(), CyGame().getSymbolID(FontSymbols.MOVES_CHAR) )
						else:
							szRightBuffer = u"%d/%d%c" %(iCurrMoves, pHeadSelectedUnit.baseMoves(), CyGame().getSymbolID(FontSymbols.MOVES_CHAR) )
# BUG - Unit Movement Fraction - end

					szBuffer = szLeftBuffer + "  " + szRightBuffer
					screen.appendTableRow( "SelectedUnitText" )
#					screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
					screen.setTableText( "SelectedUnitText", 1, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_RIGHT_JUSTIFY )
					screen.show( "SelectedUnitText" )
					screen.show( "SelectedUnitPanel" )

#					if (pHeadSelectedUnit.getLevel() > 0):
#
#						szLeftBuffer = localText.getText("INTERFACE_PANE_LEVEL", ())
#						szRightBuffer = u"%d" %(pHeadSelectedUnit.getLevel())
#
#						szBuffer = szLeftBuffer + "  " + szRightBuffer
#						screen.appendTableRow( "SelectedUnitText" )
#						screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
#						screen.setTableText( "SelectedUnitText", 2, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
#						screen.show( "SelectedUnitText" )
#						screen.show( "SelectedUnitPanel" )
#						iRow += 1

					if ((pHeadSelectedUnit.getExperience() > 0) and not pHeadSelectedUnit.isFighting()):
						szLeftBuffer = localText.getText("INTERFACE_PANE_EXPERIENCE", ())
						szRightBuffer = u"Exp (%d/%d)" %(pHeadSelectedUnit.getExperience(), pHeadSelectedUnit.experienceNeeded())
						szBuffer = szLeftBuffer + "  " + szRightBuffer
						screen.appendTableRow( "SelectedUnitText" )
#						screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.setTableText( "SelectedUnitText", 2, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.show( "SelectedUnitText" )
						screen.show( "SelectedUnitPanel" )

					iRow += 1
					bAnyEquipment = false
					for i in range(gc.getNumPromotionInfos()):
						if gc.getPromotionInfo(i).isDisplayAsEquipment():
							if pHeadSelectedUnit.isHasPromotion(i):
								bAnyEquipment = true

					if bAnyEquipment:
						szLeftBuffer = u"Eq: " %()
						screen.appendTableRow( "SelectedUnitText" )
						screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.show( "SelectedUnitText" )
						screen.show( "SelectedUnitPanel" )
						iEquipmentCount = 0
						for i in range(gc.getNumPromotionInfos()):
							if not gc.getPromotionInfo(i).isRace():
								if gc.getPromotionInfo(i).isDisplayAsEquipment():
									iPromNext = gc.getPromotionInfo(i).getPromotionNextLevel()
									if (pHeadSelectedUnit.isHasPromotion(i) and (iPromNext == -1 or pHeadSelectedUnit.isHasPromotion(iPromNext) == False)):
										szName = "PromotionButton" + str(i)
										screen.moveItem( szName, 38 + iEquipmentCount * 24, yResolution - 108 + iRow * 24, -0.3 )
										screen.moveToFront( szName )
										screen.show( szName )
										iEquipmentCount += 1

						iRow += 1

					bAnyClass = false
					for i in range(gc.getNumPromotionInfos()):
						if gc.getPromotionInfo(i).isDisplayAsClass():
							if pHeadSelectedUnit.isHasPromotion(i):
								bAnyClass = true

					if bAnyClass:
						szLeftBuffer = u"Cl: " %()
#						screen.appendTableRow( "SelectedUnitText" )
						screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, -1, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.show( "SelectedUnitText" )
						screen.show( "SelectedUnitPanel" )
						iEquipmentCount = 0
						for i in range(gc.getNumPromotionInfos()):
							if not gc.getPromotionInfo(i).isRace():
								if not gc.getPromotionInfo(i).isDisplayAsEquipment():
									if gc.getPromotionInfo(i).isDisplayAsClass():
										iPromNext = gc.getPromotionInfo(i).getPromotionNextLevel()
										if (pHeadSelectedUnit.isHasPromotion(i) and (iPromNext == -1 or pHeadSelectedUnit.isHasPromotion(iPromNext) == False)):
											szName = "PromotionButton" + str(i)
											screen.moveItem( szName, 38 + iEquipmentCount * 24, yResolution - 108 + iRow * 24, -0.3 )
											screen.moveToFront( szName )
											screen.show( szName )
											iEquipmentCount += 1

						iRow += 1

					iPromotionCount = 0
					i = 0
					for i in range(gc.getNumPromotionInfos()):
						if not gc.getPromotionInfo(i).isRace():
							if not gc.getPromotionInfo(i).isDisplayAsEquipment():
								if not gc.getPromotionInfo(i).isDisplayAsClass():
									iPromNext = gc.getPromotionInfo(i).getPromotionNextLevel()
									if (pHeadSelectedUnit.isHasPromotion(i) and (iPromNext == -1 or pHeadSelectedUnit.isHasPromotion(iPromNext) == False)):
										szName = "PromotionButton" + str(i)
										self.setPromotionButtonPosition( szName, iPromotionCount )
										screen.moveToFront( szName )
										screen.show( szName )

										iPromotionCount = iPromotionCount + 1

			if (pSelectedGroup):

				iNodeCount = pSelectedGroup.getLengthMissionQueue()

				if (iNodeCount > 1):
					for i in range( iNodeCount ):
						szLeftBuffer = u""
						szRightBuffer = u""

						if (gc.getMissionInfo(pSelectedGroup.getMissionType(i)).isBuild()):
							if (i == 0):
								szLeftBuffer = gc.getBuildInfo(pSelectedGroup.getMissionData1(i)).getDescription()
								szRightBuffer = localText.getText("INTERFACE_CITY_TURNS", (pSelectedGroup.plot().getBuildTurnsLeft(pSelectedGroup.getMissionData1(i), 0, 0), ))
							else:
								szLeftBuffer = u"%s..." %(gc.getBuildInfo(pSelectedGroup.getMissionData1(i)).getDescription())
						else:
							szLeftBuffer = u"%s..." %(gc.getMissionInfo(pSelectedGroup.getMissionType(i)).getDescription())

						szBuffer = szLeftBuffer + "  " + szRightBuffer
						screen.appendTableRow( "SelectedUnitText" )
						screen.setTableText( "SelectedUnitText", 0, iRow, szLeftBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_LEFT_JUSTIFY )
						screen.setTableText( "SelectedUnitText", 1, iRow, szRightBuffer, "", WidgetTypes.WIDGET_HELP_SELECTED, i, -1, CvUtil.FONT_RIGHT_JUSTIFY )
						screen.show( "SelectedUnitText" )
						screen.show( "SelectedUnitPanel" )
						iRow += 1

		return 0

	# Will update the scores
	def updateScoreStrings( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		screen.hide( "ScoreBackground" )

		for i in range( gc.getMAX_PLAYERS() ):
			szName = "ScoreText" + str(i)
			screen.hide( szName )
			szName = "ScoreTech" + str(i)
			screen.hide( szName )
			for j in range( Scoreboard.NUM_PARTS ):
				szName = "ScoreText%d-%d" %( i, j )
				screen.hide( szName )

#FfH Global Counter: Added by Kael 08/12/2007
		if CyGame().getWBMapScript():
			szName = "GoalTag"
			screen.hide( szName )
		szName = "CutLosersTag"
		screen.hide( szName )
		szName = "DifficultyTag"
		screen.hide( szName )
		szName = "HighToLowTag"
		screen.hide( szName )
		szName = "DisableProductionTag"
		screen.hide( szName )
		szName = "DisableResearchTag"
		screen.hide( szName )
		szName = "DisableSpellcastingTag"
		screen.hide( szName )
#FfH: End Add

		if not CyInterface().isCityScreenUp():
			szName = "InfrastructureButton"
			screen.hide( szName )

		iWidth = 0
		iCount = 0
		iBtnHeight = 22

		if ((CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY)):
			if (CyInterface().isScoresVisible() and not CyInterface().isCityScreenUp() and CyEngine().isGlobeviewUp() == false):

# BUG - Align Icons - start
				bAlignIcons = ScoreOpt.isAlignIcons()
				if (bAlignIcons):
					scores = Scoreboard.Scoreboard()
# BUG - Align Icons - end

# BUG - 3.17 No Espionage - start
				bNoEspionage = BugUtil.isNoEspionage()
# BUG - 3.17 No Espionage - end

# BUG - Power Rating - start
				bShowPower = ScoreOpt.isShowPower()
				if (bShowPower):
					iPlayerPower = gc.getActivePlayer().getPower()
					iPowerColor = ScoreOpt.getPowerColor()
					iHighPowerColor = ScoreOpt.getHighPowerColor()
					iLowPowerColor = ScoreOpt.getLowPowerColor()

					if (not bNoEspionage):
						iDemographicsMission = -1
						for iMissionLoop in range(gc.getNumEspionageMissionInfos()):
							if (gc.getEspionageMissionInfo(iMissionLoop).isSeeDemographics()):
								iDemographicsMission = iMissionLoop
								break
						if (iDemographicsMission == -1):
							bShowPower = False
# BUG - Power Rating - end

				i = gc.getMAX_CIV_TEAMS() - 1
				while (i > -1):
					eTeam = gc.getGame().getRankTeam(i)

					if (gc.getTeam(gc.getGame().getActiveTeam()).isHasMet(eTeam) or gc.getTeam(eTeam).isHuman() or gc.getGame().isDebugMode()):
# BUG - Align Icons - start
						if (bAlignIcons):
							scores.addTeam(gc.getTeam(eTeam), i)
# BUG - Align Icons - end
						j = gc.getMAX_CIV_PLAYERS() - 1
						while (j > -1):
							ePlayer = gc.getGame().getRankPlayer(j)

							if (not CyInterface().isScoresMinimized() or gc.getGame().getActivePlayer() == ePlayer):
# BUG - Dead Civs - start
								if (gc.getPlayer(ePlayer).isEverAlive() and not gc.getPlayer(ePlayer).isBarbarian()
									and (gc.getPlayer(ePlayer).isAlive() or ScoreOpt.isShowDeadCivs())):
# BUG - Dead Civs - end
# BUG - Minor Civs - start
									if (not gc.getPlayer(ePlayer).isMinorCiv() or ScoreOpt.isShowMinorCivs()):
# BUG - Minor Civs - end
										if (gc.getPlayer(ePlayer).getTeam() == eTeam):
											szBuffer = u"<font=2>"
# BUG - Align Icons - start
											if (bAlignIcons):
												scores.addPlayer(gc.getPlayer(ePlayer), j)
												# BUG: Align Icons continues throughout -- if (bAlignIcons): scores.setFoo(foo)
# BUG - Align Icons - end

											if (gc.getGame().isGameMultiPlayer()):
												if (not (gc.getPlayer(ePlayer).isTurnActive())):
													szBuffer = szBuffer + "*"
													if (bAlignIcons):
														scores.setWaiting()

# BUG - Dead Civs - start
											if (ScoreOpt.isUsePlayerName()):
												szPlayerName = gc.getPlayer(ePlayer).getName()
											else:
												szPlayerName = gc.getLeaderHeadInfo(gc.getPlayer(ePlayer).getLeaderType()).getDescription()
											szCivName = gc.getPlayer(ePlayer).getCivilizationShortDescription(0)
											if (ScoreOpt.isShowBothNames()):
												szPlayerName = szPlayerName + "/" + szCivName
											elif (ScoreOpt.isShowLeaderName()):
												szPlayerName = szPlayerName
											else:
												szPlayerName = szCivName

											#display City States
											if gc.getPlayer(ePlayer).getCityStateType()!=-1:
												szPlayerName = gc.getCityStateInfo(gc.getPlayer(ePlayer).getCityStateType()).getDescription()

											if (not gc.getPlayer(ePlayer).isAlive() and ScoreOpt.isShowDeadTag()):
												szPlayerScore = localText.getText("TXT_KEY_BUG_DEAD_CIV", ())
												if (bAlignIcons):
													scores.setScore(szPlayerScore)
											else:
												iScore = gc.getGame().getPlayerScore(ePlayer)
												szPlayerScore = u"%d" % iScore
												if (bAlignIcons):
													scores.setScore(szPlayerScore)
# BUG - Score Delta - start
												if (ScoreOpt.isShowScoreDelta()):
													iGameTurn = gc.getGame().getGameTurn()
													if (ePlayer >= gc.getGame().getActivePlayer()):
														iGameTurn -= 1
													if (ScoreOpt.isScoreDeltaIncludeCurrentTurn()):
														iScoreDelta = iScore
													elif (iGameTurn >= 0):
														iScoreDelta = gc.getPlayer(ePlayer).getScoreHistory(iGameTurn)
													else:
														iScoreDelta = 0
													iPrevGameTurn = iGameTurn - 1
													if (iPrevGameTurn >= 0):
														iScoreDelta -= gc.getPlayer(ePlayer).getScoreHistory(iPrevGameTurn)
													if (iScoreDelta != 0):
														if (iScoreDelta > 0):
															iColorType = gc.getInfoTypeForString("COLOR_GREEN")
														elif (iScoreDelta < 0):
															iColorType = gc.getInfoTypeForString("COLOR_RED")
														szScoreDelta = "%+d" % iScoreDelta
														if (iColorType >= 0):
															szScoreDelta = localText.changeTextColor(szScoreDelta, iColorType)
														szPlayerScore += szScoreDelta + u" "
														if (bAlignIcons):
															scores.setScoreDelta(szScoreDelta)
# BUG - Score Delta - end

											if (not CyInterface().isFlashingPlayer(ePlayer) or CyInterface().shouldFlash(ePlayer)):
												if (ePlayer == gc.getGame().getActivePlayer()):
													szPlayerName = u"[<color=%d,%d,%d,%d>%s</color>]" %(gc.getPlayer(ePlayer).getPlayerTextColorR(), gc.getPlayer(ePlayer).getPlayerTextColorG(), gc.getPlayer(ePlayer).getPlayerTextColorB(), gc.getPlayer(ePlayer).getPlayerTextColorA(), szPlayerName)
												else:
													if (not gc.getPlayer(ePlayer).isAlive() and ScoreOpt.isGreyOutDeadCivs()):
														szPlayerName = u"<color=%d,%d,%d,%d>%s</color>" %(175, 175, 175, gc.getPlayer(ePlayer).getPlayerTextColorA(), szPlayerName)
													else:
														szPlayerName = u"<color=%d,%d,%d,%d>%s</color>" %(gc.getPlayer(ePlayer).getPlayerTextColorR(), gc.getPlayer(ePlayer).getPlayerTextColorG(), gc.getPlayer(ePlayer).getPlayerTextColorB(), gc.getPlayer(ePlayer).getPlayerTextColorA(), szPlayerName)
											szTempBuffer = u"%s: %s" %(szPlayerScore, szPlayerName)

											if (gc.getPlayer(ePlayer).isAlive()):
												if (gc.getTeam(eTeam).isAlive()):
													if (gc.getTeam(eTeam).isAtWar(gc.getGame().getActiveTeam())):
														szTempBuffer = "("  + localText.getColorText("TXT_KEY_CONCEPT_WAR", (), gc.getInfoTypeForString("COLOR_RED")).upper() + ")"
														szPlayerName = szPlayerName + szTempBuffer


#FfH Alignment: Added by Kael 08/09/2007
											if gc.getPlayer(ePlayer).getAlignment() == gc.getInfoTypeForString('ALIGNMENT_EVIL'):
												szTempBuffer = " (" + localText.getColorText("TXT_KEY_ALIGNMENT_EVIL", (), gc.getInfoTypeForString("COLOR_RED")) + ")  "
											if gc.getPlayer(ePlayer).getAlignment() == gc.getInfoTypeForString('ALIGNMENT_NEUTRAL'):
												szTempBuffer = " (" + localText.getColorText("TXT_KEY_ALIGNMENT_NEUTRAL", (), gc.getInfoTypeForString("COLOR_GREY")) + ")  "
											if gc.getPlayer(ePlayer).getAlignment() == gc.getInfoTypeForString('ALIGNMENT_GOOD'):
												szTempBuffer = " (" + localText.getColorText("TXT_KEY_ALIGNMENT_GOOD", (), gc.getInfoTypeForString("COLOR_YELLOW")) + ")  "
											szPlayerName = szPlayerName + szTempBuffer
#FfH: End Add

											if (bAlignIcons):
												scores.setName(szPlayerName)
												scores.setID(u"<color=%d,%d,%d,%d>%d</color>" %(gc.getPlayer(ePlayer).getPlayerTextColorR(), gc.getPlayer(ePlayer).getPlayerTextColorG(), gc.getPlayer(ePlayer).getPlayerTextColorB(), gc.getPlayer(ePlayer).getPlayerTextColorA(), ePlayer))

											if (gc.getPlayer(ePlayer).isAlive()):
												if (bAlignIcons):
													scores.setAlive()
												# BUG: Rest of Dead Civs change is merely indentation by 1 level ...
												if (gc.getTeam(eTeam).isAlive()):
													if ( not (gc.getTeam(gc.getGame().getActiveTeam()).isHasMet(eTeam)) ):
														szBuffer = szBuffer + (" ?")
														if (bAlignIcons):
															scores.setNotMet()
													if (gc.getTeam(eTeam).isAtWar(gc.getGame().getActiveTeam())):
														szTempBuffer = szBuffer + "("  + localText.getColorText("TXT_KEY_CONCEPT_WAR", (), gc.getInfoTypeForString("COLOR_RED")).upper() + ")"
														if (bAlignIcons):
															scores.setWar()
													elif (gc.getTeam(gc.getGame().getActiveTeam()).isForcePeace(eTeam)):
														if (bAlignIcons):
															scores.setPeace()
													elif (gc.getTeam(eTeam).isAVassal()):
														for iOwnerTeam in range(gc.getMAX_TEAMS()):
															if (gc.getTeam(eTeam).isVassal(iOwnerTeam) and gc.getTeam(gc.getGame().getActiveTeam()).isForcePeace(iOwnerTeam)):
																if (bAlignIcons):
																	scores.setPeace()
																break
#													if (PlayerUtil.isWHEOOH(ePlayer, PlayerUtil.getActivePlayerID())):
#														szTempBuffer = u"%c" %(CyGame().getSymbolID(FontSymbols.OCCUPATION_CHAR))
#														szBuffer = szBuffer + szTempBuffer
#														if (bAlignIcons):
#															scores.setWHEOOH()
													if (gc.getPlayer(ePlayer).canTradeNetworkWith(gc.getGame().getActivePlayer()) and (ePlayer != gc.getGame().getActivePlayer())):
														szTempBuffer = u"%c" %(CyGame().getSymbolID(FontSymbols.TRADE_CHAR))
														szBuffer = szBuffer + szTempBuffer
														if (bAlignIcons):
															scores.setTrade()
													if (gc.getTeam(eTeam).isOpenBorders(gc.getGame().getActiveTeam())):
														szTempBuffer = u"%c" %(CyGame().getSymbolID(FontSymbols.OPEN_BORDERS_CHAR))
														szBuffer = szBuffer + szTempBuffer
														if (bAlignIcons):
															scores.setBorders()
													if (gc.getTeam(eTeam).isDefensivePact(gc.getGame().getActiveTeam())):
														szTempBuffer = u"%c" %(CyGame().getSymbolID(FontSymbols.DEFENSIVE_PACT_CHAR))
														szBuffer = szBuffer + szTempBuffer
														if (bAlignIcons):
															scores.setPact()
													if (gc.getPlayer(ePlayer).getStateReligion() != -1):
														if (gc.getPlayer(ePlayer).hasHolyCity(gc.getPlayer(ePlayer).getStateReligion())):
															szTempBuffer = u"%c" %(gc.getReligionInfo(gc.getPlayer(ePlayer).getStateReligion()).getHolyCityChar())
														else:
															szTempBuffer = u"%c" %(gc.getReligionInfo(gc.getPlayer(ePlayer).getStateReligion()).getChar())
														szBuffer = szBuffer + szTempBuffer
														if (bAlignIcons):
															scores.setReligion(szTempBuffer)

#													if (not bNoEspionage and gc.getTeam(eTeam).getEspionagePointsAgainstTeam(gc.getGame().getActiveTeam()) < gc.getTeam(gc.getGame().getActiveTeam()).getEspionagePointsAgainstTeam(eTeam)):
#														szTempBuffer = u"%c" %(gc.getCommerceInfo(CommerceTypes.COMMERCE_ESPIONAGE).getChar())
#														szBuffer = szBuffer + szTempBuffer
#														if (bAlignIcons):
#															scores.setEspionage()

												bEspionageCanSeeResearch = False
												if (not bNoEspionage):
													for iMissionLoop in range(gc.getNumEspionageMissionInfos()):
														if (gc.getEspionageMissionInfo(iMissionLoop).isSeeResearch()):
															bEspionageCanSeeResearch = gc.getActivePlayer().canDoEspionageMission(iMissionLoop, ePlayer, None, -1)
															break

												if (((gc.getPlayer(ePlayer).getTeam() == gc.getGame().getActiveTeam()) and (gc.getTeam(gc.getGame().getActiveTeam()).getNumMembers() > 1)) or (gc.getTeam(gc.getPlayer(ePlayer).getTeam()).isVassal(gc.getGame().getActiveTeam())) or gc.getGame().isDebugMode() or bEspionageCanSeeResearch):
													if (gc.getPlayer(ePlayer).getCurrentResearch() != -1):
														szTempBuffer = u"-%s (%d)" %(gc.getTechInfo(gc.getPlayer(ePlayer).getCurrentResearch()).getDescription(), gc.getPlayer(ePlayer).getResearchTurnsLeft(gc.getPlayer(ePlayer).getCurrentResearch(), True))
														szBuffer = szBuffer + szTempBuffer
														if (bAlignIcons):
															scores.setResearch(gc.getPlayer(ePlayer).getCurrentResearch(), gc.getPlayer(ePlayer).getResearchTurnsLeft(gc.getPlayer(ePlayer).getCurrentResearch(), True))

# BUG - Power Rating - start
												# if on, show according to espionage "see demographics" mission
												if (bShowPower
													and (gc.getGame().getActivePlayer() != ePlayer
														 and (bNoEspionage or gc.getActivePlayer().canDoEspionageMission(iDemographicsMission, ePlayer, None, -1)))):
													iPower = gc.getPlayer(ePlayer).getPower()
													if (iPower > 0): # avoid divide by zero
														fPowerRatio = float(iPlayerPower) / float(iPower)
														if (ScoreOpt.isPowerThemVersusYou()):
															if (fPowerRatio > 0):
																fPowerRatio = 1.0 / fPowerRatio
															else:
																fPowerRatio = 99.0
														cPower = gc.getGame().getSymbolID(FontSymbols.STRENGTH_CHAR)
														szTempBuffer = BugUtil.formatFloat(fPowerRatio, ScoreOpt.getPowerDecimals()) + u"%c" % (cPower)
														if (iHighPowerColor >= 0 and fPowerRatio >= ScoreOpt.getHighPowerRatio()):
															szTempBuffer = localText.changeTextColor(szTempBuffer, iHighPowerColor)
														elif (iLowPowerColor >= 0 and fPowerRatio <= ScoreOpt.getLowPowerRatio()):
															szTempBuffer = localText.changeTextColor(szTempBuffer, iLowPowerColor)
														elif (iPowerColor >= 0):
															szTempBuffer = localText.changeTextColor(szTempBuffer, iPowerColor)
														szBuffer = szBuffer + u" " + szTempBuffer
														if (bAlignIcons):
															scores.setPower(szTempBuffer)
# BUG - Power Rating - end

# BUG - Attitude Icons - start
												if (ScoreOpt.isShowAttitude()):
													if (not gc.getPlayer(ePlayer).isHuman()):
														iAtt = gc.getPlayer(ePlayer).AI_getAttitude(gc.getGame().getActivePlayer())
														cAtt =  unichr(ord(unichr(CyGame().getSymbolID(FontSymbols.POWER_CHAR) + 5)) + iAtt)
														szBuffer += cAtt
														if (bAlignIcons):
															scores.setAttitude(cAtt)
# BUG - Attitude Icons - end

# BUG - Worst Enemy - start
												if (ScoreOpt.isShowWorstEnemy()):
													if (not gc.getPlayer(ePlayer).isHuman() and gc.getGame().getActivePlayer() != ePlayer):
														szWorstEnemy = gc.getPlayer(ePlayer).getWorstEnemyName()
														if (szWorstEnemy and gc.getActivePlayer().getName() == szWorstEnemy):
															cWorstEnemy = u"%c" %(CyGame().getSymbolID(FontSymbols.ANGRY_POP_CHAR))
															szBuffer += cWorstEnemy
															if (bAlignIcons):
																scores.setWorstEnemy()
# BUG - Worst Enemy - end
												# BUG: ...end of indentation
# BUG - Dead Civs - end

											if (CyGame().isNetworkMultiPlayer()):
												szTempBuffer = CyGameTextMgr().getNetStats(ePlayer)
												szBuffer = szBuffer + szTempBuffer
												if (bAlignIcons):
													scores.setNetStats(szTempBuffer)

											if (gc.getPlayer(ePlayer).isHuman() and CyInterface().isOOSVisible()):
												szTempBuffer = u" <color=255,0,0>* %s *</color>" %(CyGameTextMgr().getOOSSeeds(ePlayer))
												szBuffer = szBuffer + szTempBuffer
												if (bAlignIcons):
													scores.setNetStats(szTempBuffer)

											szBuffer = szBuffer + "</font>"

# BUG - Align Icons - start
											if (not bAlignIcons):
												if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
													iWidth = CyInterface().determineWidth( szBuffer )

												szName = "ScoreText" + str(ePlayer)
												if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW or CyInterface().isInAdvancedStart()):
													yCoord = yResolution - 206
												else:
													yCoord = yResolution - 88

# BUG - Dead Civs - start
												# Don't try to contact dead civs
												if (gc.getPlayer(ePlayer).isAlive()):
													iWidgetType = WidgetTypes.WIDGET_CONTACT_CIV
													iPlayer = ePlayer
												else:
													iWidgetType = WidgetTypes.WIDGET_GENERAL
													iPlayer = -1
												screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - (iCount * iBtnHeight), -0.3, FontTypes.SMALL_FONT, iWidgetType, iPlayer, -1 )
# BUG - Dead Civs - end
												screen.show( szName )

												CyInterface().checkFlashReset(ePlayer)

												iCount = iCount + 1
# BUG - Align Icons - end
							j = j - 1
					i = i - 1

# BUG - Align Icons - start
				if (bAlignIcons):
					scores.draw(screen)
				else:
					if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW or CyInterface().isInAdvancedStart()):
						yCoord = yResolution - 186
					else:
						yCoord = yResolution - 68
					screen.setPanelSize( "ScoreBackground", xResolution - 21 - iWidth, yCoord - (iBtnHeight * iCount) - 4, iWidth + 12, (iBtnHeight * iCount) + 8 )
					screen.show( "ScoreBackground" )
# BUG - Align Icons - end
				yCoord = yResolution - 186
#FfH Global Counter: Added by Kael 08/12/2007
				pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
				iCountSpecial = 0
				if (gc.getGame().isOption(GameOptionTypes.GAMEOPTION_CHALLENGE_INCREASING_DIFFICULTY) or gc.getGame().isOption(GameOptionTypes.GAMEOPTION_FLEXIBLE_DIFFICULTY)):
					iCountSpecial += 1
					szName = "DifficultyTag"
					szBuffer = u"<font=2>"
					szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_DIFFICULTY", (gc.getHandicapInfo(pPlayer.getHandicapType()).getDescription(), ()), gc.getInfoTypeForString("COLOR_RED"))
					szBuffer = szBuffer + "</font>"
#					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.show( szName )
					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, 100+ ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.show( szName )

				if (gc.getGame().isOption(GameOptionTypes.GAMEOPTION_CHALLENGE_CUT_LOSERS) or gc.getGame().isOption(GameOptionTypes.GAMEOPTION_WB_BARBARIAN_ASSAULT)):
					if gc.getGame().countCivPlayersAlive() > 5:
						iCountSpecial += 1
						szName = "CutLosersTag"
						szBuffer = u"<font=2>"
						szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_CUT_LOSERS", (gc.getGame().getCutLosersCounter(), ()), gc.getInfoTypeForString("COLOR_RED"))
						szBuffer = szBuffer + "</font>"
#						screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#						screen.show( szName )
						screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, 100+ ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
						screen.show( szName )

				if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_CHALLENGE_HIGH_TO_LOW):
					iCountSpecial += 1
					szName = "HighToLowTag"
					szBuffer = u"<font=2>"
					if gc.getGame().getHighToLowCounter() == 0:
						szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_HIGH_TO_LOW_GOAL_0", (), gc.getInfoTypeForString("COLOR_RED"))
					if gc.getGame().getHighToLowCounter() == 1:
						szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_HIGH_TO_LOW_GOAL_1", (), gc.getInfoTypeForString("COLOR_RED"))
					if gc.getGame().getHighToLowCounter() > 1:
						szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_HIGH_TO_LOW_GOAL_2", (), gc.getInfoTypeForString("COLOR_RED"))
					szBuffer = szBuffer + "</font>"
#					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.show( szName )
					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, 100+ ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.show( szName )

				iCountSpecial += 1
				if pPlayer.getDisableProduction() > 0:
					iCountSpecial += 1
					szBuffer = u"<font=2>"
					szName = "DisableProductionTag"
					szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_DISABLE_PRODUCTION", (pPlayer.getDisableProduction(), ()), gc.getInfoTypeForString("COLOR_RED"))
					szBuffer = szBuffer + "</font>"
#					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.show( szName )

					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, 100+(iCountSpecial*iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.show( szName )



				if pPlayer.getDisableResearch() > 0:
					iCountSpecial += 1
					szBuffer = u"<font=2>"
					szName = "DisableResearchTag"
					szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_DISABLE_RESEARCH", (pPlayer.getDisableResearch(), ()), gc.getInfoTypeForString("COLOR_RED"))
					szBuffer = szBuffer + "</font>"
#					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.show( szName )

					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, 100+(iCountSpecial*iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.show( szName )

				if pPlayer.getDisableSpellcasting() > 0:
					iCountSpecial += 1
					szBuffer = u"<font=2>"
					szName = "DisableSpellcastingTag"
					szBuffer = szBuffer + localText.getColorText("TXT_KEY_MESSAGE_DISABLE_SPELLCASTING", (pPlayer.getDisableSpellcasting(), ()), gc.getInfoTypeForString("COLOR_RED"))
					szBuffer = szBuffer + "</font>"

#					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, yCoord - ((iCount + iCountSpecial) * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
#					screen.show( szName )

					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 12, 100+(iCountSpecial*iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
					screen.show( szName )



#FfH: End Add

				screen.setPanelSize( "ScoreBackground", xResolution - 21 - iWidth, yCoord - (iBtnHeight * iCount) - 4, iWidth + 12, (iBtnHeight * iCount) + 8 )
				screen.show( "ScoreBackground" )

	def updateMagicStrings( self ):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
		global bshowMagicBar

		iCapitalCityID = -1
		iDisplayNumItems = 10
		iTechStringLength = 0
		#TODO: figure out if there is anything to display
		bshowMagicBar = 1

		global iShowItem # 0 = Summons, 1 = GE, 2 = Terraform

		if (pPlayer.getNumCities()) == 0 or (pPlayer.getCurrentMagicRitual() != -1):
			bshowMagicBar = 0
		else:
			iCapitalCityID = pPlayer.getCapitalCity().getID()

		screen.hide( "MagicBackground" )

		screen.hide( "MagicTextDiscription" )

		for i in range(iDisplayNumItems):
			szName = "MagicButton" + str(i)
			screen.hide( szName )

		iWidth = 0
		iCount = 0
		iBtnHeight = 30
		iIconWidth = 28

		yCoord = 230
		if (CyInterface().isScoresVisible() and not CyInterface().isCityScreenUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY and CyEngine().isGlobeviewUp() == false and bshowMagicBar == 1):

			if iShowItem == 0:
				szBuffer = u"<font=2> Summons: </font>"
				if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
					iWidth = CyInterface().determineWidth( szBuffer )
				screen.setText( "MagicTextDiscription" , "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, yCoord + 24, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.show( "MagicTextDiscription" )

				iCount = 0
				for j in range(gc.getNumProjectInfos()):
					if iCount >= iDisplayNumItems:
						break
					if pPlayer.canCreateSummon(j):
						eUnit = gc.getProjectInfo(j).getSummonUnitType()
						szName = "MagicButton" + str(iCount)
						screen.setImageButton( szName, gc.getProjectInfo(j).getButton(), 5 + (iCount * iIconWidth), yCoord + 24 + iBtnHeight, 32, 32, WidgetTypes.WIDGET_DO_SUMMON, eUnit, iCapitalCityID )
						screen.show( szName )
						iCount += 1

			if iShowItem == 1:
				szBuffer = u"<font=2> Enchantments: </font>"
				if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
					iWidth = CyInterface().determineWidth( szBuffer )
				screen.setText( "MagicTextDiscription" , "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, yCoord + 24, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.show( "MagicTextDiscription" )

				iCount = 0
				for j in range(gc.getNumProjectInfos()):
					if iCount >= iDisplayNumItems:
						break
					if pPlayer.canDoGlobalEnchantment(j):
						szName = "MagicButton" + str(iCount)
						screen.setImageButton( szName, gc.getProjectInfo(j).getButton(), 5 + (iCount * iIconWidth), yCoord + 24 + iBtnHeight, 32, 32, WidgetTypes.WIDGET_DO_GLOBAL_ENCHANTMENT, j, iCapitalCityID )
						screen.show( szName )
						iCount += 1

			if iShowItem == 2:
				szBuffer = u"<font=2> Terraforming: </font>"
				if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
					iWidth = CyInterface().determineWidth( szBuffer )
				screen.setText( "MagicTextDiscription" , "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, yCoord + 24, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1 )
				screen.show( "MagicTextDiscription" )

				iCount = 0
				for j in range(gc.getNumProjectInfos()):
					if iCount >= iDisplayNumItems:
						break
					if pPlayer.canDoTerraformRitual(j):
						szName = "MagicButton" + str(iCount)
						screen.setImageButton( szName, gc.getProjectInfo(j).getButton(), 5 + (iCount * iIconWidth), yCoord + 24 + iBtnHeight, 32, 32, WidgetTypes.WIDGET_DO_TERRAFORM, j, iCapitalCityID )
						screen.show( szName )
						iCount += 1

			screen.setPanelSize( "MagicBackground", 6, yCoord + 18, iWidth + 12, 70)
			screen.show( "MagicBackground" )


	def updateCultureStrings( self ):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
		ePlayer = gc.getGame().getActivePlayer()
		global bshowCultureBar

		iDisplayNumGuildTechs = 10
		bshowCultureBar = 0
		bShowCultureTraitText = 0
		iCapitalCityID = -1
		

		for j in range(gc.getNumTechInfos()):
			if pPlayer.canGetCulturalAchievement(j):
				bshowCultureBar = 1
				break
		for j in range(gc.getNumTraitInfos()):
			if pPlayer.canGetTrait(j):
				if pPlayer.getGlobalCulture() >= pPlayer.getTraitCost(j):
					bshowCultureBar = 1
					bShowCultureTraitText = 1
					break

		if pPlayer.getNumCities() == 0:
			bShowCultureBar = 0
		else:
			iCapitalCityID = pPlayer.getCapitalCity().getID()


		screen.hide( "CultureBackground" )

		for i in range(gc.getNumTechInfos()):
			szName = "CultureButton" + str(i)
			screen.hide( szName )

		screen.hide( "CultureTextDiscription" )
		screen.hide( "CultureTrait" )

		iWidth = 0
		iCount = 0
		iBtnHeight = 30
		iIconWidth = 28

		yCoord = 300
		if (CyInterface().isScoresVisible() and not CyInterface().isCityScreenUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY and CyEngine().isGlobeviewUp() == false and bshowCultureBar == 1):

			szBuffer = u"<font=2> Empire Culture: </font>"
			if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
				iWidth = CyInterface().determineWidth( szBuffer )
			screen.setText( "CultureTextDiscription" , "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, yCoord + 24, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GUILD_SCREEN, -1, -1 )
			screen.show( "CultureTextDiscription" )

			iCount = 0
			for j in range(gc.getNumTechInfos()):
			
				#Xtended - taking alignment into account
				bCanResearchEver = true
				iAlignmentPrereq=gc.getTechInfo(j).getPreferredAlignment()
				iAlignmentBlocked=gc.getTechInfo(j).getBlockedbyAlignment()
				iAlignmentPlayer=gc.getPlayer(ePlayer).getAlignment()
				if iAlignmentPrereq != -1:
					if iAlignmentPrereq != iAlignmentPlayer:
						bCanResearchEver = false
				if iAlignmentBlocked != -1:
					if iAlignmentBlocked == iAlignmentPlayer:
						bCanResearchEver = false
						
				iWayofWise = gc.getInfoTypeForString('TECH_WAY_OF_THE_WISE')
				iWayofWicked = gc.getInfoTypeForString('TECH_WAY_OF_THE_WICKED')
				iRighteousness = gc.getInfoTypeForString('TECH_RIGHTEOUSNESS')
				iCommune = gc.getInfoTypeForString('TECH_COMMUNE_WITH_NATURE')
				iMalevolent = gc.getInfoTypeForString('TECH_MALEVOLENT_DESIGNS')

				if j == iWayofWise:
					if pPlayer.isHasTech(iWayofWicked):
						bCanResearchEver = false
				elif j == iWayofWicked:
					if pPlayer.isHasTech(iWayofWise):
						bCanResearchEver = false
				elif j == iRighteousness:
					if pPlayer.isHasTech(iCommune):
						bCanResearchEver = false
				elif j == iCommune:
					if pPlayer.isHasTech(iRighteousness) or pPlayer.isHasTech(iMalevolent):
						bCanResearchEver = false
					if pPlayer.getCivilizationType() == gc.getInfoTypeForString('CIVILIZATION_ARISTRAKH'):
						bCanResearchEver = false
				elif j == iMalevolent:
					if pPlayer.isHasTech(iRighteousness) or pPlayer.isHasTech(iCommune):
						bCanResearchEver = false
						
					
				if iCount >= iDisplayNumGuildTechs:
					break
				if pPlayer.canGetCulturalAchievement(j):
					if (bCanResearchEver): #Xtended
						szName = "CultureButton" + str(iCount)
						screen.setImageButton( szName, gc.getTechInfo(j).getButton(), 5 + (iCount * iIconWidth), yCoord + 24 + iBtnHeight, 32, 32, WidgetTypes.WIDGET_CULTURE_ACHIEVEMENT, j, iCapitalCityID )
						screen.show( szName )
						iCount += 1

			if bShowCultureTraitText:
				szBuffer = u"<font=2> New Trait </font>"
				if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
					iWidth = CyInterface().determineWidth( szBuffer )
				screen.setText( "CultureTrait" , "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, 0, yCoord + 24 + (2 * iBtnHeight), -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GUILD_SCREEN, -1, -1 )
				screen.show( "CultureTrait" )


			screen.setPanelSize( "CultureBackground", 6, yCoord + 18, iWidth + 12, 70 + bShowCultureTraitText * iBtnHeight)
			screen.show( "CultureBackground" )



#FfH: Added by Kael 10/29/2007
	def updateManaStrings( self ):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()
		pPlayer = gc.getPlayer(gc.getGame().getActivePlayer())
		global bshowManaBar

		bshowManaBar=0
		if MainOpt.isShowManabar():
			bshowManaBar=1

		screen.hide( "ManaBackground" )

		for szBonus in manaTypes1:
			szName = "ManaText" + szBonus
			screen.hide( szName )
		for szBonus in manaTypes2:
			szName = "ManaText" + szBonus
			screen.hide( szName )

		iWidth = 0
		iCount = 0
		iBtnHeight = 18

		yCoord = 150
		if (CyInterface().isScoresVisible() and not CyInterface().isCityScreenUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_MINIMAP_ONLY and CyEngine().isGlobeviewUp() == false and bshowManaBar == 1):
			for szBonus in manaTypes1:
				iBonus = gc.getInfoTypeForString(szBonus)
				szBuffer = u"<font=2>"
				szTempBuffer = u"%c: %d" %(gc.getBonusInfo(iBonus).getChar(), pPlayer.getNumAvailableBonuses(iBonus))
				szBuffer = szBuffer + szTempBuffer
				szBuffer = szBuffer + "</font>"
				if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
					iWidth = CyInterface().determineWidth( szBuffer )
				szName = "ManaText" + szBonus
				screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 80, yCoord + (iCount * iBtnHeight) + 24, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iBonus, -1 )
				screen.show( szName )
				iCount = iCount + 1
			iCount = 0
			for szBonus in manaTypes2:
				iBonus = gc.getInfoTypeForString(szBonus)
				szBuffer = u"<font=2>"
				szTempBuffer = u"%c: %d" %(gc.getBonusInfo(iBonus).getChar(), pPlayer.getNumAvailableBonuses(iBonus))
				szBuffer = szBuffer + szTempBuffer
				szBuffer = szBuffer + "</font>"
				if ( CyInterface().determineWidth( szBuffer ) > iWidth ):
					iWidth = CyInterface().determineWidth( szBuffer )
				szName = "ManaText" + szBonus
				screen.setText( szName, "Background", szBuffer, CvUtil.FONT_RIGHT_JUSTIFY, xResolution - 40, yCoord + (iCount * iBtnHeight) + 24, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_PEDIA_JUMP_TO_BONUS, iBonus, -1 )
				screen.show( szName )
				iCount = iCount + 1
			screen.setPanelSize( "ManaBackground", xResolution - (iWidth * 2) - 20, yCoord + 18, (iWidth * 2) + 12, (iBtnHeight * 4) + 12 )
			screen.show( "ManaBackground" )
#FfH: End Add

	# Will update the help Strings
	def updateHelpStrings( self ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE_ALL ):
			screen.setHelpTextString( "" )
		else:
			screen.setHelpTextString( CyInterface().getHelpString() )

		return 0

	# Will set the promotion button position
	def setPromotionButtonPosition( self, szName, iPromotionCount ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Find out our resolution
		yResolution = screen.getYResolution()

		if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):

#FfH: Modified By Kael 07/17/2007
			screen.moveItem( szName, 216 - (24 * (iPromotionCount / 6)), yResolution - 144 + (24 * (iPromotionCount % 6)), -0.3 )
#FfH: End Modify

	# Will set the selection button position
	def setResearchButtonPosition( self, szButtonID, iCount ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()

# BUG - Bars on single line for higher resolution screens - start
		if (xResolution >= 1440	and MainOpt.isShowGPProgressBar()):
			xCoord = 268 + (xResolution - 1440) / 2
			xCoord += 6 + 84
			screen.moveItem( szButtonID, 264 + ( ( xResolution - 1024 ) / 2 ) + ( 34 * ( iCount % 15 ) ), 0 + ( 34 * ( iCount / 15 ) ), -0.3 )
		else:
			xCoord = 264 + ( ( xResolution - 1024 ) / 2 )

		screen.moveItem( szButtonID, xCoord + ( 34 * ( iCount % 15 ) ), 0 + ( 34 * ( iCount / 15 ) ), -0.3 )
# BUG - Bars on single line for higher resolution screens - end

	# Will set the selection button position
	def setScoreTextPosition( self, szButtonID, iWhichLine ):

		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		yResolution = screen.getYResolution()
		if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):
			yCoord = yResolution - 180
		else:
			yCoord = yResolution - 88
		screen.moveItem( szButtonID, 996, yCoord - (iWhichLine * 18), -0.3 )

	# Will build the globeview UI
	def updateGlobeviewButtons( self ):
		kInterface = CyInterface()
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		kEngine = CyEngine()
		kGLM = CyGlobeLayerManager()
		iNumLayers = kGLM.getNumLayers()
		iCurrentLayerID = kGLM.getCurrentLayerID()

		# Positioning things based on the visibility of the globe
		if kEngine.isGlobeviewUp():

#FfH: Modified by Kael 07/17/2008
#			screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, 7, yResolution - 50, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
			screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, iHelpX, yResolution - 50, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
#FfH: End Modify

		else:
			if ( CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_SHOW ):

#FfH: Modified by Kael 07/17/2008
#				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, 7, yResolution - 172, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, iHelpX, yResolution - 172, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
#FfH: End Modify

			else:

#FfH: Modified by Kael 07/17/2008
#				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, 7, yResolution - 50, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
				screen.setHelpTextArea( 350, FontTypes.SMALL_FONT, iHelpX, yResolution - 50, -0.1, False, "", True, False, CvUtil.FONT_LEFT_JUSTIFY, 150 )
#FfH: End Modify

		# Set base Y position for the LayerOptions, if we find them
		if CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE:
			iY = yResolution - iGlobeLayerOptionsY_Minimal
		else:
			iY = yResolution - iGlobeLayerOptionsY_Regular

		# Hide the layer options ... all of them
		for i in range (20):
			szName = "GlobeLayerOption" + str(i)
			screen.hide(szName)

		# Setup the GlobeLayer panel
		iNumLayers = kGLM.getNumLayers()
		if kEngine.isGlobeviewUp() and CyInterface().getShowInterface() != InterfaceVisibility.INTERFACE_HIDE_ALL:
			# set up panel
			if iCurrentLayerID != -1 and kGLM.getLayer(iCurrentLayerID).getNumOptions() != 0:
				bHasOptions = True
			else:
				bHasOptions = False
				screen.hide( "ScoreBackground" )

				screen.hide( "CultureBackground" )
				screen.hide( "MagicBackground" )
#FfH: Added by Kael 10/29/2007
				screen.hide( "ManaBackground" )
#FfH: End Add

			# set up toggle button
			screen.setState("GlobeToggle", True)

			# Set GlobeLayer indicators correctly
			for i in range(kGLM.getNumLayers()):
				szButtonID = "GlobeLayer" + str(i)
				screen.setState( szButtonID, iCurrentLayerID == i )

			# Set up options pane
			if bHasOptions:
				kLayer = kGLM.getLayer(iCurrentLayerID)

				iCurY = iY
				iNumOptions = kLayer.getNumOptions()
				iCurOption = kLayer.getCurrentOption()
				iMaxTextWidth = -1
				for iTmp in range(iNumOptions):
					iOption = iTmp # iNumOptions - iTmp - 1
					szName = "GlobeLayerOption" + str(iOption)
					szCaption = kLayer.getOptionName(iOption)
					if(iOption == iCurOption):
						szBuffer = "  <color=0,255,0>%s</color>  " % (szCaption)
					else:
						szBuffer = "  %s  " % (szCaption)
					iTextWidth = CyInterface().determineWidth( szBuffer )

					screen.setText( szName, "Background", szBuffer, CvUtil.FONT_LEFT_JUSTIFY, xResolution - 9 - iTextWidth, iCurY-iGlobeLayerOptionHeight-10, -0.3, FontTypes.SMALL_FONT, WidgetTypes.WIDGET_GLOBELAYER_OPTION, iOption, -1 )
					screen.show( szName )

					iCurY -= iGlobeLayerOptionHeight

					if iTextWidth > iMaxTextWidth:
						iMaxTextWidth = iTextWidth

				#make extra space
				iCurY -= iGlobeLayerOptionHeight;
				iPanelWidth = iMaxTextWidth + 32
				iPanelHeight = iY - iCurY
				iPanelX = xResolution - 14 - iPanelWidth
				iPanelY = iCurY
				screen.setPanelSize( "ScoreBackground", iPanelX, iPanelY, iPanelWidth, iPanelHeight )
				screen.show( "ScoreBackground" )

				screen.setPanelSize( "CultureBackground", iPanelX, iPanelY, iPanelWidth, iPanelHeight )
				screen.show( "CultureBackground" )
				screen.setPanelSize( "MagicBackground", iPanelX, iPanelY, iPanelWidth, iPanelHeight )
				screen.show( "MagicBackground" )


#FfH: Added by Kael 10/29/2007
				screen.setPanelSize( "ManaBackground", iPanelX, iPanelY, iPanelWidth, iPanelHeight )
				screen.show( "ManaBackground" )
#FfH: End Add

		else:
			if iCurrentLayerID != -1:
				kLayer = kGLM.getLayer(iCurrentLayerID)
				if kLayer.getName() == "RESOURCES":
					screen.setState("ResourceIcons", True)
				else:
					screen.setState("ResourceIcons", False)

				if kLayer.getName() == "UNITS":
					screen.setState("UnitIcons", True)
				else:
					screen.setState("UnitIcons", False)
			else:
				screen.setState("ResourceIcons", False)
				screen.setState("UnitIcons", False)

			screen.setState("Grid", CyUserProfile().getGrid())
			screen.setState("BareMap", CyUserProfile().getMap())
			screen.setState("Yields", CyUserProfile().getYields())
			screen.setState("ScoresVisible", CyUserProfile().getScores())

			screen.hide( "InterfaceGlobeLayerPanel" )
			screen.setState("GlobeToggle", False )

	# Update minimap buttons
	def setMinimapButtonVisibility( self, bVisible):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		kInterface = CyInterface()
		kGLM = CyGlobeLayerManager()
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		if ( CyInterface().isCityScreenUp() ):
			bVisible = False

		kMainButtons = ["UnitIcons", "Grid", "BareMap", "Yields", "ScoresVisible", "ResourceIcons"]
		kGlobeButtons = []
		for i in range(kGLM.getNumLayers()):
			szButtonID = "GlobeLayer" + str(i)
			kGlobeButtons.append(szButtonID)

		if bVisible:
			if CyEngine().isGlobeviewUp():
				kHide = kMainButtons
				kShow = kGlobeButtons
			else:
				kHide = kGlobeButtons
				kShow = kMainButtons
			screen.show( "GlobeToggle" )

		else:
			kHide = kMainButtons + kGlobeButtons
			kShow = []
			screen.hide( "GlobeToggle" )

		for szButton in kHide:
			screen.hide(szButton)

		if CyInterface().getShowInterface() == InterfaceVisibility.INTERFACE_HIDE:
			iY = yResolution - iMinimapButtonsY_Minimal
			iGlobeY = yResolution - iGlobeButtonY_Minimal
		else:
			iY = yResolution - iMinimapButtonsY_Regular
			iGlobeY = yResolution - iGlobeButtonY_Regular

		iBtnX = xResolution - 39
		screen.moveItem("GlobeToggle", iBtnX, iGlobeY, 0.0)

		iBtnAdvance = 28
		iBtnX = iBtnX - len(kShow)*iBtnAdvance - 10
		if len(kShow) > 0:
			i = 0
			for szButton in kShow:
				screen.moveItem(szButton, iBtnX, iY, 0.0)
				screen.moveToFront(szButton)
				screen.show(szButton)
				iBtnX += iBtnAdvance
				i += 1


	def createGlobeviewButtons( self ):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		kEngine = CyEngine()
		kGLM = CyGlobeLayerManager()
		iNumLayers = kGLM.getNumLayers()

		for i in range (kGLM.getNumLayers()):
			szButtonID = "GlobeLayer" + str(i)

			kLayer = kGLM.getLayer(i)
			szStyle = kLayer.getButtonStyle()

			if szStyle == 0 or szStyle == "":
				szStyle = "Button_HUDSmall_Style"

			screen.addCheckBoxGFC( szButtonID, "", "", 0, 0, 28, 28, WidgetTypes.WIDGET_GLOBELAYER, i, -1, ButtonStyles.BUTTON_STYLE_LABEL )
			screen.setStyle( szButtonID, szStyle )
			screen.hide( szButtonID )


	def createMinimapButtons( self ):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		xResolution = screen.getXResolution()
		yResolution = screen.getYResolution()

		screen.addCheckBoxGFC( "UnitIcons", "", "", 0, 0, 28, 28, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_UNIT_ICONS).getActionInfoIndex(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.setStyle( "UnitIcons", "Button_HUDGlobeUnit_Style" )
		screen.setState( "UnitIcons", False )
		screen.hide( "UnitIcons" )

		screen.addCheckBoxGFC( "Grid", "", "", 0, 0, 28, 28, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_GRID).getActionInfoIndex(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.setStyle( "Grid", "Button_HUDBtnGrid_Style" )
		screen.setState( "Grid", False )
		screen.hide( "Grid" )

		screen.addCheckBoxGFC( "BareMap", "", "", 0, 0, 28, 28, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_BARE_MAP).getActionInfoIndex(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.setStyle( "BareMap", "Button_HUDBtnClearMap_Style" )
		screen.setState( "BareMap", False )
		screen.hide( "BareMap" )

		screen.addCheckBoxGFC( "Yields", "", "", 0, 0, 28, 28, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_YIELDS).getActionInfoIndex(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.setStyle( "Yields", "Button_HUDBtnTileAssets_Style" )
		screen.setState( "Yields", False )
		screen.hide( "Yields" )

		screen.addCheckBoxGFC( "ScoresVisible", "", "", 0, 0, 28, 28, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_SCORES).getActionInfoIndex(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.setStyle( "ScoresVisible", "Button_HUDBtnRank_Style" )
		screen.setState( "ScoresVisible", True )
		screen.hide( "ScoresVisible" )

		screen.addCheckBoxGFC( "ResourceIcons", "", "", 0, 0, 28, 28, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_RESOURCE_ALL).getActionInfoIndex(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.setStyle( "ResourceIcons", "Button_HUDBtnResources_Style" )
		screen.setState( "ResourceIcons", False )
		screen.hide( "ResourceIcons" )

		screen.addCheckBoxGFC( "GlobeToggle", "", "", -1, -1, 36, 36, WidgetTypes.WIDGET_ACTION, gc.getControlInfo(ControlTypes.CONTROL_GLOBELAYER).getActionInfoIndex(), -1, ButtonStyles.BUTTON_STYLE_LABEL )
		screen.setStyle( "GlobeToggle", "Button_HUDZoom_Style" )
		screen.setState( "GlobeToggle", False )
		screen.hide( "GlobeToggle" )

##################################################################################################
# Winamp Start
##################################################################################################
	# handles the diaplay of the units info pane
	def showWinAMPInfoPane(self, sWinAMPButtonInfoText):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )

		# Defining the size of the info pane
		InfoPaneX = 5
		InfoPaneY = screen.getYResolution() - 205 # 208
		InfoPaneWidth = 260
		InfoPaneHeight = 30
		screen.addPanel( "WinAMP_INFO_PANE", u"", u"", True, True, InfoPaneX, InfoPaneY, InfoPaneWidth, InfoPaneHeight, PanelStyles.PANEL_STYLE_HUD_HELP )

		# create  text
		if (sWinAMPButtonInfoText == "1"):
			sWinAMPButtonText = localText.getText("TXT_KEY_WINAMP_INFO", ())
		elif (sWinAMPButtonInfoText == "2"):
			sWinAMPButtonText = localText.getText("TXT_KEY_WINAMP_MUTE", ())

		# convert text
		szText = "<font=2>" + sWinAMPButtonText + "</font=2>"

		# create  foreground text
		szTextFore = localText.changeTextColor(szText, gc.getInfoTypeForString("COLOR_MENU_BLUE"))

		# create shadow text
		szTextBlack = localText.changeTextColor(szText, gc.getInfoTypeForString("COLOR_BLACK"))

		# display shadow text
		screen.addMultilineText( "WinAMP_INFO_TEXT_SHADOW", szTextBlack, InfoPaneX +6, InfoPaneY +6, InfoPaneWidth -3, InfoPaneHeight- 3, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

		# display foreground text
		screen.addMultilineText( "WinAMP_INFO_TEXT", szTextFore, InfoPaneX +5, InfoPaneY +5, InfoPaneWidth -3, InfoPaneHeight- 3, WidgetTypes.WIDGET_GENERAL, -1, -1, CvUtil.FONT_LEFT_JUSTIFY)

	def hideWinAMPInfoPane(self):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		screen.hide("WinAMP_INFO_TEXT")
		screen.hide("WinAMP_INFO_TEXT_SHADOW")
		screen.hide("WinAMP_INFO_PANE")

	def showWinAMPControlPanel(self):
		self.WinAMPCounter = 1
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		screen.show("WinAMPPlayButton1")
		screen.show("WinAMPPauseButton1")
		screen.show("WinAMPStopButton1")
		screen.show("WinAMPPrevButton1")
		screen.show("WinAMPNextButton1")
		screen.show("WinAMPVolumeSlider")
		screen.show("WinAMPMuteButton1")
		screen.show("WinampText")

	def hideWinAMPControlPanel(self):
		self.WinAMPCounter = 0
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		screen.hide("WinAMPPlayButton1")
		screen.hide("WinAMPPauseButton1")
		screen.hide("WinAMPStopButton1")
		screen.hide("WinAMPPrevButton1")
		screen.hide("WinAMPNextButton1")
		screen.hide("WinAMPVolumeSlider")
		screen.hide("WinAMPMuteButton1")
		screen.hide("WinampText")

	def handleWinAMPMuteButton(self):
		if (CUP.isMasterNoSound()):
			CUP.setMasterNoSound(false)
		else:
			CUP.setMasterNoSound(true)
##################################################################################################
# Winamp End
##################################################################################################


	# Will handle the input for this screen...
	def handleInput (self, inputClass):
		screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
		#BugUtil.debugInput(inputClass)
# BUG - PLE - start
		if  (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON) or \
			(inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_OFF) or \
			(inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
			if (self.MainInterfaceInputMap.has_key(inputClass.getFunctionName())):
				return self.MainInterfaceInputMap.get(inputClass.getFunctionName())(inputClass)
			if (self.MainInterfaceInputMap.has_key(inputClass.getFunctionName() + "1")):
				return self.MainInterfaceInputMap.get(inputClass.getFunctionName() + "1")(inputClass)
# BUG - PLE - end

# BUG - Raw Yields - start
		if (inputClass.getFunctionName().startswith("RawYields")):
			return self.handleRawYieldsButtons(inputClass)
# BUG - Raw Yields - end

# BUG - Great Person Bar - start
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED and inputClass.getFunctionName().startswith("GreatPersonBar")):
			# Zoom to next GP city
			iCity = inputClass.getData1()
			if (iCity == -1):
				pCity, _ = GPUtil.findNextCity()
			else:
				pCity = gc.getActivePlayer().getCity(iCity)
			if pCity and not pCity.isNone():
				CyInterface().selectCity(pCity, False)
			return 1
# BUG - Great Person Bar - end

# BUG - field of view slider - start
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_SLIDER_NEWSTOP):
			if (inputClass.getFunctionName() == self.szSliderId):
				screen = CyGInterfaceScreen( "MainInterface", CvScreenEnums.MAIN_INTERFACE )
				self.iField_View = inputClass.getData() + 1
				self.setFieldofView(screen, False)
				self.setFieldofView_Text(screen)
				MainOpt.setFieldOfView(self.iField_View)
# BUG - field of view slider - end
		global bshowManaBar
		global iShowItem

#SCIONS
		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON and (inputClass.getFunctionName() == "Awakenedchance" or inputClass.getFunctionName() == "SRText")):
			screen.show("SpawnOddsHelpText")
			screen.show("SpawnOddsHelpTextPanel")

		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON and (inputClass.getFunctionName() == "SidarCounter" or inputClass.getFunctionName() == "ShadeText")):
			screen.show("SidarCounterHelpText")
			screen.show("SidarCounterHelpTextPanel")

		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON and (inputClass.getFunctionName() == "SpiritVault" or inputClass.getFunctionName() == "SpiritVaultText")):
			screen.show("SpiritVaultHelpText")
			screen.show("SpiritVaultHelpTextPanel")

#Unblemished
#		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED and inputClass.getFunctionName() == "AccordButton"):
#			CvUnblemishedEvents.UnblemishedFunctions().Vimpopup()
#			screen.show("AccordHelpText")
#			screen.show("AccordHelpTextPanel")
#/Unblemished


#/SCIONS
		if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON and inputClass.getFunctionName() == "RawManaButton"):
			screen.show("ManaToggleHelpText")
			screen.show("ManaToggleHelpTextPanel")
		elif ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_OFF and inputClass.getFunctionName() == "RawManaButton"):
			screen.hide("ManaToggleHelpText")
			screen.hide("ManaToggleHelpTextPanel")

		if(inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED and inputClass.getFunctionName() == "RawManaButton"):
			if (bshowManaBar == 1):
				bshowManaBar = 0
				self.updateManaStrings()
				return 1
			else:
				bshowManaBar = 1
				self.updateManaStrings()
				return 1

		if(inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED and inputClass.getFunctionName() == "MagicTextDiscription"):
			iShowItem = iShowItem + 1
			if iShowItem > 2:
				iShowItem = 0
			self.updateMagicStrings()
			return 1

##################################################################################################
# Winamp Start
##################################################################################################
		if gc.getGame().isOption(GameOptionTypes.GAMEOPTION_WINAMP_GUI):
			if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON and inputClass.getFunctionName() == "WinAMPButton"):
				self.showWinAMPInfoPane("1")
			elif ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_OFF and inputClass.getFunctionName() == "WinAMPButton"):
				self.hideWinAMPInfoPane()
			if(inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED and inputClass.getFunctionName() == "WinAMPButton"):
				if (self.WinAMPCounter == 1):
					self.hideWinAMPControlPanel()
				else:
					self.showWinAMPControlPanel()

			if ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON and inputClass.getFunctionName() == "WinAMPMuteButton"):
				self.showWinAMPInfoPane("2")
			elif ( inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_OFF and inputClass.getFunctionName() == "WinAMPMuteButton"):
				self.hideWinAMPInfoPane()
			if(inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED and inputClass.getFunctionName() == "WinAMPMuteButton"):
				self.handleWinAMPMuteButton()

			elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_SLIDER_NEWSTOP):
				WinAMP.SetVolume(inputClass.getData())
##################################################################################################
# Winamp End
##################################################################################################

		return 0

# BUG - Raw Yields - start
	def handleRawYieldsButtons(self, inputClass):
		iButton = inputClass.getID()
		# BlackArchon: this should have been removed on revision 101
		#if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_ON):
		#	self.displayHelpHover(RAW_YIELD_HELP[iButton])
		#elif (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CURSOR_MOVE_OFF):
		#	self.hideInfoPane()
		if (inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED):
			global g_bYieldView
			global g_iYieldType
			global g_iYieldTiles
			if iButton == 0:
				g_bYieldView = False
			elif iButton in (1, 2, 3):
				g_bYieldView = True
				g_iYieldType = RawYields.YIELDS[iButton - 1]
			elif iButton in (4, 5, 6):
				g_bYieldView = True
				g_iYieldTiles = RawYields.TILES[iButton - 4]
			else:
				return 0
			CyInterface().setDirty(InterfaceDirtyBits.CityScreen_DIRTY_BIT, True)
			return 1
		return 0
# BUG - Raw Yields - end
	def update(self, fDelta):
		return

# BUG - field of view slider - start
	def setFieldofView(self, screen, bDefault):
		if (bDefault
		or not MainOpt.isShowFieldOfView()):
			self._setFieldofView(screen, DEFAULT_FIELD_OF_VIEW)
		else:
			self._setFieldofView(screen, self.iField_View)

	def _setFieldofView(self, screen, iFoV):
		if self.iField_View_Prev != iFoV:
			gc.setDefineFLOAT("FIELD_OF_VIEW",float(iFoV))
			self.iField_View_Prev = iFoV
			screen.setForcedRedraw(True)

	def setFieldofView_Text(self, screen):
		zsFieldOfView_Text = "%s [%i]" % (self.sFieldOfView_Text, self.iField_View)
		screen.setLabel(self.szSliderTextId, "", zsFieldOfView_Text, CvUtil.FONT_RIGHT_JUSTIFY, self.iX_FoVSlider, self.iY_FoVSlider + 6, 0, FontTypes.GAME_FONT, WidgetTypes.WIDGET_GENERAL, -1, -1)
# BUG - field of view slider - end
