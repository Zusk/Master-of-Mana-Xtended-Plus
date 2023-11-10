#-------------------------------------------------------------------------------
# Name:        CvDotMapOverlayScreen.py
# Purpose:     Custom Screen for the dot map of the Strategy Overlay
#
# Author:      Del69, EmperorFool
#
# Created:     09/01/2009
#-------------------------------------------------------------------------------

from CvPythonExtensions import *
import ColorUtil
import CvStrategyOverlay
import CvUtil
#-------------------------------------------------------------------------------
# Constants
#-------------------------------------------------------------------------------
X, Y = 0, 1    # used in point tuples instead of creating a new class
#-------------------------------------------------------------------------------
# Globals
#-------------------------------------------------------------------------------
g_GC = CyGlobalContext()
g_DotMap = None

#-------------------------------------------------------------------------------
# Keyboard event handler to show or close screen
#-------------------------------------------------------------------------------

class CvDotMapOverlayScreen:
	"""
	Screen for the dot map of the strategy overlay.
	"""
	def __init__(self, screenID):
		self.screenID = screenID
		self.SCREEN_NAME = "DotMapOverlayScreen"
		#---------------------------------------------------------------------------
		# Panel IDS
		#---------------------------------------------------------------------------
		self.PREFIX = "DotMapOverlay"
		self.COLOR_PANEL_ID = self.PREFIX + "ColorPanel"
		#---------------------------------------------------------------------------
		# Main Panel Coordinates
		#---------------------------------------------------------------------------
		self.PANEL_MARGIN = 15
		self.TITLE_HEIGHT = 16
		self.TEXT_HEIGHT = 16
		self.TEXT_SPACING = 5
		self.PANEL_SPACING = 5
		self.Z_TEXT = -0.3
		self.Z_CONTROL = -0.3
		#---------------------------------------------------------------------------
		# Color Values
		#---------------------------------------------------------------------------
		self.COLOR_KEYS = ColorUtil.getColorKeys()
		self.COLOR_TEXT = ColorUtil.getColorDisplayNames()
		#---------------------------------------------------------------------------
		# Color Panel Widgets
		#---------------------------------------------------------------------------
		self.COLOR_WIDGET_PREFIX = self.COLOR_PANEL_ID + "Widget"
		self.COLOR_WIDGET_IDS = []
		#---------------------------------------------------------------------------
		# Color Panel Coordinates
		#---------------------------------------------------------------------------
		self.COLOR_PANEL_HEADER_H = 15
		self.COLOR_PANEL_X = 10
		self.COLOR_PANEL_Y = 75 + self.PANEL_MARGIN * 2 + self.TITLE_HEIGHT * 2 + 2 * self.TEXT_HEIGHT + 3 * self.TEXT_SPACING + self.PANEL_SPACING
		self.COLOR_PANEL_TEXT_X = self.COLOR_PANEL_X + self.PANEL_MARGIN
		self.COLOR_PANEL_TEXT_Y = self.COLOR_PANEL_Y + self.PANEL_MARGIN
		self.COLOR_PANEL_W = 0
		self.COLOR_PANEL_H = len(self.COLOR_KEYS) * (self.TEXT_HEIGHT + self.TEXT_SPACING) + self.PANEL_MARGIN * 2
		#-------------------------------------------------------------------------------
		# Constants
		#-------------------------------------------------------------------------------
		self.HIGHLIGHT_CROSS_LAYER = 8
		self.FIRST_CROSS_LAYER = 9
		self.NUM_CROSS_LAYERS = len(self.COLOR_KEYS)
		self.PLOT_LAYER = PlotLandscapeLayers.PLOT_LANDSCAPE_LAYER_NUMPAD_HELP
		self.DOT_STYLE = PlotStyles.PLOT_STYLE_DOT_TARGET
		self.NO_DOT_STYLE = PlotStyles.PLOT_STYLE_NONE
		self.BFC_OFFSETS = []
		for x in range(-2, 3):
			for y in range(-2, 3):
				if abs(x) != 2 or abs(y) != 2:
					self.BFC_OFFSETS.append((x, y))
		#---------------------------------------------------------------------------
		# State variables
		#---------------------------------------------------------------------------
		self.currentColor = 0
		self.bLeftMouseDown = False
		self.bRightMouseDown = False
		self.bInitPos = False
		self.currentLayer = self.FIRST_CROSS_LAYER
		self.currentPoint = None
		#-------------------------------------------------------------------------------
		# Widget Input Mapping
		#-------------------------------------------------------------------------------
		self.inputMap = {
			self.COLOR_WIDGET_PREFIX : self.onColorButton,
		}
	#-------------------------------------------------------------------------------
	# Initialization Functions
	#-------------------------------------------------------------------------------
	def initVars(self):
		"""
		Initializes the variables for the screen.
		"""
		global g_DotMap
		g_DotMap = CvStrategyOverlay.getDotMap()
		for index in range(len(self.COLOR_KEYS)):
			self.COLOR_WIDGET_IDS.append(self.COLOR_WIDGET_PREFIX + str(index))
		#-----------------------------------------------------------------------
		# Coordinate Initializations
		#-----------------------------------------------------------------------
		if not self.bInitPos:
			for colorTxt in self.COLOR_TEXT:
				width = CyInterface().determineWidth(colorTxt)
				if (width > self.COLOR_PANEL_W):
					self.COLOR_PANEL_W = width
			self.COLOR_PANEL_W += 2 * self.PANEL_MARGIN
			self.bInitPos = True

	def isOpen(self):
		"""
		Public accessor to check if screen is open or not
		"""
		return self.getScreen().isActive()
	#-------------------------------------------------------------------------------
	# Required civ4 functions
	#-------------------------------------------------------------------------------
	def getScreen(self):
		"""
		Gets the CyGInterfaceScreen Object for this screen
		"""
		return CyGInterfaceScreen(self.SCREEN_NAME, self.screenID)

	def hideScreen(self):
		"""
		Hides the Screen
		"""
		g_DotMap.unhighlightCity()
		CyInterface().setInterfaceMode(InterfaceModeTypes.INTERFACEMODE_SELECTION)
		screen = self.getScreen()
		screen.hideScreen()

	def update(self, fDelta):
		"""
		Called each update cycle, checks for mouse button clicks when screen is up
		"""
		if CyInterface().isLeftMouseDown():
			if not self.bLeftMouseDown:
				self.onLeftMouseDown()
			self.bLeftMouseDown = True
		else:
			self.bLeftMouseDown = False
		if CyInterface().isRightMouseDown():
			if not self.bRightMouseDown:
				self.onRightMouseDown()
			self.bRightMouseDown = True
		else:
			self.bRightMouseDown = False

	def handleInput(self, inputClass):
		"""
		Handles screens widget input
		"""
		if(self.inputMap.has_key(inputClass.getFunctionName())):
			self.inputMap.get(inputClass.getFunctionName())(inputClass)
			return 1
		return 0

	def interfaceScreen(self):
		"""
		Initializes and shows the screen
		"""
		screen = self.getScreen()
		if screen.isActive():
			return
		screen.setCloseOnEscape(False)
		screen.setAlwaysShown(True)
		screen.setForcedRedraw(True)
		#-------------------------------------------------------------------------------
		# Init variables
		#-------------------------------------------------------------------------------
		self.initVars()
		#-------------------------------------------------------------------------------
		# Add controls and update them
		#-------------------------------------------------------------------------------
		self.addColorControls()
		self.updateColorControls(self.currentColor)
		#-------------------------------------------------------------------------------
		# Set the screen size and show it
		#-------------------------------------------------------------------------------
		screen.setDimensions(0,0,screen.getXResolution(),screen.getYResolution())
		screen.showScreen(PopupStates.POPUPSTATE_IMMEDIATE, True)
		#-------------------------------------------------------------------------------
		# place into correct cursor mode
		#-------------------------------------------------------------------------------
		self.resetInterfaceMode()
		self.onMouseOverPlot(())

	#-------------------------------------------------------------------------------
	#   Widget Adding/Updating Functions
	#-------------------------------------------------------------------------------
	def addColorControls(self):
		"""
		    Adds the color selection texts to the color selection panel
		"""
		screen = self.getScreen()
		screen.addPanel(self.COLOR_PANEL_ID,u"",u"",True,False,self.COLOR_PANEL_X,self.COLOR_PANEL_Y,self.COLOR_PANEL_W,self.COLOR_PANEL_H,PanelStyles.PANEL_STYLE_MAIN)
		for index in range(len(self.COLOR_KEYS)):
			txtY = self.COLOR_PANEL_TEXT_Y + index * (self.TEXT_SPACING + self.TEXT_HEIGHT)
			screen.setText(self.COLOR_WIDGET_PREFIX + str(index),self.COLOR_PANEL_ID,self.COLOR_TEXT[index],CvUtil.FONT_LEFT_JUSTIFY,self.COLOR_PANEL_TEXT_X,txtY,self.Z_TEXT,FontTypes.SMALL_FONT,WidgetTypes.WIDGET_GENERAL,-1,-1)

	def updateColorControls(self,selectedColorIndex):
		"""
			Highlights the passed in color index and dehighlights the old one
		"""
		screen = self.getScreen()
		oldColorTxt = self.COLOR_TEXT[self.currentColor]
		newColorTxt = " <color=0,255,0>%s</color> " % (self.COLOR_TEXT[selectedColorIndex])
		yOld = self.COLOR_PANEL_TEXT_Y + self.currentColor*self.TEXT_SPACING + self.currentColor*self.TEXT_HEIGHT
		yNew = self.COLOR_PANEL_TEXT_Y + selectedColorIndex*self.TEXT_SPACING + selectedColorIndex*self.TEXT_HEIGHT
		screen.setText(self.COLOR_WIDGET_PREFIX + str(self.currentColor),self.COLOR_PANEL_ID,oldColorTxt,CvUtil.FONT_LEFT_JUSTIFY,self.COLOR_PANEL_TEXT_X,yOld,self.Z_TEXT,FontTypes.SMALL_FONT,WidgetTypes.WIDGET_GENERAL,-1,-1)
		screen.setText(self.COLOR_WIDGET_PREFIX + str(selectedColorIndex),self.COLOR_PANEL_ID,newColorTxt,CvUtil.FONT_LEFT_JUSTIFY,self.COLOR_PANEL_TEXT_X,yNew,self.Z_TEXT,FontTypes.SMALL_FONT,WidgetTypes.WIDGET_GENERAL,-1,-1)
		screen.setState(self.COLOR_WIDGET_IDS[selectedColorIndex],True)
		screen.setState(self.COLOR_WIDGET_IDS[self.currentColor],False)
		self.currentColor = selectedColorIndex
		self.currentLayer = selectedColorIndex + self.FIRST_CROSS_LAYER

	#-------------------------------------------------------------------------------
	#   Input handlers
	#-------------------------------------------------------------------------------
	def onColorButton(self,inputClass):
		"""
		Called on Color Button input
		"""
		if inputClass.getNotifyCode() == NotifyCode.NOTIFY_CLICKED:
			g_DotMap.unhighlightCity()
			self.updateColorControls(inputClass.getID())
	
	def onLeftMouseDown(self):
		"""
		Called on left mouse click on the main ui
		"""
		if self.currentPoint is not None:
			g_DotMap.unhighlightCity()
			g_DotMap.addCityAt(self.currentPoint, self.currentColor, self.currentLayer)
			self.resetInterfaceMode()

	def onRightMouseDown(self):
		"""
		Called on right mouse click on the main ui
		"""
		if self.currentPoint is not None:
			g_DotMap.unhighlightCity()
			g_DotMap.removeCityAt(self.currentPoint)
			self.resetInterfaceMode()

	def onMouseOverPlot(self, argsList):
		"""
		Called from CvOverlayScreenUtils when mousing over a plot when the screen is active
		Updates the current plot and its x/y location
		"""
		plot = CyInterface().getMouseOverPlot()
		x = plot.getX()
		y = plot.getY()
		self.currentPoint = (x, y)
		g_DotMap.highlightCity(self.currentPoint, self.currentColor)
		self.resetInterfaceMode()
	
	def resetInterfaceMode(self):
		if CyInterface().getInterfaceMode() != InterfaceModeTypes.INTERFACEMODE_PYTHON_PICK_PLOT:
			CyInterface().setInterfaceMode(InterfaceModeTypes.INTERFACEMODE_PYTHON_PICK_PLOT)
